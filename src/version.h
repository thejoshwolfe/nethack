#ifndef VERSION_H
#define VERSION_H

#include <stdbool.h>

char *version_string(char *);
char *getversionstring(char *);
int doversion(void);
int doextversion(void);
bool check_version(struct version_info *, const char *,bool);
unsigned long get_feature_notice_ver(char *);
unsigned long get_current_feature_ver(void);

#endif // VERSION_H
