#include "layer1/adc_ladder.h"
#include "layer1/board.h"

#include <avr/io.h>

/* ADC変換完了まで待ち、10ビットの生値を返す。 */
static uint16_t adcReadRaw(void)
{
    ADCSRA |= (1 << ADSC);

    while(ADCSRA & (1 << ADSC))
    {
    }

    return ADC;
}

/* PB3をプルアップなしのADC3入力として初期化する。 */
void adcInit(void)
{
    DDRB &= ~(1 << PIN_ADC);
    PORTB &= ~(1 << PIN_ADC);

    /* ATtiny85 PB3 is ADC3. VCC is used as the ADC reference. */
    ADMUX = (1 << MUX1) | (1 << MUX0);

    ADCSRA =
        (1 << ADEN)  |
        (1 << ADPS2) |
        (1 << ADPS1);
}

/* 安定したボタン値を得るため最初の変換を捨て、2回目を返す。 */
uint16_t adcReadButtons(void)
{
    DDRB &= ~(1 << PIN_ADC);
    PORTB &= ~(1 << PIN_ADC);

    /* Discard the first conversion after channel setup or long idle time. */
    (void)adcReadRaw();

    return adcReadRaw();
}
