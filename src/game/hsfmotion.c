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
    (HU3D_MOTATTR_SHIFT_LOOP | HU3D_MOTATTR_SHIFT_PAUSE | HU3D_MOTATTR_SHIFT_REV)
#define HU3D_MOTATTR_NOSHIFT_ALL (HU3D_MOTATTR_LOOP|HU3D_MOTATTR_PAUSE|HU3D_MOTATTR_REV)
#define HU3D_MOTATTR_ALL (HU3D_MOTATTR_SHIFT_ALL|HU3D_MOTATTR_NOSHIFT_ALL)
#define texMtxAnimEnabled textureEnable

HU3D_MOTION Hu3DMotion[HU3D_MOTION_MAX];

static HSF_BITMAP *bitMapPtr;

/* Sets a pointer-sized field to NULL at each HU3D_MOTION-sized offset from Hu3DData. */
void Hu3DMotionInit(void)
{
    HU3D_MOTION *motP;
    s16 i;
    for(motP = (HU3D_MOTION *)Hu3DData, i=0; i<HU3D_MOTION_MAX; i++, motP++) {
        motP->hsf = NULL;
    }
}

/* Creates a motion slot from an independently loaded HSF; callers later attach it to a model. */
HU3D_MOTIONID Hu3DMotionCreate(void *data)
{
    HU3D_MOTION *motP;
    HU3D_MOTIONID motId;
    for(motP = Hu3DMotion, motId=0; motId<HU3D_MOTION_MAX; motId++, motP++) {
        if(!motP->hsf) {
            break;
        }
    }
    if(motId == HU3D_MOTION_MAX) {
        OSReport("Error: Create Motion Over!\n");
        return HU3D_MOTIONID_NONE;
    }
    motP->hsf = LoadHSF(data);
    motP->attr = 0;
    motP->modelId = HU3D_MODELID_NONE;
    return motId;
}

/* Creates a motion slot that shares the selected model's HSF data. */
HU3D_MOTIONID Hu3DMotionModelCreate(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HU3D_MOTION *motP;
    HU3D_MOTIONID motId;
    for(motP = Hu3DMotion, motId=0; motId<HU3D_MOTION_MAX; motId++, motP++) {
        if(!motP->hsf) {
            break;
        }
    }
    if(motId == HU3D_MOTION_MAX) {
        OSReport("Error: Create Motion Over!\n");
        return HU3D_MOTIONID_NONE;
    }
    motP->hsf = modelP->hsf;
    motP->attr = 0;
    motP->modelId = modelId;
    modelP->motIdSrc = motId;
    return motId;
}

/* Releases a motion slot if no other model uses it; its source model is ignored by this check. */
BOOL Hu3DMotionKill(HU3D_MOTIONID motId)
{
    HU3D_MOTION *motP = &Hu3DMotion[motId];
    HU3D_MODEL *modelP;
    s16 i;
    if(!motP->hsf) {
        return FALSE;
    }
    for(modelP=&Hu3DData[0], i=0; i<HU3D_MODEL_MAX; i++, modelP++) {
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
    if(i != HU3D_MODEL_MAX) {
        return FALSE;
    }
    if(motP->modelId == HU3D_MODELID_NONE) {
        HuMemDirectFree(motP->hsf);
    } else {
        Hu3DData[motP->modelId].motIdSrc = HU3D_MOTIONID_NONE;
    }
    motP->hsf = NULL;
    return TRUE;
}

/* Called during HSF manager shutdown to release every motion slot that can be killed. */
void Hu3DMotionAllKill(void)
{
    HU3D_MOTION *motP;
    HU3D_MOTIONID motId;
    for(motP = Hu3DMotion, motId=0; motId<HU3D_MOTION_MAX; motId++, motP++) {
        if(motP->hsf) {
            Hu3DMotionKill(motId);
        }
    }
}

/* Selects the main motion; NONE clears its motion assignments and resets main speed/time unless
* the current main slot is the model's source slot. */
void Hu3DMotionSet(HU3D_MODELID modelId, HU3D_MOTIONID motId)
{
    if(motId == HU3D_MOTIONID_NONE) {
        HU3D_MODEL *modelP = &Hu3DData[modelId];
        s16 i;
        if(modelP->motId == modelP->motIdSrc) {
            return;
        }
        modelP->motWork.speed = 1;
        for(i=0; i<HU3D_CLUSTER_MAX; i++) {
            modelP->motIdCluster[i] = HU3D_MOTIONID_NONE;
        }
        modelP->motIdOvl = HU3D_MOTIONID_NONE;
        modelP->motIdShift = HU3D_MOTIONID_NONE;
        modelP->motIdShape = HU3D_MOTIONID_NONE;
        modelP->motWork.time = 0;
        modelP->motId = HU3D_MOTIONID_NONE;
    } else {
        Hu3DData[modelId].motIdShift = HU3D_MOTIONID_NONE;
        Hu3DData[modelId].motId = motId;
        Hu3DData[modelId].motWork.time = 0;
        Hu3DData[modelId].motWork.start = 0;
        Hu3DData[modelId].motWork.end = Hu3DMotionMaxTimeGet(modelId);
    }
}

/* Starts an overlay motion on a model at time zero and normal speed. */
void Hu3DMotionOverlaySet(HU3D_MODELID modelId, HU3D_MOTIONID motId)
{
    Hu3DData[modelId].motIdOvl = motId;
    Hu3DData[modelId].motOvlWork.time = 0;
    Hu3DData[modelId].motOvlWork.speed = 1;
}

/* Detaches the model's overlay motion. */
void Hu3DMotionOverlayReset(HU3D_MODELID modelId)
{
    Hu3DData[modelId].motIdOvl = HU3D_MOTIONID_NONE;
}

/* Returns the current playback time of the model's overlay motion. */
float Hu3DMotionOverlayTimeGet(HU3D_MODELID modelId)
{
    return Hu3DData[modelId].motOvlWork.time;
}

/* Returns the duration of the motion currently assigned as the overlay. */
float Hu3DMotionOverlayMaxTimeGet(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    return Hu3DMotionMotionMaxTimeGet(modelP->motIdOvl);
}

/* Sets overlay playback time and marks the model for motion evaluation. */
void Hu3DMotionOverlayTimeSet(HU3D_MODELID modelId, float time)
{
    Hu3DData[modelId].attr |= HU3D_ATTR_MOTION_MODEL;
    Hu3DData[modelId].motOvlWork.time = time;
}

/* Sets the per-frame playback multiplier for the model's overlay motion. */
void Hu3DMotionOverlaySpeedSet(HU3D_MODELID modelId, float speed)
{
    Hu3DData[modelId].motOvlWork.speed = speed;
}

/* Checks the overlay endpoint selected by its reverse-playback flag. */
BOOL Hu3DMotionOverlayEndCheck(HU3D_MODELID modelId)
{
    if(!(Hu3DData[modelId].motAttr  & HU3D_MOTATTR_OVL_REV)) {
        return Hu3DMotionOverlayMaxTimeGet(modelId) <= Hu3DMotionOverlayTimeGet(modelId)+0.0001;
    } else {
        return Hu3DMotionOverlayTimeGet(modelId) <= 0.0001f;
    }
}

/* Begins a transition to motId; an existing pending motion first becomes the main motion.
 * On that replacement, attr's shift bits set the main flags. The new motion starts at start
 * with speed 1; end is the blend duration. */
void Hu3DMotionShiftSet(HU3D_MODELID modelId, HU3D_MOTIONID motId, float start, float end, u32 attr)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HU3D_MOTION *motP = &Hu3DMotion[motId];
    u32 motAttr;
    attr &= ~HU3D_MOTATTR;
    motAttr = HU3D_MOTATTR_NONE;
    if(modelP->motIdShift != HU3D_MOTIONID_NONE) {
        modelP->motId = modelP->motIdShift;
        modelP->motWork.time = modelP->motShiftWork.time;
        modelP->motWork.speed = modelP->motShiftWork.speed;
        modelP->motWork.start = modelP->motShiftWork.start;
        modelP->motWork.end = modelP->motShiftWork.end;
        if(attr & HU3D_MOTATTR_SHIFT_LOOP) {
            motAttr |= HU3D_MOTATTR_LOOP;
        }
        if(attr & HU3D_MOTATTR_SHIFT_PAUSE) {
            motAttr |= HU3D_MOTATTR_PAUSE;
        }
        if(attr & HU3D_MOTATTR_SHIFT_REV) {
            motAttr |= HU3D_MOTATTR_REV;
        }
        modelP->motAttr &= ~HU3D_MOTATTR_ALL;
        modelP->motAttr |= motAttr;
        modelP->motAttr &= ~HU3D_MOTATTR;
    } else {
        modelP->motAttr &= ~HU3D_MOTATTR_SHIFT_ALL;
    }
    modelP->motIdShift = motId;
    modelP->motShiftWork.time = start;
    modelP->motShiftWork.speed = 1;
    modelP->motOvlWork.start = 0;
    modelP->motOvlWork.end = end;
    modelP->motShiftWork.start = 0;
    modelP->motShiftWork.end = Hu3DMotionShiftMaxTimeGet(modelId);
    if(attr & HU3D_MOTATTR_LOOP) {
        motAttr |= HU3D_MOTATTR_SHIFT_LOOP;
    }
    if(attr & HU3D_MOTATTR_PAUSE) {
        motAttr |= HU3D_MOTATTR_SHIFT_PAUSE;
    }
    if(attr & HU3D_MOTATTR_REV) {
        motAttr |= HU3D_MOTATTR_SHIFT_REV;
    }
    attr &= ~HU3D_MOTATTR_NOSHIFT_ALL;
    modelP->motAttr |= motAttr|attr;
    modelP->motAttr &= ~HU3D_MOTATTR;
}

