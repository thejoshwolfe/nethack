/* See LICENSE in the root of this project for change info */

#include "dig.h"

#include <stdio.h>
#include <string.h>

#include "rm_util.h"
#include "display_util.h"
#include "move.h"
#include "dungeon_util.h"
#include "align.h"
#include "apply.h"
#include "attrib.h"
#include "cmd.h"
#include "coord.h"
#include "dbridge.h"
#include "decl.h"
#include "detect.h"
#include "display.h"
#include "do.h"
#include "do_name.h"
#include "dog.h"
#include "dokick.h"
#include "dungeon.h"
#include "engrave.h"
#include "flag.h"
#include "fountain.h"
#include "global.h"
#include "hack.h"
#include "hacklib.h"
#include "invent.h"
#include "makemon.h"
#include "mhitu.h"
#include "mkobj.h"
#include "mkroom.h"
#include "mon.h"
#include "mondata.h"
#include "monflag.h"
#include "monmove.h"
#include "monst.h"
#include "monsym.h"
#include "obj.h"
#include "objclass.h"
#include "objnam.h"
#include "onames.h"
#include "permonst.h"
#include "pickup.h"
#include "pline.h"
#include "pm.h"
#include "polyself.h"
#include "pray.h"
#include "priest.h"
#include "prop.h"
#include "read.h"
#include "rm.h"
#include "rnd.h"
#include "shk.h"
#include "teleport.h"
#include "timeout.h"
#include "trap.h"
#include "uhitm.h"
#include "util.h"
#include "vision.h"
#include "weapon.h"
#include "wield.h"
#include "worm.h"
#include "you.h"
#include "youprop.h"
#include "zap.h"

static bool did_dig_msg;

/* Indices returned by dig_typ() */
enum {
    DIGTYP_UNDIGGABLE = 0,
    DIGTYP_ROCK       = 1,
    DIGTYP_STATUE     = 2,
    DIGTYP_BOULDER    = 3,
    DIGTYP_DOOR       = 4,
    DIGTYP_TREE       = 5,
};

#define BY_YOU          (&youmonst)
#define BY_OBJECT       ((struct monst *)0)


static bool rm_waslit (void) {
    signed char x, y;

    if(levl[u.ux][u.uy].typ == ROOM && levl[u.ux][u.uy].waslit)
        return(true);
    for(x = u.ux-2; x < u.ux+3; x++)
        for(y = u.uy-1; y < u.uy+2; y++)
            if(isok(x,y) && levl[x][y].waslit) return(true);
    return(false);
}

/* Change level topology.  Messes with vision tables and ignores things like
 * boulders in the name of a nice effect.  Vision will get fixed up again
 * immediately after the effect is complete.
 */
static void mkcavepos(signed char x, signed char y, int dist, bool waslit, bool rockit) {
    struct rm *lev;

    if(!isok(x,y)) return;
    lev = &levl[x][y];

    if(rockit) {
        struct monst *mtmp;

        if(IS_ROCK(lev->typ)) return;
        if(t_at(x, y)) return; /* don't cover the portal */
        if ((mtmp = m_at(x, y)) != 0)   /* make sure crucial monsters survive */
            if(!passes_walls(mtmp->data)) (void) rloc(mtmp, false);
    } else if(lev->typ == ROOM) return;

    unblock_point(x,y); /* make sure vision knows this location is open */

    /* fake out saved state */
    lev->seenv = 0;
    lev->flags = 0;
    if(dist < 3) lev->lit = (rockit ? false : true);
    if(waslit) lev->waslit = (rockit ? false : true);
    lev->horizontal = false;
    viz_array[y][x] = (dist < 3 ) ?
        (IN_SIGHT|COULD_SEE) : /* short-circuit vision recalc */
        COULD_SEE;
    lev->typ = (rockit ? STONE : ROOM);
    if(dist >= 3)
        impossible("mkcavepos called with dist %d", dist);
    if(Blind())
        feel_location(x, y);
    else newsym(x,y);
}

static void mkcavearea(bool rockit) {
    int dist;
    signed char xmin = u.ux, xmax = u.ux;
    signed char ymin = u.uy, ymax = u.uy;
    signed char i;
    bool waslit = rm_waslit();

    if(rockit) pline("Crash!  The ceiling collapses around you!");
    else pline("A mysterious force %s cave around you!",
             (levl[u.ux][u.uy].typ == CORR) ? "creates a" : "extends the");
    display_nhwindow(WIN_MESSAGE, true);

    for(dist = 1; dist <= 2; dist++) {
        xmin--; xmax++;

        /* top and bottom */
        if(dist < 2) { /* the area is wider that it is high */
            ymin--; ymax++;
            for(i = xmin+1; i < xmax; i++) {
                mkcavepos(i, ymin, dist, waslit, rockit);
                mkcavepos(i, ymax, dist, waslit, rockit);
            }
        }

        /* left and right */
        for(i = ymin; i <= ymax; i++) {
            mkcavepos(xmin, i, dist, waslit, rockit);
            mkcavepos(xmax, i, dist, waslit, rockit);
        }

        flush_screen(1);        /* make sure the new glyphs shows up */
    }

    if(!rockit && levl[u.ux][u.uy].typ == CORR) {
        levl[u.ux][u.uy].typ = ROOM;
        if(waslit) levl[u.ux][u.uy].waslit = true;
        newsym(u.ux, u.uy); /* in case player is invisible */
    }

    vision_full_recalc = 1;     /* everything changed */
}

/* When digging into location <x,y>, what are you actually digging into? */
static int
dig_typ (struct obj *otmp, signed char x, signed char y)
{
        bool ispick = is_pick(otmp);

        return (ispick && sobj_at(STATUE, x, y) ? DIGTYP_STATUE :
                ispick && sobj_at(BOULDER, x, y) ? DIGTYP_BOULDER :
                closed_door(x, y) ? DIGTYP_DOOR :
                IS_TREE(levl[x][y].typ) ?
                        (ispick ? DIGTYP_UNDIGGABLE : DIGTYP_TREE) :
                ispick && IS_ROCK(levl[x][y].typ) &&
                        (!level.flags.arboreal || IS_WALL(levl[x][y].typ)) ?
                        DIGTYP_ROCK : DIGTYP_UNDIGGABLE);
}

