#include "layer2/display_mux.h"

#include "layer1/board.h"
#include "layer1/shift595.h"

#include <stdint.h>

#define MATRIX_PHASES 4
#define DIGIT_PHASES 2
#define TOTAL_PHASES (MATRIX_PHASES + DIGIT_PHASES)

static volatile uint8_t gRows[MATRIX_PHASES];
static volatile uint8_t gDimRows[MATRIX_PHASES];
static volatile uint8_t gDigits[DIGIT_PHASES];
static uint8_t gPhase;
static uint8_t gDimPhase;

/* 共有走査バッファと74HC595を全消灯状態で初期化する。 */
void displayMuxInit(void)
{
    uint8_t i;

    shift595Init();

    for(i = 0; i < MATRIX_PHASES; i++)
    {
        gRows[i] = 0;
        gDimRows[i] = 0;
    }

    for(i = 0; i < DIGIT_PHASES; i++)
    {
        gDigits[i] = 0;
    }

    gPhase = 0;
    gDimPhase = 0;
}

/* 敵など1/8輝度で点灯する行データを走査バッファへ設定する。 */
void displayMuxSetDimRow(uint8_t row, uint8_t columns)
{
    if(row < MATRIX_PHASES)
    {
        gDimRows[row] = (uint8_t)(columns & 0x1F);
    }
}

/* 通常輝度で点灯するマトリクス行データを走査バッファへ設定する。 */
void displayMuxSetRow(uint8_t row, uint8_t columns)
{
    if(row < MATRIX_PHASES)
    {
        gRows[row] = (uint8_t)(columns & 0x1F);
    }
}

/* 指定した7セグメント桁の点灯パターンを走査バッファへ設定する。 */
void displayMuxSetDigit(uint8_t digit, uint8_t segments)
{
    if(digit < DIGIT_PHASES)
    {
        gDigits[digit] = segments;
    }
}

/* 1相だけ出力し、6相走査と8フレーム周期の敵PWMを進める。 */
void displayMuxRefresh(void)
{
    uint8_t busValue;
    uint8_t selectValue;

    /* バス切替中に前の選択先が光らないよう、先に全選択を解除する。 */
    shift595Write(0, 0);

    if(gPhase < MATRIX_PHASES)
    {
        busValue = gRows[gPhase];
        if(gDimPhase == 0)
        {
            busValue |= gDimRows[gPhase];
        }
        selectValue = (uint8_t)(1 << (SELECT_ROW0 + gPhase));
    }
    else
    {
        uint8_t digit = (uint8_t)(gPhase - MATRIX_PHASES);

        busValue = gDigits[digit];
        selectValue = (uint8_t)(1 << (SELECT_DIGIT0 + digit));
    }

    shift595Write(busValue, selectValue);

    gPhase++;
    if(gPhase >= TOTAL_PHASES)
    {
        gPhase = 0;
        gDimPhase = (uint8_t)((gDimPhase + 1) & 7);
    }
}
