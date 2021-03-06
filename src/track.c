/* See LICENSE in the root of this project for change info */

#include "decl.h"
#include "hacklib.h"
#include "track.h"
#include "you.h"

#define UTSZ    50

static int utcnt, utpnt;
static coord utrack[UTSZ];

void clear_footprints(void) {
    utcnt = utpnt = 0;
}

void add_footprint(void) {
    if (utcnt < UTSZ)
        utcnt++;
    if (utpnt == UTSZ)
        utpnt = 0;
    utrack[utpnt].x = u.ux;
    utrack[utpnt].y = u.uy;
    utpnt++;
}

coord * get_footprint_near(int x, int y) {
    int cnt, ndist;
    coord *tc;
    cnt = utcnt;
    for (tc = &utrack[utpnt]; cnt--;) {
        if (tc == utrack)
            tc = &utrack[UTSZ - 1];
        else
            tc--;
        ndist = distmin(x, y, tc->x, tc->y);

        /* if far away, skip track entries til we're closer */
        if (ndist > 2) {
            ndist -= 2; /* be careful due to extra decrement at top of loop */
            cnt -= ndist;
            if (cnt <= 0)
                return (coord *)0; /* too far away, no matches possible */
            if (tc < &utrack[ndist])
                tc += (UTSZ - ndist);
            else
                tc -= ndist;
        } else if (ndist <= 1)
            return (ndist ? tc : 0);
    }
    return NULL;
}
