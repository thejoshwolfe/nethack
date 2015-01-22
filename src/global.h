/* See LICENSE in the root of this project for change info */

#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>


/* Files expected to exist in the playground directory. */
#define RECORD        "run/record" /* file containing list of topscorers */
#define HELP          "help"    /* file containing command descriptions */
#define SHELP         "hh"      /* abbreviated form of the same */
#define RUMORFILE     "rumors"  /* file with fortune cookies */
#define ORACLEFILE    "oracles" /* file with oracular information */
#define DATAFILE      "data"    /* file giving the meaning of symbols used */
#define CMDHELPFILE   "cmdhelp" /* file telling what commands do */
#define HISTORY       "history" /* file giving nethack's history */
#define LICENSE       "license" /* file with license information */
#define OPTIONFILE    "opthelp" /* file explaining runtime options */
#define OPTIONS_USED  "options" /* compile-time options, for #version */

#define LEV_EXT ".lev"          /* extension for special level files */


#define strcmpi(a,b) strncmpi((a),(b),-1)

#define SIZE(x) (int)(sizeof(x) / sizeof(x[0]))


/* A limit for some NetHack int variables.  It need not, and for comparable
 * scoring should not, depend on the actual limit on integers for a
 * particular machine, although it is set to the minimum required maximum
 * signed integer for C (2^15 -1).
 */
#define LARGEST_INT     32767

/* Used for consistency checks of various data files; */
struct version_info {
        unsigned long   incarnation;    /* actual version number */
        unsigned long   feature_set;    /* bitmask of config settings */
        unsigned long   entity_count;   /* # of monsters and objects */
        unsigned long   struct_sizes;   /* size of key structs */
};


/*
 * Configurable internal parameters.
 *
 * Please be very careful if you are going to change one of these.  Any
 * changes in these parameters, unless properly done, can render the
 * executable inoperative.
 */

/* size of terminal screen is (at least) (ROWNO+3) by COLNO */
enum {
    COLNO = 80,
    ROWNO = 21,
};

#define MAXNROFROOMS    40      /* max number of rooms per level */
#define MAX_SUBROOMS    24      /* max # of subrooms in a given room */
#define DOORMAX         120     /* max number of doors per level */

#define BUFSZ           256
#define QBUFSZ          128     /* for building question text */
#define TBUFSZ          300     /* toplines[] buffer max msg: 3 81char names */
                                /* plus longest prefix plus a few extra words */

#define PL_NSIZ         32      /* name of player, ghost, shopkeeper */
#define PL_CSIZ         32      /* sizeof pl_character */
#define PL_FSIZ         32      /* fruit name */
#define PL_PSIZ         63      /* player-given names for pets, other
                                 * monsters, objects */

#define MAXDUNGEON      16      /* current maximum number of dungeons */
#define MAXLEVEL        32      /* max number of levels in one dungeon */
#define MAXSTAIRS       1       /* max # of special stairways in a dungeon */
#define ALIGNWEIGHT     4       /* generation weight of alignment */

#define MAXULEV         30      /* max character experience level */

#define MAXMONNO        120     /* extinct monst after this number created */
#define MHPMAX          500     /* maximum monster hp */

#endif /* GLOBAL_H */
