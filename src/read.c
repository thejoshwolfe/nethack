/* See LICENSE in the root of this project for change info */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rm_util.h"
#include "display_util.h"
#include "dungeon_util.h"
#include "read.h"
#include "apply.h"
#include "attrib.h"
#include "ball.h"
#include "cmd.h"
#include "coord.h"
#include "decl.h"
#include "detect.h"
#include "display.h"
#include "do.h"
#include "do_name.h"
#include "do_wear.h"
#include "dog.h"
#include "drawing.h"
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
#include "light.h"
#include "mail.h"
#include "makemon.h"
#include "mkobj.h"
#include "mon.h"
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
#include "pline.h"
#include "pm.h"
#include "pm_props.h"
#include "polyself.h"
#include "potion.h"
#include "prop.h"
#include "questpgr.h"
#include "region.h"
#include "rm.h"
#include "rnd.h"
#include "rumors.h"
#include "shk.h"
#include "spell.h"
#include "teleport.h"
#include "timeout.h"
#include "trap.h"
#include "util.h"
#include "vision.h"
#include "weapon.h"
#include "wield.h"
#include "worn.h"
#include "you.h"
#include "youprop.h"
#include "zap.h"

/* KMH -- Copied from pray.c; this really belongs in a header file */
#define DEVOUT 14
#define STRIDENT 4

#define Your_Own_Role(mndx) \
        ((mndx) == urole.malenum || \
         (urole.femalenum != NON_PM && (mndx) == urole.femalenum))
#define Your_Own_Race(mndx) \
        ((mndx) == urace.malenum || \
         (urace.femalenum != NON_PM && (mndx) == urace.femalenum))


bool known;

static const char readable[] =
                   { ALL_CLASSES, SCROLL_CLASS, SPBOOK_CLASS, 0 };
static const char all_count[] = { ALLOW_COUNT, ALL_CLASSES, 0 };


int doread (void) {
    struct obj *scroll;
    bool confused;

    known = false;
    if(check_capacity((char *)0)) return (0);
    scroll = getobj(readable, "read");
    if(!scroll) return(0);

    /* outrumor has its own blindness check */
    if(scroll->otyp == FORTUNE_COOKIE) {
        if(flags.verbose)
            You("break up the cookie and throw away the pieces.");
        outrumor(bcsign(scroll), BY_COOKIE);
        if (!Blind()) u.uconduct.literate++;
        useup(scroll);
        return(1);
    } else if (scroll->otyp == T_SHIRT) {
        static const char *shirt_msgs[] = { /* Scott Bigham */
            "I explored the Dungeons of Doom and all I got was this lousy T-shirt!",
            "Is that Mjollnir in your pocket or are you just happy to see me?",
            "It's not the size of your sword, it's how #enhance'd you are with it.",
            "Madame Elvira's House O' Succubi Lifetime Customer",
            "Madame Elvira's House O' Succubi Employee of the Month",
            "Ludios Vault Guards Do It In Small, Dark Rooms",
            "Yendor Military Soldiers Do It In Large Groups",
            "I survived Yendor Military Boot Camp",
            "Ludios Accounting School Intra-Mural Lacrosse Team",
            "Oracle(TM) Fountains 10th Annual Wet T-Shirt Contest",
            "Hey, black dragon!  Disintegrate THIS!",
            "I'm With Stupid -->",
            "Don't blame me, I voted for Izchak!",
            "Don't Panic",                              /* HHGTTG */
            "Furinkan High School Athletic Dept.",      /* Ranma 1/2 */
            "Hel-LOOO, Nurse!",                 /* Animaniacs */
        };
        char buf[BUFSZ];
        int erosion;

        if (Blind()) {
            You_cant("feel any Braille writing.");
            return 0;
        }
        u.uconduct.literate++;
        if(flags.verbose)
            pline("It reads:");
        strcpy(buf, shirt_msgs[scroll->o_id % SIZE(shirt_msgs)]);
        erosion = greatest_erosion(scroll);
        if (erosion)
            wipeout_text(buf,
                    (int)(strlen(buf) * erosion / (2*MAX_ERODE)),
                    scroll->o_id ^ (unsigned)u.ubirthday);
        pline("\"%s\"", buf);
        return 1;
    } else if (scroll->oclass != SCROLL_CLASS
            && scroll->oclass != SPBOOK_CLASS) {
        pline(silly_thing_to, "read");
        return(0);
    } else if (Blind()) {
        const char *what = 0;
        if (scroll->oclass == SPBOOK_CLASS)
            what = "mystic runes";
        else if (!scroll->dknown)
            what = "formula on the scroll";
        if (what) {
            pline("Being blind, you cannot read the %s.", what);
            return(0);
        }
    }

    /* Actions required to win the game aren't counted towards conduct */
    if (scroll->otyp != SPE_BOOK_OF_THE_DEAD &&
            scroll->otyp != SPE_BLANK_PAPER &&
            scroll->otyp != SCR_BLANK_PAPER)
        u.uconduct.literate++;

    confused = (Confusion());
    if (scroll->otyp == SCR_MAIL) confused = false;
    if(scroll->oclass == SPBOOK_CLASS) {
        return(study_book(scroll));
    }
    scroll->in_use = true;  /* scroll, not spellbook, now being read */
    if(scroll->otyp != SCR_BLANK_PAPER) {
        if(Blind())
            pline("As you %s the formula on it, the scroll disappears.",
                    is_silent(youmonst.data) ? "cogitate" : "pronounce");
        else
            pline("As you read the scroll, it disappears.");
        if(confused) {
            if (Hallucination())
                pline("Being so trippy, you screw up...");
            else
                pline("Being confused, you mis%s the magic words...",
                        is_silent(youmonst.data) ? "understand" : "pronounce");
        }
    }
    if(!seffects(scroll))  {
        if(!objects[scroll->otyp].oc_name_known) {
            if(known) {
                makeknown(scroll->otyp);
                more_experienced(0,10);
            } else if(!objects[scroll->otyp].oc_uname)
                docall(scroll);
        }
        if(scroll->otyp != SCR_BLANK_PAPER)
            useup(scroll);
        else scroll->in_use = false;
    }
    return(1);
}

static void stripspe (struct obj *obj) {
    if (obj->blessed) {
        message_const(MSG_NOTHING_HAPPENS);
    } else {
        if (obj->spe > 0) {
            obj->spe = 0;
            if (obj->otyp == OIL_LAMP || obj->otyp == BRASS_LANTERN)
                obj->age = 0;
            message_object(MSG_YOUR_O_VIBRATES_BRIEFLY, obj);
        } else {
            message_const(MSG_NOTHING_HAPPENS);
        }
    }
}

static void p_glow1 (struct obj *otmp) {
    message_object(Blind() ? MSG_YOUR_O_VIBRATES_BRIEFLY : MSG_YOUR_O_GLOWS_BRIEFLY, otmp);
}

static void p_glow2 (struct obj *otmp, const char *color) {
    if (Blind()) {
        message_object(MSG_YOUR_O_VIBRATES_FOR_MOMENT, otmp);
    } else {
        fprintf(stderr, "TODO: your object glows color for a moment\n");
        // Your("%s %s %s for a moment.", xname(otmp), otense(otmp, "glow"), hcolor(color));
    }
}

/* Is the object chargeable?  For purposes of inventory display; it is */
/* possible to be able to charge things for which this returns false. */
bool 
is_chargeable (struct obj *obj)
{
        if (obj->oclass == WAND_CLASS) return true;
        /* known && !uname is possible after amnesia/mind flayer */
        if (obj->oclass == RING_CLASS)
            return (bool)(objects[obj->otyp].oc_charged &&
                        (obj->known || objects[obj->otyp].oc_uname));
        if (is_weptool(obj))    /* specific check before general tools */
            return false;
        if (obj->oclass == TOOL_CLASS)
            return (bool)(objects[obj->otyp].oc_charged);
        return false; /* why are weapons/armor considered charged anyway? */
}

