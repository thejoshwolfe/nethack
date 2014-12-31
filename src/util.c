#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>


/*

MSG_NO_ELBOW_ROOM:  "You don't have enough elbow-room to maneuver.";

            message_const(MSG_YOUR_LEASH_FALLS_SLACK);
            Your("leash falls slack.");

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

size_t nh_vslprintf(char *dest, size_t dest_size, char *format, va_list ap) {
    size_t n = (dest_size >= 1) ? (dest_size - 1) : 0;
    int ret = vsnprintf(dest, n, format, ap);
    assert(ret >= 0);
    dest[(ret > n) ? n : ret] = 0;
    return ret;
}

size_t nh_slprintf(char *dest, size_t dest_size, char *format, ...) {
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
*/
