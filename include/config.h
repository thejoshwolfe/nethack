#ifndef CONFIG_H
#define CONFIG_H


/*
 * Section 1:	Operating and window systems selection.
 *		Select the version of the OS you are using.
 *		For "UNIX" select BSD, ULTRIX, SYSV, or HPUX in unixconf.h.
 */

#define UNIX		/* delete if no fork(), exec() available */

#include "config1.h"	/* should auto-detect MSDOS, MAC */


/* Windowing systems...
 * Define all of those you want supported in your binary.
 * Some combinations make no sense.  See the installation document.
 */

#ifndef LISP_GRAPHICS
#define TTY_GRAPHICS	/* good old tty based graphics */

/* Debian default window system is always tty; they have to set their
 * own if they want another one (or just use the scripts */
#define DEFAULT_WINDOW_SYS "tty"
#endif

/* MAC also means MAC windows */
#ifdef MAC
# ifndef	AUX
#  define DEFAULT_WINDOW_SYS "mac"
# endif
#endif

#ifndef DEFAULT_WINDOW_SYS
# define DEFAULT_WINDOW_SYS "tty"
#endif

/*
 * Section 2:	Some global parameters and filenames.
 *		Commenting out WIZARD, LOGFILE, NEWS or PANICLOG removes that
 *		feature from the game; otherwise set the appropriate wizard
 *		name.  LOGFILE, NEWS and PANICLOG refer to files in the
 *		playground.
 */

#ifndef WIZARD		/* allow for compile-time or Makefile changes */
# ifndef KR1ED
#  define WIZARD  "root" /* the person allowed to use the -D option */
# else
#  define WIZARD
#  define WIZARD_NAME "root"
# endif
#endif

#define LOGFILE "logfile"	/* larger file for debugging purposes */
#define NEWS "news"		/* the file containing the latest hack news */

/*
 *	If COMPRESS is defined, it should contain the full path name of your
 *	'compress' program.  Defining INTERNAL_COMP causes NetHack to do
 *	simpler byte-stream compression internally.  Both COMPRESS and
 *	INTERNAL_COMP create smaller bones/level/save files, but require
 *	additional code and time.  Currently, only UNIX fully implements
 *	COMPRESS; other ports should be able to uncompress save files a
 *	la unixmain.c if so inclined.
 *	If you define COMPRESS, you must also define COMPRESS_EXTENSION
 *	as the extension your compressor appends to filenames after
 *	compression.
 */

#ifdef UNIX
/* path and file name extension for compression program */
#define COMPRESS "/bin/gzip" /* FSF gzip compression */
#define COMPRESS_EXTENSION ".gz"	/* normal gzip extension */
#endif

#ifndef COMPRESS
# define INTERNAL_COMP	/* control use of NetHack's compression routines */
#endif

/*
 *	Defining INSURANCE slows down level changes, but allows games that
 *	died due to program or system crashes to be resumed from the point
 *	of the last level change, after running a utility program.
 */
#define INSURANCE	/* allow crashed game recovery */

/*
 * Section 3:	Definitions that may vary with system type.
 *		For example, both schar and uchar should be short ints on
 *		the AT&T 3B2/3B5/etc. family.
 */

/*
 * Uncomment the following line if your compiler falsely claims to be
 * a standard C compiler (i.e., defines __STDC__ without cause).
 * Examples are Apollo's cc (in some versions) and possibly SCO UNIX's rcc.
 */
/* #define NOTSTDC */			/* define for lying compilers */

#include "tradstdc.h"

/*
 * type schar: small signed integers (8 bits suffice) (eg. TOS)
 *
 *	typedef char	schar;
 *
 *	will do when you have signed characters; otherwise use
 *
 *	typedef short int schar;
 */
#ifdef AZTEC
# define schar	char
#else
typedef signed char	schar;
#endif

/*
 * type uchar: small unsigned integers (8 bits suffice - but 7 bits do not)
 *
 *	typedef unsigned char	uchar;
 *
 *	will be satisfactory if you have an "unsigned char" type;
 *	otherwise use
 *
 *	typedef unsigned short int uchar;
 */
