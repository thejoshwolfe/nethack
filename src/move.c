#include "move.h"

#include "mon.h"
#include "sounds.h"
#include "steed.h"
#include "region.h"
#include "ball.h"
#include "timeout.h"
#include "eat.h"
#include "uhitm.h"
#include "polyself.h"
#include "objnam.h"
#include "apply.h"
#include "dog.h"
#include "display.h"
#include "do_name.h"
#include "mkmaze.h"
#include "dbridge.h"
#include "pline.h"
#include "dungeon_util.h"
#include "vision.h"
#include "monmove.h"
#include "invent.h"
#include "engrave.h"
#include "hack.h"
#include "dungeon.h"
#include "youprop.h"
#include "rm.h"
#include "trap.h"
#include "flag.h"

/*
 * Find a path from the destination (u.tx,u.ty) back to (u.ux,u.uy).
 * A shortest path is returned.  If guess is true, consider various
 * inaccessible locations as valid intermediate path points.
 * Returns true if a path was found.
 */
static bool findtravelpath(bool guess) {
    /* if travel to adjacent, reachable location, use normal movement rules */
    if (!guess && iflags.travel1 && distmin(u.ux, u.uy, u.tx, u.ty) == 1) {
        flags.run = 0;
        if (test_move(u.ux, u.uy, u.tx - u.ux, u.ty - u.uy, TEST_MOVE)) {
            u.dx = u.tx - u.ux;
            u.dy = u.ty - u.uy;
            nomul(0);
            iflags.travelcc.x = iflags.travelcc.y = -1;
            return true;
        }
        flags.run = 8;
    }
    if (u.tx != u.ux || u.ty != u.uy) {
        signed char travel[COLNO][ROWNO];
        signed char travelstepx[2][COLNO * ROWNO];
        signed char travelstepy[2][COLNO * ROWNO];
        signed char tx, ty, ux, uy;
        int n = 1; /* max offset in travelsteps */
        int set = 0; /* two sets current and previous */
        int radius = 1; /* search radius */
        int i;

        /* If guessing, first find an "obvious" goal location.  The obvious
         * goal is the position the player knows of, or might figure out
         * (couldsee) that is closest to the target on a straight path.
         */
        if (guess) {
            tx = u.ux;
            ty = u.uy;
            ux = u.tx;
            uy = u.ty;
        } else {
            tx = u.tx;
            ty = u.ty;
            ux = u.ux;
            uy = u.uy;
        }

        noguess: (void)memset((void *)travel, 0, sizeof(travel));
        travelstepx[0][0] = tx;
        travelstepy[0][0] = ty;

        while (n != 0) {
            int nn = 0;

            for (i = 0; i < n; i++) {
                int dir;
                int x = travelstepx[set][i];
                int y = travelstepy[set][i];
                static int ordered[] = { 0, 2, 4, 6, 1, 3, 5, 7 };
                /* no diagonal movement for grid bugs */
                int dirmax = u.umonnum == PM_GRID_BUG ? 4 : 8;

                for (dir = 0; dir < dirmax; ++dir) {
                    int nx = x + xdir[ordered[dir]];
                    int ny = y + ydir[ordered[dir]];

                    if (!isok(nx, ny))
                        continue;
                    if ((!Passes_walls && !can_ooze(&youmonst) && closed_door(x, y)) || sobj_at(BOULDER, x, y)) {
                        /* closed doors and boulders usually
                         * cause a delay, so prefer another path */
                        if (travel[x][y] > radius - 3) {
                            travelstepx[1 - set][nn] = x;
                            travelstepy[1 - set][nn] = y;
                            /* don't change travel matrix! */
                            nn++;
                            continue;
                        }
                    }
                    if (test_move(x, y, nx - x, ny - y, TEST_TRAV) && (levl[nx][ny].seenv || (!Blind && couldsee(nx, ny)))) {
                        if (nx == ux && ny == uy) {
                            if (!guess) {
                                u.dx = x - ux;
                                u.dy = y - uy;
                                if (x == u.tx && y == u.ty) {
                                    nomul(0);
                                    /* reset run so domove run checks work */
                                    flags.run = 8;
                                    iflags.travelcc.x = iflags.travelcc.y = -1;
                                }
                                return true;
                            }
                        } else if (!travel[nx][ny]) {
                            travelstepx[1 - set][nn] = nx;
                            travelstepy[1 - set][nn] = ny;
                            travel[nx][ny] = radius;
                            nn++;
                        }
                    }
                }
            }

            n = nn;
            set = 1 - set;
            radius++;
        }

        /* if guessing, find best location in travel matrix and go there */
        if (guess) {
            int px = tx, py = ty; /* pick location */
            int dist, nxtdist, d2, nd2;

            dist = distmin(ux, uy, tx, ty);
            d2 = dist2(ux, uy, tx, ty);
            for (tx = 1; tx < COLNO; ++tx)
                for (ty = 0; ty < ROWNO; ++ty)
                    if (travel[tx][ty]) {
                        nxtdist = distmin(ux, uy, tx, ty);
                        if (nxtdist == dist && couldsee(tx, ty)) {
                            nd2 = dist2(ux, uy, tx, ty);
                            if (nd2 < d2) {
                                /* prefer non-zigzag path */
                                px = tx;
                                py = ty;
                                d2 = nd2;
                            }
                        } else if (nxtdist < dist && couldsee(tx, ty)) {
                            px = tx;
                            py = ty;
                            dist = nxtdist;
                            d2 = dist2(ux, uy, tx, ty);
                        }
                    }

            if (px == u.ux && py == u.uy) {
                /* no guesses, just go in the general direction */
                u.dx = sgn(u.tx - u.ux);
                u.dy = sgn(u.ty - u.uy);
                if (test_move(u.ux, u.uy, u.dx, u.dy, TEST_MOVE))
                    return true;
                goto found;
            }
            tx = px;
            ty = py;
            ux = u.ux;
            uy = u.uy;
            set = 0;
            n = radius = 1;
            guess = false;
            goto noguess;
        }
        return false;
    }

    found: u.dx = 0;
    u.dy = 0;
    nomul(0);
    return false;
}

