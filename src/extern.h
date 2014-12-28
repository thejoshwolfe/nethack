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

/* ### allmain.c ### */

extern void moveloop(void);
extern void stop_occupation(void);
extern void display_gamewindows(void);
extern void newgame(void);
extern void welcome(bool);

/* ### apply.c ### */

extern int doapply(void);
extern int dorub(void);
extern int dojump(void);
extern int jump(int);
extern int number_leashed(void);
extern void o_unleash(struct obj *);
extern void m_unleash(struct monst *,bool);
extern void unleash_all(void);
extern bool next_to_u(void);
extern struct obj *get_mleash(struct monst *);
extern void check_leash(signed char,signed char);
extern bool um_dist(signed char,signed char,signed char);
extern bool snuff_candle(struct obj *);
extern bool snuff_lit(struct obj *);
extern bool catch_lit(struct obj *);
extern void use_unicorn_horn(struct obj *);
extern bool tinnable(struct obj *);
extern void reset_trapset(void);
extern void fig_transform(void *, long);
extern int unfixable_trouble_count(bool);

/* ### artifact.c ### */

extern void init_artifacts(void);
extern void save_artifacts(int);
extern void restore_artifacts(int);
extern const char *artiname(int);
extern struct obj *mk_artifact(struct obj *,aligntyp);
extern const char *artifact_name(const char *,short *);
extern bool exist_artifact(int,const char *);
extern void artifact_exists(struct obj *,const char *,bool);
extern int nartifact_exist(void);
extern bool spec_ability(struct obj *,unsigned long);
extern bool confers_luck(struct obj *);
extern bool arti_reflects(struct obj *);
extern bool restrict_name(struct obj *,const char *);
extern bool defends(int,const struct obj *);
extern bool protects(int,const struct obj *);
extern void set_artifact_intrinsic(struct obj *,bool,long);
extern int touch_artifact(struct obj *,struct monst *);
extern int spec_abon(struct obj *,struct monst *);
extern int spec_dbon(struct obj *,struct monst *,int);
extern void discover_artifact(signed char);
extern bool undiscovered_artifact(signed char);
extern int disp_artifact_discoveries(winid);
extern bool artifact_hit(struct monst *,struct monst *,
        struct obj *,int *,int);
extern int doinvoke(void);
extern void arti_speak(struct obj *);
extern bool artifact_light(struct obj *);
extern long spec_m2(struct obj *);
extern bool artifact_has_invprop(struct obj *,unsigned char);
extern long arti_cost(struct obj *);

/* ### attrib.c ### */

extern bool adjattrib(int,int,int);
extern void change_luck(signed char);
extern int stone_luck(bool);
extern void set_moreluck(void);
extern void gainstr(struct obj *,int);
extern void losestr(int);
extern void restore_attrib(void);
extern void exercise(int,bool);
extern void exerchk(void);
extern void reset_attribute_clock(void);
extern void init_attr(int);
extern void redist_attr(void);
extern void adjabil(int,int);
extern int newhp(void);
extern signed char acurr(int);
extern signed char acurrstr(void);
extern void adjalign(int);

/* ### ball.c ### */

extern void ballfall(void);
extern void placebc(void);
extern void unplacebc(void);
extern void set_bc(int);
extern void move_bc(int,int,signed char,signed char,signed char,signed char);
extern bool drag_ball(signed char,signed char,
        int *,signed char *,signed char *,signed char *,signed char *, bool *,bool);
extern void drop_ball(signed char,signed char);
extern void drag_down(void);

/* ### bones.c ### */

extern bool can_make_bones(void);
extern void savebones(struct obj *);
extern int getbones(void);

/* ### botl.c ### */

extern int xlev_to_rank(int);
extern int title_to_mon(const char *,int *,int *);
extern void max_rank_sz(void);
extern int describe_level(char *);
extern const char *rank_of(int,short,bool);
extern void bot(void);
extern void bot1str(char *);
extern void bot2str(char *);

/* ### cmd.c ### */

extern void reset_occupations(void);
extern void set_occupation(int (*)(void),const char *,int);
extern char pgetchar(void);
extern void pushch(char);
extern void savech(char);
extern void add_debug_extended_commands(void);
extern void rhack(char *);
extern int doextlist(void);
extern int extcmd_via_menu(void);
extern void enlightenment(int);
extern void show_conduct(int);
extern void dump_enlightenment(int);
extern void dump_conduct(int);
extern int xytod(signed char,signed char);
extern void dtoxy(coord *,int);
extern int movecmd(char);
extern int getdir(const char *);
extern void confdir(void);
extern int isok(int,int);
extern int get_adjacent_loc(const char *, const char *, signed char, signed char, coord *);
extern const char *click_to_cmd(int,int,int);
extern char readchar(void);
extern void sanity_check(void);
extern char yn_function(const char *, const char *, char);

/* ### decl.c ### */

extern void decl_init(void);

/* ### detect.c ### */

extern struct obj *o_in(struct obj*,char);
extern struct obj *o_material(struct obj*,unsigned);
extern int gold_detect(struct obj *);
extern int food_detect(struct obj *);
extern int object_detect(struct obj *,int);
extern int monster_detect(struct obj *,int);
extern int trap_detect(struct obj *);
extern const char *level_distance(d_level *);
extern void use_crystal_ball(struct obj *);
extern void do_mapping(void);
extern void do_vicinity_map(void);
extern void cvt_sdoor_to_door(struct rm *);
extern int findit(void);
extern int openit(void);
extern void find_trap(struct trap *);
extern int dosearch0(int);
extern int dosearch(void);
extern void sokoban_detect(void);

