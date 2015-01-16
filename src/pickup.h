#ifndef PICKUP_H
#define PICKUP_H

#include <stdbool.h>

#include "obj.h"

int collect_obj_classes(char *,struct obj *,bool,bool,bool (*)(const struct obj *), int *);
void add_valid_menu_class(int);
bool allow_all(const struct obj *);
bool allow_category(const struct obj *);
bool is_worn_by_type(const struct obj *);
void notice_stuff_here(void);
int pickup(void);
int pickup_object(struct obj *, long, bool);
struct obj *pick_obj(struct obj *);
int encumber_msg(void);
int doloot(void);
int use_container(struct obj *,int);
int loot_mon(struct monst *);
const char *safe_qbuf(const char *,unsigned, const char *,const char *,const char *);

#endif // PICKUP_H
