#ifndef COMBAT_RANDOM_H
#define COMBAT_RANDOM_H

#include <stdint.h>

/*
 * Small 16-bit LFSR random generator.
 * Suitable for tiny games; not cryptographic.
 */
void randomSeed(uint16_t seed);
uint16_t random16(void);
uint8_t random8(uint8_t max);

#endif
