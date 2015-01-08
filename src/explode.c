/* See LICENSE in the root of this project for change info */

#include "explode.h"
#include "hack.h"
#include "invent.h"
#include "objnam.h"
#include "shk.h"
#include "do_name.h"
#include "display.h"
#include "everything.h"


/* Note: Arrays are column first, while the screen is row first */
static int expl[3][3] = {
        { S_explode1, S_explode4, S_explode7 },
        { S_explode2, S_explode5, S_explode8 },
        { S_explode3, S_explode6, S_explode9 }
};

struct scatter_chain {
        struct scatter_chain *next;     /* pointer to next scatter item */
        struct obj *obj;                /* pointer to the object        */
        signed char ox;                 /* location of                  */
        signed char oy;                 /*      item                    */
        signed char dx;                 /* direction of                 */
        signed char dy;                 /*      travel                  */
        int range;                      /* range of object              */
        bool stopped;                /* flag for in-motion/stopped   */
};


const char *render_killer_no_prefix(const struct Killer *k) {
    fprintf(stderr, "TODO: render_killer_no_prefix\n");
    return "TODO: render_killer_no_prefix";
}

/* Note: I had to choose one of three possible kinds of "type" when writing
 * this function: a wand type (like in zap.c), an adtyp, or an object type.
 * Wand types get complex because they must be converted to adtyps for
 * determining such things as fire resistance.  Adtyps get complex in that
 * they don't supply enough information--was it a player or a monster that
 * did it, and with a wand, spell, or breath weapon?  Object types share both
 * these disadvantages....
 */
