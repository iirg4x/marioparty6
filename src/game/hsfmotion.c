/* Motion playback, blending, and animation-track evaluation for HSF models. */
#include "dolphin/math.h"
#include "game/hu3d.h"
#include "game/hsfload.h"
#include "game/init.h"
#include "game/main.h"
#include "stddef.h"

#include "game/ClusterExec.h"
#include "game/EnvelopeExec.h"
#include "game/ShapeExec.h"

#define HU3D_MOTATTR_SHIFT_ALL                                                                     \
    (((1 << 30) | (1 << 3)) | ((1 << 30) | (1 << 4)) | ((1 << 30) | (1 << 5)))
#define HU3D_MOTATTR_NOSHIFT_ALL \
    (((1 << 30) | (1 << 0))|((1 << 30) | (1 << 1))|((1 << 30) | (1 << 2)))
#define HU3D_MOTATTR_ALL (HU3D_MOTATTR_SHIFT_ALL|HU3D_MOTATTR_NOSHIFT_ALL)
#define texMtxAnimEnabled textureEnable

HU3D_MOTION lbl_802624B8[384];

static HSF_BITMAP *bitMapPtr;

/* Sets a pointer-sized field to NULL at each HU3D_MOTION-sized offset from Hu3DData. */
void fn_8002C47C(void)
{
    HU3D_MOTION *motP;
    s16 i;
    for(motP = (HU3D_MOTION *)lbl_802C01C8, i=0; i<384; i++, motP++) {
        motP->hsf = NULL;
    }
}

/* Creates a motion slot from an independently loaded HSF; callers later attach it to a model. */
HU3D_MOTIONID fn_8002C4C0(void *data)
{
    HU3D_MOTION *motP;
    HU3D_MOTIONID motId;
    for(motP = lbl_802624B8, motId=0; motId<384; motId++, motP++) {
        if(!motP->hsf) {
            break;
        }
    }
    if(motId == 384) {
        OSReport("Error: Create Motion Over!\n");
        return -1;
    }
    motP->hsf = LoadHSF(data);
    motP->attr = 0;
    motP->modelId = -1;
    return motId;
}

/* Creates a motion slot that shares the selected model's HSF data. */
HU3D_MOTIONID fn_8002C564(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HU3D_MOTION *motP;
    HU3D_MOTIONID motId;
    for(motP = lbl_802624B8, motId=0; motId<384; motId++, motP++) {
        if(!motP->hsf) {
            break;
        }
    }
    if(motId == 384) {
        OSReport("Error: Create Motion Over!\n");
        return -1;
    }
    motP->hsf = modelP->hsf;
    motP->attr = 0;
    motP->modelId = modelId;
    modelP->motIdSrc = motId;
    return motId;
}

/* Releases a motion slot if no other model uses it; its source model is ignored by this check. */
BOOL fn_8002C624(HU3D_MOTIONID motId)
{
    HU3D_MOTION *motP = &lbl_802624B8[motId];
    HU3D_MODEL *modelP;
    s16 i;
    if(!motP->hsf) {
        return FALSE;
    }
    for(modelP=&lbl_802C01C8[0], i=0; i<512; i++, modelP++) {
        if(!modelP->hsf || motP->modelId == i) {
            continue;
        }
        if(modelP->motId == motId ||
            modelP->motIdShape == motId ||
            modelP->motIdOvl == motId ||
            modelP->motIdShift == motId ||
            modelP->motIdCluster[0] == motId ||
            modelP->motIdCluster[1] == motId ||
            modelP->motIdCluster[2] == motId ||
            modelP->motIdCluster[3] == motId) {
                break;
            }
    }
    if(i != 512) {
        return FALSE;
    }
    if(motP->modelId == -1) {
        HuMemDirectFree(motP->hsf);
    } else {
        lbl_802C01C8[motP->modelId].motIdSrc = -1;
    }
    motP->hsf = NULL;
    return TRUE;
}

/* Called during HSF manager shutdown to release every motion slot that can be killed. */
void fn_8002C7BC(void)
{
    HU3D_MOTION *motP;
    HU3D_MOTIONID motId;
    for(motP = lbl_802624B8, motId=0; motId<384; motId++, motP++) {
        if(motP->hsf) {
            fn_8002C624(motId);
        }
    }
}

/* Selects the main motion; NONE clears its motion assignments and resets main speed/time unless
* the current main slot is the model's source slot. */
void fn_8002C95C(HU3D_MODELID modelId, HU3D_MOTIONID motId)
{
    if(motId == -1) {
        HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
        s16 i;
        if(modelP->motId == modelP->motIdSrc) {
            return;
        }
        modelP->motWork.speed = 1;
        for(i=0; i<4; i++) {
            modelP->motIdCluster[i] = -1;
        }
        modelP->motIdOvl = -1;
        modelP->motIdShift = -1;
        modelP->motIdShape = -1;
        modelP->motWork.time = 0;
        modelP->motId = -1;
    } else {
        lbl_802C01C8[modelId].motIdShift = -1;
        lbl_802C01C8[modelId].motId = motId;
        lbl_802C01C8[modelId].motWork.time = 0;
        lbl_802C01C8[modelId].motWork.start = 0;
        lbl_802C01C8[modelId].motWork.end = fn_8002D530(modelId);
    }
}

/* Starts an overlay motion on a model at time zero and normal speed. */
void fn_8002CAA8(HU3D_MODELID modelId, HU3D_MOTIONID motId)
{
    lbl_802C01C8[modelId].motIdOvl = motId;
    lbl_802C01C8[modelId].motOvlWork.time = 0;
    lbl_802C01C8[modelId].motOvlWork.speed = 1;
}

/* Detaches the model's overlay motion. */
void fn_8002CAF0(HU3D_MODELID modelId)
{
    lbl_802C01C8[modelId].motIdOvl = -1;
}

/* Returns the current playback time of the model's overlay motion. */
float fn_8002CB0C(HU3D_MODELID modelId)
{
    return lbl_802C01C8[modelId].motOvlWork.time;
}

/* Returns the duration of the motion currently assigned as the overlay. */
float fn_8002CB24(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    return Hu3DMotionMotionMaxTimeGet(modelP->motIdOvl);
}

/* Sets overlay playback time and marks the model for motion evaluation. */
void fn_8002CB64(HU3D_MODELID modelId, float time)
{
    lbl_802C01C8[modelId].attr |= (1 << 25);
    lbl_802C01C8[modelId].motOvlWork.time = time;
}

/* Sets the per-frame playback multiplier for the model's overlay motion. */
void fn_8002CB98(HU3D_MODELID modelId, float speed)
{
    lbl_802C01C8[modelId].motOvlWork.speed = speed;
}

/* Checks the overlay endpoint selected by its reverse-playback flag. */
BOOL fn_8002CBB0(HU3D_MODELID modelId)
{
    if(!(lbl_802C01C8[modelId].motAttr  & ((1 << 30) | (1 << 11)))) {
        return fn_8002CB24(modelId) <= fn_8002CB0C(modelId)+0.0001;
    } else {
        return fn_8002CB0C(modelId) <= 0.0001f;
    }
}

/* Begins a transition to motId; an existing pending motion first becomes the main motion.
 * On that replacement, attr's shift bits set the main flags. The new motion starts at start
 * with speed 1; end is the blend duration. */
