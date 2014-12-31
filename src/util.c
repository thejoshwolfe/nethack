#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>


/*

MSG_NO_ELBOW_ROOM:  "You don't have enough elbow-room to maneuver.";


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

message_string(MSG_YOU_HAVE_NO_FREE_HAND, body_part(HAND));
You("have no free %s!", body_part(HAND));

message_const(MSG_CANNOT_USE_WHILE_WEARING);
You("cannot use it while you're wearing it!");

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

*/
