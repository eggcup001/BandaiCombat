#include "layer3/stage_patterns.h"

#include <avr/pgmspace.h>

#define C(n) (1UL << (n))

static const StagePattern gPatterns[] PROGMEM =
{
    /* 0: corner vs corner */
    { C(0), C(19), 2 },

    /* 1: opposite corner groups */
    { C(0)|C(1)|C(5), C(14)|C(18)|C(19), 2 },

    /* 2: left vs right */
    {
        C(0)|C(1)|C(5)|C(6)|C(10)|C(11)|C(15)|C(16),
        C(3)|C(4)|C(8)|C(9)|C(13)|C(14)|C(18)|C(19),
        2
    },

    /* 3: top vs bottom */
    { C(0)|C(1)|C(2)|C(3)|C(4), C(15)|C(16)|C(17)|C(18)|C(19), 2 },

    /* 4: diagonal */
    { C(0)|C(6), C(13)|C(19), 2 },

    /* 5: center conflict */
    { C(6)|C(7)|C(11), C(8)|C(12)|C(13), 3 },

    /* 6: cross */
    { C(2)|C(7)|C(10)|C(11), C(13)|C(14)|C(17), 3 },

    /* 7: ring */
    {
        C(0)|C(1)|C(2)|C(3)|C(4)|C(5)|C(9)|C(15)|C(16)|C(17)|C(18)|C(19),
        C(0)|C(1)|C(2)|C(3)|C(4)|C(5)|C(9)|C(15)|C(16)|C(17)|C(18)|C(19),
        3
    },

    /* 8: ambush */
    { C(0)|C(4)|C(15)|C(19), C(7)|C(8)|C(11)|C(12), 3 },

    /* 9: structured random */
    { 0x000FFFFFUL, 0x000FFFFFUL, 3 }
};

/* 指定番号のステージ候補をFlashからRAM上の構造体へ読み出す。 */
void stagePatternsGet(uint8_t index, StagePattern *pattern)
{
    if(pattern == 0)
    {
        return;
    }

    if(index >= stagePatternsCount())
    {
        index = 0;
    }

    memcpy_P(pattern, &gPatterns[index], sizeof(*pattern));
}

/* 登録されているステージ候補数を返す。 */
uint8_t stagePatternsCount(void)
{
    return (uint8_t)(sizeof(gPatterns) / sizeof(gPatterns[0]));
}
