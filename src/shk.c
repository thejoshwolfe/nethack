/* See LICENSE in the root of this project for change info */

#include "shk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rm_util.h"
#include "rm_util.h"
#include "display_util.h"
#include "move.h"
#include "dungeon_util.h"
#include "align.h"
#include "apply.h"
#include "artifact.h"
#include "attrib.h"
#include "ball.h"
#include "cmd.h"
#include "coord.h"
#include "decl.h"
#include "dig.h"
#include "display.h"
#include "do_name.h"
#include "dog.h"
#include "dungeon.h"
#include "eat.h"
#include "end.h"
#include "eshk.h"
#include "flag.h"
#include "global.h"
#include "hack.h"
#include "hacklib.h"
#include "invent.h"
#include "lock.h"
#include "makemon.h"
#include "mhitu.h"
#include "mkobj.h"
#include "mkroom.h"
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
#include "pline.h"
#include "pm.h"
#include "priest.h"
#include "prop.h"
#include "rm.h"
#include "rnd.h"
#include "role.h"
#include "shknam.h"
#include "spell.h"
#include "steal.h"
#include "teleport.h"
#include "timeout.h"
#include "trap.h"
#include "util.h"
#include "vault.h"
#include "vision.h"
#include "wintype.h"
#include "worn.h"
#include "you.h"
#include "youprop.h"
#include "zap.h"

enum {
    PAY_SOME  =   2,
    PAY_BUY   =   1,
    PAY_CANT  =   0,  /* too poor */
    PAY_SKIP  =  -1,
    PAY_BROKE =  -2,
};

extern const struct shclass shtypes[];  /* defined in shknam.c */
extern struct obj *thrownobj;           /* defined in dothrow.c */

static long int followmsg;  /* last time of follow message */

/*
invariants: obj->unpaid iff onbill(obj) [unless bp->useup]
obj->quan <= bp->bquan
*/

static const char no_money[] = "Moreover, you%s have no money.";
static const char not_enough_money[] =
"Besides, you don't have enough to interest %s.";

static coord repo_location;     /* repossession context */

/* auto-response flag for/from "sell foo?" 'a' => 'y', 'q' => 'n' */
static char sell_response = 'a';
static int sell_how = SELL_NORMAL;
/* can't just use sell_response='y' for auto_credit because the 'a' response
   shouldn't carry over from ordinary selling to credit selling */
static bool auto_credit = false;

enum {
    HUNGRY = 2,
};

/* First 4 supplied by Ronen and Tamar, remainder by development team */
const char *Izchak_speaks[]={
    "%s says: 'These shopping malls give me a headache.'",
    "%s says: 'Slow down.  Think clearly.'",
    "%s says: 'You need to take things one at a time.'",
    "%s says: 'I don't like poofy coffee... give me Columbian Supremo.'",
    "%s says that getting the devteam's agreement on anything is difficult.",
    "%s says that he has noticed those who serve their deity will prosper.",
    "%s says: 'Don't try to steal from me - I have friends in high places!'",
    "%s says: 'You may well need something from this shop in the future.'",
    "%s comments about the Valley of the Dead as being a gateway."
};

static const int NEED_UPDATE = 1;
static const int OPEN = 2;
static const int INSHOP = 4;

static int horiz(int i) {
    return (i % 3) - 1;
}

static int vert(int i) {
    return (i / 3) - 1;
}

/* remove previously applied surcharge from all billed items */
static void pacify_shk (struct monst *shkp) {
    NOTANGRY(shkp) = true;  /* make peaceful */
    if (ESHK(shkp)->surcharge) {
        struct bill_x *bp = ESHK(shkp)->bill_p;
        int ct = ESHK(shkp)->billct;

        ESHK(shkp)->surcharge = false;
        while (ct-- > 0) {
            long reduction = (bp->price + 3L) / 4L;
            bp->price -= reduction;         /* undo 33% increase */
            bp++;
        }
    }
}

/* add aggravation surcharge to all billed items */
static void rile_shk (struct monst *shkp) {
    NOTANGRY(shkp) = false; /* make angry */
    if (!ESHK(shkp)->surcharge) {
        struct bill_x *bp = ESHK(shkp)->bill_p;
        int ct = ESHK(shkp)->billct;

        ESHK(shkp)->surcharge = true;
        while (ct-- > 0) {
            long surcharge = (bp->price + 2L) / 3L;
            bp->price += surcharge;
            bp++;
        }
    }
}


static struct monst * next_shkp(struct monst *shkp, bool withbill) {
    for (; shkp; shkp = shkp->nmon) {
        if (DEADMONSTER(shkp)) continue;
        if (shkp->isshk && (ESHK(shkp)->billct || !withbill)) break;
    }

    if (shkp) {
        if (NOTANGRY(shkp)) {
            if (ESHK(shkp)->surcharge) pacify_shk(shkp);
        } else {
            if (!ESHK(shkp)->surcharge) rile_shk(shkp);
        }
    }
    return(shkp);
}

/* called in do_name.c */
const char *shkname(const struct monst *mtmp) {
    return(ESHK(mtmp)->shknam);
}

/*
 * Do something about damage. Either (!croaked) try to repair it, or
 * (croaked) just discard damage structs for non-shared locations, since
 * they'll never get repaired. Assume that shared locations will get
 * repaired eventually by the other shopkeeper(s). This might be an erroneous
 * assumption (they might all be dead too), but we have no reasonable way of
 * telling that.
 */
static void remove_damage(struct monst *shkp, bool croaked) {
    struct damage *tmp_dam, *tmp2_dam;
    bool did_repair = false, saw_door = false;
    bool saw_floor = false, stop_picking = false;
    bool saw_untrap = false;
    unsigned char saw_walls = 0;

    tmp_dam = level.damagelist;
    tmp2_dam = 0;
    while (tmp_dam) {
        signed char x = tmp_dam->place.x, y = tmp_dam->place.y;
        char shops[5];
        int disposition;

        disposition = 0;
        strcpy(shops, in_rooms(x, y, SHOPBASE));
        if (index(shops, ESHK(shkp)->shoproom)) {
            if (croaked)
                disposition = (shops[1])? 0 : 1;
            else if (stop_picking)
                disposition = repair_damage(shkp, tmp_dam, false);
            else {
                /* Defer the stop_occupation() until after repair msgs */
                if (closed_door(x, y))
                    stop_picking = picking_at(x, y);
                disposition = repair_damage(shkp, tmp_dam, false);
                if (!disposition)
                    stop_picking = false;
            }
        }

        if (!disposition) {
            tmp2_dam = tmp_dam;
            tmp_dam = tmp_dam->next;
            continue;
        }

        if (disposition > 1) {
            did_repair = true;
            if (cansee(x, y)) {
                if (IS_WALL(levl[x][y].typ))
                    saw_walls++;
                else if (IS_DOOR(levl[x][y].typ))
                    saw_door = true;
                else if (disposition == 3)          /* untrapped */
                    saw_untrap = true;
                else
                    saw_floor = true;
            }
        }

        tmp_dam = tmp_dam->next;
        if (!tmp2_dam) {
            free((void *)level.damagelist);
            level.damagelist = tmp_dam;
        } else {
            free((void *)tmp2_dam->next);
            tmp2_dam->next = tmp_dam;
        }
    }
    if (!did_repair)
        return;
    if (saw_walls) {
        pline("Suddenly, %s section%s of wall close%s up!",
                (saw_walls == 1) ? "a" : (saw_walls <= 3) ?
                "some" : "several",
                (saw_walls == 1) ? "" : "s", (saw_walls == 1) ? "s" : "");
        if (saw_door)
            pline_The("shop door reappears!");
        if (saw_floor)
            pline_The("floor is repaired!");
    } else {
        if (saw_door)
            pline("Suddenly, the shop door reappears!");
        else if (saw_floor)
            pline("Suddenly, the floor damage is gone!");
        else if (saw_untrap)
            pline("Suddenly, the trap is removed from the floor!");
        else if (inside_shop(u.ux, u.uy) == ESHK(shkp)->shoproom)
            You_feel("more claustrophobic than before.");
        else if (flags.soundok && !rn2(10))
            Norep("The dungeon acoustics noticeably change.");
    }
    if (stop_picking)
        stop_occupation();
}

/* Clear the unpaid bit on all of the objects in the list. */
static void clear_unpaid (struct obj *list) {
    while (list) {
        if (Has_contents(list)) clear_unpaid(list->cobj);
        list->unpaid = 0;
        list = list->nobj;
    }
}


/* either you paid or left the shop or the shopkeeper died */
static void setpaid (struct monst *shkp) {
    struct obj *obj;
    struct monst *mtmp;

    /* FIXME: object handling should be limited to
       items which are on this particular shk's bill */

    clear_unpaid(invent);
    clear_unpaid(fobj);
    clear_unpaid(level.buriedobjlist);
    if (thrownobj) thrownobj->unpaid = 0;
    for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
        clear_unpaid(mtmp->minvent);
    for(mtmp = migrating_mons; mtmp; mtmp = mtmp->nmon)
        clear_unpaid(mtmp->minvent);

    while ((obj = billobjs) != 0) {
        obj_extract_self(obj);
        dealloc_obj(obj);
    }
    if(shkp) {
        ESHK(shkp)->billct = 0;
        ESHK(shkp)->credit = 0L;
        ESHK(shkp)->debit = 0L;
        ESHK(shkp)->loan = 0L;
    }
}


/* called in mon.c */
void shkgone ( struct monst *mtmp) {
    struct eshk *eshk = ESHK(mtmp);
    struct mkroom *sroom = &rooms[eshk->shoproom - ROOMOFFSET];
    struct obj *otmp;
    char *p;
    int sx, sy;

    /* [BUG: some of this should be done on the shop level */
    /*       even when the shk dies on a different level.] */
    if (on_level(&eshk->shoplevel, &u.uz)) {
        remove_damage(mtmp, true);
        sroom->resident = (struct monst *)0;
        if (!search_special(ANY_SHOP))
            level.flags.has_shop = 0;

        /* items on shop floor revert to ordinary objects */
        for (sx = sroom->lx; sx <= sroom->hx; sx++)
            for (sy = sroom->ly; sy <= sroom->hy; sy++)
                for (otmp = level.objects[sx][sy]; otmp; otmp = otmp->nexthere)
                    otmp->no_charge = 0;

        /* Make sure bill is set only when the
           dead shk is the resident shk. */
        if ((p = index(u.ushops, eshk->shoproom)) != 0) {
            setpaid(mtmp);
            eshk->bill_p = (struct bill_x *)0;
            /* remove eshk->shoproom from u.ushops */
            do { *p = *(p + 1); } while (*++p);
        }
    }
}

void set_residency(struct monst *shkp, bool zero_out) {
    if (on_level(&(ESHK(shkp)->shoplevel), &u.uz))
        rooms[ESHK(shkp)->shoproom - ROOMOFFSET].resident =
            (zero_out)? (struct monst *)0 : shkp;
}

void replshk (struct monst *mtmp, struct monst *mtmp2) {
    rooms[ESHK(mtmp2)->shoproom - ROOMOFFSET].resident = mtmp2;
    if (inhishop(mtmp) && *u.ushops == ESHK(mtmp)->shoproom) {
        ESHK(mtmp2)->bill_p = &(ESHK(mtmp2)->bill[0]);
    }
}

/* do shopkeeper specific structure munging -dlc */
void restshk(struct monst *shkp, bool ghostly) {
    if (u.uz.dlevel) {
        struct eshk *eshkp = ESHK(shkp);

        if (eshkp->bill_p != (struct bill_x *) -1000)
            eshkp->bill_p = &eshkp->bill[0];
        /* shoplevel can change as dungeons move around */
        /* savebones guarantees that non-homed shk's will be gone */
        if (ghostly) {
            assign_level(&eshkp->shoplevel, &u.uz);
            if (ANGRY(shkp) && strncmpi(eshkp->customer, plname, PL_NSIZ))
                pacify_shk(shkp);
        }
    }
}

static long addupbill (struct monst *shkp) {
    int ct = ESHK(shkp)->billct;
    struct bill_x *bp = ESHK(shkp)->bill_p;
    long total = 0L;

    while(ct--){
        total += bp->price * bp->bquan;
        bp++;
    }
    return(total);
}

static void makekops(coord *mm) {
    static const short k_mndx[4] = {
        PM_KEYSTONE_KOP, PM_KOP_SERGEANT, PM_KOP_LIEUTENANT, PM_KOP_KAPTAIN
    };
    int k_cnt[4], cnt, mndx, k;

    k_cnt[0] = cnt = abs(depth(&u.uz)) + rnd(5);
    k_cnt[1] = (cnt / 3) + 1;       /* at least one sarge */
    k_cnt[2] = (cnt / 6);           /* maybe a lieutenant */
    k_cnt[3] = (cnt / 9);           /* and maybe a kaptain */

    for (k = 0; k < 4; k++) {
        if ((cnt = k_cnt[k]) == 0) break;
        mndx = k_mndx[k];
        if (mvitals[mndx].mvflags & G_GONE) continue;

        while (cnt--)
            if (enexto(mm, mm->x, mm->y, &mons[mndx]))
                (void) makemon(&mons[mndx], mm->x, mm->y, NO_MM_FLAGS);
    }
}


static void call_kops(struct monst *shkp, bool nearshop) {
    /* Keystone Kops srt@ucla */
    bool nokops;

    if(!shkp) return;

    if (flags.soundok)
        pline("An alarm sounds!");

    nokops = ((mvitals[PM_KEYSTONE_KOP].mvflags & G_GONE) &&
            (mvitals[PM_KOP_SERGEANT].mvflags & G_GONE) &&
            (mvitals[PM_KOP_LIEUTENANT].mvflags & G_GONE) &&
            (mvitals[PM_KOP_KAPTAIN].mvflags & G_GONE));

    if(!angry_guards(!flags.soundok) && nokops) {
        if(flags.verbose && flags.soundok)
            pline("But no one seems to respond to it.");
        return;
    }

    if(nokops) return;

    {
        coord mm;

        if (nearshop) {
            /* Create swarm around you, if you merely "stepped out" */
            if (flags.verbose)
                pline_The("Keystone Kops appear!");
            mm.x = u.ux;
            mm.y = u.uy;
            makekops(&mm);
            return;
        }
        if (flags.verbose)
            pline_The("Keystone Kops are after you!");
        /* Create swarm near down staircase (hinders return to level) */
        mm.x = xdnstair;
        mm.y = ydnstair;
        makekops(&mm);
        /* Create swarm near shopkeeper (hinders return to shop) */
        mm.x = shkp->mx;
        mm.y = shkp->my;
        makekops(&mm);
    }
}

/* x,y is strictly inside shop */
char inside_shop (signed char x, signed char y) {
    char rno;

    rno = levl[x][y].roomno;
    if ((rno < ROOMOFFSET) || levl[x][y].edge || !IS_SHOP(rno-ROOMOFFSET))
        return(NO_ROOM);
    else
        return(rno);
}

/* wakeup and/or unparalyze shopkeeper */
static void rouse_shk(struct monst *shkp, bool verbosely) {
    if (!shkp->mcanmove || shkp->msleeping) {
        /* greed induced recovery... */
        if (verbosely && canspotmon(shkp)) {
            message_monster(shkp->msleeping ? MSG_M_WAKES_UP : MSG_M_CAN_MOVE_AGAIN, shkp);
        }
        shkp->msleeping = 0;
        shkp->mfrozen = 0;
        shkp->mcanmove = 1;
    }
}


