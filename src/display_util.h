
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


#define vobj_at(x,y) (level.objects[x][y])

/*
 * sensemon()
 *
 * Returns true if the hero can sense the given monster.  This includes
 * monsters that are hiding or mimicing other monsters.
 */
#define tp_sensemon(mon) (      /* The hero can always sense a monster IF:  */\
    (!mindless(mon->data)) &&   /* 1. the monster has a brain to sense AND  */\
      ((Blind && Blind_telepat) ||      /* 2a. hero is blind and telepathic OR      */\
                                /* 2b. hero is using a telepathy inducing   */\
                                /*       object and in range                */\
      (Unblind_telepat &&                                             \
        (distu(mon->mx, mon->my) <= (BOLT_LIM * BOLT_LIM))))                  \
)

#define MATCH_WARN_OF_MON(mon)   (Warn_of_mon && flags.warntype && \
                                 (flags.warntype & (mon)->data->mflags2))

#define sensemon(mon) (tp_sensemon(mon) || Detect_monsters || MATCH_WARN_OF_MON(mon))

/*
 * mon_warning() is used to warn of any dangerous monsters in your
 * vicinity, and a glyph representing the warning level is displayed.
 */

#define mon_warning(mon) (Warning && !(mon)->mpeaceful &&                               \
                         (distu((mon)->mx, (mon)->my) < 100) &&                         \
                         (((int) ((mon)->m_lev / 4)) >= flags.warnlevel))

/*
 * mon_visible()
 *
 * Returns true if the hero can see the monster.  It is assumed that the
 * hero can physically see the location of the monster.  The function
 * vobj_at() returns a pointer to an object that the hero can see there.
 * Infravision is not taken into account.
 */
#define mon_visible(mon) (              /* The hero can see the monster     */\
                                        /* IF the monster                   */\
    (!mon->minvis || See_invisible()) &&  /* 1. is not invisible AND          */\
    (!mon->mundetected) &&              /* 2. not an undetected hider       */\
    (!(mon->mburied || u.uburied))      /* 3. neither you or it is buried   */\
)

/*
 * see_with_infrared()
 *
 * This function is true if the player can see a monster using infravision.
 * The caller must check for invisibility (invisible monsters are also
 * invisible to infravision), because this is usually called from within
 * canseemon() or canspotmon() which already check that.
 */
#define see_with_infrared(mon) (!Blind && Infravision && infravisible(mon->data) && couldsee(mon->mx, mon->my))


/*
 * canseemon()
 *
 * This is the globally used canseemon().  It is not called within the display
 * routines.  Like mon_visible(), but it checks to see if the hero sees the
 * location instead of assuming it.  (And also considers worms.)
 */
#define canseemon(mon) ((mon->wormno ? worm_known(mon) : \
            (cansee(mon->mx, mon->my) || see_with_infrared(mon))) \
        && mon_visible(mon))


/*
 * canspotmon(mon)
 *
 * This function checks whether you can either see a monster or sense it by
 * telepathy, and is what you usually call for monsters about which nothing is
 * known.
 */
#define canspotmon(mon) \
        (canseemon(mon) || sensemon(mon))

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
#define knowninvisible(mon) \
        (mtmp->minvis && \
            ((cansee(mon->mx, mon->my) && (See_invisible() || Detect_monsters)) || \
                (!Blind && (HTelepat & ~INTRINSIC) && \
                    distu(mon->mx, mon->my) <= (BOLT_LIM * BOLT_LIM) \
                ) \
            ) \
        )

/*
 * is_safepet(mon)
 *
 * A special case check used in attack() and domove().  Placing the
 * definition here is convenient.
 */
#define is_safepet(mon) \
        (mon && mon->mtame && canspotmon(mon) && flags.safe_dog \
                && !Confusion() && !Hallucination() && !Stunned())


/*
 * canseeself()
 * senseself()
 *
 * This returns true if the hero can see her/himself.
 *
 * The u.uswallow check assumes that you can see yourself even if you are
 * invisible.  If not, then we don't need the check.
 */
#define canseeself()    (Blind || u.uswallow || (!Invisible && !u.uundetected))
#define senseself()     (canseeself() || Unblind_telepat || Detect_monsters)

/*
 * random_monster()
 * random_object()
 * random_trap()
 *
 * Respectively return a random monster, object, or trap number.
 */
