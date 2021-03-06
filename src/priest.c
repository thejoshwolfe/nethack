/* See LICENSE in the root of this project for change info */

#include "priest.h"

#include <stdio.h>
#include <string.h>

#include "rm_util.h"
#include "display_util.h"
#include "move.h"
#include "dungeon_util.h"
#include "attrib.h"
#include "coord.h"
#include "decl.h"
#include "display.h"
#include "do_name.h"
#include "emin.h"
#include "epri.h"
#include "eshk.h"
#include "flag.h"
#include "global.h"
#include "hack.h"
#include "hacklib.h"
#include "makemon.h"
#include "mfndpos.h"
#include "mhitu.h"
#include "minion.h"
#include "mkobj.h"
#include "mkroom.h"
#include "mon.h"
#include "monattk.h"
#include "mondata.h"
#include "monst.h"
#include "mthrowu.h"
#include "objclass.h"
#include "onames.h"
#include "permonst.h"
#include "pline.h"
#include "pm.h"
#include "polyself.h"
#include "potion.h"
#include "pray.h"
#include "prop.h"
#include "rm.h"
#include "rnd.h"
#include "shk.h"
#include "steal.h"
#include "steed.h"
#include "teleport.h"
#include "util.h"
#include "vision.h"
#include "worn.h"
#include "you.h"
#include "youprop.h"
#include "zap.h"

/* this matches the categorizations shown by enlightenment */
#define ALGN_SINNED     (-4)    /* worse than strayed */


static bool histemple_at(struct monst *,signed char,signed char);
static bool has_shrine(struct monst *);

/*
 * Move for priests and shopkeepers.  Called from shk_move() and pri_move().
 * Valid returns are  1: moved  0: didn't  -1: let m_move do it  -2: died.
 */
int move_special (struct monst *mtmp, bool in_his_shop, signed char appr,
        bool uondoor, bool avoid, signed char omx, signed char omy,
        signed char gx, signed char gy)
{
        signed char nx,ny,nix,niy;
        signed char i;
        signed char chcnt,cnt;
        coord poss[9];
        long info[9];
        long allowflags;
        struct obj *ib = (struct obj *)0;

        if(omx == gx && omy == gy)
                return(0);
        if(mtmp->mconf) {
                avoid = false;
                appr = 0;
        }

        nix = omx;
        niy = omy;
        if (mtmp->isshk) allowflags = ALLOW_SSM;
        else allowflags = ALLOW_SSM | ALLOW_SANCT;
        if (passes_walls(mtmp->data)) allowflags |= (ALLOW_ROCK|ALLOW_WALL);
        if (throws_rocks(mtmp->data)) allowflags |= ALLOW_ROCK;
        if (tunnels(mtmp->data)) allowflags |= ALLOW_DIG;
        if (!nohands(mtmp->data) && !verysmall(mtmp->data)) {
                allowflags |= OPENDOOR;
                if (m_carrying(mtmp, SKELETON_KEY)) allowflags |= BUSTDOOR;
        }
        if (is_giant(mtmp->data)) allowflags |= BUSTDOOR;
        cnt = mfndpos(mtmp, poss, info, allowflags);

        if(mtmp->isshk && avoid && uondoor) { /* perhaps we cannot avoid him */
                for(i=0; i<cnt; i++)
                    if(!(info[i] & NOTONL)) goto pick_move;
                avoid = false;
        }

#define GDIST(x,y)      (dist2(x,y,gx,gy))
pick_move:
        chcnt = 0;
        for(i=0; i<cnt; i++) {
                nx = poss[i].x;
                ny = poss[i].y;
                if(levl[nx][ny].typ == ROOM ||
                        (mtmp->ispriest &&
                            levl[nx][ny].typ == ALTAR) ||
                        (mtmp->isshk &&
                            (!in_his_shop || ESHK(mtmp)->following))) {
                    if(avoid && (info[i] & NOTONL))
                        continue;
                    if((!appr && !rn2(++chcnt)) ||
                        (appr && GDIST(nx,ny) < GDIST(nix,niy))) {
                            nix = nx;
                            niy = ny;
                    }
                }
        }
        if(mtmp->ispriest && avoid &&
                        nix == omx && niy == omy && onlineu(omx,omy)) {
                /* might as well move closer as long it's going to stay
                 * lined up */
                avoid = false;
                goto pick_move;
        }

        if(nix != omx || niy != omy) {
                remove_monster(omx, omy);
                place_monster(mtmp, nix, niy);
                newsym(nix,niy);
                if (mtmp->isshk && !in_his_shop && inhishop(mtmp))
                    check_special_room(false);
                if(ib) {
                        if (cansee(mtmp->mx,mtmp->my)) {
                            pline("%s picks up %s.", "TODO: Monnam(mtmp)", "TODO: distant_name(ib,doname)");
                        }
                        obj_extract_self(ib);
                        (void) mpickobj(mtmp, ib);
                }
                return(1);
        }
        return(0);
}

