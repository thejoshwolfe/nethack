/* See LICENSE in the root of this project for change info */

#include "pline.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decl.h"
#include "do_name.h"
#include "edog.h"
#include "end.h"
#include "epri.h"
#include "flag.h"
#include "global.h"
#include "hack.h"
#include "hacklib.h"
#include "mondata.h"
#include "monst.h"
#include "objnam.h"
#include "permonst.h"
#include "pm.h"
#include "polyself.h"
#include "prop.h"
#include "util.h"
#include "vision.h"
#include "worn.h"
#include "you.h"
#include "youprop.h"

static bool no_repeat = false;

static char *You_buf(int);

static void vpline(const char *line, va_list the_args) {
    char pbuf[BUFSZ];
    /* Do NOT use va_start and va_start in here... see above */

    if (!line || !*line) return;
    nh_vslprintf(pbuf, BUFSZ, line, the_args);
    plines(pbuf);
}

void pline(const char * line, ...) {
    va_list the_args;
    va_start(the_args, line);
    vpline(line, the_args);
    va_end(the_args);
}

void plines(const char *line) {
    if (!line || !*line) return;
    if (!iflags.window_inited) {
        fprintf(stderr, "%s\n", line);
        return;
    }
    if (no_repeat && !strcmp(line, toplines))
        return;
    if (vision_full_recalc)
        vision_recalc(0);
    putstr(WIN_MESSAGE, 0, line);
}

void Norep (const char * line, ...) {
    va_list the_args;
    va_start(the_args, line);
    no_repeat = true;
    vpline(line, the_args);
    no_repeat = false;
    va_end(the_args);
    return;
}

/* work buffer for You(), &c and verbalize() */
static char *you_buf = 0;
static int you_buf_siz = 0;

static char * You_buf (int siz) {
        if (siz > you_buf_siz) {
                if (you_buf) free((void *) you_buf);
                you_buf_siz = siz + 10;
                you_buf = (char *) malloc((unsigned) you_buf_siz);
        }
        return you_buf;
}

void free_youbuf (void) {
        if (you_buf) free((void *) you_buf),  you_buf = (char *)0;
        you_buf_siz = 0;
}

/* `prefix' must be a string literal, not a pointer */
#define YouPrefix(pointer,prefix,text) \
 strcpy((pointer = You_buf((int)(strlen(text) + sizeof prefix))), prefix)

#define YouMessage(pointer,prefix,text) \
 strcat((YouPrefix(pointer, prefix, text), pointer), text)

void You (const char * line, ...) {
    va_list the_args;
    char *tmp;
    va_start(the_args, line);
    vpline(YouMessage(tmp, "You ", line), the_args);
    va_end(the_args);
}

void Your (const char * line, ...) {
    va_list the_args;
        char *tmp;
        va_start(the_args, line);
        vpline(YouMessage(tmp, "Your ", line), the_args);
    va_end(the_args);
}

void You_feel (const char * line, ...) {
    va_list the_args;
        char *tmp;
        va_start(the_args, line);
        vpline(YouMessage(tmp, "You feel ", line), the_args);
    va_end(the_args);
}

void You_cant (const char * line, ...) {
    va_list the_args;
        char *tmp;
        va_start(the_args, line);
        vpline(YouMessage(tmp, "You can't ", line), the_args);
    va_end(the_args);
}

void pline_The (const char * line, ...) {
    va_list the_args;
        char *tmp;
        va_start(the_args, line);
        vpline(YouMessage(tmp, "The ", line), the_args);
    va_end(the_args);
}

void There (const char * line, ...) {
    va_list the_args;
        char *tmp;
        va_start(the_args, line);
        vpline(YouMessage(tmp, "There ", line), the_args);
    va_end(the_args);
}

void You_hear (const char * line, ...) {
    va_list the_args;
    char *tmp;
    va_start(the_args, line);
    if (Underwater)
        YouPrefix(tmp, "You barely hear ", line);
    else if (u.usleep)
        YouPrefix(tmp, "You dream that you hear ", line);
    else
        YouPrefix(tmp, "You hear ", line);
    vpline(strcat(tmp, line), the_args);
    va_end(the_args);
}

