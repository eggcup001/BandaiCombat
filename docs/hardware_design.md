# ATtiny85 Combat Hardware Design

This is the detailed hardware plan for the rebuilt ATtiny85 Combat board.

## Goals

- Keep ATtiny85 RESET/PB5 as reset.
- Use 5 normal GPIO pins only.
- Support 5 buttons.
- Support a 5 x 4 LED matrix.
- Support two normal 7-segment LED digits.
- Support one passive buzzer.
- Avoid sharing display scan pins with buttons or buzzer.

## Main Architecture

Use two daisy-chained 74HC595 shift registers.

- U2: LED anode bus for matrix columns and 7-segment segments.
- U3: low-side select control for matrix rows and 7-segment digits.
- U3 outputs do not drive LED cathodes directly. They drive NPN transistor bases.

The display is multiplexed in six phases:

1. Matrix row 0
2. Matrix row 1
3. Matrix row 2
4. Matrix row 3
5. 7-segment digit 0 (U4, tens)
6. 7-segment digit 1 (U5, ones)

Only one row or digit is selected at a time.

## MCU Pin Assignment

| ATtiny85 pin | Signal | Destination |
|---|---|---|
| PB0 | SHIFT_SER | U2 DS/SER |
| PB1 | SHIFT_CLK | U2 SHCP/SRCLK and U3 SHCP/SRCLK |
| PB2 | SHIFT_LATCH | U2 STCP/RCLK and U3 STCP/RCLK |
| PB3 | BTN_ADC | 5-button ADC ladder |
| PB4 | BUZZER | Passive buzzer driver |
| PB5 | RESET | Reset only, not used as GPIO |

## 74HC595 Pin Connections

Use SN74HC595, TC74HC595, or another 3 V capable HC-family part.
Avoid HCT-family parts at 3 V.

### U2: shared segment / column bus

| U2 pin | 74HC595 name | Net |
|---|---|---|
| 16 | VCC | +3V |
| 8 | GND | GND |
| 14 | DS/SER | SHIFT_SER from PB0 |
| 11 | SHCP/SRCLK | SHIFT_CLK from PB1 |
| 12 | STCP/RCLK | SHIFT_LATCH from PB2 |
| 10 | MR/SRCLR | +3V |
| 13 | OE | GND |
| 9 | Q7S/QH' | U3 DS/SER |
| 15 | QA/Q0 | BUS0 through 330 ohm array element |
| 1 | QB/Q1 | BUS1 through 330 ohm array element |
| 2 | QC/Q2 | BUS2 through 330 ohm array element |
| 3 | QD/Q3 | BUS3 through 330 ohm array element |
| 4 | QE/Q4 | BUS4 through 330 ohm array element |
| 5 | QF/Q5 | BUS5 through 330 ohm array element |
| 6 | QG/Q6 | BUS6 through 330 ohm array element |
| 7 | QH/Q7 | BUS7 through 330 ohm array element, optional DP/spare |

The eight 330 ohm array elements are shared current-limiting resistors for both
the LED matrix and the 2-digit 7-segment display. Put them between U2 outputs
and BUS0-BUS7, before the BUS lines branch to the matrix columns and segments.

Recommended starting value: 330 ohm for high-efficiency red LEDs at 3 V.
Use 220 ohm only if the LEDs are too dim and current limits are still acceptable.

Use an 8-resistor isolated type, not a bussed/common-pin type, because each LED
bus line needs its own series resistor.

### U3: row / digit select

| U3 pin | 74HC595 name | Net |
|---|---|---|
| 16 | VCC | +3V |
| 8 | GND | GND |
| 14 | DS/SER | U2 Q7S/QH' |
| 11 | SHCP/SRCLK | SHIFT_CLK from PB1 |
| 12 | STCP/RCLK | SHIFT_LATCH from PB2 |
| 10 | MR/SRCLR | +3V |
| 13 | OE | GND |
| 15 | QA/Q0 | ROW0_DRV through 4.7k |
| 1 | QB/Q1 | ROW1_DRV through 4.7k |
| 2 | QC/Q2 | ROW2_DRV through 4.7k |
| 3 | QD/Q3 | ROW3_DRV through 4.7k |
| 4 | QE/Q4 | DIG0_DRV through 4.7k |
| 5 | QF/Q5 | DIG1_DRV through 4.7k |
| 6 | QG/Q6 | spare |
| 7 | QH/Q7 | spare |
| 9 | Q7S/QH' | no connect |

## Low-Side Select Drivers

Use one NPN transistor per matrix row and per 7-segment digit.

