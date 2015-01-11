/* See LICENSE in the root of this project for change info */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "move.h"
#include "dungeon_util.h"
#include "align.h"
#include "apply.h"
#include "attrib.h"
#include "ball.h"
#include "cmd.h"
#include "coord.h"
#include "dbridge.h"
#include "decl.h"
#include "dig.h"
#include "display.h"
#include "do.h"
#include "do_name.h"
#include "do_wear.h"
#include "dog.h"
#include "dokick.h"
#include "dungeon.h"
#include "end.h"
#include "engrave.h"
#include "exper.h"
#include "explode.h"
#include "flag.h"
#include "global.h"
#include "hack.h"
#include "hacklib.h"
#include "invent.h"
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
#include "mthrowu.h"
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
#include "worm.h"
#include "worn.h"
#include "you.h"
#include "youprop.h"
#include "zap.h"

extern const char * const destroy_strings[]; /* from zap.c */

static const char * const a_your[2] = { "a", "your" };
static const char * const A_Your[2] = { "A", "Your" };
static const char tower_of_flame[] = "tower of flame";
static const char * const A_gush_of_water_hits = "A gush of water hits";
static const char * const blindgas[6] = { "humid", "odorless", "pungent", "chilling", "acrid", "biting" };

static bool burn_dmg(struct obj *item, const char *descr, struct monst *victim) {
    return rust_dmg(item, descr, 0, false, victim);
}

/* called when you're hit by fire (dofiretrap,buzz,zapyourself,explode) */
bool burnarmor(struct monst *victim) {
    struct obj *item;
    char buf[BUFSZ];
    int mat_idx;

    if (!victim)
        return 0;
    while (1) {
        switch (rn2(5)) {
            case 0:
                item = (victim == &youmonst) ? uarmh : which_armor(victim, W_ARMH);
                if (item) {
                    mat_idx = objects[item->otyp].oc_material;
                    sprintf(buf, "%s helmet", materialnm[mat_idx]);
                }
                if (!burn_dmg(item, item ? buf : "helmet", victim))
                    continue;
                break;
            case 1:
                item = (victim == &youmonst) ? uarmc : which_armor(victim, W_ARMC);
                if (item) {
                    burn_dmg(item, cloak_simple_name(item), victim);
                    return true;
                }
                item = (victim == &youmonst) ? uarm : which_armor(victim, W_ARM);
                if (item) {
                    burn_dmg(item, "TODO: xname(item)", victim);
                    return true;
                }
                item = (victim == &youmonst) ? uarmu : which_armor(victim, W_ARMU);
                if (item)
                    burn_dmg(item, "shirt", victim);
                return true;
            case 2:
                item = (victim == &youmonst) ? uarms : which_armor(victim, W_ARMS);
                if (!burn_dmg(item, "wooden shield", victim))
                    continue;
                break;
            case 3:
                item = (victim == &youmonst) ? uarmg : which_armor(victim, W_ARMG);
                if (!burn_dmg(item, "gloves", victim))
                    continue;
                break;
            case 4:
                item = (victim == &youmonst) ? uarmf : which_armor(victim, W_ARMF);
                if (!burn_dmg(item, "boots", victim))
                    continue;
                break;
        }
        break; /* Out of while loop */
    }
    return false;
}

/* Generic rust-armor function.  Returns true if a message was printed;
 * "print", if set, means to print a message (and thus to return true) even
 * if the item could not be rusted; otherwise a message is printed and true is
 * returned only for rustable items.
 */
bool rust_dmg(struct obj *otmp, const char *ostr, int type, bool print, struct monst *victim) {
    static const char * const action[] = { "smoulder", "rust", "rot", "corrode" };
    static const char * const msg[] = { "burnt", "rusted", "rotten", "corroded" };
    bool vulnerable = false;
    bool grprot = false;
    bool is_primary = true;
    bool vismon = (victim != &youmonst) && canseemon(victim);
    int erosion;

    if (!otmp)
        return (false);
    switch (type) {
        case 0:
            vulnerable = is_flammable(otmp);
            break;
        case 1:
            vulnerable = is_rustprone(otmp);
            grprot = true;
            break;
        case 2:
            vulnerable = is_rottable(otmp);
            is_primary = false;
            break;
        case 3:
            vulnerable = is_corrodeable(otmp);
            grprot = true;
            is_primary = false;
            break;
    }
    erosion = is_primary ? otmp->oeroded : otmp->oeroded2;

    if (!print && (!vulnerable || otmp->oerodeproof || erosion == MAX_ERODE))
        return false;

    if (!vulnerable) {
        if (flags.verbose) {
            if (victim == &youmonst) {
                Your("%s %s not affected.", ostr, "TODO:vtense(ostr, \"are\")");
            } else if (vismon) {
                pline("%s's %s %s not affected.", "TODO:Monnam(victim)", ostr,
                        "TODO:vtense(ostr, \"are\")");
            }
        }
    } else if (erosion < MAX_ERODE) {
        if (grprot && otmp->greased) {
            grease_protect(otmp, ostr, victim);
        } else if (otmp->oerodeproof || (otmp->blessed && !rnl(4))) {
            if (flags.verbose) {
                if (victim == &youmonst)
                    pline("Somehow, your %s %s not affected.", ostr, "TODO:vtense(ostr, \"are\")");
                else if (vismon)
                    pline("Somehow, %s's %s %s not affected.", "TODO:mon_nam(victim)", ostr,
                           "TODO: vtense(ostr, \"are\")");
            }
        } else {
            if (victim == &youmonst)
                Your("%s %s%s!", ostr, "TODO:vtense(ostr, action[type])", erosion + 1 == MAX_ERODE ? " completely" : erosion ? " further" : "");
            else if (vismon)
                pline("%s's %s %s%s!", "TODO:Monnam(victim)", ostr, "TODO:vtense(ostr, action[type])", erosion + 1 == MAX_ERODE ? " completely" : erosion ? " further" : "");
            if (is_primary)
                otmp->oeroded++;
            else
                otmp->oeroded2++;
        }
    } else {
        if (flags.verbose) {
            if (victim == &youmonst)
                Your("%s %s completely %s.", ostr, "TODO:vtense(ostr, Blind ? \"feel\" : \"look\")",
                        msg[type]);
            else if (vismon)
                pline("%s's %s %s completely %s.", "TODO:Monnam(victim)", ostr,
                        "TODO:vtense(ostr, \"look\")", msg[type]);
        }
    }
    return (true);
}

void grease_protect(struct obj *otmp, const char *ostr, struct monst *victim) {
    static const char txt[] = "protected by the layer of grease!";
    bool vismon = victim && (victim != &youmonst) && canseemon(victim);

    if (ostr) {
        if (victim == &youmonst)
            Your("%s %s %s", ostr, "TODO:vtense(ostr, \"are\")", txt);
        else if (vismon)
            pline("%s's %s %s %s", "TODO:Monnam(victim)", ostr, "TODO:vtense(ostr, \"are\")", txt);
    } else {
        if (victim == &youmonst) {
            message_object(MSG_YOUR_O_ARE_PROTECTED_BY_GREASE, otmp);
        } else if (vismon) {
            message_monster_object(MSG_M_O_ARE_PROTECTED_BY_GREASE, victim, otmp);
        }
    }
    if (!rn2(2)) {
        otmp->greased = 0;
        if (carried(otmp)) {
            pline_The("grease dissolves.");
        }
    }
}

static bool isclearpath(coord *cc, int distance, signed char dx, signed char dy) {
    unsigned char typ;
    signed char x, y;

    x = cc->x;
    y = cc->y;
    while (distance-- > 0) {
        x += dx;
        y += dy;
        typ = levl[x][y].typ;
        if (!isok(x, y) || !ZAP_POS(typ) || closed_door(x, y))
            return false;
    }
    cc->x = x;
    cc->y = y;
    return true;
}

static int mkroll_launch(struct trap *ttmp, signed char x, signed char y, short otyp, long ocount) {
    struct obj *otmp;
    int tmp;
    signed char dx, dy;
    int distance;
    coord cc;
    coord bcc;
    int trycount = 0;
    bool success = false;
    int mindist = 4;

    if (ttmp->ttyp == ROLLING_BOULDER_TRAP)
        mindist = 2;
    distance = rn1(5, 4); /* 4..8 away */
    tmp = rn2(8); /* randomly pick a direction to try first */
    while (distance >= mindist) {
        dx = xdir[tmp];
        dy = ydir[tmp];
        cc.x = x;
        cc.y = y;
        /* Prevent boulder from being placed on water */
        if (ttmp->ttyp == ROLLING_BOULDER_TRAP && is_pool(x + distance * dx, y + distance * dy))
            success = false;
        else
            success = isclearpath(&cc, distance, dx, dy);
        if (ttmp->ttyp == ROLLING_BOULDER_TRAP) {
            bool success_otherway;
            bcc.x = x;
            bcc.y = y;
            success_otherway = isclearpath(&bcc, distance, -(dx), -(dy));
            if (!success_otherway)
                success = false;
        }
        if (success)
            break;
        if (++tmp > 7)
            tmp = 0;
        if ((++trycount % 8) == 0)
            --distance;
    }
    if (!success) {
        /* create the trap without any ammo, launch pt at trap location */
        cc.x = bcc.x = x;
        cc.y = bcc.y = y;
    } else {
        otmp = mksobj(otyp, true, false);
        otmp->quan = ocount;
        otmp->owt = weight(otmp);
        place_object(otmp, cc.x, cc.y);
        stackobj(otmp);
    }
    ttmp->launch.x = cc.x;
    ttmp->launch.y = cc.y;
    if (ttmp->ttyp == ROLLING_BOULDER_TRAP) {
        ttmp->launch2.x = bcc.x;
        ttmp->launch2.y = bcc.y;
    } else
        ttmp->launch_otyp = otyp;
    newsym(ttmp->launch.x, ttmp->launch.y);
    return 1;
}

struct trap *
maketrap(int x, int y, int typ) {
    struct trap *ttmp;
    struct rm *lev;
    bool oldplace;

    if ((ttmp = t_at(x, y)) != 0) {
        if (ttmp->ttyp == MAGIC_PORTAL)
            return (struct trap *)0;
        oldplace = true;
        if (u.utrap && (x == u.ux) && (y == u.uy) && ((u.utraptype == TT_BEARTRAP && typ != BEAR_TRAP) || (u.utraptype == TT_WEB && typ != WEB) || (u.utraptype == TT_PIT && typ != PIT && typ != SPIKED_PIT)))
            u.utrap = 0;
    } else {
        oldplace = false;
        ttmp = newtrap();
        ttmp->tx = x;
        ttmp->ty = y;
        ttmp->launch.x = -1; /* force error if used before set */
        ttmp->launch.y = -1;
    }
    ttmp->ttyp = typ;
    switch (typ) {
        case STATUE_TRAP: /* create a "living" statue */
        {
            struct monst *mtmp;
            struct obj *otmp, *statue;

            statue = mkcorpstat(STATUE, (struct monst *)0, &mons[rndmonnum()], x, y, false);
            mtmp = makemon(&mons[statue->corpsenm], 0, 0, NO_MM_FLAGS);
            if (!mtmp)
                break; /* should never happen */
            while (mtmp->minvent) {
                otmp = mtmp->minvent;
                otmp->owornmask = 0;
                obj_extract_self(otmp);
                (void)add_to_container(statue, otmp);
            }
            statue->owt = weight(statue);
            mongone(mtmp);
            break;
        }
        case ROLLING_BOULDER_TRAP: /* boulder will roll towards trigger */
            mkroll_launch(ttmp, x, y, BOULDER, 1L);
            break;
        case HOLE:
        case PIT:
        case SPIKED_PIT:
        case TRAPDOOR:
            lev = &levl[x][y];
            if (*in_rooms(x, y, SHOPBASE) && ((typ == HOLE || typ == TRAPDOOR) || IS_DOOR(lev->typ) || IS_WALL(lev->typ)))
                add_damage(x, y, /* schedule repair */
                ((IS_DOOR(lev->typ) || IS_WALL(lev->typ)) && !flags.mon_moving) ? 200L : 0L);
            lev->doormask = 0; /* subsumes altarmask, icedpool... */
            if (IS_ROOM(lev->typ)) /* && !IS_AIR(lev->typ) */
                lev->typ = ROOM;

            /*
             * some cases which can happen when digging
             * down while phazing thru solid areas
             */
            else if (lev->typ == STONE || lev->typ == SCORR)
                lev->typ = CORR;
            else if (IS_WALL(lev->typ) || lev->typ == SDOOR)
                lev->typ = level.flags.is_maze_lev ? ROOM : level.flags.is_cavernous_lev ? CORR : DOOR;

            unearth_objs(x, y);
            break;
    }
    if (ttmp->ttyp == HOLE)
        ttmp->tseen = 1; /* You can't hide a hole */
    else
        ttmp->tseen = 0;
    ttmp->once = 0;
    ttmp->madeby_u = 0;
    ttmp->dst.dnum = -1;
    ttmp->dst.dlevel = -1;
    if (!oldplace) {
        ttmp->ntrap = ftrap;
        ftrap = ttmp;
    }
    return (ttmp);
}

/* td == true : trap door or hole */
void fall_through(
bool td) {
    d_level dtmp;
    char msgbuf[BUFSZ];
    const char *dont_fall = 0;
    int newlevel = dunlev(&u.uz);

    /* KMH -- You can't escape the Sokoban level traps */
    if (Blind && Levitation && !In_sokoban(&u.uz))
        return;

    do {
        newlevel++;
    } while (!rn2(4) && newlevel < dunlevs_in_dungeon(&u.uz));

    if (td) {
        struct trap *t = t_at(u.ux, u.uy);
        seetrap(t);
        if (!In_sokoban(&u.uz)) {
            if (t->ttyp == TRAPDOOR)
                pline("A trap door opens up under you!");
            else
                pline("There's a gaping hole under you!");
        }
    } else
        pline_The("%s opens up under you!", surface(u.ux, u.uy));

    if (In_sokoban(&u.uz) && Can_fall_thru(&u.uz)) {
        /* You can't escape the Sokoban level traps */
    } else if (Levitation || u.ustuck || !Can_fall_thru(&u.uz) || Flying || is_clinger(youmonst.data) || (Inhell && !u.uevent.invoked && newlevel == dunlevs_in_dungeon(&u.uz))) {
        dont_fall = "don't fall in.";
    } else if (youmonst.data->msize >= MZ_HUGE) {
        dont_fall = "don't fit through.";
    } else if (!next_to_u()) {
        dont_fall = "are jerked back by your pet!";
    }
    if (dont_fall) {
        You("%s", dont_fall);
        /* hero didn't fall through, but any objects here might */
        impact_drop((struct obj *)0, u.ux, u.uy, 0);
        if (!td) {
            display_nhwindow(WIN_MESSAGE, false);
            pline_The("opening under you closes up.");
        }
        return;
    }

    if (*u.ushops)
        shopdig(1);
    if (Is_stronghold(&u.uz)) {
        find_hell(&dtmp);
    } else {
        dtmp.dnum = u.uz.dnum;
        dtmp.dlevel = newlevel;
    }
    if (!td)
        sprintf(msgbuf, "The hole in the %s above you closes up.", ceiling(u.ux, u.uy));
    schedule_goto(&dtmp, false, true, 0, (char *)0, !td ? msgbuf : (char *)0);
}

/*
 * Animate the given statue.  May have been via shatter attempt, trap,
 * or stone to flesh spell.  Return a monster if successfully animated.
 * If the monster is animated, the object is deleted.  If fail_reason
 * is non-null, then fill in the reason for failure (or success).
 *
 * The cause of animation is:
 *
 *      ANIMATE_NORMAL  - hero "finds" the monster
 *      ANIMATE_SHATTER - hero tries to destroy the statue
 *      ANIMATE_SPELL   - stone to flesh spell hits the statue
 *
 * Perhaps x, y is not needed if we can use get_obj_location() to find
 * the statue's location... ???
 */
