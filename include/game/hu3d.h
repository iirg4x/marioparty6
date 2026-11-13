#ifndef _GAME_HSFMAN_H
#define _GAME_HSFMAN_H

#include "game/hsfformat.h"
#include "game/memory.h"
#include "game/data.h"

//Max for model properties

//Max for layer properties

//Max for global buffers

//Motion attributes

//Cluster Attributes

//Model attributes

//Texture Animation Attributes

//Attribute Animation Attributes

//Texture Scroll Attributes

//Particle Attributes

//Particle Manager Attributes

//Camera attributes

//Reflection types

//Particle Blend Modes

//Special IDs

//HSF_CONSTDATA attribute values

//Typedefs for various IDs
typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;
typedef s16 HU3D_PROJID;
typedef s16 HU3D_LIGHTID;
typedef s16 HU3D_LLIGHTID;
typedef s16 HU3D_PARMANID;
typedef s16 HU3D_ANIMID;
typedef s16 HU3D_TEXSCRID;

//Forward declarations
typedef struct Hu3DModel_s HU3D_MODEL;
typedef struct Hu3DDrawObj_s HU3D_DRAW_OBJ;
typedef struct Hu3DParticle_s HU3D_PARTICLE;

//Function pointer declarations
typedef void (*HU3D_LAYER_HOOK)(s16 layerNo);
typedef void (*HU3D_MODEL_HOOK)(HU3D_MODEL *modelP, Mtx *mtx);
typedef void (*HU3D_TIMING_HOOK)(HU3D_MODELID modelId, HU3D_MOTIONID motId, BOOL lagF);
typedef void (*HU3D_MAT_HOOK)(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material);
typedef void (*HU3D_PARTICLE_HOOK)(HU3D_MODEL *modelP, HU3D_PARTICLE *particleP, Mtx mtx);
typedef void (*HU3D_OBJ_HOOK)(HSF_OBJECT *obj, HSF_TRANSFORM *transform, Mtx *prev, Mtx *curr);

struct Hu3DDrawObj_s {
    /* 0x00 */ HU3D_MODEL *model;
    /* 0x04 */ HSF_OBJECT *object;
    /* 0x08 */ float z;
    /* 0x0C */ Mtx matrix;
    /* 0x3C */ HuVecF scale;
};

typedef struct Hu3DAttrAnim_s {
    u16 attr;
    HU3D_ANIMID animId;
    HU3D_TEXSCRID texScrId;
    HuVecF trans3D;
    HuVecF rot;
    HuVecF scale3D;
    HuVec2f scale;
    HuVec2f trans;
    HSF_BITMAP *bitMapPtr;
    u32 unk40;
} HU3D_ATTR_ANIM;

typedef struct HsfDrawData_s {
    s32 dlOfs;
    s32 dlSize;
    u16 polyCnt;
    u32 flags;
} HSF_DRAWDATA;

typedef struct HsfConstData_s {
    u32 attr;
    HU3D_MODELID hookMdlId;
    u8 shadowAlpha;
    HSF_DRAWDATA *drawData;
    void *dlBuf;
    Mtx matrix;
    ANIMDATA *hiliteMap;
    HU3D_OBJ_HOOK hook;
    u32 triCnt;
} HSF_CONSTDATA;

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
    HU3D_MOTIONID motIdCluster[4];
    s16 clusterAttr[4];
    HU3D_MOTIONID motIdSrc;
    u16 cameraBit;
    HU3D_MODELID linkMdlId;
    u16 lightNum;
    u16 lightId[8];
    HU3D_LIGHTID LLightId[8];
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
    float clusterTime[4];
    float clusterSpeed[4];
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

typedef struct Hu3DCamera_s {
    float fov;
    float near;
    float far;
    float aspect;
    float upRot;
    Vec pos;
    Vec up;
    Vec target;
    s16 scissorX;
    s16 scissorY;
    s16 scissorW;
    s16 scissorH;
    float viewportX;
    float viewportY;
    float viewportW;
    float viewportH;
    float viewportNear;
    float viewportFar;
} HU3D_CAMERA;

typedef struct Hu3DProjection_s {
    u8 alpha;
    ANIMDATA *anim;
    float fov;
    float near;
    float far;
    HuVecF camPos;
    HuVecF camTarget;
    HuVecF camUp;
    Mtx lookAtMtx;
    Mtx projMtx;
} HU3D_PROJECTION;

