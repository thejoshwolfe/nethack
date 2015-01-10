/* See LICENSE in the root of this project for change info */

#include "hack.h"

#include <stdio.h>
#include <string.h>

#include "dungeon_util.h"
#include "apply.h"
#include "artifact_names.h"
#include "attrib.h"
#include "ball.h"
#include "cmd.h"
#include "dbridge.h"
#include "decl.h"
#include "dig.h"
#include "display.h"
#include "do.h"
#include "do_name.h"
#include "do_wear.h"
#include "dog.h"
#include "dungeon.h"
#include "eat.h"
#include "end.h"
#include "engrave.h"
#include "flag.h"
#include "global.h"
#include "invent.h"
#include "mhitm.h"
#include "mhitu.h"
#include "mkmaze.h"
#include "mkobj.h"
#include "mkroom.h"
#include "mon.h"
#include "monattk.h"
#include "mondata.h"
#include "monflag.h"
#include "monmove.h"
#include "monst.h"
#include "monsym.h"
#include "o_init.h"
#include "obj.h"
#include "objclass.h"
#include "objnam.h"
#include "onames.h"
#include "permonst.h"
#include "pickup.h"
#include "pline.h"
#include "pm.h"
#include "polyself.h"
#include "priest.h"
#include "prop.h"
#include "region.h"
#include "rm.h"
#include "role.h"
#include "shk.h"
#include "skills.h"
#include "sounds.h"
#include "steed.h"
#include "teleport.h"
#include "timeout.h"
#include "trap.h"
#include "uhitm.h"
#include "util.h"
#include "vision.h"
#include "youprop.h"

static const char *killer_method_enum_str[] = {
    "DIED",
    "CHOKING",
    "POISONING",
    "STARVING",
    "DROWNING",
    "BURNING",
    "DISSOLVED",
    "CRUSHING",
    "STONING",
    "TURNED_SLIME",
    "GENOCIDED",
    "PANICKED",
    "TRICKED",
    "QUIT",
    "ESCAPED",
    "ASCENDED",
    "ELECTRIC_SHOCK",
    "TOWER_OF_FLAME",
    "BOILING_WATER",
    "DANGEROUS_WINDS",
    "LAND_MINE",
    "FELL_INTO_PIT",
    "FELL_INTO_PIT_OF_IRON_SPIKES",
    "RUSTING_AWAY",
    "SUFFOCATION",
    "STRANGULATION",
    "TURNED_INTO_GREEN_SLIME",
    "PETRIFICATION",
    "TELEPORTED_OUT_OF_THE_DUNGEON_AND_FELL_TO_DEATH",
    "WENT_TO_HEAVEN_PREMATURELY",
    "COMMITTED_SUICIDE",
    "SLIPPED_WHILE_MOUNTING_M",
    "ZAPPED_SELF_WITH_SPELL",
    "EXPLODING_RUNE",
    "CONTACT_POISONED_SPELLBOOK",
    "ELECTRIC_CHAIR",
    "CURSED_THRONE",
    "SITTING_ON_LAVA",
    "SITTING_IN_LAVA",
    "SITTING_ON_IRON_SPIKE",
    "GAS_CLOUD",
    "IMPERIOUS_ORDER",
    "GENOCIDAL_CONFUSION",
    "SCROLL_OF_GENOCIDE",
    "SCROLL_OF_FIRE",
    "EXPLODING_RING",
    "RESIDUAL_UNDEAD_TURNING",
    "ALCHEMIC_BLAST",
    "ELEMENTARY_CHEMISTRY",
    "THROWN_POTION",
    "POTION_OF_ACID",
    "BURNING_POTION_OF_OIL",
    "COLLIDING_WITH_CEILING",
    "CONTAMINATED_POTION",
    "CONTAMINATED_TAP_WATER",
    "MILDLY_CONTAMINATED_POTION",
    "POTION_UNHOLY_WATER",
    "POTION_HOLY_WATER",
    "DELIBERATELY_MEETING_MEDUSA_GAZE",
    "WHILE_STUCK_IN_CREATURE_FORM",
    "SYSTEM_SHOCK",
    "UNSUCCESSFUL_POLYMORPH",
    "SELF_GENOCIDE",
    "MAGICAL_EXPLOSION",
    "CARNIVOROUS_BAG",
    "USING_MAGIC_HORN_ON_SELF",
    "FELL_INTO_CHASM",
    "SCROLL_OF_EARTH",
    "WAND",
    "PSYCHIC_BLAST",
    "TODO",
    "BRAINLESSNESS",
    "TOUCH_OF_DEATH",
    "FELL_ON_SINK",
    "SIP_BOILING_WATER",
    "CONTAMINATED_WATER",
    "UNREFRIGERATED_JUICE",
    "MONSTER",
    "EXHAUSTION",
    "CADAVER",
    "POISONOUS_CORPSE",
    "ACIDIC_CORPSE",
    "ATE_HORSEMAN",
    "TASTING_O_MEAT",
    "O_EGG",
    "ROTTEN_ROYAL_JELLY",
    "CHOKE_QUICK_SNACK",
    "CHOKE_ON_FOOD",
    "O",
    "FALLING_OBJECT",
    "ELEMENTARY_PHYSICS",
    "WEDGING_INTO_NARROW_CREVICE",
    "TOUCH_EDGE_UNIVERSE",
    "BUMP_INTO_BOULDER",
    "CRASH_INTO_IRON_BARS",
    "BUMP_INTO_TREE",
    "BUMP_INTO_WALL",
    "BUMP_INTO_DOOR",
    "O_CORPSE",
    "KICKING_O",
    "KICKING_DOOR",
    "KICKING_TREE",
    "KICKING_WALL",
    "KICKING_ROCK",
    "KICKING_THRONE",
    "KICKING_FOUNTAIN",
    "KICKING_HEADSTONE",
    "KICKING_SINK",
    "KICKING_ALTAR",
    "KICKING_DRAWBRIDGE",
    "KICKING_STAIRS",
    "KICKING_LADDER",
    "KICKING_IRON_BAR",
    "KICKING_SOMETHING_WEIRD",
    "KICKING_O_WITHOUT_BOOTS",
    "FALLING_DOWNSTAIRS",
    "SQUISHED_UNDER_BOULDER",
    "AXING_HARD_OBJECT",
    "YOUR_OWN_O",
    "EXPLODING_CRYSTAL_BALL",
    "FALLING_DRAWBRIDGE",
    "CLOSING_DRAWBRIDGE",
    "FELL_FROM_DRAWBRIDGE",
    "EXPLODING_DRAWBRIDGE",
    "COLLAPSING_DRAWBRIDGE",
    "RIDING_ACCIDENT",
    "IRON_BALL_COLLISON",
    "DRAGGED_DOWNSTAIRS_IRON_BALL",
    "LEG_DAMAGE_PULLED_BEAR_TRAP",
    "CRUNCHED_IN_HEAD_BY_IRON_BALL",
    "TOUCH_ARTIFACT",
    "KILLED_SELF_BREAK_WAND",
    "GRAPPLING_HOOK_SELF",
    "HIT_SELF_BULLWHIP",
    "JUMP_BEAR_TRAP",
    "EXPLOSION",
    "MOLTEN_LAVA",
    "EXPLODING_WAND",
    "SELF_WITH_WAND",
    "SELF_WITH_DEATH_RAY",
    "FALLING_ROCK",
    "FLASH_TYPE",
};

