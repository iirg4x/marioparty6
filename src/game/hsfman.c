/* Initializes and renders the game's 3D models, cameras, lights, and effects. */
#define _MATH_H
#define M_PI 3.141592653589793
double sin(double x);
double cos(double x);
double tan(double x);
#include "string.h"
#include "game/init.h"
#include "game/disp.h"

#include "game/hu3d.h"
#include "game/hsfload.h"

#include "game/sprite.h"
#include "game/perf.h"
#include "game/ClusterExec.h"
#include "game/EnvelopeExec.h"
#include "game/ShapeExec.h"

#define HOOKFUNC_MALLOCNO 10000

#define REFLECT_TEX_W 128
#define REFLECT_TEX_H 128

#define LIGHT_TYPE_MASK ((1 << 8) - 1)
#define LIGHT_FLAGS_MASK (LIGHT_TYPE_MASK << 8)
#define MODEL_GLOBAL_LIGHTS_ALL ((1 << HU3D_GLIGHT_MAX) - 1)
#define MODEL_LINK_FILE_FLAG (1U << 31)
#define MATERIAL_HILITE_TYPE_MASK (((1 << 4) - 1) << 4)

#define LIGHT_TYPE_SET(lightP, lightType) ((lightP)->type &= LIGHT_FLAGS_MASK); \
    ((lightP)->type |= (lightType))

#define LIGHT_TYPE_GET(lightP) ((lightP)->type & LIGHT_TYPE_MASK)

#include "refMapData0.inc"
#include "refMapData1.inc"
#include "refMapData2.inc"
#include "refMapData3.inc"
#include "refMapData4.inc"
#include "toonMapData.inc"
#include "toonMapData2.inc"
#include "hiliteData.inc"
#include "hiliteData2.inc"
#include "hiliteData3.inc"
#include "hiliteData4.inc"

typedef struct FbCopyLayer_s {
    s16 layerNo;       /* Render layer whose callback copies the framebuffer. */
    s16 sourceX;       /* Top-left source x coordinate in framebuffer pixels. */
    s16 sourceY;       /* Top-left source y coordinate in framebuffer pixels. */
    s16 width;         /* Source width in framebuffer pixels. */
    s16 height;        /* Source height in framebuffer pixels. */
    GXTexFmt texFmt;   /* Texture format used for the copied image. */
    BOOL mipmapF;      /* Whether the copy destination uses mipmap sizing. */
    void *textureBuffer; /* Destination texture buffer. */
} FBCOPY_LAYER;

static FBCOPY_LAYER FbCopyLayer[HU3D_LAYER_HOOK_MAX];
HU3D_LIGHT Hu3DLocalLight[HU3D_LLIGHT_MAX];
HU3D_LIGHT Hu3DGlobalLight[HU3D_GLIGHT_MAX];
Mtx Hu3DCameraMtxXPose;
Mtx Hu3DCameraMtx;
HSF_SCENE FogData;
HU3D_SHADOW Hu3DShadowBuf[HU3D_CAM_MAX];
HU3D_PROJECTION Hu3DProjection[HU3D_PROJ_MAX];
ANIMDATA *hiliteAnim[4];
ANIMDATA *reflectAnim[5];
static HU3D_LAYER_HOOK layerHook[HU3D_CAM_MAX][HU3D_LAYER_HOOK_MAX];
static s16 layerNum[HU3D_LAYER_MAX];
HU3D_CAMERA Hu3DCamera[HU3D_CAM_MAX];
HU3D_MODEL *Hu3DData;

GXColor BGColor;
s16 reflectMapNo;
ANIMDATA *toonAnim;
s16 shadowNum;
s16 Hu3DShadowCamBit;
BOOL Hu3DShadowF;
BOOL shadowModelDrawF;
HU3D_SHADOW *Hu3DShadow;
s16 Hu3DProjectionNum;
s16 Hu3DCameraNo;
s16 Hu3DCameraBit;
u32 Hu3DMallocNo;
s16 Hu3DPauseF;
u16 Hu3DCameraExistF;
static u16 NoSyncF;
HU3D_MODELID Hu3DReflectModelId;
ANIMDATA *Hu3DReflectModelAnim;
float Hu3DAmbColR;
float Hu3DAmbColG;
float Hu3DAmbColB;
BOOL Hu3DShineF;
static BOOL modelKillAllF;

/* Called once by main during startup to initialize the 3D renderer and its resources. */
void Hu3DInit(void)
{
    s16 index;
    s16 cameraIndex;
    HU3D_MODEL *modelP;
    HU3D_CAMERA *cameraP;
    Hu3DDrawInit();
    Hu3DData = modelP = HuMemDirectMalloc(HEAP_HEAP, sizeof(HU3D_MODEL)*HU3D_MODEL_MAX);
    
    for(modelP, index=0; index<HU3D_MODEL_MAX; index++, modelP++) {
        modelP->hsf = NULL;
    }
    for(cameraP = &Hu3DCamera[0], index=0; index<HU3D_CAM_MAX; index++, cameraP++) {
        cameraP->fov = -1;
    }
    Hu3DMotionInit();
    Hu3DLighInit();
    BGColor.r = BGColor.g = BGColor.b = BGColor.a = 0;
    for(index=0; index<HU3D_LAYER_MAX; index++) {
        layerNum[index] = 0;
    }
    for(cameraIndex=0; cameraIndex<HU3D_CAM_MAX; cameraIndex++) {
        for(index=0; index<HU3D_LAYER_HOOK_MAX; index++) {
            layerHook[cameraIndex][index] = NULL;
        }
    }
    
    reflectAnim[0] = HuSprAnimRead(refMapData0);
    reflectAnim[1] = HuSprAnimRead(refMapData1);
    reflectAnim[2] = HuSprAnimRead(refMapData2);
    reflectAnim[3] = HuSprAnimRead(refMapData3);
    reflectAnim[4] = HuSprAnimRead(refMapData4);
    reflectMapNo = 0;
    toonAnim = HuSprAnimRead(toonMapData);
    hiliteAnim[0] = HuSprAnimRead(hiliteData);
    hiliteAnim[1] = HuSprAnimRead(hiliteData2);
    hiliteAnim[2] = HuSprAnimRead(hiliteData3);
    hiliteAnim[3] = HuSprAnimRead(hiliteData4);
    Hu3DFogClear();
    Hu3DAnimInit();
    Hu3DParManInit();
    for(index=0; index<HU3D_PROJ_MAX; index++) {
        Hu3DProjection[index].anim = NULL;
    }
    shadowNum = 0;
    Hu3DShadowCamBit = 0;
    for(index=0; index<HU3D_CAM_MAX; index++) {
        Hu3DShadowBuf[index].buf = NULL;
    }
    Hu3DShadowF = FALSE;
    Hu3DProjectionNum = 0;
    Hu3DCameraExistF = 0;
    modelKillAllF = FALSE;
    Hu3DPauseF = FALSE;
    Hu3DReflectModelId = HU3D_MODELID_NONE;
    Hu3DAmbColorSet(1, 1, 1);
    Hu3DShineF = FALSE;
    NoSyncF = FALSE;
}

/* Called once at the start of each main render frame before game logic and drawing. */
void Hu3DPreProc(void)
{
    GXColor shadowClear = { 0, 0, 0, 255 };
    s16 modelIndex;
    HU3D_MODEL *modelP;
    if(shadowNum && Hu3DShadowF) {
        GXSetCopyClear(shadowClear, GX_MAX_Z24);
    } else {
        GXSetCopyClear(BGColor, GX_MAX_Z24);
    }
    for(modelP = &Hu3DData[0], modelIndex=0; modelIndex<HU3D_MODEL_MAX; modelIndex++, modelP++) {
        if(modelP->hsf) {
            modelP->attr &= ~(HU3D_ATTR_MOTION_MODEL|HU3D_ATTR_MOT_EXEC);
        }
    }
    totalPolyCnted = totalPolyCnt;
    totalMatCnted = totalMatCnt;
    totalTexCnted = totalTexCnt;
    totalTexCacheCnted = totalTexCacheCnt;
    totalPolyCnt = totalMatCnt = totalTexCnt = totalTexCacheCnt = 0;
    GXSetAlphaUpdate(GX_TRUE);
}

static void Hu3DShadowExec(BOOL bgColorF);
static void Hu3DReflectModelExec(void);

#define HU3D_ATTR_CAMERA_UPDATE (HU3D_ATTR_CAMERA_MOTON|HU3D_ATTR_DISPOFF)
#define HU3D_ATTR_NOUPDATE_ALL (HU3D_ATTR_REFLECT_MODEL|HU3D_ATTR_MOTION_OFF|HU3D_ATTR_DISPOFF)

/* Called by main after game logic each frame to draw camera layers, models, and sprites. */
void Hu3DExec(void)
{
    GXColor clearColor = {};
    HU3D_MODEL *modelP;
    s16 i;
    HU3D_CAMERA *cameraP;
    s16 layer;
    HU3D_PROJECTION *projP;
    s16 vtxInvalidateF;
    s16 syncF;
    HU3D_LAYER_HOOK hookFunc;
    s16 cameraBit;
    s16 layerMdlNum;
    s16 shadowCameraNo;
    
    HuPerfBegin(HUPERF_USR1);
    GXSetCurrentMtx(GX_PNMTX0);
    shadowModelDrawF = FALSE;
    HuSprBegin();
    syncF = FALSE;
    if(Hu3DReflectModelId != HU3D_MODELID_NONE) {
        Hu3DReflectModelExec();
    }
    if(shadowNum != 0 && Hu3DShadowF) {
        cameraP = &Hu3DCamera[0];
        GXInvalidateVtxCache();
        if(Hu3DShadowCamBit == HU3D_CAM0) {
            Hu3DShadow = &Hu3DShadowBuf[0];
            Hu3DShadowExec(TRUE);
        } else {
            for(cameraP = &Hu3DCamera[0], i=shadowCameraNo=0; i<HU3D_CAM_MAX; i++, cameraP++) {
                if(cameraP->fov == -1) {
                    continue;
                }
                if(Hu3DShadowCamBit & Hu3DCameraBit) {
                    shadowCameraNo = i;
                }
            }
            for (cameraP = &Hu3DCamera[0], Hu3DCameraNo = 0; Hu3DCameraNo < HU3D_CAM_MAX;
                 Hu3DCameraNo++, cameraP++) {
                if(cameraP->fov == -1) {
                    continue;
                }
                cameraBit = (1 << Hu3DCameraNo);
                Hu3DCameraBit = cameraBit;
                if(Hu3DShadowCamBit & Hu3DCameraBit) {
                    Hu3DShadow = &Hu3DShadowBuf[Hu3DCameraNo];
                    if(shadowCameraNo == Hu3DCameraNo) {
                        Hu3DShadowExec(TRUE);
                    } else {
                        Hu3DShadowExec(FALSE);
                    }
                }
            }
        }
        if(NoSyncF == FALSE) {
            syncF = TRUE;
            GXSetDrawDone();
        }
    }
    for (cameraP = &Hu3DCamera[0], Hu3DCameraNo = 0; Hu3DCameraNo < HU3D_CAM_MAX;
         Hu3DCameraNo++, cameraP++) {
        if(cameraP->fov == -1) {
            continue;
        }
        GXInvalidateVtxCache();
        cameraBit = (1 << Hu3DCameraNo);
        Hu3DCameraBit = cameraBit;
        if(Hu3DShadowCamBit == HU3D_CAM0) {
            Hu3DShadow = &Hu3DShadowBuf[0];
        } else if(Hu3DShadowCamBit & Hu3DCameraBit) {
            Hu3DShadow = &Hu3DShadowBuf[Hu3DCameraNo];
        }
        for(projP=&Hu3DProjection[0], i=0; i<HU3D_PROJ_MAX; i++, projP++) {
            if(projP->anim) {
                MTXLookAt(projP->lookAtMtx, &projP->camPos, &projP->camUp, &projP->camTarget);
            }
        }
        if(Hu3DCameraNo == 0) {
            HuSprDispInit();
            HuSprExec(HUSPR_DRAWNO_BACK);
        }
        if(FogData.fogType != GX_FOG_NONE) {
            GXSetFog(FogData.fogType, FogData.fogStart, FogData.fogEnd, cameraP->near, cameraP->far,
                     FogData.fogColor);
        }
        for(layer=0; layer<HU3D_LAYER_MAX; layer++) {
            if(layerHook[Hu3DCameraNo][layer]) {
                Hu3DCameraSet(Hu3DCameraNo, Hu3DCameraMtx);
                MTXInvXpose(Hu3DCameraMtx, Hu3DCameraMtxXPose);
                hookFunc = layerHook[Hu3DCameraNo][layer];
                hookFunc(layer);
            }
            if(layerNum[layer]) {
                Hu3DDrawPreInit();
                Hu3DCameraSet(Hu3DCameraNo, Hu3DCameraMtx);
                MTXInvXpose(Hu3DCameraMtx, Hu3DCameraMtxXPose);
                for(modelP=&Hu3DData[0], layerMdlNum=i=0; i<HU3D_MODEL_MAX; i++, modelP++) {
                    if(!modelP->hsf) {
                        continue;
                    }
                    if(modelP->attr & HU3D_ATTR_CAMERA) {
                        Hu3DCameraMotionExec(i);
                        continue;
                    }
                    if ((modelP->attr & HU3D_ATTR_CAMERA_UPDATE) == HU3D_ATTR_CAMERA_UPDATE &&
                        modelP->motId != HU3D_MOTIONID_NONE) {
                        Hu3DMotionExec(i, modelP->motId, modelP->motWork.time, FALSE);
                    }
                    if(modelP->attr & HU3D_ATTR_NOUPDATE_ALL) {
                        continue;
                    }
                    if((modelP->cameraBit & cameraBit) == 0) {
                        continue;
                    }
                    if(modelP->layerNo != layer) {
                        continue;
                    }
                    if (((modelP->attr & HU3D_ATTR_MOT_EXEC) == 0 &&
                         (modelP->attr & HU3D_ATTR_MOT_SLOW) == 0) ||
                        ((modelP->attr & HU3D_ATTR_MOT_SLOW) != 0 && (modelP->tick & 1) != 0)) {
                        vtxInvalidateF = FALSE;
                        modelP->motAttr &= ~HU3D_MOTATTR;
                        if(modelP->motId != HU3D_MOTIONID_NONE) {
                            Hu3DMotionExec(i, modelP->motId, modelP->motWork.time, FALSE);
                        }
                        if(modelP->motIdShift != HU3D_MOTIONID_NONE) {
                            Hu3DSubMotionExec(i);
                        }
                        if(modelP->motIdOvl != HU3D_MOTIONID_NONE) {
                            Hu3DMotionExec(i, modelP->motIdOvl, modelP->motOvlWork.time, TRUE);
                        }
                        if(modelP->attr & HU3D_ATTR_CLUSTER_ON) {
                            ClusterMotionExec(modelP);
                            vtxInvalidateF = TRUE;
                        }
                        if(modelP->motIdShape != HU3D_MOTIONID_NONE) {
                            if(modelP->motId == HU3D_MOTIONID_NONE) {
                                Hu3DMotionExec(i, modelP->motIdShape, modelP->motShapeWork.time,
                                               FALSE);
                            } else {
                                Hu3DMotionExec(i, modelP->motIdShape, modelP->motShapeWork.time,
                                               TRUE);
                            }
                            vtxInvalidateF = TRUE;
                        }
                        if ((modelP->attr & (HU3D_ATTR_ENVELOPE_OFF | HU3D_ATTR_HOOKFUNC)) == 0 ||
                            (modelP->attr & HU3D_ATTR_MOTION_MODEL)) {
                            vtxInvalidateF = TRUE;
                            InitVtxParm(modelP->hsf);
                            if(modelP->motIdShape != HU3D_MOTIONID_NONE) {
                                ShapeProc(modelP->hsf);
                            }
                            if(modelP->attr & HU3D_ATTR_CLUSTER_ON) {
                                ClusterProc(modelP);
                            }
                            if(modelP->hsf->cenvNum) {
                                EnvelopeProc(modelP->hsf);
                            }
                            PPCSync();
                        }
                        if(vtxInvalidateF) {
                            GXInvalidateVtxCache();
                        }
                        modelP->attr |= HU3D_ATTR_MOT_EXEC;
                    }
                    if(syncF && (modelP->attr & HU3D_ATTR_HOOKFUNC)) {
                        GXWaitDrawDone();
                        syncF = FALSE;
                    }
                    if ((modelP->attr & HU3D_ATTR_HOOK) == 0 &&
                        (0.0f != modelP->scale.x || 0.0f != modelP->scale.y ||
                         0.0f != modelP->scale.z)) {
                        Mtx temp;
                        Mtx final;
                        mtxRot(temp, modelP->rot.x, modelP->rot.y, modelP->rot.z);
                        mtxScaleCat(temp, modelP->scale.x, modelP->scale.y, modelP->scale.z);
                        mtxTransCat(temp, modelP->pos.x, modelP->pos.y, modelP->pos.z);
                        PSMTXConcat(Hu3DCameraMtx, temp, final);
                        PSMTXConcat(final, modelP->mtx, final);
                        Hu3DDraw(modelP, final, &modelP->scale);
                    }
                    modelP->tick++;
                    layerMdlNum++;
                    if(layerMdlNum >= layerNum[layer]) {
                        break;
                    }
                }
                Hu3DDrawPost();
            }
            if(layerHook[Hu3DCameraNo][layer+HU3D_LAYER_HOOK_POST]) {
                Hu3DCameraSet(Hu3DCameraNo, Hu3DCameraMtx);
                MTXInvXpose(Hu3DCameraMtx, Hu3DCameraMtxXPose);
                hookFunc = layerHook[Hu3DCameraNo][layer+HU3D_LAYER_HOOK_POST];
                hookFunc(layer+HU3D_LAYER_HOOK_POST);
            }
        }
        if(!NoSyncF) {
            syncF = TRUE;
            GXSetDrawDone();
        }
    }
    HuSprDispInit();
    HuSprExec(HUSPR_DRAWNO_FRONT);
    for(modelP=&Hu3DData[0], i=0; i<HU3D_MODEL_MAX; i++, modelP++) {
        if(!modelP->hsf) {
            continue;
        }
        if ((modelP->motId != HU3D_MOTIONID_NONE || (modelP->attr & HU3D_ATTR_CLUSTER_ON) != 0 ||
             modelP->motIdShape != HU3D_MOTIONID_NONE) &&
            (Hu3DPauseF == 0 || (modelP->attr & HU3D_ATTR_NOPAUSE) != 0)) {
            Hu3DMotionNext(i);
        }
    }
    HuSprFinish();
    Hu3DAnimExec();
    HuPerfEnd(HUPERF_USR1);
    (void)hookFunc;
}

