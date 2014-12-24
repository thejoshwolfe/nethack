#ifndef EPRI_H
#define EPRI_H

struct epri {
	aligntyp shralign;	/* alignment of priest's shrine */
				/* leave as first field to match emin */
	signed char shroom;		/* index in rooms */
	coord shrpos;		/* position of shrine */
	d_level shrlevel;	/* level (& dungeon) of shrine */
};

#define EPRI(mon)	((struct epri *)&(mon)->mextra[0])

/* A priest without ispriest is a roaming priest without a shrine, so
 * the fields (except shralign, which becomes only the priest alignment)
 * are available for reuse.
 */
#define renegade shroom

#endif /* EPRI_H */
