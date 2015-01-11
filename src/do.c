/* See LICENSE in the root of this project for change info */

/* Contains code for 'd', 'D' (drop), '>', '<' (up, down) */

#include "do.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "move.h"
#include "dungeon_util.h"
#include "align.h"
#include "apply.h"
#include "artifact.h"
#include "attrib.h"
#include "ball.h"
#include "cmd.h"
#include "coord.h"
#include "dbridge.h"
#include "decl.h"
#include "dig.h"
#include "display.h"
#include "do_name.h"
#include "dog.h"
#include "dokick.h"
#include "dothrow.h"
#include "end.h"
#include "engrave.h"
#include "files.h"
#include "flag.h"
#include "fountain.h"
#include "global.h"
#include "hack.h"
#include "hacklib.h"
#include "invent.h"
#include "lev.h"
#include "makemon.h"
#include "mklev.h"
#include "mkmaze.h"
#include "mkobj.h"
#include "mon.h"
#include "mondata.h"
#include "monst.h"
#include "mplayer.h"
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
#include "potion.h"
#include "priest.h"
#include "prop.h"
#include "quest.h"
#include "questpgr.h"
#include "read.h"
#include "region.h"
#include "restore.h"
#include "rm.h"
#include "rnd.h"
#include "save.h"
#include "shk.h"
#include "spell.h"
#include "steal.h"
#include "steed.h"
#include "teleport.h"
#include "timeout.h"
#include "track.h"
#include "trap.h"
#include "uhitm.h"
#include "util.h"
#include "vision.h"
#include "weapon.h"
#include "wield.h"
#include "wintype.h"
#include "wizard.h"
#include "worn.h"
#include "you.h"
#include "youprop.h"
#include "zap.h"

static void trycall(struct obj *);
static void dosinkring(struct obj *);

static int drop(struct obj *);
static int wipeoff(void);

static int menu_drop(int);
static int currentlevel_rewrite(void);
static void final_level(void);

/* on a ladder, used in goto_level */
static bool at_ladder = false;

static const char drop_types[] = { ALLOW_COUNT, COIN_CLASS, ALL_CLASSES, 0 };

d_level save_dlevel = {0, 0};

static char *dfr_pre_msg = 0,   /* plines() before level change */
            *dfr_post_msg = 0;  /* plines() after level change */


/* 'd' command: drop one inventory item */
int dodrop (void) {
    int result, i = (invent || u.ugold) ? 0 : (SIZE(drop_types) - 1);

    if (*u.ushops) sellobj_state(SELL_DELIBERATE);
    result = drop(getobj(&drop_types[i], "drop"));
    if (*u.ushops) sellobj_state(SELL_NORMAL);
    reset_occupations();

    return result;
}


/* Called when a boulder is dropped, thrown, or pushed.  If it ends up
 * in a pool, it either fills the pool up or sinks away.  In either case,
 * it's gone for good...  If the destination is not a pool, returns false.
 */
bool boulder_hits_pool(struct obj *otmp, int rx, int ry, bool pushing) {
        if (!otmp || otmp->otyp != BOULDER)
            impossible("Not a boulder?");
        else if (!Is_waterlevel(&u.uz) && (is_pool(rx,ry) || is_lava(rx,ry))) {
            bool lava = is_lava(rx,ry), fills_up;
            const char *what = waterbody_name(rx,ry);
            signed char ltyp = levl[rx][ry].typ;
            int chance = rn2(10);               /* water: 90%; lava: 10% */
            fills_up = lava ? chance == 0 : chance != 0;

            if (fills_up) {
                struct trap *ttmp = t_at(rx, ry);

                if (ltyp == DRAWBRIDGE_UP) {
                    levl[rx][ry].drawbridgemask &= ~DB_UNDER; /* clear lava */
                    levl[rx][ry].drawbridgemask |= DB_FLOOR;
                } else
                    levl[rx][ry].typ = ROOM;

                if (ttmp) (void) delfloortrap(ttmp);
                bury_objs(rx, ry);

                newsym(rx,ry);
                if (pushing) {
                    You("push %s into the %s.", the(xname(otmp)), what);
                    if (flags.verbose && !Blind)
                        pline("Now you can cross it!");
                    /* no splashing in this case */
                }
            }
            if (!fills_up || !pushing) {        /* splashing occurs */
                if (!u.uinwater) {
                    if (pushing ? !Blind : cansee(rx,ry)) {
                        There("is a large splash as %s %s the %s.",
                              the(xname(otmp)), fills_up? "fills":"falls into",
                              what);
                    } else if (flags.soundok)
                        You_hear("a%s splash.", lava ? " sizzling" : "");
                    wake_nearto(rx, ry, 40);
                }

                if (fills_up && u.uinwater && distu(rx,ry) == 0) {
                    u.uinwater = 0;
                    docrt();
                    vision_full_recalc = 1;
                    You("find yourself on dry land again!");
                } else if (lava && distu(rx,ry) <= 2) {
                    You("are hit by molten lava%c", Fire_resistance() ? '.' : '!');
                    burn_away_slime();
                    losehp(d((Fire_resistance() ? 1 : 3), 6), killed_by_const(KM_MOLTEN_LAVA));
                } else if (!fills_up && flags.verbose && (pushing ? !Blind : cansee(rx,ry))) {
                    pline("It sinks without a trace!");
                }
            }

            /* boulder is now gone */
            if (pushing) delobj(otmp);
            else obfree(otmp, (struct obj *)0);
            return true;
        }
        return false;
}

/* Used for objects which sometimes do special things when dropped; must be
 * called with the object not in any chain.  Returns true if the object goes
 * away.
 */
