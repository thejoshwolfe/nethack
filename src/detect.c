/* See LICENSE in the root of this project for change info */

/*
 * Detection routines, including crystal ball, magic mapping, and search
 * command.
 */

#include "detect.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "display_util.h"
#include "dungeon_util.h"
#include "artifact.h"
#include "attrib.h"
#include "cmd.h"
#include "coord.h"
#include "dbridge.h"
#include "decl.h"
#include "display.h"
#include "do_name.h"
#include "drawing.h"
#include "flag.h"
#include "global.h"
#include "hack.h"
#include "invent.h"
#include "mhitu.h"
#include "mkroom.h"
#include "mon.h"
#include "mondata.h"
#include "monst.h"
#include "monsym.h"
#include "obj.h"
#include "objclass.h"
#include "objnam.h"
#include "onames.h"
#include "permonst.h"
#include "pline.h"
#include "pm.h"
#include "polyself.h"
#include "potion.h"
#include "prop.h"
#include "rm.h"
#include "rnd.h"
#include "trap.h"
#include "util.h"
#include "vault.h"
#include "vision.h"
#include "worm.h"
#include "you.h"
#include "youprop.h"

extern bool known;   /* from read.c */

static const struct {
    const char *what;
    d_level *where;
} level_detects[] = {
    { "Delphi", &oracle_level },
    { "Medusa's lair", &medusa_level },
    { "a castle", &stronghold_level },
    { "the Wizard of Yendor's tower", &wiz1_level },
};


/* Recursively search obj for an object in class oclass and return 1st found */
struct obj * o_in (struct obj *obj, char oclass) {
    struct obj* otmp;
    struct obj *temp;

    if (obj->oclass == oclass) return obj;

    if (Has_contents(obj)) {
        for (otmp = obj->cobj; otmp; otmp = otmp->nobj)
            if (otmp->oclass == oclass) return otmp;
            else if (Has_contents(otmp) && (temp = o_in(otmp, oclass)))
                return temp;
    }
    return (struct obj *) 0;
}

/* Recursively search obj for an object made of specified material and return 1st found */
struct obj * o_material (struct obj *obj, unsigned material) {
    struct obj* otmp;
    struct obj *temp;

    if (objects[obj->otyp].oc_material == material) return obj;

    if (Has_contents(obj)) {
        for (otmp = obj->cobj; otmp; otmp = otmp->nobj)
            if (objects[otmp->otyp].oc_material == material) return otmp;
            else if (Has_contents(otmp) && (temp = o_material(otmp, material)))
                return temp;
    }
    return (struct obj *) 0;
}

static void do_dknown_of (struct obj *obj) {
    struct obj *otmp;

    obj->dknown = 1;
    if (Has_contents(obj)) {
        for(otmp = obj->cobj; otmp; otmp = otmp->nobj)
            do_dknown_of(otmp);
    }
}

/* Check whether the location has an outdated object displayed on it. */
static bool check_map_spot(int x, int y, char oclass, unsigned material) {
    int glyph;
    struct obj *otmp;
    struct monst *mtmp;

    glyph = glyph_at(x,y);
    if (glyph_is_object(glyph)) {
        /* there's some object shown here */
        if (oclass == ALL_CLASSES) {
            return((bool)( !(level.objects[x][y] ||     /* stale if nothing here */
                            ((mtmp = m_at(x,y)) != 0 &&
                             (
                              mtmp->mgold ||
                              mtmp->minvent)))));
        } else {
            if (material && objects[glyph_to_obj(glyph)].oc_material == material) {
                /* the object shown here is of interest because material matches */
                for (otmp = level.objects[x][y]; otmp; otmp = otmp->nexthere)
                    if (o_material(otmp, GOLD)) return false;
                /* didn't find it; perhaps a monster is carrying it */
                if ((mtmp = m_at(x,y)) != 0) {
                    for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj)
                        if (o_material(otmp, GOLD)) return false;
                }
                /* detection indicates removal of this object from the map */
                return true;
            }
            if (oclass && objects[glyph_to_obj(glyph)].oc_class == oclass) {
                /* the object shown here is of interest because its class matches */
                for (otmp = level.objects[x][y]; otmp; otmp = otmp->nexthere)
                    if (o_in(otmp, oclass)) return false;
                /* didn't find it; perhaps a monster is carrying it */
                if ((mtmp = m_at(x,y)) != 0) {
                    if (oclass == COIN_CLASS && mtmp->mgold)
                        return false;
                    else for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj)
                        if (o_in(otmp, oclass)) return false;
                }
                /* detection indicates removal of this object from the map */
                return true;
            }
        }
    }
    return false;
}

/*
   When doing detection, remove stale data from the map display (corpses
   rotted away, objects carried away by monsters, etc) so that it won't
   reappear after the detection has completed.  Return true if noticeable
   change occurs.
   */
static bool clear_stale_map(char oclass, unsigned material) {
    int zx, zy;
    bool change_made = false;

    for (zx = 1; zx < COLNO; zx++)
        for (zy = 0; zy < ROWNO; zy++)
            if (check_map_spot(zx, zy, oclass,material)) {
                unmap_object(zx, zy);
                change_made = true;
            }

    return change_made;
}

