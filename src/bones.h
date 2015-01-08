#ifndef BONES_H
#define BONES_H

#include <stdbool.h>

#include "obj.h"

bool can_make_bones(void);
void savebones(struct obj *);
int getbones(void);

#endif
