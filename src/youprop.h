/* See LICENSE in the root of this project for change info */

#ifndef YOUPROP_H
#define YOUPROP_H

#include <stdbool.h>
#include "prop.h"
#include "permonst.h"
#include "mondata.h"
#include "pm.h"
#include "onames.h"
#include "artifact.h"
#include "artifact_names.h"
#include "decl.h"


/*
 * HXxx refers to intrinsic bitfields while in human form.
 * EXxx refers to extrinsic bitfields from worn objects.
 * BXxx refers to the cause of the property being blocked.
 * Xxx refers to any source, including polymorph forms.
 */


/*** Resistances to troubles ***/
/* With intrinsics and extrinsics */
static long get_HFire_resistance(void) {
    return u.uprops[FIRE_RES].intrinsic;
}
static void set_HFire_resistance(long hFireRes) {
    u.uprops[FIRE_RES].intrinsic = hFireRes;
}
static long EFire_resistance(void) {
    return u.uprops[FIRE_RES].extrinsic;
}
static bool Fire_resistance(void) {
    return (get_HFire_resistance() || EFire_resistance() || resists_fire(&youmonst));
}

static long get_HCold_resistance(void) {
    return u.uprops[COLD_RES].intrinsic;
}
static void set_HCold_resistance(long hColdRes) {
    u.uprops[COLD_RES].intrinsic = hColdRes;
}
static long ECold_resistance(void) {
    return u.uprops[COLD_RES].extrinsic;
}
static bool Cold_resistance(void) {
    return (get_HCold_resistance() || ECold_resistance() || resists_cold(&youmonst));
}

static long get_HSleep_resistance(void) {
    return u.uprops[SLEEP_RES].intrinsic;
}
static void set_HSleep_resistance(long hSleepRes) {
    u.uprops[SLEEP_RES].intrinsic = hSleepRes;
}
static long ESleep_resistance(void) {
    return u.uprops[SLEEP_RES].extrinsic;
}
static bool Sleep_resistance(void) {
    return (get_HSleep_resistance() || ESleep_resistance() || resists_sleep(&youmonst));
}

static long get_HDisint_resistance(void) {
    return u.uprops[DISINT_RES].intrinsic;
}
static void set_HDisint_resistance(long hDisintRes) {
    u.uprops[DISINT_RES].intrinsic = hDisintRes;
}
static long EDisint_resistance(void) {
    return u.uprops[DISINT_RES].extrinsic;
}
static bool Disint_resistance(void) {
    return (get_HDisint_resistance() || EDisint_resistance() || resists_disint(&youmonst));
}

static long get_HShock_resistance(void) {
    return u.uprops[SHOCK_RES].intrinsic;
}
static void set_HShock_resistance(long hShockRes) {
    u.uprops[SHOCK_RES].intrinsic = hShockRes;
}
static long EShock_resistance(void) {
    return u.uprops[SHOCK_RES].extrinsic;
}
static bool Shock_resistance(void) {
    return (get_HShock_resistance() || EShock_resistance() || resists_elec(&youmonst));
}

static long get_HPoison_resistance(void) {
    return u.uprops[POISON_RES].intrinsic;
}
static void set_HPoison_resistance(long hPoisonRes) {
    u.uprops[POISON_RES].intrinsic = hPoisonRes;
}
static long EPoison_resistance(void) {
    return u.uprops[POISON_RES].extrinsic;
}
static bool Poison_resistance(void) {
    return (get_HPoison_resistance() || EPoison_resistance() || resists_poison(&youmonst));
}

static long HDrain_resistance(void) {
    return u.uprops[DRAIN_RES].intrinsic;
}
static long EDrain_resistance(void) {
    return u.uprops[DRAIN_RES].extrinsic;
}
static bool Drain_resistance(void) {
    return (HDrain_resistance() || EDrain_resistance() || resists_drli(&youmonst));
}

/* Intrinsics only */
static long HSick_resistance(void) {
    return u.uprops[SICK_RES].intrinsic;
}
static long Sick_resistance(void) {
    return (HSick_resistance()
            || youmonst.data->mlet == S_FUNGUS
            || youmonst.data == &mons[PM_GHOUL]
            || defends(AD_DISE,uwep));
}

static long Invulnerable(void) {
    return u.uprops[INVULNERABLE].intrinsic; /* [Tom] */
}

/* Extrinsics only */
static long EAntimagic(void) {
    return u.uprops[ANTIMAGIC].extrinsic;
}
static bool Antimagic(void) {
    return (EAntimagic() || (Upolyd && resists_magm(&youmonst)));
}