/* look for gold, on the floor or in monsters' possession */
int gold_detect (struct obj *sobj) {
    struct obj *obj;
    struct monst *mtmp;
    int uw = u.uinwater;
    struct obj *temp;
    bool stale;

    known = stale = clear_stale_map(COIN_CLASS,
            (unsigned)(sobj->blessed ? GOLD : 0));

    /* look for gold carried by monsters (might be in a container) */
    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp)) continue;        /* probably not needed in this case but... */
        if (mtmp->mgold || monsndx(mtmp->data) == PM_GOLD_GOLEM) {
            known = true;
            goto outgoldmap;    /* skip further searching */
        } else for (obj = mtmp->minvent; obj; obj = obj->nobj)
            if (sobj->blessed && o_material(obj, GOLD)) {
                known = true;
                goto outgoldmap;
            } else if (o_in(obj, COIN_CLASS)) {
                known = true;
                goto outgoldmap;        /* skip further searching */
            }
    }

    /* look for gold objects */
    for (obj = fobj; obj; obj = obj->nobj) {
        if (sobj->blessed && o_material(obj, GOLD)) {
            known = true;
            if (obj->ox != u.ux || obj->oy != u.uy) goto outgoldmap;
        } else if (o_in(obj, COIN_CLASS)) {
            known = true;
            if (obj->ox != u.ux || obj->oy != u.uy) goto outgoldmap;
        }
    }

    if (!known) {
        /* no gold found on floor or monster's inventory.
           adjust message if you have gold in your inventory */
        if (sobj) {
            char buf[BUFSZ];
            if (youmonst.data == &mons[PM_GOLD_GOLEM]) {
                sprintf(buf, "You feel like a million %s!",
                        currency(2L));
            } else if (hidden_gold() || u.ugold) {
                strcpy(buf, "You feel worried about your future financial situation.");
            } else {
                strcpy(buf, "You feel materially poor.");
            }
            strange_feeling(sobj, buf);
        }
        return(1);
    }
    /* only under me - no separate display required */
    if (stale) docrt();
    You("notice some gold between your %s.", makeplural(body_part(FOOT)));
    return(0);

outgoldmap:
    cls();

    u.uinwater = 0;
    /* Discover gold locations. */
    for (obj = fobj; obj; obj = obj->nobj) {
        if (sobj->blessed && (temp = o_material(obj, GOLD))) {
            if (temp != obj) {
                temp->ox = obj->ox;
                temp->oy = obj->oy;
            }
            map_object(temp,1);
        } else if ((temp = o_in(obj, COIN_CLASS))) {
            if (temp != obj) {
                temp->ox = obj->ox;
                temp->oy = obj->oy;
            }
            map_object(temp,1);
        }
    }
    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp)) continue;        /* probably overkill here */
        if (mtmp->mgold || monsndx(mtmp->data) == PM_GOLD_GOLEM) {
            struct obj gold;

            gold.otyp = GOLD_PIECE;
            gold.ox = mtmp->mx;
            gold.oy = mtmp->my;
            map_object(&gold,1);
        } else for (obj = mtmp->minvent; obj; obj = obj->nobj)
            if (sobj->blessed && (temp = o_material(obj, GOLD))) {
                temp->ox = mtmp->mx;
                temp->oy = mtmp->my;
                map_object(temp,1);
                break;
            } else if ((temp = o_in(obj, COIN_CLASS))) {
                temp->ox = mtmp->mx;
                temp->oy = mtmp->my;
                map_object(temp,1);
                break;
            }
    }

    newsym(u.ux,u.uy);
    You_feel("very greedy, and sense gold!");
    exercise(A_WIS, true);
    // display_nhwindow(WIN_MAP, true);
    docrt();
    u.uinwater = uw;
    if (Underwater) under_water(2);
    if (u.uburied) under_ground(2);
    return(0);
}