static void wand_explode (struct obj *obj) {
    obj->in_use = true; /* in case losehp() is fatal */
    message_object(MSG_YOUR_O_VIBRATES_VIOLENTLY_AND_EXPLODES, obj);
    losehp(rnd(2*(u.uhpmax+1)/3), killed_by_const(KM_EXPLODING_WAND));
    useup(obj);
    exercise(A_STR, false);
}


/*
 * recharge an object; curse_bless is -1 if the recharging implement
 * was cursed, +1 if blessed, 0 otherwise.
 */
void recharge (struct obj *obj, int curse_bless) {
    int n;
    bool is_cursed, is_blessed;

    is_cursed = curse_bless < 0;
    is_blessed = curse_bless > 0;

    if (obj->oclass == WAND_CLASS) {
        /* undo any prior cancellation, even when is_cursed */
        if (obj->spe == -1) obj->spe = 0;

        /*
         * Recharging might cause wands to explode.
         *  v = number of previous recharges
         *        v = percentage chance to explode on this attempt
         *                v = cumulative odds for exploding
         *  0 :   0       0
         *  1 :   0.29    0.29
         *  2 :   2.33    2.62
         *  3 :   7.87   10.28
         *  4 :  18.66   27.02
         *  5 :  36.44   53.62
         *  6 :  62.97   82.83
         *  7 : 100     100
         */
        n = (int)obj->recharged;
        if (n > 0 && (obj->otyp == WAN_WISHING ||
                    (n * n * n > rn2(7*7*7)))) {        /* recharge_limit */
            wand_explode(obj);
            return;
        }
        /* didn't explode, so increment the recharge count */
        obj->recharged = (unsigned)(n + 1);

        /* now handle the actual recharging */
        if (is_cursed) {
            stripspe(obj);
        } else {
            int lim = (obj->otyp == WAN_WISHING) ? 3 :
                (objects[obj->otyp].oc_dir != NODIR) ? 8 : 15;

            n = (lim == 3) ? 3 : rn1(5, lim + 1 - 5);
            if (!is_blessed) n = rnd(n);

            if (obj->spe < n) obj->spe = n;
            else obj->spe++;
            if (obj->otyp == WAN_WISHING && obj->spe > 3) {
                wand_explode(obj);
                return;
            }
            if (obj->spe >= lim) p_glow2(obj, NH_BLUE);
            else p_glow1(obj);
        }

    } else if (obj->oclass == RING_CLASS &&
            objects[obj->otyp].oc_charged) {
        /* charging does not affect ring's curse/bless status */
        int s = is_blessed ? rnd(3) : is_cursed ? -rnd(2) : 1;
        bool is_on = (obj == uleft || obj == uright);

        /* destruction depends on current state, not adjustment */
        if (obj->spe > rn2(7) || obj->spe <= -5) {
            message_object(MSG_O_PULSATES_THEN_EXPLODES, obj);
            if (is_on) Ring_gone(obj);
            s = rnd(3 * abs(obj->spe));     /* amount of damage */
            useup(obj);
            losehp(s, killed_by_const(KM_EXPLODING_RING));
        } else {
            long mask = is_on ? (obj == uleft ? LEFT_RING :
                    RIGHT_RING) : 0L;
            message_object((s < 0) ? MSG_O_SPINS_COUNTER_CLOCKWISE : MSG_O_SPINS_CLOCKWISE, obj);
            /* cause attributes and/or properties to be updated */
            if (is_on) Ring_off(obj);
            obj->spe += s;  /* update the ring while it's off */
            if (is_on) setworn(obj, mask), Ring_on(obj);
            /* oartifact: if a touch-sensitive artifact ring is
               ever created the above will need to be revised  */
        }

    } else if (obj->oclass == TOOL_CLASS) {
        int rechrg = (int)obj->recharged;

        if (objects[obj->otyp].oc_charged) {
            /* tools don't have a limit, but the counter used does */
            if (rechrg < 7) /* recharge_limit */
                obj->recharged++;
        }
        switch(obj->otyp) {
            case BELL_OF_OPENING:
                if (is_cursed) stripspe(obj);
                else if (is_blessed) obj->spe += rnd(3);
                else obj->spe += 1;
                if (obj->spe > 5) obj->spe = 5;
                break;
            case MAGIC_MARKER:
            case TINNING_KIT:
            case EXPENSIVE_CAMERA:
                if (is_cursed) stripspe(obj);
                else if (rechrg && obj->otyp == MAGIC_MARKER) { /* previously recharged */
                    obj->recharged = 1; /* override increment done above */
                    if (obj->spe < 3)
                        Your("marker seems permanently dried out.");
                    else
                        message_const(MSG_NOTHING_HAPPENS);
                } else if (is_blessed) {
                    n = rn1(16,15);             /* 15..30 */
                    if (obj->spe + n <= 50)
                        obj->spe = 50;
                    else if (obj->spe + n <= 75)
                        obj->spe = 75;
                    else {
                        int chrg = (int)obj->spe;
                        if ((chrg + n) > 127)
                            obj->spe = 127;
                        else
                            obj->spe += n;
                    }
                    p_glow2(obj, NH_BLUE);
                } else {
                    n = rn1(11,10);             /* 10..20 */
                    if (obj->spe + n <= 50)
                        obj->spe = 50;
                    else {
                        int chrg = (int)obj->spe;
                        if ((chrg + n) > 127)
                            obj->spe = 127;
                        else
                            obj->spe += n;
                    }
                    p_glow2(obj, NH_WHITE);
                }
                break;
            case OIL_LAMP:
            case BRASS_LANTERN:
                if (is_cursed) {
                    stripspe(obj);
                    if (obj->lamplit) {
                        if (!Blind()) {
                            message_object(MSG_O_GOES_OUT, obj);
                        }
                        end_burn(obj, true);
                    }
                } else if (is_blessed) {
                    obj->spe = 1;
                    obj->age = 1500;
                    p_glow2(obj, NH_BLUE);
                } else {
                    obj->spe = 1;
                    obj->age += 750;
                    if (obj->age > 1500) obj->age = 1500;
                    p_glow1(obj);
                }
                break;
            case CRYSTAL_BALL:
                if (is_cursed) stripspe(obj);
                else if (is_blessed) {
                    obj->spe = 6;
                    p_glow2(obj, NH_BLUE);
                } else {
                    if (obj->spe < 5) {
                        obj->spe++;
                        p_glow1(obj);
                    } else {
                        message_const(MSG_NOTHING_HAPPENS);
                    }
                }
                break;
            case HORN_OF_PLENTY:
            case BAG_OF_TRICKS:
            case CAN_OF_GREASE:
                if (is_cursed) stripspe(obj);
                else if (is_blessed) {
                    if (obj->spe <= 10)
                        obj->spe += rn1(10, 6);
                    else obj->spe += rn1(5, 6);
                    if (obj->spe > 50) obj->spe = 50;
                    p_glow2(obj, NH_BLUE);
                } else {
                    obj->spe += rnd(5);
                    if (obj->spe > 50) obj->spe = 50;
                    p_glow1(obj);
                }
                break;
            case MAGIC_FLUTE:
            case MAGIC_HARP:
            case FROST_HORN:
            case FIRE_HORN:
            case DRUM_OF_EARTHQUAKE:
                if (is_cursed) {
                    stripspe(obj);
                } else if (is_blessed) {
                    obj->spe += d(2,4);
                    if (obj->spe > 20) obj->spe = 20;
                    p_glow2(obj, NH_BLUE);
                } else {
                    obj->spe += rnd(4);
                    if (obj->spe > 20) obj->spe = 20;
                    p_glow1(obj);
                }
                break;
            default:
                goto not_chargable;
                /*NOTREACHED*/
                break;
        } /* switch */

    } else {
not_chargable:
        You("have a feeling of loss.");
    }
}


/* Forget known information about this object class. */
static void
forget_single_object (int obj_id)
{
        objects[obj_id].oc_name_known = 0;
        objects[obj_id].oc_pre_discovered = 0;  /* a discovery when relearned */
        if (objects[obj_id].oc_uname) {
            free((void *)objects[obj_id].oc_uname);
            objects[obj_id].oc_uname = 0;
        }
        undiscover_object(obj_id);      /* after clearing oc_name_known */

        /* clear & free object names from matching inventory items too? */
}



