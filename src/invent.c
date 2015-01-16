/* See LICENSE in the root of this project for change info */

#include "invent.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "display_util.h"
#include "dungeon_util.h"
#include "align.h"
#include "apply.h"
#include "artifact.h"
#include "attrib.h"
#include "cmd.h"
#include "coord.h"
#include "dbridge.h"
#include "decl.h"
#include "display.h"
#include "do.h"
#include "do_name.h"
#include "do_wear.h"
#include "drawing.h"
#include "dungeon.h"
#include "eat.h"
#include "end.h"
#include "engrave.h"
#include "flag.h"
#include "global.h"
#include "hack.h"
#include "hacklib.h"
#include "light.h"
#include "mkobj.h"
#include "mon.h"
#include "mondata.h"
#include "monst.h"
#include "obj.h"
#include "objclass.h"
#include "objnam.h"
#include "onames.h"
#include "permonst.h"
#include "pickup.h"
#include "pline.h"
#include "pm.h"
#include "pm_props.h"
#include "polyself.h"
#include "pray.h"
#include "prop.h"
#include "quest.h"
#include "questpgr.h"
#include "read.h"
#include "rm.h"
#include "shk.h"
#include "timeout.h"
#include "trap.h"
#include "util.h"
#include "wield.h"
#include "wintype.h"
#include "worn.h"
#include "you.h"
#include "youprop.h"

#define NOINVSYM        '#'
#define CONTAINED_SYM   '>'     /* designator for inside a container */

static int lastinvnr = 51;      /* 0 ... 51 (never saved&restored) */

/* debuggers can wish for venom, which will become an invisible inventory
 * item without this.  putting it in inv_order would mean venom would
 * suddenly become a choice for all the inventory-class commands, which
 * would probably cause mass confusion.  the test for inventory venom
 * is only WIZARD and not flags.debug because the flags.debug can leave venom lying
 * around on a bones level for normal players to find.
 */
static char venom_inv[] = { VENOM_CLASS, 0 };   /* (constant) */

/* query objlist callback: return true if obj is at given location */
static coord only;

/* note: assumes ASCII; toggling a bit puts lowercase in front of uppercase */
#define inv_rank(o) ((o)->invlet ^ 040)

static const char removeables[] =
        { ARMOR_CLASS, WEAPON_CLASS, RING_CLASS, AMULET_CLASS, TOOL_CLASS, 0 };

static int this_type;

/*
 * Conversion from a class to a string for printing.
 * This must match the object class order.
 */
static const char *names[] = { 0,
        "Illegal objects", "Weapons", "Armor", "Rings", "Amulets",
        "Tools", "Comestibles", "Potions", "Scrolls", "Spellbooks",
        "Wands", "Coins", "Gems", "Boulders/Statues", "Iron balls",
        "Chains", "Venoms"
};

static const char oth_symbols[] = {
        CONTAINED_SYM,
        '\0'
};

static const char *oth_names[] = {
        "Bagged/Boxed items"
};

static char *invbuf = (char *)0;
static unsigned invbufsiz = 0;

void assigninvlet (struct obj *otmp) {
        bool inuse[52];
        int i;
        struct obj *obj;


        for(i = 0; i < 52; i++) inuse[i] = false;
        for(obj = invent; obj; obj = obj->nobj) if(obj != otmp) {
                i = obj->invlet;
                if('a' <= i && i <= 'z') inuse[i - 'a'] = true; else
                if('A' <= i && i <= 'Z') inuse[i - 'A' + 26] = true;
                if(i == otmp->invlet) otmp->invlet = 0;
        }
        if((i = otmp->invlet) &&
            (('a' <= i && i <= 'z') || ('A' <= i && i <= 'Z')))
                return;
        for(i = lastinvnr+1; i != lastinvnr; i++) {
                if(i == 52) { i = -1; continue; }
                if(!inuse[i]) break;
        }
        otmp->invlet = (inuse[i] ? NOINVSYM :
                        (i < 26) ? ('a'+i) : ('A'+i-26));
        lastinvnr = i;
}


/* sort the inventory; used by addinv() and doorganize() */
static void reorder_invent (void) {
        struct obj *otmp, *prev, *next;
        bool need_more_sorting;

        do {
            /*
             * We expect at most one item to be out of order, so this
             * isn't nearly as inefficient as it may first appear.
             */
            need_more_sorting = false;
            for (otmp = invent, prev = 0; otmp; ) {
                next = otmp->nobj;
                if (next && inv_rank(next) < inv_rank(otmp)) {
                    need_more_sorting = true;
                    if (prev) prev->nobj = next;
                    else      invent = next;
                    otmp->nobj = next->nobj;
                    next->nobj = otmp;
                    prev = next;
                } else {
                    prev = otmp;
                    otmp = next;
                }
            }
        } while (need_more_sorting);
}

/* returns true if obj  & otmp can be merged */
static bool mergable (struct obj *otmp, struct obj *obj) {
        if (obj->otyp != otmp->otyp) return false;
        if (obj->unpaid != otmp->unpaid ||
            obj->spe != otmp->spe || obj->dknown != otmp->dknown ||
            (obj->bknown != otmp->bknown && !Role_if(PM_PRIEST)) ||
            obj->cursed != otmp->cursed || obj->blessed != otmp->blessed ||
            obj->no_charge != otmp->no_charge ||
            obj->obroken != otmp->obroken ||
            obj->otrapped != otmp->otrapped ||
            obj->lamplit != otmp->lamplit ||
            obj->greased != otmp->greased ||
            obj->oeroded != otmp->oeroded ||
            obj->oeroded2 != otmp->oeroded2 ||
            obj->bypass != otmp->bypass)
            return(false);

        if ((obj->oclass==WEAPON_CLASS || obj->oclass==ARMOR_CLASS) &&
            (obj->oerodeproof!=otmp->oerodeproof || obj->rknown!=otmp->rknown))
            return false;

        if (obj->oclass == FOOD_CLASS && (obj->oeaten != otmp->oeaten ||
                                          obj->orotten != otmp->orotten))
            return(false);

        if (obj->otyp == CORPSE || obj->otyp == EGG || obj->otyp == TIN) {
                if (obj->corpsenm != otmp->corpsenm)
                                return false;
        }

        /* hatching eggs don't merge; ditto for revivable corpses */
        if ((obj->otyp == EGG && (obj->timed || otmp->timed)) ||
            (obj->otyp == CORPSE && otmp->corpsenm >= LOW_PM &&
                is_reviver(&mons[otmp->corpsenm])))
            return false;

        /* allow candle merging only if their ages are close */
        /* see begin_burn() for a reference for the magic "25" */
        if (Is_candle(obj) && obj->age/25 != otmp->age/25)
            return(false);

        /* burning potions of oil never merge */
        if (obj->otyp == POT_OIL && obj->lamplit)
            return false;

        /* don't merge surcharged item with base-cost item */
        if (obj->unpaid && !same_price(obj, otmp))
            return false;

        /* if they have names, make sure they're the same */
        if ( (obj->onamelth != otmp->onamelth &&
                ((obj->onamelth && otmp->onamelth) || obj->otyp == CORPSE)
             ) ||
            (obj->onamelth && otmp->onamelth &&
                    strncmp(ONAME(obj), ONAME(otmp), (int)obj->onamelth)))
                return false;

        /* for the moment, any additional information is incompatible */
        if (obj->oxlth || otmp->oxlth) return false;

        if(obj->oartifact != otmp->oartifact) return false;

        if(obj->known == otmp->known ||
                !objects[otmp->otyp].oc_uses_known) {
                return((bool)(objects[obj->otyp].oc_merge));
        } else return(false);
}


/* scan a list of objects to see whether another object will merge with
   one of them; used in pickup.c when all 52 inventory slots are in use,
   to figure out whether another object could still be picked up */
struct obj * merge_choice (struct obj *objlist, struct obj *obj) {
        struct monst *shkp;
        int save_nocharge;

        if (obj->otyp == SCR_SCARE_MONSTER)     /* punt on these */
            return (struct obj *)0;
        /* if this is an item on the shop floor, the attributes it will
           have when carried are different from what they are now; prevent
           that from eliciting an incorrect result from mergable() */
        save_nocharge = obj->no_charge;
        if (objlist == invent && obj->where == OBJ_FLOOR &&
                (shkp = shop_keeper(inside_shop(obj->ox, obj->oy))) != 0)
        {
            if (obj->no_charge) obj->no_charge = 0;
            /* A billable object won't have its `unpaid' bit set, so would
               erroneously seem to be a candidate to merge with a similar
               ordinary object.  That's no good, because once it's really
               picked up, it won't merge after all.  It might merge with
               another unpaid object, but we can't check that here (depends
               too much upon shk's bill) and if it doesn't merge it would
               end up in the '#' overflow inventory slot, so reject it now. */
            else if (inhishop(shkp)) return (struct obj *)0;
        }
        while (objlist) {
            if (mergable(objlist, obj)) break;
            objlist = objlist->nobj;
        }
        obj->no_charge = save_nocharge;
        return objlist;
}

/* merge obj with otmp and delete obj if types agree */
int merged (struct obj **potmp, struct obj **pobj) {
    struct obj *otmp = *potmp, *obj = *pobj;

    if(mergable(otmp, obj)) {
        /* Approximate age: we do it this way because if we were to
         * do it "accurately" (merge only when ages are identical)
         * we'd wind up never merging any corpses.
         * otmp->age = otmp->age*(1-proportion) + obj->age*proportion;
         *
         * Don't do the age manipulation if lit.  We would need
         * to stop the burn on both items, then merge the age,
         * then restart the burn.
         */
        if (!obj->lamplit)
            otmp->age = ((otmp->age*otmp->quan) + (obj->age*obj->quan))
                / (otmp->quan + obj->quan);

        otmp->quan += obj->quan;
        if (otmp->oclass == COIN_CLASS) otmp->owt = weight(otmp);
        else otmp->owt += obj->owt;
        if (!otmp->onamelth && obj->onamelth)
            otmp = *potmp = oname(otmp, ONAME(obj));
        obj_extract_self(obj);

        /* really should merge the timeouts */
        if (obj->lamplit) obj_merge_light_sources(obj, otmp);
        if (obj->timed) obj_stop_timers(obj);   /* follows lights */

        /* fixup for `#adjust' merging wielded darts, daggers, &c */
        if (obj->owornmask && carried(otmp)) {
            long wmask = otmp->owornmask | obj->owornmask;

            /* Both the items might be worn in competing slots;
               merger preference (regardless of which is which):
               primary weapon + alternate weapon -> primary weapon;
               primary weapon + quiver -> primary weapon;
               alternate weapon + quiver -> alternate weapon.
               (Prior to 3.3.0, it was not possible for the two
               stacks to be worn in different slots and `obj'
               didn't need to be unworn when merging.) */
            if (wmask & W_WEP) wmask = W_WEP;
            else if (wmask & W_SWAPWEP) wmask = W_SWAPWEP;
            else if (wmask & W_QUIVER) wmask = W_QUIVER;
            else {
                impossible("merging strangely worn items (%lx)", wmask);
                wmask = otmp->owornmask;
            }
            if ((otmp->owornmask & ~wmask) != 0L) setnotworn(otmp);
            setworn(otmp, wmask);
            setnotworn(obj);
        }

        obfree(obj,otmp);       /* free(obj), bill->otmp */
        return(1);
    }
    return 0;
}

