# ATtiny85 Combat LED Game

ATtiny85 を使ってバンダイ「コンバット」風のLSIゲームを再現するための C プロジェクトです。

## Hardware

- MCU: ATtiny85, internal 8MHz, 3V
- LED matrix: 5 x 4
- 74HC595 x 2: LED anode bus and row/digit selection
- Buttons: 5-button ADC ladder
- Display: two common-cathode 7-segment digits
- Sound: 9042_16R electromagnetic passive buzzer

## Pin assignment

| Pin | Role |
|---|---|
| PB0 | 74HC595 SER |
| PB1 | 74HC595 SRCLK |
| PB2 | 74HC595 RCLK / LATCH |
| PB3 | ADC button ladder |
| PB4 | Buzzer driver |

## Display

U4 is the tens digit and U5 is the ones digit.

```text
U4 U5
 0  0   counter 00
    3   score 3
```

- Wait screen: score
- Game start: counter starts at `00`
- Normal play: zero-padded up-counter
- Enemy defeated: score for about 500 ms, then the next stage waits for FIRE
- Game over: the final score remains visible
- FIRE starts or restarts the game directly

## Game notes

- Mines are invisible.
- Mine proximity in the four orthogonally adjacent cells is indicated by sound only.
- Diagonal mines are not detected and do not produce the warning sound.
- The mine warning uses a low 1100 Hz pulse, distinct from the normal 2500 Hz tick.
- The death effect is a low descending tone from 1500 Hz to 500 Hz.
- Player is bright.
- Enemy uses one-eighth brightness and blinks on wait screens.
- Enemy dimming is scan-synchronized at about 83 Hz to avoid visible flicker.
- Mine placement always preserves a safe path to an orthogonally adjacent
  attack position next to the enemy.
- Stage randomization mixes PB3 ADC noise with the timing of FIRE input.
- Startup samples the PB3 ADC repeatedly before generating the first stage.
- The first stage shown during the wait screen is kept unchanged when FIRE
  starts play.
- Death restarts the same stage layout.
- Press FIRE to enter attack mode, then press a direction key to attack the
  adjacent cell in that direction.
- The direction key is consumed by the attack and does not move the player.
- A missed attack returns to movement mode without moving the player.
- A successful attack prepares the next round and waits for FIRE before play resumes.
- Press FIRE again while in attack mode to cancel it and return to movement
  mode.

## Build

```bash
make
```

## Flash

```bash
make flash
```

## Fuse example

```bash
make fuse
```

Fuse values:

- LFUSE: `0xE2`
- HFUSE: `0xDF`
- EFUSE: `0xFF`

These are intended for internal 8MHz, CKDIV8 disabled, reset enabled, BOD disabled.

## Important hardware tuning

`src/layer2/buttons.c` contains ADC thresholds.  
These must be adjusted using real measured ADC values.
