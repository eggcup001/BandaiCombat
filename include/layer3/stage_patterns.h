#ifndef COMBAT_STAGE_PATTERNS_H
#define COMBAT_STAGE_PATTERNS_H

#include <stdint.h>
#include "layer3/stage_pattern.h"

void stagePatternsGet(uint8_t index, StagePattern *pattern);
uint8_t stagePatternsCount(void);

#endif