static long get_EAcid_resistance(void) {
    return u.uprops[ACID_RES].extrinsic;
}
static void set_EAcid_resistance(long eAcidRes) {
    u.uprops[ACID_RES].extrinsic = eAcidRes;
}
static bool Acid_resistance(void) {
    return (get_EAcid_resistance() || resists_acid(&youmonst));
}

static long EStone_resistance(void) {
    return u.uprops[STONE_RES].extrinsic;
}
static bool Stone_resistance(void) {
    return (EStone_resistance() || resists_ston(&youmonst));
}


/*** Troubles ***/
/* Pseudo-property */
#define Punished                (uball)

/* Those implemented solely as timeouts (we use just intrinsic) */
static long get_HStun(void) {
    return u.uprops[STUNNED].intrinsic;
}
static void set_HStun(long hStun) {
    u.uprops[STUNNED].intrinsic = hStun;
}
static bool Stunned(void) {
    /* Note: birds will also be stunned */
    return (get_HStun() || u.umonnum == PM_STALKER || youmonst.data->mlet == S_BAT);
}

static long get_HConfusion(void) {
    return u.uprops[CONFUSION].intrinsic;
}
static void set_HConfusion(long hConfusion) {
    u.uprops[CONFUSION].intrinsic = hConfusion;
}
static bool Confusion(void) {
    return get_HConfusion();
}

#define Blinded                 u.uprops[BLINDED].intrinsic
#define Blindfolded             (ublindf && ublindf->otyp != LENSES)
/* ...means blind because of a cover */

static bool Blind(void) {
    /* ...the Eyes operate even when you really are blind or don't have any eyes */
    if (ublindf && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD)
        return false;
    return Blinded || Blindfolded || !haseyes(youmonst.data);
}

#define Sick                    u.uprops[SICK].intrinsic
#define Stoned                  u.uprops[STONED].intrinsic
#define Strangled               u.uprops[STRANGLED].intrinsic
#define Vomiting                u.uprops[VOMITING].intrinsic
#define Glib                    u.uprops[GLIB].intrinsic
#define Slimed                  u.uprops[SLIMED].intrinsic      /* [Tom] */

/* Hallucination is solely a timeout; its resistance is extrinsic */
static bool Halluc_resistance(void) {
    return (u.uprops[HALLUC_RES].extrinsic || (Upolyd && dmgtype(youmonst.data, AD_HALU)));
}
static bool Hallucination(void) {
    return (u.uprops[HALLUC].intrinsic && !Halluc_resistance());
}

/* Timeout, plus a worn mask */
static long get_HFumbling(void) {
    return u.uprops[FUMBLING].intrinsic;
}
static void set_HFumbling(long hFumbling) {
    u.uprops[FUMBLING].intrinsic = hFumbling;
}
static long get_EFumbling(void) {
    return u.uprops[FUMBLING].extrinsic;
}
static void set_EFumbling(long eFumbling) {
    u.uprops[FUMBLING].extrinsic = eFumbling;
}
static bool Fumbling(void) {
    return (get_HFumbling() || get_EFumbling());
}

static long get_HWounded_legs(void) {
    return u.uprops[WOUNDED_LEGS].intrinsic;
}
static void set_HWounded_legs(long hWoundedLegs) {
    u.uprops[WOUNDED_LEGS].intrinsic = hWoundedLegs;
}
static long get_EWounded_legs(void) {
    return u.uprops[WOUNDED_LEGS].extrinsic;
}
static void set_EWounded_legs(long eWoundedLegs) {
    u.uprops[WOUNDED_LEGS].extrinsic = eWoundedLegs;
}
static bool Wounded_legs(void) {
    return (get_HWounded_legs() || get_EWounded_legs());
}

#define HSleeping               u.uprops[SLEEPING].intrinsic
#define ESleeping               u.uprops[SLEEPING].extrinsic
#define Sleeping                (HSleeping || ESleeping)

static long HHunger(void) {
    return u.uprops[HUNGER].intrinsic;
}
static long EHunger(void) {
    return u.uprops[HUNGER].extrinsic;
}
static bool Hunger(void) {
    return HHunger() || EHunger();
}


/*** Vision and senses ***/
static long get_HSee_invisible(void) {
    return u.uprops[SEE_INVIS].intrinsic;
}
static void set_HSee_invisible(long hSeeInvis) {
    u.uprops[SEE_INVIS].intrinsic = hSeeInvis;
}
static long ESee_invisible(void) {
    return u.uprops[SEE_INVIS].extrinsic;
}
static bool See_invisible(void) {
    return get_HSee_invisible() || ESee_invisible() || perceives(youmonst.data);
}

#define HTelepat                u.uprops[TELEPAT].intrinsic
#define ETelepat                u.uprops[TELEPAT].extrinsic
#define Blind_telepat           (HTelepat || ETelepat || \
                                 telepathic(youmonst.data))
