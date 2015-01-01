#ifndef WEAPON_H
#define WEAPON_H

#include <stdbool.h>
#include "obj.h"
#include "monst.h"
#include "skills.h"

int hitval(struct obj *,struct monst *);
int dmgval(struct obj *,struct monst *);
struct obj *select_rwep(struct monst *);
struct obj *select_hwep(struct monst *);
void possibly_unwield(struct monst *,bool);
int mon_wield_item(struct monst *);
int abon(void);
int dbon(void);
int enhance_weapon_skill(void);
void dump_weapon_skill(void);
void unrestrict_weapon_skill(int);
void use_skill(int,int);
void add_weapon_skill(int);
void lose_weapon_skill(int);
int weapon_type(struct obj *);
int uwep_skill_type(void);
int weapon_hit_bonus(struct obj *);
int weapon_dam_bonus(struct obj *);
void skill_init(const struct def_skill *);

#endif // WEAPON_H