/* Called when an overlay or save flow closes its scene to release all 3D resources. */
void Hu3DAllKill(void)
{
    s16 index;
    s16 cameraIndex;
    Hu3DModelAllKill();
    Hu3DMotionAllKill();
    Hu3DCameraAllKill();
    Hu3DLightAllKill();
    Hu3DAnimAllKill();
    if(reflectAnim[0] != (void *)refMapData0) {
        HuMemDirectFree(reflectAnim[0]);
    }
    reflectAnim[0] = HuSprAnimRead(refMapData0);
    for(index=0; index<HU3D_CAM_MAX; index++) {
        if(Hu3DShadowBuf[index].buf) {
            HuMemDirectFree(Hu3DShadowBuf[index].buf);
            Hu3DShadowBuf[index].buf = NULL;
        }
    }
    shadowNum = 0;
    Hu3DShadowF = 0;
    Hu3DFogClear();
    for(index=0; index<HU3D_LAYER_MAX; index++) {
        layerNum[index] = 0;
    }
    for(cameraIndex=0; cameraIndex<HU3D_CAM_MAX; cameraIndex++) {
        for(index=0; index<HU3D_LAYER_HOOK_MAX; index++) {
            layerHook[cameraIndex][index] = NULL;
        }
    }
    
    for(index=0; index<HU3D_PROJ_MAX; index++) {
        if(Hu3DProjection[index].anim) {
            Hu3DProjectionKill(index);
        }
        Hu3DProjection[index].anim = NULL;
    }
    Hu3DAmbColorSet(1, 1, 1);
    Hu3DBGColorSet(0, 0, 0);
    NoSyncF = FALSE;
    Hu3DShineF = FALSE;
}

/* Sets the background clear color's RGB channels; the existing alpha is preserved. */
void Hu3DBGColorSet(u8 r, u8 g, u8 b)
{
    BGColor.r = r;
    BGColor.g = g;
    BGColor.b = b;
}

/* Registers a callback for each selected camera at the requested render layer. */
void Hu3DCameraLayerHookSet(s16 cameraMask, s16 layerNo, HU3D_LAYER_HOOK hook)
{
    s16 cameraIndex;
    for(cameraIndex=0; cameraIndex<HU3D_CAM_MAX; cameraIndex++) {
        if((1 << cameraIndex) & cameraMask) {
            layerHook[cameraIndex][layerNo] = hook;
        }
    }
}

/* Removes the callback at the requested layer for each selected camera. */
void Hu3DCameraLayerHookReset(s16 cameraMask, s16 layerNo)
{
    s16 cameraIndex;
    for(cameraIndex=0; cameraIndex<HU3D_CAM_MAX; cameraIndex++) {
        if((1 << cameraIndex) & cameraMask) {
            layerHook[cameraIndex][layerNo] = NULL;
        }
    }
}

/* Registers one layer callback for every camera. */
void Hu3DLayerHookSet(s16 layerNo, HU3D_LAYER_HOOK hook)
{
    Hu3DCameraLayerHookSet(HU3D_CAM_ALL, layerNo, hook);
}

/* Removes a layer callback from every camera. */
void Hu3DLayerHookReset(s16 layerNo)
{
    Hu3DCameraLayerHookReset(HU3D_CAM_ALL, layerNo);
}

/* Controls whether model motion time advances in Hu3DExec; models still render. */
void Hu3DPauseSet(BOOL paused)
{
    Hu3DPauseF = paused;
}

/* Suppresses draw-completion synchronization in Hu3DExec while enabled. */
void Hu3DNoSyncSet(BOOL noSync)
{
    NoSyncF = noSync;
}

/* Creates a model from HSF data; callers then set its transform, camera, and layer as needed. */
HU3D_MODELID Hu3DModelCreate(void *hsfData)
{
    HU3D_MODEL *modelP;
    s16 modelId;
    s16 index;
    for(modelP=&Hu3DData[0], modelId=0; modelId<HU3D_MODEL_MAX; modelId++, modelP++) {
        if(!modelP->hsf) {
            break;
        }
    }
    if(modelId == HU3D_MODEL_MAX) {
        OSReport("Error: Create Model Over!\n");
        return HU3D_MODELID_NONE;
    }
    modelP->hsf = LoadHSF(hsfData);
    modelP->linkMdlId = HU3D_MODELID_NONE;
    modelP->mallocNo = Hu3DMallocNo = (u32)modelP->hsf;
    modelP->attr = HU3D_ATTR_NONE;
    modelP->motAttr = HU3D_MOTATTR_NONE;
    modelP->projBit = 0;
    MakeDisplayList(modelId, modelP->mallocNo);
    modelP->motWork.speed = 1.0f;
    for(index=0; index<HU3D_CLUSTER_MAX; index++) {
        modelP->motIdCluster[index] = HU3D_MOTIONID_NONE;
    }
    modelP->motIdOvl = HU3D_MOTIONID_NONE;
    modelP->motIdShift = HU3D_MOTIONID_NONE;
    modelP->motIdShape = HU3D_MOTIONID_NONE;
    modelP->motWork.time = 0.0f;
    modelP->timingHook = NULL;
    modelP->matHook = NULL;
    if(modelP->hsf->motionNum) {
        modelP->attr |= HU3D_ATTR_MOTION_MODEL;
        modelP->motId = modelP->motIdSrc = Hu3DMotionModelCreate(modelId);
        if(modelP->hsf->cenvNum) {
            Hu3DMotionExec(modelId, modelP->motId, 0, FALSE);
            EnvelopeProc(modelP->hsf);
            PPCSync();
        }
        if(modelP->hsf->clusterNum) {
            Hu3DMotionClusterSet(modelId, modelP->motId);
        }
        if(modelP->hsf->shapeNum) {
            Hu3DMotionShapeSet(modelId, modelP->motId);
        }
        modelP->motWork.start = 0;
        modelP->motWork.end = Hu3DMotionMaxTimeGet(modelId);
    } else {
        modelP->motIdSrc = modelP->motId = HU3D_MOTIONID_NONE;
    }
    modelP->pos.x = modelP->pos.y = modelP->pos.z = 0;
    modelP->rot.x = modelP->rot.y = modelP->rot.z = 0;
    modelP->scale.x = modelP->scale.y = modelP->scale.z = 1;
    modelP->cameraBit = HU3D_CAM_ALL;
    modelP->layerNo = 0;
    modelP->hookData = NULL;
    modelP->lightNum = 0;
    modelP->hiliteIdx = 0;
    modelP->ambR = modelP->ambG = modelP->ambB = 1;
    modelP->reflectType = HU3D_REFLECT_TYPE_NONE;
    for(index=0; index<HU3D_MODEL_LLIGHT_MAX; index++) {
        modelP->LLightId[index] = HU3D_LIGHTID_NONE;
    }
    modelP->lightBit = MODEL_GLOBAL_LIGHTS_ALL;
    modelP->camInfoBit = 0;
    modelP->tick = modelId;
    MTXIdentity(modelP->mtx);
    layerNum[0]++;
    if(modelP->hsf->sceneNum && (modelP->hsf->scene->fogStart || modelP->hsf->scene->fogEnd)) {
        Hu3DFogSet(modelP->hsf->scene->fogStart, modelP->hsf->scene->fogEnd,
                   modelP->hsf->scene->fogColor.r, modelP->hsf->scene->fogColor.g,
                   modelP->hsf->scene->fogColor.b);
    }
    return modelId;
}

/* Creates a linked instance from an existing model and copies its render and motion setup. */
HU3D_MODELID Hu3DModelLink(HU3D_MODELID linkMdlId)
{
    HU3D_MODEL *linkModelP = &Hu3DData[linkMdlId];
    HU3D_MODEL *modelP;
    HSF_OBJECT *duplicatedObjects;
    s16 index;
    s16 modelId;
    u32 file;
    for(modelP=&Hu3DData[0], modelId=0; modelId<HU3D_MODEL_MAX; modelId++, modelP++) {
        if(!modelP->hsf) {
            break;
        }
    }
    if(modelId == HU3D_MODEL_MAX) {
        return HU3D_MODELID_NONE;
    }
    modelP->hsfLink = linkModelP->hsf;
    modelP->hsf = HuMemDirectMalloc(HEAP_MODEL, sizeof(HSF_DATA));
    modelP->linkMdlId = linkMdlId;
    file = HuMemMemoryFileGet(linkModelP->hsf)|MODEL_LINK_FILE_FLAG;
    HuMemMemoryFileSet(modelP->hsf, file);
    modelP->mallocNoLink = (u32)modelP->hsf;
    *modelP->hsf = *linkModelP->hsf;
    duplicatedObjects = Hu3DObjDuplicate(modelP->hsf, modelP->mallocNoLink);
    modelP->hsf->root = (HSF_OBJECT *) ((char *) duplicatedObjects +
                                        ((u32) modelP->hsf->root - (u32) modelP->hsf->object));
    modelP->hsf->object = duplicatedObjects;
    Hu3DAttrDuplicate(modelP->hsf, modelP->mallocNoLink);
    Hu3DMatDuplicate(modelP->hsf, modelP->mallocNoLink);
    modelP->mallocNo = linkModelP->mallocNo;
    modelP->attr = linkModelP->attr;
    if(linkModelP->attr & HU3D_ATTR_SHADOW) {
        shadowNum++;
    }
    linkModelP->attr |= HU3D_ATTR_LINK;
    modelP->motAttr = linkModelP->motAttr;
    modelP->pos.x = modelP->pos.y = modelP->pos.z = 0;
    modelP->rot.x = modelP->rot.y = modelP->rot.z = 0;
    modelP->scale.x = modelP->scale.y = modelP->scale.z = 1;
    modelP->motId = linkModelP->motId;
    if(modelP->motId != HU3D_MOTIONID_NONE) {
        modelP->motWork.start = 0;
        modelP->motWork.end = Hu3DMotionMaxTimeGet(modelId);
    }
    modelP->motIdShift = modelP->motIdOvl = HU3D_MOTIONID_NONE;
    modelP->motIdShape = linkModelP->motIdShape;
    modelP->motShapeWork.time = 0;
    modelP->motShapeWork.speed = linkModelP->motShapeWork.speed;
    modelP->motShapeWork.start = linkModelP->motShapeWork.start;
    modelP->motShapeWork.end = linkModelP->motShapeWork.end;
    for(index=0; index<HU3D_CLUSTER_MAX; index++) {
        modelP->motIdCluster[index] = linkModelP->motIdCluster[index];
        if(modelP->motIdCluster[index] != HU3D_MOTIONID_NONE) {
            modelP->clusterTime[index] = 0;
            modelP->clusterSpeed[index] = linkModelP->clusterSpeed[index];
            modelP->clusterAttr[index] = linkModelP->clusterAttr[index];
            modelP->attr |= HU3D_ATTR_CLUSTER_ON;
            ClusterAdjustObject(modelP->hsf, Hu3DMotion[modelP->motIdCluster[index]].hsf);
        }
    }
    modelP->motWork.time = linkModelP->motWork.time;
    modelP->motWork.speed = linkModelP->motWork.speed;
    modelP->motIdSrc = HU3D_MOTIONID_NONE;
    modelP->cameraBit = HU3D_CAM_ALL;
    modelP->layerNo = 0;
    modelP->hookData = NULL;
    modelP->lightNum = 0;
    modelP->hiliteIdx = 0;
    modelP->projBit = 0;
    modelP->ambR = modelP->ambG = modelP->ambB = 1;
    modelP->reflectType = HU3D_REFLECT_TYPE_NONE;
    
    for(index=0; index<HU3D_MODEL_LLIGHT_MAX; index++) {
        modelP->LLightId[index] = HU3D_LIGHTID_NONE;
    }
    modelP->lightBit = MODEL_GLOBAL_LIGHTS_ALL;
    modelP->camInfoBit = 0;
    MTXIdentity(modelP->mtx);
    layerNum[0]++;
    return modelId;
}

