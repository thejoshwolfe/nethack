/* See LICENSE in the root of this project for change info */

#include <stdbool.h>
#include <string.h>

#include "display_util.h"
#include "artifact.h"
#include "artifact_names.h"
#include "decl.h"
#include "display.h"
#include "end.h"
#include "global.h"
#include "hacklib.h"
#include "monattk.h"
#include "mondata.h"
#include "monflag.h"
#include "monst.h"
#include "monsym.h"
#include "obj.h"
#include "objclass.h"
#include "objnam.h"
#include "onames.h"
#include "permonst.h"
#include "pm.h"
#include "pm_props.h"
#include "prop.h"
#include "shk.h"
#include "wizard.h"
#include "you.h"
#include "youprop.h"

/*      These routines provide basic data for any type of monster. */

void set_mon_data(struct monst * mon, struct permonst * ptr, int flag) {
    mon->data = ptr;
    if (flag == -1)
        return; /* "don't care" */

    if (flag == 1)
        mon->mintrinsics |= (ptr->mresists & 0x00FF);
    else
        mon->mintrinsics = (ptr->mresists & 0x00FF);
    return;
}

const struct attack * attacktype_fordmg(const struct permonst *ptr, int atyp, int dtyp) {
    const struct attack *a;
    for (a = &ptr->mattk[0]; a < &ptr->mattk[NATTK]; a++)
        if (a->aatyp == atyp && (dtyp == AD_ANY || a->adtyp == dtyp))
            return a;
    return NULL;
}

bool attacktype(const struct permonst *ptr, int atyp) {
    return attacktype_fordmg(ptr, atyp, AD_ANY) != NULL;
}


bool poly_when_stoned(const struct permonst * ptr) {
    return is_golem(ptr) && ptr != &mons[PM_STONE_GOLEM] &&
           !(mvitals[PM_STONE_GOLEM].mvflags & G_GENOD);
           /* allow G_EXTINCT */
}

/* returns true if monster is drain-life resistant */
bool resists_drli(const struct monst * mon) {
    const struct permonst * ptr = mon->data;
    struct obj * wep = (mon == &youmonst) ? uwep : MON_WEP(mon);

    return is_undead(ptr) || is_demon(ptr) || is_were(ptr) ||
            ptr == &mons[PM_DEATH] ||
            (wep && wep->oartifact && defends(AD_DRLI, wep));
}

/* true if monster is magic-missile resistant */
bool resists_magm(const struct monst *mon) {
    struct permonst *ptr = mon->data;
    /* as of 3.2.0:  gray dragons, Angels, Oracle, Yeenoghu */
    if (dmgtype(ptr, AD_MAGM) ||
            ptr == &mons[PM_BABY_GRAY_DRAGON] ||
            dmgtype(ptr, AD_RBRE)) /* Chromatic Dragon */
        return true;
    /* check for magic resistance granted by wielded weapon */
    const struct obj * o = (mon == &youmonst) ? uwep : MON_WEP(mon);
    if (o && o->oartifact && defends(AD_MAGM, o))
        return true;
    /* check for magic resistance granted by worn or carried items */
    o = (mon == &youmonst) ? invent : mon->minvent;
    for (; o; o = o->nobj)
        if ((o->owornmask && objects[o->otyp].oc_oprop == ANTIMAGIC) ||
                (o->oartifact && protects(AD_MAGM, o)))
            return true;
    return false;
}

/* true iff monster is resistant to light-induced blindness */
bool resists_blnd(const struct monst * mon) {
    const struct permonst *ptr = mon->data;
    bool is_you = (mon == &youmonst);
    const struct obj *o;

    if (is_you) {
        if (Blind() || u.usleep)
            return Blind();
    } else {
        if (mon->mblinded || !mon->mcansee || !haseyes(ptr) ||
                /* BUG: temporary sleep sets mfrozen, but since
                        paralysis does too, we can't check it */
                mon->msleeping)
            return true;
    }
    /* yellow light, Archon; !dust vortex, !cobra, !raven */
    if (dmgtype_fromattack(ptr, AD_BLND, AT_EXPL) ||
            dmgtype_fromattack(ptr, AD_BLND, AT_GAZE))
        return true;
    o = is_you ? uwep : MON_WEP(mon);
    if (o && o->oartifact && defends(AD_BLND, o))
        return true;
    o = is_you ? invent : mon->minvent;
    for ( ; o; o = o->nobj)
        if ((o->owornmask && objects[o->otyp].oc_oprop == BLINDED) ||
                (o->oartifact && protects(AD_BLND, o)))
            return true;
    return false;
}

