/* See LICENSE in the root of this project for change info */

#include <stdbool.h>
#include <string.h>

#include "dungeon_util.h"
#include "align.h"
#include "artifact.h"
#include "artifact_names.h"
#include "decl.h"
#include "do_name.h"
#include "dungeon.h"
#include "global.h"
#include "hack.h"
#include "makemon.h"
#include "mkobj.h"
#include "mondata.h"
#include "monst.h"
#include "muse.h"
#include "obj.h"
#include "objclass.h"
#include "objnam.h"
#include "onames.h"
#include "permonst.h"
#include "pline.h"
#include "pm.h"
#include "rm.h"
#include "rnd.h"
#include "steal.h"
#include "teleport.h"
#include "worn.h"
#include "you.h"

/* These are the names of those who
 * contributed to the development of NetHack 3.2/3.3/3.4.
 *
 * Keep in alphabetical order within teams.
 * Same first name is entered once within each team.
 */
static const char *developers[] = {
        /* devteam */
        "Dave", "Dean", "Eric", "Izchak", "Janet", "Jessie",
        "Ken", "Kevin", "Michael", "Mike", "Pat", "Paul", "Steve", "Timo",
        "Warwick",
        /* PC team */
        "Bill", "Eric", "Keizo", "Ken", "Kevin", "Michael", "Mike", "Paul",
        "Stephen", "Steve", "Timo", "Yitzhak",
        /* Amiga team */
        "Andy", "Gregg", "Janne", "Keni", "Mike", "Olaf", "Richard",
        /* Mac team */
        "Andy", "Chris", "Dean", "Jon", "Jonathan", "Kevin", "Wang",
        /* Atari team */
        "Eric", "Marvin", "Warwick",
        /* NT team */
        "Alex", "Dion", "Michael",
        /* OS/2 team */
        "Helge", "Ron", "Timo",
        /* VMS team */
        "Joshua", "Pat",
        // thejoshwolfe/nethack fork
        "Josh", "Andrew",
        ""};


/* return a randomly chosen developer name */
static const char * dev_name (void) {
    int i, m = 0, n = SIZE(developers);
    struct monst *mtmp;
    bool match;

    do {
        match = false;
        i = rn2(n);
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if(!is_mplayer(mtmp->data)) continue;
            if(!strncmp(developers[i], monster_name(mtmp),
                        strlen(developers[i]))) {
                match = true;
                break;
            }
        }
        m++;
    } while (match && m < 100); /* m for insurance */

    if (match) return (const char *)0;
    return(developers[i]);
}

static void get_mplname (struct monst *mtmp, char *nam) {
    bool fmlkind = is_female(mtmp->data);
    const char *devnam;

    devnam = dev_name();
    if (!devnam)
        strcpy(nam, fmlkind ? "Eve" : "Adam");
    else if (fmlkind && !!strcmp(devnam, "Janet"))
        strcpy(nam, rn2(2) ? "Maud" : "Eve");
    else strcpy(nam, devnam);

    if (fmlkind || !strcmp(nam, "Janet"))
        mtmp->female = 1;
    else
        mtmp->female = 0;
    strcat(nam, " the ");
    strcat(nam, rank_of((int)mtmp->m_lev,
                monsndx(mtmp->data),
                (bool)mtmp->female));
}

static void mk_mplayer_armor (struct monst *mon, short typ) {
    struct obj *obj;

    if (typ == STRANGE_OBJECT) return;
    obj = mksobj(typ, false, false);
    if (!rn2(3)) obj->oerodeproof = 1;
    if (!rn2(3)) curse(obj);
    if (!rn2(3)) bless(obj);
    /* Most players who get to the endgame who have cursed equipment
     * have it because the wizard or other monsters cursed it, so its
     * chances of having plusses is the same as usual....
     */
    obj->spe = rn2(10) ? (rn2(3) ? rn2(5) : rn1(4,4)) : -rnd(3);
    (void) mpickobj(mon, obj);
}

struct monst * mk_mplayer (struct permonst *ptr, signed char x, signed char y, bool special) {
    struct monst *mtmp;
    char nam[PL_NSIZ];

    if(!is_mplayer(ptr))
        return((struct monst *)0);

    if(MON_AT(x, y))
        (void) rloc(m_at(x, y), false); /* insurance */

    if(!In_endgame(&u.uz)) special = false;

