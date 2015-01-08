#include "stdout_msg.h"

#include <stdint.h>
#include <stdio.h>

void stdout_msg_glyph(int x, int y, int glyph) {
    uint32_t id = OUT_MSG_GLYPH;
    uint32_t size = sizeof(int32_t) * 3;
    int32_t x32 = x;
    int32_t y32 = y;
    int32_t glyph32 = glyph;
    fwrite(&id, sizeof(uint32_t), 1, stdout);
    fwrite(&size, sizeof(uint32_t), 1, stdout);
    fwrite(&x32, sizeof(int32_t), 1, stdout);
    fwrite(&y32, sizeof(int32_t), 1, stdout);
    fwrite(&glyph32, sizeof(int32_t), 1, stdout);
    fflush(stdout);
}

void stdout_msg_const(enum StdoutMsgId id) {
    uint32_t id32 = id;
    uint32_t size = 0;
    fwrite(&id32, sizeof(uint32_t), 1, stdout);
    fwrite(&size, sizeof(uint32_t), 1, stdout);
    fflush(stdout);
}
