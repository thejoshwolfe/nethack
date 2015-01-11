/* See LICENSE in the root of this project for change info */
#ifndef YOU_H
#define YOU_H

#include "attrib.h"
#include "align.h"
#include "prop.h"
#include "skills.h"
#include "dungeon.h"
#include "global.h"

/*** Substructures ***/

struct RoleName {
        const char      *m;     /* name when character is male */
        const char      *f;     /* when female; null if same as male */
};

struct RoleAdvance {
        /* "fix" is the fixed amount, "rnd" is the random amount */
        signed char infix, inrnd;       /* at character initialization */
        signed char lofix, lornd;       /* gained per level <  urole.xlev */
        signed char hifix, hirnd;       /* gained per level >= urole.xlev */
};

struct u_have {
        unsigned amulet:1;     /* carrying Amulet      */
        unsigned bell:1;       /* carrying Bell        */
        unsigned book:1;       /* carrying Book        */
        unsigned menorah:1;    /* carrying Candelabrum */
        unsigned questart:1;   /* carrying the Quest Artifact */
        unsigned unused:3;
};

struct u_event {
        unsigned minor_oracle:1;       /* received at least 1 cheap oracle */
        unsigned major_oracle:1;       /*  "  expensive oracle */
        unsigned qcalled:1;            /* called by Quest leader to do task */
        unsigned qexpelled:1;          /* expelled from the Quest dungeon */
        unsigned qcompleted:1;         /* successfully completed Quest task */
        unsigned uheard_tune:2;        /* 1=know about, 2=heard passtune */
        unsigned uopened_dbridge:1;    /* opened the drawbridge */

        unsigned invoked:1;            /* invoked Gate to the Sanctum level */
        unsigned gehennom_entered:1;   /* entered Gehennom via Valley */
        unsigned uhand_of_elbereth:2;  /* became Hand of Elbereth */
        unsigned udemigod:1;           /* killed the wiz */
        unsigned ascended:1;           /* has offered the Amulet */
};

/* KMH, conduct --
 * These are voluntary challenges.  Each field denotes the number of
 * times a challenge has been violated.
 */
struct u_conduct {              /* number of times... */
        long    unvegetarian;   /* eaten any animal */
        long    unvegan;        /* ... or any animal byproduct */
        long    food;           /* ... or any comestible */
        long    gnostic;        /* used prayer, priest, or altar */
        long    weaphit;        /* hit a monster with a weapon */
        long    killer;         /* killed a monster yourself */
        long    literate;       /* read something (other than BotD) */
        long    polypiles;      /* polymorphed an object */
        long    polyselfs;      /* transformed yourself */
        long    wishes;         /* used a wish */
        long    wisharti;       /* wished for an artifact */
                                /* genocides already listed at end of game */
};

/*** Unified structure containing role information ***/
struct Role {
        /*** Strings that name various things ***/
        struct RoleName name;   /* the role's name (from u_init.c) */
        struct RoleName rank[9]; /* names for experience levels (from botl.c) */
        const char *lgod, *ngod, *cgod; /* god names (from pray.c) */
        const char *filecode;   /* abbreviation for use in file names */
        const char *homebase;   /* quest leader's location (from questpgr.c) */
        const char *intermed;   /* quest intermediate goal (from questpgr.c) */

        /*** Indices of important monsters and objects ***/
        short malenum,          /* index (PM_) as a male (botl.c) */
              femalenum,        /* ...or as a female (NON_PM == same) */
              petnum,           /* PM_ of preferred pet (NON_PM == random) */
              ldrnum,           /* PM_ of quest leader (questpgr.c) */
              guardnum,         /* PM_ of quest guardians (questpgr.c) */
              neminum,          /* PM_ of quest nemesis (questpgr.c) */
              enemy1num,        /* specific quest enemies (NON_PM == random) */
              enemy2num;
        char  enemy1sym,        /* quest enemies by class (S_) */
              enemy2sym;
        short questarti;        /* index (ART_) of quest artifact (questpgr.c) */

