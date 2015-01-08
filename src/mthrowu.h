#ifndef MTHROWU_H
#define MTHROWU_H

#include <stdbool.h>

#include "monst.h"
#include "obj.h"
#include "permonst.h"

int thitu(int,int,struct obj *,const char *);
int ohitmon(struct monst *,struct obj *,int,bool);
void thrwmu(struct monst *);
int spitmu(struct monst *,struct attack *);
int breamu(struct monst *,struct attack *);
bool linedup(signed char,signed char,signed char,signed char);
bool lined_up(struct monst *);
struct obj *m_carrying(struct monst *,int);
void m_useup(struct monst *,struct obj *);
void m_throw(struct monst *,int,int,int,int,int,struct obj *);
bool hits_bars(struct obj **,int,int,int,int);

#endif // MTHROWU_H
