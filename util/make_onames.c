/* See LICENSE in the root of this project for change info */

#include "objclass.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static char temp[32];
static const char *Dont_Edit_Code = "// This source file is auto-generated by 'make_onames'\n";

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
    int i, sum = 0;
    char *c, *objnam;
    int nspell = 0;
    int prefix = 0;
    char class = '\0';

    char *dest = argv[1];

    FILE *f = fopen(dest, "w");
    if (!f) {
        fprintf(stderr, "Unable to open %s\n", dest);
        return -1;
    }

    fprintf(f, "%s\n", Dont_Edit_Code);
    fprintf(f, "#ifndef ONAMES_H\n#define ONAMES_H\n\n");

    fprintf(f, "enum {\n");

    for(i = 0; !i || objects[i].oc_class != ILLOBJ_CLASS; i++) {
        objects[i].oc_name_idx = objects[i].oc_descr_idx = i;   /* init */
        if (!(objnam = tmpdup(OBJ_NAME(objects[i])))) continue;

        /* make sure probabilities add up to 1000 */
        if(objects[i].oc_class != class) {
            if (sum && sum != 1000) {
                fprintf(stderr, "prob error for class %d (%d%%)", class, sum);
                return -1;
            }
            class = objects[i].oc_class;
            sum = 0;
        }

        for (c = objnam; *c; c++)
            if (*c >= 'a' && *c <= 'z') *c -= (char)('a' - 'A');
            else if (*c < 'A' || *c > 'Z') *c = '_';

        switch (class) {
            case WAND_CLASS:
                fprintf(f, "    WAN_"); prefix = 1; break;
            case RING_CLASS:
                fprintf(f, "    RIN_"); prefix = 1; break;
            case POTION_CLASS:
                fprintf(f, "    POT_"); prefix = 1; break;
            case SPBOOK_CLASS:
                fprintf(f, "    SPE_"); prefix = 1; nspell++; break;
            case SCROLL_CLASS:
                fprintf(f, "    SCR_"); prefix = 1; break;
            case AMULET_CLASS:
                /* avoid trouble with stupid C preprocessors */
                fprintf(f, "    ");
                if(objects[i].oc_material == PLASTIC) {
                    fprintf(f, "FAKE_AMULET_OF_YENDOR = %d,\n", i);
                    prefix = -1;
                    break;
                }
                break;
            default:
                fprintf(f, "    ");
        }
        if (prefix >= 0)
            fprintf(f, "%s = %d,\n", limit(objnam, prefix), i);
        prefix = 0;

        sum += objects[i].oc_prob;
    }

    /* check last set of probabilities */
    if (sum && sum != 1000) {
        fprintf(stderr, "prob error for class %d (%d%%)", class, sum);
        return -1;
    }

    fprintf(f, "    LAST_GEM = (JADE),\n");
    fprintf(f, "    MAXSPELL = %d,\n", nspell+1);
    fprintf(f, "    NUM_OBJECTS = %d,\n", i);
    fprintf(f, "};\n");

    fprintf(f, "\n#endif /* ONAMES_H */\n");
    fclose(f);
    return 0;
}
