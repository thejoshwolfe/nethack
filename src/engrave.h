/* See LICENSE in the root of this project for change info */

#ifndef ENGRAVE_H
#define ENGRAVE_H

#include <stdbool.h>
#include <stdlib.h>

struct engr {
        struct engr *nxt_engr;
        char *engr_txt;
        signed char engr_x, engr_y;
        unsigned engr_lth;      /* for save & restore; not length of text */
        long engr_time;         /* moment engraving was (will be) finished */
        signed char engr_type;
#define DUST       1
#define ENGRAVE    2
#define BURN       3
#define MARK       4
#define ENGR_BLOOD 5
#define HEADSTONE  6
#define N_ENGRAVE  6
};

extern struct engr *head_engr;

#define newengr(lth) (struct engr *)malloc((unsigned)(lth) + sizeof(struct engr))
#define dealloc_engr(engr) free((void *) (engr))

char *random_engraving(char *);
void wipeout_text(char *,int,unsigned);
bool can_reach_floor(void);
const char *surface(int,int);
const char *ceiling(int,int);
struct engr *engr_at(signed char,signed char);
int sengr_at(const char *,signed char,signed char);
void u_wipe_engr(int);
void wipe_engr_at(signed char,signed char,signed char);
void read_engr_at(int,int);
void make_engr_at(int,int,const char *,long,signed char);
void del_engr_at(int,int);
int freehand(void);
int doengrave(void);
void save_engravings(int,int);
void rest_engravings(int);
void del_engr(struct engr *);
void rloc_engr(struct engr *);
void make_grave(int,int,const char *);


#endif /* ENGRAVE_H */
