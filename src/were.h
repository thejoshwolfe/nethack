#ifndef WERE_H
#define WERE_H

#include <stdbool.h>

#include "monst.h"

void were_change(struct monst *);
void new_were(struct monst *);
int were_summon(struct permonst *,bool,int *,char *);
void you_were(void);
void you_unwere(bool);

#endif
