/* See LICENSE in the root of this project for change info */
/* shknam.c -- initialize a shop */

#include "shknam.h"

#include <stdio.h>
#include <string.h>

#include "cmd.h"
#include "coord.h"
#include "decl.h"
#include "detect.h"
#include "display.h"
#include "dungeon.h"
#include "engrave.h"
#include "eshk.h"
#include "global.h"
#include "hack.h"
#include "hacklib.h"
#include "makemon.h"
#include "mkobj.h"
#include "mkroom.h"
#include "monst.h"
#include "monsym.h"
#include "obj.h"
#include "objclass.h"
#include "onames.h"
#include "permonst.h"
#include "pm.h"
#include "rm.h"
#include "rnd.h"
#include "shk.h"
#include "teleport.h"
#include "you.h"

static const char * const shkliquors[] = {
    /* Ukraine */
    "Njezjin", "Tsjernigof", "Ossipewsk", "Gorlowka",
    /* Belarus */
    "Gomel",
    /* N. Russia */
    "Konosja", "Weliki Oestjoeg", "Syktywkar", "Sablja",
    "Narodnaja", "Kyzyl",
    /* Silezie */
    "Walbrzych", "Swidnica", "Klodzko", "Raciborz", "Gliwice",
    "Brzeg", "Krnov", "Hradec Kralove",
    /* Schweiz */
    "Leuk", "Brig", "Brienz", "Thun", "Sarnen", "Burglen", "Elm",
    "Flims", "Vals", "Schuls", "Zum Loch",
    0
};

static const char * const shkbooks[] = {
    /* Eire */
    "Skibbereen", "Kanturk", "Rath Luirc", "Ennistymon", "Lahinch",
    "Kinnegad", "Lugnaquillia", "Enniscorthy", "Gweebarra",
    "Kittamagh", "Nenagh", "Sneem", "Ballingeary", "Kilgarvan",
    "Cahersiveen", "Glenbeigh", "Kilmihil", "Kiltamagh",
    "Droichead Atha", "Inniscrone", "Clonegal", "Lisnaskea",
    "Culdaff", "Dunfanaghy", "Inishbofin", "Kesh",
    0
};

static const char * const shkarmors[] = {
    /* Turquie */
    "Demirci", "Kalecik", "Boyabai", "Yildizeli", "Gaziantep",
    "Siirt", "Akhalataki", "Tirebolu", "Aksaray", "Ermenak",
    "Iskenderun", "Kadirli", "Siverek", "Pervari", "Malasgirt",
    "Bayburt", "Ayancik", "Zonguldak", "Balya", "Tefenni",
    "Artvin", "Kars", "Makharadze", "Malazgirt", "Midyat",
    "Birecik", "Kirikkale", "Alaca", "Polatli", "Nallihan",
    0
};

static const char * const shkwands[] = {
    /* Wales */
    "Yr Wyddgrug", "Trallwng", "Mallwyd", "Pontarfynach",
    "Rhaeader", "Llandrindod", "Llanfair-ym-muallt",
    "Y-Fenni", "Maesteg", "Rhydaman", "Beddgelert",
    "Curig", "Llanrwst", "Llanerchymedd", "Caergybi",
    /* Scotland */
    "Nairn", "Turriff", "Inverurie", "Braemar", "Lochnagar",
    "Kerloch", "Beinn a Ghlo", "Drumnadrochit", "Morven",
    "Uist", "Storr", "Sgurr na Ciche", "Cannich", "Gairloch",
    "Kyleakin", "Dunvegan",
    0
};

static const char * const shkrings[] = {
    /* Hollandse familienamen */
    "Feyfer", "Flugi", "Gheel", "Havic", "Haynin", "Hoboken",
    "Imbyze", "Juyn", "Kinsky", "Massis", "Matray", "Moy",
    "Olycan", "Sadelin", "Svaving", "Tapper", "Terwen", "Wirix",
    "Ypey",
    /* Skandinaviske navne */
    "Rastegaisa", "Varjag Njarga", "Kautekeino", "Abisko",
    "Enontekis", "Rovaniemi", "Avasaksa", "Haparanda",
    "Lulea", "Gellivare", "Oeloe", "Kajaani", "Fauske",
    0
};

