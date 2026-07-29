#include <dolphin/mtx/GeoTypes.h>

#define HU3D_ATTR_DISPOFF (1 << 0)

typedef s16 HU3D_MODELID;
typedef struct AnimData_s ANIMDATA;
typedef struct Hu3DModel_s HU3D_MODEL;
typedef struct Hu3DParticle_s HU3D_PARTICLE;
typedef void (*HU3D_PARTICLE_HOOK)(
    HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

typedef struct EndingParticleCounts {
    s16 count[5];
} EndingParticleCounts;

extern ANIMDATA *lbl_1_bss_1E30[3];
extern HU3D_MODELID lbl_1_bss_1DE0[6][5];
extern float lbl_1_rodata_2F8;
extern float lbl_1_rodata_318;
extern EndingParticleCounts lbl_1_rodata_390;

HU3D_MODELID Hu3DParticleCreate(ANIMDATA *animation, s16 maxCount);
void Hu3DModelPosSet(HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelScaleSet(HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelAttrSet(HU3D_MODELID modelId, u32 attr);
void Hu3DModelLayerSet(HU3D_MODELID modelId, s16 layer);
void Hu3DParticleHookSet(HU3D_MODELID modelId, HU3D_PARTICLE_HOOK hook);
void Hu3DParticleBlendModeSet(HU3D_MODELID modelId, u8 blendMode);
void fn_1_11714(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

void fn_1_11C94(void)
{
    EndingParticleCounts counts = lbl_1_rodata_390;
    s16 group;
    s16 model;

    for (group = 0; group < 6; group++) {
        for (model = 0; model < 5; model++) {
            lbl_1_bss_1DE0[group][model] = Hu3DParticleCreate(
                lbl_1_bss_1E30[2], counts.count[model]);
            Hu3DModelPosSet(lbl_1_bss_1DE0[group][model],
                lbl_1_rodata_2F8, lbl_1_rodata_2F8, lbl_1_rodata_2F8);
            Hu3DModelScaleSet(lbl_1_bss_1DE0[group][model],
                lbl_1_rodata_318, lbl_1_rodata_318, lbl_1_rodata_318);
            Hu3DModelAttrSet(lbl_1_bss_1DE0[group][model],
                HU3D_ATTR_DISPOFF);
            Hu3DModelLayerSet(lbl_1_bss_1DE0[group][model], 2);
            Hu3DParticleHookSet(lbl_1_bss_1DE0[group][model], fn_1_11714);
            Hu3DParticleBlendModeSet(lbl_1_bss_1DE0[group][model], 1);
        }
    }
}
