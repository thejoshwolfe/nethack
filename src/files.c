/* See LICENSE in the root of this project for change info */

#include "files.h"

#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "decl.h"
#include "drawing.h"
#include "flag.h"
#include "global.h"
#include "hacklib.h"
#include "invent.h"
#include "monsym.h"
#include "objclass.h"
#include "objnam.h"
#include "options.h"
#include "pline.h"
#include "rm.h"
#include "role.h"
#include "util.h"
#include "you.h"

char bones[] = "bonesnn.xxx";
char lock[PL_NSIZ+14] = "run/1lock"; /* long enough for uid+name+.99 */

#define SAVESIZE        (PL_NSIZ + 13)  /* save/99999player.e */

char SAVEF[SAVESIZE];   /* holds relative path of save file from playground */

#define WIZKIT_MAX 128
static char wizkit[WIZKIT_MAX];
static FILE *fopen_wizkit_file(void);

extern int n_dgns;              /* from dungeon.c */

static char *set_bonesfile_name(char *,d_level*);
static char *set_bonestemp_name(void);
static int get_uchars(FILE *,char *,char *,unsigned char *,bool,int,const char *);


/*
 * fname_encode()
 *
 *   Args:
 *      legal           zero-terminated list of acceptable file name characters
 *      quotechar       lead-in character used to quote illegal characters as hex digits
 *      s               string to encode
 *      callerbuf       buffer to house result
 *      bufsz           size of callerbuf
 *
 *   Notes:
 *      The hex digits 0-9 and A-F are always part of the legal set due to
 *      their use in the encoding scheme, even if not explicitly included in 'legal'.
 *
 *   Sample:
 *      The following call:
 *          (void)fname_encode("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
 *                              '%', "This is a % test!", buf, 512);
 *      results in this encoding:
 *          "This%20is%20a%20%25%20test%21"
 */
char * fname_encode (const char *legal, char quotechar, char *s, char *callerbuf, int bufsz) {
    char *sp, *op;
    int cnt = 0;
    static char hexdigits[] = "0123456789ABCDEF";

    sp = s;
    op = callerbuf;
    *op = '\0';

    while (*sp) {
        /* Do we have room for one more character or encoding? */
        if ((bufsz - cnt) <= 4) return callerbuf;

        if (*sp == quotechar) {
            (void)sprintf(op, "%c%02X", quotechar, *sp);
            op += 3;
            cnt += 3;
        } else if ((index(legal, *sp) != 0) || (index(hexdigits, *sp) != 0)) {
            *op++ = *sp;
            *op = '\0';
            cnt++;
        } else {
            (void)sprintf(op,"%c%02X", quotechar, *sp);
            op += 3;
            cnt += 3;
        }
        sp++;
    }
    return callerbuf;
}

/*
 * fname_decode()
 *
 *   Args:
 *      quotechar       lead-in character used to quote illegal characters as hex digits
 *      s               string to decode
 *      callerbuf       buffer to house result
 *      bufsz           size of callerbuf
 */
char * fname_decode (char quotechar, char *s, char *callerbuf, int bufsz) {
    char *sp, *op;
    int k,calc,cnt = 0;
    static char hexdigits[] = "0123456789ABCDEF";

    sp = s;
    op = callerbuf;
    *op = '\0';
    calc = 0;

    while (*sp) {
        /* Do we have room for one more character? */
        if ((bufsz - cnt) <= 2) return callerbuf;
        if (*sp == quotechar) {
            sp++;
            for (k=0; k < 16; ++k) if (*sp == hexdigits[k]) break;
            if (k >= 16) return callerbuf;  /* impossible, so bail */
            calc = k << 4;
            sp++;
            for (k=0; k < 16; ++k) if (*sp == hexdigits[k]) break;
            if (k >= 16) return callerbuf;  /* impossible, so bail */
            calc += k;
            sp++;
            *op++ = calc;
            *op = '\0';
        } else {
            *op++ = *sp++;
            *op = '\0';
        }
        cnt++;
    }
    return callerbuf;
}

