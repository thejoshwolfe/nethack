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

#endif /* CONFIG_H */
