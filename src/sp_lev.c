/* See LICENSE in the root of this project for change info */

/*
 * This file contains the various functions that are related to the special
 * levels.
 * It contains also the special level loader.
 *
 */

#include "sp_lev.h"

#include <stdlib.h>
#include <string.h>

#include "rm_util.h"
#include "dungeon_util.h"
#include "cmd.h"
#include "dbridge.h"
#include "decl.h"
#include "dlb.h"
#include "do_name.h"
#include "drawing.h"
#include "dungeon.h"
#include "end.h"
#include "engrave.h"
#include "hack.h"
#include "hacklib.h"
#include "invent.h"
#include "makemon.h"
#include "mklev.h"
#include "mkmap.h"
#include "mkmaze.h"
#include "mkobj.h"
#include "mkroom.h"
#include "mon.h"
#include "mondata.h"
#include "monflag.h"
#include "monst.h"
#include "monsym.h"
#include "mplayer.h"
#include "obj.h"
#include "objclass.h"
#include "onames.h"
#include "permonst.h"
#include "pline.h"
#include "pm.h"
#include "pm_props.h"
#include "priest.h"
#include "rect.h"
#include "rm.h"
#include "rnd.h"
#include "shk.h"
#include "shknam.h"
#include "teleport.h"
#include "timeout.h"
#include "trap.h"
#include "version.h"
#include "vision.h"
#include "you.h"

extern void mkmap(lev_init *);

#define LEFT    1
#define H_LEFT  2
#define CENTER  3
#define H_RIGHT 4
#define RIGHT   5

#define TOP     1
#define BOTTOM  5

#define sq(x) ((x)*(x))

#define XLIM    4
#define YLIM    3

#define Fread   (void)dlb_fread
#define Fgetc   (signed char)dlb_fgetc
#define New(type)               (type *) malloc(sizeof(type))
#define NewTab(type, size)      (type **) malloc(sizeof(type *) * (unsigned)size)
#define Free(ptr)               if(ptr) free((void *) (ptr))

static walk walklist[50];
extern int min_rx, max_rx, min_ry, max_ry; /* from mkmap.c */

static char Map[COLNO][ROWNO];
static char robjects[10], rloc_x[10], rloc_y[10], rmonst[10];
static aligntyp ralign[3] = { AM_CHAOTIC, AM_NEUTRAL, AM_LAWFUL };
static signed char xstart, ystart;
static char xsize, ysize;

static void set_wall_property(signed char,signed char,signed char,signed char,int);
static int rnddoor(void);
static int rndtrap(void);
static void get_location(signed char *,signed char *,int);
static void sp_lev_shuffle(char *,char *,int);
static void light_region(region *);
static void load_common_data(dlb *,int);
static void load_one_monster(dlb *,monster *);
static void load_one_object(dlb *,object *);
static void load_one_engraving(dlb *,engraving *);
static bool load_rooms(dlb *);
static void maze1xy(coord *,int);
static bool load_maze(dlb *);
static void create_door(room_door *, struct mkroom *);
static void free_rooms(room **, int);
static void build_room(room *, room*);

char *lev_message = 0;
lev_region *lregions = 0;
int num_lregions = 0;
lev_init init_lev;

/*
 * Make walls of the area (x1, y1, x2, y2) non diggable/non passwall-able
 */

static void set_wall_property(signed char x1, signed char y1, signed char x2, signed char y2, int prop) {
    signed char x, y;

    for (y = y1; y <= y2; y++)
        for (x = x1; x <= x2; x++)
            if (IS_STWALL(levl[x][y].typ))
                levl[x][y].flags |= prop;
}

/*
 * Choose randomly the state (nodoor, open, closed or locked) for a door
 */
static int rnddoor(void) {
    int i = 1 << rn2(5);
    i >>= 1;
    return i;
}

/*
 * Select a random trap
 */
static int rndtrap(void) {
    int rtrap;

    do {
        rtrap = rnd(TRAPNUM - 1);
        switch (rtrap) {
            case HOLE: /* no random holes on special levels */
            case MAGIC_PORTAL:
                rtrap = NO_TRAP;
                break;
            case TRAPDOOR:
                if (!Can_dig_down(&u.uz))
                    rtrap = NO_TRAP;
                break;
            case LEVEL_TELEP:
            case TELEP_TRAP:
                if (level.flags.noteleport)
                    rtrap = NO_TRAP;
                break;
            case ROLLING_BOULDER_TRAP:
            case ROCKTRAP:
                if (In_endgame(&u.uz))
                    rtrap = NO_TRAP;
                break;
        }
    } while (rtrap == NO_TRAP);
    return rtrap;
}

/*
 * Coordinates in special level files are handled specially:
 *
 *      if x or y is -11, we generate a random coordinate.
 *      if x or y is between -1 and -10, we read one from the corresponding
 *      (x0, x1, ... x9).
 *      if x or y is nonnegative, we convert it from relative to the local map
 *      to global coordinates.
 *      The "humidity" flag is used to insure that engravings aren't
 *      created underwater, or eels on dry land.
 */
#define DRY     0x1
#define WET     0x2

static bool is_ok_location(signed char, signed char, int);

static void get_location(signed char *x, signed char *y, int humidity) {
    int cpt = 0;

    if (*x >= 0) { /* normal locations */
        *x += xstart;
        *y += ystart;
    } else if (*x > -11) { /* special locations */
        *y = ystart + rloc_y[-*y - 1];
        *x = xstart + rloc_x[-*x - 1];
    } else { /* random location */
        do {
            *x = xstart + rn2((int)xsize);
            *y = ystart + rn2((int)ysize);
            if (is_ok_location(*x, *y, humidity))
                break;
        } while (++cpt < 100);
        if (cpt >= 100) {
            int xx, yy;
            /* last try */
            for (xx = 0; xx < xsize; xx++)
                for (yy = 0; yy < ysize; yy++) {
                    *x = xstart + xx;
                    *y = ystart + yy;
                    if (is_ok_location(*x, *y, humidity))
                        goto found_it;
                }
            panic("get_location:  can't find a place!");
        }
    }
    found_it: ;

    if (!isok(*x, *y)) {
        impossible("get_location:  (%d,%d) out of bounds", *x, *y);
        *x = x_maze_max;
        *y = y_maze_max;
    }
}

static bool is_ok_location(signed char x, signed char y, int humidity) {
    int typ;

    if (Is_waterlevel(&u.uz))
        return true; /* accept any spot */

    if (humidity & DRY) {
        typ = levl[x][y].typ;
        if (typ == ROOM || typ == AIR || typ == CLOUD || typ == ICE || typ == CORR)
            return true;
    }
    if (humidity & WET) {
        if (is_pool(x, y) || is_lava(x, y))
            return true;
    }
    return false;
}

/*
 * Shuffle the registers for locations, objects or monsters
 */

static void sp_lev_shuffle(char list1[], char list2[], int n) {
    int i, j;
    char k;

    for (i = n - 1; i > 0; i--) {
        if ((j = rn2(i + 1)) == i)
            continue;
        k = list1[j];
        list1[j] = list1[i];
        list1[i] = k;
        if (list2) {
            k = list2[j];
            list2[j] = list2[i];
            list2[i] = k;
        }
    }
}

/*
 * Get a relative position inside a room.
 * negative values for x or y means RANDOM!
 */

static void get_room_loc(signed char *x, signed char *y, struct mkroom *croom) {
    coord c;

    if (*x < 0 && *y < 0) {
        if (somexy(croom, &c)) {
            *x = c.x;
            *y = c.y;
        } else
            panic("get_room_loc : can't find a place!");
    } else {
        if (*x < 0)
            *x = rn2(croom->hx - croom->lx + 1);
        if (*y < 0)
            *y = rn2(croom->hy - croom->ly + 1);
        *x += croom->lx;
        *y += croom->ly;
    }
}

/*
 * Get a relative position inside a room.
 * negative values for x or y means RANDOM!
 */

static void get_free_room_loc(signed char *x, signed char *y, struct mkroom *croom) {
    signed char try_x, try_y;
    int trycnt = 0;

    do {
        try_x = *x, try_y = *y;
        get_room_loc(&try_x, &try_y, croom);
    } while (levl[try_x][try_y].typ != ROOM && ++trycnt <= 100);

    if (trycnt > 100)
        panic("get_free_room_loc:  can't find a place!");
    *x = try_x, *y = try_y;
}

bool check_room(signed char *lowx, signed char *ddx, signed char *lowy, signed char *ddy, bool vault) {
    int x, y, hix = *lowx + *ddx, hiy = *lowy + *ddy;
    struct rm *lev;
    int xlim, ylim, ymax;

    xlim = XLIM + (vault ? 1 : 0);
    ylim = YLIM + (vault ? 1 : 0);

    if (*lowx < 3)
        *lowx = 3;
    if (*lowy < 2)
        *lowy = 2;
    if (hix > COLNO - 3)
        hix = COLNO - 3;
    if (hiy > ROWNO - 3)
        hiy = ROWNO - 3;
    chk: if (hix <= *lowx || hiy <= *lowy)
        return false;

    /* check area around room (and make room smaller if necessary) */
    for (x = *lowx - xlim; x <= hix + xlim; x++) {
        if (x <= 0 || x >= COLNO)
            continue;
        y = *lowy - ylim;
        ymax = hiy + ylim;
        if (y < 0)
            y = 0;
        if (ymax >= ROWNO)
            ymax = (ROWNO - 1);
        lev = &levl[x][y];
        for (; y <= ymax; y++) {
            if (lev++->typ) {
                if (!rn2(3))
                    return false;
                if (x < *lowx)
                    *lowx = x + xlim + 1;
                else
                    hix = x - xlim - 1;
                if (y < *lowy)
                    *lowy = y + ylim + 1;
                else
                    hiy = y - ylim - 1;
                goto chk;
            }
        }
    }
    *ddx = hix - *lowx;
    *ddy = hiy - *lowy;
    return true;
}

/*
 * Create a new room.
 * This is still very incomplete...
 */

