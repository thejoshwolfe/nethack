/* See LICENSE in the root of this project for change info */

#include <stdio.h>
#include <string.h>

#include "decl.h"
#include "display.h"
#include "do_name.h"
#include "dog.h"
#include "flag.h"
#include "global.h"
#include "hack.h"
#include "hacklib.h"
#include "makemon.h"
#include "mondata.h"
#include "monst.h"
#include "objnam.h"
#include "permonst.h"
#include "pline.h"
#include "pm.h"
#include "pm_props.h"
#include "polyself.h"
#include "rnd.h"
#include "weapon.h"
#include "were.h"
#include "worn.h"
#include "you.h"
#include "youprop.h"


void were_change(struct monst *mon) {
    if (!is_were(mon->data))
        return;

    if (is_human(mon->data)) {
        if (!Protection_from_shape_changers && !rn2(night() ? (flags.moonphase == FULL_MOON ? 3 : 30) : (flags.moonphase == FULL_MOON ? 10 : 50))) {
            new_were(mon); /* change into animal form */
            if (flags.soundok && !canseemon(mon)) {
                const char *howler;

                switch (monsndx(mon->data)) {
                    case PM_WEREWOLF:
                        howler = "wolf";
                        break;
                    case PM_WEREJACKAL:
                        howler = "jackal";
                        break;
                    default:
                        howler = (char *)0;
                        break;
                }
                if (howler)
                    You_hear("a %s howling at the moon.", howler);
            }
        }
    } else if (!rn2(30) || Protection_from_shape_changers) {
        new_were(mon); /* change back into human form */
    }
}


static int counter_were(int);

static int counter_were(int pm) {
    switch (pm) {
        case PM_WEREWOLF:
            return (PM_HUMAN_WEREWOLF);
        case PM_HUMAN_WEREWOLF:
            return (PM_WEREWOLF);
        case PM_WEREJACKAL:
            return (PM_HUMAN_WEREJACKAL);
        case PM_HUMAN_WEREJACKAL:
            return (PM_WEREJACKAL);
        case PM_WERERAT:
            return (PM_HUMAN_WERERAT);
        case PM_HUMAN_WERERAT:
            return (PM_WERERAT);
        default:
            return (0);
    }
}

void new_were(struct monst *mon) {
    int pm;

    pm = counter_were(monsndx(mon->data));
    if (!pm) {
        impossible("unknown lycanthrope %s.", mon->data->mname);
        return;
    }

    if (canseemon(mon) && !Hallucination()) {
        char name[BUFSZ];
        Monnam(name, BUFSZ, mon);
        pline("%s changes into a %s.", name, is_human(&mons[pm]) ? "human" : mons[pm].mname + 4);
    }

    set_mon_data(mon, &mons[pm], 0);
    if (mon->msleeping || !mon->mcanmove) {
        /* transformation wakens and/or revitalizes */
        mon->msleeping = 0;
        mon->mfrozen = 0; /* not asleep or paralyzed */
        mon->mcanmove = 1;
    }
    /* regenerate by 1/4 of the lost hit points */
    mon->mhp += (mon->mhpmax - mon->mhp) / 4;
    newsym(mon->mx, mon->my);
    mon_break_armor(mon, false);
    possibly_unwield(mon, false);
}

/* were-creature (even you) summons a horde */
/* visible: number of visible helpers created */
int were_summon(struct permonst *ptr, bool yours, int *visible, char *genbuf) {
    int i, typ, pm = monsndx(ptr);
    struct monst *mtmp;
    int total = 0;

    *visible = 0;
    if (Protection_from_shape_changers && !yours)
        return 0;
    for (i = rnd(5); i > 0; i--) {
        switch (pm) {

            case PM_WERERAT:
            case PM_HUMAN_WERERAT:
                typ = rn2(3) ? PM_SEWER_RAT : rn2(3) ? PM_GIANT_RAT : PM_RABID_RAT;
                if (genbuf)
                    strcpy(genbuf, "rat");
                break;
            case PM_WEREJACKAL:
            case PM_HUMAN_WEREJACKAL:
                typ = PM_JACKAL;
                if (genbuf)
                    strcpy(genbuf, "jackal");
                break;
            case PM_WEREWOLF:
            case PM_HUMAN_WEREWOLF:
                typ = rn2(5) ? PM_WOLF : PM_WINTER_WOLF;
                if (genbuf)
                    strcpy(genbuf, "wolf");
                break;
            default:
                continue;
        }
        mtmp = makemon(&mons[typ], u.ux, u.uy, NO_MM_FLAGS);
        if (mtmp) {
            total++;
            if (canseemon(mtmp))
                *visible += 1;
        }
        if (yours && mtmp)
            (void)tamedog(mtmp, (struct obj *)0);
    }
    return total;
}

void you_were(void) {
    char qbuf[QBUFSZ];

    if (Unchanging || (u.umonnum == u.ulycn))
        return;
    if (Polymorph_control) {
        /* `+4' => skip "were" prefix to get name of beast */
        sprintf(qbuf, "Do you want to change into %s? ", an(mons[u.ulycn].mname + 4));
        if (yn(qbuf) == 'n')
            return;
    }
    (void)polymon(u.ulycn);
}

void you_unwere(bool purify) {
    if (purify) {
        You_feel("purified.");
        u.ulycn = NON_PM; /* cure lycanthropy */
    }
    if (!Unchanging && is_were(youmonst.data) && (!Polymorph_control || yn("Remain in beast form?") == 'n'))
        rehumanize();
}
