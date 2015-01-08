/* See LICENSE in the root of this project for change info */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "attrib.h"
#include "decl.h"
#include "do.h"
#include "engrave.h"
#include "global.h"
#include "hack.h"
#include "hacklib.h"
#include "invent.h"
#include "mkobj.h"
#include "mondata.h"
#include "monst.h"
#include "obj.h"
#include "objclass.h"
#include "objnam.h"
#include "onames.h"
#include "pline.h"
#include "pm.h"
#include "rnd.h"
#include "shk.h"
#include "util.h"
#include "write.h"
#include "you.h"
#include "youprop.h"

static int cost(struct obj *);

/*
 * returns basecost of a scroll or a spellbook
 */
static int cost(struct obj *otmp) {

    if (otmp->oclass == SPBOOK_CLASS)
        return (10 * objects[otmp->otyp].oc_oc2);

    switch (otmp->otyp) {
        case SCR_MAIL:
            return (2);
        case SCR_LIGHT:
        case SCR_GOLD_DETECTION:
        case SCR_FOOD_DETECTION:
        case SCR_MAGIC_MAPPING:
        case SCR_AMNESIA:
        case SCR_FIRE:
        case SCR_EARTH:
            return (8);
        case SCR_DESTROY_ARMOR:
        case SCR_CREATE_MONSTER:
        case SCR_PUNISHMENT:
            return (10);
        case SCR_CONFUSE_MONSTER:
            return (12);
        case SCR_IDENTIFY:
            return (14);
        case SCR_ENCHANT_ARMOR:
        case SCR_REMOVE_CURSE:
        case SCR_ENCHANT_WEAPON:
        case SCR_CHARGING:
            return (16);
        case SCR_SCARE_MONSTER:
        case SCR_STINKING_CLOUD:
        case SCR_TAMING:
        case SCR_TELEPORTATION:
            return (20);
        case SCR_GENOCIDE:
            return (30);
        case SCR_BLANK_PAPER:
        default:
            impossible("You can't write such a weird scroll!");
    }
    return (1000);
}

static const char write_on[] = { SCROLL_CLASS, SPBOOK_CLASS, 0 };

