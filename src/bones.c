/* See LICENSE in the root of this project for change info */

#include "bones.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "apply.h"
#include "artifact.h"
#include "decl.h"
#include "display.h"
#include "do.h"
#include "do_name.h"
#include "dog.h"
#include "dungeon.h"
#include "files.h"
#include "flag.h"
#include "global.h"
#include "hack.h"
#include "invent.h"
#include "lev.h"
#include "light.h"
#include "makemon.h"
#include "mkobj.h"
#include "mon.h"
#include "monflag.h"
#include "monst.h"
#include "obj.h"
#include "objclass.h"
#include "objnam.h"
#include "onames.h"
#include "permonst.h"
#include "pline.h"
#include "pm.h"
#include "pm_props.h"
#include "questpgr.h"
#include "restore.h"
#include "rm.h"
#include "rnd.h"
#include "save.h"
#include "steed.h"
#include "timeout.h"
#include "trap.h"
#include "worn.h"
#include "you.h"

extern char bones[];    /* from files.c */

static bool no_bones_level(d_level *);
static void goodfruit(int);
static void resetobjs(struct obj *,bool);
static void drop_upon_death(struct monst *, struct obj *);

static bool no_bones_level(d_level *lev) {
    extern d_level save_dlevel; /* in do.c */
    s_level *sptr;

    if (ledger_no(&save_dlevel))
        assign_level(lev, &save_dlevel);

    if ((sptr = Is_special(lev)) != 0 && !sptr->boneid)
        return true;
    if (!dungeons[lev->dnum].boneid)
        return true;
    /* no bones on the last or multiway branch levels */
    /* in any dungeon (level 1 isn't multiway).       */
    if (Is_botlevel(lev) || (Is_branchlev(lev) && lev->dlevel > 1))
        return true;
    /* no bones in the invocation level               */
    if (In_hell(lev) && lev->dlevel == dunlevs_in_dungeon(lev) - 1)
        return true;
    return false;
}

/* Call this function for each fruit object saved in the bones level: it marks
 * that particular type of fruit as existing (the marker is that that type's
 * ID is positive instead of negative).  This way, when we later save the
 * chain of fruit types, we know to only save the types that exist.
 */
static void goodfruit (int id) {
        struct fruit *f;

        for(f=ffruit; f; f=f->nextf) {
                if(f->fid == -id) {
                        f->fid = id;
                        return;
                }
        }
}

static void resetobjs(struct obj *ochain, bool restore) {
        struct obj *otmp;

        for (otmp = ochain; otmp; otmp = otmp->nobj) {
                if (otmp->cobj)
                    resetobjs(otmp->cobj,restore);

                if (((otmp->otyp != CORPSE || otmp->corpsenm < SPECIAL_PM)
                        && otmp->otyp != STATUE)
                        && (!otmp->oartifact ||
                           (restore && (exist_artifact(otmp->otyp, ONAME(otmp))
                                        || is_quest_artifact(otmp))))) {
                        otmp->oartifact = 0;
                        otmp->onamelth = 0;
                        *ONAME(otmp) = '\0';
                } else if (otmp->oartifact && restore)
                        artifact_exists(otmp,ONAME(otmp),true);
                if (!restore) {
                        /* do not zero out o_ids for ghost levels anymore */

                        if(objects[otmp->otyp].oc_uses_known) otmp->known = 0;
                        otmp->dknown = otmp->bknown = 0;
                        otmp->rknown = 0;
                        otmp->invlet = 0;
                        otmp->no_charge = 0;

                        if (otmp->otyp == SLIME_MOLD) goodfruit(otmp->spe);
                        else if (otmp->otyp == SCR_MAIL) otmp->spe = 1;
                        else if (otmp->otyp == EGG) otmp->spe = 0;
                        else if (otmp->otyp == TIN) {
                            /* make tins of unique monster's meat be empty */
                            if (otmp->corpsenm >= LOW_PM &&
                                    (mons[otmp->corpsenm].geno & G_UNIQ))
                                otmp->corpsenm = NON_PM;
                        } else if (otmp->otyp == AMULET_OF_YENDOR) {
                            /* no longer the real Amulet */
                            otmp->otyp = FAKE_AMULET_OF_YENDOR;
                            curse(otmp);
                        } else if (otmp->otyp == CANDELABRUM_OF_INVOCATION) {
                            if (otmp->lamplit)
                                end_burn(otmp, true);
                            otmp->otyp = WAX_CANDLE;
                            otmp->age = 50L;  /* assume used */
                            if (otmp->spe > 0)
                                otmp->quan = (long)otmp->spe;
                            otmp->spe = 0;
                            otmp->owt = weight(otmp);
                            curse(otmp);
                        } else if (otmp->otyp == BELL_OF_OPENING) {
                            otmp->otyp = BELL;
                            curse(otmp);
                        } else if (otmp->otyp == SPE_BOOK_OF_THE_DEAD) {
                            otmp->otyp = SPE_BLANK_PAPER;
                            curse(otmp);
                        }
                }
        }
}