#define random_monster() rn2(NUMMONS)
#define random_object()  rn1(NUM_OBJECTS-1,1)
#define random_trap()    rn1(TRAPNUM-1,1)

/*
 * what_obj()
 * what_mon()
 * what_trap()
 *
 * If hallucinating, choose a random object/monster, otherwise, use the one
 * given.
 */
#define what_obj(obj)   (Hallucination() ? random_object()  : obj)
#define what_mon(mon)   (Hallucination() ? random_monster() : mon)
#define what_trap(trp)  (Hallucination() ? random_trap()    : trp)

/*
 * covers_objects()
 * covers_traps()
 *
 * These routines are true if what is really at the given location will
 * "cover" any objects or traps that might be there.
 */
#define covers_objects(xx,yy)                                                 \
    ((is_pool(xx,yy) && !Underwater) || (levl[xx][yy].typ == LAVAPOOL))

#define covers_traps(xx,yy)     covers_objects(xx,yy)


#define warning_to_glyph(mwarnlev) ((mwarnlev)+GLYPH_WARNING_OFF)
#define mon_to_glyph(mon) ((int) what_mon(monsndx((mon)->data))+GLYPH_MON_OFF)
#define detected_mon_to_glyph(mon) ((int) what_mon(monsndx((mon)->data))+GLYPH_DETECT_OFF)
#define ridden_mon_to_glyph(mon) ((int) what_mon(monsndx((mon)->data))+GLYPH_RIDDEN_OFF)
#define pet_to_glyph(mon) ((int) what_mon(monsndx((mon)->data))+GLYPH_PET_OFF)

/* This has the unfortunate side effect of needing a global variable    */
/* to store a result. 'otg_temp' is defined and declared in decl.{ch}.  */
#define obj_to_glyph(obj)                                                     \
    (Hallucination() ?                                                          \
        ((otg_temp = random_object()) == CORPSE ?                             \
            random_monster() + GLYPH_BODY_OFF :                               \
            otg_temp + GLYPH_OBJ_OFF)   :                                     \
        ((obj)->otyp == CORPSE ?                                              \
            (int) (obj)->corpsenm + GLYPH_BODY_OFF :                          \
            (int) (obj)->otyp + GLYPH_OBJ_OFF))

#define cmap_to_glyph(cmap_idx) ((int) (cmap_idx)   + GLYPH_CMAP_OFF)
#define explosion_to_glyph(expltype,idx)        \
                ((((expltype) * MAXEXPCHARS) + ((idx) - S_explode1)) + GLYPH_EXPLODE_OFF)

#define trap_to_glyph(trap)     \
                        cmap_to_glyph(trap_to_defsym(what_trap((trap)->ttyp)))

/* Not affected by hallucination.  Gives a generic body for CORPSE */
#define objnum_to_glyph(onum)   ((int) (onum) + GLYPH_OBJ_OFF)
#define monnum_to_glyph(mnum)   ((int) (mnum) + GLYPH_MON_OFF)
#define detected_monnum_to_glyph(mnum)  ((int) (mnum) + GLYPH_DETECT_OFF)
#define ridden_monnum_to_glyph(mnum)    ((int) (mnum) + GLYPH_RIDDEN_OFF)
#define petnum_to_glyph(mnum)   ((int) (mnum) + GLYPH_PET_OFF)

/* The hero's glyph when seen as a monster.
 */
#define hero_glyph \
        monnum_to_glyph((Upolyd || !iflags.showrace) ? u.umonnum : \
                        (flags.female && urace.femalenum != NON_PM) ? urace.femalenum : \
                        urace.malenum)


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
#define glyph_to_mon(glyph)                                             \
        (glyph_is_normal_monster(glyph) ? ((glyph)-GLYPH_MON_OFF) :     \
        glyph_is_pet(glyph) ? ((glyph)-GLYPH_PET_OFF) :                 \
        glyph_is_detected_monster(glyph) ? ((glyph)-GLYPH_DETECT_OFF) : \
        glyph_is_ridden_monster(glyph) ? ((glyph)-GLYPH_RIDDEN_OFF) :   \
        NO_GLYPH)
