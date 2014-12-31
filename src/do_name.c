/* See LICENSE in the root of this project for change info */

#include "do_name.h"
#include "hack.h"
#include "pm_props.h"
#include "invent.h"
#include "shk.h"
#include "priest.h"
#include "objnam.h"
#include "winprocs.h"
#include "display.h"
#include "color.h"
#include "util.h"

extern const char what_is_an_unknown_object[];          /* from pager.c */

/* Aliases for road-runner nemesis
 */
static const char * const coynames[] = {
        "Carnivorous Vulgaris","Road-Runnerus Digestus",
        "Eatibus Anythingus"  ,"Famishus-Famishus",
        "Eatibus Almost Anythingus","Eatius Birdius",
        "Famishius Fantasticus","Eternalii Famishiis",
        "Famishus Vulgarus","Famishius Vulgaris Ingeniusi",
        "Eatius-Slobbius","Hardheadipus Oedipus",
        "Carnivorous Slobbius","Hard-Headipus Ravenus",
        "Evereadii Eatibus","Apetitius Giganticus",
        "Hungrii Flea-Bagius","Overconfidentii Vulgaris",
        "Caninus Nervous Rex","Grotesques Appetitus",
        "Nemesis Riduclii","Canis latrans"
};

static const char * const hcolors[] = {
        "ultraviolet", "infrared", "bluish-orange",
        "reddish-green", "dark white", "light black", "sky blue-pink",
        "salty", "sweet", "sour", "bitter",
        "striped", "spiral", "swirly", "plaid", "checkered", "argyle",
        "paisley", "blotchy", "guernsey-spotted", "polka-dotted",
        "square", "round", "triangular",
        "cabernet", "sangria", "fuchsia", "wisteria",
        "lemon-lime", "strawberry-banana", "peppermint",
        "romantic", "incandescent"
};

static const char callable[] = {
        SCROLL_CLASS, POTION_CLASS, WAND_CLASS, RING_CLASS, AMULET_CLASS,
        GEM_CLASS, SPBOOK_CLASS, ARMOR_CLASS, TOOL_CLASS, 0 };

static const char * const ghostnames[] = {
        /* these names should have length < PL_NSIZ */
        /* Capitalize the names for aesthetics -dgk */
        "Adri", "Andries", "Andreas", "Bert", "David", "Dirk", "Emile",
        "Frans", "Fred", "Greg", "Hether", "Jay", "John", "Jon", "Karnov",
        "Kay", "Kenny", "Kevin", "Maud", "Michiel", "Mike", "Peter", "Robert",
        "Ron", "Tom", "Wilmar", "Nick Danger", "Phoenix", "Jiro", "Mizue",
        "Stephan", "Lance Braccus", "Shadowhawk"
};

static const char * const bogusmons[] = {
        "jumbo shrimp", "giant pigmy", "gnu", "killer penguin",
        "giant cockroach", "giant slug", "maggot", "pterodactyl",
        "tyrannosaurus rex", "basilisk", "beholder", "nightmare",
        "efreeti", "marid", "rot grub", "bookworm", "master lichen",
        "shadow", "hologram", "jester", "attorney", "sleazoid",
        "killer tomato", "amazon", "robot", "battlemech",
        "rhinovirus", "harpy", "lion-dog", "rat-ant", "Y2K bug",
        "feminist",
                                                /* misc. */
        "grue", "Christmas-tree monster", "luck sucker", "paskald",
        "brogmoid", "dornbeast",                /* Quendor (Zork, &c.) */
        "Ancient Multi-Hued Dragon", "Evil Iggy",
                                                /* Moria */
        "emu", "kestrel", "xeroc", "venus flytrap",
                                                /* Rogue */
        "creeping coins",                       /* Wizardry */
        "hydra", "siren",                       /* Greek legend */
        "killer bunny",                         /* Monty Python */
        "rodent of unusual size",               /* The Princess Bride */
        "Smokey the bear",      /* "Only you can prevent forest fires!" */
        "Luggage",                              /* Discworld */
        "Ent",                                  /* Lord of the Rings */
        "tangle tree", "nickelpede", "wiggle",  /* Xanth */
        "white rabbit", "snark",                /* Lewis Carroll */
        "pushmi-pullyu",                        /* Dr. Doolittle */
        "smurf",                                /* The Smurfs */
        "tribble", "Klingon", "Borg",           /* Star Trek */
        "Ewok",                                 /* Star Wars */
        "Totoro",                               /* Tonari no Totoro */
        "ohmu",                                 /* Nausicaa */
        "youma",                                /* Sailor Moon */
        "nyaasu",                               /* Pokemon (Meowth) */
        "Godzilla", "King Kong",                /* monster movies */
        "earthquake beast",                     /* old L of SH */
        "Invid",                                /* Robotech */
        "Terminator",                           /* The Terminator */
        "boomer",                               /* Bubblegum Crisis */
        "Dalek",                                /* Dr. Who ("Exterminate!") */
        "microscopic space fleet", "Ravenous Bugblatter Beast of Traal",
                                                /* HGttG */
        "teenage mutant ninja turtle",          /* TMNT */
        "samurai rabbit",                       /* Usagi Yojimbo */
        "aardvark",                             /* Cerebus */
        "Audrey II",                            /* Little Shop of Horrors */
        "witch doctor", "one-eyed one-horned flying purple people eater",
                                                /* 50's rock 'n' roll */
        "Barney the dinosaur",                  /* saccharine kiddy TV */
        "Morgoth",                              /* Angband */
        "Vorlon",                               /* Babylon 5 */
        "questing beast",               /* King Arthur */
        "Predator",                             /* Movie */
        "mother-in-law"                         /* common pest */
};