#ifndef _AIX32		/* identical typedef in system file causes trouble */
typedef unsigned char	uchar;
#endif

/*
 * Various structures have the option of using bitfields to save space.
 * If your C compiler handles bitfields well (e.g., it can initialize structs
 * containing bitfields), you can define BITFIELDS.  Otherwise, the game will
 * allocate a separate character for each bitfield.  (The bitfields used never
 * have more than 7 bits, and most are only 1 bit.)
 */
#define BITFIELDS	/* Good bitfield handling */

/* #define STRNCMPI */	/* compiler/library has the strncmpi function */

/*
 * There are various choices for the NetHack vision system.  There is a
 * choice of two algorithms with the same behavior.  Defining VISION_TABLES
 * creates huge (60K) tables at compile time, drastically increasing data
 * size, but runs slightly faster than the alternate algorithm.  (MSDOS in
 * particular cannot tolerate the increase in data size; other systems can
 * flip a coin weighted to local conditions.)
 *
 * If VISION_TABLES is not defined, things will be faster if you can use
 * MACRO_CPATH.  Some cpps, however, cannot deal with the size of the
 * functions that have been macroized.
 */

/* #define VISION_TABLES */ /* use vision tables generated at compile time */
#ifndef VISION_TABLES
# ifndef NO_MACRO_CPATH
#  define MACRO_CPATH	/* use clear_path macros instead of functions */
# endif
#endif

/*
 * Section 4:  THE FUN STUFF!!!
 *
 * Conditional compilation of special options are controlled here.
 * If you define the following flags, you will add not only to the
 * complexity of the game but also to the size of the load module.
 */

/* dungeon features */
#define SINKS		/* Kitchen sinks - Janet Walz */
/* dungeon levels */
#define WALLIFIED_MAZE	/* Fancy mazes - Jean-Christophe Collet */
#define REINCARNATION	/* Special Rogue-like levels */
/* monsters & objects */
#define SEDUCE		/* Succubi/incubi seduction, by KAA, suggested by IM */
#define STEED		/* Riding steeds */
#define TOURIST		/* Tourist players with cameras and Hawaiian shirts */
/* difficulty */
#define ELBERETH	/* Engraving the E-word repels monsters */
/* I/O */
#define REDO		/* support for redoing last command - DGK */
#if !defined(MAC)
# define CLIPPING	/* allow smaller screens -- ERS */
#endif

#ifdef REDO
# define DOAGAIN '\001' /* ^A, the "redo" key used in cmd.c and getline.c */
#endif

#define EXP_ON_BOTL	/* Show experience on bottom line */
#define PARANOID

#define HPMON           /* Color HP monitor */
#define SORTLOOT        /* Sort yer loot by alphabetical order, not index */

/* #define SCORE_ON_BOTL */	/* added by Gary Erickson (erickson@ucivax) */

/*
 * Section 5:  EXPERIMENTAL STUFF
 *
 * Conditional compilation of new or experimental options are controlled here.
 * Enable any of these at your own risk -- there are almost certainly
 * bugs left here.
 */

#ifdef TTY_GRAPHICS
# define MENU_COLOR
#ifdef __linux__
# define MENU_COLOR_REGEX
/* if MENU_COLOR_REGEX is defined, use regular expressions (GNU regex.h)
 * otherwise use pmatch() to match menu color lines.
 * pmatch() provides basic globbing: '*' and '?' wildcards.
 */
#endif
#endif

/*#define GOLDOBJ */	/* Gold is kept on obj chains - Helge Hafting */
#define AUTOPICKUP_EXCEPTIONS  /* exceptions to autopickup */
#define DUMP_LOG
#define DUMP_FN "run/dumps/%n.lastgame.txt"

/* End of Section 5 */

#include "global.h"	/* Define everything else according to choices above */

#endif /* CONFIG_H */