bool dig_check(struct monst *madeby, bool verbose, int x, int y) {
        struct trap *ttmp = t_at(x, y);
        const char *verb = (madeby == BY_YOU && uwep && is_axe(uwep)) ? "chop" : "dig in";

        if (On_stairs(x, y)) {
            if (x == xdnladder || x == xupladder) {
                if(verbose) pline_The("ladder resists your effort.");
            } else if(verbose) pline_The("stairs are too hard to %s.", verb);
            return(false);
        } else if (IS_THRONE(levl[x][y].typ) && madeby != BY_OBJECT) {
            if(verbose) pline_The("throne is too hard to break apart.");
            return(false);
        } else if (IS_ALTAR(levl[x][y].typ) && (madeby != BY_OBJECT ||
                                Is_astralevel(&u.uz) || Is_sanctum(&u.uz))) {
            if(verbose) pline_The("altar is too hard to break apart.");
            return(false);
        } else if (Is_airlevel(&u.uz)) {
            if(verbose) You("cannot %s thin air.", verb);
            return(false);
        } else if (Is_waterlevel(&u.uz)) {
            if(verbose) pline_The("water splashes and subsides.");
            return(false);
        } else if ((IS_ROCK(levl[x][y].typ) && levl[x][y].typ != SDOOR &&
                      (levl[x][y].flags & W_NONDIGGABLE) != 0)
                || (ttmp &&
                      (ttmp->ttyp == MAGIC_PORTAL || !Can_dig_down(&u.uz)))) {
            if(verbose) pline_The("%s here is too hard to %s.",
                                  surface(x,y), verb);
            return(false);
        } else if (sobj_at(BOULDER, x, y)) {
            if(verbose) There("isn't enough room to %s here.", verb);
            return(false);
        } else if (madeby == BY_OBJECT &&
                    /* the block against existing traps is mainly to
                       prevent broken wands from turning holes into pits */
                    (ttmp || is_pool(x,y) || is_lava(x,y))) {
            /* digging by player handles pools separately */
            return false;
        }
        return(true);
}

static int dig (void) {
    struct rm *lev;
    signed char dpx = digging.pos.x, dpy = digging.pos.y;
    bool ispick = uwep && is_pick(uwep);
    const char *verb =
        (!uwep || is_pick(uwep)) ? "dig into" : "chop through";

    lev = &levl[dpx][dpy];
    /* perhaps a nymph stole your pick-axe while you were busy digging */
    /* or perhaps you teleported away */
    if (u.uswallow || !uwep || (!ispick && !is_axe(uwep)) ||
            !on_level(&digging.level, &u.uz) ||
            ((digging.down ? (dpx != u.ux || dpy != u.uy)
              : (distu(dpx,dpy) > 2))))
        return(0);

    if (digging.down) {
        if(!dig_check(BY_YOU, true, u.ux, u.uy)) return(0);
    } else { /* !digging.down */
        if (IS_TREE(lev->typ) && !may_dig(dpx,dpy) &&
                dig_typ(uwep, dpx, dpy) == DIGTYP_TREE) {
            pline("This tree seems to be petrified.");
            return(0);
        }
        if (IS_ROCK(lev->typ) && !may_dig(dpx,dpy) &&
                dig_typ(uwep, dpx, dpy) == DIGTYP_ROCK) {
            pline("This wall is too hard to %s.", verb);
            return(0);
        }
    }
    if(Fumbling() && !rn2(3)) {
        switch(rn2(3)) {
            case 0:
                if(!welded(uwep)) {
                    You("fumble and drop your %s.", xname(uwep));
                    dropx(uwep);
                } else {
                    char hit_tense[BUFSZ];
                    otense(hit_tense, BUFSZ, uwep, "hit");
                    char bounce_tense[BUFSZ];
                    otense(bounce_tense, BUFSZ, uwep, "bounce");
                    if (u.usteed) {
                        char steed_name[BUFSZ];
                        mon_nam(steed_name, BUFSZ, u.usteed);
                        Your("%s %s and %s %s!", xname(uwep), bounce_tense, hit_tense, steed_name);
                    } else {
                        pline("Ouch!  Your %s %s and %s you!", xname(uwep), bounce_tense, hit_tense);
                    }
                    set_wounded_legs(RIGHT_SIDE, 5 + rnd(5));
                }
                break;
            case 1:
                pline("Bang!  You hit with the broad side of %s!",
                        the(xname(uwep)));
                break;
            default: Your("swing misses its mark.");
                     break;
        }
        return(0);
    }

    digging.effort += 10 + rn2(5) + abon() +
        uwep->spe - greatest_erosion(uwep) + u.udaminc;
    if (Race_if(PM_DWARF))
        digging.effort *= 2;
    if (digging.down) {
        struct trap *ttmp;

        if (digging.effort > 250) {
            (void) dighole(false);
            (void) memset((void *)&digging, 0, sizeof digging);
            return(0);  /* done with digging */
        }

        if (digging.effort <= 50 ||
                ((ttmp = t_at(dpx,dpy)) != 0 &&
                 (ttmp->ttyp == PIT || ttmp->ttyp == SPIKED_PIT ||
                  ttmp->ttyp == TRAPDOOR || ttmp->ttyp == HOLE)))
            return(1);

        if (IS_ALTAR(lev->typ)) {
            altar_wrath(dpx, dpy);
            angry_priest();
        }

        if (dighole(true)) {    /* make pit at <u.ux,u.uy> */
            digging.level.dnum = 0;
            digging.level.dlevel = -1;
        }
        return(0);
    }

    if (digging.effort > 100) {
        const char *digtxt, *dmgtxt = (const char*) 0;
        struct obj *obj;
        bool shopedge = *in_rooms(dpx, dpy, SHOPBASE);

        if ((obj = sobj_at(STATUE, dpx, dpy)) != 0) {
            if (break_statue(obj))
                digtxt = "The statue shatters.";
            else
                /* it was a statue trap; break_statue()
                 * printed a message and updated the screen
                 */
                digtxt = (char *)0;
        } else if ((obj = sobj_at(BOULDER, dpx, dpy)) != 0) {
            struct obj *bobj;

            fracture_rock(obj);
            if ((bobj = sobj_at(BOULDER, dpx, dpy)) != 0) {
                /* another boulder here, restack it to the top */
                obj_extract_self(bobj);
                place_object(bobj, dpx, dpy);
            }
            digtxt = "The boulder falls apart.";
        } else if (lev->typ == STONE || lev->typ == SCORR ||
                IS_TREE(lev->typ)) {
            if(Is_earthlevel(&u.uz)) {
                if(uwep->blessed && !rn2(3)) {
                    mkcavearea(false);
                    goto cleanup;
                } else if((uwep->cursed && !rn2(4)) ||
                        (!uwep->blessed && !rn2(6))) {
                    mkcavearea(true);
                    goto cleanup;
                }
            }
            if (IS_TREE(lev->typ)) {
                digtxt = "You cut down the tree.";
                lev->typ = ROOM;
                if (!rn2(5)) (void) rnd_treefruit_at(dpx, dpy);
            } else {
                digtxt = "You succeed in cutting away some rock.";
                lev->typ = CORR;
            }
        } else if(IS_WALL(lev->typ)) {
            if(shopedge) {
                add_damage(dpx, dpy, 10L * ACURRSTR);
                dmgtxt = "damage";
            }
            if (level.flags.is_maze_lev) {
                lev->typ = ROOM;
            } else if (level.flags.is_cavernous_lev &&
                    !in_town(dpx, dpy)) {
                lev->typ = CORR;
            } else {
                lev->typ = DOOR;
                lev->flags = D_NODOOR;
            }
            digtxt = "You make an opening in the wall.";
        } else if(lev->typ == SDOOR) {
            cvt_sdoor_to_door(lev); /* ->typ = DOOR */
            digtxt = "You break through a secret door!";
            if(!(lev->flags & D_TRAPPED))
                lev->flags = D_BROKEN;
        } else if(closed_door(dpx, dpy)) {
            digtxt = "You break through the door.";
            if(shopedge) {
                add_damage(dpx, dpy, 400L);
                dmgtxt = "break";
            }
            if(!(lev->flags & D_TRAPPED))
                lev->flags = D_BROKEN;
        } else return(0); /* statue or boulder got taken */

        if(!does_block(dpx,dpy,&levl[dpx][dpy]))
            unblock_point(dpx,dpy);     /* vision:  can see through */
        if(Blind())
            feel_location(dpx, dpy);
        else
            newsym(dpx, dpy);
        if(digtxt && !digging.quiet) plines(digtxt); /* after newsym */
        if(dmgtxt)
            pay_for_damage(dmgtxt, false);

        if(Is_earthlevel(&u.uz) && !rn2(3)) {
            struct monst *mtmp;

            switch(rn2(2)) {
                case 0:
                    mtmp = makemon(&mons[PM_EARTH_ELEMENTAL],
                            dpx, dpy, NO_MM_FLAGS);
                    break;
                default:
                    mtmp = makemon(&mons[PM_XORN],
                            dpx, dpy, NO_MM_FLAGS);
                    break;
            }
            if(mtmp) pline_The("debris from your digging comes to life!");
        }
        if(IS_DOOR(lev->typ) && (lev->flags & D_TRAPPED)) {
            lev->flags = D_NODOOR;
            b_trapped("door", 0);
            newsym(dpx, dpy);
        }
cleanup:
        digging.lastdigtime = moves;
        digging.quiet = false;
        digging.level.dnum = 0;
        digging.level.dlevel = -1;
        return(0);
    } else {                /* not enough effort has been spent yet */
        static const char *const d_target[6] = {
            "", "rock", "statue", "boulder", "door", "tree"
        };
        int dig_target = dig_typ(uwep, dpx, dpy);

        if (IS_WALL(lev->typ) || dig_target == DIGTYP_DOOR) {
            if(*in_rooms(dpx, dpy, SHOPBASE)) {
                pline("This %s seems too hard to %s.",
                        IS_DOOR(lev->typ) ? "door" : "wall", verb);
                return(0);
            }
        } else if (!IS_ROCK(lev->typ) && dig_target == DIGTYP_ROCK)
            return(0); /* statue or boulder got taken */
        if(!did_dig_msg) {
            You("hit the %s with all your might.",
                    d_target[dig_target]);
            did_dig_msg = true;
        }
    }
    return(1);
}

