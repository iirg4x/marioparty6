#include <dolphin/mtx/GeoTypes.h>

#define HU3D_CLUSTER_MAX 4
#define HU3D_MODEL_LLIGHT_MAX 8
#define HU3D_GLIGHT_MAX 8

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
typedef struct Hu3DParticleData_s HU3D_PARTICLE_DATA;

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
extern HU3D_MODELID lbl_1_bss_1E1E[2];

void fn_1_105D4(s16 index)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_1E1E[index]];
    HU3D_PARTICLE *particle = model->hookData;

    particle->dataCnt = 0;
}
