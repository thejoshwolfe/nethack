#ifndef HACKLIB_H
#define HACKLIB_H

#include "monst.h"

#include <stdbool.h>
#include <stdlib.h>

bool digit(char);
bool letter(char);
char highc(char);
char lowc(char);
char *lcase(char *);
char *upstart(char *);
char *mungspaces(char *);
char *eos(char *);
const char * const_eos(const char *s);
char *strkitten(char *,char);
size_t s_suffix(char *dest, size_t dest_size, const char *s);
const char *possessive_suffix(const char *s);
size_t monster_possessive(char *dest, size_t dest_size, const struct monst *mon);
size_t monster_possessive_cap(char *dest, size_t dest_size, const struct monst *mon);
char *xcrypt(const char *,char *);
bool onlyspace(const char *);
char *tabexpand(char *);
char *visctrl(char);
const char *ordin(int);
char *sitoa(int);
int sgn(int);
int rounddiv(long,int);
int dist2(int,int,int,int);
int distmin(int,int,int,int);
bool online2(int,int,int,int);
bool pmatch(const char *,const char *);
int strncmpi(const char *,const char *,int);
char *strstri(const char *,const char *);
bool fuzzymatch(const char *,const char *,const char *,bool);
void setrandom(void);
int getyear(void);
long yyyymmdd(time_t);
int phase_of_the_moon(void);
bool friday_13th(void);
int night(void);
int midnight(void);

#endif
