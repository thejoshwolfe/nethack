/* See LICENSE in the root of this project for change info */

/*
 * This file contains the different functions designed to manipulate the
 * musical instruments and their various effects.
 *
 * Actually the list of instruments / effects is :
 *
 * (wooden) flute       may calm snakes if player has enough dexterity
 * magic flute          may put monsters to sleep:  area of effect depends
 *                      on player level.
 * (tooled) horn        Will awaken monsters:  area of effect depends on player
 *                      level.  May also scare monsters.
 * fire horn            Acts like a wand of fire.
 * frost horn           Acts like a wand of cold.
 * bugle                Will awaken soldiers (if any):  area of effect depends
 *                      on player level.
 * (wooden) harp        May calm nymph if player has enough dexterity.
 * magic harp           Charm monsters:  area of effect depends on player
 *                      level.
 * (leather) drum       Will awaken monsters like the horn.
 * drum of earthquake   Will initiate an earthquake whose intensity depends
 *                      on player level.  That is, it creates random pits
 *                      called here chasms.
 */

#include "music.h"

#include <stdbool.h>
#include <string.h>

#include "rm_util.h"
#include "display_util.h"
#include "dungeon_util.h"
#include "attrib.h"
#include "cmd.h"
#include "dbridge.h"
#include "decl.h"
#include "display.h"
#include "do.h"
#include "do_name.h"
#include "dog.h"
#include "dungeon.h"
#include "flag.h"
#include "global.h"
#include "hack.h"
#include "hacklib.h"
#include "invent.h"
#include "mhitm.h"
#include "mkobj.h"
#include "mkroom.h"
#include "mon.h"
#include "monattk.h"
#include "mondata.h"
#include "monmove.h"
#include "monst.h"
#include "monsym.h"
#include "obj.h"
#include "objclass.h"
#include "objnam.h"
#include "onames.h"
#include "permonst.h"
#include "pline.h"
#include "pm.h"
#include "rm.h"
#include "rnd.h"
#include "shk.h"
#include "trap.h"
#include "util.h"
#include "vision.h"
#include "you.h"
#include "youprop.h"
#include "zap.h"

static void awaken_monsters(int);
static void put_monsters_to_sleep(int);
static void charm_snakes(int);
static void calm_nymphs(int);
static void charm_monsters(int);
static void do_earthquake(int);
static int do_improvisation(struct obj *);

/*
 * Wake every monster in range...
 */

static void awaken_monsters (int distance) {
    struct monst *mtmp = fmon;
    int distm;

    while(mtmp) {
        if (!DEADMONSTER(mtmp)) {
            distm = distu(mtmp->mx, mtmp->my);
            if (distm < distance) {
                mtmp->msleeping = 0;
                mtmp->mcanmove = 1;
                mtmp->mfrozen = 0;
                /* May scare some monsters */
                if (distm < distance/3 &&
                        !resist(mtmp, TOOL_CLASS, 0, NOTELL))
                    monflee(mtmp, 0, false, true);
            }
        }
        mtmp = mtmp->nmon;
    }
}

/*
 * Make monsters fall asleep.  Note that they may resist the spell.
 */
static void put_monsters_to_sleep (int distance) {
    struct monst *mtmp = fmon;

    while(mtmp) {
        if (!DEADMONSTER(mtmp) && distu(mtmp->mx, mtmp->my) < distance &&
                sleep_monst(mtmp, d(10,10), TOOL_CLASS)) {
            mtmp->msleeping = 1; /* 10d10 turns + wake_nearby to rouse */
            slept_monst(mtmp);
        }
        mtmp = mtmp->nmon;
    }
}

/*
 * Charm snakes in range.  Note that the snakes are NOT tamed.
 */

