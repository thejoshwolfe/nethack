/* See LICENSE in the root of this project for change info */

#include "hack.h"
#include "func_tab.h"
#include "pm_props.h"
#include "extern.h"
#include "hacklib.h"
#include "objnam.h"
#include "do_name.h"
#include "winprocs.h"
#include "display.h"
#include "vision.h"

// ^A, the "redo" key
static const char DOAGAIN = '\001';

/*
 * Some systems may have getchar() return EOF for various reasons, and
 * we should not quit before seeing at least NR_OF_EOFS consecutive EOFs.
 */
static const int NR_OF_EOFS = 20;

#define CMD_TRAVEL (char)0x90


static int (*timed_occ_fn)(void);

static int doprev_message(void);
static int timed_occupation(void);
static int doextcmd(void);
static int domonability(void);
static int dotravel(void);
static int wiz_wish(void);
static int wiz_identify(void);
static int wiz_map(void);
static int wiz_genesis(void);
static int wiz_where(void);
static int wiz_detect(void);
static int wiz_panic(void);
static int wiz_polyself(void);
static int wiz_level_tele(void);
static int wiz_level_change(void);
static int wiz_show_seenv(void);
static int wiz_show_vision(void);
static int wiz_mon_polycontrol(void);
static int wiz_show_wmodes(void);
static void count_obj(struct obj *, long *, long *, bool, bool);
static void obj_chain(winid, const char *, struct obj *, long *, long *);
static void mon_invent_chain(winid, const char *, struct monst *, long *, long *);
static void mon_chain(winid, const char *, struct monst *, long *, long *);
static void contained(winid, const char *, long *, long *);
static int wiz_show_stats(void);
static int enter_explore_mode(void);
static int doattributes(void);
static int doconduct(void); /**/
static bool minimal_enlightenment(void);

static void enlght_line(const char *,const char *,const char *);
static char *enlght_combatinc(const char *,int,int,char *);
static void end_of_input(void);

static const char* readchar_queue="";

static char *parse(void);
static bool help_dir(char,const char *);

/* -enlightenment and conduct- */
static winid en_win;
static const char
You_[] = "You ",
    are[]  = "are ",  were[]  = "were ",
    have[] = "have ", had[]   = "had ",
    can[]  = "can ",  could[] = "could ";
static const char
have_been[]  = "have been ",
    have_never[] = "have never ", never[] = "never ";

#define enl_msg(prefix,present,past,suffix) \
    enlght_line(prefix, final ? past : present, suffix)
#define you_are(attr)   enl_msg(You_,are,were,attr)
#define you_have(attr)  enl_msg(You_,have,had,attr)
#define you_can(attr)   enl_msg(You_,can,could,attr)
#define you_have_been(goodthing) enl_msg(You_,have_been,were,goodthing)
#define you_have_never(badthing) enl_msg(You_,have_never,never,badthing)
#define you_have_X(something)   enl_msg(You_,have,(const char *)"",something)

static char popch(void);

/* Provide a means to redo the last command.  The flag `in_doagain' is set
 * to true while redoing the command.  This flag is tested in commands that
 * require additional input (like `throw' which requires a thing and a
 * direction), and the input prompt is not shown.  Also, while in_doagain is
 * true, no keystrokes can be saved into the saveq.
 */
#define BSIZE 20
static char pushq[BSIZE], saveq[BSIZE];
static int phead, ptail, shead, stail;

#define MAX_EXT_CMD 40          /* Change if we ever have > 40 ext cmds */

static int doprev_message (void) {
    return nh_doprev_message();
}

/* Count down by decrementing multi */
static int timed_occupation (void) {
    (*timed_occ_fn)();
    if (multi > 0)
        multi--;
    return multi > 0;
}

/* If you have moved since initially setting some occupations, they
 * now shouldn't be able to restart.
 *
 * The basic rule is that if you are carrying it, you can continue
 * since it is with you.  If you are acting on something at a distance,
 * your orientation to it must have changed when you moved.
 *
 * The exception to this is taking off items, since they can be taken
 * off in a number of ways in the intervening time, screwing up ordering.
 *
 *      Currently:      Take off all armor.
 *                      Picking Locks / Forcing Chests.
 *                      Setting traps.
 */
void reset_occupations (void) {
    reset_remarm();
    reset_pick();
    reset_trapset();
}

/* If a time is given, use it to timeout this function, otherwise the
 * function times out by its own means.
 */
void set_occupation (int (*fn)(void), const char *txt, int xtime) {
    if (xtime) {
        occupation = timed_occupation;
        timed_occ_fn = fn;
    } else
        occupation = fn;
    occtxt = txt;
    occtime = 0;
    return;
}

static char popch (void) {
    /* If occupied, return '\0', letting getchar know a character should
     * be read from the keyboard.  If the character read is not the
     * ABORT character (as checked in pcmain.c), that character will be
     * pushed back on the pushq.
     */
    if (occupation) return '\0';
    if (in_doagain) return(char)((shead != stail) ? saveq[stail++] : '\0');
    else            return(char)((phead != ptail) ? pushq[ptail++] : '\0');
}

char pgetchar (void) {               /* curtesy of aeb@cwi.nl */
    int ch;

    if(!(ch = popch()))
        ch = nhgetch();
    return((char)ch);
}

/* A ch == 0 resets the pushq */
void pushch (char ch) {
    if (!ch)
        phead = ptail = 0;
    if (phead < BSIZE)
        pushq[phead++] = ch;
    return;
}

/* A ch == 0 resets the saveq.  Only save keystrokes when not
 * replaying a previous command.
 */
void savech (char ch) {
    if (!in_doagain) {
        if (!ch)
            phead = ptail = shead = stail = 0;
        else if (shead < BSIZE)
            saveq[shead++] = ch;
    }
    return;
}

/* here after # - now read a full-word command */
static int doextcmd (void) {
    int idx, retval;

    /* keep repeating until we don't run help or quit */
    do {
        idx = get_ext_cmd();
        if (idx < 0) return 0;      /* quit */

        retval = (*extcmdlist[idx].ef_funct)();
    } while (extcmdlist[idx].ef_funct == doextlist);

    return retval;
}

/* here after #? - now list all full-word commands */
int doextlist (void) {
    const struct ext_func_tab *efp;
    char     buf[BUFSZ];
    winid datawin;

    datawin = create_nhwindow(NHW_TEXT);
    putstr(datawin, 0, "");
    putstr(datawin, 0, "            Extended Commands List");
    putstr(datawin, 0, "");
    putstr(datawin, 0, "    Press '#', then type:");
    putstr(datawin, 0, "");

    for(efp = extcmdlist; efp->ef_txt; efp++) {
        sprintf(buf, "    %-15s - %s.", efp->ef_txt, efp->ef_desc);
        putstr(datawin, 0, buf);
    }
    display_nhwindow(datawin, false);
    destroy_nhwindow(datawin);
    return 0;
}

/*
 * This is currently used only by the tty port and is
 * controlled via runtime option 'extmenu'
 */
/* here after # - now show pick-list of possible commands */
int extcmd_via_menu (void) {
    const struct ext_func_tab *efp;
    menu_item *pick_list = (menu_item *)0;
    winid win;
    anything any;
    const struct ext_func_tab *choices[MAX_EXT_CMD];
    char buf[BUFSZ];
    char cbuf[QBUFSZ], prompt[QBUFSZ], fmtstr[20];
    int i, n, nchoices, acount;
    int ret,  biggest;
    int accelerator, prevaccelerator;
    int  matchlevel = 0;

    ret = 0;
    cbuf[0] = '\0';
    biggest = 0;
    while (!ret) {
        i = n = 0;
        accelerator = 0;
        any.a_void = 0;
        /* populate choices */
        for(efp = extcmdlist; efp->ef_txt; efp++) {
            if (!matchlevel || !strncmp(efp->ef_txt, cbuf, matchlevel)) {
                choices[i++] = efp;
                if ((int)strlen(efp->ef_desc) > biggest) {
                    biggest = strlen(efp->ef_desc);
                    sprintf(fmtstr,"%%-%ds", biggest + 15);
                }
            }
        }
        choices[i] = (struct ext_func_tab *)0;
        nchoices = i;
        /* if we're down to one, we have our selection so get out of here */
        if (nchoices == 1) {
            for (i = 0; extcmdlist[i].ef_txt != (char *)0; i++)
                if (!strncmpi(extcmdlist[i].ef_txt, cbuf, matchlevel)) {
                    ret = i;
                    break;
                }
            break;
        }

        /* otherwise... */
        win = create_nhwindow(NHW_MENU);
        start_menu(win);
        prevaccelerator = 0;
        acount = 0;
        for(i = 0; choices[i]; ++i) {
            accelerator = choices[i]->ef_txt[matchlevel];
            if (accelerator != prevaccelerator || nchoices < (ROWNO - 3)) {
                if (acount) {
                    /* flush the extended commands for that letter already in buf */
                    sprintf(buf, fmtstr, prompt);
                    any.a_char = prevaccelerator;
                    add_menu(win, NO_GLYPH, &any, any.a_char, 0,
                            ATR_NONE, buf, false);
                    acount = 0;
                }
            }
            prevaccelerator = accelerator;
            if (!acount || nchoices < (ROWNO - 3)) {
                sprintf(prompt, "%s [%s]", choices[i]->ef_txt,
                        choices[i]->ef_desc);
            } else if (acount == 1) {
                sprintf(prompt, "%s or %s", choices[i-1]->ef_txt,
                        choices[i]->ef_txt);
            } else {
                strcat(prompt," or ");
                strcat(prompt, choices[i]->ef_txt);
            }
            ++acount;
        }
        if (acount) {
            /* flush buf */
            sprintf(buf, fmtstr, prompt);
            any.a_char = prevaccelerator;
            add_menu(win, NO_GLYPH, &any, any.a_char, 0, ATR_NONE, buf, false);
        }
        sprintf(prompt, "Extended Command: %s", cbuf);
        end_menu(win, prompt);
        n = select_menu(win, PICK_ONE, &pick_list);
        destroy_nhwindow(win);
        if (n==1) {
            if (matchlevel > (QBUFSZ - 2)) {
                free((void *)pick_list);
                ret = -1;
            } else {
                cbuf[matchlevel++] = pick_list[0].item.a_char;
                cbuf[matchlevel] = '\0';
                free((void *)pick_list);
            }
        } else {
            if (matchlevel) {
                ret = 0;
                matchlevel = 0;
            } else
                ret = -1;
        }
    }
    return ret;
}

