#include "game/hu3d.h"

extern HU3D_MODELID lbl_1_bss_140A[];
extern ANIMDATA *lbl_1_bss_1478[];
extern const float lbl_1_rodata_398;
extern const float lbl_1_rodata_3BC;

void fn_1_31BE4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

void fn_1_321E4(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_140A[i] = Hu3DParticleCreate(lbl_1_bss_1478[3], 256);
        Hu3DModelPosSet(
            lbl_1_bss_140A[i], lbl_1_rodata_398, lbl_1_rodata_398,
            lbl_1_rodata_398);
        Hu3DModelScaleSet(
            lbl_1_bss_140A[i], lbl_1_rodata_3BC, lbl_1_rodata_3BC,
            lbl_1_rodata_3BC);
        Hu3DModelAttrSet(lbl_1_bss_140A[i], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_140A[i], 2);
        Hu3DParticleHookSet(lbl_1_bss_140A[i], fn_1_31BE4);
        Hu3DParticleBlendModeSet(
            lbl_1_bss_140A[i], HU3D_PARTICLE_BLEND_ADDCOL);
    }
}

void fn_1_32350(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        Hu3DModelKill(lbl_1_bss_140A[i]);
    }
}
