#include "layer2/sound.h"

#include "layer1/buzzer.h"
#include "util/timer.h"

#include <stdbool.h>
#include <stdint.h>
#include <avr/pgmspace.h>

typedef struct
{
    uint16_t freq;
    uint16_t duration;
} Tone;

typedef enum
{
    EFFECT_NONE = 0,
    EFFECT_HIT,
    EFFECT_KILL,
    EFFECT_DEATH,
    EFFECT_GAME_OVER
} Effect;

static const Tone gHit[] PROGMEM =
{
    { 2300, 100 },
    { 0, 0 }
};

static const Tone gKill[] PROGMEM =
{
    { 2400, 80 },
    { 2200, 80 },
    { 2000, 100 },
    { 1600, 120 },
    { 1200, 120 },
    { 0, 0 }
};

static const Tone gDeath[] PROGMEM =
{
    { 1500, 80 },
    { 1200, 90 },
    {  900, 100 },
    {  700, 120 },
    {  500, 180 },
    { 0, 0 }
};

static const Tone gGameOver[] PROGMEM =
{
    { 2400, 120 },
    { 2200, 120 },
    { 2000, 120 },
    { 1800, 120 },
    { 1600, 150 },
    { 1400, 150 },
    { 1200, 200 },
    { 1000, 250 },
    {  800, 300 },
    { 0, 0 }
};

static SoundMode gMode = SOUND_MODE_NONE;
static Effect gEffect = EFFECT_NONE;
static uint8_t gEffectStep = 0;
static uint32_t gTimer = 0;
static bool gPulseActive = false;

/* 効果音種別に対応するFlash上の音列テーブルを返す。 */
static const Tone *effectTable(Effect e)
{
    switch(e)
    {
        case EFFECT_HIT:       return gHit;
        case EFFECT_KILL:      return gKill;
        case EFFECT_DEATH:     return gDeath;
        case EFFECT_GAME_OVER: return gGameOver;
        default:               return 0;
    }
}

/* 連続音を止め、指定効果音を先頭ステップから開始する。 */
static void startEffect(Effect e)
{
    gEffect = e;
    gEffectStep = 0;
    gTimer = timerMillis();
    gPulseActive = false;

    buzzerStop();
}

/* 下位のブザードライバを初期化する。 */
void soundInit(void)
{
    buzzerInit();
}

/* 通常時に繰り返す連続音モードを設定する。 */
void soundSetMode(SoundMode mode)
{
    gMode = mode;
}

/* 優先再生する効果音が進行中かを返す。 */
bool soundBusy(void)
{
    return gEffect != EFFECT_NONE;
}

/* 敵から攻撃を受けた効果音を開始する。 */
void soundPlayHit(void)      { startEffect(EFFECT_HIT); }
/* 敵撃破の効果音を開始する。 */
void soundPlayKill(void)     { startEffect(EFFECT_KILL); }
/* プレイヤー死亡の効果音を開始する。 */
void soundPlayDeath(void)    { startEffect(EFFECT_DEATH); }
/* ゲームオーバーの効果音を開始する。 */
void soundPlayGameOver(void) { startEffect(EFFECT_GAME_OVER); }

/* Flashから現在の音程を読み、経過時間に応じて効果音を1段進める。 */
static void updateEffect(void)
{
    const Tone *table;
    uint16_t frequency;
    uint16_t duration;

    if(gEffect == EFFECT_NONE)
    {
        return;
    }

    table = effectTable(gEffect);
    if(table == 0)
    {
        buzzerStop();
        gEffect = EFFECT_NONE;
        gEffectStep = 0;
        return;
    }

    frequency = pgm_read_word(&table[gEffectStep].freq);

    if(frequency == 0)
    {
        buzzerStop();
        gEffect = EFFECT_NONE;
        gEffectStep = 0;
        return;
    }

    if(!gPulseActive)
    {
        buzzerTone(frequency);
        gPulseActive = true;
        gTimer = timerMillis();
        return;
    }

    duration = pgm_read_word(&table[gEffectStep].duration);
    if(timerElapsed(&gTimer, duration))
    {
        buzzerStop();
        gPulseActive = false;
        gEffectStep++;
    }
}

/* 通常音または警告音を、指定周期の短いパルスとして繰り返す。 */
static void updateContinuous(void)
{
    uint16_t period;
    uint16_t freq;
    uint16_t onTime;

    if(gMode == SOUND_MODE_NONE)
    {
        buzzerStop();
        gPulseActive = false;
        return;
    }

    if(gMode == SOUND_MODE_WARNING)
    {
        period = 250;
        freq = 1100;
        onTime = 80;
    }
    else
    {
        period = 200;
        freq = 2500;
        onTime = 15;
    }

    if(!gPulseActive)
    {
        if(timerElapsed(&gTimer, period))
        {
            buzzerTone(freq);
            gPulseActive = true;
            gTimer = timerMillis();
        }
    }
    else
    {
        if(timerElapsed(&gTimer, onTime))
        {
            buzzerStop();
            gPulseActive = false;
        }
    }
}

/* 効果音を優先し、再生中でなければ連続音を更新する。 */
void soundUpdate(void)
{
    if(gEffect != EFFECT_NONE)
    {
        updateEffect();
    }
    else
    {
        updateContinuous();
    }
}
