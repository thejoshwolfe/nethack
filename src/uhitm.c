/* See LICENSE in the root of this project for change info */

#include "everything.h"
#include "dog.h"
#include "timeout.h"
#include "muse.h"
#include "potion.h"
#include "dothrow.h"
#include "mkobj.h"
#include "artifact.h"
#include "mthrowu.h"
#include "zap.h"
#include "priest.h"
#include "engrave.h"
#include "polyself.h"
#include "wield.h"
#include "mhitm.h"
#include "monmove.h"
#include "weapon.h"
#include "invent.h"
#include "shk.h"
#include "do_name.h"
#include "dbridge.h"
#include "mon.h"
#include "pline.h"
#include "objnam.h"
#include "uhitm.h"
#include "hack.h"
#include "pm_props.h"
#include "display.h"
#include "worn.h"
#include "everything.h"

extern bool notonhead; /* for long worms */
/* The below might become a parameter instead if we use it a lot */
static int dieroll;
/* Used to flag attacks caused by Stormbringer's maliciousness. */
static bool override_confirmation = false;

static bool PROJECTILE(struct obj * obj) {
    return obj && is_ammo(obj);
}

static struct obj * useup_eggs(bool thrown, struct obj * o) {
    if (thrown)
        obfree(o, NULL);
    else
        useupall(o);
    /* now gone */
    return NULL;
}

/* modified from hurtarmor() in mhitu.c */
/* This is not static because it is also used for monsters rusting monsters */
void hurtmarmor(struct monst *mdef, int attk) {
    int hurt;
    struct obj *target;

    switch (attk) {
        /* 0 is burning, which we should never be called with */
        case AD_RUST:
            hurt = 1;
            break;
        case AD_CORR:
            hurt = 3;
            break;
        default:
            hurt = 2;
            break;
    }
    /* What the following code does: it keeps looping until it
     * finds a target for the rust monster.
     * Head, feet, etc... not covered by metal, or covered by
     * rusty metal, are not targets.  However, your body always
     * is, no matter what covers it.
     */
    while (1) {
        switch (rn2(5)) {
            case 0:
                target = which_armor(mdef, W_ARMH);
                if (!target || !rust_dmg(target, xname(target), hurt, false, mdef))
                    continue;
                break;
            case 1:
                target = which_armor(mdef, W_ARMC);
                if (target) {
                    (void)rust_dmg(target, xname(target), hurt, true, mdef);
                    break;
                }
                if ((target = which_armor(mdef, W_ARM)) != (struct obj *)0) {
                    (void)rust_dmg(target, xname(target), hurt, true, mdef);
                } else if ((target = which_armor(mdef, W_ARMU)) != (struct obj *)0) {
                    (void)rust_dmg(target, xname(target), hurt, true, mdef);
                }
                break;
            case 2:
                target = which_armor(mdef, W_ARMS);
                if (!target || !rust_dmg(target, xname(target), hurt, false, mdef))
                    continue;
                break;
            case 3:
                target = which_armor(mdef, W_ARMG);
                if (!target || !rust_dmg(target, xname(target), hurt, false, mdef))
                    continue;
                break;
            case 4:
                target = which_armor(mdef, W_ARMF);
                if (!target || !rust_dmg(target, xname(target), hurt, false, mdef))
                    continue;
                break;
        }
        break; /* Out of while loop */
    }
}

/* false means it's OK to attack */
/* wep: uwep for attack(), null for kick_monster() */
bool attack_checks(struct monst *mtmp, struct obj *wep) {
    char qbuf[QBUFSZ];
    char buf[BUFSZ];

    /* if you're close enough to attack, alert any waiting monster */
    mtmp->mstrategy &= ~STRAT_WAITMASK;

    if (u.uswallow && mtmp == u.ustuck)
        return false;

    if (flags.forcefight) {
        /* Do this in the caller, after we checked that the monster
         * didn't die from the blow.  Reason: putting the 'I' there
         * causes the hero to forget the square's contents since
         * both 'I' and remembered contents are stored in .glyph.
         * If the monster dies immediately from the blow, the 'I' will
         * not stay there, so the player will have suddenly forgotten
         * the square's contents for no apparent reason.
         if (!canspotmon(mtmp) &&
         !glyph_is_invisible(levl[u.ux+u.dx][u.uy+u.dy].glyph))
         map_invisible(u.ux+u.dx, u.uy+u.dy);
         */
        return false;
    }

    /* Put up an invisible monster marker, but with exceptions for
     * monsters that hide and monsters you've been warned about.
     * The former already prints a warning message and
     * prevents you from hitting the monster just via the hidden monster
     * code below; if we also did that here, similar behavior would be
     * happening two turns in a row.  The latter shows a glyph on
     * the screen, so you know something is there.
     */
    if (!canspotmon(mtmp) && !glyph_is_warning(glyph_at(u.ux + u.dx, u.uy + u.dy)) && !glyph_is_invisible(levl[u.ux+u.dx][u.uy+u.dy].glyph) && !(!Blind && mtmp->mundetected && hides_under(mtmp->data))) {
        pline("Wait!  There's %s there you can't see!", something);
        map_invisible(u.ux + u.dx, u.uy + u.dy);
        /* if it was an invisible mimic, treat it as if we stumbled
         * onto a visible mimic
         */
        if (mtmp->m_ap_type && !Protection_from_shape_changers) {
            if (!u.ustuck && !mtmp->mflee && dmgtype(mtmp->data, AD_STCK))
                u.ustuck = mtmp;
        }
        wakeup(mtmp); /* always necessary; also un-mimics mimics */
        return true;
    }

    if (mtmp->m_ap_type && !Protection_from_shape_changers && !sensemon(mtmp) && !glyph_is_warning(glyph_at(u.ux + u.dx, u.uy + u.dy))) {
        /* If a hidden mimic was in a square where a player remembers
         * some (probably different) unseen monster, the player is in
         * luck--he attacks it even though it's hidden.
         */
        if (glyph_is_invisible(levl[mtmp->mx][mtmp->my].glyph)) {
            seemimic(mtmp);
            return (false);
        }
        stumble_onto_mimic(mtmp);
        return true;
    }

    if (mtmp->mundetected && !canseemon(mtmp) && !glyph_is_warning(glyph_at(u.ux + u.dx, u.uy + u.dy)) && (hides_under(mtmp->data) || mtmp->data->mlet == S_EEL)) {
        mtmp->mundetected = mtmp->msleeping = 0;
        newsym(mtmp->mx, mtmp->my);
        if (glyph_is_invisible(levl[mtmp->mx][mtmp->my].glyph)) {
            seemimic(mtmp);
            return (false);
        }
        if (!(Blind ? Blind_telepat : Unblind_telepat)) {
            struct obj *obj;

            if (Blind || (is_pool(mtmp->mx, mtmp->my) && !Underwater))
                pline("Wait!  There's a hidden monster there!");
            else if ((obj = level.objects[mtmp->mx][mtmp->my]) != 0)
                message_monster_object(MSG_MONSTER_HIDING_UNDER_OBJECT, mtmp, obj);
            return true;
        }
    }

    /*
     * make sure to wake up a monster from the above cases if the
     * hero can sense that the monster is there.
     */
    if ((mtmp->mundetected || mtmp->m_ap_type) && sensemon(mtmp)) {
        mtmp->mundetected = 0;
        wakeup(mtmp);
    }

    return false;
}

/*
 * It is unchivalrous for a knight to attack the defenseless or from behind.
 */
void check_caitiff(struct monst *mtmp) {
    if (Role_if(PM_KNIGHT) && u.ualign.type == A_LAWFUL && (!mtmp->mcanmove || mtmp->msleeping || (mtmp->mflee && !mtmp->mavenge)) && u.ualign.record > -10) {
        You("caitiff!");
        adjalign(-1);
    }
}

signed char find_roll_to_hit(struct monst *mtmp) {
    signed char tmp;
    int tmp2;

    tmp = 1 + Luck + abon() + find_mac(mtmp) + u.uhitinc + (Upolyd ? youmonst.data->mlevel : u.ulevel);

    check_caitiff(mtmp);

    /*      attacking peaceful creatures is bad for the samurai's giri */
    if (Role_if(PM_SAMURAI) && mtmp->mpeaceful && u.ualign.record > -10) {
        You("dishonorably attack the innocent!");
        adjalign(-1);
    }

    /*      Adjust vs. (and possibly modify) monster state.         */

    if (mtmp->mstun)
        tmp += 2;
    if (mtmp->mflee)
        tmp += 2;

    if (mtmp->msleeping) {
        mtmp->msleeping = 0;
        tmp += 2;
    }
    if (!mtmp->mcanmove) {
        tmp += 4;
        if (!rn2(10)) {
            mtmp->mcanmove = 1;
            mtmp->mfrozen = 0;
        }
    }
    if (is_orc(mtmp->data) && (Upolyd ? is_elf(youmonst.data) : Race_if(PM_ELF)))
        tmp++;
    if (Role_if(PM_MONK) && !Upolyd) {
        if (uarm) {
            Your("armor is rather cumbersome...");
            tmp -= urole.spelarmr;
        } else if (!uwep && !uarms) {
            tmp += (u.ulevel / 3) + 2;
        }
    }

    /*      with a lot of luggage, your agility diminishes */
    if ((tmp2 = near_capacity()) != 0)
        tmp -= (tmp2 * 2) - 1;
    if (u.utrap)
        tmp -= 3;
    /*      Some monsters have a combination of weapon attacks and non-weapon
     *      attacks.  It is therefore wrong to add hitval to tmp; we must add
     *      it only for the specific attack (in hmonas()).
     */
    if (uwep && !Upolyd) {
        tmp += hitval(uwep, mtmp);
        tmp += weapon_hit_bonus(uwep);
    }
    return tmp;
}

/* returns true if monster still lives */
static bool known_hitum(struct monst *mon, int *mhit, struct attack *uattk) {
    bool malive = true;

    if (override_confirmation) {
        /* this may need to be generalized if weapons other than
         Stormbringer acquire similar anti-social behavior... */
        if (flags.verbose)
            Your("bloodthirsty blade attacks!");
    }

    if (!*mhit) {
        missum(mon, uattk);
    } else {
        int oldhp = mon->mhp, x = u.ux + u.dx, y = u.uy + u.dy;

        /* KMH, conduct */
        if (uwep && (uwep->oclass == WEAPON_CLASS || is_weptool(uwep)))
            u.uconduct.weaphit++;

        /* we hit the monster; be careful: it might die or
         be knocked into a different location */
        notonhead = (mon->mx != x || mon->my != y);
        malive = hmon(mon, uwep, 0);
        /* this assumes that Stormbringer was uwep not uswapwep */
        if (malive && u.twoweap && !override_confirmation &&
                m_at(x, y) == mon)
            malive = hmon(mon, uswapwep, 0);
        if (malive) {
            /* monster still alive */
            if (!rn2(25) && mon->mhp < mon->mhpmax / 2 && !(u.uswallow && mon == u.ustuck)) {
                /* maybe should regurgitate if swallowed? */
                if (!rn2(3)) {
                    monflee(mon, rnd(100), false, true);
                } else
                    monflee(mon, 0, false, true);

                if (u.ustuck == mon && !u.uswallow && !sticks(youmonst.data))
                    u.ustuck = 0;
            }
            /* Vorpal Blade hit converted to miss */
            /* could be headless monster or worm tail */
            if (mon->mhp == oldhp) {
                *mhit = 0;
                /* a miss does not break conduct */
                if (uwep && (uwep->oclass == WEAPON_CLASS || is_weptool(uwep)))
                    --u.uconduct.weaphit;
            }
            if (mon->wormno && *mhit)
                cutworm(mon, x, y, uwep);
        }
    }
    return (malive);
}

