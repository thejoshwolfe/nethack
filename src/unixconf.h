/* See LICENSE in the root of this project for change info */
#ifndef UNIXCONF_H
#define UNIXCONF_H

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