typedef struct Hu3DShadow_s {
    GXColor color;
    u16 size;
    void *buf;
    float fov;
    float near;
    float far;
    HuVecF camPos;
    HuVecF camTarget;
    HuVecF camUp;
    Mtx lookAtMtx;
    Mtx projMtx;
} HU3D_SHADOW;

typedef struct Hu3DLight_s {
    s16 type;
    s16 func;
    float cutoff;
    float brightness;
    u8 unkC[16];
    HuVecF pos;
    HuVecF dir;
    HuVecF offset;
    GXColor color;
} HU3D_LIGHT;

typedef struct Hu3DMotion_s {
    u16 attr;
    HU3D_MODELID modelId;
    HSF_DATA *hsf;
} HU3D_MOTION;

typedef struct Hu3DParticleData_s {
    s16 time;
    HU3D_PARMANID parManId;
    s16 attr;
    s16 cameraBit;
    HuVecF vel;
    HuVecF accel;
    union {
        struct {
            float speedDecay;
            float colorIdx;
            float scaleBase;
        };
        Vec effectDirection;
    };
    float scale;
    float scaleY;
    float zRot;
    HuVecF pos;
    GXColor color;
} HU3D_PARTICLE_DATA;

typedef struct Hu3DParticle_s {
    s16 dataCnt;
    s16 emitCnt;
    HuVecF pos;
    HuVecF spawnCenter;
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
} HU3D_PARTICLE; 

typedef struct Hu3DParmanParam_s {
    s16 maxTime;
    s16 unk2;
    float accelRange;
    float scaleRange;
    float angleRange;
    Vec gravity;
    float speedBase;
    float speedDecay;
    float scaleBase;
    float scaleDecay;
    s16 colorNum;
    GXColor colorStart[4];
    GXColor colorEnd[4];
} HU3D_PARMAN_PARAM;

typedef struct Hu3DTexAnim_s {
    u16 attr;
    s16 bank;
    s16 anmNo;
    HU3D_MODELID modelId;
    float time;
    float speed;
    ANIMDATA *anim;
} HU3D_TEXANIM; // Size 0x14

typedef struct Hu3DTexScroll_s {
    u16 attr;
    HU3D_MODELID modelId;
    HuVecF pos;
    HuVecF scale;
    HuVecF posMove;
    HuVecF scaleMove;
    float rot;
    float rotMove;
    Mtx texMtx;
} HU3D_TEXSCROLL;

typedef struct Hu3DParMan_s {
    HU3D_MODELID modelId;
    s16 attr;
    s16 timeLimit;
    HU3D_PARMANID parManId;
    s16 color;
    Vec pos;
    Vec vec;
    Vec vacuum;
    float vacuumSpeed;
    float accel;
    s32 jitterNo;
    HU3D_PARMAN_PARAM *param;
} HU3D_PARMAN;

typedef struct Hu3DWaterWave_s {
    HuVecF pos;
    float radiusMax;
    float radius;
    float texMtx[2][3];
    s16 time;
} HU3D_WATERWAVE;

typedef struct Hu3DWater_s {
    s16 layerNo;
    u16 cameraBit;
    float texMtx[2][3];
    s16 glowMode;
    s16 fbWaterW;
    s16 fbWaterH;
    void *fbWater;
    ANIMDATA *animBump;
    ANIMDATA *animSurface;
    ANIMDATA *animSky;
    ANIMDATA *animWave;
    void *fbDisp;
    HuVecF texPos;
    HuVecF texScale;
    HuVecF posMin;
    HuVecF posMax;
    float padY;
    GXColor glowCol;
    GXColor hiliteCol;
    BOOL mipMapF;
    s16 unk7C;
    HU3D_WATERWAVE wave[32];
    float maxTime;
} HU3D_WATER;

void fn_80017AF8(void);

void Hu3DDrawPreInit(void);
void fn_80017B34(HU3D_MODEL *model, Mtx matrix, HuVecF *scale);
BOOL fn_800186B4(HSF_DATA *hsf, HSF_OBJECT *object, Mtx mtx);
void fn_80019EA8(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *matP);
void fn_8001A7CC(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *matP);
void fn_8001CDF0(HU3D_MODEL *modelP, u32 flags, float hilitePower);
void fn_80020024(void);
void fn_80020DCC(HU3D_MODELID modelId, u32 mallocno);
HSF_CONSTDATA *fn_800215B4(HSF_OBJECT *object, u32 no);
void mtxTransCat(Mtx dst, float x, float y, float z);
void mtxRotCat(Mtx dst, float x, float y, float z);
void fn_80022E38(Mtx dst, float x, float y, float z);
void mtxScaleCat(Mtx dst, float x, float y, float z);
s16 HmfInverseMtxF3X3(Mtx dst, Mtx src);
void SetDefLight(HuVecF *pos, HuVecF *dir, u8 colorR, u8 colorG, u8 colorB, u8 ambR, u8 ambG,
    u8 ambB, u8 matR, u8 matG, u8 matB);
