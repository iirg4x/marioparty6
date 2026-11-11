/* Initializes and renders the game's 3D models, cameras, lights, and effects. */
#include "dolphin/math.h"
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
#define MODEL_GLOBAL_LIGHTS_ALL ((1 << 8) - 1)
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
#include "humath.h"

typedef struct FbCopyLayer_s {
    s16 layerNo;       /* Hook slot for the copy; post-layer slots include HU3D_LAYER_HOOK_POST. */
    s16 sourceX;       /* Top-left source x coordinate in framebuffer pixels. */
    s16 sourceY;       /* Top-left source y coordinate in framebuffer pixels. */
    s16 width;         /* Source width in framebuffer pixels. */
    s16 height;        /* Source height in framebuffer pixels. */
    GXTexFmt texFmt;   /* Texture format used for the copied image. */
    BOOL mipmapF;      /* Whether the copy destination uses mipmap sizing. */
    void *textureBuffer; /* Destination texture buffer. */
} FBCOPY_LAYER;

static FBCOPY_LAYER FbCopyLayer[16];
HU3D_LIGHT lbl_80261674[48];
HU3D_LIGHT Hu3DGlobalLight[8];
Mtx lbl_80261424;
Mtx lbl_802613F4;
HSF_SCENE FogData;
HU3D_SHADOW Hu3DShadowBuf[16];
HU3D_PROJECTION Hu3DProjection[4];
ANIMDATA *lbl_802607B4[4];
ANIMDATA *reflectAnim[5];
static HU3D_LAYER_HOOK layerHook[16][16];
static s16 layerNum[8];
HU3D_CAMERA Hu3DCamera[16];
HU3D_MODEL *lbl_802C01C8;

GXColor lbl_802C01C2;
s16 lbl_802C01C0;
ANIMDATA *lbl_802C01BC;
s16 lbl_802C01BA;
s16 lbl_802C01B8;
BOOL lbl_802C01B4;
BOOL lbl_802C01B0;
HU3D_SHADOW *lbl_802C01AC;
s16 lbl_802C01A8;
s16 lbl_802C01A6;
s16 lbl_802C01A4;
u32 lbl_802C01A0;
s16 lbl_802C019E;
u16 lbl_802C019C;
static u16 NoSyncF;
HU3D_MODELID lbl_802C0198;
ANIMDATA *lbl_802C0194;
float lbl_802C0190;
float lbl_802C018C;
float lbl_802C0188;
BOOL lbl_802C0184;
static BOOL modelKillAllF;

/* Called once by main during startup to initialize the 3D renderer and its resources. */
void fn_800249F8(void)
{
    s16 index;
    s16 cameraIndex;
    HU3D_MODEL *modelP;
    HU3D_CAMERA *cameraP;
    fn_80017AF8();
    lbl_802C01C8 = modelP = HuMemDirectMalloc(HEAP_HEAP, sizeof(HU3D_MODEL)*512);

    for(modelP, index=0; index<512; index++, modelP++) {
        modelP->hsf = NULL;
    }
    for(cameraP = &Hu3DCamera[0], index=0; index<16; index++, cameraP++) {
        cameraP->fov = -1;
    }
    fn_8002C47C();
    fn_800285BC();
    lbl_802C01C2.r = lbl_802C01C2.g = lbl_802C01C2.b = lbl_802C01C2.a = 0;
    for(index=0; index<8; index++) {
        layerNum[index] = 0;
    }
    for(cameraIndex=0; cameraIndex<16; cameraIndex++) {
        for(index=0; index<16; index++) {
            layerHook[cameraIndex][index] = NULL;
        }
    }

    reflectAnim[0] = HuSprAnimRead(refMapData0);
    reflectAnim[1] = HuSprAnimRead(refMapData1);
    reflectAnim[2] = HuSprAnimRead(refMapData2);
    reflectAnim[3] = HuSprAnimRead(refMapData3);
    reflectAnim[4] = HuSprAnimRead(refMapData4);
    lbl_802C01C0 = 0;
    lbl_802C01BC = HuSprAnimRead(toonMapData);
    lbl_802607B4[0] = HuSprAnimRead(hiliteData);
    lbl_802607B4[1] = HuSprAnimRead(hiliteData2);
    lbl_802607B4[2] = HuSprAnimRead(hiliteData3);
    lbl_802607B4[3] = HuSprAnimRead(hiliteData4);
    Hu3DFogClear();
    Hu3DAnimInit();
    Hu3DParManInit();
    for(index=0; index<4; index++) {
        Hu3DProjection[index].anim = NULL;
    }
    lbl_802C01BA = 0;
    lbl_802C01B8 = 0;
    for(index=0; index<16; index++) {
        Hu3DShadowBuf[index].buf = NULL;
    }
    lbl_802C01B4 = FALSE;
    lbl_802C01A8 = 0;
    lbl_802C019C = 0;
    modelKillAllF = FALSE;
    lbl_802C019E = FALSE;
    lbl_802C0198 = -1;
    Hu3DAmbColorSet(1, 1, 1);
    lbl_802C0184 = FALSE;
    NoSyncF = FALSE;
}

/* Called once at the start of each main render frame before game logic and drawing. */
void fn_80024CAC(void)
{
    GXColor shadowClear = { 0, 0, 0, 255 };
    s16 modelIndex;
    HU3D_MODEL *modelP;
    if(lbl_802C01BA && lbl_802C01B4) {
        GXSetCopyClear(shadowClear, GX_MAX_Z24);
    } else {
        GXSetCopyClear(lbl_802C01C2, GX_MAX_Z24);
    }
    for(modelP = &lbl_802C01C8[0], modelIndex=0; modelIndex<512; modelIndex++, modelP++) {
        if(modelP->hsf) {
            modelP->attr &= ~((1 << 25)|(1 << 11));
        }
    }
    lbl_802C0124 = lbl_802C0128;
    lbl_802C011C = lbl_802C0120;
    lbl_802C0114 = lbl_802C0118;
    lbl_802C010C = lbl_802C0110;
    lbl_802C0128 = lbl_802C0120 = lbl_802C0118 = lbl_802C0110 = 0;
    GXSetAlphaUpdate(GX_TRUE);
}

static void Hu3DShadowExec(BOOL bgColorF);
static void Hu3DReflectModelExec(void);
void Hu3DReflectMapSet(ANIMDATA *anim);

#define HU3D_ATTR_CAMERA_UPDATE ((1 << 13)|(1 << 0))
#define HU3D_ATTR_NOUPDATE_ALL ((1 << 24)|(1 << 3)|(1 << 0))

/* Called by main after game logic when LoadProcMode is 0 to draw camera layers, models, and
 * sprites. */
