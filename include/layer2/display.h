#ifndef COMBAT_DISPLAY_H
#define COMBAT_DISPLAY_H

#include <stdint.h>

/*
 * Two-digit multiplexed game display.
 *
 * Digit 0 is the tens digit (U4 / DIG0).
 * Digit 1 is the ones digit (U5 / DIG1).
 */
void displayInit(void);
void displayShowScore(uint8_t score);
void displayShowCount(uint8_t count);
void displayClear(void);

#endif
