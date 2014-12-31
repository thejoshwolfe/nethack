#ifndef DOG_H
#define DOG_H

#include "monst.h"
#include "obj.h"
#include <stdbool.h>

void initedog(struct monst *);
struct monst *make_familiar(struct obj *,signed char,signed char,bool);
struct monst *makedog(void);
void update_mlstmv(void);
void losedogs(void);
void mon_arrive(struct monst *,bool);
void mon_catchup_elapsed_time(struct monst *,long);
void keepdogs(bool);
void migrate_to_level(struct monst *,signed char,signed char,coord *);
int dogfood(struct monst *,struct obj *);
struct monst *tamedog(struct monst *,struct obj *);
void abuse_dog(struct monst *);
void wary_dog(struct monst *, bool);

#endif
