/* See LICENSE in the root of this project for change info */

#ifndef MONDATA_H
#define MONDATA_H

#include <stdbool.h>

#include "align.h"
#include "epri.h"
#include "global.h"
#include "monattk.h"
#include "monflag.h"
#include "monst.h"
#include "monsym.h"
#include "obj.h"
#include "objclass.h"
#include "permonst.h"
#include "pm.h"
#include "you.h"

void set_mon_data(struct monst *,struct permonst *,int);
const struct attack * attacktype_fordmg(const struct permonst *, int, int);
bool attacktype(const struct permonst *,int);
bool poly_when_stoned(const struct permonst *);
bool resists_drli(const struct monst *);
bool resists_magm(const struct monst *);
bool resists_blnd(const struct monst *);
bool can_blnd(const struct monst *,const struct monst *,unsigned char,const struct obj *);
bool ranged_attk(const struct permonst *);
bool hates_silver(const struct permonst *);
bool passes_bars(const struct permonst *);
bool can_track(const struct permonst *);
bool breakarm(const struct permonst *);
bool sliparm(const struct permonst *);
bool sticks(const struct permonst *);
int num_horns(const struct permonst *);
const struct attack * dmgtype_fromattack(const struct permonst *,int,int);
bool dmgtype(const struct permonst *,int);
int max_passive_dmg(const struct monst *,const struct monst *);
int monsndx(const struct permonst *);
int name_to_mon(const char *);
int gender(const struct monst *);
int pronoun_gender(const struct monst *);
bool levl_follower(const struct monst *);
int little_to_big(int);
int big_to_little(int);
const char *locomotion(const struct permonst *,const char *);
const char *stagger(const struct permonst *,const char *);
const char *on_fire(const struct permonst *,const struct attack *);
const struct permonst * raceptr(const struct monst *);


static bool verysmall(const struct permonst * ptr) {
    return ptr->msize < MZ_SMALL;
}
static bool bigmonst(const struct permonst * ptr) {
    return ptr->msize >= MZ_LARGE;
}

static bool pm_resistance(const struct permonst * ptr, unsigned char typ) {
    return (ptr->mresists & typ) != 0;
}
static bool resists_fire(const struct monst * mon) {
    return (mon->mintrinsics & MR_FIRE) != 0;
}
static bool resists_cold(const struct monst * mon) {
    return (mon->mintrinsics & MR_COLD) != 0;
}
static bool resists_sleep(const struct monst * mon) {
    return (mon->mintrinsics & MR_SLEEP) != 0;
}
static bool resists_disint(const struct monst * mon) {
    return (mon->mintrinsics & MR_DISINT) != 0;
}
static bool resists_elec(const struct monst * mon) {
    return (mon->mintrinsics & MR_ELEC) != 0;
}
static bool resists_poison(const struct monst * mon) {
    return (mon->mintrinsics & MR_POISON) != 0;
}
static bool resists_acid(const struct monst * mon) {
    return (mon->mintrinsics & MR_ACID) != 0;
}
static bool resists_ston(const struct monst * mon) {
    return (mon->mintrinsics & MR_STONE) != 0;
}

