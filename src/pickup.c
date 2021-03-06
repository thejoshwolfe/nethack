/* See LICENSE in the root of this project for change info */
/*
 *      Contains code for picking objects up, and container use.
 */

#include "pickup.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rm_util.h"
#include "display_util.h"
#include "dungeon_util.h"
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
#include "drawing.h"
#include "dungeon.h"
#include "end.h"
#include "engrave.h"
#include "flag.h"
#include "global.h"
#include "hack.h"
#include "hacklib.h"
#include "invent.h"
#include "light.h"
#include "makemon.h"
#include "mkobj.h"
#include "mkroom.h"
#include "mondata.h"
#include "monst.h"
#include "obj.h"
#include "objclass.h"
#include "objnam.h"
#include "onames.h"
#include "permonst.h"
#include "pline.h"
#include "pm.h"
#include "polyself.h"
#include "prop.h"
#include "rm.h"
#include "rnd.h"
#include "shk.h"
#include "skills.h"
#include "steed.h"
#include "timeout.h"
#include "trap.h"
#include "util.h"
#include "wield.h"
#include "wintype.h"
#include "worn.h"
#include "you.h"
#include "youprop.h"
#include "zap.h"

/* define for query_objlist() and autopickup() */
#define FOLLOW(curr, flags) \
    (((flags) & BY_NEXTHERE) ? (curr)->nexthere : (curr)->nobj)

/*
 *  How much the weight of the given container will change when the given
 *  object is removed from it.  This calculation must match the one used
 *  by weight() in mkobj.c.
 */
#define DELTA_CWT(cont,obj)             \
    ((cont)->cursed ? (obj)->owt * 2 :  \
                      1 + ((obj)->owt / ((cont)->blessed ? 4 : 2)))
#define GOLD_WT(n)              (((n) + 50L) / 100L)
/* if you can figure this out, give yourself a hearty pat on the back... */
#define GOLD_CAPACITY(w,n)      (((w) * -100L) - ((n) + 50L) - 1L)

static const char moderateloadmsg[] = "You have a little trouble lifting";
static const char nearloadmsg[] = "You have much trouble lifting";
static const char overloadmsg[] = "You have extreme difficulty lifting";

/* List of valid menu classes for query_objlist() and allow_category callback */
static char valid_menu_classes[MAXOCLASSES + 2];

/* A variable set in use_container(), to be used by the callback routines   */
/* in_container(), and out_container() from askchain() and use_container(). */
static struct obj *current_container;

int collect_obj_classes (char ilets[], struct obj *otmp, bool here,
        bool incl_gold, bool (*filter)(const struct obj *), int *itemcount)
{
        int iletct = 0;
        char c;

        *itemcount = 0;
        if (incl_gold)
            ilets[iletct++] = def_oc_syms[COIN_CLASS];
        ilets[iletct] = '\0'; /* terminate ilets so that index() will work */
        while (otmp) {
            c = def_oc_syms[(int)otmp->oclass];
            if (!index(ilets, c) && (!filter || (*filter)(otmp)))
                ilets[iletct++] = c,  ilets[iletct] = '\0';
            *itemcount += 1;
            otmp = here ? otmp->nexthere : otmp->nobj;
        }

        return iletct;
}

void add_valid_menu_class (int c) {
        static int vmc_count = 0;

        if (c == 0)  /* reset */
          vmc_count = 0;
        else
          valid_menu_classes[vmc_count++] = (char)c;
        valid_menu_classes[vmc_count] = '\0';
}

/* query_objlist callback: return true if not uchain */
static bool all_but_uchain (const struct obj *obj) {
    return (obj != uchain);
}

/* query_objlist callback: return true */
bool allow_all (const struct obj *obj) {
    return true;
}

bool allow_category (const struct obj *obj) {
    struct obj *wtf_hax = (struct obj *)(void *) obj;
    if (Role_if(PM_PRIEST)) wtf_hax->bknown = true;

    if (((index(valid_menu_classes,'u') != NULL) && obj->unpaid) ||
        (index(valid_menu_classes, obj->oclass) != NULL))
        return true;
    else if (((index(valid_menu_classes,'U') != NULL) &&
        (obj->oclass != COIN_CLASS && obj->bknown && !obj->blessed && !obj->cursed)))
        return true;
    else if (((index(valid_menu_classes,'B') != NULL) &&
        (obj->oclass != COIN_CLASS && obj->bknown && obj->blessed)))
        return true;
    else if (((index(valid_menu_classes,'C') != NULL) &&
        (obj->oclass != COIN_CLASS && obj->bknown && obj->cursed)))
        return true;
    else if (((index(valid_menu_classes,'X') != NULL) &&
        (obj->oclass != COIN_CLASS && !obj->bknown)))
        return true;
    else
        return false;
}

/* query_objlist callback: return true if valid class and worn */
bool is_worn_by_type (const struct obj *otmp) {
        return((bool)(!!(otmp->owornmask &
                        (W_ARMOR | W_RING | W_AMUL | W_TOOL | W_WEP | W_SWAPWEP | W_QUIVER)))
                && (index(valid_menu_classes, otmp->oclass) != (char *)0));
}

void notice_stuff_here(void) {
    // sent to client elsewhere
}

/*
 * Have the hero pick things from the ground
 * or a monster's inventory if swallowed.
 *
 * Returns 1 if tried to pick something up, whether
 * or not it succeeded.
 */
