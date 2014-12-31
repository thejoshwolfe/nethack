#ifndef MPLAYER_H
#define MPLAYER_H

#include "monst.h"

#include <stdbool.h>

struct monst *mk_mplayer(struct permonst *,signed char, signed char,bool);
void create_mplayers(int,bool);
void mplayer_talk(struct monst *);

#endif // MPLAYER_H