void Hu3DModelObjPosGet(HU3D_MODELID modelId, char *objName, HuVecF *pos);
void fn_80023398(HU3D_MODELID modelId, char *objName, Mtx mtx);
HSF_OBJECT *fn_800243B0(HSF_DATA *hsf, u32 mallocNo);
HSF_ATTRIBUTE *Hu3DAttrDuplicate(HSF_DATA *hsf, u32 mallocNo);
HSF_MATERIAL *Hu3DMatDuplicate(HSF_DATA *hsf, u32 mallocNo);
void fn_8002472C(void);
void fn_800247C4(HU3D_MODELID modelId, char *objName, Mtx mtx);
void fn_80024850(HU3D_MODELID modelId, HSF_OBJECT *objPtr, Mtx mtx);

void fn_800249F8(void);
void fn_80024CAC(void);
void fn_80024E08(void);
void fn_80025630(void);
void Hu3DBGColorSet(u8 r, u8 g, u8 b);
void fn_8002583C(s16 bit, s16 layerNo, HU3D_LAYER_HOOK hook);
void fn_80025918(s16 layerNo, HU3D_LAYER_HOOK hook);
void fn_800258A8(s16 bit, s16 layerNo);
void fn_80025980(s16 layerNo);

void Hu3DPauseSet(BOOL pauseF);
void Hu3DNoSyncSet(BOOL noSync);
HU3D_MODELID fn_80025A04(void *data);
HU3D_MODELID fn_80025D4C(HU3D_MODELID linkMdlId);
HU3D_MODELID fn_800260F4(HU3D_MODEL_HOOK hookFunc);
void fn_800262D0(HU3D_MODELID modelId);
void fn_80026648(void);
void fn_80026760(HU3D_MODELID modelId, float posX, float posY, float posZ);
void fn_80026790(HU3D_MODELID modelId, HuVecF *pos);
void fn_800267CC(HU3D_MODELID modelId, HuVecF *pos);
void fn_80026808(HU3D_MODELID modelId, float rotX, float rotY, float rotZ);
void fn_80026838(HU3D_MODELID modelId, HuVecF *rot);
void fn_80026874(HU3D_MODELID modelId, HuVecF *rot);

void fn_800268B0(HU3D_MODELID modelId, float scaleX, float scaleY, float scaleZ);
void fn_800268E0(HU3D_MODELID modelId, HuVecF *scale);
void fn_8002691C(HU3D_MODELID modelId, HuVecF *scale);

void fn_80026958(HU3D_MODELID modelId, Mtx *mtx);
void fn_800269A0(HU3D_MODELID modelId, Mtx *mtx);

