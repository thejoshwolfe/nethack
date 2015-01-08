#ifndef DOTHROW_H
#define DOTHROW_H

#include <stdbool.h>

#include "coord.h"
#include "monst.h"
#include "obj.h"

int dothrow(void);
int dofire(void);
void hitfloor(struct obj *);
void hurtle(int,int,int,bool);
void mhurtle(struct monst *,int,int,int);
void throwit(struct obj *,long,bool);
int omon_adj(struct monst *,struct obj *,bool);
int thitmonst(struct monst *,struct obj *);
int hero_breaks(struct obj *,signed char,signed char,bool);
int breaks(struct obj *,signed char,signed char);
bool breaktest(struct obj *);
bool walk_path(coord *, coord *, bool (*)(void *,int,int), void *);
bool hurtle_step(void *, int, int);


#endif // DOTHROW_H