/* shop merchandise has been taken; pay for it with any credit available;
   return false if the debt is fully covered by credit, true otherwise */
static bool rob_shop(struct monst *shkp) {
    struct eshk *eshkp;
    long total;

    eshkp = ESHK(shkp);
    rouse_shk(shkp, true);
    total = (addupbill(shkp) + eshkp->debit);
    if (eshkp->credit >= total) {
        Your("credit of %ld %s is used to cover your shopping bill.",
                eshkp->credit, currency(eshkp->credit));
        total = 0L;         /* credit gets cleared by setpaid() */
    } else {
        You("escaped the shop without paying!");
        total -= eshkp->credit;
    }
    setpaid(shkp);
    if (!total) return false;

    /* by this point, we know an actual robbery has taken place */
    eshkp->robbed += total;
    You("stole %ld %s worth of merchandise.",
            total, currency(total));
    if (!Role_if(PM_ROGUE)) /* stealing is unlawful */
        adjalign(-sgn(u.ualign.type));

    hot_pursuit(shkp);
    return true;
}


void u_left_shop(char *leavestring, bool newlev) {
    struct monst *shkp;
    struct eshk *eshkp;

    /*
     * IF player
     * ((didn't leave outright) AND
     *  ((he is now strictly-inside the shop) OR
     *   (he wasn't strictly-inside last turn anyway)))
     * THEN (there's nothing to do, so just return)
     */
    if(!*leavestring &&
            (!levl[u.ux][u.uy].edge || levl[u.ux0][u.uy0].edge))
        return;

    shkp = shop_keeper(*u.ushops0);
    if (!shkp || !inhishop(shkp))
        return;     /* shk died, teleported, changed levels... */

    eshkp = ESHK(shkp);
    if (!eshkp->billct && !eshkp->debit)    /* bill is settled */
        return;

    if (!*leavestring && shkp->mcanmove && !shkp->msleeping) {
        /*
         * Player just stepped onto shop-boundary (known from above logic).
         * Try to intimidate him into paying his bill
         */
        verbalize(NOTANGRY(shkp) ?
                "%s!  Please pay before leaving." :
                "%s!  Don't you leave without paying!",
                plname);
        return;
    }

    if (rob_shop(shkp)) {
        call_kops(shkp, (!newlev && levl[u.ux0][u.uy0].edge));
    }
}

/* robbery from outside the shop via telekinesis or grappling hook */
void remote_burglary (signed char x, signed char y) {
    struct monst *shkp;
    struct eshk *eshkp;

    shkp = shop_keeper(*in_rooms(x, y, SHOPBASE));
    if (!shkp || !inhishop(shkp))
        return;     /* shk died, teleported, changed levels... */

    eshkp = ESHK(shkp);
    if (!eshkp->billct && !eshkp->debit)    /* bill is settled */
        return;

    if (rob_shop(shkp)) {
        /*[might want to set 2nd arg based on distance from shop doorway]*/
        call_kops(shkp, false);
    }
}

void u_entered_shop (char *enterstring) {

    int rt;
    struct monst *shkp;
    struct eshk *eshkp;
    static const char no_shk[] = "This shop appears to be deserted.";
    static char empty_shops[5];

    if(!*enterstring)
        return;

    if(!(shkp = shop_keeper(*enterstring))) {
        if (!index(empty_shops, *enterstring) &&
                in_rooms(u.ux, u.uy, SHOPBASE) !=
                in_rooms(u.ux0, u.uy0, SHOPBASE))
            plines(no_shk);
        strcpy(empty_shops, u.ushops);
        u.ushops[0] = '\0';
        return;
    }

    eshkp = ESHK(shkp);

    if (!inhishop(shkp)) {
        /* dump core when referenced */
        eshkp->bill_p = (struct bill_x *) -1000;
        if (!index(empty_shops, *enterstring))
            plines(no_shk);
        strcpy(empty_shops, u.ushops);
        u.ushops[0] = '\0';
        return;
    }

    eshkp->bill_p = &(eshkp->bill[0]);

    if ((!eshkp->visitct || *eshkp->customer) &&
            strncmpi(eshkp->customer, plname, PL_NSIZ)) {
        /* You seem to be new here */
        eshkp->visitct = 0;
        eshkp->following = 0;
        (void) strncpy(eshkp->customer,plname,PL_NSIZ);
        pacify_shk(shkp);
    }

    if (shkp->msleeping || !shkp->mcanmove || eshkp->following)
        return;     /* no dialog */

    if (Invis) {
        pline("%s senses your presence.", shkname(shkp));
        verbalize("Invisible customers are not welcome!");
        return;
    }

    rt = rooms[*enterstring - ROOMOFFSET].rtype;

    if (ANGRY(shkp)) {
        verbalize("So, %s, you dare return to %s %s?!",
                plname,
                "TODO: s_suffix(shkname(shkp))",
                shtypes[rt - SHOPBASE].name);
    } else if (eshkp->robbed) {
        pline("%s mutters imprecations against shoplifters.", shkname(shkp));
    } else {
        verbalize("%s, %s!  Welcome%s to %s %s!",
                Hello(shkp), plname,
                eshkp->visitct++ ? " again" : "",
                "TODO: s_suffix(shkname(shkp))",
                shtypes[rt - SHOPBASE].name);
    }
    /* can't do anything about blocking if teleported in */
    if (!inside_shop(u.ux, u.uy)) {
        bool should_block;
        int cnt;
        const char *tool;
        struct obj *pick = carrying(PICK_AXE),
                   *mattock = carrying(DWARVISH_MATTOCK);

        if (pick || mattock) {
            cnt = 1;        /* so far */
            if (pick && mattock) {  /* carrying both types */
                tool = "digging tool";
                cnt = 2;    /* `more than 1' is all that matters */
            } else if (pick) {
                tool = "pick-axe";
                /* hack: `pick' already points somewhere into inventory */
                while ((pick = pick->nobj) != 0)
                    if (pick->otyp == PICK_AXE) ++cnt;
            } else {        /* assert(mattock != 0) */
                tool = "mattock";
                while ((mattock = mattock->nobj) != 0)
                    if (mattock->otyp == DWARVISH_MATTOCK) ++cnt;
                /* [ALI] Shopkeeper identifies mattock(s) */
                if (!Blind()) makeknown(DWARVISH_MATTOCK);
            }
            verbalize(NOTANGRY(shkp) ?
                    "Will you please leave your %s%s outside?" :
                    "Leave the %s%s outside.",
                    tool, plur(cnt));
            should_block = true;
        } else if (u.usteed) {
            audio_message_monster(NOTANGRY(shkp) ?
                    MSG_PLEASE_LEAVE_M_OUTSIDE : MSG_LEAVE_M_OUTSIDE, u.usteed);
            should_block = true;
        } else {
            should_block = (Fast && (sobj_at(PICK_AXE, u.ux, u.uy) ||
                        sobj_at(DWARVISH_MATTOCK, u.ux, u.uy)));
        }
        if (should_block) (void) dochug(shkp);  /* shk gets extra move */
    }
    return;
}

static struct bill_x * onbill(const struct obj *obj, struct monst *shkp, bool silent) {
    if (shkp) {
        struct bill_x *bp = ESHK(shkp)->bill_p;
        int ct = ESHK(shkp)->billct;

        while (--ct >= 0)
            if (bp->bo_id == obj->o_id) {
                if (!obj->unpaid) pline("onbill: paid obj on bill?");
                return bp;
            } else bp++;
    }
    if(obj->unpaid & !silent) pline("onbill: unpaid obj not on bill?");
    return (struct bill_x *)0;
}

/*
   Decide whether two unpaid items are mergable; caller is responsible for
   making sure they're unpaid and the same type of object; we check the price
   quoted by the shopkeeper and also that they both belong to the same shk.
   */
bool same_price(struct obj *obj1, struct obj *obj2) {
    struct monst *shkp1, *shkp2;
    struct bill_x *bp1 = 0, *bp2 = 0;
    bool are_mergable = false;

    /* look up the first object by finding shk whose bill it's on */
    for (shkp1 = next_shkp(fmon, true); shkp1; shkp1 = next_shkp(shkp1->nmon, true)) {
        if ((bp1 = onbill(obj1, shkp1, true)) != 0)
            break;
    }
    /* second object is probably owned by same shk; if not, look harder */
    if (shkp1 && (bp2 = onbill(obj2, shkp1, true)) != 0) {
        shkp2 = shkp1;
    } else {
        for (shkp2 = next_shkp(fmon, true); shkp2;
                shkp2 = next_shkp(shkp2->nmon, true))
            if ((bp2 = onbill(obj2, shkp2, true)) != 0) break;
    }

    if (!bp1 || !bp2) impossible("same_price: object wasn't on any bill!");
    else are_mergable = (shkp1 == shkp2 && bp1->price == bp2->price);
    return are_mergable;
}

/*
 * Figure out how much is owed to a given shopkeeper.
 * At present, we ignore any amount robbed from the shop, to avoid
 * turning the `$' command into a way to discover that the current
 * level is bones data which has a shk on the warpath.
 */
static long shop_debt (struct eshk *eshkp) {
    struct bill_x *bp;
    int ct;
    long debt = eshkp->debit;

    for (bp = eshkp->bill_p, ct = eshkp->billct; ct > 0; bp++, ct--)
        debt += bp->price * bp->bquan;
    return debt;
}

/* called in response to the `$' command */
void shopper_financial_report (void) {
    struct monst *shkp, *this_shkp = shop_keeper(inside_shop(u.ux, u.uy));
    struct eshk *eshkp;
    long amt;
    int pass;

    if (this_shkp &&
            !(ESHK(this_shkp)->credit || shop_debt(ESHK(this_shkp)))) {
        You("have no credit or debt in here.");
        this_shkp = 0;      /* skip first pass */
    }

    /* pass 0: report for the shop we're currently in, if any;
       pass 1: report for all other shops on this level. */
    for (pass = this_shkp ? 0 : 1; pass <= 1; pass++)
        for (shkp = next_shkp(fmon, false);
                shkp; shkp = next_shkp(shkp->nmon, false)) {
            if ((shkp != this_shkp) ^ pass) continue;
            eshkp = ESHK(shkp);
            if ((amt = eshkp->credit) != 0) {
                message_monster_int_string(MSG_YOU_HAVE_X_CREDIT_AT_M_S, shkp,
                        amt, shtypes[eshkp->shoptype - SHOPBASE].name);
            } else if (shkp == this_shkp) {
                You("have no credit in here.");
            }

            if ((amt = shop_debt(eshkp)) != 0) {
                You("owe %s %ld %s.", shkname(shkp), amt, currency(amt));
            } else if (shkp == this_shkp) {
                You("don't owe any money here.");
            }
        }
}

int inhishop (struct monst *mtmp) {
    return(index(in_rooms(mtmp->mx, mtmp->my, SHOPBASE),
                ESHK(mtmp)->shoproom) &&
            on_level(&(ESHK(mtmp)->shoplevel), &u.uz));
}

struct monst * shop_keeper (char rmno) {
    struct monst *shkp = rmno >= ROOMOFFSET ?
        rooms[rmno - ROOMOFFSET].resident : 0;

    if (shkp) {
        if (NOTANGRY(shkp)) {
            if (ESHK(shkp)->surcharge) pacify_shk(shkp);
        } else {
            if (!ESHK(shkp)->surcharge) rile_shk(shkp);
        }
    }
    return shkp;
}

bool tended_shop(struct mkroom *sroom) {
    struct monst *mtmp = sroom->resident;

    if (!mtmp)
        return(false);
    else
        return((bool)(inhishop(mtmp)));
}

/* Delete the contents of the given object. */
void delete_contents (struct obj *obj) {
    struct obj *curr;

    while ((curr = obj->cobj) != 0) {
        obj_extract_self(curr);
        obfree(curr, (struct obj *)0);
    }
}

static void add_to_billobjs (struct obj *obj) {
    if (obj->where != OBJ_FREE)
        panic("add_to_billobjs: obj not free");
    if (obj->timed)
        obj_stop_timers(obj);

    obj->nobj = billobjs;
    billobjs = obj;
    obj->where = OBJ_ONBILL;
}


/* called with two args on merge */
void obfree (struct obj *obj, struct obj *merge) {
    struct bill_x *bp;
    struct bill_x *bpm;
    struct monst *shkp;

    if (obj->otyp == LEASH && obj->leashmon) o_unleash(obj);
    if (obj->oclass == FOOD_CLASS) food_disappears(obj);
    if (obj->oclass == SPBOOK_CLASS) book_disappears(obj);
    if (Has_contents(obj)) delete_contents(obj);

    shkp = 0;
    if (obj->unpaid) {
        /* look for a shopkeeper who owns this object */
        for (shkp = next_shkp(fmon, true); shkp;
                shkp = next_shkp(shkp->nmon, true))
            if (onbill(obj, shkp, true)) break;
    }
    /* sanity check, more or less */
    if (!shkp) shkp = shop_keeper(*u.ushops);
    /*
     * Note:  `shkp = shop_keeper(*u.ushops)' used to be
     *        unconditional.  But obfree() is used all over
     *        the place, so making its behavior be dependent
     *        upon player location doesn't make much sense.
     */

    if ((bp = onbill(obj, shkp, false)) != 0) {
        if(!merge){
            bp->useup = 1;
            obj->unpaid = 0;        /* only for doinvbill */
            add_to_billobjs(obj);
            return;
        }
        bpm = onbill(merge, shkp, false);
        if(!bpm){
            /* this used to be a rename */
            impossible("obfree: not on bill??");
            return;
        } else {
            /* this was a merger */
            bpm->bquan += bp->bquan;
            ESHK(shkp)->billct--;
            *bp = ESHK(shkp)->bill_p[ESHK(shkp)->billct];
        }
    }
    dealloc_obj(obj);
}

static long check_credit (long tmp, struct monst *shkp) {
    long credit = ESHK(shkp)->credit;

    if(credit == 0L) return(tmp);
    if(credit >= tmp) {
        pline_The("price is deducted from your credit.");
        ESHK(shkp)->credit -=tmp;
        tmp = 0L;
    } else {
        pline_The("price is partially covered by your credit.");
        ESHK(shkp)->credit = 0L;
        tmp -= credit;
    }
    return(tmp);
}

static void pay (long tmp, struct monst *shkp) {
    long robbed = ESHK(shkp)->robbed;
    long balance = ((tmp <= 0L) ? tmp : check_credit(tmp, shkp));

    u.ugold -= balance;
    shkp->mgold += balance;
    if(robbed) {
        robbed -= tmp;
        if(robbed < 0) robbed = 0L;
        ESHK(shkp)->robbed = robbed;
    }
}

static void kops_gone(bool silent) {
    int cnt = 0;
    struct monst *mtmp, *mtmp2;

    for (mtmp = fmon; mtmp; mtmp = mtmp2) {
        mtmp2 = mtmp->nmon;
        if (mtmp->data->mlet == S_KOP) {
            if (canspotmon(mtmp)) cnt++;
            mongone(mtmp);
        }
    }
    if (cnt && !silent)
        pline_The("Kop%s (disappointed) vanish%s into thin air.",
                plur(cnt), cnt == 1 ? "es" : "");
}