bool create_room(signed char x, signed char y, signed char w, signed char h, signed char xal, signed char yal, signed char rtype, signed char rlit) {
    signed char xabs, yabs;
    int wtmp, htmp, xaltmp, yaltmp, xtmp, ytmp;
    NhRect *r1 = 0, r2;
    int trycnt = 0;
    bool vault = false;
    int xlim = XLIM, ylim = YLIM;

    if (rtype == -1) /* Is the type random ? */
        rtype = OROOM;

    if (rtype == VAULT) {
        vault = true;
        xlim++;
        ylim++;
    }

    /* on low levels the room is lit (usually) */
    /* some other rooms may require lighting */

    /* is light state random ? */
    if (rlit == -1)
        rlit = (rnd(1 + abs(depth(&u.uz))) < 11 && rn2(77)) ? true : false;

    /*
     * Here we will try to create a room. If some parameters are
     * random we are willing to make several try before we give
     * it up.
     */
    do {
        signed char xborder, yborder;
        wtmp = w;
        htmp = h;
        xtmp = x;
        ytmp = y;
        xaltmp = xal;
        yaltmp = yal;

        /* First case : a totaly random room */

        if ((xtmp < 0 && ytmp < 0 && wtmp < 0 && xaltmp < 0 && yaltmp < 0) || vault) {
            signed char hx, hy, lx, ly, dx, dy;
            r1 = rnd_rect(); /* Get a random rectangle */

            if (!r1) { /* No more free rectangles ! */
                return false;
            }
            hx = r1->hx;
            hy = r1->hy;
            lx = r1->lx;
            ly = r1->ly;
            if (vault)
                dx = dy = 1;
            else {
                dx = 2 + rn2((hx - lx > 28) ? 12 : 8);
                dy = 2 + rn2(4);
                if (dx * dy > 50)
                    dy = 50 / dx;
            }
            xborder = (lx > 0 && hx < COLNO - 1) ? 2 * xlim : xlim + 1;
            yborder = (ly > 0 && hy < ROWNO - 1) ? 2 * ylim : ylim + 1;
            if (hx - lx < dx + 3 + xborder || hy - ly < dy + 3 + yborder) {
                r1 = 0;
                continue;
            }
            xabs = lx + (lx > 0 ? xlim : 3) + rn2(hx - (lx > 0 ? lx : 3) - dx - xborder + 1);
            yabs = ly + (ly > 0 ? ylim : 2) + rn2(hy - (ly > 0 ? ly : 2) - dy - yborder + 1);
            if (ly == 0 && hy >= (ROWNO - 1) && (!nroom || !rn2(nroom)) && (yabs + dy > ROWNO / 2)) {
                yabs = rn1(3, 2);
                if (nroom < 4 && dy > 1)
                    dy--;
            }
            if (!check_room(&xabs, &dx, &yabs, &dy, vault)) {
                r1 = 0;
                continue;
            }
            wtmp = dx + 1;
            htmp = dy + 1;
            r2.lx = xabs - 1;
            r2.ly = yabs - 1;
            r2.hx = xabs + wtmp;
            r2.hy = yabs + htmp;
        } else { /* Only some parameters are random */
            int rndpos = 0;
            if (xtmp < 0 && ytmp < 0) { /* Position is RANDOM */
                xtmp = rnd(5);
                ytmp = rnd(5);
                rndpos = 1;
            }
            if (wtmp < 0 || htmp < 0) { /* Size is RANDOM */
                wtmp = rn1(15, 3);
                htmp = rn1(8, 2);
            }
            if (xaltmp == -1) /* Horizontal alignment is RANDOM */
                xaltmp = rnd(3);
            if (yaltmp == -1) /* Vertical alignment is RANDOM */
                yaltmp = rnd(3);

            /* Try to generate real (absolute) coordinates here! */

            xabs = (((xtmp - 1) * COLNO) / 5) + 1;
            yabs = (((ytmp - 1) * ROWNO) / 5) + 1;
            switch (xaltmp) {
                case LEFT:
                    break;
                case RIGHT:
                    xabs += (COLNO / 5) - wtmp;
                    break;
                case CENTER:
                    xabs += ((COLNO / 5) - wtmp) / 2;
                    break;
            }
            switch (yaltmp) {
                case TOP:
                    break;
                case BOTTOM:
                    yabs += (ROWNO / 5) - htmp;
                    break;
                case CENTER:
                    yabs += ((ROWNO / 5) - htmp) / 2;
                    break;
            }

            if (xabs + wtmp - 1 > COLNO - 2)
                xabs = COLNO - wtmp - 3;
            if (xabs < 2)
                xabs = 2;
            if (yabs + htmp - 1 > ROWNO - 2)
                yabs = ROWNO - htmp - 3;
            if (yabs < 2)
                yabs = 2;

            /* Try to find a rectangle that fit our room ! */

            r2.lx = xabs - 1;
            r2.ly = yabs - 1;
            r2.hx = xabs + wtmp + rndpos;
            r2.hy = yabs + htmp + rndpos;
            r1 = get_rect(&r2);
        }
    } while (++trycnt <= 100 && !r1);
    if (!r1) { /* creation of room failed ? */
        return false;
    }
    split_rects(r1, &r2);

    if (!vault) {
        smeq[nroom] = nroom;
        add_room(xabs, yabs, xabs + wtmp - 1, yabs + htmp - 1, rlit, rtype, false);
    } else {
        rooms[nroom].lx = xabs;
        rooms[nroom].ly = yabs;
    }
    return true;
}

/*
 * Create a subroom in room proom at pos x,y with width w & height h.
 * x & y are relative to the parent room.
 */

static bool create_subroom(struct mkroom *proom, signed char x, signed char y, signed char w, signed char h, signed char rtype, signed char rlit) {
    signed char width, height;

    width = proom->hx - proom->lx + 1;
    height = proom->hy - proom->ly + 1;

    /* There is a minimum size for the parent room */
    if (width < 4 || height < 4)
        return false;

    /* Check for random position, size, etc... */

    if (w == -1)
        w = rnd(width - 3);
    if (h == -1)
        h = rnd(height - 3);
    if (x == -1)
        x = rnd(width - w - 1) - 1;
    if (y == -1)
        y = rnd(height - h - 1) - 1;
    if (x == 1)
        x = 0;
    if (y == 1)
        y = 0;
    if ((x + w + 1) == width)
        x++;
    if ((y + h + 1) == height)
        y++;
    if (rtype == -1)
        rtype = OROOM;
    if (rlit == -1)
        rlit = (rnd(1 + abs(depth(&u.uz))) < 11 && rn2(77)) ? true : false;
    add_subroom(proom, proom->lx + x, proom->ly + y, proom->lx + x + w - 1, proom->ly + y + h - 1, rlit, rtype, false);
    return true;
}

/*
 * Create a new door in a room.
 * It's placed on a wall (north, south, east or west).
 */

static void create_door(room_door *dd, struct mkroom *broom) {
    int x, y;
    int trycnt = 0;

    if (dd->secret == -1)
        dd->secret = rn2(2);

    if (dd->mask == -1) {
        /* is it a locked door, closed, or a doorway? */
        if (!dd->secret) {
            if (!rn2(3)) {
                if (!rn2(5))
                    dd->mask = D_ISOPEN;
                else if (!rn2(6))
                    dd->mask = D_LOCKED;
                else
                    dd->mask = D_CLOSED;
                if (dd->mask != D_ISOPEN && !rn2(25))
                    dd->mask |= D_TRAPPED;
            } else
                dd->mask = D_NODOOR;
        } else {
            if (!rn2(5))
                dd->mask = D_LOCKED;
            else
                dd->mask = D_CLOSED;

            if (!rn2(20))
                dd->mask |= D_TRAPPED;
        }
    }

    do {
        int dwall, dpos;

        dwall = dd->wall;
        if (dwall == -1) /* The wall is RANDOM */
            dwall = 1 << rn2(4);

        dpos = dd->pos;
        if (dpos == -1) /* The position is RANDOM */
            dpos = rn2((dwall == W_WEST || dwall == W_EAST) ? (broom->hy - broom->ly) : (broom->hx - broom->lx));

        /* Convert wall and pos into an absolute coordinate! */

        switch (dwall) {
            case W_NORTH:
                y = broom->ly - 1;
                x = broom->lx + dpos;
                break;
            case W_SOUTH:
                y = broom->hy + 1;
                x = broom->lx + dpos;
                break;
            case W_WEST:
                x = broom->lx - 1;
                y = broom->ly + dpos;
                break;
            case W_EAST:
                x = broom->hx + 1;
                y = broom->ly + dpos;
                break;
            default:
                x = y = 0;
                panic("create_door: No wall for door!");
                break;
        }
        if (okdoor(x, y))
            break;
    } while (++trycnt <= 100);
    if (trycnt > 100) {
        impossible("create_door: Can't find a proper place!");
        return;
    }
    add_door(x, y, broom);
    levl[x][y].typ = (dd->secret ? SDOOR : DOOR);
    levl[x][y].flags = dd->mask;
}

/*
 * Create a secret door in croom on any one of the specified walls.
 */
/* walls: any of W_NORTH | W_SOUTH | W_EAST | W_WEST (or W_ANY) */
void create_secret_door(struct mkroom *croom, signed char walls) {
    signed char sx, sy; /* location of the secret door */
    int count;

    for (count = 0; count < 100; count++) {
        sx = rn1(croom->hx - croom->lx + 1, croom->lx);
        sy = rn1(croom->hy - croom->ly + 1, croom->ly);

        switch (rn2(4)) {
            case 0: /* top */
                if (!(walls & W_NORTH))
                    continue;
                sy = croom->ly - 1;
                break;
            case 1: /* bottom */
                if (!(walls & W_SOUTH))
                    continue;
                sy = croom->hy + 1;
                break;
            case 2: /* left */
                if (!(walls & W_EAST))
                    continue;
                sx = croom->lx - 1;
                break;
            case 3: /* right */
                if (!(walls & W_WEST))
                    continue;
                sx = croom->hx + 1;
                break;
        }

        if (okdoor(sx, sy)) {
            levl[sx][sy].typ = SDOOR;
            levl[sx][sy].flags = D_CLOSED;
            add_door(sx, sy, croom);
            return;
        }
    }

    impossible("couldn't create secret door on any walls 0x%x", walls);
}