#define IS_SHOP(x)      (rooms[x].rtype >= SHOPBASE)

static int wc;  /* current weight_cap(); valid after call to inv_weight() */

bool revive_nasty(int x, int y, const char *msg) {
    struct obj *otmp, *otmp2;
    struct monst *mtmp;
    coord cc;
    bool revived = false;

    for (otmp = level.objects[x][y]; otmp; otmp = otmp2) {
        otmp2 = otmp->nexthere;
        if (otmp->otyp == CORPSE && (is_rider(&mons[otmp->corpsenm]) || otmp->corpsenm == PM_WIZARD_OF_YENDOR)) {
            /* move any living monster already at that location */
            if ((mtmp = m_at(x, y)) && enexto(&cc, x, y, mtmp->data))
                rloc_to(mtmp, cc.x, cc.y);
            if (msg)
                Norep("%s", msg);
            revived = revive_corpse(otmp);
        }
    }

    /* this location might not be safe, if not, move revived monster */
    if (revived) {
        mtmp = m_at(x, y);
        if (mtmp && !goodpos(x, y, mtmp, 0) && enexto(&cc, x, y, mtmp->data)) {
            rloc_to(mtmp, cc.x, cc.y);
        }
        /* else impossible? */
    }

    return (revived);
}

static int moverock(void) {
    signed char rx, ry, sx, sy;
    struct obj *otmp;
    struct trap *ttmp;
    struct monst *mtmp;

    sx = u.ux + u.dx, sy = u.uy + u.dy; /* boulder starting position */
    while ((otmp = sobj_at(BOULDER, sx, sy)) != 0) {
        /* make sure that this boulder is visible as the top object */
        if (otmp != level.objects[sx][sy])
            movobj(otmp, sx, sy);

        rx = u.ux + 2 * u.dx; /* boulder destination position */
        ry = u.uy + 2 * u.dy;
        nomul(0);
        if (Levitation || Is_airlevel(&u.uz)) {
            if (Blind)
                feel_location(sx, sy);
            You("don't have enough leverage to push %s.", the(xname(otmp)));
            /* Give them a chance to climb over it? */
            return -1;
        }
        if (verysmall(youmonst.data) && !u.usteed) {
            if (Blind)
                feel_location(sx, sy);
            pline("You're too small to push that %s.", xname(otmp));
            goto cannot_push;
        }
        if (isok(rx, ry) && !IS_ROCK(levl[rx][ry].typ) && levl[rx][ry].typ != IRONBARS && (!IS_DOOR(levl[rx][ry].typ) || !(u.dx && u.dy) || ((levl[rx][ry].doormask & ~D_BROKEN) == D_NODOOR)) && !sobj_at(BOULDER, rx, ry)) {
            ttmp = t_at(rx, ry);
            mtmp = m_at(rx, ry);

            /* KMH -- Sokoban doesn't let you push boulders diagonally */
            if (In_sokoban(&u.uz) && u.dx && u.dy) {
                if (Blind)
                    feel_location(sx, sy);
                pline("%s won't roll diagonally on this %s.", The(xname(otmp)), surface(sx, sy));
                goto cannot_push;
            }

            if (revive_nasty(rx, ry, "You sense movement on the other side."))
                return (-1);

            if (mtmp && !noncorporeal(mtmp->data) && (!mtmp->mtrapped || !(ttmp && ((ttmp->ttyp == PIT) || (ttmp->ttyp == SPIKED_PIT))))) {
                if (Blind)
                    feel_location(sx, sy);
                if (canspotmon(mtmp)) {
                    char name[BUFSZ];
                    a_monnam(name, BUFSZ, mtmp);
                    pline("There's %s on the other side.", name);
                } else {
                    You_hear("a monster behind %s.", the(xname(otmp)));
                    map_invisible(rx, ry);
                }
                if (flags.verbose) {
                    const char *subject;
                    if (u.usteed) {
                        char name[BUFSZ];
                        y_monnam(name, BUFSZ, u.usteed);
                        subject = name;
                    } else {
                        subject = "you";
                    }
                    pline("Perhaps that's why %s cannot move it.", subject);
                }
                goto cannot_push;
            }

            if (ttmp)
                switch (ttmp->ttyp) {
                    case LANDMINE:
                        if (rn2(10)) {
                            obj_extract_self(otmp);
                            place_object(otmp, rx, ry);
                            unblock_point(sx, sy);
                            newsym(sx, sy);
                            char trigger_clause[BUFSZ];
                            Tobjnam(trigger_clause, BUFSZ, otmp, "trigger");
                            pline("KAABLAMM!!!  %s %s land mine.", trigger_clause, ttmp->madeby_u ? "your" : "a");
                            blow_up_landmine(ttmp);
                            /* if the boulder remains, it should fill the pit */
                            fill_pit(u.ux, u.uy);
                            if (cansee(rx, ry))
                                newsym(rx, ry);
                            continue;
                        }
                        break;
                    case SPIKED_PIT:
                    case PIT:
                        obj_extract_self(otmp);
                        /* vision kludge to get messages right;
                         the pit will temporarily be seen even
                         if this is one among multiple boulders */
                        if (!Blind)
                            viz_array[ry][rx] |= IN_SIGHT;
                        if (!flooreffects(otmp, rx, ry, "fall")) {
                            place_object(otmp, rx, ry);
                        }
                        if (mtmp && !Blind)
                            newsym(rx, ry);
                        continue;
                    case HOLE:
                    case TRAPDOOR:
                        if (Blind) {
                            pline("Kerplunk!  You no longer feel %s.", the(xname(otmp)));
                        } else {
                            char plug_tense[BUFSZ];
                            otense(plug_tense, BUFSZ, otmp, "plug");
                            char verb_clause[BUFSZ];
                            Tobjnam(verb_clause, BUFSZ, otmp, (ttmp->ttyp == TRAPDOOR) ? "trigger" : "fall");
                            pline("%s%s and %s a %s in the %s!", verb_clause, (ttmp->ttyp == TRAPDOOR) ? "" : " into", plug_tense, (ttmp->ttyp == TRAPDOOR) ? "trap door" : "hole", surface(rx, ry));
                        }
                        deltrap(ttmp);
                        delobj(otmp);
                        bury_objs(rx, ry);
                        if (cansee(rx, ry))
                            newsym(rx, ry);
                        continue;
                    case LEVEL_TELEP:
                    case TELEP_TRAP:
                        if (u.usteed) {
                            char name[BUFSZ];
                            y_monnam(name, BUFSZ, u.usteed);
                            pline("%s pushes %s and suddenly it disappears!", upstart(name), the(xname(otmp)));
                        } else {
                            You("push %s and suddenly it disappears!", the(xname(otmp)));
                        }
                        if (ttmp->ttyp == TELEP_TRAP) {
                            rloco(otmp);
                        } else {
                            int newlev = random_teleport_level();
                            d_level dest;

                            if (newlev == depth(&u.uz) || In_endgame(&u.uz))
                                continue;
                            obj_extract_self(otmp);
                            add_to_migration(otmp);
                            get_level(&dest, newlev);
                            otmp->ox = dest.dnum;
                            otmp->oy = dest.dlevel;
                            otmp->owornmask = (long)MIGR_RANDOM;
                        }
                        seetrap(ttmp);
                        continue;
                }
            if (closed_door(rx, ry))
                goto nopushmsg;
            if (boulder_hits_pool(otmp, rx, ry, true))
                continue;
            /*
             * Re-link at top of fobj chain so that pile order is preserved
             * when level is restored.
             */
            if (otmp != fobj) {
                remove_object(otmp);
                place_object(otmp, otmp->ox, otmp->oy);
            }

            {
                /* note: reset to zero after save/restore cycle */
                static long lastmovetime;
                if (!u.usteed) {
                    if (moves > lastmovetime + 2 || moves < lastmovetime)
                        pline("With %s effort you move %s.", throws_rocks(youmonst.data) ? "little" : "great", the(xname(otmp)));
                    exercise(A_STR, true);
                } else {
                    char name[BUFSZ];
                    y_monnam(name, BUFSZ, u.usteed);
                    pline("%s moves %s.", upstart(name), the(xname(otmp)));
                }
                lastmovetime = moves;
            }

            /* Move the boulder *after* the message. */
            if (glyph_is_invisible(levl[rx][ry].glyph))
                unmap_object(rx, ry);
            movobj(otmp, rx, ry); /* does newsym(rx,ry) */
            if (Blind) {
                feel_location(rx, ry);
                feel_location(sx, sy);
            } else {
                newsym(sx, sy);
            }
        } else {
            nopushmsg: if (u.usteed) {
                char name[BUFSZ];
                y_monnam(name, BUFSZ, u.usteed);
                pline("%s tries to move %s, but cannot.", upstart(name), the(xname(otmp)));
            } else {
                You("try to move %s, but in vain.", the(xname(otmp)));
            }
            if (Blind)
                feel_location(sx, sy);
            cannot_push: if (throws_rocks(youmonst.data)) {
                if (u.usteed && P_SKILL(P_RIDING) < P_BASIC) {
                    char name[BUFSZ];
                    y_monnam(name, BUFSZ, u.usteed);
                    You("aren't skilled enough to push aside %s from %s.", the(xname(otmp)), name);
                } else {
                    pline("However, you can easily push it aside.");
                    if (In_sokoban(&u.uz))
                        change_luck(-1); /* Sokoban guilt */
                    break;
                }
                break;
            }

            if (!u.usteed && (((!invent || inv_weight() <= -850) && (!u.dx || !u.dy || (IS_ROCK(levl[u.ux][sy].typ) && IS_ROCK(levl[sx][u.uy].typ)))) || verysmall(youmonst.data))) {
                pline("However, you can squeeze yourself into a small opening.");
                if (In_sokoban(&u.uz))
                    change_luck(-1); /* Sokoban guilt */
                break;
            } else {
                return (-1);
            }
        }
    }
    return (0);
}