/* return shkp to home position */
void home_shk(struct monst *shkp, bool killkops) {
    signed char x = ESHK(shkp)->shk.x, y = ESHK(shkp)->shk.y;

    (void) mnearto(shkp, x, y, true);
    level.flags.has_shop = 1;
    if (killkops) {
        kops_gone(true);
        pacify_guards();
    }
    after_shk_move(shkp);
}

static bool angry_shk_exists(void) {
    struct monst *shkp;

    for (shkp = next_shkp(fmon, false);
            shkp; shkp = next_shkp(shkp->nmon, false))
        if (ANGRY(shkp)) return(true);
    return(false);
}

void make_happy_shk(struct monst *shkp, bool silentkops) {
    bool wasmad = ANGRY(shkp);
    struct eshk *eshkp = ESHK(shkp);

    pacify_shk(shkp);
    eshkp->following = 0;
    eshkp->robbed = 0L;
    if (!Role_if(PM_ROGUE))
        adjalign(sgn(u.ualign.type));
    if (!inhishop(shkp)) {
        char shk_nam[BUFSZ];
        mon_nam(shk_nam, BUFSZ, shkp);
        bool vanished = canseemon(shkp);
        if (on_level(&eshkp->shoplevel, &u.uz)) {
            home_shk(shkp, false);
            /* didn't disappear if shk can still be seen */
            if (canseemon(shkp)) vanished = false;
        } else {
            /* if sensed, does disappear regardless whether seen */
            if (sensemon(shkp)) vanished = true;
            /* can't act as porter for the Amulet, even if shk
               happens to be going farther down rather than up */
            mdrop_special_objs(shkp);
            /* arrive near shop's door */
            migrate_to_level(shkp, ledger_no(&eshkp->shoplevel),
                    MIGR_APPROX_XY, &eshkp->shd);
        }
        if (vanished)
            pline("Satisfied, %s suddenly disappears!", shk_nam);
    } else if(wasmad) {
        message_monster(MSG_M_CALMS_DOWN, shkp);
    }

    if(!angry_shk_exists()) {
        kops_gone(silentkops);
        pacify_guards();
    }
}

void hot_pursuit (struct monst *shkp) {
    if(!shkp->isshk) return;

    rile_shk(shkp);
    (void) strncpy(ESHK(shkp)->customer, plname, PL_NSIZ);
    ESHK(shkp)->following = 1;
}

/* used when the shkp is teleported or falls (ox == 0) out of his shop,
 * or when the player is not on a costly_spot and he
 * damages something inside the shop.  these conditions
 * must be checked by the calling function.
 */
void make_angry_shk (struct monst *shkp, signed char ox, signed char oy) {
    signed char sx, sy;
    struct eshk *eshkp = ESHK(shkp);

    /* all pending shop transactions are now "past due" */
    if (eshkp->billct || eshkp->debit || eshkp->loan || eshkp->credit) {
        eshkp->robbed += (addupbill(shkp) + eshkp->debit + eshkp->loan);
        eshkp->robbed -= eshkp->credit;
        if (eshkp->robbed < 0L) eshkp->robbed = 0L;
        /* billct, debit, loan, and credit will be cleared by setpaid */
        setpaid(shkp);
    }

    /* If you just used a wand of teleportation to send the shk away, you
       might not be able to see her any more.  Monnam would yield "it",
       which makes this message look pretty silly, so temporarily restore
       her original location during the call to Monnam. */
    sx = shkp->mx,  sy = shkp->my;
    if (isok(ox, oy) && cansee(ox, oy) && !cansee(sx, sy))
        shkp->mx = ox,  shkp->my = oy;
    unsigned char angry_flag = ANGRY(shkp) ? MSG_FLAG_SHOPKEEPER_ANGRY : 0;
    message_monster_flag(MSG_M_GETS_ANGRY, shkp, angry_flag);
    shkp->mx = sx,  shkp->my = sy;
    hot_pursuit(shkp);
}

/* delivers the cheapest item on the list */
static long cheapest_item ( struct monst *shkp) {
    int ct = ESHK(shkp)->billct;
    struct bill_x *bp = ESHK(shkp)->bill_p;
    long gmin = (bp->price * bp->bquan);

    while(ct--){
        if(bp->price * bp->bquan < gmin)
            gmin = bp->price * bp->bquan;
        bp++;
    }
    return(gmin);
}

/* find obj on one of the lists */
static struct obj * bp_to_obj (struct bill_x *bp) {
    struct obj *obj;
    unsigned int id = bp->bo_id;

    if(bp->useup)
        obj = o_on(id, billobjs);
    else
        obj = find_oid(id);
    return obj;
}

/* shopkeeper tells you what you bought or sold, sometimes partly IDing it */
/* fmt: "%s %ld %s %s", doname(obj), amt, plur(amt), arg */
static void shk_names_obj ( struct monst *shkp, struct obj *obj,
        const char *fmt, long amt, const char *arg)
{
    char *obj_name, fmtbuf[BUFSZ];
    bool was_unknown = !obj->dknown;

    obj->dknown = true;
    /* Use real name for ordinary weapons/armor, and spell-less
     * scrolls/books (that is, blank and mail), but only if the
     * object is within the shk's area of interest/expertise.
     */
    if (!objects[obj->otyp].oc_magic && saleable(shkp, obj) &&
            (obj->oclass == WEAPON_CLASS || obj->oclass == ARMOR_CLASS ||
             obj->oclass == SCROLL_CLASS || obj->oclass == SPBOOK_CLASS ||
             obj->otyp == MIRROR)) {
        was_unknown |= !objects[obj->otyp].oc_name_known;
        makeknown(obj->otyp);
    }
    obj_name = doname(obj);
    /* Use an alternate message when extra information is being provided */
    if (was_unknown) {
        sprintf(fmtbuf, "%%s; you %s", fmt);
        obj_name[0] = highc(obj_name[0]);
        pline(fmtbuf, obj_name, (obj->quan > 1) ? "them" : "it",
                amt, plur(amt), arg);
    } else {
        You(fmt, obj_name, amt, plur(amt), arg);
    }
}

/* return 2 if used-up portion paid */
/*        1 if paid successfully    */
/*        0 if not enough money     */
/*       -1 if skip this object     */
/*       -2 if no money/credit left */
/* which: 0 => used-up item, 1 => other (unpaid or lost) */
static int dopayobj(struct monst *shkp, struct bill_x *bp, struct obj **obj_p,
        int which, bool itemize)
{
    struct obj *obj = *obj_p;
    long ltmp, quan, save_quan;
    int buy;
    bool stashed_gold = (hidden_gold() > 0L),
            consumed = (which == 0);

    if(!obj->unpaid && !bp->useup){
        impossible("Paid object on bill??");
        return PAY_BUY;
    }
    if(itemize && u.ugold + ESHK(shkp)->credit == 0L){
        You("%shave no money or credit left.",
                stashed_gold ? "seem to " : "");
        return PAY_BROKE;
    }
    /* we may need to temporarily adjust the object, if part of the
       original quantity has been used up but part remains unpaid  */
    save_quan = obj->quan;
    if (consumed) {
        /* either completely used up (simple), or split needed */
        quan = bp->bquan;
        if (quan > obj->quan)       /* difference is amount used up */
            quan -= obj->quan;
    } else {
        /* dealing with ordinary unpaid item */
        quan = obj->quan;
    }
    obj->quan = quan;       /* to be used by doname() */
    obj->unpaid = 0;        /* ditto */
    ltmp = bp->price * quan;
    buy = PAY_BUY;          /* flag; if changed then return early */

    if (itemize) {
        char qbuf[BUFSZ];
        sprintf(qbuf,"%s for %ld %s.  Pay?", quan == 1L ?
                Doname2(obj) : doname(obj), ltmp, currency(ltmp));
        if (yn(qbuf) == 'n') {
            buy = PAY_SKIP;         /* don't want to buy */
        } else if (quan < bp->bquan && !consumed) { /* partly used goods */
            obj->quan = bp->bquan - save_quan;      /* used up amount */
            unsigned char angry_flag = ANGRY(shkp) ? MSG_FLAG_SHOPKEEPER_ANGRY : 0;
            unsigned char quantity_flag = (save_quan > 1L) ? MSG_FLAG_QUANTITY_MULTI : 0;
            audio_message_object_flag(MSG_PAY_FOR_THE_OTHER_O_BEFORE_BUYING_THIS,
                    obj, angry_flag|quantity_flag);
            buy = PAY_SKIP;         /* shk won't sell */
        }
    }
    if (buy == PAY_BUY && u.ugold + ESHK(shkp)->credit < ltmp) {
        You("don't%s have gold%s enough to pay for %s.",
                stashed_gold ? " seem to" : "",
                (ESHK(shkp)->credit > 0L) ? " or credit" : "",
                doname(obj));
        buy = itemize ? PAY_SKIP : PAY_CANT;
    }

    if (buy != PAY_BUY) {
        /* restore unpaid object to original state */
        obj->quan = save_quan;
        obj->unpaid = 1;
        return buy;
    }

    pay(ltmp, shkp);
    shk_names_obj(shkp, obj, consumed ?
            "paid for %s at a cost of %ld gold piece%s.%s" :
            "bought %s for %ld gold piece%s.%s", ltmp, "");
    obj->quan = save_quan;          /* restore original count */
    /* quan => amount just bought, save_quan => remaining unpaid count */
    if (consumed) {
        if (quan != bp->bquan) {
            /* eliminate used-up portion; remainder is still unpaid */
            bp->bquan = obj->quan;
            obj->unpaid = 1;
            bp->useup = 0;
            buy = PAY_SOME;
        } else {    /* completely used-up, so get rid of it */
            obj_extract_self(obj);
            /* assert( obj == *obj_p ); */
            dealloc_obj(obj);
            *obj_p = 0;     /* destroy pointer to freed object */
        }
    }
    return buy;
}



int dopay (void) {
    struct eshk *eshkp;
    struct monst *shkp;
    struct monst *nxtm, *resident;
    long ltmp;
    int pass, tmp, sk = 0, seensk = 0;
    bool paid = false, stashed_gold = (hidden_gold() > 0L);

    multi = 0;

    /* find how many shk's there are, how many are in */
    /* sight, and are you in a shop room with one.    */
    nxtm = resident = 0;
    for (shkp = next_shkp(fmon, false);
            shkp; shkp = next_shkp(shkp->nmon, false)) {
        sk++;
        if (ANGRY(shkp) && distu(shkp->mx, shkp->my) <= 2) nxtm = shkp;
        if (canspotmon(shkp)) seensk++;
        if (inhishop(shkp) && (*u.ushops == ESHK(shkp)->shoproom))
            resident = shkp;
    }

    if (nxtm) {                     /* Player should always appease an */
        shkp = nxtm;               /* irate shk standing next to them. */
        goto proceed;
    }

    if ((!sk && (!Blind() || Blind_telepat)) || (!Blind() && !seensk)) {
        There("appears to be no shopkeeper here to receive your payment.");
        return(0);
    }

    if(!seensk) {
        You_cant("see...");
        return(0);
    }

    /* the usual case.  allow paying at a distance when */
    /* inside a tended shop.  should we change that?    */
    if(sk == 1 && resident) {
        shkp = resident;
        goto proceed;
    }

    if (seensk == 1) {
        for (shkp = next_shkp(fmon, false);
                shkp; shkp = next_shkp(shkp->nmon, false))
            if (canspotmon(shkp)) break;
        if (shkp != resident && distu(shkp->mx, shkp->my) > 2) {
            message_monster(MSG_M_NOT_NEAR_ENOUGH_FOR_PAYMENT, shkp);
            return 0;
        }
    } else {
        struct monst *mtmp;
        coord cc;
        int cx, cy;

        pline("Pay whom?");
        cc.x = u.ux;
        cc.y = u.uy;
        if (getpos(&cc, true, "the creature you want to pay") < 0)
            return 0;   /* player pressed ESC */
        cx = cc.x;
        cy = cc.y;
        if(cx < 0) {
            pline("Try again...");
            return(0);
        }
        if(u.ux == cx && u.uy == cy) {
            You("are generous to yourself.");
            return(0);
        }
        mtmp = m_at(cx, cy);
        if(!mtmp) {
            There("is no one there to receive your payment.");
            return(0);
        }
        if(!mtmp->isshk) {
            message_monster(MSG_M_NOT_INTERESTED_IN_YOUR_PAYMENT, mtmp);
            return(0);
        }
        if (mtmp != resident && distu(mtmp->mx, mtmp->my) > 2) {
            message_monster(MSG_M_TOO_FAR_TO_RECEIVE_PAYMENT, mtmp);
            return 0;
        }
        shkp = mtmp;
    }

    if(!shkp) {
        return(0);
    }
proceed:
    eshkp = ESHK(shkp);
    ltmp = eshkp->robbed;

    /* wake sleeping shk when someone who owes money offers payment */
    if (ltmp || eshkp->billct || eshkp->debit)
        rouse_shk(shkp, true);

    if (!shkp->mcanmove || shkp->msleeping) { /* still asleep/paralyzed */
        unsigned char nap_flag = rn2(2) ? MSG_FLAG_NAPPING : 0;
        message_monster_flag(MSG_M_SEEMS_TO_BE_NAPPING, shkp, nap_flag);
        return 0;
    }

    if(shkp != resident && NOTANGRY(shkp)) {
        if (!ltmp) {
            message_monster(MSG_YOU_DO_NOT_OWE_M_ANYTHING, shkp);
        } else if(!u.ugold) {
            You("%shave no money.", stashed_gold ? "seem to " : "");
            if(stashed_gold)
                pline("But you have some gold stashed away.");
        } else {
            long ugold = u.ugold;
            if (ugold > ltmp) {
                message_monster_int(MSG_YOU_GIVE_M_THE_X_GOLD_PIECES, shkp, ltmp);
                pay(ltmp, shkp);
            } else {
                message_monster(MSG_YOU_GIVE_M_ALL_YOUR_GOLD, shkp);
                pay(u.ugold, shkp);
            }
            if((ugold < ltmp/2L) || (ugold < ltmp && stashed_gold)) {
                message_monster(MSG_UNFORTUNATELY_M_DOESNT_LOOK_SATISFIED, shkp);
            } else {
                make_happy_shk(shkp, false);
            }
        }
        return(1);
    }

    /* ltmp is still eshkp->robbed here */
    if (!eshkp->billct && !eshkp->debit) {
        if(!ltmp && NOTANGRY(shkp)) {
            message_monster(MSG_YOU_DO_NOT_OWE_M_ANYTHING, shkp);
            if (!u.ugold)
                pline(no_money, stashed_gold ? " seem to" : "");
        } else if(ltmp) {
            message_monster(MSG_M_IS_AFTER_BLOOD_NOT_MONEY, shkp);
            if(u.ugold < ltmp/2L || (u.ugold < ltmp && stashed_gold)) {
                if (!u.ugold)
                    pline(no_money, stashed_gold ? " seem to" : "");
                else
                    pline(not_enough_money, mhim(shkp));
                return 1;
            }
            if (u.ugold < ltmp) {
                message_monster(MSG_YOU_PARTIALLY_COMPENSATE_M_FOR_HIS_LOSSES, shkp);
            } else {
                message_monster(MSG_YOU_COMPENSATE_M_FOR_HIS_LOSSES, shkp);
            }

            pay(u.ugold < ltmp ? u.ugold : ltmp, shkp);
            make_happy_shk(shkp, false);
        } else {
            /* shopkeeper is angry, but has not been robbed --
             * door broken, attacked, etc. */
            message_monster(MSG_M_AFTER_YOUR_HIDE_NOT_YOUR_MONEY, shkp);
            if(u.ugold < 1000L) {
                if (!u.ugold)
                    pline(no_money, stashed_gold ? " seem to" : "");
                else pline(not_enough_money, mhim(shkp));
                return(1);
            }
            message_monster(MSG_YOU_TRY_APPEASE_M_BY_GIVING_1000_GOLD_PIECES, shkp);
            pay(1000L,shkp);
            if (strncmp(eshkp->customer, plname, PL_NSIZ) || rn2(3)) {
                make_happy_shk(shkp, false);
            } else {
                message_monster(MSG_BUT_M_IS_AS_ANGRY_AS_EVER, shkp);
            }
        }
        return 1;
    }
    if(shkp != resident) {
        impossible("dopay: not to shopkeeper?");
        if(resident) setpaid(resident);
        return(0);
    }
    /* pay debt, if any, first */
    if(eshkp->debit) {
        long dtmp = eshkp->debit;
        long loan = eshkp->loan;
        char sbuf[BUFSZ];
        sprintf(sbuf, "You owe %s %ld %s ",
                shkname(shkp), dtmp, currency(dtmp));
        if(loan) {
            if(loan == dtmp)
                strcat(sbuf, "you picked up in the store.");
            else strcat(sbuf,
                    "for gold picked up and the use of merchandise.");
        } else strcat(sbuf, "for the use of merchandise.");
        plines(sbuf);
        if (u.ugold + eshkp->credit < dtmp) {
            pline("But you don't%s have enough gold%s.",
                    stashed_gold ? " seem to" : "",
                    eshkp->credit ? " or credit" : "");
            return(1);
        } else {
            if (eshkp->credit >= dtmp) {
                eshkp->credit -= dtmp;
                eshkp->debit = 0L;
                eshkp->loan = 0L;
                Your("debt is covered by your credit.");
            } else if (!eshkp->credit) {
                u.ugold -= dtmp;
                shkp->mgold += dtmp;
                eshkp->debit = 0L;
                eshkp->loan = 0L;
                You("pay that debt.");
            } else {
                dtmp -= eshkp->credit;
                eshkp->credit = 0L;
                u.ugold -= dtmp;
                shkp->mgold += dtmp;
                eshkp->debit = 0L;
                eshkp->loan = 0L;
                pline("That debt is partially offset by your credit.");
                You("pay the remainder.");
            }
            paid = true;
        }
    }
    /* now check items on bill */
    if (eshkp->billct) {
        bool itemize;
        if (!u.ugold && !eshkp->credit) {
            You("%shave no money or credit%s.",
                    stashed_gold ? "seem to " : "",
                    paid ? " left" : "");
            return(0);
        }
        if ((u.ugold + eshkp->credit) < cheapest_item(shkp)) {
            You("don't have enough money to buy%s the item%s you picked.",
                    eshkp->billct > 1 ? " any of" : "", plur(eshkp->billct));
            if(stashed_gold)
                pline("Maybe you have some gold stashed away?");
            return(0);
        }

        /* this isn't quite right; it itemizes without asking if the
         * single item on the bill is partly used up and partly unpaid */
        itemize = (eshkp->billct > 1 ? yn("Itemized billing?") == 'y' : 1);

        for (pass = 0; pass <= 1; pass++) {
            tmp = 0;
            while (tmp < eshkp->billct) {
                struct obj *otmp;
                struct bill_x *bp = &(eshkp->bill_p[tmp]);

                /* find the object on one of the lists */
                if ((otmp = bp_to_obj(bp)) != 0) {
                    /* if completely used up, object quantity is stale;
                       restoring it to its original value here avoids
                       making the partly-used-up code more complicated */
                    if (bp->useup) otmp->quan = bp->bquan;
                } else {
                    impossible("Shopkeeper administration out of order.");
                    setpaid(shkp);  /* be nice to the player */
                    return 1;
                }
                if (pass == bp->useup && otmp->quan == bp->bquan) {
                    /* pay for used-up items on first pass and others
                     * on second, so player will be stuck in the store
                     * less often; things which are partly used up
                     * are processed on both passes */
                    tmp++;
                } else {
                    switch (dopayobj(shkp, bp, &otmp, pass, itemize)) {
                        case PAY_CANT:
                            return 1;       /*break*/
                        case PAY_BROKE:
                            paid = true;
                            goto thanks;    /*break*/
                        case PAY_SKIP:
                            tmp++;
                            continue;       /*break*/
                        case PAY_SOME:
                            paid = true;
                            continue;       /*break*/
                        case PAY_BUY:
                            paid = true;
                            break;
                    }
                    *bp = eshkp->bill_p[--eshkp->billct];
                }
            }
        }
thanks: ;
    }
    if(!ANGRY(shkp) && paid) {
        audio_message_monster_string(MSG_THANK_YOU_FOR_SHOPPING_IN_M_S, shkp,
                shtypes[eshkp->shoptype - SHOPBASE].name);
    }
    return(1);
}