    if ((mtmp = makemon(ptr, x, y, NO_MM_FLAGS)) != 0) {
        short weapon = rn2(2) ? LONG_SWORD : rnd_class(SPEAR, BULLWHIP);
        short armor = rnd_class(GRAY_DRAGON_SCALE_MAIL, YELLOW_DRAGON_SCALE_MAIL);
        short cloak = !rn2(8) ? STRANGE_OBJECT :
            rnd_class(OILSKIN_CLOAK, CLOAK_OF_DISPLACEMENT);
        short helm = !rn2(8) ? STRANGE_OBJECT :
            rnd_class(ELVEN_LEATHER_HELM, HELM_OF_TELEPATHY);
        short shield = !rn2(8) ? STRANGE_OBJECT :
            rnd_class(ELVEN_SHIELD, SHIELD_OF_REFLECTION);
        int quan;
        struct obj *otmp;

        mtmp->m_lev = (special ? rn1(16,15) : rnd(16));
        mtmp->mhp = mtmp->mhpmax = d((int)mtmp->m_lev,10) +
            (special ? (30 + rnd(30)) : 30);
        if(special) {
            get_mplname(mtmp, nam);
            mtmp = christen_monst(mtmp, nam);
            /* that's why they are "stuck" in the endgame :-) */
            (void)mongets(mtmp, FAKE_AMULET_OF_YENDOR);
        }
        mtmp->mpeaceful = 0;
        set_malign(mtmp); /* peaceful may have changed again */

        switch(monsndx(ptr)) {
            case PM_ARCHEOLOGIST:
                if (rn2(2)) weapon = BULLWHIP;
                break;
            case PM_BARBARIAN:
                if (rn2(2)) {
                    weapon = rn2(2) ? TWO_HANDED_SWORD : BATTLE_AXE;
                    shield = STRANGE_OBJECT;
                }
                if (rn2(2)) armor = rnd_class(PLATE_MAIL, CHAIN_MAIL);
                if (helm == HELM_OF_BRILLIANCE) helm = STRANGE_OBJECT;
                break;
            case PM_CAVEMAN:
            case PM_CAVEWOMAN:
                if (rn2(4)) weapon = MACE;
                else if (rn2(2)) weapon = CLUB;
                if (helm == HELM_OF_BRILLIANCE) helm = STRANGE_OBJECT;
                break;
            case PM_HEALER:
                if (rn2(4)) weapon = QUARTERSTAFF;
                else if (rn2(2)) weapon = rn2(2) ? UNICORN_HORN : SCALPEL;
                if (rn2(4)) helm = rn2(2) ? HELM_OF_BRILLIANCE : HELM_OF_TELEPATHY;
                if (rn2(2)) shield = STRANGE_OBJECT;
                break;
            case PM_KNIGHT:
                if (rn2(4)) weapon = LONG_SWORD;
                if (rn2(2)) armor = rnd_class(PLATE_MAIL, CHAIN_MAIL);
                break;
            case PM_MONK:
                weapon = STRANGE_OBJECT;
                armor = STRANGE_OBJECT;
                cloak = ROBE;
                if (rn2(2)) shield = STRANGE_OBJECT;
                break;
            case PM_PRIEST:
            case PM_PRIESTESS:
                if (rn2(2)) weapon = MACE;
                if (rn2(2)) armor = rnd_class(PLATE_MAIL, CHAIN_MAIL);
                if (rn2(4)) cloak = ROBE;
                if (rn2(4)) helm = rn2(2) ? HELM_OF_BRILLIANCE : HELM_OF_TELEPATHY;
                if (rn2(2)) shield = STRANGE_OBJECT;
                break;
            case PM_RANGER:
                if (rn2(2)) weapon = ELVEN_DAGGER;
                break;
            case PM_ROGUE:
                if (rn2(2)) weapon = SHORT_SWORD;
                break;
            case PM_SAMURAI:
                if (rn2(2)) weapon = KATANA;
                break;
            case PM_TOURIST:
                /* Defaults are just fine */
                break;
            case PM_VALKYRIE:
                if (rn2(2)) weapon = WAR_HAMMER;
                if (rn2(2)) armor = rnd_class(PLATE_MAIL, CHAIN_MAIL);
                break;
            case PM_WIZARD:
                if (rn2(4)) weapon = rn2(2) ? QUARTERSTAFF : ATHAME;
                if (rn2(2)) {
                    armor = rn2(2) ? BLACK_DRAGON_SCALE_MAIL :
                        SILVER_DRAGON_SCALE_MAIL;
                    cloak = CLOAK_OF_MAGIC_RESISTANCE;
                }
                if (rn2(4)) helm = HELM_OF_BRILLIANCE;
                shield = STRANGE_OBJECT;
                break;
            default: impossible("bad mplayer monster");
                     weapon = 0;
                     break;
        }

        if (weapon != STRANGE_OBJECT) {
            otmp = mksobj(weapon, true, false);
            otmp->spe = (special ? rn1(5,4) : rn2(4));
            if (!rn2(3)) otmp->oerodeproof = 1;
            else if (!rn2(2)) otmp->greased = 1;
            if (special && rn2(2))
                otmp = mk_artifact(otmp, A_NONE);
            /* mplayers knew better than to overenchant Magicbane */
            if (otmp->oartifact == ART_MAGICBANE)
                otmp->spe = rnd(4);
            (void) mpickobj(mtmp, otmp);
        }

        if(special) {
            if (!rn2(10))
                (void) mongets(mtmp, rn2(3) ? LUCKSTONE : LOADSTONE);
            mk_mplayer_armor(mtmp, armor);
            mk_mplayer_armor(mtmp, cloak);
            mk_mplayer_armor(mtmp, helm);
            mk_mplayer_armor(mtmp, shield);
            if (rn2(8))
                mk_mplayer_armor(mtmp, rnd_class(LEATHER_GLOVES,
                            GAUNTLETS_OF_DEXTERITY));
            if (rn2(8))
                mk_mplayer_armor(mtmp, rnd_class(LOW_BOOTS, LEVITATION_BOOTS));
            m_dowear(mtmp, true);

            quan = rn2(3) ? rn2(3) : rn2(16);
            while(quan--)
                (void)mongets(mtmp, rnd_class(DILITHIUM_CRYSTAL, JADE));
            /* To get the gold "right" would mean a player can double his */
            /* gold supply by killing one mplayer.  Not good. */
            mtmp->mgold = rn2(1000);
            quan = rn2(10);
            while(quan--)
                (void) mpickobj(mtmp, mkobj(RANDOM_CLASS, false));
        }
        quan = rnd(3);
        while(quan--)
            (void)mongets(mtmp, rnd_offensive_item(mtmp));
        quan = rnd(3);
        while(quan--)
            (void)mongets(mtmp, rnd_defensive_item(mtmp));
        quan = rnd(3);
        while(quan--)
            (void)mongets(mtmp, rnd_misc_item(mtmp));
    }