static int explum(struct monst *mdef, struct attack *mattk) {
    int tmp = d((int)mattk->damn, (int)mattk->damd);

    You("explode!");
    switch (mattk->adtyp) {
        bool resistance; /* only for cold/fire/elec */

        case AD_BLND:
            if (!resists_blnd(mdef)) {
                message_monster(MSG_M_IS_BLINDED_BY_YOUR_FLASH_OF_LIGHT, mdef);
                mdef->mblinded = min((int )mdef->mblinded + tmp, 127);
                mdef->mcansee = 0;
            }
            break;
        case AD_HALU:
            if (haseyes(mdef->data) && mdef->mcansee) {
                message_monster(MSG_M_IS_AFFECTED_BY_YOUR_FLASH_OF_LIGHT, mdef);
                mdef->mconf = 1;
            }
            break;
        case AD_COLD:
            resistance = resists_cold(mdef);
            goto common;
        case AD_FIRE:
            resistance = resists_fire(mdef);
            goto common;
        case AD_ELEC:
            resistance = resists_elec(mdef);
            common: if (!resistance) {
                message_monster(MSG_M_GETS_BLASTED, mdef);
                mdef->mhp -= tmp;
                if (mdef->mhp <= 0) {
                    killed(mdef);
                    return (2);
                }
            } else {
                shieldeff(mdef->mx, mdef->my);
                if (is_golem(mdef->data)) {
                    golemeffects(mdef, (int)mattk->adtyp, tmp);
                } else {
                    message_monster(MSG_THE_BLAST_DOESNT_SEEM_TO_AFFECT_M, mdef);
                }
            }
            break;
        default:
            break;
    }
    return (1);
}

static void start_engulf(struct monst *mdef) {
    if (!Invisible) {
        map_location(u.ux, u.uy, true);
        tmp_at(DISP_ALWAYS, mon_to_glyph(&youmonst));
        tmp_at(mdef->mx, mdef->my);
    }
    message_monster(MSG_YOU_ENGULF_M, mdef);
    my_delay_output();
    my_delay_output();
}

static void end_engulf(void) {
    if (!Invisible) {
        tmp_at(DISP_END, 0);
        newsym(u.ux, u.uy);
    }
}

static int gulpum(struct monst *mdef, struct attack *mattk) {
    int tmp;
    int dam = d((int)mattk->damn, (int)mattk->damd);
    struct obj *otmp;
    /* Not totally the same as for real monsters.  Specifically, these
     * don't take multiple moves.  (It's just too hard, for too little
     * result, to program monsters which attack from inside you, which
     * would be necessary if done accurately.)  Instead, we arbitrarily
     * kill the monster immediately for AD_DGST and we regurgitate them
     * after exactly 1 round of attack otherwise.  -KAA
     */

    if (mdef->data->msize >= MZ_HUGE)
        return 0;

    if (u.uhunger < 1500 && !u.uswallow) {
        for (otmp = mdef->minvent; otmp; otmp = otmp->nobj)
            (void)snuff_lit(otmp);

        if (!touch_petrifies(mdef->data) || Stone_resistance) {
            static char msgbuf[BUFSZ];
            start_engulf(mdef);
            switch (mattk->adtyp) {
                case AD_DGST:
                    /* eating a Rider or its corpse is fatal */
                    if (is_rider(mdef->data)) {
                        pline("Unfortunately, digesting any of it is fatal.");
                        end_engulf();
                        sprintf(msgbuf, "unwisely tried to eat %s", mdef->data->mname);
                        fprintf(stderr, "TODO: killer = %s\n", msgbuf);
                        done(DIED);
                        return 0; /* lifesaved */
                    }

                    if (Slow_digestion) {
                        dam = 0;
                        break;
                    }

                    /* KMH, conduct */
                    u.uconduct.food++;
                    if (!vegan(mdef->data))
                        u.uconduct.unvegan++;
                    if (!vegetarian(mdef->data))
                        violated_vegetarian();

                    /* Use up amulet of life saving */
                    if (!!(otmp = mlifesaver(mdef)))
                        m_useup(mdef, otmp);

                    newuhs(false);
                    xkilled(mdef, 2);
                    if (mdef->mhp > 0) { /* monster lifesaved */
                        You("hurriedly regurgitate the sizzling in your %s.", body_part(STOMACH));
                    } else {
                        tmp = 1 + (mdef->data->cwt >> 8);
                        if (corpse_chance(mdef, &youmonst, true) && !(mvitals[monsndx(mdef->data)].mvflags & G_NOCORPSE)) {
                            /* nutrition only if there can be a corpse */
                            u.uhunger += (mdef->data->cnutrit + 1) / 2;
                        } else
                            tmp = 0;
                        sprintf(msgbuf, "You totally digest %s.", "TODO:mon_nam(mdef)");
                        if (tmp != 0) {
                            /* setting afternmv = end_engulf is tempting,
                             * but will cause problems if the player is
                             * attacked (which uses his real location) or
                             * if his See_invisible wears off
                             */
                            message_monster(MSG_YOU_DIGEST_M, mdef);
                            if (Slow_digestion)
                                tmp *= 2;
                            nomul(-tmp);
                            nomovemsg = msgbuf;
                        } else
                            pline("%s", msgbuf);
                        if (mdef->data == &mons[PM_GREEN_SLIME]) {
                            sprintf(msgbuf, "%s isn't sitting well with you.", The(mdef->data->mname));
                            if (!Unchanging) {
                                Slimed= 5L;
                                flags.botl = 1;
                            }
                        } else
                            exercise(A_CON, true);
                    }
                    end_engulf();
                    return(2);
                case AD_PHYS:
                    if (youmonst.data == &mons[PM_FOG_CLOUD]) {
                        message_monster(MSG_M_IS_LADEN_WITH_YOUR_MOISTURE, mdef);
                        if (amphibious(mdef->data) && !flaming(mdef->data)) {
                            dam = 0;
                            message_monster(MSG_M_SEEMS_UNHARMED, mdef);
                        }
                    } else {
                        message_monster(MSG_M_IS_PUMMELED_WITH_YOUR_DEBRIS, mdef);
                    }
                    break;
                case AD_ACID:
                    message_monster(MSG_M_IS_COVERED_WITH_YOUR_GOO, mdef);
                    if (resists_acid(mdef)) {
                        message_monster(MSG_IT_SEEMS_HARMLESS_TO_M, mdef);
                        dam = 0;
                    }
                    break;
                case AD_BLND:
                    if (can_blnd(&youmonst, mdef, mattk->aatyp, (struct obj *)0)) {
                        if (mdef->mcansee) {
                            message_monster(MSG_M_CANT_SEE_IN_THERE, mdef);
                        }
                        mdef->mcansee = 0;
                        dam += mdef->mblinded;
                        if (dam > 127) dam = 127;
                        mdef->mblinded = dam;
                    }
                    dam = 0;
                    break;
                case AD_ELEC:
                    if (rn2(2)) {
                        message_monster(MSG_THE_AIR_AROUND_M_CRACKLES_WITH_ELECTRICITY, mdef);
                        if (resists_elec(mdef)) {
                            message_monster(MSG_M_SEEMS_UNHURT, mdef);
                            dam = 0;
                        }
                        golemeffects(mdef,(int)mattk->adtyp,dam);
                    } else dam = 0;
                    break;
                case AD_COLD:
                    if (rn2(2)) {
                        if (resists_cold(mdef)) {
                            message_monster(MSG_M_SEEMS_MILDLY_CHILLY, mdef);
                            dam = 0;
                        } else {
                            message_monster(MSG_M_IS_FREEZING_TO_DEATH, mdef);
                        }
                        golemeffects(mdef,(int)mattk->adtyp,dam);
                    } else dam = 0;
                    break;
                case AD_FIRE:
                    if (rn2(2)) {
                        if (resists_fire(mdef)) {
                            message_monster(MSG_M_SEEMS_MIDLY_HOT, mdef);
                            dam = 0;
                        } else {
                            message_monster(MSG_M_IS_BURNING_TO_A_CRISP, mdef);
                        }
                        golemeffects(mdef,(int)mattk->adtyp,dam);
                    } else dam = 0;
                    break;
            }
            end_engulf();
            if ((mdef->mhp -= dam) <= 0) {
                killed(mdef);
                if (mdef->mhp <= 0) /* not lifesaved */
                    return (2);
            }
            You("%s %s!", is_animal(youmonst.data) ? "regurgitate" : "expel", "TODO:mon_nam(mdef)");
            if (Slow_digestion || is_animal(youmonst.data)) {
                message_monster(MSG_YOU_DID_NOT_LIKE_M_TASTE, mdef);
            }
        } else {
            char kbuf[BUFSZ];

            message_monster(MSG_YOU_BITE_INTO_M, mdef);
            sprintf(kbuf, "swallowing %s whole", an(mdef->data->mname));
            instapetrify(kbuf);
        }
    }
    return (0);
}