/* ### dig.c ### */

extern bool is_digging(void);
extern int holetime(void);
extern bool dig_check(struct monst *, bool, int, int);
extern void digactualhole(int,int,struct monst *,int);
extern bool dighole(bool);
extern int use_pick_axe(struct obj *);
extern int use_pick_axe2(struct obj *);
extern bool mdig_tunnel(struct monst *);
extern void watch_dig(struct monst *,signed char,signed char,bool);
extern void zap_dig(void);
extern struct obj *bury_an_obj(struct obj *);
extern void bury_objs(int,int);
extern void unearth_objs(int,int);
extern void rot_organic(void *, long);
extern void rot_corpse(void *, long);

/* ### display.c ### */

extern void magic_map_background(signed char,signed char,int);
extern void map_background(signed char,signed char,int);
extern void map_trap(struct trap *,int);
extern void map_object(struct obj *,int);
extern void map_invisible(signed char,signed char);
extern void unmap_object(int,int);
extern void map_location(int,int,int);
extern void feel_location(signed char,signed char);
extern void newsym(int,int);
extern void shieldeff(signed char,signed char);
extern void tmp_at(int,int);
extern void swallowed(int);
extern void under_ground(int);
extern void under_water(int);
extern void see_monsters(void);
extern void set_mimic_blocking(void);
extern void see_objects(void);
extern void see_traps(void);
extern void curs_on_u(void);
extern int doredraw(void);
extern void docrt(void);
extern void show_glyph(int,int,int);
extern void clear_glyph_buffer(void);
extern void row_refresh(int,int,int);
extern void cls(void);
extern void flush_screen(int);
extern void dump_screen(void);
extern int back_to_glyph(signed char,signed char);
extern int zapdir_to_glyph(int,int,int);
extern int glyph_at(signed char,signed char);
extern void set_wall_state(void);

/* ### do.c ### */

extern int dodrop(void);
extern bool boulder_hits_pool(struct obj *,int,int,bool);
extern bool flooreffects(struct obj *,int,int,const char *);
extern void doaltarobj(struct obj *);
extern bool canletgo(struct obj *,const char *);
extern void dropx(struct obj *);
extern void dropy(struct obj *);
extern void obj_no_longer_held(struct obj *);
extern int doddrop(void);
extern int dodown(void);
extern int doup(void);
extern void save_currentstate(void);
extern void goto_level(d_level *,bool,bool,bool);
extern void schedule_goto(d_level *,bool,bool,int,
        const char *,const char *);
extern void deferred_goto(void);
extern bool revive_corpse(struct obj *);
extern void revive_mon(void *, long);
extern int donull(void);
extern int dowipe(void);
extern void set_wounded_legs(long,int);
extern void heal_legs(void);

/* ### do_wear.c ### */

extern void off_msg(struct obj *);
extern void set_wear(void);
extern bool donning(struct obj *);
extern void cancel_don(void);
extern int Armor_off(void);
extern int Armor_gone(void);
extern int Helmet_off(void);
extern int Gloves_off(void);
extern int Boots_off(void);
extern int Cloak_off(void);
extern int Shield_off(void);
extern int Shirt_off(void);
extern void Amulet_off(void);
extern void Ring_on(struct obj *);
extern void Ring_off(struct obj *);
extern void Ring_gone(struct obj *);
extern void Blindf_on(struct obj *);
extern void Blindf_off(struct obj *);
extern int dotakeoff(void);
extern int doremring(void);
extern int cursed(struct obj *);
extern int armoroff(struct obj *);
extern int canwearobj(struct obj *, long *, bool);
extern int dowear(void);
extern int doputon(void);
extern void find_ac(void);
extern void glibr(void);
extern struct obj *some_armor(struct monst *);
extern void erode_armor(struct monst *,bool);
extern struct obj *stuck_ring(struct obj *,int);
extern struct obj *unchanger(void);
extern void reset_remarm(void);
extern int doddoremarm(void);
extern int destroy_arm(struct obj *);
extern void adj_abon(struct obj *,signed char);

/* ### dog.c ### */

extern void initedog(struct monst *);
extern struct monst *make_familiar(struct obj *,signed char,signed char,bool);
extern struct monst *makedog(void);
extern void update_mlstmv(void);
extern void losedogs(void);
extern void mon_arrive(struct monst *,bool);
extern void mon_catchup_elapsed_time(struct monst *,long);
extern void keepdogs(bool);
extern void migrate_to_level(struct monst *,signed char,signed char,coord *);
extern int dogfood(struct monst *,struct obj *);
extern struct monst *tamedog(struct monst *,struct obj *);
extern void abuse_dog(struct monst *);
extern void wary_dog(struct monst *, bool);

/* ### dogmove.c ### */

extern int dog_nutrition(struct monst *,struct obj *);
extern int dog_eat(struct monst *,struct obj *,int,int,bool);
extern int dog_move(struct monst *,int);

/* ### dokick.c ### */

extern bool ghitm(struct monst *,struct obj *);
extern void container_impact_dmg(struct obj *);
extern int dokick(void);
extern bool ship_object(struct obj *,signed char,signed char,bool);
extern void obj_delivery(void);
extern signed char down_gate(signed char,signed char);
extern void impact_drop(struct obj *,signed char,signed char,signed char);

/* ### dothrow.c ### */

