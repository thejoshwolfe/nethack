/* See LICENSE in the root of this project for change info */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "move.h"
#include "dungeon_util.h"
#include "apply.h"
#include "artifact.h"
#include "attrib.h"
#include "cmd.h"
#include "coord.h"
#include "dbridge.h"
#include "decl.h"
#include "detect.h"
#include "dig.h"
#include "display.h"
#include "do.h"
#include "do_name.h"
#include "do_wear.h"
#include "dog.h"
#include "dothrow.h"
#include "dungeon.h"
#include "eat.h"
#include "end.h"
#include "engrave.h"
#include "explode.h"
#include "flag.h"
#include "global.h"
#include "hack.h"
#include "hacklib.h"
#include "invent.h"
#include "light.h"
#include "lock.h"
#include "makemon.h"
#include "mkobj.h"
#include "mkroom.h"
#include "mon.h"
#include "monattk.h"
#include "mondata.h"
#include "monflag.h"
#include "monmove.h"
#include "monst.h"
#include "monsym.h"
#include "muse.h"
#include "music.h"
#include "obj.h"
#include "objclass.h"
#include "objnam.h"
#include "onames.h"
#include "permonst.h"
#include "pickup.h"
#include "pline.h"
#include "pm.h"
#include "polyself.h"
#include "potion.h"
#include "prop.h"
#include "read.h"
#include "rm.h"
#include "rnd.h"
#include "shk.h"
#include "skills.h"
#include "sounds.h"
#include "steal.h"
#include "steed.h"
#include "teleport.h"
#include "timeout.h"
#include "trap.h"
#include "uhitm.h"
#include "util.h"
#include "vision.h"
#include "weapon.h"
#include "wield.h"
#include "wintype.h"
#include "wizard.h"
#include "worn.h"
#include "write.h"
#include "you.h"
#include "youprop.h"
#include "zap.h"

static const char tools[] = { TOOL_CLASS, WEAPON_CLASS, WAND_CLASS, 0 };
static const char tools_too[] = { ALL_CLASSES, TOOL_CLASS, POTION_CLASS,
    WEAPON_CLASS, WAND_CLASS, GEM_CLASS, 0 };

#define MAXLEASHED      2


#define WEAK    3       /* from eat.c */

static const char cuddly[] = { TOOL_CLASS, GEM_CLASS, 0 };
static const char lubricables[] = { ALL_CLASSES, ALLOW_NONE, 0 };
static struct trapinfo {
    struct obj *tobj;
    signed char tx, ty;
    int time_needed;
    bool force_bungle;
} trapinfo;

#define BY_OBJECT       ((struct monst *)0)

#define PROP_COUNT 6            /* number of properties we're dealing with */
#define ATTR_COUNT (A_MAX*3)    /* number of attribute points we might fix */

#define prop2trbl(X)    ((X) + A_MAX)


static int use_camera (struct obj *obj) {
    struct monst *mtmp;

    if (Underwater) {
        message_const(MSG_USING_CAMERA_UNDERWATER);
        return(0);
    }
    if(!getdir((char *)0)) return(0);

    if (obj->spe <= 0) {
        message_const(MSG_NOTHING_HAPPENS);
        return 1;
    }
    consume_obj_charge(obj, true);

    if (obj->cursed && !rn2(2)) {
        zapyourself(obj, true);
    } else if (u.uswallow) {
        message_monster_string(MSG_YOU_TAKE_PICTURE_OF_SWALLOW, u.ustuck,
                mbodypart(u.ustuck, STOMACH));
    } else if (u.dz) {
        message_string(MSG_YOU_TAKE_PICTURE_OF_DUNGEON,
                (u.dz > 0) ? surface(u.ux,u.uy) : ceiling(u.ux,u.uy));
    } else if (!u.dx && !u.dy) {
        zapyourself(obj, true);
    } else if ((mtmp = bhit(u.dx, u.dy, COLNO, FLASHED_LIGHT, NULL, NULL, obj)) != 0) {
        obj->ox = u.ux,  obj->oy = u.uy;
        flash_hits_mon(mtmp, obj);
    }
    return 1;
}

static int use_towel (struct obj *obj) {
    if (!freehand()) {
        message_const(MSG_YOU_HAVE_NO_FREE_HAND);
        return 0;
    } else if (obj->owornmask) {
        message_const(MSG_CANNOT_USE_WHILE_WEARING);
        return 0;
    } else if (obj->cursed) {
        long old;
        switch (rn2(3)) {
            case 2:
                old = Glib;
                Glib += rn1(10, 3);
                message_const(old ? MSG_YOUR_HANDS_FILTHIER : MSG_YOUR_HANDS_GET_SLIMY);
                return 1;
            case 1:
                if (!ublindf) {
                    old = u.ucreamed;
                    u.ucreamed += rn1(10, 3);
                    message_const(old ? MSG_YOUR_FACE_HAS_MORE_GUNK : MSG_YOUR_FACE_NOW_HAS_GUNK);
                    make_blinded(Blinded + (long)u.ucreamed - old, true);
                } else {
                    if (ublindf->cursed) {
                        message_const(MSG_YOU_PUSH_YOUR_LENSES_CROOKED);
                    } else {
                        message_const(MSG_YOU_PUSH_YOUR_LENSES_OFF);
                        struct obj *saved_ublindf = ublindf;
                        Blindf_off(ublindf);
                        dropx(saved_ublindf);
                    }
                }
                return 1;
            case 0:
                break;
        }
    }

    if (Glib) {
        Glib = 0;
        message_const(MSG_YOU_WIPE_OFF_YOUR_HANDS);
        return 1;
    } else if(u.ucreamed) {
        Blinded -= u.ucreamed;
        u.ucreamed = 0;

        if (!Blinded) {
            message_const(MSG_YOU_GOT_THE_GLOP_OFF);
            Blinded = 1;
            make_blinded(0L,true);
        } else {
            message_const(MSG_YOUR_FACE_FEELS_CLEAN_NOW);
        }
        return 1;
    }

    message_const(MSG_YOUR_FACE_AND_HAND_ARE_CLEAN);

    return 0;
}

/* maybe give a stethoscope message based on floor objects */
static bool its_dead(int rx, int ry, int *resp) {
    struct obj *otmp;
    struct trap *ttmp;

    if (!can_reach_floor()) return false;

    /* additional stethoscope messages from jyoung@apanix.apana.org.au */
    if (Hallucination() && sobj_at(CORPSE, rx, ry)) {
        /* (a corpse doesn't retain the monster's sex,
           so we're forced to use generic pronoun here) */
        message_const(MSG_ITS_DEAD_JIM);
        *resp = 1;
        return true;
    } else if (Role_if(PM_HEALER) && ((otmp = sobj_at(CORPSE, rx, ry)) != 0 ||
                (otmp = sobj_at(STATUE, rx, ry)) != 0)) {
        /* possibly should check uppermost {corpse,statue} in the pile
           if both types are present, but it's not worth the effort */
        if (vobj_at(rx, ry)->otyp == STATUE) otmp = vobj_at(rx, ry);
        if (otmp->otyp == CORPSE) {
            message_const(MSG_YOU_DETERMINE_ITS_DEAD);
        } else {
            ttmp = t_at(rx, ry);
            message_object((ttmp && ttmp->ttyp == STATUE_TRAP) ?
                    MSG_STATUE_APPEARS_EXCELLENT : MSG_STATUE_APPEARS_EXTRAORDINARY, otmp);
        }
        return true;
    }
    return false;
}

/* Strictly speaking it makes no sense for usage of a stethoscope to
   not take any time; however, unless it did, the stethoscope would be
   almost useless.  As a compromise, one use per turn is free, another
   uses up the turn; this makes curse status have a tangible effect. */
static int use_stethoscope (struct obj *obj) {
    static long last_used_move = -1;
    static short last_used_movement = 0;
    struct monst *mtmp;
    struct rm *lev;
    int rx, ry, res;
    bool interference = (u.uswallow && is_whirly(u.ustuck->data) &&
            !rn2(Role_if(PM_HEALER) ? 10 : 3));

    if (nohands(youmonst.data)) {   /* should also check for no ears and/or deaf */
        message_const(MSG_YOU_HAVE_NO_HANDS);
        return 0;
    } else if (!freehand()) {
        message_const(MSG_YOU_HAVE_NO_FREE_HANDS);
        return 0;
    }
    if (!getdir((char *)0)) return 0;

    res = (moves == last_used_move) &&
        (youmonst.movement == last_used_movement);
    last_used_move = moves;
    last_used_movement = youmonst.movement;

    if (u.usteed && u.dz > 0) {
        if (interference) {
            message_monster(MSG_MONSTER_INTERFERES, u.ustuck);
            mstatusline(u.ustuck);
        } else
            mstatusline(u.usteed);
        return res;
    } else
        if (u.uswallow && (u.dx || u.dy || u.dz)) {
            mstatusline(u.ustuck);
            return res;
        } else if (u.uswallow && interference) {
            message_monster(MSG_MONSTER_INTERFERES, u.ustuck);
            mstatusline(u.ustuck);
            return res;
        } else if (u.dz) {
            if (Underwater) {
                message_const(MSG_YOU_HEAR_FAINT_SPLASHING);
            } else if (u.dz < 0 || !can_reach_floor()) {
                message_string(MSG_YOU_CANNOT_REACH_THE_DUNGEON,
                        (u.dz > 0) ? surface(u.ux,u.uy) : ceiling(u.ux,u.uy));
            } else if (its_dead(u.ux, u.uy, &res)) {
                /* message already given */
            } else if (Is_stronghold(&u.uz)) {
                message_const(MSG_YOU_HEAR_CRACKLING_OF_HELLFIRE);
            } else {
                message_string(MSG_DUNGEON_SEEMS_HEALTHY_ENOUGH, surface(u.ux,u.uy));
            }
            return res;
        } else if (obj->cursed && !rn2(2)) {
            message_const(MSG_YOU_HEAR_YOUR_HEART_BEAT);
            return res;
        }
    if (Stunned() || (Confusion() && !rn2(5))) confdir();
    if (!u.dx && !u.dy) {
        ustatusline();
        return res;
    }
    rx = u.ux + u.dx; ry = u.uy + u.dy;
    if (!isok(rx,ry)) {
        message_const(MSG_YOU_HEAR_FAINT_TYPING_NOISE);
        return 0;
    }
    if ((mtmp = m_at(rx,ry)) != 0) {
        mstatusline(mtmp);
        if (mtmp->mundetected) {
            mtmp->mundetected = 0;
            if (cansee(rx,ry)) newsym(mtmp->mx,mtmp->my);
        }
        if (!canspotmon(mtmp))
            map_invisible(rx,ry);
        return res;
    }
    if (glyph_is_invisible(levl[rx][ry].glyph)) {
        unmap_object(rx, ry);
        newsym(rx, ry);
        message_const(MSG_THE_INVISIBLE_MONSTER_MOVED);
    }
    lev = &levl[rx][ry];
    switch(lev->typ) {
        case SDOOR:
            message_const(MSG_FOUND_SECRET_DOOR);
            cvt_sdoor_to_door(lev);         /* ->typ = DOOR */
            if (Blind) feel_location(rx,ry);
            else newsym(rx,ry);
            return res;
        case SCORR:
            message_const(MSG_FOUND_SECRET_PASSAGE);
            lev->typ = CORR;
            unblock_point(rx,ry);
            if (Blind) feel_location(rx,ry);
            else newsym(rx,ry);
            return res;
    }

    if (!its_dead(rx, ry, &res))
        message_const(MSG_YOU_HEAR_NOTHING_SPECIAL);

    return res;
}

static void use_whistle (struct obj *obj) {
    message_const(obj->cursed ? MSG_WHISTLE_SHRILL : MSG_WHISTLE_HIGH);
    wake_nearby();
}

static void use_magic_whistle (struct obj *obj) {
    struct monst *mtmp, *nextmon;

    if(obj->cursed && !rn2(2)) {
        message_const(MSG_YOU_PRODUCE_HIGH_HUMMING_NOISE);
        wake_nearby();
    } else {
        message_const(MSG_WHISTLE_MAGIC);
        int pet_cnt = 0;
        for(mtmp = fmon; mtmp; mtmp = nextmon) {
            nextmon = mtmp->nmon; /* trap might kill mon */
            if (DEADMONSTER(mtmp)) continue;
            if (mtmp->mtame) {
                if (mtmp->mtrapped) {
                    /* no longer in previous trap (affects mintrap) */
                    mtmp->mtrapped = 0;
                    fill_pit(mtmp->mx, mtmp->my);
                }
                mnexto(mtmp);
                if (canspotmon(mtmp)) ++pet_cnt;
                if (mintrap(mtmp) == 2) change_luck(-1);
            }
        }
        if (pet_cnt > 0) makeknown(obj->otyp);
    }
}

