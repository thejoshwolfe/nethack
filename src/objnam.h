#ifndef OBJNAM_H
#define OBJNAM_H

#include "obj.h"
#include "monst.h"

#include <stdbool.h>
#include <stdlib.h>

char *obj_typename(int);
char *simple_typename(int);
bool obj_is_pname(const struct obj *);
char *distant_name(const struct obj *,char *(*)(const struct obj *));
char *fruitname(bool);
char *xname(const struct obj *);
char *mshot_xname(const struct obj *);
bool the_unique_obj(const struct obj *obj);
char *doname(const struct obj *);
bool not_fully_identified(const struct obj *);
char *corpse_xname(const struct obj *,bool);
char *cxname(const struct obj *);
char *cxname2(const struct obj *);
const char *singular(const struct obj *,char *(*)(const struct obj *));
const char *an_prefix(const char *str);
char *an(const char *);
char *An(const char *);
char *The(const char *);
char *the(const char *);
char *aobjnam(const struct obj *,const char *);
size_t Tobjnam (char *out_buf, size_t buf_size, const struct obj *otmp, const char *verb);
size_t otense (char *out_buf, size_t buf_size, const struct obj *otmp, const char *verb);
size_t vtense (char *out_buf, size_t buf_size, const char *subj, const char *verb);
char *Doname2(const struct obj *);
char *yname(const struct obj *);
char *Yname2(const struct obj *);
char *ysimple_name(const struct obj *);
char *Ysimple_name2(const struct obj *);
char *makeplural(const char *);
char *makesingular(const char *);
struct obj *readobjnam(char *,const struct obj *,bool);
int rnd_class(int,int);
const char *cloak_simple_name(const struct obj *);
const char *mimic_obj_name(const struct monst *);
const char * rank_of(int lev, short monnum, bool female);
int title_to_mon (const char *str, int *rank_indx, int *title_length);
int xlev_to_rank (int xlev);

#endif // OBJNAM_H
