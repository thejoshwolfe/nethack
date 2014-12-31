#ifndef ZAP_H
#define ZAP_H

int bhitm(struct monst *,struct obj *);
void probe_monster(struct monst *);
bool get_obj_location(struct obj *,signed char *,signed char *,int);
bool get_mon_location(struct monst *,signed char *,signed char *,int);
struct monst *get_container_location(struct obj *obj, int *, int *);
struct monst *montraits(struct obj *,coord *);
struct monst *revive(struct obj *);
int unturn_dead(struct monst *);
void cancel_item(struct obj *);
bool drain_item(struct obj *);
struct obj *poly_obj(struct obj *, int);
bool obj_resists(struct obj *,int,int);
bool obj_shudders(struct obj *);
void do_osshock(struct obj *);
int bhito(struct obj *,struct obj *);
int bhitpile(struct obj *,int (*)(struct obj *,struct obj *),int,int);
int zappable(struct obj *);
void zapnodir(struct obj *);
int dozap(void);
int zapyourself(struct obj *,bool);
bool cancel_monst(struct monst *,struct obj *, bool,bool,bool);
void weffects(struct obj *);
int spell_damage_bonus(void);
const char *exclam(int force);
void hit(const char *,struct monst *,const char *);
void miss(const char *,struct monst *);
struct monst *bhit(int,int,int,int,int (*)(struct monst *,struct obj *),
        int (*)(struct obj *,struct obj *),struct obj *);
struct monst *boomhit(int,int);
int burn_floor_paper(int,int,bool,bool);
void buzz(int,int,signed char,signed char,int,int);
void melt_ice(signed char,signed char);
int zap_over_floor(signed char,signed char,int,bool *);
void fracture_rock(struct obj *);
bool break_statue(struct obj *);
void destroy_item(int,int);
int destroy_mitem(struct monst *,int,int);
int resist(struct monst *,char,int,int);
void makewish(void);

#endif // ZAP_H
