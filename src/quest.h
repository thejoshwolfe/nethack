/* See LICENSE in the root of this project for change info */

#ifndef QUEST_H
#define QUEST_H

#include <stdbool.h>

#include "monst.h"

struct q_score {                        /* Quest "scorecard" */
    unsigned first_start:1;        /* only set the first time */
    unsigned met_leader:1;         /* has met the leader */
    unsigned not_ready:3;          /* rejected due to alignment, etc. */
    unsigned pissed_off:1;         /* got the leader angry */
    unsigned got_quest:1;          /* got the quest assignment */

    unsigned first_locate:1;       /* only set the first time */
    unsigned met_intermed:1;       /* used if the locate is a person. */
    unsigned got_final:1;          /* got the final quest assignment */

    unsigned made_goal:3;          /* # of times on goal level */
    unsigned met_nemesis:1;        /* has met the nemesis before */
    unsigned killed_nemesis:1;     /* set when the nemesis is killed */
    unsigned in_battle:1;          /* set when nemesis fighting you */

    unsigned cheater:1;            /* set if cheating detected */
    unsigned touched_artifact:1;   /* for a special message */
    unsigned offered_artifact:1;   /* offered to leader */
    unsigned got_thanks:1;         /* final message from leader */

    /* keep track of leader presence/absence even if leader is
       polymorphed, raised from dead, etc */
    unsigned leader_is_dead:1;
    unsigned leader_m_id;
};

enum {
    MAX_QUEST_TRIES = 7,      /* exceed this and you "fail" */
    MIN_QUEST_ALIGN = 20,      /* at least this align.record to start */
  /* note: align 20 matches "pious" as reported by enlightenment (cmd.c) */
    MIN_QUEST_LEVEL = 14,      /* at least this u.ulevel to start */
  /* note: exp.lev. 14 is threshold level for 5th rank (class title, role.c) */
};


void onquest(void);
void nemdead(void);
void artitouch(void);
bool ok_to_quest(void);
void leader_speaks(struct monst *);
void nemesis_speaks(void);
void quest_chat(struct monst *);
void quest_talk(struct monst *);
void quest_stat_check(struct monst *);
void finish_quest(struct obj *);

#endif /* QUEST_H */