struct monst *
animate_statue(struct obj *statue, signed char x, signed char y, int cause, int *fail_reason) {
    struct permonst *mptr;
    struct monst *mon = 0;
    struct obj *item;
    coord cc;
    bool historic = (Role_if(PM_ARCHEOLOGIST) && !flags.mon_moving && (statue->spe & STATUE_HISTORIC));
    char statuename[BUFSZ];

    strcpy(statuename, the(xname(statue)));

    if (statue->oxlth && statue->oattached == OATTACHED_MONST) {
        cc.x = x, cc.y = y;
        mon = montraits(statue, &cc);
        if (mon && mon->mtame && !mon->isminion)
            wary_dog(mon, true);
    } else {
        /* statue of any golem hit with stone-to-flesh becomes flesh golem */
        if (is_golem(&mons[statue->corpsenm]) && cause == ANIMATE_SPELL)
            mptr = &mons[PM_FLESH_GOLEM];
        else
            mptr = &mons[statue->corpsenm];
        /*
         * Guard against someone wishing for a statue of a unique monster
         * (which is allowed in normal play) and then tossing it onto the
         * [detected or guessed] location of a statue trap.  Normally the
         * uppermost statue is the one which would be activated.
         */
        if ((mptr->geno & G_UNIQ) && cause != ANIMATE_SPELL) {
            if (fail_reason)
                *fail_reason = AS_MON_IS_UNIQUE;
            return (struct monst *)0;
        }
        if (cause == ANIMATE_SPELL && ((mptr->geno & G_UNIQ) || mptr->msound == MS_GUARDIAN)) {
            /* Statues of quest guardians or unique monsters
             * will not stone-to-flesh as the real thing.
             */
            mon = makemon(&mons[PM_DOPPELGANGER], x, y,
            NO_MINVENT | MM_NOCOUNTBIRTH | MM_ADJACENTOK);
            if (mon) {
                /* makemon() will set mon->cham to
                 * CHAM_ORDINARY if hero is wearing
                 * ring of protection from shape changers
                 * when makemon() is called, so we have to
                 * check the field before calling newcham().
                 */
                if (mon->cham == CHAM_DOPPELGANGER)
                    (void)newcham(mon, mptr, false, false);
            }
        } else
            mon = makemon(mptr, x, y, (cause == ANIMATE_SPELL) ? (NO_MINVENT | MM_ADJACENTOK) : NO_MINVENT);
    }

    if (!mon) {
        if (fail_reason)
            *fail_reason = AS_NO_MON;
        return (struct monst *)0;
    }

    /* in case statue is wielded and hero zaps stone-to-flesh at self */
    if (statue->owornmask)
        remove_worn_item(statue, true);

    /* allow statues to be of a specific gender */
    if (statue->spe & STATUE_MALE)
        mon->female = false;
    else if (statue->spe & STATUE_FEMALE)
        mon->female = true;
    /* if statue has been named, give same name to the monster */
    if (statue->onamelth)
        mon = christen_monst(mon, ONAME(statue));
    /* transfer any statue contents to monster's inventory */
    while ((item = statue->cobj) != 0) {
        obj_extract_self(item);
        (void)add_to_minv(mon, item);
    }
    m_dowear(mon, true);
    delobj(statue);

    /* mimic statue becomes seen mimic; other hiders won't be hidden */
    if (mon->m_ap_type)
        seemimic(mon);
    else
        mon->mundetected = false;
    if ((x == u.ux && y == u.uy) || cause == ANIMATE_SPELL) {
        const char *comes_to_life = nonliving(mon->data) ? "moves" : "comes to life";
        if (cause == ANIMATE_SPELL)
            pline("%s %s!", upstart(statuename),
            canspotmon(mon) ? comes_to_life : "disappears");
        else
            pline_The("statue %s!",
            canspotmon(mon) ? comes_to_life : "disappears");
        if (historic) {
            You_feel("guilty that the historic statue is now gone.");
            adjalign(-1);
        }
    } else if (cause == ANIMATE_SHATTER) {
        pline("Instead of shattering, the statue suddenly %s!",
        canspotmon(mon) ? "comes to life" : "disappears");
    } else { /* cause == ANIMATE_NORMAL */
        if (canspotmon(mon)) {
            message_monster(MSG_YOU_FIND_M_POSING_AS_STATUE, mon);
        } else {
            message_const(MSG_YOU_FIND_SOMETHING_POSING_AS_STATUE);
        }
        stop_occupation();
    }
    /* avoid hiding under nothing */
    if (x == u.ux && y == u.uy &&
    Upolyd && hides_under(youmonst.data) && !OBJ_AT(x, y))
        u.uundetected = 0;

    if (fail_reason)
        *fail_reason = AS_OK;
    return mon;
}

/*
 * You've either stepped onto a statue trap's location or you've triggered a
 * statue trap by searching next to it or by trying to break it with a wand
 * or pick-axe.
 */
struct monst *
activate_statue_trap(struct trap *trap, signed char x, signed char y, bool shatter) {
    struct monst *mtmp = (struct monst *)0;
    struct obj *otmp = sobj_at(STATUE, x, y);
    int fail_reason;

    /*
     * Try to animate the first valid statue.  Stop the loop when we
     * actually create something or the failure cause is not because
     * the mon was unique.
     */
    deltrap(trap);
    while (otmp) {
        mtmp = animate_statue(otmp, x, y, shatter ? ANIMATE_SHATTER : ANIMATE_NORMAL, &fail_reason);
        if (mtmp || fail_reason != AS_MON_IS_UNIQUE)
            break;

        while ((otmp = otmp->nexthere) != 0)
            if (otmp->otyp == STATUE)
                break;
    }

    if (Blind)
        feel_location(x, y);
    else
        newsym(x, y);
    return mtmp;
}

static bool keep_saddle_with_steedcorpse(unsigned steed_mid, struct obj *objchn, struct obj *saddle) {
    if (!saddle)
        return false;
    while (objchn) {
        if (objchn->otyp == CORPSE && objchn->oattached == OATTACHED_MONST && objchn->oxlth) {
            struct monst *mtmp = (struct monst *)objchn->oextra;
            if (mtmp->m_id == steed_mid) {
                /* move saddle */
                signed char x, y;
                if (get_obj_location(objchn, &x, &y, 0)) {
                    obj_extract_self(saddle);
                    place_object(saddle, x, y);
                    stackobj(saddle);
                }
                return true;
            }
        }
        if (Has_contents(objchn) && keep_saddle_with_steedcorpse(steed_mid, objchn->cobj, saddle))
            return true;
        objchn = objchn->nobj;
    }
    return false;
}

/* Monster is hit by trap. */
/* Note: doesn't work if both obj and d_override are null */
static bool thitm(int tlev, struct monst *mon, struct obj *obj, int d_override, bool nocorpse) {
    int strike;
    bool trapkilled = false;

    if (d_override)
        strike = 1;
    else if (obj)
        strike = (find_mac(mon) + tlev + obj->spe <= rnd(20));
    else
        strike = (find_mac(mon) + tlev <= rnd(20));

    /* Actually more accurate than thitu, which doesn't take
     * obj->spe into account.
     */
    if (!strike) {
        if (obj && cansee(mon->mx, mon->my)) {
            message_monster_object(MSG_M_IS_ALMOST_HIT_BY_O, mon, obj);
        }
    } else {
        int dam = 1;

        if (obj && cansee(mon->mx, mon->my)) {
            message_monster_object(MSG_M_IS_HIT_BY_O, mon, obj);
        }
        if (d_override)
            dam = d_override;
        else if (obj) {
            dam = dmgval(obj, mon);
            if (dam < 1)
                dam = 1;
        }
        if ((mon->mhp -= dam) <= 0) {
            int xx = mon->mx;
            int yy = mon->my;

            monkilled(mon, "", nocorpse ? -AD_RBRE : AD_PHYS);
            if (mon->mhp <= 0) {
                newsym(xx, yy);
                trapkilled = true;
            }
        }
    }
    if (obj && (!strike || d_override)) {
        place_object(obj, mon->mx, mon->my);
        stackobj(obj);
    } else if (obj)
        dealloc_obj(obj);

    return trapkilled;
}

static int steedintrap(struct trap *trap, struct obj *otmp) {
    struct monst *mtmp = u.usteed;
    struct permonst *mptr;
    int tt;
    bool in_sight;
    bool trapkilled = false;
    bool steedhit = false;

    if (!u.usteed || !trap)
        return 0;
    mptr = mtmp->data;
    tt = trap->ttyp;
    mtmp->mx = u.ux;
    mtmp->my = u.uy;

    in_sight = !Blind;
    switch (tt) {
        case ARROW_TRAP:
            if (!otmp) {
                impossible("steed hit by non-existant arrow?");
                return 0;
            }
            if (thitm(8, mtmp, otmp, 0, false))
                trapkilled = true;
            steedhit = true;
            break;
        case DART_TRAP:
            if (!otmp) {
                impossible("steed hit by non-existant dart?");
                return 0;
            }
            if (thitm(7, mtmp, otmp, 0, false))
                trapkilled = true;
            steedhit = true;
            break;
        case SLP_GAS_TRAP:
            if (!resists_sleep(mtmp) && !breathless(mptr) && !mtmp->msleeping && mtmp->mcanmove) {
                mtmp->mcanmove = 0;
                mtmp->mfrozen = rnd(25);
                if (in_sight) {
                    message_monster(MSG_M_SUDDENLY_FALLS_ASLEEP, mtmp);
                }
            }
            steedhit = true;
            break;
        case LANDMINE:
            if (thitm(0, mtmp, (struct obj *)0, rnd(16), false))
                trapkilled = true;
            steedhit = true;
            break;
        case PIT:
        case SPIKED_PIT:
            if (mtmp->mhp <= 0 || thitm(0, mtmp, (struct obj *)0, rnd((tt == PIT) ? 6 : 10), false))
                trapkilled = true;
            steedhit = true;
            break;
        case POLY_TRAP:
            if (!resists_magm(mtmp)) {
                if (!resist(mtmp, WAND_CLASS, 0, NOTELL)) {
                    (void)newcham(mtmp, (struct permonst *)0,
                    false, false);
                    if (!can_saddle(mtmp) || !can_ride(mtmp)) {
                        dismount_steed(DISMOUNT_POLY);
                    } else {
                        message_monster(MSG_YOU_HAVE_TO_ADJUST_YOURSELF_IN_THE_SADDLE_ON_M, mtmp);
                    }

                }
                steedhit = true;
                break;
                default:
                return 0;
            }
    }
    if (trapkilled) {
        dismount_steed(DISMOUNT_POLY);
        return 2;
    } else if (steedhit)
        return 1;
    else
        return 0;
}

/* box: null for floor trap */
static void dofiretrap(struct obj *box) {
    bool see_it = !Blind;
    int num, alt;

    /* Bug: for box case, the equivalent of burn_floor_paper() ought
     * to be done upon its contents.
     */

    if ((box && !carried(box)) ? is_pool(box->ox, box->oy) : Underwater) {
        pline("A cascade of steamy bubbles erupts from %s!", the(box ? xname(box) : surface(u.ux, u.uy)));
        if (Fire_resistance())
            You("are uninjured.");
        else
            losehp(rnd(3), killed_by_const(KM_BOILING_WATER));
        return;
    }
    pline("A %s %s from %s!", tower_of_flame, box ? "bursts" : "erupts", the(box ? xname(box) : surface(u.ux, u.uy)));
    if (Fire_resistance()) {
        shieldeff(u.ux, u.uy);
        num = rn2(2);
    } else if (Upolyd) {
        num = d(2, 4);
        switch (u.umonnum) {
            case PM_PAPER_GOLEM:
                alt = u.mhmax;
                break;
            case PM_STRAW_GOLEM:
                alt = u.mhmax / 2;
                break;
            case PM_WOOD_GOLEM:
                alt = u.mhmax / 4;
                break;
            case PM_LEATHER_GOLEM:
                alt = u.mhmax / 8;
                break;
            default:
                alt = 0;
                break;
        }
        if (alt > num)
            num = alt;
        if (u.mhmax > mons[u.umonnum].mlevel)
            u.mhmax -= rn2(min(u.mhmax, num + 1));
    } else {
        num = d(2, 4);
        if (u.uhpmax > u.ulevel)
            u.uhpmax -= rn2(min(u.uhpmax, num + 1));
    }
    if (!num)
        You("are uninjured.");
    else
        losehp(num, killed_by_const(KM_TOWER_OF_FLAME));
    burn_away_slime();

    if (burnarmor(&youmonst) || rn2(3)) {
        destroy_item(SCROLL_CLASS, AD_FIRE);
        destroy_item(SPBOOK_CLASS, AD_FIRE);
        destroy_item(POTION_CLASS, AD_FIRE);
    }
    if (!box && burn_floor_paper(u.ux, u.uy, see_it, true) && !see_it)
        You("smell paper burning.");
    if (is_ice(u.ux, u.uy))
        melt_ice(u.ux, u.uy);
}

static void domagictrap(void) {
    int fate = rnd(20);

    /* What happened to the poor sucker? */

    if (fate < 10) {
        /* Most of the time, it creates some monsters. */
        int cnt = rnd(4);

        if (!resists_blnd(&youmonst)) {
            You("are momentarily blinded by a flash of light!");
            make_blinded((long)rn1(5, 10), false);
            if (!Blind)
                Your("%s", vision_clears);
        } else if (!Blind) {
            You("see a flash of light!");
        } else
            You_hear("a deafening roar!");
        while (cnt--)
            (void)makemon((struct permonst *)0, u.ux, u.uy, NO_MM_FLAGS);
    } else
        switch (fate) {

            case 10:
            case 11:
                /* sometimes nothing happens */
                break;
            case 12: /* a flash of fire */
                dofiretrap((struct obj *)0);
                break;

                /* odd feelings */
            case 13:
                pline("A shiver runs up and down your %s!", body_part(SPINE));
                break;
            case 14:
                You_hear(Hallucination() ? "the moon howling at you." : "distant howling.");
                break;
            case 15:
                if (on_level(&u.uz, &qstart_level))
                    You_feel("%slike the prodigal son.", (flags.female || (Upolyd && is_neuter(youmonst.data))) ? "oddly " : "");
                else
                    You("suddenly yearn for %s.", Hallucination() ? "Cleveland" : (In_quest(&u.uz) || at_dgn_entrance("The Quest")) ? "your nearby homeland" : "your distant homeland");
                break;
            case 16:
                Your("pack shakes violently!");
                break;
            case 17:
                You(Hallucination() ? "smell hamburgers." : "smell charred flesh.");
                break;
            case 18:
                You_feel("tired.");
                break;

                /* very occasionally something nice happens. */

            case 19:
                /* tame nearby monsters */
            {
                int i, j;
                struct monst *mtmp;

                (void)adjattrib(A_CHA, 1, false);
                for (i = -1; i <= 1; i++)
                    for (j = -1; j <= 1; j++) {
                        if (!isok(u.ux + i, u.uy + j))
                            continue;
                        mtmp = m_at(u.ux + i, u.uy + j);
                        if (mtmp)
                            (void)tamedog(mtmp, (struct obj *)0);
                    }
                break;
            }

            case 20:
                /* uncurse stuff */
            {
                struct obj pseudo;
                long save_conf = get_HConfusion();

                pseudo = zeroobj; /* neither cursed nor blessed */
                pseudo.otyp = SCR_REMOVE_CURSE;
                set_HConfusion(0L);
                (void)seffects(&pseudo);
                set_HConfusion(save_conf);
                break;
            }
            default:
                break;
        }
}

