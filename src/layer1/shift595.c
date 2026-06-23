#include "layer1/shift595.h"
#include "layer1/board.h"

#include <avr/io.h>

/* 74HC595のシフトクロックへ立ち上がりパルスを送る。 */
static void pulseClock(void)
{
    PORTB |= (1 << PIN_SHIFT_CLOCK);
    PORTB &= ~(1 << PIN_SHIFT_CLOCK);
}

/* 74HC595のラッチクロックを駆動し、転送済みデータを出力へ反映する。 */
static void pulseLatch(void)
{
    PORTB |= (1 << PIN_SHIFT_LATCH);
    PORTB &= ~(1 << PIN_SHIFT_LATCH);
}

/* 1バイトをMSBから直列データ線へ送る。 */
static void shiftByte(uint8_t value)
{
    uint8_t i;

    for(i = 0; i < 8; i++)
    {
        if(value & 0x80)
        {
            PORTB |= (1 << PIN_SHIFT_SER);
        }
        else
        {
            PORTB &= ~(1 << PIN_SHIFT_SER);
        }

        pulseClock();
        value <<= 1;
    }
}

/* 3本の制御ピンを出力に設定し、全出力を消灯状態にする。 */
void shift595Init(void)
{
    DDRB |=
        (1 << PIN_SHIFT_SER) |
        (1 << PIN_SHIFT_CLOCK) |
        (1 << PIN_SHIFT_LATCH);

    PORTB &=
        (uint8_t)~(
            (1 << PIN_SHIFT_SER) |
            (1 << PIN_SHIFT_CLOCK) |
            (1 << PIN_SHIFT_LATCH));

    shift595Write(0, 0);
}

/* U3選択値、U2バス値の順に送り、2個の74HC595を同時更新する。 */
void shift595Write(uint8_t busValue, uint8_t selectValue)
{
    shiftByte(selectValue);
    shiftByte(busValue);
    pulseLatch();
}
