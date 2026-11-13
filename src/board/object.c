/* Manages board models, their motions, transforms, visibility, and shadows. */
#include <math.h>

#include "game/board/object.h"
#include "game/board/audio.h"
#include "game/board/camera.h"
#include "game/board/main.h"
#include "game/data.h"
#include "game/memory.h"

#include "string.h"
#include "game/board/math.h"

static OMOBJ *objManOMObj;
static MBOBJMODEL *objManData;
static s16 objManNum;
static int objManDispNum;
static int dataDirDay;
static int dataDirNight;

static void ObjManOMExec(OMOBJ *obj);
static MBMODELID ObjManObjCreate(int charNo, int dataNum, const int *motDataNum, BOOL linkF);
static BOOL ObjManModelCreate(MBOBJMODEL *modelP, int dataNum, BOOL linkF);
static int ObjManMotionCreate(MBOBJMODEL *modelP, int motNum, const int *motDataNum);
static MBOBJMODEL *ObjManLinkSearch(int dataNum);

/* Replaces an opposite-time directory with the configured directory for the current board
 * time, preserving the file number. */
int mbObjDataNumGet(int dataNum)
{
    int result;

    if (!lbl_80265B70.curTime) {
        if (DIRNUM(dataNum) == dataDirNight) {
            result = dataDirDay | FILENUM(dataNum);
            goto done;
        }
    } else if (DIRNUM(dataNum) == dataDirDay) {
        result = dataDirNight | FILENUM(dataNum);
        goto done;
    }
    result = dataNum;
done:
    return result;
}

/* Called during board setup to allocate the model table and register its update callback. */
void mbObjInit(void)
{
    objManNum = 0;
    dataDirDay = dataDirNight = DIRNUM(-1);
    objManData = fn_802137C0(sizeof(MBOBJMODEL) * MB_OBJ_MAX);
    objManOMObj = fn_8003AB38(mbObjMan, 32259, 0, 0, -1, ObjManOMExec);
    omSetStatBit(objManOMObj, (1 << 5) | (1 << 7));
}

/* Called during board teardown to kill active models and release the model table. */
void mbObjClose(void)
{
    int i;

    for (i = 1; i < MB_OBJ_MAX; i++) {
        if (objManData[i].id != 0) {
            mbObjKill(i);
        }
    }
    if (objManData) {
        MBOBJMODEL *modelP = objManData;

        HuMemDirectFree(modelP);
        objManData = NULL;
    }
}

/* Registered by mbObjInit; updates active board models during each object-manager execution. */
static void ObjManOMExec(OMOBJ *obj)
{
    MBOBJMODEL *modelP;
    HuVecF pos;
    HuVecF pos2D;
    u32 attr;
    float fadePos;
    float fade;
    int i;

    if (mbExitCheck()) {
        objManOMObj = NULL;
        omDelObjEx(fn_8000DD9C(), obj);
        return;
    }

    modelP = &objManData[1];
    objManDispNum = 0;
    for (i = 1; i < MB_OBJ_MAX; i++, modelP++) {
        if (modelP->id == 0) {
            continue;
        }
        if (modelP->cullRadius >= 0.0f) {
            pos = modelP->pos;
            PSVECAdd(&pos, &modelP->offset, &pos);
            if (!mbCameraCullCheck(&pos, modelP->cullRadius)) {
                continue;
            }
        }
        if (modelP->fadeF) {
            /* Convert pos to screen depth for fading. Only the culling branch assigns pos;
             * disabled culling leaves it uninitialized here. */
            mbPos3Dto2D(&pos, &pos2D);
            if (pos2D.z < 1000.0f) {
                fadePos = pos2D.z / 1000.0f;
                if (fadePos < 0.0f) {
                    fadePos = 0.0f;
                }
                fade = 1.0 - cos((M_PI * (90.0f * fadePos)) / 180.0);
            } else {
                fade = 1.0f;
            }
            fn_80026CBC(modelP->modelId, fade * ((float) modelP->alpha / 255.0f));
        }

        /* Read the current model attribute mask, then ignore it before applying display state. */
        attr = fn_80026A90(modelP->modelId);
        if (!modelP->dispF) {
            fn_800269E8(modelP->modelId, (1 << 0));
        } else {
            fn_80026A44(modelP->modelId, (1 << 0));
            objManDispNum++;
        }
        fn_80026760(modelP->modelId,
            modelP->pos.x + modelP->offset.x,
            modelP->pos.y + modelP->offset.y,
            modelP->pos.z + modelP->offset.z);
        fn_80026838(modelP->modelId, &modelP->rot);
        fn_800268E0(modelP->modelId, &modelP->scale);
    }
}