/*
 * Create a trap in a room.
 */

static void create_trap(trap *t, struct mkroom *croom) {
    signed char x, y;
    coord tm;

    if (rn2(100) < t->chance) {
        x = t->x;
        y = t->y;
        if (croom)
            get_free_room_loc(&x, &y, croom);
        else
            get_location(&x, &y, DRY);

        tm.x = x;
        tm.y = y;

        mktrap(t->type, 1, (struct mkroom*)0, &tm);
    }
}

/*
 * Create a monster in a room.
 */

static int noncoalignment(aligntyp alignment) {
    int k;

    k = rn2(2);
    if (!alignment)
        return (k ? -1 : 1);
    return (k ? -alignment : 0);
}

static void create_monster(monster *m, struct mkroom *croom) {
    struct monst *mtmp;
    signed char x, y;
    char class;
    aligntyp amask;
    coord cc;
    struct permonst *pm;
    unsigned g_mvflags;

    if (rn2(100) < m->chance) {

        if (m->class >= 0)
            class = (char)def_char_to_monclass((char)m->class);
        else if (m->class > -11)
            class = (char)def_char_to_monclass(rmonst[-m->class - 1]);
        else
            class = 0;

        if (class == MAXMCLASSES)
            panic("create_monster: unknown monster class '%c'", m->class);

        amask = (m->align == AM_SPLEV_CO) ? Align2amask(u.ualignbase[A_ORIGINAL]) : (m->align == AM_SPLEV_NONCO) ? Align2amask(noncoalignment(u.ualignbase[A_ORIGINAL])) : (m->align <= -11) ? induced_align(80) : (m->align < 0 ? ralign[-m->align - 1] : m->align);

        if (!class)
            pm = (struct permonst *)0;
        else if (m->id != NON_PM) {
            pm = &mons[m->id];
            g_mvflags = (unsigned)mvitals[monsndx(pm)].mvflags;
            if ((pm->geno & G_UNIQ) && (g_mvflags & G_EXTINCT))
                goto m_done;
            else if (g_mvflags & G_GONE) /* genocided or extinct */
                pm = (struct permonst *)0; /* make random monster */
        } else {
            pm = mkclass(class, G_NOGEN);
            /* if we can't get a specific monster type (pm == 0) then the
             class has been genocided, so settle for a random monster */
        }
        if (In_mines(&u.uz) && pm && your_race(pm) && (Race_if(PM_DWARF) || Race_if(PM_GNOME)) && rn2(3))
            pm = (struct permonst *)0;

        x = m->x;
        y = m->y;
        if (croom)
            get_room_loc(&x, &y, croom);
        else {
            if (!pm || !is_swimmer(pm))
                get_location(&x, &y, DRY);
            else if (pm->mlet == S_EEL)
                get_location(&x, &y, WET);
            else
                get_location(&x, &y, DRY | WET);
        }
        /* try to find a close place if someone else is already there */
        if (MON_AT(x,y) && enexto(&cc, x, y, pm))
            x = cc.x, y = cc.y;

        if (m->align != -12)
            mtmp = mk_roamer(pm, Amask2align(amask), x, y, m->peaceful);
        else if (PM_ARCHEOLOGIST <= m->id && m->id <= PM_WIZARD)
            mtmp = mk_mplayer(pm, x, y, false);
        else
            mtmp = makemon(pm, x, y, NO_MM_FLAGS);

        if (mtmp) {
            /* handle specific attributes for some special monsters */
            if (m->name.str)
                mtmp = christen_monst(mtmp, m->name.str);

            /*
             * This is currently hardwired for mimics only.  It should
             * eventually be expanded.
             */
            if (m->appear_as.str && mtmp->data->mlet == S_MIMIC) {
                int i;

                switch (m->appear) {
                    case M_AP_NOTHING:
                        impossible("create_monster: mon has an appearance, \"%s\", but no type", m->appear_as.str);
                        break;

                    case M_AP_FURNITURE:
                        for (i = 0; i < MAXPCHARS; i++)
                            if (!strcmp(defsyms[i].explanation, m->appear_as.str))
                                break;
                        if (i == MAXPCHARS) {
                            impossible("create_monster: can't find feature \"%s\"", m->appear_as.str);
                        } else {
                            mtmp->m_ap_type = M_AP_FURNITURE;
                            mtmp->mappearance = i;
                        }
                        break;

                    case M_AP_OBJECT:
                        for (i = 0; i < NUM_OBJECTS; i++)
                            if (OBJ_NAME(objects[i]) && !strcmp(OBJ_NAME(objects[i]), m->appear_as.str))
                                break;
                        if (i == NUM_OBJECTS) {
                            impossible("create_monster: can't find object \"%s\"", m->appear_as.str);
                        } else {
                            mtmp->m_ap_type = M_AP_OBJECT;
                            mtmp->mappearance = i;
                        }
                        break;

                    case M_AP_MONSTER:
                        /* note: mimics don't appear as monsters! */
                        /*       (but chameleons can :-)          */
                    default:
                        impossible("create_monster: unimplemented mon appear type [%d,\"%s\"]", m->appear, m->appear_as.str);
                        break;
                }
                if (does_block(x, y, &levl[x][y]))
                    block_point(x, y);
            }

            if (m->peaceful >= 0) {
                mtmp->mpeaceful = m->peaceful;
                /* changed mpeaceful again; have to reset malign */
                set_malign(mtmp);
            }
            if (m->asleep >= 0) {
                mtmp->msleeping = m->asleep;
            }
        }

    } /* if (rn2(100) < m->chance) */
    m_done:
    Free(m->name.str);
    Free(m->appear_as.str);
}

/*
 * Create an object in a room.
 */

static void create_object(object *o, struct mkroom *croom) {
    struct obj *otmp;
    signed char x, y;
    char c;
    bool named; /* has a name been supplied in level description? */

    if (rn2(100) < o->chance) {
        named = o->name.str ? true : false;

        x = o->x;
        y = o->y;
        if (croom)
            get_room_loc(&x, &y, croom);
        else
            get_location(&x, &y, DRY);

        if (o->class >= 0)
            c = o->class;
        else if (o->class > -11)
            c = robjects[-(o->class + 1)];
        else
            c = 0;

        if (!c)
            otmp = mkobj_at(RANDOM_CLASS, x, y, !named);
        else if (o->id != -1)
            otmp = mksobj_at(o->id, x, y, true, !named);
        else {
            /*
             * The special levels are compiled with the default "text" object
             * class characters.  We must convert them to the internal format.
             */
            char oclass = (char)def_char_to_objclass(c);

            if (oclass == MAXOCLASSES)
                panic("create_object:  unexpected object class '%c'", c);

            /* KMH -- Create piles of gold properly */
            if (oclass == COIN_CLASS)
                otmp = mkgold(0L, x, y);
            else
                otmp = mkobj_at(oclass, x, y, !named);
        }

        if (o->spe != -127) /* That means NOT RANDOM! */
            otmp->spe = (signed char)o->spe;

        switch (o->curse_state) {
            case 1:
                bless(otmp);
                break; /* BLESSED */
            case 2:
                unbless(otmp);
                uncurse(otmp);
                break; /* uncursed */
            case 3:
                curse(otmp);
                break; /* CURSED */
            default:
                break; /* Otherwise it's random and we're happy
                 * with what mkobj gave us! */
        }

        /*      corpsenm is "empty" if -1, random if -2, otherwise specific */
        if (o->corpsenm == NON_PM - 1)
            otmp->corpsenm = rndmonnum();
        else if (o->corpsenm != NON_PM)
            otmp->corpsenm = o->corpsenm;

        /* assume we wouldn't be given an egg corpsenm unless it was
         hatchable */
        if (otmp->otyp == EGG && otmp->corpsenm != NON_PM) {
            if (dead_species(otmp->otyp, true))
                kill_egg(otmp); /* make sure nothing hatches */
            else
                attach_egg_hatch_timeout(otmp); /* attach new hatch timeout */
        }

        if (named)
            otmp = oname(otmp, o->name.str);

        switch (o->containment) {
            static struct obj *container = 0;

            /* contents */
        case 1:
            if (!container) {
                impossible("create_object: no container");
                break;
            }
            remove_object(otmp);
            (void)add_to_container(container, otmp);
            goto o_done;
            /* don't stack, but do other cleanup */
            /* container */
        case 2:
            delete_contents(otmp);
            container = otmp;
            break;
            /* nothing */
        case 0:
            break;

        default:
            impossible("containment type %d?", (int)o->containment);
        }

        /* Medusa level special case: statues are petrified monsters, so they
         * are not stone-resistant and have monster inventory.  They also lack
         * other contents, but that can be specified as an empty container.
         */
        if (o->id == STATUE && Is_medusa_level(&u.uz) && o->corpsenm == NON_PM) {
            struct monst *was;
            struct obj *obj;
            int wastyp;

            /* Named random statues are of player types, and aren't stone-
             * resistant (if they were, we'd have to reset the name as well as
             * setting corpsenm).
             */
            for (wastyp = otmp->corpsenm;; wastyp = rndmonnum()) {
                /* makemon without rndmonst() might create a group */
                was = makemon(&mons[wastyp], 0, 0, NO_MM_FLAGS);
                if (!resists_ston(was))
                    break;
                mongone(was);
            }
            otmp->corpsenm = wastyp;
            while (was->minvent) {
                obj = was->minvent;
                obj->owornmask = 0;
                obj_extract_self(obj);
                (void)add_to_container(otmp, obj);
            }
            otmp->owt = weight(otmp);
            mongone(was);
        }

        stackobj(otmp);

    } /* if (rn2(100) < o->chance) */
    o_done:
    Free(o->name.str);
}

/*
 * Randomly place a specific engraving, then release its memory.
 */
static void create_engraving(engraving *e, struct mkroom *croom) {
    signed char x, y;

    x = e->x, y = e->y;
    if (croom)
        get_room_loc(&x, &y, croom);
    else
        get_location(&x, &y, DRY);

    make_engr_at(x, y, e->engr.str, 0L, e->etype);
    free((void *)e->engr.str);
}

