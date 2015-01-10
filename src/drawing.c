/* See LICENSE in the root of this project for change info */

#include "drawing.h"

#include "color.h"
#include "decl.h"
#include "hack.h"
#include "monsym.h"
#include "objclass.h"
#include "rm.h"

/* Relevent header information in rm.h and objclass.h. */

#define g_FILLER(symbol) 0

unsigned char oc_syms[MAXOCLASSES] = DUMMY; /* the current object  display symbols */
unsigned char showsyms[MAXPCHARS]  = DUMMY; /* the current feature display symbols */
unsigned char monsyms[MAXMCLASSES] = DUMMY; /* the current monster display symbols */
unsigned char warnsyms[WARNCOUNT]  = DUMMY;  /* the current warning display symbols */

/* Default object class symbols.  See objclass.h. */
const char def_oc_syms[MAXOCLASSES] = {
/* 0*/  '\0',           /* placeholder for the "random class" */
        ILLOBJ_SYM,
        WEAPON_SYM,
        ARMOR_SYM,
        RING_SYM,
/* 5*/  AMULET_SYM,
        TOOL_SYM,
        FOOD_SYM,
        POTION_SYM,
        SCROLL_SYM,
/*10*/  SPBOOK_SYM,
        WAND_SYM,
        GOLD_SYM,
        GEM_SYM,
        ROCK_SYM,
/*15*/  BALL_SYM,
        CHAIN_SYM,
        VENOM_SYM
};

const char invisexplain[] = "remembered, unseen, creature";

/* Object descriptions.  Used in do_look(). */
const char * const objexplain[] = {     /* these match def_oc_syms, above */
/* 0*/  0,
        "strange object",
        "weapon",
        "suit or piece of armor",
        "ring",
/* 5*/  "amulet",
        "useful item (pick-axe, key, lamp...)",
        "piece of food",
        "potion",
        "scroll",
/*10*/  "spellbook",
        "wand",
        "pile of coins",
        "gem or rock",
        "boulder or statue",
/*15*/  "iron ball",
        "iron chain",
        "splash of venom"
};

/* Object class names.  Used in object_detect(). */
const char * const oclass_names[] = {
/* 0*/  0,
        "illegal objects",
        "weapons",
        "armor",
        "rings",
/* 5*/  "amulets",
        "tools",
        "food",
        "potions",
        "scrolls",
/*10*/  "spellbooks",
        "wands",
        "coins",
        "rocks",
        "large stones",
/*15*/  "iron balls",
        "chains",
        "venoms"
};

/* Default monster class symbols.  See monsym.h. */
const char def_monsyms[MAXMCLASSES] = {
        '\0',           /* holder */
        DEF_ANT,
        DEF_BLOB,
        DEF_COCKATRICE,
        DEF_DOG,
        DEF_EYE,
        DEF_FELINE,
        DEF_GREMLIN,
        DEF_HUMANOID,
        DEF_IMP,
        DEF_JELLY,              /* 10 */
        DEF_KOBOLD,
        DEF_LEPRECHAUN,
        DEF_MIMIC,
        DEF_NYMPH,
        DEF_ORC,
        DEF_PIERCER,
        DEF_QUADRUPED,
        DEF_RODENT,
        DEF_SPIDER,
        DEF_TRAPPER,            /* 20 */
        DEF_UNICORN,
        DEF_VORTEX,
        DEF_WORM,
        DEF_XAN,
        DEF_LIGHT,
        DEF_ZRUTY,
        DEF_ANGEL,
        DEF_BAT,
        DEF_CENTAUR,
        DEF_DRAGON,             /* 30 */
        DEF_ELEMENTAL,
        DEF_FUNGUS,
        DEF_GNOME,
        DEF_GIANT,
        '\0',
        DEF_JABBERWOCK,
        DEF_KOP,
        DEF_LICH,
        DEF_MUMMY,
        DEF_NAGA,               /* 40 */
        DEF_OGRE,
        DEF_PUDDING,
        DEF_QUANTMECH,
        DEF_RUSTMONST,
        DEF_SNAKE,
        DEF_TROLL,
        DEF_UMBER,
        DEF_VAMPIRE,
        DEF_WRAITH,
        DEF_XORN,               /* 50 */
        DEF_YETI,
        DEF_ZOMBIE,
        DEF_HUMAN,
        DEF_GHOST,
        DEF_GOLEM,
        DEF_DEMON,
        DEF_EEL,
        DEF_LIZARD,
        DEF_WORM_TAIL,
        DEF_MIMIC_DEF,          /* 60 */
};

/* The explanations below are also used when the user gives a string
 * for blessed genocide, so no text should wholly contain any later
 * text.  They should also always contain obvious names (eg. cat/feline).
 */
