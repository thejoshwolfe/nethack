/* See LICENSE in the root of this project for change info */
#ifndef FUNC_TAB_H
#define FUNC_TAB_H

#include "global.h"

struct func_tab {
        char f_char;
        boolean can_if_buried;
        int (*f_funct)(void);
        const char *f_text;
};

struct ext_func_tab {
        const char *ef_txt, *ef_desc;
        int (*ef_funct)(void);
        boolean can_if_buried;
};

extern struct ext_func_tab extcmdlist[];

#endif /* FUNC_TAB_H */