int dowrite(struct obj *pen) {
    struct obj *paper;
    char namebuf[BUFSZ], *nm, *bp;
    struct obj *new_obj;
    int basecost, actualcost;
    int curseval;
    char qbuf[QBUFSZ];
    int first, last, i;
    bool by_descr = false;
    const char *typeword;

    if (nohands(youmonst.data)) {
        message_const(MSG_YOU_NEED_HANDS_TO_WRITE);
        return 0;
    } else if (Glib) {
        message_object(MSG_IT_SLIPS_FROM_YOUR_FINGERS, pen);
        dropx(pen);
        return 1;
    }

    /* get paper to write on */
    paper = getobj(write_on, "write on");
    if (!paper)
        return 0;
    typeword = (paper->oclass == SPBOOK_CLASS) ? "spellbook" : "scroll";
    if (Blind && !paper->dknown) {
        message_string(MSG_YOU_DONT_KNOW_THAT_IS_BLANK, typeword);
        return 1;
    }
    paper->dknown = 1;
    if (paper->otyp != SCR_BLANK_PAPER && paper->otyp != SPE_BLANK_PAPER) {
        message_string(MSG_THAT_IS_NOT_BLANK, typeword);
        exercise(A_WIS, false);
        return 1;
    }

    /* what to write */
    sprintf(qbuf, "What type of %s do you want to write?", typeword);
    getlin(qbuf, namebuf);
    (void)mungspaces(namebuf); /* remove any excess whitespace */
    if (namebuf[0] == '\033' || !namebuf[0])
        return (1);
    nm = namebuf;
    if (!strncmpi(nm, "scroll ", 7))
        nm += 7;
    else if (!strncmpi(nm, "spellbook ", 10))
        nm += 10;
    if (!strncmpi(nm, "of ", 3))
        nm += 3;

    if ((bp = strstri(nm, " armour")) != 0) {
        (void)strncpy(bp, " armor ", 7); /* won't add '\0' */
        (void)mungspaces(bp + 1); /* remove the extra space */
    }

    first = bases[(int)paper->oclass];
    last = bases[(int)paper->oclass + 1] - 1;
    for (i = first; i <= last; i++) {
        /* extra shufflable descr not representing a real object */
        if (!OBJ_NAME(objects[i]))
            continue;

        if (!strcmpi(OBJ_NAME(objects[i]), nm))
            goto found;
        if (!strcmpi(OBJ_DESCR(objects[i]), nm)) {
            by_descr = true;
            goto found;
        }
    }

    There("is no such %s!", typeword);
    return 1;

    found:
    if (i == SCR_BLANK_PAPER || i == SPE_BLANK_PAPER) {
        message_const(MSG_CANT_WRITE_THAT_ITS_OBSCENE);
        return 1;
    } else if (i == SPE_BOOK_OF_THE_DEAD) {
        message_const(MSG_CANT_WRITE_BOOK_OF_THE_DEAD);
        return 1;
    } else if (by_descr && paper->oclass == SPBOOK_CLASS && !objects[i].oc_name_known) {
        /* can't write unknown spellbooks by description */
        message_const(MSG_CANT_WRITE_WHAT_YOU_DONT_KNOW);
        return 1;
    }

    /* KMH, conduct */
    u.uconduct.literate++;

    new_obj = mksobj(i, false, false);
    new_obj->bknown = (paper->bknown && pen->bknown);

    /* shk imposes a flat rate per use, not based on actual charges used */
    check_unpaid(pen);

    /* see if there's enough ink */
    basecost = cost(new_obj);
    if (pen->spe < basecost / 2) {
        message_const(MSG_MARKER_TOO_DRY);
        obfree(new_obj, NULL);
        return 1;
    }

    /* we're really going to write now, so calculate cost
     */
    actualcost = rn1(basecost / 2, basecost / 2);
    curseval = bcsign(pen) + bcsign(paper);
    exercise(A_WIS, true);
    /* dry out marker */
    if (pen->spe < actualcost) {
        pen->spe = 0;
        message_const(MSG_MARKER_DRIES_OUT);
        /* scrolls disappear, spellbooks don't */
        if (paper->oclass == SPBOOK_CLASS) {
            message_const(MSG_SPELLBOOK_IS_UNFINISHED);
        } else {
            message_const(MSG_SCROLL_IS_NOW_USELESS);
            useup(paper);
        }
        obfree(new_obj, (struct obj *)0);
        return 1;
    }
    pen->spe -= actualcost;

    /* can't write if we don't know it - unless we're lucky */
    if (!(objects[new_obj->otyp].oc_name_known) && !(objects[new_obj->otyp].oc_uname) && (rnl(Role_if(PM_WIZARD) ? 3 : 15))) {
        You("%s to write that!", by_descr ? "fail" : "don't know how");
        /* scrolls disappear, spellbooks don't */
        if (paper->oclass == SPBOOK_CLASS) {
            You("write in your best handwriting:  \"My Diary\", but it quickly fades.");
        } else {
            if (by_descr) {
                strcpy(namebuf, OBJ_DESCR(objects[new_obj->otyp]));
                wipeout_text(namebuf, (6 + MAXULEV - u.ulevel) / 6, 0);
            } else
                sprintf(namebuf, "%s was here!", plname);
            You("write \"%s\" and the scroll disappears.", namebuf);
            useup(paper);
        }
        obfree(new_obj, (struct obj *)0);
        return 1;
    }

    /* useup old scroll / spellbook */
    useup(paper);

    /* success */
    if (new_obj->oclass == SPBOOK_CLASS) {
        /* acknowledge the change in the object's description... */
        pline_The("spellbook warps strangely, then turns %s.", OBJ_DESCR(objects[new_obj->otyp]));
    }
    new_obj->blessed = (curseval > 0);
    new_obj->cursed = (curseval < 0);
    if (new_obj->otyp == SCR_MAIL)
        new_obj->spe = 1;
    new_obj = hold_another_object(new_obj, "Oops!  %s out of your grasp!", The(aobjnam(new_obj, "slip")), (const char *)0);
    return (1);
}