bool is_digging (void) {
    if (occupation == dig) {
        return true;
    }
    return false;
}

/* When will hole be finished? Very rough indication used by shopkeeper. */
int holetime (void) {
    if(occupation != dig || !*u.ushops) return(-1);
    return ((250 - digging.effort) / 20);
}

/* Return typ of liquid to fill a hole with, or ROOM, if no liquid nearby */
static signed char fillholetyp(int x,int y) {
    int x1, y1;
    int lo_x = max(1,x-1), hi_x = min(x+1,COLNO-1),
        lo_y = max(0,y-1), hi_y = min(y+1,ROWNO-1);
    int pool_cnt = 0, moat_cnt = 0, lava_cnt = 0;

    for (x1 = lo_x; x1 <= hi_x; x1++)
        for (y1 = lo_y; y1 <= hi_y; y1++)
            if (levl[x1][y1].typ == POOL)
                pool_cnt++;
            else if (levl[x1][y1].typ == MOAT ||
                    (levl[x1][y1].typ == DRAWBRIDGE_UP &&
                        (levl[x1][y1].flags & DB_UNDER) == DB_MOAT))
                moat_cnt++;
            else if (levl[x1][y1].typ == LAVAPOOL ||
                    (levl[x1][y1].typ == DRAWBRIDGE_UP &&
                        (levl[x1][y1].flags & DB_UNDER) == DB_LAVA))
                lava_cnt++;
    pool_cnt /= 3;              /* not as much liquid as the others */

    if (lava_cnt > moat_cnt + pool_cnt && rn2(lava_cnt + 1))
        return LAVAPOOL;
    else if (moat_cnt > 0 && rn2(moat_cnt + 1))
        return MOAT;
    else if (pool_cnt > 0 && rn2(pool_cnt + 1))
        return POOL;
    else
        return ROOM;
}

