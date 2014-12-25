/* See LICENSE in the root of this project for change info */
#ifndef EMIN_H
#define EMIN_H

struct emin {
        aligntyp min_align;     /* alignment of minion */
};

#define EMIN(mon)       ((struct emin *)&(mon)->mextra[0])

#endif /* EMIN_H */
