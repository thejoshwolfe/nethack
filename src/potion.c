/* See LICENSE in the root of this project for change info */

#include "potion.h"
#include "hack.h"
#include "pm_props.h"
#include "display.h"
#include "everything.h"

bool notonhead = false;

static int nothing, unkn;
static const char beverages[] = { POTION_CLASS, 0 };

static long itimeout(long);
static long itimeout_incr(long,int);
static void ghost_from_bottle(void);
static short mixtype(struct obj *,struct obj *);

static const char vismsg[] = "vision seems to %s for a moment but is %s now.";
static const char eyemsg[] = "%s momentarily %s.";

const char *bottlenames[] = {
        "bottle", "phial", "flagon", "carafe", "flask", "jar", "vial"
};


/* force `val' to be within valid range for intrinsic timeout value */
static long itimeout (long val) {
    if (val >= TIMEOUT) val = TIMEOUT;
    else if (val < 1) val = 0;

    return val;
}

/* increment `old' by `incr' and force result to be valid intrinsic timeout */
static long itimeout_incr (long old, int incr) {
    return itimeout((old & TIMEOUT) + (long)incr);
}

/* set the timeout field of intrinsic `which' */
void set_itimeout (long *which, long val) {
    *which &= ~TIMEOUT;
    *which |= itimeout(val);
}

/* increment the timeout field of intrinsic `which' */
void incr_itimeout (long *which, int incr) {
    set_itimeout(which, itimeout_incr(*which, incr));
}

void make_confused (long xtime, bool talk) {
        long old = HConfusion;

        if (!xtime && old) {
                if (talk)
                    You_feel("less %s now.",
                        Hallucination() ? "trippy" : "confused");
        }

        set_itimeout(&HConfusion, xtime);
}

void make_stunned (long xtime, bool talk) {
        long old = HStun;

        if (!xtime && old) {
                if (talk)
                    You_feel("%s now.",
                        Hallucination() ? "less wobbly" : "a bit steadier");
        }
        if (xtime && !old) {
                if (talk) {
                        if (u.usteed)
                                You("wobble in the saddle.");
                        else
                        You("%s...", stagger(youmonst.data, "stagger"));
                }
        }

        set_itimeout(&HStun, xtime);
}

// const char *cause,      /* sickness cause */
void make_sick ( long xtime, const char *cause, bool talk, int type) {
    long old = Sick;

    if (xtime > 0L) {
        if (Sick_resistance()) return;
        if (!old) {
            /* newly sick */
            You_feel("deathly sick.");
        } else {
            /* already sick */
            if (talk) You_feel("%s worse.",
                    xtime <= Sick/2L ? "much" : "even");
        }
        set_itimeout(&Sick, xtime);
        u.usick_type |= type;
    } else if (old && (type & u.usick_type)) {
        /* was sick, now not */
        u.usick_type &= ~type;
        if (u.usick_type) { /* only partly cured */
            if (talk) You_feel("somewhat better.");
            set_itimeout(&Sick, Sick * 2); /* approximation */
        } else {
            if (talk) pline("What a relief!");
            Sick = 0L;              /* set_itimeout(&Sick, 0L) */
        }
    }

    if (Sick) {
        exercise(A_CON, false);
        if (cause) {
            (void) strncpy(u.usick_cause, cause, sizeof(u.usick_cause));
            u.usick_cause[sizeof(u.usick_cause)-1] = 0;
        }
        else
            u.usick_cause[0] = 0;
    } else
        u.usick_cause[0] = 0;
}

void make_vomiting (long xtime, bool talk) {
    long old = Vomiting;

    if(!xtime && old)
        if(talk) You_feel("much less nauseated now.");

    set_itimeout(&Vomiting, xtime);
}

void make_blinded (long xtime, bool talk) {
    long old = Blinded;
    bool u_could_see, can_see_now;
    int eyecnt;
    char buf[BUFSZ];

    /* we need to probe ahead in case the Eyes of the Overworld
       are or will be overriding blindness */
    u_could_see = !Blind;
    Blinded = xtime ? 1L : 0L;
    can_see_now = !Blind;
    Blinded = old;          /* restore */

    if (u.usleep) talk = false;

    if (can_see_now && !u_could_see) {      /* regaining sight */
        if (talk) {
            if (Hallucination())
                pline("Far out!  Everything is all cosmic again!");
            else
                You("can see again.");
        }
    } else if (old && !xtime) {
        /* clearing temporary blindness without toggling blindness */
        if (talk) {
            if (!haseyes(youmonst.data)) {
                strange_feeling((struct obj *)0, (char *)0);
            } else if (Blindfolded) {
                strcpy(buf, body_part(EYE));
                eyecnt = eyecount(youmonst.data);
                Your(eyemsg, (eyecnt == 1) ? buf : makeplural(buf),
                        (eyecnt == 1) ? "itches" : "itch");
            } else {        /* Eyes of the Overworld */
                Your(vismsg, "brighten",
                        Hallucination() ? "sadder" : "normal");
            }
        }
    }

    if (u_could_see && !can_see_now) {      /* losing sight */
        if (talk) {
            if (Hallucination())
                pline("Oh, bummer!  Everything is dark!  Help!");
            else
                pline("A cloud of darkness falls upon you.");
        }
        /* Before the hero goes blind, set the ball&chain variables. */
        if (Punished) set_bc(0);
    } else if (!old && xtime) {
        /* setting temporary blindness without toggling blindness */
        if (talk) {
            if (!haseyes(youmonst.data)) {
                strange_feeling((struct obj *)0, (char *)0);
            } else if (Blindfolded) {
                strcpy(buf, body_part(EYE));
                eyecnt = eyecount(youmonst.data);
                Your(eyemsg, (eyecnt == 1) ? buf : makeplural(buf),
                        (eyecnt == 1) ? "twitches" : "twitch");
            } else {        /* Eyes of the Overworld */
                Your(vismsg, "dim",
                        Hallucination() ? "happier" : "normal");
            }
        }
    }

    set_itimeout(&Blinded, xtime);

    if (u_could_see ^ can_see_now) {  /* one or the other but not both */
        vision_full_recalc = 1;     /* blindness just got toggled */
        if (Blind_telepat || Infravision) see_monsters();
    }
}

// long xtime,     /* nonzero if this is an attempt to turn on hallucination */
// bool talk,
// long mask      /* nonzero if resistance status should change by mask */
bool make_hallucinated ( long xtime, bool talk, long mask) {
    long old = u.uprops[HALLUC].intrinsic;
    bool changed = 0;
    const char *message, *verb;

    message = (!xtime) ? "Everything %s SO boring now." :
        "Oh wow!  Everything %s so cosmic!";
    verb = (!Blind) ? "looks" : "feels";

    if (mask) {
        if (u.uprops[HALLUC].intrinsic) changed = true;

        if (!xtime) u.uprops[HALLUC_RES].extrinsic |= mask;
        else u.uprops[HALLUC_RES].extrinsic &= ~mask;
    } else {
        if (!u.uprops[HALLUC_RES].extrinsic && (!!u.uprops[HALLUC].intrinsic != !!xtime))
            changed = true;
        set_itimeout(&u.uprops[HALLUC].intrinsic, xtime);

        /* clearing temporary hallucination without toggling vision */
        if (!changed && !u.uprops[HALLUC].intrinsic && old && talk) {
            if (!haseyes(youmonst.data)) {
                strange_feeling((struct obj *)0, (char *)0);
            } else if (Blind) {
                char buf[BUFSZ];
                int eyecnt = eyecount(youmonst.data);

                strcpy(buf, body_part(EYE));
                Your(eyemsg, (eyecnt == 1) ? buf : makeplural(buf),
                        (eyecnt == 1) ? "itches" : "itch");
            } else {        /* Grayswandir */
                Your(vismsg, "flatten", "normal");
            }
        }
    }

    if (changed) {
        if (u.uswallow) {
            swallowed(0);   /* redraw swallow display */
        } else {
            /* The see_* routines should be called *before* the pline. */
            see_monsters();
            see_objects();
            see_traps();
        }

        if (talk) pline(message, verb);
    }
    return changed;
}