static const char pick_chars[] = ".,;:";

/* the response for '?' help request in getpos() */
static void getpos_help(bool force, const char *goal) {
    char sbuf[BUFSZ];
    bool doing_what_is;
    winid tmpwin = create_nhwindow(NHW_MENU);

    sprintf(sbuf, "Use [%s] to move the cursor to %s.",
            iflags.num_pad ? "2468" : "hjkl", goal);
    putstr(tmpwin, 0, sbuf);
    putstr(tmpwin, 0, "Use [HJKL] to move the cursor 8 units at a time.");
    putstr(tmpwin, 0, "Or enter a background symbol (ex. <).");
    /* disgusting hack; the alternate selection characters work for any
       getpos call, but they only matter for dowhatis (and doquickwhatis) */
    doing_what_is = (goal == what_is_an_unknown_object);
    sprintf(sbuf, "Type a .%s when you are at the right place.",
            doing_what_is ? " or , or ; or :" : "");
    putstr(tmpwin, 0, sbuf);
    if (!force)
        putstr(tmpwin, 0, "Type Space or Escape when you're done.");
    putstr(tmpwin, 0, "");
    display_nhwindow(tmpwin, true);
    destroy_nhwindow(tmpwin);
}

int getpos(coord *cc, bool force, const char *goal) {
    int result = 0;
    int cx, cy, i, c;
    int sidx, tx, ty;
    bool msg_given = true;   /* clear message window by default */
    const char *cp;
    const char *sdp;
    if(iflags.num_pad) sdp = ndir; else sdp = sdir;     /* DICE workaround */

    if (flags.verbose) {
        pline("(For instructions type a ?)");
        msg_given = true;
    }
    cx = cc->x;
    cy = cc->y;
    curs(WIN_MAP, cx,cy);
    flush_screen(0);
    for (;;) {
        c = nh_poskey(&tx, &ty, &sidx);
        if (c == '\033') {
            cx = cy = -10;
            msg_given = true;   /* force clear */
            result = -1;
            break;
        }
        if(c == 0) {
            if (!isok(tx, ty)) continue;
            /* a mouse click event, just assign and return */
            cx = tx;
            cy = ty;
            break;
        }
        if ((cp = index(pick_chars, c)) != 0) {
            /* '.' => 0, ',' => 1, ';' => 2, ':' => 3 */
            result = cp - pick_chars;
            break;
        }
        for (i = 0; i < 8; i++) {
            int dx, dy;

            if (sdp[i] == c) {
                /* a normal movement letter or digit */
                dx = xdir[i];
                dy = ydir[i];
            } else if (sdir[i] == lowc((char)c)) {
                /* a shifted movement letter */
                dx = 8 * xdir[i];
                dy = 8 * ydir[i];
            } else
                continue;

            /* truncate at map edge; diagonal moves complicate this... */
            if (cx + dx < 1) {
                dy -= sgn(dy) * (1 - (cx + dx));
                dx = 1 - cx;            /* so that (cx+dx == 1) */
            } else if (cx + dx > COLNO-1) {
                dy += sgn(dy) * ((COLNO-1) - (cx + dx));
                dx = (COLNO-1) - cx;
            }
            if (cy + dy < 0) {
                dx -= sgn(dx) * (0 - (cy + dy));
                dy = 0 - cy;            /* so that (cy+dy == 0) */
            } else if (cy + dy > ROWNO-1) {
                dx += sgn(dx) * ((ROWNO-1) - (cy + dy));
                dy = (ROWNO-1) - cy;
            }
            cx += dx;
            cy += dy;
            goto nxtc;
        }

        if(c == '?'){
            getpos_help(force, goal);
        } else {
            if (!index(quitchars, c)) {
                char matching[MAXPCHARS];
                int pass, lo_x, lo_y, hi_x, hi_y, k = 0;
                (void)memset((void *)matching, 0, sizeof matching);
                for (sidx = 1; sidx < MAXPCHARS; sidx++)
                    if (c == defsyms[sidx].sym || c == (int)showsyms[sidx])
                        matching[sidx] = (char) ++k;
                if (k) {
                    for (pass = 0; pass <= 1; pass++) {
                        /* pass 0: just past current pos to lower right;
                           pass 1: upper left corner to current pos */
                        lo_y = (pass == 0) ? cy : 0;
                        hi_y = (pass == 0) ? ROWNO - 1 : cy;
                        for (ty = lo_y; ty <= hi_y; ty++) {
                            lo_x = (pass == 0 && ty == lo_y) ? cx + 1 : 1;
                            hi_x = (pass == 1 && ty == hi_y) ? cx : COLNO - 1;
                            for (tx = lo_x; tx <= hi_x; tx++) {
                                k = levl[tx][ty].glyph;
                                if (glyph_is_cmap(k) &&
                                        matching[glyph_to_cmap(k)]) {
                                    cx = tx,  cy = ty;
                                    if (msg_given) {
                                        clear_nhwindow(WIN_MESSAGE);
                                        msg_given = false;
                                    }
                                    goto nxtc;
                                }
                            }   /* column */
                        }       /* row */
                    }           /* pass */
                    pline("Can't find dungeon feature '%c'.", c);
                    msg_given = true;
                    goto nxtc;
                } else {
                    pline("Unknown direction: '%s' (%s).",
                          visctrl((char)c),
                          !force ? "aborted" :
                          iflags.num_pad ? "use 2468 or ." : "use hjkl or .");
                    msg_given = true;
                } /* k => matching */
            } /* !quitchars */
            if (force) goto nxtc;
            pline("Done.");
            msg_given = false;  /* suppress clear */
            cx = -1;
            cy = 0;
            result = 0; /* not -1 */
            break;
        }
    nxtc:       ;
        curs(WIN_MAP,cx,cy);
        flush_screen(0);
    }
    if (msg_given) clear_nhwindow(WIN_MESSAGE);
    cc->x = cx;
    cc->y = cy;
    return result;
}

