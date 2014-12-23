/*
 *  Utility for reconstructing NetHack save file from a set of individual
 *  level files.  Requires that the `checkpoint' option be enabled at the
 *  time NetHack creates those level files.
 */
#include "config.h"
#if !defined(O_WRONLY) && !defined(LSC) && !defined(AZTEC_C)
#include <fcntl.h>
#endif

#ifdef SECURE
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#ifdef VMS
extern int FDECL(vms_creat, (const char *,unsigned));
extern int FDECL(vms_open, (const char *,int,unsigned));
#endif	/* VMS */

int FDECL(restore_savefile, (char *));
void FDECL(set_levelfile_name, (int));
int FDECL(open_levelfile, (int));
int NDECL(create_savefile);
void FDECL(copy_bytes, (int,int));

#ifndef WIN_CE
#define Fprintf	(void)fprintf
#else
#define Fprintf	(void)nhce_message
static void nhce_message(FILE*, const char*, ...);
#endif
#define Perror  (void)perror
#define Close	(void)close

#define SAVESIZE	(PL_NSIZ + 13)	/* save/99999player.e */

#if defined(EXEPATH)
char *FDECL(exepath, (char *));
#endif

char savename[SAVESIZE]; /* holds relative path of save file from playground */

const char *dir = (char*)0;

int
main(argc, argv)
int argc;
char *argv[];
{
	int argno;


	if (!dir) dir = getenv("NETHACKDIR");
	if (!dir) dir = getenv("HACKDIR");
#if defined(EXEPATH)
	if (!dir) dir = exepath(argv[0]);
#endif
	if (argc == 1 || (argc == 2 && !strcmp(argv[1], "-"))) {
	    Fprintf(stderr,
		"Usage: %s [ -d directory ] base1 [ base2 ... ]\n", argv[0]);
	    exit(EXIT_FAILURE);
	}

	argno = 1;
	if (!strncmp(argv[argno], "-d", 2)) {
		dir = argv[argno]+2;
		if (*dir == '=' || *dir == ':') dir++;
		if (!*dir && argc > argno) {
			argno++;
			dir = argv[argno];
		}
		if (!*dir) {
		    Fprintf(stderr,
			"%s: flag -d must be followed by a directory name.\n",
			argv[0]);
		    exit(EXIT_FAILURE);
		}
		argno++;
	}
#if defined(SECURE) && !defined(VMS)
	if (dir
# ifdef VAR_PLAYGROUND
	        && strcmp(dir, VAR_PLAYGROUND)
# else
#  ifdef HACKDIR
		&& strcmp(dir, HACKDIR)
#  endif 
# endif /* VAR_PLAYGROUND */
		) {
		(void) setgid(getgid());
		(void) setuid(getuid());
	}
#endif	/* SECURE && !VMS */

#ifdef VAR_PLAYGROUND
	if (!dir) dir = VAR_PLAYGROUND;
#endif

#ifdef HACKDIR
	if (!dir) dir = HACKDIR;
#endif

	if (dir && chdir((char *) dir) < 0) {
		Fprintf(stderr, "%s: cannot chdir:", argv[0]);
                Perror(dir);
		exit(EXIT_FAILURE);
	}

	while (argc > argno) {
		if (restore_savefile(argv[argno]) == 0)
		    Fprintf(stderr, "recovered \"%s\" to %s\n",
			    argv[argno], savename);
		argno++;
	}
	exit(EXIT_SUCCESS);
	/*NOTREACHED*/
	return 0;
}

static char lock[256];

void
set_levelfile_name(lev)
int lev;
{
	char *tf;

	tf = rindex(lock, '.');
	if (!tf) tf = lock + strlen(lock);
	(void) sprintf(tf, ".%d", lev);
#ifdef VMS
	(void) strcat(tf, ";1");
#endif
}

#ifdef SECURE
static uid_t save_uid = -1;
#endif

int
open_levelfile(lev)
int lev;
{
	int fd;
#ifdef SECURE
	struct stat level_stat;
	uid_t uid;
#endif

	set_levelfile_name(lev);
	fd = open(lock, O_RDONLY, 0);
	/* Security check: does the user calling recover own the file? */
#ifdef SECURE
	if (fd != -1) {
		uid = getuid();
		if (fstat(fd, &level_stat) == -1) {
			Fprintf(stderr, "No permission to stat level file %s.\n", lock);
			return -1;
		}
		if (uid != 0 && level_stat.st_uid != uid) {
			Fprintf(stderr, "You are not the owner of level file %s.\n", lock);
			return -1;
		}
		save_uid = level_stat.st_uid;
	}
#endif
	return fd;
}

