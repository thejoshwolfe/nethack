#ifndef WORN_H
#define WORN_H

void setworn(struct obj *,long);
void setnotworn(struct obj *);
void mon_set_minvis(struct monst *);
void mon_adjust_speed(struct monst *,int,struct obj *);
void update_mon_intrinsics(struct monst *,struct obj *,bool,bool);
int find_mac(struct monst *);
void m_dowear(struct monst *,bool);
struct obj *which_armor(struct monst *,long);
void mon_break_armor(struct monst *,bool);
void bypass_obj(struct obj *);
void clear_bypasses(void);
int racial_exception(struct monst *, struct obj *);

#endif // WORN_H