extern int dothrow(void);
extern int dofire(void);
extern void hitfloor(struct obj *);
extern void hurtle(int,int,int,bool);
extern void mhurtle(struct monst *,int,int,int);
extern void throwit(struct obj *,long,bool);
extern int omon_adj(struct monst *,struct obj *,bool);
extern int thitmonst(struct monst *,struct obj *);
extern int hero_breaks(struct obj *,signed char,signed char,bool);
extern int breaks(struct obj *,signed char,signed char);
extern bool breaktest(struct obj *);
extern bool walk_path(coord *, coord *, bool (*)(void *,int,int), void *);
extern bool hurtle_step(void *, int, int);

/* ### drawing.c ### */
extern int def_char_to_objclass(char);
extern int def_char_to_monclass(char);
extern void assign_graphics(unsigned char *,int,int,int);
extern void switch_graphics(int);

/* ### dungeon.c ### */

extern void save_dungeon(int,bool,bool);
extern void restore_dungeon(int);
extern void insert_branch(branch *,bool);
extern void init_dungeons(void);
extern s_level *find_level(const char *);
extern s_level *Is_special(d_level *);
extern branch *Is_branchlev(d_level *);
extern signed char ledger_no(d_level *);
extern signed char maxledgerno(void);
extern signed char depth(d_level *);
extern signed char dunlev(d_level *);
extern signed char dunlevs_in_dungeon(d_level *);
extern signed char ledger_to_dnum(signed char);
extern signed char ledger_to_dlev(signed char);
extern signed char deepest_lev_reached(bool);
extern bool on_level(d_level *,d_level *);
extern void next_level(bool);
extern void prev_level(bool);
extern void u_on_newpos(int,int);
extern void u_on_sstairs(void);
extern void u_on_upstairs(void);
extern void u_on_dnstairs(void);
extern bool On_stairs(signed char,signed char);
extern void get_level(d_level *,int);
extern bool Is_botlevel(d_level *);
extern bool Can_fall_thru(d_level *);
extern bool Can_dig_down(d_level *);
extern bool Can_rise_up(int,int,d_level *);
extern bool In_quest(d_level *);
extern bool In_mines(d_level *);
extern branch *dungeon_branch(const char *);
extern bool at_dgn_entrance(const char *);
extern bool In_hell(d_level *);
extern bool In_V_tower(d_level *);
extern bool On_W_tower_level(d_level *);
extern bool In_W_tower(int,int,d_level *);
extern void find_hell(d_level *);
extern void goto_hell(bool,bool);
extern void assign_level(d_level *,d_level *);
extern void assign_rnd_level(d_level *,d_level *,int);
extern int induced_align(int);
extern bool Invocation_lev(d_level *);
extern signed char level_difficulty(void);
extern signed char lev_by_name(const char *);
extern signed char print_dungeon(bool,signed char *,signed char *);

/* ### eat.c ### */

extern bool is_edible(struct obj *);
extern void init_uhunger(void);
extern int Hear_again(void);
extern void reset_eat(void);
extern int doeat(void);
extern void gethungry(void);
extern void morehungry(int);
extern void lesshungry(int);
extern bool is_fainted(void);
extern void reset_faint(void);
extern void violated_vegetarian(void);
extern void newuhs(bool);
extern struct obj *floorfood(const char *,int);
extern void vomit(void);
extern int eaten_stat(int,struct obj *);
extern void food_disappears(struct obj *);
extern void food_substitution(struct obj *,struct obj *);
extern void fix_petrification(void);
extern void consume_oeaten(struct obj *,int);
extern bool maybe_finished_meal(bool);

/* ### end.c ### */

extern void done1(int);
extern int done2(void);
extern void done_in_by(struct monst *);
extern void panic(const char *, ...) __attribute__ ((format (printf, 1, 2)));
extern void done(int);
extern void container_contents(struct obj *,bool,bool);
extern void dump(char *, char *);
extern void do_containerconts(struct obj *,bool,bool,bool);
extern void terminate(int);
extern int num_genocides(void);

/* ### engrave.c ### */

extern char *random_engraving(char *);
extern void wipeout_text(char *,int,unsigned);
extern bool can_reach_floor(void);
extern const char *surface(int,int);
extern const char *ceiling(int,int);
extern struct engr *engr_at(signed char,signed char);
extern int sengr_at(const char *,signed char,signed char);
extern void u_wipe_engr(int);
extern void wipe_engr_at(signed char,signed char,signed char);
extern void read_engr_at(int,int);
extern void make_engr_at(int,int,const char *,long,signed char);
extern void del_engr_at(int,int);
extern int freehand(void);
extern int doengrave(void);
extern void save_engravings(int,int);
extern void rest_engravings(int);
extern void del_engr(struct engr *);
extern void rloc_engr(struct engr *);
extern void make_grave(int,int,const char *);

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

/* ### files.c ### */

