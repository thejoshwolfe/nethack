/* See LICENSE in the root of this project for change info */
/*      attrib.h - Header file for character class processing. */

#ifndef ATTRIB_H
#define ATTRIB_H

#define A_STR   0
#define A_INT   1
#define A_WIS   2
#define A_DEX   3
#define A_CON   4
#define A_CHA   5

#define A_MAX   6       /* used in rn2() selection of attrib */

#define ABASE(x)        (u.acurr.a[x])
#define ABON(x)         (u.abon.a[x])
#define AEXE(x)         (u.aexe.a[x])
#define ACURR(x)        (acurr(x))
#define ACURRSTR        (acurrstr())
/* should be: */
/* #define ACURR(x) (ABON(x) + ATEMP(x) + (Upolyd  ? MBASE(x) : ABASE(x)) */
#define MCURR(x)        (u.macurr.a[x])
#define AMAX(x)         (u.amax.a[x])
#define MMAX(x)         (u.mamax.a[x])

#define ATEMP(x)        (u.atemp.a[x])
#define ATIME(x)        (u.atime.a[x])

/* KMH -- Conveniences when dealing with strength constants */
#define STR18(x)        (18+(x))        /* 18/xx */
#define STR19(x)        (100+(x))       /* For 19 and above */

struct  attribs {
        signed char     a[A_MAX];
};

#define ATTRMAX(x) ((x == A_STR && Upolyd && strongmonst(youmonst.data)) ? STR18(100) : urace.attrmax[x])
#define ATTRMIN(x) (urace.attrmin[x])

bool adjattrib(int,int,int);
void change_luck(signed char);
int stone_luck(bool);
void set_moreluck(void);
void gainstr(struct obj *,int);
void losestr(int);
void restore_attrib(void);
void exercise(int,bool);
void exerchk(void);
void reset_attribute_clock(void);
void init_attr(int);
void redist_attr(void);
void adjabil(int,int);
int newhp(void);
signed char acurr(int);
signed char acurrstr(void);
void adjalign(int);

#endif // ATTRIB_H