/*
 * Create stairs in a room.
 *
 */

static void create_stairs(stair *s, struct mkroom *croom) {
    signed char x, y;

    x = s->x;
    y = s->y;
    get_free_room_loc(&x, &y, croom);
    mkstairs(x, y, (char)s->up, croom);
}

/*
 * Create an altar in a room.
 */

static void
create_altar (altar *a, struct mkroom *croom)
{
        signed char             sproom,x,y;
        aligntyp        amask;
        bool         croom_is_temple = true;
        int oldtyp;

        x = a->x; y = a->y;

        if (croom) {
            get_free_room_loc(&x, &y, croom);
            if (croom->rtype != TEMPLE)
                croom_is_temple = false;
        } else {
            get_location(&x, &y, DRY);
            if ((sproom = (signed char) *in_rooms(x, y, TEMPLE)) != 0)
                croom = &rooms[sproom - ROOMOFFSET];
            else
                croom_is_temple = false;
        }

        /* check for existing features */
        oldtyp = levl[x][y].typ;
        if (oldtyp == STAIRS || oldtyp == LADDER)
            return;

        a->x = x;
        a->y = y;

        /* Is the alignment random ?
         * If so, it's an 80% chance that the altar will be co-aligned.
         *
         * The alignment is encoded as amask values instead of alignment
         * values to avoid conflicting with the rest of the encoding,
         * shared by many other parts of the special level code.
         */

        amask = (a->align == AM_SPLEV_CO) ?
                        Align2amask(u.ualignbase[A_ORIGINAL]) :
                (a->align == AM_SPLEV_NONCO) ?
                        Align2amask(noncoalignment(u.ualignbase[A_ORIGINAL])) :
                (a->align == -11) ? induced_align(80) :
                (a->align < 0 ? ralign[-a->align-1] : a->align);

        levl[x][y].typ = ALTAR;
        levl[x][y].flags = amask;

        if (a->shrine < 0) a->shrine = rn2(2);  /* handle random case */

        if (oldtyp == FOUNTAIN)
            level.flags.nfountains--;
        else if (oldtyp == SINK)
            level.flags.nsinks--;

        if (!croom_is_temple || !a->shrine) return;

        if (a->shrine) {        /* Is it a shrine  or sanctum? */
            priestini(&u.uz, croom, x, y, (a->shrine > 1));
            levl[x][y].flags |= AM_SHRINE;
            level.flags.has_temple = true;
        }
}

/*
 * Create a gold pile in a room.
 */

static void
create_gold (gold *g, struct mkroom *croom)
{
        signed char             x,y;

        x = g->x; y= g->y;
        if (croom)
            get_room_loc(&x, &y, croom);
        else
            get_location(&x, &y, DRY);

        if (g->amount == -1)
            g->amount = rnd(200);
        (void) mkgold((long) g->amount, x, y);
}

/*
 * Create a feature (e.g a fountain) in a room.
 */

static void
create_feature (int fx, int fy, struct mkroom *croom, int typ)
{
        signed char             x,y;
        int             trycnt = 0;

        x = fx;  y = fy;
        if (croom) {
            if (x < 0 && y < 0)
                do {
                    x = -1;  y = -1;
                    get_room_loc(&x, &y, croom);
                } while (++trycnt <= 200 && occupied(x,y));
            else
                get_room_loc(&x, &y, croom);
            if(trycnt > 200)
                return;
        } else {
            get_location(&x, &y, DRY);
        }
        /* Don't cover up an existing feature (particularly randomly
           placed stairs).  However, if the _same_ feature is already
           here, it came from the map drawing and we still need to
           update the special counters. */
        if (IS_FURNITURE(levl[x][y].typ) && levl[x][y].typ != typ)
            return;

        levl[x][y].typ = typ;
        if (typ == FOUNTAIN)
            level.flags.nfountains++;
        else if (typ == SINK)
            level.flags.nsinks++;
}

/*
 * Search for a door in a room on a specified wall.
 */

static bool search_door(struct mkroom *croom, signed char *x, signed char *y,
        signed char wall, int cnt)
{
        int dx, dy;
        int xx,yy;

        switch(wall) {
              case W_NORTH:
                dy = 0; dx = 1;
                xx = croom->lx;
                yy = croom->hy + 1;
                break;
              case W_SOUTH:
                dy = 0; dx = 1;
                xx = croom->lx;
                yy = croom->ly - 1;
                break;
              case W_EAST:
                dy = 1; dx = 0;
                xx = croom->hx + 1;
                yy = croom->ly;
                break;
              case W_WEST:
                dy = 1; dx = 0;
                xx = croom->lx - 1;
                yy = croom->ly;
                break;
              default:
                dx = dy = xx = yy = 0;
                panic("search_door: Bad wall!");
                break;
        }
        while (xx <= croom->hx+1 && yy <= croom->hy+1) {
                if (IS_DOOR(levl[xx][yy].typ) || levl[xx][yy].typ == SDOOR) {
                        *x = xx;
                        *y = yy;
                        if (cnt-- <= 0)
                            return true;
                }
                xx += dx;
                yy += dy;
        }
        return false;
}

/*
 * Dig a corridor between two points.
 */

bool 
dig_corridor (coord *org, coord *dest, bool nxcor, signed char ftyp, signed char btyp)
{
        int dx=0, dy=0, dix, diy, cct;
        struct rm *crm;
        int tx, ty, xx, yy;

        xx = org->x;  yy = org->y;
        tx = dest->x; ty = dest->y;
        if (xx <= 0 || yy <= 0 || tx <= 0 || ty <= 0 ||
            xx > COLNO-1 || tx > COLNO-1 ||
            yy > ROWNO-1 || ty > ROWNO-1) {
                return false;
        }
        if (tx > xx)            dx = 1;
        else if (ty > yy)       dy = 1;
        else if (tx < xx)       dx = -1;
        else                    dy = -1;

        xx -= dx;
        yy -= dy;
        cct = 0;
        while(xx != tx || yy != ty) {
            /* loop: dig corridor at [xx,yy] and find new [xx,yy] */
            if(cct++ > 500 || (nxcor && !rn2(35)))
                return false;

            xx += dx;
            yy += dy;

            if(xx >= COLNO-1 || xx <= 0 || yy <= 0 || yy >= ROWNO-1)
                return false;           /* impossible */

            crm = &levl[xx][yy];
            if(crm->typ == btyp) {
                if(ftyp != CORR || rn2(100)) {
                        crm->typ = ftyp;
                        if(nxcor && !rn2(50))
                                (void) mksobj_at(BOULDER, xx, yy, true, false);
                } else {
                        crm->typ = SCORR;
                }
            } else
            if(crm->typ != ftyp && crm->typ != SCORR) {
                /* strange ... */
                return false;
            }

            /* find next corridor position */
            dix = abs(xx-tx);
            diy = abs(yy-ty);

            /* do we have to change direction ? */
            if(dy && dix > diy) {
                int ddx = (xx > tx) ? -1 : 1;

                crm = &levl[xx+ddx][yy];
                if(crm->typ == btyp || crm->typ == ftyp || crm->typ == SCORR) {
                    dx = ddx;
                    dy = 0;
                    continue;
                }
            } else if(dx && diy > dix) {
                int ddy = (yy > ty) ? -1 : 1;

                crm = &levl[xx][yy+ddy];
                if(crm->typ == btyp || crm->typ == ftyp || crm->typ == SCORR) {
                    dy = ddy;
                    dx = 0;
                    continue;
                }
            }

            /* continue straight on? */
            crm = &levl[xx+dx][yy+dy];
            if(crm->typ == btyp || crm->typ == ftyp || crm->typ == SCORR)
                continue;

            /* no, what must we do now?? */
            if(dx) {
                dx = 0;
                dy = (ty < yy) ? -1 : 1;
            } else {
                dy = 0;
                dx = (tx < xx) ? -1 : 1;
            }
            crm = &levl[xx+dx][yy+dy];
            if(crm->typ == btyp || crm->typ == ftyp || crm->typ == SCORR)
                continue;
            dy = -dy;
            dx = -dx;
        }
        return true;
}

/*
 * Disgusting hack: since special levels have their rooms filled before
 * sorting the rooms, we have to re-arrange the speed values upstairs_room
 * and dnstairs_room after the rooms have been sorted.  On normal levels,
 * stairs don't get created until _after_ sorting takes place.
 */
static void
fix_stair_rooms (void)
{
    int i;
    struct mkroom *croom;

    if(xdnstair &&
       !((dnstairs_room->lx <= xdnstair && xdnstair <= dnstairs_room->hx) &&
         (dnstairs_room->ly <= ydnstair && ydnstair <= dnstairs_room->hy))) {
        for(i=0; i < nroom; i++) {
            croom = &rooms[i];
            if((croom->lx <= xdnstair && xdnstair <= croom->hx) &&
               (croom->ly <= ydnstair && ydnstair <= croom->hy)) {
                dnstairs_room = croom;
                break;
            }
        }
        if(i == nroom)
            panic("Couldn't find dnstair room in fix_stair_rooms!");
    }
    if(xupstair &&
       !((upstairs_room->lx <= xupstair && xupstair <= upstairs_room->hx) &&
         (upstairs_room->ly <= yupstair && yupstair <= upstairs_room->hy))) {
        for(i=0; i < nroom; i++) {
            croom = &rooms[i];
            if((croom->lx <= xupstair && xupstair <= croom->hx) &&
               (croom->ly <= yupstair && yupstair <= croom->hy)) {
                upstairs_room = croom;
                break;
            }
        }
        if(i == nroom)
            panic("Couldn't find upstair room in fix_stair_rooms!");
    }
}

/*
 * Corridors always start from a door. But it can end anywhere...
 * Basically we search for door coordinates or for endpoints coordinates
 * (from a distance).
 */