struct monst * christen_monst (struct monst *mtmp, const char *name) {
    int lth;
    struct monst *mtmp2;
    char buf[PL_PSIZ];

    /* dogname & catname are PL_PSIZ arrays; object names have same limit */
    lth = *name ? (int)(strlen(name) + 1) : 0;
    if(lth > PL_PSIZ){
        lth = PL_PSIZ;
        name = strncpy(buf, name, PL_PSIZ - 1);
        buf[PL_PSIZ - 1] = '\0';
    }
    if (lth == mtmp->mnamelth) {
        /* don't need to allocate a new monst struct */
        if (lth) set_monster_name(mtmp, name);
        return mtmp;
    }
    mtmp2 = newmonst(mtmp->mxlth + lth);
    *mtmp2 = *mtmp;
    (void) memcpy((void *)mtmp2->mextra,
            (void *)mtmp->mextra, mtmp->mxlth);
    mtmp2->mnamelth = lth;
    if (lth) set_monster_name(mtmp2, name);
    replmon(mtmp,mtmp2);
    return(mtmp2);
}

int do_mname (void) {
    coord cc;
    int cx,cy;
    struct monst *mtmp;
    char qbuf[QBUFSZ];

    if (Hallucination()) {
        You("would never recognize it anyway.");
        return 0;
    }
    cc.x = u.ux;
    cc.y = u.uy;
    if (getpos(&cc, false, "the monster you want to name") < 0 ||
            (cx = cc.x) < 0)
        return 0;
    cy = cc.y;

    if (cx == u.ux && cy == u.uy) {
        if (u.usteed && canspotmon(u.usteed))
            mtmp = u.usteed;
        else {
            pline("This %s creature is called %s and cannot be renamed.",
                    ACURR(A_CHA) > 14 ?
                    (flags.female ? "beautiful" : "handsome") :
                    "ugly",
                    plname);
            return(0);
        }
    } else
        mtmp = m_at(cx, cy);

    if (!mtmp || (!sensemon(mtmp) &&
                (!(cansee(cx,cy) || see_with_infrared(mtmp)) || mtmp->mundetected
                 || mtmp->m_ap_type == M_AP_FURNITURE
                 || mtmp->m_ap_type == M_AP_OBJECT
                 || (mtmp->minvis && !See_invisible)))) {
        pline("I see no monster there.");
        return(0);
    }
    /* special case similar to the one in lookat() */
    char buf[BUFSZ];
    distant_monnam(buf, BUFSZ, mtmp, ARTICLE_THE);
    sprintf(qbuf, "What do you want to call %s?", buf);
    getlin(qbuf,buf);
    if(!*buf || *buf == '\033') return(0);
    /* strip leading and trailing spaces; unnames monster if all spaces */
    mungspaces(buf);

    if (mtmp->data->geno & G_UNIQ) {
        char name[BUFSZ];
        Monnam(name, BUFSZ, mtmp);
        pline("%s doesn't like being called names!", name);
    } else {
        christen_monst(mtmp, buf);
    }
    return 0;
}

