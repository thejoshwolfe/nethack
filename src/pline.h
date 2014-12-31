#ifndef PLINE_H
#define PLINE_H

#include "align.h"

void pline(const char *,...) __attribute__ ((format (printf, 1, 2)));
void plines(const char *);
void Norep(const char *,...) __attribute__ ((format (printf, 1, 2)));
void free_youbuf(void);
void You(const char *,...) __attribute__ ((format (printf, 1, 2)));
void Your(const char *,...) __attribute__ ((format (printf, 1, 2)));
void You_feel(const char *,...) __attribute__ ((format (printf, 1, 2)));
void You_cant(const char *,...) __attribute__ ((format (printf, 1, 2)));
void You_hear(const char *,...) __attribute__ ((format (printf, 1, 2)));
void pline_The(const char *,...) __attribute__ ((format (printf, 1, 2)));
void There(const char *,...) __attribute__ ((format (printf, 1, 2)));
void verbalize(const char *,...) __attribute__ ((format (printf, 1, 2)));
void raw_printf(const char *,...) __attribute__ ((format (printf, 1, 2)));
void impossible(const char *,...) __attribute__ ((format (printf, 1, 2)));
const char *align_str(aligntyp);
void mstatusline(struct monst *);
void ustatusline(void);
void self_invis_message(void);

#endif // PLINE_H
