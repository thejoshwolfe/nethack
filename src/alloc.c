/* See LICENSE in the root of this project for change info */

#include "global.h"
#include <stdlib.h>

long *alloc(unsigned int);
extern void panic(const char *,...);


long * alloc(unsigned int lth) {
    void * ptr;

    ptr = malloc(lth);
    if (!ptr) panic("Memory allocation failure; cannot get %u bytes", lth);
    return((long *) ptr);
}