void fn_8002CCC0(HU3D_MODELID modelId, HU3D_MOTIONID motId, float start, float end, u32 attr)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HU3D_MOTION *motP = &lbl_802624B8[motId];
    u32 motAttr;
    attr &= ~(1 << 30);
    motAttr = 0;
    if(modelP->motIdShift != -1) {
        modelP->motId = modelP->motIdShift;
        modelP->motWork.time = modelP->motShiftWork.time;
        modelP->motWork.speed = modelP->motShiftWork.speed;
        modelP->motWork.start = modelP->motShiftWork.start;
        modelP->motWork.end = modelP->motShiftWork.end;
        if(attr & ((1 << 30) | (1 << 3))) {
            motAttr |= ((1 << 30) | (1 << 0));
        }
        if(attr & ((1 << 30) | (1 << 4))) {
            motAttr |= ((1 << 30) | (1 << 1));
        }
        if(attr & ((1 << 30) | (1 << 5))) {
            motAttr |= ((1 << 30) | (1 << 2));
        }
        modelP->motAttr &= ~HU3D_MOTATTR_ALL;
        modelP->motAttr |= motAttr;
        modelP->motAttr &= ~(1 << 30);
    } else {
        modelP->motAttr &= ~HU3D_MOTATTR_SHIFT_ALL;
    }
    modelP->motIdShift = motId;
    modelP->motShiftWork.time = start;
    modelP->motShiftWork.speed = 1;
    modelP->motOvlWork.start = 0;
    modelP->motOvlWork.end = end;
    modelP->motShiftWork.start = 0;
    modelP->motShiftWork.end = fn_8002D5D4(modelId);
    if(attr & ((1 << 30) | (1 << 0))) {
        motAttr |= ((1 << 30) | (1 << 3));
    }
    if(attr & ((1 << 30) | (1 << 1))) {
        motAttr |= ((1 << 30) | (1 << 4));
    }
    if(attr & ((1 << 30) | (1 << 2))) {
        motAttr |= ((1 << 30) | (1 << 5));
    }
    attr &= ~HU3D_MOTATTR_NOSHIFT_ALL;
    modelP->motAttr |= motAttr|attr;
    modelP->motAttr &= ~(1 << 30);
}

/* Assigns a shape animation and initializes its playback range and speed. */
void fn_8002CEB0(HU3D_MODELID modelId, HU3D_MOTIONID motId)
{
    lbl_802C01C8[modelId].motIdShape = motId;
    lbl_802C01C8[modelId].motShapeWork.time = 0;
    lbl_802C01C8[modelId].motShapeWork.speed = 1;
    lbl_802C01C8[modelId].motShapeWork.start = 0;
    lbl_802C01C8[modelId].motShapeWork.end = fn_8002CFE4(modelId);
}

/* Returns the motion slot assigned to the model's shape animation. */
HU3D_MOTIONID fn_8002CF58(HU3D_MODELID modelId)
{
    return lbl_802C01C8[modelId].motIdShape;
}

/* Sets the playback multiplier for the model's shape animation. */
void fn_8002CF70(HU3D_MODELID modelId, float speed)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->motShapeWork.speed = speed;
}

/* Sets shape-animation time and marks the model for motion evaluation. */
void fn_8002CF98(HU3D_MODELID modelId, float time)
{
    lbl_802C01C8[modelId].attr |= (1 << 25);
    lbl_802C01C8[modelId].motShapeWork.time = time;
}

/* Returns the current time of the model's shape animation. */
float fn_8002CFCC(HU3D_MODELID modelId)
{
    return lbl_802C01C8[modelId].motShapeWork.time;
}

/* Returns the duration of the motion assigned to the model's shape animation. */
float fn_8002CFE4(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    return Hu3DMotionMotionMaxTimeGet(modelP->motIdShape);
}

/* Sets shape playback bounds; looping wraps at or beyond end and only below start. */
void fn_8002D024(HU3D_MODELID modelId, float start, float end)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->motShapeWork.start = start;
    modelP->motShapeWork.end = end;
}

/* Assigns a cluster animation to the first free cluster slot and returns that slot. */
s16 fn_8002D050(HU3D_MODELID modelId, HU3D_MOTIONID motId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    s16 i;
    for(i=0; i<4; i++) {
        if(modelP->motIdCluster[i] == -1) {
            modelP->motIdCluster[i] = motId;
            modelP->clusterTime[i] = 0;
            modelP->clusterSpeed[i] = 1;
            modelP->clusterAttr[i] = 0;
            modelP->attr |= (1 << 10);
            ClusterAdjustObject(modelP->hsf, lbl_802624B8[motId].hsf);
            return i;
        }
    }
    OSReport("Error: Cluster Entry Over\n");
    return -1;
}

/* Replaces the motion in a cluster slot, resetting its time and speed but retaining its cluster
 * attributes. */
s16 fn_8002D160(HU3D_MODELID modelId, HU3D_MOTIONID motId, s16 clusterNo)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    fn_8002D248(modelId, clusterNo);
    modelP->motIdCluster[clusterNo] = motId;
    modelP->clusterTime[clusterNo] = 0;
    modelP->clusterSpeed[clusterNo] = 1;
    modelP->attr |= (1 << 10);
    ClusterAdjustObject(modelP->hsf, lbl_802624B8[motId].hsf);
    return clusterNo;
}

/* Removes the model's shape-animation assignment. */
void fn_8002D22C(HU3D_MODELID modelId)
{
    lbl_802C01C8[modelId].motIdShape = -1;
}

/* Removes cluster motion assignments; per-slot time, speed, and attributes remain unchanged. */
void fn_8002D248(HU3D_MODELID modelId, s16 clusterNo)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    s16 i;
    if(clusterNo == -1) {
        for(i=0; i<4; i++) {
            modelP->motIdCluster[i] = -1;
        }
        modelP->attr &= ~(1 << 10);
    } else {
        modelP->motIdCluster[clusterNo] = -1;
        for(i=0; i<4; i++) {
            if(modelP->motIdCluster[i] != -1) {
                return;
            }
        }
        modelP->attr &= ~(1 << 10);
    }
}

/* Sets the playback multiplier for one cluster animation. */
void fn_8002D30C(HU3D_MODELID modelId, s16 clusterNo, float speed)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->clusterSpeed[clusterNo] = speed;
}

/* Sets the current playback time for one cluster animation. */
void fn_8002D340(HU3D_MODELID modelId, s16 clusterNo, float time)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->clusterTime[clusterNo] = time;
}

/* Returns the duration of the motion assigned to one cluster slot. */
float fn_8002D374(HU3D_MODELID modelId, s16 clusterNo)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    return Hu3DMotionMotionMaxTimeGet(modelP->motIdCluster[clusterNo]);
}

/* Returns the model's current main-motion slot. */
HU3D_MOTIONID fn_8002D3C4(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    return modelP->motId;
}

/* Returns the model's pending transition-motion slot. */
HU3D_MOTIONID fn_8002D3EC(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    return modelP->motIdShift;
}

/* Sets main-motion time, clamped to zero and the motion's maximum time. */
void fn_8002D414(HU3D_MODELID modelId, float time)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    if(fn_8002D530(modelId) <= time) {
        time = fn_8002D530(modelId);
    }
    if(time < 0) {
        time = 0;
    }
    modelP->motWork.time = time;
    /* Camera-only records carry -1 instead of HSF data. Paused models with envelopes
    * request deformation work after their playback time changes. */
    if (modelP->hsf != ((void *) -1) && modelP->hsf->cenvNum != 0 &&
        (modelP->motAttr & ((1 << 30) | (1 << 1)))) {
        modelP->attr |= (1 << 25);
    }
}

/* Returns the current main-motion playback time. */
float fn_8002D4E0(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    return modelP->motWork.time;
}

/* Returns the current time of the motion being blended in. */
float fn_8002D508(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    return modelP->motShiftWork.time;
}

/* Returns the main motion's duration after converting maxTime + 0.0001 through s16;
 * returns zero if no main motion is set. */
float fn_8002D530(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HU3D_MOTION *motP;
    HSF_MOTION *hsfMotP;
    s16 maxTime;
    if(modelP->motId == -1) {
        return 0;
    }
    motP = &lbl_802624B8[modelP->motId];
    hsfMotP = motP->hsf->motion;
    maxTime = hsfMotP->maxTime+0.0001;
    return maxTime;
}

/* Returns the pending motion's duration after converting maxTime + 0.0001 through s16;
 * returns zero when none is assigned. */
float fn_8002D5D4(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HU3D_MOTION *motP;
    HSF_MOTION *hsfMotP;
    s16 maxTime;
    if(modelP->motIdShift == -1) {
        return 0;
    }
    motP = &lbl_802624B8[modelP->motIdShift];
    hsfMotP = motP->hsf->motion;
    maxTime = hsfMotP->maxTime+0.0001;
    return maxTime;
}

/* Sets the playback range for the motion being blended in. */
void fn_8002D678(HU3D_MODELID modelId, float start, float end)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->motShiftWork.start = start;
    modelP->motShiftWork.end = end;
}

