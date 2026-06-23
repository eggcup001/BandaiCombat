#ifndef COMBAT_TYPES_H
#define COMBAT_TYPES_H

#include <stdint.h>

/*
 * Common types shared by all layers.
 *
 * Cell layout:
 *
 *   0  1  2  3  4
 *   5  6  7  8  9
 *  10 11 12 13 14
 *  15 16 17 18 19
 */
typedef uint8_t Cell;

typedef struct
{
    uint8_t x;
    uint8_t y;
} Point;

typedef enum
{
    BTN_NONE = 0,
    BTN_LEFT,
    BTN_RIGHT,
    BTN_UP,
    BTN_DOWN,
    BTN_FIRE
} Button;

#endif