static void ghost_from_bottle (void) {
    struct monst *mtmp = makemon(&mons[PM_GHOST], u.ux, u.uy, NO_MM_FLAGS);

    if (!mtmp) {
        pline("This bottle turns out to be empty.");
        return;
    }
    if (Blind) {
        pline("As you open the bottle, %s emerges.", something);
        return;
    }
    pline("As you open the bottle, an enormous %s emerges!",
            Hallucination() ? rndmonnam() : "ghost");
    if(flags.verbose)
        You("are frightened to death, and unable to move.");
    nomul(-3);
    nomovemsg = "You regain your composure.";
}

/* "Quaffing is like drinking, except you spill more."  -- Terry Pratchett */
int dodrink (void) {
    struct obj *otmp;
    const char *potion_descr;

    if (Strangled) {
        pline("If you can't breathe air, how can you drink liquid?");
        return 0;
    }
    /* Is there a fountain to drink from here? */
    if (IS_FOUNTAIN(levl[u.ux][u.uy].typ) && !Levitation) {
        if(yn("Drink from the fountain?") == 'y') {
            drinkfountain();
            return 1;
        }
    }
    /* Or a kitchen sink? */
    if (IS_SINK(levl[u.ux][u.uy].typ)) {
        if (yn("Drink from the sink?") == 'y') {
            drinksink();
            return 1;
        }
    }

    /* Or are you surrounded by water? */
    if (Underwater) {
        if (yn("Drink the water around you?") == 'y') {
            pline("Do you know what lives in this water!");
            return 1;
        }
    }

    otmp = getobj(beverages, "drink");
    if(!otmp) return(0);
    otmp->in_use = true;            /* you've opened the stopper */

#define POTION_OCCUPANT_CHANCE(n) (13 + 2*(n))  /* also in muse.c */

    potion_descr = OBJ_DESCR(objects[otmp->otyp]);
    if (potion_descr) {
        if (!strcmp(potion_descr, "milky") &&
                flags.ghost_count < MAXMONNO &&
                !rn2(POTION_OCCUPANT_CHANCE(flags.ghost_count))) {
            ghost_from_bottle();
            useup(otmp);
            return(1);
        } else if (!strcmp(potion_descr, "smoky") &&
                flags.djinni_count < MAXMONNO &&
                !rn2(POTION_OCCUPANT_CHANCE(flags.djinni_count))) {
            djinni_from_bottle(otmp);
            useup(otmp);
            return(1);
        }
    }
    return dopotion(otmp);
}

int dopotion (struct obj *otmp) {
    int retval;

    otmp->in_use = true;
    nothing = unkn = 0;
    if((retval = peffects(otmp)) >= 0) return(retval);

    if(nothing) {
        unkn++;
        You("have a %s feeling for a moment, then it passes.",
                Hallucination() ? "normal" : "peculiar");
    }
    if(otmp->dknown && !objects[otmp->otyp].oc_name_known) {
        if(!unkn) {
            makeknown(otmp->otyp);
            more_experienced(0,10);
        } else if(!objects[otmp->otyp].oc_uname)
            docall(otmp);
    }
    useup(otmp);
    return(1);
}