extern char *fname_encode(const char *, char, char *, char *, int);
extern char *fname_decode(char, char *, char *, int);
extern const char *fqname(const char *, int, int);
extern FILE *fopen_datafile(const char *,const char *,int);
extern bool uptodate(int,const char *);
extern void store_version(int);
extern void set_levelfile_name(char *,int);
extern int create_levelfile(int,char *);
extern int open_levelfile(int,char *);
extern void delete_levelfile(int);
extern void clearlocks(void);
extern int create_bonesfile(d_level*,char **, char *);
extern void commit_bonesfile(d_level *);
extern int open_bonesfile(d_level*,char **);
extern int delete_bonesfile(d_level*);
extern void set_savefile_name(void);
extern void save_savefile_name(int);
extern void set_error_savefile(void);
extern int create_savefile(void);
extern int open_savefile(void);
extern int delete_savefile(void);
extern int restore_saved_game(void);
extern bool lock_file(const char *,int,int);
extern void unlock_file(const char *);
extern void read_config_file(const char *);
extern void check_recordfile(const char *);
extern void read_wizkit(void);
extern void paniclog(const char *, const char *);
extern int validate_prefix_locations(char *);
extern char** get_saved_games(void);
extern void free_saved_games(char**);

/* ### fountain.c ### */

extern void floating_above(const char *);
extern void dogushforth(int);
extern void dryup(signed char,signed char, bool);
extern void drinkfountain(void);
extern void dipfountain(struct obj *);
extern void breaksink(int,int);
extern void drinksink(void);

/* ### hack.c ### */

extern bool revive_nasty(int,int,const char*);
extern void movobj(struct obj *,signed char,signed char);
extern bool may_dig(signed char,signed char);
extern bool may_passwall(signed char,signed char);
extern bool bad_rock(struct permonst *,signed char,signed char);
extern bool invocation_pos(signed char,signed char);
extern bool test_move(int, int, int, int, int);
extern void domove(void);
extern void invocation_message(void);
extern void spoteffects(bool);
extern char *in_rooms(signed char,signed char,int);
extern bool in_town(int,int);
extern void check_special_room(bool);
extern int dopickup(void);
extern void lookaround(void);
extern int monster_nearby(void);
extern void nomul(int);
extern void unmul(const char *);
extern void losehp(int,const char *,bool);
extern int weight_cap(void);
extern int inv_weight(void);
extern int near_capacity(void);
extern int calc_capacity(int);
extern int max_capacity(void);
extern bool check_capacity(const char *);
extern int inv_cnt(void);

/* ### invent.c ### */

extern void assigninvlet(struct obj *);
extern struct obj *merge_choice(struct obj *,struct obj *);
extern int merged(struct obj **,struct obj **);
extern void addinv_core1(struct obj *);
extern void addinv_core2(struct obj *);
extern struct obj *addinv(struct obj *);
extern struct obj *hold_another_object(struct obj *,const char *,const char *,const char *);
extern void useupall(struct obj *);
extern void useup(struct obj *);
extern void consume_obj_charge(struct obj *,bool);
extern void freeinv_core(struct obj *);
extern void freeinv(struct obj *);
extern void delallobj(int,int);
extern void delobj(struct obj *);
extern struct obj *sobj_at(int,int,int);
extern struct obj *carrying(int);
extern bool have_lizard(void);
extern struct obj *o_on(unsigned int,struct obj *);
extern bool obj_here(struct obj *,int,int);
extern bool wearing_armor(void);
extern bool is_worn(struct obj *);
extern struct obj *g_at(int,int);
extern struct obj *mkgoldobj(long);
extern struct obj *getobj(const char *,const char *);
extern int ggetobj(const char *,int (*)(struct obj *),int,bool,unsigned *);
extern void fully_identify_obj(struct obj *);
extern int identify(struct obj *);
extern void identify_pack(int);
extern int askchain(struct obj **,const char *,int,int (*)(struct obj *),
        int (*)(struct obj *),int,const char *);
extern void prinv(const char *,struct obj *,long);
extern char *xprname(struct obj *,const char *,char,bool,long,long);
extern int ddoinv(void);
extern char display_inventory(const char *,bool);
extern char dump_inventory(const char *,bool);
extern int display_binventory(int,int,bool);
extern struct obj *display_cinventory(struct obj *);
extern struct obj *display_minventory(struct monst *,int,char *);
extern int dotypeinv(void);
extern const char *dfeature_at(int,int,char *);
extern int look_here(int,bool);
extern int dolook(void);
extern bool will_feel_cockatrice(struct obj *,bool);
extern void feel_cockatrice(struct obj *,bool);
extern void stackobj(struct obj *);
extern int doprgold(void);
extern int doprwep(void);
extern int doprarm(void);
extern int doprring(void);
extern int dopramulet(void);
extern int doprtool(void);
extern int doprinuse(void);
extern void useupf(struct obj *,long);
extern char *let_to_name(char,bool);
extern void free_invbuf(void);
extern void reassign(void);
extern int doorganize(void);
extern int count_unpaid(struct obj *);
extern int count_buc(struct obj *,int);
extern void carry_obj_effects(struct obj *);
extern const char *currency(long);
extern void silly_thing(const char *,struct obj *);

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

/* ### mail.c ### */

extern void getmailstatus(void);
extern void ckmailstatus(void);
extern void readmail(struct obj *);

/* ### makemon.c ### */

extern bool is_home_elemental(struct permonst *);
extern struct monst *clone_mon(struct monst *,signed char,signed char);
extern struct monst *makemon(struct permonst *,int,int,int);
extern bool create_critters(int,struct permonst *);
extern struct permonst *rndmonst(void);
extern void reset_rndmonst(int);
extern struct permonst *mkclass(char,int);
extern int adj_lev(struct permonst *);
extern struct permonst *grow_up(struct monst *,struct monst *);
extern int mongets(struct monst *,int);
extern int golemhp(int);
extern bool peace_minded(struct permonst *);
extern void set_malign(struct monst *);
extern void set_mimic_sym(struct monst *);
extern int mbirth_limit(int);
extern void mimic_hit_msg(struct monst *, short);
extern void bagotricks(struct obj *);
extern bool propagate(int, bool,bool);