/*
 *  still_chewing()
 *
 *  Chew on a wall, door, or boulder.  Returns true if still eating, false
 *  when done.
 */
static int still_chewing(signed char x, signed char y) {
    struct rm *lev = &levl[x][y];
    struct obj *boulder = sobj_at(BOULDER, x, y);
    const char *digtxt = (char *)0, *dmgtxt = (char *)0;

    if (digging.down) /* not continuing previous dig (w/ pick-axe) */
        (void)memset((void *)&digging, 0, sizeof digging);

    if (!boulder && IS_ROCK(lev->typ) && !may_dig(x, y)) {
        You("hurt your teeth on the %s.",
        IS_TREE(lev->typ) ? "tree" : "hard stone");
        nomul(0);
        return 1;
    } else if (digging.pos.x != x || digging.pos.y != y || !on_level(&digging.level, &u.uz)) {
        digging.down = false;
        digging.chew = true;
        digging.warned = false;
        digging.pos.x = x;
        digging.pos.y = y;
        assign_level(&digging.level, &u.uz);
        /* solid rock takes more work & time to dig through */
        digging.effort = (IS_ROCK(lev->typ) && !IS_TREE(lev->typ) ? 30 : 60) + u.udaminc;
        You("start chewing %s %s.", (boulder || IS_TREE(lev->typ)) ? "on a" : "a hole in the", boulder ? "boulder" : IS_TREE(lev->typ) ? "tree" : IS_ROCK(lev->typ) ? "rock" : "door");
        watch_dig((struct monst *)0, x, y, false);
        return 1;
    } else if ((digging.effort += (30 + u.udaminc)) <= 100) {
        if (flags.verbose)
            You("%s chewing on the %s.", digging.chew ? "continue" : "begin", boulder ? "boulder" : IS_TREE(lev->typ) ? "tree" : IS_ROCK(lev->typ) ? "rock" : "door");
        digging.chew = true;
        watch_dig((struct monst *)0, x, y, false);
        return 1;
    }

    /* Okay, you've chewed through something */
    u.uconduct.food++;
    u.uhunger += rnd(20);

    if (boulder) {
        delobj(boulder); /* boulder goes bye-bye */
        You("eat the boulder."); /* yum */

        /*
         *  The location could still block because of
         *      1. More than one boulder
         *      2. Boulder stuck in a wall/stone/door.
         *
         *  [perhaps use does_block() below (from vision.c)]
         */
        if (IS_ROCK(lev->typ) || closed_door(x, y) || sobj_at(BOULDER, x, y)) {
            block_point(x, y); /* delobj will unblock the point */
            /* reset dig state */
            (void)memset((void *)&digging, 0, sizeof digging);
            return 1;
        }

    } else if (IS_WALL(lev->typ)) {
        if (*in_rooms(x, y, SHOPBASE)) {
            add_damage(x, y, 10L * ACURRSTR);
            dmgtxt = "damage";
        }
        digtxt = "chew a hole in the wall.";
        if (level.flags.is_maze_lev) {
            lev->typ = ROOM;
        } else if (level.flags.is_cavernous_lev && !in_town(x, y)) {
            lev->typ = CORR;
        } else {
            lev->typ = DOOR;
            lev->doormask = D_NODOOR;
        }
    } else if (IS_TREE(lev->typ)) {
        digtxt = "chew through the tree.";
        lev->typ = ROOM;
    } else if (lev->typ == SDOOR) {
        if (lev->doormask & D_TRAPPED) {
            lev->doormask = D_NODOOR;
            b_trapped("secret door", 0);
        } else {
            digtxt = "chew through the secret door.";
            lev->doormask = D_BROKEN;
        }
        lev->typ = DOOR;

    } else if (IS_DOOR(lev->typ)) {
        if (*in_rooms(x, y, SHOPBASE)) {
            add_damage(x, y, 400L);
            dmgtxt = "break";
        }
        if (lev->doormask & D_TRAPPED) {
            lev->doormask = D_NODOOR;
            b_trapped("door", 0);
        } else {
            digtxt = "chew through the door.";
            lev->doormask = D_BROKEN;
        }

    } else { /* STONE or SCORR */
        digtxt = "chew a passage through the rock.";
        lev->typ = CORR;
    }

    unblock_point(x, y); /* vision */
    newsym(x, y);
    if (digtxt)
        You("%s", digtxt); /* after newsym */
    if (dmgtxt)
        pay_for_damage(dmgtxt, false);
    (void)memset((void *)&digging, 0, sizeof digging);
    return 0;
}

