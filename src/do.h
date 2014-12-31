#ifndef DO_H
#define DO_H

int dodrop(void);
bool boulder_hits_pool(struct obj *,int,int,bool);
bool flooreffects(struct obj *,int,int,const char *);
void doaltarobj(struct obj *);
bool canletgo(struct obj *,const char *);
void dropx(struct obj *);
void dropy(struct obj *);
void obj_no_longer_held(struct obj *);
int doddrop(void);
int dodown(void);
int doup(void);
void save_currentstate(void);
void goto_level(d_level *,bool,bool,bool);
void schedule_goto(d_level *,bool,bool,int, const char *,const char *);
void deferred_goto(void);
bool revive_corpse(struct obj *);
void revive_mon(void *, long);
int donull(void);
int dowipe(void);
void set_wounded_legs(long,int);
void heal_legs(void);

#endif // DO_H