/* ### mapglyph.c ### */

extern void mapglyph(int, int *, int *, unsigned *, int, int);

/* ### mcastu.c ### */

extern int castmu(struct monst *,struct attack *,bool,bool);
extern int buzzmu(struct monst *,struct attack *);

/* ### mhitm.c ### */

extern int fightm(struct monst *);
extern int mattackm(struct monst *,struct monst *);
extern int noattacks(struct permonst *);
extern int sleep_monst(struct monst *,int,int);
extern void slept_monst(struct monst *);
extern long attk_protection(int);

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

/* ### mklev.c ### */

extern void sort_rooms(void);
extern void add_room(int,int,int,int,bool,signed char,bool);
extern void add_subroom(struct mkroom *,int,int,int,int,
        bool,signed char,bool);
extern void makecorridors(void);
extern void add_door(int,int,struct mkroom *);
extern void mklev(void);
extern void topologize(struct mkroom *);
extern void place_branch(branch *,signed char,signed char);
extern bool occupied(signed char,signed char);
extern int okdoor(signed char,signed char);
extern void dodoor(int,int,struct mkroom *);
extern void mktrap(int,int,struct mkroom *,coord*);
extern void mkstairs(signed char,signed char,char,struct mkroom *);
extern void mkinvokearea(void);

/* ### mkmap.c ### */

void flood_fill_rm(int,int,int,bool,bool);
void remove_rooms(int,int,int,int);

/* ### mkmaze.c ### */

extern void wallification(int,int,int,int);
extern void walkfrom(int,int);
extern void makemaz(const char *);
extern void mazexy(coord *);
extern void bound_digging(void);
extern void mkportal(signed char,signed char,signed char,signed char);
extern bool bad_location(signed char,signed char,signed char,signed char,signed char,signed char);
extern void place_lregion(signed char,signed char,signed char,signed char,
        signed char,signed char,signed char,signed char,
        signed char,d_level *);
extern void movebubbles(void);
extern void water_friction(void);
extern void save_waterlevel(int,int);
extern void restore_waterlevel(int);
extern const char *waterbody_name(signed char,signed char);

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

/* ### mon.c ### */

extern int undead_to_corpse(int);
extern int genus(int,int);
extern int pm_to_cham(int);
extern int minliquid(struct monst *);
extern int movemon(void);
extern int meatmetal(struct monst *);
extern int meatobj(struct monst *);
extern void mpickgold(struct monst *);
extern bool mpickstuff(struct monst *,const char *);
extern int curr_mon_load(struct monst *);
extern int max_mon_load(struct monst *);
extern bool can_carry(struct monst *,struct obj *);
extern int mfndpos(struct monst *,coord *,long *,long);
extern bool monnear(struct monst *,int,int);
extern void dmonsfree(void);
extern int mcalcmove(struct monst*);
extern void mcalcdistress(void);
extern void replmon(struct monst *,struct monst *);
extern void relmon(struct monst *);
extern struct obj *mlifesaver(struct monst *);
extern bool corpse_chance(struct monst *,struct monst *,bool);
extern void mondead(struct monst *);
extern void mondied(struct monst *);
extern void mongone(struct monst *);
extern void monstone(struct monst *);
extern void monkilled(struct monst *,const char *,int);
extern void unstuck(struct monst *);
extern void killed(struct monst *);
extern void xkilled(struct monst *,int);
extern void mon_to_stone(struct monst*);
extern void mnexto(struct monst *);
extern bool mnearto(struct monst *,signed char,signed char,bool);
extern void poisontell(int);
extern void poisoned(const char *,int,const char *,int);
extern void m_respond(struct monst *);
extern void setmangry(struct monst *);
extern void wakeup(struct monst *);
extern void wake_nearby(void);
extern void wake_nearto(int,int,int);
extern void seemimic(struct monst *);
extern void rescham(void);
extern void restartcham(void);
extern void restore_cham(struct monst *);
extern void mon_animal_list(bool);
extern int newcham(struct monst *,struct permonst *,bool,bool);
extern int can_be_hatched(int);
extern int egg_type_from_parent(int,bool);
extern bool dead_species(int,bool);
extern void kill_genocided_monsters(void);
extern void golemeffects(struct monst *,int,int);
extern bool angry_guards(bool);
extern void pacify_guards(void);

/* ### mondata.c ### */

extern void set_mon_data(struct monst *,struct permonst *,int);
extern const struct attack * attacktype_fordmg(const struct permonst *, int, int);
extern bool attacktype(const struct permonst *,int);
extern bool poly_when_stoned(const struct permonst *);
extern bool resists_drli(const struct monst *);
extern bool resists_magm(const struct monst *);
extern bool resists_blnd(const struct monst *);
extern bool can_blnd(const struct monst *,const struct monst *,unsigned char,const struct obj *);
extern bool ranged_attk(const struct permonst *);
extern bool hates_silver(const struct permonst *);
extern bool passes_bars(const struct permonst *);
extern bool can_track(const struct permonst *);
extern bool breakarm(const struct permonst *);
extern bool sliparm(const struct permonst *);
extern bool sticks(const struct permonst *);
extern int num_horns(const struct permonst *);
extern const struct attack * dmgtype_fromattack(const struct permonst *,int,int);
extern bool dmgtype(const struct permonst *,int);
extern int max_passive_dmg(const struct monst *,const struct monst *);
extern int monsndx(const struct permonst *);
extern int name_to_mon(const char *);
extern int gender(const struct monst *);
extern int pronoun_gender(const struct monst *);
extern bool levl_follower(const struct monst *);
extern int little_to_big(int);
extern int big_to_little(int);
extern const char *locomotion(const struct permonst *,const char *);
extern const char *stagger(const struct permonst *,const char *);
extern const char *on_fire(const struct permonst *,const struct attack *);
extern const struct permonst * raceptr(const struct monst *);

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

