#ifndef MON_H
#define MON_H

#include "monst.h"
#include <stdbool.h>
#include "obj.h"

int undead_to_corpse(int);
int genus(int,int);
int pm_to_cham(int);
int minliquid(struct monst *);
int movemon(void);
int meatmetal(struct monst *);
int meatobj(struct monst *);
void mpickgold(struct monst *);
bool mpickstuff(struct monst *,const char *);
int curr_mon_load(struct monst *);
int max_mon_load(struct monst *);
bool can_carry(struct monst *,struct obj *);
int mfndpos(struct monst *,coord *,long *,long);
bool monnear(struct monst *,int,int);
void dmonsfree(void);
int mcalcmove(struct monst*);
void mcalcdistress(void);
void replmon(struct monst *,struct monst *);
void relmon(struct monst *);
struct obj *mlifesaver(struct monst *);
bool corpse_chance(struct monst *,struct monst *,bool);
void mondead(struct monst *);
void mondied(struct monst *);
void mongone(struct monst *);
void monstone(struct monst *);
void monkilled(struct monst *,const char *,int);
void unstuck(struct monst *);
void killed(struct monst *);
void xkilled(struct monst *,int);
void mon_to_stone(struct monst*);
void mnexto(struct monst *);
bool mnearto(struct monst *,signed char,signed char,bool);
void poisontell(int);
void poisoned(const char *,int,const char *,int);
void m_respond(struct monst *);
void setmangry(struct monst *);
void wakeup(struct monst *);
void wake_nearby(void);
void wake_nearto(int,int,int);
void seemimic(struct monst *);
void rescham(void);
void restartcham(void);
void restore_cham(struct monst *);
void mon_animal_list(bool);
int newcham(struct monst *,struct permonst *,bool,bool);
int can_be_hatched(int);
int egg_type_from_parent(int,bool);
bool dead_species(int,bool);
void kill_genocided_monsters(void);
void golemeffects(struct monst *,int,int);
bool angry_guards(bool);
void pacify_guards(void);

#endif
