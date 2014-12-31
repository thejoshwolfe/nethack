#ifndef DIG_H
#define DIG_H

#include <stdbool.h>
#include "monst.h"
#include "obj.h"

bool is_digging(void);
int holetime(void);
bool dig_check(struct monst *, bool, int, int);
void digactualhole(int,int,struct monst *,int);
bool dighole(bool);
int use_pick_axe(struct obj *);
int use_pick_axe2(struct obj *);
bool mdig_tunnel(struct monst *);
void watch_dig(struct monst *,signed char,signed char,bool);
void zap_dig(void);
struct obj *bury_an_obj(struct obj *);
void bury_objs(int,int);
void unearth_objs(int,int);
void rot_organic(void *, long);
void rot_corpse(void *, long);

#endif