static bool is_flyer(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_FLY) != 0L;
}
static bool is_floater(const struct permonst * ptr) {
    return ptr->mlet == S_EYE;
}
static bool is_clinger(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_CLING) != 0L;
}
static bool is_swimmer(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_SWIM) != 0L;
}
static bool breathless(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_BREATHLESS) != 0L;
}
static bool amphibious(const struct permonst * ptr) {
    return (ptr->mflags1 & (M1_AMPHIBIOUS | M1_BREATHLESS)) != 0L;
}
static bool passes_walls(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_WALLWALK) != 0L;
}
static bool amorphous(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_AMORPHOUS) != 0L;
}
static bool noncorporeal(const struct permonst * ptr) {
    return ptr->mlet == S_GHOST;
}
static bool tunnels(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_TUNNEL) != 0L;
}
static bool needspick(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_NEEDPICK) != 0L;
}
static bool hides_under(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_CONCEAL) != 0L;
}
static bool is_hider(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_HIDE) != 0L;
}
static bool haseyes(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_NOEYES) == 0L;
}
static int eyecount(const struct permonst * ptr) {
    if (!haseyes(ptr))
        return 0;
    if (ptr == &mons[PM_CYCLOPS] || ptr == &mons[PM_FLOATING_EYE])
        return 1;
    return 2;
}
static bool nohands(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_NOHANDS) != 0L;
}
static bool nolimbs(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_NOLIMBS) == M1_NOLIMBS;
}
static bool notake(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_NOTAKE) != 0L;
}
static bool has_head(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_NOHEAD) == 0L;
}
static bool has_horns(const struct permonst * ptr) {
    return num_horns(ptr) > 0;
}
static bool is_whirly(const struct permonst * ptr) {
    return ptr->mlet == S_VORTEX ||
            ptr == &mons[PM_AIR_ELEMENTAL];
}
static bool flaming(const struct permonst * ptr) {
    return ptr == &mons[PM_FIRE_VORTEX] ||
            ptr == &mons[PM_FLAMING_SPHERE] ||
            ptr == &mons[PM_FIRE_ELEMENTAL] ||
            ptr == &mons[PM_SALAMANDER];
}
static bool is_silent(const struct permonst * ptr) {
    return ptr->msound == MS_SILENT;
}
static bool unsolid(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_UNSOLID) != 0L;
}
static bool mindless(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_MINDLESS) != 0L;
}
static bool humanoid(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_HUMANOID) != 0L;
}
static bool is_animal(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_ANIMAL) != 0L;
}
static bool slithy(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_SLITHY) != 0L;
}
static bool is_wooden(const struct permonst * ptr) {
    return ptr == &mons[PM_WOOD_GOLEM];
}
static bool thick_skinned(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_THICK_HIDE) != 0L;
}
static bool lays_eggs(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_OVIPAROUS) != 0L;
}
static bool regenerates(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_REGEN) != 0L;
}
static bool perceives(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_SEE_INVIS) != 0L;
}
static bool can_teleport(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_TPORT) != 0L;
}
static bool control_teleport(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_TPORT_CNTRL) != 0L;
}
static bool telepathic(const struct permonst * ptr) {
    return ptr == &mons[PM_FLOATING_EYE] ||
            ptr == &mons[PM_MIND_FLAYER] ||
            ptr == &mons[PM_MASTER_MIND_FLAYER];
}
static bool is_armed(const struct permonst * ptr) {
    return attacktype(ptr, AT_WEAP);
}
static bool acidic(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_ACID) != 0L;
}
static bool poisonous(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_POIS) != 0L;
}
static bool carnivorous(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_CARNIVORE) != 0L;
}
static bool herbivorous(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_HERBIVORE) != 0L;
}
static bool metallivorous(const struct permonst * ptr) {
    return (ptr->mflags1 & M1_METALLIVORE) != 0L;
}
static bool polyok(const struct permonst * ptr) {
    return (ptr->mflags2 & M2_NOPOLY) == 0L;
}
static bool is_undead(const struct permonst * ptr) {
    return (ptr->mflags2 & M2_UNDEAD) != 0L;
}
static bool is_were(const struct permonst * ptr) {
    return (ptr->mflags2 & M2_WERE) != 0L;
}
static bool is_elf(const struct permonst * ptr) {
    return (ptr->mflags2 & M2_ELF) != 0L;
}
static bool is_dwarf(const struct permonst * ptr) {
    return (ptr->mflags2 & M2_DWARF) != 0L;
}
static bool is_gnome(const struct permonst * ptr) {
    return (ptr->mflags2 & M2_GNOME) != 0L;
}
static bool is_orc(const struct permonst * ptr) {
    return (ptr->mflags2 & M2_ORC) != 0L;
}
static bool is_human(const struct permonst * ptr) {
    return (ptr->mflags2 & M2_HUMAN) != 0L;
}
static bool your_race(const struct permonst * ptr) {
    return (ptr->mflags2 & urace.selfmask) != 0L;
}
static bool is_bat(const struct permonst * ptr) {
    return ptr == &mons[PM_BAT] ||
            ptr == &mons[PM_GIANT_BAT] ||
            ptr == &mons[PM_VAMPIRE_BAT];
}
static bool is_bird(const struct permonst * ptr) {
  return ptr->mlet == S_BAT && !is_bat(ptr);
}
static bool is_giant(const struct permonst * ptr) {
  return (ptr->mflags2 & M2_GIANT) != 0L;
}
static bool is_golem(const struct permonst * ptr) {
  return ptr->mlet == S_GOLEM;
}
static bool is_domestic(const struct permonst * ptr) {
  return (ptr->mflags2 & M2_DOMESTIC) != 0L;
}
static bool is_demon(const struct permonst * ptr) {
  return (ptr->mflags2 & M2_DEMON) != 0L;
}
static bool is_mercenary(const struct permonst * ptr) {
  return (ptr->mflags2 & M2_MERC) != 0L;
}
static bool is_male(const struct permonst * ptr) {
  return (ptr->mflags2 & M2_MALE) != 0L;
}
static bool is_female(const struct permonst * ptr) {
  return (ptr->mflags2 & M2_FEMALE) != 0L;
}
static bool is_neuter(const struct permonst * ptr) {
  return (ptr->mflags2 & M2_NEUTER) != 0L;
}
static bool is_wanderer(const struct permonst * ptr) {
  return (ptr->mflags2 & M2_WANDER) != 0L;
}
static bool always_hostile(const struct permonst * ptr) {
  return (ptr->mflags2 & M2_HOSTILE) != 0L;
}
static bool always_peaceful(const struct permonst * ptr) {
  return (ptr->mflags2 & M2_PEACEFUL) != 0L;
}
static bool race_hostile(const struct permonst * ptr) {
  return (ptr->mflags2 & urace.hatemask) != 0L;
}
static bool race_peaceful(const struct permonst * ptr) {
  return (ptr->mflags2 & urace.lovemask) != 0L;
}
static bool extra_nasty(const struct permonst * ptr) {
  return (ptr->mflags2 & M2_NASTY) != 0L;
}
static bool strongmonst(const struct permonst * ptr) {
  return (ptr->mflags2 & M2_STRONG) != 0L;
}
static bool can_breathe(const struct permonst * ptr) {
  return attacktype(ptr, AT_BREA);
}
static bool cantwield(const struct permonst * ptr) {
  return nohands(ptr) || verysmall(ptr);
}
static bool could_twoweap(const struct permonst * ptr) {
  return ptr->mattk[1].aatyp == AT_WEAP;
}
static bool cantweararm(const struct permonst * ptr) {
  return breakarm(ptr) || sliparm(ptr);
}
static bool throws_rocks(const struct permonst * ptr) {
  return (ptr->mflags2 & M2_ROCKTHROW) != 0L;
}
static bool type_is_pname(const struct permonst * ptr) {
  return (ptr->mflags2 & M2_PNAME) != 0L;
}
static bool is_lord(const struct permonst * ptr) {
  return (ptr->mflags2 & M2_LORD) != 0L;
}
static bool is_prince(const struct permonst * ptr) {
  return (ptr->mflags2 & M2_PRINCE) != 0L;
}
static bool is_ndemon(const struct permonst * ptr) {
  return is_demon(ptr) &&
         (ptr->mflags2 & (M2_LORD|M2_PRINCE)) == 0L;
}
static bool is_dlord(const struct permonst * ptr) {
  return is_demon(ptr) && is_lord(ptr);
}
static bool is_dprince(const struct permonst * ptr) {
  return is_demon(ptr) && is_prince(ptr);
}
static bool is_minion(const struct permonst * ptr) {
  return ptr->mflags2 & M2_MINION;
}
static bool likes_gold(const struct permonst * ptr) {
  return (ptr->mflags2 & M2_GREEDY) != 0L;
}
static bool likes_gems(const struct permonst * ptr) {
  return (ptr->mflags2 & M2_JEWELS) != 0L;
}
static bool likes_objs(const struct permonst * ptr){
  return (ptr->mflags2 & M2_COLLECT) != 0L ||
         is_armed(ptr);
}
static bool likes_magic(const struct permonst * ptr) {
  return (ptr->mflags2 & M2_MAGIC) != 0L;
}
static bool webmaker(const struct permonst * ptr) {
  return ptr == &mons[PM_CAVE_SPIDER] ||
         ptr == &mons[PM_GIANT_SPIDER];
}
static bool is_unicorn(const struct permonst * ptr) {
  return ptr->mlet == S_UNICORN && likes_gems(ptr);
}
static bool is_longworm(const struct permonst * ptr) {
  return ptr == &mons[PM_BABY_LONG_WORM] ||
         ptr == &mons[PM_LONG_WORM] ||
         ptr == &mons[PM_LONG_WORM_TAIL];
}
static bool is_covetous(const struct permonst * ptr) {
  return !!((ptr->mflags3 & M3_COVETOUS));
}
static bool infravision(const struct permonst * ptr) {
  return !!((ptr->mflags3 & M3_INFRAVISION));
}
static bool infravisible(const struct permonst * ptr) {
  return !!((ptr->mflags3 & M3_INFRAVISIBLE));
}
static bool is_mplayer(const struct permonst * ptr) {
  return ptr >= &mons[PM_ARCHEOLOGIST] &&
         ptr <= &mons[PM_WIZARD];
}
static bool is_rider(const struct permonst * ptr) {
  return ptr == &mons[PM_DEATH] ||
         ptr == &mons[PM_FAMINE] ||
         ptr == &mons[PM_PESTILENCE];
}
static bool is_placeholder(const struct permonst * ptr) {
  return ptr == &mons[PM_ORC] ||
         ptr == &mons[PM_GIANT] ||
         ptr == &mons[PM_ELF] ||
         ptr == &mons[PM_HUMAN];
}
/* return true if the monster tends to revive */
static bool is_reviver(const struct permonst * ptr) {
  return is_rider(ptr) || ptr->mlet == S_TROLL;
}

