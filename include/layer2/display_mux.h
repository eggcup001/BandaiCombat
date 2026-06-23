#ifndef COMBAT_DISPLAY_MUX_H
#define COMBAT_DISPLAY_MUX_H

#include <stdint.h>

/*
 * Shared six-phase scanner for the 5x4 matrix and two 7-segment digits.
 * Buffer writes occur in the main loop; displayMuxRefresh() runs from Timer0.
 */
void displayMuxInit(void);
void displayMuxSetRow(uint8_t row, uint8_t columns);
void displayMuxSetDimRow(uint8_t row, uint8_t columns);
void displayMuxSetDigit(uint8_t digit, uint8_t segments);
void displayMuxRefresh(void);

#endif