static bool hmonas( /* attack monster as a monster. */
        struct monst *mon, int tmp) {
    struct attack *mattk, alt_attk;
    int i, sum[NATTK], hittmp = 0;
    int nsum = 0;
    int dhit = 0;

    for (i = 0; i < NATTK; i++) {

        sum[i] = 0;
        mattk = getmattk(youmonst.data, i, sum, &alt_attk);
        switch (mattk->aatyp) {
            case AT_WEAP:
                use_weapon:
                /* Certain monsters don't use weapons when encountered as enemies,
                 * but players who polymorph into them have hands or claws and thus
                 * should be able to use weapons.  This shouldn't prohibit the use
                 * of most special abilities, either.
                 */
                /* Potential problem: if the monster gets multiple weapon attacks,
                 * we currently allow the player to get each of these as a weapon
                 * attack.  Is this really desirable?
                 */
                if (uwep) {
                    hittmp = hitval(uwep, mon);
                    hittmp += weapon_hit_bonus(uwep);
                    tmp += hittmp;
                }
            dhit = (tmp > (dieroll = rnd(20)) || u.uswallow);
            /* KMH -- Don't accumulate to-hit bonuses */
            if (uwep)
                tmp -= hittmp;
            /* Enemy dead, before any special abilities used */
            if (!known_hitum(mon, &dhit, mattk)) {
                sum[i] = 2;
                break;
            } else
                sum[i] = dhit;
            /* might be a worm that gets cut in half */
            if (m_at(u.ux+u.dx, u.uy+u.dy) != mon)
                return ((bool)(nsum != 0));
            /* Do not print "You hit" message, since known_hitum
             * already did it.
             */
            if (dhit && mattk->adtyp != AD_SPEL && mattk->adtyp != AD_PHYS)
                sum[i] = damageum(mon, mattk);
            break;
            case AT_CLAW:
                if (i == 0 && uwep && !cantwield(youmonst.data))
                    goto use_weapon;
                /* succubi/incubi are humanoid, but their _second_
                 * attack is AT_CLAW, not their first...
                 */
                if (i == 1 && uwep && (u.umonnum == PM_SUCCUBUS || u.umonnum == PM_INCUBUS))
                    goto use_weapon;
            case AT_KICK:
            case AT_BITE:
            case AT_STNG:
            case AT_TUCH:
            case AT_BUTT:
            case AT_TENT:
                if (i == 0 && uwep && (youmonst.data->mlet == S_LICH))
                    goto use_weapon;
                if ((dhit = (tmp > rnd(20) || u.uswallow)) != 0) {
                    int compat;

                    if (!u.uswallow && (compat = could_seduce(&youmonst, mon, mattk))) {
                        You("%s %s %s.", mon->mcansee &&
                                haseyes(mon->data) ? "smile at" : "talk to",
                                "TODO:mon_nam(mon)", compat == 2 ? "engagingly" : "seductively");
                        /* doesn't anger it; no wakeup() */
                        sum[i] = damageum(mon, mattk);
                        break;
                    }
                    wakeup(mon);
                    /* maybe this check should be in damageum()? */
                    if (mon->data == &mons[PM_SHADE] && !(mattk->aatyp == AT_KICK && uarmf && uarmf->blessed)) {
                        message_monster(MSG_YOUR_ATTACK_PASSES_HARMLESSLY_THROUGH_M, mon);
                        break;
                    }
                    if (mattk->aatyp == AT_KICK) {
                        message_monster(MSG_YOU_KICK_M, mon);
                    } else if (mattk->aatyp == AT_BITE) {
                        message_monster(MSG_YOU_BITE_M, mon);
                    } else if (mattk->aatyp == AT_STNG) {
                        message_monster(MSG_YOU_STING_M, mon);
                    } else if (mattk->aatyp == AT_BUTT) {
                        message_monster(MSG_YOU_BUTT_M, mon);
                    } else if (mattk->aatyp == AT_TUCH) {
                        message_monster(MSG_YOU_TOUCH_M, mon);
                    } else if (mattk->aatyp == AT_TENT) {
                        message_monster(MSG_YOUR_TENTACLES_SUCK_M, mon);
                    } else {
                        message_monster(MSG_YOU_HIT_M, mon);
                    }
                    sum[i] = damageum(mon, mattk);
                } else
                    missum(mon, mattk);
                break;

            case AT_HUGS:
                /* automatic if prev two attacks succeed, or if
                 * already grabbed in a previous attack
                 */
                dhit = 1;
                wakeup(mon);
                if (mon->data == &mons[PM_SHADE]) {
                    message_monster(MSG_YOUR_HUG_PASSES_HARMLESSLY_THROUGH_M, mon);
                } else if (!sticks(mon->data) && !u.uswallow) {
                    if (mon == u.ustuck) {
                        pline("%s is being %s.", "TODO:Monnam(mon)", u.umonnum == PM_ROPE_GOLEM ? "choked" : "crushed");
                        sum[i] = damageum(mon, mattk);
                    } else if (i >= 2 && sum[i - 1] && sum[i - 2]) {
                        message_monster(MSG_YOU_GRAB_M, mon);
                        u.ustuck = mon;
                        sum[i] = damageum(mon, mattk);
                    }
                }
                break;

            case AT_EXPL: /* automatic hit if next to */
                dhit = -1;
                wakeup(mon);
                sum[i] = explum(mon, mattk);
                break;

            case AT_ENGL:
                if ((dhit = (tmp > rnd(20 + i)))) {
                    wakeup(mon);
                    if (mon->data == &mons[PM_SHADE]) {
                        message_monster(MSG_YOUR_ATTEMPT_TO_SURROUND_M_IS_HARMLESS, mon);
                    } else {
                        sum[i] = gulpum(mon, mattk);
                        if (sum[i] == 2 && (mon->data->mlet == S_ZOMBIE || mon->data->mlet == S_MUMMY) && rn2(5) && !Sick_resistance) {
                            You_feel("%ssick.", (Sick) ? "very " : "");
                            mdamageu(mon, rnd(8));
                        }
                    }
                } else
                    missum(mon, mattk);
                break;

            case AT_MAGC:
                /* No check for uwep; if wielding nothing we want to
                 * do the normal 1-2 points bare hand damage...
                 */
                if (i==0 && (youmonst.data->mlet==S_KOBOLD
                        || youmonst.data->mlet==S_ORC
                        || youmonst.data->mlet==S_GNOME
                )) goto use_weapon;

            case AT_NONE:
            case AT_BOOM:
                continue;
                /* Not break--avoid passive attacks from enemy */

            case AT_BREA:
            case AT_SPIT:
            case AT_GAZE: /* all done using #monster command */
                dhit = 0;
                break;

            default: /* Strange... */
                impossible("strange attack of yours (%d)",
                        mattk->aatyp);
        }
        if (dhit == -1) {
            u.mh = -1; /* dead in the current form */
            rehumanize();
        }
        if (sum[i] == 2)
            return ((bool)passive(mon, 1, 0, mattk->aatyp));
        /* defender dead */
        else {
            (void)passive(mon, sum[i], 1, mattk->aatyp);
            nsum |= sum[i];
        }
        if (!Upolyd)
            break; /* No extra attacks if no longer a monster */
        if (multi < 0)
            break; /* If paralyzed while attacking, i.e. floating eye */
    }
    return ((bool)(nsum != 0));
}

static bool hitum( /* returns true if monster still lives */
        struct monst *mon, int tmp, struct attack *uattk) {
    bool malive;
    int mhit = (tmp > (dieroll = rnd(20)) || u.uswallow);

    if (tmp > dieroll)
        exercise(A_DEX, true);
    malive = known_hitum(mon, &mhit, uattk);
    (void)passive(mon, mhit, malive, AT_WEAP);
    return (malive);
}

/* try to attack; return false if monster evaded */
/* u.dx and u.dy must be set */
bool attack(struct monst *mtmp) {
    signed char tmp;
    struct permonst *mdat = mtmp->data;

    /* This section of code provides protection against accidentally
     * hitting peaceful (like '@') and tame (like 'd') monsters.
     * Protection is provided as long as player is not: blind, confused,
     * hallucinating or stunned.
     * changes by wwp 5/16/85
     * More changes 12/90, -dkh-. if its tame and safepet, (and protected
     * 07/92) then we assume that you're not trying to attack. Instead,
     * you'll usually just swap places if this is a movement command
     */
    /* Intelligent chaotic weapons (Stormbringer) want blood */
    if (is_safepet(mtmp) && !flags.forcefight) {
        if (!uwep || uwep->oartifact != ART_STORMBRINGER) {
            /* there are some additional considerations: this won't work
             * if in a shop or Punished or you miss a random roll or
             * if you can walk thru walls and your pet cannot (KAA) or
             * if your pet is a long worm (unless someone does better).
             * there's also a chance of displacing a "frozen" monster.
             * sleeping monsters might magically walk in their sleep.
             */
            bool foo = (Punished || !rn2(7) || is_longworm(mtmp->data)), inshop = false;
            char *p;

            for (p = in_rooms(mtmp->mx, mtmp->my, SHOPBASE); *p; p++)
                if (tended_shop(&rooms[*p - ROOMOFFSET])) {
                    inshop = true;
                    break;
                }

            if (inshop || foo || (IS_ROCK(levl[u.ux][u.uy].typ) && !passes_walls(mtmp->data))) {
                char buf[BUFSZ];

                monflee(mtmp, rnd(6), false, false);
                y_monnam(buf, BUFSZ, mtmp);
                buf[0] = highc(buf[0]);
                You("stop.  %s is in the way!", buf);
                return true;
            } else if ((mtmp->mfrozen || (!mtmp->mcanmove) || (mtmp->data->mmove == 0)) && rn2(6)) {
                char name[BUFSZ];
                Monnam(name, BUFSZ, mtmp);
                pline("%s doesn't seem to move!", name);
                return true;
            } else
                return false;
        }
    }

    /* possibly set in attack_checks;
     examined in known_hitum, called via hitum or hmonas below */
    override_confirmation = false;
    if (attack_checks(mtmp, uwep))
        return (true);

    if (Upolyd) {
        /* certain "pacifist" monsters don't attack */
        if (noattacks(youmonst.data)) {
            You("have no way to attack monsters physically.");
            mtmp->mstrategy &= ~STRAT_WAITMASK;
            goto atk_done;
        }
    }

    if (check_capacity("You cannot fight while so heavily loaded."))
        goto atk_done;

    if (u.twoweap && !can_twoweapon())
        untwoweapon();

    if (unweapon) {
        unweapon = false;
        if (flags.verbose) {
            if (uwep)
                You("begin bashing monsters with your %s.", aobjnam(uwep, (char *)0));
            else if (!cantwield(youmonst.data))
                You("begin %sing monsters with your %s %s.",
                        Role_if(PM_MONK) ? "strik" : "bash", uarmg ? "gloved" : "bare", /* Del Lamb */
                                makeplural(body_part(HAND)));
        }
    }
    exercise(A_STR, true); /* you're exercising muscles */
    /* andrew@orca: prevent unlimited pick-axe attacks */
    u_wipe_engr(3);

    /* Is the "it died" check actually correct? */
    if (mdat->mlet == S_LEPRECHAUN && !mtmp->mfrozen && !mtmp->msleeping && !mtmp->mconf && mtmp->mcansee && !rn2(7) && (m_move(mtmp, 0) == 2 || /* it died */
            mtmp->mx != u.ux + u.dx || mtmp->my != u.uy + u.dy)) /* it moved */
        return (false);

    tmp = find_roll_to_hit(mtmp);
    if (Upolyd)
        hmonas(mtmp, tmp);
    else
        hitum(mtmp, tmp, youmonst.data->mattk);
    mtmp->mstrategy &= ~STRAT_WAITMASK;

    atk_done:
    /* see comment in attack_checks() */
    /* we only need to check for this if we did an attack_checks()
     * and it returned 0 (it's okay to attack), and the monster didn't
     * evade.
     */
    if (flags.forcefight && mtmp->mhp > 0 && !canspotmon(mtmp) && !glyph_is_invisible(levl[u.ux+u.dx][u.uy+u.dy].glyph) && !(u.uswallow && mtmp == u.ustuck))
        map_invisible(u.ux + u.dx, u.uy + u.dy);

    return (true);
}

/* used when hitting a monster with a lance while mounted */
/* mon: target */
/* obj: weapon */
static int joust(struct monst *mon, struct obj *obj) {
    int skill_rating, joust_dieroll;

    if (Fumbling || Stunned)
        return 0;
    /* sanity check; lance must be wielded in order to joust */
    if (obj != uwep && (obj != uswapwep || !u.twoweap))
        return 0;

    /* if using two weapons, use worse of lance and two-weapon skills */
    skill_rating = P_SKILL(weapon_type(obj)); /* lance skill */
    if (u.twoweap && P_SKILL(P_TWO_WEAPON_COMBAT) < skill_rating)
        skill_rating = P_SKILL(P_TWO_WEAPON_COMBAT);
    if (skill_rating == P_ISRESTRICTED)
        skill_rating = P_UNSKILLED; /* 0=>1 */

    /* odds to joust are expert:80%, skilled:60%, basic:40%, unskilled:20% */
    if ((joust_dieroll = rn2(5)) < skill_rating) {
        if (joust_dieroll == 0 && rnl(50) == (50 - 1) && !unsolid(mon->data) && !obj_resists(obj, 0, 100))
            return -1; /* hit that breaks lance */
        return 1; /* successful joust */
    }
    return 0; /* no joust bonus; revert to ordinary attack */
}