bool flooreffects (struct obj *obj, int x, int y, const char *verb) {
        struct trap *t;
        struct monst *mtmp;

        if (obj->where != OBJ_FREE)
            panic("flooreffects: obj not free");

        /* make sure things like water_damage() have no pointers to follow */
        obj->nobj = obj->nexthere = (struct obj *)0;

        if (obj->otyp == BOULDER && boulder_hits_pool(obj, x, y, false))
                return true;
        else if (obj->otyp == BOULDER && (t = t_at(x,y)) != 0 &&
                 (t->ttyp==PIT || t->ttyp==SPIKED_PIT || t->ttyp==TRAPDOOR || t->ttyp==HOLE))
        {
                if (((mtmp = m_at(x, y)) && mtmp->mtrapped) ||
                        (u.utrap && u.ux == x && u.uy == y)) {
                    if (*verb) {
                        const char *withyou = (mtmp) ? "" : " with you";
                        char action[BUFSZ];
                        vtense(action, BUFSZ, NULL, verb);
                        pline_The("boulder %s into the pit%s.", action, withyou);
                    }
                    if (mtmp) {
                        if (!passes_walls(mtmp->data) &&
                                !throws_rocks(mtmp->data)) {
                            if (hmon(mtmp, obj, true) && !is_whirly(mtmp->data))
                                return false;   /* still alive */
                        }
                        mtmp->mtrapped = 0;
                    } else {
                        if (!Passes_walls && !throws_rocks(youmonst.data)) {
                            losehp(rnd(15), killed_by_const(KM_SQUISHED_UNDER_BOULDER));
                            return false;       /* player remains trapped */
                        } else u.utrap = 0;
                    }
                }
                if (*verb) {
                        if (Blind) {
                                if ((x == u.ux) && (y == u.uy))
                                        You_hear("a CRASH! beneath you.");
                                else
                                        You_hear("the boulder %s.", verb);
                        } else if (cansee(x, y)) {
                                pline_The("boulder %s%s.",
                                    t->tseen ? "" : "triggers and ",
                                    t->ttyp == TRAPDOOR ? "plugs a trap door" :
                                    t->ttyp == HOLE ? "plugs a hole" :
                                    "fills a pit");
                        }
                }
                deltrap(t);
                obfree(obj, (struct obj *)0);
                bury_objs(x, y);
                newsym(x,y);
                return true;
        } else if (is_lava(x, y)) {
                return fire_damage(obj, false, false, x, y);
        } else if (is_pool(x, y)) {
                /* Reasonably bulky objects (arbitrary) splash when dropped.
                 * If you're floating above the water even small things make noise.
                 * Stuff dropped near fountains always misses */
                if ((Blind || (Levitation || Flying)) && flags.soundok &&
                    ((x == u.ux) && (y == u.uy))) {
                    if (!Underwater) {
                        if (weight(obj) > 9) {
                                pline("Splash!");
                        } else if (Levitation || Flying) {
                                pline("Plop!");
                        }
                    }
                    map_background(x, y, 0);
                    newsym(x, y);
                }
                water_damage(obj, false, false);
        } else if (u.ux == x && u.uy == y &&
                (!u.utrap || u.utraptype != TT_PIT) &&
                (t = t_at(x,y)) != 0 && t->tseen &&
                        (t->ttyp==PIT || t->ttyp==SPIKED_PIT)) {
                /* you escaped a pit and are standing on the precipice */
                char tumble_tense[BUFSZ];
                otense(tumble_tense, BUFSZ, obj, "tumble");
                if (Blind && flags.soundok) {
                    You_hear("%s %s downwards.", The(xname(obj)), tumble_tense);
                } else {
                    pline("%s %s into %s pit.", The(xname(obj)), tumble_tense, the_your[t->madeby_u]);
                }
        }
        return false;
}

/* obj is an object dropped on an altar */
void doaltarobj (struct obj *obj) {
    if (Blind)
        return;

    /* KMH, conduct */
    u.uconduct.gnostic++;

    if ((obj->blessed || obj->cursed) && obj->oclass != COIN_CLASS) {
        char hit_tense[BUFSZ];
        otense(hit_tense, BUFSZ, obj, "hit");
        There("is %s flash as %s %s the altar.",
                an(hcolor(obj->blessed ? NH_AMBER : NH_BLACK)),
                doname(obj), hit_tense);
        if (!Hallucination())
            obj->bknown = 1;
    } else {
        char land_tense[BUFSZ];
        otense(land_tense, BUFSZ, obj, "land");
        pline("%s %s on the altar.", Doname2(obj), land_tense);
        obj->bknown = 1;
    }
}

static void trycall (struct obj *obj) {
    if(!objects[obj->otyp].oc_name_known &&
            !objects[obj->otyp].oc_uname)
        docall(obj);
}

/* obj is a ring being dropped over a kitchen sink */
static void dosinkring (  struct obj *obj) {
    struct obj *otmp,*otmp2;
    bool ideed = true;

    You("drop %s down the drain.", doname(obj));
    obj->in_use = true;     /* block free identification via interrupt */
    switch(obj->otyp) {     /* effects that can be noticed without eyes */
        case RIN_SEARCHING:
            You("thought your %s got lost in the sink, but there it is!",
                    xname(obj));
            goto giveback;
        case RIN_SLOW_DIGESTION:
            pline_The("ring is regurgitated!");
giveback:
            obj->in_use = false;
            dropx(obj);
            trycall(obj);
            return;
        case RIN_LEVITATION:
            pline_The("sink quivers upward for a moment.");
            break;
        case RIN_POISON_RESISTANCE:
            You("smell rotten %s.", makeplural(fruitname(false)));
            break;
        case RIN_AGGRAVATE_MONSTER:
            pline("Several flies buzz angrily around the sink.");
            break;
        case RIN_SHOCK_RESISTANCE:
            pline("Static electricity surrounds the sink.");
            break;
        case RIN_CONFLICT:
            You_hear("loud noises coming from the drain.");
            break;
        case RIN_SUSTAIN_ABILITY:   /* KMH */
            pline_The("water flow seems fixed.");
            break;
        case RIN_GAIN_STRENGTH:
            pline_The("water flow seems %ser now.",
                    (obj->spe<0) ? "weak" : "strong");
            break;
        case RIN_GAIN_CONSTITUTION:
            pline_The("water flow seems %ser now.",
                    (obj->spe<0) ? "less" : "great");
            break;
        case RIN_INCREASE_ACCURACY: /* KMH */
            pline_The("water flow %s the drain.",
                    (obj->spe<0) ? "misses" : "hits");
            break;
        case RIN_INCREASE_DAMAGE:
            pline_The("water's force seems %ser now.",
                    (obj->spe<0) ? "small" : "great");
            break;
        case RIN_HUNGER:
            ideed = false;
            for(otmp = level.objects[u.ux][u.uy]; otmp; otmp = otmp2) {
                otmp2 = otmp->nexthere;
                if (otmp != uball && otmp != uchain && !obj_resists(otmp, 1, 99)) {
                    if (!Blind) {
                        char vanish_tense[BUFSZ];
                        otense(vanish_tense, BUFSZ, otmp, "vanish");
                        pline("Suddenly, %s %s from the sink!", doname(otmp), vanish_tense);
                        ideed = true;
                    }
                    delobj(otmp);
                }
            }
            break;
        case MEAT_RING:
            /* Not the same as aggravate monster; besides, it's obvious. */
            pline("Several flies buzz around the sink.");
            break;
        default:
            ideed = false;
            break;
    }
    if(!Blind && !ideed && obj->otyp != RIN_HUNGER) {
        ideed = true;
        switch(obj->otyp) {         /* effects that need eyes */
            case RIN_ADORNMENT:
                pline_The("faucets flash brightly for a moment.");
                break;
            case RIN_REGENERATION:
                pline_The("sink looks as good as new.");
                break;
            case RIN_INVISIBILITY:
                You("don't see anything happen to the sink.");
                break;
            case RIN_FREE_ACTION:
                You("see the ring slide right down the drain!");
                break;
            case RIN_SEE_INVISIBLE:
                You("see some air in the sink.");
                break;
            case RIN_STEALTH:
                pline_The("sink seems to blend into the floor for a moment.");
                break;
            case RIN_FIRE_RESISTANCE:
                pline_The("hot water faucet flashes brightly for a moment.");
                break;
            case RIN_COLD_RESISTANCE:
                pline_The("cold water faucet flashes brightly for a moment.");
                break;
            case RIN_PROTECTION_FROM_SHAPE_CHAN:
                pline_The("sink looks nothing like a fountain.");
                break;
            case RIN_PROTECTION:
                pline_The("sink glows %s for a moment.",
                        hcolor((obj->spe<0) ? NH_BLACK : NH_SILVER));
                break;
            case RIN_WARNING:
                pline_The("sink glows %s for a moment.", hcolor(NH_WHITE));
                break;
            case RIN_TELEPORTATION:
                pline_The("sink momentarily vanishes.");
                break;
            case RIN_TELEPORT_CONTROL:
                pline_The("sink looks like it is being beamed aboard somewhere.");
                break;
            case RIN_POLYMORPH:
                pline_The("sink momentarily looks like a fountain.");
                break;
            case RIN_POLYMORPH_CONTROL:
                pline_The("sink momentarily looks like a regularly erupting geyser.");
                break;
        }
    }
    if(ideed)
        trycall(obj);
    else
        You_hear("the ring bouncing down the drainpipe.");
    if (!rn2(20)) {
        pline_The("sink backs up, leaving %s.", doname(obj));
        obj->in_use = false;
        dropx(obj);
    } else
        useup(obj);
}


