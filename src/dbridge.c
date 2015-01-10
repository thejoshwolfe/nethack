/* See LICENSE in the root of this project for change info */
/*
 * This file contains the drawbridge manipulation (create, open, close,
 * destroy).
 *
 * Added comprehensive monster-handling, and the "entity" structure to
 * deal with players as well. - 11/89
 */

#include <stddef.h>

#include "move.h"
#include "dungeon_util.h"
#include "cmd.h"
#include "coord.h"
#include "dbridge.h"
#include "decl.h"
#include "display.h"
#include "do.h"
#include "do_name.h"
#include "dungeon.h"
#include "end.h"
#include "flag.h"
#include "global.h"
#include "hack.h"
#include "invent.h"
#include "mkobj.h"
#include "mon.h"
#include "monattk.h"
#include "mondata.h"
#include "monst.h"
#include "objnam.h"
#include "onames.h"
#include "permonst.h"
#include "pline.h"
#include "pm.h"
#include "region.h"
#include "rm.h"
#include "rnd.h"
#include "steed.h"
#include "teleport.h"
#include "trap.h"
#include "util.h"
#include "vision.h"
#include "you.h"
#include "youprop.h"

static void get_wall_for_db(int *, int *);
static struct entity *e_at(int, int);
static void m_to_e(struct monst *, int, int, struct entity *);
static void u_to_e(struct entity *);
static void set_entity(int, int, struct entity *);
static bool e_survives_at(struct entity *, int, int);
static void e_died(struct entity *, int, int);
static bool automiss(struct entity *);
static bool e_missed(struct entity *, bool);
static bool e_jumps(struct entity *);
static void do_entity(struct entity *);

bool is_pool (int x, int y) {
    signed char ltyp;

    if (!isok(x,y)) return false;
    ltyp = levl[x][y].typ;
    if (ltyp == POOL || ltyp == MOAT || ltyp == WATER) return true;
    if (ltyp == DRAWBRIDGE_UP &&
        (levl[x][y].drawbridgemask & DB_UNDER) == DB_MOAT) return true;
    return false;
}

bool is_lava (int x, int y) {
    signed char ltyp;

    if (!isok(x,y)) return false;
    ltyp = levl[x][y].typ;
    if (ltyp == LAVAPOOL
        || (ltyp == DRAWBRIDGE_UP
            && (levl[x][y].drawbridgemask & DB_UNDER) == DB_LAVA)) return true;
    return false;
}

bool is_ice (int x, int y) {
    signed char ltyp;

    if (!isok(x,y)) return false;
    ltyp = levl[x][y].typ;
    if (ltyp == ICE
        || (ltyp == DRAWBRIDGE_UP
            && (levl[x][y].drawbridgemask & DB_UNDER) == DB_ICE)) return true;
    return false;
}

/*
 * We want to know whether a wall (or a door) is the portcullis (passageway)
 * of an eventual drawbridge.
 *
 * Return value:  the direction of the drawbridge.
 */

int is_drawbridge_wall (int x, int y) {
        struct rm *lev;

        lev = &levl[x][y];
        if (lev->typ != DOOR && lev->typ != DBWALL)
                return (-1);

        if (IS_DRAWBRIDGE(levl[x+1][y].typ) &&
            (levl[x+1][y].drawbridgemask & DB_DIR) == DB_WEST)
                return (DB_WEST);
        if (IS_DRAWBRIDGE(levl[x-1][y].typ) &&
            (levl[x-1][y].drawbridgemask & DB_DIR) == DB_EAST)
                return (DB_EAST);
        if (IS_DRAWBRIDGE(levl[x][y-1].typ) &&
            (levl[x][y-1].drawbridgemask & DB_DIR) == DB_SOUTH)
                return (DB_SOUTH);
        if (IS_DRAWBRIDGE(levl[x][y+1].typ) &&
            (levl[x][y+1].drawbridgemask & DB_DIR) == DB_NORTH)
                return (DB_NORTH);

        return (-1);
}

