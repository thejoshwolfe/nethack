/* See LICENSE in the root of this project for change info */

#include "steal.h"

#include <string.h>

#include "display_util.h"
#include "apply.h"
#include "decl.h"
#include "display.h"
#include "do.h"
#include "do_wear.h"
#include "eat.h"
#include "flag.h"
#include "global.h"
#include "hack.h"
#include "invent.h"
#include "light.h"
#include "mkobj.h"
#include "mon.h"
#include "monattk.h"
#include "mondata.h"
#include "monmove.h"
#include "monst.h"
#include "obj.h"
#include "objclass.h"
#include "objnam.h"
#include "onames.h"
#include "permonst.h"
#include "pline.h"
#include "polyself.h"
#include "prop.h"
#include "questpgr.h"
#include "read.h"
#include "rm.h"
#include "rnd.h"
#include "shk.h"
#include "teleport.h"
#include "timeout.h"
#include "trap.h"
#include "util.h"
#include "vision.h"
#include "weapon.h"
#include "wield.h"
#include "worn.h"
#include "you.h"
#include "youprop.h"
#include "zap.h"

/* steal armor after you finish taking it off */
unsigned int stealoid;          /* object to be stolen */
unsigned int stealmid;          /* monster doing the stealing */

static const char * equipname (struct obj *otmp) {
    return (
            (otmp == uarmu) ? "shirt" :
            (otmp == uarmf) ? "boots" :
            (otmp == uarms) ? "shield" :
            (otmp == uarmg) ? "gloves" :
            (otmp == uarmc) ? cloak_simple_name(otmp) :
            (otmp == uarmh) ? "helmet" : "armor");
}

long somegold (void) {
    return (long)( (u.ugold < 100) ? u.ugold :
            (u.ugold > 10000) ? rnd(10000) : rnd((int) u.ugold) );
}

void stealgold (struct monst *mtmp) {
    struct obj *gold = g_at(u.ux, u.uy);
    long tmp;

    if (gold && ( !u.ugold || gold->quan > u.ugold || !rn2(5))) {
        mtmp->mgold += gold->quan;
        delobj(gold);
        newsym(u.ux, u.uy);
        message_monster(MSG_M_QUICKLY_SNATCHES_GOLD_FROM_BEETWEEN_YOUR_LEGS, mtmp);
        if(!u.ugold || !rn2(5)) {
            if (!tele_restrict(mtmp)) (void) rloc(mtmp, false);
            /* do not set mtmp->mavenge here; gold on the floor is fair game */
            monflee(mtmp, 0, false, false);
        }
    } else if(u.ugold) {
        u.ugold -= (tmp = somegold());
        Your("purse feels lighter.");
        mtmp->mgold += tmp;
        if (!tele_restrict(mtmp)) (void) rloc(mtmp, false);
        mtmp->mavenge = 1;
        monflee(mtmp, 0, false, false);
    }
}

static int stealarm (void) {
    struct monst *mtmp;
    struct obj *otmp;

    for(otmp = invent; otmp; otmp = otmp->nobj) {
        if(otmp->o_id == stealoid) {
            for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
                if(mtmp->m_id == stealmid) {
                    if(DEADMONSTER(mtmp)) impossible("stealarm(): dead monster stealing");
                    if(!dmgtype(mtmp->data, AD_SITM)) /* polymorphed */
                        goto botm;
                    if(otmp->unpaid)
                        subfrombill(otmp, shop_keeper(*u.ushops));
                    freeinv(otmp);
                    message_monster_object(MSG_M_STEALS_O, mtmp, otmp);
                    mpickobj(mtmp,otmp);     /* may free otmp */
                    /* Implies seduction, "you gladly hand over ..."
                       so we don't set mavenge bit here. */
                    monflee(mtmp, 0, false, false);
                    if (!tele_restrict(mtmp)) (void) rloc(mtmp, false);
                    break;
                }
            }
            break;
        }
    }