/* returns 1 if nothing was detected            */
/* returns 0 if something was detected          */
int food_detect (struct obj *sobj) {
    struct obj *obj;
    struct monst *mtmp;
    int ct = 0, ctu = 0;
    bool confused = (Confusion() || (sobj && sobj->cursed)), stale;
    char oclass = confused ? POTION_CLASS : FOOD_CLASS;
    const char *what = confused ? something : "food";
    int uw = u.uinwater;

    stale = clear_stale_map(oclass, 0);

    for (obj = fobj; obj; obj = obj->nobj)
        if (o_in(obj, oclass)) {
            if (obj->ox == u.ux && obj->oy == u.uy) ctu++;
            else ct++;
        }
    for (mtmp = fmon; mtmp && !ct; mtmp = mtmp->nmon) {
        /* no DEADMONSTER(mtmp) check needed since dmons never have inventory */
        for (obj = mtmp->minvent; obj; obj = obj->nobj)
            if (o_in(obj, oclass)) {
                ct++;
                break;
            }
    }

    if (!ct && !ctu) {
        known = stale && !confused;
        if (stale) {
            docrt();
            You("sense a lack of %s nearby.", what);
            if (sobj && sobj->blessed) {
                if (!u.uedibility) Your("%s starts to tingle.", body_part(NOSE));
                u.uedibility = 1;
            }
        } else if (sobj) {
            char buf[BUFSZ];
            sprintf(buf, "Your %s twitches%s.", body_part(NOSE),
                    (sobj->blessed && !u.uedibility) ? " then starts to tingle" : "");
            if (sobj->blessed && !u.uedibility) {
                bool savebeginner = flags.beginner;  /* prevent non-delivery of */
                flags.beginner = false;                 /*      message            */
                strange_feeling(sobj, buf);
                flags.beginner = savebeginner;
                u.uedibility = 1;
            } else
                strange_feeling(sobj, buf);
        }
        return !stale;
    } else if (!ct) {
        known = true;
        You("%s %s nearby.", sobj ? "smell" : "sense", what);
        if (sobj && sobj->blessed) {
            if (!u.uedibility) pline("Your %s starts to tingle.", body_part(NOSE));
            u.uedibility = 1;
        }
    } else {
        struct obj *temp;
        known = true;
        cls();
        u.uinwater = 0;
        for (obj = fobj; obj; obj = obj->nobj)
            if ((temp = o_in(obj, oclass)) != 0) {
                if (temp != obj) {
                    temp->ox = obj->ox;
                    temp->oy = obj->oy;
                }
                map_object(temp,1);
            }
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
            /* no DEADMONSTER(mtmp) check needed since dmons never have inventory */
            for (obj = mtmp->minvent; obj; obj = obj->nobj)
                if ((temp = o_in(obj, oclass)) != 0) {
                    temp->ox = mtmp->mx;
                    temp->oy = mtmp->my;
                    map_object(temp,1);
                    break;      /* skip rest of this monster's inventory */
                }
        newsym(u.ux,u.uy);
        if (sobj) {
            if (sobj->blessed) {
                Your("%s %s to tingle and you smell %s.", body_part(NOSE),
                        u.uedibility ? "continues" : "starts", what);
                u.uedibility = 1;
            } else
                Your("%s tingles and you smell %s.", body_part(NOSE), what);
        }
        else You("sense %s.", what);
        //display_nhwindow(WIN_MAP, true);
        exercise(A_WIS, true);
        docrt();
        u.uinwater = uw;
        if (Underwater) under_water(2);
        if (u.uburied) under_ground(2);
    }
    return(0);
}

/*
 * Used for scrolls, potions, spells, and crystal balls.  Returns:
 *
 *      1 - nothing was detected
 *      0 - something was detected
 */
/*
   struct obj *detector,      // object doing the detecting 
   int class          // an object class, 0 for all
   */
