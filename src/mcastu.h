#ifndef MCASTU_H
#define MCASTU_H

#include <stdbool.h>

#include "monst.h"
#include "permonst.h"

int castmu(struct monst *,struct attack *,bool,bool);
int buzzmu(struct monst *,struct attack *);

#endif // MCASTU_H