/*
Adjust hero intrinsics as if this object was being added to the hero's
inventory.  Called _before_ the object has been added to the hero's
inventory.

This is called when adding objects to the hero's inventory normally (via
addinv) or when an object in the hero's inventory has been polymorphed
in-place.

It may be valid to merge this code with with addinv_core2().
*/
void addinv_core1 (struct obj *obj) {
        if (obj->oclass == COIN_CLASS) {
                u.ugold += obj->quan;
        } else if (obj->otyp == AMULET_OF_YENDOR) {
                if (u.uhave.amulet) impossible("already have amulet?");
                u.uhave.amulet = 1;
        } else if (obj->otyp == CANDELABRUM_OF_INVOCATION) {
                if (u.uhave.menorah) impossible("already have candelabrum?");
                u.uhave.menorah = 1;
        } else if (obj->otyp == BELL_OF_OPENING) {
                if (u.uhave.bell) impossible("already have silver bell?");
                u.uhave.bell = 1;
        } else if (obj->otyp == SPE_BOOK_OF_THE_DEAD) {
                if (u.uhave.book) impossible("already have the book?");
                u.uhave.book = 1;
        } else if (obj->oartifact) {
                if (is_quest_artifact(obj)) {
                    if (u.uhave.questart)
                        impossible("already have quest artifact?");
                    u.uhave.questart = 1;
                    artitouch();
                }
                set_artifact_intrinsic(obj, 1, W_ART);
        }
}

/*
Adjust hero intrinsics as if this object was being added to the hero's
inventory.  Called _after_ the object has been added to the hero's
inventory.

This is called when adding objects to the hero's inventory normally (via
addinv) or when an object in the hero's inventory has been polymorphed
in-place.
*/
void addinv_core2 (struct obj *obj) {
        if (confers_luck(obj)) {
                /* new luckstone must be in inventory by this point
                 * for correct calculation */
                set_moreluck();
        }
}

/*
Add obj to the hero's inventory.  Make sure the object is "free".
Adjust hero attributes as necessary.
*/
struct obj * addinv (struct obj *obj) {
    struct obj *otmp, *prev;

    if (obj->where != OBJ_FREE)
        panic("addinv: obj not free");
    obj->no_charge = 0;     /* not meaningful for invent */

    addinv_core1(obj);
    /* if handed gold, we're done */
    if (obj->oclass == COIN_CLASS)
        return obj;

    /* merge if possible; find end of chain in the process */
    for (prev = 0, otmp = invent; otmp; prev = otmp, otmp = otmp->nobj)
        if (merged(&otmp, &obj)) {
            obj = otmp;
            goto added;
        }
    /* didn't merge, so insert into chain */
    if (flags.invlet_constant || !prev) {
        if (flags.invlet_constant) assigninvlet(obj);
        obj->nobj = invent;         /* insert at beginning */
        invent = obj;
        if (flags.invlet_constant) reorder_invent();
    } else {
        prev->nobj = obj;           /* insert at end */
        obj->nobj = 0;
    }
    obj->where = OBJ_INVENT;

added:
    addinv_core2(obj);
    carry_obj_effects(obj);         /* carrying affects the obj */
    return(obj);
}

/*
 * Some objects are affected by being carried.
 * Make those adjustments here. Called _after_ the object
 * has been added to the hero's or monster's inventory,
 * and after hero's intrinsics have been updated.
 */
void carry_obj_effects (struct obj *obj) {
        /* Cursed figurines can spontaneously transform
           when carried. */
        if (obj->otyp == FIGURINE) {
                if (obj->cursed
                    && obj->corpsenm != NON_PM
                    && !dead_species(obj->corpsenm,true)) {
                        attach_fig_transform_timeout(obj);
                    }
        }
}


/* Add an item to the inventory unless we're fumbling or it refuses to be
 * held (via touch_artifact), and give a message.
 * If there aren't any free inventory slots, we'll drop it instead.
 * If both success and failure messages are NULL, then we're just doing the
 * fumbling/slot-limit checking for a silent grab.  In any case,
 * touch_artifact will print its own messages if they are warranted.
 */
struct obj * hold_another_object (struct obj *obj, const char *drop_fmt,
        const char *drop_arg, const char *hold_msg)
{
        char buf[BUFSZ];

        if (!Blind()) obj->dknown = 1;    /* maximize mergibility */
        if (obj->oartifact) {
            /* place_object may change these */
            bool crysknife = (obj->otyp == CRYSKNIFE);
            int oerode = obj->oerodeproof;
            bool wasUpolyd = Upolyd;

            /* in case touching this object turns out to be fatal */
            place_object(obj, u.ux, u.uy);

            if (!touch_artifact(obj, &youmonst)) {
                obj_extract_self(obj);  /* remove it from the floor */
                dropy(obj);             /* now put it back again :-) */
                return obj;
            } else if (wasUpolyd && !Upolyd) {
                /* loose your grip if you revert your form */
                if (drop_fmt) pline(drop_fmt, drop_arg);
                obj_extract_self(obj);
                dropy(obj);
                return obj;
            }
            obj_extract_self(obj);
            if (crysknife) {
                obj->otyp = CRYSKNIFE;
                obj->oerodeproof = oerode;
            }
        }
        if (Fumbling()) {
            if (drop_fmt) pline(drop_fmt, drop_arg);
            dropy(obj);
        } else {
            long oquan = obj->quan;
            int prev_encumbr = near_capacity(); /* before addinv() */

            /* encumbrance only matters if it would now become worse
               than max( current_value, stressed ) */
            if (prev_encumbr < MOD_ENCUMBER) prev_encumbr = MOD_ENCUMBER;
            /* addinv() may redraw the entire inventory, overwriting
               drop_arg when it comes from something like doname() */
            if (drop_arg) drop_arg = strcpy(buf, drop_arg);

            obj = addinv(obj);
            if (inv_cnt() > 52
                    || ((obj->otyp != LOADSTONE || !obj->cursed)
                        && near_capacity() > prev_encumbr)) {
                if (drop_fmt) pline(drop_fmt, drop_arg);
                /* undo any merge which took place */
                if (obj->quan > oquan) obj = splitobj(obj, oquan);
                dropx(obj);
            } else {
                if (flags.autoquiver && !uquiver && !obj->owornmask &&
                        (is_missile(obj) ||
                            ammo_and_launcher(obj, uwep) ||
                            ammo_and_launcher(obj, uswapwep)))
                    setuqwep(obj);
                if (hold_msg || drop_fmt) prinv(hold_msg, obj, oquan);
            }
        }
        return obj;
}

/* useup() all of an item regardless of its quantity */
void useupall (struct obj *obj) {
        setnotworn(obj);
        freeinv(obj);
        obfree(obj, (struct obj *)0);   /* deletes contents also */
}

void useup (struct obj *obj) {
        /*  Note:  This works correctly for containers because they */
        /*         (containers) don't merge.                        */
        if (obj->quan > 1L) {
                obj->in_use = false;    /* no longer in use */
                obj->quan--;
                obj->owt = weight(obj);
        } else {
                useupall(obj);
        }
}

/* use one charge from an item and possibly incur shop debt for it */
/* maybe_unpaid: false if caller handles shop billing */
void consume_obj_charge ( struct obj *obj, bool maybe_unpaid) {
        if (maybe_unpaid) check_unpaid(obj);
        obj->spe -= 1;
}


/*
Adjust hero's attributes as if this object was being removed from the
hero's inventory.  This should only be called from freeinv() and
where we are polymorphing an object already in the hero's inventory.

Should think of a better name...
*/
void freeinv_core (struct obj *obj) {
        if (obj->oclass == COIN_CLASS) {
                u.ugold -= obj->quan;
                obj->in_use = false;
                return;
        } else if (obj->otyp == AMULET_OF_YENDOR) {
                if (!u.uhave.amulet) impossible("don't have amulet?");
                u.uhave.amulet = 0;
        } else if (obj->otyp == CANDELABRUM_OF_INVOCATION) {
                if (!u.uhave.menorah) impossible("don't have candelabrum?");
                u.uhave.menorah = 0;
        } else if (obj->otyp == BELL_OF_OPENING) {
                if (!u.uhave.bell) impossible("don't have silver bell?");
                u.uhave.bell = 0;
        } else if (obj->otyp == SPE_BOOK_OF_THE_DEAD) {
                if (!u.uhave.book) impossible("don't have the book?");
                u.uhave.book = 0;
        } else if (obj->oartifact) {
                if (is_quest_artifact(obj)) {
                    if (!u.uhave.questart)
                        impossible("don't have quest artifact?");
                    u.uhave.questart = 0;
                }
                set_artifact_intrinsic(obj, 0, W_ART);
        }

        if (obj->otyp == LOADSTONE) {
                curse(obj);
        } else if (confers_luck(obj)) {
                set_moreluck();
        } else if (obj->otyp == FIGURINE && obj->timed) {
                (void) stop_timer(FIG_TRANSFORM, (void *) obj);
        }
}

/* remove an object from the hero's inventory */
void freeinv (struct obj *obj) {
        extract_nobj(obj, &invent);
        freeinv_core(obj);
}

void delallobj (int x, int y) {
        struct obj *otmp, *otmp2;

        for (otmp = level.objects[x][y]; otmp; otmp = otmp2) {
                if (otmp == uball)
                        unpunish();
                /* after unpunish(), or might get deallocated chain */
                otmp2 = otmp->nexthere;
                if (otmp == uchain)
                        continue;
                delobj(otmp);
        }
}


/* destroy object in fobj chain (if unpaid, it remains on the bill) */
void delobj (struct obj *obj) {
        bool update_map;

        if (obj->otyp == AMULET_OF_YENDOR ||
                        obj->otyp == CANDELABRUM_OF_INVOCATION ||
                        obj->otyp == BELL_OF_OPENING ||
                        obj->otyp == SPE_BOOK_OF_THE_DEAD) {
                /* player might be doing something stupid, but we
                 * can't guarantee that.  assume special artifacts
                 * are indestructible via drawbridges, and exploding
                 * chests, and golem creation, and ...
                 */
                return;
        }
        update_map = (obj->where == OBJ_FLOOR);
        obj_extract_self(obj);
        if (update_map) newsym(obj->ox, obj->oy);
        obfree(obj, (struct obj *) 0);  /* frees contents also */
}

struct obj * sobj_at (int n, int x, int y) {
        struct obj *otmp;

        for(otmp = level.objects[x][y]; otmp; otmp = otmp->nexthere)
                if(otmp->otyp == n)
                    return(otmp);
        return((struct obj *)0);
}


struct obj * carrying (int type) {
        struct obj *otmp;

        for(otmp = invent; otmp; otmp = otmp->nobj)
                if(otmp->otyp == type)
                        return(otmp);
        return((struct obj *) 0);
}

const char * currency (long amount) {
        if (amount == 1L) return "zorkmid";
        else return "zorkmids";
}

bool have_lizard (void) {
        struct obj *otmp;

        for(otmp = invent; otmp; otmp = otmp->nobj)
                if(otmp->otyp == CORPSE && otmp->corpsenm == PM_LIZARD)
                        return(true);
        return(false);
}

struct obj * o_on (unsigned int id, struct obj *objchn) {
        struct obj *temp;

        while(objchn) {
                if(objchn->o_id == id) return(objchn);
                if (Has_contents(objchn) && (temp = o_on(id,objchn->cobj)))
                        return temp;
                objchn = objchn->nobj;
        }
        return((struct obj *) 0);
}