/* Allocates a model slot, creates its model and motions, and applies board-object defaults. */
static MBMODELID ObjManObjCreate(int charNo, int dataNum, const int *motDataNum, BOOL linkF)
{
    MBOBJMODEL *modelP;
    HU3D_MODEL *model3DP;
    BOOL result;
    int motNum;
    int i;

    for (modelP = &objManData[1], i = 1; i < MB_OBJ_MAX; i++, modelP++) {
        if (modelP->id == 0) {
            break;
        }
    }
    if (i >= MB_OBJ_MAX) {
        return MB_MODEL_NONE;
    }

    memset(modelP, 0, sizeof(MBOBJMODEL));
    modelP->id = i;
    modelP->charNo = charNo;
    /* Both creation return values are stored in result and ignored; motion creation below
     * overwrites the model-creation result. */
    result = ObjManModelCreate(modelP, dataNum, linkF);

    motNum = 0;
    if (motDataNum != NULL) {
        while (motDataNum[motNum] >= 0) {
            motNum++;
        }
    }
    for (i = 0; i < MB_OBJ_MOT_MAX; i++) {
        modelP->motId[i] = -1;
    }
    result = ObjManMotionCreate(modelP, motNum, motDataNum);

    modelP->dispF = TRUE;
    modelP->dispOldF = modelP->dispF;
    modelP->cameraBit = (1 << 0);
    modelP->layer = 0;
    modelP->cullRadius = -1.0f;
    modelP->alpha = 255;
    fn_80026B64(modelP->modelId, modelP->cameraBit);
    fn_80026B8C(modelP->modelId, modelP->layer);
    mbObjPosSet(modelP->id, 0.0f, 0.0f, 0.0f);
    mbObjRotSet(modelP->id, 0.0f, 0.0f, 0.0f);
    mbObjScaleSet(modelP->id, 1.0f, 1.0f, 1.0f);
    mbObjOffsetSet(modelP->id, 0.0f, 0.0f, 0.0f);
    mbObjAmbSet(modelP->id, 1.0f, 1.0f, 1.0f);
    objManNum++;

    model3DP = &lbl_802C01C8[modelP->modelId];
    if (model3DP->linkMdlId < 0) {
        mbObjCullInit(modelP->id);
    }
    return modelP->id;
}

/* Called by board features to create a model; linkF permits reuse of a loaded asset. */
MBMODELID mbObjCreate(int dataNum, const int *motDataNum, BOOL linkF)
{
    MBMODELID modelId;

    /* Seed the shared source model hidden when this is the first linked instance. */
    if (linkF && !ObjManLinkSearch(dataNum)) {
        modelId = ObjManObjCreate(-1, dataNum, motDataNum, linkF);
        mbObjDispSet(modelId, FALSE);
    }
    return ObjManObjCreate(-1, dataNum, motDataNum, linkF);
}

/* Called by board features to create a character model and place it on the character layer. */
MBMODELID mbObjCharCreate(int charNo, int dataNum, const int *motDataNum, BOOL linkF)
{
    MBMODELID modelId = ObjManObjCreate(charNo, dataNum, motDataNum, linkF);

    mbObjLayerSet(modelId, 3);
    return modelId;
}

/* ObjManObjCreate creates a model here: non-character entries link or load configured day/night
 * data; character entries call CharModelCreate and skip data loading. */
static BOOL ObjManModelCreate(MBOBJMODEL *modelP, int dataNum, BOOL linkF)
{
    MBOBJMODEL *linkP = NULL;
    void *data;
    int readDataNum;

    modelP->linkNum = 0;
    if (linkF && modelP->charNo == -1) {
        linkP = ObjManLinkSearch(dataNum);
        if (linkP) {
            modelP->modelId = fn_80025D4C(linkP->modelId);
            modelP->dataNum = 0;
            modelP->linkP = linkP;
            linkP->linkNum++;
            return TRUE;
        }
    }

    modelP->linkP = NULL;
    if (modelP->charNo == -1) {
        if (!lbl_80265B70.curTime) {
            if (DIRNUM(dataNum) == dataDirNight) {
                readDataNum = dataDirDay | FILENUM(dataNum);
                goto read_data;
            }
        } else if (DIRNUM(dataNum) == dataDirDay) {
            readDataNum = dataDirNight | FILENUM(dataNum);
            goto read_data;
        }
        readDataNum = dataNum;
read_data:
        data = fn_80008DD0(readDataNum, 0x10000000, 2);
        if (!data) {
            /* These remaining-memory values are read here but do not affect the load path. */
            s32 modelRestMem = fn_8000B790(2)-HuMemUsedMallocSizeGet(2);
            s32 dvdRestMem = fn_8000B790(3)-HuMemUsedMallocSizeGet(3);
        }
    } else {
        data = NULL;
    }

    if (modelP->charNo == -1) {
        modelP->modelId = fn_80025A04(data);
    } else {
        modelP->modelId = fn_80057068(modelP->charNo, (1 << 2));
    }
    if (!linkF) {
        modelP->dataNum = 0;
    } else {
        modelP->dataNum = dataNum;
    }
    return TRUE;
}