        /*** Bitmasks ***/
        short allow;            /* bit mask of allowed variations */
#define ROLE_RACEMASK   0x0ff8          /* allowable races */
#define ROLE_GENDMASK   0xf000          /* allowable genders */
#define ROLE_MALE       0x1000
#define ROLE_FEMALE     0x2000
#define ROLE_NEUTER     0x4000
#define ROLE_ALIGNMASK  AM_MASK         /* allowable alignments */
#define ROLE_LAWFUL     AM_LAWFUL
#define ROLE_NEUTRAL    AM_NEUTRAL
#define ROLE_CHAOTIC    AM_CHAOTIC

        /*** Attributes (from attrib.c and exper.c) ***/
        signed char attrbase[A_MAX];    /* lowest initial attributes */
        signed char attrdist[A_MAX];    /* distribution of initial attributes */
        struct RoleAdvance hpadv; /* hit point advancement */
        struct RoleAdvance enadv; /* energy advancement */
        signed char xlev;               /* cutoff experience level */
        signed char initrecord; /* initial alignment record */

        /*** Spell statistics (from spell.c) ***/
        int spelbase;           /* base spellcasting penalty */
        int spelheal;           /* penalty (-bonus) for healing spells */
        int spelshld;           /* penalty for wearing any shield */
        int spelarmr;           /* penalty for wearing metal armour */
        int spelstat;           /* which stat (A_) is used */
        int spelspec;           /* spell (SPE_) the class excels at */
        int spelsbon;           /* penalty (-bonus) for that spell */

        /*** Properties in variable-length arrays ***/
        /* intrinsics (see attrib.c) */
        /* initial inventory (see u_init.c) */
        /* skills (see u_init.c) */

        /*** Don't forget to add... ***/
        /* quest leader, guardians, nemesis (monst.c) */
        /* quest artifact (artilist.h) */
        /* quest dungeon definition (dat/Xyz.dat) */
        /* quest text (dat/quest.txt) */
        /* dictionary entries (dat/data.bas) */
};

extern const struct Role roles[];       /* table of available roles */
extern struct Role urole;
#define Role_if(X)      (urole.malenum == (X))
#define Role_switch     (urole.malenum)

/* used during initialization for race, gender, and alignment
   as well as for character class */
#define ROLE_NONE       (-1)
#define ROLE_RANDOM     (-2)

/*** Unified structure specifying race information ***/

struct Race {
        /*** Strings that name various things ***/
        const char *noun;       /* noun ("human", "elf") */
        const char *adj;        /* adjective ("human", "elven") */
        const char *coll;       /* collective ("humanity", "elvenkind") */
        const char *filecode;   /* code for filenames */
        struct RoleName individual; /* individual as a noun ("man", "elf") */

        /*** Indices of important monsters and objects ***/
        short malenum,          /* PM_ as a male monster */
              femalenum,        /* ...or as a female (NON_PM == same) */
              mummynum,         /* PM_ as a mummy */
              zombienum;        /* PM_ as a zombie */

        /*** Bitmasks ***/
        short allow;            /* bit mask of allowed variations */
        short selfmask,         /* your own race's bit mask */
              lovemask,         /* bit mask of always peaceful */
              hatemask;         /* bit mask of always hostile */

        /*** Attributes ***/
        signed char attrmin[A_MAX];     /* minimum allowable attribute */
        signed char attrmax[A_MAX];     /* maximum allowable attribute */
        struct RoleAdvance hpadv; /* hit point advancement */
        struct RoleAdvance enadv; /* energy advancement */

        /*** Properties in variable-length arrays ***/
        /* intrinsics (see attrib.c) */

        /*** Don't forget to add... ***/
        /* quest leader, guardians, nemesis (monst.c) */
        /* quest dungeon definition (dat/Xyz.dat) */
        /* quest text (dat/quest.txt) */
        /* dictionary entries (dat/data.bas) */
};

