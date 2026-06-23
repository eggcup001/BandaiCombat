#ifndef COMBAT_STAGE_H
#define COMBAT_STAGE_H

#include <stdbool.h>
#include <stdint.h>
#include "util/types.h"

typedef struct
{
    Cell player;
    Cell enemy;
    uint32_t mineMask;
} Stage;

void stageGenerate(Stage *stage);

bool stageIsMine(const Stage *stage, Cell cell);

/* Mine proximity warning: four directions only. */
bool stageNearMine(const Stage *stage, Cell cell);

/* Enemy attack range: eight surrounding cells. */
bool stageNearEnemy(const Stage *stage, Cell cell);

/* Manhattan distance. */
uint8_t stageDistance(Cell a, Cell b);

#endif
