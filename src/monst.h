/* See LICENSE in the root of this project for change info */

#ifndef MONST_H
#define MONST_H

#include <stddef.h>

#include "align.h"
#include "coord.h"
#include "util.h"

/* The weapon_check flag is used two ways:
 * 1) When calling mon_wield_item, is 2-6 depending on what is desired.
 * 2) Between calls to mon_wield_item, is 0 or 1 depending on whether or not
 *    the weapon is known by the monster to be cursed (so it shouldn't bother
 *    trying for another weapon).
 * I originally planned to also use 0 if the monster already had its best
 * weapon, to avoid the overhead of a call to mon_wield_item, but it turns out
 * that there are enough situations which might make a monster change its
 * weapon that this is impractical.  --KAA
 */
enum {
    NO_WEAPON_WANTED = 0,
    NEED_WEAPON = 1,
    NEED_RANGED_WEAPON = 2,
    NEED_HTH_WEAPON = 3,
    NEED_PICK_AXE = 4,
    NEED_AXE = 5,
    NEED_PICK_OR_AXE = 6,
};

/* The following flags are used for the second argument to display_minventory
 * in invent.c:
 *
 * MINV_NOLET  If set, don't display inventory letters on monster's inventory.
 * MINV_ALL    If set, display all items in monster's inventory, otherwise
 *             just display wielded weapons and worn items.
 */
#define MINV_NOLET 0x01
#define MINV_ALL   0x02

enum {
    CHAM_ORDINARY          = 0,       /* not a shapechanger */
    CHAM_CHAMELEON         = 1,       /* animal */
    CHAM_DOPPELGANGER      = 2,       /* demi-human */
    CHAM_SANDESTIN         = 3,       /* demon */
    CHAM_MAX_INDX          = CHAM_SANDESTIN,
};

enum {
    M_AP_NOTHING    = 0,       /* mappearance is unused -- monster appears
                                   as itself */
    M_AP_FURNITURE  = 1,       /* stairs, a door, an altar, etc. */
    M_AP_OBJECT     = 2,       /* an object */
    M_AP_MONSTER    = 3,       /* a monster */
};

#define MTSZ    4

#define MAX_NUM_WORMS   32      /* should be 2^(wormno bitfield size) */

#define STRAT_ARRIVE    0x40000000L     /* just arrived on current level */
#define STRAT_WAITFORU  0x20000000L
#define STRAT_CLOSE     0x10000000L
#define STRAT_WAITMASK  0x30000000L
#define STRAT_HEAL      0x08000000L
#define STRAT_GROUND    0x04000000L
#define STRAT_MONSTR    0x02000000L
#define STRAT_PLAYER    0x01000000L
#define STRAT_NONE      0x00000000L
#define STRAT_STRATMASK 0x0f000000L
#define STRAT_XMASK     0x00ff0000L
#define STRAT_YMASK     0x0000ff00L
#define STRAT_GOAL      0x000000ffL
#define STRAT_GOALX(s)  ((signed char)((s & STRAT_XMASK) >> 16))
#define STRAT_GOALY(s)  ((signed char)((s & STRAT_YMASK) >> 8))

struct monst {
    struct monst *nmon;
    struct permonst *data;
    unsigned m_id;
    short mnum;             /* permanent monster index number */
    short movement;         /* movement points (derived from permonst definition and added effects */
    unsigned char m_lev;            /* adjusted difficulty level of monster */
    aligntyp malign;        /* alignment of this monster, relative to the
                               player (positive = good to kill) */
    signed char mx, my;
    signed char mux, muy;           /* where the monster thinks you are */
    coord mtrack[MTSZ];     /* monster track */
    int mhp, mhpmax;
    unsigned mappearance;   /* for undetected mimics and the wiz */
    unsigned char    m_ap_type;     /* what mappearance is describing: */

    signed char mtame;              /* level of tameness, implies peaceful */
    unsigned short mintrinsics;     /* low 8 correspond to mresists */
    int mspec_used;         /* monster's special ability attack timeout */

