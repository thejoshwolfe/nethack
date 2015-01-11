/* See LICENSE in the root of this project for change info */

#include "save.h"

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "artifact.h"
#include "attrib.h"
#include "decl.h"
#include "display.h"
#include "dungeon.h"
#include "eat.h"
#include "end.h"
#include "engrave.h"
#include "files.h"
#include "flag.h"
#include "global.h"
#include "hack.h"
#include "invent.h"
#include "lev.h"
#include "light.h"
#include "mkmaze.h"
#include "mkobj.h"
#include "mkroom.h"
#include "mon.h"
#include "monst.h"
#include "o_init.h"
#include "obj.h"
#include "objclass.h"
#include "onames.h"
#include "permonst.h"
#include "pline.h"
#include "quest.h"
#include "questpgr.h"
#include "region.h"
#include "restore.h"
#include "rm.h"
#include "rumors.h"
#include "spell.h"
#include "timeout.h"
#include "trap.h"
#include "util.h"
#include "vision.h"
#include "worm.h"
#include "you.h"

/* need to preserve these during save to avoid accessing freed memory */
static unsigned ustuck_id = 0, usteed_id = 0;

static int bw_fd = -1;
static FILE *bw_FILE = 0;
static bool buffering = false;
static bool savestateinlock_havestate = true;

int dosave (void) {
    clear_nhwindow(WIN_MESSAGE);
    if(yn("Really save?") == 'n') {
        clear_nhwindow(WIN_MESSAGE);
    } else {
        clear_nhwindow(WIN_MESSAGE);
        pline("Saving...");
        program_state.done_hup = 0;
        if(dosave0()) {
            program_state.something_worth_saving = 0;
            u.uhp = -1;             /* universal game's over indicator */
            /* make sure they see the Saving message */
            display_nhwindow(WIN_MESSAGE, true);
            exit(0);
        } else (void)doredraw();
    }
    return 0;
}


// called as signal() handler, so sent at least one arg
void hangup(int sig_unused) {
    if (!program_state.done_hup++) {
        if (program_state.something_worth_saving)
            (void) dosave0();
        {
            exit(1);
        }
    }
    return;
}

static void saveobjchn (int fd, struct obj *otmp, int mode) {
    struct obj *otmp2;
    unsigned int xl;
    int minusone = -1;

    while(otmp) {
        otmp2 = otmp->nobj;
        if (perform_bwrite(mode)) {
            xl = otmp->oxlth + otmp->onamelth;
            bwrite(fd, (void *) &xl, sizeof(int));
            bwrite(fd, (void *) otmp, xl + sizeof(struct obj));
        }
        if (Has_contents(otmp))
            saveobjchn(fd,otmp->cobj,mode);
        if (release_data(mode)) {
            if (otmp->oclass == FOOD_CLASS) food_disappears(otmp);
            if (otmp->oclass == SPBOOK_CLASS) book_disappears(otmp);
            otmp->where = OBJ_FREE; /* set to free so dealloc will work */
            otmp->timed = 0;        /* not timed any more */
            otmp->lamplit = 0;      /* caller handled lights */
            dealloc_obj(otmp);
        }
        otmp = otmp2;
    }
    if (perform_bwrite(mode))
        bwrite(fd, (void *) &minusone, sizeof(int));
}

static void savemonchn (int fd, struct monst *mtmp, int mode) {
    struct monst *mtmp2;
    unsigned int xl;
    int minusone = -1;
    struct permonst *monbegin = &mons[0];

    if (perform_bwrite(mode))
        bwrite(fd, (void *) &monbegin, sizeof(monbegin));

    while (mtmp) {
        mtmp2 = mtmp->nmon;
        if (perform_bwrite(mode)) {
            xl = mtmp->mxlth + mtmp->mnamelth;
            bwrite(fd, (void *) &xl, sizeof(int));
            bwrite(fd, (void *) mtmp, xl + sizeof(struct monst));
        }
        if (mtmp->minvent)
            saveobjchn(fd,mtmp->minvent,mode);
        if (release_data(mode))
            dealloc_monst(mtmp);
        mtmp = mtmp2;
    }
    if (perform_bwrite(mode))
        bwrite(fd, (void *) &minusone, sizeof(int));
}

