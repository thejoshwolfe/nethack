/* See LICENSE in the root of this project for change info */
#ifndef MONDATA_H
#define MONDATA_H

#include "global.h"
#include "pm.h"
#include "monsym.h"
#include "you.h"
#include "extern.h"

#define verysmall(ptr)          ((ptr)->msize < MZ_SMALL)
#define bigmonst(ptr)           ((ptr)->msize >= MZ_LARGE)

#define pm_resistance(ptr,typ)  (((ptr)->mresists & (typ)) != 0)

#define resists_fire(mon)       (((mon)->mintrinsics & MR_FIRE) != 0)
#define resists_cold(mon)       (((mon)->mintrinsics & MR_COLD) != 0)
#define resists_sleep(mon)      (((mon)->mintrinsics & MR_SLEEP) != 0)
#define resists_disint(mon)     (((mon)->mintrinsics & MR_DISINT) != 0)
#define resists_elec(mon)       (((mon)->mintrinsics & MR_ELEC) != 0)
#define resists_poison(mon)     (((mon)->mintrinsics & MR_POISON) != 0)
#define resists_acid(mon)       (((mon)->mintrinsics & MR_ACID) != 0)
#define resists_ston(mon)       (((mon)->mintrinsics & MR_STONE) != 0)

#define is_lminion(mon)         (is_minion((mon)->data) && \
                                 (mon)->data->maligntyp >= A_COALIGNED && \
                                 ((mon)->data != &mons[PM_ANGEL] || \
                                  EPRI(mon)->shralign > 0))

#define is_flyer(ptr)           (((ptr)->mflags1 & M1_FLY) != 0L)
#define is_floater(ptr)         ((ptr)->mlet == S_EYE)
#define is_clinger(ptr)         (((ptr)->mflags1 & M1_CLING) != 0L)
#define is_swimmer(ptr)         (((ptr)->mflags1 & M1_SWIM) != 0L)
#define breathless(ptr)         (((ptr)->mflags1 & M1_BREATHLESS) != 0L)
#define amphibious(ptr)         (((ptr)->mflags1 & (M1_AMPHIBIOUS | M1_BREATHLESS)) != 0L)
#define passes_walls(ptr)       (((ptr)->mflags1 & M1_WALLWALK) != 0L)
#define amorphous(ptr)          (((ptr)->mflags1 & M1_AMORPHOUS) != 0L)
#define noncorporeal(ptr)       ((ptr)->mlet == S_GHOST)
#define tunnels(ptr)            (((ptr)->mflags1 & M1_TUNNEL) != 0L)
#define needspick(ptr)          (((ptr)->mflags1 & M1_NEEDPICK) != 0L)
#define hides_under(ptr)        (((ptr)->mflags1 & M1_CONCEAL) != 0L)
#define is_hider(ptr)           (((ptr)->mflags1 & M1_HIDE) != 0L)
#define haseyes(ptr)            (((ptr)->mflags1 & M1_NOEYES) == 0L)
#define eyecount(ptr)           (!haseyes(ptr) ? 0 : \
                                 ((ptr) == &mons[PM_CYCLOPS] || \
                                  (ptr) == &mons[PM_FLOATING_EYE]) ? 1 : 2)
#define nohands(ptr)            (((ptr)->mflags1 & M1_NOHANDS) != 0L)
#define nolimbs(ptr)            (((ptr)->mflags1 & M1_NOLIMBS) == M1_NOLIMBS)
#define notake(ptr)             (((ptr)->mflags1 & M1_NOTAKE) != 0L)
#define has_head(ptr)           (((ptr)->mflags1 & M1_NOHEAD) == 0L)
#define has_horns(ptr)          (num_horns(ptr) > 0)
#define is_whirly(ptr)          ((ptr)->mlet == S_VORTEX || \
                                 (ptr) == &mons[PM_AIR_ELEMENTAL])
#define flaming(ptr)            ((ptr) == &mons[PM_FIRE_VORTEX] || \
                                 (ptr) == &mons[PM_FLAMING_SPHERE] || \
                                 (ptr) == &mons[PM_FIRE_ELEMENTAL] || \
                                 (ptr) == &mons[PM_SALAMANDER])