void movobj(struct obj *obj, signed char ox, signed char oy) {
    /* optimize by leaving on the fobj chain? */
    remove_object(obj);
    newsym(obj->ox, obj->oy);
    place_object(obj, ox, oy);
    newsym(ox, oy);
}

static void dosinkfall(void) {
    struct obj *obj;

    if (is_floater(youmonst.data) || (HLevitation& FROMOUTSIDE)) {
        You("wobble unsteadily for a moment.");
    } else {
        long save_ELev = ELevitation, save_HLev = HLevitation;

        /* fake removal of levitation in advance so that final
         disclosure will be right in case this turns out to
         be fatal; fortunately the fact that rings and boots
         are really still worn has no effect on bones data */
        ELevitation = HLevitation = 0L;
        You("crash to the floor!");
        losehp(rn1(8, 25 - (int)ACURR(A_CON)), killed_by_const(KM_FELL_ON_SINK));
        exercise(A_DEX, false);
        selftouch("Falling, you");
        for (obj = level.objects[u.ux][u.uy]; obj; obj = obj->nexthere)
        if (obj->oclass == WEAPON_CLASS || is_weptool(obj)) {
            You("fell on %s.", doname(obj));
            losehp(rnd(3), killed_by_const(KM_FELL_ON_SINK));
            exercise(A_CON, false);
        }
        ELevitation = save_ELev;
        HLevitation = save_HLev;
    }

    ELevitation&= ~W_ARTI;
    HLevitation&= ~(I_SPECIAL|TIMEOUT);
    HLevitation++;
    if (uleft && uleft->otyp == RIN_LEVITATION) {
        obj = uleft;
        Ring_off(obj);
        off_msg(obj);
    }
    if (uright && uright->otyp == RIN_LEVITATION) {
        obj = uright;
        Ring_off(obj);
        off_msg(obj);
    }
    if (uarmf && uarmf->otyp == LEVITATION_BOOTS) {
        obj = uarmf;
        (void)Boots_off();
        off_msg(obj);
    }
    HLevitation--;
}

/* intended to be called only on ROCKs */
bool may_dig(signed char x, signed char y) {
    return (bool)(!(IS_STWALL(levl[x][y].typ) && (levl[x][y].wall_info & W_NONDIGGABLE)));
}

bool may_passwall(signed char x, signed char y) {
    return (bool)(!(IS_STWALL(levl[x][y].typ) && (levl[x][y].wall_info & W_NONPASSWALL)));
}

bool bad_rock(struct permonst *mdat, signed char x, signed char y) {
    return ((bool)((In_sokoban(&u.uz) && sobj_at(BOULDER, x, y)) || (IS_ROCK(levl[x][y].typ) && (!tunnels(mdat) || needspick(mdat) || !may_dig(x, y)) && !(passes_walls(mdat) && may_passwall(x, y)))));
}

bool invocation_pos(signed char x, signed char y) {
    return ((bool)(Invocation_lev(&u.uz) && x == inv_pos.x && y == inv_pos.y));
}

/* return true if (dx,dy) is an OK place to move
 * mode is one of DO_MOVE, TEST_MOVE or TEST_TRAV
 */
bool test_move(int ux, int uy, int dx, int dy, int mode) {
    int x = ux + dx;
    int y = uy + dy;
    struct rm *tmpr = &levl[x][y];
    struct rm *ust;

    /*
     *  Check for physical obstacles.  First, the place we are going.
     */
    if (IS_ROCK(tmpr->typ) || tmpr->typ == IRONBARS) {
        if (Blind && mode == DO_MOVE)
            feel_location(x, y);
        if (Passes_walls && may_passwall(x, y)) {
            ; /* do nothing */
        } else if (tmpr->typ == IRONBARS) {
            if (!(Passes_walls || passes_bars(youmonst.data)))
                return false;
        } else if (tunnels(youmonst.data) && !needspick(youmonst.data)) {
            /* Eat the rock. */
            if (mode == DO_MOVE && still_chewing(x, y))
                return false;
        } else if (flags.autodig && !flags.run && !flags.nopick && uwep && is_pick(uwep)) {
            /* MRKR: Automatic digging when wielding the appropriate tool */
            if (mode == DO_MOVE)
                (void)use_pick_axe2(uwep);
            return false;
        } else {
            if (mode == DO_MOVE) {
                if (Is_stronghold(&u.uz) && is_db_wall(x, y))
                    pline_The("drawbridge is up!");
                if (Passes_walls && !may_passwall(x, y) && In_sokoban(&u.uz))
                    pline_The("Sokoban walls resist your ability.");
            }
            return false;
        }
    } else if (IS_DOOR(tmpr->typ)) {
        if (closed_door(x, y)) {
            if (Blind && mode == DO_MOVE)
                feel_location(x, y);
            if (Passes_walls) {
                /* do nothing */
            } else if (can_ooze(&youmonst)) {
                if (mode == DO_MOVE)
                    You("ooze under the door.");
            } else if (tunnels(youmonst.data) && !needspick(youmonst.data)) {
                /* Eat the door. */
                if (mode == DO_MOVE && still_chewing(x, y))
                    return false;
            } else {
                if (mode == DO_MOVE) {
                    if (amorphous(youmonst.data))
                        You("try to ooze under the door, but can't squeeze your possessions through.");
                    else if (x == ux || y == uy) {
                        if (Blind || Stunned() || ACURR(A_DEX) < 10 || Fumbling) {
                            if (u.usteed) {
                                char name[BUFSZ];
                                y_monnam(name, BUFSZ, u.usteed);
                                You_cant("lead %s through that closed door.", name);
                            } else {
                                pline("Ouch!  You bump into a door.");
                                exercise(A_DEX, false);
                            }
                        } else
                            pline("That door is closed.");
                    }
                } else if (mode == TEST_TRAV)
                    goto testdiag;
                return false;
            }
        } else {
            testdiag: if (dx && dy && !Passes_walls && ((tmpr->doormask & ~D_BROKEN) || block_door(x, y))) {
                /* Diagonal moves into a door are not allowed. */
                if (Blind && mode == DO_MOVE)
                    feel_location(x, y);
                return false;
            }
        }
    }
    if (dx && dy && bad_rock(youmonst.data, ux, y) && bad_rock(youmonst.data, x, uy)) {
        /* Move at a diagonal. */
        if (In_sokoban(&u.uz)) {
            if (mode == DO_MOVE)
                You("cannot pass that way.");
            return false;
        }
        if (bigmonst(youmonst.data)) {
            if (mode == DO_MOVE)
                Your("body is too large to fit through.");
            return false;
        }
        if (invent && (inv_weight() + weight_cap() > 600)) {
            if (mode == DO_MOVE)
                You("are carrying too much to get through.");
            return false;
        }
    }
    /* Pick travel path that does not require crossing a trap.
     * Avoid water and lava using the usual running rules.
     * (but not u.ux/u.uy because findtravelpath walks toward u.ux/u.uy) */
    if (flags.run == 8 && mode != DO_MOVE && (x != u.ux || y != u.uy)) {
        struct trap* t = t_at(x, y);

        if ((t && t->tseen) || (!Levitation && !Flying && !is_clinger(youmonst.data) && (is_pool(x, y) || is_lava(x, y)) && levl[x][y].seenv))
            return false;
    }

    ust = &levl[ux][uy];

    /* Now see if other things block our way . . */
    if (dx && dy && !Passes_walls && (IS_DOOR(ust->typ) && ((ust->doormask & ~D_BROKEN) || block_entry(x, y)))) {
        /* Can't move at a diagonal out of a doorway with door. */
        return false;
    }

    if (sobj_at(BOULDER, x, y) && (In_sokoban(&u.uz) || !Passes_walls)) {
        if (!(Blind || Hallucination()) && (flags.run >= 2) && mode != TEST_TRAV)
            return false;
        if (mode == DO_MOVE) {
            /* tunneling monsters will chew before pushing */
            if (tunnels(youmonst.data) && !needspick(youmonst.data) && !In_sokoban(&u.uz)) {
                if (still_chewing(x, y))
                    return false;
            } else if (moverock() < 0)
                return false;
        } else if (mode == TEST_TRAV) {
            struct obj* obj;

            /* don't pick two boulders in a row, unless there's a way thru */
            if (sobj_at(BOULDER, ux, uy) && !In_sokoban(&u.uz)) {
                if (!Passes_walls && !(tunnels(youmonst.data) && !needspick(youmonst.data)) && !carrying(PICK_AXE) && !carrying(DWARVISH_MATTOCK) && !((obj = carrying(WAN_DIGGING)) && !objects[obj->otyp].oc_name_known))
                    return false;
            }
        }
        /* assume you'll be able to push it when you get there... */
    }

    /* OK, it is a legal place to move. */
    return true;
}