/* Creates a model slot whose callback the HSF drawing pipeline invokes for draw objects. */
HU3D_MODELID Hu3DHookFuncCreate(HU3D_MODEL_HOOK hookFunc)
{
    HU3D_MODEL *modelP;
    s16 index;
    s16 modelId;
    for(modelP=&Hu3DData[0], modelId=0; modelId<HU3D_MODEL_MAX; modelId++, modelP++) {
        if(!modelP->hsf) {
            break;
        }
    }
    if(modelId == HU3D_MODEL_MAX) {
        return HU3D_MODELID_NONE;
    }
    modelP->hookFunc = hookFunc;
    modelP->mallocNo = modelId+HOOKFUNC_MALLOCNO;
    modelP->attr = HU3D_ATTR_HOOKFUNC;
    modelP->motAttr = HU3D_MOTATTR_NONE;
    modelP->pos.x = modelP->pos.y = modelP->pos.z = 0;
    modelP->rot.x = modelP->rot.y = modelP->rot.z = 0;
    modelP->scale.x = modelP->scale.y = modelP->scale.z = 1;
    modelP->motId = modelP->motIdShift = modelP->motIdOvl = modelP->motIdShape = HU3D_MOTIONID_NONE;
    for(index=0; index<HU3D_CLUSTER_MAX; index++) {
        modelP->motIdCluster[index] = HU3D_MOTIONID_NONE;
    }
    modelP->motWork.time = 0;
    modelP->motWork.speed = 1;
    modelP->motIdSrc = HU3D_MOTIONID_NONE;
    modelP->timingHook = NULL;
    modelP->matHook = NULL;
    modelP->cameraBit = HU3D_CAM_ALL;
    modelP->layerNo = 0;
    modelP->hookData = NULL;
    modelP->lightNum = 0;
    modelP->hiliteIdx = 0;
    modelP->linkMdlId = HU3D_MODELID_NONE;
    modelP->projBit = 0;
    modelP->reflectType = HU3D_REFLECT_TYPE_NONE;
    for(index=0; index<HU3D_MODEL_LLIGHT_MAX; index++) {
        modelP->LLightId[index] = HU3D_LIGHTID_NONE;
    }
    modelP->lightBit = MODEL_GLOBAL_LIGHTS_ALL;
    modelP->camInfoBit = 0;
    MTXIdentity(modelP->mtx);
    layerNum[0]++;
    return modelId;
}

/* Releases a model and its owned resources when an overlay or scene removes it. */
void Hu3DModelKill(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HSF_DATA *hsf = modelP->hsf;
    HU3D_MODEL *iterModelP;
    s16 index;
    s16 sharedModelCount;
    if(!hsf) {
        return;
    }
    layerNum[modelP->layerNo]--;
    if(modelP->attr & HU3D_ATTR_HOOKFUNC) {
        HuMemDirectFreeNum(HEAP_MODEL, modelP->mallocNo);
        if(modelP->attr & HU3D_ATTR_PARTICLE) {
            HU3D_PARTICLE *particleP = modelP->hookData;
            HuSprAnimKill(particleP->anim);
        }
        if(modelP->attr & HU3D_ATTR_SHADOW) {
            shadowNum--;
        }
        modelP->hookFunc = NULL;
        return;
    }
    if(modelP->attr & HU3D_ATTR_CAMERA) {
        if(modelP->motId != HU3D_MOTIONID_NONE) {
            Hu3DMotionKill(modelP->motId);
        }
        HuMemDirectFreeNum(HEAP_MODEL, modelP->mallocNo);
        if(modelP->attr & HU3D_ATTR_SHADOW) {
            shadowNum--;
        }
        modelP->hsf = NULL;
        return;
    }
    if(modelP->attr & HU3D_ATTR_DIE) {
        Hu3DModelDieKill(modelId);
    }
    Hu3DAnimModelKill(modelId);
    if(modelP->linkMdlId != HU3D_MODELID_NONE) {
        HuMemDirectFree(modelP->hsf);
        HuMemDirectFreeNum(HEAP_MODEL, modelP->mallocNoLink);
        hsf = modelP->hsfLink;
        modelP->hsf = hsf;
    }
    for (iterModelP = &Hu3DData[0], sharedModelCount = index = 0; index < HU3D_MODEL_MAX;
         index++, iterModelP++) {
        if(!iterModelP->hsf) {
            continue;
        }
        if (iterModelP->hsf == hsf ||
            (iterModelP->linkMdlId != HU3D_MODELID_NONE && iterModelP->hsfLink == hsf)) {
            sharedModelCount++;
        }
    }
    if(sharedModelCount > 1) {
        if(modelP->attr & HU3D_ATTR_SHADOW) {
            shadowNum--;
        }
        modelP->hsf = NULL;
        iterModelP=&Hu3DData[0];
        if(modelP->motIdSrc != HU3D_MOTIONID_NONE) {
            for(index=0; index<HU3D_MODEL_MAX; index++, iterModelP++) {
                if (iterModelP->hsf && iterModelP->linkMdlId != HU3D_MODELID_NONE &&
                    iterModelP->hsfLink == hsf) {
                    Hu3DMotion[modelP->motIdSrc].modelId = index;
                    
                    break;
                }
            }
        }
        return;
    }
    if(modelP->motIdSrc != HU3D_MOTIONID_NONE && Hu3DMotionKill(modelP->motIdSrc) == FALSE) {
        Hu3DMotion[modelP->motIdSrc].modelId = HU3D_MOTIONID_NONE;
        HuMemDirectFreeNum(HEAP_MODEL, modelP->mallocNo);
        if(modelP->attr & HU3D_ATTR_SHADOW) {
            shadowNum--;
        }
        modelP->hsf = NULL;
        return;
    }
    HuMemDirectFree(modelP->hsf);
    HuMemDirectFreeNum(HEAP_MODEL, modelP->mallocNo);
    for(index=0; index<modelP->lightNum; index++) {
        Hu3DGLightKill(modelP->lightId[index]);
    }
    for(index=0; index<HU3D_MODEL_LLIGHT_MAX; index++) {
        if(modelP->LLightId[index] != HU3D_LIGHTID_NONE) {
            Hu3DLLightKill(modelId, index);
        }
    }
    if(modelP->attr & HU3D_ATTR_SHADOW) {
        shadowNum--;
    }
    modelP->hsf = NULL;
    return;
}

/* Called by Hu3DAllKill to remove every model and clear camera-layer hooks. */
void Hu3DModelAllKill(void)
{
    s16 index;
    s16 cameraIndex;
    HU3D_MODEL *modelP;
    
    modelKillAllF = TRUE;
    for(modelP=&Hu3DData[0], index=0; index<HU3D_MODEL_MAX; index++, modelP++) {
        if(modelP->hsf) {
            Hu3DModelKill(index);
        }
    }
    modelKillAllF = FALSE;
    for(index=0; index<HU3D_LAYER_MAX; index++) {
        layerNum[index] = 0;
    }
    for(cameraIndex=0; cameraIndex<HU3D_CAM_MAX; cameraIndex++) {
        for(index=0; index<HU3D_LAYER_HOOK_MAX; index++) {
            layerHook[cameraIndex][index] = NULL;
        }
    }
    
    Hu3DParManAllKill();
    HuMemDCFlush(HEAP_MODEL);
    Hu3DReflectModelId = HU3D_MODELID_NONE;
}

/* Model setup and animation callers use this to place a model in world space. */
void Hu3DModelPosSet(HU3D_MODELID modelId, float posX, float posY, float posZ)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->pos.x = posX;
    modelP->pos.y = posY;
    modelP->pos.z = posZ;
}

/* Model setup copies a complete world-space position into the selected model record. */
void Hu3DModelPosSetV(HU3D_MODELID modelId, HuVecF *pos)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->pos = *pos;
}

/* Model queries copy the selected model's stored world-space position to the caller. */
void Hu3DModelPosGet(HU3D_MODELID modelId, HuVecF *pos)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    *pos = modelP->pos;
}

/* Model setup and animation callers use this to set the model's three rotation components. */
void Hu3DModelRotSet(HU3D_MODELID modelId, float rotX, float rotY, float rotZ)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->rot.x = rotX;
    modelP->rot.y = rotY;
    modelP->rot.z = rotZ;
}

/* Model setup copies all three rotation components into the selected model record. */
void Hu3DModelRotSetV(HU3D_MODELID modelId, HuVecF *rot)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->rot = *rot;
}

/* Model queries copy the selected model's stored rotation vector to the caller. */
void Hu3DModelRotGet(HU3D_MODELID modelId, HuVecF *rot)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    *rot = modelP->rot;
}

/* Model setup callers use this to set independent scale factors on each model axis. */
void Hu3DModelScaleSet(HU3D_MODELID modelId, float scaleX, float scaleY, float scaleZ)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->scale.x = scaleX;
    modelP->scale.y = scaleY;
    modelP->scale.z = scaleZ;
}

/* Model setup copies all three scale factors into the selected model record. */
void Hu3DModelScaleSetV(HU3D_MODELID modelId, HuVecF *scale)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->scale = *scale;
}

/* Model queries copy the selected model's stored scale vector to the caller. */
void Hu3DModelScaleGet(HU3D_MODELID modelId, HuVecF *scale)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    *scale = modelP->scale;
}

/* Model setup callers can replace the model's stored transform matrix with this value. */
void Hu3DModelMtxSet(HU3D_MODELID modelId, Mtx *mtx)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    MTXCopy(*mtx, modelP->mtx);
    
}

/* Model queries copy the model's stored transform matrix into the caller's matrix. */
void Hu3DModelMtxGet(HU3D_MODELID modelId, Mtx *mtx)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    MTXCopy(modelP->mtx, *mtx);
}

/* Model setup callers enable model or motion attributes; HU3D_MOTATTR routes bits to motAttr. */
void Hu3DModelAttrSet(HU3D_MODELID modelId, u32 attr)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    if(attr & HU3D_MOTATTR) {
        modelP->motAttr |= attr & ~HU3D_MOTATTR;
        modelP->attr |= HU3D_ATTR_MOTION_MODEL;
    } else {
        modelP->attr |= attr;
    }
}

/* Model setup callers clear model or motion attributes; HU3D_MOTATTR selects the motion mask. */
void Hu3DModelAttrReset(HU3D_MODELID modelId, u32 attr)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    if(attr & HU3D_MOTATTR) {
        modelP->motAttr &= ~attr;
    } else {
        modelP->attr &= ~attr;
    }
}

/* Model queries return the model-level attribute mask. */
u32 Hu3DModelAttrGet(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    return modelP->attr;
}

/* Motion queries return the motion mask with HU3D_MOTATTR set to identify its bit domain. */
u32 Hu3DModelMotionAttrGet(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    return modelP->motAttr|HU3D_MOTATTR;
}

/* Motion setup enables the requested attribute bits on one model cluster. */
void Hu3DModelClusterAttrSet(HU3D_MODELID modelId, s16 clusterNo, s32 clusterAttr)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->clusterAttr[clusterNo] |= clusterAttr;
}

/* Motion setup clears the requested attribute bits on one model cluster. */
void Hu3DModelClusterAttrReset(HU3D_MODELID modelId, s16 clusterNo, s32 clusterAttr)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->clusterAttr[clusterNo] &= ~clusterAttr;
}

/* Render setup restricts the model to the supplied camera-selection bit mask. */
void Hu3DModelCameraSet(HU3D_MODELID modelId, u16 cameraBit)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->cameraBit = cameraBit;
}

/* Render setup moves a model between layers while keeping each layer's model count current. */
void Hu3DModelLayerSet(HU3D_MODELID modelId, s16 layerNo)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    layerNum[modelP->layerNo]--;
    modelP->layerNo = layerNo;
    layerNum[layerNo]++;
}

/* Model and effect setup uses the normalized HSF object name to find its object record. */
HSF_OBJECT *Hu3DModelObjPtrGet(HU3D_MODELID modelId, char *objName)
{
    char name[HSF_OBJNAME_MAX_LEN];
    HSF_DATA *hsf = Hu3DData[modelId].hsf;
    s16 i;
    HSF_OBJECT *objPtr = hsf->object;
    strcpy(name, MakeObjectName((s8 *)objName));
    for(i=0; i<hsf->objectNum; objPtr++, i++) {
        HSF_OBJECT *obj = objPtr;
        if(strcmp(name, obj->name) == 0) {
            return objPtr;
        }
    }
    if(i == hsf->objectNum) {
        OSReport("Error: OBJPtr Error! %s\n", objName);
    }
    return NULL;
}

/* Model setup applies one transparency level to every material and marks mesh data translucent. */
void Hu3DModelTPLvlSet(HU3D_MODELID modelId, float tpLvl)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HSF_DATA *hsf = modelP->hsf;
    HSF_MATERIAL *matPtr;
    HSF_OBJECT *objPtr;
    s16 i;
    for(matPtr = hsf->material, i=0; i<hsf->materialNum; i++, matPtr++) {
        matPtr->invAlpha = 1.0f-tpLvl;
        if(tpLvl != 1.0f) {
            HSF_MATERIAL_SETPASS(matPtr, 1);
        } else {
            HSF_MATERIAL_SETPASS(matPtr, 0);
        }
    }
    for(objPtr = hsf->object, i=0; i<hsf->objectNum; objPtr++, i++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->type == HSF_OBJ_MESH) {
            HSF_CONSTDATA *constData = obj->constData;
            constData->attr |= HU3D_CONST_XLU;
        }
    }
    modelP->attr |= HU3D_ATTR_TPLVL_SET;
}

