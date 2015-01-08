/* See LICENSE in the root of this project for change info */

#include "steed.h"

#include <stdio.h>
#include <string.h>

#include "apply.h"
#include "attrib.h"
#include "cmd.h"
#include "coord.h"
#include "dbridge.h"
#include "decl.h"
#include "display.h"
#include "do.h"
#include "engrave.h"
#include "flag.h"
#include "global.h"
#include "hack.h"
#include "hacklib.h"
#include "invent.h"
#include "mon.h"
#include "mondata.h"
#include "monflag.h"
#include "monmove.h"
#include "monst.h"
#include "monsym.h"
#include "obj.h"
#include "objclass.h"
#include "objnam.h"
#include "onames.h"
#include "permonst.h"
#include "pickup.h"
#include "pline.h"
#include "pm.h"
#include "polyself.h"
#include "prop.h"
#include "rm.h"
#include "rnd.h"
#include "skills.h"
#include "steal.h"
#include "teleport.h"
#include "trap.h"
#include "util.h"
#include "weapon.h"
#include "worn.h"
#include "you.h"
#include "youprop.h"


/* Monsters that might be ridden */
static const char steeds[] = {
        S_QUADRUPED, S_UNICORN, S_ANGEL, S_CENTAUR, S_DRAGON, S_JABBERWOCK, '\0'
};

/* caller has decided that hero can't reach something while mounted */
void rider_cant_reach (void) {
    message_monster(MSG_YOU_ARE_NOT_SKILLED_ENOUGH_TO_REACH_FROM_M, u.usteed);
}

/*** Putting the saddle on ***/

/* Can this monster wear a saddle? */
bool can_saddle (struct monst *mtmp) {
    struct permonst *ptr = mtmp->data;

    return (index(steeds, ptr->mlet) && (ptr->msize >= MZ_MEDIUM) &&
            (!humanoid(ptr) || ptr->mlet == S_CENTAUR) &&
            !amorphous(ptr) && !noncorporeal(ptr) &&
            !is_whirly(ptr) && !unsolid(ptr));
}

int use_saddle (struct obj *otmp) {
    struct monst *mtmp;
    struct permonst *ptr;
    int chance;
    const char *s;


    /* Can you use it? */
    if (nohands(youmonst.data)) {
        You("have no hands!");  /* not `body_part(HAND)' */
        return 0;
    } else if (!freehand()) {
        You("have no free %s.", body_part(HAND));
        return 0;
    }

    /* Select an animal */
    if (u.uswallow || Underwater || !getdir((char *)0)) {
        plines(Never_mind);
        return 0;
    }
    if (!u.dx && !u.dy) {
        pline("Saddle yourself?  Very funny...");
        return 0;
    }
    if (!isok(u.ux+u.dx, u.uy+u.dy) ||
            !(mtmp = m_at(u.ux+u.dx, u.uy+u.dy)) ||
            !canspotmon(mtmp)) {
        pline("I see nobody there.");
        return 1;
    }

    /* Is this a valid monster? */
    if ((mtmp->misc_worn_check & W_SADDLE) || which_armor(mtmp, W_SADDLE)) {
        message_monster(MSG_M_DOES_NOT_NEED_ANOTHER_ONE, mtmp);
        return 1;
    }
    ptr = mtmp->data;
    if (touch_petrifies(ptr) && !uarmg && !Stone_resistance()) {
        char kbuf[BUFSZ];

        message_monster(MSG_YOU_TOUCH_M, mtmp);
        if (!(poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM))) {
            sprintf(kbuf, "attempting to saddle %s", an(mtmp->data->mname));
            instapetrify(kbuf);
        }
    }
    if (ptr == &mons[PM_INCUBUS] || ptr == &mons[PM_SUCCUBUS]) {
        pline("Shame on you!");
        exercise(A_WIS, false);
        return 1;
    }
    if (mtmp->isminion || mtmp->isshk || mtmp->ispriest || mtmp->isgd || mtmp->iswiz) {
        message_monster(MSG_I_THINK_M_WOULD_MIND, mtmp);
        return 1;
    }
    if (!can_saddle(mtmp)) {
        You_cant("saddle such a creature.");
        return 1;
    }

    /* Calculate your chance */
    chance = ACURR(A_DEX) + ACURR(A_CHA)/2 + 2*mtmp->mtame;
    chance += u.ulevel * (mtmp->mtame ? 20 : 5);
    if (!mtmp->mtame) chance -= 10*mtmp->m_lev;
    if (Role_if(PM_KNIGHT))
        chance += 20;
    switch (P_SKILL(P_RIDING)) {
        case P_ISRESTRICTED:
        case P_UNSKILLED:
        default:
            chance -= 20;       break;
        case P_BASIC:
            break;
        case P_SKILLED:
            chance += 15;       break;
        case P_EXPERT:
            chance += 30;       break;
    }
    if (Confusion() || Fumbling || Glib)
        chance -= 20;
    else if (uarmg &&
            (s = OBJ_DESCR(objects[uarmg->otyp])) != (char *)0 &&
            !strncmp(s, "riding ", 7))
        /* Bonus for wearing "riding" (but not fumbling) gloves */
        chance += 10;
    else if (uarmf &&
            (s = OBJ_DESCR(objects[uarmf->otyp])) != (char *)0 &&
            !strncmp(s, "riding ", 7))
        /* ... or for "riding boots" */
        chance += 10;
    if (otmp->cursed)
        chance -= 50;

    /* Make the attempt */
    if (rn2(100) < chance) {
        message_monster(MSG_YOU_PUT_SADDLE_ON_M, mtmp);
        if (otmp->owornmask) remove_worn_item(otmp, false);
        freeinv(otmp);
        /* mpickobj may free otmp it if merges, but we have already
           checked for a saddle above, so no merger should happen */
        (void) mpickobj(mtmp, otmp);
        mtmp->misc_worn_check |= W_SADDLE;
        otmp->owornmask = W_SADDLE;
        otmp->leashmon = mtmp->m_id;
        update_mon_intrinsics(mtmp, otmp, true, false);
    } else {
        message_monster(MSG_M_RESISTS, mtmp);
    }
    return 1;
}