/* ### monst.c ### */

extern void monst_init(void);

/* ### monstr.c ### */

extern void monstr_init(void);

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

/* ### o_init.c ### */

extern void init_objects(void);
extern int find_skates(void);
extern void oinit(void);
extern void savenames(int,int);
extern void restnames(int);
extern void discover_object(int,bool,bool);
extern void undiscover_object(int);
extern int dodiscovered(void);

/* ### objects.c ### */

extern void objects_init(void);

/* ### options.c ### */

extern bool match_optname(const char *,const char *,int,bool);
extern void initoptions(void);
extern void parseoptions(char *,bool,bool);
extern int doset(void);
extern int dotogglepickup(void);
extern void option_help(void);
extern void next_opt(winid,const char *);
extern int fruitadd(char *);
extern int choose_classes_menu(const char *,int,bool,char *,char *);
extern void add_menu_cmd_alias(char, char);
extern char map_menu_cmd(char);
extern void assign_warnings(unsigned char *);
extern char *nh_getenv(const char *);
extern void set_duplicate_opt_detection(int);
extern void set_wc_option_mod_status(unsigned long, int);
extern void set_wc2_option_mod_status(unsigned long, int);
extern void set_option_mod_status(const char *,int);
extern int add_autopickup_exception(const char *);
extern void free_autopickup_exceptions(void);

/* ### pager.c ### */

extern int dowhatis(void);
extern int doquickwhatis(void);
extern int doidtrap(void);
extern int dowhatdoes(void);
extern char *dowhatdoes_core(char, char *);
extern int dohelp(void);
extern int dohistory(void);

/* ### pickup.c ### */

extern int collect_obj_classes(char *,struct obj *,bool,bool,bool (*)(struct obj *), int *);
extern void add_valid_menu_class(int);
extern bool allow_all(struct obj *);
extern bool allow_category(struct obj *);
extern bool is_worn_by_type(struct obj *);
extern int pickup(int);
extern int pickup_object(struct obj *, long, bool);
extern int query_category(const char *, struct obj *, int,
        menu_item **, int);
extern int query_objlist(const char *, struct obj *, int,
        menu_item **, int, bool (*)(struct obj *));
extern struct obj *pick_obj(struct obj *);
extern int encumber_msg(void);
extern int doloot(void);
extern int use_container(struct obj *,int);
extern int loot_mon(struct monst *,int *,bool *);
extern const char *safe_qbuf(const char *,unsigned,
        const char *,const char *,const char *);
extern bool is_autopickup_exception(struct obj *, bool);

/* ### pline.c ### */

extern void pline(const char *,...) __attribute__ ((format (printf, 1, 2)));
extern void plines(const char *);
extern void Norep(const char *,...) __attribute__ ((format (printf, 1, 2)));
extern void free_youbuf(void);
extern void You(const char *,...) __attribute__ ((format (printf, 1, 2)));
extern void Your(const char *,...) __attribute__ ((format (printf, 1, 2)));
extern void You_feel(const char *,...) __attribute__ ((format (printf, 1, 2)));
extern void You_cant(const char *,...) __attribute__ ((format (printf, 1, 2)));
extern void You_hear(const char *,...) __attribute__ ((format (printf, 1, 2)));
extern void pline_The(const char *,...) __attribute__ ((format (printf, 1, 2)));
extern void There(const char *,...) __attribute__ ((format (printf, 1, 2)));
extern void verbalize(const char *,...) __attribute__ ((format (printf, 1, 2)));
extern void raw_printf(const char *,...) __attribute__ ((format (printf, 1, 2)));
extern void impossible(const char *,...) __attribute__ ((format (printf, 1, 2)));
extern const char *align_str(aligntyp);
extern void mstatusline(struct monst *);
extern void ustatusline(void);
extern void self_invis_message(void);

/* ### polyself.c ### */

extern void set_uasmon(void);
extern void change_sex(void);
extern void polyself(bool);
extern int polymon(int);
extern void rehumanize(void);
extern int dobreathe(void);
extern int dospit(void);
extern int doremove(void);
extern int dospinweb(void);
extern int dosummon(void);
extern int dogaze(void);
extern int dohide(void);
extern int domindblast(void);
extern void skinback(bool);
extern const char *mbodypart(struct monst *,int);
extern const char *body_part(int);
extern int poly_gender(void);
extern void ugolemeffects(int,int);

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

/* ### questpgr.c ### */

extern void load_qtlist(void);
extern void unload_qtlist(void);
extern short quest_info(int);
extern const char *ldrname(void);
extern bool is_quest_artifact(struct obj*);
extern void com_pager(int);
extern void qt_pager(int);
extern struct permonst *qt_montype(void);

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

