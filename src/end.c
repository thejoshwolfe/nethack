/* See LICENSE in the root of this project for change info */

#include "end.h"
#include "hack.h"
#include "eshk.h"
#include "dlb.h"
#include "pm_props.h"
#include "invent.h"
#include "priest.h"
#include "objnam.h"
#include "shk.h"
#include "do_name.h"
#include "display.h"
#include "pline.h"
#include "everything.h"

#include <stdarg.h>

        /* these probably ought to be generated by makedefs, like LAST_GEM */
#define FIRST_GEM    DILITHIUM_CRYSTAL
#define FIRST_AMULET AMULET_OF_ESP
#define LAST_AMULET  AMULET_OF_YENDOR

struct valuable_data { long count; int typ; };

static struct valuable_data
        gems[LAST_GEM+1 - FIRST_GEM + 1], /* 1 extra for glass */
        amulets[LAST_AMULET+1 - FIRST_AMULET];

static struct val_list { struct valuable_data *list; int size; } valuables[] = {
        { gems,    sizeof gems / sizeof *gems },
        { amulets, sizeof amulets / sizeof *amulets },
        { 0, 0 }
};

extern void dump_spells(void);
void do_vanquished(int, bool, bool);

#define nethack_exit exit

#define done_stopprint program_state.stopprint

#define NH_abort()      abort()

/*
 * The order of these needs to match the macros in hack.h.
 */
static const char *deaths[] = {         /* the array of death */
        "died", "choked", "poisoned", "starvation", "drowning",
        "burning", "dissolving under the heat and pressure",
        "crushed", "turned to stone", "turned into slime",
        "genocided", "panic", "trickery",
        "quit", "escaped", "ascended"
};

static const char *ends[] = {           /* "when you..." */
        "died", "choked", "were poisoned", "starved", "drowned",
        "burned", "dissolved in the lava",
        "were crushed", "turned to stone", "turned into slime",
        "were genocided", "panicked", "were tricked",
        "quit", "escaped", "ascended"
};

extern const char * const killed_by_prefix[];   /* from topten.c */

FILE *dump_fp = NULL;  /* file pointer for dumps */
/* functions dump_init, dump_exit and dump are from the dump patch */


static void regularize(char * s) {
    char *lp;

    while((lp=index(s, '.')) || (lp=index(s, '/')) || (lp=index(s,' ')))
        *lp = '_';
}

void dump_init (void) {
  if (dump_fn[0]) {
    char *p = (char *) strstr(dump_fn, "%n");
    if (p) {
      int new_dump_fn_len = strlen(dump_fn)+strlen(plname)-2; /* %n */
      char *new_dump_fn = (char *) malloc((unsigned)(new_dump_fn_len+1));
      char *q = new_dump_fn;
      strncpy(q, dump_fn, p-dump_fn);
      q += p-dump_fn;
      strncpy(q, plname, strlen(plname) + 1);
      regularize(q);
      q[strlen(plname)] = '\0';
      q += strlen(q);
      p += 2;   /* skip "%n" */
      strncpy(q, p, strlen(p));
      new_dump_fn[new_dump_fn_len] = '\0';

      dump_fp = fopen(new_dump_fn, "w");
      if (!dump_fp) {
        pline("Can't open %s for output.", new_dump_fn);
        pline("Dump file not created.");
      }
      free(new_dump_fn);

    } else {
      dump_fp = fopen (dump_fn, "w");

      if (!dump_fp) {
        pline("Can't open %s for output.", dump_fn);
        pline("Dump file not created.");
      }
    }
  }
}

void dump_exit (void) {
    if (dump_fp)
        fclose (dump_fp);
}

void dump (char *pre, char *str) {
    if (dump_fp)
        fprintf (dump_fp, "%s%s\n", pre, str);
}

/* called as signal() handler, so sent at least one arg */
void done1 (int sig_unused) {
    signal(SIGINT,SIG_IGN);
    if(flags.ignintr) {
        signal(SIGINT, done1);
        clear_nhwindow(WIN_MESSAGE);
        curs_on_u();
        if(multi > 0) nomul(0);
    } else {
        done2();
    }
}