void dotrap(struct trap *trap, unsigned trflags) {
    int ttype = trap->ttyp;
    struct obj *otmp;
    bool already_seen = trap->tseen;
    bool webmsgok = (!(trflags & NOWEBMSG));
    bool forcebungle = (trflags & FORCEBUNGLE);

    nomul(0);

    /* KMH -- You can't escape the Sokoban level traps */
    if (In_sokoban(&u.uz) && (ttype == PIT || ttype == SPIKED_PIT || ttype == HOLE || ttype == TRAPDOOR)) {
        /* The "air currents" message is still appropriate -- even when
         * the hero isn't flying or levitating -- because it conveys the
         * reason why the player cannot escape the trap with a dexterity
         * check, clinging to the ceiling, etc.
         */
        pline("Air currents pull you down into %s %s!", a_your[trap->madeby_u], defsyms[trap_to_defsym(ttype)].explanation);
        /* then proceed to normal trap effect */
    } else if (already_seen) {
        if ((Levitation || Flying) && (ttype == PIT || ttype == SPIKED_PIT || ttype == HOLE || ttype == BEAR_TRAP)) {
            You("%s over %s %s.",
            Levitation ? "float" : "fly", a_your[trap->madeby_u], defsyms[trap_to_defsym(ttype)].explanation);
            return;
        }
        if (!Fumbling() && ttype != MAGIC_PORTAL && ttype != ANTI_MAGIC && !forcebungle && (!rn2(5) || ((ttype == PIT || ttype == SPIKED_PIT) && is_clinger(youmonst.data)))) {
            You("escape %s %s.", (ttype == ARROW_TRAP && !trap->madeby_u) ? "an" : a_your[trap->madeby_u], defsyms[trap_to_defsym(ttype)].explanation);
            return;
        }
    }

    if (u.usteed)
        u.usteed->mtrapseen |= (1 << (ttype - 1));

    switch (ttype) {
        case ARROW_TRAP:
            if (trap->once && trap->tseen && !rn2(15)) {
                You_hear("a loud click!");
                deltrap(trap);
                newsym(u.ux, u.uy);
                break;
            }
            trap->once = 1;
            seetrap(trap);
            pline("An arrow shoots out at you!");
            otmp = mksobj(ARROW, true, false);
            otmp->quan = 1L;
            otmp->owt = weight(otmp);
            otmp->opoisoned = 0;
            if (u.usteed && !rn2(2) && steedintrap(trap, otmp)) {
                /* nothing */
            } else if (thitu(8, dmgval(otmp, &youmonst), otmp, "arrow")) {
                obfree(otmp, (struct obj *)0);
            } else {
                place_object(otmp, u.ux, u.uy);
                if (!Blind)
                    otmp->dknown = 1;
                stackobj(otmp);
                newsym(u.ux, u.uy);
            }
            break;
        case DART_TRAP:
            if (trap->once && trap->tseen && !rn2(15)) {
                You_hear("a soft click.");
                deltrap(trap);
                newsym(u.ux, u.uy);
                break;
            }
            trap->once = 1;
            seetrap(trap);
            pline("A little dart shoots out at you!");
            otmp = mksobj(DART, true, false);
            otmp->quan = 1L;
            otmp->owt = weight(otmp);
            if (!rn2(6))
                otmp->opoisoned = 1;
            if (u.usteed && !rn2(2) && steedintrap(trap, otmp)) {
                /* nothing */
            } else if (thitu(7, dmgval(otmp, &youmonst), otmp, "little dart")) {
                if (otmp->opoisoned)
                    poisoned("dart", A_CON, "little dart", -10);
                obfree(otmp, (struct obj *)0);
            } else {
                place_object(otmp, u.ux, u.uy);
                if (!Blind)
                    otmp->dknown = 1;
                stackobj(otmp);
                newsym(u.ux, u.uy);
            }
            break;
        case ROCKTRAP:
            if (trap->once && trap->tseen && !rn2(15)) {
                pline("A trap door in %s opens, but nothing falls out!", the(ceiling(u.ux, u.uy)));
                deltrap(trap);
                newsym(u.ux, u.uy);
            } else {
                int dmg = d(2, 6); /* should be std ROCK dmg? */

                trap->once = 1;
                seetrap(trap);
                otmp = mksobj_at(ROCK, u.ux, u.uy, true, false);
                otmp->quan = 1L;
                otmp->owt = weight(otmp);

                message_object_string(MSG_TRAP_DOOR_IN_CEILING_OPENS_AND_O_FALLS_ON_YOUR_HEAD,
                        otmp, ceiling(u.ux, u.uy));

                if (uarmh) {
                    if (is_metallic(uarmh)) {
                        pline("Fortunately, you are wearing a hard helmet.");
                        dmg = 2;
                    } else if (flags.verbose) {
                        message_object(MSG_YOU_RO_DOES_NOT_PROTECT_YOU, uarmh);
                    }
                }

                if (!Blind)
                    otmp->dknown = 1;
                stackobj(otmp);
                newsym(u.ux, u.uy); /* map the rock */

                losehp(dmg, killed_by_const(KM_FALLING_ROCK));
                exercise(A_STR, false);
            }
            break;

        case SQKY_BOARD: /* stepped on a squeaky board */
            if (Levitation || Flying) {
                if (!Blind) {
                    seetrap(trap);
                    if (Hallucination())
                        You("notice a crease in the linoleum.");
                    else
                        You("notice a loose board below you.");
                }
            } else {
                seetrap(trap);
                pline("A board beneath you squeaks loudly.");
                wake_nearby();
            }
            break;

        case BEAR_TRAP:
            if (Levitation || Flying)
                break;
            seetrap(trap);
            if (amorphous(youmonst.data) || is_whirly(youmonst.data) || unsolid(youmonst.data)) {
                pline("%s bear trap closes harmlessly through you.", A_Your[trap->madeby_u]);
                break;
            }
            if (!u.usteed && youmonst.data->msize <= MZ_SMALL) {
                pline("%s bear trap closes harmlessly over you.", A_Your[trap->madeby_u]);
                break;
            }
            u.utrap = rn1(4, 4);
            u.utraptype = TT_BEARTRAP;
            unsigned char made_by_you_flag = trap->madeby_u ? MSG_FLAG_MADE_BY_YOU : 0;
            if (u.usteed) {
                message_monster_flag(MSG_BEAR_TRAP_CLOSES_ON_M_FOOT, u.usteed, made_by_you_flag);
            } else {
                message_flag(MSG_BEAR_TRAP_CLOSES_ON_YOUR_FOOT, made_by_you_flag);
                if (u.umonnum == PM_OWLBEAR || u.umonnum == PM_BUGBEAR)
                    You("howl in anger!");
            }
            exercise(A_DEX, false);
            break;

        case SLP_GAS_TRAP:
            seetrap(trap);
            if (Sleep_resistance() || breathless(youmonst.data)) {
                You("are enveloped in a cloud of gas!");
                break;
            }
            pline("A cloud of gas puts you to sleep!");
            fall_asleep(-rnd(25), true);
            (void)steedintrap(trap, (struct obj *)0);
            break;

        case RUST_TRAP:
            seetrap(trap);
            if (u.umonnum == PM_IRON_GOLEM) {
                int dam = u.mhmax;

                pline("%s you!", A_gush_of_water_hits);
                You("are covered with rust!");
                if (Half_physical_damage)
                    dam = (dam + 1) / 2;
                losehp(dam, killed_by_const(KM_RUSTING_AWAY));
                break;
            } else if (u.umonnum == PM_GREMLIN && rn2(3)) {
                pline("%s you!", A_gush_of_water_hits);
                (void)split_mon(&youmonst, (struct monst *)0);
                break;
            }

            /* Unlike monsters, traps cannot aim their rust attacks at
             * you, so instead of looping through and taking either the
             * first rustable one or the body, we take whatever we get,
             * even if it is not rustable.
             */
            switch (rn2(5)) {
                case 0:
                    pline("%s you on the %s!", A_gush_of_water_hits, body_part(HEAD));
                    (void)rust_dmg(uarmh, "helmet", 1, true, &youmonst);
                    break;
                case 1:
                    pline("%s your left %s!", A_gush_of_water_hits, body_part(ARM));
                    if (rust_dmg(uarms, "shield", 1, true, &youmonst))
                        break;
                    if (u.twoweap || (uwep && bimanual(uwep)))
                        erode_obj(u.twoweap ? uswapwep : uwep, false, true);
                    glovecheck: (void)rust_dmg(uarmg, "gauntlets", 1, true, &youmonst);
                    /* Not "metal gauntlets" since it gets called
                     * even if it's leather for the message
                     */
                    break;
                case 2:
                    pline("%s your right %s!", A_gush_of_water_hits, body_part(ARM));
                    erode_obj(uwep, false, true);
                    goto glovecheck;
                default:
                    pline("%s you!", A_gush_of_water_hits);
                    for (otmp = invent; otmp; otmp = otmp->nobj)
                        (void)snuff_lit(otmp);
                    if (uarmc)
                        (void)rust_dmg(uarmc, cloak_simple_name(uarmc), 1, true, &youmonst);
                    else if (uarm)
                        (void)rust_dmg(uarm, "armor", 1, true, &youmonst);
                    else if (uarmu)
                        (void)rust_dmg(uarmu, "shirt", 1, true, &youmonst);
            }
            break;

        case FIRE_TRAP:
            seetrap(trap);
            dofiretrap(NULL);
            break;

        case PIT:
        case SPIKED_PIT:
            /* KMH -- You can't escape the Sokoban level traps */
            if (!In_sokoban(&u.uz) && (Levitation || Flying))
                break;
            seetrap(trap);
            if (!In_sokoban(&u.uz) && is_clinger(youmonst.data)) {
                if (trap->tseen) {
                    You("see %s %spit below you.", a_your[trap->madeby_u], ttype == SPIKED_PIT ? "spiked " : "");
                } else {
                    pline("%s pit %sopens up under you!", A_Your[trap->madeby_u], ttype == SPIKED_PIT ? "full of spikes " : "");
                    You("don't fall in!");
                }
                break;
            }
            if (!In_sokoban(&u.uz)) {
                char verbbuf[BUFSZ];
                if (u.usteed) {
                    if ((trflags & RECURSIVETRAP) != 0) {
                        char name[BUFSZ];
                        x_monnam(name, BUFSZ, u.usteed,
                                    u.usteed->mnamelth ? ARTICLE_NONE : ARTICLE_THE,
                                    NULL, SUPPRESS_SADDLE, false);
                        sprintf(verbbuf, "and %s fall", name);
                    } else {
                        char name[BUFSZ];
                        x_monnam(name, BUFSZ, u.usteed,
                                    u.usteed->mnamelth ? ARTICLE_NONE : ARTICLE_THE,
                                    "poor", SUPPRESS_SADDLE, false);
                        sprintf(verbbuf, "lead %s", name);
                    }
                } else {
                    strcpy(verbbuf, "fall");
                }
                You("%s into %s pit!", verbbuf, a_your[trap->madeby_u]);
            }
            /* wumpus reference */
            if (Role_if(PM_RANGER) && !trap->madeby_u && !trap->once && In_quest(&u.uz) && Is_qlocate(&u.uz)) {
                pline("Fortunately it has a bottom after all...");
                trap->once = 1;
            } else if (u.umonnum == PM_PIT_VIPER || u.umonnum == PM_PIT_FIEND)
                pline("How pitiful.  Isn't that the pits?");
            if (ttype == SPIKED_PIT) {
                if (u.usteed) {
                    message_monster(MSG_M_LANDS_ON_SHARP_IRON_SPIKES, u.usteed);
                } else {
                    message_const(MSG_YOU_LAND_ON_SHARP_IRON_SPIKES);
                }
            }
            if (!Passes_walls)
                u.utrap = rn1(6, 2);
            u.utraptype = TT_PIT;
            if (!steedintrap(trap, (struct obj *)0)) {
                if (ttype == SPIKED_PIT) {
                    losehp(rnd(10), killed_by_const(KM_FELL_INTO_PIT_OF_IRON_SPIKES));
                    if (!rn2(6))
                        poisoned("spikes", A_STR, "fall onto poison spikes", 8);
                } else
                    losehp(rnd(6), killed_by_const(KM_FELL_INTO_PIT));
                if (Punished && !carried(uball)) {
                    unplacebc();
                    ballfall();
                    placebc();
                }
                selftouch("Falling, you");
                vision_full_recalc = 1; /* vision limits change */
                exercise(A_STR, false);
                exercise(A_DEX, false);
            }
            break;
        case HOLE:
        case TRAPDOOR:
            if (!Can_fall_thru(&u.uz)) {
                seetrap(trap); /* normally done in fall_through */
                impossible("dotrap: %ss cannot exist on this level.", defsyms[trap_to_defsym(ttype)].explanation);
                break; /* don't activate it after all */
            }
            fall_through(true);
            break;

        case TELEP_TRAP:
            seetrap(trap);
            tele_trap(trap);
            break;
        case LEVEL_TELEP:
            seetrap(trap);
            level_tele_trap(trap);
            break;

        case WEB: /* Our luckless player has stumbled into a web. */
            seetrap(trap);
            if (amorphous(youmonst.data) || is_whirly(youmonst.data) || unsolid(youmonst.data)) {
                if (acidic(youmonst.data) || u.umonnum == PM_GELATINOUS_CUBE || u.umonnum == PM_FIRE_ELEMENTAL) {
                    if (webmsgok)
                        You("%s %s spider web!", (u.umonnum == PM_FIRE_ELEMENTAL) ? "burn" : "dissolve", a_your[trap->madeby_u]);
                    deltrap(trap);
                    newsym(u.ux, u.uy);
                    break;
                }
                if (webmsgok)
                    You("flow through %s spider web.", a_your[trap->madeby_u]);
                break;
            }
            if (webmaker(youmonst.data)) {
                if (webmsgok)
                    pline(trap->madeby_u ? "You take a walk on your web." : "There is a spider web here.");
                break;
            }
            if (webmsgok) {
                char verbbuf[BUFSZ];
                verbbuf[0] = '\0';
                if (u.usteed) {
                    char name[BUFSZ];
                    x_monnam(name, BUFSZ, u.usteed, u.usteed->mnamelth ?
                            ARTICLE_NONE : ARTICLE_THE, "poor", SUPPRESS_SADDLE, false);
                    sprintf(verbbuf, "lead %s", name);
                } else {
                    sprintf(verbbuf, "%s", Levitation ? "float" : locomotion(youmonst.data, "stumble"));
                }
                You("%s into %s spider web!", verbbuf, a_your[trap->madeby_u]);
            }
            u.utraptype = TT_WEB;

            /* Time stuck in the web depends on your/steed strength. */
            {
                int str = ACURR(A_STR);

                /* If mounted, the steed gets trapped.  Use mintrap
                 * to do all the work.  If mtrapped is set as a result,
                 * unset it and set utrap instead.  In the case of a
                 * strongmonst and mintrap said it's trapped, use a
                 * short but non-zero trap time.  Otherwise, monsters
                 * have no specific strength, so use player strength.
                 * This gets skipped for webmsgok, which implies that
                 * the steed isn't a factor.
                 */
                if (u.usteed && webmsgok) {
                    /* mtmp location might not be up to date */
                    u.usteed->mx = u.ux;
                    u.usteed->my = u.uy;

                    /* mintrap currently does not return 2(died) for webs */
                    if (mintrap(u.usteed)) {
                        u.usteed->mtrapped = 0;
                        if (strongmonst(u.usteed->data))
                            str = 17;
                    } else {
                        break;
                    }

                    webmsgok = false; /* mintrap printed the messages */
                }
                if (str <= 3)
                    u.utrap = rn1(6, 6);
                else if (str < 6)
                    u.utrap = rn1(6, 4);
                else if (str < 9)
                    u.utrap = rn1(4, 4);
                else if (str < 12)
                    u.utrap = rn1(4, 2);
                else if (str < 15)
                    u.utrap = rn1(2, 2);
                else if (str < 18)
                    u.utrap = rnd(2);
                else if (str < 69)
                    u.utrap = 1;
                else {
                    u.utrap = 0;
                    if (webmsgok)
                        You("tear through %s web!", a_your[trap->madeby_u]);
                    deltrap(trap);
                    newsym(u.ux, u.uy); /* get rid of trap symbol */
                }
            }
            break;

        case STATUE_TRAP:
            (void)activate_statue_trap(trap, u.ux, u.uy, false);
            break;

        case MAGIC_TRAP: /* A magic trap. */
            seetrap(trap);
            if (!rn2(30)) {
                deltrap(trap);
                newsym(u.ux, u.uy); /* update position */
                You("are caught in a magical explosion!");
                losehp(rnd(10), killed_by_const(KM_MAGICAL_EXPLOSION));
                Your("body absorbs some of the magical energy!");
                u.uen = (u.uenmax += 2);
            } else
                domagictrap();
            (void)steedintrap(trap, (struct obj *)0);
            break;

        case ANTI_MAGIC:
            seetrap(trap);
            if (Antimagic()) {
                shieldeff(u.ux, u.uy);
                You_feel("momentarily lethargic.");
            } else
                drain_en(rnd(u.ulevel) + 1);
            break;

        case POLY_TRAP: {
            char verbbuf[BUFSZ];
            seetrap(trap);
            if (u.usteed) {
                char name[BUFSZ];
                x_monnam(name, BUFSZ, u.usteed, u.usteed->mnamelth ?
                        ARTICLE_NONE : ARTICLE_THE, (char *)0, SUPPRESS_SADDLE, false);
                sprintf(verbbuf, "lead %s", name);
            } else {
                sprintf(verbbuf, "%s",
                Levitation ? "float" : locomotion(youmonst.data, "step"));
            }
            You("%s onto a polymorph trap!", verbbuf);
            if (Antimagic() || Unchanging) {
                shieldeff(u.ux, u.uy);
                You_feel("momentarily different.");
                /* Trap did nothing; don't remove it --KAA */
            } else {
                (void)steedintrap(trap, (struct obj *)0);
                deltrap(trap); /* delete trap before polymorph */
                newsym(u.ux, u.uy); /* get rid of trap symbol */
                You_feel("a change coming over you.");
                polyself(false);
            }
            break;
        }
        case LANDMINE: {
            unsigned steed_mid = 0;
            struct obj *saddle = 0;
            if (Levitation || Flying) {
                if (!already_seen && rn2(3))
                    break;
                seetrap(trap);
                pline("%s %s in a pile of soil below you.", already_seen ? "There is" : "You discover", trap->madeby_u ? "the trigger of your mine" : "a trigger");
                if (already_seen && rn2(3))
                    break;
                pline("KAABLAMM!!!  %s %s%s off!", forcebungle ? "Your inept attempt sets" : "The air currents set", already_seen ? a_your[trap->madeby_u] : "", already_seen ? " land mine" : "it");
            } else {
                /* prevent landmine from killing steed, throwing you to
                 * the ground, and you being affected again by the same
                 * mine because it hasn't been deleted yet
                 */
                static bool recursive_mine = false;

                if (recursive_mine)
                    break;
                seetrap(trap);
                pline("KAABLAMM!!!  You triggered %s land mine!", a_your[trap->madeby_u]);
                if (u.usteed)
                    steed_mid = u.usteed->m_id;
                recursive_mine = true;
                (void)steedintrap(trap, (struct obj *)0);
                recursive_mine = false;
                saddle = sobj_at(SADDLE, u.ux, u.uy);
                set_wounded_legs(LEFT_SIDE, rn1(35, 41));
                set_wounded_legs(RIGHT_SIDE, rn1(35, 41));
                exercise(A_DEX, false);
            }
            blow_up_landmine(trap);
            if (steed_mid && saddle && !u.usteed)
                (void)keep_saddle_with_steedcorpse(steed_mid, fobj, saddle);
            newsym(u.ux, u.uy); /* update trap symbol */
            losehp(rnd(16), killed_by_const(KM_LAND_MINE));
            /* fall recursively into the pit... */
            if ((trap = t_at(u.ux, u.uy)) != 0)
                dotrap(trap, RECURSIVETRAP);
            fill_pit(u.ux, u.uy);
            break;
        }
        case ROLLING_BOULDER_TRAP: {
            int style = ROLL | (trap->tseen ? LAUNCH_KNOWN : 0);

            seetrap(trap);
            pline("Click! You trigger a rolling boulder trap!");
            if (!launch_obj(BOULDER, trap->launch.x, trap->launch.y, trap->launch2.x, trap->launch2.y, style)) {
                deltrap(trap);
                newsym(u.ux, u.uy); /* get rid of trap symbol */
                pline("Fortunately for you, no boulder was released.");
            }
            break;
        }
        case MAGIC_PORTAL:
            seetrap(trap);
            domagicportal(trap);
            break;

        default:
            seetrap(trap);
            impossible("You hit a trap of type %u", trap->ttyp);
    }
}