void fn_80024E08(void)
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
    lbl_802C01B0 = FALSE;
    HuSprBegin();
    syncF = FALSE;
    if(lbl_802C0198 != -1) {
        Hu3DReflectModelExec();
    }
    if(lbl_802C01BA != 0 && lbl_802C01B4) {
        cameraP = &Hu3DCamera[0];
        GXInvalidateVtxCache();
        if(lbl_802C01B8 == (1 << 0)) {
            lbl_802C01AC = &Hu3DShadowBuf[0];
            Hu3DShadowExec(TRUE);
        } else {
            /* This scan reuses the existing Hu3DCameraBit; it does not compute a bit for each
             * camera. */
            for(cameraP = &Hu3DCamera[0], i=shadowCameraNo=0; i<16; i++, cameraP++) {
                if(cameraP->fov == -1) {
                    continue;
                }
                if(lbl_802C01B8 & lbl_802C01A4) {
                    shadowCameraNo = i;
                }
            }
            for (cameraP = &Hu3DCamera[0], lbl_802C01A6 = 0; lbl_802C01A6 < 16;
                 lbl_802C01A6++, cameraP++) {
                if(cameraP->fov == -1) {
                    continue;
                }
                cameraBit = (1 << lbl_802C01A6);
                lbl_802C01A4 = cameraBit;
                if(lbl_802C01B8 & lbl_802C01A4) {
                    lbl_802C01AC = &Hu3DShadowBuf[lbl_802C01A6];
                    if(shadowCameraNo == lbl_802C01A6) {
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
    for (cameraP = &Hu3DCamera[0], lbl_802C01A6 = 0; lbl_802C01A6 < 16;
         lbl_802C01A6++, cameraP++) {
        if(cameraP->fov == -1) {
            continue;
        }
        GXInvalidateVtxCache();
        lbl_802C01A4 = cameraBit = (1 << lbl_802C01A6);
        if(lbl_802C01B8 == (1 << 0)) {
            lbl_802C01AC = &Hu3DShadowBuf[0];
        } else if(lbl_802C01B8 & lbl_802C01A4) {
            lbl_802C01AC = &Hu3DShadowBuf[lbl_802C01A6];
        }
        for(projP=&Hu3DProjection[0], i=0; i<4; i++, projP++) {
            if(projP->anim) {
                MTXLookAt(projP->lookAtMtx, &projP->camPos, &projP->camUp, &projP->camTarget);
            }
        }
        if(lbl_802C01A6 == 0) {
            HuSprDispInit();
            HuSprExec(HUSPR_DRAWNO_BACK);
        }
        if(FogData.fogType != GX_FOG_NONE) {
            GXSetFog(FogData.fogType, FogData.fogStart, FogData.fogEnd, cameraP->near, cameraP->far,
                     FogData.fogColor);
        }
        for(layer=0; layer<8; layer++) {
            if(layerHook[lbl_802C01A6][layer]) {
                Hu3DCameraSet(lbl_802C01A6, lbl_802613F4);
                MTXInvXpose(lbl_802613F4, lbl_80261424);
                hookFunc = layerHook[lbl_802C01A6][layer];
                hookFunc(layer);
            }
            if(layerNum[layer]) {
                Hu3DDrawPreInit();
                Hu3DCameraSet(lbl_802C01A6, lbl_802613F4);
                MTXInvXpose(lbl_802613F4, lbl_80261424);
                for(modelP=&lbl_802C01C8[0], layerMdlNum=i=0; i<512; i++, modelP++) {
                    if(!modelP->hsf) {
                        continue;
                    }
                    if(modelP->attr & (1 << 16)) {
                        fn_8002EC20(i);
                        continue;
                    }
                    if ((modelP->attr & HU3D_ATTR_CAMERA_UPDATE) == HU3D_ATTR_CAMERA_UPDATE &&
                        modelP->motId != -1) {
                        fn_8002E4C4(i, modelP->motId, modelP->motWork.time, FALSE);
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
                    if (((modelP->attr & (1 << 11)) == 0 &&
                         (modelP->attr & (1 << 8)) == 0) ||
                        ((modelP->attr & (1 << 8)) != 0 && (modelP->tick & 1) != 0)) {
                        vtxInvalidateF = FALSE;
                        modelP->motAttr &= ~(1 << 30);
                        if(modelP->motId != -1) {
                            fn_8002E4C4(i, modelP->motId, modelP->motWork.time, FALSE);
                        }
                        if(modelP->motIdShift != -1) {
                            fn_8002ECDC(i);
                        }
                        if(modelP->motIdOvl != -1) {
                            fn_8002E4C4(i, modelP->motIdOvl, modelP->motOvlWork.time, TRUE);
                        }
                        if(modelP->attr & (1 << 10)) {
                            ClusterMotionExec(modelP);
                            vtxInvalidateF = TRUE;
                        }
                        if(modelP->motIdShape != -1) {
                            /* With no main motion, shape evaluation restores base transforms
                             * after the shift and overlay passes and can discard their transform
                             * results. */
                            if(modelP->motId == -1) {
                                fn_8002E4C4(i, modelP->motIdShape, modelP->motShapeWork.time,
                                               FALSE);
                            } else {
                                fn_8002E4C4(i, modelP->motIdShape, modelP->motShapeWork.time,
                                               TRUE);
                            }
                            vtxInvalidateF = TRUE;
                        }
                        if ((modelP->attr & ((1 << 7) | (1 << 4))) == 0 ||
                            (modelP->attr & (1 << 25))) {
                            vtxInvalidateF = TRUE;
                            InitVtxParm(modelP->hsf);
                            if(modelP->motIdShape != -1) {
                                ShapeProc(modelP->hsf);
                            }
                            if(modelP->attr & (1 << 10)) {
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
                        modelP->attr |= (1 << 11);
                    }
                    if(syncF && (modelP->attr & (1 << 4))) {
                        GXWaitDrawDone();
                        syncF = FALSE;
                    }
                    if ((modelP->attr & (1 << 15)) == 0 &&
                        (0.0f != modelP->scale.x || 0.0f != modelP->scale.y ||
                         0.0f != modelP->scale.z)) {
                        Mtx temp;
                        Mtx final;
                        fn_80022E38(temp, modelP->rot.x, modelP->rot.y, modelP->rot.z);
                        mtxScaleCat(temp, modelP->scale.x, modelP->scale.y, modelP->scale.z);
                        mtxTransCat(temp, modelP->pos.x, modelP->pos.y, modelP->pos.z);
                        PSMTXConcat(lbl_802613F4, temp, final);
                        PSMTXConcat(final, modelP->mtx, final);
                        fn_80017B34(modelP, final, &modelP->scale);
                    }
                    modelP->tick++;
                    layerMdlNum++;
                    if(layerMdlNum >= layerNum[layer]) {
                        break;
                    }
                }
                fn_80020024();
            }
            if(layerHook[lbl_802C01A6][layer+8]) {
                Hu3DCameraSet(lbl_802C01A6, lbl_802613F4);
                MTXInvXpose(lbl_802613F4, lbl_80261424);
                hookFunc = layerHook[lbl_802C01A6][layer+8];
                hookFunc(layer+8);
            }
        }
        if(!NoSyncF) {
            syncF = TRUE;
            GXSetDrawDone();
        }
    }
    HuSprDispInit();
    HuSprExec(HUSPR_DRAWNO_FRONT);
    for(modelP=&lbl_802C01C8[0], i=0; i<512; i++, modelP++) {
        if(!modelP->hsf) {
            continue;
        }
        if ((modelP->motId != -1 || (modelP->attr & (1 << 10)) != 0 ||
             modelP->motIdShape != -1) &&
            (lbl_802C019E == 0 || (modelP->attr & (1 << 21)) != 0)) {
            fn_8002DCE0(i);
        }
    }
    HuSprFinish();
    Hu3DAnimExec();
    HuPerfEnd(HUPERF_USR1);
}

/* Called when an overlay or save flow closes its scene to release scene resources and reset
* rendering defaults. */
void fn_80025630(void)
{
    s16 index;
    s16 cameraIndex;
    fn_80026648();
    fn_8002C7BC();
    Hu3DCameraAllKill();
    fn_80028D70();
    fn_80031260();
    if(reflectAnim[0] != (void *)refMapData0) {
        HuMemDirectFree(reflectAnim[0]);
    }
    reflectAnim[0] = HuSprAnimRead(refMapData0);
    for(index=0; index<16; index++) {
        if(Hu3DShadowBuf[index].buf) {
            HuMemDirectFree(Hu3DShadowBuf[index].buf);
            Hu3DShadowBuf[index].buf = NULL;
        }
    }
    lbl_802C01BA = 0;
    lbl_802C01B4 = 0;
    Hu3DFogClear();
    for(index=0; index<8; index++) {
        layerNum[index] = 0;
    }
    for(cameraIndex=0; cameraIndex<16; cameraIndex++) {
        for(index=0; index<16; index++) {
            layerHook[cameraIndex][index] = NULL;
        }
    }

    for(index=0; index<4; index++) {
        if(Hu3DProjection[index].anim) {
            Hu3DProjectionKill(index);
        }
        Hu3DProjection[index].anim = NULL;
    }
    Hu3DAmbColorSet(1, 1, 1);
    Hu3DBGColorSet(0, 0, 0);
    NoSyncF = FALSE;
    lbl_802C0184 = FALSE;
}

/* Sets the background clear color's RGB channels; the existing alpha is preserved. */
void Hu3DBGColorSet(u8 r, u8 g, u8 b)
{
    lbl_802C01C2.r = r;
    lbl_802C01C2.g = g;
    lbl_802C01C2.b = b;
}

/* Replaces the callback in the requested hook slot for each selected camera; slots with
* HU3D_LAYER_HOOK_POST run after the corresponding layer. */
void fn_8002583C(s16 cameraMask, s16 layerNo, HU3D_LAYER_HOOK hook)
{
    s16 cameraIndex;
    for(cameraIndex=0; cameraIndex<16; cameraIndex++) {
        if((1 << cameraIndex) & cameraMask) {
            layerHook[cameraIndex][layerNo] = hook;
        }
    }
}

/* Removes the callback in the requested hook slot for each selected camera; the slot may
* select a callback before or after its corresponding layer. */
void fn_800258A8(s16 cameraMask, s16 layerNo)
{
    s16 cameraIndex;
    for(cameraIndex=0; cameraIndex<16; cameraIndex++) {
        if((1 << cameraIndex) & cameraMask) {
            layerHook[cameraIndex][layerNo] = NULL;
        }
    }
}

/* Replaces the callback in this hook slot for every camera; HU3D_LAYER_HOOK_POST selects
* the slot after the corresponding layer. */
void fn_80025918(s16 layerNo, HU3D_LAYER_HOOK hook)
{
    fn_8002583C(((1 << 16) - 1), layerNo, hook);
}

/* Removes the callback in this hook slot from every camera, whether before or after a layer. */
void fn_80025980(s16 layerNo)
{
    fn_800258A8(((1 << 16) - 1), layerNo);
}

/* Pauses motion-time advancement in Hu3DExec except for HU3D_ATTR_NOPAUSE models; drawing
 * continues. */
void Hu3DPauseSet(BOOL paused)
{
    lbl_802C019E = paused;
}

/* Suppresses draw-completion synchronization in Hu3DExec while enabled. */
void Hu3DNoSyncSet(BOOL noSync)
{
    NoSyncF = noSync;
}

/* Creates a model from HSF data; embedded nonzero fog distances replace the scene fog.
* Callers then set its transform, camera, and layer as needed. */
HU3D_MODELID fn_80025A04(void *hsfData)
{
    HU3D_MODEL *modelP;
    s16 modelId;
    s16 index;
    for(modelP=&lbl_802C01C8[0], modelId=0; modelId<512; modelId++, modelP++) {
        if(!modelP->hsf) {
            break;
        }
    }
    if(modelId == 512) {
        OSReport("Error: Create Model Over!\n");
        return -1;
    }
    modelP->hsf = LoadHSF(hsfData);
    modelP->linkMdlId = -1;
    modelP->mallocNo = lbl_802C01A0 = (u32)modelP->hsf;
    modelP->attr = 0;
    modelP->motAttr = 0;
    modelP->projBit = 0;
    fn_80020DCC(modelId, modelP->mallocNo);
    modelP->motWork.speed = 1.0f;
    for(index=0; index<4; index++) {
        modelP->motIdCluster[index] = -1;
    }
    modelP->motIdOvl = -1;
    modelP->motIdShift = -1;
    modelP->motIdShape = -1;
    modelP->motWork.time = 0.0f;
    modelP->timingHook = NULL;
    modelP->matHook = NULL;
    if(modelP->hsf->motionNum) {
        modelP->attr |= (1 << 25);
        modelP->motId = modelP->motIdSrc = fn_8002C564(modelId);
        if(modelP->hsf->cenvNum) {
            fn_8002E4C4(modelId, modelP->motId, 0, FALSE);
            EnvelopeProc(modelP->hsf);
            PPCSync();
        }
        if(modelP->hsf->clusterNum) {
            fn_8002D050(modelId, modelP->motId);
        }
        if(modelP->hsf->shapeNum) {
            fn_8002CEB0(modelId, modelP->motId);
        }
        modelP->motWork.start = 0;
        modelP->motWork.end = fn_8002D530(modelId);
    } else {
        modelP->motIdSrc = modelP->motId = -1;
    }
    modelP->pos.x = modelP->pos.y = modelP->pos.z = 0;
    modelP->rot.x = modelP->rot.y = modelP->rot.z = 0;
    modelP->scale.x = modelP->scale.y = modelP->scale.z = 1;
    modelP->cameraBit = ((1 << 16) - 1);
    modelP->layerNo = 0;
    modelP->hookData = NULL;
    modelP->lightNum = 0;
    modelP->hiliteIdx = 0;
    modelP->ambR = modelP->ambG = modelP->ambB = 1;
    modelP->reflectType = -1;
    for(index=0; index<8; index++) {
        modelP->LLightId[index] = -1;
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

/* Duplicates model objects while sharing base resources and the main motion; transforms, camera
 * mask, layer, and lighting start with defaults. */
HU3D_MODELID fn_80025D4C(HU3D_MODELID linkMdlId)
{
    HU3D_MODEL *linkModelP = &lbl_802C01C8[linkMdlId];
    HU3D_MODEL *modelP;
    HSF_OBJECT *duplicatedObjects;
    s16 index;
    s16 modelId;
    u32 file;
    for(modelP=&lbl_802C01C8[0], modelId=0; modelId<512; modelId++, modelP++) {
        if(!modelP->hsf) {
            break;
        }
    }
    if(modelId == 512) {
        return -1;
    }
    modelP->hsfLink = linkModelP->hsf;
    modelP->hsf = HuMemDirectMalloc(2, sizeof(HSF_DATA));
    modelP->linkMdlId = linkMdlId;
    file = fn_8000C1B4(linkModelP->hsf)|MODEL_LINK_FILE_FLAG;
    fn_8000C164(modelP->hsf, file);
    modelP->mallocNoLink = (u32)modelP->hsf;
    *modelP->hsf = *linkModelP->hsf;
    duplicatedObjects = fn_800243B0(modelP->hsf, modelP->mallocNoLink);
    modelP->hsf->root = (HSF_OBJECT *) ((char *) duplicatedObjects +
                                        ((u32) modelP->hsf->root - (u32) modelP->hsf->object));
    modelP->hsf->object = duplicatedObjects;
    Hu3DAttrDuplicate(modelP->hsf, modelP->mallocNoLink);
    Hu3DMatDuplicate(modelP->hsf, modelP->mallocNoLink);
    modelP->mallocNo = linkModelP->mallocNo;
    modelP->attr = linkModelP->attr;
    if(linkModelP->attr & (1 << 2)) {
        lbl_802C01BA++;
    }
    linkModelP->attr |= (1 << 20);
    modelP->motAttr = linkModelP->motAttr;
    modelP->pos.x = modelP->pos.y = modelP->pos.z = 0;
    modelP->rot.x = modelP->rot.y = modelP->rot.z = 0;
    modelP->scale.x = modelP->scale.y = modelP->scale.z = 1;
    modelP->motId = linkModelP->motId;
    if(modelP->motId != -1) {
        modelP->motWork.start = 0;
        modelP->motWork.end = fn_8002D530(modelId);
    }
    modelP->motIdShift = modelP->motIdOvl = -1;
    modelP->motIdShape = linkModelP->motIdShape;
    modelP->motShapeWork.time = 0;
    modelP->motShapeWork.speed = linkModelP->motShapeWork.speed;
    modelP->motShapeWork.start = linkModelP->motShapeWork.start;
    modelP->motShapeWork.end = linkModelP->motShapeWork.end;
    for(index=0; index<4; index++) {
        modelP->motIdCluster[index] = linkModelP->motIdCluster[index];
        if(modelP->motIdCluster[index] != -1) {
            modelP->clusterTime[index] = 0;
            modelP->clusterSpeed[index] = linkModelP->clusterSpeed[index];
            modelP->clusterAttr[index] = linkModelP->clusterAttr[index];
            modelP->attr |= (1 << 10);
            ClusterAdjustObject(modelP->hsf, lbl_802624B8[modelP->motIdCluster[index]].hsf);
        }
    }
    modelP->motWork.time = linkModelP->motWork.time;
    modelP->motWork.speed = linkModelP->motWork.speed;
    modelP->motIdSrc = -1;
    modelP->cameraBit = ((1 << 16) - 1);
    modelP->layerNo = 0;
    modelP->hookData = NULL;
    modelP->lightNum = 0;
    modelP->hiliteIdx = 0;
    modelP->projBit = 0;
    modelP->ambR = modelP->ambG = modelP->ambB = 1;
    modelP->reflectType = -1;

    for(index=0; index<8; index++) {
        modelP->LLightId[index] = -1;
    }
    modelP->lightBit = MODEL_GLOBAL_LIGHTS_ALL;
    modelP->camInfoBit = 0;
    MTXIdentity(modelP->mtx);
    layerNum[0]++;
    return modelId;
}

/* Creates a model slot whose callback the HSF drawing pipeline invokes for draw objects. */
HU3D_MODELID fn_800260F4(HU3D_MODEL_HOOK hookFunc)
{
    HU3D_MODEL *modelP;
    s16 index;
    s16 modelId;
    for(modelP=&lbl_802C01C8[0], modelId=0; modelId<512; modelId++, modelP++) {
        if(!modelP->hsf) {
            break;
        }
    }
    if(modelId == 512) {
        return -1;
    }
    modelP->hookFunc = hookFunc;
    modelP->mallocNo = modelId+HOOKFUNC_MALLOCNO;
    modelP->attr = (1 << 4);
    modelP->motAttr = 0;
    modelP->pos.x = modelP->pos.y = modelP->pos.z = 0;
    modelP->rot.x = modelP->rot.y = modelP->rot.z = 0;
    modelP->scale.x = modelP->scale.y = modelP->scale.z = 1;
    modelP->motId = modelP->motIdShift = modelP->motIdOvl = modelP->motIdShape = -1;
    for(index=0; index<4; index++) {
        modelP->motIdCluster[index] = -1;
    }
    modelP->motWork.time = 0;
    modelP->motWork.speed = 1;
    modelP->motIdSrc = -1;
    modelP->timingHook = NULL;
    modelP->matHook = NULL;
    modelP->cameraBit = ((1 << 16) - 1);
    modelP->layerNo = 0;
    modelP->hookData = NULL;
    modelP->lightNum = 0;
    modelP->hiliteIdx = 0;
    modelP->linkMdlId = -1;
    modelP->projBit = 0;
    modelP->reflectType = -1;
    for(index=0; index<8; index++) {
        modelP->LLightId[index] = -1;
    }
    modelP->lightBit = MODEL_GLOBAL_LIGHTS_ALL;
    modelP->camInfoBit = 0;
    MTXIdentity(modelP->mtx);
    layerNum[0]++;
    return modelId;
}

/* Releases a model and its owned resources when an overlay or scene removes it. */
void fn_800262D0(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HSF_DATA *hsf = modelP->hsf;
    HU3D_MODEL *iterModelP;
    s16 index;
    s16 sharedModelCount;
    if(!hsf) {
        return;
    }
    layerNum[modelP->layerNo]--;
    if(modelP->attr & (1 << 4)) {
        HuMemDirectFreeNum(2, modelP->mallocNo);
        if(modelP->attr & (1 << 5)) {
            HU3D_PARTICLE *particleP = modelP->hookData;
            HuSprAnimKill(particleP->anim);
        }
        if(modelP->attr & (1 << 2)) {
            lbl_802C01BA--;
        }
        modelP->hookFunc = NULL;
        return;
    }
    if(modelP->attr & (1 << 16)) {
        if(modelP->motId != -1) {
            fn_8002C624(modelP->motId);
        }
        HuMemDirectFreeNum(2, modelP->mallocNo);
        if(modelP->attr & (1 << 2)) {
            lbl_802C01BA--;
        }
        modelP->hsf = NULL;
        return;
    }
    if(modelP->attr & (1 << 27)) {
        fn_80036F80(modelId);
    }
    fn_8003111C(modelId);
    if(modelP->linkMdlId != -1) {
        HuMemDirectFree(modelP->hsf);
        HuMemDirectFreeNum(2, modelP->mallocNoLink);
        hsf = modelP->hsfLink;
        modelP->hsf = hsf;
    }
    for (iterModelP = &lbl_802C01C8[0], sharedModelCount = index = 0; index < 512;
         index++, iterModelP++) {
        if(!iterModelP->hsf) {
            continue;
        }
        if (iterModelP->hsf == hsf ||
            (iterModelP->linkMdlId != -1 && iterModelP->hsfLink == hsf)) {
            sharedModelCount++;
        }
    }
    if(sharedModelCount > 1) {
        if(modelP->attr & (1 << 2)) {
            lbl_802C01BA--;
        }
        modelP->hsf = NULL;
        iterModelP=&lbl_802C01C8[0];
        if(modelP->motIdSrc != -1) {
            for(index=0; index<512; index++, iterModelP++) {
                if (iterModelP->hsf && iterModelP->linkMdlId != -1 &&
                    iterModelP->hsfLink == hsf) {
                    lbl_802624B8[modelP->motIdSrc].modelId = index;

                    break;
                }
            }
        }
        return;
    }
    if(modelP->motIdSrc != -1 && fn_8002C624(modelP->motIdSrc) == FALSE) {
        lbl_802624B8[modelP->motIdSrc].modelId = -1;
        HuMemDirectFreeNum(2, modelP->mallocNo);
        if(modelP->attr & (1 << 2)) {
            lbl_802C01BA--;
        }
        modelP->hsf = NULL;
        return;
    }
    HuMemDirectFree(modelP->hsf);
    HuMemDirectFreeNum(2, modelP->mallocNo);
    for(index=0; index<modelP->lightNum; index++) {
        Hu3DGLightKill(modelP->lightId[index]);
    }
    for(index=0; index<8; index++) {
        if(modelP->LLightId[index] != -1) {
            fn_80028CB8(modelId, index);
        }
    }
    if(modelP->attr & (1 << 2)) {
        lbl_802C01BA--;
    }
    modelP->hsf = NULL;
    return;
}

/* Called by Hu3DAllKill to remove every model and clear camera-layer hooks. */
void fn_80026648(void)
{
    s16 index;
    s16 cameraIndex;
    HU3D_MODEL *modelP;

    modelKillAllF = TRUE;
    for(modelP=&lbl_802C01C8[0], index=0; index<512; index++, modelP++) {
        if(modelP->hsf) {
            fn_800262D0(index);
        }
    }
    modelKillAllF = FALSE;
    for(index=0; index<8; index++) {
        layerNum[index] = 0;
    }
    for(cameraIndex=0; cameraIndex<16; cameraIndex++) {
        for(index=0; index<16; index++) {
            layerHook[cameraIndex][index] = NULL;
        }
    }

    fn_800339C8();
    HuMemDCFlush(2);
    lbl_802C0198 = -1;
}

/* Stores the model translation; hooked models apply it relative to their attachment object. */
void fn_80026760(HU3D_MODELID modelId, float posX, float posY, float posZ)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->pos.x = posX;
    modelP->pos.y = posY;
    modelP->pos.z = posZ;
}

/* Copies the model translation; hooked models apply it relative to their attachment object. */
void fn_80026790(HU3D_MODELID modelId, HuVecF *pos)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->pos = *pos;
}

/* Copies the stored model translation, which is relative to the attachment object when hooked. */
void fn_800267CC(HU3D_MODELID modelId, HuVecF *pos)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    *pos = modelP->pos;
}

/* Model setup and animation callers use this to set the model's three rotation components. */
void fn_80026808(HU3D_MODELID modelId, float rotX, float rotY, float rotZ)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->rot.x = rotX;
    modelP->rot.y = rotY;
    modelP->rot.z = rotZ;
}

/* Model setup copies all three rotation components into the selected model record. */
void fn_80026838(HU3D_MODELID modelId, HuVecF *rot)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->rot = *rot;
}

/* Model queries copy the selected model's stored rotation vector to the caller. */
void fn_80026874(HU3D_MODELID modelId, HuVecF *rot)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    *rot = modelP->rot;
}

/* Model setup callers use this to set independent scale factors on each model axis. */
void fn_800268B0(HU3D_MODELID modelId, float scaleX, float scaleY, float scaleZ)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->scale.x = scaleX;
    modelP->scale.y = scaleY;
    modelP->scale.z = scaleZ;
}

/* Model setup copies all three scale factors into the selected model record. */
void fn_800268E0(HU3D_MODELID modelId, HuVecF *scale)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->scale = *scale;
}

/* Model queries copy the selected model's stored scale vector to the caller. */
void fn_8002691C(HU3D_MODELID modelId, HuVecF *scale)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    *scale = modelP->scale;
}