bool obj_here (struct obj *obj, int x, int y) {
        struct obj *otmp;

        for(otmp = level.objects[x][y]; otmp; otmp = otmp->nexthere)
                if(obj == otmp) return(true);
        return(false);
}

struct obj * g_at (int x, int y) {
        struct obj *obj = level.objects[x][y];
        while(obj) {
            if (obj->oclass == COIN_CLASS) return obj;
            obj = obj->nexthere;
        }
        return((struct obj *)0);
}

/* Make a gold object from the hero's gold. */
struct obj * mkgoldobj (long q) {
        struct obj *otmp;

        otmp = mksobj(GOLD_PIECE, false, false);
        u.ugold -= q;
        otmp->quan = q;
        otmp->owt = weight(otmp);
        return(otmp);
}

/* compact a string of inventory letters by dashing runs of letters */
static void compactify (char *buf) {
        int i1 = 1, i2 = 1;
        char ilet, ilet1, ilet2;

        ilet2 = buf[0];
        ilet1 = buf[1];
        buf[++i2] = buf[++i1];
        ilet = buf[i1];
        while(ilet) {
                if(ilet == ilet1+1) {
                        if(ilet1 == ilet2+1)
                                buf[i2 - 1] = ilet1 = '-';
                        else if(ilet2 == '-') {
                                buf[i2 - 1] = ++ilet1;
                                buf[i2] = buf[++i1];
                                ilet = buf[i1];
                                continue;
                        }
                }
                ilet2 = ilet1;
                ilet1 = ilet;
                buf[++i2] = buf[++i1];
                ilet = buf[i1];
        }
}

/* match the prompt for either 'T' or 'R' command */
static bool taking_off (const char *action) {
    return !strcmp(action, "take off") || !strcmp(action, "remove");
}

/* match the prompt for either 'W' or 'P' command */
static bool putting_on (const char *action) {
    return !strcmp(action, "wear") || !strcmp(action, "put on");
}

/*
 * Internal function used by display_inventory and getobj that can display
 * inventory and return a count as well as a letter. If out_cnt is not null,
 * any count returned from the menu selection is placed here.
 */
static char display_pickinv (const char *lets, bool want_reply, long *out_cnt, bool want_dump) {
    struct obj *otmp;
    struct obj **oarray;
    int i, j;
    char ilet, ret;
    char *invlet = flags.inv_order;
    int n, classcount;
    winid win;                              /* windows being used */
    static winid local_win = WIN_ERR;       /* window for partial menus */
    anything any;
    menu_item *selected;

    /* overriden by global flag */
    if (flags.perm_invent) {
        win = (lets && *lets) ? local_win : WIN_INVEN;
        /* create the first time used */
        if (win == WIN_ERR)
            win = local_win = create_nhwindow(NHW_MENU);
    } else
        win = WIN_INVEN;

    if (want_dump)   dump("", "Your inventory");

    /*
       Exit early if no inventory -- but keep going if we are doing
       a permanent inventory update.  We need to keep going so the
       permanent inventory window updates itself to remove the last
       item(s) dropped.  One down side:  the addition of the exception
       for permanent inventory window updates _can_ pop the window
       up when it's not displayed -- even if it's empty -- because we
       don't know at this level if its up or not.  This may not be
       an issue if empty checks are done before hand and the call
       to here is short circuited away.
       */
    if (!invent && !(flags.perm_invent && !lets && !want_reply)) {
        pline("Not carrying anything%s.", u.ugold ? " except gold" : "");
        if (want_dump) {
            dump("  Not carrying anything",
                    u.ugold ? " except gold." : ".");
        }
        return 0;
    }

    /* oxymoron? temporarily assign permanent inventory letters */
    if (!flags.invlet_constant) reassign();

    if (lets && strlen(lets) == 1) {
        /* when only one item of interest, use pline instead of menus;
           we actually use a fake message-line menu in order to allow
           the user to perform selection at the --More-- prompt for tty */
        ret = '\0';
        for (otmp = invent; otmp; otmp = otmp->nobj) {
            if (otmp->invlet == lets[0]) {
                ret = message_menu(lets[0],
                        want_reply ? PICK_ONE : PICK_NONE,
                        xprname(otmp, (char *)0, lets[0], true, 0L, 0L));
                if (out_cnt) *out_cnt = -1L;        /* select all */
                if (want_dump) {
                    char letbuf[7];
                    sprintf(letbuf, "  %c - ", lets[0]);
                    dump(letbuf,
                            xprname(otmp, (char *)0, lets[0], true, 0L, 0L));
                }
                break;
            }
        }
        return ret;
    }

    /* count the number of items */
    for (n = 0, otmp = invent; otmp; otmp = otmp->nobj)
        if(!lets || !*lets || index(lets, otmp->invlet)) n++;

    /* Make a temporary array to store the objects sorted */
    oarray = (struct obj **)malloc(n*sizeof(struct obj*));

    /* Add objects to the array */
    i = 0;
    for(otmp = invent; otmp; otmp = otmp->nobj)
        if(!lets || !*lets || index(lets, otmp->invlet)) {
            if (iflags.sortloot == 'f') {
                /* Insert object at correct index */
                for (j = i; j; j--) {
                    if (strcmpi(cxname2(otmp), cxname2(oarray[j-1]))>0) break;
                    oarray[j] = oarray[j-1];
                }
                oarray[j] = otmp;
                i++;
            } else {
                /* Just add it to the array */
                oarray[i++] = otmp;
            }
        }

    start_menu(win);
nextclass:
    classcount = 0;
    any.a_void = 0;         /* set all bits to zero */
    for(i = 0; i < n; i++) {
        otmp = oarray[i];
        ilet = otmp->invlet;
        if (!flags.sortpack || otmp->oclass == *invlet) {
            if (flags.sortpack && !classcount) {
                any.a_void = 0;             /* zero */
                add_menu(win, NO_GLYPH, &any, 0, 0, ATR_INVERSE,
                        let_to_name(*invlet, false), MENU_UNSELECTED);
                if (want_dump)
                    dump("  ", let_to_name(*invlet, false));
                classcount++;
            }
            any.a_char = ilet;
            add_menu(win, obj_to_glyph(otmp),
                    &any, ilet, 0, ATR_NONE, doname(otmp),
                    MENU_UNSELECTED);
            if (want_dump) {
                char letbuf[7];
                sprintf(letbuf, "  %c - ", ilet);
                dump(letbuf, doname(otmp));
            }
        }
    }
    if (flags.sortpack) {
        if (*++invlet) goto nextclass;
        if (--invlet != venom_inv) {
            invlet = venom_inv;
            goto nextclass;
        }
    }
    free(oarray);
    end_menu(win, (char *) 0);

    n = select_menu(win, want_reply ? PICK_ONE : PICK_NONE, &selected);
    if (n > 0) {
        ret = selected[0].item.a_char;
        if (out_cnt) *out_cnt = selected[0].count;
        free((void *)selected);
    } else
        ret = !n ? '\0' : '\033';   /* cancelled */
    if (want_dump)  dump("", "");

    return ret;
}

/*
 * getobj returns:
 *      struct obj *xxx:        object to do something with.
 *      (struct obj *) 0        error return: no object.
 *      &zeroobj                explicitly no object (as in w-).
 */
struct obj * getobj (const char *let, const char *word) {
        struct obj *otmp;
        char ilet;
        char buf[BUFSZ], qbuf[QBUFSZ];
        char lets[BUFSZ], altlets[BUFSZ], *ap;
        int foo = 0;
        char *bp = buf;
        signed char allowcnt = 0;       /* 0, 1 or 2 */
        bool allowgold = false;      /* can't use gold because they don't have any */
        bool usegold = false;        /* can't use gold because its illegal */
        bool allowall = false;
        bool allownone = false;
        bool useboulder = false;
        signed char foox = 0;
        long cnt;
        bool prezero = false;
        long dummymask;

        if(*let == ALLOW_COUNT) let++, allowcnt = 1;
        if(*let == COIN_CLASS) let++,
                usegold = true, allowgold = (u.ugold ? true : false);

        /* Equivalent of an "ugly check" for gold */
        if (usegold && !strcmp(word, "eat") &&
            (!metallivorous(youmonst.data)
             || youmonst.data == &mons[PM_RUST_MONSTER]))
                usegold = allowgold = false;

        if(*let == ALL_CLASSES) let++, allowall = true;
        if(*let == ALLOW_NONE) let++, allownone = true;
        /* "ugly check" for reading fortune cookies, part 1 */
        /* The normal 'ugly check' keeps the object on the inventory list.
         * We don't want to do that for shirts/cookies, so the check for
         * them is handled a bit differently (and also requires that we set
         * allowall in the caller)
         */
        if(allowall && !strcmp(word, "read")) allowall = false;

        /* another ugly check: show boulders (not statues) */
        if(*let == WEAPON_CLASS &&
           !strcmp(word, "throw") && throws_rocks(youmonst.data))
            useboulder = true;

        if(allownone) *bp++ = '-';
        if(allowgold) *bp++ = def_oc_syms[COIN_CLASS];
        if(bp > buf && bp[-1] == '-') *bp++ = ' ';
        ap = altlets;

        ilet = 'a';
        for (otmp = invent; otmp; otmp = otmp->nobj) {
            if (!flags.invlet_constant)
                otmp->invlet = ilet;    /* reassign() */
            if (!*let || index(let, otmp->oclass)
                || (useboulder && otmp->otyp == BOULDER)
                ) {
                int otyp = otmp->otyp;
                bp[foo++] = otmp->invlet;

                /* ugly check: remove inappropriate things */
                if ((taking_off(word) &&
                    (!(otmp->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL))
                     || (otmp==uarm && uarmc)
                     || (otmp==uarmu && (uarm || uarmc))
                    ))
                || (putting_on(word) &&
                     (otmp->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL)))
                                                        /* already worn */
                || (!strcmp(word, "ready") &&
                    (otmp == uwep || (otmp == uswapwep && u.twoweap)))
                    ) {
                        foo--;
                        foox++;
                }

