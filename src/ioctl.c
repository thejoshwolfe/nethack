/* See LICENSE in the root of this project for change info */

/* This cannot be part of hack.tty.c (as it was earlier) since on some
   systems (e.g. MUNIX) the include files <termio.h> and <sgtty.h>
   define the same constants, and the C preprocessor complains. */

#include "hack.h"

#include <termios.h>
struct termios termio;

void getwindowsz(void) { }

void getioctls (void) {
    (void) tcgetattr(fileno(stdin), &termio);
    getwindowsz();
}

void setioctls(void) {
    tcsetattr(fileno(stdin), TCSADRAIN, &termio);
}
