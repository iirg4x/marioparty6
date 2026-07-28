#include "game/hu3d.h"

extern HU3D_MODELID lbl_1_bss_140A[];
extern HU3D_MODELID lbl_1_bss_140E[];
extern ANIMDATA *lbl_1_bss_1478[];
extern const float lbl_1_rodata_398;
extern const float lbl_1_rodata_3BC;

void fn_1_313A0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

void fn_1_31818(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_140E[i] = Hu3DParticleCreate(lbl_1_bss_1478[0], 10);
        Hu3DModelPosSet(
            lbl_1_bss_140E[i], lbl_1_rodata_398, lbl_1_rodata_398,
            lbl_1_rodata_398);
        Hu3DModelScaleSet(
            lbl_1_bss_140E[i], lbl_1_rodata_3BC, lbl_1_rodata_3BC,
            lbl_1_rodata_3BC);
        Hu3DModelAttrSet(lbl_1_bss_140E[i], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_140E[i], 2);
        Hu3DParticleHookSet(lbl_1_bss_140E[i], fn_1_313A0);
        Hu3DParticleBlendModeSet(
            lbl_1_bss_140E[i], HU3D_PARTICLE_BLEND_ADDCOL);
    }
}

void fn_1_31984(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        Hu3DModelKill(lbl_1_bss_140E[i]);
    }
}

void fn_1_319DC(s16 index, s16 show)
{
    if (show) {
        Hu3DModelAttrReset(lbl_1_bss_140A[index], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_140A[index], HU3D_ATTR_DISPOFF);
    }
}