                /* Second ugly check; unlike the first it won't trigger an
                 * "else" in "you don't have anything else to ___".
                 */
                else if ((putting_on(word) &&
                    ((otmp->oclass == FOOD_CLASS && otmp->otyp != MEAT_RING) ||
                    (otmp->oclass == TOOL_CLASS &&
                     otyp != BLINDFOLD && otyp != TOWEL && otyp != LENSES)))
                || (!strcmp(word, "wield") &&
                    (otmp->oclass == TOOL_CLASS && !is_weptool(otmp)))
                || (!strcmp(word, "eat") && !is_edible(otmp))
                || (!strcmp(word, "sacrifice") &&
                    (otyp != CORPSE &&
                     otyp != AMULET_OF_YENDOR && otyp != FAKE_AMULET_OF_YENDOR))
                || (!strcmp(word, "write with") &&
                    (otmp->oclass == TOOL_CLASS &&
                     otyp != MAGIC_MARKER && otyp != TOWEL))
                || (!strcmp(word, "tin") &&
                    (otyp != CORPSE || !tinnable(otmp)))
                || (!strcmp(word, "rub") &&
                    ((otmp->oclass == TOOL_CLASS &&
                      otyp != OIL_LAMP && otyp != MAGIC_LAMP &&
                      otyp != BRASS_LANTERN) ||
                     (otmp->oclass == GEM_CLASS && !is_graystone(otmp))))
                || (!strncmp(word, "rub on the stone", 16) &&
                    *let == GEM_CLASS &&        /* using known touchstone */
                    otmp->dknown && objects[otyp].oc_name_known)
                || ((!strcmp(word, "use or apply") ||
                        !strcmp(word, "untrap with")) &&
                     /* Picks, axes, pole-weapons, bullwhips */
                    ((otmp->oclass == WEAPON_CLASS && !is_pick(otmp) &&
                      !is_axe(otmp) && !is_pole(otmp) && otyp != BULLWHIP) ||
                     (otmp->oclass == POTION_CLASS &&
                     /* only applicable potion is oil, and it will only
                        be offered as a choice when already discovered */
                     (otyp != POT_OIL || !otmp->dknown ||
                      !objects[POT_OIL].oc_name_known)) ||
                     (otmp->oclass == FOOD_CLASS &&
                      otyp != CREAM_PIE && otyp != EUCALYPTUS_LEAF) ||
                     (otmp->oclass == GEM_CLASS && !is_graystone(otmp))))
                || (!strcmp(word, "invoke") &&
                    (!otmp->oartifact && !objects[otyp].oc_unique &&
                     (otyp != FAKE_AMULET_OF_YENDOR || otmp->known) &&
                     otyp != CRYSTAL_BALL &&    /* #invoke synonym for apply */
                   /* note: presenting the possibility of invoking non-artifact
                      mirrors and/or lamps is a simply a cruel deception... */
                     otyp != MIRROR && otyp != MAGIC_LAMP &&
                     (otyp != OIL_LAMP ||       /* don't list known oil lamp */
                      (otmp->dknown && objects[OIL_LAMP].oc_name_known))))
                || (!strcmp(word, "untrap with") &&
                    (otmp->oclass == TOOL_CLASS && otyp != CAN_OF_GREASE))
                || (!strcmp(word, "charge") && !is_chargeable(otmp))
                    )
                        foo--;
                /* ugly check for unworn armor that can't be worn */
                else if (putting_on(word) && *let == ARMOR_CLASS &&
                         !canwearobj(otmp, &dummymask, false)) {
                        foo--;
                        allowall = true;
                        *ap++ = otmp->invlet;
                }
            } else {

                /* "ugly check" for reading fortune cookies, part 2 */
                if ((!strcmp(word, "read") &&
                    (otmp->otyp == FORTUNE_COOKIE
                        || otmp->otyp == T_SHIRT
                    )))
                        allowall = true;
            }

            if(ilet == 'z') ilet = 'A'; else ilet++;
        }
        bp[foo] = 0;
        if(foo == 0 && bp > buf && bp[-1] == ' ') *--bp = 0;
        strcpy(lets, bp);       /* necessary since we destroy buf */
        if(foo > 5)                     /* compactify string */
                compactify(bp);
        *ap = '\0';

        if(!foo && !allowall && !allowgold && !allownone) {
                You("don't have anything %sto %s.",
                        foox ? "else " : "", word);
                return((struct obj *)0);
        }
        for(;;) {
                cnt = 0;
                if (allowcnt == 2) allowcnt = 1;  /* abort previous count */
                if(!buf[0]) {
                        sprintf(qbuf, "What do you want to %s? [*]", word);
                } else {
                        sprintf(qbuf, "What do you want to %s? [%s or ?*]",
                                word, buf);
                }
                ilet = yn_function(qbuf, (char *)0, '\0');
                if(ilet == '0') prezero = true;
                while(digit(ilet) && allowcnt) {
                        if (ilet != '?' && ilet != '*') savech(ilet);
                        cnt = 10*cnt + (ilet - '0');
                        allowcnt = 2;   /* signal presence of cnt */
                        ilet = readchar();
                }
                if(digit(ilet)) {
                        pline("No count allowed with this command.");
                        continue;
                }
                if(index(quitchars,ilet)) {
                    if(flags.verbose)
                        plines(Never_mind);
                    return((struct obj *)0);
                }
                if(ilet == '-') {
                        return(allownone ? &zeroobj : (struct obj *) 0);
                }
                if(ilet == def_oc_syms[COIN_CLASS]) {
                        if (!usegold) {
                            if (!strncmp(word, "rub on ", 7)) {
                                /* the dangers of building sentences... */
                                You("cannot rub gold%s.", word + 3);
                            } else {
                                You("cannot %s gold.", word);
                            }
                            return(struct obj *)0;
                        } else if (!allowgold) {
                                You("are not carrying any gold.");
                                return(struct obj *)0;
                        }
                        if(cnt == 0 && prezero) return((struct obj *)0);
                        /* Historic note: early Nethack had a bug which was
                         * first reported for Larn, where trying to drop 2^32-n
                         * gold pieces was allowed, and did interesting things
                         * to your money supply.  The LRS is the tax bureau
                         * from Larn.
                         */
                        if(cnt < 0) {
        pline_The("LRS would be very interested to know you have that much.");
                                return(struct obj *)0;
                        }

                        if(!(allowcnt == 2 && cnt < u.ugold))
                                cnt = u.ugold;
                        return(mkgoldobj(cnt));
                }
                if(ilet == '?' || ilet == '*') {
                    char *allowed_choices = (ilet == '?') ? lets : (char *)0;
                    long ctmp = 0;

                    if (ilet == '?' && !*lets && *altlets)
                        allowed_choices = altlets;
                    ilet = display_pickinv(allowed_choices, true,
                                           allowcnt ? &ctmp : (long *)0
                                           , false
                                           );
                    if(!ilet) continue;
                    if (allowcnt && ctmp >= 0) {
                        cnt = ctmp;
                        if (!cnt) prezero = true;
                        allowcnt = 2;
                    }
                    if(ilet == '\033') {
                        if(flags.verbose)
                            plines(Never_mind);
                        return((struct obj *)0);
                    }
                    /* they typed a letter (not a space) at the prompt */
                }
                if(allowcnt == 2 && !strcmp(word,"throw")) {
                    /* permit counts for throwing gold, but don't accept
                     * counts for other things since the throw code will
                     * split off a single item anyway */
                        allowcnt = 1;
                    if(cnt == 0 && prezero) return((struct obj *)0);
                    if(cnt > 1) {
                        You("can only throw one item at a time.");
                        continue;
                    }
                }
                savech(ilet);
                for (otmp = invent; otmp; otmp = otmp->nobj)
                        if (otmp->invlet == ilet) break;
                if(!otmp) {
                        You("don't have that object.");
                        continue;
                } else if (cnt < 0 || otmp->quan < cnt) {
                        You("don't have that many!  You have only %ld.",
                            otmp->quan);
                        continue;
                }
                break;
        }
        if(!allowall && let && !index(let,otmp->oclass)
           ) {
                silly_thing(word, otmp);
                return((struct obj *)0);
        }
        if(allowcnt == 2) {     /* cnt given */
            if(cnt == 0) return (struct obj *)0;
            if(cnt != otmp->quan) {
                /* don't split a stack of cursed loadstones */
                if (otmp->otyp == LOADSTONE && otmp->cursed)
                    /* kludge for canletgo()'s can't-drop-this message */
                    otmp->corpsenm = (int) cnt;
                else
                    otmp = splitobj(otmp, cnt);
            }
        }
        return(otmp);
}

void silly_thing (const char *word, struct obj *otmp) {
        const char *s1, *s2, *s3, *what;
        int ocls = otmp->oclass, otyp = otmp->otyp;

        s1 = s2 = s3 = 0;
        /* check for attempted use of accessory commands ('P','R') on armor
           and for corresponding armor commands ('W','T') on accessories */
        if (ocls == ARMOR_CLASS) {
            if (!strcmp(word, "put on"))
                s1 = "W", s2 = "wear", s3 = "";
            else if (!strcmp(word, "remove"))
                s1 = "T", s2 = "take", s3 = " off";
        } else if ((ocls == RING_CLASS || otyp == MEAT_RING) ||
                ocls == AMULET_CLASS ||
                (otyp == BLINDFOLD || otyp == TOWEL || otyp == LENSES)) {
            if (!strcmp(word, "wear"))
                s1 = "P", s2 = "put", s3 = " on";
            else if (!strcmp(word, "take off"))
                s1 = "R", s2 = "remove", s3 = "";
        }
        if (s1) {
            what = "that";
            /* quantity for armor and accessory objects is always 1,
               but some things should be referred to as plural */
            if (otyp == LENSES || is_gloves(otmp) || is_boots(otmp))
                what = "those";
            pline("Use the '%s' command to %s %s%s.", s1, s2, what, s3);
        } else {
            pline(silly_thing_to, word);
        }
}


static int ckvalidcat (const struct obj *otmp) {
    /* use allow_category() from pickup.c */
    return((int)allow_category(otmp));
}

static int ckunpaid (const struct obj *otmp) {
        return((int)(otmp->unpaid));
}

bool wearing_armor (void) {
        return((bool)(uarm || uarmc || uarmf || uarmg || uarmh || uarms || uarmu));
}

bool is_worn (const struct obj *otmp) {
    return !!(otmp->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL |
                        W_SADDLE |
                        W_WEP | W_SWAPWEP | W_QUIVER));
}