bool um_dist(signed char x, signed char y, signed char n) {
    return((bool)(abs(u.ux - x) > n  || abs(u.uy - y) > n));
}

int number_leashed(void) {
    int i = 0;
    struct obj *obj;

    for(obj = invent; obj; obj = obj->nobj)
        if(obj->otyp == LEASH && obj->leashmon != 0) i++;
    return(i);
}

/* otmp is about to be destroyed or stolen */
void o_unleash (struct obj *otmp) {
    struct monst *mtmp;

    for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
        if(mtmp->m_id == (unsigned)otmp->leashmon)
            mtmp->mleashed = 0;
    otmp->leashmon = 0;
}

/* mtmp is about to die, or become untame */
void m_unleash(struct monst *mtmp, bool feedback) {
    struct obj *otmp;

    if (feedback) {
        if (canseemon(mtmp)) {
            message_monster(MSG_M_PULLS_FREE_OF_LEASH, mtmp);
        } else {
            message_const(MSG_YOUR_LEASH_FALLS_SLACK);
        }
    }
    for(otmp = invent; otmp; otmp = otmp->nobj) {
        if(otmp->otyp == LEASH && otmp->leashmon == (int)mtmp->m_id)
            otmp->leashmon = 0;
    }
    mtmp->mleashed = 0;
}

/* player is about to die (for bones) */
void unleash_all (void) {
    struct obj *otmp;
    struct monst *mtmp;

    for(otmp = invent; otmp; otmp = otmp->nobj)
        if(otmp->otyp == LEASH) otmp->leashmon = 0;
    for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
        mtmp->mleashed = 0;
}

static void use_leash (struct obj *obj) {
    coord cc;
    struct monst *mtmp;
    int spotmon;

    if (!obj->leashmon && number_leashed() >= MAXLEASHED) {
        message_const(MSG_YOU_CANNOT_LEASH_MORE_PETS);
        return;
    }

    if(!get_adjacent_loc((char *)0, (char *)0, u.ux, u.uy, &cc)) return;

    if((cc.x == u.ux) && (cc.y == u.uy)) {
        if (u.usteed && u.dz > 0) {
            mtmp = u.usteed;
            spotmon = 1;
            goto got_target;
        }
        message_const(MSG_LEASH_YOURSELF);
        return;
    }

    if(!(mtmp = m_at(cc.x, cc.y))) {
        message_const(MSG_THERE_IS_NO_CREATURE_THERE);
        return;
    }

    spotmon = canspotmon(mtmp);
got_target:

    if(!mtmp->mtame) {
        if(!spotmon) {
            message_const(MSG_THERE_IS_NO_CREATURE_THERE);
        } else {
            message_monster(MSG_M_NOT_LEASHED, mtmp);
        }
        return;
    }
    if(!obj->leashmon) {
        if(mtmp->mleashed) {
            message_monster(MSG_M_ALREADY_LEASHED, mtmp);
            return;
        }
        message_monster(MSG_YOU_SLIP_LEASH_AROUND_M, mtmp);
        mtmp->mleashed = 1;
        obj->leashmon = (int)mtmp->m_id;
        mtmp->msleeping = 0;
        return;
    }
    if(obj->leashmon != (int)mtmp->m_id) {
        message_const(MSG_LEASH_NOT_ATTACHED_TO_CREATURE);
        return;
    } else {
        if(obj->cursed) {
            message_const(MSG_LEASH_NOT_COME_OFF);
            obj->bknown = true;
            return;
        }
        mtmp->mleashed = 0;
        obj->leashmon = 0;
        message_monster(MSG_YOU_REMOVE_LEASH_FROM_M, mtmp);
    }
}

/* assuming mtmp->mleashed has been checked */
struct obj * get_mleash (struct monst *mtmp) {
    struct obj *otmp;

    otmp = invent;
    while(otmp) {
        if(otmp->otyp == LEASH && otmp->leashmon == (int)mtmp->m_id)
            return(otmp);
        otmp = otmp->nobj;
    }
    return NULL;
}

bool next_to_u(void) {
    struct monst *mtmp;
    struct obj *otmp;

    for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp)) continue;
        if(mtmp->mleashed) {
            if (distu(mtmp->mx,mtmp->my) > 2) mnexto(mtmp);
            if (distu(mtmp->mx,mtmp->my) > 2) {
                for(otmp = invent; otmp; otmp = otmp->nobj)
                    if(otmp->otyp == LEASH && otmp->leashmon == (int)mtmp->m_id) {
                        if(otmp->cursed)
                            return false;
                        message_const(MSG_YOU_FEEL_LEASH_GO_SLACK);
                        mtmp->mleashed = 0;
                        otmp->leashmon = 0;
                    }
            }
        }
    }
    /* no pack mules for the Amulet */
    if (u.usteed && mon_has_amulet(u.usteed)) return false;
    return(true);
}

void check_leash (signed char x, signed char y) {
    struct obj *otmp;
    struct monst *mtmp;

    for (otmp = invent; otmp; otmp = otmp->nobj) {
        if (otmp->otyp != LEASH || otmp->leashmon == 0) continue;
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp)) continue;
            if ((int)mtmp->m_id == otmp->leashmon) break;
        }
        if (!mtmp) {
            impossible("leash in use isn't attached to anything?");
            otmp->leashmon = 0;
            continue;
        }
        if (dist2(u.ux,u.uy,mtmp->mx,mtmp->my) > dist2(x,y,mtmp->mx,mtmp->my)) {
            if (!um_dist(mtmp->mx, mtmp->my, 3)) {
                ;   /* still close enough */
            } else if (otmp->cursed && !breathless(mtmp->data)) {
                if (um_dist(mtmp->mx, mtmp->my, 5) || (mtmp->mhp -= rnd(2)) <= 0) {
                    long save_pacifism = u.uconduct.killer;

                    message_monster(MSG_YOUR_LEASH_CHOKES_M_TO_DEATH, mtmp);
                    /* hero might not have intended to kill pet, but
                       that's the result of his actions; gain experience,
                       lose pacifism, take alignment and luck hit, make
                       corpse less likely to remain tame after revival */
                    xkilled(mtmp, 0);       /* no "you kill it" message */
                    /* life-saving doesn't ordinarily reset this */
                    if (mtmp->mhp > 0) u.uconduct.killer = save_pacifism;
                } else {
                    message_monster(MSG_M_CHOKES_ON_LEASH, mtmp);

                    /* tameness eventually drops to 1 here (never 0) */
                    if (mtmp->mtame && rn2(mtmp->mtame))
                        mtmp->mtame--;
                }
            } else {
                if (um_dist(mtmp->mx, mtmp->my, 5)) {
                    message_monster(MSG_M_LEASH_SNAPS_LOOSE, mtmp);
                    m_unleash(mtmp, false);
                } else {
                    message_const(MSG_YOU_PULL_ON_LEASH);
                    if (mtmp->data->msound != MS_SILENT) {
                        switch (rn2(3)) {
                            case 0:  growl(mtmp);   break;
                            case 1:  yelp(mtmp);    break;
                            default: whimper(mtmp); break;
                        }
                    }
                }
            }
        }
    }
}

static int use_mirror (struct obj *obj) {
    struct monst *mtmp;
    char mlet;
    bool vis;

    if(!getdir((char *)0)) return 0;
    if(obj->cursed && !rn2(2)) {
        if (!Blind) {
            message_const(MSG_MIRROR_FOGS_UP);
        }
        return 1;
    }
    if (!u.dx && !u.dy && !u.dz) {
        if(!Blind && !Invisible) {
            if (u.umonnum == PM_FLOATING_EYE) {
                if (!Free_action) {
                    message_const(MSG_MIRROR_STARES_BACK);
                    nomul(-rnd((MAXULEV+6) - u.ulevel));
                } else {
                    message_const(MSG_YOU_STIFFEN_MOMENTARILY_UNDER_YOUR_GAZE);
                }
            } else if (youmonst.data->mlet == S_VAMPIRE) {
                message_const(MSG_YOU_HAVE_NO_REFLECTION);
            } else if (u.umonnum == PM_UMBER_HULK) {
                message_const(MSG_HUH_NO_LOOK_LIKE_YOU);
                make_confused(get_HConfusion() + d(3,4), false);
            } else if (Hallucination()) {
                message_int(MSG_YOU_LOOK_COLOR, halluc_color_int());
            } else if (Sick) {
                message_const(MSG_YOU_LOOK_PEAKED);
            } else if (u.uhs >= WEAK) {
                message_const(MSG_YOU_LOOK_UNDERNOURISHED);
            } else {
                message_const(MSG_YOU_LOOK_GOOD_AS_EVER);
            }
        } else {
            message_const(MSG_YOU_CANT_SEE_YOUR_FACE);
        }
        return 1;
    }
    if (u.uswallow) {
        if (!Blind)
            message_monster(MSG_YOU_REFLECT_M_STOMACH, u.ustuck);
        return 1;
    }
    if (Underwater) {
        message_const(MSG_YOU_APPLY_MIRROR_UNDERWATER);
        return 1;
    }
    if (u.dz) {
        if (!Blind) {
            message_string(MSG_YOU_REFLECT_THE_DUNGEON,
                    (u.dz > 0) ? surface(u.ux,u.uy) : ceiling(u.ux,u.uy));
        }
        return 1;
    }
    mtmp = bhit(u.dx, u.dy, COLNO, INVIS_BEAM, NULL, NULL, obj);
    if (!mtmp || !haseyes(mtmp->data))
        return 1;

    vis = canseemon(mtmp);
    mlet = mtmp->data->mlet;
    if (mtmp->msleeping) {
        if (vis) {
            message_monster(MSG_M_TOO_TIRED_LOOK_MIRROR, mtmp);
        }
    } else if (!mtmp->mcansee) {
        if (vis) {
            message_monster(MSG_M_CANT_SEE_ANYTHING, mtmp);
        }
        /* some monsters do special things */
    } else if (mlet == S_VAMPIRE || mlet == S_GHOST) {
        if (vis) {
            message_monster(MSG_M_HAS_NO_REFLECTION, mtmp);
        }
    } else if(!mtmp->mcan && !mtmp->minvis && mtmp->data == &mons[PM_MEDUSA]) {
        if (mon_reflects(mtmp, "The gaze is reflected away by %s %s!"))
            return 1;
        if (vis)
            message_monster(MSG_M_IS_TURNED_TO_STONE, mtmp);
        stoned = true;
        killed(mtmp);
    } else if(!mtmp->mcan && !mtmp->minvis &&
            mtmp->data == &mons[PM_FLOATING_EYE]) {
        int tmp = d((int)mtmp->m_lev, (int)mtmp->data->mattk[0].damd);
        if (!rn2(4)) tmp = 120;
        if (vis)
            message_monster(MSG_M_FROZEN_BY_REFLECTION, mtmp);
        else
            message_const(MSG_SOMETHING_STOP_MOVING);
        mtmp->mcanmove = 0;
        if ( (int) mtmp->mfrozen + tmp > 127)
            mtmp->mfrozen = 127;
        else mtmp->mfrozen += tmp;
    } else if(!mtmp->mcan && !mtmp->minvis && mtmp->data == &mons[PM_UMBER_HULK]) {
        if (vis) {
            message_monster(MSG_M_CONFUSES_ITSELF, mtmp);
        }
        mtmp->mconf = 1;
    } else if(!mtmp->mcan && !mtmp->minvis && (mlet == S_NYMPH
                || mtmp->data==&mons[PM_SUCCUBUS])) {
        if (vis) {
            char name[BUFSZ];
            Monnam(name, BUFSZ, mtmp);
            pline ("%s admires herself in your mirror.", name);
            pline ("She takes it!");
        } else {
            pline ("It steals your mirror!");
        }
        setnotworn(obj); /* in case mirror was wielded */
        freeinv(obj);
        mpickobj(mtmp,obj);
        if (!tele_restrict(mtmp))
            rloc(mtmp, false);
    } else if (!is_unicorn(mtmp->data) && !humanoid(mtmp->data) &&
            (!mtmp->minvis || perceives(mtmp->data)) && rn2(5)) {
        if (vis) {
            char name[BUFSZ];
            Monnam(name, BUFSZ, mtmp);
            pline("%s is frightened by its reflection.", name);
        }
        monflee(mtmp, d(2,4), false, false);
    } else if (!Blind) {
        if (mtmp->minvis && !See_invisible()) {
            // nothing
        } else if ((mtmp->minvis && !perceives(mtmp->data)) || !haseyes(mtmp->data)) {
            char name[BUFSZ];
            Monnam(name, BUFSZ, mtmp);
            pline("%s doesn't seem to notice its reflection.", name);
        } else {
            char name[BUFSZ];
            Monnam(name, BUFSZ, mtmp);
            pline("%s ignores %s reflection.", name, mhis(mtmp));
        }
    }
    return 1;
}