char temple_occupied (char *array) {
        char *ptr;

        for (ptr = array; *ptr; ptr++)
                if (rooms[*ptr - ROOMOFFSET].rtype == TEMPLE)
                        return(*ptr);
        return('\0');
}

static bool histemple_at(struct monst *priest, signed char x, signed char y) {
        return((bool)((EPRI(priest)->shroom == *in_rooms(x, y, TEMPLE)) &&
               on_level(&(EPRI(priest)->shrlevel), &u.uz)));
}

/*
 * pri_move: return 1: moved  0: didn't  -1: let m_move do it  -2: died
 */
int pri_move (struct monst *priest) {
    signed char gx,gy,omx,omy;
    signed char temple;
    bool avoid = true;

    omx = priest->mx;
    omy = priest->my;

    if(!histemple_at(priest, omx, omy)) return(-1);

    temple = EPRI(priest)->shroom;

    gx = EPRI(priest)->shrpos.x;
    gy = EPRI(priest)->shrpos.y;

    gx += rn1(3,-1);        /* mill around the altar */
    gy += rn1(3,-1);

    if(!priest->mpeaceful ||
            (Conflict && !resist(priest, RING_CLASS, 0, 0))) {
        if(monnear(priest, u.ux, u.uy)) {
            if (Displaced)
                message_monster(MSG_YOUR_DISPLACED_IMAGE_DOESNT_FOOL_M, priest);
            mattacku(priest);
            return(0);
        } else if(index(u.urooms, temple)) {
            /* chase player if inside temple & can see him */
            if(priest->mcansee && m_canseeu(priest)) {
                gx = u.ux;
                gy = u.uy;
            }
            avoid = false;
        }
    } else if(Invis) avoid = false;

    return(move_special(priest,false,true,false,avoid,omx,omy,gx,gy));
}

/* exclusively for mktemple() */
// bool sanctum   /* is it the seat of the high priest? */
void priestini ( d_level *lvl, struct mkroom *sroom, int sx, int sy, bool sanctum) {
        struct monst *priest;
        struct obj *otmp;
        int cnt;

        if(MON_AT(sx+1, sy))
                (void) rloc(m_at(sx+1, sy), false); /* insurance */

        priest = makemon(&mons[sanctum ? PM_HIGH_PRIEST : PM_ALIGNED_PRIEST],
                         sx + 1, sy, NO_MM_FLAGS);
        if (priest) {
                EPRI(priest)->shroom = (sroom - rooms) + ROOMOFFSET;
                EPRI(priest)->shralign = Amask2align(levl[sx][sy].flags);
                EPRI(priest)->shrpos.x = sx;
                EPRI(priest)->shrpos.y = sy;
                assign_level(&(EPRI(priest)->shrlevel), lvl);
                priest->mtrapseen = ~0; /* traps are known */
                priest->mpeaceful = 1;
                priest->ispriest = 1;
                priest->msleeping = 0;
                set_malign(priest); /* mpeaceful may have changed */

                /* now his/her goodies... */
                if(sanctum && EPRI(priest)->shralign == A_NONE &&
                     on_level(&sanctum_level, &u.uz)) {
                        (void) mongets(priest, AMULET_OF_YENDOR);
                }
                /* 2 to 4 spellbooks */
                for (cnt = rn1(3,2); cnt > 0; --cnt) {
                    (void) mpickobj(priest, mkobj(SPBOOK_CLASS, false));
                }
                /* robe [via makemon()] */
                if (rn2(2) && (otmp = which_armor(priest, W_ARMC)) != 0) {
                    if (p_coaligned(priest))
                        uncurse(otmp);
                    else
                        curse(otmp);
                }
        }
}

