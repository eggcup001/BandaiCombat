#include "layer3/combat_game.h"

#include "layer2/buttons.h"
#include "layer2/display.h"
#include "layer2/matrix.h"
#include "layer2/sound.h"
#include "layer3/stage.h"
#include "util/cell.h"
#include "util/config.h"
#include "util/direction.h"
#include "util/random.h"
#include "util/timer.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    GAME_WAIT = 0,
    GAME_PLAY,
    GAME_DEAD,
    GAME_NEXT_STAGE,
    GAME_STAGE_WAIT,
    GAME_OVER
} GameState;

static GameState gState;
static Stage gStage;
static Stage gStageBackup;

static uint16_t gScore;
static uint8_t gLives;
static uint8_t gCount;
static uint8_t gEnemyHits;

static uint32_t gCountTimer;
static uint32_t gEnemyAttackTimer;
static uint32_t gStateTimer;

static bool gAttackMode;
static bool gGameOverSoundStarted;

#define SCORE_DISPLAY_MS 500

/* 現在のプレイヤーと敵の位置をマトリクス表示へ反映する。 */
static void drawStage(void)
{
    matrixSetPlayer(gStage.player);
    matrixSetEnemy(gStage.enemy);
    matrixUpdate();
}

/* 新しいステージを生成して死亡時の再開用コピーを保存する。 */
static void startNewStage(void)
{
    stageGenerate(&gStage);
    gStageBackup = gStage;
    gEnemyHits = 0;
    drawStage();
}

/* スコア、カウント、残機を初期化し、最初のステージを準備する。 */
static void startNewGame(void)
{
    uint32_t now = timerMillis();
    uint16_t seed;

    /* FIREを押した時刻とPB3のADC揺らぎを混ぜ、起動ごとの固定系列を避ける。 */
    seed = buttonsEntropy();
    seed ^= (uint16_t)now;
    seed ^= (uint16_t)(now >> 16);
    randomSeed(seed);

    gScore = 0;
    gCount = 0;
    gLives = PLAYER_LIVES;
    gEnemyHits = 0;
    gAttackMode = false;
    gGameOverSoundStarted = false;
    gCountTimer = timerMillis();

    startNewStage();

    displayShowCount(gCount);
}

/* ゲーム状態を切り替え、表示、音、状態固有タイマを入口で整える。 */
static void enterState(GameState s)
{
    gState = s;
    gStateTimer = timerMillis();

    if(s == GAME_WAIT)
    {
        matrixEnemyBlink(true);
        soundSetMode(SOUND_MODE_NONE);
        displayShowScore((uint8_t)(gScore % 100));
    }
    else if(s == GAME_PLAY)
    {
        gAttackMode = false;
        matrixEnemyBlink(false);
        gEnemyAttackTimer = timerMillis();
        displayShowCount(gCount);
    }
    else if(s == GAME_DEAD)
    {
        gAttackMode = false;
        soundSetMode(SOUND_MODE_NONE);
        soundPlayDeath();
        gLives--;
    }
    else if(s == GAME_NEXT_STAGE)
    {
        gAttackMode = false;
        soundSetMode(SOUND_MODE_NONE);
        soundPlayKill();
        displayShowScore((uint8_t)(gScore % 100));
    }
    else if(s == GAME_STAGE_WAIT)
    {
        gAttackMode = false;
        matrixEnemyBlink(true);
        soundSetMode(SOUND_MODE_NONE);
        displayShowScore((uint8_t)(gScore % 100));
    }
    else if(s == GAME_OVER)
    {
        gAttackMode = false;
        soundSetMode(SOUND_MODE_NONE);
        displayShowScore((uint8_t)(gScore % 100));
        gGameOverSoundStarted = false;
    }
}

/* プレイヤー死亡状態へ移行し、死亡処理を開始する。 */
static void playerKilled(void)
{
    enterState(GAME_DEAD);
}

/* 盤内ならプレイヤーを1マス動かし、敵や地雷との衝突も判定する。 */
static bool tryMovePlayer(Direction dir)
{
    Cell next;

    if(!cellMove(gStage.player, dir, &next))
    {
        return false;
    }

    gStage.player = next;

    if(gStage.player == gStage.enemy)
    {
        playerKilled();
        return true;
    }

    if(stageIsMine(&gStage, gStage.player))
    {
        playerKilled();
        return true;
    }

    drawStage();
    return true;
}

/* 隣接する指定方向を攻撃し、敵に命中した場合だけ得点して次へ進む。 */
static void tryAttack(Direction dir)
{
    Cell target;

    if(cellMove(gStage.player, dir, &target))
    {
        if(target == gStage.enemy)
        {
            gScore++;
            enterState(GAME_NEXT_STAGE);
        }
    }
}

/* 今回押された方向ボタンを攻撃方向として1つ取得する。 */
static bool pressedDirection(Direction *dir)
{
    if(buttonsPressed(BTN_LEFT))
    {
        *dir = DIR_LEFT;
    }
    else if(buttonsPressed(BTN_RIGHT))
    {
        *dir = DIR_RIGHT;
    }
    else if(buttonsPressed(BTN_UP))
    {
        *dir = DIR_UP;
    }
    else if(buttonsPressed(BTN_DOWN))
    {
        *dir = DIR_DOWN;
    }
    else
    {
        return false;
    }

    return true;
}