/* Returns a motion slot's duration after converting maxTime + 0.0001 through s16;
 * returns zero for an empty slot. */
float Hu3DMotionMotionMaxTimeGet(HU3D_MOTIONID motId)
{
    HU3D_MOTION *motP = &lbl_802624B8[motId];
    HSF_MOTION *hsfMotP;
    s16 maxTime;
    if(!motP->hsf) {
        return 0;
    }
    hsfMotP = motP->hsf->motion;
    maxTime = hsfMotP->maxTime+0.0001;
    return maxTime;
}

/* Sets the playback range for the model's main motion. */
void fn_8002D730(HU3D_MODELID modelId, float start, float end)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->motWork.start = start;
    modelP->motWork.end = end;
}

/* Checks the main-motion endpoint according to the model's reverse flag. */
BOOL fn_8002D75C(HU3D_MODELID modelId)
{
    if(!(lbl_802C01C8[modelId].motAttr & ((1 << 30) | (1 << 2)))) {
        return lbl_802C01C8[modelId].motWork.end <= fn_8002D4E0(modelId)+0.0001f;
    } else {
        return fn_8002D4E0(modelId) <= lbl_802C01C8[modelId].motWork.start+0.0001f;
    }
}

/* Sets the per-frame playback multiplier for the model's main motion. */
void fn_8002D86C(HU3D_MODELID modelId, float speed)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->motWork.speed = speed;
}

/* Sets the per-frame playback multiplier for the transition motion. */
void fn_8002D894(HU3D_MODELID modelId, float speed)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->motShiftWork.speed = speed;
}

/* Sets requested current transform components to their base values and blocks track writes; a
 * base-transform reset can still overwrite forced scales. */
void fn_8002D8BC(HU3D_MODELID modelId, char *objName, u32 forceAttr)
{
    HSF_OBJECT *objP = fn_80026BF4(modelId, objName);
    HSF_CONSTDATA *constData;
    if(!objP->constData) {
        constData = fn_800215B4(objP, lbl_802C01C8[modelId].mallocNo);
    } else {
        constData = objP->constData;
    }
    constData->attr |= forceAttr;
    if(forceAttr & (1 << 4)) {
        objP->mesh.curr.pos.x = objP->mesh.base.pos.x;
    }
    if(forceAttr & (1 << 5)) {
        objP->mesh.curr.pos.y = objP->mesh.base.pos.y;
    }
    if(forceAttr & (1 << 6)) {
        objP->mesh.curr.pos.z = objP->mesh.base.pos.z;
    }
    if(forceAttr & (1 << 7)) {
        objP->mesh.curr.rot.x = objP->mesh.base.rot.x;
    }
    if(forceAttr & (1 << 8)) {
        objP->mesh.curr.rot.y = objP->mesh.base.rot.y;
    }
    if(forceAttr & (1 << 9)) {
        objP->mesh.curr.rot.z = objP->mesh.base.rot.z;
    }
    if(forceAttr & (1 << 20)) {
        objP->mesh.curr.scale.x = objP->mesh.base.scale.x;
    }
    if(forceAttr & (1 << 21)) {
        objP->mesh.curr.scale.y = objP->mesh.base.scale.y;
    }
    if(forceAttr & (1 << 22)) {
        objP->mesh.curr.scale.z = objP->mesh.base.scale.z;
    }
}

/* Releases the requested forced transform components on the selected object. */
void Hu3DMotionNoMotReset(HU3D_MODELID modelId, char *objName, u32 forceAttr)
{
    HSF_OBJECT *objP = fn_80026BF4(modelId, objName);
    HSF_CONSTDATA *constData = objP->constData;
    constData->attr &= ~forceAttr;
}

/* Sets selected current transform components to value and blocks their animation-track writes.
 * A base-transform reset can still overwrite forced scale components. */
void fn_8002DA64(HU3D_MODELID modelId, char *objName, u32 forceAttr, float value)
{
    HSF_OBJECT *objP = fn_80026BF4(modelId, objName);
    HSF_CONSTDATA *constData;
    if(!objP->constData) {
        constData = fn_800215B4(objP, lbl_802C01C8[modelId].mallocNo);
    } else {
        constData = objP->constData;
    }
    constData->attr |= forceAttr;
    if(forceAttr & (1 << 4)) {
        objP->mesh.curr.pos.x = value;
    }
    if(forceAttr & (1 << 5)) {
        objP->mesh.curr.pos.y = value;
    }
    if(forceAttr & (1 << 6)) {
        objP->mesh.curr.pos.z = value;
    }
    if(forceAttr & (1 << 7)) {
        objP->mesh.curr.rot.x = value;
    }
    if(forceAttr & (1 << 8)) {
        objP->mesh.curr.rot.y = value;
    }
    if(forceAttr & (1 << 9)) {
        objP->mesh.curr.rot.z = value;
    }
    if(forceAttr & (1 << 20)) {
        objP->mesh.curr.scale.x = value;
    }
    if(forceAttr & (1 << 21)) {
        objP->mesh.curr.scale.y = value;
    }
    if(forceAttr & (1 << 22)) {
        objP->mesh.curr.scale.z = value;
    }
}

/* Stores the callback and selects the first object whose name contains NOW; if none exists,
 * timingHookObj is left unchanged. */
void fn_8002DB98(HU3D_MODELID modelId, HU3D_TIMING_HOOK timingHook)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HSF_DATA *hsf = modelP->hsf;
    s16 i;
    HSF_OBJECT *objPtr;

    modelP->timingHook = timingHook;
    for(objPtr=hsf->object, i=0; i<hsf->objectNum; objPtr++, i++) {
        HSF_OBJECT *obj = objPtr;
        char *name;
        for(name = obj->name; *name; name++) {
            if(name[0] == 'N' && name[1] == 'O' && name[2] == 'W') {
                modelP->timingHookObj = obj;
                return;
            }
        }
    }
}

/* Clears the callback used for the model's NOW timing track. */
void Hu3DMotionTimingHookReset(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->timingHook = NULL;
}

/* Adds motion-slot attributes used by playback and motion evaluation. */
void fn_8002DC74(HU3D_MOTIONID motId, u16 attr)
{
    HU3D_MOTION *motP = &lbl_802624B8[motId];
    motP->attr |= attr;
}

/* Removes selected attributes from a motion slot. */
void fn_8002DCA8(HU3D_MOTIONID motId, u16 attr)
{
    HU3D_MOTION *motP = &lbl_802624B8[motId];
    motP->attr &= ~attr;
}