/*** Riding the monster ***/

/* Can we ride this monster?  Caller should also check can_saddle() */
bool can_ride (struct monst *mtmp) {
    return (mtmp->mtame && humanoid(youmonst.data) &&
            !verysmall(youmonst.data) && !bigmonst(youmonst.data) &&
            (!Underwater || is_swimmer(mtmp->data)));
}

int doride (void) {
    bool forcemount = false;

    if (u.usteed)
        dismount_steed(DISMOUNT_BYCHOICE);
    else if (getdir((char *)0) && isok(u.ux+u.dx, u.uy+u.dy)) {
        if (flags.debug && yn("Force the mount to succeed?") == 'y')
            forcemount = true;
        return (mount_steed(m_at(u.ux+u.dx, u.uy+u.dy), forcemount));
    } else
        return 0;
    return 1;
}


/* Start riding, with the given monster */
// struct monst *mtmp,     /* The animal */
// bool force          /* Quietly force this animal */
bool mount_steed ( struct monst *mtmp, bool force) {
    struct obj *otmp;
    char buf[BUFSZ];
    struct permonst *ptr;

    /* Sanity checks */
    if (u.usteed) {
        message_monster(MSG_YOU_ARE_ALREADY_RIDING_M, u.usteed);
        return (false);
    }

    /* Is the player in the right form? */
    if (Hallucination() && !force) {
        pline("Maybe you should find a designated driver.");
        return (false);
    }
    /* While riding Wounded_legs refers to the steed's,
     * not the hero's legs.
     * That opens up a potential abuse where the player
     * can mount a steed, then dismount immediately to
     * heal leg damage, because leg damage is always
     * healed upon dismount (Wounded_legs context switch).
     * By preventing a hero with Wounded_legs from
     * mounting a steed, the potential for abuse is
     * minimized, if not eliminated altogether.
     */
    if (Wounded_legs) {
        Your("%s are in no shape for riding.", makeplural(body_part(LEG)));
        if (force && flags.debug && yn("Heal your legs?") == 'y')
            HWounded_legs = EWounded_legs = 0;
        else
            return (false);
    }

    if (Upolyd && (!humanoid(youmonst.data) || verysmall(youmonst.data) ||
                bigmonst(youmonst.data) || slithy(youmonst.data))) {
        You("won't fit on a saddle.");
        return (false);
    }
    if(!force && (near_capacity() > SLT_ENCUMBER)) {
        You_cant("do that while carrying so much stuff.");
        return (false);
    }

    /* Can the player reach and see the monster? */
    if (!mtmp || (!force && ((Blind && !Blind_telepat) ||
                    mtmp->mundetected ||
                    mtmp->m_ap_type == M_AP_FURNITURE ||
                    mtmp->m_ap_type == M_AP_OBJECT))) {
        pline("I see nobody there.");
        return (false);
    }
    if (u.uswallow || u.ustuck || u.utrap || Punished ||
            !test_move(u.ux, u.uy, mtmp->mx-u.ux, mtmp->my-u.uy, TEST_MOVE)) {
        if (Punished || !(u.uswallow || u.ustuck || u.utrap))
            You("are unable to swing your %s over.", body_part(LEG));
        else
            You("are stuck here for now.");
        return (false);
    }

    /* Is this a valid monster? */
    otmp = which_armor(mtmp, W_SADDLE);
    if (!otmp) {
        message_monster(MSG_M_IS_NOT_SADDLED, mtmp);
        return (false);
    }
    ptr = mtmp->data;
    if (touch_petrifies(ptr) && !Stone_resistance()) {
        char kbuf[BUFSZ];

        message_monster(MSG_YOU_TOUCH_M, mtmp);
        sprintf(kbuf, "attempting to ride %s", an(mtmp->data->mname));
        instapetrify(kbuf);
    }
    if (!mtmp->mtame || mtmp->isminion) {
        message_monster(MSG_I_THINK_M_WOULD_MIND, mtmp);
        return (false);
    }
    if (mtmp->mtrapped) {
        struct trap *t = t_at(mtmp->mx, mtmp->my);

        message_monster_string(MSG_YOU_CANT_MOUNT_M_WHILE_HES_TRAPPED_IN_S, mtmp,
                defsyms[trap_to_defsym(t->ttyp)].explanation);
        return (false);
    }

    if (!force && !Role_if(PM_KNIGHT) && !(--mtmp->mtame)) {
        /* no longer tame */
        newsym(mtmp->mx, mtmp->my);
        unsigned char leash_flag = mtmp->mleashed ? MSG_FLAG_LEASH_OFF : 0;
        message_monster_flag(MSG_M_RESISTS, mtmp, leash_flag);
        if (mtmp->mleashed) m_unleash(mtmp, false);
        return (false);
    }
    if (!force && Underwater && !is_swimmer(ptr)) {
        You_cant("ride that creature while under water.");
        return (false);
    }
    if (!can_saddle(mtmp) || !can_ride(mtmp)) {
        You_cant("ride such a creature.");
        return (0);
    }

    /* Is the player impaired? */
    if (!force && !is_floater(ptr) && !is_flyer(ptr) && Levitation && !Lev_at_will) {
        message_monster(MSG_YOU_CANNOT_REACH_M, mtmp);
        return (false);
    }
    if (!force && uarm && is_metallic(uarm) && greatest_erosion(uarm)) {
        message_monster_string(MSG_YOUR_RUSTY_ARMOR_TOO_STIFF_TO_MOUNT_M, mtmp,
            uarm->oeroded ? "rusty" : "corroded");
        return (false);
    }
    if (!force && (Confusion() || Fumbling || Glib || Wounded_legs ||
                otmp->cursed || (u.ulevel+mtmp->mtame < rnd(MAXULEV/2+5))))
    {
        if (Levitation) {
            message_monster(MSG_M_SLIPS_AWAY_FROM_YOU, mtmp);
            return false;
        }
        message_monster(MSG_YOU_SLIP_WHILE_TRYING_TO_GET_ON_M, mtmp);

        losehp(rn1(5,10), killed_by_monster(KM_SLIPPED_WHILE_MOUNTING_M, mtmp));
        return (false);
    }

    /* Success */
    if (!force) {
        if (Levitation && !is_floater(ptr) && !is_flyer(ptr)) {
            /* Must have Lev_at_will at this point */
            message_monster(MSG_M_MAGICALLY_FLOATS_UP, mtmp);
        }
        message_monster(MSG_YOU_MOUNT_M, mtmp);
    }
    /* setuwep handles polearms differently when you're mounted */
    if (uwep && is_pole(uwep)) unweapon = false;
    u.usteed = mtmp;
    remove_monster(mtmp->mx, mtmp->my);
    teleds(mtmp->mx, mtmp->my, true);
    return true;
}