/* interactive version of getobj - used for Drop, Identify and */
/* Takeoff (A). Return the number of times fn was called successfully */
/* If combo is true, we just use this to get a category list */
// bool combo,          /* combination menu flag */
int ggetobj ( const char *word, int (*fn)(struct obj *), int mx, bool combo,
        unsigned *resultflags)
{
    int (*ckfn)(const struct obj *) = NULL;
    bool (*filter)(const struct obj *) = NULL;
    bool takeoff, ident, allflag, m_seen;
    int itemcount;
    int oletct, iletct, allowgold, unpaid, oc_of_sym;
    char sym, *ip, olets[MAXOCLASSES+5], ilets[MAXOCLASSES+5];
    char extra_removeables[3+1];    /* uwep,uswapwep,uquiver */
    char buf[BUFSZ], qbuf[QBUFSZ];

    if (resultflags) *resultflags = 0;
    allowgold = (u.ugold && !strcmp(word, "drop")) ? 1 : 0;
    takeoff = ident = allflag = m_seen = false;
    if(!invent && !allowgold){
        You("have nothing to %s.", word);
        return(0);
    }
    add_valid_menu_class(0);        /* reset */
    if (taking_off(word)) {
        takeoff = true;
        filter = is_worn;
    } else if (!strcmp(word, "identify")) {
        ident = true;
        filter = not_fully_identified;
    }

    iletct = collect_obj_classes(ilets, invent,
            false,
            (allowgold != 0),
            filter, &itemcount);
    unpaid = count_unpaid(invent);

    if (ident && !iletct) {
        return -1;          /* no further identifications */
    } else if (!takeoff && (unpaid || invent)) {
        ilets[iletct++] = ' ';
        if (unpaid) ilets[iletct++] = 'u';
        if (count_buc(invent, BUC_BLESSED))  ilets[iletct++] = 'B';
        if (count_buc(invent, BUC_UNCURSED)) ilets[iletct++] = 'U';
        if (count_buc(invent, BUC_CURSED))   ilets[iletct++] = 'C';
        if (count_buc(invent, BUC_UNKNOWN))  ilets[iletct++] = 'X';
        if (invent) ilets[iletct++] = 'a';
    } else if (takeoff && invent) {
        ilets[iletct++] = ' ';
    }
    ilets[iletct++] = 'i';
    if (!combo)
        ilets[iletct++] = 'm';      /* allow menu presentation on request */
    ilets[iletct] = '\0';

    for (;;) {
        sprintf(qbuf,"What kinds of thing do you want to %s? [%s]",
                word, ilets);
        getlin(qbuf, buf);
        if (buf[0] == '\033') return(0);
        if (index(buf, 'i')) {
            if (display_inventory((char *)0, true) == '\033') return 0;
        } else
            break;
    }

    extra_removeables[0] = '\0';
    if (takeoff) {
        /* arbitrary types of items can be placed in the weapon slots
           [any duplicate entries in extra_removeables[] won't matter] */
        if (uwep) (void)strkitten(extra_removeables, uwep->oclass);
        if (uswapwep) (void)strkitten(extra_removeables, uswapwep->oclass);
        if (uquiver) (void)strkitten(extra_removeables, uquiver->oclass);
    }

    ip = buf;
    olets[oletct = 0] = '\0';
    while ((sym = *ip++) != '\0') {
        if (sym == ' ') continue;
        oc_of_sym = def_char_to_objclass(sym);
        if (takeoff && oc_of_sym != MAXOCLASSES) {
            if (index(extra_removeables, oc_of_sym)) {
                ;   /* skip rest of takeoff checks */
            } else if (!index(removeables, oc_of_sym)) {
                pline("Not applicable.");
                return 0;
            } else if (oc_of_sym == ARMOR_CLASS && !wearing_armor()) {
                You("are not wearing any armor.");
                return 0;
            } else if (oc_of_sym == WEAPON_CLASS &&
                    !uwep && !uswapwep && !uquiver) {
                You("are not wielding anything.");
                return 0;
            } else if (oc_of_sym == RING_CLASS && !uright && !uleft) {
                You("are not wearing rings.");
                return 0;
            } else if (oc_of_sym == AMULET_CLASS && !uamul) {
                You("are not wearing an amulet.");
                return 0;
            } else if (oc_of_sym == TOOL_CLASS && !ublindf) {
                You("are not wearing a blindfold.");
                return 0;
            }
        }

        if (oc_of_sym == COIN_CLASS && !combo) {
            if (allowgold == 1)
                (*fn)(mkgoldobj(u.ugold));
            else if (!u.ugold)
                You("have no gold.");
            allowgold = 2;
        } else if (sym == 'a') {
            allflag = true;
        } else if (sym == 'A') {
            /* same as the default */ ;
        } else if (sym == 'u') {
            add_valid_menu_class('u');
            ckfn = ckunpaid;
        } else if (sym == 'B') {
            add_valid_menu_class('B');
            ckfn = ckvalidcat;
        } else if (sym == 'U') {
            add_valid_menu_class('U');
            ckfn = ckvalidcat;
        } else if (sym == 'C') {
            add_valid_menu_class('C');
            ckfn = ckvalidcat;
        } else if (sym == 'X') {
            add_valid_menu_class('X');
            ckfn = ckvalidcat;
        } else if (sym == 'm') {
            m_seen = true;
        } else if (oc_of_sym == MAXOCLASSES) {
            You("don't have any %c's.", sym);
        } else if (oc_of_sym != VENOM_CLASS) {      /* suppress venom */
            if (!index(olets, oc_of_sym)) {
                add_valid_menu_class(oc_of_sym);
                olets[oletct++] = oc_of_sym;
                olets[oletct] = 0;
            }
        }
    }

    if (m_seen) {
        return (allflag || (!oletct && ckfn != ckunpaid)) ? -2 : -3;
    } else if (combo && !allflag) {
        return 0;
    } else if (allowgold == 2 && !oletct) {
        return 1;   /* you dropped gold (or at least tried to) */
    } else {
        int cnt = askchain(&invent, olets, allflag, fn, ckfn, mx, word);
        /*
         * askchain() has already finished the job in this case
         * so set a special flag to convey that back to the caller
         * so that it won't continue processing.
         * Fix for bug C331-1 reported by Irina Rempt-Drijfhout.
         */
        if (combo && allflag && resultflags)
            *resultflags |= ALL_FINISHED;
        return cnt;
    }
}

/*
 * Walk through the chain starting at objchn and ask for all objects
 * with olet in olets (if nonNULL) and satisfying ckfn (if nonnull)
 * whether the action in question (i.e., fn) has to be performed.
 * If allflag then no questions are asked. Max gives the max nr of
 * objects to be treated. Return the number of objects treated.
 */
// const char *word       /* olets is an Obj Class char array */
int askchain ( struct obj **objchn, const char *olets, int allflag,
        int (*fn)( struct obj *), int (*ckfn)(const struct obj *), int mx,
        const char *word)
{
        struct obj *otmp, *otmp2, *otmpo;
        char sym, ilet;
        int cnt = 0, dud = 0, tmp;
        bool takeoff, nodot, ident, ininv;
        char qbuf[QBUFSZ];

        takeoff = taking_off(word);
        ident = !strcmp(word, "identify");
        nodot = (!strcmp(word, "nodot") || !strcmp(word, "drop") ||
                 ident || takeoff);
        ininv = (*objchn == invent);
        /* Changed so the askchain is interrogated in the order specified.
         * For example, if a person specifies =/ then first all rings will be
         * asked about followed by all wands -dgk
         */
nextclass:
        ilet = 'a'-1;
        if (*objchn && (*objchn)->oclass == COIN_CLASS)
                ilet--;         /* extra iteration */
        for (otmp = *objchn; otmp; otmp = otmp2) {
                if(ilet == 'z') ilet = 'A'; else ilet++;
                otmp2 = otmp->nobj;
                if (olets && *olets && otmp->oclass != *olets) continue;
                if (takeoff && !is_worn(otmp)) continue;
                if (ident && !not_fully_identified(otmp)) continue;
                if (ckfn && !(*ckfn)(otmp)) continue;
                if (!allflag) {
                        strcpy(qbuf, !ininv ? doname(otmp) :
                                xprname(otmp, (char *)0, ilet, !nodot, 0L, 0L));
                        strcat(qbuf, "?");
                        sym = (takeoff || ident || otmp->quan < 2L) ?
                                nyaq(qbuf) : nyNaq(qbuf);
                }
                else    sym = 'y';

                otmpo = otmp;
                if (sym == '#') {
                 /* Number was entered; split the object unless it corresponds
                    to 'none' or 'all'.  2 special cases: cursed loadstones and
                    welded weapons (eg, multiple daggers) will remain as merged
                    unit; done to avoid splitting an object that won't be
                    droppable (even if we're picking up rather than dropping).
                  */
                    if (!yn_number)
                        sym = 'n';
                    else {
                        sym = 'y';
                        if (yn_number < otmp->quan && !welded(otmp) &&
                            (!otmp->cursed || otmp->otyp != LOADSTONE)) {
                            otmp = splitobj(otmp, yn_number);
                        }
                    }
                }
                switch(sym){
                case 'a':
                        allflag = 1;
                case 'y':
                        tmp = (*fn)(otmp);
                        if(tmp < 0) {
                            if (otmp != otmpo) {
                                /* split occurred, merge again */
                                (void) merged(&otmpo, &otmp);
                            }
                            goto ret;
                        }
                        cnt += tmp;
                        if(--mx == 0) goto ret;
                case 'n':
                        if(nodot) dud++;
                default:
                        break;
                case 'q':
                        /* special case for seffects() */
                        if (ident) cnt = -1;
                        goto ret;
                }
        }
        if (olets && *olets && *++olets)
                goto nextclass;
        if(!takeoff && (dud || cnt)) pline("That was all.");
        else if(!dud && !cnt) pline("No applicable objects.");
ret:
        return(cnt);
}


/*
 *      Object identification routines:
 */

/* make an object actually be identified; no display updating */
void fully_identify_obj (struct obj *otmp) {
    makeknown(otmp->otyp);
    if (otmp->oartifact) discover_artifact((signed char)otmp->oartifact);
    otmp->known = otmp->dknown = otmp->bknown = otmp->rknown = 1;
    if (otmp->otyp == EGG && otmp->corpsenm != NON_PM)
        learn_egg_type(otmp->corpsenm);
}

/* ggetobj callback routine; identify an object and give immediate feedback */
int identify (struct obj *otmp) {
    fully_identify_obj(otmp);
    prinv((char *)0, otmp, 0L);
    return 1;
}

/* menu of unidentified objects; select and identify up to id_limit of them */
static void menu_identify (int id_limit) {
    menu_item *pick_list;
    int n, i, first = 1;
    char buf[BUFSZ];
    /* assumptions:  id_limit > 0 and at least one unID'd item is present */

    while (id_limit) {
        sprintf(buf, "What would you like to identify %s?",
                first ? "first" : "next");
        n = query_objlist(buf, invent, SIGNAL_NOMENU|USE_INVLET|INVORDER_SORT,
                &pick_list, PICK_ANY, not_fully_identified);

        if (n > 0) {
            if (n > id_limit) n = id_limit;
            for (i = 0; i < n; i++, id_limit--)
                (void) identify(pick_list[i].item.a_obj);
            free((void *) pick_list);
        } else {
            if (n < 0) pline("That was all.");
            id_limit = 0; /* Stop now */
        }
        first = 0;
    }
}

/* dialog with user to identify a given number of items; 0 means all */
void identify_pack (int id_limit) {
    struct obj *obj, *the_obj;
    int n, unid_cnt;

    unid_cnt = 0;
    the_obj = 0;                /* if unid_cnt ends up 1, this will be it */
    for (obj = invent; obj; obj = obj->nobj)
        if (not_fully_identified(obj)) ++unid_cnt, the_obj = obj;

    if (!unid_cnt) {
        You("have already identified all of your possessions.");
    } else if (!id_limit) {
        /* identify everything */
        if (unid_cnt == 1) {
            (void) identify(the_obj);
        } else {

            /* TODO:  use fully_identify_obj and cornline/menu/whatever here */
            for (obj = invent; obj; obj = obj->nobj)
                if (not_fully_identified(obj)) (void) identify(obj);

        }
    } else {
        /* identify up to `id_limit' items */
        n = 0;
        if (n == 0 || n < -1)
            menu_identify(id_limit);
    }
}


// obj_to_let ( /* should of course only be called for things in invent */
static char obj_to_let ( struct obj *obj) {
        if (obj->oclass == COIN_CLASS)
                return GOLD_SYM;
        if (!flags.invlet_constant) {
                obj->invlet = NOINVSYM;
                reassign();
        }
        return obj->invlet;
}

/*
 * Print the indicated quantity of the given object.  If quan == 0L then use
 * the current quantity.
 */
void prinv (const char *prefix, struct obj *obj, long quan) {
        if (!prefix) prefix = "";
        pline("%s%s%s",
              prefix, *prefix ? " " : "",
              xprname(obj, (char *)0, obj_to_let(obj), true, 0L, quan));
}

