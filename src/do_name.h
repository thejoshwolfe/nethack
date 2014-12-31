#ifndef DO_NAME_H
#define DO_NAME_H

#include "coord.h"

#include <stdbool.h>
#include <stdlib.h>

int getpos(coord *, bool, const char *);
struct monst *christen_monst(struct monst *, const char *);
int do_mname(void);
struct obj *oname(struct obj *, const char *);
int ddocall(void);
void docall(struct obj *);
const char *rndghostname(void);
size_t x_monnam(char *out_buf, int buf_size, const struct monst *mtmp, int article,
        const char *adjective, int suppress, bool called);
size_t l_monnam(char *out_buf, size_t buf_size, const struct monst *mtmp);
size_t mon_nam(char *out_buf, size_t buf_size, const struct monst *mtmp);
size_t noit_mon_nam(char *out_buf, size_t buf_size, const struct monst *mtmp);
size_t Monnam(char *out_buf, size_t buf_size, const struct monst *mtmp);
size_t noit_Monnam(char *out_buf, size_t buf_size, const struct monst *mtmp);
size_t m_monnam (char *out_buf, size_t buf_size, const struct monst *mtmp);
size_t y_monnam (char *out_buf, size_t buf_size, const struct monst *mtmp);
size_t Adjmonnam(char *out_buf, size_t buf_size, const struct monst *mtmp, const char *adj);
size_t Amonnam (char *out_buf, size_t buf_size, const struct monst *mtmp);
size_t a_monnam(char *out_buf, size_t buf_size, const struct monst *mtmp);
size_t distant_monnam(char *out_buf, size_t buf_size, const struct monst *mon, int article);
const char *rndmonnam(void);
const char *hcolor(const char *);
int halluc_color_int(void);
const char *rndcolor(void);
struct obj *realloc_obj(struct obj *, int, void *, int, const char *);
size_t coyotename (char *out_buf, size_t buf_size, const struct monst *mtmp);

#endif // DO_NAME_H
