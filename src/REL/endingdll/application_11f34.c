#include <dolphin/mtx/GeoTypes.h>

#include "game/memory.h"

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
extern HU3D_MODELID lbl_1_bss_1E2C;
extern HU3D_MODELID lbl_1_bss_1E2A;
extern HU3D_MODELID lbl_1_bss_1E26[2];
extern HU3D_MODELID lbl_1_bss_1E1C;
extern HU3D_MODELID lbl_1_bss_1E22[2];
extern HU3D_MODELID lbl_1_bss_1E1E[2];
extern HU3D_MODELID lbl_1_bss_1DE0[6][5];

extern float lbl_1_rodata_2F8;
extern float lbl_1_rodata_318;
extern float lbl_1_rodata_340;
extern float lbl_1_rodata_350;
extern EndingParticleCounts lbl_1_rodata_390;
extern u32 lbl_1_data_128[3];

void *HuDataSelHeapReadNum(s32 dataNum, s32 num, HEAPID heap);
ANIMDATA *HuSprAnimRead(void *data);
HU3D_MODELID Hu3DParticleCreate(ANIMDATA *animation, s16 maxCount);
void Hu3DModelPosSet(HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelPosSetV(HU3D_MODELID modelId, Vec *position);
void Hu3DModelScaleSet(HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelLayerSet(HU3D_MODELID modelId, s16 layer);
void Hu3DModelAttrSet(HU3D_MODELID modelId, u32 attr);
void Hu3DModelAttrReset(HU3D_MODELID modelId, u32 attr);
void Hu3DParticleHookSet(HU3D_MODELID modelId, HU3D_PARTICLE_HOOK hook);
void Hu3DParticleBlendModeSet(HU3D_MODELID modelId, u8 blendMode);
void fn_1_E270(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_EBCC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_F2CC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_FDD4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_10628(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_112E0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_11714(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

static inline void fn_1_E9A8(void)
{
    lbl_1_bss_1E2C = Hu3DParticleCreate(lbl_1_bss_1E30[0], 16);
    Hu3DModelPosSet(lbl_1_bss_1E2C, lbl_1_rodata_2F8,
        lbl_1_rodata_2F8, lbl_1_rodata_2F8);
    Hu3DModelScaleSet(lbl_1_bss_1E2C, lbl_1_rodata_318,
        lbl_1_rodata_318, lbl_1_rodata_318);
    Hu3DModelLayerSet(lbl_1_bss_1E2C, 6);
    Hu3DParticleHookSet(lbl_1_bss_1E2C, fn_1_E270);
    Hu3DParticleBlendModeSet(lbl_1_bss_1E2C, 1);
}

static inline void fn_1_EF58(void)
{
    lbl_1_bss_1E2A = Hu3DParticleCreate(lbl_1_bss_1E30[1], 4);
    Hu3DModelPosSet(lbl_1_bss_1E2A, lbl_1_rodata_2F8,
        lbl_1_rodata_340, lbl_1_rodata_350);
    Hu3DModelScaleSet(lbl_1_bss_1E2A, lbl_1_rodata_318,
        lbl_1_rodata_318, lbl_1_rodata_318);
    Hu3DModelLayerSet(lbl_1_bss_1E2A, 6);
    Hu3DParticleHookSet(lbl_1_bss_1E2A, fn_1_EBCC);
    Hu3DParticleBlendModeSet(lbl_1_bss_1E2A, 1);
}

static inline void fn_1_F068(s16 index, s16 display, Vec *position)
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

static inline void fn_1_F9B8(void)
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

static inline void fn_1_1025C(void)
{
    s16 index;

    for (index = 0; index < 2; index++) {
        lbl_1_bss_1E22[index] = Hu3DParticleCreate(lbl_1_bss_1E30[1], 10);
        Hu3DModelPosSet(lbl_1_bss_1E22[index], lbl_1_rodata_2F8,
            lbl_1_rodata_2F8, lbl_1_rodata_2F8);
        Hu3DModelScaleSet(lbl_1_bss_1E22[index], lbl_1_rodata_318,
            lbl_1_rodata_318, lbl_1_rodata_318);
        Hu3DModelAttrSet(lbl_1_bss_1E22[index], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_1E22[index], 2);
        Hu3DParticleHookSet(lbl_1_bss_1E22[index], fn_1_FDD4);
        Hu3DParticleBlendModeSet(lbl_1_bss_1E22[index], 1);
    }
}

static inline void fn_1_10C28(void)
{
    s16 index;

    for (index = 0; index < 2; index++) {
        lbl_1_bss_1E1E[index] = Hu3DParticleCreate(lbl_1_bss_1E30[0], 128);
        Hu3DModelPosSet(lbl_1_bss_1E1E[index], lbl_1_rodata_2F8,
            lbl_1_rodata_2F8, lbl_1_rodata_2F8);
        Hu3DModelScaleSet(lbl_1_bss_1E1E[index], lbl_1_rodata_318,
            lbl_1_rodata_318, lbl_1_rodata_318);
        Hu3DModelAttrSet(lbl_1_bss_1E1E[index], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_1E1E[index], 2);
        Hu3DParticleHookSet(lbl_1_bss_1E1E[index], fn_1_10628);
        Hu3DParticleBlendModeSet(lbl_1_bss_1E1E[index], 1);
    }
}

static inline void fn_1_11368(void)
{
    lbl_1_bss_1E1C = Hu3DParticleCreate(lbl_1_bss_1E30[1], 100);
    Hu3DModelPosSet(lbl_1_bss_1E1C, lbl_1_rodata_2F8,
        lbl_1_rodata_2F8, lbl_1_rodata_2F8);
    Hu3DModelScaleSet(lbl_1_bss_1E1C, lbl_1_rodata_318,
        lbl_1_rodata_318, lbl_1_rodata_318);
    Hu3DModelLayerSet(lbl_1_bss_1E1C, 6);
    Hu3DParticleHookSet(lbl_1_bss_1E1C, fn_1_112E0);
    Hu3DParticleBlendModeSet(lbl_1_bss_1E1C, 1);
}

static inline void fn_1_11C94(void)
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

void fn_1_11F34(void)
{
    s16 index;

    for (index = 0; index < 3; index++) {
        lbl_1_bss_1E30[index] = HuSprAnimRead(HuDataSelHeapReadNum(
            lbl_1_data_128[index], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    fn_1_E9A8();
    fn_1_EF58();
    fn_1_F9B8();
    fn_1_11368();
    fn_1_1025C();
    fn_1_10C28();
    fn_1_11C94();
}
