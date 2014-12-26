#ifndef PM_PROPS_H
#define PM_PROPS_H

#include "pm.h"

#define NON_PM          PM_PLAYERMON            /* "not a monster" */
#define LOW_PM          (NON_PM+1)              /* first monster in mons[] */
#define SPECIAL_PM      PM_LONG_WORM_TAIL       /* [normal] < ~ < [special] */
        /* mons[SPECIAL_PM] through mons[NUMMONS-1], inclusive, are
           never generated randomly and cannot be polymorphed into */

#endif /* PM_PROPS_H */
