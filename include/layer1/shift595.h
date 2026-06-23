#ifndef COMBAT_SHIFT595_H
#define COMBAT_SHIFT595_H

#include <stdint.h>

/*
 * Low-level writer for two daisy-chained 74HC595 devices.
 *
 * Serial data enters U2 and then U3. The select byte is shifted first so it
 * reaches U3; the bus byte is shifted second and remains in U2.
 */
void shift595Init(void);
void shift595Write(uint8_t busValue, uint8_t selectValue);

#endif
