#include "game/hu3d.h"

extern HU3D_MODELID lbl_1_bss_13CE[][5];

void fn_1_32988(s16 groupNo)
{
    s16 i;
    HU3D_MODEL *model;
    s16 *work;

    for (i = 0; i < 5; i++) {
        model = &Hu3DData[lbl_1_bss_13CE[groupNo][i]];
        work = model->hookData;
        *work = 0;
    }
}