int object_detect (struct obj *detector, int class) {
    int x, y;
    char stuff[BUFSZ];
    int is_cursed = (detector && detector->cursed);
    int do_dknown = (detector && (detector->oclass == POTION_CLASS ||
                detector->oclass == SPBOOK_CLASS) &&
            detector->blessed);
    int ct = 0, ctu = 0;
    struct obj *obj, *otmp = (struct obj *)0;
    struct monst *mtmp;
    int uw = u.uinwater;
    int sym, boulder = 0;

    if (class < 0 || class >= MAXOCLASSES) {
        impossible("object_detect:  illegal class %d", class);
        class = 0;
    }

    /* Special boulder symbol check - does the class symbol happen
     * to match iflags.bouldersym which is a user-defined?
     * If so, that means we aren't sure what they really wanted to
     * detect. Rather than trump anything, show both possibilities.
     * We can exclude checking the buried obj chain for boulders below.
     */
    sym = class ? def_oc_syms[class] : 0;
    if (sym && iflags.bouldersym && sym == iflags.bouldersym)
        boulder = ROCK_CLASS;

    if (Hallucination() || (Confusion() && class == SCROLL_CLASS))
        strcpy(stuff, something);
    else
        strcpy(stuff, class ? oclass_names[class] : "objects");
    if (boulder && class != ROCK_CLASS) strcat(stuff, " and/or large stones");

    if (do_dknown) for(obj = invent; obj; obj = obj->nobj) do_dknown_of(obj);

    for (obj = fobj; obj; obj = obj->nobj) {
        if ((!class && !boulder) || o_in(obj, class) || o_in(obj, boulder)) {
            if (obj->ox == u.ux && obj->oy == u.uy) ctu++;
            else ct++;
        }
        if (do_dknown) do_dknown_of(obj);
    }

    for (obj = level.buriedobjlist; obj; obj = obj->nobj) {
        if (!class || o_in(obj, class)) {
            if (obj->ox == u.ux && obj->oy == u.uy) ctu++;
            else ct++;
        }
        if (do_dknown) do_dknown_of(obj);
    }

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp)) continue;
        for (obj = mtmp->minvent; obj; obj = obj->nobj) {
            if ((!class && !boulder) || o_in(obj, class) || o_in(obj, boulder)) ct++;
            if (do_dknown) do_dknown_of(obj);
        }
        if ((is_cursed && mtmp->m_ap_type == M_AP_OBJECT &&
                    (!class || class == objects[mtmp->mappearance].oc_class)) ||
                (mtmp->mgold && (!class || class == COIN_CLASS))) {
            ct++;
            break;
        }
    }

    if (!clear_stale_map(!class ? ALL_CLASSES : class, 0) && !ct) {
        if (!ctu) {
            if (detector)
                strange_feeling(detector, "You feel a lack of something.");
            return 1;
        }

        You("sense %s nearby.", stuff);
        return 0;
    }

    cls();

    u.uinwater = 0;
    /*
     *      Map all buried objects first.
     */
    for (obj = level.buriedobjlist; obj; obj = obj->nobj)
        if (!class || (otmp = o_in(obj, class))) {
            if (class) {
                if (otmp != obj) {
                    otmp->ox = obj->ox;
                    otmp->oy = obj->oy;
                }
                map_object(otmp, 1);
            } else
                map_object(obj, 1);
        }
    /*
     * If we are mapping all objects, map only the top object of a pile or
     * the first object in a monster's inventory.  Otherwise, go looking
     * for a matching object class and display the first one encountered
     * at each location.
     *
     * Objects on the floor override buried objects.
     */
    for (x = 1; x < COLNO; x++)
        for (y = 0; y < ROWNO; y++)
            for (obj = level.objects[x][y]; obj; obj = obj->nexthere)
                if ((!class && !boulder) ||
                        (otmp = o_in(obj, class)) || (otmp = o_in(obj, boulder))) {
                    if (class || boulder) {
                        if (otmp != obj) {
                            otmp->ox = obj->ox;
                            otmp->oy = obj->oy;
                        }
                        map_object(otmp, 1);
                    } else
                        map_object(obj, 1);
                    break;
                }

    /* Objects in the monster's inventory override floor objects. */
    for (mtmp = fmon ; mtmp ; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp)) continue;
        for (obj = mtmp->minvent; obj; obj = obj->nobj)
            if ((!class && !boulder) ||
                    (otmp = o_in(obj, class)) || (otmp = o_in(obj, boulder))) {
                if (!class && !boulder) otmp = obj;
                otmp->ox = mtmp->mx;            /* at monster location */
                otmp->oy = mtmp->my;
                map_object(otmp, 1);
                break;
            }
        /* Allow a mimic to override the detected objects it is carrying. */
        if (is_cursed && mtmp->m_ap_type == M_AP_OBJECT &&
                (!class || class == objects[mtmp->mappearance].oc_class)) {
            struct obj temp;

            temp.otyp = mtmp->mappearance;      /* needed for obj_to_glyph() */
            temp.ox = mtmp->mx;
            temp.oy = mtmp->my;
            temp.corpsenm = PM_TENGU;           /* if mimicing a corpse */
            map_object(&temp, 1);
        } else if (mtmp->mgold && (!class || class == COIN_CLASS)) {
            struct obj gold;

            gold.otyp = GOLD_PIECE;
            gold.ox = mtmp->mx;
            gold.oy = mtmp->my;
            map_object(&gold, 1);
        }
    }

    newsym(u.ux,u.uy);
    You("detect the %s of %s.", ct ? "presence" : "absence", stuff);
    //display_nhwindow(WIN_MAP, true);
    /*
     * What are we going to do when the hero does an object detect while blind
     * and the detected object covers a known pool?
     */
    docrt();    /* this will correctly reset vision */

    u.uinwater = uw;
    if (Underwater) under_water(2);
    if (u.uburied) under_ground(2);
    return 0;
}

/*
 * Used by: crystal balls, potions, fountains
 *
 * Returns 1 if nothing was detected.
 * Returns 0 if something was detected.
 struct obj *otmp,        detecting object (if any)
 int mclass                      monster class, 0 for all
 */
int monster_detect (struct obj *otmp, int mclass) {
    struct monst *mtmp;
    int mcnt = 0;


    /* Note: This used to just check fmon for a non-zero value
     * but in versions since 3.3.0 fmon can test true due to the
     * presence of dmons, so we have to find at least one
     * with positive hit-points to know for sure.
     */
    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
        if (!DEADMONSTER(mtmp)) {
            mcnt++;
            break;
        }

    if (!mcnt) {
        if (otmp)
            strange_feeling(otmp, Hallucination() ?
                    "You get the heebie jeebies." :
                    "You feel threatened.");
        return 1;
    } else {
        bool woken = false;

        cls();
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp)) continue;
            if (!mclass || mtmp->data->mlet == mclass ||
                    (mtmp->data == &mons[PM_LONG_WORM] && mclass == S_WORM_TAIL))
                if (mtmp->mx > 0) {
                    if (mclass && def_monsyms[mclass] == ' ')
                        show_glyph(mtmp->mx,mtmp->my,
                                detected_mon_to_glyph(mtmp));
                    else
                        show_glyph(mtmp->mx,mtmp->my,mon_to_glyph(mtmp));
                    /* don't be stingy - display entire worm */
                    if (mtmp->data == &mons[PM_LONG_WORM]) detect_wsegs(mtmp,0);
                }
            if (otmp && otmp->cursed &&
                    (mtmp->msleeping || !mtmp->mcanmove)) {
                mtmp->msleeping = mtmp->mfrozen = 0;
                mtmp->mcanmove = 1;
                woken = true;
            }
        }
        display_self();
        You("sense the presence of monsters.");
        if (woken)
            pline("Monsters sense the presence of you.");
        //display_nhwindow(WIN_MAP, true);
        docrt();
        if (Underwater) under_water(2);
        if (u.uburied) under_ground(2);
    }
    return 0;
}