#define is_silent(ptr)          ((ptr)->msound == MS_SILENT)
#define unsolid(ptr)            (((ptr)->mflags1 & M1_UNSOLID) != 0L)
#define mindless(ptr)           (((ptr)->mflags1 & M1_MINDLESS) != 0L)
#define humanoid(ptr)           (((ptr)->mflags1 & M1_HUMANOID) != 0L)
#define is_animal(ptr)          (((ptr)->mflags1 & M1_ANIMAL) != 0L)
#define slithy(ptr)             (((ptr)->mflags1 & M1_SLITHY) != 0L)
#define is_wooden(ptr)          ((ptr) == &mons[PM_WOOD_GOLEM])
#define thick_skinned(ptr)      (((ptr)->mflags1 & M1_THICK_HIDE) != 0L)
#define lays_eggs(ptr)          (((ptr)->mflags1 & M1_OVIPAROUS) != 0L)
#define regenerates(ptr)        (((ptr)->mflags1 & M1_REGEN) != 0L)
#define perceives(ptr)          (((ptr)->mflags1 & M1_SEE_INVIS) != 0L)
#define can_teleport(ptr)       (((ptr)->mflags1 & M1_TPORT) != 0L)
#define control_teleport(ptr)   (((ptr)->mflags1 & M1_TPORT_CNTRL) != 0L)
#define telepathic(ptr)         ((ptr) == &mons[PM_FLOATING_EYE] || \
                                 (ptr) == &mons[PM_MIND_FLAYER] || \
                                 (ptr) == &mons[PM_MASTER_MIND_FLAYER])
#define is_armed(ptr)           attacktype(ptr, AT_WEAP)
#define acidic(ptr)             (((ptr)->mflags1 & M1_ACID) != 0L)
#define poisonous(ptr)          (((ptr)->mflags1 & M1_POIS) != 0L)
#define carnivorous(ptr)        (((ptr)->mflags1 & M1_CARNIVORE) != 0L)
#define herbivorous(ptr)        (((ptr)->mflags1 & M1_HERBIVORE) != 0L)
#define metallivorous(ptr)      (((ptr)->mflags1 & M1_METALLIVORE) != 0L)
#define polyok(ptr)             (((ptr)->mflags2 & M2_NOPOLY) == 0L)
#define is_undead(ptr)          (((ptr)->mflags2 & M2_UNDEAD) != 0L)
#define is_were(ptr)            (((ptr)->mflags2 & M2_WERE) != 0L)
#define is_elf(ptr)             (((ptr)->mflags2 & M2_ELF) != 0L)
#define is_dwarf(ptr)           (((ptr)->mflags2 & M2_DWARF) != 0L)
#define is_gnome(ptr)           (((ptr)->mflags2 & M2_GNOME) != 0L)
#define is_orc(ptr)             (((ptr)->mflags2 & M2_ORC) != 0L)
#define is_human(ptr)           (((ptr)->mflags2 & M2_HUMAN) != 0L)
#define your_race(ptr)          (((ptr)->mflags2 & urace.selfmask) != 0L)
#define is_bat(ptr)             ((ptr) == &mons[PM_BAT] || \
                                 (ptr) == &mons[PM_GIANT_BAT] || \
                                 (ptr) == &mons[PM_VAMPIRE_BAT])
