/*	Common include file for save and restore routines */

#ifndef LEV_H
#define LEV_H

#define COUNT_SAVE	0x1
#define WRITE_SAVE	0x2
#define FREE_SAVE	0x4

/* operations of the various saveXXXchn & co. routines */
#define perform_bwrite(mode)	((mode) & (COUNT_SAVE|WRITE_SAVE))
#define release_data(mode)	((mode) & FREE_SAVE)

/* The following are used in mkmaze.c */
struct container {
	struct container *next;
	signed char x, y;
	short what;
	void * list;
};

#define CONS_OBJ   0
#define CONS_MON   1
#define CONS_HERO  2
#define CONS_TRAP  3

struct bubble {
	signed char x, y;	/* coordinates of the upper left corner */
	signed char dx, dy;	/* the general direction of the bubble's movement */
	unsigned char *bm;	/* pointer to the bubble bit mask */
	struct bubble *prev, *next; /* need to traverse the list up and down */
	struct container *cons;
};

/* used in light.c */
typedef struct ls_t {
    struct ls_t *next;
    signed char x, y;		/* source's position */
    short range;	/* source's current range */
    short flags;
    short type;		/* type of light source */
    void * id;	/* source's identifier */
} light_source;

#endif /* LEV_H */
