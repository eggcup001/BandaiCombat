#ifndef COMBAT_TIMER_H
#define COMBAT_TIMER_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Timer0 provides:
 *   - 1ms system tick
 *   - 250 us matrix and 7-segment refresh interrupt
 *
 * The ISR intentionally does only two things:
 *   1. increment millisecond counter
 *   2. call displayMuxRefresh()
 */
void timerInit(void);
uint32_t timerMillis(void);

/*
 * Helper for non-blocking timing.
 *
 * Example:
 *   static uint32_t t;
 *   if(timerElapsed(&t, 500)) { ... }
 */
bool timerElapsed(uint32_t *timestamp, uint16_t intervalMs);

#endif