/* true iff monster can be blinded by the given attack */
/* Note: may return true when mdef is blind (e.g. new cream-pie attack) */
/* NULL == no specific aggressor */
/* aatyp == AT_WEAP, AT_SPIT */
bool can_blnd(const struct monst *magr, const struct monst *mdef, unsigned char aatyp, const struct obj *obj) {
    bool is_you = (mdef == &youmonst);
    bool check_visor = false;
    const struct obj * o;
    const char *s;

    /* no eyes protect against all attacks for now */
    if (!haseyes(mdef->data))
        return false;

    switch (aatyp) {
        case AT_EXPL:
        case AT_BOOM:
        case AT_GAZE:
        case AT_MAGC:
        case AT_BREA: /* assumed to be lightning */
            /* light-based attacks may be cancelled or resisted */
            if (magr && magr->mcan)
                return false;
            return !resists_blnd(mdef);

        case AT_WEAP:
        case AT_SPIT:
        case AT_NONE:
            /* an object is used (thrown/spit/other) */
            if (obj && (obj->otyp == CREAM_PIE)) {
                if (is_you && Blindfolded)
                    return false;
            } else if (obj && (obj->otyp == BLINDING_VENOM)) {
                /* all ublindf, including LENSES, protect, cream-pies too */
                if (is_you && (ublindf || u.ucreamed))
                    return false;
                check_visor = true;
            } else if (obj && (obj->otyp == POT_BLINDNESS)) {
                return true; /* no defense */
            } else
                return false; /* other objects cannot cause blindness yet */
            if ((magr == &youmonst) && u.uswallow)
                return false; /* can't affect eyes while inside monster */
            break;

        case AT_ENGL:
            if (is_you && (Blindfolded || u.usleep || u.ucreamed))
                return false;
            if (!is_you && mdef->msleeping)
                return false;
            break;

        case AT_CLAW:
            /* e.g. raven: all ublindf, including LENSES, protect */
            if (is_you && ublindf)
                return false;
            if ((magr == &youmonst) && u.uswallow)
                return false; /* can't affect eyes while inside monster */
            check_visor = true;
            break;

        case AT_TUCH:
        case AT_STNG:
            /* some physical, blind-inducing attacks can be cancelled */
            if (magr && magr->mcan)
                return false;
            break;

        default:
            break;
    }

    /* check if wearing a visor (only checked if visor might help) */
    if (check_visor) {
        o = (mdef == &youmonst) ? invent : mdef->minvent;
        for (; o; o = o->nobj) {
            if ((o->owornmask & W_ARMH) && (s = OBJ_DESCR(objects[o->otyp])) != (char *)0 && !strcmp(s, "visored helmet"))
                return false;
        }
    }

    return true;
}


/* returns true if monster can attack at range */
bool ranged_attk(const struct permonst * ptr) {
    int i, atyp;
    long atk_mask = (1L << AT_BREA) | (1L << AT_SPIT) | (1L << AT_GAZE);

    for (i = 0; i < NATTK; i++) {
        atyp = ptr->mattk[i].aatyp;
        if (atyp >= AT_WEAP)
            return true;
        /* assert(atyp < 32); */
        if ((atk_mask & (1L << atyp)) != 0L)
            return true;
    }

    return false;
}

/* returns true if monster is especially affected by silver weapons */
bool hates_silver(const struct permonst *ptr) {
    return is_were(ptr) ||
            ptr->mlet == S_VAMPIRE ||
            is_demon(ptr) ||
            ptr == &mons[PM_SHADE] ||
            (ptr->mlet == S_IMP && ptr != &mons[PM_TENGU]);
}

/* true iff the type of monster pass through iron bars */
bool passes_bars(const struct permonst *mptr) {
    return passes_walls(mptr) ||
            amorphous(mptr) ||
            is_whirly(mptr) ||
            verysmall(mptr) ||
            (slithy(mptr) && !bigmonst(mptr));
}


/* returns true if monster can track well */
bool can_track(const struct permonst * ptr) {
    if (uwep && uwep->oartifact == ART_EXCALIBUR)
        return true;
    else
        return haseyes(ptr);
}