/* Called from the HSF manager's per-frame update to advance active motion clocks for one model. */
void fn_8002DCE0(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HU3D_MOTION *motP = &lbl_802624B8[modelP->motId];
    u32 attr = modelP->motAttr;
    HSF_MOTION *hsfMotP;

    if(modelP->motId != -1) {
        hsfMotP = motP->hsf->motion;
        motP = &lbl_802624B8[modelP->motId];
        if(!(attr & ((1 << 30) | (1 << 1)))) {
            modelP->attr |= (1 << 25);
            if(!(attr & ((1 << 30) | (1 << 2)))) {
                modelP->motWork.time += modelP->motWork.speed*minimumVcountf;
            } else {
                modelP->motWork.time -= modelP->motWork.speed*minimumVcountf;
            }
            /* Main-motion loops preserve overshoot with one wrap. Overlay, shift, shape,
            * and cluster loops instead snap to the opposite endpoint. */
            if(attr & ((1 << 30) | (1 << 0))) {
                if(modelP->motWork.time < modelP->motWork.start) {
                    modelP->motWork.time =
                        modelP->motWork.end - (modelP->motWork.start - modelP->motWork.time);
                } else if(modelP->motWork.time >= modelP->motWork.end) {
                    modelP->motWork.time =
                        modelP->motWork.start + (modelP->motWork.time - modelP->motWork.end);
                }
            } else if(modelP->motWork.time < modelP->motWork.start) {
                modelP->motWork.time = modelP->motWork.start;
            } else if(modelP->motWork.time >= modelP->motWork.end) {
                modelP->motWork.time = modelP->motWork.end;
            }
        }
    }
    if(modelP->motIdOvl != -1) {
        motP = &lbl_802624B8[modelP->motIdOvl];
        hsfMotP = motP->hsf->motion;
        if(!(attr & ((1 << 30) | (1 << 10)))) {
            modelP->attr |= (1 << 25);
            if(!(attr & ((1 << 30) | (1 << 11)))) {
                modelP->motOvlWork.time += modelP->motOvlWork.speed*minimumVcountf;
            } else {
                modelP->motOvlWork.time -= modelP->motOvlWork.speed*minimumVcountf;
            }
            if(attr & ((1 << 30) | (1 << 9))) {
                if(modelP->motOvlWork.time < 0.0f) {
                    modelP->motOvlWork.time = hsfMotP->maxTime;
                } else if(modelP->motOvlWork.time >= hsfMotP->maxTime) {
                    modelP->motOvlWork.time = 0;
                }
            } else if(modelP->motOvlWork.time < 0.0f) {
                modelP->motOvlWork.time = 0.0f;
            } else if(modelP->motOvlWork.time >= hsfMotP->maxTime) {
                modelP->motOvlWork.time = hsfMotP->maxTime;
            }
        }
    }
    if(modelP->motIdShift != -1) {
        modelP->attr |= (1 << 25);
        if(!(attr & ((1 << 30) | (1 << 12)))) {
            modelP->motOvlWork.start += minimumVcountf;
            if(modelP->motOvlWork.start >= modelP->motOvlWork.end) {
                modelP->motId = modelP->motIdShift;
                modelP->motWork.time = modelP->motShiftWork.time;
                modelP->motWork.speed = modelP->motShiftWork.speed;
                modelP->motWork.start = modelP->motShiftWork.start;
                modelP->motWork.end = modelP->motShiftWork.end;
                modelP->motIdShift = -1;
                /* Converted shift flags include the HU3D_MOTATTR tag, so any promoted flag makes
                * the shape-pause test below succeed. With none, shape advances forward without
                * looping, ignoring its own pause, reverse, and loop flags for this call. */
                attr = 0;
                if(modelP->motAttr & ((1 << 30) | (1 << 3))) {
                    attr |= ((1 << 30) | (1 << 0));
                }
                if(modelP->motAttr & ((1 << 30) | (1 << 4))) {
                    attr |= ((1 << 30) | (1 << 1));
                }
                if(modelP->motAttr & ((1 << 30) | (1 << 5))) {
                    attr |= ((1 << 30) | (1 << 2));
                }
                modelP->motAttr &= ~HU3D_MOTATTR_ALL;
                modelP->motAttr |= attr;
                modelP->motAttr &= ~(1 << 30);
                goto end_shift;
            }
        }
        if(!(modelP->motAttr & ((1 << 30) | (1 << 4)))) {
            motP = &lbl_802624B8[modelP->motIdShift];
            if(!(modelP->motAttr & ((1 << 30) | (1 << 5)))) {
                modelP->motShiftWork.time += modelP->motShiftWork.speed*minimumVcountf;
            } else {
                modelP->motShiftWork.time -= modelP->motShiftWork.speed*minimumVcountf;
            }
            if(modelP->motAttr & ((1 << 30) | (1 << 3))) {
                if(modelP->motShiftWork.time < modelP->motShiftWork.start) {
                    modelP->motShiftWork.time = modelP->motShiftWork.end;
                } else if(modelP->motShiftWork.time >= modelP->motShiftWork.end) {
                    modelP->motShiftWork.time = modelP->motShiftWork.start;
                }
            } else if(modelP->motShiftWork.time < modelP->motShiftWork.start) {
                modelP->motShiftWork.time = modelP->motShiftWork.start;
            } else if(modelP->motShiftWork.time >= modelP->motShiftWork.end) {
                modelP->motShiftWork.time = modelP->motShiftWork.end;
            }
        }
    }
    end_shift:
    if(modelP->motIdShape != -1 && !(attr & ((1 << 30) | (1 << 7)))) {
        modelP->attr |= (1 << 25);
        motP = &lbl_802624B8[modelP->motIdShape];
        hsfMotP = motP->hsf->motion;
        if(!(attr & ((1 << 30) | (1 << 8)))) {
            modelP->motShapeWork.time += modelP->motShapeWork.speed*minimumVcountf;
        } else {
            modelP->motShapeWork.time -= modelP->motShapeWork.speed*minimumVcountf;
        }
        if(attr & ((1 << 30) | (1 << 6))) {
            if(modelP->motShapeWork.time < modelP->motShapeWork.start) {
                modelP->motShapeWork.time = modelP->motShapeWork.end;
            } else if(modelP->motShapeWork.time >= modelP->motShapeWork.end) {
                modelP->motShapeWork.time = modelP->motShapeWork.start;
            }
        } else if(modelP->motShapeWork.time < modelP->motShapeWork.start) {
            modelP->motShapeWork.time = modelP->motShapeWork.start;
        } else if(modelP->motShapeWork.time >= modelP->motShapeWork.end) {
            modelP->motShapeWork.time = modelP->motShapeWork.end;
        }
    }
    if(modelP->attr & (1 << 10)) {
        s16 i;
        for(i=0; i<4; i++) {
            if(modelP->motIdCluster[i] == -1) {
                continue;
            }
            if(modelP->clusterAttr[i] & ((s32)((3U << 30) | (1U << 1)))) {
                continue;
            }
            modelP->attr |= (1 << 25);
            motP = &lbl_802624B8[modelP->motIdCluster[i]];
            hsfMotP = motP->hsf->motion;
            if(!(modelP->clusterAttr[i] & ((s32)((3U << 30) | (1U << 2))))) {
                modelP->clusterTime[i] += modelP->clusterSpeed[i]*minimumVcountf;
            } else {
                modelP->clusterTime[i] -= modelP->clusterSpeed[i]*minimumVcountf;
            }
            if(modelP->clusterAttr[i] & ((s32)((3U << 30) | (1U << 0)))) {
                if(modelP->clusterTime[i] < 0.0f) {
                    modelP->clusterTime[i] = hsfMotP->maxTime;
                } else if(modelP->clusterTime[i] >= hsfMotP->maxTime) {
                    modelP->clusterTime[i] = 0.0f;
                }
            } else if(modelP->clusterTime[i] < 0.0f) {
                modelP->clusterTime[i] = 0.0f;
            } else if(modelP->clusterTime[i] >= hsfMotP->maxTime) {
                modelP->clusterTime[i] = hsfMotP->maxTime;
            }
        }
    }

}

#define HU3D_CONST_FORCE_POS ((1 << 4)|(1 << 5)|(1 << 6))
#define HU3D_CONST_FORCE_ROT ((1 << 7)|(1 << 8)|(1 << 9))
#define HU3D_CONST_FORCE_SCALE                                                                     \
    ((1 << 20) | (1 << 21) | (1 << 22))

#define HU3D_CONST_FORCE_ANY (HU3D_CONST_FORCE_POS|HU3D_CONST_FORCE_ROT|HU3D_CONST_FORCE_SCALE)

static BOOL MLoopCheck(HSF_OBJECT *objPtr);
static BOOL TimingHookCheck(HU3D_MODELID modelId, HU3D_MOTIONID motId, HSF_OBJECT *objP);

#define OBJ_TRX_NONE ((float *)-1)

float *fn_8002F114(HSF_OBJECT *objPtr, u16 channel);

