/* See LICENSE in the root of this project for change info */

#include "decl.h"
#include "global.h"
#include "rip.h"
#include "hack.h"

#include <string.h>

extern const char * const killed_by_prefix[];   /* from topten.c */

/* A normal tombstone for end of game display. */
static const char *rip_txt[] = {
"                       ----------",
"                      /          \\",
"                     /    REST    \\",
"                    /      IN      \\",
"                   /     PEACE      \\",
"                  /                  \\",
"                  |                  |", /* Name of player */
"                  |                  |", /* Amount of $ */
"                  |                  |", /* Type of death */
"                  |                  |", /* . */
"                  |                  |", /* . */
"                  |                  |", /* . */
"                  |       1001       |", /* Real year of death */
"                 *|     *  *  *      | *",
"        _________)/\\\\_//(\\/(/\\)/\\//\\/|_)_______",
0
};
#define STONE_LINE_CENT 28      /* char[] element of center of stone face */
#define STONE_LINE_LEN 16       /* # chars that fit on one line
                                 * (note 1 ' ' border)
                                 */
#define NAME_LINE 6             /* *char[] line # for player name */
#define GOLD_LINE 7             /* *char[] line # for amount of gold */
#define DEATH_LINE 8            /* *char[] line # for death description */
#define YEAR_LINE 12            /* *char[] line # for year */

static char **rip;

static void center (int line, char *text) {
    char *ip,*op;
    ip = text;
    op = &rip[line][STONE_LINE_CENT - ((strlen(text)+1)>>1)];
    while(*ip) *op++ = *ip++;
}


void genl_outrip(winid tmpwin, int how) {
    fprintf(stderr, "TODO: genl_outrip\n");
}