int peffects (struct obj *otmp) {
    int i, ii, lim;

    switch(otmp->otyp){
        case POT_RESTORE_ABILITY:
        case SPE_RESTORE_ABILITY:
            unkn++;
            if(otmp->cursed) {
                pline("Ulch!  This makes you feel mediocre!");
                break;
            } else {
                pline("Wow!  This makes you feel %s!",
                        (otmp->blessed) ?
                        (unfixable_trouble_count(false) ? "better" : "great")
                        : "good");
                i = rn2(A_MAX);             /* start at a random point */
                for (ii = 0; ii < A_MAX; ii++) {
                    lim = AMAX(i);
                    if (i == A_STR && u.uhs >= 3) --lim;    /* WEAK */
                    if (ABASE(i) < lim) {
                        ABASE(i) = lim;
                        /* only first found if not blessed */
                        if (!otmp->blessed) break;
                    }
                    if(++i >= A_MAX) i = 0;
                }
            }
            break;
        case POT_HALLUCINATION:
            if (Hallucination() || Halluc_resistance()) nothing++;
            (void) make_hallucinated(itimeout_incr(u.uprops[HALLUC].intrinsic,
                        rn1(200, 600 - 300 * bcsign(otmp))),
                    true, 0L);
            break;
        case POT_WATER:
            if(!otmp->blessed && !otmp->cursed) {
                pline("This tastes like water.");
                u.uhunger += rnd(10);
                newuhs(false);
                break;
            }
            unkn++;
            if(is_undead(youmonst.data) || is_demon(youmonst.data) ||
                    u.ualign.type == A_CHAOTIC) {
                if(otmp->blessed) {
                    pline("This burns like acid!");
                    exercise(A_CON, false);
                    if (u.ulycn >= LOW_PM) {
                        Your("affinity to %s disappears!",
                                makeplural(mons[u.ulycn].mname));
                        if (youmonst.data == &mons[u.ulycn])
                            you_unwere(false);
                        u.ulycn = NON_PM;   /* cure lycanthropy */
                    }
                    losehp(d(2,6), killed_by_const(KM_POTION_HOLY_WATER));
                } else if(otmp->cursed) {
                    You_feel("quite proud of yourself.");
                    healup(d(2,6),0,0,0);
                    if (u.ulycn >= LOW_PM && !Upolyd) you_were();
                    exercise(A_CON, true);
                }
            } else {
                if(otmp->blessed) {
                    You_feel("full of awe.");
                    make_sick(0L, (char *) 0, true, SICK_ALL);
                    exercise(A_WIS, true);
                    exercise(A_CON, true);
                    if (u.ulycn >= LOW_PM)
                        you_unwere(true);   /* "Purified" */
                    /* make_confused(0L,true); */
                } else {
                    if(u.ualign.type == A_LAWFUL) {
                        pline("This burns like acid!");
                        losehp(d(2,6), killed_by_const(KM_POTION_UNHOLY_WATER));
                    } else {
                        You_feel("full of dread.");
                    }
                    if (u.ulycn >= LOW_PM && !Upolyd) you_were();
                    exercise(A_CON, false);
                }
            }
            break;
        case POT_BOOZE:
            unkn++;
            pline("Ooph!  This tastes like %s%s!",
                    otmp->odiluted ? "watered down " : "",
                    Hallucination() ? "dandelion wine" : "liquid fire");
            if (!otmp->blessed)
                make_confused(itimeout_incr(HConfusion, d(3,8)), false);
            /* the whiskey makes us feel better */
            if (!otmp->odiluted) healup(1, 0, false, false);
            u.uhunger += 10 * (2 + bcsign(otmp));
            newuhs(false);
            exercise(A_WIS, false);
            if(otmp->cursed) {
                You("pass out.");
                multi = -rnd(15);
                nomovemsg = "You awake with a headache.";
            }
            break;
        case POT_ENLIGHTENMENT:
            if(otmp->cursed) {
                unkn++;
                You("have an uneasy feeling...");
                exercise(A_WIS, false);
            } else {
                if (otmp->blessed) {
                    (void) adjattrib(A_INT, 1, false);
                    (void) adjattrib(A_WIS, 1, false);
                }
                You_feel("self-knowledgeable...");
                display_nhwindow(WIN_MESSAGE, false);
                enlightenment(0);
                pline_The("feeling subsides.");
                exercise(A_WIS, true);
            }
            break;
        case SPE_INVISIBILITY:
            /* spell cannot penetrate mummy wrapping */
            if (BInvis && uarmc->otyp == MUMMY_WRAPPING) {
                You_feel("rather itchy under your %s.", xname(uarmc));
                break;
            }
            /* FALLTHRU */
        case POT_INVISIBILITY:
            if (Invis || Blind || BInvis) {
                nothing++;
            } else {
                self_invis_message();
            }
            if (otmp->blessed) HInvis |= FROMOUTSIDE;
            else incr_itimeout(&HInvis, rn1(15,31));
            newsym(u.ux,u.uy);      /* update position */
            if(otmp->cursed) {
                pline("For some reason, you feel your presence is known.");
                aggravate();
            }
            break;
        case POT_SEE_INVISIBLE:
            /* tastes like fruit juice in Rogue */
        case POT_FRUIT_JUICE:
            {
                int msg = Invisible && !Blind;

                unkn++;
                if (otmp->cursed)
                    pline("Yecch!  This tastes %s.",
                            Hallucination() ? "overripe" : "rotten");
                else
                    pline(Hallucination() ?
                            "This tastes like 10%% real %s%s all-natural beverage." :
                            "This tastes like %s%s.",
                            otmp->odiluted ? "reconstituted " : "",
                            fruitname(true));
                if (otmp->otyp == POT_FRUIT_JUICE) {
                    u.uhunger += (otmp->odiluted ? 5 : 10) * (2 + bcsign(otmp));
                    newuhs(false);
                    break;
                }
                if (!otmp->cursed) {
                    /* Tell them they can see again immediately, which
                     * will help them identify the potion...
                     */
                    make_blinded(0L,true);
                }
                if (otmp->blessed)
                    HSee_invisible |= FROMOUTSIDE;
                else
                    incr_itimeout(&HSee_invisible, rn1(100,750));
                set_mimic_blocking(); /* do special mimic handling */
                see_monsters(); /* see invisible monsters */
                newsym(u.ux,u.uy); /* see yourself! */
                if (msg && !Blind) { /* Blind possible if polymorphed */
                    You("can see through yourself, but you are visible!");
                    unkn--;
                }
                break;
            }
        case POT_PARALYSIS:
            if (Free_action)
                You("stiffen momentarily.");
            else {
                if (Levitation || Is_airlevel(&u.uz)||Is_waterlevel(&u.uz))
                    You("are motionlessly suspended.");
                else if (u.usteed)
                    You("are frozen in place!");
                else
                    Your("%s are frozen to the %s!",
                            makeplural(body_part(FOOT)), surface(u.ux, u.uy));
                nomul(-(rn1(10, 25 - 12*bcsign(otmp))));
                nomovemsg = You_can_move_again;
                exercise(A_DEX, false);
            }
            break;
        case POT_SLEEPING:
            if(Sleep_resistance() || Free_action)
                You("yawn.");
            else {
                You("suddenly fall asleep!");
                fall_asleep(-rn1(10, 25 - 12*bcsign(otmp)), true);
            }
            break;
        case POT_MONSTER_DETECTION:
        case SPE_DETECT_MONSTERS:
            if (otmp->blessed) {
                int x, y;

                if (Detect_monsters) nothing++;
                unkn++;
                /* after a while, repeated uses become less effective */
                if (HDetect_monsters >= 300L)
                    i = 1;
                else
                    i = rn1(40,21);
                incr_itimeout(&HDetect_monsters, i);
                for (x = 1; x < COLNO; x++) {
                    for (y = 0; y < ROWNO; y++) {
                        if (levl[x][y].glyph == GLYPH_INVISIBLE) {
                            unmap_object(x, y);
                            newsym(x,y);
                        }
                        if (MON_AT(x,y)) unkn = 0;
                    }
                }
                see_monsters();
                if (unkn) You_feel("lonely.");
                break;
            }
            if (monster_detect(otmp, 0))
                return(1);              /* nothing detected */
            exercise(A_WIS, true);
            break;
        case POT_OBJECT_DETECTION:
        case SPE_DETECT_TREASURE:
            if (object_detect(otmp, 0))
                return(1);              /* nothing detected */
            exercise(A_WIS, true);
            break;
        case POT_SICKNESS:
            pline("Yecch!  This stuff tastes like poison.");
            if (otmp->blessed) {
                pline("(But in fact it was mildly stale %s.)", fruitname(true));
                if (!Role_if(PM_HEALER)) {
                    /* NB: blessed otmp->fromsink is not possible */
                    losehp(1, killed_by_const(KM_MILDLY_CONTAMINATED_POTION));
                }
            } else {
                if(Poison_resistance())
                    pline( "(But in fact it was biologically contaminated %s.)", fruitname(true));
                if (Role_if(PM_HEALER))
                    pline("Fortunately, you have been immunized.");
                else {
                    int typ = rn2(A_MAX);

                    if (!Fixed_abil) {
                        poisontell(typ);
                        (void) adjattrib(typ,
                                Poison_resistance() ? -1 : -rn1(4,3),
                                true);
                    }
                    if(!Poison_resistance()) {
                        if (otmp->fromsink) {
                            losehp(rnd(10)+5*!!(otmp->cursed),
                                    killed_by_const(KM_CONTAMINATED_TAP_WATER));
                        } else {
                            losehp(rnd(10)+5*!!(otmp->cursed),
                                    killed_by_const(KM_CONTAMINATED_POTION));
                        }
                    }
                    exercise(A_CON, false);
                }
            }
            if(Hallucination()) {
                You("are shocked back to your senses!");
                (void) make_hallucinated(0L,false,0L);
            }
            break;
        case POT_CONFUSION:
            if(!Confusion)
                if (Hallucination()) {
                    pline("What a trippy feeling!");
                    unkn++;
                } else
                    pline("Huh, What?  Where am I?");
                else    nothing++;
            make_confused(itimeout_incr(HConfusion,
                        rn1(7, 16 - 8 * bcsign(otmp))),
                    false);
            break;
        case POT_GAIN_ABILITY:
            if(otmp->cursed) {
                pline("Ulch!  That potion tasted foul!");
                unkn++;
            } else if (Fixed_abil) {
                nothing++;
            } else {      /* If blessed, increase all; if not, try up to */
                int itmp; /* 6 times to find one which can be increased. */
                i = -1;             /* increment to 0 */
                for (ii = A_MAX; ii > 0; ii--) {
                    i = (otmp->blessed ? i + 1 : rn2(A_MAX));
                    /* only give "your X is already as high as it can get"
                       message on last attempt (except blessed potions) */
                    itmp = (otmp->blessed || ii == 1) ? 0 : -1;
                    if (adjattrib(i, 1, itmp) && !otmp->blessed)
                        break;
                }
            }
            break;
        case POT_SPEED:
            if(Wounded_legs && !otmp->cursed
                    && !u.usteed /* heal_legs() would heal steeds legs */
              ) {
                heal_legs();
                unkn++;
                break;
            } /* and fall through */
        case SPE_HASTE_SELF:
            if(!Very_fast) /* wwf@doe.carleton.ca */
                You("are suddenly moving %sfaster.",
                        Fast ? "" : "much ");
            else {
                Your("%s get new energy.",
                        makeplural(body_part(LEG)));
                unkn++;
            }
            exercise(A_DEX, true);
            incr_itimeout(&HFast, rn1(10, 100 + 60 * bcsign(otmp)));
            break;
        case POT_BLINDNESS:
            if(Blind) nothing++;
            make_blinded(itimeout_incr(Blinded,
                        rn1(200, 250 - 125 * bcsign(otmp))),
                    (bool)!Blind);
            break;
        case POT_GAIN_LEVEL:
            if (otmp->cursed) {
                unkn++;
                /* they went up a level */
                if((ledger_no(&u.uz) == 1 && u.uhave.amulet) ||
                        Can_rise_up(u.ux, u.uy, &u.uz)) {
                    const char *riseup ="rise up, through the %s!";
                    if(ledger_no(&u.uz) == 1) {
                        You(riseup, ceiling(u.ux,u.uy));
                        goto_level(&earth_level, false, false, false);
                    } else {
                        int newlev = depth(&u.uz)-1;
                        d_level newlevel;

                        get_level(&newlevel, newlev);
                        if(on_level(&newlevel, &u.uz)) {
                            pline("It tasted bad.");
                            break;
                        } else You(riseup, ceiling(u.ux,u.uy));
                        goto_level(&newlevel, false, false, false);
                    }
                }
                else You("have an uneasy feeling.");
                break;
            }
            pluslvl(false);
            if (otmp->blessed)
                /* blessed potions place you at a random spot in the
                 * middle of the new level instead of the low point
                 */
                u.uexp = rndexp(true);
            break;
        case POT_HEALING:
            You_feel("better.");
            healup(d(6 + 2 * bcsign(otmp), 4),
                    !otmp->cursed ? 1 : 0, !!otmp->blessed, !otmp->cursed);
            exercise(A_CON, true);
            break;
        case POT_EXTRA_HEALING:
            You_feel("much better.");
            healup(d(6 + 2 * bcsign(otmp), 8),
                    otmp->blessed ? 5 : !otmp->cursed ? 2 : 0,
                    !otmp->cursed, true);
            (void) make_hallucinated(0L,true,0L);
            exercise(A_CON, true);
            exercise(A_STR, true);
            break;
        case POT_FULL_HEALING:
            You_feel("completely healed.");
            healup(400, 4+4*bcsign(otmp), !otmp->cursed, true);
            /* Restore one lost level if blessed */
            if (otmp->blessed && u.ulevel < u.ulevelmax) {
                /* when multiple levels have been lost, drinking
                   multiple potions will only get half of them back */
                u.ulevelmax -= 1;
                pluslvl(false);
            }
            (void) make_hallucinated(0L,true,0L);
            exercise(A_STR, true);
            exercise(A_CON, true);
            break;
        case POT_LEVITATION:
        case SPE_LEVITATION:
            if (otmp->cursed) HLevitation &= ~I_SPECIAL;
            if(!Levitation) {
                /* kludge to ensure proper operation of float_up() */
                HLevitation = 1;
                float_up();
                /* reverse kludge */
                HLevitation = 0;
                if (otmp->cursed && !Is_waterlevel(&u.uz)) {
                    if((u.ux != xupstair || u.uy != yupstair)
                            && (u.ux != sstairs.sx || u.uy != sstairs.sy || !sstairs.up)
                            && (!xupladder || u.ux != xupladder || u.uy != yupladder)
                      ) {
                        You("hit your %s on the %s.",
                                body_part(HEAD),
                                ceiling(u.ux,u.uy));
                        losehp(uarmh ? 1 : rnd(10), killed_by_const(KM_COLLIDING_WITH_CEILING));
                    } else (void) doup();
                }
            } else
                nothing++;
            if (otmp->blessed) {
                incr_itimeout(&HLevitation, rn1(50,250));
                HLevitation |= I_SPECIAL;
            } else incr_itimeout(&HLevitation, rn1(140,10));
            spoteffects(false);     /* for sinks */
            break;
        case POT_GAIN_ENERGY:                   /* M. Stephenson */
            {       int num;
                if(otmp->cursed)
                    You_feel("lackluster.");
                else
                    pline("Magical energies course through your body.");
                num = rnd(5) + 5 * otmp->blessed + 1;
                u.uenmax += (otmp->cursed) ? -num : num;
                u.uen += (otmp->cursed) ? -num : num;
                if(u.uenmax <= 0) u.uenmax = 0;
                if(u.uen <= 0) u.uen = 0;
                exercise(A_WIS, true);
            }
            break;
        case POT_OIL:                           /* P. Winner */
            {
                bool good_for_you = false;

                if (otmp->lamplit) {
                    if (likes_fire(youmonst.data)) {
                        pline("Ahh, a refreshing drink.");
                        good_for_you = true;
                    } else {
                        You("burn your %s.", body_part(FACE));
                        losehp(d(Fire_resistance() ? 1 : 3, 4),
                                killed_by_const(KM_BURNING_POTION_OF_OIL));
                    }
                } else if(otmp->cursed) {
                    pline("This tastes like castor oil.");
                } else {
                    pline("That was smooth!");
                }
                exercise(A_WIS, good_for_you);
            }
            break;
        case POT_ACID:
            if (Acid_resistance)
                /* Not necessarily a creature who _likes_ acid */
                pline("This tastes %s.", Hallucination() ? "tangy" : "sour");
            else {
                pline("This burns%s!", otmp->blessed ? " a little" :
                        otmp->cursed ? " a lot" : " like acid");
                losehp(d(otmp->cursed ? 2 : 1, otmp->blessed ? 4 : 8),
                        killed_by_const(KM_POTION_OF_ACID));
                exercise(A_CON, false);
            }
            if (Stoned) fix_petrification();
            unkn++; /* holy/unholy water can burn like acid too */
            break;
        case POT_POLYMORPH:
            You_feel("a little %s.", Hallucination() ? "normal" : "strange");
            if (!Unchanging) polyself(false);
            break;
        default:
            impossible("What a funny potion! (%u)", otmp->otyp);
            return(0);
    }
    return(-1);
}