/* some common tests when trying to drop or throw items */
bool canletgo (struct obj *obj, const char *word) {
    if(obj->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL)){
        if (*word)
            Norep("You cannot %s %s you are wearing.",word,
                    something);
        return(false);
    }
    if (obj->otyp == LOADSTONE && obj->cursed) {
        /* getobj() kludge sets corpsenm to user's specified count
           when refusing to split a stack of cursed loadstones */
        if (*word) {
            /* getobj() ignores a count for throwing since that is
               implicitly forced to be 1; replicate its kludge... */
            if (!strcmp(word, "throw") && obj->quan > 1L)
                obj->corpsenm = 1;
            pline("For some reason, you cannot %s%s the stone%s!",
                    word, obj->corpsenm ? " any of" : "",
                    plur(obj->quan));
        }
        obj->corpsenm = 0;              /* reset */
        obj->bknown = 1;
        return(false);
    }
    if (obj->otyp == LEASH && obj->leashmon != 0) {
        if (*word)
            pline_The("leash is tied around your %s.",
                    body_part(HAND));
        return(false);
    }
    if (obj->owornmask & W_SADDLE) {
        if (*word)
            You("cannot %s %s you are sitting on.", word,
                    something);
        return (false);
    }
    return(true);
}

static int drop (struct obj *obj) {
    if(!obj) return(0);
    if(!canletgo(obj,"drop"))
        return(0);
    if(obj == uwep) {
        if(welded(uwep)) {
            weldmsg(obj);
            return(0);
        }
        setuwep(NULL);
    }
    if(obj == uquiver) {
        setuqwep(NULL);
    }
    if (obj == uswapwep) {
        setuswapwep(NULL);
    }

    if (u.uswallow) {
        /* barrier between you and the floor */
        if(flags.verbose) {
            char name[BUFSZ];
            mon_nam(name, BUFSZ, u.ustuck);
            You("drop %s into %s%s %s.", doname(obj),
                    name, possessive_suffix(name), mbodypart(u.ustuck, STOMACH));
        }
    } else {
        if((obj->oclass == RING_CLASS || obj->otyp == MEAT_RING) &&
                IS_SINK(levl[u.ux][u.uy].typ))
        {
            dosinkring(obj);
            return 1;
        }
        if (!can_reach_floor()) {
            if(flags.verbose) You("drop %s.", doname(obj));
            if (obj->oclass != COIN_CLASS || obj == invent) freeinv(obj);
            hitfloor(obj);
            return 1;
        }
        if (!IS_ALTAR(levl[u.ux][u.uy].typ) && flags.verbose)
            You("drop %s.", doname(obj));
    }
    dropx(obj);
    return 1;
}

/* Called in several places - may produce output */
/* eg ship_object() and dropy() -> sellobj() both produce output */
void dropx (struct obj *obj) {
        if (obj->oclass != COIN_CLASS || obj == invent) freeinv(obj);
        if (!u.uswallow) {
            if (ship_object(obj, u.ux, u.uy, false)) return;
            if (IS_ALTAR(levl[u.ux][u.uy].typ))
                doaltarobj(obj); /* set bknown */
        }
        dropy(obj);
}

void
dropy (struct obj *obj)
{
        if (obj == uwep) setuwep((struct obj *)0);
        if (obj == uquiver) setuqwep((struct obj *)0);
        if (obj == uswapwep) setuswapwep((struct obj *)0);

        if (!u.uswallow && flooreffects(obj,u.ux,u.uy,"drop")) return;
        /* uswallow check done by GAN 01/29/87 */
        if(u.uswallow) {
            bool could_petrify = false;
            bool could_poly = false;
            bool could_slime = false;
            bool could_grow = false;
            bool could_heal = false;

            if (obj != uball) {         /* mon doesn't pick up ball */
                if (obj->otyp == CORPSE) {
                    could_petrify = touch_petrifies(&mons[obj->corpsenm]);
                    could_poly = polyfodder(obj);
                    could_slime = (obj->corpsenm == PM_GREEN_SLIME);
                    could_grow = (obj->corpsenm == PM_WRAITH);
                    could_heal = (obj->corpsenm == PM_NURSE);
                }
                mpickobj(u.ustuck,obj);
                if (is_animal(u.ustuck->data)) {
                    if (could_poly || could_slime) {
                        (void) newcham(u.ustuck,
                                       could_poly ? (struct permonst *)0 :
                                       &mons[PM_GREEN_SLIME],
                                       false, could_slime);
                        delobj(obj);    /* corpse is digested */
                    } else if (could_petrify) {
                        minstapetrify(u.ustuck, true);
                        /* Don't leave a cockatrice corpse in a statue */
                        if (!u.uswallow) delobj(obj);
                    } else if (could_grow) {
                        (void) grow_up(u.ustuck, (struct monst *)0);
                        delobj(obj);    /* corpse is digested */
                    } else if (could_heal) {
                        u.ustuck->mhp = u.ustuck->mhpmax;
                        delobj(obj);    /* corpse is digested */
                    }
                }
            }
        } else  {
            place_object(obj, u.ux, u.uy);
            if (obj == uball)
                drop_ball(u.ux,u.uy);
            else
                sellobj(obj, u.ux, u.uy);
            stackobj(obj);
            if(Blind && Levitation)
                map_object(obj, 0);
            newsym(u.ux,u.uy);  /* remap location under self */
        }
}

/* things that must change when not held; recurse into containers.
   Called for both player and monsters */
void
obj_no_longer_held (struct obj *obj)
{
        if (!obj) {
            return;
        } else if ((Is_container(obj) || obj->otyp == STATUE) && obj->cobj) {
            struct obj *contents;
            for(contents=obj->cobj; contents; contents=contents->nobj)
                obj_no_longer_held(contents);
        }
        switch(obj->otyp) {
        case CRYSKNIFE:
            /* KMH -- Fixed crysknives have only 10% chance of reverting */
            /* only changes when not held by player or monster */
            if (!obj->oerodeproof || !rn2(10)) {
                obj->otyp = WORM_TOOTH;
                obj->oerodeproof = 0;
            }
            break;
        }
}