/*
 * Specially aligned monsters are named specially.
 *      - aligned priests with ispriest and high priests have shrines
 *              they retain ispriest and epri when polymorphed
 *      - aligned priests without ispriest and Angels are roamers
 *              they retain isminion and access epri as emin when polymorphed
 *              (coaligned Angels are also created as minions, but they
 *              use the same naming convention)
 *      - minions do not have ispriest but have isminion and emin
 */
size_t priestname(char *out_buf, size_t buf_size, const struct monst *mon,
        bool block_invis_and_halluc)
{
    const char *what = (Hallucination() && !block_invis_and_halluc) ?
        rndmonnam() : mon->data->mname;
    const char *invisible = (mon->minvis && !block_invis_and_halluc) ?
        "invisible " : "";

    if (mon->ispriest || mon->data == &mons[PM_ALIGNED_PRIEST] ||
            mon->data == &mons[PM_ANGEL])
    {
        // use epri
        const char *guardian = (mon->mtame && mon->data == &mons[PM_ANGEL]) ?
            "guardian " : "";

        const char *what_subject = "";
        const char *what_subject_space = "";
        if (mon->data != &mons[PM_ALIGNED_PRIEST] &&
                mon->data != &mons[PM_HIGH_PRIEST])
        {
            what_subject = what;
            what_subject_space = " ";
        }

        const char *renegade = "";
        const char *high = "";
        const char *generic_subject = "";
        if (mon->data != &mons[PM_ANGEL]) {
            if (!mon->ispriest && EPRI(mon)->renegade)
                renegade = "renegade ";

            if (mon->data == &mons[PM_HIGH_PRIEST])
                high = "high ";

            if (Hallucination() && !block_invis_and_halluc)
                generic_subject = "poohbah ";
            else if (mon->female)
                generic_subject = "priestess ";
            else
                generic_subject = "priest ";
        }


        aligntyp alignment = EPRI(mon)->shralign;
        const char *god_name = block_invis_and_halluc ?
            align_gname(alignment) : halu_gname(alignment);
        return nh_slprintf(out_buf, buf_size, "the %s%s%s%s%s%s%sof %s", invisible, guardian,
                what_subject, what_subject_space, renegade, high, generic_subject, god_name);
    } else {
        // use emin instead of epri
        aligntyp alignment = EMIN(mon)->min_align;
        const char *god_name = block_invis_and_halluc ?
            align_gname(alignment) : halu_gname(alignment);
        return nh_slprintf(out_buf, buf_size, "the %s%s of %s", invisible, what, god_name);
    }
}

bool p_coaligned (struct monst *priest) {
    return((bool)(u.ualign.type == ((int)EPRI(priest)->shralign)));
}

static bool has_shrine (struct monst *pri) {
        struct rm *lev;

        if(!pri)
                return(false);
        lev = &levl[EPRI(pri)->shrpos.x][EPRI(pri)->shrpos.y];
        if (!IS_ALTAR(lev->typ) || !(lev->flags & AM_SHRINE))
                return(false);
        return((bool)(EPRI(pri)->shralign == Amask2align(lev->flags & ~AM_SHRINE)));
}

struct monst *
findpriest (char roomno)
{
        struct monst *mtmp;

        for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp)) continue;
            if(mtmp->ispriest && (EPRI(mtmp)->shroom == roomno) &&
               histemple_at(mtmp,mtmp->mx,mtmp->my))
                return(mtmp);
        }
        return (struct monst *)0;
}