/* creature will slide out of armor */
bool sliparm(const struct permonst * ptr) {
    return is_whirly(ptr) ||
            ptr->msize <= MZ_SMALL ||
            noncorporeal(ptr);
}

/* creature will break out of armor */
bool breakarm(const struct permonst * ptr) {
    if (sliparm(ptr))
        return false;
    return bigmonst(ptr) ||
            (ptr->msize > MZ_SMALL && !humanoid(ptr)) ||
            /* special cases of humanoids that cannot wear body armor */
            ptr == &mons[PM_MARILITH] || ptr == &mons[PM_WINGED_GARGOYLE];
}

/* creature sticks other creatures it hits */
bool sticks(const struct permonst *ptr) {
    return dmgtype(ptr, AD_STCK) ||
            dmgtype(ptr, AD_WRAP) ||
            attacktype(ptr, AT_HUGS);
}

/* number of horns this type of monster has on its head */
int num_horns(const struct permonst * ptr) {
    switch (monsndx(ptr)) {
        case PM_HORNED_DEVIL:
        case PM_MINOTAUR:
        case PM_ASMODEUS:
        case PM_BALROG:
            return 2;
        case PM_WHITE_UNICORN:
        case PM_GRAY_UNICORN:
        case PM_BLACK_UNICORN:
        case PM_KI_RIN:
            return 1;
        default:
            break;
    }
    return 0;
}

const struct attack * dmgtype_fromattack(const struct permonst *ptr, int dtyp, int atyp)
{
    const struct attack * a;
    for (a = &ptr->mattk[0]; a < &ptr->mattk[NATTK]; a++)
        if (a->adtyp == dtyp && (atyp == AT_ANY || a->aatyp == atyp))
            return a;
    return NULL;
}

bool dmgtype(const struct permonst *ptr, int dtyp) {
    return dmgtype_fromattack(ptr, dtyp, AT_ANY) ? true : false;
}

/* returns the maximum damage a defender can do to the attacker via
 * a passive defense */
int max_passive_dmg(const struct monst * mdef, const struct monst * magr) {
    for (int i = 0; i < NATTK; i++) {
        if (mdef->data->mattk[i].aatyp == AT_NONE || mdef->data->mattk[i].aatyp == AT_BOOM) {
            unsigned char adtyp = mdef->data->mattk[i].adtyp;
            if ((adtyp == AD_ACID && !resists_acid(magr)) || (adtyp == AD_COLD && !resists_cold(magr)) || (adtyp == AD_FIRE && !resists_fire(magr)) || (adtyp == AD_ELEC && !resists_elec(magr)) || adtyp == AD_PHYS) {
                int dmg = mdef->data->mattk[i].damn;
                if (!dmg)
                    dmg = mdef->data->mlevel + 1;
                dmg *= mdef->data->mattk[i].damd;
                return dmg;
            } else
                return 0;
        }
    }
    return 0;
}


/* return an index into the mons array */
int monsndx(const struct permonst *ptr) {
    int i = (int)(ptr - &mons[0]);
    if (i < LOW_PM || i >= NUMMONS) {
        panic("monsndx - could not index monster (%p)", ptr);
        return NON_PM; /* will not get here */
    }
    return (i);
}