//  const char *txt,        /* text to print instead of obj */
//  char let,               /* inventory letter */
//  bool dot,            /* append period; (dot && cost => Iu) */
//  long cost,              /* cost (for inventory of unpaid or expended items) */
//  long quan              /* if non-0, print this quantity, not obj->quan */
char * xprname (struct obj *obj, const char *txt, char let, bool dot, long cost, long quan) {
    static char li[BUFSZ];
    bool use_invlet = flags.invlet_constant && let != CONTAINED_SYM;
    long savequan = 0;

    if (quan && obj) {
        savequan = obj->quan;
        obj->quan = quan;
    }

    /*
     * If let is:
     *  *  Then obj == null and we are printing a total amount.
     *  >  Then the object is contained and doesn't have an inventory letter.
     */
    if (cost != 0 || let == '*') {
        /* if dot is true, we're doing Iu, otherwise Ix */
        sprintf(li, "%c - %-45s %6ld %s",
                (dot && use_invlet ? obj->invlet : let),
                (txt ? txt : doname(obj)), cost, currency(cost));
    } else if (obj && obj->oclass == COIN_CLASS) {
        sprintf(li, "%ld gold piece%s%s", obj->quan, plur(obj->quan),
                (dot ? "." : ""));
    } else {
        /* ordinary inventory display or pickup message */
        sprintf(li, "%c - %s%s",
                (use_invlet ? obj->invlet : let),
                (txt ? txt : doname(obj)), (dot ? "." : ""));
    }
    if (savequan) obj->quan = savequan;

    return li;
}

/* the 'i' command */
int ddoinv (void) {
        (void) display_inventory((char *)0, false);
        return 0;
}

/*
 * find_unpaid()
 *
 * Scan the given list of objects.  If last_found is NULL, return the first
 * unpaid object found.  If last_found is not NULL, then skip over unpaid
 * objects until last_found is reached, then set last_found to NULL so the
 * next unpaid object is returned.  This routine recursively follows
 * containers.
 */
static struct obj * find_unpaid (struct obj *list, struct obj **last_found) {
    struct obj *obj;

    while (list) {
        if (list->unpaid) {
            if (*last_found) {
                /* still looking for previous unpaid object */
                if (list == *last_found)
                    *last_found = (struct obj *) 0;
            } else
                return (*last_found = list);
        }
        if (Has_contents(list)) {
            if ((obj = find_unpaid(list->cobj, last_found)) != 0)
                return obj;
        }
        list = list->nobj;
    }
    return (struct obj *) 0;
}


/*
 * If lets == NULL or "", list all objects in the inventory.  Otherwise,
 * list all objects with object classes that match the order in lets.
 *
 * Returns the letter identifier of a selected item, or 0 if nothing
 * was selected.
 */
char display_inventory (const char *lets, bool want_reply) {
        return display_pickinv(lets, want_reply, (long *)0
                                , false
        );
}

/* See display_inventory. This is the same thing WITH dumpfile creation */
char dump_inventory (const char *lets, bool want_reply) {
  return display_pickinv(lets, want_reply, (long *)0, true);
}

/*
 * Returns the number of unpaid items within the given list.  This includes
 * contained objects.
 */
int count_unpaid (struct obj *list) {
    int count = 0;

    while (list) {
        if (list->unpaid) count++;
        if (Has_contents(list))
            count += count_unpaid(list->cobj);
        list = list->nobj;
    }
    return count;
}

/*
 * Returns the number of items with b/u/c/unknown within the given list.
 * This does NOT include contained objects.
 */
int count_buc (struct obj *list, int type) {
    int count = 0;

    while (list) {
        if (Role_if(PM_PRIEST)) list->bknown = true;
        switch(type) {
            case BUC_BLESSED:
                if (list->oclass != COIN_CLASS && list->bknown && list->blessed)
                    count++;
                break;
            case BUC_CURSED:
                if (list->oclass != COIN_CLASS && list->bknown && list->cursed)
                    count++;
                break;
            case BUC_UNCURSED:
                if (list->oclass != COIN_CLASS &&
                        list->bknown && !list->blessed && !list->cursed)
                    count++;
                break;
            case BUC_UNKNOWN:
                if (list->oclass != COIN_CLASS && !list->bknown)
                    count++;
                break;
            default:
                impossible("need count of curse status %d?", type);
                return 0;
        }
        list = list->nobj;
    }
    return count;
}

static void dounpaid (void) {
    winid win;
    struct obj *otmp, *marker;
    char ilet;
    char *invlet = flags.inv_order;
    int classcount, count, num_so_far;
    int save_unpaid = 0;        /* lint init */
    long cost, totcost;

    count = count_unpaid(invent);

    if (count == 1) {
        marker = (struct obj *) 0;
        otmp = find_unpaid(invent, &marker);

        /* see if the unpaid item is in the top level inventory */
        for (marker = invent; marker; marker = marker->nobj)
            if (marker == otmp) break;

        pline("%s", xprname(otmp, distant_name(otmp, doname),
                            marker ? otmp->invlet : CONTAINED_SYM,
                            true, unpaid_cost(otmp), 0L));
        return;
    }

    win = create_nhwindow(NHW_MENU);
    cost = totcost = 0;
    num_so_far = 0;     /* count of # printed so far */
    if (!flags.invlet_constant) reassign();

    do {
        classcount = 0;
        for (otmp = invent; otmp; otmp = otmp->nobj) {
            ilet = otmp->invlet;
            if (otmp->unpaid) {
                if (!flags.sortpack || otmp->oclass == *invlet) {
                    if (flags.sortpack && !classcount) {
                        putstr(win, 0, let_to_name(*invlet, true));
                        classcount++;
                    }

                    totcost += cost = unpaid_cost(otmp);
                    /* suppress "(unpaid)" suffix */
                    save_unpaid = otmp->unpaid;
                    otmp->unpaid = 0;
                    putstr(win, 0, xprname(otmp, distant_name(otmp, doname),
                                           ilet, true, cost, 0L));
                    otmp->unpaid = save_unpaid;
                    num_so_far++;
                }
            }
        }
    } while (flags.sortpack && (*++invlet));

    if (count > num_so_far) {
        /* something unpaid is contained */
        if (flags.sortpack)
            putstr(win, 0, let_to_name(CONTAINED_SYM, true));
        /*
         * Search through the container objects in the inventory for
         * unpaid items.  The top level inventory items have already
         * been listed.
         */
        for (otmp = invent; otmp; otmp = otmp->nobj) {
            if (Has_contents(otmp)) {
                marker = (struct obj *) 0;      /* haven't found any */
                while (find_unpaid(otmp->cobj, &marker)) {
                    totcost += cost = unpaid_cost(marker);
                    save_unpaid = marker->unpaid;
                    marker->unpaid = 0;    /* suppress "(unpaid)" suffix */
                    putstr(win, 0,
                           xprname(marker, distant_name(marker, doname),
                                   CONTAINED_SYM, true, cost, 0L));
                    marker->unpaid = save_unpaid;
                }
            }
        }
    }

    putstr(win, 0, "");
    putstr(win, 0, xprname((struct obj *)0, "Total:", '*', false, totcost, 0L));
    display_nhwindow(win, false);
    destroy_nhwindow(win);
}


/* query objlist callback: return true if obj type matches "this_type" */

static bool this_type_only (const struct obj *obj) {
    return (obj->oclass == this_type);
}

/* the 'I' command */
int dotypeinv (void) {
        char c = '\0';
        int n, i = 0;
        char *extra_types, types[BUFSZ];
        int class_count, oclass, unpaid_count, itemcount;
        bool billx = *u.ushops && doinvbill(0);
        menu_item *pick_list;
        bool traditional = true;
        const char *prompt = "What type of object do you want an inventory of?";

        if (!invent && !u.ugold && !billx) {
            You("aren't carrying anything.");
            return 0;
        }
        unpaid_count = count_unpaid(invent);
        traditional = false;
        i = UNPAID_TYPES;
        if (billx) i |= BILLED_TYPES;
        n = query_category(prompt, invent, i, &pick_list, PICK_ONE);
        if (!n) return 0;
        this_type = c = pick_list[0].item.a_int;
        free((void *) pick_list);
        if (traditional) {
            /* collect a list of classes of objects carried, for use as a prompt */
            types[0] = 0;
            class_count = collect_obj_classes(types, invent, false, (u.ugold != 0), NULL, &itemcount);
            if (unpaid_count) {
                strcat(types, "u");
                class_count++;
            }
            if (billx) {
                strcat(types, "x");
                class_count++;
            }
            /* add everything not already included; user won't see these */
            extra_types = eos(types);
            *extra_types++ = '\033';
            if (!unpaid_count) *extra_types++ = 'u';
            if (!billx) *extra_types++ = 'x';
            *extra_types = '\0';        /* for index() */
            for (i = 0; i < MAXOCLASSES; i++)
                if (!index(types, def_oc_syms[i])) {
                    *extra_types++ = def_oc_syms[i];
                    *extra_types = '\0';
                }

            if(class_count > 1) {
                c = yn_function(prompt, types, '\0');
                savech(c);
                if(c == '\0') {
                        clear_nhwindow(WIN_MESSAGE);
                        return 0;
                }
            } else {
                /* only one thing to itemize */
                if (unpaid_count)
                    c = 'u';
                else if (billx)
                    c = 'x';
                else
                    c = types[0];
            }
        }
        if (c == 'x') {
            if (billx)
                (void) doinvbill(1);
            else
                pline("No used-up objects on your shopping bill.");
            return 0;
        }
        if (c == 'u') {
            if (unpaid_count)
                dounpaid();
            else
                You("are not carrying any unpaid objects.");
            return 0;
        }
        if (traditional) {
            oclass = def_char_to_objclass(c); /* change to object class */
            if (oclass == COIN_CLASS) {
                return doprgold();
            } else if (index(types, c) > index(types, '\033')) {
                You("have no such objects.");
                return 0;
            }
            this_type = oclass;
        }
        if (query_objlist(NULL, invent,
                    (flags.invlet_constant ? USE_INVLET : 0)|INVORDER_SORT,
                    &pick_list, PICK_NONE, this_type_only) > 0)
        {
            free(pick_list);
        }
        return 0;
}

/* return a string describing the dungeon feature at <x,y> if there
   is one worth mentioning at that location; otherwise null */
const char * dfeature_at (int x, int y, char *buf) {
        struct rm *lev = &levl[x][y];
        int ltyp = lev->typ, cmap = -1;
        const char *dfeature = 0;
        static char altbuf[BUFSZ];

        if (IS_DOOR(ltyp)) {
            switch (lev->doormask) {
            case D_NODOOR:      cmap = S_ndoor; break;  /* "doorway" */
            case D_ISOPEN:      cmap = S_vodoor; break; /* "open door" */
            case D_BROKEN:      dfeature = "broken door"; break;
            default:    cmap = S_vcdoor; break; /* "closed door" */
            }
            /* override door description for open drawbridge */
            if (is_drawbridge_wall(x, y) >= 0)
                dfeature = "open drawbridge portcullis",  cmap = -1;
        } else if (IS_FOUNTAIN(ltyp))
            cmap = S_fountain;                          /* "fountain" */
        else if (IS_THRONE(ltyp))
            cmap = S_throne;                            /* "opulent throne" */
        else if (is_lava(x,y))
            cmap = S_lava;                              /* "molten lava" */
        else if (is_ice(x,y))
            cmap = S_ice;                               /* "ice" */
        else if (is_pool(x,y))
            dfeature = "pool of water";
        else if (IS_SINK(ltyp))
            cmap = S_sink;                              /* "sink" */
        else if (IS_ALTAR(ltyp)) {
            sprintf(altbuf, "altar to %s (%s)", a_gname(),
                    align_str(Amask2align(lev->altarmask & ~AM_SHRINE)));
            dfeature = altbuf;
        } else if ((x == xupstair && y == yupstair) ||
                 (x == sstairs.sx && y == sstairs.sy && sstairs.up))
            cmap = S_upstair;                           /* "staircase up" */
        else if ((x == xdnstair && y == ydnstair) ||
                 (x == sstairs.sx && y == sstairs.sy && !sstairs.up))
            cmap = S_dnstair;                           /* "staircase down" */
        else if (x == xupladder && y == yupladder)
            cmap = S_upladder;                          /* "ladder up" */
        else if (x == xdnladder && y == ydnladder)
            cmap = S_dnladder;                          /* "ladder down" */
        else if (ltyp == DRAWBRIDGE_DOWN)
            cmap = S_vodbridge;                 /* "lowered drawbridge" */
        else if (ltyp == DBWALL)
            cmap = S_vcdbridge;                 /* "raised drawbridge" */
        else if (IS_GRAVE(ltyp))
            cmap = S_grave;                             /* "grave" */
        else if (ltyp == TREE)
            cmap = S_tree;                              /* "tree" */
        else if (ltyp == IRONBARS)
            dfeature = "set of iron bars";

        if (cmap >= 0) dfeature = defsyms[cmap].explanation;
        if (dfeature) strcpy(buf, dfeature);
        return dfeature;
}