static int pickup(void) {
    int i, n, res, n_tried = 0, n_picked = 0;
    bool autopickup = false;
    struct obj *objchain;
    int traverse_how;

    if (!u.uswallow) {
        if (notake(youmonst.data)) {
            You("are physically incapable of picking anything up.");
            return 0;
        }
    }

    add_valid_menu_class(0); /* reset */
    if (!u.uswallow) {
        objchain = level.objects[u.ux][u.uy];
        traverse_how = BY_NEXTHERE;
    } else {
        objchain = u.ustuck->minvent;
        traverse_how = 0; /* nobj */
    }
    /*
     * Start the actual pickup process.  This is split into two main
     * sections, the newer menu and the older "traditional" methods.
     * Automatic pickup has been split into its own menu-style routine
     * to make things less confusing.
     */
    menu_item * pick_list = NULL;
    /* use menus exclusively */
    int qflags = traverse_how | AUTOSELECT_SINGLE | INVORDER_SORT | FEEL_COCKATRICE;
    n = query_objlist("Pick up what?", objchain, qflags, &pick_list, PICK_ANY, all_but_uchain);
    n_tried = n;
    for (n_picked = i = 0; i < n; i++) {
        res = pickup_object(pick_list[i].item.a_obj, pick_list[i].count, false);
        if (res < 0)
            break; /* can't continue */
        n_picked += res;
    }
    if (pick_list)
        free(pick_list);

    if (!u.uswallow) {
        if (!OBJ_AT(u.ux, u.uy))
            u.uundetected = 0;

        /* position may need updating (invisible hero) */
        if (n_picked)
            newsym(u.ux, u.uy);
    }
    return n_tried > 0;
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


/*
 * Put up a menu using the given object list.  Only those objects on the
 * list that meet the approval of the allow function are displayed.  Return
 * a count of the number of items selected, as well as an allocated array of
 * menu_items, containing pointers to the objects selected and counts.  The
 * returned counts are guaranteed to be in bounds and non-zero.
 *
 * Query flags:
 *      BY_NEXTHERE       - Follow object list via nexthere instead of nobj.
 *      AUTOSELECT_SINGLE - Don't ask if only 1 object qualifies - just
 *                          use it.
 *      USE_INVLET        - Use object's invlet.
 *      INVORDER_SORT     - Use hero's pack order.
 *      SIGNAL_NOMENU     - Return -1 rather than 0 if nothing passes "allow".
 */
// const char *qstr,               /* query string */
// struct obj *olist,              /* the list to pick from */
// int qflags,                     /* options to control the query */
// menu_item **pick_list,          /* return list of items picked */
// int how,                        /* type of query */
// bool (*allow)(struct obj *)/* allow function */
int query_objlist(const char *qstr, struct obj *olist, int qflags,
        menu_item **pick_list, int how, bool (*allow)(const struct obj *))
{
        int i, j;
        int n;
        winid win;
        struct obj *curr, *last;
        struct obj **oarray;
        char *pack;
        anything any;
        bool printed_type_name;

        *pick_list = (menu_item *) 0;
        if (!olist) return 0;

        /* count the number of items allowed */
        for (n = 0, last = 0, curr = olist; curr; curr = FOLLOW(curr, qflags))
            if ((*allow)(curr)) {
                last = curr;
                n++;
            }

        if (n == 0)     /* nothing to pick here */
            return (qflags & SIGNAL_NOMENU) ? -1 : 0;

        if (n == 1 && (qflags & AUTOSELECT_SINGLE)) {
            *pick_list = (menu_item *) malloc(sizeof(menu_item));
            (*pick_list)->item.a_obj = last;
            (*pick_list)->count = last->quan;
            return 1;
        }

        /* Make a temporary array to store the objects sorted */
        oarray = (struct obj **)malloc(n*sizeof(struct obj*));

        /* Add objects to the array */
        i = 0;
        for (curr = olist; curr; curr = FOLLOW(curr, qflags)) {
          if ((*allow)(curr)) {
            if (iflags.sortloot == 'f' ||
                (iflags.sortloot == 'l' && !(qflags & USE_INVLET)))
              {
                /* Insert object at correct index */
                for (j = i; j; j--)
                  {
                    if (strcmpi(cxname2(curr), cxname2(oarray[j-1]))>0) break;
                    oarray[j] = oarray[j-1];
                  }
                oarray[j] = curr;
                i++;
              } else {
                /* Just add it to the array */
                oarray[i++] = curr;
              }
          }
        }

        win = create_nhwindow(NHW_MENU);
        start_menu(win);
        any.a_obj = (struct obj *) 0;

        /*
         * Run through the list and add the objects to the menu.  If
         * INVORDER_SORT is set, we'll run through the list once for
         * each type so we can group them.  The allow function will only
         * be called once per object in the list.
         */
        pack = flags.inv_order;
        do {
            printed_type_name = false;
            for (i = 0; i < n; i++) {
                curr = oarray[i];
                if ((qflags & FEEL_COCKATRICE) && curr->otyp == CORPSE &&
                     will_feel_cockatrice(curr, false)) {
                        destroy_nhwindow(win);  /* stop the menu and revert */
                        look_here(0);
                        return 0;
                }
                if ((!(qflags & INVORDER_SORT) || curr->oclass == *pack)
                                                        && (*allow)(curr)) {

                    /* if sorting, print type name (once only) */
                    if ((qflags & INVORDER_SORT) && !printed_type_name) {
                        any.a_obj = (struct obj *) 0;
                        add_menu(win, NO_GLYPH, &any, 0, 0, iflags.menu_headings,
                                        let_to_name(*pack, false), MENU_UNSELECTED);
                        printed_type_name = true;
                    }

                    any.a_obj = curr;
                    add_menu(win, obj_to_glyph(curr), &any,
                            qflags & USE_INVLET ? curr->invlet : 0,
                            def_oc_syms[(int)objects[curr->otyp].oc_class],
                            ATR_NONE, doname(curr), MENU_UNSELECTED);
                }
            }
            pack++;
        } while ((qflags & INVORDER_SORT) && *pack);

        free(oarray);
        end_menu(win, qstr);
        n = select_menu(win, how, pick_list);
        destroy_nhwindow(win);

        if (n > 0) {
            menu_item *mi;
            int i;

            /* fix up counts:  -1 means no count used => pick all */
            for (i = 0, mi = *pick_list; i < n; i++, mi++)
                if (mi->count == -1L || mi->count > mi->item.a_obj->quan)
                    mi->count = mi->item.a_obj->quan;
        } else if (n < 0) {
            n = 0;      /* caller's don't expect -1 */
        }
        return n;
}

static int count_categories (struct obj *olist, int qflags) {
    char *pack;
    bool counted_category;
    int ccount = 0;
    struct obj *curr;

    pack = flags.inv_order;
    do {
        counted_category = false;
        for (curr = olist; curr; curr = FOLLOW(curr, qflags)) {
            if (curr->oclass == *pack) {
                if ((qflags & WORN_TYPES) &&
                        !(curr->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL |
                                W_WEP | W_SWAPWEP | W_QUIVER)))
                    continue;
                if (!counted_category) {
                    ccount++;
                    counted_category = true;
                }
            }
        }
        pack++;
    } while (*pack);
    return ccount;
}


/*
 * allow menu-based category (class) selection (for Drop,take off etc.)
 *
 */