/* Model setup callers can replace the model's stored transform matrix with this value. */
void fn_80026958(HU3D_MODELID modelId, Mtx *mtx)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    MTXCopy(*mtx, modelP->mtx);

}

/* Model queries copy the model's stored transform matrix into the caller's matrix. */
void fn_800269A0(HU3D_MODELID modelId, Mtx *mtx)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    MTXCopy(modelP->mtx, *mtx);
}

/* Model setup callers enable model or motion attributes; HU3D_MOTATTR routes bits to motAttr and
 * sets HU3D_ATTR_MOTION_MODEL. */
void fn_800269E8(HU3D_MODELID modelId, u32 attr)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    if(attr & (1 << 30)) {
        modelP->motAttr |= attr & ~(1 << 30);
        modelP->attr |= (1 << 25);
    } else {
        modelP->attr |= attr;
    }
}

/* Model setup callers clear model or motion attributes; HU3D_MOTATTR selects the motion mask. */
void fn_80026A44(HU3D_MODELID modelId, u32 attr)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    if(attr & (1 << 30)) {
        modelP->motAttr &= ~attr;
    } else {
        modelP->attr &= ~attr;
    }
}

/* Model queries return the model-level attribute mask. */
u32 fn_80026A90(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    return modelP->attr;
}

/* Motion queries return the motion mask with HU3D_MOTATTR set to identify its bit domain. */
u32 fn_80026AB8(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    return modelP->motAttr|(1 << 30);
}