/* called from check_special_room() when the player enters the temple room */
void
intemple (int roomno)
{
        struct monst *priest = findpriest((char)roomno);
        bool tended = (priest != (struct monst *)0);
        bool shrined, sanctum, can_speak;
        const char *msg1, *msg2;
        char buf[BUFSZ];

        if(!temple_occupied(u.urooms0)) {
            if(tended) {
                shrined = has_shrine(priest);
                sanctum = (priest->data == &mons[PM_HIGH_PRIEST] &&
                           (Is_sanctum(&u.uz) || In_endgame(&u.uz)));
                can_speak = (priest->mcanmove && !priest->msleeping &&
                             flags.soundok);
                if (can_speak) {
                    unsigned save_priest = priest->ispriest;
                    /* don't reveal the altar's owner upon temple entry in
                       the endgame; for the Sanctum, the next message names
                       Moloch so suppress the "of Moloch" for him here too */
                    if (sanctum && !Hallucination()) priest->ispriest = 0;
                    if (canseemon(priest)) {
                        message_monster(MSG_M_INTONES, priest);
                    } else {
                        pline("A nearby voice intones:");
                    }
                    priest->ispriest = save_priest;
                }
                msg2 = 0;
                if(sanctum && Is_sanctum(&u.uz)) {
                    if(priest->mpeaceful) {
                        msg1 = "Infidel, you have entered Moloch's Sanctum!";
                        msg2 = "Be gone!";
                        priest->mpeaceful = 0;
                        set_malign(priest);
                    } else
                        msg1 = "You desecrate this place by your presence!";
                } else {
                    sprintf(buf, "Pilgrim, you enter a %s place!",
                            !shrined ? "desecrated" : "sacred");
                    msg1 = buf;
                }
                if (can_speak) {
                    verbalize("%s", msg1);
                    if (msg2) verbalize("%s", msg2);
                }
                if(!sanctum) {
                    /* !tended -> !shrined */
                    if (!shrined || !p_coaligned(priest) ||
                            u.ualign.record <= ALGN_SINNED)
                        You("have a%s forbidding feeling...",
                                (!shrined) ? "" : " strange");
                    else You("experience a strange sense of peace.");
                }
            } else {
                switch(rn2(3)) {
                  case 0: You("have an eerie feeling..."); break;
                  case 1: You_feel("like you are being watched."); break;
                  default: pline("A shiver runs down your %s.",
                        body_part(SPINE)); break;
                }
                if(!rn2(5)) {
                    struct monst *mtmp;

                    if(!(mtmp = makemon(&mons[PM_GHOST],u.ux,u.uy,NO_MM_FLAGS)))
                        return;
                    if (!Blind() || sensemon(mtmp))
                        pline("An enormous ghost appears next to you!");
                    else You("sense a presence close by!");
                    mtmp->mpeaceful = 0;
                    set_malign(mtmp);
                    if(flags.verbose)
                        You("are frightened to death, and unable to move.");
                    nomul(-3);
                    nomovemsg = "You regain your composure.";
               }
           }
       }
}

