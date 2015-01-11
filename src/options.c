/* See LICENSE in the root of this project for change info */

#include "options.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "coord.h"
#include "decl.h"
#include "display.h"
#include "drawing.h"
#include "files.h"
#include "flag.h"
#include "global.h"
#include "hack.h"
#include "hacklib.h"
#include "invent.h"
#include "mondata.h"
#include "monsym.h"
#include "objclass.h"
#include "onames.h"
#include "pline.h"
#include "pm_props.h"
#include "rm.h"
#include "rnd.h"
#include "role.h"
#include "util.h"
#include "version.h"
#include "vision.h"
#include "wintype.h"
#include "you.h"

#define WINTYPELEN 16

/*
 *  NOTE:  If you add (or delete) an option, please update the short
 *  options help (option_help()), the long options help (dat/opthelp),
 *  and the current options setting display function (doset()),
 *  and also the Guidebooks.
 *
 *  The order matters.  If an option is a an initial substring of another
 *  option (e.g. time and timed_delay) the shorter one must come first.
 */

static struct Bool_Opt {
    const char *name;
    bool *addr, initvalue;
    int optflags;
} boolopt[] = {
    {"altmeta", (bool *)0, true, DISP_IN_GAME},
    {"ascii_map",     &iflags.wc_ascii_map, true, SET_IN_GAME},    /*WC*/
    {"asksavedisk", (bool *)0, false, SET_IN_FILE},
    {"autodig", &flags.autodig, true, SET_IN_GAME},
    {"autoquiver", &flags.autoquiver, false, SET_IN_GAME},
    {"BIOS", (bool *)0, false, SET_IN_FILE},
    {"checkpoint", &flags.ins_chkpt, true, SET_IN_GAME},
    {"checkspace", (bool *)0, false, SET_IN_FILE},
    {"cmdassist", &iflags.cmdassist, true, SET_IN_GAME},
    {"color",         &iflags.wc_color, true, SET_IN_GAME},        /*WC*/
    {"confirm",&flags.confirm, true, SET_IN_GAME},
    {"cursesgraphics", (bool *)0, false, SET_IN_FILE},
    {"DECgraphics", &iflags.DECgraphics, false, SET_IN_GAME},
    {"eight_bit_tty", &iflags.wc_eight_bit_input, false, SET_IN_GAME},      /*WC*/
    {"extmenu", &iflags.extmenu, false, SET_IN_GAME},
    {"hpmon", &iflags.use_hpmon, true, SET_IN_GAME},
    {"female", &flags.female, false, DISP_IN_GAME},
    {"fixinv", &flags.invlet_constant, true, SET_IN_GAME},
    {"fullscreen", &iflags.wc2_fullscreen, false, SET_IN_FILE},
    {"guicolor", &iflags.wc2_guicolor, true, SET_IN_GAME},
    {"help", &flags.help, true, SET_IN_GAME},
    {"hilite_pet",    &iflags.wc_hilite_pet, true, SET_IN_GAME},   /*WC*/
    {"IBMgraphics", &iflags.IBMgraphics, false, SET_IN_GAME},
    {"ignintr", &flags.ignintr, false, SET_IN_GAME},
    {"large_font", &iflags.obsolete, false, SET_IN_FILE},   /* OBSOLETE */
    {"legacy", &flags.legacy, true, DISP_IN_GAME},
    {"lit_corridor", &flags.lit_corridor, false, SET_IN_GAME},
    {"lootabc", &iflags.lootabc, false, SET_IN_GAME},
    {"Macgraphics", NULL, false, SET_IN_FILE},
    {"mail", &flags.biff, true, SET_IN_GAME},
    /* for menu debugging only*/
    {"menu_tab_sep", &iflags.menu_tab_sep, false, SET_IN_GAME},
    {"mouse_support", &iflags.wc_mouse_support, true, DISP_IN_GAME},        /*WC*/
    {"news", &iflags.news, true, DISP_IN_GAME},
    {"null", &flags.null, true, SET_IN_GAME},
    {"page_wait", (bool *)0, false, SET_IN_FILE},
    {"paranoid_hit", &iflags.paranoid_hit, false, SET_IN_GAME},
    {"perm_invent", &flags.perm_invent, false, SET_IN_GAME},
    {"popup_dialog",  &iflags.wc_popup_dialog, false, SET_IN_GAME}, /*WC*/
    {"prayconfirm", &flags.prayconfirm, true, SET_IN_GAME},
    {"preload_tiles", &iflags.wc_preload_tiles, true, DISP_IN_GAME},        /*WC*/
    {"pushweapon", &flags.pushweapon, false, SET_IN_GAME},
    {"rawio", (bool *)0, false, SET_IN_FILE},
    {"rest_on_space", &flags.rest_on_space, false, SET_IN_GAME},
    {"safe_pet", &flags.safe_dog, true, SET_IN_GAME},
    {"sanity_check", &iflags.sanity_check, false, SET_IN_GAME},
    {"showexp", &flags.showexp, false, SET_IN_GAME},
    {"showrace", &iflags.showrace, false, SET_IN_GAME},
    {"silent", &flags.silent, true, SET_IN_GAME},
    {"softkeyboard", &iflags.wc2_softkeyboard, false, SET_IN_FILE},
    {"sortpack", &flags.sortpack, true, SET_IN_GAME},
    {"sound", &flags.soundok, true, SET_IN_GAME},
    {"sparkle", &flags.sparkle, true, SET_IN_GAME},
    {"standout", &flags.standout, false, SET_IN_GAME},
    {"splash_screen",     &iflags.wc_splash_screen, true, DISP_IN_GAME},    /*WC*/
    {"tiled_map",     &iflags.wc_tiled_map, false, DISP_IN_GAME},    /*WC*/
    {"time", &flags.time, false, SET_IN_GAME},
    {"tombstone",&flags.tombstone, true, SET_IN_GAME},
    {"toptenwin",&flags.toptenwin, false, SET_IN_GAME},
    {"use_inverse",   &iflags.wc_inverse, false, SET_IN_GAME},              /*WC*/
    {"verbose", &flags.verbose, true, SET_IN_GAME},
    {"wraptext", &iflags.wc2_wraptext, false, SET_IN_GAME},
    {(char *)0, (bool *)0, false, 0}
};

/* compound options, for option_help() and external programs
 * frontend */
static struct Comp_Opt {
    const char *name, *descr;
    int size;       /* for frontends and such allocating space --
                     * usually allowed size of data in game, but
                     * occasionally maximum reasonable size for
                     * typing when game maintains information in
                     * a different format */
    int optflags;
} compopt[] = {
    { "align",    "your starting alignment (lawful, neutral, or chaotic)",
        8, DISP_IN_GAME },
    { "align_message", "message window alignment", 20, DISP_IN_GAME },      /*WC*/
    { "align_status", "status window alignment", 20, DISP_IN_GAME },        /*WC*/
    { "altkeyhandler", "alternate key handler", 20, DISP_IN_GAME },
    { "boulder",  "the symbol to use for displaying boulders",
        1, SET_IN_GAME },
    { "catname",  "the name of your (first) cat (e.g., catname:Tabby)",
        PL_PSIZ, DISP_IN_GAME },
    { "disclose", "the kinds of information to disclose at end of game",
        sizeof(flags.end_disclose) * 2,
        SET_IN_GAME },
    { "dogname",  "the name of your (first) dog (e.g., dogname:Fang)",
        PL_PSIZ, DISP_IN_GAME },
    { "dumpfile", "where to dump data (e.g., dumpfile:/tmp/dump.nh)",
        PL_PSIZ, DISP_IN_GAME },
    { "dungeon",  "the symbols to use in drawing the dungeon map",
        MAXDCHARS+1, SET_IN_FILE },
    { "effects",  "the symbols to use in drawing special effects",
        MAXECHARS+1, SET_IN_FILE },
    { "font_map", "the font to use in the map window", 40, DISP_IN_GAME },  /*WC*/
    { "font_menu", "the font to use in menus", 40, DISP_IN_GAME },          /*WC*/
    { "font_message", "the font to use in the message window",
        40, DISP_IN_GAME },             /*WC*/
    { "font_size_map", "the size of the map font", 20, DISP_IN_GAME },      /*WC*/
    { "font_size_menu", "the size of the menu font", 20, DISP_IN_GAME },    /*WC*/
    { "font_size_message", "the size of the message font", 20, DISP_IN_GAME },      /*WC*/
    { "font_size_status", "the size of the status font", 20, DISP_IN_GAME },        /*WC*/
    { "font_size_text", "the size of the text font", 20, DISP_IN_GAME },    /*WC*/
    { "font_status", "the font to use in status window", 40, DISP_IN_GAME }, /*WC*/
    { "font_text", "the font to use in text windows", 40, DISP_IN_GAME },   /*WC*/
    { "fruit",    "the name of a fruit you enjoy eating",
        PL_FSIZ, SET_IN_GAME },
    { "gender",   "your starting gender (male or female)",
        8, DISP_IN_GAME },
    { "horsename", "the name of your (first) horse (e.g., horsename:Silver)",
        PL_PSIZ, DISP_IN_GAME },
    { "map_mode", "map display mode under Windows", 20, DISP_IN_GAME },     /*WC*/
    { "menucolor", "set menu colors", PL_PSIZ, SET_IN_FILE },
    { "menustyle", "user interface for object selection",
        MENUTYPELEN, SET_IN_GAME },
    { "menu_deselect_all", "deselect all items in a menu", 4, SET_IN_FILE },
    { "menu_deselect_page", "deselect all items on this page of a menu",
        4, SET_IN_FILE },
    { "menu_first_page", "jump to the first page in a menu",
        4, SET_IN_FILE },
    { "menu_headings", "bold, inverse, or underline headings", 9, SET_IN_GAME },
    { "menu_invert_all", "invert all items in a menu", 4, SET_IN_FILE },
    { "menu_invert_page", "invert all items on this page of a menu",
        4, SET_IN_FILE },
    { "menu_last_page", "jump to the last page in a menu", 4, SET_IN_FILE },
    { "menu_next_page", "goto the next menu page", 4, SET_IN_FILE },
    { "menu_previous_page", "goto the previous menu page", 4, SET_IN_FILE },
    { "menu_search", "search for a menu item", 4, SET_IN_FILE },
    { "menu_select_all", "select all items in a menu", 4, SET_IN_FILE },
    { "menu_select_page", "select all items on this page of a menu",
        4, SET_IN_FILE },
    { "monsters", "the symbols to use for monsters",
        MAXMCLASSES, SET_IN_FILE },
    { "msghistory", "number of top line messages to save",
        5, DISP_IN_GAME },
    {"msg_window", "the type of message window required",1, SET_IN_GAME},
    { "name",     "your character's name (e.g., name:Merlin-W)",
        PL_NSIZ, DISP_IN_GAME },
    { "objects",  "the symbols to use for objects",
        MAXOCLASSES, SET_IN_FILE },
    { "packorder", "the inventory order of the items in your pack",
        MAXOCLASSES, SET_IN_GAME },
    { "petattr",  "attributes for highlighting pets", 12, SET_IN_FILE },
    { "pettype",  "your preferred initial pet type", 4, DISP_IN_GAME },
    { "pickup_burden",  "maximum burden picked up before prompt",
        20, SET_IN_GAME },
    { "pickup_types", "types of objects to pick up automatically",
        MAXOCLASSES, SET_IN_GAME },
    { "player_selection", "choose character via dialog or prompts",
        12, DISP_IN_GAME },
    { "race",     "your starting race (e.g., Human, Elf)",
        PL_CSIZ, DISP_IN_GAME },
    { "role",     "your starting role (e.g., Barbarian, Valkyrie)",
        PL_CSIZ, DISP_IN_GAME },
    { "runmode", "display frequency when `running' or `travelling'",
        sizeof "teleport", SET_IN_GAME },
    { "scores",   "the parts of the score list you wish to see",
        32, SET_IN_GAME },
    { "scroll_amount", "amount to scroll map when scroll_margin is reached",
        20, DISP_IN_GAME }, /*WC*/
    { "scroll_margin", "scroll map when this far from the edge", 20, DISP_IN_GAME }, /*WC*/
    { "sortloot", "sort object selection lists by description", 4, SET_IN_GAME },
    { "suppress_alert", "suppress alerts about version-specific features",
        8, SET_IN_GAME },
    { "tile_width", "width of tiles", 20, DISP_IN_GAME},    /*WC*/
    { "tile_height", "height of tiles", 20, DISP_IN_GAME},  /*WC*/
    { "tile_file", "name of tile file", 70, DISP_IN_GAME},  /*WC*/
    { "traps",    "the symbols to use in drawing traps",
        MAXTCHARS+1, SET_IN_FILE },
    { "vary_msgcount", "show more old messages at a time", 20, DISP_IN_GAME }, /*WC*/
    { "windowcolors",  "the foreground/background colors of windows",       /*WC*/
        80, DISP_IN_GAME },
    { "windowtype", "windowing system to use", WINTYPELEN, DISP_IN_GAME },
    { (char *)0, (char *)0, 0, 0 }
};

