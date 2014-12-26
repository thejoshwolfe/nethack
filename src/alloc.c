/* See LICENSE in the root of this project for change info */

#include "global.h"
#include <stdlib.h>

char *fmt_ptr(const void *,char *);

long *alloc(unsigned int);
extern void panic(const char *,...);


long * alloc(unsigned int lth) {
    void * ptr;

    ptr = malloc(lth);
    if (!ptr) panic("Memory allocation failure; cannot get %u bytes", lth);
    return((long *) ptr);
}

/* format a pointer for display purposes; caller supplies the result buffer */
char * fmt_ptr (const void *ptr, char *buf) {
    Sprintf(buf, "%06lx", (unsigned long)ptr);
    return buf;
}