botm:   stealoid = 0;
        return 0;
}

/* An object you're wearing has been taken off by a monster (theft or
   seduction).  Also used if a worn item gets transformed (stone to flesh). */
// bool unchain_ball   /* whether to unpunish or just unwield */
void remove_worn_item ( struct obj *obj, bool unchain_ball) {
    if (donning(obj))
        cancel_don();
    if (!obj->owornmask)
        return;

    if (obj->owornmask & W_ARMOR) {
        if (obj == uskin) {
            impossible("Removing embedded scales?");
            skinback(true);         /* uarm = uskin; uskin = 0; */
        }
        if (obj == uarm) (void) Armor_off();
        else if (obj == uarmc) (void) Cloak_off();
        else if (obj == uarmf) (void) Boots_off();
        else if (obj == uarmg) (void) Gloves_off();
        else if (obj == uarmh) (void) Helmet_off();
        else if (obj == uarms) (void) Shield_off();
        else if (obj == uarmu) (void) Shirt_off();
        /* catchall -- should never happen */
        else setworn((struct obj *)0, obj->owornmask & W_ARMOR);
    } else if (obj->owornmask & W_AMUL) {
        Amulet_off();
    } else if (obj->owornmask & W_RING) {
        Ring_gone(obj);
    } else if (obj->owornmask & W_TOOL) {
        Blindf_off(obj);
    } else if (obj->owornmask & (W_WEP|W_SWAPWEP|W_QUIVER)) {
        if (obj == uwep)
            uwepgone();
        if (obj == uswapwep)
            uswapwepgone();
        if (obj == uquiver)
            uqwepgone();
    }

    if (obj->owornmask & (W_BALL|W_CHAIN)) {
        if (unchain_ball) unpunish();
    } else if (obj->owornmask) {
        /* catchall */
        setnotworn(obj);
    }
}

/* Returns 1 when something was stolen (or at least, when N should flee now)
 * Returns -1 if the monster died in the attempt
 * Avoid stealing the object stealoid
 */
