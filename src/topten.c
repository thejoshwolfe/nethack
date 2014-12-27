/* See LICENSE in the root of this project for change info */

#include "hack.h"
#include "dlb.h"
#include "patchlevel.h"
#include "pm_props.h"
#include "extern.h"
#include "winprocs.h"
#include "flag.h"
#include "onames.h"
#include "artifact_names.h"

// larger file for debugging purposes
static const char * LOGFILE = "run/logfile";

#define done_stopprint program_state.stopprint

#define newttentry() (struct toptenentry *) malloc(sizeof(struct toptenentry))
#define dealloc_ttentry(ttent) free((void *) (ttent))
#define NAMSZ   10
#define DTHSZ   100
#define ROLESZ   3
#define PERSMAX  3              /* entries per name/uid per char. allowed */
#define POINTSMIN       1       /* must be > 0 */
#define ENTRYMAX        100     /* must be >= 10 */

#define PERS_IS_UID             /* delete for PERSMAX per name; now per uid */
struct toptenentry {
        struct toptenentry *tt_next;
        long points;
        int deathdnum, deathlev;
        int maxlvl, hp, maxhp, deaths;
        int ver_major, ver_minor, patchlevel;
        long deathdate, birthdate;
        int uid;
        char plrole[ROLESZ+1];
        char plrace[ROLESZ+1];
        char plgend[ROLESZ+1];
        char plalign[ROLESZ+1];
        char name[NAMSZ+1];
        char death[DTHSZ+1];
} *tt_head;

static void topten_print(const char *);
static void topten_print_bold(const char *);
static signed char observable_depth(d_level *);
static void outheader(void);
static void outentry(int,struct toptenentry *,bool);
static void readentry(FILE *,struct toptenentry *);
static void writeentry(FILE *,struct toptenentry *);
static void free_ttlist(struct toptenentry *);
static int classmon(char *,bool);
static int score_wanted(bool, int,struct toptenentry *,int,const char **,int);

/* must fit with end.c; used in rip.c */
const char * const killed_by_prefix[] = {
        "killed by ", "choked on ", "poisoned by ", "died of ", "drowned in ",
        "burned by ", "dissolved in ", "crushed to death by ", "petrified by ",
        "turned to slime by ", "killed by ", "", "", "", "", ""
};

static winid toptenwin = WIN_ERR;

static void
topten_print (const char *x)
{
        if (toptenwin == WIN_ERR)
            raw_print(x);
        else
            putstr(toptenwin, ATR_NONE, x);
}

static void
topten_print_bold (const char *x)
{
        if (toptenwin == WIN_ERR)
            raw_print_bold(x);
        else
            putstr(toptenwin, ATR_BOLD, x);
}

static signed char
observable_depth (d_level *lev)
{
            return depth(lev);
}

static void 
readentry (FILE *rfile, struct toptenentry *tt)
{
        static const char fmt[] = "%d.%d.%d %ld %d %d %d %d %d %d %ld %ld %d ";
        static const char fmt32[] = "%c%c %[^,],%[^\n]%*c";
        static const char fmt33[] = "%s %s %s %s %[^,],%[^\n]%*c";

        const int TTFIELDS = 13;
        if(fscanf(rfile, fmt,
                        &tt->ver_major, &tt->ver_minor, &tt->patchlevel,
                        &tt->points, &tt->deathdnum, &tt->deathlev,
                        &tt->maxlvl, &tt->hp, &tt->maxhp, &tt->deaths,
                        &tt->deathdate, &tt->birthdate,
                        &tt->uid) != TTFIELDS)
                tt->points = 0;
        else {
                /* Check for backwards compatibility */
                if (tt->ver_major < 3 ||
                                (tt->ver_major == 3 && tt->ver_minor < 3)) {
                        int i;

                    if (fscanf(rfile, fmt32,
                                tt->plrole, tt->plgend,
                                tt->name, tt->death) != 4)
                        tt->points = 0;
                    tt->plrole[1] = '\0';
                    if ((i = str2role(tt->plrole)) >= 0)
                        strcpy(tt->plrole, roles[i].filecode);
                    strcpy(tt->plrace, "?");
                    strcpy(tt->plgend, (tt->plgend[0] == 'M') ? "Mal" : "Fem");
                    strcpy(tt->plalign, "?");
                } else if (fscanf(rfile, fmt33,
                                tt->plrole, tt->plrace, tt->plgend,
                                tt->plalign, tt->name, tt->death) != 6)
                        tt->points = 0;
        }

        /* check old score entries for Y2K problem and fix whenever found */
        if (tt->points > 0) {
                if (tt->birthdate < 19000000L) tt->birthdate += 19000000L;
                if (tt->deathdate < 19000000L) tt->deathdate += 19000000L;
        }
}

