/* See LICENSE in the root of this project for change info */

#include "hack.h"

#include <stdio.h>
#include <string.h>

#include "dungeon_util.h"
#include "apply.h"
#include "artifact_names.h"
#include "attrib.h"
#include "ball.h"
#include "cmd.h"
#include "dbridge.h"
#include "decl.h"
#include "dig.h"
#include "display.h"
#include "do.h"
#include "do_name.h"
#include "do_wear.h"
#include "dog.h"
#include "dungeon.h"
#include "eat.h"
#include "end.h"
#include "engrave.h"
#include "flag.h"
#include "global.h"
#include "invent.h"
#include "mhitm.h"
#include "mhitu.h"
#include "mkmaze.h"
#include "mkobj.h"
#include "mkroom.h"
#include "mon.h"
#include "monattk.h"
#include "mondata.h"
#include "monflag.h"
#include "monmove.h"
#include "monst.h"
#include "monsym.h"
#include "o_init.h"
#include "obj.h"
#include "objclass.h"
#include "objnam.h"
#include "onames.h"
#include "permonst.h"
#include "pickup.h"
#include "pline.h"
#include "pm.h"
#include "polyself.h"
#include "priest.h"
#include "prop.h"
#include "region.h"
#include "rm.h"
#include "role.h"
#include "shk.h"
#include "skills.h"
#include "sounds.h"
#include "steed.h"
#include "teleport.h"
#include "timeout.h"
#include "trap.h"
#include "uhitm.h"
#include "util.h"
#include "vision.h"
#include "youprop.h"

