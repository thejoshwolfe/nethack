/* See LICENSE in the root of this project for change info */
#ifndef CONFIG_H
#define CONFIG_H


/*
 * Section 1:   Operating and window systems selection.
 */

/* Windowing systems...
 * Define all of those you want supported in your binary.
 * Some combinations make no sense.  See the installation document.
 */

#define TTY_GRAPHICS    /* good old tty based graphics */

/* Debian default window system is always tty; they have to set their
 * own if they want another one (or just use the scripts */
#define DEFAULT_WINDOW_SYS "tty"

/*
 * Section 2:   Some global parameters and filenames.
 *              Commenting out WIZARD, PANICLOG removes that
 *              feature from the game; otherwise set the appropriate wizard
 *              name.  PANICLOG refer to files in the
 *              playground.
 */

#ifndef WIZARD          /* allow for compile-time or Makefile changes */
# ifndef KR1ED
#  define WIZARD  "root" /* the person allowed to use the -D option */
# else
#  define WIZARD
#  define WIZARD_NAME "root"
# endif
#endif

/*
 *      If COMPRESS is defined, it should contain the full path name of your
 *      'compress' program.  Defining INTERNAL_COMP causes NetHack to do
 *      simpler byte-stream compression internally.  Both COMPRESS and
 *      INTERNAL_COMP create smaller bones/level/save files, but require
 *      additional code and time.  Currently, only UNIX fully implements
 *      COMPRESS; other ports should be able to uncompress save files
 *      if so inclined.
 *      If you define COMPRESS, you must also define COMPRESS_EXTENSION
 *      as the extension your compressor appends to filenames after
 *      compression.
 */

/* path and file name extension for compression program */
#define COMPRESS "/bin/gzip" /* FSF gzip compression */
#define COMPRESS_EXTENSION ".gz"        /* normal gzip extension */

#ifndef COMPRESS
# define INTERNAL_COMP  /* control use of NetHack's compression routines */
#endif

/*
 * Section 3:   Definitions that may vary with system type.
 *              For example, both signed char and unsigned char should be short ints on
 *              the AT&T 3B2/3B5/etc. family.
 */

/*
 * There are various choices for the NetHack vision system.  There is a
 * choice of two algorithms with the same behavior.  Defining VISION_TABLES
 * creates huge (60K) tables at compile time, drastically increasing data
 * size, but runs slightly faster than the alternate algorithm.
 *
 * If VISION_TABLES is not defined, things will be faster if you can use
 * MACRO_CPATH.  Some cpps, however, cannot deal with the size of the
 * functions that have been macroized.
 */

/* #define VISION_TABLES */ /* use vision tables generated at compile time */
#ifndef VISION_TABLES
# ifndef NO_MACRO_CPATH
#  define MACRO_CPATH   /* use clear_path macros instead of functions */
# endif
#endif

/*
 * Section 4:  THE FUN STUFF!!!
 *
 * Conditional compilation of special options are controlled here.
 * If you define the following flags, you will add not only to the
 * complexity of the game but also to the size of the load module.
 */

/* monsters & objects */

#define DOAGAIN '\001' /* ^A, the "redo" key used in cmd.c and getline.c */


/*
 * Section 5:  EXPERIMENTAL STUFF
 *
 * Conditional compilation of new or experimental options are controlled here.
 * Enable any of these at your own risk -- there are almost certainly
 * bugs left here.
 */

#ifdef TTY_GRAPHICS
# define MENU_COLOR
#endif

#define AUTOPICKUP_EXCEPTIONS  /* exceptions to autopickup */

/* End of Section 5 */

#endif /* CONFIG_H */