static void drop_upon_death (struct monst *mtmp, struct obj *cont) {
        struct obj *otmp;

        uswapwep = 0; /* ensure curse() won't cause swapwep to drop twice */
        while ((otmp = invent) != 0) {
                obj_extract_self(otmp);
                obj_no_longer_held(otmp);

                otmp->owornmask = 0;
                /* lamps don't go out when dropped */
                if ((cont || artifact_light(otmp)) && obj_is_burning(otmp))
                    end_burn(otmp, true);       /* smother in statue */

                if(otmp->otyp == SLIME_MOLD) goodfruit(otmp->spe);

                if(rn2(5)) curse(otmp);
                if (mtmp)
                        (void) add_to_minv(mtmp, otmp);
                else if (cont)
                        (void) add_to_container(cont, otmp);
                else
                        place_object(otmp, u.ux, u.uy);
        }
        if(u.ugold) {
                long ugold = u.ugold;
                if (mtmp) mtmp->mgold = ugold;
                else if (cont) (void) add_to_container(cont, mkgoldobj(ugold));
                else (void)mkgold(ugold, u.ux, u.uy);
                u.ugold = ugold;        /* undo mkgoldobj()'s removal */
        }
        if (cont) cont->owt = weight(cont);
}

/* check whether bones are feasible */
bool can_make_bones(void) {
        struct trap *ttmp;

        if (ledger_no(&u.uz) <= 0 || ledger_no(&u.uz) > maxledgerno())
            return false;
        if (no_bones_level(&u.uz))
            return false;               /* no bones for specific levels */
        if (u.uswallow) {
            return false;               /* no bones when swallowed */
        }
        if (!Is_branchlev(&u.uz)) {
            /* no bones on non-branches with portals */
            for(ttmp = ftrap; ttmp; ttmp = ttmp->ntrap)
                if (ttmp->ttyp == MAGIC_PORTAL) return false;
        }

        if(depth(&u.uz) <= 0 ||         /* bulletproofing for endgame */
           (!rn2(1 + (depth(&u.uz)>>2)) /* fewer ghosts on low levels */
                && !flags.debug
                )) return false;
        /* don't let multiple restarts generate multiple copies of objects
         * in bones files */
        if (flags.explore) return false;
        return true;
}

/* save bones and possessions of a deceased adventurer */
void savebones (struct obj *corpse) {
        int fd, x, y;
        struct trap *ttmp;
        struct monst *mtmp;
        struct permonst *mptr;
        struct fruit *f;
        char c, *bonesid;
        char whynot[BUFSZ];

        /* caller has already checked `can_make_bones()' */

        clear_bypasses();
        fd = open_bonesfile(&u.uz, &bonesid);
        if (fd >= 0) {
                (void) close(fd);
                if (flags.debug) {
                    if (yn("Bones file already exists.  Replace it?") == 'y') {
                        if (delete_bonesfile(&u.uz)) goto make_bones;
                        else pline("Cannot unlink old bones.");
                    }
                }
                return;
        }

 make_bones:
        unleash_all();
        /* in case these characters are not in their home bases */
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp)) continue;
            mptr = mtmp->data;
            if (mtmp->iswiz || mptr == &mons[PM_MEDUSA] ||
                    mptr->msound == MS_NEMESIS || mptr->msound == MS_LEADER ||
                    mptr == &mons[PM_VLAD_THE_IMPALER])
                mongone(mtmp);
        }
        if (u.usteed) dismount_steed(DISMOUNT_BONES);
        dmonsfree();            /* discard dead or gone monsters */

        /* mark all fruits as nonexistent; when we come to them we'll mark
         * them as existing (using goodfruit())
         */
        for(f=ffruit; f; f=f->nextf) f->fid = -f->fid;

        /* check iron balls separately--maybe they're not carrying it */
        if (uball) uball->owornmask = uchain->owornmask = 0;

        /* dispose of your possessions, usually cursed */
        if (u.ugrave_arise == (NON_PM - 1)) {
                struct obj *otmp;

                /* embed your possessions in your statue */
                otmp = mk_named_object(STATUE, &mons[u.umonnum],
                                       u.ux, u.uy, plname);

                drop_upon_death((struct monst *)0, otmp);
                if (!otmp) return;      /* couldn't make statue */
                mtmp = (struct monst *)0;
        } else if (u.ugrave_arise < LOW_PM) {
                /* drop everything */
                drop_upon_death((struct monst *)0, (struct obj *)0);
                /* trick makemon() into allowing monster creation
                 * on your location
                 */
                in_mklev = true;
                mtmp = makemon(&mons[PM_GHOST], u.ux, u.uy, MM_NONAME);
                in_mklev = false;
                if (!mtmp) return;
                mtmp = christen_monst(mtmp, plname);
                if (corpse)
                        (void) obj_attach_mid(corpse, mtmp->m_id);
        } else {
                /* give your possessions to the monster you become */
                in_mklev = true;
                mtmp = makemon(&mons[u.ugrave_arise], u.ux, u.uy, NO_MM_FLAGS);
                in_mklev = false;
                if (!mtmp) {
                        drop_upon_death((struct monst *)0, (struct obj *)0);
                        return;
                }
                mtmp = christen_monst(mtmp, plname);
                newsym(u.ux, u.uy);
                Your("body rises from the dead as %s...",
                        an(mons[u.ugrave_arise].mname));
                // display_nhwindow(WIN_MESSAGE, false);
                drop_upon_death(mtmp, (struct obj *)0);
                m_dowear(mtmp, true);
        }
        if (mtmp) {
                mtmp->m_lev = (u.ulevel ? u.ulevel : 1);
                mtmp->mhp = mtmp->mhpmax = u.uhpmax;
                mtmp->female = flags.female;
                mtmp->msleeping = 1;
        }
        for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
                resetobjs(mtmp->minvent,false);
                /* do not zero out m_ids for bones levels any more */
                mtmp->mlstmv = 0L;
                if(mtmp->mtame) mtmp->mtame = mtmp->mpeaceful = 0;
        }
        for(ttmp = ftrap; ttmp; ttmp = ttmp->ntrap) {
                ttmp->madeby_u = 0;
                ttmp->tseen = (ttmp->ttyp == HOLE);
        }
        resetobjs(fobj,false);
        resetobjs(level.buriedobjlist, false);

        /* Hero is no longer on the map. */
        u.ux = u.uy = 0;

        /* Clear all memory from the level. */
        for(x=0; x<COLNO; x++) for(y=0; y<ROWNO; y++) {
            levl[x][y].seenv = 0;
            levl[x][y].waslit = 0;
            levl[x][y].glyph = cmap_to_glyph(S_stone);
        }

        fd = create_bonesfile(&u.uz, &bonesid, whynot);
        if(fd < 0) {
                if(flags.debug)
                        pline("%s", whynot);
                /* bones file creation problems are silent to the player.
                 * Keep it that way, but place a clue into the paniclog.
                 */
                paniclog("savebones", whynot);
                return;
        }
        c = (char) (strlen(bonesid) + 1);

        store_version(fd);
        bwrite(fd, (void *) &c, sizeof c);
        bwrite(fd, (void *) bonesid, (unsigned) c);     /* DD.nnn */
        savefruitchn(fd, WRITE_SAVE | FREE_SAVE);
        update_mlstmv();        /* update monsters for eventual restoration */
        savelev(fd, ledger_no(&u.uz), WRITE_SAVE | FREE_SAVE);
        bclose(fd);
        commit_bonesfile(&u.uz);
}