/* this returns the light's range, or 0 if none; if we add more light emitting
   monsters, we'll likely have to add a new light range field to mons[] */
static int emits_light(const struct permonst * ptr) {
  return (ptr->mlet == S_LIGHT ||
          ptr == &mons[PM_FLAMING_SPHERE] ||
          ptr == &mons[PM_SHOCKING_SPHERE] ||
          ptr == &mons[PM_FIRE_VORTEX]) ? 1 :
         (ptr == &mons[PM_FIRE_ELEMENTAL]) ? 1 : 0;
/*      [note: the light ranges above were reduced to 1 for performance...] */
}
static bool likes_lava(const struct permonst * ptr) {
  return ptr == &mons[PM_FIRE_ELEMENTAL] ||
         ptr == &mons[PM_SALAMANDER];
}
static bool pm_invisible(const struct permonst * ptr) {
  return ptr == &mons[PM_STALKER] ||
         ptr == &mons[PM_BLACK_LIGHT];
}

static bool likes_fire(const struct permonst * ptr) {
  return ptr == &mons[PM_FIRE_VORTEX] ||
         ptr == &mons[PM_FLAMING_SPHERE] ||
         likes_lava(ptr);
}

static bool touch_petrifies(const struct permonst * ptr) {
	return ptr == &mons[PM_COCKATRICE] ||
         ptr == &mons[PM_CHICKATRICE];
}

