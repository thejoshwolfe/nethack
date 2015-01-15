/* See LICENSE in the root of this project for change info */

#include "minion.h"

#include <stdio.h>

#include "artifact_names.h"
#include "decl.h"
#include "display.h"
#include "do_name.h"
#include "dungeon.h"
#include "emin.h"
#include "epri.h"
#include "flag.h"
#include "global.h"
#include "hack.h"
#include "hacklib.h"
#include "invent.h"
#include "makemon.h"
#include "mon.h"
#include "mondata.h"
#include "monflag.h"
#include "monst.h"
#include "monsym.h"
#include "obj.h"
#include "permonst.h"
#include "pline.h"
#include "pm.h"
#include "pm_props.h"
#include "pray.h"
#include "rnd.h"
#include "teleport.h"
#include "util.h"
#include "wizard.h"
#include "you.h"
#include "youprop.h"

/* mon summons a monster */
void msummon ( struct monst *mon) {
    struct permonst *ptr;
    int dtype = NON_PM, cnt = 0;
    aligntyp atyp;
    struct monst *mtmp;

    if (mon) {
        ptr = mon->data;
        atyp = (ptr->maligntyp==A_NONE) ? A_NONE : sgn(ptr->maligntyp);
        if (mon->ispriest || mon->data == &mons[PM_ALIGNED_PRIEST]
                || mon->data == &mons[PM_ANGEL])
            atyp = EPRI(mon)->shralign;
    } else {
        ptr = &mons[PM_WIZARD_OF_YENDOR];
        atyp = (ptr->maligntyp==A_NONE) ? A_NONE : sgn(ptr->maligntyp);
    }

    if (is_dprince(ptr) || (ptr == &mons[PM_WIZARD_OF_YENDOR])) {
        dtype = (!rn2(20)) ? dprince(atyp) :
            (!rn2(4)) ? dlord(atyp) : ndemon(atyp);
        cnt = (!rn2(4) && is_ndemon(&mons[dtype])) ? 2 : 1;
    } else if (is_dlord(ptr)) {
        dtype = (!rn2(50)) ? dprince(atyp) :
            (!rn2(20)) ? dlord(atyp) : ndemon(atyp);
        cnt = (!rn2(4) && is_ndemon(&mons[dtype])) ? 2 : 1;
    } else if (is_ndemon(ptr)) {
        dtype = (!rn2(20)) ? dlord(atyp) :
            (!rn2(6)) ? ndemon(atyp) : monsndx(ptr);
        cnt = 1;
    } else if (is_lminion(mon)) {
        dtype = (is_lord(ptr) && !rn2(20)) ? llord() :
            (is_lord(ptr) || !rn2(6)) ? lminion() : monsndx(ptr);
        cnt = (!rn2(4) && !is_lord(&mons[dtype])) ? 2 : 1;
    } else if (ptr == &mons[PM_ANGEL]) {
        /* non-lawful angels can also summon */
        if (!rn2(6)) {
            switch (atyp) { /* see summon_minion */
                case A_NEUTRAL:
                    dtype = PM_AIR_ELEMENTAL + rn2(4);
                    break;
                case A_CHAOTIC:
                case A_NONE:
                    dtype = ndemon(atyp);
                    break;
            }
        } else {
            dtype = PM_ANGEL;
        }
        cnt = (!rn2(4) && !is_lord(&mons[dtype])) ? 2 : 1;
    }

    if (dtype == NON_PM) return;

    /* sanity checks */
    if (cnt > 1 && (mons[dtype].geno & G_UNIQ)) cnt = 1;
    /*
     * If this daemon is unique and being re-summoned (the only way we
     * could get this far with an extinct dtype), try another.
     */
    if (mvitals[dtype].mvflags & G_GONE) {
        dtype = ndemon(atyp);
        if (dtype == NON_PM) return;
    }

    while (cnt > 0) {
        mtmp = makemon(&mons[dtype], u.ux, u.uy, NO_MM_FLAGS);
        if (mtmp && (dtype == PM_ANGEL)) {
            /* alignment should match the summoner */
            EPRI(mtmp)->shralign = atyp;
        }
        cnt--;
    }
}

void summon_minion (aligntyp alignment, bool talk) {
    struct monst *mon;
    int mnum;

    switch ((int)alignment) {
        case A_LAWFUL:
            mnum = lminion();
            break;
        case A_NEUTRAL:
            mnum = PM_AIR_ELEMENTAL + rn2(4);
            break;
        case A_CHAOTIC:
        case A_NONE:
            mnum = ndemon(alignment);
            break;
        default:
            impossible("unaligned player?");
            mnum = ndemon(A_NONE);
            break;
    }
    if (mnum == NON_PM) {
        mon = 0;
    } else if (mons[mnum].pxlth == 0) {
        struct permonst *pm = &mons[mnum];
        mon = makemon(pm, u.ux, u.uy, MM_EMIN);
        if (mon) {
            mon->isminion = true;
            EMIN(mon)->min_align = alignment;
        }
    } else if (mnum == PM_ANGEL) {
        mon = makemon(&mons[mnum], u.ux, u.uy, NO_MM_FLAGS);
        if (mon) {
            mon->isminion = true;
            EPRI(mon)->shralign = alignment;    /* always A_LAWFUL here */
        }
    } else
        mon = makemon(&mons[mnum], u.ux, u.uy, NO_MM_FLAGS);
    if (mon) {
        if (talk) {
            pline_The("voice of %s booms:", align_gname(alignment));
            verbalize("Thou shalt pay for thy indiscretion!");
            if (!Blind()) {
                char name[BUFSZ];
                Amonnam(name, BUFSZ, mon);
                pline("%s appears before you.", name);
            }
        }
        mon->mpeaceful = false;
        /* don't call set_malign(); player was naughty */
    }
}