/* "#quit" command or keyboard interrupt */
int done2 (void) {
    if(yn("Really quit?") == 'n') {
        (void) signal(SIGINT, done1);
        clear_nhwindow(WIN_MESSAGE);
        curs_on_u();
        if(multi > 0) nomul(0);
        if(multi == 0) {
            u.uinvulnerable = false;    /* avoid ctrl-C bug -dlc */
            u.usleep = 0;
        }
        return 0;
    }
    if(wizard) {
        int c;
        const char *tmp = "Dump core?";
        if ((c = ynq(tmp)) == 'y') {
            signal(SIGINT, done1);
            NH_abort();
        } else if (c == 'q') done_stopprint++;
    }
    done(QUIT);
    return 0;
}

/* called as signal() handler, so sent at least one arg */
static void done_intr ( int sig_unused) {
    done_stopprint++;
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    return;
}

// signal() handler
static void done_hangup(int sig) {
    program_state.done_hup++;
    signal(SIGHUP, SIG_IGN);
    done_intr(sig);
    return;
}

void done_in_by (struct monst *mtmp) {
    char buf[BUFSZ];
    bool distorted = (bool)(Hallucination() && canspotmon(mtmp));

    You("die...");
    buf[0] = '\0';
    /* "killed by the high priest of Crom" is okay, "killed by the high
       priest" alone isn't */
    if ((mtmp->data->geno & G_UNIQ) != 0 && !(mtmp->data == &mons[PM_HIGH_PRIEST] && !mtmp->ispriest)) {
        if (!type_is_pname(mtmp->data))
            strcat(buf, "the ");
    }
    /* _the_ <invisible> <distorted> ghost of Dudley */
    if (mtmp->data == &mons[PM_GHOST] && mtmp->mnamelth) {
        strcat(buf, "the ");
    }
    if (mtmp->minvis)
        strcat(buf, "invisible ");
    if (distorted)
        strcat(buf, "hallucinogen-distorted ");

    if(mtmp->data == &mons[PM_GHOST]) {
        strcat(buf, "ghost");
        if (mtmp->mnamelth) sprintf(eos(buf), " of %s", monster_name(mtmp));
    } else if(mtmp->isshk) {
        sprintf(eos(buf), "%s %s, the shopkeeper",
                (mtmp->female ? "Ms." : "Mr."), shkname(mtmp));
    } else if (mtmp->ispriest || mtmp->isminion) {
        /* m_monnam() suppresses "the" prefix plus "invisible", and
           it overrides the effect of Hallucination() on priestname() */
    } else {
        strcat(buf, mtmp->data->mname);
        if (mtmp->mnamelth)
            sprintf(eos(buf), " called %s", monster_name(mtmp));
    }

    if (multi)
        strcat(buf, ", while helpless");
    killer = killed_by_monster(KM_MONSTER, mtmp);
    if (mtmp->data->mlet == S_WRAITH)
        u.ugrave_arise = PM_WRAITH;
    else if (mtmp->data->mlet == S_MUMMY && urace.mummynum != NON_PM)
        u.ugrave_arise = urace.mummynum;
    else if (mtmp->data->mlet == S_VAMPIRE && Race_if(PM_HUMAN))
        u.ugrave_arise = PM_VAMPIRE;
    else if (mtmp->data == &mons[PM_GHOUL])
        u.ugrave_arise = PM_GHOUL;
    if (u.ugrave_arise >= LOW_PM &&
            (mvitals[u.ugrave_arise].mvflags & G_GENOD))
        u.ugrave_arise = NON_PM;
    if (touch_petrifies(mtmp->data))
        done(STONING);
    else
        done(DIED);
    return;
}

void panic (const char * str, ...) {
    va_list the_args;
    va_start(the_args, str);

    if (program_state.panicking++)
        NH_abort(); /* avoid loops - this should never happen*/

    if (iflags.window_inited) {
        fprintf(stderr, "Oops...\n");
        iflags.window_inited = 0; /* they're gone; force raw_print()ing */
    }

    fprintf(stderr, program_state.gameover ?
            "Postgame wrapup disrupted." :
            !program_state.something_worth_saving ?
            "Program initialization has failed." :
            "Suddenly, the dungeon collapses.");
    if (program_state.something_worth_saving) {
        set_error_savefile();
        dosave0();
    }
    vfprintf(stderr, str, the_args);
    if (wizard)
        NH_abort(); /* generate core dump */
    va_end(the_args);
    done(PANICKED);
}