/* You and your steed have moved */
void exercise_steed (void) {
    if (!u.usteed)
        return;

    /* It takes many turns of riding to exercise skill */
    if (u.urideturns++ >= 100) {
        u.urideturns = 0;
        use_skill(P_RIDING, 1);
    }
    return;
}


/* The player kicks or whips the steed */
void kick_steed (void) {
    char He[4];
    if (!u.usteed)
        return;

    /* [ALI] Various effects of kicking sleeping/paralyzed steeds */
    if (u.usteed->msleeping || !u.usteed->mcanmove) {
        /* We assume a message has just been output of the form
         * "You kick <steed>."
         */
        strcpy(He, mhe(u.usteed));
        *He = highc(*He);
        if ((u.usteed->mcanmove || u.usteed->mfrozen) && !rn2(2)) {
            if (u.usteed->mcanmove)
                u.usteed->msleeping = 0;
            else if (u.usteed->mfrozen > 2)
                u.usteed->mfrozen -= 2;
            else {
                u.usteed->mfrozen = 0;
                u.usteed->mcanmove = 1;
            }
            if (u.usteed->msleeping || !u.usteed->mcanmove)
                pline("%s stirs.", He);
            else
                pline("%s rouses %sself!", He, mhim(u.usteed));
        } else
            pline("%s does not respond.", He);
        return;
    }

    /* Make the steed less tame and check if it resists */
    if (u.usteed->mtame) u.usteed->mtame--;
    if (!u.usteed->mtame && u.usteed->mleashed) m_unleash(u.usteed, true);
    if (!u.usteed->mtame || (u.ulevel+u.usteed->mtame < rnd(MAXULEV/2+5))) {
        newsym(u.usteed->mx, u.usteed->my);
        dismount_steed(DISMOUNT_THROWN);
        return;
    }

    message_monster(MSG_M_GALLOPS, u.usteed);
    u.ugallop += rn1(20, 30);
    return;
}

