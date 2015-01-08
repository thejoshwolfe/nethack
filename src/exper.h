#ifndef EXPER_H
#define EXPER_H

#include <stdbool.h>

#include "monst.h"

int experience(struct monst *,int);
void more_experienced(int,int);
void losexp(const char *);
void newexplevel(void);
void pluslvl(bool);
long rndexp(bool);

#endif // EXPER_H