static bool shade_aware(struct obj *obj) {
    if (!obj)
        return false;
    /*
     * The things in this list either
     * 1) affect shades.
     *  OR
     * 2) are dealt with properly by other routines
     *    when it comes to shades.
     */
    if (obj->otyp == BOULDER || obj->otyp == HEAVY_IRON_BALL || obj->otyp == IRON_CHAIN /* dmgval handles those first three */
            || obj->otyp == MIRROR /* silver in the reflective surface */
            || obj->otyp == CLOVE_OF_GARLIC /* causes shades to flee */
            || objects[obj->otyp].oc_material == SILVER)
        return true;
    return false;
}

static void nohandglow(struct monst *mon) {
    char *hands = makeplural(body_part(HAND));

    if (!u.umconf || mon->mconf)
        return;
    if (u.umconf == 1) {
        if (Blind)
            Your("%s stop tingling.", hands);
        else
            Your("%s stop glowing %s.", hands, hcolor(NH_RED));
    } else {
        if (Blind)
            pline_The("tingling in your %s lessens.", hands);
        else
            Your("%s no longer glow so brightly %s.", hands, hcolor(NH_RED));
    }
    u.umconf--;
}

/* guts of hmon() */
static bool hmon_hitmon(struct monst *mon, struct obj *obj, int thrown) {
    int tmp;
    struct permonst *mdat = mon->data;
    int barehand_silver_rings = 0;
    /* The basic reason we need all these booleans is that we don't want
     * a "hit" message when a monster dies, so we have to know how much
     * damage it did _before_ outputting a hit message, but any messages
     * associated with the damage don't come out until _after_ outputting
     * a hit message.
     */
    bool hittxt = false, destroyed = false, already_killed = false;
    bool get_dmg_bonus = true;
    bool ispoisoned = false, needpoismsg = false, poiskilled = false;
    bool silvermsg = false, silverobj = false;
    bool valid_weapon_attack = false;
    bool unarmed = !uwep && !uarm && !uarms;
    int jousting = 0;
    int wtype;
    struct obj *monwep;
    char yourbuf[BUFSZ];
    struct obj * unconventional = NULL;
    char saved_oname[BUFSZ];

    saved_oname[0] = '\0';

    wakeup(mon);
    if (!obj) { /* attack with bare hands */
        if (mdat == &mons[PM_SHADE])
            tmp = 0;
        else if (martial_bonus())
            tmp = rnd(4); /* bonus for martial arts */
        else
            tmp = rnd(2);
        valid_weapon_attack = (tmp > 1);
        /* blessed gloves give bonuses when fighting 'bare-handed' */
        if (uarmg && uarmg->blessed && (is_undead(mdat) || is_demon(mdat)))
            tmp += rnd(4);
        /* So do silver rings.  Note: rings are worn under gloves, so you
         * don't get both bonuses.
         */
        if (!uarmg) {
            if (uleft && objects[uleft->otyp].oc_material == SILVER)
                barehand_silver_rings++;
            if (uright && objects[uright->otyp].oc_material == SILVER)
                barehand_silver_rings++;
            if (barehand_silver_rings && hates_silver(mdat)) {
                tmp += rnd(20);
                silvermsg = true;
            }
        }
    } else {
        strcpy(saved_oname, cxname(obj));
        if (obj->oclass == WEAPON_CLASS || is_weptool(obj) || obj->oclass == GEM_CLASS) {

            /* is it not a melee weapon? */
            if (/* if you strike with a bow... */
                    is_launcher(obj) ||
                    /* or strike with a missile in your hand... */
                    (!thrown && (is_missile(obj) || is_ammo(obj))) ||
                    /* or use a pole at short range and not mounted... */
                    (!thrown && !u.usteed && is_pole(obj)) ||
                    /* or throw a missile without the proper bow... */
                    (is_ammo(obj) && !ammo_and_launcher(obj, uwep))) {
                /* then do only 1-2 points of damage */
                if (mdat == &mons[PM_SHADE] && obj->otyp != SILVER_ARROW)
                    tmp = 0;
                else
                    tmp = rnd(2);
                if (!thrown && obj == uwep && obj->otyp == BOOMERANG && rnl(4) == 4 - 1) {
                    bool more_than_1 = (obj->quan > 1L);

                    char name[BUFSZ];
                    mon_nam(name, BUFSZ, mon);
                    pline("As you hit %s, %s%s %s breaks into splinters.", name, more_than_1 ? "one of " : "", shk_your(yourbuf, obj), xname(obj));
                    if (!more_than_1)
                        uwepgone(); /* set unweapon */
                    useup(obj);
                    if (!more_than_1)
                        obj = (struct obj *)0;
                    hittxt = true;
                    if (mdat != &mons[PM_SHADE])
                        tmp++;
                }
            } else {
                tmp = dmgval(obj, mon);
                /* a minimal hit doesn't exercise proficiency */
                valid_weapon_attack = (tmp > 1);
                if (!valid_weapon_attack || mon == u.ustuck || u.twoweap) {
                    ; /* no special bonuses */
                } else if (mon->mflee && Role_if(PM_ROGUE) && !Upolyd) {
                    char name[BUFSZ];
                    mon_nam(name, BUFSZ, mon);
                    You("strike %s from behind!", name);
                    tmp += rnd(u.ulevel);
                    hittxt = true;
                } else if (dieroll == 2 && obj == uwep && obj->oclass == WEAPON_CLASS && (bimanual(obj) || (Role_if(PM_SAMURAI) && obj->otyp == KATANA && !uarms)) && ((wtype = uwep_skill_type()) != P_NONE &&
                        P_SKILL(wtype) >= P_SKILLED) && ((monwep = MON_WEP(mon)) != 0 && !is_flimsy(monwep) && !obj_resists(monwep, 50 + 15 * greatest_erosion(obj), 100))) {
                    /*
                     * 2.5% chance of shattering defender's weapon when
                     * using a two-handed weapon; less if uwep is rusted.
                     * [dieroll == 2 is most successful non-beheading or
                     * -bisecting hit, in case of special artifact damage;
                     * the percentage chance is (1/20)*(50/100).]
                     */
                    setmnotwielded(mon, monwep);
                    MON_NOWEP(mon);
                    mon->weapon_check = NEED_WEAPON;
                    char name[BUFSZ];
                    Monnam(name, BUFSZ, mon);
                    char it_shatters[BUFSZ];
                    otense(it_shatters, BUFSZ, monwep, "shatter");
                    pline("%s%s %s %s from the force of your blow!", name, possessive_suffix(name), xname(monwep), it_shatters);
                    m_useup(mon, monwep);
                    /* If someone just shattered MY weapon, I'd flee! */
                    if (rn2(4)) {
                        monflee(mon, d(2, 3), true, true);
                    }
                    hittxt = true;
                }

                if (obj->oartifact && artifact_hit(&youmonst, mon, obj, &tmp, dieroll)) {
                    if (mon->mhp <= 0) /* artifact killed monster */
                        return false;
                    if (tmp == 0)
                        return true;
                    hittxt = true;
                }
                if (objects[obj->otyp].oc_material == SILVER && hates_silver(mdat)) {
                    silvermsg = true;
                    silverobj = true;
                }
                if (u.usteed && !thrown && tmp > 0 && weapon_type(obj) == P_LANCE && mon != u.ustuck) {
                    jousting = joust(mon, obj);
                    /* exercise skill even for minimal damage hits */
                    if (jousting)
                        valid_weapon_attack = true;
                }
                if (thrown && (is_ammo(obj) || is_missile(obj))) {
                    if (ammo_and_launcher(obj, uwep)) {
                        /* Elves and Samurai do extra damage using
                         * their bows&arrows; they're highly trained.
                         */
                        if (Role_if(PM_SAMURAI) && obj->otyp == YA && uwep->otyp == YUMI)
                            tmp++;
                        else if (Race_if(PM_ELF) && obj->otyp == ELVEN_ARROW && uwep->otyp == ELVEN_BOW)
                            tmp++;
                    }
                    if (obj->opoisoned && is_poisonable(obj))
                        ispoisoned = true;
                }
            }
        } else if (obj->oclass == POTION_CLASS) {
            if (obj->quan > 1L)
                obj = splitobj(obj, 1L);
            else
                setuwep((struct obj *)0);
            freeinv(obj);
            potionhit(mon, obj, true);
            if (mon->mhp <= 0)
                return false; /* killed */
            hittxt = true;
            /* in case potion effect causes transformation */
            mdat = mon->data;
            tmp = (mdat == &mons[PM_SHADE]) ? 0 : 1;
        } else {
            if (mdat == &mons[PM_SHADE] && !shade_aware(obj)) {
                unconventional = obj;
                tmp = 0;
            } else {
                switch (obj->otyp) {
                    case BOULDER: /* 1d20 */
                    case HEAVY_IRON_BALL: /* 1d25 */
                    case IRON_CHAIN: /* 1d4+1 */
                        tmp = dmgval(obj, mon);
                        break;
                    case MIRROR:
                        if (breaktest(obj)) {
                            You("break %s mirror.  That's bad luck!", shk_your(yourbuf, obj));
                            change_luck(-2);
                            useup(obj);
                            obj = (struct obj *)0;
                            unarmed = false; /* avoid obj==0 confusion */
                            get_dmg_bonus = false;
                            hittxt = true;
                        }
                        tmp = 1;
                        break;
                    case EXPENSIVE_CAMERA:
                        You("succeed in destroying %s camera.  Congratulations!", shk_your(yourbuf, obj));
                        useup(obj);
                        return (true);
                        /*NOTREACHED*/
                        break;
                    case CORPSE: /* fixed by polder@cs.vu.nl */
                        if (touch_petrifies(&mons[obj->corpsenm])) {
                            static const char withwhat[] = "corpse";
                            tmp = 1;
                            hittxt = true;
                            char name[BUFSZ];
                            mon_nam(name, BUFSZ, mon);
                            You("hit %s with %s %s.", name, obj->dknown ? the(mons[obj->corpsenm].mname) : an(mons[obj->corpsenm].mname), (obj->quan > 1) ? makeplural(withwhat) : withwhat);
                            if (!munstone(mon, true))
                                minstapetrify(mon, true);
                            if (resists_ston(mon))
                                break;
                            /* note: hp may be <= 0 even if munstoned==true */
                            return (bool)(mon->mhp > 0);
                        }
                        tmp = (obj->corpsenm >= LOW_PM ? mons[obj->corpsenm].msize : 0) + 1;
                        break;
                    case EGG: {
                        long cnt = obj->quan;

                        tmp = 1; /* nominal physical damage */
                        get_dmg_bonus = false;
                        hittxt = true; /* message always given */
                        /* egg is always either used up or transformed, so next
                         hand-to-hand attack should yield a "bashing" mesg */
                        if (obj == uwep)
                            unweapon = true;
                        if (obj->spe && obj->corpsenm >= LOW_PM) {
                            if (obj->quan < 5)
                                change_luck((signed char)-(obj->quan));
                            else
                                change_luck(-5);
                        }

                        if (touch_petrifies(&mons[obj->corpsenm])) {
                            /*learn_egg_type(obj->corpsenm);*/
                            char name[BUFSZ];
                            mon_nam(name, BUFSZ, mon);
                            pline("Splat! You hit %s with %s %s egg%s!", name, obj->known ? "the" : cnt > 1L ? "some" : "a", obj->known ? mons[obj->corpsenm].mname : "petrifying", plur(cnt));
                            obj->known = 1; /* (not much point...) */
                            obj = useup_eggs(thrown, obj);
                            if (!munstone(mon, true))
                                minstapetrify(mon, true);
                            if (resists_ston(mon))
                                break;
                            return (bool)(mon->mhp > 0);
                        } else { /* ordinary egg(s) */
                            const char *eggp = (obj->corpsenm != NON_PM && obj->known) ? the(mons[obj->corpsenm].mname) : (cnt > 1L) ? "some" : "an";
                            char name[BUFSZ];
                            mon_nam(name, BUFSZ, mon);
                            You("hit %s with %s egg%s.", name, eggp, plur(cnt));
                            if (touch_petrifies(mdat) && !stale_egg(obj)) {
                                pline_The("egg%s %s alive any more...", plur(cnt), (cnt == 1L) ? "isn't" : "aren't");
                                if (obj->timed)
                                    obj_stop_timers(obj);
                                obj->otyp = ROCK;
                                obj->oclass = GEM_CLASS;
                                obj->oartifact = 0;
                                obj->spe = 0;
                                obj->known = obj->dknown = obj->bknown = 0;
                                obj->owt = weight(obj);
                                if (thrown)
                                    place_object(obj, mon->mx, mon->my);
                            } else {
                                pline("Splat!");
                                obj = useup_eggs(thrown, obj);
                                exercise(A_WIS, false);
                            }
                        }
                        break;
                    }
                    case CLOVE_OF_GARLIC: /* no effect against demons */
                        if (is_undead(mdat)) {
                            monflee(mon, d(2, 4), false, true);
                        }
                        tmp = 1;
                        break;
                    case CREAM_PIE:
                    case BLINDING_VENOM:
                        mon->msleeping = 0;
                        if (can_blnd(&youmonst, mon, (unsigned char)(obj->otyp == BLINDING_VENOM ? AT_SPIT : AT_WEAP), obj)) {
                            if (Blind) {
                                pline(obj->otyp == CREAM_PIE ? "Splat!" : "Splash!");
                            } else if (obj->otyp == BLINDING_VENOM) {
                                char name[BUFSZ];
                                mon_nam(name, BUFSZ, mon);
                                pline_The("venom blinds %s%s!", name, mon->mcansee ? "" : " further");
                            } else {
                                char whom[BUFSZ];
                                mon_nam(whom, BUFSZ, mon);
                                char *what = The(xname(obj));
                                if (!thrown && obj->quan > 1)
                                    what = An(singular(obj, xname));
                                char face[BUFSZ];
                                face[0] = '\0';
                                if (haseyes(mdat) && mdat != &mons[PM_FLOATING_EYE])
                                    strcat(strcat(strcat(face, possessive_suffix(whom)), " "), mbodypart(mon, FACE));
                                char splashes[BUFSZ];
                                vtense(splashes, BUFSZ, what, "splash");
                                pline("%s %s over %s%s!", what, splashes, whom, face);
                            }
                            setmangry(mon);
                            mon->mcansee = 0;
                            tmp = rn1(25, 21);
                            if (((int)mon->mblinded + tmp) > 127)
                                mon->mblinded = 127;
                            else
                                mon->mblinded += tmp;
                        } else {
                            plines(obj->otyp == CREAM_PIE ? "Splat!" : "Splash!");
                            setmangry(mon);
                        }
                        if (thrown)
                            obfree(obj, (struct obj *)0);
                        else
                            useup(obj);
                        hittxt = true;
                        get_dmg_bonus = false;
                        tmp = 0;
                        break;
                    case ACID_VENOM: { /* thrown (or spit) */
                        char name[BUFSZ];
                        mon_nam(name, BUFSZ, mon);
                        if (resists_acid(mon)) {
                            Your("venom hits %s harmlessly.", name);
                            tmp = 0;
                        } else {
                            Your("venom burns %s!", name);
                            tmp = dmgval(obj, mon);
                        }
                        if (thrown)
                            obfree(obj, (struct obj *)0);
                        else
                            useup(obj);
                        hittxt = true;
                        get_dmg_bonus = false;
                        break;
                    }
                    default:
                        /* non-weapons can damage because of their weight */
                        /* (but not too much) */
                        tmp = obj->owt / 100;
                        if (tmp < 1)
                            tmp = 1;
                        else
                            tmp = rnd(tmp);
                        if (tmp > 6)
                            tmp = 6;
                        /*
                         * Things like silver wands can arrive here so
                         * so we need another silver check.
                         */
                        if (objects[obj->otyp].oc_material == SILVER && hates_silver(mdat)) {
                            tmp += rnd(20);
                            silvermsg = true;
                            silverobj = true;
                        }
                }
            }
        }
    }

    /****** NOTE: perhaps obj is undefined!! (if !thrown && BOOMERANG)
     *      *OR* if attacking bare-handed!! */

    if (get_dmg_bonus && tmp > 0) {
        tmp += u.udaminc;
        /* If you throw using a propellor, you don't get a strength
         * bonus but you do get an increase-damage bonus.
         */
        if (!thrown || !obj || !uwep || !ammo_and_launcher(obj, uwep))
            tmp += dbon();
    }

    if (valid_weapon_attack) {
        struct obj *wep;

        /* to be valid a projectile must have had the correct projector */
        wep = PROJECTILE(obj) ? uwep : obj;
        tmp += weapon_dam_bonus(wep);
        /* [this assumes that `!thrown' implies wielded...] */
        wtype = thrown ? weapon_type(wep) : uwep_skill_type();
        use_skill(wtype, 1);
    }

    if (ispoisoned) {
        int nopoison = (10 - (obj->owt / 10));
        if (nopoison < 2)
            nopoison = 2;
        if (Role_if(PM_SAMURAI)) {
            You("dishonorably use a poisoned weapon!");
            adjalign(-sgn(u.ualign.type));
        } else if ((u.ualign.type == A_LAWFUL) && (u.ualign.record > -10)) {
            You_feel("like an evil coward for using a poisoned weapon.");
            adjalign(-1);
        }
        if (obj && !rn2(nopoison)) {
            obj->opoisoned = false;
            message_object(MSG_YOUR_OBJECT_IS_NO_LONGER_POISONED, obj);
        }
        if (resists_poison(mon))
            needpoismsg = true;
        else if (rn2(10))
            tmp += rnd(6);
        else
            poiskilled = true;
    }
    if (tmp < 1) {
        /* make sure that negative damage adjustment can't result
         in inadvertently boosting the victim's hit points */
        tmp = 0;
        if (mdat == &mons[PM_SHADE]) {
            if (!hittxt) {
                if (unconventional == NULL)
                    message_monster(MSG_YOUR_ATTACK_PASSES_HARMLESSLY_THROUGH_MONSTER, mon);
                else
                    message_monster_object(MSG_YOUR_WEAPON_PASSES_HARMLESSLY_THROUGH_MONSTER, mon, unconventional);
                hittxt = true;
            }
        } else {
            if (get_dmg_bonus)
                tmp = 1;
        }
    }

    if (jousting) {
        tmp += d(2, (obj == uwep) ? 10 : 2); /* [was in dmgval()] */
        message_monster_string(MSG_YOU_JOUST_IT, mon, canseemon(mon) ? exclam(tmp) : ".");
        if (jousting < 0) {
            message_object(MSG_YOUR_LANCE_SHATTERS, obj);
            /* (must be either primary or secondary weapon to get here) */
            u.twoweap = false; /* untwoweapon() is too verbose here */
            if (obj == uwep)
                uwepgone(); /* set unweapon */
            /* minor side-effect: broken lance won't split puddings */
            useup(obj);
            obj = 0;
        }
        /* avoid migrating a dead monster */
        if (mon->mhp > tmp) {
            mhurtle(mon, u.dx, u.dy, 1);
            mdat = mon->data; /* in case of a polymorph trap */
            if (DEADMONSTER(mon))
                already_killed = true;
        }
        hittxt = true;
    } else

        /* VERY small chance of stunning opponent if unarmed. */
        if (unarmed && tmp > 1 && !thrown && !obj && !Upolyd) {
            if (rnd(100) < P_SKILL(P_BARE_HANDED_COMBAT) && !bigmonst(mdat) && !thick_skinned(mdat)) {
                if (canspotmon(mon))
                    message_monster(MSG_M_STAGGERS_FROM_YOUR_POWERFUL_STRIKE, mon);
                /* avoid migrating a dead monster */
                if (mon->mhp > tmp) {
                    mhurtle(mon, u.dx, u.dy, 1);
                    mdat = mon->data; /* in case of a polymorph trap */
                    if (DEADMONSTER(mon))
                        already_killed = true;
                }
                hittxt = true;
            }
        }

    if (!already_killed)
        mon->mhp -= tmp;
    /* adjustments might have made tmp become less than what
     a level draining artifact has already done to max HP */
    if (mon->mhp > mon->mhpmax)
        mon->mhp = mon->mhpmax;
    if (mon->mhp < 1)
        destroyed = true;
    if (mon->mtame && (!mon->mflee || mon->mfleetim) && tmp > 0) {
        abuse_dog(mon);
        monflee(mon, 10 * rnd(tmp), false, false);
    }
    if ((mdat == &mons[PM_BLACK_PUDDING] || mdat == &mons[PM_BROWN_PUDDING]) && obj && obj == uwep && objects[obj->otyp].oc_material == IRON && mon->mhp > 1 && !thrown && !mon->mcan
    /* && !destroyed  -- guaranteed by mhp > 1 */) {
        if (clone_mon(mon, 0, 0)) {
            message_monster(MSG_M_DIVIDES_AS_YOU_HIT_IT, mon);
            hittxt = true;
        }
    }

    if (!hittxt && /*( thrown => obj exists )*/
            (!destroyed || (thrown && m_shot.n > 1 && m_shot.o == obj->otyp))) {
        if (thrown)
            hit(mshot_xname(obj), mon, exclam(tmp));
        else if (!flags.verbose)
            You("hit it.");
        else
            message_monster_string(MSG_YOU_HIT_M, mon, canseemon(mon) ? exclam(tmp) : ".");
    }

    if (silvermsg) {
        if (canspotmon(mon)) {
            if (barehand_silver_rings == 1 || barehand_silver_rings == 2) {
                message_monster(MSG_YOUR_SILVER_RING_SEARS_M_FLESH, mon);
            } else if (silverobj && saved_oname[0]) {
                char silverobjbuf[BUFSZ];
                sprintf(silverobjbuf, "Your %s%s %s %%s!", strstri(saved_oname, "silver") ? "" : "silver ", saved_oname, "TODO: vtense(saved_oname, \"sear\")");
                char *whom = "TODO: mon_nam(mon)";
                // TODO:
                // if (!noncorporeal(mdat))
                //     whom = strcat(s_suffix(whom), " flesh");
                pline(silverobjbuf, whom);
            } else {
                if (noncorporeal(mdat))
                    message_monster(MSG_THE_SILVER_SEARS_M, mon);
                else
                    message_monster(MSG_THE_SILVER_SEARS_M_FLESH, mon);
            }
        } else {
            if (noncorporeal(mdat))
                message_const(MSG_IT_IS_SEARED);
            else
                message_const(MSG_ITS_FLESH_IS_SEARED);
        }
    }

    if (needpoismsg)
        message_monster(MSG_THE_POISON_DOESNT_SEEM_TO_AFFECT_M, mon);
    if (poiskilled) {
        message_const(MSG_THE_POISON_WAS_DEADLY);
        if (!already_killed)
            xkilled(mon, 0);
        return false;
    } else if (destroyed) {
        if (!already_killed)
            killed(mon); /* takes care of most messages */
    } else if (u.umconf && !thrown) {
        nohandglow(mon);
        if (!mon->mconf && !resist(mon, SPBOOK_CLASS, 0, NOTELL)) {
            mon->mconf = 1;
            if (!mon->mstun && mon->mcanmove && !mon->msleeping && canseemon(mon))
                message_monster(MSG_M_APPEARS_CONFUSED, mon);
        }
    }

    return ((bool)(destroyed ? false : true));
}

