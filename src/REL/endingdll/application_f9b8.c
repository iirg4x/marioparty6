#include <dolphin/mtx/GeoTypes.h>

#define HU3D_ATTR_DISPOFF (1 << 0)

typedef Vec HuVecF;
typedef s16 HU3D_MODELID;
typedef struct AnimData_s ANIMDATA;
typedef struct Hu3DModel_s HU3D_MODEL;
typedef struct Hu3DParticle_s HU3D_PARTICLE;
typedef void (*HU3D_PARTICLE_HOOK)(
    HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

extern ANIMDATA *lbl_1_bss_1E30[3];
extern HU3D_MODELID lbl_1_bss_1E26[2];
extern float lbl_1_rodata_2F8;
extern float lbl_1_rodata_318;

HU3D_MODELID Hu3DParticleCreate(ANIMDATA *animation, s16 maxCount);
void Hu3DModelPosSet(HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelPosSetV(HU3D_MODELID modelId, HuVecF *position);
void Hu3DModelScaleSet(HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelLayerSet(HU3D_MODELID modelId, s16 layer);
void Hu3DParticleHookSet(HU3D_MODELID modelId, HU3D_PARTICLE_HOOK hook);
void Hu3DParticleBlendModeSet(HU3D_MODELID modelId, u8 blendMode);
void Hu3DModelAttrSet(HU3D_MODELID modelId, u32 attr);
void Hu3DModelAttrReset(HU3D_MODELID modelId, u32 attr);
void fn_1_F2CC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

static inline void fn_1_F068(s16 index, s16 display, HuVecF *position)
{
    if (position != NULL) {
        Hu3DModelPosSetV(lbl_1_bss_1E26[index], position);
    }
    if (display == 0) {
        Hu3DModelAttrSet(lbl_1_bss_1E26[index], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrReset(lbl_1_bss_1E26[index], HU3D_ATTR_DISPOFF);
    }
}

void fn_1_F9B8(void)
{
    s16 index;

    for (index = 0; index < 2; index++) {
        lbl_1_bss_1E26[index] = Hu3DParticleCreate(lbl_1_bss_1E30[0], 360);
        Hu3DModelPosSet(lbl_1_bss_1E26[index], lbl_1_rodata_2F8,
            lbl_1_rodata_2F8, lbl_1_rodata_2F8);
        Hu3DModelScaleSet(lbl_1_bss_1E26[index], lbl_1_rodata_318,
            lbl_1_rodata_318, lbl_1_rodata_318);
        Hu3DModelLayerSet(lbl_1_bss_1E26[index], 6);
        Hu3DParticleHookSet(lbl_1_bss_1E26[index], fn_1_F2CC);
        Hu3DParticleBlendModeSet(lbl_1_bss_1E26[index], 1);
    }
    fn_1_F068(0, 0, NULL);
    fn_1_F068(1, 0, NULL);
}