static void savelevchn (int fd, int mode) {
    s_level *tmplev, *tmplev2;
    int cnt = 0;

    for (tmplev = sp_levchn; tmplev; tmplev = tmplev->next) cnt++;
    if (perform_bwrite(mode))
        bwrite(fd, (void *) &cnt, sizeof(int));

    for (tmplev = sp_levchn; tmplev; tmplev = tmplev2) {
        tmplev2 = tmplev->next;
        if (perform_bwrite(mode))
            bwrite(fd, (void *) tmplev, sizeof(s_level));
        if (release_data(mode))
            free((void *) tmplev);
    }
    if (release_data(mode))
        sp_levchn = 0;
}


static void savegamestate (int fd, int mode) {
    int uid;

    uid = getuid();
    bwrite(fd, (void *) &uid, sizeof uid);
    bwrite(fd, (void *) &flags, sizeof(struct flag));
    bwrite(fd, (void *) &u, sizeof(struct you));

    /* must come before migrating_objs and migrating_mons are freed */
    save_timers(fd, mode, RANGE_GLOBAL);
    save_light_sources(fd, mode, RANGE_GLOBAL);

    saveobjchn(fd, invent, mode);
    saveobjchn(fd, migrating_objs, mode);
    savemonchn(fd, migrating_mons, mode);
    if (release_data(mode)) {
        invent = 0;
        migrating_objs = 0;
        migrating_mons = 0;
    }
    bwrite(fd, (void *) mvitals, sizeof(mvitals));

    save_dungeon(fd, (bool)!!perform_bwrite(mode),
            (bool)!!release_data(mode));
    savelevchn(fd, mode);
    bwrite(fd, (void *) &moves, sizeof moves);
    bwrite(fd, (void *) &monstermoves, sizeof monstermoves);
    bwrite(fd, (void *) &quest_status, sizeof(struct q_score));
    bwrite(fd, (void *) spl_book,
            sizeof(struct spell) * (MAXSPELL + 1));
    save_artifacts(fd);
    save_oracles(fd, mode);
    if(ustuck_id)
        bwrite(fd, (void *) &ustuck_id, sizeof ustuck_id);
    if(usteed_id)
        bwrite(fd, (void *) &usteed_id, sizeof usteed_id);
    bwrite(fd, (void *) pl_character, sizeof pl_character);
    bwrite(fd, (void *) pl_fruit, sizeof pl_fruit);
    bwrite(fd, (void *) &current_fruit, sizeof current_fruit);
    savefruitchn(fd, mode);
    savenames(fd, mode);
    save_waterlevel(fd, mode);
    bflush(fd);
}