/* Assigns a shape animation and initializes its playback range and speed. */
void Hu3DMotionShapeSet(HU3D_MODELID modelId, HU3D_MOTIONID motId)
{
    Hu3DData[modelId].motIdShape = motId;
    Hu3DData[modelId].motShapeWork.time = 0;
    Hu3DData[modelId].motShapeWork.speed = 1;
    Hu3DData[modelId].motShapeWork.start = 0;
    Hu3DData[modelId].motShapeWork.end = Hu3DMotionShapeMaxTimeGet(modelId);
}

/* Returns the motion slot assigned to the model's shape animation. */
HU3D_MOTIONID Hu3DMotionShapeIDGet(HU3D_MODELID modelId)
{
    return Hu3DData[modelId].motIdShape;
}

/* Sets the playback multiplier for the model's shape animation. */
void Hu3DMotionShapeSpeedSet(HU3D_MODELID modelId, float speed)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->motShapeWork.speed = speed;
}

/* Sets shape-animation time and marks the model for motion evaluation. */
void Hu3DMotionShapeTimeSet(HU3D_MODELID modelId, float time)
{
    Hu3DData[modelId].attr |= HU3D_ATTR_MOTION_MODEL;
    Hu3DData[modelId].motShapeWork.time = time;
}

/* Returns the current time of the model's shape animation. */
float Hu3DMotionShapeTimeGet(HU3D_MODELID modelId)
{
    return Hu3DData[modelId].motShapeWork.time;
}

/* Returns the duration of the motion assigned to the model's shape animation. */
float Hu3DMotionShapeMaxTimeGet(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    return Hu3DMotionMotionMaxTimeGet(modelP->motIdShape);
}

/* Sets shape playback bounds; looping wraps at or beyond end and only below start. */
void Hu3DMotionShapeStartEndSet(HU3D_MODELID modelId, float start, float end)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->motShapeWork.start = start;
    modelP->motShapeWork.end = end;
}

/* Assigns a cluster animation to the first free cluster slot and returns that slot. */
s16 Hu3DMotionClusterSet(HU3D_MODELID modelId, HU3D_MOTIONID motId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    s16 i;
    for(i=0; i<HU3D_CLUSTER_MAX; i++) {
        if(modelP->motIdCluster[i] == HU3D_MOTIONID_NONE) {
            modelP->motIdCluster[i] = motId;
            modelP->clusterTime[i] = 0;
            modelP->clusterSpeed[i] = 1;
            modelP->clusterAttr[i] = HU3D_ATTR_NONE;
            modelP->attr |= HU3D_ATTR_CLUSTER_ON;
            ClusterAdjustObject(modelP->hsf, Hu3DMotion[motId].hsf);
            return i;
        }
    }
    OSReport("Error: Cluster Entry Over\n");
    return HU3D_CLUSTER_NONE;
}

/* Replaces the motion in a cluster slot, resetting its time and speed but retaining its cluster
 * attributes. */
s16 Hu3DMotionClusterNoSet(HU3D_MODELID modelId, HU3D_MOTIONID motId, s16 clusterNo)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    Hu3DMotionClusterReset(modelId, clusterNo);
    modelP->motIdCluster[clusterNo] = motId;
    modelP->clusterTime[clusterNo] = 0;
    modelP->clusterSpeed[clusterNo] = 1;
    modelP->attr |= HU3D_ATTR_CLUSTER_ON;
    ClusterAdjustObject(modelP->hsf, Hu3DMotion[motId].hsf);
    return clusterNo;
}

/* Removes the model's shape-animation assignment. */
void Hu3DMotionShapeReset(HU3D_MODELID modelId)
{
    Hu3DData[modelId].motIdShape = HU3D_MOTIONID_NONE;
}

/* Removes cluster motion assignments; per-slot time, speed, and attributes remain unchanged. */
void Hu3DMotionClusterReset(HU3D_MODELID modelId, s16 clusterNo)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    s16 i;
    if(clusterNo == HU3D_CLUSTER_NONE) {
        for(i=0; i<HU3D_CLUSTER_MAX; i++) {
            modelP->motIdCluster[i] = HU3D_MOTIONID_NONE;
        }
        modelP->attr &= ~HU3D_ATTR_CLUSTER_ON;
    } else {
        modelP->motIdCluster[clusterNo] = HU3D_MOTIONID_NONE;
        for(i=0; i<HU3D_CLUSTER_MAX; i++) {
            if(modelP->motIdCluster[i] != HU3D_MOTIONID_NONE) {
                return;
            }
        }
        modelP->attr &= ~HU3D_ATTR_CLUSTER_ON;
    }
}