static void
create_corridor (corridor *c)
{
        coord org, dest;

        if (c->src.room == -1) {
                sort_rooms();
                fix_stair_rooms();
                makecorridors();
                return;
        }

        if( !search_door(&rooms[c->src.room], &org.x, &org.y, c->src.wall,
                         c->src.door))
            return;

        if (c->dest.room != -1) {
                if(!search_door(&rooms[c->dest.room], &dest.x, &dest.y,
                                c->dest.wall, c->dest.door))
                    return;
                switch(c->src.wall) {
                      case W_NORTH: org.y--; break;
                      case W_SOUTH: org.y++; break;
                      case W_WEST:  org.x--; break;
                      case W_EAST:  org.x++; break;
                }
                switch(c->dest.wall) {
                      case W_NORTH: dest.y--; break;
                      case W_SOUTH: dest.y++; break;
                      case W_WEST:  dest.x--; break;
                      case W_EAST:  dest.x++; break;
                }
                (void) dig_corridor(&org, &dest, false, CORR, STONE);
        }
}


/*
 * Fill a room (shop, zoo, etc...) with appropriate stuff.
 */

void 
fill_room (struct mkroom *croom, bool prefilled)
{
        if (!croom || croom->rtype == OROOM)
            return;

        if (!prefilled) {
            int x,y;

            /* Shop ? */
            if (croom->rtype >= SHOPBASE) {
                    stock_room(croom->rtype - SHOPBASE, croom);
                    level.flags.has_shop = true;
                    return;
            }

            switch (croom->rtype) {
                case VAULT:
                    for (x=croom->lx;x<=croom->hx;x++)
                        for (y=croom->ly;y<=croom->hy;y++)
                            (void) mkgold((long)rn1(abs(depth(&u.uz))*100, 51), x, y);
                    break;
                case COURT:
                case ZOO:
                case BEEHIVE:
                case MORGUE:
                case BARRACKS:
                    fill_zoo(croom);
                    break;
            }
        }
        switch (croom->rtype) {
            case VAULT:
                level.flags.has_vault = true;
                break;
            case ZOO:
                level.flags.has_zoo = true;
                break;
            case COURT:
                level.flags.has_court = true;
                break;
            case MORGUE:
                level.flags.has_morgue = true;
                break;
            case BEEHIVE:
                level.flags.has_beehive = true;
                break;
            case BARRACKS:
                level.flags.has_barracks = true;
                break;
            case TEMPLE:
                level.flags.has_temple = true;
                break;
            case SWAMP:
                level.flags.has_swamp = true;
                break;
        }
}

static void
free_rooms (room **ro, int n)
{
        short j;
        room *r;

        while(n--) {
                r = ro[n];
                Free(r->name);
                Free(r->parent);
                if ((j = r->ndoor) != 0) {
                        while(j--)
                            Free(r->doors[j]);
                        Free(r->doors);
                }
                if ((j = r->nstair) != 0) {
                        while(j--)
                            Free(r->stairs[j]);
                        Free(r->stairs);
                }
                if ((j = r->naltar) != 0) {
                        while (j--)
                            Free(r->altars[j]);
                        Free(r->altars);
                }
                if ((j = r->nfountain) != 0) {
                        while(j--)
                            Free(r->fountains[j]);
                        Free(r->fountains);
                }
                if ((j = r->nsink) != 0) {
                        while(j--)
                            Free(r->sinks[j]);
                        Free(r->sinks);
                }
                if ((j = r->npool) != 0) {
                        while(j--)
                            Free(r->pools[j]);
                        Free(r->pools);
                }
                if ((j = r->ntrap) != 0) {
                        while (j--)
                            Free(r->traps[j]);
                        Free(r->traps);
                }
                if ((j = r->nmonster) != 0) {
                        while (j--)
                                Free(r->monsters[j]);
                        Free(r->monsters);
                }
                if ((j = r->nobject) != 0) {
                        while (j--)
                                Free(r->objects[j]);
                        Free(r->objects);
                }
                if ((j = r->ngold) != 0) {
                        while(j--)
                            Free(r->golds[j]);
                        Free(r->golds);
                }
                if ((j = r->nengraving) != 0) {
                        while (j--)
                                Free(r->engravings[j]);
                        Free(r->engravings);
                }
                Free(r);
        }
        Free(ro);
}

static void
build_room (room *r, room *pr)
{
        bool okroom;
        struct mkroom   *aroom;
        short i;
        signed char rtype = (!r->chance || rn2(100) < r->chance) ? r->rtype : OROOM;

        if(pr) {
                aroom = &subrooms[nsubroom];
                okroom = create_subroom(pr->mkr, r->x, r->y, r->w, r->h,
                                        rtype, r->rlit);
        } else {
                aroom = &rooms[nroom];
                okroom = create_room(r->x, r->y, r->w, r->h, r->xalign,
                                     r->yalign, rtype, r->rlit);
                r->mkr = aroom;
        }

        if (okroom) {
                /* Create subrooms if necessary... */
                for(i=0; i < r->nsubroom; i++)
                    build_room(r->subrooms[i], r);
                /* And now we can fill the room! */

                /* Priority to the stairs */

                for(i=0; i <r->nstair; i++)
                    create_stairs(r->stairs[i], aroom);

                /* Then to the various elements (sinks, etc..) */
                for(i = 0; i<r->nsink; i++)
                    create_feature(r->sinks[i]->x, r->sinks[i]->y, aroom, SINK);
                for(i = 0; i<r->npool; i++)
                    create_feature(r->pools[i]->x, r->pools[i]->y, aroom, POOL);
                for(i = 0; i<r->nfountain; i++)
                    create_feature(r->fountains[i]->x, r->fountains[i]->y,
                                   aroom, FOUNTAIN);
                for(i = 0; i<r->naltar; i++)
                    create_altar(r->altars[i], aroom);
                for(i = 0; i<r->ndoor; i++)
                    create_door(r->doors[i], aroom);

                /* The traps */
                for(i = 0; i<r->ntrap; i++)
                    create_trap(r->traps[i], aroom);

                /* The monsters */
                for(i = 0; i<r->nmonster; i++)
                    create_monster(r->monsters[i], aroom);

                /* The objects */
                for(i = 0; i<r->nobject; i++)
                    create_object(r->objects[i], aroom);

                /* The gold piles */
                for(i = 0; i<r->ngold; i++)
                    create_gold(r->golds[i], aroom);

                /* The engravings */
                for (i = 0; i < r->nengraving; i++)
                    create_engraving(r->engravings[i], aroom);

                topologize(aroom);                      /* set roomno */
                /* MRS - 07/04/91 - This is temporary but should result
                 * in proper filling of shops, etc.
                 * DLC - this can fail if corridors are added to this room
                 * at a later point.  Currently no good way to fix this.
                 */
                if(aroom->rtype != OROOM && r->filled) fill_room(aroom, false);
        }
}

/*
 * set lighting in a region that will not become a room.
 */
static void
light_region (region *tmpregion)
{
    bool litstate = tmpregion->rlit ? 1 : 0;
    int hiy = tmpregion->y2;
    int x, y;
    struct rm *lev;
    int lowy = tmpregion->y1;
    int lowx = tmpregion->x1, hix = tmpregion->x2;

    if(litstate) {
        /* adjust region size for walls, but only if lighted */
        lowx = max(lowx-1,1);
        hix = min(hix+1,COLNO-1);
        lowy = max(lowy-1,0);
        hiy = min(hiy+1, ROWNO-1);
    }
    for(x = lowx; x <= hix; x++) {
        lev = &levl[x][lowy];
        for(y = lowy; y <= hiy; y++) {
            if (lev->typ != LAVAPOOL) /* this overrides normal lighting */
                lev->lit = litstate;
            lev++;
        }
    }
}

/* initialization common to all special levels */
static void
load_common_data (dlb *fd, int typ)
{
        unsigned char   n;
        long    lev_flags;
        int     i;

      {
        aligntyp atmp;
        /* shuffle 3 alignments; can't use sp_lev_shuffle() on aligntyp's */
        i = rn2(3);   atmp=ralign[2]; ralign[2]=ralign[i]; ralign[i]=atmp;
        if (rn2(2)) { atmp=ralign[1]; ralign[1]=ralign[0]; ralign[0]=atmp; }
      }

        level.flags.is_maze_lev = typ == SP_LEV_MAZE;

        /* Read the level initialization data */
        Fread((void *) &init_lev, 1, sizeof(lev_init), fd);
        if(init_lev.init_present) {
            if(init_lev.lit < 0)
                init_lev.lit = rn2(2);
            mkmap(&init_lev);
        }

        /* Read the per level flags */
        Fread((void *) &lev_flags, 1, sizeof(lev_flags), fd);
        if (lev_flags & NOTELEPORT)
            level.flags.noteleport = 1;
        if (lev_flags & HARDFLOOR)
            level.flags.hardfloor = 1;
        if (lev_flags & NOMMAP)
            level.flags.nommap = 1;
        if (lev_flags & SHORTSIGHTED)
            level.flags.shortsighted = 1;
        if (lev_flags & ARBOREAL)
            level.flags.arboreal = 1;

        /* Read message */
        Fread((void *) &n, 1, sizeof(n), fd);
        if (n) {
            lev_message = (char *) malloc(n + 1);
            Fread((void *) lev_message, 1, (int) n, fd);
            lev_message[n] = 0;
        }
}

static void
load_one_monster (dlb *fd, monster *m)
{
        int size;

        Fread((void *) m, 1, sizeof *m, fd);
        if ((size = m->name.len) != 0) {
            m->name.str = (char *) malloc((unsigned)size + 1);
            Fread((void *) m->name.str, 1, size, fd);
            m->name.str[size] = '\0';
        } else
            m->name.str = (char *) 0;
        if ((size = m->appear_as.len) != 0) {
            m->appear_as.str = (char *) malloc((unsigned)size + 1);
            Fread((void *) m->appear_as.str, 1, size, fd);
            m->appear_as.str[size] = '\0';
        } else
            m->appear_as.str = (char *) 0;
}

static void
load_one_object (dlb *fd, object *o)
{
        int size;

        Fread((void *) o, 1, sizeof *o, fd);
        if ((size = o->name.len) != 0) {
            o->name.str = (char *) malloc((unsigned)size + 1);
            Fread((void *) o->name.str, 1, size, fd);
            o->name.str[size] = '\0';
        } else
            o->name.str = (char *) 0;
}

