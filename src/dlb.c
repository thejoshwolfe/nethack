/* See LICENSE in the root of this project for change info */
#include "dlb.h"

#include <stdlib.h>
#include <string.h>

#define DATAPREFIX 4

/*
 * Data librarian.  Present a STDIO-like interface to NetHack while
 * multiplexing on one or more "data libraries".  If a file is not found
 * in a given library, look for it outside the libraries.
 */

typedef struct dlb_procs {
    bool (*dlb_init_proc)(void);
    void (*dlb_cleanup_proc)(void);
    bool (*dlb_fopen_proc)(dlb *,const char *,const char *);
    int (*dlb_fclose_proc)(dlb *);
    int (*dlb_fread_proc)(char *,int,int,dlb *);
    int (*dlb_fseek_proc)(dlb *,long,int);
    char *(*dlb_fgets_proc)(char *,int,dlb *);
    int (*dlb_fgetc_proc)(dlb *);
    long (*dlb_ftell_proc)(dlb *);
} dlb_procs_t;

/* without extern.h via hack.h, these haven't been declared for us */
extern FILE *fopen_datafile(const char *,const char *,int);

/*
 * Library Implementation:
 *
 * When initialized, we open all library files and read in their tables
 * of contents.  The library files stay open all the time.  When
 * a open is requested, the libraries' directories are searched.  If
 * successful, we return a descriptor that contains the library, file
 * size, and current file mark.  This descriptor is used for all
 * successive calls.
 *
 * The ability to open more than one library is supported but used
 * only in the Amiga port (the second library holds the sound files).
 * For Unix, the idea would be to split the NetHack library
 * into text and binary parts, where the text version could be shared.
 */

#define MAX_LIBS 4
static library dlb_libs[MAX_LIBS];

static bool readlibdir(library *lp);
static bool find_file(const char *name, library **lib, long *startp,
                                long *sizep);
static bool lib_dlb_init(void);
static void lib_dlb_cleanup(void);
static bool lib_dlb_fopen(dlb *, const char *, const char *);
static int lib_dlb_fclose(dlb *);
static int lib_dlb_fread(char *, int, int, dlb *);
static int lib_dlb_fseek(dlb *, long, int);
static char *lib_dlb_fgets(char *, int, dlb *);
static int lib_dlb_fgetc(dlb *);
static long lib_dlb_ftell(dlb *);

/* not static because shared with dlb_main.c */
bool open_library(const char *lib_name, library *lp);
void close_library(library *lp);

/* without extern.h via hack.h, these haven't been declared for us */
extern char *eos(char *);



/*
 * Read the directory out of the library.  Return 1 if successful,
 * 0 if it failed.
 *
 * NOTE: An improvement of the file structure should be the file
 * size as part of the directory entry or perhaps in place of the
 * offset -- the offset can be calculated by a running tally of
 * the sizes.
 *
 * Library file structure:
 *
 * HEADER:
 * %3ld    library FORMAT revision (currently rev 1)
 * %1c    space
 * %8ld    # of files in archive (includes 1 for directory)
 * %1c    space
 * %8ld    size of allocation for string space for directory names
 * %1c    space
 * %8ld    library offset - sanity check - lseek target for start of first file
 * %1c    space
 * %8ld    size - sanity check - byte size of complete archive file
 *
 * followed by one DIRECTORY entry for each file in the archive, including
 *  the directory itself:
 * %1c    handling information (compression, etc.)  Always ' ' in rev 1.
 * %s    file name
 * %1c    space
 * %8ld    offset in archive file of start of this file
 * %c    newline
 *
 * followed by the contents of the files
 */
#define DLB_MIN_VERS  1    /* min library version readable by this code */
#define DLB_MAX_VERS  1    /* max library version readable by this code */

/*
 * Read the directory from the library file.   This will allocate and
 * fill in our globals.  The file pointer is reset back to position
 * zero.  If any part fails, leave nothing that needs to be deallocated.
 *
 * Return true on success, false on failure.
 */
