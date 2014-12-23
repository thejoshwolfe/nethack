/* This file collects some Unix dependencies; pager.c contains some more */

#include "hack.h"
#include "wintty.h"

#include	<sys/stat.h>

#ifdef OVLB

# ifdef WANT_GETHDATE
static struct stat hbuf;
# endif

#ifdef PC_LOCKING
static int NDECL(eraseoldlocks);
#endif


#ifdef PC_LOCKING
static int
eraseoldlocks()
{
	register int i;

	/* cannot use maxledgerno() here, because we need to find a lock name
	 * before starting everything (including the dungeon initialization
	 * that sets astral_level, needed for maxledgerno()) up
	 */
	for(i = 1; i <= MAXDUNGEON*MAXLEVEL + 1; i++) {
		/* try to remove all */
		set_levelfile_name(lock, i);
		(void) unlink(fqname(lock, LEVELPREFIX, 0));
	}
	set_levelfile_name(lock, 0);
#ifdef HOLD_LOCKFILE_OPEN
	really_close();
#endif
	if(unlink(fqname(lock, LEVELPREFIX, 0)))
		return 0;				/* cannot remove it */
	return(1);					/* success! */
}

void
getlock()
{
	register int fd, c, ci, ct, ern;
	char tbuf[BUFSZ];
	const char *fq_lock;
# if defined(MSDOS) && defined(NO_TERMS)
	int grmode = iflags.grmode;
# endif
	/* we ignore QUIT and INT at this point */
	if (!lock_file(HLOCK, LOCKPREFIX, 10)) {
		wait_synch();
		error("Quitting.");
	}

	/* regularize(lock); */ /* already done in pcmain */
	Sprintf(tbuf,"%s",fqname(lock, LEVELPREFIX, 0));
	set_levelfile_name(lock, 0);
	fq_lock = fqname(lock, LEVELPREFIX, 1);
	if((fd = open(fq_lock,0)) == -1) {
		if(errno == ENOENT) goto gotlock;    /* no such file */
# if defined(HOLD_LOCKFILE_OPEN)
#  if defined(HOLD_LOCKFILE_OPEN)
 		if(errno == EACCES) {
#define OOPS_BUFSZ 512
 		    char oops[OOPS_BUFSZ];
 		    Strcpy(oops,
			     "\nThere are files from a game in progress under your name.");
		    Strcat(oops, "\nThe files are locked or inaccessible.");
		    Strcat(oops, " Is the other game still running?\n");
		    if (strlen(fq_lock) < ((OOPS_BUFSZ -16) - strlen(oops)))
			    Sprintf(eos(oops), "Cannot open %s", fq_lock);
		    Strcat(oops, "\n");
		    unlock_file(HLOCK);
		    error(oops);
 		} else
#  endif
		error("Bad directory or name: %s\n%s\n",
				fq_lock, strerror(errno));
# else
		perror(fq_lock);
# endif
		unlock_file(HLOCK); 
		error("Cannot open %s", fq_lock);
	}

	(void) close(fd);

	if(iflags.window_inited) { 
# ifdef SELF_RECOVER
	  c = yn("There are files from a game in progress under your name. Recover?");
# else
	  pline("There is already a game in progress under your name.");
	  pline("You may be able to use \"recover %s\" to get it back.\n",tbuf);
	  c = yn("Do you want to destroy the old game?");
# endif
	} else {
# if defined(MSDOS) && defined(NO_TERMS)
		grmode = iflags.grmode;
		if (grmode) gr_finish();
# endif
		c = 'n';
		ct = 0;
# ifdef SELF_RECOVER
		msmsg(
		"There are files from a game in progress under your name. Recover? [yn]");
# else
		msmsg("\nThere is already a game in progress under your name.\n");
		msmsg("If this is unexpected, you may be able to use \n");
		msmsg("\"recover %s\" to get it back.",tbuf);
		msmsg("\nDo you want to destroy the old game? [yn] ");
# endif
		while ((ci=nhgetch()) != '\n') {
		    if (ct > 0) {
			msmsg("\b \b");
			ct = 0;
			c = 'n';
		    }
		    if (ci == 'y' || ci == 'n' || ci == 'Y' || ci == 'N') {
		    	ct = 1;
		        c = ci;
		        msmsg("%c",c);
		    }
		}
	}
	if(c == 'y' || c == 'Y')
# ifndef SELF_RECOVER
		if(eraseoldlocks()) {
			goto gotlock;
		} else {
			unlock_file(HLOCK);
			error("Couldn't destroy old game.");
		}
# else /*SELF_RECOVER*/
		if(recover_savefile()) {
			goto gotlock;
		} else {
			unlock_file(HLOCK);
			error("Couldn't recover old game.");
		}
# endif /*SELF_RECOVER*/
	else {
		unlock_file(HLOCK);
		error("%s", "Cannot start a new game.");
	}

gotlock:
	fd = creat(fq_lock, FCMASK);
	if (fd == -1) ern = errno;
	unlock_file(HLOCK);
	if(fd == -1) {
		error("cannot creat file (%s.)", fq_lock);
	} else {
		if(write(fd, (char *) &hackpid, sizeof(hackpid))
		    != sizeof(hackpid)){
			error("cannot write lock (%s)", fq_lock);
		}
		if(close(fd) == -1) {
			error("cannot close lock (%s)", fq_lock);
		}
	}
# if defined(MSDOS) && defined(NO_TERMS)
	if (grmode) gr_init();
# endif
}	
#endif /* PC_LOCKING */

void regularize(char *s) {
/*
 * normalize file name - we don't like .'s, /'s, spaces, and
 * lots of other things
 */
	register char *lp;

	for (lp = s; *lp; lp++)
		if (*lp <= ' ' || *lp == '"' || (*lp >= '*' && *lp <= ',') ||
		    *lp == '.' || *lp == '/' || (*lp >= ':' && *lp <= '?') ||
# ifdef OS2
		    *lp == '&' || *lp == '(' || *lp == ')' ||
# endif
		    *lp == '|' || *lp >= 127 || (*lp >= '[' && *lp <= ']'))
                        *lp = '_';
}
#endif /* OVLB */


#ifdef __EMX__
void seteuid(int i){;}
#endif