static void sense_trap (struct trap *trap, signed char x, signed char y, int src_cursed) {
    if (Hallucination() || src_cursed) {
        struct obj obj;                 /* fake object */
        if (trap) {
            obj.ox = trap->tx;
            obj.oy = trap->ty;
        } else {
            obj.ox = x;
            obj.oy = y;
        }
        obj.otyp = (src_cursed) ? GOLD_PIECE : random_object();
        obj.corpsenm = random_monster();        /* if otyp == CORPSE */
        map_object(&obj,1);
    } else if (trap) {
        map_trap(trap,1);
        trap->tseen = 1;
    } else {
        struct trap temp_trap;          /* fake trap */
        temp_trap.tx = x;
        temp_trap.ty = y;
        temp_trap.ttyp = BEAR_TRAP;     /* some kind of trap */
        map_trap(&temp_trap,1);
    }

}

/* the detections are pulled out so they can    */
/* also be used in the crystal ball routine     */
/* returns 1 if nothing was detected            */
/* returns 0 if something was detected          */
/* sobj is null if crystal ball, *scroll if gold detection scroll */
int trap_detect (struct obj *sobj) {
    struct trap *ttmp;
    struct obj *obj;
    int door;
    int uw = u.uinwater;
    bool found = false;
    coord cc;

    for (ttmp = ftrap; ttmp; ttmp = ttmp->ntrap) {
        if (ttmp->tx != u.ux || ttmp->ty != u.uy)
            goto outtrapmap;
        else found = true;
    }
    for (obj = fobj; obj; obj = obj->nobj) {
        if ((obj->otyp==LARGE_BOX || obj->otyp==CHEST) && obj->otrapped) {
            if (obj->ox != u.ux || obj->oy != u.uy)
                goto outtrapmap;
            else found = true;
        }
    }
    for (door = 0; door < doorindex; door++) {
        cc = doors[door];
        if (levl[cc.x][cc.y].flags & D_TRAPPED) {
            if (cc.x != u.ux || cc.y != u.uy)
                goto outtrapmap;
            else found = true;
        }
    }
    if (!found) {
        char buf[42];
        sprintf(buf, "Your %s stop itching.", makeplural(body_part(TOE)));
        strange_feeling(sobj,buf);
        return(1);
    }
    /* traps exist, but only under me - no separate display required */
    Your("%s itch.", makeplural(body_part(TOE)));
    return(0);
outtrapmap:
    cls();

    u.uinwater = 0;
    for (ttmp = ftrap; ttmp; ttmp = ttmp->ntrap)
        sense_trap(ttmp, 0, 0, sobj && sobj->cursed);

    for (obj = fobj; obj; obj = obj->nobj)
        if ((obj->otyp==LARGE_BOX || obj->otyp==CHEST) && obj->otrapped)
            sense_trap((struct trap *)0, obj->ox, obj->oy, sobj && sobj->cursed);

    for (door = 0; door < doorindex; door++) {
        cc = doors[door];
        if (levl[cc.x][cc.y].flags & D_TRAPPED)
            sense_trap((struct trap *)0, cc.x, cc.y, sobj && sobj->cursed);
    }

    newsym(u.ux,u.uy);
    You_feel("%s.", sobj && sobj->cursed ? "very greedy" : "entrapped");
    //display_nhwindow(WIN_MAP, true);
    docrt();
    u.uinwater = uw;
    if (Underwater) under_water(2);
    if (u.uburied) under_ground(2);
    return(0);
}

const char * level_distance(d_level *where) {
    signed char ll = depth(&u.uz) - depth(where);
    bool indun = (u.uz.dnum == where->dnum);

    if (ll < 0) {
        if (ll < (-8 - rn2(3)))
            if (!indun) return "far away";
            else        return "far below";
        else if (ll < -1)
            if (!indun) return "away below you";
            else        return "below you";
        else
            if (!indun) return "in the distance";
            else        return "just below";
    } else if (ll > 0) {
        if (ll > (8 + rn2(3)))
            if (!indun) return "far away";
            else        return "far above";
        else if (ll > 1)
            if (!indun) return "away above you";
            else        return "above you";
        else
            if (!indun) return "in the distance";
            else        return "just above";
    } else
        if (!indun) return "in the distance";
        else        return "near you";
}

