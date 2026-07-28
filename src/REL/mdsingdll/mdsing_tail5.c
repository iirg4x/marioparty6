#include "game/hu3d.h"

extern HU3D_MODELID lbl_1_bss_1414[];
extern HU3D_MODELID lbl_1_bss_141C;
extern ANIMDATA *lbl_1_bss_1478[];
extern const float lbl_1_rodata_398;
extern const float lbl_1_rodata_3BC;
extern const float lbl_1_rodata_470;

void fn_1_2F4C4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

void fn_1_2FC50(void)
{
    lbl_1_bss_141C = Hu3DParticleCreate(lbl_1_bss_1478[2], 1000);
    Hu3DModelPosSet(
        lbl_1_bss_141C, lbl_1_rodata_398, lbl_1_rodata_470,
        lbl_1_rodata_398);
    Hu3DModelScaleSet(
        lbl_1_bss_141C, lbl_1_rodata_3BC, lbl_1_rodata_3BC,
        lbl_1_rodata_3BC);
    Hu3DModelLayerSet(lbl_1_bss_141C, 7);
    Hu3DModelAttrSet(lbl_1_bss_141C, HU3D_ATTR_DISPOFF);
    Hu3DParticleScaleSet(lbl_1_bss_141C, lbl_1_rodata_3BC);
    Hu3DParticleHookSet(lbl_1_bss_141C, fn_1_2F4C4);
}

void fn_1_2FD50(void)
{
    Hu3DModelKill(lbl_1_bss_141C);
}

void fn_1_2FD7C(s16 index, float particleId, HuVecF *position)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    model = &Hu3DData[lbl_1_bss_1414[index]];
    particle = model->hookData;
    Hu3DModelPosSetV(lbl_1_bss_1414[index], position);
    data = particle->data;
    data->time = 0;
    data->parManId = particleId;
    particle->dataCnt = 1;
    model->attr &= ~HU3D_ATTR_DISPOFF;
}