void spoteffects(bool pick) {
    struct monst *mtmp;

    if (u.uinwater) {
        int was_underwater;

        if (!is_pool(u.ux, u.uy)) {
            if (Is_waterlevel(&u.uz))
                You("pop into an air bubble.");
            else if (is_lava(u.ux, u.uy))
                You("leave the water..."); /* oops! */
            else
                You("are on solid %s again.", is_ice(u.ux, u.uy) ? "ice" : "land");
        } else if (Is_waterlevel(&u.uz))
            goto stillinwater;
        else if (Levitation)
            You("pop out of the water like a cork!");
        else if (Flying)
            You("fly out of the water.");
        else if (Wwalking)
            You("slowly rise above the surface.");
        else
            goto stillinwater;
        was_underwater = Underwater && !Is_waterlevel(&u.uz);
        u.uinwater = 0; /* leave the water */
        if (was_underwater) { /* restore vision */
            docrt();
            vision_full_recalc = 1;
        }
    }
    stillinwater: ;
    if (!Levitation && !u.ustuck && !Flying) {
        /* limit recursive calls through teleds() */
        if (is_pool(u.ux, u.uy) || is_lava(u.ux, u.uy)) {
            if (u.usteed && !is_flyer(u.usteed->data) && !is_floater(u.usteed->data) && !is_clinger(u.usteed->data)) {
                dismount_steed(Underwater ?
                DISMOUNT_FELL :
                                            DISMOUNT_GENERIC);
                /* dismount_steed() -> float_down() -> pickup() */
                if (!Is_airlevel(&u.uz) && !Is_waterlevel(&u.uz))
                    pick = false;
            } else if (is_lava(u.ux, u.uy)) {
                if (lava_effects())
                    return;
            } else if (!Wwalking && drown())
                return;
        }
    }
    check_special_room(false);
    if (IS_SINK(levl[u.ux][u.uy].typ) && Levitation)
        dosinkfall();
    if (!in_steed_dismounting) { /* if dismounting, we'll check again later */
        struct trap *trap = t_at(u.ux, u.uy);
        bool pit;
        pit = (trap && (trap->ttyp == PIT || trap->ttyp == SPIKED_PIT));
        if (trap && pit)
            dotrap(trap, 0); /* fall into pit */
        if (pick)
            pickup(1);
        if (trap && !pit)
            dotrap(trap, 0); /* fall into arrow trap, etc. */
    }
    if ((mtmp = m_at(u.ux, u.uy)) && !u.uswallow) {
        mtmp->mundetected = mtmp->msleeping = 0;
        switch (mtmp->data->mlet) {
            case S_PIERCER: {
                char name[BUFSZ];
                Amonnam(name, BUFSZ, mtmp);
                pline("%s suddenly drops from the %s!", name, ceiling(u.ux, u.uy));
                if (mtmp->mtame) {
                    /* jumps to greet you, not attack */
                } else if (uarmh && is_metallic(uarmh)) {
                    pline("Its blow glances off your helmet.");
                } else if (u.uac + 3 <= rnd(20)) {
                    char name[BUFSZ];
                    x_monnam(name, BUFSZ, mtmp, ARTICLE_A, "falling", 0, true);
                    You("are almost hit by %s!", name);
                } else {
                    char name[BUFSZ];
                    x_monnam(name, BUFSZ, mtmp, ARTICLE_A, "falling", 0, true);
                    You("are hit by %s!", name);
                    int dmg = d(4, 6);
                    if (Half_physical_damage)
                        dmg = (dmg + 1) / 2;
                    mdamageu(mtmp, dmg);
                }
            }
                break;
            default: /* monster surprises you. */
                if (mtmp->mtame) {
                    char name[BUFSZ];
                    Amonnam(name, BUFSZ, mtmp);
                    pline("%s jumps near you from the %s.", name, ceiling(u.ux, u.uy));
                } else if (mtmp->mpeaceful) {
                    char name[BUFSZ];
                    a_monnam(name, BUFSZ, mtmp);
                    You("surprise %s!", Blind && !sensemon(mtmp) ? something : name);
                    mtmp->mpeaceful = 0;
                } else {
                    char name[BUFSZ];
                    Amonnam(name, BUFSZ, mtmp);
                    pline("%s attacks you by surprise!", name);
                }
                break;
        }
        mnexto(mtmp); /* have to move the monster */
    }
}

static bool monstinroom(struct permonst *mdat, int roomno) {
    struct monst *mtmp;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
        if (!DEADMONSTER(mtmp) && mtmp->data == mdat && index(in_rooms(mtmp->mx, mtmp->my, 0), roomno + ROOMOFFSET))
            return (true);
    return (false);
}

char * in_rooms(signed char x, signed char y, int typewanted) {
    static char buf[5];
    char rno, *ptr = &buf[4];
    int typefound, min_x, min_y, max_x, max_y_offset, step;
    struct rm *lev;

#define goodtype(rno) (!typewanted || \
             ((typefound = rooms[rno - ROOMOFFSET].rtype) == typewanted) || \
             ((typewanted == SHOPBASE) && (typefound > SHOPBASE))) \

    switch (rno = levl[x][y].roomno) {
        case NO_ROOM:
            return (ptr);
        case SHARED:
            step = 2;
            break;
        case SHARED_PLUS:
            step = 1;
            break;
        default: /* i.e. a regular room # */
            if (goodtype(rno))
                *(--ptr) = rno;
            return (ptr);
    }

    min_x = x - 1;
    max_x = x + 1;
    if (x < 1)
        min_x += step;
    else if (x >= COLNO)
        max_x -= step;

    min_y = y - 1;
    max_y_offset = 2;
    if (min_y < 0) {
        min_y += step;
        max_y_offset -= step;
    } else if ((min_y + max_y_offset) >= ROWNO)
        max_y_offset -= step;

    for (x = min_x; x <= max_x; x += step) {
        lev = &levl[x][min_y];
        y = 0;
        if (((rno = lev[y].roomno) >= ROOMOFFSET) && !index(ptr, rno) && goodtype(rno))
            *(--ptr) = rno;
        y += step;
        if (y > max_y_offset)
            continue;
        if (((rno = lev[y].roomno) >= ROOMOFFSET) && !index(ptr, rno) && goodtype(rno))
            *(--ptr) = rno;
        y += step;
        if (y > max_y_offset)
            continue;
        if (((rno = lev[y].roomno) >= ROOMOFFSET) && !index(ptr, rno) && goodtype(rno))
            *(--ptr) = rno;
    }
    return (ptr);
}