/* ## region.c ### */
extern void clear_regions(void);
extern void run_regions(void);
extern bool in_out_region(signed char,signed char);
extern bool m_in_out_region(struct monst *,signed char,signed char);
extern void update_player_regions(void);
extern void update_monster_region(struct monst *);
extern NhRegion *visible_region_at(signed char,signed char);
extern void show_region(NhRegion*, signed char, signed char);
extern void save_regions(int,int);
extern void rest_regions(int,bool);
extern NhRegion* create_gas_cloud(signed char, signed char, int, int);

/* ### restore.c ### */

extern void inven_inuse(bool);
extern int dorecover(int);
extern void trickery(char *);
extern void getlev(int,int,signed char,bool);
extern void minit(void);
extern bool lookup_id_mapping(unsigned, unsigned *);
extern void mread(int,void *,unsigned int);

/* ### rip.c ### */

extern void genl_outrip(winid,int);

/* ### rnd.c ### */

extern int rn2(int);
extern int rnl(int);
extern int rnd(int);
extern int d(int,int);
extern int rne(int);
extern int rnz(int);

/* ### role.c ### */

extern bool validrole(int);
extern bool validrace(int, int);
extern bool validgend(int, int, int);
extern bool validalign(int, int, int);
extern int randrole(void);
extern int randrace(int);
extern int randgend(int, int);
extern int randalign(int, int);
extern int str2role(char *);
extern int str2race(char *);
extern int str2gend(char *);
extern int str2align(char *);
extern bool ok_role(int, int, int, int);
extern int pick_role(int, int, int, int);
extern bool ok_race(int, int, int, int);
extern int pick_race(int, int, int, int);
extern bool ok_gend(int, int, int, int);
extern int pick_gend(int, int, int, int);
extern bool ok_align(int, int, int, int);
extern int pick_align(int, int, int, int);
extern void role_init(void);
extern void rigid_role_checks(void);
extern void plnamesuffix(void);
extern const char *Hello(struct monst *);
extern const char *Goodbye(void);
extern char *build_plselection_prompt(char *, int, int, int, int, int);
extern char *root_plselection_prompt(char *, int, int, int, int, int);

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

/* ### shk.c ### */

extern char *shkname(struct monst *);
extern void shkgone(struct monst *);
extern void set_residency(struct monst *,bool);
extern void replshk(struct monst *,struct monst *);
extern void restshk(struct monst *,bool);
extern char inside_shop(signed char,signed char);
extern void u_left_shop(char *,bool);
extern void remote_burglary(signed char,signed char);
extern void u_entered_shop(char *);
extern bool same_price(struct obj *,struct obj *);
extern void shopper_financial_report(void);
extern int inhishop(struct monst *);
extern struct monst *shop_keeper(char);
extern bool tended_shop(struct mkroom *);
extern void delete_contents(struct obj *);
extern void obfree(struct obj *,struct obj *);
extern void home_shk(struct monst *,bool);
extern void make_happy_shk(struct monst *,bool);
extern void hot_pursuit(struct monst *);
extern void make_angry_shk(struct monst *,signed char,signed char);
extern int dopay(void);
extern bool paybill(int);
extern void finish_paybill(void);
extern struct obj *find_oid(unsigned);
extern long contained_cost(struct obj *,struct monst *,long,bool, bool);
extern long contained_gold(struct obj *);
extern void picked_container(struct obj *);
extern long unpaid_cost(struct obj *);
extern void addtobill(struct obj *,bool,bool,bool);
extern void splitbill(struct obj *,struct obj *);
extern void subfrombill(struct obj *,struct monst *);
extern long stolen_value(struct obj *,signed char,signed char,bool,bool);
extern void sellobj_state(int);
extern void sellobj(struct obj *,signed char,signed char);
extern int doinvbill(int);
extern struct monst *shkcatch(struct obj *,signed char,signed char);
extern void add_damage(signed char,signed char,long);
extern int repair_damage(struct monst *,struct damage *,bool);
extern int shk_move(struct monst *);
extern void after_shk_move(struct monst *);
extern bool is_fshk(const struct monst *);
extern void shopdig(int);
extern void pay_for_damage(const char *,bool);
extern bool costly_spot(signed char,signed char);
extern struct obj *shop_object(signed char,signed char);
extern void price_quote(struct obj *);
extern void shk_chat(struct monst *);
extern void check_unpaid_usage(struct obj *,bool);
extern void check_unpaid(struct obj *);
extern void costly_gold(signed char,signed char,long);
extern bool block_door(signed char,signed char);
extern bool block_entry(signed char,signed char);
extern char *shk_your(char *,struct obj *);
extern char *Shk_Your(char *,struct obj *);

/* ### shknam.c ### */

extern void stock_room(int,struct mkroom *);
extern bool saleable(struct monst *,struct obj *);
extern int get_shop_item(int);

/* ### sit.c ### */

extern void take_gold(void);
extern int dosit(void);
extern void rndcurse(void);
extern void attrcurse(void);

/* ### sounds.c ### */

extern void dosounds(void);
extern const char *growl_sound(struct monst *);
extern void growl(struct monst *);
extern void yelp(struct monst *);
extern void whimper(struct monst *);
extern void beg(struct monst *);
extern int dotalk(void);

/* ### sp_lev.c ### */

extern bool check_room(signed char *,signed char *,signed char *,signed char *,bool);
extern bool create_room(signed char,signed char,signed char,signed char,
        signed char,signed char,signed char,signed char);
extern void create_secret_door(struct mkroom *,signed char);
extern bool dig_corridor(coord *,coord *,bool,signed char,signed char);
extern void fill_room(struct mkroom *,bool);
extern bool load_special(const char *);

