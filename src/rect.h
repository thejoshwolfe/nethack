/* See LICENSE in the root of this project for change info */
#ifndef RECT_H
#define RECT_H

typedef struct nhrect {
        signed char lx, ly;
        signed char hx, hy;
} NhRect;

void init_rect(void);
NhRect *get_rect(NhRect *);
NhRect *rnd_rect(void);
void remove_rect(NhRect *);
void add_rect(NhRect *);
void split_rects(NhRect *,NhRect *);

#endif /* RECT_H */
