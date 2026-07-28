#include "game/hu3d.h"

extern HU3D_MODELID lbl_1_bss_1414[];
extern ANIMDATA *lbl_1_bss_1478[];
extern const float lbl_1_rodata_398;
extern const float lbl_1_rodata_3BC;

void fn_1_2FE48(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

void fn_1_30730(void)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        lbl_1_bss_1414[i] = Hu3DParticleCreate(lbl_1_bss_1478[2], 64);
        Hu3DModelPosSet(
            lbl_1_bss_1414[i], lbl_1_rodata_398, lbl_1_rodata_398,
            lbl_1_rodata_398);
        Hu3DModelScaleSet(
            lbl_1_bss_1414[i], lbl_1_rodata_3BC, lbl_1_rodata_3BC,
            lbl_1_rodata_3BC);
        Hu3DModelLayerSet(lbl_1_bss_1414[i], 7);
        Hu3DModelAttrSet(lbl_1_bss_1414[i], HU3D_ATTR_DISPOFF);
        Hu3DParticleScaleSet(lbl_1_bss_1414[i], lbl_1_rodata_3BC);
        Hu3DParticleHookSet(lbl_1_bss_1414[i], fn_1_2FE48);
        Hu3DParticleBlendModeSet(
            lbl_1_bss_1414[i], HU3D_PARTICLE_BLEND_ADDCOL);
    }
}

void fn_1_308C4(void)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        Hu3DModelKill(lbl_1_bss_1414[i]);
    }
}