/* randomize the given list of numbers  0 <= i < count */
static void
randomize (int *indices, int count)
{
        int i, iswap, temp;

        for (i = count - 1; i > 0; i--) {
            if ((iswap = rn2(i + 1)) == i) continue;
            temp = indices[i];
            indices[i] = indices[iswap];
            indices[iswap] = temp;
        }
}


/* Forget % of known objects. */
void
forget_objects (int percent)
{
        int i, count;
        int indices[NUM_OBJECTS];

        if (percent == 0) return;
        if (percent <= 0 || percent > 100) {
            impossible("forget_objects: bad percent %d", percent);
            return;
        }

        for (count = 0, i = 1; i < NUM_OBJECTS; i++)
            if (OBJ_DESCR(objects[i]) &&
                    (objects[i].oc_name_known || objects[i].oc_uname))
                indices[count++] = i;

        randomize(indices, count);

        /* forget first % of randomized indices */
        count = ((count * percent) + 50) / 100;
        for (i = 0; i < count; i++)
            forget_single_object(indices[i]);
}


/* Forget some or all of map (depends on parameters). */
void
forget_map (int howmuch)
{
        int zx, zy;

        if (In_sokoban(&u.uz))
            return;

        known = true;
        for(zx = 0; zx < COLNO; zx++) for(zy = 0; zy < ROWNO; zy++)
            if ((howmuch & ALL_MAP) || rn2(7)) {
                /* Zonk all memory of this location. */
                levl[zx][zy].seenv = 0;
                levl[zx][zy].waslit = 0;
                levl[zx][zy].glyph = cmap_to_glyph(S_stone);
            }
}

/* Forget all traps on the level. */
void
forget_traps (void)
{
        struct trap *trap;

        /* forget all traps (except the one the hero is in :-) */
        for (trap = ftrap; trap; trap = trap->ntrap)
            if ((trap->tx != u.ux || trap->ty != u.uy) && (trap->ttyp != HOLE))
                trap->tseen = 0;
}

/*
 * Forget given % of all levels that the hero has visited and not forgotten,
 * except this one.
 */
void
forget_levels (int percent)
{
        int i, count;
        signed char  maxl, this_lev;
        int indices[MAXLINFO];

        if (percent == 0) return;

        if (percent <= 0 || percent > 100) {
            impossible("forget_levels: bad percent %d", percent);
            return;
        }

        this_lev = ledger_no(&u.uz);
        maxl = maxledgerno();

        /* count & save indices of non-forgotten visited levels */
        /* Sokoban levels are pre-mapped for the player, and should stay
         * so, or they become nearly impossible to solve.  But try to
         * shift the forgetting elsewhere by fiddling with percent
         * instead of forgetting fewer levels.
         */
        for (count = 0, i = 0; i <= maxl; i++)
            if ((level_info[i].flags & VISITED) &&
                        !(level_info[i].flags & FORGOTTEN) && i != this_lev) {
                if (ledger_to_dnum(i) == sokoban_dnum)
                    percent += 2;
                else
                    indices[count++] = i;
            }

        if (percent > 100) percent = 100;

        randomize(indices, count);

        /* forget first % of randomized indices */
        count = ((count * percent) + 50) / 100;
        for (i = 0; i < count; i++) {
            level_info[indices[i]].flags |= FORGOTTEN;
        }
}

/*
 * Forget some things (e.g. after reading a scroll of amnesia).  When called,
 * the following are always forgotten:
 *
 *      - felt ball & chain
 *      - traps
 *      - part (6 out of 7) of the map
 *
 * Other things are subject to flags:
 *
 *      howmuch & ALL_MAP       = forget whole map
 *      howmuch & ALL_SPELLS    = forget all spells
 */
static void forget (int howmuch) {

        if (Punished) u.bc_felt = 0;    /* forget felt ball&chain */

        forget_map(howmuch);
        forget_traps();

        /* 1 in 3 chance of forgetting some levels */
        if (!rn2(3)) forget_levels(rn2(25));

        /* 1 in 3 chance of forgeting some objects */
        if (!rn2(3)) forget_objects(rn2(25));

        if (howmuch & ALL_SPELLS) losespells();
        /*
         * Make sure that what was seen is restored correctly.  To do this,
         * we need to go blind for an instant --- turn off the display,
         * then restart it.  All this work is needed to correctly handle
         * walls which are stone on one side and wall on the other.  Turning
         * off the seen bits above will make the wall revert to stone,  but
         * there are cases where we don't want this to happen.  The easiest
         * thing to do is to run it through the vision system again, which
         * is always correct.
         */
        docrt();                /* this correctly will reset vision */
}

/* monster is hit by scroll of taming's effect */
static void maybe_tame (struct monst *mtmp, struct obj *sobj) {
    if (sobj->cursed) {
        setmangry(mtmp);
    } else {
        if (mtmp->isshk)
            make_happy_shk(mtmp, false);
        else if (!resist(mtmp, sobj->oclass, 0, NOTELL))
            (void) tamedog(mtmp, (struct obj *) 0);
    }
}

static void do_class_genocide (void) {
    int i, j, immunecnt, gonecnt, goodcnt, class, feel_dead = 0;
    char buf[BUFSZ];
    bool gameover = false;       /* true iff killed self */

    for(j=0; ; j++) {
        if (j >= 5) {
            plines(thats_enough_tries);
            return;
        }
        do {
            getlin("What class of monsters do you wish to genocide?",
                    buf);
            (void)mungspaces(buf);
        } while (buf[0]=='\033' || !buf[0]);
        /* choosing "none" preserves genocideless conduct */
        if (!strcmpi(buf, "none") ||
                !strcmpi(buf, "nothing")) return;

        if (strlen(buf) == 1) {
            if (buf[0] == ILLOBJ_SYM)
                buf[0] = def_monsyms[S_MIMIC];
            class = def_char_to_monclass(buf[0]);
        } else {
            char buf2[BUFSZ];

            class = 0;
            strcpy(buf2, makesingular(buf));
            strcpy(buf, buf2);
        }
        immunecnt = gonecnt = goodcnt = 0;
        for (i = LOW_PM; i < NUMMONS; i++) {
            if (class == 0 &&
                    strstri(monexplain[(int)mons[i].mlet], buf) != 0)
                class = mons[i].mlet;
            if (mons[i].mlet == class) {
                if (!(mons[i].geno & G_GENO)) immunecnt++;
                else if(mvitals[i].mvflags & G_GENOD) gonecnt++;
                else goodcnt++;
            }
        }
        /*
         * TODO[?]: If user's input doesn't match any class
         *          description, check individual species names.
         */
        if (!goodcnt && class != mons[urole.malenum].mlet &&
                class != mons[urace.malenum].mlet) {
            if (gonecnt)
                pline("All such monsters are already nonexistent.");
            else if (immunecnt ||
                    (buf[0] == DEF_INVISIBLE && buf[1] == '\0'))
                You("aren't permitted to genocide such monsters.");
            else
                if (flags.debug && buf[0] == '*') {
                    struct monst *mtmp, *mtmp2;

                    gonecnt = 0;
                    for (mtmp = fmon; mtmp; mtmp = mtmp2) {
                        mtmp2 = mtmp->nmon;
                        if (DEADMONSTER(mtmp)) continue;
                        mongone(mtmp);
                        gonecnt++;
                    }
                    pline("Eliminated %d monster%s.", gonecnt, plur(gonecnt));
                    return;
                } else
                    pline("That symbol does not represent any monster.");
            continue;
        }

        for (i = LOW_PM; i < NUMMONS; i++) {
            if(mons[i].mlet == class) {
                char nam[BUFSZ];

                strcpy(nam, makeplural(mons[i].mname));
                /* Although "genus" is Latin for race, the hero benefits
                 * from both race and role; thus genocide affects either.
                 */
                if (Your_Own_Role(i) || Your_Own_Race(i) ||
                        ((mons[i].geno & G_GENO)
                         && !(mvitals[i].mvflags & G_GENOD))) {
                    /* This check must be first since player monsters might
                     * have G_GENOD or !G_GENO.
                     */
                    mvitals[i].mvflags |= (G_GENOD|G_NOCORPSE);
                    reset_rndmonst(i);
                    kill_genocided_monsters();
                    pline("Wiped out all %s.", nam);
                    if (Upolyd && i == u.umonnum) {
                        u.mh = -1;
                        if (Unchanging) {
                            if (!feel_dead++) You("die.");
                            /* finish genociding this class of
                               monsters before ultimately dying */
                            gameover = true;
                        } else
                            rehumanize();
                    }
                    /* Self-genocide if it matches either your race
                       or role.  Assumption:  male and female forms
                       share same monster class. */
                    if (i == urole.malenum || i == urace.malenum) {
                        u.uhp = -1;
                        if (Upolyd) {
                            if (!feel_dead++) You_feel("dead inside.");
                        } else {
                            if (!feel_dead++) You("die.");
                            gameover = true;
                        }
                    }
                } else if (mvitals[i].mvflags & G_GENOD) {
                    if (!gameover)
                        pline("All %s are already nonexistent.", nam);
                } else if (!gameover) {
                    /* suppress feedback about quest beings except
                       for those applicable to our own role */
                    if ((mons[i].msound != MS_LEADER ||
                                quest_info(MS_LEADER) == i)
                            && (mons[i].msound != MS_NEMESIS ||
                                quest_info(MS_NEMESIS) == i)
                            && (mons[i].msound != MS_GUARDIAN ||
                                quest_info(MS_GUARDIAN) == i)
                            /* non-leader/nemesis/guardian role-specific monster */
                            && (i != PM_NINJA ||         /* nuisance */
                                Role_if(PM_SAMURAI))) {
                        bool named, uniq;

                        named = type_is_pname(&mons[i]) ? true : false;
                        uniq = (mons[i].geno & G_UNIQ) ? true : false;
                        /* one special case */
                        if (i == PM_HIGH_PRIEST) uniq = false;

                        You("aren't permitted to genocide %s%s.",
                                (uniq && !named) ? "the " : "",
                                (uniq || named) ? mons[i].mname : nam);
                    }
                }
            }
        }
        if (gameover || u.uhp == -1) {
            killer = killed_by_const(KM_SCROLL_OF_GENOCIDE);
            if (gameover)
                done(GENOCIDED);
        }
        return;
    }
}