static bool is_mind_flayer(const struct permonst * ptr) {
    return (ptr == &mons[PM_MIND_FLAYER] ||
            ptr == &mons[PM_MASTER_MIND_FLAYER]);
}

static bool nonliving(const struct permonst * ptr) {
    return is_golem(ptr) ||
           is_undead(ptr) ||
           ptr->mlet == S_VORTEX ||
           ptr == &mons[PM_MANES];
}

/* Used for conduct with corpses, tins, and digestion attacks */
/* G_NOCORPSE monsters might still be swallowed as a purple worm */
/* Maybe someday this could be in mflags... */
static bool vegan(const struct permonst * ptr) {
    return ptr->mlet == S_BLOB ||
           ptr->mlet == S_JELLY ||
           ptr->mlet == S_FUNGUS ||
           ptr->mlet == S_VORTEX ||
           ptr->mlet == S_LIGHT ||
           (ptr->mlet == S_ELEMENTAL && ptr != &mons[PM_STALKER]) ||
           (ptr->mlet == S_GOLEM && ptr != &mons[PM_FLESH_GOLEM] && ptr != &mons[PM_LEATHER_GOLEM]) ||
           noncorporeal(ptr);
}
static bool vegetarian(const struct permonst * ptr) {
    return vegan(ptr) ||
           (ptr->mlet == S_PUDDING && ptr != &mons[PM_BLACK_PUDDING]);
}

static bool befriend_with_obj(const struct permonst * ptr, const struct obj * obj) {
    return obj->oclass == FOOD_CLASS && is_domestic(ptr);
}

static bool is_lminion(struct monst * mon) {
    return is_minion(mon->data) &&
           mon->data->maligntyp >= A_COALIGNED &&
           (mon->data != &mons[PM_ANGEL] || EPRI(mon)->shralign > 0);
}

#endif /* MONDATA_H */