/* #monster command - use special monster ability while polymorphed */
static int domonability (void) {
    if (can_breathe(youmonst.data)) return dobreathe();
    else if (attacktype(youmonst.data, AT_SPIT)) return dospit();
    else if (youmonst.data->mlet == S_NYMPH) return doremove();
    else if (attacktype(youmonst.data, AT_GAZE)) return dogaze();
    else if (is_were(youmonst.data)) return dosummon();
    else if (webmaker(youmonst.data)) return dospinweb();
    else if (is_hider(youmonst.data)) return dohide();
    else if (is_mind_flayer(youmonst.data)) return domindblast();
    else if (u.umonnum == PM_GREMLIN) {
        if(IS_FOUNTAIN(levl[u.ux][u.uy].typ)) {
            if (split_mon(&youmonst, (struct monst *)0))
                dryup(u.ux, u.uy, true);
        } else There("is no fountain here.");
    } else if (is_unicorn(youmonst.data)) {
        use_unicorn_horn((struct obj *)0);
        return 1;
    } else if (youmonst.data->msound == MS_SHRIEK) {
        You("shriek.");
        if(u.uburied)
            pline("Unfortunately sound does not carry well through rock.");
        else aggravate();
    } else if (Upolyd)
        pline("Any special ability you may have is purely reflexive.");
    else You("don't have a special ability in your normal form!");
    return 0;
}

static int enter_explore_mode (void) {
    if(!discover && !wizard) {
        pline("Beware!  From explore mode there will be no return to normal game.");
        if (yn("Do you want to enter explore mode?") == 'y') {
            clear_nhwindow(WIN_MESSAGE);
            You("are now in non-scoring explore mode.");
            discover = true;
        }
        else {
            clear_nhwindow(WIN_MESSAGE);
            pline("Resuming normal game.");
        }
    }
    return 0;
}


/* ^W command - wish for something */
/* Unlimited wishes for debug mode by Paul Polderman */
static int wiz_wish (void) {
    if (wizard) {
        bool save_verbose = flags.verbose;

        flags.verbose = false;
        makewish();
        flags.verbose = save_verbose;
        (void) encumber_msg();
    } else
        pline("Unavailable command '^W'.");
    return 0;
}

/* ^I command - identify hero's inventory */
static int wiz_identify (void) {
    if (wizard)     identify_pack(0);
    else            pline("Unavailable command '^I'.");
    return 0;
}

/* ^F command - reveal the level map and any traps on it */
static int wiz_map (void) {
    if (wizard) {
        struct trap *t;
        long save_Hconf = HConfusion,
             save_Hhallu = u.uprops[HALLUC].intrinsic;

        HConfusion = u.uprops[HALLUC].intrinsic = 0L;
        for (t = ftrap; t != 0; t = t->ntrap) {
            t->tseen = 1;
            map_trap(t, true);
        }
        do_mapping();
        HConfusion = save_Hconf;
        u.uprops[HALLUC].intrinsic = save_Hhallu;
    } else
        pline("Unavailable command '^F'.");
    return 0;
}

/* ^G command - generate monster(s); a count prefix will be honored */
static int wiz_genesis (void) {
    if (wizard)     (void) create_particular();
    else            pline("Unavailable command '^G'.");
    return 0;
}

/* ^O command - display dungeon layout */
static int wiz_where (void) {
    if (wizard) (void) print_dungeon(false, (signed char *)0, (signed char *)0);
    else        pline("Unavailable command '^O'.");
    return 0;
}

/* ^E command - detect unseen (secret doors, traps, hidden monsters) */
static int wiz_detect (void) {
    if(wizard)  (void) findit();
    else        pline("Unavailable command '^E'.");
    return 0;
}

/* ^V command - level teleport */
static int wiz_level_tele (void) {
    if (wizard)     level_tele();
    else            pline("Unavailable command '^V'.");
    return 0;
}

/* #monpolycontrol command - choose new form for shapechangers, polymorphees */
static int wiz_mon_polycontrol (void) {
    iflags.mon_polycontrol = !iflags.mon_polycontrol;
    pline("Monster polymorph control is %s.",
            iflags.mon_polycontrol ? "on" : "off");
    return 0;
}

/* #levelchange command - adjust hero's experience level */
static int wiz_level_change (void) {
    char buf[BUFSZ];
    int newlevel;
    int ret;

    getlin("To what experience level do you want to be set?", buf);
    (void)mungspaces(buf);
    if (buf[0] == '\033' || buf[0] == '\0') ret = 0;
    else ret = sscanf(buf, "%d", &newlevel);

    if (ret != 1) {
        plines(Never_mind);
        return 0;
    }
    if (newlevel == u.ulevel) {
        You("are already that experienced.");
    } else if (newlevel < u.ulevel) {
        if (u.ulevel == 1) {
            You("are already as inexperienced as you can get.");
            return 0;
        }
        if (newlevel < 1) newlevel = 1;
        while (u.ulevel > newlevel)
            losexp("#levelchange");
    } else {
        if (u.ulevel >= MAXULEV) {
            You("are already as experienced as you can get.");
            return 0;
        }
        if (newlevel > MAXULEV) newlevel = MAXULEV;
        while (u.ulevel < newlevel)
            pluslvl(false);
    }
    u.ulevelmax = u.ulevel;
    return 0;
}

/* #panic command - test program's panic handling */
static int wiz_panic (void) {
    if (yn("Do you want to call panic() and end your game?") == 'y')
        panic("crash test.");
    return 0;
}

/* #polyself command - change hero's form */
static int wiz_polyself (void) {
    polyself(true);
    return 0;
}

/* #seenv command */
static int wiz_show_seenv (void) {
    winid win;
    int x, y, v, startx, stopx, curx;
    char row[COLNO+1];

    win = create_nhwindow(NHW_TEXT);
    /*
     * Each seenv description takes up 2 characters, so center
     * the seenv display around the hero.
     */
    startx = max(1, u.ux-(COLNO/4));
    stopx = min(startx+(COLNO/2), COLNO);
    /* can't have a line exactly 80 chars long */
    if (stopx - startx == COLNO/2) startx++;

    for (y = 0; y < ROWNO; y++) {
        for (x = startx, curx = 0; x < stopx; x++, curx += 2) {
            if (x == u.ux && y == u.uy) {
                row[curx] = row[curx+1] = '@';
            } else {
                v = levl[x][y].seenv & 0xff;
                if (v == 0)
                    row[curx] = row[curx+1] = ' ';
                else
                    sprintf(&row[curx], "%02x", v);
            }
        }
        /* remove trailing spaces */
        for (x = curx-1; x >= 0; x--)
            if (row[x] != ' ') break;
        row[x+1] = '\0';

        putstr(win, 0, row);
    }
    display_nhwindow(win, true);
    destroy_nhwindow(win);
    return 0;
}

/* #vision command */
static int wiz_show_vision (void) {
    winid win;
    int x, y, v;
    char row[COLNO+1];

    win = create_nhwindow(NHW_TEXT);
    sprintf(row, "Flags: 0x%x could see, 0x%x in sight, 0x%x temp lit",
            COULD_SEE, IN_SIGHT, TEMP_LIT);
    putstr(win, 0, row);
    putstr(win, 0, "");
    for (y = 0; y < ROWNO; y++) {
        for (x = 1; x < COLNO; x++) {
            if (x == u.ux && y == u.uy)
                row[x] = '@';
            else {
                v = viz_array[y][x]; /* data access should be hidden */
                if (v == 0)
                    row[x] = ' ';
                else
                    row[x] = '0' + viz_array[y][x];
            }
        }
        /* remove trailing spaces */
        for (x = COLNO-1; x >= 1; x--)
            if (row[x] != ' ') break;
        row[x+1] = '\0';

        putstr(win, 0, &row[1]);
    }
    display_nhwindow(win, true);
    destroy_nhwindow(win);
    return 0;
}

/* #wmode command */
static int wiz_show_wmodes (void) {
    winid win;
    int x,y;
    char row[COLNO+1];
    struct rm *lev;

    win = create_nhwindow(NHW_TEXT);
    for (y = 0; y < ROWNO; y++) {
        for (x = 0; x < COLNO; x++) {
            lev = &levl[x][y];
            if (x == u.ux && y == u.uy)
                row[x] = '@';
            else if (IS_WALL(lev->typ) || lev->typ == SDOOR)
                row[x] = '0' + (lev->wall_info & WM_MASK);
            else if (lev->typ == CORR)
                row[x] = '#';
            else if (IS_ROOM(lev->typ) || IS_DOOR(lev->typ))
                row[x] = '.';
            else
                row[x] = 'x';
        }
        row[COLNO] = '\0';
        putstr(win, 0, row);
    }
    display_nhwindow(win, true);
    destroy_nhwindow(win);
    return 0;
}

static void enlght_line (const char *start, const char *middle, const char *end) {
    char buf[BUFSZ];

    sprintf(buf, "%s%s%s.", start, middle, end);
    putstr(en_win, 0, buf);
}

/* format increased damage or chance to hit */
static char * enlght_combatinc (const char *inctyp, int incamt, int final, char *outbuf) {
    char numbuf[24];
    const char *modif, *bonus;

    if (final
            || wizard
       ) {
        sprintf(numbuf, "%s%d",
                (incamt > 0) ? "+" : "", incamt);
        modif = (const char *) numbuf;
    } else {
        int absamt = abs(incamt);

        if (absamt <= 3) modif = "small";
        else if (absamt <= 6) modif = "moderate";
        else if (absamt <= 12) modif = "large";
        else modif = "huge";
    }
    bonus = (incamt > 0) ? "bonus" : "penalty";
    /* "bonus to hit" vs "damage bonus" */
    if (!strcmp(inctyp, "damage")) {
        const char *ctmp = inctyp;
        inctyp = bonus;
        bonus = ctmp;
    }
    sprintf(outbuf, "%s %s %s", an(modif), bonus, inctyp);
    return outbuf;
}

