#include "game/hu3d.h"

extern HU3D_MODELID lbl_1_bss_13CE[][5];

void fn_1_331E0(void)
{
    s16 i;
    s16 j;

    for (i = 0; i < 6; i++) {
        for (j = 0; j < 5; j++) {
            Hu3DModelKill(lbl_1_bss_13CE[i][j]);
        }
    }
}