void healup (int nhp, int nxtra, bool curesick, bool cureblind) {
    if (nhp) {
        if (Upolyd) {
            u.mh += nhp;
            if (u.mh > u.mhmax) u.mh = (u.mhmax += nxtra);
        } else {
            u.uhp += nhp;
            if(u.uhp > u.uhpmax) u.uhp = (u.uhpmax += nxtra);
        }
    }
    if(cureblind)   make_blinded(0L,true);
    if(curesick)    make_sick(0L, (char *) 0, true, SICK_ALL);
    return;
}

void strange_feeling (struct obj *obj, const char *txt) {
    if (flags.beginner || !txt)
        You("have a %s feeling for a moment, then it passes.",
                Hallucination() ? "normal" : "strange");
    else
        plines(txt);

    if(!obj)        /* e.g., crystal ball finds no traps */
        return;

    if(obj->dknown && !objects[obj->otyp].oc_name_known &&
            !objects[obj->otyp].oc_uname)
        docall(obj);
    useup(obj);
}

const char * bottlename (void) {
    return bottlenames[rn2(SIZE(bottlenames))];
}

void potionhit (struct monst *mon, struct obj *obj, bool your_fault) {
    const char *botlnam = bottlename();
    bool isyou = (mon == &youmonst);
    int distance;

    if(isyou) {
        distance = 0;
        pline_The("%s crashes on your %s and breaks into shards.", botlnam, body_part(HEAD));
        losehp(rnd(2), killed_by_const(KM_THROWN_POTION));
    } else {
        distance = distu(mon->mx,mon->my);
        if (!cansee(mon->mx,mon->my)) {
            pline("Crash!");
        } else {
            char buf[BUFSZ];

            if(has_head(mon->data)) {
                char pname[BUFSZ];
                monster_possessive(pname, BUFSZ, mon);
                nh_slprintf(buf, BUFSZ, "%s %s", pname, (notonhead ? "body" : "head"));
            } else {
                mon_nam(buf, BUFSZ, mon);
            }
            pline_The("%s crashes on %s and breaks into shards.", botlnam, buf);
        }
        if(rn2(5) && mon->mhp > 1)
            mon->mhp--;
    }

    /* oil doesn't instantly evaporate */
    if (obj->otyp != POT_OIL && cansee(mon->mx,mon->my))
        message_object(MSG_O_EVAPORATES, obj);

    if (isyou) {
        switch (obj->otyp) {
            case POT_OIL:
                if (obj->lamplit)
                    splatter_burning_oil(u.ux, u.uy);
                break;
            case POT_POLYMORPH:
                You_feel("a little %s.", Hallucination() ? "normal" : "strange");
                if (!Unchanging && !Antimagic()) polyself(false);
                break;
            case POT_ACID:
                if (!Acid_resistance) {
                    pline("This burns%s!", obj->blessed ? " a little" :
                            obj->cursed ? " a lot" : "");
                    losehp(d(obj->cursed ? 2 : 1, obj->blessed ? 4 : 8),
                            killed_by_const(KM_POTION_OF_ACID));
                }
                break;
        }
    } else {
        bool angermon = true;

        if (!your_fault) angermon = false;
        switch (obj->otyp) {
            case POT_HEALING:
            case POT_EXTRA_HEALING:
            case POT_FULL_HEALING:
                if (mon->data == &mons[PM_PESTILENCE]) goto do_illness;
                /*FALLTHRU*/
            case POT_RESTORE_ABILITY:
            case POT_GAIN_ABILITY:
do_healing:
                angermon = false;
                if(mon->mhp < mon->mhpmax) {
                    mon->mhp = mon->mhpmax;
                    if (canseemon(mon)) {
                        message_monster(MSG_M_LOOKS_SOUND_AND_HALE, mon);
                    }
                }
                break;
            case POT_SICKNESS:
                if (mon->data == &mons[PM_PESTILENCE]) goto do_healing;
                if (dmgtype(mon->data, AD_DISE) ||
                        dmgtype(mon->data, AD_PEST) || /* won't happen, see prior goto */
                        resists_poison(mon)) {
                    if (canseemon(mon)) {
                        message_monster(MSG_M_LOOKS_UNHARMED, mon);
                    }
                    break;
                }
do_illness:
                if((mon->mhpmax > 3) && !resist(mon, POTION_CLASS, 0, NOTELL))
                    mon->mhpmax /= 2;
                if((mon->mhp > 2) && !resist(mon, POTION_CLASS, 0, NOTELL))
                    mon->mhp /= 2;
                if (mon->mhp > mon->mhpmax) mon->mhp = mon->mhpmax;
                if (canseemon(mon)) {
                    message_monster(MSG_M_LOOKS_RATHER_ILL, mon);
                }
                break;
            case POT_CONFUSION:
            case POT_BOOZE:
                if(!resist(mon, POTION_CLASS, 0, NOTELL))  mon->mconf = true;
                break;
            case POT_INVISIBILITY:
                angermon = false;
                mon_set_minvis(mon);
                break;
            case POT_SLEEPING:
                /* wakeup() doesn't rouse victims of temporary sleep */
                if (sleep_monst(mon, rnd(12), POTION_CLASS)) {
                    message_monster(MSG_M_FALLS_ASLEEP, mon);
                    slept_monst(mon);
                }
                break;
            case POT_PARALYSIS:
                if (mon->mcanmove) {
                    mon->mcanmove = 0;
                    /* really should be rnd(5) for consistency with players
                     * breathing potions, but...
                     */
                    mon->mfrozen = rnd(25);
                }
                break;
            case POT_SPEED:
                angermon = false;
                mon_adjust_speed(mon, 1, obj);
                break;
            case POT_BLINDNESS:
                if(haseyes(mon->data)) {
                    int btmp = 64 + rn2(32) +
                        rn2(32) * !resist(mon, POTION_CLASS, 0, NOTELL);
                    btmp += mon->mblinded;
                    mon->mblinded = min(btmp,127);
                    mon->mcansee = 0;
                }
                break;
            case POT_WATER:
                if (is_undead(mon->data) || is_demon(mon->data) ||
                        is_were(mon->data)) {
                    if (obj->blessed) {
                        message_monster(MSG_M_SHRIEKS_IN_PAIN, mon);
                        mon->mhp -= d(2,6);
                        /* should only be by you */
                        if (mon->mhp < 1) killed(mon);
                        else if (is_were(mon->data) && !is_human(mon->data))
                            new_were(mon);      /* revert to human */
                    } else if (obj->cursed) {
                        angermon = false;
                        if (canseemon(mon))
                            message_monster(MSG_M_LOOKS_HEALTHIER, mon);
                        mon->mhp += d(2,6);
                        if (mon->mhp > mon->mhpmax) mon->mhp = mon->mhpmax;
                        if (is_were(mon->data) && is_human(mon->data) &&
                                !Protection_from_shape_changers)
                        {
                            new_were(mon);      /* transform into beast */
                        }
                    }
                } else if(mon->data == &mons[PM_GREMLIN]) {
                    angermon = false;
                    split_mon(mon, NULL);
                } else if(mon->data == &mons[PM_IRON_GOLEM]) {
                    if (canseemon(mon))
                        message_monster(MSG_M_RUSTS, mon);
                    mon->mhp -= d(1,6);
                    /* should only be by you */
                    if (mon->mhp < 1) killed(mon);
                }
                break;
            case POT_OIL:
                if (obj->lamplit)
                    splatter_burning_oil(mon->mx, mon->my);
                break;
            case POT_ACID:
                if (!resists_acid(mon) && !resist(mon, POTION_CLASS, 0, NOTELL)) {
                    message_monster(MSG_M_SHRIEKS_IN_PAIN, mon);
                    mon->mhp -= d(obj->cursed ? 2 : 1, obj->blessed ? 4 : 8);
                    if (mon->mhp < 1) {
                        if (your_fault)
                            killed(mon);
                        else
                            monkilled(mon, "", AD_ACID);
                    }
                }
                break;
            case POT_POLYMORPH:
                (void) bhitm(mon, obj);
                break;
                /*
                   case POT_GAIN_LEVEL:
                   case POT_LEVITATION:
                   case POT_FRUIT_JUICE:
                   case POT_MONSTER_DETECTION:
                   case POT_OBJECT_DETECTION:
                   break;
                   */
        }
        if (angermon)
            wakeup(mon);
        else
            mon->msleeping = 0;
    }

    /* Note: potionbreathe() does its own docall() */
    if ((distance==0 || ((distance < 3) && rn2(5))) &&
            (!breathless(youmonst.data) || haseyes(youmonst.data)))
        potionbreathe(obj);
    else if (obj->dknown && !objects[obj->otyp].oc_name_known &&
            !objects[obj->otyp].oc_uname && cansee(mon->mx,mon->my))
        docall(obj);
    if(*u.ushops && obj->unpaid) {
        struct monst *shkp =
            shop_keeper(*in_rooms(u.ux, u.uy, SHOPBASE));

        if(!shkp)
            obj->unpaid = 0;
        else {
            (void)stolen_value(obj, u.ux, u.uy,
                    (bool)shkp->mpeaceful, false);
            subfrombill(obj, shkp);
        }
    }
    obfree(obj, (struct obj *)0);
}