void verbalize (const char *line, ...) {
    va_list the_args;
    char *tmp;
    if (!flags.soundok) return;
    va_start(the_args, line);
    tmp = You_buf((int)strlen(line) + sizeof "\"\"");
    strcpy(tmp, "\"");
    strcat(tmp, line);
    strcat(tmp, "\"");
    vpline(tmp, the_args);
    va_end(the_args);
}

static void vraw_printf(const char *,va_list);

void raw_printf (const char * line, ...) {
    va_list the_args;
        va_start(the_args, line);
        vraw_printf(line, the_args);
    va_end(the_args);
}

static void vraw_printf(const char *line, va_list the_args) {
    /* Do NOT use va_start and va_end in here... see above */

    if (!index(line, '%')) {
        fprintf(stderr, "%s\n", line);
    } else {
        char pbuf[BUFSZ];
        vsprintf(pbuf,line,the_args);
        fprintf(stderr, "%s\n", pbuf);
    }
}


void impossible (const char * s, ...) {
    va_list the_args;
    va_start(the_args, s);
    if (program_state.in_impossible)
        panic("impossible called impossible");
    program_state.in_impossible = 1;
    vpline(s,the_args);
    pline("Program in disorder - perhaps you'd better #quit.");
    program_state.in_impossible = 0;
    va_end(the_args);
}

const char * align_str (aligntyp alignment) {
    switch ((int)alignment) {
        case A_CHAOTIC: return "chaotic";
        case A_NEUTRAL: return "neutral";
        case A_LAWFUL:  return "lawful";
        case A_NONE:    return "unaligned";
    }
    return "unknown";
}

void mstatusline (struct monst *mtmp) {
    aligntyp alignment;
    char info[BUFSZ], monnambuf[BUFSZ];

    if (mtmp->ispriest || mtmp->data == &mons[PM_ALIGNED_PRIEST]
            || mtmp->data == &mons[PM_ANGEL])
        alignment = EPRI(mtmp)->shralign;
    else
        alignment = mtmp->data->maligntyp;
    alignment = (alignment > 0) ? A_LAWFUL :
        (alignment < 0) ? A_CHAOTIC :
        A_NEUTRAL;

    info[0] = 0;
    if (mtmp->mtame) {        strcat(info, ", tame");
        if (flags.debug) {
            sprintf(eos(info), " (%d", mtmp->mtame);
            if (!mtmp->isminion)
                sprintf(eos(info), "; hungry %ld; apport %d",
                        EDOG(mtmp)->hungrytime, EDOG(mtmp)->apport);
            strcat(info, ")");
        }
    }
    else if (mtmp->mpeaceful) strcat(info, ", peaceful");
    if (mtmp->meating)        strcat(info, ", eating");
    if (mtmp->mcan)           strcat(info, ", cancelled");
    if (mtmp->mconf)          strcat(info, ", confused");
    if (mtmp->mblinded || !mtmp->mcansee)
        strcat(info, ", blind");
    if (mtmp->mstun)          strcat(info, ", stunned");
    if (mtmp->msleeping)      strcat(info, ", asleep");
    else if (mtmp->mfrozen || !mtmp->mcanmove)
        strcat(info, ", can't move");
    /* [arbitrary reason why it isn't moving] */
    else if (mtmp->mstrategy & STRAT_WAITMASK)
        strcat(info, ", meditating");
    else if (mtmp->mflee)     strcat(info, ", scared");
    if (mtmp->mtrapped)       strcat(info, ", trapped");
    if (mtmp->mspeed)         strcat(info,
            mtmp->mspeed == MFAST ? ", fast" :
            mtmp->mspeed == MSLOW ? ", slow" :
            ", ???? speed");
    if (mtmp->mundetected)    strcat(info, ", concealed");
    if (mtmp->minvis)         strcat(info, ", invisible");
    if (mtmp == u.ustuck)     strcat(info,
            (sticks(youmonst.data)) ? ", held by you" :
            u.uswallow ? (is_animal(u.ustuck->data) ?
                ", swallowed you" :
                ", engulfed you") :
            ", holding you");
    if (mtmp == u.usteed)     strcat(info, ", carrying you");

    /* avoid "Status of the invisible newt ..., invisible" */
    /* and unlike a normal mon_nam, use "saddled" even if it has a name */
    char name[BUFSZ];
    x_monnam(name, BUFSZ, mtmp, ARTICLE_THE, NULL, (SUPPRESS_IT|SUPPRESS_INVISIBLE), false);
    strcpy(monnambuf, name);

    pline("Status of %s (%s):  Level %d  HP %d(%d)  AC %d%s.",
            monnambuf,
            align_str(alignment),
            mtmp->m_lev,
            mtmp->mhp,
            mtmp->mhpmax,
            find_mac(mtmp),
            info);
}

