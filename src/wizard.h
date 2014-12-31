#ifndef WIZARD_H
#define WIZARD_H

void amulet(void);
bool mon_has_amulet(const struct monst *);
int mon_has_special(struct monst *);
int tactics(struct monst *);
void aggravate(void);
void clonewiz(void);
int pick_nasty(void);
int nasty(struct monst*);
void resurrect(void);
void intervene(void);
void wizdead(void);
void cuss(struct monst *);

#endif // WIZARD_H