int getbones(void) {
    int fd;
    int ok;
    char c, *bonesid, oldbonesid[10];

    /* flags.debug check added by GAN 02/05/87 */
    /* only once in three times do we find bones */
    if (rn2(3) && !flags.debug)
        return 0;
    if (no_bones_level(&u.uz))
        return 0;
    fd = open_bonesfile(&u.uz, &bonesid);
    if (fd < 0)
        return 0;

    if ((ok = uptodate(fd, bones)) == 0) {
        if (!flags.debug)
            pline("Discarding unuseable bones; no need to panic...");
    } else {
        if (flags.debug) {
            if (yn("Get bones?") == 'n') {
                close(fd);
                return (0);
            }
        }
        mread(fd, (void *)&c, sizeof c); /* length incl. '\0' */
        mread(fd, (void *)oldbonesid, (unsigned)c); /* DD.nnn */
        if (strcmp(bonesid, oldbonesid) != 0) {
            char errbuf[BUFSZ];

            sprintf(errbuf, "This is bones level '%s', not '%s'!", oldbonesid, bonesid);
            if (flags.debug) {
                pline("%s", errbuf);
                ok = false; /* won't die of trickery */
            }
            trickery(errbuf);
        } else {
            struct monst *mtmp;

            getlev(fd, 0, 0, true);

            /* Note that getlev() now keeps tabs on unique
             * monsters such as demon lords, and tracks the
             * birth counts of all species just as makemon()
             * does.  If a bones monster is extinct or has been
             * subject to genocide, their mhpmax will be
             * set to the magic DEFUNCT_MONSTER cookie value.
             */
            for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
                if (mtmp->mhpmax == DEFUNCT_MONSTER) {
                    mongone(mtmp);
                } else
                    /* to correctly reset named artifacts on the level */
                    resetobjs(mtmp->minvent, true);
            }
            resetobjs(fobj, true);
            resetobjs(level.buriedobjlist, true);
        }
    }
    close(fd);

    if (flags.debug) {
        if (yn("Unlink bones?") == 'n') {
            return ok;
        }
    }
    if (!delete_bonesfile(&u.uz)) {
        /* When N games try to simultaneously restore the same
         * bones file, N-1 of them will fail to delete it
         * (the first N-1 under AmigaDOS, the last N-1 under UNIX).
         * So no point in a mysterious message for a normal event
         * -- just generate a new level for those N-1 games.
         */
        return 0;
    }
    return ok;
}