/* vapors are inhaled or get in your eyes */
void potionbreathe (struct obj *obj) {
    int i, ii, isdone, kn = 0;

    switch(obj->otyp) {
        case POT_RESTORE_ABILITY:
        case POT_GAIN_ABILITY:
            if(obj->cursed) {
                if (!breathless(youmonst.data))
                    pline("Ulch!  That potion smells terrible!");
                else if (haseyes(youmonst.data)) {
                    int numeyes = eyecount(youmonst.data);
                    Your("%s sting%s!",
                            (numeyes == 1) ? body_part(EYE) : makeplural(body_part(EYE)),
                            (numeyes == 1) ? "s" : "");
                }
                break;
            } else {
                i = rn2(A_MAX);             /* start at a random point */
                for(isdone = ii = 0; !isdone && ii < A_MAX; ii++) {
                    if(ABASE(i) < AMAX(i)) {
                        ABASE(i)++;
                        /* only first found if not blessed */
                        isdone = !(obj->blessed);
                    }
                    if(++i >= A_MAX) i = 0;
                }
            }
            break;
        case POT_FULL_HEALING:
            if (Upolyd && u.mh < u.mhmax) u.mh++;
            if (u.uhp < u.uhpmax) u.uhp++;
            /*FALL THROUGH*/
        case POT_EXTRA_HEALING:
            if (Upolyd && u.mh < u.mhmax) u.mh++;
            if (u.uhp < u.uhpmax) u.uhp++;
            /*FALL THROUGH*/
        case POT_HEALING:
            if (Upolyd && u.mh < u.mhmax) u.mh++;
            if (u.uhp < u.uhpmax) u.uhp++;
            exercise(A_CON, true);
            break;
        case POT_SICKNESS:
            if (!Role_if(PM_HEALER)) {
                if (Upolyd) {
                    if (u.mh <= 5) u.mh = 1; else u.mh -= 5;
                } else {
                    if (u.uhp <= 5) u.uhp = 1; else u.uhp -= 5;
                }
                exercise(A_CON, false);
            }
            break;
        case POT_HALLUCINATION:
            You("have a momentary vision.");
            break;
        case POT_CONFUSION:
        case POT_BOOZE:
            if(!Confusion)
                You_feel("somewhat dizzy.");
            make_confused(itimeout_incr(HConfusion, rnd(5)), false);
            break;
        case POT_INVISIBILITY:
            if (!Blind && !Invis) {
                kn++;
                pline("For an instant you %s!",
                        See_invisible ? "could see right through yourself"
                        : "couldn't see yourself");
            }
            break;
        case POT_PARALYSIS:
            kn++;
            if (!Free_action) {
                pline("%s seems to be holding you.", Something);
                nomul(-rnd(5));
                nomovemsg = You_can_move_again;
                exercise(A_DEX, false);
            } else You("stiffen momentarily.");
            break;
        case POT_SLEEPING:
            kn++;
            if (!Free_action && !Sleep_resistance()) {
                You_feel("rather tired.");
                nomul(-rnd(5));
                nomovemsg = You_can_move_again;
                exercise(A_DEX, false);
            } else You("yawn.");
            break;
        case POT_SPEED:
            if (!Fast) Your("knees seem more flexible now.");
            incr_itimeout(&HFast, rnd(5));
            exercise(A_DEX, true);
            break;
        case POT_BLINDNESS:
            if (!Blind && !u.usleep) {
                kn++;
                pline("It suddenly gets dark.");
            }
            make_blinded(itimeout_incr(Blinded, rnd(5)), false);
            if (!Blind && !u.usleep) Your("%s", vision_clears);
            break;
        case POT_WATER:
            if(u.umonnum == PM_GREMLIN) {
                (void)split_mon(&youmonst, (struct monst *)0);
            } else if (u.ulycn >= LOW_PM) {
                /* vapor from [un]holy water will trigger
                   transformation but won't cure lycanthropy */
                if (obj->blessed && youmonst.data == &mons[u.ulycn])
                    you_unwere(false);
                else if (obj->cursed && !Upolyd)
                    you_were();
            }
            break;
        case POT_ACID:
        case POT_POLYMORPH:
            exercise(A_CON, false);
            break;
            /*
               case POT_GAIN_LEVEL:
               case POT_LEVITATION:
               case POT_FRUIT_JUICE:
               case POT_MONSTER_DETECTION:
               case POT_OBJECT_DETECTION:
               case POT_OIL:
               break;
               */
    }
    /* note: no obfree() */
    if (obj->dknown) {
        if (kn)
            makeknown(obj->otyp);
        else if (!objects[obj->otyp].oc_name_known &&
                !objects[obj->otyp].oc_uname)
            docall(obj);
    }
}

