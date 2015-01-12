/* See LICENSE in the root of this project for change info */

#include "region.h"

#include <stdlib.h>
#include <string.h>

#include "display_util.h"
#include "cmd.h"
#include "decl.h"
#include "display.h"
#include "do_name.h"
#include "flag.h"
#include "global.h"
#include "hack.h"
#include "lev.h"
#include "light.h"
#include "mon.h"
#include "monattk.h"
#include "mondata.h"
#include "monst.h"
#include "objnam.h"
#include "pline.h"
#include "polyself.h"
#include "potion.h"
#include "restore.h"
#include "rm.h"
#include "rnd.h"
#include "save.h"
#include "util.h"
#include "vision.h"
#include "you.h"
#include "youprop.h"

/*
 * This should really go into the level structure, but
 * I'll start here for ease. It *WILL* move into the level
 * structure eventually.
 */

static NhRegion **regions;
static int n_regions = 0;
static int max_regions = 0;

#define NO_CALLBACK (-1)

bool inside_gas_cloud(void *,void *);
bool expire_gas_cloud(void *,void *);
bool inside_rect(NhRect *,int,int);
bool inside_region(NhRegion *,int,int);
NhRegion *create_region(NhRect *,int);
void add_rect_to_reg(NhRegion *,NhRect *);
void add_mon_to_reg(NhRegion *,struct monst *);
void remove_mon_from_reg(NhRegion *,struct monst *);
bool mon_in_region(NhRegion *,struct monst *);

void free_region(NhRegion *);
void add_region(NhRegion *);
void remove_region(NhRegion *);

static void reset_region_mids(NhRegion *);

static callback_proc callbacks[] = {
#define INSIDE_GAS_CLOUD 0
    inside_gas_cloud,
#define EXPIRE_GAS_CLOUD 1
    expire_gas_cloud
};

/* Should be inlined. */
bool 
inside_rect (NhRect *r, int x, int y)
{
    return (x >= r->lx && x <= r->hx && y >= r->ly && y <= r->hy);
}

/*
 * Check if a point is inside a region.
 */
bool 
inside_region (NhRegion *reg, int x, int y)
{
    int i;

    if (reg == NULL || !inside_rect(&(reg->bounding_box), x, y))
        return false;
    for (i = 0; i < reg->nrects; i++)
        if (inside_rect(&(reg->rects[i]), x, y))
            return true;
    return false;
}

/*
 * Create a region. It does not activate it.
 */
NhRegion *
create_region (NhRect *rects, int nrect)
{
    int i;
    NhRegion *reg;

    reg = (NhRegion *) malloc(sizeof (NhRegion));
    /* Determines bounding box */
    if (nrect > 0) {
        reg->bounding_box = rects[0];
    } else {
        reg->bounding_box.lx = 99;
        reg->bounding_box.ly = 99;
        reg->bounding_box.hx = 0;
        reg->bounding_box.hy = 0;
    }
    reg->nrects = nrect;
    reg->rects = nrect > 0 ? (NhRect *)malloc((sizeof (NhRect)) * nrect) : NULL;
    for (i = 0; i < nrect; i++) {
        if (rects[i].lx < reg->bounding_box.lx)
            reg->bounding_box.lx = rects[i].lx;
        if (rects[i].ly < reg->bounding_box.ly)
            reg->bounding_box.ly = rects[i].ly;
        if (rects[i].hx > reg->bounding_box.hx)
            reg->bounding_box.hx = rects[i].hx;
        if (rects[i].hy > reg->bounding_box.hy)
            reg->bounding_box.hy = rects[i].hy;
        reg->rects[i] = rects[i];
    }
    reg->ttl = -1;              /* Defaults */
    reg->attach_2_u = false;
    reg->attach_2_m = 0;
    /* reg->attach_2_o = NULL; */
    reg->enter_msg = NULL;
    reg->leave_msg = NULL;
    reg->expire_f = NO_CALLBACK;
    reg->enter_f = NO_CALLBACK;
    reg->can_enter_f = NO_CALLBACK;
    reg->leave_f = NO_CALLBACK;
    reg->can_leave_f = NO_CALLBACK;
    reg->inside_f = NO_CALLBACK;
    clear_hero_inside(reg);
    clear_heros_fault(reg);
    reg->n_monst = 0;
    reg->max_monst = 0;
    reg->monsters = NULL;
    reg->arg = NULL;
    return reg;
}