static void set_repo_loc (struct eshk *eshkp) {
    signed char ox, oy;

    /* if you're not in this shk's shop room, or if you're in its doorway
       or entry spot, then your gear gets dumped all the way inside */
    if (*u.ushops != eshkp->shoproom ||
            IS_DOOR(levl[u.ux][u.uy].typ) ||
            (u.ux == eshkp->shk.x && u.uy == eshkp->shk.y)) {
        /* shk.x,shk.y is the position immediately in
         * front of the door -- move in one more space
         */
        ox = eshkp->shk.x;
        oy = eshkp->shk.y;
        ox += sgn(ox - eshkp->shd.x);
        oy += sgn(oy - eshkp->shd.y);
    } else {                /* already inside this shk's shop */
        ox = u.ux;
        oy = u.uy;
    }
    /* finish_paybill will deposit invent here */
    repo_location.x = ox;
    repo_location.y = oy;
}

static bool inherits(struct monst *shkp, int numsk, int croaked) {
    long loss = 0L;
    struct eshk *eshkp = ESHK(shkp);
    bool take = false, taken = false;
    int roomno = *u.ushops;
    char takes[BUFSZ];

    /* the simplifying principle is that first-come */
    /* already took everything you had.             */
    if (numsk > 1) {
        if (cansee(shkp->mx, shkp->my && croaked)) {
            unsigned char wakes_up_flag = (!shkp->mcanmove || shkp->msleeping) ?
                MSG_FLAG_WAKES_UP : 0;
            unsigned char shakes_head_flag = (!rn2(2)) ? MSG_FLAG_SHAKES_HEAD : 0;
            unsigned char in_shop_flag = inhishop(shkp) ? MSG_FLAG_IN_SHOP : 0;
            message_monster_flag(MSG_M_LOOKS_AT_YOUR_CORPSE_AND_DISAPPEARS, shkp,
                    wakes_up_flag|shakes_head_flag|in_shop_flag);
        }
        rouse_shk(shkp, false);     /* wake shk for bones */
        taken = (roomno == eshkp->shoproom);
        goto skip;
    }

    /* get one case out of the way: you die in the shop, the */
    /* shopkeeper is peaceful, nothing stolen, nothing owed. */
    if(roomno == eshkp->shoproom && inhishop(shkp) &&
            !eshkp->billct && !eshkp->robbed && !eshkp->debit &&
            NOTANGRY(shkp) && !eshkp->following) {
        if (invent)
            pline("%s gratefully inherits all your possessions.",
                    shkname(shkp));
        set_repo_loc(eshkp);
        goto clear;
    }

    if (eshkp->billct || eshkp->debit || eshkp->robbed) {
        if (roomno == eshkp->shoproom && inhishop(shkp))
            loss = addupbill(shkp) + eshkp->debit;
        if (loss < eshkp->robbed) loss = eshkp->robbed;
        take = true;
    }

    if (eshkp->following || ANGRY(shkp) || take) {
        if (!invent && !u.ugold) goto skip;
        takes[0] = '\0';
        if (!shkp->mcanmove || shkp->msleeping)
            strcat(takes, "wakes up and ");
        if (distu(shkp->mx, shkp->my) > 2)
            strcat(takes, "comes and ");
        strcat(takes, "takes");

        if (loss > u.ugold || !loss || roomno == eshkp->shoproom) {
            eshkp->robbed -= u.ugold;
            if (eshkp->robbed < 0L) eshkp->robbed = 0L;
            shkp->mgold += u.ugold;
            u.ugold = 0L;
            pline("%s %s all your possessions.",
                    shkname(shkp), takes);
            taken = true;
            /* where to put player's invent (after disclosure) */
            set_repo_loc(eshkp);
        } else {
            shkp->mgold += loss;
            u.ugold -= loss;
            pline("%s %s the %ld %s %sowed %s.",
                    "TODO: Monnam(shkp)", takes,
                    loss, currency(loss),
                    strncmp(eshkp->customer, plname, PL_NSIZ) ?
                    "" : "you ",
                    shkp->female ? "her" : "him");
            /* shopkeeper has now been paid in full */
            pacify_shk(shkp);
            eshkp->following = 0;
            eshkp->robbed = 0L;
        }
skip:
        /* in case we create bones */
        rouse_shk(shkp, false); /* wake up */
        if (!inhishop(shkp))
            home_shk(shkp, false);
    }
clear:
    setpaid(shkp);
    return(taken);
}

/* routine called after dying (or quitting) */
/* -1: escaped dungeon; 0: quit; 1: died */
bool paybill(int croaked) {
    struct monst *mtmp, *mtmp2, *resident= (struct monst *)0;
    bool taken = false;
    int numsk = 0;

    /* if we escaped from the dungeon, shopkeepers can't reach us;
       shops don't occur on level 1, but this could happen if hero
       level teleports out of the dungeon and manages not to die */
    if (croaked < 0) return false;

    /* this is where inventory will end up if any shk takes it */
    repo_location.x = repo_location.y = 0;

    /* give shopkeeper first crack */
    if ((mtmp = shop_keeper(*u.ushops)) && inhishop(mtmp)) {
        numsk++;
        resident = mtmp;
        taken = inherits(resident, numsk, croaked);
    }
    for (mtmp = next_shkp(fmon, false);
            mtmp; mtmp = next_shkp(mtmp2, false)) {
        mtmp2 = mtmp->nmon;
        if (mtmp != resident) {
            /* for bones: we don't want a shopless shk around */
            if(!on_level(&(ESHK(mtmp)->shoplevel), &u.uz))
                mongone(mtmp);
            else {
                numsk++;
                taken |= inherits(mtmp, numsk, croaked);
            }
        }
    }
    if(numsk == 0) return(false);
    return(taken);
}

/* called at game exit, after inventory disclosure but before making bones */
void finish_paybill (void) {
    struct obj *otmp;
    int ox = repo_location.x,
        oy = repo_location.y;

    /* normally done by savebones(), but that's too late in this case */
    unleash_all();
    /* transfer all of the character's inventory to the shop floor */
    while ((otmp = invent) != 0) {
        otmp->owornmask = 0L;       /* perhaps we should call setnotworn? */
        otmp->lamplit = 0;          /* avoid "goes out" msg from freeinv */
        if (rn2(5)) curse(otmp);    /* normal bones treatment for invent */
        obj_extract_self(otmp);
        place_object(otmp, ox, oy);
    }
}

/*
 * Look for o_id on all lists but billobj.  Return obj or NULL if not found.
 * Its OK for restore_timers() to call this function, there should not
 * be any timeouts on the billobjs chain.
 */
struct obj * find_oid (unsigned id) {
    struct obj *obj;
    struct monst *mon, *mmtmp[3];
    int i;

    /* first check various obj lists directly */
    if ((obj = o_on(id, invent)) != 0) return obj;
    if ((obj = o_on(id, fobj)) != 0) return obj;
    if ((obj = o_on(id, level.buriedobjlist)) != 0) return obj;
    if ((obj = o_on(id, migrating_objs)) != 0) return obj;

    /* not found yet; check inventory for members of various monst lists */
    mmtmp[0] = fmon;
    mmtmp[1] = migrating_mons;
    mmtmp[2] = mydogs;              /* for use during level changes */
    for (i = 0; i < 3; i++)
        for (mon = mmtmp[i]; mon; mon = mon->nmon)
            if ((obj = o_on(id, mon->minvent)) != 0) return obj;

    /* not found at all */
    return (struct obj *)0;
}

static long getprice(struct obj *obj, bool shk_buying) {
    long tmp = (long) objects[obj->otyp].oc_cost;

    if (obj->oartifact) {
        tmp = arti_cost(obj);
        if (shk_buying) tmp /= 4;
    }
    switch(obj->oclass) {
        case FOOD_CLASS:
            /* simpler hunger check, (2-4)*cost */
            if (u.uhs >= HUNGRY && !shk_buying) tmp *= (long) u.uhs;
            if (obj->oeaten) tmp = 0L;
            break;
        case WAND_CLASS:
            if (obj->spe == -1) tmp = 0L;
            break;
        case POTION_CLASS:
            if (obj->otyp == POT_WATER && !obj->blessed && !obj->cursed)
                tmp = 0L;
            break;
        case ARMOR_CLASS:
        case WEAPON_CLASS:
            if (obj->spe > 0) tmp += 10L * (long) obj->spe;
            break;
        case TOOL_CLASS:
            if (Is_candle(obj) &&
                    obj->age < 20L * (long)objects[obj->otyp].oc_cost)
                tmp /= 2L;
            break;
    }
    return tmp;
}

/* calculate the value that the shk will charge for [one of] an object */
/* if angry, impose a surcharge */
static long get_cost ( struct obj *obj, struct monst *shkp ) {
    long tmp = getprice(obj, false);

    if (!tmp) tmp = 5L;
    /* shopkeeper may notice if the player isn't very knowledgeable -
       especially when gem prices are concerned */
    if (!obj->dknown || !objects[obj->otyp].oc_name_known) {
        if (obj->oclass == GEM_CLASS &&
                objects[obj->otyp].oc_material == GLASS) {
            int i;
            /* get a value that's 'random' from game to game, but the
               same within the same game */
            bool pseudorand =
                (((int)u.ubirthday % obj->otyp) >= obj->otyp/2);

            /* all gems are priced high - real or not */
            switch(obj->otyp - LAST_GEM) {
                case 1: /* white */
                    i = pseudorand ? DIAMOND : OPAL;
                    break;
                case 2: /* blue */
                    i = pseudorand ? SAPPHIRE : AQUAMARINE;
                    break;
                case 3: /* red */
                    i = pseudorand ? RUBY : JASPER;
                    break;
                case 4: /* yellowish brown */
                    i = pseudorand ? AMBER : TOPAZ;
                    break;
                case 5: /* orange */
                    i = pseudorand ? JACINTH : AGATE;
                    break;
                case 6: /* yellow */
                    i = pseudorand ? CITRINE : CHRYSOBERYL;
                    break;
                case 7: /* black */
                    i = pseudorand ? BLACK_OPAL : JET;
                    break;
                case 8: /* green */
                    i = pseudorand ? EMERALD : JADE;
                    break;
                case 9: /* violet */
                    i = pseudorand ? AMETHYST : FLUORITE;
                    break;
                default: impossible("bad glass gem %d?", obj->otyp);
                         i = STRANGE_OBJECT;
                         break;
            }
            tmp = (long) objects[i].oc_cost;
        } else if (!(obj->o_id % 4)) /* arbitrarily impose surcharge */
            tmp += tmp / 3L;
    }
    if ((Role_if(PM_TOURIST) && u.ulevel < (MAXULEV/2))
            || (uarmu && !uarm && !uarmc))      /* touristy shirt visible */
        tmp += tmp / 3L;
    else if (uarmh && uarmh->otyp == DUNCE_CAP)
        tmp += tmp / 3L;

    if (ACURR(A_CHA) > 18)          tmp /= 2L;
    else if (ACURR(A_CHA) > 17)     tmp -= tmp / 3L;
    else if (ACURR(A_CHA) > 15)     tmp -= tmp / 4L;
    else if (ACURR(A_CHA) < 6)      tmp *= 2L;
    else if (ACURR(A_CHA) < 8)      tmp += tmp / 2L;
    else if (ACURR(A_CHA) < 11)     tmp += tmp / 3L;
    if (tmp <= 0L) tmp = 1L;
    else if (obj->oartifact) tmp *= 4L;
    /* anger surcharge should match rile_shk's */
    if (shkp && ESHK(shkp)->surcharge) tmp += (tmp + 2L) / 3L;
    return tmp;
}

