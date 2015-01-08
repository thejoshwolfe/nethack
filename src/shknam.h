#ifndef SHKNAM_H
#define SHKNAM_H

#include <stdbool.h>

#include "mkroom.h"
#include "monst.h"

void stock_room(int,struct mkroom *);
bool saleable(struct monst *,struct obj *);
int get_shop_item(int);

#endif // SHKNAM_H