static bool 
readlibdir (
    library *lp    /* library pointer to fill in */
)
{
    int i;
    char *sp;
    long liboffset, totalsize;

    if (fscanf(lp->fdata, "%ld %ld %ld %ld %ld\n",
        &lp->rev,&lp->nentries,&lp->strsize,&liboffset,&totalsize) != 5)
    return false;
    if (lp->rev > DLB_MAX_VERS || lp->rev < DLB_MIN_VERS) return false;

    lp->dir = (libdir *) malloc(lp->nentries * sizeof(libdir));
    lp->sspace = (char *) malloc(lp->strsize);

    /* read in each directory entry */
    for (i = 0, sp = lp->sspace; i < lp->nentries; i++) {
    lp->dir[i].fname = sp;
    if (fscanf(lp->fdata, "%c%s %ld\n",
            &lp->dir[i].handling, sp, &lp->dir[i].foffset) != 3) {
        free((void *) lp->dir);
        free((void *) lp->sspace);
        lp->dir = (libdir *) 0;
        lp->sspace = (char *) 0;
        return false;
    }
    sp = eos(sp) + 1;
    }

    /* calculate file sizes using offset information */
    for (i = 0; i < lp->nentries; i++) {
    if (i == lp->nentries - 1)
        lp->dir[i].fsize = totalsize - lp->dir[i].foffset;
    else
        lp->dir[i].fsize = lp->dir[i+1].foffset - lp->dir[i].foffset;
    }

    (void) fseek(lp->fdata, 0L, SEEK_SET);    /* reset back to zero */
    lp->fmark = 0;

    return true;
}

/*
 * Look for the file in our directory structure.  Return 1 if successful,
 * 0 if not found.  Fill in the size and starting position.
 */
static bool 
find_file (const char *name, library **lib, long *startp, long *sizep)
{
    int i, j;
    library *lp;

    for (i = 0; i < MAX_LIBS && dlb_libs[i].fdata; i++) {
    lp = &dlb_libs[i];
    for (j = 0; j < lp->nentries; j++) {
        if (strcmp(name, lp->dir[j].fname) == 0) {
        *lib = lp;
        *startp = lp->dir[j].foffset;
        *sizep = lp->dir[j].fsize;
        return true;
        }
    }
    }
    *lib = (library *) 0;
    *startp = *sizep = 0;
    return false;
}

/*
 * Open the library of the given name and fill in the given library
 * structure.  Return true if successful, false otherwise.
 */
bool open_library(const char *lib_name, library *lp) {
    lp->fdata = fopen_datafile(lib_name, "r", DATAPREFIX);
    if (lp->fdata) {
        if (readlibdir(lp)) {
            return true;
        } else {
            (void) fclose(lp->fdata);
            lp->fdata = (FILE *) 0;
        }
    }
    return false;
}

void
close_library (library *lp)
{
    (void) fclose(lp->fdata);
    free((void *) lp->dir);
    free((void *) lp->sspace);

    (void) memset((char *)lp, 0, sizeof(library));
}

/*
 * Open the library file once using stdio.  Keep it open, but
 * keep track of the file position.
 */
static bool lib_dlb_init(void) {
    /* zero out array */
    (void) memset((char *)&dlb_libs[0], 0, sizeof(dlb_libs));

    /* To open more than one library, add open library calls here. */
    if (!open_library(DLBFILE, &dlb_libs[0])) return false;
    return true;
}

static void lib_dlb_cleanup(void) {
    int i;

    /* close the data file(s) */
    for (i = 0; i < MAX_LIBS && dlb_libs[i].fdata; i++)
    close_library(&dlb_libs[i]);
}

static bool 
lib_dlb_fopen (dlb *dp, const char *name, const char *mode)
{
    long start, size;
    library *lp;

    /* look up file in directory */
    if (find_file(name, &lp, &start, &size)) {
    dp->lib = lp;
    dp->start = start;
    dp->size = size;
    dp->mark = 0;
    return true;
    }

    return false;    /* failed */
}

static int
lib_dlb_fclose (dlb *dp)
{
    /* nothing needs to be done */
    return 0;
}

static int
lib_dlb_fread (char *buf, int size, int quan, dlb *dp)
{
    long pos, nread, nbytes;

    /* make sure we don't read into the next file */
    if ((dp->size - dp->mark) < (size * quan))
    quan = (dp->size - dp->mark) / size;
    if (quan == 0) return 0;

    pos = dp->start + dp->mark;
    if (dp->lib->fmark != pos) {
    fseek(dp->lib->fdata, pos, SEEK_SET);    /* check for error??? */
    dp->lib->fmark = pos;
    }

    nread = fread(buf, size, quan, dp->lib->fdata);
    nbytes = nread * size;
    dp->mark += nbytes;
    dp->lib->fmark += nbytes;

    return nread;
}

static int
lib_dlb_fseek (dlb *dp, long pos, int whence)
{
    long curpos;

    switch (whence) {
    case SEEK_CUR:       curpos = dp->mark + pos;    break;
    case SEEK_END:       curpos = dp->size - pos;    break;
    default: /* set */ curpos = pos;        break;
    }
    if (curpos < 0) curpos = 0;
    if (curpos > dp->size) curpos = dp->size;

    dp->mark = curpos;
    return 0;
}