/*
 * This routine changes the address of obj. Be careful not to call it
 * when there might be pointers around in unknown places. For now: only
 * when obj is in the inventory.
 */
static void do_oname (struct obj *obj) {
    char buf[BUFSZ], qbuf[QBUFSZ];
    const char *aname;
    short objtyp;

    sprintf(qbuf, "What do you want to name %s %s?", is_plural(obj) ? "these" : "this", xname(obj));
    getlin(qbuf, buf);
    if(!*buf || *buf == '\033')     return;
    /* strip leading and trailing spaces; unnames item if all spaces */
    (void)mungspaces(buf);

    /* relax restrictions over proper capitalization for artifacts */
    if ((aname = artifact_name(buf, &objtyp)) != 0 && objtyp == obj->otyp)
        strcpy(buf, aname);

    if (obj->oartifact) {
        pline_The("artifact seems to resist the attempt.");
        return;
    } else if (restrict_name(obj, buf) || exist_artifact(obj->otyp, buf)) {
        int n = rn2((int)strlen(buf));
        char c1, c2;

        c1 = lowc(buf[n]);
        do c2 = 'a' + rn2('z'-'a'); while (c1 == c2);
        buf[n] = (buf[n] == c1) ? c2 : highc(c2);  /* keep same case */
        pline("While engraving your %s slips.", body_part(HAND));
        display_nhwindow(WIN_MESSAGE, false);
        You("engrave: \"%s\".",buf);
    }
    obj = oname(obj, buf);
}

/*
 * Allocate a new and possibly larger storage space for an obj.
 */
//  int oextra_size,            /* storage to allocate for oextra            */
//  int oname_size,                     /* size of name string + 1 (null terminator) */
struct obj * realloc_obj ( struct obj *obj, int oextra_size, void *oextra_src,
        int oname_size, const char *name)
{
        struct obj *otmp;

        otmp = newobj(oextra_size + oname_size);
        *otmp = *obj;   /* the cobj pointer is copied to otmp */
        if (oextra_size) {
            if (oextra_src)
                (void) memcpy((void *)otmp->oextra, oextra_src,
                                                        oextra_size);
        } else {
            otmp->oattached = OATTACHED_NOTHING;
        }
        otmp->oxlth = oextra_size;

        otmp->onamelth = oname_size;
        otmp->timed = 0;        /* not timed, yet */
        otmp->lamplit = 0;      /* ditto */
        /* __GNUC__ note:  if the assignment of otmp->onamelth immediately
           precedes this `if' statement, a gcc bug will miscompile the
           test on vax (`insv' instruction used to store bitfield does
           not set condition codes, but optimizer behaves as if it did).
           gcc-2.7.2.1 finally fixed this. */
        if (oname_size) {
            if (name)
                strcpy(ONAME(otmp), name);
        }

        if (obj->owornmask) {
                bool save_twoweap = u.twoweap;
                /* unwearing the old instance will clear dual-wield mode
                   if this object is either of the two weapons */
                setworn((struct obj *)0, obj->owornmask);
                setworn(otmp, otmp->owornmask);
                u.twoweap = save_twoweap;
        }

        /* replace obj with otmp */
        replace_object(obj, otmp);

        /* fix ocontainer pointers */
        if (Has_contents(obj)) {
                struct obj *inside;

                for(inside = obj->cobj; inside; inside = inside->nobj)
                        inside->ocontainer = otmp;
        }

        /* move timers and light sources from obj to otmp */
        if (obj->timed) obj_move_timers(obj, otmp);
        if (obj->lamplit) obj_move_light_source(obj, otmp);

        /* objects possibly being manipulated by multi-turn occupations
           which have been interrupted but might be subsequently resumed */
        if (obj->oclass == FOOD_CLASS)
            food_substitution(obj, otmp);       /* eat food or open tin */
        else if (obj->oclass == SPBOOK_CLASS)
            book_substitution(obj, otmp);       /* read spellbook */

        /* obfree(obj, otmp);   now unnecessary: no pointers on bill */
        dealloc_obj(obj);       /* let us hope nobody else saved a pointer */
        return otmp;
}