void
digactualhole (int x, int y, struct monst *madeby, int ttyp)
{
        struct obj *oldobjs, *newobjs;
        struct trap *ttmp;
        char surface_type[BUFSZ];
        struct rm *lev = &levl[x][y];
        bool shopdoor;
        struct monst *mtmp = m_at(x, y);        /* may be madeby */
        bool madeby_u = (madeby == BY_YOU);
        bool madeby_obj = (madeby == BY_OBJECT);
        bool at_u = (x == u.ux) && (y == u.uy);
        bool wont_fall = Levitation || Flying;

        if (u.utrap && u.utraptype == TT_INFLOOR) u.utrap = 0;

        /* these furniture checks were in dighole(), but wand
           breaking bypasses that routine and calls us directly */
        if (IS_FOUNTAIN(lev->typ)) {
            dogushforth(false);
            SET_FOUNTAIN_WARNED(x,y);           /* force dryup */
            dryup(x, y, madeby_u);
            return;
        } else if (IS_SINK(lev->typ)) {
            breaksink(x, y);
            return;
        } else if (lev->typ == DRAWBRIDGE_DOWN ||
                   (is_drawbridge_wall(x, y) >= 0)) {
            int bx = x, by = y;
            /* if under the portcullis, the bridge is adjacent */
            (void) find_drawbridge(&bx, &by);
            destroy_drawbridge(bx, by);
            return;
        }

        if (ttyp != PIT && !Can_dig_down(&u.uz)) {
            impossible("digactualhole: can't dig %s on this level.",
                       defsyms[trap_to_defsym(ttyp)].explanation);
            ttyp = PIT;
        }

        /* maketrap() might change it, also, in this situation,
           surface() returns an inappropriate string for a grave */
        if (IS_GRAVE(lev->typ))
            strcpy(surface_type, "grave");
        else
            strcpy(surface_type, surface(x,y));
        shopdoor = IS_DOOR(lev->typ) && *in_rooms(x, y, SHOPBASE);
        oldobjs = level.objects[x][y];
        ttmp = maketrap(x, y, ttyp);
        if (!ttmp) return;
        newobjs = level.objects[x][y];
        ttmp->tseen = (madeby_u || cansee(x,y));
        ttmp->madeby_u = madeby_u;
        newsym(ttmp->tx,ttmp->ty);

        if (ttyp == PIT) {

            if(madeby_u) {
                You("dig a pit in the %s.", surface_type);
                if (shopdoor) pay_for_damage("ruin", false);
            } else if (!madeby_obj && canseemon(madeby)) {
                char name[BUFSZ];
                Monnam(name, BUFSZ, madeby);
                pline("%s digs a pit in the %s.", name, surface_type);
            } else if (cansee(x, y) && flags.verbose) {
                pline("A pit appears in the %s.", surface_type);
            }

            if(at_u) {
                if (!wont_fall) {
                    if (!Passes_walls)
                        u.utrap = rn1(4,2);
                    u.utraptype = TT_PIT;
                    vision_full_recalc = 1;     /* vision limits change */
                } else
                    u.utrap = 0;
                if (oldobjs != newobjs) /* something unearthed */
                        notice_stuff_here();       /* detects pit */
            } else if(mtmp) {
                if(is_flyer(mtmp->data) || is_floater(mtmp->data)) {
                    if(canseemon(mtmp)) {
                        char name[BUFSZ];
                        Monnam(name, BUFSZ, mtmp);
                        const char *verb = (is_flyer(mtmp->data)) ?  "flies" : "floats";
                        pline("%s %s over the pit.", name, verb);
                    }
                } else if(mtmp != madeby) {
                    mintrap(mtmp);
                }
            }
        } else {        /* was TRAPDOOR now a HOLE*/
            if (madeby_u) {
                You("dig a hole through the %s.", surface_type);
            } else if(!madeby_obj && canseemon(madeby)) {
                char name[BUFSZ];
                Monnam(name, BUFSZ, madeby);
                pline("%s digs a hole through the %s.", name, surface_type);
            } else if(cansee(x, y) && flags.verbose) {
                pline("A hole appears in the %s.", surface_type);
            }

            if (at_u) {
                if (!u.ustuck && !wont_fall && !next_to_u()) {
                    You("are jerked back by your pet!");
                    wont_fall = true;
                }

                /* Floor objects get a chance of falling down.  The case where
                 * the hero does NOT fall down is treated here.  The case
                 * where the hero does fall down is treated in goto_level().
                 */
                if (u.ustuck || wont_fall) {
                    if (newobjs)
                        impact_drop((struct obj *)0, x, y, 0);
                    if (oldobjs != newobjs)
                        notice_stuff_here();
                    if (shopdoor && madeby_u) pay_for_damage("ruin", false);

                } else {
                    d_level newlevel;

                    if (*u.ushops && madeby_u)
                        shopdig(1); /* shk might snatch pack */
                    /* handle earlier damage, eg breaking wand of digging */
                    else if (!madeby_u) pay_for_damage("dig into", true);

                    You("fall through...");
                    /* Earlier checks must ensure that the destination
                     * level exists and is in the present dungeon.
                     */
                    newlevel.dnum = u.uz.dnum;
                    newlevel.dlevel = u.uz.dlevel + 1;
                    goto_level(&newlevel, false, true, false);
                    /* messages for arriving in special rooms */
                    spoteffects(false);
                }
            } else {
                if (shopdoor && madeby_u) pay_for_damage("ruin", false);
                if (newobjs)
                    impact_drop((struct obj *)0, x, y, 0);
                if (mtmp) {
                     /*[don't we need special sokoban handling here?]*/
                    if (is_flyer(mtmp->data) || is_floater(mtmp->data) ||
                        mtmp->data == &mons[PM_WUMPUS] ||
                        (mtmp->wormno && count_wsegs(mtmp) > 5) ||
                        mtmp->data->msize >= MZ_HUGE) return;
                    if (mtmp == u.ustuck)       /* probably a vortex */
                            return;             /* temporary? kludge */

                    if (teleport_pet(mtmp, false)) {
                        d_level tolevel;

                        if (Is_stronghold(&u.uz)) {
                            assign_level(&tolevel, &valley_level);
                        } else if (Is_botlevel(&u.uz)) {
                            if (canseemon(mtmp)) {
                                char name[BUFSZ];
                                Monnam(name, BUFSZ, mtmp);
                                pline("%s avoids the trap.", name);
                            }
                            return;
                        } else {
                            get_level(&tolevel, depth(&u.uz) + 1);
                        }
                        if (mtmp->isshk) make_angry_shk(mtmp, 0, 0);
                        migrate_to_level(mtmp, ledger_no(&tolevel),
                                         MIGR_RANDOM, (coord *)0);
                    }
                }
            }
        }
}

static void dig_up_grave (void) {
    struct obj *otmp;

    /* Grave-robbing is frowned upon... */
    exercise(A_WIS, false);
    if (Role_if(PM_ARCHEOLOGIST)) {
        adjalign(-sgn(u.ualign.type)*3);
        You_feel("like a despicable grave-robber!");
    } else if (Role_if(PM_SAMURAI)) {
        adjalign(-sgn(u.ualign.type));
        You("disturb the honorable dead!");
    } else if ((u.ualign.type == A_LAWFUL) && (u.ualign.record > -10)) {
        adjalign(-sgn(u.ualign.type));
        You("have violated the sanctity of this grave!");
    }

    switch (rn2(5)) {
        case 0:
        case 1:
            You("unearth a corpse.");
            if (!!(otmp = mk_tt_object(CORPSE, u.ux, u.uy)))
                otmp->age -= 100;               /* this is an *OLD* corpse */;
            break;
        case 2:
            if (!Blind()) pline(Hallucination() ? "Dude!  The living dead!" :
                    "The grave's owner is very upset!");
            (void) makemon(mkclass(S_ZOMBIE,0), u.ux, u.uy, NO_MM_FLAGS);
            break;
        case 3:
            if (!Blind()) pline(Hallucination() ? "I want my mummy!" :
                    "You've disturbed a tomb!");
            (void) makemon(mkclass(S_MUMMY,0), u.ux, u.uy, NO_MM_FLAGS);
            break;
        default:
            /* No corpse */
            pline_The("grave seems unused.  Strange....");
            break;
    }
    levl[u.ux][u.uy].typ = ROOM;
    del_engr_at(u.ux, u.uy);
    newsym(u.ux,u.uy);
    return;
}

