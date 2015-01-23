#include "stdout_msg.h"

#include <stdint.h>
#include <stdio.h>
#include <limits.h>

#include "engrave.h"
#include "util_list.h"
#include "dungeon_util.h"
#include "nethack.h"
#include "rm.h"
#include "display.h"
#include "pline.h"

typedef struct {
    uint32_t data[8];
} uint256_t;

typedef struct {
    DungeonFeature dungeon_feature;
    bool permanent_light;
    bool temporary_light;
    VisionTypes seen_how;
} Tile;

typedef struct {
    int32_t x;
    int32_t y;
} Coord;

typedef struct {
    uint256_t level_id;
    Coord coord;
} Location;

typedef struct {
    Location location;
    uint32_t trap_type;
} Trap;

typedef struct {
    Location location;
    uint32_t engraving_type;
    uint32_t text_length;
} Engraving;


static DungeonFeature get_apparent_wall_dungeon_feature(struct rm * wall) {
    // TODO
    return DungeonFeature_STONE_WALL;
}

static DungeonFeature get_apparent_dungeon_feature(struct rm * ptr) {
    switch (ptr->typ) {
        case ROOM:
            return DungeonFeature_ROOM_FLOOR;
        case CORR:
            return DungeonFeature_CORRIDOR;

        case SCORR:
        case STONE:
            if (level.flags.arboreal)
                return DungeonFeature_TREE;
            // fallthrough
        case HWALL:
        case VWALL:
        case TLCORNER:
        case TRCORNER:
        case BLCORNER:
        case BRCORNER:
        case CROSSWALL:
        case TUWALL:
        case TDWALL:
        case TLWALL:
        case TRWALL:
        case SDOOR:
            return get_apparent_wall_dungeon_feature(ptr);

        case DOOR:
            if (ptr->flags) {
                if (ptr->flags & D_BROKEN)
                    return DungeonFeature_DOOR_BROKEN;
                if (ptr->flags & D_ISOPEN)
                    return ptr->horizontal ? DungeonFeature_DOOR_OPEN_H : DungeonFeature_DOOR_OPEN_V;
                /* else is closed */
                return ptr->horizontal ? DungeonFeature_DOOR_CLOSED_H : DungeonFeature_DOOR_CLOSED_V;
            }
            return DungeonFeature_DOORWAY;
        case IRONBARS:
            return DungeonFeature_IRON_BARS;
        case TREE:
            return DungeonFeature_TREE;
        case POOL:
            return DungeonFeature_POOL;
        case MOAT:
            return DungeonFeature_MOAT;
        case STAIRS:
            return (ptr->flags & LA_DOWN) ? DungeonFeature_STAIRS_DOWN : DungeonFeature_STAIRS_UP;
        case LADDER:
            return (ptr->flags & LA_DOWN) ? DungeonFeature_LADDER_DOWN : DungeonFeature_LADDER_UP;
        case FOUNTAIN:
            return DungeonFeature_FOUNTAIN;
        case SINK:
            return DungeonFeature_SINK;
        case ALTAR:
            return DungeonFeature_ALTER;
        case GRAVE:
            return DungeonFeature_GRAVE;
        case THRONE:
            return DungeonFeature_THRONE;
        case LAVAPOOL:
            return DungeonFeature_LAVA;
        case ICE:
            return DungeonFeature_ICE;
        case AIR: {
            bool plane_of_water = false; // TODO
            return plane_of_water ? DungeonFeature_PLANE_OF_WATER_AIR_BUBBLE : DungeonFeature_PLANE_OF_AIR;
        }
        case CLOUD:
            return DungeonFeature_PLANE_OF_AIR_CLOUD;
        case WATER:
            return DungeonFeature_PLANE_OF_WATER;
        case DBWALL:
            return DungeonFeature_DRAWBRIDGE_RAISED;
        case DRAWBRIDGE_UP:
            switch (ptr->flags & DB_UNDER) {
                case DB_MOAT:
                    return DungeonFeature_MOAT;
                case DB_LAVA:
                    // TODO: really?
                    return DungeonFeature_LAVA;
                case DB_ICE:
                    return DungeonFeature_ICE;
                case DB_FLOOR:
                    return DungeonFeature_ROOM_FLOOR;
            }
            impossible("Strange db-under: %d", ptr->flags & DB_UNDER);
            return DungeonFeature_UNKNOWN;
        case DRAWBRIDGE_DOWN:
            return DungeonFeature_DRAWBRIDGE_LOWERED;
        default:
            impossible("back_to_glyph:  unknown level type [ = %d ]",ptr->typ);
            return DungeonFeature_UNKNOWN;
    }
}