int seffects (struct obj *sobj) {
    int cval;
    bool confused = (Confusion());
    struct obj *otmp;

    if (objects[sobj->otyp].oc_magic)
        exercise(A_WIS, true);          /* just for trying */
    switch(sobj->otyp) {
        case SCR_MAIL:
            known = true;
            if (sobj->spe)
                pline("This seems to be junk mail addressed to the finder of the Eye of Larn.");
            /* note to the puzzled: the game Larn actually sends you junk
             * mail if you win!
             */
            else readmail(sobj);
            break;
        case SCR_ENCHANT_ARMOR:
            {
                signed char s;
                bool special_armor;
                bool same_color;

                otmp = some_armor(&youmonst);
                if(!otmp) {
                    strange_feeling(sobj,
                            !Blind() ? "Your skin glows then fades." :
                            "Your skin feels warm for a moment.");
                    exercise(A_CON, !sobj->cursed);
                    exercise(A_STR, !sobj->cursed);
                    return(1);
                }
                if(confused) {
                    otmp->oerodeproof = !(sobj->cursed);
                    if(Blind()) {
                        otmp->rknown = false;
                        message_object(MSG_YOUR_O_FEEL_WARM_MOMENT, otmp);
                    } else {
                        otmp->rknown = true;
                        message_object(sobj->cursed ?  MSG_O_ARE_COVERED_BY_BLACK_GLOW :
                                MSG_O_ARE_COVERED_BY_GOLDEN_GLOW, otmp);
                    }
                    if (otmp->oerodeproof && (otmp->oeroded || otmp->oeroded2)) {
                        otmp->oeroded = otmp->oeroded2 = 0;
                        message_object(MSG_O_LOOK_AS_GOOD_AS_NEW, otmp);
                    }
                    break;
                }
                /* elven armor vibrates warningly when enchanted beyond a limit */
                special_armor = is_elven_armor(otmp) ||
                    (Role_if(PM_WIZARD) && otmp->otyp == CORNUTHAUM);
                if (sobj->cursed)
                    same_color =
                        (otmp->otyp == BLACK_DRAGON_SCALE_MAIL ||
                         otmp->otyp == BLACK_DRAGON_SCALES);
                else
                    same_color =
                        (otmp->otyp == SILVER_DRAGON_SCALE_MAIL ||
                         otmp->otyp == SILVER_DRAGON_SCALES ||
                         otmp->otyp == SHIELD_OF_REFLECTION);
                if (Blind()) same_color = false;

                /* KMH -- catch underflow */
                s = sobj->cursed ? -otmp->spe : otmp->spe;
                if (s > (special_armor ? 5 : 3) && rn2(s)) {
                    fprintf(stderr, "TODO: your armor glows violently for a while then evaporates\n");
                    /*
                    Your("%s violently %s%s%s for a while, then %s.",
                            xname(otmp),
                            otense(otmp, Blind() ? "vibrate" : "glow"),
                            (!Blind() && !same_color) ? " " : nul,
                            (Blind() || same_color) ? nul :
                            hcolor(sobj->cursed ? NH_BLACK : NH_SILVER),
                            otense(otmp, "evaporate"));
                            */
                    if (is_cloak(otmp)) Cloak_off();
                    if (is_boots(otmp)) Boots_off();
                    if (is_helmet(otmp)) Helmet_off();
                    if (is_gloves(otmp)) Gloves_off();
                    if (is_shield(otmp)) Shield_off();
                    if (otmp == uarm) Armor_gone();
                    useup(otmp);
                    break;
                }
                s = sobj->cursed ? -1 :
                    otmp->spe >= 9 ? (rn2(otmp->spe) == 0) :
                    sobj->blessed ? rnd(3-otmp->spe/3) : 1;
                if (s >= 0 && otmp->otyp >= GRAY_DRAGON_SCALES &&
                        otmp->otyp <= YELLOW_DRAGON_SCALES) {
                    /* dragon scales get turned into dragon scale mail */
                    message_object(MSG_O_MERGES_AND_HARDENS, otmp);
                    setworn((struct obj *)0, W_ARM);
                    /* assumes same order */
                    otmp->otyp = GRAY_DRAGON_SCALE_MAIL +
                        otmp->otyp - GRAY_DRAGON_SCALES;
                    otmp->cursed = 0;
                    if (sobj->blessed) {
                        otmp->spe++;
                        otmp->blessed = 1;
                    }
                    otmp->known = 1;
                    setworn(otmp, W_ARM);
                    break;
                }
                fprintf(stderr, "TODO: your pack glows for a while\n");
                /*
                Your("%s %s%s%s%s for a %s.",
                        xname(otmp),
                        s == 0 ? "violently " : nul,
                        otense(otmp, Blind() ? "vibrate" : "glow"),
                        (!Blind() && !same_color) ? " " : nul,
                        (Blind() || same_color) ? nul : hcolor(sobj->cursed ? NH_BLACK : NH_SILVER),
                        (s*s>1) ? "while" : "moment");
                        */
                otmp->cursed = sobj->cursed;
                if (!otmp->blessed || sobj->cursed)
                    otmp->blessed = sobj->blessed;
                if (s) {
                    otmp->spe += s;
                    adj_abon(otmp, s);
                    known = otmp->known;
                }

                if ((otmp->spe > (special_armor ? 5 : 3)) && (special_armor || !rn2(7)))
                    message_object(MSG_YOUR_O_SUDDENLY_VIBRATES_UNEXPECTEDLY, otmp);
                break;
            }
        case SCR_DESTROY_ARMOR:
            {
                otmp = some_armor(&youmonst);
                if(confused) {
                    if(!otmp) {
                        strange_feeling(sobj,"Your bones itch.");
                        exercise(A_STR, false);
                        exercise(A_CON, false);
                        return(1);
                    }
                    otmp->oerodeproof = sobj->cursed;
                    p_glow2(otmp, NH_PURPLE);
                    break;
                }
                if(!sobj->cursed || !otmp || !otmp->cursed) {
                    if(!destroy_arm(otmp)) {
                        strange_feeling(sobj,"Your skin itches.");
                        exercise(A_STR, false);
                        exercise(A_CON, false);
                        return(1);
                    } else
                        known = true;
                } else {        /* armor and scroll both cursed */
                    message_object(MSG_YOUR_O_VIBRATES, otmp);
                    if (otmp->spe >= -6) otmp->spe--;
                    make_stunned(get_HStun() + rn1(10, 10), true);
                }
            }
            break;
        case SCR_CONFUSE_MONSTER:
        case SPE_CONFUSE_MONSTER:
            if(youmonst.data->mlet != S_HUMAN || sobj->cursed) {
                if(!get_HConfusion()) You_feel("confused.");
                make_confused(get_HConfusion() + rnd(100),false);
            } else  if(confused) {
                if(!sobj->blessed) {
                    Your("%s begin to %s%s.",
                            makeplural(body_part(HAND)),
                            Blind() ? "tingle" : "glow ",
                                    Blind() ? nul : hcolor(NH_PURPLE));
                    make_confused(get_HConfusion() + rnd(100),false);
                } else {
                    pline("A %s%s surrounds your %s.",
                            Blind() ? nul : hcolor(NH_RED),
                                    Blind() ? "faint buzz" : " glow",
                            body_part(HEAD));
                    make_confused(0L,true);
                }
            } else {
                if (!sobj->blessed) {
                    Your("%s%s %s%s.",
                            makeplural(body_part(HAND)),
                            Blind() ? "" : " begin to glow",
                                    Blind() ? (const char *)"tingle" : hcolor(NH_RED),
                            u.umconf ? " even more" : "");
                    u.umconf++;
                } else {
                    if (Blind())
                        Your("%s tingle %s sharply.",
                                makeplural(body_part(HAND)),
                                u.umconf ? "even more" : "very");
                    else
                        Your("%s glow a%s brilliant %s.",
                                makeplural(body_part(HAND)),
                                u.umconf ? "n even more" : "",
                                hcolor(NH_RED));
                    /* after a while, repeated uses become less effective */
                    if (u.umconf >= 40)
                        u.umconf++;
                    else
                        u.umconf += rn1(8, 2);
                }
            }
            break;
        case SCR_SCARE_MONSTER:
        case SPE_CAUSE_FEAR:
            {   int ct = 0;
                struct monst *mtmp;

                for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
                    if (DEADMONSTER(mtmp)) continue;
                    if(cansee(mtmp->mx,mtmp->my)) {
                        if(confused || sobj->cursed) {
                            mtmp->mflee = mtmp->mfrozen = mtmp->msleeping = 0;
                            mtmp->mcanmove = 1;
                        } else
                            if (! resist(mtmp, sobj->oclass, 0, NOTELL))
                                monflee(mtmp, 0, false, false);
                        if(!mtmp->mtame) ct++;  /* pets don't laugh at you */
                    }
                }
                if(!ct)
                    You_hear("%s in the distance.",
                            (confused || sobj->cursed) ? "sad wailing" :
                            "maniacal laughter");
                else if(sobj->otyp == SCR_SCARE_MONSTER)
                    You_hear("%s close by.",
                            (confused || sobj->cursed) ? "sad wailing" :
                            "maniacal laughter");
                break;
            }
        case SCR_BLANK_PAPER:
            if (Blind())
                You("don't remember there being any magic words on this scroll.");
            else
                pline("This scroll seems to be blank.");
            known = true;
            break;
        case SCR_REMOVE_CURSE:
        case SPE_REMOVE_CURSE:
            {   struct obj *obj;
                if(confused)
                    if (Hallucination())
                        You_feel("the power of the Force against you!");
                    else
                        You_feel("like you need some help.");
                else
                    if (Hallucination())
                        You_feel("in touch with the Universal Oneness.");
                    else
                        You_feel("like someone is helping you.");

                if (sobj->cursed) {
                    pline_The("scroll disintegrates.");
                } else {
                    for (obj = invent; obj; obj = obj->nobj) {
                        long wornmask;
                        wornmask = (obj->owornmask & ~(W_BALL|W_ART|W_ARTI));
                        if (wornmask && !sobj->blessed) {
                            /* handle a couple of special cases; we don't
                               allow auxiliary weapon slots to be used to
                               artificially increase number of worn items */
                            if (obj == uswapwep) {
                                if (!u.twoweap) wornmask = 0L;
                            } else if (obj == uquiver) {
                                if (obj->oclass == WEAPON_CLASS) {
                                    /* mergeable weapon test covers ammo,
                                       missiles, spears, daggers & knives */
                                    if (!objects[obj->otyp].oc_merge)
                                        wornmask = 0L;
                                } else if (obj->oclass == GEM_CLASS) {
                                    /* possibly ought to check whether
                                       alternate weapon is a sling... */
                                    if (!uslinging()) wornmask = 0L;
                                } else {
                                    /* weptools don't merge and aren't
                                       reasonable quivered weapons */
                                    wornmask = 0L;
                                }
                            }
                        }
                        if (sobj->blessed || wornmask ||
                                obj->otyp == LOADSTONE ||
                                (obj->otyp == LEASH && obj->leashmon)) {
                            if(confused) blessorcurse(obj, 2);
                            else uncurse(obj);
                        }
                    }
                }
                if(Punished && !confused) unpunish();
                break;
            }
        case SCR_CREATE_MONSTER:
        case SPE_CREATE_MONSTER:
            if (create_critters(1 + ((confused || sobj->cursed) ? 12 : 0) +
                        ((sobj->blessed || rn2(73)) ? 0 : rnd(4)),
                        confused ? &mons[PM_ACID_BLOB] : (struct permonst *)0))
                known = true;
            /* no need to flush monsters; we ask for identification only if the
             * monsters are not visible
             */
            break;
        case SCR_ENCHANT_WEAPON:
            if(uwep && (uwep->oclass == WEAPON_CLASS || is_weptool(uwep))
                    && confused) {
                /* oclass check added 10/25/86 GAN */
                uwep->oerodeproof = !(sobj->cursed);
                if (Blind()) {
                    uwep->rknown = false;
                    Your("weapon feels warm for a moment.");
                } else {
                    uwep->rknown = true;
                    Your("%s covered by a %s %s %s!",
                            aobjnam(uwep, "are"),
                            sobj->cursed ? "mottled" : "shimmering",
                            hcolor(sobj->cursed ? NH_PURPLE : NH_GOLDEN),
                            sobj->cursed ? "glow" : "shield");
                }
                if (uwep->oerodeproof && (uwep->oeroded || uwep->oeroded2)) {
                    uwep->oeroded = uwep->oeroded2 = 0;
                    Your("%s as good as new!",
                            aobjnam(uwep, Blind() ? "feel" : "look"));
                }
            } else return !chwepon(sobj,
                    sobj->cursed ? -1 :
                    !uwep ? 1 :
                    uwep->spe >= 9 ? (rn2(uwep->spe) == 0) :
                    sobj->blessed ? rnd(3-uwep->spe/3) : 1);
            break;
        case SCR_TAMING:
        case SPE_CHARM_MONSTER:
            if (u.uswallow) {
                maybe_tame(u.ustuck, sobj);
            } else {
                int i, j, bd = confused ? 5 : 1;
                struct monst *mtmp;

                for (i = -bd; i <= bd; i++) for(j = -bd; j <= bd; j++) {
                    if (!isok(u.ux + i, u.uy + j)) continue;
                    if ((mtmp = m_at(u.ux + i, u.uy + j)) != 0)
                        maybe_tame(mtmp, sobj);
                }
            }
            break;
        case SCR_GENOCIDE:
            You("have found a scroll of genocide!");
            known = true;
            if (sobj->blessed) do_class_genocide();
            else do_genocide(!sobj->cursed | (2 * !!Confusion()));
            break;
        case SCR_LIGHT:
            if(!Blind()) known = true;
            litroom(!confused && !sobj->cursed, sobj);
            break;
        case SCR_TELEPORTATION:
            if(confused || sobj->cursed) level_tele();
            else {
                if (sobj->blessed && !Teleport_control) {
                    known = true;
                    if (yn("Do you wish to teleport?")=='n')
                        break;
                }
                tele();
                if(Teleport_control || !couldsee(u.ux0, u.uy0) ||
                        (distu(u.ux0, u.uy0) >= 16))
                    known = true;
            }
            break;
        case SCR_GOLD_DETECTION:
            if (confused || sobj->cursed) return(trap_detect(sobj));
            else return(gold_detect(sobj));
        case SCR_FOOD_DETECTION:
        case SPE_DETECT_FOOD:
            if (food_detect(sobj))
                return(1);      /* nothing detected */
            break;
        case SPE_IDENTIFY:
            cval = rn2(5);
            goto id;
        case SCR_IDENTIFY:
            /* known = true; */
            if(confused)
                You("identify this as an identify scroll.");
            else
                pline("This is an identify scroll.");
            if (sobj->blessed || (!sobj->cursed && !rn2(5))) {
                cval = rn2(5);
                /* Note: if rn2(5)==0, identify all items */
                if (cval == 1 && sobj->blessed && Luck > 0) ++cval;
            } else  cval = 1;
            if(!objects[sobj->otyp].oc_name_known) more_experienced(0,10);
            useup(sobj);
            makeknown(SCR_IDENTIFY);
id:
            if(invent && !confused) {
                identify_pack(cval);
            }
            return(1);
        case SCR_CHARGING:
            if (confused) {
                You_feel("charged up!");
                if (u.uen < u.uenmax)
                    u.uen = u.uenmax;
                else
                    u.uen = (u.uenmax += d(5,4));
                break;
            }
            known = true;
            pline("This is a charging scroll.");
            otmp = getobj(all_count, "charge");
            if (!otmp) break;
            recharge(otmp, sobj->cursed ? -1 : (sobj->blessed ? 1 : 0));
            break;
        case SCR_MAGIC_MAPPING:
            if (level.flags.nommap) {
                Your("mind is filled with crazy lines!");
                if (Hallucination())
                    pline("Wow!  Modern art.");
                else
                    Your("%s spins in bewilderment.", body_part(HEAD));
                make_confused(get_HConfusion() + rnd(30), false);
                break;
            }
            if (sobj->blessed) {
                int x, y;

                for (x = 1; x < COLNO; x++)
                    for (y = 0; y < ROWNO; y++)
                        if (levl[x][y].typ == SDOOR)
                            cvt_sdoor_to_door(&levl[x][y]);
                /* do_mapping() already reveals secret passages */
            }
            known = true;
        case SPE_MAGIC_MAPPING:
            if (level.flags.nommap) {
                Your("%s spins as %s blocks the spell!", body_part(HEAD), something);
                make_confused(get_HConfusion() + rnd(30), false);
                break;
            }
            pline("A map coalesces in your mind!");
            cval = (sobj->cursed && !confused);
            if(cval) set_HConfusion(1);        /* to screw up map */
            do_mapping();
            if(cval) {
                set_HConfusion(0);             /* restore */
                pline("Unfortunately, you can't grasp the details.");
            }
            break;
        case SCR_AMNESIA:
            known = true;
            forget( (!sobj->blessed ? ALL_SPELLS : 0) |
                    (!confused || sobj->cursed ? ALL_MAP : 0) );
            if (Hallucination()) /* Ommmmmm! */
                Your("mind releases itself from mundane concerns.");
            else if (!strncmpi(plname, "Maud", 4))
                pline("As your mind turns inward on itself, you forget everything else.");
            else if (rn2(2))
                pline("Who was that Maud person anyway?");
            else
                pline("Thinking of Maud you forget everything else.");
            exercise(A_WIS, false);
            break;
        case SCR_FIRE:
            /*
             * Note: Modifications have been made as of 3.0 to allow for
             * some damage under all potential cases.
             */
            cval = bcsign(sobj);
            if(!objects[sobj->otyp].oc_name_known) more_experienced(0,10);
            useup(sobj);
            makeknown(SCR_FIRE);
            if(confused) {
                if(Fire_resistance()) {
                    shieldeff(u.ux, u.uy);
                    if(!Blind())
                        pline("Oh, look, what a pretty fire in your %s.",
                                makeplural(body_part(HAND)));
                    else You_feel("a pleasant warmth in your %s.",
                            makeplural(body_part(HAND)));
                } else {
                    pline_The("scroll catches fire and you burn your %s.",
                            makeplural(body_part(HAND)));
                    losehp(1, killed_by_const(KM_SCROLL_OF_FIRE));
                }
                return(1);
            }
            if (Underwater)
                pline_The("water around you vaporizes violently!");
            else {
                pline_The("scroll erupts in a tower of flame!");
                burn_away_slime();
            }
            explode(u.ux, u.uy, 11, (2*(rn1(3, 3) + 2 * cval) + 1)/3,
                    SCROLL_CLASS, EXPL_FIERY);
            return(1);
        case SCR_EARTH:
            /* TODO: handle steeds */
            if (
                    (!In_endgame(&u.uz) || Is_earthlevel(&u.uz))) {
                int x, y;

                /* Identify the scroll */
                pline_The("%s rumbles %s you!", ceiling(u.ux,u.uy),
                        sobj->blessed ? "around" : "above");
                known = 1;
                if (In_sokoban(&u.uz))
                    change_luck(-1);    /* Sokoban guilt */

                /* Loop through the surrounding squares */
                if (!sobj->cursed) for (x = u.ux-1; x <= u.ux+1; x++) {
                    for (y = u.uy-1; y <= u.uy+1; y++) {

                        /* Is this a suitable spot? */
                        if (isok(x, y) && !closed_door(x, y) &&
                                !IS_ROCK(levl[x][y].typ) &&
                                !IS_AIR(levl[x][y].typ) &&
                                (x != u.ux || y != u.uy)) {
                            struct obj *otmp2;
                            struct monst *mtmp;

                            /* Make the object(s) */
                            otmp2 = mksobj(confused ? ROCK : BOULDER,
                                    false, false);
                            if (!otmp2) continue;  /* Shouldn't happen */
                            otmp2->quan = confused ? rn1(5,2) : 1;
                            otmp2->owt = weight(otmp2);

                            /* Find the monster here (won't be player) */
                            mtmp = m_at(x, y);
                            if (mtmp && !amorphous(mtmp->data) &&
                                    !passes_walls(mtmp->data) &&
                                    !noncorporeal(mtmp->data) &&
                                    !unsolid(mtmp->data)) {
                                struct obj *helmet = which_armor(mtmp, W_ARMH);
                                int mdmg;

                                if (cansee(mtmp->mx, mtmp->my)) {
                                    message_monster_object(MSG_M_IS_HIT_BY_O, mtmp, otmp2);
                                    if (mtmp->minvis && !canspotmon(mtmp))
                                        map_invisible(mtmp->mx, mtmp->my);
                                }
                                mdmg = dmgval(otmp2, mtmp) * otmp2->quan;
                                if (helmet) {
                                    if(is_metallic(helmet)) {
                                        if (canspotmon(mtmp)) {
                                            message_monster(MSG_M_WEARING_HARD_HELMET, mtmp);
                                        } else if (flags.soundok) {
                                            You_hear("a clanging sound.");
                                        }
                                        if (mdmg > 2) mdmg = 2;
                                    } else {
                                        if (canspotmon(mtmp)) {
                                            message_monster_object(MSG_M_O_DOES_NOT_PROTECT_HIM,
                                                    mtmp, helmet);
                                        }
                                    }
                                }
                                mtmp->mhp -= mdmg;
                                if (mtmp->mhp <= 0)
                                    xkilled(mtmp, 1);
                            }
                            /* Drop the rock/boulder to the floor */
                            if (!flooreffects(otmp2, x, y, "fall")) {
                                place_object(otmp2, x, y);
                                stackobj(otmp2);
                                newsym(x, y);  /* map the rock */
                            }
                        }
                    }
                }
                /* Attack the player */
                if (!sobj->blessed) {
                    int dmg;
                    struct obj *otmp2;

                    /* Okay, _you_ write this without repeating the code */
                    otmp2 = mksobj(confused ? ROCK : BOULDER,
                            false, false);
                    if (!otmp2) break;
                    otmp2->quan = confused ? rn1(5,2) : 1;
                    otmp2->owt = weight(otmp2);
                    if (!amorphous(youmonst.data) &&
                            !Passes_walls &&
                            !noncorporeal(youmonst.data) &&
                            !unsolid(youmonst.data)) {
                        You("are hit by %s!", doname(otmp2));
                        dmg = dmgval(otmp2, &youmonst) * otmp2->quan;
                        if (uarmh && !sobj->cursed) {
                            if(is_metallic(uarmh)) {
                                pline("Fortunately, you are wearing a hard helmet.");
                                if (dmg > 2) dmg = 2;
                            } else if (flags.verbose) {
                                message_object(MSG_YOUR_O_DOES_NOT_PROTECT_YOU, uarmh);
                            }
                        }
                    } else
                        dmg = 0;
                    /* Must be before the losehp(), for bones files */
                    if (!flooreffects(otmp2, u.ux, u.uy, "fall")) {
                        place_object(otmp2, u.ux, u.uy);
                        stackobj(otmp2);
                        newsym(u.ux, u.uy);
                    }
                    if (dmg)
                        losehp(dmg, killed_by_const(KM_SCROLL_OF_EARTH));
                }
            }
            break;
        case SCR_PUNISHMENT:
            known = true;
            if(confused || sobj->blessed) {
                You_feel("guilty.");
                break;
            }
            punish(sobj);
            break;
        case SCR_STINKING_CLOUD: {
                                     coord cc;

                                     You("have found a scroll of stinking cloud!");
                                     known = true;
                                     pline("Where do you want to center the cloud?");
                                     cc.x = u.ux;
                                     cc.y = u.uy;
                                     if (getpos(&cc, true, "the desired position") < 0) {
                                         plines(Never_mind);
                                         return 0;
                                     }
                                     if (!cansee(cc.x, cc.y) || distu(cc.x, cc.y) >= 32) {
                                         You("smell rotten eggs.");
                                         return 0;
                                     }
                                     (void) create_gas_cloud(cc.x, cc.y, 3+bcsign(sobj),
                                             8+4*bcsign(sobj));
                                     break;
                                 }
        default:
                                 impossible("What weird effect is this? (%u)", sobj->otyp);
    }
    return(0);
}