/* return true if digging succeeded, false otherwise */
bool dighole(bool pit_only) {
        struct trap *ttmp = t_at(u.ux, u.uy);
        struct rm *lev = &levl[u.ux][u.uy];
        struct obj *boulder_here;
        signed char typ;
        bool nohole = !Can_dig_down(&u.uz);

        if ((ttmp && (ttmp->ttyp == MAGIC_PORTAL || nohole)) ||
           (IS_ROCK(lev->typ) && lev->typ != SDOOR &&
            (lev->flags & W_NONDIGGABLE) != 0)) {
                pline_The("%s here is too hard to dig in.", surface(u.ux,u.uy));

        } else if (is_pool(u.ux, u.uy) || is_lava(u.ux, u.uy)) {
                pline_The("%s sloshes furiously for a moment, then subsides.",
                        is_lava(u.ux, u.uy) ? "lava" : "water");
                wake_nearby();  /* splashing */

        } else if (lev->typ == DRAWBRIDGE_DOWN ||
                   (is_drawbridge_wall(u.ux, u.uy) >= 0)) {
                /* drawbridge_down is the platform crossing the moat when the
                   bridge is extended; drawbridge_wall is the open "doorway" or
                   closed "door" where the portcullis/mechanism is located */
                if (pit_only) {
                    pline_The("drawbridge seems too hard to dig through.");
                    return false;
                } else {
                    int x = u.ux, y = u.uy;
                    /* if under the portcullis, the bridge is adjacent */
                    (void) find_drawbridge(&x, &y);
                    destroy_drawbridge(x, y);
                    return true;
                }

        } else if ((boulder_here = sobj_at(BOULDER, u.ux, u.uy)) != 0) {
                if (ttmp && (ttmp->ttyp == PIT || ttmp->ttyp == SPIKED_PIT) &&
                    rn2(2)) {
                        pline_The("boulder settles into the pit.");
                        ttmp->ttyp = PIT;        /* crush spikes */
                } else {
                        /*
                         * digging makes a hole, but the boulder immediately
                         * fills it.  Final outcome:  no hole, no boulder.
                         */
                        pline("KADOOM! The boulder falls in!");
                        (void) delfloortrap(ttmp);
                }
                delobj(boulder_here);
                return true;

        } else if (IS_GRAVE(lev->typ)) {
            digactualhole(u.ux, u.uy, BY_YOU, PIT);
            dig_up_grave();
            return true;
        } else if (lev->typ == DRAWBRIDGE_UP) {
                /* must be floor or ice, other cases handled above */
                /* dig "pit" and let fluid flow in (if possible) */
                typ = fillholetyp(u.ux,u.uy);

                if (typ == ROOM) {
                        /*
                         * We can't dig a hole here since that will destroy
                         * the drawbridge.  The following is a cop-out. --dlc
                         */
                        pline_The("%s here is too hard to dig in.",
                              surface(u.ux, u.uy));
                        return false;
                }

                lev->flags &= ~DB_UNDER;
                lev->flags |= (typ == LAVAPOOL) ? DB_LAVA : DB_MOAT;

 liquid_flow:
                if (ttmp) (void) delfloortrap(ttmp);
                /* if any objects were frozen here, they're released now */
                unearth_objs(u.ux, u.uy);

                pline("As you dig, the hole fills with %s!",
                      typ == LAVAPOOL ? "lava" : "water");
                if (!Levitation && !Flying) {
                    if (typ == LAVAPOOL)
                        (void) lava_effects();
                    else if (!Wwalking)
                        (void) drown();
                }
                return true;

        /* the following two are here for the wand of digging */
        } else if (IS_THRONE(lev->typ)) {
                pline_The("throne is too hard to break apart.");

        } else if (IS_ALTAR(lev->typ)) {
                pline_The("altar is too hard to break apart.");

        } else {
                typ = fillholetyp(u.ux,u.uy);

                if (typ != ROOM) {
                        lev->typ = typ;
                        goto liquid_flow;
                }

                /* finally we get to make a hole */
                if (nohole || pit_only)
                        digactualhole(u.ux, u.uy, BY_YOU, PIT);
                else
                        digactualhole(u.ux, u.uy, BY_YOU, HOLE);

                return true;
        }

        return false;
}

int
use_pick_axe (struct obj *obj)
{
        bool ispick;
        char dirsyms[12];
        char qbuf[QBUFSZ];
        char *dsp = dirsyms;
        int rx, ry;
        int res = 0;
        const char *verb;

        /* Check tool */
        if (obj != uwep) {
            if (!wield_tool(obj, "swing")) return 0;
            else res = 1;
        }
        ispick = is_pick(obj);
        verb = ispick ? "dig" : "chop";

        if (u.utrap && u.utraptype == TT_WEB) {
            pline("%s you can't %s while entangled in a web.",
                  /* res==0 => no prior message;
                     res==1 => just got "You now wield a pick-axe." message */
                  !res ? "Unfortunately," : "But", verb);
            return res;
        }

        sprintf(qbuf, "In what direction do you want to %s? ", verb);
        if(!getdir(qbuf))
                return(res);

        return(use_pick_axe2(obj));
}

/* MRKR: use_pick_axe() is split in two to allow autodig to bypass */
/*       the "In what direction do you want to dig?" query.        */
/*       use_pick_axe2() uses the existing u.dx, u.dy and u.dz    */

