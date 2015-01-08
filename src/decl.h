/* See LICENSE in the root of this project for change info */

#ifndef DECL_H
#define DECL_H

#include <stdbool.h>

#include "coord.h"
#include "dungeon.h"
#include "global.h"
#include "hack.h"
#include "monst.h"
#include "monsym.h"
#include "obj.h"
#include "objclass.h"
#include "pm.h"
#include "quest.h"
#include "spell.h"
#include "wintype.h"
#include "you.h"


extern int (*occupation)(void);
extern int (*afternmv)(void);

extern const char *hname;
extern int hackpid;
extern int locknum;

extern char SAVEF[];

extern int bases[MAXOCLASSES];

extern int multi;
extern int nroom;
extern int nsubroom;
extern int occtime;

#define WARNCOUNT 6                     /* number of different warning levels */
extern unsigned char warnsyms[WARNCOUNT];

extern int x_maze_max, y_maze_max;
extern int otg_temp;

extern struct dgn_topology {            /* special dungeon levels for speed */
    d_level     d_oracle_level;
    d_level     d_bigroom_level;        /* unused */
    d_level     d_medusa_level;
    d_level     d_stronghold_level;
    d_level     d_valley_level;
    d_level     d_wiz1_level;
    d_level     d_wiz2_level;
    d_level     d_wiz3_level;
    d_level     d_juiblex_level;
    d_level     d_orcus_level;
    d_level     d_baalzebub_level;      /* unused */
    d_level     d_asmodeus_level;       /* unused */
    d_level     d_portal_level;         /* only in goto_level() [do.c] */
    d_level     d_sanctum_level;
    d_level     d_earth_level;
    d_level     d_water_level;
    d_level     d_fire_level;
    d_level     d_air_level;
    d_level     d_astral_level;
    signed char d_tower_dnum;
    signed char d_sokoban_dnum;
    signed char d_mines_dnum, d_quest_dnum;
    d_level     d_qstart_level, d_qlocate_level, d_nemesis_level;
    d_level     d_knox_level;
} dungeon_topology;
/* macros for accesing the dungeon levels by their old names */
#define oracle_level            (dungeon_topology.d_oracle_level)
#define bigroom_level           (dungeon_topology.d_bigroom_level)
#define medusa_level            (dungeon_topology.d_medusa_level)
#define stronghold_level        (dungeon_topology.d_stronghold_level)
#define valley_level            (dungeon_topology.d_valley_level)
#define wiz1_level              (dungeon_topology.d_wiz1_level)
#define wiz2_level              (dungeon_topology.d_wiz2_level)
#define wiz3_level              (dungeon_topology.d_wiz3_level)
#define juiblex_level           (dungeon_topology.d_juiblex_level)
#define orcus_level             (dungeon_topology.d_orcus_level)
#define baalzebub_level         (dungeon_topology.d_baalzebub_level)
#define asmodeus_level          (dungeon_topology.d_asmodeus_level)
#define portal_level            (dungeon_topology.d_portal_level)
#define sanctum_level           (dungeon_topology.d_sanctum_level)
#define earth_level             (dungeon_topology.d_earth_level)
#define water_level             (dungeon_topology.d_water_level)
#define fire_level              (dungeon_topology.d_fire_level)
#define air_level               (dungeon_topology.d_air_level)
#define astral_level            (dungeon_topology.d_astral_level)
#define tower_dnum              (dungeon_topology.d_tower_dnum)
#define sokoban_dnum            (dungeon_topology.d_sokoban_dnum)
#define mines_dnum              (dungeon_topology.d_mines_dnum)
#define quest_dnum              (dungeon_topology.d_quest_dnum)
#define qstart_level            (dungeon_topology.d_qstart_level)
#define qlocate_level           (dungeon_topology.d_qlocate_level)
#define nemesis_level           (dungeon_topology.d_nemesis_level)
#define knox_level              (dungeon_topology.d_knox_level)

extern stairway dnstair, upstair;               /* stairs up and down */
#define xdnstair        (dnstair.sx)
#define ydnstair        (dnstair.sy)
#define xupstair        (upstair.sx)
#define yupstair        (upstair.sy)

extern stairway dnladder, upladder;             /* ladders up and down */
#define xdnladder       (dnladder.sx)
#define ydnladder       (dnladder.sy)
#define xupladder       (upladder.sx)
#define yupladder       (upladder.sy)

extern stairway sstairs;

extern dest_area updest, dndest;        /* level-change destination areas */

extern coord inv_pos;
extern dungeon dungeons[];
extern s_level *sp_levchn;
#define dunlev_reached(x)       (dungeons[(x)->dnum].dunlev_ureached)

extern struct q_score quest_status;

extern char pl_character[PL_CSIZ];
extern char pl_race;            /* character's race */

extern char pl_fruit[PL_FSIZ];
extern int current_fruit;
extern struct fruit *ffruit;

extern char tune[6];

#define MAXLINFO (MAXDUNGEON * MAXLEVEL)
extern struct linfo level_info[MAXLINFO];

extern struct sinfo {
        int gameover;           /* self explanatory? */
        int stopprint;          /* inhibit further end of game disclosure */
        int done_hup;           /* SIGHUP or moral equivalent received
                                 * -- no more screen output */
        int something_worth_saving;     /* in case of panic */
        int panicking;          /* `panic' is in progress */
        int in_impossible;
} program_state;

