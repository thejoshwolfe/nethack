/* See LICENSE in the root of this project for change info */
#ifndef COORD_H
#define COORD_H

typedef struct {
    signed char x,y;
} coord;

typedef struct {
    signed char x;
    signed char y;
    signed char z; // +down -up (like counting dungeon levels)
} Direction;

#endif /* COORD_H */