int steal (struct monst *mtmp, char *objnambuf) {
    struct obj *otmp;
    int tmp, could_petrify, named = 0, armordelay;
    bool monkey_business; /* true iff an animal is doing the thievery */

    if (objnambuf) *objnambuf = '\0';
    /* the following is true if successful on first of two attacks. */
    if(!monnear(mtmp, u.ux, u.uy)) return(0);

    /* food being eaten might already be used up but will not have
       been removed from inventory yet; we don't want to steal that,
       so this will cause it to be removed now */
    if (occupation) (void) maybe_finished_meal(false);

    if (!invent || (inv_cnt() == 1 && uskin)) {
nothing_to_steal:
        /* Not even a thousand men in armor can strip a naked man. */
        if (Blind) {
            pline("Somebody tries to rob you, but finds nothing to steal.");
        } else {
            message_monster(MSG_M_TRIES_TO_ROB_BUT_YOURE_NAKED, mtmp);
        }
        return 1;  /* let her flee */
    }

    monkey_business = is_animal(mtmp->data);
    if (monkey_business) {
        ;   /* skip ring special cases */
    } else if (Adornment & LEFT_RING) {
        otmp = uleft;
        goto gotobj;
    } else if (Adornment & RIGHT_RING) {
        otmp = uright;
        goto gotobj;
    }

    tmp = 0;
    for(otmp = invent; otmp; otmp = otmp->nobj)
        if ((!uarm || otmp != uarmc) && otmp != uskin
           )
            tmp += ((otmp->owornmask &
                        (W_ARMOR | W_RING | W_AMUL | W_TOOL)) ? 5 : 1);
    if (!tmp) goto nothing_to_steal;
    tmp = rn2(tmp);
    for(otmp = invent; otmp; otmp = otmp->nobj)
        if ((!uarm || otmp != uarmc) && otmp != uskin
           )
            if((tmp -= ((otmp->owornmask &
                                (W_ARMOR | W_RING | W_AMUL | W_TOOL)) ? 5 : 1)) < 0)
                break;
    if(!otmp) {
        impossible("Steal fails!");
        return(0);
    }
    /* can't steal gloves while wielding - so steal the wielded item. */
    if (otmp == uarmg && uwep)
        otmp = uwep;
    /* can't steal armor while wearing cloak - so steal the cloak. */
    else if(otmp == uarm && uarmc) otmp = uarmc;
    else if(otmp == uarmu && uarmc) otmp = uarmc;
    else if(otmp == uarmu && uarm) otmp = uarm;
gotobj:
    if(otmp->o_id == stealoid) return(0);

    /* animals can't overcome curse stickiness nor unlock chains */
    if (monkey_business) {
        bool ostuck;
        /* is the player prevented from voluntarily giving up this item?
           (ignores loadstones; the !can_carry() check will catch those) */
        if (otmp == uball)
            ostuck = true;  /* effectively worn; curse is implicit */
        else if (otmp == uquiver || (otmp == uswapwep && !u.twoweap))
            ostuck = false; /* not really worn; curse doesn't matter */
        else
            ostuck = (otmp->cursed && otmp->owornmask);

        if (ostuck || !can_carry(mtmp, otmp)) {
            static const char * const how[] = { "steal","snatch","grab","take" };
cant_take:
            message_monster_object_string(MSG_M_TRIES_TO_STEAL_YOUR_O_BUT_GIVES_UP, mtmp, otmp,
                    how[rn2(SIZE(how))]);
            /* the fewer items you have, the less likely the thief
               is going to stick around to try again (0) instead of
               running away (1) */
            return !rn2(inv_cnt() / 5 + 2);
        }
    }

    if (otmp->otyp == LEASH && otmp->leashmon) {
        if (monkey_business && otmp->cursed) goto cant_take;
        o_unleash(otmp);
    }

    /* you're going to notice the theft... */
    stop_occupation();

    if((otmp->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL))){
        switch(otmp->oclass) {
            case TOOL_CLASS:
            case AMULET_CLASS:
            case RING_CLASS:
            case FOOD_CLASS: /* meat ring */
                remove_worn_item(otmp, true);
                break;
            case ARMOR_CLASS:
                armordelay = objects[otmp->otyp].oc_delay;
                /* Stop putting on armor which has been stolen. */
                if (donning(otmp)) {
                    remove_worn_item(otmp, true);
                    break;
                } else if (monkey_business) {
                    /* animals usually don't have enough patience
                       to take off items which require extra time */
                    if (armordelay >= 1 && rn2(10)) goto cant_take;
                    remove_worn_item(otmp, true);
                    break;
                } else {
                    int curssv = otmp->cursed;
                    int slowly;
                    bool seen = canspotmon(mtmp);

                    otmp->cursed = 0;
                    /* can't charm you without first waking you */
                    if (multi < 0 && is_fainted()) unmul((char *)0);
                    slowly = (armordelay >= 1 || multi < 0);
                    if (flags.female) {
                        pline("%s charms you.  You gladly %s your %s.",
                                !seen ? "She" : "TODO: Monnam(mtmp)",
                                curssv ? "let her take" :
                                slowly ? "start removing" : "hand over",
                                equipname(otmp));
                    } else {
                        pline("%s seduces you and %s off your %s.",
                                !seen ? "She" : "TODO: Adjmonnam(mtmp, \"beautiful\")",
                                curssv ? "helps you to take" :
                                slowly ? "you start taking" : "you take",
                                equipname(otmp));
                    }
                    named++;
                    /* the following is to set multi for later on */
                    nomul(-armordelay);
                    remove_worn_item(otmp, true);
                    otmp->cursed = curssv;
                    if(multi < 0){
                        /*
                           multi = 0;
                           nomovemsg = 0;
                           afternmv = 0;
                           */
                        stealoid = otmp->o_id;
                        stealmid = mtmp->m_id;
                        afternmv = stealarm;
                        return(0);
                    }
                }
                break;
            default:
                impossible("Tried to steal a strange worn thing. [%d]",
                        otmp->oclass);
        }
    }
    else if (otmp->owornmask)
        remove_worn_item(otmp, true);

    /* do this before removing it from inventory */
    if (objnambuf) strcpy(objnambuf, yname(otmp));
    /* set mavenge bit so knights won't suffer an
     * alignment penalty during retaliation;
     */
    mtmp->mavenge = 1;

    freeinv(otmp);
    message_monster_object(MSG_M_STOLE_O, mtmp, otmp);
    could_petrify = (otmp->otyp == CORPSE && touch_petrifies(&mons[otmp->corpsenm]));
    mpickobj(mtmp,otmp);     /* may free otmp */
    if (could_petrify && !(mtmp->misc_worn_check & W_ARMG)) {
        minstapetrify(mtmp, true);
        return -1;
    }
    return((multi < 0) ? 0 : 1);
}

