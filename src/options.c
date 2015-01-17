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

static char def_inv_order[MAXOCLASSES] = {
    COIN_CLASS, AMULET_CLASS, WEAPON_CLASS, ARMOR_CLASS, FOOD_CLASS,
    SCROLL_CLASS, SPBOOK_CLASS, POTION_CLASS, RING_CLASS, WAND_CLASS,
    TOOL_CLASS, GEM_CLASS, ROCK_CLASS, BALL_CLASS, CHAIN_CLASS, 0,
};

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

static void nmcpy(char *dest, const char *src, int maxlen) {
    int count;

    for (count = 1; count < maxlen; count++) {
        if (*src == ',' || *src == '\0')
            break; /*exit on \0 terminator*/
        *dest++ = *src++;
    }
    *dest = 0;
}

void initoptions(void) {
    char *opts;
    int i;

    /* initialize the random number generator */
    setrandom();

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
        oc_syms[i] = (unsigned char)def_oc_syms[i];
    for (i = 0; i < MAXMCLASSES; i++)
        monsyms[i] = (unsigned char)def_monsyms[i];
    for (i = 0; i < WARNCOUNT; i++)
        warnsyms[i] = def_warnsyms[i].sym;
    iflags.bouldersym = 0;
    flags.warntype = 0L;

    /* assert( sizeof flags.inv_order == sizeof def_inv_order ); */
    (void)memcpy((void *)flags.inv_order, (void *)def_inv_order, sizeof flags.inv_order);
    flags.pickup_burden = MOD_ENCUMBER;

    iflags.sortloot = 'n';

    for (i = 0; i < NUM_DISCLOSURE_OPTIONS; i++)
        flags.end_disclose[i] = DISCLOSE_PROMPT_DEFAULT_NO;

    /* since this is done before init_objects(), do partial init here */
    objects[SLIME_MOLD].oc_name_idx = SLIME_MOLD;
    nmcpy(pl_fruit, OBJ_NAME(objects[SLIME_MOLD]), PL_FSIZ);

    (void)fruitadd(pl_fruit);
    /* Remove "slime mold" from list of object names; this will     */
    /* prevent it from being wished unless it's actually present    */
    /* as a named (or default) fruit.  Wishing for "fruit" will     */
    /* result in the player's preferred fruit [better than "\033"]. */
    objects[SLIME_MOLD].oc_name = "fruit";
}

/* Returns the fid of the fruit type; if that type already exists, it
 * returns the fid of that one; if it does not exist, it adds a new fruit
 * type to the chain and returns the new one.
 */
int fruitadd(char *str) {
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

        for (i = bases[FOOD_CLASS]; objects[i].oc_class == FOOD_CLASS; i++) {
            if (!strcmp(OBJ_NAME(objects[i]), pl_fruit)) {
                found = true;
                break;
            }
        }
        {
            char *c;

            c = pl_fruit;

            for (c = pl_fruit; *c >= '0' && *c <= '9'; c++)
                ;
            if (isspace(*c) || *c == 0)
                numeric = true;
        }
        if (found || numeric || !strncmp(str, "cursed ", 7) || !strncmp(str, "uncursed ", 9) || !strncmp(str, "blessed ", 8) || !strncmp(str, "partly eaten ", 13) || (!strncmp(str, "tin of ", 7) && (!strcmp(str + 7, "spinach") || name_to_mon(str + 7) >= LOW_PM)) || !strcmp(str, "empty tin") || ((!strncmp(eos(str) - 7, " corpse", 7) || !strncmp(eos(str) - 4, " egg", 4)) && name_to_mon(str) >= LOW_PM)) {
            strcpy(buf, pl_fruit);
            strcpy(pl_fruit, "candied ");
            nmcpy(pl_fruit + 8, buf, PL_FSIZ - 8);
        }
    }
    for (f = ffruit; f; f = f->nextf) {
        lastf = f;
        if (f->fid > highest_fruit_id)
            highest_fruit_id = f->fid;
        if (!strncmp(str, f->fname, PL_FSIZ))
            goto nonew;
    }
    /* if adding another fruit would overflow spe, use a random
     fruit instead... we've got a lot to choose from. */
    if (highest_fruit_id >= 127)
        return rnd(127);
    highest_fruit_id++;
    f = newfruit();
    if (ffruit)
        lastf->nextf = f;
    else
        ffruit = f;
    strcpy(f->fname, str);
    f->fid = highest_fruit_id;
    f->nextf = 0;
    nonew: if (user_specified)
        current_fruit = highest_fruit_id;
    return f->fid;
}

