#ifndef POLYSELF_H
#define POLYSELF_H

#include <stdbool.h>
#include "monst.h"

void set_uasmon(void);
void change_sex(void);
void polyself(bool);
int polymon(int);
void rehumanize(void);
int dobreathe(void);
int dospit(void);
int doremove(void);
int dospinweb(void);
int dosummon(void);
int dogaze(void);
int dohide(void);
int domindblast(void);
void skinback(bool);
const char *mbodypart(struct monst *,int);
const char *body_part(int);
int poly_gender(void);
void ugolemeffects(int,int);

#endif
