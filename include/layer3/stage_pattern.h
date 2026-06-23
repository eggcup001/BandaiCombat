#ifndef COMBAT_STAGE_PATTERN_H
#define COMBAT_STAGE_PATTERN_H

#include <stdint.h>

typedef struct
{
    uint32_t playerMask;
    uint32_t enemyMask;
    uint8_t mineCount;
} StagePattern;

#endif
