#ifndef COMBAT_ADC_LADDER_H
#define COMBAT_ADC_LADDER_H

#include <stdint.h>

/*
 * Raw ADC access for the PB3/ADC3 button ladder.
 * This layer does not know button meanings.
 */
void adcInit(void);
uint16_t adcReadButtons(void);

#endif