/* Returns 1 if otmp is free'd, 0 otherwise. */
int mpickobj (struct monst *mtmp, struct obj *otmp) {
    int freed_otmp;

    if (otmp->oclass == COIN_CLASS) {
        mtmp->mgold += otmp->quan;
        obfree(otmp, (struct obj *)0);
        freed_otmp = 1;
    } else {
        bool snuff_otmp = false;
        /* don't want hidden light source inside the monster; assumes that
           engulfers won't have external inventories; whirly monsters cause
           the light to be extinguished rather than letting it shine thru */
        if (otmp->lamplit &&  /* hack to avoid function calls for most objs */
                obj_sheds_light(otmp) &&
                attacktype(mtmp->data, AT_ENGL)) {
            /* this is probably a burning object that you dropped or threw */
            if (u.uswallow && mtmp == u.ustuck && !Blind) {
                message_object(MSG_O_GO_OUT, otmp);
            }
            snuff_otmp = true;
        }
        /* Must do carrying effects on object prior to add_to_minv() */
        carry_obj_effects(otmp);
        /* add_to_minv() might free otmp [if merged with something else],
           so we have to call it after doing the object checks */
        freed_otmp = add_to_minv(mtmp, otmp);
        /* and we had to defer this until object is in mtmp's inventory */
        if (snuff_otmp) snuff_light_source(mtmp->mx, mtmp->my);
    }
    return freed_otmp;
}

void stealamulet (struct monst *mtmp) {
    struct obj *otmp = (struct obj *)0;
    int real=0, fake=0;

    /* select the artifact to steal */
    if(u.uhave.amulet) {
        real = AMULET_OF_YENDOR;
        fake = FAKE_AMULET_OF_YENDOR;
    } else if(u.uhave.questart) {
        for(otmp = invent; otmp; otmp = otmp->nobj)
            if(is_quest_artifact(otmp)) break;
        if (!otmp) return;      /* should we panic instead? */
    } else if(u.uhave.bell) {
        real = BELL_OF_OPENING;
        fake = BELL;
    } else if(u.uhave.book) {
        real = SPE_BOOK_OF_THE_DEAD;
    } else if(u.uhave.menorah) {
        real = CANDELABRUM_OF_INVOCATION;
    } else return;      /* you have nothing of special interest */

    if (!otmp) {
        /* If we get here, real and fake have been set up. */
        for(otmp = invent; otmp; otmp = otmp->nobj)
            if(otmp->otyp == real || (otmp->otyp == fake && !mtmp->iswiz))
                break;
    }

    if (otmp) { /* we have something to snatch */
        if (otmp->owornmask)
            remove_worn_item(otmp, true);
        freeinv(otmp);
        /* mpickobj wont merge otmp because none of the above things
           to steal are mergable */
        (void) mpickobj(mtmp,otmp);     /* may merge and free otmp */
        message_monster_object(MSG_M_STOLE_O, mtmp, otmp);
        if (can_teleport(mtmp->data) && !tele_restrict(mtmp))
            (void) rloc(mtmp, false);
    }
}