// const char *qstr,               /* query string */
// struct obj *olist,              /* the list to pick from */
// int qflags,                     /* behaviour modification flags */
// menu_item **pick_list,          /* return list of items picked */
// int how                        /* type of query */
int query_category ( const char *qstr, struct obj *olist, int qflags,
        menu_item **pick_list, int how)
{
    int n;
    winid win;
    struct obj *curr;
    char *pack;
    anything any;
    bool collected_type_name;
    char invlet;
    int ccount;
    bool do_unpaid = false;
    bool do_blessed = false, do_cursed = false, do_uncursed = false,
         do_buc_unknown = false;
    int num_buc_types = 0;

    *pick_list = (menu_item *) 0;
    if (!olist) return 0;
    if ((qflags & UNPAID_TYPES) && count_unpaid(olist)) do_unpaid = true;
    if ((qflags & BUC_BLESSED) && count_buc(olist, BUC_BLESSED)) {
        do_blessed = true;
        num_buc_types++;
    }
    if ((qflags & BUC_CURSED) && count_buc(olist, BUC_CURSED)) {
        do_cursed = true;
        num_buc_types++;
    }
    if ((qflags & BUC_UNCURSED) && count_buc(olist, BUC_UNCURSED)) {
        do_uncursed = true;
        num_buc_types++;
    }
    if ((qflags & BUC_UNKNOWN) && count_buc(olist, BUC_UNKNOWN)) {
        do_buc_unknown = true;
        num_buc_types++;
    }

    ccount = count_categories(olist, qflags);
    /* no point in actually showing a menu for a single category */
    if (ccount == 1 && !do_unpaid && num_buc_types <= 1 && !(qflags & BILLED_TYPES)) {
        for (curr = olist; curr; curr = FOLLOW(curr, qflags)) {
            if ((qflags & WORN_TYPES) &&
                    !(curr->owornmask & (W_ARMOR|W_RING|W_AMUL|W_TOOL|W_WEP|W_SWAPWEP|W_QUIVER)))
                continue;
            break;
        }
        if (curr) {
            *pick_list = (menu_item *) malloc(sizeof(menu_item));
            (*pick_list)->item.a_int = curr->oclass;
            return 1;
        }
        return 0;
    }

    win = create_nhwindow(NHW_MENU);
    start_menu(win);
    pack = flags.inv_order;
    if ((qflags & ALL_TYPES) && (ccount > 1)) {
        invlet = 'a';
        any.a_void = 0;
        any.a_int = ALL_TYPES_SELECTED;
        add_menu(win, NO_GLYPH, &any, invlet, 0, ATR_NONE,
                (qflags & WORN_TYPES) ? "All worn types" : "All types",
                MENU_UNSELECTED);
        invlet = 'b';
    } else
        invlet = 'a';
    do {
        collected_type_name = false;
        for (curr = olist; curr; curr = FOLLOW(curr, qflags)) {
            if (curr->oclass == *pack) {
                if ((qflags & WORN_TYPES) &&
                        !(curr->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL |
                                W_WEP | W_SWAPWEP | W_QUIVER)))
                    continue;
                if (!collected_type_name) {
                    any.a_void = 0;
                    any.a_int = curr->oclass;
                    add_menu(win, NO_GLYPH, &any, invlet++,
                            def_oc_syms[(int)objects[curr->otyp].oc_class],
                            ATR_NONE, let_to_name(*pack, false),
                            MENU_UNSELECTED);
                    collected_type_name = true;
                }
            }
        }
        pack++;
        if (invlet >= 'u') {
            impossible("query_category: too many categories");
            return 0;
        }
    } while (*pack);
    /* unpaid items if there are any */
    if (do_unpaid) {
        invlet = 'u';
        any.a_void = 0;
        any.a_int = 'u';
        add_menu(win, NO_GLYPH, &any, invlet, 0, ATR_NONE,
                "Unpaid items", MENU_UNSELECTED);
    }
    /* billed items: checked by caller, so always include if BILLED_TYPES */
    if (qflags & BILLED_TYPES) {
        invlet = 'x';
        any.a_void = 0;
        any.a_int = 'x';
        add_menu(win, NO_GLYPH, &any, invlet, 0, ATR_NONE,
                "Unpaid items already used up", MENU_UNSELECTED);
    }
    if (qflags & CHOOSE_ALL) {
        invlet = 'A';
        any.a_void = 0;
        any.a_int = 'A';
        add_menu(win, NO_GLYPH, &any, invlet, 0, ATR_NONE,
                (qflags & WORN_TYPES) ?
                "Auto-select every item being worn" :
                "Auto-select every item", MENU_UNSELECTED);
    }
    /* items with b/u/c/unknown if there are any */
    if (do_blessed) {
        invlet = 'B';
        any.a_void = 0;
        any.a_int = 'B';
        add_menu(win, NO_GLYPH, &any, invlet, 0, ATR_NONE,
                "Items known to be Blessed", MENU_UNSELECTED);
    }
    if (do_cursed) {
        invlet = 'C';
        any.a_void = 0;
        any.a_int = 'C';
        add_menu(win, NO_GLYPH, &any, invlet, 0, ATR_NONE,
                "Items known to be Cursed", MENU_UNSELECTED);
    }
    if (do_uncursed) {
        invlet = 'U';
        any.a_void = 0;
        any.a_int = 'U';
        add_menu(win, NO_GLYPH, &any, invlet, 0, ATR_NONE,
                "Items known to be Uncursed", MENU_UNSELECTED);
    }
    if (do_buc_unknown) {
        invlet = 'X';
        any.a_void = 0;
        any.a_int = 'X';
        add_menu(win, NO_GLYPH, &any, invlet, 0, ATR_NONE,
                "Items of unknown B/C/U status",
                MENU_UNSELECTED);
    }
    end_menu(win, qstr);
    n = select_menu(win, how, pick_list);
    destroy_nhwindow(win);
    if (n < 0)
        n = 0;      /* caller's don't expect -1 */
    return n;
}

/* could we carry `obj'? if not, could we carry some of it/them? */
// struct obj *container,    /* object to pick up, bag it's coming out of */
static long carry_count ( struct obj *obj, struct obj *container, long count,
        bool telekinesis, int *wt_before, int *wt_after)
{
    bool adjust_wt = container && carried(container),
         is_gold = obj->oclass == COIN_CLASS;
    int wt, iw, ow, oow;
    long qq, savequan;
    unsigned saveowt;
    const char *verb, *prefx1, *prefx2, *suffx;
    char obj_nambuf[BUFSZ], where[BUFSZ];

    savequan = obj->quan;
    saveowt = obj->owt;

    iw = max_capacity();

    if (count != savequan) {
        obj->quan = count;
        obj->owt = (unsigned)weight(obj);
    }
    wt = iw + (int)obj->owt;
    if (adjust_wt)
        wt -= (container->otyp == BAG_OF_HOLDING) ?
            (int)DELTA_CWT(container, obj) : (int)obj->owt;
    if (is_gold)        /* merged gold might affect cumulative weight */
        wt -= (GOLD_WT(u.ugold) + GOLD_WT(count) - GOLD_WT(u.ugold + count));
    if (count != savequan) {
        obj->quan = savequan;
        obj->owt = saveowt;
    }
    *wt_before = iw;
    *wt_after  = wt;

    if (wt < 0)
        return count;

    /* see how many we can lift */
    if (is_gold) {
        iw -= (int)GOLD_WT(u.ugold);
        if (!adjust_wt) {
            qq = GOLD_CAPACITY((long)iw, u.ugold);
        } else {
            oow = 0;
            qq = 50L - (u.ugold % 100L) - 1L;
            if (qq < 0L) qq += 100L;
            for ( ; qq <= count; qq += 100L) {
                obj->quan = qq;
                obj->owt = (unsigned)GOLD_WT(qq);
                ow = (int)GOLD_WT(u.ugold + qq);
                ow -= (container->otyp == BAG_OF_HOLDING) ?
                    (int)DELTA_CWT(container, obj) : (int)obj->owt;
                if (iw + ow >= 0) break;
                oow = ow;
            }
            iw -= oow;
            qq -= 100L;
        }
        if (qq < 0L) qq = 0L;
        else if (qq > count) qq = count;
        wt = iw + (int)GOLD_WT(u.ugold + qq);
    } else if (count > 1 || count < obj->quan) {
        /*
         * Ugh. Calc num to lift by changing the quan of of the
         * object and calling weight.
         *
         * This works for containers only because containers
         * don't merge.         -dean
         */
        for (qq = 1L; qq <= count; qq++) {
            obj->quan = qq;
            obj->owt = (unsigned)(ow = weight(obj));
            if (adjust_wt)
                ow -= (container->otyp == BAG_OF_HOLDING) ?
                    (int)DELTA_CWT(container, obj) : (int)obj->owt;
            if (iw + ow >= 0)
                break;
            wt = iw + ow;
        }
        --qq;
    } else {
        /* there's only one, and we can't lift it */
        qq = 0L;
    }
    obj->quan = savequan;
    obj->owt = saveowt;

    if (qq < count) {
        /* some message will be given */
        strcpy(obj_nambuf, doname(obj));
        if (container) {
            sprintf(where, "in %s", the(xname(container)));
            verb = "carry";
        } else {
            strcpy(where, "lying here");
            verb = telekinesis ? "acquire" : "lift";
        }
    } else {
        /* lint supppression */
        *obj_nambuf = *where = '\0';
        verb = "";
    }
    /* we can carry qq of them */
    if (qq > 0) {
        if (qq < count)
            You("can only %s %s of the %s %s.",
                    verb, (qq == 1L) ? "one" : "some", obj_nambuf, where);
        *wt_after = wt;
        return qq;
    }

    if (!container) strcpy(where, "here");  /* slightly shorter form */
    if (invent || u.ugold) {
        prefx1 = "you cannot ";
        prefx2 = "";
        suffx  = " any more";
    } else {
        prefx1 = (obj->quan == 1L) ? "it " : "even one ";
        prefx2 = "is too heavy for you to ";
        suffx  = "";
    }
    char are_tense[BUFSZ];
    otense(are_tense, BUFSZ, obj, "are");
    There("%s %s %s, but %s%s%s%s.", are_tense, obj_nambuf, where, prefx1, prefx2, verb, suffx);

    /* *wt_after = iw; */
    return 0L;
}