static const char * const shkfoods[] = {
    /* Indonesia */
    "Djasinga", "Tjibarusa", "Tjiwidej", "Pengalengan",
    "Bandjar", "Parbalingga", "Bojolali", "Sarangan",
    "Ngebel", "Djombang", "Ardjawinangun", "Berbek",
    "Papar", "Baliga", "Tjisolok", "Siboga", "Banjoewangi",
    "Trenggalek", "Karangkobar", "Njalindoeng", "Pasawahan",
    "Pameunpeuk", "Patjitan", "Kediri", "Pemboeang", "Tringanoe",
    "Makin", "Tipor", "Semai", "Berhala", "Tegal", "Samoe",
    0
};

static const char * const shkweapons[] = {
    /* Perigord */
    "Voulgezac", "Rouffiac", "Lerignac", "Touverac", "Guizengeard",
    "Melac", "Neuvicq", "Vanzac", "Picq", "Urignac", "Corignac",
    "Fleac", "Lonzac", "Vergt", "Queyssac", "Liorac", "Echourgnac",
    "Cazelon", "Eypau", "Carignan", "Monbazillac", "Jonzac",
    "Pons", "Jumilhac", "Fenouilledes", "Laguiolet", "Saujon",
    "Eymoutiers", "Eygurande", "Eauze", "Labouheyre",
    0
};

static const char * const shktools[] = {
    /* Spmi */
    "Ymla", "Eed-morra", "Cubask", "Nieb", "Bnowr Falr", "Telloc Cyaj",
    "Sperc", "Noskcirdneh", "Yawolloh", "Hyeghu", "Niskal", "Trahnil",
    "Htargcm", "Enrobwem", "Kachzi Rellim", "Regien", "Donmyar",
    "Yelpur", "Nosnehpets", "Stewe", "Renrut", "_Zlaw", "Nosalnef",
    "Rewuorb", "Rellenk", "Yad", "Cire Htims", "Y-crad", "Nenilukah",
    "Corsh", "Aned",
    0
};

static const char * const shklight[] = {
    /* Romania */
    "Zarnesti", "Slanic", "Nehoiasu", "Ludus", "Sighisoara", "Nisipitu",
    "Razboieni", "Bicaz", "Dorohoi", "Vaslui", "Fetesti", "Tirgu Neamt",
    "Babadag", "Zimnicea", "Zlatna", "Jiu", "Eforie", "Mamaia",
    /* Bulgaria */
    "Silistra", "Tulovo", "Panagyuritshte", "Smolyan", "Kirklareli",
    "Pernik", "Lom", "Haskovo", "Dobrinishte", "Varvara", "Oryahovo",
    "Troyan", "Lovech", "Sliven",
    0
};

static const char * const shkgeneral[] = {
    /* Suriname */
    "Hebiwerie", "Possogroenoe", "Asidonhopo", "Manlobbi",
    "Adjama", "Pakka Pakka", "Kabalebo", "Wonotobo",
    "Akalapi", "Sipaliwini",
    /* Greenland */
    "Annootok", "Upernavik", "Angmagssalik",
    /* N. Canada */
    "Aklavik", "Inuvik", "Tuktoyaktuk",
    "Chicoutimi", "Ouiatchouane", "Chibougamau",
    "Matagami", "Kipawa", "Kinojevis",
    "Abitibi", "Maganasipi",
    /* Iceland */
    "Akureyri", "Kopasker", "Budereyri", "Akranes", "Bordeyri",
    "Holmavik",
    0
};

/*
 * To add new shop types, all that is necessary is to edit the shtypes[] array.
 * See mkroom.h for the structure definition.  Typically, you'll have to lower
 * some or all of the probability fields in old entries to free up some
 * percentage for the new type.
 *
 * The placement type field is not yet used but will be in the near future.
 *
 * The iprobs array in each entry defines the probabilities for various kinds
 * of objects to be present in the given shop type.  You can associate with
 * each percentage either a generic object type (represented by one of the
 * *_CLASS macros) or a specific object (represented by an onames.h define).
 * In the latter case, prepend it with a unary minus so the code can know
 * (by testing the sign) whether to use mkobj() or mksobj().
 */