void priest_talk (struct monst *priest) {
    bool coaligned = p_coaligned(priest);
    bool strayed = (u.ualign.record < 0);

    /* KMH, conduct */
    u.uconduct.gnostic++;

    if(priest->mflee || (!priest->ispriest && coaligned && strayed)) {
        message_monster(MSG_M_WANTS_NOTHING_TO_DO_WITH_YOU, priest);
        priest->mpeaceful = 0;
        return;
    }

    /* priests don't chat unless peaceful and in their own temple */
    if(!histemple_at(priest,priest->mx,priest->my) ||
            !priest->mpeaceful || !priest->mcanmove || priest->msleeping) {
        static const char *cranky_msg[3] = {
            "Thou wouldst have words, eh?  I'll give thee a word or two!",
            "Talk?  Here is what I have to say!",
            "Pilgrim, I would speak no longer with thee."
        };

        if(!priest->mcanmove || priest->msleeping) {
            message_monster(MSG_M_BREAKS_OUT_OF_HIS_REVERIE, priest);
            priest->mfrozen = priest->msleeping = 0;
            priest->mcanmove = 1;
        }
        priest->mpeaceful = 0;
        verbalize("%s", cranky_msg[rn2(3)]);
        return;
    }

    /* you desecrated the temple and now you want to chat? */
    if(priest->mpeaceful && *in_rooms(priest->mx, priest->my, TEMPLE) &&
            !has_shrine(priest)) {
        verbalize("Begone!  Thou desecratest this holy place with thy presence.");
        priest->mpeaceful = 0;
        return;
    }
    if(!u.ugold) {
        if(coaligned && !strayed) {
            if (priest->mgold > 0L) {
                /* Note: two bits is actually 25 cents.  Hmm. */
                message_monster((priest->mgold == 1L) ?
                        MSG_M_GIVES_YOU_ONE_ALE : MSG_M_GIVES_YOU_TWO_ALE, priest);
                if (priest->mgold > 1L)
                    u.ugold = 2L;
                else
                    u.ugold = 1L;
                priest->mgold -= u.ugold;
            } else {
                message_monster(MSG_M_PREACHES_VIRTUES_POVERTY, priest);
            }
            exercise(A_WIS, true);
        } else {
            message_monster(MSG_M_IS_NOT_INTERESTED, priest);
        }
        return;
    } else {
        long offer;

        message_monster(MSG_M_ASKS_FOR_CONTRIBUTION_TEMPLE, priest);
        if((offer = bribe(priest)) == 0) {
            verbalize("Thou shalt regret thine action!");
            if(coaligned) adjalign(-1);
        } else if(offer < (u.ulevel * 200)) {
            if(u.ugold > (offer * 2L)) verbalize("Cheapskate.");
            else {
                verbalize("I thank thee for thy contribution.");
                /*  give player some token  */
                exercise(A_WIS, true);
            }
        } else if(offer < (u.ulevel * 400)) {
            verbalize("Thou art indeed a pious individual.");
            if(u.ugold < (offer * 2L)) {
                if (coaligned && u.ualign.record <= ALGN_SINNED)
                    adjalign(1);
                verbalize("I bestow upon thee a blessing.");
                incr_itimeout(&HClairvoyant, rn1(500,500));
            }
        } else if(offer < (u.ulevel * 600) &&
                u.ublessed < 20 &&
                (u.ublessed < 9 || !rn2(u.ublessed))) {
            verbalize("Thy devotion has been rewarded.");
            if (!(HProtection & INTRINSIC)) {
                HProtection |= FROMOUTSIDE;
                if (!u.ublessed)
                    u.ublessed = rn1(3, 2);
            } else {
                u.ublessed++;
            }
        } else {
            verbalize("Thy selfless generosity is deeply appreciated.");
            if(u.ugold < (offer * 2L) && coaligned) {
                if(strayed && (moves - u.ucleansed) > 5000L) {
                    u.ualign.record = 0; /* cleanse thee */
                    u.ucleansed = moves;
                } else {
                    adjalign(2);
                }
            }
        }
    }
}

struct monst *
mk_roamer (struct permonst *ptr, aligntyp alignment, signed char x, signed char y, bool peaceful)
{
        struct monst *roamer;
        bool coaligned = (u.ualign.type == alignment);

        if (ptr != &mons[PM_ALIGNED_PRIEST] && ptr != &mons[PM_ANGEL])
                return((struct monst *)0);

        if (MON_AT(x, y)) (void) rloc(m_at(x, y), false);       /* insurance */

        if (!(roamer = makemon(ptr, x, y, NO_MM_FLAGS)))
                return((struct monst *)0);

        EPRI(roamer)->shralign = alignment;
        if (coaligned && !peaceful)
                EPRI(roamer)->renegade = true;
        /* roamer->ispriest == false naturally */
        roamer->isminion = true;        /* borrowing this bit */
        roamer->mtrapseen = ~0;         /* traps are known */
        roamer->mpeaceful = peaceful;
        roamer->msleeping = 0;
        set_malign(roamer); /* peaceful may have changed */

        /* MORE TO COME */
        return(roamer);
}

void
reset_hostility (struct monst *roamer)
{
        if(!(roamer->isminion && (roamer->data == &mons[PM_ALIGNED_PRIEST] ||
                                  roamer->data == &mons[PM_ANGEL])))
                return;

        if(EPRI(roamer)->shralign != u.ualign.type) {
            roamer->mpeaceful = roamer->mtame = 0;
            set_malign(roamer);
        }
        newsym(roamer->mx, roamer->my);
}

bool 
in_your_sanctuary (
    struct monst *mon,      /* if non-null, <mx,my> overrides <x,y> */
    signed char x,
    signed char y
)
{
        char roomno;
        struct monst *priest;

        if (mon) {
            if (is_minion(mon->data) || is_rider(mon->data)) return false;
            x = mon->mx, y = mon->my;
        }
        if (u.ualign.record <= ALGN_SINNED)     /* sinned or worse */
            return false;
        if ((roomno = temple_occupied(u.urooms)) == 0 ||
                roomno != *in_rooms(x, y, TEMPLE))
            return false;
        if ((priest = findpriest(roomno)) == 0)
            return false;
        return (bool)(has_shrine(priest) &&
                         p_coaligned(priest) &&
                         priest->mpeaceful);
}

