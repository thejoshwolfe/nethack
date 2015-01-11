/* See LICENSE in the root of this project for change info */

/* If you change the flag structure make sure you increment EDITLEVEL in   */
/* patchlevel.h if needed.  Changing the instance_flags structure does     */
/* not require incrementing EDITLEVEL.                                     */

#ifndef FLAG_H
#define FLAG_H

#include "global.h"
#include "objclass.h"
#include "coord.h"
/*
 * Persistent flags that are saved and restored with the game.
 *
 */

struct flag {
        bool  autodig;       /* MRKR: Automatically dig */
        bool  autoquiver;    /* Automatically fill quiver */
        bool  beginner;
        bool  biff;          /* enable checking for mail */
        bool  botlx;         /* print an entirely new bottom line */
        bool  confirm;       /* confirm before hitting tame monsters */
        bool  debug;         /* in debugging mode */
        bool  end_own;       /* list all own scores */
        bool  female;
        bool  forcefight;
        bool  friday13;      /* it's Friday the 13th */
        bool  help;          /* look in data file for info about stuff */
        bool  ignintr;       /* ignore interrupts */
        bool  ins_chkpt;     /* checkpoint as appropriate */
        bool  invlet_constant; /* let objects keep their inventory symbol */
        bool  legacy;        /* print game entry "story" */
        bool  lit_corridor;  /* show a dark corr as lit if it is in sight */
        bool  made_amulet;
        bool  mon_moving;    /* monsters' turn to move */
        bool  move;
        bool  mv;
        bool  bypasses;      /* bypass flag is set on at least one fobj */
        bool  nopick;        /* do not pickup objects (as when running) */
        bool  null;          /* OK to send nulls to the terminal */
        bool  perm_invent;   /* keep full inventories up until dismissed */

        bool  pushweapon;    /* When wielding, push old weapon into second slot */
        bool  rest_on_space; /* space means rest */
        bool  safe_dog;      /* give complete protection to the dog */
        bool  showexp;       /* show experience points */
        bool  silent;        /* whether the bell rings or not */
        bool  sortpack;      /* sorted inventory */
        bool  soundok;       /* ok to tell about sounds heard */
        bool  sparkle;       /* show "resisting" special FX (Scott Bigham) */
        bool  standout;      /* use standout for --More-- */
        bool  time;          /* display elapsed 'time' */
        bool  tombstone;     /* print tombstone */
        bool  toptenwin;     /* ending list in window instead of stdout */
        bool  verbose;       /* max battle info */
        bool  prayconfirm;   /* confirm before praying */
        int      end_top, end_around;   /* describe desired score list */
        unsigned ident;         /* social security number for each monster */
        unsigned moonphase;
        unsigned long suppress_alert;
#define NEW_MOON        0
#define FULL_MOON       4
        unsigned no_of_wizards; /* 0, 1 or 2 (wizard and his shadow) */
        bool  travel;        /* find way automatically to u.tx,u.ty */
        unsigned run;           /* 0: h (etc), 1: H (etc), 2: fh (etc) */
                                /* 3: FH, 4: ff+, 5: ff-, 6: FF+, 7: FF- */
                                /* 8: travel */
        unsigned long warntype; /* warn_of_mon monster type M2 */
        int      warnlevel;
        int      djinni_count, ghost_count;     /* potion effect tuning */
        int      pickup_burden;         /* maximum burden before prompt */
        char     inv_order[MAXOCLASSES];
#define NUM_DISCLOSURE_OPTIONS          5
#define DISCLOSE_PROMPT_DEFAULT_YES     'y'
#define DISCLOSE_PROMPT_DEFAULT_NO      'n'
#define DISCLOSE_YES_WITHOUT_PROMPT     '+'
#define DISCLOSE_NO_WITHOUT_PROMPT      '-'
        char     end_disclose[NUM_DISCLOSURE_OPTIONS + 1];  /* disclose various info
                                                                upon exit */
        char     menu_style;    /* User interface style setting */

        /* KMH, role patch -- Variables used during startup.
         *
         * If the user wishes to select a role, race, gender, and/or alignment
         * during startup, the choices should be recorded here.  This
         * might be specified through command-line options, environmental
         * variables, a popup dialog box, menus, etc.
         *
         * These values are each an index into an array.  They are not
         * characters or letters, because that limits us to 26 roles.
         * They are not booleans, because someday someone may need a neuter
         * gender.  Negative values are used to indicate that the user
         * hasn't yet specified that particular value.  If you determine
         * that the user wants a random choice, then you should set an
         * appropriate random value; if you just left the negative value,
         * the user would be asked again!
         *
         * These variables are stored here because the u structure is
         * cleared during character initialization, and because the
         * flags structure is restored for saved games.  Thus, we can
         * use the same parameters to build the role entry for both
         * new and restored games.
         *
         * These variables should not be referred to after the character
         * is initialized or restored (specifically, after role_init()
         * is called).
         */
        int      initrole;      /* starting role      (index into roles[])   */
        int      initrace;      /* starting race      (index into races[])   */
        int      initgend;      /* starting gender    (index into genders[]) */
        int      initalign;     /* starting alignment (index into aligns[])  */
        int      randomall;     /* randomly assign everything not specified */
        int      pantheon;      /* deity selection for priest character */
};

/*
 * Flags that are set each time the game is started.
 * These are not saved with the game.
 *
 */

