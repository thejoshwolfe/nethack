#ifndef TRACK_H
#define TRACK_H

#include "coord.h"

void clear_footprints(void);
void add_footprint(void);
coord *get_footprint_near(int,int);

#endif // TRACK_H