static char *
lib_dlb_fgets (char *buf, int len, dlb *dp)
{
    int i;
    char *bp, c = 0;

    if (len <= 0) return buf;    /* sanity check */

    /* return NULL on EOF */
    if (dp->mark >= dp->size) return (char *) 0;

    len--;    /* save room for null */
    for (i = 0, bp = buf;
        i < len && dp->mark < dp->size && c != '\n'; i++, bp++) {
    if (dlb_fread(bp, 1, 1, dp) <= 0) break;    /* EOF or error */
    c = *bp;
    }
    *bp = '\0';

    return buf;
}

static int
lib_dlb_fgetc (dlb *dp)
{
    char c;

    if (lib_dlb_fread(&c, 1, 1, dp) != 1) return EOF;
    return (int) c;
}


static long
lib_dlb_ftell (dlb *dp)
{
    return dp->mark;
}

const dlb_procs_t lib_dlb_procs = {
    lib_dlb_init,
    lib_dlb_cleanup,
    lib_dlb_fopen,
    lib_dlb_fclose,
    lib_dlb_fread,
    lib_dlb_fseek,
    lib_dlb_fgets,
    lib_dlb_fgetc,
    lib_dlb_ftell
};

/* Global wrapper functions ------------------------------------------------ */

#define do_dlb_init (*dlb_procs->dlb_init_proc)
#define do_dlb_cleanup (*dlb_procs->dlb_cleanup_proc)
#define do_dlb_fopen (*dlb_procs->dlb_fopen_proc)
#define do_dlb_fclose (*dlb_procs->dlb_fclose_proc)
#define do_dlb_fread (*dlb_procs->dlb_fread_proc)
#define do_dlb_fseek (*dlb_procs->dlb_fseek_proc)
#define do_dlb_fgets (*dlb_procs->dlb_fgets_proc)
#define do_dlb_fgetc (*dlb_procs->dlb_fgetc_proc)
#define do_dlb_ftell (*dlb_procs->dlb_ftell_proc)

static const dlb_procs_t *dlb_procs;
static bool dlb_initialized = false;

bool dlb_init(void) {
    if (!dlb_initialized) {
    dlb_procs = &lib_dlb_procs;

    if (dlb_procs)
        dlb_initialized = do_dlb_init();
    }

    return dlb_initialized;
}

void dlb_cleanup(void) {
    if (dlb_initialized) {
        do_dlb_cleanup();
        dlb_initialized = false;
    }
}

dlb * dlb_fopen(const char *name, const char *mode) {
    FILE *fp;
    dlb *dp;

    if (!dlb_initialized)
        return NULL;

    dp = (dlb *) malloc(sizeof(dlb));
    if (do_dlb_fopen(dp, name, mode)) {
        dp->fp = NULL;
    } else if ((fp = fopen_datafile(name, mode, DATAPREFIX)) != 0) {
        dp->fp = fp;
    } else {
        /* can't find anything */
        free(dp);
        dp = NULL;
    }

    return dp;
}

int dlb_fclose(dlb *dp) {
    int ret = 0;

    if (dlb_initialized) {
        if (dp->fp) ret = fclose(dp->fp);
        else ret = do_dlb_fclose(dp);

        free((void *) dp);
    }
    return ret;
}

int dlb_fread(char *buf, int size, int quan, dlb *dp) {
    if (!dlb_initialized || size <= 0 || quan <= 0) return 0;
    if (dp->fp) return (int) fread(buf, size, quan, dp->fp);
    return do_dlb_fread(buf, size, quan, dp);
}

int dlb_fseek(dlb *dp, long pos, int whence) {
    if (!dlb_initialized) return EOF;
    if (dp->fp) return fseek(dp->fp, pos, whence);
    return do_dlb_fseek(dp, pos, whence);
}

char * dlb_fgets(char *buf, int len, dlb *dp) {
    if (!dlb_initialized) return (char *) 0;
    if (dp->fp) return fgets(buf, len, dp->fp);
    return do_dlb_fgets(buf, len, dp);
}

int dlb_fgetc(dlb *dp) {
    if (!dlb_initialized) return EOF;
    if (dp->fp) return fgetc(dp->fp);
    return do_dlb_fgetc(dp);
}

long dlb_ftell(dlb *dp) {
    if (!dlb_initialized) return 0;
    if (dp->fp) return ftell(dp->fp);
    return do_dlb_ftell(dp);
}