/* some actions common to both player and monsters for triggered landmine */
void blow_up_landmine(struct trap *trap) {
    (void)scatter(trap->tx, trap->ty, 4,
    MAY_DESTROY | MAY_HIT | MAY_FRACTURE | VIS_EFFECTS, (struct obj *)0);
    del_engr_at(trap->tx, trap->ty);
    wake_nearto(trap->tx, trap->ty, 400);
    if (IS_DOOR(levl[trap->tx][trap->ty].typ))
        levl[trap->tx][trap->ty].doormask = D_BROKEN;
    /* TODO: destroy drawbridge if present */
    /* caller may subsequently fill pit, e.g. with a boulder */
    trap->ttyp = PIT; /* explosion creates a pit */
    trap->madeby_u = false; /* resulting pit isn't yours */
    seetrap(trap); /* and it isn't concealed */
}

/*
 * Move obj from (x1,y1) to (x2,y2)
 *
 * Return 0 if no object was launched.
 *        1 if an object was launched and placed somewhere.
 *        2 if an object was launched, but used up.
 */
int launch_obj(short otyp, int x1, int y1, int x2, int y2, int style) {
    struct monst *mtmp;
    struct obj *otmp, *otmp2;
    int dx, dy;
    struct obj *singleobj;
    bool used_up = false;
    bool otherside = false;
    int dist;

    otmp = sobj_at(otyp, x1, y1);
    /* Try the other side too, for rolling boulder traps */
    if (!otmp && otyp == BOULDER) {
        otherside = true;
        otmp = sobj_at(otyp, x2, y2);
    }
    if (!otmp)
        return 0;
    if (otherside) { /* swap 'em */
        int tx, ty;

        tx = x1;
        ty = y1;
        x1 = x2;
        y1 = y2;
        x2 = tx;
        y2 = ty;
    }

    if (otmp->quan == 1L) {
        obj_extract_self(otmp);
        singleobj = otmp;
        otmp = (struct obj *)0;
    } else {
        singleobj = splitobj(otmp, 1L);
        obj_extract_self(singleobj);
    }
    newsym(x1, y1);
    /* in case you're using a pick-axe to chop the boulder that's being
     launched (perhaps a monster triggered it), destroy context so that
     next dig attempt never thinks you're resuming previous effort */
    if ((otyp == BOULDER || otyp == STATUE) && singleobj->ox == digging.pos.x && singleobj->oy == digging.pos.y)
        (void)memset((void *)&digging, 0, sizeof digging);

    dist = distmin(x1, y1, x2, y2);
    bhitpos.x = x1;
    bhitpos.y = y1;
    dx = sgn(x2 - x1);
    dy = sgn(y2 - y1);
    switch (style) {
        case ROLL | LAUNCH_UNSEEN:
            if (otyp == BOULDER) {
                You_hear(Hallucination() ? "someone bowling." : "rumbling in the distance.");
            }
            style &= ~LAUNCH_UNSEEN;
            goto roll;
        case ROLL | LAUNCH_KNOWN:
            /* use otrapped as a flag to ohitmon */
            singleobj->otrapped = 1;
            style &= ~LAUNCH_KNOWN;
            /* fall through */
            roll: case ROLL:
            /* fall through */
        default:
            if (!cansee(bhitpos.x, bhitpos.y))
                curs_on_u();
            tmp_at(DISP_FLASH, obj_to_glyph(singleobj));
            tmp_at(bhitpos.x, bhitpos.y);
    }

    /* Set the object in motion */
    while (dist-- > 0 && !used_up) {
        struct trap *t;
        tmp_at(bhitpos.x, bhitpos.y);

        bhitpos.x += dx;
        bhitpos.y += dy;
        t = t_at(bhitpos.x, bhitpos.y);

        if ((mtmp = m_at(bhitpos.x, bhitpos.y)) != 0) {
            if (otyp == BOULDER && throws_rocks(mtmp->data)) {
                if (rn2(3)) {
                    message_monster(MSG_M_SNATCHES_THE_BOULDER, mtmp);
                    singleobj->otrapped = 0;
                    (void)mpickobj(mtmp, singleobj);
                    used_up = true;
                    break;
                }
            }
            if (ohitmon(mtmp, singleobj, (style == ROLL) ? -1 : dist, false)) {
                used_up = true;
                break;
            }
        } else if (bhitpos.x == u.ux && bhitpos.y == u.uy) {
            if (multi)
                nomul(0);
            if (thitu(9 + singleobj->spe, dmgval(singleobj, &youmonst), singleobj, (char *)0))
                stop_occupation();
        }
        if (style == ROLL) {
            if (down_gate(bhitpos.x, bhitpos.y) != -1) {
                if (ship_object(singleobj, bhitpos.x, bhitpos.y, false)) {
                    used_up = true;
                    break;
                }
            }
            if (t && otyp == BOULDER) {
                switch (t->ttyp) {
                    case LANDMINE:
                        if (rn2(10) > 2) {
                            pline("KAABLAMM!!!%s",
                            cansee(bhitpos.x, bhitpos.y) ? " The rolling boulder triggers a land mine." : "");
                            deltrap(t);
                            del_engr_at(bhitpos.x, bhitpos.y);
                            place_object(singleobj, bhitpos.x, bhitpos.y);
                            singleobj->otrapped = 0;
                            fracture_rock(singleobj);
                            (void)scatter(bhitpos.x, bhitpos.y, 4,
                            MAY_DESTROY | MAY_HIT | MAY_FRACTURE | VIS_EFFECTS, (struct obj *)0);
                            if (cansee(bhitpos.x, bhitpos.y))
                                newsym(bhitpos.x, bhitpos.y);
                            used_up = true;
                        }
                        break;
                    case LEVEL_TELEP:
                    case TELEP_TRAP:
                        if (cansee(bhitpos.x, bhitpos.y))
                            pline("Suddenly the rolling boulder disappears!");
                        else
                            You_hear("a rumbling stop abruptly.");
                        singleobj->otrapped = 0;
                        if (t->ttyp == TELEP_TRAP)
                            rloco(singleobj);
                        else {
                            int newlev = random_teleport_level();
                            d_level dest;

                            if (newlev == depth(&u.uz) || In_endgame(&u.uz))
                                continue;
                            add_to_migration(singleobj);
                            get_level(&dest, newlev);
                            singleobj->ox = dest.dnum;
                            singleobj->oy = dest.dlevel;
                            singleobj->owornmask = (long)MIGR_RANDOM;
                        }
                        seetrap(t);
                        used_up = true;
                        break;
                    case PIT:
                    case SPIKED_PIT:
                    case HOLE:
                    case TRAPDOOR:
                        /* the boulder won't be used up if there is a
                         monster in the trap; stop rolling anyway */
                        x2 = bhitpos.x, y2 = bhitpos.y; /* stops here */
                        if (flooreffects(singleobj, x2, y2, "fall"))
                            used_up = true;
                        dist = -1; /* stop rolling immediately */
                        break;
                }
                if (used_up || dist == -1)
                    break;
            }
            if (flooreffects(singleobj, bhitpos.x, bhitpos.y, "fall")) {
                used_up = true;
                break;
            }
            if (otyp == BOULDER && (otmp2 = sobj_at(BOULDER, bhitpos.x, bhitpos.y)) != 0) {
                const char *bmsg = " as one boulder sets another in motion";

                if (!isok(bhitpos.x + dx, bhitpos.y + dy) || !dist || IS_ROCK(levl[bhitpos.x + dx][bhitpos.y + dy].typ))
                    bmsg = " as one boulder hits another";

                You_hear("a loud crash%s!",
                cansee(bhitpos.x, bhitpos.y) ? bmsg : "");
                obj_extract_self(otmp2);
                /* pass off the otrapped flag to the next boulder */
                otmp2->otrapped = singleobj->otrapped;
                singleobj->otrapped = 0;
                place_object(singleobj, bhitpos.x, bhitpos.y);
                singleobj = otmp2;
                otmp2 = (struct obj *)0;
                wake_nearto(bhitpos.x, bhitpos.y, 10 * 10);
            }
        }
        if (otyp == BOULDER && closed_door(bhitpos.x, bhitpos.y)) {
            if (cansee(bhitpos.x, bhitpos.y))
                pline_The("boulder crashes through a door.");
            levl[bhitpos.x][bhitpos.y].doormask = D_BROKEN;
            if (dist)
                unblock_point(bhitpos.x, bhitpos.y);
        }

        /* if about to hit iron bars, do so now */
        if (dist > 0 && isok(bhitpos.x + dx, bhitpos.y + dy) &&
        levl[bhitpos.x + dx][bhitpos.y + dy].typ == IRONBARS) {
            x2 = bhitpos.x, y2 = bhitpos.y; /* object stops here */
            if (hits_bars(&singleobj, x2, y2, !rn2(20), 0)) {
                if (!singleobj)
                    used_up = true;
                break;
            }
        }
    }
    tmp_at(DISP_END, 0);
    if (!used_up) {
        singleobj->otrapped = 0;
        place_object(singleobj, x2, y2);
        newsym(x2, y2);
        return 1;
    } else
        return 2;
}

void seetrap(struct trap *trap) {
    if (!trap->tseen) {
        trap->tseen = 1;
        newsym(trap->tx, trap->ty);
    }
}

