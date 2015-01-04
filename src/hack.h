/* See LICENSE in the root of this project for change info */

#ifndef HACK_H
#define HACK_H

#include "coord.h"
#include "hacklib.h"
#include "o_init.h"
#include "rnd.h"
#include "cmd.h"

enum KillerMethod {
    // This first section is based on a bunch of #defines from original game
    KM_DIED,
    KM_CHOKING,
    KM_POISONING,
    KM_STARVING,
    KM_DROWNING,
    KM_BURNING,
    KM_DISSOLVED,
    KM_CRUSHING,
    KM_STONING,
    KM_TURNED_SLIME,
    KM_GENOCIDED,
    KM_PANICKED,
    KM_TRICKED,
    KM_QUIT,
    KM_ESCAPED,
    KM_ASCENDED,

    KM_ZAPPED_SELF_WITH_SPELL,
    KM_EXPLODING_RUNE,
    KM_CONTACT_POISONED_SPELLBOOK,
    KM_ELECTRIC_CHAIR,
    KM_CURSED_THRONE,
    KM_SITTING_ON_LAVA,
    KM_SITTING_IN_LAVA, // not to be confused with KM_SITTING_ON_LAVA...
    KM_SITTING_ON_IRON_SPIKE,
    KM_GAS_CLOUD,
    KM_IMPERIOUS_ORDER,
    KM_GENOCIDAL_CONFUSION,
    KM_SCROLL_OF_GENOCIDE,
    KM_SCROLL_OF_FIRE,
    KM_EXPLODING_RING,
    KM_RESIDUAL_UNDEAD_TURNING,
    KM_ALCHEMIC_BLAST,
    KM_ELEMENTARY_CHEMISTRY,
    KM_THROWN_POTION,
    KM_POTION_OF_ACID,
    KM_BURNING_POTION_OF_OIL,
    KM_COLLIDING_WITH_CEILING,
    KM_CONTAMINATED_POTION,
    KM_CONTAMINATED_TAP_WATER,
    KM_MILDLY_CONTAMINATED_POTION,
    KM_POTION_UNHOLY_WATER,
    KM_POTION_HOLY_WATER,
    KM_DELIBERATELY_MEETING_MEDUSA_GAZE,
    KM_WHILE_STUCK_IN_CREATURE_FORM,
    KM_SYSTEM_SHOCK,
    KM_UNSUCCESSFUL_POLYMORPH,
    KM_SELF_GENOCIDE,
    KM_MAGICAL_EXPLOSION,
    KM_CARNIVOROUS_BAG,
    KM_USING_MAGIC_HORN_ON_SELF,
    KM_FELL_INTO_CHASM,
    KM_SCROLL_OF_EARTH,
    KM_WAND,
    KM_PSYCHIC_BLAST,
    KM_TODO,
    KM_BRAINLESSNESS,
    KM_TOUCH_OF_DEATH,
    KM_FELL_ON_SINK,
    KM_SIP_BOILING_WATER,
    KM_CONTAMINATED_WATER,
    KM_UNREFRIGERATED_JUICE,
    KM_MONSTER,
    KM_EXHAUSTION,
    KM_CADAVER,
    KM_POISONOUS_CORPSE,
    KM_ACIDIC_CORPSE,
    KM_ATE_HORSEMAN,
    KM_TASTING_O_MEAT,
    KM_O_EGG,
    KM_ROTTEN_ROYAL_JELLY,
    KM_CHOKE_QUICK_SNACK,
    KM_CHOKE_ON_FOOD,
    KM_O,
    KM_FALLING_OBJECT,
    KM_ELEMENTARY_PHYSICS,
    KM_WEDGING_INTO_NARROW_CREVICE,
    KM_TOUCH_EDGE_UNIVERSE,
    KM_BUMP_INTO_BOULDER,
    KM_CRASH_INTO_IRON_BARS,
    KM_BUMP_INTO_TREE,
    KM_BUMP_INTO_WALL,
    KM_BUMP_INTO_DOOR,
    KM_O_CORPSE,
    KM_KICKING_O,
    KM_KICKING_DOOR,
    KM_KICKING_TREE,
    KM_KICKING_WALL,
    KM_KICKING_ROCK,
    KM_KICKING_THRONE,
    KM_KICKING_FOUNTAIN,
    KM_KICKING_HEADSTONE,
    KM_KICKING_SINK,
    KM_KICKING_ALTAR,
    KM_KICKING_DRAWBRIDGE,
    KM_KICKING_STAIRS,
    KM_KICKING_LADDER,
    KM_KICKING_IRON_BAR,
    KM_KICKING_SOMETHING_WEIRD,
    KM_KICKING_O_WITHOUT_BOOTS,
    KM_FALLING_DOWNSTAIRS,
    KM_SQUISHED_UNDER_BOULDER,
    KM_AXING_HARD_OBJECT,
    KM_YOUR_OWN_O,
    KM_EXPLODING_CRYSTAL_BALL,
    KM_FALLING_DRAWBRIDGE,
    KM_CLOSING_DRAWBRIDGE,
    KM_FELL_FROM_DRAWBRIDGE,
    KM_EXPLODING_DRAWBRIDGE,
    KM_COLLAPSING_DRAWBRIDGE,
    KM_RIDING_ACCIDENT,
    KM_IRON_BALL_COLLISON,
    KM_DRAGGED_DOWNSTAIRS_IRON_BALL,
    KM_LEG_DAMAGE_PULLED_BEAR_TRAP,
    KM_CRUNCHED_IN_HEAD_BY_IRON_BALL,
    KM_TOUCH_ARTIFACT,
    KM_KILLED_SELF_BREAK_WAND,
    KM_GRAPPLING_HOOK_SELF,
    KM_HIT_SELF_BULLWHIP,
    KM_JUMP_BEAR_TRAP,
    KM_EXPLOSION,
    KM_MOLTEN_LAVA,
    KM_EXPLODING_WAND, // "exploding wand"
    KM_SELF_WITH_WAND, // "zapped %sself with a wand", uhim()
    KM_SELF_WITH_DEATH_RAY, // "shot %sself with a death ray", uhim()
    KM_FALLING_ROCK, //  "falling rock"
};
struct Killer {
    enum KillerMethod method;
    const struct monst *monster;
    const struct obj *object;
    const struct artifact *art;
    const char *string;
    int integer;
};

