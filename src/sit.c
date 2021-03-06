/* See LICENSE in the root of this project for change info */

#include "sit.h"

#include <stdbool.h>

#include "rm_util.h"
#include "dungeon_util.h"
#include "artifact.h"
#include "artifact_names.h"
#include "attrib.h"
#include "dbridge.h"
#include "decl.h"
#include "detect.h"
#include "display.h"
#include "do.h"
#include "do_name.h"
#include "dungeon.h"
#include "eat.h"
#include "engrave.h"
#include "flag.h"
#include "hack.h"
#include "invent.h"
#include "makemon.h"
#include "mkobj.h"
#include "mkroom.h"
#include "mon.h"
#include "mondata.h"
#include "monst.h"
#include "obj.h"
#include "objclass.h"
#include "objnam.h"
#include "onames.h"
#include "pline.h"
#include "polyself.h"
#include "potion.h"
#include "pray.h"
#include "prop.h"
#include "read.h"
#include "rm.h"
#include "rnd.h"
#include "teleport.h"
#include "timeout.h"
#include "trap.h"
#include "util.h"
#include "wizard.h"
#include "worn.h"
#include "you.h"
#include "youprop.h"
#include "zap.h"

void take_gold (void) {
    if (u.ugold <= 0)  {
        You_feel("a strange sensation.");
    } else {
        You("notice you have no gold!");
        u.ugold = 0;
    }
}