int use_pick_axe2 (struct obj *obj) {
    int rx, ry;
    struct rm *lev;
    int dig_target;
    bool ispick = is_pick(obj);
    const char *verbing = ispick ? "digging" : "chopping";

    if (u.uswallow && attack(u.ustuck)) {
        ;  /* return(1) */
    } else if (Underwater) {
        pline("Turbulence torpedoes your %s attempts.", verbing);
    } else if(u.delta.z < 0) {
        if(Levitation)
            You("don't have enough leverage.");
        else
            You_cant("reach the %s.",ceiling(u.ux,u.uy));
    } else if(!u.delta.x && !u.delta.y && !u.delta.z) {
        char buf[BUFSZ];
        int dam;

        dam = rnd(2) + dbon() + obj->spe;
        if (dam <= 0) dam = 1;
        You("hit yourself with %s.", yname(uwep));
        losehp(dam, killed_by_object(KM_YOUR_OWN_O, obj));
        return(1);
    } else if(u.delta.z == 0) {
        if(Stunned() || (Confusion() && !rn2(5))) confdir();
        rx = u.ux + u.delta.x;
        ry = u.uy + u.delta.y;
        if(!isok(rx, ry)) {
            pline("Clash!");
            return(1);
        }
        lev = &levl[rx][ry];
        if(MON_AT(rx, ry) && attack(m_at(rx, ry)))
            return(1);
        dig_target = dig_typ(obj, rx, ry);
        if (dig_target == DIGTYP_UNDIGGABLE) {
            /* ACCESSIBLE or POOL */
            struct trap *trap = t_at(rx, ry);

            if (trap && trap->ttyp == WEB) {
                if (!trap->tseen) {
                    seetrap(trap);
                    There("is a spider web there!");
                }
                Your("%s entangled in the web.",
                        aobjnam(obj, "become"));
                /* you ought to be able to let go; tough luck */
                /* (maybe `move_into_trap()' would be better) */
                nomul(-d(2,2));
                nomovemsg = "You pull free.";
            } else if (lev->typ == IRONBARS) {
                pline("Clang!");
                wake_nearby();
            } else if (IS_TREE(lev->typ)) {
                You("need an axe to cut down a tree.");
            } else if (IS_ROCK(lev->typ)) {
                You("need a pick to dig rock.");
            } else if (!ispick && (sobj_at(STATUE, rx, ry) || sobj_at(BOULDER, rx, ry))) {
                bool vibrate = !rn2(3);
                pline("Sparks fly as you whack the %s.%s",
                        sobj_at(STATUE, rx, ry) ? "statue" : "boulder",
                        vibrate ? " The axe-handle vibrates violently!" : "");
                if (vibrate)
                    losehp(2, killed_by_const(KM_AXING_HARD_OBJECT));
            } else {
                You("swing your %s through thin air.", aobjnam(obj, NULL));
            }
        } else {
            static const char * const d_action[6] = {
                "swinging",
                "digging",
                "chipping the statue",
                "hitting the boulder",
                "chopping at the door",
                "cutting the tree"
            };
            did_dig_msg = false;
            digging.quiet = false;
            if (digging.pos.x != rx || digging.pos.y != ry ||
                    !on_level(&digging.level, &u.uz) || digging.down) {
                if (flags.autodig &&
                        dig_target == DIGTYP_ROCK && !digging.down &&
                        digging.pos.x == u.ux &&
                        digging.pos.y == u.uy &&
                        (moves <= digging.lastdigtime+2 &&
                         moves >= digging.lastdigtime)) {
                    /* avoid messages if repeated autodigging */
                    did_dig_msg = true;
                    digging.quiet = true;
                }
                digging.down = digging.chew = false;
                digging.warned = false;
                digging.pos.x = rx;
                digging.pos.y = ry;
                assign_level(&digging.level, &u.uz);
                digging.effort = 0;
                if (!digging.quiet)
                    You("start %s.", d_action[dig_target]);
            } else {
                You("%s %s.", digging.chew ? "begin" : "continue",
                        d_action[dig_target]);
                digging.chew = false;
            }
            set_occupation(dig, verbing);
        }
    } else if (Is_airlevel(&u.uz) || Is_waterlevel(&u.uz)) {
        /* it must be air -- water checked above */
        You("swing your %s through thin air.", aobjnam(obj, (char *)0));
    } else if (!can_reach_floor()) {
        You_cant("reach the %s.", surface(u.ux,u.uy));
    } else if (is_pool(u.ux, u.uy) || is_lava(u.ux, u.uy)) {
        /* Monsters which swim also happen not to be able to dig */
        You("cannot stay under%s long enough.",
                is_pool(u.ux, u.uy) ? "water" : " the lava");
    } else if (!ispick) {
        Your("%s merely scratches the %s.",
                aobjnam(obj, (char *)0), surface(u.ux,u.uy));
        u_wipe_engr(3);
    } else {
        if (digging.pos.x != u.ux || digging.pos.y != u.uy ||
                !on_level(&digging.level, &u.uz) || !digging.down) {
            digging.chew = false;
            digging.down = true;
            digging.warned = false;
            digging.pos.x = u.ux;
            digging.pos.y = u.uy;
            assign_level(&digging.level, &u.uz);
            digging.effort = 0;
            You("start %s downward.", verbing);
            if (*u.ushops) shopdig(0);
        } else
            You("continue %s downward.", verbing);
        did_dig_msg = false;
        set_occupation(dig, verbing);
    }
    return(1);
}

/*
 * Town Watchmen frown on damage to the town walls, trees or fountains.
 * It's OK to dig holes in the ground, however.
 * If mtmp is assumed to be a watchman, a watchman is found if mtmp == 0
 * zap == true if wand/spell of digging, false otherwise (chewing)
 */
void watch_dig(struct monst *mtmp, signed char x, signed char y, bool zap) {
        struct rm *lev = &levl[x][y];

        if (in_town(x, y) &&
            (closed_door(x, y) || lev->typ == SDOOR ||
             IS_WALL(lev->typ) || IS_FOUNTAIN(lev->typ) || IS_TREE(lev->typ))) {
            if (!mtmp) {
                for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
                    if (DEADMONSTER(mtmp)) continue;
                    if ((mtmp->data == &mons[PM_WATCHMAN] ||
                         mtmp->data == &mons[PM_WATCH_CAPTAIN]) &&
                        mtmp->mcansee && m_canseeu(mtmp) &&
                        couldsee(mtmp->mx, mtmp->my) && mtmp->mpeaceful)
                        break;
                }
            }

            if (mtmp) {
                if(zap || digging.warned) {
                    verbalize("Halt, vandal!  You're under arrest!");
                    (void) angry_guards(!(flags.soundok));
                } else {
                    const char *str;

                    if (IS_DOOR(lev->typ))
                        str = "door";
                    else if (IS_TREE(lev->typ))
                        str = "tree";
                    else if (IS_ROCK(lev->typ))
                        str = "wall";
                    else
                        str = "fountain";
                    verbalize("Hey, stop damaging that %s!", str);
                    digging.warned = true;
                }
                if (is_digging())
                    stop_occupation();
            }
        }
}


