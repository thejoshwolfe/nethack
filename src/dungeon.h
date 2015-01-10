/* See LICENSE in the root of this project for change info */
#ifndef DUNGEON_H
#define DUNGEON_H

#include <stdbool.h>

#include "global.h"
#include "permonst.h"

typedef struct d_flags {        /* dungeon/level type flags */
        unsigned town: 1;      /* is this a town? (levels only) */
        unsigned hellish: 1;   /* is this part of hell? */
        unsigned maze_like: 1; /* is this a maze? */
        unsigned rogue_like: 1; /* is this an old-fashioned presentation? */
        unsigned align: 3;     /* dungeon alignment. */
        unsigned unused: 1;    /* etc... */
} d_flags;

typedef struct d_level {        /* basic dungeon level element */
        signed char     dnum;           /* dungeon number */
        signed char     dlevel;         /* level number */
} d_level;

typedef struct s_level {        /* special dungeon level element */
        struct  s_level *next;
        d_level dlevel;         /* dungeon & level numbers */
        char    proto[15];      /* name of prototype file (eg. "tower") */
        char    boneid;         /* character to id level in bones files */
        unsigned char   rndlevs;        /* no. of randomly available similar levels */
        d_flags flags;          /* type flags */
} s_level;

typedef struct stairway {       /* basic stairway identifier */
        signed char     sx, sy;         /* x / y location of the stair */
        d_level tolev;          /* where does it go */
        char    up;             /* what type of stairway (up/down) */
} stairway;

/* level region types */
enum {
    LR_DOWNSTAIR = 0,
    LR_UPSTAIR = 1,
    LR_PORTAL = 2,
    LR_BRANCH = 3,
    LR_TELE = 4,
    LR_UPTELE = 5,
    LR_DOWNTELE = 6,
};

typedef struct dest_area {      /* non-stairway level change indentifier */
        signed char     lx, ly;         /* "lower" left corner (near [0,0]) */
        signed char     hx, hy;         /* "upper" right corner (near [COLNO,ROWNO]) */
        signed char     nlx, nly;       /* outline of invalid area */
        signed char     nhx, nhy;       /* opposite corner of invalid area */
} dest_area;

typedef struct dungeon {        /* basic dungeon identifier */
        char    dname[24];      /* name of the dungeon (eg. "Hell") */
        char    proto[15];      /* name of prototype file (eg. "tower") */
        char    boneid;         /* character to id dungeon in bones files */
        d_flags flags;          /* dungeon flags */
        signed char     entry_lev;      /* entry level */
        signed char     num_dunlevs;    /* number of levels in this dungeon */
        signed char     dunlev_ureached; /* how deep you have been in this dungeon */
        int     ledger_start,   /* the starting depth in "real" terms */
                depth_start;    /* the starting depth in "logical" terms */
} dungeon;

/*
 * A branch structure defines the connection between two dungeons.  They
 * will be ordered by the dungeon number/level number of 'end1'.  Ties
 * are resolved by 'end2'.  'Type' uses 'end1' arbitrarily as the primary
 * point.
 */
typedef struct branch {
    struct branch *next;        /* next in the branch chain */
    int           id;           /* branch identifier */
    int           type;         /* type of branch */
    d_level       end1;         /* "primary" end point */
    d_level       end2;         /* other end point */
    bool       end1_up;      /* does end1 go up? */
} branch;

/* branch types */
enum {
    BR_STAIR   = 0,    /* "Regular" connection, 2 staircases. */
    BR_NO_END1 = 1,    /* "Regular" connection.  However, no stair from  */
                       /*      end1 to end2.  There is a stair from end2 */
                       /*      to end1.                                  */
    BR_NO_END2 = 2,    /* "Regular" connection.  However, no stair from  */
                       /*      end2 to end1.  There is a stair from end1 */
                       /*      to end2.                                  */
    BR_PORTAL  = 3,    /* Connection by magic portals (traps) */
};


/* monster and object migration codes */

enum {
    MIGR_NOWHERE          =  -1,      /* failure flag for down_gate() */
    MIGR_RANDOM           =   0,
    MIGR_APPROX_XY        =   1,       /* approximate coordinates */
    MIGR_EXACT_XY         =   2,       /* specific coordinates */
    MIGR_STAIRS_UP        =   3,
    MIGR_STAIRS_DOWN      =   4,
    MIGR_LADDER_UP        =   5,
    MIGR_LADDER_DOWN      =   6,
    MIGR_SSTAIRS          =   7,       /* dungeon branch */
    MIGR_PORTAL           =   8,       /* magic portal */
    MIGR_NEAR_PLAYER      =   9,       /* mon: followers; obj: trap door */
};

/* level information (saved via ledger number) */

struct linfo {
        unsigned char   flags;
#define VISITED         0x01    /* hero has visited this level */
#define FORGOTTEN       0x02    /* hero will forget this level when reached */
#define LFILE_EXISTS    0x04    /* a level file exists for this level */
/*
 * Note:  VISITED and LFILE_EXISTS are currently almost always set at the
 * same time.  However they _mean_ different things.
 */
};

void save_dungeon(int,bool,bool);
void restore_dungeon(int);
void insert_branch(branch *,bool);
void init_dungeons(void);
s_level *find_level(const char *);
s_level *Is_special(d_level *);
branch *Is_branchlev(d_level *);
signed char ledger_no(d_level *);
signed char maxledgerno(void);
signed char depth(d_level *);
signed char dunlev(d_level *);
signed char dunlevs_in_dungeon(d_level *);
signed char ledger_to_dnum(signed char);
signed char ledger_to_dlev(signed char);
signed char deepest_lev_reached(bool);
bool on_level(d_level *,d_level *);
void next_level(bool);
void prev_level(bool);
void u_on_newpos(int,int);
void u_on_sstairs(void);
void u_on_upstairs(void);
void u_on_dnstairs(void);
bool On_stairs(signed char,signed char);
void get_level(d_level *,int);
bool Is_botlevel(d_level *);
bool Can_fall_thru(d_level *);
bool Can_dig_down(d_level *);
bool Can_rise_up(int,int,d_level *);
bool In_quest(d_level *);
bool In_mines(d_level *);
branch *dungeon_branch(const char *);
bool at_dgn_entrance(const char *);
bool In_hell(d_level *);
bool In_V_tower(d_level *);
bool On_W_tower_level(d_level *);
bool In_W_tower(int,int,d_level *);
void find_hell(d_level *);
void goto_hell(bool,bool);
void assign_level(d_level *,d_level *);
void assign_rnd_level(d_level *,d_level *,int);
int induced_align(int);
bool Invocation_lev(d_level *);
signed char level_difficulty(void);
signed char lev_by_name(const char *);
signed char print_dungeon(bool,signed char *,signed char *);
bool may_dig(signed char,signed char);
bool may_passwall(signed char,signed char);
bool bad_rock(struct permonst *,signed char,signed char);
bool invocation_pos(signed char,signed char);

#endif /* DUNGEON_H */