static bool should_query_disclose_option (int category, char *defquery) {
    int idx;
    char *dop = index(disclosure_options, category);

    if (dop && defquery) {
        idx = dop - disclosure_options;
        if (idx < 0 || idx > (NUM_DISCLOSURE_OPTIONS - 1)) {
            impossible(
                   "should_query_disclose_option: bad disclosure index %d %c",
                       idx, category);
            *defquery = DISCLOSE_PROMPT_DEFAULT_YES;
            return true;
        }
        if (flags.end_disclose[idx] == DISCLOSE_YES_WITHOUT_PROMPT) {
            *defquery = 'y';
            return false;
        } else if (flags.end_disclose[idx] == DISCLOSE_NO_WITHOUT_PROMPT) {
            *defquery = 'n';
            return false;
        } else if (flags.end_disclose[idx] == DISCLOSE_PROMPT_DEFAULT_YES) {
            *defquery = 'y';
            return true;
        } else if (flags.end_disclose[idx] == DISCLOSE_PROMPT_DEFAULT_NO) {
            *defquery = 'n';
            return true;
        }
    }
    if (defquery)
        impossible("should_query_disclose_option: bad category %c", category);
    else
        impossible("should_query_disclose_option: null defquery");
    return true;
}

static void list_genocided (int defquery, bool ask, bool want_dump) {
}

static void disclose (int how, bool taken) {
        char    c = 0, defquery;
        char    qbuf[QBUFSZ];
        bool ask;

        if (invent) {
            if(taken)
                sprintf(qbuf,"Do you want to see what you had when you %s?",
                        (how == QUIT) ? "quit" : "died");
            else
                strcpy(qbuf,"Do you want your possessions identified?");

            ask = should_query_disclose_option('i', &defquery);
            if (!done_stopprint) {
                c = ask ? yn_function(qbuf, ynqchars, defquery) : defquery;
                if (c == 'y') {
                        struct obj *obj;

                        for (obj = invent; obj; obj = obj->nobj) {
                            makeknown(obj->otyp);
                            obj->known = obj->bknown = obj->dknown = obj->rknown = 1;
                        }
                        (void) dump_inventory((char *)0, true);
                        do_containerconts(invent, true, true, true);
                }
                if (c == 'q')  done_stopprint++;
            }
        }

        ask = should_query_disclose_option('a', &defquery);
        if (!done_stopprint) {
            c = ask ? yn_function("Do you want to see your attributes?",
                                  ynqchars, defquery) : defquery;
            if (c == 'y')
                enlightenment(how >= PANICKED ? 1 : 2); /* final */
            if (c == 'q') done_stopprint++;
        }
        if (dump_fp) {
          dump_enlightenment((int) (how >= PANICKED ? 1 : 2));
          dump_spells();
        }

        ask = should_query_disclose_option('v', &defquery);
        if (!done_stopprint)
            do_vanquished(defquery, ask, true);

        ask = should_query_disclose_option('g', &defquery);
        if (!done_stopprint)
            list_genocided(defquery, ask,true);

        ask = should_query_disclose_option('c', &defquery);
        if (!done_stopprint) {
            c = ask ? yn_function("Do you want to see your conduct?",
                                  ynqchars, defquery) : defquery;
            if (c == 'y')
                show_conduct(how >= PANICKED ? 1 : 2);
            if (c == 'q') done_stopprint++;
        }
        if (dump_fp) {
            dump_conduct(how >= PANICKED ? 1 : 2);
            dump_weapon_skill();
        }
}

/* try to get the player back in a viable state after being killed */
static void savelife (int how) {
        u.uswldtim = 0;
        u.uhp = u.uhpmax;
        if (u.uhunger < 500) {
            u.uhunger = 500;
            newuhs(false);
        }
        /* cure impending doom of sickness hero won't have time to fix */
        if ((Sick & TIMEOUT) == 1) {
            u.usick_type = 0;
            Sick = 0;
        }
        if (how == CHOKING) init_uhunger();
        nomovemsg = "You survived that attempt on your life.";
        flags.move = 0;
        if(multi > 0) multi = 0; else multi = -1;
        if(u.utrap && u.utraptype == TT_LAVA) u.utrap = 0;
        flags.botl = 1;
        u.ugrave_arise = NON_PM;
        HUnchanging = 0L;
        curs_on_u();
}