struct Killer killed_by_const(enum KillerMethod method);
struct Killer killed_by_monster(enum KillerMethod method, const struct monst *);
struct Killer killed_by_object(enum KillerMethod method, const struct obj *);
struct Killer killed_by_string(enum KillerMethod method, const char *);
struct Killer killed_by_int(enum KillerMethod method, int);

// see flash_types
struct Killer killed_by_flash_text(const char * fltxt);
struct Killer killed_by_artifact(enum KillerMethod method, const struct artifact *art);

// const char * how = destroy_strings[dindx * 3 + 2];
// bool one = (cnt == 1L);
struct Killer killed_by_destroy_string(const char * how, bool one);

bool revive_nasty(int,int,const char*);
void movobj(struct obj *,signed char,signed char);
bool may_dig(signed char,signed char);
bool may_passwall(signed char,signed char);
bool bad_rock(struct permonst *,signed char,signed char);
bool invocation_pos(signed char,signed char);
bool test_move(int, int, int, int, int);
void domove(void);
void invocation_message(void);
void spoteffects(bool);
char *in_rooms(signed char,signed char,int);
bool in_town(int,int);
void check_special_room(bool);
int dopickup(void);
void lookaround(void);
int monster_nearby(void);
void nomul(int);
void unmul(const char *);
void losehp(int, struct Killer);
int weight_cap(void);
int inv_weight(void);
int near_capacity(void);
int calc_capacity(int);
int max_capacity(void);
bool check_capacity(const char *);
int inv_cnt(void);


#define TELL            1
#define NOTELL          0
#define ON              1
#define OFF             0
#define BOLT_LIM        8 /* from this distance ranged attacks will be made */
#define MAX_CARR_CAP    1000    /* so that boulders can be heavier */
#define DUMMY { 0 }

/* symbolic names for capacity levels */
#define UNENCUMBERED    0
#define SLT_ENCUMBER    1       /* Burdened */
#define MOD_ENCUMBER    2       /* Stressed */
#define HVY_ENCUMBER    3       /* Strained */
#define EXT_ENCUMBER    4       /* Overtaxed */
#define OVERLOADED      5       /* Overloaded */

/* Macros for how a rumor was delivered in outrumor() */
#define BY_ORACLE       0
#define BY_COOKIE       1
#define BY_PAPER        2
#define BY_OTHER        9

