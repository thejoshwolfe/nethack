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

/* ### explode.c ### */

extern void explode(int,int,int,int,char,int);
extern long scatter(int, int, int, unsigned int, struct obj *);
extern void splatter_burning_oil(int, int);

/* ### fountain.c ### */

extern void floating_above(const char *);
extern void dogushforth(int);
extern void dryup(signed char,signed char, bool);
extern void drinkfountain(void);
extern void dipfountain(struct obj *);
extern void breaksink(int,int);
extern void drinksink(void);

/* ### light.c ### */

extern void new_light_source(signed char, signed char, int, int, void *);
extern void del_light_source(int, void *);
extern void do_light_sources(char **);
extern struct monst *find_mid(unsigned, unsigned);
extern void save_light_sources(int, int, int);
extern void restore_light_sources(int);
extern void relink_light_sources(bool);
extern void obj_move_light_source(struct obj *, struct obj *);
extern bool any_light_source(void);
extern void snuff_light_source(int, int);
extern bool obj_sheds_light(struct obj *);
extern bool obj_is_burning(struct obj *);
extern void obj_split_light_source(struct obj *, struct obj *);
extern void obj_merge_light_sources(struct obj *,struct obj *);
extern int candle_light_range(struct obj *);
extern int wiz_light_sources(void);

/* ### lock.c ### */

extern bool picking_lock(int *,int *);
extern bool picking_at(int,int);
extern void reset_pick(void);
extern int pick_lock(struct obj *);
extern int doforce(void);
extern bool boxlock(struct obj *,struct obj *);
extern bool doorlock(struct obj *,int,int);
extern int doopen(void);
extern int doclose(void);

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

/* ### mkobj.c ### */

extern struct obj *mkobj_at(char,int,int,bool);
extern struct obj *mksobj_at(int,int,int,bool,bool);
extern struct obj *mkobj(char,bool);
extern int rndmonnum(void);
extern struct obj *splitobj(struct obj *,long);
extern void replace_object(struct obj *,struct obj *);
extern void bill_dummy_object(struct obj *);
extern struct obj *mksobj(int,bool,bool);
extern int bcsign(struct obj *);
extern int weight(struct obj *);
extern struct obj *mkgold(long,int,int);
extern struct obj *mkcorpstat(int,struct monst *,struct permonst *,int,int,bool);
extern struct obj *obj_attach_mid(struct obj *, unsigned);
extern struct monst *get_mtraits(struct obj *, bool);
extern struct obj *mk_tt_object(int,int,int);
extern struct obj *mk_named_object(int,struct permonst *,int,int,const char *);
extern struct obj *rnd_treefruit_at(int, int);
extern void start_corpse_timeout(struct obj *);
extern void bless(struct obj *);
extern void unbless(struct obj *);
extern void curse(struct obj *);
extern void uncurse(struct obj *);
extern void blessorcurse(struct obj *,int);
extern bool is_flammable(struct obj *);
extern bool is_rottable(struct obj *);
extern void place_object(struct obj *,int,int);
extern void remove_object(struct obj *);
extern void discard_minvent(struct monst *);
extern void obj_extract_self(struct obj *);
extern void extract_nobj(struct obj *, struct obj **);
extern void extract_nexthere(struct obj *, struct obj **);
extern int add_to_minv(struct monst *, struct obj *);
extern struct obj *add_to_container(struct obj *, struct obj *);
extern void add_to_migration(struct obj *);
extern void add_to_buried(struct obj *);
extern void dealloc_obj(struct obj *);
extern void obj_ice_effects(int, int, bool);
extern long peek_at_iced_corpse_age(struct obj *);
extern void obj_sanity_check(void);

/* ### mkroom.c ### */

extern void mkroom(int);
extern void fill_zoo(struct mkroom *);
extern bool nexttodoor(int,int);
extern bool has_dnstairs(struct mkroom *);
extern bool has_upstairs(struct mkroom *);
extern int somex(struct mkroom *);
extern int somey(struct mkroom *);
extern bool inside_room(struct mkroom *,signed char,signed char);
extern bool somexy(struct mkroom *,coord *);
extern void mkundead(coord *,bool,int);
extern struct permonst *courtmon(void);
extern void save_rooms(int);
extern void rest_rooms(int);
extern struct mkroom *search_special(signed char);

/* ### monmove.c ### */

extern bool itsstuck(struct monst *);
extern bool mb_trapped(struct monst *);
extern void mon_regen(struct monst *,bool);
extern int dochugw(struct monst *);
extern bool onscary(int,int,struct monst *);
extern void monflee(struct monst *, int, bool, bool);
extern int dochug(struct monst *);
extern int m_move(struct monst *,int);
extern bool closed_door(int,int);
extern bool accessible(int,int);
extern void set_apparxy(struct monst *);
extern bool can_ooze(struct monst *);

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

/* ### muse.c ### */