/* Return true if monster died, false otherwise.  Called from m_move(). */
bool 
mdig_tunnel (struct monst *mtmp)
{
        struct rm *here;
        int pile = rnd(12);

        here = &levl[mtmp->mx][mtmp->my];
        if (here->typ == SDOOR)
            cvt_sdoor_to_door(here);    /* ->typ = DOOR */

        /* Eats away door if present & closed or locked */
        if (closed_door(mtmp->mx, mtmp->my)) {
            if (*in_rooms(mtmp->mx, mtmp->my, SHOPBASE))
                add_damage(mtmp->mx, mtmp->my, 0L);
            unblock_point(mtmp->mx, mtmp->my);  /* vision */
            if (here->flags & D_TRAPPED) {
                here->flags = D_NODOOR;
                if (mb_trapped(mtmp)) { /* mtmp is killed */
                    newsym(mtmp->mx, mtmp->my);
                    return true;
                }
            } else {
                if (!rn2(3) && flags.verbose)   /* not too often.. */
                    You_feel("an unexpected draft.");
                here->flags = D_BROKEN;
            }
            newsym(mtmp->mx, mtmp->my);
            return false;
        } else if (!IS_ROCK(here->typ) && !IS_TREE(here->typ)) /* no dig */
            return false;

        /* Only rock, trees, and walls fall through to this point. */
        if ((here->flags & W_NONDIGGABLE) != 0) {
            impossible("mdig_tunnel:  %s at (%d,%d) is undiggable",
                       (IS_WALL(here->typ) ? "wall" : "stone"),
                       (int) mtmp->mx, (int) mtmp->my);
            return false;       /* still alive */
        }

        if (IS_WALL(here->typ)) {
            /* KMH -- Okay on arboreal levels (room walls are still stone) */
            if (flags.soundok && flags.verbose && !rn2(5))
                You_hear("crashing rock.");
            if (*in_rooms(mtmp->mx, mtmp->my, SHOPBASE))
                add_damage(mtmp->mx, mtmp->my, 0L);
            if (level.flags.is_maze_lev) {
                here->typ = ROOM;
            } else if (level.flags.is_cavernous_lev &&
                       !in_town(mtmp->mx, mtmp->my)) {
                here->typ = CORR;
            } else {
                here->typ = DOOR;
                here->flags = D_NODOOR;
            }
        } else if (IS_TREE(here->typ)) {
            here->typ = ROOM;
            if (pile && pile < 5)
                (void) rnd_treefruit_at(mtmp->mx, mtmp->my);
        } else {
            here->typ = CORR;
            if (pile && pile < 5)
                (void) mksobj_at((pile == 1) ? BOULDER : ROCK,
                             mtmp->mx, mtmp->my, true, false);
        }
        newsym(mtmp->mx, mtmp->my);
        if (!sobj_at(BOULDER, mtmp->mx, mtmp->my))
            unblock_point(mtmp->mx, mtmp->my);  /* vision */

        return false;
}


/* digging via wand zap or spell cast */
void zap_dig(void) {
        struct rm *room;
        struct monst *mtmp;
        struct obj *otmp;
        int zx, zy, digdepth;
        bool shopdoor, shopwall, maze_dig;
        /*
         * Original effect (approximately):
         * from CORR: dig until we pierce a wall
         * from ROOM: pierce wall and dig until we reach
         * an ACCESSIBLE place.
         * Currently: dig for digdepth positions;
         * also down on request of Lennart Augustsson.
         */

        if (u.uswallow) {
            mtmp = u.ustuck;

            if (!is_whirly(mtmp->data)) {
                if (is_animal(mtmp->data)) {
                    char name[BUFSZ];
                    mon_nam(name, BUFSZ, mtmp);
                    You("pierce %s%s %s wall!", name, possessive_suffix(name),
                            mbodypart(mtmp, STOMACH));
                }
                mtmp->mhp = 1;          /* almost dead */
                expels(mtmp, mtmp->data, !is_animal(mtmp->data));
            }
            return;
        } /* swallowed */

        if (u.delta.z) {
            if (!Is_airlevel(&u.uz) && !Is_waterlevel(&u.uz) && !Underwater) {
                if (u.delta.z < 0 || On_stairs(u.ux, u.uy)) {
                    if (On_stairs(u.ux, u.uy))
                        pline_The("beam bounces off the %s and hits the %s.",
                              (u.ux == xdnladder || u.ux == xupladder) ?
                              "ladder" : "stairs", ceiling(u.ux, u.uy));
                    You("loosen a rock from the %s.", ceiling(u.ux, u.uy));
                    pline("It falls on your %s!", body_part(HEAD));
                    int lost_hp = rnd((uarmh && is_metallic(uarmh)) ? 2 : 6);
                    losehp(lost_hp, killed_by_const(KM_FALLING_ROCK));
                    otmp = mksobj_at(ROCK, u.ux, u.uy, false, false);
                    if (otmp) {
                        (void)xname(otmp);      /* set dknown, maybe bknown */
                        stackobj(otmp);
                    }
                    newsym(u.ux, u.uy);
                } else {
                    watch_dig((struct monst *)0, u.ux, u.uy, true);
                    (void) dighole(false);
                }
            }
            return;
        } /* up or down */

        /* normal case: digging across the level */
        shopdoor = shopwall = false;
        maze_dig = level.flags.is_maze_lev && !Is_earthlevel(&u.uz);
        zx = u.ux + u.delta.x;
        zy = u.uy + u.delta.y;
        digdepth = rn1(18, 8);
        tmp_at(DISP_BEAM, cmap_to_glyph(S_digbeam));
        while (--digdepth >= 0) {
            if (!isok(zx,zy)) break;
            room = &levl[zx][zy];
            tmp_at(zx,zy);
            if (closed_door(zx, zy) || room->typ == SDOOR) {
                if (*in_rooms(zx,zy,SHOPBASE)) {
                    add_damage(zx, zy, 400L);
                    shopdoor = true;
                }
                if (room->typ == SDOOR)
                    room->typ = DOOR;
                else if (cansee(zx, zy))
                    pline_The("door is razed!");
                watch_dig((struct monst *)0, zx, zy, true);
                room->flags = D_NODOOR;
                unblock_point(zx,zy); /* vision */
                digdepth -= 2;
                if (maze_dig) break;
            } else if (maze_dig) {
                if (IS_WALL(room->typ)) {
                    if (!(room->flags & W_NONDIGGABLE)) {
                        if (*in_rooms(zx,zy,SHOPBASE)) {
                            add_damage(zx, zy, 200L);
                            shopwall = true;
                        }
                        room->typ = ROOM;
                        unblock_point(zx,zy); /* vision */
                    } else if (!Blind())
                        pline_The("wall glows then fades.");
                    break;
                } else if (IS_TREE(room->typ)) { /* check trees before stone */
                    if (!(room->flags & W_NONDIGGABLE)) {
                        room->typ = ROOM;
                        unblock_point(zx,zy); /* vision */
                    } else if (!Blind())
                        pline_The("tree shudders but is unharmed.");
                    break;
                } else if (room->typ == STONE || room->typ == SCORR) {
                    if (!(room->flags & W_NONDIGGABLE)) {
                        room->typ = CORR;
                        unblock_point(zx,zy); /* vision */
                    } else if (!Blind())
                        pline_The("rock glows then fades.");
                    break;
                }
            } else if (IS_ROCK(room->typ)) {
                if (!may_dig(zx,zy)) break;
                if (IS_WALL(room->typ) || room->typ == SDOOR) {
                    if (*in_rooms(zx,zy,SHOPBASE)) {
                        add_damage(zx, zy, 200L);
                        shopwall = true;
                    }
                    watch_dig((struct monst *)0, zx, zy, true);
                    if (level.flags.is_cavernous_lev && !in_town(zx, zy)) {
                        room->typ = CORR;
                    } else {
                        room->typ = DOOR;
                        room->flags = D_NODOOR;
                    }
                    digdepth -= 2;
                } else if (IS_TREE(room->typ)) {
                    room->typ = ROOM;
                    digdepth -= 2;
                } else {        /* IS_ROCK but not IS_WALL or SDOOR */
                    room->typ = CORR;
                    digdepth--;
                }
                unblock_point(zx,zy); /* vision */
            }
            zx += u.delta.x;
            zy += u.delta.y;
        } /* while */
        tmp_at(DISP_END,0);     /* closing call */
        if (shopdoor || shopwall)
            pay_for_damage(shopdoor ? "destroy" : "dig into", false);
        return;
}