/*
 * Add rectangle to region.
 */
void
add_rect_to_reg (NhRegion *reg, NhRect *rect)
{
    NhRect *tmp_rect;

    tmp_rect = (NhRect *) malloc(sizeof (NhRect) * (reg->nrects + 1));
    if (reg->nrects > 0) {
        (void) memcpy((void *) tmp_rect, (void *) reg->rects,
                      (sizeof (NhRect) * reg->nrects));
        free((void *) reg->rects);
    }
    tmp_rect[reg->nrects] = *rect;
    reg->nrects++;
    reg->rects = tmp_rect;
    /* Update bounding box if needed */
    if (reg->bounding_box.lx > rect->lx)
        reg->bounding_box.lx = rect->lx;
    if (reg->bounding_box.ly > rect->ly)
        reg->bounding_box.ly = rect->ly;
    if (reg->bounding_box.hx < rect->hx)
        reg->bounding_box.hx = rect->hx;
    if (reg->bounding_box.hy < rect->hy)
        reg->bounding_box.hy = rect->hy;
}

/*
 * Add a monster to the region
 */
void
add_mon_to_reg (NhRegion *reg, struct monst *mon)
{
    int i;
    unsigned *tmp_m;

    if (reg->max_monst <= reg->n_monst) {
        tmp_m = (unsigned *)
                    malloc(sizeof (unsigned) * (reg->max_monst + MONST_INC));
        if (reg->max_monst > 0) {
            for (i = 0; i < reg->max_monst; i++)
                tmp_m[i] = reg->monsters[i];
            free((void *) reg->monsters);
        }
        reg->monsters = tmp_m;
        reg->max_monst += MONST_INC;
    }
    reg->monsters[reg->n_monst++] = mon->m_id;
}

/*
 * Remove a monster from the region list (it left or died...)
 */
void
remove_mon_from_reg (NhRegion *reg, struct monst *mon)
{
    int i;

    for (i = 0; i < reg->n_monst; i++)
        if (reg->monsters[i] == mon->m_id) {
            reg->n_monst--;
            reg->monsters[i] = reg->monsters[reg->n_monst];
            return;
        }
}

/*
 * Check if a monster is inside the region.
 * It's probably quicker to check with the region internal list
 * than to check for coordinates.
 */
bool 
mon_in_region (NhRegion *reg, struct monst *mon)
{
    int i;

    for (i = 0; i < reg->n_monst; i++)
        if (reg->monsters[i] == mon->m_id)
            return true;
    return false;
}


/*
 * Free mem from region.
 */
void
free_region (NhRegion *reg)
{
    if (reg) {
        if (reg->rects)
            free((void *) reg->rects);
        if (reg->monsters)
            free((void *) reg->monsters);
        free((void *) reg);
    }
}

/*
 * Add a region to the list.
 * This actually activates the region.
 */