static bool need_redraw; /* for doset() */

static char def_inv_order[MAXOCLASSES] = {
    COIN_CLASS, AMULET_CLASS, WEAPON_CLASS, ARMOR_CLASS, FOOD_CLASS,
    SCROLL_CLASS, SPBOOK_CLASS, POTION_CLASS, RING_CLASS, WAND_CLASS,
    TOOL_CLASS, GEM_CLASS, ROCK_CLASS, BALL_CLASS, CHAIN_CLASS, 0,
};

/*
 * Default menu manipulation command accelerators.  These may _not_ be:
 *
 *      + a number - reserved for counts
 *      + an upper or lower case US ASCII letter - used for accelerators
 *      + ESC - reserved for escaping the menu
 *      + NULL, CR or LF - reserved for commiting the selection(s).  NULL
 *        is kind of odd, but the tty's xwaitforspace() will return it if
 *        someone hits a <ret>.
 *      + a default object class symbol - used for object class accelerators
 *
 * Standard letters (for now) are:
 *
 *              <  back 1 page
 *              >  forward 1 page
 *              ^  first page
 *              |  last page
 *              :  search
 *
 *              page            all
 *               ,    select     .
 *               \    deselect   -
 *               ~    invert     @
 *
 * The command name list is duplicated in the compopt array.
 */
typedef struct {
    const char *name;
    char cmd;
} menu_cmd_t;

#define NUM_MENU_CMDS 11
static const menu_cmd_t default_menu_cmd_info[NUM_MENU_CMDS] = {
    /* 0*/  { "menu_first_page",    MENU_FIRST_PAGE },
    { "menu_last_page",     MENU_LAST_PAGE },
    { "menu_next_page",     MENU_NEXT_PAGE },
    { "menu_previous_page", MENU_PREVIOUS_PAGE },
    { "menu_select_all",    MENU_SELECT_ALL },
    /* 5*/  { "menu_deselect_all",  MENU_UNSELECT_ALL },
    { "menu_invert_all",    MENU_INVERT_ALL },
    { "menu_select_page",   MENU_SELECT_PAGE },
    { "menu_deselect_page", MENU_UNSELECT_PAGE },
    { "menu_invert_page",   MENU_INVERT_PAGE },
    /*10*/  { "menu_search",                MENU_SEARCH },
};

static const char *menutype[] = {
    "traditional", "combination", "partial", "full"
};

static const char *burdentype[] = {
    "unencumbered", "burdened", "stressed",
    "strained", "overtaxed", "overloaded"
};

static const char *runmodes[] = {
    "teleport", "run", "walk", "crawl"
};

static const char *sortltype[] = {
    "none", "loot", "full"
};

#define OPTIONS_HEADING "NETHACKOPTIONS"

static char fmtstr_doset_add_menu[] = "%s%-15s [%s]   ";
static char fmtstr_doset_add_menu_tab[] = "%s\t[%s]";

#define rolestring(val,array,field) ((val >= 0) ? array[val].field : \
        (val == ROLE_RANDOM) ? randomrole : none)

/* data for option_help() */
static const char *opt_intro[] = {
    "",
    "                 NetHack Options Help:",
    "",
#define CONFIG_SLOT 3   /* fill in next value at run-time */
    (char *)0,
    "or use `NETHACKOPTIONS=\"<options>\"' in your environment",
    "(<options> is a list of options separated by commas)",
    "or press \"O\" while playing and use the menu.",
    "",
    "Boolean options (which can be negated by prefixing them with '!' or \"no\"):",
    (char *)0
};

static const char *opt_epilog[] = {
    "",
    "Some of the options can be set only before the game is started; those",
    "items will not be selectable in the 'O' command's menu.",
    (char *)0
};

struct wc_Opt {
        const char *wc_name;
        unsigned long wc_bit;
};

/*
 * Allow the user to map incoming characters to various menu commands.
 * The accelerator list must be a valid C string.
 */
#define MAX_MENU_MAPPED_CMDS 32 /* some number */
char mapped_menu_cmds[MAX_MENU_MAPPED_CMDS+1];   /* exported */
static char mapped_menu_op[MAX_MENU_MAPPED_CMDS+1];
static short n_menu_mapped = 0;


static bool initial, from_file;

static void doset_add_menu(winid,const char *,int);
static void nmcpy(char *, const char *, int);
static void escapes(const char *, char *);
static void rejectoption(const char *);
static void badoption(const char *);
static char *string_for_opt(char *,bool);
static char *string_for_env_opt(const char *, char *,bool);
static void bad_negation(const char *,bool);
static int change_inv_order(char *);
static void oc_to_str(char *, char *);
static void graphics_opts(char *,const char *,int,int);
static int feature_alert_opts(char *, const char *);
static const char *get_compopt_value(const char *, char *);
static bool special_handling(const char *, bool, bool);
static void warning_opts(char *,const char *);
static void duplicate_opt_detection(const char *, int);

static void wc_set_font_name(int, char *);
static int wc_set_window_colors(char *);
static bool is_wc_option(const char *);
static bool wc_supported(const char *);
static bool is_wc2_option(const char *);
static bool wc2_supported(const char *);

/* check whether a user-supplied option string is a proper leading
   substring of a particular option name; option string might have
   a colon or equals sign and arbitrary value appended to it */
bool match_optname(const char *user_string, const char *opt_name,
        int min_length, bool val_allowed)
{
    int len = (int)strlen(user_string);

    if (val_allowed) {
        const char *p = index(user_string, ':'),
              *q = index(user_string, '=');

        if (!p || (q && q < p)) p = q;
        while(p && p > user_string && isspace(*(p-1))) p--;
        if (p) len = (int)(p - user_string);
    }

    return (len >= min_length) && !strncmpi(opt_name, user_string, len);
}

/* most environment variables will eventually be printed in an error
 * message if they don't work, and most error message paths go through
 * BUFSZ buffers, which could be overflowed by a maliciously long
 * environment variable.  if a variable can legitimately be long, or
 * if it's put in a smaller buffer, the responsible code will have to
 * bounds-check itself.
 */
char * nh_getenv (const char *ev) {
    char *getev = getenv(ev);

    if (getev && strlen(getev) <= (BUFSZ / 2))
        return getev;
    else
        return (char *)0;
}

void initoptions (void) {
    char *opts;
    int i;

    /* initialize the random number generator */
    setrandom();

    /* for detection of configfile options specified multiple times */
    iflags.opt_booldup = iflags.opt_compdup = (int *)0;

    for (i = 0; boolopt[i].name; i++) {
        if (boolopt[i].addr)
            *(boolopt[i].addr) = boolopt[i].initvalue;
    }
    flags.end_own = false;
    flags.end_top = 3;
    flags.end_around = 2;
    iflags.msg_history = 20;
    iflags.prevmsg_window = 's';
    iflags.menu_headings = ATR_INVERSE;

    /* Use negative indices to indicate not yet selected */
    flags.initrole = -1;
    flags.initrace = -1;
    flags.initgend = -1;
    flags.initalign = -1;

    /* Set the default monster and object class symbols.  Don't use */
    /* memcpy() --- sizeof char != sizeof unsigned char on some machines.   */
    for (i = 0; i < MAXOCLASSES; i++)
        oc_syms[i] = (unsigned char) def_oc_syms[i];
    for (i = 0; i < MAXMCLASSES; i++)
        monsyms[i] = (unsigned char) def_monsyms[i];
    for (i = 0; i < WARNCOUNT; i++)
        warnsyms[i] = def_warnsyms[i].sym;
    iflags.bouldersym = 0;
    flags.warnlevel = 1;
    flags.warntype = 0L;

    /* assert( sizeof flags.inv_order == sizeof def_inv_order ); */
    (void)memcpy((void *)flags.inv_order,
            (void *)def_inv_order, sizeof flags.inv_order);
    flags.pickup_burden = MOD_ENCUMBER;

    iflags.sortloot = 'n';

    for (i = 0; i < NUM_DISCLOSURE_OPTIONS; i++)
        flags.end_disclose[i] = DISCLOSE_PROMPT_DEFAULT_NO;
    /*
     * Set defaults for some options depending on what we can
     * detect about the environment's capabilities.
     * This has to be done after the global initialization above
     * and before reading user-specific initialization via
     * config file/environment variable below.
     */
    /* this detects the IBM-compatible console on most 386 boxes */
    if ((opts = nh_getenv("TERM")) && !strncmp(opts, "AT", 2)) {
        iflags.use_color = true;
    }

    flags.menu_style = MENU_FULL;

    /* since this is done before init_objects(), do partial init here */
    objects[SLIME_MOLD].oc_name_idx = SLIME_MOLD;
    nmcpy(pl_fruit, OBJ_NAME(objects[SLIME_MOLD]), PL_FSIZ);
    opts = getenv("NETHACKOPTIONS");
    if (!opts) opts = getenv("HACKOPTIONS");
    if (opts) {
        if (*opts == '/' || *opts == '\\' || *opts == '@') {
            if (*opts == '@') opts++;       /* @filename */
            /* looks like a filename */
            if (strlen(opts) < BUFSZ/2)
                read_config_file(opts);
        } else {
            read_config_file((char *)0);
            /* let the total length of options be long;
             * parseoptions() will check each individually
             */
            parseoptions(opts, true, false);
        }
    } else
        read_config_file((char *)0);

    (void)fruitadd(pl_fruit);
    /* Remove "slime mold" from list of object names; this will     */
    /* prevent it from being wished unless it's actually present    */
    /* as a named (or default) fruit.  Wishing for "fruit" will     */
    /* result in the player's preferred fruit [better than "\033"]. */
    objects[SLIME_MOLD].oc_name = "fruit";

    return;
}

static void nmcpy (char *dest, const char *src, int maxlen) {
    int     count;

    for(count = 1; count < maxlen; count++) {
        if(*src == ',' || *src == '\0') break; /*exit on \0 terminator*/
        *dest++ = *src++;
    }
    *dest = 0;
}

/*
 * escapes: escape expansion for showsyms. C-style escapes understood include
 * \n, \b, \t, \r, \xnnn (hex), \onnn (octal), \nnn (decimal). The ^-prefix
 * for control characters is also understood, and \[mM] followed by any of the
 * previous forms or by a character has the effect of 'meta'-ing the value (so
 * that the alternate character set will be enabled).
 */
static void escapes (const char *cp, char *tp) {
    while (*cp)
    {
        int     cval = 0, meta = 0;

        if (*cp == '\\' && index("mM", cp[1])) {
            meta = 1;
            cp += 2;
        }
        if (*cp == '\\' && index("0123456789xXoO", cp[1]))
        {
            const char *dp, *hex = "00112233445566778899aAbBcCdDeEfF";
            int dcount = 0;

            cp++;
            if (*cp == 'x' || *cp == 'X')
                for (++cp; (dp = index(hex, *cp)) && (dcount++ < 2); cp++)
                    cval = (cval * 16) + (dp - hex) / 2;
            else if (*cp == 'o' || *cp == 'O')
                for (++cp; (index("01234567",*cp)) && (dcount++ < 3); cp++)
                    cval = (cval * 8) + (*cp - '0');
            else
                for (; (index("0123456789",*cp)) && (dcount++ < 3); cp++)
                    cval = (cval * 10) + (*cp - '0');
        }
        else if (*cp == '\\')           /* C-style character escapes */
        {
            switch (*++cp)
            {
                case '\\': cval = '\\'; break;
                case 'n': cval = '\n'; break;
                case 't': cval = '\t'; break;
                case 'b': cval = '\b'; break;
                case 'r': cval = '\r'; break;
                default: cval = *cp;
            }
            cp++;
        }
        else if (*cp == '^')            /* expand control-character syntax */
        {
            cval = (*++cp & 0x1f);
            cp++;
        }
        else
            cval = *cp++;
        if (meta)
            cval |= 0x80;
        *tp++ = cval;
    }
    *tp = '\0';
}

