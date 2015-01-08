/* See LICENSE in the root of this project for change info */
#ifndef VISION_H
#define VISION_H

#include <stdbool.h>

#include "decl.h"
#include "rm.h"

#define COULD_SEE 0x1           /* location could be seen, if it were lit */
#define IN_SIGHT  0x2           /* location can be seen */
#define TEMP_LIT  0x4           /* location is temporarily lit */

/*
 * Light source sources
 */
#define LS_OBJECT 0
#define LS_MONSTER 1

/*
 *  cansee()    - Returns true if the hero can see the location.
 *
 *  couldsee()  - Returns true if the hero has a clear line of sight to
 *                the location.
 */
#define cansee(x,y)     (viz_array[y][x] & IN_SIGHT)
#define couldsee(x,y)   (viz_array[y][x] & COULD_SEE)
#define templit(x,y)    (viz_array[y][x] & TEMP_LIT)

/*
 *  The following assume the monster is not blind.
 *
 *  m_cansee()  - Returns true if the monster can see the given location.
 *
 *  m_canseeu() - Returns true if the monster could see the hero.  Assumes
 *                that if the hero has a clear line of sight to the monster's
 *                location and the hero is visible, then monster can see the
 *                hero.
 */
#define m_cansee(mtmp,x2,y2)    clear_path((mtmp)->mx,(mtmp)->my,(x2),(y2))

#define m_canseeu(m)    ((!Invis || perceives((m)->data)) && \
                          !(Underwater || u.uburied || (m)->mburied) ? \
                             couldsee((m)->mx,(m)->my) : 0)

/*
 *  Circle information
 */
#define MAX_RADIUS 15   /* this is in points from the source */

/* Use this macro to get a list of distances of the edges (see vision.c). */
#define circle_ptr(z) (&circle_data[(int)circle_start[z]])

void vision_init(void);
int does_block(int,int,struct rm*);
void vision_reset(void);
void vision_recalc(int);
void block_point(int,int);
void unblock_point(int,int);
bool clear_path(int,int,int,int);
void do_clear_area(int,int,int, void (*)(int,int,void *),void *);


#endif /* VISION_H */