/* 0 => still in progress; 1 => over, survived; 2 => dead */
void enlightenment (int final) {
    char buf[BUFSZ];

    en_win = create_nhwindow(NHW_MENU);
    putstr(en_win, 0, final ? "Final Attributes:" : "Current Attributes:");
    putstr(en_win, 0, "");

    if (u.uevent.uhand_of_elbereth) {
        static const char * const hofe_titles[3] = {
            "the Hand of Elbereth",
            "the Envoy of Balance",
            "the Glory of Arioch"
        };
        you_are(hofe_titles[u.uevent.uhand_of_elbereth - 1]);
    }

    /* note: piousness 20 matches MIN_QUEST_ALIGN (quest.h) */
    if (u.ualign.record >= 20)      you_are("piously aligned");
    else if (u.ualign.record > 13)  you_are("devoutly aligned");
    else if (u.ualign.record > 8)   you_are("fervently aligned");
    else if (u.ualign.record > 3)   you_are("stridently aligned");
    else if (u.ualign.record == 3)  you_are("aligned");
    else if (u.ualign.record > 0)   you_are("haltingly aligned");
    else if (u.ualign.record == 0)  you_are("nominally aligned");
    else if (u.ualign.record >= -3) you_have("strayed");
    else if (u.ualign.record >= -8) you_have("sinned");
    else you_have("transgressed");
    if (wizard) {
        sprintf(buf, " %d", u.ualign.record);
        enl_msg("Your alignment ", "is", "was", buf);
    }

    /*** Resistances to troubles ***/
    if (Fire_resistance) you_are("fire resistant");
    if (Cold_resistance) you_are("cold resistant");
    if (Sleep_resistance) you_are("sleep resistant");
    if (Disint_resistance) you_are("disintegration-resistant");
    if (Shock_resistance) you_are("shock resistant");
    if (Poison_resistance) you_are("poison resistant");
    if (Drain_resistance) you_are("level-drain resistant");
    if (Sick_resistance) you_are("immune to sickness");
    if (Antimagic) you_are("magic-protected");
    if (Acid_resistance) you_are("acid resistant");
    if (Stone_resistance)
        you_are("petrification resistant");
    if (Invulnerable) you_are("invulnerable");
    if (u.uedibility) you_can("recognize detrimental food");

    /*** Troubles ***/
    if (Halluc_resistance())
        enl_msg("You resist", "", "ed", " hallucinations");
    if (final) {
        if (Hallucination) you_are("hallucinating");
        if (Stunned) you_are("stunned");
        if (Confusion) you_are("confused");
        if (Blinded) you_are("blinded");
        if (Sick) {
            if (u.usick_type & SICK_VOMITABLE)
                you_are("sick from food poisoning");
            if (u.usick_type & SICK_NONVOMITABLE)
                you_are("sick from illness");
        }
    }
    if (Stoned) you_are("turning to stone");
    if (Slimed) you_are("turning into slime");
    if (Strangled) you_are((u.uburied) ? "buried" : "being strangled");
    if (Glib) {
        sprintf(buf, "slippery %s", makeplural(body_part(FINGER)));
        you_have(buf);
    }
    if (Fumbling) enl_msg("You fumble", "", "d", "");
    if (Wounded_legs
            && !u.usteed
       ) {
        sprintf(buf, "wounded %s", makeplural(body_part(LEG)));
        you_have(buf);
    }
    if (Wounded_legs && u.usteed && wizard) {
        x_monnam(buf, BUFSZ, u.usteed, ARTICLE_YOUR, NULL,
                SUPPRESS_SADDLE | SUPPRESS_HALLUCINATION, false);
        *buf = highc(*buf);
        enl_msg(buf, " has", " had", " wounded legs");
    }
    if (Sleeping) enl_msg("You ", "fall", "fell", " asleep");
    if (Hunger) enl_msg("You hunger", "", "ed", " rapidly");

    /*** Vision and senses ***/
    if (See_invisible) enl_msg(You_, "see", "saw", " invisible");
    if (Blind_telepat) you_are("telepathic");
    if (Warning) you_are("warned");
    if (Warn_of_mon && flags.warntype) {
        sprintf(buf, "aware of the presence of %s",
                (flags.warntype & M2_ORC) ? "orcs" :
                (flags.warntype & M2_DEMON) ? "demons" :
                something);
        you_are(buf);
    }
    if (Undead_warning) you_are("warned of undead");
    if (Searching) you_have("automatic searching");
    if (Clairvoyant) you_are("clairvoyant");
    if (Infravision) you_have("infravision");
    if (Detect_monsters) you_are("sensing the presence of monsters");
    if (u.umconf) you_are("going to confuse monsters");

    /*** Appearance and behavior ***/
    if (Adornment) {
        int adorn = 0;

        if(uleft && uleft->otyp == RIN_ADORNMENT) adorn += uleft->spe;
        if(uright && uright->otyp == RIN_ADORNMENT) adorn += uright->spe;
        if (adorn < 0)
            you_are("poorly adorned");
        else
            you_are("adorned");
    }
    if (Invisible) you_are("invisible");
    else if (Invis) you_are("invisible to others");
    /* ordinarily "visible" is redundant; this is a special case for
       the situation when invisibility would be an expected attribute */
    else if ((HInvis || EInvis || pm_invisible(youmonst.data)) && BInvis)
        you_are("visible");
    if (Displaced) you_are("displaced");
    if (Stealth) you_are("stealthy");
    if (Aggravate_monster) enl_msg("You aggravate", "", "d", " monsters");
    if (Conflict) enl_msg("You cause", "", "d", " conflict");

    /*** Transportation ***/
    if (Jumping) you_can("jump");
    if (Teleportation) you_can("teleport");
    if (Teleport_control) you_have("teleport control");
    if (Lev_at_will) you_are("levitating, at will");
    else if (Levitation) you_are("levitating");     /* without control */
    else if (Flying) you_can("fly");
    if (Wwalking) you_can("walk on water");
    if (Swimming) you_can("swim");
    if (Breathless) you_can("survive without air");
    else if (Amphibious) you_can("breathe water");
    if (Passes_walls) you_can("walk through walls");
    /* If you die while dismounting, u.usteed is still set.  Since several
     * places in the done() sequence depend on u.usteed, just detect this
     * special case. */
    if (u.usteed && (final < 2 || strcmp(killer, "riding accident"))) {
        sprintf(buf, "riding %s", y_monnam(u.usteed));
        you_are(buf);
    }
    if (u.uswallow) {
        sprintf(buf, "swallowed by %s", a_monnam(u.ustuck));
        if (wizard) sprintf(eos(buf), " (%u)", u.uswldtim);
        you_are(buf);
    } else if (u.ustuck) {
        sprintf(buf, "%s %s",
                (Upolyd && sticks(youmonst.data)) ? "holding" : "held by",
                a_monnam(u.ustuck));
        you_are(buf);
    }

    /*** Physical attributes ***/
    if (u.uhitinc)
        you_have(enlght_combatinc("to hit", u.uhitinc, final, buf));
    if (u.udaminc)
        you_have(enlght_combatinc("damage", u.udaminc, final, buf));
    if (Slow_digestion) you_have("slower digestion");
    if (Regeneration) enl_msg("You regenerate", "", "d", "");
    if (u.uspellprot || Protection) {
        int prot = 0;

        if(uleft && uleft->otyp == RIN_PROTECTION) prot += uleft->spe;
        if(uright && uright->otyp == RIN_PROTECTION) prot += uright->spe;
        if (HProtection & INTRINSIC) prot += u.ublessed;
        prot += u.uspellprot;

        if (prot < 0)
            you_are("ineffectively protected");
        else
            you_are("protected");
    }
    if (Protection_from_shape_changers)
        you_are("protected from shape changers");
    if (Polymorph) you_are("polymorphing");
    if (Polymorph_control) you_have("polymorph control");
    if (u.ulycn >= LOW_PM) {
        strcpy(buf, an(mons[u.ulycn].mname));
        you_are(buf);
    }
    if (Upolyd) {
        if (u.umonnum == u.ulycn) strcpy(buf, "in beast form");
        else sprintf(buf, "polymorphed into %s", an(youmonst.data->mname));
        if (wizard) sprintf(eos(buf), " (%d)", u.mtimedone);
        you_are(buf);
    }
    if (Unchanging) you_can("not change from your current form");
    if (Fast) you_are(Very_fast ? "very fast" : "fast");
    if (Reflecting) you_have("reflection");
    if (Free_action) you_have("free action");
    if (Fixed_abil) you_have("fixed abilities");
    if (Lifesaved)
        enl_msg("Your life ", "will be", "would have been", " saved");
    if (u.twoweap) you_are("wielding two weapons at once");

    /*** Miscellany ***/
    if (Luck) {
        int ltmp = abs((int)Luck);
        sprintf(buf, "%s%slucky",
                ltmp >= 10 ? "extremely " : ltmp >= 5 ? "very " : "",
                Luck < 0 ? "un" : "");
        if (wizard) sprintf(eos(buf), " (%d)", Luck);
        you_are(buf);
    }
    else if (wizard) enl_msg("Your luck ", "is", "was", " zero");
    if (u.moreluck > 0) you_have("extra luck");
    else if (u.moreluck < 0) you_have("reduced luck");
    if (carrying(LUCKSTONE) || stone_luck(true)) {
        int ltmp = stone_luck(false);
        if (ltmp <= 0)
            enl_msg("Bad luck ", "does", "did", " not time out for you");
        if (ltmp >= 0)
            enl_msg("Good luck ", "does", "did", " not time out for you");
    }

    if (u.ugangr) {
        sprintf(buf, " %sangry with you",
                u.ugangr > 6 ? "extremely " : u.ugangr > 3 ? "very " : "");
        if (wizard) sprintf(eos(buf), " (%d)", u.ugangr);
        enl_msg(u_gname(), " is", " was", buf);
    } else
        /*
         * We need to suppress this when the game is over, because death
         * can change the value calculated by can_pray(), potentially
         * resulting in a false claim that you could have prayed safely.
         */
        if (!final) {
            sprintf(buf, "%ssafely pray", can_pray(false) ? "" : "not ");
            if (wizard) sprintf(eos(buf), " (%d)", u.ublesscnt);
            you_can(buf);
        }

    {
        const char *p;

        buf[0] = '\0';
        if (final < 2) {    /* still in progress, or quit/escaped/ascended */
            p = "survived after being killed ";
            switch (u.umortality) {
                case 0:  p = !final ? (char *)0 : "survived";  break;
                case 1:  strcpy(buf, "once");  break;
                case 2:  strcpy(buf, "twice");  break;
                case 3:  strcpy(buf, "thrice");  break;
                default: sprintf(buf, "%d times", u.umortality);
                         break;
            }
        } else {                /* game ended in character's death */
            p = "are dead";
            switch (u.umortality) {
                case 0:  impossible("dead without dying?");
                case 1:  break;                     /* just "are dead" */
                default: sprintf(buf, " (%d%s time!)", u.umortality,
                                 ordin(u.umortality));
                         break;
            }
        }
        if (p) enl_msg(You_, "have been killed ", p, buf);
    }

    display_nhwindow(en_win, true);
    destroy_nhwindow(en_win);
    return;
}