/*
 * Get valuables from the given list.  Revised code: the list always remains
 * intact.
 */
/* inventory or container contents */
static void get_valuables ( struct obj *list ) {
    struct obj *obj;
    int i;

    /* find amulets and gems, ignoring all artifacts */
    for (obj = list; obj; obj = obj->nobj)
        if (Has_contents(obj)) {
            get_valuables(obj->cobj);
        } else if (obj->oartifact) {
            continue;
        } else if (obj->oclass == AMULET_CLASS) {
            i = obj->otyp - FIRST_AMULET;
            if (!amulets[i].count) {
                amulets[i].count = obj->quan;
                amulets[i].typ = obj->otyp;
            } else amulets[i].count += obj->quan; /* always adds one */
        } else if (obj->oclass == GEM_CLASS && obj->otyp < LUCKSTONE) {
            i = min(obj->otyp, LAST_GEM + 1) - FIRST_GEM;
            if (!gems[i].count) {
                gems[i].count = obj->quan;
                gems[i].typ = obj->otyp;
            } else gems[i].count += obj->quan;
        }
    return;
}

/*
 *  Sort collected valuables, most frequent to least.  We could just
 *  as easily use qsort, but we don't care about efficiency here.
 */
/* max value is less than 20 */
static void sort_valuables ( struct valuable_data list[], int size) {
    int i, j;
    struct valuable_data ltmp;

    /* move greater quantities to the front of the list */
    for (i = 1; i < size; i++) {
        if (list[i].count == 0) continue;       /* empty slot */
        ltmp = list[i]; /* structure copy */
        for (j = i; j > 0; --j)
            if (list[j-1].count >= ltmp.count) break;
            else {
                list[j] = list[j-1];
            }
        list[j] = ltmp;
    }
    return;
}

/* called twice; first to calculate total, then to list relevant items */
// bool counting,       /* true => add up points; false => display them */
static void artifact_score ( struct obj *list, bool counting, winid endwin) {
}

static const char * render_killer(const struct Killer *k) {
    return "killed by TODO";
}

