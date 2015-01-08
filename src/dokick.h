#ifndef DOKICK_H
#define DOKICK_H

#include <stdbool.h>

#include "obj.h"

bool ghitm(struct monst *,struct obj *);
void container_impact_dmg(struct obj *);
int dokick(void);
bool ship_object(struct obj *,signed char,signed char,bool);
void obj_delivery(void);
signed char down_gate(signed char,signed char);
void impact_drop(struct obj *,signed char,signed char,signed char);


#endif // DOKICK_H