/* is (x,y) in a town? */
bool in_town(int x, int y) {
    s_level *slev = Is_special(&u.uz);
    struct mkroom *sroom;
    bool has_subrooms = false;

    if (!slev || !slev->flags.town)
        return false;

    /*
     * See if (x,y) is in a room with subrooms, if so, assume it's the
     * town.  If there are no subrooms, the whole level is in town.
     */
    for (sroom = &rooms[0]; sroom->hx > 0; sroom++) {
        if (sroom->nsubrooms > 0) {
            has_subrooms = true;
            if (inside_room(sroom, x, y))
                return true;
        }
    }

    return !has_subrooms;
}

static void move_update(bool newlev) {
    char *ptr1, *ptr2, *ptr3, *ptr4;

    strcpy(u.urooms0, u.urooms);
    strcpy(u.ushops0, u.ushops);
    if (newlev) {
        u.urooms[0] = '\0';
        u.uentered[0] = '\0';
        u.ushops[0] = '\0';
        u.ushops_entered[0] = '\0';
        strcpy(u.ushops_left, u.ushops0);
        return;
    }
    strcpy(u.urooms, in_rooms(u.ux, u.uy, 0));

    for (ptr1 = &u.urooms[0], ptr2 = &u.uentered[0], ptr3 = &u.ushops[0], ptr4 = &u.ushops_entered[0]; *ptr1; ptr1++) {
        if (!index(u.urooms0, *ptr1))
            *(ptr2++) = *ptr1;
        if (IS_SHOP(*ptr1 - ROOMOFFSET)) {
            *(ptr3++) = *ptr1;
            if (!index(u.ushops0, *ptr1))
                *(ptr4++) = *ptr1;
        }
    }
    *ptr2 = '\0';
    *ptr3 = '\0';
    *ptr4 = '\0';

    /* filter u.ushops0 -> u.ushops_left */
    for (ptr1 = &u.ushops0[0], ptr2 = &u.ushops_left[0]; *ptr1; ptr1++)
        if (!index(u.ushops, *ptr1))
            *(ptr2++) = *ptr1;
    *ptr2 = '\0';
}

void check_special_room(bool newlev) {
    struct monst *mtmp;
    char *ptr;

    move_update(newlev);

    if (*u.ushops0)
        u_left_shop(u.ushops_left, newlev);

    if (!*u.uentered && !*u.ushops_entered) /* implied by newlev */
        return; /* no entrance messages necessary */

    /* Did we just enter a shop? */
    if (*u.ushops_entered)
        u_entered_shop(u.ushops_entered);

    for (ptr = &u.uentered[0]; *ptr; ptr++) {
        int roomno = *ptr - ROOMOFFSET, rt = rooms[roomno].rtype;

        /* Did we just enter some other special room? */
        /* vault.c insists that a vault remain a VAULT,
         * and temples should remain TEMPLEs,
         * but everything else gives a message only the first time */
        switch (rt) {
            case ZOO:
                pline("Welcome to David's treasure zoo!");
                break;
            case SWAMP:
                pline("It %s rather %s down here.", Blind ? "feels" : "looks", Blind ? "humid" : "muddy");
                break;
            case COURT:
                You("enter an opulent throne room!");
                break;
            case LEPREHALL:
                You("enter a leprechaun hall!");
                break;
            case MORGUE:
                if (midnight()) {
                    const char *run = locomotion(youmonst.data, "Run");
                    pline("%s away!  %s away!", run, run);
                } else {
                    You("have an uncanny feeling...");
                }
                break;
            case BEEHIVE:
                You("enter a giant beehive!");
                break;
            case COCKNEST:
                You("enter a disgusting nest!");
                break;
            case ANTHOLE:
                You("enter an anthole!");
                break;
            case BARRACKS:
                if (monstinroom(&mons[PM_SOLDIER], roomno) || monstinroom(&mons[PM_SERGEANT], roomno) || monstinroom(&mons[PM_LIEUTENANT], roomno) || monstinroom(&mons[PM_CAPTAIN], roomno))
                    You("enter a military barracks!");
                else
                    You("enter an abandoned barracks.");
                break;
            case DELPHI:
                if (monstinroom(&mons[PM_ORACLE], roomno))
                    verbalize("%s, %s, welcome to Delphi!", Hello((struct monst *)0), plname);
                break;
            case TEMPLE:
                intemple(roomno + ROOMOFFSET);
                /* fall through */
            default:
                rt = 0;
        }

        if (rt != 0) {
            rooms[roomno].rtype = OROOM;
            if (!search_special(rt)) {
                /* No more room of that type */
                switch (rt) {
                    case COURT:
                        level.flags.has_court = 0;
                        break;
                    case SWAMP:
                        level.flags.has_swamp = 0;
                        break;
                    case MORGUE:
                        level.flags.has_morgue = 0;
                        break;
                    case ZOO:
                        level.flags.has_zoo = 0;
                        break;
                    case BARRACKS:
                        level.flags.has_barracks = 0;
                        break;
                    case TEMPLE:
                        level.flags.has_temple = 0;
                        break;
                    case BEEHIVE:
                        level.flags.has_beehive = 0;
                        break;
                }
            }
            if (rt == COURT || rt == SWAMP || rt == MORGUE || rt == ZOO)
                for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
                    if (!DEADMONSTER(mtmp) && !Stealth && !rn2(3))
                        mtmp->msleeping = 0;
        }
    }
}