void fn_800269E8(HU3D_MODELID modelId, u32 attr);
void fn_80026A44(HU3D_MODELID modelId, u32 attr);
u32 fn_80026A90(HU3D_MODELID modelId);
u32 fn_80026AB8(HU3D_MODELID modelId);
void fn_80026AE4(HU3D_MODELID modelId, s16 clusterNo, s32 clusterAttr);
void fn_80026B24(HU3D_MODELID modelId, s16 clusterNo, s32 clusterAttr);
void fn_80026B64(HU3D_MODELID modelId, u16 cameraBit);
void fn_80026B8C(HU3D_MODELID modelId, s16 layerNo);
HSF_OBJECT *fn_80026BF4(HU3D_MODELID modelId, char *objName);
void fn_80026CBC(HU3D_MODELID modelId, float tpLvl);
void fn_80026DB0(HU3D_MODELID modelId, ANIMDATA *anim);
void fn_80026E44(HU3D_MODELID modelId);
void fn_80026EF0(HU3D_MODELID modelId);
void fn_80026F9C(HU3D_MODELID modelId);
void fn_80026FCC(HU3D_MODELID modelId);
void fn_80026FFC(HU3D_MODELID modelId);
void fn_8002736C(HU3D_MODELID modelId);
void fn_80027080(HU3D_MODELID modelId, char *objName);
void fn_80027138(HU3D_MODELID modelId, char *objName);
void fn_800271F0(HU3D_MODELID modelId, float tpLvl);
void fn_80027290(HU3D_MODELID modelId, char *objName, float tpLvl);
void fn_800273EC(HU3D_MODELID modelId, float ambR, float ambG, float ambB);
void fn_8002741C(HU3D_MODELID modelId, char *objName, HU3D_MODELID hookMdlId);
void fn_80027508(HU3D_MODELID modelId);
void fn_800275B4(HU3D_MODELID modelId, char *objName);
void fn_8002769C(HU3D_MODELID modelId, HU3D_PROJID projId);
void fn_800276DC(HU3D_MODELID modelId, HU3D_PROJID projId);
void fn_8002771C(HU3D_MODELID modelId, s16 hiliteType);
void fn_800277D0(HU3D_MODELID modelId, s16 reflectType);
void fn_800277FC(HU3D_MODELID modelId);
void fn_80027884(HU3D_MODELID modelId, HU3D_MAT_HOOK matHook);
void Hu3DCameraCreate(int cameraBit);
void Hu3DCameraPerspectiveSet(int cameraBit, float fov, float near, float far, float aspect);
void Hu3DCameraViewportSet(int cameraBit, float vpX, float vpY, float vpW, float vpH,
    float vpNearZ, float vpFarZ);
void Hu3DCameraScissorSet(int cameraBit, unsigned int scissorX, unsigned int scissorY,
    unsigned int scissorW, unsigned int scissorH);
void Hu3DCameraPosSet(int cameraBit, float posX, float posY, float posZ, float upX, float upY,
    float upZ, float targetX, float targetY, float targetZ);
void Hu3DCameraPosSetV(int cameraBit, Vec *pos, Vec *up, Vec *target);
void fn_80027CD0(int cameraBit, Vec *pos, Vec *up, Vec *target);
void fn_80027D8C(int cameraBit, float *fov, float *near, float *far);
void Hu3DCameraKill(int cameraBit);
void Hu3DCameraAllKill(void);
void Hu3DCameraSet(s32 cameraNo, Mtx modelView);
BOOL fn_8002803C(HU3D_MODELID modelId, u16 cameraBit);
s16 fn_800283F4(HU3D_MOTIONID motId, u16 cameraBit);
void fn_80028478(HU3D_MODELID modelId, u16 cameraBit);
void fn_800284C8(HU3D_MODELID modelId, u16 cameraBit);
void fn_8002858C(HU3D_MODELID modelId);
void fn_800285BC(void);
HU3D_LIGHTID Hu3DGLightCreate(float posX, float posY, float posZ, float dirX, float dirY,
    float dirZ, u8 colorR, u8 colorG, u8 colorB);
HU3D_LIGHTID fn_800286D0(HuVecF *pos, HuVecF *dir, GXColor *color);
HU3D_LLIGHTID Hu3DLLightCreate(HU3D_MODELID modelId, float posX, float posY, float posZ,
    float dirX, float dirY, float dirZ, u8 colorR, u8 colorG, u8 colorB);
HU3D_LLIGHTID fn_800288B4(HU3D_MODELID modelId, HuVecF *pos, HuVecF *dir, GXColor *color);
void Hu3DGLightSpotSet(HU3D_LIGHTID lightId, float cutoff, GXSpotFn spotFunc);
void fn_80028AA8(HU3D_MODELID modelId, HU3D_LLIGHTID lightId, float cutoff,
    GXSpotFn spotFunc);
void Hu3DGLightInfinitytSet(HU3D_LIGHTID lightId);
void fn_80028B58(HU3D_MODELID modelId, HU3D_LLIGHTID lightId);
void Hu3DGLightPointSet(HU3D_LIGHTID lightId, float refDistance, float refBrightness,
    GXDistAttnFn distFunc);
void fn_80028C1C(HU3D_MODELID modelId, HU3D_LLIGHTID lightId, float refDistance,
    float refBrightness, GXDistAttnFn distFunc);