/* 'D' command: drop several things */
int
doddrop (void)
{
        int result = 0;

        add_valid_menu_class(0); /* clear any classes already there */
        if (*u.ushops) sellobj_state(SELL_DELIBERATE);
        if (flags.menu_style != MENU_TRADITIONAL ||
                (result = ggetobj("drop", drop, 0, false, (unsigned *)0)) < -1)
            result = menu_drop(result);
        if (*u.ushops) sellobj_state(SELL_NORMAL);
        reset_occupations();

        return result;
}

/* Drop things from the hero's inventory, using a menu. */
static int
menu_drop (int retry)
{
    int n, i, n_dropped = 0;
    long cnt;
    struct obj *otmp, *otmp2;
    struct obj *u_gold = 0;
    menu_item *pick_list;
    bool all_categories = true;
    bool drop_everything = false;

    if (u.ugold) {
        /* Hack: gold is not in the inventory, so make a gold object
           and put it at the head of the inventory list. */
        u_gold = mkgoldobj(u.ugold);    /* removes from u.ugold */
        u_gold->in_use = true;
        u.ugold = u_gold->quan;         /* put the gold back */
        assigninvlet(u_gold);           /* might end up as NOINVSYM */
        u_gold->nobj = invent;
        invent = u_gold;
    }
    if (retry) {
        all_categories = (retry == -2);
    } else if (flags.menu_style == MENU_FULL) {
        all_categories = false;
        n = query_category("Drop what type of items?",
                        invent,
                        UNPAID_TYPES | ALL_TYPES | CHOOSE_ALL |
                        BUC_BLESSED | BUC_CURSED | BUC_UNCURSED | BUC_UNKNOWN,
                        &pick_list, PICK_ANY);
        if (!n) goto drop_done;
        for (i = 0; i < n; i++) {
            if (pick_list[i].item.a_int == ALL_TYPES_SELECTED)
                all_categories = true;
            else if (pick_list[i].item.a_int == 'A')
                drop_everything = true;
            else
                add_valid_menu_class(pick_list[i].item.a_int);
        }
        free((void *) pick_list);
    } else if (flags.menu_style == MENU_COMBINATION) {
        unsigned ggoresults = 0;
        all_categories = false;
        /* Gather valid classes via traditional NetHack method */
        i = ggetobj("drop", drop, 0, true, &ggoresults);
        if (i == -2) all_categories = true;
        if (ggoresults & ALL_FINISHED) {
                n_dropped = i;
                goto drop_done;
        }
    }

    if (drop_everything) {
        for(otmp = invent; otmp; otmp = otmp2) {
            otmp2 = otmp->nobj;
            n_dropped += drop(otmp);
        }
    } else {
        /* should coordinate with perm invent, maybe not show worn items */
        n = query_objlist("What would you like to drop?", invent,
                        USE_INVLET|INVORDER_SORT, &pick_list,
                        PICK_ANY, all_categories ? allow_all : allow_category);
        if (n > 0) {
            for (i = 0; i < n; i++) {
                otmp = pick_list[i].item.a_obj;
                cnt = pick_list[i].count;
                if (cnt < otmp->quan) {
                    if (welded(otmp)) {
                        ;       /* don't split */
                    } else if (otmp->otyp == LOADSTONE && otmp->cursed) {
                        /* same kludge as getobj(), for canletgo()'s use */
                        otmp->corpsenm = (int) cnt;     /* don't split */
                    } else {
                        if (otmp->oclass == COIN_CLASS)
                            (void) splitobj(otmp, otmp->quan - cnt);
                        else
                            otmp = splitobj(otmp, cnt);
                    }
                }
                n_dropped += drop(otmp);
            }
            free((void *) pick_list);
        }
    }

 drop_done:
    if (u_gold && invent && invent->oclass == COIN_CLASS) {
        /* didn't drop [all of] it */
        u_gold = invent;
        invent = u_gold->nobj;
        u_gold->in_use = false;
        dealloc_obj(u_gold);
    }
    return n_dropped;
}

int dodown (void) {
    struct trap *trap = 0;
    bool stairs_down = ((u.ux == xdnstair && u.uy == ydnstair) ||
            (u.ux == sstairs.sx && u.uy == sstairs.sy && !sstairs.up)),
         ladder_down = (u.ux == xdnladder && u.uy == ydnladder);

    if (u.usteed && !u.usteed->mcanmove) {
        char name[BUFSZ];
        Monnam(name, BUFSZ, u.usteed);
        pline("%s won't move!", name);
        return(0);
    } else if (u.usteed && u.usteed->meating) {
        char name[BUFSZ];
        Monnam(name, BUFSZ, u.usteed);
        pline("%s is still eating.", name);
        return(0);
    } else
        if (Levitation) {
            if ((HLevitation & I_SPECIAL) || (ELevitation & W_ARTI)) {
                /* end controlled levitation */
                if (ELevitation & W_ARTI) {
                    struct obj *obj;

                    for(obj = invent; obj; obj = obj->nobj) {
                        if (obj->oartifact &&
                                artifact_has_invprop(obj,LEVITATION)) {
                            if (obj->age < monstermoves)
                                obj->age = monstermoves + rnz(100);
                            else
                                obj->age += rnz(100);
                        }
                    }
                }
                if (float_down(I_SPECIAL|TIMEOUT, W_ARTI))
                    return (1);   /* came down, so moved */
            }
            floating_above(stairs_down ? "stairs" : ladder_down ?
                    "ladder" : surface(u.ux, u.uy));
            return (0);   /* didn't move */
        }
    if (!stairs_down && !ladder_down) {
        if (!(trap = t_at(u.ux,u.uy)) ||
                (trap->ttyp != TRAPDOOR && trap->ttyp != HOLE)
                || !Can_fall_thru(&u.uz) || !trap->tseen) {

            if (flags.autodig && !flags.nopick &&
                    uwep && is_pick(uwep)) {
                return use_pick_axe2(uwep);
            } else {
                You_cant("go down here.");
                return(0);
            }
        }
    }
    if(u.ustuck) {
        You("are %s, and cannot go down.",
                !u.uswallow ? "being held" : is_animal(u.ustuck->data) ?
                "swallowed" : "engulfed");
        return(1);
    }
    if (on_level(&valley_level, &u.uz) && !u.uevent.gehennom_entered) {
        You("are standing at the gate to Gehennom.");
        pline("Unspeakable cruelty and harm lurk down there.");
        if (yn("Are you sure you want to enter?") != 'y')
            return(0);
        else pline("So be it.");
        u.uevent.gehennom_entered = 1;  /* don't ask again */
    }

    if(!next_to_u()) {
        You("are held back by your pet!");
        return(0);
    }

    if (trap)
        You("%s %s.", locomotion(youmonst.data, "jump"),
                trap->ttyp == HOLE ? "down the hole" : "through the trap door");

    if (trap && Is_stronghold(&u.uz)) {
        flags.nopick = 1;
        goto_hell(false, true);
    } else {
        at_ladder = (bool) (levl[u.ux][u.uy].typ == LADDER);
        flags.nopick = 1;
        next_level(!trap);
        at_ladder = false;
    }
    return(1);
}