const char * fqname(const char * basename, int whichprefix, int buffnum) {
    return basename;
}

/* fopen a file, with OS-dependent bells and whistles */
/* NOTE: a simpler version of this routine also exists in util/dlb_main.c */
FILE * fopen_datafile(const char *filename, const char *mode, int prefix) {
    return fopen(filename, mode);
}

/* ----------  BEGIN LEVEL FILE HANDLING ----------- */

/* Construct a file name for a level-type file, which is of the form
 * something.level (with any old level stripped off).
 * This assumes there is space on the end of 'file' to append
 * a two digit number.  This is true for 'level'
 * but be careful if you use it for other things -dgk
 */
void set_levelfile_name(char *file, int lev) {
    char *tf;

    tf = rindex(file, '.');
    if (!tf) tf = eos(file);
    sprintf(tf, ".%d", lev);
    return;
}

int create_levelfile(int lev, char errbuf[]) {
    int fd;
    if (errbuf)
        *errbuf = '\0';
    set_levelfile_name(lock, lev);

    fd = creat(lock, 0660);

    if (fd >= 0)
        level_info[lev].flags |= LFILE_EXISTS;
    else if (errbuf) /* failure explanation */
        sprintf(errbuf, "Cannot create file \"%s\" for level %d (errno %d).", lock, lev, errno);

    return fd;
}

int open_levelfile(int lev, char errbuf[]) {
    int fd;
    if (errbuf)
        *errbuf = '\0';
    set_levelfile_name(lock, lev);
    fd = open(lock, O_RDONLY, 0);

    /* for failure, return an explanation that our caller can use;
     settle for `lock' instead of `fq_lock' because the latter
     might end up being too big for nethack's BUFSZ */
    if (fd < 0 && errbuf)
        sprintf(errbuf, "Cannot open file \"%s\" for level %d (errno %d).", lock, lev, errno);

    return fd;
}

void delete_levelfile(int lev) {
    /*
     * Level 0 might be created by port specific code that doesn't
     * call create_levfile(), so always assume that it exists.
     */
    if (lev == 0 || (level_info[lev].flags & LFILE_EXISTS)) {
        set_levelfile_name(lock, lev);
        unlink(lock);
        level_info[lev].flags &= ~LFILE_EXISTS;
    }
}


void clearlocks(void) {
    int x;

    signal(SIGHUP, SIG_IGN);
    /* can't access maxledgerno() before dungeons are created -dlc */
    for (x = (n_dgns ? maxledgerno() : 0); x >= 0; x--)
        delete_levelfile(x);    /* not all levels need be present */
}


/* ----------  END LEVEL FILE HANDLING ----------- */


/* ----------  BEGIN BONES FILE HANDLING ----------- */

/* set up "file" to be file name for retrieving bones, and return a
 * bonesid to be read/written in the bones file.
 */
static char * set_bonesfile_name (char *file, d_level *lev) {
    s_level *sptr;
    char *dptr;

    sprintf(file, "bon%c%s", dungeons[lev->dnum].boneid,
            In_quest(lev) ? urole.filecode : "0");
    dptr = eos(file);
    if ((sptr = Is_special(lev)) != 0)
        sprintf(dptr, ".%c", sptr->boneid);
    else
        sprintf(dptr, ".%d", lev->dlevel);
    return(dptr-2);
}

/* set up temporary file name for writing bones, to avoid another game's
 * trying to read from an uncompleted bones file.  we want an uncontentious
 * name, so use one in the namespace reserved for this game's level files.
 * (we are not reading or writing level files while writing bones files, so
 * the same array may be used instead of copying.)
 */
static char * set_bonestemp_name (void) {
    char *tf;

    tf = rindex(lock, '.');
    if (!tf) tf = eos(lock);
    sprintf(tf, ".bn");
    return lock;
}