int dosit (void) {
    static const char sit_message[] = "sit on the %s.";
    struct trap *trap;
    int typ = levl[u.ux][u.uy].typ;


    if (u.usteed) {
        message_monster(MSG_YOU_ARE_ALEADY_SITTING_ON_M, u.usteed);
        return 0;
    }

    if(!can_reach_floor())  {
        if (Levitation)
            You("tumble in place.");
        else
            You("are sitting on air.");
        return 0;
    } else if (is_pool(u.ux, u.uy) && !Underwater) {  /* water walking */
        goto in_water;
    }

    if(OBJ_AT(u.ux, u.uy)) {
        struct obj *obj;

        obj = level.objects[u.ux][u.uy];
        You("sit on %s.", the(xname(obj)));
        if (!(Is_box(obj) || objects[obj->otyp].oc_material == CLOTH))
            pline("It's not very comfortable...");

    } else if ((trap = t_at(u.ux, u.uy)) != 0 ||
            (u.utrap && (u.utraptype >= TT_LAVA))) {

        if (u.utrap) {
            exercise(A_WIS, false); /* you're getting stuck longer */
            if(u.utraptype == TT_BEARTRAP) {
                You_cant("sit down with your %s in the bear trap.", body_part(FOOT));
                u.utrap++;
            } else if(u.utraptype == TT_PIT) {
                if(trap->ttyp == SPIKED_PIT) {
                    You("sit down on a spike.  Ouch!");
                    losehp(1, killed_by_const(KM_SITTING_ON_IRON_SPIKE));
                    exercise(A_STR, false);
                } else
                    You("sit down in the pit.");
                u.utrap += rn2(5);
            } else if(u.utraptype == TT_WEB) {
                You("sit in the spider web and get entangled further!");
                u.utrap += rn1(10, 5);
            } else if(u.utraptype == TT_LAVA) {
                /* Must have fire resistance or they'd be dead already */
                You("sit in the lava!");
                u.utrap += rnd(4);
                losehp(d(2,10), killed_by_const(KM_SITTING_IN_LAVA));
            } else if(u.utraptype == TT_INFLOOR) {
                You_cant("maneuver to sit!");
                u.utrap++;
            }
        } else {
            You("sit down.");
            dotrap(trap, 0);
        }
    } else if(Underwater || Is_waterlevel(&u.uz)) {
        if (Is_waterlevel(&u.uz))
            There("are no cushions floating nearby.");
        else
            You("sit down on the muddy bottom.");
    } else if(is_pool(u.ux, u.uy)) {
in_water:
        You("sit in the water.");
        if (!rn2(10) && uarm)
            (void) rust_dmg(uarm, "armor", 1, true, &youmonst);
        if (!rn2(10) && uarmf && uarmf->otyp != WATER_WALKING_BOOTS)
            (void) rust_dmg(uarm, "armor", 1, true, &youmonst);
    } else if(IS_SINK(typ)) {

        You(sit_message, defsyms[S_sink].explanation);
        Your("%s gets wet.", humanoid(youmonst.data) ? "rump" : "underside");
    } else if(IS_ALTAR(typ)) {

        You(sit_message, defsyms[S_altar].explanation);
        altar_wrath(u.ux, u.uy);

    } else if(IS_GRAVE(typ)) {

        You(sit_message, defsyms[S_grave].explanation);

    } else if(typ == STAIRS) {

        You(sit_message, "stairs");

    } else if(typ == LADDER) {

        You(sit_message, "ladder");

    } else if (is_lava(u.ux, u.uy)) {

        /* must be WWalking */
        You(sit_message, "lava");
        burn_away_slime();
        if (likes_lava(youmonst.data)) {
            pline_The("lava feels warm.");
            return 1;
        }
        pline_The("lava burns you!");
        losehp(d((Fire_resistance() ? 2 : 10), 10), killed_by_const(KM_SITTING_ON_LAVA));

    } else if (is_ice(u.ux, u.uy)) {

        You(sit_message, defsyms[S_ice].explanation);
        if (!Cold_resistance()) pline_The("ice feels cold.");

    } else if (typ == DRAWBRIDGE_DOWN) {

        You(sit_message, "drawbridge");

    } else if(IS_THRONE(typ)) {

        You(sit_message, defsyms[S_throne].explanation);
        if (rnd(6) > 4)  {
            switch (rnd(13))  {
                case 1:
                    (void) adjattrib(rn2(A_MAX), -rn1(4,3), false);
                    losehp(rnd(10), killed_by_const(KM_CURSED_THRONE));
                    break;
                case 2:
                    (void) adjattrib(rn2(A_MAX), 1, false);
                    break;
                case 3:
                    pline("A%s electric shock shoots through your body!",
                            (Shock_resistance()) ? "n" : " massive");
                    losehp(Shock_resistance() ? rnd(6) : rnd(30), killed_by_const(KM_ELECTRIC_CHAIR));
                    exercise(A_CON, false);
                    break;
                case 4:
                    You_feel("much, much better!");
                    if (Upolyd) {
                        if (u.mh >= (u.mhmax - 5))  u.mhmax += 4;
                        u.mh = u.mhmax;
                    }
                    if(u.uhp >= (u.uhpmax - 5))  u.uhpmax += 4;
                    u.uhp = u.uhpmax;
                    make_blinded(0L,true);
                    make_sick(0L, (char *) 0, false, SICK_ALL);
                    heal_legs();
                    break;
                case 5:
                    take_gold();
                    break;
                case 6:
                    if(u.uluck + rn2(5) < 0) {
                        You_feel("your luck is changing.");
                        change_luck(1);
                    } else      makewish();
                    break;
                case 7:
                    {
                        int cnt = rnd(10);

                        pline("A voice echoes:");
                        verbalize("Thy audience hath been summoned, %s!",
                                flags.female ? "Dame" : "Sire");
                        while(cnt--)
                            (void) makemon(courtmon(), u.ux, u.uy, NO_MM_FLAGS);
                        break;
                    }
                case 8:
                    pline("A voice echoes:");
                    verbalize("By thy Imperious order, %s...",
                            flags.female ? "Dame" : "Sire");
                    do_genocide(5); /* REALLY|ONTHRONE, see do_genocide() */
                    break;
                case 9:
                    pline("A voice echoes:");
                    verbalize("A curse upon thee for sitting upon this most holy throne!");
                    if (Luck > 0)  {
                        make_blinded(Blinded + rn1(100,250),true);
                    } else      rndcurse();
                    break;
                case 10:
                    if (Luck < 0 || (get_HSee_invisible() & INTRINSIC))  {
                        if (level.flags.nommap) {
                            pline(
                                    "A terrible drone fills your head!");
                            make_confused(get_HConfusion() + rnd(30),
                                    false);
                        } else {
                            pline("An image forms in your mind.");
                            do_mapping();
                        }
                    } else  {
                        Your("vision becomes clear.");
                        set_HSee_invisible(get_HSee_invisible() | FROMOUTSIDE);
                        newsym(u.ux, u.uy);
                    }
                    break;
                case 11:
                    if (Luck < 0)  {
                        You_feel("threatened.");
                        aggravate();
                    } else  {

                        You_feel("a wrenching sensation.");
                        tele();             /* teleport him */
                    }
                    break;
                case 12:
                    You("are granted an insight!");
                    if (invent) {
                        /* rn2(5) agrees w/seffects() */
                        identify_pack(rn2(5));
                    }
                    break;
                case 13:
                    Your("mind turns into a pretzel!");
                    make_confused(get_HConfusion() + rn1(7,16),false);
                    break;
                default:    impossible("throne effect");
                            break;
            }
        } else {
            if (is_prince(youmonst.data))
                You_feel("very comfortable here.");
            else
                You_feel("somehow out of place...");
        }

        if (!rn2(3) && IS_THRONE(levl[u.ux][u.uy].typ)) {
            /* may have teleported */
            levl[u.ux][u.uy].typ = ROOM;
            pline_The("throne vanishes in a puff of logic.");
            newsym(u.ux,u.uy);
        }

    } else if (lays_eggs(youmonst.data)) {
        struct obj *uegg;

        if (!flags.female) {
            pline("Males can't lay eggs!");
            return 0;
        }

        if (u.uhunger < (int)objects[EGG].oc_nutrition) {
            You("don't have enough energy to lay an egg.");
            return 0;
        }

        uegg = mksobj(EGG, false, false);
        uegg->spe = 1;
        uegg->quan = 1;
        uegg->owt = weight(uegg);
        uegg->corpsenm = egg_type_from_parent(u.umonnum, false);
        uegg->known = uegg->dknown = 1;
        attach_egg_hatch_timeout(uegg);
        You("lay an egg.");
        dropy(uegg);
        stackobj(uegg);
        morehungry((int)objects[EGG].oc_nutrition);
    } else if (u.uswallow)
        There("are no seats in here!");
    else
        pline("Having fun sitting on the %s?", surface(u.ux,u.uy));
    return(1);
}

