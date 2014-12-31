#ifndef LIGHT_H
#define LIGHT_H

#include <stdbool.h>

void new_light_source(signed char, signed char, int, int, void *);
void del_light_source(int, void *);
void do_light_sources(char **);
struct monst *find_mid(unsigned, unsigned);
void save_light_sources(int, int, int);
void restore_light_sources(int);
void relink_light_sources(bool);
void obj_move_light_source(struct obj *, struct obj *);
bool any_light_source(void);
void snuff_light_source(int, int);
bool obj_sheds_light(struct obj *);
bool obj_is_burning(struct obj *);
void obj_split_light_source(struct obj *, struct obj *);
void obj_merge_light_sources(struct obj *,struct obj *);
int candle_light_range(struct obj *);
int wiz_light_sources(void);

#endif // LIGHT_H
