/* See LICENSE in the root of this project for change info */

#include "artilist.h"

#include <stdio.h>
#include <string.h>

static const char *Dont_Edit_Code = "// This source file is auto-generated by 'make_artifact_names'\n";
static char temp[32];

static char * tmpdup (const char *str) {
    static char buf[128];

    if (!str)
        return NULL;

    strncpy(buf, str, 127);
    return buf;
}

// limit a name to 30 characters length
static char * limit (char *name, int pref) {
    strncpy(temp, name, pref ? 26 : 30);
    temp[pref ? 26 : 30] = 0;
    return temp;
}

int main(int argc, char *argv[]) {
    char *dest = argv[1];

    FILE *f = fopen(dest, "w");
    if (!f) {
        fprintf(stderr, "Unable to open %s\n", dest);
        return -1;
    }

    fprintf(f, "%s\n", Dont_Edit_Code);
    fprintf(f, "#ifndef ARTIFACT_NAMES_H\n");
    fprintf(f, "#define ARTIFACT_NAMES_H\n\n");

    fprintf(f, "enum {\n");
    int i;
    char *c;
    char *objnam;
    for (i = 1; artilist[i].otyp; i += 1) {
        for (c = objnam = tmpdup(artilist[i].name); *c; c++)
            if (*c >= 'a' && *c <= 'z') *c -= (char)('a' - 'A');
            else if (*c < 'A' || *c > 'Z') *c = '_';

        if (!strncmp(objnam, "THE_", 4))
            objnam += 4;
        /* fudge _platinum_ YENDORIAN EXPRESS CARD */
        if (!strncmp(objnam, "PLATINUM_", 9))
            objnam += 9;
        fprintf(f,"    ART_%s = %d,\n", limit(objnam, 1), i);
    }

    fprintf(f, "    NROFARTIFACTS = %d,\n", i-1);
    fprintf(f, "};\n");

    fprintf(f,"\n#endif /* ARTIFACT_NAMES_H */\n");
    fclose(f);
    return 0;
}
