#ifndef DETECT_H
#define DETECT_H

#include "dungeon.h"
#include "rm.h"
#include "trap.h"

struct obj *o_in(struct obj*,char);
struct obj *o_material(struct obj*,unsigned);
int gold_detect(struct obj *);
int food_detect(struct obj *);
int object_detect(struct obj *,int);
int monster_detect(struct obj *,int);
int trap_detect(struct obj *);
const char *level_distance(d_level *);
void use_crystal_ball(struct obj *);
void do_mapping(void);
void do_vicinity_map(void);
void cvt_sdoor_to_door(struct rm *);
int findit(void);
int openit(void);
void find_trap(struct trap *);
int dosearch0(int);
int dosearch(void);
void sokoban_detect(void);

#endif // DETECT_H
