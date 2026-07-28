#include "game/hu3d.h"

extern HU3D_MODELID lbl_1_bss_140A[];

void fn_1_31B90(s16 index)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;

    model = &Hu3DData[lbl_1_bss_140A[index]];
    particle = model->hookData;
    particle->dataCnt = 0;
}