/* Model setup assigns a highlight texture to every mesh object in the model. */
void Hu3DModelHiliteMapSet(HU3D_MODELID modelId, ANIMDATA *hiliteMap)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HSF_DATA *hsf = modelP->hsf;
    s16 i;
    HSF_OBJECT *objPtr;
    
    for(objPtr = hsf->object, i=0; i<hsf->objectNum; objPtr++, i++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->type == HSF_OBJ_MESH) {
            HSF_CONSTDATA *constData;
            obj->flags |= HSF_MATERIAL_HILITE;
            constData = obj->constData;
            constData->attr |= HU3D_CONST_HILITE;
            constData->hiliteMap = hiliteMap;
        }
    }
}

/* Render setup enables shadow casting on the model and its objects with constant data. */
void Hu3DModelShadowSet(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HSF_DATA *hsf = modelP->hsf;
    s16 i;
    HSF_OBJECT *objPtr;
    if(!(modelP->attr & HU3D_ATTR_SHADOW)) {
        shadowNum++;
    }
    modelP->attr |= HU3D_ATTR_SHADOW;
    for(objPtr = hsf->object, i=0; i<hsf->objectNum;  i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->constData) {
            HSF_CONSTDATA *constData;
            constData = obj->constData;
            constData->attr |= HU3D_CONST_SHADOW;
        }
    }
}

/* Render setup disables shadow casting and removes the shadow flag from the model's objects. */
void Hu3DModelShadowReset(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HSF_DATA *hsf = modelP->hsf;
    s16 i;
    HSF_OBJECT *objPtr;
    if(modelP->attr & HU3D_ATTR_SHADOW) {
        shadowNum--;
    }
    modelP->attr &= ~HU3D_ATTR_SHADOW;
    for(objPtr = hsf->object, i=0; i<hsf->objectNum;  i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->constData) {
            HSF_CONSTDATA *constData;
            constData = obj->constData;
            constData->attr &= ~HU3D_CONST_SHADOW;
        }
    }
}

void Hu3DModelShadowDispOn(HU3D_MODELID modelId)
{
    Hu3DModelAttrSet(modelId, HU3D_ATTR_SHADOW);
}

void Hu3DModelShadowDispOff(HU3D_MODELID modelId)
{
    Hu3DModelAttrReset(modelId, HU3D_ATTR_SHADOW);
}

/* Render setup marks every object with constant data for shadow-map rendering. */
void Hu3DModelShadowMapSet(HU3D_MODELID modelId)
{
    HSF_DATA *hsf = Hu3DData[modelId].hsf;
    s16 i;
    HSF_OBJECT *objPtr;

    for(objPtr = hsf->object, i=0; i<hsf->objectNum;  i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->constData) {
            HSF_CONSTDATA *constData;
            constData = obj->constData;
            constData->attr |= HU3D_CONST_SHADOW_MAP;
        }
    }
}

/* Render setup enables shadow-map rendering for the named object when it has constant data. */
void Hu3DModelShadowMapObjSet(HU3D_MODELID modelId, char *objName)
{
    char name[HSF_OBJNAME_MAX_LEN];
    HSF_DATA *hsf = Hu3DData[modelId].hsf;
    s16 i;
    HSF_OBJECT *objPtr = hsf->object;
    strcpy(name, MakeObjectName((s8 *)objName));
    for(i=0; i<hsf->objectNum; i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->constData && !strcmp(name, obj->name)) {
            HSF_CONSTDATA *constData;
            constData = obj->constData;
            constData->attr |= HU3D_CONST_SHADOW_MAP;
            break;
        }
    }
}

/* Render setup disables shadow-map rendering for the named object when it has constant data. */
void Hu3DModelShadowMapObjReset(HU3D_MODELID modelId, char *objName)
{
    char name[HSF_OBJNAME_MAX_LEN];
    HSF_DATA *hsf = Hu3DData[modelId].hsf;
    s16 i;
    HSF_OBJECT *objPtr = hsf->object;
    strcpy(name, MakeObjectName((s8 *)objName));
    for(i=0; i<hsf->objectNum; i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->constData && !strcmp(name, obj->name)) {
            HSF_CONSTDATA *constData;
            constData = obj->constData;
            constData->attr &= ~HU3D_CONST_SHADOW_MAP;
            break;
        }
    }
}

/* Render setup assigns a byte alpha derived from tpLvl to every object's shadow map. */
void Hu3DModelShadowMapTPLvlSet(HU3D_MODELID modelId, float tpLvl)
{
    HSF_DATA *hsf = Hu3DData[modelId].hsf;
    s16 i;
    HSF_OBJECT *objPtr = hsf->object;
    for(i=0; i<hsf->objectNum; i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->constData) {
            HSF_CONSTDATA *constData;
            constData = obj->constData;
            constData->attr |= HU3D_CONST_SHADOW_MAP|HU3D_CONST_SHADOW_MAP_TPLVL;
            constData->shadowAlpha = tpLvl*255;
        }
    }
}

/* Render setup assigns the requested shadow-map alpha to the named object only. */
void Hu3DModelShadowMapObjTPLvlSet(HU3D_MODELID modelId, char *objName, float tpLvl)
{
    char name[HSF_OBJNAME_MAX_LEN];
    HSF_DATA *hsf = Hu3DData[modelId].hsf;
    s16 i;
    HSF_OBJECT *objPtr = hsf->object;
    strcpy(name, MakeObjectName((s8 *)objName));
    for(i=0; i<hsf->objectNum; i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->constData && !strcmp(name, obj->name)) {
            HSF_CONSTDATA *constData;
            constData = obj->constData;
            constData->attr |= HU3D_CONST_SHADOW_MAP|HU3D_CONST_SHADOW_MAP_TPLVL;
            constData->shadowAlpha = tpLvl*255;
            break;
        }
    }
}

/* Render setup clears shadow-map flags from all model objects with constant data. */
void Hu3DModelShadowMapReset(HU3D_MODELID modelId)
{
    HSF_DATA *hsf = Hu3DData[modelId].hsf;
    s32 i;
    HSF_OBJECT *objPtr = hsf->object;
    for(i=0; i<hsf->objectNum; i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->constData) {
            HSF_CONSTDATA *constData;
            constData = obj->constData;
            constData->attr &= ~HU3D_CONST_SHADOW_MAP;
        }
    }
}

/* Lighting setup stores the model's ambient red, green, and blue components. */
void Hu3DModelAmbSet(HU3D_MODELID modelId, float ambR, float ambG, float ambB)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->ambR = ambR;
    modelP->ambG = ambG;
    modelP->ambB = ambB;
}

/* Attachment setup hooks hookMdlId to a named object after calculating the source model motion. */
void Hu3DModelHookSet(HU3D_MODELID modelId, char *objName, HU3D_MODELID hookMdlId)
{
    char name[HSF_OBJNAME_MAX_LEN];
    HSF_DATA *hsf = Hu3DData[modelId].hsf;
    s16 i;
    HSF_OBJECT *objPtr;
    
    Hu3DMotionCalc(modelId);
    objPtr = hsf->object;
    strcpy(name, MakeObjectName((s8 *)objName));
    for(i=0; i<hsf->objectNum; i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->constData && !strcmp(name, obj->name)) {
            HSF_CONSTDATA *constData = obj->constData;
            constData->hookMdlId = hookMdlId;
            Hu3DModelAttrSet(hookMdlId, HU3D_ATTR_HOOK);
            return;
        }
    }
    OSReport( "Error: Not Found %s for HookSet\n", objName);
}

/* Attachment cleanup clears each attached model hook attribute and resets its object hook ID. */
void Hu3DModelHookReset(HU3D_MODELID modelId)
{
    HSF_DATA *hsf = Hu3DData[modelId].hsf;
    s16 i;
    HSF_OBJECT *objPtr = hsf->object;
    for(i=0; i<hsf->objectNum; i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->constData) {
            HSF_CONSTDATA *constData = obj->constData;
            if(constData->hookMdlId != HU3D_MODELID_NONE) {
                Hu3DModelAttrReset(constData->hookMdlId, HU3D_ATTR_HOOK);
                constData->hookMdlId = HU3D_MODELID_NONE;
            }
        }
    }
}

/* Attachment cleanup releases the hook stored on one named object. */
void Hu3DModelHookObjReset(HU3D_MODELID modelId, char *objName)
{
    char name[HSF_OBJNAME_MAX_LEN];
    HSF_DATA *hsf = Hu3DData[modelId].hsf;
    s16 i;
    HSF_OBJECT *objPtr = hsf->object;
    strcpy(name, MakeObjectName((s8 *)objName));
    for(i=0; i<hsf->objectNum; i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->constData && !strcmp(name, obj->name)) {
            HSF_CONSTDATA *constData = obj->constData;
            Hu3DModelAttrReset(constData->hookMdlId, HU3D_ATTR_HOOK);
            constData->hookMdlId = HU3D_MODELID_NONE;
            return;
        }
    }
    OSReport("Error: Not Found %s for HookReset\n", objName);
}

/* Sets the selected projection bit on the model. */
void Hu3DModelProjectionSet(HU3D_MODELID modelId, HU3D_PROJID projId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->projBit |= (1 << projId);
}

/* Clears the selected projection bit on the model. */
void Hu3DModelProjectionReset(HU3D_MODELID modelId, HU3D_PROJID projId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->projBit &= ~(1 << projId);
}

/* Render setup writes the highlight type into each material and enables model highlights. */
void Hu3DModelHiliteTypeSet(HU3D_MODELID modelId, s16 hiliteType)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HSF_DATA *hsf = modelP->hsf;
    HSF_MATERIAL *matPtr = hsf->material;
    s16 i;
    hiliteType = hiliteType << 4;
    hiliteType &= MATERIAL_HILITE_TYPE_MASK;
    for(i=0; i<hsf->materialNum; i++, matPtr++) {
        HSF_MATERIAL_SETHILITETYPE(matPtr, hiliteType);
        matPtr->flags |= HSF_MATERIAL_HILITE;
    }
    Hu3DModelAttrSet(modelId, HU3D_ATTR_HILITE);
}

/* Stores the model reflection type. */
void Hu3DModelReflectTypeSet(HU3D_MODELID modelId, s16 reflectType)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->reflectType = reflectType;
}

/* Reflection setup selects the model, allocates its 128-by-128 texture, and initializes the map. */
void Hu3DReflectModelSet(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP;
    Hu3DReflectModelId = modelId;
    modelP = &Hu3DData[modelId];
    modelP->attr |= HU3D_ATTR_REFLECT_MODEL;
    Hu3DReflectModelAnim = HuSprAnimMake(REFLECT_TEX_W, REFLECT_TEX_H, ANIM_BMP_RGB5A3);
    Hu3DReflectModelAnim->bmp->data = HuMemDirectMalloc(HEAP_MODEL, REFLECT_TEX_W*REFLECT_TEX_H*2);
    Hu3DReflectMapSet(Hu3DReflectModelAnim);
}

void Hu3DModelMatHookSet(HU3D_MODELID modelId, HU3D_MAT_HOOK matHook)
{
    Hu3DData[modelId].matHook = matHook;
}

HU3D_CAMERA defCamera = {
    45.0f,
    20.0f,
    5000.0f,
    HU_DISP_ASPECT,
    0.0f,
    {0.0f, 0.0f, 100.0f},
    {0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f},
    0, 0, HU_FB_WIDTH, HU_FB_HEIGHT,
    0.0f, 0.0f, HU_FB_WIDTH, HU_FB_HEIGHT,
    0.0f, 1.0f
};

/* Display setup initializes each selected camera from the current render-mode dimensions. */
void Hu3DCameraCreate(int cameraBit)
{
    s16 i;
    s16 bit;
    defCamera.viewportW = RenderMode->fbWidth;
    defCamera.viewportH = RenderMode->efbHeight;
    defCamera.scissorW = RenderMode->fbWidth;
    defCamera.scissorH = RenderMode->efbHeight;
    Hu3DCameraExistF |= cameraBit;
    for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
        if(cameraBit & bit) {
            HU3D_CAMERA *cameraP = &Hu3DCamera[i];
            *cameraP = defCamera;
        }
    }
}

/* Camera setup updates perspective parameters for every camera selected by cameraBit. */
void Hu3DCameraPerspectiveSet(int cameraBit, float fov, float near, float far, float aspect)
{
    s16 i;
    s16 bit;
    for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
        if(cameraBit & bit) {
            HU3D_CAMERA *cameraP = &Hu3DCamera[i];
            cameraP->fov = fov;
            cameraP->near = near;
            cameraP->far = far;
            cameraP->aspect = aspect;
        }
    }
}

/* Display setup sets viewport bounds for selected cameras; 240-line output halves viewport
 * height. */
void Hu3DCameraViewportSet(int cameraBit, float vpX, float vpY, float vpW, float vpH, float vpNearZ,
                           float vpFarZ)
{
    s16 i;
    s16 bit;
    if(RenderMode->xfbHeight == 240) {
        vpH *= 0.5f;
    }
    for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
        if(cameraBit & bit) {
            HU3D_CAMERA *cameraP = &Hu3DCamera[i];
            cameraP->viewportX = vpX;
            cameraP->viewportY = vpY;
            cameraP->viewportW = vpW;
            cameraP->viewportH = vpH;
            cameraP->viewportNear = vpNearZ;
            cameraP->viewportFar = vpFarZ;
        }
    }
}

/* Display setup sets the integer scissor rectangle for every selected camera. */
void Hu3DCameraScissorSet(int cameraBit, unsigned int scissorX, unsigned int scissorY,
                          unsigned int scissorW, unsigned int scissorH)
{
    s16 i;
    s16 bit;
    for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
        if(cameraBit & bit) {
            HU3D_CAMERA *cameraP = &Hu3DCamera[i];
            cameraP->scissorX = scissorX;
            cameraP->scissorY = scissorY;
            cameraP->scissorW = scissorW;
            cameraP->scissorH = scissorH;
        }
    }
}

/* Camera setup writes position, up direction, and look target for each selected camera. */
void Hu3DCameraPosSet(int cameraBit, float posX, float posY, float posZ, float upX, float upY,
                      float upZ, float targetX, float targetY, float targetZ)
{
    s16 i;
    s16 bit;
    for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
        if(cameraBit & bit) {
            HU3D_CAMERA *cameraP = &Hu3DCamera[i];
            cameraP->pos.x = posX;
            cameraP->pos.y = posY;
            cameraP->pos.z = posZ;
            cameraP->up.x = upX;
            cameraP->up.y = upY;
            cameraP->up.z = upZ;
            cameraP->target.x = targetX;
            cameraP->target.y = targetY;
            cameraP->target.z = targetZ;
        }
    }
}

