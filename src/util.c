#include "util.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

static const char *message_id_str[] = {
    "A_BLIND_M_CANNOT_DEFEND_ITSELF",
    "YOU_MISS_M",
    "M_IS_BLINDED_BY_THE_FLASH",
    "M_LOOKS_CONFUSED",
    "M_SLOWS_DOWN",
    "M_IS_PUT_TO_SLEEP_BY_YOU",
    "M_IS_FROZEN_BY_YOU",
    "M_IS_BEING_CRUSHED",
    "M_IS_BURNING_TO_A_CRISP",
    "M_SEEMS_MIDLY_HOT",
    "M_IS_FREEZING_TO_DEATH",
    "M_SEEMS_MILDLY_CHILLY",
    "M_SEEMS_UNHURT",
    "M_CANT_SEE_IN_THERE",
    "M_IS_COVERED_WITH_YOUR_GOO",
    "M_IS_PUMMELED_WITH_YOUR_DEBRIS",
    "M_SEEMS_UNHARMED",
    "M_IS_LADEN_WITH_YOUR_MOISTURE",
    "M_GETS_BLASTED",
    "M_IS_AFFECTED_BY_YOUR_FLASH_OF_LIGHT",
    "M_IS_BLINDED_BY_YOUR_FLASH_OF_LIGHT",
    "THE_FLAG_AWAKENS_M",
    "YOU_ARE_FROZEN_BY_M",
    "YOU_MOMENTARILY_STIFFEN_UNDER_M_GAZE",
    "YOU_ARE_SPLASHED_BY_M_ACID",
    "YOU_PRETEND_TO_BE_FRIENDLY_TO_M",
    "YOU_BITE_INTO_M",
    "YOU_DID_NOT_LIKE_M_TASTE",
    "THE_AIR_AROUND_M_CRACKLES_WITH_ELECTRICITY",
    "IT_SEEMS_HARMLESS_TO_M",
    "YOU_DIGEST_M",
    "YOU_ENGULF_M",
    "THE_BLAST_DOESNT_SEEM_TO_AFFECT_M",
    "YOU_TURN_M_INTO_SLIME",
    "YOU_BRUSH_AGAINST_M_LEG",
    "YOU_DROWN_M",
    "YOUR_ATTEMPT_TO_SURROUND_M_IS_HARMLESS",
    "YOU_GRAB_M",
    "YOUR_HUG_PASSES_HARMLESSLY_THROUGH_M",
    "YOUR_TENTACLES_SUCK_M",
    "YOU_BUTT_M",
    "YOU_STING_M",
    "YOU_BITE_M",
    "YOU_KICK_M",
    "YOUR_ATTACK_PASSES_HARMLESSLY_THROUGH_M",
    "O_EXPLODES",
    "M_STIRS",
    "M_AWAKENS",
    "M_IS_NOT_TRAPPED",
    "M_THINKS_IT_WAS_NICE_OF_YOU_TO_TRY",
    "M_IS_GRATEFUL",
    "M_REMAINS_ENTANGLED",
    "YOU_PULL_M_OUT_OF_THE_PIT",
    "YOU_REACH_OUT_YOUR_ARM_AND_GRAB_M",
    "YOU_TRY_TO_GRAB_M_BUT_CANNOT_GET_A_GRASP",
    "M_IS_IN_THE_WAY",
    "YOU_MUST_LET_GO_OF_M_FIRST",
    "M_CAN_NO_LONGER_HOLD_YOU",
    "YOU_ARE_NOT_ABLE_TO_HOLD",
    "YOU_CANNOT_STAY_ON_M",
    "M_TURNS_TO_STONE",
    "TRIGGER_APPEARS_IN_SOIL_BELOW_M",
    "M_SEEMS_TO_BE_YANKED_DOWN",
    "M_IS_UNINJURED",
    "YOU_SEE_TOWER_OF_FLAME_ERUPT_FROM_S",
    "TOWER_OF_FLAME_ERUPTS_FROM_S_UNDER_M",
    "MAY_M_RUST_IN_PEACE",
    "GUSH_OF_WATER_HITS_M",
    "GUSH_OF_WATER_HITS_M_RIGHT_ARM",
    "GUSH_OF_WATER_HITS_M_LEFT_ARM",
    "GUSH_OF_WATER_HITS_M_HEAD",
    "M_CAUGHT_IN_BEAR_TRAP",
    "BOARD_BENEATH_M_SQEAKS_LOUDLY",
    "TRAP_DOOR_ABOVE_M_OPENS_NOTHING_FALLS_OUT",
    "M_TRIGGERS_TRAP_BUT_NOTHING_HAPPENS",
    "M_MUNCHES_ON_SOME_SPIKES",
    "M_EATS_A_BEAR_TRAP",
    "M_PULLS_FREE",
    "M_SNATCHES_THE_BOULDER",
    "YOU_HAVE_TO_ADJUST_YOURSELF_IN_THE_SADDLE_ON_M",
    "M_SUDDENLY_FALLS_ASLEEP",
    "M_IS_ALMOST_HIT_BY_O",
    "YOU_LAND_ON_SHARP_IRON_SPIKES",
    "M_LANDS_ON_SHARP_IRON_SPIKES",
    "BEAR_TRAP_CLOSES_ON_YOUR_FOOT",
    "BEAR_TRAP_CLOSES_ON_M_FOOT",
    "YOU_RO_DOES_NOT_PROTECT_YOU",
    "TRAP_DOOR_IN_CEILING_OPENS_AND_O_FALLS_ON_YOUR_HEAD",
    "YOU_FIND_M_POSING_AS_STATUE",
    "YOU_FIND_SOMETHING_POSING_AS_STATUE",
    "YOUR_O_ARE_PROTECTED_BY_GREASE",
    "M_O_ARE_PROTECTED_BY_GREASE",
    "M_LANTERN_GETTING_DIM",
    "M_SLIPS_ON_THE_ICE",
    "YOU_SLIP_ON_THE_ICE",
    "YOU_ARE_NO_LONGER_INSIDE_M",
    "M_RESISTS_YOUR_MAGIC",
    "SUDDENDLY_M_DISAPPEARS_OUT_OF_SIGHT",
    "M_SHUDDERS_FOR_A_MOMENT",
    "M_SEEMS_DISORIENTED_FOR_A_MOMENT",
    "M_SEEMS_TO_SHIMMER_FOR_A_MOMENT",
    "M_AVOIDS_THE_TRAP",
    "MYSTERIOUS_FORCE_PREVENTS_M_FROM_TELEPORTING",
    "M_IS_PULLED_INTO_THE_LAVA",
    "M_FALLS_INTO_THE_S",
    "YOU_DISMOUNT_M",
    "YOU_CAN_NO_LONGER_RIDE_M",
    "YOU_FALL_OFF_OF_M",
    "M_GALLOPS",
    "YOU_MOUNT_M",
    "M_MAGICALLY_FLOATS_UP",
    "YOU_SLIP_WHILE_TRYING_TO_GET_ON_M",
    "M_SLIPS_AWAY_FROM_YOU",
    "YOUR_RUSTY_ARMOR_TOO_STIFF_TO_MOUNT_M",
    "YOU_CANNOT_REACH_M",
    "YOU_CANT_MOUNT_M_WHILE_HES_TRAPPED_IN_S",
    "M_IS_NOT_SADDLED",
    "YOU_ARE_ALREADY_RIDING_M",
    "M_RESISTS",
    "YOU_PUT_SADDLE_ON_M",
    "I_THINK_M_WOULD_MIND",
    "YOU_TOUCH_M",
    "M_DOES_NOT_NEED_ANOTHER_ONE",
    "YOU_ARE_NOT_SKILLED_ENOUGH_TO_REACH_FROM_M",
    "O_GO_OUT",
    "M_DROPS_X_GOLD_PIECES",
    "M_DROPS_O",
    "M_STOLE_O",
    "M_TRIES_TO_STEAL_YOUR_O_BUT_GIVES_UP",
    "M_TRIES_TO_ROB_BUT_YOURE_NAKED",
    "M_STEALS_O",
    "M_QUICKLY_SNATCHES_GOLD_FROM_BEETWEEN_YOUR_LEGS",
    "M_IS_EAT_NOISILY",
    "M_SEEMS_NOT_TO_NOTICE_YOU",
    "M_BOASTS_ABOUT_GEM_COLLECTION",
    "M_RATTLES_NOISILY",
    "M_THROWS_BACK_HEAD_LETS_OUT_HOWL",
    "M_WHIMPERS",
    "M_YELPS",
    "M_GROWLS",
    "M_O_GLOWS_COLOR",
    "M_O_GLOWS_BLACK",
    "M_O_GLOWS_BROWN",
    "O_RESISTS",
    "YOU_ARE_ALEADY_SITTING_ON_M",
    "YOU_OWE_M_X_GOLD",
    "YOU_OWE_M_X_GOLD_FOR_THEM",
    "M_ASKS_WHETHER_YOUVE_SEEN_UNTENDED_SHOPS",
    "M_DOESNT_LIKE_CUSTOMERS_WHO_DONT_PAY",
    "M_NIMBLY_CATCHES_O",
    "M_CANNOT_PAY_YOU_AT_PRESENT",
    "M_SEEMS_UNINTERESTED",
    "M_DOES_NOT_NOTICE",
    "THANK_YOU_FOR_SHOPPING_IN_M_S",
    "PAY_FOR_THE_OTHER_O_BEFORE_BUYING_THIS",
    "M_SEEMS_TO_BE_NAPPING",
    "M_TOO_FAR_TO_RECEIVE_PAYMENT",
    "M_NOT_INTERESTED_IN_YOUR_PAYMENT",
    "M_NOT_NEAR_ENOUGH_FOR_PAYMENT",
    "M_GETS_ANGRY",
    "M_HAS_NO_INTEREST_IN_O",
    "M_LOOKS_AT_YOUR_CORPSE_AND_DISAPPEARS",
    "BUT_M_IS_AS_ANGRY_AS_EVER",
    "YOU_TRY_APPEASE_M_BY_GIVING_1000_GOLD_PIECES",
    "M_AFTER_YOUR_HIDE_NOT_YOUR_MONEY",
    "YOU_PARTIALLY_COMPENSATE_M_FOR_HIS_LOSSES",
    "YOU_COMPENSATE_M_FOR_HIS_LOSSES",
    "M_IS_AFTER_BLOOD_NOT_MONEY",
    "M_CALMS_DOWN",
    "UNFORTUNATELY_M_DOESNT_LOOK_SATISFIED",
    "YOU_GIVE_M_ALL_YOUR_GOLD",
    "YOU_GIVE_M_THE_X_GOLD_PIECES",
    "YOU_DO_NOT_OWE_M_ANYTHING",
    "YOU_HAVE_X_CREDIT_AT_M_S",
    "PLEASE_LEAVE_M_OUTSIDE",
    "LEAVE_M_OUTSIDE",
    "M_WAKES_UP",
    "M_CAN_MOVE_AGAIN",
    "M_NO_MOOD_FOR_CONSULTATIONS",
    "O_GOES_OUT",
    "M_GLISTENS",
    "M_SHINES_BRIEFLY",
    "M_STOMACH_IS_LIT",
    "ONLY_LIGHT_LEFT_COMES_FROM_O",
    "YOUR_O_VIBRATES_VIOLENTLY_AND_EXPLODES",
    "YOUR_O_DOES_NOT_PROTECT_YOU",
    "M_O_DOES_NOT_PROTECT_HIM",
    "M_WEARING_HARD_HELMET",
    "M_IS_HIT_BY_O",
    "YOUR_O_VIBRATES",
    "YOUR_O_SUDDENLY_VIBRATES_UNEXPECTEDLY",
    "O_MERGES_AND_HARDENS",
    "O_LOOK_AS_GOOD_AS_NEW",
    "O_ARE_COVERED_BY_BLACK_GLOW",
    "O_ARE_COVERED_BY_GOLDEN_GLOW",
    "YOUR_O_FEEL_WARM_MOMENT",
    "O_SPINS_COUNTER_CLOCKWISE",
    "O_SPINS_CLOCKWISE",
    "O_PULSATES_THEN_EXPLODES",
    "YOUR_O_VIBRATES_FOR_MOMENT",
    "YOUR_O_GLOWS_BRIEFLY",
    "YOUR_O_VIBRATES_BRIEFLY",
    "M_SPEAKS",
    "YOUR_DISPLACED_IMAGE_DOESNT_FOOL_M",
    "M_ASKS_FOR_CONTRIBUTION_TEMPLE",
    "M_IS_NOT_INTERESTED",
    "M_PREACHES_VIRTUES_POVERTY",
    "M_GIVES_YOU_ONE_ALE",
    "M_GIVES_YOU_TWO_ALE",
    "M_BREAKS_OUT_OF_HIS_REVERIE",
    "M_WANTS_NOTHING_TO_DO_WITH_YOU",
    "M_INTONES",
    "YOU_HAVE_SUMMONED_M",
    "WIDE_ANGLE_DISINTEGRATION_BEAM_HITS_M",
    "M_SEEMS_UNAFFECTED",
    "M_FRIES_TO_CRISP",
    "IT_STRIKES_M",
    "M_O_SOFTLY_GLOWS_COLOR",
    "M_O_SOFTLY_GLOWS_AMBER",
    "M_LOOKS_RATHER_NAUSEATED",
    "M_MULTIPLIES",
    "M_VANISHES",
    "DJINNI_EMERGES_BLIND",
    "DJINNI_EMERGES",
    "M_RUSTS",
    "M_LOOKS_HEALTHIER",
    "M_SHRIEKS_IN_PAIN",
    "M_FALLS_ASLEEP",
    "M_LOOKS_RATHER_ILL",
    "M_LOOKS_UNHARMED",
    "M_LOOKS_SOUND_AND_HALE",
    "O_EVAPORATES",
    "M_NO_LONGER_IN_YOUR_CLUTCHES",
    "GAZING_AT_AWAKE_MEDUSA_BAD_IDEA",
    "YOU_AVOID_GAZING_AT_M",
    "YOU_CANT_SEE_WHERE_GAZE_M",
    "M_SEEMS_NOT_NOTICE_GAZE",
    "STIFFEN_MOMENTARILY_UNDER_M_GAZE",
    "YOU_ARE_FROZEN_BY_M_GAZE",
    "FIRE_DOES_NOT_BURN_M",
    "ATTACK_M_WITH_FIERY_GAZE",
    "M_GETTING_MORE_CONFUSED",
    "GAZE_CONFUSES_M",
    "WEB_DISSOLVES_INTO_M",
    "RELEASE_WEB_FLUID_INSIDE_M",
    "YOUR_HORNS_PIERCE_O",
    "TOUCH_PETRIFYING_STEED",
    "O_SEEMS_TO_BE_LOCKED",
    "M_INSIDE_BOX_IS_ALIVE",
    "O_ARE_ATTACHED_TO_PET",
    "YOU_CANT_LOOT_WITH_M_IN_THE_WAY",
    "M_CONFUSES_ITSELF",
    "SOMETHING_STOP_MOVING",
    "M_FROZEN_BY_REFLECTION",
    "M_IS_TURNED_TO_STONE",
    "M_HAS_NO_REFLECTION",
    "M_CANT_SEE_ANYTHING",
    "M_TOO_TIRED_LOOK_MIRROR",
    "YOU_REFLECT_THE_DUNGEON",
    "YOU_APPLY_MIRROR_UNDERWATER",
    "YOU_REFLECT_M_STOMACH",
    "YOU_CANT_SEE_YOUR_FACE",
    "YOU_LOOK_GOOD_AS_EVER",
    "MIRROR_FOGS_UP",
    "YOU_PULL_ON_LEASH",
    "M_LEASH_SNAPS_LOOSE",
    "M_CHOKES_ON_LEASH",
    "YOUR_LEASH_CHOKES_M_TO_DEATH",
    "YOU_FEEL_LEASH_GO_SLACK",
    "YOU_REMOVE_LEASH_FROM_M",
    "LEASH_NOT_COME_OFF",
    "LEASH_NOT_ATTACHED_TO_CREATURE",
    "YOU_SLIP_LEASH_AROUND_M",
    "M_ALREADY_LEASHED",
    "M_NOT_LEASHED",
    "THERE_IS_NO_CREATURE_THERE",
    "LEASH_YOURSELF",
    "YOU_CANNOT_LEASH_MORE_PETS",
    "YOU_PRODUCE_HIGH_HUMMING_NOISE",
    "M_PULLS_FREE_OF_LEASH",
    "YOUR_LEASH_FALLS_SLACK",
    "NOTHING_HAPPENS",
    "YOU_HEAR_NOTHING_SPECIAL",
    "THE_INVISIBLE_MONSTER_MOVED",
    "YOU_HEAR_FAINT_TYPING_NOISE",
    "YOU_HEAR_YOUR_HEART_BEAT",
    "DUNGEON_SEEMS_HEALTHY_ENOUGH",
    "YOU_HEAR_CRACKLING_OF_HELLFIRE",
    "YOU_CANNOT_REACH_THE_DUNGEON",
    "YOU_HEAR_FAINT_SPLASHING",
    "MONSTER_INTERFERES",
    "YOU_HAVE_NO_FREE_HANDS",
    "YOU_HAVE_NO_HANDS",
    "STATUE_APPEARS_EXCELLENT",
    "STATUE_APPEARS_EXTRAORDINARY",
    "YOU_DETERMINE_ITS_DEAD",
    "ITS_DEAD_JIM",
    "YOUR_FACE_AND_HAND_ARE_CLEAN",
    "YOUR_FACE_FEELS_CLEAN_NOW",
    "YOU_GOT_THE_GLOP_OFF",
    "YOU_WIPE_OFF_YOUR_HANDS",
    "YOU_PUSH_YOUR_LENSES_CROOKED",
    "YOU_PUSH_YOUR_LENSES_OFF",
    "YOUR_FACE_HAS_MORE_GUNK",
    "YOUR_FACE_NOW_HAS_GUNK",
    "YOUR_HANDS_FILTHIER",
    "YOUR_HANDS_GET_SLIMY",
    "CANNOT_USE_WHILE_WEARING",
    "YOU_HAVE_NO_FREE_HAND",
    "YOU_TAKE_PICTURE_OF_DUNGEON",
    "YOU_TAKE_PICTURE_OF_SWALLOW",
    "USING_CAMERA_UNDERWATER",
    "NO_HIT_IF_CANNOT_SEE_SPOT",
    "YOU_CANNOT_REACH_SPOT_FROM_HERE",
    "NOT_ENOUGH_ROOM_TO_USE",
    "YOU_MUST_REMOVE_O_TO_GREASE_O",
    "YOU_MUST_REMOVE_O_AND_O_TO_GREASE_O",
    "MIRROR_STARES_BACK",
    "YOU_STIFFEN_MOMENTARILY_UNDER_YOUR_GAZE",
    "HUH_NO_LOOK_LIKE_YOU",
    "YOU_HAVE_NO_REFLECTION",
    "YOU_LOOK_UNDERNOURISHED",
    "YOU_LOOK_PEAKED",
    "YOU_LOOK_COLOR",
    "WHISTLE_MAGIC",
    "WHISTLE_SHRILL",
    "WHISTLE_HIGH",
    "FOUND_SECRET_DOOR",
    "FOUND_SECRET_PASSAGE",
    "NO_ELBOW_ROOM",
    "FAILED_POLYMORPH",
    "WELDS_TO_YOUR_HAND",
    "YOU_DISRUPT",
    "A_HUGE_HOLE_OPENS_UP",
    "MONSTER_TURNS_INVISIBLE",
    "ENGULFER_OPENS_ITS_MOUTH",
    "MONSTER_LOOKS_BETTER",
    "MONSTER_LOOKS_MUCH_BETTER",
    "GOLEM_TURNS_TO_FLESH",
    "GOLEM_LOOKS_FLESHY",
    "MONSTER_LOOKS_WEAKER",
    "MONSTER_IS_NOT_CARRYING_ANYTHING",
    "DRAWN_INTO_FORMER_BODY",
    "A_MONSTER_SUDDENLY_APPEARS",
    "POLYPILE_CREATES_GOLEM",
    "SHOP_KEEPER_GETS_ANGRY",
    "SHOP_KEEPER_IS_FURIOUS",
    "YOU_NEED_HANDS_TO_WRITE",
    "IT_SLIPS_FROM_YOUR_FINGERS",
    "YOU_DONT_KNOW_THAT_IS_BLANK",
    "THAT_IS_NOT_BLANK",
    "CANT_WRITE_THAT_ITS_OBSCENE",
    "CANT_WRITE_BOOK_OF_THE_DEAD",
    "CANT_WRITE_WHAT_YOU_DONT_KNOW",
    "MARKER_TOO_DRY",
    "MARKER_DRIES_OUT",
    "SPELLBOOK_IS_UNFINISHED",
    "SCROLL_IS_NOW_USELESS",
    "YOU_DISTRUPT_ENGULFER",
    "VISION_QUICKLY_CLEARS",
    "THATS_ENOUGH_TRIES",
    "OBJECT_STOPS_GLOWING",
    "OBJECT_GLOWS_BRILLIANTLY",
    "MONSTER_HIDING_UNDER_OBJECT",
    "YOUR_OBJECT_IS_NO_LONGER_POISONED",
    "STEED_STOPS_GALLOPING",
    "YOUR_ATTACK_PASSES_HARMLESSLY_THROUGH_MONSTER",
    "YOUR_WEAPON_PASSES_HARMLESSLY_THROUGH_MONSTER",
    "YOU_JOUST_IT",
    "YOUR_LANCE_SHATTERS",
    "M_STAGGERS_FROM_YOUR_POWERFUL_STRIKE",
    "M_DIVIDES_AS_YOU_HIT_IT",
    "YOU_HIT_M",
    "YOUR_SILVER_RING_SEARS_M_FLESH",
    "IT_IS_SEARED",
    "ITS_FLESH_IS_SEARED",
    "THE_SILVER_SEARS_M",
    "THE_SILVER_SEARS_M_FLESH",
    "THE_POISON_DOESNT_SEEM_TO_AFFECT_M",
    "THE_POISON_WAS_DEADLY",
    "M_APPEARS_CONFUSED",
    "THE_GREASE_WEARS_OFF",
    "YOU_SLIP_OFF_M_GREASED_O",
    "YOU_SLIP_OFF_M_SLIPPERY_O",
    "YOU_GRAB_BUT_CANNOT_HOLD_ONTO_M_GREASED_O",
    "YOU_GRAB_BUT_CANNOT_HOLD_ONTO_M_SLIPPERY_O",
    "YOU_CHARM_HER_AND_STEAL_EVERYTHING",
    "YOU_SEDUCE_M_AND_HE_GETS_NAKED",
    "HE_FINISHES_TAKING_OFF_HIS_SUIT",
    "M_STAGGERS_FOR_A_MOMENT",
    "M_IS_ON_FIRE",
    "M_BURNS_COMPLETELY",
    "THE_FIRE_DOESNT_HEAT_M",
    "M_IS_COVERED_IN_FROST",
    "THE_FROST_DOESNT_CHILL_M",
    "M_IS_ZAPPED",
    "THE_ZAP_DOESNT_SHOCK_M",
    "M_IS_BLINDED",
    "WRITING_VANISHES_FROM_M_HEAD",
    "M_SUDDENLY_SEEMS_WEAKER",
    "M_FALLS_TO_PIECES",
    "M_DIES",
    "M_DOESNT_SEEM_HARMED",
    "M_HELMET_BLOCKS_YOUR_ATTACK_TO_HIS_HEAD",
    "YOU_EAT_M_BRAIN",
    "M_DOESN_NOTICE",
    "YOU_SWING_YOURSELF_AROUND_M",
};

