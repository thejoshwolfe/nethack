/* See LICENSE in the root of this project for change info */
#ifndef VAULT_H
#define VAULT_H

#define FCSIZ	(ROWNO+COLNO)
struct fakecorridor {
	signed char fx,fy,ftyp;
};

struct egd {
	int fcbeg, fcend;	/* fcend: first unused pos */
	int vroom;		/* room number of the vault */
	signed char gdx, gdy;		/* goal of guard's walk */
	signed char ogx, ogy;		/* guard's last position */
	d_level gdlevel;	/* level (& dungeon) guard was created in */
	signed char warncnt;		/* number of warnings to follow */
	Bitfield(gddone,1);	/* true iff guard has released player */
	Bitfield(unused,7);
	struct fakecorridor fakecorr[FCSIZ];
};

#define EGD(mon)	((struct egd *)&(mon)->mextra[0])

#endif /* VAULT_H */
