#ifndef TELEPORT_H
#define TELEPORT_H

#include <stdbool.h>

#include "coord.h"
#include "monst.h"
#include "trap.h"

bool goodpos(int,int,struct monst *,unsigned);
bool enexto(coord *,signed char,signed char,struct permonst *);
bool enexto_core(coord *,signed char,signed char,struct permonst *,unsigned);
void teleds(int,int,bool);
bool safe_teleds(bool);
bool teleport_pet(struct monst *,bool);
void tele(void);
int dotele(void);
void level_tele(void);
void domagicportal(struct trap *);
void tele_trap(struct trap *);
void level_tele_trap(struct trap *);
void rloc_to(struct monst *,int,int);
bool rloc(struct monst *, bool);
bool tele_restrict(struct monst *);
void mtele_trap(struct monst *, struct trap *,int);
int mlevel_tele_trap(struct monst *, struct trap *,bool,int);
void rloco(struct obj *);
int random_teleport_level(void);
bool u_teleport_mon(struct monst *,bool);

#endif // TELEPORT_H