static void charm_snakes (int distance) {
    struct monst *mtmp = fmon;
    int could_see_mon, was_peaceful;

    while (mtmp) {
        if (!DEADMONSTER(mtmp) && mtmp->data->mlet == S_SNAKE && mtmp->mcanmove &&
                distu(mtmp->mx, mtmp->my) < distance) {
            was_peaceful = mtmp->mpeaceful;
            mtmp->mpeaceful = 1;
            mtmp->mavenge = 0;
            could_see_mon = canseemon(mtmp);
            mtmp->mundetected = 0;
            newsym(mtmp->mx, mtmp->my);
            if (canseemon(mtmp)) {
                if (!could_see_mon) {
                    char name[BUFSZ];
                    a_monnam(name, BUFSZ, mtmp);
                    You("notice %s, swaying with the music.", name);
                } else {
                    char name[BUFSZ];
                    Monnam(name, BUFSZ, mtmp);
                    pline("%s freezes, then sways with the music%s.", name,
                            was_peaceful ? "" : ", and now seems quieter");
                }
            }
        }
        mtmp = mtmp->nmon;
    }
}

/*
 * Calm nymphs in range.
 */
static void calm_nymphs (int distance) {
    struct monst *mtmp = fmon;

    while (mtmp) {
        if (!DEADMONSTER(mtmp) && mtmp->data->mlet == S_NYMPH && mtmp->mcanmove &&
                distu(mtmp->mx, mtmp->my) < distance) {
            mtmp->msleeping = 0;
            mtmp->mpeaceful = 1;
            mtmp->mavenge = 0;
            if (canseemon(mtmp)) {
                char name[BUFSZ];
                Monnam(name, BUFSZ, mtmp);
                pline( "%s listens cheerfully to the music, then seems quieter.", name);
            }
        }
        mtmp = mtmp->nmon;
    }
}

/* Awake only soldiers of the level. */
void awaken_soldiers (void) {
    struct monst *mtmp = fmon;

    while(mtmp) {
        if (!DEADMONSTER(mtmp) &&
                is_mercenary(mtmp->data) && mtmp->data != &mons[PM_GUARD]) {
            mtmp->mpeaceful = mtmp->msleeping = mtmp->mfrozen = 0;
            mtmp->mcanmove = 1;
            if (canseemon(mtmp)) {
                char name[BUFSZ];
                Monnam(name, BUFSZ, mtmp);
                pline("%s is now ready for battle!", name);
            } else {
                Norep("You hear the rattle of battle gear being readied.");
            }
        }
        mtmp = mtmp->nmon;
    }
}

/* Charm monsters in range.  Note that they may resist the spell.
 * If swallowed, range is reduced to 0.
 */

static void charm_monsters (int distance) {
    struct monst *mtmp, *mtmp2;

    if (u.uswallow) {
        if (!resist(u.ustuck, TOOL_CLASS, 0, NOTELL))
            (void) tamedog(u.ustuck, (struct obj *) 0);
    } else {
        for (mtmp = fmon; mtmp; mtmp = mtmp2) {
            mtmp2 = mtmp->nmon;
            if (DEADMONSTER(mtmp)) continue;

            if (distu(mtmp->mx, mtmp->my) <= distance) {
                if (!resist(mtmp, TOOL_CLASS, 0, NOTELL))
                    (void) tamedog(mtmp, (struct obj *) 0);
            }
        }
    }

}

/* Generate earthquake :-) of desired force.
 * That is:  create random chasms (pits).
 */
