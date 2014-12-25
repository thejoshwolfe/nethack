/* See LICENSE in the root of this project for change info */
#define ALLOC_C         /* comment line for pre-compiled headers */
/* since this file is also used in auxiliary programs, don't include all the
 * function declarations for all of nethack
 */
#define EXTERN_H        /* comment line for pre-compiled headers */
#include "config.h"

#if defined(MONITOR_HEAP) || defined(WIZARD)
char *fmt_ptr(const void *,char *);
#endif

#ifdef MONITOR_HEAP
#undef alloc
#undef free
extern void free(void *);
static void heapmon_init(void);

static FILE *heaplog = 0;
static boolean tried_heaplog = FALSE;
#endif

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


#if defined(MONITOR_HEAP) || defined(WIZARD)

# ifdef MONITOR_PTR_FMT
#  define PTR_FMT "%p"
#  define PTR_TYP void *
# else
#  define PTR_FMT "%06lx"
#  define PTR_TYP unsigned long
# endif

/* format a pointer for display purposes; caller supplies the result buffer */
char *
fmt_ptr (const void *ptr, char *buf)
{
        Sprintf(buf, PTR_FMT, (PTR_TYP)ptr);
        return buf;
}

#endif

#ifdef MONITOR_HEAP

/* If ${NH_HEAPLOG} is defined and we can create a file by that name,
   then we'll log the allocation and release information to that file. */
static void 
heapmon_init (void)
{
        char *logname = getenv("NH_HEAPLOG");

        if (logname && *logname)
                heaplog = fopen(logname, "w");
        tried_heaplog = TRUE;
}

long *
nhalloc (unsigned int lth, const char *file, int line)
{
        long *ptr = alloc(lth);
        char ptr_address[20];

        if (!tried_heaplog) heapmon_init();
        if (heaplog)
                (void) fprintf(heaplog, "+%5u %s %4d %s\n", lth,
                                fmt_ptr((void *)ptr, ptr_address),
                                line, file);
        /* potential panic in alloc() was deferred til here */
        if (!ptr) panic("Cannot get %u bytes, line %d of %s",
                        lth, line, file);

        return ptr;
}

void 
nhfree (void *ptr, const char *file, int line)
{
        char ptr_address[20];

        if (!tried_heaplog) heapmon_init();
        if (heaplog)
                (void) fprintf(heaplog, "-      %s %4d %s\n",
                                fmt_ptr((void *)ptr, ptr_address),
                                line, file);

        free(ptr);
}

#endif /* MONITOR_HEAP */