/* ObjManObjCreate stores the built-in motion in slot zero and loads requested additional
 * motions here. Data or motion creation failure returns -1 before setting the motion count. */
static int ObjManMotionCreate(MBOBJMODEL *modelP, int motNum, const int *motDataNum)
{
    void *data;
    HU3D_MOTIONID motId;
    int i;

    modelP->motId[0] = lbl_802C01C8[modelP->modelId].motId;
    if (motDataNum == NULL) {
        modelP->motNum = 1;
        return TRUE;
    }

    data = NULL;
    for (i = 0; i < motNum; i++) {
        if (modelP->charNo == -1) {
            data = fn_80008DD0(mbObjDataNumGet(motDataNum[i]), 0x10000000, 2);
            if (data == NULL) {
                return -1;
            }
            motId = fn_80030458(modelP->modelId, data);
        } else {
            motId = fn_8005B0B8(modelP->charNo, motDataNum[i]);
        }
        if (motId < 0) {
            return -1;
        }
        modelP->motId[i + 1] = motId;
        modelP->motData[i + 1] = data;
        modelP->motDataNum[i + 1] = motDataNum[i];
    }
    modelP->motNum = motNum + 1;
    return TRUE;
}

/* Model creation finds any active model with this data number for linking; no character filter
 * is applied. */
static MBOBJMODEL *ObjManLinkSearch(int dataNum)
{
    MBOBJMODEL *modelP;
    int i;

    for (modelP = &objManData[1], i = 1; i < MB_OBJ_MAX; i++, modelP++) {
        if (modelP->id != 0 && modelP->dataNum == dataNum) {
            return modelP;
        }
    }
    return NULL;
}

/* Called when a board feature releases a model; linked instances are removed before their
 * source. */
void mbObjKill(MBMODELID modelId)
{
    MBOBJMODEL *modelP = &objManData[modelId];
    MBOBJMODEL *searchP;
    int i;

    if (modelP->charNo == -1) {
        if (modelP->linkNum != 0) {
            for (searchP = &objManData[1], i = 1; i < MB_OBJ_MAX; i++, searchP++) {
                if (searchP->id != 0 && searchP->linkP == modelP) {
                    mbObjKill(searchP->id);
                }
            }
            return;
        }
        modelP->motId[0] = -1;
        if (modelP->modelId >= 0) {
            fn_800262D0(modelP->modelId);
            modelP->modelId = -1;
        }
        for (i = 1; i < MB_OBJ_MOT_MAX; i++) {
            if (modelP->motId[i] >= 0) {
                fn_8002C624(modelP->motId[i]);
                modelP->motId[i] = -1;
            }
            modelP->motData[i] = NULL;
        }
        modelP->dataNum = 0;
        if (modelP->linkP) {
            if (--modelP->linkP->linkNum == 0) {
                mbObjKill(modelP->linkP->id);
            }
        }
    } else {
        for (i = 0; i < MB_OBJ_MOT_MAX; i++) {
            modelP->motId[i] = -1;
            modelP->motData[i] = NULL;
        }
        if (modelP->modelId >= 0) {
            modelP->modelId = -1;
        }
        fn_8005BB8C(modelP->charNo);
        modelP->charNo = -1;
    }
    objManNum--;
    modelP->id = 0;
    modelP->dispF = FALSE;
}

/* Board setup records the day and night data directories used for seasonal model and motion
 * loads. */
void mbObjDirSet(int day, int night)
{
    dataDirDay = day;
    dataDirNight = night;
}

MBOBJMODEL *mbObjGet(MBMODELID modelId)
{
    return &objManData[modelId];
}

BOOL mbObjDispGet(MBMODELID modelId)
{
    return objManData[modelId].dispF;
}

void mbObjDispSet(MBMODELID modelId, BOOL dispF)
{
    objManData[modelId].dispF = dispF;
}

/* Board features call this to set the cameras that can render one model. */
void mbObjCameraSet(MBMODELID modelId, u16 cameraBit)
{
    MBOBJMODEL *modelP = &objManData[modelId];

    modelP->cameraBit = cameraBit;
    fn_80026B64(modelP->modelId, cameraBit);
}

int mbObjLayerGet(MBMODELID modelId)
{
    return objManData[modelId].layer;
}