static void use_bell (struct obj **optr) {
    struct obj *obj = *optr;
    struct monst *mtmp;
    bool wakem = false, learno = false,
            ordinary = (obj->otyp != BELL_OF_OPENING || !obj->spe),
            invoking = (obj->otyp == BELL_OF_OPENING &&
                    invocation_pos(u.ux, u.uy) && !On_stairs(u.ux, u.uy));

    You("ring %s.", the(xname(obj)));

    if (Underwater || (u.uswallow && ordinary)) {
        pline("But the sound is muffled.");

    } else if (invoking && ordinary) {
        /* needs to be recharged... */
        pline("But it makes no sound.");
        learno = true;      /* help player figure out why */

    } else if (ordinary) {
        if (obj->cursed && !rn2(4) &&
                /* note: once any of them are gone, we stop all of them */
                !(mvitals[PM_WOOD_NYMPH].mvflags & G_GONE) &&
                !(mvitals[PM_WATER_NYMPH].mvflags & G_GONE) &&
                !(mvitals[PM_MOUNTAIN_NYMPH].mvflags & G_GONE) &&
                (mtmp = makemon(mkclass(S_NYMPH, 0),
                                u.ux, u.uy, NO_MINVENT)) != 0) {
            char a_name[BUFSZ];
            a_monnam(a_name, BUFSZ, mtmp);
            You("summon %s!", a_name);
            if (!obj_resists(obj, 93, 100)) {
                char clause[BUFSZ];
                Tobjnam(clause, BUFSZ, obj, "have");
                pline("%s shattered!", clause);
                useup(obj);
                *optr = 0;
            } else switch (rn2(3)) {
                default:
                    break;
                case 1:
                    mon_adjust_speed(mtmp, 2, (struct obj *)0);
                    break;
                case 2: /* no explanation; it just happens... */
                    nomovemsg = "";
                    nomul(-rnd(2));
                    break;
            }
        }
        wakem = true;

    } else {
        /* charged Bell of Opening */
        consume_obj_charge(obj, true);

        if (u.uswallow) {
            if (!obj->cursed)
                (void) openit();
            else
                message_const(MSG_NOTHING_HAPPENS);

        } else if (obj->cursed) {
            coord mm;

            mm.x = u.ux;
            mm.y = u.uy;
            mkundead(&mm, false, NO_MINVENT);
            wakem = true;

        } else if (invoking) {
            char clause[BUFSZ];
            Tobjnam(clause, BUFSZ, obj, "issue");
            pline("%s an unsettling shrill sound...", clause);
            obj->age = moves;
            learno = true;
            wakem = true;

        } else if (obj->blessed) {
            int res = 0;

            if (uchain) {
                unpunish();
                res = 1;
            }
            res += openit();
            switch (res) {
                case 0:
                    message_const(MSG_NOTHING_HAPPENS);
                    break;
                case 1:
                    pline("%s opens...", Something);
                    learno = true;
                    break;
                default:
                    pline("Things open around you...");
                    learno = true;
                    break;
            }

        } else {  /* uncursed */
            if (findit() != 0)
                learno = true;
            else
                message_const(MSG_NOTHING_HAPPENS);
        }

    }       /* charged BofO */

    if (learno) {
        makeknown(BELL_OF_OPENING);
        obj->known = 1;
    }
    if (wakem) wake_nearby();
}

static void use_candelabrum (struct obj *obj) {
    const char *s = (obj->spe != 1) ? "candles" : "candle";

    if (Underwater) {
        You("cannot make fire under water.");
        return;
    }
    if (obj->lamplit) {
        You("snuff the %s.", s);
        end_burn(obj, true);
        return;
    }
    if (obj->spe <= 0) {
        pline("This %s has no %s.", xname(obj), s);
        return;
    }
    if (u.uswallow || obj->cursed) {
        if (!Blind) {
            char flicker[BUFSZ];
            vtense(flicker, BUFSZ, s, "flicker");
            char die[BUFSZ];
            vtense(die, BUFSZ, s, "die");
            pline_The("%s %s for a moment, then %s.", s, flicker, die);
        }
        return;
    }
    if (obj->spe < 7) {
        char are[BUFSZ];
        vtense(are, BUFSZ, s, "are");
        There("%s only %d %s in %s.", are, obj->spe, s, the(xname(obj)));
        if (!Blind) {
            char clause[BUFSZ];
            Tobjnam(clause, BUFSZ, obj, "shine");
            pline("%s lit.  %s dimly.", obj->spe == 1 ? "It is" : "They are", clause);
        }
    } else {
        pline("%s's %s burn%s", The(xname(obj)), s, (Blind ? "." : " brightly!"));
    }
    if (!invocation_pos(u.ux, u.uy)) {
        char are[BUFSZ];
        vtense(are, BUFSZ, s, "are");
        pline_The("%s %s being rapidly consumed!", s, are);
        obj->age /= 2;
    } else {
        if(obj->spe == 7) {
            if (Blind) {
                char radiate_clause[BUFSZ];
                Tobjnam(radiate_clause, BUFSZ, obj, "radiate");
                pline("%s a strange warmth!", radiate_clause);
            } else {
                char glow_clause[BUFSZ];
                Tobjnam(glow_clause, BUFSZ, obj, "glow");
                pline("%s with a strange light!", glow_clause);
            }
        }
        obj->known = 1;
    }
    begin_burn(obj, false);
}

static void use_lamp (struct obj *obj) {
    char buf[BUFSZ];

    if(Underwater) {
        pline("This is not a diving lamp.");
        return;
    }
    if(obj->lamplit) {
        if(obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
                obj->otyp == BRASS_LANTERN)
            pline("%s lamp is now off.", Shk_Your(buf, obj));
        else
            You("snuff out %s.", yname(obj));
        end_burn(obj, true);
        return;
    }
    /* magic lamps with an spe == 0 (wished for) cannot be lit */
    if ((!Is_candle(obj) && obj->age == 0)
            || (obj->otyp == MAGIC_LAMP && obj->spe == 0)) {
        if (obj->otyp == BRASS_LANTERN)
            Your("lamp has run out of power.");
        else pline("This %s has no oil.", xname(obj));
        return;
    }
    if (obj->cursed && !rn2(2)) {
        char otense_buf[BUFSZ];
        otense(otense_buf, BUFSZ, obj, "die");
        char flicker_clause[BUFSZ];
        Tobjnam(flicker_clause, BUFSZ, obj, "flicker");
        pline("%s for a moment, then %s.", flicker_clause, otense_buf);
    } else {
        if(obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
                obj->otyp == BRASS_LANTERN) {
            check_unpaid(obj);
            pline("%s lamp is now on.", Shk_Your(buf, obj));
        } else {        /* candle(s) */
            const char *name = Yname2(obj);
            char otense_buf[BUFSZ];
            otense(otense_buf, BUFSZ, obj, "burn");
            pline("%s%s flame%s %s%s", name, possessive_suffix(name),
                    plur(obj->quan), otense_buf,
                    Blind ? "." : " brightly!");
            if (obj->unpaid && costly_spot(u.ux, u.uy) &&
                    obj->age == 20L * (long)objects[obj->otyp].oc_cost) {
                const char *ithem = obj->quan > 1L ? "them" : "it";
                verbalize("You burn %s, you bought %s!", ithem, ithem);
                bill_dummy_object(obj);
            }
        }
        begin_burn(obj, false);
    }
}

static void use_candle (struct obj **optr) {
    struct obj *obj = *optr;
    struct obj *otmp;
    const char *s = (obj->quan != 1) ? "candles" : "candle";
    char qbuf[QBUFSZ];

    if(u.uswallow) {
        message_const(MSG_NO_ELBOW_ROOM);
        return;
    }
    if(Underwater) {
        pline("Sorry, fire and water don't mix.");
        return;
    }

    otmp = carrying(CANDELABRUM_OF_INVOCATION);
    if(!otmp || otmp->spe == 7) {
        use_lamp(obj);
        return;
    }

    sprintf(qbuf, "Attach %s", the(xname(obj)));
    sprintf(eos(qbuf), " to %s?",
            safe_qbuf(qbuf, sizeof(" to ?"), the(xname(otmp)),
                the(simple_typename(otmp->otyp)), "it"));
    if(yn(qbuf) == 'n') {
        if (!obj->lamplit)
            You("try to light %s...", the(xname(obj)));
        use_lamp(obj);
        return;
    } else {
        if ((long)otmp->spe + obj->quan > 7L)
            obj = splitobj(obj, 7L - (long)otmp->spe);
        else *optr = 0;
        You("attach %ld%s %s to %s.",
                obj->quan, !otmp->spe ? "" : " more",
                s, the(xname(otmp)));
        if (!otmp->spe || otmp->age > obj->age)
            otmp->age = obj->age;
        otmp->spe += (int)obj->quan;
        if (otmp->lamplit && !obj->lamplit) {
            char ignite[BUFSZ];
            vtense(ignite, BUFSZ, s, "ignite");
            pline_The("new %s magically %s!", s, ignite);
        } else if (!otmp->lamplit && obj->lamplit) {
            pline("%s out.", (obj->quan > 1L) ? "They go" : "It goes");
        }
        if (obj->unpaid)
            verbalize("You %s %s, you bought %s!",
                    otmp->lamplit ? "burn" : "use",
                    (obj->quan > 1L) ? "them" : "it",
                    (obj->quan > 1L) ? "them" : "it");
        if (obj->quan < 7L && otmp->spe == 7)
            pline("%s now has seven%s candles attached.",
                    The(xname(otmp)), otmp->lamplit ? " lit" : "");
        /* candelabrum's light range might increase */
        if (otmp->lamplit) obj_merge_light_sources(otmp, otmp);
        /* candles are no longer a separate light source */
        if (obj->lamplit) end_burn(obj, true);
        /* candles are now gone */
        useupall(obj);
    }
}

/* call in drop, throw, and put in box, etc. */
bool snuff_candle(struct obj *otmp) {
    bool candle = Is_candle(otmp);

    if ((candle || otmp->otyp == CANDELABRUM_OF_INVOCATION) &&
            otmp->lamplit) {
        char buf[BUFSZ];
        signed char x, y;
        bool many = candle ? otmp->quan > 1L : otmp->spe > 1;

        (void) get_obj_location(otmp, &x, &y, 0);
        if (otmp->where == OBJ_MINVENT ? cansee(x,y) : !Blind)
            pline("%s %scandle%s flame%s extinguished.",
                    Shk_Your(buf, otmp),
                    (candle ? "" : "candelabrum's "),
                    (many ? "s'" : "'s"), (many ? "s are" : " is"));
        end_burn(otmp, true);
        return(true);
    }
    return(false);
}

/* called when lit lamp is hit by water or put into a container or
   you've been swallowed by a monster; obj might be in transit while
   being thrown or dropped so don't assume that its location is valid */
bool snuff_lit(struct obj *obj) {
    signed char x, y;

    if (obj->lamplit) {
        if (obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
                obj->otyp == BRASS_LANTERN || obj->otyp == POT_OIL)
        {
            get_obj_location(obj, &x, &y, 0);
            if (obj->where == OBJ_MINVENT ? cansee(x,y) : !Blind) {
                char otense_buf[BUFSZ];
                otense(otense_buf, BUFSZ, obj, "go");
                pline("%s %s out!", Yname2(obj), otense_buf);
            }
            end_burn(obj, true);
            return true;
        }
        if (snuff_candle(obj)) return true;
    }
    return false;
}

/* Called when potentially lightable object is affected by fire_damage().
   Return true if object was lit and false otherwise --ALI */
