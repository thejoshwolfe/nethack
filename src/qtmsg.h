
#ifndef QTMSG_H
#define QTMSG_H

#define N_HDR   16              /* Maximum number of categories */
                                /* (i.e., num roles + 1) */
#define LEN_HDR 3               /* Maximum length of a category name */

struct qtmsg {
        int     msgnum;
        char    delivery;
        long    offset,
                size;
};

#endif
