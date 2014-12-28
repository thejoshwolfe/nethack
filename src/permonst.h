/* See LICENSE in the root of this project for change info */
#ifndef PERMONST_H
#define PERMONST_H

#include "align.h"
#include "monattk.h"
#include "monflag.h"

/*      This structure covers all attack forms.
 *      aatyp is the gross attack type (eg. claw, bite, breath, ...)
 *      adtyp is the damage type (eg. physical, fire, cold, spell, ...)
 *      damn is the number of hit dice of damage from the attack.
 *      damd is the number of sides on each die.
 *
 *      Some attacks can do no points of damage.  Additionally, some can
 *      have special effects *and* do damage as well.  If damn and damd
 *      are set, they may have a special meaning.  For example, if set
 *      for a blinding attack, they determine the amount of time blinded.
 */

struct attack {
        unsigned char           aatyp;
        unsigned char           adtyp, damn, damd;
};

/*      Max # of attacks for any given monster.
 */

#define NATTK           6

/*      Weight of a human body
 */

#define WT_HUMAN        1450

struct permonst {
    const char      *mname;                 /* full name */
    char            mlet;                   /* symbol */
    signed char     mlevel,                 /* base monster level */
                    mmove,                  /* move speed */
                    ac,                     /* (base) armor class */
                    mr;                     /* (base) magic resistance */
    aligntyp        maligntyp;              /* basic monster alignment */
    unsigned short  geno;                   /* creation/geno mask value */
    struct  attack  mattk[NATTK];           /* attacks matrix */
    unsigned short  cwt,                    /* weight of corpse */
                    cnutrit;                /* its nutritional value */
    short           pxlth;                  /* length of extension */
    unsigned char   msound;                 /* noise it makes (6 bits) */
    unsigned char   msize;                  /* physical size (3 bits) */
    unsigned char   mresists;               /* resistances */
    unsigned char   mconveys;               /* conveyed by eating */
    unsigned long   mflags1,                /* bool bitflags */
                    mflags2;                /* more bool bitflags */
    unsigned short  mflags3;                /* yet more bool bitflags */
    unsigned char   mcolor;                 /* color to use */
};

/* the master list of monster types */
extern struct permonst mons[];

#define VERY_SLOW 3
#define SLOW_SPEED 9
#define NORMAL_SPEED 12 /* movement rates */
#define FAST_SPEED 15
#define VERY_FAST 24

#endif /* PERMONST_H */
