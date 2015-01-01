#ifndef FOUNTAIN_H
#define FOUNTAIN_H

#include <stdbool.h>
#include "obj.h"

void floating_above(const char *);
void dogushforth(int);
void dryup(signed char,signed char, bool);
void drinkfountain(void);
void dipfountain(struct obj *);
void breaksink(int,int);
void drinksink(void);

#endif // FOUNTAIN_H
