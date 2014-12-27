/* See LICENSE in the root of this project for change info */
#ifndef ALIGN_H
#define ALIGN_H

typedef signed char aligntyp;    /* basic alignment type */

typedef struct align {        /* alignment & record */
    aligntyp type;
    int      record;
} align;

/* bounds for "record" -- respect initial alignments of 10 */
#define ALIGNLIM (10L + (moves/200L))

enum {
    A_NONE = (-128),    /* the value range of type */

    A_CHAOTIC = (-1),
    A_NEUTRAL = 0,
    A_LAWFUL  = 1,

    A_COALIGNED =  1,
    A_OPALIGNED = (-1),
};

enum {
    AM_NONE    = 0,
    AM_CHAOTIC = 1,
    AM_NEUTRAL = 2,
    AM_LAWFUL  = 4,

    AM_MASK    = 7,

    AM_SPLEV_CO    = 3,
    AM_SPLEV_NONCO = 7,
};

#define Amask2align(x)    ((aligntyp) ((!(x)) ? A_NONE \
             : ((x) == AM_LAWFUL) ? A_LAWFUL : ((int)x) - 2))
#define Align2amask(x)    (((x) == A_NONE) ? AM_NONE \
             : ((x) == A_LAWFUL) ? AM_LAWFUL : (x) + 2)

#endif /* ALIGN_H */
