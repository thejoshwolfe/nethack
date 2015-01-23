/* See LICENSE in the root of this project for change info */

#include "restore.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dungeon_util.h"
#include "artifact.h"
#include "attrib.h"
#include "coord.h"
#include "decl.h"
#include "display.h"
#include "dog.h"
#include "dungeon.h"
#include "end.h"
#include "engrave.h"
#include "files.h"
#include "flag.h"
#include "global.h"
#include "hack.h"
#include "invent.h"
#include "lev.h"
#include "light.h"
#include "makemon.h"
#include "mkmaze.h"
#include "mkobj.h"
#include "mkroom.h"
#include "mon.h"
#include "mondata.h"
#include "monst.h"
#include "o_init.h"
#include "obj.h"
#include "objclass.h"
#include "objnam.h"
#include "onames.h"
#include "options.h"
#include "permonst.h"
#include "pline.h"
#include "polyself.h"
#include "priest.h"
#include "prop.h"
#include "questpgr.h"
#include "region.h"
#include "rm.h"
#include "role.h"
#include "rumors.h"
#include "save.h"
#include "shk.h"
#include "spell.h"
#include "steed.h"
#include "timeout.h"
#include "trap.h"
#include "util.h"
#include "vision.h"
#include "wield.h"
#include "worm.h"
#include "worn.h"
#include "you.h"

/*
 * Save a mapping of IDs from ghost levels to the current level.  This
 * map is used by the timer routines when restoring ghost levels.
 */
#define N_PER_BUCKET 64
struct bucket {
    struct bucket *next;
    struct {
        unsigned gid; /* ghost ID */
        unsigned nid; /* new ID */
    } map[N_PER_BUCKET];
};

static int n_ids_mapped = 0;
static struct bucket * id_map = NULL;

#include "quest.h"

bool restoring = false;
static struct fruit * oldfruit;
static long omoves;

/* Recalculate level.objects[x][y], since this info was not saved. */
static void find_lev_obj(void) {
    struct obj *fobjtmp = (struct obj *)0;
    struct obj *otmp;
    int x, y;

    for (x = 0; x < COLNO; x++)
        for (y = 0; y < ROWNO; y++)
            level.objects[x][y] = (struct obj *)0;

    /*
     * Reverse the entire fobj chain, which is necessary so that we can
     * place the objects in the proper order.  Make all obj in chain
     * OBJ_FREE so place_object will work correctly.
     */
    while ((otmp = fobj) != 0) {
        fobj = otmp->nobj;
        otmp->nobj = fobjtmp;
        otmp->where = OBJ_FREE;
        fobjtmp = otmp;
    }
    /* fobj should now be empty */

    /* Set level.objects (as well as reversing the chain back again) */
    while ((otmp = fobjtmp) != 0) {
        fobjtmp = otmp->nobj;
        place_object(otmp, otmp->ox, otmp->oy);
    }
}

/* Things that were marked "in_use" when the game was saved (ex. via the
 * infamous "HUP" cheat) get used up here.
 */
void inven_inuse(bool quietly) {
    struct obj *otmp, *otmp2;

    for (otmp = invent; otmp; otmp = otmp2) {
        otmp2 = otmp->nobj;
        if (otmp->oclass == COIN_CLASS) {
            /* in_use gold is created by some menu operations */
            if (!otmp->in_use) {
                impossible("inven_inuse: !in_use gold in inventory");
            }
            extract_nobj(otmp, &invent);
            otmp->in_use = false;
            dealloc_obj(otmp);
        } else if (otmp->in_use) {
            if (!quietly)
                pline("Finishing off %s...", xname(otmp));
            useup(otmp);
        }
    }
}

static void restlevchn(int fd) {
    int cnt;
    s_level *tmplev, *x;

    sp_levchn = (s_level *)0;
    mread(fd, (void *)&cnt, sizeof(int));
    for (; cnt > 0; cnt--) {

        tmplev = (s_level *)malloc(sizeof(s_level));
        mread(fd, (void *)tmplev, sizeof(s_level));
        if (!sp_levchn)
            sp_levchn = tmplev;
        else {

            for (x = sp_levchn; x->next; x = x->next)
                ;
            x->next = tmplev;
        }
        tmplev->next = (s_level *)0;
    }
}

