/* See LICENSE in the root of this project for change info */
#ifndef SP_LEV_H
#define SP_LEV_H

#include "global.h"
#include "align.h"

    /* wall directions */
enum {
    W_NORTH       = 1,
    W_SOUTH       = 2,
    W_EAST        = 4,
    W_WEST        = 8,
    W_ANY         = (W_NORTH|W_SOUTH|W_EAST|W_WEST),
};

    /* MAP limits */
enum {
    MAP_X_LIM     = 76,
    MAP_Y_LIM     = 21,
};

    /* Per level flags */
enum {
    NOTELEPORT    = 1,
    HARDFLOOR     = 2,
    NOMMAP        = 4,
    SHORTSIGHTED  = 8,
    ARBOREAL      = 16,
};

    /* special level types */
enum {
    SP_LEV_ROOMS  = 1,
    SP_LEV_MAZE   = 2,
};

/*
 * Structures manipulated by the special levels loader & compiler
 */

typedef union str_or_len {
        char *str;
        int   len;
} Str_or_Len;

typedef struct {
        bool init_present, padding;
        char    fg, bg;
        bool smoothed, joined;
        signed char     lit, walled;
} lev_init;

typedef struct {
        signed char x, y, mask;
} door;

typedef struct {
        signed char wall, pos, secret, mask;
} room_door;

typedef struct {
        signed char x, y, chance, type;
} trap;

typedef struct {
        Str_or_Len name, appear_as;
        short id;
        aligntyp align;
        signed char x, y, chance, class, appear;
        signed char peaceful, asleep;
} monster;

typedef struct {
        Str_or_Len name;
        int   corpsenm;
        short id, spe;
        signed char x, y, chance, class, containment;
        signed char curse_state;
} object;

typedef struct {
        signed char             x, y;
        aligntyp        align;
        signed char             shrine;
} altar;

typedef struct {
        signed char x, y, dir, db_open;
} drawbridge;

typedef struct {
        signed char x, y, dir;
} walk;

typedef struct {
        signed char x1, y1, x2, y2;
} digpos;

typedef struct {
        signed char x, y, up;
} lad;

typedef struct {
        signed char x, y, up;
} stair;

typedef struct {
        signed char x1, y1, x2, y2;
        signed char rtype, rlit, rirreg;
} region;

/* values for rtype are defined in dungeon.h */
typedef struct {
        struct { signed char x1, y1, x2, y2; } inarea;
        struct { signed char x1, y1, x2, y2; } delarea;
        bool in_islev, del_islev;
        signed char rtype, padding;
        Str_or_Len rname;
} lev_region;

typedef struct {
        signed char x, y;
        int   amount;
} gold;

typedef struct {
        signed char x, y;
        Str_or_Len engr;
        signed char etype;
} engraving;

typedef struct {
        signed char x, y;
} fountain;

typedef struct {
        signed char x, y;
} sink;

typedef struct {
        signed char x, y;
} pool;

typedef struct {
        char halign, valign;
        char xsize, ysize;
        char **map;
        char nrobjects;
        char *robjects;
        char nloc;
        char *rloc_x;
        char *rloc_y;
        char nrmonst;
        char *rmonst;
        char nreg;
        region **regions;
        char nlreg;
        lev_region **lregions;
        char ndoor;
        door **doors;
        char ntrap;
        trap **traps;
        char nmonster;
        monster **monsters;
        char nobject;
        object **objects;
        char ndrawbridge;
        drawbridge **drawbridges;
        char nwalk;
        walk **walks;
        char ndig;
        digpos **digs;
        char npass;
        digpos **passs;
        char nlad;
        lad **lads;
        char nstair;
        stair **stairs;
        char naltar;
        altar **altars;
        char ngold;
        gold **golds;
        char nengraving;
        engraving **engravings;
        char nfountain;
        fountain **fountains;
} mazepart;

typedef struct {
        long flags;
        lev_init init_lev;
        signed char filling;
        char numpart;
        mazepart **parts;
} specialmaze;

typedef struct _room {
        char  *name;
        char  *parent;
        signed char x, y, w, h;
        signed char xalign, yalign;
        signed char rtype, chance, rlit, filled;
        char ndoor;
        room_door **doors;
        char ntrap;
        trap **traps;
        char nmonster;
        monster **monsters;
        char nobject;
        object **objects;
        char naltar;
        altar **altars;
        char nstair;
        stair **stairs;
        char ngold;
        gold **golds;
        char nengraving;
        engraving **engravings;
        char nfountain;
        fountain **fountains;
        char nsink;
        sink **sinks;
        char npool;
        pool **pools;
        /* These three fields are only used when loading the level... */
        int nsubroom;
        struct _room *subrooms[MAX_SUBROOMS];
        struct mkroom *mkr;
} room;

typedef struct {
        struct {
                signed char room;
                signed char wall;
                signed char door;
        } src, dest;
} corridor;

/* used only by lev_comp */
typedef struct {
        long flags;
        lev_init init_lev;
        char nrobjects;
        char *robjects;
        char nrmonst;
        char *rmonst;
        signed char nroom;
        room **rooms;
        signed char ncorr;
        corridor **corrs;
} splev;

#endif /* SP_LEV_H */