static bool is_bird(struct permonst * ptr) {
  return ptr->mlet == S_BAT && !is_bat(ptr);
}
static bool is_giant(struct permonst * ptr) {
  return (ptr->mflags2 & M2_GIANT) != 0L;
}
static bool is_golem(struct permonst * ptr) {
  return ptr->mlet == S_GOLEM;
}
static bool is_domestic(struct permonst * ptr) {
  return (ptr->mflags2 & M2_DOMESTIC) != 0L;
}
static bool is_demon(struct permonst * ptr) {
  return (ptr->mflags2 & M2_DEMON) != 0L;
}
static bool is_mercenary(struct permonst * ptr) {
  return (ptr->mflags2 & M2_MERC) != 0L;
}
static bool is_male(struct permonst * ptr) {
  return (ptr->mflags2 & M2_MALE) != 0L;
}
static bool is_female(struct permonst * ptr) {
  return (ptr->mflags2 & M2_FEMALE) != 0L;
}
static bool is_neuter(struct permonst * ptr) {
  return (ptr->mflags2 & M2_NEUTER) != 0L;
}
static bool is_wanderer(struct permonst * ptr) {
  return (ptr->mflags2 & M2_WANDER) != 0L;
}
static bool always_hostile(struct permonst * ptr) {
  return (ptr->mflags2 & M2_HOSTILE) != 0L;
}
static bool always_peaceful(struct permonst * ptr) {
  return (ptr->mflags2 & M2_PEACEFUL) != 0L;
}
static bool race_hostile(struct permonst * ptr) {
  return (ptr->mflags2 & urace.hatemask) != 0L;
}
static bool race_peaceful(struct permonst * ptr) {
  return (ptr->mflags2 & urace.lovemask) != 0L;
}
static bool extra_nasty(struct permonst * ptr) {
  return (ptr->mflags2 & M2_NASTY) != 0L;
}
static bool strongmonst(struct permonst * ptr) {
  return (ptr->mflags2 & M2_STRONG) != 0L;
}
static bool can_breathe(struct permonst * ptr) {
  return attacktype(ptr, AT_BREA);
}
static bool cantwield(struct permonst * ptr) {
  return nohands(ptr) || verysmall(ptr);
}
static bool could_twoweap(struct permonst * ptr) {
  return ptr->mattk[1].aatyp == AT_WEAP;
}
static bool cantweararm(struct permonst * ptr) {
  return breakarm(ptr) || sliparm(ptr);
}
static bool throws_rocks(struct permonst * ptr) {
  return (ptr->mflags2 & M2_ROCKTHROW) != 0L;
}
static bool type_is_pname(struct permonst * ptr) {
  return (ptr->mflags2 & M2_PNAME) != 0L;
}
static bool is_lord(struct permonst * ptr) {
  return (ptr->mflags2 & M2_LORD) != 0L;
}
static bool is_prince(struct permonst * ptr) {
  return (ptr->mflags2 & M2_PRINCE) != 0L;
}
static bool is_ndemon(struct permonst * ptr) {
  return is_demon(ptr) &&
         (ptr->mflags2 & (M2_LORD|M2_PRINCE)) == 0L;
}
static bool is_dlord(struct permonst * ptr) {
  return is_demon(ptr) && is_lord(ptr);
}
static bool is_dprince(struct permonst * ptr) {
  return is_demon(ptr) && is_prince(ptr);
}
static bool is_minion(struct permonst * ptr) {
  return ptr->mflags2 & M2_MINION;
}
static bool likes_gold(struct permonst * ptr) {
  return (ptr->mflags2 & M2_GREEDY) != 0L;
}
static bool likes_gems(struct permonst * ptr) {
  return (ptr->mflags2 & M2_JEWELS) != 0L;
}
static bool likes_objs(struct permonst * ptr){
  return (ptr->mflags2 & M2_COLLECT) != 0L ||
         is_armed(ptr);
}
static bool likes_magic(struct permonst * ptr) {
  return (ptr->mflags2 & M2_MAGIC) != 0L;
}
static bool webmaker(struct permonst * ptr) {
  return ptr == &mons[PM_CAVE_SPIDER] ||
         ptr == &mons[PM_GIANT_SPIDER];
}
static bool is_unicorn(struct permonst * ptr) {
  return ptr->mlet == S_UNICORN && likes_gems(ptr);
}
static bool is_longworm(struct permonst * ptr) {
  return ptr == &mons[PM_BABY_LONG_WORM] ||
         ptr == &mons[PM_LONG_WORM] ||
         ptr == &mons[PM_LONG_WORM_TAIL];
}
static bool is_covetous(struct permonst * ptr) {
  return !!((ptr->mflags3 & M3_COVETOUS));
}
static bool infravision(struct permonst * ptr) {
  return !!((ptr->mflags3 & M3_INFRAVISION));
}
static bool infravisible(struct permonst * ptr) {
  return !!((ptr->mflags3 & M3_INFRAVISIBLE));
}
static bool is_mplayer(struct permonst * ptr) {
  return ptr >= &mons[PM_ARCHEOLOGIST] &&
         ptr <= &mons[PM_WIZARD];
}
static bool is_rider(struct permonst * ptr) {
  return ptr == &mons[PM_DEATH] ||
         ptr == &mons[PM_FAMINE] ||
         ptr == &mons[PM_PESTILENCE];
}
static bool is_placeholder(struct permonst * ptr) {
  return ptr == &mons[PM_ORC] ||
         ptr == &mons[PM_GIANT] ||
         ptr == &mons[PM_ELF] ||
         ptr == &mons[PM_HUMAN];
}
/* return true if the monster tends to revive */
static bool is_reviver(struct permonst * ptr) {
  return is_rider(ptr) || ptr->mlet == S_TROLL;
}