static void 
writeentry (FILE *rfile, struct toptenentry *tt)
{
        (void) fprintf(rfile,"%d.%d.%d %ld %d %d %d %d %d %d %ld %ld %d ",
                tt->ver_major, tt->ver_minor, tt->patchlevel,
                tt->points, tt->deathdnum, tt->deathlev,
                tt->maxlvl, tt->hp, tt->maxhp, tt->deaths,
                tt->deathdate, tt->birthdate, tt->uid);
        if (tt->ver_major < 3 ||
                        (tt->ver_major == 3 && tt->ver_minor < 3))
                (void) fprintf(rfile,"%c%c %s,%s\n",
                        tt->plrole[0], tt->plgend[0],
                        onlyspace(tt->name) ? "_" : tt->name, tt->death);
        else
                (void) fprintf(rfile,"%s %s %s %s %s,%s\n",
                        tt->plrole, tt->plrace, tt->plgend, tt->plalign,
                        onlyspace(tt->name) ? "_" : tt->name, tt->death);

}

static void
free_ttlist (struct toptenentry *tt)
{
        struct toptenentry *ttnext;

        while (tt->points > 0) {
                ttnext = tt->tt_next;
                dealloc_ttentry(tt);
                tt = ttnext;
        }
        dealloc_ttentry(tt);
}