struct obj * oname (struct obj *obj, const char *name) {
        int lth;
        char buf[PL_PSIZ];

        lth = *name ? (int)(strlen(name) + 1) : 0;
        if (lth > PL_PSIZ) {
                lth = PL_PSIZ;
                name = strncpy(buf, name, PL_PSIZ - 1);
                buf[PL_PSIZ - 1] = '\0';
        }
        /* If named artifact exists in the game, do not create another.
         * Also trying to create an artifact shouldn't de-artifact
         * it (e.g. Excalibur from prayer). In this case the object
         * will retain its current name. */
        if (obj->oartifact || (lth && exist_artifact(obj->otyp, name)))
                return obj;

        if (lth == obj->onamelth) {
                /* no need to replace entire object */
                if (lth) strcpy(ONAME(obj), name);
        } else {
                obj = realloc_obj(obj, obj->oxlth,
                              (void *)obj->oextra, lth, name);
        }
        if (lth) artifact_exists(obj, name, true);
        if (obj->oartifact) {
            /* can't dual-wield with artifact as secondary weapon */
            if (obj == uswapwep) untwoweapon();
            /* activate warning if you've just named your weapon "Sting" */
            if (obj == uwep) set_artifact_intrinsic(obj, true, W_WEP);
        }
        if (carried(obj)) update_inventory();
        return obj;
}

int ddocall (void) {
        struct obj *obj;
        char    ch;
        char allowall[2];

        switch(
                ch =
                ynq("Name an individual object?")) {
        case 'q':
                break;
        case 'y':
                savech(ch);
                allowall[0] = ALL_CLASSES; allowall[1] = '\0';
                obj = getobj(allowall, "name");
                if(obj) do_oname(obj);
                break;
        default :
                savech(ch);
                obj = getobj(callable, "call");
                if (obj) {
                        /* behave as if examining it in inventory;
                           this might set dknown if it was picked up
                           while blind and the hero can now see */
                        (void) xname(obj);

                        if (!obj->dknown) {
                                You("would never recognize another one.");
                                return 0;
                        }
                        docall(obj);
                }
                break;
        }
        return 0;
}

void docall (struct obj *obj) {
        char buf[BUFSZ], qbuf[QBUFSZ];
        struct obj otemp;
        char **str1;

        if (!obj->dknown) return; /* probably blind */
        otemp = *obj;
        otemp.quan = 1L;
        otemp.onamelth = 0;
        otemp.oxlth = 0;
        if (objects[otemp.otyp].oc_class == POTION_CLASS && otemp.fromsink)
            /* kludge, meaning it's sink water */
            sprintf(qbuf,"Call a stream of %s fluid:",
                    OBJ_DESCR(objects[otemp.otyp]));
        else
            sprintf(qbuf, "Call %s:", an(xname(&otemp)));
        getlin(qbuf, buf);
        if(!*buf || *buf == '\033')
                return;

        /* clear old name */
        str1 = &(objects[obj->otyp].oc_uname);
        if(*str1) free((void *)*str1);

        /* strip leading and trailing spaces; uncalls item if all spaces */
        (void)mungspaces(buf);
        if (!*buf) {
            if (*str1) {        /* had name, so possibly remove from disco[] */
                /* strip name first, for the update_inventory() call
                   from undiscover_object() */
                *str1 = (char *)0;
                undiscover_object(obj->otyp);
            }
        } else {
            *str1 = strcpy((char *) malloc((unsigned)strlen(buf)+1), buf);
            discover_object(obj->otyp, false, true); /* possibly add to disco[] */
        }
}