// int type, /* the same as in zap.c */
void explode ( int x, int y, int type, int dam, char olet, int expltype) {
    int i, j, k, damu = dam;
    bool starting = 1;
    bool visible, any_shield;
    int uhurt = 0; /* 0=unhurt, 1=items damaged, 2=you and items damaged */
    const char *str;
    int idamres, idamnonres;
    struct monst *mtmp;
    unsigned char adtyp;
    int explmask[3][3];
    /* 0=normal explosion, 1=do shieldeff, 2=do nothing */
    bool shopdamage = false;
    bool generic = false;

    /* retributive strike */
    if (olet == WAND_CLASS) {
        switch (Role_switch) {
            case PM_PRIEST:
            case PM_MONK:
            case PM_WIZARD:
                damu /= 5;
                break;
            case PM_HEALER:
            case PM_KNIGHT:
                damu /= 2;
                break;
            default:  break;
        }
    }

    if (olet == MON_EXPLODE) {
        str = render_killer_no_prefix(&killer);
        killer.method = KM_DIED;         /* set again later as needed */
        adtyp = AD_PHYS;
    } else {
        switch (abs(type) % 10) {
            case 0:
                str = "magical blast";
                adtyp = AD_MAGM;
                break;
            case 1:
                str =   olet == BURNING_OIL ?   "burning oil" :
                    olet == SCROLL_CLASS ?  "tower of flame" :
                    "fireball";
                adtyp = AD_FIRE;
                break;
            case 2:
                str = "ball of cold";
                adtyp = AD_COLD;
                break;
            case 4:
                str =  (olet == WAND_CLASS) ? "death field" :
                    "disintegration field";
                adtyp = AD_DISN;
                break;
            case 5:
                str = "ball of lightning";
                adtyp = AD_ELEC;
                break;
            case 6:
                str = "poison gas cloud";
                adtyp = AD_DRST;
                break;
            case 7:
                str = "splash of acid";
                adtyp = AD_ACID;
                break;
            default: impossible("explosion base type %d?", type); return;
        }
    }

    any_shield = visible = false;
    for (i=0; i<3; i++) for (j=0; j<3; j++) {
        if (!isok(i+x-1, j+y-1)) {
            explmask[i][j] = 2;
            continue;
        } else
            explmask[i][j] = 0;

        if (i+x-1 == u.ux && j+y-1 == u.uy) {
            switch(adtyp) {
                case AD_PHYS:
                    explmask[i][j] = 0;
                    break;
                case AD_MAGM:
                    explmask[i][j] = !!Antimagic();
                    break;
                case AD_FIRE:
                    explmask[i][j] = !!Fire_resistance();
                    break;
                case AD_COLD:
                    explmask[i][j] = !!Cold_resistance();
                    break;
                case AD_DISN:
                    explmask[i][j] = (olet == WAND_CLASS) ?
                        !!(nonliving(youmonst.data) || is_demon(youmonst.data)) :
                        !!Disint_resistance();
                    break;
                case AD_ELEC:
                    explmask[i][j] = !!Shock_resistance();
                    break;
                case AD_DRST:
                    explmask[i][j] = !!Poison_resistance();
                    break;
                case AD_ACID:
                    explmask[i][j] = !!Acid_resistance;
                    break;
                default:
                    impossible("explosion type %d?", adtyp);
                    break;
            }
        }
        /* can be both you and mtmp if you're swallowed */
        mtmp = m_at(i+x-1, j+y-1);
        if (!mtmp && i+x-1 == u.ux && j+y-1 == u.uy)
            mtmp = u.usteed;
        if (mtmp) {
            if (mtmp->mhp < 1) explmask[i][j] = 2;
            else switch(adtyp) {
                case AD_PHYS:
                    break;
                case AD_MAGM:
                    explmask[i][j] |= resists_magm(mtmp);
                    break;
                case AD_FIRE:
                    explmask[i][j] |= resists_fire(mtmp);
                    break;
                case AD_COLD:
                    explmask[i][j] |= resists_cold(mtmp);
                    break;
                case AD_DISN:
                    explmask[i][j] |= (olet == WAND_CLASS) ?
                        (nonliving(mtmp->data) || is_demon(mtmp->data)) :
                        resists_disint(mtmp);
                    break;
                case AD_ELEC:
                    explmask[i][j] |= resists_elec(mtmp);
                    break;
                case AD_DRST:
                    explmask[i][j] |= resists_poison(mtmp);
                    break;
                case AD_ACID:
                    explmask[i][j] |= resists_acid(mtmp);
                    break;
                default:
                    impossible("explosion type %d?", adtyp);
                    break;
            }
        }
        if (mtmp && cansee(i+x-1,j+y-1) && !canspotmon(mtmp))
            map_invisible(i+x-1, j+y-1);
        else if (!mtmp && glyph_is_invisible(levl[i+x-1][j+y-1].glyph)) {
            unmap_object(i+x-1, j+y-1);
            newsym(i+x-1, j+y-1);
        }
        if (cansee(i+x-1, j+y-1)) visible = true;
        if (explmask[i][j] == 1) any_shield = true;
    }

    if (visible) {
        /* Start the explosion */
        for (i=0; i<3; i++) for (j=0; j<3; j++) {
            if (explmask[i][j] == 2) continue;
            tmp_at(starting ? DISP_BEAM : DISP_CHANGE,
                    explosion_to_glyph(expltype,expl[i][j]));
            tmp_at(i+x-1, j+y-1);
            starting = 0;
        }
        curs_on_u();    /* will flush screen and output */

        if (any_shield && flags.sparkle) { /* simulate shield effect */
            for (k = 0; k < SHIELD_COUNT; k++) {
                for (i=0; i<3; i++) for (j=0; j<3; j++) {
                    if (explmask[i][j] == 1)
                        /*
                         * Bypass tmp_at() and send the shield glyphs
                         * directly to the buffered screen.  tmp_at()
                         * will clean up the location for us later.
                         */
                        show_glyph(i+x-1, j+y-1,
                                cmap_to_glyph(shield_static[k]));
                }
                curs_on_u();    /* will flush screen and output */
            }

            /* Cover last shield glyph with blast symbol. */
            for (i=0; i<3; i++) for (j=0; j<3; j++) {
                if (explmask[i][j] == 1)
                    show_glyph(i+x-1,j+y-1,
                            explosion_to_glyph(expltype, expl[i][j]));
            }

        }

        tmp_at(DISP_END, 0); /* clear the explosion */
    } else {
        if (olet == MON_EXPLODE) {
            str = "explosion";
            generic = true;
        }
        if (flags.soundok) You_hear("a blast.");
    }

    if (dam)
        for (i=0; i<3; i++) for (j=0; j<3; j++) {
            if (explmask[i][j] == 2) continue;
            if (i+x-1 == u.ux && j+y-1 == u.uy)
                uhurt = (explmask[i][j] == 1) ? 1 : 2;
            idamres = idamnonres = 0;
            if (type >= 0)
                (void)zap_over_floor((signed char)(i+x-1), (signed char)(j+y-1),
                        type, &shopdamage);

            mtmp = m_at(i+x-1, j+y-1);
            if (!mtmp && i+x-1 == u.ux && j+y-1 == u.uy)
                mtmp = u.usteed;
            if (!mtmp) continue;
            if (u.uswallow && mtmp == u.ustuck) {
                char name[BUFSZ];
                Monnam(name, BUFSZ, u.ustuck);
                if (is_animal(u.ustuck->data)) {
                    pline("%s gets %s!",
                            name,
                            (adtyp == AD_FIRE) ? "heartburn" :
                            (adtyp == AD_COLD) ? "chilly" :
                            (adtyp == AD_DISN) ? ((olet == WAND_CLASS) ?
                                "irradiated by pure energy" : "perforated") :
                            (adtyp == AD_ELEC) ? "shocked" :
                            (adtyp == AD_DRST) ? "poisoned" :
                            (adtyp == AD_ACID) ? "an upset stomach" :
                            "fried");
                } else {
                    pline("%s gets slightly %s!",
                            name,
                            (adtyp == AD_FIRE) ? "toasted" :
                            (adtyp == AD_COLD) ? "chilly" :
                            (adtyp == AD_DISN) ? ((olet == WAND_CLASS) ?
                                "overwhelmed by pure energy" : "perforated") :
                            (adtyp == AD_ELEC) ? "shocked" :
                            (adtyp == AD_DRST) ? "intoxicated" :
                            (adtyp == AD_ACID) ? "burned" :
                            "fried");
                }
            } else if (cansee(i+x-1, j+y-1)) {
                if(mtmp->m_ap_type) seemimic(mtmp);
                char name[BUFSZ];
                Monnam(name, BUFSZ, mtmp);
                pline("%s is caught in the %s!", name, str);
            }

            idamres += destroy_mitem(mtmp, SCROLL_CLASS, (int) adtyp);
            idamres += destroy_mitem(mtmp, SPBOOK_CLASS, (int) adtyp);
            idamnonres += destroy_mitem(mtmp, POTION_CLASS, (int) adtyp);
            idamnonres += destroy_mitem(mtmp, WAND_CLASS, (int) adtyp);
            idamnonres += destroy_mitem(mtmp, RING_CLASS, (int) adtyp);

            if (explmask[i][j] == 1) {
                golemeffects(mtmp, (int) adtyp, dam + idamres);
                mtmp->mhp -= idamnonres;
            } else {
                /* call resist with 0 and do damage manually so 1) we can
                 * get out the message before doing the damage, and 2) we can
                 * call mondied, not killed, if it's not your blast
                 */
                int mdam = dam;

                if (resist(mtmp, olet, 0, false)) {
                    if (cansee(i+x-1,j+y-1)) {
                        char name[BUFSZ];
                        Monnam(name, BUFSZ, mtmp);
                        pline("%s resists the %s!", name, str);
                    }
                    mdam = dam/2;
                }
                if (mtmp == u.ustuck)
                    mdam *= 2;
                if (resists_cold(mtmp) && adtyp == AD_FIRE)
                    mdam *= 2;
                else if (resists_fire(mtmp) && adtyp == AD_COLD)
                    mdam *= 2;
                mtmp->mhp -= mdam;
                mtmp->mhp -= (idamres + idamnonres);
            }
            if (mtmp->mhp <= 0) {
                /* KMH -- Don't blame the player for pets killing gas spores */
                if (!flags.mon_moving) killed(mtmp);
                else monkilled(mtmp, "", (int)adtyp);
            } else if (!flags.mon_moving) setmangry(mtmp);
        }

    /* Do your injury last */
    if (uhurt) {
        if ((type >= 0 || adtyp == AD_PHYS) &&  /* gas spores */
                flags.verbose && olet != SCROLL_CLASS)
            You("are caught in the %s!", str);
        /* do property damage first, in case we end up leaving bones */
        if (adtyp == AD_FIRE) burn_away_slime();
        if (Invulnerable()) {
            damu = 0;
            You("are unharmed!");
        } else if (Half_physical_damage && adtyp == AD_PHYS)
            damu = (damu+1) / 2;
        if (adtyp == AD_FIRE) (void) burnarmor(&youmonst);
        destroy_item(SCROLL_CLASS, (int) adtyp);
        destroy_item(SPBOOK_CLASS, (int) adtyp);
        destroy_item(POTION_CLASS, (int) adtyp);
        destroy_item(RING_CLASS, (int) adtyp);
        destroy_item(WAND_CLASS, (int) adtyp);

        ugolemeffects((int) adtyp, damu);
        if (uhurt == 2) {
            if (Upolyd)
                u.mh  -= damu;
            else
                u.uhp -= damu;
        }

        if (u.uhp <= 0 || (Upolyd && u.mh <= 0)) {
            if (Upolyd) {
                rehumanize();
            } else {
                if (olet == MON_EXPLODE) {
                    /* killer handled by caller */
                    if (str != killer_buf && !generic)
                        strcpy(killer_buf, str);
                } else if (type >= 0 && olet != SCROLL_CLASS) {
                    sprintf(killer_buf, "caught %sself in %s own %s",
                            uhim(), uhis(), str);
                } else if (!strncmpi(str,"tower of flame", 8) ||
                        !strncmpi(str,"fireball", 8)) {
                    strcpy(killer_buf, str);
                } else {
                    strcpy(killer_buf, str);
                }
                fprintf(stderr, "TODO: killer = %s\n", killer_buf);
                /* Known BUG: BURNING suppresses corpse in bones data,
                   but done does not handle killer reason correctly */
                done((adtyp == AD_FIRE) ? BURNING : DIED);
            }
        }
        exercise(A_STR, false);
    }

    if (shopdamage) {
        pay_for_damage(adtyp == AD_FIRE ? "burn away" :
                adtyp == AD_COLD ? "shatter" :
                adtyp == AD_DISN ? "disintegrate" : "destroy",
                false);
    }

    /* explosions are noisy */
    i = dam * dam;
    if (i < 50) i = 50;     /* in case random damage is very small */
    wake_nearto(x, y, i);
}