int name_to_mon (const char *in_str) {
    /* Be careful.  We must check the entire string in case it was
     * something such as "ettin zombie corpse".  The calling routine
     * doesn't know about the "corpse" until the monster name has
     * already been taken off the front, so we have to be able to
     * read the name with extraneous stuff such as "corpse" stuck on
     * the end.
     * This causes a problem for names which prefix other names such
     * as "ettin" on "ettin zombie".  In this case we want the _longest_
     * name which exists.
     * This also permits plurals created by adding suffixes such as 's'
     * or 'es'.  Other plurals must still be handled explicitly.
     */
    int i;
    int mntmp = NON_PM;
    char *s, *str, *term;
    char buf[BUFSZ];
    int len, slen;

    str = strcpy(buf, in_str);

    if (!strncmp(str, "a ", 2))
        str += 2;
    else if (!strncmp(str, "an ", 3))
        str += 3;

    slen = strlen(str);
    term = str + slen;

    if ((s = strstri(str, "vortices")) != 0)
        strcpy(s + 4, "ex");
    /* be careful with "ies"; "priest", "zombies" */
    else if (slen > 3 && !strcmpi(term - 3, "ies") && (slen < 7 || strcmpi(term - 7, "zombies")))
        strcpy(term - 3, "y");
    /* luckily no monster names end in fe or ve with ves plurals */
    else if (slen > 3 && !strcmpi(term - 3, "ves"))
        strcpy(term - 3, "f");

    slen = strlen(str); /* length possibly needs recomputing */

    {
        static const struct alt_spl { const char* name; short pm_val; }
        names[] = {
            /* Alternate spellings */
                { "grey dragon",        PM_GRAY_DRAGON },
                { "baby grey dragon",   PM_BABY_GRAY_DRAGON },
                { "grey unicorn",       PM_GRAY_UNICORN },
                { "grey ooze",          PM_GRAY_OOZE },
                { "gray-elf",           PM_GREY_ELF },
            /* Hyphenated names */
                { "ki rin",             PM_KI_RIN },
                { "uruk hai",           PM_URUK_HAI },
                { "orc captain",        PM_ORC_CAPTAIN },
                { "woodland elf",       PM_WOODLAND_ELF },
                { "green elf",          PM_GREEN_ELF },
                { "grey elf",           PM_GREY_ELF },
                { "gray elf",           PM_GREY_ELF },
                { "elf lord",           PM_ELF_LORD },
                { "olog hai",           PM_OLOG_HAI },
                { "arch lich",          PM_ARCH_LICH },
            /* Some irregular plurals */
                { "incubi",             PM_INCUBUS },
                { "succubi",            PM_SUCCUBUS },
                { "violet fungi",       PM_VIOLET_FUNGUS },
                { "homunculi",          PM_HOMUNCULUS },
                { "baluchitheria",      PM_BALUCHITHERIUM },
                { "lurkers above",      PM_LURKER_ABOVE },
                { "cavemen",            PM_CAVEMAN },
                { "cavewomen",          PM_CAVEWOMAN },
                { "djinn",              PM_DJINNI },
                { "mumakil",            PM_MUMAK },
                { "erinyes",            PM_ERINYS },
            /* falsely caught by -ves check above */
                { "master of thief",    PM_MASTER_OF_THIEVES },
            /* end of list */
                { 0, 0 }
        };
        const struct alt_spl *namep;

        for (namep = names; namep->name; namep++)
            if (!strncmpi(str, namep->name, (int)strlen(namep->name)))
                return namep->pm_val;
    }

    for (len = 0, i = LOW_PM; i < NUMMONS; i++) {
        int m_i_len = strlen(mons[i].mname);
        if (m_i_len > len && !strncmpi(mons[i].mname, str, m_i_len)) {
            if (m_i_len == slen)
                return i; /* exact match */
            else if (slen > m_i_len &&
                     (str[m_i_len] == ' ' ||
                      !strcmpi(&str[m_i_len], "s") ||
                      !strncmpi(&str[m_i_len], "s ", 2) ||
                      !strcmpi(&str[m_i_len], "'") ||
                      !strncmpi(&str[m_i_len], "' ", 2) ||
                      !strcmpi(&str[m_i_len], "'s") ||
                      !strncmpi(&str[m_i_len], "'s ", 3) ||
                      !strcmpi(&str[m_i_len], "es") ||
                      !strncmpi(&str[m_i_len], "es ", 3))) {
                mntmp = i;
                len = m_i_len;
            }
        }
    }
    if (mntmp == NON_PM)
        mntmp = title_to_mon(str, (int *)0, (int *)0);
    return mntmp;
}


/* returns 3 values (0=male, 1=female, 2=none) */
int gender(const struct monst *mtmp) {
    if (is_neuter(mtmp->data))
        return 2;
    return mtmp->female;
}

/* Like gender(), but lower animals and such are still "it". */
/* This is the one we want to use when printing messages. */
int pronoun_gender(const struct monst * mtmp) {
    if (is_neuter(mtmp->data) || !canspotmon(mtmp))
        return 2;
    if (humanoid(mtmp->data) || (mtmp->data->geno & G_UNIQ) || type_is_pname(mtmp->data))
        return (int)mtmp->female;
    return 2;
}


/* used for nearby monsters when you go to another level */
bool levl_follower(const struct monst *mtmp) {
    /* monsters with the Amulet--even pets--won't follow across levels */
    if (mon_has_amulet(mtmp))
        return false;

    /* some monsters will follow even while intending to flee from you */
    if (mtmp->mtame || mtmp->iswiz || is_fshk(mtmp))
        return true;

    /* stalking types follow, but won't when fleeing unless you hold
     the Amulet */
    return (mtmp->data->mflags2 & M2_STALK) && (!mtmp->mflee || u.uhave.amulet);
}