const struct shclass shtypes[] = {
        {"general store", RANDOM_CLASS, 44,
            D_SHOP, {{100, RANDOM_CLASS}, {0, 0}, {0, 0}}, shkgeneral},
        {"used armor dealership", ARMOR_CLASS, 14,
            D_SHOP, {{90, ARMOR_CLASS}, {10, WEAPON_CLASS}, {0, 0}},
             shkarmors},
        {"second-hand bookstore", SCROLL_CLASS, 10, D_SHOP,
            {{90, SCROLL_CLASS}, {10, SPBOOK_CLASS}, {0, 0}}, shkbooks},
        {"liquor emporium", POTION_CLASS, 10, D_SHOP,
            {{100, POTION_CLASS}, {0, 0}, {0, 0}}, shkliquors},
        {"antique weapons outlet", WEAPON_CLASS, 5, D_SHOP,
            {{90, WEAPON_CLASS}, {10, ARMOR_CLASS}, {0, 0}}, shkweapons},
        {"delicatessen", FOOD_CLASS, 5, D_SHOP,
            {{83, FOOD_CLASS}, {5, -POT_FRUIT_JUICE}, {4, -POT_BOOZE},
             {5, -POT_WATER}, {3, -ICE_BOX}}, shkfoods},
        {"jewelers", RING_CLASS, 3, D_SHOP,
            {{85, RING_CLASS}, {10, GEM_CLASS}, {5, AMULET_CLASS}, {0, 0}},
            shkrings},
        {"quality apparel and accessories", WAND_CLASS, 3, D_SHOP,
            {{90, WAND_CLASS}, {5, -LEATHER_GLOVES}, {5, -ELVEN_CLOAK}, {0, 0}},
             shkwands},
        {"hardware store", TOOL_CLASS, 3, D_SHOP,
            {{100, TOOL_CLASS}, {0, 0}, {0, 0}}, shktools},
        /* Actually shktools is ignored; the code specifically chooses a
         * random implementor name (along with candle shops having
         * random shopkeepers)
         */
        {"rare books", SPBOOK_CLASS, 3, D_SHOP,
            {{90, SPBOOK_CLASS}, {10, SCROLL_CLASS}, {0, 0}}, shkbooks},
        /* Shops below this point are "unique".  That is they must all have a
         * probability of zero.  They are only created via the special level
         * loader.
         */
        {"lighting store", TOOL_CLASS, 0, D_SHOP,
            {{32, -WAX_CANDLE}, {50, -TALLOW_CANDLE},
             {5, -BRASS_LANTERN}, {10, -OIL_LAMP}, {3, -MAGIC_LAMP}}, shklight},
        {(char *)0, 0, 0, 0, {{0, 0}, {0, 0}, {0, 0}}, 0}
};

/* make an object of the appropriate type for a shop square */
static void mkshobj_at ( const struct shclass *shp, int sx, int sy) {
    struct monst *mtmp;
    int atype;
    struct permonst *ptr;

    if (rn2(100) < depth(&u.uz) &&
            !MON_AT(sx, sy) && (ptr = mkclass(S_MIMIC,0)) &&
            (mtmp = makemon(ptr,sx,sy,NO_MM_FLAGS)) != 0) {
        /* note: makemon will set the mimic symbol to a shop item */
        if (rn2(10) >= depth(&u.uz)) {
            mtmp->m_ap_type = M_AP_OBJECT;
            mtmp->mappearance = STRANGE_OBJECT;
        }
    } else {
        atype = get_shop_item(shp - shtypes);
        if (atype < 0)
            (void) mksobj_at(-atype, sx, sy, true, true);
        else
            (void) mkobj_at(atype, sx, sy, true);
    }
}

