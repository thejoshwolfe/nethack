/* See LICENSE in the root of this project for change info */
#ifndef TIMEOUT_H
#define TIMEOUT_H

#include <stdbool.h>

#include "obj.h"

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

void burn_away_slime(void);
void nh_timeout(void);
void fall_asleep(int, bool);
void attach_egg_hatch_timeout(struct obj *);
void attach_fig_transform_timeout(struct obj *);
void kill_egg(struct obj *);
void hatch_egg(void *, long);
void learn_egg_type(int);
void burn_object(void *, long);
void begin_burn(struct obj *, bool);
void end_burn(struct obj *, bool);
void do_storms(void);
bool start_timer(long, short, short, void *);
long stop_timer(short, void *);
void run_timers(void);
void obj_move_timers(struct obj *, struct obj *);
void obj_split_timers(struct obj *, struct obj *);
void obj_stop_timers(struct obj *);
bool obj_is_local(struct obj *);
void save_timers(int,int,int);
void restore_timers(int,int,bool,long);
void relink_timers(bool);
int wiz_timeout_queue(void);
void timer_sanity_check(void);
void stop_occupation(void);

#endif /* TIMEOUT_H */