/*
 * Low-level lit-field update routine.
 */
static void set_lit(int x, int y, void *val) {
    if (val)
        levl[x][y].lit = 1;
    else {
        levl[x][y].lit = 0;
        snuff_light_source(x, y);
    }
}

void litroom (bool on, struct obj *obj) {
    char is_lit;    /* value is irrelevant; we use its address
                       as a `not null' flag for set_lit() */

    /* first produce the text (provided you're not blind) */
    if(!on) {
        struct obj *otmp;

        if (!Blind()) {
            if(u.uswallow) {
                pline("It seems even darker in here than before.");
                return;
            }
            if (uwep && artifact_light(uwep) && uwep->lamplit) {
                message_object(MSG_ONLY_LIGHT_LEFT_COMES_FROM_O, uwep);
            } else {
                You("are surrounded by darkness!");
            }
        }

        /* the magic douses lamps, et al, too */
        for(otmp = invent; otmp; otmp = otmp->nobj)
            if (otmp->lamplit)
                (void) snuff_lit(otmp);
        if (Blind()) goto do_it;
    } else {
        if (Blind()) goto do_it;
        if(u.uswallow){
            if (is_animal(u.ustuck->data)) {
                message_monster(MSG_M_STOMACH_IS_LIT, u.ustuck);
            } else {
                if (is_whirly(u.ustuck->data)) {
                    message_monster(MSG_M_SHINES_BRIEFLY, u.ustuck);
                } else {
                    message_monster(MSG_M_GLISTENS, u.ustuck);
                }
            }
            return;
        }
        pline("A lit field surrounds you!");
    }

do_it:
    /* No-op in water - can only see the adjacent squares and that's it! */
    if (Underwater || Is_waterlevel(&u.uz)) return;
    /*
     *  If we are darkening the room and the hero is punished but not
     *  blind, then we have to pick up and replace the ball and chain so
     *  that we don't remember them if they are out of sight.
     */
    if (Punished && !on && !Blind())
        move_bc(1, 0, uball->ox, uball->oy, uchain->ox, uchain->oy);

    do_clear_area(u.ux,u.uy,
            (obj && obj->oclass==SCROLL_CLASS && obj->blessed) ? 9 : 5,
            set_lit, (void *)(on ? &is_lit : (char *)0));

    /*
     *  If we are not blind, then force a redraw on all positions in sight
     *  by temporarily blinding the hero.  The vision recalculation will
     *  correctly update all previously seen positions *and* correctly
     *  set the waslit bit [could be messed up from above].
     */
    if (!Blind()) {
        vision_recalc(2);

        /* replace ball&chain */
        if (Punished && !on)
            move_bc(0, 0, uball->ox, uball->oy, uchain->ox, uchain->oy);
    }

    vision_full_recalc = 1; /* delayed vision recalculation */
}