/*
 * Try to find a dismount point adjacent to the steed's location.
 * If all else fails, try enexto().  Use enexto() as a last resort because
 * enexto() chooses its point randomly, possibly even outside the
 * room's walls, which is not what we want.
 * Adapted from mail daemon code.
 */
// coord *spot,    /* landing position (we fill it in) */
static bool landing_spot ( coord *spot, int reason, int forceit) {
    int i = 0, x, y, distance, min_distance = -1;
    bool found = false;
    struct trap *t;

    /* avoid known traps (i == 0) and boulders, but allow them as a backup */
    if (reason != DISMOUNT_BYCHOICE || Stunned() || Confusion() || Fumbling) i = 1;
    for (; !found && i < 2; ++i) {
        for (x = u.ux-1; x <= u.ux+1; x++)
            for (y = u.uy-1; y <= u.uy+1; y++) {
                if (!isok(x, y) || (x == u.ux && y == u.uy)) continue;

                if (ACCESSIBLE(levl[x][y].typ) &&
                        !MON_AT(x,y) && !closed_door(x,y)) {
                    distance = distu(x,y);
                    if (min_distance < 0 || distance < min_distance ||
                            (distance == min_distance && rn2(2))) {
                        if (i > 0 || (((t = t_at(x, y)) == 0 || !t->tseen) &&
                                    (!sobj_at(BOULDER, x, y) ||
                                     throws_rocks(youmonst.data)))) {
                            spot->x = x;
                            spot->y = y;
                            min_distance = distance;
                            found = true;
                        }
                    }
                }
            }
    }

    /* If we didn't find a good spot and forceit is on, try enexto(). */
    if (forceit && min_distance < 0 &&
            !enexto(spot, u.ux, u.uy, youmonst.data))
        return false;

    return found;
}