void dump_enlightenment (int final) {
    int ltmp;
    char buf[BUFSZ];
    char buf2[BUFSZ];
    char *youwere = "  You were ";
    char *youhave = "  You have ";
    char *youhad  = "  You had ";
    char *youcould = "  You could ";

    dump("", "Final attributes");

    if (u.uevent.uhand_of_elbereth) {
        static const char * const hofe_titles[3] = {
            "the Hand of Elbereth",
            "the Envoy of Balance",
            "the Glory of Arioch"
        };
        dump(youwere,
                (char *)hofe_titles[u.uevent.uhand_of_elbereth - 1]);
    }

    if (u.ualign.record >= 20)
        dump(youwere, "piously aligned");
    else if (u.ualign.record > 13)
        dump(youwere, "devoutly aligned");
    else if (u.ualign.record > 8)
        dump(youwere, "fervently aligned");
    else if (u.ualign.record > 3)
        dump(youwere, "stridently aligned");
    else if (u.ualign.record == 3)
        dump(youwere, "aligned");
    else if (u.ualign.record > 0)
        dump(youwere, "haltingly aligned");
    else if (u.ualign.record == 0)
        dump(youwere, "nominally aligned");
    else if (u.ualign.record >= -3) dump(youhave, "strayed");
    else if (u.ualign.record >= -8) dump(youhave, "sinned");
    else dump("  You have ", "transgressed");
    if (wizard) {
        sprintf(buf, " %d", u.ualign.record);
        dump("  Your alignment was ", buf);
    }

    /*** Resistances to troubles ***/
    if (Fire_resistance) dump(youwere, "fire resistant");
    if (Cold_resistance) dump(youwere, "cold resistant");
    if (Sleep_resistance) dump(youwere, "sleep resistant");
    if (Disint_resistance) dump(youwere, "disintegration-resistant");
    if (Shock_resistance) dump(youwere, "shock resistant");
    if (Poison_resistance) dump(youwere, "poison resistant");
    if (Drain_resistance) dump(youwere, "level-drain resistant");
    if (Sick_resistance) dump(youwere, "immune to sickness");
    if (Antimagic) dump(youwere, "magic-protected");
    if (Acid_resistance) dump(youwere, "acid resistant");
    if (Stone_resistance) dump(youwere, "petrification resistant");
    if (Invulnerable) dump(youwere, "invulnerable");
    if (u.uedibility) dump(youcould, "recognize detrimental food");

    /*** Troubles ***/
    if (Halluc_resistance())  dump("  ", "You resisted hallucinations");
    if (Hallucination) dump(youwere, "hallucinating");
    if (Stunned) dump(youwere, "stunned");
    if (Confusion) dump(youwere, "confused");
    if (Blinded) dump(youwere, "blinded");
    if (Sick) {
        if (u.usick_type & SICK_VOMITABLE)
            dump(youwere, "sick from food poisoning");
        if (u.usick_type & SICK_NONVOMITABLE)
            dump(youwere, "sick from illness");
    }
    if (Stoned) dump(youwere, "turning to stone");
    if (Slimed) dump(youwere, "turning into slime");
    if (Strangled)
        dump(youwere, (u.uburied) ? "buried" : "being strangled");
    if (Glib) {
        sprintf(buf, "slippery %s", makeplural(body_part(FINGER)));
        dump(youhad, buf);
    }
    if (Fumbling) dump("  ", "You fumbled");
    if (Wounded_legs
            && !u.usteed
       ) {
        sprintf(buf, "wounded %s", makeplural(body_part(LEG)));
        dump(youhad, buf);
    }
    if (Wounded_legs && u.usteed) {
        x_monnam(buf, BUFSZ, u.usteed, ARTICLE_YOUR, NULL,
                SUPPRESS_SADDLE | SUPPRESS_HALLUCINATION, false);
        *buf = highc(*buf);
        strcat(buf, " had wounded legs");
        dump("  ", buf);
    }
    if (Sleeping) dump("  ", "You fell asleep");
    if (Hunger) dump("  ", "You hungered rapidly");

    /*** Vision and senses ***/
    if (See_invisible) dump("  ", "You saw invisible");
    if (Blind_telepat) dump(youwere, "telepathic");
    if (Warning) dump(youwere, "warned");
    if (Warn_of_mon && flags.warntype) {
        sprintf(buf, "aware of the presence of %s",
                (flags.warntype & M2_ORC) ? "orcs" :
                (flags.warntype & M2_DEMON) ? "demons" :
                something);
        dump(youwere, buf);
    }
    if (Undead_warning) dump(youwere, "warned of undead");
    if (Searching) dump(youhad, "automatic searching");
    if (Clairvoyant) dump(youwere, "clairvoyant");
    if (Infravision) dump(youhad, "infravision");
    if (Detect_monsters)
        dump(youwere, "sensing the presence of monsters");
    if (u.umconf) dump(youwere, "going to confuse monsters");

    /*** Appearance and behavior ***/
    if (Adornment) {
        int adorn = 0;
        if(uleft && uleft->otyp == RIN_ADORNMENT) adorn += uleft->spe;
        if(uright && uright->otyp == RIN_ADORNMENT) adorn += uright->spe;
        if (adorn < 0)
            dump(youwere, "poorly adorned");
        else
            dump(youwere, "adorned");
    }
    if (Invisible) dump(youwere, "invisible");
    else if (Invis) dump(youwere, "invisible to others");
    /* ordinarily "visible" is redundant; this is a special case for
       the situation when invisibility would be an expected attribute */
    else if ((HInvis || EInvis || pm_invisible(youmonst.data)) && BInvis)
        dump(youwere, "visible");
    if (Displaced) dump(youwere, "displaced");
    if (Stealth) dump(youwere, "stealthy");
    if (Aggravate_monster) dump("  ", "You aggravated monsters");
    if (Conflict) dump("  ", "You caused conflict");

    /*** Transportation ***/
    if (Jumping) dump(youcould, "jump");
    if (Teleportation) dump(youcould, "teleport");
    if (Teleport_control) dump(youhad, "teleport control");
    if (Lev_at_will) dump(youwere, "levitating, at will");
    else if (Levitation)
        dump(youwere, "levitating");  /* without control */
    else if (Flying) dump(youcould, "fly");
    if (Wwalking) dump(youcould, "walk on water");
    if (Swimming) dump(youcould, "swim");
    if (Breathless) dump(youcould, "survive without air");
    else if (Amphibious) dump(youcould, "breathe water");
    if (Passes_walls) dump(youcould, "walk through walls");
    if (u.usteed && (final < 2 || strcmp(killer, "riding accident"))) {
        sprintf(buf, "riding %s", y_monnam(u.usteed));
        dump(youwere, buf);
    }
    if (u.uswallow) {
        sprintf(buf, "swallowed by %s", a_monnam(u.ustuck));
        if (wizard) sprintf(eos(buf), " (%u)", u.uswldtim);
        dump(youwere, buf);
    } else if (u.ustuck) {
        sprintf(buf, "%s %s",
                (Upolyd && sticks(youmonst.data)) ? "holding" : "held by",
                a_monnam(u.ustuck));
        dump(youwere, buf);
    }

    /*** Physical attributes ***/
    if (u.uhitinc)
        dump(youhad,
                enlght_combatinc("to hit", u.uhitinc, final, buf));
    if (u.udaminc)
        dump(youhad,
                enlght_combatinc("damage", u.udaminc, final, buf));
    if (Slow_digestion) dump(youhad, "slower digestion");
    if (Regeneration) dump("  ", "You regenerated");
    if (u.uspellprot || Protection) {
        int prot = 0;

        if(uleft && uleft->otyp == RIN_PROTECTION) prot += uleft->spe;
        if(uright && uright->otyp == RIN_PROTECTION) prot += uright->spe;
        if (HProtection & INTRINSIC) prot += u.ublessed;
        prot += u.uspellprot;

        if (prot < 0)
            dump(youwere, "ineffectively protected");
        else
            dump(youwere, "protected");
    }
    if (Protection_from_shape_changers)
        dump(youwere, "protected from shape changers");
    if (Polymorph) dump(youwere, "polymorphing");
    if (Polymorph_control) dump(youhad, "polymorph control");
    if (u.ulycn >= LOW_PM) {
        strcpy(buf, an(mons[u.ulycn].mname));
        dump(youwere, buf);
    }
    if (Upolyd) {
        if (u.umonnum == u.ulycn) strcpy(buf, "in beast form");
        else sprintf(buf, "polymorphed into %s",
                an(youmonst.data->mname));
        if (wizard) sprintf(eos(buf), " (%d)", u.mtimedone);
        dump(youwere, buf);
    }
    if (Unchanging)
        dump(youcould, "not change from your current form");
    if (Fast) dump(youwere, Very_fast ? "very fast" : "fast");
    if (Reflecting) dump(youhad, "reflection");
    if (Free_action) dump(youhad, "free action");
    if (Fixed_abil) dump(youhad, "fixed abilities");
    if (Lifesaved)
        dump("  ", "Your life would have been saved");
    if (u.twoweap) dump(youwere, "wielding two weapons at once");

    /*** Miscellany ***/
    if (Luck) {
        ltmp = abs((int)Luck);
        sprintf(buf, "%s%slucky (%d)",
                ltmp >= 10 ? "extremely " : ltmp >= 5 ? "very " : "",
                Luck < 0 ? "un" : "", Luck);
        dump(youwere, buf);
    }
    else if (wizard) dump("  ", "Your luck was zero");
    if (u.moreluck > 0) dump(youhad, "extra luck");
    else if (u.moreluck < 0) dump(youhad, "reduced luck");
    if (carrying(LUCKSTONE) || stone_luck(true)) {
        ltmp = stone_luck(false);
        if (ltmp <= 0)
            dump("  ", "Bad luck did not time out for you");
        if (ltmp >= 0)
            dump("  ", "Good luck did not time out for you");
    }

    if (u.ugangr) {
        sprintf(buf, " %sangry with you",
                u.ugangr > 6 ? "extremely " : u.ugangr > 3 ? "very " : "");
        if (wizard) sprintf(eos(buf), " (%d)", u.ugangr);
        sprintf(buf2, "%s was %s", u_gname(), buf);
        dump("  ", buf2);
    }

    {
        const char *p;

        buf[0] = '\0';
        if (final < 2) {    /* quit/escaped/ascended */
            p = "survived after being killed ";
            switch (u.umortality) {
                case 0:  p = "survived";  break;
                case 1:  strcpy(buf, "once");  break;
                case 2:  strcpy(buf, "twice");  break;
                case 3:  strcpy(buf, "thrice");  break;
                default: sprintf(buf, "%d times", u.umortality);
                         break;
            }
        } else {                /* game ended in character's death */
            p = "are dead";
            switch (u.umortality) {
                case 0:  impossible("dead without dying?");
                case 1:  break;                     /* just "are dead" */
                default: sprintf(buf, " (%d%s time!)", u.umortality,
                                 ordin(u.umortality));
                         break;
            }
        }
        if (p) {
            sprintf(buf2, "You %s %s", p, buf);
            dump("  ", buf2);
        }
    }
    dump("", "");
    return;

} /* dump_enlightenment */

/*
 * Courtesy function for non-debug, non-explorer mode players
 * to help refresh them about who/what they are.
 * Returns false if menu cancelled (dismissed with ESC), true otherwise.
 */