#define REALLY 1
#define PLAYER 2
#define ONTHRONE 4
void
do_genocide (int how)
/* 0 = no genocide; create monsters (cursed scroll) */
/* 1 = normal genocide */
/* 3 = forced genocide of player */
/* 5 (4 | 1) = normal genocide from throne */
{
        char buf[BUFSZ];
        int     i, killplayer = 0;
        int mndx;
        struct permonst *ptr;
        const char *which;

        if (how & PLAYER) {
                mndx = u.umonster;      /* non-polymorphed mon num */
                ptr = &mons[mndx];
                strcpy(buf, ptr->mname);
                killplayer++;
        } else {
            for(i = 0; ; i++) {
                if(i >= 5) {
                    plines(thats_enough_tries);
                    return;
                }
                getlin("What monster do you want to genocide? [type the name]",
                        buf);
                (void)mungspaces(buf);
                /* choosing "none" preserves genocideless conduct */
                if (!strcmpi(buf, "none") || !strcmpi(buf, "nothing")) {
                    /* ... but no free pass if cursed */
                    if (!(how & REALLY)) {
                        ptr = rndmonst();
                        if (!ptr) return; /* no message, like normal case */
                        mndx = monsndx(ptr);
                        break;          /* remaining checks don't apply */
                    } else return;
                }

                mndx = name_to_mon(buf);
                if (mndx == NON_PM || (mvitals[mndx].mvflags & G_GENOD)) {
                        pline("Such creatures %s exist in this world.",
                              (mndx == NON_PM) ? "do not" : "no longer");
                        continue;
                }
                ptr = &mons[mndx];
                /* Although "genus" is Latin for race, the hero benefits
                 * from both race and role; thus genocide affects either.
                 */
                if (Your_Own_Role(mndx) || Your_Own_Race(mndx)) {
                        killplayer++;
                        break;
                }
                if (is_human(ptr)) adjalign(-sgn(u.ualign.type));
                if (is_demon(ptr)) adjalign(sgn(u.ualign.type));

                if(!(ptr->geno & G_GENO)) {
                        if(flags.soundok) {
        /* fixme: unconditional "caverns" will be silly in some circumstances */
                            if(flags.verbose)
                        pline("A thunderous voice booms through the caverns:");
                            verbalize("No, mortal!  That will not be done.");
                        }
                        continue;
                }
                /* KMH -- Unchanging prevents rehumanization */
                if (Unchanging && ptr == youmonst.data)
                    killplayer++;
                break;
            }
        }

        which = "all ";
        if (Hallucination()) {
            if (Upolyd)
                strcpy(buf,youmonst.data->mname);
            else {
                strcpy(buf, (flags.female && urole.name.f) ?
                                urole.name.f : urole.name.m);
                buf[0] = lowc(buf[0]);
            }
        } else {
            strcpy(buf, ptr->mname); /* make sure we have standard singular */
            if ((ptr->geno & G_UNIQ) && ptr != &mons[PM_HIGH_PRIEST])
                which = !type_is_pname(ptr) ? "the " : "";
        }
        if (how & REALLY) {
            /* setting no-corpse affects wishing and random tin generation */
            mvitals[mndx].mvflags |= (G_GENOD | G_NOCORPSE);
            pline("Wiped out %s%s.", which, (*which != 'a') ? buf : makeplural(buf));

            if (killplayer) {
                /* might need to wipe out dual role */
                if (urole.femalenum != NON_PM && mndx == urole.malenum)
                    mvitals[urole.femalenum].mvflags |= (G_GENOD | G_NOCORPSE);
                if (urole.femalenum != NON_PM && mndx == urole.femalenum)
                    mvitals[urole.malenum].mvflags |= (G_GENOD | G_NOCORPSE);
                if (urace.femalenum != NON_PM && mndx == urace.malenum)
                    mvitals[urace.femalenum].mvflags |= (G_GENOD | G_NOCORPSE);
                if (urace.femalenum != NON_PM && mndx == urace.femalenum)
                    mvitals[urace.malenum].mvflags |= (G_GENOD | G_NOCORPSE);

                u.uhp = -1;
                if (how & PLAYER) {
                    killer = killed_by_const(KM_GENOCIDAL_CONFUSION);
                } else if (how & ONTHRONE) {
                    /* player selected while on a throne */
                    killer = killed_by_const(KM_IMPERIOUS_ORDER);
                } else { /* selected player deliberately, not confused */
                    killer = killed_by_const(KM_SCROLL_OF_GENOCIDE);
                }

        /* Polymorphed characters will die as soon as they're rehumanized. */
        /* KMH -- Unchanging prevents rehumanization */
                if (Upolyd && ptr != youmonst.data) {
                        delayed_killer = killer;
                        killer.method = KM_DIED;
                        You_feel("dead inside.");
                } else
                        done(GENOCIDED);
            } else if (ptr == youmonst.data) {
                rehumanize();
            }
            reset_rndmonst(mndx);
            kill_genocided_monsters();
        } else {
            int cnt = 0;

            if (!(mons[mndx].geno & G_UNIQ) &&
                    !(mvitals[mndx].mvflags & (G_GENOD | G_EXTINCT)))
                for (i = rn1(3, 4); i > 0; i--) {
                    if (!makemon(ptr, u.ux, u.uy, NO_MINVENT))
                        break;  /* couldn't make one */
                    ++cnt;
                    if (mvitals[mndx].mvflags & G_EXTINCT)
                        break;  /* just made last one */
                }
            if (cnt)
                pline("Sent in some %s.", makeplural(buf));
            else
                message_const(MSG_NOTHING_HAPPENS);
        }
}