/* extract a shopkeeper name for the given shop type */
static void nameshk (struct monst *shk, const char *const *nlp) {
    int i, trycnt, names_avail;
    const char *shname = 0;
    struct monst *mtmp;
    int name_wanted;
    s_level *sptr;

    if (nlp == shklight && In_mines(&u.uz)
            && (sptr = Is_special(&u.uz)) != 0 && sptr->flags.town) {
        /* special-case minetown lighting shk */
        shname = "Izchak";
        shk->female = false;
    } else {
        /* We want variation from game to game, without needing the save
           and restore support which would be necessary for randomization;
           try not to make too many assumptions about time_t's internals;
           use ledger_no rather than depth to keep mine town distinct. */
        int nseed = (int)((long)u.ubirthday / 257L);

        name_wanted = ledger_no(&u.uz) + (nseed % 13) - (nseed % 5);
        if (name_wanted < 0) name_wanted += (13 + 5);
        shk->female = name_wanted & 1;

        for (names_avail = 0; nlp[names_avail]; names_avail++)
            continue;

        for (trycnt = 0; trycnt < 50; trycnt++) {
            if (nlp == shktools) {
                shname = shktools[rn2(names_avail)];
                shk->female = (*shname == '_');
                if (shk->female) shname++;
            } else if (name_wanted < names_avail) {
                shname = nlp[name_wanted];
            } else if ((i = rn2(names_avail)) != 0) {
                shname = nlp[i - 1];
            } else if (nlp != shkgeneral) {
                nlp = shkgeneral;   /* try general names */
                for (names_avail = 0; nlp[names_avail]; names_avail++)
                    continue;
                continue;           /* next `trycnt' iteration */
            } else {
                shname = shk->female ? "Lucrezia" : "Dirk";
            }

            /* is name already in use on this level? */
            for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
                if (DEADMONSTER(mtmp) || (mtmp == shk) || !mtmp->isshk) continue;
                if (strcmp(ESHK(mtmp)->shknam, shname)) continue;
                break;
            }
            if (!mtmp) break;       /* new name */
        }
    }
    (void) strncpy(ESHK(shk)->shknam, shname, PL_NSIZ);
    ESHK(shk)->shknam[PL_NSIZ-1] = 0;
}

/* create a new shopkeeper in the given room */
static int shkinit ( const struct shclass *shp, struct mkroom *sroom) {
    int sh, sx, sy;
    struct monst *shk;

    /* place the shopkeeper in the given room */
    sh = sroom->fdoor;
    sx = doors[sh].x;
    sy = doors[sh].y;

    /* check that the shopkeeper placement is sane */
    if(sroom->irregular) {
        int rmno = (sroom - rooms) + ROOMOFFSET;
        if (isok(sx-1,sy) && !levl[sx-1][sy].edge &&
                (int) levl[sx-1][sy].roomno == rmno) sx--;
        else if (isok(sx+1,sy) && !levl[sx+1][sy].edge &&
                (int) levl[sx+1][sy].roomno == rmno) sx++;
        else if (isok(sx,sy-1) && !levl[sx][sy-1].edge &&
                (int) levl[sx][sy-1].roomno == rmno) sy--;
        else if (isok(sx,sy+1) && !levl[sx][sy+1].edge &&
                (int) levl[sx][sy+1].roomno == rmno) sx++;
        else goto shk_failed;
    }
    else if(sx == sroom->lx-1) sx++;
    else if(sx == sroom->hx+1) sx--;
    else if(sy == sroom->ly-1) sy++;
    else if(sy == sroom->hy+1) sy--; else {
shk_failed:
        return(-1);
    }

    if(MON_AT(sx, sy)) (void) rloc(m_at(sx, sy), false); /* insurance */

    /* now initialize the shopkeeper monster structure */
    if(!(shk = makemon(&mons[PM_SHOPKEEPER], sx, sy, NO_MM_FLAGS)))
        return(-1);
    shk->isshk = shk->mpeaceful = 1;
    set_malign(shk);
    shk->msleeping = 0;
    shk->mtrapseen = ~0;    /* we know all the traps already */
    ESHK(shk)->shoproom = (sroom - rooms) + ROOMOFFSET;
    sroom->resident = shk;
    ESHK(shk)->shoptype = sroom->rtype;
    assign_level(&(ESHK(shk)->shoplevel), &u.uz);
    ESHK(shk)->shd = doors[sh];
    ESHK(shk)->shk.x = sx;
    ESHK(shk)->shk.y = sy;
    ESHK(shk)->robbed = 0L;
    ESHK(shk)->credit = 0L;
    ESHK(shk)->debit = 0L;
    ESHK(shk)->loan = 0L;
    ESHK(shk)->visitct = 0;
    ESHK(shk)->following = 0;
    ESHK(shk)->billct = 0;
    shk->mgold = 1000L + 30L*(long)rnd(100);        /* initial capital */
    if (shp->shknms == shkrings)
        (void) mongets(shk, TOUCHSTONE);
    nameshk(shk, shp->shknms);

    return(sh);
}