static bool minimal_enlightenment (void) {
    winid tmpwin;
    menu_item *selected;
    anything any;
    int genidx, n;
    char buf[BUFSZ], buf2[BUFSZ];
    static const char untabbed_fmtstr[] = "%-15s: %-12s";
    static const char untabbed_deity_fmtstr[] = "%-17s%s";
    static const char tabbed_fmtstr[] = "%s:\t%-12s";
    static const char tabbed_deity_fmtstr[] = "%s\t%s";
    static const char *fmtstr;
    static const char *deity_fmtstr;

    fmtstr = iflags.menu_tab_sep ? tabbed_fmtstr : untabbed_fmtstr;
    deity_fmtstr = iflags.menu_tab_sep ?
        tabbed_deity_fmtstr : untabbed_deity_fmtstr;
    any.a_void = 0;
    buf[0] = buf2[0] = '\0';
    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin);
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, iflags.menu_headings, "Starting", false);

    /* Starting name, race, role, gender */
    sprintf(buf, fmtstr, "name", plname);
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, false);
    sprintf(buf, fmtstr, "race", urace.noun);
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, false);
    sprintf(buf, fmtstr, "role",
            (flags.initgend && urole.name.f) ? urole.name.f : urole.name.m);
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, false);
    sprintf(buf, fmtstr, "gender", genders[flags.initgend].adj);
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, false);

    /* Starting alignment */
    sprintf(buf, fmtstr, "alignment", align_str(u.ualignbase[A_ORIGINAL]));
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, false);

    /* Current name, race, role, gender */
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, "", false);
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, iflags.menu_headings, "Current", false);
    sprintf(buf, fmtstr, "race", Upolyd ? youmonst.data->mname : urace.noun);
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, false);
    if (Upolyd) {
        sprintf(buf, fmtstr, "role (base)",
                (u.mfemale && urole.name.f) ? urole.name.f : urole.name.m);
        add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, false);
    } else {
        sprintf(buf, fmtstr, "role",
                (flags.female && urole.name.f) ? urole.name.f : urole.name.m);
        add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, false);
    }
    /* don't want poly_gender() here; it forces `2' for non-humanoids */
    genidx = is_neuter(youmonst.data) ? 2 : flags.female;
    sprintf(buf, fmtstr, "gender", genders[genidx].adj);
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, false);
    if (Upolyd && (int)u.mfemale != genidx) {
        sprintf(buf, fmtstr, "gender (base)", genders[u.mfemale].adj);
        add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, false);
    }

    /* Current alignment */
    sprintf(buf, fmtstr, "alignment", align_str(u.ualign.type));
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, false);

    /* Deity list */
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, "", false);
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, iflags.menu_headings, "Deities", false);
    sprintf(buf2, deity_fmtstr, align_gname(A_CHAOTIC),
            (u.ualignbase[A_ORIGINAL] == u.ualign.type
             && u.ualign.type == A_CHAOTIC) ? " (s,c)" :
            (u.ualignbase[A_ORIGINAL] == A_CHAOTIC)       ? " (s)" :
            (u.ualign.type   == A_CHAOTIC)       ? " (c)" : "");
    sprintf(buf, fmtstr, "Chaotic", buf2);
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, false);

    sprintf(buf2, deity_fmtstr, align_gname(A_NEUTRAL),
            (u.ualignbase[A_ORIGINAL] == u.ualign.type
             && u.ualign.type == A_NEUTRAL) ? " (s,c)" :
            (u.ualignbase[A_ORIGINAL] == A_NEUTRAL)       ? " (s)" :
            (u.ualign.type   == A_NEUTRAL)       ? " (c)" : "");
    sprintf(buf, fmtstr, "Neutral", buf2);
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, false);

    sprintf(buf2, deity_fmtstr, align_gname(A_LAWFUL),
            (u.ualignbase[A_ORIGINAL] == u.ualign.type &&
             u.ualign.type == A_LAWFUL)  ? " (s,c)" :
            (u.ualignbase[A_ORIGINAL] == A_LAWFUL)        ? " (s)" :
            (u.ualign.type   == A_LAWFUL)        ? " (c)" : "");
    sprintf(buf, fmtstr, "Lawful", buf2);
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, false);

    end_menu(tmpwin, "Base Attributes");
    n = select_menu(tmpwin, PICK_NONE, &selected);
    destroy_nhwindow(tmpwin);
    return (n != -1);
}

static int doattributes (void) {
    if (!minimal_enlightenment())
        return 0;
    if (wizard || discover)
        enlightenment(0);
    return 0;
}

/* KMH, #conduct
 * (shares enlightenment's tense handling)
 */
static int doconduct (void) {
    show_conduct(0);
    return 0;
}

void show_conduct (int final) {
    char buf[BUFSZ];
    int ngenocided;

    /* Create the conduct window */
    en_win = create_nhwindow(NHW_MENU);
    putstr(en_win, 0, "Voluntary challenges:");
    putstr(en_win, 0, "");

    if (!u.uconduct.food)
        enl_msg(You_, "have gone", "went", " without food");
    /* But beverages are okay */
    else if (!u.uconduct.unvegan)
        you_have_X("followed a strict vegan diet");
    else if (!u.uconduct.unvegetarian)
        you_have_been("vegetarian");

    if (!u.uconduct.gnostic)
        you_have_been("an atheist");

    if (!u.uconduct.weaphit)
        you_have_never("hit with a wielded weapon");
    else if (wizard) {
        sprintf(buf, "used a wielded weapon %ld time%s",
                u.uconduct.weaphit, plur(u.uconduct.weaphit));
        you_have_X(buf);
    }
    if (!u.uconduct.killer)
        you_have_been("a pacifist");

    if (!u.uconduct.literate)
        you_have_been("illiterate");
    else if (wizard) {
        sprintf(buf, "read items or engraved %ld time%s",
                u.uconduct.literate, plur(u.uconduct.literate));
        you_have_X(buf);
    }

    ngenocided = num_genocides();
    if (ngenocided == 0) {
        you_have_never("genocided any monsters");
    } else {
        sprintf(buf, "genocided %d type%s of monster%s",
                ngenocided, plur(ngenocided), plur(ngenocided));
        you_have_X(buf);
    }

    if (!u.uconduct.polypiles)
        you_have_never("polymorphed an object");
    else if (wizard) {
        sprintf(buf, "polymorphed %ld item%s",
                u.uconduct.polypiles, plur(u.uconduct.polypiles));
        you_have_X(buf);
    }

    if (!u.uconduct.polyselfs)
        you_have_never("changed form");
    else if (wizard) {
        sprintf(buf, "changed form %ld time%s",
                u.uconduct.polyselfs, plur(u.uconduct.polyselfs));
        you_have_X(buf);
    }

    if (!u.uconduct.wishes)
        you_have_X("used no wishes");
    else {
        sprintf(buf, "used %ld wish%s",
                u.uconduct.wishes, (u.uconduct.wishes > 1L) ? "es" : "");
        you_have_X(buf);

        if (!u.uconduct.wisharti)
            enl_msg(You_, "have not wished", "did not wish",
                    " for any artifacts");
    }

    /* Pop up the window and wait for a key */
    display_nhwindow(en_win, true);
    destroy_nhwindow(en_win);
}

void dump_conduct (int final) {
    char buf[BUFSZ];
    int ngenocided;

    dump("", "Voluntary challenges");

    if (!u.uconduct.food)
        dump("", "  You went without food");
    /* But beverages are okay */
    else if (!u.uconduct.unvegan)
        dump("", "  You followed a strict vegan diet");
    else if (!u.uconduct.unvegetarian)
        dump("", "  You were a vegetarian");
    else if (Role_if(PM_MONK) && u.uconduct.unvegetarian < 10) {
        sprintf(buf, "  You ate non-vegetarian food %ld time%s.",
                u.uconduct.unvegetarian, plur(u.uconduct.unvegetarian));
        dump("", buf);
    }

    if (!u.uconduct.gnostic)
        dump("", "  You were an atheist");

    if (!u.uconduct.weaphit)
        dump("", "  You never hit with a wielded weapon");
    else if (Role_if(PM_MONK) && u.uconduct.weaphit < 10) {
        sprintf(buf, "  You hit with a wielded weapon %ld time%s",
                u.uconduct.weaphit, plur(u.uconduct.weaphit));
        dump("", buf);
    }
    else if (wizard) {
        sprintf(buf, "hit with a wielded weapon %ld time%s",
                u.uconduct.weaphit, plur(u.uconduct.weaphit));
        dump("  You ", buf);
    }
    if (!u.uconduct.killer)
        dump("", "  You were a pacifist");

    if (!u.uconduct.literate)
        dump("", "  You were illiterate");
    else if (wizard) {
        sprintf(buf, "read items or engraved %ld time%s",
                u.uconduct.literate, plur(u.uconduct.literate));
        dump("  You ", buf);
    }

    ngenocided = num_genocides();
    if (ngenocided == 0) {
        dump("", "  You never genocided any monsters");
    } else {
        sprintf(buf, "genocided %d type%s of monster%s",
                ngenocided, plur(ngenocided), plur(ngenocided));
        dump("  You ", buf);
    }

    if (!u.uconduct.polypiles)
        dump("", "  You never polymorphed an object");
    else {
        sprintf(buf, "polymorphed %ld item%s",
                u.uconduct.polypiles, plur(u.uconduct.polypiles));
        dump("  You ", buf);
    }

    if (!u.uconduct.polyselfs)
        dump("", "  You never changed form");
    else {
        sprintf(buf, "changed form %ld time%s",
                u.uconduct.polyselfs, plur(u.uconduct.polyselfs));
        dump("  You ", buf);
    }

    if (!u.uconduct.wishes)
        dump("", "  You used no wishes");
    else {
        sprintf(buf, "used %ld wish%s",
                u.uconduct.wishes, (u.uconduct.wishes > 1L) ? "es" : "");
        dump("  You ", buf);

        if (!u.uconduct.wisharti)
            dump("", "  You did not wish for any artifacts");
    }

    dump("", "");
}

#define M(c) ((c) - 128)
#define C(c) (0x1f & (c))