void topten (int how) {
        int uid = getuid();
        int rank, rank0 = -1, rank1 = 0;
        int occ_cnt = PERSMAX;
        struct toptenentry *t0, *tprev;
        struct toptenentry *t1;
        FILE *rfile;
        int flg = 0;
        bool t0_used;


/* If we are in the midst of a panic, cut out topten entirely.
 * topten uses malloc() several times, which will lead to
 * problems if the panic was the result of an malloc() failure.
 */
        if (program_state.panicking)
                return;

        if (flags.toptenwin) {
            toptenwin = create_nhwindow(NHW_TEXT);
        }

#define HUP     if (!program_state.done_hup)

        /* create a new 'topten' entry */
        t0_used = false;
        t0 = newttentry();
        /* deepest_lev_reached() is in terms of depth(), and reporting the
         * deepest level reached in the dungeon death occurred in doesn't
         * seem right, so we have to report the death level in depth() terms
         * as well (which also seems reasonable since that's all the player
         * sees on the screen anyway)
         */
        t0->ver_major = VERSION_MAJOR;
        t0->ver_minor = VERSION_MINOR;
        t0->patchlevel = PATCHLEVEL;
        t0->points = u.urexp;
        t0->deathdnum = u.uz.dnum;
        t0->deathlev = observable_depth(&u.uz);
        t0->maxlvl = deepest_lev_reached(true);
        t0->hp = u.uhp;
        t0->maxhp = u.uhpmax;
        t0->deaths = u.umortality;
        t0->uid = uid;
        (void) strncpy(t0->plrole, urole.filecode, ROLESZ);
        t0->plrole[ROLESZ] = '\0';
        (void) strncpy(t0->plrace, urace.filecode, ROLESZ);
        t0->plrace[ROLESZ] = '\0';
        (void) strncpy(t0->plgend, genders[flags.female].filecode, ROLESZ);
        t0->plgend[ROLESZ] = '\0';
        (void) strncpy(t0->plalign, aligns[1-u.ualign.type].filecode, ROLESZ);
        t0->plalign[ROLESZ] = '\0';
        (void) strncpy(t0->name, plname, NAMSZ);
        t0->name[NAMSZ] = '\0';
        t0->death[0] = '\0';
        switch (killer_format) {
                default: impossible("bad killer format?");
                case KILLED_BY_AN:
                        strcat(t0->death, killed_by_prefix[how]);
                        (void) strncat(t0->death, an(killer),
                                                DTHSZ-strlen(t0->death));
                        break;
                case KILLED_BY:
                        strcat(t0->death, killed_by_prefix[how]);
                        (void) strncat(t0->death, killer,
                                                DTHSZ-strlen(t0->death));
                        break;
                case NO_KILLER_PREFIX:
                        (void) strncat(t0->death, killer, DTHSZ);
                        break;
        }
        t0->birthdate = yyyymmdd(u.ubirthday);
        t0->deathdate = yyyymmdd((time_t)0L);
        t0->tt_next = 0;

        if (wizard || discover) {
            if (how != PANICKED) HUP {
                char pbuf[BUFSZ];
                topten_print("");
                sprintf(pbuf,
              "Since you were in %s mode, the score list will not be checked.",
                    wizard ? "wizard" : "discover");
                topten_print(pbuf);
                if (dump_fn[0]) {
                  dump("", pbuf);
                  dump("", "");
                }
            }
            goto showwin;
        }

        if (!lock_file(RECORD, SCOREPREFIX, 60))
                goto destroywin;

        rfile = fopen_datafile(RECORD, "r", SCOREPREFIX);

        if (!rfile) {
                HUP raw_print("Cannot open record file!");
                unlock_file(RECORD);
                goto destroywin;
        }

        HUP topten_print("");
        dump("", "");

        /* assure minimum number of points */
        if(t0->points < POINTSMIN) t0->points = 0;

        t1 = tt_head = newttentry();
        tprev = 0;
        /* rank0: -1 undefined, 0 not_on_list, n n_th on list */
        for(rank = 1; ; ) {
            readentry(rfile, t1);
            if (t1->points < POINTSMIN) t1->points = 0;
            if(rank0 < 0 && t1->points < t0->points) {
                rank0 = rank++;
                if(tprev == 0)
                        tt_head = t0;
                else
                        tprev->tt_next = t0;
                t0->tt_next = t1;
                t0_used = true;
                occ_cnt--;
                flg++;          /* ask for a rewrite */
            } else tprev = t1;

            if(t1->points == 0) break;
            if(
                t1->uid == t0->uid &&
                !strncmp(t1->plrole, t0->plrole, ROLESZ) &&
                --occ_cnt <= 0) {
                    if(rank0 < 0) {
                        rank0 = 0;
                        rank1 = rank;
                        HUP {
                            char pbuf[BUFSZ];
                            sprintf(pbuf,
                          "You didn't beat your previous score of %ld points.",
                                    t1->points);
                            topten_print(pbuf);
                            topten_print("");
                            dump("", pbuf);
                            dump("", "");
                        }
                    }
                    if(occ_cnt < 0) {
                        flg++;
                        continue;
                    }
                }
            if(rank <= ENTRYMAX) {
                t1->tt_next = newttentry();
                t1 = t1->tt_next;
                rank++;
            }
            if(rank > ENTRYMAX) {
                t1->points = 0;
                break;
            }
        }
        if(flg) {       /* rewrite record file */
                (void) fclose(rfile);
                if(!(rfile = fopen_datafile(RECORD, "w", SCOREPREFIX))){
                        HUP raw_print("Cannot write record file");
                        unlock_file(RECORD);
                        free_ttlist(tt_head);
                        goto destroywin;
                }
                if(!done_stopprint) if(rank0 > 0){
                    if(rank0 <= 10) {
                        topten_print("You made the top ten list!");
                        dump("", "You made the top ten list!");
                    } else {
                        char pbuf[BUFSZ];
                        sprintf(pbuf,
                          "You reached the %d%s place on the top %d list.",
                                rank0, ordin(rank0), ENTRYMAX);
                        topten_print(pbuf);
                        dump("", pbuf);
                    }
                    topten_print("");
                    dump("", "");
                }
        }
        if(rank0 == 0) rank0 = rank1;
        if(rank0 <= 0) rank0 = rank;
        if(!done_stopprint) outheader();
        t1 = tt_head;
        for(rank = 1; t1->points != 0; rank++, t1 = t1->tt_next) {
            if(flg
                ) writeentry(rfile, t1);
            if (done_stopprint) continue;
            if (rank > flags.end_top &&
                    (rank < rank0 - flags.end_around ||
                     rank > rank0 + flags.end_around) &&
                    (!flags.end_own ||
                                        t1->uid != t0->uid
                )) continue;
            if (rank == rank0 - flags.end_around &&
                    rank0 > flags.end_top + flags.end_around + 1 &&
                    !flags.end_own) {
                topten_print("");
                dump("", "");
            }
            if(rank != rank0)
                outentry(rank, t1, false);
            else if(!rank1)
                outentry(rank, t1, true);
            else {
                outentry(rank, t1, true);
                outentry(0, t0, true);
            }
        }
        if(rank0 >= rank) if(!done_stopprint)
                outentry(0, t0, true);
        (void) fclose(rfile);
        unlock_file(RECORD);
        free_ttlist(tt_head);

  showwin:
        if (flags.toptenwin && !done_stopprint) display_nhwindow(toptenwin, 1);
  destroywin:
        if (!t0_used) dealloc_ttentry(t0);
        if (flags.toptenwin) {
            destroy_nhwindow(toptenwin);
            toptenwin=WIN_ERR;
        }
}