Recommended parts:

- 2N3904
- 2SC1815
- BC547
- S8050

For each select transistor:

| Node | Connection |
|---|---|
| Base | U3 output through 4.7k |
| Emitter | GND |
| Collector | selected cathode bus |

Optional but recommended:

- 100k from base to GND for each transistor, to keep it off during reset.

The 4.7k base resistors may be replaced with an isolated resistor array.
The optional 100k base pulldowns may use a bussed resistor array with the common
pin connected to GND.

## LED Matrix Wiring

Use a 5 x 4 matrix with column anodes and row cathodes.

| Matrix signal | Connection |
|---|---|
| COL0 anodes | BUS0 |
| COL1 anodes | BUS1 |
| COL2 anodes | BUS2 |
| COL3 anodes | BUS3 |
| COL4 anodes | BUS4 |
| ROW0 cathodes | Q_ROW0 collector |
| ROW1 cathodes | Q_ROW1 collector |
| ROW2 cathodes | Q_ROW2 collector |
| ROW3 cathodes | Q_ROW3 collector |

The current-limiting resistors are the shared 330 ohm array paths between U2
outputs and BUS0-BUS4.

## 2-Digit 7-Segment Wiring

Use common-cathode 7-segment displays.

| Segment | Connection |
|---|---|
| a | BUS0 |
| b | BUS1 |
| c | BUS2 |
| d | BUS3 |
| e | BUS4 |
| f | BUS5 |
| g | BUS6 |
| dp | BUS7, optional |
| Digit 0 common cathode | Q_DIG0 collector |
| Digit 1 common cathode | Q_DIG1 collector |

The current-limiting resistors are the shared 330 ohm array paths between U2
outputs and BUS0-BUS7.

## Button ADC Ladder

Use PB3 as the only button input.

Recommended starting circuit:

- PB3/ADC node pulled up to +3V through 10k.
- Each button connects PB3/ADC to GND through a different resistor.

Suggested resistor values:

| Button | Series resistor to GND |
|---|---|
| RIGHT | 0 ohm |
| DOWN | 1.0k |
| UP | 2.2k |
| LEFT | 4.7k |
| FIRE | 10k |

Expected ADC values with a 10k pull-up are only approximate and must be measured
on the real board.

Use individual resistors for the ADC ladder unless the exact resistor network
pinout is checked carefully. A bussed resistor array is usually not suitable for
the button ladder values because each button branch needs a different value.

Add a small capacitor from PB3/ADC to GND if readings are noisy.
Recommended starting value: 1 nF to 10 nF.

## Buzzer Driver

Use PB4 as a dedicated buzzer output.

The completed schematic uses a 9042_16R electromagnetic passive buzzer, a
2SC1815 low-side driver, and a 1N4148 flyback diode.

Recommended circuit:

| Node | Connection |
|---|---|
| PB4 | 1k resistor to NPN base |
| NPN emitter | GND |
| NPN collector | buzzer negative |
| buzzer positive | +3V |
| 1N4148 cathode (banded end) | +3V |
| 1N4148 anode | NPN collector / buzzer negative |

Connect the 1N4148 in parallel with the buzzer, not in series. The diode is
reverse-biased during normal operation and clamps the inductive voltage when
the transistor turns off.

## Power and Decoupling

Add:

- 0.1 uF ceramic capacitor near ATtiny85 VCC/GND.
- 0.1 uF ceramic capacitor near each 74HC595 VCC/GND.
- 10 uF electrolytic bulk capacitor between +3V and GND near U2/U3. Connect
  its positive lead to +3V and its negative lead to GND.

If powered from a coin cell, display brightness may be poor because multiplexed
LED peak current causes voltage sag. AAA cells or a regulated supply are safer.

## Software Polarity

U2 bus bits are active high.

- BUS bit 1: segment/column anode enabled.
- BUS bit 0: segment/column off.

U3 select bits are active high before the NPN driver.

- U3 select bit 1: corresponding row/digit is selected.
- U3 select bit 0: corresponding row/digit off.

The refresh routine should:

1. Shift U3 select bits all off and U2 bus all off.
2. Latch.
3. Shift the desired U2 bus pattern and one U3 select bit.
4. Latch.
5. Advance to the next phase on the next refresh tick.

This avoids ghosting while the shift registers are being updated.

## Two-Digit Display Behavior

U4 is the tens digit and U5 is the ones digit. The wait screen shows the
score. During play, the zero-padded counter starts at `00` and counts up.
Defeating an enemy shows the score for about 500 ms before returning to the
counter. Game over keeps the final score visible.