void Hu3DGLightKill(HU3D_LIGHTID lightId);
void fn_80028CB8(HU3D_MODELID modelId, HU3D_LLIGHTID lightId);
void fn_80028D70(void);
void Hu3DGLightColorSet(HU3D_LIGHTID lightId, u8 r, u8 g, u8 b, u8 a);
void fn_80028E48(HU3D_MODELID modelId, HU3D_LLIGHTID lightId, u8 r, u8 g, u8 b, u8 a);
void Hu3DGLightPosSetV(HU3D_LIGHTID lightId, HuVecF *pos, HuVecF *dir);
void fn_80028F14(HU3D_MODELID modelId, HU3D_LLIGHTID lightId, HuVecF *pos, HuVecF *dir);
void Hu3DGLightPosSet(HU3D_LIGHTID lightId, float posX, float posY, float posZ, float dirX,
    float dirY, float dirZ);
void fn_8002903C(HU3D_MODELID modelId, HU3D_LLIGHTID lightId, float posX, float posY,
    float posZ, float dirX, float dirY, float dirZ);
void fn_800290F4(HU3D_LIGHTID lightId, float posX, float posY, float posZ, float angleX,
    float angleY);
void fn_80029240(HU3D_MODELID modelId, HU3D_LLIGHTID lightId, float posX, float posY,
    float posZ, float angleX, float angleY);
void Hu3DGLightPosAimSetV(HU3D_LIGHTID lightId, HuVecF *pos, HuVecF *aim);
void fn_80029434(HU3D_MODELID modelId, HU3D_LLIGHTID lightId, HuVecF *pos, HuVecF *aim);
void Hu3DGLightPosAimSet(HU3D_LIGHTID lightId, float posX, float posY, float posZ, float aimX,
    float aimY, float aimZ);
void fn_800295A0(HU3D_MODELID modelId, HU3D_LLIGHTID lightId, float posX, float posY,
    float posZ, float aimX, float aimY, float aimZ);
void Hu3DGLightStaticSet(HU3D_LIGHTID lightId, BOOL staticF);
void fn_800296E0(HU3D_MODELID modelId, HU3D_LLIGHTID lightId, BOOL staticF);
s16 fn_80029758(HU3D_LIGHTID LLightId, Vec *pos, Vec *dir, GXColor *color);
s16 fn_800297D4(HU3D_LIGHTID gLightId, Vec *pos, Vec *dir, GXColor *color);
s32 fn_80029850(HU3D_MODELID modelId, s16 staticF);
HU3D_LIGHTID fn_80029D24(HU3D_MODELID modelId, char *objName);
u8 fn_80029E00(HU3D_MODELID modelId, HU3D_LIGHTID lightId);
u8 fn_80029E44(HU3D_MODELID modelId, HU3D_LIGHTID lightId);

s16 fn_80029E8C(HU3D_MODEL *modelP, Mtx cameraMtx, Mtx cameraMtxXPose, float hilitePower);
void fn_8002A2A4(s16 no);
void Hu3DFogSet(float start, float end, u8 r, u8 g, u8 b);
void Hu3DFogClear(void);
void Hu3DAmbColorSet(float r, float g, float b);
void fn_8002A390(BOOL shineF);
void fn_8002A3B0(float fov, float near, float far, s16 cameraBit);
void Hu3DShadowCreate(float fov, float near, float far);
void fn_8002A674(HuVecF *camPos, const HuVecF *camUp, HuVecF *camTarget, s16 cameraBit);
void Hu3DShadowPosSet(HuVecF *camPos, HuVecF *camUp, HuVecF *camTarget);
void fn_8002A7D8(float tpLvl, s16 cameraBit);
void Hu3DShadowTPLvlSet(float tpLvl);
void fn_8002A8CC(u16 size, s16 cameraBit);
void Hu3DShadowSizeSet(u16 size);
void fn_8002AA3C(u8 r, u8 g, u8 b, s16 cameraBit);
void fn_8002AAB4(u8 r, u8 g, u8 b);
HU3D_PROJID Hu3DProjectionCreate(ANIMDATA *anim, float fov, float near, float far);
void Hu3DProjectionKill(HU3D_PROJID projId);
void Hu3DProjectionPosSet(HU3D_PROJID projId, HuVecF *camPos, HuVecF *camUp, HuVecF *camTarget);
void Hu3DProjectionTPLvlSet(HU3D_PROJID projId, float tpLvl);
void fn_8002BAF8(void *animData, HU3D_MODELID modelId, char *bmpName);
void fn_8002BCDC(s16 x, s16 y, s16 w, s16 h, GXTexFmt texFmt, s16 mipmapF, void *buf);
void fn_8002BDB4(s16 layerNo, s16 x, s16 y, s16 w, s16 h, GXTexFmt texFmt, s16 mipmapF,
    void *buf);