int dopickup(void) {
    int count;
    struct trap *traphere = t_at(u.ux, u.uy);
    /* awful kludge to work around parse()'s pre-decrement */
    count = multi ? multi + 1 : 0;
    multi = 0; /* always reset */
    if (u.uswallow) {
        if (!u.ustuck->minvent) {
            if (is_animal(u.ustuck->data)) {
                char name[BUFSZ];
                mon_nam(name, BUFSZ, u.ustuck);
                You("pick up %s%s tongue.", name, possessive_suffix(name));
                pline("But it's kind of slimy, so you drop it.");
            } else {
                You("don't %s anything in here to pick up.", Blind ? "feel" : "see");
            }
            return 1;
        } else {
            int tmpcount = -count;
            return loot_mon(u.ustuck, &tmpcount, (bool *)0);
        }
    }
    if (is_pool(u.ux, u.uy)) {
        if (Wwalking || is_floater(youmonst.data) || is_clinger(youmonst.data) || (Flying && !Breathless)) {
            You("cannot dive into the water to pick things up.");
            return (0);
        } else if (!Underwater) {
            You_cant("even see the bottom, let alone pick up %s.", something);
            return (0);
        }
    }
    if (is_lava(u.ux, u.uy)) {
        if (Wwalking || is_floater(youmonst.data) || is_clinger(youmonst.data) || (Flying && !Breathless)) {
            You_cant("reach the bottom to pick things up.");
            return (0);
        } else if (!likes_lava(youmonst.data)) {
            You("would burn to a crisp trying to pick things up.");
            return (0);
        }
    }
    if (!OBJ_AT(u.ux, u.uy)) {
        There("is nothing here to pick up.");
        return (0);
    }
    if (!can_reach_floor()) {
        if (u.usteed && P_SKILL(P_RIDING) < P_BASIC) {
            char name[BUFSZ];
            y_monnam(name, BUFSZ, u.usteed);
            You("aren't skilled enough to reach from %s.", name);
        } else {
            You("cannot reach the %s.", surface(u.ux, u.uy));
        }
        return 0;
    }

    if (traphere && traphere->tseen) {
        /* Allow pickup from holes and trap doors that you escaped from
         * because that stuff is teetering on the edge just like you, but
         * not pits, because there is an elevation discrepancy with stuff
         * in pits.
         */
        if ((traphere->ttyp == PIT || traphere->ttyp == SPIKED_PIT) && (!u.utrap || (u.utrap && u.utraptype != TT_PIT))) {
            You("cannot reach the bottom of the pit.");
            return (0);
        }
    }

    return (pickup(-count));
}

/* stop running if we see something interesting */
/* turn around a corner if that is the only way we can proceed */
/* do not turn left or right twice */
void lookaround (void) {
    int x, y, i, x0 = 0, y0 = 0, m0 = 1, i0 = 9;
    int corrct = 0, noturn = 0;
    struct monst *mtmp;
    struct trap *trap;

    /* Grid bugs stop if trying to move diagonal, even if blind.  Maybe */
    /* they polymorphed while in the middle of a long move. */
    if (u.umonnum == PM_GRID_BUG && u.dx && u.dy) {
        nomul(0);
        return;
    }

    if(Blind || flags.run == 0) return;
    for(x = u.ux-1; x <= u.ux+1; x++) for(y = u.uy-1; y <= u.uy+1; y++) {
        if(!isok(x,y)) continue;

        if(u.umonnum == PM_GRID_BUG && x != u.ux && y != u.uy) continue;

        if(x == u.ux && y == u.uy) continue;

        if((mtmp = m_at(x,y)) &&
                    mtmp->m_ap_type != M_AP_FURNITURE &&
                    mtmp->m_ap_type != M_AP_OBJECT &&
                    (!mtmp->minvis || See_invisible) && !mtmp->mundetected) {
            if((flags.run != 1 && !mtmp->mtame)
                                        || (x == u.ux+u.dx && y == u.uy+u.dy))
                goto stop;
        }

        if (levl[x][y].typ == STONE) continue;
        if (x == u.ux-u.dx && y == u.uy-u.dy) continue;

        if (IS_ROCK(levl[x][y].typ) || (levl[x][y].typ == ROOM) ||
            IS_AIR(levl[x][y].typ))
            continue;
        else if (closed_door(x,y) ||
                 (mtmp && mtmp->m_ap_type == M_AP_FURNITURE &&
                  (mtmp->mappearance == S_hcdoor ||
                   mtmp->mappearance == S_vcdoor))) {
            if(x != u.ux && y != u.uy) continue;
            if(flags.run != 1) goto stop;
            goto bcorr;
        } else if (levl[x][y].typ == CORR) {
bcorr:
            if(levl[u.ux][u.uy].typ != ROOM) {
                if(flags.run == 1 || flags.run == 3 || flags.run == 8) {
                    i = dist2(x,y,u.ux+u.dx,u.uy+u.dy);
                    if(i > 2) continue;
                    if(corrct == 1 && dist2(x,y,x0,y0) != 1)
                        noturn = 1;
                    if(i < i0) {
                        i0 = i;
                        x0 = x;
                        y0 = y;
                        m0 = mtmp ? 1 : 0;
                    }
                }
                corrct++;
            }
            continue;
        } else if ((trap = t_at(x,y)) && trap->tseen) {
            if(flags.run == 1) goto bcorr;      /* if you must */
            if(x == u.ux+u.dx && y == u.uy+u.dy) goto stop;
            continue;
        } else if (is_pool(x,y) || is_lava(x,y)) {
            /* water and lava only stop you if directly in front, and stop
             * you even if you are running
             */
            if(!Levitation && !Flying && !is_clinger(youmonst.data) &&
                                x == u.ux+u.dx && y == u.uy+u.dy)
                        /* No Wwalking check; otherwise they'd be able
                         * to test boots by trying to SHIFT-direction
                         * into a pool and seeing if the game allowed it
                         */
                        goto stop;
            continue;
        } else {                /* e.g. objects or trap or stairs */
            if(flags.run == 1) goto bcorr;
            if(flags.run == 8) continue;
            if(mtmp) continue;          /* d */
            if(((x == u.ux - u.dx) && (y != u.uy + u.dy)) ||
               ((y == u.uy - u.dy) && (x != u.ux + u.dx)))
               continue;
        }
stop:
        nomul(0);
        return;
    } /* end for loops */

    if(corrct > 1 && flags.run == 2) goto stop;
    if((flags.run == 1 || flags.run == 3 || flags.run == 8) &&
        !noturn && !m0 && i0 && (corrct == 1 || (corrct == 2 && i0 == 1)))
    {
        /* make sure that we do not turn too far */
        if(i0 == 2) {
            if(u.dx == y0-u.uy && u.dy == u.ux-x0)
                i = 2;          /* straight turn right */
            else
                i = -2;         /* straight turn left */
        } else if(u.dx && u.dy) {
            if((u.dx == u.dy && y0 == u.uy) || (u.dx != u.dy && y0 != u.uy))
                i = -1;         /* half turn left */
            else
                i = 1;          /* half turn right */
        } else {
            if((x0-u.ux == y0-u.uy && !u.dy) || (x0-u.ux != y0-u.uy && u.dy))
                i = 1;          /* half turn right */
            else
                i = -1;         /* half turn left */
        }

        i += u.last_str_turn;
        if(i <= 2 && i >= -2) {
            u.last_str_turn = i;
            u.dx = x0-u.ux;
            u.dy = y0-u.uy;
        }
    }
}

/* something like lookaround, but we are not running */
/* react only to monsters that might hit us */
int monster_nearby (void) {
        int x,y;
        struct monst *mtmp;

        /* Also see the similar check in dochugw() in monmove.c */
        for(x = u.ux-1; x <= u.ux+1; x++)
            for(y = u.uy-1; y <= u.uy+1; y++) {
                if(!isok(x,y)) continue;
                if(x == u.ux && y == u.uy) continue;
                if((mtmp = m_at(x,y)) &&
                   mtmp->m_ap_type != M_AP_FURNITURE &&
                   mtmp->m_ap_type != M_AP_OBJECT &&
                   (!mtmp->mpeaceful || Hallucination()) &&
                   (!is_hider(mtmp->data) || !mtmp->mundetected) &&
                   !noattacks(mtmp->data) &&
                   mtmp->mcanmove && !mtmp->msleeping &&  /* aplvax!jcn */
                   !onscary(u.ux, u.uy, mtmp) &&
                   canspotmon(mtmp))
                        return(1);
        }
        return(0);
}