static const struct func_tab cmdlist[] = {
    {C('d'), false, dokick}, /* "D" is for door!...?  Msg is in dokick.c */
    {C('e'), true, wiz_detect},
    {C('f'), true, wiz_map},
    {C('g'), true, wiz_genesis},
    {C('i'), true, wiz_identify},
    {C('l'), true, doredraw}, /* if number_pad is set */
    {C('o'), true, wiz_where},
    {C('p'), true, doprev_message},
    {C('r'), true, doredraw},
    {C('t'), true, dotele},
    {C('v'), true, wiz_level_tele},
    {C('w'), true, wiz_wish},
    {C('x'), true, doattributes},
    {'a', false, doapply},
    {'A', false, doddoremarm},
    {M('a'), true, doorganize},
    /*      'b', 'B' : go sw */
    {'c', false, doclose},
    {'C', true, do_mname},
    {M('c'), true, dotalk},
    {'d', false, dodrop},
    {'D', false, doddrop},
    {M('d'), false, dodip},
    {'e', false, doeat},
    {'E', false, doengrave},
    {M('e'), true, enhance_weapon_skill},
    {'f', false, dofire},
    /*      'F' : fight (one time) */
    {M('f'), false, doforce},
    /*      'g', 'G' : multiple go */
    /*      'h', 'H' : go west */
    {'h', true, dohelp}, /* if number_pad is set */
    {'i', true, ddoinv},
    {'I', true, dotypeinv},         /* Robert Viduya */
    {M('i'), true, doinvoke},
    /*      'j', 'J', 'k', 'K', 'l', 'L', 'm', 'M', 'n', 'N' : move commands */
    {'j', false, dojump}, /* if number_pad is on */
    {M('j'), false, dojump},
    {'k', false, dokick}, /* if number_pad is on */
    {'l', false, doloot}, /* if number_pad is on */
    {M('l'), false, doloot},
    /*      'n' prefixes a count if number_pad is on */
    {M('m'), true, domonability},
    {'N', true, ddocall}, /* if number_pad is on */
    {M('n'), true, ddocall},
    {M('N'), true, ddocall},
    {'o', false, doopen},
    {'O', true, doset},
    {M('o'), false, dosacrifice},
    {'p', false, dopay},
    {'P', false, doputon},
    {M('p'), true, dopray},
    {'q', false, dodrink},
    {'Q', false, dowieldquiver},
    {M('q'), true, done2},
    {'r', false, doread},
    {'R', false, doremring},
    {M('r'), false, dorub},
    {'s', true, dosearch, "searching"},
    {'S', true, dosave},
    {M('s'), false, dosit},
    {'t', false, dothrow},
    {'T', false, dotakeoff},
    {M('t'), true, doturn},
    /*      'u', 'U' : go ne */
    {'u', false, dountrap}, /* if number_pad is on */
    {M('u'), false, dountrap},
    {'v', true, doversion},
    {'V', true, dohistory},
    {M('v'), true, doextversion},
    {'w', false, dowield},
    {'W', false, dowear},
    {M('w'), false, dowipe},
    {'x', false, doswapweapon},
    {'X', true, enter_explore_mode},
    /*      'y', 'Y' : go nw */
    {'z', false, dozap},
    {'Z', true, docast},
    {'<', false, doup},
    {'>', false, dodown},
    {'/', true, dowhatis},
    {'&', true, dowhatdoes},
    {'?', true, dohelp},
    {M('?'), true, doextlist},
    {'.', true, donull, "waiting"},
    {' ', true, donull, "waiting"},
    {',', false, dopickup},
    {':', true, dolook},
    {';', true, doquickwhatis},
    {'^', true, doidtrap},
    {'\\', true, dodiscovered},             /* Robert Viduya */
    {'@', true, dotogglepickup},
    {M('2'), false, dotwoweapon},
    {WEAPON_SYM,  true, doprwep},
    {ARMOR_SYM,  true, doprarm},
    {RING_SYM,  true, doprring},
    {AMULET_SYM, true, dopramulet},
    {TOOL_SYM, true, doprtool},
    {'*', true, doprinuse}, /* inventory of all equipment in use */
    {GOLD_SYM, true, doprgold},
    {SPBOOK_SYM, true, dovspell},                   /* Mike Stephenson */
    {'#', true, doextcmd},
    {'_', true, dotravel},
    {0,0,0,0}
};

struct ext_func_tab extcmdlist[] = {
    {"adjust", "adjust inventory letters", doorganize, true},
    {"chat", "talk to someone", dotalk, true},      /* converse? */
    {"conduct", "list which challenges you have adhered to", doconduct, true},
    {"dip", "dip an object into something", dodip, false},
    {"enhance", "advance or check weapons skills", enhance_weapon_skill,
        true},
    {"force", "force a lock", doforce, false},
    {"invoke", "invoke an object's powers", doinvoke, true},
    {"jump", "jump to a location", dojump, false},
    {"loot", "loot a box on the floor", doloot, false},
    {"monster", "use a monster's special ability", domonability, true},
    {"name", "name an item or type of object", ddocall, true},
    {"offer", "offer a sacrifice to the gods", dosacrifice, false},
    {"pray", "pray to the gods for help", dopray, true},
    {"quit", "exit without saving current game", done2, true},
    {"ride", "ride (or stop riding) a monster", doride, false},
    {"rub", "rub a lamp or a stone", dorub, false},
    {"sit", "sit down", dosit, false},
    {"turn", "turn undead", doturn, true},
    {"twoweapon", "toggle two-weapon combat", dotwoweapon, false},
    {"untrap", "untrap something", dountrap, false},
    {"version", "list compile time options for this version of NetHack",
        doextversion, true},
    {"wipe", "wipe off your face", dowipe, false},
    {"?", "get this list of extended commands", doextlist, true},
    /*
     * There must be a blank entry here for every entry in the table
     * below.
     */
    {(char *)0, (char *)0, donull, true},
    {(char *)0, (char *)0, donull, true},
    {(char *)0, (char *)0, donull, true},
    {(char *)0, (char *)0, donull, true},
    {(char *)0, (char *)0, donull, true},
    {(char *)0, (char *)0, donull, true},
    {(char *)0, (char *)0, donull, true},
    {(char *)0, (char *)0, donull, true},
    {(char *)0, (char *)0, donull, true},
    {(char *)0, (char *)0, donull, true},
    {(char *)0, (char *)0, donull, true}    /* sentinel */
};

static const struct ext_func_tab debug_extcmdlist[] = {
    {"levelchange", "change experience level", wiz_level_change, true},
    {"lightsources", "show mobile light sources", wiz_light_sources, true},
    {"monpolycontrol", "control monster polymorphs", wiz_mon_polycontrol, true},
    {"panic", "test panic routine (fatal to game)", wiz_panic, true},
    {"polyself", "polymorph self", wiz_polyself, true},
    {"seenv", "show seen vectors", wiz_show_seenv, true},
    {"stats", "show memory statistics", wiz_show_stats, true},
    {"timeout", "look at timeout queue", wiz_timeout_queue, true},
    {"vision", "show vision array", wiz_show_vision, true},
    {"wmode", "show wall modes", wiz_show_wmodes, true},
    {(char *)0, (char *)0, donull, true}
};

/*
 * Insert debug commands into the extended command list.  This function
 * assumes that the last entry will be the help entry.
 *
 * You must add entries in ext_func_tab every time you add one to the
 * debug_extcmdlist().
 */
void add_debug_extended_commands (void) {
    int i, j, k, n;

    /* count the # of help entries */
    for (n = 0; extcmdlist[n].ef_txt[0] != '?'; n++)
        ;

    for (i = 0; debug_extcmdlist[i].ef_txt; i++) {
        for (j = 0; j < n; j++)
            if (strcmp(debug_extcmdlist[i].ef_txt, extcmdlist[j].ef_txt) < 0) break;

        /* insert i'th debug entry into extcmdlist[j], pushing down  */
        for (k = n; k >= j; --k)
            extcmdlist[k+1] = extcmdlist[k];
        extcmdlist[j] = debug_extcmdlist[i];
        n++;        /* now an extra entry */
    }
}


static const char template[] = "%-18s %4ld  %6ld";
static const char count_str[] = "                   count  bytes";
static const char separator[] = "------------------ -----  ------";

static void count_obj(struct obj *chain, long *total_count, long *total_size,
        bool top, bool recurse)
{
    long count, size;
    struct obj *obj;

    for (count = size = 0, obj = chain; obj; obj = obj->nobj) {
        if (top) {
            count++;
            size += sizeof(struct obj) + obj->oxlth + obj->onamelth;
        }
        if (recurse && obj->cobj)
            count_obj(obj->cobj, total_count, total_size, true, true);
    }
    *total_count += count;
    *total_size += size;
}

static void obj_chain (winid win, const char *src, struct obj *chain,
        long *total_count, long *total_size)
{
    char buf[BUFSZ];
    long count = 0, size = 0;

    count_obj(chain, &count, &size, true, false);
    *total_count += count;
    *total_size += size;
    sprintf(buf, template, src, count, size);
    putstr(win, 0, buf);
}

static void mon_invent_chain (winid win, const char *src, struct monst *chain,
        long *total_count, long *total_size)
{
    char buf[BUFSZ];
    long count = 0, size = 0;
    struct monst *mon;

    for (mon = chain; mon; mon = mon->nmon)
        count_obj(mon->minvent, &count, &size, true, false);
    *total_count += count;
    *total_size += size;
    sprintf(buf, template, src, count, size);
    putstr(win, 0, buf);
}

static void contained (winid win, const char *src, long *total_count, long *total_size) {
    char buf[BUFSZ];
    long count = 0, size = 0;
    struct monst *mon;

    count_obj(invent, &count, &size, false, true);
    count_obj(fobj, &count, &size, false, true);
    count_obj(level.buriedobjlist, &count, &size, false, true);
    count_obj(migrating_objs, &count, &size, false, true);
    /* DEADMONSTER check not required in this loop since they have no inventory */
    for (mon = fmon; mon; mon = mon->nmon)
        count_obj(mon->minvent, &count, &size, false, true);
    for (mon = migrating_mons; mon; mon = mon->nmon)
        count_obj(mon->minvent, &count, &size, false, true);

    *total_count += count; *total_size += size;

    sprintf(buf, template, src, count, size);
    putstr(win, 0, buf);
}

static void mon_chain (winid win, const char *src, struct monst *chain,
        long *total_count, long *total_size)
{
    char buf[BUFSZ];
    long count, size;
    struct monst *mon;

    for (count = size = 0, mon = chain; mon; mon = mon->nmon) {
        count++;
        size += sizeof(struct monst) + mon->mxlth + mon->mnamelth;
    }
    *total_count += count;
    *total_size += size;
    sprintf(buf, template, src, count, size);
    putstr(win, 0, buf);
}

/*
 * Display memory usage of all monsters and objects on the level.
 */
