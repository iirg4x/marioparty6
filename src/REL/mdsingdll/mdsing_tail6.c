#include "game/hu3d.h"

extern HU3D_MODELID lbl_1_bss_140E[];

void fn_1_31198(s16 index, s16 show)
{
    if (show) {
        Hu3DModelAttrReset(lbl_1_bss_140E[index], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_140E[index], HU3D_ATTR_DISPOFF);
    }
}

void fn_1_31214(s16 index, HuVecF *position, GXColor *color)
{
    s16 i;
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    model = &Hu3DData[lbl_1_bss_140E[index]];
    particle = model->hookData;
    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        data->time = 1;
        if (color != NULL) {
            data->color.r = color->r;
            data->color.g = color->g;
            data->color.b = color->b;
        }
    }
    if (position != NULL) {
        Hu3DModelPosSetV(lbl_1_bss_140E[index], position);
    }
    Hu3DModelAttrReset(lbl_1_bss_140E[index], HU3D_ATTR_DISPOFF);
}

void fn_1_31318(s16 index)
{
    s16 i;
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    model = &Hu3DData[lbl_1_bss_140E[index]];
    particle = model->hookData;
    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        data->time = 2;
    }
}