static void rejectoption (const char *optname) {
    pline("%s can be set only from NETHACKOPTIONS or %s.", optname, configfile);
}

static void badoption (const char *opts) {
    if (!initial) {
        if (!strncmp(opts, "h", 1) || !strncmp(opts, "?", 1))
            option_help();
        else
            pline("Bad syntax: %s.  Enter \"?g\" for help.", opts);
        return;
    }

    if(from_file)
        raw_printf("Bad syntax in OPTIONS in %s: %s.", configfile, opts);
    else
        raw_printf("Bad syntax in NETHACKOPTIONS: %s.", opts);

}

static char * string_for_opt(char *opts, bool val_optional) {
    char *colon, *equals;

    colon = index(opts, ':');
    equals = index(opts, '=');
    if (!colon || (equals && equals < colon)) colon = equals;

    if (!colon || !*++colon) {
        if (!val_optional) badoption(opts);
        return (char *)0;
    }
    return colon;
}

static char * string_for_env_opt(const char *optname, char *opts, bool val_optional) {
    if(!initial) {
        rejectoption(optname);
        return (char *)0;
    }
    return string_for_opt(opts, val_optional);
}

static void bad_negation(const char *optname, bool with_parameter) {
    pline_The("%s option may not %sbe negated.",
            optname,
            with_parameter ? "both have a value and " : "");
}

/*
 * Change the inventory order, using the given string as the new order.
 * Missing characters in the new order are filled in at the end from
 * the current inv_order, except for gold, which is forced to be first
 * if not explicitly present.
 *
 * This routine returns 1 unless there is a duplicate or bad char in
 * the string.
 */
static int change_inv_order (char *op) {
    int oc_sym, num;
    char *sp, buf[BUFSZ];

    num = 0;
    if (!index(op, GOLD_SYM))
        buf[num++] = COIN_CLASS;

    for (sp = op; *sp; sp++) {
        oc_sym = def_char_to_objclass(*sp);
        /* reject bad or duplicate entries */
        if (oc_sym == MAXOCLASSES ||
                oc_sym == RANDOM_CLASS || oc_sym == ILLOBJ_CLASS ||
                !index(flags.inv_order, oc_sym) || index(sp+1, *sp))
            return 0;
        /* retain good ones */
        buf[num++] = (char) oc_sym;
    }
    buf[num] = '\0';

    /* fill in any omitted classes, using previous ordering */
    for (sp = flags.inv_order; *sp; sp++)
        if (!index(buf, *sp)) {
            buf[num++] = *sp;
            buf[num] = '\0';    /* explicitly terminate for next index() */
        }

    strcpy(flags.inv_order, buf);
    return 1;
}

static void graphics_opts (char *opts, const char *optype, int maxlen, int offset) {
    unsigned char translate[MAXPCHARS+1];
    int length, i;

    if (!(opts = string_for_env_opt(optype, opts, false)))
        return;
    escapes(opts, opts);

    length = strlen(opts);
    if (length > maxlen) length = maxlen;
    /* match the form obtained from PC configuration files */
    for (i = 0; i < length; i++)
        translate[i] = (unsigned char) opts[i];
    assign_graphics(translate, length, maxlen, offset);
}

static void warning_opts (char *opts, const char *optype) {
    unsigned char translate[MAXPCHARS+1];
    int length, i;

    if (!(opts = string_for_env_opt(optype, opts, false)))
        return;
    escapes(opts, opts);

    length = strlen(opts);
    if (length > WARNCOUNT) length = WARNCOUNT;
    /* match the form obtained from PC configuration files */
    for (i = 0; i < length; i++)
        translate[i] = (((i < WARNCOUNT) && opts[i]) ?
                (unsigned char) opts[i] : def_warnsyms[i].sym);
    assign_warnings(translate);
}

void assign_warnings (unsigned char *graph_chars) {
    int i;
    for (i = 0; i < WARNCOUNT; i++)
        if (graph_chars[i]) warnsyms[i] = graph_chars[i];
}

static int feature_alert_opts (char *op, const char *optn) {
    char buf[BUFSZ];
    bool rejectver = false;
    unsigned long fnv = get_feature_notice_ver(op);         /* version.c */
    if (fnv == 0L) return 0;
    if (fnv > get_current_feature_ver())
        rejectver = true;
    else
        flags.suppress_alert = fnv;
    if (rejectver) {
        if (!initial)
            You_cant("disable new feature alerts for future versions.");
        else {
            sprintf(buf,
                    "\n%s=%s Invalid reference to a future version ignored",
                    optn, op);
            badoption(buf);
        }
        return 0;
    }
    if (!initial) {
        sprintf(buf, "%lu.%lu.%lu", FEATURE_NOTICE_VER_MAJ,
                FEATURE_NOTICE_VER_MIN, FEATURE_NOTICE_VER_PATCH);
        pline("Feature change alerts disabled for NetHack %s features and prior.",
                buf);
    }
    return 1;
}

void set_duplicate_opt_detection (int on_or_off) {
    int k, *optptr;
    if (on_or_off != 0) {
        /*-- ON --*/
        if (iflags.opt_booldup)
            impossible("iflags.opt_booldup already on (memory leak)");
        iflags.opt_booldup = (int *)malloc(SIZE(boolopt) * sizeof(int));
        optptr = iflags.opt_booldup;
        for (k = 0; k < SIZE(boolopt); ++k)
            *optptr++ = 0;

        if (iflags.opt_compdup)
            impossible("iflags.opt_compdup already on (memory leak)");
        iflags.opt_compdup = (int *)malloc(SIZE(compopt) * sizeof(int));
        optptr = iflags.opt_compdup;
        for (k = 0; k < SIZE(compopt); ++k)
            *optptr++ = 0;
    } else {
        /*-- OFF --*/
        if (iflags.opt_booldup) free((void *) iflags.opt_booldup);
        iflags.opt_booldup = (int *)0;
        if (iflags.opt_compdup) free((void *) iflags.opt_compdup);
        iflags.opt_compdup = (int *)0;
    }
}

/* 0 == bool option, 1 == compound */
static void duplicate_opt_detection (const char *opts, int bool_or_comp) {
    int i, *optptr;
    if ((bool_or_comp == 0) && iflags.opt_booldup && initial && from_file) {
        for (i = 0; boolopt[i].name; i++) {
            if (match_optname(opts, boolopt[i].name, 3, false)) {
                optptr = iflags.opt_booldup + i;
                if (*optptr == 1) {
                    raw_printf(
                            "\nWarning - Boolean option specified multiple times: %s.\n",
                            opts);
                }
                *optptr += 1;
                break; /* don't match multiple options */
            }
        }
    } else if ((bool_or_comp == 1) && iflags.opt_compdup && initial && from_file) {
        for (i = 0; compopt[i].name; i++) {
            if (match_optname(opts, compopt[i].name, strlen(compopt[i].name), true)) {
                optptr = iflags.opt_compdup + i;
                if (*optptr == 1) {
                    raw_printf(
                            "\nWarning - compound option specified multiple times: %s.\n",
                            compopt[i].name);
                }
                *optptr += 1;
                break; /* don't match multiple options */
            }
        }
    }
}


