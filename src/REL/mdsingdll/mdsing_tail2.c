#include "game/hu3d.h"

extern HU3D_MODELID lbl_1_bss_1426;
extern ANIMDATA *lbl_1_bss_1478;
extern const float lbl_1_rodata_398;
extern const float lbl_1_rodata_3BC;

void fn_1_2E7DC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);

void fn_1_2EA9C(void)
{
    lbl_1_bss_1426 = Hu3DParticleCreate(lbl_1_bss_1478, 8);
    Hu3DModelPosSet(
        lbl_1_bss_1426, lbl_1_rodata_398, lbl_1_rodata_398,
        lbl_1_rodata_398);
    Hu3DModelScaleSet(
        lbl_1_bss_1426, lbl_1_rodata_3BC, lbl_1_rodata_3BC,
        lbl_1_rodata_3BC);
    Hu3DModelLayerSet(lbl_1_bss_1426, 7);
    Hu3DModelAttrSet(lbl_1_bss_1426, HU3D_ATTR_DISPOFF);
    Hu3DParticleScaleSet(lbl_1_bss_1426, lbl_1_rodata_3BC);
    Hu3DParticleHookSet(lbl_1_bss_1426, fn_1_2E7DC);
    Hu3DParticleBlendModeSet(
        lbl_1_bss_1426, HU3D_PARTICLE_BLEND_ADDCOL);
}

void fn_1_2EBB0(void)
{
    Hu3DModelKill(lbl_1_bss_1426);
}
