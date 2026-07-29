#include <dolphin/mtx/GeoTypes.h>

typedef s16 HU3D_MODELID;
typedef struct AnimData_s ANIMDATA;
typedef struct Hu3DModel_s HU3D_MODEL;
typedef struct Hu3DParticle_s HU3D_PARTICLE;
typedef void (*HU3D_PARTICLE_HOOK)(
    HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

extern ANIMDATA *lbl_1_bss_48[2];
extern HU3D_MODELID lbl_1_bss_3E;
extern const float lbl_1_rodata_10;
extern const float lbl_1_rodata_40;

HU3D_MODELID Hu3DParticleCreate(ANIMDATA *animation, s16 maxCount);
void Hu3DModelPosSet(
    HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelScaleSet(
    HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelLayerSet(HU3D_MODELID modelId, s16 layer);
void Hu3DParticleHookSet(
    HU3D_MODELID modelId, HU3D_PARTICLE_HOOK hook);
void Hu3DParticleBlendModeSet(HU3D_MODELID modelId, s16 mode);
void fn_1_7E34(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

void fn_1_8300(void)
{
    lbl_1_bss_3E = Hu3DParticleCreate(lbl_1_bss_48[0], 0x100);
    Hu3DModelPosSet(lbl_1_bss_3E, lbl_1_rodata_10, lbl_1_rodata_10,
        lbl_1_rodata_10);
    Hu3DModelScaleSet(lbl_1_bss_3E, lbl_1_rodata_40, lbl_1_rodata_40,
        lbl_1_rodata_40);
    Hu3DModelLayerSet(lbl_1_bss_3E, 2);
    Hu3DParticleHookSet(lbl_1_bss_3E, fn_1_7E34);
    Hu3DParticleBlendModeSet(lbl_1_bss_3E, 1);
}