#define glyph_to_obj(glyph)                                             \
        (glyph_is_body(glyph) ? CORPSE :                                \
        glyph_is_normal_object(glyph) ? ((glyph)-GLYPH_OBJ_OFF) :       \
        NO_GLYPH)
#define glyph_to_trap(glyph)                                            \
        (glyph_is_trap(glyph) ?                                         \
                ((int) defsym_to_trap((glyph) - GLYPH_CMAP_OFF)) :      \
        NO_GLYPH)
#define glyph_to_cmap(glyph)                                            \
        (glyph_is_cmap(glyph) ? ((glyph) - GLYPH_CMAP_OFF) :            \
        NO_GLYPH)
#define glyph_to_swallow(glyph)                                         \
        (glyph_is_swallow(glyph) ? (((glyph) - GLYPH_SWALLOW_OFF) & 0x7) : \
        0)
#define glyph_to_warning(glyph)                                         \
        (glyph_is_warning(glyph) ? ((glyph) - GLYPH_WARNING_OFF) :      \
        NO_GLYPH);

/*
 * Return true if the given glyph is what we want.  Note that bodies are
 * considered objects.
 */
#define glyph_is_monster(glyph)                                         \
                (glyph_is_normal_monster(glyph)                         \
                || glyph_is_pet(glyph)                                  \
                || glyph_is_ridden_monster(glyph)                       \
                || glyph_is_detected_monster(glyph))
#define glyph_is_normal_monster(glyph)                                  \
    ((glyph) >= GLYPH_MON_OFF && (glyph) < (GLYPH_MON_OFF+NUMMONS))
#define glyph_is_pet(glyph)                                             \
    ((glyph) >= GLYPH_PET_OFF && (glyph) < (GLYPH_PET_OFF+NUMMONS))
#define glyph_is_body(glyph)                                            \
    ((glyph) >= GLYPH_BODY_OFF && (glyph) < (GLYPH_BODY_OFF+NUMMONS))
#define glyph_is_ridden_monster(glyph)                                  \
    ((glyph) >= GLYPH_RIDDEN_OFF && (glyph) < (GLYPH_RIDDEN_OFF+NUMMONS))
#define glyph_is_detected_monster(glyph)                                \
    ((glyph) >= GLYPH_DETECT_OFF && (glyph) < (GLYPH_DETECT_OFF+NUMMONS))
#define glyph_is_invisible(glyph) ((glyph) == GLYPH_INVISIBLE)
#define glyph_is_normal_object(glyph)                                   \
    ((glyph) >= GLYPH_OBJ_OFF && (glyph) < (GLYPH_OBJ_OFF+NUM_OBJECTS))
#define glyph_is_object(glyph)                                          \
                (glyph_is_normal_object(glyph)                          \
                || glyph_is_body(glyph))
#define glyph_is_trap(glyph)                                            \
    ((glyph) >= (GLYPH_CMAP_OFF+trap_to_defsym(1)) &&                   \
     (glyph) <  (GLYPH_CMAP_OFF+trap_to_defsym(1)+TRAPNUM))
#define glyph_is_cmap(glyph)                                            \
    ((glyph) >= GLYPH_CMAP_OFF && (glyph) < (GLYPH_CMAP_OFF+MAXPCHARS))
#define glyph_is_swallow(glyph) \
    ((glyph) >= GLYPH_SWALLOW_OFF && (glyph) < (GLYPH_SWALLOW_OFF+(NUMMONS << 3)))
#define glyph_is_warning(glyph) \
    ((glyph) >= GLYPH_WARNING_OFF && (glyph) < (GLYPH_WARNING_OFF + WARNCOUNT))


/*
 * display_self()
 *
 * Display the hero.  It is assumed that all checks necessary to determine
 * _if_ the hero can be seen have already been done.
 */
static void display_self(void) {
    show_glyph(u.ux, u.uy, (u.usteed && mon_visible(u.usteed)) ?  ridden_mon_to_glyph(u.usteed) :
        youmonst.m_ap_type == M_AP_NOTHING ?
                                hero_glyph :
        youmonst.m_ap_type == M_AP_FURNITURE ?
                                cmap_to_glyph(youmonst.mappearance) :
        youmonst.m_ap_type == M_AP_OBJECT ?
                                objnum_to_glyph(youmonst.mappearance) :
        /* else M_AP_MONSTER */ monnum_to_glyph(youmonst.mappearance));
}
