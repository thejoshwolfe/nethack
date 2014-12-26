/* See LICENSE in the root of this project for change info */
#ifndef UNIXCONF_H
#define UNIXCONF_H

/*
 * If you define MAIL, then the player will be notified of new mail
 * when it arrives.  If you also define DEF_MAILREADER then this will
 * be the default mail reader, and can be overridden by the environment
 * variable MAILREADER; otherwise an internal pager will be used.
 * A stat system call is done on the mailbox every MAILCKFREQ moves.
 */

#define MAIL                    /* Deliver mail during the game */
#define DEF_MAILREADER  "/usr/bin/mail"

#define FCMASK  0660    /* file creation mask */

#include <time.h>

#ifndef REDO
#define Getchar nhgetch
#endif
#define tgetch getchar

#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/* Use the high quality random number routines. */
#define Rand()  random()

#define msleep(k) usleep((k)*1000)

#endif /* UNIXCONF_H */
