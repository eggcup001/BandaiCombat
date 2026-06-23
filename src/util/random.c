#include "util/random.h"

static uint16_t gLfsr = 0xACE1u;

/* 疑似乱数系列の初期値を設定し、0は停止状態を避けるため1へ置き換える。 */
void randomSeed(uint16_t seed)
{
    if(seed == 0)
    {
        seed = 1;
    }

    gLfsr = seed;
}

/* 16ビットLFSRを1段進め、次の疑似乱数を返す。 */
uint16_t random16(void)
{
    gLfsr = (uint16_t)((gLfsr >> 1) ^ (uint16_t)(-(int16_t)(gLfsr & 1u) & 0xB400u));
    return gLfsr;
}

/* 0以上max未満の疑似乱数を返し、maxが0なら0を返す。 */
uint8_t random8(uint8_t max)
{
    if(max == 0)
    {
        return 0;
    }

    return (uint8_t)(random16() % max);
}
