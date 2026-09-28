#define _MATH_H
#include "game/hu3d.h"

extern ANIMDATA *lbl_1_bss_938[9];
extern HU3D_MODELID lbl_1_bss_8DA;
extern const f32 lbl_1_rodata_288;
extern const f32 lbl_1_rodata_360;
extern const f32 lbl_1_rodata_2A8;
void fn_1_2185C(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);

void fn_1_21FE8(void)
{
    lbl_1_bss_8DA = Hu3DParticleCreate(lbl_1_bss_938[2], 1000);
    Hu3DModelPosSet(lbl_1_bss_8DA,
                    lbl_1_rodata_288,
                    lbl_1_rodata_360,
                    lbl_1_rodata_288);
    Hu3DModelScaleSet(lbl_1_bss_8DA,
                      lbl_1_rodata_2A8,
                      lbl_1_rodata_2A8,
                      lbl_1_rodata_2A8);
    Hu3DModelLayerSet(lbl_1_bss_8DA, 7);
    Hu3DModelAttrSet(lbl_1_bss_8DA, 1U);
    Hu3DParticleScaleSet(lbl_1_bss_8DA, lbl_1_rodata_2A8);
    Hu3DParticleHookSet(lbl_1_bss_8DA, fn_1_2185C);
}
