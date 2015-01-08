#ifndef QUESTPGR_H
#define QUESTPGR_H

#include <stdbool.h>

#include "obj.h"
#include "permonst.h"

void load_qtlist(void);
void unload_qtlist(void);
short quest_info(int);
const char *ldrname(void);
bool is_quest_artifact(struct obj*);
void com_pager(int);
void qt_pager(int);
struct permonst *qt_montype(void);

#endif // QUESTPGR_H