/* Board features call this to assign one model to a render layer. */
void mbObjLayerSet(MBMODELID modelId, u8 layer)
{
    MBOBJMODEL *modelP = &objManData[modelId];

    modelP->layer = layer;
    fn_80026B8C(modelP->modelId, layer);
}

/* Board features can move all active engine models to one layer; stored layer fields are
 * unchanged. */
void mbObjLayerSetAll(u8 layer)
{
    MBOBJMODEL *modelP;
    int i;

    for (modelP = &objManData[1], i = 1; i < MB_OBJ_MAX; i++, modelP++) {
        if (modelP->id != 0) {
            fn_80026B8C(modelP->modelId, layer);
        }
    }
}

HU3D_MODELID mbObjModelIDGet(MBMODELID modelId)
{
    return objManData[modelId].modelId;
}

void mbObjPosGet(MBMODELID modelId, HuVecF *pos)
{
    *pos = objManData[modelId].pos;
}

void mbObjPosSetV(MBMODELID modelId, const HuVecF *pos)
{
    mbObjPosSet(modelId, pos->x, pos->y, pos->z);
}

/* Board features set the model position; its stored offset is added for rendering. */
void mbObjPosSet(MBMODELID modelId, float x, float y, float z)
{
    MBOBJMODEL *modelP = &objManData[modelId];

    modelP->pos.x = x;
    modelP->pos.y = y;
    modelP->pos.z = z;
    fn_80026760(modelP->modelId,
        modelP->pos.x + modelP->offset.x,
        modelP->pos.y + modelP->offset.y,
        modelP->pos.z + modelP->offset.z);
}

void mbObjOffsetGet(MBMODELID modelId, HuVecF *offset)
{
    *offset = objManData[modelId].offset;
}

void mbObjOffsetSetV(MBMODELID modelId, const HuVecF *offset)
{
    mbObjOffsetSet(modelId, offset->x, offset->y, offset->z);
}

/* Board features set a model-local position offset, then refresh its rendered position. */
void mbObjOffsetSet(MBMODELID modelId, float x, float y, float z)
{
    MBOBJMODEL *modelP = &objManData[modelId];
    HuVecF pos;

    modelP->offset.x = x;
    modelP->offset.y = y;
    modelP->offset.z = z;
    mbObjPosGet(modelId, &pos);
    mbObjPosSetV(modelId, &pos);
}

void mbObjRotGet(MBMODELID modelId, HuVecF *rot)
{
    *rot = objManData[modelId].rot;
}

void mbObjRotSetV(MBMODELID modelId, const HuVecF *rot)
{
    mbObjRotSet(modelId, rot->x, rot->y, rot->z);
}

/* Board features set all three stored rotation components and apply them to the model. */
void mbObjRotSet(MBMODELID modelId, float x, float y, float z)
{
    MBOBJMODEL *modelP = &objManData[modelId];

    modelP->rot.x = x;
    modelP->rot.y = y;
    modelP->rot.z = z;
    fn_80026838(modelP->modelId, &modelP->rot);
}

float mbObjRotYGet(MBMODELID modelId)
{
    return objManData[modelId].rot.y;
}

/* Board features change only yaw while preserving the model's X and Z rotations. */
void mbObjRotYSet(MBMODELID modelId, float rotY)
{
    HuVecF rot;

    mbObjRotGet(modelId, &rot);
    mbObjRotSet(modelId, rot.x, rotY, rot.z);
}

void mbObjScaleGet(MBMODELID modelId, HuVecF *scale)
{
    *scale = objManData[modelId].scale;
}

void mbObjScaleSetV(MBMODELID modelId, const HuVecF *scale)
{
    mbObjScaleSet(modelId, scale->x, scale->y, scale->z);
}

/* Board features set the model's per-axis scale and apply the resulting vector. */
void mbObjScaleSet(MBMODELID modelId, float x, float y, float z)
{
    MBOBJMODEL *modelP = &objManData[modelId];

    modelP->scale.x = x;
    modelP->scale.y = y;
    modelP->scale.z = z;
    fn_800268E0(modelP->modelId, &modelP->scale);
}

/* Board features read the current engine model matrix into mtx. */
void mbObjMtxGet(MBMODELID modelId, Mtx *mtx)
{
    MBOBJMODEL *modelP = &objManData[modelId];
    HU3D_MODEL *model3DP = &lbl_802C01C8[modelP->modelId];

    PSMTXCopy(model3DP->mtx, *mtx);
}

/* Board features replace the engine model matrix with mtx. */
void mbObjMtxSet(MBMODELID modelId, Mtx *mtx)
{
    MBOBJMODEL *modelP = &objManData[modelId];
    HU3D_MODEL *model3DP = &lbl_802C01C8[modelP->modelId];

    PSMTXCopy(*mtx, model3DP->mtx);
}

