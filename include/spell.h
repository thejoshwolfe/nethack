/* See LICENSE in the root of this project for change info */
#ifndef SPELL_H
#define SPELL_H

struct spell {
    short	sp_id;			/* spell id (== object.otyp) */
    signed char	sp_lev;			/* power level */
    int		sp_know;		/* knowlege of spell */
};

/* levels of memory destruction with a scroll of amnesia */
#define ALL_MAP		0x1
#define ALL_SPELLS	0x2

#define decrnknow(spell)	spl_book[spell].sp_know--
#define spellid(spell)		spl_book[spell].sp_id
#define spellknow(spell)	spl_book[spell].sp_know

#endif /* SPELL_H */