#define Unblind_telepat         (ETelepat)

#define HWarning                u.uprops[WARNING].intrinsic
#define EWarning                u.uprops[WARNING].extrinsic
#define Warning                 (HWarning || EWarning)

/* Warning for a specific type of monster */
#define HWarn_of_mon            u.uprops[WARN_OF_MON].intrinsic
#define EWarn_of_mon            u.uprops[WARN_OF_MON].extrinsic
#define Warn_of_mon             (HWarn_of_mon || EWarn_of_mon)

#define HUndead_warning         u.uprops[WARN_UNDEAD].intrinsic
#define Undead_warning          (HUndead_warning)

#define HSearching              u.uprops[SEARCHING].intrinsic
#define ESearching              u.uprops[SEARCHING].extrinsic
#define Searching               (HSearching || ESearching)

#define HClairvoyant            u.uprops[CLAIRVOYANT].intrinsic
#define EClairvoyant            u.uprops[CLAIRVOYANT].extrinsic
#define BClairvoyant            u.uprops[CLAIRVOYANT].blocked
#define Clairvoyant             ((HClairvoyant || EClairvoyant) &&\
                                 !BClairvoyant)

#define HInfravision            u.uprops[INFRAVISION].intrinsic
#define EInfravision            u.uprops[INFRAVISION].extrinsic
#define Infravision             (HInfravision || EInfravision || \
                                  infravision(youmonst.data))

#define HDetect_monsters        u.uprops[DETECT_MONSTERS].intrinsic
#define EDetect_monsters        u.uprops[DETECT_MONSTERS].extrinsic
static bool Detect_monsters(void) {
    return HDetect_monsters|| EDetect_monsters;
}


/*** Appearance and behavior ***/
#define Adornment               u.uprops[ADORNED].extrinsic

#define HInvis                  u.uprops[INVIS].intrinsic
#define EInvis                  u.uprops[INVIS].extrinsic
#define BInvis                  u.uprops[INVIS].blocked
#define Invis                   ((HInvis || EInvis || \
                                 pm_invisible(youmonst.data)) && !BInvis)
#define Invisible               (Invis && !See_invisible())
                /* Note: invisibility also hides inventory and steed */

#define EDisplaced              u.uprops[DISPLACED].extrinsic
#define Displaced               EDisplaced

#define HStealth                u.uprops[STEALTH].intrinsic
#define EStealth                u.uprops[STEALTH].extrinsic
#define BStealth                u.uprops[STEALTH].blocked
#define Stealth                 ((HStealth || EStealth) && !BStealth)

#define HAggravate_monster      u.uprops[AGGRAVATE_MONSTER].intrinsic
#define EAggravate_monster      u.uprops[AGGRAVATE_MONSTER].extrinsic
#define Aggravate_monster       (HAggravate_monster || EAggravate_monster)

#define HConflict               u.uprops[CONFLICT].intrinsic
#define EConflict               u.uprops[CONFLICT].extrinsic
#define Conflict                (HConflict || EConflict)


/*** Transportation ***/
#define HJumping                u.uprops[JUMPING].intrinsic
#define EJumping                u.uprops[JUMPING].extrinsic
#define Jumping                 (HJumping || EJumping)

#define HTeleportation          u.uprops[TELEPORT].intrinsic
#define ETeleportation          u.uprops[TELEPORT].extrinsic
#define Teleportation           (HTeleportation || ETeleportation || \
                                 can_teleport(youmonst.data))

#define HTeleport_control       u.uprops[TELEPORT_CONTROL].intrinsic
#define ETeleport_control       u.uprops[TELEPORT_CONTROL].extrinsic
#define Teleport_control        (HTeleport_control || ETeleport_control || \
                                 control_teleport(youmonst.data))

#define HLevitation             u.uprops[LEVITATION].intrinsic
#define ELevitation             u.uprops[LEVITATION].extrinsic
#define Levitation              (HLevitation || ELevitation || \
                                 is_floater(youmonst.data))
        /* Can't touch surface, can't go under water; overrides all others */
#define Lev_at_will             (((HLevitation & I_SPECIAL) != 0L || \
                                 (ELevitation & W_ARTI) != 0L) && \
                                 (HLevitation & ~(I_SPECIAL|TIMEOUT)) == 0L && \
                                 (ELevitation & ~W_ARTI) == 0L && \
                                 !is_floater(youmonst.data))

#define EFlying                 u.uprops[FLYING].extrinsic
#define Flying                 (EFlying || is_flyer(youmonst.data) || \
                                 (u.usteed && is_flyer(u.usteed->data)))
        /* May touch surface; does not override any others */

