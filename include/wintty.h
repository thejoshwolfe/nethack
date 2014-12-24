/* See LICENSE in the root of this project for change info */
#ifndef WINTTY_H
#define WINTTY_H

#ifndef WINDOW_STRUCTS
#define WINDOW_STRUCTS

/* menu structure */
typedef struct tty_mi {
    struct tty_mi *next;
    anything identifier;	/* user identifier */
    long count;			/* user count */
    char *str;			/* description string (including accelerator) */
    int attr;			/* string attribute */
    boolean selected;		/* TRUE if selected by user */
    char selector;		/* keyboard accelerator */
    char gselector;		/* group accelerator */
} tty_menu_item;

/* descriptor for tty-based windows */
struct WinDesc {
    int flags;			/* window flags */
    signed char type;			/* type of window */
    boolean active;		/* true if window is active */
    unsigned char offx, offy;		/* offset from topleft of display */
    short rows, cols;		/* dimensions */
    short curx, cury;		/* current cursor position */
    short maxrow, maxcol;	/* the maximum size used -- for MENU wins */
				/* maxcol is also used by WIN_MESSAGE for */
				/* tracking the ^P command */
    short *datlen;		/* allocation size for *data */
    char **data;		/* window data [row][column] */
    char *morestr;		/* string to display instead of default */
    tty_menu_item *mlist;	/* menu information (MENU) */
    tty_menu_item **plist;	/* menu page pointers (MENU) */
    short plist_size;		/* size of allocated plist (MENU) */
    short npages;		/* number of pages in menu (MENU) */
    short nitems;		/* total number of items (MENU) */
    short how;			/* menu mode - pick 1 or N (MENU) */
    char menu_ch;		/* menu char (MENU) */
};

/* window flags */
#define WIN_CANCELLED 1
#define WIN_STOP 1		/* for NHW_MESSAGE; stops output */

/* descriptor for tty-based displays -- all the per-display data */
struct DisplayDesc {
    unsigned char rows, cols;		/* width and height of tty display */
    unsigned char curx, cury;		/* current cursor position on the screen */
    int color;			/* current color */
    int attrs;			/* attributes in effect */
    int toplin;			/* flag for topl stuff */
    int rawprint;		/* number of raw_printed lines since synch */
    int inmore;			/* non-zero if more() is active */
    int inread;			/* non-zero if reading a character */
    int intr;			/* non-zero if inread was interrupted */
    winid lastwin;		/* last window used for I/O */
    char dismiss_more;		/* extra character accepted at --More-- */
};

#endif /* WINDOW_STRUCTS */

#define MAXWIN 20		/* maximum number of windows, cop-out */

/* tty dependent window types */
#ifdef NHW_BASE
#undef NHW_BASE
#endif
#define NHW_BASE    6

extern struct window_procs tty_procs;

/* port specific variable declarations */
extern winid BASE_WINDOW;

extern struct WinDesc *wins[MAXWIN];

extern struct DisplayDesc *ttyDisplay;	/* the tty display descriptor */

extern char morc;		/* last character typed to xwaitforspace */
extern char defmorestr[];	/* default --more-- prompt */

/* port specific external function references */

/* ### getline.c ### */
extern void FDECL(xwaitforspace, (const char *));

/* ### termcap.c, video.c ### */

extern void FDECL(tty_startup,(int*, int*));
extern void NDECL(tty_shutdown);
extern void FDECL(xputc, (char));
extern void FDECL(xputs, (const char *));
#if defined(SCREEN_VGA) || defined(SCREEN_8514)
extern void FDECL(xputg, (int, int, unsigned));
#endif
extern void NDECL(cl_end);
extern void NDECL(clear_screen);
extern void NDECL(home);
extern void NDECL(standoutbeg);
extern void NDECL(standoutend);
extern void NDECL(backsp);
extern void NDECL(graph_on);
extern void NDECL(graph_off);
extern void NDECL(cl_eos);

/*
 * termcap.c (or facsimiles in other ports) is the right place for doing
 * strange and arcane things such as outputting escape sequences to select
 * a color or whatever.  wintty.c should concern itself with WHERE to put
 * stuff in a window.
 */
extern void FDECL(term_start_attr,(int attr));
extern void FDECL(term_end_attr,(int attr));
extern void NDECL(term_start_raw_bold);
extern void NDECL(term_end_raw_bold);

extern void NDECL(term_end_color);
extern void FDECL(term_start_color,(int color));
extern int FDECL(has_color,(int color));


/* ### topl.c ### */

extern void FDECL(addtopl, (const char *));
extern void NDECL(more);
extern void FDECL(update_topl, (const char *));
extern void FDECL(putsyms, (const char*));

/* ### wintty.c ### */
#ifdef CLIPPING
extern void NDECL(setclipped);
#endif
extern void FDECL(docorner, (int, int));
extern void NDECL(end_glyphout);
extern void FDECL(g_putch, (int));
extern void NDECL(win_tty_init);

/* external declarations */
extern void FDECL(tty_init_nhwindows, (int *, char **));
extern void NDECL(tty_player_selection);
extern void NDECL(tty_askname);
extern void NDECL(tty_get_nh_event) ;
extern void FDECL(tty_exit_nhwindows, (const char *));
extern void FDECL(tty_suspend_nhwindows, (const char *));
extern void NDECL(tty_resume_nhwindows);
extern winid FDECL(tty_create_nhwindow, (int));
extern void FDECL(tty_clear_nhwindow, (winid));
extern void FDECL(tty_display_nhwindow, (winid, boolean));
extern void FDECL(tty_dismiss_nhwindow, (winid));
extern void FDECL(tty_destroy_nhwindow, (winid));
extern void FDECL(tty_curs, (winid,int,int));
extern void FDECL(tty_putstr, (winid, int, const char *));
extern void FDECL(tty_display_file, (const char *, boolean));
extern void FDECL(tty_start_menu, (winid));
extern void FDECL(tty_add_menu, (winid,int,const ANY_P *,
			char,char,int,const char *, boolean));
extern void FDECL(tty_end_menu, (winid, const char *));
extern int FDECL(tty_select_menu, (winid, int, MENU_ITEM_P **));
extern char FDECL(tty_message_menu, (char,int,const char *));
extern void NDECL(tty_update_inventory);
extern void NDECL(tty_mark_synch);
extern void NDECL(tty_wait_synch);
#ifdef CLIPPING
extern void FDECL(tty_cliparound, (int, int));
#endif
#ifdef POSITIONBAR
extern void FDECL(tty_update_positionbar, (char *));
#endif
extern void FDECL(tty_print_glyph, (winid,signed char,signed char,int));
extern void FDECL(tty_raw_print, (const char *));
extern void FDECL(tty_raw_print_bold, (const char *));
extern int NDECL(tty_nhgetch);
extern int FDECL(tty_nh_poskey, (int *, int *, int *));
extern void NDECL(tty_nhbell);
extern int NDECL(tty_doprev_message);
extern char FDECL(tty_yn_function, (const char *, const char *, char));
extern void FDECL(tty_getlin, (const char *,char *));
extern int NDECL(tty_get_ext_cmd);
extern void FDECL(tty_number_pad, (int));
extern void NDECL(tty_delay_output);
#ifdef CHANGE_COLOR
extern void FDECL(tty_change_color,(int color,long rgb,int reverse));
extern char * NDECL(tty_get_color_string);
#endif

/* other defs that really should go away (they're tty specific) */
extern void NDECL(tty_start_screen);
extern void NDECL(tty_end_screen);

extern void FDECL(genl_outrip, (winid,int));

#endif /* WINTTY_H */
