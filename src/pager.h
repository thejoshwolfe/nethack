#ifndef PAGER_H
#define PAGER_H

int dowhatis(void);
int doquickwhatis(void);
int doidtrap(void);
int dowhatdoes(void);
char *dowhatdoes_core(char, char *);
int dohelp(void);
int dohistory(void);

#endif // PAGER_H