int mintrap(struct monst *mtmp) {
    struct trap *trap = t_at(mtmp->mx, mtmp->my);
    bool trapkilled = false;
    struct permonst *mptr = mtmp->data;
    struct obj *otmp;

    if (!trap) {
        mtmp->mtrapped = 0; /* perhaps teleported? */
    } else if (mtmp->mtrapped) { /* is currently in the trap */
        if (!trap->tseen && cansee(mtmp->mx, mtmp->my) && canseemon(mtmp) && (trap->ttyp == SPIKED_PIT || trap->ttyp == BEAR_TRAP || trap->ttyp == HOLE || trap->ttyp == PIT || trap->ttyp == WEB)) {
            /* If you come upon an obviously trapped monster, then
             * you must be able to see the trap it's in too.
             */
            seetrap(trap);
        }

        if (!rn2(40)) {
            if (sobj_at(BOULDER, mtmp->mx, mtmp->my) && (trap->ttyp == PIT || trap->ttyp == SPIKED_PIT)) {
                if (!rn2(2)) {
                    mtmp->mtrapped = 0;
                    if (canseemon(mtmp)) {
                        message_monster(MSG_M_PULLS_FREE, mtmp);
                    }
                    fill_pit(mtmp->mx, mtmp->my);
                }
            } else {
                mtmp->mtrapped = 0;
            }
        } else if (metallivorous(mptr)) {
            if (trap->ttyp == BEAR_TRAP) {
                if (canseemon(mtmp)) {
                    message_monster(MSG_M_EATS_A_BEAR_TRAP, mtmp);
                }
                deltrap(trap);
                mtmp->meating = 5;
                mtmp->mtrapped = 0;
            } else if (trap->ttyp == SPIKED_PIT) {
                if (canseemon(mtmp)) {
                    message_monster(MSG_M_MUNCHES_ON_SOME_SPIKES, mtmp);
                }
                trap->ttyp = PIT;
                mtmp->meating = 5;
            }
        }
    } else {
        int tt = trap->ttyp;
        bool in_sight, tear_web, see_it, inescapable = ((tt == HOLE || tt == PIT) && In_sokoban(&u.uz) && !trap->madeby_u);
        const char *fallverb;

        /* true when called from dotrap, inescapable is not an option */
        if (mtmp == u.usteed)
            inescapable = true;
        if (!inescapable && ((mtmp->mtrapseen & (1 << (tt - 1))) != 0 || (tt == HOLE && !mindless(mtmp->data)))) {
            /* it has been in such a trap - perhaps it escapes */
            if (rn2(4))
                return (0);
        } else {
            mtmp->mtrapseen |= (1 << (tt - 1));
        }
        /* Monster is aggravated by being trapped by you.
         Recognizing who made the trap isn't completely
         unreasonable; everybody has their own style. */
        if (trap->madeby_u && rnl(5))
            setmangry(mtmp);

        in_sight = canseemon(mtmp);
        see_it = cansee(mtmp->mx, mtmp->my);
        /* assume hero can tell what's going on for the steed */
        if (mtmp == u.usteed)
            in_sight = true;
        switch (tt) {
            case ARROW_TRAP:
                if (trap->once && trap->tseen && !rn2(15)) {
                    if (in_sight && see_it) {
                        message_monster(MSG_M_TRIGGERS_TRAP_BUT_NOTHING_HAPPENS, mtmp);
                    }
                    deltrap(trap);
                    newsym(mtmp->mx, mtmp->my);
                    break;
                }
                trap->once = 1;
                otmp = mksobj(ARROW, true, false);
                otmp->quan = 1L;
                otmp->owt = weight(otmp);
                otmp->opoisoned = 0;
                if (in_sight)
                    seetrap(trap);
                if (thitm(8, mtmp, otmp, 0, false))
                    trapkilled = true;
                break;
            case DART_TRAP:
                if (trap->once && trap->tseen && !rn2(15)) {
                    if (in_sight && see_it) {
                        message_monster(MSG_M_TRIGGERS_TRAP_BUT_NOTHING_HAPPENS, mtmp);
                    }
                    deltrap(trap);
                    newsym(mtmp->mx, mtmp->my);
                    break;
                }
                trap->once = 1;
                otmp = mksobj(DART, true, false);
                otmp->quan = 1L;
                otmp->owt = weight(otmp);
                if (!rn2(6))
                    otmp->opoisoned = 1;
                if (in_sight)
                    seetrap(trap);
                if (thitm(7, mtmp, otmp, 0, false))
                    trapkilled = true;
                break;
            case ROCKTRAP:
                if (trap->once && trap->tseen && !rn2(15)) {
                    if (in_sight && see_it) {
                        message_monster(MSG_TRAP_DOOR_ABOVE_M_OPENS_NOTHING_FALLS_OUT, mtmp);
                    }
                    deltrap(trap);
                    newsym(mtmp->mx, mtmp->my);
                    break;
                }
                trap->once = 1;
                otmp = mksobj(ROCK, true, false);
                otmp->quan = 1L;
                otmp->owt = weight(otmp);
                if (in_sight)
                    seetrap(trap);
                if (thitm(0, mtmp, otmp, d(2, 6), false))
                    trapkilled = true;
                break;

            case SQKY_BOARD:
                if (is_flyer(mptr))
                    break;
                /* stepped on a squeaky board */
                if (in_sight) {
                    message_monster(MSG_BOARD_BENEATH_M_SQEAKS_LOUDLY, mtmp);
                    seetrap(trap);
                } else
                    You_hear("a distant squeak.");
                /* wake up nearby monsters */
                wake_nearto(mtmp->mx, mtmp->my, 40);
                break;

            case BEAR_TRAP:
                if (mptr->msize > MZ_SMALL && !amorphous(mptr) && !is_flyer(mptr) && !is_whirly(mptr) && !unsolid(mptr)) {
                    mtmp->mtrapped = 1;
                    if (in_sight) {
                        unsigned char made_by_you_flag = trap->madeby_u ? MSG_FLAG_MADE_BY_YOU : 0;
                        message_monster_flag(MSG_M_CAUGHT_IN_BEAR_TRAP, mtmp, made_by_you_flag);
                        seetrap(trap);
                    } else {
                        if ((mptr == &mons[PM_OWLBEAR] || mptr == &mons[PM_BUGBEAR]) && flags.soundok)
                            You_hear("the roaring of an angry bear!");
                    }
                }
                break;

            case SLP_GAS_TRAP:
                if (!resists_sleep(mtmp) && !breathless(mptr) && !mtmp->msleeping && mtmp->mcanmove) {
                    mtmp->mcanmove = 0;
                    mtmp->mfrozen = rnd(25);
                    if (in_sight) {
                        message_monster(MSG_M_SUDDENLY_FALLS_ASLEEP, mtmp);
                        seetrap(trap);
                    }
                }
                break;

            case RUST_TRAP: {
                struct obj *target;

                if (in_sight)
                    seetrap(trap);
                switch (rn2(5)) {
                    case 0:
                        if (in_sight) {
                            message_monster(MSG_GUSH_OF_WATER_HITS_M_HEAD, mtmp);
                        }
                        target = which_armor(mtmp, W_ARMH);
                        (void)rust_dmg(target, "helmet", 1, true, mtmp);
                        break;
                    case 1:
                        if (in_sight) {
                            message_monster(MSG_GUSH_OF_WATER_HITS_M_LEFT_ARM, mtmp);
                        }
                        target = which_armor(mtmp, W_ARMS);
                        if (rust_dmg(target, "shield", 1, true, mtmp))
                            break;
                        target = MON_WEP(mtmp);
                        if (target && bimanual(target))
                            erode_obj(target, false, true);
                        glovecheck: target = which_armor(mtmp, W_ARMG);
                        (void)rust_dmg(target, "gauntlets", 1, true, mtmp);
                        break;
                    case 2:
                        if (in_sight) {
                            message_monster(MSG_GUSH_OF_WATER_HITS_M_RIGHT_ARM, mtmp);
                        }
                        erode_obj(MON_WEP(mtmp), false, true);
                        goto glovecheck;
                    default:
                        if (in_sight) {
                            message_monster(MSG_GUSH_OF_WATER_HITS_M, mtmp);
                        }
                        for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj)
                            (void)snuff_lit(otmp);
                        target = which_armor(mtmp, W_ARMC);
                        if (target)
                            (void)rust_dmg(target, cloak_simple_name(target), 1, true, mtmp);
                        else {
                            target = which_armor(mtmp, W_ARM);
                            if (target)
                                (void)rust_dmg(target, "armor", 1, true, mtmp);
                            else {
                                target = which_armor(mtmp, W_ARMU);
                                (void)rust_dmg(target, "shirt", 1, true, mtmp);
                            }
                        }
                }
                if (mptr == &mons[PM_IRON_GOLEM]) {
                    if (in_sight) {
                        message_monster(MSG_M_FALLS_TO_PIECES, mtmp);
                    } else if (mtmp->mtame) {
                        message_monster(MSG_MAY_M_RUST_IN_PEACE, mtmp);
                    }
                    mondied(mtmp);
                    if (mtmp->mhp <= 0)
                        trapkilled = true;
                } else if (mptr == &mons[PM_GREMLIN] && rn2(3)) {
                    (void)split_mon(mtmp, (struct monst *)0);
                }
                break;
            }
            case FIRE_TRAP:
                mfiretrap:
                            if (in_sight) {
                                message_monster_string(MSG_TOWER_OF_FLAME_ERUPTS_FROM_S_UNDER_M,
                                        mtmp, surface(mtmp->mx, mtmp->my));
                            } else if (see_it) {
                                /* evidently `mtmp' is invisible */
                                message_string(MSG_YOU_SEE_TOWER_OF_FLAME_ERUPT_FROM_S,
                                    surface(mtmp->mx, mtmp->my));
                            }

                if (resists_fire(mtmp)) {
                    if (in_sight) {
                        shieldeff(mtmp->mx, mtmp->my);
                        message_monster(MSG_M_IS_UNINJURED, mtmp);
                    }
                } else {
                    int num = d(2, 4), alt;
                    bool immolate = false;

                    /* paper burns very fast, assume straw is tightly
                     * packed and burns a bit slower */
                    switch (monsndx(mtmp->data)) {
                        case PM_PAPER_GOLEM:
                            immolate = true;
                            alt = mtmp->mhpmax;
                            break;
                        case PM_STRAW_GOLEM:
                            alt = mtmp->mhpmax / 2;
                            break;
                        case PM_WOOD_GOLEM:
                            alt = mtmp->mhpmax / 4;
                            break;
                        case PM_LEATHER_GOLEM:
                            alt = mtmp->mhpmax / 8;
                            break;
                        default:
                            alt = 0;
                            break;
                    }
                    if (alt > num)
                        num = alt;

                    if (thitm(0, mtmp, (struct obj *)0, num, immolate))
                        trapkilled = true;
                    else
                        /* we know mhp is at least `num' below mhpmax,
                         so no (mhp > mhpmax) check is needed here */
                        mtmp->mhpmax -= rn2(num + 1);
                }
                if (burnarmor(mtmp) || rn2(3)) {
                    (void)destroy_mitem(mtmp, SCROLL_CLASS, AD_FIRE);
                    (void)destroy_mitem(mtmp, SPBOOK_CLASS, AD_FIRE);
                    (void)destroy_mitem(mtmp, POTION_CLASS, AD_FIRE);
                }
                if (burn_floor_paper(mtmp->mx, mtmp->my, see_it, false) && !see_it && distu(mtmp->mx, mtmp->my) <= 3 * 3)
                    You("smell smoke.");
                if (is_ice(mtmp->mx, mtmp->my))
                    melt_ice(mtmp->mx, mtmp->my);
                if (see_it)
                    seetrap(trap);
                break;

            case PIT:
            case SPIKED_PIT:
                fallverb = "falls";
                if (is_flyer(mptr) || is_floater(mptr) || (mtmp->wormno && count_wsegs(mtmp) > 5) || is_clinger(mptr)) {
                    if (!inescapable)
                        break; /* avoids trap */
                    fallverb = "is dragged"; /* sokoban pit */
                }
                if (!passes_walls(mptr))
                    mtmp->mtrapped = 1;
                if (in_sight) {
                    pline("%s %s into %s pit!", "TODO:Monnam(mtmp)", fallverb, a_your[trap->madeby_u]);
                    if (mptr == &mons[PM_PIT_VIPER] || mptr == &mons[PM_PIT_FIEND])
                        pline("How pitiful.  Isn't that the pits?");
                    seetrap(trap);
                }
                mselftouch(mtmp, "Falling, ", false);
                if (mtmp->mhp <= 0 || thitm(0, mtmp, (struct obj *)0, rnd((tt == PIT) ? 6 : 10), false))
                    trapkilled = true;
                break;
            case HOLE:
            case TRAPDOOR:
                if (!Can_fall_thru(&u.uz)) {
                    impossible("mintrap: %ss cannot exist on this level.", defsyms[trap_to_defsym(tt)].explanation);
                    break; /* don't activate it after all */
                }
                if (is_flyer(mptr) || is_floater(mptr) || mptr == &mons[PM_WUMPUS] || (mtmp->wormno && count_wsegs(mtmp) > 5) || mptr->msize >= MZ_HUGE) {
                    if (inescapable) { /* sokoban hole */
                        if (in_sight) {
                            message_monster(MSG_M_SEEMS_TO_BE_YANKED_DOWN, mtmp);
                            /* suppress message in mlevel_tele_trap() */
                            in_sight = false;
                            seetrap(trap);
                        }
                    } else
                        break;
                }
                /* Fall through */
            case LEVEL_TELEP:
            case MAGIC_PORTAL: {
                int mlev_res;
                mlev_res = mlevel_tele_trap(mtmp, trap, inescapable, in_sight);
                if (mlev_res)
                    return (mlev_res);
            }
                break;

            case TELEP_TRAP:
                mtele_trap(mtmp, trap, in_sight);
                break;

            case WEB:
                /* Monster in a web. */
                if (webmaker(mptr))
                    break;
                if (amorphous(mptr) || is_whirly(mptr) || unsolid(mptr)) {
                    if (acidic(mptr) || mptr == &mons[PM_GELATINOUS_CUBE] || mptr == &mons[PM_FIRE_ELEMENTAL]) {
                        if (in_sight) {
                            pline("%s %s %s spider web!", "TODO:Monnam(mtmp)", (mptr == &mons[PM_FIRE_ELEMENTAL]) ? "burns" : "dissolves", a_your[trap->madeby_u]);
                        }
                        deltrap(trap);
                        newsym(mtmp->mx, mtmp->my);
                        break;
                    }
                    if (in_sight) {
                        pline("%s flows through %s spider web.", "TODO:Monnam(mtmp)", a_your[trap->madeby_u]);
                        seetrap(trap);
                    }
                    break;
                }
                tear_web = false;
                switch (monsndx(mptr)) {
                    case PM_OWLBEAR: /* Eric Backus */
                    case PM_BUGBEAR:
                        if (!in_sight) {
                            You_hear("the roaring of a confused bear!");
                            mtmp->mtrapped = 1;
                            break;
                        }
                        /* fall though */
                    default:
                        if (mptr->mlet == S_GIANT || (mptr->mlet == S_DRAGON && extra_nasty(mptr)) || /* excl. babies */
                        (mtmp->wormno && count_wsegs(mtmp) > 5)) {
                            tear_web = true;
                        } else if (in_sight) {
                            pline("%s is caught in %s spider web.", "TODO:Monnam(mtmp)", a_your[trap->madeby_u]);
                            seetrap(trap);
                        }
                        mtmp->mtrapped = tear_web ? 0 : 1;
                        break;
                        /* this list is fairly arbitrary; it deliberately
                         excludes wumpus & giant/ettin zombies/mummies */
                    case PM_TITANOTHERE:
                    case PM_BALUCHITHERIUM:
                    case PM_PURPLE_WORM:
                    case PM_JABBERWOCK:
                    case PM_IRON_GOLEM:
                    case PM_BALROG:
                    case PM_KRAKEN:
                    case PM_MASTODON:
                        tear_web = true;
                        break;
                }
                if (tear_web) {
                    if (in_sight)
                        pline("%s tears through %s spider web!", "TODO:Monnam(mtmp)", a_your[trap->madeby_u]);
                    deltrap(trap);
                    newsym(mtmp->mx, mtmp->my);
                }
                break;

            case STATUE_TRAP:
                break;

            case MAGIC_TRAP:
                /* A magic trap.  Monsters usually immune. */
                if (!rn2(21))
                    goto mfiretrap;
                break;
            case ANTI_MAGIC:
                break;

            case LANDMINE:
                if (rn2(3))
                    break; /* monsters usually don't set it off */
                if (is_flyer(mptr)) {
                    bool already_seen = trap->tseen;
                    if (in_sight && !already_seen) {
                        message_monster(MSG_TRIGGER_APPEARS_IN_SOIL_BELOW_M, mtmp);
                        seetrap(trap);
                    }
                    if (rn2(3))
                        break;
                    if (in_sight) {
                        newsym(mtmp->mx, mtmp->my);
                        pline_The("air currents set %s off!", already_seen ? "a land mine" : "it");
                    }
                } else if (in_sight) {
                    newsym(mtmp->mx, mtmp->my);
                    pline("KAABLAMM!!!  %s triggers %s land mine!", "TODO:Monnam(mtmp)", a_your[trap->madeby_u]);
                }
                if (!in_sight)
                    pline("Kaablamm!  You hear an explosion in the distance!");
                blow_up_landmine(trap);
                if (thitm(0, mtmp, (struct obj *)0, rnd(16), false))
                    trapkilled = true;
                else {
                    /* monsters recursively fall into new pit */
                    if (mintrap(mtmp) == 2)
                        trapkilled = true;
                }
                /* a boulder may fill the new pit, crushing monster */
                fill_pit(trap->tx, trap->ty);
                if (mtmp->mhp <= 0)
                    trapkilled = true;
                if (unconscious()) {
                    multi = -1;
                    nomovemsg = "The explosion awakens you!";
                }
                break;

            case POLY_TRAP:
                if (resists_magm(mtmp)) {
                    shieldeff(mtmp->mx, mtmp->my);
                } else if (!resist(mtmp, WAND_CLASS, 0, NOTELL)) {
                    (void)newcham(mtmp, (struct permonst *)0,
                    false, false);
                    if (in_sight)
                        seetrap(trap);
                }
                break;

            case ROLLING_BOULDER_TRAP:
                if (!is_flyer(mptr)) {
                    int style = ROLL | (in_sight ? 0 : LAUNCH_UNSEEN);

                    newsym(mtmp->mx, mtmp->my);
                    if (in_sight)
                        pline("Click! %s triggers %s.", "TODO:Monnam(mtmp)", trap->tseen ? "a rolling boulder trap" : something);
                    if (launch_obj(BOULDER, trap->launch.x, trap->launch.y, trap->launch2.x, trap->launch2.y, style)) {
                        if (in_sight)
                            trap->tseen = true;
                        if (mtmp->mhp <= 0)
                            trapkilled = true;
                    } else {
                        deltrap(trap);
                        newsym(mtmp->mx, mtmp->my);
                    }
                }
                break;

            default:
                impossible("Some monster encountered a strange trap of type %d.", tt);
        }
    }
    if (trapkilled)
        return 2;
    return mtmp->mtrapped;
}

/* Combine cockatrice checks into single functions to avoid repeating code. */
void instapetrify(const char *str) {
    if (Stone_resistance())
        return;
    if (poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM))
        return;
    You("turn to stone...");
    fprintf(stderr, "TODO: killer = %s\n", str);
    done(STONING);
}

void minstapetrify(struct monst *mon, bool byplayer) {
    if (resists_ston(mon))
        return;
    if (poly_when_stoned(mon->data)) {
        mon_to_stone(mon);
        return;
    }

    /* give a "<mon> is slowing down" message and also remove
     intrinsic speed (comparable to similar effect on the hero) */
    mon_adjust_speed(mon, -3, (struct obj *)0);

    if (cansee(mon->mx, mon->my)) {
        message_monster(MSG_M_TURNS_TO_STONE, mon);
    }
    if (byplayer) {
        stoned = true;
        xkilled(mon, 0);
    } else
        monstone(mon);
}

void selftouch(const char *arg) {
    char kbuf[BUFSZ];

    if (uwep && uwep->otyp == CORPSE && touch_petrifies(&mons[uwep->corpsenm]) && !Stone_resistance()) {
        pline("%s touch the %s corpse.", arg, mons[uwep->corpsenm].mname);
        sprintf(kbuf, "%s corpse", an(mons[uwep->corpsenm].mname));
        instapetrify(kbuf);
    }
    /* Or your secondary weapon, if wielded */
    if (u.twoweap && uswapwep && uswapwep->otyp == CORPSE && touch_petrifies(&mons[uswapwep->corpsenm]) && !Stone_resistance()) {
        pline("%s touch the %s corpse.", arg, mons[uswapwep->corpsenm].mname);
        sprintf(kbuf, "%s corpse", an(mons[uswapwep->corpsenm].mname));
        instapetrify(kbuf);
    }
}

void mselftouch(struct monst *mon, const char *arg, bool byplayer) {
    struct obj *mwep = MON_WEP(mon);

    if (mwep && mwep->otyp == CORPSE && touch_petrifies(&mons[mwep->corpsenm])) {
        if (cansee(mon->mx, mon->my)) {
            pline("%s%s touches the %s corpse.", arg ? arg : "",
                    arg ? "TODO:mon_nam(mon)" : "TODO:Monnam(mon)",
                    mons[mwep->corpsenm].mname);
        }
        minstapetrify(mon, byplayer);
    }
}

