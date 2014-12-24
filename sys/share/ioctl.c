/* This cannot be part of hack.tty.c (as it was earlier) since on some
   systems (e.g. MUNIX) the include files <termio.h> and <sgtty.h>
   define the same constants, and the C preprocessor complains. */

#include "hack.h"

#if defined(BSD_JOB_CONTROL) || defined(_BULL_SOURCE)
# ifdef HPUX
#include <bsdtty.h>
# else
#  if defined(AIX_31) && !defined(_ALL_SOURCE)
#   define _ALL_SOURCE	/* causes struct winsize to be present */
#   ifdef _AIX32
#    include <sys/ioctl.h>
#   endif
#  endif
#  if defined(_BULL_SOURCE)
#   include <termios.h>
struct termios termio;
#   undef TIMEOUT		/* defined in you.h and sys/tty.h */
#   include <sys/tty.h>		/* define winsize */
#   include <sys/ttold.h>	/* define struct ltchars */
#   include <sys/bsdioctl.h>	/* define TIOGWINSZ */
#  else
#   include <sgtty.h>
#  endif
# endif
struct ltchars ltchars;
struct ltchars ltchars0 = { -1, -1, -1, -1, -1, -1 }; /* turn all off */
#else

#include <termios.h>
struct termios termio;
#  if defined(BSD) || defined(_AIX32)
#   if defined(_AIX32) && !defined(_ALL_SOURCE)
#    define _ALL_SOURCE
#   endif
#include <sys/ioctl.h>
#  endif
# ifdef AMIX
#include <sys/ioctl.h>
# endif /* AMIX */
#endif

#if defined(TIOCGWINSZ) && (defined(BSD) || defined(ULTRIX) || defined(AIX_31) || defined(_BULL_SOURCE) || defined(SVR4))
#define USE_WIN_IOCTL
#include "tcap.h"	/* for LI and CO */
#endif

#ifdef _M_UNIX
extern void NDECL(sco_mapon);
extern void NDECL(sco_mapoff);
#endif
#ifdef __linux__
extern void NDECL(linux_mapon);
extern void NDECL(linux_mapoff);
#endif

void getwindowsz(void) {
#ifdef USE_WIN_IOCTL
    /*
     * ttysize is found on Suns and BSD
     * winsize is found on Suns, BSD, and Ultrix
     */
    struct winsize ttsz;

    if (ioctl(fileno(stdin), (int)TIOCGWINSZ, (char *)&ttsz) != -1) {
	/*
	 * Use the kernel's values for lines and columns if it has
	 * any idea.
	 */
	if (ttsz.ws_row)
	    LI = ttsz.ws_row;
	if (ttsz.ws_col)
	    CO = ttsz.ws_col;
    }
#endif
}

void
getioctls()
{
#ifdef BSD_JOB_CONTROL
	(void) ioctl(fileno(stdin), (int) TIOCGLTC, (char *) &ltchars);
	(void) ioctl(fileno(stdin), (int) TIOCSLTC, (char *) &ltchars0);
#else
	(void) tcgetattr(fileno(stdin), &termio);
#endif
	getwindowsz();
}

void setioctls(void) {
#ifdef BSD_JOB_CONTROL
	(void) ioctl(fileno(stdin), (int) TIOCSLTC, (char *) &ltchars);
#else
	tcsetattr(fileno(stdin), TCSADRAIN, &termio);
#endif
}