void
ghod_hitsu (      /* when attacking "priest" in his temple */
    struct monst *priest
)
{
        int x, y, ax, ay, roomno = (int)temple_occupied(u.urooms);
        struct mkroom *troom;

        if (!roomno || !has_shrine(priest))
                return;

        ax = x = EPRI(priest)->shrpos.x;
        ay = y = EPRI(priest)->shrpos.y;
        troom = &rooms[roomno - ROOMOFFSET];

        if((u.ux == x && u.uy == y) || !linedup(u.ux, u.uy, x, y)) {
            if(IS_DOOR(levl[u.ux][u.uy].typ)) {

                if(u.ux == troom->lx - 1) {
                    x = troom->hx;
                    y = u.uy;
                } else if(u.ux == troom->hx + 1) {
                    x = troom->lx;
                    y = u.uy;
                } else if(u.uy == troom->ly - 1) {
                    x = u.ux;
                    y = troom->hy;
                } else if(u.uy == troom->hy + 1) {
                    x = u.ux;
                    y = troom->ly;
                }
            } else {
                switch(rn2(4)) {
                case 0:  x = u.ux; y = troom->ly; break;
                case 1:  x = u.ux; y = troom->hy; break;
                case 2:  x = troom->lx; y = u.uy; break;
                default: x = troom->hx; y = u.uy; break;
                }
            }
            if(!linedup(u.ux, u.uy, x, y)) return;
        }

        switch(rn2(3)) {
        case 0:
            pline("%s roars in anger:  \"Thou shalt suffer!\"",
                        a_gname_at(ax, ay));
            break;
        case 1:
            pline("%s voice booms:  \"How darest thou harm my servant!\"",
                        "TODO: s_suffix(a_gname_at(ax, ay))");
            break;
        default:
            pline("%s roars:  \"Thou dost profane my shrine!\"",
                        a_gname_at(ax, ay));
            break;
        }

        buzz(-10-(AD_ELEC-1), 6, x, y, sgn(tbx), sgn(tby)); /* bolt of lightning */
        exercise(A_WIS, false);
}

void
angry_priest (void)
{
        struct monst *priest;
        struct rm *lev;

        if ((priest = findpriest(temple_occupied(u.urooms))) != 0) {
            wakeup(priest);
            /*
             * If the altar has been destroyed or converted, let the
             * priest run loose.
             * (When it's just a conversion and there happens to be
             *  a fresh corpse nearby, the priest ought to have an
             *  opportunity to try converting it back; maybe someday...)
             */
            lev = &levl[EPRI(priest)->shrpos.x][EPRI(priest)->shrpos.y];
            if (!IS_ALTAR(lev->typ) ||
                ((aligntyp)Amask2align(lev->flags & AM_MASK) !=
                        EPRI(priest)->shralign)) {
                priest->ispriest = 0;           /* now a roamer */
                priest->isminion = 1;           /* but still aligned */
                /* this overloads the `shroom' field, which is now clobbered */
                EPRI(priest)->renegade = 0;
            }
        }
}

/*
 * When saving bones, find priests that aren't on their shrine level,
 * and remove them.   This avoids big problems when restoring bones.
 */
void
clearpriests (void)
{
    struct monst *mtmp, *mtmp2;

    for(mtmp = fmon; mtmp; mtmp = mtmp2) {
        mtmp2 = mtmp->nmon;
        if (!DEADMONSTER(mtmp) && mtmp->ispriest && !on_level(&(EPRI(mtmp)->shrlevel), &u.uz))
            mongone(mtmp);
    }
}

/* munge priest-specific structure when restoring -dlc */
void 
restpriest (struct monst *mtmp, bool ghostly)
{
    if(u.uz.dlevel) {
        if (ghostly)
            assign_level(&(EPRI(mtmp)->shrlevel), &u.uz);
    }
}


/*priest.c*/
