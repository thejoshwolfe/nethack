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
    DungeonFeature_UNKNOWN,
} DungeonFeature;

typedef enum {
    TrapType_PIT,
} TrapType;

typedef enum {
    SpeciesType_GIANT_ANT,
} SpeciesType;

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
    ItemDescription_MAGENTA_POTION,
} ItemDescription;

typedef enum {
    ItemType_POTION_OF_GAIN_LEVEL,
} ItemType;

typedef enum {
    BucStatus_BLESSED,
    BucStatus_UNCURSED,
    BucStatus_CURSED,
    BucStatus_UNKNOWN,
} BucStatus;