/* ghost names formerly set by x_monnam(), now by makemon() instead */
const char * rndghostname (void) {
    return rn2(7) ? ghostnames[rn2(SIZE(ghostnames))] : (const char *)plname;
}

/* Monster naming functions:
 * x_monnam is the generic monster-naming function.
 *                seen        unseen       detected               named
 * mon_nam:     the newt        it      the invisible orc       Fido
 * noit_mon_nam:the newt (as if detected) the invisible orc     Fido
 * l_monnam:    newt            it      invisible orc           dog called fido
 * Monnam:      The newt        It      The invisible orc       Fido
 * noit_Monnam: The newt (as if detected) The invisible orc     Fido
 * Adjmonnam:   The poor newt   It      The poor invisible orc  The poor Fido
 * Amonnam:     A newt          It      An invisible orc        Fido
 * a_monnam:    a newt          it      an invisible orc        Fido
 * m_monnam:    newt            xan     orc                     Fido
 * y_monnam:    your newt     your xan  your invisible orc      Fido
 */

/* Bug: if the monster is a priest or shopkeeper, not every one of these
 * options works, since those are special cases.
 */
/*
struct monst *mtmp;
int article;
 ARTICLE_NONE, ARTICLE_THE, ARTICLE_A: obvious
 * ARTICLE_YOUR: "your" on pets, "the" on everything else
 *
 * If the monster would be referred to as "it" or if the monster has a name
 * _and_ there is no adjective, "invisible", "saddled", etc., override this
 * and always use no article.
 
const char *adjective;
int suppress;
 SUPPRESS_IT, SUPPRESS_INVISIBLE, SUPPRESS_HALLUCINATION, SUPPRESS_SADDLE.
 * EXACT_NAME: combination of all the above
*/
size_t x_monnam(char *out_buf, int buf_size, const struct monst *mtmp, int article,
        const char *adjective, int suppress, bool called)
{
    struct permonst *mdat = mtmp->data;
    char *bp;

    if (program_state.gameover)
        suppress |= SUPPRESS_HALLUCINATION;
    if (article == ARTICLE_YOUR && !mtmp->mtame)
        article = ARTICLE_THE;

    bool do_hallu = Hallucination() && !(suppress & SUPPRESS_HALLUCINATION);
    bool do_invis = mtmp->minvis && !(suppress & SUPPRESS_INVISIBLE);
    bool do_it = !canspotmon(mtmp) &&
        article != ARTICLE_YOUR &&
        !program_state.gameover &&
        mtmp != u.usteed &&
        !(u.uswallow && mtmp == u.ustuck) &&
        !(suppress & SUPPRESS_IT);
    bool do_saddle = !(suppress & SUPPRESS_SADDLE);

    /* unseen monsters, etc.  Use "it" */
    if (do_it)
        return nh_strlcpy(out_buf, "it", buf_size);

    /* priests and minions: don't even use this function */
    if (mtmp->ispriest || mtmp->isminion) {
        char buf[BUFSZ];
        char *name = buf;
        priestname(name, BUFSZ, mtmp, true);

        if (article == ARTICLE_NONE && !strncmp(name, "the ", 4))
            name += 4;
        return nh_strlcpy(out_buf, name, buf_size);
    }

    /* Shopkeepers: use shopkeeper name.  For normal shopkeepers, just
     * "Asidonhopo"; for unusual ones, "Asidonhopo the invisible
     * shopkeeper" or "Asidonhopo the blue dragon".  If hallucinating,
     * none of this applies.
     */
    if (mtmp->isshk && !do_hallu) {
        if (adjective && article == ARTICLE_THE) {
            /* pathological case: "the angry Asidonhopo the blue dragon"
               sounds silly */
            return nh_slprintf(out_buf, buf_size, "the %s %s", adjective, shkname(mtmp));
        }
        if (mdat == &mons[PM_SHOPKEEPER] && !do_invis)
            return nh_strlcpy(out_buf, shkname(mtmp), buf_size);

        return nh_slprintf(out_buf, buf_size, "%s the %s%s",
                shkname(mtmp),
                do_invis ? "invisible " : "",
                mdat->mname);
    }

    /* Put the adjectives in the buffer */
    bool has_adjectives = false;
    const char *first_adjective = "";
    const char *first_space = "";
    const char *invisible = "";
    const char *saddled = "";
    const char *noun = "";

    if (adjective) {
        first_adjective = adjective;
        first_space = " ";
        has_adjectives = true;
    }
    if (do_invis) {
        invisible = "invisible ";
        has_adjectives = true;
    }
    if (do_saddle && (mtmp->misc_worn_check & W_SADDLE) && !Blind && !Hallucination()) {
        saddled = "saddled ";
        has_adjectives = true;
    }
    bool name_at_start = false;
    if (do_hallu) {
        noun = rndmonnam();
    } else if (mtmp->mnamelth) {
        const char *name = monster_name(mtmp);
        if (mdat == &mons[PM_GHOST]) {
            char ghost_buf[BUFSZ];
            nh_slprintf(ghost_buf, BUFSZ, "%s%s ghost", name, possessive_suffix(name));
            noun = ghost_buf;
            name_at_start = true;
        } else if (called) {
            char named_monster_buf[BUFSZ];
            nh_slprintf(named_monster_buf, BUFSZ, "%s called %s", mdat->mname, name);
            noun = named_monster_buf;
            name_at_start = type_is_pname(mdat);
        } else if (is_mplayer(mdat) && (bp = strstri(name, " the ")) != 0) {
            // <name> the <adjective> <invisible> <saddled> <rank>
            char name_buf[BUFSZ];
            nh_strlcpy(name_buf, name, BUFSZ);
            name_buf[bp - name + 5] = '\0';
            noun = bp + 5;
            return nh_slprintf(out_buf, buf_size, "%s the %s%s%s%s%s", name_buf,
                    first_adjective, first_space, invisible, saddled, noun);
        } else {
            noun = name;
            name_at_start = true;
        }
    } else if (is_mplayer(mdat) && !In_endgame(&u.uz)) {
        char pbuf[BUFSZ];
        nh_strlcpy(pbuf, rank_of((int)mtmp->m_lev, monsndx(mdat), mtmp->female), BUFSZ);
        lcase(pbuf);
        noun = pbuf;
        name_at_start = false;
    } else {
        noun = mdat->mname;
        name_at_start = type_is_pname(mdat);
    }

    // first_adjective first_space invisible saddled noun

    if (name_at_start && (article == ARTICLE_YOUR || !has_adjectives)) {
        article = (mdat == &mons[PM_WIZARD_OF_YENDOR]) ? ARTICLE_THE : ARTICLE_NONE;
    } else if ((mdat->geno & G_UNIQ) && article == ARTICLE_A) {
        article = ARTICLE_THE;
    }

    switch (article) {
        case ARTICLE_YOUR:
            return nh_slprintf(out_buf, buf_size, "your %s%s%s%s%s",
                    first_adjective, first_space, invisible, saddled, noun);
        case ARTICLE_THE:
            return nh_slprintf(out_buf, buf_size, "the %s%s%s%s%s",
                    first_adjective, first_space, invisible, saddled, noun);
        case ARTICLE_A: {
            char an_buf[BUFSZ];
            nh_slprintf(an_buf, BUFSZ, "%s%s%s%s%s",
                    first_adjective, first_space, invisible, saddled, noun);
            return nh_slprintf(out_buf, buf_size, "%s%s", an_prefix(an_buf), an_buf);
        }
        case ARTICLE_NONE:
        default:
            return nh_slprintf(out_buf, buf_size, "%s%s%s%s%s",
                    first_adjective, first_space, invisible, saddled, noun);
    }
}

