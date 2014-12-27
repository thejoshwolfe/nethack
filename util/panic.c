/* See LICENSE in the root of this project for change info */
/*
 *      This code was adapted from the code in end.c to run in a standalone
 *      mode for the makedefs / drg code.
 */

#include <stdarg.h>
#include "global.h"

/*VARARGS1*/
bool panicking;
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