extern const struct Race races[];       /* Table of available races */
extern struct Race urace;
#define Race_if(X)      (urace.malenum == (X))
#define Race_switch     (urace.malenum)

/*** Unified structure specifying gender information ***/
struct Gender {
        const char *adj;        /* male/female/neuter */
        const char *he;         /* he/she/it */
        const char *him;        /* him/her/it */
        const char *his;        /* his/her/its */
        const char *filecode;   /* file code */
        short allow;            /* equivalent ROLE_ mask */
};
#define ROLE_GENDERS    2       /* number of permitted player genders */
                                /* increment to 3 if you allow neuter roles */

extern const struct Gender genders[];   /* table of available genders */
#define uhe()   (genders[flags.female ? 1 : 0].he)
#define uhim()  (genders[flags.female ? 1 : 0].him)
#define uhis()  (genders[flags.female ? 1 : 0].his)
#define mhe(mtmp)       (genders[pronoun_gender(mtmp)].he)
#define mhim(mtmp)      (genders[pronoun_gender(mtmp)].him)
#define mhis(mtmp)      (genders[pronoun_gender(mtmp)].his)


/*** Unified structure specifying alignment information ***/
struct Align {
        const char *noun;       /* law/balance/chaos */
        const char *adj;        /* lawful/neutral/chaotic */
        const char *filecode;   /* file code */
        short allow;            /* equivalent ROLE_ mask */
        aligntyp value;         /* equivalent A_ value */
};
#define ROLE_ALIGNS     3       /* number of permitted player alignments */

extern const struct Align aligns[];     /* table of available alignments */

enum {
    TT_BEARTRAP    = 0,
    TT_PIT         = 1,
    TT_WEB         = 2,
    TT_LAVA        = 3,
    TT_INFLOOR     = 4,
};

/*** Information about the player ***/
struct you {
        signed char ux, uy;

        /* direction of move (or zap or ... ) */
        Direction delta;
        signed char dz;

        signed char di;         /* direction of FF */
        signed char tx, ty;             /* destination of travel */
        signed char ux0, uy0;           /* initial position FF */
        d_level uz, uz0;        /* your level on this and the previous turn */
        d_level utolev;         /* level monster teleported you to, or uz */
        unsigned char utotype;          /* bitmask of goto_level() flags for utolev */
        bool umoved;         /* changed map location (post-move) */
        int last_str_turn;      /* 0: none, 1: half turn, 2: full turn */
                                /* +: turn right, -: turn left */
        int ulevel;             /* 1 to MAXULEV */
        int ulevelmax;
        unsigned utrap;         /* trap timeout */
        unsigned utraptype;     /* defined if utrap nonzero */
        char    urooms[5];      /* rooms (roomno + 3) occupied now */
        char    urooms0[5];     /* ditto, for previous position */
        char    uentered[5];    /* rooms (roomno + 3) entered this turn */
        char    ushops[5];      /* shop rooms (roomno + 3) occupied now */
        char    ushops0[5];     /* ditto, for previous position */
        char    ushops_entered[5]; /* ditto, shops entered this turn */
        char    ushops_left[5]; /* ditto, shops exited this turn */

        int      uhunger;       /* refd only in eat.c and shk.c */
        unsigned uhs;           /* hunger state - see eat.c */

        struct prop uprops[LAST_PROP+1];

        unsigned umconf;
        char usick_cause[PL_PSIZ+20]; /* sizeof "unicorn horn named "+1 */
        unsigned usick_type:2;
#define SICK_VOMITABLE 0x01
#define SICK_NONVOMITABLE 0x02
#define SICK_ALL 0x03

        /* These ranges can never be more than MAX_RANGE (vision.h). */
        int nv_range;           /* current night vision range */
        int xray_range;         /* current xray vision range */