/* Board features enable supplied model or motion attribute bits through Hu3DModelAttrSet. */
void mbObjAttrSet(MBMODELID modelId, u32 attr)
{
    MBOBJMODEL *modelP = &objManData[modelId];

    fn_800269E8(modelP->modelId, attr);
}

/* Board features clear supplied model or motion attribute bits through Hu3DModelAttrReset. */
void mbObjAttrReset(MBMODELID modelId, u32 attr)
{
    MBOBJMODEL *modelP = &objManData[modelId];

    fn_80026A44(modelP->modelId, attr);
}

void mbObjAmbSet(MBMODELID modelId, float r, float g, float b)
{
    fn_800273EC(objManData[modelId].modelId, r, g, b);
}

/* Board lighting updates use this to apply one ambient color to every active model. */
void mbObjAmbSetAll(float r, float g, float b)
{
    MBOBJMODEL *modelP;
    int i;

    for (modelP = &objManData[1], i = 1; i < MB_OBJ_MAX; i++, modelP++) {
        if (modelP->id != 0) {
            mbObjAmbSet(modelP->id, r, g, b);
        }
    }
}

u8 mbObjAlphaGet(MBMODELID modelId)
{
    return objManData[modelId].alpha;
}

/* Board features set model opacity from 0 (transparent) through 255 (opaque). */
void mbObjAlphaSet(MBMODELID modelId, u8 alpha)
{
    MBOBJMODEL *modelP = &objManData[modelId];

    modelP->alpha = alpha;
    fn_80026CBC(modelP->modelId, (float) alpha / 255.0f);
}

void mbObjFadeSet(MBMODELID modelId, BOOL fadeF)
{
    objManData[modelId].fadeF = fadeF;
}

/* Board effects toggle whether this model writes depth while rendering. */
void mbObjZWriteOffSet(MBMODELID modelId, BOOL zWriteOffF)
{
    MBOBJMODEL *modelP = &objManData[modelId];

    if (zWriteOffF) {
        fn_800269E8(modelP->modelId, (1 << 1));
    } else {
        fn_80026A44(modelP->modelId, (1 << 1));
    }
}

/* Board features set the camera-culling radius; a negative input disables culling. */
void mbObjCullRadiusSet(MBMODELID modelId, float radius)
{
    if (radius < 0.0f) {
        radius = -1.0f;
    }
    objManData[modelId].cullRadius = radius;
}

/* Board features hook hookModelId to objName on modelId, then set the hooked model's stored
 * base position to zero; its offset remains applied. */
void mbObjHookSet(MBMODELID modelId, char *objName, MBMODELID hookModelId)
{
    MBOBJMODEL *modelP = &objManData[modelId];
    MBOBJMODEL *hookP = &objManData[hookModelId];

    fn_8002741C(modelP->modelId, objName, hookP->modelId);
    mbObjPosSet(hookModelId, 0.0f, 0.0f, 0.0f);
}

/* Board features detach all model hooks from this model. */
void mbObjHookReset(MBMODELID modelId)
{
    MBOBJMODEL *modelP = &objManData[modelId];

    fn_80027508(modelP->modelId);
}

/* Board features detach the named hooked object from this model. */
void mbObjHookObjReset(MBMODELID modelId, char *objName)
{
    MBOBJMODEL *modelP = &objManData[modelId];

    fn_800275B4(modelP->modelId, objName);
}

/* Board features play a sound at the model's stored base position; its render offset is not
 * included. */
int mbObjSePlay(MBMODELID modelId, s16 seId)
{
    MBOBJMODEL *modelP = &objManData[modelId];

    return mbAudFXPosPlay(seId, &modelP->pos);
}

/* Character board models use this to enable or disable voice for a motion slot. */
void mbObjMotionVoiceOnSet(MBMODELID modelId, int motNo, BOOL voiceOnF)
{
    MBOBJMODEL *modelP = &objManData[modelId];

    if (modelP->charNo != -1) {
        fn_8005DEB0(modelP->charNo, modelP->motDataNum[motNo], voiceOnF);
    }
}

/* Board features query the motion slot most recently selected for this model. */
int mbObjMotionGet(MBMODELID modelId)
{
    MBOBJMODEL *modelP = &objManData[modelId];

    return modelP->motNo;
}

/* Board features select a motion, reset speed and range, and seek to the end for reverse flags.
 * Only loop/pause bits are cleared before applying attrs and recording motNo, so old reverse
 * and shape flags can persist. */
