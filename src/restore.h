#ifndef RESTORE_H
#define RESTORE_H

#include <stdbool.h>

void inven_inuse(bool);
int dorecover(int);
void trickery(char *);
void getlev(int,int,signed char,bool);
bool lookup_id_mapping(unsigned, unsigned *);
void mread(int,void *,unsigned int);

#endif // RESTORE_H