bool hmon( /* return true if mon still alive */
        struct monst *mon, struct obj *obj, int thrown) {
    bool result, anger_guards;

    anger_guards = (mon->mpeaceful && (mon->ispriest || mon->isshk || mon->data == &mons[PM_WATCHMAN] || mon->data == &mons[PM_WATCH_CAPTAIN]));
    result = hmon_hitmon(mon, obj, thrown);
    if (mon->ispriest && !rn2(2))
        ghod_hitsu(mon);
    if (anger_guards)
        (void)angry_guards(!flags.soundok);
    return result;
}

/* check whether slippery clothing protects from hug or wrap attack */
/* [currently assumes that you are the attacker] */
static bool m_slips_free(struct monst *mdef, struct attack *mattk) {
    struct obj *obj;

    if (mattk->adtyp == AD_DRIN) {
        /* intelligence drain attacks the head */
        obj = which_armor(mdef, W_ARMH);
    } else {
        /* grabbing attacks the body */
        obj = which_armor(mdef, W_ARMC); /* cloak */
        if (!obj)
            obj = which_armor(mdef, W_ARM); /* suit */
        if (!obj)
            obj = which_armor(mdef, W_ARMU); /* shirt */
    }

    /* if your cloak/armor is greased, monster slips off; this
     protection might fail (33% chance) when the armor is cursed */
    if (obj && (obj->greased || obj->otyp == OILSKIN_CLOAK) && (!obj->cursed || rn2(3))) {
        if (mattk->adtyp == AD_WRAP) {
            if (obj->greased)
                message_monster_object(MSG_YOU_SLIP_OFF_M_GREASED_O, mdef, obj);
            else
                message_monster_object(MSG_YOU_SLIP_OFF_M_SLIPPERY_O, mdef, obj);
        } else {
            if (obj->greased)
                message_monster_object(MSG_YOU_GRAB_BUT_CANNOT_HOLD_ONTO_M_GREASED_O, mdef, obj);
            else
                message_monster_object(MSG_YOU_GRAB_BUT_CANNOT_HOLD_ONTO_M_SLIPPERY_O, mdef, obj);
        }
        if (obj->greased && !rn2(2)) {
            message_const(MSG_THE_GREASE_WEARS_OFF);
            obj->greased = 0;
        }
        return true;
    }
    return false;
}

