#ifndef MAKEMON_H
#define MAKEMON_H

#include <stdbool.h>
#include "monst.h"
#include "obj.h"

bool is_home_elemental(struct permonst *);
struct monst *clone_mon(struct monst *,signed char,signed char);
struct monst *makemon(struct permonst *,int,int,int);
bool create_critters(int,struct permonst *);
struct permonst *rndmonst(void);
void reset_rndmonst(int);
struct permonst *mkclass(char,int);
int adj_lev(struct permonst *);
struct permonst *grow_up(struct monst *,struct monst *);
int mongets(struct monst *,int);
int golemhp(int);
bool peace_minded(struct permonst *);
void set_malign(struct monst *);
void set_mimic_sym(struct monst *);
int mbirth_limit(int);
void mimic_hit_msg(struct monst *, short);
void bagotricks(struct obj *);
bool propagate(int, bool,bool);

#endif
