/* See LICENSE in the root of this project for change info */
/*
 *      This code was adapted from the code in end.c to run in a standalone
 *      mode for the makedefs / drg code.
 */

#include <stdarg.h>
#include "global.h"

#ifdef AZTEC
#define abort() exit()
#endif

/*VARARGS1*/
boolean panicking;
void panic (char * str, ...) {
    va_list the_args;
        va_start(the_args, str);
        if(panicking++)
    abort();    /* avoid loops - this should never happen*/

        (void) fputs(" ERROR:  ", stderr);
        Vfprintf(stderr, str, the_args);
        (void) fflush(stderr);
    abort();    /* generate core dump */
    va_end(the_args);
        exit(EXIT_FAILURE);             /* redundant */
        return;
}

#ifdef ALLOCA_HACK
/*
 * In case bison-generated foo_yacc.c tries to use alloca(); if we don't
 * have it then just use malloc() instead.  This may not work on some
 * systems, but they should either use yacc or get a real alloca routine.
 */
long *alloca(unsigned cnt) {
        return cnt ? alloc(cnt) : (long *)0;
}
#endif
