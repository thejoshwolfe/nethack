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
 *  Circle information
 */
#define MAX_RADIUS 15   /* this is in points from the source */

extern char circle_data[];
extern char circle_start[];

void vision_init(void);
int does_block(int,int,struct rm*);
void vision_reset(void);
void vision_recalc(int);
void block_point(int,int);
void unblock_point(int,int);
bool clear_path(int,int,int,int);
void do_clear_area(int,int,int, void (*)(int,int,void *),void *);


#endif /* VISION_H */
