#define _MATH_H
#include "game/main.h"
#include "game/data.h"
#include "game/memory.h"
#include "game/hu3d.h"
#include "game/sprite.h"

/* Existing target-backed views only; this TU defines no storage. */
extern ANIMDATA *lbl_1_bss_938[9];
extern s16 lbl_1_bss_8D2[4];
extern const f32 lbl_1_rodata_288;
extern const f32 lbl_1_rodata_2A8;
void fn_1_221E0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);

void fn_1_22AC8(void)
{
    s16 var_r31;

    var_r31 = 0;
    while (var_r31 < 4) {
        lbl_1_bss_8D2[var_r31] = Hu3DParticleCreate(lbl_1_bss_938[2], 64);
        Hu3DModelPosSet(lbl_1_bss_8D2[var_r31], lbl_1_rodata_288, lbl_1_rodata_288, lbl_1_rodata_288);
        Hu3DModelScaleSet(lbl_1_bss_8D2[var_r31], lbl_1_rodata_2A8, lbl_1_rodata_2A8, lbl_1_rodata_2A8);
        Hu3DModelLayerSet(lbl_1_bss_8D2[var_r31], 7);
        Hu3DModelAttrSet(lbl_1_bss_8D2[var_r31], 1U);
        Hu3DParticleScaleSet(lbl_1_bss_8D2[var_r31], lbl_1_rodata_2A8);
        Hu3DParticleHookSet(lbl_1_bss_8D2[var_r31], fn_1_221E0);
        Hu3DParticleBlendModeSet(lbl_1_bss_8D2[var_r31], 1U);
        var_r31 += 1;
    }
}