/* returns the potion type when o1 is dipped in o2 */
static short mixtype (struct obj *o1, struct obj *o2) {
    /* cut down on the number of cases below */
    if (o1->oclass == POTION_CLASS &&
            (o2->otyp == POT_GAIN_LEVEL ||
             o2->otyp == POT_GAIN_ENERGY ||
             o2->otyp == POT_HEALING ||
             o2->otyp == POT_EXTRA_HEALING ||
             o2->otyp == POT_FULL_HEALING ||
             o2->otyp == POT_ENLIGHTENMENT ||
             o2->otyp == POT_FRUIT_JUICE)) {
        struct obj *swp;

        swp = o1; o1 = o2; o2 = swp;
    }

    switch (o1->otyp) {
        case POT_HEALING:
            switch (o2->otyp) {
                case POT_SPEED:
                case POT_GAIN_LEVEL:
                case POT_GAIN_ENERGY:
                    return POT_EXTRA_HEALING;
            }
        case POT_EXTRA_HEALING:
            switch (o2->otyp) {
                case POT_GAIN_LEVEL:
                case POT_GAIN_ENERGY:
                    return POT_FULL_HEALING;
            }
        case POT_FULL_HEALING:
            switch (o2->otyp) {
                case POT_GAIN_LEVEL:
                case POT_GAIN_ENERGY:
                    return POT_GAIN_ABILITY;
            }
        case UNICORN_HORN:
            switch (o2->otyp) {
                case POT_SICKNESS:
                    return POT_FRUIT_JUICE;
                case POT_HALLUCINATION:
                case POT_BLINDNESS:
                case POT_CONFUSION:
                    return POT_WATER;
            }
            break;
        case AMETHYST:          /* "a-methyst" == "not intoxicated" */
            if (o2->otyp == POT_BOOZE)
                return POT_FRUIT_JUICE;
            break;
        case POT_GAIN_LEVEL:
        case POT_GAIN_ENERGY:
            switch (o2->otyp) {
                case POT_CONFUSION:
                    return (rn2(3) ? POT_BOOZE : POT_ENLIGHTENMENT);
                case POT_HEALING:
                    return POT_EXTRA_HEALING;
                case POT_EXTRA_HEALING:
                    return POT_FULL_HEALING;
                case POT_FULL_HEALING:
                    return POT_GAIN_ABILITY;
                case POT_FRUIT_JUICE:
                    return POT_SEE_INVISIBLE;
                case POT_BOOZE:
                    return POT_HALLUCINATION;
            }
            break;
        case POT_FRUIT_JUICE:
            switch (o2->otyp) {
                case POT_SICKNESS:
                    return POT_SICKNESS;
                case POT_SPEED:
                    return POT_BOOZE;
                case POT_GAIN_LEVEL:
                case POT_GAIN_ENERGY:
                    return POT_SEE_INVISIBLE;
            }
            break;
        case POT_ENLIGHTENMENT:
            switch (o2->otyp) {
                case POT_LEVITATION:
                    if (rn2(3)) return POT_GAIN_LEVEL;
                    break;
                case POT_FRUIT_JUICE:
                    return POT_BOOZE;
                case POT_BOOZE:
                    return POT_CONFUSION;
            }
            break;
    }

    return 0;
}

/* returns true if something happened (potion should be used up) */
bool get_wet (struct obj *obj) {
    char Your_buf[BUFSZ];

    if (snuff_lit(obj)) return(true);

    if (obj->greased) {
        grease_protect(obj,(char *)0,&youmonst);
        return(false);
    }
    (void) Shk_Your(Your_buf, obj);
    /* (Rusting shop goods ought to be charged for.) */
    switch (obj->oclass) {
        case POTION_CLASS:
            if (obj->otyp == POT_WATER) return false;
            /* KMH -- Water into acid causes an explosion */
            if (obj->otyp == POT_ACID) {
                pline("It boils vigorously!");
                You("are caught in the explosion!");
                losehp(rnd(10), killed_by_const(KM_ELEMENTARY_CHEMISTRY));
                makeknown(obj->otyp);
                return (true);
            }
            pline("%s %s%s.", Your_buf, aobjnam(obj,"dilute"),
                    obj->odiluted ? " further" : "");
            if(obj->unpaid && costly_spot(u.ux, u.uy)) {
                You("dilute it, you pay for it.");
                bill_dummy_object(obj);
            }
            if (obj->odiluted) {
                obj->odiluted = 0;
                obj->blessed = obj->cursed = false;
                obj->otyp = POT_WATER;
            } else obj->odiluted++;
            return true;
        case SCROLL_CLASS:
            if (obj->otyp != SCR_BLANK_PAPER
                    && obj->otyp != SCR_MAIL
               ) {
                if (!Blind) {
                    bool oq1 = obj->quan == 1L;
                    char fade_tense[BUFSZ];
                    otense(fade_tense, BUFSZ, obj, "fade");
                    pline_The("scroll%s %s.", oq1 ? "" : "s", fade_tense);
                }
                if(obj->unpaid && costly_spot(u.ux, u.uy)) {
                    You("erase it, you pay for it.");
                    bill_dummy_object(obj);
                }
                obj->otyp = SCR_BLANK_PAPER;
                obj->spe = 0;
                return true;
            } else break;
        case SPBOOK_CLASS:
            if (obj->otyp != SPE_BLANK_PAPER) {

                if (obj->otyp == SPE_BOOK_OF_THE_DEAD) {
                    pline("%s suddenly heats up; steam rises and it remains dry.",
                            The(xname(obj)));
                } else {
                    if (!Blind) {
                        bool oq1 = obj->quan == 1L;
                        char fade_tense[BUFSZ];
                        otense(fade_tense, BUFSZ, obj, "fade");
                        pline_The("spellbook%s %s.", oq1 ? "" : "s", fade_tense);
                    }
                    if(obj->unpaid && costly_spot(u.ux, u.uy)) {
                        You("erase it, you pay for it.");
                        bill_dummy_object(obj);
                    }
                    obj->otyp = SPE_BLANK_PAPER;
                }
                return true;
            }
            break;
        case WEAPON_CLASS:
            /* Just "fall through" to generic rustprone check for now. */
            /* fall through */
        default:
            if (!obj->oerodeproof && is_rustprone(obj) &&
                    (obj->oeroded < MAX_ERODE) && !rn2(2)) {
                pline("%s %s some%s.",
                        Your_buf, aobjnam(obj, "rust"),
                        obj->oeroded ? " more" : "what");
                obj->oeroded++;
                return true;
            } else break;
    }
    pline("%s %s wet.", Your_buf, aobjnam(obj,"get"));
    return false;
}

