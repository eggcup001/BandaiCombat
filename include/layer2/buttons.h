#ifndef COMBAT_BUTTONS_H
#define COMBAT_BUTTONS_H

#include <stdbool.h>
#include "util/types.h"

/*
 * Layer2 button decoder.
 *
 * Raw ADC values are converted to semantic buttons, with:
 *   - debounce
 *   - pressed edge
 *   - released edge
 *   - key repeat for movement
 *
 * ADC thresholds are intentionally easy to adjust in buttons.c.
 */
void buttonsInit(void);
/* 起動直後のADCを複数回採取し、最初のステージ用seed材料を蓄積する。 */
void buttonsCollectStartupEntropy(void);
void buttonsUpdate(void);
Button buttonsGet(void);
bool buttonsPressed(Button button);
bool buttonsReleased(Button button);
bool buttonsRepeat(Button button);

/* ADCの微小な揺らぎと操作時刻を混ぜた、疑似乱数seed用の値を返す。 */
uint16_t buttonsEntropy(void);

#endif