static const char *killer_method_enum_str[] = {
    "DIED",
    "CHOKING",
    "POISONING",
    "STARVING",
    "DROWNING",
    "BURNING",
    "DISSOLVED",
    "CRUSHING",
    "STONING",
    "TURNED_SLIME",
    "GENOCIDED",
    "PANICKED",
    "TRICKED",
    "QUIT",
    "ESCAPED",
    "ASCENDED",
    "ELECTRIC_SHOCK",
    "TOWER_OF_FLAME",
    "BOILING_WATER",
    "DANGEROUS_WINDS",
    "LAND_MINE",
    "FELL_INTO_PIT",
    "FELL_INTO_PIT_OF_IRON_SPIKES",
    "RUSTING_AWAY",
    "SUFFOCATION",
    "STRANGULATION",
    "TURNED_INTO_GREEN_SLIME",
    "PETRIFICATION",
    "TELEPORTED_OUT_OF_THE_DUNGEON_AND_FELL_TO_DEATH",
    "WENT_TO_HEAVEN_PREMATURELY",
    "COMMITTED_SUICIDE",
    "SLIPPED_WHILE_MOUNTING_M",
    "ZAPPED_SELF_WITH_SPELL",
    "EXPLODING_RUNE",
    "CONTACT_POISONED_SPELLBOOK",
    "ELECTRIC_CHAIR",
    "CURSED_THRONE",
    "SITTING_ON_LAVA",
    "SITTING_IN_LAVA",
    "SITTING_ON_IRON_SPIKE",
    "GAS_CLOUD",
    "IMPERIOUS_ORDER",
    "GENOCIDAL_CONFUSION",
    "SCROLL_OF_GENOCIDE",
    "SCROLL_OF_FIRE",
    "EXPLODING_RING",
    "RESIDUAL_UNDEAD_TURNING",
    "ALCHEMIC_BLAST",
    "ELEMENTARY_CHEMISTRY",
    "THROWN_POTION",
    "POTION_OF_ACID",
    "BURNING_POTION_OF_OIL",
    "COLLIDING_WITH_CEILING",
    "CONTAMINATED_POTION",
    "CONTAMINATED_TAP_WATER",
    "MILDLY_CONTAMINATED_POTION",
    "POTION_UNHOLY_WATER",
    "POTION_HOLY_WATER",
    "DELIBERATELY_MEETING_MEDUSA_GAZE",
    "WHILE_STUCK_IN_CREATURE_FORM",
    "SYSTEM_SHOCK",
    "UNSUCCESSFUL_POLYMORPH",
    "SELF_GENOCIDE",
    "MAGICAL_EXPLOSION",
    "CARNIVOROUS_BAG",
    "USING_MAGIC_HORN_ON_SELF",
    "FELL_INTO_CHASM",
    "SCROLL_OF_EARTH",
    "WAND",
    "PSYCHIC_BLAST",
    "TODO",
    "BRAINLESSNESS",
    "TOUCH_OF_DEATH",
    "FELL_ON_SINK",
    "SIP_BOILING_WATER",
    "CONTAMINATED_WATER",
    "UNREFRIGERATED_JUICE",
    "MONSTER",
    "EXHAUSTION",
    "CADAVER",
    "POISONOUS_CORPSE",
    "ACIDIC_CORPSE",
    "ATE_HORSEMAN",
    "TASTING_O_MEAT",
    "O_EGG",
    "ROTTEN_ROYAL_JELLY",
    "CHOKE_QUICK_SNACK",
    "CHOKE_ON_FOOD",
    "O",
    "FALLING_OBJECT",
    "ELEMENTARY_PHYSICS",
    "WEDGING_INTO_NARROW_CREVICE",
    "TOUCH_EDGE_UNIVERSE",
    "BUMP_INTO_BOULDER",
    "CRASH_INTO_IRON_BARS",
    "BUMP_INTO_TREE",
    "BUMP_INTO_WALL",
    "BUMP_INTO_DOOR",
    "O_CORPSE",
    "KICKING_O",
    "KICKING_DOOR",
    "KICKING_TREE",
    "KICKING_WALL",
    "KICKING_ROCK",
    "KICKING_THRONE",
    "KICKING_FOUNTAIN",
    "KICKING_HEADSTONE",
    "KICKING_SINK",
    "KICKING_ALTAR",
    "KICKING_DRAWBRIDGE",
    "KICKING_STAIRS",
    "KICKING_LADDER",
    "KICKING_IRON_BAR",
    "KICKING_SOMETHING_WEIRD",
    "KICKING_O_WITHOUT_BOOTS",
    "FALLING_DOWNSTAIRS",
    "SQUISHED_UNDER_BOULDER",
    "AXING_HARD_OBJECT",
    "YOUR_OWN_O",
    "EXPLODING_CRYSTAL_BALL",
    "FALLING_DRAWBRIDGE",
    "CLOSING_DRAWBRIDGE",
    "FELL_FROM_DRAWBRIDGE",
    "EXPLODING_DRAWBRIDGE",
    "COLLAPSING_DRAWBRIDGE",
    "RIDING_ACCIDENT",
    "IRON_BALL_COLLISON",
    "DRAGGED_DOWNSTAIRS_IRON_BALL",
    "LEG_DAMAGE_PULLED_BEAR_TRAP",
    "CRUNCHED_IN_HEAD_BY_IRON_BALL",
    "TOUCH_ARTIFACT",
    "KILLED_SELF_BREAK_WAND",
    "GRAPPLING_HOOK_SELF",
    "HIT_SELF_BULLWHIP",
    "JUMP_BEAR_TRAP",
    "EXPLOSION",
    "MOLTEN_LAVA",
    "EXPLODING_WAND",
    "SELF_WITH_WAND",
    "SELF_WITH_DEATH_RAY",
    "FALLING_ROCK",
    "FLASH_TYPE",
};

static int wc;  /* current weight_cap(); valid after call to inv_weight() */

bool revive_nasty(int x, int y, const char *msg) {
    struct obj *otmp, *otmp2;
    struct monst *mtmp;
    coord cc;
    bool revived = false;

    for (otmp = level.objects[x][y]; otmp; otmp = otmp2) {
        otmp2 = otmp->nexthere;
        if (otmp->otyp == CORPSE && (is_rider(&mons[otmp->corpsenm]) || otmp->corpsenm == PM_WIZARD_OF_YENDOR)) {
            /* move any living monster already at that location */
            if ((mtmp = m_at(x, y)) && enexto(&cc, x, y, mtmp->data))
                rloc_to(mtmp, cc.x, cc.y);
            if (msg)
                Norep("%s", msg);
            revived = revive_corpse(otmp);
        }
    }

    /* this location might not be safe, if not, move revived monster */
    if (revived) {
        mtmp = m_at(x, y);
        if (mtmp && !goodpos(x, y, mtmp, 0) && enexto(&cc, x, y, mtmp->data)) {
            rloc_to(mtmp, cc.x, cc.y);
        }
        /* else impossible? */
    }

    return revived;
}

