#include "layer3/combat_game.h"

/*
 * main intentionally depends only on Layer3.
 * All hardware details are hidden below the game layer.
 */
/* 各レイヤを初期化し、ゲーム更新を繰り返すファームウェアの入口。 */
int main(void)
{
    combatGameInit();

    while(1)
    {
        combatGameUpdate();
    }
}
