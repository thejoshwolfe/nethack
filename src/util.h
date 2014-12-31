#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdarg.h>

// These functions return the number of bytes that we *wanted* to write,
// excluding the \0. If this number is >= than dest_size, the buffer was
// too small.
size_t nh_strlcpy(char *dest, const char *source, size_t dest_size);
size_t nh_vslprintf(char *dest, size_t dest_size, char *format, va_list ap);
size_t nh_slprintf(char *dest, size_t dest_size, char *format, ...) __attribute__ ((format (printf, 3, 4)));



void update_inventory(void);
void getlin(const char *, char *);

enum MessageId {
    MSG_YOUR_FACE_AND_HAND_ARE_CLEAN,
    MSG_YOUR_FACE_FEELS_CLEAN_NOW,
    MSG_YOU_GOT_THE_GLOP_OFF,
    MSG_YOU_WIPE_OFF_YOUR_HANDS,
    MSG_YOU_PUSH_YOUR_LENSES_CROOKED,
    MSG_YOU_PUSH_YOUR_LENSES_OFF,
    MSG_YOUR_FACE_HAS_MORE_GUNK,
    MSG_YOUR_FACE_NOW_HAS_GUNK,
    MSG_YOUR_HANDS_FILTHIER,
    MSG_YOUR_HANDS_GET_SLIMY,
    MSG_CANNOT_USE_WHILE_WEARING,
    MSG_YOU_HAVE_NO_FREE_HAND,
    MSG_YOU_TAKE_PICTURE_OF_DUNGEON,
    MSG_YOU_TAKE_PICTURE_OF_SWALLOW,
    MSG_USING_CAMERA_UNDERWATER,
    MSG_NO_HIT_IF_CANNOT_SEE_SPOT,
    MSG_YOU_CANNOT_REACH_SPOT_FROM_HERE,
    MSG_NOT_ENOUGH_ROOM_TO_USE,
    MSG_YOU_MUST_REMOVE_O_TO_GREASE_O,
    MSG_YOU_MUST_REMOVE_O_AND_O_TO_GREASE_O,
    MSG_MIRROR_STARES_BACK,
    MSG_YOU_STIFFEN_MOMENTARILY_UNDER_YOUR_GAZE,
    MSG_HUH_NO_LOOK_LIKE_YOU,
    MSG_YOU_HAVE_NO_REFLECTION,
    MSG_YOU_LOOK_UNDERNOURISHED,
    MSG_YOU_LOOK_PEAKED,
    MSG_YOU_LOOK_COLOR,
    MSG_WHISTLE_MAGIC,
    MSG_WHISTLE_SHRILL,
    MSG_WHISTLE_HIGH,
    MSG_FOUND_SECRET_DOOR,
    MSG_FOUND_SECRET_PASSAGE,
    MSG_NO_ELBOW_ROOM,
    MSG_FAILED_POLYMORPH,
    MSG_WELDS_TO_YOUR_HAND,
};

struct Message {
    enum MessageId id;
    const struct monst *monster1;
    const char *string1;
    const struct obj *object1;
    const struct obj *object2;
    const struct obj *object3;
    int int1;
};

void message_const(enum MessageId id);
void message_monster(enum MessageId id, const struct monst *m);
void message_object(enum MessageId id, const struct obj *o);
void message_object2(enum MessageId id, const struct obj *o1, const struct obj *o2);
void message_object3(enum MessageId id, const struct obj *o1, const struct obj *o2,
        const struct obj *o3);
void message_int(enum MessageId id, int i);
void message_monster_string(enum MessageId id, const struct monst *m, const char *s);
void message_string(enum MessageId id, const char *s);

#endif // UTIL_H