bool catch_lit(struct obj *obj) {
    signed char x, y;

    if (!obj->lamplit && (obj->otyp == MAGIC_LAMP || ignitable(obj))) {
        if ((obj->otyp == MAGIC_LAMP ||
                    obj->otyp == CANDELABRUM_OF_INVOCATION) &&
                obj->spe == 0)
            return false;
        else if (obj->otyp != MAGIC_LAMP && obj->age == 0)
            return false;
        if (!get_obj_location(obj, &x, &y, 0))
            return false;
        if (obj->otyp == CANDELABRUM_OF_INVOCATION && obj->cursed)
            return false;
        if ((obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
                    obj->otyp == BRASS_LANTERN) && obj->cursed && !rn2(2))
            return false;
        if (obj->where == OBJ_MINVENT ? cansee(x,y) : !Blind) {
            char otense_buf[BUFSZ];
            otense(otense_buf, BUFSZ, obj, "catch");
            pline("%s %s light!", Yname2(obj), otense_buf);
        }
        if (obj->otyp == POT_OIL) makeknown(obj->otyp);
        if (obj->unpaid && costly_spot(u.ux, u.uy) && (obj->where == OBJ_INVENT)) {
            /* if it catches while you have it, then it's your tough luck */
            check_unpaid(obj);
            verbalize("That's in addition to the cost of %s %s, of course.",
                    Yname2(obj), obj->quan == 1 ? "itself" : "themselves");
            bill_dummy_object(obj);
        }
        begin_burn(obj, false);
        return true;
    }
    return false;
}


/* obj is a potion of oil */
static void light_cocktail (struct obj *obj) {
    char buf[BUFSZ];

    if (u.uswallow) {
        message_const(MSG_NO_ELBOW_ROOM);
        return;
    }

    if (obj->lamplit) {
        You("snuff the lit potion.");
        end_burn(obj, true);
        /*
         * Free & add to re-merge potion.  This will average the
         * age of the potions.  Not exactly the best solution,
         * but its easy.
         */
        freeinv(obj);
        (void) addinv(obj);
        return;
    } else if (Underwater) {
        There("is not enough oxygen to sustain a fire.");
        return;
    }

    You("light %s potion.%s", shk_your(buf, obj),
            Blind ? "" : "  It gives off a dim light.");
    if (obj->unpaid && costly_spot(u.ux, u.uy)) {
        /* Normally, we shouldn't both partially and fully charge
         * for an item, but (Yendorian Fuel) Taxes are inevitable...
         */
        check_unpaid(obj);
        verbalize("That's in addition to the cost of the potion, of course.");
        bill_dummy_object(obj);
    }
    makeknown(obj->otyp);

    if (obj->quan > 1L) {
        obj = splitobj(obj, 1L);
        begin_burn(obj, false);     /* burn before free to get position */
        obj_extract_self(obj);      /* free from inv */

        /* shouldn't merge */
        obj = hold_another_object(obj, "You drop %s!",
                doname(obj), (const char *)0);
    } else
        begin_burn(obj, false);
}

/* touchstones - by Ken Arnold */
static void use_stone (struct obj *tstone) {
    struct obj *obj;
    bool do_scratch;
    const char *streak_color, *choices;
    char stonebuf[QBUFSZ];
    static const char scritch[] = "\"scritch, scritch\"";
    static const char allowall[3] = { COIN_CLASS, ALL_CLASSES, 0 };
    static const char justgems[3] = { ALLOW_NONE, GEM_CLASS, 0 };
    struct obj goldobj;

    /* in case it was acquired while blinded */
    if (!Blind) tstone->dknown = 1;
    /* when the touchstone is fully known, don't bother listing extra
       junk as likely candidates for rubbing */
    choices = (tstone->otyp == TOUCHSTONE && tstone->dknown &&
            objects[TOUCHSTONE].oc_name_known) ? justgems : allowall;
    sprintf(stonebuf, "rub on the stone%s", plur(tstone->quan));
    if ((obj = getobj(choices, stonebuf)) == 0)
        return;
    if (obj->oclass == COIN_CLASS) {
        u.ugold += obj->quan;   /* keep botl up to date */
        goldobj = *obj;
        dealloc_obj(obj);
        obj = &goldobj;
    }

    if (obj == tstone && obj->quan == 1) {
        You_cant("rub %s on itself.", the(xname(obj)));
        return;
    }

    if (tstone->otyp == TOUCHSTONE && tstone->cursed &&
            obj->oclass == GEM_CLASS && !is_graystone(obj) &&
            !obj_resists(obj, 80, 100)) {
        if (Blind)
            pline("You feel something shatter.");
        else if (Hallucination())
            pline("Oh, wow, look at the pretty shards.");
        else
            pline("A sharp crack shatters %s%s.",
                    (obj->quan > 1) ? "one of " : "", the(xname(obj)));
        useup(obj);
        return;
    }

    if (Blind) {
        plines(scritch);
        return;
    } else if (Hallucination()) {
        pline("Oh wow, man: Fractals!");
        return;
    }

    do_scratch = false;
    streak_color = 0;

    switch (obj->oclass) {
        case GEM_CLASS:     /* these have class-specific handling below */
        case RING_CLASS:
            if (tstone->otyp != TOUCHSTONE) {
                do_scratch = true;
            } else if (obj->oclass == GEM_CLASS && (tstone->blessed ||
                        (!tstone->cursed &&
                         (Role_if(PM_ARCHEOLOGIST) || Race_if(PM_GNOME)))))
            {
                makeknown(TOUCHSTONE);
                makeknown(obj->otyp);
                prinv((char *)0, obj, 0L);
                return;
            } else {
                /* either a ring or the touchstone was not effective */
                if (objects[obj->otyp].oc_material == GLASS) {
                    do_scratch = true;
                    break;
                }
            }
            streak_color = c_obj_colors[objects[obj->otyp].oc_color];
            break;          /* gem or ring */

        default:
            switch (objects[obj->otyp].oc_material) {
                case CLOTH:
                    {
                        char look_clause[BUFSZ];
                        Tobjnam(look_clause, BUFSZ, tstone, "look");
                        pline("%s a little more polished now.", look_clause);
                        return;
                    }
                case LIQUID:
                    /* note: not "whetstone" */
                    if (!obj->known) {
                        You("must think this is a wetstone, do you?");
                    } else {
                        char are_clause[BUFSZ];
                        Tobjnam(are_clause, BUFSZ, tstone, "are");
                        pline("%s a little wetter now.", are_clause);
                    }
                    return;
                case WAX:
                    streak_color = "waxy";
                    break;              /* okay even if not touchstone */
                case WOOD:
                    streak_color = "wooden";
                    break;              /* okay even if not touchstone */
                case GOLD:
                    do_scratch = true;  /* scratching and streaks */
                    streak_color = "golden";
                    break;
                case SILVER:
                    do_scratch = true;  /* scratching and streaks */
                    streak_color = "silvery";
                    break;
                default:
                    /* Objects passing the is_flimsy() test will not
                       scratch a stone.  They will leave streaks on
                       non-touchstones and touchstones alike. */
                    if (is_flimsy(obj))
                        streak_color = c_obj_colors[objects[obj->otyp].oc_color];
                    else
                        do_scratch = (tstone->otyp != TOUCHSTONE);
                    break;
            }
            break;          /* default oclass */
    }

    sprintf(stonebuf, "stone%s", plur(tstone->quan));
    if (do_scratch)
        pline("You make %s%sscratch marks on the %s.",
                streak_color ? streak_color : (const char *)"",
                streak_color ? " " : "", stonebuf);
    else if (streak_color)
        pline("You see %s streaks on the %s.", streak_color, stonebuf);
    else
        plines(scritch);
    return;
}


int dorub (void) {
    struct obj *obj = getobj(cuddly, "rub");

    if (obj && obj->oclass == GEM_CLASS) {
        if (is_graystone(obj)) {
            use_stone(obj);
            return 1;
        } else {
            pline("Sorry, I don't know how to use that.");
            return 0;
        }
    }

    if (!obj || !wield_tool(obj, "rub")) return 0;

    /* now uwep is obj */
    if (uwep->otyp == MAGIC_LAMP) {
        if (uwep->spe > 0 && !rn2(3)) {
            check_unpaid_usage(uwep, true);         /* unusual item use */
            djinni_from_bottle(uwep);
            makeknown(MAGIC_LAMP);
            uwep->otyp = OIL_LAMP;
            uwep->spe = 0; /* for safety */
            uwep->age = rn1(500,1000);
            if (uwep->lamplit) begin_burn(uwep, true);
        } else if (rn2(2) && !Blind) {
            You("see a puff of smoke.");
        } else {
            message_const(MSG_NOTHING_HAPPENS);
        }
    } else if (obj->otyp == BRASS_LANTERN) {
        /* message from Adventure */
        pline("Rubbing the electric lamp is not particularly rewarding.");
        pline("Anyway, nothing exciting happens.");
    } else {
        message_const(MSG_NOTHING_HAPPENS);
    }
    return 1;
}

int dojump (void) {
    /* Physical jump */
    return jump(0);
}

/* 0=Physical, otherwise skill level */
int jump ( int magic ) {
    coord cc;

    if (!magic && (nolimbs(youmonst.data) || slithy(youmonst.data))) {
        /* normally (nolimbs || slithy) implies !Jumping,
           but that isn't necessarily the case for knights */
        You_cant("jump; you have no legs!");
        return 0;
    } else if (!magic && !Jumping) {
        You_cant("jump very far.");
        return 0;
    } else if (u.uswallow) {
        if (magic) {
            You("bounce around a little.");
            return 1;
        }
        pline("You've got to be kidding!");
        return 0;
    } else if (u.uinwater) {
        if (magic) {
            You("swish around a little.");
            return 1;
        }
        pline("This calls for swimming, not jumping!");
        return 0;
    } else if (u.ustuck) {
        char name[BUFSZ];
        mon_nam(name, BUFSZ, u.ustuck);
        if (u.ustuck->mtame && !Conflict && !u.ustuck->mconf) {
            You("pull free from %s.", name);
            u.ustuck = 0;
            return 1;
        }
        if (magic) {
            You("writhe a little in the grasp of %s!", name);
            return 1;
        }
        You("cannot escape from %s!", name);
        return 0;
    } else if (Levitation || Is_airlevel(&u.uz) || Is_waterlevel(&u.uz)) {
        if (magic) {
            You("flail around a little.");
            return 1;
        }
        You("don't have enough traction to jump.");
        return 0;
    } else if (!magic && near_capacity() > UNENCUMBERED) {
        You("are carrying too much to jump!");
        return 0;
    } else if (!magic && (u.uhunger <= 100 || ACURR(A_STR) < 6)) {
        You("lack the strength to jump!");
        return 0;
    } else if (Wounded_legs()) {
        long wl = (Wounded_legs() & BOTH_SIDES);
        const char *bp = body_part(LEG);

        if (wl == BOTH_SIDES) bp = makeplural(bp);
        if (u.usteed) {
            char name[BUFSZ];
            Monnam(name, BUFSZ, u.usteed);
            pline("%s is in no shape for jumping.", name);
        } else {
            Your("%s%s %s in no shape for jumping.",
                    (wl == LEFT_SIDE) ? "left " :
                    (wl == RIGHT_SIDE) ? "right " : "",
                    bp, (wl == BOTH_SIDES) ? "are" : "is");
        }
        return 0;
    } else if (u.usteed && u.utrap) {
        char name[BUFSZ];
        Monnam(name, BUFSZ, u.usteed);
        pline("%s is stuck in a trap.", name);
        return (0);
    }

    pline("Where do you want to jump?");
    cc.x = u.ux;
    cc.y = u.uy;
    if (getpos(&cc, true, "the desired position") < 0)
        return 0;       /* user pressed ESC */
    if (!magic && !(HJumping & ~INTRINSIC) && !EJumping &&
            distu(cc.x, cc.y) != 5) {
        /* The Knight jumping restriction still applies when riding a
         * horse.  After all, what shape is the knight piece in chess?
         */
        pline("Illegal move!");
        return 0;
    } else if (distu(cc.x, cc.y) > (magic ? 6+magic*3 : 9)) {
        pline("Too far!");
        return 0;
    } else if (!cansee(cc.x, cc.y)) {
        You("cannot see where to land!");
        return 0;
    } else if (!isok(cc.x, cc.y)) {
        You("cannot jump there!");
        return 0;
    } else {
        coord uc;
        int range, temp;

        if(u.utrap)
            switch(u.utraptype) {
                case TT_BEARTRAP:
                    {
                        long side = rn2(3) ? LEFT_SIDE : RIGHT_SIDE;
                        You("rip yourself free of the bear trap!  Ouch!");
                        losehp(rnd(10), killed_by_const(KM_JUMP_BEAR_TRAP));
                        set_wounded_legs(side, rn1(1000,500));
                        break;
                    }
                case TT_PIT:
                    You("leap from the pit!");
                    break;
                case TT_WEB:
                    You("tear the web apart as you pull yourself free!");
                    deltrap(t_at(u.ux,u.uy));
                    break;
                case TT_LAVA:
                    You("pull yourself above the lava!");
                    u.utrap = 0;
                    return 1;
                case TT_INFLOOR:
                    You("strain your %s, but you're still stuck in the floor.",
                            makeplural(body_part(LEG)));
                    set_wounded_legs(LEFT_SIDE, rn1(10, 11));
                    set_wounded_legs(RIGHT_SIDE, rn1(10, 11));
                    return 1;
            }

        /*
         * Check the path from uc to cc, calling hurtle_step at each
         * location.  The final position actually reached will be
         * in cc.
         */
        uc.x = u.ux;
        uc.y = u.uy;
        /* calculate max(abs(dx), abs(dy)) as the range */
        range = cc.x - uc.x;
        if (range < 0) range = -range;
        temp = cc.y - uc.y;
        if (temp < 0) temp = -temp;
        if (range < temp)
            range = temp;
        walk_path(&uc, &cc, hurtle_step, (void *)&range);

        /* A little Sokoban guilt... */
        if (In_sokoban(&u.uz))
            change_luck(-1);

        teleds(cc.x, cc.y, true);
        nomul(-1);
        nomovemsg = "";
        morehungry(rnd(25));
        return 1;
    }
}