static void
load_one_engraving (dlb *fd, engraving *e)
{
        int size;

        Fread((void *) e, 1, sizeof *e, fd);
        size = e->engr.len;
        e->engr.str = (char *) malloc((unsigned)size+1);
        Fread((void *) e->engr.str, 1, size, fd);
        e->engr.str[size] = '\0';
}

static bool 
load_rooms (dlb *fd)
{
        signed char             nrooms, ncorr;
        char            n;
        short           size;
        corridor        tmpcor;
        room**          tmproom;
        int             i, j;

        load_common_data(fd, SP_LEV_ROOMS);

        Fread((void *) &n, 1, sizeof(n), fd); /* nrobjects */
        if (n) {
                Fread((void *)robjects, sizeof(*robjects), n, fd);
                sp_lev_shuffle(robjects, (char *)0, (int)n);
        }

        Fread((void *) &n, 1, sizeof(n), fd); /* nrmonst */
        if (n) {
                Fread((void *)rmonst, sizeof(*rmonst), n, fd);
                sp_lev_shuffle(rmonst, (char *)0, (int)n);
        }

        Fread((void *) &nrooms, 1, sizeof(nrooms), fd);
                                                /* Number of rooms to read */
        tmproom = NewTab(room,nrooms);
        for (i=0;i<nrooms;i++) {
                room *r;

                r = tmproom[i] = New(room);

                /* Let's see if this room has a name */
                Fread((void *) &size, 1, sizeof(size), fd);
                if (size > 0) { /* Yup, it does! */
                        r->name = (char *) malloc((unsigned)size + 1);
                        Fread((void *) r->name, 1, size, fd);
                        r->name[size] = 0;
                } else
                    r->name = (char *) 0;

                /* Let's see if this room has a parent */
                Fread((void *) &size, 1, sizeof(size), fd);
                if (size > 0) { /* Yup, it does! */
                        r->parent = (char *) malloc((unsigned)size + 1);
                        Fread((void *) r->parent, 1, size, fd);
                        r->parent[size] = 0;
                } else
                    r->parent = (char *) 0;

                Fread((void *) &r->x, 1, sizeof(r->x), fd);
                                        /* x pos on the grid (1-5) */
                Fread((void *) &r->y, 1, sizeof(r->y), fd);
                                         /* y pos on the grid (1-5) */
                Fread((void *) &r->w, 1, sizeof(r->w), fd);
                                         /* width of the room */
                Fread((void *) &r->h, 1, sizeof(r->h), fd);
                                         /* height of the room */
                Fread((void *) &r->xalign, 1, sizeof(r->xalign), fd);
                                         /* horizontal alignment */
                Fread((void *) &r->yalign, 1, sizeof(r->yalign), fd);
                                         /* vertical alignment */
                Fread((void *) &r->rtype, 1, sizeof(r->rtype), fd);
                                         /* type of room (zoo, shop, etc.) */
                Fread((void *) &r->chance, 1, sizeof(r->chance), fd);
                                         /* chance of room being special. */
                Fread((void *) &r->rlit, 1, sizeof(r->rlit), fd);
                                         /* lit or not ? */
                Fread((void *) &r->filled, 1, sizeof(r->filled), fd);
                                         /* to be filled? */
                r->nsubroom= 0;

                /* read the doors */
                Fread((void *) &r->ndoor, 1, sizeof(r->ndoor), fd);
                if ((n = r->ndoor) != 0)
                    r->doors = NewTab(room_door, n);
                while(n--) {
                        r->doors[(int)n] = New(room_door);
                        Fread((void *) r->doors[(int)n], 1,
                                sizeof(room_door), fd);
                }

                /* read the stairs */
                Fread((void *) &r->nstair, 1, sizeof(r->nstair), fd);
                if ((n = r->nstair) != 0)
                    r->stairs = NewTab(stair, n);
                while (n--) {
                        r->stairs[(int)n] = New(stair);
                        Fread((void *) r->stairs[(int)n], 1,
                                sizeof(stair), fd);
                }

                /* read the altars */
                Fread((void *) &r->naltar, 1, sizeof(r->naltar), fd);
                if ((n = r->naltar) != 0)
                    r->altars = NewTab(altar, n);
                while (n--) {
                        r->altars[(int)n] = New(altar);
                        Fread((void *) r->altars[(int)n], 1,
                                sizeof(altar), fd);
                }

                /* read the fountains */
                Fread((void *) &r->nfountain, 1,
                        sizeof(r->nfountain), fd);
                if ((n = r->nfountain) != 0)
                    r->fountains = NewTab(fountain, n);
                while (n--) {
                        r->fountains[(int)n] = New(fountain);
                        Fread((void *) r->fountains[(int)n], 1,
                                sizeof(fountain), fd);
                }

                /* read the sinks */
                Fread((void *) &r->nsink, 1, sizeof(r->nsink), fd);
                if ((n = r->nsink) != 0)
                    r->sinks = NewTab(sink, n);
                while (n--) {
                        r->sinks[(int)n] = New(sink);
                        Fread((void *) r->sinks[(int)n], 1, sizeof(sink), fd);
                }

                /* read the pools */
                Fread((void *) &r->npool, 1, sizeof(r->npool), fd);
                if ((n = r->npool) != 0)
                    r->pools = NewTab(pool,n);
                while (n--) {
                        r->pools[(int)n] = New(pool);
                        Fread((void *) r->pools[(int)n], 1, sizeof(pool), fd);
                }

                /* read the traps */
                Fread((void *) &r->ntrap, 1, sizeof(r->ntrap), fd);
                if ((n = r->ntrap) != 0)
                    r->traps = NewTab(trap, n);
                while(n--) {
                        r->traps[(int)n] = New(trap);
                        Fread((void *) r->traps[(int)n], 1, sizeof(trap), fd);
                }

                /* read the monsters */
                Fread((void *) &r->nmonster, 1, sizeof(r->nmonster), fd);
                if ((n = r->nmonster) != 0) {
                    r->monsters = NewTab(monster, n);
                    while(n--) {
                        r->monsters[(int)n] = New(monster);
                        load_one_monster(fd, r->monsters[(int)n]);
                    }
                } else
                    r->monsters = 0;

                /* read the objects, in same order as mazes */
                Fread((void *) &r->nobject, 1, sizeof(r->nobject), fd);
                if ((n = r->nobject) != 0) {
                    r->objects = NewTab(object, n);
                    for (j = 0; j < n; ++j) {
                        r->objects[j] = New(object);
                        load_one_object(fd, r->objects[j]);
                    }
                } else
                    r->objects = 0;

                /* read the gold piles */
                Fread((void *) &r->ngold, 1, sizeof(r->ngold), fd);
                if ((n = r->ngold) != 0)
                    r->golds = NewTab(gold, n);
                while (n--) {
                        r->golds[(int)n] = New(gold);
                        Fread((void *) r->golds[(int)n], 1, sizeof(gold), fd);
                }

                /* read the engravings */
                Fread((void *) &r->nengraving, 1,
                        sizeof(r->nengraving), fd);
                if ((n = r->nengraving) != 0) {
                    r->engravings = NewTab(engraving,n);
                    while (n--) {
                        r->engravings[(int)n] = New(engraving);
                        load_one_engraving(fd, r->engravings[(int)n]);
                    }
                } else
                    r->engravings = 0;

        }

        /* Now that we have loaded all the rooms, search the
         * subrooms and create the links.
         */

        for (i = 0; i<nrooms; i++)
            if (tmproom[i]->parent) {
                    /* Search the parent room */
                    for(j=0; j<nrooms; j++)
                        if (tmproom[j]->name && !strcmp(tmproom[j]->name,
                                                       tmproom[i]->parent)) {
                                n = tmproom[j]->nsubroom++;
                                tmproom[j]->subrooms[(int)n] = tmproom[i];
                                break;
                        }
            }

        /*
         * Create the rooms now...
         */

        for (i=0; i < nrooms; i++)
            if(!tmproom[i]->parent)
                build_room(tmproom[i], (room *) 0);

        free_rooms(tmproom, nrooms);

        /* read the corridors */

        Fread((void *) &ncorr, sizeof(ncorr), 1, fd);
        for (i=0; i<ncorr; i++) {
                Fread((void *) &tmpcor, 1, sizeof(tmpcor), fd);
                create_corridor(&tmpcor);
        }

        return true;
}

/*
 * Select a random coordinate in the maze.
 *
 * We want a place not 'touched' by the loader.  That is, a place in
 * the maze outside every part of the special level.
 */

static void
maze1xy (coord *m, int humidity)
{
        int x, y, tryct = 2000;
        /* tryct:  normally it won't take more than ten or so tries due
           to the circumstances under which we'll be called, but the
           `humidity' screening might drastically change the chances */

        do {
            x = rn1(x_maze_max - 3, 3);
            y = rn1(y_maze_max - 3, 3);
            if (--tryct < 0) break;     /* give up */
        } while (!(x % 2) || !(y % 2) || Map[x][y] ||
                 !is_ok_location((signed char)x, (signed char)y, humidity));

        m->x = (signed char)x,  m->y = (signed char)y;
}

/*
 * The Big Thing: special maze loader
 *
 * Could be cleaner, but it works.
 */

