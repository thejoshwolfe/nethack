#ifndef EAT_H
#define EAT_H

#include <stdbool.h>
#include "obj.h"

bool is_edible(struct obj *);
void init_uhunger(void);
int Hear_again(void);
void reset_eat(void);
int doeat(void);
void gethungry(void);
void morehungry(int);
void lesshungry(int);
bool is_fainted(void);
void reset_faint(void);
void violated_vegetarian(void);
void newuhs(bool);
struct obj *floorfood(const char *,int);
void vomit(void);
int eaten_stat(int,struct obj *);
void food_disappears(struct obj *);
void food_substitution(struct obj *,struct obj *);
void fix_petrification(void);
void consume_oeaten(struct obj *,int);
bool maybe_finished_meal(bool);


#endif // EAT_H
