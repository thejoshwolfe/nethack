#ifndef MHITU_H
#define MHITU_H

#include "monst.h"

#include <stdbool.h>

const char *mpoisons_subj(struct monst *,struct attack *);
void u_slow_down(void);
struct monst *cloneu(void);
void expels(struct monst *,struct permonst *,bool);
struct attack *getmattk(struct permonst *,int,int *,struct attack *);
int mattacku(struct monst *);
int magic_negation(struct monst *);
int gazemu(struct monst *,struct attack *);
void mdamageu(struct monst *,int);
int could_seduce(struct monst *,struct monst *,struct attack *);
int doseduce(struct monst *);


#define MHITU_H // MHITU_H