static void
outheader (void)
{
        char linebuf[BUFSZ];
        char *bp;

        strcpy(linebuf, " No  Points     Name");
        bp = eos(linebuf);
        while(bp < linebuf + COLNO - 9) *bp++ = ' ';
        strcpy(bp, "Hp [max]");
        topten_print(linebuf);
        dump("", linebuf);
}

/* so=1: standout line; so=0: ordinary line */
static void outentry (int rank, struct toptenentry *t1, bool so) {
        bool second_line = true;
        char linebuf[BUFSZ];
        char *bp, hpbuf[24], linebuf3[BUFSZ];
        int hppos, lngr;


        linebuf[0] = '\0';
        if (rank) sprintf(eos(linebuf), "%3d", rank);
        else strcat(linebuf, "   ");

        sprintf(eos(linebuf), " %10ld  %.10s", t1->points, t1->name);
        sprintf(eos(linebuf), "-%s", t1->plrole);
        if (t1->plrace[0] != '?')
                sprintf(eos(linebuf), "-%s", t1->plrace);
        /* Printing of gender and alignment is intentional.  It has been
         * part of the NetHack Geek Code, and illustrates a proper way to
         * specify a character from the command line.
         */
        sprintf(eos(linebuf), "-%s", t1->plgend);
        if (t1->plalign[0] != '?')
                sprintf(eos(linebuf), "-%s ", t1->plalign);
        else
                strcat(linebuf, " ");
        if (!strncmp("escaped", t1->death, 7)) {
            sprintf(eos(linebuf), "escaped the dungeon %s[max level %d]",
                    !strncmp(" (", t1->death + 7, 2) ? t1->death + 7 + 2 : "",
                    t1->maxlvl);
            /* fixup for closing paren in "escaped... with...Amulet)[max..." */
            if ((bp = index(linebuf, ')')) != 0)
                *bp = (t1->deathdnum == astral_level.dnum) ? '\0' : ' ';
            second_line = false;
        } else if (!strncmp("ascended", t1->death, 8)) {
            sprintf(eos(linebuf), "ascended to demigod%s-hood",
                    (t1->plgend[0] == 'F') ? "dess" : "");
            second_line = false;
        } else {
            if (!strncmp(t1->death, "quit", 4)) {
                strcat(linebuf, "quit");
                second_line = false;
            } else if (!strncmp(t1->death, "died of st", 10)) {
                strcat(linebuf, "starved to death");
                second_line = false;
            } else if (!strncmp(t1->death, "choked", 6)) {
                sprintf(eos(linebuf), "choked on h%s food",
                        (t1->plgend[0] == 'F') ? "er" : "is");
            } else if (!strncmp(t1->death, "poisoned", 8)) {
                strcat(linebuf, "was poisoned");
            } else if (!strncmp(t1->death, "crushed", 7)) {
                strcat(linebuf, "was crushed to death");
            } else if (!strncmp(t1->death, "petrified by ", 13)) {
                strcat(linebuf, "turned to stone");
            } else strcat(linebuf, "died");

            if (t1->deathdnum == astral_level.dnum) {
                const char *arg, *fmt = " on the Plane of %s";

                switch (t1->deathlev) {
                case -5:
                        fmt = " on the %s Plane";
                        arg = "Astral"; break;
                case -4:
                        arg = "Water";  break;
                case -3:
                        arg = "Fire";   break;
                case -2:
                        arg = "Air";    break;
                case -1:
                        arg = "Earth";  break;
                default:
                        arg = "Void";   break;
                }
                sprintf(eos(linebuf), fmt, arg);
            } else {
                sprintf(eos(linebuf), " in %s", dungeons[t1->deathdnum].dname);
                if (t1->deathdnum != knox_level.dnum)
                    sprintf(eos(linebuf), " on level %d", t1->deathlev);
                if (t1->deathlev != t1->maxlvl)
                    sprintf(eos(linebuf), " [max %d]", t1->maxlvl);
            }

            /* kludge for "quit while already on Charon's boat" */
            if (!strncmp(t1->death, "quit ", 5))
                strcat(linebuf, t1->death + 4);
        }
        strcat(linebuf, ".");

        /* Quit, starved, ascended, and escaped contain no second line */
        if (second_line)
            sprintf(eos(linebuf), "  %c%s.", highc(*(t1->death)), t1->death+1);

        lngr = (int)strlen(linebuf);
        if (t1->hp <= 0) hpbuf[0] = '-', hpbuf[1] = '\0';
        else sprintf(hpbuf, "%d", t1->hp);
        /* beginning of hp column after padding (not actually padded yet) */
        hppos = COLNO - (sizeof("  Hp [max]")-1); /* sizeof(str) includes \0 */
        while (lngr >= hppos) {
            for(bp = eos(linebuf);
                    !(*bp == ' ' && (bp-linebuf < hppos));
                    bp--)
                ;
            /* special case: if about to wrap in the middle of maximum
               dungeon depth reached, wrap in front of it instead */
            if (bp > linebuf + 5 && !strncmp(bp - 5, " [max", 5)) bp -= 5;
            strcpy(linebuf3, bp+1);
            *bp = 0;
            if (so) {
                while (bp < linebuf + (COLNO-1)) *bp++ = ' ';
                *bp = 0;
                topten_print_bold(linebuf);
                dump("*", linebuf[0]==' '? linebuf+1: linebuf);
            } else {
                topten_print(linebuf);
                dump(" ", linebuf[0]==' '? linebuf+1: linebuf);
            }
            sprintf(linebuf, "%15s %s", "", linebuf3);
            lngr = strlen(linebuf);
        }
        /* beginning of hp column not including padding */
        hppos = COLNO - 7 - (int)strlen(hpbuf);
        bp = eos(linebuf);

        if (bp <= linebuf + hppos) {
            /* pad any necessary blanks to the hit point entry */
            while (bp < linebuf + hppos) *bp++ = ' ';
            strcpy(bp, hpbuf);
            sprintf(eos(bp), " %s[%d]",
                    (t1->maxhp < 10) ? "  " : (t1->maxhp < 100) ? " " : "",
                    t1->maxhp);
        }

        if (so) {
            bp = eos(linebuf);
            while (bp < linebuf) *bp++ = ' ';
            *bp = 0;
            topten_print_bold(linebuf);
        } else
            topten_print(linebuf);
        dump(" ", linebuf[0]==' '? linebuf+1: linebuf);
}