void parseoptions(char *opts, bool tinitial, bool tfrom_file) {
    char *op;
    unsigned num;
    bool negated;
    int i;
    const char *fullname;

    initial = tinitial;
    from_file = tfrom_file;
    if ((op = index(opts, ',')) != 0) {
        *op++ = 0;
        parseoptions(op, initial, from_file);
    }
    if (strlen(opts) > BUFSZ/2) {
        badoption("option too long");
        return;
    }

    /* strip leading and trailing white space */
    while (isspace(*opts)) opts++;
    op = eos(opts);
    while (--op >= opts && isspace(*op)) *op = '\0';

    if (!*opts) return;
    negated = false;
    while ((*opts == '!') || !strncmpi(opts, "no", 2)) {
        if (*opts == '!') opts++; else opts += 2;
        negated = !negated;
    }

    /* variant spelling */

    if (match_optname(opts, "colour", 5, false))
        strcpy(opts, "color");  /* fortunately this isn't longer */

    if (!match_optname(opts, "subkeyvalue", 11, true)) /* allow multiple */
        duplicate_opt_detection(opts, 1);       /* 1 means compound opts */

    /* special bool options */

    if (match_optname(opts, "female", 3, false)) {
        if(!initial && flags.female == negated)
            pline("That is not anatomically possible.");
        else
            flags.initgend = flags.female = !negated;
        return;
    }

    if (match_optname(opts, "male", 4, false)) {
        if(!initial && flags.female != negated)
            pline("That is not anatomically possible.");
        else
            flags.initgend = flags.female = negated;
        return;
    }

    /* compound options */

    fullname = "pettype";
    if (match_optname(opts, fullname, 3, true)) {
        if ((op = string_for_env_opt(fullname, opts, negated)) != 0) {
            if (negated) bad_negation(fullname, true);
            else switch (*op) {
                case 'd':       /* dog */
                case 'D':
                    preferred_pet = 'd';
                    break;
                case 'c':       /* cat */
                case 'C':
                case 'f':       /* feline */
                case 'F':
                    preferred_pet = 'c';
                    break;
                case 'n':       /* no pet */
                case 'N':
                    preferred_pet = 'n';
                    break;
                default:
                    pline("Unrecognized pet type '%s'.", op);
                    break;
            }
        } else if (negated) preferred_pet = 'n';
        return;
    }

    fullname = "catname";
    if (match_optname(opts, fullname, 3, true)) {
        if (negated) bad_negation(fullname, false);
        else if ((op = string_for_env_opt(fullname, opts, false)) != 0)
            nmcpy(catname, op, PL_PSIZ);
        return;
    }

    fullname = "dogname";
    if (match_optname(opts, fullname, 3, true)) {
        if (negated) bad_negation(fullname, false);
        else if ((op = string_for_env_opt(fullname, opts, false)) != 0)
            nmcpy(dogname, op, PL_PSIZ);
        return;
    }

    fullname = "dumpfile";
    if (match_optname(opts, fullname, 3, true)) {
        return;
    }

    fullname = "horsename";
    if (match_optname(opts, fullname, 5, true)) {
        if (negated) bad_negation(fullname, false);
        else if ((op = string_for_env_opt(fullname, opts, false)) != 0)
            nmcpy(horsename, op, PL_PSIZ);
        return;
    }

    /* menucolor:"regex_string"=color */
    fullname = "menucolor";
    if (match_optname(opts, fullname, 9, true)) {
        return;
    }

    fullname = "msghistory";
    if (match_optname(opts, fullname, 3, true)) {
        op = string_for_env_opt(fullname, opts, negated);
        if ((negated && !op) || (!negated && op)) {
            iflags.msg_history = negated ? 0 : atoi(op);
        } else if (negated) bad_negation(fullname, true);
        return;
    }

    fullname="msg_window";
    /* msg_window:single, combo, full or reversed */
    if (match_optname(opts, fullname, 4, true)) {
        /* allow option to be silently ignored by non-tty ports */
        int tmp;
        if (!(op = string_for_opt(opts, true))) {
            tmp = negated ? 's' : 'f';
        } else {
            if (negated) {
                bad_negation(fullname, true);
                return;
            }
            tmp = tolower(*op);
        }
        switch (tmp) {
            case 's':       /* single message history cycle (default if negated) */
                iflags.prevmsg_window = 's';
                break;
            case 'c':       /* combination: two singles, then full page reversed */
                iflags.prevmsg_window = 'c';
                break;
            case 'f':       /* full page (default if no opts) */
                iflags.prevmsg_window = 'f';
                break;
            case 'r':       /* full page (reversed) */
                iflags.prevmsg_window = 'r';
                break;
            default:
                badoption(opts);
        }
        return;
    }

    /* WINCAP
     * setting font options  */
    fullname = "font";
    if (!strncmpi(opts, fullname, 4))
    {
        int wintype = -1;
        char *fontopts = opts + 4;

        if (!strncmpi(fontopts, "map", 3) ||
                !strncmpi(fontopts, "_map", 4))
            wintype = NHW_MAP;
        else if (!strncmpi(fontopts, "message", 7) ||
                !strncmpi(fontopts, "_message", 8))
            wintype = NHW_MESSAGE;
        else if (!strncmpi(fontopts, "text", 4) ||
                !strncmpi(fontopts, "_text", 5))
            wintype = NHW_TEXT;
        else if (!strncmpi(fontopts, "menu", 4) ||
                !strncmpi(fontopts, "_menu", 5))
            wintype = NHW_MENU;
        else if (!strncmpi(fontopts, "status", 6) ||
                !strncmpi(fontopts, "_status", 7))
            wintype = NHW_STATUS;
        else if (!strncmpi(fontopts, "_size", 5)) {
            if (!strncmpi(fontopts, "_size_map", 8))
                wintype = NHW_MAP;
            else if (!strncmpi(fontopts, "_size_message", 12))
                wintype = NHW_MESSAGE;
            else if (!strncmpi(fontopts, "_size_text", 9))
                wintype = NHW_TEXT;
            else if (!strncmpi(fontopts, "_size_menu", 9))
                wintype = NHW_MENU;
            else if (!strncmpi(fontopts, "_size_status", 11))
                wintype = NHW_STATUS;
            else {
                badoption(opts);
                return;
            }
            if (wintype > 0 && !negated &&
                    (op = string_for_opt(opts, false)) != 0) {
                switch(wintype)  {
                    case NHW_MAP:
                        iflags.wc_fontsiz_map = atoi(op);
                        break;
                    case NHW_MESSAGE:
                        iflags.wc_fontsiz_message = atoi(op);
                        break;
                    case NHW_TEXT:
                        iflags.wc_fontsiz_text = atoi(op);
                        break;
                    case NHW_MENU:
                        iflags.wc_fontsiz_menu = atoi(op);
                        break;
                    case NHW_STATUS:
                        iflags.wc_fontsiz_status = atoi(op);
                        break;
                }
            }
            return;
        } else {
            badoption(opts);
        }
        if (wintype > 0 &&
                (op = string_for_opt(opts, false)) != 0) {
            wc_set_font_name(wintype, op);
            return;
        } else if (negated) bad_negation(fullname, true);
        return;
    }

    if (match_optname(opts, "fruit", 2, true)) {
        char empty_str = '\0';
        op = string_for_opt(opts, negated);
        if (negated) {
            if (op) {
                bad_negation("fruit", true);
                return;
            }
            op = &empty_str;
            goto goodfruit;
        }
        if (!op) return;
        if (!initial) {
            struct fruit *f;

            num = 0;
            for(f=ffruit; f; f=f->nextf) {
                if (!strcmp(op, f->fname)) goto goodfruit;
                num++;
            }
            if (num >= 100) {
                pline("Doing that so many times isn't very fruitful.");
                return;
            }
        }
goodfruit:
        nmcpy(pl_fruit, op, PL_FSIZ);
        /* OBJ_NAME(objects[SLIME_MOLD]) won't work after initialization */
        if (!*pl_fruit)
            nmcpy(pl_fruit, "slime mold", PL_FSIZ);
        if (!initial)
            (void)fruitadd(pl_fruit);
        /* If initial, then initoptions is allowed to do it instead
         * of here (initoptions always has to do it even if there's
         * no fruit option at all.  Also, we don't want people
         * setting multiple fruits in their options.)
         */
        return;
    }

    /* graphics:string */
    fullname = "graphics";
    if (match_optname(opts, fullname, 2, true)) {
        if (negated) bad_negation(fullname, false);
        else graphics_opts(opts, fullname, MAXPCHARS, 0);
        return;
    }
    fullname = "dungeon";
    if (match_optname(opts, fullname, 2, true)) {
        if (negated) bad_negation(fullname, false);
        else graphics_opts(opts, fullname, MAXDCHARS, 0);
        return;
    }
    fullname = "traps";
    if (match_optname(opts, fullname, 2, true)) {
        if (negated) bad_negation(fullname, false);
        else graphics_opts(opts, fullname, MAXTCHARS, MAXDCHARS);
        return;
    }
    fullname = "effects";
    if (match_optname(opts, fullname, 2, true)) {
        if (negated) bad_negation(fullname, false);
        else
            graphics_opts(opts, fullname, MAXECHARS, MAXDCHARS+MAXTCHARS);
        return;
    }

    /* objects:string */
    fullname = "objects";
    if (match_optname(opts, fullname, 7, true)) {
        int length;

        if (negated) {
            bad_negation(fullname, false);
            return;
        }
        if (!(opts = string_for_env_opt(fullname, opts, false)))
            return;
        escapes(opts, opts);

        /*
         * Override the default object class symbols.  The first
         * object in the object class is the "random object".  I
         * don't want to use 0 as an object class, so the "random
         * object" is basically a place holder.
         *
         * The object class symbols have already been initialized in
         * initoptions().
         */
        length = strlen(opts);
        if (length >= MAXOCLASSES)
            length = MAXOCLASSES-1;     /* don't count RANDOM_OBJECT */

        for (i = 0; i < length; i++)
            oc_syms[i+1] = (unsigned char) opts[i];
        return;
    }

    /* monsters:string */
    fullname = "monsters";
    if (match_optname(opts, fullname, 8, true)) {
        int length;

        if (negated) {
            bad_negation(fullname, false);
            return;
        }
        if (!(opts = string_for_env_opt(fullname, opts, false)))
            return;
        escapes(opts, opts);

        /* Override default mon class symbols set in initoptions(). */
        length = strlen(opts);
        if (length >= MAXMCLASSES)
            length = MAXMCLASSES-1;     /* mon class 0 unused */

        for (i = 0; i < length; i++)
            monsyms[i+1] = (unsigned char) opts[i];
        return;
    }
    fullname = "warnings";
    if (match_optname(opts, fullname, 5, true)) {
        if (negated) bad_negation(fullname, false);
        else warning_opts(opts, fullname);
        return;
    }
    /* boulder:symbol */
    fullname = "boulder";
    if (match_optname(opts, fullname, 7, true)) {
        int clash = 0;
        if (negated) {
            bad_negation(fullname, false);
            return;
        }
        /*              if (!(opts = string_for_env_opt(fullname, opts, false))) */
        if (!(opts = string_for_opt(opts, false)))
            return;
        escapes(opts, opts);
        if (def_char_to_monclass(opts[0]) != MAXMCLASSES)
            clash = 1;
        else if (opts[0] >= '1' && opts[0] <= '5')
            clash = 2;
        if (clash) {
            /* symbol chosen matches a used monster or warning
               symbol which is not good - reject it*/
            pline(
                    "Badoption - boulder symbol '%c' conflicts with a %s symbol.",
                    opts[0], (clash == 1) ? "monster" : "warning");
        } else {
            /*
             * Override the default boulder symbol.
             */
            iflags.bouldersym = (unsigned char) opts[0];
        }
        if (!initial) need_redraw = true;
        return;
    }

    /* name:string */
    fullname = "name";
    if (match_optname(opts, fullname, 4, true)) {
        if (negated) bad_negation(fullname, false);
        else if ((op = string_for_env_opt(fullname, opts, false)) != 0)
            nmcpy(plname, op, PL_NSIZ);
        return;
    }

    /* role:string or character:string */
    fullname = "role";
    if (match_optname(opts, fullname, 4, true) ||
            match_optname(opts, (fullname = "character"), 4, true)) {
        if (negated) bad_negation(fullname, false);
        else if ((op = string_for_env_opt(fullname, opts, false)) != 0) {
            if ((flags.initrole = str2role(op)) == ROLE_NONE)
                badoption(opts);
            else  /* Backwards compatibility */
                nmcpy(pl_character, op, PL_NSIZ);
        }
        return;
    }

    /* race:string */
    fullname = "race";
    if (match_optname(opts, fullname, 4, true)) {
        if (negated) bad_negation(fullname, false);
        else if ((op = string_for_env_opt(fullname, opts, false)) != 0) {
            if ((flags.initrace = str2race(op)) == ROLE_NONE)
                badoption(opts);
            else /* Backwards compatibility */
                pl_race = *op;
        }
        return;
    }

    /* gender:string */
    fullname = "gender";
    if (match_optname(opts, fullname, 4, true)) {
        if (negated) bad_negation(fullname, false);
        else if ((op = string_for_env_opt(fullname, opts, false)) != 0) {
            if ((flags.initgend = str2gend(op)) == ROLE_NONE)
                badoption(opts);
            else
                flags.female = flags.initgend;
        }
        return;
    }

    /* altkeyhandler:string */
    fullname = "altkeyhandler";
    if (match_optname(opts, fullname, 4, true)) {
        if (negated) bad_negation(fullname, false);
        else if ((op = string_for_opt(opts, negated))) {
        }
        return;
    }

    /* the order to list the pack */
    fullname = "packorder";
    if (match_optname(opts, fullname, 4, true)) {
        if (negated) {
            bad_negation(fullname, false);
            return;
        } else if (!(op = string_for_opt(opts, false))) return;

        if (!change_inv_order(op))
            badoption(opts);
        return;
    }

    /* maximum burden picked up before prompt (Warren Cheung) */
    fullname = "pickup_burden";
    if (match_optname(opts, fullname, 8, true)) {
        if (negated) {
            bad_negation(fullname, false);
            return;
        } else if ((op = string_for_env_opt(fullname, opts, false)) != 0) {
            switch (tolower(*op)) {
                /* Unencumbered */
                case 'u':
                    flags.pickup_burden = UNENCUMBERED;
                    break;
                    /* Burdened (slight encumbrance) */
                case 'b':
                    flags.pickup_burden = SLT_ENCUMBER;
                    break;
                    /* streSsed (moderate encumbrance) */
                case 's':
                    flags.pickup_burden = MOD_ENCUMBER;
                    break;
                    /* straiNed (heavy encumbrance) */
                case 'n':
                    flags.pickup_burden = HVY_ENCUMBER;
                    break;
                    /* OverTaxed (extreme encumbrance) */
                case 'o':
                case 't':
                    flags.pickup_burden = EXT_ENCUMBER;
                    break;
                    /* overLoaded */
                case 'l':
                    flags.pickup_burden = OVERLOADED;
                    break;
                default:
                    badoption(opts);
            }
        }
        return;
    }

    /* things to disclose at end of game */
    if (match_optname(opts, "disclose", 7, true)) {
        /*
         * The order that the end_disclore options are stored:
         * inventory, attribs, vanquished, genocided, conduct
         * There is an array in flags:
         *      end_disclose[NUM_DISCLOSURE_OPT];
         * with option settings for the each of the following:
         * iagvc [see disclosure_options in decl.c]:
         * Legal setting values in that array are:
         *      DISCLOSE_PROMPT_DEFAULT_YES  ask with default answer yes
         *      DISCLOSE_PROMPT_DEFAULT_NO   ask with default answer no
         *      DISCLOSE_YES_WITHOUT_PROMPT  always disclose and don't ask
         *      DISCLOSE_NO_WITHOUT_PROMPT   never disclose and don't ask
         *
         * Those setting values can be used in the option
         * string as a prefix to get the desired behaviour.
         *
         * For backward compatibility, no prefix is required,
         * and the presence of a i,a,g,v, or c without a prefix
         * sets the corresponding value to DISCLOSE_YES_WITHOUT_PROMPT.
         */
        bool badopt = false;
        int idx, prefix_val;

        op = string_for_opt(opts, true);
        if (op && negated) {
            bad_negation("disclose", true);
            return;
        }
        /* "disclose" without a value means "all with prompting"
           and negated means "none without prompting" */
        if (!op || !strcmpi(op, "all") || !strcmpi(op, "none")) {
            if (op && !strcmpi(op, "none")) negated = true;
            for (num = 0; num < NUM_DISCLOSURE_OPTIONS; num++)
                flags.end_disclose[num] = negated ?
                    DISCLOSE_NO_WITHOUT_PROMPT :
                    DISCLOSE_PROMPT_DEFAULT_YES;
            return;
        }

        num = 0;
        prefix_val = -1;
        while (*op && num < sizeof flags.end_disclose - 1) {
            char c, *dop;
            static char valid_settings[] = {
                DISCLOSE_PROMPT_DEFAULT_YES,
                DISCLOSE_PROMPT_DEFAULT_NO,
                DISCLOSE_YES_WITHOUT_PROMPT,
                DISCLOSE_NO_WITHOUT_PROMPT,
                '\0'
            };
            c = lowc(*op);
            if (c == 'k') c = 'v';  /* killed -> vanquished */
            dop = index(disclosure_options, c);
            if (dop) {
                idx = dop - disclosure_options;
                if (idx < 0 || idx > NUM_DISCLOSURE_OPTIONS - 1) {
                    impossible("bad disclosure index %d %c",
                            idx, c);
                    continue;
                }
                if (prefix_val != -1) {
                    flags.end_disclose[idx] = prefix_val;
                    prefix_val = -1;
                } else
                    flags.end_disclose[idx] = DISCLOSE_YES_WITHOUT_PROMPT;
            } else if (index(valid_settings, c)) {
                prefix_val = c;
            } else if (c == ' ') {
                /* do nothing */
            } else
                badopt = true;
            op++;
        }
        if (badopt) badoption(opts);
        return;
    }

    /* scores:5t[op] 5a[round] o[wn] */
    if (match_optname(opts, "scores", 4, true)) {
        if (negated) {
            bad_negation("scores", false);
            return;
        }
        if (!(op = string_for_opt(opts, false))) return;

        while (*op) {
            int inum = 1;

            if (digit(*op)) {
                inum = atoi(op);
                while (digit(*op)) op++;
            } else if (*op == '!') {
                negated = !negated;
                op++;
            }
            while (*op == ' ') op++;

            switch (*op) {
                case 't':
                case 'T':  flags.end_top = inum;
                           break;
                case 'a':
                case 'A':  flags.end_around = inum;
                           break;
                case 'o':
                case 'O':  flags.end_own = !negated;
                           break;
                default:   badoption(opts);
                           return;
            }
            while (letter(*++op) || *op == ' ') continue;
            if (*op == '/') op++;
        }
        return;
    }

    fullname = "sortloot";
    if (match_optname(opts, fullname, 4, true)) {
        op = string_for_env_opt(fullname, opts, false);
        if (op) {
            switch (tolower(*op)) {
                case 'n':
                case 'l':
                case 'f': iflags.sortloot = tolower(*op);
                          break;
                default:  badoption(opts);
                          return;
            }
        }
        return;
    }

    fullname = "suppress_alert";
    if (match_optname(opts, fullname, 4, true)) {
        op = string_for_opt(opts, negated);
        if (negated) bad_negation(fullname, false);
        else if (op) (void) feature_alert_opts(op,fullname);
        return;
    }

    /* WINCAP
     * vary_msgcount:nn */
    fullname = "vary_msgcount";
    if (match_optname(opts, fullname, sizeof("vary_msgcount")-1, true)) {
        op = string_for_opt(opts, negated);
        if ((negated && !op) || (!negated && op)) {
            iflags.wc_vary_msgcount = negated ? 0 : atoi(op);
        } else if (negated) bad_negation(fullname, true);
        return;
    }

    /* WINCAP
     * setting window colors
     * syntax: windowcolors=menu foregrnd/backgrnd text foregrnd/backgrnd
     */
    fullname = "windowcolors";
    if (match_optname(opts, fullname, 7, true)) {
        if ((op = string_for_opt(opts, false)) != 0) {
            if (!wc_set_window_colors(op))
                badoption(opts);
        } else if (negated) bad_negation(fullname, true);
        return;
    }

    /* WINCAP2
     * term_cols:amount */
    fullname = "term_cols";
    if (match_optname(opts, fullname, sizeof("term_cols")-1, true)) {
        op = string_for_opt(opts, negated);
        iflags.wc2_term_cols = atoi(op);
        if (negated) bad_negation(fullname, false);
        return;
    }

    /* WINCAP2
     * term_rows:amount */
    fullname = "term_rows";
    if (match_optname(opts, fullname, sizeof("term_rows")-1, true)) {
        op = string_for_opt(opts, negated);
        iflags.wc2_term_rows = atoi(op);
        if (negated) bad_negation(fullname, false);
        return;
    }


    /* WINCAP2
     * windowborders:n */
    fullname = "windowborders";
    if (match_optname(opts, fullname, sizeof("windowborders")-1, true)) {
        op = string_for_opt(opts, negated);
        if (negated && op) bad_negation(fullname, true);
        else {
            if (negated)
                iflags.wc2_windowborders = 2; /* Off */
            else if (!op)
                iflags.wc2_windowborders = 1; /* On */
            else    /* Value supplied */
                iflags.wc2_windowborders = atoi(op);
            if ((iflags.wc2_windowborders > 3) ||
                    (iflags.wc2_windowborders < 1)) {
                iflags.wc2_windowborders = 0;
                badoption(opts);
            }
        }
        return;
    }

    /* menustyle:traditional or combo or full or partial */
    if (match_optname(opts, "menustyle", 4, true)) {
        int tmp;
        bool val_required = (strlen(opts) > 5 && !negated);

        if (!(op = string_for_opt(opts, !val_required))) {
            if (val_required) return; /* string_for_opt gave feedback */
            tmp = negated ? 'n' : 'f';
        } else {
            tmp = tolower(*op);
        }
        switch (tmp) {
            case 'n':       /* none */
            case 't':       /* traditional */
                flags.menu_style = MENU_TRADITIONAL;
                break;
            case 'c':       /* combo: trad.class sel+menu */
                flags.menu_style = MENU_COMBINATION;
                break;
            case 'p':       /* partial: no class menu */
                flags.menu_style = MENU_PARTIAL;
                break;
            case 'f':       /* full: class menu + menu */
                flags.menu_style = MENU_FULL;
                break;
            default:
                badoption(opts);
        }
        return;
    }

    fullname = "menu_headings";
    if (match_optname(opts, fullname, 12, true)) {
        if (negated) {
            bad_negation(fullname, false);
            return;
        }
        else if (!(opts = string_for_env_opt(fullname, opts, false))) {
            return;
        }
        if (!strcmpi(opts,"bold"))
            iflags.menu_headings = ATR_BOLD;
        else if (!strcmpi(opts,"inverse"))
            iflags.menu_headings = ATR_INVERSE;
        else if (!strcmpi(opts,"underline"))
            iflags.menu_headings = ATR_ULINE;
        else
            badoption(opts);
        return;
    }

    /* check for menu command mapping */
    for (i = 0; i < NUM_MENU_CMDS; i++) {
        fullname = default_menu_cmd_info[i].name;
        if (match_optname(opts, fullname, (int)strlen(fullname), true)) {
            if (negated)
                bad_negation(fullname, false);
            else if ((op = string_for_opt(opts, false)) != 0) {
                int j;
                char c, op_buf[BUFSZ];
                bool isbad = false;

                escapes(op, op_buf);
                c = *op_buf;

                if (c == 0 || c == '\r' || c == '\n' || c == '\033' ||
                        c == ' ' || digit(c) || (letter(c) && c != '@'))
                    isbad = true;
                else        /* reject default object class symbols */
                    for (j = 1; j < MAXOCLASSES; j++)
                        if (c == def_oc_syms[i]) {
                            isbad = true;
                            break;
                        }

                if (isbad)
                    badoption(opts);
                else
                    add_menu_cmd_alias(c, default_menu_cmd_info[i].cmd);
            }
            return;
        }
    }

    /* OK, if we still haven't recognized the option, check the bool
     * options list
     */
    for (i = 0; boolopt[i].name; i++) {
        if (match_optname(opts, boolopt[i].name, 3, false)) {
            /* options that don't exist */
            if (!boolopt[i].addr) {
                if (!initial && !negated)
                    pline_The("\"%s\" option is not available.",
                            boolopt[i].name);
                return;
            }
            /* options that must come from config file */
            if (!initial && (boolopt[i].optflags == SET_IN_FILE)) {
                rejectoption(boolopt[i].name);
                return;
            }

            *(boolopt[i].addr) = !negated;

            duplicate_opt_detection(boolopt[i].name, 0);

            /* only do processing below if setting with doset() */
            if (initial) return;

            if ((boolopt[i].addr) == &flags.time || (boolopt[i].addr) == &flags.showexp) {
            } else if ((boolopt[i].addr) == &flags.invlet_constant) {
                if (flags.invlet_constant) reassign();
            }
            else if ((boolopt[i].addr) == &flags.lit_corridor) {
                /*
                 * All corridor squares seen via night vision or
                 * candles & lamps change.  Update them by calling
                 * newsym() on them.  Don't do this if we are
                 * initializing the options --- the vision system
                 * isn't set up yet.
                 */
                vision_recalc(2);           /* shut down vision */
                vision_full_recalc = 1;     /* delayed recalc */
            }
            else if ((boolopt[i].addr) == &iflags.use_inverse ||
                    (boolopt[i].addr) == &iflags.showrace ||
                    (boolopt[i].addr) == &iflags.hilite_pet ||
                    (boolopt[i].addr) == &iflags.wc2_guicolor) {
                need_redraw = true;
            }
            else if ((boolopt[i].addr) == &iflags.use_color) {
                need_redraw = true;
            }

            return;
        }
    }

    /* out of valid options */
    badoption(opts);
}