void fn_8002BF48(void);
void fn_8002C2AC(s16 layerNo);
void fn_8002C31C(void);

void fn_8002C47C(void);
HU3D_MOTIONID fn_8002C4C0(void *data);
HU3D_MOTIONID fn_8002C564(HU3D_MODELID modelId);
BOOL fn_8002C624(HU3D_MOTIONID motId);
void fn_8002C7BC(void);
void fn_8002C95C(HU3D_MODELID modelId, HU3D_MOTIONID motId);
void fn_8002CAA8(HU3D_MODELID modelId, HU3D_MOTIONID motId);
void fn_8002CAF0(HU3D_MODELID modelId);
float fn_8002CB0C(HU3D_MODELID modelId);
float fn_8002CB24(HU3D_MODELID modelId);
BOOL fn_8002CBB0(HU3D_MODELID modelId);
void fn_8002CB64(HU3D_MODELID modelId, float time);
void fn_8002CB98(HU3D_MODELID modelId, float speed);
void fn_8002CCC0(HU3D_MODELID modelId, HU3D_MOTIONID motId, float start, float end,
    u32 attr);
void fn_8002CEB0(HU3D_MODELID modelId, HU3D_MOTIONID motId);
HU3D_MOTIONID fn_8002CF58(HU3D_MODELID modelId);
void fn_8002CF70(HU3D_MODELID modelId, float speed);
void fn_8002CF98(HU3D_MODELID modelId, float time);
float fn_8002CFCC(HU3D_MODELID modelId);

float fn_8002CFE4(HU3D_MODELID modelId);
void fn_8002D024(HU3D_MODELID modelId, float start, float end);
s16 fn_8002D050(HU3D_MODELID modelId, HU3D_MOTIONID motId);
s16 fn_8002D160(HU3D_MODELID modelId, HU3D_MOTIONID motId, s16 clusterNo);
void fn_8002D22C(HU3D_MODELID modelId);
void fn_8002D248(HU3D_MODELID modelId, s16 clusterNo);
void fn_8002D30C(HU3D_MODELID modelId, s16 clusterNo, float speed);
void fn_8002D340(HU3D_MODELID modelId, s16 clusterNo, float time);
float fn_8002D374(HU3D_MODELID modelId, s16 clusterNo);
HU3D_MOTIONID fn_8002D3C4(HU3D_MODELID modelId);
HU3D_MOTIONID fn_8002D3EC(HU3D_MODELID modelId);
void fn_8002D414(HU3D_MODELID modelId, float time);
float fn_8002D4E0(HU3D_MODELID modelId);
float fn_8002D508(HU3D_MODELID modelId);
float fn_8002D530(HU3D_MODELID modelId);
float fn_8002D5D4(HU3D_MODELID modelId);
void fn_8002D678(HU3D_MODELID modelId, float start, float end);
float Hu3DMotionMotionMaxTimeGet(HU3D_MOTIONID motId);
void fn_8002D730(HU3D_MODELID modelId, float start, float end);
BOOL fn_8002D75C(HU3D_MODELID modelId);
void fn_8002D86C(HU3D_MODELID modelId, float speed);
void fn_8002D894(HU3D_MODELID modelId, float speed);
void fn_8002D8BC(HU3D_MODELID modelId, char *objName, u32 forceAttr);
void Hu3DMotionNoMotReset(HU3D_MODELID modelId, char *objName, u32 forceAttr);
void fn_8002DA64(HU3D_MODELID modelId, char *objName, u32 forceAttr, float value);
void fn_8002DB98(HU3D_MODELID modelId, HU3D_TIMING_HOOK timingHook);
void fn_8002DC74(HU3D_MOTIONID motId, u16 attr);
void fn_8002DCA8(HU3D_MOTIONID motId, u16 attr);
void fn_8002DCE0(HU3D_MODELID modelId);
void fn_8002E4C4(HU3D_MODELID modelId, HU3D_MOTIONID motId, float time, BOOL copyXFormF);
void fn_8002EC20(HU3D_MODELID modelId);
void fn_8002ECDC(HU3D_MODELID modelId);
float *fn_8002F114(HSF_OBJECT *objPtr, u16 channel);
void fn_8002F334(HU3D_MODELID modelId, HSF_TRACK *trackP, float value);
void fn_8002F538(HU3D_MODELID modelId, HSF_TRACK *trackP, float value);
void fn_8002F780(HU3D_MODELID modelId, HSF_TRACK *trackP, float value);
void fn_8002FD84(HU3D_MODELID modelId, HSF_TRACK *trackP, float value);
float GetCurve(HSF_TRACK *trackP, float time);
float GetConstant(s32 keyNum, HSF_CONSTANT_KEY *key, float time);
float GetLinear(s32 keyNum, HSF_LINEAR_KEY *key, float time);
float GetBezier(s32 keyNum, HSF_TRACK *trackP, float time);
HSF_BITMAP *GetBitMap(s32 keyNum, HSF_BITMAP_KEY *key, float time);
HU3D_MOTIONID fn_80030458(HU3D_MODELID modelId, void *data);
void fn_80030524(HU3D_MODELID modelId, HU3D_MOTIONID motId);
void fn_80030644(HU3D_MODELID modelId);
void fn_800308F0(HU3D_MODELID modelId, HU3D_MOTIONID motId, float time);
void fn_800309F4(HU3D_MODELID modelId);
void fn_80030A4C(HU3D_MODELID modelId, float time);
void fn_80030A88(HU3D_MODELID modelId, float time);