void float_up(void) {
    if (u.utrap) {
        if (u.utraptype == TT_PIT) {
            u.utrap = 0;
            You("float up, out of the pit!");
            vision_full_recalc = 1; /* vision limits change */
            fill_pit(u.ux, u.uy);
        } else if (u.utraptype == TT_INFLOOR) {
            Your("body pulls upward, but your %s are still stuck.", makeplural(body_part(LEG)));
        } else {
            You("float up, only your %s is still stuck.", body_part(LEG));
        }
    } else if (Is_waterlevel(&u.uz))
        pline("It feels as though you've lost some weight.");
    else if (u.uinwater)
        spoteffects(true);
    else if (u.uswallow)
        You(is_animal(u.ustuck->data) ? "float away from the %s." : "spiral up into %s.",
                is_animal(u.ustuck->data) ? surface(u.ux, u.uy) : "TODO:mon_nam(u.ustuck)");
    else if (Hallucination())
        pline("Up, up, and awaaaay!  You're walking on air!");
    else if (Is_airlevel(&u.uz))
        You("gain control over your movements.");
    else
        You("start to float in the air!");
    if (u.usteed && !is_floater(u.usteed->data) && !is_flyer(u.usteed->data)) {
        if (Lev_at_will) {
            message_monster(MSG_M_MAGICALLY_FLOATS_UP, u.usteed);
        } else {
            message_monster(MSG_YOU_CANNOT_STAY_ON_M, u.usteed);
            dismount_steed(DISMOUNT_GENERIC);
        }
    }
    return;
}

void fill_pit(int x, int y) {
    struct obj *otmp;
    struct trap *t;

    if ((t = t_at(x, y)) && ((t->ttyp == PIT) || (t->ttyp == SPIKED_PIT)) && (otmp = sobj_at(BOULDER, x, y))) {
        obj_extract_self(otmp);
        (void)flooreffects(otmp, x, y, "settle");
    }
}

/* emask: might cancel timeout */
int float_down(long hmask, long emask) {
    struct trap *trap = (struct trap *)0;
    d_level current_dungeon_level;
    bool no_msg = false;

    HLevitation&= ~hmask;
    ELevitation&= ~emask;
    if (Levitation)
        return (0); /* maybe another ring/potion/boots */
    if (u.uswallow) {
        You("float down, but you are still %s.", is_animal(u.ustuck->data) ? "swallowed" : "engulfed");
        return (1);
    }

    if (Punished && !carried(uball) && (is_pool(uball->ox, uball->oy) || ((trap = t_at(uball->ox, uball->oy)) && ((trap->ttyp == PIT) || (trap->ttyp == SPIKED_PIT) || (trap->ttyp == TRAPDOOR) || (trap->ttyp == HOLE))))) {
        u.ux0 = u.ux;
        u.uy0 = u.uy;
        u.ux = uball->ox;
        u.uy = uball->oy;
        movobj(uchain, uball->ox, uball->oy);
        newsym(u.ux0, u.uy0);
        vision_full_recalc = 1; /* in case the hero moved. */
    }
    /* check for falling into pool - added by GAN 10/20/86 */
    if (!Flying) {
        if (!u.uswallow && u.ustuck) {
            if (sticks(youmonst.data)) {
                message_monster(MSG_YOU_ARE_NOT_ABLE_TO_HOLD, u.ustuck);
            } else {
                message_monster(MSG_M_CAN_NO_LONGER_HOLD_YOU, u.ustuck);
            }
            u.ustuck = 0;
        }
        /* kludge alert:
         * drown() and lava_effects() print various messages almost
         * every time they're called which conflict with the "fall
         * into" message below.  Thus, we want to avoid printing
         * confusing, duplicate or out-of-order messages.
         * Use knowledge of the two routines as a hack -- this
         * should really be handled differently -dlc
         */
        if (is_pool(u.ux, u.uy) && !Wwalking && !Swimming && !u.uinwater)
            no_msg = drown();

        if (is_lava(u.ux, u.uy)) {
            (void)lava_effects();
            no_msg = true;
        }
    }
    if (!trap) {
        trap = t_at(u.ux, u.uy);
        if (Is_airlevel(&u.uz))
            You("begin to tumble in place.");
        else if (Is_waterlevel(&u.uz) && !no_msg)
            You_feel("heavier.");
        /* u.uinwater msgs already in spoteffects()/drown() */
        else if (!u.uinwater && !no_msg) {
            if (!(emask & W_SADDLE)) {
                bool sokoban_trap = (In_sokoban(&u.uz) && trap);
                if (Hallucination())
                    pline("Bummer!  You've %s.", is_pool(u.ux, u.uy) ? "splashed down" : sokoban_trap ? "crashed" : "hit the ground");
                else {
                    if (!sokoban_trap)
                        You("float gently to the %s.", surface(u.ux, u.uy));
                    else {
                        /* Justification elsewhere for Sokoban traps
                         * is based on air currents. This is
                         * consistent with that.
                         * The unexpected additional force of the
                         * air currents once leviation
                         * ceases knocks you off your feet.
                         */
                        You("fall over.");
                        losehp(rnd(2), killed_by_const(KM_DANGEROUS_WINDS));
                        if (u.usteed)
                            dismount_steed(DISMOUNT_FELL);
                        selftouch("As you fall, you");
                    }
                }
            }
        }
    }

    /* can't rely on u.uz0 for detecting trap door-induced level change;
     it gets changed to reflect the new level before we can check it */
    assign_level(&current_dungeon_level, &u.uz);

    if (trap)
        switch (trap->ttyp) {
            case STATUE_TRAP:
                break;
            case HOLE:
            case TRAPDOOR:
                if (!Can_fall_thru(&u.uz) || u.ustuck)
                    break;
                /* fall into next case */
            default:
                if (!u.utrap) /* not already in the trap */
                    dotrap(trap, 0);
        }

    if (!Is_airlevel(&u.uz) && !Is_waterlevel(&u.uz) && !u.uswallow &&
    /* falling through trap door calls goto_level,
     and goto_level does its own pickup() call */
    on_level(&u.uz, &current_dungeon_level))
        (void)pickup(1);
    return 1;
}

/*
 * Scrolls, spellbooks, potions, and flammable items
 * may get affected by the fire.
 *
 * Return number of objects destroyed. --ALI
 */
int fire_damage(struct obj *chain, bool force, bool here, signed char x, signed char y) {
    int chance;
    struct obj *obj, *otmp, *nobj, *ncobj;
    int retval = 0;
    int in_sight = !Blind && couldsee(x, y); /* Don't care if it's lit */
    int dindx;

    for (obj = chain; obj; obj = nobj) {
        nobj = here ? obj->nexthere : obj->nobj;

        /* object might light in a controlled manner */
        if (catch_lit(obj))
            continue;

        if (Is_container(obj)) {
            switch (obj->otyp) {
                case ICE_BOX:
                    continue; /* Immune */
                    /*NOTREACHED*/
                    break;
                case CHEST:
                    chance = 40;
                    break;
                case LARGE_BOX:
                    chance = 30;
                    break;
                default:
                    chance = 20;
                    break;
            }
            if (!force && (Luck + 5) > rn2(chance))
                continue;
            /* Container is burnt up - dump contents out */
            if (in_sight)
                pline("%s catches fire and burns.", Yname2(obj));
            if (Has_contents(obj)) {
                if (in_sight)
                    pline("Its contents fall out.");
                for (otmp = obj->cobj; otmp; otmp = ncobj) {
                    ncobj = otmp->nobj;
                    obj_extract_self(otmp);
                    if (!flooreffects(otmp, x, y, ""))
                        place_object(otmp, x, y);
                }
            }
            delobj(obj);
            retval++;
        } else if (!force && (Luck + 5) > rn2(20)) {
            /*  chance per item of sustaining damage:
             *  max luck (full moon):    5%
             *  max luck (elsewhen):    10%
             *  avg luck (Luck==0):     75%
             *  awful luck (Luck<-4):  100%
             */
            continue;
        } else if (obj->oclass == SCROLL_CLASS || obj->oclass == SPBOOK_CLASS) {
            if (obj->otyp == SCR_FIRE || obj->otyp == SPE_FIREBALL)
                continue;
            if (obj->otyp == SPE_BOOK_OF_THE_DEAD) {
                if (in_sight)
                    pline("Smoke rises from %s.", the(xname(obj)));
                continue;
            }
            dindx = (obj->oclass == SCROLL_CLASS) ? 2 : 3;
            if (in_sight)
                pline("%s %s.", Yname2(obj), (obj->quan > 1) ? destroy_strings[dindx * 3 + 1] : destroy_strings[dindx * 3]);
            delobj(obj);
            retval++;
        } else if (obj->oclass == POTION_CLASS) {
            dindx = 1;
            if (in_sight)
                pline("%s %s.", Yname2(obj), (obj->quan > 1) ? destroy_strings[dindx * 3 + 1] : destroy_strings[dindx * 3]);
            delobj(obj);
            retval++;
        } else if (is_flammable(obj) && obj->oeroded < MAX_ERODE && !(obj->oerodeproof || (obj->blessed && !rnl(4)))) {
            if (in_sight) {
                pline("%s %s%s.", Yname2(obj),
                        "TODO:otense(obj, \"burn\")",
                        obj->oeroded + 1 == MAX_ERODE ? " completely" :
                            obj->oeroded ? " further" : "");
            }
            obj->oeroded++;
        }
    }

    if (retval && !in_sight)
        You("smell smoke.");
    return retval;
}

void water_damage(struct obj *obj, bool force, bool here) {
    struct obj *otmp;

    /* Scrolls, spellbooks, potions, weapons and
     pieces of armor may get affected by the water */
    for (; obj; obj = otmp) {
        otmp = here ? obj->nexthere : obj->nobj;

        (void)snuff_lit(obj);

        if (obj->otyp == CAN_OF_GREASE && obj->spe > 0) {
            continue;
        } else if (obj->greased) {
            if (force || !rn2(2))
                obj->greased = 0;
        } else if (Is_container(obj) && !Is_box(obj) && (obj->otyp != OILSKIN_SACK || (obj->cursed && !rn2(3)))) {
            water_damage(obj->cobj, force, false);
        } else if (!force && (Luck + 5) > rn2(20)) {
            /*  chance per item of sustaining damage:
             *      max luck (full moon):    5%
             *      max luck (elsewhen):    10%
             *      avg luck (Luck==0):     75%
             *      awful luck (Luck<-4):  100%
             */
            continue;
        } else if (obj->oclass == SCROLL_CLASS) {
            if (obj->otyp != SCR_MAIL) {
                obj->otyp = SCR_BLANK_PAPER;
                obj->spe = 0;
            }
        } else if (obj->oclass == SPBOOK_CLASS) {
            if (obj->otyp == SPE_BOOK_OF_THE_DEAD)
                pline("Steam rises from %s.", the(xname(obj)));
            else
                obj->otyp = SPE_BLANK_PAPER;
        } else if (obj->oclass == POTION_CLASS) {
            if (obj->otyp == POT_ACID) {
                /* damage player/monster? */
                pline("A potion explodes!");
                delobj(obj);
                continue;
            } else if (obj->odiluted) {
                obj->otyp = POT_WATER;
                obj->blessed = obj->cursed = 0;
                obj->odiluted = 0;
            } else if (obj->otyp != POT_WATER)
                obj->odiluted++;
        } else if (is_rustprone(obj) && obj->oeroded < MAX_ERODE && !(obj->oerodeproof || (obj->blessed && !rnl(4)))) {
            /* all metal stuff and armor except (body armor
             protected by oilskin cloak) */
            if (obj->oclass != ARMOR_CLASS || obj != uarm || !uarmc || uarmc->otyp != OILSKIN_CLOAK || (uarmc->cursed && !rn2(3)))
                obj->oeroded++;
        }
    }
}

/*
 * This function is potentially expensive - rolling
 * inventory list multiple times.  Luckily it's seldom needed.
 * Returns true if disrobing made player unencumbered enough to
 * crawl out of the current predicament.
 */
static bool emergency_disrobe(bool *lostsome) {
    int invc = inv_cnt();

    while (near_capacity() > (Punished ? UNENCUMBERED : SLT_ENCUMBER)) {
        struct obj *obj, *otmp = (struct obj *)0;
        int i;

        /* Pick a random object */
        if (invc > 0) {
            i = rn2(invc);
            for (obj = invent; obj; obj = obj->nobj) {
                /*
                 * Undroppables are: body armor, boots, gloves,
                 * amulets, and rings because of the time and effort
                 * in removing them + loadstone and other cursed stuff
                 * for obvious reasons.
                 */
                if (!((obj->otyp == LOADSTONE && obj->cursed) || obj == uamul || obj == uleft || obj == uright || obj == ublindf || obj == uarm || obj == uarmc || obj == uarmg || obj == uarmf || obj == uarmu || (obj->cursed && (obj == uarmh || obj == uarms)) || welded(obj)))
                    otmp = obj;
                /* reached the mark and found some stuff to drop? */
                if (--i < 0 && otmp)
                    break;

                /* else continue */
            }
        }
        if (!otmp) {
            /* Nothing available left to drop; try gold */
            if (u.ugold) {
                pline("In desperation, you drop your purse.");
                /* Hack: gold is not in the inventory, so make a gold object
                 * and put it at the head of the inventory list.
                 */
                obj = mkgoldobj(u.ugold); /* removes from u.ugold */
                obj->in_use = true;
                u.ugold = obj->quan; /* put the gold back */
                assigninvlet(obj); /* might end up as NOINVSYM */
                obj->nobj = invent;
                invent = obj;
                *lostsome = true;
                dropx(obj);
                continue; /* Try again */
            }
            /* We can't even drop gold! */
            return (false);
        }
        if (otmp->owornmask)
            remove_worn_item(otmp, false);
        *lostsome = true;
        dropx(otmp);
        invc--;
    }
    return (true);
}

/*
 *  return(true) == player relocated
 */
bool drown(void) {
    bool inpool_ok = false, crawl_ok;
    int i, x, y;

    /* happily wading in the same contiguous pool */
    if (u.uinwater && is_pool(u.ux - u.dx, u.uy - u.dy) && (Swimming || Amphibious)) {
        /* water effects on objects every now and then */
        if (!rn2(5))
            inpool_ok = true;
        else
            return (false);
    }

    if (!u.uinwater) {
        You("%s into the water%c",
        Is_waterlevel(&u.uz) ? "plunge" : "fall",
        Amphibious || Swimming ? '.' : '!');
        if (!Swimming && !Is_waterlevel(&u.uz))
            You("sink like %s.", Hallucination() ? "the Titanic" : "a rock");
    }

    water_damage(invent, false, false);

    if (u.umonnum == PM_GREMLIN && rn2(3))
        (void)split_mon(&youmonst, (struct monst *)0);
    else if (u.umonnum == PM_IRON_GOLEM) {
        You("rust!");
        i = d(2, 6);
        if (u.mhmax > i)
            u.mhmax -= i;
        losehp(i, killed_by_const(KM_RUSTING_AWAY));
    }
    if (inpool_ok)
        return (false);

    if ((i = number_leashed()) > 0) {
        pline_The("leash%s slip%s loose.", (i > 1) ? "es" : "", (i > 1) ? "" : "s");
        unleash_all();
    }

    if (Amphibious || Swimming) {
        if (Amphibious) {
            if (flags.verbose)
                pline("But you aren't drowning.");
            if (!Is_waterlevel(&u.uz)) {
                if (Hallucination())
                    Your("keel hits the bottom.");
                else
                    You("touch bottom.");
            }
        }
        if (Punished) {
            unplacebc();
            placebc();
        }
        vision_recalc(2); /* unsee old position */
        u.uinwater = 1;
        under_water(1);
        vision_full_recalc = 1;
        return (false);
    }
    if ((Teleportation || can_teleport(youmonst.data)) && !u.usleep && (Teleport_control || rn2(3) < Luck + 2)) {
        You("attempt a teleport spell."); /* utcsri!carroll */
        if (!level.flags.noteleport) {
            (void)dotele();
            if (!is_pool(u.ux, u.uy))
                return (true);
        } else
            pline_The("attempted teleport spell fails.");
    }
    if (u.usteed) {
        dismount_steed(DISMOUNT_GENERIC);
        if (!is_pool(u.ux, u.uy))
            return (true);
    }
    crawl_ok = false;
    x = y = 0; /* lint suppression */
    /* if sleeping, wake up now so that we don't crawl out of water
     while still asleep; we can't do that the same way that waking
     due to combat is handled; note unmul() clears u.usleep */
    if (u.usleep)
        unmul("Suddenly you wake up!");
    /* can't crawl if unable to move (crawl_ok flag stays false) */
    if (multi < 0 || (Upolyd && !youmonst.data->mmove))
        goto crawl;
    /* look around for a place to crawl to */
    for (i = 0; i < 100; i++) {
        x = rn1(3, u.ux - 1);
        y = rn1(3, u.uy - 1);
        if (goodpos(x, y, &youmonst, 0)) {
            crawl_ok = true;
            goto crawl;
        }
    }
    /* one more scan */
    for (x = u.ux - 1; x <= u.ux + 1; x++)
        for (y = u.uy - 1; y <= u.uy + 1; y++)
            if (goodpos(x, y, &youmonst, 0)) {
                crawl_ok = true;
                goto crawl;
            }
    crawl: if (crawl_ok) {
        bool lost = false;
        /* time to do some strip-tease... */
        bool succ = Is_waterlevel(&u.uz) ? true : emergency_disrobe(&lost);

        You("try to crawl out of the water.");
        if (lost)
            You("dump some of your gear to lose weight...");
        if (succ) {
            pline("Pheew!  That was close.");
            teleds(x, y, true);
            return (true);
        }
        /* still too much weight */
        pline("But in vain.");
    }
    u.uinwater = 1;
    You("drown.");
    fprintf(stderr, "TODO: killer = %s\n", (levl[u.ux][u.uy].typ == POOL || Is_medusa_level(&u.uz)) ? "pool of water" : "moat");
    done(DROWNING);
    /* oops, we're still alive.  better get out of the water. */
    while (!safe_teleds(true)) {
        pline("You're still drowning.");
        done(DROWNING);
    }
    if (u.uinwater) {
        u.uinwater = 0;
        You("find yourself back %s.", Is_waterlevel(&u.uz) ? "in an air bubble" : "on land");
    }
    return (true);
}