/*
 * Convert the given string of object classes to a string of default object
 * symbols.
 */
static void oc_to_str (char *src, char *dest) {
    int i;

    while ((i = (int) *src++) != 0) {
        if (i < 0 || i >= MAXOCLASSES)
            impossible("oc_to_str:  illegal object class %d", i);
        else
            *dest++ = def_oc_syms[i];
    }
    *dest = '\0';
}

/*
 * Add the given mapping to the menu command map list.  Always keep the
 * maps valid C strings.
 */
void add_menu_cmd_alias (char from_ch, char to_ch) {
    if (n_menu_mapped >= MAX_MENU_MAPPED_CMDS)
        pline("out of menu map space.");
    else {
        mapped_menu_cmds[n_menu_mapped] = from_ch;
        mapped_menu_op[n_menu_mapped] = to_ch;
        n_menu_mapped++;
        mapped_menu_cmds[n_menu_mapped] = 0;
        mapped_menu_op[n_menu_mapped] = 0;
    }
}

/*
 * Map the given character to its corresponding menu command.  If it
 * doesn't match anything, just return the original.
 */
char map_menu_cmd (char ch) {
    char *found = index(mapped_menu_cmds, ch);
    if (found) {
        int idx = found - mapped_menu_cmds;
        ch = mapped_menu_op[idx];
    }
    return ch;
}



/*
   winid win,                  window to add to
   const char *option,         option name
   int indexoffset             value to add to index in compopt[], or zero
   if option cannot be changed
   */
static void doset_add_menu (winid win, const char *option, int indexoffset) {
    const char *value = "unknown";              /* current value */
    char buf[BUFSZ], buf2[BUFSZ];
    anything any;
    int i;

    any.a_void = 0;
    if (indexoffset == 0) {
        any.a_int = 0;
        value = get_compopt_value(option, buf2);
    } else {
        for (i=0; compopt[i].name; i++)
            if (strcmp(option, compopt[i].name) == 0) break;

        if (compopt[i].name) {
            any.a_int = i + 1 + indexoffset;
            value = get_compopt_value(option, buf2);
        } else {
            /* We are trying to add an option not found in compopt[].
               This is almost certainly bad, but we'll let it through anyway
               (with a zero value, so it can't be selected). */
            any.a_int = 0;
        }
    }
    /* "    " replaces "a - " -- assumes menus follow that style */
    if (!iflags.menu_tab_sep)
        sprintf(buf, fmtstr_doset_add_menu, any.a_int ? "" : "    ", option, value);
    else
        sprintf(buf, fmtstr_doset_add_menu_tab, option, value);
    add_menu(win, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, MENU_UNSELECTED);
}