static void do_earthquake (int force) {
    int x,y;
    struct monst *mtmp;
    struct obj *otmp;
    struct trap *chasm;
    int start_x, start_y, end_x, end_y;

    start_x = u.ux - (force * 2);
    start_y = u.uy - (force * 2);
    end_x = u.ux + (force * 2);
    end_y = u.uy + (force * 2);
    if (start_x < 1) start_x = 1;
    if (start_y < 1) start_y = 1;
    if (end_x >= COLNO) end_x = COLNO - 1;
    if (end_y >= ROWNO) end_y = ROWNO - 1;
    for (x=start_x; x<=end_x; x++) for (y=start_y; y<=end_y; y++) {
        if ((mtmp = m_at(x,y)) != 0) {
            wakeup(mtmp);   /* peaceful monster will become hostile */
            if (mtmp->mundetected && is_hider(mtmp->data)) {
                mtmp->mundetected = 0;
                if (cansee(x,y)) {
                    char name[BUFSZ];
                    Amonnam(name, BUFSZ, mtmp);
                    pline("%s is shaken loose from the ceiling!", name);
                } else {
                    You_hear("a thumping sound.");
                }
                if (x==u.ux && y==u.uy) {
                    char name[BUFSZ];
                    mon_nam(name, BUFSZ, mtmp);
                    You("easily dodge the falling %s.", name);
                }
                newsym(x,y);
            }
        }
        if (!rn2(14 - force)) switch (levl[x][y].typ) {
            case FOUNTAIN : /* Make the fountain disappear */
                if (cansee(x,y))
                    pline_The("fountain falls into a chasm.");
                goto do_pit;
            case SINK :
                if (cansee(x,y))
                    pline_The("kitchen sink falls into a chasm.");
                goto do_pit;
            case ALTAR :
                if (Is_astralevel(&u.uz) || Is_sanctum(&u.uz)) break;

                if (cansee(x,y))
                    pline_The("altar falls into a chasm.");
                goto do_pit;
            case GRAVE :
                if (cansee(x,y))
                    pline_The("headstone topples into a chasm.");
                goto do_pit;
            case THRONE :
                if (cansee(x,y))
                    pline_The("throne falls into a chasm.");
                /* Falls into next case */
            case ROOM :
            case CORR : /* Try to make a pit */
do_pit:             chasm = maketrap(x,y,PIT);
                    if (!chasm) break;  /* no pit if portal at that location */
                    chasm->tseen = 1;

                    levl[x][y].doormask = 0;

                    mtmp = m_at(x,y);

                    if ((otmp = sobj_at(BOULDER, x, y)) != 0) {
                        if (cansee(x, y))
                            pline("KADOOM! The boulder falls into a chasm%s!",
                                    ((x == u.ux) && (y == u.uy)) ? " below you" : "");
                        if (mtmp)
                            mtmp->mtrapped = 0;
                        obj_extract_self(otmp);
                        (void) flooreffects(otmp, x, y, "");
                        break;
                    }

                    /* We have to check whether monsters or player
                       falls in a chasm... */

                    if (mtmp) {
                        if(!is_flyer(mtmp->data) && !is_clinger(mtmp->data)) {
                            mtmp->mtrapped = 1;
                            if (cansee(x,y)) {
                                char name[BUFSZ];
                                Monnam(name, BUFSZ, mtmp);
                                pline("%s falls into a chasm!", name);
                            } else if (flags.soundok && humanoid(mtmp->data)) {
                                You_hear("a scream!");
                            }
                            mselftouch(mtmp, "Falling, ", true);
                            if (mtmp->mhp > 0)
                                if ((mtmp->mhp -= rnd(6)) <= 0) {
                                    if(!cansee(x,y)) {
                                        pline("It is destroyed!");
                                    } else {
                                        char name[BUFSZ];
                                        if (mtmp->mtame) {
                                            x_monnam(name, BUFSZ, mtmp,
                                                    ARTICLE_THE, "poor",
                                                    mtmp->mnamelth ? SUPPRESS_SADDLE : 0,
                                                    false);
                                        } else {
                                            mon_nam(name, BUFSZ, mtmp);
                                        }
                                        You("destroy %s!", name);
                                    }
                                    xkilled(mtmp,0);
                                }
                        }
                    } else if (x == u.ux && y == u.uy) {
                        if (Levitation || Flying || is_clinger(youmonst.data)) {
                            pline("A chasm opens up under you!");
                            You("don't fall in!");
                        } else {
                            You("fall into a chasm!");
                            u.utrap = rn1(6,2);
                            u.utraptype = TT_PIT;
                            losehp(rnd(6), killed_by_const(KM_FELL_INTO_CHASM));
                            selftouch("Falling, you");
                        }
                    } else newsym(x,y);
                    break;
            case DOOR : /* Make the door collapse */
                    if (levl[x][y].doormask == D_NODOOR) goto do_pit;
                    if (cansee(x,y))
                        pline_The("door collapses.");
                    if (*in_rooms(x, y, SHOPBASE))
                        add_damage(x, y, 0L);
                    levl[x][y].doormask = D_NODOOR;
                    unblock_point(x,y);
                    newsym(x,y);
                    break;
        }
    }
}

