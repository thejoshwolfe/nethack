#ifndef MHITM_H
#define MHITM_H

#include "monst.h"

int fightm(struct monst *);
int mattackm(struct monst *,struct monst *);
int noattacks(struct permonst *);
int sleep_monst(struct monst *,int,int);
void slept_monst(struct monst *);
long attk_protection(int);

#endif // MHITM_H