/* stock a newly-created room with objects */
void stock_room (int shp_indx, struct mkroom *sroom) {
    /*
     * Someday soon we'll dispatch on the shdist field of shclass to do
     * different placements in this routine. Currently it only supports
     * shop-style placement (all squares except a row nearest the first
     * door get objects).
     */
    int sx, sy, sh;
    char buf[BUFSZ];
    int rmno = (sroom - rooms) + ROOMOFFSET;
    const struct shclass *shp = &shtypes[shp_indx];

    /* first, try to place a shopkeeper in the room */
    if ((sh = shkinit(shp, sroom)) < 0)
        return;

    /* make sure no doorways without doors, and no */
    /* trapped doors, in shops.                    */
    sx = doors[sroom->fdoor].x;
    sy = doors[sroom->fdoor].y;

    if(levl[sx][sy].doormask == D_NODOOR) {
        levl[sx][sy].doormask = D_ISOPEN;
        newsym(sx,sy);
    }
    if(levl[sx][sy].typ == SDOOR) {
        cvt_sdoor_to_door(&levl[sx][sy]);   /* .typ = DOOR */
        newsym(sx,sy);
    }
    if(levl[sx][sy].doormask & D_TRAPPED)
        levl[sx][sy].doormask = D_LOCKED;

    if(levl[sx][sy].doormask == D_LOCKED) {
        int m = sx, n = sy;

        if(inside_shop(sx+1,sy)) m--;
        else if(inside_shop(sx-1,sy)) m++;
        if(inside_shop(sx,sy+1)) n--;
        else if(inside_shop(sx,sy-1)) n++;
        sprintf(buf, "Closed for inventory");
        make_engr_at(m, n, buf, 0L, DUST);
    }

    for(sx = sroom->lx; sx <= sroom->hx; sx++)
        for(sy = sroom->ly; sy <= sroom->hy; sy++) {
            if(sroom->irregular) {
                if (levl[sx][sy].edge || (int) levl[sx][sy].roomno != rmno ||
                        distmin(sx, sy, doors[sh].x, doors[sh].y) <= 1)
                    continue;
            } else if((sx == sroom->lx && doors[sh].x == sx-1) ||
                    (sx == sroom->hx && doors[sh].x == sx+1) ||
                    (sy == sroom->ly && doors[sh].y == sy-1) ||
                    (sy == sroom->hy && doors[sh].y == sy+1)) continue;
            mkshobj_at(shp, sx, sy);
        }

    /*
     * Special monster placements (if any) should go here: that way,
     * monsters will sit on top of objects and not the other way around.
     */

    level.flags.has_shop = true;
}


/* does shkp's shop stock this item type? */
bool saleable (struct monst *shkp, struct obj *obj) {
    int i, shp_indx = ESHK(shkp)->shoptype - SHOPBASE;
    const struct shclass *shp = &shtypes[shp_indx];

    if (shp->symb == RANDOM_CLASS) return true;
    else for (i = 0; i < SIZE(shtypes[0].iprobs) && shp->iprobs[i].iprob; i++)
        if (shp->iprobs[i].itype < 0 ?
                shp->iprobs[i].itype == - obj->otyp :
                shp->iprobs[i].itype == obj->oclass) return true;
    /* not found */
    return false;
}

/* positive value: class; negative value: specific object type */
int get_shop_item (int type) {
    const struct shclass *shp = shtypes+type;
    int i,j;

    /* select an appropriate object type at random */
    for(j = rnd(100), i = 0; (j -= shp->iprobs[i].iprob) > 0; i++)
        continue;

    return shp->iprobs[i].itype;
}