/* calculate how much the shk will pay when buying [all of] an object */
static long set_cost (struct obj *obj, struct monst *shkp) {
    long tmp = getprice(obj, true) * obj->quan;

    if ((Role_if(PM_TOURIST) && u.ulevel < (MAXULEV/2))
            || (uarmu && !uarm && !uarmc))      /* touristy shirt visible */
        tmp /= 3L;
    else if (uarmh && uarmh->otyp == DUNCE_CAP)
        tmp /= 3L;
    else
        tmp /= 2L;

    /* shopkeeper may notice if the player isn't very knowledgeable -
       especially when gem prices are concerned */
    if (!obj->dknown || !objects[obj->otyp].oc_name_known) {
        if (obj->oclass == GEM_CLASS) {
            /* different shop keepers give different prices */
            if (objects[obj->otyp].oc_material == GEMSTONE ||
                    objects[obj->otyp].oc_material == GLASS) {
                tmp = (obj->otyp % (6 - shkp->m_id % 3));
                tmp = (tmp + 3) * obj->quan;
            }
        } else if (tmp > 1L && !rn2(4))
            tmp -= tmp / 4L;
    }
    return tmp;
}

/* returns the price of a container's content.  the price
 * of the "top" container is added in the calling functions.
 * a different price quoted for selling as vs. buying.
 */
long contained_cost(const struct obj *obj, struct monst *shkp, long price,
        bool usell, bool unpaid_only)
{
    struct obj *otmp;

    /* the price of contained objects */
    for (otmp = obj->cobj; otmp; otmp = otmp->nobj) {
        if (otmp->oclass == COIN_CLASS) continue;
        /* the "top" container is evaluated by caller */
        if (usell) {
            if (saleable(shkp, otmp) &&
                    !otmp->unpaid && otmp->oclass != BALL_CLASS &&
                    !(otmp->oclass == FOOD_CLASS && otmp->oeaten) &&
                    !(Is_candle(otmp) && otmp->age <
                        20L * (long)objects[otmp->otyp].oc_cost))
                price += set_cost(otmp, shkp);
        } else if (!otmp->no_charge &&
                (!unpaid_only || (unpaid_only && otmp->unpaid))) {
            price += get_cost(otmp, shkp) * otmp->quan;
        }

        if (Has_contents(otmp))
            price += contained_cost(otmp, shkp, price, usell, unpaid_only);
    }

    return(price);
}

long contained_gold (struct obj *obj) {
    struct obj *otmp;
    long value = 0L;

    /* accumulate contained gold */
    for (otmp = obj->cobj; otmp; otmp = otmp->nobj)
        if (otmp->oclass == COIN_CLASS)
            value += otmp->quan;
        else if (Has_contents(otmp))
            value += contained_gold(otmp);

    return(value);
}

static void dropped_container(struct obj *obj, struct monst *shkp, bool sale) {
    struct obj *otmp;

    /* the "top" container is treated in the calling fn */
    for (otmp = obj->cobj; otmp; otmp = otmp->nobj) {
        if (otmp->oclass == COIN_CLASS) continue;

        if (!otmp->unpaid && !(sale && saleable(shkp, otmp)))
            otmp->no_charge = 1;

        if (Has_contents(otmp))
            dropped_container(otmp, shkp, sale);
    }
}

void picked_container (struct obj *obj) {
    struct obj *otmp;

    /* the "top" container is treated in the calling fn */
    for (otmp = obj->cobj; otmp; otmp = otmp->nobj) {
        if (otmp->oclass == COIN_CLASS) continue;

        if (otmp->no_charge)
            otmp->no_charge = 0;

        if (Has_contents(otmp))
            picked_container(otmp);
    }
}

/* called from doinv(invent.c) for inventory of unpaid objects */
/* known to be unpaid */
long unpaid_cost ( const struct obj *unp_obj) {
    struct bill_x *bp = (struct bill_x *)0;
    struct monst *shkp;

    for(shkp = next_shkp(fmon, true); shkp;
            shkp = next_shkp(shkp->nmon, true))
        if ((bp = onbill(unp_obj, shkp, true)) != 0) break;

    /* onbill() gave no message if unexpected problem occurred */
    if(!bp) impossible("unpaid_cost: object wasn't on any bill!");

    return bp ? unp_obj->quan * bp->price : 0L;
}

static void add_one_tobill(struct obj *obj, bool dummy) {
    struct monst *shkp;
    struct bill_x *bp;
    int bct;
    char roomno = *u.ushops;

    if (!roomno) return;
    if (!(shkp = shop_keeper(roomno))) return;
    if (!inhishop(shkp)) return;

    if (onbill(obj, shkp, false) || /* perhaps thrown away earlier */
            (obj->oclass == FOOD_CLASS && obj->oeaten))
        return;

    if (ESHK(shkp)->billct == BILLSZ) {
        You("got that for free!");
        return;
    }

    /* To recognize objects the shopkeeper is not interested in. -dgk
    */
    if (obj->no_charge) {
        obj->no_charge = 0;
        return;
    }

    bct = ESHK(shkp)->billct;
    bp = &(ESHK(shkp)->bill_p[bct]);
    bp->bo_id = obj->o_id;
    bp->bquan = obj->quan;
    if(dummy) {               /* a dummy object must be inserted into  */
        bp->useup = 1;        /* the billobjs chain here.  crucial for */
        add_to_billobjs(obj); /* eating floorfood in shop.  see eat.c  */
    } else  bp->useup = 0;
    bp->price = get_cost(obj, shkp);
    ESHK(shkp)->billct++;
    obj->unpaid = 1;
}

/* recursive billing of objects within containers. */
static void bill_box_content(struct obj *obj, bool ininv,
        bool dummy, struct monst *shkp)
{
    struct obj *otmp;

    for (otmp = obj->cobj; otmp; otmp = otmp->nobj) {
        if (otmp->oclass == COIN_CLASS) continue;

        /* the "top" box is added in addtobill() */
        if (!otmp->no_charge)
            add_one_tobill(otmp, dummy);
        if (Has_contents(otmp))
            bill_box_content(otmp, ininv, dummy, shkp);
    }

}

void addtobill(struct obj *obj, bool ininv, bool dummy, bool silent) {
    struct monst *shkp;
    char roomno = *u.ushops;
    long ltmp = 0L, cltmp = 0L, gltmp = 0L;
    bool container = Has_contents(obj);

    if(!*u.ushops) return;

    if(!(shkp = shop_keeper(roomno))) return;

    if(!inhishop(shkp)) return;

    if(/* perhaps we threw it away earlier */
            onbill(obj, shkp, false) ||
            (obj->oclass == FOOD_CLASS && obj->oeaten)
      ) return;

    if(ESHK(shkp)->billct == BILLSZ) {
        You("got that for free!");
        return;
    }

    if(obj->oclass == COIN_CLASS) {
        costly_gold(obj->ox, obj->oy, obj->quan);
        return;
    }

    if(!obj->no_charge)
        ltmp = get_cost(obj, shkp);

    if (obj->no_charge && !container) {
        obj->no_charge = 0;
        return;
    }

    if(container) {
        if(obj->cobj == (struct obj *)0) {
            if(obj->no_charge) {
                obj->no_charge = 0;
                return;
            } else {
                add_one_tobill(obj, dummy);
                goto speak;
            }
        } else {
            cltmp += contained_cost(obj, shkp, cltmp, false, false);
            gltmp += contained_gold(obj);
        }

        if(ltmp) add_one_tobill(obj, dummy);
        if(cltmp) bill_box_content(obj, ininv, dummy, shkp);
        picked_container(obj); /* reset contained obj->no_charge */

        ltmp += cltmp;

        if(gltmp) {
            costly_gold(obj->ox, obj->oy, gltmp);
            if(!ltmp) return;
        }

        if(obj->no_charge)
            obj->no_charge = 0;

    } else /* i.e., !container */
        add_one_tobill(obj, dummy);
speak:
    if (shkp->mcanmove && !shkp->msleeping && !silent) {
        char buf[BUFSZ];

        if(!ltmp) {
            message_monster_object(MSG_M_HAS_NO_INTEREST_IN_O, shkp, obj);
            return;
        }
        strcpy(buf, "\"For you, ");
        if (ANGRY(shkp)) strcat(buf, "scum ");
        else {
            static const char *honored[5] = {
                "good", "honored", "most gracious", "esteemed",
                "most renowned and sacred"
            };
            strcat(buf, honored[rn2(4) + u.uevent.udemigod]);
            if (!is_human(youmonst.data)) strcat(buf, " creature");
            else
                strcat(buf, (flags.female) ? " lady" : " sir");
        }
        if(ininv) {
            long quan = obj->quan;
            obj->quan = 1L; /* fool xname() into giving singular */
            pline("%s; only %ld %s %s.\"", buf, ltmp,
                    (quan > 1L) ? "per" : "for this", xname(obj));
            obj->quan = quan;
        } else
            pline("%s will cost you %ld %s%s.",
                    The(xname(obj)), ltmp, currency(ltmp),
                    (obj->quan > 1L) ? " each" : "");
    } else if(!silent) {
        if(ltmp) {
            pline_The("list price of %s is %ld %s%s.",
                the(xname(obj)), ltmp, currency(ltmp),
                (obj->quan > 1L) ? " each" : "");
        } else {
            message_monster(MSG_M_DOES_NOT_NOTICE, shkp);
        }
    }
}

void splitbill (struct obj *obj, struct obj *otmp) {
    /* otmp has been split off from obj */
    struct bill_x *bp;
    long tmp;
    struct monst *shkp = shop_keeper(*u.ushops);

    if(!shkp || !inhishop(shkp)) {
        impossible("splitbill: no resident shopkeeper??");
        return;
    }
    bp = onbill(obj, shkp, false);
    if(!bp) {
        impossible("splitbill: not on bill?");
        return;
    }
    if(bp->bquan < otmp->quan) {
        impossible("Negative quantity on bill??");
    }
    if(bp->bquan == otmp->quan) {
        impossible("Zero quantity on bill??");
    }
    bp->bquan -= otmp->quan;

    if(ESHK(shkp)->billct == BILLSZ) otmp->unpaid = 0;
    else {
        tmp = bp->price;
        bp = &(ESHK(shkp)->bill_p[ESHK(shkp)->billct]);
        bp->bo_id = otmp->o_id;
        bp->bquan = otmp->quan;
        bp->useup = 0;
        bp->price = tmp;
        ESHK(shkp)->billct++;
    }
}

static void sub_one_frombill (struct obj *obj, struct monst *shkp) {
    struct bill_x *bp;

    if((bp = onbill(obj, shkp, false)) != 0) {
        struct obj *otmp;

        obj->unpaid = 0;
        if(bp->bquan > obj->quan){
            otmp = newobj(0);
            *otmp = *obj;
            bp->bo_id = otmp->o_id = flags.ident++;
            otmp->where = OBJ_FREE;
            otmp->quan = (bp->bquan -= obj->quan);
            otmp->owt = 0;  /* superfluous */
            otmp->onamelth = 0;
            otmp->oxlth = 0;
            otmp->oattached = OATTACHED_NOTHING;
            bp->useup = 1;
            add_to_billobjs(otmp);
            return;
        }
        ESHK(shkp)->billct--;
        *bp = ESHK(shkp)->bill_p[ESHK(shkp)->billct];
        return;
    } else if (obj->unpaid) {
        impossible("sub_one_frombill: unpaid object not on bill");
        obj->unpaid = 0;
    }
}

/* recursive check of unpaid objects within nested containers. */
void subfrombill (struct obj *obj, struct monst *shkp) {
    struct obj *otmp;

    sub_one_frombill(obj, shkp);

    if (Has_contents(obj))
        for(otmp = obj->cobj; otmp; otmp = otmp->nobj) {
            if(otmp->oclass == COIN_CLASS) continue;

            if (Has_contents(otmp))
                subfrombill(otmp, shkp);
            else
                sub_one_frombill(otmp, shkp);
        }
}

static long stolen_container(struct obj *obj, struct monst *shkp, long price, bool ininv) {
    struct obj *otmp;

    if(ininv && obj->unpaid)
        price += get_cost(obj, shkp);
    else {
        if(!obj->no_charge)
            price += get_cost(obj, shkp);
        obj->no_charge = 0;
    }

    /* the price of contained objects, if any */
    for(otmp = obj->cobj; otmp; otmp = otmp->nobj) {

        if(otmp->oclass == COIN_CLASS) continue;

        if (!Has_contents(otmp)) {
            if(ininv) {
                if(otmp->unpaid)
                    price += otmp->quan * get_cost(otmp, shkp);
            } else {
                if(!otmp->no_charge) {
                    if(otmp->oclass != FOOD_CLASS || !otmp->oeaten)
                        price += otmp->quan * get_cost(otmp, shkp);
                }
                otmp->no_charge = 0;
            }
        } else
            price += stolen_container(otmp, shkp, price, ininv);
    }

    return(price);
}

long stolen_value(struct obj *obj, signed char x, signed char y,
        bool peaceful, bool silent)
{
    long value = 0L, gvalue = 0L;
    struct monst *shkp = shop_keeper(*in_rooms(x, y, SHOPBASE));

    if (!shkp || !inhishop(shkp))
        return (0L);

    if(obj->oclass == COIN_CLASS) {
        gvalue += obj->quan;
    } else if (Has_contents(obj)) {
        bool ininv = !!count_unpaid(obj->cobj);

        value += stolen_container(obj, shkp, value, ininv);
        if(!ininv) gvalue += contained_gold(obj);
    } else if (!obj->no_charge && saleable(shkp, obj)) {
        value += get_cost(obj, shkp);
    }

    if(gvalue + value == 0L) return(0L);

    value += gvalue;

    if(peaceful) {
        bool credit_use = !!ESHK(shkp)->credit;
        value = check_credit(value, shkp);
        /* 'peaceful' affects general treatment, but doesn't affect
         * the fact that other code expects that all charges after the
         * shopkeeper is angry are included in robbed, not debit */
        if (ANGRY(shkp))
            ESHK(shkp)->robbed += value;
        else
            ESHK(shkp)->debit += value;

        if(!silent) {
            if (credit_use) {
                if (ESHK(shkp)->credit) {
                    You("have %ld %s credit remaining.",
                            ESHK(shkp)->credit, currency(ESHK(shkp)->credit));
                    return value;
                } else if (!value) {
                    You("have no credit remaining.");
                    return 0;
                }
            }
            unsigned char still_flag = credit_use ? MSG_FLAG_STILL : 0;
            if(obj->oclass == COIN_CLASS) {
                message_monster_int_flag(MSG_YOU_OWE_M_X_GOLD, shkp, value, still_flag);
            } else {
                unsigned char them_flag = MSG_FLAG_THEM;
                message_monster_int_flag(MSG_YOU_OWE_M_X_GOLD_FOR_THEM, shkp,
                        value, still_flag|them_flag);
            }
        }
    } else {
        ESHK(shkp)->robbed += value;

        if(!silent) {
            if(cansee(shkp->mx, shkp->my)) {
                char name[BUFSZ];
                Monnam(name, BUFSZ, shkp);
                Norep("%s booms: \"%s, you are a thief!\"", name, plname);
            } else {
                Norep("You hear a scream, \"Thief!\"");
            }
        }
        hot_pursuit(shkp);
        (void) angry_guards(false);
    }
    return(value);
}