/* Motion setup enables the requested attribute bits on one model cluster. */
void fn_80026AE4(HU3D_MODELID modelId, s16 clusterNo, s32 clusterAttr)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->clusterAttr[clusterNo] |= clusterAttr;
}

/* Motion setup clears the requested attribute bits on one model cluster. */
void fn_80026B24(HU3D_MODELID modelId, s16 clusterNo, s32 clusterAttr)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->clusterAttr[clusterNo] &= ~clusterAttr;
}

/* Render setup restricts the model to the supplied camera-selection bit mask. */
void fn_80026B64(HU3D_MODELID modelId, u16 cameraBit)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->cameraBit = cameraBit;
}

/* Render setup moves a model between layers while keeping each layer's model count current. */
void fn_80026B8C(HU3D_MODELID modelId, s16 layerNo)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    layerNum[modelP->layerNo]--;
    modelP->layerNo = layerNo;
    layerNum[layerNo]++;
}

/* Model and effect setup uses the normalized HSF object name to find its object record. */
HSF_OBJECT *fn_80026BF4(HU3D_MODELID modelId, char *objName)
{
    char name[256];
    HSF_DATA *hsf = lbl_802C01C8[modelId].hsf;
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
void fn_80026CBC(HU3D_MODELID modelId, float tpLvl)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HSF_DATA *hsf = modelP->hsf;
    HSF_MATERIAL *matPtr;
    HSF_OBJECT *objPtr;
    s16 i;
    for(matPtr = hsf->material, i=0; i<hsf->materialNum; i++, matPtr++) {
        matPtr->invAlpha = 1.0f-tpLvl;
        if(tpLvl != 1.0f) {
            matPtr->pass = (matPtr->pass & 0xF0) | 1;
        } else {
            matPtr->pass &= 0xF0;
        }
    }
    for(objPtr = hsf->object, i=0; i<hsf->objectNum; objPtr++, i++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->type == 2) {
            HSF_CONSTDATA *constData = obj->constData;
            constData->attr |= (1 << 0);
        }
    }
    modelP->attr |= (1 << 18);
}

/* Model setup assigns a highlight texture to every mesh object in the model. */
void fn_80026DB0(HU3D_MODELID modelId, ANIMDATA *hiliteMap)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HSF_DATA *hsf = modelP->hsf;
    s16 i;
    HSF_OBJECT *objPtr;

    for(objPtr = hsf->object, i=0; i<hsf->objectNum; objPtr++, i++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->type == 2) {
            HSF_CONSTDATA *constData;
            obj->flags |= (1 << 8);
            constData = obj->constData;
            constData->attr |= (1 << 15);
            constData->hiliteMap = hiliteMap;
        }
    }
}

/* Render setup enables shadow casting on the model and its objects with constant data. */
void fn_80026E44(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HSF_DATA *hsf = modelP->hsf;
    s16 i;
    HSF_OBJECT *objPtr;
    if(!(modelP->attr & (1 << 2))) {
        lbl_802C01BA++;
    }
    modelP->attr |= (1 << 2);
    for(objPtr = hsf->object, i=0; i<hsf->objectNum;  i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->constData) {
            HSF_CONSTDATA *constData;
            constData = obj->constData;
            constData->attr |= (1 << 10);
        }
    }
}

/* Render setup disables shadow casting and removes the shadow flag from the model's objects. */
void fn_80026EF0(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HSF_DATA *hsf = modelP->hsf;
    s16 i;
    HSF_OBJECT *objPtr;
    if(modelP->attr & (1 << 2)) {
        lbl_802C01BA--;
    }
    modelP->attr &= ~(1 << 2);
    for(objPtr = hsf->object, i=0; i<hsf->objectNum;  i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->constData) {
            HSF_CONSTDATA *constData;
            constData = obj->constData;
            constData->attr &= ~(1 << 10);
        }
    }
}

void fn_80026F9C(HU3D_MODELID modelId)
{
    fn_800269E8(modelId, (1 << 2));
}

void fn_80026FCC(HU3D_MODELID modelId)
{
    fn_80026A44(modelId, (1 << 2));
}

/* Render setup marks every object with constant data for shadow-map rendering. */
void fn_80026FFC(HU3D_MODELID modelId)
{
    HSF_DATA *hsf = lbl_802C01C8[modelId].hsf;
    s16 i;
    HSF_OBJECT *objPtr;

    for(objPtr = hsf->object, i=0; i<hsf->objectNum;  i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->constData) {
            HSF_CONSTDATA *constData;
            constData = obj->constData;
            constData->attr |= (1 << 3);
        }
    }
}

/* Render setup enables shadow-map rendering for the named object when it has constant data. */
void fn_80027080(HU3D_MODELID modelId, char *objName)
{
    char name[256];
    HSF_DATA *hsf = lbl_802C01C8[modelId].hsf;
    s16 i;
    HSF_OBJECT *objPtr = hsf->object;
    strcpy(name, MakeObjectName((s8 *)objName));
    for(i=0; i<hsf->objectNum; i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->constData && !strcmp(name, obj->name)) {
            HSF_CONSTDATA *constData;
            constData = obj->constData;
            constData->attr |= (1 << 3);
            break;
        }
    }
}

/* Render setup disables shadow-map rendering for the named object when it has constant data. */
void fn_80027138(HU3D_MODELID modelId, char *objName)
{
    char name[256];
    HSF_DATA *hsf = lbl_802C01C8[modelId].hsf;
    s16 i;
    HSF_OBJECT *objPtr = hsf->object;
    strcpy(name, MakeObjectName((s8 *)objName));
    for(i=0; i<hsf->objectNum; i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->constData && !strcmp(name, obj->name)) {
            HSF_CONSTDATA *constData;
            constData = obj->constData;
            constData->attr &= ~(1 << 3);
            break;
        }
    }
}

/* Render setup assigns a byte alpha derived from tpLvl to every object's shadow map. */
void fn_800271F0(HU3D_MODELID modelId, float tpLvl)
{
    HSF_DATA *hsf = lbl_802C01C8[modelId].hsf;
    s16 i;
    HSF_OBJECT *objPtr = hsf->object;
    for(i=0; i<hsf->objectNum; i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->constData) {
            HSF_CONSTDATA *constData;
            constData = obj->constData;
            constData->attr |= (1 << 3)|(1 << 19);
            constData->shadowAlpha = tpLvl*255;
        }
    }
}

/* Render setup assigns the requested shadow-map alpha to the named object only. */
void fn_80027290(HU3D_MODELID modelId, char *objName, float tpLvl)
{
    char name[256];
    HSF_DATA *hsf = lbl_802C01C8[modelId].hsf;
    s16 i;
    HSF_OBJECT *objPtr = hsf->object;
    strcpy(name, MakeObjectName((s8 *)objName));
    for(i=0; i<hsf->objectNum; i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->constData && !strcmp(name, obj->name)) {
            HSF_CONSTDATA *constData;
            constData = obj->constData;
            constData->attr |= (1 << 3)|(1 << 19);
            constData->shadowAlpha = tpLvl*255;
            break;
        }
    }
}

/* Render setup clears HU3D_CONST_SHADOW_MAP on objects with constant data but leaves
 * HU3D_CONST_SHADOW_MAP_TPLVL set. */
void fn_8002736C(HU3D_MODELID modelId)
{
    HSF_DATA *hsf = lbl_802C01C8[modelId].hsf;
    s32 i;
    HSF_OBJECT *objPtr = hsf->object;
    for(i=0; i<hsf->objectNum; i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->constData) {
            HSF_CONSTDATA *constData;
            constData = obj->constData;
            constData->attr &= ~(1 << 3);
        }
    }
}

/* Lighting setup stores the model's ambient red, green, and blue components. */
void fn_800273EC(HU3D_MODELID modelId, float ambR, float ambG, float ambB)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->ambR = ambR;
    modelP->ambG = ambG;
    modelP->ambB = ambB;
}

/* Attachment setup hooks hookMdlId to a named object after calculating the source model motion. */
void fn_8002741C(HU3D_MODELID modelId, char *objName, HU3D_MODELID hookMdlId)
{
    char name[256];
    HSF_DATA *hsf = lbl_802C01C8[modelId].hsf;
    s16 i;
    HSF_OBJECT *objPtr;

    fn_80030644(modelId);
    objPtr = hsf->object;
    strcpy(name, MakeObjectName((s8 *)objName));
    for(i=0; i<hsf->objectNum; i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->constData && !strcmp(name, obj->name)) {
            HSF_CONSTDATA *constData = obj->constData;
            /* Replacing an attachment leaves the previous attached model's hook attribute set. */
            constData->hookMdlId = hookMdlId;
            fn_800269E8(hookMdlId, (1 << 15));
            return;
        }
    }
    OSReport( "Error: Not Found %s for HookSet\n", objName);
}

/* Attachment cleanup clears each attached model hook attribute and resets its object hook ID. */
void fn_80027508(HU3D_MODELID modelId)
{
    HSF_DATA *hsf = lbl_802C01C8[modelId].hsf;
    s16 i;
    HSF_OBJECT *objPtr = hsf->object;
    for(i=0; i<hsf->objectNum; i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->constData) {
            HSF_CONSTDATA *constData = obj->constData;
            if(constData->hookMdlId != -1) {
                fn_80026A44(constData->hookMdlId, (1 << 15));
                constData->hookMdlId = -1;
            }
        }
    }
}

/* Clears a named object's attachment; its hook model ID must be valid because NONE is not
 * checked. */
