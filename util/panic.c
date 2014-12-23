/*
 *	This code was adapted from the code in end.c to run in a standalone
 *	mode for the makedefs / drg code.
 */

#define NEED_VARARGS
#include "config.h"

#ifdef AZTEC
#define abort() exit()
#endif

/*VARARGS1*/
boolean panicking;
void VDECL(panic, (char *,...));

void
panic VA_DECL(char *,str)
	VA_START(str);
	VA_INIT(str, char *);
	if(panicking++)
    abort();    /* avoid loops - this should never happen*/

	(void) fputs(" ERROR:  ", stderr);
	Vfprintf(stderr, str, VA_ARGS);
	(void) fflush(stderr);
    abort();	/* generate core dump */
	VA_END();
	exit(EXIT_FAILURE);		/* redundant */
	return;
}

#ifdef ALLOCA_HACK
/*
 * In case bison-generated foo_yacc.c tries to use alloca(); if we don't
 * have it then just use malloc() instead.  This may not work on some
 * systems, but they should either use yacc or get a real alloca routine.
 */
long *alloca(cnt)
unsigned cnt;
{
	return cnt ? alloc(cnt) : (long *)0;
}
#endif

/*panic.c*/
