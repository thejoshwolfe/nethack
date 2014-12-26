/* See LICENSE in the root of this project for change info */
#ifndef CONFIG_H
#define CONFIG_H

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

#endif /* CONFIG_H */