extern bool restoring;

extern const char quitchars[];
extern const char vowels[];
extern const char ynchars[];
extern const char ynqchars[];
extern const char ynaqchars[];
extern const char ynNaqchars[];
extern long yn_number;

extern const char disclosure_options[];

extern int smeq[];
extern int doorindex;
extern char *save_cm;

extern struct Killer killer;
extern struct Killer delayed_killer;

extern char dump_fn[];          /* dumpfile name (dump patch) */
extern const char *configfile;
extern char plname[PL_NSIZ];
extern char dogname[];
extern char catname[];
extern char horsename[];
extern char preferred_pet;
extern const char *occtxt;                      /* defined when occupation != NULL */
extern const char *nomovemsg;
extern const char nul[];
extern char lock[];

extern const char sdir[], ndir[];
extern const signed char xdir[], ydir[], zdir[];

extern signed char tbx, tby;            /* set in mthrowu.c */

extern struct multishot { int n, i; short o; bool s; } m_shot;

extern struct dig_info {                /* apply.c, hack.c */
        int     effort;
        d_level level;
        coord   pos;
        long lastdigtime;
        bool down, chew, warned, quiet;
} digging;

extern long moves, monstermoves;
extern long wailmsg;

extern bool in_mklev;
extern bool stoned;
extern bool unweapon;
extern bool mrg_to_wielded;
extern struct obj *current_wand;

extern bool in_steed_dismounting;

extern const int shield_static[];

extern struct spell spl_book[]; /* sized in decl.c */

extern const int zapcolors[];

extern const char def_oc_syms[MAXOCLASSES];     /* default class symbols */
extern unsigned char oc_syms[MAXOCLASSES];              /* current class symbols */
extern const char def_monsyms[MAXMCLASSES];     /* default class symbols */
extern unsigned char monsyms[MAXMCLASSES];              /* current class symbols */

extern struct obj *invent,
        *uarm, *uarmc, *uarmh, *uarms, *uarmg, *uarmf,
        *uarmu,                         /* under-wear, so to speak */
        *uskin, *uamul, *uleft, *uright, *ublindf,
        *uwep, *uswapwep, *uquiver;

extern struct obj *uchain;              /* defined only when punished */
extern struct obj *uball;
extern struct obj *migrating_objs;
extern struct obj *billobjs;
extern struct obj zeroobj;              /* init'd and defined in decl.c */

extern struct you u;


extern struct monst youmonst;   /* init'd and defined in decl.c */
extern struct monst *mydogs, *migrating_mons;

extern struct mvitals {
        unsigned char   born;
        unsigned char   died;
        unsigned char   mvflags;
} mvitals[NUMMONS];

extern struct c_color_names {
    const char  *const c_black, *const c_amber, *const c_golden,
                *const c_light_blue,*const c_red, *const c_green,
                *const c_silver, *const c_blue, *const c_purple,
                *const c_white;
} c_color_names;
#define NH_BLACK                c_color_names.c_black
#define NH_AMBER                c_color_names.c_amber
#define NH_GOLDEN               c_color_names.c_golden
#define NH_LIGHT_BLUE           c_color_names.c_light_blue
#define NH_RED                  c_color_names.c_red
#define NH_GREEN                c_color_names.c_green
#define NH_SILVER               c_color_names.c_silver
#define NH_BLUE                 c_color_names.c_blue
#define NH_PURPLE               c_color_names.c_purple
#define NH_WHITE                c_color_names.c_white

/* The names of the colors used for gems, etc. */
extern const char *c_obj_colors[];

/* material strings */
extern const char *materialnm[];

/* Monster name articles */
enum {
    ARTICLE_NONE  = 0,
    ARTICLE_THE   = 1,
    ARTICLE_A     = 2,
    ARTICLE_YOUR  = 3,
};

/* Monster name suppress masks */
#define SUPPRESS_IT             0x01
#define SUPPRESS_INVISIBLE      0x02
#define SUPPRESS_HALLUCINATION  0x04
#define SUPPRESS_SADDLE         0x08
#define EXACT_NAME              0x0F

/* Vision */
extern bool vision_full_recalc;      /* true if need vision recalc */
extern char **viz_array;                /* could see/in sight row pointers */

/* Window system stuff */
extern winid WIN_MESSAGE, WIN_STATUS;
extern winid WIN_MAP, WIN_INVEN;
extern char toplines[];

/* xxxexplain[] is in drawing.c */
extern const char * const monexplain[], invisexplain[], * const objexplain[], * const oclass_names[];

/* Some systems want to use full pathnames for some subsets of file names,
 * rather than assuming that they're all in the current directory.  This
 * provides all the subclasses that seem reasonable, and sets up for all
 * prefixes being null.  Port code can set those that it wants.
 */
#define HACKPREFIX      0
#define LEVELPREFIX     1
#define SAVEPREFIX      2
#define BONESPREFIX     3
#define DATAPREFIX      4       /* this one must match hardcoded value in dlb.c */
#define SCOREPREFIX     5
#define LOCKPREFIX      6
#define CONFIGPREFIX    7
#define TROUBLEPREFIX   8
#define PREFIX_COUNT    9

extern char *fqn_prefix_names[PREFIX_COUNT];

#endif /* DECL_H */