/* look at what is here; if there are many objects (5 or more),
   don't show them unless obj_cnt is 0 */
// int obj_cnt,    /* obj_cnt > 0 implies that autopickup is in progess */
int look_here(int obj_cnt) {
    struct obj *otmp;
    struct trap *trap;
    const char *verb = Blind() ? "feel" : "see";
    const char *dfeature = (char *)0;
    char fbuf[BUFSZ], fbuf2[BUFSZ];
    winid tmpwin;
    bool skip_objects = (obj_cnt >= 5), felt_cockatrice = false;

    if (u.uswallow && u.ustuck) {
        struct monst *mtmp = u.ustuck;
        char name[BUFSZ];
        mon_nam(name, BUFSZ, mtmp);
        sprintf(fbuf, "Contents of %s%s %s", name, possessive_suffix(name), mbodypart(mtmp, STOMACH));
        /* Skip "Contents of " by using fbuf index 12 */
        You("%s to %s what is lying in %s.",
                Blind() ? "try" : "look around", verb, &fbuf[12]);
        otmp = mtmp->minvent;
        if (otmp) {
            for (; otmp; otmp = otmp->nobj) {
                /* If swallower is an animal, it should have become stone but... */
                if (otmp->otyp == CORPSE)
                    feel_cockatrice(otmp, false);
            }
            if (Blind())
                strcpy(fbuf, "You feel");
            strcat(fbuf, ":");
            (void)display_minventory(mtmp, MINV_ALL, fbuf);
        } else {
            You("%s no objects here.", verb);
        }
        return Blind();
    }
    if (!skip_objects && (trap = t_at(u.ux, u.uy)) && trap->tseen)
        There("is %s here.", an(defsyms[trap_to_defsym(trap->ttyp)].explanation));

    otmp = level.objects[u.ux][u.uy];
    dfeature = dfeature_at(u.ux, u.uy, fbuf2);
    if (dfeature && !strcmp(dfeature, "pool of water") && Underwater)
        dfeature = 0;

    if (Blind()) {
        bool drift = Is_airlevel(&u.uz) || Is_waterlevel(&u.uz);
        if (dfeature && !strncmp(dfeature, "altar ", 6)) {
            /* don't say "altar" twice, dfeature has more info */
            You("try to feel what is here.");
        } else {
            You("try to feel what is %s%s.", drift ? "floating here" : "lying here on the ", drift ? "" : surface(u.ux, u.uy));
        }
        if (dfeature && !drift && !strcmp(dfeature, surface(u.ux, u.uy)))
            dfeature = 0; /* ice already identifed */
        if (!can_reach_floor()) {
            pline("But you can't reach it!");
            return 0;
        }
    }

    if (dfeature)
        sprintf(fbuf, "There is %s here.", an(dfeature));

    if (!otmp || is_lava(u.ux, u.uy) || (is_pool(u.ux, u.uy) && !Underwater)) {
        if (dfeature)
            plines(fbuf);
        read_engr_at(u.ux, u.uy); /* Eric Backus */
        if (!skip_objects && (Blind() || !dfeature))
            You("%s no objects here.", verb);
        return Blind();
    }
    /* we know there is something here */

    if (skip_objects) {
        if (dfeature)
            plines(fbuf);
        read_engr_at(u.ux, u.uy); /* Eric Backus */
        There("are %s objects here.", (obj_cnt <= 10) ? "several" : "many");
    } else if (!otmp->nexthere) {
        /* only one object */
        if (dfeature)
            plines(fbuf);
        read_engr_at(u.ux, u.uy); /* Eric Backus */
        You("%s here %s.", verb, doname(otmp));
        if (otmp->otyp == CORPSE)
            feel_cockatrice(otmp, false);
    } else {
        display_nhwindow(WIN_MESSAGE, false);
        tmpwin = create_nhwindow(NHW_MENU);
        if (dfeature) {
            putstr(tmpwin, 0, fbuf);
            putstr(tmpwin, 0, "");
        }
        putstr(tmpwin, 0, Blind() ? "Things that you feel here:" : "Things that are here:");
        for (; otmp; otmp = otmp->nexthere) {
            if (otmp->otyp == CORPSE && will_feel_cockatrice(otmp, false)) {
                char buf[BUFSZ];
                felt_cockatrice = true;
                strcpy(buf, doname(otmp));
                strcat(buf, "...");
                putstr(tmpwin, 0, buf);
                break;
            }
            putstr(tmpwin, 0, doname(otmp));
        }
        display_nhwindow(tmpwin, true);
        destroy_nhwindow(tmpwin);
        if (felt_cockatrice)
            feel_cockatrice(otmp, false);
        read_engr_at(u.ux, u.uy); /* Eric Backus */
    }
    return Blind();
}

/* explicilty look at what is here, including all objects */
int dolook(void) {
    return look_here(0);
}

bool will_feel_cockatrice (struct obj *otmp, bool force_touch) {
        if ((Blind() || force_touch) && !uarmg && !Stone_resistance() &&
                (otmp->otyp == CORPSE && touch_petrifies(&mons[otmp->corpsenm])))
                        return true;
        return false;
}

void feel_cockatrice (struct obj *otmp, bool force_touch) {
        char kbuf[BUFSZ];

        if (will_feel_cockatrice(otmp, force_touch)) {
            if(poly_when_stoned(youmonst.data))
                        You("touched the %s corpse with your bare %s.",
                                mons[otmp->corpsenm].mname, makeplural(body_part(HAND)));
            else
                        pline("Touching the %s corpse is a fatal mistake...",
                                mons[otmp->corpsenm].mname);
                sprintf(kbuf, "%s corpse", an(mons[otmp->corpsenm].mname));
                instapetrify(kbuf);
        }
}

void stackobj (struct obj *obj) {
        struct obj *otmp;

        for(otmp = level.objects[obj->ox][obj->oy]; otmp; otmp = otmp->nexthere)
                if(otmp != obj && merged(&obj,&otmp))
                        break;
        return;
}

int doprgold (void) {
        /* the messages used to refer to "carrying gold", but that didn't
           take containers into account */
        if(!u.ugold)
            Your("wallet is empty.");
        else
            Your("wallet contains %ld gold piece%s.", u.ugold, plur(u.ugold));
        shopper_financial_report();
        return 0;
}

int doprwep (void) {
    if (!uwep) {
        You("are empty %s.", body_part(HANDED));
    } else {
        prinv((char *)0, uwep, 0L);
        if (u.twoweap) prinv((char *)0, uswapwep, 0L);
    }
    return 0;
}

int doprarm (void) {
        if(!wearing_armor())
                You("are not wearing any armor.");
        else {
                char lets[8];
                int ct = 0;

                if(uarmu) lets[ct++] = obj_to_let(uarmu);
                if(uarm) lets[ct++] = obj_to_let(uarm);
                if(uarmc) lets[ct++] = obj_to_let(uarmc);
                if(uarmh) lets[ct++] = obj_to_let(uarmh);
                if(uarms) lets[ct++] = obj_to_let(uarms);
                if(uarmg) lets[ct++] = obj_to_let(uarmg);
                if(uarmf) lets[ct++] = obj_to_let(uarmf);
                lets[ct] = 0;
                (void) display_inventory(lets, false);
        }
        return 0;
}

int doprring (void) {
        if(!uleft && !uright)
                You("are not wearing any rings.");
        else {
                char lets[3];
                int ct = 0;

                if(uleft) lets[ct++] = obj_to_let(uleft);
                if(uright) lets[ct++] = obj_to_let(uright);
                lets[ct] = 0;
                (void) display_inventory(lets, false);
        }
        return 0;
}

int dopramulet (void) {
        if (!uamul)
                You("are not wearing an amulet.");
        else
                prinv((char *)0, uamul, 0L);
        return 0;
}

static bool tool_in_use (struct obj *obj) {
        if ((obj->owornmask & (W_TOOL
                        | W_SADDLE
                        )) != 0L) return true;
        if (obj->oclass != TOOL_CLASS) return false;
        return (bool)(obj == uwep || obj->lamplit ||
                                (obj->otyp == LEASH && obj->leashmon));
}

int doprtool (void) {
        struct obj *otmp;
        int ct = 0;
        char lets[52+1];

        for (otmp = invent; otmp; otmp = otmp->nobj)
            if (tool_in_use(otmp))
                lets[ct++] = obj_to_let(otmp);
        lets[ct] = '\0';
        if (!ct) You("are not using any tools.");
        else (void) display_inventory(lets, false);
        return 0;
}

/* '*' command; combines the ')' + '[' + '=' + '"' + '(' commands;
   show inventory of all currently wielded, worn, or used objects */
int doprinuse (void) {
        struct obj *otmp;
        int ct = 0;
        char lets[52+1];

        for (otmp = invent; otmp; otmp = otmp->nobj)
            if (is_worn(otmp) || tool_in_use(otmp))
                lets[ct++] = obj_to_let(otmp);
        lets[ct] = '\0';
        if (!ct) You("are not wearing or wielding anything.");
        else (void) display_inventory(lets, false);
        return 0;
}

/*
 * uses up an object that's on the floor, charging for it as necessary
 */
void useupf (struct obj *obj, long numused) {
        struct obj *otmp;
        bool at_u = (obj->ox == u.ux && obj->oy == u.uy);

        /* burn_floor_paper() keeps an object pointer that it tries to
         * useupf() multiple times, so obj must survive if plural */
        if (obj->quan > numused)
                otmp = splitobj(obj, numused);
        else
                otmp = obj;
        if(costly_spot(otmp->ox, otmp->oy)) {
            if(index(u.urooms, *in_rooms(otmp->ox, otmp->oy, 0)))
                addtobill(otmp, false, false, false);
            else (void)stolen_value(otmp, otmp->ox, otmp->oy, false, false);
        }
        delobj(otmp);
        if (at_u && u.uundetected && hides_under(youmonst.data))
            u.uundetected = OBJ_AT(u.ux, u.uy);
}



char * let_to_name (char let, bool unpaid) {
        const char *class_name;
        const char *pos;
        int oclass = (let >= 1 && let < MAXOCLASSES) ? let : 0;
        unsigned len;

        if (oclass)
            class_name = names[oclass];
        else if ((pos = index(oth_symbols, let)) != 0)
            class_name = oth_names[pos - oth_symbols];
        else
            class_name = names[0];

        len = strlen(class_name) + (unpaid ? sizeof "unpaid_" : sizeof "");
        if (len > invbufsiz) {
            if (invbuf) free((void *)invbuf);
            invbufsiz = len + 10; /* add slop to reduce incremental realloc */
            invbuf = (char *) malloc(invbufsiz);
        }
        if (unpaid)
            strcat(strcpy(invbuf, "Unpaid "), class_name);
        else
            strcpy(invbuf, class_name);
        return invbuf;
}

