#ifndef PRAY_H
#define PRAY_H

#include "align.h"

#include <stdbool.h>

int dosacrifice(void);
bool can_pray(bool);
int dopray(void);
const char *u_gname(void);
int doturn(void);
const char *a_gname(void);
const char *a_gname_at(signed char x,signed char y);
const char *align_gname(aligntyp);
const char *halu_gname(aligntyp);
const char *align_gtitle(aligntyp);
void altar_wrath(int,int);

#endif // PRAY_H
