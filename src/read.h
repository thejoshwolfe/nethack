#ifndef READ_H
#define READ_H

#include <stdbool.h>

int doread(void);
bool is_chargeable(struct obj *);
void recharge(struct obj *,int);
void forget_objects(int);
void forget_levels(int);
void forget_traps(void);
void forget_map(int);
int seffects(struct obj *);
void litroom(bool,struct obj *);
void do_genocide(int);
void punish(struct obj *);
void unpunish(void);
bool cant_create(int *, bool);
bool create_particular(void);

#endif // READ_H