size_t l_monnam(char *out_buf, size_t buf_size, const struct monst *mtmp) {
    return x_monnam(out_buf, buf_size, mtmp, ARTICLE_NONE, NULL,
            mtmp->mnamelth ? SUPPRESS_SADDLE : 0, true);
}


size_t mon_nam(char *out_buf, size_t buf_size, const struct monst *mtmp) {
    return x_monnam(out_buf, buf_size, mtmp, ARTICLE_THE, NULL,
            mtmp->mnamelth ? SUPPRESS_SADDLE : 0, false);
}

/* print the name as if mon_nam() was called, but assume that the player
 * can always see the monster--used for probing and for monsters aggravating
 * the player with a cursed potion of invisibility
 */
size_t noit_mon_nam(char *out_buf, size_t buf_size, const struct monst *mtmp) {
    return x_monnam(out_buf, buf_size, mtmp, ARTICLE_THE, NULL,
            mtmp->mnamelth ? (SUPPRESS_SADDLE|SUPPRESS_IT) : SUPPRESS_IT, false);
}

size_t Monnam(char *out_buf, size_t buf_size, const struct monst *mtmp) {
    size_t ret = mon_nam(out_buf, buf_size, mtmp);
    *out_buf = highc(*out_buf);
    return ret;
}

size_t noit_Monnam(char *out_buf, size_t buf_size, const struct monst *mtmp) {
    size_t ret = noit_mon_nam(out_buf, buf_size, mtmp);
    *out_buf = highc(*out_buf);
    return ret;
}

