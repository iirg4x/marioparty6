#define _MATH_H
#include "game/hu3d.h"

/* Consumed view: both grid constructors copy these five halfwords, then
 * pass signed indexed elements to Hu3DParticleCreate's s16 argument. */
typedef struct MDMinidllHalfwordTemplate {
    s16 values[5];
} MDMinidllHalfwordTemplate;

extern const MDMinidllHalfwordTemplate lbl_1_rodata_398;
extern const f32 lbl_1_rodata_288;
extern const f32 lbl_1_rodata_2A8;
extern ANIMDATA *lbl_1_bss_938[9];
extern HU3D_MODELID lbl_1_bss_88E[6][5];
void fn_1_24524(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);

void fn_1_24AD0(void)
{
    MDMinidllHalfwordTemplate localTemplate;
    s16 var_r31;
    s16 var_r30;

    localTemplate = lbl_1_rodata_398;
    for (var_r30 = 0; var_r30 < 6; var_r30 += 1) {
        for (var_r31 = 0; var_r31 < 5; var_r31 += 1) {
            lbl_1_bss_88E[var_r30][var_r31] = Hu3DParticleCreate(
                lbl_1_bss_938[var_r31 + 4],
                localTemplate.values[var_r31]);
            Hu3DModelPosSet(lbl_1_bss_88E[var_r30][var_r31],
                lbl_1_rodata_288, lbl_1_rodata_288, lbl_1_rodata_288);
            Hu3DModelScaleSet(lbl_1_bss_88E[var_r30][var_r31],
                lbl_1_rodata_2A8, lbl_1_rodata_2A8, lbl_1_rodata_2A8);
            Hu3DModelAttrSet(lbl_1_bss_88E[var_r30][var_r31], 1U);
            Hu3DModelLayerSet(lbl_1_bss_88E[var_r30][var_r31], 2);
            Hu3DParticleHookSet(lbl_1_bss_88E[var_r30][var_r31], fn_1_24524);
            Hu3DParticleBlendModeSet(lbl_1_bss_88E[var_r30][var_r31], 1U);
        }
    }
}
