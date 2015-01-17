/* See LICENSE in the root of this project for change info */

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dungeon_util.h"
#include "move.h"
#include "align.h"
#include "apply.h"
#include "artifact.h"
#include "attrib.h"
#include "cmd.h"
#include "decl.h"
#include "detect.h"
#include "display.h"
#include "do.h"
#include "do_name.h"
#include "do_wear.h"
#include "dokick.h"
#include "dothrow.h"
#include "dungeon.h"
#include "eat.h"
#include "end.h"
#include "engrave.h"
#include "exper.h"
#include "files.h"
#include "flag.h"
#include "fountain.h"
#include "func_tab.h"
#include "global.h"
#include "hack.h"
#include "hacklib.h"
#include "invent.h"
#include "light.h"
#include "lock.h"
#include "mkobj.h"
#include "monattk.h"
#include "mondata.h"
#include "monflag.h"
#include "monst.h"
#include "monsym.h"
#include "o_init.h"
#include "obj.h"
#include "objclass.h"
#include "objnam.h"
#include "onames.h"
#include "options.h"
#include "pager.h"
#include "permonst.h"
#include "pickup.h"
#include "pline.h"
#include "pm.h"
#include "pm_props.h"
#include "polyself.h"
#include "potion.h"
#include "pray.h"
#include "prop.h"
#include "read.h"
#include "rm.h"
#include "rnd.h"
#include "save.h"
#include "shk.h"
#include "sit.h"
#include "sounds.h"
#include "spell.h"
#include "steed.h"
#include "teleport.h"
#include "timeout.h"
#include "trap.h"
#include "util.h"
#include "version.h"
#include "weapon.h"
#include "wield.h"
#include "wizard.h"
#include "you.h"
#include "youprop.h"
#include "zap.h"

/*
 * Some systems may have getchar() return EOF for various reasons, and
 * we should not quit before seeing at least NR_OF_EOFS consecutive EOFs.
 */
static const int NR_OF_EOFS = 20;

static const char* readchar_queue="";

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
void set_occupation (int (*fn)(void), const char *txt) {
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
    return(char)((phead != ptail) ? pushq[ptail++] : '\0');
}

char pgetchar (void) {               /* curtesy of aeb@cwi.nl */
    int ch;

    if(!(ch = popch()))
        ch = getchar();
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
    if (!ch)
        phead = ptail = shead = stail = 0;
    else if (shead < BSIZE)
        saveq[shead++] = ch;
}

/* here after # - now read a full-word command */
/*
 * This is currently used only by the tty port and is
 * controlled via runtime option 'extmenu'
 */
/* here after # - now show pick-list of possible commands */

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
        else
            aggravate();
    } else if (Upolyd) {
        pline("Any special ability you may have is purely reflexive.");
    } else {
        You("don't have a special ability in your normal form!");
    }
    return 0;
}

