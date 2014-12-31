#ifndef STEED_H
#define STEED_H

#include <stdbool.h>
#include "monst.h"
#include "obj.h"

void rider_cant_reach(void);
bool can_saddle(struct monst *);
int use_saddle(struct obj *);
bool can_ride(struct monst *);
int doride(void);
bool mount_steed(struct monst *, bool);
void exercise_steed(void);
void kick_steed(void);
void dismount_steed(int);
void place_monster(struct monst *,int,int);

#endif // STEED_H
