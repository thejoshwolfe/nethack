#ifndef PRIEST_H
#define PRIEST_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "align.h"
#include "dungeon.h"
#include "mkroom.h"
#include "monst.h"

int move_special(struct monst *,bool,signed char,bool,bool, signed char,signed char,signed char,signed char);
char temple_occupied(char *);
int pri_move(struct monst *);
void priestini(d_level *,struct mkroom *,int,int,bool);
size_t priestname(char *out_buf, size_t buf_size, const struct monst *mon, bool block_invis_and_halluc);
bool p_coaligned(struct monst *);
struct monst *findpriest(char);
void intemple(int);
void priest_talk(struct monst *);
struct monst *mk_roamer(struct permonst *,aligntyp, signed char,signed char,bool);
void reset_hostility(struct monst *);
bool in_your_sanctuary(struct monst *,signed char,signed char);
void ghod_hitsu(struct monst *);
void angry_priest(void);
void clearpriests(void);
void restpriest(struct monst *,bool);

#endif
