#ifndef OBJNAM_H
#define OBJNAM_H

#include "obj.h"

#include <stdbool.h>
#include <stdlib.h>

char *obj_typename(int);
char *simple_typename(int);
bool obj_is_pname(struct obj *);
char *distant_name(struct obj *,char *(*)(struct obj *));
char *fruitname(bool);
char *xname(struct obj *);
char *mshot_xname(struct obj *);
bool the_unique_obj(struct obj *obj);
char *doname(struct obj *);
bool not_fully_identified(struct obj *);
char *corpse_xname(struct obj *,bool);
char *cxname(struct obj *);
char *cxname2(struct obj *);
char *killer_xname(struct obj *);
const char *singular(struct obj *,char *(*)(struct obj *));
const char *an_prefix(const char *str);
char *an(const char *);
char *An(const char *);
char *The(const char *);
char *the(const char *);
char *aobjnam(struct obj *,const char *);
char *Tobjnam(struct obj *,const char *);
size_t otense (char *out_buf, size_t buf_size, const struct obj *otmp, const char *verb);
size_t vtense (char *out_buf, size_t buf_size, const char *subj, const char *verb);
char *Doname2(struct obj *);
char *yname(struct obj *);
char *Yname2(struct obj *);
char *ysimple_name(struct obj *);
char *Ysimple_name2(struct obj *);
char *makeplural(const char *);
char *makesingular(const char *);
struct obj *readobjnam(char *,struct obj *,bool);
int rnd_class(int,int);
const char *cloak_simple_name(struct obj *);
const char *mimic_obj_name(struct monst *);

#endif // OBJNAM_H
