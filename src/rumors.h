#ifndef RUMORS_H
#define RUMORS_H

#include <stdbool.h>

#include "monst.h"

char *getrumor(int,char *, bool);
void outrumor(int,int);
void outoracle(bool, bool);
void save_oracles(int,int);
void restore_oracles(int);
int doconsult(struct monst *);

#endif // RUMORS_H