static int score_wanted (bool current_ver, int rank,
        struct toptenentry *t1, int playerct, const char **players, int uid)
{
        int i;

        if (current_ver && (t1->ver_major != VERSION_MAJOR ||
                            t1->ver_minor != VERSION_MINOR ||
                            t1->patchlevel != PATCHLEVEL))
                return 0;

        if (!playerct && t1->uid == uid)
                return 1;

        for (i = 0; i < playerct; i++) {
            if (players[i][0] == '-' && index("pr", players[i][1]) &&
                players[i][2] == 0 && i + 1 < playerct) {
                char *arg = (char *)players[i + 1];
                if ((players[i][1] == 'p' &&
                     str2role(arg) == str2role(t1->plrole)) ||
                    (players[i][1] == 'r' &&
                     str2race(arg) == str2race(t1->plrace)))
                    return 1;
                i++;
            } else if (strcmp(players[i], "all") == 0 ||
                    strncmp(t1->name, players[i], NAMSZ) == 0 ||
                    (players[i][0] == '-' &&
                     players[i][1] == t1->plrole[0] &&
                     players[i][2] == 0) ||
                    (digit(players[i][0]) && rank <= atoi(players[i])))
                return 1;
        }
        return 0;
}

/*
 * print selected parts of score list.
 * argc >= 2, with argv[0] untrustworthy (directory names, et al.),
 * and argv[1] starting with "-s".
 */
