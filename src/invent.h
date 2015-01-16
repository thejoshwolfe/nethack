#ifndef INVENT_H
#define INVENT_H

#include <stdbool.h>

#include "obj.h"

void assigninvlet(struct obj *);
struct obj *merge_choice(struct obj *,struct obj *);
int merged(struct obj **,struct obj **);
void addinv_core1(struct obj *);
void addinv_core2(struct obj *);
struct obj *addinv(struct obj *);
struct obj *hold_another_object(struct obj *,const char *,const char *,const char *);
void useupall(struct obj *);
void useup(struct obj *);
void consume_obj_charge(struct obj *,bool);
void freeinv_core(struct obj *);
void freeinv(struct obj *);
void delallobj(int,int);
void delobj(struct obj *);
struct obj *sobj_at(int,int,int);
struct obj *carrying(int);
bool have_lizard(void);
struct obj *o_on(unsigned int,struct obj *);
bool obj_here(struct obj *,int,int);
bool wearing_armor(void);
bool is_worn(const struct obj *);
struct obj *g_at(int,int);
struct obj *mkgoldobj(long);
struct obj *getobj(const char *,const char *);
int ggetobj(const char *,int (*)(struct obj *),int,bool,unsigned *);
void fully_identify_obj(struct obj *);
int identify(struct obj *);
void identify_pack(int);
int askchain(struct obj **,const char *,int,int (*)(struct obj *),
        int (*)(const struct obj *),int,const char *);
void prinv(const char *,struct obj *,long);
char *xprname(struct obj *,const char *,char,bool,long,long);
int ddoinv(void);
char display_inventory(const char *,bool);
char dump_inventory(const char *,bool);
int display_binventory(int,int,bool);
struct obj *display_cinventory(struct obj *);
struct obj *display_minventory(struct monst *,int,char *);
int dotypeinv(void);
const char *dfeature_at(int,int,char *);
int look_here(int);
int dolook(void);
bool will_feel_cockatrice(struct obj *,bool);
void feel_cockatrice(struct obj *,bool);
void stackobj(struct obj *);
int doprgold(void);
int doprwep(void);
int doprarm(void);
int doprring(void);
int dopramulet(void);
int doprtool(void);
int doprinuse(void);
void useupf(struct obj *,long);
char *let_to_name(char,bool);
void free_invbuf(void);
void reassign(void);
int doorganize(void);
int count_unpaid(struct obj *);
int count_buc(struct obj *,int);
void carry_obj_effects(struct obj *);
const char *currency(long);
void silly_thing(const char *,struct obj *);
int dopickup(void);

#endif // INVENT_H
