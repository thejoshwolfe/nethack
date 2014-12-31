#ifndef LOCK_H
#define LOCK_H

bool picking_lock(int *,int *);
bool picking_at(int,int);
void reset_pick(void);
int pick_lock(struct obj *);
int doforce(void);
bool boxlock(struct obj *,struct obj *);
bool doorlock(struct obj *,int,int);
int doopen(void);
int doclose(void);

#endif // LOCK_H