static bool 
load_maze (dlb *fd)
{
    signed char   x, y, typ;
    bool prefilled, room_not_needed;

    char    n, numpart = 0;
    signed char   nwalk = 0, nwalk_sav;
    signed char   filling;
    char    halign, valign;

    int     xi, dir, size;
    coord   mm;
    int     mapcount, mapcountmax, mapfact;

    lev_region  tmplregion;
    region  tmpregion;
    door    tmpdoor;
    trap    tmptrap;
    monster tmpmons;
    object  tmpobj;
    drawbridge tmpdb;
    walk    tmpwalk;
    digpos  tmpdig;
    lad     tmplad;
    stair   tmpstair, prevstair;
    altar   tmpaltar;
    gold    tmpgold;
    fountain tmpfountain;
    engraving tmpengraving;
    signed char   mustfill[(MAXNROFROOMS+1)*2];
    struct trap *badtrap;
    bool has_bounds;

    (void) memset((void *)&Map[0][0], 0, sizeof Map);
    load_common_data(fd, SP_LEV_MAZE);

    /* Initialize map */
    Fread((void *) &filling, 1, sizeof(filling), fd);
    if (!init_lev.init_present) { /* don't init if mkmap() has been called */
      for(x = 2; x <= x_maze_max; x++)
        for(y = 0; y <= y_maze_max; y++)
            if (filling == -1) {
                    levl[x][y].typ =
                        (y < 2 || ((x % 2) && (y % 2))) ? STONE : HWALL;
            } else {
                    levl[x][y].typ = filling;
            }
    }

    /* Start reading the file */
    Fread((void *) &numpart, 1, sizeof(numpart), fd);
                                                /* Number of parts */
    if (!numpart || numpart > 9)
        panic("load_maze error: numpart = %d", (int) numpart);

    while (numpart--) {
        Fread((void *) &halign, 1, sizeof(halign), fd);
                                        /* Horizontal alignment */
        Fread((void *) &valign, 1, sizeof(valign), fd);
                                        /* Vertical alignment */
        Fread((void *) &xsize, 1, sizeof(xsize), fd);
                                        /* size in X */
        Fread((void *) &ysize, 1, sizeof(ysize), fd);
                                        /* size in Y */
        switch((int) halign) {
            case LEFT:      xstart = 3;                                 break;
            case H_LEFT:    xstart = 2+((x_maze_max-2-xsize)/4);        break;
            case CENTER:    xstart = 2+((x_maze_max-2-xsize)/2);        break;
            case H_RIGHT:   xstart = 2+((x_maze_max-2-xsize)*3/4);      break;
            case RIGHT:     xstart = x_maze_max-xsize-1;                break;
        }
        switch((int) valign) {
            case TOP:       ystart = 3;                                 break;
            case CENTER:    ystart = 2+((y_maze_max-2-ysize)/2);        break;
            case BOTTOM:    ystart = y_maze_max-ysize-1;                break;
        }
        if (!(xstart % 2)) xstart++;
        if (!(ystart % 2)) ystart++;
        if ((ystart < 0) || (ystart + ysize > ROWNO)) {
            /* try to move the start a bit */
            ystart += (ystart > 0) ? -2 : 2;
            if(ysize == ROWNO) ystart = 0;
            if(ystart < 0 || ystart + ysize > ROWNO)
                panic("reading special level with ysize too large");
        }

        /*
         * If any CROSSWALLs are found, must change to ROOM after REGION's
         * are laid out.  CROSSWALLS are used to specify "invisible"
         * boundaries where DOOR syms look bad or aren't desirable.
         */
        has_bounds = false;

        if(init_lev.init_present && xsize <= 1 && ysize <= 1) {
            xstart = 1;
            ystart = 0;
            xsize = COLNO-1;
            ysize = ROWNO;
        } else {
            /* Load the map */
            for(y = ystart; y < ystart+ysize; y++)
                for(x = xstart; x < xstart+xsize; x++) {
                    levl[x][y].typ = Fgetc(fd);
                    levl[x][y].lit = false;
                    /* clear out levl: load_common_data may set them */
                    levl[x][y].flags = 0;
                    levl[x][y].horizontal = 0;
                    levl[x][y].roomno = 0;
                    levl[x][y].edge = 0;
                    /*
                     * Note: Even though levl[x][y].typ is type signed char,
                     *   lev_comp.y saves it as type char. Since signed char != char
                     *   all the time we must make this exception or hack
                     *   through lev_comp.y to fix.
                     */

                    /*
                     *  Set secret doors to closed (why not trapped too?).  Set
                     *  the horizontal bit.
                     */
                    if (levl[x][y].typ == SDOOR || IS_DOOR(levl[x][y].typ)) {
                        if(levl[x][y].typ == SDOOR)
                            levl[x][y].flags = D_CLOSED;
                        /*
                         *  If there is a wall to the left that connects to a
                         *  (secret) door, then it is horizontal.  This does
                         *  not allow (secret) doors to be corners of rooms.
                         */
                        if (x != xstart && (IS_WALL(levl[x-1][y].typ) ||
                                            levl[x-1][y].horizontal))
                            levl[x][y].horizontal = 1;
                    } else if(levl[x][y].typ == HWALL ||
                                levl[x][y].typ == IRONBARS)
                        levl[x][y].horizontal = 1;
                    else if(levl[x][y].typ == LAVAPOOL)
                        levl[x][y].lit = 1;
                    else if(levl[x][y].typ == CROSSWALL)
                        has_bounds = true;
                    Map[x][y] = 1;
                }
            if (init_lev.init_present && init_lev.joined)
                remove_rooms(xstart, ystart, xstart+xsize, ystart+ysize);
        }

        Fread((void *) &n, 1, sizeof(n), fd);
                                                /* Number of level regions */
        if(n) {
            if(num_lregions) {
                /* realloc the lregion space to add the new ones */
                /* don't really free it up until the whole level is done */
                lev_region *newl = (lev_region *) malloc(sizeof(lev_region) *
                                                (unsigned)(n+num_lregions));
                (void) memcpy((void *)(newl+n), (void *)lregions,
                                        sizeof(lev_region) * num_lregions);
                Free(lregions);
                num_lregions += n;
                lregions = newl;
            } else {
                num_lregions = n;
                lregions = (lev_region *)
                                malloc(sizeof(lev_region) * (unsigned)n);
            }
        }

        while(n--) {
            Fread((void *) &tmplregion, sizeof(tmplregion), 1, fd);
            if ((size = tmplregion.rname.len) != 0) {
                tmplregion.rname.str = (char *) malloc((unsigned)size + 1);
                Fread((void *) tmplregion.rname.str, size, 1, fd);
                tmplregion.rname.str[size] = '\0';
            } else
                tmplregion.rname.str = (char *) 0;
            if(!tmplregion.in_islev) {
                get_location(&tmplregion.inarea.x1, &tmplregion.inarea.y1,
                                                                DRY|WET);
                get_location(&tmplregion.inarea.x2, &tmplregion.inarea.y2,
                                                                DRY|WET);
            }
            if(!tmplregion.del_islev) {
                get_location(&tmplregion.delarea.x1, &tmplregion.delarea.y1,
                                                                DRY|WET);
                get_location(&tmplregion.delarea.x2, &tmplregion.delarea.y2,
                                                                DRY|WET);
            }
            lregions[(int)n] = tmplregion;
        }

        Fread((void *) &n, 1, sizeof(n), fd);
                                                /* Random objects */
        if(n) {
                Fread((void *)robjects, sizeof(*robjects), (int) n, fd);
                sp_lev_shuffle(robjects, (char *)0, (int)n);
        }

        Fread((void *) &n, 1, sizeof(n), fd);
                                                /* Random locations */
        if(n) {
                Fread((void *)rloc_x, sizeof(*rloc_x), (int) n, fd);
                Fread((void *)rloc_y, sizeof(*rloc_y), (int) n, fd);
                sp_lev_shuffle(rloc_x, rloc_y, (int)n);
        }

        Fread((void *) &n, 1, sizeof(n), fd);
                                                /* Random monsters */
        if(n) {
                Fread((void *)rmonst, sizeof(*rmonst), (int) n, fd);
                sp_lev_shuffle(rmonst, (char *)0, (int)n);
        }

        (void) memset((void *)mustfill, 0, sizeof(mustfill));
        Fread((void *) &n, 1, sizeof(n), fd);
                                                /* Number of subrooms */
        while(n--) {
                struct mkroom *troom;

                Fread((void *)&tmpregion, 1, sizeof(tmpregion), fd);

                if(tmpregion.rtype > MAXRTYPE) {
                    tmpregion.rtype -= MAXRTYPE+1;
                    prefilled = true;
                } else
                    prefilled = false;

                if(tmpregion.rlit < 0)
                    tmpregion.rlit = (rnd(1+abs(depth(&u.uz))) < 11 && rn2(77))
                        ? true : false;

                get_location(&tmpregion.x1, &tmpregion.y1, DRY|WET);
                get_location(&tmpregion.x2, &tmpregion.y2, DRY|WET);

                /* for an ordinary room, `prefilled' is a flag to force
                   an actual room to be created (such rooms are used to
                   control placement of migrating monster arrivals) */
                room_not_needed = (tmpregion.rtype == OROOM &&
                                   !tmpregion.rirreg && !prefilled);
                if (room_not_needed || nroom >= MAXNROFROOMS) {
                    if (!room_not_needed)
                        impossible("Too many rooms on new level!");
                    light_region(&tmpregion);
                    continue;
                }

                troom = &rooms[nroom];

                /* mark rooms that must be filled, but do it later */
                if (tmpregion.rtype != OROOM)
                    mustfill[nroom] = (prefilled ? 2 : 1);

                if(tmpregion.rirreg) {
                    min_rx = max_rx = tmpregion.x1;
                    min_ry = max_ry = tmpregion.y1;
                    flood_fill_rm(tmpregion.x1, tmpregion.y1,
                                  nroom+ROOMOFFSET, tmpregion.rlit, true);
                    add_room(min_rx, min_ry, max_rx, max_ry,
                             false, tmpregion.rtype, true);
                    troom->rlit = tmpregion.rlit;
                    troom->irregular = true;
                } else {
                    add_room(tmpregion.x1, tmpregion.y1,
                             tmpregion.x2, tmpregion.y2,
                             tmpregion.rlit, tmpregion.rtype, true);
                    topologize(troom);                  /* set roomno */
                }
        }

        Fread((void *) &n, 1, sizeof(n), fd);
                                                /* Number of doors */
        while(n--) {
                struct mkroom *croom = &rooms[0];

                Fread((void *)&tmpdoor, 1, sizeof(tmpdoor), fd);

                x = tmpdoor.x;  y = tmpdoor.y;
                typ = tmpdoor.mask == -1 ? rnddoor() : tmpdoor.mask;

                get_location(&x, &y, DRY);
                if(levl[x][y].typ != SDOOR)
                        levl[x][y].typ = DOOR;
                else {
                        if(typ < D_CLOSED)
                            typ = D_CLOSED; /* force it to be closed */
                }
                levl[x][y].flags = typ;

                /* Now the complicated part, list it with each subroom */
                /* The dog move and mail daemon routines use this */
                while(croom->hx >= 0 && doorindex < DOORMAX) {
                    if(croom->hx >= x-1 && croom->lx <= x+1 &&
                       croom->hy >= y-1 && croom->ly <= y+1) {
                        /* Found it */
                        add_door(x, y, croom);
                    }
                    croom++;
                }
        }

        /* now that we have rooms _and_ associated doors, fill the rooms */
        for(n = 0; n < SIZE(mustfill); n++)
            if(mustfill[(int)n])
                fill_room(&rooms[(int)n], (mustfill[(int)n] == 2));

        /* if special boundary syms (CROSSWALL) in map, remove them now */
        if(has_bounds) {
            for(x = xstart; x < xstart+xsize; x++)
                for(y = ystart; y < ystart+ysize; y++)
                    if(levl[x][y].typ == CROSSWALL)
                        levl[x][y].typ = ROOM;
        }

        Fread((void *) &n, 1, sizeof(n), fd);
                                                /* Number of drawbridges */
        while(n--) {
                Fread((void *)&tmpdb, 1, sizeof(tmpdb), fd);

                x = tmpdb.x;  y = tmpdb.y;
                get_location(&x, &y, DRY|WET);

                if (!create_drawbridge(x, y, tmpdb.dir, tmpdb.db_open))
                    impossible("Cannot create drawbridge.");
        }

        Fread((void *) &n, 1, sizeof(n), fd);
                                                /* Number of mazewalks */
        while(n--) {
                Fread((void *)&tmpwalk, 1, sizeof(tmpwalk), fd);

                get_location(&tmpwalk.x, &tmpwalk.y, DRY|WET);

                walklist[nwalk++] = tmpwalk;
        }

        Fread((void *) &n, 1, sizeof(n), fd);
                                                /* Number of non_diggables */
        while(n--) {
                Fread((void *)&tmpdig, 1, sizeof(tmpdig), fd);

                get_location(&tmpdig.x1, &tmpdig.y1, DRY|WET);
                get_location(&tmpdig.x2, &tmpdig.y2, DRY|WET);

                set_wall_property(tmpdig.x1, tmpdig.y1,
                                  tmpdig.x2, tmpdig.y2, W_NONDIGGABLE);
        }

        Fread((void *) &n, 1, sizeof(n), fd);
                                                /* Number of non_passables */
        while(n--) {
                Fread((void *)&tmpdig, 1, sizeof(tmpdig), fd);

                get_location(&tmpdig.x1, &tmpdig.y1, DRY|WET);
                get_location(&tmpdig.x2, &tmpdig.y2, DRY|WET);

                set_wall_property(tmpdig.x1, tmpdig.y1,
                                  tmpdig.x2, tmpdig.y2, W_NONPASSWALL);
        }

        Fread((void *) &n, 1, sizeof(n), fd);
                                                /* Number of ladders */
        while(n--) {
                Fread((void *)&tmplad, 1, sizeof(tmplad), fd);

                x = tmplad.x;  y = tmplad.y;
                get_location(&x, &y, DRY);

                levl[x][y].typ = LADDER;
                if (tmplad.up == 1) {
                        xupladder = x;  yupladder = y;
                        levl[x][y].flags = LA_UP;
                } else {
                        xdnladder = x;  ydnladder = y;
                        levl[x][y].flags = LA_DOWN;
                }
        }

        prevstair.x = prevstair.y = 0;
        Fread((void *) &n, 1, sizeof(n), fd);
                                                /* Number of stairs */
        while(n--) {
                Fread((void *)&tmpstair, 1, sizeof(tmpstair), fd);

                xi = 0;
                do {
                    x = tmpstair.x;  y = tmpstair.y;
                    get_location(&x, &y, DRY);
                } while(prevstair.x && xi++ < 100 &&
                        distmin(x,y,prevstair.x,prevstair.y) <= 8);
                if ((badtrap = t_at(x,y)) != 0) deltrap(badtrap);
                mkstairs(x, y, (char)tmpstair.up, (struct mkroom *)0);
                prevstair.x = x;
                prevstair.y = y;
        }

        Fread((void *) &n, 1, sizeof(n), fd);
                                                /* Number of altars */
        while(n--) {
                Fread((void *)&tmpaltar, 1, sizeof(tmpaltar), fd);

                create_altar(&tmpaltar, (struct mkroom *)0);
        }

        Fread((void *) &n, 1, sizeof(n), fd);
                                                /* Number of fountains */
        while (n--) {
                Fread((void *)&tmpfountain, 1, sizeof(tmpfountain), fd);

                create_feature(tmpfountain.x, tmpfountain.y,
                               (struct mkroom *)0, FOUNTAIN);
        }

        Fread((void *) &n, 1, sizeof(n), fd);
                                                /* Number of traps */
        while(n--) {
                Fread((void *)&tmptrap, 1, sizeof(tmptrap), fd);

                create_trap(&tmptrap, (struct mkroom *)0);
        }

        Fread((void *) &n, 1, sizeof(n), fd);
                                                /* Number of monsters */
        while(n--) {
                load_one_monster(fd, &tmpmons);

                create_monster(&tmpmons, (struct mkroom *)0);
        }

        Fread((void *) &n, 1, sizeof(n), fd);
                                                /* Number of objects */
        while(n--) {
                load_one_object(fd, &tmpobj);

                create_object(&tmpobj, (struct mkroom *)0);
        }

        Fread((void *) &n, 1, sizeof(n), fd);
                                                /* Number of gold piles */
        while (n--) {
                Fread((void *)&tmpgold, 1, sizeof(tmpgold), fd);

                create_gold(&tmpgold, (struct mkroom *)0);
        }

        Fread((void *) &n, 1, sizeof(n), fd);
                                                /* Number of engravings */
        while(n--) {
                load_one_engraving(fd, &tmpengraving);

                create_engraving(&tmpengraving, (struct mkroom *)0);
        }

    }           /* numpart loop */

    nwalk_sav = nwalk;
    while(nwalk--) {
            x = (signed char) walklist[nwalk].x;
            y = (signed char) walklist[nwalk].y;
            dir = walklist[nwalk].dir;

            /* don't use move() - it doesn't use W_NORTH, etc. */
            switch (dir) {
                case W_NORTH: --y; break;
                case W_SOUTH: y++; break;
                case W_EAST:  x++; break;
                case W_WEST:  --x; break;
                default: panic("load_maze: bad MAZEWALK direction");
            }

            if(!IS_DOOR(levl[x][y].typ)) {
                levl[x][y].typ = ROOM;
                levl[x][y].flags = 0;
            }

            /*
             * We must be sure that the parity of the coordinates for
             * walkfrom() is odd.  But we must also take into account
             * what direction was chosen.
             */
            if(!(x % 2)) {
                if (dir == W_EAST)
                    x++;
                else
                    x--;

                /* no need for IS_DOOR check; out of map bounds */
                levl[x][y].typ = ROOM;
                levl[x][y].flags = 0;
            }

            if (!(y % 2)) {
                if (dir == W_SOUTH)
                    y++;
                else
                    y--;
            }

            walkfrom(x, y);
    }
    wallification(1, 0, COLNO-1, ROWNO-1);

    /*
     * If there's a significant portion of maze unused by the special level,
     * we don't want it empty.
     *
     * Makes the number of traps, monsters, etc. proportional
     * to the size of the maze.
     */
    mapcountmax = mapcount = (x_maze_max - 2) * (y_maze_max - 2);

    for(x = 2; x < x_maze_max; x++)
        for(y = 0; y < y_maze_max; y++)
            if(Map[x][y]) mapcount--;

    if (nwalk_sav && (mapcount > (int) (mapcountmax / 10))) {
            mapfact = (int) ((mapcount * 100L) / mapcountmax);
            for(x = rnd((int) (20 * mapfact) / 100); x; x--) {
                    maze1xy(&mm, DRY);
                    (void) mkobj_at(rn2(2) ? GEM_CLASS : RANDOM_CLASS,
                                                        mm.x, mm.y, true);
            }
            for(x = rnd((int) (12 * mapfact) / 100); x; x--) {
                    maze1xy(&mm, DRY);
                    (void) mksobj_at(BOULDER, mm.x, mm.y, true, false);
            }
            for (x = rn2(2); x; x--) {
                maze1xy(&mm, DRY);
                (void) makemon(&mons[PM_MINOTAUR], mm.x, mm.y, NO_MM_FLAGS);
            }
            for(x = rnd((int) (12 * mapfact) / 100); x; x--) {
                    maze1xy(&mm, WET|DRY);
                    (void) makemon((struct permonst *) 0, mm.x, mm.y, NO_MM_FLAGS);
            }
            for(x = rn2((int) (15 * mapfact) / 100); x; x--) {
                    maze1xy(&mm, DRY);
                    (void) mkgold(0L,mm.x,mm.y);
            }
            for(x = rn2((int) (15 * mapfact) / 100); x; x--) {
                    int trytrap;

                    maze1xy(&mm, DRY);
                    trytrap = rndtrap();
                    if (sobj_at(BOULDER, mm.x, mm.y))
                        while (trytrap == PIT || trytrap == SPIKED_PIT ||
                                trytrap == TRAPDOOR || trytrap == HOLE)
                            trytrap = rndtrap();
                    (void) maketrap(mm.x, mm.y, trytrap);
            }
    }
    return true;
}

/*
 * General loader
 */

bool 
load_special (const char *name)
{
        dlb *fd;
        bool result = false;
        char c;
        struct version_info vers_info;

        fd = dlb_fopen(name, "r");
        if (!fd) return false;

        Fread((void *) &vers_info, sizeof vers_info, 1, fd);
        if (!check_version(&vers_info, name, true))
            goto give_up;

        Fread((void *) &c, sizeof c, 1, fd); /* c Header */

        switch (c) {
                case SP_LEV_ROOMS:
                    result = load_rooms(fd);
                    break;
                case SP_LEV_MAZE:
                    result = load_maze(fd);
                    break;
                default:        /* ??? */
                    result = false;
        }
 give_up:
        (void)dlb_fclose(fd);
        return result;
}

/*sp_lev.c*/