/* returns 1 if save successful */
int dosave0(void) {
    int fd, ofd;
    signed char ltmp;
    d_level uz_save;
    char whynot[BUFSZ];

    if (!SAVEF[0])
        return 0;

    signal(SIGHUP, SIG_IGN);
    signal(SIGINT, SIG_IGN);

    if (!program_state.done_hup) {
        if (iflags.window_inited) {
            fd = open_savefile();
            if (fd > 0) {
                (void) close(fd);
                clear_nhwindow(WIN_MESSAGE);
                There("seems to be an old save file.");
                if (yn("Overwrite the old file?") == 'n') {
                    return 0;
                }
            }
        }
    }


    fd = create_savefile();
    if(fd < 0) {
        if (!program_state.done_hup)
            pline("Cannot open save file.");
        (void) delete_savefile();       /* ab@unido */
        return(0);
    }

    vision_recalc(2);       /* shut down vision to prevent problems
                               in the event of an impossible() call */

    /* undo date-dependent luck adjustments made at startup time */
    if(flags.moonphase == FULL_MOON)        /* ut-sally!fletcher */
        change_luck(-1);                /* and unido!ab */
    if(flags.friday13)
        change_luck(1);
    if(iflags.window_inited)
        if (!program_state.done_hup) clear_nhwindow(WIN_MESSAGE);

    store_version(fd);
    ustuck_id = (u.ustuck ? u.ustuck->m_id : 0);
    usteed_id = (u.usteed ? u.usteed->m_id : 0);
    savelev(fd, ledger_no(&u.uz), WRITE_SAVE | FREE_SAVE);
    savegamestate(fd, WRITE_SAVE | FREE_SAVE);

    /* While copying level files around, zero out u.uz to keep
     * parts of the restore code from completely initializing all
     * in-core data structures, since all we're doing is copying.
     * This also avoids at least one nasty core dump.
     */
    uz_save = u.uz;
    u.uz.dnum = u.uz.dlevel = 0;
    /* these pointers are no longer valid, and at least u.usteed
     * may mislead place_monster() on other levels
     */
    u.ustuck = (struct monst *)0;
    u.usteed = (struct monst *)0;

    for(ltmp = (signed char)1; ltmp <= maxledgerno(); ltmp++) {
        if (ltmp == ledger_no(&uz_save)) continue;
        if (!(level_info[ltmp].flags & LFILE_EXISTS)) continue;
        ofd = open_levelfile(ltmp, whynot);
        if (ofd < 0) {
            if (!program_state.done_hup) pline("%s", whynot);
            (void) close(fd);
            (void) delete_savefile();
            if (!program_state.done_hup) {
                fprintf(stderr, "TODO: killer = %s\n", whynot);
                done(TRICKED);
            }
            return(0);
        }
        getlev(ofd, hackpid, ltmp, false);
        (void) close(ofd);
        bwrite(fd, (void *) &ltmp, sizeof ltmp); /* level number*/
        savelev(fd, ltmp, WRITE_SAVE | FREE_SAVE);     /* actual level*/
        delete_levelfile(ltmp);
    }
    bclose(fd);

    u.uz = uz_save;

    /* get rid of current level --jgm */
    delete_levelfile(ledger_no(&u.uz));
    delete_levelfile(0);
    return(1);
}

void savestateinlock (void) {
    char whynot[BUFSZ];

    /* When checkpointing is on, the full state needs to be written
     * on each checkpoint.  When checkpointing is off, only the pid
     * needs to be in the level.0 file, so it does not need to be
     * constantly rewritten.  When checkpointing is turned off during
     * a game, however, the file has to be rewritten once to truncate
     * it and avoid restoring from outdated information.
     *
     * Restricting havestate to this routine means that an additional
     * noop pid rewriting will take place on the first "checkpoint" after
     * the game is started or restored, if checkpointing is off.
     */
    if (flags.ins_chkpt || savestateinlock_havestate) {
        /* save the rest of the current game state in the lock file,
         * following the original int pid, the current level number,
         * and the current savefile name, which should not be subject
         * to any internal compression schemes since they must be
         * readable by an external utility
         */
        int fd = create_levelfile(0, whynot);
        if (fd < 0) {
            pline("%s", whynot);
            fprintf(stderr, "TODO: killer = %s\n", whynot);
            done(TRICKED);
            return;
        }
        (void) write(fd, (void *) &hackpid, sizeof(hackpid));
        if (flags.ins_chkpt) {
            int currlev = ledger_no(&u.uz);

            (void) write(fd, (void *) &currlev, sizeof(currlev));
            save_savefile_name(fd);
            store_version(fd);
            ustuck_id = (u.ustuck ? u.ustuck->m_id : 0);
            usteed_id = (u.usteed ? u.usteed->m_id : 0);
            savegamestate(fd, WRITE_SAVE);
        }
        bclose(fd);
    }
    savestateinlock_havestate = flags.ins_chkpt;
}

static void savetrapchn (int fd, struct trap *trap, int mode) {
    struct trap *trap2;

    while (trap) {
        trap2 = trap->ntrap;
        if (perform_bwrite(mode))
            bwrite(fd, (void *) trap, sizeof(struct trap));
        if (release_data(mode))
            dealloc_trap(trap);
        trap = trap2;
    }
    if (perform_bwrite(mode))
        bwrite(fd, (void *)nul, sizeof(struct trap));
}