/* Changing options via menu by Per Liboriussen */
int doset (void) {
    char buf[BUFSZ], buf2[BUFSZ];
    int i, pass, boolcount, pick_cnt, pick_idx, opt_indx;
    bool *bool_p;
    winid tmpwin;
    anything any;
    menu_item *pick_list;
    int indexoffset, startpass, endpass;
    bool setinitial = false, fromfile = false;
    int biggest_name = 0;

    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin);

    any.a_void = 0;
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, iflags.menu_headings,
            "Booleans (selecting will toggle value):", MENU_UNSELECTED);
    any.a_int = 0;
    /* first list any other non-modifiable booleans, then modifiable ones */
    for (pass = 0; pass <= 1; pass++)
        for (i = 0; boolopt[i].name; i++)
            if ((bool_p = boolopt[i].addr) != 0 &&
                    ((boolopt[i].optflags == DISP_IN_GAME && pass == 0) ||
                     (boolopt[i].optflags == SET_IN_GAME && pass == 1))) {
                if (bool_p == &flags.female) continue;  /* obsolete */
                if (bool_p == &iflags.sanity_check && !flags.debug) continue;
                if (bool_p == &iflags.menu_tab_sep && !flags.debug) continue;
                if (is_wc_option(boolopt[i].name) &&
                        !wc_supported(boolopt[i].name)) continue;
                if (is_wc2_option(boolopt[i].name) &&
                        !wc2_supported(boolopt[i].name)) continue;
                any.a_int = (pass == 0) ? 0 : i + 1;
                if (!iflags.menu_tab_sep)
                    sprintf(buf, "%s%-13s [%s]",
                            pass == 0 ? "    " : "",
                            boolopt[i].name, *bool_p ? "true" : "false");
                else
                    sprintf(buf, "%s\t[%s]",
                            boolopt[i].name, *bool_p ? "true" : "false");
                add_menu(tmpwin, NO_GLYPH, &any, 0, 0,
                        ATR_NONE, buf, MENU_UNSELECTED);
            }

    boolcount = i;
    indexoffset = boolcount;
    any.a_void = 0;
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, "", MENU_UNSELECTED);
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, iflags.menu_headings,
            "Compounds (selecting will prompt for new value):",
            MENU_UNSELECTED);

    startpass = DISP_IN_GAME;
    endpass = SET_IN_GAME;

    /* spin through the options to find the biggest name
       and adjust the format string accordingly if needed */
    biggest_name = 0;
    for (i = 0; compopt[i].name; i++)
        if (compopt[i].optflags >= startpass && compopt[i].optflags <= endpass &&
                strlen(compopt[i].name) > (unsigned) biggest_name)
            biggest_name = (int) strlen(compopt[i].name);
    if (biggest_name > 30) biggest_name = 30;
    if (!iflags.menu_tab_sep)
        sprintf(fmtstr_doset_add_menu, "%%s%%-%ds [%%s]", biggest_name);

    /* deliberately put `name', `role', `race', `gender' first */
    doset_add_menu(tmpwin, "name", 0);
    doset_add_menu(tmpwin, "role", 0);
    doset_add_menu(tmpwin, "race", 0);
    doset_add_menu(tmpwin, "gender", 0);

    for (pass = startpass; pass <= endpass; pass++)
        for (i = 0; compopt[i].name; i++)
            if (compopt[i].optflags == pass) {
                if (!strcmp(compopt[i].name, "name") ||
                        !strcmp(compopt[i].name, "role") ||
                        !strcmp(compopt[i].name, "race") ||
                        !strcmp(compopt[i].name, "gender"))
                    continue;
                else if (is_wc_option(compopt[i].name) &&
                        !wc_supported(compopt[i].name))
                    continue;
                else if (is_wc2_option(compopt[i].name) &&
                        !wc2_supported(compopt[i].name))
                    continue;
                else
                    doset_add_menu(tmpwin, compopt[i].name,
                            (pass == DISP_IN_GAME) ? 0 : indexoffset);
            }
    any.a_int = -1;
    sprintf(buf, "autopickup exceptions (%d currently set)", 0);
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, MENU_UNSELECTED);

    any.a_void = 0;
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, "", MENU_UNSELECTED);
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, iflags.menu_headings,
            "Variable playground locations:", MENU_UNSELECTED);
    for (i = 0; i < PREFIX_COUNT; i++)
        doset_add_menu(tmpwin, fqn_prefix_names[i], 0);
    end_menu(tmpwin, "Set what options?");
    need_redraw = false;
    if ((pick_cnt = select_menu(tmpwin, PICK_ANY, &pick_list)) > 0) {
        /*
         * Walk down the selection list and either invert the booleans
         * or prompt for new values. In most cases, call parseoptions()
         * to take care of options that require special attention, like
         * redraws.
         */
        for (pick_idx = 0; pick_idx < pick_cnt; ++pick_idx) {
            opt_indx = pick_list[pick_idx].item.a_int - 1;
            if (opt_indx == -2) {
                special_handling("autopickup_exception",
                        setinitial, fromfile);
            } else
                if (opt_indx < boolcount) {
                    /* bool option */
                    sprintf(buf, "%s%s", *boolopt[opt_indx].addr ? "!" : "",
                            boolopt[opt_indx].name);
                    parseoptions(buf, setinitial, fromfile);
                } else {
                    /* compound option */
                    opt_indx -= boolcount;

                    if (!special_handling(compopt[opt_indx].name,
                                setinitial, fromfile)) {
                        sprintf(buf, "Set %s to what?", compopt[opt_indx].name);
                        getlin(buf, buf2);
                        if (buf2[0] == '\033')
                            continue;
                        sprintf(buf, "%s:%s", compopt[opt_indx].name, buf2);
                        /* pass the buck */
                        parseoptions(buf, setinitial, fromfile);
                    }
                }
        }
        free((void *)pick_list);
        pick_list = (menu_item *)0;
    }

    destroy_nhwindow(tmpwin);
    if (need_redraw)
        (void) doredraw();
    return 0;
}

static bool special_handling(const char *optname, bool setinitial, bool setfromfile) {
    winid tmpwin;
    anything any;
    int i;
    char buf[BUFSZ];
    bool retval = false;

    /* Special handling of menustyle, pickup_burden, pickup_types,
     * disclose, runmode, msg_window, menu_headings, and sortloot
     * Also takes care of interactive autopickup_exception_handling changes.
     */
    if (!strcmp("menustyle", optname)) {
        const char *style_name;
        menu_item *style_pick = (menu_item *)0;
        tmpwin = create_nhwindow(NHW_MENU);
        start_menu(tmpwin);
        for (i = 0; i < SIZE(menutype); i++) {
            style_name = menutype[i];
            /* note: separate `style_name' variable used
               to avoid an optimizer bug in VAX C V2.3 */
            any.a_int = i + 1;
            add_menu(tmpwin, NO_GLYPH, &any, *style_name, 0,
                    ATR_NONE, style_name, MENU_UNSELECTED);
        }
        end_menu(tmpwin, "Select menustyle:");
        if (select_menu(tmpwin, PICK_ONE, &style_pick) > 0) {
            flags.menu_style = style_pick->item.a_int - 1;
            free((void *)style_pick);
        }
        destroy_nhwindow(tmpwin);
        retval = true;
    } else if (!strcmp("pickup_burden", optname)) {
        const char *burden_name, *burden_letters = "ubsntl";
        menu_item *burden_pick = (menu_item *)0;
        tmpwin = create_nhwindow(NHW_MENU);
        start_menu(tmpwin);
        for (i = 0; i < SIZE(burdentype); i++) {
            burden_name = burdentype[i];
            any.a_int = i + 1;
            add_menu(tmpwin, NO_GLYPH, &any, burden_letters[i], 0,
                    ATR_NONE, burden_name, MENU_UNSELECTED);
        }
        end_menu(tmpwin, "Select encumbrance level:");
        if (select_menu(tmpwin, PICK_ONE, &burden_pick) > 0) {
            flags.pickup_burden = burden_pick->item.a_int - 1;
            free((void *)burden_pick);
        }
        destroy_nhwindow(tmpwin);
        retval = true;
    } else if (!strcmp("pickup_types", optname)) {
        /* parseoptions will prompt for the list of types */
        parseoptions(strcpy(buf, "pickup_types"), setinitial, setfromfile);
        retval = true;
    } else if (!strcmp("disclose", optname)) {
        int pick_cnt, pick_idx, opt_idx;
        menu_item *disclosure_category_pick = (menu_item *)0;
        /*
         * The order of disclose_names[]
         * must correspond to disclosure_options in decl.h
         */
        static const char *disclosure_names[] = {
            "inventory", "attributes", "vanquished", "genocides", "conduct"
        };
        int disc_cat[NUM_DISCLOSURE_OPTIONS];
        const char *disclosure_name;

        tmpwin = create_nhwindow(NHW_MENU);
        start_menu(tmpwin);
        for (i = 0; i < NUM_DISCLOSURE_OPTIONS; i++) {
            disclosure_name = disclosure_names[i];
            any.a_int = i + 1;
            add_menu(tmpwin, NO_GLYPH, &any, disclosure_options[i], 0,
                    ATR_NONE, disclosure_name, MENU_UNSELECTED);
            disc_cat[i] = 0;
        }
        end_menu(tmpwin, "Change which disclosure options categories:");
        if ((pick_cnt = select_menu(tmpwin, PICK_ANY, &disclosure_category_pick)) > 0) {
            for (pick_idx = 0; pick_idx < pick_cnt; ++pick_idx) {
                opt_idx = disclosure_category_pick[pick_idx].item.a_int - 1;
                disc_cat[opt_idx] = 1;
            }
            free((void *)disclosure_category_pick);
            disclosure_category_pick = (menu_item *)0;
        }
        destroy_nhwindow(tmpwin);

        for (i = 0; i < NUM_DISCLOSURE_OPTIONS; i++) {
            if (disc_cat[i]) {
                char dbuf[BUFSZ];
                menu_item *disclosure_option_pick = (menu_item *)0;
                sprintf(dbuf, "Disclosure options for %s:", disclosure_names[i]);
                tmpwin = create_nhwindow(NHW_MENU);
                start_menu(tmpwin);
                any.a_char = DISCLOSE_NO_WITHOUT_PROMPT;
                add_menu(tmpwin, NO_GLYPH, &any, 'a', 0,
                        ATR_NONE,"Never disclose and don't prompt", MENU_UNSELECTED);
                any.a_void = 0;
                any.a_char = DISCLOSE_YES_WITHOUT_PROMPT;
                add_menu(tmpwin, NO_GLYPH, &any, 'b', 0,
                        ATR_NONE,"Always disclose and don't prompt", MENU_UNSELECTED);
                any.a_void = 0;
                any.a_char = DISCLOSE_PROMPT_DEFAULT_NO;
                add_menu(tmpwin, NO_GLYPH, &any, 'c', 0,
                        ATR_NONE,"Prompt and default answer to \"No\"", MENU_UNSELECTED);
                any.a_void = 0;
                any.a_char = DISCLOSE_PROMPT_DEFAULT_YES;
                add_menu(tmpwin, NO_GLYPH, &any, 'd', 0,
                        ATR_NONE,"Prompt and default answer to \"Yes\"", MENU_UNSELECTED);
                end_menu(tmpwin, dbuf);
                if (select_menu(tmpwin, PICK_ONE, &disclosure_option_pick) > 0) {
                    flags.end_disclose[i] = disclosure_option_pick->item.a_char;
                    free((void *)disclosure_option_pick);
                }
                destroy_nhwindow(tmpwin);
            }
        }
        retval = true;
    } else if (!strcmp("msg_window", optname)) {
        /* by Christian W. Cooper */
        menu_item *window_pick = (menu_item *)0;
        tmpwin = create_nhwindow(NHW_MENU);
        start_menu(tmpwin);
        any.a_char = 's';
        add_menu(tmpwin, NO_GLYPH, &any, 's', 0,
                ATR_NONE, "single", MENU_UNSELECTED);
        any.a_char = 'c';
        add_menu(tmpwin, NO_GLYPH, &any, 'c', 0,
                ATR_NONE, "combination", MENU_UNSELECTED);
        any.a_char = 'f';
        add_menu(tmpwin, NO_GLYPH, &any, 'f', 0,
                ATR_NONE, "full", MENU_UNSELECTED);
        any.a_char = 'r';
        add_menu(tmpwin, NO_GLYPH, &any, 'r', 0,
                ATR_NONE, "reversed", MENU_UNSELECTED);
        end_menu(tmpwin, "Select message history display type:");
        if (select_menu(tmpwin, PICK_ONE, &window_pick) > 0) {
            iflags.prevmsg_window = window_pick->item.a_char;
            free((void *)window_pick);
        }
        destroy_nhwindow(tmpwin);
        retval = true;
    } else if (!strcmp("sortloot", optname)) {
        const char *sortl_name;
        menu_item *sortl_pick = (menu_item *)0;
        tmpwin = create_nhwindow(NHW_MENU);
        start_menu(tmpwin);
        for (i = 0; i < SIZE(sortltype); i++) {
            sortl_name = sortltype[i];
            any.a_char = *sortl_name;
            add_menu(tmpwin, NO_GLYPH, &any, *sortl_name, 0,
                    ATR_NONE, sortl_name, MENU_UNSELECTED);
        }
        end_menu(tmpwin, "Select loot sorting type:");
        if (select_menu(tmpwin, PICK_ONE, &sortl_pick) > 0) {
            iflags.sortloot = sortl_pick->item.a_char;
            free((void *)sortl_pick);
        }
        destroy_nhwindow(tmpwin);
        retval = true;
    } else if (!strcmp("menu_headings", optname)) {
        static const char *mhchoices[3] = {"bold", "inverse", "underline"};
        const char *npletters = "biu";
        menu_item *mode_pick = (menu_item *)0;

        tmpwin = create_nhwindow(NHW_MENU);
        start_menu(tmpwin);
        for (i = 0; i < SIZE(mhchoices); i++) {
            any.a_int = i + 1;
            add_menu(tmpwin, NO_GLYPH, &any, npletters[i], 0,
                    ATR_NONE, mhchoices[i], MENU_UNSELECTED);
        }
        end_menu(tmpwin, "How to highlight menu headings:");
        if (select_menu(tmpwin, PICK_ONE, &mode_pick) > 0) {
            int mode = mode_pick->item.a_int - 1;
            switch(mode) {
                case 2:
                    iflags.menu_headings = ATR_ULINE;
                    break;
                case 0:
                    iflags.menu_headings = ATR_BOLD;
                    break;
                case 1:
                default:
                    iflags.menu_headings = ATR_INVERSE;
            }
            free((void *)mode_pick);
        }
        destroy_nhwindow(tmpwin);
        retval = true;
    }
    return retval;
}