static int wiz_show_stats (void) {
    char buf[BUFSZ];
    winid win;
    long total_obj_size = 0, total_obj_count = 0;
    long total_mon_size = 0, total_mon_count = 0;

    win = create_nhwindow(NHW_TEXT);
    putstr(win, 0, "Current memory statistics:");
    putstr(win, 0, "");
    sprintf(buf, "Objects, size %d", (int) sizeof(struct obj));
    putstr(win, 0, buf);
    putstr(win, 0, "");
    putstr(win, 0, count_str);

    obj_chain(win, "invent", invent, &total_obj_count, &total_obj_size);
    obj_chain(win, "fobj", fobj, &total_obj_count, &total_obj_size);
    obj_chain(win, "buried", level.buriedobjlist,
            &total_obj_count, &total_obj_size);
    obj_chain(win, "migrating obj", migrating_objs,
            &total_obj_count, &total_obj_size);
    mon_invent_chain(win, "minvent", fmon,
            &total_obj_count,&total_obj_size);
    mon_invent_chain(win, "migrating minvent", migrating_mons,
            &total_obj_count, &total_obj_size);

    contained(win, "contained",
            &total_obj_count, &total_obj_size);

    putstr(win, 0, separator);
    sprintf(buf, template, "Total", total_obj_count, total_obj_size);
    putstr(win, 0, buf);

    putstr(win, 0, "");
    putstr(win, 0, "");
    sprintf(buf, "Monsters, size %d", (int) sizeof(struct monst));
    putstr(win, 0, buf);
    putstr(win, 0, "");

    mon_chain(win, "fmon", fmon,
            &total_mon_count, &total_mon_size);
    mon_chain(win, "migrating", migrating_mons,
            &total_mon_count, &total_mon_size);

    putstr(win, 0, separator);
    sprintf(buf, template, "Total", total_mon_count, total_mon_size);
    putstr(win, 0, buf);

    display_nhwindow(win, false);
    destroy_nhwindow(win);
    return 0;
}

void sanity_check (void) {
    obj_sanity_check();
    timer_sanity_check();
}



#define unctrl(c)       ((c) <= C('z') ? (0x60 | (c)) : (c))
#define unmeta(c)       (0x7f & (c))


    void
rhack (char *cmd)
{
    bool do_walk, do_rush, prefix_seen, bad_command,
            firsttime = (cmd == 0);

    iflags.menu_requested = false;
    if (firsttime) {
        flags.nopick = 0;
        cmd = parse();
    }
    if (*cmd == '\033') {
        flags.move = false;
        return;
    }
    if (*cmd == DOAGAIN && !in_doagain && saveq[0]) {
        in_doagain = true;
        stail = 0;
        rhack((char *)0);       /* read and execute command */
        in_doagain = false;
        return;
    }
    /* Special case of *cmd == ' ' handled better below */
    if(!*cmd || *cmd == (char)0377) {
        nhbell();
        flags.move = false;
        return;         /* probably we just had an interrupt */
    }
    if (iflags.num_pad && iflags.num_pad_mode == 1) {
        /* This handles very old inconsistent DOS/Windows behaviour
         * in a new way: earlier, the keyboard handler mapped these,
         * which caused counts to be strange when entered from the
         * number pad. Now do not map them until here.
         */
        switch (*cmd) {
            case '5':       *cmd = 'g'; break;
            case M('5'):    *cmd = 'G'; break;
            case M('0'):    *cmd = 'I'; break;
        }
    }
    /* handle most movement commands */
    do_walk = do_rush = prefix_seen = false;
    flags.travel = iflags.travel1 = 0;
    switch (*cmd) {
        case 'g':  if (movecmd(cmd[1])) {
                       flags.run = 2;
                       do_rush = true;
                   } else
                       prefix_seen = true;
                   break;
        case '5':  if (!iflags.num_pad) break; /* else FALLTHRU */
        case 'G':  if (movecmd(lowc(cmd[1]))) {
                       flags.run = 3;
                       do_rush = true;
                   } else
                       prefix_seen = true;
                   break;
        case '-':  if (!iflags.num_pad) break; /* else FALLTHRU */
                       /* Effects of movement commands and invisible monsters:
                        * m: always move onto space (even if 'I' remembered)
                        * F: always attack space (even if 'I' not remembered)
                        * normal movement: attack if 'I', move otherwise
                        */
        case 'F':  if (movecmd(cmd[1])) {
                       flags.forcefight = 1;
                       do_walk = true;
                   } else
                       prefix_seen = true;
                   break;
        case 'm':  if (movecmd(cmd[1]) || u.dz) {
                       flags.run = 0;
                       flags.nopick = 1;
                       if (!u.dz) do_walk = true;
                       else cmd[0] = cmd[1];   /* "m<" or "m>" */
                   } else
                       prefix_seen = true;
                   break;
        case 'M':  if (movecmd(lowc(cmd[1]))) {
                       flags.run = 1;
                       flags.nopick = 1;
                       do_rush = true;
                   } else
                       prefix_seen = true;
                   break;
        case '0':  if (!iflags.num_pad) break;
                       (void)ddoinv(); /* a convenience borrowed from the PC */
                   flags.move = false;
                   multi = 0;
                   return;
        case CMD_TRAVEL:
                   if (iflags.travelcmd) {
                       flags.travel = 1;
                       iflags.travel1 = 1;
                       flags.run = 8;
                       flags.nopick = 1;
                       do_rush = true;
                       break;
                   }
                   /*FALLTHRU*/
        default:   if (movecmd(*cmd)) {        /* ordinary movement */
                       flags.run = 0;  /* only matters here if it was 8 */
                       do_walk = true;
                   } else if (movecmd(iflags.num_pad ?
                               unmeta(*cmd) : lowc(*cmd))) {
                       flags.run = 1;
                       do_rush = true;
                   } else if (movecmd(unctrl(*cmd))) {
                       flags.run = 3;
                       do_rush = true;
                   }
                   break;
    }

    /* some special prefix handling */
    /* overload 'm' prefix for ',' to mean "request a menu" */
    if (prefix_seen && cmd[1] == ',') {
        iflags.menu_requested = true;
        ++cmd;
    }

    if (do_walk) {
        if (multi) flags.mv = true;
        domove();
        flags.forcefight = 0;
        return;
    } else if (do_rush) {
        if (firsttime) {
            if (!multi) multi = max(COLNO,ROWNO);
            u.last_str_turn = 0;
        }
        flags.mv = true;
        domove();
        return;
    } else if (prefix_seen && cmd[1] == '\033') {   /* <prefix><escape> */
        /* don't report "unknown command" for change of heart... */
        bad_command = false;
    } else if (*cmd == ' ' && !flags.rest_on_space) {
        bad_command = true;         /* skip cmdlist[] loop */

        /* handle all other commands */
    } else {
        const struct func_tab *tlist;
        int res, (*func)(void);

        for (tlist = cmdlist; tlist->f_char; tlist++) {
            if ((*cmd & 0xff) != (tlist->f_char & 0xff)) continue;

            if (u.uburied && !tlist->can_if_buried) {
                You_cant("do that while you are buried!");
                res = 0;
            } else {
                /* we discard 'const' because some compilers seem to have
                   trouble with the pointer passed to set_occupation() */
                func = ((struct func_tab *)tlist)->f_funct;
                if (tlist->f_text && !occupation && multi)
                    set_occupation(func, tlist->f_text, multi);
                res = (*func)();            /* perform the command */
            }
            if (!res) {
                flags.move = false;
                multi = 0;
            }
            return;
        }
        /* if we reach here, cmd wasn't found in cmdlist[] */
        bad_command = true;
    }

    if (bad_command) {
        char expcmd[10];
        char *cp = expcmd;

        while (*cmd && (int)(cp - expcmd) < (int)(sizeof expcmd - 3)) {
            if (*cmd >= 040 && *cmd < 0177) {
                *cp++ = *cmd++;
            } else if (*cmd & 0200) {
                *cp++ = 'M';
                *cp++ = '-';
                *cp++ = *cmd++ &= ~0200;
            } else {
                *cp++ = '^';
                *cp++ = *cmd++ ^ 0100;
            }
        }
        *cp = '\0';
        if (!prefix_seen || !iflags.cmdassist ||
                !help_dir(0, "Invalid direction key!"))
            Norep("Unknown command '%s'.", expcmd);
    }
    /* didn't move */
    flags.move = false;
    multi = 0;
    return;
}

int
xytod ( /* convert an x,y pair into a direction code */
        signed char x,
        signed char y
      )
{
    int dd;

    for(dd = 0; dd < 8; dd++)
        if(x == xdir[dd] && y == ydir[dd]) return dd;

    return -1;
}

/* convert a direction code into an x,y pair */
void dtoxy ( coord *cc, int dd) {
    cc->x = xdir[dd];
    cc->y = ydir[dd];
    return;
}

/* also sets u.dz, but returns false for <> */
int movecmd (char sym) {
    const char *dp;
    const char *sdp;
    if(iflags.num_pad) sdp = ndir; else sdp = sdir; /* DICE workaround */

    u.dz = 0;
    if(!(dp = index(sdp, sym))) return 0;
    u.dx = xdir[dp-sdp];
    u.dy = ydir[dp-sdp];
    u.dz = zdir[dp-sdp];
    if (u.dx && u.dy && u.umonnum == PM_GRID_BUG) {
        u.dx = u.dy = 0;
        return 0;
    }
    return !u.dz;
}

/*
 * uses getdir() but unlike getdir() it specifically
 * produces coordinates using the direction from getdir()
 * and verifies that those coordinates are ok.
 *
 * If the call to getdir() returns 0, Never_mind is displayed.
 * If the resulting coordinates are not okay, emsg is displayed.
 *
 * Returns non-zero if coordinates in cc are valid.
 */
int get_adjacent_loc (const char *prompt, const char *emsg, signed char x, signed char y, coord *cc) {
    signed char new_x, new_y;
    if (!getdir(prompt)) {
        plines(Never_mind);
        return 0;
    }
    new_x = x + u.dx;
    new_y = y + u.dy;
    if (cc && isok(new_x,new_y)) {
        cc->x = new_x;
        cc->y = new_y;
    } else {
        if (emsg) plines(emsg);
        return 0;
    }
    return 1;
}

int getdir (const char *s) {
    char dirsym;

    if(in_doagain || *readchar_queue)
        dirsym = readchar();
    else
        dirsym = yn_function ((s && *s != '^') ? s : "In what direction?",
                (char *)0, '\0');
    savech(dirsym);
    if(dirsym == '.' || dirsym == 's')
        u.dx = u.dy = u.dz = 0;
    else if(!movecmd(dirsym) && !u.dz) {
        bool did_help = false;
        if(!index(quitchars, dirsym)) {
            if (iflags.cmdassist) {
                did_help = help_dir((s && *s == '^') ? dirsym : 0,
                        "Invalid direction key!");
            }
            if (!did_help) pline("What a strange direction!");
        }
        return 0;
    }
    if(!u.dz && (Stunned || (Confusion && !rn2(5)))) confdir();
    return 1;
}

