/* See LICENSE in the root of this project for change info */
#ifndef TIMEOUT_H
#define TIMEOUT_H

/* generic timeout function */
typedef void (*timeout_proc)(void *, long);

/* kind of timer */
enum {
    TIMER_LEVEL   = 0,       /* event specific to level */
    TIMER_GLOBAL  = 1,       /* event follows current play */
    TIMER_OBJECT  = 2,       /* event follows a object */
    TIMER_MONSTER = 3,       /* event follows a monster */
};

/* save/restore timer ranges */
enum {
    RANGE_LEVEL  = 0,          /* save/restore timers staying on level */
    RANGE_GLOBAL = 1,          /* save/restore timers following global play */
};

/*
 * Timeout functions.  Add a define here, then put it in the table
 * in timeout.c.  "One more level of indirection will fix everything."
 */
enum {
    ROT_ORGANIC    = 0,       /* for buried organics */
    ROT_CORPSE     = 1,
    REVIVE_MON     = 2,
    BURN_OBJECT    = 3,
    HATCH_EGG      = 4,
    FIG_TRANSFORM  = 5,
    NUM_TIME_FUNCS = 6,
};

/* used in timeout.c */
typedef struct fe {
    struct fe *next;            /* next item in chain */
    long timeout;               /* when we time out */
    unsigned long tid;          /* timer ID */
    short kind;                 /* kind of use */
    short func_index;           /* what to call when we time out */
    void * arg;         /* pointer to timeout argument */
    unsigned needs_fixup:1;   /* does arg need to be patched? */
} timer_element;

#endif /* TIMEOUT_H */