/*
 * Use is_db_wall where you want to verify that a
 * drawbridge "wall" is UP in the location x, y
 * (instead of UP or DOWN, as with is_drawbridge_wall).
 */
bool is_db_wall (int x, int y) {
        return((bool)( levl[x][y].typ == DBWALL ));
}

/*
 * Return true with x,y pointing to the drawbridge if x,y initially indicate
 * a drawbridge or drawbridge wall.
 */
bool find_drawbridge (int *x, int *y) {
        int dir;

        if (IS_DRAWBRIDGE(levl[*x][*y].typ))
                return true;
        dir = is_drawbridge_wall(*x,*y);
        if (dir >= 0) {
                switch(dir) {
                        case DB_NORTH: (*y)++; break;
                        case DB_SOUTH: (*y)--; break;
                        case DB_EAST:  (*x)--; break;
                        case DB_WEST:  (*x)++; break;
                }
                return true;
        }
        return false;
}

/*
 * Find the drawbridge wall associated with a drawbridge.
 */
static void get_wall_for_db (int *x, int *y) {
        switch (levl[*x][*y].drawbridgemask & DB_DIR) {
                case DB_NORTH: (*y)--; break;
                case DB_SOUTH: (*y)++; break;
                case DB_EAST:  (*x)++; break;
                case DB_WEST:  (*x)--; break;
        }
}

/*
 * Creation of a drawbridge at pos x,y.
 *     dir is the direction.
 *     flag must be put to true if we want the drawbridge to be opened.
 */

bool create_drawbridge(int x,int y,int dir,bool flag) {
        int x2,y2;
        bool horiz;
        bool lava = levl[x][y].typ == LAVAPOOL; /* assume initialized map */

        x2 = x; y2 = y;
        switch(dir) {
                case DB_NORTH:
                        horiz = true;
                        y2--;
                        break;
                case DB_SOUTH:
                        horiz = true;
                        y2++;
                        break;
                case DB_EAST:
                        horiz = false;
                        x2++;
                        break;
                default:
                        impossible("bad direction in create_drawbridge");
                        /* fall through */
                case DB_WEST:
                        horiz = false;
                        x2--;
                        break;
        }
        if (!IS_WALL(levl[x2][y2].typ))
                return(false);
        if (flag) {             /* We want the bridge open */
                levl[x][y].typ = DRAWBRIDGE_DOWN;
                levl[x2][y2].typ = DOOR;
                levl[x2][y2].doormask = D_NODOOR;
        } else {
                levl[x][y].typ = DRAWBRIDGE_UP;
                levl[x2][y2].typ = DBWALL;
                /* Drawbridges are non-diggable. */
                levl[x2][y2].wall_info = W_NONDIGGABLE;
        }
        levl[x][y].horizontal = !horiz;
        levl[x2][y2].horizontal = horiz;
        levl[x][y].drawbridgemask = dir;
        if(lava) levl[x][y].drawbridgemask |= DB_LAVA;
        return(true);
}

struct entity {
        struct monst *emon;       /* youmonst for the player */
        struct permonst *edata;   /* must be non-zero for record to be valid */
        int ex, ey;
};

#define ENTITIES 2

static struct entity occupants[ENTITIES];

static struct entity * e_at (int x, int y) {
        int entitycnt;

        for (entitycnt = 0; entitycnt < ENTITIES; entitycnt++)
                if ((occupants[entitycnt].edata) &&
                    (occupants[entitycnt].ex == x) &&
                    (occupants[entitycnt].ey == y))
                        break;
        return((entitycnt == ENTITIES)?
               (struct entity *)0 : &(occupants[entitycnt]));
}

static void m_to_e (struct monst *mtmp, int x, int y, struct entity *etmp) {
        etmp->emon = mtmp;
        if (mtmp) {
                etmp->ex = x;
                etmp->ey = y;
                if (mtmp->wormno && (x != mtmp->mx || y != mtmp->my))
                        etmp->edata = &mons[PM_LONG_WORM_TAIL];
                else
                        etmp->edata = mtmp->data;
        } else
                etmp->edata = (struct permonst *)0;
}