static void restdamage(int fd, bool ghostly) {
    int counter;
    struct damage *tmp_dam;

    mread(fd, (void *)&counter, sizeof(counter));
    if (!counter)
        return;
    tmp_dam = (struct damage *)malloc(sizeof(struct damage));
    while (--counter >= 0) {
        char damaged_shops[5], *shp = (char *)0;

        mread(fd, (void *)tmp_dam, sizeof(*tmp_dam));
        if (ghostly)
            tmp_dam->when += (monstermoves - omoves);
        strcpy(damaged_shops, in_rooms(tmp_dam->place.x, tmp_dam->place.y, SHOPBASE));
        if (u.uz.dlevel) {
            /* when restoring, there are two passes over the current
             * level.  the first time, u.uz isn't set, so neither is
             * shop_keeper().  just wait and process the damage on
             * the second pass.
             */
            for (shp = damaged_shops; *shp; shp++) {
                struct monst *shkp = shop_keeper(*shp);

                if (shkp && inhishop(shkp) && repair_damage(shkp, tmp_dam, true))
                    break;
            }
        }
        if (!shp || !*shp) {
            tmp_dam->next = level.damagelist;
            level.damagelist = tmp_dam;
            tmp_dam = (struct damage *)malloc(sizeof(*tmp_dam));
        }
    }
    free((void *)tmp_dam);
}

/* Add a mapping to the ID map. */
static void add_id_mapping(unsigned gid, unsigned nid) {
    int idx;

    idx = n_ids_mapped % N_PER_BUCKET;
    /* idx is zero on first time through, as well as when a new bucket is */
    /* needed */
    if (idx == 0) {
        struct bucket *gnu = (struct bucket *)malloc(sizeof(struct bucket));
        gnu->next = id_map;
        id_map = gnu;
    }

    id_map->map[idx].gid = gid;
    id_map->map[idx].nid = nid;
    n_ids_mapped++;
}

static void ghostfruit(struct obj *otmp) {
    struct fruit *oldf;

    for (oldf = oldfruit; oldf; oldf = oldf->nextf)
        if (oldf->fid == otmp->spe)
            break;

    if (!oldf)
        impossible("no old fruit?");
    else
        otmp->spe = fruitadd(oldf->fname);
}

static struct obj * restobjchn(int fd, bool ghostly, bool frozen) {
    struct obj *otmp, *otmp2 = 0;
    struct obj *first = (struct obj *)0;
    int xl;

    while (1) {
        mread(fd, (void *)&xl, sizeof(xl));
        if (xl == -1)
            break;
        otmp = newobj(xl);
        if (!first)
            first = otmp;
        else
            otmp2->nobj = otmp;
        mread(fd, (void *)otmp, (unsigned)xl + sizeof(struct obj));
        if (ghostly) {
            unsigned nid = flags.ident++;
            add_id_mapping(otmp->o_id, nid);
            otmp->o_id = nid;
        }
        if (ghostly && otmp->otyp == SLIME_MOLD)
            ghostfruit(otmp);
        /* Ghost levels get object age shifted from old player's clock
         * to new player's clock.  Assumption: new player arrived
         * immediately after old player died.
         */
        if (ghostly && !frozen && !age_is_relative(otmp))
            otmp->age = monstermoves - omoves + otmp->age;

        /* get contents of a container or statue */
        if (Has_contents(otmp)) {
            struct obj *otmp3;
            otmp->cobj = restobjchn(fd, ghostly, otmp->otyp == ICE_BOX);
            /* restore container back pointers */
            for (otmp3 = otmp->cobj; otmp3; otmp3 = otmp3->nobj)
                otmp3->ocontainer = otmp;
        }
        if (otmp->bypass)
            otmp->bypass = 0;

        otmp2 = otmp;
    }
    if (first && otmp2->nobj) {
        impossible("Restobjchn: error reading objchn.");
        otmp2->nobj = 0;
    }

    return (first);
}

static struct monst * restmonchn(int fd, bool ghostly) {
    struct monst *mtmp, *mtmp2 = 0;
    struct monst *first = (struct monst *)0;
    int xl;
    struct permonst *monbegin;
    bool moved;

    /* get the original base address */
    mread(fd, (void *)&monbegin, sizeof(monbegin));
    moved = (monbegin != mons);