void drain_en(int n) {
    if (!u.uenmax)
        return;
    You_feel("your magical energy drain away!");
    u.uen -= n;
    if (u.uen < 0) {
        u.uenmax += u.uen;
        if (u.uenmax < 0)
            u.uenmax = 0;
        u.uen = 0;
    }
}

int dountrap(void) /* disarm a trap */
{
    if (near_capacity() >= HVY_ENCUMBER) {
        pline("You're too strained to do that.");
        return 0;
    }
    if ((nohands(youmonst.data) && !webmaker(youmonst.data)) || !youmonst.data->mmove) {
        pline("And just how do you expect to do that?");
        return 0;
    } else if (u.ustuck && sticks(youmonst.data)) {
        message_monster(MSG_YOU_MUST_LET_GO_OF_M_FIRST, u.ustuck);
        return 0;
    }
    if (u.ustuck || (welded(uwep) && bimanual(uwep))) {
        Your("%s seem to be too busy for that.", makeplural(body_part(HAND)));
        return 0;
    }
    return untrap(false);
}

/* Probability of disabling a trap.  Helge Hafting */
static int untrap_prob(struct trap *ttmp) {
    int chance = 3;

    /* Only spiders know how to deal with webs reliably */
    if (ttmp->ttyp == WEB && !webmaker(youmonst.data))
        chance = 30;
    if (Confusion() || Hallucination()) chance++;
    if (Blind)
        chance++;
    if (Stunned())
        chance += 2;
    if (Fumbling())
        chance *= 2;
    /* Your own traps are better known than others. */
    if (ttmp && ttmp->madeby_u)
        chance--;
    if (Role_if(PM_ROGUE)) {
        if (rn2(2 * MAXULEV) < u.ulevel)
            chance--;
        if (u.uhave.questart && chance > 1)
            chance--;
    } else if (Role_if(PM_RANGER) && chance > 1)
        chance--;
    return rn2(chance);
}

/* Replace trap with object(s).  Helge Hafting */
static void cnv_trap_obj(int otyp, int cnt, struct trap *ttmp) {
    struct obj *otmp = mksobj(otyp, true, false);
    otmp->quan = cnt;
    otmp->owt = weight(otmp);
    /* Only dart traps are capable of being poisonous */
    if (otyp != DART)
        otmp->opoisoned = 0;
    place_object(otmp, ttmp->tx, ttmp->ty);
    /* Sell your own traps only... */
    if (ttmp->madeby_u)
        sellobj(otmp, ttmp->tx, ttmp->ty);
    stackobj(otmp);
    newsym(ttmp->tx, ttmp->ty);
    deltrap(ttmp);
}

/* while attempting to disarm an adjacent trap, we've fallen into it */
static void move_into_trap(struct trap *ttmp) {
    int bc;
    signed char x = ttmp->tx, y = ttmp->ty, bx, by, cx, cy;
    bool unused;

    /* we know there's no monster in the way, and we're not trapped */
    if (!Punished || drag_ball(x, y, &bc, &bx, &by, &cx, &cy, &unused,
    true)) {
        u.ux0 = u.ux, u.uy0 = u.uy;
        u.ux = x, u.uy = y;
        u.umoved = true;
        newsym(u.ux0, u.uy0);
        vision_recalc(1);
        check_leash(u.ux0, u.uy0);
        if (Punished)
            move_bc(0, bc, bx, by, cx, cy);
        spoteffects(false); /* dotrap() */
        exercise(A_WIS, false);
    }
}

/* 0: doesn't even try
 * 1: tries and fails
 * 2: succeeds
 */
static int try_disarm(struct trap *ttmp, bool force_failure) {
    struct monst *mtmp = m_at(ttmp->tx, ttmp->ty);
    int ttype = ttmp->ttyp;
    bool under_u = (!u.dx && !u.dy);
    bool holdingtrap = (ttype == BEAR_TRAP || ttype == WEB);

    /* Test for monster first, monsters are displayed instead of trap. */
    if (mtmp && (!mtmp->mtrapped || !holdingtrap)) {
        message_monster(MSG_M_IS_IN_THE_WAY, mtmp);
        return 0;
    }
    /* We might be forced to move onto the trap's location. */
    if (sobj_at(BOULDER, ttmp->tx, ttmp->ty) && !Passes_walls && !under_u) {
        There("is a boulder in your way.");
        return 0;
    }
    /* duplicate tight-space checks from test_move */
    if (u.dx && u.dy && bad_rock(youmonst.data, u.ux, ttmp->ty) && bad_rock(youmonst.data, ttmp->tx, u.uy)) {
        if ((invent && (inv_weight() + weight_cap() > 600)) || bigmonst(youmonst.data)) {
            /* don't allow untrap if they can't get thru to it */
            You("are unable to reach the %s!", defsyms[trap_to_defsym(ttype)].explanation);
            return 0;
        }
    }
    /* untrappable traps are located on the ground. */
    if (!can_reach_floor()) {
        if (u.usteed && P_SKILL(P_RIDING) < P_BASIC) {
            message_monster(MSG_YOU_ARE_NOT_SKILLED_ENOUGH_TO_REACH_FROM_M, u.usteed);
        } else {
            You("are unable to reach the %s!", defsyms[trap_to_defsym(ttype)].explanation);
        }
        return 0;
    }

    /* Will our hero succeed? */
    if (force_failure || untrap_prob(ttmp)) {
        if (rnl(5)) {
            pline("Whoops...");
            if (mtmp) { /* must be a trap that holds monsters */
                if (ttype == BEAR_TRAP) {
                    if (mtmp->mtame)
                        abuse_dog(mtmp);
                    if ((mtmp->mhp -= rnd(4)) <= 0)
                        killed(mtmp);
                } else if (ttype == WEB) {
                    if (!webmaker(youmonst.data)) {
                        struct trap *ttmp2 = maketrap(u.ux, u.uy, WEB);
                        if (ttmp2) {
                            pline_The("webbing sticks to you. You're caught too!");
                            dotrap(ttmp2, NOWEBMSG);
                            if (u.usteed && u.utrap) {
                                /* you, not steed, are trapped */
                                dismount_steed(DISMOUNT_FELL);
                            }
                        }
                    } else {
                        message_monster(MSG_M_REMAINS_ENTANGLED, mtmp);
                    }
                }
            } else if (under_u) {
                dotrap(ttmp, 0);
            } else {
                move_into_trap(ttmp);
            }
        } else {
            pline("%s %s is difficult to %s.", ttmp->madeby_u ? "Your" : under_u ? "This" : "That", defsyms[trap_to_defsym(ttype)].explanation, (ttype == WEB) ? "remove" : "disarm");
        }
        return 1;
    }
    return 2;
}

static void reward_untrap(struct trap *ttmp, struct monst *mtmp) {
    if (!ttmp->madeby_u) {
        if (rnl(10) < 8 && !mtmp->mpeaceful && !mtmp->msleeping && !mtmp->mfrozen &&
                !mindless(mtmp->data) && mtmp->data->mlet != S_HUMAN)
        {
            mtmp->mpeaceful = 1;
            set_malign(mtmp); /* reset alignment */
            message_monster(MSG_M_IS_GRATEFUL, mtmp);
        }
        /* Helping someone out of a trap is a nice thing to do,
         * A lawful may be rewarded, but not too often.  */
        if (!rn2(3) && !rnl(8) && u.ualign.type == A_LAWFUL) {
            adjalign(1);
            You_feel("that you did the right thing.");
        }
    }
}

static int disarm_holdingtrap( /* Helge Hafting */
struct trap *ttmp) {
    struct monst *mtmp;
    int fails = try_disarm(ttmp, false);

    if (fails < 2)
        return fails;

    /* ok, disarm it. */

    /* untrap the monster, if any.
     There's no need for a cockatrice test, only the trap is touched */
    if ((mtmp = m_at(ttmp->tx, ttmp->ty)) != 0) {
        mtmp->mtrapped = 0;
        You("remove %s %s from %s.",
                the_your[ttmp->madeby_u],
                (ttmp->ttyp == BEAR_TRAP) ? "bear trap" : "webbing", "TODO:mon_nam(mtmp)");
        reward_untrap(ttmp, mtmp);
    } else {
        if (ttmp->ttyp == BEAR_TRAP) {
            You("disarm %s bear trap.", the_your[ttmp->madeby_u]);
            cnv_trap_obj(BEARTRAP, 1, ttmp);
        } else /* if (ttmp->ttyp == WEB) */{
            You("succeed in removing %s web.", the_your[ttmp->madeby_u]);
            deltrap(ttmp);
        }
    }
    newsym(u.ux + u.dx, u.uy + u.dy);
    return 1;
}

static int disarm_landmine( /* Helge Hafting */
struct trap *ttmp) {
    int fails = try_disarm(ttmp, false);

    if (fails < 2)
        return fails;
    You("disarm %s land mine.", the_your[ttmp->madeby_u]);
    cnv_trap_obj(LAND_MINE, 1, ttmp);
    return 1;
}

/* getobj will filter down to cans of grease and known potions of oil */
static const char oil[] = { ALL_CLASSES, TOOL_CLASS, POTION_CLASS, 0 };

/* it may not make much sense to use grease on floor boards, but so what? */
static int disarm_squeaky_board(struct trap *ttmp) {
    struct obj *obj;
    bool bad_tool;
    int fails;

    obj = getobj(oil, "untrap with");
    if (!obj)
        return 0;

    bad_tool = (obj->cursed || ((obj->otyp != POT_OIL || obj->lamplit) && (obj->otyp != CAN_OF_GREASE || !obj->spe)));

    fails = try_disarm(ttmp, bad_tool);
    if (fails < 2)
        return fails;

    /* successfully used oil or grease to fix squeaky board */
    if (obj->otyp == CAN_OF_GREASE) {
        consume_obj_charge(obj, true);
    } else {
        useup(obj); /* oil */
        makeknown(POT_OIL);
    }
    You("repair the squeaky board."); /* no madeby_u */
    deltrap(ttmp);
    newsym(u.ux + u.dx, u.uy + u.dy);
    more_experienced(1, 5);
    newexplevel();
    return 1;
}

/* removes traps that shoot arrows, darts, etc. */
static int disarm_shooting_trap(struct trap *ttmp, int otyp) {
    int fails = try_disarm(ttmp, false);

    if (fails < 2)
        return fails;
    You("disarm %s trap.", the_your[ttmp->madeby_u]);
    cnv_trap_obj(otyp, 50 - rnl(50), ttmp);
    return 1;
}

/* Is the weight too heavy?
 * Formula as in near_capacity() & check_capacity() */
static int try_lift(struct monst *mtmp, struct trap *ttmp, int wt, bool stuff) {
    int wc = weight_cap();

    if (((wt * 2) / wc) >= HVY_ENCUMBER) {
        pline("%s is %s for you to lift.", "TODO:Monnam(mtmp)", stuff ? "carrying too much" : "too heavy");
        if (!ttmp->madeby_u && !mtmp->mpeaceful && mtmp->mcanmove &&
                !mindless(mtmp->data) && mtmp->data->mlet != S_HUMAN && rnl(10) < 3)
        {
            mtmp->mpeaceful = 1;
            set_malign(mtmp); /* reset alignment */
            message_monster(MSG_M_THINKS_IT_WAS_NICE_OF_YOU_TO_TRY, mtmp);
        }
        return 0;
    }
    return 1;
}

/* Help trapped monster (out of a (spiked) pit) */
static int help_monster_out(struct monst *mtmp, struct trap *ttmp) {
    int wt;
    struct obj *otmp;
    bool uprob;

    /*
     * This works when levitating too -- consistent with the ability
     * to hit monsters while levitating.
     *
     * Should perhaps check that our hero has arms/hands at the
     * moment.  Helping can also be done by engulfing...
     *
     * Test the monster first - monsters are displayed before traps.
     */
    if (!mtmp->mtrapped) {
        message_monster(MSG_M_IS_NOT_TRAPPED, mtmp);
        return 0;
    }
    /* Do you have the necessary capacity to lift anything? */
    if (check_capacity((char *)0))
        return 1;

    /* Will our hero succeed? */
    if ((uprob = untrap_prob(ttmp)) && !mtmp->msleeping && mtmp->mcanmove) {
        You("try to reach out your %s, but %s backs away skeptically.",
                makeplural(body_part(ARM)),
                "TODO:mon_nam(mtmp)");
        return 1;
    }

    /* is it a cockatrice?... */
    if (touch_petrifies(mtmp->data) && !uarmg && !Stone_resistance()) {
        You("grab the trapped %s using your bare %s.", mtmp->data->mname, makeplural(body_part(HAND)));

        if (poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM))
            display_nhwindow(WIN_MESSAGE, false);
        else {
            char kbuf[BUFSZ];

            sprintf(kbuf, "trying to help %s out of a pit", an(mtmp->data->mname));
            instapetrify(kbuf);
            return 1;
        }
    }
    /* need to do cockatrice check first if sleeping or paralyzed */
    if (uprob) {
        message_monster(MSG_YOU_TRY_TO_GRAB_M_BUT_CANNOT_GET_A_GRASP, mtmp);
        if (mtmp->msleeping) {
            mtmp->msleeping = 0;
            message_monster(MSG_M_AWAKENS, mtmp);
        }
        return 1;
    }

    message_monster(MSG_YOU_REACH_OUT_YOUR_ARM_AND_GRAB_M, mtmp);

    if (mtmp->msleeping) {
        mtmp->msleeping = 0;
        message_monster(MSG_M_AWAKENS, mtmp);
    } else if (mtmp->mfrozen && !rn2(mtmp->mfrozen)) {
        /* After such manhandling, perhaps the effect wears off */
        mtmp->mcanmove = 1;
        mtmp->mfrozen = 0;
        message_monster(MSG_M_STIRS, mtmp);
    }

    /* is the monster too heavy? */
    wt = inv_weight() + mtmp->data->cwt;
    if (!try_lift(mtmp, ttmp, wt, false))
        return 1;

    /* is the monster with inventory too heavy? */
    for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj)
        wt += otmp->owt;
    if (!try_lift(mtmp, ttmp, wt, true))
        return 1;

    message_monster(MSG_YOU_PULL_M_OUT_OF_THE_PIT, mtmp);
    mtmp->mtrapped = 0;
    fill_pit(mtmp->mx, mtmp->my);
    reward_untrap(ttmp, mtmp);
    return 1;
}

