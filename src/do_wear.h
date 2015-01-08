#ifndef DO_WEAR_H
#define DO_WEAR_H

#include <stdbool.h>

#include "obj.h"

void off_msg(struct obj *);
void set_wear(void);
bool donning(struct obj *);
void cancel_don(void);
int Armor_off(void);
int Armor_gone(void);
int Helmet_off(void);
int Gloves_off(void);
int Boots_off(void);
int Cloak_off(void);
int Shield_off(void);
int Shirt_off(void);
void Amulet_off(void);
void Ring_on(struct obj *);
void Ring_off(struct obj *);
void Ring_gone(struct obj *);
void Blindf_on(struct obj *);
void Blindf_off(struct obj *);
int dotakeoff(void);
int doremring(void);
int cursed(struct obj *);
int armoroff(struct obj *);
int canwearobj(struct obj *, long *, bool);
int dowear(void);
int doputon(void);
void find_ac(void);
void glibr(void);
struct obj *some_armor(struct monst *);
void erode_armor(struct monst *,bool);
struct obj *stuck_ring(struct obj *,int);
struct obj *unchanger(void);
void reset_remarm(void);
int doddoremarm(void);
int destroy_arm(struct obj *);
void adj_abon(struct obj *,signed char);

#endif