bool tinnable(struct obj *corpse) {
    if (corpse->oeaten) return 0;
    if (!mons[corpse->corpsenm].cnutrit) return 0;
    return 1;
}

static void use_tinning_kit (struct obj *obj) {
    struct obj *corpse, *can;

    /* This takes only 1 move.  If this is to be changed to take many
     * moves, we've got to deal with decaying corpses...
     */
    if (obj->spe <= 0) {
        You("seem to be out of tins.");
        return;
    }
    if (!(corpse = floorfood("tin", 2))) return;
    if (corpse->oeaten) {
        You("cannot tin %s which is partly eaten.",something);
        return;
    }
    if (touch_petrifies(&mons[corpse->corpsenm])
            && !Stone_resistance() && !uarmg) {
        char kbuf[BUFSZ];

        if (poly_when_stoned(youmonst.data))
            You("tin %s without wearing gloves.",
                    an(mons[corpse->corpsenm].mname));
        else {
            pline("Tinning %s without wearing gloves is a fatal mistake...",
                    an(mons[corpse->corpsenm].mname));
            sprintf(kbuf, "trying to tin %s without gloves",
                    an(mons[corpse->corpsenm].mname));
        }
        instapetrify(kbuf);
    }
    if (is_rider(&mons[corpse->corpsenm])) {
        (void) revive_corpse(corpse);
        verbalize("Yes...  But War does not preserve its enemies...");
        return;
    }
    if (mons[corpse->corpsenm].cnutrit == 0) {
        pline("That's too insubstantial to tin.");
        return;
    }
    consume_obj_charge(obj, true);

    if ((can = mksobj(TIN, false, false)) != 0) {
        static const char you_buy_it[] = "You tin it, you bought it!";

        can->corpsenm = corpse->corpsenm;
        can->cursed = obj->cursed;
        can->blessed = obj->blessed;
        can->owt = weight(can);
        can->known = 1;
        can->spe = -1;  /* Mark tinned tins. No spinach allowed... */
        if (carried(corpse)) {
            if (corpse->unpaid)
                verbalize(you_buy_it);
            useup(corpse);
        } else {
            if (costly_spot(corpse->ox, corpse->oy) && !corpse->no_charge)
                verbalize(you_buy_it);
            useupf(corpse, 1L);
        }
        can = hold_another_object(can, "You make, but cannot pick up, %s.",
                doname(can), (const char *)0);
    } else impossible("Tinning failed.");
}

void use_unicorn_horn (struct obj *obj) {
    int idx, val, val_limit,
        trouble_count, unfixable_trbl, did_prop, did_attr;
    int trouble_list[PROP_COUNT + ATTR_COUNT];

    if (obj && obj->cursed) {
        long lcount = (long) rnd(100);

        switch (rn2(6)) {
            case 0: make_sick(Sick ? Sick/3L + 1L : (long)rn1(ACURR(A_CON),20),
                            xname(obj), true, SICK_NONVOMITABLE);
                    break;
            case 1: make_blinded(Blinded + lcount, true);
                    break;
            case 2: if (!Confusion())
                        You("suddenly feel %s.",
                                Hallucination() ? "trippy" : "confused");
                    make_confused(get_HConfusion() + lcount, true);
                    break;
            case 3: make_stunned(get_HStun() + lcount, true);
                    break;
            case 4: (void) adjattrib(rn2(A_MAX), -1, false);
                    break;
            case 5: (void) make_hallucinated(u.uprops[HALLUC].intrinsic + lcount, true, 0L);
                    break;
        }
        return;
    }

    /*
     * Entries in the trouble list use a very simple encoding scheme.
     */

    trouble_count = unfixable_trbl = did_prop = did_attr = 0;

    /* collect property troubles */
    if (Sick) trouble_list[trouble_count++] = prop2trbl(SICK);
    if (Blinded > (long)u.ucreamed) trouble_list[trouble_count++] = prop2trbl(BLINDED);
    if (u.uprops[HALLUC].intrinsic) trouble_list[trouble_count++] = prop2trbl(HALLUC);
    if (Vomiting) trouble_list[trouble_count++] = prop2trbl(VOMITING);
    if (get_HConfusion()) trouble_list[trouble_count++] = prop2trbl(CONFUSION);
    if (get_HStun()) trouble_list[trouble_count++] = prop2trbl(STUNNED);

    unfixable_trbl = unfixable_trouble_count(true);

    /* collect attribute troubles */
    for (idx = 0; idx < A_MAX; idx++) {
        val_limit = AMAX(idx);
        /* don't recover strength lost from hunger */
        if (idx == A_STR && u.uhs >= WEAK) val_limit--;
        /* don't recover more than 3 points worth of any attribute */
        if (val_limit > ABASE(idx) + 3) val_limit = ABASE(idx) + 3;

        for (val = ABASE(idx); val < val_limit; val++)
            trouble_list[trouble_count++] = idx;
        /* keep track of unfixed trouble, for message adjustment below */
        unfixable_trbl += (AMAX(idx) - val_limit);
    }

    if (trouble_count == 0) {
        message_const(MSG_NOTHING_HAPPENS);
        return;
    } else if (trouble_count > 1) {         /* shuffle */
        int i, j, k;

        for (i = trouble_count - 1; i > 0; i--)
            if ((j = rn2(i + 1)) != i) {
                k = trouble_list[j];
                trouble_list[j] = trouble_list[i];
                trouble_list[i] = k;
            }
    }

    /*
     *              Chances for number of troubles to be fixed
     *               0      1      2      3      4      5      6      7
     *   blessed:  22.7%  22.7%  19.5%  15.4%  10.7%   5.7%   2.6%   0.8%
     *  uncursed:  35.4%  35.4%  22.9%   6.3%    0      0      0      0
     */
    val_limit = rn2( d(2, (obj && obj->blessed) ? 4 : 2) );
    if (val_limit > trouble_count) val_limit = trouble_count;

    /* fix [some of] the troubles */
    for (val = 0; val < val_limit; val++) {
        idx = trouble_list[val];

        switch (idx) {
            case prop2trbl(SICK):
                make_sick(0L, (char *) 0, true, SICK_ALL);
                did_prop++;
                break;
            case prop2trbl(BLINDED):
                make_blinded((long)u.ucreamed, true);
                did_prop++;
                break;
            case prop2trbl(HALLUC):
                (void) make_hallucinated(0L, true, 0L);
                did_prop++;
                break;
            case prop2trbl(VOMITING):
                make_vomiting(0L, true);
                did_prop++;
                break;
            case prop2trbl(CONFUSION):
                make_confused(0L, true);
                did_prop++;
                break;
            case prop2trbl(STUNNED):
                make_stunned(0L, true);
                did_prop++;
                break;
            default:
                if (idx >= 0 && idx < A_MAX) {
                    ABASE(idx) += 1;
                    did_attr++;
                } else
                    panic("use_unicorn_horn: bad trouble? (%d)", idx);
                break;
        }
    }

    if (did_attr)
        pline("This makes you feel %s!",
                (did_prop + did_attr) == (trouble_count + unfixable_trbl) ?
                "great" : "better");
    else if (!did_prop)
        pline("Nothing seems to happen.");
}

static bool figurine_location_checks(struct obj *obj, coord *cc, bool quietly) {
    signed char x,y;

    if (carried(obj) && u.uswallow) {
        if (!quietly)
            You("don't have enough room in here.");
        return false;
    }
    x = cc->x; y = cc->y;
    if (!isok(x,y)) {
        if (!quietly)
            You("cannot put the figurine there.");
        return false;
    }
    if (IS_ROCK(levl[x][y].typ) &&
            !(passes_walls(&mons[obj->corpsenm]) && may_passwall(x,y))) {
        if (!quietly)
            You("cannot place a figurine in %s!",
                    IS_TREE(levl[x][y].typ) ? "a tree" : "solid rock");
        return false;
    }
    if (sobj_at(BOULDER,x,y) && !passes_walls(&mons[obj->corpsenm])
            && !throws_rocks(&mons[obj->corpsenm])) {
        if (!quietly)
            You("cannot fit the figurine on the boulder.");
        return false;
    }
    return true;
}


/*
 * Timer callback routine: turn figurine into monster
 */
void fig_transform (void *arg, long timeout) {
    struct obj *figurine = (struct obj *)arg;
    struct monst *mtmp;
    coord cc;
    bool cansee_spot, silent, okay_spot;
    bool redraw = false;
    char monnambuf[BUFSZ], carriedby[BUFSZ];

    if (!figurine) {
        return;
    }
    silent = (timeout != monstermoves); /* happened while away */
    okay_spot = get_obj_location(figurine, &cc.x, &cc.y, 0);
    if (figurine->where == OBJ_INVENT ||
            figurine->where == OBJ_MINVENT)
        okay_spot = enexto(&cc, cc.x, cc.y,
                &mons[figurine->corpsenm]);
    if (!okay_spot || !figurine_location_checks(figurine,&cc, true)) {
        /* reset the timer to try again later */
        start_timer((long)rnd(5000), TIMER_OBJECT, FIG_TRANSFORM, (void *)figurine);
        return;
    }

    cansee_spot = cansee(cc.x, cc.y);
    mtmp = make_familiar(figurine, cc.x, cc.y, true);
    if (mtmp) {
        char name[BUFSZ];
        m_monnam(name, BUFSZ, mtmp);
        sprintf(monnambuf, "%s",an(name));
        switch (figurine->where) {
            case OBJ_INVENT:
                if (Blind)
                    You_feel("%s %s from your pack!", something,
                            locomotion(mtmp->data,"drop"));
                else
                    You("see %s %s out of your pack!",
                            monnambuf,
                            locomotion(mtmp->data,"drop"));
                break;

            case OBJ_FLOOR:
                if (cansee_spot && !silent) {
                    You("suddenly see a figurine transform into %s!",
                            monnambuf);
                    redraw = true;  /* update figurine's map location */
                }
                break;

            case OBJ_MINVENT:
                if (cansee_spot && !silent) {
                    struct monst *mon;
                    mon = figurine->ocarry;
                    /* figurine carring monster might be invisible */
                    if (canseemon(figurine->ocarry)) {
                        char name[BUFSZ];
                        a_monnam(name, BUFSZ, mon);
                        sprintf(carriedby, "%s%s pack", name, possessive_suffix(name));
                    }
                    else if (is_pool(mon->mx, mon->my))
                        strcpy(carriedby, "empty water");
                    else
                        strcpy(carriedby, "thin air");
                    You("see %s %s out of %s!", monnambuf, locomotion(mtmp->data, "drop"), carriedby);
                }
                break;

            default:
                impossible("figurine came to life where? (%d)",
                        (int)figurine->where);
                break;
        }
    }
    /* free figurine now */
    obj_extract_self(figurine);
    obfree(figurine, (struct obj *)0);
    if (redraw) newsym(cc.x, cc.y);
}

static void use_figurine (struct obj **optr) {
    struct obj *obj = *optr;
    signed char x, y;
    coord cc;

    if (u.uswallow) {
        /* can't activate a figurine while swallowed */
        if (!figurine_location_checks(obj, (coord *)0, false))
            return;
    }
    if(!getdir((char *)0)) {
        flags.move = multi = 0;
        return;
    }
    x = u.ux + u.dx; y = u.uy + u.dy;
    cc.x = x; cc.y = y;
    /* Passing false arg here will result in messages displayed */
    if (!figurine_location_checks(obj, &cc, false)) return;
    You("%s and it transforms.",
            (u.dx||u.dy) ? "set the figurine beside you" :
            (Is_airlevel(&u.uz) || Is_waterlevel(&u.uz) ||
             is_pool(cc.x, cc.y)) ?
            "release the figurine" :
            (u.dz < 0 ?
             "toss the figurine into the air" :
             "set the figurine on the ground"));
    (void) make_familiar(obj, cc.x, cc.y, false);
    (void) stop_timer(FIG_TRANSFORM, (void *)obj);
    useup(obj);
    *optr = 0;
}


