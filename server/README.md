# Nethack client/server protocol

## Data model

The server assumes the client is keeping the following data model which can be updated by messages from the protocol.

```cs

// these enums are defined in nethack.h
extern enum BranchId;
extern enum DungeonFeature;
extern enum TrapType;
extern enum EngravingType;
extern enum SpeciesType;
extern enum VisionTypes;
extern enum ItemDescription;
extern enum ItemType;
extern enum BucStatus;

SIZE_X = 80;
SIZE_Y = 21;

struct Location {
  level_id as u256;
  coord as Coord;
}
// {-1, -1} means unknown or none.
struct Coord {
  x as int;
  y as int;
}

levels as dict {
  level_id as u256 => level as struct Level {
    depth as int; // aka Dlvl. determines difficulty of spawned monsters, etc.
    branch_id as BranchId; // DUNGEONS_OF_DOOM, SOKOBAN, etc.
    map as struct Map {
      tile as array[SIZE_X][SIZE_Y] of struct Tile {
        dungeon_feature as DungeonFeature; // FLOOR, VWALL, ALTER, etc.
        permanent_light as bool; // from a scroll of light, etc.
        temporary_light as bool; // from a lantern, etc.
        seen_how as VisionTypes;
      };
    };
    // normal stairs always lead to the same branch_id with depth +/- 1.
    // these fields will only be filled in after you traverse them for the first time;
    // this is to keep them indistinguishable from special_stairs.
    staris_up as Coord;
    stairs_down as Coord;
    // special stairs might be stairs, a ladder, or a magic portal.
    portal as Coord;
    // usually different from current branch_id (the elemental planes are each a different branch).
    // this is UNKNOWN until you go there (or if there isn't one).
    portal_branch_id as BranchId;
  },
};
traps as dict {
  trap_id as u256 => trap as struct Trap {
    trap_type as TrapType;
    location as Location;
  },
};
engravings as dict {
  engraving_id as u256 => engraving as struct Engraving {
    location as Location;
    engraving_type as EngravingType;
    text as String;
  },
};
monsters as dict {
  monster_id as u256 => monster as struct Monster {
    species_id as SpeciesType; // VAMPIRE_LORD, JUIBLEX, etc.
    custom_name as String;
    location as Location;
    seen_how as VisionTypes; // if none, then location is last-known location
    // this list is complete when you can see the monster well enough to see their equipment.
    // changes with "The gnome wields a bow.", etc.
    equipped_item_ids as list of u256;
  },
};
item_piles as dict {
  item_pile_id as u256 => item_pile as struct ItemPile {
    location as Location;
    seen_how as VisionTypes;
    // this will be accurate if seen_how is not blank; otherwise, it's your memory.
    // if you're looking closely here, this field is the same as item_ids[0].
    top_item_id as u256;
    // true means there's at least 1 more item under the top item.
    is_pile as bool;
    // this will be empty until you look closely at it.
    // after you walk away, it will be what you remember.
    item_ids as list of u256;
  },
};
items as dict {
  item_id as u256 => item as struct Item {
    description as ItemDescription; // MAGENTA_POTION, etc.
    count as int; // -1 means an unspecified amount more than 1.
    custom_name as String; // by naming an individual item.
    buc_status as BucStatus;
    metadata as int; // different meaning for different types of items.
  },
};
item_identities as dict {
  description as ItemDescription => item_type as ItemType,
};
item_group_names as dict {
  description as ItemDescription => item_grou_name as String,
};

time as int;

you as struct You {
  // player characters are two species, and they are both genocidable.
  role as SpeciesType; // MONK, CAVEMAN, etc.
  race as SpeciesType; // HUMAN, ELF, etc.
  location as Location;
  // TODO...
};
```