void
add_region (NhRegion *reg)
{
    NhRegion **tmp_reg;
    int i, j;

    if (max_regions <= n_regions) {
        tmp_reg = regions;
        regions = (NhRegion **)malloc(sizeof (NhRegion *) * (max_regions + 10));
        if (max_regions > 0) {
            (void) memcpy((void *) regions, (void *) tmp_reg,
                          max_regions * sizeof (NhRegion *));
            free((void *) tmp_reg);
        }
        max_regions += 10;
    }
    regions[n_regions] = reg;
    n_regions++;
    /* Check for monsters inside the region */
    for (i = reg->bounding_box.lx; i <= reg->bounding_box.hx; i++)
        for (j = reg->bounding_box.ly; j <= reg->bounding_box.hy; j++) {
            /* Some regions can cross the level boundaries */
            if (!isok(i,j))
                continue;
            if (MON_AT(i, j) && inside_region(reg, i, j))
                add_mon_to_reg(reg, level.monsters[i][j]);
            if (reg->visible && cansee(i, j))
                newsym(i, j);
        }
    /* Check for player now... */
    if (inside_region(reg, u.ux, u.uy))
        set_hero_inside(reg);
    else
        clear_hero_inside(reg);
}

/*
 * Remove a region from the list & free it.
 */
void
remove_region (NhRegion *reg)
{
    int i, x, y;

    for (i = 0; i < n_regions; i++)
        if (regions[i] == reg)
            break;
    if (i == n_regions)
        return;

    /* Update screen if necessary */
    if (reg->visible)
        for (x = reg->bounding_box.lx; x <= reg->bounding_box.hx; x++)
            for (y = reg->bounding_box.ly; y <= reg->bounding_box.hy; y++)
                if (isok(x,y) && inside_region(reg, x, y) && cansee(x, y))
                    newsym(x, y);

    free_region(reg);
    regions[i] = regions[n_regions - 1];
    regions[n_regions - 1] = (NhRegion *) 0;
    n_regions--;
}

/*
 * Remove all regions and clear all related data (This must be down
 * when changing level, for instance).
 */
void
clear_regions (void)
{
    int i;

    for (i = 0; i < n_regions; i++)
        free_region(regions[i]);
    n_regions = 0;
    if (max_regions > 0)
        free((void *) regions);
    max_regions = 0;
    regions = NULL;
}

/*
 * This function is called every turn.
 * It makes the regions age, if necessary and calls the appropriate
 * callbacks when needed.
 */
void
run_regions (void)
{
    int i, j, k;
    int f_indx;

    /* End of life ? */
    /* Do it backward because the array will be modified */
    for (i = n_regions - 1; i >= 0; i--) {
        if (regions[i]->ttl == 0) {
            if ((f_indx = regions[i]->expire_f) == NO_CALLBACK ||
                (*callbacks[f_indx])(regions[i], (void *) 0))
                remove_region(regions[i]);
        }
    }

    /* Process remaining regions */
    for (i = 0; i < n_regions; i++) {
        /* Make the region age */
        if (regions[i]->ttl > 0)
            regions[i]->ttl--;
        /* Check if player is inside region */
        f_indx = regions[i]->inside_f;
        if (f_indx != NO_CALLBACK && hero_inside(regions[i]))
            (void) (*callbacks[f_indx])(regions[i], (void *) 0);
        /* Check if any monster is inside region */
        if (f_indx != NO_CALLBACK) {
            for (j = 0; j < regions[i]->n_monst; j++) {
                struct monst *mtmp = find_mid(regions[i]->monsters[j], FM_FMON);

                if (!mtmp || mtmp->mhp <= 0 ||
                                (*callbacks[f_indx])(regions[i], mtmp)) {
                    /* The monster died, remove it from list */
                    k = (regions[i]->n_monst -= 1);
                    regions[i]->monsters[j] = regions[i]->monsters[k];
                    regions[i]->monsters[k] = 0;
                    --j;    /* current slot has been reused; recheck it next */
                }
            }
        }
    }
}

/*
 * check whether player enters/leaves one or more regions.
 */