/* Evaluates one motion's tracks into a model during Hu3DMotionCalc. */
void fn_8002E4C4(HU3D_MODELID modelId, HU3D_MOTIONID motId, float time, BOOL preserveXFormF)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HU3D_MOTION *motP = &lbl_802624B8[motId];
    HSF_DATA *hsf = modelP->hsf;
    HSF_DATA *motHsf = motP->hsf;
    HSF_MOTION *hsfMotP = motHsf->motion;
    HSF_TRACK *trackP = hsfMotP->track;
    HSF_OBJECT *objPtr = hsf->object;
    HSF_OBJECT *objP;
    HSF_CONSTDATA *constData;
    HSF_TRACK *trackPEnd;
    if(preserveXFormF == FALSE) {
        s16 attr;
        s16 i;
        for(i=0; i<hsf->objectNum; objPtr++, i++) {
            objP = objPtr;
            if(objP->constData) {
                constData = objP->constData;
                if(constData->attr & HU3D_CONST_FORCE_ANY) {
                    /* This short mask loses scale-force bits; the hilite flag makes it negative and
                     * thereby preserves all scale components during this reset. */
                    s16 forceAttr = constData->attr;
                    if(!(forceAttr & (1 << 4))) {
                        objP->mesh.curr.pos.x = objP->mesh.base.pos.x;
                    }
                    if(!(forceAttr & (1 << 5))) {
                        objP->mesh.curr.pos.y = objP->mesh.base.pos.y;
                    }
                    if(!(forceAttr & (1 << 6))) {
                        objP->mesh.curr.pos.z = objP->mesh.base.pos.z;
                    }
                    if(!(forceAttr & (1 << 7))) {
                        objP->mesh.curr.rot.x = objP->mesh.base.rot.x;
                    }
                    if(!(forceAttr & (1 << 8))) {
                        objP->mesh.curr.rot.y = objP->mesh.base.rot.y;
                    }
                    if(!(forceAttr & (1 << 9))) {
                        objP->mesh.curr.rot.z = objP->mesh.base.rot.z;
                    }
                    if(!(forceAttr & (1 << 20))) {
                        objP->mesh.curr.scale.x = objP->mesh.base.scale.x;
                    }
                    if(!(forceAttr & (1 << 21))) {
                        objP->mesh.curr.scale.y = objP->mesh.base.scale.y;
                    }
                    if(!(forceAttr & (1 << 22))) {
                        objP->mesh.curr.scale.z = objP->mesh.base.scale.z;
                    }
                } else {
                    objP->mesh.curr = objP->mesh.base;
                }
            } else {
                objP->mesh.curr = objP->mesh.base;
            }
        }
    }
    for(trackPEnd = &trackP[hsfMotP->numTracks]; trackP < trackPEnd; trackP++) {
        switch(trackP->type) {
            HSF_CLUSTER *clusterP;

            case 2:
                if(trackP->target < hsf->objectNum && trackP->target != -1) {
                    objP = &hsf->object[trackP->target];
                    if(trackP->channel == 40) {
                        objP->mesh.mesh.baseMorph = GetCurve(trackP, time);
                    } else if(objP->type == 7) {
                        if(modelP->attr & (1 << 13)) {
                            fn_8002F780(modelId, trackP, GetCurve(trackP, time));
                        }
                    } else if(objP->type == 8) {
                        fn_8002FD84(modelId, trackP, GetCurve(trackP, time));
                    } else if(trackP->channel == 24) {
                        /* A 1.0 lock key can move the main-motion loop start, even in an overlay or
                         * shape pass, and invoke the NOW hook. Invoking the hook leaves the lock
                         * unchanged; otherwise 1.0 releases it and other values lock it. */
                        if ((motP->attr & (1 << 0)) &&
                            0.0 == modelP->motWork.start && GetCurve(trackP, time) == 1.0f &&
                            MLoopCheck(objP)) {
                            modelP->motWork.start = time;
                        }
                        if (!modelP->timingHook || objP != modelP->timingHookObj ||
                            GetCurve(trackP, time) != 1.0f ||
                            !TimingHookCheck(modelId, motId, objP)) {
                            if(objP->constData) {
                                constData = objP->constData;
                                if(GetCurve(trackP, time) == 1.0f) {
                                    constData->attr &= ~(1 << 12);
                                } else {
                                    constData->attr |= (1 << 12);
                                }
                            }
                        }
                    } else if(trackP->channel == 26) {
                        /* Value 1.0 clears the runtime display-off flag; every other value sets
                         * it. */
                        if(objP->constData) {
                            constData = objP->constData;
                            if(GetCurve(trackP, time) == 1.0f) {
                                constData->attr &= ~(1 << 13);
                            } else {
                                constData->attr |= (1 << 13);
                            }
                        }
                    } else {
                        float *trxP = fn_8002F114(objP, trackP->channel);
                        if(trxP != OBJ_TRX_NONE) {
                            *trxP = GetCurve(trackP, time);
                        }
                    }
                }
                break;

            case 3:
            {
                HSF_TRACK *track = trackP;
                if(track->target < hsf->objectNum) {
                    objP = &hsf->object[track->target];
                    objP->mesh.mesh.morphWeight[track->morphWeight] = GetCurve(track, time);

                }
            }
                break;

            case 9:
                if(!(modelP->attr & (1 << 19)) && trackP->attrIdx < hsf->materialNum) {
                    fn_8002F334(modelId, trackP, GetCurve(trackP, time));
                }
                break;

            case 5:
                if (!(modelP->attr & (1 << 19))) {
                    clusterP = &hsf->cluster[trackP->cluster];
                    clusterP->index = GetClusterCurve(trackP, time);
                }
                break;

            case 6:
                if (!(modelP->attr & (1 << 19))) {
                    HSF_TRACK *track = trackP;
                    clusterP = &hsf->cluster[track->cluster];
                    clusterP->weight[track->clusterWeight] = GetClusterWeightCurve(track, time);
                }
                break;

            case 10:
            {
                HSF_TRACK *track = trackP;
                /* Tracks whose cluster field differs from -1 bypass HU3D_ATTR_CURVE_MOTOFF;
                * that flag blocks only attribute tracks with cluster == -1. */
                if(track->cluster != -1 || !(modelP->attr & (1 << 19))) {
                    if(track->attrIdx != -1 && track->attrIdx < hsf->attributeNum) {
                        fn_8002F538(modelId, track, GetCurve(track, time));
                    }
                }
            }
                break;
        }
    }
}

/* Detects the MLOOP marker in an object's name for motion-lock restart handling. */
static BOOL MLoopCheck(HSF_OBJECT *objPtr)
{
    char *name;
    for(name = objPtr->name; *name; name++) {
        if(name[0] == 'M' && name[1] == 'L' && name[2] == 'O' && name[3] == 'O' && name[4] == 'P') {
            return TRUE;
        }
    }
    return FALSE;
}

/* Calls the model timing hook for a NOW marker and records the update frame. */
static BOOL TimingHookCheck(HU3D_MODELID modelId, HU3D_MOTIONID motId, HSF_OBJECT *objP)
{
    char *name;
    for(name = objP->name; *name; name++) {
        if(name[0] == 'N' && name[1] == 'O' && name[2] == 'W') {
            HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
            HU3D_TIMING_HOOK timingHook;
            BOOL lagF;
            /* The lag flag is false only when the previous hook call was one frame counter
            * tick ago; a second call during the same tick also reports lag. */
            if(GlobalCounter-modelP->endCounter == 1) {
                lagF = FALSE;
            } else {
                lagF = TRUE;
            }
            timingHook = modelP->timingHook;
            timingHook(modelId, motId, lagF);
            modelP->endCounter = GlobalCounter;
            return TRUE;
        }
    }
    return FALSE;
}

/* Applies camera tracks from the model's current main motion when camera motion is enabled. */
void fn_8002EC20(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HU3D_MOTION *motP = &lbl_802624B8[modelP->motId];
    HSF_DATA *hsf = motP->hsf;
    HSF_MOTION *hsfMotP = hsf->motion;
    HSF_TRACK *trackP = hsfMotP->track;
    if(modelP->attr & (1 << 13)) {
        HSF_TRACK *trackPEnd;
        for(trackPEnd = &trackP[hsfMotP->numTracks]; trackP < trackPEnd; trackP++) {
            if(trackP->type == 2 && trackP->index == 7) {
                fn_8002F780(modelId, trackP, GetCurve(trackP, modelP->motWork.time));
            }
        }
    }
}