/* move objects from fobj/nexthere lists to buriedobjlist, keeping position */
/* information */
struct obj *
bury_an_obj (struct obj *otmp)
{
        struct obj *otmp2;
        bool under_ice;

        if (otmp == uball)
                unpunish();
        /* after unpunish(), or might get deallocated chain */
        otmp2 = otmp->nexthere;
        /*
         * obj_resists(,0,0) prevents Rider corpses from being buried.
         * It also prevents The Amulet and invocation tools from being
         * buried.  Since they can't be confined to bags and statues,
         * it makes sense that they can't be buried either, even though
         * the real reason there (direct accessibility when carried) is
         * completely different.
         */
        if (otmp == uchain || obj_resists(otmp, 0, 0))
                return(otmp2);

        if (otmp->otyp == LEASH && otmp->leashmon != 0)
                o_unleash(otmp);

        if (otmp->lamplit && otmp->otyp != POT_OIL)
                end_burn(otmp, true);

        obj_extract_self(otmp);

        under_ice = is_ice(otmp->ox, otmp->oy);
        if (otmp->otyp == ROCK && !under_ice) {
                /* merges into burying material */
                obfree(otmp, (struct obj *)0);
                return(otmp2);
        }
        /*
         * Start a rot on organic material.  Not corpses -- they
         * are already handled.
         */
        if (otmp->otyp == CORPSE) {
            ;           /* should cancel timer if under_ice */
        } else if ((under_ice ? otmp->oclass == POTION_CLASS : is_organic(otmp))
                && !obj_resists(otmp, 5, 95)) {
            (void) start_timer((under_ice ? 0L : 250L) + (long)rnd(250),
                               TIMER_OBJECT, ROT_ORGANIC, (void *)otmp);
        }
        add_to_buried(otmp);
        return(otmp2);
}

void bury_objs (int x, int y) {
        struct obj *otmp, *otmp2;

        for (otmp = level.objects[x][y]; otmp; otmp = otmp2)
                otmp2 = bury_an_obj(otmp);

        /* don't expect any engravings here, but just in case */
        del_engr_at(x, y);
        newsym(x, y);
}

/* move objects from buriedobjlist to fobj/nexthere lists */
void
unearth_objs (int x, int y)
{
        struct obj *otmp, *otmp2;

        for (otmp = level.buriedobjlist; otmp; otmp = otmp2) {
                otmp2 = otmp->nobj;
                if (otmp->ox == x && otmp->oy == y) {
                    obj_extract_self(otmp);
                    if (otmp->timed)
                        (void) stop_timer(ROT_ORGANIC, (void *)otmp);
                    place_object(otmp, x, y);
                    stackobj(otmp);
                }
        }
        del_engr_at(x, y);
        newsym(x, y);
}

/*
 * The organic material has rotted away while buried.  As an expansion,
 * we could add add partial damage.  A damage count is kept in the object
 * and every time we are called we increment the count and reschedule another
 * timeout.  Eventually the object rots away.
 *
 * This is used by buried objects other than corpses.  When a container rots
 * away, any contents become newly buried objects.
 */
void rot_organic ( void *arg, long timeout) {
    struct obj *obj = (struct obj *) arg;

    while (Has_contents(obj)) {
        /* We don't need to place contained object on the floor
           first, but we do need to update its map coordinates. */
        obj->cobj->ox = obj->ox,  obj->cobj->oy = obj->oy;
        /* Everything which can be held in a container can also be
           buried, so bury_an_obj's use of obj_extract_self insures
           that Has_contents(obj) will eventually become false. */
        (void)bury_an_obj(obj->cobj);
    }
    obj_extract_self(obj);
    obfree(obj, (struct obj *) 0);
}

/*
 * Called when a corpse has rotted completely away.
 */
void rot_corpse ( void *arg, long timeout) {
    signed char x = 0, y = 0;
    struct obj *obj = (struct obj *) arg;
    bool on_floor = obj->where == OBJ_FLOOR,
         in_invent = obj->where == OBJ_INVENT;

    if (on_floor) {
        x = obj->ox;
        y = obj->oy;
    } else if (in_invent) {
        if (flags.verbose) {
            char *cname = corpse_xname(obj, false);
            char rot_tense[BUFSZ];
            otense(rot_tense, BUFSZ, obj, "rot");
            Your("%s%s %s away%c",
                    obj == uwep ? "wielded " : nul, cname,
                    rot_tense, obj == uwep ? '!' : '.');
        }
        if (obj == uwep) {
            uwepgone();     /* now bare handed */
            stop_occupation();
        } else if (obj == uswapwep) {
            uswapwepgone();
            stop_occupation();
        } else if (obj == uquiver) {
            uqwepgone();
            stop_occupation();
        }
    } else if (obj->where == OBJ_MINVENT && obj->owornmask) {
        if (obj == MON_WEP(obj->ocarry)) {
            setmnotwielded(obj->ocarry,obj);
            MON_NOWEP(obj->ocarry);
        }
    }
    rot_organic(arg, timeout);
    if (on_floor) newsym(x, y);
}
