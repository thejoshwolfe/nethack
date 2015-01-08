#ifndef MUSE_H
#define MUSE_H

#include <stdbool.h>

#include "monst.h"
#include "obj.h"

bool find_defensive(struct monst *);
int use_defensive(struct monst *);
int rnd_defensive_item(struct monst *);
bool find_offensive(struct monst *);
int use_offensive(struct monst *);
int rnd_offensive_item(struct monst *);
bool find_misc(struct monst *);
int use_misc(struct monst *);
int rnd_misc_item(struct monst *);
bool searches_for_item(struct monst *,struct obj *);
bool mon_reflects(struct monst *,const char *);
bool ureflects(const char *,const char *);
bool munstone(struct monst *,bool);

#endif // MUSE_H