static void use_grease (struct obj *obj) {
    struct obj *otmp;
    char buf[BUFSZ];

    if (Glib) {
        char slip_clause[BUFSZ];
        Tobjnam(slip_clause, BUFSZ, obj, "slip");
        pline("%s from your %s.", slip_clause, makeplural(body_part(FINGER)));
        dropx(obj);
        return;
    }

    if (obj->spe > 0) {
        if ((obj->cursed || Fumbling()) && !rn2(2)) {
            consume_obj_charge(obj, true);

            char slip_clause[BUFSZ];
            Tobjnam(slip_clause, BUFSZ, obj, "slip");
            pline("%s from your %s.", slip_clause, makeplural(body_part(FINGER)));
            dropx(obj);
            return;
        }
        otmp = getobj(lubricables, "grease");
        if (!otmp) return;
        if ((otmp->owornmask & WORN_ARMOR) && uarmc) {
            message_object2(MSG_YOU_MUST_REMOVE_O_TO_GREASE_O, uarmc, otmp);
            return;
        }
        if ((otmp->owornmask & WORN_SHIRT) && (uarmc || uarm)) {
            message_object3(MSG_YOU_MUST_REMOVE_O_AND_O_TO_GREASE_O, uarmc, uarm, otmp);
            return;
        }
        consume_obj_charge(obj, true);

        if (otmp != &zeroobj) {
            You("cover %s with a thick layer of grease.",
                    yname(otmp));
            otmp->greased = 1;
            if (obj->cursed && !nohands(youmonst.data)) {
                incr_itimeout(&Glib, rnd(15));
                pline("Some of the grease gets all over your %s.",
                        makeplural(body_part(HAND)));
            }
        } else {
            Glib += rnd(15);
            You("coat your %s with grease.",
                    makeplural(body_part(FINGER)));
        }
    } else {
        if (obj->known) {
            char are_clause[BUFSZ];
            Tobjnam(are_clause, BUFSZ, obj, "are");
            pline("%s empty.", are_clause);
        } else {
            char seem_clause[BUFSZ];
            Tobjnam(seem_clause, BUFSZ, obj, "seem");
            pline("%s to be empty.", seem_clause);
        }
    }
}

void reset_trapset (void) {
    trapinfo.tobj = 0;
    trapinfo.force_bungle = 0;
}

static int set_trap (void) {
    struct obj *otmp = trapinfo.tobj;
    struct trap *ttmp;
    int ttyp;

    if (!otmp || !carried(otmp) ||
            u.ux != trapinfo.tx || u.uy != trapinfo.ty) {
        /* ?? */
        reset_trapset();
        return 0;
    }

    if (--trapinfo.time_needed > 0) return 1;       /* still busy */

    ttyp = (otmp->otyp == LAND_MINE) ? LANDMINE : BEAR_TRAP;
    ttmp = maketrap(u.ux, u.uy, ttyp);
    if (ttmp) {
        ttmp->tseen = 1;
        ttmp->madeby_u = 1;
        newsym(u.ux, u.uy); /* if our hero happens to be invisible */
        if (*in_rooms(u.ux,u.uy,SHOPBASE)) {
            add_damage(u.ux, u.uy, 0L);             /* schedule removal */
        }
        if (!trapinfo.force_bungle)
            You("finish arming %s.",
                    the(defsyms[trap_to_defsym(what_trap(ttyp))].explanation));
        if (((otmp->cursed || Fumbling()) && (rnl(10) > 5)) || trapinfo.force_bungle)
            dotrap(ttmp,
                    (unsigned)(trapinfo.force_bungle ? FORCEBUNGLE : 0));
    } else {
        /* this shouldn't happen */
        Your("trap setting attempt fails.");
    }
    useup(otmp);
    reset_trapset();
    return 0;
}


/* Place a landmine/bear trap.  Helge Hafting */
static void use_trap (struct obj *otmp) {
    int ttyp, tmp;
    const char *what = (char *)0;
    char buf[BUFSZ];
    const char *occutext = "setting the trap";

    if (nohands(youmonst.data))
        what = "without hands";
    else if (Stunned())
        what = "while stunned";
    else if (u.uswallow)
        what = is_animal(u.ustuck->data) ? "while swallowed" :
            "while engulfed";
    else if (Underwater)
        what = "underwater";
    else if (Levitation)
        what = "while levitating";
    else if (is_pool(u.ux, u.uy))
        what = "in water";
    else if (is_lava(u.ux, u.uy))
        what = "in lava";
    else if (On_stairs(u.ux, u.uy))
        what = (u.ux == xdnladder || u.ux == xupladder) ?
            "on the ladder" : "on the stairs";
    else if (IS_FURNITURE(levl[u.ux][u.uy].typ) ||
            IS_ROCK(levl[u.ux][u.uy].typ) ||
            closed_door(u.ux, u.uy) || t_at(u.ux, u.uy))
        what = "here";
    if (what) {
        You_cant("set a trap %s!",what);
        reset_trapset();
        return;
    }
    ttyp = (otmp->otyp == LAND_MINE) ? LANDMINE : BEAR_TRAP;
    if (otmp == trapinfo.tobj &&
            u.ux == trapinfo.tx && u.uy == trapinfo.ty) {
        You("resume setting %s %s.",
                shk_your(buf, otmp),
                defsyms[trap_to_defsym(what_trap(ttyp))].explanation);
        set_occupation(set_trap, occutext);
        return;
    }
    trapinfo.tobj = otmp;
    trapinfo.tx = u.ux,  trapinfo.ty = u.uy;
    tmp = ACURR(A_DEX);
    trapinfo.time_needed = (tmp > 17) ? 2 : (tmp > 12) ? 3 :
        (tmp > 7) ? 4 : 5;
    if (Blind) trapinfo.time_needed *= 2;
    tmp = ACURR(A_STR);
    if (ttyp == BEAR_TRAP && tmp < 18)
        trapinfo.time_needed += (tmp > 12) ? 1 : (tmp > 7) ? 2 : 4;
    /*[fumbling and/or confusion and/or cursed object check(s)
      should be incorporated here instead of in set_trap]*/
    if (u.usteed && P_SKILL(P_RIDING) < P_BASIC) {
        bool chance;

        if (Fumbling() || otmp->cursed) chance = (rnl(10) > 3);
        else  chance = (rnl(10) > 5);

        char name[BUFSZ];
        mon_nam(name, BUFSZ, u.usteed);
        You("aren't very skilled at reaching from %s.", name);
        sprintf(buf, "Continue your attempt to set %s?",
                the(defsyms[trap_to_defsym(what_trap(ttyp))].explanation));
        if(yn(buf) == 'y') {
            if (chance) {
                switch(ttyp) {
                    case LANDMINE:      /* set it off */
                        trapinfo.time_needed = 0;
                        trapinfo.force_bungle = true;
                        break;
                    case BEAR_TRAP:     /* drop it without arming it */
                        reset_trapset();
                        You("drop %s!",
                                the(defsyms[trap_to_defsym(what_trap(ttyp))].explanation));
                        dropx(otmp);
                        return;
                }
            }
        } else {
            reset_trapset();
            return;
        }
    }
    You("begin setting %s %s.", shk_your(buf, otmp),
            defsyms[trap_to_defsym(what_trap(ttyp))].explanation);
    set_occupation(set_trap, occutext);
}

static int use_whip (struct obj *obj) {
    char buf[BUFSZ];
    struct monst *mtmp;
    struct obj *otmp;
    int rx, ry, proficient, res = 0;
    const char *msg_slipsfree = "The bullwhip slips free.";
    const char *msg_snap = "Snap!";

    if (obj != uwep) {
        if (!wield_tool(obj, "lash")) return 0;
        else res = 1;
    }
    if (!getdir((char *)0)) return res;

    if (Stunned() || (Confusion() && !rn2(5))) confdir();
    rx = u.ux + u.dx;
    ry = u.uy + u.dy;
    mtmp = m_at(rx, ry);

    /* fake some proficiency checks */
    proficient = 0;
    if (Role_if(PM_ARCHEOLOGIST)) ++proficient;
    if (ACURR(A_DEX) < 6) proficient--;
    else if (ACURR(A_DEX) >= 14) proficient += (ACURR(A_DEX) - 14);
    if (Fumbling()) --proficient;
    if (proficient > 3) proficient = 3;
    if (proficient < 0) proficient = 0;

    if (u.uswallow && attack(u.ustuck)) {
        There("is not enough room to flick your bullwhip.");

    } else if (Underwater) {
        There("is too much resistance to flick your bullwhip.");

    } else if (u.dz < 0) {
        You("flick a bug off of the %s.",ceiling(u.ux,u.uy));

    } else if ((!u.dx && !u.dy) || (u.dz > 0)) {
        int dam;

        /* Sometimes you hit your steed by mistake */
        if (u.usteed && !rn2(proficient + 2)) {
            char name[BUFSZ];
            mon_nam(name, BUFSZ, u.usteed);
            You("whip %s!", name);
            kick_steed();
            return 1;
        }
        if (Levitation || u.usteed) {
            /* Have a shot at snaring something on the floor */
            otmp = level.objects[u.ux][u.uy];
            if (otmp && otmp->otyp == CORPSE && otmp->corpsenm == PM_HORSE) {
                pline("Why beat a dead horse?");
                return 1;
            }
            if (otmp && proficient) {
                You("wrap your bullwhip around %s on the %s.",
                        an(singular(otmp, xname)), surface(u.ux, u.uy));
                if (rnl(6) || pickup_object(otmp, 1L, true) < 1)
                    plines(msg_slipsfree);
                return 1;
            }
        }
        dam = rnd(2) + dbon() + obj->spe;
        if (dam <= 0) dam = 1;
        You("hit your %s with your bullwhip.", body_part(FOOT));
        losehp(dam, killed_by_const(KM_HIT_SELF_BULLWHIP));
        return 1;

    } else if ((Fumbling() || Glib) && !rn2(5)) {
        pline_The("bullwhip slips out of your %s.", body_part(HAND));
        dropx(obj);

    } else if (u.utrap && u.utraptype == TT_PIT) {
        /*
         *     Assumptions:
         *
         *      if you're in a pit
         *              - you are attempting to get out of the pit
         *              - or, if you are applying it towards a small
         *                monster then it is assumed that you are
         *                trying to hit it.
         *      else if the monster is wielding a weapon
         *              - you are attempting to disarm a monster
         *      else
         *              - you are attempting to hit the monster
         *
         *      if you're confused (and thus off the mark)
         *              - you only end up hitting.
         *
         */
        const char *wrapped_what = (char *)0;

        if (mtmp) {
            if (bigmonst(mtmp->data)) {
                char name[BUFSZ];
                mon_nam(name, BUFSZ, mtmp);
                wrapped_what = strcpy(buf, name);
            } else if (proficient) {
                if (attack(mtmp)) return 1;
                else plines(msg_snap);
            }
        }
        if (!wrapped_what) {
            if (IS_FURNITURE(levl[rx][ry].typ))
                wrapped_what = something;
            else if (sobj_at(BOULDER, rx, ry))
                wrapped_what = "a boulder";
        }
        if (wrapped_what) {
            coord cc;

            cc.x = rx; cc.y = ry;
            You("wrap your bullwhip around %s.", wrapped_what);
            if (proficient && rn2(proficient + 2)) {
                if (!mtmp || enexto(&cc, rx, ry, youmonst.data)) {
                    You("yank yourself out of the pit!");
                    teleds(cc.x, cc.y, true);
                    u.utrap = 0;
                    vision_full_recalc = 1;
                }
            } else {
                plines(msg_slipsfree);
            }
            if (mtmp) wakeup(mtmp);
        } else plines(msg_snap);

    } else if (mtmp) {
        if (!canspotmon(mtmp) &&
                !glyph_is_invisible(levl[rx][ry].glyph)) {
            pline("A monster is there that you couldn't see.");
            map_invisible(rx, ry);
        }
        otmp = MON_WEP(mtmp);   /* can be null */
        if (otmp) {
            char onambuf[BUFSZ];
            const char *mon_hand;
            bool gotit = proficient && (!Fumbling() || !rn2(10));

            strcpy(onambuf, cxname(otmp));
            if (gotit) {
                mon_hand = mbodypart(mtmp, HAND);
                if (bimanual(otmp)) mon_hand = makeplural(mon_hand);
            } else
                mon_hand = 0;   /* lint suppression */

            char name[BUFSZ];
            mon_nam(name, BUFSZ, mtmp);
            You("wrap your bullwhip around %s%s %s.", name, possessive_suffix(name), onambuf);
            if (gotit && otmp->cursed) {
                pline("%s welded to %s %s%c",
                        (otmp->quan == 1L) ? "It is" : "They are",
                        mhis(mtmp), mon_hand,
                        !otmp->bknown ? '!' : '.');
                otmp->bknown = 1;
                gotit = false;  /* can't pull it free */
            }
            if (gotit) {
                obj_extract_self(otmp);
                possibly_unwield(mtmp, false);
                setmnotwielded(mtmp,otmp);

                char name[BUFSZ];
                mon_nam(name, BUFSZ, mtmp);
                switch (rn2(proficient + 1)) {
                    case 2:
                        /* to floor near you */
                        You("yank %s%s %s to the %s!", name, possessive_suffix(name),
                                onambuf, surface(u.ux, u.uy));
                        place_object(otmp, u.ux, u.uy);
                        stackobj(otmp);
                        break;
                    case 3:
                        /* right into your inventory */
                        You("snatch %s%s %s!", name, possessive_suffix(name), onambuf);
                        if (otmp->otyp == CORPSE &&
                                touch_petrifies(&mons[otmp->corpsenm]) &&
                                !uarmg && !Stone_resistance() &&
                                !(poly_when_stoned(youmonst.data) &&
                                    polymon(PM_STONE_GOLEM))) {
                            char kbuf[BUFSZ];

                            sprintf(kbuf, "%s corpse",
                                    an(mons[otmp->corpsenm].mname));
                            pline("Snatching %s is a fatal mistake.", kbuf);
                            instapetrify(kbuf);
                        }
                        otmp = hold_another_object(otmp, "You drop %s!",
                                doname(otmp), (const char *)0);
                        break;
                    default:
                        /* to floor beneath mon */
                        You("yank %s from %s%s %s!", the(onambuf),
                                name, possessive_suffix(name), mon_hand);
                        obj_no_longer_held(otmp);
                        place_object(otmp, mtmp->mx, mtmp->my);
                        stackobj(otmp);
                        break;
                }
            } else {
                plines(msg_slipsfree);
            }
            wakeup(mtmp);
        } else {
            if (mtmp->m_ap_type && !Protection_from_shape_changers && !sensemon(mtmp)) {
                stumble_onto_mimic(mtmp);
            } else {
                char name[BUFSZ];
                mon_nam(name, BUFSZ, mtmp);
                You("flick your bullwhip towards %s.", name);
            }
            if (proficient) {
                if (attack(mtmp)) return 1;
                else plines(msg_snap);
            }
        }

    } else if (Is_airlevel(&u.uz) || Is_waterlevel(&u.uz)) {
        /* it must be air -- water checked above */
        You("snap your whip through thin air.");

    } else {
        plines(msg_snap);

    }
    return 1;
}