int doup (void) {
    if( (u.ux != xupstair || u.uy != yupstair)
            && (!xupladder || u.ux != xupladder || u.uy != yupladder)
            && (!sstairs.sx || u.ux != sstairs.sx || u.uy != sstairs.sy
                || !sstairs.up))
    {
        You_cant("go up here.");
        return(0);
    }
    if (u.usteed && !u.usteed->mcanmove) {
        char name[BUFSZ];
        Monnam(name, BUFSZ, u.usteed);
        pline("%s won't move!", name);
        return(0);
    } else if (u.usteed && u.usteed->meating) {
        char name[BUFSZ];
        Monnam(name, BUFSZ, u.usteed);
        pline("%s is still eating.", name);
        return(0);
    } else
        if(u.ustuck) {
            You("are %s, and cannot go up.",
                    !u.uswallow ? "being held" : is_animal(u.ustuck->data) ?
                    "swallowed" : "engulfed");
            return(1);
        }
    if(near_capacity() > SLT_ENCUMBER) {
        /* No levitation check; inv_weight() already allows for it */
        Your("load is too heavy to climb the %s.",
                levl[u.ux][u.uy].typ == STAIRS ? "stairs" : "ladder");
        return(1);
    }
    if(ledger_no(&u.uz) == 1) {
        if (yn("Beware, there will be no return! Still climb?") != 'y')
            return(0);
    }
    if(!next_to_u()) {
        You("are held back by your pet!");
        return(0);
    }
    at_ladder = (bool) (levl[u.ux][u.uy].typ == LADDER);
    flags.nopick = 1;
    prev_level(true);
    at_ladder = false;
    return(1);
}

/* check that we can write out the current level */
static int currentlevel_rewrite (void) {
        int fd;
        char whynot[BUFSZ];

        fd = create_levelfile(ledger_no(&u.uz), whynot);
        if (fd < 0) {
                /*
                 * This is not quite impossible: e.g., we may have
                 * exceeded our quota. If that is the case then we
                 * cannot leave this level, and cannot save either.
                 * Another possibility is that the directory was not
                 * writable.
                 */
                pline("%s", whynot);
                return -1;
        }

        return fd;
}

void save_currentstate (void) {
        int fd;

        if (flags.ins_chkpt) {
                /* write out just-attained level, with pets and everything */
                fd = currentlevel_rewrite();
                if(fd < 0) return;
                bufon(fd);
                savelev(fd,ledger_no(&u.uz), WRITE_SAVE);
                bclose(fd);
        }

        /* write out non-level state */
        savestateinlock();
}