/* Blends pending transform tracks into current transforms; if no main motion is set,
* first resets every object to its base transform, including forced components. */
void fn_8002ECDC(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HU3D_MOTION *motP = &lbl_802624B8[modelP->motIdShift];
    HSF_DATA *hsf = modelP->hsf;
    HSF_DATA *motHsf = motP->hsf;
    HSF_MOTION *hsfMotP = motHsf->motion;
    HSF_TRACK *trackP = hsfMotP->track;
    HSF_OBJECT *objPtr = hsf->object;
    HSF_OBJECT *obj;
    s16 i;
    float weight;
    float invWeight;
    if(modelP->motId == -1) {
        for(i=0; i<hsf->objectNum; objPtr++, i++) {
            obj = objPtr;
            obj->mesh.curr = obj->mesh.base;
        }
    }
    if(modelP->motOvlWork.end) {
        weight = modelP->motOvlWork.start/modelP->motOvlWork.end;
    } else {
        weight = 1.0f;
    }
    invWeight = 1.0f-weight;
    for(i=0; i<hsfMotP->numTracks; i++, trackP++) {
        switch(trackP->type) {
            case 2:
                if (trackP->target < hsf->objectNum && trackP->target != -1) {
                    s16 channel;
                    float *trxP;
                    obj = &hsf->object[trackP->target];
                    channel = trackP->channel;
                    if(channel == 24) {
                        /* Sub-motion lock tracks can move the shift-loop start and invoke the NOW
                         * hook, but do not change the object's lock flag. */
                        if ((motP->attr & (1 << 0)) &&
                            0.0 == modelP->motShiftWork.start &&
                            GetCurve(trackP, modelP->motShiftWork.time) == 1.0f &&
                            MLoopCheck(obj)) {
                            modelP->motShiftWork.start = modelP->motShiftWork.time;
                        }
                        if (modelP->timingHook && obj == modelP->timingHookObj &&
                            GetCurve(trackP, modelP->motShiftWork.time) == 1.0f) {
                            TimingHookCheck(modelId, modelP->motIdShift, obj);
                        }
                    } else {
                        float *trxP = fn_8002F114(obj, channel);
                        if(trxP != OBJ_TRX_NONE) {
                            if (channel == 28 || channel == 29 ||
                                channel == 30) {
                                /* Correct negative angles by one turn and adjust across the
                                 * 180-degree boundary before blending. These corrections also
                                 * modify the stored current angle in place. */
                                float rot = GetCurve(trackP, modelP->motShiftWork.time);
                                if(rot < 0.0f) {
                                    rot += 360.0f;
                                }
                                if(*trxP < 0.0f) {
                                    *trxP += 360.0f;
                                }
                                if(rot < 180.0f) {
                                    if(*trxP > rot+180.0f) {
                                        *trxP -= 360.0f;
                                    }
                                } else {
                                    if(*trxP < rot-180.0f) {
                                        rot -= 360.0f;
                                    }
                                }
                                *trxP = (invWeight*(*trxP))+(weight*rot);
                            } else {
                                *trxP = (invWeight * (*trxP)) +
                                        (weight * GetCurve(trackP, modelP->motShiftWork.time));
                            }
                        }
                    }

                }
                break;
        }
    }
}

/* Returns a pointer to a current position, rotation, or scale component; forced or unsupported
 * channels return the -1 sentinel. */
float *fn_8002F114(HSF_OBJECT *objPtr, u16 channel)
{
    HSF_CONSTDATA *constData = objPtr->constData;
    if(channel <= 10) {
        if(channel == 8) {
            if(constData && (constData->attr & (1 << 4))) {
                return OBJ_TRX_NONE;
            }
            return &objPtr->mesh.curr.pos.x;
        } else if(channel == 9) {
            if(constData && (constData->attr & (1 << 5))) {
                return OBJ_TRX_NONE;
            }
            return &objPtr->mesh.curr.pos.y;
        } else if(channel == 10) {
            if(constData && (constData->attr & (1 << 6))) {
                return OBJ_TRX_NONE;
            }
            return &objPtr->mesh.curr.pos.z;
        }
    } else if(channel <= 30) {
        if(channel == 28) {
            if(constData && (constData->attr & (1 << 7))) {
                return OBJ_TRX_NONE;
            }
            return &objPtr->mesh.curr.rot.x;
        } else if(channel == 29) {
            if(constData && (constData->attr & (1 << 8))) {
                return OBJ_TRX_NONE;
            }
            return &objPtr->mesh.curr.rot.y;
        } else if(channel == 30) {
            if(constData && (constData->attr & (1 << 9))) {
                return OBJ_TRX_NONE;
            }
            return &objPtr->mesh.curr.rot.z;
        }
    } else if(channel >= 31 && channel <= 33) {
        if(channel == 31) {
            if(constData && (constData->attr & (1 << 20))) {
                return OBJ_TRX_NONE;
            }
            return &objPtr->mesh.curr.scale.x;
        } else if(channel == 32) {
            if(constData && (constData->attr & (1 << 21))) {
                return OBJ_TRX_NONE;
            }
            return &objPtr->mesh.curr.scale.y;
        } else if(channel == 33) {
            if(constData && (constData->attr & (1 << 22))) {
                return OBJ_TRX_NONE;
            }
            return &objPtr->mesh.curr.scale.z;
        }
    }
    return OBJ_TRX_NONE;
}

/* Applies a material color or alpha track after clamping its value to [0, 1].
* Inverse-alpha tracks are ignored while HU3D_ATTR_TPLVL_SET is active. */
void fn_8002F334(HU3D_MODELID modelId, HSF_TRACK *trackP, float value)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HSF_DATA *hsf = modelP->hsf;
    HSF_MATERIAL *matP = &hsf->material[trackP->attrIdx];
    float weight = value;
    if(value > 1) {
        weight = 1;
    } else if(value < 0) {
        weight = 0;
    }
    switch(trackP->channel) {
        case 0:
            matP->litColor[0] = weight*255;
            break;

        case 1:
            matP->litColor[1] = weight*255;
            break;

        case 2:
            matP->litColor[2] = weight*255;
            break;

        case 49:
            matP->color[0] = weight*255;
            break;

        case 50:
            matP->color[1] = weight*255;
            break;

        case 51:
            matP->color[2] = weight*255;
            break;

        case 52:
            matP->shadowColor[0] = weight*255;
            break;

        case 53:
            matP->shadowColor[1] = weight*255;
            break;

        case 54:
            matP->shadowColor[2] = weight*255;
            break;

        case 57:
            if(!(modelP->attr & (1 << 18))) {
                matP->invAlpha = weight;
            }
            break;

        case 60:
            matP->refAlpha = weight;
            break;
    }
}

/* Applies a texture-attribute track, creating per-model animation work when needed. */
void fn_8002F538(HU3D_MODELID modelId, HSF_TRACK *trackP, float value)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HSF_DATA *hsf = modelP->hsf;
    HSF_ATTRIBUTE *attrP = &hsf->attribute[trackP->attrIdx];
    float weight = value;
    HU3D_ATTR_ANIM *attrAnimP;
    if(value > 1) {
        weight = 1;
    } else if(value < 0) {
        weight = 0;
    }
    switch(trackP->channel) {
        case 8:
        case 9:
        case 10:
        case 28:
        case 29:
        case 30:
        case 31:
        case 32:
        case 33:
        case 67:
            if(!attrP->animWorkP) {
                if(modelP->linkMdlId != -1) {
                    attrAnimP = HuMemDirectMallocNum(2, sizeof(HU3D_ATTR_ANIM),
                                                     lbl_802C01C8[modelId].mallocNoLink);
                    attrP->animWorkP = attrAnimP;
                } else {
                    attrAnimP = HuMemDirectMallocNum(2, sizeof(HU3D_ATTR_ANIM),
                                                     lbl_802C01C8[modelId].mallocNo);
                    attrP->animWorkP = attrAnimP;
                }

                attrAnimP->attr = 0;
                attrAnimP->trans3D.x = attrAnimP->trans3D.y = attrAnimP->trans3D.z = 0;
                attrAnimP->rot.x = attrAnimP->rot.y = attrAnimP->rot.z = 0;
                attrAnimP->scale3D.x = attrAnimP->scale3D.y = attrAnimP->scale3D.z = 1;
            } else {
                attrAnimP = attrP->animWorkP;
            }
            if(trackP->channel != 67) {
                attrAnimP->attr |= (1 << 2);
            } else {
                attrAnimP->attr |= (1 << 3);
            }
            break;
    }
    switch(trackP->channel) {
        case 8:
            attrAnimP->trans3D.x = value;
            break;

        case 9:
            attrAnimP->trans3D.y = value;
            break;

        case 10:
            attrAnimP->trans3D.z = value;
            break;

        case 28:
            attrAnimP->rot.x = value;
            break;

        case 29:
            attrAnimP->rot.y = value;
            break;

        case 30:
            attrAnimP->rot.z = value;
            break;

        case 31:
            attrAnimP->scale3D.x = value;
            break;

        case 32:
            attrAnimP->scale3D.y = value;
            break;

        case 33:
            attrAnimP->scale3D.z = value;
            break;

        case 64:
            /* hsfdraw checks for exactly 1.0 to enable animated texture-matrix handling. */
            attrP->texMtxAnimEnabled = weight;
            break;

        case 62:
            attrP->kColor = weight;
            break;

        case 63:
            attrP->nbtTpLvl = value;
            break;

        case 67:
            attrAnimP->bitMapPtr = bitMapPtr;
            break;
    }
}

