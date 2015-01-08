/* See LICENSE in the root of this project for change info */
#ifndef SPELL_H
#define SPELL_H

#include <stdbool.h>

struct spell {
    short       sp_id;                  /* spell id (== object.otyp) */
    signed char sp_lev;                 /* power level */
    int         sp_know;                /* knowlege of spell */
};

/* levels of memory destruction with a scroll of amnesia */
#define ALL_MAP         0x1
#define ALL_SPELLS      0x2

#define decrnknow(spell)        spl_book[spell].sp_know--
#define spellid(spell)          spl_book[spell].sp_id
#define spellknow(spell)        spl_book[spell].sp_know

int study_book(struct obj *);
void book_disappears(struct obj *);
void book_substitution(struct obj *,struct obj *);
void age_spells(void);
int docast(void);
int spell_skilltype(int);
int spelleffects(int,bool);
void losespells(void);
int dovspell(void);
void initialspell(struct obj *);


#endif /* SPELL_H */
