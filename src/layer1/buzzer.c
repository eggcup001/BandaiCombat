#include "layer1/buzzer.h"
#include "layer1/board.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdbool.h>

static volatile bool gToneActive = false;

typedef struct
{
    uint16_t divider;
    uint8_t clockSelect;
} Timer1Prescaler;

static const Timer1Prescaler gPrescalers[] PROGMEM =
{
    {    1, (1 << CS10) },
    {    2, (1 << CS11) },
    {    4, (1 << CS11) | (1 << CS10) },
    {    8, (1 << CS12) },
    {   16, (1 << CS12) | (1 << CS10) },
    {   32, (1 << CS12) | (1 << CS11) },
    {   64, (1 << CS12) | (1 << CS11) | (1 << CS10) },
    {  128, (1 << CS13) },
    {  256, (1 << CS13) | (1 << CS10) },
    {  512, (1 << CS13) | (1 << CS11) },
    { 1024, (1 << CS13) | (1 << CS11) | (1 << CS10) }
};

/* Timer1比較割り込みを止め、発音状態を初期化する。 */
static void timer1Off(void)
{
    TIMSK &= ~(1 << OCIE1A);
    TCCR1 = 0;
    TCNT1 = 0;
    gToneActive = false;
}

/* ブザー駆動ピンとTimer1を無音状態で初期化する。 */
void buzzerInit(void)
{
    timer1Off();
    DDRB |= (1 << PIN_BUZZER);
    PORTB &= ~(1 << PIN_BUZZER);
}

/* 指定周波数に最も細かく対応できるTimer1分周比を選んで発音を開始する。 */
void buzzerTone(uint16_t frequency)
{
    uint8_t i;
    uint8_t ocr = 0;
    uint8_t clockSelect = 0;
    uint8_t sreg;

    if(frequency == 0)
    {
        buzzerStop();
        return;
    }

    for(i = 0; i < (uint8_t)(sizeof(gPrescalers) / sizeof(gPrescalers[0])); i++)
    {
        uint32_t ticks = (uint32_t)F_CPU;
        uint16_t divider = pgm_read_word(&gPrescalers[i].divider);

        ticks /= (uint32_t)divider;
        ticks /= (uint32_t)frequency;
        ticks /= 2UL;

        if(ticks > 0 && ticks <= 256UL)
        {
            ocr = (uint8_t)(ticks - 1UL);
            clockSelect = pgm_read_byte(&gPrescalers[i].clockSelect);
            break;
        }
    }

    if(clockSelect == 0)
    {
        buzzerStop();
        return;
    }

    sreg = SREG;
    cli();

    PORTB &= ~(1 << PIN_BUZZER);
    TIMSK &= ~(1 << OCIE1A);
    TCCR1 = 0;
    TCNT1 = 0;
    OCR1A = ocr;
    OCR1C = ocr;

#ifdef PSR1
    GTCCR |= (1 << PSR1);
#endif

    gToneActive = true;
    TIMSK |= (1 << OCIE1A);
    TCCR1 = (1 << CTC1) | clockSelect;

    SREG = sreg;
}

/* 割り込みとの競合を避けながらTimer1とブザー出力を停止する。 */
void buzzerStop(void)
{
    uint8_t sreg = SREG;

    cli();
    timer1Off();
    PORTB &= ~(1 << PIN_BUZZER);
    SREG = sreg;
}

/* 比較一致ごとにPB4を反転し、ブザー用の矩形波を作る。 */
ISR(TIMER1_COMPA_vect)
{
    if(gToneActive)
    {
        PORTB ^= (uint8_t)(1 << PIN_BUZZER);
    }
}