void mbObjMotionSet(MBMODELID modelId, int motNo, u32 attr)
{
    MBOBJMODEL *modelP = &objManData[modelId];

    if (modelP->charNo == -1) {
        fn_8002C95C(modelP->modelId, modelP->motId[motNo]);
    } else {
        fn_8005BE80(modelP->charNo, modelP->motId[motNo]);
    }
    fn_8002D86C(modelP->modelId, 1.0f);
    modelP->motStart = 0.0f;
    if (modelP->charNo == -1) {
        modelP->motEnd = fn_8002D530(modelP->modelId);
    } else {
        modelP->motEnd = fn_8005C1FC(modelP->charNo);
    }
    if (attr & (((1 << 30) | (1 << 2)) | ((1 << 30) | (1 << 5)))) {
        if (modelP->charNo == -1) {
            fn_8002D414(modelP->modelId, modelP->motEnd);
        } else {
            fn_8005C16C(modelP->charNo, modelP->motEnd);
        }
    }
    fn_80026A44(modelP->modelId,
        (1 << 30) | ((1 << 30) | (1 << 0)) | ((1 << 30) | (1 << 1)));
    fn_800269E8(modelP->modelId, attr);
    modelP->motNo = motNo;
}

int mbObjMotionIDCurGet(MBMODELID modelId)
{
    return mbObjMotionIDGet(modelId, mbObjMotionGet(modelId));
}

/* Board features retrieve the engine motion ID stored in a specific slot. */
int mbObjMotionIDGet(MBMODELID modelId, int motNo)
{
    MBOBJMODEL *modelP = &objManData[modelId];

    return modelP->motId[motNo];
}

/* Board features start a blend with supplied start/end. Shared motStart is reset to zero and
 * motEnd to the current main-motion maximum before the call, then motNo is recorded. */
void mbObjMotionShiftSet(MBMODELID modelId, int motNo, float start, float end, u32 attr)
{
    MBOBJMODEL *modelP = &objManData[modelId];

    modelP->motStart = 0.0f;
    if (modelP->charNo == -1) {
        modelP->motEnd = fn_8002D530(modelP->modelId);
    } else {
        modelP->motEnd = fn_8005C1FC(modelP->charNo);
    }
    if (modelP->charNo == -1) {
        fn_8002CCC0(modelP->modelId, modelP->motId[motNo], start, end, attr);
    } else {
        fn_8005C2C8(modelP->charNo, modelP->motId[motNo], start, end, attr);
    }
    modelP->motNo = motNo;
}

/* Board features query the engine ID of the model's motion currently being blended. */
int mbObjMotionShiftIDGet(MBMODELID modelId)
{
    MBOBJMODEL *modelP = &objManData[modelId];

    return fn_8002D3EC(modelP->modelId);
}

/* Board features load a motion into a specified slot. Character data closes immediately and
 * its data number is not recorded; motNum increments even on failure. */
void mbObjMotionNoCreate(MBMODELID objectOrMotionId, int dataNum, int motNo)
{
    MBOBJMODEL *modelP = &objManData[objectOrMotionId];
    int readDataNum;

    /* The input object-table ID is reused below to hold the created motion ID. */
    if (modelP->charNo == -1) {
        if (!lbl_80265B70.curTime) {
            if (DIRNUM(dataNum) == dataDirNight) {
                readDataNum = dataDirDay | FILENUM(dataNum);
                goto read_data;
            }
        } else if (DIRNUM(dataNum) == dataDirDay) {
            readDataNum = dataDirNight | FILENUM(dataNum);
            goto read_data;
        }
        readDataNum = dataNum;
read_data:
        modelP->motData[motNo] = fn_80008DD0(readDataNum, 0x10000000, 2);
        objectOrMotionId = fn_80030458(modelP->modelId, modelP->motData[motNo]);
    } else {
        objectOrMotionId = fn_8005B0B8(modelP->charNo, dataNum);
        fn_8005B4BC(modelP->charNo);
    }
    modelP->motId[motNo] = objectOrMotionId;
    modelP->motNum++;
}

/* Board features load a motion into the first free nonzero slot. Character data numbers are
 * not retained; a full table falls through to an out-of-bounds slot instead of failing. */
int mbObjMotionCreate(MBMODELID objectOrMotionId, int dataNum)
{
    MBOBJMODEL *modelP = &objManData[objectOrMotionId];
    int motNo;
    int readDataNum;

    for (motNo = 1; motNo < MB_OBJ_MOT_MAX; motNo++) {
        if (modelP->motId[motNo] == -1) {
            break;
        }
    }
    /* The input object-table ID is reused below to hold the created motion ID. */
    if (modelP->charNo == -1) {
        if (!lbl_80265B70.curTime) {
            if (DIRNUM(dataNum) == dataDirNight) {
                readDataNum = dataDirDay | FILENUM(dataNum);
                goto read_data;
            }
        } else if (DIRNUM(dataNum) == dataDirDay) {
            readDataNum = dataDirNight | FILENUM(dataNum);
            goto read_data;
        }
        readDataNum = dataNum;
read_data:
        modelP->motData[motNo] = fn_80008DD0(readDataNum, 0x10000000, 2);
        objectOrMotionId = fn_80030458(modelP->modelId, modelP->motData[motNo]);
    } else {
        objectOrMotionId = fn_8005B0B8(modelP->charNo, dataNum);
        fn_8005B4BC(modelP->charNo);
    }
    modelP->motId[motNo] = objectOrMotionId;
    modelP->motNum++;
    return motNo;
}