/* Be careful not to call panic from here! */
void done (int how) {
    bool taken;
    char kilbuf[BUFSZ], pbuf[BUFSZ];
    winid endwin = WIN_ERR;
    bool bones_ok, have_windows = iflags.window_inited;
    struct obj *corpse = (struct obj *)0;
    long umoney;

    if (how == TRICKED) {
        killer.method = KM_DIED;
        if (wizard) {
            You("are a very tricky wizard, it seems.");
            return;
        }
    }

    /* kilbuf: used to copy killer in case it comes from something like
     *      xname(), which would otherwise get overwritten when we call
     *      xname() when listing possessions
     * pbuf: holds sprintf'd output for raw_print and putstr
     */
    /* Avoid killed by "a" burning or "a" starvation */
    if (killer.method == KM_DIED)
        killer.method = how;

    if (how < KM_PANICKED) u.umortality++;
    if (Lifesaved && (how <= GENOCIDED)) {
        pline("But wait...");
        makeknown(AMULET_OF_LIFE_SAVING);
        Your("medallion %s!",
                !Blind ? "begins to glow" : "feels warm");
        if (how == CHOKING) You("vomit ...");
        You_feel("much better!");
        pline_The("medallion crumbles to dust!");
        if (uamul) useup(uamul);

        (void) adjattrib(A_CON, -1, true);
        if(u.uhpmax <= 0) u.uhpmax = 10;        /* arbitrary */
        savelife(how);
        if (how == GENOCIDED)
            pline("Unfortunately you are still genocided...");
        else {
            killer.method = KM_DIED;
            return;
        }
    }
    if (( wizard || discover) && (how <= GENOCIDED)) {
        if(yn("Die?") == 'y') goto die;
        pline("OK, so you don't %s.", (how == CHOKING) ? "choke" : "die");
        if(u.uhpmax <= 0) u.uhpmax = u.ulevel * 8;      /* arbitrary */
        savelife(how);
        killer.method = KM_DIED;
        return;
    }

    /*
     *  The game is now over...
     */

die:
    program_state.gameover = 1;
    /* in case of a subsequent panic(), there's no point trying to save */
    program_state.something_worth_saving = 0;
    /* D: Grab screen dump right here */
    if (dump_fn[0]) {
        dump_init();
        sprintf(pbuf, "%s, %s %s %s %s", plname,
                aligns[1 - u.ualign.type].adj,
                genders[flags.female].adj,
                urace.adj,
                (flags.female && urole.name.f)?
                urole.name.f : urole.name.m);
        dump("", pbuf);
        /* D: Add a line for clearance from the screen dump */
        dump("", "");
        dump_screen();
    }
    /* render vision subsystem inoperative */
    iflags.vision_inited = 0;
    /* might have been killed while using a disposable item, so make sure
       it's gone prior to inventory disclosure and creation of bones data */
    inven_inuse(true);

    /* Sometimes you die on the first move.  Life's not fair.
     * On those rare occasions you get hosed immediately, go out
     * smiling... :-)  -3.
     */
    if (moves <= 1 && how < PANICKED)       /* You die... --More-- */
        pline("Do not pass go.  Do not collect 200 %s.", currency(200L));

    bones_ok = (how < GENOCIDED) && can_make_bones();

    if (how == TURNED_SLIME)
        u.ugrave_arise = PM_GREEN_SLIME;

    if (bones_ok && u.ugrave_arise < LOW_PM) {
        /* corpse gets burnt up too */
        if (how == BURNING)
            u.ugrave_arise = (NON_PM - 2);  /* leave no corpse */
        else if (how == STONING)
            u.ugrave_arise = (NON_PM - 1);  /* statue instead of corpse */
        else if (u.ugrave_arise == NON_PM &&
                !(mvitals[u.umonnum].mvflags & G_NOCORPSE)) {
            int mnum = u.umonnum;

            if (!Upolyd) {
                /* Base corpse on race when not poly'd since original
                 * u.umonnum is based on role, and all role monsters
                 * are human.
                 */
                mnum = (flags.female && urace.femalenum != NON_PM) ?
                    urace.femalenum : urace.malenum;
            }
            corpse = mk_named_object(CORPSE, &mons[mnum],
                    u.ux, u.uy, plname);
            sprintf(pbuf, "%s, %s", plname, render_killer(&killer));
            make_grave(u.ux, u.uy, pbuf);
        }
    }

    if (how == QUIT) {
        if (u.uhp < 1) {
            how = DIED;
            u.umortality++; /* skipped above when how==QUIT */
            /* note that killer is pointing at kilbuf */
            strcpy(kilbuf, "quit while already on Charon's boat");
        }
    }

    if (how != PANICKED) {
        /* these affect score and/or bones, but avoid them during panic */
        taken = paybill((how == ESCAPED) ? -1 : (how != QUIT));
        paygd();
        clearpriests();
    } else  taken = false;  /* lint; assert( !bones_ok ); */

    clearlocks();

    if (have_windows) display_nhwindow(WIN_MESSAGE, false);

    if (strcmp(flags.end_disclose, "none") && how != PANICKED)
        disclose(how, taken);
    /* finish_paybill should be called after disclosure but before bones */
    if (bones_ok && taken) finish_paybill();

    /* calculate score, before creating bones [container gold] */
    {
        long tmp;
        int deepest = deepest_lev_reached(false);

        umoney = u.ugold;
        tmp = u.ugold0;
        umoney += hidden_gold();    /* accumulate gold from containers */
        tmp = umoney - tmp;         /* net gain */

        if (tmp < 0L)
            tmp = 0L;
        if (how < PANICKED)
            tmp -= tmp / 10L;
        u.urexp += tmp;
        u.urexp += 50L * (long)(deepest - 1);
        if (deepest > 20)
            u.urexp += 1000L * (long)((deepest > 30) ? 10 : deepest - 20);
        if (how == ASCENDED) u.urexp *= 2L;
    }

    if (bones_ok) {
        if (!wizard || yn("Save bones?") == 'y')
            savebones(corpse);
        /* corpse may be invalid pointer now so
           ensure that it isn't used again */
        corpse = (struct obj *)0;
    }

    /* update gold for the rip output, which can't use hidden_gold()
       (containers will be gone by then if bones just got saved...) */
    u.ugold = umoney;

    /* clean up unneeded windows */
    if (have_windows) {
        display_nhwindow(WIN_MESSAGE, true);
        destroy_nhwindow(WIN_MAP);
        destroy_nhwindow(WIN_STATUS);
        destroy_nhwindow(WIN_MESSAGE);
        WIN_MESSAGE = WIN_STATUS = WIN_MAP = WIN_ERR;

        if(!done_stopprint || flags.tombstone)
            endwin = create_nhwindow(NHW_TEXT);

    } else
        done_stopprint = 1; /* just avoid any more output */

    /* changing kilbuf really changes killer. we do it this way because
       killer is declared a (const char *)
       */
    if (u.uhave.amulet) strcat(kilbuf, " (with the Amulet)");
    else if (how == ESCAPED) {
        if (Is_astralevel(&u.uz))   /* offered Amulet to wrong deity */
            strcat(kilbuf, " (in celestial disgrace)");
        else if (carrying(FAKE_AMULET_OF_YENDOR))
            strcat(kilbuf, " (with a fake Amulet)");
        /* don't bother counting to see whether it should be plural */
    }

    sprintf(pbuf, "%s %s the %s...", Goodbye(), plname,
            how != ASCENDED ?
            (const char *) ((flags.female && urole.name.f) ?
                urole.name.f : urole.name.m) :
            (const char *) (flags.female ? "Demigoddess" : "Demigod"));
    if (!done_stopprint) {
        putstr(endwin, 0, pbuf);
        putstr(endwin, 0, "");
    }
    if (dump_fp) dump("", pbuf);

    if (how == ESCAPED || how == ASCENDED) {
        struct monst *mtmp;
        struct obj *otmp;
        struct val_list *val;
        int i;

        for (val = valuables; val->list; val++)
            for (i = 0; i < val->size; i++) {
                val->list[i].count = 0L;
            }
        get_valuables(invent);

        /* add points for collected valuables */
        for (val = valuables; val->list; val++)
            for (i = 0; i < val->size; i++)
                if (val->list[i].count != 0L)
                    u.urexp += val->list[i].count
                        * (long)objects[val->list[i].typ].oc_cost;

        /* count the points for artifacts */
        artifact_score(invent, true, endwin);

        keepdogs(true);
        viz_array[0][0] |= IN_SIGHT; /* need visibility for naming */
        mtmp = mydogs;
        strcpy(pbuf, "You");
        if (mtmp) {
            while (mtmp) {
                char name[BUFSZ];
                mon_nam(name, BUFSZ, mtmp);
                sprintf(eos(pbuf), " and %s", name);
                if (mtmp->mtame)
                    u.urexp += mtmp->mhp;
                mtmp = mtmp->nmon;
            }
            if (!done_stopprint) putstr(endwin, 0, pbuf);
            if (dump_fp) dump("", pbuf);
            pbuf[0] = '\0';
        } else {
            if (!done_stopprint) strcat(pbuf, " ");
        }
        sprintf(eos(pbuf), "%s with %ld point%s,",
                how==ASCENDED ? "went to your reward" :
                "escaped from the dungeon",
                u.urexp, plur(u.urexp));
        if (dump_fp) dump("", pbuf);
        if (!done_stopprint) {
            putstr(endwin, 0, pbuf);
        }

        if (!done_stopprint)
            artifact_score(invent, false, endwin);  /* list artifacts */

        /* list valuables here */
        for (val = valuables; val->list; val++) {
            sort_valuables(val->list, val->size);
            for (i = 0; i < val->size && !done_stopprint; i++) {
                int typ = val->list[i].typ;
                long count = val->list[i].count;

                if (count == 0L) continue;
                if (objects[typ].oc_class != GEM_CLASS || typ <= LAST_GEM) {
                    otmp = mksobj(typ, false, false);
                    makeknown(otmp->otyp);
                    otmp->known = 1;        /* for fake amulets */
                    otmp->dknown = 1;       /* seen it (blindness fix) */
                    otmp->onamelth = 0;
                    otmp->quan = count;
                    sprintf(pbuf, "%8ld %s (worth %ld %s),",
                            count, xname(otmp),
                            count * (long)objects[typ].oc_cost, currency(2L));
                    obfree(otmp, (struct obj *)0);
                } else {
                    sprintf(pbuf,
                            "%8ld worthless piece%s of colored glass,",
                            count, plur(count));
                }
                putstr(endwin, 0, pbuf);
                if (dump_fp) dump("", pbuf);
            }
        }

    } else if (!done_stopprint) {
        /* did not escape or ascend */
        if (u.uz.dnum == 0 && u.uz.dlevel <= 0) {
            /* level teleported out of the dungeon; `how' is DIED,
               due to falling or to "arriving at heaven prematurely" */
            sprintf(pbuf, "You %s beyond the confines of the dungeon",
                    (u.uz.dlevel < 0) ? "passed away" : ends[how]);
        } else {
            /* more conventional demise */
            const char *where = dungeons[u.uz.dnum].dname;

            if (Is_astralevel(&u.uz)) where = "The Astral Plane";
            sprintf(pbuf, "You %s in %s", ends[how], where);
            if (!In_endgame(&u.uz) && !Is_knox(&u.uz))
                sprintf(eos(pbuf), " on dungeon level %d",
                        In_quest(&u.uz) ? dunlev(&u.uz) : depth(&u.uz));
        }

        sprintf(eos(pbuf), " with %ld point%s,",
                u.urexp, plur(u.urexp));
        putstr(endwin, 0, pbuf);
        if (dump_fp) dump("", pbuf);
    }

    if (!done_stopprint) {
        sprintf(pbuf, "and %ld piece%s of gold, after %ld move%s.",
                umoney, plur(umoney), moves, plur(moves));
        putstr(endwin, 0, pbuf);
        if (dump_fp) {
            dump("", pbuf);
            sprintf(pbuf, "Killer: %s", render_killer(&killer));
            dump("", pbuf);
        }
    }
    if (!done_stopprint) {
        sprintf(pbuf,
                "You were level %d with a maximum of %d hit point%s when you %s.",
                u.ulevel, u.uhpmax, plur(u.uhpmax), ends[how]);
        putstr(endwin, 0, pbuf);
        putstr(endwin, 0, "");
        if (dump_fp) dump("", pbuf);
    }
    if (!done_stopprint)
        display_nhwindow(endwin, true);
    if (endwin != WIN_ERR)
        destroy_nhwindow(endwin);

    /* "So when I die, the first thing I will see in Heaven is a
     * score list?" */
    if (dump_fp) dump_exit();

    terminate(EXIT_SUCCESS);
}

