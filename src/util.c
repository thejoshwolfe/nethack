#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>


/*
MSG_FAILED_POLYMORPH, "%(monster_subject) shudders!"

MSG_WELDS_TO_YOUR_HAND:
                const char *tmp = xname(wep), *thestr = "The ";
                if (strncmp(tmp, thestr, 4) && !strncmp(The(tmp),thestr,4))
                    tmp = thestr;
                else
                    tmp = "";


                pline("%s%s %s to your %s!", tmp, aobjnam(wep, "weld"),
                        (wep->quan == 1L) ? "itself" : "themselves",
                        bimanual(wep) ?
                                (const char *)makeplural(body_part(HAND))
                                : body_part(HAND));

*/

size_t nh_strlcpy(char *dest, const char *source, size_t dest_size) {
    char *d = dest;
    const char *s = source;
    size_t n = dest_size;

    // copy as many bytes as will fit
    if (n != 0 && --n != 0) {
        do {
            if ((*d++ = *s++) == 0)
                break;
        } while (--n != 0);
    }

    // not enough room in dest, add \0 and traverse rest of source
    if (n == 0) {
        if (dest_size != 0)
            *d = '\0';
        while (*s++)
            continue;
    }

    // count does not include \0
    return s - source - 1;
}

size_t nh_vslprintf(char *dest, size_t dest_size, char *format, va_list ap) {
    size_t n = (dest_size >= 1) ? (dest_size - 1) : 0;
    int ret = vsnprintf(dest, n, format, ap);
    assert(ret >= 0);
    dest[(ret > n) ? n : ret] = 0;
    return ret;
}

size_t nh_slprintf(char *dest, size_t dest_size, char *format, ...) {
    va_list ap;  
    va_start(ap, format);
    int ret = nh_vslprintf(dest, dest_size, format, ap);
    va_end(ap);
    return ret;
}

/*

MSG_YOU_DISRUPT, You("disrupt %s!", name);
MSG_A_HUGE_HOLE_OPENS_UP, "A huge hole opens up..."

*/
