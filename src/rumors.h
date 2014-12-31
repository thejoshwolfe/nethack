#ifndef RUMORS_H
#define RUMORS_H

#include "monst.h"

#include <stdbool.h>

char *getrumor(int,char *, bool);
void outrumor(int,int);
void outoracle(bool, bool);
void save_oracles(int,int);
void restore_oracles(int);
int doconsult(struct monst *);

#endif // RUMORS_H
