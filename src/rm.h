/* See LICENSE in the root of this project for change info */
#ifndef RM_H
#define RM_H

#include "decl.h"
/*
 * The dungeon presentation graphics code and data structures were rewritten
 * and generalized for NetHack's release 2 by Eric S. Raymond (eric@snark)
 * building on Don G. Kneller's MS-DOS implementation.  See drawing.c for
 * the code that permits the user to set the contents of the symbol structure.
 *
 * The door representation was changed by Ari Huttunen(ahuttune@niksula.hut.fi)
 */

/*
 * TLCORNER     TDWALL          TRCORNER
 * +-           -+-             -+
 * |             |               |
 *
 * TRWALL       CROSSWALL       TLWALL          HWALL
 * |             |               |
 * +-           -+-             -+              ---
 * |             |               |
 *
 * BLCORNER     TUWALL          BRCORNER        VWALL
 * |             |               |              |
 * +-           -+-             -+              |
 */

/* Level location types */
enum {
    STONE           = 0,
    VWALL           = 1,
    HWALL           = 2,
    TLCORNER        = 3,
    TRCORNER        = 4,
    BLCORNER        = 5,
    BRCORNER        = 6,
    CROSSWALL       = 7,       /* For pretty mazes and special levels */
    TUWALL          = 8,
    TDWALL          = 9,
    TLWALL          = 10,
    TRWALL          = 11,
    DBWALL          = 12,
    TREE            = 13,      /* KMH */
    SDOOR           = 14,
    SCORR           = 15,
    POOL            = 16,
    MOAT            = 17,      /* pool that doesn't boil, adjust messages */
    WATER           = 18,
    DRAWBRIDGE_UP   = 19,
    LAVAPOOL        = 20,
    IRONBARS        = 21,      /* KMH */
    DOOR            = 22,
    CORR            = 23,
    ROOM            = 24,
    STAIRS          = 25,
    LADDER          = 26,
    FOUNTAIN        = 27,
    THRONE          = 28,
    SINK            = 29,
    GRAVE           = 30,
    ALTAR           = 31,
    ICE             = 32,
    DRAWBRIDGE_DOWN = 33,
    AIR             = 34,
    CLOUD           = 35,

    MAX_TYPE        = 36,
    INVALID_TYPE    = 127,
};

/*
 * The screen symbols may be the default or defined at game startup time.
 * See drawing.c for defaults.
 * Note: {ibm|dec}_graphics[] arrays (also in drawing.c) must be kept in synch.
 */

enum {
/* begin dungeon characters */
    S_stone         = 0,
    S_vwall         = 1,
    S_hwall         = 2,
    S_tlcorn        = 3,
    S_trcorn        = 4,
    S_blcorn        = 5,
    S_brcorn        = 6,
    S_crwall        = 7,
    S_tuwall        = 8,
    S_tdwall        = 9,
    S_tlwall        = 10,
    S_trwall        = 11,
    S_ndoor         = 12,
    S_vodoor        = 13,
    S_hodoor        = 14,
    S_vcdoor        = 15,      /* closed door, vertical wall */
    S_hcdoor        = 16,      /* closed door, horizontal wall */
    S_bars          = 17,      /* KMH -- iron bars */
    S_tree          = 18,      /* KMH */
    S_room          = 19,
    S_corr          = 20,
    S_litcorr       = 21,
    S_upstair       = 22,
    S_dnstair       = 23,
    S_upladder      = 24,
    S_dnladder      = 25,
    S_altar         = 26,
    S_grave         = 27,
    S_throne        = 28,
    S_sink          = 29,
    S_fountain      = 30,
    S_pool          = 31,
    S_ice           = 32,
    S_lava          = 33,
    S_vodbridge     = 34,
    S_hodbridge     = 35,
    S_vcdbridge     = 36,      /* closed drawbridge, vertical wall */
    S_hcdbridge     = 37,      /* closed drawbridge, horizontal wall */
    S_air           = 38,
    S_cloud         = 39,
    S_water         = 40,

/* end dungeon characters, begin traps */
    S_arrow_trap            = 41,
    S_dart_trap             = 42,
    S_falling_rock_trap     = 43,
    S_squeaky_board         = 44,
    S_bear_trap             = 45,
    S_land_mine             = 46,
    S_rolling_boulder_trap  = 47,
    S_sleeping_gas_trap     = 48,
    S_rust_trap             = 49,
    S_fire_trap             = 50,
    S_pit                   = 51,
    S_spiked_pit            = 52,
    S_hole                  = 53,
    S_trap_door             = 54,
    S_teleportation_trap    = 55,
    S_level_teleporter      = 56,
    S_magic_portal          = 57,
    S_web                   = 58,
    S_statue_trap           = 59,
    S_magic_trap            = 60,
    S_anti_magic_trap       = 61,
    S_polymorph_trap        = 62,

/* end traps, begin special effects */