void
ustatusline (void)
{
        char info[BUFSZ];

        info[0] = '\0';
        if (Sick) {
                strcat(info, ", dying from");
                if (u.usick_type & SICK_VOMITABLE)
                        strcat(info, " food poisoning");
                if (u.usick_type & SICK_NONVOMITABLE) {
                        if (u.usick_type & SICK_VOMITABLE)
                                strcat(info, " and");
                        strcat(info, " illness");
                }
        }
        if (Stoned)             strcat(info, ", solidifying");
        if (Slimed)             strcat(info, ", becoming slimy");
        if (Strangled)          strcat(info, ", being strangled");
        if (Vomiting)           strcat(info, ", nauseated"); /* !"nauseous" */
        if (Confusion())          strcat(info, ", confused");
        if (Blind) {
            strcat(info, ", blind");
            if (u.ucreamed) {
                if ((long)u.ucreamed < Blinded || Blindfolded
                                                || !haseyes(youmonst.data))
                    strcat(info, ", cover");
                strcat(info, "ed by sticky goop");
            }   /* note: "goop" == "glop"; variation is intentional */
        }
        if (Stunned())            strcat(info, ", stunned");
        if (!u.usteed)
        if (Wounded_legs) {
            const char *what = body_part(LEG);
            if ((Wounded_legs & BOTH_SIDES) == BOTH_SIDES)
                what = makeplural(what);
                                sprintf(eos(info), ", injured %s", what);
        }
        if (Glib)               sprintf(eos(info), ", slippery %s",
                                        makeplural(body_part(HAND)));
        if (u.utrap)            strcat(info, ", trapped");
        if (Fast)               strcat(info, Very_fast ?
                                                ", very fast" : ", fast");
        if (u.uundetected)      strcat(info, ", concealed");
        if (Invis)              strcat(info, ", invisible");
        if (u.ustuck) {
            if (sticks(youmonst.data))
                strcat(info, ", holding ");
            else
                strcat(info, ", held by ");
            char name[BUFSZ];
            mon_nam(name, BUFSZ, u.ustuck);
            strcat(info, name);
        }

        pline("Status of %s (%s%s):  Level %d  HP %d(%d)  AC %d%s.",
                plname,
                    (u.ualign.record >= 20) ? "piously " :
                    (u.ualign.record > 13) ? "devoutly " :
                    (u.ualign.record > 8) ? "fervently " :
                    (u.ualign.record > 3) ? "stridently " :
                    (u.ualign.record == 3) ? "" :
                    (u.ualign.record >= 1) ? "haltingly " :
                    (u.ualign.record == 0) ? "nominally " :
                                            "insufficiently ",
                align_str(u.ualign.type),
                Upolyd ? mons[u.umonnum].mlevel : u.ulevel,
                Upolyd ? u.mh : u.uhp,
                Upolyd ? u.mhmax : u.uhpmax,
                u.uac,
                info);
}

void
self_invis_message (void)
{
        pline("%s %s.",
            Hallucination() ? "Far out, man!  You" : "Gee!  All of a sudden, you",
            See_invisible ? "can see right through yourself" :
                "can't see yourself");
}

/*pline.c*/