/* 初期待機中の1面目を表示し、FIRE後も同じ配置のまま開始する。 */
static void updateWait(void)
{
    soundSetMode(SOUND_MODE_NONE);
    drawStage();

    if(buttonsPressed(BTN_FIRE))
    {
        /* 待機中に見えている1面目を維持し、カウントだけここから開始する。 */
        gCountTimer = timerMillis();
        enterState(GAME_PLAY);
    }
}

/* カウント、移動、攻撃、敵攻撃、地雷警告を通常プレイ中に更新する。 */
static void updatePlay(void)
{
    Direction attackDirection;

    if(timerElapsed(&gCountTimer, 1000))
    {
        if(gCount < 99)
        {
            gCount++;
        }

        displayShowCount(gCount);

        if(gCount >= 99)
        {
            enterState(GAME_OVER);
            return;
        }
    }

    if(buttonsPressed(BTN_FIRE))
    {
        gAttackMode = !gAttackMode;
    }

    /* 攻撃モードの方向入力は攻撃だけに使い、プレイヤーを移動させない。 */
    if(gAttackMode)
    {
        if(pressedDirection(&attackDirection))
        {
            gAttackMode = false;
            tryAttack(attackDirection);
        }
    }
    else if(buttonsRepeat(BTN_LEFT))
    {
        tryMovePlayer(DIR_LEFT);
    }
    else if(buttonsRepeat(BTN_RIGHT))
    {
        tryMovePlayer(DIR_RIGHT);
    }
    else if(buttonsRepeat(BTN_UP))
    {
        tryMovePlayer(DIR_UP);
    }
    else if(buttonsRepeat(BTN_DOWN))
    {
        tryMovePlayer(DIR_DOWN);
    }

    if(gState != GAME_PLAY)
    {
        return;
    }

    if(stageNearEnemy(&gStage, gStage.player))
    {
        if(timerElapsed(&gEnemyAttackTimer, 1000))
        {
            gEnemyHits++;
            soundPlayHit();

            if(gEnemyHits >= 3)
            {
                playerKilled();
                return;
            }
        }
    }
    else
    {
        gEnemyHits = 0;
        gEnemyAttackTimer = timerMillis();
    }

    if(stageNearMine(&gStage, gStage.player))
    {
        soundSetMode(SOUND_MODE_WARNING);
    }
    else
    {
        soundSetMode(SOUND_MODE_TICK);
    }

    drawStage();
}

/* 死亡演出中は盤面を消し、効果音後に同じ配置またはゲームオーバーへ進む。 */
static void updateDead(void)
{
    matrixClear();

    if(soundBusy())
    {
        return;
    }

    if(timerElapsed(&gStateTimer, 500))
    {
        if(gLives == 0)
        {
            enterState(GAME_OVER);
        }
        else
        {
            gStage = gStageBackup;
            gEnemyHits = 0;
            drawStage();
            enterState(GAME_PLAY);
        }
    }
}

/* 敵撃破後のスコア表示時間を待ち、次のステージを準備する。 */
static void updateNextStage(void)
{
    if(timerElapsed(&gStateTimer, SCORE_DISPLAY_MS))
    {
        startNewStage();
        enterState(GAME_STAGE_WAIT);
    }
}

/* 次ステージの待機表示を保ち、FIREでカウントを再開する。 */
static void updateStageWait(void)
{
    soundSetMode(SOUND_MODE_NONE);
    drawStage();

    if(buttonsPressed(BTN_FIRE))
    {
        gCountTimer = timerMillis();
        enterState(GAME_PLAY);
    }
}

/* 最終スコアと点滅表示を保ち、FIREで新規ゲームを開始する。 */
static void updateGameOver(void)
{
    displayShowScore((uint8_t)(gScore % 100));
    matrixEnemyBlink(true);
    drawStage();

    if(!gGameOverSoundStarted)
    {
        soundPlayGameOver();
        gGameOverSoundStarted = true;
    }

    if(buttonsPressed(BTN_FIRE))
    {
        startNewGame();
        enterState(GAME_PLAY);
    }
}

/* 全デバイスとゲームデータを初期化し、初期待機状態へ入る。 */
void combatGameInit(void)
{
    matrixInit();
    buttonsInit();
    displayInit();
    soundInit();
    timerInit();

    /* 盤面を見せる前にADC揺らぎを集め、そのseedで1面目を生成する。 */
    buttonsCollectStartupEntropy();
    startNewGame();
    enterState(GAME_WAIT);
}

/* 入力、音、表示を更新してから、現在状態に対応する処理を1回実行する。 */
void combatGameUpdate(void)
{
    buttonsUpdate();
    soundUpdate();
    matrixUpdate();

    switch(gState)
    {
        case GAME_WAIT:
            updateWait();
            break;

        case GAME_PLAY:
            updatePlay();
            break;

        case GAME_DEAD:
            updateDead();
            break;

        case GAME_NEXT_STAGE:
            updateNextStage();
            break;

        case GAME_STAGE_WAIT:
            updateStageWait();
            break;

        case GAME_OVER:
            updateGameOver();
            break;

        default:
            enterState(GAME_WAIT);
            break;
    }
}