/* stop running if we see something interesting */
/* turn around a corner if that is the only way we can proceed */
/* do not turn left or right twice */
void lookaround(void) {
    int x, y, i, x0 = 0, y0 = 0, m0 = 1, i0 = 9;
    int corrct = 0, noturn = 0;
    struct monst *mtmp;
    struct trap *trap;

    /* Grid bugs stop if trying to move diagonal, even if blind.  Maybe */
    /* they polymorphed while in the middle of a long move. */
    if (u.umonnum == PM_GRID_BUG && u.dx && u.dy) {
        nomul(0);
        return;
    }

    if (Blind || flags.run == 0)
        return;
    for (x = u.ux - 1; x <= u.ux + 1; x++)
        for (y = u.uy - 1; y <= u.uy + 1; y++) {
            if (!isok(x, y))
                continue;

            if (u.umonnum == PM_GRID_BUG && x != u.ux && y != u.uy)
                continue;

            if (x == u.ux && y == u.uy)
                continue;

            if ((mtmp = m_at(x, y)) && mtmp->m_ap_type != M_AP_FURNITURE && mtmp->m_ap_type != M_AP_OBJECT && (!mtmp->minvis || See_invisible()) && !mtmp->mundetected) {
                if ((flags.run != 1 && !mtmp->mtame) || (x == u.ux + u.dx && y == u.uy + u.dy))
                    goto stop;
            }

            if (levl[x][y].typ == STONE)
                continue;
            if (x == u.ux - u.dx && y == u.uy - u.dy)
                continue;

            if (IS_ROCK(levl[x][y].typ) || (levl[x][y].typ == ROOM) || IS_AIR(levl[x][y].typ))
                continue;
            else if (closed_door(x, y) || (mtmp && mtmp->m_ap_type == M_AP_FURNITURE && (mtmp->mappearance == S_hcdoor || mtmp->mappearance == S_vcdoor))) {
                if (x != u.ux && y != u.uy)
                    continue;
                if (flags.run != 1)
                    goto stop;
                goto bcorr;
            } else if (levl[x][y].typ == CORR) {
                bcorr: if (levl[u.ux][u.uy].typ != ROOM) {
                    if (flags.run == 1 || flags.run == 3 || flags.run == 8) {
                        i = dist2(x, y, u.ux + u.dx, u.uy + u.dy);
                        if (i > 2)
                            continue;
                        if (corrct == 1 && dist2(x, y, x0, y0) != 1)
                            noturn = 1;
                        if (i < i0) {
                            i0 = i;
                            x0 = x;
                            y0 = y;
                            m0 = mtmp ? 1 : 0;
                        }
                    }
                    corrct++;
                }
                continue;
            } else if ((trap = t_at(x, y)) && trap->tseen) {
                if (flags.run == 1)
                    goto bcorr;
                /* if you must */
                if (x == u.ux + u.dx && y == u.uy + u.dy)
                    goto stop;
                continue;
            } else if (is_pool(x, y) || is_lava(x, y)) {
                /* water and lava only stop you if directly in front, and stop
                 * you even if you are running
                 */
                if (!Levitation && !Flying && !is_clinger(youmonst.data) && x == u.ux + u.dx && y == u.uy + u.dy)
                    /* No Wwalking check; otherwise they'd be able
                     * to test boots by trying to SHIFT-direction
                     * into a pool and seeing if the game allowed it
                     */
                    goto stop;
                continue;
            } else { /* e.g. objects or trap or stairs */
                if (flags.run == 1)
                    goto bcorr;
                if (flags.run == 8)
                    continue;
                if (mtmp)
                    continue; /* d */
                if (((x == u.ux - u.dx) && (y != u.uy + u.dy)) || ((y == u.uy - u.dy) && (x != u.ux + u.dx)))
                    continue;
            }
            stop: nomul(0);
            return;
        } /* end for loops */

    if (corrct > 1 && flags.run == 2)
        goto stop;
    if ((flags.run == 1 || flags.run == 3 || flags.run == 8) && !noturn && !m0 && i0 && (corrct == 1 || (corrct == 2 && i0 == 1))) {
        /* make sure that we do not turn too far */
        if (i0 == 2) {
            if (u.dx == y0 - u.uy && u.dy == u.ux - x0)
                i = 2; /* straight turn right */
            else
                i = -2; /* straight turn left */
        } else if (u.dx && u.dy) {
            if ((u.dx == u.dy && y0 == u.uy) || (u.dx != u.dy && y0 != u.uy))
                i = -1; /* half turn left */
            else
                i = 1; /* half turn right */
        } else {
            if ((x0 - u.ux == y0 - u.uy && !u.dy) || (x0 - u.ux != y0 - u.uy && u.dy))
                i = 1; /* half turn right */
            else
                i = -1; /* half turn left */
        }

        i += u.last_str_turn;
        if (i <= 2 && i >= -2) {
            u.last_str_turn = i;
            u.dx = x0 - u.ux;
            u.dy = y0 - u.uy;
        }
    }
}