/* determine whether character is able and player is willing to carry `obj' */
/* object to pick up, bag it's coming out of */
static int lift_object(struct obj *obj, struct obj *container, long *cnt_p, bool telekinesis) {
    int result, old_wt, new_wt, prev_encumbr, next_encumbr;

    if (obj->otyp == BOULDER && In_sokoban(&u.uz)) {
        You("cannot get your %s around this %s.",
                body_part(HAND), xname(obj));
        return -1;
    }
    if (obj->otyp == LOADSTONE ||
            (obj->otyp == BOULDER && throws_rocks(youmonst.data)))
        return 1;               /* lift regardless of current situation */

    *cnt_p = carry_count(obj, container, *cnt_p, telekinesis, &old_wt, &new_wt);
    if (*cnt_p < 1L) {
        result = -1;    /* nothing lifted */
    } else if (obj->oclass != COIN_CLASS && inv_cnt() >= 52 &&
            !merge_choice(invent, obj)) {
        Your("knapsack cannot accommodate any more items.");
        result = -1;    /* nothing lifted */
    } else {
        result = 1;
        prev_encumbr = near_capacity();
        if (prev_encumbr < flags.pickup_burden)
            prev_encumbr = flags.pickup_burden;
        next_encumbr = calc_capacity(new_wt - old_wt);
        if (next_encumbr > prev_encumbr) {
            if (telekinesis) {
                result = 0;     /* don't lift */
            } else {
                char qbuf[BUFSZ];
                long savequan = obj->quan;

                obj->quan = *cnt_p;
                strcpy(qbuf,
                        (next_encumbr > HVY_ENCUMBER) ? overloadmsg :
                        (next_encumbr > MOD_ENCUMBER) ? nearloadmsg :
                        moderateloadmsg);
                sprintf(eos(qbuf), " %s. Continue?",
                        safe_qbuf(qbuf, sizeof(" . Continue?"),
                            doname(obj), an(simple_typename(obj->otyp)), "something"));
                obj->quan = savequan;
                switch (ynq(qbuf)) {
                    case 'q':  result = -1; break;
                    case 'n':  result =  0; break;
                    default:   break;       /* 'y' => result == 1 */
                }
                clear_nhwindow(WIN_MESSAGE);
            }
        }
    }

    if (obj->otyp == SCR_SCARE_MONSTER && result <= 0 && !container)
        obj->spe = 0;
    return result;
}

/* To prevent qbuf overflow in prompts use planA only
 * if it fits, or planB if PlanA doesn't fit,
 * finally using the fallback as a last resort.
 * last_restort is expected to be very short.
 */
const char * safe_qbuf (const char *qbuf, unsigned padlength, const char *planA,
        const char *planB, const char *last_resort)
{
        /* convert size_t (or int for ancient systems) to ordinary unsigned */
        unsigned len_qbuf = (unsigned)strlen(qbuf),
                 len_planA = (unsigned)strlen(planA),
                 len_planB = (unsigned)strlen(planB),
                 len_lastR = (unsigned)strlen(last_resort);
        unsigned textleft = QBUFSZ - (len_qbuf + padlength);

        if (len_lastR >= textleft) {
            impossible("safe_qbuf: last_resort too large at %u characters.",
                       len_lastR);
            return "";
        }
        return (len_planA < textleft) ? planA :
                    (len_planB < textleft) ? planB : last_resort;
}

/*
 * Pick up <count> of obj from the ground and add it to the hero's inventory.
 * Returns -1 if caller should break out of its loop, 0 if nothing picked
 * up, 1 if otherwise.
 */
// bool telekinesis    /* not picking it up directly by hand */
int pickup_object ( struct obj *obj, long count, bool telekinesis) {
    int res, nearload;
    const char *where = (obj->ox == u.ux && obj->oy == u.uy) ?
        "here" : "there";

    if (obj->quan < count) {
        impossible("pickup_object: count %ld > quan %ld?",
                count, obj->quan);
        return 0;
    }

    /* In case of auto-pickup, where we haven't had a chance
       to look at it yet; affects docall(SCR_SCARE_MONSTER). */
    if (!Blind())
        obj->dknown = 1;

    if (obj == uchain) {    /* do not pick up attached chain */
        return 0;
    } else if (obj->oartifact && !touch_artifact(obj,&youmonst)) {
        return 0;
    } else if (obj->oclass == COIN_CLASS) {
        /* Special consideration for gold pieces... */
        long iw = (long)max_capacity() - GOLD_WT(u.ugold);
        long gold_capacity = GOLD_CAPACITY(iw, u.ugold);

        if (gold_capacity <= 0L) {
            char are_tense[BUFSZ];
            otense(are_tense, BUFSZ, obj, "are");
            pline( "There %s %ld gold piece%s %s, but you cannot carry any more.", are_tense,
                    obj->quan, plur(obj->quan), where);
            return 0;
        } else if (gold_capacity < count) {
            You("can only %s %s of the %ld gold pieces lying %s.",
                    telekinesis ? "acquire" : "carry",
                    gold_capacity == 1L ? "one" : "some", obj->quan, where);
            pline("%s %ld gold piece%s.",
                    nearloadmsg, gold_capacity, plur(gold_capacity));
            u.ugold += gold_capacity;
            obj->quan -= gold_capacity;
            costly_gold(obj->ox, obj->oy, gold_capacity);
        } else {
            u.ugold += count;
            if ((nearload = near_capacity()) != 0)
                pline("%s %ld gold piece%s.",
                        nearload < MOD_ENCUMBER ?
                        moderateloadmsg : nearloadmsg,
                        count, plur(count));
            else
                prinv((char *) 0, obj, count);
            costly_gold(obj->ox, obj->oy, count);
            if (count == obj->quan)
                delobj(obj);
            else
                obj->quan -= count;
        }
        return 1;
    } else if (obj->otyp == CORPSE) {
        if ( (touch_petrifies(&mons[obj->corpsenm])) && !uarmg
                && !Stone_resistance() && !telekinesis) {
            if (poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM))
                display_nhwindow(WIN_MESSAGE, false);
            else {
                char kbuf[BUFSZ];

                strcpy(kbuf, an(corpse_xname(obj, true)));
                pline("Touching %s is a fatal mistake.", kbuf);
                instapetrify(kbuf);
                return -1;
            }
        } else if (is_rider(&mons[obj->corpsenm])) {
            pline("At your %s, the corpse suddenly moves...",
                    telekinesis ? "attempted acquisition" : "touch");
            (void) revive_corpse(obj);
            exercise(A_WIS, false);
            return -1;
        }
    } else  if (obj->otyp == SCR_SCARE_MONSTER) {
        if (obj->blessed) {
            obj->blessed = 0;
        } else if (!obj->spe && !obj->cursed) {
            obj->spe = 1;
        } else {
            char turn_tense[BUFSZ];
            otense(turn_tense, BUFSZ, obj, "turn");
            pline_The("scroll%s %s to dust as you %s %s up.",
                    plur(obj->quan), turn_tense,
                    telekinesis ? "raise" : "pick",
                    (obj->quan == 1L) ? "it" : "them");
            if (!(objects[SCR_SCARE_MONSTER].oc_name_known) &&
                    !(objects[SCR_SCARE_MONSTER].oc_uname))
            {
                docall(obj);
            }
            useupf(obj, obj->quan);
            return 1;       /* tried to pick something up and failed, but
                               don't want to terminate pickup loop yet   */
        }
    }

    if ((res = lift_object(obj, (struct obj *)0, &count, telekinesis)) <= 0)
        return res;

    if (obj->quan != count && obj->otyp != LOADSTONE)
        obj = splitobj(obj, count);

    obj = pick_obj(obj);

    if (uwep && uwep == obj) mrg_to_wielded = true;
    nearload = near_capacity();
    prinv(nearload == SLT_ENCUMBER ? moderateloadmsg : (char *) 0,
            obj, count);
    mrg_to_wielded = false;
    return 1;
}