    while (1) {
        mread(fd, (void *)&xl, sizeof(xl));
        if (xl == -1)
            break;
        mtmp = newmonst(xl);
        if (!first)
            first = mtmp;
        else
            mtmp2->nmon = mtmp;
        mread(fd, (void *)mtmp, (unsigned)xl + sizeof(struct monst));
        if (ghostly) {
            unsigned nid = flags.ident++;
            add_id_mapping(mtmp->m_id, nid);
            mtmp->m_id = nid;
        }
        if (moved && mtmp->data) {
            int offset = mtmp->data - monbegin; /*(ptrdiff_t)*/
            mtmp->data = mons + offset; /* new permonst location */
        }
        if (ghostly) {
            int mndx = monsndx(mtmp->data);
            if (propagate(mndx, true, ghostly) == 0) {
                /* cookie to trigger purge in getbones() */
                mtmp->mhpmax = DEFUNCT_MONSTER;
            }
        }
        if (mtmp->minvent) {
            struct obj *obj;
            mtmp->minvent = restobjchn(fd, ghostly, false);
            /* restore monster back pointer */
            for (obj = mtmp->minvent; obj; obj = obj->nobj)
                obj->ocarry = mtmp;
        }
        if (mtmp->mw) {
            struct obj *obj;

            for (obj = mtmp->minvent; obj; obj = obj->nobj)
                if (obj->owornmask & W_WEP)
                    break;
            if (obj)
                mtmp->mw = obj;
            else {
                MON_NOWEP(mtmp);
                impossible("bad monster weapon restore");
            }
        }

        if (mtmp->isshk)
            restshk(mtmp, ghostly);
        if (mtmp->ispriest)
            restpriest(mtmp, ghostly);

        mtmp2 = mtmp;
    }
    if (first && mtmp2->nmon) {
        impossible("Restmonchn: error reading monchn.");
        mtmp2->nmon = 0;
    }
    return (first);
}

static struct fruit * loadfruitchn(int fd) {
    struct fruit *flist, *fnext;

    flist = 0;
    while (fnext = newfruit(), mread(fd, (void *)fnext, sizeof *fnext), fnext->fid != 0) {
        fnext->nextf = flist;
        flist = fnext;
    }
    dealloc_fruit(fnext);
    return flist;
}

static void freefruitchn(struct fruit *flist) {
    struct fruit *fnext;

    while (flist) {
        fnext = flist->nextf;
        dealloc_fruit(flist);
        flist = fnext;
    }
}

static bool restgamestate(int fd, unsigned int *stuckid, unsigned int *steedid) {
    struct obj *otmp;
    int uid;

    mread(fd, (void *)&uid, sizeof uid);
    if (uid != getuid()) { /* strange ... */
        /* for flags.debug mode, issue a reminder; for others, treat it
         as an attempt to cheat and refuse to restore this file */
        pline("Saved game was not yours.");
        if (!flags.debug)
            return false;
    }

    mread(fd, (void *)&flags, sizeof(struct flag));
    flags.bypasses = 0; /* never use the saved value of bypasses */

    role_init(); /* Reset the initial role, race, gender, and alignment */
    mread(fd, (void *)&u, sizeof(struct you));
    set_uasmon();
    if (u.uhp <= 0 && (!Upolyd || u.mh <= 0)) {
        u.ux = u.uy = 0; /* affects pline() [hence You()] */
        You("were not healthy enough to survive restoration.");
        /* wiz1_level.dlevel is used by mklev.c to see if lots of stuff is
         * uninitialized, so we only have to set it and not the other stuff.
         */
        wiz1_level.dlevel = 0;
        u.uz.dnum = 0;
        u.uz.dlevel = 1;
        return (false);
    }

    /* this stuff comes after potential aborted restore attempts */
    restore_timers(fd, RANGE_GLOBAL, false, 0L);
    restore_light_sources(fd);
    invent = restobjchn(fd, false, false);
    migrating_objs = restobjchn(fd, false, false);
    migrating_mons = restmonchn(fd, false);
    mread(fd, (void *)mvitals, sizeof(mvitals));

    /* this comes after inventory has been loaded */
    for (otmp = invent; otmp; otmp = otmp->nobj)
        if (otmp->owornmask)
            setworn(otmp, otmp->owornmask);
    /* reset weapon so that player will get a reminder about "bashing"
     during next fight when bare-handed or wielding an unconventional
     item; for pick-axe, we aren't able to distinguish between having
     applied or wielded it, so be conservative and assume the former */
    otmp = uwep; /* `uwep' usually init'd by setworn() in loop above */
    uwep = 0; /* clear it and have setuwep() reinit */
    setuwep(otmp); /* (don't need any null check here) */
    if (!uwep || uwep->otyp == PICK_AXE || uwep->otyp == GRAPPLING_HOOK)
        unweapon = true;

    restore_dungeon(fd);
    restlevchn(fd);
    mread(fd, (void *)&moves, sizeof moves);
    mread(fd, (void *)&monstermoves, sizeof monstermoves);
    mread(fd, (void *)&quest_status, sizeof(struct q_score));
    mread(fd, (void *)spl_book, sizeof(struct spell) * (MAXSPELL + 1));
    restore_artifacts(fd);
    restore_oracles(fd);
    if (u.ustuck)
        mread(fd, (void *)stuckid, sizeof(*stuckid));
    if (u.usteed)
        mread(fd, (void *)steedid, sizeof(*steedid));
    mread(fd, (void *)pl_character, sizeof pl_character);

    mread(fd, (void *)pl_fruit, sizeof pl_fruit);
    mread(fd, (void *)&current_fruit, sizeof current_fruit);
    freefruitchn(ffruit); /* clean up fruit(s) made by initoptions() */
    ffruit = loadfruitchn(fd);

    restnames(fd);
    restore_waterlevel(fd);
    /* must come after all mons & objs are restored */
    relink_timers(false);
    relink_light_sources(false);
    return (true);
}