void goto_level(d_level *newlevel, bool at_stairs, bool falling, bool portal) {
        int fd, l_idx;
        signed char new_ledger;
        bool cant_go_back,
                up = (depth(newlevel) < depth(&u.uz)),
                newdungeon = (u.uz.dnum != newlevel->dnum),
                was_in_W_tower = In_W_tower(u.ux, u.uy, &u.uz),
                familiar = false;
        bool new = false;    /* made a new level? */
        struct monst *mtmp;
        char whynot[BUFSZ];

        if (dunlev(newlevel) > dunlevs_in_dungeon(newlevel))
                newlevel->dlevel = dunlevs_in_dungeon(newlevel);
        if (newdungeon && In_endgame(newlevel)) { /* 1st Endgame Level !!! */
                if (u.uhave.amulet)
                    assign_level(newlevel, &earth_level);
                else return;
        }
        new_ledger = ledger_no(newlevel);
        if (new_ledger <= 0)
                done(ESCAPED);  /* in fact < 0 is impossible */

        /* If you have the amulet and are trying to get out of Gehennom, going
         * up a set of stairs sometimes does some very strange things!
         * Biased against law and towards chaos, but not nearly as strongly
         * as it used to be (prior to 3.2.0).
         * Odds:            old                             new
         *      "up"    L      N      C         "up"    L      N      C
         *       +1   75.0   75.0   75.0         +1   75.0   75.0   75.0
         *        0    0.0   12.5   25.0          0    6.25   8.33  12.5
         *       -1    8.33   4.17   0.0         -1    6.25   8.33  12.5
         *       -2    8.33   4.17   0.0         -2    6.25   8.33   0.0
         *       -3    8.33   4.17   0.0         -3    6.25   0.0    0.0
         */
        if (Inhell && up && u.uhave.amulet && !newdungeon && !portal &&
                                (dunlev(&u.uz) < dunlevs_in_dungeon(&u.uz)-3)) {
                if (!rn2(4)) {
                    int odds = 3 + (int)u.ualign.type,          /* 2..4 */
                        diff = odds <= 1 ? 0 : rn2(odds);       /* paranoia */

                    if (diff != 0) {
                        assign_rnd_level(newlevel, &u.uz, diff);
                        /* if inside the tower, stay inside */
                        if (was_in_W_tower &&
                            !On_W_tower_level(newlevel)) diff = 0;
                    }
                    if (diff == 0)
                        assign_level(newlevel, &u.uz);

                    new_ledger = ledger_no(newlevel);

                    pline("A mysterious force momentarily surrounds you...");
                    if (on_level(newlevel, &u.uz)) {
                        (void) safe_teleds(false);
                        (void) next_to_u();
                        return;
                    } else
                        at_stairs = at_ladder = false;
                }
        }

        /* Prevent the player from going past the first quest level unless
         * (s)he has been given the go-ahead by the leader.
         */
        if (on_level(&u.uz, &qstart_level) && !newdungeon && !ok_to_quest()) {
                pline("A mysterious force prevents you from descending.");
                return;
        }

        if (on_level(newlevel, &u.uz)) return;          /* this can happen */

        fd = currentlevel_rewrite();
        if (fd < 0) return;

        if (falling) /* assuming this is only trap door or hole */
            impact_drop((struct obj *)0, u.ux, u.uy, newlevel->dlevel);

        check_special_room(true);               /* probably was a trap door */
        if (Punished) unplacebc();
        u.utrap = 0;                            /* needed in level_tele */
        fill_pit(u.ux, u.uy);
        u.ustuck = 0;                           /* idem */
        u.uinwater = 0;
        u.uundetected = 0;      /* not hidden, even if means are available */
        keepdogs(false);
        if (u.uswallow)                         /* idem */
                u.uswldtim = u.uswallow = 0;
        /*
         *  We no longer see anything on the level.  Make sure that this
         *  follows u.uswallow set to null since uswallow overrides all
         *  normal vision.
         */
        vision_recalc(2);

        /*
         * Save the level we're leaving.  If we're entering the endgame,
         * we can get rid of all existing levels because they cannot be
         * reached any more.  We still need to use savelev()'s cleanup
         * for the level being left, to recover dynamic memory in use and
         * to avoid dangling timers and light sources.
         */
        cant_go_back = (newdungeon && In_endgame(newlevel));
        if (!cant_go_back) {
            update_mlstmv();    /* current monsters are becoming inactive */
            bufon(fd);          /* use buffered output */
        }
        savelev(fd, ledger_no(&u.uz),
                cant_go_back ? FREE_SAVE : (WRITE_SAVE | FREE_SAVE));
        bclose(fd);
        if (cant_go_back) {
            /* discard unreachable levels; keep #0 */
            for (l_idx = maxledgerno(); l_idx > 0; --l_idx)
                delete_levelfile(l_idx);
        }

        assign_level(&u.uz0, &u.uz);
        assign_level(&u.uz, newlevel);
        assign_level(&u.utolev, newlevel);
        u.utotype = 0;
        if (dunlev_reached(&u.uz) < dunlev(&u.uz))
                dunlev_reached(&u.uz) = dunlev(&u.uz);
        reset_rndmonst(NON_PM);   /* u.uz change affects monster generation */

        /* set default level change destination areas */
        /* the special level code may override these */
        (void) memset((void *) &updest, 0, sizeof updest);
        (void) memset((void *) &dndest, 0, sizeof dndest);

        if (!(level_info[new_ledger].flags & LFILE_EXISTS)) {
                /* entering this level for first time; make it now */
                if (level_info[new_ledger].flags & (FORGOTTEN|VISITED)) {
                    impossible("goto_level: returning to discarded level?");
                    level_info[new_ledger].flags &= ~(FORGOTTEN|VISITED);
                }
                mklev();
                new = true;     /* made the level */
        } else {
                /* returning to previously visited level; reload it */
                fd = open_levelfile(new_ledger, whynot);
                if (fd < 0) {
                        pline("%s", whynot);
                        pline("Probably someone removed it.");
                        killer.method = KM_TRICKED;
                        done(KM_TRICKED);
                        /* we'll reach here if running in flags.debug mode */
                        fprintf(stderr, "Cannot continue this game.\n");
                }
                getlev(fd, hackpid, new_ledger, false);
                (void) close(fd);
        }
        /* do this prior to level-change pline messages */
        vision_reset();         /* clear old level's line-of-sight */
        vision_full_recalc = 0; /* don't let that reenable vision yet */
        flush_screen(-1);       /* ensure all map flushes are postponed */

        if (portal && !In_endgame(&u.uz)) {
            /* find the portal on the new level */
            struct trap *ttrap;

            for (ttrap = ftrap; ttrap; ttrap = ttrap->ntrap)
                if (ttrap->ttyp == MAGIC_PORTAL) break;

            if (!ttrap) panic("goto_level: no corresponding portal!");
            seetrap(ttrap);
            u_on_newpos(ttrap->tx, ttrap->ty);
        } else if (at_stairs && !In_endgame(&u.uz)) {
            if (up) {
                if (at_ladder) {
                    u_on_newpos(xdnladder, ydnladder);
                } else {
                    if (newdungeon) {
                        if (Is_stronghold(&u.uz)) {
                            signed char x, y;

                            do {
                                x = (COLNO - 2 - rnd(5));
                                y = rn1(ROWNO - 4, 3);
                            } while(occupied(x, y) ||
                                    IS_WALL(levl[x][y].typ));
                            u_on_newpos(x, y);
                        } else u_on_sstairs();
                    } else u_on_dnstairs();
                }
                /* Remove bug which crashes with levitation/punishment  KAA */
                if (Punished && !Levitation) {
                        pline("With great effort you climb the %s.",
                                at_ladder ? "ladder" : "stairs");
                } else if (at_ladder)
                    You("climb up the ladder.");
            } else {    /* down */
                if (at_ladder) {
                    u_on_newpos(xupladder, yupladder);
                } else {
                    if (newdungeon) u_on_sstairs();
                    else u_on_upstairs();
                }
                if (u.delta.z && Flying)
                    You("fly down along the %s.",
                        at_ladder ? "ladder" : "stairs");
                else if (u.delta.z &&
                    (near_capacity() > UNENCUMBERED || Punished || Fumbling())) {
                    You("fall down the %s.", at_ladder ? "ladder" : "stairs");
                    if (Punished) {
                        drag_down();
                        if (carried(uball)) {
                            if (uwep == uball)
                                setuwep((struct obj *)0);
                            if (uswapwep == uball)
                                setuswapwep((struct obj *)0);
                            if (uquiver == uball)
                                setuqwep((struct obj *)0);
                            freeinv(uball);
                        }
                    }
                    /* falling off steed has its own losehp() call */
                    if (u.usteed) {
                        dismount_steed(DISMOUNT_FELL);
                    } else {
                        losehp(rnd(3), killed_by_const(KM_FALLING_DOWNSTAIRS));
                    }
                    selftouch("Falling, you");
                } else if (u.delta.z && at_ladder)
                    You("climb down the ladder.");
            }
        } else {        /* trap door or level_tele or In_endgame */
            if (was_in_W_tower && On_W_tower_level(&u.uz))
                /* Stay inside the Wizard's tower when feasible.        */
                /* Note: up vs down doesn't really matter in this case. */
                place_lregion(dndest.nlx, dndest.nly,
                                dndest.nhx, dndest.nhy,
                                0,0, 0,0, LR_DOWNTELE, (d_level *) 0);
            else if (up)
                place_lregion(updest.lx, updest.ly,
                                updest.hx, updest.hy,
                                updest.nlx, updest.nly,
                                updest.nhx, updest.nhy,
                                LR_UPTELE, (d_level *) 0);
            else
                place_lregion(dndest.lx, dndest.ly,
                                dndest.hx, dndest.hy,
                                dndest.nlx, dndest.nly,
                                dndest.nhx, dndest.nhy,
                                LR_DOWNTELE, (d_level *) 0);
            if (falling) {
                if (Punished) ballfall();
                selftouch("Falling, you");
            }
        }

        if (Punished) placebc();
        obj_delivery();         /* before killing geno'd monsters' eggs */
        losedogs();
        kill_genocided_monsters();  /* for those wiped out while in limbo */
        /*
         * Expire all timers that have gone off while away.  Must be
         * after migrating monsters and objects are delivered
         * (losedogs and obj_delivery).
         */
        run_timers();

        initrack();

        if ((mtmp = m_at(u.ux, u.uy)) != 0
                && mtmp != u.usteed
                ) {
            /* There's a monster at your target destination; it might be one
               which accompanied you--see mon_arrive(dogmove.c)--or perhaps
               it was already here.  Randomly move you to an adjacent spot
               or else the monster to any nearby location.  Prior to 3.3.0
               the latter was done unconditionally. */
            coord cc;

            if (!rn2(2) &&
                    enexto(&cc, u.ux, u.uy, youmonst.data) &&
                    distu(cc.x, cc.y) <= 2)
                u_on_newpos(cc.x, cc.y);        /*[maybe give message here?]*/
            else
                mnexto(mtmp);

            if ((mtmp = m_at(u.ux, u.uy)) != 0) {
                impossible("mnexto failed (do.c)?");
                (void) rloc(mtmp, false);
            }
        }

        /* initial movement of bubbles just before vision_recalc */
        if (Is_waterlevel(&u.uz))
                movebubbles();

        if (level_info[new_ledger].flags & FORGOTTEN) {
            forget_map(ALL_MAP);        /* forget the map */
            forget_traps();             /* forget all traps too */
            familiar = true;
            level_info[new_ledger].flags &= ~FORGOTTEN;
        }

        /* Reset the screen. */
        vision_reset();         /* reset the blockages */
        docrt();                /* does a full vision recalc */
        flush_screen(-1);

        /*
         *  Move all plines beyond the screen reset.
         */

        /* give room entrance message, if any */
        check_special_room(false);

        /* Check whether we just entered Gehennom. */
        if (!In_hell(&u.uz0) && Inhell) {
            if (Is_valley(&u.uz)) {
                You("arrive at the Valley of the Dead...");
                pline_The("odor of burnt flesh and decay pervades the air.");
                You_hear("groans and moans everywhere.");
            } else pline("It is hot here.  You smell smoke...");
        }

        if (familiar) {
            static const char * const fam_msgs[4] = {
                "You have a sense of deja vu.",
                "You feel like you've been here before.",
                "This place %s familiar...",
                0       /* no message */
            };
            static const char * const halu_fam_msgs[4] = {
                "Whoa!  Everything %s different.",
                "You are surrounded by twisty little passages, all alike.",
                "Gee, this %s like uncle Conan's place...",
                0       /* no message */
            };
            const char *mesg;
            char buf[BUFSZ];
            int which = rn2(4);

            if (Hallucination())
                mesg = halu_fam_msgs[which];
            else
                mesg = fam_msgs[which];
            if (mesg && index(mesg, '%')) {
                sprintf(buf, mesg, !Blind ? "looks" : "seems");
                mesg = buf;
            }
            if (mesg) plines(mesg);
        }

        /* Final confrontation */
        if (In_endgame(&u.uz) && newdungeon && u.uhave.amulet)
                resurrect();
        if (newdungeon && In_V_tower(&u.uz) && In_hell(&u.uz0))
                pline_The("heat and smoke are gone.");

        /* the message from your quest leader */
        if (!In_quest(&u.uz0) && at_dgn_entrance("The Quest") &&
                !(u.uevent.qexpelled || u.uevent.qcompleted || quest_status.leader_is_dead)) {

                if (u.uevent.qcalled) {
                        com_pager(Role_if(PM_ROGUE) ? 4 : 3);
                } else {
                        com_pager(2);
                        u.uevent.qcalled = true;
                }
        }

        /* once Croesus is dead, his alarm doesn't work any more */
        if (Is_knox(&u.uz) && (new || !mvitals[PM_CROESUS].died)) {
                You("penetrated a high security area!");
                pline("An alarm sounds!");
                for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
                    if (!DEADMONSTER(mtmp) && mtmp->msleeping) mtmp->msleeping = 0;
        }

        if (on_level(&u.uz, &astral_level))
            final_level();
        else
            onquest();
        assign_level(&u.uz0, &u.uz); /* reset u.uz0 */

        save_currentstate();

        /* assume this will always return true when changing level */
        (void) in_out_region(u.ux, u.uy);
        (void) pickup(1);
}

