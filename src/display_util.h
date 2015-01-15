
#include "display.h"

#include "dbridge.h"
#include "decl.h"
#include "flag.h"
#include "hack.h"
#include "mondata.h"
#include "monst.h"
#include "permonst.h"
#include "pm_props.h"
#include "prop.h"
#include "rm.h"
#include "rnd.h"
#include "vision.h"
#include "worm.h"
#include "you.h"
#include "youprop.h"


static struct obj * vobj_at(signed char x, signed char y) {
    return level.objects[x][y];
}

/*
 * sensemon()
 *
 * Returns true if the hero can sense the given monster.  This includes
 * monsters that are hiding or mimicing other monsters.
 */
static bool tp_sensemon(const struct monst * mon) {
    /* The hero can always sense a monster IF:  */
    /* 1. the monster has a brain to sense AND  */
    if (mindless(mon->data))
        return false;
    /* 2a. hero is blind and telepathic OR      */
    if (Blind() && Blind_telepat)
        return true;
    /* 2b. hero is using a telepathy inducing   */
    /*       object and in range                */
    if (Unblind_telepat && (distu(mon->mx, mon->my) <= (BOLT_LIM * BOLT_LIM)))
        return true;
    return false;
}

static bool MATCH_WARN_OF_MON(const struct monst * mon) {
    return (Warn_of_mon && flags.warntype && (flags.warntype & (mon)->data->mflags2));
}

static bool sensemon(const struct monst * mon) {
    return (tp_sensemon(mon) || Detect_monsters || MATCH_WARN_OF_MON(mon));
}

/*
 * mon_warning() is used to warn of any dangerous monsters in your
 * vicinity, and a glyph representing the warning level is displayed.
 */
static bool mon_warning(const struct monst * mon) {
    return (Warning && !(mon)->mpeaceful && (distu((mon)->mx, (mon)->my) < 100) && (((int)((mon)->m_lev / 4)) >= flags.warnlevel));
}

/*
 * mon_visible()
 *
 * Returns true if the hero can see the monster.  It is assumed that the
 * hero can physically see the location of the monster.  The function
 * vobj_at() returns a pointer to an object that the hero can see there.
 * Infravision is not taken into account.
 */
static bool mon_visible(const struct monst * mon) {
    /* The hero can see the monster     */
    /* IF the monster                   */
    /* 1. is not invisible AND          */
    /* 2. not an undetected hider       */
    /* 3. neither you or it is buried   */
    return ((!mon->minvis || See_invisible()) && (!mon->mundetected) && (!(mon->mburied || u.uburied)));
}

/*
 * see_with_infrared()
 *
 * This function is true if the player can see a monster using infravision.
 * The caller must check for invisibility (invisible monsters are also
 * invisible to infravision), because this is usually called from within
 * canseemon() or canspotmon() which already check that.
 */
static bool see_with_infrared(const struct monst * mon) {
    return (!Blind() && Infravision && infravisible(mon->data) && couldsee(mon->mx, mon->my));
}


/*
 * canseemon()
 *
 * This is the globally used canseemon().  It is not called within the display
 * routines.  Like mon_visible(), but it checks to see if the hero sees the
 * location instead of assuming it.  (And also considers worms.)
 */
static bool canseemon(const struct monst * mon) {
    return ((mon->wormno ? worm_known(mon) : (cansee(mon->mx, mon->my) || see_with_infrared(mon))) && mon_visible(mon));
}


/*
 * canspotmon(mon)
 *
 * This function checks whether you can either see a monster or sense it by
 * telepathy, and is what you usually call for monsters about which nothing is
 * known.
 */
static bool canspotmon(const struct monst * mon) {
    return (canseemon(mon) || sensemon(mon));
}

/* knowninvisible(mon)
 * This one checks to see if you know a monster is both there and invisible.
 * 1) If you can see the monster and have see invisible, it is assumed the
 * monster is transparent, but visible in some manner.  (Earlier versions of
 * Nethack were really inconsistent on this.)
 * 2) If you can't see the monster, but can see its location and you have
 * telepathy that works when you can see, you can tell that there is a
 * creature in an apparently empty spot.
 * Infravision is not relevant; we assume that invisible monsters are also
 * invisible to infravision.
 */
static bool knowninvisible(const struct monst * mon) {
    return (mon->minvis && ((cansee(mon->mx, mon->my) && (See_invisible() || Detect_monsters)) || (!Blind() && (HTelepat& ~INTRINSIC) && distu(mon->mx, mon->my) <= (BOLT_LIM * BOLT_LIM))));
}

/*
 * is_safepet(mon)
 *
 * A special case check used in attack() and domove().  Placing the
 * definition here is convenient.
 */