/* update game state pointers to those valid for the current level (so we
 * don't dereference a wild u.ustuck when saving the game state, for instance)
 */
static void restlevelstate(unsigned int stuckid, unsigned int steedid) {
    struct monst *mtmp;

    if (stuckid) {
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
            if (mtmp->m_id == stuckid)
                break;
        if (!mtmp)
            panic("Cannot find the monster ustuck.");
        u.ustuck = mtmp;
    }
    if (steedid) {
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
            if (mtmp->m_id == steedid)
                break;
        if (!mtmp)
            panic("Cannot find the monster usteed.");
        u.usteed = mtmp;
        remove_monster(mtmp->mx, mtmp->my);
    }
}

static int restlevelfile(int fd, signed char ltmp) {
    int nfd;
    char whynot[BUFSZ];

    nfd = create_levelfile(ltmp, whynot);
    if (nfd < 0) {
        /* BUG: should suppress any attempt to write a panic
         save file if file creation is now failing... */
        panic("restlevelfile: %s", whynot);
    }
    bufon(nfd);
    savelev(nfd, ltmp, WRITE_SAVE | FREE_SAVE);
    bclose(nfd);
    return (2);
}

int dorecover(int fd) {
    unsigned int stuckid = 0, steedid = 0;
    signed char ltmp;
    int rtmp;
    struct obj * otmp;

    restoring = true;
    getlev(fd, 0, (signed char)0, false);
    if (!restgamestate(fd, &stuckid, &steedid)) {
        display_nhwindow(WIN_MESSAGE, true);
        savelev(-1, 0, FREE_SAVE); /* discard current level */
        close(fd);
        delete_savefile();
        restoring = false;
        return (0);
    }
    restlevelstate(stuckid, steedid);
    savestateinlock();
    rtmp = restlevelfile(fd, ledger_no(&u.uz));
    if (rtmp < 2)
        return (rtmp); /* dorecover called recursively */

    /* these pointers won't be valid while we're processing the
     * other levels, but they'll be reset again by restlevelstate()
     * afterwards, and in the meantime at least u.usteed may mislead
     * place_monster() on other levels
     */
    u.ustuck = (struct monst *)0;
    u.usteed = (struct monst *)0;

    while (1) {
        if (read(fd, (void *)&ltmp, sizeof ltmp) != sizeof ltmp)
            break;
        getlev(fd, 0, ltmp, false);
        rtmp = restlevelfile(fd, ltmp);
        if (rtmp < 2)
            return (rtmp); /* dorecover called recursively */
    }

    lseek(fd, (off_t)0, 0);
    uptodate(fd, (char *)0); /* skip version info */
    getlev(fd, 0, (signed char)0, false);
    close(fd);

    if (!flags.debug)
        delete_savefile();
    restlevelstate(stuckid, steedid);
    /* take care of iron ball & chain */
    for (otmp = fobj; otmp; otmp = otmp->nobj)
        if (otmp->owornmask)
            setworn(otmp, otmp->owornmask);

    /* in_use processing must be after:
     *    + The inventory has been read so that freeinv() works.
     *    + The current level has been restored so billing information
     *      is available.
     */
    inven_inuse(false);

    load_qtlist(); /* re-load the quest text info */
    reset_attribute_clock();
    /* Set up the vision internals, after levl[] data is loaded */
    /* but before docrt().                                      */
    vision_reset();
    vision_full_recalc = 1; /* recompute vision (not saved) */

    run_timers(); /* expire all timers that have gone off while away */
    docrt();
    restoring = false;
    clear_nhwindow(WIN_MESSAGE);
    program_state.something_worth_saving++; /* useful data now exists */

    /* Success! */
    return 1;
}

