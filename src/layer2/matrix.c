#include "layer2/matrix.h"

#include "layer2/display_mux.h"
#include "util/cell.h"
#include "util/timer.h"

#include <stdbool.h>
#include <stdint.h>

static Cell gPlayer = 0;
static Cell gEnemy = 19;

static bool gEnabled = true;
static bool gEnemyBlinkEnabled = false;
static bool gEnemyBlinkVisible = true;

static uint8_t gRowBuffer[4];
static uint8_t gDimRowBuffer[4];

static uint32_t gBlinkTimer = 0;

/* セル位置に対応する行バッファの列ビットを立てる。 */
static void drawCell(uint8_t buffer[4], Cell cell)
{
    uint8_t x;
    uint8_t y;

    if(!cellValid(cell))
    {
        return;
    }

    x = cellX(cell);
    y = cellY(cell);

    buffer[y] |= (uint8_t)(1 << x);
}

/* LEDマトリクスと明暗2種類の行バッファを初期化する。 */
void matrixInit(void)
{
    uint8_t i;

    displayMuxInit();

    for(i = 0; i < 4; i++)
    {
        gRowBuffer[i] = 0;
        gDimRowBuffer[i] = 0;
        displayMuxSetRow(i, 0);
        displayMuxSetDimRow(i, 0);
    }
}

/* マトリクス表示を無効化し、全行を消灯する。 */
void matrixClear(void)
{
    uint8_t i;

    gEnabled = false;

    for(i = 0; i < 4; i++)
    {
        gRowBuffer[i] = 0;
        gDimRowBuffer[i] = 0;
        displayMuxSetRow(i, 0);
        displayMuxSetDimRow(i, 0);
    }
}

/* プレイヤー位置を更新し、マトリクス表示を有効にする。 */
void matrixSetPlayer(Cell cell)
{
    gPlayer = cell;
    gEnabled = true;
}

/* 敵位置を更新し、マトリクス表示を有効にする。 */
void matrixSetEnemy(Cell cell)
{
    gEnemy = cell;
    gEnabled = true;
}

/* 待機画面用の敵点滅を切り替え、解除時は必ず表示状態へ戻す。 */
void matrixEnemyBlink(bool enable)
{
    gEnemyBlinkEnabled = enable;

    if(!enable)
    {
        gEnemyBlinkVisible = true;
    }
}

/* 点滅時刻を処理し、プレイヤー用と敵用の行バッファを再構築する。 */
void matrixUpdate(void)
{
    uint8_t i;

    if(timerElapsed(&gBlinkTimer, 500))
    {
        if(gEnemyBlinkEnabled)
        {
            gEnemyBlinkVisible = !gEnemyBlinkVisible;
        }

    }

    for(i = 0; i < 4; i++)
    {
        gRowBuffer[i] = 0;
        gDimRowBuffer[i] = 0;
    }

    if(!gEnabled)
    {
        for(i = 0; i < 4; i++)
        {
            displayMuxSetRow(i, 0);
            displayMuxSetDimRow(i, 0);
        }

        return;
    }

    /*
     * Keep bright and dim pixels separate. The scan ISR applies the enemy's
     * one-eighth PWM at a fixed rate, independent of main-loop timing.
     */
    if(gEnemyBlinkVisible)
    {
        drawCell(gDimRowBuffer, gEnemy);
    }

    drawCell(gRowBuffer, gPlayer);

    for(i = 0; i < 4; i++)
    {
        displayMuxSetRow(i, gRowBuffer[i]);
        displayMuxSetDimRow(i, gDimRowBuffer[i]);
    }
}