static void savedamage (int fd, int mode) {
    struct damage *damageptr, *tmp_dam;
    unsigned int xl = 0;

    damageptr = level.damagelist;
    for (tmp_dam = damageptr; tmp_dam; tmp_dam = tmp_dam->next)
        xl++;
    if (perform_bwrite(mode))
        bwrite(fd, (void *) &xl, sizeof(xl));

    while (xl--) {
        if (perform_bwrite(mode))
            bwrite(fd, (void *) damageptr, sizeof(*damageptr));
        tmp_dam = damageptr;
        damageptr = damageptr->next;
        if (release_data(mode))
            free((void *)tmp_dam);
    }
    if (release_data(mode))
        level.damagelist = 0;
}


void savelev(int fd, signed char lev, int mode) {
    /* if we're tearing down the current level without saving anything
       (which happens upon entrance to the endgame or after an aborted
       restore attempt) then we don't want to do any actual I/O */
    if (mode == FREE_SAVE) goto skip_lots;
    if (iflags.purge_monsters) {
        /* purge any dead monsters (necessary if we're starting
         * a panic save rather than a normal one, or sometimes
         * when changing levels without taking time -- e.g.
         * create statue trap then immediately level teleport) */
        dmonsfree();
    }

    if(fd < 0) panic("Save on bad file!");  /* impossible */
    if (lev >= 0 && lev <= maxledgerno())
        level_info[lev].flags |= VISITED;
    bwrite(fd,(void *) &hackpid,sizeof(hackpid));
    bwrite(fd,(void *) &lev,sizeof(lev));
    bwrite(fd,(void *) levl,sizeof(levl));

    bwrite(fd,(void *) &monstermoves,sizeof(monstermoves));
    bwrite(fd,(void *) &upstair,sizeof(stairway));
    bwrite(fd,(void *) &dnstair,sizeof(stairway));
    bwrite(fd,(void *) &upladder,sizeof(stairway));
    bwrite(fd,(void *) &dnladder,sizeof(stairway));
    bwrite(fd,(void *) &sstairs,sizeof(stairway));
    bwrite(fd,(void *) &updest,sizeof(dest_area));
    bwrite(fd,(void *) &dndest,sizeof(dest_area));
    bwrite(fd,(void *) &level.flags,sizeof(level.flags));
    bwrite(fd, (void *) doors, sizeof(doors));
    save_rooms(fd); /* no dynamic memory to reclaim */

    /* from here on out, saving also involves allocated memory cleanup */
skip_lots:
    /* must be saved before mons, objs, and buried objs */
    save_timers(fd, mode, RANGE_LEVEL);
    save_light_sources(fd, mode, RANGE_LEVEL);

    savemonchn(fd, fmon, mode);
    save_worm(fd, mode);    /* save worm information */
    savetrapchn(fd, ftrap, mode);
    saveobjchn(fd, fobj, mode);
    saveobjchn(fd, level.buriedobjlist, mode);
    saveobjchn(fd, billobjs, mode);
    if (release_data(mode)) {
        fmon = 0;
        ftrap = 0;
        fobj = 0;
        level.buriedobjlist = 0;
        billobjs = 0;
    }
    save_engravings(fd, mode);
    savedamage(fd, mode);
    save_regions(fd, mode);
    if (mode != FREE_SAVE) bflush(fd);
}

void bufon (int fd) {
    if(bw_fd >= 0)
        panic("double buffering unexpected");
    bw_fd = fd;
    if((bw_FILE = fdopen(fd, "w")) == 0)
        panic("buffering of file %d failed", fd);
    buffering = true;
}

void bufoff (int fd) {
    bflush(fd);
    buffering = false;
}

void bflush (int fd) {
    if(fd == bw_fd) {
        if(fflush(bw_FILE) == EOF)
            panic("flush of savefile failed!");
    }
    return;
}

void bwrite (int fd, void *loc, unsigned num) {
    bool failed;

    if (buffering) {
        if(fd != bw_fd)
            panic("unbuffered write to fd %d (!= %d)", fd, bw_fd);

        failed = (fwrite(loc, (int)num, 1, bw_FILE) != 1);
    } else {
        failed = (write(fd, loc, num) != num);
    }

    if (failed) {
        if (program_state.done_hup)
            terminate(EXIT_FAILURE);
        else
            panic("cannot write %u bytes to file #%d", num, fd);
    }
}

