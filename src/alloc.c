/* See LICENSE in the root of this project for change info */

#include "global.h"
#include <stdlib.h>

char *fmt_ptr(const void *,char *);

long *alloc(unsigned int);
extern void panic(const char *,...);


long * alloc(unsigned int lth) {
        void * ptr;

        ptr = malloc(lth);
#ifndef MONITOR_HEAP
        if (!ptr) panic("Memory allocation failure; cannot get %u bytes", lth);
#endif
        return((long *) ptr);
}



#define PTR_FMT "%06lx"
#define PTR_TYP unsigned long

/* format a pointer for display purposes; caller supplies the result buffer */
char *
fmt_ptr (const void *ptr, char *buf)
{
        Sprintf(buf, PTR_FMT, (PTR_TYP)ptr);
        return buf;
}