int dodip(void) {
    struct obj *potion, *obj;
    struct obj *singlepotion;
    const char *tmp;
    unsigned char here;
    char allowall[2];
    short mixture;
    char qbuf[QBUFSZ], Your_buf[BUFSZ];

    allowall[0] = ALL_CLASSES; allowall[1] = '\0';
    if(!(obj = getobj(allowall, "dip")))
        return(0);

    here = levl[u.ux][u.uy].typ;
    /* Is there a fountain to dip into here? */
    if (IS_FOUNTAIN(here)) {
        if(yn("Dip it into the fountain?") == 'y') {
            dipfountain(obj);
            return(1);
        }
    } else if (is_pool(u.ux,u.uy)) {
        tmp = waterbody_name(u.ux,u.uy);
        sprintf(qbuf, "Dip it into the %s?", tmp);
        if (yn(qbuf) == 'y') {
            if (Levitation) {
                floating_above(tmp);
            } else if (u.usteed && !is_swimmer(u.usteed->data) &&
                    P_SKILL(P_RIDING) < P_BASIC) {
                rider_cant_reach(); /* not skilled enough to reach */
            } else {
                (void) get_wet(obj);
                if (obj->otyp == POT_ACID) useup(obj);
            }
            return 1;
        }
    }

    if(!(potion = getobj(beverages, "dip into")))
        return(0);
    if (potion == obj && potion->quan == 1L) {
        pline("That is a potion bottle, not a Klein bottle!");
        return 0;
    }
    potion->in_use = true;          /* assume it will be used up */
    if(potion->otyp == POT_WATER) {
        bool useeit = !Blind;
        if (useeit) (void) Shk_Your(Your_buf, obj);
        if (potion->blessed) {
            if (obj->cursed) {
                if (useeit)
                    pline("%s %s %s.",
                            Your_buf,
                            aobjnam(obj, "softly glow"),
                            hcolor(NH_AMBER));
                uncurse(obj);
                obj->bknown=1;
poof:
                if(!(objects[potion->otyp].oc_name_known) &&
                        !(objects[potion->otyp].oc_uname))
                    docall(potion);
                useup(potion);
                return(1);
            } else if(!obj->blessed) {
                if (useeit) {
                    tmp = hcolor(NH_LIGHT_BLUE);
                    pline("%s %s with a%s %s aura.",
                            Your_buf,
                            aobjnam(obj, "softly glow"),
                            index(vowels, *tmp) ? "n" : "", tmp);
                }
                bless(obj);
                obj->bknown=1;
                goto poof;
            }
        } else if (potion->cursed) {
            if (obj->blessed) {
                if (useeit)
                    pline("%s %s %s.",
                            Your_buf,
                            aobjnam(obj, "glow"),
                            hcolor((const char *)"brown"));
                unbless(obj);
                obj->bknown=1;
                goto poof;
            } else if(!obj->cursed) {
                if (useeit) {
                    tmp = hcolor(NH_BLACK);
                    pline("%s %s with a%s %s aura.",
                            Your_buf,
                            aobjnam(obj, "glow"),
                            index(vowels, *tmp) ? "n" : "", tmp);
                }
                curse(obj);
                obj->bknown=1;
                goto poof;
            }
        } else
            if (get_wet(obj))
                goto poof;
    } else if (obj->otyp == POT_POLYMORPH ||
            potion->otyp == POT_POLYMORPH) {
        /* some objects can't be polymorphed */
        if (obj->otyp == potion->otyp ||    /* both POT_POLY */
                obj->otyp == WAN_POLYMORPH ||
                obj->otyp == SPE_POLYMORPH ||
                obj == uball || obj == uskin ||
                obj_resists(obj->otyp == POT_POLYMORPH ?
                    potion : obj, 5, 95)) {
            message_const(MSG_NOTHING_HAPPENS);
        } else {
            bool was_wep = false, was_swapwep = false, was_quiver = false;
            short save_otyp = obj->otyp;
            /* KMH, conduct */
            u.uconduct.polypiles++;

            if (obj == uwep) was_wep = true;
            else if (obj == uswapwep) was_swapwep = true;
            else if (obj == uquiver) was_quiver = true;

            obj = poly_obj(obj, STRANGE_OBJECT);

            if (was_wep) setuwep(obj);
            else if (was_swapwep) setuswapwep(obj);
            else if (was_quiver) setuqwep(obj);

            if (obj->otyp != save_otyp) {
                makeknown(POT_POLYMORPH);
                useup(potion);
                prinv((char *)0, obj, 0L);
                return 1;
            } else {
                pline("Nothing seems to happen.");
                goto poof;
            }
        }
        potion->in_use = false;     /* didn't go poof */
        return(1);
    } else if(obj->oclass == POTION_CLASS && obj->otyp != potion->otyp) {
        /* Mixing potions is dangerous... */
        pline_The("potions mix...");
        /* KMH, balance patch -- acid is particularly unstable */
        if (obj->cursed || obj->otyp == POT_ACID || !rn2(10)) {
            pline("BOOM!  They explode!");
            exercise(A_STR, false);
            if (!breathless(youmonst.data) || haseyes(youmonst.data))
                potionbreathe(obj);
            useup(obj);
            useup(potion);
            losehp(rnd(10), killed_by_const(KM_ALCHEMIC_BLAST));
            return(1);
        }

        obj->blessed = obj->cursed = obj->bknown = 0;
        if (Blind || Hallucination()) obj->dknown = 0;

        if ((mixture = mixtype(obj, potion)) != 0) {
            obj->otyp = mixture;
        } else {
            switch (obj->odiluted ? 1 : rnd(8)) {
                case 1:
                    obj->otyp = POT_WATER;
                    break;
                case 2:
                case 3:
                    obj->otyp = POT_SICKNESS;
                    break;
                case 4:
                    {
                        struct obj *otmp;
                        otmp = mkobj(POTION_CLASS,false);
                        obj->otyp = otmp->otyp;
                        obfree(otmp, (struct obj *)0);
                    }
                    break;
                default:
                    if (!Blind)
                        pline_The("mixture glows brightly and evaporates.");
                    useup(obj);
                    useup(potion);
                    return(1);
            }
        }

        obj->odiluted = (obj->otyp != POT_WATER);

        if (obj->otyp == POT_WATER && !Hallucination()) {
            pline_The("mixture bubbles%s.",
                    Blind ? "" : ", then clears");
        } else if (!Blind) {
            pline_The("mixture looks %s.",
                    hcolor(OBJ_DESCR(objects[obj->otyp])));
        }

        useup(potion);
        return(1);
    }


    if(is_poisonable(obj)) {
        if(potion->otyp == POT_SICKNESS && !obj->opoisoned) {
            char buf[BUFSZ];
            if (potion->quan > 1L)
                sprintf(buf, "One of %s", the(xname(potion)));
            else
                strcpy(buf, The(xname(potion)));
            pline("%s forms a coating on %s.",
                    buf, the(xname(obj)));
            obj->opoisoned = true;
            goto poof;
        } else if(obj->opoisoned &&
                (potion->otyp == POT_HEALING ||
                 potion->otyp == POT_EXTRA_HEALING ||
                 potion->otyp == POT_FULL_HEALING)) {
            pline("A coating wears off %s.", the(xname(obj)));
            obj->opoisoned = 0;
            goto poof;
        }
    }

    if (potion->otyp == POT_OIL) {
        bool wisx = false;
        if (potion->lamplit) {      /* burning */
            int omat = objects[obj->otyp].oc_material;
            /* the code here should be merged with fire_damage */
            if (catch_lit(obj)) {
                /* catch_lit does all the work if true */
            } else if (obj->oerodeproof || obj_resists(obj, 5, 95) ||
                    !is_flammable(obj) || obj->oclass == FOOD_CLASS)
            {
                char seem_tense[BUFSZ];
                otense(seem_tense, BUFSZ, obj, "seem");
                pline("%s %s to burn for a moment.", Yname2(obj), seem_tense);
            } else {
                if ((omat == PLASTIC || omat == PAPER) && !obj->oartifact)
                    obj->oeroded = MAX_ERODE;
                pline_The("burning oil %s %s.",
                        obj->oeroded == MAX_ERODE ? "destroys" : "damages",
                        yname(obj));
                if (obj->oeroded == MAX_ERODE) {
                    obj_extract_self(obj);
                    obfree(obj, (struct obj *)0);
                    obj = (struct obj *) 0;
                } else {
                    /* we know it's carried */
                    if (obj->unpaid) {
                        /* create a dummy duplicate to put on bill */
                        verbalize("You burnt it, you bought it!");
                        bill_dummy_object(obj);
                    }
                    obj->oeroded++;
                }
            }
        } else if (potion->cursed) {
            pline_The("potion spills and covers your %s with oil.",
                    makeplural(body_part(FINGER)));
            incr_itimeout(&Glib, d(2,10));
        } else if (obj->oclass != WEAPON_CLASS && !is_weptool(obj)) {
            /* the following cases apply only to weapons */
            goto more_dips;
            /* Oil removes rust and corrosion, but doesn't unburn.
             * Arrows, etc are classed as metallic due to arrowhead
             * material, but dipping in oil shouldn't repair them.
             */
        } else if ((!is_rustprone(obj) && !is_corrodeable(obj)) ||
                is_ammo(obj) || (!obj->oeroded && !obj->oeroded2)) {
            /* uses up potion, doesn't set obj->greased */
            char gleam_tense[BUFSZ];
            otense(gleam_tense, BUFSZ, obj, "gleam");
            pline("%s %s with an oily sheen.", Yname2(obj), gleam_tense);
        } else {
            char are_tense[BUFSZ];
            otense(are_tense, BUFSZ, obj, "are");
            pline("%s %s less %s.",
                    Yname2(obj), are_tense,
                    (obj->oeroded && obj->oeroded2) ? "corroded and rusty" :
                    obj->oeroded ? "rusty" : "corroded");
            if (obj->oeroded > 0) obj->oeroded--;
            if (obj->oeroded2 > 0) obj->oeroded2--;
            wisx = true;
        }
        exercise(A_WIS, wisx);
        makeknown(potion->otyp);
        useup(potion);
        return 1;
    }
more_dips:

    /* Allow filling of MAGIC_LAMPs to prevent identification by player */
    if ((obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP) &&
            (potion->otyp == POT_OIL)) {
        /* Turn off engine before fueling, turn off fuel too :-)  */
        if (obj->lamplit || potion->lamplit) {
            useup(potion);
            explode(u.ux, u.uy, 11, d(6,6), 0, EXPL_FIERY);
            exercise(A_WIS, false);
            return 1;
        }
        /* Adding oil to an empty magic lamp renders it into an oil lamp */
        if ((obj->otyp == MAGIC_LAMP) && obj->spe == 0) {
            obj->otyp = OIL_LAMP;
            obj->age = 0;
        }
        if (obj->age > 1000L) {
            char are_tense[BUFSZ];
            otense(are_tense, BUFSZ, obj, "are");
            pline("%s %s full.", Yname2(obj), are_tense);
            potion->in_use = false; /* didn't go poof */
        } else {
            You("fill %s with oil.", yname(obj));
            check_unpaid(potion);   /* Yendorian Fuel Tax */
            obj->age += 2*potion->age;      /* burns more efficiently */
            if (obj->age > 1500L) obj->age = 1500L;
            useup(potion);
            exercise(A_WIS, true);
        }
        makeknown(POT_OIL);
        obj->spe = 1;
        return 1;
    }

    potion->in_use = false;         /* didn't go poof */
    if ((obj->otyp == UNICORN_HORN || obj->otyp == AMETHYST) &&
            (mixture = mixtype(obj, potion)) != 0) {
        char oldbuf[BUFSZ], newbuf[BUFSZ];
        short old_otyp = potion->otyp;
        bool old_dknown = false;
        bool more_than_one = potion->quan > 1;

        oldbuf[0] = '\0';
        if (potion->dknown) {
            old_dknown = true;
            sprintf(oldbuf, "%s ",
                    hcolor(OBJ_DESCR(objects[potion->otyp])));
        }
        /* with multiple merged potions, split off one and
           just clear it */
        if (potion->quan > 1L) {
            singlepotion = splitobj(potion, 1L);
        } else singlepotion = potion;

        if(singlepotion->unpaid && costly_spot(u.ux, u.uy)) {
            You("use it, you pay for it.");
            bill_dummy_object(singlepotion);
        }
        singlepotion->otyp = mixture;
        singlepotion->blessed = 0;
        if (mixture == POT_WATER)
            singlepotion->cursed = singlepotion->odiluted = 0;
        else
            singlepotion->cursed = obj->cursed;  /* odiluted left as-is */
        singlepotion->bknown = false;
        if (Blind) {
            singlepotion->dknown = false;
        } else {
            singlepotion->dknown = !Hallucination();
            if (mixture == POT_WATER && singlepotion->dknown)
                sprintf(newbuf, "clears");
            else
                sprintf(newbuf, "turns %s",
                        hcolor(OBJ_DESCR(objects[mixture])));
            pline_The("%spotion%s %s.", oldbuf,
                    more_than_one ? " that you dipped into" : "",
                    newbuf);
            if(!objects[old_otyp].oc_uname &&
                    !objects[old_otyp].oc_name_known && old_dknown) {
                struct obj fakeobj;
                fakeobj = zeroobj;
                fakeobj.dknown = 1;
                fakeobj.otyp = old_otyp;
                fakeobj.oclass = POTION_CLASS;
                docall(&fakeobj);
            }
        }
        obj_extract_self(singlepotion);
        singlepotion = hold_another_object(singlepotion,
                "You juggle and drop %s!",
                doname(singlepotion), (const char *)0);
        return(1);
    }

    pline("Interesting...");
    return(1);
}