void
punish (struct obj *sobj)
{
        /* KMH -- Punishment is still okay when you are riding */
        You("are being punished for your misbehavior!");
        if(Punished){
                Your("iron ball gets heavier.");
                uball->owt += 160 * (1 + sobj->cursed);
                return;
        }
        if (amorphous(youmonst.data) || is_whirly(youmonst.data) || unsolid(youmonst.data)) {
                pline("A ball and chain appears, then falls away.");
                dropy(mkobj(BALL_CLASS, true));
                return;
        }
        setworn(mkobj(CHAIN_CLASS, true), W_CHAIN);
        setworn(mkobj(BALL_CLASS, true), W_BALL);
        uball->spe = 1;         /* special ball (see save) */

        /*
         *  Place ball & chain if not swallowed.  If swallowed, the ball &
         *  chain variables will be set at the next call to placebc().
         */
        if (!u.uswallow) {
            placebc();
            if (Blind()) set_bc(1);       /* set up ball and chain variables */
            newsym(u.ux,u.uy);          /* see ball&chain if can't see self */
        }
}

void
unpunish (void)
{           /* remove the ball and chain */
        struct obj *savechain = uchain;

        obj_extract_self(uchain);
        newsym(uchain->ox,uchain->oy);
        setworn((struct obj *)0, W_CHAIN);
        dealloc_obj(savechain);
        uball->spe = 0;
        setworn((struct obj *)0, W_BALL);
}