/* drop one object taken from a (possibly dead) monster's inventory */
static void mdrop_obj (struct monst *mon, struct obj *obj, bool verbosely) {
    int omx = mon->mx, omy = mon->my;

    if (obj->owornmask) {
        /* perform worn item handling if the monster is still alive */
        if (mon->mhp > 0) {
            mon->misc_worn_check &= ~obj->owornmask;
            update_mon_intrinsics(mon, obj, false, true);
            /* obj_no_longer_held(obj); -- done by place_object */
            if (obj->owornmask & W_WEP) setmnotwielded(mon, obj);
            /* don't charge for an owned saddle on dead steed */
        } else if (mon->mtame && (obj->owornmask & W_SADDLE) &&
                !obj->unpaid && costly_spot(omx, omy)) {
            obj->no_charge = 1;
        }
        obj->owornmask = 0L;
    }
    if (verbosely && cansee(omx, omy)) {
        message_monster_object(MSG_M_DROPS_O, mon, obj);
    }
    if (!flooreffects(obj, omx, omy, "fall")) {
        place_object(obj, omx, omy);
        stackobj(obj);
    }
}

/* some monsters bypass the normal rules for moving between levels or
   even leaving the game entirely; when that happens, prevent them from
   taking the Amulet or invocation tools with them */
void mdrop_special_objs (struct monst *mon) {
    struct obj *obj, *otmp;

    for (obj = mon->minvent; obj; obj = otmp) {
        otmp = obj->nobj;
        /* the Amulet, invocation tools, and Rider corpses resist even when
           artifacts and ordinary objects are given 0% resistance chance */
        if (obj_resists(obj, 0, 0)) {
            obj_extract_self(obj);
            mdrop_obj(mon, obj, false);
        }
    }
}

/* release the objects the creature is carrying */
// bool is_pet         /* If true, pet should keep wielded/worn items */
void relobj ( struct monst *mtmp, int show, bool is_pet) {
    struct obj *otmp;
    int omx = mtmp->mx, omy = mtmp->my;
    struct obj *keepobj = 0;
    struct obj *wep = MON_WEP(mtmp);
    bool item1 = false, item2 = false;

    if (!is_pet || mindless(mtmp->data) || is_animal(mtmp->data))
        item1 = item2 = true;
    if (!tunnels(mtmp->data) || !needspick(mtmp->data))
        item1 = true;

    while ((otmp = mtmp->minvent) != 0) {
        obj_extract_self(otmp);
        /* special case: pick-axe and unicorn horn are non-worn */
        /* items that we also want pets to keep 1 of */
        /* (It is a coincidence that these can also be wielded.) */
        if (otmp->owornmask || otmp == wep ||
                ((!item1 && otmp->otyp == PICK_AXE) ||
                 (!item2 && otmp->otyp == UNICORN_HORN && !otmp->cursed))) {
            if (is_pet) { /* dont drop worn/wielded item */
                if (otmp->otyp == PICK_AXE)
                    item1 = true;
                if (otmp->otyp == UNICORN_HORN && !otmp->cursed)
                    item2 = true;
                otmp->nobj = keepobj;
                keepobj = otmp;
                continue;
            }
        }
        mdrop_obj(mtmp, otmp, is_pet && flags.verbose);
    }

    /* put kept objects back */
    while ((otmp = keepobj) != (struct obj *)0) {
        keepobj = otmp->nobj;
        (void) add_to_minv(mtmp, otmp);
    }
    if (mtmp->mgold) {
        long g = mtmp->mgold;
        (void) mkgold(g, omx, omy);
        if (is_pet && cansee(omx, omy) && flags.verbose) {
            message_monster_int(MSG_M_DROPS_X_GOLD_PIECES, mtmp, g);
        }
        mtmp->mgold = 0L;
    }

    if (show & cansee(omx, omy))
        newsym(omx, omy);
}