/*
 * The player is trying to extract something from his/her instrument.
 */
static int do_improvisation (struct obj *instr) {
    int damage, do_spec = !Confusion();
    if (!do_spec)
        pline("What you produce is quite far from music...");
    else
        You("start playing %s.", the(xname(instr)));

    switch (instr->otyp) {
        case MAGIC_FLUTE:               /* Make monster fall asleep */
            if (do_spec && instr->spe > 0) {
                consume_obj_charge(instr, true);

                You("produce soft music.");
                put_monsters_to_sleep(u.ulevel * 5);
                exercise(A_DEX, true);
                break;
            } /* else FALLTHRU */
        case WOODEN_FLUTE:              /* May charm snakes */
            {
                do_spec &= (rn2(ACURR(A_DEX)) + u.ulevel > 25);
                char verb_clause[BUFSZ];
                Tobjnam(verb_clause, BUFSZ, instr, do_spec ? "trill" : "toot");
                pline("%s.", verb_clause);
                if (do_spec) charm_snakes(u.ulevel * 3);
                exercise(A_DEX, true);
            }
            break;
        case FROST_HORN:                /* Idem wand of cold */
        case FIRE_HORN:                 /* Idem wand of fire */
            if (do_spec && instr->spe > 0) {
                consume_obj_charge(instr, true);

                if (!getdir(NULL)) {
                    char vibrate_clause[BUFSZ];
                    Tobjnam(vibrate_clause, BUFSZ, instr, "vibrate");
                    pline("%s.", vibrate_clause);
                    break;
                } else if (!u.delta.x && !u.delta.y && !u.delta.z) {
                    if ((damage = zapyourself(instr, true)) != 0) {
                        losehp(damage, killed_by_const(KM_USING_MAGIC_HORN_ON_SELF));
                    }
                } else {
                    buzz((instr->otyp == FROST_HORN) ? AD_COLD-1 : AD_FIRE-1,
                            rn1(6,6), u.ux, u.uy, u.delta.x, u.delta.y);
                }
                makeknown(instr->otyp);
                break;
            } /* else FALLTHRU */
        case TOOLED_HORN:               /* Awaken or scare monsters */
            You("produce a frightful, grave sound.");
            awaken_monsters(u.ulevel * 30);
            exercise(A_WIS, false);
            break;
        case BUGLE:                     /* Awaken & attract soldiers */
            You("extract a loud noise from %s.", the(xname(instr)));
            awaken_soldiers();
            exercise(A_WIS, false);
            break;
        case MAGIC_HARP:                /* Charm monsters */
            if (do_spec && instr->spe > 0) {
                consume_obj_charge(instr, true);

                char produce_clause[BUFSZ];
                Tobjnam(produce_clause, BUFSZ, instr, "produce");
                pline("%s very attractive music.", produce_clause);
                charm_monsters((u.ulevel - 1) / 3 + 1);
                exercise(A_DEX, true);
                break;
            } /* else FALLTHRU */
        case WOODEN_HARP:               /* May calm Nymph */
            do_spec &= (rn2(ACURR(A_DEX)) + u.ulevel > 25);
            pline("%s %s.", The(xname(instr)),
                    do_spec ? "produces a lilting melody" : "twangs");
            if (do_spec) calm_nymphs(u.ulevel * 3);
            exercise(A_DEX, true);
            break;
        case DRUM_OF_EARTHQUAKE:        /* create several pits */
            if (do_spec && instr->spe > 0) {
                consume_obj_charge(instr, true);

                You("produce a heavy, thunderous rolling!");
                pline_The("entire dungeon is shaking around you!");
                do_earthquake((u.ulevel - 1) / 3 + 1);
                /* shake up monsters in a much larger radius... */
                awaken_monsters(ROWNO * COLNO);
                makeknown(DRUM_OF_EARTHQUAKE);
                break;
            } /* else FALLTHRU */
        case LEATHER_DRUM:              /* Awaken monsters */
            You("beat a deafening row!");
            awaken_monsters(u.ulevel * 40);
            exercise(A_WIS, false);
            break;
        default:
            impossible("What a weird instrument (%d)!", instr->otyp);
            break;
    }
    return 2;               /* That takes time */
}