/* Board features release one motion slot, selecting the base motion first if it was active. */
void mbObjMotionKill(MBMODELID modelId, int motNo)
{
    MBOBJMODEL *modelP = &objManData[modelId];

    if (motNo == modelP->motNo && modelP->motNo != 0) {
        mbObjMotionSet(modelId, 0, 0);
    }
    if (modelP->motId[motNo] >= 0) {
        if (modelP->charNo == -1) {
            fn_8002C624(modelP->motId[motNo]);
        } else {
            fn_8005B420(modelP->charNo, modelP->motId[motNo]);
        }
        modelP->motId[motNo] = -1;
    }
    modelP->motData[motNo] = NULL;
    modelP->motNum--;
}

/* Board features query the current playback frame of the model's active motion. */
float mbObjMotionTimeGet(MBMODELID modelId)
{
    MBOBJMODEL *modelP = &objManData[modelId];
    float time;

    if (modelP->charNo == -1) {
        time = lbl_802C01C8[modelP->modelId].motWork.time;
    } else {
        time = fn_8005C1B8(modelP->charNo);
    }
    return time;
}

/* Board features seek the active motion to the requested frame; the engine clamps time to
 * [0, max]. */
void mbObjMotionTimeSet(MBMODELID modelId, float time)
{
    MBOBJMODEL *modelP = &objManData[modelId];

    if (modelP->charNo == -1) {
        fn_8002D414(modelP->modelId, time);
    } else {
        fn_8005C16C(modelP->charNo, time);
    }
}

/* Board features query the final frame of the active motion. */
float mbObjMotionMaxTimeGet(MBMODELID modelId)
{
    MBOBJMODEL *modelP = &objManData[modelId];
    float time;

    if (modelP->charNo == -1) {
        time = fn_8002D530(modelP->modelId);
    } else {
        time = fn_8005C1FC(modelP->charNo);
    }
    return time;
}

/* Board features query the playback speed stored by the engine for this model. */
float mbObjMotionSpeedGet(MBMODELID modelId)
{
    HU3D_MODEL *model3DP;
    MBOBJMODEL *modelP = &objManData[modelId];

    model3DP = &lbl_802C01C8[modelP->modelId];

    return model3DP->motWork.speed;
}

/* Board features set motion playback speed and the speed used by motion blending. */
void mbObjMotionSpeedSet(MBMODELID modelId, float speed)
{
    MBOBJMODEL *modelP = &objManData[modelId];
    HU3D_MODEL *model3DP;

    if (modelP->charNo == -1) {
        fn_8002D86C(modelP->modelId, speed);
    } else {
        fn_8005C390(modelP->charNo, speed);
    }
    model3DP = &lbl_802C01C8[modelP->modelId];
    model3DP->motShiftWork.speed = speed;
}

/* Board features poll whether the active motion has reached its end. */
BOOL mbObjMotionEndCheck(MBMODELID modelId)
{
    MBOBJMODEL *modelP = &objManData[modelId];
    BOOL result;

    if (modelP->charNo == -1) {
        result = fn_8002D75C(modelP->modelId);
    } else {
        result = fn_8005C240(modelP->charNo);
    }
    return result;
}

/* Board features update nonnegative playback bounds and pass the stored frame range to the
 * engine. */
void mbObjMotionStartEndSet(MBMODELID modelId, s16 start, s16 end)
{
    MBOBJMODEL *modelP = &objManData[modelId];

    if (start >= 0) {
        modelP->motStart = start;
    }
    if (end >= 0) {
        modelP->motEnd = end;
    }
    fn_8002D730(modelP->modelId, modelP->motStart, modelP->motEnd);
}

/* Board features enable or disable looping for the model's motion playback. */
void mbObjMotionLoopSet(MBMODELID modelId, BOOL loopF)
{
    MBOBJMODEL *modelP = &objManData[modelId];

    if (loopF) {
        fn_800269E8(modelP->modelId, ((1 << 30) | (1 << 0)));
    } else {
        fn_80026A44(modelP->modelId, ((1 << 30) | (1 << 0)));
    }
}