void free_invbuf (void) {
        if (invbuf) free((void *)invbuf),  invbuf = (char *)0;
        invbufsiz = 0;
}


void reassign (void) {
        int i;
        struct obj *obj;

        for(obj = invent, i = 0; obj; obj = obj->nobj, i++)
                obj->invlet = (i < 26) ? ('a'+i) : ('A'+i-26);
        lastinvnr = i;
}

/* inventory organizer by Del Lamb */
int doorganize (void) {
        struct obj *obj, *otmp;
        int ix, cur;
        char let;
        char alphabet[52+1], buf[52+1];
        char qbuf[QBUFSZ];
        char allowall[2];
        const char *adj_type;

        if (!flags.invlet_constant) reassign();
        /* get a pointer to the object the user wants to organize */
        allowall[0] = ALL_CLASSES; allowall[1] = '\0';
        if (!(obj = getobj(allowall,"adjust"))) return(0);

        /* initialize the list with all upper and lower case letters */
        for (let = 'a', ix = 0;  let <= 'z';) alphabet[ix++] = let++;
        for (let = 'A', ix = 26; let <= 'Z';) alphabet[ix++] = let++;
        alphabet[52] = 0;

        /* blank out all the letters currently in use in the inventory */
        /* except those that will be merged with the selected object   */
        for (otmp = invent; otmp; otmp = otmp->nobj)
                if (otmp != obj && !mergable(otmp,obj)) {
                        if (otmp->invlet <= 'Z')
                                alphabet[(otmp->invlet) - 'A' + 26] = ' ';
                        else    alphabet[(otmp->invlet) - 'a']      = ' ';
                }

        /* compact the list by removing all the blanks */
        for (ix = cur = 0; ix <= 52; ix++)
                if (alphabet[ix] != ' ') buf[cur++] = alphabet[ix];

        /* and by dashing runs of letters */
        if(cur > 5) compactify(buf);

        /* get new letter to use as inventory letter */
        for (;;) {
                sprintf(qbuf, "Adjust letter to what [%s]?",buf);
                let = yn_function(qbuf, (char *)0, '\0');
                if(index(quitchars,let)) {
                        plines(Never_mind);
                        return(0);
                }
                if (let == '@' || !letter(let))
                        pline("Select an inventory slot letter.");
                else
                        break;
        }

        /* change the inventory and print the resulting item */
        adj_type = "Moving:";

        /*
         * don't use freeinv/addinv to avoid double-touching artifacts,
         * dousing lamps, losing luck, cursing loadstone, etc.
         */
        extract_nobj(obj, &invent);

        for (otmp = invent; otmp;)
                if (merged(&otmp,&obj)) {
                        adj_type = "Merging:";
                        obj = otmp;
                        otmp = otmp->nobj;
                        extract_nobj(obj, &invent);
                } else {
                        if (otmp->invlet == let) {
                                adj_type = "Swapping:";
                                otmp->invlet = obj->invlet;
                        }
                        otmp = otmp->nobj;
                }

        /* inline addinv (assuming flags.invlet_constant and !merged) */
        obj->invlet = let;
        obj->nobj = invent; /* insert at beginning */
        obj->where = OBJ_INVENT;
        invent = obj;
        reorder_invent();

        prinv(adj_type, obj, 0L);
        return(0);
}

/* common to display_minventory and display_cinventory */
static void invdisp_nothing (const char *hdr, const char *txt) {
        winid win;
        anything any;
        menu_item *selected;

        any.a_void = 0;
        win = create_nhwindow(NHW_MENU);
        start_menu(win);
        add_menu(win, NO_GLYPH, &any, 0, 0, iflags.menu_headings, hdr, MENU_UNSELECTED);
        add_menu(win, NO_GLYPH, &any, 0, 0, ATR_NONE, "", MENU_UNSELECTED);
        add_menu(win, NO_GLYPH, &any, 0, 0, ATR_NONE, txt, MENU_UNSELECTED);
        end_menu(win, (char *)0);
        if (select_menu(win, PICK_NONE, &selected) > 0)
            free((void *)selected);
        destroy_nhwindow(win);
        return;
}

/* query_objlist callback: return things that could possibly be worn/wielded */
static bool worn_wield_only (const struct obj *obj) {
    return (obj->oclass == WEAPON_CLASS
                || obj->oclass == ARMOR_CLASS
                || obj->oclass == AMULET_CLASS
                || obj->oclass == RING_CLASS
                || obj->oclass == TOOL_CLASS);
}

/*
 * Display a monster's inventory.
 * Returns a pointer to the object from the monster's inventory selected
 * or NULL if nothing was selected.
 *
 * By default, only worn and wielded items are displayed.  The caller
 * can pick one.  Modifier flags are:
 *
 *      MINV_NOLET      - nothing selectable
 *      MINV_ALL        - display all inventory
 */
struct obj * display_minventory (struct monst *mon, int dflags, char *title) {
    struct obj *ret;
    struct obj m_gold;
    char tmp[QBUFSZ];
    int n;
    menu_item *selected = 0;
    int do_all = (dflags & MINV_ALL) != 0,
        do_gold = (do_all && mon->mgold);

    char name[BUFSZ];
    noit_Monnam(name, BUFSZ, mon);
    nh_slprintf(tmp, QBUFSZ, "%s%s %s:", name, possessive_suffix(name),
            do_all ? "possessions" : "armament");

    if (do_all ? (mon->minvent || mon->mgold)
            : (mon->misc_worn_check || MON_WEP(mon))) {
        /* Fool the 'weapon in hand' routine into
         * displaying 'weapon in claw', etc. properly.
         */
        youmonst.data = mon->data;

        if (do_gold) {
            /*
             * Make temporary gold object and insert at the head of
             * the mon's inventory.  We can get away with using a
             * stack variable object because monsters don't carry
             * gold in their inventory, so it won't merge.
             */
            m_gold = zeroobj;
            m_gold.otyp = GOLD_PIECE;  m_gold.oclass = COIN_CLASS;
            m_gold.quan = mon->mgold;  m_gold.dknown = 1;
            m_gold.where = OBJ_FREE;
            /* we had better not merge and free this object... */
            if (add_to_minv(mon, &m_gold))
                panic("display_minventory: static object freed.");
        }

        n = query_objlist(title ? title : tmp, mon->minvent, INVORDER_SORT, &selected,
                (dflags & MINV_NOLET) ? PICK_NONE : PICK_ONE,
                do_all ? allow_all : worn_wield_only);

        if (do_gold) obj_extract_self(&m_gold);

        set_uasmon();
    } else {
        invdisp_nothing(title ? title : tmp, "(none)");
        n = 0;
    }

    if (n > 0) {
        ret = selected[0].item.a_obj;
        free((void *)selected);
        /*
         * Unfortunately, we can't return a pointer to our temporary
         * gold object.  We'll have to work out a scheme where this
         * can happen.  Maybe even put gold in the inventory list...
         */
        if (ret == &m_gold) ret = (struct obj *) 0;
    } else {
        ret = (struct obj *) 0;
    }
    return ret;
}

/*
 * Display the contents of a container in inventory style.
 * Currently, this is only used for statues, via wand of probing.
 */
struct obj * display_cinventory (struct obj *obj) {
        struct obj *ret;
        char tmp[QBUFSZ];
        int n;
        menu_item *selected = 0;

        sprintf(tmp,"Contents of %s:", doname(obj));

        if (obj->cobj) {
            n = query_objlist(tmp, obj->cobj, INVORDER_SORT, &selected,
                            PICK_NONE, allow_all);
        } else {
            invdisp_nothing(tmp, "(empty)");
            n = 0;
        }
        if (n > 0) {
            ret = selected[0].item.a_obj;
            free((void *)selected);
        } else
            ret = (struct obj *) 0;
        return ret;
}

static bool only_here (const struct obj *obj) {
    return (obj->ox == only.x && obj->oy == only.y);
}

/*
 * Display a list of buried items in inventory style.  Return a non-zero
 * value if there were items at that spot.
 *
 * Currently, this is only used with a wand of probing zapped downwards.
 */
int display_binventory (int x, int y, bool as_if_seen) {
        struct obj *obj;
        menu_item *selected = 0;
        int n;

        /* count # of objects here */
        for (n = 0, obj = level.buriedobjlist; obj; obj = obj->nobj)
            if (obj->ox == x && obj->oy == y) {
                if (as_if_seen) obj->dknown = 1;
                n++;
            }

        if (n) {
            only.x = x;
            only.y = y;
            if (query_objlist("Things that are buried here:",
                              level.buriedobjlist, INVORDER_SORT,
                              &selected, PICK_NONE, only_here) > 0)
            {
                free(selected);
            }
            only.x = only.y = 0;
        }
        return n;
}

int dopickup(void) {
    struct trap *traphere = t_at(u.ux, u.uy);
    if (u.uswallow) {
        if (!u.ustuck->minvent) {
            if (is_animal(u.ustuck->data)) {
                char name[BUFSZ];
                mon_nam(name, BUFSZ, u.ustuck);
                You("pick up %s%s tongue.", name, possessive_suffix(name));
                pline("But it's kind of slimy, so you drop it.");
            } else {
                You("don't %s anything in here to pick up.", Blind() ? "feel" : "see");
            }
            return 1;
        } else {
            /* 3.4.0 introduced the ability to pick things up from within swallower's stomach */
            return pickup();
        }
    }
    if (is_pool(u.ux, u.uy)) {
        if (Wwalking || is_floater(youmonst.data) || is_clinger(youmonst.data) || (Flying && !Breathless)) {
            You("cannot dive into the water to pick things up.");
            return (0);
        } else if (!Underwater) {
            You_cant("even see the bottom, let alone pick up %s.", something);
            return (0);
        }
    }
    if (is_lava(u.ux, u.uy)) {
        if (Wwalking || is_floater(youmonst.data) || is_clinger(youmonst.data) || (Flying && !Breathless)) {
            You_cant("reach the bottom to pick things up.");
            return (0);
        } else if (!likes_lava(youmonst.data)) {
            You("would burn to a crisp trying to pick things up.");
            return (0);
        }
    }
    if (!OBJ_AT(u.ux, u.uy)) {
        There("is nothing here to pick up.");
        return (0);
    }
    if (!can_reach_floor()) {
        if (u.usteed && P_SKILL(P_RIDING) < P_BASIC) {
            char name[BUFSZ];
            y_monnam(name, BUFSZ, u.usteed);
            You("aren't skilled enough to reach from %s.", name);
        } else {
            You("cannot reach the %s.", surface(u.ux, u.uy));
        }
        return 0;
    }

    if (traphere && traphere->tseen) {
        /* Allow pickup from holes and trap doors that you escaped from
         * because that stuff is teetering on the edge just like you, but
         * not pits, because there is an elevation discrepancy with stuff
         * in pits.
         */
        if ((traphere->ttyp == PIT || traphere->ttyp == SPIKED_PIT) && (!u.utrap || (u.utrap && u.utraptype != TT_PIT))) {
            You("cannot reach the bottom of the pit.");
            return (0);
        }
    }

    return pickup();
}