void domove(void) {
    struct monst *mtmp;
    struct rm *tmpr;
    signed char x, y;
    struct trap *trap;
    int wtcap;
    bool on_ice;
    signed char chainx, chainy, ballx, bally; /* ball&chain new positions */
    int bc_control; /* control for ball&chain */
    bool cause_delay = false; /* dragging ball will skip a move */
    const char *predicament;

    u_wipe_engr(rnd(5));

    if (flags.travel) {
        if (!findtravelpath(false))
            (void)findtravelpath(true);
        iflags.travel1 = 0;
    }

    if (((wtcap = near_capacity()) >= OVERLOADED || (wtcap > SLT_ENCUMBER && (Upolyd ? (u.mh < 5 && u.mh != u.mhmax) : (u.uhp < 10 && u.uhp != u.uhpmax)))) && !Is_airlevel(&u.uz)) {
        if (wtcap < OVERLOADED) {
            You("don't have enough stamina to move.");
            exercise(A_CON, false);
        } else
            You("collapse under your load.");
        nomul(0);
        return;
    }
    if (u.uswallow) {
        u.dx = u.dy = 0;
        u.ux = x = u.ustuck->mx;
        u.uy = y = u.ustuck->my;
        mtmp = u.ustuck;
    } else {
        if (Is_airlevel(&u.uz) && rn2(4) && !Levitation && !Flying) {
            switch (rn2(3)) {
                case 0:
                    You("tumble in place.");
                    exercise(A_DEX, false);
                    break;
                case 1:
                    You_cant("control your movements very well.");
                    break;
                case 2:
                    pline("It's hard to walk in thin air.");
                    exercise(A_DEX, true);
                    break;
            }
            return;
        }

        /* check slippery ice */
        on_ice = !Levitation && is_ice(u.ux, u.uy);
        if (on_ice) {
            static int skates = 0;
            if (!skates)
                skates = find_skates();
            if ((uarmf && uarmf->otyp == skates) || resists_cold(&youmonst) || Flying || is_floater(youmonst.data) || is_clinger(youmonst.data) || is_whirly(youmonst.data))
                on_ice = false;
            else if (!rn2(Cold_resistance() ? 3 : 2)) {
                HFumbling|= FROMOUTSIDE;
                HFumbling &= ~TIMEOUT;
                HFumbling += 1; /* slip on next move */
            }
        }
        if (!on_ice && (HFumbling& FROMOUTSIDE))
        HFumbling &= ~FROMOUTSIDE;

        x = u.ux + u.dx;
        y = u.uy + u.dy;
        if (Stunned() || (Confusion() && !rn2(5))) {
            int tries = 0;

            do {
                if(tries++ > 50) {
                    nomul(0);
                    return;
                }
                confdir();
                x = u.ux + u.dx;
                y = u.uy + u.dy;
            }while(!isok(x, y) || bad_rock(youmonst.data, x, y));
        }
        /* turbulence might alter your actual destination */
        if (u.uinwater) {
            water_friction();
            if (!u.dx && !u.dy) {
                nomul(0);
                return;
            }
            x = u.ux + u.dx;
            y = u.uy + u.dy;
        }
        if (!isok(x, y)) {
            nomul(0);
            return;
        }
        if (((trap = t_at(x, y)) && trap->tseen) || (Blind && !Levitation && !Flying && !is_clinger(youmonst.data) && (is_pool(x, y) || is_lava(x, y)) && levl[x][y].seenv)) {
            if (flags.run >= 2) {
                nomul(0);
                flags.move = 0;
                return;
            } else
                nomul(0);
        }

        if (u.ustuck && (x != u.ustuck->mx || y != u.ustuck->my)) {
            if (distu(u.ustuck->mx, u.ustuck->my) > 2) {
                /* perhaps it fled (or was teleported or ... ) */
                u.ustuck = 0;
            } else if (sticks(youmonst.data)) {
                /* When polymorphed into a sticking monster,
                 * u.ustuck means it's stuck to you, not you to it.
                 */
                char name[BUFSZ];
                mon_nam(name, BUFSZ, u.ustuck);
                You("release %s.", name);
                u.ustuck = 0;
            } else {
                /* If holder is asleep or paralyzed:
                 *      37.5% chance of getting away,
                 *      12.5% chance of waking/releasing it;
                 * otherwise:
                 *       7.5% chance of getting away.
                 * [strength ought to be a factor]
                 * If holder is tame and there is no conflict,
                 * guaranteed escape.
                 */
                switch (rn2(!u.ustuck->mcanmove ? 8 : 40)) {
                    case 0:
                    case 1:
                    case 2:
                        pull_free: {
                            char name[BUFSZ];
                            mon_nam(name, BUFSZ, u.ustuck);
                            You("pull free from %s.", name);
                            u.ustuck = 0;
                        }
                        break;
                    case 3:
                        if (!u.ustuck->mcanmove) {
                            /* it's free to move on next turn */
                            u.ustuck->mfrozen = 1;
                            u.ustuck->msleeping = 0;
                        }
                        /*FALLTHRU*/
                    default: {
                        if (u.ustuck->mtame && !Conflict && !u.ustuck->mconf)
                            goto pull_free;
                        char name[BUFSZ];
                        mon_nam(name, BUFSZ, u.ustuck);
                        You("cannot escape from %s!", name);
                        nomul(0);
                        return;
                    }
                }
            }
        }

        mtmp = m_at(x, y);
        if (mtmp) {
            /* Don't attack if you're running, and can see it */
            /* We should never get here if forcefight */
            if (flags.run && ((!Blind && mon_visible(mtmp) && ((mtmp->m_ap_type != M_AP_FURNITURE && mtmp->m_ap_type != M_AP_OBJECT) ||
            Protection_from_shape_changers)) || sensemon(mtmp))) {
                nomul(0);
                flags.move = 0;
                return;
            }
        }
    }

    u.ux0 = u.ux;
    u.uy0 = u.uy;
    bhitpos.x = x;
    bhitpos.y = y;
    tmpr = &levl[x][y];

    /* attack monster */
    if (mtmp) {
        nomul(0);
        /* only attack if we know it's there */
        /* or if we used the 'F' command to fight blindly */
        /* or if it hides_under, in which case we call attack() to print
         * the Wait! message.
         * This is different from ceiling hiders, who aren't handled in
         * attack().
         */

        /* If they used a 'm' command, trying to move onto a monster
         * prints the below message and wastes a turn.  The exception is
         * if the monster is unseen and the player doesn't remember an
         * invisible monster--then, we fall through to attack() and
         * attack_check(), which still wastes a turn, but prints a
         * different message and makes the player remember the monster.                  */
        if (flags.nopick && (canspotmon(mtmp) || glyph_is_invisible(levl[x][y].glyph))) {
            if (mtmp->m_ap_type && !Protection_from_shape_changers && !sensemon(mtmp)) {
                stumble_onto_mimic(mtmp);
            } else if (mtmp->mpeaceful && !Hallucination()) {
                char name[BUFSZ];
                m_monnam(name, BUFSZ, mtmp);
                pline("Pardon me, %s.", name);
            } else {
                char name[BUFSZ];
                m_monnam(name, BUFSZ, mtmp);
                You("move right into %s.", name);
            }
            return;
        }
        if (flags.forcefight || !mtmp->mundetected || sensemon(mtmp) || ((hides_under(mtmp->data) || mtmp->data->mlet == S_EEL) && !is_safepet(mtmp))) {
            gethungry();
            if (wtcap >= HVY_ENCUMBER && moves % 3) {
                if (Upolyd && u.mh > 1) {
                    u.mh--;
                } else if (!Upolyd && u.uhp > 1) {
                    u.uhp--;
                } else {
                    You("pass out from exertion!");
                    exercise(A_CON, false);
                    fall_asleep(-10, false);
                }
            }
            if (multi < 0)
                return; /* we just fainted */

            /* try to attack; note that it might evade */
            /* also, we don't attack tame when _safepet_ */
            if (attack(mtmp))
                return;
        }
    }

    /* specifying 'F' with no monster wastes a turn */
    if (flags.forcefight ||
    /* remembered an 'I' && didn't use a move command */
    (glyph_is_invisible(levl[x][y].glyph) && !flags.nopick)) {
        bool expl = (Upolyd && attacktype(youmonst.data, AT_EXPL));
        char buf[BUFSZ];
        sprintf(buf, "a vacant spot on the %s", surface(x, y));
        You("%s %s.", expl ? "explode at" : "attack", !Underwater ? "thin air" : is_pool(x, y) ? "empty water" : buf);
        unmap_object(x, y); /* known empty -- remove 'I' if present */
        newsym(x, y);
        nomul(0);
        if (expl) {
            u.mh = -1; /* dead in the current form */
            rehumanize();
        }
        return;
    }
    if (glyph_is_invisible(levl[x][y].glyph)) {
        unmap_object(x, y);
        newsym(x, y);
    }
    /* not attacking an animal, so we try to move */
    if (u.usteed && !u.usteed->mcanmove && (u.dx || u.dy)) {
        char name[BUFSZ];
        y_monnam(name, BUFSZ, u.usteed);
        pline("%s won't move!", name);
        nomul(0);
        return;
    } else if (!youmonst.data->mmove) {
        You("are rooted %s.",
        Levitation || Is_airlevel(&u.uz) || Is_waterlevel(&u.uz) ? "in place" : "to the ground");
        nomul(0);
        return;
    }
    if (u.utrap) {
        if (u.utraptype == TT_PIT) {
            if (!rn2(2) && sobj_at(BOULDER, u.ux, u.uy)) {
                Your("%s gets stuck in a crevice.", body_part(LEG));
                display_nhwindow(WIN_MESSAGE, false);
                clear_nhwindow(WIN_MESSAGE);
                You("free your %s.", body_part(LEG));
            } else if (!(--u.utrap)) {
                You("%s to the edge of the pit.", (In_sokoban(&u.uz) && Levitation) ? "struggle against the air currents and float" : u.usteed ? "ride" : "crawl");
                fill_pit(u.ux, u.uy);
                vision_full_recalc = 1; /* vision limits change */
            } else if (flags.verbose) {
                if (u.usteed) {
                    char name[BUFSZ];
                    y_monnam(name, BUFSZ, u.usteed);
                    Norep("%s is still in a pit.", upstart(name));
                } else {
                    Norep((Hallucination() && !rn2(5)) ? "You've fallen, and you can't get up." : "You are still in a pit.");
                }
            }
        } else if (u.utraptype == TT_LAVA) {
            if (flags.verbose) {
                predicament = "stuck in the lava";
                if (u.usteed) {
                    char name[BUFSZ];
                    y_monnam(name, BUFSZ, u.usteed);
                    Norep("%s is %s.", upstart(name), predicament);
                } else {
                    Norep("You are %s.", predicament);
                }
            }
            if (!is_lava(x, y)) {
                u.utrap--;
                if ((u.utrap & 0xff) == 0) {
                    if (u.usteed) {
                        char name[BUFSZ];
                        y_monnam(name, BUFSZ, u.usteed);
                        You("lead %s to the edge of the lava.", name);
                    } else {
                        You("pull yourself to the edge of the lava.");
                    }
                    u.utrap = 0;
                }
            }
            u.umoved = true;
        } else if (u.utraptype == TT_WEB) {
            if (uwep && uwep->oartifact == ART_STING) {
                u.utrap = 0;
                pline("Sting cuts through the web!");
                return;
            }
            if (--u.utrap) {
                if (flags.verbose) {
                    predicament = "stuck to the web";
                    if (u.usteed) {
                        char name[BUFSZ];
                        y_monnam(name, BUFSZ, u.usteed);
                        Norep("%s is %s.", upstart(name), predicament);
                    } else {
                        Norep("You are %s.", predicament);
                    }
                }
            } else {
                if (u.usteed) {
                    char name[BUFSZ];
                    y_monnam(name, BUFSZ, u.usteed);
                    pline("%s breaks out of the web.", upstart(name));
                } else {
                    You("disentangle yourself.");
                }
            }
        } else if (u.utraptype == TT_INFLOOR) {
            if (--u.utrap) {
                if (flags.verbose) {
                    predicament = "stuck in the";
                    if (u.usteed) {
                        char name[BUFSZ];
                        y_monnam(name, BUFSZ, u.usteed);
                        Norep("%s is %s %s.", upstart(name), predicament, surface(u.ux, u.uy));
                    } else {
                        Norep("You are %s %s.", predicament, surface(u.ux, u.uy));
                    }
                }
            } else {
                if (u.usteed) {
                    char name[BUFSZ];
                    y_monnam(name, BUFSZ, u.usteed);
                    pline("%s finally wiggles free.", upstart(name));
                } else {
                    You("finally wiggle free.");
                }
            }
        } else {
            if (flags.verbose) {
                predicament = "caught in a bear trap";
                if (u.usteed) {
                    char name[BUFSZ];
                    y_monnam(name, BUFSZ, u.usteed);
                    Norep("%s is %s.", upstart(name), predicament);
                } else {
                    Norep("You are %s.", predicament);
                }
            }
            if ((u.dx && u.dy) || !rn2(5))
                u.utrap--;
        }
        return;
    }

    if (!test_move(u.ux, u.uy, x - u.ux, y - u.uy, DO_MOVE)) {
        flags.move = 0;
        nomul(0);
        return;
    }

    /* Move ball and chain.  */
    if (Punished)
        if (!drag_ball(x, y, &bc_control, &ballx, &bally, &chainx, &chainy, &cause_delay, true))
            return;

    /* Check regions entering/leaving */
    if (!in_out_region(x, y))
        return;

    /* now move the hero */
    mtmp = m_at(x, y);
    u.ux += u.dx;
    u.uy += u.dy;
    /* Move your steed, too */
    if (u.usteed) {
        u.usteed->mx = u.ux;
        u.usteed->my = u.uy;
        exercise_steed();
    }

    /*
     * If safepet at destination then move the pet to the hero's
     * previous location using the same conditions as in attack().
     * there are special extenuating circumstances:
     * (1) if the pet dies then your god angers,
     * (2) if the pet gets trapped then your god may disapprove,
     * (3) if the pet was already trapped and you attempt to free it
     * not only do you encounter the trap but you may frighten your
     * pet causing it to go wild!  moral: don't abuse this privilege.
     *
     * Ceiling-hiding pets are skipped by this section of code, to
     * be caught by the normal falling-monster code.
     */
    if (is_safepet(mtmp) && !(is_hider(mtmp->data) && mtmp->mundetected)) {
        /* if trapped, there's a chance the pet goes wild */
        if (mtmp->mtrapped) {
            if (!rn2(mtmp->mtame)) {
                mtmp->mtame = mtmp->mpeaceful = mtmp->msleeping = 0;
                if (mtmp->mleashed)
                    m_unleash(mtmp, true);
                growl(mtmp);
            } else {
                yelp(mtmp);
            }
        }
        mtmp->mundetected = 0;
        if (mtmp->m_ap_type)
            seemimic(mtmp);
        else if (!mtmp->mtame)
            newsym(mtmp->mx, mtmp->my);

        if (mtmp->mtrapped && (trap = t_at(mtmp->mx, mtmp->my)) != 0 && (trap->ttyp == PIT || trap->ttyp == SPIKED_PIT) && sobj_at(BOULDER, trap->tx, trap->ty)) {
            /* can't swap places with pet pinned in a pit by a boulder */
            u.ux = u.ux0, u.uy = u.uy0; /* didn't move after all */
        } else if (u.ux0 != x && u.uy0 != y && bad_rock(mtmp->data, x, u.uy0) && bad_rock(mtmp->data, u.ux0, y) && (bigmonst(mtmp->data) || (curr_mon_load(mtmp) > 600))) {
            /* can't swap places when pet won't fit thru the opening */
            u.ux = u.ux0, u.uy = u.uy0; /* didn't move after all */
            char name[BUFSZ];
            y_monnam(name, BUFSZ, mtmp);
            You("stop.  %s won't fit through.", upstart(name));
        } else {
            char pnambuf[BUFSZ];

            /* save its current description in case of polymorph */
            char name[BUFSZ];
            y_monnam(name, BUFSZ, mtmp);
            strcpy(pnambuf, name);
            mtmp->mtrapped = 0;
            remove_monster(x, y);
            place_monster(mtmp, u.ux0, u.uy0);

            /* check for displacing it into pools and traps */
            switch (minliquid(mtmp) ? 2 : mintrap(mtmp)) {
                case 0:
                    You("%s %s.", mtmp->mtame ? "displaced" : "frightened", pnambuf);
                    break;
                case 1: /* trapped */
                case 3: /* changed levels */
                    /* there's already been a trap message, reinforce it */
                    abuse_dog(mtmp);
                    adjalign(-3);
                    break;
                case 2:
                    /* it may have drowned or died.  that's no way to
                     * treat a pet!  your god gets angry.
                     */
                    if (rn2(4)) {
                        You_feel("guilty about losing your pet like this.");
                        u.ugangr++;
                        adjalign(-15);
                    }

                    /* you killed your pet by direct action.
                     * minliquid and mintrap don't know to do this
                     */
                    u.uconduct.killer++;
                    break;
                default:
                    pline("that's strange, unknown mintrap result!");
                    break;
            }
        }
    }

    reset_occupations();
    if (flags.run) {
        if (flags.run < 8)
            if (IS_DOOR(tmpr->typ) || IS_ROCK(tmpr->typ) || IS_FURNITURE(tmpr->typ))
                nomul(0);
    }

    if (hides_under(youmonst.data))
        u.uundetected = OBJ_AT(u.ux, u.uy);
    else if (youmonst.data->mlet == S_EEL)
        u.uundetected = is_pool(u.ux, u.uy) && !Is_waterlevel(&u.uz);
    else if (u.dx || u.dy)
        u.uundetected = 0;

    /*
     * Mimics (or whatever) become noticeable if they move and are
     * imitating something that doesn't move.  We could extend this
     * to non-moving monsters...
     */
    if ((u.dx || u.dy) && (youmonst.m_ap_type == M_AP_OBJECT || youmonst.m_ap_type == M_AP_FURNITURE))
        youmonst.m_ap_type = M_AP_NOTHING;

    check_leash(u.ux0, u.uy0);

    if (u.ux0 != u.ux || u.uy0 != u.uy) {
        u.umoved = true;
        /* Clean old position -- vision_recalc() will print our new one. */
        newsym(u.ux0, u.uy0);
        /* Since the hero has moved, adjust what can be seen/unseen. */
        vision_recalc(1); /* Do the work now in the recover time. */
        invocation_message();
    }

    if (Punished) /* put back ball and chain */
        move_bc(0, bc_control, ballx, bally, chainx, chainy);

    spoteffects(true);

    /* delay next move because of ball dragging */
    /* must come after we finished picking up, in spoteffects() */
    if (cause_delay) {
        nomul(-2);
        nomovemsg = "";
    }

    if (flags.run && iflags.runmode != RUN_TPORT) {
        /* display every step or every 7th step depending upon mode */
        if (iflags.runmode != RUN_LEAP || !(moves % 7L)) {
            curs_on_u();
        }
    }
}

void invocation_message(void) {
    /* a special clue-msg when on the Invocation position */
    if (invocation_pos(u.ux, u.uy) && !On_stairs(u.ux, u.uy)) {
        char buf[BUFSZ];
        struct obj *otmp = carrying(CANDELABRUM_OF_INVOCATION);

        nomul(0); /* stop running or travelling */
        if (u.usteed) {
            char name[BUFSZ];
            y_monnam(name, BUFSZ, u.usteed);
            sprintf(buf, "beneath %s", name);
        } else {
            if (Levitation || Flying)
                strcpy(buf, "beneath you");
            else
                sprintf(buf, "under your %s", makeplural(body_part(FOOT)));
        }

        You_feel("a strange vibration %s.", buf);
        if (otmp && otmp->spe == 7 && otmp->lamplit)
            pline("%s %s!", The(xname(otmp)), Blind ? "throbs palpably" : "glows with a strange light");
    }
}
