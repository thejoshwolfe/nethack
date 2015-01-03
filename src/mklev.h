#ifndef MKLEV_H
#define MKLEV_H

#include "mkroom.h"
#include "dungeon.h"

void sort_rooms(void);
void add_room(int,int,int,int,bool,signed char,bool);
void add_subroom(struct mkroom *,int,int,int,int, bool,signed char,bool);
void makecorridors(void);
void add_door(int,int,struct mkroom *);
void mklev(void);
void topologize(struct mkroom *);
void place_branch(branch *,signed char,signed char);
bool occupied(signed char,signed char);
int okdoor(signed char,signed char);
void dodoor(int,int,struct mkroom *);
void mktrap(int,int,struct mkroom *,coord*);
void mkstairs(signed char,signed char,char,struct mkroom *);
void mkinvokearea(void);

#endif // MKLEV_H