static void final_level (void) {
        struct monst *mtmp;
        struct obj *otmp;
        coord mm;
        int i;

        /* reset monster hostility relative to player */
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
            if (!DEADMONSTER(mtmp)) reset_hostility(mtmp);

        /* create some player-monsters */
        create_mplayers(rn1(4, 3), true);

        /* create a guardian angel next to player, if worthy */
        if (Conflict) {
            pline(
             "A voice booms: \"Thy desire for conflict shall be fulfilled!\"");
            for (i = rnd(4); i > 0; --i) {
                mm.x = u.ux;
                mm.y = u.uy;
                if (enexto(&mm, mm.x, mm.y, &mons[PM_ANGEL]))
                    (void) mk_roamer(&mons[PM_ANGEL], u.ualign.type,
                                     mm.x, mm.y, false);
            }

        } else if (u.ualign.record > 8) {       /* fervent */
            pline("A voice whispers: \"Thou hast been worthy of me!\"");
            mm.x = u.ux;
            mm.y = u.uy;
            if (enexto(&mm, mm.x, mm.y, &mons[PM_ANGEL])) {
                if ((mtmp = mk_roamer(&mons[PM_ANGEL], u.ualign.type,
                                      mm.x, mm.y, true)) != 0) {
                    if (!Blind)
                        pline("An angel appears near you.");
                    else
                        You_feel("the presence of a friendly angel near you.");
                    /* guardian angel -- the one case mtame doesn't
                     * imply an edog structure, so we don't want to
                     * call tamedog().
                     */
                    mtmp->mtame = 10;
                    /* make him strong enough vs. endgame foes */
                    mtmp->m_lev = rn1(8,15);
                    mtmp->mhp = mtmp->mhpmax =
                                        d((int)mtmp->m_lev,10) + 30 + rnd(30);
                    if ((otmp = select_hwep(mtmp)) == 0) {
                        otmp = mksobj(SILVER_SABER, false, false);
                        if (mpickobj(mtmp, otmp))
                            panic("merged weapon?");
                    }
                    bless(otmp);
                    if (otmp->spe < 4) otmp->spe += rnd(4);
                    if ((otmp = which_armor(mtmp, W_ARMS)) == 0 ||
                            otmp->otyp != SHIELD_OF_REFLECTION) {
                        (void) mongets(mtmp, AMULET_OF_REFLECTION);
                        m_dowear(mtmp, true);
                    }
                }
            }
        }
}

/* change levels at the end of this turn, after monsters finish moving */
void schedule_goto(d_level *tolev, bool at_stairs, bool falling,
        int portal_flag, const char *pre_msg, const char *post_msg)
{
        int typmask = 0100;             /* non-zero triggers `deferred_goto' */

        /* destination flags (`goto_level' args) */
        if (at_stairs)   typmask |= 1;
        if (falling)     typmask |= 2;
        if (portal_flag) typmask |= 4;
        if (portal_flag < 0) typmask |= 0200;   /* flag for portal removal */
        u.utotype = typmask;
        /* destination level */
        assign_level(&u.utolev, tolev);

        if (pre_msg)
            dfr_pre_msg = strcpy((char *)malloc(strlen(pre_msg) + 1), pre_msg);
        if (post_msg)
            dfr_post_msg = strcpy((char *)malloc(strlen(post_msg)+1), post_msg);
}

/* handle something like portal ejection */
void deferred_goto (void) {
        if (!on_level(&u.uz, &u.utolev)) {
            d_level dest;
            int typmask = u.utotype; /* save it; goto_level zeroes u.utotype */

            assign_level(&dest, &u.utolev);
            if (dfr_pre_msg) plines(dfr_pre_msg);
            goto_level(&dest, !!(typmask&1), !!(typmask&2), !!(typmask&4));
            if (typmask & 0200) {       /* remove portal */
                struct trap *t = t_at(u.ux, u.uy);

                if (t) {
                    deltrap(t);
                    newsym(u.ux, u.uy);
                }
            }
            if (dfr_post_msg) plines(dfr_post_msg);
        }
        u.utotype = 0;          /* our caller keys off of this */
        if (dfr_pre_msg)
            free((void *)dfr_pre_msg),  dfr_pre_msg = 0;
        if (dfr_post_msg)
            free((void *)dfr_post_msg),  dfr_post_msg = 0;
}

