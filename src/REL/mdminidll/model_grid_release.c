#define _MATH_H
#include "game/hu3d.h"

extern s16 lbl_1_bss_88E[6][5];

void fn_1_24CFC(void)
{
    s16 row;
    s16 column;

    for (row = 0; row < 6; row++) {
        column = 0;
        while (column < 5) {
            Hu3DModelKill(lbl_1_bss_88E[row][column]);
            column += 1;
        }
    }
}