/* something like lookaround, but we are not running */
/* react only to monsters that might hit us */
int monster_nearby(void) {
    int x, y;
    struct monst *mtmp;

    /* Also see the similar check in dochugw() in monmove.c */
    for (x = u.ux - 1; x <= u.ux + 1; x++)
        for (y = u.uy - 1; y <= u.uy + 1; y++) {
            if (!isok(x, y))
                continue;
            if (x == u.ux && y == u.uy)
                continue;
            if ((mtmp = m_at(x, y)) && mtmp->m_ap_type != M_AP_FURNITURE && mtmp->m_ap_type != M_AP_OBJECT && (!mtmp->mpeaceful || Hallucination()) && (!is_hider(mtmp->data) || !mtmp->mundetected) && !noattacks(mtmp->data) && mtmp->mcanmove && !mtmp->msleeping && /* aplvax!jcn */
            !onscary(u.ux, u.uy, mtmp) && canspotmon(mtmp))
                return 1;
        }
    return 0;
}

void nomul(int nval) {
    if (multi < nval)
        return; /* This is a bug fix by ab@unido */
    u.uinvulnerable = false; /* Kludge to avoid ctrl-C bug -dlc */
    u.usleep = 0;
    multi = nval;
    flags.mv = flags.run = 0;
}

/* called when a non-movement, multi-turn action has completed */
void unmul(const char *msg_override) {
    multi = 0; /* caller will usually have done this already */
    if (msg_override)
        nomovemsg = msg_override;
    else if (!nomovemsg)
        nomovemsg = You_can_move_again;
    if (*nomovemsg)
        plines(nomovemsg);
    nomovemsg = 0;
    u.usleep = 0;
    if (afternmv)
        (*afternmv)();
    afternmv = 0;
}

static void maybe_wail(void) {
    static short powers[] = { TELEPORT, SEE_INVIS, POISON_RES, COLD_RES,
                              SHOCK_RES, FIRE_RES, SLEEP_RES, DISINT_RES,
                              TELEPORT_CONTROL, STEALTH, FAST, INVIS };

    if (moves <= wailmsg + 50)
        return;

    wailmsg = moves;
    if (Role_if(PM_WIZARD) || Race_if(PM_ELF) || Role_if(PM_VALKYRIE)) {
        const char *who;
        int i, powercnt;

        who = (Role_if(PM_WIZARD) || Role_if(PM_VALKYRIE)) ? urole.name.m : "Elf";
        if (u.uhp == 1) {
            pline("%s is about to die.", who);
        } else {
            for (i = 0, powercnt = 0; i < SIZE(powers); ++i)
                if (u.uprops[powers[i]].intrinsic & INTRINSIC)
                    ++powercnt;

            pline(powercnt >= 4 ? "%s, all your powers will be lost..." : "%s, your life force is running out.", who);
        }
    } else {
        You_hear(u.uhp == 1 ? "the wailing of the Banshee..." : "the howling of the CwnAnnwn...");
    }
}

void losehp(int n, struct Killer k) {
    if (Upolyd) {
        u.mh -= n;
        if (u.mhmax < u.mh)
            u.mhmax = u.mh;
        if (u.mh < 1)
            rehumanize();
        else if (n > 0 && u.mh * 10 < u.mhmax && Unchanging)
            maybe_wail();
        return;
    }

    u.uhp -= n;
    if (u.uhp > u.uhpmax)
        u.uhpmax = u.uhp; /* perhaps n was negative */
    if (u.uhp < 1) {
        killer = k; /* the thing that killed you */
        You("die...");
        done(DIED);
    } else if (n > 0 && u.uhp * 10 < u.uhpmax) {
        maybe_wail();
    }
}