/* This is ugly. We have all the option names in the compopt[] array,
   but we need to look at each option individually to get the value. */
static const char * get_compopt_value(const char *optname, char *buf) {
    char ocl[MAXOCLASSES + 1];
    static const char none[] = "(none)", randomrole[] = "random", to_be_done[] = "(to be done)", defopt[] = "default", defbrief[] = "def";
    int i;

    buf[0] = '\0';
    if (!strcmp(optname, "boulder"))
        sprintf(buf, "%c", iflags.bouldersym ? iflags.bouldersym : oc_syms[(int)objects[BOULDER].oc_class]);
    else if (!strcmp(optname, "catname"))
        sprintf(buf, "%s", catname[0] ? catname : none);
    else if (!strcmp(optname, "disclose")) {
        for (i = 0; i < NUM_DISCLOSURE_OPTIONS; i++) {
            char topt[2];
            if (i)
                strcat(buf, " ");
            topt[1] = '\0';
            topt[0] = flags.end_disclose[i];
            strcat(buf, topt);
            topt[0] = disclosure_options[i];
            strcat(buf, topt);
        }
    } else if (!strcmp(optname, "dogname"))
        sprintf(buf, "%s", dogname[0] ? dogname : none);
    else if (!strcmp(optname, "dumpfile"))
        sprintf(buf, "%s", dump_fn[0] ? dump_fn : none);
    else if (!strcmp(optname, "dungeon"))
        sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "effects"))
        sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "font_map"))
        sprintf(buf, "%s", iflags.wc_font_map ? iflags.wc_font_map : defopt);
    else if (!strcmp(optname, "font_message"))
        sprintf(buf, "%s", iflags.wc_font_message ? iflags.wc_font_message : defopt);
    else if (!strcmp(optname, "font_status"))
        sprintf(buf, "%s", iflags.wc_font_status ? iflags.wc_font_status : defopt);
    else if (!strcmp(optname, "font_menu"))
        sprintf(buf, "%s", iflags.wc_font_menu ? iflags.wc_font_menu : defopt);
    else if (!strcmp(optname, "font_text"))
        sprintf(buf, "%s", iflags.wc_font_text ? iflags.wc_font_text : defopt);
    else if (!strcmp(optname, "font_size_map")) {
        if (iflags.wc_fontsiz_map)
            sprintf(buf, "%d", iflags.wc_fontsiz_map);
        else
            strcpy(buf, defopt);
    } else if (!strcmp(optname, "font_size_message")) {
        if (iflags.wc_fontsiz_message)
            sprintf(buf, "%d", iflags.wc_fontsiz_message);
        else
            strcpy(buf, defopt);
    } else if (!strcmp(optname, "font_size_status")) {
        if (iflags.wc_fontsiz_status)
            sprintf(buf, "%d", iflags.wc_fontsiz_status);
        else
            strcpy(buf, defopt);
    } else if (!strcmp(optname, "font_size_menu")) {
        if (iflags.wc_fontsiz_menu)
            sprintf(buf, "%d", iflags.wc_fontsiz_menu);
        else
            strcpy(buf, defopt);
    } else if (!strcmp(optname, "font_size_text")) {
        if (iflags.wc_fontsiz_text)
            sprintf(buf, "%d", iflags.wc_fontsiz_text);
        else
            strcpy(buf, defopt);
    } else if (!strcmp(optname, "fruit"))
        sprintf(buf, "%s", pl_fruit);
    else if (!strcmp(optname, "gender"))
        sprintf(buf, "%s", rolestring(flags.initgend, genders, adj));
    else if (!strcmp(optname, "horsename"))
        sprintf(buf, "%s", horsename[0] ? horsename : none);
    else if (!strcmp(optname, "menustyle"))
        sprintf(buf, "%s", menutype[(int)flags.menu_style]);
    else if (!strcmp(optname, "menu_deselect_all"))
        sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "menu_deselect_page"))
        sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "menu_first_page"))
        sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "menu_invert_all"))
        sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "menu_headings")) {
        sprintf(buf, "%s", (iflags.menu_headings == ATR_BOLD) ? "bold" : (iflags.menu_headings == ATR_INVERSE) ? "inverse" : (iflags.menu_headings == ATR_ULINE) ? "underline" : "unknown");
    } else if (!strcmp(optname, "menu_invert_page"))
        sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "menu_last_page"))
        sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "menu_next_page"))
        sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "menu_previous_page"))
        sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "menu_search"))
        sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "menu_select_all"))
        sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "menu_select_page"))
        sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "monsters"))
        sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "msghistory"))
        sprintf(buf, "%u", iflags.msg_history);
    else if (!strcmp(optname, "msg_window"))
        sprintf(buf, "%s", (iflags.prevmsg_window == 's') ? "single" : (iflags.prevmsg_window == 'c') ? "combination" : (iflags.prevmsg_window == 'f') ? "full" : "reversed");
    else if (!strcmp(optname, "name"))
        sprintf(buf, "%s", plname);
    else if (!strcmp(optname, "objects"))
        sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "packorder")) {
        oc_to_str(flags.inv_order, ocl);
        sprintf(buf, "%s", ocl);
    } else if (!strcmp(optname, "pettype"))
        sprintf(buf, "%s", (preferred_pet == 'c') ? "cat" : (preferred_pet == 'd') ? "dog" : (preferred_pet == 'n') ? "none" : "random");
    else if (!strcmp(optname, "pickup_burden"))
        sprintf(buf, "%s", burdentype[flags.pickup_burden]);
    else if (!strcmp(optname, "race"))
        sprintf(buf, "%s", rolestring(flags.initrace, races, noun));
    else if (!strcmp(optname, "role"))
        sprintf(buf, "%s", rolestring(flags.initrole, roles, name.m));
    else if (!strcmp(optname, "scores")) {
        sprintf(buf, "%d top/%d around%s", flags.end_top, flags.end_around, flags.end_own ? "/own" : "");
    } else if (!strcmp(optname, "scroll_amount")) {
        if (iflags.wc_scroll_amount)
            sprintf(buf, "%d", iflags.wc_scroll_amount);
        else
            strcpy(buf, defopt);
    } else if (!strcmp(optname, "scroll_margin")) {
        if (iflags.wc_scroll_margin)
            sprintf(buf, "%d", iflags.wc_scroll_margin);
        else
            strcpy(buf, defopt);
    } else if (!strcmp(optname, "sortloot")) {
        char *sortname = (char *)NULL;
        for (i = 0; i < SIZE(sortltype) && sortname == (char *)NULL; i++) {
            if (iflags.sortloot == sortltype[i][0])
                sortname = (char *)sortltype[i];
        }
        if (sortname != (char *)NULL)
            sprintf(buf, "%s", sortname);
    } else if (!strcmp(optname, "player_selection"))
        sprintf(buf, "%s", iflags.wc_player_selection ? "prompts" : "dialog");
    else if (!strcmp(optname, "suppress_alert")) {
        if (flags.suppress_alert == 0L)
            strcpy(buf, none);
        else
            sprintf(buf, "%lu.%lu.%lu",
            FEATURE_NOTICE_VER_MAJ,
            FEATURE_NOTICE_VER_MIN,
            FEATURE_NOTICE_VER_PATCH);
    } else if (!strcmp(optname, "term_cols")) {
        if (iflags.wc2_term_cols)
            sprintf(buf, "%d", iflags.wc2_term_cols);
        else
            strcpy(buf, defopt);
    } else if (!strcmp(optname, "term_rows")) {
        if (iflags.wc2_term_rows)
            sprintf(buf, "%d", iflags.wc2_term_rows);
        else
            strcpy(buf, defopt);
    } else if (!strcmp(optname, "tile_file"))
        sprintf(buf, "%s", iflags.wc_tile_file ? iflags.wc_tile_file : defopt);
    else if (!strcmp(optname, "tile_height")) {
        if (iflags.wc_tile_height)
            sprintf(buf, "%d", iflags.wc_tile_height);
        else
            strcpy(buf, defopt);
    } else if (!strcmp(optname, "tile_width")) {
        if (iflags.wc_tile_width)
            sprintf(buf, "%d", iflags.wc_tile_width);
        else
            strcpy(buf, defopt);
    } else if (!strcmp(optname, "traps")) {
        sprintf(buf, "%s", to_be_done);
    } else if (!strcmp(optname, "vary_msgcount")) {
        if (iflags.wc_vary_msgcount)
            sprintf(buf, "%d", iflags.wc_vary_msgcount);
        else
            strcpy(buf, defopt);
    }

    if (buf[0])
        return buf;
    else
        return "unknown";
}

int dotogglepickup(void) {
    return 0;
}