void container_contents (struct obj *list, bool identified, bool all_containers) {
    do_containerconts(list, identified, all_containers, false);
}

/* The original container_contents function */
void do_containerconts (struct obj *list, bool identified, bool all_containers, bool want_dump) {
        struct obj *box, *obj;
        struct obj **oarray;
        int i,j,n;
        char *invlet;
        char buf[BUFSZ];

        for (box = list; box; box = box->nobj) {
            if (Is_container(box) || box->otyp == STATUE) {
                if (box->otyp == BAG_OF_TRICKS) {
                    continue;   /* wrong type of container */
                } else if (box->cobj) {
                    winid tmpwin = create_nhwindow(NHW_MENU);
                    /* count the number of items */
                    for (n = 0, obj = box->cobj; obj; obj = obj->nobj) n++;
                    /* Make a temporary array to store the objects sorted */
                    oarray = (struct obj **) malloc(n*sizeof(struct obj*));

                    /* Add objects to the array */
                    i = 0;
                    invlet = flags.inv_order;
                nextclass:
                    for (obj = box->cobj; obj; obj = obj->nobj) {
                      if (!flags.sortpack || obj->oclass == *invlet) {
                        if (iflags.sortloot == 'f'
                            || iflags.sortloot == 'l') {
                          /* Insert object at correct index */
                          for (j = i; j; j--) {
                            if (strcmpi(cxname2(obj), cxname2(oarray[j-1]))>0
                            || (flags.sortpack &&
                                oarray[j-1]->oclass != obj->oclass))
                              break;
                            oarray[j] = oarray[j-1];
                          }
                          oarray[j] = obj;
                          i++;
                        } else {
                          /* Just add it to the array */
                          oarray[i++] = obj;
                        }
                      }
                    } /* for loop */
                    if (flags.sortpack) {
                      if (*++invlet) goto nextclass;
                    }
                    sprintf(buf, "Contents of %s:", the(xname(box)));
                    putstr(tmpwin, 0, buf);
                    putstr(tmpwin, 0, "");
                    if (dump_fp) dump("", buf);
                    for (i = 0; i < n; i++) {
                        obj = oarray[i];
                        if (identified) {
                            makeknown(obj->otyp);
                            obj->known = obj->bknown =
                            obj->dknown = obj->rknown = 1;
                        }
                        putstr(tmpwin, 0, doname(obj));
                        if (want_dump)  dump("  ", doname(obj));
                    }
                    if (want_dump)  dump("","");
                    display_nhwindow(tmpwin, true);
                    destroy_nhwindow(tmpwin);
                    if (all_containers) {
                        do_containerconts(box->cobj, identified, true,
                                          want_dump);
                    }
                } else {
                    char are_clause[BUFSZ];
                    Tobjnam(are_clause, BUFSZ, box, "are");
                    pline("%s empty.", are_clause);
                    display_nhwindow(WIN_MESSAGE, false);
                    if (want_dump) {
                      dump(The(xname(box)), " is empty.");
                      dump("", "");
                    }
                }
            }
            if (!all_containers)
                break;
        }
}