extern bool find_defensive(struct monst *);
extern int use_defensive(struct monst *);
extern int rnd_defensive_item(struct monst *);
extern bool find_offensive(struct monst *);
extern int use_offensive(struct monst *);
extern int rnd_offensive_item(struct monst *);
extern bool find_misc(struct monst *);
extern int use_misc(struct monst *);
extern int rnd_misc_item(struct monst *);
extern bool searches_for_item(struct monst *,struct obj *);
extern bool mon_reflects(struct monst *,const char *);
extern bool ureflects(const char *,const char *);
extern bool munstone(struct monst *,bool);

/* ### music.c ### */

extern void awaken_soldiers(void);
extern int do_play_instrument(struct obj *);

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

/* ### potion.c ### */

extern void set_itimeout(long *,long);
extern void incr_itimeout(long *,int);
extern void make_confused(long,bool);
extern void make_stunned(long,bool);
extern void make_blinded(long,bool);
extern void make_sick(long, const char *, bool,int);
extern void make_vomiting(long,bool);
extern bool make_hallucinated(long,bool,long);
extern int dodrink(void);
extern int dopotion(struct obj *);
extern int peffects(struct obj *);
extern void healup(int,int,bool,bool);
extern void strange_feeling(struct obj *,const char *);
extern void potionhit(struct monst *,struct obj *,bool);
extern void potionbreathe(struct obj *);
extern bool get_wet(struct obj *);
extern int dodip(void);
extern void djinni_from_bottle(struct obj *);
extern struct monst *split_mon(struct monst *,struct monst *);
extern const char *bottlename(void);

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

/* ### read.c ### */

extern int doread(void);
extern bool is_chargeable(struct obj *);
extern void recharge(struct obj *,int);
extern void forget_objects(int);
extern void forget_levels(int);
extern void forget_traps(void);
extern void forget_map(int);
extern int seffects(struct obj *);
extern void litroom(bool,struct obj *);
extern void do_genocide(int);
extern void punish(struct obj *);
extern void unpunish(void);
extern bool cant_create(int *, bool);
extern bool create_particular(void);

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

/* ### steal.c ### */

extern long somegold(void);
extern void stealgold(struct monst *);
extern void remove_worn_item(struct obj *,bool);
extern int steal(struct monst *, char *);
extern int mpickobj(struct monst *,struct obj *);
extern void stealamulet(struct monst *);
extern void mdrop_special_objs(struct monst *);
extern void relobj(struct monst *,int,bool);

/* ### steed.c ### */

extern void rider_cant_reach(void);
extern bool can_saddle(struct monst *);
extern int use_saddle(struct obj *);
extern bool can_ride(struct monst *);
extern int doride(void);
extern bool mount_steed(struct monst *, bool);
extern void exercise_steed(void);
extern void kick_steed(void);
extern void dismount_steed(int);
extern void place_monster(struct monst *,int,int);

/* ### topten.c ### */

extern void topten(int);
extern void prscore(int,char **);
extern struct obj *tt_oname(struct obj *);

/* ### trap.c ### */

extern bool burnarmor(struct monst *);
extern bool rust_dmg(struct obj *,const char *,int,bool,struct monst *);
extern void grease_protect(struct obj *,const char *,struct monst *);
extern struct trap *maketrap(int,int,int);
extern void fall_through(bool);
extern struct monst *animate_statue(struct obj *,signed char,signed char,int,int *);
extern struct monst *activate_statue_trap(struct trap *,signed char,signed char,bool);
extern void dotrap(struct trap *, unsigned);
extern void seetrap(struct trap *);
extern int mintrap(struct monst *);
extern void instapetrify(const char *);
extern void minstapetrify(struct monst *,bool);
extern void selftouch(const char *);
extern void mselftouch(struct monst *,const char *,bool);
extern void float_up(void);
extern void fill_pit(int,int);
extern int float_down(long, long);
extern int fire_damage(struct obj *,bool,bool,signed char,signed char);
extern void water_damage(struct obj *,bool,bool);
extern bool drown(void);
extern void drain_en(int);
extern int dountrap(void);
extern int untrap(bool);
extern bool chest_trap(struct obj *,int,bool);
extern void deltrap(struct trap *);
extern bool delfloortrap(struct trap *);
extern struct trap *t_at(int,int);
extern void b_trapped(const char *,int);
extern bool unconscious(void);
extern bool lava_effects(void);
extern void blow_up_landmine(struct trap *);
extern int launch_obj(short,int,int,int,int,int);

/* ### uhitm.c ### */

extern void hurtmarmor(struct monst *,int);
extern bool attack_checks(struct monst *,struct obj *);
extern void check_caitiff(struct monst *);
extern signed char find_roll_to_hit(struct monst *);
extern bool attack(struct monst *);
extern bool hmon(struct monst *,struct obj *,int);
extern int damageum(struct monst *,struct attack *);
extern void missum(struct monst *,struct attack *);
extern int passive(struct monst *,bool,int,unsigned char);
extern void passive_obj(struct monst *,struct obj *,struct attack *);
extern void stumble_onto_mimic(struct monst *);
extern int flash_hits_mon(struct monst *,struct obj *);