/*
 * Send in a demon pet for the hero.  Exercise wisdom.
 *
 * This function used to be inline to damageum(), but the Metrowerks compiler
 * (DR4 and DR4.5) screws up with an internal error 5 "Expression Too Complex."
 * Pulling it out makes it work.
 */
static void demonpet(void) {
    int i;
    struct permonst *pm;
    struct monst *dtmp;

    pline("Some hell-p has arrived!");
    i = !rn2(6) ? ndemon(u.ualign.type) : NON_PM;
    pm = i != NON_PM ? &mons[i] : youmonst.data;
    if ((dtmp = makemon(pm, u.ux, u.uy, NO_MM_FLAGS)) != 0)
        (void)tamedog(dtmp, (struct obj *)0);
    exercise(A_WIS, true);
}

/*
 * Player uses theft attack against monster.
 *
 * If the target is wearing body armor, take all of its possesions;
 * otherwise, take one object.  [Is this really the behavior we want?]
 *
 * This routine implicitly assumes that there is no way to be able to
 * resist petfication (ie, be polymorphed into a xorn or golem) at the
 * same time as being able to steal (poly'd into nymph or succubus).
 * If that ever changes, the check for touching a cockatrice corpse
 * will need to be smarter about whether to break out of the theft loop.
 */
static void steal_it(struct monst *mdef, struct attack *mattk) {
    struct obj *otmp, *stealoid, **minvent_ptr;
    long unwornmask;

    if (!mdef->minvent)
        return; /* nothing to take */

    /* look for worn body armor */
    stealoid = (struct obj *)0;
    if (could_seduce(&youmonst, mdef, mattk)) {
        /* find armor, and move it to end of inventory in the process */
        minvent_ptr = &mdef->minvent;
        while ((otmp = *minvent_ptr) != 0)
            if (otmp->owornmask & W_ARM) {
                if (stealoid)
                    panic("steal_it: multiple worn suits");
                *minvent_ptr = otmp->nobj; /* take armor out of minvent */
                stealoid = otmp;
                stealoid->nobj = (struct obj *)0;
            } else {
                minvent_ptr = &otmp->nobj;
            }
        *minvent_ptr = stealoid; /* put armor back into minvent */
    }

    if (stealoid) { /* we will be taking everything */
        if (gender(mdef) == (int)u.mfemale && youmonst.data->mlet == S_NYMPH)
            message_monster(MSG_YOU_CHARM_HER_AND_STEAL_EVERYTHING, mdef);
        else
            message_monster_string(MSG_YOU_SEDUCE_M_AND_HE_GETS_NAKED, mdef, mhe(mdef));
    }

    while ((otmp = mdef->minvent) != 0) {
        if (!Upolyd)
            break; /* no longer have ability to steal */
        /* take the object away from the monster */
        obj_extract_self(otmp);
        if ((unwornmask = otmp->owornmask) != 0L) {
            mdef->misc_worn_check &= ~unwornmask;
            if (otmp->owornmask & W_WEP) {
                setmnotwielded(mdef, otmp);
                MON_NOWEP(mdef);
            }
            otmp->owornmask = 0L;
            update_mon_intrinsics(mdef, otmp, false, false);

            if (otmp == stealoid) /* special message for final item */
                message_monster_string(MSG_HE_FINISHES_TAKING_OFF_HIS_SUIT, mdef, mhis(mdef));
        }
        /* give the object to the character */
        otmp = hold_another_object(otmp, "You snatched but dropped %s.", doname(otmp), "You steal: ");
        if (otmp->where != OBJ_INVENT)
            continue;
        if (otmp->otyp == CORPSE && touch_petrifies(&mons[otmp->corpsenm]) && !uarmg) {
            char kbuf[BUFSZ];

            sprintf(kbuf, "stolen %s corpse", mons[otmp->corpsenm].mname);
            instapetrify(kbuf);
            break; /* stop the theft even if hero survives */
        }
        /* more take-away handling, after theft message */
        if (unwornmask & W_WEP) { /* stole wielded weapon */
            possibly_unwield(mdef, false);
        } else if (unwornmask & W_ARMG) { /* stole worn gloves */
            mselftouch(mdef, (const char *)0, true);
            if (mdef->mhp <= 0) /* it's now a statue */
                return; /* can't continue stealing */
        }

        if (!stealoid)
            break; /* only taking one item */
    }
}

