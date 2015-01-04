#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>


/*

MSG_NO_ELBOW_ROOM:  "You don't have enough elbow-room to maneuver.";

                Your("displaced image doesn't fool %s!", mon_nam(shkp));
                message_monster(MSG_YOUR_DISPLACED_IMAGE_DOESNT_FOOL_M, shkp);

            unsigned char still_flag = credit_use ? MSG_FLAG_STILL : 0;
            if(obj->oclass == COIN_CLASS) {
                You("%sowe %s %ld %s!", still, mon_nam(shkp), value, currency(value));
                message_monster_int_flag(MSG_YOU_OWE_M_X_GOLD, shkp, value, still_flag);
            } else {
                You("%sowe %s %ld %s for %s!", still, mon_nam(shkp), value, currency(value),
                        obj->quan > 1L ? "them" : "it");
                unsigned char them_flag = MSG_FLAG_THEM;
                message_monster_int_flag(MSG_YOU_OWE_M_X_GOLD_FOR_THEM, shkp,
                        value, still_flag|them_flag);
            }

        pline("%s asks whether you've seen any untended shops recently.", Monnam(shkp));
        message_monster(MSG_M_ASKS_WHETHER_YOUVE_SEEN_UNTENDED_SHOPS, shkp);

                    pline("%s doesn't like customers who don't pay.", Monnam(shkp));
                    message_monster(MSG_M_DOESNT_LIKE_CUSTOMERS_WHO_DONT_PAY, shkp);

            pline("%s nimbly%s catches %s.",
                    Monnam(shkp),
                     ? "" : " reaches over and",
                    the(xname(obj)));
            unsigned char reaches_over_flag = (x == shkp->mx && y == shkp->my) ?
                0 : MSG_FLAG_REACHES_OVER;
            message_monster_object_flag(MSG_M_NIMBLY_CATCHES_O, shkp, reaches_over_flag, obj);

            pline("%s cannot pay you at present.", Monnam(shkp));
            message_monster(MSG_M_CANNOT_PAY_YOU_AT_PRESENT, shkp);

        pline("%s seems uninterested%s.", Monnam(shkp), cgold ? " in the rest" : "");
        unsigned char in_the_rest_flag = cgold ? MSG_FLAG_IN_THE_REST : 0;
        message_monster_flag(MSG_M_SEEMS_UNINTERESTED, shkp, in_the_rest_flag);

            pline("%s seems uninterested.", Monnam(shkp));
            monster_message(MSG_M_SEEMS_UNINTERESTED, shkp);

            pline("%s does not notice.", Monnam(shkp));
            monster_message(MSG_M_DOES_NOT_NOTICE, shkp);

        verbalize("Thank you for shopping in %s %s!",
                s_suffix(shkname(shkp)),
                shtypes[eshkp->shoptype - SHOPBASE].name);
        audio_message_monster_string(MSG_THANK_YOU_FOR_SHOPPING_IN_M_S, shkp,
                shtypes[eshkp->shoptype - SHOPBASE].name);

        pline("%s %s.", Monnam(shkp), rn2(2) ? "seems to be napping" : "doesn't respond");
        unsigned char nap_flag = rn2(2) ? MSG_FLAG_NAPPING : 0;
        message_monster_flag(MSG_M_SEEMS_TO_BE_NAPPING, shkp, nap_flag);

            pline("%s is too far to receive your payment.", Monnam(mtmp));
            message_monster(MSG_M_TOO_FAR_TO_RECEIVE_PAYMENT, mtmp);

            pline("%s is not interested in your payment.", Monnam(mtmp));
            message_monster(MSG_M_NOT_INTERESTED_IN_YOUR_PAYMENT, mtmp);

            pline("%s is not near enough to receive your payment.", Monnam(shkp));
            message_monster(MSG_M_NOT_NEAR_ENOUGH_FOR_PAYMENT, shkp);

    pline("%s %s!", Monnam(shkp), !ANGRY(shkp) ? "gets angry" : "is furious");
    unsigned char angry_flag = ANGRY(shkp) ? MSG_FLAG_SHOPKEEPER_ANGRY : 0;
    message_monster_flag(MSG_M_GETS_ANGRY, angry_flag);

            pline("%s has no interest in %s.", Monnam(shkp), the(xname(obj)));
            message_monster_object(MSG_M_HAS_NO_INTEREST_IN_O, shkp, obj);

            pline("%s %slooks at your corpse%s and %s.",
                    Monnam(shkp),
                    (!shkp->mcanmove || shkp->msleeping) ? "wakes up, " : "",
                    !rn2(2) ? (shkp->female ? ", shakes her head," :
                        ", shakes his head,") : "",
                    !inhishop(shkp) ? "disappears" : "sighs");
            unsigned char wakes_up_flag = (!shkp->mcanmove || shkp->msleeping) ?
                MSG_FLAG_WAKES_UP : 0;
            unsigned char shakes_head_flag = (!rn2(2)) ? MSG_FLAG_SHAKES_HEAD : 0;
            unsigned char in_shop_flag = inhishop(shkp) ? MSG_FLAG_IN_SHOP : 0;
            message_monster_flag(MSG_M_LOOKS_AT_YOUR_CORPSE_AND_DISAPPEARS, shkp,
                    wakes_up_flag|shakes_head_flag|in_shop_flag);

            verbalize("%s for the other %s before buying %s.",
                    ANGRY(shkp) ? "Pay" : "Please pay", xname(obj),
                    save_quan > 1L ? "these" : "this one");
            unsigned char angry_flag = ANGRY(shkp) ? MSG_FLAG_SHOPKEEPER_ANGRY : 0;
            unsigned char quantity_flag = (save_quan > 1L) ? MSG_FLAG_QUANTITY_MULTI : 0;
            audio_message_object_flag(MSG_PAY_FOR_THE_OTHER_O_BEFORE_BUYING_THIS,
                    obj, angry_flag|quantity_flag);

                pline("But %s is as angry as ever.", mon_nam(shkp));
                message_monster(MSG_BUT_M_IS_AS_ANGRY_AS_EVER, shkp);

            You("try to appease %s by giving %s 1000 gold pieces.",
                    x_monnam(shkp, ARTICLE_THE, "angry", 0, false),
                    mhim(shkp));
            message_monster(MSG_YOU_TRY_APPEASE_M_BY_GIVING_1000_GOLD_PIECES, shkp);

            pline("%s is after your hide, not your money!", Monnam(shkp));
            message_monster(MSG_M_AFTER_YOUR_HIDE_NOT_YOUR_MONEY, shkp);

            pline("But since %s shop has been robbed recently,", mhis(shkp));
            if (u.ugold < ltmp) {
                message_const(MSG_YOU_PARTIALLY_COMPENSATE_M_FOR_HIS_LOSSES, shkp);
            } else {
                message_const(MSG_YOU_COMPENSATE_M_FOR_HIS_LOSSES, shkp);
            }
            pline("you %scompensate %s for %s losses.",
                    (u.ugold < ltmp) ?  "partially " : "", mon_nam(shkp), mhis(shkp));

            pline("%s is after blood, not money!", Monnam(shkp));
            message_monster(MSG_M_IS_AFTER_BLOOD_NOT_MONEY, shkp);

        pline("%s calms down.", Monnam(shkp));
        message_monster(MSG_M_CALMS_DOWN, shkp);

                pline("Unfortunately, %s doesn't look satisfied.", mhe(shkp));
                message_monster(MSG_UNFORTUNATELY_M_DOESNT_LOOK_SATISFIED, shkp);

                You("give %s all your%s gold.", mon_nam(shkp), stashed_gold ? " openly kept" : "");
                message_monster(MSG_YOU_GIVE_M_ALL_YOUR_GOLD, shkp);
                if (stashed_gold) pline("But you have hidden gold!");

                You("give %s the %ld gold piece%s %s asked for.",
                        mon_nam(shkp), ltmp, plur(ltmp), mhe(shkp));
                message_monster_int(MSG_YOU_GIVE_M_THE_X_GOLD_PIECES, shkp, ltmp);

            You("do not owe %s anything.", mon_nam(shkp));
            message_monster(MSG_YOU_DO_NOT_OWE_M_ANYTHING, shkp);

                You("have %ld %s credit at %s %s.",
                        amt, currency(amt), s_suffix(shkname(shkp)),
                        shtypes[eshkp->shoptype - SHOPBASE].name);
                message_monster_int_string(MSG_YOU_HAVE_X_CREDIT_AT_M_S, shkp,
                        amt, shtypes[eshkp->shoptype - SHOPBASE].name);

            verbalize(NOTANGRY(shkp) ?
                    "Will you please leave %s outside?" :
                    "Leave %s outside.", y_monnam(u.usteed));
            audio_message_monster(NOTANGRY(shkp) ?
                    MSG_PLEASE_LEAVE_M_OUTSIDE : MSG_LEAVE_M_OUTSIDE, u.usteed);

            pline("%s %s.", Monnam(shkp), shkp->msleeping ? "wakes up" : "can move again");
            message_monster(shkp->msleeping ? MSG_M_WAKES_UP : MSG_M_CAN_MOVE_AGAIN, shkp);

        pline("%s is in no mood for consultations.", Monnam(oracl));
        message_monster(MSG_M_NO_MOOD_FOR_CONSULTATIONS, oracl);

                            pline("%s out!", Tobjnam(obj, "go"));
                            message_object(MSG_O_GOES_OUT, obj);

                    pline("%s glistens.", Monnam(u.ustuck));
                    message_monster(MSG_M_GLISTENS, u.ustuck);

                    pline("%s shines briefly.", Monnam(u.ustuck));
                    message_monster(MSG_M_SHINES_BRIEFLY, u.ustuck);

                pline("%s %s is lit.", s_suffix(Monnam(u.ustuck)), mbodypart(u.ustuck, STOMACH));
                message_monster(MSG_M_STOMACH_IS_LIT, u.ustuck);

                pline("Suddenly, the only light left comes from %s!", the(xname(uwep)));
                message_object(MSG_ONLY_LIGHT_LEFT_COMES_FROM_O, uwep);

    Your("%s vibrates violently, and explodes!",xname(obj));
    message_object(MSG_YOUR_O_VIBRATES_VIOLENTLY_AND_EXPLODES, obj);

                                Your("%s does not protect you.", xname(uarmh));
                                message_const(M_YOUR_O_DOES_NOT_PROTECT_YOU, uarmh);

                                            pline("%s's %s does not protect %s.",
                                                    Monnam(mtmp), xname(helmet), mhim(mtmp));
                                            message_monster_object(MSG_M_O_DOES_NOT_PROTECT_HIM,
                                                    mtmp, helmet);

                            pline("Fortunately, %s is wearing a hard helmet.", mon_nam(mtmp));
                            message_monster(MSG_M_WEARING_HARD_HELMET, mtmp);

                                    pline("%s is hit by %s!", Monnam(mtmp), doname(otmp2));
                                    message_monster_object(MSG_M_IS_HIT_BY_O, mtmp, otmp2);

                    Your("%s %s.", xname(otmp), otense(otmp, "vibrate"));
                    message_object(MSG_YOUR_O_VIBRATES, otmp);

                    Your("%s suddenly %s %s.",
                            xname(otmp), otense(otmp, "vibrate"),
                            Blind ? "again" : "unexpectedly");
                    message_object(MSG_YOUR_O_SUDDENLY_VIBRATES_UNEXPECTEDLY, otmp);

                    Your("%s merges and hardens!", xname(otmp));
                    message_object(MSG_O_MERGES_AND_HARDENS, otmp);

                            Your("%s %s as good as new!",
                                 xname(otmp),
                                 otense(otmp, Blind ? "feel" : "look"));
                            message_object(MSG_O_LOOK_AS_GOOD_AS_NEW, otmp);

                            Your("%s %s covered by a %s %s %s!",
                                xname(otmp), otense(otmp, "are"),
                                sobj->cursed ? "mottled" : "shimmering",
                                 hcolor(sobj->cursed ? NH_BLACK : NH_GOLDEN),
                                sobj->cursed ? "glow" :
                                  (is_shield(otmp) ? "layer" : "shield"));
                            message_object(sobj->cursed ?  MSG_O_ARE_COVERED_BY_BLACK_GLOW :
                                    MSG_O_ARE_COVERED_BY_GOLDEN_GLOW, otmp);

                            Your("%s %s warm for a moment.", xname(otmp), otense(otmp, "feel"));
                            message_object(MSG_YOUR_O_FEEL_WARM_MOMENT, otmp);

                Your("%s spins %sclockwise for a moment.", xname(obj), s < 0 ? "counter" : "");
                message_object((s < 0) ? MSG_O_SPINS_COUNTER_CLOCKWISE : MSG_O_SPINS_CLOCKWISE, obj);

                Your("%s %s momentarily, then %s!",
                     xname(obj), otense(obj,"pulsate"), otense(obj,"explode"));
                message_object(MSG_O_PULSATES_THEN_EXPLODES, obj);

        Your("%s %s for a moment.",
                xname(otmp),
                otense(otmp, "vibrate"))
        message_object(MSG_YOUR_O_VIBRATES_FOR_MOMENT, otmp);

    Your("%s %s briefly.", xname(otmp), otense(otmp, Blind ? "vibrate" : "glow"));
    message_object(Blind ? MSG_YOUR_O_VIBRATES_BRIEFLY : MSG_YOUR_O_GLOWS_BRIEFLY, otmp);

            Your("%s %s briefly.",xname(obj), otense(obj, "vibrate"));
            message_object(MSG_YOUR_O_VIBRATES_BRIEFLY, obj);

            pline("%s speaks:", Monnam(mtmp));
            message_monster(MSG_M_SPEAKS, mtmp);

                Your("displaced image doesn't fool %s!", mon_nam(priest));
                message_monster(MSG_YOUR_DISPLACED_IMAGE_DOESNT_FOOL_M, priest);

            pline("%s asks you for a contribution for the temple.", Monnam(priest));
            message_monster(MSG_M_ASKS_FOR_CONTRIBUTION_TEMPLE, priest);

                pline("%s is not interested.", Monnam(priest));
                message_monster(MSG_M_IS_NOT_INTERESTED, priest);

                    pline("%s preaches the virtues of poverty.", Monnam(priest));
                    message_monster(MSG_M_PREACHES_VIRTUES_POVERTY, priest);

                    pline("%s gives you %s for an ale.", Monnam(priest),
                        (priest->mgold == 1L) ? "one bit" : "two bits");
                    message_monster((priest->mgold == 1L) ?
                            MSG_M_GIVES_YOU_ONE_ALE : MSG_M_GIVES_YOU_TWO_ALE, priest);

                pline("%s breaks out of %s reverie!", Monnam(priest), mhis(priest));
                message_monster(MSG_M_BREAKS_OUT_OF_HIS_REVERIE, priest);

            pline("%s doesn't want anything to do with you!", Monnam(priest));
            message_monster(MSG_M_WANTS_NOTHING_TO_DO_WITH_YOU, priest);

                        pline("%s intones:", Monnam(priest));
                        message_monster(MSG_M_INTONES, priest);

                    You("have summoned %s!", a_monnam(dmon));
                    message_monster(MSG_YOU_HAVE_SUMMONED_M, dmon);

            pline("%s seems unaffected.", Monnam(u.ustuck));
            message_monster(MSG_M_SEEMS_UNAFFECTED, u.ustuck);

            pline("%s fries to a crisp!", Monnam(u.ustuck));
            message_monster(MSG_M_FRIES_TO_CRISP, u.ustuck);

        pline("A wide-angle disintegration beam aimed at you hits %s!", mon_nam(u.ustuck));
        message_monster(MSG_WIDE_ANGLE_DISINTEGRATION_BEAM_HITS_M, u.ustuck);

            pline("%s seems unaffected.", Monnam(u.ustuck));
            message_monster(MSG_M_SEEMS_UNAFFECTED, u.ustuck);

            pline("%s fries to a crisp!", Monnam(u.ustuck));
            message_monster(MSG_M_FRIES_TO_CRISP, u.ustuck);

        pline("It strikes %s!", mon_nam(u.ustuck));
        message_monster(MSG_IT_STRIKES_M, u.ustuck);

                pline("%s %s %s.",
                        s_suffix(upstart(y_monnam(u.usteed))),
                        aobjnam(otmp, "softly glow"),
                        hcolor(NH_AMBER));
                MSG_M_O_SOFTLY_GLOWS_COLOR
                MSG_M_O_SOFTLY_GLOWS_AMBER

            pline("%s looks rather %s.", Monnam(mtmp),
                    is_animal(mtmp->data) ? "nauseated" : "shook up");
            message_monster(MSG_M_LOOKS_RATHER_NAUSEATED, mtmp);

                pline("%s multiplies%s!", Monnam(mon), reason);
                message_monster(MSG_M_MULTIPLIES, mon);

                 message_monster(MSG_M_VANISHES, mtmp);
                 pline("%s vanishes.", Monnam(mtmp));

        message_const(MSG_DJINNI_EMERGES_BLIND);
        You("smell acrid fumes.");
        pline("%s speaks.", Something);

        message_monster(MSG_DJINNI_EMERGES, mtmp);
        pline("In a cloud of smoke, %s emerges!", a_monnam(mtmp));
        pline("%s speaks.", Monnam(mtmp));

                    pline("%s %s in pain!", Monnam(mon),
                            is_silent(mon->data) ? "writhes" : "shrieks");
                    message_monster(MSG_M_SHRIEKS_IN_PAIN, mon);

                        pline("%s rusts.", Monnam(mon));
                        message_monster(MSG_M_RUSTS, mon);

                            pline("%s looks healthier.", Monnam(mon));
                            message_monster(MSG_M_LOOKS_HEALTHIER, mon);

                        pline("%s %s in pain!", Monnam(mon),
                                is_silent(mon->data) ? "writhes" : "shrieks");
                        message_monster(MSG_M_SHRIEKS_IN_PAIN, mon);

                    message_monster(MSG_M_FALLS_ASLEEP, mon);
                    pline("%s falls asleep.", Monnam(mon));

                    pline("%s looks rather ill.", Monnam(mon));
                    message_monster(MSG_M_LOOKS_RATHER_ILL, mon);

                        pline("%s looks unharmed.", Monnam(mon));
                        message_monster(MSG_M_LOOKS_UNHARMED, mon);

                        message_monster(MSG_M_LOOKS_SOUND_AND_HALE, mon);
                        pline("%s looks sound and hale again.", Monnam(mon));

        pline("%s.", Tobjnam(obj, "evaporate"));
        message_object(MSG_O_EVAPORATES, obj);

    pline("%s is no longer in your clutches.", Monnam(u.ustuck));
    message_monster(MSG_M_NO_LONGER_IN_YOUR_CLUTCHES, u.ustuck);

                        message_monster(MSG_GAZING_AT_AWAKE_MEDUSA_BAD_IDEA, mtmp);
                        pline( "Gazing at the awake %s is not a very good idea.", l_monnam(mtmp));

                    message_monster(MSG_M_SEEMS_NOT_NOTICE_GAZE, mtmp);
                    pline("%s seems not to notice your gaze.", Monnam(mtmp));

                    message_monster(MSG_YOU_AVOID_GAZING_AT_M, mtmp);
                    You("avoid gazing at %s.", y_monnam(mtmp));

                    You_cant("see where to gaze at %s.", Monnam(mtmp));
                    message_monster(MSG_YOU_CANT_SEE_WHERE_GAZE_M, mtmp);

                            You("stiffen momentarily under %s gaze.", s_suffix(mon_nam(mtmp)));
                            message_monster(MSG_STIFFEN_MOMENTARILY_UNDER_M_GAZE, mtmp);

                            You("are frozen by %s gaze!", s_suffix(mon_nam(mtmp)));
                            message_monster(MSG_YOU_ARE_FROZEN_BY_M_GAZE, mtmp);

                            pline_The("fire doesn't burn %s!", mon_nam(mtmp));
                            message_monster(MSG_FIRE_DOES_NOT_BURN_M, mtmp);

                        message_monster(MSG_ATTACK_M_WITH_FIERY_GAZE, mtmp);
                        You("attack %s with a fiery gaze!", mon_nam(mtmp));

                    You_cant("loot anything %sthere with %s in the way.",
                            prev_inquiry ? "else " : "", mon_nam(mtmp));
                    message_monster(MSG_YOU_CANT_LOOT_WITH_M_IN_THE_WAY, mtmp);

                pline("No longer petrifying-resistant, you touch %s.", mon_nam(u.usteed));
                message_monster(MSG_TOUCH_PETRIFYING_STEED, u.usteed);

        You("release web fluid inside %s.", mon_nam(u.ustuck));
        message_monster(MSG_RELEASE_WEB_FLUID_INSIDE_M, u.ustuck);

        pline_The("web dissolves into %s.", mon_nam(u.ustuck));
        message_monster(MSG_WEB_DISSOLVES_INTO_M, u.ustuck);

                            message_monster(MSG_GAZE_CONFUSES_M, mtmp);
                            Your("gaze confuses %s!", mon_nam(mtmp));

                            message_monster(MSG_M_GETTING_MORE_CONFUSED, mtmp);
                            pline("%s is getting more and more confused.", Monnam(mtmp));

                sprintf(hornbuf, "horn%s", plur(num_horns(youmonst.data)));
                message_object(MSG_YOUR_HORNS_PIERCE_O, otmp);
                Your("%s %s through %s %s.", hornbuf, vtense(hornbuf, "pierce"),
                     shk_your(yourbuf, otmp), xname(otmp));

        message_object(MSG_O_SEEMS_TO_BE_LOCKED, obj);
        pline("%s to be locked.", Tobjnam(obj, "seem"));

            message_const(MSG_YOUR_LEASH_FALLS_SLACK);
            Your("leash falls slack.");

            message_monster(MSG_M_INSIDE_BOX_IS_ALIVE, livecat);
            pline("%s inside the box is still alive!", Monnam(livecat));

        pline("%s attached to your pet.", Tobjnam(obj, "are"));
        message_const(MSG_O_ARE_ATTACHED_TO_PET, obj);

            You_hear("%s stop moving.",something);
            message_const(MSG_SOMETHING_STOP_MOVING);

            char name[BUFSZ];
            Monnam(name, BUFSZ, mtmp);
            pline ("%s confuses itself!", name);
            message_monster(MSG_M_CONFUSES_ITSELF, mtmp);

            char name[BUFSZ];
            Monnam(name, BUFSZ, mtmp);
            pline ("%s is too tired to look at your mirror.", name);
            message_monster(MSG_M_TOO_TIRED_LOOK_MIRROR, mtmp);

            char name[BUFSZ];
            Monnam(name, BUFSZ, mtmp);
            pline("%s can't see anything right now.", name);
            message_monster(MSG_M_CANT_SEE_ANYTHING, mtmp);

            char name[BUFSZ];
            Monnam(name, BUFSZ, mtmp);
            pline ("%s doesn't have a reflection.", name);
            message_monster(MSG_M_HAS_NO_REFLECTION, mtmp);

            char name[BUFSZ];
            Monnam(name, BUFSZ, mtmp);
            pline("%s is frozen by its reflection.", name);
            message_monster(MSG_M_FROZEN_BY_REFLECTION, mtmp);

            char name[BUFSZ];
            Monnam(name, BUFSZ, mtmp);
            pline("%s is turned to stone!", name);
            message_monster(MSG_M_IS_TURNED_TO_STONE, mtmp);

            message_string(MSG_YOU_REFLECT_THE_DUNGEON, 
            You("reflect the %s.", );

        message_const(MSG_YOU_APPLY_MIRROR_UNDERWATER);
        You(Hallucination() ?
                "give the fish a chance to fix their makeup." :
                "reflect the murky water.");

                    You("pull on the leash.");
                    message_const(MSG_YOU_PULL_ON_LEASH);

            message_const(MSG_MIRROR_FOGS_UP);
            pline_The("mirror fogs up and doesn't reflect!");

                You("look as %s as ever.",
                    ACURR(A_CHA) > 14 ?
                    (poly_gender()==1 ? "beautiful" : "handsome") : "ugly");
                message_const(MSG_YOU_LOOK_GOOD_AS_EVER);

            message_const(MSG_YOU_CANT_SEE_YOUR_FACE);
            You_cant("see your %s %s.",
                    ACURR(A_CHA) > 14 ?
                    (poly_gender()==1 ? "beautiful" : "handsome") :
                    "ugly",
                    body_part(FACE));

        char name[BUFSZ];
        mon_nam(name, BUFSZ, u.ustuck);
        if (!Blind) {
            You("reflect %s%s %s.", name, possessive_suffix(name), mbodypart(u.ustuck, STOMACH));
            message_monster(MSG_YOU_REFLECT_M_STOMACH, u.ustuck);
        }

MSG_NOTHING_HAPPENS
struct c_common_strings c_common_strings = {
        "Nothing happens.",             "That's enough tries!",
        "That is a silly thing to %s.", "shudder for a moment.",
        "something", "Something", "You can move again.", "Never mind.",
        "vision quickly clears.", {"the", "your"}
};

            char name[BUFSZ];
            Monnam(name, BUFSZ, mtmp);
            pline("%s pulls free of %s leash!", name, mhis(mtmp));
            message_monster(MSG_M_PULLS_FREE_OF_LEASH, mtmp);

        message_const(MSG_YOU_PRODUCE_HIGH_HUMMING_NOISE);
        You("produce a high-pitched humming noise.");

        message_const(MSG_YOU_CANNOT_LEASH_MORE_PETS);
        You("cannot leash any more pets.");

        message_const(MSG_LEASH_YOURSELF);
        pline("Leash yourself?  Very funny...");

        message_const(MSG_THERE_IS_NO_CREATURE_THERE);
        There("is no creature there.");

            char name[BUFSZ];
            Monnam(name, BUFSZ, mtmp);
            pline("%s %s leashed!", name, (!obj->leashmon) ?  "cannot be" : "is not");
            message_monster(MSG_M_NOT_LEASHED, mtmp);

            const char *name;
            if (spotmon) {
                char buf[BUFSZ];
                l_monnam(buf, BUFSZ, mtmp);
                name = buf;
            } else {
                name = "monster";
            }
            pline("This %s is already leashed.", name);
            message_monster(MSG_M_ALREADY_LEASHED, mtmp);

        char name[BUFSZ];
        l_monnam(name, BUFSZ, mtmp);
        You("slip the leash around %s%s.", spotmon ? "your " : "", name);
        message_monster(MSG_YOU_SLIP_LEASH_AROUND_M, mtmp);

        pline("This leash is not attached to that creature.");
        message_const(MSG_LEASH_NOT_ATTACHED_TO_CREATURE);

            message_const(MSG_LEASH_NOT_COME_OFF);
            pline_The("leash would not come off!");

        char name[BUFSZ];
        l_monnam(name, BUFSZ, mtmp);
        You("remove the leash from %s%s.", spotmon ? "your " : "", name);
        message_monster(MSG_YOU_REMOVE_LEASH_FROM_M, mtmp);

                        You_feel("%s leash go slack.", (number_leashed() > 1) ? "a" : "the");
                        message_const(MSG_YOU_FEEL_LEASH_GO_SLACK);

                    char name[BUFSZ];
                    Monnam(name, BUFSZ, mtmp);
                    pline("%s chokes on the leash!", name);
                    message_monster(MSG_M_CHOKES_ON_LEASH, mtmp);

                    char name[BUFSZ];
                    Monnam(name, BUFSZ, mtmp);
                    pline("%s%s leash snaps loose!", name, possessive_suffix(name));
                    message_monster(MSG_M_LEASH_SNAPS_LOOSE, mtmp);

                    char name[BUFSZ];
                    mon_nam(name, BUFSZ, mtmp);
                    Your("leash chokes %s to death!", name);
                    message_monster(MSG_YOUR_LEASH_CHOKES_M_TO_DEATH, mtmp);

MSG_FOUND_SECRET_DOOR, "You hear a hollow sound.  This must be a secret %s!";
MSG_FOUND_SECRET_PASSAGE, "You hear a hollow sound.  This must be a secret %s!";

MSG_WHISTLE_SHRILL, "You produce a %s whistling sound.";
MSG_WHISTLE_HIGH,"You produce a %s whistling sound.";

MSG_WHISTLE_MAGIC
        You("produce a %s whistling sound", Hallucination() ? "normal" : "strange");

MSG_FAILED_POLYMORPH, "%(monster_subject) shudders!"

MSG_YOU_LOOK_COLOR,
static const char look_str[] = "look %s.";
                You(look_str, hcolor(NULL));
static const char * const hcolors[] = {
MSG_YOU_LOOK_PEAKED
                You(look_str, "peaked");
MSG_YOU_LOOK_UNDERNOURISHED,
                You(look_str, "undernourished");

message_const(MSG_YOU_HAVE_NO_REFLECTION);
    You("don't have a reflection.");

message_const(MSG_HUH_NO_LOOK_LIKE_YOU);
    pline("Huh?  That doesn't look like you!");

message_const(MSG_YOU_STIFFEN_MOMENTARILY_UNDER_YOUR_GAZE);
    You("stiffen momentarily under your gaze.");

message_const(MSG_MIRROR_STARES_BACK);
    pline(Hallucination() ?
        "Yow!  The mirror stares back!" :
        "Yikes!  You've frozen yourself!");

strcpy(buf, xname(uarmc));
You(need_to_remove_outer_armor, buf, xname(otmp));
message_object2(MSG_YOU_MUST_REMOVE_O_TO_GREASE_O, uarmc, otmp);
static const char need_to_remove_outer_armor[] = "need to remove your %s to grease your %s.";

strcpy(buf, uarmc ? xname(uarmc) : "");
if (uarmc && uarm) strcat(buf, " and ");
strcat(buf, uarm ? xname(uarm) : "");
You(need_to_remove_outer_armor, buf, xname(otmp));
message_object3(MSG_YOU_MUST_REMOVE_O_AND_O_TO_GREASE_O, uarmc, uarm, otmp);
static const char need_to_remove_outer_armor[] = "need to remove your %s to grease your %s.";

MSG_NOT_ENOUGH_ROOM_TO_USE = "There's not enough room here to use that.",

message_const(MSG_NO_HIT_IF_CANNOT_SEE_SPOT);
    cant_see_spot[] = "won't hit anything if you can't see that spot.",

message_const(MSG_YOU_CANNOT_REACH_SPOT_FROM_HERE);
    cant_reach[] = "can't reach that spot from here.";

message_const(MSG_USING_CAMERA_UNDERWATER);
    pline("Using your camera underwater would void the warranty.");

MSG_YOU_TAKE_PICTURE_OF_SWALLOW
        char name[BUFSZ];
        mon_nam(name, BUFSZ, u.ustuck);
        You("take a picture of %s%s %s.", name, possessive_suffix(name), mbodypart(u.ustuck, STOMACH));

MSG_YOU_TAKE_PICTURE_OF_DUNGEON
        You("take a picture of the %s.", (u.dz > 0) ? surface(u.ux,u.uy) : ceiling(u.ux,u.uy));

message_const(MSG_YOU_HAVE_NO_FREE_HAND);
You("have no free %s!", body_part(HAND));

message_const(MSG_CANNOT_USE_WHILE_WEARING);
You("cannot use it while you're wearing it!");

message_const(old ? MSG_YOUR_HANDS_FILTHIER : MSG_YOUR_HANDS_GET_SLIMY)
Your("%s %s!", makeplural(body_part(HAND)),
        (old ? "are filthier than ever" : "get slimy"));

message_const(old ? MSG_YOUR_FACE_HAS_MORE_GUNK : MSG_YOUR_FACE_NOW_HAS_GUNK);
pline("Yecch! Your %s %s gunk on it!", body_part(FACE),
    (old ? "has more" : "now has"));

const char *what = (ublindf->otyp == LENSES) ?  "lenses" : "blindfold";
if (ublindf->cursed) {
    message_const(MSG_YOU_PUSH_YOUR_LENSES_CROOKED);
    You("push your %s %s.", what, rn2(2) ? "cock-eyed" : "crooked");
} else {
    struct obj *saved_ublindf = ublindf;
    message_const(MSG_YOU_PUSH_YOUR_LENSES_OFF);
    You("push your %s off.", what);
    Blindf_off(ublindf);
    dropx(saved_ublindf);
}

message_const(MSG_YOU_WIPE_OFF_YOUR_HANDS);
You("wipe off your %s.", makeplural(body_part(HAND)));

message_const(MSG_YOU_GOT_THE_GLOP_OFF);
pline("You've got the glop off.");

message_const(MSG_YOUR_FACE_FEELS_CLEAN_NOW);
Your("%s feels clean now.", body_part(FACE));

    message_const(MSG_YOUR_FACE_AND_HAND_ARE_CLEAN);
    Your("%s and %s are already clean.",
            body_part(FACE), makeplural(body_part(HAND)));

        message_const(MSG_ITS_DEAD_JIM);
        You_hear("a voice say, \"It's dead, Jim.\"");

            message_const(MSG_YOU_DETERMINE_ITS_DEAD);
            You("determine that %s unfortunate being is dead.",
                    (rx == u.ux && ry == u.uy) ? "this" : "that");

            message_object((ttmp && ttmp->ttyp == STATUE_TRAP) ?
                    MSG_STATUE_APPEARS_EXCELLENT : MSG_STATUE_APPEARS_EXTRAORDINARY);
            pline("%s appears to be in %s health for a statue.",
                    The(mons[otmp->corpsenm].mname),
                    (ttmp && ttmp->ttyp == STATUE_TRAP) ?  "extraordinary" : "excellent");

        message_const(MSG_YOU_HAVE_NO_HANDS);
        You("have no hands!");  

        message_const(MSG_YOU_HAVE_NO_FREE_HANDS);
        You("have no free %s.", body_part(HAND));

            char name[BUFSZ];
            Monnam(name, BUFSZ, u.ustuck);
            pline("%s interferes.", name);
            message_monster(MSG_MONSTER_INTERFERES, u.ustuck);
            mstatusline(u.ustuck);

                You_hear("faint splashing.");
                message_const(MSG_YOU_HEAR_FAINT_SPLASHING);

                message_string(MSG_YOU_CANNOT_REACH_THE_DUNGEON,
                        (u.dz > 0) ? surface(u.ux,u.uy) : ceiling(u.ux,u.uy));
                You_cant("reach the %s.", );

                message_const(MSG_YOU_HEAR_CRACKLING_OF_HELLFIRE);
                You_hear("the crackling of hellfire.");

                message_string(MSG_DUNGEON_SEEMS_HEALTHY_ENOUGH, surface(u.ux,u.uy));
                pline_The("%s seems healthy enough.", surface(u.ux,u.uy));

            message_const(MSG_YOU_HEAR_YOUR_HEART_BEAT);
            You_hear("your heart beat.");

        message_const(MSG_YOU_HEAR_FAINT_TYPING_NOISE);
        You_hear("a faint typing noise.");

        message_const(MSG_THE_INVISIBLE_MONSTER_MOVED);
        pline_The("invisible monster must have moved.");

        You("hear nothing special.");       // not You_hear()
        message_const(MSG_YOU_HEAR_NOTHING_SPECIAL);

MSG_WELDS_TO_YOUR_HAND:
                const char *tmp = xname(wep), *thestr = "The ";
                if (strncmp(tmp, thestr, 4) && !strncmp(The(tmp),thestr,4))
                    tmp = thestr;
                else
                    tmp = "";


                pline("%s%s %s to your %s!", tmp, aobjnam(wep, "weld"),
                        (wep->quan == 1L) ? "itself" : "themselves",
                        bimanual(wep) ?
                                (const char *)makeplural(body_part(HAND))
                                : body_part(HAND));

*/