/*
 * Do the actual work of picking otmp from the floor or monster's interior
 * and putting it in the hero's inventory.  Take care of billing.  Return a
 * pointer to the object where otmp ends up.  This may be different
 * from otmp because of merging.
 *
 * Gold never reaches this routine.
 */
struct obj * pick_obj (struct obj *otmp) {
    obj_extract_self(otmp);
    if (!u.uswallow && otmp != uball && costly_spot(otmp->ox, otmp->oy)) {
        char saveushops[5], fakeshop[2];

        /* addtobill cares about your location rather than the object's;
           usually they'll be the same, but not when using telekinesis
           (if ever implemented) or a grappling hook */
        strcpy(saveushops, u.ushops);
        fakeshop[0] = *in_rooms(otmp->ox, otmp->oy, SHOPBASE);
        fakeshop[1] = '\0';
        strcpy(u.ushops, fakeshop);
        /* sets obj->unpaid if necessary */
        addtobill(otmp, true, false, false);
        strcpy(u.ushops, saveushops);
        /* if you're outside the shop, make shk notice */
        if (!index(u.ushops, *fakeshop))
            remote_burglary(otmp->ox, otmp->oy);
    }
    if (otmp->no_charge)    /* only applies to objects outside invent */
        otmp->no_charge = 0;
    newsym(otmp->ox, otmp->oy);
    return addinv(otmp);    /* might merge it with other objects */
}

/*
 * prints a message if encumbrance changed since the last check and
 * returns the new encumbrance value (from near_capacity()).
 */
int encumber_msg (void) {
    static int oldcap = UNENCUMBERED;
    int newcap = near_capacity();

    if(oldcap < newcap) {
        switch(newcap) {
            case 1: Your("movements are slowed slightly because of your load.");
                    break;
            case 2: You("rebalance your load.  Movement is difficult.");
                    break;
            case 3: You("%s under your heavy load.  Movement is very hard.",
                            stagger(youmonst.data, "stagger"));
                    break;
            default: You("%s move a handspan with this load!",
                             newcap == 4 ? "can barely" : "can't even");
                     break;
        }
    } else if(oldcap > newcap) {
        switch(newcap) {
            case 0: Your("movements are now unencumbered.");
                    break;
            case 1: Your("movements are only slowed slightly by your load.");
                    break;
            case 2: You("rebalance your load.  Movement is still difficult.");
                    break;
            case 3: You("%s under your load.  Movement is still very hard.",
                            stagger(youmonst.data, "stagger"));
                    break;
        }
    }

    oldcap = newcap;
    return (newcap);
}

/* Is there a container at x,y. Optional: return count of containers at x,y */
static int container_at (int x, int y, bool countem) {
    struct obj *cobj, *nobj;
    int container_count = 0;

    for(cobj = level.objects[x][y]; cobj; cobj = nobj) {
        nobj = cobj->nexthere;
        if(Is_container(cobj)) {
            container_count++;
            if (!countem) break;
        }
    }
    return container_count;
}

static bool able_to_loot (int x, int y) {
    if (!can_reach_floor()) {
        if (u.usteed && P_SKILL(P_RIDING) < P_BASIC)
            rider_cant_reach(); /* not skilled enough to reach */
        else
            You("cannot reach the %s.", surface(x, y));
        return false;
    } else if (is_pool(x, y) || is_lava(x, y)) {
        /* at present, can't loot in water even when Underwater */
        You("cannot loot things that are deep in the %s.",
                is_lava(x, y) ? "lava" : "water");
        return false;
    } else if (nolimbs(youmonst.data)) {
        pline("Without limbs, you cannot loot anything.");
        return false;
    } else if (!freehand()) {
        pline("Without a free %s, you cannot loot anything.",
                body_part(HAND));
        return false;
    }
    return true;
}

static bool mon_beside(int x, int y) {
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int nx = x + i;
            int ny = y + j;
            if (isok(nx, ny) && MON_AT(nx, ny))
                return true;
        }
    }
    return false;
}

/* loot_mon() returns amount of time passed.
 */
static int loot_mon(struct monst *mtmp) {
    int c = -1;
    int timepassed = 0;
    struct obj *otmp;
    char qbuf[QBUFSZ];

    /* 3.3.1 introduced the ability to remove saddle from a steed             */
    /*  *passed_info is set to true if a loot query was given.               */
    /*  *prev_loot is set to true if something was actually acquired in here. */
    if (mtmp && mtmp != u.usteed && (otmp = which_armor(mtmp, W_SADDLE))) {
        long unwornmask;
        sprintf(qbuf, "Do you want to remove the saddle from %s?", "TODO: x_monnam");
        if ((c = yn_function(qbuf, ynqchars, 'n')) == 'y') {
            if (nolimbs(youmonst.data)) {
                You_cant("do that without limbs."); /* not body_part(HAND) */
                return 0;
            }
            if (otmp->cursed) {
                You("can't. The saddle seems to be stuck to %s.", "TODO: x_monnam");
                /* the attempt costs you time */
                return 1;
            }
            obj_extract_self(otmp);
            if ((unwornmask = otmp->owornmask) != 0L) {
                mtmp->misc_worn_check &= ~unwornmask;
                otmp->owornmask = 0L;
                update_mon_intrinsics(mtmp, otmp, false, false);
            }
            otmp = hold_another_object(otmp, "You drop %s!", doname(otmp), (const char *)0);
            timepassed = rnd(3);
        } else if (c == 'q') {
            return 0;
        }
    }
    return timepassed;
}

