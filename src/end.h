#ifndef END_H
#define END_H

#include <stdbool.h>
#include "monst.h"

void done1(int);
int done2(void);
void done_in_by(struct monst *);
void panic(const char *, ...) __attribute__ ((format (printf, 1, 2)));
void done(int);
void container_contents(struct obj *,bool,bool);
void dump(char *, char *);
void do_containerconts(struct obj *,bool,bool,bool);
void terminate(int);
int num_genocides(void);


#endif // END_H
