#include "layer2/display.h"
#include "layer2/display_mux.h"

#include <stdint.h>
#include <avr/pgmspace.h>

#define SEG_BLANK 0x00

static const uint8_t gDigits[10] PROGMEM =
{
    0x3F, /* 0 */
    0x06, /* 1 */
    0x5B, /* 2 */
    0x4F, /* 3 */
    0x66, /* 4 */
    0x6D, /* 5 */
    0x7D, /* 6 */
    0x07, /* 7 */
    0x7F, /* 8 */
    0x6F  /* 9 */
};

/* 値を十の位の先頭ゼロなしで2桁分のセグメントへ変換する。 */
static void twoDigitNoPad(uint8_t value, uint8_t segments[2])
{
    if(value >= 100)
    {
        value %= 100;
    }

    if(value >= 10)
    {
        segments[0] = pgm_read_byte(&gDigits[value / 10]);
    }
    else
    {
        segments[0] = SEG_BLANK;
    }

    segments[1] = pgm_read_byte(&gDigits[value % 10]);
}

/* 値を先頭ゼロ付きで2桁分のセグメントへ変換する。 */
static void twoDigitZeroPad(uint8_t value, uint8_t segments[2])
{
    value %= 100;
    segments[0] = pgm_read_byte(&gDigits[value / 10]);
    segments[1] = pgm_read_byte(&gDigits[value % 10]);
}

/* 変換済みパターンをU4の十の位とU5の一の位へ設定する。 */
static void applySegments(const uint8_t segments[2])
{
    /* Hardware mapping: digit 0 = U4 tens, digit 1 = U5 ones. */
    displayMuxSetDigit(0, segments[0]);
    displayMuxSetDigit(1, segments[1]);
}

/* 2桁7セグメント表示を消灯状態で初期化する。 */
void displayInit(void)
{
    displayClear();
}

/* スコアを先頭ゼロなしで表示する。 */
void displayShowScore(uint8_t score)
{
    uint8_t segments[2];

    twoDigitNoPad(score, segments);
    applySegments(segments);
}

/* ゲームカウントを00～99のゼロ埋め表示にする。 */
void displayShowCount(uint8_t count)
{
    uint8_t segments[2];

    twoDigitZeroPad(count, segments);
    applySegments(segments);
}

/* 両方の7セグメント桁を消灯する。 */
void displayClear(void)
{
    displayMuxSetDigit(0, SEG_BLANK);
    displayMuxSetDigit(1, SEG_BLANK);
}
