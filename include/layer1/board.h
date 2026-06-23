#ifndef COMBAT_BOARD_H
#define COMBAT_BOARD_H

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include "util/config.h"

#include <avr/io.h>

/*
 * Hardware specification for the ATtiny85 Combat project.
 *
 *   PB0 = 74HC595 serial data
 *   PB1 = 74HC595 shift clock
 *   PB2 = 74HC595 latch clock
 *   PB3 = button ADC ladder
 *   PB4 = passive buzzer driver
 */

#define PIN_SHIFT_SER     PB0
#define PIN_SHIFT_CLOCK   PB1
#define PIN_SHIFT_LATCH   PB2
#define PIN_ADC           PB3
#define PIN_BUZZER        PB4

/* U3 row/digit select output bit layout. */
#define SELECT_ROW0       0
#define SELECT_ROW1       1
#define SELECT_ROW2       2
#define SELECT_ROW3       3
#define SELECT_DIGIT0     4
#define SELECT_DIGIT1     5

#endif