/* ### version.c ### */

extern char *version_string(char *);
extern char *getversionstring(char *);
extern int doversion(void);
extern int doextversion(void);
extern bool check_version(struct version_info *, const char *,bool);
extern unsigned long get_feature_notice_ver(char *);
extern unsigned long get_current_feature_ver(void);

/* ### weapon.c ### */

extern int hitval(struct obj *,struct monst *);
extern int dmgval(struct obj *,struct monst *);
extern struct obj *select_rwep(struct monst *);
extern struct obj *select_hwep(struct monst *);
extern void possibly_unwield(struct monst *,bool);
extern int mon_wield_item(struct monst *);
extern int abon(void);
extern int dbon(void);
extern int enhance_weapon_skill(void);
extern void dump_weapon_skill(void);
extern void unrestrict_weapon_skill(int);
extern void use_skill(int,int);
extern void add_weapon_skill(int);
extern void lose_weapon_skill(int);
extern int weapon_type(struct obj *);
extern int uwep_skill_type(void);
extern int weapon_hit_bonus(struct obj *);
extern int weapon_dam_bonus(struct obj *);
extern void skill_init(const struct def_skill *);

/* ### wield.c ### */

extern void setuwep(struct obj *);
extern void setuqwep(struct obj *);
extern void setuswapwep(struct obj *);
extern int dowield(void);
extern int doswapweapon(void);
extern int dowieldquiver(void);
extern bool wield_tool(struct obj *,const char *);
extern int can_twoweapon(void);
extern void drop_uswapwep(void);
extern int dotwoweapon(void);
extern void uwepgone(void);
extern void uswapwepgone(void);
extern void uqwepgone(void);
extern void untwoweapon(void);
extern void erode_obj(struct obj *,bool,bool);
extern int chwepon(struct obj *,int);
extern int welded(struct obj *);
extern void weldmsg(struct obj *);
extern void setmnotwielded(struct monst *,struct obj *);

/* ### worm.c ### */

extern int get_wormno(void);
extern void initworm(struct monst *,int);
extern void worm_move(struct monst *);
extern void worm_nomove(struct monst *);
extern void wormgone(struct monst *);
extern void wormhitu(struct monst *);
extern void cutworm(struct monst *,signed char,signed char,struct obj *);
extern void see_wsegs(struct monst *);
extern void detect_wsegs(struct monst *,bool);
extern void save_worm(int,int);
extern void rest_worm(int);
extern void place_wsegs(struct monst *);
extern void remove_worm(struct monst *);
extern void place_worm_tail_randomly(struct monst *,signed char,signed char);
extern int count_wsegs(struct monst *);
extern bool worm_known(const struct monst *);

/* ### write.c ### */

extern int dowrite(struct obj *);

/* ### zap.c ### */

extern int bhitm(struct monst *,struct obj *);
extern void probe_monster(struct monst *);
extern bool get_obj_location(struct obj *,signed char *,signed char *,int);
extern bool get_mon_location(struct monst *,signed char *,signed char *,int);
extern struct monst *get_container_location(struct obj *obj, int *, int *);
extern struct monst *montraits(struct obj *,coord *);
extern struct monst *revive(struct obj *);
extern int unturn_dead(struct monst *);
extern void cancel_item(struct obj *);
extern bool drain_item(struct obj *);
extern struct obj *poly_obj(struct obj *, int);
extern bool obj_resists(struct obj *,int,int);
extern bool obj_shudders(struct obj *);
extern void do_osshock(struct obj *);
extern int bhito(struct obj *,struct obj *);
extern int bhitpile(struct obj *,int (*)(struct obj *,struct obj *),int,int);
extern int zappable(struct obj *);
extern void zapnodir(struct obj *);
extern int dozap(void);
extern int zapyourself(struct obj *,bool);
extern bool cancel_monst(struct monst *,struct obj *,
        bool,bool,bool);
extern void weffects(struct obj *);
extern int spell_damage_bonus(void);
extern const char *exclam(int force);
extern void hit(const char *,struct monst *,const char *);
extern void miss(const char *,struct monst *);
extern struct monst *bhit(int,int,int,int,int (*)(struct monst *,struct obj *),
        int (*)(struct obj *,struct obj *),struct obj *);
extern struct monst *boomhit(int,int);
extern int burn_floor_paper(int,int,bool,bool);
extern void buzz(int,int,signed char,signed char,int,int);
extern void melt_ice(signed char,signed char);
extern int zap_over_floor(signed char,signed char,int,bool *);
extern void fracture_rock(struct obj *);
extern bool break_statue(struct obj *);
extern void destroy_item(int,int);
extern int destroy_mitem(struct monst *,int,int);
extern int resist(struct monst *,char,int,int);
extern void makewish(void);

#endif /* EXTERN_H */