void trickery(char *reason) {
    pline("Strange, this map is not as I remember it.");
    pline("Somebody is trying some trickery here...");
    pline("This game is void.");
    fprintf(stderr, "TODO: killer = %s\n", reason);
    done(TRICKED);
}

static void reset_oattached_mids(bool ghostly) {
    struct obj *otmp;
    unsigned oldid, nid;
    for (otmp = fobj; otmp; otmp = otmp->nobj) {
        if (ghostly && otmp->oattached == OATTACHED_MONST && otmp->oxlth) {
            struct monst *mtmp = (struct monst *)otmp->oextra;

            mtmp->m_id = 0;
            mtmp->mpeaceful = mtmp->mtame = 0; /* pet's owner died! */
        }
        if (ghostly && otmp->oattached == OATTACHED_M_ID) {
            memcpy((void *)&oldid, (void *)otmp->oextra, sizeof(oldid));
            if (lookup_id_mapping(oldid, &nid))
                memcpy((void *)otmp->oextra, (void *)&nid, sizeof(nid));
            else
                otmp->oattached = OATTACHED_NOTHING;
        }
    }
}

/* Clear all structures for object and monster ID mapping. */
static void clear_id_mapping(void) {
    struct bucket *curr;

    while ((curr = id_map) != 0) {
        id_map = curr->next;
        free((void *)curr);
    }
    n_ids_mapped = 0;
}