/* Distance attacks by pole-weapons */
static int use_pole (struct obj *obj) {
    int res = 0, typ, max_range = 4, min_range = 4;
    coord cc;
    struct monst *mtmp;


    /* Are you allowed to use the pole? */
    if (u.uswallow) {
        message_const(MSG_NOT_ENOUGH_ROOM_TO_USE);
        return 0;
    }
    if (obj != uwep) {
        if (!wield_tool(obj, "swing")) return(0);
        else res = 1;
    }
    /* assert(obj == uwep); */

    /* Prompt for a location */
    // plines(where_to_hit);
    cc.x = u.ux;
    cc.y = u.uy;
    if (getpos(&cc, true, "the spot to hit") < 0)
        return 0;   /* user pressed ESC */

    /* Calculate range */
    typ = uwep_skill_type();
    if (typ == P_NONE || P_SKILL(typ) <= P_BASIC) max_range = 4;
    else if (P_SKILL(typ) == P_SKILLED) max_range = 5;
    else max_range = 8;
    if (distu(cc.x, cc.y) > max_range) {
        pline("Too far!");
        return (res);
    } else if (distu(cc.x, cc.y) < min_range) {
        pline("Too close!");
        return (res);
    } else if (!cansee(cc.x, cc.y) &&
            ((mtmp = m_at(cc.x, cc.y)) == (struct monst *)0 ||
             !canseemon(mtmp)))
    {
        message_const(MSG_NO_HIT_IF_CANNOT_SEE_SPOT);
        return res;
    } else if (!couldsee(cc.x, cc.y)) { /* Eyes of the Overworld */
        message_const(MSG_YOU_CANNOT_REACH_SPOT_FROM_HERE);
        return res;
    }

    /* Attack the monster there */
    if ((mtmp = m_at(cc.x, cc.y)) != (struct monst *)0) {
        int oldhp = mtmp->mhp;

        bhitpos = cc;
        check_caitiff(mtmp);
        (void) thitmonst(mtmp, uwep);
        /* check the monster's HP because thitmonst() doesn't return
         * an indication of whether it hit.  Not perfect (what if it's a
         * non-silver weapon on a shade?)
         */
        if (mtmp->mhp < oldhp)
            u.uconduct.weaphit++;
    } else
        /* Now you know that nothing is there... */
        message_const(MSG_NOTHING_HAPPENS);
    return (1);
}

static int use_cream_pie (struct obj *obj) {
    bool wasblind = Blind;
    bool wascreamed = u.ucreamed;
    bool several = false;

    if (obj->quan > 1L) {
        several = true;
        obj = splitobj(obj, 1L);
    }
    if (Hallucination())
        You("give yourself a facial.");
    else
        pline("You immerse your %s in %s%s.", body_part(FACE),
                several ? "one of " : "",
                several ? makeplural(the(xname(obj))) : the(xname(obj)));
    if(can_blnd((struct monst*)0, &youmonst, AT_WEAP, obj)) {
        int blindinc = rnd(25);
        u.ucreamed += blindinc;
        make_blinded(Blinded + (long)blindinc, false);
        if (!Blind || (Blind && wasblind))
            pline("There's %ssticky goop all over your %s.",
                    wascreamed ? "more " : "",
                    body_part(FACE));
        else /* Blind  && !wasblind */
            You_cant("see through all the sticky goop on your %s.",
                    body_part(FACE));
    }
    if (obj->unpaid) {
        verbalize("You used it, you bought it!");
        bill_dummy_object(obj);
    }
    obj_extract_self(obj);
    delobj(obj);
    return(0);
}

static int use_grapple (struct obj *obj) {
    int res = 0, typ, max_range = 4, tohit;
    coord cc;
    struct monst *mtmp;
    struct obj *otmp;

    /* Are you allowed to use the hook? */
    if (u.uswallow) {
        message_const(MSG_NOT_ENOUGH_ROOM_TO_USE);
        return 0;
    }
    if (obj != uwep) {
        if (!wield_tool(obj, "cast")) return(0);
        else res = 1;
    }
    /* assert(obj == uwep); */

    /* Prompt for a location */
    // plines(where_to_hit);
    cc.x = u.ux;
    cc.y = u.uy;
    if (getpos(&cc, true, "the spot to hit") < 0)
        return 0;   /* user pressed ESC */

    /* Calculate range */
    typ = uwep_skill_type();
    if (typ == P_NONE || P_SKILL(typ) <= P_BASIC) max_range = 4;
    else if (P_SKILL(typ) == P_SKILLED) max_range = 5;
    else max_range = 8;
    if (distu(cc.x, cc.y) > max_range) {
        pline("Too far!");
        return (res);
    } else if (!cansee(cc.x, cc.y)) {
        message_const(MSG_NO_HIT_IF_CANNOT_SEE_SPOT);
        return res;
    } else if (!couldsee(cc.x, cc.y)) { /* Eyes of the Overworld */
        message_const(MSG_YOU_CANNOT_REACH_SPOT_FROM_HERE);
        return res;
    }

    /* What do you want to hit? */
    tohit = rn2(5);
    if (typ != P_NONE && P_SKILL(typ) >= P_SKILLED) {
        // winid tmpwin = create_nhwindow(NHW_MENU);
        anything any;
        char buf[BUFSZ];
        menu_item *selected;

        any.a_void = 0;     /* set all bits to zero */
        any.a_int = 1;      /* use index+1 (cant use 0) as identifier */
        // start_menu(tmpwin);
        // any.a_int++;
        // sprintf(buf, "an object on the %s", surface(cc.x, cc.y));
        // add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE,
        //         buf, MENU_UNSELECTED);
        // any.a_int++;
        // add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE,
        //         "a monster", MENU_UNSELECTED);
        // any.a_int++;
        // sprintf(buf, "the %s", surface(cc.x, cc.y));
        // add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE,
        //         buf, MENU_UNSELECTED);
        // end_menu(tmpwin, "Aim for what?");
        // tohit = rn2(4);
        // if (select_menu(tmpwin, PICK_ONE, &selected) > 0 &&
        //         rn2(P_SKILL(typ) > P_SKILLED ? 20 : 2))
        //     tohit = selected[0].item.a_int - 1;
        // free((void *)selected);
        // destroy_nhwindow(tmpwin);
    }

    /* What did you hit? */
    switch (tohit) {
        case 0: /* Trap */
            /* FIXME -- untrap needs to deal with non-adjacent traps */
            break;
        case 1: /* Object */
            if ((otmp = level.objects[cc.x][cc.y]) != 0) {
                You("snag an object from the %s!", surface(cc.x, cc.y));
                (void) pickup_object(otmp, 1L, false);
                /* If pickup fails, leave it alone */
                newsym(cc.x, cc.y);
                return (1);
            }
            break;
        case 2: /* Monster */
            if ((mtmp = m_at(cc.x, cc.y)) == (struct monst *)0) break;
            if (verysmall(mtmp->data) && !rn2(4) && enexto(&cc, u.ux, u.uy, NULL)) {
                char name[BUFSZ];
                mon_nam(name, BUFSZ, mtmp);
                You("pull in %s!", name);
                mtmp->mundetected = 0;
                rloc_to(mtmp, cc.x, cc.y);
                return (1);
            } else if ((!bigmonst(mtmp->data) && !strongmonst(mtmp->data)) ||
                    rn2(4)) {
                (void) thitmonst(mtmp, uwep);
                return (1);
            }
            /* FALL THROUGH */
        case 3: /* Surface */
            if (IS_AIR(levl[cc.x][cc.y].typ) || is_pool(cc.x, cc.y))
                pline_The("hook slices through the %s.", surface(cc.x, cc.y));
            else {
                You("are yanked toward the %s!", surface(cc.x, cc.y));
                hurtle(sgn(cc.x-u.ux), sgn(cc.y-u.uy), 1, false);
                spoteffects(true);
            }
            return (1);
        default:        /* Yourself (oops!) */
            if (P_SKILL(typ) <= P_BASIC) {
                You("hook yourself!");
                losehp(rn1(10,10), killed_by_const(KM_GRAPPLING_HOOK_SELF));
                return (1);
            }
            break;
    }
    message_const(MSG_NOTHING_HAPPENS);
    return (1);
}