bool 
in_out_region (signed char x, signed char y)
{
    int i, f_indx;

    /* First check if we can do the move */
    for (i = 0; i < n_regions; i++) {
        if (inside_region(regions[i], x, y)
            && !hero_inside(regions[i]) && !regions[i]->attach_2_u) {
            if ((f_indx = regions[i]->can_enter_f) != NO_CALLBACK)
                if (!(*callbacks[f_indx])(regions[i], (void *) 0))
                    return false;
        } else
            if (hero_inside(regions[i])
                && !inside_region(regions[i], x, y)
                && !regions[i]->attach_2_u) {
            if ((f_indx = regions[i]->can_leave_f) != NO_CALLBACK)
                if (!(*callbacks[f_indx])(regions[i], (void *) 0))
                    return false;
        }
    }

    /* Callbacks for the regions we do leave */
    for (i = 0; i < n_regions; i++)
        if (hero_inside(regions[i]) &&
                !regions[i]->attach_2_u && !inside_region(regions[i], x, y)) {
            clear_hero_inside(regions[i]);
            if (regions[i]->leave_msg != NULL)
                plines(regions[i]->leave_msg);
            if ((f_indx = regions[i]->leave_f) != NO_CALLBACK)
                (void) (*callbacks[f_indx])(regions[i], (void *) 0);
        }

    /* Callbacks for the regions we do enter */
    for (i = 0; i < n_regions; i++)
        if (!hero_inside(regions[i]) &&
                !regions[i]->attach_2_u && inside_region(regions[i], x, y)) {
            set_hero_inside(regions[i]);
            if (regions[i]->enter_msg != NULL)
                plines(regions[i]->enter_msg);
            if ((f_indx = regions[i]->enter_f) != NO_CALLBACK)
                (void) (*callbacks[f_indx])(regions[i], (void *) 0);
        }
    return true;
}

/*
 * check wether a monster enters/leaves one or more region.
*/
bool 
m_in_out_region (struct monst *mon, signed char x, signed char y)
{
    int i, f_indx;

    /* First check if we can do the move */
    for (i = 0; i < n_regions; i++) {
        if (inside_region(regions[i], x, y) &&
                !mon_in_region(regions[i], mon) &&
                regions[i]->attach_2_m != mon->m_id) {
            if ((f_indx = regions[i]->can_enter_f) != NO_CALLBACK)
                if (!(*callbacks[f_indx])(regions[i], mon))
                    return false;
        } else if (mon_in_region(regions[i], mon) &&
                !inside_region(regions[i], x, y) &&
                regions[i]->attach_2_m != mon->m_id) {
            if ((f_indx = regions[i]->can_leave_f) != NO_CALLBACK)
                if (!(*callbacks[f_indx])(regions[i], mon))
                    return false;
        }
    }

    /* Callbacks for the regions we do leave */
    for (i = 0; i < n_regions; i++)
        if (mon_in_region(regions[i], mon) &&
                regions[i]->attach_2_m != mon->m_id &&
                !inside_region(regions[i], x, y)) {
            remove_mon_from_reg(regions[i], mon);
            if ((f_indx = regions[i]->leave_f) != NO_CALLBACK)
                (void) (*callbacks[f_indx])(regions[i], mon);
        }

    /* Callbacks for the regions we do enter */
    for (i = 0; i < n_regions; i++)
        if (!hero_inside(regions[i]) &&
                !regions[i]->attach_2_u && inside_region(regions[i], x, y)) {
            add_mon_to_reg(regions[i], mon);
            if ((f_indx = regions[i]->enter_f) != NO_CALLBACK)
                (void) (*callbacks[f_indx])(regions[i], mon);
        }
    return true;
}

/*
 * Checks player's regions after a teleport for instance.
 */
void
update_player_regions (void)
{
    int i;

    for (i = 0; i < n_regions; i++)
        if (!regions[i]->attach_2_u && inside_region(regions[i], u.ux, u.uy))
            set_hero_inside(regions[i]);
        else
            clear_hero_inside(regions[i]);
}

/*
 * Ditto for a specified monster.
 */
void
update_monster_region (struct monst *mon)
{
    int i;

    for (i = 0; i < n_regions; i++) {
        if (inside_region(regions[i], mon->mx, mon->my)) {
            if (!mon_in_region(regions[i], mon))
                add_mon_to_reg(regions[i], mon);
        } else {
            if (mon_in_region(regions[i], mon))
                remove_mon_from_reg(regions[i], mon);
        }
    }
}