void fn_800275B4(HU3D_MODELID modelId, char *objName)
{
    char name[256];
    HSF_DATA *hsf = lbl_802C01C8[modelId].hsf;
    s16 i;
    HSF_OBJECT *objPtr = hsf->object;
    strcpy(name, MakeObjectName((s8 *)objName));
    for(i=0; i<hsf->objectNum; i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->constData && !strcmp(name, obj->name)) {
            HSF_CONSTDATA *constData = obj->constData;
            fn_80026A44(constData->hookMdlId, (1 << 15));
            constData->hookMdlId = -1;
            return;
        }
    }
    OSReport("Error: Not Found %s for HookReset\n", objName);
}

/* Sets the selected projection bit on the model. */
void fn_8002769C(HU3D_MODELID modelId, HU3D_PROJID projId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->projBit |= (1 << projId);
}

/* Clears the selected projection bit on the model. */
void fn_800276DC(HU3D_MODELID modelId, HU3D_PROJID projId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->projBit &= ~(1 << projId);
}

/* Render setup writes the highlight type into each material and enables model highlights. */
void fn_8002771C(HU3D_MODELID modelId, s16 hiliteType)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HSF_DATA *hsf = modelP->hsf;
    HSF_MATERIAL *matPtr = hsf->material;
    s16 i;
    hiliteType = hiliteType << 4;
    hiliteType &= MATERIAL_HILITE_TYPE_MASK;
    for(i=0; i<hsf->materialNum; i++, matPtr++) {
        matPtr->pass = (matPtr->pass & 0xF) | hiliteType;
        matPtr->flags |= (1 << 8);
    }
    fn_800269E8(modelId, (1 << 17));
}

/* Stores the model reflection type. */
void fn_800277D0(HU3D_MODELID modelId, s16 reflectType)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->reflectType = reflectType;
}

/* Reflection setup selects the model, allocates its 128-by-128 texture, and initializes the map. */
void fn_800277FC(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP;
    lbl_802C0198 = modelId;
    modelP = &lbl_802C01C8[modelId];
    modelP->attr |= (1 << 24);
    lbl_802C0194 = HuSprAnimMake(REFLECT_TEX_W, REFLECT_TEX_H, 2);
    lbl_802C0194->bmp->data = HuMemDirectMalloc(2, REFLECT_TEX_W*REFLECT_TEX_H*2);
    Hu3DReflectMapSet(lbl_802C0194);
}