void sellobj_state (int deliberate) {
    /* If we're deliberately dropping something, there's no automatic
       response to the shopkeeper's "want to sell" query; however, if we
       accidentally drop anything, the shk will buy it/them without asking.
       This retains the old pre-query risk that slippery fingers while in
       shops entailed:  you drop it, you've lost it.
       */
    sell_response = (deliberate != SELL_NORMAL) ? '\0' : 'a';
    sell_how = deliberate;
    auto_credit = false;
}

void sellobj (struct obj *obj, signed char x, signed char y) {
    struct monst *shkp;
    struct eshk *eshkp;
    long ltmp = 0L, cltmp = 0L, gltmp = 0L, offer;
    bool saleitem, cgold = false, container = Has_contents(obj);
    bool isgold = (obj->oclass == COIN_CLASS);
    bool only_partially_your_contents = false;

    if(!(shkp = shop_keeper(*in_rooms(x, y, SHOPBASE))) ||
            !inhishop(shkp)) return;
    if(!costly_spot(x, y))  return;
    if(!*u.ushops) return;

    if(obj->unpaid && !container && !isgold) {
        sub_one_frombill(obj, shkp);
        return;
    }
    if(container) {
        /* find the price of content before subfrombill */
        cltmp += contained_cost(obj, shkp, cltmp, true, false);
        /* find the value of contained gold */
        gltmp += contained_gold(obj);
        cgold = (gltmp > 0L);
    }

    saleitem = saleable(shkp, obj);
    if(!isgold && !obj->unpaid && saleitem)
        ltmp = set_cost(obj, shkp);

    offer = ltmp + cltmp;

    /* get one case out of the way: nothing to sell, and no gold */
    if(!isgold &&
            ((offer + gltmp) == 0L || sell_how == SELL_DONTSELL)) {
        bool unpaid = (obj->unpaid ||
                (container && count_unpaid(obj->cobj)));

        if(container) {
            dropped_container(obj, shkp, false);
            if(!obj->unpaid && !saleitem)
                obj->no_charge = 1;
            if(obj->unpaid || count_unpaid(obj->cobj))
                subfrombill(obj, shkp);
        } else obj->no_charge = 1;

        if(!unpaid && (sell_how != SELL_DONTSELL))
            message_monster(MSG_M_SEEMS_UNINTERESTED, shkp);
        return;
    }

    /* you dropped something of your own - probably want to sell it */
    rouse_shk(shkp, true);  /* wake up sleeping or paralyzed shk */
    eshkp = ESHK(shkp);

    if (ANGRY(shkp)) { /* they become shop-objects, no pay */
        pline("Thank you, scum!");
        subfrombill(obj, shkp);
        return;
    }

    if(eshkp->robbed) {  /* shkp is not angry? */
        if(isgold) offer = obj->quan;
        else if(cgold) offer += cgold;
        if((eshkp->robbed -= offer < 0L))
            eshkp->robbed = 0L;
        if(offer) verbalize(
                "Thank you for your contribution to restock this recently plundered shop.");
        subfrombill(obj, shkp);
        return;
    }

    if(isgold || cgold) {
        if(!cgold) gltmp = obj->quan;

        if(eshkp->debit >= gltmp) {
            if(eshkp->loan) { /* you carry shop's gold */
                if(eshkp->loan >= gltmp)
                    eshkp->loan -= gltmp;
                else eshkp->loan = 0L;
            }
            eshkp->debit -= gltmp;
            Your("debt is %spaid off.",
                    eshkp->debit ? "partially " : "");
        } else {
            long delta = gltmp - eshkp->debit;

            eshkp->credit += delta;
            if(eshkp->debit) {
                eshkp->debit = 0L;
                eshkp->loan = 0L;
                Your("debt is paid off.");
            }
            pline("%ld %s %s added to your credit.",
                    delta, currency(delta), delta > 1L ? "are" : "is");
        }
        if(offer) goto move_on;
        else {
            if(!isgold) {
                if (container)
                    dropped_container(obj, shkp, false);
                if (!obj->unpaid && !saleitem) obj->no_charge = 1;
                subfrombill(obj, shkp);
            }
            return;
        }
    }
move_on:
    if((!saleitem && !(container && cltmp > 0L))
            || eshkp->billct == BILLSZ
            || obj->oclass == BALL_CLASS
            || obj->oclass == CHAIN_CLASS || offer == 0L
            || (obj->oclass == FOOD_CLASS && obj->oeaten)
            || (Is_candle(obj) &&
                obj->age < 20L * (long)objects[obj->otyp].oc_cost))
    {
        unsigned char in_the_rest_flag = cgold ? MSG_FLAG_IN_THE_REST : 0;
        message_monster_flag(MSG_M_SEEMS_UNINTERESTED, shkp, in_the_rest_flag);
        if (container)
            dropped_container(obj, shkp, false);
        obj->no_charge = 1;
        return;
    }

    if(!shkp->mgold) {
        char c, qbuf[BUFSZ];
        long tmpcr = ((offer * 9L) / 10L) + (offer <= 1L);

        if (sell_how == SELL_NORMAL || auto_credit) {
            c = sell_response = 'y';
        } else if (sell_response != 'n') {
            message_monster(MSG_M_CANNOT_PAY_YOU_AT_PRESENT, shkp);
            sprintf(qbuf,
                    "Will you accept %ld %s in credit for %s?",
                    tmpcr, currency(tmpcr), doname(obj));
            /* won't accept 'a' response here */
            /* KLY - 3/2000 yes, we will, it's a damn nuisance
               to have to constantly hit 'y' to sell for credit */
            c = ynaq(qbuf);
            if (c == 'a') {
                c = 'y';
                auto_credit = true;
            }
        } else          /* previously specified "quit" */
            c = 'n';

        if (c == 'y') {
            shk_names_obj(shkp, obj, (sell_how != SELL_NORMAL) ?
                    "traded %s for %ld zorkmid%s in %scredit." :
                    "relinquish %s and acquire %ld zorkmid%s in %scredit.",
                    tmpcr,
                    (eshkp->credit > 0L) ? "additional " : "");
            eshkp->credit += tmpcr;
            subfrombill(obj, shkp);
        } else {
            if (c == 'q') sell_response = 'n';
            if (container)
                dropped_container(obj, shkp, false);
            if (!obj->unpaid) obj->no_charge = 1;
            subfrombill(obj, shkp);
        }
    } else {
        char qbuf[BUFSZ];
        bool short_funds = (offer > shkp->mgold);
        if (short_funds) offer = shkp->mgold;
        if (!sell_response) {
            only_partially_your_contents =
                (contained_cost(obj, shkp, 0L, false, false) !=
                 contained_cost(obj, shkp, 0L, false, true));
            char shkp_name[BUFSZ];
            Monnam(shkp_name, BUFSZ, shkp);
            sprintf(qbuf,
                    "%s offers%s %ld gold piece%s for%s %s %s.  Sell %s?",
                    shkp_name, short_funds ? " only" : "",
                    offer, plur(offer),
                    (!ltmp && cltmp && only_partially_your_contents) ?
                    " your items in" : (!ltmp && cltmp) ? " the contents of" : "",
                    obj->unpaid ? "the" : "your", cxname(obj),
                    (obj->quan == 1L &&
                     !(!ltmp && cltmp && only_partially_your_contents)) ?
                    "it" : "them");
        } else  qbuf[0] = '\0';         /* just to pacify lint */

        switch (sell_response ? sell_response : ynaq(qbuf)) {
            case 'q':  sell_response = 'n';
            case 'n':  if (container)
                           dropped_container(obj, shkp, false);
                       if (!obj->unpaid) obj->no_charge = 1;
                       subfrombill(obj, shkp);
                       break;
            case 'a':  sell_response = 'y';
            case 'y':  if (container)
                           dropped_container(obj, shkp, true);
                       if (!obj->unpaid && !saleitem) obj->no_charge = 1;
                       subfrombill(obj, shkp);
                       pay(-offer, shkp);
                       shk_names_obj(shkp, obj, (sell_how != SELL_NORMAL) ?
                               (!ltmp && cltmp && only_partially_your_contents) ?
                               "sold some items inside %s for %ld gold pieces%s.%s" :
                               "sold %s for %ld gold piece%s.%s" :
                               "relinquish %s and receive %ld gold piece%s in compensation.%s",
                               offer, "");
                       break;
            default:   impossible("invalid sell response");
        }
    }
}

// mode  0: deliver count 1: paged
int doinvbill (int mode) {
    struct monst *shkp;
    struct eshk *eshkp;
    struct bill_x *bp, *end_bp;
    struct obj *obj;
    long totused;
    char *buf_p;
    winid datawin;

    shkp = shop_keeper(*u.ushops);
    if (!shkp || !inhishop(shkp)) {
        if (mode != 0) impossible("doinvbill: no shopkeeper?");
        return 0;
    }
    eshkp = ESHK(shkp);

    if (mode == 0) {
        /* count expended items, so that the `I' command can decide
           whether to include 'x' in its prompt string */
        int cnt = !eshkp->debit ? 0 : 1;

        for (bp = eshkp->bill_p, end_bp = &eshkp->bill_p[eshkp->billct];
                bp < end_bp; bp++)
            if (bp->useup ||
                    ((obj = bp_to_obj(bp)) != 0 && obj->quan < bp->bquan))
                cnt++;
        return cnt;
    }

    datawin = create_nhwindow(NHW_MENU);
    putstr(datawin, 0, "Unpaid articles already used up:");
    putstr(datawin, 0, "");

    totused = 0L;
    for (bp = eshkp->bill_p, end_bp = &eshkp->bill_p[eshkp->billct];
            bp < end_bp; bp++) {
        obj = bp_to_obj(bp);
        if(!obj) {
            impossible("Bad shopkeeper administration.");
            goto quit;
        }
        if(bp->useup || bp->bquan > obj->quan) {
            long oquan, uquan, thisused;
            unsigned save_unpaid;

            save_unpaid = obj->unpaid;
            oquan = obj->quan;
            uquan = (bp->useup ? bp->bquan : bp->bquan - oquan);
            thisused = bp->price * uquan;
            totused += thisused;
            obj->unpaid = 0;                /* ditto */
            /* Why 'x'?  To match `I x', more or less. */
            buf_p = xprname(obj, (char *)0, 'x', false, thisused, uquan);
            obj->unpaid = save_unpaid;
            putstr(datawin, 0, buf_p);
        }
    }
    if (eshkp->debit) {
        /* additional shop debt which has no itemization available */
        if (totused) putstr(datawin, 0, "");
        totused += eshkp->debit;
        buf_p = xprname((struct obj *)0,
                "usage charges and/or other fees",
                GOLD_SYM, false, eshkp->debit, 0L);
        putstr(datawin, 0, buf_p);
    }
    buf_p = xprname((struct obj *)0, "Total:", '*', false, totused, 0L);
    putstr(datawin, 0, "");
    putstr(datawin, 0, buf_p);
    display_nhwindow(datawin, false);
quit:
    destroy_nhwindow(datawin);
    return(0);
}


/* shk catches thrown pick-axe */
struct monst * shkcatch (struct obj *obj, signed char x, signed char y) {
    struct monst *shkp;

    if (!(shkp = shop_keeper(inside_shop(x, y))) ||
            !inhishop(shkp)) return(0);

    if (shkp->mcanmove && !shkp->msleeping &&
            (*u.ushops != ESHK(shkp)->shoproom || !inside_shop(u.ux, u.uy)) &&
            dist2(shkp->mx, shkp->my, x, y) < 3 &&
            /* if it is the shk's pos, you hit and anger him */
            (shkp->mx != x || shkp->my != y)) {
        if (mnearto(shkp, x, y, true))
            verbalize("Out of my way, scum!");
        if (cansee(x, y)) {
            unsigned char reaches_over_flag = (x == shkp->mx && y == shkp->my) ?
                0 : MSG_FLAG_REACHES_OVER;
            message_monster_object_flag(MSG_M_NIMBLY_CATCHES_O, shkp, obj, reaches_over_flag);
            if (!canspotmon(shkp))
                map_invisible(x, y);
        }
        subfrombill(obj, shkp);
        mpickobj(shkp, obj);
        return shkp;
    }
    return NULL;
}

void add_damage (signed char x, signed char y, long cost) {
    struct damage *tmp_dam;
    char *shops;

    if (IS_DOOR(levl[x][y].typ)) {
        struct monst *mtmp;

        /* Don't schedule for repair unless it's a real shop entrance */
        for (shops = in_rooms(x, y, SHOPBASE); *shops; shops++)
            if ((mtmp = shop_keeper(*shops)) != 0 &&
                    x == ESHK(mtmp)->shd.x && y == ESHK(mtmp)->shd.y)
                break;
        if (!*shops) return;
    }
    for (tmp_dam = level.damagelist; tmp_dam; tmp_dam = tmp_dam->next)
        if (tmp_dam->place.x == x && tmp_dam->place.y == y) {
            tmp_dam->cost += cost;
            return;
        }
    tmp_dam = (struct damage *)malloc((unsigned)sizeof(struct damage));
    tmp_dam->when = monstermoves;
    tmp_dam->place.x = x;
    tmp_dam->place.y = y;
    tmp_dam->cost = cost;
    tmp_dam->typ = levl[x][y].typ;
    tmp_dam->next = level.damagelist;
    level.damagelist = tmp_dam;
    /* If player saw damage, display as a wall forever */
    if (cansee(x, y))
        levl[x][y].seenv = SVALL;
}

/*
 * 0: repair postponed, 1: silent repair (no messages), 2: normal repair
 * 3: untrap
 *
 * catchup: restoring a level
 */