void getlev(int fd, int pid, signed char lev, bool ghostly) {
    struct trap * trap;
    struct monst * mtmp;
    branch * br;
    int hpid;
    signed char dlvl;
    int x, y;

    if (ghostly)
        clear_id_mapping();

    /* Load the old fruit info.  We have to do it first, so the
     * information is available when restoring the objects.
     */
    if (ghostly)
        oldfruit = loadfruitchn(fd);

    /* First some sanity checks */
    mread(fd, (void *)&hpid, sizeof(hpid));
    /* CHECK:  This may prevent restoration */
    mread(fd, (void *)&dlvl, sizeof(dlvl));
    if ((pid && pid != hpid) || (lev && dlvl != lev)) {
        char trickbuf[BUFSZ];

        if (pid && pid != hpid)
            sprintf(trickbuf, "PID (%d) doesn't match saved PID (%d)!", hpid, pid);
        else
            sprintf(trickbuf, "This is level %d, not %d!", dlvl, lev);
        if (flags.debug)
            plines(trickbuf);
        trickery(trickbuf);
    }

    mread(fd, (void *) levl, sizeof(levl));

    mread(fd, (void *)&omoves, sizeof(omoves));
    mread(fd, (void *)&upstair, sizeof(stairway));
    mread(fd, (void *)&dnstair, sizeof(stairway));
    mread(fd, (void *)&upladder, sizeof(stairway));
    mread(fd, (void *)&dnladder, sizeof(stairway));
    mread(fd, (void *)&sstairs, sizeof(stairway));
    mread(fd, (void *)&updest, sizeof(dest_area));
    mread(fd, (void *)&dndest, sizeof(dest_area));
    mread(fd, (void *)&level.flags, sizeof(level.flags));
    mread(fd, (void *)doors, sizeof(doors));
    rest_rooms(fd); /* No joke :-) */
    if (nroom)
        doorindex = rooms[nroom - 1].fdoor + rooms[nroom - 1].doorct;
    else
        doorindex = 0;

    restore_timers(fd, RANGE_LEVEL, ghostly, monstermoves - omoves);
    restore_light_sources(fd);
    fmon = restmonchn(fd, ghostly);

    /* regenerate animals while on another level */
    if (u.uz.dlevel) {
        struct monst *mtmp2;

        for (mtmp = fmon; mtmp; mtmp = mtmp2) {
            mtmp2 = mtmp->nmon;
            if (ghostly) {
                /* reset peaceful/malign relative to new character */
                if (!mtmp->isshk)
                    /* shopkeepers will reset based on name */
                    mtmp->mpeaceful = peace_minded(mtmp->data);
                set_malign(mtmp);
            } else if (monstermoves > omoves)
                mon_catchup_elapsed_time(mtmp, monstermoves - omoves);

            /* update shape-changers in case protection against
             them is different now than when the level was saved */
            restore_cham(mtmp);
        }
    }

    rest_worm(fd); /* restore worm information */
    ftrap = 0;
    while (trap = newtrap(), mread(fd, (void *)trap, sizeof(struct trap)), trap->tx != 0) {
        trap->ntrap = ftrap;
        ftrap = trap;
    }
    dealloc_trap(trap);
    fobj = restobjchn(fd, ghostly, false);
    find_lev_obj();
    /* restobjchn()'s `frozen' argument probably ought to be a callback
     routine so that we can check for objects being buried under ice */
    level.buriedobjlist = restobjchn(fd, ghostly, false);
    billobjs = restobjchn(fd, ghostly, false);
    rest_engravings(fd);

    /* reset level.monsters for new level */
    for (x = 0; x < COLNO; x++)
        for (y = 0; y < ROWNO; y++)
            level.monsters[x][y] = (struct monst *)0;
    for (mtmp = level.monlist; mtmp; mtmp = mtmp->nmon) {
        if (mtmp->isshk)
            set_residency(mtmp, false);
        place_monster(mtmp, mtmp->mx, mtmp->my);
        if (mtmp->wormno)
            place_wsegs(mtmp);
    }
    restdamage(fd, ghostly);

    rest_regions(fd, ghostly);
    if (ghostly) {
        /* Now get rid of all the temp fruits... */
        freefruitchn(oldfruit), oldfruit = 0;

        if (lev > ledger_no(&medusa_level) && lev < ledger_no(&stronghold_level) && xdnstair == 0) {
            coord cc;

            mazexy(&cc);
            xdnstair = cc.x;
            ydnstair = cc.y;
            levl[cc.x][cc.y].typ = STAIRS;
        }

        br = Is_branchlev(&u.uz);
        if (br && u.uz.dlevel == 1) {
            d_level ltmp;

            if (on_level(&u.uz, &br->end1))
                assign_level(&ltmp, &br->end2);
            else
                assign_level(&ltmp, &br->end1);

            switch (br->type) {
                case BR_STAIR:
                case BR_NO_END1:
                case BR_NO_END2: /* OK to assign to sstairs if it's not used */
                    assign_level(&sstairs.tolev, &ltmp);
                    break;
                case BR_PORTAL: /* max of 1 portal per level */
                {
                    struct trap *ttmp;
                    for (ttmp = ftrap; ttmp; ttmp = ttmp->ntrap)
                        if (ttmp->ttyp == MAGIC_PORTAL)
                            break;
                    if (!ttmp)
                        panic("getlev: need portal but none found");
                    assign_level(&ttmp->dst, &ltmp);
                }
                    break;
            }
        } else if (!br) {
            /* Remove any dangling portals. */
            struct trap *ttmp;
            for (ttmp = ftrap; ttmp; ttmp = ttmp->ntrap)
                if (ttmp->ttyp == MAGIC_PORTAL) {
                    deltrap(ttmp);
                    break; /* max of 1 portal/level */
                }
        }
    }

    /* must come after all mons & objs are restored */
    relink_timers(ghostly);
    relink_light_sources(ghostly);
    reset_oattached_mids(ghostly);

    if (ghostly)
        clear_id_mapping();
}

/*
 * Global routine to look up a mapping.  If found, return true and fill
 * in the new ID value.  Otherwise, return false and return -1 in the new
 * ID.
 */
bool lookup_id_mapping(unsigned gid, unsigned *nidp) {
    int i;
    struct bucket *curr;

    if (n_ids_mapped)
        for (curr = id_map; curr; curr = curr->next) {
            /* first bucket might not be totally full */
            if (curr == id_map) {
                i = n_ids_mapped % N_PER_BUCKET;
                if (i == 0)
                    i = N_PER_BUCKET;
            } else
                i = N_PER_BUCKET;

            while (--i >= 0)
                if (gid == curr->map[i].gid) {
                    *nidp = curr->map[i].nid;
                    return true;
                }
        }

    return false;
}

void mread(int fd, void *buf, unsigned int len) {
    int rlen;

    rlen = read(fd, buf, (unsigned)len);
    if ((unsigned)rlen != len) {
        pline("Read %d instead of %u bytes.", rlen, len);
        if (restoring) {
            close(fd);
            delete_savefile();
            fprintf(stderr, "Error restoring old game.\n");
        }
        panic("Error reading level file.");
    }
}