void fn_80027884(HU3D_MODELID modelId, HU3D_MAT_HOOK matHook)
{
    lbl_802C01C8[modelId].matHook = matHook;
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
    lbl_802C019C |= cameraBit;
    for(i=0, bit=1; i<16; i++, bit <<= 1) {
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
    for(i=0, bit=1; i<16; i++, bit <<= 1) {
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
    for(i=0, bit=1; i<16; i++, bit <<= 1) {
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
    for(i=0, bit=1; i<16; i++, bit <<= 1) {
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
    for(i=0, bit=1; i<16; i++, bit <<= 1) {
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
    for(i=0, bit=1; i<16; i++, bit <<= 1) {
        if(cameraBit & bit) {
            HU3D_CAMERA *cameraP = &Hu3DCamera[i];
            cameraP->pos = *pos;
            cameraP->up = *up;
            cameraP->target = *target;
        }
    }
}

/* Camera queries copy the first selected camera's position, up direction, and target to outputs. */
void fn_80027CD0(int cameraBit, Vec *pos, Vec *up, Vec *target)
{
    s16 i;
    s16 bit;
    for(i=0, bit=1; i<16; i++, bit <<= 1) {
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
void fn_80027D8C(int cameraBit, float *fov, float *near, float *far)
{
    s16 i;
    s16 bit;
    for(i=0, bit=1; i<16; i++, bit <<= 1) {
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
    for(i=0, bit=1; i<16; i++, bit <<= 1) {
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
    for(i=0, bit=1; i<16; i++, bit <<= 1, cameraP++) {
        if(-1 != cameraP->fov) {
            Hu3DCameraKill(bit);
        }
    }
    lbl_802C019C = 0;
}

/* Loads one camera's projection, viewport, and scissor into GX and writes its view matrix to
 * modelView. */
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

/* Applies the first HSF camera object's position and perspective using cameraBit as a mask and
 * enables camera motion with its low eight bits. The upRot write instead uses cameraBit as an
 * index. */
BOOL fn_8002803C(HU3D_MODELID modelId, u16 cameraBit)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HSF_DATA *hsf = modelP->hsf;
    HU3D_CAMERA *cameraP = &Hu3DCamera[cameraBit];
    HSF_OBJECT *objPtr;
    s16 i;
    for(objPtr = hsf->object, i=0; i<hsf->objectNum; i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        float upRot;
        HuVecF upOfs;
        HuVecF up;

        if(obj->type != 7) {
            continue;
        }
        upRot = obj->camera.upRot;
        cameraP->upRot = upRot;
        HuSubVecF(&up, &obj->camera.pos, &obj->camera.target);
        upOfs.x = ((up.x * up.y * (1-cos(M_PI*upRot/180.0)))
            - (up.z * sin(M_PI*upRot/180.0)));
        upOfs.y = ((up.y * up.y) + (1-(up.y*up.y)) * cos(M_PI*upRot/180.0));
        upOfs.z = (((up.y * up.z) * (1-cos(M_PI*upRot/180.0)))
            + (up.x * sin(M_PI*upRot/180.0)));
        VECNormalize(&upOfs, &up);
        Hu3DCameraPosSet(cameraBit, obj->camera.pos.x, obj->camera.pos.y, obj->camera.pos.z,
            up.x, up.y, up.z,
            obj->camera.target.x, obj->camera.target.y, obj->camera.target.z);
        Hu3DCameraPerspectiveSet(cameraBit, obj->camera.fov, obj->camera.near, obj->camera.far,
                                 HU_DISP_ASPECT);
        modelP->camInfoBit = cameraBit;
        fn_800269E8(modelId, (1 << 13));
        return TRUE;
    }
    return FALSE;
}

/* Camera-animation setup creates a model record with the low eight bits of the camera mask.
 * The caller must keep a model slot free; this function does not handle a full model table. */
s16 fn_800283F4(HU3D_MOTIONID motId, u16 cameraBit)
{
    HU3D_MODELID modelId = fn_800260F4((HU3D_MODEL_HOOK)-1);
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->attr &= ~(1 << 4);
    modelP->attr |= (1 << 16) | (1 << 13);
    modelP->motId = motId;
    modelP->camInfoBit = cameraBit;

    return modelId;
}

/* Stores the low eight bits of the camera-motion mask and enables the model's camera-motion
 * flag. */
void fn_80028478(HU3D_MODELID modelId, u16 cameraBit)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->camInfoBit = cameraBit;
    fn_800269E8(modelId, (1 << 13));
}

/* Camera-motion control resumes the model motion, sets its full motion range, and starts at time
 * zero. */
void fn_800284C8(HU3D_MODELID modelId, u16 cameraBit)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    fn_80028478(modelId, cameraBit);
    fn_80026A44(modelId, ((1 << 30) | (1 << 1)));
    fn_8002D730(modelId, 0,  Hu3DMotionMotionMaxTimeGet(modelP->motId));
    fn_8002D414(modelId, 0);
}

/* Camera-motion control clears the flag that lets the model's motion drive camera state. */
void fn_8002858C(HU3D_MODELID modelId)
{
    fn_80026A44(modelId, (1 << 13));
}

/* 3D system initialization marks all global and model-local light slots unused. */
void fn_800285BC(void)
{
    HU3D_LIGHT *lightP;
    s16 i;
    lightP = &Hu3DGlobalLight[0];
    for(i=0; i<8; i++, lightP++) {
        lightP->type = -1;
    }
    lightP = &lbl_80261674[0];
    for(i=0; i<48; i++, lightP++) {
        lightP->type = -1;
    }
}

/* Called by light creation APIs to initialize a spotlight from scene position, direction, and
 * color. */
static void Hu3DLightCreate(HU3D_LIGHT *lightP, HuVecF *pos, HuVecF *dir, GXColor *color)
{
    lightP->type = 0;
    lightP->pos = *pos;
    /* If the caller's direction is zero, change its z component to 1 before copying and normalizing
     * it. */
    if(dir->x == 0 && dir->y == 0 && dir->z == 0){
        dir->z = 1;
    }
    lightP->dir = *dir;
    lightP->offset.x = lightP->offset.y = lightP->offset.z = 0;
    lightP->cutoff = 30;
    lightP->func = GX_SP_COS;
    VECNormalize(&lightP->dir, &lightP->dir);
    lightP->color = *color;
}

/* Packages scalar position, direction, and RGB values for a global light, forcing alpha to 128. */
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
    return fn_800286D0(&pos, &dir, &color);
}

/* Claims the first unused global light slot or returns NONE when full. Initialization changes a
* zero input direction to +Z in the caller's vector, then normalizes the stored direction. */
HU3D_LIGHTID fn_800286D0(HuVecF *pos, HuVecF *dir, GXColor *color)
{
    HU3D_LIGHTID lightId;
    HU3D_LIGHT *lightP;
    for(lightP=&Hu3DGlobalLight[0], lightId=0; lightId<8; lightId++, lightP++) {
        if(lightP->type == -1) {
            break;
        }
    }
    if(lightId == 8) {
        return -1;
    }

    Hu3DLightCreate(lightP, pos, dir, color);
    return lightId;
}

/* Packages scalar position, direction, and RGB values for a model-local light, forcing alpha to
 * 255. */
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
    return fn_800288B4(modelId, &pos, &dir, &color);
}

/* Allocates from the shared local-light pool and assigns the slot to the model. Initialization
* changes a zero caller-owned direction to +Z before checking the model list. Returns NONE if
* either pool is full; a full model list leaves the shared slot claimed. */
HU3D_LLIGHTID fn_800288B4(HU3D_MODELID modelId, HuVecF *pos, HuVecF *dir, GXColor *color)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HU3D_LIGHTID lightId;
    HU3D_LLIGHTID LLightId;
    HU3D_LIGHT *lightP;
    for(lightP=&lbl_80261674[0], lightId=0; lightId<48; lightId++, lightP++) {
        if(lightP->type == -1) {
            break;
        }
    }
    if(lightId == 48) {
        return -1;
    }
    Hu3DLightCreate(lightP, pos, dir, color);
    for(LLightId=0; LLightId<8; LLightId++) {
        if(modelP->LLightId[LLightId] == -1) {
            break;
        }
    }
    if(LLightId == 8) {
        return -1;
    }
    modelP->LLightId[LLightId] = lightId;
    modelP->attr |= (1 << 12);
    return LLightId;
}

/* Selects spot-light attenuation and its cone cutoff for subsequent GX light setup. */
static void Hu3DLightSpotSet(HU3D_LIGHT *lightP, GXSpotFn spotFunc, float cutoff)
{
    LIGHT_TYPE_SET(lightP, 0);
    lightP->cutoff = cutoff;
    lightP->func = spotFunc;
}

/* Configures a global light as a spotlight for later model draws. */
void Hu3DGLightSpotSet(HU3D_LIGHTID lightId, float cutoff, GXSpotFn spotFunc)
{
    HU3D_LIGHT *lightP = &Hu3DGlobalLight[lightId];
    Hu3DLightSpotSet(lightP, spotFunc, cutoff);
}

/* Configures one of a model's local lights as a spotlight for later model draws. */
void fn_80028AA8(HU3D_MODELID modelId, HU3D_LLIGHTID lightId, float cutoff, GXSpotFn spotFunc)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HU3D_LIGHT *lightP = &lbl_80261674[modelP->LLightId[lightId]];
    Hu3DLightSpotSet(lightP, spotFunc, cutoff);
}

/* Marks a light directional; drawing replaces its position with a distant point derived from
 * direction. */
static void Hu3DLightInfinitytSet(HU3D_LIGHT *lightP)
{
    LIGHT_TYPE_SET(lightP, 1);
}

/* Sets a global light to directional lighting. */
void Hu3DGLightInfinitytSet(HU3D_LIGHTID lightId)
{
    HU3D_LIGHT *lightP = &Hu3DGlobalLight[lightId];
    Hu3DLightInfinitytSet(lightP);
}

/* Sets one of a model's local lights to directional lighting. */
void fn_80028B58(HU3D_MODELID modelId, HU3D_LLIGHTID lightId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HU3D_LIGHT *lightP = &lbl_80261674[modelP->LLightId[lightId]];
    Hu3DLightInfinitytSet(lightP);
}

/* Sets point-light attenuation using a reference distance and brightness. */
static void Hu3DLightPointSet(HU3D_LIGHT *lightP, float refDistance, float refBrightness,
                              GXDistAttnFn distFunc)
{
    LIGHT_TYPE_SET(lightP, 2);
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
void fn_80028C1C(HU3D_MODELID modelId, HU3D_LLIGHTID lightId, float refDistance,
                        float refBrightness, GXDistAttnFn distFunc)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HU3D_LIGHT *lightP = &lbl_80261674[modelP->LLightId[lightId]];
    Hu3DLightPointSet(lightP, refDistance, refBrightness, distFunc);
}

/* Releases a global light slot so a later light creation can reuse it. */
void Hu3DGLightKill(HU3D_LIGHTID lightId)
{
    Hu3DGlobalLight[lightId].type = -1;
}

/* Releases the model-local light slot; the follow-up scan leaves HU3D_ATTR_LLIGHT set because the
 * released slot is empty. */
void fn_80028CB8(HU3D_MODELID modelId, HU3D_LLIGHTID lightId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HU3D_LIGHT *lightP = &lbl_80261674[modelP->LLightId[lightId]];
    s16 i;
    lightP->type = -1;
    modelP->LLightId[lightId] = -1;
    for(i=0; i<8; i++) {
        if(modelP->LLightId[i] == -1) {
            break;
        }
    }
    if(i == 8) {
        modelP->attr &= ~(1 << 12);
    }
}

/* Marks every global and local light slot unused during teardown, preserving model light-slot
* references and HU3D_ATTR_LLIGHT. */
void fn_80028D70(void)
{
    HU3D_LIGHTID lightId;
    HU3D_LIGHT *lightP;
    for(lightP=&Hu3DGlobalLight[0], lightId=0; lightId<8; lightId++, lightP++) {
        if(lightP->type != -1) {
            Hu3DGLightKill(lightId);
        }
    }
    for(lightP=&lbl_80261674[0], lightId=0; lightId<48; lightId++, lightP++) {
        if(lightP->type != -1) {
            lightP->type = -1;
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
void fn_80028E48(HU3D_MODELID modelId, HU3D_LLIGHTID lightId, u8 r, u8 g, u8 b, u8 a)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HU3D_LIGHT *lightP = &lbl_80261674[modelP->LLightId[lightId]];
    Hu3DLightColSet(lightP, r, g, b, a);
}

/* Stores a light position and normalizes the supplied direction for GX. */
static void Hu3DLightPSetV(HU3D_LIGHT *lightP, HuVecF *pos, HuVecF *dir)
{
    lightP->pos = *pos;
    VECNormalize(dir, &lightP->dir);
}

/* Updates a global light from vector position and direction values. */
void Hu3DGLightPosSetV(HU3D_LIGHTID lightId, HuVecF *pos, HuVecF *dir)
{
    HU3D_LIGHT *lightP = &Hu3DGlobalLight[lightId];
    Hu3DLightPSetV(lightP, pos, dir);
}

/* Updates a model-local light from vector position and direction values. */
void fn_80028F14(HU3D_MODELID modelId, HU3D_LLIGHTID lightId, HuVecF *pos, HuVecF *dir)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HU3D_LIGHT *lightP = &lbl_80261674[modelP->LLightId[lightId]];
    Hu3DLightPSetV(lightP, pos, dir);
}

/* Scalar light-position setters use this to store position and normalize the supplied direction. */
static void Hu3DLightPSet(HU3D_LIGHT *lightP, float posX, float posY, float posZ, float dirX,
                          float dirY, float dirZ)
{
    lightP->pos.x = posX;
    lightP->pos.y = posY;
    lightP->pos.z = posZ;
    lightP->dir.x = dirX;
    lightP->dir.y = dirY;
    lightP->dir.z = dirZ;
    VECNormalize(&lightP->dir, &lightP->dir);
}

/* Updates a global light from scalar position and direction components. */
void Hu3DGLightPosSet(HU3D_LIGHTID lightId, float posX, float posY, float posZ, float dirX,
                      float dirY, float dirZ)
{
    HU3D_LIGHT *lightP = &Hu3DGlobalLight[lightId];
    Hu3DLightPSet(lightP, posX, posY, posZ, dirX, dirY, dirZ);
}

/* Updates a model-local light from scalar position and direction components. */
void fn_8002903C(HU3D_MODELID modelId, HU3D_LLIGHTID lightId, float posX, float posY,
                      float posZ, float dirX, float dirY, float dirZ)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HU3D_LIGHT *lightP = &lbl_80261674[modelP->LLightId[lightId]];
    Hu3DLightPSet(lightP, posX, posY, posZ, dirX, dirY, dirZ);
}

/* Stores the light position and builds a normalized direction from the supplied angles. */
static void Hu3DLightPASet(HU3D_LIGHT *lightP, float posX, float posY, float posZ, float angleX,
                           float angleY)
{
    lightP->pos.x = posX;
    lightP->pos.y = posY;
    lightP->pos.z = posZ;
    lightP->dir.x =
        sin(M_PI*((float)(angleY-180.0))/180.0)*cos(M_PI*angleX/180.0);
    lightP->dir.y = -sin(M_PI*angleX/180.0);
    lightP->dir.z =
        cos(M_PI*((float)(angleY-180.0))/180.0)*cos(M_PI*angleX/180.0);
    VECNormalize(&lightP->dir, &lightP->dir);
}

/* Positions a global light and aims it using horizontal and vertical angles. */
void fn_800290F4(HU3D_LIGHTID lightId, float posX, float posY, float posZ, float angleX,
                           float angleY)
{
    HU3D_LIGHT *lightP = &Hu3DGlobalLight[lightId];
    Hu3DLightPASet(lightP, posX, posY, posZ, angleX, angleY);
}

/* Positions a model-local light and aims it using horizontal and vertical angles. */
void fn_80029240(HU3D_MODELID modelId, HU3D_LLIGHTID lightId, float posX, float posY,
                           float posZ, float angleX, float angleY)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HU3D_LIGHT *lightP = &lbl_80261674[modelP->LLightId[lightId]];
    Hu3DLightPASet(lightP, posX, posY, posZ, angleX, angleY);
}

/* Stores the light position and points its normalized direction toward the aim point. */
static void Hu3DLightPAimSetV(HU3D_LIGHT *lightP, HuVecF *pos, HuVecF *aim)
{
    lightP->pos = *pos;
    HuSubVecF(&lightP->dir, aim, pos);
    VECNormalize(&lightP->dir, &lightP->dir);
}

/* Points a global light from a vector position toward a vector aim point. */
void Hu3DGLightPosAimSetV(HU3D_LIGHTID lightId, HuVecF *pos, HuVecF *aim)
{
    HU3D_LIGHT *lightP = &Hu3DGlobalLight[lightId];
    Hu3DLightPAimSetV(lightP, pos, aim);
}

/* Points one of a model's local lights from a position toward a vector aim point. */
void fn_80029434(HU3D_MODELID modelId, HU3D_LLIGHTID lightId, HuVecF *pos, HuVecF *aim)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HU3D_LIGHT *lightP = &lbl_80261674[modelP->LLightId[lightId]];
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
void fn_800295A0(HU3D_MODELID modelId, HU3D_LLIGHTID lightId, float posX, float posY,
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
    fn_80029434(modelId, lightId,  &pos, &aim);
}

/* Sets whether camera transforms are applied to this light during GX setup. */
static void Hu3DLightStatSet(HU3D_LIGHT *lightP, BOOL staticF)
{
    if(staticF) {
        lightP->type |= (1 << 15);
    } else {
        lightP->type &= ~(1 << 15);
    }
}
/* Selects camera-relative or fixed placement for a global light. */
void Hu3DGLightStaticSet(HU3D_LIGHTID lightId, BOOL staticF)
{
    HU3D_LIGHT *lightP = &Hu3DGlobalLight[lightId];
    Hu3DLightStatSet(lightP, staticF);
}

/* Selects camera-relative or fixed placement for one of a model's local lights. */
void fn_800296E0(HU3D_MODELID modelId, HU3D_LLIGHTID lightId, BOOL staticF)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HU3D_LIGHT *lightP = &lbl_80261674[modelP->LLightId[lightId]];
    Hu3DLightStatSet(lightP, staticF);
}

/* Returns a local-light slot's position, direction, color, and packed light type. */
s16 fn_80029758(HU3D_LIGHTID LLightId, Vec *pos, Vec *dir, GXColor *color)
{
    HU3D_LIGHT *lightP = &lbl_80261674[LLightId];
    *pos = lightP->pos;
    *dir = lightP->dir;
    *color = lightP->color;
    return lightP->type;
}

/* Returns a global-light slot's position, direction, color, and packed light type. */
s16 fn_800297D4(HU3D_LIGHTID lightId, Vec *pos, Vec *dir, GXColor *color)
{
    HU3D_LIGHT *lightP = &Hu3DGlobalLight[lightId];
    *pos = lightP->pos;
    *dir = lightP->dir;
    *color = lightP->color;
    return lightP->type;
}

