#ifndef WIELD_H
#define WIELD_H

#include <stdbool.h>

#include "obj.h"

void setuwep(struct obj *);
void setuqwep(struct obj *);
void setuswapwep(struct obj *);
int dowield(void);
int doswapweapon(void);
int dowieldquiver(void);
bool wield_tool(struct obj *,const char *);
int can_twoweapon(void);
void drop_uswapwep(void);
int dotwoweapon(void);
void uwepgone(void);
void uswapwepgone(void);
void uqwepgone(void);
void untwoweapon(void);
void erode_obj(struct obj *,bool,bool);
int chwepon(struct obj *,int);
int welded(struct obj *);
void weldmsg(struct obj *);

#endif // WIELD_H