/* should be called with either EXIT_SUCCESS or EXIT_FAILURE */
void terminate(int status) {
    /* don't bother to try to release memory if we're in panic mode, to
       avoid trouble in case that happens to be due to memory problems */
    if (!program_state.panicking) {
        freedynamicdata();
        dlb_cleanup();
    }

    nethack_exit(status);
}

void do_vanquished (int defquery, bool ask, bool want_dump) {
    int i, lev;
    int ntypes = 0, max_lev = 0, nkilled;
    long total_killed = 0L;
    char c;
    winid klwin;
    char buf[BUFSZ];

    /* get totals first */
    for (i = LOW_PM; i < NUMMONS; i++) {
        if (mvitals[i].died) ntypes++;
        total_killed += (long)mvitals[i].died;
        if (mons[i].mlevel > max_lev) max_lev = mons[i].mlevel;
    }

    /* vanquished creatures list;
     * includes all dead monsters, not just those killed by the player
     */
    if (ntypes != 0) {
        c = ask ? yn_function("Do you want an account of creatures vanquished?",
                              ynqchars, defquery) : defquery;
        if (c == 'q') done_stopprint++;
        if (c == 'y') {
            klwin = create_nhwindow(NHW_MENU);
            putstr(klwin, 0, "Vanquished creatures:");
            putstr(klwin, 0, "");
            if (want_dump)  dump("", "Vanquished creatures");

            /* countdown by monster "toughness" */
            for (lev = max_lev; lev >= 0; lev--)
              for (i = LOW_PM; i < NUMMONS; i++)
                if (mons[i].mlevel == lev && (nkilled = mvitals[i].died) > 0) {
                    if ((mons[i].geno & G_UNIQ) && i != PM_HIGH_PRIEST) {
                        sprintf(buf, "%s%s",
                                !type_is_pname(&mons[i]) ? "The " : "",
                                mons[i].mname);
                        if (nkilled > 1) {
                            switch (nkilled) {
                                case 2:  sprintf(eos(buf)," (twice)");  break;
                                case 3:  sprintf(eos(buf)," (thrice)");  break;
                                default: sprintf(eos(buf)," (%d time%s)",
                                                 nkilled, plur(nkilled));
                                         break;
                            }
                        }
                    } else {
                        /* trolls or undead might have come back,
                           but we don't keep track of that */
                        if (nkilled == 1)
                            strcpy(buf, an(mons[i].mname));
                        else
                            sprintf(buf, "%d %s",
                                    nkilled, makeplural(mons[i].mname));
                    }
                    putstr(klwin, 0, buf);
                    if (want_dump)  dump("  ", buf);
                }
            /*
             * if (Hallucination())
             *     putstr(klwin, 0, "and a partridge in a pear tree");
             */
            if (ntypes > 1) {
                putstr(klwin, 0, "");
                sprintf(buf, "%ld creatures vanquished.", total_killed);
                putstr(klwin, 0, buf);
                if (want_dump)  dump("  ", buf);
            }
            display_nhwindow(klwin, true);
            destroy_nhwindow(klwin);
            if (want_dump)  dump("", "");
        }
    }
}

/* number of monster species which have been genocided */
int num_genocides (void) {
    int i, n = 0;

    for (i = LOW_PM; i < NUMMONS; ++i)
        if (mvitals[i].mvflags & G_GENOD) ++n;

    return n;
}