/* this returns the light's range, or 0 if none; if we add more light emitting
   monsters, we'll likely have to add a new light range field to mons[] */
static int emits_light(struct permonst * ptr) {
  return (ptr->mlet == S_LIGHT ||
          ptr == &mons[PM_FLAMING_SPHERE] ||
          ptr == &mons[PM_SHOCKING_SPHERE] ||
          ptr == &mons[PM_FIRE_VORTEX]) ? 1 :
         (ptr == &mons[PM_FIRE_ELEMENTAL]) ? 1 : 0;
/*      [note: the light ranges above were reduced to 1 for performance...] */
}
static bool likes_lava(struct permonst * ptr) {
  return ptr == &mons[PM_FIRE_ELEMENTAL] ||
         ptr == &mons[PM_SALAMANDER];
}
static bool pm_invisible(struct permonst * ptr) {
  return ptr == &mons[PM_STALKER] ||
         ptr == &mons[PM_BLACK_LIGHT];
}

static bool likes_fire(struct permonst * ptr) {
  return ptr == &mons[PM_FIRE_VORTEX] ||
         ptr == &mons[PM_FLAMING_SPHERE] ||
         likes_lava(ptr);
}

static bool touch_petrifies(struct permonst * ptr) {
	return ptr == &mons[PM_COCKATRICE] ||
         ptr == &mons[PM_CHICKATRICE];
}

#define is_mind_flayer(ptr)     ((ptr) == &mons[PM_MIND_FLAYER] || \
                                 (ptr) == &mons[PM_MASTER_MIND_FLAYER])

#define nonliving(ptr)          (is_golem(ptr) || is_undead(ptr) || \
                                 (ptr)->mlet == S_VORTEX || \
                                 (ptr) == &mons[PM_MANES])

/* Used for conduct with corpses, tins, and digestion attacks */
/* G_NOCORPSE monsters might still be swallowed as a purple worm */
/* Maybe someday this could be in mflags... */
#define vegan(ptr)              ((ptr)->mlet == S_BLOB || \
                                 (ptr)->mlet == S_JELLY ||            \
                                 (ptr)->mlet == S_FUNGUS ||           \
                                 (ptr)->mlet == S_VORTEX ||           \
                                 (ptr)->mlet == S_LIGHT ||            \
                                ((ptr)->mlet == S_ELEMENTAL &&        \
                                 (ptr) != &mons[PM_STALKER]) ||       \
                                ((ptr)->mlet == S_GOLEM &&            \
                                 (ptr) != &mons[PM_FLESH_GOLEM] &&    \
                                 (ptr) != &mons[PM_LEATHER_GOLEM]) || \
                                 noncorporeal(ptr))
#define vegetarian(ptr)         (vegan(ptr) || \
                                ((ptr)->mlet == S_PUDDING &&         \
                                 (ptr) != &mons[PM_BLACK_PUDDING]))

#define befriend_with_obj(ptr, obj) ((obj)->oclass == FOOD_CLASS && \
                                     is_domestic(ptr))

#endif /* MONDATA_H */
