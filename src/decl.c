/* See LICENSE in the root of this project for change info */

#include <stdbool.h>

#include "color.h"
#include "coord.h"
#include "decl.h"
#include "display.h"
#include "dungeon.h"
#include "flag.h"
#include "global.h"
#include "hack.h"
#include "mkroom.h"
#include "monst.h"
#include "obj.h"
#include "objclass.h"
#include "onames.h"
#include "pm.h"
#include "quest.h"
#include "rm.h"
#include "spell.h"
#include "wintype.h"
#include "you.h"

int (*afternmv)(void);
int (*occupation)(void);

/* from xxxmain.c */
const char *hname = 0;          /* name of the game (argv[0] of main) */
int hackpid = 0;                /* current process id */
int locknum = 0;                /* max num of simultaneous users */

int bases[MAXOCLASSES] = DUMMY;

int multi = 0; // never positive
int nroom = 0;
int nsubroom = 0;
int occtime = 0;

int x_maze_max, y_maze_max;     /* initialized in main, used in mkmaze.c */
int otg_temp;                   /* used by object_to_glyph() [otg] */

/*
 *      The following structure will be initialized at startup time with
 *      the level numbers of some "important" things in the game.
 */
struct dgn_topology dungeon_topology = {DUMMY};

struct q_score  quest_status = DUMMY;

int smeq[MAXNROFROOMS+1] = DUMMY;
int doorindex = 0;

struct Killer killer;
struct Killer delayed_killer;

const char *nomovemsg = 0;
const char nul[40] = DUMMY;                     /* contains zeros */
char plname[PL_NSIZ] = DUMMY;           /* player name */
char pl_character[PL_CSIZ] = DUMMY;
char pl_race = '\0';

char pl_fruit[PL_FSIZ] = DUMMY;
int current_fruit = 0;
struct fruit *ffruit = (struct fruit *)0;

char tune[6] = DUMMY;

const char *occtxt = DUMMY;
const char quitchars[] = " \r\n\033";
const char vowels[] = "aeiouAEIOU";
const char ynchars[] = "yn";
const char ynqchars[] = "ynq";
const char ynaqchars[] = "ynaq";
const char ynNaqchars[] = "yn#aq";
long yn_number = 0L;

const char disclosure_options[] = "iavgc";


struct linfo level_info[MAXLINFO];

struct sinfo program_state;

const char ndir[] = "47896321";
const signed char xdir[] = { -1, -1,  0,  1,  1,  1,  0, -1 };
const signed char ydir[] = {  0, -1, -1, -1,  0,  1,  1,  1 };

signed char tbx = 0, tby = 0;   /* mthrowu: target */

/* for xname handling of multiple shot missile volleys:
   number of shots, index of current one, validity check, shoot vs throw */
struct multishot m_shot = { 0, 0, STRANGE_OBJECT, false };

struct dig_info digging;

dungeon dungeons[MAXDUNGEON];   /* ini'ed by init_dungeon() */
s_level *sp_levchn;
stairway upstair = { 0, 0 }, dnstair = { 0, 0 };
stairway upladder = { 0, 0 }, dnladder = { 0, 0 };
stairway sstairs = { 0, 0 };
dest_area updest = { 0, 0, 0, 0, 0, 0, 0, 0 };
dest_area dndest = { 0, 0, 0, 0, 0, 0, 0, 0 };
coord inv_pos = { 0, 0 };

bool in_mklev = false;
bool stoned = false; /* done to monsters hit by 'c' */
bool unweapon = false;
bool mrg_to_wielded = false;
                         /* weapon picked is merged with wielded one */
struct obj *current_wand = 0;   /* wand currently zapped/applied */

bool in_steed_dismounting = false;

coord bhitpos = DUMMY;
coord doors[DOORMAX] = {DUMMY};

struct mkroom rooms[(MAXNROFROOMS+1)*2] = {DUMMY};
struct mkroom* subrooms = &rooms[MAXNROFROOMS+1];
struct mkroom *upstairs_room, *dnstairs_room, *sstairs_room;

dlevel_t level;         /* level map */
struct trap *ftrap = (struct trap *)0;
struct monst youmonst = DUMMY;
struct flag flags = DUMMY;
struct instance_flags iflags = DUMMY;
struct you u = DUMMY;