void djinni_from_bottle (struct obj *obj) {
    struct monst *mtmp;
    int chance;

    if(!(mtmp = makemon(&mons[PM_DJINNI], u.ux, u.uy, NO_MM_FLAGS))){
        pline("It turns out to be empty.");
        return;
    }

    if (Blind) {
        message_const(MSG_DJINNI_EMERGES_BLIND);
    } else {
        message_monster(MSG_DJINNI_EMERGES, mtmp);
    }

    chance = rn2(5);
    if (obj->blessed) chance = (chance == 4) ? rnd(4) : 0;
    else if (obj->cursed) chance = (chance == 0) ? rn2(4) : 4;
    /* 0,1,2,3,4:  b=80%,5,5,5,5; nc=20%,20,20,20,20; c=5%,5,5,5,80 */

    switch (chance) {
        case 0 : verbalize("I am in your debt.  I will grant one wish!");
                 makewish();
                 mongone(mtmp);
                 break;
        case 1 : verbalize("Thank you for freeing me!");
                 (void) tamedog(mtmp, (struct obj *)0);
                 break;
        case 2 : verbalize("You freed me!");
                 mtmp->mpeaceful = true;
                 set_malign(mtmp);
                 break;
        case 3 : verbalize("It is about time!");
                 message_monster(MSG_M_VANISHES, mtmp);
                 mongone(mtmp);
                 break;
        default: verbalize("You disturbed me, fool!");
                 break;
    }
}

/* clone a gremlin or mold (2nd arg non-null implies heat as the trigger);
   hit points are cut in half (odd HP stays with original) */
// struct monst *mon,  /* monster being split */
// struct monst *mtmp  /* optional attacker whose heat triggered it */
struct monst * split_mon ( struct monst *mon, struct monst *mtmp) {
    struct monst *mtmp2;
    char reason[BUFSZ];

    reason[0] = '\0';
    if (mtmp) {
        char pname[BUFSZ];
        monster_possessive(pname, BUFSZ, mtmp);
        sprintf(reason, " from %s heat", (mtmp == &youmonst) ? "your" : pname);
    }

    if (mon == &youmonst) {
        mtmp2 = cloneu();
        if (mtmp2) {
            mtmp2->mhpmax = u.mhmax / 2;
            u.mhmax -= mtmp2->mhpmax;
            You("multiply%s!", reason);
        }
    } else {
        mtmp2 = clone_mon(mon, 0, 0);
        if (mtmp2) {
            mtmp2->mhpmax = mon->mhpmax / 2;
            mon->mhpmax -= mtmp2->mhpmax;
            if (canspotmon(mon))
                message_monster(MSG_M_MULTIPLIES, mon);
        }
    }
    return mtmp2;
}