static bool is_safepet(const struct monst * mon) {
    return (mon && mon->mtame && canspotmon(mon) && flags.safe_dog && !Confusion() && !Hallucination() && !Stunned());
}


/*
 * canseeself()
 * senseself()
 *
 * This returns true if the hero can see her/himself.
 *
 * The u.uswallow check assumes that you can see yourself even if you are
 * invisible.  If not, then we don't need the check.
 */
static bool canseeself(void) {
    return (Blind() || u.uswallow || (!Invisible && !u.uundetected));
}
static bool senseself(void) {
    return (canseeself() || Unblind_telepat || Detect_monsters);
}

/*
 * random_monster()
 * random_object()
 * random_trap()
 *
 * Respectively return a random monster, object, or trap number.
 */
static int random_monster(void) {
    return rn2(NUMMONS);
}
static int random_object(void) {
    return rn1(NUM_OBJECTS - 1, 1);
}
static int random_trap(void) {
    return rn1(TRAPNUM-1, 1);
}

static int what_mon(int mon) {
    return Hallucination() ? random_monster() : mon;
}
static int what_trap(int trp) {
    return Hallucination() ? random_trap() : trp;
}

/*
 * covers_objects()
 * covers_traps()
 *
 * These routines are true if what is really at the given location will
 * "cover" any objects or traps that might be there.
 */
static bool covers_objects(int xx, int yy) {
    return (is_pool(xx, yy) && !Underwater) || levl[xx][yy].typ == LAVAPOOL;
}
static bool covers_traps(int xx, int yy) {
    return covers_objects(xx, yy);
}


static int warning_to_glyph(int mwarnlev) {
    return mwarnlev + GLYPH_WARNING_OFF;
}
static int mon_to_glyph(const struct monst * mon) {
    return what_mon(monsndx(mon->data)) + GLYPH_MON_OFF;
}
static int detected_mon_to_glyph(const struct monst * mon) {
    return what_mon(monsndx(mon->data)) + GLYPH_DETECT_OFF;
}
static int ridden_mon_to_glyph(const struct monst * mon) {
    return what_mon(monsndx(mon->data)) + GLYPH_RIDDEN_OFF;
}
static int pet_to_glyph(const struct monst * mon) {
    return what_mon(monsndx(mon->data)) + GLYPH_PET_OFF;
}

static int obj_to_glyph(const struct obj * obj) {
    if (Hallucination()) {
        int otg_temp = random_object();
        if (otg_temp == CORPSE)
            return random_monster() + GLYPH_BODY_OFF;
        else
            return otg_temp + GLYPH_OBJ_OFF;
    } else {
        if (obj->otyp == CORPSE)
            return obj->corpsenm + GLYPH_BODY_OFF;
        else
            return (int)obj->otyp + GLYPH_OBJ_OFF;
    }
}

static int cmap_to_glyph(unsigned cmap_idx) {
    return (int)cmap_idx + GLYPH_CMAP_OFF;
}
static int explosion_to_glyph(int expltype, int idx) {
    return ((expltype * MAXEXPCHARS) + (idx - S_explode1)) + GLYPH_EXPLODE_OFF;
}

static int trap_to_glyph(const struct trap * trap) {
    return cmap_to_glyph(trap_to_defsym(what_trap((trap)->ttyp)));
}

/* Not affected by hallucination.  Gives a generic body for CORPSE */
static int objnum_to_glyph(int onum) {
    return onum + GLYPH_OBJ_OFF;
}
static int monnum_to_glyph(int mnum) {
    return mnum + GLYPH_MON_OFF;
}
static int detected_monnum_to_glyph(int mnum) {
    return mnum + GLYPH_DETECT_OFF;
}
static int ridden_monnum_to_glyph(int mnum) {
    return mnum + GLYPH_RIDDEN_OFF;
}
static int petnum_to_glyph(int mnum) {
    return mnum + GLYPH_PET_OFF;
}

/* The hero's glyph when seen as a monster.
 */
static int hero_glyph() {
    return monnum_to_glyph((Upolyd || !iflags.showrace) ? u.umonnum : (flags.female && urace.femalenum != NON_PM) ? urace.femalenum : urace.malenum);
}

/*
 * Return true if the given glyph is what we want.
 * Note that corpses are considered objects.
 */