/* loot a container on the floor or loot saddle from mon. */
int doloot(void) {
    struct obj *cobj, *nobj;
    int c = -1;
    int timepassed = 0;
    coord cc;
    bool underfoot = true;
    const char *dont_find_anything = "don't find anything";
    struct monst *mtmp;
    char qbuf[BUFSZ];

    if (check_capacity((char *)0)) {
        /* "Can't do that while carrying so much stuff." */
        return 0;
    }
    if (nohands(youmonst.data)) {
        You("have no hands!"); /* not `body_part(HAND)' */
        return 0;
    }
    cc.x = u.ux;
    cc.y = u.uy;

    lootcont:

    if (container_at(cc.x, cc.y, false)) {
        bool any = false;

        if (!able_to_loot(cc.x, cc.y))
            return 0;
        for (cobj = level.objects[cc.x][cc.y]; cobj; cobj = nobj) {
            nobj = cobj->nexthere;

            if (Is_container(cobj)) {
                sprintf(qbuf, "There is %s here, loot it?", safe_qbuf("", sizeof("There is  here, loot it?"), doname(cobj), an(simple_typename(cobj->otyp)), "a container"));
                c = ynq(qbuf);
                if (c == 'q')
                    return timepassed;
                if (c == 'n')
                    continue;
                any = true;

                if (cobj->olocked) {
                    pline("Hmmm, it seems to be locked.");
                    continue;
                }
                if (cobj->otyp == BAG_OF_TRICKS) {
                    int tmp;
                    You("carefully open the bag...");
                    pline("It develops a huge set of teeth and bites you!");
                    tmp = rnd(10);
                    if (Half_physical_damage)
                        tmp = (tmp + 1) / 2;
                    losehp(tmp, killed_by_const(KM_CARNIVOROUS_BAG));
                    makeknown(BAG_OF_TRICKS);
                    timepassed = 1;
                    continue;
                }

                You("carefully open %s...", the(xname(cobj)));
                timepassed |= use_container(cobj, 0);
                if (multi < 0)
                    return 1; /* chest trap */
            }
        }
        if (any)
            c = 'y';
    } else if (Confusion()) {
        if (u.ugold) {
            long contribution = rnd((int)min(LARGEST_INT, u.ugold));
            struct obj *goldob = mkgoldobj(contribution);
            if (IS_THRONE(levl[u.ux][u.uy].typ)) {
                struct obj *coffers;
                int pass;
                /* find the original coffers chest, or any chest */
                for (pass = 2; pass > -1; pass -= 2)
                    for (coffers = fobj; coffers; coffers = coffers->nobj)
                        if (coffers->otyp == CHEST && coffers->spe == pass)
                            goto gotit; /* two level break */
                gotit:

                if (coffers) {
                    verbalize("Thank you for your contribution to reduce the debt.");
                    (void)add_to_container(coffers, goldob);
                    coffers->owt = weight(coffers);
                } else {
                    struct monst *mon = makemon(courtmon(), u.ux, u.uy, NO_MM_FLAGS);
                    if (mon) {
                        mon->mgold += goldob->quan;
                        delobj(goldob);
                        pline("The exchequer accepts your contribution.");
                    } else {
                        dropx(goldob);
                    }
                }
            } else {
                dropx(goldob);
                pline("Ok, now there is loot here.");
            }
        }
    } else if (IS_GRAVE(levl[cc.x][cc.y].typ)) {
        You("need to dig up the grave to effectively loot it...");
    }
    /*
     * 3.3.1 introduced directional looting for some things.
     */
    if (c != 'y' && mon_beside(u.ux, u.uy)) {
        if (!get_adjacent_loc("Loot in what direction?", "Invalid loot location", u.ux, u.uy, &cc))
            return 0;
        if (cc.x == u.ux && cc.y == u.uy) {
            underfoot = true;
            if (container_at(cc.x, cc.y, false))
                goto lootcont;
        } else {
            underfoot = false;
        }
        if (u.delta.z < 0) {
            You("%s to loot on the %s.", dont_find_anything, ceiling(cc.x, cc.y));
            timepassed = 1;
            return timepassed;
        }
        mtmp = m_at(cc.x, cc.y);
        if (mtmp)
            timepassed = loot_mon(mtmp);

        /* Preserve pre-3.3.1 behaviour for containers.
         * Adjust this if-block to allow container looting
         * from one square away to change that in the future.
         */
        if (!underfoot) {
            if (container_at(cc.x, cc.y, false)) {
                if (mtmp) {
                    message_monster(MSG_YOU_CANT_LOOT_WITH_M_IN_THE_WAY, mtmp);
                    return timepassed;
                } else {
                    You("have to be at a container to loot it.");
                }
            } else {
                //You("%s %sthere to loot.", dont_find_anything, (prev_inquiry || prev_loot) ? "else " : "");
                return timepassed;
            }
        }
    } else if (c != 'y' && c != 'n') {
        You("%s %s to loot.", dont_find_anything, underfoot ? "here" : "there");
    }
    return timepassed;
}

/*
 * Decide whether an object being placed into a magic bag will cause
 * it to explode.  If the object is a bag itself, check recursively.
 */
static bool mbag_explodes (struct obj *obj, int depthin) {
    /* these won't cause an explosion when they're empty */
    if ((obj->otyp == WAN_CANCELLATION || obj->otyp == BAG_OF_TRICKS) &&
            obj->spe <= 0)
        return false;

    /* odds: 1/1, 2/2, 3/4, 4/8, 5/16, 6/32, 7/64, 8/128, 9/128, 10/128,... */
    if ((Is_mbag(obj) || obj->otyp == WAN_CANCELLATION) &&
        (rn2(1 << (depthin > 7 ? 7 : depthin)) <= depthin))
        return true;
    else if (Has_contents(obj)) {
        struct obj *otmp;

        for (otmp = obj->cobj; otmp; otmp = otmp->nobj)
            if (mbag_explodes(otmp, depthin+1)) return true;
    }
    return false;
}

/* Returns: -1 to stop, 1 item was inserted, 0 item was not inserted. */
static int in_container (struct obj *obj) {
    bool floor_container = !carried(current_container);
    bool was_unpaid = false;
    char buf[BUFSZ];

    if (!current_container) {
        impossible("<in> no current_container?");
        return 0;
    } else if (obj == uball || obj == uchain) {
        You("must be kidding.");
        return 0;
    } else if (obj == current_container) {
        pline("That would be an interesting topological exercise.");
        return 0;
    } else if (obj->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL)) {
        Norep("You cannot %s %s you are wearing.",
                (current_container->otyp == ICE_BOX) ? "refrigerate" : "stash", something);
        return 0;
    } else if ((obj->otyp == LOADSTONE) && obj->cursed) {
        obj->bknown = 1;
        pline_The("stone%s won't leave your person.", plur(obj->quan));
        return 0;
    } else if (obj->otyp == AMULET_OF_YENDOR ||
            obj->otyp == CANDELABRUM_OF_INVOCATION ||
            obj->otyp == BELL_OF_OPENING ||
            obj->otyp == SPE_BOOK_OF_THE_DEAD) {
        /* Prohibit Amulets in containers; if you allow it, monsters can't
         * steal them.  It also becomes a pain to check to see if someone
         * has the Amulet.  Ditto for the Candelabrum, the Bell and the Book.
         */
        pline("%s cannot be confined in such trappings.", The(xname(obj)));
        return 0;
    } else if (obj->otyp == LEASH && obj->leashmon != 0) {
        message_object(MSG_O_ARE_ATTACHED_TO_PET, obj);
        return 0;
    } else if (obj == uwep) {
        if (welded(obj)) {
            weldmsg(obj);
            return 0;
        }
        setuwep(NULL);
        if (uwep) return 0;     /* unwielded, died, rewielded */
    } else if (obj == uswapwep) {
        setuswapwep(NULL);
        if (uswapwep) return 0;     /* unwielded, died, rewielded */
    } else if (obj == uquiver) {
        setuqwep(NULL);
        if (uquiver) return 0;     /* unwielded, died, rewielded */
    }

    if (obj->otyp == CORPSE) {
        if ( (touch_petrifies(&mons[obj->corpsenm])) && !uarmg
                && !Stone_resistance()) {
            if (poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM))
                display_nhwindow(WIN_MESSAGE, false);
            else {
                char kbuf[BUFSZ];

                strcpy(kbuf, an(corpse_xname(obj, true)));
                pline("Touching %s is a fatal mistake.", kbuf);
                instapetrify(kbuf);
                return -1;
            }
        }
    }

    /* boxes, boulders, and big statues can't fit into any container */
    if (obj->otyp == ICE_BOX || Is_box(obj) || obj->otyp == BOULDER ||
            (obj->otyp == STATUE && bigmonst(&mons[obj->corpsenm]))) {
        /*
         *  xname() uses a static result array.  Save obj's name
         *  before current_container's name is computed.  Don't
         *  use the result of strcpy() within You() --- the order
         *  of evaluation of the parameters is undefined.
         */
        strcpy(buf, the(xname(obj)));
        You("cannot fit %s into %s.", buf,
                the(xname(current_container)));
        return 0;
    }

    freeinv(obj);

    if (obj_is_burning(obj))        /* this used to be part of freeinv() */
        (void) snuff_lit(obj);

    if (floor_container && costly_spot(u.ux, u.uy)) {
        if (current_container->no_charge && !obj->unpaid) {
            /* don't sell when putting the item into your own container */
            obj->no_charge = 1;
        } else if (obj->oclass != COIN_CLASS) {
            /* sellobj() will take an unpaid item off the shop bill
             * note: coins are handled later */
            was_unpaid = obj->unpaid ? true : false;
            sellobj_state(SELL_DELIBERATE);
            sellobj(obj, u.ux, u.uy);
            sellobj_state(SELL_NORMAL);
        }
    }
    if ((current_container->otyp == ICE_BOX) && !age_is_relative(obj)) {
        obj->age = monstermoves - obj->age; /* actual age */
        /* stop any corpse timeouts when frozen */
        if (obj->otyp == CORPSE && obj->timed) {
            long rot_alarm = stop_timer(ROT_CORPSE, (void *)obj);
            (void) stop_timer(REVIVE_MON, (void *)obj);
            /* mark a non-reviving corpse as such */
            if (rot_alarm) obj->norevive = 1;
        }
    } else if (Is_mbag(current_container) && mbag_explodes(obj, 0)) {
        /* explicitly mention what item is triggering the explosion */
        pline(
                "As you put %s inside, you are blasted by a magical explosion!",
                doname(obj));
        /* did not actually insert obj yet */
        if (was_unpaid) addtobill(obj, false, false, true);
        obfree(obj, (struct obj *)0);
        delete_contents(current_container);
        if (!floor_container)
            useup(current_container);
        else if (obj_here(current_container, u.ux, u.uy))
            useupf(current_container, obj->quan);
        else
            panic("in_container:  bag not found.");

        losehp(d(6,6), killed_by_const(KM_MAGICAL_EXPLOSION));
        current_container = 0;  /* baggone = true; */
    }

    if (current_container) {
        strcpy(buf, the(xname(current_container)));
        You("put %s into %s.", doname(obj), buf);

        /* gold in container always needs to be added to credit */
        if (floor_container && obj->oclass == COIN_CLASS)
            sellobj(obj, current_container->ox, current_container->oy);
        (void) add_to_container(current_container, obj);
        current_container->owt = weight(current_container);
    }

    return(current_container ? 1 : -1);
}