/* Sets the playback multiplier for one cluster animation. */
void Hu3DMotionClusterSpeedSet(HU3D_MODELID modelId, s16 clusterNo, float speed)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->clusterSpeed[clusterNo] = speed;
}

/* Sets the current playback time for one cluster animation. */
void Hu3DMotionClusterTimeSet(HU3D_MODELID modelId, s16 clusterNo, float time)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->clusterTime[clusterNo] = time;
}

/* Returns the duration of the motion assigned to one cluster slot. */
float Hu3DMotionClusterMaxTimeGet(HU3D_MODELID modelId, s16 clusterNo)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    return Hu3DMotionMotionMaxTimeGet(modelP->motIdCluster[clusterNo]);
}

/* Returns the model's current main-motion slot. */
HU3D_MOTIONID Hu3DMotionIDGet(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    return modelP->motId;
}

/* Returns the model's pending transition-motion slot. */
HU3D_MOTIONID Hu3DMotionShiftIDGet(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    return modelP->motIdShift;
}

/* Sets main-motion time, clamped to zero and the motion's maximum time. */
void Hu3DMotionTimeSet(HU3D_MODELID modelId, float time)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    if(Hu3DMotionMaxTimeGet(modelId) <= time) {
        time = Hu3DMotionMaxTimeGet(modelId);
    }
    if(time < 0) {
        time = 0;
    }
    modelP->motWork.time = time;
    /* Camera-only records carry -1 instead of HSF data. Paused models with envelopes
    * request deformation work after their playback time changes. */
    if (modelP->hsf != ((void *) -1) && modelP->hsf->cenvNum != 0 &&
        (modelP->motAttr & HU3D_MOTATTR_PAUSE)) {
        modelP->attr |= HU3D_ATTR_MOTION_MODEL;
    }
}

/* Returns the current main-motion playback time. */
float Hu3DMotionTimeGet(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    return modelP->motWork.time;
}

/* Returns the current time of the motion being blended in. */
float Hu3DMotionShiftTimeGet(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    return modelP->motShiftWork.time;
}

/* Returns the main motion's duration after converting maxTime + 0.0001 through s16;
 * returns zero if no main motion is set. */
float Hu3DMotionMaxTimeGet(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HU3D_MOTION *motP;
    HSF_MOTION *hsfMotP;
    s16 maxTime;
    if(modelP->motId == HU3D_MOTIONID_NONE) {
        return 0;
    }
    motP = &Hu3DMotion[modelP->motId];
    hsfMotP = motP->hsf->motion;
    maxTime = hsfMotP->maxTime+0.0001;
    return maxTime;
}

/* Returns the pending motion's duration after converting maxTime + 0.0001 through s16;
 * returns zero when none is assigned. */
float Hu3DMotionShiftMaxTimeGet(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HU3D_MOTION *motP;
    HSF_MOTION *hsfMotP;
    s16 maxTime;
    if(modelP->motIdShift == HU3D_MOTIONID_NONE) {
        return 0;
    }
    motP = &Hu3DMotion[modelP->motIdShift];
    hsfMotP = motP->hsf->motion;
    maxTime = hsfMotP->maxTime+0.0001;
    return maxTime;
}

/* Sets the playback range for the motion being blended in. */
void Hu3DMotionShiftStartEndSet(HU3D_MODELID modelId, float start, float end)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->motShiftWork.start = start;
    modelP->motShiftWork.end = end;
}

/* Returns a motion slot's duration after converting maxTime + 0.0001 through s16;
 * returns zero for an empty slot. */
float Hu3DMotionMotionMaxTimeGet(HU3D_MOTIONID motId)
{
    HU3D_MOTION *motP = &Hu3DMotion[motId];
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
void Hu3DMotionStartEndSet(HU3D_MODELID modelId, float start, float end)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->motWork.start = start;
    modelP->motWork.end = end;
}

/* Checks the main-motion endpoint according to the model's reverse flag. */
BOOL Hu3DMotionEndCheck(HU3D_MODELID modelId)
{
    if(!(Hu3DData[modelId].motAttr & HU3D_MOTATTR_REV)) {
        return Hu3DData[modelId].motWork.end <= Hu3DMotionTimeGet(modelId)+0.0001f;
    } else {
        return Hu3DMotionTimeGet(modelId) <= Hu3DData[modelId].motWork.start+0.0001f;
    }
}

/* Sets the per-frame playback multiplier for the model's main motion. */
void Hu3DMotionSpeedSet(HU3D_MODELID modelId, float speed)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->motWork.speed = speed;
}

/* Sets the per-frame playback multiplier for the transition motion. */
void Hu3DMotionShiftSpeedSet(HU3D_MODELID modelId, float speed)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->motShiftWork.speed = speed;
}

/* Sets requested current transform components to their base values and blocks track writes; a
 * base-transform reset can still overwrite forced scales. */
void Hu3DMotionNoMotSet(HU3D_MODELID modelId, char *objName, u32 forceAttr)
{
    HSF_OBJECT *objP = Hu3DModelObjPtrGet(modelId, objName);
    HSF_CONSTDATA *constData;
    if(!objP->constData) {
        constData = ObjConstantMake(objP, Hu3DData[modelId].mallocNo);
    } else {
        constData = objP->constData;
    }
    constData->attr |= forceAttr;
    if(forceAttr & HU3D_CONST_FORCE_POSX) {
        objP->mesh.curr.pos.x = objP->mesh.base.pos.x;
    }
    if(forceAttr & HU3D_CONST_FORCE_POSY) {
        objP->mesh.curr.pos.y = objP->mesh.base.pos.y;
    }
    if(forceAttr & HU3D_CONST_FORCE_POSZ) {
        objP->mesh.curr.pos.z = objP->mesh.base.pos.z;
    }
    if(forceAttr & HU3D_CONST_FORCE_ROTX) {
        objP->mesh.curr.rot.x = objP->mesh.base.rot.x;
    }
    if(forceAttr & HU3D_CONST_FORCE_ROTY) {
        objP->mesh.curr.rot.y = objP->mesh.base.rot.y;
    }
    if(forceAttr & HU3D_CONST_FORCE_ROTZ) {
        objP->mesh.curr.rot.z = objP->mesh.base.rot.z;
    }
    if(forceAttr & HU3D_CONST_FORCE_SCALEX) {
        objP->mesh.curr.scale.x = objP->mesh.base.scale.x;
    }
    if(forceAttr & HU3D_CONST_FORCE_SCALEY) {
        objP->mesh.curr.scale.y = objP->mesh.base.scale.y;
    }
    if(forceAttr & HU3D_CONST_FORCE_SCALEZ) {
        objP->mesh.curr.scale.z = objP->mesh.base.scale.z;
    }
}

/* Releases the requested forced transform components on the selected object. */
void Hu3DMotionNoMotReset(HU3D_MODELID modelId, char *objName, u32 forceAttr)
{
    HSF_OBJECT *objP = Hu3DModelObjPtrGet(modelId, objName);
    HSF_CONSTDATA *constData = objP->constData;
    constData->attr &= ~forceAttr;
}