/* Camera setup copies position, up direction, and look target to each selected camera. */
void Hu3DCameraPosSetV(int cameraBit, Vec *pos, Vec *up, Vec *target)
{
    s16 i;
    s16 bit;
    for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
        if(cameraBit & bit) {
            HU3D_CAMERA *cameraP = &Hu3DCamera[i];
            cameraP->pos = *pos;
            cameraP->up = *up;
            cameraP->target = *target;
        }
    }
}

/* Camera queries copy the first selected camera's position, up direction, and target to outputs. */
void Hu3DCameraPosGet(int cameraBit, Vec *pos, Vec *up, Vec *target)
{
    s16 i;
    s16 bit;
    for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
        if(cameraBit & bit) {
            HU3D_CAMERA *cameraP = &Hu3DCamera[i];
            *pos = cameraP->pos;
            *up = cameraP->up;
            *target = cameraP->target;
            break;
        }
    }
}

/* Camera queries copy perspective values from the first camera selected by cameraBit. */
void Hu3DCameraPerspectiveGet(int cameraBit, float *fov, float *near, float *far)
{
    s16 i;
    s16 bit;
    for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
        if(cameraBit & bit) {
            HU3D_CAMERA *cameraP = &Hu3DCamera[i];
            *fov = cameraP->fov;
            *near = cameraP->near;
            *far = cameraP->far;
            break;
        }
    }
}

/* Camera teardown marks every selected camera inactive by setting its field of view to -1. */
void Hu3DCameraKill(int cameraBit)
{
    s16 i;
    s16 bit;
    for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
        if(cameraBit & bit) {
            HU3D_CAMERA *cameraP = &Hu3DCamera[i];
            cameraP->fov = -1;
        }
    }
}

/* Global display teardown marks all active cameras inactive and clears the camera-exists mask. */
void Hu3DCameraAllKill(void)
{
    HU3D_CAMERA *cameraP = &Hu3DCamera[0];
    s16 i;
    s16 bit;
    for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1, cameraP++) {
        if(-1 != cameraP->fov) {
            Hu3DCameraKill(bit);
        }
    }
    Hu3DCameraExistF = 0;
}

/* The display path loads one camera's projection, viewport, scissor, and view matrix into GX. */
void Hu3DCameraSet(s32 cameraNo, Mtx modelView)
{
    Mtx44 proj;
    HU3D_CAMERA *cameraP = &Hu3DCamera[cameraNo];
    MTXPerspective(proj, cameraP->fov, cameraP->aspect, cameraP->near, cameraP->far);
    GXSetProjection(proj, GX_PERSPECTIVE);
    if(RenderMode->field_rendering) {
        GXSetViewportJitter(cameraP->viewportX, cameraP->viewportY, cameraP->viewportW,
                            cameraP->viewportH, cameraP->viewportNear, cameraP->viewportFar,
                            VIGetNextField());
    } else {
        GXSetViewport(cameraP->viewportX, cameraP->viewportY, cameraP->viewportW,
                      cameraP->viewportH, cameraP->viewportNear, cameraP->viewportFar);
    }
    GXSetScissor(cameraP->scissorX, cameraP->scissorY, cameraP->scissorW, cameraP->scissorH);
    MTXLookAt(modelView, &cameraP->pos, &cameraP->up, &cameraP->target);
}

/* Camera-motion setup reads the model's camera object, updates that camera, and enables model
 * camera motion. */
BOOL Hu3DModelCameraInfoSet(HU3D_MODELID modelId, u16 cameraBit)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HSF_DATA *hsf = modelP->hsf;
    HU3D_CAMERA *cameraP = &Hu3DCamera[cameraBit];
    HSF_OBJECT *objPtr;
    s16 i;
    for(objPtr = hsf->object, i=0; i<hsf->objectNum; i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        float upRot;
        HuVecF upOfs;
        HuVecF up;
        
        if(obj->type != HSF_OBJ_CAMERA) {
            continue;
        }
        upRot = obj->camera.upRot;
        cameraP->upRot = upRot;
        HuSubVecF(&up, &obj->camera.pos, &obj->camera.target);
        upOfs.x = ((up.x * up.y * (1-HuCos(upRot))) - (up.z * HuSin(upRot)));
        upOfs.y = ((up.y * up.y) + (1-HuSquare(up.y)) * HuCos(upRot));
        upOfs.z = (((up.y * up.z) * (1-HuCos(upRot))) + (up.x * HuSin(upRot)));
        HuNormVecF(&upOfs, &up);
        Hu3DCameraPosSet(cameraBit, obj->camera.pos.x, obj->camera.pos.y, obj->camera.pos.z,
            up.x, up.y, up.z,
            obj->camera.target.x, obj->camera.target.y, obj->camera.target.z);
        Hu3DCameraPerspectiveSet(cameraBit, obj->camera.fov, obj->camera.near, obj->camera.far,
                                 HU_DISP_ASPECT);
        modelP->camInfoBit = cameraBit;
        Hu3DModelAttrSet(modelId, HU3D_ATTR_CAMERA_MOTON);
        return TRUE;
    }
    return FALSE;
}

/* Camera-motion setup creates a motion-driven model record associated with the selected camera
 * bit. */
s16 Hu3DModelCameraCreate(HU3D_MOTIONID motId, u16 cameraBit)
{
    HU3D_MODELID modelId = Hu3DHookFuncCreate((HU3D_MODEL_HOOK)-1);
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->attr &= ~HU3D_ATTR_HOOKFUNC;
    modelP->attr |= HU3D_ATTR_CAMERA | HU3D_ATTR_CAMERA_MOTON;
    modelP->motId = motId;
    modelP->camInfoBit = cameraBit;
    
    return modelId;
}

/* Camera-motion control associates the model with a camera and enables its camera-motion flag. */
void Hu3DCameraMotionOn(HU3D_MODELID modelId, u16 cameraBit)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->camInfoBit = cameraBit;
    Hu3DModelAttrSet(modelId, HU3D_ATTR_CAMERA_MOTON);
}

/* Camera-motion control resumes the model motion, sets its full motion range, and starts at time
 * zero. */
void Hu3DCameraMotionStart(HU3D_MODELID modelId, u16 cameraBit)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    Hu3DCameraMotionOn(modelId, cameraBit);
    Hu3DModelAttrReset(modelId, HU3D_MOTATTR_PAUSE);
    Hu3DMotionStartEndSet(modelId, 0,  Hu3DMotionMotionMaxTimeGet(modelP->motId));
    Hu3DMotionTimeSet(modelId, 0);
}

/* Camera-motion control clears the flag that lets the model's motion drive camera state. */
void Hu3DCameraMotionOff(HU3D_MODELID modelId)
{
    Hu3DModelAttrReset(modelId, HU3D_ATTR_CAMERA_MOTON);
}

/* 3D system initialization marks all global and model-local light slots unused. */
void Hu3DLighInit(void)
{
    HU3D_LIGHT *lightP;
    s16 i;
    lightP = &Hu3DGlobalLight[0];
    for(i=0; i<HU3D_GLIGHT_MAX; i++, lightP++) {
        lightP->type = HU3D_LIGHT_TYPE_NONE;
    }
    lightP = &Hu3DLocalLight[0];
    for(i=0; i<HU3D_LLIGHT_MAX; i++, lightP++) {
        lightP->type = HU3D_LIGHT_TYPE_NONE;
    }
}

/* Called by light creation APIs to initialize a spotlight from scene position, direction, and
 * color. */
static void Hu3DLightCreate(HU3D_LIGHT *lightP, HuVecF *pos, HuVecF *dir, GXColor *color)
{
    lightP->type = HU3D_LIGHT_TYPE_SPOT;
    lightP->pos = *pos;
    /* Give a directionless light a usable forward axis before normalization. */
    if(dir->x == 0 && dir->y == 0 && dir->z == 0){ 
        dir->z = 1;
    }
    lightP->dir = *dir;
    lightP->offset.x = lightP->offset.y = lightP->offset.z = 0;
    lightP->cutoff = 30;
    lightP->func = GX_SP_COS;
    HuNormVecF(&lightP->dir, &lightP->dir);
    lightP->color = *color;
}

/* Scene setup packages scalar position, direction, and RGB values before allocating a global
 * light. */
HU3D_LIGHTID Hu3DGLightCreate(float posX, float posY, float posZ, float dirX, float dirY,
                              float dirZ, u8 colorR, u8 colorG, u8 colorB)
{
    Vec pos;
    Vec dir;
    GXColor color;
    pos.x = posX;
    pos.y = posY;
    pos.z = posZ;
    dir.x = dirX;
    dir.y = dirY;
    dir.z = dirZ;
    color.r = colorR;
    color.g = colorG;
    color.b = colorB;
    color.a = 128;
    return Hu3DGLightCreateV(&pos, &dir, &color);
}

/* Scene setup claims the first unused global light slot, initializes it, or returns NONE when
 * full. */
HU3D_LIGHTID Hu3DGLightCreateV(HuVecF *pos, HuVecF *dir, GXColor *color)
{
    HU3D_LIGHTID lightId;
    HU3D_LIGHT *lightP;
    for(lightP=&Hu3DGlobalLight[0], lightId=0; lightId<HU3D_GLIGHT_MAX; lightId++, lightP++) {
        if(lightP->type == HU3D_LIGHT_TYPE_NONE) {
            break;
        }
    }
    if(lightId == HU3D_GLIGHT_MAX) {
        return HU3D_LIGHTID_NONE;
    }
    
    Hu3DLightCreate(lightP, pos, dir, color);
    return lightId;
}

/* Model lighting setup packages scalar position, direction, and RGB values for a local light. */
HU3D_LLIGHTID Hu3DLLightCreate(HU3D_MODELID modelId, float posX, float posY, float posZ, float dirX,
                               float dirY, float dirZ, u8 colorR, u8 colorG, u8 colorB)
{
    Vec pos;
    Vec dir;
    GXColor color;
    pos.x = posX;
    pos.y = posY;
    pos.z = posZ;
    dir.x = dirX;
    dir.y = dirY;
    dir.z = dirZ;
    color.r = colorR;
    color.g = colorG;
    color.b = colorB;
    color.a = 255;
    return Hu3DLLightCreateV(modelId, &pos, &dir, &color);
}

/* Allocates a global light slot and assigns it to a model-local slot. Returns NONE if either
 * pool is full; if the model-local pool is full, the global slot remains claimed. */
HU3D_LLIGHTID Hu3DLLightCreateV(HU3D_MODELID modelId, HuVecF *pos, HuVecF *dir, GXColor *color)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HU3D_LIGHTID lightId;
    HU3D_LLIGHTID LLightId;
    HU3D_LIGHT *lightP;
    for(lightP=&Hu3DLocalLight[0], lightId=0; lightId<HU3D_LLIGHT_MAX; lightId++, lightP++) {
        if(lightP->type == HU3D_LIGHT_TYPE_NONE) {
            break;
        }
    }
    if(lightId == HU3D_LLIGHT_MAX) {
        return HU3D_LIGHTID_NONE;
    }
    Hu3DLightCreate(lightP, pos, dir, color);
    for(LLightId=0; LLightId<HU3D_MODEL_LLIGHT_MAX; LLightId++) {
        if(modelP->LLightId[LLightId] == HU3D_LIGHTID_NONE) {
            break;
        }
    }
    if(LLightId == HU3D_MODEL_LLIGHT_MAX) {
        return HU3D_LIGHTID_NONE;
    }
    modelP->LLightId[LLightId] = lightId;
    modelP->attr |= HU3D_ATTR_LLIGHT;  
    return LLightId;
}

/* Selects spot-light attenuation and its cone cutoff for subsequent GX light setup. */
static void Hu3DLightSpotSet(HU3D_LIGHT *lightP, GXSpotFn spotFunc, float cutoff)
{
    LIGHT_TYPE_SET(lightP, HU3D_LIGHT_TYPE_SPOT);
    lightP->cutoff = cutoff;
    lightP->func = spotFunc;
}

/* Configures a global light as a spotlight for later model draws. */
void Hu3DGLightSpotSet(HU3D_LIGHTID lightId, GXSpotFn spotFunc, float cutoff)
{
    HU3D_LIGHT *lightP = &Hu3DGlobalLight[lightId];
    Hu3DLightSpotSet(lightP, spotFunc, cutoff);
}

/* Configures one of a model's local lights as a spotlight for later model draws. */
void Hu3DLLightSpotSet(HU3D_MODELID modelId, HU3D_LLIGHTID lightId, float cutoff, GXSpotFn spotFunc)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HU3D_LIGHT *lightP = &Hu3DLocalLight[modelP->LLightId[lightId]];
    Hu3DLightSpotSet(lightP, spotFunc, cutoff);
}

/* Marks a light as directional, so its position is treated as an infinite source. */
static void Hu3DLightInfinitytSet(HU3D_LIGHT *lightP)
{
    LIGHT_TYPE_SET(lightP, HU3D_LIGHT_TYPE_INFINITYT);
}

/* Sets a global light to directional lighting. */
void Hu3DGLightInfinitytSet(HU3D_LIGHTID lightId)
{
    HU3D_LIGHT *lightP = &Hu3DGlobalLight[lightId];
    Hu3DLightInfinitytSet(lightP);
}

/* Sets one of a model's local lights to directional lighting. */
void Hu3DLLightInfinitytSet(HU3D_MODELID modelId, HU3D_LLIGHTID lightId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HU3D_LIGHT *lightP = &Hu3DLocalLight[modelP->LLightId[lightId]];
    Hu3DLightInfinitytSet(lightP);
}

/* Sets point-light attenuation using a reference distance and brightness. */
static void Hu3DLightPointSet(HU3D_LIGHT *lightP, float refDistance, float refBrightness,
                              GXDistAttnFn distFunc)
{
    LIGHT_TYPE_SET(lightP, HU3D_LIGHT_TYPE_POINT);
    lightP->cutoff = refDistance;
    lightP->brightness = refBrightness;
    lightP->func = distFunc;
}

/* Configures a global light as a point source for later model draws. */
void Hu3DGLightPointSet(HU3D_LIGHTID lightId, float refDistance, float refBrightness,
                        GXDistAttnFn distFunc)
{
    HU3D_LIGHT *lightP = &Hu3DGlobalLight[lightId];
    Hu3DLightPointSet(lightP, refDistance, refBrightness, distFunc);
}