    unsigned female:1;     /* is female */
    unsigned minvis:1;     /* currently invisible */
    unsigned invis_blkd:1; /* invisibility blocked */
    unsigned perminvis:1;  /* intrinsic minvis value */
    unsigned cham:3;       /* shape-changer */
    /* note: lychanthropes are handled elsewhere */
    unsigned mundetected:1;        /* not seen in present hiding place */
    /* implies one of M1_CONCEAL or M1_HIDE,
     * but not mimic (that is, snake, spider,
     * trapper, piercer, eel)
     */

    unsigned mcan:1;       /* has been cancelled */
    unsigned mburied:1;    /* has been buried */
    unsigned mspeed:2;     /* current speed */
    unsigned permspeed:2;  /* intrinsic mspeed value */
    unsigned mrevived:1;   /* has been revived from the dead */
    unsigned mavenge:1;    /* did something to deserve retaliation */

    unsigned mflee:1;      /* fleeing */
    unsigned mfleetim:7;   /* timeout for mflee */

    unsigned mcansee:1;    /* cansee 1, temp.blinded 0, blind 0 */
    unsigned mblinded:7;   /* cansee 0, temp.blinded n, blind 0 */

    unsigned mcanmove:1;   /* paralysis, similar to mblinded */
    unsigned mfrozen:7;

    unsigned msleeping:1;  /* asleep until woken */
    unsigned mstun:1;      /* stunned (off balance) */
    unsigned mconf:1;      /* confused */
    unsigned mpeaceful:1;  /* does not attack unprovoked */
    unsigned mtrapped:1;   /* trapped in a pit, web or bear trap */
    unsigned mleashed:1;   /* monster is on a leash */
    unsigned isshk:1;      /* is shopkeeper */
    unsigned isminion:1;   /* is a minion */

    unsigned isgd:1;       /* is guard */
    unsigned ispriest:1;   /* is a priest */
    unsigned iswiz:1;      /* is the Wizard of Yendor */
    unsigned wormno:5;     /* at most 31 worms on any level */

    long mstrategy;         /* for monsters with mflag3: current strategy */

    long mtrapseen;         /* bitmap of traps we've been trapped in */
    long mlstmv;            /* for catching up with lost time */
    long mgold;
    struct obj *minvent;

    struct obj *mw;
    long misc_worn_check;
    signed char weapon_check;

    unsigned char mnamelth;         /* length of name (following mxlth) */
    short mxlth;            /* length of following data */
    /* in order to prevent alignment problems mextra should
       be (or follow) a long int */
    int meating;            /* monster is eating timeout */
    long mextra[1]; /* monster dependent info */
};

/*
 * Note that mextra[] may correspond to any of a number of structures, which
 * are indicated by some of the other fields.
 *      isgd     ->     struct egd
 *      ispriest ->     struct epri
 *      isshk    ->     struct eshk
 *      isminion ->     struct emin
 *                      (struct epri for roaming priests and angels, which is
 *                       compatible with emin for polymorph purposes)
 *      mtame    ->     struct edog
 *                      (struct epri for guardian angels, which do not eat
 *                       or do other doggy things)
 * Since at most one structure can be indicated in this manner, it is not
 * possible to tame any creatures using the other structures (the only
 * exception being the guardian angels which are tame on creation).
 */

#define newmonst(xl) (struct monst *)malloc((unsigned)(xl) + sizeof(struct monst))
#define dealloc_monst(mon) free((void *)(mon))

/* these are in mspeed */
#define MSLOW 1         /* slow monster */
#define MFAST 2         /* speeded monster */

static const char *monster_name(const struct monst *m) {
    return ((char *)m->mextra) + m->mxlth;
}

static size_t set_monster_name(struct monst *m, const char *value) {
    char *str_ptr = ((char *)m->mextra) + m->mxlth;
    return nh_strlcpy(str_ptr, value, m->mnamelth);
}

#define MON_WEP(mon)    ((mon)->mw)
#define MON_NOWEP(mon)  ((mon)->mw = (struct obj *)0)

#define DEADMONSTER(mon)        ((mon)->mhp < 1)

#endif /* MONST_H */