static const short grownups[][2] = {
        {PM_CHICKATRICE, PM_COCKATRICE},
        {PM_LITTLE_DOG, PM_DOG}, {PM_DOG, PM_LARGE_DOG},
        {PM_HELL_HOUND_PUP, PM_HELL_HOUND},
        {PM_WINTER_WOLF_CUB, PM_WINTER_WOLF},
        {PM_KITTEN, PM_HOUSECAT}, {PM_HOUSECAT, PM_LARGE_CAT},
        {PM_PONY, PM_HORSE}, {PM_HORSE, PM_WARHORSE},
        {PM_KOBOLD, PM_LARGE_KOBOLD}, {PM_LARGE_KOBOLD, PM_KOBOLD_LORD},
        {PM_GNOME, PM_GNOME_LORD}, {PM_GNOME_LORD, PM_GNOME_KING},
        {PM_DWARF, PM_DWARF_LORD}, {PM_DWARF_LORD, PM_DWARF_KING},
        {PM_MIND_FLAYER, PM_MASTER_MIND_FLAYER},
        {PM_ORC, PM_ORC_CAPTAIN}, {PM_HILL_ORC, PM_ORC_CAPTAIN},
        {PM_MORDOR_ORC, PM_ORC_CAPTAIN}, {PM_URUK_HAI, PM_ORC_CAPTAIN},
        {PM_SEWER_RAT, PM_GIANT_RAT},
        {PM_CAVE_SPIDER, PM_GIANT_SPIDER},
        {PM_OGRE, PM_OGRE_LORD}, {PM_OGRE_LORD, PM_OGRE_KING},
        {PM_ELF, PM_ELF_LORD}, {PM_WOODLAND_ELF, PM_ELF_LORD},
        {PM_GREEN_ELF, PM_ELF_LORD}, {PM_GREY_ELF, PM_ELF_LORD},
        {PM_ELF_LORD, PM_ELVENKING},
        {PM_LICH, PM_DEMILICH}, {PM_DEMILICH, PM_MASTER_LICH},
        {PM_MASTER_LICH, PM_ARCH_LICH},
        {PM_VAMPIRE, PM_VAMPIRE_LORD}, {PM_BAT, PM_GIANT_BAT},
        {PM_BABY_GRAY_DRAGON, PM_GRAY_DRAGON},
        {PM_BABY_SILVER_DRAGON, PM_SILVER_DRAGON},
        {PM_BABY_RED_DRAGON, PM_RED_DRAGON},
        {PM_BABY_WHITE_DRAGON, PM_WHITE_DRAGON},
        {PM_BABY_ORANGE_DRAGON, PM_ORANGE_DRAGON},
        {PM_BABY_BLACK_DRAGON, PM_BLACK_DRAGON},
        {PM_BABY_BLUE_DRAGON, PM_BLUE_DRAGON},
        {PM_BABY_GREEN_DRAGON, PM_GREEN_DRAGON},
        {PM_BABY_YELLOW_DRAGON, PM_YELLOW_DRAGON},
        {PM_RED_NAGA_HATCHLING, PM_RED_NAGA},
        {PM_BLACK_NAGA_HATCHLING, PM_BLACK_NAGA},
        {PM_GOLDEN_NAGA_HATCHLING, PM_GOLDEN_NAGA},
        {PM_GUARDIAN_NAGA_HATCHLING, PM_GUARDIAN_NAGA},
        {PM_SMALL_MIMIC, PM_LARGE_MIMIC}, {PM_LARGE_MIMIC, PM_GIANT_MIMIC},
        {PM_BABY_LONG_WORM, PM_LONG_WORM},
        {PM_BABY_PURPLE_WORM, PM_PURPLE_WORM},
        {PM_BABY_CROCODILE, PM_CROCODILE},
        {PM_SOLDIER, PM_SERGEANT},
        {PM_SERGEANT, PM_LIEUTENANT},
        {PM_LIEUTENANT, PM_CAPTAIN},
        {PM_WATCHMAN, PM_WATCH_CAPTAIN},
        {PM_ALIGNED_PRIEST, PM_HIGH_PRIEST},
        {PM_STUDENT, PM_ARCHEOLOGIST},
        {PM_ATTENDANT, PM_HEALER},
        {PM_PAGE, PM_KNIGHT},
        {PM_ACOLYTE, PM_PRIEST},
        {PM_APPRENTICE, PM_WIZARD},
        {PM_MANES,PM_LEMURE},
        {PM_KEYSTONE_KOP, PM_KOP_SERGEANT},
        {PM_KOP_SERGEANT, PM_KOP_LIEUTENANT},
        {PM_KOP_LIEUTENANT, PM_KOP_KAPTAIN},
        {NON_PM,NON_PM}
};

