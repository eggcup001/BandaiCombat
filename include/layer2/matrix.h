#ifndef COMBAT_MATRIX_H
#define COMBAT_MATRIX_H

#include <stdbool.h>
#include "util/types.h"

/*
 * Layer2 LED matrix driver.
 *
 * Displays only:
 *   - player: bright
 *   - enemy : dimmer
 *
 * Mines are intentionally invisible. Mine warnings are audio-only.
 */
void matrixInit(void);
void matrixClear(void);
void matrixSetPlayer(Cell cell);
void matrixSetEnemy(Cell cell);
void matrixEnemyBlink(bool enable);

/* Called from main loop; rebuilds bright/dim row buffers and handles blinking. */
void matrixUpdate(void);

#endif