/* Applies one camera track to every camera selected by the model's camera mask. */
void fn_8002F780(HU3D_MODELID modelId, HSF_TRACK *trackP, float value)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    s16 cameraBit = modelP->camInfoBit;
    s16 i;
    s16 bit;
    float newValue;
    float weight;
    if(cameraBit == 0) {
        return;
    }
    /* The code clamps weight, but camera channels below use value directly. */
    weight = value;
    if(value > 1) {
        weight = 1;
    } else if(value < 0) {
        weight = 0;
    }
    /* Position and target tracks scale the sum of value and model position, including
    * the model translation itself. Model rotation is not applied to these channels. */
    switch(trackP->channel) {
        case 8:
            newValue = modelP->scale.x*(value+modelP->pos.x);
            for(i=0, bit=1; i<16; i++, bit <<= 1) {
                if(bit & cameraBit) {
                    Hu3DCamera[i].pos.x = newValue;
                }
            }
            break;

        case 9:
            newValue = modelP->scale.y*(value+modelP->pos.y);
            for(i=0, bit=1; i<16; i++, bit <<= 1) {
                if(bit & cameraBit) {
                    Hu3DCamera[i].pos.y = newValue;
                }
            }
            break;

        case 10:
            newValue = modelP->scale.z*(value+modelP->pos.z);
            for(i=0, bit=1; i<16; i++, bit <<= 1) {
                if(bit & cameraBit) {
                    Hu3DCamera[i].pos.z = newValue;
                }
            }
            break;

         case 11:
            newValue = modelP->scale.x*(value+modelP->pos.x);
            for(i=0, bit=1; i<16; i++, bit <<= 1) {
                if(bit & cameraBit) {
                    Hu3DCamera[i].target.x = newValue;
                }
            }
            break;

        case 12:
            newValue = modelP->scale.y*(value+modelP->pos.y);
            for(i=0, bit=1; i<16; i++, bit <<= 1) {
                if(bit & cameraBit) {
                    Hu3DCamera[i].target.y = newValue;
                }
            }
            break;

        case 13:
            newValue = modelP->scale.z*(value+modelP->pos.z);
            for(i=0, bit=1; i<16; i++, bit <<= 1) {
                if(bit & cameraBit) {
                    Hu3DCamera[i].target.z = newValue;
                }
            }
            break;

        /* Rotates world-up around the normalized target-to-camera direction by value degrees,
        * then normalizes the result for the camera's up vector. */
        case 14:
            for(i=0, bit=1; i<16; i++, bit <<= 1) {
                if(bit & cameraBit) {
                    HuVecF upOfs;
                    HuVecF dir;
                    VECSubtract(&Hu3DCamera[i].pos, &Hu3DCamera[i].target, &dir);
                    VECNormalize(&dir, &dir);
                    upOfs.x = ((dir.x * dir.y * (1-cos(3.141592653589793*value/180.0)))
                        - (dir.z * sin(3.141592653589793*value/180.0)));
                    upOfs.y = ((dir.y * dir.y)
                        + (1-(dir.y*dir.y)) * cos(3.141592653589793*value/180.0));
                    upOfs.z = (((dir.y * dir.z) * (1-cos(3.141592653589793*value/180.0)))
                        + (dir.x * sin(3.141592653589793*value/180.0)));
                    VECNormalize(&upOfs, &Hu3DCamera[i].up);
                    Hu3DCamera[i].upRot = value;
                }
            }
            break;

        case 15:
            for(i=0, bit=1; i<16; i++, bit <<= 1) {
                if(bit & cameraBit) {
                    Hu3DCamera[i].fov = value;
                }
            }
            break;

        case 17:
            for(i=0, bit=1; i<16; i++, bit <<= 1) {
                if(bit & cameraBit) {
                    Hu3DCamera[i].near = value;
                }
            }
            break;

        case 18:
            for(i=0, bit=1; i<16; i++, bit <<= 1) {
                if(bit & cameraBit) {
                    Hu3DCamera[i].far = value;
                }
            }
            break;
    }
}

/* Applies one light track to the light corresponding to its HSF object target. */
void fn_8002FD84(HU3D_MODELID modelId, HSF_TRACK *trackP, float value)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HSF_DATA *hsf = modelP->hsf;
    HSF_OBJECT *objPtr = hsf->object;
    HU3D_LIGHT *lightP;
    float weight;
    s16 i;
    s16 lightId;
    lightId = 0;
    for(i=lightId; i<hsf->objectNum; i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->type == 8) {
            if(i == trackP->target) {
                break;
            }
            lightId++;
        }
    }
    if(i == hsf->objectNum) {
        return;
    }
    lightP = &Hu3DGlobalLight[modelP->lightId[lightId]];
    weight = value;
    if(value > 1) {
        weight = 1;
    } else if(value < 0) {
        weight = 0;
    }
    /* Position tracks change the stored position without updating direction.
    * Target tracks rebuild direction using all three stored target coordinates. */
    switch(trackP->channel) {
        case 43:
            lightP->color.r = 255.0f*weight;
            break;

        case 44:
            lightP->color.g = 255.0f*weight;
            break;

        case 45:
            lightP->color.b = 255.0f*weight;
            break;

        case 8:
            lightP->pos.x = value;
            break;

        case 9:
            lightP->pos.y = value;
            break;

        case 10:
            lightP->pos.z = value;
            break;

        case 11:
            lightP->offset.x = value;
            Hu3DGLightPosAimSetV(modelP->lightId[lightId], &lightP->pos, &lightP->offset);
            break;

        case 12:
            lightP->offset.y = value;
            Hu3DGLightPosAimSetV(modelP->lightId[lightId], &lightP->pos, &lightP->offset);
            break;

        case 13:
            lightP->offset.z = value;
            Hu3DGLightPosAimSetV(modelP->lightId[lightId], &lightP->pos, &lightP->offset);
            break;
    }
}

/* Samples the selected curve type; bitmap tracks update bitMapPtr and return zero here. */
float GetCurve(HSF_TRACK *trackP, float time)
{
    switch (trackP->curveType) {
        case 1:
            return GetLinear(trackP->numKeyframes, trackP->data, time);

        case 2:
            return GetBezier(trackP->numKeyframes, trackP, time);

        case 3:
            bitMapPtr = GetBitMap(trackP->numKeyframes, trackP->data, time);
            break;
        case 4:
        {
            float *data = (float *)&trackP->data;
            return *data;
        }

        case 0:
            return GetConstant(trackP->numKeyframes, trackP->data, time);
    }
    return 0;
}

/* Samples the preceding step key; time zero or a single key returns the first value.
 * Other times before the first key read preceding storage rather than clamping. */
float GetConstant(s32 keyNum, HSF_CONSTANT_KEY *key, float time)
{
    HSF_CONSTANT_KEY *data = key;
    s16 i;
    if(0.0f == time || keyNum == 1) {
        return key[0][1];
    }
    for(i=0; i<keyNum; i++, data++) {
        if(time < data[0][0]) {
            return data[0][-1];
        }
    }
    return data[0][-1];
}

/* Interpolates surrounding keys and clamps above the final key; time zero or one key
 * returns the first value. Other times before the first key use key[-1]. */