/*
 * Check if a spot is under a visible region (eg: gas cloud).
 * Returns NULL if not, otherwise returns region.
 */
NhRegion *
visible_region_at (signed char x, signed char y)
{
    int i;

    for (i = 0; i < n_regions; i++)
        if (inside_region(regions[i], x, y) && regions[i]->visible &&
                regions[i]->ttl != 0)
            return regions[i];
    return (NhRegion *) 0;
}

void
show_region (NhRegion *reg, signed char x, signed char y)
{
    show_glyph(x, y, reg->glyph);
}

/**
 * save_regions :
 */
void
save_regions (int fd, int mode)
{
    int i, j;
    unsigned n;

    if (!perform_bwrite(mode)) goto skip_lots;

    bwrite(fd, (void *) &moves, sizeof (moves));        /* timestamp */
    bwrite(fd, (void *) &n_regions, sizeof (n_regions));
    for (i = 0; i < n_regions; i++) {
        bwrite(fd, (void *) &regions[i]->bounding_box, sizeof (NhRect));
        bwrite(fd, (void *) &regions[i]->nrects, sizeof (short));
        for (j = 0; j < regions[i]->nrects; j++)
            bwrite(fd, (void *) &regions[i]->rects[j], sizeof (NhRect));
        bwrite(fd, (void *) &regions[i]->attach_2_u, sizeof (bool));
        n = 0;
        bwrite(fd, (void *) &regions[i]->attach_2_m, sizeof (unsigned));
        n = regions[i]->enter_msg != NULL ? strlen(regions[i]->enter_msg) : 0;
        bwrite(fd, (void *) &n, sizeof n);
        if (n > 0)
            bwrite(fd, (void *) regions[i]->enter_msg, n);
        n = regions[i]->leave_msg != NULL ? strlen(regions[i]->leave_msg) : 0;
        bwrite(fd, (void *) &n, sizeof n);
        if (n > 0)
            bwrite(fd, (void *) regions[i]->leave_msg, n);
        bwrite(fd, (void *) &regions[i]->ttl, sizeof (short));
        bwrite(fd, (void *) &regions[i]->expire_f, sizeof (short));
        bwrite(fd, (void *) &regions[i]->can_enter_f, sizeof (short));
        bwrite(fd, (void *) &regions[i]->enter_f, sizeof (short));
        bwrite(fd, (void *) &regions[i]->can_leave_f, sizeof (short));
        bwrite(fd, (void *) &regions[i]->leave_f, sizeof (short));
        bwrite(fd, (void *) &regions[i]->inside_f, sizeof (short));
        bwrite(fd, (void *) &regions[i]->player_flags, sizeof (bool));
        bwrite(fd, (void *) &regions[i]->n_monst, sizeof (short));
        for (j = 0; j < regions[i]->n_monst; j++)
            bwrite(fd, (void *) &regions[i]->monsters[j],
             sizeof (unsigned));
        bwrite(fd, (void *) &regions[i]->visible, sizeof (bool));
        bwrite(fd, (void *) &regions[i]->glyph, sizeof (int));
        bwrite(fd, (void *) &regions[i]->arg, sizeof (void *));
    }

skip_lots:
    if (release_data(mode))
        clear_regions();
}