/* Configures one of a model's local lights as a point source. */
void Hu3DLLightPointSet(HU3D_MODELID modelId, HU3D_LLIGHTID lightId, float refDistance,
                        float refBrightness, GXDistAttnFn distFunc)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HU3D_LIGHT *lightP = &Hu3DLocalLight[modelP->LLightId[lightId]];
    Hu3DLightPointSet(lightP, refDistance, refBrightness, distFunc);
}

/* Releases a global light slot so a later light creation can reuse it. */
void Hu3DGLightKill(HU3D_LIGHTID lightId)
{
    Hu3DGlobalLight[lightId].type = HU3D_LIGHT_TYPE_NONE;
}

/* Releases a model's local light slot and clears its local-light attribute when none remain. */
void Hu3DLLightKill(HU3D_MODELID modelId, HU3D_LLIGHTID lightId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HU3D_LIGHT *lightP = &Hu3DLocalLight[modelP->LLightId[lightId]];
    s16 i;
    lightP->type = HU3D_LIGHT_TYPE_NONE;
    modelP->LLightId[lightId] = HU3D_LIGHTID_NONE;
    for(i=0; i<HU3D_MODEL_LLIGHT_MAX; i++) {
        if(modelP->LLightId[i] == HU3D_LIGHTID_NONE) {
            break;
        }
    }
    if(i == HU3D_MODEL_LLIGHT_MAX) {
        modelP->attr &= ~HU3D_ATTR_LLIGHT;
    }
}

/* Clears every active global and local light during renderer or scene teardown. */
void Hu3DLightAllKill(void)
{
    HU3D_LIGHTID lightId;
    HU3D_LIGHT *lightP;
    for(lightP=&Hu3DGlobalLight[0], lightId=0; lightId<HU3D_GLIGHT_MAX; lightId++, lightP++) {
        if(lightP->type != HU3D_LIGHT_TYPE_NONE) {
            Hu3DGLightKill(lightId);
        }
    }
    for(lightP=&Hu3DLocalLight[0], lightId=0; lightId<HU3D_LLIGHT_MAX; lightId++, lightP++) {
        if(lightP->type != HU3D_LIGHT_TYPE_NONE) {
            lightP->type = HU3D_LIGHT_TYPE_NONE;
        }
    }
}

/* Stores the RGBA color used when this light is loaded for a model draw. */
static void Hu3DLightColSet(HU3D_LIGHT *lightP, u8 r, u8 g, u8 b, u8 a)
{
    lightP->color.r = r;
    lightP->color.g = g;
    lightP->color.b = b;
    lightP->color.a = a;
}

/* Changes a global light's RGBA color. */
void Hu3DGLightColorSet(HU3D_LIGHTID lightId, u8 r, u8 g, u8 b, u8 a)
{
    HU3D_LIGHT *lightP = &Hu3DGlobalLight[lightId];
    Hu3DLightColSet(lightP, r, g, b, a);
}

/* Changes the RGBA color of one of a model's local lights. */
void Hu3DLLightColorSet(HU3D_MODELID modelId, HU3D_LLIGHTID lightId, u8 r, u8 g, u8 b, u8 a)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HU3D_LIGHT *lightP = &Hu3DLocalLight[modelP->LLightId[lightId]];
    Hu3DLightColSet(lightP, r, g, b, a);
}

/* Stores a light position and normalizes the supplied direction for GX. */
static void Hu3DLightPSetV(HU3D_LIGHT *lightP, HuVecF *pos, HuVecF *dir)
{
    lightP->pos = *pos;
    HuNormVecF(dir, &lightP->dir);
}

/* Updates a global light from vector position and direction values. */
void Hu3DGLightPosSetV(HU3D_LIGHTID lightId, HuVecF *pos, HuVecF *dir)
{
    HU3D_LIGHT *lightP = &Hu3DGlobalLight[lightId];
    Hu3DLightPSetV(lightP, pos, dir);
}

/* Updates a model-local light from vector position and direction values. */
void Hu3DLLightPosSetV(HU3D_MODELID modelId, HU3D_LLIGHTID lightId, HuVecF *pos, HuVecF *dir)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HU3D_LIGHT *lightP = &Hu3DLocalLight[modelP->LLightId[lightId]];
    Hu3DLightPSetV(lightP, pos, dir);
}

static void Hu3DLightPSet(HU3D_LIGHT *lightP, float posX, float posY, float posZ, float dirX,
                          float dirY, float dirZ)
{
    lightP->pos.x = posX;
    lightP->pos.y = posY;
    lightP->pos.z = posZ;
    lightP->dir.x = dirX;
    lightP->dir.y = dirY;
    lightP->dir.z = dirZ;
    HuNormVecF(&lightP->dir, &lightP->dir);
}

/* Updates a global light from scalar position and direction components. */
void Hu3DGLightPosSet(HU3D_LIGHTID lightId, float posX, float posY, float posZ, float dirX,
                      float dirY, float dirZ)
{
    HU3D_LIGHT *lightP = &Hu3DGlobalLight[lightId];
    Hu3DLightPSet(lightP, posX, posY, posZ, dirX, dirY, dirZ);
}

/* Updates a model-local light from scalar position and direction components. */
void Hu3DLLightPosSet(HU3D_MODELID modelId, HU3D_LLIGHTID lightId, float posX, float posY,
                      float posZ, float dirX, float dirY, float dirZ)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HU3D_LIGHT *lightP = &Hu3DLocalLight[modelP->LLightId[lightId]];
    Hu3DLightPSet(lightP, posX, posY, posZ, dirX, dirY, dirZ);
}

/* Builds a normalized light direction from position and the supplied angles. */
static void Hu3DLightPASet(HU3D_LIGHT *lightP, float posX, float posY, float posZ, float angleX,
                           float angleY)
{
    lightP->pos.x = posX;
    lightP->pos.y = posY;
    lightP->pos.z = posZ;
    lightP->dir.x = HuSin((float)(angleY-180.0))*HuCos(angleX);
    lightP->dir.y = -HuSin(angleX);
    lightP->dir.z = HuCos((float)(angleY-180.0))*HuCos(angleX);
    HuNormVecF(&lightP->dir, &lightP->dir);
}

/* Positions a global light and aims it using horizontal and vertical angles. */
void Hu3DGLightPosAngleSet(HU3D_LIGHTID lightId, float posX, float posY, float posZ, float angleX,
                           float angleY)
{
    HU3D_LIGHT *lightP = &Hu3DGlobalLight[lightId];
    Hu3DLightPASet(lightP, posX, posY, posZ, angleX, angleY);
}

/* Positions a model-local light and aims it using horizontal and vertical angles. */
void Hu3DLLightPosAngleSet(HU3D_MODELID modelId, HU3D_LLIGHTID lightId, float posX, float posY,
                           float posZ, float angleX, float angleY)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HU3D_LIGHT *lightP = &Hu3DLocalLight[modelP->LLightId[lightId]];
    Hu3DLightPASet(lightP, posX, posY, posZ, angleX, angleY);
}

/* Stores the light position and points its normalized direction toward the aim point. */
static void Hu3DLightPAimSetV(HU3D_LIGHT *lightP, HuVecF *pos, HuVecF *aim)
{
    lightP->pos = *pos;
    HuSubVecF(&lightP->dir, aim, pos);
    HuNormVecF(&lightP->dir, &lightP->dir);
}

/* Points a global light from a vector position toward a vector aim point. */
void Hu3DGLightPosAimSetV(HU3D_LIGHTID lightId, HuVecF *pos, HuVecF *aim)
{
    HU3D_LIGHT *lightP = &Hu3DGlobalLight[lightId];
    Hu3DLightPAimSetV(lightP, pos, aim);
}

/* Points one of a model's local lights from a position toward a vector aim point. */
void Hu3DLLightPosAimSetV(HU3D_MODELID modelId, HU3D_LLIGHTID lightId, HuVecF *pos, HuVecF *aim)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HU3D_LIGHT *lightP = &Hu3DLocalLight[modelP->LLightId[lightId]];
    Hu3DLightPAimSetV(lightP, pos, aim);
}

/* Points a global light toward an aim point given as scalar coordinates. */
void Hu3DGLightPosAimSet(HU3D_LIGHTID lightId, float posX, float posY, float posZ, float aimX,
                         float aimY, float aimZ)
{
    Vec pos;
    Vec aim;
    pos.x = posX;
    pos.y = posY;
    pos.z = posZ;
    aim.x = aimX;
    aim.y = aimY;
    aim.z = aimZ;
    Hu3DGLightPosAimSetV(lightId, &pos, &aim);
}

/* Points a model-local light toward an aim point given as scalar coordinates. */
void Hu3DLLightPosAimSet(HU3D_MODELID modelId, HU3D_LLIGHTID lightId, float posX, float posY,
                         float posZ, float aimX, float aimY, float aimZ)
{
    Vec pos;
    Vec aim;
    pos.x = posX;
    pos.y = posY;
    pos.z = posZ;
    aim.x = aimX;
    aim.y = aimY;
    aim.z = aimZ;
    Hu3DLLightPosAimSetV(modelId, lightId,  &pos, &aim);
}

/* Sets whether camera transforms are applied to this light during GX setup. */
static void Hu3DLightStatSet(HU3D_LIGHT *lightP, BOOL staticF)
{
    if(staticF) {
        lightP->type |= HU3D_LIGHT_TYPE_STATIC;
    } else {
        lightP->type &= ~HU3D_LIGHT_TYPE_STATIC;
    }
}
/* Selects camera-relative or fixed placement for a global light. */
void Hu3DGLightStaticSet(HU3D_LIGHTID lightId, BOOL staticF)
{
    HU3D_LIGHT *lightP = &Hu3DGlobalLight[lightId];
    Hu3DLightStatSet(lightP, staticF);
}

/* Selects camera-relative or fixed placement for one of a model's local lights. */
void Hu3DLLightStaticSet(HU3D_MODELID modelId, HU3D_LLIGHTID lightId, BOOL staticF)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HU3D_LIGHT *lightP = &Hu3DLocalLight[modelP->LLightId[lightId]];
    Hu3DLightStatSet(lightP, staticF);
}

/* Returns a local-light slot's position, direction, color, and packed light type. */
s16 Hu3DLLightParamGet(HU3D_LIGHTID LLightId, Vec *pos, Vec *dir, GXColor *color)
{
    HU3D_LIGHT *lightP = &Hu3DLocalLight[LLightId];
    *pos = lightP->pos;
    *dir = lightP->dir;
    *color = lightP->color;
    return lightP->type;
}

/* Returns a global-light slot's position, direction, color, and packed light type. */
s16 Hu3DGLightParamGet(HU3D_LIGHTID lightId, Vec *pos, Vec *dir, GXColor *color)
{
    HU3D_LIGHT *lightP = &Hu3DGlobalLight[lightId];
    *pos = lightP->pos;
    *dir = lightP->dir;
    *color = lightP->color;
    return lightP->type;
}

/* Imports HSF light objects into the global light pool when a model is set up. */
s32 Hu3DModelLightInfoSet(HU3D_MODELID modelId, s16 staticF)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HSF_DATA *hsf = modelP->hsf;
    HSF_OBJECT *objPtr;
    s16 lightNum;
    s16 i;
    if(modelP->lightNum) {
        return modelP->lightNum;
    }
    for(objPtr = hsf->object, i=lightNum=0; i<hsf->objectNum; i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        HU3D_LIGHTID lightId;
        HU3D_LIGHT *lightP;
        HuVecF lightDir;
        if(obj->type != HSF_OBJ_LIGHT) {
            continue;
        }
        lightDir.x = obj->light.target.x - obj->light.pos.x;
        lightDir.y = obj->light.target.y - obj->light.pos.y;
        lightDir.z = obj->light.target.z - obj->light.pos.z;
        lightId =
            Hu3DGLightCreate(obj->light.pos.x, obj->light.pos.y, obj->light.pos.z, lightDir.x,
                             lightDir.y, lightDir.z, obj->light.r, obj->light.g, obj->light.b);
        modelP->lightId[lightNum] = lightId;
        lightP = &Hu3DGlobalLight[lightId];
        Hu3DGLightStaticSet(lightId, staticF);
        switch (obj->light.type) {
            case 0:
                Hu3DGLightSpotSet(lightId, GX_SP_COS, obj->light.cutoff);
                break;
            
            case 1:
                Hu3DGLightPointSet(lightId, obj->light.refBrightness - obj->light.refDistance, 1.0f,
                                   GX_DA_MEDIUM);
                Hu3DGLightPosSet(lightId, obj->light.pos.x, obj->light.pos.y, obj->light.pos.z, 0,
                                 1, 0);
                break;
            
            case 2:
                Hu3DGLightInfinitytSet(lightId);
                break;
        }
        lightNum++;
        if(lightNum >= HU3D_GLIGHT_MAX) {
            break;
        }
    }
    modelP->lightNum = lightNum;
    return lightNum;
}

/* Finds an imported HSF light by object name and reports NONE when it is absent. */
HU3D_LIGHTID Hu3DModelLightIdGet(HU3D_MODELID modelId, char *objName)
{
    char name[HSF_OBJNAME_MAX_LEN];
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HSF_DATA *hsf = modelP->hsf;
    HSF_OBJECT *objPtr;
    s16 i;
    s16 lightNo;
    strcpy(name, MakeObjectName((s8 *)objName));
    for(objPtr = hsf->object, i=lightNo=0; i<hsf->objectNum; i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->type != HSF_OBJ_LIGHT) {
            continue;
        }
        if(strcmp(name, obj->name) == 0) {
            return modelP->lightId[lightNo];
        }
        lightNo++;
    }
    OSReport("Error: Not Find Light Name.(%s)\n", objName);
    return HU3D_LIGHTID_NONE;
}

/* Enables a global light in this model's draw-time light mask. */
u8 Hu3DModelLightBitSet(HU3D_MODELID modelId, HU3D_LIGHTID lightId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->lightBit |= 1 << lightId;
    return modelP->lightBit;
}

/* Disables a global light in this model's draw-time light mask. */
u8 Hu3DModelLightBitReset(HU3D_MODELID modelId, HU3D_LIGHTID lightId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->lightBit &= (u8)~(1 << lightId);
    return modelP->lightBit;
}

static void lightSet(HU3D_LIGHT *lightP, s16 bit, Mtx cameraMtxXPose, Mtx cameraMtx,
                     float hilitePower);