// monster's own name
size_t m_monnam (char *out_buf, size_t buf_size, const struct monst *mtmp) {
    return x_monnam(out_buf, buf_size, mtmp, ARTICLE_NONE, NULL, EXACT_NAME, false);
}

// pet name: "your little dog"
size_t y_monnam (char *out_buf, size_t buf_size, const struct monst *mtmp) {
    int prefix = mtmp->mtame ? ARTICLE_YOUR : ARTICLE_THE;
    int suppression_flag = (mtmp->mnamelth
            // "saddled" is redundant when mounted
            || mtmp == u.usteed
            ) ? SUPPRESS_SADDLE : 0;

    return x_monnam(out_buf, buf_size, mtmp, prefix, NULL, suppression_flag, false);
}

size_t Adjmonnam(char *out_buf, size_t buf_size, const struct monst *mtmp, const char *adj) {
    size_t ret = x_monnam(out_buf, buf_size, mtmp, ARTICLE_THE, adj,
            mtmp->mnamelth ? SUPPRESS_SADDLE : 0, false);
    *out_buf = highc(*out_buf);
    return ret;
}

size_t a_monnam(char *out_buf, size_t buf_size, const struct monst *mtmp) {
    return x_monnam(out_buf, buf_size, mtmp, ARTICLE_A, NULL,
            mtmp->mnamelth ? SUPPRESS_SADDLE : 0, false);
}

size_t Amonnam (char *out_buf, size_t buf_size, const struct monst *mtmp) {
    size_t ret = a_monnam(out_buf, buf_size, mtmp);
    *out_buf = highc(*out_buf);
    return ret;
}

/* used for monster ID by the '/', ';', and 'C' commands to block remote
   identification of the endgame altars via their attending priests */
// int article,        /* only ARTICLE_NONE and ARTICLE_THE are handled here */
size_t distant_monnam(char *out_buf, size_t buf_size, const struct monst *mon, int article) {
    /* high priest(ess)'s identity is concealed on the Astral Plane,
       unless you're adjacent (overridden for hallucination which does
       its own obfuscation) */
    if (mon->data == &mons[PM_HIGH_PRIEST] && !Hallucination() &&
            Is_astralevel(&u.uz) && distu(mon->mx, mon->my) > 2)
    {
        return nh_slprintf(out_buf, buf_size, "%s%s",
            article == ARTICLE_THE ? "the " : "",
            mon->female ? "high priestess" : "high priest");
    } else {
        return x_monnam(out_buf, buf_size, mon, article, NULL, 0, true);
    }
}

/* Return a random monster name, for hallucination.
 * KNOWN BUG: May be a proper name (Godzilla, Barney), may not
 * (the Terminator, a Dalek).  There's no elegant way to deal
 * with this without radically modifying the calling functions.
 */
const char * rndmonnam (void) {
    int name;

    do {
        name = rn1(SPECIAL_PM + SIZE(bogusmons) - LOW_PM, LOW_PM);
    } while (name < SPECIAL_PM &&
            (type_is_pname(&mons[name]) || (mons[name].geno & G_NOGEN)));

    if (name >= SPECIAL_PM) return bogusmons[name - SPECIAL_PM];
    return mons[name].mname;
}

int halluc_color_int(void) {
    return rn2(SIZE(hcolors));
}

const char * hcolor (const char *colorpref) {
    return (Hallucination() || !colorpref) ?  hcolors[halluc_color_int] : colorpref;
}

/* return a random real color unless hallucinating */
const char * rndcolor (void) {
    int k = rn2(CLR_MAX);
    return Hallucination() ? hcolor((char *)0) : (k == NO_COLOR) ?
        "colorless" : c_obj_colors[k];
}

size_t coyotename (char *out_buf, size_t buf_size, const struct monst *mtmp) {
    char name[BUFSZ];
    x_monnam(name, BUFSZ, mtmp, ARTICLE_NONE, NULL, 0, true);
    return nh_slprintf(out_buf, buf_size, "%s - %s", name,
        mtmp->mcan ? coynames[SIZE(coynames)-1] : coynames[rn2(SIZE(coynames)-1)]);
}