/* Board features query the current frame of the model's shape animation. */
float mbObjMotionShapeTimeGet(MBMODELID modelId)
{
    MBOBJMODEL *modelP = &objManData[modelId];
    HU3D_MODEL *model3DP = &lbl_802C01C8[modelP->modelId];

    return model3DP->motShapeWork.time;
}

/* Board features select a shape motion and reset speed/range. Reverse starts at the main-motion
 * maximum; attribute bits are only added, so old shape flags persist. */
void mbObjMotionShapeSet(MBMODELID modelId, int motNo, u32 attr)
{
    MBOBJMODEL *modelP = &objManData[modelId];
    HU3D_MODEL *model3DP = &lbl_802C01C8[modelP->modelId];

    model3DP->motShapeWork.speed = 1.0f;
    fn_8002CEB0(modelP->modelId, modelP->motId[motNo]);
    if (attr & ((1 << 30) | (1 << 8))) {
        model3DP->motShapeWork.time = fn_8002D530(modelP->modelId);
    }
    fn_800269E8(modelP->modelId, attr);
}

/* Board event callbacks such as fn_1_8860 write the shape frame directly; no range clamp or
 * endpoint update is performed. */
void mbObjMotionShapeTimeSet(MBMODELID modelId, float time)
{
    MBOBJMODEL *modelP = &objManData[modelId];
    HU3D_MODEL *model3DP = &lbl_802C01C8[modelP->modelId];

    model3DP->motShapeWork.time = time;
}

/* Shape-end checks use this to read the configured last frame of the model's shape motion. */
float mbObjMotionShapeMaxTimeGet(MBMODELID modelId)
{
    MBOBJMODEL *modelP = &objManData[modelId];
    HU3D_MODEL *model3DP = &lbl_802C01C8[modelP->modelId];

    model3DP = &lbl_802C01C8[modelP->modelId];
    return model3DP->motShapeWork.end;
}

/* Board/event code sets shape playback speed, including zero to pause and negative values to
 * reverse. */
void mbObjMotionShapeSpeedSet(MBMODELID modelId, float speed)
{
    MBOBJMODEL *modelP = &objManData[modelId];
    HU3D_MODEL *model3DP = &lbl_802C01C8[modelP->modelId];

    model3DP->motShapeWork.speed = speed;
}

/* Board event code queries the playback rate when it needs the current shape-animation speed. */
float mbObjMotionShapeSpeedGet(MBMODELID modelId)
{
    HU3D_MODEL *model3DP;
    MBOBJMODEL *modelP = &objManData[modelId];

    model3DP = &lbl_802C01C8[modelP->modelId];

    return model3DP->motShapeWork.speed;
}

/* Board event code checks shape time against its configured end. This attr test does not see
 * shape flags stored in motAttr, so reverse playback uses the forward end test. */
BOOL mbObjMotionShapeEndCheck(MBMODELID modelId)
{
    MBOBJMODEL *modelP = &objManData[modelId];
    HU3D_MODEL *model3DP = &lbl_802C01C8[modelP->modelId];

    if (model3DP->attr & ((1 << 30) | (1 << 8))) {
        return model3DP->motShapeWork.time <= 0.0f;
    }
    return mbObjMotionShapeMaxTimeGet(modelId) <= model3DP->motShapeWork.time;
}

/* Board model setup sets shape range from shared motStart/motEnd; negative inputs retain those
 * shared fields, which main-motion operations can change. */
void mbObjMotionShapeStartEndSet(MBMODELID modelId, s16 start, s16 end)
{
    MBOBJMODEL *modelP = &objManData[modelId];

    if (start >= 0) {
        modelP->motStart = start;
    }
    if (end >= 0) {
        modelP->motEnd = end;
    }
    fn_8002D024(modelP->modelId, modelP->motStart, modelP->motEnd);
}

/* Board and minigame model setup call this to enable the model's cast shadows. */
void mbObjShadowSet(MBMODELID modelId)
{
    MBOBJMODEL *modelP = &objManData[modelId];

    fn_80026E44(modelP->modelId);
}

/* Board and minigame cleanup call this to disable the model's cast shadows. */
void mbObjShadowReset(MBMODELID modelId)
{
    MBOBJMODEL *modelP = &objManData[modelId];

    fn_80026EF0(modelP->modelId);
}

/* Board and minigame model setup call this to include the model in shadow-map rendering. */
void mbObjShadowMapSet(MBMODELID modelId)
{
    MBOBJMODEL *modelP = &objManData[modelId];

    fn_80026FFC(modelP->modelId);
}

/* Board and minigame cleanup call this to remove the model from shadow-map rendering. */
void mbObjShadowMapReset(MBMODELID modelId)
{
    MBOBJMODEL *modelP = &objManData[modelId];

    fn_8002736C(modelP->modelId);
}

int mbObjStub0(void)
{
    return 0;
}
