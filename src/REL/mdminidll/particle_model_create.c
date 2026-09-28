#define _MATH_H
#include "game/hu3d.h"

extern ANIMDATA *lbl_1_bss_938[];
extern HU3D_MODELID lbl_1_bss_8E4;
extern const f32 lbl_1_rodata_288;
extern const f32 lbl_1_rodata_2A8;
extern void fn_1_20B74(HU3D_MODEL *modelP, HU3D_PARTICLE *particleP, Mtx mtx);

void fn_1_20E34(void)
{
    lbl_1_bss_8E4 = Hu3DParticleCreate(lbl_1_bss_938[0], 8);
    Hu3DModelPosSet(lbl_1_bss_8E4, lbl_1_rodata_288, lbl_1_rodata_288,
                    lbl_1_rodata_288);
    Hu3DModelScaleSet(lbl_1_bss_8E4, lbl_1_rodata_2A8, lbl_1_rodata_2A8,
                      lbl_1_rodata_2A8);
    Hu3DModelLayerSet(lbl_1_bss_8E4, 7);
    Hu3DModelAttrSet(lbl_1_bss_8E4, 1U);
    Hu3DParticleScaleSet(lbl_1_bss_8E4, lbl_1_rodata_2A8);
    Hu3DParticleHookSet(lbl_1_bss_8E4, fn_1_20B74);
    Hu3DParticleBlendModeSet(lbl_1_bss_8E4, 1U);
}