/* Macros for why you are no longer riding */
#define DISMOUNT_GENERIC        0
#define DISMOUNT_FELL           1
#define DISMOUNT_THROWN         2
#define DISMOUNT_POLY           3
#define DISMOUNT_ENGULFED       4
#define DISMOUNT_BONES          5
#define DISMOUNT_BYCHOICE       6

/* Special returns from mapglyph() */
#define MG_CORPSE       0x01
#define MG_INVIS        0x02
#define MG_DETECT       0x04
#define MG_PET          0x08
#define MG_RIDDEN       0x10

/* sellobj_state() states */
#define SELL_NORMAL     (0)
#define SELL_DELIBERATE (1)
#define SELL_DONTSELL   (2)

extern coord bhitpos;   /* place where throw or zap hits or stops */

/* types of calls to bhit() */
#define ZAPPED_WAND     0
#define THROWN_WEAPON   1
#define KICKED_WEAPON   2
#define FLASHED_LIGHT   3
#define INVIS_BEAM      4

#define NO_SPELL        0

/* flags to control makemon() */
#define NO_MM_FLAGS       0x00  /* use this rather than plain 0 */
#define NO_MINVENT        0x01  /* suppress minvent when creating mon */
#define MM_NOWAIT         0x02  /* don't set STRAT_WAITMASK flags */
#define MM_EDOG           0x04  /* add edog structure */
#define MM_EMIN           0x08  /* add emin structure */
#define MM_ANGRY          0x10  /* monster is created angry */
#define MM_NONAME         0x20  /* monster is not christened */
#define MM_NOCOUNTBIRTH   0x40  /* don't increment born counter (for revival) */
#define MM_IGNOREWATER    0x80  /* ignore water when positioning */
#define MM_ADJACENTOK     0x100 /* it is acceptable to use adjacent coordinates */

/* special mhpmax value when loading bones monster to flag as extinct or genocided */
#define DEFUNCT_MONSTER (-100)

/* flags for special ggetobj status returns */
#define ALL_FINISHED      0x01  /* called routine already finished the job */

/* flags to control query_objlist() */
#define BY_NEXTHERE       0x1   /* follow objlist by nexthere field */
#define AUTOSELECT_SINGLE 0x2   /* if only 1 object, don't ask */
#define USE_INVLET        0x4   /* use object's invlet */
#define INVORDER_SORT     0x8   /* sort objects by packorder */
#define SIGNAL_NOMENU     0x10  /* return -1 rather than 0 if none allowed */
#define FEEL_COCKATRICE   0x20  /* engage cockatrice checks and react */

/* Flags to control query_category() */
/* BY_NEXTHERE used by query_category() too, so skip 0x01 */
#define UNPAID_TYPES 0x02
#define GOLD_TYPES   0x04
#define WORN_TYPES   0x08
#define ALL_TYPES    0x10
#define BILLED_TYPES 0x20
#define CHOOSE_ALL   0x40
#define BUC_BLESSED  0x80
#define BUC_CURSED   0x100
#define BUC_UNCURSED 0x200
#define BUC_UNKNOWN  0x400
#define BUC_ALLBKNOWN (BUC_BLESSED|BUC_CURSED|BUC_UNCURSED)
#define ALL_TYPES_SELECTED -2

/* Flags to control find_mid() */
#define FM_FMON        0x01     /* search the fmon chain */
#define FM_MIGRATE     0x02     /* search the migrating monster chain */
#define FM_MYDOGS      0x04     /* search mydogs */
#define FM_EVERYWHERE  (FM_FMON | FM_MIGRATE | FM_MYDOGS)

/* Flags to control pick_[race,role,gend,align] routines in role.c */
#define PICK_RANDOM     0
#define PICK_RIGID      1

/* Flags to control dotrap() in trap.c */
#define NOWEBMSG        0x01    /* suppress stumble into web message */
#define FORCEBUNGLE     0x02    /* adjustments appropriate for bungling */
#define RECURSIVETRAP   0x04    /* trap changed into another type this same turn */

/* Flags to control test_move in hack.c */
#define DO_MOVE         0       /* really doing the move */
#define TEST_MOVE       1       /* test a normal move (move there next) */
#define TEST_TRAV       2       /* test a future travel location */