/* ^W command - wish for something */
/* Unlimited wishes for debug mode by Paul Polderman */
static int wiz_wish (void) {
    if (flags.debug) {
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
    if (flags.debug)     identify_pack(0);
    else            pline("Unavailable command '^I'.");
    return 0;
}

/* ^F command - reveal the level map and any traps on it */
static int wiz_map (void) {
    if (flags.debug) {
        struct trap *t;
        long save_Hconf = get_HConfusion(),
             save_Hhallu = u.uprops[HALLUC].intrinsic;

        set_HConfusion(0L);
        u.uprops[HALLUC].intrinsic = 0L;
        for (t = ftrap; t != 0; t = t->ntrap) {
            t->tseen = 1;
            map_trap(t, true);
        }
        do_mapping();
        set_HConfusion(save_Hconf);
        u.uprops[HALLUC].intrinsic = save_Hhallu;
    } else
        pline("Unavailable command '^F'.");
    return 0;
}

/* ^G command - generate monster(s); a count prefix will be honored */
static int wiz_genesis (void) {
    if (flags.debug)
        create_particular();
    else
        pline("Unavailable command '^G'.");
    return 0;
}

/* ^O command - display dungeon layout */
static int wiz_where (void) {
    if (flags.debug) (void) print_dungeon(false, (signed char *)0, (signed char *)0);
    else        pline("Unavailable command '^O'.");
    return 0;
}

/* ^E command - detect unseen (secret doors, traps, hidden monsters) */
static int wiz_detect (void) {
    if(flags.debug)  (void) findit();
    else        pline("Unavailable command '^E'.");
    return 0;
}

/* ^V command - level teleport */
static int wiz_level_tele (void) {
    if (flags.debug)
        level_tele();
    else
        pline("Unavailable command '^V'.");
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

/* format increased damage or chance to hit */
static char * enlght_combatinc (const char *inctyp, int incamt, int final, char *outbuf) {
    char numbuf[24];
    const char *modif, *bonus;

    if (final || flags.debug) {
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
    // have to redo this function differently; it depended heavily on tty
    fprintf(stderr, "TODO: enlightenment\n");
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
    if (flags.debug) {
        sprintf(buf, " %d", u.ualign.record);
        dump("  Your alignment was ", buf);
    }

    /*** Resistances to troubles ***/
    if (Fire_resistance()) dump(youwere, "fire resistant");
    if (Cold_resistance()) dump(youwere, "cold resistant");
    if (Sleep_resistance()) dump(youwere, "sleep resistant");
    if (Disint_resistance()) dump(youwere, "disintegration-resistant");
    if (Shock_resistance()) dump(youwere, "shock resistant");
    if (Poison_resistance()) dump(youwere, "poison resistant");
    if (Drain_resistance()) dump(youwere, "level-drain resistant");
    if (Sick_resistance()) dump(youwere, "immune to sickness");
    if (Antimagic()) dump(youwere, "magic-protected");
    if (Acid_resistance()) dump(youwere, "acid resistant");
    if (Stone_resistance()) dump(youwere, "petrification resistant");
    if (Invulnerable()) dump(youwere, "invulnerable");
    if (u.uedibility) dump(youcould, "recognize detrimental food");

    /*** Troubles ***/
    if (Halluc_resistance())  dump("  ", "You resisted hallucinations");
    if (Hallucination) dump(youwere, "hallucinating");
    if (Stunned()) dump(youwere, "stunned");
    if (Confusion()) dump(youwere, "confused");
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
    if (Fumbling()) dump("  ", "You fumbled");
    if (Wounded_legs() && !u.usteed) {
        sprintf(buf, "wounded %s", makeplural(body_part(LEG)));
        dump(youhad, buf);
    }
    if (Wounded_legs() && u.usteed) {
        x_monnam(buf, BUFSZ, u.usteed, ARTICLE_YOUR, NULL,
                SUPPRESS_SADDLE | SUPPRESS_HALLUCINATION, false);
        *buf = highc(*buf);
        strcat(buf, " had wounded legs");
        dump("  ", buf);
    }
    if (Sleeping) dump("  ", "You fell asleep");
    if (Hunger()) dump("  ", "You hungered rapidly");

    /*** Vision and senses ***/
    if (See_invisible()) dump("  ", "You saw invisible");
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
    if (Detect_monsters())
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
    if (u.usteed && (final < 2 || killer.method == KM_RIDING_ACCIDENT)) {
        /*
        char name[BUFSZ];
        y_monnam(name, BUFSZ, u.usteed);
        sprintf(buf, "riding %s", name);
        dump(youwere, buf);
        */
    }
    if (u.uswallow) {
        /*
        char name[BUFSZ];
        a_monnam(name, BUFSZ, u.ustuck);
        if (flags.debug) {
            nh_slprintf(buf, BUFSZ, "swallowed by %s (%u)", name, u.uswldtim);
        } else {
            nh_slprintf(buf, BUFSZ, "swallowed by %s", name);
        }
        dump(youwere, buf);
        */
    } else if (u.ustuck) {
        /*
        char name[BUFSZ];
        a_monnam(name, BUFSZ, u.ustuck);
        const char *held_str = (Upolyd && sticks(youmonst.data)) ? "holding" : "held by";
        nh_slprintf(buf, BUFSZ, "%s %s", held_str, name);
        dump(youwere, buf);
        */
    }

    /*** Physical attributes ***/
    if (u.uhitinc)
        dump(youhad, enlght_combatinc("to hit", u.uhitinc, final, buf));
    if (u.udaminc)
        dump(youhad, enlght_combatinc("damage", u.udaminc, final, buf));
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
        if (flags.debug) sprintf(eos(buf), " (%d)", u.mtimedone);
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
    else if (flags.debug) dump("  ", "Your luck was zero");
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
        if (flags.debug) sprintf(eos(buf), " (%d)", u.ugangr);
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
    return true;
}

static int doattributes (void) {
    if (!minimal_enlightenment())
        return 0;
    if (flags.debug)
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
}

void dump_conduct (int final) {
}

static int doprev_message(void) {
    return 0;
}

static int doextcmd(void) {
    return 0;
}

static int wiz_show_seenv(void) {
    return 0;
}

/*
 * Display memory usage of all monsters and objects on the level.
 */
static int wiz_show_stats (void) {
    return 0;
}

static int wiz_show_vision(void) {
    return 0;
}

static int wiz_show_wmodes(void) {
    return 0;
}


#define M(c) ((c) - 128)
#define C(c) (0x1f & (c))

static const struct func_tab cmdlist[] = {
    {C('d'), false, dokick},
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
    /*      'y', 'Y' : go nw */
    {'z', false, dozap},
    {'Z', true, docast},
    {'<', false, doup},
    {'>', false, dodown},
    {'/', true, dowhatis},
    {'&', true, dowhatdoes},
    {'?', true, dohelp},
    {'.', true, donull, "waiting"},
    {',', false, dopickup},
    {':', true, dolook},
    {';', true, doquickwhatis},
    {'^', true, doidtrap},
    {'\\', true, dodiscovered},             /* Robert Viduya */
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

void sanity_check (void) {
    obj_sanity_check();
    timer_sanity_check();
}


void parse(char * buf, size_t bufsize) {
    ssize_t count = read(STDIN_FILENO, buf, bufsize - 1);
    if (count < 0)
        count = 0;
    buf[count] = '\0';
}

static bool help_dir(char sym, const char *msg) {
    return true;
}

/* also sets .z, but returns false for <> */
static bool movecmd(char sym, Direction * out_direction) {
    const char *dp;
    if (!(dp = index(ndir, sym)))
        return false;
    Direction output;
    output.x = xdir[dp - ndir];
    output.y = ydir[dp - ndir];
    output.z = 0;
    if (output.x && output.y && u.umonnum == PM_GRID_BUG) {
        output.x = output.y = 0;
        return false;
    }
    *out_direction = output;
    return output.z == 0;
}

void read_inputs(void) {
    bool bad_command;

    iflags.menu_requested = false;
    flags.nopick = 0;
    char cmd_buffer[BUFSZ];
    parse(cmd_buffer, BUFSZ);
    char * cmd = cmd_buffer;
    if (*cmd == '\033') {
        flags.move = false;
        return;
    }
    if (!*cmd || *cmd == (char)0377) {
        flags.move = false;
        return; /* probably we just had an interrupt */
    }
    /* handle most movement commands */
    bool do_walk = false;
    u.dz = 0;
    switch (*cmd) {
            /* Effects of movement commands and invisible monsters:
             * m: always move onto space (even if 'I' remembered)
             * F: always attack space (even if 'I' not remembered)
             * normal movement: attack if 'I', move otherwise
             */
        case 'F':
            if (movecmd(cmd[1], &u.delta)) {
                flags.forcefight = true;
                do_walk = true;
            }
            break;
        case 'm':
            if (movecmd(cmd[1], &u.delta)) {
                flags.nopick = 1;
                do_walk = true;
            }
            break;
        default:
            /* ordinary movement */
            if (movecmd(*cmd, &u.delta)) {
                do_walk = true;
            }
            break;
    }

    if (do_walk) {
        domove();
        flags.forcefight = false;
        return;
    } else {
        for (const struct func_tab * tlist = cmdlist; tlist->f_char; tlist++) {
            if ((*cmd & 0xff) != (tlist->f_char & 0xff))
                continue;

            int res;
            if (u.uburied && !tlist->can_if_buried) {
                You_cant("do that while you are buried!");
                res = 0;
            } else {
                /* perform the command */
                res = tlist->f_funct();
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
        if (!iflags.cmdassist || !help_dir(0, "Invalid direction key!"))
            Norep("Unknown command '%s'.", expcmd);
    }
    /* didn't move */
    flags.move = false;
    multi = 0;
}

/* convert an x,y pair into a direction code */
int xytod(signed char x, signed char y) {
    for (int dd = 0; dd < 8; dd++)
        if (x == xdir[dd] && y == ydir[dd])
            return dd;
    return -1;
}

/* convert a direction code into an x,y pair */
void dtoxy(coord * cc, int dd) {
    cc->x = xdir[dd];
    cc->y = ydir[dd];
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
int get_adjacent_loc(const char *prompt, const char *emsg, signed char x, signed char y, coord *cc) {
    signed char new_x, new_y;
    if (!getdir(prompt)) {
        plines(Never_mind);
        return 0;
    }
    new_x = x + u.delta.x;
    new_y = y + u.delta.y;
    if (cc && isok(new_x, new_y)) {
        cc->x = new_x;
        cc->y = new_y;
    } else {
        if (emsg)
            plines(emsg);
        return 0;
    }
    return 1;
}

char yn_function(const char *prompt, const char *options, char default_option) {
    fprintf(stderr, "TODO: waiting on yn...\n");
    char yn[2];
    parse(yn, 2);
    const char *c = options;
    while (*c) {
        if (*c == yn[0])
            return *c;
        c += 1;
    }
    return default_option;
}

int getdir(const char *s) {
    fprintf(stderr, "TODO: getdir()\n");
    // TODO: for realz plz
    u.delta.x = u.delta.y = u.delta.z = 0;
    confdir();

    if (!u.delta.z && (Stunned() || (Confusion() && !rn2(5))))
        confdir();
    return 1;
}

void confdir (void) {
    int x = (u.umonnum == PM_GRID_BUG) ? 2*rn2(4) : rn2(8);
    u.delta.x = xdir[x];
    u.delta.y = ydir[x];
}


int isok (int x, int y) {
    /* x corresponds to curx, so x==1 is the first column. Ach. %% */
    return x >= 1 && x <= COLNO-1 && y >= 0 && y <= ROWNO-1;
}

static void end_of_input (void) {
    if (!program_state.done_hup++ && program_state.something_worth_saving)
        (void) dosave0();
    // exit_nhwindows((char *)0);
    clearlocks();
    terminate(EXIT_SUCCESS);
}


char readchar(void) {
    int sym;
    int x = u.ux, y = u.uy, mod = 0;

    if (*readchar_queue)
        sym = *readchar_queue++;
    else
        sym = getchar();

    if (sym == EOF) {
        int cnt = NR_OF_EOFS;
        /*
         * Some SYSV systems seem to return EOFs for various reasons
         * (?like when one hits break or for interrupted systemcalls?),
         * and we must see several before we quit.
         */
        do {
            clearerr(stdin); /* omit if clearerr is undefined */
            sym = pgetchar();
        } while (--cnt && sym == EOF);
    }
    if (sym == EOF)
        end_of_input();

    return (char)sym;
}