/* Returns: -1 to stop, 1 item was removed, 0 item was not removed. */
static int out_container (struct obj *obj) {
    struct obj *otmp;
    bool is_gold = (obj->oclass == COIN_CLASS);
    int res, loadlev;
    long count;

    if (!current_container) {
        impossible("<out> no current_container?");
        return -1;
    } else if (is_gold) {
        obj->owt = weight(obj);
    }

    if(obj->oartifact && !touch_artifact(obj,&youmonst)) return 0;

    if (obj->otyp == CORPSE) {
        if ( (touch_petrifies(&mons[obj->corpsenm])) && !uarmg
                && !Stone_resistance()) {
            if (poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM))
                display_nhwindow(WIN_MESSAGE, false);
            else {
                char kbuf[BUFSZ];

                strcpy(kbuf, an(corpse_xname(obj, true)));
                pline("Touching %s is a fatal mistake.", kbuf);
                instapetrify(kbuf);
                return -1;
            }
        }
    }

    count = obj->quan;
    if ((res = lift_object(obj, current_container, &count, false)) <= 0)
        return res;

    if (obj->quan != count && obj->otyp != LOADSTONE)
        obj = splitobj(obj, count);

    /* Remove the object from the list. */
    obj_extract_self(obj);
    current_container->owt = weight(current_container);

    if ((current_container->otyp == ICE_BOX) && !age_is_relative(obj)) {
        obj->age = monstermoves - obj->age; /* actual age */
        if (obj->otyp == CORPSE)
            start_corpse_timeout(obj);
    }
    /* simulated point of time */

    if(!obj->unpaid && !carried(current_container) &&
            costly_spot(current_container->ox, current_container->oy)) {
        obj->ox = current_container->ox;
        obj->oy = current_container->oy;
        addtobill(obj, false, false, false);
    }
    if (is_pick(obj) && !obj->unpaid && *u.ushops && shop_keeper(*u.ushops))
        verbalize("You sneaky cad! Get out of here with that pick!");

    otmp = addinv(obj);
    loadlev = near_capacity();
    prinv(loadlev ?
            (loadlev < MOD_ENCUMBER ?
             "You have a little trouble removing" :
             "You have much trouble removing") : (char *)0,
            otmp, count);

    if (is_gold) {
        dealloc_obj(obj);
    }
    return 1;
}

/* an object inside a cursed bag of holding is being destroyed */
static long mbag_item_gone (int held, struct obj *item) {
    struct monst *shkp;
    long loss = 0L;

    if (item->dknown) {
        char have_tense[BUFSZ];
        otense(have_tense, BUFSZ, item, "have");
        pline("%s %s vanished!", Doname2(item), have_tense);
    } else {
        You("%s %s disappear!", Blind() ? "notice" : "see", doname(item));
    }

    if (*u.ushops && (shkp = shop_keeper(*u.ushops)) != 0) {
        if (held ? (bool) item->unpaid : costly_spot(u.ux, u.uy))
            loss = stolen_value(item, u.ux, u.uy,
                    (bool)shkp->mpeaceful, true);
    }
    obfree(item, (struct obj *) 0);
    return loss;
}

static void observe_quantum_cat (struct obj *box) {
    static const char sc[] = "Schroedinger's Cat";
    struct obj *deadcat;
    struct monst *livecat;
    signed char ox, oy;

    box->spe = 0;               /* box->owt will be updated below */
    if (get_obj_location(box, &ox, &oy, 0))
        box->ox = ox, box->oy = oy;     /* in case it's being carried */

    /* this isn't really right, since any form of observation
       (telepathic or monster/object/food detection) ought to
       force the determination of alive vs dead state; but basing
       it just on opening the box is much simpler to cope with */
    livecat = rn2(2) ? makemon(&mons[PM_HOUSECAT],
            box->ox, box->oy, NO_MINVENT) : 0;
    if (livecat) {
        livecat->mpeaceful = 1;
        set_malign(livecat);
        if (!canspotmon(livecat)) {
            You("think %s brushed your %s.", something, body_part(FOOT));
        } else {
            message_monster(MSG_M_INSIDE_BOX_IS_ALIVE, livecat);
        }
        christen_monst(livecat, sc);
    } else {
        deadcat = mk_named_object(CORPSE, &mons[PM_HOUSECAT],
                box->ox, box->oy, sc);
        if (deadcat) {
            obj_extract_self(deadcat);
            (void) add_to_container(box, deadcat);
        }
        pline_The("%s inside the box is dead!",
                Hallucination() ? rndmonnam() : "housecat");
    }
    box->owt = weight(box);
    return;
}

