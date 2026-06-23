#include "layer2/buttons.h"

#include "layer1/adc_ladder.h"
#include "util/timer.h"

#include <stdbool.h>
#include <stdint.h>
#include <avr/pgmspace.h>

#define DEBOUNCE_REQUIRED 3
#define SAMPLE_INTERVAL_MS 10
#define REPEAT_FIRST_MS 300
#define REPEAT_NEXT_MS 100

typedef struct
{
    uint16_t min;
    uint16_t max;
    Button button;
} ButtonRange;

/*
 * IMPORTANT:
 * Expected values for a 10k pull-up and 0/1k/2.2k/4.7k/10k branches are
 * approximately 0, 93, 185, 327, and 512.
 * Measure actual values on hardware and adjust.
 */
static const ButtonRange gRanges[] PROGMEM =
{
    {   0,  40, BTN_RIGHT },
    {  41, 135, BTN_DOWN  },
    { 136, 255, BTN_UP    },
    { 256, 420, BTN_LEFT  },
    { 421, 700, BTN_FIRE  }
};

static Button gCurrent = BTN_NONE;
static Button gPrevious = BTN_NONE;
static Button gCandidate = BTN_NONE;
static Button gStable = BTN_NONE;
static uint8_t gStableCount = 0;

static uint32_t gSampleTimer = 0;
static uint32_t gRepeatTimer = 0;
static bool gRepeatStarted = false;
static uint16_t gEntropy = 0xA5C3u;

/* ADC値をFlash上のしきい値表と照合し、対応するボタンへ変換する。 */
static Button decodeAdc(uint16_t value)
{
    uint8_t i;

    for(i = 0; i < (uint8_t)(sizeof(gRanges) / sizeof(gRanges[0])); i++)
    {
        uint16_t min = pgm_read_word(&gRanges[i].min);
        uint16_t max = pgm_read_word(&gRanges[i].max);

        if(value >= min && value <= max)
        {
            return (Button)pgm_read_byte(&gRanges[i].button);
        }
    }

    return BTN_NONE;
}

/* ボタンラダーを読むADCを初期化する。 */
void buttonsInit(void)
{
    adcInit();
}

/* 起動直後のPB3 ADCを複数回読み、最初のステージ用の揺らぎを蓄積する。 */
void buttonsCollectStartupEntropy(void)
{
    uint8_t i;

    for(i = 0; i < 32; i++)
    {
        uint16_t adcValue = adcReadButtons();

        gEntropy = (uint16_t)((gEntropy << 5) | (gEntropy >> 11));
        gEntropy ^= adcValue;
        gEntropy ^= (uint16_t)timerMillis();
    }
}

/* 10ms周期でADCを読み、連続一致した入力だけを確定状態へ反映する。 */
void buttonsUpdate(void)
{
    Button raw;
    uint16_t adcValue;

    /* Button edges are valid for exactly one game-update iteration. */
    gPrevious = gCurrent;

    if(!timerElapsed(&gSampleTimer, SAMPLE_INTERVAL_MS))
    {
        return;
    }

    adcValue = adcReadButtons();

    /*
     * ADC下位ビットの微小な揺らぎと、人間の操作で変化する時刻を蓄積する。
     * 暗号用途ではなく、電源投入ごとのステージ系列を変えるための値。
     */
    gEntropy = (uint16_t)((gEntropy << 5) | (gEntropy >> 11));
    gEntropy ^= adcValue;
    gEntropy ^= (uint16_t)timerMillis();

    raw = decodeAdc(adcValue);

    if(raw == gCandidate)
    {
        if(gStableCount < 255)
        {
            gStableCount++;
        }
    }
    else
    {
        gCandidate = raw;
        gStableCount = 0;
    }

    if(gStableCount >= DEBOUNCE_REQUIRED)
    {
        if(gStable != gCandidate)
        {
            gStable = gCandidate;
            gRepeatStarted = false;
            gRepeatTimer = timerMillis();
        }

        gCurrent = gStable;
    }
}

/* デバウンス後の現在のボタン状態を返す。 */
Button buttonsGet(void)
{
    return gCurrent;
}

/* 今回のゲーム更新で指定ボタンが押された瞬間かを返す。 */
bool buttonsPressed(Button button)
{
    return (gPrevious != button) && (gCurrent == button);
}

/* 今回のゲーム更新で指定ボタンが離された瞬間かを返す。 */
bool buttonsReleased(Button button)
{
    return (gPrevious == button) && (gCurrent != button);
}

/* 押した瞬間と、長押し開始後の一定間隔でtrueを返す。 */
bool buttonsRepeat(Button button)
{
    uint16_t interval;

    if(gCurrent != button || button == BTN_NONE)
    {
        return false;
    }

    if(buttonsPressed(button))
    {
        return true;
    }

    interval = gRepeatStarted ? REPEAT_NEXT_MS : REPEAT_FIRST_MS;

    if(timerElapsed(&gRepeatTimer, interval))
    {
        gRepeatStarted = true;
        return true;
    }

    return false;
}

/* 蓄積したADC揺らぎと操作時刻の混合値を返す。 */
uint16_t buttonsEntropy(void)
{
    return gEntropy;
}
