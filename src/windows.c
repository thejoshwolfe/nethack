/* See LICENSE in the root of this project for change info */

#include "hack.h"
#include "wintty.h"
#include "config.h"
#include "extern.h"
#include "winprocs.h"

static void def_raw_print(const char *s);

struct window_procs windowprocs;

static
struct win_choices {
    struct window_procs *procs;
    void (*ini_routine)(void);          /* optional (can be 0) */
} winchoices[] = {
    { &tty_procs, win_tty_init },
    { 0, 0 }            /* must be last */
};

static void def_raw_print(const char *s) {
    puts(s);
}

void choose_windows(const char *s) {
    char *ow; const char *wt;
    int i;

    if (!strcmp(s, "sys") && (ow = getenv("OVERRIDEWIN")))
      wt = ow;
    else
      wt = s;

    for(i=0; winchoices[i].procs; i++)
        if (!strcmpi(wt, winchoices[i].procs->name)) {
            windowprocs = *winchoices[i].procs;
            if (winchoices[i].ini_routine) (*winchoices[i].ini_routine)();
            return;
        }

    if (!windowprocs.win_raw_print)
        windowprocs.win_raw_print = def_raw_print;

    raw_printf("Window type %s not recognized.  Choices are:", s);
    for(i=0; winchoices[i].procs; i++)
        raw_printf("        %s", winchoices[i].procs->name);

    if (windowprocs.win_raw_print == def_raw_print)
        terminate(EXIT_SUCCESS);
    wait_synch();
}

/*
 * tty_message_menu() provides a means to get feedback from the
 * --More-- prompt; other interfaces generally don't need that.
 */
/*ARGSUSED*/
char
genl_message_menu (char let, int how, const char *mesg)
{
    pline("%s", mesg);
    return 0;
}

/*ARGSUSED*/
void
genl_preference_update (const char *pref)
{
        /* window ports are expected to provide
           their own preference update routine
           for the preference capabilities that
           they support.
           Just return in this genl one. */
}