struct obj *invent = (struct obj *)0,
        *uwep = (struct obj *)0, *uarm = (struct obj *)0,
        *uswapwep = (struct obj *)0,
        *uquiver = (struct obj *)0, /* quiver */
        *uarmu = (struct obj *)0, /* under-wear, so to speak */
        *uskin = (struct obj *)0, /* dragon armor, if a dragon */
        *uarmc = (struct obj *)0, *uarmh = (struct obj *)0,
        *uarms = (struct obj *)0, *uarmg = (struct obj *)0,
        *uarmf = (struct obj *)0, *uamul = (struct obj *)0,
        *uright = (struct obj *)0,
        *uleft = (struct obj *)0,
        *ublindf = (struct obj *)0,
        *uchain = (struct obj *)0,
        *uball = (struct obj *)0;

/*
 *  This must be the same order as used for buzz() in zap.c.
 */
const int zapcolors[NUM_ZAP] = {
    HI_ZAP,             /* 0 - missile */
    CLR_ORANGE,         /* 1 - fire */
    CLR_WHITE,          /* 2 - frost */
    HI_ZAP,             /* 3 - sleep */
    CLR_BLACK,          /* 4 - death */
    CLR_WHITE,          /* 5 - lightning */
    CLR_YELLOW,         /* 6 - poison gas */
    CLR_GREEN,          /* 7 - acid */
};

const int shield_static[SHIELD_COUNT] = {
    S_ss1, S_ss2, S_ss3, S_ss2, S_ss1, S_ss2, S_ss4,    /* 7 per row */
    S_ss1, S_ss2, S_ss3, S_ss2, S_ss1, S_ss2, S_ss4,
    S_ss1, S_ss2, S_ss3, S_ss2, S_ss1, S_ss2, S_ss4,
};

struct spell spl_book[MAXSPELL + 1] = {DUMMY};

long moves = 1L, monstermoves = 1L;
         /* These diverge when player is Fast */
long wailmsg = 0L;

/* objects that are moving to another dungeon level */
struct obj *migrating_objs = (struct obj *)0;
/* objects not yet paid for */
struct obj *billobjs = (struct obj *)0;

/* used to zero all elements of a struct obj */
struct obj zeroobj = DUMMY;

/* originally from dog.c */
char dogname[PL_PSIZ] = DUMMY;
char catname[PL_PSIZ] = DUMMY;
char horsename[PL_PSIZ] = DUMMY;
char preferred_pet;     /* '\0', 'c', 'd', 'n' (none) */
/* monsters that went down/up together with @ */
struct monst *mydogs = (struct monst *)0;
/* monsters that are moving to another dungeon level */
struct monst *migrating_mons = (struct monst *)0;

struct mvitals mvitals[NUMMONS];

char dump_fn[] = "run/dumps/%n.lastgame.txt";

struct c_color_names c_color_names = {
        "black", "amber", "golden",
        "light blue", "red", "green",
        "silver", "blue", "purple",
        "white"
};

const char *c_obj_colors[] = {
        "black",                /* CLR_BLACK */
        "red",                  /* CLR_RED */
        "green",                /* CLR_GREEN */
        "brown",                /* CLR_BROWN */
        "blue",                 /* CLR_BLUE */
        "magenta",              /* CLR_MAGENTA */
        "cyan",                 /* CLR_CYAN */
        "gray",                 /* CLR_GRAY */
        "transparent",          /* no_color */
        "orange",               /* CLR_ORANGE */
        "bright green",         /* CLR_BRIGHT_GREEN */
        "yellow",               /* CLR_YELLOW */
        "bright blue",          /* CLR_BRIGHT_BLUE */
        "bright magenta",       /* CLR_BRIGHT_MAGENTA */
        "bright cyan",          /* CLR_BRIGHT_CYAN */
        "white",                /* CLR_WHITE */
};

/* NOTE: the order of these words exactly corresponds to the
   order of oc_material values #define'd in objclass.h. */
const char *materialnm[] = {
        "mysterious", "liquid", "wax", "organic", "flesh",
        "paper", "cloth", "leather", "wooden", "bone", "dragonhide",
        "iron", "metal", "copper", "silver", "gold", "platinum", "mithril",
        "plastic", "glass", "gemstone", "stone"
};

/* Vision */
bool vision_full_recalc = 0;
char     **viz_array = 0;/* used in cansee() and couldsee() macros */

/* Global windowing data, defined here for multi-window-system support */
winid WIN_MESSAGE = WIN_ERR, WIN_STATUS = WIN_ERR;
winid WIN_MAP = WIN_ERR, WIN_INVEN = WIN_ERR;
char toplines[TBUFSZ];

char *fqn_prefix_names[PREFIX_COUNT] = { "hackdir", "leveldir", "savedir",
                                        "bonesdir", "datadir", "scoredir",
                                        "lockdir", "configdir", "troubledir" };