void use_crystal_ball (struct obj *obj) {
    char ch;
    int oops;

    if (Blind()) {
        pline("Too bad you can't see %s.", the(xname(obj)));
        return;
    }
    oops = (rnd(20) > ACURR(A_INT) || obj->cursed);
    if (oops && (obj->spe > 0)) {
        switch (rnd(obj->oartifact ? 4 : 5)) {
            case 1:
                {
                    char are_clause[BUFSZ];
                    Tobjnam(are_clause, BUFSZ, obj, "are");
                    pline("%s too much to comprehend!", are_clause);
                }
                break;
            case 2:
                {
                    char confuse_clause[BUFSZ];
                    Tobjnam(confuse_clause, BUFSZ, obj, "confuse");
                    pline("%s you!", confuse_clause);
                    make_confused(get_HConfusion() + rnd(100),false);
                }
                break;
            case 3:
                if (!resists_blnd(&youmonst)) {
                    char damage_clause[BUFSZ];
                    Tobjnam(damage_clause, BUFSZ, obj, "damage");
                    pline("%s your vision!", damage_clause);
                    make_blinded(Blinded + rnd(100),false);
                    if (!Blind()) Your("%s", vision_clears);
                } else {
                    char assault_clause[BUFSZ];
                    Tobjnam(assault_clause, BUFSZ, obj, "assault");
                    pline("%s your vision.", assault_clause);
                    You("are unaffected!");
                }
                break;
            case 4:
                {
                    char zap_clause[BUFSZ];
                    Tobjnam(zap_clause, BUFSZ, obj, "zap");
                    pline("%s your mind!", zap_clause);
                    make_hallucinated(u.uprops[HALLUC].intrinsic + rnd(100),false,0L);
                }
                break;
            case 5:
                {
                    char explode_clause[BUFSZ];
                    Tobjnam(explode_clause, BUFSZ, obj, "explode");
                    pline("%s!", explode_clause);
                    useup(obj);
                    obj = 0;    /* it's gone */
                    losehp(rnd(30), killed_by_const(KM_EXPLODING_CRYSTAL_BALL));
                }
                break;
        }
        if (obj) consume_obj_charge(obj, true);
        return;
    }

    if (Hallucination()) {
        if (!obj->spe) {
            pline("All you see is funky %s haze.", hcolor((char *)0));
        } else {
            switch(rnd(6)) {
                case 1 : You("grok some groovy globs of incandescent lava.");
                         break;
                case 2 : pline("Whoa!  Psychedelic colors, %s!",
                                 poly_gender() == 1 ? "babe" : "dude");
                         break;
                case 3 : pline_The("crystal pulses with sinister %s light!",
                                 hcolor((char *)0));
                         break;
                case 4 : You("see goldfish swimming above fluorescent rocks.");
                         break;
                case 5 : You("see tiny snowflakes spinning around a miniature farmhouse.");
                         break;
                default: pline("Oh wow... like a kaleidoscope!");
                         break;
            }
            consume_obj_charge(obj, true);
        }
        return;
    }

    /* read a single character */
    if (flags.verbose) You("may look for an object or monster symbol.");
    ch = yn_function("What do you look for?", (char *)0, '\0');
    /* Don't filter out ' ' here; it has a use */
    if ((ch != def_monsyms[S_GHOST]) && index(quitchars,ch)) {
        if (flags.verbose) plines(Never_mind);
        return;
    }
    You("peer into %s...", the(xname(obj)));
    nomul(-rnd(10));
    nomovemsg = "";
    if (obj->spe <= 0)
        pline_The("vision is unclear.");
    else {
        int class;
        int ret = 0;

        makeknown(CRYSTAL_BALL);
        consume_obj_charge(obj, true);

        /* special case: accept ']' as synonym for mimic
         * we have to do this before the def_char_to_objclass check
         */
        if (ch == DEF_MIMIC_DEF) ch = DEF_MIMIC;

        if ((class = def_char_to_objclass(ch)) != MAXOCLASSES)
            ret = object_detect((struct obj *)0, class);
        else if ((class = def_char_to_monclass(ch)) != MAXMCLASSES)
            ret = monster_detect((struct obj *)0, class);
        else if (iflags.bouldersym && (ch == iflags.bouldersym))
            ret = object_detect((struct obj *)0, ROCK_CLASS);
        else switch(ch) {
            case '^':
                ret = trap_detect((struct obj *)0);
                break;
            default:
                {
                    int i = rn2(SIZE(level_detects));
                    You("see %s, %s.",
                            level_detects[i].what,
                            level_distance(level_detects[i].where));
                }
                ret = 0;
                break;
        }

        if (ret) {
            if (!rn2(100))  /* make them nervous */
                You("see the Wizard of Yendor gazing out at you.");
            else pline_The("vision is unclear.");
        }
    }
    return;
}

static void show_map_spot (int x, int y) {
    struct rm *lev;

    if (Confusion() && rn2(7)) return;
    lev = &levl[x][y];

    lev->seenv = SVALL;

    /* Secret corridors are found, but not secret doors. */
    if (lev->typ == SCORR) {
        lev->typ = CORR;
        unblock_point(x,y);
    }

    /* if we don't remember an object or trap there, map it */
    if (lev->typ == ROOM ?
            (glyph_is_cmap(lev->glyph) && !glyph_is_trap(lev->glyph) &&
             glyph_to_cmap(lev->glyph) != ROOM) :
            (!glyph_is_object(lev->glyph) && !glyph_is_trap(lev->glyph))) {
        if (level.flags.hero_memory) {
            magic_map_background(x,y,0);
            newsym(x,y);                        /* show it, if not blocked */
        } else {
            magic_map_background(x,y,1);        /* display it */
        }
    }
}

void do_mapping (void) {
    int zx, zy;
    int uw = u.uinwater;

    u.uinwater = 0;
    for (zx = 1; zx < COLNO; zx++)
        for (zy = 0; zy < ROWNO; zy++)
            show_map_spot(zx, zy);
    exercise(A_WIS, true);
    u.uinwater = uw;
    if (!level.flags.hero_memory || Underwater) {
        flush_screen(1);                        /* flush temp screen */
        //display_nhwindow(WIN_MAP, true);        /* wait */
        docrt();
    }
}

