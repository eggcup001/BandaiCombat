#ifndef COMBAT_SOUND_H
#define COMBAT_SOUND_H

#include <stdbool.h>

/*
 * Single-channel sound system.
 *
 * Continuous modes:
 *   TICK    : normal play "ka-ka-ka"
 *   WARNING : mine proximity warning
 *
 * One-shot effects:
 *   HIT      : enemy attack
 *   KILL     : enemy destroyed
 *   DEATH    : player death
 *   GAMEOVER : final tone
 *
 * Only one sound plays at a time. Effects suppress continuous modes.
 */
typedef enum
{
    SOUND_MODE_NONE = 0,
    SOUND_MODE_TICK,
    SOUND_MODE_WARNING
} SoundMode;

void soundInit(void);
void soundUpdate(void);
void soundSetMode(SoundMode mode);

void soundPlayHit(void);
void soundPlayKill(void);
void soundPlayDeath(void);
void soundPlayGameOver(void);

bool soundBusy(void);

#endif
