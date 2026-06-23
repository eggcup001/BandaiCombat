#include "util/cell.h"
#include "util/config.h"

/* 5x4盤面のXY座標を、0～19のセル番号へ変換する。 */
Cell cellFromXY(uint8_t x, uint8_t y)
{
    return (Cell)(y * MATRIX_WIDTH + x);
}

/* セル番号から横方向の座標を取り出す。 */
uint8_t cellX(Cell cell)
{
    return (uint8_t)(cell % MATRIX_WIDTH);
}

/* セル番号から縦方向の座標を取り出す。 */
uint8_t cellY(Cell cell)
{
    return (uint8_t)(cell / MATRIX_WIDTH);
}

/* セル番号が盤面内に収まっているかを調べる。 */
bool cellValid(Cell cell)
{
    return cell < MATRIX_CELLS;
}

/* 指定方向へ1マス移動し、盤外ならfalseを返してnextを変更しない。 */
bool cellMove(Cell current, Direction dir, Cell *next)
{
    uint8_t x;
    uint8_t y;

    if(next == 0 || !cellValid(current))
    {
        return false;
    }

    x = cellX(current);
    y = cellY(current);

    switch(dir)
    {
        case DIR_LEFT:
            if(x == 0) return false;
            x--;
            break;

        case DIR_RIGHT:
            if(x >= (MATRIX_WIDTH - 1)) return false;
            x++;
            break;

        case DIR_UP:
            if(y == 0) return false;
            y--;
            break;

        case DIR_DOWN:
            if(y >= (MATRIX_HEIGHT - 1)) return false;
            y++;
            break;

        default:
            return false;
    }

    *next = cellFromXY(x, y);
    return true;
}
