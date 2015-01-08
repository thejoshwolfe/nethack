#ifndef APPLY_H
#define APPLY_H

#include <stdbool.h>

#include "monst.h"

int doapply(void);
int dorub(void);
int dojump(void);
int jump(int);
int number_leashed(void);
void o_unleash(struct obj *);
void m_unleash(struct monst *,bool);
void unleash_all(void);
bool next_to_u(void);
struct obj *get_mleash(struct monst *);
void check_leash(signed char,signed char);
bool um_dist(signed char,signed char,signed char);
bool snuff_candle(struct obj *);
bool snuff_lit(struct obj *);
bool catch_lit(struct obj *);
void use_unicorn_horn(struct obj *);
bool tinnable(struct obj *);
void reset_trapset(void);
void fig_transform(void *, long);
int unfixable_trouble_count(bool);

#endif