#define Wwalking                (u.uprops[WWALKING].extrinsic && \
                                 !Is_waterlevel(&u.uz))
        /* Don't get wet, can't go under water; overrides others except levitation */
        /* Wwalking is meaningless on water level */

#define HSwimming               u.uprops[SWIMMING].intrinsic
#define ESwimming               u.uprops[SWIMMING].extrinsic    /* [Tom] */
#define Swimming               (HSwimming || ESwimming || \
                                 is_swimmer(youmonst.data) || \
                                 (u.usteed && is_swimmer(u.usteed->data)))
        /* Get wet, don't go under water unless if amphibious */

#define HMagical_breathing      u.uprops[MAGICAL_BREATHING].intrinsic
#define EMagical_breathing      u.uprops[MAGICAL_BREATHING].extrinsic
#define Amphibious              (HMagical_breathing || EMagical_breathing || \
                                 amphibious(youmonst.data))
        /* Get wet, may go under surface */

#define Breathless              (HMagical_breathing || EMagical_breathing || \
                                 breathless(youmonst.data))

#define Underwater              (u.uinwater)
/* Note that Underwater and u.uinwater are both used in code.
   The latter form is for later implementation of other in-water
   states, like swimming, wading, etc. */

#define HPasses_walls           u.uprops[PASSES_WALLS].intrinsic
#define EPasses_walls           u.uprops[PASSES_WALLS].extrinsic
#define Passes_walls            (HPasses_walls || EPasses_walls || \
                                 passes_walls(youmonst.data))


/*** Physical attributes ***/
#define HSlow_digestion         u.uprops[SLOW_DIGESTION].intrinsic
#define ESlow_digestion         u.uprops[SLOW_DIGESTION].extrinsic
#define Slow_digestion          (HSlow_digestion || ESlow_digestion)  /* KMH */

#define HHalf_spell_damage      u.uprops[HALF_SPDAM].intrinsic
#define EHalf_spell_damage      u.uprops[HALF_SPDAM].extrinsic
#define Half_spell_damage       (HHalf_spell_damage || EHalf_spell_damage)

#define HHalf_physical_damage   u.uprops[HALF_PHDAM].intrinsic
#define EHalf_physical_damage   u.uprops[HALF_PHDAM].extrinsic
#define Half_physical_damage    (HHalf_physical_damage || EHalf_physical_damage)

#define HRegeneration           u.uprops[REGENERATION].intrinsic
#define ERegeneration           u.uprops[REGENERATION].extrinsic
#define Regeneration            (HRegeneration || ERegeneration || \
                                 regenerates(youmonst.data))

#define HEnergy_regeneration    u.uprops[ENERGY_REGENERATION].intrinsic
#define EEnergy_regeneration    u.uprops[ENERGY_REGENERATION].extrinsic
#define Energy_regeneration     (HEnergy_regeneration || EEnergy_regeneration)

#define HProtection             u.uprops[PROTECTION].intrinsic
#define EProtection             u.uprops[PROTECTION].extrinsic
#define Protection              (HProtection || EProtection)

#define HProtection_from_shape_changers \
                                u.uprops[PROT_FROM_SHAPE_CHANGERS].intrinsic
#define EProtection_from_shape_changers \
                                u.uprops[PROT_FROM_SHAPE_CHANGERS].extrinsic
#define Protection_from_shape_changers \
                                (HProtection_from_shape_changers || \
                                 EProtection_from_shape_changers)

#define HPolymorph              u.uprops[POLYMORPH].intrinsic
#define EPolymorph              u.uprops[POLYMORPH].extrinsic
#define Polymorph               (HPolymorph || EPolymorph)

#define HPolymorph_control      u.uprops[POLYMORPH_CONTROL].intrinsic
#define EPolymorph_control      u.uprops[POLYMORPH_CONTROL].extrinsic
#define Polymorph_control       (HPolymorph_control || EPolymorph_control)

#define HUnchanging             u.uprops[UNCHANGING].intrinsic
#define EUnchanging             u.uprops[UNCHANGING].extrinsic
#define Unchanging              (HUnchanging || EUnchanging)    /* KMH */

#define HFast                   u.uprops[FAST].intrinsic
#define EFast                   u.uprops[FAST].extrinsic
#define Fast                    (HFast || EFast)
#define Very_fast               ((HFast & ~INTRINSIC) || EFast)

#define EReflecting             u.uprops[REFLECTING].extrinsic
#define Reflecting              (EReflecting || \
                                 (youmonst.data == &mons[PM_SILVER_DRAGON]))

#define Free_action             u.uprops[FREE_ACTION].extrinsic /* [Tom] */

#define Fixed_abil              u.uprops[FIXED_ABIL].extrinsic  /* KMH */

#define Lifesaved               u.uprops[LIFESAVED].extrinsic


#endif /* YOUPROP_H */