int untrap(bool force) {
    struct obj *otmp;
    bool confused = (Confusion() > 0 || Hallucination() > 0);
    int x, y;
    int ch;
    struct trap *ttmp;
    struct monst *mtmp;
    bool trap_skipped = false;
    bool box_here = false;
    bool deal_with_floor_trap = false;
    char the_trap[BUFSZ], qbuf[QBUFSZ];
    int containercnt = 0;

    if (!getdir((char *)0))
        return (0);
    x = u.ux + u.dx;
    y = u.uy + u.dy;

    for (otmp = level.objects[x][y]; otmp; otmp = otmp->nexthere) {
        if (Is_box(otmp) && !u.dx && !u.dy) {
            box_here = true;
            containercnt++;
            if (containercnt > 1)
                break;
        }
    }

    if ((ttmp = t_at(x, y)) && ttmp->tseen) {
        deal_with_floor_trap = true;
        strcpy(the_trap, the(defsyms[trap_to_defsym(ttmp->ttyp)].explanation));
        if (box_here) {
            if (ttmp->ttyp == PIT || ttmp->ttyp == SPIKED_PIT) {
                You_cant("do much about %s%s.", the_trap, u.utrap ? " that you're stuck in" : " while standing on the edge of it");
                trap_skipped = true;
                deal_with_floor_trap = false;
            } else {
                sprintf(qbuf, "There %s and %s here. %s %s?", (containercnt == 1) ? "is a container" : "are containers", an(defsyms[trap_to_defsym(ttmp->ttyp)].explanation), ttmp->ttyp == WEB ? "Remove" : "Disarm", the_trap);
                switch (ynq(qbuf)) {
                    case 'q':
                        return (0);
                    case 'n':
                        trap_skipped = true;
                        deal_with_floor_trap = false;
                        break;
                }
            }
        }
        if (deal_with_floor_trap) {
            if (u.utrap) {
                You("cannot deal with %s while trapped%s!", the_trap, (x == u.ux && y == u.uy) ? " in it" : "");
                return 1;
            }
            switch (ttmp->ttyp) {
                case BEAR_TRAP:
                case WEB:
                    return disarm_holdingtrap(ttmp);
                case LANDMINE:
                    return disarm_landmine(ttmp);
                case SQKY_BOARD:
                    return disarm_squeaky_board(ttmp);
                case DART_TRAP:
                    return disarm_shooting_trap(ttmp, DART);
                case ARROW_TRAP:
                    return disarm_shooting_trap(ttmp, ARROW);
                case PIT:
                case SPIKED_PIT:
                    if (!u.dx && !u.dy) {
                        You("are already on the edge of the pit.");
                        return 0;
                    }
                    if (!(mtmp = m_at(x, y))) {
                        pline("Try filling the pit instead.");
                        return 0;
                    }
                    return help_monster_out(mtmp, ttmp);
                default:
                    You("cannot disable %s trap.", (u.dx || u.dy) ? "that" : "this");
                    return 0;
            }
        }
    } /* end if */

    if (!u.dx && !u.dy) {
        for (otmp = level.objects[x][y]; otmp; otmp = otmp->nexthere)
            if (Is_box(otmp)) {
                sprintf(qbuf, "There is %s here. Check it for traps?", safe_qbuf("", sizeof("There is  here. Check it for traps?"), doname(otmp), an(simple_typename(otmp->otyp)), "a box"));
                switch (ynq(qbuf)) {
                    case 'q':
                        return (0);
                    case 'n':
                        continue;
                }
                if (u.usteed && P_SKILL(P_RIDING) < P_BASIC) {
                    message_monster(MSG_YOU_ARE_NOT_SKILLED_ENOUGH_TO_REACH_FROM_M, u.usteed);
                    return (0);
                }
                if ((otmp->otrapped && (force || (!confused && rn2(MAXULEV + 1 - u.ulevel) < 10))) || (!force && confused && !rn2(3))) {
                    You("find a trap on %s!", the(xname(otmp)));
                    if (!confused)
                        exercise(A_WIS, true);

                    switch (ynq("Disarm it?")) {
                        case 'q':
                            return (1);
                        case 'n':
                            trap_skipped = true;
                            continue;
                    }

                    if (otmp->otrapped) {
                        exercise(A_DEX, true);
                        ch = ACURR(A_DEX) + u.ulevel;
                        if (Role_if(PM_ROGUE))
                            ch *= 2;
                        if (!force && (confused || Fumbling() || rnd(75 + level_difficulty() / 2) > ch)) {
                            (void)chest_trap(otmp, FINGER, true);
                        } else {
                            You("disarm it!");
                            otmp->otrapped = 0;
                        }
                    } else
                        pline("That %s was not trapped.", xname(otmp));
                    return (1);
                } else {
                    You("find no traps on %s.", the(xname(otmp)));
                    return (1);
                }
            }

        You(trap_skipped ? "find no other traps here." : "know of no traps here.");
        return (0);
    }

    if ((mtmp = m_at(x, y)) && mtmp->m_ap_type == M_AP_FURNITURE && (mtmp->mappearance == S_hcdoor || mtmp->mappearance == S_vcdoor) && !Protection_from_shape_changers) {

        stumble_onto_mimic(mtmp);
        return (1);
    }

    if (!IS_DOOR(levl[x][y].typ)) {
        if ((ttmp = t_at(x, y)) && ttmp->tseen)
            You("cannot disable that trap.");
        else
            You("know of no traps there.");
        return (0);
    }

    switch (levl[x][y].doormask) {
        case D_NODOOR:
            You("%s no door there.", Blind ? "feel" : "see");
            return (0);
        case D_ISOPEN:
            pline("This door is safely open.");
            return (0);
        case D_BROKEN:
            pline("This door is broken.");
            return (0);
    }

    if (((levl[x][y].doormask & D_TRAPPED) && (force || (!confused && rn2(MAXULEV - u.ulevel + 11) < 10))) || (!force && confused && !rn2(3))) {
        You("find a trap on the door!");
        exercise(A_WIS, true);
        if (ynq("Disarm it?") != 'y')
            return (1);
        if (levl[x][y].doormask & D_TRAPPED) {
            ch = 15 + (Role_if(PM_ROGUE) ? u.ulevel * 3 : u.ulevel);
            exercise(A_DEX, true);
            if (!force && (confused || Fumbling() || rnd(75 + level_difficulty() / 2) > ch)) {
                You("set it off!");
                b_trapped("door", FINGER);
                levl[x][y].doormask = D_NODOOR;
                unblock_point(x, y);
                newsym(x, y);
                /* (probably ought to charge for this damage...) */
                if (*in_rooms(x, y, SHOPBASE))
                    add_damage(x, y, 0L);
            } else {
                You("disarm it!");
                levl[x][y].doormask &= ~D_TRAPPED;
            }
        } else
            pline("This door was not trapped.");
        return (1);
    } else {
        You("find no traps on the door.");
        return (1);
    }
}

/* only called when the player is doing something to the chest directly */
bool chest_trap(struct obj *obj, int bodypart, bool disarm) {
    struct obj *otmp = obj, *otmp2;
    char buf[80];
    const char *msg;
    coord cc;

    if (get_obj_location(obj, &cc.x, &cc.y, 0)) /* might be carried */
        obj->ox = cc.x, obj->oy = cc.y;

    otmp->otrapped = 0; /* trap is one-shot; clear flag first in case
     chest kills you and ends up in bones file */
    You(disarm ? "set it off!" : "trigger a trap!");
    display_nhwindow(WIN_MESSAGE, false);
    if (Luck > -13 && rn2(13 + Luck) > 7) { /* saved by luck */
        /* trap went off, but good luck prevents damage */
        switch (rn2(13)) {
            case 12:
            case 11:
                msg = "explosive charge is a dud";
                break;
            case 10:
            case 9:
                msg = "electric charge is grounded";
                break;
            case 8:
            case 7:
                msg = "flame fizzles out";
                break;
            case 6:
            case 5:
            case 4:
                msg = "poisoned needle misses";
                break;
            case 3:
            case 2:
            case 1:
            case 0:
                msg = "gas cloud blows away";
                break;
            default:
                impossible("chest disarm bug");
                msg = (char *)0;
                break;
        }
        if (msg)
            pline("But luckily the %s!", msg);
    } else {
        switch (rn2(20) ? ((Luck >= 13) ? 0 : rn2(13 - Luck)) : rn2(26)) {
            case 25:
            case 24:
            case 23:
            case 22:
            case 21: {
                struct monst *shkp = 0;
                long loss = 0L;
                bool costly, insider;
                signed char ox = obj->ox, oy = obj->oy;

                /* the obj location need not be that of player */
                costly = (costly_spot(ox, oy) && (shkp = shop_keeper(*in_rooms(ox, oy, SHOPBASE))) != (struct monst *)0);
                insider = (*u.ushops && inside_shop(u.ux, u.uy) && *in_rooms(ox, oy, SHOPBASE) == *u.ushops);

                message_object(MSG_O_EXPLODES, obj);
                sprintf(buf, "exploding %s", xname(obj));

                if (costly)
                    loss += stolen_value(obj, ox, oy, (bool)shkp->mpeaceful, true);
                delete_contents(obj);
                /* we're about to delete all things at this location,
                 * which could include the ball & chain.
                 * If we attempt to call unpunish() in the
                 * for-loop below we can end up with otmp2
                 * being invalid once the chain is gone.
                 * Deal with ball & chain right now instead.
                 */
                if (Punished && !carried(uball) && ((uchain->ox == u.ux && uchain->oy == u.uy) || (uball->ox == u.ux && uball->oy == u.uy)))
                    unpunish();

                for (otmp = level.objects[u.ux][u.uy]; otmp; otmp = otmp2) {
                    otmp2 = otmp->nexthere;
                    if (costly)
                        loss += stolen_value(otmp, otmp->ox, otmp->oy, (bool)shkp->mpeaceful,
                        true);
                    delobj(otmp);
                }
                wake_nearby();
                fprintf(stderr, "losehp killed by %s\n", buf);
                losehp(d(6, 6), killed_by_const(KM_TODO));
                exercise(A_STR, false);
                if (costly && loss) {
                    if (insider)
                        You("owe %ld %s for objects destroyed.", loss, currency(loss));
                    else {
                        You("caused %ld %s worth of damage!", loss, currency(loss));
                        make_angry_shk(shkp, ox, oy);
                    }
                }
                return true;
            }
            case 20:
            case 19:
            case 18:
            case 17:
                pline("A cloud of noxious gas billows from %s.", the(xname(obj)));
                poisoned("gas cloud", A_STR, "cloud of poison gas", 15);
                exercise(A_CON, false);
                break;
            case 16:
            case 15:
            case 14:
            case 13:
                You_feel("a needle prick your %s.", body_part(bodypart));
                poisoned("needle", A_CON, "poisoned needle", 10);
                exercise(A_CON, false);
                break;
            case 12:
            case 11:
            case 10:
            case 9:
                dofiretrap(obj);
                break;
            case 8:
            case 7:
            case 6: {
                int dmg;

                You("are jolted by a surge of electricity!");
                if (Shock_resistance()) {
                    shieldeff(u.ux, u.uy);
                    You("don't seem to be affected.");
                    dmg = 0;
                } else
                    dmg = d(4, 4);
                destroy_item(RING_CLASS, AD_ELEC);
                destroy_item(WAND_CLASS, AD_ELEC);
                if (dmg)
                    losehp(dmg, killed_by_const(KM_ELECTRIC_SHOCK));
                break;
            }
            case 5:
            case 4:
            case 3:
                if (!Free_action) {
                    pline("Suddenly you are frozen in place!");
                    nomul(-d(5, 6));
                    exercise(A_DEX, false);
                    nomovemsg = You_can_move_again;
                } else You("momentarily stiffen.");
                break;
                case 2:
                case 1:
                case 0:
                pline("A cloud of %s gas billows from %s.",
                        Blind ? blindgas[rn2(SIZE(blindgas))] :
                        rndcolor(), the(xname(obj)));
                if(!Stunned()) {
                    if (Hallucination())
                    pline("What a groovy feeling!");
                    else if (Blind)
                    You("%s and get dizzy...",
                            stagger(youmonst.data, "stagger"));
                    else
                    You("%s and your vision blurs...",
                            stagger(youmonst.data, "stagger"));
                }
                make_stunned(get_HStun() + rn1(7, 16),false);
                (void) make_hallucinated(u.uprops[HALLUC].intrinsic + rn1(5, 16),false,0L);
                break;
                default: impossible("bad chest trap");
                break;
            }
        }

    return false;
}

struct trap *
t_at(int x, int y) {
    struct trap *trap = ftrap;
    while (trap) {
        if (trap->tx == x && trap->ty == y)
            return (trap);
        trap = trap->ntrap;
    }
    return ((struct trap *)0);
}

void deltrap(struct trap *trap) {
    struct trap *ttmp;

    if (trap == ftrap)
        ftrap = ftrap->ntrap;
    else {
        for (ttmp = ftrap; ttmp->ntrap != trap; ttmp = ttmp->ntrap)
            ;
        ttmp->ntrap = trap->ntrap;
    }
    dealloc_trap(trap);
}

bool delfloortrap(struct trap *ttmp) {
    /* Destroy a trap that emanates from the floor. */
    /* some of these are arbitrary -dlc */
    if (ttmp && ((ttmp->ttyp == SQKY_BOARD) || (ttmp->ttyp == BEAR_TRAP) || (ttmp->ttyp == LANDMINE) || (ttmp->ttyp == FIRE_TRAP) || (ttmp->ttyp == PIT) || (ttmp->ttyp == SPIKED_PIT) || (ttmp->ttyp == HOLE) || (ttmp->ttyp == TRAPDOOR) || (ttmp->ttyp == TELEP_TRAP) || (ttmp->ttyp == LEVEL_TELEP) || (ttmp->ttyp == WEB) || (ttmp->ttyp == MAGIC_TRAP) || (ttmp->ttyp == ANTI_MAGIC))) {
        struct monst *mtmp;

        if (ttmp->tx == u.ux && ttmp->ty == u.uy) {
            u.utrap = 0;
            u.utraptype = 0;
        } else if ((mtmp = m_at(ttmp->tx, ttmp->ty)) != 0) {
            mtmp->mtrapped = 0;
        }
        deltrap(ttmp);
        return true;
    } else
        return false;
}

/* used for doors (also tins).  can be used for anything else that opens. */
void b_trapped(const char *item, int bodypart) {
    int lvl = level_difficulty();
    int dmg = rnd(5 + (lvl < 5 ? lvl : 2 + lvl / 2));

    pline("KABOOM!!  %s was booby-trapped!", The(item));
    wake_nearby();
    losehp(dmg, killed_by_const(KM_EXPLOSION));
    exercise(A_STR, false);
    if (bodypart)
        exercise(A_CON, false);
    make_stunned(get_HStun() + dmg, true);
}

bool unconscious(void) {
    return ((bool)(multi < 0 && (!nomovemsg || u.usleep || !strncmp(nomovemsg, "You regain con", 14) || !strncmp(nomovemsg, "You are consci", 14))));
}

bool lava_effects(void) {
    struct obj *obj, *obj2;
    int dmg;
    bool usurvive;

    burn_away_slime();
    if (likes_lava(youmonst.data))
        return false;

    if (!Fire_resistance()) {
        if (Wwalking) {
            dmg = d(6, 6);
            pline_The("lava here burns you!");
            if (dmg < u.uhp) {
                losehp(dmg, killed_by_const(KM_MOLTEN_LAVA));
                goto burn_stuff;
            }
        } else {
            You("fall into the lava!");
        }

        usurvive = Lifesaved;
        if (flags.debug)
            usurvive = true;
        for (obj = invent; obj; obj = obj2) {
            obj2 = obj->nobj;
            if (is_organic(obj) && !obj->oerodeproof) {
                if (obj->owornmask) {
                    if (usurvive)
                        Your("%s into flame!", aobjnam(obj, "burst"));

                    if (obj == uarm)
                        (void)Armor_gone();
                    else if (obj == uarmc)
                        (void)Cloak_off();
                    else if (obj == uarmh)
                        (void)Helmet_off();
                    else if (obj == uarms)
                        (void)Shield_off();
                    else if (obj == uarmg)
                        (void)Gloves_off();
                    else if (obj == uarmf)
                        (void)Boots_off();
                    else if (obj == uarmu)
                        setnotworn(obj);
                    else if (obj == uleft)
                        Ring_gone(obj);
                    else if (obj == uright)
                        Ring_gone(obj);
                    else if (obj == ublindf)
                        Blindf_off(obj);
                    else if (obj == uamul)
                        Amulet_off();
                    else if (obj == uwep)
                        uwepgone();
                    else if (obj == uquiver)
                        uqwepgone();
                    else if (obj == uswapwep)
                        uswapwepgone();
                }
                useupall(obj);
            }
        }

        /* s/he died... */
        u.uhp = -1;
        killer = killed_by_const(KM_MOLTEN_LAVA);
        You("burn to a crisp...");
        done(BURNING);
        while (!safe_teleds(true)) {
            pline("You're still burning.");
            done(BURNING);
        }
        You("find yourself back on solid %s.", surface(u.ux, u.uy));
        return (true);
    }

    if (!Wwalking) {
        u.utrap = rn1(4, 4) + (rn1(4, 12) << 8);
        u.utraptype = TT_LAVA;
        You("sink into the lava, but it only burns slightly!");
        if (u.uhp > 1)
            losehp(1, killed_by_const(KM_MOLTEN_LAVA));
    }
    /* just want to burn boots, not all armor; destroy_item doesn't work on
     armor anyway */
    burn_stuff: if (uarmf && !uarmf->oerodeproof && is_organic(uarmf)) {
        /* save uarmf value because Boots_off() sets uarmf to null */
        obj = uarmf;
        Your("%s bursts into flame!", xname(obj));
        (void)Boots_off();
        useup(obj);
    }
    destroy_item(SCROLL_CLASS, AD_FIRE);
    destroy_item(SPBOOK_CLASS, AD_FIRE);
    destroy_item(POTION_CLASS, AD_FIRE);
    return (false);
}