/* Stop riding the current steed */
/* Player was thrown off etc. */
void dismount_steed ( int reason) {
    struct monst *mtmp;
    struct obj *otmp;
    coord cc;
    const char *verb = "fall";
    bool repair_leg_damage = true;
    unsigned save_utrap = u.utrap;
    bool have_spot = landing_spot(&cc,reason,0);

    mtmp = u.usteed;                /* make a copy of steed pointer */
    /* Sanity check */
    if (!mtmp)              /* Just return silently */
        return;

    /* Check the reason for dismounting */
    otmp = which_armor(mtmp, W_SADDLE);
    switch (reason) {
        case DISMOUNT_THROWN:
            verb = "are thrown";
        case DISMOUNT_FELL:
            message_monster_string(MSG_YOU_FALL_OFF_OF_M, mtmp, verb);
            if (!have_spot) have_spot = landing_spot(&cc,reason,1);
            losehp(rn1(10,10), killed_by_const(KM_RIDING_ACCIDENT));
            set_wounded_legs(BOTH_SIDES, (int)HWounded_legs + rn1(5,5));
            repair_leg_damage = false;
            break;
        case DISMOUNT_POLY:
            message_monster(MSG_YOU_CAN_NO_LONGER_RIDE_M, u.usteed);
            if (!have_spot) have_spot = landing_spot(&cc,reason,1);
            break;
        case DISMOUNT_ENGULFED:
            /* caller displays message */
            break;
        case DISMOUNT_BONES:
            /* hero has just died... */
            break;
        case DISMOUNT_GENERIC:
            /* no messages, just make it so */
            break;
        case DISMOUNT_BYCHOICE:
        default:
            if (otmp && otmp->cursed) {
                You("can't.  The saddle %s cursed.",
                        otmp->bknown ? "is" : "seems to be");
                otmp->bknown = true;
                return;
            }
            if (!have_spot) {
                You("can't. There isn't anywhere for you to stand.");
                return;
            }
            if (!mtmp->mnamelth) {
                pline("You've been through the dungeon on %s with no name.",
                        an(mtmp->data->mname));
                if (Hallucination())
                    pline("It felt good to get out of the rain.");
            } else {
                message_monster(MSG_YOU_DISMOUNT_M, mtmp);
            }
    }
    /* While riding these refer to the steed's legs
     * so after dismounting they refer to the player's
     * legs once again.
     */
    if (repair_leg_damage) HWounded_legs = EWounded_legs = 0;

    /* Release the steed and saddle */
    u.usteed = 0;
    u.ugallop = 0L;

    /* Set player and steed's position.  Try moving the player first
       unless we're in the midst of creating a bones file. */
    if (reason == DISMOUNT_BONES) {
        /* move the steed to an adjacent square */
        if (enexto(&cc, u.ux, u.uy, mtmp->data))
            rloc_to(mtmp, cc.x, cc.y);
        else        /* evidently no room nearby; move steed elsewhere */
            (void) rloc(mtmp, false);
        return;
    }
    if (!DEADMONSTER(mtmp)) {
        place_monster(mtmp, u.ux, u.uy);
        if (!u.uswallow && !u.ustuck && have_spot) {
            struct permonst *mdat = mtmp->data;

            /* The steed may drop into water/lava */
            if (!is_flyer(mdat) && !is_floater(mdat) && !is_clinger(mdat)) {
                if (is_pool(u.ux, u.uy)) {
                    if (!Underwater) {
                        message_monster_string(MSG_M_FALLS_INTO_THE_S, mtmp, surface(u.ux, u.uy));
                    }
                    if (!is_swimmer(mdat) && !amphibious(mdat)) {
                        killed(mtmp);
                        adjalign(-1);
                    }
                } else if (is_lava(u.ux, u.uy)) {
                    message_monster(MSG_M_IS_PULLED_INTO_THE_LAVA, mtmp);
                    if (!likes_lava(mdat)) {
                        killed(mtmp);
                        adjalign(-1);
                    }
                }
            }
            /* Steed dismounting consists of two steps: being moved to another
             * square, and descending to the floor.  We have functions to do
             * each of these activities, but they're normally called
             * individually and include an attempt to look at or pick up the
             * objects on the floor:
             * teleds() --> spoteffects() --> pickup()
             * float_down() --> pickup()
             * We use this kludge to make sure there is only one such attempt.
             *
             * Clearly this is not the best way to do it.  A full fix would
             * involve having these functions not call pickup() at all, instead
             * calling them first and calling pickup() afterwards.  But it
             * would take a lot of work to keep this change from having any
             * unforseen side effects (for instance, you would no longer be
             * able to walk onto a square with a hole, and autopickup before
             * falling into the hole).
             */
            /* [ALI] No need to move the player if the steed died. */
            if (!DEADMONSTER(mtmp)) {
                /* Keep steed here, move the player to cc;
                 * teleds() clears u.utrap
                 */
                in_steed_dismounting = true;
                teleds(cc.x, cc.y, true);
                in_steed_dismounting = false;

                /* Put your steed in your trap */
                if (save_utrap)
                    (void) mintrap(mtmp);
            }
            /* Couldn't... try placing the steed */
        } else if (enexto(&cc, u.ux, u.uy, mtmp->data)) {
            /* Keep player here, move the steed to cc */
            rloc_to(mtmp, cc.x, cc.y);
            /* Player stays put */
            /* Otherwise, kill the steed */
        } else {
            killed(mtmp);
            adjalign(-1);
        }
    }

    /* Return the player to the floor */
    if (reason != DISMOUNT_ENGULFED) {
        in_steed_dismounting = true;
        (void) float_down(0L, W_SADDLE);
        in_steed_dismounting = false;
        (void)encumber_msg();
        vision_full_recalc = 1;
    }
    /* polearms behave differently when not mounted */
    if (uwep && is_pole(uwep)) unweapon = true;
    return;
}

void place_monster (struct monst *mon, int x, int y) {
    if (mon == u.usteed ||
            /* special case is for convoluted vault guard handling */
            (DEADMONSTER(mon) && !(mon->isgd && x == 0 && y == 0))) {
        impossible("placing %s onto map?",
                (mon == u.usteed) ? "steed" : "defunct monster");
        return;
    }
    mon->mx = x, mon->my = y;
    level.monsters[x][y] = mon;
}