        /*
         * These variables are valid globally only when punished and blind.
         */
#define BC_BALL  0x01   /* bit mask for ball  in 'bc_felt' below */
#define BC_CHAIN 0x02   /* bit mask for chain in 'bc_felt' below */
        int bglyph;     /* glyph under the ball */
        int cglyph;     /* glyph under the chain */
        int bc_order;   /* ball & chain order [see bc_order() in ball.c] */
        int bc_felt;    /* mask for ball/chain being felt */

        int umonster;                   /* hero's "real" monster num */
        int umonnum;                    /* current monster number */

        int mh, mhmax, mtimedone;       /* for polymorph-self */
        struct attribs  macurr,         /* for monster attribs */
                        mamax;          /* for monster attribs */
        int ulycn;                      /* lycanthrope type */

        unsigned ucreamed;
        unsigned uswldtim;              /* time you have been swallowed */

        unsigned uswallow:1;           /* true if swallowed */
        unsigned uinwater:1;           /* if you're currently in water (only
                                           underwater possible currently) */
        unsigned uundetected:1;        /* if you're a hiding monster/piercer */
        unsigned mfemale:1;            /* saved human value of flags.female */
        unsigned uinvulnerable:1;      /* you're invulnerable (praying) */
        unsigned uburied:1;            /* you're buried */
        unsigned uedibility:1;         /* blessed food detection; sense unsafe food */
        /* 1 free bit! */

        unsigned udg_cnt;               /* how long you have been demigod */
        struct u_event  uevent;         /* certain events have happened */
        struct u_have   uhave;          /* you're carrying special objects */
        struct u_conduct uconduct;      /* KMH, conduct */
        struct attribs  acurr,          /* your current attributes (eg. str)*/
                        aexe,           /* for gain/loss via "exercise" */
                        abon,           /* your bonus attributes (eg. str) */
                        amax,           /* your max attributes (eg. str) */
                        atemp,          /* used for temporary loss/gain */
                        atime;          /* used for loss/gain countdown */
        align   ualign;                 /* character alignment */
#define CONVERT         2
#define A_ORIGINAL      1
#define A_CURRENT       0
        aligntyp ualignbase[CONVERT];   /* for ualign conversion record */
        signed char uluck, moreluck;            /* luck and luck bonus */
#define Luck    (u.uluck + u.moreluck)
#define LUCKADD         3       /* added value when carrying luck stone */
#define LUCKMAX         10      /* on moonlit nights 11 */
#define LUCKMIN         (-10)
        signed char     uhitinc;
        signed char     udaminc;
        signed char     uac;
        unsigned char   uspellprot;             /* protection by SPE_PROTECTION */
        unsigned char   usptime;                /* #moves until uspellprot-- */
        unsigned char   uspmtime;               /* #moves between uspellprot-- */
        int     uhp,uhpmax;
        int     uen, uenmax;            /* magical energy - M. Stephenson */
        int ugangr;                     /* if the gods are angry at you */
        int ugifts;                     /* number of artifacts bestowed */
        int ublessed, ublesscnt;        /* blessing/duration from #pray */
        long    ugold, ugold0;
        long    uexp, urexp;
        long    ucleansed;      /* to record moves when player was cleansed */
        long    usleep;         /* sleeping; monstermove you last started */
        int uinvault;
        struct monst *ustuck;
        struct monst *usteed;
        long ugallop;
        int urideturns;
        int     umortality;             /* how many times you died */
        int ugrave_arise; /* you die and become something aside from a ghost */
        time_t  ubirthday;              /* real world time when game began */

        int     weapon_slots;           /* unused skill slots */
        int     skills_advanced;                /* # of advances made so far */
        signed char     skill_record[P_SKILL_LIMIT];    /* skill advancements */
        struct skills weapon_skills[P_NUM_SKILLS];
        bool twoweap;                /* KMH -- Using two-weapon combat */

};      /* end of `struct you' */

#define Upolyd (u.umonnum != u.umonster)

#endif  /* YOU_H */