/*

MSG_NO_ELBOW_ROOM:  "You don't have enough elbow-room to maneuver.";

                        pline("%s cannot defend itself.", Adjmonnam(mon,"blind"));
                        message_monster(MSG_A_BLIND_M_CANNOT_DEFEND_ITSELF, mon);

                pline("%s is blinded by the flash!", Monnam(mtmp));
                message_monster(MSG_M_IS_BLINDED_BY_THE_FLASH, mtmp);

                    pline("%s looks confused.", Monnam(mdef));
                    message_monster(MSG_M_LOOKS_CONFUSED, mdef);

                    pline("%s slows down.", Monnam(mdef));
                    message_monster(MSG_M_SLOWS_DOWN, mdef);

                    pline("%s is put to sleep by you!", Monnam(mdef));
                    message_monster(MSG_M_IS_PUT_TO_SLEEP_BY_YOU, mdef);

                    pline("%s is frozen by you!", Monnam(mdef));
                    message_monster(MSG_M_IS_FROZEN_BY_YOU, mdef);

                        pline("%s is being crushed.", Monnam(mdef));
                        message_monster(MSG_M_IS_BEING_CRUSHED, mdef);

                            pline("%s is burning to a crisp!",Monnam(mdef));
                            message_monster(MSG_M_IS_BURNING_TO_A_CRISP, mdef);

                            pline("%s seems mildly hot.", Monnam(mdef));
                            message_monster(MSG_M_SEEMS_MIDLY_HOT, mdef);

                            pline("%s is freezing to death!",Monnam(mdef));
                            message_monster(MSG_M_IS_FREEZING_TO_DEATH, mdef);

                            pline("%s seems mildly chilly.", Monnam(mdef));
                            message_monster(MSG_M_SEEMS_MILDLY_CHILLY, mdef);

                            pline("%s seems unhurt.", Monnam(mdef));
                            message_monster(MSG_M_SEEMS_UNHURT, mdef);

                            pline("%s can't see in there!", Monnam(mdef));
                            message_monster(MSG_M_CANT_SEE_IN_THERE, mdef);

                    pline("%s is covered with your goo!", Monnam(mdef));
                    message_monster(MSG_M_IS_COVERED_WITH_YOUR_GOO, mdef);

                        pline("%s is pummeled with your debris!", Monnam(mdef));
                        message_monster(MSG_M_IS_PUMMELED_WITH_YOUR_DEBRIS, mdef);

                            pline("%s seems unharmed.", Monnam(mdef));
                            message_monster(MSG_M_SEEMS_UNHARMED, mdef);

                        pline("%s is laden with your moisture.", Monnam(mdef));
                        message_monster(MSG_M_IS_LADEN_WITH_YOUR_MOISTURE, mdef);

                pline("%s gets blasted!", Monnam(mdef));
                message_monster(MSG_M_GETS_BLASTED, mdef);

                pline("%s is affected by your flash of light!", Monnam(mdef));
                message_monster(MSG_M_IS_AFFECTED_BY_YOUR_FLASH_OF_LIGHT, mdef);

                pline("%s is blinded by your flash of light!", Monnam(mdef));
                message_monster(MSG_M_IS_BLINDED_BY_YOUR_FLASH_OF_LIGHT, mdef);

            pline_The("flash awakens %s.", mon_nam(mtmp));
            message_monster(MSG_THE_FLAG_AWAKENS_M, mtmp);

                    You("are frozen by %s!", mon_nam(mon));
                    message_monster(MSG_YOU_ARE_FROZEN_BY_M, mon);

                            You("are frozen by %s gaze!", s_suffix(mon_nam(mon)));
                            message_monster(MSG_YOU_ARE_FROZEN_BY_M_GAZE, mon);

                            You("momentarily stiffen under %s gaze!", s_suffix(mon_nam(mon)));
                            message_monster(MSG_YOU_MOMENTARILY_STIFFEN_UNDER_M_GAZE, mon);

                    You("are splashed by %s acid!", s_suffix(mon_nam(mon)));
                    message_monster(MSG_YOU_ARE_SPLASHED_BY_M_ACID, mon);

        You("pretend to be friendly to %s.", mon_nam(mdef));
        message_monster(MSG_YOU_PRETEND_TO_BE_FRIENDLY_TO_M, mdef);

            You("bite into %s.", mon_nam(mdef));
            message_monster(MSG_YOU_BITE_INTO_M, mdef);

                pline("Obviously, you didn't like %s taste.", s_suffix(mon_nam(mdef)));
                message_monster(MSG_YOU_DID_NOT_LIKE_M_TASTE, mdef);

                        pline_The("air around %s crackles with electricity.", mon_nam(mdef));
                        message_monster(MSG_THE_AIR_AROUND_M_CRACKLES_WITH_ELECTRICITY, mdef);

                        pline("It seems harmless to %s.", mon_nam(mdef));
                        message_monster(MSG_IT_SEEMS_HARMLESS_TO_M, mdef);

                            You("digest %s.", mon_nam(mdef));
                            message_monster(MSG_YOU_DIGEST_M, mdef);

    You("engulf %s!", mon_nam(mdef));
    message_monster(MSG_YOU_ENGULF_M, mdef);

                    pline_The("blast doesn't seem to affect %s.", mon_nam(mdef));
                    message_monster(MSG_THE_BLAST_DOESNT_SEEM_TO_AFFECT_M, mdef);

                You("turn %s into slime.", mon_nam(mdef));
                message_monster(MSG_YOU_TURN_M_INTO_SLIME, mdef);

                        You("brush against %s %s.", s_suffix(mon_nam(mdef)), mbodypart(mdef, LEG));
                        message_monster(MSG_YOU_BRUSH_AGAINST_M_LEG, mdef);

                        You("drown %s...", mon_nam(mdef));
                        message_monster(MSG_YOU_DROWN_M, mdef);

                        Your("attempt to surround %s is harmless.", mon_nam(mon));
                        message_monster(MSG_YOUR_ATTEMPT_TO_SURROUND_M_IS_HARMLESS, mon);

                        You("grab %s!", mon_nam(mon));
                        message_monster(MSG_YOU_GRAB_M, mon);

                    Your("hug passes harmlessly through %s.", mon_nam(mon));
                    message_monster(MSG_YOUR_HUG_PASSES_HARMLESSLY_THROUGH_M, mon);

                        You("hit %s.", mon_nam(mon));
                        message_monster(MSG_YOU_HIT_M, mon);

                        Your("tentacles suck %s.", mon_nam(mon));
                        message_monster(MSG_YOUR_TENTACLES_SUCK_M, mon);

                        You("touch %s.", mon_nam(mon));
                        message_monster(MSG_YOU_TOUCH_M, mon);

                        You("butt %s.", mon_nam(mon));
                        message_monster(MSG_YOU_BUTT_M, mon);

                        You("sting %s.", mon_nam(mon));
                        message_monster(MSG_YOU_STING_M, mon);

                        You("bite %s.", mon_nam(mon));
                        message_monster(MSG_YOU_BITE_M, mon);

                        You("kick %s.", mon_nam(mon));
                        message_monster(MSG_YOU_KICK_M, mon);

                        Your("attack passes harmlessly through %s.", mon_nam(mon));
                        message_monster(MSG_YOUR_ATTACK_PASSES_HARMLESSLY_THROUGH_M, mon);

                pline("%s!", Tobjnam(obj, "explode"));
                message_object(MSG_O_EXPLODES, obj);

        pline("%s stirs.", Monnam(mtmp));
        message_monster(MSG_M_STIRS, mtmp);

            pline("%s awakens.", Monnam(mtmp));
            message_monster(MSG_M_AWAKENS, mtmp);

        pline("%s isn't trapped.", Monnam(mtmp));
        message_monster(MSG_M_IS_NOT_TRAPPED, mtmp);

            pline("%s thinks it was nice of you to try.", Monnam(mtmp));
            message_monster(MSG_M_THINKS_IT_WAS_NICE_OF_YOU_TO_TRY, mtmp);

            pline("%s is grateful.", Monnam(mtmp));
            message_monster(MSG_M_IS_GRATEFUL, mtmp);

                        pline("%s remains entangled.", Monnam(mtmp));
                        message_monster(MSG_M_REMAINS_ENTANGLED, mtmp);

    You("pull %s out of the pit.", mon_nam(mtmp));
    message_monster(MSG_YOU_PULL_M_OUT_OF_THE_PIT, mtmp);

    You("reach out your %s and grab %s.", makeplural(body_part(ARM)), mon_nam(mtmp));
    message_monster(MSG_YOU_REACH_OUT_YOUR_ARM_AND_GRAB_M);

        You("try to grab %s, but cannot get a firm grasp.", mon_nam(mtmp));
        message_monster(MSG_YOU_TRY_TO_GRAB_M_BUT_CANNOT_GET_A_GRASP, mtmp);

            You("aren't skilled enough to reach from %s.", mon_nam(u.usteed));
            message_monster(MSG_YOU_ARE_NOT_SKILLED_ENOUGH_TO_REACH_FROM_M, u.usteed);

        pline("%s is in the way.", Monnam(mtmp));
        message_monster(MSG_M_IS_IN_THE_wAY, mtmp);

        pline("You'll have to let go of %s first.", mon_nam(u.ustuck));
        message_monster(MSG_YOU_MUST_LET_GO_OF_M_FIRST, u.ustuck);

                pline("Startled, %s can no longer hold you!", mon_nam(u.ustuck));
                message_monster(MSG_M_CAN_NO_LONGER_HOLD_YOU, u.ustuck);

                You("aren't able to maintain your hold on %s.", mon_nam(u.ustuck));
                message_monster(MSG_YOU_ARE_NOT_ABLE_TO_HOLD, u.ustuck);

            You("cannot stay on %s.", mon_nam(u.usteed));
            message_monster(MSG_YOU_CANNOT_STAY_ON_M, u.usteed);

            pline("%s magically floats up!", Monnam(u.usteed));
            message_monster(MSG_M_MAGICALLY_FLOATS_UP, u.usteed);

        pline("%s turns to stone.", Monnam(mon));
        message_monster(MSG_M_TURNS_TO_STONE, mon);

                        pline("A trigger appears in a pile of soil below %s.", mon_nam(mtmp));
                        message_monster(MSG_TRIGGER_APPEARS_IN_SOIL_BELOW_M, mtmp);

                            pline("%s seems to be yanked down!", Monnam(mtmp));
                            message_monster(MSG_M_SEEMS_TO_BE_YANKED_DOWN, mtmp);

                        pline("%s is uninjured.", Monnam(mtmp));
                        message_monster(MSG_M_IS_UNINJURED, mtmp);

                                You("see a %s erupt from the %s!", tower_of_flame, surface(mtmp->mx, mtmp->my));
                                message_monster_string(MSG_YOU_SEE_TOWER_OF_FLAME_ERUPT_FROM_S,
                                    surface(mtmp->mx, mtmp->my));

                                pline("A %s erupts from the %s under %s!", tower_of_flame, surface(mtmp->mx, mtmp->my), mon_nam(mtmp));
                                message_monster_string(MSG_TOWER_OF_FLAME_ERUPTS_FROM_S_UNDER_M,
                                        mtmp, surface(mtmp->mx, mtmp->my));

                        pline("May %s rust in peace.", mon_nam(mtmp));
                        message_monste(MSG_MAY_M_RUST_IN_PEACE, mtmp);

                        pline("%s falls to pieces!", Monnam(mtmp));
                        message_monster(MSG_M_FALLS_TO_PIECES, mtmp);

                            pline("%s %s!", A_gush_of_water_hits, mon_nam(mtmp));
                            message_monster(MSG_GUSH_OF_WATER_HITS_M, mtmp);

                            pline("%s %s's right %s!", A_gush_of_water_hits, mon_nam(mtmp), mbodypart(mtmp, ARM));
                            message_monster(MSG_GUSH_OF_WATER_HITS_M_RIGHT_ARM, mtmp);

                            pline("%s %s's left %s!", A_gush_of_water_hits, mon_nam(mtmp), mbodypart(mtmp, ARM));
                            message_monster(MSG_GUSH_OF_WATER_HITS_M_LEFT_ARM, mtmp);

                            pline("%s %s on the %s!", A_gush_of_water_hits, mon_nam(mtmp), mbodypart(mtmp, HEAD));
                            message_monster(MSG_GUSH_OF_WATER_HITS_M_HEAD, mtmp);

                        pline("%s is caught in %s bear trap!", Monnam(mtmp), a_your[trap->madeby_u]);
                        unsigned char made_by_you_flag = trap->madeby_u ? MSG_FLAG_MADE_BY_YOU : 0;
                        message_monster_flag(MSG_M_CAUGHT_IN_BEAR_TRAP, made_by_you_flag);

                    pline("A board beneath %s squeaks loudly.", mon_nam(mtmp));
                    message_monster(MSG_BOARD_BENEATH_M_SQEAKS_LOUDLY, mtmp);

                        pline("A trap door above %s opens, but nothing falls out!", mon_nam(mtmp));
                        message_monster(MSG_TRAP_DOOR_ABOVE_M_OPENS_NOTHING_FALLS_OUT, mtmp);

                        pline("%s triggers a trap but nothing happens.", Monnam(mtmp));
                        message_monster(MSG_M_TRIGGERS_TRAP_BUT_NOTHING_HAPPENS, mtmp);

                    pline("%s munches on some spikes!", Monnam(mtmp));
                    message_monster(MSG_M_MUNCHES_ON_SOME_SPIKES, mtmp);

                    pline("%s eats a bear trap!", Monnam(mtmp));
                    message_monster(MSG_M_EATS_A_BEAR_TRAP, mtmp);

                        pline("%s pulls free...", Monnam(mtmp));
                        message_monster(MSG_M_PULLS_FREE, mtmp);

                    pline("%s snatches the boulder.", Monnam(mtmp));
                    message_monster(MSG_M_SNATCHES_THE_BOULDER, mtmp);

                        You("have to adjust yourself in the saddle on %s.", x_monnam(mtmp, mtmp->mnamelth ? ARTICLE_NONE : ARTICLE_A, (char *)0, SUPPRESS_SADDLE, false));
                        message_monster(MSG_YOU_HAVE_TO_ADJUST_YOURSELF_IN_THE_SADDLE_ON_M, mtmp);

                    pline("%s suddenly falls asleep!", Monnam(mtmp));
                    message_monster(MSG_M_SUDDENLY_FALLS_ASLEEP, mtmp);

            pline("%s is hit by %s!", Monnam(mon), doname(obj));
            message_monster_object(MSG_M_IS_HIT_BY_O, mon, obj);

            pline("%s is almost hit by %s!", Monnam(mon), doname(obj));
            message_monster_object(MSG_M_IS_ALMOST_HIT_BY_O, mon, obj);

                const char *predicament = "on a set of sharp iron spikes";
                    message_const(MSG_YOU_LAND_ON_SHARP_IRON_SPIKES);
                if (u.usteed) {
                    message_monster(MSG_M_LANDS_ON_SHARP_IRON_SPIKES, u.usteed);
                } else {
                    You("land %s!", predicament);
                    pline("%s lands %s!", upstart(x_monnam(u.usteed, u.usteed->mnamelth ? ARTICLE_NONE : ARTICLE_THE, "poor", SUPPRESS_SADDLE, false)), predicament);
                    message_monster(MSG_M_LANDS_ON_SHARP_IRON_SPIKES, u.usteed);

                pline("%s bear trap closes on your %s!", A_Your[trap->madeby_u], body_part(FOOT));
                message_flag(MSG_BEAR_TRAP_CLOSES_ON_YOUR_FOOT, made_by_you_flag);

                pline("%s bear trap closes on %s %s!",
                        A_Your[trap->madeby_u], s_suffix(mon_nam(u.usteed)), mbodypart(u.usteed, FOOT));
                unsigned char made_by_you_flag = trap->madeby_u ? MSG_FLAG_MADE_BY_YOU : 0;
                message_monster_flag(MSG_BEAR_TRAP_CLOSES_ON_M_FOOT, u.usteed, made_by_you_flag);

                        Your("%s does not protect you.", xname(uarmh));
                        message_object(MSG_YOU_RO_DOES_NOT_PROTECT_YOU, uarmh);

                pline("A trap door in %s opens and %s falls on your %s!",
                        the(ceiling(u.ux, u.uy)), an(xname(otmp)), body_part(HEAD));
                message_object_string(MSG_TRAP_DOOR_IN_CEILING_OPENS_AND_O_FALLS_ON_YOUR_HEAD,
                        otmp, ceiling(u.ux, u.uy));

        You("find %s posing as a statue.", canspotmon(mon) ? a_monnam(mon) : something);
        if (canspotmon(mon)) {
            message_monster(MSG_YOU_FIND_M_POSING_AS_STATUE, mon);
        } else {
            message_const(MSG_YOU_FIND_SOMETHING_POSING_AS_STATUE);
        }

            Your("%s %s", aobjnam(otmp, "are"), txt);
            message_object(MSG_YOUR_O_ARE_PROTECTED_BY_GREASE, otmp);

            pline("%s's %s %s", Monnam(victim), aobjnam(otmp, "are"), txt);
            message_monster_object(MSG_M_O_ARE_PROTECTED_BY_GREASE, victim, otmp);

            pline("%s lantern is getting dim.", s_suffix(Monnam(obj->ocarry)));
            message_monster(MSG_M_LANTERN_GETTING_DIM, obj->ocarry);

        pline("%s %s%s on the ice.",
                u.usteed ? upstart(x_monnam(u.usteed, u.usteed->mnamelth ? ARTICLE_NONE : ARTICLE_THE, (char *)0, SUPPRESS_SADDLE, false)) : "You", rn2(2) ? "slip" : "slide", on_foot ? "" : "s");
        const char *slip = rn2(2) ? "slip" : "slide";
        if (u.usteed) {
            message_monster_string(MSG_M_SLIPS_ON_THE_ICE, u.usteed, slip);
        } else {
            message_string(MSG_YOU_SLIP_ON_THE_ICE, slip);

                You("are no longer inside %s!", mon_nam(mtmp));
                message_monster(MSG_YOU_ARE_NO_LONGER_INSIDE_M, mtmp);

                pline("%s resists your magic!", Monnam(mtmp));
                message_monster(MSG_M_RESISTS_YOUR_MAGIC, mtmp);

                pline("Suddenly, %s disappears out of sight.", mon_nam(mtmp));
                message_monster(MSG_SUDDENDLY_M_DISAPPEARS_OUT_OF_SIGHT, mtmp);

                        pline("%s shudders for a moment.", Monnam(mtmp));
                        message_monster(MSG_M_SHUDDERS_FOR_A_MOMENT, mtmp);

                        pline("%s seems very disoriented for a moment.", Monnam(mtmp));
                        message_monster(MSG_M_SEEMS_DISORIENTED_FOR_A_MOMENT, mtmp);

                        pline("%s seems to shimmer for a moment.", Monnam(mtmp));
                        message_monster(MSG_M_SEEMS_TO_SHIMMER_FOR_A_MOMENT, mtmp);

                        pline("%s avoids the %s.", Monnam(mtmp), (tt == HOLE) ? "hole" : "trap");
                        message_monster_string(MSG_M_AVOIDS_THE_TRAP, mtmp,
                                (tt == HOLE) ? "hole" : "trap");

                    pline("A mysterious force prevents %s from teleporting!", mon_nam(mon));
                    message_monster(MSG_MYSTERIOUS_FORCE_PREVENTS_M_FROM_TELEPORTING, mon);

                    pline("%s is pulled into the lava!", Monnam(mtmp));
                    message_monster(MSG_M_IS_PULLED_INTO_THE_LAVA, mtmp);

                        pline("%s falls into the %s!", Monnam(mtmp), surface(u.ux, u.uy));
                        message_monster_string(MSG_M_FALLS_INTO_THE_S, mtmp, surface(u.ux, u.uy));

                You("dismount %s.", mon_nam(mtmp));
                message_monster(MSG_YOU_DISMOUNT_M, mtmp);

            You("can no longer ride %s.", mon_nam(u.usteed));
            message_monster(MSG_YOU_CAN_NO_LONGER_RIDE_M, u.usteed);

            You("%s off of %s!", verb, mon_nam(mtmp));
            message_monster_string(MSG_YOU_FALL_OFF_OF_M, mtmp, verb);

    pline("%s gallops!", Monnam(u.usteed));
    message_monster(MSG_M_GALLOPS, u.usteed);

        You("mount %s.", mon_nam(mtmp));
        message_monster(MSG_YOU_MOUNT_M, mtmp);

            pline("%s magically floats up!", Monnam(mtmp));
            message_monster(MSG_M_MAGICALLY_FLOATS_UP, mtmp);

        You("slip while trying to get on %s.", mon_nam(mtmp));
        message_monster(MSG_YOU_SLIP_WHILE_TRYING_TO_GET_ON_M, mtmp);

            pline("%s slips away from you.", Monnam(mtmp));
            message_monster(MSG_M_SLIPS_AWAY_FROM_YOU, mtmp);

        Your("%s armor is too stiff to be able to mount %s.",
                uarm->oeroded ? "rusty" : "corroded", mon_nam(mtmp));
        message_monster_string(MSG_YOUR_RUSTY_ARMOR_TOO_STIFF_TO_MOUNT_M, mtmp,
            uarm->oeroded ? "rusty" : "corroded");

        You("cannot reach %s.", mon_nam(mtmp));
        message_monster(MSG_YOU_CANNOT_REACH_M, mtmp);

        pline("%s resists%s!", Monnam(mtmp),
                mtmp->mleashed ? " and its leash comes off" : "");
        unsigned char leash_flag = mtmp->mleashed ? MSG_FLAG_LEASH_OFF : 0;
        message_monster_flag(MSG_M_RESISTS, mtmp, leash_flag);

        You_cant("mount %s while %s's trapped in %s.",
                mon_nam(mtmp), mhe(mtmp),
                an(defsyms[trap_to_defsym(t->ttyp)].explanation));
        message_monster_string(MSG_YOU_CANT_MOUNT_M_WHILE_HES_TRAPPED_IN_S, mtmp,
                defsyms[trap_to_defsym(t->ttyp)].explanation);

        pline("%s is not saddled.", Monnam(mtmp));
        message_monster(MSG_M_IS_NOT_SADDLED, mtmp);

        You("are already riding %s.", mon_nam(u.usteed));
        message_monster(MSG_YOU_ARE_ALREADY_RIDING_M, u.usteed);

        pline("%s resists!", Monnam(mtmp));
        message_monster(MSG_M_RESISTS, mtmp);

        You("put the saddle on %s.", mon_nam(mtmp));
        message_monster(MSG_YOU_PUT_SADDLE_ON_M, mtmp);

        pline("I think %s would mind.", mon_nam(mtmp));
        message_monster(MSG_I_THINK_M_WOULD_MIND, mtmp);

        You("touch %s.", mon_nam(mtmp));
        message_monster(MSG_YOU_TOUCH_M, mtmp);

        pline("%s doesn't need another one.", Monnam(mtmp));
        message_monster(MSG_M_DOES_NOT_NEED_ANOTHER_ONE, mtmp);

    You("aren't skilled enough to reach from %s.", y_monnam(u.usteed));
    message_monster(MSG_YOU_ARE_NOT_SKILLED_ENOUGH_TO_REACH_FROM_M, u.usteed);

                pline("%s out.", Tobjnam(otmp, "go"));
                message_object(MSG_O_GO_OUT, otmp);

            pline("%s drops %ld gold piece%s.", Monnam(mtmp), g, plur(g));
            message_monster_int(MSG_M_DROPS_X_GOLD_PIECES, mtmp, g);

        pline("%s drops %s.", Monnam(mon), distant_name(obj, doname));
        message_monster_object(MSG_M_DROPS_O, mon, obj);

        pline("%s stole %s!", Monnam(mtmp), doname(otmp));
        message_monster_object(MSG_M_STOLE_O, mtmp, otmp);

    pline("%s stole %s.", named ? "She" : Monnam(mtmp), doname(otmp));
    message_monster_object(MSG_M_STOLE_O, mtmp, otmp)

            pline("%s tries to %s your %s but gives up.",
                    Monnam(mtmp), how[rn2(SIZE(how))],
                    (otmp->owornmask & W_ARMOR) ? equipname(otmp) :
                    cxname(otmp));
            message_monster_object_string(MSG_M_TRIES_TO_STEAL_YOUR_O_BUT_GIVES_UP, mtmp, otmp, how);

            pline("%s tries to rob you, but there is nothing to steal!", Monnam(mtmp));
            message_monster(MSG_M_TRIES_TO_ROB_BUT_YOURE_NAKED, mtmp);

                    pline("%s steals %s!", Monnam(mtmp), doname(otmp));
                    message_monster_object(MSG_M_STEALS_O, mtmp, otmp);

        pline("%s quickly snatches some gold from between your %s!",
                Monnam(mtmp), makeplural(body_part(FOOT)));
        message_monster(MSG_M_QUICKLY_SNATCHES_GOLD_FROM_BEETWEEN_YOUR_LEGS, mtmp);

        pline("%s is eating noisily.", Monnam(mtmp));
        message_monster(MSG_M_IS_EAT_NOISILY, mtmp);

            pline("%s seems not to notice you.", Monnam(mtmp));
            message_monster(MSG_M_SEEMS_NOT_TO_NOTICE_YOU, mtmp);

                        pline("%s boasts about %s gem collection.", Monnam(mtmp), mhis(mtmp));
                        message_monster(MSG_M_BOASTS_ABOUT_GEM_COLLECTION, mtmp);

            pline("%s rattles noisily.", Monnam(mtmp));
            message_monster(MSG_M_RATTLES_NOISILY, mtmp);

                pline("%s throws back %s head and lets out a blood curdling %s!",
                        Monnam(mtmp), mhis(mtmp),
                        ptr == &mons[PM_HUMAN_WERERAT] ? "shriek" : "howl");
                message_monster_string(MSG_M_THROWS_BACK_HEAD_LETS_OUT_HOWL, mtmp,
                        ptr == &mons[PM_HUMAN_WERERAT] ? "shriek" : "howl");

        pline("%s %s.", Monnam(mtmp), vtense((char *)0, whimper_verb));
        message_monster_string(MSG_M_WHIMPERS, mtmp, whimper_verb);

        pline("%s %s!", Monnam(mtmp), vtense((char *)0, yelp_verb));
        message_monster_string(MSG_M_YELPS, mtmp, yelp_verb);

    if (Hallucination())
        growl_verb = h_sounds[rn2(SIZE(h_sounds))];
    else
        growl_verb = growl_sound(mtmp);

    pline("%s %s!", Monnam(mtmp), vtense((char *)0, growl_verb));
    message_monster_string(MSG_M_GROWLS, mtmp, growl_verb);

            pline("%s %s %s.",
                    s_suffix(upstart(y_monnam(u.usteed))),
                    aobjnam(otmp, "glow"),
                    hcolor(otmp->cursed ? NH_BLACK : "brown"));
            if (Hallucination()) {
                message_monster_object_int(MSG_M_O_GLOWS_COLOR, u.usteed, otmp, halluc_color_int());
            } else if (otmp->cursed) {
                message_monster_object(MSG_M_O_GLOWS_BLACK, u.usteed, otmp);
            } else {
                message_monster_object(MSG_M_O_GLOWS_BROWN, u.usteed, otmp);
            }

                pline("%s!", Tobjnam(otmp, "resist"));
                message_object(MSG_O_RESISTS, otmp);

        You("are already sitting on %s.", mon_nam(u.usteed));
        message_monster(MSG_YOU_ARE_ALEADY_SITTING_ON_M, u.usteed);

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

void message_const(enum MessageId id) {
    printf("message_const %s\n", message_id_str[id]);
}

void message_flag(enum MessageId id, unsigned char flags) {
    printf("message_flag %s\n", message_id_str[id]);
}

void message_monster(enum MessageId id, const struct monst *m) {
    printf("message_monster %s\n", message_id_str[id]);
}

void message_monster_force_visible(enum MessageId id, const struct monst *m) {
    printf("message_monster_force_visible %s\n", message_id_str[id]);
}

void message_object(enum MessageId id, const struct obj *o) {
    printf("message_object %s\n", message_id_str[id]);
}

void message_object2(enum MessageId id, const struct obj *o1, const struct obj *o2) {
    printf("message_object2 %s\n", message_id_str[id]);
}

void message_object3(enum MessageId id, const struct obj *o1, const struct obj *o2,
        const struct obj *o3)
{
    printf("message_object3 %s\n", message_id_str[id]);
}

void message_int(enum MessageId id, int i) {
    printf("message_int %s\n", message_id_str[id]);
}
void message_monster_string(enum MessageId id, const struct monst *m, const char *s) {
    printf("message_monster_string %s\n", message_id_str[id]);
}
void message_string(enum MessageId id, const char *s) {
    printf("message_string %s\n", message_id_str[id]);
}
void message_monster_object(enum MessageId id, const struct monst * mtmp, struct obj * obj) {
    printf("message_monster_object %s\n", message_id_str[id]);
}
void message_monster_object_int(enum MessageId id, const struct monst * mtmp, struct obj * obj, int i) {
    printf("message_monster_object_int %s\n", message_id_str[id]);
}
void message_monster_object_flag(enum MessageId id, const struct monst * m,
        struct obj * o, unsigned char c)
{
    printf("message_monster_object_flag %s\n", message_id_str[id]);
}
void message_monster_object_string(enum MessageId id, const struct monst * mtmp,
        struct obj * obj, const char *string)
{
    printf("message_monster_object_string %s\n", message_id_str[id]);
}
void message_object_string(enum MessageId id, const struct obj *o, const char * string) {
    printf("message_object_string %s\n", message_id_str[id]);
}

void message_monster_int_string(enum MessageId id, const struct monst *m, int i,
        const char *string)
{
    printf("message_monster_int_string %s\n", message_id_str[id]);
}
void message_monster_int(enum MessageId id, const struct monst *m, int i) {
    printf("message_monster_int %s\n", message_id_str[id]);
}
void message_monster_flag(enum MessageId id, const struct monst *m, unsigned char flags) {
    printf("message_monster_flag %s\n", message_id_str[id]);
}
void message_monster_int_flag(enum MessageId id, const struct monst *m, int i, unsigned char flags) {
    printf("message_monster_int_flag %s\n", message_id_str[id]);
}

void audio_message_monster(enum MessageId id, const struct monst *m) {
    printf("audio_message_monster %s\n", message_id_str[id]);
}
void audio_message_object_flag(enum MessageId id, const struct obj *o, unsigned char flags) {
    printf("audio_message_object_flag %s\n", message_id_str[id]);
}
void audio_message_monster_string(enum MessageId id, const struct monst *m, const char *string) {
    printf("audio_message_monster_string %s\n", message_id_str[id]);
}

void getlin(const char *query, char *out_buf) {
    fprintf(stderr, "TODO: getlin(\"%s\")", query);
    nh_strlcpy(out_buf, "LOL", 32);
};

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
MSG_M_STAGGERS_FOR_A_MOMENT, pline("%s %s for a moment.", Monnam(mdef), makeplural(stagger(mdef->data, "stagger")));
MSG_M_IS_ON_FIRE, pline("%s is %s!", Monnam(mdef), on_fire(mdef->data, mattk));
MSG_M_BURNS_COMPLETELY, pline("%s burns completely!", Monnam(mdef));
MSG_THE_FIRE_DOESNT_HEAT_M, pline_The("fire doesn't heat %s!", mon_nam(mdef));
MSG_M_IS_COVERED_IN_FROST, pline("%s is covered in frost!", Monnam(mdef));
MSG_THE_FROST_DOESNT_CHILL_M, pline_The("frost doesn't chill %s!", mon_nam(mdef));
MSG_M_IS_ZAPPED, pline("%s is zapped!", Monnam(mdef));
MSG_THE_ZAP_DOESNT_SHOCK_M, pline_The("zap doesn't shock %s!", mon_nam(mdef));
MSG_M_IS_BLINDED, pline("%s is blinded.", Monnam(mdef));
MSG_WRITING_VANISHES_FROM_M_HEAD, pline("Some writing vanishes from %s head!", s_suffix(mon_nam(mdef)));
MSG_M_SUDDENLY_SEEMS_WEAKER, pline("%s suddenly seems weaker!", Monnam(mdef));
MSG_M_FALLS_TO_PIECES, pline("%s falls to pieces!", Monnam(mdef));
MSG_M_DIES, pline("%s dies!", Monnam(mdef));
MSG_M_DOESNT_SEEM_HARMED, pline("%s doesn't seem harmed.", Monnam(mdef));
MSG_M_HELMET_BLOCKS_YOUR_ATTACK_TO_HIS_HEAD, pline("%s helmet blocks your attack to %s head.", s_suffix(Monnam(mdef)), mhis(mdef));
MSG_YOU_EAT_M_BRAIN, You("eat %s brain!", s_suffix(mon_nam(mdef)));
MSG_M_DOESN_NOTICE, pline("%s doesn't notice.", Monnam(mdef));
MSG_YOU_SWING_YOURSELF_AROUND_M, You("swing yourself around %s!", mon_nam(mdef));

*/