void Hu3DAnimInit(void);
HU3D_ANIMID fn_80030B2C(void *dataP, HU3D_MODELID modelId, char *bmpName);
ANIMDATA *fn_80030D50(HU3D_ANIMID animId, ANIMDATA *animP);
HU3D_ANIMID fn_80030DF0(HU3D_ANIMID linkAnimId, HU3D_MODELID modelId, char *bmpName);
void fn_80031010(HU3D_ANIMID animId);
void fn_8003111C(HU3D_MODELID modelId);
void fn_80031260(void);
void Hu3DAnimAttrSet(HU3D_ANIMID animId, u16 attr);
void Hu3DAnimAttrReset(HU3D_ANIMID animId, u16 attr);
void Hu3DAnimSpeedSet(HU3D_ANIMID animId, float speed);
void Hu3DAnimBankSet(HU3D_ANIMID animId, u16 bank);
void Hu3DAnmNoSet(HU3D_ANIMID animId, u16 anmNo);
void Hu3DAnimExec(void);
HU3D_TEXSCRID fn_80031C10(HU3D_MODELID modelId, char *bmpName);
void fn_80031DB4(HU3D_TEXSCRID texScrId);
void fn_80031E90(void);
void Hu3DTexScrollPosSet(HU3D_TEXSCRID texScrId, float posX, float posY, float posZ);
void Hu3DTexScrollPosMoveSet(HU3D_TEXSCRID texScrId, float posX, float posY, float posZ);
void Hu3DTexScrollRotSet(HU3D_TEXSCRID texScrId, float rot);
void Hu3DTexScrollRotMoveSet(HU3D_TEXSCRID texScrId, float rot);
void Hu3DTexScrollPauseDisableSet(HU3D_TEXSCRID texScrId, BOOL pauseDiableF);
HU3D_MODELID fn_8003216C(ANIMDATA *anim, s16 maxCnt);
void fn_800324E0(HU3D_MODELID modelId, float scale);
void fn_80032548(HU3D_MODELID modelId, float zRot);
void fn_800325B0(HU3D_MODELID modelId, u8 r, u8 g, u8 b);
void fn_80032620(HU3D_MODELID modelId, float tpLvl);
void fn_8003269C(HU3D_MODELID modelId, u8 blendMode);
void fn_800326D0(HU3D_MODELID modelId, HU3D_PARTICLE_HOOK hook);
void fn_80032704(HU3D_MODELID modelId, u8 attr);
void fn_80032740(HU3D_MODELID modelId, u8 attr);
void fn_80032780(HU3D_MODELID modelId, s16 count);
void fn_800327B8(HU3D_MODELID modelId, s16 animBank);
void Hu3DParManInit(void);
HU3D_PARMANID fn_800334F4(ANIMDATA *anim, s16 maxCnt, HU3D_PARMAN_PARAM *param);
HU3D_PARMANID Hu3DParManLink(HU3D_PARMANID linkParManId, HU3D_PARMAN_PARAM *param);
void fn_8003383C(HU3D_PARMANID parManId);
void fn_800339C8(void);
HU3D_PARMAN *Hu3DParManPtrGet(HU3D_PARMANID parManId);
void Hu3DParManPosSet(HU3D_PARMANID parManId, float posX, float posY, float posZ);
void Hu3DParManVecSet(HU3D_PARMANID parManId, float x, float y, float z);
void Hu3DParManRotSet(HU3D_PARMANID parManId, float rotX, float rotY, float rotZ);
void Hu3DParManAttrSet(HU3D_PARMANID parManId, s32 attr);
void Hu3DParManAttrReset(HU3D_PARMANID parManId, s32 attr);
HU3D_MODELID Hu3DParManModelIDGet(HU3D_PARMANID parManId);
void Hu3DParManTimeLimitSet(HU3D_PARMANID parManId, s32 timeLimit);
void Hu3DParManVacumeSet(HU3D_PARMANID parManId, float x, float y, float z, float speed);
void Hu3DParManColorSet(HU3D_PARMANID parManId, s16 color);
void fn_80033E64(HU3D_PARMANID parManId, s16 layer);

