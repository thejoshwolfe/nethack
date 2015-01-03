#ifndef UTIL_H
#define UTIL_H

#include "wintype.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

// These functions return the number of bytes that we *wanted* to write,
// excluding the \0. If this number is >= than dest_size, the buffer was
// too small.
size_t nh_strlcpy(char *dest, const char *source, size_t dest_size);
size_t nh_vslprintf(char *dest, size_t dest_size, const char *format, va_list ap);
size_t nh_slprintf(char *dest, size_t dest_size, const char *format, ...) __attribute__ ((format (printf, 3, 4)));



void update_inventory(void);
void getlin(const char *, char *);

enum MessageId {
    MSG_YOU_HAVE_SUMMONED_M,
    MSG_WIDE_ANGLE_DISINTEGRATION_BEAM_HITS_M,
    MSG_M_SEEMS_UNAFFECTED,
    MSG_M_FRIES_TO_CRISP,
    MSG_IT_STRIKES_M,
    MSG_M_O_SOFTLY_GLOWS_AMBER,
    MSG_M_LOOKS_RATHER_NAUSEATED,
    MSG_M_MULTIPLIES,
    MSG_M_VANISHES,
    MSG_DJINNI_EMERGES_BLIND,
    MSG_DJINNI_EMERGES,
    MSG_M_RUSTS,
    MSG_M_LOOKS_HEALTHIER,
    MSG_M_SHRIEKS_IN_PAIN,
    MSG_M_FALLS_ASLEEP,
    MSG_M_LOOKS_RATHER_ILL,
    MSG_M_LOOKS_UNHARMED,
    MSG_M_LOOKS_SOUND_AND_HALE,
    MSG_O_EVAPORATES,
    MSG_M_NO_LONGER_IN_YOUR_CLUTCHES,
    MSG_GAZING_AT_AWAKE_MEDUSA_BAD_IDEA,
    MSG_YOU_AVOID_GAZING_AT_M,
    MSG_YOU_CANT_SEE_WHERE_GAZE_M,
    MSG_M_SEEMS_NOT_NOTICE_GAZE,
    MSG_STIFFEN_MOMENTARILY_UNDER_M_GAZE,
    MSG_YOU_ARE_FROZEN_BY_M_GAZE,
    MSG_FIRE_DOES_NOT_BURN_M,
    MSG_ATTACK_M_WITH_FIERY_GAZE,
    MSG_M_GETTING_MORE_CONFUSED,
    MSG_GAZE_CONFUSES_M,
    MSG_WEB_DISSOLVES_INTO_M,
    MSG_RELEASE_WEB_FLUID_INSIDE_M,
    MSG_YOUR_HORNS_PIERCE_O,
    MSG_TOUCH_PETRIFYING_STEED,
    MSG_O_SEEMS_TO_BE_LOCKED,
    MSG_M_INSIDE_BOX_IS_ALIVE,
    MSG_O_ARE_ATTACHED_TO_PET,
    MSG_YOU_CANT_LOOT_WITH_M_IN_THE_WAY,
    MSG_M_CONFUSES_ITSELF,
    MSG_SOMETHING_STOP_MOVING,
    MSG_M_FROZEN_BY_REFLECTION,
    MSG_M_IS_TURNED_TO_STONE,
    MSG_M_HAS_NO_REFLECTION,
    MSG_M_CANT_SEE_ANYTHING,
    MSG_M_TOO_TIRED_LOOK_MIRROR,
    MSG_YOU_REFLECT_THE_DUNGEON,
    MSG_YOU_APPLY_MIRROR_UNDERWATER,
    MSG_YOU_REFLECT_M_STOMACH,
    MSG_YOU_CANT_SEE_YOUR_FACE,
    MSG_YOU_LOOK_GOOD_AS_EVER,
    MSG_MIRROR_FOGS_UP,
    MSG_YOU_PULL_ON_LEASH,
    MSG_M_LEASH_SNAPS_LOOSE,
    MSG_M_CHOKES_ON_LEASH,
    MSG_YOUR_LEASH_CHOKES_M_TO_DEATH,
    MSG_YOU_FEEL_LEASH_GO_SLACK,
    MSG_YOU_REMOVE_LEASH_FROM_M,
    MSG_LEASH_NOT_COME_OFF,
    MSG_LEASH_NOT_ATTACHED_TO_CREATURE,
    MSG_YOU_SLIP_LEASH_AROUND_M,
    MSG_M_ALREADY_LEASHED,
    MSG_M_NOT_LEASHED,
    MSG_THERE_IS_NO_CREATURE_THERE,
    MSG_LEASH_YOURSELF,
    MSG_YOU_CANNOT_LEASH_MORE_PETS,
    MSG_YOU_PRODUCE_HIGH_HUMMING_NOISE,
    MSG_M_PULLS_FREE_OF_LEASH,
    MSG_YOUR_LEASH_FALLS_SLACK,
    MSG_NOTHING_HAPPENS,
    MSG_YOU_HEAR_NOTHING_SPECIAL,
    MSG_THE_INVISIBLE_MONSTER_MOVED,
    MSG_YOU_HEAR_FAINT_TYPING_NOISE,
    MSG_YOU_HEAR_YOUR_HEART_BEAT,
    MSG_DUNGEON_SEEMS_HEALTHY_ENOUGH,
    MSG_YOU_HEAR_CRACKLING_OF_HELLFIRE,
    MSG_YOU_CANNOT_REACH_THE_DUNGEON,
    MSG_YOU_HEAR_FAINT_SPLASHING,
    MSG_MONSTER_INTERFERES,
    MSG_YOU_HAVE_NO_FREE_HANDS,
    MSG_YOU_HAVE_NO_HANDS,
    MSG_STATUE_APPEARS_EXCELLENT,
    MSG_STATUE_APPEARS_EXTRAORDINARY,
    MSG_YOU_DETERMINE_ITS_DEAD,
    MSG_ITS_DEAD_JIM,
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
    MSG_YOU_DISRUPT,
    MSG_A_HUGE_HOLE_OPENS_UP,
    MSG_MONSTER_TURNS_INVISIBLE,
    MSG_ENGULFER_OPENS_ITS_MOUTH,
    MSG_MONSTER_LOOKS_BETTER,
    MSG_MONSTER_LOOKS_MUCH_BETTER,
    MSG_GOLEM_TURNS_TO_FLESH,
    MSG_GOLEM_LOOKS_FLESHY,
    MSG_MONSTER_LOOKS_WEAKER,
    MSG_MONSTER_IS_NOT_CARRYING_ANYTHING,
    MSG_DRAWN_INTO_FORMER_BODY,
    MSG_A_MONSTER_SUDDENLY_APPEARS,
    MSG_POLYPILE_CREATES_GOLEM,
    MSG_SHOP_KEEPER_GETS_ANGRY,
    MSG_SHOP_KEEPER_IS_FURIOUS,
    MSG_YOU_NEED_HANDS_TO_WRITE,
    MSG_IT_SLIPS_FROM_YOUR_FINGERS,
    MSG_YOU_DONT_KNOW_THAT_IS_BLANK,
    MSG_THAT_IS_NOT_BLANK,
    MSG_CANT_WRITE_THAT_ITS_OBSCENE,
    MSG_CANT_WRITE_BOOK_OF_THE_DEAD,
    MSG_CANT_WRITE_WHAT_YOU_DONT_KNOW,
    MSG_MARKER_TOO_DRY,
    MSG_MARKER_DRIES_OUT,
    MSG_SPELLBOOK_IS_UNFINISHED,
    MSG_SCROLL_IS_NOW_USELESS,
    MSG_YOU_DISTRUPT_ENGULFER,
    MSG_VISION_QUICKLY_CLEARS,
    MSG_THATS_ENOUGH_TRIES,
    MSG_OBJECT_STOPS_GLOWING,
    MSG_OBJECT_GLOWS_BRILLIANTLY,
    MSG_MONSTER_HIDING_UNDER_OBJECT,
    MSG_YOUR_OBJECT_IS_NO_LONGER_POISONED,
    MSG_STEED_STOPS_GALLOPING,
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
void message_monster_force_visible(enum MessageId id, const struct monst *m);
void message_object(enum MessageId id, const struct obj *o);
void message_object2(enum MessageId id, const struct obj *o1, const struct obj *o2);
void message_object3(enum MessageId id, const struct obj *o1, const struct obj *o2, const struct obj *o3);
void message_int(enum MessageId id, int i);
void message_monster_string(enum MessageId id, const struct monst *m, const char *s);
void message_string(enum MessageId id, const char *s);
void message_monster_object(enum MessageId id, const struct monst * mtmp, struct obj * obj);

// stuff to make it just friggin compile
// creep forward and delete all this junk

static const char *Something = "Something";
static const char *something = "something";
static const char *Never_mind = "Never mind.";
static const char *nothing_happens = "Nothing happens.";
static const char *thats_enough_tries = "That's enough tries!";
static const char *silly_thing_to = "That is a silly thing to %s.";
static const char *shudder_for_moment = "shudder for a moment.";
static const char *You_can_move_again = "You can move again.";
static const char *vision_clears = "vision quickly clears.";
static const char *the_your[2] = {"the", "your"};
static char killer_buf[256] = {0};


#define DIED           KM_DIED
#define CHOKING        KM_CHOKING
#define POISONING      KM_POISONING
#define STARVING       KM_STARVING
#define DROWNING       KM_DROWNING
#define BURNING        KM_BURNING
#define DISSOLVED      KM_DISSOLVED
#define CRUSHING       KM_CRUSHING
#define STONING        KM_STONING
#define TURNED_SLIME   KM_TURNED_SLIME
#define GENOCIDED      KM_GENOCIDED
#define PANICKED       KM_PANICKED
#define TRICKED        KM_TRICKED
#define QUIT           KM_QUIT
#define ESCAPED        KM_ESCAPED
#define ASCENDED       KM_ASCENDED

int create_nhwindow(int);
void destroy_nhwindow(int);
void display_nhwindow(int, bool);
void clear_nhwindow(int);
void my_delay_output(void);
void print_glyph(int, signed char x, signed char y, int glyph);
int query_category ( const char *qstr, struct obj *olist, int qflags, menu_item **pick_list, int how);
int query_objlist(const char *qstr, struct obj *olist, int qflags,
        menu_item **pick_list, int how, bool (*allow)(const struct obj *));
void mark_synch(void);
void wait_synch(void);
void bot(void);
char message_menu(char, int, const char *);
void start_menu(int);
void add_menu(int, int, const void *, char, char, int, const char *, bool);
void end_menu(int, const char *);
int select_menu(int, int, menu_item **);

void putstr(int, int, const char *);

#endif // UTIL_H