/* Loads the enabled global and local lights for one model draw. */
s16 Hu3DLightSet(HU3D_MODEL *modelP, Mtx cameraMtx, Mtx cameraMtxXPose, float hilitePower)
{
    s16 bit;
    HU3D_LIGHT *lightP;
    s16 lightBit;
    s16 i;
    s16 flag;
    s16 mask;
    
    lightBit = 0;
    bit = (1 << 0);
    flag = 1;

    for (lightP = &Hu3DGlobalLight[0], mask = modelP->lightBit, i = 0; i < HU3D_GLIGHT_MAX;
         i++, lightP++, mask >>= 1) {
        if(lightP->type != HU3D_LIGHT_TYPE_NONE && (mask & 1)) {
            lightSet(lightP, bit, cameraMtxXPose, cameraMtx, hilitePower);
            lightBit |= bit;
            bit <<= 1;
        }
    }
    if(modelP->attr & HU3D_ATTR_LLIGHT) {
        for(i=0; i<HU3D_MODEL_LLIGHT_MAX; i++) {
            if(modelP->LLightId[i] != HU3D_LIGHTID_NONE) {
                lightP = &Hu3DLocalLight[modelP->LLightId[i]];
                lightSet(lightP, bit, cameraMtxXPose, cameraMtx, hilitePower);
                lightBit |= bit;
                bit <<= 1;
            }
        }
    }
    return lightBit;
}

/* Transforms one light and loads its attenuation, direction, color, and GX light slot. */
static void lightSet(HU3D_LIGHT *lightP, s16 lightBit, Mtx cameraMtx, Mtx cameraMtxXPose,
                     float hilitePower)
{
    GXLightObj lightObj;
    HuVecF dir;
    HuVecF pos;
    GXColor color;
    switch(LIGHT_TYPE_GET(lightP)) {
        case HU3D_LIGHT_TYPE_SPOT:
            GXInitLightAttn(&lightObj, 1, 0, 0, 1, 0, 0);
            GXInitLightSpot(&lightObj, lightP->cutoff, lightP->func);
            break;
            
        case HU3D_LIGHT_TYPE_INFINITYT:
            GXInitLightAttnA(&lightObj, 1, 0, 0);
            GXInitLightDistAttn(&lightObj, 0, 1, GX_DA_OFF);
            HuScaleVecF(&lightP->dir, &lightP->pos, -1000000.0f);
            break;
        
        case HU3D_LIGHT_TYPE_POINT:
            GXInitLightAttn(&lightObj, 1, 0, 0, 0, 0, 0);
            GXInitLightDistAttn(&lightObj, lightP->cutoff, lightP->brightness, lightP->func);
            break;
    }
    if(lightP->type & HU3D_LIGHT_TYPE_STATIC) {
        MTXMultVec(cameraMtx, &lightP->dir, &dir);
        MTXMultVec(cameraMtxXPose, &lightP->pos, &pos);
        GXInitLightPos(&lightObj, pos.x, pos.y, pos.z);
    } else {
        GXInitLightPos(&lightObj, lightP->pos.x, lightP->pos.y, lightP->pos.z);
        dir = lightP->dir;
    }
    if(0.0f == hilitePower) {
        GXInitLightDir(&lightObj, dir.x, dir.y, dir.z);
    } else {
        GXInitSpecularDir(&lightObj, dir.x, dir.y, dir.z);
        GXInitLightAttn(&lightObj, 0, 0, 1, hilitePower/2, 0, 1-(hilitePower/2));
    }
    color = lightP->color;
    GXInitLightColor(&lightObj, color);
    GXLoadLightObjImm(&lightObj, lightBit);
}

/* Replaces the reflection texture animation used by reflective model materials. */
void Hu3DReflectMapSet(ANIMDATA *anim)
{
    if (reflectAnim[0] != (void *)refMapData0) {
        HuMemDirectFree(reflectAnim[0]);
    }
    reflectAnim[0] = HuSprAnimRead(anim);
    reflectMapNo = 0;
}

void Hu3DReflectNoSet(s16 no)
{
    reflectMapNo = no;
}

/* Sets perspective exponential fog range and color for subsequent scene rendering. */
void Hu3DFogSet(float start, float end, u8 r, u8 g, u8 b)
{
    FogData.fogType = GX_FOG_PERSP_EXP;
    FogData.fogStart = start;
    FogData.fogEnd = end;
    FogData.fogColor.r = r;
    FogData.fogColor.g = g;
    FogData.fogColor.b = b;
    FogData.fogColor.a = 255;
}

/* Clears the stored fog mode and disables fog in GX. */
void Hu3DFogClear(void)
{
    FogData.fogType = GX_FOG_NONE;
    GXSetFog(GX_FOG_NONE, 0, 0, 0, 0, BGColor);
}

/* Sets the RGB ambient color used by the 3D renderer. */
void Hu3DAmbColorSet(float r, float g, float b)
{
    Hu3DAmbColR = r;
    Hu3DAmbColG = g;
    Hu3DAmbColB = b;
}

/* Enables or disables the renderer's shine effect. */
void Hu3DShineSet(BOOL shineF)
{
    if(shineF) {
        Hu3DShineF = TRUE;
    } else {
        Hu3DShineF = FALSE;
    }
}

#define SHADOW_DEFAULT_SIZE 192

/* Allocates and initializes shadow camera buffers for each selected camera. */
void Hu3DShadowMultiCreate(float fov, float near, float far, s16 cameraBit)
{
    s16 i;
    for(i=0; i<HU3D_CAM_MAX; i++) {
        if((1 << i) & cameraBit) {
            HU3D_SHADOW *shadowP = &Hu3DShadowBuf[i];
            shadowP->size = SHADOW_DEFAULT_SIZE;
            if(!shadowP->buf) {
                shadowP->buf =
                    HuMemDirectMalloc(HEAP_MODEL, SHADOW_DEFAULT_SIZE * SHADOW_DEFAULT_SIZE);
            }
            shadowP->fov = fov;
            shadowP->near = near;
            shadowP->far = far;
            shadowP->camPos.x = 300;
            shadowP->camPos.y = 300;
            shadowP->camPos.z = 0;
            shadowP->camTarget.x = shadowP->camTarget.y = shadowP->camTarget.z = 0;
            shadowP->camUp.x = -1;
            shadowP->camUp.y = 1;
            shadowP->camUp.z = 0;
            MTXLightPerspective(shadowP->projMtx, fov, HU_DISP_ASPECT, 0.5f, -0.5f, 0.5f, 0.5f);
            HuNormVecF(&shadowP->camUp, &shadowP->camUp);
            shadowP->color.r = shadowP->color.g = shadowP->color.b = 0;
            shadowP->color.a = 128;
        }
    }
    Hu3DShadowCamBit = cameraBit;
    Hu3DShadowF = TRUE;
}

void Hu3DShadowCreate(float fov, float near, float far)
{
    Hu3DShadowMultiCreate(fov, near, far, HU3D_CAM0);
}

/* Updates the shadow camera position, up vector, and target for selected cameras. */
void Hu3DShadowMultiPosSet(HuVecF *camPos, HuVecF *camUp, HuVecF *camTarget, s16 cameraBit)
{
    s16 i;
    for(i=0; i<HU3D_CAM_MAX; i++) {
        if((1 << i) & (cameraBit & Hu3DShadowCamBit)) {
            HU3D_SHADOW *shadowP = &Hu3DShadowBuf[i];
            shadowP->camPos = *camPos;
            shadowP->camTarget = *camTarget;
            shadowP->camUp = *camUp;
        }
    }
}

void Hu3DShadowPosSet(HuVecF *camPos, HuVecF *camUp, HuVecF *camTarget)
{
    Hu3DShadowMultiPosSet(camPos, camUp, camTarget, HU3D_CAM0);
}

/* Sets the shadow texture alpha for each selected shadow camera. */
void Hu3DShadowMultiTPLvlSet(float tpLvl, s16 cameraBit)
{
    s16 i;
    for(i=0; i<HU3D_CAM_MAX; i++) {
        if((1 << i) & (cameraBit & Hu3DShadowCamBit)) {
            Hu3DShadowBuf[i].color.a = tpLvl*255.0f;
        }
    }
}

void Hu3DShadowTPLvlSet(float tpLvl)
{
    Hu3DShadowMultiTPLvlSet(tpLvl, HU3D_CAM0);
}

/* Resizes shadow-map buffers for the selected shadow cameras. */
void Hu3DShadowMultiSizeSet(u16 size, s16 cameraBit)
{
    s16 i;
    for(i=0; i<HU3D_CAM_MAX; i++) {
        if((1 << i) & (cameraBit & Hu3DShadowCamBit)) {
            HU3D_SHADOW *shadowP = &Hu3DShadowBuf[i];
            shadowP->size = size;
            if(shadowP->buf) {
                HuMemDirectFree(shadowP->buf);
            }
            shadowP->buf = HuMemDirectMalloc(HEAP_MODEL, size*size);
        }
    }
}

void Hu3DShadowSizeSet(u16 size)
{
    Hu3DShadowMultiSizeSet(size, HU3D_CAM0);
}

/* Sets the RGB tint applied to shadows from the selected shadow cameras. */
void Hu3DShadowMultiColSet(u8 r, u8 g, u8 b, s16 cameraBit)
{
    s16 i;
    for(i=0; i<HU3D_CAM_MAX; i++) {
        if((1 << i) & (cameraBit & Hu3DShadowCamBit)) {
            HU3D_SHADOW *shadowP = &Hu3DShadowBuf[i];
            shadowP->color.r = r;
            shadowP->color.g = g;
            shadowP->color.b = b;
        }
    }
}

void Hu3DShadowColSet(u8 r, u8 g, u8 b)
{
    Hu3DShadowMultiColSet(r, g, b, HU3D_CAM0);
}

/* Renders shadow-casting models from the active shadow camera and copies its map to the texture. */
static void Hu3DShadowExec(BOOL bgColorF)
{
    Mtx temp;
    Mtx modelview;
    Mtx rot;
    Mtx44 proj;
    GXColor black = { 0, 0, 0, 255 };
    HU3D_MODEL *modelP;
    s16 i;
    s32 dataSize;
    Hu3DDrawPreInit();
    GXSetCopyClear(black, GX_MAX_Z24);
    MTXPerspective(proj, Hu3DShadow->fov, HU_DISP_ASPECT, Hu3DShadow->near, Hu3DShadow->far);
    GXSetProjection(proj, GX_PERSPECTIVE);
    if(Hu3DShadow->size <= 240) {
        GXSetScissor(2, 2, (Hu3DShadow->size*2)-4, (Hu3DShadow->size*2)-4);
        GXSetViewport(0, 0, (Hu3DShadow->size*2), (Hu3DShadow->size*2), 0, 1);
        dataSize = (Hu3DShadow->size/2)*(Hu3DShadow->size/2);
    } else {
        GXSetScissor(1, 1, Hu3DShadow->size-2, Hu3DShadow->size-2);
        GXSetViewport(0, 0, Hu3DShadow->size, Hu3DShadow->size, 0, 1);
        dataSize = Hu3DShadow->size*Hu3DShadow->size;
    }
    MTXLookAt(Hu3DCameraMtx, &Hu3DShadow->camPos, &Hu3DShadow->camUp, &Hu3DShadow->camTarget);
    MTXCopy(Hu3DCameraMtx, Hu3DShadow->lookAtMtx);
    modelP = &Hu3DData[0];
    shadowModelDrawF = TRUE;
    GXInvalidateTexAll();
    GXSetFog(GX_FOG_NONE, 0, 0, 0, 0, BGColor);
    for(i=0; i<HU3D_MODEL_MAX; i++, modelP++) {
        if(!modelP->hsf) {
            continue;
        }
        if ((modelP->attr & HU3D_ATTR_SHADOW) &&
            (modelP->attr & (HU3D_ATTR_DISPOFF | HU3D_ATTR_REFLECT_MODEL)) == 0 &&
            (modelP->attr & HU3D_ATTR_HOOK) == 0) {
            if(Hu3DShadowCamBit == HU3D_CAM0 || (Hu3DCameraBit & modelP->cameraBit)) {
                BOOL vtxInvalidateF;
                if(modelP->attr & HU3D_ATTR_MOTION_OFF) {
                    vtxInvalidateF = FALSE;
                    if(modelP->motId != HU3D_MOTIONID_NONE) {
                        Hu3DMotionExec(i, modelP->motId, modelP->motWork.time, FALSE);
                    }
                    if(modelP->motIdShift != HU3D_MOTIONID_NONE) {
                        Hu3DSubMotionExec(i);
                    }
                    if(modelP->motIdOvl != HU3D_MOTIONID_NONE) {
                        Hu3DMotionExec(i, modelP->motIdOvl, modelP->motOvlWork.time, TRUE);
                    }
                    if(modelP->attr & HU3D_ATTR_CLUSTER_ON) {
                        ClusterMotionExec(modelP);
                        vtxInvalidateF = TRUE;
                    }
                    if(modelP->motIdShape != HU3D_MOTIONID_NONE) {
                        if(modelP->motId == HU3D_MOTIONID_NONE) {
                            Hu3DMotionExec(i, modelP->motIdShape, modelP->motShapeWork.time, FALSE);
                        } else {
                            Hu3DMotionExec(i, modelP->motIdShape, modelP->motShapeWork.time, TRUE);
                        }
                    }
                    if ((modelP->attr & (HU3D_ATTR_ENVELOPE_OFF | HU3D_ATTR_HOOKFUNC)) == 0 ||
                        (modelP->attr & HU3D_ATTR_MOTION_MODEL)) {
                        vtxInvalidateF = TRUE;
                        InitVtxParm(modelP->hsf);
                        if(modelP->motIdShape != HU3D_MOTIONID_NONE) {
                            ShapeProc(modelP->hsf);
                        }
                        if(modelP->attr & HU3D_ATTR_CLUSTER_ON) {
                            ClusterProc(modelP);
                        }
                        if(modelP->hsf->cenvNum) {
                            EnvelopeProc(modelP->hsf);
                        }
                        PPCSync();
                    }
                    modelP->attr |= HU3D_ATTR_MOT_EXEC;
                }
                mtxRot(rot, modelP->rot.x, modelP->rot.y, modelP->rot.z);
                MTXScale(temp, modelP->scale.x, modelP->scale.y, modelP->scale.z);
                MTXConcat(rot, temp, temp);
                mtxTransCat(temp, modelP->pos.x, modelP->pos.y, modelP->pos.z);
                PSMTXConcat(Hu3DCameraMtx, temp, modelview);
                PSMTXConcat(modelview, modelP->mtx, modelview);
                Hu3DDraw(modelP, modelview, &modelP->scale);
            }
        }
    }
    Hu3DDrawPost();
    shadowModelDrawF = FALSE;
    if(Hu3DShadow->size <= 240) {
        GXSetTexCopySrc(0, 0, Hu3DShadow->size*2, Hu3DShadow->size*2);
        GXSetTexCopyDst(Hu3DShadow->size, Hu3DShadow->size, GX_CTF_R8, GX_TRUE);
    } else {
        GXSetTexCopySrc(0, 0, Hu3DShadow->size, Hu3DShadow->size);
        GXSetTexCopyDst(Hu3DShadow->size, Hu3DShadow->size, GX_CTF_R8, GX_FALSE);
    }
    GXCopyTex(Hu3DShadow->buf, GX_TRUE);
    GXSetViewport(0.0f, 0.0f, RenderMode->fbWidth, RenderMode->xfbHeight, 0.0f, 1.0f);
    GXSetScissor(0, 0, RenderMode->fbWidth, RenderMode->efbHeight);
    MTXOrtho(proj, 0, 1, 0, 1, 0, 1);
    GXSetProjection(proj, GX_ORTHOGRAPHIC);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_U8, 0);
    if(bgColorF) {
        GXSetTevColor(GX_TEVREG0, BGColor);
    } else {
        GXSetTevColor(GX_TEVREG0, black);
    }
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumChans(0);
    MTXIdentity(modelview);
    GXLoadPosMtxImm(modelview, GX_PNMTX0);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_TRUE);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_CLAMP, GX_AF_NONE);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3u8(0, 0, 0);
    GXPosition3u8(1, 0, 0);
    GXPosition3u8(1, 1, 0);
    GXPosition3u8(0, 1, 0);
    GXEnd();
}