/*** some utility macros ***/
#define yn(query) yn_function(query,ynchars, 'n')
#define ynq(query) yn_function(query,ynqchars, 'q')
#define ynaq(query) yn_function(query,ynaqchars, 'y')
#define nyaq(query) yn_function(query,ynaqchars, 'n')
#define nyNaq(query) yn_function(query,ynNaqchars, 'n')
#define ynNaq(query) yn_function(query,ynNaqchars, 'y')

/* Macros for scatter */
#define VIS_EFFECTS     0x01    /* display visual effects */
#define MAY_HITMON      0x02    /* objects may hit monsters */
#define MAY_HITYOU      0x04    /* objects may hit you */
#define MAY_HIT         (MAY_HITMON|MAY_HITYOU)
#define MAY_DESTROY     0x08    /* objects may be destroyed at random */
#define MAY_FRACTURE    0x10    /* boulders & statues may fracture */

/* Macros for launching objects */
#define ROLL            0x01    /* the object is rolling */
#define FLING           0x02    /* the object is flying thru the air */
#define LAUNCH_UNSEEN   0x40    /* hero neither caused nor saw it */
#define LAUNCH_KNOWN    0x80    /* the hero caused this by explicit action */

/* Macros for explosion types */
enum {
    EXPL_DARK     = 0,
    EXPL_NOXIOUS  = 1,
    EXPL_MUDDY    = 2,
    EXPL_WET      = 3,
    EXPL_MAGICAL  = 4,
    EXPL_FIERY    = 5,
    EXPL_FROSTY   = 6,
    EXPL_MAX      = 7,
};

/* Macros for messages referring to hands, eyes, feet, etc... */
enum {
    ARM = 0,
    EYE = 1,
    FACE = 2,
    FINGER = 3,
    FINGERTIP = 4,
    FOOT = 5,
    HAND = 6,
    HANDED = 7,
    HEAD = 8,
    LEG = 9,
    LIGHT_HEADED = 10,
    NECK = 11,
    SPINE = 12,
    TOE = 13,
    HAIR = 14,
    BLOOD = 15,
    LUNG = 16,
    NOSE = 17,
    STOMACH = 18,
};

/* Flags to control menus */
#define MENUTYPELEN sizeof("traditional ")
#define MENU_TRADITIONAL 0
#define MENU_COMBINATION 1
#define MENU_PARTIAL     2
#define MENU_FULL        3

#define MENU_SELECTED   true
#define MENU_UNSELECTED false

/*
 * Option flags
 * Each higher number includes the characteristics of the numbers
 * below it.
 */
#define SET_IN_FILE     0 /* config file option only */
#define SET_VIA_PROG    1 /* may be set via extern program, not seen in game */
#define DISP_IN_GAME    2 /* may be set via extern program, displayed in game */
#define SET_IN_GAME     3 /* may be set via extern program or set in the game */

#define FEATURE_NOTICE_VER(major,minor,patch) (((unsigned long)major << 24) | \
        ((unsigned long)minor << 16) | \
        ((unsigned long)patch << 8) | \
        ((unsigned long)0))

#define FEATURE_NOTICE_VER_MAJ    (flags.suppress_alert >> 24)
#define FEATURE_NOTICE_VER_MIN    (((unsigned long)(0x0000000000FF0000L & flags.suppress_alert)) >> 16)
#define FEATURE_NOTICE_VER_PATCH  (((unsigned long)(0x000000000000FF00L & flags.suppress_alert)) >>  8)

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(x,y) ((x) < (y) ? (x) : (y))
#define plur(x) (((x) == 1) ? "" : "s")

#define ARM_BONUS(obj)  (objects[(obj)->otyp].oc_oc1 + (obj)->spe \
                         - min((int)greatest_erosion(obj),objects[(obj)->otyp].oc_oc1))

#define makeknown(x)    discover_object((x),true,true)
#define distu(xx,yy)    dist2((int)(xx),(int)(yy),(int)u.ux,(int)u.uy)
#define onlineu(xx,yy)  online2((int)(xx),(int)(yy),(int)u.ux,(int)u.uy)

#define rn1(x,y)        (rn2(x)+(y))

/* negative armor class is randomly weakened to prevent invulnerability */
#define AC_VALUE(AC)    ((AC) >= 0 ? (AC) : -rnd(-(AC)))

#endif /* HACK_H */