int weight_cap(void) {
    long carrcap;

    carrcap = 25 * (ACURRSTR + ACURR(A_CON)) + 50;
    if (Upolyd) {
        /* consistent with can_carry() in mon.c */
        if (youmonst.data->mlet == S_NYMPH)
            carrcap = MAX_CARR_CAP;
        else if (!youmonst.data->cwt)
            carrcap = (carrcap * (long)youmonst.data->msize) / MZ_HUMAN;
        else if (!strongmonst(youmonst.data) || (strongmonst(youmonst.data) && (youmonst.data->cwt > WT_HUMAN)))
            carrcap = (carrcap * (long)youmonst.data->cwt / WT_HUMAN);
    }

    if (Levitation || Is_airlevel(&u.uz) || (u.usteed && strongmonst(u.usteed->data))) {
        carrcap = MAX_CARR_CAP;
    } else {
        if (carrcap > MAX_CARR_CAP)
            carrcap = MAX_CARR_CAP;
        if (!Flying) {
            if (get_EWounded_legs() & LEFT_SIDE)
                carrcap -= 100;
            if (get_EWounded_legs() & RIGHT_SIDE)
                carrcap -= 100;
        }
        if (carrcap < 0)
            carrcap = 0;
    }
    return (int)carrcap;
}


/* returns how far beyond the normal capacity the player is currently. */
/* inv_weight() is negative if the player is below normal capacity. */
int inv_weight(void) {
    struct obj *otmp = invent;
    int wt = 0;

    /* when putting stuff into containers, gold is inserted at the head
     of invent for easier manipulation by askchain & co, but it's also
     retained in u.ugold in order to keep the status line accurate; we
     mustn't add its weight in twice under that circumstance */
    wt = (otmp && otmp->oclass == COIN_CLASS) ? 0 : (int)((u.ugold + 50L) / 100L);
    while (otmp) {
        if (otmp->otyp != BOULDER || !throws_rocks(youmonst.data))
            wt += otmp->owt;
        otmp = otmp->nobj;
    }
    wc = weight_cap();
    return (wt - wc);
}

/*
 * Returns 0 if below normal capacity, or the number of "capacity units"
 * over the normal capacity the player is loaded.  Max is 5.
 */
int calc_capacity(int xtra_wt) {
    int cap, wt = inv_weight() + xtra_wt;

    if (wt <= 0)
        return UNENCUMBERED;
    if (wc <= 1)
        return OVERLOADED;
    cap = (wt * 2 / wc) + 1;
    return min(cap, OVERLOADED);
}

int near_capacity(void) {
    return calc_capacity(0);
}

int max_capacity(void) {
    int wt = inv_weight();

    return (wt - (2 * wc));
}

bool check_capacity(const char *str) {
    if (near_capacity() >= EXT_ENCUMBER) {
        if (str)
            plines(str);
        else
            You_cant("do that while carrying so much stuff.");
        return 1;
    }
    return 0;
}

int inv_cnt(void) {
    struct obj *otmp = invent;
    int ct = 0;

    while (otmp) {
        ct++;
        otmp = otmp->nobj;
    }
    return ct;
}

struct Killer killed_by_const(enum KillerMethod method) {
    printf("killed_by_const %s\n", killer_method_enum_str[method]);
    return (struct Killer){method, NULL, NULL, NULL, NULL, 0};
}
struct Killer killed_by_monster(enum KillerMethod method, const struct monst *m) {
    printf("killed_by_monster %s\n", killer_method_enum_str[method]);
    return (struct Killer){method, m, NULL, NULL, NULL, 0};
}
struct Killer killed_by_object(enum KillerMethod method, const struct obj *o) {
    printf("killed_by_object %s\n", killer_method_enum_str[method]);
    return (struct Killer){method, NULL, o, NULL, NULL, 0};
}
struct Killer killed_by_string(enum KillerMethod method, const char *string) {
    printf("killed_by_string %s\n", killer_method_enum_str[method]);
    return (struct Killer){method, NULL, NULL, NULL, string, 0};
}
struct Killer killed_by_int(enum KillerMethod method, int i) {
    printf("killed_by_int %s\n", killer_method_enum_str[method]);
    return (struct Killer){method, NULL, NULL, NULL, NULL, i};
}
struct Killer killed_by_artifact(enum KillerMethod method, const struct artifact *art) {
    printf("killed_by_artifact %s\n", killer_method_enum_str[method]);
    return (struct Killer){method, NULL, NULL, art, NULL, 0};
}