int repair_damage(struct monst *shkp, struct damage *tmp_dam, bool catchup) {
    signed char x, y, i;
    signed char litter[9];
    struct monst *mtmp;
    struct obj *otmp;
    struct trap *ttmp;

    if ((monstermoves - tmp_dam->when) < REPAIR_DELAY)
        return(0);
    if (shkp->msleeping || !shkp->mcanmove || ESHK(shkp)->following)
        return(0);
    x = tmp_dam->place.x;
    y = tmp_dam->place.y;
    if (!IS_ROOM(tmp_dam->typ)) {
        if (x == u.ux && y == u.uy)
            if (!Passes_walls)
                return(0);
        if (x == shkp->mx && y == shkp->my)
            return(0);
        if ((mtmp = m_at(x, y)) && (!passes_walls(mtmp->data)))
            return(0);
    }
    if ((ttmp = t_at(x, y)) != 0) {
        if (x == u.ux && y == u.uy)
            if (!Passes_walls)
                return(0);
        if (ttmp->ttyp == LANDMINE || ttmp->ttyp == BEAR_TRAP) {
            /* convert to an object */
            otmp = mksobj((ttmp->ttyp == LANDMINE) ? LAND_MINE :
                    BEARTRAP, true, false);
            otmp->quan= 1;
            otmp->owt = weight(otmp);
            (void) mpickobj(shkp, otmp);
        }
        deltrap(ttmp);
        if(IS_DOOR(tmp_dam->typ)) {
            levl[x][y].doormask = D_CLOSED; /* arbitrary */
            block_point(x, y);
        } else if (IS_WALL(tmp_dam->typ)) {
            levl[x][y].typ = tmp_dam->typ;
            block_point(x, y);
        }
        newsym(x, y);
        return(3);
    }
    if (IS_ROOM(tmp_dam->typ)) {
        /* No messages, because player already filled trap door */
        return(1);
    }
    if ((tmp_dam->typ == levl[x][y].typ) &&
            (!IS_DOOR(tmp_dam->typ) || (levl[x][y].doormask > D_BROKEN)))
        /* No messages if player already replaced shop door */
        return(1);
    levl[x][y].typ = tmp_dam->typ;
    (void) memset((void *)litter, 0, sizeof(litter));
    if ((otmp = level.objects[x][y]) != 0) {
        /* Scatter objects haphazardly into the shop */
        for (i = 0; i < 9; i++) {
            if ((i == 4) || (!ZAP_POS(levl[x+horiz(i)][y+vert(i)].typ)))
                continue;
            litter[i] = OPEN;
            if (inside_shop(x+horiz(i),
                        y+vert(i)) == ESHK(shkp)->shoproom)
                litter[i] |= INSHOP;
        }
        if (Punished && !u.uswallow &&
                ((uchain->ox == x && uchain->oy == y) ||
                 (uball->ox == x && uball->oy == y))) {
            /*
             * Either the ball or chain is in the repair location.
             *
             * Take the easy way out and put ball&chain under hero.
             */
            verbalize("Get your junk out of my wall!");
            unplacebc();    /* pick 'em up */
            placebc();      /* put 'em down */
        }
        while ((otmp = level.objects[x][y]) != 0)
            /* Don't mess w/ boulders -- just merge into wall */
            if ((otmp->otyp == BOULDER) || (otmp->otyp == ROCK)) {
                obj_extract_self(otmp);
                obfree(otmp, (struct obj *)0);
            } else {
                while (!(litter[i = rn2(9)] & INSHOP));
                remove_object(otmp);
                place_object(otmp, x+horiz(i), y+vert(i));
                litter[i] |= NEED_UPDATE;
            }
    }
    if (catchup) return 1;  /* repair occurred while off level */

    block_point(x, y);
    if(IS_DOOR(tmp_dam->typ)) {
        levl[x][y].doormask = D_CLOSED; /* arbitrary */
        newsym(x, y);
    } else {
        /* don't set doormask  - it is (hopefully) the same as it was */
        /* if not, perhaps save it with the damage array...  */

        if (IS_WALL(tmp_dam->typ) && cansee(x, y)) {
            /* Player sees actual repair process, so they KNOW it's a wall */
            levl[x][y].seenv = SVALL;
            newsym(x, y);
        }
        /* Mark this wall as "repaired".  There currently is no code */
        /* to do anything about repaired walls, so don't do it.      */
    }
    for (i = 0; i < 9; i++)
        if (litter[i] & NEED_UPDATE)
            newsym(x+horiz(i), y+vert(i));
    return(2);
}
/*
 * shk_move: return 1: moved  0: didn't  -1: let m_move do it  -2: died
 */
int shk_move (struct monst *shkp) {
    signed char gx,gy,omx,omy;
    int udist;
    signed char appr;
    struct eshk *eshkp = ESHK(shkp);
    int z;
    bool uondoor = false, satdoor, avoid = false, badinv;

    omx = shkp->mx;
    omy = shkp->my;

    if (inhishop(shkp))
        remove_damage(shkp, false);

    if((udist = distu(omx,omy)) < 3 &&
            (shkp->data != &mons[PM_GRID_BUG] || (omx==u.ux || omy==u.uy))) {
        if(ANGRY(shkp) ||
                (Conflict && !resist(shkp, RING_CLASS, 0, 0))) {
            if (Displaced)
                message_monster(MSG_YOUR_DISPLACED_IMAGE_DOESNT_FOOL_M, shkp);
            mattacku(shkp);
            return(0);
        }
        if(eshkp->following) {
            if(strncmp(eshkp->customer, plname, PL_NSIZ)) {
                verbalize("%s, %s!  I was looking for %s.", Hello(shkp), plname, eshkp->customer);
                eshkp->following = 0;
                return(0);
            }
            if(moves > followmsg+4) {
                verbalize("%s, %s!  Didn't you forget to pay?", Hello(shkp), plname);
                followmsg = moves;
                if (!rn2(9)) {
                    message_monster(MSG_M_DOESNT_LIKE_CUSTOMERS_WHO_DONT_PAY, shkp);
                    rile_shk(shkp);
                }
            }
            if(udist < 2)
                return(0);
        }
    }

    appr = 1;
    gx = eshkp->shk.x;
    gy = eshkp->shk.y;
    satdoor = (gx == omx && gy == omy);
    if(eshkp->following || ((z = holetime()) >= 0 && z*z <= udist)){
        /* [This distance check used to apply regardless of
           whether the shk was following, but that resulted in
           m_move() sometimes taking the shk out of the shop if
           the player had fenced him in with boulders or traps.
           Such voluntary abandonment left unpaid objects in
           invent, triggering billing impossibilities on the
           next level once the character fell through the hole.] */
        if (udist > 4 && eshkp->following)
            return(-1); /* leave it to m_move */
        gx = u.ux;
        gy = u.uy;
    } else if(ANGRY(shkp)) {
        /* Move towards the hero if the shopkeeper can see him. */
        if(shkp->mcansee && m_canseeu(shkp)) {
            gx = u.ux;
            gy = u.uy;
        }
        avoid = false;
    } else {
#define GDIST(x,y)      (dist2(x,y,gx,gy))
        if (Invis
                || u.usteed
           ) {
            avoid = false;
        } else {
            uondoor = (u.ux == eshkp->shd.x && u.uy == eshkp->shd.y);
            if(uondoor) {
                badinv = (carrying(PICK_AXE) || carrying(DWARVISH_MATTOCK) ||
                        (Fast && (sobj_at(PICK_AXE, u.ux, u.uy) ||
                                  sobj_at(DWARVISH_MATTOCK, u.ux, u.uy))));
                if(satdoor && badinv)
                    return(0);
                avoid = !badinv;
            } else {
                avoid = (*u.ushops && distu(gx,gy) > 8);
                badinv = false;
            }

            if(((!eshkp->robbed && !eshkp->billct && !eshkp->debit)
                        || avoid) && GDIST(omx,omy) < 3) {
                if (!badinv && !onlineu(omx,omy))
                    return(0);
                if(satdoor)
                    appr = gx = gy = 0;
            }
        }
    }

    z = move_special(shkp,inhishop(shkp),appr,uondoor,avoid,omx,omy,gx,gy);
    if (z > 0) after_shk_move(shkp);

    return z;
}

/* called after shopkeeper moves, in case the move causes re-entry into shop */
void after_shk_move (struct monst *shkp) {
    struct eshk *eshkp = ESHK(shkp);

    if (eshkp->bill_p == (struct bill_x *) -1000 && inhishop(shkp)) {
        /* reset bill_p, need to re-calc player's occupancy too */
        eshkp->bill_p = &eshkp->bill[0];
        check_special_room(false);
    }
}


/* for use in levl_follower (mondata.c) */
bool is_fshk(const struct monst *mtmp) {
    return mtmp->isshk && ESHK(mtmp)->following;
}

/* You are digging in the shop. */
void shopdig (int fall) {
    struct monst *shkp = shop_keeper(*u.ushops);
    int lang;
    const char *grabs = "grabs";

    if(!shkp) return;

    /* 0 == can't speak, 1 == makes animal noises, 2 == speaks */
    lang = 0;
    if (shkp->msleeping || !shkp->mcanmove || is_silent(shkp->data)) {
        /* lang stays 0 */
    } else if (shkp->data->msound <= MS_ANIMAL) {
        lang = 1;
    } else if (shkp->data->msound >= MS_HUMANOID) {
        lang = 2;
    }

    if(!inhishop(shkp)) {
        if (Role_if(PM_KNIGHT)) {
            You_feel("like a common thief.");
            adjalign(-sgn(u.ualign.type));
        }
        return;
    }

    if(!fall) {
        if (lang == 2) {
            if(u.utraptype == TT_PIT)
                verbalize(
                        "Be careful, %s, or you might fall through the floor.",
                        flags.female ? "madam" : "sir");
            else
                verbalize("%s, do not damage the floor here!",
                        flags.female ? "Madam" : "Sir");
        }
        if (Role_if(PM_KNIGHT)) {
            You_feel("like a common thief.");
            adjalign(-sgn(u.ualign.type));
        }
    } else if(!um_dist(shkp->mx, shkp->my, 5) &&
            !shkp->msleeping && shkp->mcanmove &&
            (ESHK(shkp)->billct || ESHK(shkp)->debit)) {
        struct obj *obj, *obj2;
        if (nolimbs(shkp->data)) {
            grabs = "knocks off";
        }
        if (distu(shkp->mx, shkp->my) > 2) {
            mnexto(shkp);
            /* for some reason the shopkeeper can't come next to you */
            if (distu(shkp->mx, shkp->my) > 2) {
                if (lang == 2)
                    pline("%s curses you in anger and frustration!",
                            shkname(shkp));
                rile_shk(shkp);
                return;
            } else
                pline("%s %s, and %s your backpack!",
                        shkname(shkp),
                        makeplural(locomotion(shkp->data,"leap")), grabs);
        } else
            pline("%s %s your backpack!", shkname(shkp), grabs);

        for(obj = invent; obj; obj = obj2) {
            obj2 = obj->nobj;
            if ((obj->owornmask & ~(W_SWAPWEP|W_QUIVER)) != 0 ||
                    (obj == uswapwep && u.twoweap) ||
                    (obj->otyp == LEASH && obj->leashmon)) continue;
            if (obj == current_wand) continue;
            setnotworn(obj);
            freeinv(obj);
            subfrombill(obj, shkp);
            (void) add_to_minv(shkp, obj);  /* may free obj */
        }
    }
}

void pay_for_damage(const char *dmgstr, bool cant_mollify) {
    struct monst *shkp = (struct monst *)0;
    char shops_affected[5];
    bool uinshp = (*u.ushops != '\0');
    char qbuf[80];
    signed char x, y;
    bool dugwall = !strcmp(dmgstr, "dig into") ||        /* wand */
        !strcmp(dmgstr, "damage");            /* pick-axe */
    struct damage *tmp_dam, *appear_here = 0;
    /* any number >= (80*80)+(24*24) would do, actually */
    long cost_of_damage = 0L;
    unsigned int nearest_shk = 7000, nearest_damage = 7000;
    int picks = 0;

    for (tmp_dam = level.damagelist;
            (tmp_dam && (tmp_dam->when == monstermoves));
            tmp_dam = tmp_dam->next) {
        char *shp;

        if (!tmp_dam->cost)
            continue;
        cost_of_damage += tmp_dam->cost;
        strcpy(shops_affected,
                in_rooms(tmp_dam->place.x, tmp_dam->place.y, SHOPBASE));
        for (shp = shops_affected; *shp; shp++) {
            struct monst *tmp_shk;
            unsigned int shk_distance;

            if (!(tmp_shk = shop_keeper(*shp)))
                continue;
            if (tmp_shk == shkp) {
                unsigned int damage_distance =
                    distu(tmp_dam->place.x, tmp_dam->place.y);

                if (damage_distance < nearest_damage) {
                    nearest_damage = damage_distance;
                    appear_here = tmp_dam;
                }
                continue;
            }
            if (!inhishop(tmp_shk))
                continue;
            shk_distance = distu(tmp_shk->mx, tmp_shk->my);
            if (shk_distance > nearest_shk)
                continue;
            if ((shk_distance == nearest_shk) && picks) {
                if (rn2(++picks))
                    continue;
            } else
                picks = 1;
            shkp = tmp_shk;
            nearest_shk = shk_distance;
            appear_here = tmp_dam;
            nearest_damage = distu(tmp_dam->place.x, tmp_dam->place.y);
        }
    }

    if (!cost_of_damage || !shkp)
        return;

    x = appear_here->place.x;
    y = appear_here->place.y;

    /* not the best introduction to the shk... */
    (void) strncpy(ESHK(shkp)->customer,plname,PL_NSIZ);

    /* if the shk is already on the war path, be sure it's all out */
    if(ANGRY(shkp) || ESHK(shkp)->following) {
        hot_pursuit(shkp);
        return;
    }

    /* if the shk is not in their shop.. */
    if(!*in_rooms(shkp->mx,shkp->my,SHOPBASE)) {
        if(!cansee(shkp->mx, shkp->my))
            return;
        goto getcad;
    }

    if(uinshp) {
        if(um_dist(shkp->mx, shkp->my, 1) &&
                !um_dist(shkp->mx, shkp->my, 3)) {
            pline("%s leaps towards you!", shkname(shkp));
            mnexto(shkp);
        }
        if(um_dist(shkp->mx, shkp->my, 1)) goto getcad;
    } else {
        /*
         * Make shkp show up at the door.  Effect:  If there is a monster
         * in the doorway, have the hero hear the shopkeeper yell a bit,
         * pause, then have the shopkeeper appear at the door, having
         * yanked the hapless critter out of the way.
         */
        if (MON_AT(x, y)) {
            if(flags.soundok) {
                You_hear("an angry voice:");
                verbalize("Out of my way, scum!");
                sleep(1);
            }
        }
        (void) mnearto(shkp, x, y, true);
    }

    if((um_dist(x, y, 1) && !uinshp) || cant_mollify ||
            (u.ugold + ESHK(shkp)->credit) < cost_of_damage
            || !rn2(50)) {
        if(um_dist(x, y, 1) && !uinshp) {
            pline("%s shouts:", shkname(shkp));
            verbalize("Who dared %s my %s?", dmgstr,
                    dugwall ? "shop" : "door");
        } else {
getcad:
            verbalize("How dare you %s my %s?", dmgstr,
                    dugwall ? "shop" : "door");
        }
        hot_pursuit(shkp);
        return;
    }

    if (Invis) Your("invisibility does not fool %s!", shkname(shkp));
    sprintf(qbuf,"\"Cad!  You did %ld %s worth of damage!\"  Pay? ",
            cost_of_damage, currency(cost_of_damage));
    if(yn(qbuf) != 'n') {
        cost_of_damage = check_credit(cost_of_damage, shkp);
        u.ugold -= cost_of_damage;
        shkp->mgold += cost_of_damage;
        pline("Mollified, %s accepts your restitution.",
                shkname(shkp));
        /* move shk back to his home loc */
        home_shk(shkp, false);
        pacify_shk(shkp);
    } else {
        verbalize("Oh, yes!  You'll pay!");
        hot_pursuit(shkp);
        adjalign(-sgn(u.ualign.type));
    }
}
/* called in dokick.c when we kick an object that might be in a store */
bool costly_spot(signed char x, signed char y) {
    struct monst *shkp;

    if (!level.flags.has_shop) return false;
    shkp = shop_keeper(*in_rooms(x, y, SHOPBASE));
    if(!shkp || !inhishop(shkp)) return(false);

    return((bool)(inside_shop(x, y) &&
                !(x == ESHK(shkp)->shk.x &&
                    y == ESHK(shkp)->shk.y)));
}