/* Sets selected current transform components to value and blocks their animation-track writes.
 * A base-transform reset can still overwrite forced scale components. */
void Hu3DMotionForceSet(HU3D_MODELID modelId, char *objName, u32 forceAttr, float value)
{
    HSF_OBJECT *objP = Hu3DModelObjPtrGet(modelId, objName);
    HSF_CONSTDATA *constData;
    if(!objP->constData) {
        constData = ObjConstantMake(objP, Hu3DData[modelId].mallocNo);
    } else {
        constData = objP->constData;
    }
    constData->attr |= forceAttr;
    if(forceAttr & HU3D_CONST_FORCE_POSX) {
        objP->mesh.curr.pos.x = value;
    }
    if(forceAttr & HU3D_CONST_FORCE_POSY) {
        objP->mesh.curr.pos.y = value;
    }
    if(forceAttr & HU3D_CONST_FORCE_POSZ) {
        objP->mesh.curr.pos.z = value;
    }
    if(forceAttr & HU3D_CONST_FORCE_ROTX) {
        objP->mesh.curr.rot.x = value;
    }
    if(forceAttr & HU3D_CONST_FORCE_ROTY) {
        objP->mesh.curr.rot.y = value;
    }
    if(forceAttr & HU3D_CONST_FORCE_ROTZ) {
        objP->mesh.curr.rot.z = value;
    }
    if(forceAttr & HU3D_CONST_FORCE_SCALEX) {
        objP->mesh.curr.scale.x = value;
    }
    if(forceAttr & HU3D_CONST_FORCE_SCALEY) {
        objP->mesh.curr.scale.y = value;
    }
    if(forceAttr & HU3D_CONST_FORCE_SCALEZ) {
        objP->mesh.curr.scale.z = value;
    }
}

/* Stores the callback and selects the first object whose name contains NOW; if none exists,
 * timingHookObj is left unchanged. */
void Hu3DMotionTimingHookSet(HU3D_MODELID modelId, HU3D_TIMING_HOOK timingHook)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
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
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->timingHook = NULL;
}

/* Adds motion-slot attributes used by playback and motion evaluation. */
void Hu3DMotionAttrSet(HU3D_MOTIONID motId, u16 attr)
{
    HU3D_MOTION *motP = &Hu3DMotion[motId];
    motP->attr |= attr;
}

/* Removes selected attributes from a motion slot. */
void Hu3DMotionAttrReset(HU3D_MOTIONID motId, u16 attr)
{
    HU3D_MOTION *motP = &Hu3DMotion[motId];
    motP->attr &= ~attr;
}