    S_vbeam        = 63,      /* The 4 zap beam symbols.  Do NOT separate. */
    S_hbeam        = 64,      /* To change order or add, see function     */
    S_lslant       = 65,      /* zapdir_to_glyph() in display.c.          */
    S_rslant       = 66,
    S_digbeam      = 67,      /* dig beam symbol */
    S_flashbeam    = 68,      /* camera flash symbol */
    S_boomleft     = 69,      /* thrown boomerang, open left, e.g ')'    */
    S_boomright    = 70,      /* thrown boomerand, open right, e.g. '('  */
    S_ss1          = 71,      /* 4 magic shield glyphs */
    S_ss2          = 72,
    S_ss3          = 73,
    S_ss4          = 74,

/* The 8 swallow symbols.  Do NOT separate.  To change order or add, see */
/* the function swallow_to_glyph() in display.c.                         */
    S_sw_tl        = 75,      /* swallow top left [1]                 */
    S_sw_tc        = 76,      /* swallow top center [2]       Order:  */
    S_sw_tr        = 77,      /* swallow top right [3]                */
    S_sw_ml        = 78,      /* swallow middle left [4]      1 2 3   */
    S_sw_mr        = 79,      /* swallow middle right [6]     4 5 6   */
    S_sw_bl        = 80,      /* swallow bottom left [7]      7 8 9   */
    S_sw_bc        = 81,      /* swallow bottom center [8]            */
    S_sw_br        = 82,      /* swallow bottom right [9]             */

    S_explode1     = 83,      /* explosion top left                   */
    S_explode2     = 84,      /* explosion top center                 */
    S_explode3     = 85,      /* explosion top right           Ex.    */
    S_explode4     = 86,      /* explosion middle left                */
    S_explode5     = 87,      /* explosion middle center       /-\    */
    S_explode6     = 88,      /* explosion middle right        |@|    */
    S_explode7     = 89,      /* explosion bottom left         \-/    */
    S_explode8     = 90,      /* explosion bottom center              */
    S_explode9     = 91,      /* explosion bottom right               */

/* end effects */

    MAXPCHARS      = 92,      /* maximum number of mapped characters */
    MAXDCHARS      = 41,      /* maximum of mapped dungeon characters */
    MAXTCHARS      = 22,      /* maximum of mapped trap characters */
    MAXECHARS      = 29,      /* maximum of mapped effects characters */
    MAXEXPCHARS    = 9,       /* number of explosion characters */
};

struct symdef {
    unsigned char sym;
    const char  *explanation;
    unsigned char color;
};

extern const struct symdef defsyms[MAXPCHARS];  /* defaults */
extern unsigned char showsyms[MAXPCHARS];
extern const struct symdef def_warnsyms[WARNCOUNT];

/*
 * Graphics sets for display symbols
 */
enum {
    ASCII_GRAPHICS  = 0,       /* regular characters: '-', '+', &c */
    IBM_GRAPHICS    = 1,       /* PC graphic characters */
    DEC_GRAPHICS    = 2,       /* VT100 line drawing characters */
    MAC_GRAPHICS    = 3,       /* Macintosh drawing characters */
    CURS_GRAPHICS   = 4,   /* Portable curses drawing characters */
};

/*
 * The 5 possible states of doors
 */

enum {
    D_NODOOR      = 0,
    D_BROKEN      = 1,
    D_ISOPEN      = 2,
    D_CLOSED      = 4,
    D_LOCKED      = 8,
    D_TRAPPED     = 16,
};

/*
 * Some altars are considered as shrines, so we need a flag.
 */
#define AM_SHRINE       8

/*
 * Thrones should only be looted once.
 */
#define T_LOOTED        1

/*
 * Trees have more than one kick result.
 */
enum {
    TREE_LOOTED    = 1,
    TREE_SWARM     = 2,
};