/* called by dotalk(sounds.c) when #chatting; returns obj if location
   contains shop goods and shopkeeper is willing & able to speak */
struct obj * shop_object (signed char x, signed char y) {
    struct obj *otmp;
    struct monst *shkp;

    if(!(shkp = shop_keeper(*in_rooms(x, y, SHOPBASE))) || !inhishop(shkp))
        return(struct obj *)0;

    for (otmp = level.objects[x][y]; otmp; otmp = otmp->nexthere)
        if (otmp->oclass != COIN_CLASS)
            break;
    /* note: otmp might have ->no_charge set, but that's ok */
    return (otmp && costly_spot(x, y) && NOTANGRY(shkp)
            && shkp->mcanmove && !shkp->msleeping)
        ? otmp : (struct obj *)0;
}

static const char * shk_embellish (struct obj *itm, long cost) {
    if (!rn2(3)) {
        int o, choice = rn2(5);
        if (choice == 0) choice = (cost < 100L ? 1 : cost < 500L ? 2 : 3);
        switch (choice) {
            case 4:
                if (cost < 10L) break; else o = itm->oclass;
                if (o == FOOD_CLASS) return ", gourmets' delight!";
                if (objects[itm->otyp].oc_name_known
                        ? objects[itm->otyp].oc_magic
                        : (o == AMULET_CLASS || o == RING_CLASS   ||
                            o == WAND_CLASS   || o == POTION_CLASS ||
                            o == SCROLL_CLASS || o == SPBOOK_CLASS))
                    return ", painstakingly developed!";
                return ", superb craftsmanship!";
            case 3: return ", finest quality.";
            case 2: return ", an excellent choice.";
            case 1: return ", a real bargain.";
            default: break;
        }
    } else if (itm->oartifact) {
        return ", one of a kind!";
    }
    return ".";
}

/* give price quotes for all objects linked to this one (ie, on this spot) */
void price_quote (struct obj *first_obj) {
    struct obj *otmp;
    char buf[BUFSZ], price[40];
    long cost;
    int cnt = 0;
    winid tmpwin;
    struct monst *shkp = shop_keeper(inside_shop(u.ux, u.uy));

    tmpwin = create_nhwindow(NHW_MENU);
    putstr(tmpwin, 0, "Fine goods for sale:");
    putstr(tmpwin, 0, "");
    for (otmp = first_obj; otmp; otmp = otmp->nexthere) {
        if (otmp->oclass == COIN_CLASS) continue;
        cost = (otmp->no_charge || otmp == uball || otmp == uchain) ? 0L :
            get_cost(otmp, (struct monst *)0);
        if (Has_contents(otmp))
            cost += contained_cost(otmp, shkp, 0L, false, false);
        if (!cost) {
            strcpy(price, "no charge");
        } else {
            sprintf(price, "%ld %s%s", cost, currency(cost),
                    otmp->quan > 1L ? " each" : "");
        }
        sprintf(buf, "%s, %s", doname(otmp), price);
        putstr(tmpwin, 0, buf),  cnt++;
    }
    if (cnt > 1) {
        display_nhwindow(tmpwin, true);
    } else if (cnt == 1) {
        if (first_obj->no_charge || first_obj == uball || first_obj == uchain){
            pline("%s!", buf);  /* buf still contains the string */
        } else {
            /* print cost in slightly different format, so can't reuse buf */
            cost = get_cost(first_obj, (struct monst *)0);
            if (Has_contents(first_obj))
                cost += contained_cost(first_obj, shkp, 0L, false, false);
            pline("%s, price %ld %s%s%s", doname(first_obj),
                    cost, currency(cost), first_obj->quan > 1L ? " each" : "",
                    shk_embellish(first_obj, cost));
        }
    }
    destroy_nhwindow(tmpwin);
}

void shk_chat (struct monst *shkp) {
    struct eshk *eshk;
    if (!shkp->isshk) {
        /* The monster type is shopkeeper, but this monster is
           not actually a shk, which could happen if someone
           wishes for a shopkeeper statue and then animates it.
           (Note: shkname() would be "" in a case like this.) */
        message_monster(MSG_M_ASKS_WHETHER_YOUVE_SEEN_UNTENDED_SHOPS, shkp);
        /* [Perhaps we ought to check whether this conversation
           is taking place inside an untended shop, but a shopless
           shk can probably be expected to be rather disoriented.] */
        return;
    }

    eshk = ESHK(shkp);
    if (ANGRY(shkp))
        pline("%s mentions how much %s dislikes %s customers.",
                shkname(shkp), mhe(shkp),
                eshk->robbed ? "non-paying" : "rude");
    else if (eshk->following) {
        if (strncmp(eshk->customer, plname, PL_NSIZ)) {
            verbalize("%s %s!  I was looking for %s.",
                    Hello(shkp), plname, eshk->customer);
            eshk->following = 0;
        } else {
            verbalize("%s %s!  Didn't you forget to pay?",
                    Hello(shkp), plname);
        }
    } else if (eshk->billct) {
        long total = addupbill(shkp) + eshk->debit;
        pline("%s says that your bill comes to %ld %s.",
                shkname(shkp), total, currency(total));
    } else if (eshk->debit)
        pline("%s reminds you that you owe %s %ld %s.",
                shkname(shkp), mhim(shkp),
                eshk->debit, currency(eshk->debit));
    else if (eshk->credit)
        pline("%s encourages you to use your %ld %s of credit.",
                shkname(shkp), eshk->credit, currency(eshk->credit));
    else if (eshk->robbed)
        pline("%s complains about a recent robbery.", shkname(shkp));
    else if (shkp->mgold < 50)
        pline("%s complains that business is bad.", shkname(shkp));
    else if (shkp->mgold > 4000)
        pline("%s says that business is good.", shkname(shkp));
    else if (strcmp(shkname(shkp), "Izchak") == 0)
        pline(Izchak_speaks[rn2(SIZE(Izchak_speaks))],shkname(shkp));
    else
        pline("%s talks about the problem of shoplifters.",shkname(shkp));
}

// altusage: some items have an "alternate" use with different cost
static long cost_per_charge(struct monst *shkp, struct obj *otmp, bool altusage) {
    long tmp = 0L;

    if(!shkp || !inhishop(shkp)) return(0L); /* insurance */
    tmp = get_cost(otmp, shkp);

    /* The idea is to make the exhaustive use of */
    /* an unpaid item more expensive than buying */
    /* it outright.                              */
    if(otmp->otyp == MAGIC_LAMP) {                   /* 1 */
        /* normal use (ie, as light source) of a magic lamp never
           degrades its value, but not charging anything would make
           identifcation too easy; charge an amount comparable to
           what is charged for an ordinary lamp (don't bother with
           angry shk surchage) */
        if (!altusage) tmp = (long) objects[OIL_LAMP].oc_cost;
        else tmp += tmp / 3L;   /* djinni is being released */
    } else if(otmp->otyp == MAGIC_MARKER) {          /* 70 - 100 */
        /* no way to determine in advance   */
        /* how many charges will be wasted. */
        /* so, arbitrarily, one half of the */
        /* price per use.                   */
        tmp /= 2L;
    } else if(otmp->otyp == BAG_OF_TRICKS ||         /* 1 - 20 */
            otmp->otyp == HORN_OF_PLENTY) {
        tmp /= 5L;
    } else if(otmp->otyp == CRYSTAL_BALL ||          /* 1 - 5 */
            otmp->otyp == OIL_LAMP ||              /* 1 - 10 */
            otmp->otyp == BRASS_LANTERN ||
            (otmp->otyp >= MAGIC_FLUTE &&
             otmp->otyp <= DRUM_OF_EARTHQUAKE) ||   /* 5 - 9 */
            otmp->oclass == WAND_CLASS) {          /* 3 - 11 */
        if (otmp->spe > 1) tmp /= 4L;
    } else if (otmp->oclass == SPBOOK_CLASS) {
        tmp -= tmp / 5L;
    } else if (otmp->otyp == CAN_OF_GREASE ||
            otmp->otyp == TINNING_KIT
            || otmp->otyp == EXPENSIVE_CAMERA) {
        tmp /= 10L;
    } else if (otmp->otyp == POT_OIL) {
        tmp /= 5L;
    }
    return(tmp);
}

/* Charge the player for partial use of an unpaid object.
 *
 * Note that bill_dummy_object() should be used instead
 * when an object is completely used.
 */
void check_unpaid_usage(struct obj *otmp, bool altusage) {
    struct monst *shkp;
    const char *fmt, *arg1, *arg2;
    long tmp;

    if (!otmp->unpaid || !*u.ushops ||
            (otmp->spe <= 0 && objects[otmp->otyp].oc_charged))
        return;
    if (!(shkp = shop_keeper(*u.ushops)) || !inhishop(shkp))
        return;
    if ((tmp = cost_per_charge(shkp, otmp, altusage)) == 0L)
        return;

    arg1 = arg2 = "";
    if (otmp->oclass == SPBOOK_CLASS) {
        fmt = "%sYou owe%s %ld %s.";
        arg1 = rn2(2) ? "This is no free library, cad!  " : "";
        arg2 = ESHK(shkp)->debit > 0L ? " an additional" : "";
    } else if (otmp->otyp == POT_OIL) {
        fmt = "%s%sThat will cost you %ld %s (Yendorian Fuel Tax).";
    } else {
        fmt = "%s%sUsage fee, %ld %s.";
        if (!rn2(3)) arg1 = "Hey!  ";
        if (!rn2(3)) arg2 = "Ahem.  ";
    }

    if (shkp->mcanmove || !shkp->msleeping)
        verbalize(fmt, arg1, arg2, tmp, currency(tmp));
    ESHK(shkp)->debit += tmp;
    exercise(A_WIS, true);          /* you just got info */
}

/* for using charges of unpaid objects "used in the normal manner" */
void check_unpaid (struct obj *otmp) {
    check_unpaid_usage(otmp, false);                /* normal item use */
}

void costly_gold (signed char x, signed char y, long amount) {
    long delta;
    struct monst *shkp;
    struct eshk *eshkp;

    if(!costly_spot(x, y)) return;
    /* shkp now guaranteed to exist by costly_spot() */
    shkp = shop_keeper(*in_rooms(x, y, SHOPBASE));

    eshkp = ESHK(shkp);
    if(eshkp->credit >= amount) {
        if(eshkp->credit > amount)
            Your("credit is reduced by %ld %s.",
                    amount, currency(amount));
        else Your("credit is erased.");
        eshkp->credit -= amount;
    } else {
        delta = amount - eshkp->credit;
        if(eshkp->credit)
            Your("credit is erased.");
        if(eshkp->debit)
            Your("debt increases by %ld %s.",
                    delta, currency(delta));
        else You("owe %s %ld %s.",
                shkname(shkp), delta, currency(delta));
        eshkp->debit += delta;
        eshkp->loan += delta;
        eshkp->credit = 0L;
    }
}

/* used in domove to block diagonal shop-exit */
/* x,y should always be a door */
bool block_door(signed char x,signed char y) {
    int roomno = *in_rooms(x, y, SHOPBASE);
    struct monst *shkp;

    if(roomno < 0 || !IS_SHOP(roomno)) return(false);
    if(!IS_DOOR(levl[x][y].typ)) return(false);
    if(roomno != *u.ushops) return(false);

    if(!(shkp = shop_keeper((char)roomno)) || !inhishop(shkp))
        return(false);

    if(shkp->mx == ESHK(shkp)->shk.x && shkp->my == ESHK(shkp)->shk.y
            /* Actually, the shk should be made to block _any_
             * door, including a door the player digs, if the
             * shk is within a 'jumping' distance.
             */
            && ESHK(shkp)->shd.x == x && ESHK(shkp)->shd.y == y
            && shkp->mcanmove && !shkp->msleeping
            && (ESHK(shkp)->debit || ESHK(shkp)->billct ||
                ESHK(shkp)->robbed)) {
        pline("%s%s blocks your way!", shkname(shkp),
                Invis ? " senses your motion and" : "");
        return(true);
    }
    return(false);
}

/* used in domove to block diagonal shop-entry */
/* u.ux, u.uy should always be a door */
bool block_entry(signed char x,signed char y) {
    signed char sx, sy;
    int roomno;
    struct monst *shkp;

    if(!(IS_DOOR(levl[u.ux][u.uy].typ) &&
                levl[u.ux][u.uy].doormask == D_BROKEN)) return(false);

    roomno = *in_rooms(x, y, SHOPBASE);
    if(roomno < 0 || !IS_SHOP(roomno)) return(false);
    if(!(shkp = shop_keeper((char)roomno)) || !inhishop(shkp))
        return(false);

    if(ESHK(shkp)->shd.x != u.ux || ESHK(shkp)->shd.y != u.uy)
        return(false);

    sx = ESHK(shkp)->shk.x;
    sy = ESHK(shkp)->shk.y;

    if(shkp->mx == sx && shkp->my == sy
            && shkp->mcanmove && !shkp->msleeping
            && (x == sx-1 || x == sx+1 || y == sy-1 || y == sy+1)
            && (Invis || carrying(PICK_AXE) || carrying(DWARVISH_MATTOCK)
                || u.usteed
               )) {
        pline("%s%s blocks your way!", shkname(shkp),
                Invis ? " senses your motion and" : "");
        return(true);
    }
    return(false);
}

static char * shk_owns (char *buf, const struct obj *obj) {
    struct monst *shkp;
    signed char x, y;

    if (get_obj_location(obj, &x, &y, 0) &&
            (obj->unpaid ||
             (obj->where==OBJ_FLOOR && !obj->no_charge && costly_spot(x,y)))) {
        shkp = shop_keeper(inside_shop(x, y));
        char suffixed_name[BUFSZ];
        s_suffix(suffixed_name, BUFSZ, shkname(shkp));
        return strcpy(buf, shkp ? suffixed_name : "the");
    }
    return (char *)0;
}

static char * mon_owns (char *buf, const struct obj *obj) {
    if (obj->where == OBJ_MINVENT) {
        char name[BUFSZ];
        mon_nam(name, BUFSZ, obj->ocarry);
        char suffixed_name[BUFSZ];
        s_suffix(suffixed_name, BUFSZ, name);
        return strcpy(buf, suffixed_name);
    }
    return NULL;
}

char * shk_your (char *buf, const struct obj *obj) {
    if (!shk_owns(buf, obj) && !mon_owns(buf, obj))
        strcpy(buf, carried(obj) ? "your" : "the");
    return buf;
}

char * Shk_Your (char *buf, struct obj *obj) {
    shk_your(buf, obj);
    *buf = highc(*buf);
    return buf;
}