void
prscore (int argc, char **argv)
{
        const char **players;
        int playerct, rank;
        bool current_ver = true, init_done = false;
        struct toptenentry *t1;
        FILE *rfile;
        bool match_found = false;
        int i;
        char pbuf[BUFSZ];
        int uid = -1;

        if (argc < 2 || strncmp(argv[1], "-s", 2)) {
                raw_printf("prscore: bad arguments (%d)", argc);
                return;
        }

        rfile = fopen_datafile(RECORD, "r", SCOREPREFIX);
        if (!rfile) {
                raw_print("Cannot open record file!");
                return;
        }

        /* If the score list isn't after a game, we never went through
         * initialization. */
        if (wiz1_level.dlevel == 0) {
                dlb_init();
                init_dungeons();
                init_done = true;
        }

        if (!argv[1][2]){       /* plain "-s" */
                argc--;
                argv++;
        } else  argv[1] += 2;

        if (argc > 1 && !strcmp(argv[1], "-v")) {
                current_ver = false;
                argc--;
                argv++;
        }

        if (argc <= 1) {
                uid = getuid();
                playerct = 0;
                players = (const char **)0;
        } else {
                playerct = --argc;
                players = (const char **)++argv;
        }
        raw_print("");

        t1 = tt_head = newttentry();
        for (rank = 1; ; rank++) {
            readentry(rfile, t1);
            if (t1->points == 0) break;
            if (!match_found &&
                    score_wanted(current_ver, rank, t1, playerct, players, uid))
                match_found = true;
            t1->tt_next = newttentry();
            t1 = t1->tt_next;
        }

        (void) fclose(rfile);
        if (init_done) {
            free_dungeons();
            dlb_cleanup();
        }

        if (match_found) {
            outheader();
            t1 = tt_head;
            for (rank = 1; t1->points != 0; rank++, t1 = t1->tt_next) {
                if (score_wanted(current_ver, rank, t1, playerct, players, uid))
                    (void) outentry(rank, t1, 0);
            }
        } else {
            sprintf(pbuf, "Cannot find any %sentries for ",
                                current_ver ? "current " : "");
            if (playerct < 1) strcat(pbuf, "you.");
            else {
                if (playerct > 1) strcat(pbuf, "any of ");
                for (i = 0; i < playerct; i++) {
                    /* stop printing players if there are too many to fit */
                    if (strlen(pbuf) + strlen(players[i]) + 2 >= BUFSZ) {
                        if (strlen(pbuf) < BUFSZ-4) strcat(pbuf, "...");
                        else strcpy(pbuf+strlen(pbuf)-4, "...");
                        break;
                    }
                    strcat(pbuf, players[i]);
                    if (i < playerct-1) {
                        if (players[i][0] == '-' &&
                            index("pr", players[i][1]) && players[i][2] == 0)
                            strcat(pbuf, " ");
                        else strcat(pbuf, ":");
                    }
                }
            }
            raw_print(pbuf);
            raw_printf("Usage: %s -s [-v] <playertypes> [maxrank] [playernames]",

                         hname);
            raw_printf("Player types are: [-p role] [-r race]");
        }
        free_ttlist(tt_head);
}

static int 
classmon (char *plch, bool fem)
{
        int i;

        /* Look for this role in the role table */
        for (i = 0; roles[i].name.m; i++)
            if (!strncmp(plch, roles[i].filecode, ROLESZ)) {
                if (fem && roles[i].femalenum != NON_PM)
                    return roles[i].femalenum;
                else if (roles[i].malenum != NON_PM)
                    return roles[i].malenum;
                else
                    return PM_HUMAN;
            }
        /* this might be from a 3.2.x score for former Elf class */
        if (!strcmp(plch, "E")) return PM_RANGER;

        impossible("What weird role is this? (%s)", plch);
        return (PM_HUMAN_MUMMY);
}

/*
 * Get a random player name and class from the high score list,
 * and attach them to an object (for statues or morgue corpses).
 */
struct obj *
tt_oname (struct obj *otmp)
{
        int rank;
        int i;
        struct toptenentry *tt;
        FILE *rfile;
        struct toptenentry tt_buf;

        if (!otmp) return((struct obj *) 0);

        rfile = fopen_datafile(RECORD, "r", SCOREPREFIX);
        if (!rfile) {
                impossible("Cannot open record file!");
                return (struct obj *)0;
        }

        tt = &tt_buf;
        rank = rnd(10);
pickentry:
        for(i = rank; i; i--) {
            readentry(rfile, tt);
            if(tt->points == 0) break;
        }

        if(tt->points == 0) {
                if(rank > 1) {
                        rank = 1;
                        rewind(rfile);
                        goto pickentry;
                }
                otmp = (struct obj *) 0;
        } else {
                /* reset timer in case corpse started out as lizard or troll */
                if (otmp->otyp == CORPSE) obj_stop_timers(otmp);
                otmp->corpsenm = classmon(tt->plrole, (tt->plgend[0] == 'F'));
                otmp->owt = weight(otmp);
                otmp = oname(otmp, tt->name);
                if (otmp->otyp == CORPSE) start_corpse_timeout(otmp);
        }

        (void) fclose(rfile);
        return otmp;
}