int
create_savefile()
{
	int fd;

	fd = creat(savename, FCMASK);

#ifdef SECURE
	if (fchown(fd, save_uid, -1) == -1) {
		Fprintf(stderr, "could not chown %s to %i!\n", savename, 
			save_uid);
	}
#endif
	return fd;
}

void
copy_bytes(ifd, ofd)
int ifd, ofd;
{
	char buf[BUFSIZ];
	int nfrom, nto;

	do {
		nfrom = read(ifd, buf, BUFSIZ);
		nto = write(ofd, buf, nfrom);
		if (nto != nfrom) {
			Fprintf(stderr, "file copy failed!\n");
			exit(EXIT_FAILURE);
		}
	} while (nfrom == BUFSIZ);
}

int
restore_savefile(basename)
char *basename;
{
	int gfd, lfd, sfd;
	int lev, savelev, hpid;
	xchar levc;
	struct version_info version_data;

	/* level 0 file contains:
	 *	pid of creating process (ignored here)
	 *	level number for current level of save file
	 *	name of save file nethack would have created
	 *	and game state
	 */
	(void) strcpy(lock, basename);
	gfd = open_levelfile(0);
	if (gfd < 0) {
	    Fprintf(stderr, "Cannot open level 0 for %s in directory %s: ",
                basename, dir);
            Perror(lock);
	    return(-1);
	}
	if (read(gfd, (void *) &hpid, sizeof hpid) != sizeof hpid) {
	    Fprintf(stderr, "%s\n%s%s%s\n",
	     "Checkpoint data incompletely written or subsequently clobbered;",
		    "recovery for \"", basename, "\" impossible.");
	    Close(gfd);
	    return(-1);
	}
	if (read(gfd, (void *) &savelev, sizeof(savelev))
							!= sizeof(savelev)) {
	    Fprintf(stderr,
	    "Checkpointing was not in effect for %s -- recovery impossible.\n",
		    basename);
	    Close(gfd);
	    return(-1);
	}
	if ((read(gfd, (void *) savename, sizeof savename)
		!= sizeof savename) ||
	    (read(gfd, (void *) &version_data, sizeof version_data)
		!= sizeof version_data)) {
	    Fprintf(stderr, "Error reading, can't recover: ");
            Perror(lock);
	    Close(gfd);
	    return(-1);
	}

	/* save file should contain:
	 *	version info
	 *	current level (including pets)
	 *	(non-level-based) game state
	 *	other levels
	 */
	sfd = create_savefile();
	if (sfd < 0) {
	    Fprintf(stderr, "Cannot create savefile in %s: ", dir);
            Perror(savename);
	    Close(gfd);
	    return(-1);
	}

	lfd = open_levelfile(savelev);
	if (lfd < 0) {
	    Fprintf(stderr, "Cannot open level of save for %s: ", basename);
            Perror(lock);
	    Close(gfd);
	    Close(sfd);
	    return(-1);
	}

	if (write(sfd, (void *) &version_data, sizeof version_data)
		!= sizeof version_data) {
	    Fprintf(stderr, "Error writing, recovery failed: ");
            Perror(savename);
	    Close(gfd);
	    Close(sfd);
	    return(-1);
	}

	copy_bytes(lfd, sfd);
	Close(lfd);
	(void) unlink(lock);

	copy_bytes(gfd, sfd);
	Close(gfd);
	set_levelfile_name(0);
	(void) unlink(lock);

	for (lev = 1; lev < 256; lev++) {
		/* level numbers are kept in xchars in save.c, so the
		 * maximum level number (for the endlevel) must be < 256
		 */
		if (lev != savelev) {
			lfd = open_levelfile(lev);
			if (lfd >= 0) {
				/* any or all of these may not exist */
				levc = (xchar) lev;
				write(sfd, (void *) &levc, sizeof(levc));
				copy_bytes(lfd, sfd);
				Close(lfd);
				(void) unlink(lock);
			}
		}
	}

	Close(sfd);

	return(0);
}

#ifdef EXEPATH
# ifdef __DJGPP__
#define PATH_SEPARATOR '/'
# else
#define PATH_SEPARATOR '\\'
# endif

#define EXEPATHBUFSZ 256
char exepathbuf[EXEPATHBUFSZ];

char *exepath(str)
char *str;
{
	char *tmp, *tmp2;
	int bsize;

	if (!str) return (char *)0;
	bsize = EXEPATHBUFSZ;
	tmp = exepathbuf;
	strcpy (tmp, str);
	tmp2 = strrchr(tmp, PATH_SEPARATOR);
	if (tmp2) *tmp2 = '\0';
	return tmp;
}
#endif /* EXEPATH */
