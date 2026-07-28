#include "game/hu3d.h"

extern HU3D_MODELID lbl_1_bss_141E[];

void fn_1_2EBDC(s16 index, float particleId, HuVecF *position, GXColor color)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    model = &Hu3DData[lbl_1_bss_141E[index]];
    particle = model->hookData;
    Hu3DModelPosSetV(lbl_1_bss_141E[index], position);
    data = particle->data;
    data->time = 1;
    data->color.r = color.r;
    data->color.g = color.g;
    data->color.b = color.b;
    data->color.a = color.a;
    data->time = 0;
    data->parManId = particleId;
    particle->dataCnt = 1;
    model->attr &= ~HU3D_ATTR_DISPOFF;
}