void do_vicinity_map (void) {
    int zx, zy;
    int lo_y = (u.uy-5 < 0 ? 0 : u.uy-5),
        hi_y = (u.uy+6 > ROWNO ? ROWNO : u.uy+6),
        lo_x = (u.ux-9 < 1 ? 1 : u.ux-9),       /* avoid column 0 */
        hi_x = (u.ux+10 > COLNO ? COLNO : u.ux+10);

    for (zx = lo_x; zx < hi_x; zx++)
        for (zy = lo_y; zy < hi_y; zy++)
            show_map_spot(zx, zy);

    if (!level.flags.hero_memory || Underwater) {
        flush_screen(1);                        /* flush temp screen */
        display_nhwindow(WIN_MAP, true);        /* wait */
        docrt();
    }
}

/* convert a secret door into a normal door */
void cvt_sdoor_to_door (struct rm *lev) {
    int newmask = lev->flags & ~WM_MASK;

    /* newly exposed door is closed */
    if (!(newmask & D_LOCKED)) newmask |= D_CLOSED;

    lev->typ = DOOR;
    lev->flags = newmask;
}

static void findone (int zx, int zy, void *num) {
    struct trap *ttmp;
    struct monst *mtmp;

    if(levl[zx][zy].typ == SDOOR) {
        cvt_sdoor_to_door(&levl[zx][zy]);       /* .typ = DOOR */
        magic_map_background(zx, zy, 0);
        newsym(zx, zy);
        (*(int*)num)++;
    } else if(levl[zx][zy].typ == SCORR) {
        levl[zx][zy].typ = CORR;
        unblock_point(zx,zy);
        magic_map_background(zx, zy, 0);
        newsym(zx, zy);
        (*(int*)num)++;
    } else if ((ttmp = t_at(zx, zy)) != 0) {
        if(!ttmp->tseen && ttmp->ttyp != STATUE_TRAP) {
            ttmp->tseen = 1;
            newsym(zx,zy);
            (*(int*)num)++;
        }
    } else if ((mtmp = m_at(zx, zy)) != 0) {
        if(mtmp->m_ap_type) {
            seemimic(mtmp);
            (*(int*)num)++;
        }
        if (mtmp->mundetected &&
                (is_hider(mtmp->data) || mtmp->data->mlet == S_EEL)) {
            mtmp->mundetected = 0;
            newsym(zx, zy);
            (*(int*)num)++;
        }
        if (!canspotmon(mtmp) &&
                !glyph_is_invisible(levl[zx][zy].glyph))
            map_invisible(zx, zy);
    } else if (glyph_is_invisible(levl[zx][zy].glyph)) {
        unmap_object(zx, zy);
        newsym(zx, zy);
        (*(int*)num)++;
    }
}

static void openone (int zx, int zy, void *num) {
    struct trap *ttmp;
    struct obj *otmp;

    if(OBJ_AT(zx, zy)) {
        for(otmp = level.objects[zx][zy];
                otmp; otmp = otmp->nexthere) {
            if(Is_box(otmp) && otmp->olocked) {
                otmp->olocked = 0;
                (*(int*)num)++;
            }
        }
        /* let it fall to the next cases. could be on trap. */
    }
    if(levl[zx][zy].typ == SDOOR || (levl[zx][zy].typ == DOOR &&
                (levl[zx][zy].flags & (D_CLOSED|D_LOCKED)))) {
        if(levl[zx][zy].typ == SDOOR)
            cvt_sdoor_to_door(&levl[zx][zy]);   /* .typ = DOOR */
        if(levl[zx][zy].flags & D_TRAPPED) {
            if(distu(zx, zy) < 3) b_trapped("door", 0);
            else Norep("You %s an explosion!",
                    cansee(zx, zy) ? "see" :
                    (flags.soundok ? "hear" :
                     "feel the shock of"));
            wake_nearto(zx, zy, 11*11);
            levl[zx][zy].flags = D_NODOOR;
        } else
            levl[zx][zy].flags = D_ISOPEN;
        unblock_point(zx, zy);
        newsym(zx, zy);
        (*(int*)num)++;
    } else if(levl[zx][zy].typ == SCORR) {
        levl[zx][zy].typ = CORR;
        unblock_point(zx, zy);
        newsym(zx, zy);
        (*(int*)num)++;
    } else if ((ttmp = t_at(zx, zy)) != 0) {
        if (!ttmp->tseen && ttmp->ttyp != STATUE_TRAP) {
            ttmp->tseen = 1;
            newsym(zx,zy);
            (*(int*)num)++;
        }
    } else if (find_drawbridge(&zx, &zy)) {
        /* make sure it isn't an open drawbridge */
        open_drawbridge(zx, zy);
        (*(int*)num)++;
    }
}

/* returns number of things found */
int findit (void) {
    int num = 0;

    if(u.uswallow) return(0);
    do_clear_area(u.ux, u.uy, BOLT_LIM, findone, (void *) &num);
    return(num);
}

/* returns number of things found and opened */
int openit (void) {
    int num = 0;

    if(u.uswallow) {
        if (is_animal(u.ustuck->data)) {
            if (Blind()) {
                pline("Its mouth opens!");
            } else {
                char subject[BUFSZ];
                Monnam(subject, BUFSZ, u.ustuck);
                pline("%s opens its mouth!", subject);
            }
        }
        expels(u.ustuck, u.ustuck->data, true);
        return(-1);
    }

    do_clear_area(u.ux, u.uy, BOLT_LIM, openone, (void *) &num);
    return(num);
}

