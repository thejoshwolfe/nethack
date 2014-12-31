#ifndef MKOBJ_H
#define MKOBJ_H

#include <stdbool.h>
#include "obj.h"
#include "monst.h"

struct obj *mkobj_at(char,int,int,bool);
struct obj *mksobj_at(int,int,int,bool,bool);
struct obj *mkobj(char,bool);
int rndmonnum(void);
struct obj *splitobj(struct obj *,long);
void replace_object(struct obj *,struct obj *);
void bill_dummy_object(struct obj *);
struct obj *mksobj(int,bool,bool);
int bcsign(struct obj *);
int weight(struct obj *);
struct obj *mkgold(long,int,int);
struct obj *mkcorpstat(int,struct monst *,struct permonst *,int,int,bool);
struct obj *obj_attach_mid(struct obj *, unsigned);
struct monst *get_mtraits(struct obj *, bool);
struct obj *mk_tt_object(int,int,int);
struct obj *mk_named_object(int,struct permonst *,int,int,const char *);
struct obj *rnd_treefruit_at(int, int);
void start_corpse_timeout(struct obj *);
void bless(struct obj *);
void unbless(struct obj *);
void curse(struct obj *);
void uncurse(struct obj *);
void blessorcurse(struct obj *,int);
bool is_flammable(struct obj *);
bool is_rottable(struct obj *);
void place_object(struct obj *,int,int);
void remove_object(struct obj *);
void discard_minvent(struct monst *);
void obj_extract_self(struct obj *);
void extract_nobj(struct obj *, struct obj **);
void extract_nexthere(struct obj *, struct obj **);
int add_to_minv(struct monst *, struct obj *);
struct obj *add_to_container(struct obj *, struct obj *);
void add_to_migration(struct obj *);
void add_to_buried(struct obj *);
void dealloc_obj(struct obj *);
void obj_ice_effects(int, int, bool);
long peek_at_iced_corpse_age(struct obj *);
void obj_sanity_check(void);

#endif // MKOBJ_H