/* some creatures have special data structures that only make sense in their
 * normal locations -- if the player tries to create one elsewhere, or to revive
 * one, the disoriented creature becomes a zombie
 */
bool 
cant_create (int *mtype, bool revival)
{

        /* SHOPKEEPERS can be revived now */
        if (*mtype==PM_GUARD || (*mtype==PM_SHOPKEEPER && !revival)
             || *mtype==PM_ALIGNED_PRIEST || *mtype==PM_ANGEL) {
                *mtype = PM_HUMAN_ZOMBIE;
                return true;
        } else if (*mtype==PM_LONG_WORM_TAIL) { /* for create_particular() */
                *mtype = PM_LONG_WORM;
                return true;
        }
        return false;
}

/*
 * Make a new monster with the type controlled by the user.
 *
 * Note:  when creating a monster by class letter, specifying the
 * "strange object" (']') symbol produces a random monster rather
 * than a mimic; this behavior quirk is useful so don't "fix" it...
 */
bool 
create_particular (void)
{
        char buf[BUFSZ], *bufp, monclass = MAXMCLASSES;
        int which, tries, i;
        struct permonst *whichpm;
        struct monst *mtmp;
        bool madeany = false;
        bool maketame, makepeaceful, makehostile;

        tries = 0;
        do {
            which = urole.malenum;      /* an arbitrary index into mons[] */
            maketame = makepeaceful = makehostile = false;
            getlin("Create what kind of monster? [type the name or symbol]",
                   buf);
            bufp = mungspaces(buf);
            if (*bufp == '\033') return false;
            /* allow the initial disposition to be specified */
            if (!strncmpi(bufp, "tame ", 5)) {
                bufp += 5;
                maketame = true;
            } else if (!strncmpi(bufp, "peaceful ", 9)) {
                bufp += 9;
                makepeaceful = true;
            } else if (!strncmpi(bufp, "hostile ", 8)) {
                bufp += 8;
                makehostile = true;
            }
            /* decide whether a valid monster was chosen */
            if (strlen(bufp) == 1) {
                monclass = def_char_to_monclass(*bufp);
                if (monclass != MAXMCLASSES) break;     /* got one */
            } else {
                which = name_to_mon(bufp);
                if (which >= LOW_PM) break;             /* got one */
            }
            /* no good; try again... */
            pline("I've never heard of such monsters.");
        } while (++tries < 5);

        if (tries == 5) {
            plines(thats_enough_tries);
        } else {
            (void) cant_create(&which, false);
            whichpm = &mons[which];
            for (i = 0; i <= multi; i++) {
                if (monclass != MAXMCLASSES)
                    whichpm = mkclass(monclass, 0);
                if (maketame) {
                    mtmp = makemon(whichpm, u.ux, u.uy, MM_EDOG);
                    if (mtmp) {
                        initedog(mtmp);
                        set_malign(mtmp);
                    }
                } else {
                    mtmp = makemon(whichpm, u.ux, u.uy, NO_MM_FLAGS);
                    if ((makepeaceful || makehostile) && mtmp) {
                        mtmp->mtame = 0;        /* sanity precaution */
                        mtmp->mpeaceful = makepeaceful ? 1 : 0;
                        set_malign(mtmp);
                    }
                }
                if (mtmp) madeany = true;
            }
        }
        return madeany;
}


/*read.c*/
