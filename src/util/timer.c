#include "util/timer.h"
#include "layer2/display_mux.h"

#include <avr/interrupt.h>
#include <avr/io.h>

static volatile uint32_t gMillis = 0;
static uint8_t gMillisDivider = 0;

/* Timer0を表示走査250us周期とシステム時刻1ms周期に設定する。 */
void timerInit(void)
{
    /*
     * F_CPU = 8MHz
     * prescaler = 8
     * timer clock = 1MHz
     * OCR0A = 249 => 250us display interrupt
     * Four display interrupts form one game-clock millisecond.
     */
    TCCR0A = (1 << WGM01);
    TCCR0B = (1 << CS01);
    OCR0A = 249;
    TIMSK |= (1 << OCIE0A);
    sei();
}

/* 割り込み更新中の32ビット値を壊さずに現在時刻として読み出す。 */
uint32_t timerMillis(void)
{
    uint32_t value;
    uint8_t sreg = SREG;

    cli();
    value = gMillis;
    SREG = sreg;

    return value;
}

/* 指定時間の経過を判定し、経過時は基準時刻を現在時刻へ更新する。 */
bool timerElapsed(uint32_t *timestamp, uint16_t intervalMs)
{
    uint32_t now;

    if(timestamp == 0)
    {
        return false;
    }

    now = timerMillis();

    if((uint32_t)(now - *timestamp) >= intervalMs)
    {
        *timestamp = now;
        return true;
    }

    return false;
}

/* 250usごとに表示を1相進め、4回ごとにミリ秒時計を進める。 */
ISR(TIMER0_COMPA_vect)
{
    displayMuxRefresh();

    gMillisDivider++;
    if(gMillisDivider >= 4)
    {
        gMillisDivider = 0;
        gMillis++;
    }
}
