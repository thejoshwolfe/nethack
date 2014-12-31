/* See LICENSE in the root of this project for change info */

#ifndef EXTERN_H
#define EXTERN_H

#include "obj.h"
#include "trap.h"
#include "monst.h"
#include "global.h"
#include "rm.h"
#include "permonst.h"
#include "skills.h"
#include "engrave.h"
#include "mkroom.h"
#include "rect.h"
#include "region.h"

/* ### exper.c ### */

extern int experience(struct monst *,int);
extern void more_experienced(int,int);
extern void losexp(const char *);
extern void newexplevel(void);
extern void pluslvl(bool);
extern long rndexp(bool);

/* ### fountain.c ### */

extern void floating_above(const char *);
extern void dogushforth(int);
extern void dryup(signed char,signed char, bool);
extern void drinkfountain(void);
extern void dipfountain(struct obj *);
extern void breaksink(int,int);
extern void drinksink(void);

/* ### mapglyph.c ### */

extern void mapglyph(int, int *, int *, unsigned *, int, int);

/* ### mcastu.c ### */

extern int castmu(struct monst *,struct attack *,bool,bool);
extern int buzzmu(struct monst *,struct attack *);

/* ### mhitu.c ### */

extern const char *mpoisons_subj(struct monst *,struct attack *);
extern void u_slow_down(void);
extern struct monst *cloneu(void);
extern void expels(struct monst *,struct permonst *,bool);
extern struct attack *getmattk(struct permonst *,int,int *,struct attack *);
extern int mattacku(struct monst *);
extern int magic_negation(struct monst *);
extern int gazemu(struct monst *,struct attack *);
extern void mdamageu(struct monst *,int);
extern int could_seduce(struct monst *,struct monst *,struct attack *);
extern int doseduce(struct monst *);

/* ### minion.c ### */

extern void msummon(struct monst *);
extern void summon_minion(aligntyp,bool);
extern int demon_talk(struct monst *);
extern long bribe(struct monst *);
extern int dprince(aligntyp);
extern int dlord(aligntyp);
extern int llord(void);
extern int ndemon(aligntyp);
extern int lminion(void);

/* ### mkmap.c ### */

void flood_fill_rm(int,int,int,bool,bool);
void remove_rooms(int,int,int,int);

/* ### mplayer.c ### */

extern struct monst *mk_mplayer(struct permonst *,signed char, signed char,bool);
extern void create_mplayers(int,bool);
extern void mplayer_talk(struct monst *);

/* ### mthrowu.c ### */

extern int thitu(int,int,struct obj *,const char *);
extern int ohitmon(struct monst *,struct obj *,int,bool);
extern void thrwmu(struct monst *);
extern int spitmu(struct monst *,struct attack *);
extern int breamu(struct monst *,struct attack *);
extern bool linedup(signed char,signed char,signed char,signed char);
extern bool lined_up(struct monst *);
extern struct obj *m_carrying(struct monst *,int);
extern void m_useup(struct monst *,struct obj *);
extern void m_throw(struct monst *,int,int,int,int,int,struct obj *);
extern bool hits_bars(struct obj **,int,int,int,int);

/* ### objects.c ### */

extern void objects_init(void);

/* ### pager.c ### */

extern int dowhatis(void);
extern int doquickwhatis(void);
extern int doidtrap(void);
extern int dowhatdoes(void);
extern char *dowhatdoes_core(char, char *);
extern int dohelp(void);
extern int dohistory(void);

/* ### pray.c ### */

extern int dosacrifice(void);
extern bool can_pray(bool);
extern int dopray(void);
extern const char *u_gname(void);
extern int doturn(void);
extern const char *a_gname(void);
extern const char *a_gname_at(signed char x,signed char y);
extern const char *align_gname(aligntyp);
extern const char *halu_gname(aligntyp);
extern const char *align_gtitle(aligntyp);
extern void altar_wrath(int,int);


/* ### quest.c ### */

extern void onquest(void);
extern void nemdead(void);
extern void artitouch(void);
extern bool ok_to_quest(void);
extern void leader_speaks(struct monst *);
extern void nemesis_speaks(void);
extern void quest_chat(struct monst *);
extern void quest_talk(struct monst *);
extern void quest_stat_check(struct monst *);
extern void finish_quest(struct obj *);

/* ### rect.c ### */

extern void init_rect(void);
extern NhRect *get_rect(NhRect *);
extern NhRect *rnd_rect(void);
extern void remove_rect(NhRect *);
extern void add_rect(NhRect *);
extern void split_rects(NhRect *,NhRect *);

/* ### rip.c ### */

extern void genl_outrip(winid,int);

/* ### rumors.c ### */

extern char *getrumor(int,char *, bool);
extern void outrumor(int,int);
extern void outoracle(bool, bool);
extern void save_oracles(int,int);
extern void restore_oracles(int);
extern int doconsult(struct monst *);

/* ### save.c ### */

extern int dosave(void);
extern void hangup(int);
extern int dosave0(void);
extern void savestateinlock(void);
extern void savelev(int,signed char,int);
extern void bufon(int);
extern void bufoff(int);
extern void bflush(int);
extern void bwrite(int,void *,unsigned int);
extern void bclose(int);
extern void savefruitchn(int,int);
extern void free_dungeons(void);
extern void freedynamicdata(void);

/* ### shknam.c ### */

extern void stock_room(int,struct mkroom *);
extern bool saleable(struct monst *,struct obj *);
extern int get_shop_item(int);

/* ### sit.c ### */

extern void take_gold(void);
extern int dosit(void);
extern void rndcurse(void);
extern void attrcurse(void);

/* ### sp_lev.c ### */

extern bool check_room(signed char *,signed char *,signed char *,signed char *,bool);
extern bool create_room(signed char,signed char,signed char,signed char,
        signed char,signed char,signed char,signed char);
extern void create_secret_door(struct mkroom *,signed char);
extern bool dig_corridor(coord *,coord *,bool,signed char,signed char);
extern void fill_room(struct mkroom *,bool);
extern bool load_special(const char *);

/* ### topten.c ### */

extern void topten(int);
extern void prscore(int,char **);
extern struct obj *tt_oname(struct obj *);

/* ### version.c ### */

extern char *version_string(char *);
extern char *getversionstring(char *);
extern int doversion(void);
extern int doextversion(void);
extern bool check_version(struct version_info *, const char *,bool);
extern unsigned long get_feature_notice_ver(char *);
extern unsigned long get_current_feature_ver(void);

#endif /* EXTERN_H */
