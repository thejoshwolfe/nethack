/* See LICENSE in the root of this project for change info */
#ifndef ESHK_H
#define ESHK_H

#include "global.h"
#include "coord.h"
#include "dungeon.h"

#define REPAIR_DELAY    5       /* minimum delay between shop damage & repair */

#define BILLSZ  200

struct bill_x {
        unsigned bo_id;
        boolean useup;
        long price;             /* price per unit */
        long bquan;             /* amount used up */
};

struct eshk {
        long robbed;            /* amount stolen by most recent customer */
        long credit;            /* amount credited to customer */
        long debit;             /* amount of debt for using unpaid items */
        long loan;              /* shop-gold picked (part of debit) */
        int shoptype;           /* the value of rooms[shoproom].rtype */
        signed char shoproom;           /* index in rooms; set by inshop() */
        signed char unused;             /* to force alignment for stupid compilers */
        boolean following;      /* following customer since he owes us sth */
        boolean surcharge;      /* angry shk inflates prices */
        coord shk;              /* usual position shopkeeper */
        coord shd;              /* position shop door */
        d_level shoplevel;      /* level (& dungeon) of his shop */
        int billct;             /* no. of entries of bill[] in use */
        struct bill_x bill[BILLSZ];
        struct bill_x *bill_p;
        int visitct;            /* nr of visits by most recent customer */
        char customer[PL_NSIZ]; /* most recent customer */
        char shknam[PL_NSIZ];
};

#define ESHK(mon)       ((struct eshk *)&(mon)->mextra[0])

#define NOTANGRY(mon)   ((mon)->mpeaceful)
#define ANGRY(mon)      (!NOTANGRY(mon))

#endif /* ESHK_H */