static bool help_dir(char sym, const char *msg) {
    char ctrl;
    winid win;
    static const char wiz_only_list[] = "EFGIOVW";
    char buf[BUFSZ], buf2[BUFSZ], *expl;

    win = create_nhwindow(NHW_TEXT);
    if (!win) return false;
    if (msg) {
        sprintf(buf, "cmdassist: %s", msg);
        putstr(win, 0, buf);
        putstr(win, 0, "");
    }
    if (letter(sym)) {
        sym = highc(sym);
        ctrl = (sym - 'A') + 1;
        if ((expl = dowhatdoes_core(ctrl, buf2))
                && (!index(wiz_only_list, sym)
                    || wizard
                   )) {
            sprintf(buf, "Are you trying to use ^%c%s?", sym,
                    index(wiz_only_list, sym) ? "" :
                    " as specified in the Guidebook");
            putstr(win, 0, buf);
            putstr(win, 0, "");
            putstr(win, 0, expl);
            putstr(win, 0, "");
            putstr(win, 0, "To use that command, you press");
            sprintf(buf,
                    "the <Ctrl> key, and the <%c> key at the same time.", sym);
            putstr(win, 0, buf);
            putstr(win, 0, "");
        }
    }
    if (iflags.num_pad && u.umonnum == PM_GRID_BUG) {
        putstr(win, 0, "Valid direction keys in your current form (with number_pad on) are:");
        putstr(win, 0, "             8   ");
        putstr(win, 0, "             |   ");
        putstr(win, 0, "          4- . -6");
        putstr(win, 0, "             |   ");
        putstr(win, 0, "             2   ");
    } else if (u.umonnum == PM_GRID_BUG) {
        putstr(win, 0, "Valid direction keys in your current form are:");
        putstr(win, 0, "             k   ");
        putstr(win, 0, "             |   ");
        putstr(win, 0, "          h- . -l");
        putstr(win, 0, "             |   ");
        putstr(win, 0, "             j   ");
    } else if (iflags.num_pad) {
        putstr(win, 0, "Valid direction keys (with number_pad on) are:");
        putstr(win, 0, "          7  8  9");
        putstr(win, 0, "           \\ | / ");
        putstr(win, 0, "          4- . -6");
        putstr(win, 0, "           / | \\ ");
        putstr(win, 0, "          1  2  3");
    } else {
        putstr(win, 0, "Valid direction keys are:");
        putstr(win, 0, "          y  k  u");
        putstr(win, 0, "           \\ | / ");
        putstr(win, 0, "          h- . -l");
        putstr(win, 0, "           / | \\ ");
        putstr(win, 0, "          b  j  n");
    };
    putstr(win, 0, "");
    putstr(win, 0, "          <  up");
    putstr(win, 0, "          >  down");
    putstr(win, 0, "          .  direct at yourself");
    putstr(win, 0, "");
    putstr(win, 0, "(Suppress this message with !cmdassist in config file.)");
    display_nhwindow(win, false);
    destroy_nhwindow(win);
    return true;
}


void confdir (void) {
    int x = (u.umonnum == PM_GRID_BUG) ? 2*rn2(4) : rn2(8);
    u.dx = xdir[x];
    u.dy = ydir[x];
    return;
}


int isok (int x, int y) {
    /* x corresponds to curx, so x==1 is the first column. Ach. %% */
    return x >= 1 && x <= COLNO-1 && y >= 0 && y <= ROWNO-1;
}

static int last_multi;

/*
 * convert a MAP window position into a movecmd
 */
const char * click_to_cmd (int x, int y, int mod) {
    int dir;
    static char cmd[4];
    cmd[1]=0;

    x -= u.ux;
    y -= u.uy;

    if (iflags.travelcmd) {
        if (abs(x) <= 1 && abs(y) <= 1 ) {
            x = sgn(x), y = sgn(y);
        } else {
            u.tx = u.ux+x;
            u.ty = u.uy+y;
            cmd[0] = CMD_TRAVEL;
            return cmd;
        }

        if(x == 0 && y == 0) {
            /* here */
            if(IS_FOUNTAIN(levl[u.ux][u.uy].typ) || IS_SINK(levl[u.ux][u.uy].typ)) {
                cmd[0]=mod == CLICK_1 ? 'q' : M('d');
                return cmd;
            } else if(IS_THRONE(levl[u.ux][u.uy].typ)) {
                cmd[0]=M('s');
                return cmd;
            } else if((u.ux == xupstair && u.uy == yupstair)
                    || (u.ux == sstairs.sx && u.uy == sstairs.sy && sstairs.up)
                    || (u.ux == xupladder && u.uy == yupladder)) {
                return "<";
            } else if((u.ux == xdnstair && u.uy == ydnstair)
                    || (u.ux == sstairs.sx && u.uy == sstairs.sy && !sstairs.up)
                    || (u.ux == xdnladder && u.uy == ydnladder)) {
                return ">";
            } else if(OBJ_AT(u.ux, u.uy)) {
                cmd[0] = Is_container(level.objects[u.ux][u.uy]) ? M('l') : ',';
                return cmd;
            } else {
                return "."; /* just rest */
            }
        }

        /* directional commands */

        dir = xytod(x, y);

        if (!m_at(u.ux+x, u.uy+y) && !test_move(u.ux, u.uy, x, y, TEST_MOVE)) {
            cmd[1] = (iflags.num_pad ? ndir[dir] : sdir[dir]);
            cmd[2] = 0;
            if (IS_DOOR(levl[u.ux+x][u.uy+y].typ)) {
                /* slight assistance to the player: choose kick/open for them */
                if (levl[u.ux+x][u.uy+y].doormask & D_LOCKED) {
                    cmd[0] = C('d');
                    return cmd;
                }
                if (levl[u.ux+x][u.uy+y].doormask & D_CLOSED) {
                    cmd[0] = 'o';
                    return cmd;
                }
            }
            if (levl[u.ux+x][u.uy+y].typ <= SCORR) {
                cmd[0] = 's';
                cmd[1] = 0;
                return cmd;
            }
        }
    } else {
        /* convert without using floating point, allowing sloppy clicking */
        if(x > 2*abs(y))
            x = 1, y = 0;
        else if(y > 2*abs(x))
            x = 0, y = 1;
        else if(x < -2*abs(y))
            x = -1, y = 0;
        else if(y < -2*abs(x))
            x = 0, y = -1;
        else
            x = sgn(x), y = sgn(y);

        if(x == 0 && y == 0)    /* map click on player to "rest" command */
            return ".";

        dir = xytod(x, y);
    }

    /* move, attack, etc. */
    cmd[1] = 0;
    if(mod == CLICK_1) {
        cmd[0] = (iflags.num_pad ? ndir[dir] : sdir[dir]);
    } else {
        cmd[0] = (iflags.num_pad ? M(ndir[dir]) :
                (sdir[dir] - 'a' + 'A')); /* run command */
    }

    return cmd;
}

static char * parse (void) {
    static char in_line[COLNO];
    int foo;
    bool prezero = false;

    multi = 0;
    flags.move = 1;
    flush_screen(1); /* Flush screen buffer. Put the cursor on the hero. */

    if (!iflags.num_pad || (foo = readchar()) == 'n')
        for (;;) {
            foo = readchar();
            if (foo >= '0' && foo <= '9') {
                multi = 10 * multi + foo - '0';
                if (multi < 0 || multi >= LARGEST_INT) multi = LARGEST_INT;
                if (multi > 9) {
                    clear_nhwindow(WIN_MESSAGE);
                    sprintf(in_line, "Count: %d", multi);
                    plines(in_line);
                    mark_synch();
                }
                last_multi = multi;
                if (!multi && foo == '0') prezero = true;
            } else break;   /* not a digit */
        }

    if (foo == '\033') {   /* esc cancels count (TH) */
        clear_nhwindow(WIN_MESSAGE);
        multi = last_multi = 0;
    } else if (foo == DOAGAIN || in_doagain) {
        multi = last_multi;
    } else {
        last_multi = multi;
        savech(0);  /* reset input queue */
        savech((char)foo);
    }

    if (multi) {
        multi--;
        save_cm = in_line;
    } else {
        save_cm = (char *)0;
    }
    in_line[0] = foo;
    in_line[1] = '\0';
    if (foo == 'g' || foo == 'G' || foo == 'm' || foo == 'M' ||
            foo == 'F' || (iflags.num_pad && (foo == '5' || foo == '-'))) {
        foo = readchar();
        savech((char)foo);
        in_line[1] = foo;
        in_line[2] = 0;
    }
    clear_nhwindow(WIN_MESSAGE);
    if (prezero) in_line[0] = '\033';
    return(in_line);
}


static void end_of_input (void) {
    if (!program_state.done_hup++ && program_state.something_worth_saving)
        (void) dosave0();
    exit_nhwindows((char *)0);
    clearlocks();
    terminate(EXIT_SUCCESS);
}


char readchar (void) {
    int sym;
    int x = u.ux, y = u.uy, mod = 0;

    if ( *readchar_queue )
        sym = *readchar_queue++;
    else
        sym = in_doagain ? pgetchar() : nh_poskey(&x, &y, &mod);

    if (sym == EOF) {
        int cnt = NR_OF_EOFS;
        /*
         * Some SYSV systems seem to return EOFs for various reasons
         * (?like when one hits break or for interrupted systemcalls?),
         * and we must see several before we quit.
         */
        do {
            clearerr(stdin);        /* omit if clearerr is undefined */
            sym = pgetchar();
        } while (--cnt && sym == EOF);
    }
    if (sym == EOF)
        end_of_input();

    if(sym == 0) {
        /* click event */
        readchar_queue = click_to_cmd(x, y, mod);
        sym = *readchar_queue++;
    }
    return((char) sym);
}

static int dotravel (void) {
    /* Keyboard travel command */
    static char cmd[2];
    coord cc;

    if (!iflags.travelcmd) return 0;
    cmd[1]=0;
    cc.x = iflags.travelcc.x;
    cc.y = iflags.travelcc.y;
    if (cc.x == -1 && cc.y == -1) {
        /* No cached destination, start attempt from current position */
        cc.x = u.ux;
        cc.y = u.uy;
    }
    pline("Where do you want to travel to?");
    if (getpos(&cc, true, "the desired destination") < 0) {
        /* user pressed ESC */
        return 0;
    }
    iflags.travelcc.x = u.tx = cc.x;
    iflags.travelcc.y = u.ty = cc.y;
    cmd[0] = CMD_TRAVEL;
    readchar_queue = cmd;
    return 0;
}

/*
 *   Parameter validator for generic yes/no function to prevent
 *   the core from sending too long a prompt string to the
 *   window port causing a buffer overflow there.
 */
char yn_function (const char *query, const char *resp, char def) {
    char qbuf[QBUFSZ];
    unsigned truncspot, reduction = sizeof(" [N]  ?") + 1;

    if (resp) reduction += strlen(resp) + sizeof(" () ");
    if (strlen(query) < (QBUFSZ - reduction))
        return (*windowprocs.win_yn_function)(query, resp, def);
    paniclog("Query truncated: ", query);
    reduction += sizeof("...");
    truncspot = QBUFSZ - reduction;
    (void) strncpy(qbuf, query, (int)truncspot);
    qbuf[truncspot] = '\0';
    strcat(qbuf,"...");
    return (*windowprocs.win_yn_function)(qbuf, resp, def);
}
