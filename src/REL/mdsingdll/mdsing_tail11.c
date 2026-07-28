#include "game/hu3d.h"

extern HU3D_MODELID lbl_1_bss_13CE[][5];

void fn_1_3276C(s16 groupNo, s16 show)
{
    s16 i;

    for (i = 0; i < 5; i++) {
        if (show) {
            Hu3DModelAttrReset(
                lbl_1_bss_13CE[groupNo][i], HU3D_ATTR_DISPOFF);
        } else {
            Hu3DModelAttrSet(
                lbl_1_bss_13CE[groupNo][i], HU3D_ATTR_DISPOFF);
        }
    }
}