/* return 1 if the wand is broken, hence some time elapsed */
static int do_break_wand (struct obj *obj) {
    static const char nothing_else_happens[] = "But nothing else happens...";
    int i, x, y;
    struct monst *mon;
    int dmg, damage;
    bool affects_objects;
    bool shop_damage = false;
    int expltype = EXPL_MAGICAL;
    char confirm[QBUFSZ], the_wand[BUFSZ], buf[BUFSZ];

    strcpy(the_wand, yname(obj));
    sprintf(confirm, "Are you really sure you want to break %s?",
            safe_qbuf("", sizeof("Are you really sure you want to break ?"),
                the_wand, ysimple_name(obj), "the wand"));
    if (yn(confirm) == 'n' ) return 0;

    if (nohands(youmonst.data)) {
        You_cant("break %s without hands!", the_wand);
        return 0;
    } else if (ACURR(A_STR) < 10) {
        You("don't have the strength to break %s!", the_wand);
        return 0;
    }
    pline("Raising %s high above your %s, you break it in two!",
            the_wand, body_part(HEAD));

    /* [ALI] Do this first so that wand is removed from bill. Otherwise,
     * the freeinv() below also hides it from setpaid() which causes problems.
     */
    if (obj->unpaid) {
        check_unpaid(obj);              /* Extra charge for use */
        bill_dummy_object(obj);
    }

    current_wand = obj;         /* destroy_item might reset this */
    freeinv(obj);               /* hide it from destroy_item instead... */
    setnotworn(obj);            /* so we need to do this ourselves */

    if (obj->spe <= 0) {
        plines(nothing_else_happens);
        goto discard_broken_wand;
    }
    obj->ox = u.ux;
    obj->oy = u.uy;
    dmg = obj->spe * 4;
    affects_objects = false;

    switch (obj->otyp) {
        case WAN_WISHING:
        case WAN_NOTHING:
        case WAN_LOCKING:
        case WAN_PROBING:
        case WAN_ENLIGHTENMENT:
        case WAN_OPENING:
        case WAN_SECRET_DOOR_DETECTION:
            plines(nothing_else_happens);
            goto discard_broken_wand;
        case WAN_DEATH:
        case WAN_LIGHTNING:
            dmg *= 4;
            goto wanexpl;
        case WAN_FIRE:
            expltype = EXPL_FIERY;
        case WAN_COLD:
            if (expltype == EXPL_MAGICAL) expltype = EXPL_FROSTY;
            dmg *= 2;
        case WAN_MAGIC_MISSILE:
wanexpl:
            explode(u.ux, u.uy, (obj->otyp - WAN_MAGIC_MISSILE), dmg, WAND_CLASS, expltype);
            makeknown(obj->otyp);   /* explode described the effect */
            goto discard_broken_wand;
        case WAN_STRIKING:
            /* we want this before the explosion instead of at the very end */
            pline("A wall of force smashes down around you!");
            dmg = d(1 + obj->spe,6);        /* normally 2d12 */
        case WAN_CANCELLATION:
        case WAN_POLYMORPH:
        case WAN_TELEPORTATION:
        case WAN_UNDEAD_TURNING:
            affects_objects = true;
            break;
        default:
            break;
    }

    /* magical explosion and its visual effect occur before specific effects */
    explode(obj->ox, obj->oy, 0, rnd(dmg), WAND_CLASS, EXPL_MAGICAL);

    /* this makes it hit us last, so that we can see the action first */
    for (i = 0; i <= 8; i++) {
        bhitpos.x = x = obj->ox + xdir[i];
        bhitpos.y = y = obj->oy + ydir[i];
        if (!isok(x,y)) continue;

        if (obj->otyp == WAN_DIGGING) {
            if(dig_check(BY_OBJECT, false, x, y)) {
                if (IS_WALL(levl[x][y].typ) || IS_DOOR(levl[x][y].typ)) {
                    /* normally, pits and holes don't anger guards, but they
                     * do if it's a wall or door that's being dug */
                    watch_dig((struct monst *)0, x, y, true);
                    if (*in_rooms(x,y,SHOPBASE)) shop_damage = true;
                }
                digactualhole(x, y, BY_OBJECT,
                        (rn2(obj->spe) < 3 || !Can_dig_down(&u.uz)) ?
                        PIT : HOLE);
            }
            continue;
        } else if(obj->otyp == WAN_CREATE_MONSTER) {
            /* u.ux,u.uy creates it near you--x,y might create it in rock */
            (void) makemon((struct permonst *)0, u.ux, u.uy, NO_MM_FLAGS);
            continue;
        } else {
            if (x == u.ux && y == u.uy) {
                /* teleport objects first to avoid race with tele control and
                   autopickup.  Other wand/object effects handled after
                   possible wand damage is assessed */
                if (obj->otyp == WAN_TELEPORTATION &&
                        affects_objects && level.objects[x][y]) {
                    (void) bhitpile(obj, bhito, x, y);
                }
                damage = zapyourself(obj, false);
                if (damage) {
                    losehp(damage, killed_by_const(KM_KILLED_SELF_BREAK_WAND));
                }
            } else if ((mon = m_at(x, y)) != 0) {
                (void) bhitm(mon, obj);
            }
            if (affects_objects && level.objects[x][y]) {
                (void) bhitpile(obj, bhito, x, y);
            }
        }
    }

    /* Note: if player fell thru, this call is a no-op.
       Damage is handled in digactualhole in that case */
    if (shop_damage) pay_for_damage("dig into", false);

    if (obj->otyp == WAN_LIGHT)
        litroom(true, obj);     /* only needs to be done once */

discard_broken_wand:
    obj = current_wand;         /* [see dozap() and destroy_item()] */
    current_wand = 0;
    if (obj)
        delobj(obj);
    nomul(0);
    return 1;
}

static bool uhave_graystone(void) {
    struct obj *otmp;

    for(otmp = invent; otmp; otmp = otmp->nobj)
        if(is_graystone(otmp))
            return true;
    return false;
}

static void add_class (char *cl, char class) {
    char tmp[2];
    tmp[0] = class;
    tmp[1] = '\0';
    strcat(cl, tmp);
}

int doapply (void) {
    struct obj *obj;
    int res = 1;
    char class_list[MAXOCLASSES+2];

    if(check_capacity((char *)0)) return (0);

    if (carrying(POT_OIL) || uhave_graystone())
        strcpy(class_list, tools_too);
    else
        strcpy(class_list, tools);
    if (carrying(CREAM_PIE) || carrying(EUCALYPTUS_LEAF))
        add_class(class_list, FOOD_CLASS);

    obj = getobj(class_list, "use or apply");
    if(!obj) return 0;

    if (obj->oartifact && !touch_artifact(obj, &youmonst))
        return 1;   /* evading your grasp costs a turn; just be
                       grateful that you don't drop it as well */

    if (obj->oclass == WAND_CLASS)
        return do_break_wand(obj);

    switch(obj->otyp){
        case BLINDFOLD:
        case LENSES:
            if (obj == ublindf) {
                if (!cursed(obj)) Blindf_off(obj);
            } else if (!ublindf)
                Blindf_on(obj);
            else You("are already %s.",
                    ublindf->otyp == TOWEL ?     "covered by a towel" :
                    ublindf->otyp == BLINDFOLD ? "wearing a blindfold" :
                    "wearing lenses");
            break;
        case CREAM_PIE:
            res = use_cream_pie(obj);
            break;
        case BULLWHIP:
            res = use_whip(obj);
            break;
        case GRAPPLING_HOOK:
            res = use_grapple(obj);
            break;
        case LARGE_BOX:
        case CHEST:
        case ICE_BOX:
        case SACK:
        case BAG_OF_HOLDING:
        case OILSKIN_SACK:
            res = use_container(obj, 1);
            break;
        case BAG_OF_TRICKS:
            bagotricks(obj);
            break;
        case CAN_OF_GREASE:
            use_grease(obj);
            break;
        case LOCK_PICK:
        case CREDIT_CARD:
        case SKELETON_KEY:
            pick_lock(obj);
            break;
        case PICK_AXE:
        case DWARVISH_MATTOCK:
            res = use_pick_axe(obj);
            break;
        case TINNING_KIT:
            use_tinning_kit(obj);
            break;
        case LEASH:
            use_leash(obj);
            break;
        case SADDLE:
            res = use_saddle(obj);
            break;
        case MAGIC_WHISTLE:
            use_magic_whistle(obj);
            break;
        case TIN_WHISTLE:
            use_whistle(obj);
            break;
        case EUCALYPTUS_LEAF:
            /* MRKR: Every Australian knows that a gum leaf makes an */
            /*       excellent whistle, especially if your pet is a  */
            /*       tame kangaroo named Skippy.                     */
            if (obj->blessed) {
                use_magic_whistle(obj);
                /* sometimes the blessing will be worn off */
                if (!rn2(49)) {
                    if (!Blind) {
                        char buf[BUFSZ];

                        pline("%s %s %s.", Shk_Your(buf, obj),
                                aobjnam(obj, "glow"), hcolor("brown"));
                        obj->bknown = 1;
                    }
                    unbless(obj);
                }
            } else {
                use_whistle(obj);
            }
            break;
        case STETHOSCOPE:
            res = use_stethoscope(obj);
            break;
        case MIRROR:
            res = use_mirror(obj);
            break;
        case BELL:
        case BELL_OF_OPENING:
            use_bell(&obj);
            break;
        case CANDELABRUM_OF_INVOCATION:
            use_candelabrum(obj);
            break;
        case WAX_CANDLE:
        case TALLOW_CANDLE:
            use_candle(&obj);
            break;
        case OIL_LAMP:
        case MAGIC_LAMP:
        case BRASS_LANTERN:
            use_lamp(obj);
            break;
        case POT_OIL:
            light_cocktail(obj);
            break;
        case EXPENSIVE_CAMERA:
            res = use_camera(obj);
            break;
        case TOWEL:
            res = use_towel(obj);
            break;
        case CRYSTAL_BALL:
            use_crystal_ball(obj);
            break;
        case MAGIC_MARKER:
            res = dowrite(obj);
            break;
        case TIN_OPENER:
            if(!carrying(TIN)) {
                You("have no tin to open.");
                goto xit;
            }
            You("cannot open a tin without eating or discarding its contents.");
            if(flags.verbose)
                pline("In order to eat, use the 'e' command.");
            if(obj != uwep)
                pline("Opening the tin will be much easier if you wield the tin opener.");
            goto xit;

        case FIGURINE:
            use_figurine(&obj);
            break;
        case UNICORN_HORN:
            use_unicorn_horn(obj);
            break;
        case WOODEN_FLUTE:
        case MAGIC_FLUTE:
        case TOOLED_HORN:
        case FROST_HORN:
        case FIRE_HORN:
        case WOODEN_HARP:
        case MAGIC_HARP:
        case BUGLE:
        case LEATHER_DRUM:
        case DRUM_OF_EARTHQUAKE:
            res = do_play_instrument(obj);
            break;
        case HORN_OF_PLENTY:    /* not a musical instrument */
            if (obj->spe > 0) {
                struct obj *otmp;
                const char *what;

                consume_obj_charge(obj, true);
                if (!rn2(13)) {
                    otmp = mkobj(POTION_CLASS, false);
                    if (objects[otmp->otyp].oc_magic) do {
                        otmp->otyp = rnd_class(POT_BOOZE, POT_WATER);
                    } while (otmp->otyp == POT_SICKNESS);
                    what = "A potion";
                } else {
                    otmp = mkobj(FOOD_CLASS, false);
                    if (otmp->otyp == FOOD_RATION && !rn2(7))
                        otmp->otyp = LUMP_OF_ROYAL_JELLY;
                    what = "Some food";
                }
                pline("%s spills out.", what);
                otmp->blessed = obj->blessed;
                otmp->cursed = obj->cursed;
                otmp->owt = weight(otmp);
                otmp = hold_another_object(otmp, u.uswallow ?
                        "Oops!  %s out of your reach!" :
                        (Is_airlevel(&u.uz) ||
                         Is_waterlevel(&u.uz) ||
                         levl[u.ux][u.uy].typ < IRONBARS ||
                         levl[u.ux][u.uy].typ >= ICE) ?
                        "Oops!  %s away from you!" :
                        "Oops!  %s to the floor!",
                        The(aobjnam(otmp, "slip")),
                        (const char *)0);
                makeknown(HORN_OF_PLENTY);
            } else
                message_const(MSG_NOTHING_HAPPENS);
            break;
        case LAND_MINE:
        case BEARTRAP:
            use_trap(obj);
            break;
        case FLINT:
        case LUCKSTONE:
        case LOADSTONE:
        case TOUCHSTONE:
            use_stone(obj);
            break;
        default:
            /* Pole-weapons can strike at a distance */
            if (is_pole(obj)) {
                res = use_pole(obj);
                break;
            } else if (is_pick(obj) || is_axe(obj)) {
                res = use_pick_axe(obj);
                break;
            }
            pline("Sorry, I don't know how to use that.");
xit:
            nomul(0);
            return 0;
    }
    if (res && obj && obj->oartifact) arti_speak(obj);
    nomul(0);
    return res;
}

/* Keep track of unfixable troubles for purposes of messages saying you feel
 * great.
 */
int unfixable_trouble_count(bool is_horn) {
    int unfixable_trbl = 0;

    if (Stoned) unfixable_trbl++;
    if (Strangled) unfixable_trbl++;
    if (Wounded_legs() && !u.usteed) unfixable_trbl++;
    if (Slimed) unfixable_trbl++;
    /* lycanthropy is not desirable, but it doesn't actually make you feel
       bad */

    /* we'll assume that intrinsic stunning from being a bat/stalker
       doesn't make you feel bad */
    if (!is_horn) {
        if (Confusion()) unfixable_trbl++;
        if (Sick) unfixable_trbl++;
        if (u.uprops[HALLUC].intrinsic) unfixable_trbl++;
        if (Vomiting) unfixable_trbl++;
        if (get_HStun()) unfixable_trbl++;
    }
    return unfixable_trbl;
}