static bool glyph_is_normal_monster(int glyph) {
    return glyph >= GLYPH_MON_OFF && glyph < GLYPH_MON_OFF + NUMMONS;
}
static bool glyph_is_pet(int glyph) {
    return glyph >= GLYPH_PET_OFF && glyph < GLYPH_PET_OFF + NUMMONS;
}
static bool glyph_is_body(int glyph) {
    return glyph >= GLYPH_BODY_OFF && glyph < GLYPH_BODY_OFF + NUMMONS;
}
static bool glyph_is_ridden_monster(int glyph) {
    return glyph >= GLYPH_RIDDEN_OFF && glyph < GLYPH_RIDDEN_OFF + NUMMONS;
}
static bool glyph_is_detected_monster(int glyph) {
    return glyph >= GLYPH_DETECT_OFF && glyph < GLYPH_DETECT_OFF + NUMMONS;
}
static bool glyph_is_invisible(int glyph) {
    return glyph == GLYPH_INVISIBLE;
}
static bool glyph_is_normal_object(int glyph) {
    return glyph >= GLYPH_OBJ_OFF && glyph < GLYPH_OBJ_OFF + NUM_OBJECTS;
}
static bool glyph_is_object(int glyph) {
    return glyph_is_normal_object(glyph) || glyph_is_body(glyph);
}
static bool glyph_is_trap(int glyph) {
    return glyph >= GLYPH_CMAP_OFF + trap_to_defsym(1) && glyph < GLYPH_CMAP_OFF + trap_to_defsym(1) + TRAPNUM;
}
static bool glyph_is_cmap(int glyph) {
    return glyph >= GLYPH_CMAP_OFF && glyph < GLYPH_CMAP_OFF + MAXPCHARS;
}
static bool glyph_is_swallow(int glyph) {
    return glyph >= GLYPH_SWALLOW_OFF && glyph < GLYPH_SWALLOW_OFF + (NUMMONS << 3);
}
static bool glyph_is_warning(int glyph) {
    return glyph >= GLYPH_WARNING_OFF && glyph < GLYPH_WARNING_OFF + WARNCOUNT;
}
static bool glyph_is_monster(int glyph) {
    return glyph_is_normal_monster(glyph) || glyph_is_pet(glyph) || glyph_is_ridden_monster(glyph) || glyph_is_detected_monster(glyph);
}


/*
 * Change the given glyph into it's given type.  Note:
 *      1) Pets, detected, and ridden monsters are animals and are converted
 *         to the proper monster number.
 *      2) Bodies are all mapped into the generic CORPSE object
 *      3) If handed a glyph out of range for the type, these functions
 *         will return NO_GLYPH (see exception below)
 *      4) glyph_to_swallow() does not return a showsyms[] index, but an
 *         offset from the first swallow symbol.  If handed something
 *         out of range, it will return zero (for lack of anything better
 *         to return).
 */
static int glyph_to_mon(int glyph) {
    return (glyph_is_normal_monster(glyph) ? ((glyph) - GLYPH_MON_OFF) : glyph_is_pet(glyph) ? ((glyph) - GLYPH_PET_OFF) : glyph_is_detected_monster(glyph) ? ((glyph) - GLYPH_DETECT_OFF) : glyph_is_ridden_monster(glyph) ? ((glyph) - GLYPH_RIDDEN_OFF) : NO_GLYPH);
}
static int glyph_to_obj(int glyph) {
    return (glyph_is_body(glyph) ? CORPSE : glyph_is_normal_object(glyph) ? ((glyph) - GLYPH_OBJ_OFF) : NO_GLYPH);
}
static int glyph_to_trap(int glyph) {
    return (glyph_is_trap(glyph) ? ((int)defsym_to_trap((glyph) - GLYPH_CMAP_OFF)) : NO_GLYPH);
}
static int glyph_to_cmap(int glyph) {
    return (glyph_is_cmap(glyph) ? ((glyph) - GLYPH_CMAP_OFF) : NO_GLYPH);
}
static int glyph_to_swallow(int glyph) {
    return (glyph_is_swallow(glyph) ? (((glyph) - GLYPH_SWALLOW_OFF) & 0x7) : 0);
}
static int glyph_to_warning(int glyph) {
    return (glyph_is_warning(glyph) ? ((glyph) - GLYPH_WARNING_OFF) : NO_GLYPH);
}

/*
 * display_self()
 *
 * Display the hero.  It is assumed that all checks necessary to determine
 * _if_ the hero can be seen have already been done.
 */
static void display_self(void) {
    show_glyph(u.ux, u.uy, (u.usteed && mon_visible(u.usteed)) ?  ridden_mon_to_glyph(u.usteed) :
        youmonst.m_ap_type == M_AP_NOTHING ?
                                hero_glyph() :
        youmonst.m_ap_type == M_AP_FURNITURE ?
                                cmap_to_glyph(youmonst.mappearance) :
        youmonst.m_ap_type == M_AP_OBJECT ?
                                objnum_to_glyph(youmonst.mappearance) :
        /* else M_AP_MONSTER */ monnum_to_glyph(youmonst.mappearance));
}

