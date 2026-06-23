#ifndef COMBAT_BUZZER_H
#define COMBAT_BUZZER_H

#include <stdint.h>

/* Timer1-driven square wave for the PB4 passive buzzer driver. */
void buzzerInit(void);
void buzzerTone(uint16_t frequency);
void buzzerStop(void);

#endif