const char * const monexplain[MAXMCLASSES] = {
    0,
    "ant or other insect",      "blob",                 "cockatrice",
    "dog or other canine",      "eye or sphere",        "cat or other feline",
    "gremlin",                  "humanoid",             "imp or minor demon",
    "jelly",                    "kobold",               "leprechaun",
    "mimic",                    "nymph",                "orc",
    "piercer",                  "quadruped",            "rodent",
    "arachnid or centipede",    "trapper or lurker above", "unicorn or horse",
    "vortex",           "worm", "xan or other mythical/fantastic insect",
    "light",                    "zruty",

    "angelic being",            "bat or bird",          "centaur",
    "dragon",                   "elemental",            "fungus or mold",
    "gnome",                    "giant humanoid",       0,
    "jabberwock",               "Keystone Kop",         "lich",
    "mummy",                    "naga",                 "ogre",
    "pudding or ooze",          "quantum mechanic",     "rust monster or disenchanter",
    "snake",                    "troll",                "umber hulk",
    "vampire",                  "wraith",               "xorn",
    "apelike creature",         "zombie",

    "human or elf",             "ghost",                "golem",
    "major demon",              "sea monster",          "lizard",
    "long worm tail",           "mimic"
};

const struct symdef def_warnsyms[WARNCOUNT] = {
        {'0', "unknown creature causing you worry", CLR_WHITE},      /* white warning  */
        {'1', "unknown creature causing you concern", CLR_RED},      /* pink warning   */
        {'2', "unknown creature causing you anxiety", CLR_RED},      /* red warning    */
        {'3', "unknown creature causing you disquiet", CLR_RED},     /* ruby warning   */
        {'4', "unknown creature causing you alarm",
                                                CLR_MAGENTA},        /* purple warning */
        {'5', "unknown creature causing you dread",
                                                CLR_BRIGHT_MAGENTA}  /* black warning  */
};

/*
 *  Default screen symbols with explanations and colors.
 *  Note:  {ibm|dec|mac}_graphics[] arrays also depend on this symbol order.
 */
