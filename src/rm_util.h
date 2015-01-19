#ifndef RM_UTIL_H
#define RM_UTIL_H

#include "rm.h"

/*
 * Avoid using the level types in inequalities:
 * these types are subject to change.
 * Instead, use one of the macros below.
 */
static bool IS_WALL(signed char typ) {
    return typ && typ <= DBWALL;
}
/* STONE <= (typ) <= DBWALL */
static bool IS_STWALL(signed char typ) {
    return typ <= DBWALL;
}
/* absolutely nonaccessible */
static bool IS_ROCK(signed char typ) {
    return typ < POOL;
}
static bool IS_DOOR(signed char typ) {
    return typ == DOOR;
}
static bool IS_TREE(signed char typ) {
    return typ == TREE || (level.flags.arboreal && typ == STONE);
}
/* good position */
static bool ACCESSIBLE(signed char typ) {
    return typ >= DOOR;
}
/* ROOM, STAIRS, furniture.. */
static bool IS_ROOM(signed char typ) {
    return typ >= ROOM;
}
static bool ZAP_POS(signed char typ) {
    return typ >= POOL;
}
static bool SPACE_POS(signed char typ) {
    return typ > DOOR;
}
static bool IS_POOL(signed char typ) {
    return typ >= POOL && typ <= DRAWBRIDGE_UP;
}
static bool IS_THRONE(signed char typ) {
    return typ == THRONE;
}
static bool IS_FOUNTAIN(signed char typ) {
    return typ == FOUNTAIN;
}
static bool IS_SINK(signed char typ) {
    return typ == SINK;
}
static bool IS_GRAVE(signed char typ) {
    return typ == GRAVE;
}
static bool IS_ALTAR(signed char typ) {
    return typ == ALTAR;
}
static bool IS_DRAWBRIDGE(signed char typ) {
    return typ == DRAWBRIDGE_UP || typ == DRAWBRIDGE_DOWN;
}
static bool IS_FURNITURE(signed char typ) {
    return typ >= STAIRS && typ <= ALTAR;
}
static bool IS_AIR(signed char typ) {
    return typ == AIR || typ == CLOUD;
}
static bool IS_SOFT(signed char typ) {
    return typ == AIR || typ == CLOUD || IS_POOL(typ);
}

#endif /* RM_UTIL_H */
