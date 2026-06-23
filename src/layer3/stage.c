#include "layer3/stage.h"

#include "layer3/stage_patterns.h"
#include "util/cell.h"
#include "util/config.h"
#include "util/random.h"

#include <stdbool.h>
#include <stdint.h>

/* マスク中で許可されたセルを数え、その中から均等に1つ選ぶ。 */
static Cell randomCellFromMask(uint32_t mask)
{
    uint8_t count = 0;
    uint8_t pick;
    uint8_t i;

    for(i = 0; i < MATRIX_CELLS; i++)
    {
        if(mask & (1UL << i))
        {
            count++;
        }
    }

    if(count == 0)
    {
        return 0;
    }

    pick = random8(count);

    for(i = 0; i < MATRIX_CELLS; i++)
    {
        if(mask & (1UL << i))
        {
            if(pick == 0)
            {
                return (Cell)i;
            }

            pick--;
        }
    }

    return 0;
}

/* 2セル間のマンハッタン距離を返す。 */
uint8_t stageDistance(Cell a, Cell b)
{
    uint8_t ax = cellX(a);
    uint8_t ay = cellY(a);
    uint8_t bx = cellX(b);
    uint8_t by = cellY(b);
    uint8_t dx = (ax > bx) ? (ax - bx) : (bx - ax);
    uint8_t dy = (ay > by) ? (ay - by) : (by - ay);

    return (uint8_t)(dx + dy);
}

/* 指定セルに地雷が配置されているかを調べる。 */
bool stageIsMine(const Stage *stage, Cell cell)
{
    if(stage == 0 || !cellValid(cell))
    {
        return false;
    }

    return (stage->mineMask & (1UL << cell)) != 0;
}

/* 指定セルの上下左右に地雷があるかを調べる。斜めは警告対象外。 */
bool stageNearMine(const Stage *stage, Cell cell)
{
    Cell n;

    if(stage == 0)
    {
        return false;
    }

    if(cellMove(cell, DIR_LEFT, &n)  && stageIsMine(stage, n)) return true;
    if(cellMove(cell, DIR_RIGHT, &n) && stageIsMine(stage, n)) return true;
    if(cellMove(cell, DIR_UP, &n)    && stageIsMine(stage, n)) return true;
    if(cellMove(cell, DIR_DOWN, &n)  && stageIsMine(stage, n)) return true;

    return false;
}

/* 指定セルが敵の周囲8マスに入り、敵の攻撃範囲内かを調べる。 */
bool stageNearEnemy(const Stage *stage, Cell cell)
{
    int8_t dx;
    int8_t dy;
    int8_t x;
    int8_t y;
    int8_t ex;
    int8_t ey;

    if(stage == 0)
    {
        return false;
    }

    x = (int8_t)cellX(cell);
    y = (int8_t)cellY(cell);
    ex = (int8_t)cellX(stage->enemy);
    ey = (int8_t)cellY(stage->enemy);

    for(dy = -1; dy <= 1; dy++)
    {
        for(dx = -1; dx <= 1; dx++)
        {
            if(dx == 0 && dy == 0)
            {
                continue;
            }

            if((x + dx) == ex && (y + dy) == ey)
            {
                return true;
            }
        }
    }

    return false;
}

/* 初期位置から敵の上下左右まで進める、地雷禁止の最短経路を作る。 */
static uint32_t makeSafePathMask(const Stage *stage)
{
    uint32_t mask;
    Cell current;
    Cell next;
    uint8_t x;
    uint8_t y;
    uint8_t ex;
    uint8_t ey;
    Direction dir;

    current = stage->player;
    mask = (1UL << current);
    ex = cellX(stage->enemy);
    ey = cellY(stage->enemy);

    while(stageDistance(current, stage->enemy) > 1)
    {
        x = cellX(current);
        y = cellY(current);

        /* Randomize the bend when both axes can move toward the enemy. */
        if(x != ex && y != ey)
        {
            if(random8(2) == 0)
            {
                dir = (x < ex) ? DIR_RIGHT : DIR_LEFT;
            }
            else
            {
                dir = (y < ey) ? DIR_DOWN : DIR_UP;
            }
        }
        else if(x != ex)
        {
            dir = (x < ex) ? DIR_RIGHT : DIR_LEFT;
        }
        else
        {
            dir = (y < ey) ? DIR_DOWN : DIR_UP;
        }

        if(!cellMove(current, dir, &next))
        {
            break;
        }

        current = next;
        mask |= (1UL << current);
    }

    return mask;
}

/* セルが開始地点、敵、安全経路を避けた地雷候補かを判定する。 */
static bool mineCandidateOk(const Stage *stage, Cell c, uint32_t safePathMask)
{
    if(c == stage->player || c == stage->enemy)
    {
        return false;
    }

    if(stageIsMine(stage, c))
    {
        return false;
    }

    if(safePathMask & (1UL << c))
    {
        return false;
    }

    /*
     * Avoid mines immediately next to the initial player position.
     * This gives the player at least one safe moment at stage start.
     */
    if(stageDistance(stage->player, c) < 2)
    {
        return false;
    }

    return true;
}

/* 安全経路を保ちながら地雷を置き、乱数で不足した場合は順番に補う。 */
static void placeMines(Stage *stage, uint8_t count)
{
    uint8_t placed = 0;
    uint8_t tries = 0;
    Cell c;
    uint32_t safePathMask = makeSafePathMask(stage);

    stage->mineMask = 0;

    while(placed < count && tries < 100)
    {
        tries++;
        c = random8(MATRIX_CELLS);

        if(mineCandidateOk(stage, c, safePathMask))
        {
            stage->mineMask |= (1UL << c);
            placed++;
        }
    }

    /*
     * Fallback: fill another valid cell while preserving the attack path.
     */
    c = 0;
    while(placed < count && c < MATRIX_CELLS)
    {
        if(mineCandidateOk(stage, c, safePathMask))
        {
            stage->mineMask |= (1UL << c);
            placed++;
        }

        c++;
    }
}

/* パターンを選び、プレイヤー、敵、攻略可能な地雷配置を生成する。 */
void stageGenerate(Stage *stage)
{
    StagePattern pattern;
    uint8_t patternIndex;
    uint8_t tries;

    if(stage == 0)
    {
        return;
    }

    patternIndex = random8(stagePatternsCount());
    stagePatternsGet(patternIndex, &pattern);

    stage->player = randomCellFromMask(pattern.playerMask);

    tries = 0;
    do
    {
        stage->enemy = randomCellFromMask(pattern.enemyMask);
        tries++;
    }
    while(
        (
            stage->enemy == stage->player ||
            (patternIndex == 9 && stageDistance(stage->player, stage->enemy) < 4)
        )
        &&
        tries < 50
    );

    if(stage->enemy == stage->player)
    {
        stage->enemy = (Cell)((stage->player + 7) % MATRIX_CELLS);
    }

    placeMines(stage, pattern.mineCount);
}
