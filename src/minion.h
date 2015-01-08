#ifndef MINION_H
#define MINION_H

#include <stdbool.h>

#include "align.h"
#include "monst.h"

void msummon(struct monst *);
void summon_minion(aligntyp,bool);
int demon_talk(struct monst *);
long bribe(struct monst *);
int dprince(aligntyp);
int dlord(aligntyp);
int llord(void);
int ndemon(aligntyp);
int lminion(void);

#endif // MINION_H
