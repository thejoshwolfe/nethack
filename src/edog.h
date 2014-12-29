/* See LICENSE in the root of this project for change info */

#ifndef EDOG_H
#define EDOG_H

#include "coord.h"
#include "monst.h"

/*      various types of food, the lower, the better liked.     */

enum {
    DOGFOOD = 0,
    CADAVER = 1,
    ACCFOOD = 2,
    MANFOOD = 3,
    APPORT  = 4,
    POISON  = 5,
    UNDEF   = 6,
    TABU    = 7,
};

struct edog {
    long droptime;                  /* moment dog dropped object */
    unsigned dropdist;              /* dist of drpped obj from @ */
    int apport;                     /* amount of training */
    long whistletime;               /* last time he whistled */
    long hungrytime;                /* will get hungry at this time */
    coord ogoal;                    /* previous goal location */
    int abuse;                      /* track abuses to this pet */
    int revivals;                   /* count pet deaths */
    int mhpmax_penalty;             /* while starving, points reduced */
    unsigned killed_by_u: 1;       /* you attempted to kill him */
};

static struct edog * EDOG(struct monst *mon) {
    return (struct edog *) &mon->mextra[0];
}

#endif /* EDOG_H */