/* returns 1 if it won't attack. */
int demon_talk ( struct monst *mtmp) {
    long cash, demand, offer;

    char name[BUFSZ];
    Amonnam(name, BUFSZ, mtmp);
    if (uwep && uwep->oartifact == ART_EXCALIBUR) {
        pline("%s looks very angry.", name);
        mtmp->mpeaceful = mtmp->mtame = 0;
        set_malign(mtmp);
        newsym(mtmp->mx, mtmp->my);
        return 0;
    }

    /* Slight advantage given. */
    if (is_dprince(mtmp->data) && mtmp->minvis) {
        mtmp->minvis = mtmp->perminvis = 0;
        if (!Blind()) pline("%s appears before you.", name);
        newsym(mtmp->mx,mtmp->my);
    }
    if (youmonst.data->mlet == S_DEMON) {   /* Won't blackmail their own. */
        pline("%s says, \"Good hunting, %s.\"",
                name, flags.female ? "Sister" : "Brother");
        if (!tele_restrict(mtmp)) (void) rloc(mtmp, false);
        return(1);
    }
    cash = u.ugold;
    demand = (cash * (rnd(80) + 20 * (In_hell(&u.uz) && !mtmp->cham))) /
        (100 * (1 + (sgn(u.ualign.type) == sgn(mtmp->data->maligntyp))));

    if (!demand) {          /* you have no gold */
        mtmp->mpeaceful = 0;
        set_malign(mtmp);
        return 0;
    } else {
        /* make sure that the demand is unmeetable if the monster
           has the Amulet, preventing monster from being satisified
           and removed from the game (along with said Amulet...) */
        if (mon_has_amulet(mtmp))
            demand = cash + (long)rn1(1000,40);

        pline("%s demands %ld %s for safe passage.", name, demand, currency(demand));

        if ((offer = bribe(mtmp)) >= demand) {
            pline("%s vanishes, laughing about cowardly mortals.", name);
        } else if (offer > 0L && (long)rnd(40) > (demand - offer)) {
            pline("%s scowls at you menacingly, then vanishes.", name);
        } else {
            pline("%s gets angry...", name);
            mtmp->mpeaceful = 0;
            set_malign(mtmp);
            return 0;
        }
    }
    mongone(mtmp);
    return(1);
}

long bribe (struct monst *mtmp) {
    char buf[BUFSZ];
    long offer;

    getlin("How much will you offer?", buf);
    if (sscanf(buf, "%ld", &offer) != 1) offer = 0L;

    /*Michael Paddon -- fix for negative offer to monster*/
    /*JAR880815 - */
    char name[BUFSZ];
    mon_nam(name, BUFSZ, mtmp);
    if (offer < 0L) {
        You("try to shortchange %s, but fumble.", name);
        return 0L;
    } else if (offer == 0L) {
        You("refuse.");
        return 0L;
    } else if (offer >= u.ugold) {
        You("give %s all your gold.", name);
        offer = u.ugold;
    } else {
        You("give %s %ld %s.", name, offer, currency(offer));
    }
    u.ugold -= offer;
    mtmp->mgold += offer;
    return offer;
}

int dprince (aligntyp atyp) {
    int tryct, pm;

    for (tryct = 0; tryct < 20; tryct++) {
        pm = rn1(PM_DEMOGORGON + 1 - PM_ORCUS, PM_ORCUS);
        if (!(mvitals[pm].mvflags & G_GONE) &&
                (atyp == A_NONE || sgn(mons[pm].maligntyp) == sgn(atyp)))
            return(pm);
    }
    return(dlord(atyp));    /* approximate */
}

int dlord (aligntyp atyp) {
    int tryct, pm;

    for (tryct = 0; tryct < 20; tryct++) {
        pm = rn1(PM_YEENOGHU + 1 - PM_JUIBLEX, PM_JUIBLEX);
        if (!(mvitals[pm].mvflags & G_GONE) &&
                (atyp == A_NONE || sgn(mons[pm].maligntyp) == sgn(atyp)))
            return(pm);
    }
    return(ndemon(atyp));   /* approximate */
}

/* create lawful (good) lord */
int llord (void) {
    if (!(mvitals[PM_ARCHON].mvflags & G_GONE))
        return(PM_ARCHON);

    return(lminion());      /* approximate */
}

int lminion (void) {
    int     tryct;
    struct  permonst *ptr;

    for (tryct = 0; tryct < 20; tryct++) {
        ptr = mkclass(S_ANGEL,0);
        if (ptr && !is_lord(ptr))
            return(monsndx(ptr));
    }

    return NON_PM;
}

int ndemon (aligntyp atyp) {
    int     tryct;
    struct  permonst *ptr;

    for (tryct = 0; tryct < 20; tryct++) {
        ptr = mkclass(S_DEMON, 0);
        if (ptr && is_ndemon(ptr) &&
                (atyp == A_NONE || sgn(ptr->maligntyp) == sgn(atyp)))
            return(monsndx(ptr));
    }

    return NON_PM;
}