int create_bonesfile (d_level *lev, char **bonesid, char errbuf[]) {
    if (errbuf) *errbuf = '\0';
    *bonesid = set_bonesfile_name(bones, lev);
    const char * file = set_bonestemp_name();

    int fd = creat(file, 0660);
    if (fd < 0 && errbuf) /* failure explanation */
        sprintf(errbuf,
                "Cannot create bones \"%s\", id %s (errno %d).",
                lock, *bonesid, errno);

    return fd;
}

/* move completed bones file to proper name */
void commit_bonesfile(d_level *lev) {
    const char *fq_bones, *tempname;
    int ret;

    set_bonesfile_name(bones, lev);
    tempname = set_bonestemp_name();

    ret = rename(tempname, bones);
    if (flags.debug && ret != 0)
        pline("couldn't rename %s to %s.", tempname, bones);
}

int open_bonesfile(d_level *lev, char **bonesid) {
    *bonesid = set_bonesfile_name(bones, lev);
    return open(bones, O_RDONLY, 0);
}

int delete_bonesfile (d_level *lev) {
    set_bonesfile_name(bones, lev);
    return !(unlink(bones) < 0);
}


/* ----------  END BONES FILE HANDLING ----------- */


/* ----------  BEGIN SAVE FILE HANDLING ----------- */

/* set savefile name in OS-dependent manner from pre-existing plname,
 * avoiding troublesome characters */
static void sanitize_filename(char * filename) {
    for (int i = 0; filename[i] != '\0'; i++) {
        switch (filename[i]) {
            case '.':
            case '/':
            case ' ':
                filename[i] = '_';
                break;
        }
    }
}

void init_savefile_name(void) {
    char sanitized_plname[BUFSZ];
    nh_slprintf(sanitized_plname, BUFSZ, "%s", plname);
    sanitize_filename(sanitized_plname);

    nh_slprintf(SAVEF, sizeof(SAVEF), "run/save/%s", sanitized_plname);
}

void save_savefile_name(int fd) {
    write(fd, (void *)SAVEF, sizeof(SAVEF));
}

/* change pre-existing savefile name to indicate an error savefile */
void set_error_savefile(void) {
    strcat(SAVEF, ".e");
}

/* create save file, overwriting one if it already exists */
int create_savefile(void) {
    return creat(SAVEF, 0660);
}

/* open savefile for reading */
int open_savefile(void) {
    return open(SAVEF, O_RDONLY, 0);
}

int delete_savefile(void) {
    unlink(SAVEF);
    /* for restore_saved_game() (ex-xxxmain.c) test */
    return 0;
}

/* try to open up a save file and prepare to restore it */
int restore_saved_game(void) {
    init_savefile_name();
    return open_savefile();
}

char ** get_saved_games (void) {
    return 0;
}

void free_saved_games(char **saved) {
    if (saved) {
        int i = 0;
        while (saved[i])
            free((void *)saved[i++]);
        free((void *)saved);
    }
}

/* ----------  END SAVE FILE HANDLING ----------- */

/* ----------  BEGIN SCOREBOARD CREATION ----------- */

/* verify that we can write to the scoreboard file; if not, try to create one */
void check_recordfile(const char *dir) {
    int fd = open(RECORD, O_RDWR, 0);
    if (fd >= 0) {
        close(fd); /* RECORD is accessible */
    } else if ((fd = open(RECORD, O_CREAT | O_RDWR, 0660)) >= 0) {
        close(fd); /* RECORD newly created */
    } else {
        raw_printf("Warning: cannot write scoreboard file %s: %s", RECORD, strerror(errno));
    }
}

/* ----------  END SCOREBOARD CREATION ----------- */


/* type: panic, impossible, trickery */
/* reason: explanation */
void paniclog(const char *type, const char *reason) {
    return;
}