static void u_to_e (struct entity *etmp) {
        etmp->emon = &youmonst;
        etmp->ex = u.ux;
        etmp->ey = u.uy;
        etmp->edata = youmonst.data;
}

static void set_entity (int x, int y, struct entity *etmp) {
        if ((x == u.ux) && (y == u.uy))
                u_to_e(etmp);
        else if (MON_AT(x, y))
                m_to_e(m_at(x, y), x, y, etmp);
        else
                etmp->edata = (struct permonst *)0;
}

#define is_u(etmp) ((etmp)->emon == &youmonst)
#define e_canseemon(etmp) (is_u(etmp) ? (bool)true : canseemon((etmp)->emon))

static size_t e_nam(char *out_buf, size_t buf_size, const struct entity *e) {
    return is_u(e) ?
        nh_strlcpy(out_buf, "you", buf_size) :
        mon_nam(out_buf, buf_size, e->emon);
}

/*
 * Generates capitalized entity name, makes 2nd -> 3rd person conversion on
 * verb, where necessary.
 */

static size_t E_phrase (char *out_buf, size_t buf_size,
        const struct entity *etmp, const char *verb)
{
    if (is_u(etmp)) {
        if (*verb)
            return nh_slprintf(out_buf, buf_size, "You %s", verb);
        else
            return nh_strlcpy(out_buf, "You", buf_size);
    } else {
        if (*verb) {
            char name[BUFSZ];
            Monnam(name, BUFSZ, etmp->emon);
            char verb_buf[BUFSZ];
            vtense(verb_buf, BUFSZ, NULL, verb);
            return nh_slprintf(out_buf, buf_size, "%s %s", name, verb_buf);
        } else {
            return Monnam(out_buf, buf_size, etmp->emon);
        }
    }
}

/*
 * Simple-minded "can it be here?" routine
 */

static bool e_survives_at (struct entity *etmp, int x, int y) {
        if (noncorporeal(etmp->edata))
                return(true);
        if (is_pool(x, y))
                return (bool)((is_u(etmp) &&
                                (Wwalking || Amphibious || Swimming ||
                                Flying || Levitation)) ||
                        is_swimmer(etmp->edata) || is_flyer(etmp->edata) ||
                        is_floater(etmp->edata));
        /* must force call to lava_effects in e_died if is_u */
        if (is_lava(x, y))
                return (bool)((is_u(etmp) && (Levitation || Flying)) ||
                            likes_lava(etmp->edata) || is_flyer(etmp->edata));
        if (is_db_wall(x, y))
                return((bool)(is_u(etmp) ? Passes_walls :
                        passes_walls(etmp->edata)));
        return(true);
}

static void e_died (struct entity *etmp, int dest, int how) {
    if (is_u(etmp)) {
        if (how == DROWNING) {
            killer.method = KM_DIED; // drown() sets its own killer
            drown();
        } else if (how == BURNING) {
            killer.method = KM_DIED; // lava_effects() sets its own killer
            lava_effects();
        } else {
            coord xy;

            /* use more specific killer if specified */
            if (killer.method != KM_DIED) {
                killer.method = KM_FALLING_DRAWBRIDGE;
            }
            done(how);
            /* So, you didn't die */
            if (!e_survives_at(etmp, etmp->ex, etmp->ey)) {
                if (enexto(&xy, etmp->ex, etmp->ey, etmp->edata)) {
                    pline("A %s force teleports you away...",
                            Hallucination() ? "normal" : "strange");
                    teleds(xy.x, xy.y, false);
                }
                /* otherwise on top of the drawbridge is the
                 * only viable spot in the dungeon, so stay there
                 */
            }
        }
        /* we might have crawled out of the moat to survive */
        etmp->ex = u.ux,  etmp->ey = u.uy;
    } else {
        int entitycnt;

        /* fake "digested to death" damage-type suppresses corpse */
        /* if monsters are moving, one of them caused the destruction */
        if (flags.mon_moving) {
            monkilled(etmp->emon, (dest & 1) ? "" : NULL, (dest & 2) ? AD_DGST : AD_PHYS);
        } else {
            // you caused it
            xkilled(etmp->emon, dest);
        }
        etmp->edata = (struct permonst *)0;

        /* dead long worm handling */
        for (entitycnt = 0; entitycnt < ENTITIES; entitycnt++) {
            if (etmp != &(occupants[entitycnt]) &&
                    etmp->emon == occupants[entitycnt].emon)
                occupants[entitycnt].edata = (struct permonst *)0;
        }
    }
}

