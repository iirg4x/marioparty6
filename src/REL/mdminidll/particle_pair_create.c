#define _MATH_H
#include "game/hu3d.h"

extern s16 lbl_1_bss_8CE[];
extern ANIMDATA *lbl_1_bss_938[];
extern const f32 lbl_1_rodata_288;
extern const f32 lbl_1_rodata_2A8;
void fn_1_22EBC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);

void fn_1_23334(void)
{
    s16 var_r31;

    var_r31 = 0;
    while (var_r31 < 2) {
        lbl_1_bss_8CE[var_r31] = Hu3DParticleCreate(*lbl_1_bss_938, 10);
        Hu3DModelPosSet(lbl_1_bss_8CE[var_r31], lbl_1_rodata_288, lbl_1_rodata_288, lbl_1_rodata_288);
        Hu3DModelScaleSet(lbl_1_bss_8CE[var_r31], lbl_1_rodata_2A8, lbl_1_rodata_2A8, lbl_1_rodata_2A8);
        Hu3DModelAttrSet(lbl_1_bss_8CE[var_r31], 1U);
        Hu3DModelLayerSet(lbl_1_bss_8CE[var_r31], 2);
        Hu3DParticleHookSet(lbl_1_bss_8CE[var_r31], fn_1_22EBC);
        Hu3DParticleBlendModeSet(lbl_1_bss_8CE[var_r31], 1U);
        var_r31 += 1;
    }
}