    return(mtmp);
}

/* create the indicated number (num) of monster-players,
 * randomly chosen, and in randomly chosen (free) locations
 * on the level.  If "special", the size of num should not
 * be bigger than the number of _non-repeated_ names in the
 * developers array, otherwise a bunch of Adams and Eves will
 * fill up the overflow.
 */
void create_mplayers (int num, bool special) {
    int pm, x, y;
    struct monst fakemon;

    while(num) {
        int tryct = 0;

        /* roll for character class */
        pm = PM_ARCHEOLOGIST + rn2(PM_WIZARD - PM_ARCHEOLOGIST + 1);
        fakemon.data = &mons[pm];

        /* roll for an available location */
        do {
            x = rn1(COLNO-4, 2);
            y = rnd(ROWNO-2);
        } while(!goodpos(x, y, &fakemon, 0) && tryct++ <= 50);

        /* if pos not found in 50 tries, don't bother to continue */
        if(tryct > 50) return;

        (void) mk_mplayer(&mons[pm], (signed char)x, (signed char)y, special);
        num--;
    }
}

void mplayer_talk (struct monst *mtmp) {
    static const char *same_class_msg[3] = {
        "I can't win, and neither will you!",
        "You don't deserve to win!",
        "Mine should be the honor, not yours!",
    },                *other_class_msg[3] = {
        "The low-life wants to talk, eh?",
        "Fight, scum!",
        "Here is what I have to say!",
    };

    if(mtmp->mpeaceful) return; /* will drop to humanoid talk */

    pline("Talk? -- %s",
            (mtmp->data == &mons[urole.malenum] ||
             mtmp->data == &mons[urole.femalenum]) ?
            same_class_msg[rn2(3)] : other_class_msg[rn2(3)]);
}