/* curse a few inventory items at random! */
void rndcurse (void) {
    int     nobj = 0;
    int     cnt, onum;
    struct  obj     *otmp;
    static const char mal_aura[] = "feel a malignant aura surround %s.";

    if (uwep && (uwep->oartifact == ART_MAGICBANE) && rn2(20)) {
        You(mal_aura, "the magic-absorbing blade");
        return;
    }

    if(Antimagic()) {
        shieldeff(u.ux, u.uy);
        You(mal_aura, "you");
    }

    for (otmp = invent; otmp; otmp = otmp->nobj) {
        nobj++;
    }
    if (nobj) {
        for (cnt = rnd(6/((!!Antimagic()) + (!!Half_spell_damage) + 1));
                cnt > 0; cnt--)  {
            onum = rnd(nobj);
            for (otmp = invent; otmp; otmp = otmp->nobj) {
                if (--onum == 0) break;     /* found the target */
            }
            /* the !otmp case should never happen; picking an already
               cursed item happens--avoid "resists" message in that case */
            if (!otmp || otmp->cursed) continue;    /* next target */

            if(otmp->oartifact && spec_ability(otmp, SPFX_INTEL) && rn2(10) < 8) {
                message_object(MSG_O_RESISTS, otmp);
                continue;
            }

            if(otmp->blessed)
                unbless(otmp);
            else
                curse(otmp);
        }
    }

    /* treat steed's saddle as extended part of hero's inventory */
    if (u.usteed && !rn2(4) &&
            (otmp = which_armor(u.usteed, W_SADDLE)) != 0 &&
            !otmp->cursed) /* skip if already cursed */
    {
        if (otmp->blessed)
            unbless(otmp);
        else
            curse(otmp);
        if (!Blind()) {
            if (Hallucination()) {
                message_monster_object_int(MSG_M_O_GLOWS_COLOR, u.usteed, otmp, halluc_color_int());
            } else if (otmp->cursed) {
                message_monster_object(MSG_M_O_GLOWS_BLACK, u.usteed, otmp);
            } else {
                message_monster_object(MSG_M_O_GLOWS_BROWN, u.usteed, otmp);
            }
            otmp->bknown = true;
        }
    }
}

/* remove a random INTRINSIC ability */
void attrcurse (void) {
    switch(rnd(11)) {
        case 1 : if (get_HFire_resistance() & INTRINSIC) {
                     set_HFire_resistance(get_HFire_resistance() & ~INTRINSIC);
                     You_feel("warmer.");
                     break;
                 }
        case 2 : if (HTeleportation & INTRINSIC) {
                     HTeleportation &= ~INTRINSIC;
                     You_feel("less jumpy.");
                     break;
                 }
        case 3 : if (get_HPoison_resistance() & INTRINSIC) {
                     set_HPoison_resistance(get_HPoison_resistance() & ~INTRINSIC);
                     You_feel("a little sick!");
                     break;
                 }
        case 4 : if (HTelepat & INTRINSIC) {
                     HTelepat &= ~INTRINSIC;
                     if (Blind() && !Blind_telepat)
                         see_monsters();     /* Can't sense mons anymore! */
                     Your("senses fail!");
                     break;
                 }
        case 5 : if (get_HCold_resistance() & INTRINSIC) {
                     set_HCold_resistance(get_HCold_resistance() & ~INTRINSIC);
                     You_feel("cooler.");
                     break;
                 }
        case 6 : if (HInvis & INTRINSIC) {
                     HInvis &= ~INTRINSIC;
                     You_feel("paranoid.");
                     break;
                 }
        case 7 : if (get_HSee_invisible() & INTRINSIC) {
                     set_HSee_invisible(get_HSee_invisible() & ~INTRINSIC);
                     You("%s!", Hallucination() ? "tawt you taw a puttie tat"
                             : "thought you saw something");
                     break;
                 }
        case 8 : if (HFast & INTRINSIC) {
                     HFast &= ~INTRINSIC;
                     You_feel("slower.");
                     break;
                 }
        case 9 : if (HStealth & INTRINSIC) {
                     HStealth &= ~INTRINSIC;
                     You_feel("clumsy.");
                     break;
                 }
        case 10: if (HProtection & INTRINSIC) {
                     HProtection &= ~INTRINSIC;
                     You_feel("vulnerable.");
                     break;
                 }
        case 11: if (HAggravate_monster & INTRINSIC) {
                     HAggravate_monster &= ~INTRINSIC;
                     You_feel("less attractive.");
                     break;
                 }
        default: break;
    }
}