struct instance_flags {
        bool  cbreak;        /* in cbreak mode, rogue format */
        bool  DECgraphics;   /* use DEC VT-xxx extended character set */
        bool  echo;          /* 1 to echo characters */
        bool  IBMgraphics;   /* use IBM extended character set */
        unsigned msg_history;   /* hint: # of top lines to save */
        bool  news;          /* print news */
        bool  window_inited; /* true if init_nhwindows() completed */
        bool  vision_inited; /* true if vision is ready */
        bool  menu_tab_sep;  /* Use tabs to separate option menu fields */
        bool  menu_requested; /* Flag for overloaded use of 'm' prefix
                                  * on some non-move commands */
        int     menu_headings;  /* ATR for menu headings */
        int      purge_monsters;        /* # of dead monsters still on fmon list */
        int *opt_booldup;       /* for duplication of bool opts in config file */
        int *opt_compdup;       /* for duplication of compound opts in config file */
        unsigned char   bouldersym;     /* symbol for boulder display */
        bool travel1;        /* first travel step */
        coord   travelcc;       /* coordinates for travel_cache */
        bool  sanity_check;  /* run sanity checks */
        bool  mon_polycontrol;       /* debug: control monster polymorphs */
        char prevmsg_window;    /* type of old message window to use */
        bool  extmenu;       /* extended commands use menu interface */
        char sortloot;          /* sort items to loot alphabetically */
        bool  paranoid_hit;  /* Ask for 'yes' when hitting peacefuls */

        bool use_hpmon;

/*
 * Window capability support.
 */
        bool wc_color;               /* use color graphics                  */
        bool wc_hilite_pet;          /* hilight pets                        */
        bool wc_ascii_map;           /* show map using traditional ascii    */
        bool wc_tiled_map;           /* show map using tiles                */
        bool wc_preload_tiles;       /* preload tiles into memory           */
        int     wc_tile_width;          /* tile width                          */
        int     wc_tile_height;         /* tile height                         */
        char    *wc_tile_file;          /* name of tile file;overrides default */
        bool wc_inverse;             /* use inverse video for some things   */
        int     wc_align_status;        /*  status win at top|bot|right|left   */
        int     wc_align_message;       /* message win at top|bot|right|left   */
        int     wc_vary_msgcount;       /* show more old messages at a time    */
        char    *wc_foregrnd_menu;      /* points to foregrnd color name for menu win   */
        char    *wc_backgrnd_menu;      /* points to backgrnd color name for menu win   */
        char    *wc_foregrnd_message;   /* points to foregrnd color name for msg win    */
        char    *wc_backgrnd_message;   /* points to backgrnd color name for msg win    */
        char    *wc_foregrnd_status;    /* points to foregrnd color name for status win */
        char    *wc_backgrnd_status;    /* points to backgrnd color name for status win */
        char    *wc_foregrnd_text;      /* points to foregrnd color name for text win   */
        char    *wc_backgrnd_text;      /* points to backgrnd color name for text win   */
        char    *wc_font_map;           /* points to font name for the map win */
        char    *wc_font_message;       /* points to font name for message win */
        char    *wc_font_status;        /* points to font name for status win  */
        char    *wc_font_menu;          /* points to font name for menu win    */
        char    *wc_font_text;          /* points to font name for text win    */
        int     wc_fontsiz_map;         /* font size for the map win           */
        int     wc_fontsiz_message;     /* font size for the message window    */
        int     wc_fontsiz_status;      /* font size for the status window     */
        int     wc_fontsiz_menu;        /* font size for the menu window       */
        int     wc_fontsiz_text;        /* font size for text windows          */
        int     wc_scroll_amount;       /* scroll this amount at scroll_margin */
        int     wc_scroll_margin;       /* scroll map when this far from
                                                the edge */
        int     wc_map_mode;            /* specify map viewing options, mostly
                                                for backward compatibility */
        int     wc_player_selection;    /* method of choosing character */
        bool wc_splash_screen;       /* display an opening splash screen or not */
        bool wc_popup_dialog;        /* put queries in pop up dialogs instead of
                                                in the message window */
        bool wc_eight_bit_input;     /* allow eight bit input               */
        bool wc_mouse_support;       /* allow mouse support */
        bool wc2_fullscreen;         /* run fullscreen */
        bool wc2_softkeyboard;       /* use software keyboard */
        bool wc2_wraptext;           /* wrap text */
    int     wc2_term_cols;      /* terminal width, in characters */
    int     wc2_term_rows;      /* terminal height, in characters */
    int     wc2_windowborders;  /* display borders on NetHack windows */
    int     wc2_petattr;        /* points to text attributes for pet */
    bool wc2_guicolor;       /* allow colors in GUI (outside map) */

        bool  cmdassist;     /* provide detailed assistance for some commands */
        bool  obsolete;      /* obsolete options can point at this, it isn't used */
        /* Items which belong in flags, but are here to allow save compatibility */
        bool  lootabc;       /* use "a/b/c" rather than "o/i/b" when looting */
        bool  showrace;      /* show hero glyph by race rather than by role */
        bool  travelcmd;     /* allow travel command */
        int      runmode;       /* update screen display during run moves */
#define AP_LEAVE 0
#define AP_GRAB  1
};

/*
 * Old deprecated names
 */
#define eight_bit_tty wc_eight_bit_input

#define use_color wc_color
#define hilite_pet wc_hilite_pet
#define use_inverse wc_inverse
#define preload_tiles wc_preload_tiles

extern struct flag flags;
extern struct instance_flags iflags;

/* runmode options */
#define RUN_TPORT       0       /* don't update display until movement stops */
#define RUN_LEAP        1       /* update display every 7 steps */
#define RUN_STEP        2       /* update display every single step */
#define RUN_CRAWL       3       /* walk w/ extra delay after each update */

#endif /* FLAG_H */