/*
 * Return true if we created a monster for the corpse.  If successful, the
 * corpse is gone.
 */
bool revive_corpse(struct obj *corpse) {
    struct monst *mtmp, *mcarry;
    bool is_uwep, chewed;
    signed char where;
    char *cname, cname_buf[BUFSZ];
    struct obj *container = (struct obj *)0;
    int container_where = 0;

    where = corpse->where;
    is_uwep = corpse == uwep;
    cname = eos(strcpy(cname_buf, "bite-covered "));
    strcpy(cname, corpse_xname(corpse, true));
    mcarry = (where == OBJ_MINVENT) ? corpse->ocarry : 0;

    if (where == OBJ_CONTAINED) {
        struct monst *mtmp2 = (struct monst *)0;
        container = corpse->ocontainer;
        mtmp2 = get_container_location(container, &container_where, (int *)0);
        /* container_where is the outermost container's location even if nested */
        if (container_where == OBJ_MINVENT && mtmp2) mcarry = mtmp2;
    }
    mtmp = revive(corpse);      /* corpse is gone if successful */

    if (mtmp) {
        chewed = (mtmp->mhp < mtmp->mhpmax);
        if (chewed) cname = cname_buf;  /* include "bite-covered" prefix */
        switch (where) {
            case OBJ_INVENT:
                if (is_uwep)
                    pline_The("%s writhes out of your grasp!", cname);
                else
                    You_feel("squirming in your backpack!");
                break;

            case OBJ_FLOOR:
                if (cansee(mtmp->mx, mtmp->my)) {
                    char name[BUFSZ];
                    if (chewed) {
                        Adjmonnam(name, BUFSZ, mtmp, "bite-covered");
                    } else {
                        Monnam(name, BUFSZ, mtmp);
                    }
                    pline("%s rises from the dead!", name);
                }
                break;

            case OBJ_MINVENT:           /* probably a nymph's */
                if (cansee(mtmp->mx, mtmp->my)) {
                    if (canseemon(mcarry)) {
                        char name[BUFSZ];
                        mon_nam(name, BUFSZ, mcarry);
                        pline("Startled, %s drops %s as it revives!", name, an(cname));
                    } else {
                        char name[BUFSZ];
                        if (chewed) {
                            Adjmonnam(name, BUFSZ, mtmp, "bite-covered");
                        } else {
                            Monnam(name, BUFSZ, mtmp);
                        }
                        pline("%s suddenly appears!", name);
                    }
                }
                break;
           case OBJ_CONTAINED:
                if (container_where == OBJ_MINVENT && cansee(mtmp->mx, mtmp->my) &&
                    mcarry && canseemon(mcarry) && container)
                {
                    char subject[BUFSZ];
                    Amonnam(subject, BUFSZ, mtmp);
                    char name[BUFSZ];
                    mon_nam(name, BUFSZ, mcarry);
                    pline("%s writhes out of %s%s %s!", subject,
                            name, possessive_suffix(name), xname(container));
                } else if (container_where == OBJ_INVENT && container) {
                        char sackname[BUFSZ];
                        nh_strlcpy(sackname, an(xname(container)), BUFSZ);
                        const char *subject;
                        if (Blind) {
                            subject = Something;
                        } else {
                            char name[BUFSZ];
                            Amonnam(name, BUFSZ, mtmp);
                            subject = name;
                        }
                        pline("%s %s out of %s in your pack!", subject,
                                locomotion(mtmp->data,"writhes"), sackname);
                } else if (container_where == OBJ_FLOOR && container &&
                            cansee(mtmp->mx, mtmp->my))
                {
                        char sackname[BUFSZ];
                        nh_strlcpy(sackname, an(xname(container)), BUFSZ);
                        char name[BUFSZ];
                        Amonnam(name, BUFSZ, mtmp);
                        pline("%s escapes from %s!", name, sackname);
                }
                break;
            default:
                /* we should be able to handle the other cases... */
                impossible("revive_corpse: lost corpse @ %d", where);
                break;
        }
        return true;
    }
    return false;
}

/* Revive the corpse via a timeout. */
void revive_mon(void *arg, long timeout) {
    struct obj *body = (struct obj *) arg;

    // if we succeed, the corpse is gone, otherwise, rot it away
    if (!revive_corpse(body)) {
        if (is_rider(&mons[body->corpsenm]))
            You_feel("less hassled.");
        start_timer(250L - (monstermoves-body->age), TIMER_OBJECT, ROT_CORPSE, arg);
    }
}

int donull (void) {
    return(1);      /* Do nothing, but let other things happen */
}


static int wipeoff (void) {
        if(u.ucreamed < 4)      u.ucreamed = 0;
        else                    u.ucreamed -= 4;
        if (Blinded < 4)        Blinded = 0;
        else                    Blinded -= 4;
        if (!Blinded) {
                pline("You've got the glop off.");
                u.ucreamed = 0;
                Blinded = 1;
                make_blinded(0L,true);
                return(0);
        } else if (!u.ucreamed) {
                Your("%s feels clean now.", body_part(FACE));
                return(0);
        }
        return(1);              /* still busy */
}

int dowipe (void) {
        if(u.ucreamed)  {
                static char buf[39];

                sprintf(buf, "wiping off your %s", body_part(FACE));
                set_occupation(wipeoff, buf);
                /* Not totally correct; what if they change back after now
                 * but before they're finished wiping?
                 */
                return(1);
        }
        Your("%s is already clean.", body_part(FACE));
        return(1);
}

void set_wounded_legs (long side, int timex) {
        /* KMH -- STEED
         * If you are riding, your steed gets the wounded legs instead.
         * You still call this function, but don't lose hp.
         * Caller is also responsible for adjusting messages.
         */

        if(!Wounded_legs()) {
                ATEMP(A_DEX)--;
        }

        if(!Wounded_legs() || (get_HWounded_legs() & TIMEOUT))
            set_HWounded_legs(timex);
        set_EWounded_legs(side);
        (void)encumber_msg();
}

void heal_legs (void) {
        if(Wounded_legs()) {
                if (ATEMP(A_DEX) < 0) {
                        ATEMP(A_DEX)++;
                }

                if (!u.usteed)
                {
                        /* KMH, intrinsics patch */
                        if((get_EWounded_legs() & BOTH_SIDES) == BOTH_SIDES) {
                        Your("%s feel somewhat better.",
                                makeplural(body_part(LEG)));
                } else {
                        Your("%s feels somewhat better.",
                                body_part(LEG));
                }
                }
                set_HWounded_legs(0);
                set_EWounded_legs(0);
        }
        (void)encumber_msg();
}