static uint256_t current_level_id(void) {
    // uh... i think 256 bits might be a little too many.
    uint256_t level_id = { 0x6b592506, 0xe706b4d6, 0x36da5ab2, 0xb9deace7, 0xdc86f140, 0x39bb63, 0x60d5ce87, 0xd3b07c2f };
    return level_id;
}

static void output_map(void) {
    uint32_t id = OUT_MSG_MAP;
    uint32_t size = COLNO * ROWNO * sizeof(uint32_t);
    fwrite(&id, sizeof(uint32_t), 1, stdout);
    fwrite(&size, sizeof(uint32_t), 1, stdout);
    for (int y = 0; y < ROWNO; y++) {
        for (int x = 0; x < COLNO; x++) {
            struct rm * hrrm = &level.locations[x][y];
            uint32_t dungeon_feature = get_apparent_dungeon_feature(hrrm);
            Tile tile = { dungeon_feature, false, false, 0 };
            fwrite(&dungeon_feature, sizeof(uint32_t), 1, stdout);
            // TODO: write it for realz
        }
    }
    fflush(stdout);
}

static TrapType get_trap_type(int ttype) {
    return ttype - 1;
}

static void output_traps(void) {
    List * trap_list = List_new();
    for (struct trap * trap = ftrap; trap != NULL; trap = trap->ntrap) {
        // TODO: conditional logic here for it you know about it.
        List_add(trap_list, trap);
    }
    uint32_t id = OUT_MSG_TRAPS;
    uint32_t trap_count = trap_list->size;
    uint32_t msg_size = sizeof(trap_count) + trap_count * sizeof(Trap);
    fwrite(&id, sizeof(uint32_t), 1, stdout);
    fwrite(&msg_size, sizeof(uint32_t), 1, stdout);
    fwrite(&trap_count, sizeof(uint32_t), 1, stdout);
    for (size_t i = 0; i < trap_list->size; i++) {
        struct trap * trap = trap_list->items[i];
        Location location = { current_level_id(), { trap->tx, trap->ty } };
        TrapType trap_type = get_trap_type(trap->ttyp);
        Trap output_trap = { location, trap_type };
        fwrite(&output_trap, sizeof(Trap), 1, stdout);
    }
    fflush(stdout);
    List_delete(trap_list);
}

static EngravingType get_engraving_type(signed char engr_type) {
    return engr_type - 1;
}

static void output_engravings(void) {
    List * list = List_new();
    uint32_t string_data_size = 0;
    for (struct engr * engraving = head_engr; engraving != NULL; engraving = engraving->nxt_engr) {
        // TODO: conditional logic here for it you know about it.
        List_add(list, engraving);
        string_data_size += strlen(engraving->engr_txt);
    }
    uint32_t id = OUT_MSG_ENGRAVINGS;
    uint32_t count = list->size;
    uint32_t msg_size = sizeof(count) + count * sizeof(Engraving) + string_data_size;
    fwrite(&id, sizeof(uint32_t), 1, stdout);
    fwrite(&msg_size, sizeof(uint32_t), 1, stdout);
    fwrite(&count, sizeof(uint32_t), 1, stdout);
    for (size_t i = 0; i < list->size; i++) {
        struct engr * engraving = list->items[i];
        Location location = { current_level_id(), { engraving->engr_x, engraving->engr_y } };
        EngravingType engraving_type = get_engraving_type(engraving->engr_type);
        uint32_t text_length = strlen(engraving->engr_txt);
        Engraving output_trap = { location, engraving_type, text_length };
        fwrite(&output_trap, sizeof(Engraving), 1, stdout);
        fwrite(engraving->engr_txt, sizeof(char), text_length, stdout);
    }
    fflush(stdout);
    List_delete(list);
}

static void output_items(void) {
}
static void output_item_piles(void) {
}
static void output_item_identities(void) {
}
static void output_item_group_names(void) {
}
static void output_monsters(void) {
}

void output_everything(void) {
    output_map();
    output_traps();
    output_engravings();
    output_items();
    output_item_piles();
    output_item_identities();
    output_item_group_names();
    output_monsters();
}