/*
 * These are never directly affected by a bridge or portcullis.
 */

static bool automiss (struct entity *etmp) {
        return (bool)((is_u(etmp) ? Passes_walls :
                        passes_walls(etmp->edata)) || noncorporeal(etmp->edata));
}

/*
 * Does falling drawbridge or portcullis miss etmp?
 */

static bool e_missed(struct entity *etmp, bool chunks) {
        int misses;

        if (automiss(etmp))
                return(true);

        if (is_flyer(etmp->edata) &&
            (is_u(etmp)? !Sleeping :
             (etmp->emon->mcanmove && !etmp->emon->msleeping)))
                                                 /* flying requires mobility */
                misses = 5;     /* out of 8 */
        else if (is_floater(etmp->edata) ||
                    (is_u(etmp) && Levitation))  /* doesn't require mobility */
                misses = 3;
        else if (chunks && is_pool(etmp->ex, etmp->ey))
                misses = 2;                                 /* sitting ducks */
        else
                misses = 0;

        if (is_db_wall(etmp->ex, etmp->ey))
                misses -= 3;                                /* less airspace */


        return((bool)((misses >= rnd(8))? true : false));
}

/*
 * Can etmp jump from death?
 */

static bool e_jumps (struct entity *etmp) {
        int tmp = 4;            /* out of 10 */

        if (is_u(etmp)? (Sleeping || Fumbling) :
                        (!etmp->emon->mcanmove || etmp->emon->msleeping ||
                         !etmp->edata->mmove   || etmp->emon->wormno))
                return(false);

        if (is_u(etmp)? Confusion() : etmp->emon->mconf)
                tmp -= 2;

        if (is_u(etmp)? Stunned() : etmp->emon->mstun)
                tmp -= 3;

        if (is_db_wall(etmp->ex, etmp->ey))
                tmp -= 2;                           /* less room to maneuver */

        return((bool)((tmp >= rnd(10))? true : false));
}

