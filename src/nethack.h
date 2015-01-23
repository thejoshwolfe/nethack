// TODO: all the enums will eventually need explicit values
typedef enum {
    BranchId_DUNGEONS_OF_DOOM,
    BranchId_THE_GNOMISH_MINES,
    BranchId_SOKOBAN,
    BranchId_THE_QUEST,
    BranchId_FORT_LUDIOS,
    BranchId_GEHENNOM,
    BranchId_VLADS_TOWER,
    BranchId_PLANE_OF_EARTH,
    BranchId_PLANE_OF_FIRE,
    BranchId_PLANE_OF_AIR,
    BranchId_PLANE_OF_WATER,
    BranchId_ASTRAL_PLANE,
} BranchId;

typedef enum {
    BucStatus_BLESSED,
    BucStatus_UNCURSED,
    BucStatus_CURSED,
    BucStatus_UNKNOWN,
} BucStatus;

typedef enum {
    VisionTypes_NORMAL_VISION = 0x1,
    VisionTypes_SEE_INVISIBLE = 0x2,
    VisionTypes_INFRAVISION = 0x4,
    VisionTypes_TELEPATHY = 0x8,
    VisionTypes_ASTRAL_VISION = 0x10,
    VisionTypes_DETECT_MONSTERS = 0x20,
    VisionTypes_WARNING = 0x40,
    VisionTypes_PARANOID_DELUSION = 0x80,
    VisionTypes_WARNED_OF_ORCS = 0x100,
    VisionTypes_WARNED_OF_DEMONS = 0x200,
} VisionTypes;

typedef enum {
    DungeonFeature_UNKNOWN,
    DungeonFeature_STONE_WALL,
    DungeonFeature_ROOM_WALL_LT,
    DungeonFeature_ROOM_WALL_TR,
    DungeonFeature_ROOM_WALL_RB,
    DungeonFeature_ROOM_WALL_LB,
    DungeonFeature_ROOM_WALL_LR,
    DungeonFeature_ROOM_WALL_TB,
    DungeonFeature_ROOM_WALL_LTR,
    DungeonFeature_ROOM_WALL_TRB,
    DungeonFeature_ROOM_WALL_LRB,
    DungeonFeature_ROOM_WALL_LTB,
    DungeonFeature_ROOM_WALL_LTRB,
    DungeonFeature_DOOR_CLOSED_H,
    DungeonFeature_DOOR_CLOSED_V,
    DungeonFeature_DOOR_OPEN_H,
    DungeonFeature_DOOR_OPEN_V,
    DungeonFeature_DOOR_BROKEN,
    DungeonFeature_DOORWAY,
    DungeonFeature_ROOM_FLOOR,
    DungeonFeature_CORRIDOR,
    DungeonFeature_STAIRS_UP,
    DungeonFeature_STAIRS_DOWN,
    DungeonFeature_LADDER_UP,
    DungeonFeature_LADDER_DOWN,
    DungeonFeature_ALTER,
    DungeonFeature_GRAVE,
    DungeonFeature_THRONE,
    DungeonFeature_SINK,
    DungeonFeature_FOUNTAIN,
    DungeonFeature_POOL,
    DungeonFeature_MOAT,
    DungeonFeature_ICE,
    DungeonFeature_LAVA,
    DungeonFeature_IRON_BARS,
    DungeonFeature_TREE,
    DungeonFeature_DRAWBRIDGE_RAISED,
    DungeonFeature_DRAWBRIDGE_LOWERED,
    DungeonFeature_PORTCULLIS_OPEN,
    DungeonFeature_PLANE_OF_AIR,
    DungeonFeature_PLANE_OF_AIR_CLOUD,
    DungeonFeature_PLANE_OF_WATER,
    DungeonFeature_PLANE_OF_WATER_AIR_BUBBLE,
} DungeonFeature;

typedef enum {
    TrapType_PIT,
    // TODO...
} TrapType;

typedef enum {
    SpeciesType_GIANT_ANT,
    // TODO...
} SpeciesType;
typedef enum {
    ItemDescription_MAGENTA_POTION,
    // TODO...
} ItemDescription;

typedef enum {
    ItemType_POTION_OF_GAIN_LEVEL,
    // TODO...
} ItemType;
