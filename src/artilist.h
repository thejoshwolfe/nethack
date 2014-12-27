/* See LICENSE in the root of this project for change info */

#include "artifact.h"
#include "onames.h"
#include "monsym.h"
#include "prop.h"
#include "pm_props.h"

#include <stdlib.h>

static struct artifact artilist[] = {

/* Artifact cost rationale:
 * 1.  The more useful the artifact, the better its cost.
 * 2.  Quest artifacts are highly valued.
 * 3.  Chaotic artifacts are inflated due to scarcity (and balance).
 */


/*  dummy element #0, so that all interesting indices are non-zero */
{ STRANGE_OBJECT, "",
        0, 0, 0, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, 0, A_NONE, NON_PM, NON_PM, 0L },

{ LONG_SWORD, "Excalibur",
        (SPFX_NOGEN|SPFX_RESTR|SPFX_SEEK|SPFX_DEFN|SPFX_INTEL|SPFX_SEARCH),0,0,
        {0,AD_PHYS,5,10},     {0,AD_DRLI,0,0},      {0,0,0,0},        0, A_LAWFUL, PM_KNIGHT, NON_PM, 4000L },
/*
 *      Stormbringer only has a 2 because it can drain a level,
 *      providing 8 more.
 */
{ RUNESWORD, "Stormbringer",
        (SPFX_RESTR|SPFX_ATTK|SPFX_DEFN|SPFX_INTEL|SPFX_DRLI), 0, 0,
        {0,AD_DRLI,5,2},      {0,AD_DRLI,0,0},      {0,0,0,0},        0, A_CHAOTIC, NON_PM, NON_PM, 8000L },
/*
 *      Mjollnir will return to the hand of the wielder when thrown
 *      if the wielder is a Valkyrie wearing Gauntlets of Power.
 */
{ WAR_HAMMER,             /* Mjo:llnir */ "Mjollnir",
        (SPFX_RESTR|SPFX_ATTK),  0, 0,
        {0,AD_ELEC,5,24},     {0,0,0,0},        {0,0,0,0},        0, A_NEUTRAL, PM_VALKYRIE, NON_PM, 4000L },

{ BATTLE_AXE, "Cleaver",
        SPFX_RESTR, 0, 0,
        {0,AD_PHYS,3,6},      {0,0,0,0},        {0,0,0,0},        0, A_NEUTRAL, PM_BARBARIAN, NON_PM, 1500L },

{ ORCISH_DAGGER, "Grimtooth",
        SPFX_RESTR, 0, 0,
        {0,AD_PHYS,2,6},      {0,0,0,0},        {0,0,0,0},        0, A_CHAOTIC, NON_PM, PM_ORC, 300L },
/*
 *      Orcrist and Sting have same alignment as elves.
 */
{ ELVEN_BROADSWORD, "Orcrist",
        SPFX_DFLAG2, 0, M2_ORC,
        {0,AD_PHYS,5,0},      {0,0,0,0},        {0,0,0,0},        0, A_CHAOTIC, NON_PM, PM_ELF, 2000L },

/*
 *      The combination of SPFX_WARN and M2_something on an artifact
 *      will trigger EWarn_of_mon for all monsters that have the appropriate
 *      M2_something flags.  In Sting's case it will trigger EWarn_of_mon
 *      for M2_ORC monsters.
 */
{ ELVEN_DAGGER, "Sting",
        (SPFX_WARN|SPFX_DFLAG2), 0, M2_ORC,
        {0,AD_PHYS,5,0},      {0,0,0,0},        {0,0,0,0},        0, A_CHAOTIC, NON_PM, PM_ELF, 800L },
/*
 *      Magicbane is a bit different!  Its magic fanfare
 *      unbalances victims in addition to doing some damage.
 */
{ ATHAME, "Magicbane",
        (SPFX_RESTR|SPFX_ATTK|SPFX_DEFN), 0, 0,
        {0,AD_STUN,3,4},      {0,AD_MAGM,0,0},  {0,0,0,0},        0, A_NEUTRAL, PM_WIZARD, NON_PM, 3500L },

{ LONG_SWORD, "Frost Brand",
        (SPFX_RESTR|SPFX_ATTK|SPFX_DEFN), 0, 0,
        {0,AD_COLD,5,0},      {0,AD_COLD,0,0},      {0,0,0,0},        0, A_NONE, NON_PM, NON_PM, 3000L },

{ LONG_SWORD, "Fire Brand",
        (SPFX_RESTR|SPFX_ATTK|SPFX_DEFN), 0, 0,
        {0,AD_FIRE,5,0},      {0,AD_FIRE,0,0},      {0,0,0,0},        0, A_NONE, NON_PM, NON_PM, 3000L },

{ BROADSWORD, "Dragonbane",
        (SPFX_RESTR|SPFX_DCLAS), 0, S_DRAGON,
        {0,AD_PHYS,5,0},      {0,0,0,0},        {0,0,0,0},        0, A_NONE, NON_PM, NON_PM, 500L },

{ LONG_SWORD, "Demonbane",
        (SPFX_RESTR|SPFX_DFLAG2), 0, M2_DEMON,
        {0,AD_PHYS,5,0},      {0,0,0,0},        {0,0,0,0},        0, A_LAWFUL, NON_PM, NON_PM, 2500L },

{ SILVER_SABER, "Werebane",
        (SPFX_RESTR|SPFX_DFLAG2), 0, M2_WERE,
        {0,AD_PHYS,5,0},      {0,AD_WERE,0,0},  {0,0,0,0},        0, A_NONE, NON_PM, NON_PM, 1500L },

{ SILVER_SABER, "Grayswandir",
        (SPFX_RESTR|SPFX_HALRES), 0, 0,
        {0,AD_PHYS,5,0},      {0,0,0,0},        {0,0,0,0},        0, A_LAWFUL, NON_PM, NON_PM, 8000L },

{ LONG_SWORD, "Giantslayer",
        (SPFX_RESTR|SPFX_DFLAG2), 0, M2_GIANT,
        {0,AD_PHYS,5,0},      {0,0,0,0},        {0,0,0,0},        0, A_NEUTRAL, NON_PM, NON_PM, 200L },

{ WAR_HAMMER, "Ogresmasher",
        (SPFX_RESTR|SPFX_DCLAS), 0, S_OGRE,
        {0,AD_PHYS,5,0},      {0,0,0,0},        {0,0,0,0},        0, A_NONE, NON_PM, NON_PM, 200L },

{ MORNING_STAR, "Trollsbane",
        (SPFX_RESTR|SPFX_DCLAS), 0, S_TROLL,
        {0,AD_PHYS,5,0},      {0,0,0,0},        {0,0,0,0},        0, A_NONE, NON_PM, NON_PM, 200L },
/*
 *      Two problems:  1) doesn't let trolls regenerate heads,
 *      2) doesn't give unusual message for 2-headed monsters (but
 *      allowing those at all causes more problems than worth the effort).
 */
{ LONG_SWORD, "Vorpal Blade",
        (SPFX_RESTR|SPFX_BEHEAD), 0, 0,
        {0,AD_PHYS,5,1},      {0,0,0,0},        {0,0,0,0},        0, A_NEUTRAL, NON_PM, NON_PM, 4000L },
/*
 *      Ah, never shall I forget the cry,
 *              or the shriek that shrieked he,
 *      As I gnashed my teeth, and from my sheath
 *              I drew my Snickersnee!
 *                      --Koko, Lord high executioner of Titipu
 *                        (From Sir W.S. Gilbert's "The Mikado")
 */
{ KATANA, "Snickersnee",
        SPFX_RESTR, 0, 0,
        {0,AD_PHYS,0,8},      {0,0,0,0},        {0,0,0,0},        0, A_LAWFUL, PM_SAMURAI, NON_PM, 1200L },

{ LONG_SWORD, "Sunsword",
        (SPFX_RESTR|SPFX_DFLAG2), 0, M2_UNDEAD,
        {0,AD_PHYS,5,0},      {0,AD_BLND,0,0},  {0,0,0,0},        0, A_LAWFUL, NON_PM, NON_PM, 1500L },

/*
 *      The artifacts for the quest dungeon, all self-willed.
 */

{ CRYSTAL_BALL, "The Orb of Detection",
        (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL), (SPFX_ESP|SPFX_HSPDAM), 0,
        {0,0,0,0},        {0,0,0,0},        {0,AD_MAGM,0,0},
        INVIS,          A_LAWFUL, PM_ARCHEOLOGIST, NON_PM, 2500L },

{ LUCKSTONE, "The Heart of Ahriman",
        (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL), SPFX_STLTH, 0,
        /* this stone does double damage if used as a projectile weapon */
        {0,AD_PHYS,5,0},      {0,0,0,0},        {0,0,0,0},
        LEVITATION,     A_NEUTRAL, PM_BARBARIAN, NON_PM, 2500L },

{ MACE, "The Sceptre of Might",
        (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_DALIGN), 0, 0,
        {0,AD_PHYS,5,0},      {0,0,0,0},        {0,AD_MAGM,0,0},
        CONFLICT,       A_LAWFUL, PM_CAVEMAN, NON_PM, 2500L },

{ QUARTERSTAFF, "The Staff of Aesculapius",
        (SPFX_NOGEN|SPFX_RESTR|SPFX_ATTK|SPFX_INTEL|SPFX_DRLI|SPFX_REGEN), 0,0,
        {0,AD_DRLI,0,0},      {0,AD_DRLI,0,0},      {0,0,0,0},
        HEALING,        A_NEUTRAL, PM_HEALER, NON_PM, 5000L },

{ MIRROR, "The Magic Mirror of Merlin",
        (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_SPEAK), SPFX_ESP, 0,
        {0,0,0,0},        {0,0,0,0},        {0,AD_MAGM,0,0},
        0,              A_LAWFUL, PM_KNIGHT, NON_PM, 1500L },

{ LENSES, "The Eyes of the Overworld",
        (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_XRAY), 0, 0,
        {0,0,0,0},        {0,0,0,0},        {0,AD_MAGM,0,0},
        ENLIGHTENING,   A_NEUTRAL,       PM_MONK, NON_PM, 2500L },

{ HELM_OF_BRILLIANCE, "The Mitre of Holiness",
        (SPFX_NOGEN|SPFX_RESTR|SPFX_DFLAG2|SPFX_INTEL), 0, M2_UNDEAD,
        {0,0,0,0},        {0,0,0,0},        {0,AD_FIRE,0,0},
        ENERGY_BOOST,   A_LAWFUL, PM_PRIEST, NON_PM, 2000L },

{ BOW, "The Longbow of Diana",
        (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_REFLECT), SPFX_ESP, 0,
        {0,AD_PHYS,5,0},      {0,0,0,0},        {0,0,0,0},
        CREATE_AMMO, A_CHAOTIC, PM_RANGER, NON_PM, 4000L },

{ SKELETON_KEY, "The Master Key of Thievery",
        (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_SPEAK),
                (SPFX_WARN|SPFX_TCTRL|SPFX_HPHDAM), 0,
                {0,0,0,0},        {0,0,0,0},        {0,0,0,0},
        UNTRAP,         A_CHAOTIC, PM_ROGUE, NON_PM, 3500L },

{ TSURUGI, "The Tsurugi of Muramasa",
        (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_BEHEAD|SPFX_LUCK), 0, 0,
        {0,AD_PHYS,0,8},      {0,0,0,0},        {0,0,0,0},
        0,              A_LAWFUL, PM_SAMURAI, NON_PM, 4500L },

{ CREDIT_CARD, "The Platinum Yendorian Express Card",
        (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_DEFN),
                (SPFX_ESP|SPFX_HSPDAM), 0,
                {0,0,0,0},        {0,0,0,0},        {0,AD_MAGM,0,0},
        CHARGE_OBJ,     A_NEUTRAL, PM_TOURIST, NON_PM, 7000L },

{ CRYSTAL_BALL, "The Orb of Fate",
        (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_LUCK),
                (SPFX_WARN|SPFX_HSPDAM|SPFX_HPHDAM), 0,
                {0,0,0,0},        {0,0,0,0},        {0,0,0,0},
        LEV_TELE,       A_NEUTRAL, PM_VALKYRIE, NON_PM, 3500L },

{ AMULET_OF_ESP, "The Eye of the Aethiopica",
        (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL), (SPFX_EREGEN|SPFX_HSPDAM), 0,
        {0,0,0,0},        {0,0,0,0},        {0,AD_MAGM,0,0},
        CREATE_PORTAL,  A_NEUTRAL, PM_WIZARD, NON_PM, 4000L },

/*
 *  terminator; otyp must be zero
 */
{ 0, NULL, 0, 0, 0, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, 0, A_NONE, NON_PM, NON_PM, 0L },

};