int little_to_big(int montype) {
    for (int i = 0; grownups[i][0] >= LOW_PM; i++)
        if (montype == grownups[i][0])
            return grownups[i][1];
    return montype;
}

int big_to_little(int montype) {
    for (int i = 0; grownups[i][0] >= LOW_PM; i++)
        if (montype == grownups[i][1])
            return grownups[i][0];
    return montype;
}

/*
 * Return the permonst ptr for the race of the monster.
 * Returns correct pointer for non-polymorphed and polymorphed
 * player.  It does not return a pointer to player role character.
 */
const struct permonst * raceptr(const struct monst *mtmp) {
    if (mtmp == &youmonst && !Upolyd)
        return (&mons[urace.malenum]);
    return (mtmp->data);
}

static const char *levitate[4]  = { "float", "Float", "wobble", "Wobble" };
static const char *flys[4]      = { "fly", "Fly", "flutter", "Flutter" };
static const char *flyl[4]      = { "fly", "Fly", "stagger", "Stagger" };
static const char *slither[4]   = { "slither", "Slither", "falter", "Falter" };
static const char *ooze[4]      = { "ooze", "Ooze", "tremble", "Tremble" };
static const char *immobile[4]  = { "wiggle", "Wiggle", "pulsate", "Pulsate" };
static const char *crawl[4]     = { "crawl", "Crawl", "falter", "Falter" };

const char * locomotion(const struct permonst * ptr, const char * def) {
    int capitalize = (*def == highc(*def));
    if (is_floater(ptr))
        return levitate[capitalize];
    if (is_flyer(ptr) && ptr->msize <= MZ_SMALL)
        return flys[capitalize];
    if (is_flyer(ptr) && ptr->msize > MZ_SMALL)
        return flyl[capitalize];
    if (slithy(ptr))
        return slither[capitalize];
    if (amorphous(ptr))
        return ooze[capitalize];
    if (!ptr->mmove)
        return immobile[capitalize];
    if (nolimbs(ptr))
        return crawl[capitalize];
    return def;

}

const char * stagger(const struct permonst *ptr, const char *def) {
    int capitalize = 2 + (*def == highc(*def));

    if (is_floater(ptr))
        return levitate[capitalize];
    if (is_flyer(ptr) && ptr->msize <= MZ_SMALL)
        return flys[capitalize];
    if (is_flyer(ptr) && ptr->msize > MZ_SMALL)
        return flyl[capitalize];
    if (slithy(ptr))
        return slither[capitalize];
    if (amorphous(ptr))
        return ooze[capitalize];
    if (!ptr->mmove)
        return immobile[capitalize];
    if (nolimbs(ptr))
        return crawl[capitalize];
    return def;
}

/* return a phrase describing the effect of fire attack on a type of monster */
const char * on_fire(const struct permonst * mptr, const struct attack * mattk) {
    switch (monsndx(mptr)) {
        case PM_FLAMING_SPHERE:
        case PM_FIRE_VORTEX:
        case PM_FIRE_ELEMENTAL:
        case PM_SALAMANDER:
            return "already on fire";
        case PM_WATER_ELEMENTAL:
        case PM_FOG_CLOUD:
        case PM_STEAM_VORTEX:
            return "boiling";
        case PM_ICE_VORTEX:
        case PM_GLASS_GOLEM:
            return "melting";
        case PM_STONE_GOLEM:
        case PM_CLAY_GOLEM:
        case PM_GOLD_GOLEM:
        case PM_AIR_ELEMENTAL:
        case PM_EARTH_ELEMENTAL:
        case PM_DUST_VORTEX:
        case PM_ENERGY_VORTEX:
            return "heating up";
        default:
            return (mattk->aatyp == AT_HUGS) ? "being roasted" : "on fire";
    }
}