/* Renders the reflection model into its texture before reflective models are drawn. */
static void Hu3DReflectModelExec(void)
{
    Mtx temp;
    Mtx modelview;
    Mtx44 proj;
    Vec pos;
    Vec target;
    HU3D_CAMERA *cameraP = &Hu3DCamera[0];
    HU3D_MODEL *modelP;
    GXColor clearColor = {};
    Hu3DDrawPreInit();
    GXSetCopyClear(clearColor, GX_MAX_Z24);
    MTXPerspective(proj, 50.0f, ((float)REFLECT_TEX_W/(float)REFLECT_TEX_H), 20.0f, 1000.0f);
    GXSetProjection(proj, GX_PERSPECTIVE);
    GXSetScissor(0, 0, REFLECT_TEX_W, REFLECT_TEX_H);
    GXSetViewport(0, 0, REFLECT_TEX_W, REFLECT_TEX_H, 0, 1);
    HuSubVecF(&pos, &cameraP->pos, &cameraP->target);
    HuNormVecF(&pos, &pos);
    HuScaleVecF(&pos, &pos, -100);
    target.x = target.y = target.z = 0;
    MTXLookAt(Hu3DCameraMtx, &pos, &cameraP->up, &target);
    modelP = &Hu3DData[Hu3DReflectModelId];
    GXInvalidateTexAll();
    GXSetFog(GX_FOG_NONE, 0, 0, 0, 0, BGColor);
    if(!modelP->hsf) {
        return;
    }
    if((modelP->attr & HU3D_ATTR_DISPOFF) || (modelP->attr & HU3D_ATTR_HOOK)) {
        return;
    }
    if(modelP->motId != HU3D_MOTIONID_NONE) {
        Hu3DMotionExec(Hu3DReflectModelId, modelP->motId, modelP->motWork.time, FALSE);
    }
    if ((modelP->attr & (HU3D_ATTR_ENVELOPE_OFF | HU3D_ATTR_HOOKFUNC)) == 0 ||
        (modelP->attr & HU3D_ATTR_MOTION_MODEL)) {
        InitVtxParm(modelP->hsf);
        if(modelP->motIdShape != HU3D_MOTIONID_NONE) {
            ShapeProc(modelP->hsf);
        }
        if(modelP->attr & HU3D_ATTR_CLUSTER_ON) {
            ClusterProc(modelP);
        }
        if(modelP->hsf->cenvNum) {
            EnvelopeProc(modelP->hsf);
        }
        PPCSync();
    }
    MTXConcat(Hu3DCameraMtx, temp, modelview); //BUG: Use of uninitialized temporary matrix
    MTXConcat(Hu3DCameraMtx, modelP->mtx, modelview);
    Hu3DDraw(modelP, modelview, &modelP->scale);
    Hu3DDrawPost();
    GXSetTexCopySrc(0, 0, REFLECT_TEX_W, REFLECT_TEX_H);
    GXSetTexCopyDst(REFLECT_TEX_W, REFLECT_TEX_H, GX_TF_RGB5A3, GX_FALSE);
    GXCopyTex(Hu3DReflectModelAnim->bmp->data, GX_TRUE);
    GXPixModeSync();
    GXSetViewport(0.0f, 0.0f, RenderMode->fbWidth, RenderMode->xfbHeight, 0.0f, 1.0f);
    GXSetScissor(0, 0, RenderMode->fbWidth, RenderMode->efbHeight);
    MTXOrtho(proj, 0, 1, 0, 1, 0, 1);
    GXSetProjection(proj, GX_ORTHOGRAPHIC);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetTevColor(GX_TEVREG0, BGColor);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumChans(0);
    MTXIdentity(modelview);
    GXLoadPosMtxImm(modelview, GX_PNMTX0);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_TRUE);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_CLAMP, GX_AF_NONE);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(0, 0, 0);
    GXPosition3f32(1, 0, 0);
    GXPosition3f32(1, 1, 0);
    GXPosition3f32(0, 1, 0);
    GXEnd();
}

/* Reserves a projection slot and initializes its camera and texture alpha settings. */
HU3D_PROJID Hu3DProjectionCreate(ANIMDATA *anim, float fov, float near, float far)
{
    HU3D_PROJECTION *projP;
    HU3D_PROJID projId;
    for(projP=&Hu3DProjection[0], projId=0; projId<HU3D_PROJ_MAX; projId++, projP++) {
        if(!projP->anim) {
            break;
        }
    }
    if(projId == HU3D_PROJ_MAX) {
        return HU3D_PROJID_NONE;
    }
    projP->anim = anim;
    projP->fov = fov;
    projP->near = near;
    projP->far = far;
    projP->camPos.x = 1000;
    projP->camPos.y = 1000;
    projP->camPos.z = 0;
    projP->camTarget.x = projP->camTarget.y = projP->camTarget.z = 0;
    projP->camUp.x = -1;
    projP->camUp.y = 1;
    projP->camUp.z = 0;
    MTXLightPerspective(projP->projMtx, fov, HU_DISP_ASPECT, 0.5f, -0.5f, 0.5f, 0.5f);
    HuNormVecF(&projP->camUp, &projP->camUp);
    projP->alpha = 128;
    Hu3DProjectionNum++;
    return projId;
}

/* Frees the projection animation and marks its slot unused. */
void Hu3DProjectionKill(HU3D_PROJID projId)
{
    HuSprAnimKill(Hu3DProjection[projId].anim);
    Hu3DProjection[projId].anim = NULL;
}

/* Updates the camera vectors used to draw a projection texture. */
void Hu3DProjectionPosSet(HU3D_PROJID projId, HuVecF *camPos, HuVecF *camUp, HuVecF *camTarget)
{
    Hu3DProjection[projId].camPos = *camPos;
    Hu3DProjection[projId].camTarget = *camTarget;
    Hu3DProjection[projId].camUp = *camUp;
}

void Hu3DProjectionTPLvlSet(HU3D_PROJID projId, float tpLvl)
{
    Hu3DProjection[projId].alpha = tpLvl*255;
}

/* Replaces a named model bitmap with the levels stored in an animation resource. */
void Hu3DMipMapSet(void *animData, HU3D_MODELID modelId, char *bmpName)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HSF_DATA *hsf = modelP->hsf;
    HSF_ATTRIBUTE *attrPtr;
    HSF_BITMAP *bmp;
    ANIMBMP *animBmp;
    s16 i;
    ANIMDATA *anim;
    s32 dataSize;
    char *data;
    char *dataPtr;
    
    for(attrPtr=hsf->attribute, i=0; i<hsf->attributeNum; i++, attrPtr++) {
        if(!strcmp(bmpName, attrPtr->bitmap->name)) {
            break;
        }
    }
    if(i == hsf->attributeNum) {
        OSReport("Error: Not Found %s for MipMapSet\n", bmpName);
        return;
    }
    bmp = attrPtr->bitmap;
    anim = HuSprAnimRead(animData);
    animBmp = anim->bmp;
    for(dataSize=i=0; i<anim->bmpNum; i++, animBmp++) {
        dataSize += animBmp->dataSize;
    }
    data = HuMemDirectMallocNum(HEAP_MODEL, dataSize, modelP->mallocNo);
    dataPtr = data;
    animBmp = anim->bmp;
    bmp->data = dataPtr;
    bmp->sizeX = animBmp->sizeX;
    bmp->sizeY = animBmp->sizeY;
    bmp->palSize = animBmp->palNum;
    bmp->palData = animBmp->palData;
    bmp->maxLod = anim->bmpNum;
    switch (animBmp->dataFmt) { 
        case ANIM_BMP_RGBA8:
            bmp->dataFmt = HSF_BMPFMT_RGBA8;
            break;
        
        case ANIM_BMP_RGB5A3:
            bmp->dataFmt = HSF_BMPFMT_RGB5A3;
            break;
        
        case ANIM_BMP_C8:
            bmp->dataFmt = HSF_BMPFMT_CI_RGB5A3;
            bmp->pixSize = 8;
            break;
        
        case ANIM_BMP_C4:
            bmp->dataFmt = HSF_BMPFMT_CI_RGB5A3;
            bmp->pixSize = 4;
            break;
    }
    for(i=0; i<anim->bmpNum; i++, animBmp++) {
        memcpy(data, animBmp->data, animBmp->dataSize);
        data += animBmp->dataSize;
    }
    DCFlushRange(dataPtr, dataSize);
}

/* Copies the requested framebuffer rectangle into a texture buffer. */
void Hu3DFbCopyExec(s16 x, s16 y, s16 w, s16 h, GXTexFmt texFmt, s16 mipmapF, void *buf)
{
    GXSetTexCopySrc(x, y, w, h);
    if(mipmapF) {
        GXSetTexCopyDst(w/2, h/2, texFmt, mipmapF);
    } else {
        GXSetTexCopyDst(w, h, texFmt, mipmapF);
    }
    GXCopyTex(buf, GX_FALSE);
}

static void FbCopyLayerHook(s16 layerNo);

/* Registers a layer callback that copies a framebuffer rectangle after the layer draws. */
void Hu3DFbCopyLayerSet(s16 layerNo, s16 x, s16 y, s16 w, s16 h, GXTexFmt texFmt, s16 mipmapF,
                        void *buf)
{
    FBCOPY_LAYER *copyLayerP = &FbCopyLayer[layerNo];
    copyLayerP->layerNo = layerNo;
    copyLayerP->sourceX = x;
    copyLayerP->sourceY = y;
    copyLayerP->width = w;
    copyLayerP->height = h;
    copyLayerP->texFmt = texFmt;
    copyLayerP->mipmapF = mipmapF;
    copyLayerP->textureBuffer = buf;
    Hu3DLayerHookSet(layerNo, FbCopyLayerHook);
}

/* Copies the framebuffer rectangle configured for this layer. */
static void FbCopyLayerHook(s16 layerNo)
{
    FBCOPY_LAYER *copyLayerP = &FbCopyLayer[layerNo];
    Hu3DFbCopyExec(copyLayerP->sourceX, copyLayerP->sourceY, copyLayerP->width, copyLayerP->height,
                   copyLayerP->texFmt, copyLayerP->mipmapF, copyLayerP->textureBuffer);
}

/* Draws a color-write-disabled quad one unit before the camera far plane to clear scene depth. */
void Hu3DZClear(void)
{
    HU3D_CAMERA *cameraP = &Hu3DCamera[Hu3DCameraNo];
    Mtx44 proj;
    Mtx modelview;
    Vec pos;
    Vec target;
    Vec up;
    float z;
    
    MTXPerspective(proj, cameraP->fov, cameraP->aspect, cameraP->near, cameraP->far);
    GXSetProjection(proj, GX_PERSPECTIVE);
    GXSetViewport(cameraP->viewportX, cameraP->viewportY, cameraP->viewportW, cameraP->viewportH,
                  cameraP->viewportNear, cameraP->viewportFar);
    GXSetScissor(cameraP->scissorX, cameraP->scissorY, cameraP->scissorW, cameraP->scissorH);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ONE);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetCullMode(GX_CULL_NONE);
    GXSetAlphaCompare(GX_GEQUAL, 0, GX_AOP_AND, GX_GEQUAL, 0);
    GXSetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);
    GXSetColorUpdate(GX_FALSE);
    pos.x = pos.y = pos.z = 0;
    target.x = target.y = 0;
    target.z = -100;
    up.x = up.z = 0;
    up.y = 1;
    MTXLookAt(modelview, &pos, &up, &target);
    GXLoadPosMtxImm(modelview, GX_PNMTX0);
    z = cameraP->far-1;
    pos.x = pos.y = z*HuTan(cameraP->fov/2);
    pos.x *= cameraP->aspect;
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-pos.x, -pos.y, -z);
    GXPosition3f32(pos.x, -pos.y, -z);
    GXPosition3f32(pos.x, pos.y, -z);
    GXPosition3f32(-pos.x, pos.y, -z);
    GXSetColorUpdate(GX_TRUE);
}

void Hu3DZClearLayerSet(s16 layerNo)
{
    Hu3DLayerHookSet(layerNo, (HU3D_LAYER_HOOK)Hu3DZClear);
}

/* Prints loaded model and motion IDs with their source file locations. */
void Hu3DModelDebug(void)
{
    HU3D_MODEL *modelP;
    s16 i;
    HU3D_MOTION *motionP;
    
    modelP = Hu3DData;
    OSReport("Model ******\n");
    OSReport("ID :Dir :File\n");
    for(i=0; i<HU3D_MODEL_MAX; i++, modelP++) {
        if(modelP->hsf && (modelP->attr & (HU3D_ATTR_HOOKFUNC|HU3D_ATTR_CAMERA)) == 0) {
            OSReport("%3d:%04x:%3d", i, HuMemMemoryFileGet(modelP->hsf) >> 16,
                     FILENUM(HuMemMemoryFileGet(modelP->hsf)));
            if(modelP->motId != HU3D_MOTIONID_NONE) {
                OSReport(" motionNo %d\n", modelP->motId);
            } else {
                OSReport("\n");
            }
        }
        
    }
    OSReport("Motion *****\n");
    OSReport("ID :Dir :File\n");
    for(motionP=Hu3DMotion, i=0; i<HU3D_MOTION_MAX; i++, motionP++) {
        if(motionP->hsf) {
            OSReport("%3d:%04x:%3d\n", i, HuMemMemoryFileGet(motionP->hsf) >> 16,
                     FILENUM(HuMemMemoryFileGet(motionP->hsf)));
        }
    }
}