/* Called from the HSF manager's per-frame update to advance active motion clocks for one model. */
void Hu3DMotionNext(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HU3D_MOTION *motP = &Hu3DMotion[modelP->motId];
    u32 attr = modelP->motAttr;
    HSF_MOTION *hsfMotP;

    if(modelP->motId != HU3D_MOTIONID_NONE) {
        hsfMotP = motP->hsf->motion;
        motP = &Hu3DMotion[modelP->motId];
        if(!(attr & HU3D_MOTATTR_PAUSE)) {
            modelP->attr |= HU3D_ATTR_MOTION_MODEL;
            if(!(attr & HU3D_MOTATTR_REV)) {
                modelP->motWork.time += modelP->motWork.speed*minimumVcountf;
            } else {
                modelP->motWork.time -= modelP->motWork.speed*minimumVcountf;
            }
            /* Main-motion loops preserve overshoot with one wrap. Overlay, shift, shape,
            * and cluster loops instead snap to the opposite endpoint. */
            if(attr & HU3D_MOTATTR_LOOP) {
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
    if(modelP->motIdOvl != HU3D_MOTIONID_NONE) {
        motP = &Hu3DMotion[modelP->motIdOvl];
        hsfMotP = motP->hsf->motion;
        if(!(attr & HU3D_MOTATTR_OVL_PAUSE)) {
            modelP->attr |= HU3D_ATTR_MOTION_MODEL;
            if(!(attr & HU3D_MOTATTR_OVL_REV)) {
                modelP->motOvlWork.time += modelP->motOvlWork.speed*minimumVcountf;
            } else {
                modelP->motOvlWork.time -= modelP->motOvlWork.speed*minimumVcountf;
            }
            if(attr & HU3D_MOTATTR_OVL_LOOP) {
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
    if(modelP->motIdShift != HU3D_MOTIONID_NONE) {
        modelP->attr |= HU3D_ATTR_MOTION_MODEL;
        if(!(attr & HU3D_MOTATTR_EXECSHIFT)) {
            modelP->motOvlWork.start += minimumVcountf;
            if(modelP->motOvlWork.start >= modelP->motOvlWork.end) {
                modelP->motId = modelP->motIdShift;
                modelP->motWork.time = modelP->motShiftWork.time;
                modelP->motWork.speed = modelP->motShiftWork.speed;
                modelP->motWork.start = modelP->motShiftWork.start;
                modelP->motWork.end = modelP->motShiftWork.end;
                modelP->motIdShift = HU3D_MOTIONID_NONE;
                /* Converted shift flags include the HU3D_MOTATTR tag, so any promoted flag makes
                * the shape-pause test below succeed. With none, shape advances forward without
                * looping, ignoring its own pause, reverse, and loop flags for this call. */
                attr = HU3D_MOTATTR_NONE;
                if(modelP->motAttr & HU3D_MOTATTR_SHIFT_LOOP) {
                    attr |= HU3D_MOTATTR_LOOP;
                }
                if(modelP->motAttr & HU3D_MOTATTR_SHIFT_PAUSE) {
                    attr |= HU3D_MOTATTR_PAUSE;
                }
                if(modelP->motAttr & HU3D_MOTATTR_SHIFT_REV) {
                    attr |= HU3D_MOTATTR_REV;
                }
                modelP->motAttr &= ~HU3D_MOTATTR_ALL;
                modelP->motAttr |= attr;
                modelP->motAttr &= ~HU3D_MOTATTR;
                goto end_shift;
            }
        }
        if(!(modelP->motAttr & HU3D_MOTATTR_SHIFT_PAUSE)) {
            motP = &Hu3DMotion[modelP->motIdShift];
            if(!(modelP->motAttr & HU3D_MOTATTR_SHIFT_REV)) {
                modelP->motShiftWork.time += modelP->motShiftWork.speed*minimumVcountf;
            } else {
                modelP->motShiftWork.time -= modelP->motShiftWork.speed*minimumVcountf;
            }
            if(modelP->motAttr & HU3D_MOTATTR_SHIFT_LOOP) {
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
    if(modelP->motIdShape != HU3D_MOTIONID_NONE && !(attr & HU3D_MOTATTR_SHAPE_PAUSE)) {
        modelP->attr |= HU3D_ATTR_MOTION_MODEL;
        motP = &Hu3DMotion[modelP->motIdShape];
        hsfMotP = motP->hsf->motion;
        if(!(attr & HU3D_MOTATTR_SHAPE_REV)) {
            modelP->motShapeWork.time += modelP->motShapeWork.speed*minimumVcountf;
        } else {
            modelP->motShapeWork.time -= modelP->motShapeWork.speed*minimumVcountf;
        }
        if(attr & HU3D_MOTATTR_SHAPE_LOOP) {
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
    if(modelP->attr & HU3D_ATTR_CLUSTER_ON) {
        s16 i;
        for(i=0; i<HU3D_CLUSTER_MAX; i++) {
            if(modelP->motIdCluster[i] == HU3D_MOTIONID_NONE) {
                continue;
            }
            if(modelP->clusterAttr[i] & HU3D_CLUSTER_ATTR_PAUSE) {
                continue;
            }
            modelP->attr |= HU3D_ATTR_MOTION_MODEL;
            motP = &Hu3DMotion[modelP->motIdCluster[i]];
            hsfMotP = motP->hsf->motion;
            if(!(modelP->clusterAttr[i] & HU3D_CLUSTER_ATTR_REV)) {
                modelP->clusterTime[i] += modelP->clusterSpeed[i]*minimumVcountf;
            } else {
                modelP->clusterTime[i] -= modelP->clusterSpeed[i]*minimumVcountf;
            }
            if(modelP->clusterAttr[i] & HU3D_CLUSTER_ATTR_LOOP) {
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

#define HU3D_CONST_FORCE_POS (HU3D_CONST_FORCE_POSX|HU3D_CONST_FORCE_POSY|HU3D_CONST_FORCE_POSZ)
#define HU3D_CONST_FORCE_ROT (HU3D_CONST_FORCE_ROTX|HU3D_CONST_FORCE_ROTY|HU3D_CONST_FORCE_ROTZ)
#define HU3D_CONST_FORCE_SCALE                                                                     \
    (HU3D_CONST_FORCE_SCALEX | HU3D_CONST_FORCE_SCALEY | HU3D_CONST_FORCE_SCALEZ)

#define HU3D_CONST_FORCE_ANY (HU3D_CONST_FORCE_POS|HU3D_CONST_FORCE_ROT|HU3D_CONST_FORCE_SCALE)

static BOOL MLoopCheck(HSF_OBJECT *objPtr);
static BOOL TimingHookCheck(HU3D_MODELID modelId, HU3D_MOTIONID motId, HSF_OBJECT *objP);

#define OBJ_TRX_NONE ((float *)-1)

float *GetObjTRXPtr(HSF_OBJECT *objPtr, u16 channel);

/* Evaluates one motion's tracks into a model during Hu3DMotionCalc. */
void Hu3DMotionExec(HU3D_MODELID modelId, HU3D_MOTIONID motId, float time, BOOL preserveXFormF)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HU3D_MOTION *motP = &Hu3DMotion[motId];
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
                    if(!(forceAttr & HU3D_CONST_FORCE_POSX)) {
                        objP->mesh.curr.pos.x = objP->mesh.base.pos.x;
                    }
                    if(!(forceAttr & HU3D_CONST_FORCE_POSY)) {
                        objP->mesh.curr.pos.y = objP->mesh.base.pos.y;
                    }
                    if(!(forceAttr & HU3D_CONST_FORCE_POSZ)) {
                        objP->mesh.curr.pos.z = objP->mesh.base.pos.z;
                    }
                    if(!(forceAttr & HU3D_CONST_FORCE_ROTX)) {
                        objP->mesh.curr.rot.x = objP->mesh.base.rot.x;
                    }
                    if(!(forceAttr & HU3D_CONST_FORCE_ROTY)) {
                        objP->mesh.curr.rot.y = objP->mesh.base.rot.y;
                    }
                    if(!(forceAttr & HU3D_CONST_FORCE_ROTZ)) {
                        objP->mesh.curr.rot.z = objP->mesh.base.rot.z;
                    }
                    if(!(forceAttr & HU3D_CONST_FORCE_SCALEX)) {
                        objP->mesh.curr.scale.x = objP->mesh.base.scale.x;
                    }
                    if(!(forceAttr & HU3D_CONST_FORCE_SCALEY)) {
                        objP->mesh.curr.scale.y = objP->mesh.base.scale.y;
                    }
                    if(!(forceAttr & HU3D_CONST_FORCE_SCALEZ)) {
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

            case HSF_TRACK_TRANSFORM:
                if(trackP->target < hsf->objectNum && trackP->target != -1) {
                    objP = &hsf->object[trackP->target];
                    if(trackP->channel == HSF_CHANNEL_MORPH) {
                        objP->mesh.mesh.baseMorph = GetCurve(trackP, time);
                    } else if(objP->type == HSF_OBJ_CAMERA) {
                        if(modelP->attr & HU3D_ATTR_CAMERA_MOTON) {
                            SetObjCameraMotion(modelId, trackP, GetCurve(trackP, time));
                        }
                    } else if(objP->type == HSF_OBJ_LIGHT) {
                        SetObjLightMotion(modelId, trackP, GetCurve(trackP, time));
                    } else if(trackP->channel == HSF_CHANNEL_LOCK) {
                        /* A 1.0 lock key can move the main-motion loop start, even in an overlay or
                         * shape pass, and invoke the NOW hook. Invoking the hook leaves the lock
                         * unchanged; otherwise 1.0 releases it and other values lock it. */
                        if ((motP->attr & HU3D_ATTR_MOT_RESET_LOCK) &&
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
                                    constData->attr &= ~HU3D_CONST_LOCK;
                                } else {
                                    constData->attr |= HU3D_CONST_LOCK;
                                }
                            }
                        }
                    } else if(trackP->channel == HSF_CHANNEL_DISPOFF) {
                        /* Value 1.0 clears the runtime display-off flag; every other value sets
                         * it. */
                        if(objP->constData) {
                            constData = objP->constData;
                            if(GetCurve(trackP, time) == 1.0f) {
                                constData->attr &= ~HU3D_CONST_DISPOFF;
                            } else {
                                constData->attr |= HU3D_CONST_DISPOFF;
                            }
                        }
                    } else {
                        float *trxP = GetObjTRXPtr(objP, trackP->channel);
                        if(trxP != OBJ_TRX_NONE) {
                            *trxP = GetCurve(trackP, time);
                        }
                    }
                }
                break;

            case HSF_TRACK_MORPH:
            {
                HSF_TRACK *track = trackP;
                if(track->target < hsf->objectNum) {
                    objP = &hsf->object[track->target];
                    objP->mesh.mesh.morphWeight[track->morphWeight] = GetCurve(track, time);

                }
            }
                break;

            case HSF_TRACK_MATERIAL:
                if(!(modelP->attr & HU3D_ATTR_CURVE_MOTOFF) && trackP->attrIdx < hsf->materialNum) {
                    SetObjMatMotion(modelId, trackP, GetCurve(trackP, time));
                }
                break;

            case HSF_TRACK_CLUSTER:
                if (!(modelP->attr & HU3D_ATTR_CURVE_MOTOFF)) {
                    clusterP = &hsf->cluster[trackP->cluster];
                    clusterP->index = GetClusterCurve(trackP, time);
                }
                break;

            case HSF_TRACK_CLUSTER_WEIGHT:
                if (!(modelP->attr & HU3D_ATTR_CURVE_MOTOFF)) {
                    HSF_TRACK *track = trackP;
                    clusterP = &hsf->cluster[track->cluster];
                    clusterP->weight[track->clusterWeight] = GetClusterWeightCurve(track, time);
                }
                break;

            case HSF_TRACK_ATTRIBUTE:
            {
                HSF_TRACK *track = trackP;
                /* Tracks whose cluster field differs from -1 bypass HU3D_ATTR_CURVE_MOTOFF;
                * that flag blocks only attribute tracks with cluster == -1. */
                if(track->cluster != -1 || !(modelP->attr & HU3D_ATTR_CURVE_MOTOFF)) {
                    if(track->attrIdx != -1 && track->attrIdx < hsf->attributeNum) {
                        SetObjAttrMotion(modelId, track, GetCurve(track, time));
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
            HU3D_MODEL *modelP = &Hu3DData[modelId];
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
void Hu3DCameraMotionExec(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HU3D_MOTION *motP = &Hu3DMotion[modelP->motId];
    HSF_DATA *hsf = motP->hsf;
    HSF_MOTION *hsfMotP = hsf->motion;
    HSF_TRACK *trackP = hsfMotP->track;
    if(modelP->attr & HU3D_ATTR_CAMERA_MOTON) {
        HSF_TRACK *trackPEnd;
        for(trackPEnd = &trackP[hsfMotP->numTracks]; trackP < trackPEnd; trackP++) {
            if(trackP->type == HSF_TRACK_TRANSFORM && trackP->index == HSF_OBJ_CAMERA) {
                SetObjCameraMotion(modelId, trackP, GetCurve(trackP, modelP->motWork.time));
            }
        }
    }
}

/* Blends pending transform tracks into current transforms; if no main motion is set,
* first resets every object to its base transform, including forced components. */
void Hu3DSubMotionExec(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HU3D_MOTION *motP = &Hu3DMotion[modelP->motIdShift];
    HSF_DATA *hsf = modelP->hsf;
    HSF_DATA *motHsf = motP->hsf;
    HSF_MOTION *hsfMotP = motHsf->motion;
    HSF_TRACK *trackP = hsfMotP->track;
    HSF_OBJECT *objPtr = hsf->object;
    HSF_OBJECT *obj;
    s16 i;
    float weight;
    float invWeight;
    if(modelP->motId == HU3D_MOTIONID_NONE) {
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
            case HSF_TRACK_TRANSFORM:
                if (trackP->target < hsf->objectNum && trackP->target != -1) {
                    s16 channel;
                    float *trxP;
                    obj = &hsf->object[trackP->target];
                    channel = trackP->channel;
                    if(channel == HSF_CHANNEL_LOCK) {
                        /* Sub-motion lock tracks can move the shift-loop start and invoke the NOW
                         * hook, but do not change the object's lock flag. */
                        if ((motP->attr & HU3D_ATTR_MOT_RESET_LOCK) &&
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
                        float *trxP = GetObjTRXPtr(obj, channel);
                        if(trxP != OBJ_TRX_NONE) {
                            if (channel == HSF_CHANNEL_ROTX || channel == HSF_CHANNEL_ROTY ||
                                channel == HSF_CHANNEL_ROTZ) {
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
float *GetObjTRXPtr(HSF_OBJECT *objPtr, u16 channel)
{
    HSF_CONSTDATA *constData = objPtr->constData;
    if(channel <= HSF_CHANNEL_POSZ) {
        if(channel == HSF_CHANNEL_POSX) {
            if(constData && (constData->attr & HU3D_CONST_FORCE_POSX)) {
                return OBJ_TRX_NONE;
            }
            return &objPtr->mesh.curr.pos.x;
        } else if(channel == HSF_CHANNEL_POSY) {
            if(constData && (constData->attr & HU3D_CONST_FORCE_POSY)) {
                return OBJ_TRX_NONE;
            }
            return &objPtr->mesh.curr.pos.y;
        } else if(channel == HSF_CHANNEL_POSZ) {
            if(constData && (constData->attr & HU3D_CONST_FORCE_POSZ)) {
                return OBJ_TRX_NONE;
            }
            return &objPtr->mesh.curr.pos.z;
        }
    } else if(channel <= HSF_CHANNEL_ROTZ) {
        if(channel == HSF_CHANNEL_ROTX) {
            if(constData && (constData->attr & HU3D_CONST_FORCE_ROTX)) {
                return OBJ_TRX_NONE;
            }
            return &objPtr->mesh.curr.rot.x;
        } else if(channel == HSF_CHANNEL_ROTY) {
            if(constData && (constData->attr & HU3D_CONST_FORCE_ROTY)) {
                return OBJ_TRX_NONE;
            }
            return &objPtr->mesh.curr.rot.y;
        } else if(channel == HSF_CHANNEL_ROTZ) {
            if(constData && (constData->attr & HU3D_CONST_FORCE_ROTZ)) {
                return OBJ_TRX_NONE;
            }
            return &objPtr->mesh.curr.rot.z;
        }
    } else if(channel >= HSF_CHANNEL_SCALEX && channel <= HSF_CHANNEL_SCALEZ) {
        if(channel == HSF_CHANNEL_SCALEX) {
            if(constData && (constData->attr & HU3D_CONST_FORCE_SCALEX)) {
                return OBJ_TRX_NONE;
            }
            return &objPtr->mesh.curr.scale.x;
        } else if(channel == HSF_CHANNEL_SCALEY) {
            if(constData && (constData->attr & HU3D_CONST_FORCE_SCALEY)) {
                return OBJ_TRX_NONE;
            }
            return &objPtr->mesh.curr.scale.y;
        } else if(channel == HSF_CHANNEL_SCALEZ) {
            if(constData && (constData->attr & HU3D_CONST_FORCE_SCALEZ)) {
                return OBJ_TRX_NONE;
            }
            return &objPtr->mesh.curr.scale.z;
        }
    }
    return OBJ_TRX_NONE;
}

/* Applies a material color or alpha track after clamping its value to [0, 1].
* Inverse-alpha tracks are ignored while HU3D_ATTR_TPLVL_SET is active. */
void SetObjMatMotion(HU3D_MODELID modelId, HSF_TRACK *trackP, float value)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HSF_DATA *hsf = modelP->hsf;
    HSF_MATERIAL *matP = &hsf->material[trackP->attrIdx];
    float weight = value;
    if(value > 1) {
        weight = 1;
    } else if(value < 0) {
        weight = 0;
    }
    switch(trackP->channel) {
        case HSF_CHANNEL_LITCOLOR_R:
            matP->litColor[0] = weight*255;
            break;

        case HSF_CHANNEL_LITCOLOR_G:
            matP->litColor[1] = weight*255;
            break;

        case HSF_CHANNEL_LITCOLOR_B:
            matP->litColor[2] = weight*255;
            break;

        case HSF_CHANNEL_COLOR_R:
            matP->color[0] = weight*255;
            break;

        case HSF_CHANNEL_COLOR_G:
            matP->color[1] = weight*255;
            break;

        case HSF_CHANNEL_COLOR_B:
            matP->color[2] = weight*255;
            break;

        case HSF_CHANNEL_SHADOWCOLOR_R:
            matP->shadowColor[0] = weight*255;
            break;

        case HSF_CHANNEL_SHADOWCOLOR_G:
            matP->shadowColor[1] = weight*255;
            break;

        case HSF_CHANNEL_SHADOWCOLOR_B:
            matP->shadowColor[2] = weight*255;
            break;

        case HSF_CHANNEL_INVALPHA:
            if(!(modelP->attr & HU3D_ATTR_TPLVL_SET)) {
                matP->invAlpha = weight;
            }
            break;

        case HSF_CHANNEL_REFALPHA:
            matP->refAlpha = weight;
            break;
    }
}

/* Applies a texture-attribute track, creating per-model animation work when needed. */
void SetObjAttrMotion(HU3D_MODELID modelId, HSF_TRACK *trackP, float value)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
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
        case HSF_CHANNEL_POSX:
        case HSF_CHANNEL_POSY:
        case HSF_CHANNEL_POSZ:
        case HSF_CHANNEL_ROTX:
        case HSF_CHANNEL_ROTY:
        case HSF_CHANNEL_ROTZ:
        case HSF_CHANNEL_SCALEX:
        case HSF_CHANNEL_SCALEY:
        case HSF_CHANNEL_SCALEZ:
        case HSF_CHANNEL_BITMAP:
            if(!attrP->animWorkP) {
                if(modelP->linkMdlId != HU3D_MODELID_NONE) {
                    attrAnimP = HuMemDirectMallocNum(HEAP_MODEL, sizeof(HU3D_ATTR_ANIM),
                                                     Hu3DData[modelId].mallocNoLink);
                    attrP->animWorkP = attrAnimP;
                } else {
                    attrAnimP = HuMemDirectMallocNum(HEAP_MODEL, sizeof(HU3D_ATTR_ANIM),
                                                     Hu3DData[modelId].mallocNo);
                    attrP->animWorkP = attrAnimP;
                }

                attrAnimP->attr = 0;
                attrAnimP->trans3D.x = attrAnimP->trans3D.y = attrAnimP->trans3D.z = 0;
                attrAnimP->rot.x = attrAnimP->rot.y = attrAnimP->rot.z = 0;
                attrAnimP->scale3D.x = attrAnimP->scale3D.y = attrAnimP->scale3D.z = 1;
            } else {
                attrAnimP = attrP->animWorkP;
            }
            if(trackP->channel != HSF_CHANNEL_BITMAP) {
                attrAnimP->attr |= HU3D_ATTRANIM_ATTR_ANIM3D;
            } else {
                attrAnimP->attr |= HU3D_ATTRANIM_ATTR_BMPANIM;
            }
            break;
    }
    switch(trackP->channel) {
        case HSF_CHANNEL_POSX:
            attrAnimP->trans3D.x = value;
            break;

        case HSF_CHANNEL_POSY:
            attrAnimP->trans3D.y = value;
            break;

        case HSF_CHANNEL_POSZ:
            attrAnimP->trans3D.z = value;
            break;

        case HSF_CHANNEL_ROTX:
            attrAnimP->rot.x = value;
            break;

        case HSF_CHANNEL_ROTY:
            attrAnimP->rot.y = value;
            break;

        case HSF_CHANNEL_ROTZ:
            attrAnimP->rot.z = value;
            break;

        case HSF_CHANNEL_SCALEX:
            attrAnimP->scale3D.x = value;
            break;

        case HSF_CHANNEL_SCALEY:
            attrAnimP->scale3D.y = value;
            break;

        case HSF_CHANNEL_SCALEZ:
            attrAnimP->scale3D.z = value;
            break;

        case HSF_CHANNEL_64:
            /* hsfdraw checks for exactly 1.0 to enable animated texture-matrix handling. */
            attrP->texMtxAnimEnabled = weight;
            break;

        case HSF_CHANNEL_KCOLOR:
            attrP->kColor = weight;
            break;

        case HSF_CHANNEL_NBT_TPLVL:
            attrP->nbtTpLvl = value;
            break;

        case HSF_CHANNEL_BITMAP:
            attrAnimP->bitMapPtr = bitMapPtr;
            break;
    }
}

/* Applies one camera track to every camera selected by the model's camera mask. */
void SetObjCameraMotion(HU3D_MODELID modelId, HSF_TRACK *trackP, float value)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    s16 cameraBit = modelP->camInfoBit;
    s16 i;
    s16 bit;
    float newValue;
    float weight;
    if(cameraBit == HU3D_CAM_NONE) {
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
        case HSF_CHANNEL_POSX:
            newValue = modelP->scale.x*(value+modelP->pos.x);
            for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
                if(bit & cameraBit) {
                    Hu3DCamera[i].pos.x = newValue;
                }
            }
            break;

        case HSF_CHANNEL_POSY:
            newValue = modelP->scale.y*(value+modelP->pos.y);
            for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
                if(bit & cameraBit) {
                    Hu3DCamera[i].pos.y = newValue;
                }
            }
            break;

        case HSF_CHANNEL_POSZ:
            newValue = modelP->scale.z*(value+modelP->pos.z);
            for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
                if(bit & cameraBit) {
                    Hu3DCamera[i].pos.z = newValue;
                }
            }
            break;

         case HSF_CHANNEL_TARGETX:
            newValue = modelP->scale.x*(value+modelP->pos.x);
            for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
                if(bit & cameraBit) {
                    Hu3DCamera[i].target.x = newValue;
                }
            }
            break;

        case HSF_CHANNEL_TARGETY:
            newValue = modelP->scale.y*(value+modelP->pos.y);
            for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
                if(bit & cameraBit) {
                    Hu3DCamera[i].target.y = newValue;
                }
            }
            break;

        case HSF_CHANNEL_TARGETZ:
            newValue = modelP->scale.z*(value+modelP->pos.z);
            for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
                if(bit & cameraBit) {
                    Hu3DCamera[i].target.z = newValue;
                }
            }
            break;

        /* Rotates world-up around the normalized target-to-camera direction by value degrees,
        * then normalizes the result for the camera's up vector. */
        case HSF_CHANNEL_UPROT:
            for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
                if(bit & cameraBit) {
                    HuVecF upOfs;
                    HuVecF dir;
                    VECSubtract(&Hu3DCamera[i].pos, &Hu3DCamera[i].target, &dir);
                    HuNormVecF(&dir, &dir);
                    upOfs.x = ((dir.x * dir.y * (1-HuCos(value))) - (dir.z * HuSin(value)));
                    upOfs.y = ((dir.y * dir.y) + (1-HuSquare(dir.y)) * HuCos(value));
                    upOfs.z = (((dir.y * dir.z) * (1-HuCos(value))) + (dir.x * HuSin(value)));
                    HuNormVecF(&upOfs, &Hu3DCamera[i].up);
                    Hu3DCamera[i].upRot = value;
                }
            }
            break;

        case HSF_CHANNEL_FOV:
            for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
                if(bit & cameraBit) {
                    Hu3DCamera[i].fov = value;
                }
            }
            break;

        case HSF_CHANNEL_NEAR:
            for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
                if(bit & cameraBit) {
                    Hu3DCamera[i].near = value;
                }
            }
            break;

        case HSF_CHANNEL_FAR:
            for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
                if(bit & cameraBit) {
                    Hu3DCamera[i].far = value;
                }
            }
            break;
    }
}

/* Applies one light track to the light corresponding to its HSF object target. */
void SetObjLightMotion(HU3D_MODELID modelId, HSF_TRACK *trackP, float value)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HSF_DATA *hsf = modelP->hsf;
    HSF_OBJECT *objPtr = hsf->object;
    HU3D_LIGHT *lightP;
    float weight;
    s16 i;
    s16 lightId;
    lightId = 0;
    for(i=lightId; i<hsf->objectNum; i++, objPtr++) {
        HSF_OBJECT *obj = objPtr;
        if(obj->type == HSF_OBJ_LIGHT) {
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
        case HSF_CHANNEL_LIGHTCOLOR_R:
            lightP->color.r = 255.0f*weight;
            break;

        case HSF_CHANNEL_LIGHTCOLOR_G:
            lightP->color.g = 255.0f*weight;
            break;

        case HSF_CHANNEL_LIGHTCOLOR_B:
            lightP->color.b = 255.0f*weight;
            break;

        case HSF_CHANNEL_POSX:
            lightP->pos.x = value;
            break;

        case HSF_CHANNEL_POSY:
            lightP->pos.y = value;
            break;

        case HSF_CHANNEL_POSZ:
            lightP->pos.z = value;
            break;

        case HSF_CHANNEL_TARGETX:
            lightP->offset.x = value;
            Hu3DGLightPosAimSetV(modelP->lightId[lightId], &lightP->pos, &lightP->offset);
            break;

        case HSF_CHANNEL_TARGETY:
            lightP->offset.y = value;
            Hu3DGLightPosAimSetV(modelP->lightId[lightId], &lightP->pos, &lightP->offset);
            break;

        case HSF_CHANNEL_TARGETZ:
            lightP->offset.z = value;
            Hu3DGLightPosAimSetV(modelP->lightId[lightId], &lightP->pos, &lightP->offset);
            break;
    }
}

/* Samples the selected curve type; bitmap tracks update bitMapPtr and return zero here. */
float GetCurve(HSF_TRACK *trackP, float time)
{
    switch (trackP->curveType) {
        case HSF_CURVE_LINEAR:
            return GetLinear(trackP->numKeyframes, trackP->data, time);

        case HSF_CURVE_BEZIER:
            return GetBezier(trackP->numKeyframes, trackP, time);

        case HSF_CURVE_BITMAP:
            bitMapPtr = GetBitMap(trackP->numKeyframes, trackP->data, time);
            break;
        case HSF_CURVE_CONST:
        {
            float *data = (float *)&trackP->data;
            return *data;
        }

        case HSF_CURVE_STEP:
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
HU3D_MOTIONID Hu3DJointMotion(HU3D_MODELID modelId, void *data)
{
    HU3D_MOTIONID motId = Hu3DMotionCreate(data);
    JointModel_Motion(modelId, motId);
    return motId;
}

static s32 SearchObjectIndex(HSF_DATA *hsf, u32 nameOffset);
static s32 SearchAttributeIndex(HSF_DATA *hsf, u32 nameOffset);

/* Resolves transform/morph object targets and missing attribute indices when attaching a motion. */
void JointModel_Motion(HU3D_MODELID modelId, HU3D_MOTIONID motId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HU3D_MOTION *motP = &Hu3DMotion[motId];
    HSF_DATA *hsf = modelP->hsf;
    HSF_DATA *motHsf = motP->hsf;
    HSF_MOTION *hsfMotP = motHsf->motion;
    HSF_TRACK *trackP = hsfMotP->track;
    s32 i;
    for(i=0; i<hsfMotP->numTracks; i++, trackP++) {
        switch (trackP->type) {
            case HSF_TRACK_TRANSFORM:
            {
                HSF_TRACK *track = trackP;
                track->target = SearchObjectIndex(hsf, track->target);
            }
                break;

            case HSF_TRACK_MORPH:
            {
                HSF_TRACK *track = trackP;
                track->target = SearchObjectIndex(hsf, track->target);
            }
                break;

            case HSF_TRACK_ATTRIBUTE:
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
void Hu3DMotionCalc(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    if ((modelP->attr & HU3D_ATTR_DISPOFF) || (modelP->attr & HU3D_ATTR_HOOK)) {
        return;
    }
    if(modelP->motId != HU3D_MOTIONID_NONE) {
        Hu3DMotionExec(modelId, modelP->motId, modelP->motWork.time, FALSE);
    }
    if(modelP->motIdShift != HU3D_MOTIONID_NONE) {
        Hu3DSubMotionExec(modelId);
    }
    if(modelP->motIdOvl != HU3D_MOTIONID_NONE) {
        Hu3DMotionExec(modelId, modelP->motIdOvl, modelP->motOvlWork.time, TRUE);
    }
    if(modelP->attr & HU3D_ATTR_CLUSTER_ON) {
        ClusterMotionExec(modelP);
    }
    if(modelP->motIdShape != HU3D_MOTIONID_NONE) {
        /* With no main motion, shape evaluation resets base transforms after any
         * sub-motion and overlay passes, discarding their unforced transform results. */
        if(modelP->motId == HU3D_MOTIONID_NONE) {
            Hu3DMotionExec(modelId, modelP->motIdShape, modelP->motShapeWork.time, FALSE);
        } else {
            Hu3DMotionExec(modelId, modelP->motIdShape, modelP->motShapeWork.time, TRUE);
        }
    }
    if ((modelP->attr & (HU3D_ATTR_ENVELOPE_OFF|HU3D_ATTR_HOOKFUNC)) == 0) {
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
void Hu3DSubMotionSet(HU3D_MODELID modelId, HU3D_MOTIONID motId, float blendWeight)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];

    modelP->motIdShift = motId;
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_EXECSHIFT);
    modelP->motOvlWork.start = 100.0*blendWeight;
    modelP->motOvlWork.end = 100;
    modelP->motShiftWork.speed = 1;
    modelP->motShiftWork.start = 0;
    modelP->motShiftWork.end = Hu3DMotionMotionMaxTimeGet(motId);
}

/* Cancels the pending sub-motion and clears its execution flag. */
void Hu3DSubMotionReset(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->motIdShift = HU3D_MOTIONID_NONE;
    Hu3DModelAttrReset(modelId, HU3D_MOTATTR_EXECSHIFT);
}

/* Sets the blend weight from a fraction; playback holds it while EXECSHIFT is set. */
void Hu3DSubMotionTimeSet(HU3D_MODELID modelId, float blendWeight)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->motOvlWork.start = 100.0*blendWeight;
    modelP->motOvlWork.end = 100;
}

/* Sets playback time for the motion currently being blended in. */
void Hu3DMotionShiftTimeSet(HU3D_MODELID modelId, float time)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    modelP->motShiftWork.time = time;
}
