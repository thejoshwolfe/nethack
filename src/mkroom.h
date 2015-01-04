/* See LICENSE in the root of this project for change info */
#ifndef MKROOM_H
#define MKROOM_H

#include "global.h"
#include "coord.h"

/* mkroom.h - types and structures for room and shop initialization */

struct mkroom {
        signed char lx,hx,ly,hy;        /* usually signed char, but hx may be -1 */
        signed char rtype;              /* type of room (zoo, throne, etc...) */
        signed char rlit;               /* is the room lit ? */
        signed char doorct;             /* door count */
        signed char fdoor;              /* index for the first door of the room */
        signed char nsubrooms;  /* number of subrooms */
        bool irregular;      /* true if room is non-rectangular */
        struct mkroom *sbrooms[MAX_SUBROOMS];  /* Subrooms pointers */
        struct monst *resident; /* priest/shopkeeper/guard for this room */
};

enum {
    D_SCATTER, /* normal placement */
    D_SHOP, /* shop-like placement */
    D_TEMPLE, /* temple-like placement */
};

struct shclass {
        const char *name;       /* name of the shop type */
        char    symb;           /* this identifies the shop type */
        int     prob;           /* the shop type probability in % */
        signed char     shdist;         /* object placement type */
        struct itp {
            int iprob;          /* probability of an item type */
            int itype;  /* item type: if >=0 a class, if < 0 a specific item */
        } iprobs[5];
        const char * const *shknms;     /* list of shopkeeper names for this type */
};

extern struct mkroom rooms[(MAXNROFROOMS+1)*2];
extern struct mkroom* subrooms;
/* the normal rooms on the current level are described in rooms[0..n] for
 * some n<MAXNROFROOMS
 * the vault, if any, is described by rooms[n+1]
 * the next rooms entry has hx -1 as a flag
 * there is at most one non-vault special room on a level
 */

extern struct mkroom *dnstairs_room, *upstairs_room, *sstairs_room;

extern coord doors[DOORMAX];

/* values for rtype in the room definition structure */
enum {
    OROOM, /* ordinary room */

    COURT = 2, /* contains a throne */
    SWAMP, /* contains pools */
    VAULT, /* contains piles of gold */
    BEEHIVE, /* contains killer bees and royal jelly */
    MORGUE, /* contains corpses, undead and ghosts */
    BARRACKS, /* contains soldiers and their gear */
    ZOO, /* floor covered with treasure and monsters */
    DELPHI, /* contains Oracle and peripherals */
    TEMPLE, /* contains a shrine */
    LEPREHALL, /* leprechaun hall (Tom Proudfoot) */
    COCKNEST, /* cockatrice nest (Tom Proudfoot) */
    ANTHOLE, /* ants (Tom Proudfoot) */
    SHOPBASE, /* everything above this is a shop */
    ARMORSHOP, /* specific shop defines for level compiler */
    SCROLLSHOP,
    POTIONSHOP,
    WEAPONSHOP,
    FOODSHOP,
    RINGSHOP,
    WANDSHOP,
    TOOLSHOP,
    BOOKSHOP,

    UNIQUESHOP = 24, /* shops here & above not randomly gen'd. */
    CANDLESHOP = 24,
    MAXRTYPE   = 24, /* maximum valid room type */
};

/* Special type for search_special() */
enum {
    ANY_SHOP = -2,
    ANY_TYPE,
};

enum {
    NO_ROOM, /* indicates lack of room-occupancy */
    SHARED, /* indicates normal shared boundary */
    SHARED_PLUS, /* indicates shared boundary - extra adjacent-
                  * square searching required */
};

static const int ROOMOFFSET = 3; /*
                                 * (levl[x][y].roomno - ROOMOFFSET) gives
                                 * rooms[] index, for inside-squares and
                                 * non-shared boundaries.
                                 */

#define IS_ROOM_PTR(x)          ((x) >= rooms && (x) < rooms + MAXNROFROOMS)
#define IS_ROOM_INDEX(x)        ((x) >= 0 && (x) < MAXNROFROOMS)
#define IS_SUBROOM_PTR(x)       ((x) >= subrooms && \
                                 (x) < subrooms + MAXNROFROOMS)
#define IS_SUBROOM_INDEX(x)     ((x) > MAXNROFROOMS && (x) < (MAXNROFROOMS*2))
#define ROOM_INDEX(x)           ((x) - rooms)
#define SUBROOM_INDEX(x)        ((x) - subrooms)
#define IS_LAST_ROOM_PTR(x)     (ROOM_INDEX(x) == nroom)
#define IS_LAST_SUBROOM_PTR(x)  (!nsubroom || SUBROOM_INDEX(x) == nsubroom)

#endif /* MKROOM_H */
