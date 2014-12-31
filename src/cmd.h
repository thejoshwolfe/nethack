#ifndef CMD_H
#define CMD_H

#include "coord.h"

void reset_occupations(void);
void set_occupation(int (*)(void),const char *,int);
char pgetchar(void);
void pushch(char);
void savech(char);
void add_debug_extended_commands(void);
void rhack(char *);
int doextlist(void);
int extcmd_via_menu(void);
void enlightenment(int);
void show_conduct(int);
void dump_enlightenment(int);
void dump_conduct(int);
int xytod(signed char,signed char);
void dtoxy(coord *,int);
int movecmd(char);
int getdir(const char *);
void confdir(void);
int isok(int,int);
int get_adjacent_loc(const char *, const char *, signed char, signed char, coord *);
const char *click_to_cmd(int,int,int);
char readchar(void);
void sanity_check(void);
char yn_function(const char *, const char *, char);

#endif //CMD_H