/* ### spell.c ### */

extern int study_book(struct obj *);
extern void book_disappears(struct obj *);
extern void book_substitution(struct obj *,struct obj *);
extern void age_spells(void);
extern int docast(void);
extern int spell_skilltype(int);
extern int spelleffects(int,bool);
extern void losespells(void);
extern int dovspell(void);
extern void initialspell(struct obj *);

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

/* ### teleport.c ### */

extern bool goodpos(int,int,struct monst *,unsigned);
extern bool enexto(coord *,signed char,signed char,struct permonst *);
extern bool enexto_core(coord *,signed char,signed char,struct permonst *,unsigned);
extern void teleds(int,int,bool);
extern bool safe_teleds(bool);
extern bool teleport_pet(struct monst *,bool);
extern void tele(void);
extern int dotele(void);
extern void level_tele(void);
extern void domagicportal(struct trap *);
extern void tele_trap(struct trap *);
extern void level_tele_trap(struct trap *);
extern void rloc_to(struct monst *,int,int);
extern bool rloc(struct monst *, bool);
extern bool tele_restrict(struct monst *);
extern void mtele_trap(struct monst *, struct trap *,int);
extern int mlevel_tele_trap(struct monst *, struct trap *,bool,int);
extern void rloco(struct obj *);
extern int random_teleport_level(void);
extern bool u_teleport_mon(struct monst *,bool);

/* ### timeout.c ### */

extern void burn_away_slime(void);
extern void nh_timeout(void);
extern void fall_asleep(int, bool);
extern void attach_egg_hatch_timeout(struct obj *);
extern void attach_fig_transform_timeout(struct obj *);
extern void kill_egg(struct obj *);
extern void hatch_egg(void *, long);
extern void learn_egg_type(int);
extern void burn_object(void *, long);
extern void begin_burn(struct obj *, bool);
extern void end_burn(struct obj *, bool);
extern void do_storms(void);
extern bool start_timer(long, short, short, void *);
extern long stop_timer(short, void *);
extern void run_timers(void);
extern void obj_move_timers(struct obj *, struct obj *);
extern void obj_split_timers(struct obj *, struct obj *);
extern void obj_stop_timers(struct obj *);
extern bool obj_is_local(struct obj *);
extern void save_timers(int,int,int);
extern void restore_timers(int,int,bool,long);
extern void relink_timers(bool);
extern int wiz_timeout_queue(void);
extern void timer_sanity_check(void);

/* ### topten.c ### */

extern void topten(int);
extern void prscore(int,char **);
extern struct obj *tt_oname(struct obj *);

/* ### track.c ### */

extern void initrack(void);
extern void settrack(void);
extern coord *gettrack(int,int);

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

/* ### u_init.c ### */

extern void u_init(void);

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

/* ### unixtty.c ### */

extern void gettty(void);
extern void settty(const char *);
extern void setftty(void);
extern void intron(void);
extern void introff(void);
extern void error(const char *,...);

/* ### unixunix.c ### */

extern void getlock(void);
extern void regularize(char *);

/* ### vault.c ### */

extern bool grddead(struct monst *);
extern char vault_occupied(char *);
extern void invault(void);
extern int gd_move(struct monst *);
extern void paygd(void);
extern long hidden_gold(void);
extern bool gd_sound(void);

/* ### version.c ### */

extern char *version_string(char *);
extern char *getversionstring(char *);
extern int doversion(void);
extern int doextversion(void);
extern bool check_version(struct version_info *, const char *,bool);
extern unsigned long get_feature_notice_ver(char *);
extern unsigned long get_current_feature_ver(void);

/* ### vision.c ### */

extern void vision_init(void);
extern int does_block(int,int,struct rm*);
extern void vision_reset(void);
extern void vision_recalc(int);
extern void block_point(int,int);
extern void unblock_point(int,int);
extern bool clear_path(int,int,int,int);
extern void do_clear_area(int,int,int,
        void (*)(int,int,void *),void *);

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

/* ### were.c ### */

extern void were_change(struct monst *);
extern void new_were(struct monst *);
extern int were_summon(struct permonst *,bool,int *,char *);
extern void you_were(void);
extern void you_unwere(bool);

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

/* ### windows.c ### */

extern void choose_windows(const char *);
extern char genl_message_menu(char,int,const char *);
extern void genl_preference_update(const char *);

/* ### wizard.c ### */

extern void amulet(void);
extern bool mon_has_amulet(const struct monst *);
extern int mon_has_special(struct monst *);
extern int tactics(struct monst *);
extern void aggravate(void);
extern void clonewiz(void);
extern int pick_nasty(void);
extern int nasty(struct monst*);
extern void resurrect(void);
extern void intervene(void);
extern void wizdead(void);
extern void cuss(struct monst *);

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

/* ### worn.c ### */

extern void setworn(struct obj *,long);
extern void setnotworn(struct obj *);
extern void mon_set_minvis(struct monst *);
extern void mon_adjust_speed(struct monst *,int,struct obj *);
extern void update_mon_intrinsics(struct monst *,struct obj *,bool,bool);
extern int find_mac(struct monst *);
extern void m_dowear(struct monst *,bool);
extern struct obj *which_armor(struct monst *,long);
extern void mon_break_armor(struct monst *,bool);
extern void bypass_obj(struct obj *);
extern void clear_bypasses(void);
extern int racial_exception(struct monst *, struct obj *);

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
