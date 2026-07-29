#include <dolphin/gx/GXStruct.h>
#include <dolphin/mtx/GeoTypes.h>

#define HU3D_CLUSTER_MAX 4
#define HU3D_MODEL_LLIGHT_MAX 8
#define HU3D_GLIGHT_MAX 8
#define HU3D_ATTR_DISPOFF (1 << 0)

typedef Vec HuVecF;
typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;
typedef s16 HU3D_LIGHTID;
typedef s16 HU3D_PARMANID;

typedef struct AnimData_s ANIMDATA;
typedef struct HsfData_s HSF_DATA;
typedef struct HsfMaterial_s HSF_MATERIAL;
typedef struct HsfObject_s HSF_OBJECT;
typedef struct Hu3DDrawObj_s HU3D_DRAW_OBJ;
typedef struct Hu3DModel_s HU3D_MODEL;
typedef struct Hu3DParticle_s HU3D_PARTICLE;

typedef void (*HU3D_MODEL_HOOK)(HU3D_MODEL *model, Mtx *matrix);
typedef void (*HU3D_TIMING_HOOK)(
    HU3D_MODELID modelId, HU3D_MOTIONID motionId, BOOL lag);
typedef void (*HU3D_MAT_HOOK)(
    HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material);
typedef void (*HU3D_PARTICLE_HOOK)(
    HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

typedef struct Hu3DMotWork_s {
    float time;
    float speed;
    float start;
    float end;
} HU3D_MOTWORK;

struct Hu3DModel_s {
    u8 tick;
    u8 camInfoBit;
    u8 projBit;
    u8 hiliteIdx;
    s8 reflectType;
    u8 lightBit;
    s16 layerNo;
    HU3D_MOTIONID motId;
    HU3D_MOTIONID motIdOvl;
    HU3D_MOTIONID motIdShift;
    HU3D_MOTIONID motIdShape;
    HU3D_MOTIONID motIdCluster[HU3D_CLUSTER_MAX];
    s16 clusterAttr[HU3D_CLUSTER_MAX];
    HU3D_MOTIONID motIdSrc;
    u16 cameraBit;
    HU3D_MODELID linkMdlId;
    u16 lightNum;
    u16 lightId[HU3D_GLIGHT_MAX];
    HU3D_LIGHTID LLightId[HU3D_MODEL_LLIGHT_MAX];
    u32 mallocNo;
    u32 mallocNoLink;
    u32 attr;
    u32 motAttr;
    float ambR;
    float ambB;
    float ambG;
    HU3D_MOTWORK motWork;
    HU3D_MOTWORK motOvlWork;
    HU3D_MOTWORK motShiftWork;
    HU3D_MOTWORK motShapeWork;
    float clusterTime[HU3D_CLUSTER_MAX];
    float clusterSpeed[HU3D_CLUSTER_MAX];
    union {
        HSF_DATA *hsf;
        HU3D_MODEL_HOOK hookFunc;
    };
    HSF_DATA *hsfLink;
    HuVecF pos;
    HuVecF rot;
    HuVecF scale;
    Mtx mtx;
    void *hookData;
    HU3D_TIMING_HOOK timingHook;
    HSF_OBJECT *timingHookObj;
    HU3D_MAT_HOOK matHook;
    u32 endCounter;
};

typedef struct Hu3DParticleData_s {
    s16 time;
    HU3D_PARMANID parManId;
    s16 attr;
    s16 cameraBit;
    HuVecF vel;
    HuVecF accel;
    float speedDecay;
    float colorIdx;
    float scaleBase;
    float scale;
    float scaleY;
    float zRot;
    HuVecF pos;
    GXColor color;
} HU3D_PARTICLE_DATA;

struct Hu3DParticle_s {
    s16 dataCnt;
    s16 emitCnt;
    HuVecF pos;
    HuVecF unk_10;
    void *work;
    s16 animBank;
    s16 animNo;
    float animSpeed;
    float animTime;
    u8 blendMode;
    u8 attr;
    s16 unk_2E;
    s16 maxCnt;
    u32 count;
    u32 prevCounter;
    u32 prevCount;
    u32 dlSize;
    ANIMDATA *anim;
    HU3D_PARTICLE_DATA *data;
    HuVecF *vtxBuf;
    void *dlBuf;
    HU3D_PARTICLE_HOOK hook;
};

extern HU3D_MODEL *Hu3DData;

void Hu3DModelKill(HU3D_MODELID modelId);
void Hu3DModelPosSet(
    HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelPosSetV(HU3D_MODELID modelId, HuVecF *position);
void Hu3DModelAttrSet(HU3D_MODELID modelId, u32 attr);
void Hu3DModelAttrReset(HU3D_MODELID modelId, u32 attr);

extern HU3D_MODELID lbl_1_bss_1E26[2];
extern float lbl_1_rodata_2F8;

void fn_1_F11C(s16 index, s16 count)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_1E26[index]];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data;
    s16 i;

    particle->dataCnt = count;
    i = 0;
    data = particle->data;
    while (i < particle->maxCnt) {
        data->time = 0;
        data->scale = lbl_1_rodata_2F8;
        i++;
        data++;
    }
}

void fn_1_F1B8(s16 display, HuVecF *pos)
{
    if (pos != NULL) {
        Hu3DModelPosSetV(lbl_1_bss_1E26[0], pos);
    }
    if (display == 0) {
        Hu3DModelAttrSet(lbl_1_bss_1E26[0], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrReset(lbl_1_bss_1E26[0], HU3D_ATTR_DISPOFF);
    }
}

void fn_1_F23C(s16 count)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_1E26[0]];
    HU3D_PARTICLE_DATA *data;
    HU3D_PARTICLE *particle = model->hookData;
    s16 i;

    particle->dataCnt = count;
    i = 0;
    data = particle->data;
    while (i < particle->maxCnt) {
        data->time = 0;
        data->scale = lbl_1_rodata_2F8;
        i++;
        data++;
    }
}