/*
 * So you want music...
 */
int do_play_instrument (struct obj *instr) {
    char buf[BUFSZ], c = 'y';
    char *s;
    int x,y;
    bool ok;

    if (Underwater) {
        You_cant("play music underwater!");
        return(0);
    }
    if (instr->otyp != LEATHER_DRUM && instr->otyp != DRUM_OF_EARTHQUAKE) {
        c = yn("Improvise?");
    }
    if (c == 'n') {
        if (u.uevent.uheard_tune == 2 && yn("Play the passtune?") == 'y') {
            strcpy(buf, tune);
        } else {
            getlin("What tune are you playing? [5 notes, A-G]", buf);
            (void)mungspaces(buf);
            /* convert to uppercase and change any "H" to the expected "B" */
            for (s = buf; *s; s++) {
                *s = highc(*s);
                if (*s == 'H') *s = 'B';
            }
        }
        You("extract a strange sound from %s!", the(xname(instr)));
        /* Check if there was the Stronghold drawbridge near
         * and if the tune conforms to what we're waiting for.
         */
        if(Is_stronghold(&u.uz)) {
            exercise(A_WIS, true);              /* just for trying */
            if(!strcmp(buf,tune)) {
                /* Search for the drawbridge */
                for(y=u.uy-1; y<=u.uy+1; y++) {
                    for(x=u.ux-1;x<=u.ux+1;x++) {
                        if (isok(x,y)) {
                            if(find_drawbridge(&x,&y)) {
                                u.uevent.uheard_tune = 2; /* tune now fully known */
                                if(levl[x][y].typ == DRAWBRIDGE_DOWN)
                                    close_drawbridge(x,y);
                                else
                                    open_drawbridge(x,y);
                                return 0;
                            }
                        }
                    }
                }
            } else if(flags.soundok) {
                if (u.uevent.uheard_tune < 1) u.uevent.uheard_tune = 1;
                /* Okay, it wasn't the right tune, but perhaps
                 * we can give the player some hints like in the
                 * Mastermind game */
                ok = false;
                for(y = u.uy-1; y <= u.uy+1 && !ok; y++)
                    for(x = u.ux-1; x <= u.ux+1 && !ok; x++)
                        if(isok(x,y))
                            if(IS_DRAWBRIDGE(levl[x][y].typ) ||
                                    is_drawbridge_wall(x,y) >= 0)
                                ok = true;
                if(ok) { /* There is a drawbridge near */
                    int tumblers, gears;
                    bool matched[5];

                    tumblers = gears = 0;
                    for(x=0; x < 5; x++)
                        matched[x] = false;

                    for(x=0; x < (int)strlen(buf); x++)
                        if(x < 5) {
                            if(buf[x] == tune[x]) {
                                gears++;
                                matched[x] = true;
                            } else
                                for(y=0; y < 5; y++)
                                    if(!matched[y] &&
                                            buf[x] == tune[y] &&
                                            buf[y] != tune[y]) {
                                        tumblers++;
                                        matched[y] = true;
                                        break;
                                    }
                        }
                    if(tumblers)
                        if(gears)
                            You_hear("%d tumbler%s click and %d gear%s turn.",
                                    tumblers, plur(tumblers), gears, plur(gears));
                        else
                            You_hear("%d tumbler%s click.",
                                    tumblers, plur(tumblers));
                    else if(gears) {
                        You_hear("%d gear%s turn.", gears, plur(gears));
                        /* could only get `gears == 5' by playing five
                           correct notes followed by excess; otherwise,
                           tune would have matched above */
                        if (gears == 5) u.uevent.uheard_tune = 2;
                    }
                }
            }
        }
        return 1;
    } else
        return do_improvisation(instr);
}
