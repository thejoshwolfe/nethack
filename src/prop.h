/* See LICENSE in the root of this project for change info */

#ifndef PROP_H
#define PROP_H

/*** What the properties are ***/
enum {
    FIRE_RES              =  1,
    COLD_RES              =  2,
    SLEEP_RES             =  3,
    DISINT_RES            =  4,
    SHOCK_RES             =  5,
    POISON_RES            =  6,
    ACID_RES              =  7,
    STONE_RES             =  8,
    // note: for the first eight properties, MR_xxx == (1 << (xxx_RES - 1)) */
    ADORNED               =  9,
    REGENERATION          = 10,
    SEARCHING             = 11,
    SEE_INVIS             = 12,
    INVIS                 = 13,
    TELEPORT              = 14,
    TELEPORT_CONTROL      = 15,
    POLYMORPH             = 16,
    POLYMORPH_CONTROL     = 17,
    LEVITATION            = 18,
    STEALTH               = 19,
    AGGRAVATE_MONSTER     = 20,
    CONFLICT              = 21,
    PROTECTION            = 22,
    PROT_FROM_SHAPE_CHANGERS = 23,
    WARNING               = 24,
    TELEPAT               = 25,
    FAST                  = 26,
    STUNNED               = 27,
    CONFUSION             = 28,
    SICK                  = 29,
    BLINDED               = 30,
    SLEEPING              = 31,
    WOUNDED_LEGS          = 32,
    STONED                = 33,
    STRANGLED             = 34,
    HALLUC                = 35,
    HALLUC_RES            = 36,
    FUMBLING              = 37,
    JUMPING               = 38,
    WWALKING              = 39,
    HUNGER                = 40,
    GLIB                  = 41,
    REFLECTING            = 42,
    LIFESAVED             = 43,
    ANTIMAGIC             = 44,
    DISPLACED             = 45,
    CLAIRVOYANT           = 46,
    VOMITING              = 47,
    ENERGY_REGENERATION   = 48,
    MAGICAL_BREATHING     = 49,
    HALF_SPDAM            = 50,
    HALF_PHDAM            = 51,
    SICK_RES              = 52,
    DRAIN_RES             = 53,
    WARN_UNDEAD           = 54,
    INVULNERABLE          = 55,
    FREE_ACTION           = 56,
    SWIMMING              = 57,
    SLIMED                = 58,
    FIXED_ABIL            = 59,
    FLYING                = 60,
    UNCHANGING            = 61,
    PASSES_WALLS          = 62,
    SLOW_DIGESTION        = 63,
    INFRAVISION           = 64,
    WARN_OF_MON           = 65,
    DETECT_MONSTERS       = 66,
    LAST_PROP             = DETECT_MONSTERS,
};

/*** Where the properties come from ***/
/* Definitions were moved here from obj.h and you.h */
struct prop {
        /*** Properties conveyed by objects ***/
        long extrinsic;
        /* Armor */
#define W_ARM        0x00000001L /* Body armor */
#define W_ARMC       0x00000002L /* Cloak */
#define W_ARMH       0x00000004L /* Helmet/hat */
#define W_ARMS       0x00000008L /* Shield */
#define W_ARMG       0x00000010L /* Gloves/gauntlets */
#define W_ARMF       0x00000020L /* Footwear */
#define W_ARMU       0x00000040L /* Undershirt */
#define W_ARMOR       (W_ARM | W_ARMC | W_ARMH | W_ARMS | W_ARMG | W_ARMF | W_ARMU)
        /* Weapons and artifacts */
#define W_WEP        0x00000100L /* Wielded weapon */
#define W_QUIVER     0x00000200L /* Quiver for (f)iring ammo */
#define W_SWAPWEP    0x00000400L /* Secondary weapon */
#define W_ART        0x00001000L /* Carrying artifact (not really worn) */
#define W_ARTI       0x00002000L /* Invoked artifact  (not really worn) */
        /* Amulets, rings, tools, and other items */
#define W_AMUL       0x00010000L /* Amulet */
#define W_RINGL      0x00020000L /* Left ring */
#define W_RINGR      0x00040000L /* Right ring */
#define W_RING       (W_RINGL | W_RINGR)
#define W_TOOL       0x00080000L /* Eyewear */
#define W_SADDLE     0x00100000L /* KMH -- For riding monsters */
#define W_BALL       0x00200000L /* Punishment ball */
#define W_CHAIN      0x00400000L /* Punishment chain */

        /*** Property is blocked by an object ***/
        long blocked;                                   /* Same assignments as extrinsic */

        /*** Timeouts, permanent properties, and other flags ***/
        long intrinsic;
        /* Timed properties */
#define TIMEOUT      0x00ffffffL /* Up to 16 million turns */
        /* Permanent properties */
#define FROMEXPER    0x01000000L /* Gain/lose with experience, for role */
#define FROMRACE     0x02000000L /* Gain/lose with experience, for race */
#define FROMOUTSIDE  0x04000000L /* By corpses, prayer, thrones, etc. */
#define INTRINSIC    (FROMOUTSIDE|FROMRACE|FROMEXPER)
        /* Control flags */
#define I_SPECIAL    0x10000000L /* Property is controllable */
};

/*** Definitions for backwards compatibility ***/
#define LEFT_RING       W_RINGL
#define RIGHT_RING      W_RINGR
#define LEFT_SIDE       LEFT_RING
#define RIGHT_SIDE      RIGHT_RING
#define BOTH_SIDES      (LEFT_SIDE | RIGHT_SIDE)
#define WORN_ARMOR      W_ARM
#define WORN_CLOAK      W_ARMC
#define WORN_HELMET     W_ARMH
#define WORN_SHIELD     W_ARMS
#define WORN_GLOVES     W_ARMG
#define WORN_BOOTS      W_ARMF
#define WORN_AMUL       W_AMUL
#define WORN_BLINDF     W_TOOL
#define WORN_SHIRT      W_ARMU

#endif /* PROP_H */
