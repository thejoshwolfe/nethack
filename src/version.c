/* See LICENSE in the root of this project for change info */

#include <stdlib.h>
#include <string.h>

#include "date.h"
#include "global.h"
#include "hack.h"
#include "patchlevel.h"
#include "version.h"

/* fill buffer with short version (so caller can avoid including date.h) */
char * version_string(char *buf) {
    return strcpy(buf, VERSION_STRING);
}

/* fill and return the given buffer with the long nethack version string */
char * getversionstring(char *buf) {
    strcpy(buf, VERSION_ID);
    return buf;
}

int doversion(void) {
    return 0;
}

int doextversion(void) {
    return 0;
}

bool check_version(struct version_info *version_data, const char *filename, bool complain) {
    return true;
}

bool uptodate(int fd, const char *name) {
    return true;
}
void store_version(int fd) {
    // i'll get right on that
}

unsigned long get_feature_notice_ver(char *str) {
    char buf[BUFSZ];
    int ver_maj, ver_min, patch;
    char *istr[3];
    int j = 0;

    if (!str)
        return 0L;
    str = strcpy(buf, str);
    istr[j] = str;
    while (*str) {
        if (*str == '.') {
            *str++ = '\0';
            j++;
            istr[j] = str;
            if (j == 2)
                break;
        } else if (index("0123456789", *str) != 0) {
            str++;
        } else
            return 0L;
    }
    if (j != 2)
        return 0L;
    ver_maj = atoi(istr[0]);
    ver_min = atoi(istr[1]);
    patch = atoi(istr[2]);
    return FEATURE_NOTICE_VER(ver_maj, ver_min, patch);
}

unsigned long get_current_feature_ver(void) {
    return FEATURE_NOTICE_VER(VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL);
}