void option_help (void) {
    char buf[BUFSZ], buf2[BUFSZ];
    int i;
    winid datawin;

    datawin = create_nhwindow(NHW_TEXT);
    sprintf(buf, "Set options as OPTIONS=<options> in %s", configfile);
    opt_intro[CONFIG_SLOT] = (const char *) buf;
    for (i = 0; opt_intro[i]; i++)
        putstr(datawin, 0, opt_intro[i]);

    /* Boolean options */
    for (i = 0; boolopt[i].name; i++) {
        if (boolopt[i].addr) {
            if (boolopt[i].addr == &iflags.sanity_check && !flags.debug) continue;
            if (boolopt[i].addr == &iflags.menu_tab_sep && !flags.debug) continue;
            next_opt(datawin, boolopt[i].name);
        }
    }
    next_opt(datawin, "");

    /* Compound options */
    putstr(datawin, 0, "Compound options:");
    for (i = 0; compopt[i].name; i++) {
        sprintf(buf2, "`%s'", compopt[i].name);
        sprintf(buf, "%-20s - %s%c", buf2, compopt[i].descr,
                compopt[i+1].name ? ',' : '.');
        putstr(datawin, 0, buf);
    }

    for (i = 0; opt_epilog[i]; i++)
        putstr(datawin, 0, opt_epilog[i]);

    display_nhwindow(datawin, false);
    destroy_nhwindow(datawin);
    return;
}

/*
 * prints the next bool option, on the same line if possible, on a new
 * line if not. End with next_opt("").
 */
void next_opt (winid datawin, const char *str) {
    static char *buf = 0;
    int i;
    char *s;

    if (!buf) *(buf = (char *)malloc(BUFSZ)) = '\0';

    if (!*str) {
        s = eos(buf);
        if (s > &buf[1] && s[-2] == ',')
            strcpy(s - 2, "."); /* replace last ", " */
        i = COLNO;      /* (greater than COLNO - 2) */
    } else {
        i = strlen(buf) + strlen(str) + 2;
    }

    if (i > COLNO - 2) { /* rule of thumb */
        putstr(datawin, 0, buf);
        buf[0] = 0;
    }
    if (*str) {
        strcat(buf, str);
        strcat(buf, ", ");
    } else {
        putstr(datawin, 0, str);
        free(buf),  buf = 0;
    }
    return;
}

/* Returns the fid of the fruit type; if that type already exists, it
 * returns the fid of that one; if it does not exist, it adds a new fruit
 * type to the chain and returns the new one.
 */
int fruitadd (char *str) {
    int i;
    struct fruit *f;
    struct fruit *lastf = 0;
    int highest_fruit_id = 0;
    char buf[PL_FSIZ];
    bool user_specified = (str == pl_fruit);
    /* if not user-specified, then it's a fruit name for a fruit on
     * a bones level...
     */

    /* Note: every fruit has an id (spe for fruit objects) of at least
     * 1; 0 is an error.
     */
    if (user_specified) {
        /* disallow naming after other foods (since it'd be impossible
         * to tell the difference)
         */

        bool found = false, numeric = false;

        for (i = bases[FOOD_CLASS]; objects[i].oc_class == FOOD_CLASS;
                i++) {
            if (!strcmp(OBJ_NAME(objects[i]), pl_fruit)) {
                found = true;
                break;
            }
        }
        {
            char *c;

            c = pl_fruit;

            for(c = pl_fruit; *c >= '0' && *c <= '9'; c++)
                ;
            if (isspace(*c) || *c == 0) numeric = true;
        }
        if (found || numeric ||
                !strncmp(str, "cursed ", 7) ||
                !strncmp(str, "uncursed ", 9) ||
                !strncmp(str, "blessed ", 8) ||
                !strncmp(str, "partly eaten ", 13) ||
                (!strncmp(str, "tin of ", 7) &&
                 (!strcmp(str+7, "spinach") ||
                  name_to_mon(str+7) >= LOW_PM)) ||
                !strcmp(str, "empty tin") ||
                ((!strncmp(eos(str)-7," corpse",7) ||
                  !strncmp(eos(str)-4, " egg",4)) &&
                 name_to_mon(str) >= LOW_PM))
        {
            strcpy(buf, pl_fruit);
            strcpy(pl_fruit, "candied ");
            nmcpy(pl_fruit+8, buf, PL_FSIZ-8);
        }
    }
    for(f=ffruit; f; f = f->nextf) {
        lastf = f;
        if(f->fid > highest_fruit_id) highest_fruit_id = f->fid;
        if(!strncmp(str, f->fname, PL_FSIZ))
            goto nonew;
    }
    /* if adding another fruit would overflow spe, use a random
       fruit instead... we've got a lot to choose from. */
    if (highest_fruit_id >= 127) return rnd(127);
    highest_fruit_id++;
    f = newfruit();
    if (ffruit) lastf->nextf = f;
    else ffruit = f;
    strcpy(f->fname, str);
    f->fid = highest_fruit_id;
    f->nextf = 0;
nonew:
    if (user_specified) current_fruit = highest_fruit_id;
    return f->fid;
}

/*
 * This is a somewhat generic menu for taking a list of NetHack style
 * class choices and presenting them via a description
 * rather than the traditional NetHack characters.
 * (Benefits users whose first exposure to NetHack is via tiles).
 *
 * prompt
 *           The title at the top of the menu.
 *
 * category: 0 = monster class
 *           1 = object  class
 *
 * way
 *           false = PICK_ONE, true = PICK_ANY
 *
 * class_list
 *           a null terminated string containing the list of choices.
 *
 * class_selection
 *           a null terminated string containing the selected characters.
 *
 * Returns number selected.
 */
int choose_classes_menu(const char *prompt, int category, bool way,
        char *class_list, char *class_select)
{
    menu_item *pick_list = (menu_item *)0;
    winid win;
    anything any;
    char buf[BUFSZ];
    int i, n;
    int ret;
    int next_accelerator, accelerator;

    if (class_list == (char *)0 || class_select == (char *)0) return 0;
    accelerator = 0;
    next_accelerator = 'a';
    any.a_void = 0;
    win = create_nhwindow(NHW_MENU);
    start_menu(win);
    while (*class_list) {
        const char *text;
        bool selected;

        text = (char *)0;
        selected = false;
        switch (category) {
            case 0:
                text = monexplain[def_char_to_monclass(*class_list)];
                accelerator = *class_list;
                sprintf(buf, "%s", text);
                break;
            case 1:
                text = objexplain[def_char_to_objclass(*class_list)];
                accelerator = next_accelerator;
                sprintf(buf, "%c  %s", *class_list, text);
                break;
            default:
                impossible("choose_classes_menu: invalid category %d",
                        category);
        }
        if (way && *class_select) {     /* Selections there already */
            if (index(class_select, *class_list)) {
                selected = true;
            }
        }
        any.a_int = *class_list;
        add_menu(win, NO_GLYPH, &any, accelerator,
                category ? *class_list : 0,
                ATR_NONE, buf, selected);
        ++class_list;
        if (category > 0) {
            ++next_accelerator;
            if (next_accelerator == ('z' + 1)) next_accelerator = 'A';
            if (next_accelerator == ('Z' + 1)) break;
        }
    }
    end_menu(win, prompt);
    n = select_menu(win, way ? PICK_ANY : PICK_ONE, &pick_list);
    destroy_nhwindow(win);
    if (n > 0) {
        for (i = 0; i < n; ++i)
            *class_select++ = (char)pick_list[i].item.a_int;
        free((void *)pick_list);
        ret = n;
    } else if (n == -1) {
        class_select = eos(class_select);
        ret = -1;
    } else
        ret = 0;
    *class_select = '\0';
    return ret;
}

/*
 * If a port wants to change or ensure that the
 * SET_IN_FILE, DISP_IN_GAME, or SET_IN_GAME status of an option is
 * correct (for controlling its display in the option menu) call
 * set_option_mod_status()
 * with the second argument of 0,2, or 3 respectively.
 */
void set_option_mod_status (const char *optnam, int status) {
    int k;
    if (status < SET_IN_FILE || status > SET_IN_GAME) {
        impossible("set_option_mod_status: status out of range %d.",
                status);
        return;
    }
    for (k = 0; boolopt[k].name; k++) {
        if (!strncmpi(boolopt[k].name, optnam, strlen(optnam))) {
            boolopt[k].optflags = status;
            return;
        }
    }
    for (k = 0; compopt[k].name; k++) {
        if (!strncmpi(compopt[k].name, optnam, strlen(optnam))) {
            compopt[k].optflags = status;
            return;
        }
    }
}

static bool is_wc_option(const char *optnam) {
    return false;
}

static bool wc_supported(const char *optnam) {
    return false;
}


/*
 * You can set several wc2_options in one call to
 * set_wc2_option_mod_status() by setting
 * the appropriate bits for each option that you
 * are setting in the optmask argument
 * prior to calling.
 *    example: set_wc2_option_mod_status(WC2_FULLSCREEN|WC2_SOFTKEYBOARD|WC2_WRAPTEXT, SET_IN_FILE);
 */

void set_wc2_option_mod_status (unsigned long optmask, int status) {
}

static bool is_wc2_option(const char *optnam) {
    return false;
}

static bool wc2_supported(const char *optnam) {
    return false;
}

static void wc_set_font_name (int wtype, char *fontname) {
    char **fn = (char **)0;
    if (!fontname) return;
    switch(wtype) {
        case NHW_MAP:
            fn = &iflags.wc_font_map;
            break;
        case NHW_MESSAGE:
            fn = &iflags.wc_font_message;
            break;
        case NHW_TEXT:
            fn = &iflags.wc_font_text;
            break;
        case NHW_MENU:
            fn = &iflags.wc_font_menu;
            break;
        case NHW_STATUS:
            fn = &iflags.wc_font_status;
            break;
        default:
            return;
    }
    if (fn) {
        if (*fn) free(*fn);
        *fn = (char *)malloc(strlen(fontname) + 1);
        strcpy(*fn, fontname);
    }
    return;
}

static int wc_set_window_colors (char *op) {
    /* syntax:
     *  menu white/black message green/yellow status white/blue text white/black
     */

    int j;
    char buf[BUFSZ];
    char *wn, *tfg, *tbg, *newop;
    static const char *wnames[] = { "menu", "message", "status", "text" };
    static const char *shortnames[] = { "mnu", "msg", "sts", "txt" };
    static char **fgp[] = {
        &iflags.wc_foregrnd_menu,
        &iflags.wc_foregrnd_message,
        &iflags.wc_foregrnd_status,
        &iflags.wc_foregrnd_text
    };
    static char **bgp[] = {
        &iflags.wc_backgrnd_menu,
        &iflags.wc_backgrnd_message,
        &iflags.wc_backgrnd_status,
        &iflags.wc_backgrnd_text
    };

    strcpy(buf, op);
    newop = mungspaces(buf);
    while (newop && *newop) {

        wn = tfg = tbg = (char *)0;

        /* until first non-space in case there's leading spaces - before colorname*/
        while(*newop && isspace(*newop)) newop++;
        if (*newop) wn = newop;
        else return 0;

        /* until first space - colorname*/
        while(*newop && !isspace(*newop)) newop++;
        if (*newop) *newop = '\0';
        else return 0;
        newop++;

        /* until first non-space - before foreground*/
        while(*newop && isspace(*newop)) newop++;
        if (*newop) tfg = newop;
        else return 0;

        /* until slash - foreground */
        while(*newop && *newop != '/') newop++;
        if (*newop) *newop = '\0';
        else return 0;
        newop++;

        /* until first non-space (in case there's leading space after slash) - before background */
        while(*newop && isspace(*newop)) newop++;
        if (*newop) tbg = newop;
        else return 0;

        /* until first space - background */
        while(*newop && !isspace(*newop)) newop++;
        if (*newop) *newop++ = '\0';

        for (j = 0; j < 4; ++j) {
            if (!strcmpi(wn, wnames[j]) ||
                    !strcmpi(wn, shortnames[j])) {
                if (tfg && !strstri(tfg, " ")) {
                    if (*fgp[j]) free(*fgp[j]);
                    *fgp[j] = (char *)malloc(strlen(tfg) + 1);
                    strcpy(*fgp[j], tfg);
                }
                if (tbg && !strstri(tbg, " ")) {
                    if (*bgp[j]) free(*bgp[j]);
                    *bgp[j] = (char *)malloc(strlen(tbg) + 1);
                    strcpy(*bgp[j], tbg);
                }
                break;
            }
        }
    }
    return 1;
}
