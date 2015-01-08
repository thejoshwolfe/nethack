#ifndef WORM_H
#define WORM_H

#include <stdbool.h>

#include "monst.h"
#include "obj.h"

int get_wormno(void);
void initworm(struct monst *,int);
void worm_move(struct monst *);
void worm_nomove(struct monst *);
void wormgone(struct monst *);
void wormhitu(struct monst *);
void cutworm(struct monst *,signed char,signed char,struct obj *);
void see_wsegs(struct monst *);
void detect_wsegs(struct monst *,bool);
void save_worm(int,int);
void rest_worm(int);
void place_wsegs(struct monst *);
void remove_worm(struct monst *);
void place_worm_tail_randomly(struct monst *,signed char,signed char);
int count_wsegs(struct monst *);
bool worm_known(const struct monst *);

#endif // WORM_H
