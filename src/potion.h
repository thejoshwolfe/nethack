#ifndef POTION_H
#define POTION_H

#include <stdbool.h>

#include "monst.h"
#include "obj.h"

void set_itimeout(long *,long);
void incr_itimeout(long *,int);
void make_confused(long,bool);
void make_stunned(long,bool);
void make_blinded(long,bool);
void make_sick(long, const char *, bool,int);
void make_vomiting(long,bool);
bool make_hallucinated(long,bool,long);
int dodrink(void);
int dopotion(struct obj *);
int peffects(struct obj *);
void healup(int,int,bool,bool);
void strange_feeling(struct obj *,const char *);
void potionhit(struct monst *,struct obj *,bool);
void potionbreathe(struct obj *);
bool get_wet(struct obj *);
int dodip(void);
void djinni_from_bottle(struct obj *);
struct monst *split_mon(struct monst *,struct monst *);
const char *bottlename(void);

#endif // POTION_H
