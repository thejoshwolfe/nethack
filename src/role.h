#ifndef ROLE_H
#define ROLE_H

bool validrole(int);
bool validrace(int, int);
bool validgend(int, int, int);
bool validalign(int, int, int);
int randrole(void);
int randrace(int);
int randgend(int, int);
int randalign(int, int);
int str2role(char *);
int str2race(char *);
int str2gend(char *);
int str2align(char *);
bool ok_role(int, int, int, int);
int pick_role(int, int, int, int);
bool ok_race(int, int, int, int);
int pick_race(int, int, int, int);
bool ok_gend(int, int, int, int);
int pick_gend(int, int, int, int);
bool ok_align(int, int, int, int);
int pick_align(int, int, int, int);
void role_init(void);
void rigid_role_checks(void);
void plnamesuffix(void);
const char *Hello(struct monst *);
const char *Goodbye(void);
char *build_plselection_prompt(char *, int, int, int, int, int);
char *root_plselection_prompt(char *, int, int, int, int, int);

#endif