void 
rest_regions (
    int fd,
    bool ghostly /* If a bones file restore */
)
{
    int i, j;
    unsigned n;
    long tmstamp;
    char *msg_buf;

    clear_regions();            /* Just for security */
    mread(fd, (void *) &tmstamp, sizeof (tmstamp));
    if (ghostly) tmstamp = 0;
    else tmstamp = (moves - tmstamp);
    mread(fd, (void *) &n_regions, sizeof (n_regions));
    max_regions = n_regions;
    if (n_regions > 0)
        regions = (NhRegion **) malloc(sizeof (NhRegion *) * n_regions);
    for (i = 0; i < n_regions; i++) {
        regions[i] = (NhRegion *) malloc(sizeof (NhRegion));
        mread(fd, (void *) &regions[i]->bounding_box, sizeof (NhRect));
        mread(fd, (void *) &regions[i]->nrects, sizeof (short));

        if (regions[i]->nrects > 0)
            regions[i]->rects = (NhRect *)
                                  malloc(sizeof (NhRect) * regions[i]->nrects);
        for (j = 0; j < regions[i]->nrects; j++)
            mread(fd, (void *) &regions[i]->rects[j], sizeof (NhRect));
        mread(fd, (void *) &regions[i]->attach_2_u, sizeof (bool));
        mread(fd, (void *) &regions[i]->attach_2_m, sizeof (unsigned));

        mread(fd, (void *) &n, sizeof n);
        if (n > 0) {
            msg_buf = (char *) malloc(n + 1);
            mread(fd, (void *) msg_buf, n);
            msg_buf[n] = '\0';
            regions[i]->enter_msg = (const char *) msg_buf;
        } else
            regions[i]->enter_msg = NULL;

        mread(fd, (void *) &n, sizeof n);
        if (n > 0) {
            msg_buf = (char *) malloc(n + 1);
            mread(fd, (void *) msg_buf, n);
            msg_buf[n] = '\0';
            regions[i]->leave_msg = (const char *) msg_buf;
        } else
            regions[i]->leave_msg = NULL;

        mread(fd, (void *) &regions[i]->ttl, sizeof (short));
        /* check for expired region */
        if (regions[i]->ttl >= 0)
            regions[i]->ttl =
                (regions[i]->ttl > tmstamp) ? regions[i]->ttl - tmstamp : 0;
        mread(fd, (void *) &regions[i]->expire_f, sizeof (short));
        mread(fd, (void *) &regions[i]->can_enter_f, sizeof (short));
        mread(fd, (void *) &regions[i]->enter_f, sizeof (short));
        mread(fd, (void *) &regions[i]->can_leave_f, sizeof (short));
        mread(fd, (void *) &regions[i]->leave_f, sizeof (short));
        mread(fd, (void *) &regions[i]->inside_f, sizeof (short));
        mread(fd, (void *) &regions[i]->player_flags, sizeof (bool));
        if (ghostly) {  /* settings pertained to old player */
            clear_hero_inside(regions[i]);
            clear_heros_fault(regions[i]);
        }
        mread(fd, (void *) &regions[i]->n_monst, sizeof (short));
        if (regions[i]->n_monst > 0)
            regions[i]->monsters =
                (unsigned *) malloc(sizeof (unsigned) * regions[i]->n_monst);
        else
            regions[i]->monsters = NULL;
        regions[i]->max_monst = regions[i]->n_monst;
        for (j = 0; j < regions[i]->n_monst; j++)
            mread(fd, (void *) &regions[i]->monsters[j],
                  sizeof (unsigned));
        mread(fd, (void *) &regions[i]->visible, sizeof (bool));
        mread(fd, (void *) &regions[i]->glyph, sizeof (int));
        mread(fd, (void *) &regions[i]->arg, sizeof (void *));
    }
    /* remove expired regions, do not trigger the expire_f callback (yet!);
       also update monster lists if this data is coming from a bones file */
    for (i = n_regions - 1; i >= 0; i--)
        if (regions[i]->ttl == 0)
            remove_region(regions[i]);
        else if (ghostly && regions[i]->n_monst > 0)
            reset_region_mids(regions[i]);
}

/* update monster IDs for region being loaded from bones; `ghostly' implied */
static void
reset_region_mids (NhRegion *reg)
{
    int i = 0, n = reg->n_monst;
    unsigned *mid_list = reg->monsters;

    while (i < n)
        if (!lookup_id_mapping(mid_list[i], &mid_list[i])) {
            /* shrink list to remove missing monster; order doesn't matter */
            mid_list[i] = mid_list[--n];
        } else {
            /* move on to next monster */
            ++i;
        }
    reg->n_monst = n;
    return;
}