/* Imports up to eight HSF light objects into the global light pool during model setup.
* An existing import returns its count without applying staticF again. Requires enough free
* global slots: creation results are used as indices without checking NONE. */
s32 fn_80029850(HU3D_MODELID modelId, s16 staticF)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
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
        if(obj->type != 8) {
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
                Hu3DGLightSpotSet(lightId, obj->light.cutoff, GX_SP_COS);
                break;

            case 1:
                /* Point lights use refBrightness - refDistance as distance, brightness 1, and
                 * direction +Y. */
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
        if(lightNum >= 8) {
            break;
        }
    }
    modelP->lightNum = lightNum;
    return lightNum;
}

/* Finds an imported HSF light by object name and reports NONE when it is absent. */
HU3D_LIGHTID fn_80029D24(HU3D_MODELID modelId, char *objName)
{
    char name[256];
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HSF_DATA *hsf = modelP->hsf;
    HSF_OBJECT *objPtr;
    s16 i;
    s16 lightNo;
    strcpy(name, MakeObjectName((s8 *)objName));
    for(objPtr = hsf->object, i=lightNo=0; i<hsf->objectNum; i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->type != 8) {
            continue;
        }
        if(strcmp(name, obj->name) == 0) {
            return modelP->lightId[lightNo];
        }
        lightNo++;
    }
    OSReport("Error: Not Find Light Name.(%s)\n", objName);
    return -1;
}

/* Enables a global light in this model's draw-time light mask. */
u8 fn_80029E00(HU3D_MODELID modelId, HU3D_LIGHTID lightId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->lightBit |= 1 << lightId;
    return modelP->lightBit;
}

/* Disables a global light in this model's draw-time light mask. */
u8 fn_80029E44(HU3D_MODELID modelId, HU3D_LIGHTID lightId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->lightBit &= (u8)~(1 << lightId);
    return modelP->lightBit;
}

static void lightSet(HU3D_LIGHT *lightP, s16 bit, Mtx cameraMtxXPose, Mtx cameraMtx,
                     float hilitePower);

/* Loads the enabled global and local lights for one model draw. */
s16 fn_80029E8C(HU3D_MODEL *modelP, Mtx cameraMtx, Mtx cameraMtxXPose, float hilitePower)
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

    for (lightP = &Hu3DGlobalLight[0], mask = modelP->lightBit, i = 0; i < 8;
         i++, lightP++, mask >>= 1) {
        if(lightP->type != -1 && (mask & 1)) {
            lightSet(lightP, bit, cameraMtxXPose, cameraMtx, hilitePower);
            lightBit |= bit;
            bit <<= 1;
        }
    }
    if(modelP->attr & (1 << 12)) {
        for(i=0; i<8; i++) {
            if(modelP->LLightId[i] != -1) {
                lightP = &lbl_80261674[modelP->LLightId[i]];
                lightSet(lightP, bit, cameraMtxXPose, cameraMtx, hilitePower);
                lightBit |= bit;
                bit <<= 1;
            }
        }
    }
    return lightBit;
}

/* Loads one GX light, applying camera transforms only for STATIC lights. Directional lights
 * overwrite their stored position with -1000000 times direction; highlights replace attenuation. */
