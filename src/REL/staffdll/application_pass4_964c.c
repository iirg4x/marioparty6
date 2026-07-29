#include <dolphin/mtx/GeoTypes.h>

#define HU_MEMNUM_OVL 0x10000000
#define HU3D_ATTR_DISPOFF (1 << 0)

typedef s16 HU3D_MODELID;

typedef enum HeapID_s {
    HEAP_HEAP,
    HEAP_SOUND,
    HEAP_MODEL,
    HEAP_DVD,
    HEAP_SPACE,
    HEAP_MAX
} HEAPID;

typedef struct AnimData_s ANIMDATA;
typedef struct Hu3DModel_s HU3D_MODEL;
typedef struct Hu3DParticle_s HU3D_PARTICLE;
typedef void (*HU3D_PARTICLE_HOOK)(
    HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

extern const float lbl_1_rodata_10;
extern const float lbl_1_rodata_40;

extern s32 lbl_1_data_9F8[2];

extern HU3D_MODELID lbl_1_bss_36[2];
extern HU3D_MODELID lbl_1_bss_3A[2];
extern HU3D_MODELID lbl_1_bss_3E;
extern HU3D_MODELID lbl_1_bss_40[2];
extern HU3D_MODELID lbl_1_bss_44[2];
extern ANIMDATA *lbl_1_bss_48[2];

void *HuDataSelHeapReadNum(int dataNum, s32 num, HEAPID heap);
ANIMDATA *HuSprAnimRead(void *data);
HU3D_MODELID Hu3DParticleCreate(ANIMDATA *animation, s16 maxCount);
void Hu3DModelKill(HU3D_MODELID modelId);
void Hu3DModelPosSet(
    HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelScaleSet(
    HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelAttrSet(HU3D_MODELID modelId, u32 attr);
void Hu3DModelLayerSet(HU3D_MODELID modelId, s16 layer);
void Hu3DParticleHookSet(
    HU3D_MODELID modelId, HU3D_PARTICLE_HOOK hook);
void Hu3DParticleBlendModeSet(HU3D_MODELID modelId, s16 mode);
void fn_1_6E1C(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_7670(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_7E34(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_8618(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_8FC4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

static inline void fn_1_72A4(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_44[i] = Hu3DParticleCreate(lbl_1_bss_48[0], 10);
        Hu3DModelPosSet(lbl_1_bss_44[i], lbl_1_rodata_10,
            lbl_1_rodata_10, lbl_1_rodata_10);
        Hu3DModelScaleSet(lbl_1_bss_44[i], lbl_1_rodata_40,
            lbl_1_rodata_40, lbl_1_rodata_40);
        Hu3DModelAttrSet(lbl_1_bss_44[i], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_44[i], 2);
        Hu3DParticleHookSet(lbl_1_bss_44[i], fn_1_6E1C);
        Hu3DParticleBlendModeSet(lbl_1_bss_44[i], 1);
    }
}

static inline void fn_1_7C70(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_40[i] = Hu3DParticleCreate(lbl_1_bss_48[1], 0x80);
        Hu3DModelPosSet(lbl_1_bss_40[i], lbl_1_rodata_10,
            lbl_1_rodata_10, lbl_1_rodata_10);
        Hu3DModelScaleSet(lbl_1_bss_40[i], lbl_1_rodata_40,
            lbl_1_rodata_40, lbl_1_rodata_40);
        Hu3DModelAttrSet(lbl_1_bss_40[i], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_40[i], 2);
        Hu3DParticleHookSet(lbl_1_bss_40[i], fn_1_7670);
        Hu3DParticleBlendModeSet(lbl_1_bss_40[i], 1);
    }
}

static inline void fn_1_8300(void)
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

static inline void fn_1_8BF8(void)
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

static inline void fn_1_94E0(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_36[i] = Hu3DParticleCreate(lbl_1_bss_48[1], 0x80);
        Hu3DModelPosSet(lbl_1_bss_36[i], lbl_1_rodata_10,
            lbl_1_rodata_10, lbl_1_rodata_10);
        Hu3DModelScaleSet(lbl_1_bss_36[i], lbl_1_rodata_40,
            lbl_1_rodata_40, lbl_1_rodata_40);
        Hu3DModelAttrSet(lbl_1_bss_36[i], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_36[i], 2);
        Hu3DParticleHookSet(lbl_1_bss_36[i], fn_1_8FC4);
        Hu3DParticleBlendModeSet(lbl_1_bss_36[i], 1);
    }
}

void fn_1_964C(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        Hu3DModelKill(lbl_1_bss_36[i]);
    }
}

void fn_1_96A4(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_48[i] = HuSprAnimRead(HuDataSelHeapReadNum(
            lbl_1_data_9F8[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    fn_1_72A4();
    fn_1_7C70();
    fn_1_8300();
    fn_1_8BF8();
    fn_1_94E0();
}

void fn_1_9D14(void)
{
    s16 i;
    s16 j;
    s16 k;
    s16 l;

    for (i = 0; i < 2; i++) {
        Hu3DModelKill(lbl_1_bss_44[i]);
    }
    for (j = 0; j < 2; j++) {
        Hu3DModelKill(lbl_1_bss_40[j]);
    }
    Hu3DModelKill(lbl_1_bss_3E);
    for (k = 0; k < 2; k++) {
        Hu3DModelKill(lbl_1_bss_3A[k]);
    }
    for (l = 0; l < 2; l++) {
        Hu3DModelKill(lbl_1_bss_36[l]);
    }
}