void bclose(int fd) {
    bufoff(fd);
    if (fd == bw_fd) {
        (void) fclose(bw_FILE);
        bw_fd = -1;
        bw_FILE = 0;
    } else
        (void) close(fd);
    return;
}

/* save all the fruit names and ID's; this is used only in saving whole games
 * (not levels) and in saving bones levels.  When saving a bones level,
 * we only want to save the fruits which exist on the bones level; the bones
 * level routine marks nonexistent fruits by making the fid negative.
 */
void savefruitchn (int fd, int mode) {
    struct fruit *f2, *f1;

    f1 = ffruit;
    while (f1) {
        f2 = f1->nextf;
        if (f1->fid >= 0 && perform_bwrite(mode))
            bwrite(fd, (void *) f1, sizeof(struct fruit));
        if (release_data(mode))
            dealloc_fruit(f1);
        f1 = f2;
    }
    if (perform_bwrite(mode))
        bwrite(fd, (void *)nul, sizeof(struct fruit));
    if (release_data(mode))
        ffruit = 0;
}

/* also called by prscore(); this probably belongs in dungeon.c... */
void free_dungeons (void) {
    savelevchn(0, FREE_SAVE);
    save_dungeon(0, false, true);
    return;
}

void freedynamicdata (void) {
    unload_qtlist();
    free_invbuf();  /* let_to_name (invent.c) */
    free_youbuf();  /* You_buf,&c (pline.c) */
    tmp_at(DISP_FREEMEM, 0);        /* temporary display effects */
#define freeobjchn(X)  (saveobjchn(0, X, FREE_SAVE),  X = 0)
#define freemonchn(X)  (savemonchn(0, X, FREE_SAVE),  X = 0)
#define freetrapchn(X) (savetrapchn(0, X, FREE_SAVE), X = 0)
#define freefruitchn()  savefruitchn(0, FREE_SAVE)
#define freenames()     savenames(0, FREE_SAVE)
#define free_oracles() save_oracles(0, FREE_SAVE)
#define free_waterlevel() save_waterlevel(0, FREE_SAVE)
#define free_worm()     save_worm(0, FREE_SAVE)
#define free_timers(R)  save_timers(0, FREE_SAVE, R)
#define free_light_sources(R) save_light_sources(0, FREE_SAVE, R);
#define free_engravings() save_engravings(0, FREE_SAVE)
#define freedamage()    savedamage(0, FREE_SAVE)
#define free_animals()  mon_animal_list(false)

    /* move-specific data */
    dmonsfree();            /* release dead monsters */

    /* level-specific data */
    free_timers(RANGE_LEVEL);
    free_light_sources(RANGE_LEVEL);
    freemonchn(fmon);
    free_worm();            /* release worm segment information */
    freetrapchn(ftrap);
    freeobjchn(fobj);
    freeobjchn(level.buriedobjlist);
    freeobjchn(billobjs);
    free_engravings();
    freedamage();

    /* game-state data */
    free_timers(RANGE_GLOBAL);
    free_light_sources(RANGE_GLOBAL);
    freeobjchn(invent);
    freeobjchn(migrating_objs);
    freemonchn(migrating_mons);
    freemonchn(mydogs);             /* ascension or dungeon escape */
    /* freelevchn();   [folded into free_dungeons()] */
    free_animals();
    free_oracles();
    freefruitchn();
    freenames();
    free_waterlevel();
    free_dungeons();

    /* some pointers in iflags */
    if (iflags.wc_font_map) free(iflags.wc_font_map);
    if (iflags.wc_font_message) free(iflags.wc_font_message);
    if (iflags.wc_font_text) free(iflags.wc_font_text);
    if (iflags.wc_font_menu) free(iflags.wc_font_menu);
    if (iflags.wc_font_status) free(iflags.wc_font_status);
    if (iflags.wc_tile_file) free(iflags.wc_tile_file);

    return;
}
