#include <dolphin/mtx/GeoTypes.h>

#define HU3D_ATTR_DISPOFF (1 << 0)

typedef Vec HuVecF;
typedef s16 HU3D_MODELID;
typedef struct AnimData_s ANIMDATA;
typedef struct Hu3DModel_s HU3D_MODEL;
typedef struct Hu3DParticle_s HU3D_PARTICLE;
typedef void (*HU3D_PARTICLE_HOOK)(
    HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

extern ANIMDATA *lbl_1_bss_48[2];
extern HU3D_MODELID lbl_1_bss_3A[2];
extern const float lbl_1_rodata_10;
extern const float lbl_1_rodata_40;

HU3D_MODELID Hu3DParticleCreate(ANIMDATA *animation, s16 maxCount);
void Hu3DModelPosSet(
    HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelScaleSet(
    HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelAttrSet(HU3D_MODELID modelId, u32 attr);
void Hu3DModelLayerSet(HU3D_MODELID modelId, s16 layer);
void Hu3DParticleHookSet(
    HU3D_MODELID modelId, HU3D_PARTICLE_HOOK hook);
void Hu3DParticleBlendModeSet(HU3D_MODELID modelId, s16 mode);
void fn_1_8618(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

void fn_1_8BF8(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_3A[i] = Hu3DParticleCreate(lbl_1_bss_48[1], 0x80);
        Hu3DModelPosSet(lbl_1_bss_3A[i], lbl_1_rodata_10,
            lbl_1_rodata_10, lbl_1_rodata_10);
        Hu3DModelScaleSet(lbl_1_bss_3A[i], lbl_1_rodata_40,
            lbl_1_rodata_40, lbl_1_rodata_40);
        Hu3DModelAttrSet(lbl_1_bss_3A[i], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_3A[i], 2);
        Hu3DParticleHookSet(lbl_1_bss_3A[i], fn_1_8618);
        Hu3DParticleBlendModeSet(lbl_1_bss_3A[i], 1);
    }
}