void nomul (int nval) {
        if(multi < nval) return;        /* This is a bug fix by ab@unido */
        u.uinvulnerable = false;        /* Kludge to avoid ctrl-C bug -dlc */
        u.usleep = 0;
        multi = nval;
        flags.travel = iflags.travel1 = flags.mv = flags.run = 0;
}

/* called when a non-movement, multi-turn action has completed */
void unmul (const char *msg_override) {
        multi = 0;      /* caller will usually have done this already */
        if (msg_override) nomovemsg = msg_override;
        else if (!nomovemsg) nomovemsg = You_can_move_again;
        if (*nomovemsg) plines(nomovemsg);
        nomovemsg = 0;
        u.usleep = 0;
        if (afternmv) (*afternmv)();
        afternmv = 0;
}

static void maybe_wail (void) {
    static short powers[] = { TELEPORT, SEE_INVIS, POISON_RES, COLD_RES,
                              SHOCK_RES, FIRE_RES, SLEEP_RES, DISINT_RES,
                              TELEPORT_CONTROL, STEALTH, FAST, INVIS };

    if (moves <= wailmsg + 50) return;

    wailmsg = moves;
    if (Role_if(PM_WIZARD) || Race_if(PM_ELF) || Role_if(PM_VALKYRIE)) {
        const char *who;
        int i, powercnt;

        who = (Role_if(PM_WIZARD) || Role_if(PM_VALKYRIE)) ?
                urole.name.m : "Elf";
        if (u.uhp == 1) {
            pline("%s is about to die.", who);
        } else {
            for (i = 0, powercnt = 0; i < SIZE(powers); ++i)
                if (u.uprops[powers[i]].intrinsic & INTRINSIC) ++powercnt;

            pline(powercnt >= 4 ? "%s, all your powers will be lost..."
                                : "%s, your life force is running out.", who);
        }
    } else {
        You_hear(u.uhp == 1 ? "the wailing of the Banshee..."
                            : "the howling of the CwnAnnwn...");
    }
}

void losehp (int n, struct Killer k) {
    if (Upolyd) {
        u.mh -= n;
        if (u.mhmax < u.mh) u.mhmax = u.mh;
        if (u.mh < 1)
            rehumanize();
        else if (n > 0 && u.mh*10 < u.mhmax && Unchanging)
            maybe_wail();
        return;
    }

    u.uhp -= n;
    if(u.uhp > u.uhpmax)
        u.uhpmax = u.uhp;       /* perhaps n was negative */
    if(u.uhp < 1) {
        killer = k;          /* the thing that killed you */
        You("die...");
        done(DIED);
    } else if (n > 0 && u.uhp*10 < u.uhpmax) {
        maybe_wail();
    }
}

int weight_cap (void) {
        long carrcap;

        carrcap = 25*(ACURRSTR + ACURR(A_CON)) + 50;
        if (Upolyd) {
                /* consistent with can_carry() in mon.c */
                if (youmonst.data->mlet == S_NYMPH)
                        carrcap = MAX_CARR_CAP;
                else if (!youmonst.data->cwt)
                        carrcap = (carrcap * (long)youmonst.data->msize) / MZ_HUMAN;
                else if (!strongmonst(youmonst.data)
                        || (strongmonst(youmonst.data) && (youmonst.data->cwt > WT_HUMAN)))
                        carrcap = (carrcap * (long)youmonst.data->cwt / WT_HUMAN);
        }

        if (Levitation || Is_airlevel(&u.uz)    /* pugh@cornell */
                        || (u.usteed && strongmonst(u.usteed->data))
        )
                carrcap = MAX_CARR_CAP;
        else {
                if(carrcap > MAX_CARR_CAP) carrcap = MAX_CARR_CAP;
                if (!Flying) {
                        if(EWounded_legs & LEFT_SIDE) carrcap -= 100;
                        if(EWounded_legs & RIGHT_SIDE) carrcap -= 100;
                }
                if (carrcap < 0) carrcap = 0;
        }
        return((int) carrcap);
}


/* returns how far beyond the normal capacity the player is currently. */
/* inv_weight() is negative if the player is below normal capacity. */
int inv_weight (void) {
        struct obj *otmp = invent;
        int wt = 0;

        /* when putting stuff into containers, gold is inserted at the head
           of invent for easier manipulation by askchain & co, but it's also
           retained in u.ugold in order to keep the status line accurate; we
           mustn't add its weight in twice under that circumstance */
        wt = (otmp && otmp->oclass == COIN_CLASS) ? 0 :
                (int)((u.ugold + 50L) / 100L);
        while (otmp) {
                if (otmp->otyp != BOULDER || !throws_rocks(youmonst.data))
                        wt += otmp->owt;
                otmp = otmp->nobj;
        }
        wc = weight_cap();
        return (wt - wc);
}

/*
 * Returns 0 if below normal capacity, or the number of "capacity units"
 * over the normal capacity the player is loaded.  Max is 5.
 */
int calc_capacity (int xtra_wt) {
    int cap, wt = inv_weight() + xtra_wt;

    if (wt <= 0) return UNENCUMBERED;
    if (wc <= 1) return OVERLOADED;
    cap = (wt*2 / wc) + 1;
    return min(cap, OVERLOADED);
}

int near_capacity (void) {
    return calc_capacity(0);
}

int max_capacity (void) {
    int wt = inv_weight();

    return (wt - (2 * wc));
}

bool check_capacity (const char *str) {
    if(near_capacity() >= EXT_ENCUMBER) {
        if(str)
            plines(str);
        else
            You_cant("do that while carrying so much stuff.");
        return 1;
    }
    return 0;
}

int inv_cnt (void) {
    struct obj *otmp = invent;
    int ct = 0;

    while(otmp){
        ct++;
        otmp = otmp->nobj;
    }
    return(ct);
}

struct Killer killed_by_const(enum KillerMethod method) {
    printf("killed_by_const %s\n", killer_method_enum_str[method]);
    return (struct Killer){method, NULL, NULL, NULL, NULL, 0};
}
struct Killer killed_by_monster(enum KillerMethod method, const struct monst *m) {
    printf("killed_by_monster %s\n", killer_method_enum_str[method]);
    return (struct Killer){method, m, NULL, NULL, NULL, 0};
}
struct Killer killed_by_object(enum KillerMethod method, const struct obj *o) {
    printf("killed_by_object %s\n", killer_method_enum_str[method]);
    return (struct Killer){method, NULL, o, NULL, NULL, 0};
}
struct Killer killed_by_string(enum KillerMethod method, const char *string) {
    printf("killed_by_string %s\n", killer_method_enum_str[method]);
    return (struct Killer){method, NULL, NULL, NULL, string, 0};
}
struct Killer killed_by_int(enum KillerMethod method, int i) {
    printf("killed_by_int %s\n", killer_method_enum_str[method]);
    return (struct Killer){method, NULL, NULL, NULL, NULL, i};
}
struct Killer killed_by_artifact(enum KillerMethod method, const struct artifact *art) {
    printf("killed_by_artifact %s\n", killer_method_enum_str[method]);
    return (struct Killer){method, NULL, NULL, art, NULL, 0};
}