/*--------------------------------------------------------------*
 *                                                              *
 *                      Gas cloud related code                  *
 *                                                              *
 *--------------------------------------------------------------*/

/*
 * Here is an example of an expire function that may prolong
 * region life after some mods...
 */
bool 
expire_gas_cloud (void *p1, void *p2)
{
    NhRegion *reg;
    int damage;

    reg = (NhRegion *) p1;
    damage = (int) reg->arg;

    /* If it was a thick cloud, it dissipates a little first */
    if (damage >= 5) {
        damage /= 2;            /* It dissipates, let's do less damage */
        reg->arg = (void *) damage;
        reg->ttl = 2;           /* Here's the trick : reset ttl */
        return false;           /* THEN return false, means "still there" */
    }
    return true;                /* OK, it's gone, you can free it! */
}

bool 
inside_gas_cloud (void *p1, void *p2)
{
    NhRegion *reg;
    struct monst *mtmp;
    int dam;

    reg = (NhRegion *) p1;
    dam = (int) reg->arg;
    if (p2 == NULL) {           /* This means *YOU* Bozo! */
        if (nonliving(youmonst.data) || Breathless)
            return false;
        if (!Blind)
            make_blinded(1L, false);
        if (!Poison_resistance()) {
            pline("%s is burning your %s!", Something, makeplural(body_part(LUNG)));
            You("cough and spit blood!");
            losehp(rnd(dam) + 5, killed_by_const(KM_GAS_CLOUD));
            return false;
        } else {
            You("cough!");
            return false;
        }
    } else {                    /* A monster is inside the cloud */
        mtmp = (struct monst *) p2;

        /* Non living and non breathing monsters are not concerned */
        if (!nonliving(mtmp->data) && !breathless(mtmp->data)) {
            if (cansee(mtmp->mx, mtmp->my)) {
                char name[BUFSZ];
                Monnam(name, BUFSZ, mtmp);
                pline("%s coughs!", name);
            }
            setmangry(mtmp);
            if (haseyes(mtmp->data) && mtmp->mcansee) {
                mtmp->mblinded = 1;
                mtmp->mcansee = 0;
            }
            if (resists_poison(mtmp))
                return false;
            mtmp->mhp -= rnd(dam) + 5;
            if (mtmp->mhp <= 0) {
                if (heros_fault(reg))
                    killed(mtmp);
                else
                    monkilled(mtmp, "gas cloud", AD_DRST);
                if (mtmp->mhp <= 0) {   /* not lifesaved */
                    return true;
                }
            }
        }
    }
    return false;               /* Monster is still alive */
}

NhRegion *
create_gas_cloud (signed char x, signed char y, int radius, int damage)
{
    NhRegion *cloud;
    int i, nrect;
    NhRect tmprect;

    cloud = create_region((NhRect *) 0, 0);
    nrect = radius;
    tmprect.lx = x;
    tmprect.hx = x;
    tmprect.ly = y - (radius - 1);
    tmprect.hy = y + (radius - 1);
    for (i = 0; i < nrect; i++) {
        add_rect_to_reg(cloud, &tmprect);
        tmprect.lx--;
        tmprect.hx++;
        tmprect.ly++;
        tmprect.hy--;
    }
    cloud->ttl = rn1(3,4);
    if (!in_mklev && !flags.mon_moving)
        set_heros_fault(cloud);         /* assume player has created it */
    cloud->inside_f = INSIDE_GAS_CLOUD;
    cloud->expire_f = EXPIRE_GAS_CLOUD;
    cloud->arg = (void *) damage;
    cloud->visible = true;
    cloud->glyph = cmap_to_glyph(S_cloud);
    add_region(cloud);
    return cloud;
}

/*region.c*/
