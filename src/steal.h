#ifndef STEAL_H
#define STEAL_H

#include <stdbool.h>

#include "global.h"
#include "monst.h"
#include "obj.h"

long somegold(void);
void stealgold(struct monst *);
void remove_worn_item(struct obj *,bool);
int steal(struct monst *, char *);
int mpickobj(struct monst *,struct obj *);
void stealamulet(struct monst *);
void mdrop_special_objs(struct monst *);
void relobj(struct monst *,int,bool);

#endif