/*
 * scflags:
 *      VIS_EFFECTS     Add visual effects to display
 *      MAY_HITMON      Objects may hit monsters
 *      MAY_HITYOU      Objects may hit hero
 *      MAY_HIT         Objects may hit you or monsters
 *      MAY_DESTROY     Objects may be destroyed at random
 *      MAY_FRACTURE    Stone objects can be fractured (statues, boulders)
 */

/* returns number of scattered objects */
// int sy,                             /* location of objects to scatter */
// int blastforce,                             /* force behind the scattering  */
// struct obj *obj                     /* only scatter this obj        */
long scatter ( int sx, int sy, int blastforce, unsigned int scflags, struct obj *obj) {
    struct obj *otmp;
    int tmp;
    int farthest = 0;
    unsigned char typ;
    long qtmp;
    bool used_up;
    bool individual_object = obj ? true : false;
    struct monst *mtmp;
    struct scatter_chain *stmp, *stmp2 = 0;
    struct scatter_chain *schain = (struct scatter_chain *)0;
    long total = 0L;

    while ((otmp = individual_object ? obj : level.objects[sx][sy]) != 0) {
        if (otmp->quan > 1L) {
            qtmp = otmp->quan - 1;
            if (qtmp > LARGEST_INT) qtmp = LARGEST_INT;
            qtmp = (long)rnd((int)qtmp);
            otmp = splitobj(otmp, qtmp);
        } else {
            obj = (struct obj *)0; /* all used */
        }
        obj_extract_self(otmp);
        used_up = false;

        /* 9 in 10 chance of fracturing boulders or statues */
        if ((scflags & MAY_FRACTURE)
                && ((otmp->otyp == BOULDER) || (otmp->otyp == STATUE))
                && rn2(10)) {
            if (otmp->otyp == BOULDER) {
                char break_clause[BUFSZ];
                Tobjnam(break_clause, BUFSZ, otmp, "break");
                pline("%s apart.", break_clause);
                fracture_rock(otmp);
                place_object(otmp, sx, sy);
                if ((otmp = sobj_at(BOULDER, sx, sy)) != 0) {
                    /* another boulder here, restack it to the top */
                    obj_extract_self(otmp);
                    place_object(otmp, sx, sy);
                }
            } else {
                struct trap *trap;

                if ((trap = t_at(sx,sy)) && trap->ttyp == STATUE_TRAP)
                    deltrap(trap);
                char crumble_clause[BUFSZ];
                Tobjnam(crumble_clause, BUFSZ, otmp, "crumble");
                pline("%s.", crumble_clause);
                break_statue(otmp);
                place_object(otmp, sx, sy); /* put fragments on floor */
            }
            used_up = true;

            /* 1 in 10 chance of destruction of obj; glass, egg destruction */
        } else if ((scflags & MAY_DESTROY) && (!rn2(10)
                    || (objects[otmp->otyp].oc_material == GLASS
                        || otmp->otyp == EGG))) {
            if (breaks(otmp, (signed char)sx, (signed char)sy)) used_up = true;
        }

        if (!used_up) {
            stmp = (struct scatter_chain *) malloc(sizeof(struct scatter_chain));
            stmp->next = (struct scatter_chain *)0;
            stmp->obj = otmp;
            stmp->ox = sx;
            stmp->oy = sy;
            tmp = rn2(8);           /* get the direction */
            stmp->dx = xdir[tmp];
            stmp->dy = ydir[tmp];
            tmp = blastforce - (otmp->owt/40);
            if (tmp < 1) tmp = 1;
            stmp->range = rnd(tmp); /* anywhere up to that determ. by wt */
            if (farthest < stmp->range) farthest = stmp->range;
            stmp->stopped = false;
            if (!schain)
                schain = stmp;
            else
                stmp2->next = stmp;
            stmp2 = stmp;
        }
    }

    while (farthest-- > 0) {
        for (stmp = schain; stmp; stmp = stmp->next) {
            if ((stmp->range-- > 0) && (!stmp->stopped)) {
                bhitpos.x = stmp->ox + stmp->dx;
                bhitpos.y = stmp->oy + stmp->dy;
                typ = levl[bhitpos.x][bhitpos.y].typ;
                if(!isok(bhitpos.x, bhitpos.y)) {
                    bhitpos.x -= stmp->dx;
                    bhitpos.y -= stmp->dy;
                    stmp->stopped = true;
                } else if(!ZAP_POS(typ) ||
                        closed_door(bhitpos.x, bhitpos.y)) {
                    bhitpos.x -= stmp->dx;
                    bhitpos.y -= stmp->dy;
                    stmp->stopped = true;
                } else if ((mtmp = m_at(bhitpos.x, bhitpos.y)) != 0) {
                    if (scflags & MAY_HITMON) {
                        stmp->range--;
                        if (ohitmon(mtmp, stmp->obj, 1, false)) {
                            stmp->obj = (struct obj *)0;
                            stmp->stopped = true;
                        }
                    }
                } else if (bhitpos.x==u.ux && bhitpos.y==u.uy) {
                    if (scflags & MAY_HITYOU) {
                        int hitvalu, hitu;

                        if (multi) nomul(0);
                        hitvalu = 8 + stmp->obj->spe;
                        if (bigmonst(youmonst.data)) hitvalu++;
                        hitu = thitu(hitvalu,
                                dmgval(stmp->obj, &youmonst),
                                stmp->obj, (char *)0);
                        if (hitu) {
                            stmp->range -= 3;
                            stop_occupation();
                        }
                    }
                }
                stmp->ox = bhitpos.x;
                stmp->oy = bhitpos.y;
            }
        }
    }
    for (stmp = schain; stmp; stmp = stmp2) {
        int x,y;

        stmp2 = stmp->next;
        x = stmp->ox; y = stmp->oy;
        if (stmp->obj) {
            if ( x!=sx || y!=sy )
                total += stmp->obj->quan;
            place_object(stmp->obj, x, y);
            stackobj(stmp->obj);
        }
        free((void *)stmp);
        newsym(x,y);
    }

    return total;
}


/*
 * Splatter burning oil from x,y to the surrounding area.
 *
 * This routine should really take a how and direction parameters.
 * The how is how it was caused, e.g. kicked verses thrown.  The
 * direction is which way to spread the flaming oil.  Different
 * "how"s would give different dispersal patterns.  For example,
 * kicking a burning flask will splatter differently from a thrown
 * flask hitting the ground.
 *
 * For now, just perform a "regular" explosion.
 */
void splatter_burning_oil (int x, int y) {
/* ZT_SPELL(ZT_FIRE) = ZT_SPELL(AD_FIRE-1) = 10+(2-1) = 11 */
#define ZT_SPELL_O_FIRE 11 /* value kludge, see zap.c */
    explode(x, y, ZT_SPELL_O_FIRE, d(4,4), BURNING_OIL, EXPL_FIERY);
}