static void do_entity (struct entity *etmp) {
    int newx, newy, at_portcullis, oldx, oldy;
    bool must_jump = false, relocates = false, e_inview;
    struct rm *crm;

    if (!etmp->edata)
        return;

    e_inview = e_canseemon(etmp);
    oldx = etmp->ex;
    oldy = etmp->ey;
    at_portcullis = is_db_wall(oldx, oldy);
    crm = &levl[oldx][oldy];

    if (automiss(etmp) && e_survives_at(etmp, oldx, oldy)) {
        if (e_inview && (at_portcullis || IS_DRAWBRIDGE(crm->typ))) {
            char name[BUFSZ];
            e_nam(name, BUFSZ, etmp);
            pline_The("%s passes through %s!", at_portcullis ? "portcullis" : "drawbridge", name);
        }
        if (is_u(etmp)) spoteffects(false);
        return;
    }
    if (e_missed(etmp, false)) {
        if (at_portcullis) {
            char name[BUFSZ];
            e_nam(name, BUFSZ, etmp);
            pline_The("portcullis misses %s!", name);
        }
        if (e_survives_at(etmp, oldx, oldy))
            return;
        else {
            if (at_portcullis)
                must_jump = true;
            else
                relocates = true; /* just ride drawbridge in */
        }
    } else {
        if (crm->typ == DRAWBRIDGE_DOWN) {
            // no jump
            char subject[BUFSZ];
            E_phrase(subject, BUFSZ, etmp, "are");
            pline("%s crushed underneath the drawbridge.", subject);
            e_died(etmp, e_inview? 3 : 2, CRUSHING);/* no corpse */
            return;   /* Note: Beyond this point, we know we're  */
        }                 /* not at an opened drawbridge, since all  */
        must_jump = true; /* *missable* creatures survive on the     */
    }                         /* square, and all the unmissed ones die.  */
    if (must_jump) {
        if (at_portcullis) {
            if (e_jumps(etmp)) {
                relocates = true;
            } else {
                if (e_inview) {
                    char subject[BUFSZ];
                    E_phrase(subject, BUFSZ, etmp, "are");
                    pline("%s crushed by the falling portcullis!", subject);
                } else if (flags.soundok) {
                    You_hear("a crushing sound.");
                }
                e_died(etmp, e_inview? 3 : 2, CRUSHING);
                // no corpse
                return;
            }
        } else { /* tries to jump off bridge to original square */
            relocates = !e_jumps(etmp);
        }
    }

    /*
     * Here's where we try to do relocation.  Assumes that etmp is not arriving
     * at the portcullis square while the drawbridge is falling, since this square
     * would be inaccessible (i.e. etmp started on drawbridge square) or
     * unnecessary (i.e. etmp started here) in such a situation.
     */
    newx = oldx;
    newy = oldy;
    find_drawbridge(&newx, &newy);
    if ((newx == oldx) && (newy == oldy))
        get_wall_for_db(&newx, &newy);
    if (relocates && (e_at(newx, newy))) {

        /*
         * Standoff problem:  one or both entities must die, and/or both switch
         * places.  Avoid infinite recursion by checking first whether the other
         * entity is staying put.  Clean up if we happen to move/die in recursion.
         */
        struct entity *other;

        other = e_at(newx, newy);
        if (e_survives_at(other, newx, newy) && automiss(other)) {
            relocates = false;            /* "other" won't budge */
        } else {

            while ((e_at(newx, newy) != 0) &&
                    (e_at(newx, newy) != etmp))
                do_entity(other);
            if (e_at(oldx, oldy) != etmp) {
                return;
            }
        }
    }
    if (relocates && !e_at(newx, newy)) {/* if e_at() entity = worm tail */
        if (!is_u(etmp)) {
            remove_monster(etmp->ex, etmp->ey);
            place_monster(etmp->emon, newx, newy);
            update_monster_region(etmp->emon);
        } else {
            u.ux = newx;
            u.uy = newy;
        }
        etmp->ex = newx;
        etmp->ey = newy;
        e_inview = e_canseemon(etmp);
    }
    if (is_db_wall(etmp->ex, etmp->ey)) {
        if (e_inview) {
            if (is_u(etmp)) {
                You("tumble towards the closed portcullis!");
                if (automiss(etmp))
                    You("pass through it!");
                else
                    pline_The("drawbridge closes in...");
            } else {
                char subject[BUFSZ];
                E_phrase(subject, BUFSZ, etmp, "disappear");
                pline("%s behind the drawbridge.", subject);
            }
        }
        if (!e_survives_at(etmp, etmp->ex, etmp->ey)) {
            killer.method = KM_CLOSING_DRAWBRIDGE;
            e_died(etmp, 0, KM_CRUSHING);
            return;
        }
    } else {
        if (is_pool(etmp->ex, etmp->ey) && !e_inview)
            if (flags.soundok)
                You_hear("a splash.");
        if (e_survives_at(etmp, etmp->ex, etmp->ey)) {
            if (e_inview && !is_flyer(etmp->edata) && !is_floater(etmp->edata)) {
                char subject[BUFSZ];
                E_phrase(subject, BUFSZ, etmp, "fall");
                pline("%s from the bridge.", subject);
            }
            return;
        }
        if (is_pool(etmp->ex, etmp->ey) || is_lava(etmp->ex, etmp->ey))
            if (e_inview && !is_u(etmp)) {
                /* drown() will supply msgs if nec. */
                const char *moat_str = is_lava(etmp->ex, etmp->ey) ? "lava" : "moat";
                if (Hallucination()) {
                    char subject[BUFSZ];
                    E_phrase(subject, BUFSZ, etmp, "drink");
                    pline("%s the %s and disappears.", subject, moat_str);
                } else {
                    char subject[BUFSZ];
                    E_phrase(subject, BUFSZ, etmp, "fall");
                    pline("%s into the %s.", subject, moat_str);
                }
            }
        killer.method = KM_FELL_FROM_DRAWBRIDGE;
        e_died(etmp, e_inview ? 3 : 2,      /* CRUSHING is arbitrary */
                (is_pool(etmp->ex, etmp->ey)) ? KM_DROWNING :
                (is_lava(etmp->ex, etmp->ey)) ? KM_BURNING :
                KM_CRUSHING); /*no corpse*/
        return;
    }
}

