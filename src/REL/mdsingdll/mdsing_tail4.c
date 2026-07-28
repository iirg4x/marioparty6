#include "game/hu3d.h"

extern HU3D_MODELID lbl_1_bss_141C;
extern HU3D_MODELID lbl_1_bss_141E[];
extern const float lbl_1_rodata_398;

void fn_1_2F3CC(void)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        Hu3DModelKill(lbl_1_bss_141E[i]);
    }
}

void fn_1_2F424(void)
{
    s16 i;
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    model = &Hu3DData[lbl_1_bss_141C];
    particle = model->hookData;
    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        data->time = 0;
        data->scale = lbl_1_rodata_398;
    }
    particle->dataCnt = 1;
    model->attr &= ~HU3D_ATTR_DISPOFF;
}
