/* See LICENSE in the root of this project for change info */
#ifndef UNIXCONF_H
#define UNIXCONF_H

#ifdef TTY_GRAPHICS
/*
 * To enable the `timed_delay' option for using a timer rather than extra
 * screen output when pausing for display effect.  Requires that `msleep'
 * function be available (with time argument specified in milliseconds).
 * Various output devices can produce wildly varying delays when the
 * "extra output" method is used, but not all systems provide access to
 * a fine-grained timer.
 */
#define TIMED_DELAY	/* usleep() */
#endif

/*
 * If you define MAIL, then the player will be notified of new mail
 * when it arrives.  If you also define DEF_MAILREADER then this will
 * be the default mail reader, and can be overridden by the environment
 * variable MAILREADER; otherwise an internal pager will be used.
 * A stat system call is done on the mailbox every MAILCKFREQ moves.
 */

#define MAIL			/* Deliver mail during the game */
#define DEF_MAILREADER	"/usr/bin/mail"

#define FCMASK	0660	/* file creation mask */

#include <time.h>

#define HLOCK	"run/perm" /* an empty file used for locking purposes */

#ifndef REDO
#define Getchar nhgetch
#endif
#define tgetch getchar

#define SHELL		/* do not delete the '!' command */

#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/* Use the high quality random number routines. */
#define Rand()	random()

#ifdef TIMED_DELAY
# define msleep(k) usleep((k)*1000)
#endif

#endif /* UNIXCONF_H */
