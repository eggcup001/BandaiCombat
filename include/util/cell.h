#ifndef COMBAT_CELL_H
#define COMBAT_CELL_H

#include <stdbool.h>
#include <stdint.h>
#include "util/types.h"
#include "util/direction.h"

/*
 * Convert x,y to Cell.
 * x: 0..4
 * y: 0..3
 */
Cell cellFromXY(uint8_t x, uint8_t y);

/* Return x coordinate from Cell. */
uint8_t cellX(Cell cell);

/* Return y coordinate from Cell. */
uint8_t cellY(Cell cell);

/* True if cell is inside the 5x4 matrix. */
bool cellValid(Cell cell);

/*
 * Move one cell in a direction.
 * Returns false if movement would leave the board.
 */
bool cellMove(Cell current, Direction dir, Cell *next);

#endif