/*
 * Fountains have limits, and special warnings.
 */
#define F_LOOTED        1
#define F_WARNED        2
#define FOUNTAIN_IS_WARNED(x,y)         (levl[x][y].flags & F_WARNED)
#define FOUNTAIN_IS_LOOTED(x,y)         (levl[x][y].flags & F_LOOTED)
#define SET_FOUNTAIN_WARNED(x,y)        levl[x][y].flags |= F_WARNED;
#define SET_FOUNTAIN_LOOTED(x,y)        levl[x][y].flags |= F_LOOTED;
#define CLEAR_FOUNTAIN_LOOTED(x,y)      levl[x][y].flags &= ~F_LOOTED;

/*
 * Doors are even worse :-) The special warning has a side effect
 * of instantly trapping the door, and if it was defined as trapped,
 * the guards consider that you have already been warned!
 */
#define D_WARNED        16

/*
 * Sinks have 3 different types of loot that shouldn't be abused
 */
enum {
    S_LPUDDING    = 1,
    S_LDWASHER    = 2,
    S_LRING       = 4,
};

/*
 * The four directions for a DrawBridge.
 */
enum {
    DB_NORTH      = 0,
    DB_SOUTH      = 1,
    DB_EAST       = 2,
    DB_WEST       = 3,
    DB_DIR        = 3,       /* mask for direction */
};

/*
 * What's under a drawbridge.
 */
enum {
    DB_MOAT       = 0,
    DB_LAVA       = 4,
    DB_ICE        = 8,
    DB_FLOOR      = 16,
    DB_UNDER      = 28,      /* mask for underneath */
};

/*
 * Wall information.
 */
#define WM_MASK         0x07    /* wall mode (bottom three bits) */
#define W_NONDIGGABLE   0x08
#define W_NONPASSWALL   0x10

/*
 * Ladders (in Vlad's tower) may be up or down.
 */
enum {
    LA_UP         = 1,
    LA_DOWN       = 2,
};

/*
 * Room areas may be iced pools
 */
enum {
    ICED_POOL     = 8,
    ICED_MOAT     = 16,
};

/*
 * The structure describing a coordinate position.
 * Before adding fields, remember that this will significantly affect
 * the size of temporary files and save files.
 */
struct rm {
    int glyph;             /* what the hero thinks is there */
    signed char typ;       /* what is really there */
    unsigned char seenv;   /* seen vector */
    unsigned flags:5;      /* extra information for typ */
    unsigned horizontal:1; /* wall/door/etc is horiz. (more typ info) */
    unsigned lit:1;        /* speed hack for lit rooms */
    unsigned waslit:1;     /* remember if a location was lit */
    unsigned roomno:6;     /* room # for special rooms */
    unsigned edge:1;       /* marks boundaries for special rooms*/
};

/*
 * Add wall angle viewing by defining "modes" for each wall type.  Each
 * mode describes which parts of a wall are finished (seen as as wall)
 * and which are unfinished (seen as rock).
 *
 * We use the bottom 3 bits of the flags field for the mode.  This comes
 * in conflict with secret doors, but we avoid problems because until
 * a secret door becomes discovered, we know what sdoor's bottom three
 * bits are.
 *
 * The following should cover all of the cases.
 *
 *      type    mode                            Examples: R=rock, F=finished
 *      -----   ----                            ----------------------------
 *      WALL:   0 none                          hwall, mode 1
 *              1 left/top (1/2 rock)                   RRR
 *              2 right/bottom (1/2 rock)               ---
 *                                                      FFF
 *
 *      CORNER: 0 none                          trcorn, mode 2
 *              1 outer (3/4 rock)                      FFF
 *              2 inner (1/4 rock)                      F+-
 *                                                      F|R
 *
 *      TWALL:  0 none                          tlwall, mode 3
 *              1 long edge (1/2 rock)                  F|F
 *              2 bottom left (on a tdwall)             -+F
 *              3 bottom right (on a tdwall)            R|F
 *
 *      CRWALL: 0 none                          crwall, mode 5
 *              1 top left (1/4 rock)                   R|F
 *              2 top right (1/4 rock)                  -+-
 *              3 bottom left (1/4 rock)                F|R
 *              4 bottom right (1/4 rock)
 *              5 top left & bottom right (1/2 rock)
 *              6 bottom left & top right (1/2 rock)
 */