const struct symdef defsyms[MAXPCHARS] = {
/* 0*/  {' ', "dark part of a room",NO_COLOR},       /* stone */
        {'|', "wall",           CLR_GRAY},   /* vwall */
        {'-', "wall",           CLR_GRAY},   /* hwall */
        {'-', "wall",           CLR_GRAY},   /* tlcorn */
        {'-', "wall",           CLR_GRAY},   /* trcorn */
        {'-', "wall",           CLR_GRAY},   /* blcorn */
        {'-', "wall",           CLR_GRAY},   /* brcorn */
        {'-', "wall",           CLR_GRAY},   /* crwall */
        {'-', "wall",           CLR_GRAY},   /* tuwall */
        {'-', "wall",           CLR_GRAY},   /* tdwall */
/*10*/  {'|', "wall",           CLR_GRAY},   /* tlwall */
        {'|', "wall",           CLR_GRAY},   /* trwall */
        {'.', "doorway",        CLR_GRAY},   /* ndoor */
        {'-', "open door",      CLR_BROWN},  /* vodoor */
        {'|', "open door",      CLR_BROWN},  /* hodoor */
        {'+', "closed door",    CLR_BROWN},  /* vcdoor */
        {'+', "closed door",    CLR_BROWN},  /* hcdoor */
        {'#', "iron bars",      HI_METAL},   /* bars */
        {'#', "tree",           CLR_GREEN},  /* tree */
        {'.', "floor of a room",CLR_GRAY},   /* room */
/*20*/  {'#', "corridor",       CLR_GRAY},   /* dark corr */
        {'#', "lit corridor",   CLR_GRAY},   /* lit corr (see mapglyph.c) */
        {'<', "staircase up",   CLR_GRAY},   /* upstair */
        {'>', "staircase down", CLR_GRAY},   /* dnstair */
        {'<', "ladder up",      CLR_BROWN},  /* upladder */
        {'>', "ladder down",    CLR_BROWN},  /* dnladder */
        {'_', "altar",          CLR_GRAY},   /* altar */
        {'|', "grave",      CLR_GRAY},   /* grave */
        {'\\', "opulent throne",HI_GOLD},    /* throne */
        {'#', "sink",           CLR_GRAY},   /* sink */
/*30*/  {'{', "fountain",       CLR_BLUE},   /* fountain */
        {'}', "water",          CLR_BLUE},   /* pool */
        {'.', "ice",            CLR_CYAN},   /* ice */
        {'}', "molten lava",    CLR_RED},    /* lava */
        {'.', "lowered drawbridge",CLR_BROWN},       /* vodbridge */
        {'.', "lowered drawbridge",CLR_BROWN},       /* hodbridge */
        {'#', "raised drawbridge",CLR_BROWN},/* vcdbridge */
        {'#', "raised drawbridge",CLR_BROWN},/* hcdbridge */
        {' ', "air",            CLR_CYAN},   /* open air */
        {'#', "cloud",          CLR_GRAY},   /* [part of] a cloud */
/*40*/  {'}', "water",          CLR_BLUE},   /* under water */
        {'^', "arrow trap",     HI_METAL},   /* trap */
        {'^', "dart trap",      HI_METAL},   /* trap */
        {'^', "falling rock trap",CLR_GRAY}, /* trap */
        {'^', "squeaky board",  CLR_BROWN},  /* trap */
        {'^', "bear trap",      HI_METAL},   /* trap */
        {'^', "land mine",      CLR_RED},    /* trap */
        {'^', "rolling boulder trap",   CLR_GRAY},   /* trap */
        {'^', "sleeping gas trap",HI_ZAP},   /* trap */
        {'^', "rust trap",      CLR_BLUE},   /* trap */
/*50*/  {'^', "fire trap",      CLR_ORANGE}, /* trap */
        {'^', "pit",            CLR_BLACK},  /* trap */
        {'^', "spiked pit",     CLR_BLACK},  /* trap */
        {'^', "hole",   CLR_BROWN},  /* trap */
        {'^', "trap door",      CLR_BROWN},  /* trap */
        {'^', "teleportation trap", CLR_MAGENTA},    /* trap */
        {'^', "level teleporter", CLR_MAGENTA},      /* trap */
        {'^', "magic portal",   CLR_BRIGHT_MAGENTA}, /* trap */
        {'"', "web",            CLR_GRAY},   /* web */
        {'^', "statue trap",    CLR_GRAY},   /* trap */
/*60*/  {'^', "magic trap",     HI_ZAP},     /* trap */
        {'^', "anti-magic field", HI_ZAP},   /* trap */
        {'^', "polymorph trap", CLR_BRIGHT_GREEN},   /* trap */
        {'|', "wall",           CLR_GRAY},   /* vbeam */
        {'-', "wall",           CLR_GRAY},   /* hbeam */
        {'\\',"wall",           CLR_GRAY},   /* lslant */
        {'/', "wall",           CLR_GRAY},   /* rslant */
        {'*', "",               CLR_WHITE},  /* dig beam */
        {'!', "",               CLR_WHITE},  /* camera flash beam */
        {')', "",               HI_WOOD},    /* boomerang open left */
/*70*/  {'(', "",               HI_WOOD},    /* boomerang open right */
        {'0', "",               HI_ZAP},     /* 4 magic shield symbols */
        {'#', "",               HI_ZAP},
        {'@', "",               HI_ZAP},
        {'*', "",               HI_ZAP},
        {'/', "",               CLR_GREEN},  /* swallow top left     */
        {'-', "",               CLR_GREEN},  /* swallow top center   */
        {'\\', "",              CLR_GREEN},  /* swallow top right    */
        {'|', "",               CLR_GREEN},  /* swallow middle left  */
        {'|', "",               CLR_GREEN},  /* swallow middle right */
/*80*/  {'\\', "",              CLR_GREEN},  /* swallow bottom left  */
        {'-', "",               CLR_GREEN},  /* swallow bottom center*/
        {'/', "",               CLR_GREEN},  /* swallow bottom right */
        {'/', "",               CLR_ORANGE}, /* explosion top left     */
        {'-', "",               CLR_ORANGE}, /* explosion top center   */
        {'\\', "",              CLR_ORANGE}, /* explosion top right    */
        {'|', "",               CLR_ORANGE}, /* explosion middle left  */
        {' ', "",               CLR_ORANGE}, /* explosion middle center*/
        {'|', "",               CLR_ORANGE}, /* explosion middle right */
        {'\\', "",              CLR_ORANGE}, /* explosion bottom left  */
/*90*/  {'-', "",               CLR_ORANGE}, /* explosion bottom center*/
        {'/', "",               CLR_ORANGE}, /* explosion bottom right */
/*
 *  Note: Additions to this array should be reflected in the
 *        {ibm,dec,mac}_graphics[] arrays below.
 */
};

/*
 * Convert the given character to an object class.  If the character is not
 * recognized, then MAXOCLASSES is returned.  Used in detect.c invent.c,
 * options.c, pickup.c, sp_lev.c, and lev_main.c.
 */
int
def_char_to_objclass (char ch)
{
    int i;
    for (i = 1; i < MAXOCLASSES; i++)
        if (ch == def_oc_syms[i]) break;
    return i;
}

/*
 * Convert a character into a monster class.  This returns the _first_
 * match made.  If there are are no matches, return MAXMCLASSES.
 */
int
def_char_to_monclass (char ch)
{
    int i;
    for (i = 1; i < MAXMCLASSES; i++)
        if (def_monsyms[i] == ch) break;
    return i;
}

void
assign_graphics (unsigned char *graph_chars, int glth, int maxlen, int offset)
{
    int i;

    for (i = 0; i < maxlen; i++)
        showsyms[i+offset] = (((i < glth) && graph_chars[i]) ?
                       graph_chars[i] : defsyms[i+offset].sym);
}