void find_trap (struct trap *trap) {
    int tt = what_trap(trap->ttyp);
    bool cleared = false;

    trap->tseen = 1;
    exercise(A_WIS, true);
    if (Blind())
        feel_location(trap->tx, trap->ty);
    else
        newsym(trap->tx, trap->ty);

    if (levl[trap->tx][trap->ty].glyph != trap_to_glyph(trap)) {
        /* There's too much clutter to see your find otherwise */
        cls();
        map_trap(trap, 1);
        display_self();
        cleared = true;
    }

    You("find %s.", an(defsyms[trap_to_defsym(tt)].explanation));

    if (cleared) {
        display_nhwindow(WIN_MAP, true);        /* wait */
        docrt();
    }
}

int dosearch0 (int aflag) {
    signed char x, y;
    struct trap *trap;
    struct monst *mtmp;

    if(u.uswallow) {
        if (!aflag)
            pline("What are you looking for?  The exit?");
    } else {
        int fund = (uwep && uwep->oartifact &&
                spec_ability(uwep, SPFX_SEARCH)) ?
            uwep->spe : 0;
        if (ublindf && ublindf->otyp == LENSES && !Blind())
            fund += 2; /* JDS: lenses help searching */
        if (fund > 5) fund = 5;
        for(x = u.ux-1; x < u.ux+2; x++)
            for(y = u.uy-1; y < u.uy+2; y++) {
                if(!isok(x,y)) continue;
                if(x != u.ux || y != u.uy) {
                    if (Blind() && !aflag) feel_location(x,y);
                    if(levl[x][y].typ == SDOOR) {
                        if(rnl(7-fund)) continue;
                        cvt_sdoor_to_door(&levl[x][y]); /* .typ = DOOR */
                        exercise(A_WIS, true);
                        nomul(0);
                        if (Blind() && !aflag)
                            feel_location(x,y); /* make sure it shows up */
                        else
                            newsym(x,y);
                    } else if(levl[x][y].typ == SCORR) {
                        if(rnl(7-fund)) continue;
                        levl[x][y].typ = CORR;
                        unblock_point(x,y);     /* vision */
                        exercise(A_WIS, true);
                        nomul(0);
                        newsym(x,y);
                    } else {
                        /* Be careful not to find anything in an SCORR or SDOOR */
                        if((mtmp = m_at(x, y)) && !aflag) {
                            if(mtmp->m_ap_type) {
                                seemimic(mtmp);
find:                           exercise(A_WIS, true);
                                if (!canspotmon(mtmp)) {
                                    if (glyph_is_invisible(levl[x][y].glyph)) {
                                        /* found invisible monster in a square
                                         * which already has an 'I' in it.
                                         * Logically, this should still take
                                         * time and lead to a return(1), but if
                                         * we did that the player would keep
                                         * finding the same monster every turn.
                                         */
                                        continue;
                                    } else {
                                        You_feel("an unseen monster!");
                                        map_invisible(x, y);
                                    }
                                } else if (!sensemon(mtmp)) {
                                    char name[BUFSZ];
                                    a_monnam(name, BUFSZ, mtmp);
                                    You("find %s.", name);
                                }
                                return 1;
                            }
                            if(!canspotmon(mtmp)) {
                                if (mtmp->mundetected &&
                                        (is_hider(mtmp->data) || mtmp->data->mlet == S_EEL))
                                    mtmp->mundetected = 0;
                                newsym(x,y);
                                goto find;
                            }
                        }

                        /* see if an invisible monster has moved--if Blind,
                         * feel_location() already did it
                         */
                        if (!aflag && !mtmp && !Blind() &&
                                glyph_is_invisible(levl[x][y].glyph)) {
                            unmap_object(x,y);
                            newsym(x,y);
                        }

                        if ((trap = t_at(x,y)) && !trap->tseen && !rnl(8)) {
                            nomul(0);

                            if (trap->ttyp == STATUE_TRAP) {
                                if (activate_statue_trap(trap, x, y, false))
                                    exercise(A_WIS, true);
                                return(1);
                            } else {
                                find_trap(trap);
                            }
                        }
                    }
                }
            }
    }
    return 1;
}

int dosearch (void) {
    return(dosearch0(0));
}

/* Pre-map the sokoban levels */
void sokoban_detect (void) {
    int x, y;
    struct trap *ttmp;
    struct obj *obj;

    /* Map the background and boulders */
    for (x = 1; x < COLNO; x++)
        for (y = 0; y < ROWNO; y++) {
            levl[x][y].seenv = SVALL;
            levl[x][y].waslit = true;
            map_background(x, y, 1);
            for (obj = level.objects[x][y]; obj; obj = obj->nexthere)
                if (obj->otyp == BOULDER)
                    map_object(obj, 1);
        }

    /* Map the traps */
    for (ttmp = ftrap; ttmp; ttmp = ttmp->ntrap) {
        ttmp->tseen = 1;
        map_trap(ttmp, 1);
    }
}
