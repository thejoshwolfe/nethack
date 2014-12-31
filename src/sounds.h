#ifndef SOUNDS_H
#define SOUNDS_H

void dosounds(void);
const char *growl_sound(struct monst *);
void growl(struct monst *);
void yelp(struct monst *);
void whimper(struct monst *);
void beg(struct monst *);
int dotalk(void);

#endif
