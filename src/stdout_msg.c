#include "stdout_msg.h"

#include <stdint.h>
#include <stdio.h>

#include "nethack.h"
#include "rm.h"


typedef struct {
    DungeonFeature dungeon_feature;
    bool permanent_light; // from a scroll of light, etc.
    bool temporary_light; // from a lantern, etc.
    VisionTypes seen_how;
} Tile;

void output_everything(void) {
}
