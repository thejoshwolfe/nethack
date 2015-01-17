#ifndef VISION_UTIL_H
#define VISION_UTIL_H

#include "vision.h"
#include "youprop.h"

/*
 *  cansee()    - Returns true if the hero can see the location.
 *
 *  couldsee()  - Returns true if the hero has a clear line of sight to
 *                the location.
 */
static bool cansee(signed char x, signed char y) {
    return viz_array[y][x] & IN_SIGHT;
}
static bool couldsee(signed char x, signed char y) {
    return viz_array[y][x] & COULD_SEE;
}

/*
 *  The following assume the monster is not blind.
 *
 *  m_canseeu() - Returns true if the monster could see the hero.  Assumes
 *                that if the hero has a clear line of sight to the monster's
 *                location and the hero is visible, then monster can see the
 *                hero.
 */
static bool m_canseeu(const struct monst * m) {
    return ((!Invis || perceives(m->data)) && !(Underwater || u.uburied || m->mburied) ? couldsee(m->mx, m->my) : 0);
}

/* get a list of distances of the edges (see vision.c). */
static char * circle_ptr(int z) {
    return &circle_data[(int)circle_start[z]];
}

#endif /* VISION_UTIL_H */