float GetLinear(s32 keyNum, HSF_LINEAR_KEY *key, float time)
{
    float ret;
    float diff;
    s16 i;
    if(0.0f == time || keyNum == 1) {
        return key[0][1];
    }
    for(i=0; i<keyNum; i++) {
        if(time < key[i][0]) {
            s16 prevIdx = i-1;
            diff = key[i][0]-key[prevIdx][0];
            ret =
                key[prevIdx][1] + (time - key[prevIdx][0]) * ((key[i][1] - key[prevIdx][1]) / diff);
            return ret;
        }
    }
    return key[keyNum-1][1];
}

/* Samples a Bezier curve and caches the upper key index in trackP->start.
 * Time zero or one key returns the first value; other times before the first key
 * can select index zero and then use key[-1] instead of clamping. */
float GetBezier(s32 keyNum, HSF_TRACK *trackP, float time)
{
    HSF_BEZIER_KEY *key = trackP->data;
    HSF_BEZIER_KEY *keyPrev;
    s32 startKey;
    float keyTime;
    float keyTimePrev;
    float t;
    float tt;
    float tt2;
    float tt3;
    float ttt;
    float ttt2;

    if (time == 0.0f || keyNum == 1) {
        return key[0][1];
    }
    startKey = -1;
    if (trackP->start == 0 && time < key[0][0]) {
        startKey = 0;
    }
    if (startKey == -1) {
        key = (HSF_BEZIER_KEY *) trackP->data + trackP->start;
        keyPrev = key - 1;
        for (startKey=trackP->start; startKey<keyNum; startKey++, key++) {
            if (time >= keyPrev[0][0] && time < key[0][0]) {
                break;
            }
            keyPrev = key;
        }
        if (startKey >= keyNum) {
            key = trackP->data;
            for (startKey=0; startKey<keyNum; startKey++, key++) {
                if (time < key[0][0]) {
                    break;
                }
            }
        }
    }
    trackP->start = startKey;
    if (startKey == keyNum) {
        return key[-1][1];
    }
    keyTime = key[0][0];
    key--;
    keyTimePrev = key[0][0];
    t = (time - keyTimePrev) / (keyTime - keyTimePrev);
    tt = t * t;
    tt2 = 2.0f * tt;
    tt3 = 3.0f * tt;
    ttt = t * tt;
    ttt2 = t * tt2;
    return key[0][1] * (ttt2 - tt3 + 1.0f)
        + key[1][1] * (-ttt2 + tt3)
        + key[0][2] * (ttt - tt2 + t)
        + key[1][3] * (ttt - tt);
}

/* Selects a bitmap key by converting its seconds to 60-frame motion time.
 * Time zero or one key selects the first bitmap; other times before the first key
 * return key[-1].data rather than clamping. */
HSF_BITMAP *GetBitMap(s32 keyNum, HSF_BITMAP_KEY *key, float time)
{
    s16 i;
    if(0.0f == time || keyNum == 1) {
        return key->data;
    }
    for(i=0; i<keyNum; i++, key++) {
        if(time < key->time*60.0f) {
            break;
        }
    }
    return key[-1].data;
}

/* Creates a motion and resolves its track targets against the model's HSF objects. */
HU3D_MOTIONID fn_80030458(HU3D_MODELID modelId, void *data)
{
    HU3D_MOTIONID motId = fn_8002C4C0(data);
    fn_80030524(modelId, motId);
    return motId;
}

static s32 SearchObjectIndex(HSF_DATA *hsf, u32 nameOffset);
static s32 SearchAttributeIndex(HSF_DATA *hsf, u32 nameOffset);

/* Resolves transform/morph object targets and missing attribute indices when attaching a motion. */
void fn_80030524(HU3D_MODELID modelId, HU3D_MOTIONID motId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    HU3D_MOTION *motP = &lbl_802624B8[motId];
    HSF_DATA *hsf = modelP->hsf;
    HSF_DATA *motHsf = motP->hsf;
    HSF_MOTION *hsfMotP = motHsf->motion;
    HSF_TRACK *trackP = hsfMotP->track;
    s32 i;
    for(i=0; i<hsfMotP->numTracks; i++, trackP++) {
        switch (trackP->type) {
            case 2:
            {
                HSF_TRACK *track = trackP;
                track->target = SearchObjectIndex(hsf, track->target);
            }
                break;

            case 3:
            {
                HSF_TRACK *track = trackP;
                track->target = SearchObjectIndex(hsf, track->target);
            }
                break;

            case 10:
            {
                HSF_TRACK *track = trackP;
                if(track->attrIdx == -1) {
                    s32 attrIdx = SearchAttributeIndex(hsf, track->cluster);
                    if(attrIdx != -1) {
                        track->attrIdx = attrIdx;
                    } else {
                        track->attrIdx = -1;
                    }
                }
            }
                break;
        }
    }
}

/* Called by model update code to evaluate active motions and deform the model's HSF. */
void fn_80030644(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    if ((modelP->attr & (1 << 0)) || (modelP->attr & (1 << 15))) {
        return;
    }
    if(modelP->motId != -1) {
        fn_8002E4C4(modelId, modelP->motId, modelP->motWork.time, FALSE);
    }
    if(modelP->motIdShift != -1) {
        fn_8002ECDC(modelId);
    }
    if(modelP->motIdOvl != -1) {
        fn_8002E4C4(modelId, modelP->motIdOvl, modelP->motOvlWork.time, TRUE);
    }
    if(modelP->attr & (1 << 10)) {
        ClusterMotionExec(modelP);
    }
    if(modelP->motIdShape != -1) {
        /* With no main motion, shape evaluation resets base transforms after any
         * sub-motion and overlay passes, discarding their unforced transform results. */
        if(modelP->motId == -1) {
            fn_8002E4C4(modelId, modelP->motIdShape, modelP->motShapeWork.time, FALSE);
        } else {
            fn_8002E4C4(modelId, modelP->motIdShape, modelP->motShapeWork.time, TRUE);
        }
    }
    if ((modelP->attr & ((1 << 7)|(1 << 4))) == 0) {
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

/* Resolves an encoded HSF object name to its index in the model's object array. */
static s32 SearchObjectIndex(HSF_DATA *hsf, u32 nameOffset)
{
    s32 i;
    HSF_OBJECT *objPtr = hsf->object;
    char *name = SetName(&nameOffset);

    for (i=0; i<hsf->objectNum; i++, objPtr++) {
        if(!CmpObjectName(objPtr->name, name)) {
            return i;
        }
    }
    return -1;
}

/* Resolves an encoded attribute name to the first attribute name that prefixes it. */
static s32 SearchAttributeIndex(HSF_DATA *hsf, u32 nameOffset)
{
    size_t nameLen;
    HSF_ATTRIBUTE *attrP = hsf->attribute;
    char *name = SetName(&nameOffset);
    s32 i;
    for (i=0; i<hsf->attributeNum; i++, attrP++) {
        if(attrP->name) {
            nameLen = strlen(attrP->name);
            if(!strncmp(attrP->name, name, nameLen)) {
                return i;
            }
        }
    }
    return -1;
}

/* Selects a sub-motion with a held blend weight supplied as a fraction.
 * The incoming motion's playback time is left unchanged. */
void fn_800308F0(HU3D_MODELID modelId, HU3D_MOTIONID motId, float blendWeight)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];

    modelP->motIdShift = motId;
    fn_800269E8(modelId, ((1 << 30) | (1 << 12)));
    modelP->motOvlWork.start = 100.0*blendWeight;
    modelP->motOvlWork.end = 100;
    modelP->motShiftWork.speed = 1;
    modelP->motShiftWork.start = 0;
    modelP->motShiftWork.end = Hu3DMotionMotionMaxTimeGet(motId);
}

/* Cancels the pending sub-motion and clears its execution flag. */
void fn_800309F4(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->motIdShift = -1;
    fn_80026A44(modelId, ((1 << 30) | (1 << 12)));
}

/* Sets the blend weight from a fraction; playback holds it while EXECSHIFT is set. */
void fn_80030A4C(HU3D_MODELID modelId, float blendWeight)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->motOvlWork.start = 100.0*blendWeight;
    modelP->motOvlWork.end = 100;
}

/* Sets playback time for the motion currently being blended in. */
void fn_80030A88(HU3D_MODELID modelId, float time)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    modelP->motShiftWork.time = time;
}
