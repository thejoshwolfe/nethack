#ifndef STDOUT_MSG_H
#define STDOUT_MSG_H

enum StdoutMsgId {
    OUT_MSG_GLYPH,
    OUT_MSG_SET_ALL_ROCK,
};

void stdout_msg_glyph(int x, int y, int glyph);
void stdout_msg_const(enum StdoutMsgId id);

#endif // STDOUT_MSG_H
