#ifndef DOGMOVE_H
#define DOGMOVE_H

#include <stdbool.h>

int dog_nutrition(struct monst *,struct obj *);
int dog_eat(struct monst *,struct obj *,int,int,bool);
int dog_move(struct monst *,int);

#endif // DOGMOVE_H
