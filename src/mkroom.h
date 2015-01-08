/* See LICENSE in the root of this project for change info */
#ifndef MKROOM_H
#define MKROOM_H

#include <stdbool.h>

#include "coord.h"
#include "global.h"

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

struct shclass {
        const char *name;       /* name of the shop type */
        char    symb;           /* this identifies the shop type */
        int     prob;           /* the shop type probability in % */
        signed char     shdist;         /* object placement type */
#define D_SCATTER       0       /* normal placement */
#define D_SHOP          1       /* shop-like placement */
#define D_TEMPLE        2       /* temple-like placement */
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
#define OROOM            0      /* ordinary room */
#define COURT            2      /* contains a throne */
#define SWAMP            3      /* contains pools */
#define VAULT            4      /* contains piles of gold */
#define BEEHIVE          5      /* contains killer bees and royal jelly */
#define MORGUE           6      /* contains corpses, undead and ghosts */
#define BARRACKS         7      /* contains soldiers and their gear */
#define ZOO              8      /* floor covered with treasure and monsters */
#define DELPHI           9      /* contains Oracle and peripherals */
#define TEMPLE          10      /* contains a shrine */
#define LEPREHALL       11      /* leprechaun hall (Tom Proudfoot) */
#define COCKNEST        12      /* cockatrice nest (Tom Proudfoot) */
#define ANTHOLE         13      /* ants (Tom Proudfoot) */
#define SHOPBASE        14      /* everything above this is a shop */
#define ARMORSHOP       15      /* specific shop defines for level compiler */
#define SCROLLSHOP      16
#define POTIONSHOP      17
#define WEAPONSHOP      18
#define FOODSHOP        19
#define RINGSHOP        20
#define WANDSHOP        21
#define TOOLSHOP        22
#define BOOKSHOP        23
#define UNIQUESHOP      24      /* shops here & above not randomly gen'd. */
#define CANDLESHOP      24
#define MAXRTYPE        24      /* maximum valid room type */

/* Special type for search_special() */
#define ANY_TYPE        (-1)
#define ANY_SHOP        (-2)

#define NO_ROOM         0       /* indicates lack of room-occupancy */
#define SHARED          1       /* indicates normal shared boundary */
#define SHARED_PLUS     2       /* indicates shared boundary - extra adjacent-
                                 * square searching required */

#define ROOMOFFSET      3       /*
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

void mkroom(int);
void fill_zoo(struct mkroom *);
bool nexttodoor(int,int);
bool has_dnstairs(struct mkroom *);
bool has_upstairs(struct mkroom *);
int somex(struct mkroom *);
int somey(struct mkroom *);
bool inside_room(struct mkroom *,signed char,signed char);
bool somexy(struct mkroom *,coord *);
void mkundead(coord *,bool,int);
struct permonst *courtmon(void);
void save_rooms(int);
void rest_rooms(int);
struct mkroom *search_special(signed char);


#endif /* MKROOM_H */