/*
 * Close the drawbridge located at x,y
 */

void close_drawbridge (int x, int y) {
        struct rm *lev1, *lev2;
        struct trap *t;
        int x2, y2;

        lev1 = &levl[x][y];
        if (lev1->typ != DRAWBRIDGE_DOWN) return;
        x2 = x; y2 = y;
        get_wall_for_db(&x2,&y2);
        if (cansee(x,y) || cansee(x2,y2))
                You("see a drawbridge %s up!",
                    (((u.ux == x || u.uy == y) && !Underwater) ||
                     distu(x2,y2) < distu(x,y)) ? "coming" : "going");
        lev1->typ = DRAWBRIDGE_UP;
        lev2 = &levl[x2][y2];
        lev2->typ = DBWALL;
        switch (lev1->drawbridgemask & DB_DIR) {
                case DB_NORTH:
                case DB_SOUTH:
                        lev2->horizontal = true;
                        break;
                case DB_WEST:
                case DB_EAST:
                        lev2->horizontal = false;
                        break;
        }
        lev2->wall_info = W_NONDIGGABLE;
        set_entity(x, y, &(occupants[0]));
        set_entity(x2, y2, &(occupants[1]));
        do_entity(&(occupants[0]));             /* Do set_entity after first */
        set_entity(x2, y2, &(occupants[1]));    /* do_entity for worm tail */
        do_entity(&(occupants[1]));
        if(OBJ_AT(x,y) && flags.soundok)
            You_hear("smashing and crushing.");
        (void) revive_nasty(x,y,(char *)0);
        (void) revive_nasty(x2,y2,(char *)0);
        delallobj(x, y);
        delallobj(x2, y2);
        if ((t = t_at(x, y)) != 0) deltrap(t);
        if ((t = t_at(x2, y2)) != 0) deltrap(t);
        newsym(x, y);
        newsym(x2, y2);
        block_point(x2,y2);     /* vision */
}

/*
 * Open the drawbridge located at x,y
 */

void open_drawbridge (int x, int y) {
        struct rm *lev1, *lev2;
        struct trap *t;
        int x2, y2;

        lev1 = &levl[x][y];
        if (lev1->typ != DRAWBRIDGE_UP) return;
        x2 = x; y2 = y;
        get_wall_for_db(&x2,&y2);
        if (cansee(x,y) || cansee(x2,y2))
                You("see a drawbridge %s down!",
                    (distu(x2,y2) < distu(x,y)) ? "going" : "coming");
        lev1->typ = DRAWBRIDGE_DOWN;
        lev2 = &levl[x2][y2];
        lev2->typ = DOOR;
        lev2->doormask = D_NODOOR;
        set_entity(x, y, &(occupants[0]));
        set_entity(x2, y2, &(occupants[1]));
        do_entity(&(occupants[0]));             /* do set_entity after first */
        set_entity(x2, y2, &(occupants[1]));    /* do_entity for worm tails */
        do_entity(&(occupants[1]));
        (void) revive_nasty(x,y,(char *)0);
        delallobj(x, y);
        if ((t = t_at(x, y)) != 0) deltrap(t);
        if ((t = t_at(x2, y2)) != 0) deltrap(t);
        newsym(x, y);
        newsym(x2, y2);
        unblock_point(x2,y2);   /* vision */
        if (Is_stronghold(&u.uz)) u.uevent.uopened_dbridge = true;
}

/*
 * Let's destroy the drawbridge located at x,y
 */