int damageum(struct monst *mdef, struct attack *mattk) {
    struct permonst *pd = mdef->data;
    int tmp = d((int)mattk->damn, (int)mattk->damd);
    int armpro;
    bool negated;

    armpro = magic_negation(mdef);
    /* since hero can't be cancelled, only defender's armor applies */
    negated = !((rn2(3) >= armpro) || !rn2(50));

    if (is_demon(youmonst.data) && !rn2(13) && !uwep && u.umonnum != PM_SUCCUBUS && u.umonnum != PM_INCUBUS && u.umonnum != PM_BALROG) {
        demonpet();
        return (0);
    }
    switch (mattk->adtyp) {
        case AD_STUN:
            if (!Blind)
                message_monster(MSG_M_STAGGERS_FOR_A_MOMENT, mdef);
            mdef->mstun = 1;
            goto physical;
        case AD_LEGS:
            /* if (u.ucancelled) { */
            /*    tmp = 0;         */
            /*    break;           */
            /* }                   */
            goto physical;
        case AD_WERE: /* no special effect on monsters */
        case AD_HEAL: /* likewise */
        case AD_PHYS:
            physical: if (mattk->aatyp == AT_WEAP) {
                if (uwep)
                    tmp = 0;
            } else if (mattk->aatyp == AT_KICK) {
                if (thick_skinned(mdef->data))
                    tmp = 0;
                if (mdef->data == &mons[PM_SHADE]) {
                    if (!(uarmf && uarmf->blessed)) {
                        impossible("bad shade attack function flow?");
                        tmp = 0;
                    } else
                        tmp = rnd(4); /* bless damage */
                }
            }
        break;
        case AD_FIRE:
            if (negated) {
                tmp = 0;
                break;
            }
            if (!Blind)
                message_monster_string(MSG_M_IS_ON_FIRE, mdef, on_fire(mdef->data, mattk));
            if (pd == &mons[PM_STRAW_GOLEM] || pd == &mons[PM_PAPER_GOLEM]) {
                if (!Blind)
                    message_monster(MSG_M_BURNS_COMPLETELY, mdef);
                xkilled(mdef, 2);
                tmp = 0;
                break;
                /* Don't return yet; keep hp<1 and tmp=0 for pet msg */
            }
            tmp += destroy_mitem(mdef, SCROLL_CLASS, AD_FIRE);
            tmp += destroy_mitem(mdef, SPBOOK_CLASS, AD_FIRE);
            if (resists_fire(mdef)) {
                if (!Blind)
                    message_monster(MSG_THE_FIRE_DOESNT_HEAT_M, mdef);
                golemeffects(mdef, AD_FIRE, tmp);
                shieldeff(mdef->mx, mdef->my);
                tmp = 0;
            }
            /* only potions damage resistant players in destroy_item */
            tmp += destroy_mitem(mdef, POTION_CLASS, AD_FIRE);
            break;
        case AD_COLD:
            if (negated) {
                tmp = 0;
                break;
            }
            if (!Blind)
                message_monster(MSG_M_IS_COVERED_IN_FROST, mdef);
            if (resists_cold(mdef)) {
                shieldeff(mdef->mx, mdef->my);
                if (!Blind)
                    message_monster(MSG_THE_FROST_DOESNT_CHILL_M, mdef);
                golemeffects(mdef, AD_COLD, tmp);
                tmp = 0;
            }
            tmp += destroy_mitem(mdef, POTION_CLASS, AD_COLD);
            break;
        case AD_ELEC:
            if (negated) {
                tmp = 0;
                break;
            }
            if (!Blind)
                message_monster(MSG_M_IS_ZAPPED, mdef);
            tmp += destroy_mitem(mdef, WAND_CLASS, AD_ELEC);
            if (resists_elec(mdef)) {
                if (!Blind)
                    message_monster(MSG_THE_ZAP_DOESNT_SHOCK_M, mdef);
                golemeffects(mdef, AD_ELEC, tmp);
                shieldeff(mdef->mx, mdef->my);
                tmp = 0;
            }
            /* only rings damage resistant players in destroy_item */
            tmp += destroy_mitem(mdef, RING_CLASS, AD_ELEC);
            break;
        case AD_ACID:
            if (resists_acid(mdef))
                tmp = 0;
            break;
        case AD_STON:
            if (!munstone(mdef, true))
                minstapetrify(mdef, true);
            tmp = 0;
            break;
        case AD_SSEX:
        case AD_SEDU:
        case AD_SITM:
            steal_it(mdef, mattk);
            tmp = 0;
            break;
        case AD_SGLD:
            if (mdef->mgold) {
                u.ugold += mdef->mgold;
                mdef->mgold = 0;
                Your("purse feels heavier.");
            }
            exercise(A_DEX, true);
            tmp = 0;
            break;
        case AD_TLPT:
            if (tmp <= 0)
                tmp = 1;
            if (!negated && tmp < mdef->mhp) {
                char nambuf[BUFSZ];
                bool u_saw_mon = canseemon(mdef) || (u.uswallow && u.ustuck == mdef);
                /* record the name before losing sight of monster */
                strcpy(nambuf, "TODO: Monnam(mdef)");
                if (u_teleport_mon(mdef, false) && u_saw_mon && !canseemon(mdef))
                    pline("%s suddenly disappears!", nambuf);
            }
            break;
        case AD_BLND:
            if (can_blnd(&youmonst, mdef, mattk->aatyp, (struct obj*)0)) {
                if (!Blind && mdef->mcansee)
                    message_monster(MSG_M_IS_BLINDED, mdef);
                mdef->mcansee = 0;
                tmp += mdef->mblinded;
                if (tmp > 127)
                    tmp = 127;
                mdef->mblinded = tmp;
            }
            tmp = 0;
            break;
        case AD_CURS:
            if (night() && !rn2(10) && !mdef->mcan) {
                if (mdef->data == &mons[PM_CLAY_GOLEM]) {
                    if (!Blind)
                        message_monster(MSG_WRITING_VANISHES_FROM_M_HEAD, mdef);
                    xkilled(mdef, 0);
                    /* Don't return yet; keep hp<1 and tmp=0 for pet msg */
                } else {
                    mdef->mcan = 1;
                    You("chuckle.");
                }
            }
            tmp = 0;
            break;
        case AD_DRLI:
            if (!negated && !rn2(3) && !resists_drli(mdef)) {
                int xtmp = d(2, 6);
                message_monster(MSG_M_SUDDENLY_SEEMS_WEAKER, mdef);
                mdef->mhpmax -= xtmp;
                if ((mdef->mhp -= xtmp) <= 0 || !mdef->m_lev) {
                    message_monster(MSG_M_DIES, mdef);
                    xkilled(mdef, 0);
                } else
                    mdef->m_lev--;
                tmp = 0;
            }
            break;
        case AD_RUST:
            if (pd == &mons[PM_IRON_GOLEM]) {
                message_monster(MSG_M_FALLS_TO_PIECES, mdef);
                xkilled(mdef, 0);
            }
            hurtmarmor(mdef, AD_RUST);
            tmp = 0;
            break;
        case AD_CORR:
            hurtmarmor(mdef, AD_CORR);
            tmp = 0;
            break;
        case AD_DCAY:
            if (pd == &mons[PM_WOOD_GOLEM] || pd == &mons[PM_LEATHER_GOLEM]) {
                message_monster(MSG_M_FALLS_TO_PIECES, mdef);
                xkilled(mdef, 0);
            }
            hurtmarmor(mdef, AD_DCAY);
            tmp = 0;
            break;
        case AD_DRST:
        case AD_DRDX:
        case AD_DRCO:
            if (!negated && !rn2(8)) {
                Your("%s was poisoned!", mpoisons_subj(&youmonst, mattk));
                if (resists_poison(mdef)) {
                    message_monster(MSG_THE_POISON_DOESNT_SEEM_TO_AFFECT_M, mdef);
                } else {
                    if (!rn2(10)) {
                        Your("poison was deadly...");
                        tmp = mdef->mhp;
                    } else {
                        tmp += rn1(10, 6);
                    }
                }
            }
            break;
        case AD_DRIN:
            if (notonhead || !has_head(mdef->data)) {
                message_monster(MSG_M_DOESNT_SEEM_HARMED, mdef);
                tmp = 0;
                if (!Unchanging && mdef->data == &mons[PM_GREEN_SLIME]) {
                    if (!Slimed) {
                        You("suck in some slime and don't feel very well.");
                        Slimed = 10L;
                    }
                }
                break;
            }
            if (m_slips_free(mdef, mattk))
                break;

            if ((mdef->misc_worn_check & W_ARMH) && rn2(8)) {
                message_monster_string(MSG_M_HELMET_BLOCKS_YOUR_ATTACK_TO_HIS_HEAD, mdef, mhis(mdef));
                break;
            }

            message_monster(MSG_YOU_EAT_M_BRAIN, mdef);
            u.uconduct.food++;
            if (touch_petrifies(mdef->data) && !Stone_resistance && !Stoned) {
                Stoned = 5;
                fprintf(stderr, "TODO: delayed_killer = %s\n", mdef->data->mname);
            }
            if (!vegan(mdef->data))
                u.uconduct.unvegan++;
            if (!vegetarian(mdef->data))
                violated_vegetarian();
            if (mindless(mdef->data)) {
                message_monster(MSG_M_DOESN_NOTICE, mdef);
                break;
            }
            tmp += rnd(10);
            morehungry(-rnd(30)); /* cannot choke */
            if (ABASE(A_INT) < AMAX(A_INT)) {
                ABASE(A_INT) += rnd(4);
                if (ABASE(A_INT) > AMAX(A_INT))
                    ABASE(A_INT) = AMAX(A_INT);
                flags.botl = 1;
            }
            exercise(A_WIS, true);
            break;
        case AD_STCK:
            if (!negated && !sticks(mdef->data))
                u.ustuck = mdef; /* it's now stuck to you */
            break;
        case AD_WRAP:
            if (!sticks(mdef->data)) {
                if (!u.ustuck && !rn2(10)) {
                    if (m_slips_free(mdef, mattk)) {
                        tmp = 0;
                    } else {
                        message_monster(MSG_YOU_SWING_YOURSELF_AROUND_M, mdef);
                        u.ustuck = mdef;
                    }
                } else if(u.ustuck == mdef) {
                    /* Monsters don't wear amulets of magical breathing */
                    if (is_pool(u.ux, u.uy) && !is_swimmer(mdef->data) && !amphibious(mdef->data)) {
                        message_monster(MSG_YOU_DROWN_M, mdef);
                        tmp = mdef->mhp;
                    } else if(mattk->aatyp == AT_HUGS) {
                        message_monster(MSG_M_IS_BEING_CRUSHED, mdef);
                    }
                } else {
                    tmp = 0;
                    if (flags.verbose) {
                        message_monster(MSG_YOU_BRUSH_AGAINST_M_LEG, mdef);
                    }
                }
            } else tmp = 0;
            break;
        case AD_PLYS:
            if (!negated && mdef->mcanmove && !rn2(3) && tmp < mdef->mhp) {
                if (!Blind) {
                    message_monster(MSG_M_IS_FROZEN_BY_YOU, mdef);
                }
                mdef->mcanmove = 0;
                mdef->mfrozen = rnd(10);
            }
            break;
        case AD_SLEE:
            if (!negated && !mdef->msleeping &&
                    sleep_monst(mdef, rnd(10), -1)) {
                if (!Blind) {
                    message_monster(MSG_M_IS_PUT_TO_SLEEP_BY_YOU, mdef);
                }
                slept_monst(mdef);
            }
            break;
        case AD_SLIM:
            if (negated) break; /* physical damage only */
            if (!rn2(4) && !flaming(mdef->data) &&
                    mdef->data != &mons[PM_GREEN_SLIME]) {
                message_monster(MSG_YOU_TURN_M_INTO_SLIME, mdef);
                (void) newcham(mdef, &mons[PM_GREEN_SLIME], false, false);
                tmp = 0;
            }
            break;
        case AD_ENCH: /* KMH -- remove enchantment (disenchanter) */
            /* there's no msomearmor() function, so just do damage */
            /* if (negated) break; */
            break;
        case AD_SLOW:
            if (!negated && mdef->mspeed != MSLOW) {
                unsigned int oldspeed = mdef->mspeed;

                mon_adjust_speed(mdef, -1, (struct obj *)0);
                if (mdef->mspeed != oldspeed && canseemon(mdef)) {
                    message_monster(MSG_M_SLOWS_DOWN, mdef);
                }
            }
            break;
        case AD_CONF:
            if (!mdef->mconf) {
                if (canseemon(mdef)) {
                    message_monster(MSG_M_LOOKS_CONFUSED, mdef);
                }
                mdef->mconf = 1;
            }
            break;
        default: tmp = 0;
        break;
    }

    mdef->mstrategy &= ~STRAT_WAITFORU; /* in case player is very fast */
    if ((mdef->mhp -= tmp) < 1) {
        if (mdef->mtame && !cansee(mdef->mx, mdef->my)) {
            You_feel("embarrassed for a moment.");
            if (tmp)
                xkilled(mdef, 0); /* !tmp but hp<1: already killed */
        } else if (!flags.verbose) {
            You("destroy it!");
            if (tmp)
                xkilled(mdef, 0);
        } else if (tmp)
            killed(mdef);
        return (2);
    }
    return (1);
}

void missum(struct monst *mdef, struct attack *mattk) {
    if (could_seduce(&youmonst, mdef, mattk)) {
        message_monster(MSG_YOU_PRETEND_TO_BE_FRIENDLY_TO_M, mdef);
    } else if (canspotmon(mdef) && flags.verbose) {
        message_monster(MSG_YOU_MISS_M, mdef);
    } else {
        You("miss it.");
    }
    if (!mdef->msleeping && mdef->mcanmove)
        wakeup(mdef);
}

/*      Special (passive) attacks on you by monsters done here.         */