void fn_80034F8C(s16 layerNo, void *animBump, void *animSurface, void *animSky, BOOL mipMapF,
    HuVecF *posMin, HuVecF *posMax);
void fn_800352BC(float posX, float posY, float posZ);
void fn_800352E4(float scaleX, float scaleY, float scaleZ);
void fn_8003530C(float padY);
void fn_8003532C(s16 glowMode, GXColor *glowCol);
void fn_8003536C(float texMtx[2][3]);
void fn_800353FC(float level);
void fn_80035438(float tpLvl);
void fn_8003546C(u16 cameraBit);
void fn_8003548C(HuVecF *pos, float radius, float radiusMax);
void fn_800360D8(void *buf, s16 w, s16 h, u32 format, GXTexWrapMode wrapS, GXTexWrapMode wrapT,
    BOOL filterF, GXTexMapID texMapId);

HU3D_MODELID fn_80036DF8(void *data);
HU3D_MODELID fn_80036EBC(HU3D_MODELID linkMdlId);
void fn_80036F80(HU3D_MODELID modelId);
void fn_800248C4(HU3D_MODELID modelId, char *objName, HU3D_OBJ_HOOK hook);

extern HuVecF lbl_8025FDF8;
extern HuVecF lbl_8025FE04;
extern u32 lbl_802C0128;
extern u32 lbl_802C0124;
extern u32 lbl_802C0120;
extern u32 lbl_802C011C;
extern u32 lbl_802C0118;
extern u32 lbl_802C0114;
extern u32 lbl_802C0110;
extern u32 lbl_802C010C;

extern HU3D_LIGHT lbl_80261674[48];
extern HU3D_LIGHT Hu3DGlobalLight[8];
extern Mtx lbl_80261424;
extern Mtx lbl_802613F4;
extern HSF_SCENE FogData;
extern HU3D_SHADOW Hu3DShadowBuf[16];
extern HU3D_PROJECTION Hu3DProjection[4];
extern ANIMDATA *lbl_802607B4[4];
extern ANIMDATA *reflectAnim[5];
extern HU3D_CAMERA Hu3DCamera[16];
extern HU3D_MODEL *lbl_802C01C8;

extern GXColor lbl_802C01C2;
extern s16 lbl_802C01C0;
extern ANIMDATA *lbl_802C01BC;
extern s16 lbl_802C01BA;
extern s16 lbl_802C01B8;
extern BOOL lbl_802C01B4;
extern BOOL lbl_802C01B0;
extern HU3D_SHADOW *lbl_802C01AC;
extern s16 lbl_802C01A8;
extern s16 lbl_802C01A6;
extern s16 lbl_802C01A4;
extern u32 lbl_802C01A0;
extern s16 lbl_802C019E;
extern u16 lbl_802C019C;
extern HU3D_MODELID lbl_802C0198;
extern ANIMDATA *lbl_802C0194;
extern float lbl_802C0190;
extern float lbl_802C018C;
extern float lbl_802C0188;
extern HuVecF lbl_8025FDF8;
extern HuVecF lbl_8025FE04;
extern BOOL lbl_802C0184;

extern HU3D_MOTION lbl_802624B8[384];

extern HU3D_TEXANIM Hu3DTexAnimData[256];
extern HU3D_TEXSCROLL Hu3DTexScrData[16];

extern HU3D_WATER lbl_80264C78;

#endif