size_t nh_strlcpy(char *dest, const char *source, size_t dest_size) {
    char *d = dest;
    const char *s = source;
    size_t n = dest_size;

    // copy as many bytes as will fit
    if (n != 0 && --n != 0) {
        do {
            if ((*d++ = *s++) == 0)
                break;
        } while (--n != 0);
    }

    // not enough room in dest, add \0 and traverse rest of source
    if (n == 0) {
        if (dest_size != 0)
            *d = '\0';
        while (*s++)
            continue;
    }

    // count does not include \0
    return s - source - 1;
}

size_t nh_vslprintf(char *dest, size_t dest_size, const char *format, va_list ap) {
    size_t n = (dest_size >= 1) ? (dest_size - 1) : 0;
    int ret = vsnprintf(dest, n, format, ap);
    assert(ret >= 0);
    dest[(ret > n) ? n : ret] = 0;
    return ret;
}

size_t nh_slprintf(char *dest, size_t dest_size, const char *format, ...) {
    va_list ap;  
    va_start(ap, format);
    int ret = nh_vslprintf(dest, dest_size, format, ap);
    va_end(ap);
    return ret;
}

/*

MSG_YOU_DISRUPT, You("disrupt %s!", name);
MSG_A_HUGE_HOLE_OPENS_UP, "A huge hole opens up..."
MSG_MONSTER_TURNS_INVISIBLE, "%s turns transparent!"
MSG_ENGULFER_OPENS_ITS_MOUTH:
                            if (Blind) {
                                You_feel("a sudden rush of air!");
                            } else {
                                char name[BUFSZ];
                                Monnam(name, BUFSZ, mtmp);
                                pline("%s opens its mouth!", name);
                            }
MSG_MONSTER_LOOKS_BETTER, "%s looks better."
MSG_MONSTER_LOOKS_MUCH_BETTER, "%s looks much better."
MSG_GOLEM_TURNS_TO_FLESH, "%s turns to flesh!"
MSG_GOLEM_LOOKS_FLESHY, "%s looks rather fleshy for a moment."
MSG_MONSTER_LOOKS_WEAKER, "%s suddenly seems weaker!"
MSG_MONSTER_IS_NOT_CARRYING_ANYTHING, "%s is not carrying anything."
MSG_DRAWN_INTO_FORMER_BODY, "%s is suddenly drawn into its former body!"
MSG_A_MONSTER_SUDDENLY_APPEARS, "%s suddenly appears!" % Amonnam()
MSG_POLYPILE_CREATES_GOLEM, "Some %(material)sobjects meld, and %(a_monnam)s arises from the pile!"
    material = {
        PM_IRON_GOLEM: "metal ";
        PM_STONE_GOLEM: "lithic ";
        PM_CLAY_GOLEM: "lithic ";
        PM_FLESH_GOLEM: "organic ";
        PM_WOOD_GOLEM: "wood ";
        PM_LEATHER_GOLEM: "leather ";
        PM_ROPE_GOLEM: "cloth ";
        PM_SKELETON: "bony ";
        PM_GOLD_GOLEM: "gold ";
        PM_GLASS_GOLEM: "glassy ";
        PM_PAPER_GOLEM: "paper ";
        PM_STRAW_GOLEM: "";
    }
MSG_SHOP_KEEPER_GETS_ANGRY, "%s gets angry!"
MSG_SHOP_KEEPER_IS_FURIOUS, "%s is furious!"
MSG_YOU_NEED_HANDS_TO_WRITE, You("need hands to be able to write!");
MSG_IT_SLIPS_FROM_YOUR_FINGERS, pline("%s from your %s.", Tobjnam(pen, "slip"), makeplural(body_part(FINGER)));
MSG_YOU_DONT_KNOW_THAT_IS_BLANK, You("don't know if that %s is blank or not!", typeword);
MSG_THAT_IS_NOT_BLANK, pline("That %s is not blank!", typeword);
MSG_CANT_WRITE_THAT_ITS_OBSCENE, You_cant("write that!"); pline("It's obscene!");
MSG_CANT_WRITE_BOOK_OF_THE_DEAD, "No mere dungeon adventurer could write that.";
MSG_CANT_WRITE_WHAT_YOU_DONT_KNOW, "Unfortunately you don't have enough information to go on.";
MSG_MARKER_TOO_DRY, Your("marker is too dry to write that!");
MSG_MARKER_DRIES_OUT, Your("marker dries out!");
MSG_SPELLBOOK_IS_UNFINISHED, pline_The("spellbook is left unfinished and your writing fades.");
MSG_SCROLL_IS_NOW_USELESS, pline_The("scroll is now useless and disappears!");
MSG_YOU_DISTRUPT_ENGULFER, You("disrupt %s!", name)
MSG_VISION_QUICKLY_CLEARS, Your("vision quickly clears.");
MSG_THATS_ENOUGH_TRIES, "That's enough tries!"
MSG_OBJECT_STOPS_GLOWING, pline("%s glowing.", Tobjnam(olduwep, "stop"))
MSG_OBJECT_GLOWS_BRILLIANTLY, pline("%s to glow brilliantly!", Tobjnam(wep, "begin"));;
MSG_MONSTER_HIDING_UNDER_OBJECT, pline("Wait!  There's %s hiding under %s!", an(l_monnam(mtmp)), doname(obj));
MSG_YOUR_OBJECT_IS_NO_LONGER_POISONED, Your("%s %s no longer poisoned.", xname(obj), otense(obj, "are"));
MSG_STEED_STOPS_GALLOPING, pline("%s stops galloping.", Monnam(u.usteed));
MSG_YOUR_ATTACK_PASSES_HARMLESSLY_THROUGH_MONSTER, "Your attack passes harmlessly through %s." mon_nam(mon)
MSG_YOUR_WEAPON_PASSES_HARMLESSLY_THROUGH_MONSTER, Your("%s %s harmlessly through %s.", cxname(obj), vtense(cxname(obj), "pass"), mon_nam(mon));
MSG_YOU_JOUST_IT, You("joust %s%s", mon_nam(mon), canseemon(mon) ? exclam(tmp) : ".");
MSG_YOUR_LANCE_SHATTERS, Your("%s shatters on impact!", xname(obj));
MSG_M_STAGGERS_FROM_YOUR_POWERFUL_STRIKE, pline("%s %s from your powerful strike!", Monnam(mon), makeplural(stagger(mon->data, "stagger")));
MSG_M_DIVIDES_AS_YOU_HIT_IT, pline("%s divides as you hit it!", Monnam(mon));
MSG_YOU_HIT_M, You("%s %s%s", Role_if(PM_BARBARIAN) ? "smite" : "hit", mon_nam(mon), canseemon(mon) ? exclam(tmp) : ".");
MSG_YOUR_SILVER_RING_SEARS_M_FLESH:
        char *whom = mon_nam(mon);
        if (!noncorporeal(mdat))
            whom = strcat(s_suffix(whom), " flesh");
        "Your silver ring sears %s!";
        "Your silver rings sear %s!";
MSG_IT_IS_SEARED, "It is seared!"
MSG_ITS_FLESH_IS_SEARED, "Its flesh is seared!"
MSG_THE_SILVER_SEARS_M, "The silver sears %s!" mon_name();
MSG_THE_SILVER_SEARS_M_FLESH, "The silver sears %s flesh!" s_suffix(mon_name());
MSG_THE_POISON_DOESNT_SEEM_TO_AFFECT_M, pline_The("poison doesn't seem to affect %s.", mon_nam(mon));
MSG_THE_POISON_WAS_DEADLY, pline_The("poison was deadly...");
MSG_M_APPEARS_CONFUSED, pline("%s appears confused.", Monnam(mon));
MSG_THE_GREASE_WEARS_OFF, pline_The("grease wears off.");
MSG_YOU_SLIP_OFF_M_GREASED_O, You("slip off of %s greased %s!", s_suffix(mon_nam(mdef)), xname(obj));
MSG_YOU_SLIP_OFF_M_SLIPPERY_O You("slip off of %s slippery %s!", s_suffix(mon_nam(mdef)), xname(obj).replace(/^slippery /, ""));
MSG_YOU_GRAB_BUT_CANNOT_HOLD_ONTO_M_GREASED_O, You("grab, but cannot hold onto %s greased %s!", s_suffix(mon_nam(mdef)), xname(obj));
MSG_YOU_GRAB_BUT_CANNOT_HOLD_ONTO_M_SLIPPERY_O You("grab, but cannot hold onto %s slippery %s!", s_suffix(mon_nam(mdef)), xname(obj).replace(/^slippery /, ""));
MSG_YOU_CHARM_HER_AND_STEAL_EVERYTHING, You("charm %s.  She gladly hands over her possessions.", mon_nam(mdef));
MSG_YOU_SEDUCE_M_AND_HE_GETS_NAKED, You("seduce %s and %s starts to take off %s clothes.", mon_nam(mdef), mhe(mdef), mhis(mdef));
MSG_HE_FINISHES_TAKING_OFF_HIS_SUIT, pline("%s finishes taking off %s suit.", Monnam(mdef), mhis(mdef));
*/
