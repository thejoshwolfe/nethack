#include "stdout_msg.h"

#include <stdint.h>
#include <stdio.h>
#include <limits.h>

#include "dungeon_util.h"
#include "nethack.h"
#include "rm.h"
#include "display.h"
#include "pline.h"

typedef struct {
    DungeonFeature dungeon_feature;
    bool permanent_light;
    bool temporary_light;
    VisionTypes seen_how;
} Tile;

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

void output_everything(void) {
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
        }
    }
    fflush(stdout);
}