void destroy_drawbridge (int x, int y) {
    struct rm *lev1, *lev2;
    struct trap *t;
    int x2, y2;
    bool e_inview;
    struct entity *etmp1 = &(occupants[0]), *etmp2 = &(occupants[1]);

    lev1 = &levl[x][y];
    if (!IS_DRAWBRIDGE(lev1->typ))
        return;
    x2 = x; y2 = y;
    get_wall_for_db(&x2,&y2);
    lev2 = &levl[x2][y2];
    if ((lev1->drawbridgemask & DB_UNDER) == DB_MOAT ||
            (lev1->drawbridgemask & DB_UNDER) == DB_LAVA) {
        struct obj *otmp;
        bool lava = (lev1->drawbridgemask & DB_UNDER) == DB_LAVA;
        if (lev1->typ == DRAWBRIDGE_UP) {
            if (cansee(x2,y2))
                pline_The("portcullis of the drawbridge falls into the %s!",
                        lava ? "lava" : "moat");
            else if (flags.soundok)
                You_hear("a loud *SPLASH*!");
        } else {
            if (cansee(x,y))
                pline_The("drawbridge collapses into the %s!",
                        lava ? "lava" : "moat");
            else if (flags.soundok)
                You_hear("a loud *SPLASH*!");
        }
        lev1->typ = lava ? LAVAPOOL : MOAT;
        lev1->drawbridgemask = 0;
        if ((otmp = sobj_at(BOULDER,x,y)) != 0) {
            obj_extract_self(otmp);
            (void) flooreffects(otmp,x,y,"fall");
        }
    } else {
        if (cansee(x,y))
            pline_The("drawbridge disintegrates!");
        else
            You_hear("a loud *CRASH*!");
        lev1->typ =
            ((lev1->drawbridgemask & DB_ICE) ? ICE : ROOM);
        lev1->icedpool =
            ((lev1->drawbridgemask & DB_ICE) ? ICED_MOAT : 0);
    }
    wake_nearto(x, y, 500);
    lev2->typ = DOOR;
    lev2->doormask = D_NODOOR;
    if ((t = t_at(x, y)) != 0) deltrap(t);
    if ((t = t_at(x2, y2)) != 0) deltrap(t);
    newsym(x,y);
    newsym(x2,y2);
    if (!does_block(x2,y2,lev2)) unblock_point(x2,y2);      /* vision */
    if (Is_stronghold(&u.uz)) u.uevent.uopened_dbridge = true;

    set_entity(x2, y2, etmp2); /* currently only automissers can be here */
    if (etmp2->edata) {
        e_inview = e_canseemon(etmp2);
        if (!automiss(etmp2)) {
            if (e_inview) {
                char subject[BUFSZ];
                E_phrase(subject, BUFSZ, etmp2, "are");
                pline("%s blown apart by flying debris.", subject);
            }
            killer.method = KM_EXPLODING_DRAWBRIDGE;
            e_died(etmp2, e_inview? 3 : 2, KM_CRUSHING); /*no corpse*/
        }            /* nothing which is vulnerable can survive this */
    }
    set_entity(x, y, etmp1);
    if (etmp1->edata) {
        e_inview = e_canseemon(etmp1);
        if (e_missed(etmp1, true)) {
        } else {
            if (e_inview) {
                if (!is_u(etmp1) && Hallucination()) {
                    char subject[BUFSZ];
                    E_phrase(subject, BUFSZ, etmp1, "get");
                    pline("%s into some heavy metal!", subject);
                } else {
                    char subject[BUFSZ];
                    E_phrase(subject, BUFSZ, etmp1, "are");
                    pline("%s hit by a huge chunk of metal!", subject);
                }
            } else {
                if (flags.soundok && !is_u(etmp1) && !is_pool(x,y))
                    You_hear("a crushing sound.");
            }
            killer.method = KM_COLLAPSING_DRAWBRIDGE;
            e_died(etmp1, e_inview? 3 : 2, KM_CRUSHING); /*no corpse*/
            if(lev1->typ == MOAT) do_entity(etmp1);
        }
    }
}