static void lightSet(HU3D_LIGHT *lightP, s16 lightBit, Mtx cameraMtxXPose, Mtx cameraMtx,
                     float hilitePower)
{
    GXLightObj lightObj;
    HuVecF dir;
    HuVecF pos;
    GXColor color;
    switch(LIGHT_TYPE_GET(lightP)) {
        case 0:
            GXInitLightAttn(&lightObj, 1, 0, 0, 1, 0, 0);
            GXInitLightSpot(&lightObj, lightP->cutoff, lightP->func);
            break;

        case 1:
            GXInitLightAttnA(&lightObj, 1, 0, 0);
            GXInitLightDistAttn(&lightObj, 0, 1, GX_DA_OFF);
            VECScale(&lightP->dir, &lightP->pos, -1000000.0f);
            break;

        case 2:
            GXInitLightAttn(&lightObj, 1, 0, 0, 0, 0, 0);
            GXInitLightDistAttn(&lightObj, lightP->cutoff, lightP->brightness, lightP->func);
            break;
    }
    if(lightP->type & (1 << 15)) {
        MTXMultVec(cameraMtxXPose, &lightP->dir, &dir);
        MTXMultVec(cameraMtx, &lightP->pos, &pos);
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

/* Frees the previous custom animation, reads the replacement, and resets reflectMapNo to 0. */
void Hu3DReflectMapSet(ANIMDATA *anim)
{
    if (reflectAnim[0] != (void *)refMapData0) {
        HuMemDirectFree(reflectAnim[0]);
    }
    reflectAnim[0] = HuSprAnimRead(anim);
    lbl_802C01C0 = 0;
}

void fn_8002A2A4(s16 no)
{
    lbl_802C01C0 = no;
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
    GXSetFog(GX_FOG_NONE, 0, 0, 0, 0, lbl_802C01C2);
}

/* Sets the RGB ambient color used by the 3D renderer. */
void Hu3DAmbColorSet(float r, float g, float b)
{
    lbl_802C0190 = r;
    lbl_802C018C = g;
    lbl_802C0188 = b;
}

/* Sets the global shine flag; models with HU3D_ATTR_SHINE still use shine when it is false. */
void fn_8002A390(BOOL shineF)
{
    if(shineF) {
        lbl_802C0184 = TRUE;
    } else {
        lbl_802C0184 = FALSE;
    }
}

#define SHADOW_DEFAULT_SIZE 192

/* Initializes selected shadow cameras, replaces Hu3DShadowCamBit with cameraBit, and enables shadow
 * rendering. */
void fn_8002A3B0(float fov, float near, float far, s16 cameraBit)
{
    s16 i;
    for(i=0; i<16; i++) {
        if((1 << i) & cameraBit) {
            HU3D_SHADOW *shadowP = &Hu3DShadowBuf[i];
            /* Reset the map size to 192 even when reusing an existing texture buffer. */
            shadowP->size = SHADOW_DEFAULT_SIZE;
            if(!shadowP->buf) {
                shadowP->buf =
                    HuMemDirectMalloc(2, SHADOW_DEFAULT_SIZE * SHADOW_DEFAULT_SIZE);
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
            VECNormalize(&shadowP->camUp, &shadowP->camUp);
            shadowP->color.r = shadowP->color.g = shadowP->color.b = 0;
            shadowP->color.a = 128;
        }
    }
    lbl_802C01B8 = cameraBit;
    lbl_802C01B4 = TRUE;
}

void Hu3DShadowCreate(float fov, float near, float far)
{
    fn_8002A3B0(fov, near, far, (1 << 0));
}

/* Updates position, up, and target only for enabled shadow cameras also selected by cameraBit. */
void fn_8002A674(HuVecF *camPos, HuVecF *camUp, HuVecF *camTarget, s16 cameraBit)
{
    s16 i;
    for(i=0; i<16; i++) {
        if((1 << i) & (cameraBit & lbl_802C01B8)) {
            HU3D_SHADOW *shadowP = &Hu3DShadowBuf[i];
            shadowP->camPos = *camPos;
            shadowP->camTarget = *camTarget;
            shadowP->camUp = *camUp;
        }
    }
}

void Hu3DShadowPosSet(HuVecF *camPos, HuVecF *camUp, HuVecF *camTarget)
{
    fn_8002A674(camPos, camUp, camTarget, (1 << 0));
}

/* Sets shadow texture alpha only for enabled shadow cameras also selected by cameraBit. */
void fn_8002A7D8(float tpLvl, s16 cameraBit)
{
    s16 i;
    for(i=0; i<16; i++) {
        if((1 << i) & (cameraBit & lbl_802C01B8)) {
            Hu3DShadowBuf[i].color.a = tpLvl*255.0f;
        }
    }
}

void Hu3DShadowTPLvlSet(float tpLvl)
{
    fn_8002A7D8(tpLvl, (1 << 0));
}

/* Resizes buffers only for enabled shadow cameras also selected by cameraBit. */
void fn_8002A8CC(u16 size, s16 cameraBit)
{
    s16 i;
    for(i=0; i<16; i++) {
        if((1 << i) & (cameraBit & lbl_802C01B8)) {
            HU3D_SHADOW *shadowP = &Hu3DShadowBuf[i];
            shadowP->size = size;
            if(shadowP->buf) {
                HuMemDirectFree(shadowP->buf);
            }
            shadowP->buf = HuMemDirectMalloc(2, size*size);
        }
    }
}

void Hu3DShadowSizeSet(u16 size)
{
    fn_8002A8CC(size, (1 << 0));
}

/* Sets shadow tint only for enabled shadow cameras also selected by cameraBit. */
void fn_8002AA3C(u8 r, u8 g, u8 b, s16 cameraBit)
{
    s16 i;
    for(i=0; i<16; i++) {
        if((1 << i) & (cameraBit & lbl_802C01B8)) {
            HU3D_SHADOW *shadowP = &Hu3DShadowBuf[i];
            shadowP->color.r = r;
            shadowP->color.g = g;
            shadowP->color.b = b;
        }
    }
}

void fn_8002AAB4(u8 r, u8 g, u8 b)
{
    fn_8002AA3C(r, g, b, (1 << 0));
}

/* Renders shadow-casting models and copies the active shadow map to its texture, then fills the
* framebuffer with BGColor when bgColorF is true, otherwise black. */
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
    MTXPerspective(proj, lbl_802C01AC->fov, HU_DISP_ASPECT, lbl_802C01AC->near, lbl_802C01AC->far);
    GXSetProjection(proj, GX_PERSPECTIVE);
    if(lbl_802C01AC->size <= 240) {
        GXSetScissor(2, 2, (lbl_802C01AC->size*2)-4, (lbl_802C01AC->size*2)-4);
        GXSetViewport(0, 0, (lbl_802C01AC->size*2), (lbl_802C01AC->size*2), 0, 1);
        dataSize = (lbl_802C01AC->size/2)*(lbl_802C01AC->size/2);
    } else {
        GXSetScissor(1, 1, lbl_802C01AC->size-2, lbl_802C01AC->size-2);
        GXSetViewport(0, 0, lbl_802C01AC->size, lbl_802C01AC->size, 0, 1);
        dataSize = lbl_802C01AC->size*lbl_802C01AC->size;
    }
    MTXLookAt(lbl_802613F4, &lbl_802C01AC->camPos, &lbl_802C01AC->camUp, &lbl_802C01AC->camTarget);
    MTXCopy(lbl_802613F4, lbl_802C01AC->lookAtMtx);
    modelP = &lbl_802C01C8[0];
    lbl_802C01B0 = TRUE;
    GXInvalidateTexAll();
    GXSetFog(GX_FOG_NONE, 0, 0, 0, 0, lbl_802C01C2);
    for(i=0; i<512; i++, modelP++) {
        if(!modelP->hsf) {
            continue;
        }
        if ((modelP->attr & (1 << 2)) &&
            (modelP->attr & ((1 << 0) | (1 << 24))) == 0 &&
            (modelP->attr & (1 << 15)) == 0) {
            if(lbl_802C01B8 == (1 << 0) || (lbl_802C01A4 & modelP->cameraBit)) {
                BOOL vtxInvalidateF;
                /* The shadow pass evaluates motion for models marked MOTION_OFF, which normal
                 * drawing skips. */
                if(modelP->attr & (1 << 3)) {
                    vtxInvalidateF = FALSE;
                    if(modelP->motId != -1) {
                        fn_8002E4C4(i, modelP->motId, modelP->motWork.time, FALSE);
                    }
                    if(modelP->motIdShift != -1) {
                        fn_8002ECDC(i);
                    }
                    if(modelP->motIdOvl != -1) {
                        fn_8002E4C4(i, modelP->motIdOvl, modelP->motOvlWork.time, TRUE);
                    }
                    if(modelP->attr & (1 << 10)) {
                        ClusterMotionExec(modelP);
                        vtxInvalidateF = TRUE;
                    }
                    if(modelP->motIdShape != -1) {
                        /* With no main motion, shape evaluation restores base transforms after
                         * the shift and overlay passes and can discard their transform results. */
                        if(modelP->motId == -1) {
                            fn_8002E4C4(i, modelP->motIdShape, modelP->motShapeWork.time, FALSE);
                        } else {
                            fn_8002E4C4(i, modelP->motIdShape, modelP->motShapeWork.time, TRUE);
                        }
                    }
                    if ((modelP->attr & ((1 << 7) | (1 << 4))) == 0 ||
                        (modelP->attr & (1 << 25))) {
                        vtxInvalidateF = TRUE;
                        InitVtxParm(modelP->hsf);
                        if(modelP->motIdShape != -1) {
                            ShapeProc(modelP->hsf);
                        }
                        if(modelP->attr & (1 << 10)) {
                            ClusterProc(modelP);
                        }
                        if(modelP->hsf->cenvNum) {
                            EnvelopeProc(modelP->hsf);
                        }
                        PPCSync();
                    }
                    modelP->attr |= (1 << 11);
                }
                fn_80022E38(rot, modelP->rot.x, modelP->rot.y, modelP->rot.z);
                MTXScale(temp, modelP->scale.x, modelP->scale.y, modelP->scale.z);
                MTXConcat(rot, temp, temp);
                mtxTransCat(temp, modelP->pos.x, modelP->pos.y, modelP->pos.z);
                PSMTXConcat(lbl_802613F4, temp, modelview);
                PSMTXConcat(modelview, modelP->mtx, modelview);
                fn_80017B34(modelP, modelview, &modelP->scale);
            }
        }
    }
    fn_80020024();
    lbl_802C01B0 = FALSE;
    if(lbl_802C01AC->size <= 240) {
        GXSetTexCopySrc(0, 0, lbl_802C01AC->size*2, lbl_802C01AC->size*2);
        GXSetTexCopyDst(lbl_802C01AC->size, lbl_802C01AC->size, GX_CTF_R8, GX_TRUE);
    } else {
        GXSetTexCopySrc(0, 0, lbl_802C01AC->size, lbl_802C01AC->size);
        GXSetTexCopyDst(lbl_802C01AC->size, lbl_802C01AC->size, GX_CTF_R8, GX_FALSE);
    }
    GXCopyTex(lbl_802C01AC->buf, GX_TRUE);
    GXSetViewport(0.0f, 0.0f, RenderMode->fbWidth, RenderMode->xfbHeight, 0.0f, 1.0f);
    GXSetScissor(0, 0, RenderMode->fbWidth, RenderMode->efbHeight);
    MTXOrtho(proj, 0, 1, 0, 1, 0, 1);
    GXSetProjection(proj, GX_ORTHOGRAPHIC);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_U8, 0);
    if(bgColorF) {
        GXSetTevColor(GX_TEVREG0, lbl_802C01C2);
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

/* Renders the reflection model into its texture before reflective models are drawn, then fills
* the framebuffer with BGColor after copying the texture. */
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
    VECNormalize(&pos, &pos);
    VECScale(&pos, &pos, -100);
    target.x = target.y = target.z = 0;
    MTXLookAt(lbl_802613F4, &pos, &cameraP->up, &target);
    modelP = &lbl_802C01C8[lbl_802C0198];
    GXInvalidateTexAll();
    GXSetFog(GX_FOG_NONE, 0, 0, 0, 0, lbl_802C01C2);
    if(!modelP->hsf) {
        return;
    }
    if((modelP->attr & (1 << 0)) || (modelP->attr & (1 << 15))) {
        return;
    }
    if(modelP->motId != -1) {
        fn_8002E4C4(lbl_802C0198, modelP->motId, modelP->motWork.time, FALSE);
    }
    if ((modelP->attr & ((1 << 7) | (1 << 4))) == 0 ||
        (modelP->attr & (1 << 25))) {
        InitVtxParm(modelP->hsf);
        if(modelP->motIdShape != -1) {
            ShapeProc(modelP->hsf);
        }
        if(modelP->attr & (1 << 10)) {
            ClusterProc(modelP);
        }
        if(modelP->hsf->cenvNum) {
            EnvelopeProc(modelP->hsf);
        }
        PPCSync();
    }
    // This reads an uninitialized matrix; the next concatenation overwrites its result.
    MTXConcat(lbl_802613F4, temp, modelview);
    MTXConcat(lbl_802613F4, modelP->mtx, modelview);
    fn_80017B34(modelP, modelview, &modelP->scale);
    fn_80020024();
    GXSetTexCopySrc(0, 0, REFLECT_TEX_W, REFLECT_TEX_H);
    GXSetTexCopyDst(REFLECT_TEX_W, REFLECT_TEX_H, GX_TF_RGB5A3, GX_FALSE);
    GXCopyTex(lbl_802C0194->bmp->data, GX_TRUE);
    GXPixModeSync();
    GXSetViewport(0.0f, 0.0f, RenderMode->fbWidth, RenderMode->xfbHeight, 0.0f, 1.0f);
    GXSetScissor(0, 0, RenderMode->fbWidth, RenderMode->efbHeight);
    MTXOrtho(proj, 0, 1, 0, 1, 0, 1);
    GXSetProjection(proj, GX_ORTHOGRAPHIC);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetTevColor(GX_TEVREG0, lbl_802C01C2);
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
    for(projP=&Hu3DProjection[0], projId=0; projId<4; projId++, projP++) {
        if(!projP->anim) {
            break;
        }
    }
    if(projId == 4) {
        return -1;
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
    VECNormalize(&projP->camUp, &projP->camUp);
    projP->alpha = 128;
    lbl_802C01A8++;
    return projId;
}

/* Releases the projection's animation reference, marks its slot unused, and leaves
* Hu3DProjectionNum unchanged. Shared animation data is freed only after its final user. */
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
void fn_8002BAF8(void *animData, HU3D_MODELID modelId, char *bmpName)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
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
    data = HuMemDirectMallocNum(2, dataSize, modelP->mallocNo);
    dataPtr = data;
    animBmp = anim->bmp;
    bmp->data = dataPtr;
    bmp->sizeX = animBmp->sizeX;
    bmp->sizeY = animBmp->sizeY;
    bmp->palSize = animBmp->palNum;
    bmp->palData = animBmp->palData;
    bmp->maxLod = anim->bmpNum;
    switch (animBmp->dataFmt) {
        case 0:
            bmp->dataFmt = 6;
            break;

        case 2:
            bmp->dataFmt = 5;
            break;

        case 3:
            bmp->dataFmt = 10;
            bmp->pixSize = 8;
            break;

        case 4:
            bmp->dataFmt = 10;
            bmp->pixSize = 4;
            break;
    }
    for(i=0; i<anim->bmpNum; i++, animBmp++) {
        memcpy(data, animBmp->data, animBmp->dataSize);
        data += animBmp->dataSize;
    }
    DCFlushRange(dataPtr, dataSize);
}

/* Copies a framebuffer rectangle to a texture; mipmapF halves the destination width and height. */
void fn_8002BCDC(s16 x, s16 y, s16 w, s16 h, GXTexFmt texFmt, s16 mipmapF, void *buf)
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

/* Registers a framebuffer-copy hook before a layer, or after it when layerNo includes
* HU3D_LAYER_HOOK_POST. */
void fn_8002BDB4(s16 layerNo, s16 x, s16 y, s16 w, s16 h, GXTexFmt texFmt, s16 mipmapF,
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
    fn_80025918(layerNo, FbCopyLayerHook);
}

/* Copies the framebuffer rectangle configured for this layer. */
static void FbCopyLayerHook(s16 layerNo)
{
    FBCOPY_LAYER *copyLayerP = &FbCopyLayer[layerNo];
    fn_8002BCDC(copyLayerP->sourceX, copyLayerP->sourceY, copyLayerP->width, copyLayerP->height,
                   copyLayerP->texFmt, copyLayerP->mipmapF, copyLayerP->textureBuffer);
}

/* Draws a color-write-disabled quad one unit before the camera far plane to clear scene depth. */
void fn_8002BF48(void)
{
    HU3D_CAMERA *cameraP = &Hu3DCamera[lbl_802C01A6];
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
    pos.x = pos.y = z*tan(M_PI*(cameraP->fov/2)/180.0);
    pos.x *= cameraP->aspect;
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-pos.x, -pos.y, -z);
    GXPosition3f32(pos.x, -pos.y, -z);
    GXPosition3f32(pos.x, pos.y, -z);
    GXPosition3f32(-pos.x, pos.y, -z);
    GXSetColorUpdate(GX_TRUE);
}

void fn_8002C2AC(s16 layerNo)
{
    fn_80025918(layerNo, (HU3D_LAYER_HOOK)fn_8002BF48);
}

/* Prints loaded model and motion IDs with their source file locations. */
void fn_8002C31C(void)
{
    HU3D_MODEL *modelP;
    s16 i;
    HU3D_MOTION *motionP;

    modelP = lbl_802C01C8;
    OSReport("Model ******\n");
    OSReport("ID :Dir :File\n");
    for(i=0; i<512; i++, modelP++) {
        if(modelP->hsf && (modelP->attr & ((1 << 4)|(1 << 16))) == 0) {
            OSReport("%3d:%04x:%3d", i, fn_8000C1B4(modelP->hsf) >> 16,
                     FILENUM(fn_8000C1B4(modelP->hsf)));
            if(modelP->motId != -1) {
                OSReport(" motionNo %d\n", modelP->motId);
            } else {
                OSReport("\n");
            }
        }

    }
    OSReport("Motion *****\n");
    OSReport("ID :Dir :File\n");
    for(motionP=lbl_802624B8, i=0; i<384; i++, motionP++) {
        if(motionP->hsf) {
            OSReport("%3d:%04x:%3d\n", i, fn_8000C1B4(motionP->hsf) >> 16,
                     FILENUM(fn_8000C1B4(motionP->hsf)));
        }
    }
}