static int in_or_out_menu (const char *prompt, struct obj *obj, bool outokay, bool inokay) {
    winid win;
    anything any;
    menu_item *pick_list;
    char buf[BUFSZ];
    int n;
    const char *menuselector = iflags.lootabc ? "abc" : "oib";

    any.a_void = 0;
    win = create_nhwindow(NHW_MENU);
    start_menu(win);
    if (outokay) {
        any.a_int = 1;
        sprintf(buf,"Take %s out of %s", something, the(xname(obj)));
        add_menu(win, NO_GLYPH, &any, *menuselector, 0, ATR_NONE,
                buf, MENU_UNSELECTED);
    }
    menuselector++;
    if (inokay) {
        any.a_int = 2;
        sprintf(buf,"Put %s into %s", something, the(xname(obj)));
        add_menu(win, NO_GLYPH, &any, *menuselector, 0, ATR_NONE, buf, MENU_UNSELECTED);
    }
    menuselector++;
    if (outokay && inokay) {
        any.a_int = 3;
        add_menu(win, NO_GLYPH, &any, *menuselector, 0, ATR_NONE,
                "Both of the above", MENU_UNSELECTED);
    }
    end_menu(win, prompt);
    n = select_menu(win, PICK_ONE, &pick_list);
    destroy_nhwindow(win);
    if (n > 0) {
        n = pick_list[0].item.a_int;
        free((void *) pick_list);
    }
    return n;
}

/* Loot a container (take things out, put things in), using a menu. */
static int menu_loot (int retry, struct obj *container, bool put_in) {
    int n, i, n_looted = 0;
    bool all_categories = true, loot_everything = false;
    char buf[BUFSZ];
    const char *takeout = "Take out", *putin = "Put in";
    struct obj *otmp, *otmp2;
    menu_item *pick_list;
    int mflags, res;
    long count;

    if (retry) {
        all_categories = (retry == -2);
    } else {
        all_categories = false;
        sprintf(buf,"%s what type of objects?", put_in ? putin : takeout);
        mflags = put_in ? ALL_TYPES | BUC_ALLBKNOWN | BUC_UNKNOWN :
            ALL_TYPES | CHOOSE_ALL | BUC_ALLBKNOWN | BUC_UNKNOWN;
        n = query_category(buf, put_in ? invent : container->cobj,
                mflags, &pick_list, PICK_ANY);
        if (!n) return 0;
        for (i = 0; i < n; i++) {
            if (pick_list[i].item.a_int == 'A')
                loot_everything = true;
            else if (pick_list[i].item.a_int == ALL_TYPES_SELECTED)
                all_categories = true;
            else
                add_valid_menu_class(pick_list[i].item.a_int);
        }
        free((void *) pick_list);
    }

    if (loot_everything) {
        for (otmp = container->cobj; otmp; otmp = otmp2) {
            otmp2 = otmp->nobj;
            res = out_container(otmp);
            if (res < 0) break;
        }
    } else {
        mflags = INVORDER_SORT;
        if (put_in && flags.invlet_constant) mflags |= USE_INVLET;
        sprintf(buf,"%s what?", put_in ? putin : takeout);
        n = query_objlist(buf, put_in ? invent : container->cobj,
                mflags, &pick_list, PICK_ANY,
                all_categories ? allow_all : allow_category);
        if (n) {
            n_looted = n;
            for (i = 0; i < n; i++) {
                otmp = pick_list[i].item.a_obj;
                count = pick_list[i].count;
                if (count > 0 && count < otmp->quan) {
                    otmp = splitobj(otmp, count);
                    /* special split case also handled by askchain() */
                }
                res = put_in ? in_container(otmp) : out_container(otmp);
                if (res < 0) {
                    if (otmp != pick_list[i].item.a_obj) {
                        /* split occurred, merge again */
                        (void) merged(&pick_list[i].item.a_obj, &otmp);
                    }
                    break;
                }
            }
            free((void *)pick_list);
        }
    }
    return n_looted;
}

int use_container (struct obj *obj, int held) {
    struct obj *curr, *otmp;
    struct obj *u_gold = (struct obj *)0;
    bool one_by_one, allflag, quantum_cat = false,
         loot_out = false, loot_in = false;
    char select[MAXOCLASSES+1];
    char qbuf[BUFSZ], emptymsg[BUFSZ], pbuf[QBUFSZ];
    long loss = 0L;
    int cnt = 0, used = 0,
        menu_on_request;

    emptymsg[0] = '\0';
    if (nohands(youmonst.data)) {
        You("have no hands!");  /* not `body_part(HAND)' */
        return 0;
    } else if (!freehand()) {
        You("have no free %s.", body_part(HAND));
        return 0;
    }
    if (obj->olocked) {
        message_object(MSG_O_SEEMS_TO_BE_LOCKED, obj);
        if (held)
            You("must put it down to unlock.");
        return 0;
    } else if (obj->otrapped) {
        if (held) You("open %s...", the(xname(obj)));
        (void) chest_trap(obj, HAND, false);
        /* even if the trap fails, you've used up this turn */
        if (multi >= 0) {   /* in case we didn't become paralyzed */
            nomul(-1);
            nomovemsg = "";
        }
        return 1;
    }
    current_container = obj;        /* for use by in/out_container */

    if (obj->spe == 1) {
        observe_quantum_cat(obj);
        used = 1;
        quantum_cat = true; /* for adjusting "it's empty" message */
    }
    /* Count the number of contained objects. Sometimes toss objects if */
    /* a cursed magic bag.                                              */
    for (curr = obj->cobj; curr; curr = otmp) {
        otmp = curr->nobj;
        if (Is_mbag(obj) && obj->cursed && !rn2(13)) {
            obj_extract_self(curr);
            loss += mbag_item_gone(held, curr);
            used = 1;
        } else {
            cnt++;
        }
    }

    if (loss)       /* magic bag lost some shop goods */
        You("owe %ld %s for lost merchandise.", loss, currency(loss));
    obj->owt = weight(obj); /* in case any items were lost */

    if (!cnt)
        sprintf(emptymsg, "%s is %sempty.", Yname2(obj),
                quantum_cat ? "now " : "");

    strcpy(qbuf, "Do you want to take something out of ");
    sprintf(eos(qbuf), "%s?",
            safe_qbuf(qbuf, 1, yname(obj), ysimple_name(obj), "it"));
    int t;
    char menuprompt[BUFSZ];
    bool outokay = (cnt != 0);
    bool inokay = (invent != 0) || (u.ugold != 0);
    if (!outokay && !inokay) {
        pline("%s", emptymsg);
        You("don't have anything to put in.");
        return used;
    }
    menuprompt[0] = '\0';
    if (!cnt) sprintf(menuprompt, "%s ", emptymsg);
    strcat(menuprompt, "Do what?");
    t = in_or_out_menu(menuprompt, current_container, outokay, inokay);
    if (t <= 0) return 0;
    loot_out = (t & 0x01) != 0;
    loot_in  = (t & 0x02) != 0;
    if (loot_out) {
        add_valid_menu_class(0);    /* reset */
        used |= menu_loot(0, current_container, false) > 0;
    }

    if (!invent && u.ugold == 0) {
        /* nothing to put in, but some feedback is necessary */
        You("don't have anything to put in.");
        return used;
    }
    /*
     * Gone: being nice about only selecting food if we know we are
     * putting things in an ice chest.
     */
    if (loot_in) {
        if (u.ugold) {
            /*
             * Hack: gold is not in the inventory, so make a gold object
             * and put it at the head of the inventory list.
             */
            u_gold = mkgoldobj(u.ugold);    /* removes from u.ugold */
            u_gold->in_use = true;
            u.ugold = u_gold->quan;         /* put the gold back */
            assigninvlet(u_gold);           /* might end up as NOINVSYM */
            u_gold->nobj = invent;
            invent = u_gold;
        }
        add_valid_menu_class(0);      /* reset */
        used |= menu_loot(0, current_container, true) > 0;
    }

    if (u_gold && invent && invent->oclass == COIN_CLASS) {
        /* didn't stash [all of] it */
        u_gold = invent;
        invent = u_gold->nobj;
        u_gold->in_use = false;
        dealloc_obj(u_gold);
    }
    return used;
}