int passive(struct monst *mon, bool mhit, int malive, unsigned char aatyp) {
    struct permonst *ptr = mon->data;
    int i, tmp;

    for (i = 0;; i++) {
        if (i >= NATTK)
            return (malive | mhit); /* no passive attacks */
        if (ptr->mattk[i].aatyp == AT_NONE)
            break; /* try this one */
    }
    /* Note: tmp not always used */
    if (ptr->mattk[i].damn)
        tmp = d((int)ptr->mattk[i].damn, (int)ptr->mattk[i].damd);
    else if (ptr->mattk[i].damd)
        tmp = d((int)mon->m_lev + 1, (int)ptr->mattk[i].damd);
    else
        tmp = 0;

    /*      These affect you even if they just died */

    switch (ptr->mattk[i].adtyp) {

        case AD_ACID:
            if (mhit && rn2(2)) {
                if (Blind || !flags.verbose) {
                    You("are splashed!");
                } else {
                    message_monster(MSG_YOU_ARE_SPLASHED_BY_M_ACID, mon);
                }

                if (!Acid_resistance)
                    mdamageu(mon, tmp);
                if (!rn2(30))
                    erode_armor(&youmonst, true);
            }
            if (mhit) {
                if (aatyp == AT_KICK) {
                    if (uarmf && !rn2(6))
                        (void)rust_dmg(uarmf, xname(uarmf), 3, true, &youmonst);
                } else if (aatyp == AT_WEAP || aatyp == AT_CLAW || aatyp == AT_MAGC || aatyp == AT_TUCH)
                    passive_obj(mon, (struct obj*)0, &(ptr->mattk[i]));
            }
            exercise(A_STR, false);
            break;
        case AD_STON:
            if (mhit) { /* successful attack */
                long protector = attk_protection((int)aatyp);

                /* hero using monsters' AT_MAGC attack is hitting hand to
                 hand rather than casting a spell */
                if (aatyp == AT_MAGC)
                    protector = W_ARMG;

                if (protector == 0L || /* no protection */
                        (protector == W_ARMG && !uarmg && !uwep) || (protector == W_ARMF && !uarmf) || (protector == W_ARMH && !uarmh) || (protector == (W_ARMC | W_ARMG) && (!uarmc || !uarmg))) {
                    if (!Stone_resistance && !(poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM))) {
                        You("turn to stone...");
                        done_in_by(mon);
                        return 2;
                    }
                }
            }
            break;
        case AD_RUST:
            if (mhit && !mon->mcan) {
                if (aatyp == AT_KICK) {
                    if (uarmf)
                        (void)rust_dmg(uarmf, xname(uarmf), 1, true, &youmonst);
                } else if (aatyp == AT_WEAP || aatyp == AT_CLAW || aatyp == AT_MAGC || aatyp == AT_TUCH)
                    passive_obj(mon, (struct obj*)0, &(ptr->mattk[i]));
            }
            break;
        case AD_CORR:
            if (mhit && !mon->mcan) {
                if (aatyp == AT_KICK) {
                    if (uarmf)
                        (void)rust_dmg(uarmf, xname(uarmf), 3, true, &youmonst);
                } else if (aatyp == AT_WEAP || aatyp == AT_CLAW || aatyp == AT_MAGC || aatyp == AT_TUCH)
                    passive_obj(mon, (struct obj*)0, &(ptr->mattk[i]));
            }
            break;
        case AD_MAGM:
            /* wrath of gods for attacking Oracle */
            if (Antimagic) {
                shieldeff(u.ux, u.uy);
                pline("A hail of magic missiles narrowly misses you!");
            } else {
                You("are hit by magic missiles appearing from thin air!");
                mdamageu(mon, tmp);
            }
            break;
        case AD_ENCH: /* KMH -- remove enchantment (disenchanter) */
            if (mhit) {
                struct obj *obj = (struct obj *)0;

                if (aatyp == AT_KICK) {
                    obj = uarmf;
                    if (!obj)
                        break;
                } else if (aatyp == AT_BITE || aatyp == AT_BUTT || (aatyp >= AT_STNG && aatyp < AT_WEAP)) {
                    break; /* no object involved */
                }
                passive_obj(mon, obj, &(ptr->mattk[i]));
            }
            break;
        default:
            break;
    }

    /*      These only affect you if they still live */

    if (malive && !mon->mcan && rn2(3)) {

        switch (ptr->mattk[i].adtyp) {

            case AD_PLYS:
                if (ptr == &mons[PM_FLOATING_EYE]) {
                    if (!canseemon(mon)) {
                        break;
                    }
                    if (mon->mcansee) {
                        char pname[BUFSZ];
                        monster_possessive_cap(pname, BUFSZ, mon);
                        if (ureflects("%s gaze is reflected by your %s.", pname)) {
                            // nothing
                        } else if (Free_action) {
                            message_monster(MSG_YOU_MOMENTARILY_STIFFEN_UNDER_M_GAZE, mon);
                        } else {
                            message_monster(MSG_YOU_ARE_FROZEN_BY_M_GAZE, mon);
                            nomul((ACURR(A_WIS) > 12 || rn2(4)) ? -tmp : -127);
                        }
                    } else {
                        message_monster(MSG_A_BLIND_M_CANNOT_DEFEND_ITSELF, mon);
                        if(!rn2(500)) change_luck(-1);
                    }
                } else if (Free_action) {
                    You("momentarily stiffen.");
                } else { /* gelatinous cube */
                    message_monster(MSG_YOU_ARE_FROZEN_BY_M, mon);
                    nomovemsg = 0; /* default: "you can move again" */
                    nomul(-tmp);
                    exercise(A_DEX, false);
                }
                break;
            case AD_COLD: /* brown mold or blue jelly */
                if(monnear(mon, u.ux, u.uy)) {
                    if(Cold_resistance) {
                        shieldeff(u.ux, u.uy);
                        You_feel("a mild chill.");
                        ugolemeffects(AD_COLD, tmp);
                        break;
                    }
                    You("are suddenly very cold!");
                    mdamageu(mon, tmp);
                    /* monster gets stronger with your heat! */
                    mon->mhp += tmp / 2;
                    if (mon->mhpmax < mon->mhp) mon->mhpmax = mon->mhp;
                    /* at a certain point, the monster will reproduce! */
                    if(mon->mhpmax > ((int) (mon->m_lev+1) * 8))
                        (void)split_mon(mon, &youmonst);
                }
                break;
            case AD_STUN: /* specifically yellow mold */
                if(!Stunned)
                    make_stunned((long)tmp, true);
                break;
            case AD_FIRE:
                if(monnear(mon, u.ux, u.uy)) {
                    if(Fire_resistance) {
                        shieldeff(u.ux, u.uy);
                        You_feel("mildly warm.");
                        ugolemeffects(AD_FIRE, tmp);
                        break;
                    }
                    You("are suddenly very hot!");
                    mdamageu(mon, tmp);
                }
                break;
            case AD_ELEC:
                if(Shock_resistance) {
                    shieldeff(u.ux, u.uy);
                    You_feel("a mild tingle.");
                    ugolemeffects(AD_ELEC, tmp);
                    break;
                }
                You("are jolted with electricity!");
                mdamageu(mon, tmp);
                break;
            default:
                break;
        }
    }
    return (malive | mhit);
}

/*
 * Special (passive) attacks on an attacking object by monsters done here.
 * Assumes the attack was successful.
 */
/* obj: null means pick uwep, uswapwep or uarmg */
/* matt: null means we find one internally */
void passive_obj(struct monst *mon, struct obj *obj, struct attack *mattk) {
    struct permonst *ptr = mon->data;
    int i;

    /* if caller hasn't specified an object, use uwep, uswapwep or uarmg */
    if (!obj) {
        obj = (u.twoweap && uswapwep && !rn2(2)) ? uswapwep : uwep;
        if (!obj && mattk->adtyp == AD_ENCH)
            obj = uarmg; /* no weapon? then must be gloves */
        if (!obj)
            return; /* no object to affect */
    }

    /* if caller hasn't specified an attack, find one */
    if (!mattk) {
        for (i = 0;; i++) {
            if (i >= NATTK)
                return; /* no passive attacks */
            if (ptr->mattk[i].aatyp == AT_NONE)
                break; /* try this one */
        }
        mattk = &(ptr->mattk[i]);
    }

    switch (mattk->adtyp) {

        case AD_ACID:
            if (!rn2(6)) {
                erode_obj(obj, true, false);
            }
            break;
        case AD_RUST:
            if (!mon->mcan) {
                erode_obj(obj, false, false);
            }
            break;
        case AD_CORR:
            if (!mon->mcan) {
                erode_obj(obj, true, false);
            }
            break;
        case AD_ENCH:
            if (!mon->mcan) {
                if (drain_item(obj) && carried(obj) && (obj->known || obj->oclass == ARMOR_CLASS)) {
                    Your("%s less effective.", aobjnam(obj, "seem"));
                }
                break;
            }
        default:
            break;
    }

}

/* Note: caller must ascertain mtmp is mimicking... */
void stumble_onto_mimic(struct monst *mtmp) {
    const char *fmt = "Wait!  That's %s!", *generic = "a monster", *what = 0;

    if (!u.ustuck && !mtmp->mflee && dmgtype(mtmp->data, AD_STCK))
        u.ustuck = mtmp;

    if (Blind) {
        if (!Blind_telepat)
            what = generic; /* with default fmt */
        else if (mtmp->m_ap_type == M_AP_MONSTER)
            what = "TODO:a_monnam(mtmp)"; /* differs from what was sensed */
    } else {
        int glyph = levl[u.ux + u.dx][u.uy + u.dy].glyph;

        if (glyph_is_cmap(glyph) && (glyph_to_cmap(glyph) == S_hcdoor ||
                glyph_to_cmap(glyph) == S_vcdoor))
            fmt = "The door actually was %s!";
        else if (glyph_is_object(glyph) &&
                glyph_to_obj(glyph) == GOLD_PIECE)
            fmt = "That gold was %s!";

        /* cloned Wiz starts out mimicking some other monster and
         might make himself invisible before being revealed */
        if (mtmp->minvis && !See_invisible)
            what = generic;
        else
            what = "TODO:a_monnam(mtmp)";
    }
    if (what)
        pline(fmt, what);

    wakeup(mtmp); /* clears mimicking */
}

/* source of flash */
int flash_hits_mon(struct monst *mtmp, struct obj *otmp) {
    int tmp, amt, res = 0, useeit = canseemon(mtmp);

    if (mtmp->msleeping) {
        mtmp->msleeping = 0;
        if (useeit) {
            message_monster(MSG_THE_FLAG_AWAKENS_M, mtmp);
            res = 1;
        }
    } else if (mtmp->data->mlet != S_LIGHT) {
        if (!resists_blnd(mtmp)) {
            tmp = dist2(otmp->ox, otmp->oy, mtmp->mx, mtmp->my);
            if (useeit) {
                message_monster(MSG_M_IS_BLINDED_BY_THE_FLASH, mtmp);
                res = 1;
            }
            if (mtmp->data == &mons[PM_GREMLIN]) {
                /* Rule #1: Keep them out of the light. */
                amt = otmp->otyp == WAN_LIGHT ? d(1 + otmp->spe, 4) : rn2(min(mtmp->mhp, 4));
                pline("%s %s!", "TODO:Monnam(mtmp)",
                        amt > mtmp->mhp / 2 ? "wails in agony" : "cries out in pain");
                if ((mtmp->mhp -= amt) <= 0) {
                    if (flags.mon_moving)
                        monkilled(mtmp, (char *)0, AD_BLND);
                    else
                        killed(mtmp);
                } else if (cansee(mtmp->mx,mtmp->my) && !canspotmon(mtmp)) {
                    map_invisible(mtmp->mx, mtmp->my);
                }
            }
            if (mtmp->mhp > 0) {
                if (!flags.mon_moving)
                    setmangry(mtmp);
                if (tmp < 9 && !mtmp->isshk && rn2(4)) {
                    if (rn2(4))
                        monflee(mtmp, rnd(100), false, true);
                    else
                        monflee(mtmp, 0, false, true);
                }
                mtmp->mcansee = 0;
                mtmp->mblinded = (tmp < 3) ? 0 : rnd(1 + 50 / tmp);
            }
        }
    }
    return res;
}
