#ifndef MONMOVE_H
#define MONMOVE_H

#include <stdbool.h>
#include "monst.h"

bool itsstuck(struct monst *);
bool mb_trapped(struct monst *);
void mon_regen(struct monst *,bool);
int dochugw(struct monst *);
bool onscary(int,int,struct monst *);
void monflee(struct monst *, int, bool, bool);
int dochug(struct monst *);
int m_move(struct monst *,int);
bool closed_door(int,int);
bool accessible(int,int);
void set_apparxy(struct monst *);
bool can_ooze(struct monst *);

#endif // MONMOVE_H
