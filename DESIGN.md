# Design Notes

## Layer rule

Dependency direction is strictly:

```text
Layer3 game
  ↓
Layer2 device abstraction
  ↓
Layer1 hardware access
```

`main.c` calls only Layer3.

## Layer1

Hardware-specific only.

- `board.h`
- `shift595`
- `adc_ladder`
- `buzzer`

## Layer2

Device abstraction.

- `matrix`
- `buttons`
- `display`
- `display_mux`
- `sound`

Layer2 owns the shared six-phase display scan.

## Layer3

Game rules.

- `stage_patterns`
- `stage`
- `combat_game`

Layer3 must never directly touch GPIO or ADC.

## Hardware pin design

### PB0

PB0 is the serial data output for the 74HC595 chain.

### PB1

PB1 is the shared shift clock for U2 and U3.

### PB2

PB2 is the shared latch clock for U2 and U3.

### PB3

PB3/ADC3 reads the five-button resistor ladder.

### PB4

PB4 drives the 9042_16R passive buzzer through an NPN transistor.

## Display scan

`display_mux` refreshes one output phase every 250 us:

1. Matrix row 0
2. Matrix row 1
3. Matrix row 2
4. Matrix row 3
5. 7-segment digit 0 (U4, tens)
6. 7-segment digit 1 (U5, ones)

Each update first latches an all-off pattern and then latches the new BUS and
select values to prevent ghosting.

Display behavior:

- Wait screen: score
- Play: zero-padded up-counter starting at `00`
- Enemy defeated: score for about 500 ms, then next-stage wait until FIRE
- Game over: final score

## Stage system

Stages are generated from `StagePattern`.

Each pattern contains:

- player candidate mask
- enemy candidate mask
- mine count

Mines are generated randomly but never displayed. Generation reserves a
mine-free path from the player to an orthogonally adjacent attack position next
to the enemy, so every stage remains winnable.

The random seed mixes small variations in the PB3 button ADC readings with the
player's FIRE input timing. It is intended for varied gameplay, not security.
Startup collects multiple PB3 ADC samples before generating the first stage.
The first stage shown during the wait screen is reused unchanged when FIRE
starts play.

## Controls

- Direction key: move one cell
- FIRE, then direction key: attack the adjacent cell in that direction
- A missed attack consumes the direction input, does not move the player, and
  returns to movement mode
- A successful attack generates the next round, enters a wait state, and resumes
  play when FIRE is pressed
- FIRE while already in attack mode cancels the attack and returns to movement
  mode

## Sound system

Single-channel only.

Continuous modes:

- TICK: normal play, 2500 Hz short pulse
- WARNING: mine proximity, 1100 Hz low pulse

Effects:

- HIT
- KILL
- DEATH: low descending effect from 1500 Hz to 500 Hz
- GAME_OVER

Effects suppress continuous modes.

## Matrix display

Only player and enemy are drawn.

- Player: full brightness
- Enemy: one-eighth brightness by phase skipping
- Enemy PWM: synchronized to scan frames at about 83 Hz to avoid visible flicker
- WAIT, stage wait, and game over: enemy blinking + dimming
- Mines: never drawn

Mine proximity means the four orthogonally adjacent cells only. Diagonal mines
are not detected and do not produce the warning sound.

## Known tuning points

- ADC thresholds in `buttons.c`
- Buzzer tone frequencies
- Enemy dim ratio
- Sound frequency/duration tables
