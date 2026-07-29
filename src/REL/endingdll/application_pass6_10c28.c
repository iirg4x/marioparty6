#include <dolphin/mtx/GeoTypes.h>

#define HU3D_ATTR_DISPOFF (1 << 0)

typedef s16 HU3D_MODELID;
typedef struct AnimData_s ANIMDATA;
typedef struct Hu3DModel_s HU3D_MODEL;
typedef struct Hu3DParticle_s HU3D_PARTICLE;
typedef void (*HU3D_PARTICLE_HOOK)(
    HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

HU3D_MODELID Hu3DParticleCreate(ANIMDATA *anim, s16 maxCnt);
void Hu3DParticleHookSet(HU3D_MODELID modelId, HU3D_PARTICLE_HOOK hook);
void Hu3DParticleBlendModeSet(HU3D_MODELID modelId, u8 blendMode);
void Hu3DModelPosSet(
    HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelScaleSet(
    HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelAttrSet(HU3D_MODELID modelId, u32 attr);
void Hu3DModelLayerSet(HU3D_MODELID modelId, s16 layer);
void Hu3DModelKill(HU3D_MODELID modelId);

void fn_1_10628(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

extern ANIMDATA *lbl_1_bss_1E30[3];
extern HU3D_MODELID lbl_1_bss_1E1E[2];
extern float lbl_1_rodata_2F8;
extern float lbl_1_rodata_318;

void fn_1_10C28(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_1E1E[i] = Hu3DParticleCreate(lbl_1_bss_1E30[0], 128);
        Hu3DModelPosSet(lbl_1_bss_1E1E[i], lbl_1_rodata_2F8,
            lbl_1_rodata_2F8, lbl_1_rodata_2F8);
        Hu3DModelScaleSet(lbl_1_bss_1E1E[i], lbl_1_rodata_318,
            lbl_1_rodata_318, lbl_1_rodata_318);
        Hu3DModelAttrSet(lbl_1_bss_1E1E[i], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_1E1E[i], 2);
        Hu3DParticleHookSet(lbl_1_bss_1E1E[i], fn_1_10628);
        Hu3DParticleBlendModeSet(lbl_1_bss_1E1E[i], 1);
    }
}

void fn_1_10D94(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        Hu3DModelKill(lbl_1_bss_1E1E[i]);
    }
}