enum {
    WM_W_LEFT = 1,                     /* vertical or horizontal wall */
    WM_W_RIGHT = 2,
    WM_W_TOP = WM_W_LEFT,
    WM_W_BOTTOM = WM_W_RIGHT,
    
    WM_C_OUTER = 1,                    /* corner wall */
    WM_C_INNER = 2,
    
    WM_T_LONG = 1,                     /* T wall */
    WM_T_BL   = 2,
    WM_T_BR   = 3,
    
    WM_X_TL   = 1,                     /* cross wall */
    WM_X_TR   = 2,
    WM_X_BL   = 3,
    WM_X_BR   = 4,
    WM_X_TLBR = 5,
    WM_X_BLTR = 6,
};

/*
 * Seen vector values.  The seen vector is an array of 8 bits, one for each
 * octant around a given center x:
 *
 *                      0 1 2
 *                      7 x 3
 *                      6 5 4
 *
 * In the case of walls, a single wall square can be viewed from 8 possible
 * directions.  If we know the type of wall and the directions from which
 * it has been seen, then we can determine what it looks like to the hero.
 */
enum {
    SV0 = 0x1,
    SV1 = 0x2,
    SV2 = 0x4,
    SV3 = 0x8,
    SV4 = 0x10,
    SV5 = 0x20,
    SV6 = 0x40,
    SV7 = 0x80,
    SVALL = 0xFF,
};

struct damage {
    struct damage *next;
    long when, cost;
    coord place;
    signed char typ;
};

struct levelflags {
    unsigned char   nfountains;             /* number of fountains on level */
    unsigned char   nsinks;                 /* number of sinks on the level */
    /* Several flags that give hints about what's on the level */
    unsigned has_shop: 1;
    unsigned has_vault: 1;
    unsigned has_zoo: 1;
    unsigned has_court: 1;
    unsigned has_morgue: 1;
    unsigned has_beehive: 1;
    unsigned has_barracks: 1;
    unsigned has_temple: 1;

    unsigned has_swamp: 1;
    unsigned noteleport:1;
    unsigned hardfloor:1;
    unsigned nommap:1;
    unsigned hero_memory:1;        /* true for everything but the water level */
    unsigned shortsighted:1;       /* monsters are shortsighted */
    unsigned graveyard:1;          /* has_morgue, but remains set */
    unsigned is_maze_lev:1;

    unsigned is_cavernous_lev:1;
    unsigned arboreal: 1;          /* Trees replace rock */
};

typedef struct
{
    struct rm           locations[COLNO][ROWNO];
    struct obj          *objects[COLNO][ROWNO];
    struct monst        *monsters[COLNO][ROWNO];
    struct obj          *objlist;
    struct obj          *buriedobjlist;
    struct monst        *monlist;
    struct damage       *damagelist;
    struct levelflags   flags;
}
dlevel_t;

extern dlevel_t level;  /* structure describing the current level */

/*
 * Macros for compatibility with old code. Someday these will go away.
 */
#define levl            level.locations
#define fobj            level.objlist
#define fmon            level.monlist

/*
 * Covert a trap number into the defsym graphics array.
 * Convert a defsym number into a trap number.
 * Assumes that arrow trap will always be the first trap.
 */
#define trap_to_defsym(t) (S_arrow_trap+(t)-1)
#define defsym_to_trap(d) ((d)-S_arrow_trap+1)

#define OBJ_AT(x,y)     (level.objects[x][y] != (struct obj *)0)
/*
 * Macros for encapsulation of level.monsters references.
 */
#define MON_AT(x,y)     (level.monsters[x][y] != (struct monst *)0 && \
                         !(level.monsters[x][y])->mburied)
#define MON_BURIED_AT(x,y)      (level.monsters[x][y] != (struct monst *)0 && \
                                (level.monsters[x][y])->mburied)
#define place_worm_seg(m,x,y)   level.monsters[x][y] = m
#define remove_monster(x,y)     level.monsters[x][y] = (struct monst *)0
#define m_at(x,y)               (MON_AT(x,y) ? level.monsters[x][y] : \
                                                (struct monst *)0)
#define m_buried_at(x,y)        (MON_BURIED_AT(x,y) ? level.monsters[x][y] : \
                                                       (struct monst *)0)

#endif /* RM_H */
