/* Scene props, course paths, animation and opening sequences. */
#include "REL/m676Dll/effect_types.h"
#include <math.h>
#include "dolphin/gx.h"
#include "game/object.h"
#include "game/hsfex.h"
#include "game/memory.h"
#include "game/hu3d.h"
#include <string.h>
#include "REL/m676Dll/prop_types.h"
#include "REL/m676Dll/scene_types.h"
#include "game/main.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include "msm_se.h"
#include "dolphin/mtx.h"
#include "REL/m676Dll/particle_types.h"
#include "dolphin/types.h"

#define M676_COLLISION_MASK_PROP 19
#define M676_COLLISION_MASK_PARTICLE 16

typedef struct M676SceneManagerWork {
    /* These zeroed fields are not read by the scene manager callback. */
    s32 reservedA;
    s32 reservedB;
    /* Number of times the manager update callback has run. */
    s32 updateCount;
} M676SceneManagerWork;

s32 fn_1_1480(s32 resource);
int fn_1_50A8(int resource);
s32 fn_1_51B0(s32 model, s32 resource);

extern void fn_1_1263C(s32 playerIndex, f32 *x, f32 *y);
extern void fn_1_12514(s32 playerIndex, s32 amount);
extern void fn_1_4754(s32 resource, Vec *position);
extern void fn_1_5698(Vec *start, Vec *end, f32 factor, Vec *result);

extern f32 fn_1_52C0(f32 start, f32 end, f32 speed);
extern f32 fn_1_53DC(f32 start, f32 end);

extern s32 fn_1_3DD0(Vec *position);

extern s32 fn_1_13F54(void *registration);

extern f32 fn_1_5490(Vec *start, Vec *end);
extern u32 MgSeqModeGet(void);

extern int fn_1_1650(void);

extern u32 MgSeqModeGet(void);

extern u32 MgSeqModeGet(void);

extern s32 fn_1_56F0(Vec *start, Vec *end, f32 delta, f32 step,
    Vec *velocity);

s32 fn_1_1498(void);

extern s32 fn_1_13EC4(M676CollisionEntry *registration);

extern int fn_1_15E0(void);
extern s32 fn_1_49C8(s32 effect, Vec *position, s32 panEnabled, s32 volumeEnabled);
extern void fn_1_10288(s32 type, s32 index, Vec *position,
                      s32 requestValue);

extern f32 fn_1_552C(Vec *first, Vec *second);
void fn_1_8888(s32 playerIndex);
void fn_1_88C0(HUPROCESS *process);
void fn_1_89F0(void);
void fn_1_89F4(OMOBJ *obj);
void fn_1_8A04(OMOBJ *owner);
void fn_1_8A58(HUPROCESS *process, M676PropParam *propParameters);
void fn_1_8D18(OMOBJ *object);
OMOBJ *fn_1_8D6C(s32 propType, s32 propId);
void fn_1_8DF4(OMOBJ *object, s32 motionIndex, s32 updateShapeMotion);
void fn_1_8F2C(OMOBJ *object, s32 motionIndex);
void fn_1_9038(OMOBJ *object, s32 motionIndex);
BOOL fn_1_915C(OMOBJ *object);
BOOL fn_1_91C4(s32 propType, s32 propId, char *partName, Vec *position, Vec *rotation);
void fn_1_9300(s32 objectAddress);
void fn_1_9338(OMOBJ *object);
void fn_1_93F4(OMOBJ *object);
void fn_1_94A0(OMOBJ *object);
void fn_1_94A4(OMOBJ *object);
void fn_1_958C(OMOBJ *object);
void fn_1_9650(OMOBJ *object);
void fn_1_9688(OMOBJ *object);
void fn_1_96A8(OMOBJ *object);
void fn_1_9708(OMOBJ *object);
void fn_1_9A94(OMOBJ *object);
void fn_1_9C58(OMOBJ *object);
void fn_1_9ED8(OMOBJ *object);
void fn_1_A080(OMOBJ *object);
void fn_1_A358(OMOBJ *object);
void fn_1_A624(OMOBJ *object);
void fn_1_AA3C(OMOBJ *object);
void fn_1_AB10(OMOBJ *object);
void fn_1_AC9C(OMOBJ *work);
s32 fn_1_AE34(void *objectContext, Vec *position);
void fn_1_AF20(OMOBJ *object);
void fn_1_B228(OMOBJ *object);
void fn_1_B284(OMOBJ *object);
void fn_1_B2C4(OMOBJ *object);
void fn_1_B2F4(OMOBJ *object);
void fn_1_B4B4(OMOBJ *object);
s32 fn_1_BE3C(M676CollisionEntry *first, M676CollisionEntry *second);
void fn_1_BFA4(OMOBJ *object);

void fn_1_C110(OMOBJ *object);
void fn_1_C1B8(void);
s32 fn_1_C2E4(Vec *position, s32 increment);
void fn_1_C4A4(Vec *position, f32 depthOffset, f32 positionDelta);
void fn_1_C5D8(Vec *position, f32 depthOffset);
void fn_1_C800(OMOBJ *owner);
void fn_1_CB74(OMOBJ *object, s32 resourceId, s32 collisionType,
    f32 heightOffset, f32 storedValue);
void fn_1_CCE0(s32 playerIndex, Point3d *destination);
void fn_1_CD50(OMOBJ *object);
void fn_1_CDC0(OMOBJ *object);
void fn_1_CE30(OMOBJ *owner);
void fn_1_D030(M676ParticleRecord *state);
void fn_1_D1E4(M676ParticleRecord *state);
void fn_1_D320(M676ParticleRecord *state);
void fn_1_D488(M676ParticleRecord *state);
void fn_1_D63C(M676ParticleRecord *record);
void fn_1_D724(M676ParticleRecord *record);
void fn_1_D768(M676ParticleRecord *state);
void fn_1_D794(OMOBJ *object);
void fn_1_D8E4(OMOBJ *object);
void fn_1_DA08(OMOBJ *object);
s32 fn_1_DB74(M676PropWork *work);
s32 fn_1_E024(M676PropWork *state);
s32 fn_1_E184(M676PropWork *sequence);
s32 fn_1_E260(M676PropWork *work);
s32 fn_1_E4D4(M676PropWork *work);
s32 fn_1_EAC8(M676PropWork *work);
s32 fn_1_EC00(M676PropWork *work, f32 speed);
s32 fn_1_ED6C(M676PropWork *work, f32 speed);
s32 fn_1_F214(M676PropWork *work, f32 delta, f32 step,
    s32 count, s32 angleMode);
void fn_1_F548(OMOBJ *object);
void fn_1_F678(OMOBJ *object);
s32 fn_1_F7BC(void);
s32 fn_1_F7CC(void);
s32 fn_1_F7DC(Vec *value);
s32 fn_1_F828(void);
s32 fn_1_F8B4(void);
s32 fn_1_F940(s32 index, Vec *resultValues);
void fn_1_F9B0(s32 objectIndex, s32 result, s32 character);
s32 fn_1_FA68(Vec *position, f32 angle);
s32 fn_1_FBCC(s32 objectIndex);
s32 fn_1_FC30(s32 index, Vec *resultValues);
f32 fn_1_FCA0(void);
f32 fn_1_FD64(void);
s32 fn_1_FE28(s32 index);
OMOBJ *fn_1_FF18(Vec *position, f32 heading, f32 angleLimit,
                  f32 maximumDistance, OMOBJ *excludedObject);
void fn_1_10240(s32 index);

Vec lbl_1_data_2D0[2] = {{0.0f, 0.0f, 0.0f}, {0.0f, 151.0f, 5200.0f}};
Vec lbl_1_data_2E8 = {-1400.0f, 10.0f, 800.0f};
Vec lbl_1_data_2F4 = {-1300.0f, 0.0f, 1500.0f};
M676PropMotionParam lbl_1_data_300[9] = {
    {4, 0.20000000298023224f, HU3D_MOTATTR_LOOP},
    {9, 0.20000000298023224f, HU3D_MOTATTR_LOOP},
    {10, 0.20000000298023224f, HU3D_MOTATTR_LOOP},
    {5, 0.20000000298023224f, HU3D_MOTATTR_LOOP},
    {6, 0.20000000298023224f, HU3D_ATTR_NONE},
    {7, 0.20000000298023224f, HU3D_ATTR_NONE},
    {8, 0.20000000298023224f, HU3D_ATTR_NONE},
    {11, 0.20000000298023224f, HU3D_ATTR_NONE},
    {-2, 0.0f, HU3D_ATTR_NONE}
};
M676PropParam lbl_1_data_36C[19] = {
    {1, 0, 1, 0, 3, HU3D_ATTR_NONE, 0, -1, NULL, NULL, 0.0f, fn_1_94A4},
    {1, 0, 0, 0, 3, HU3D_MOTATTR_LOOP, 0, 16, NULL, NULL, 0.0f, fn_1_9650},
    {1, 0, 0, 1, 3, HU3D_ATTR_NOCULL, 0, 17, NULL, NULL, 0.0f, fn_1_94A0},
    {1, 0, 0, 2, 0, HU3D_MOTATTR_LOOP, 0, 20, NULL, NULL, 0.0f, fn_1_94A0},
    {1, 0, 0, 3, 1, HU3D_MOTATTR_LOOP | HU3D_MOTATTR_SHAPE_LOOP, 0, 22, NULL, &lbl_1_data_2E8, 0.0f, fn_1_96A8},
    {8, 1, 2, 0, 1, HU3D_ATTR_NONE, 0, 21, NULL, &lbl_1_data_2F4, 0.0f, fn_1_9A94},
    {20, 1, 3, 0, 3, HU3D_ATTR_NONE, 2, 18, NULL, NULL, 0.0f, fn_1_AF20},
    {1, 0, 4, 0, 4, HU3D_ATTR_NONE, 1, 26, NULL, NULL, 0.0f, fn_1_B2F4},
    {1, 0, 4, 1, 4, HU3D_ATTR_NONE, 1, 27, NULL, NULL, 0.0f, fn_1_B2F4},
    {1, 0, 4, 2, 4, HU3D_ATTR_NONE, 1, 28, NULL, NULL, 0.0f, fn_1_B2F4},
    {1, 0, 4, 3, 4, HU3D_ATTR_NONE, 1, 29, NULL, NULL, 0.0f, fn_1_B2F4},
    {1, 0, 4, 4, 4, HU3D_ATTR_NONE, 1, 30, NULL, NULL, 0.0f, fn_1_B2F4},
    {5, 1, 8, 2, 7, HU3D_MOTATTR_LOOP, 0, 23, NULL, NULL, 0.0f, fn_1_9ED8},
    {15, 1, 9, 0, 1, HU3D_MOTATTR_LOOP, 0, 24, NULL, NULL, 0.0f, fn_1_A358},
    {15, 1, 10, 0, 1, HU3D_MOTATTR_LOOP, 0, 25, NULL, NULL, 0.0f, fn_1_A358},
    {1, 0, 5, 0, 4, HU3D_ATTR_NONE, 0, -1, NULL, NULL, 0.0f, fn_1_CD50},
    {1, 0, 6, 0, 4, HU3D_ATTR_NONE, 0, -1, NULL, NULL, 0.0f, fn_1_CDC0},
    {1, 0, 7, 0, 4, HU3D_ATTR_NONE, 1, 3, lbl_1_data_300, &lbl_1_data_2D0[1], 0.0f, fn_1_D794},
    {-1, 0, 0, 0, 0, HU3D_ATTR_NONE, 0, 0, NULL, NULL, 0.0f, NULL}
};

/* Initializes the scene prop manager and creates each prop in the scene table. */
void fn_1_88C0(HUPROCESS *process) {
    OMOBJ *managerObject;
    OMOBJ **registeredObject;
    M676PropParam *propParameters;
    s32 objectIndex;
    void *managerWork;

    registeredObject = lbl_1_bss_EA0;
    objectIndex = 0;
    while (objectIndex < 128) {
        *registeredObject = 0;
        objectIndex += 1;
        registeredObject += 1;
    }
    managerObject = omAddObjEx(process, 12, 0U, 0U, -1, fn_1_89F4);
    lbl_1_bss_10A0 = managerObject;
    managerObject->stat |= OM_STAT_MODELPAUSE;
    managerWork = HuMemDirectMallocNum(HEAP_HEAP, 12, HU_MEMNUM_OVL);
    managerObject->data = managerWork;
    memset(managerWork, 0, 12);
    lbl_1_bss_E90 = 0;
    lbl_1_bss_E8C = 0;
    lbl_1_bss_E88 = 0;
    fn_1_AA3C(managerObject);
    fn_1_B284(managerObject);
    fn_1_C110(managerObject);
    propParameters = lbl_1_data_36C;
    while (propParameters->count != -1) {
        fn_1_8A58(process, propParameters);
        propParameters++;
    }
}

void fn_1_89F0(void)
{
}

void fn_1_89F4(OMOBJ *obj)
{
    obj->objFunc = fn_1_8A04;
}

/* Runs each manager update and starts scene processing on its second call. */
void fn_1_8A04(OMOBJ *owner) {
    M676SceneManagerWork *managerWork = owner->data;
    if (managerWork->updateCount == 1) {
        fn_1_C800(owner);
    }
    managerWork->updateCount += 1;
}

/* Creates the configured prop group during scene setup. */
void fn_1_8A58(HUPROCESS *process, M676PropParam *propParameters)
{
    s32 index;
    OMOBJ *object;
    s32 modelId;
    M676PropWork *propWork;
    M676PropState *propState;
    M676PropMotionParam *motionParameters;
    s32 groupIndex;
    s32 motionResource;

    for (groupIndex = 0; groupIndex < propParameters->count; groupIndex++) {
        /* Scene setup assumes the object manager and heap allocations succeed. */
        object = omAddObjEx(process, 12, 2, 8, 0, propParameters->update);
        fn_1_8D18(object);
        propWork = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M676PropWork), HU_MEMNUM_OVL);
        object->data = propWork;
        memset(propWork, 0, sizeof(M676PropWork));
        object->stat |= OM_STAT_MODELPAUSE;
        propState = &propWork->state;
        propWork->param = propParameters;
        propWork->id = propParameters->firstId + groupIndex;
        propState->actor.currentMotion = -1;
        for (index = 0; index < 2; index++) {
            object->mdlId[index] = -1;
        }
        for (index = 0; index < 8; index++) {
            object->mtnId[index] = -1;
        }
        if (propParameters->modelResource >= 0) {
            if (propParameters->linkModel != 0) {
                modelId = fn_1_50A8(fn_1_1480(propParameters->modelResource));
            } else {
                modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
                    fn_1_1480(propParameters->modelResource), HU_MEMNUM_OVL, HEAP_MODEL));
            }
        } else {
            modelId = -1;
        }
        object->mdlId[0] = modelId;
        motionParameters = propParameters->motions;
        if (motionParameters != NULL) {
            for (index = 0; index < 8; index++, motionParameters++) {
                motionResource = motionParameters->resource;
                if (motionResource == -2) {
                    break;
                }
                if (motionResource != -1) {
                    object->mtnId[index] = fn_1_51B0(modelId, fn_1_1480(motionResource));
                }
            }
        }
        if (propParameters->position != NULL) {
            propState->actor.position = *propParameters->position;
        }
        propWork->startPosition = propState->actor.position;
        propState->actor.rotation.y = propParameters->rotationY;
        if (modelId >= 0) {
            Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
            Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
            Hu3DModelLayerSet(modelId, propParameters->layer);
            Hu3DModelAttrSet(modelId, propParameters->attr);
            switch (propParameters->shadow) {
            case 1:
                Hu3DModelShadowSet(modelId);
                break;
            case 2:
                Hu3DModelShadowMapSet(modelId);
                break;
            }
        }
    }
}

/* Adds a newly created prop to the first available scene lookup slot. */
void fn_1_8D18(OMOBJ *object)
{
    OMOBJ **objectSlot = lbl_1_bss_EA0;
    s32 slotIndex = 0;

    while (slotIndex < 128) {
        if (*objectSlot == NULL) {
            *objectSlot = object;
            break;
        }
        ++slotIndex;
        ++objectSlot;
    }
    /* If all 128 lookup slots are occupied, this prop remains unregistered. */
}

/* Finds a registered prop by its configured type and per-group ID. */
OMOBJ *fn_1_8D6C(s32 propType, s32 propId)
{
    OMOBJ *object;
    OMOBJ **objectSlot = lbl_1_bss_EA0;
    M676PropWork *propWork;
    s32 slotIndex;
    for (slotIndex = 0; slotIndex < 128; slotIndex++, objectSlot++) {
        object = *objectSlot;
        if (object != NULL) {
            propWork = object->data;
            if (propWork->param->type == propType && propWork->id == propId) {
                return object;
            }
        }
    }
    return NULL;
}

/* Changes a prop's motion when its requested motion differs from the current one. */
void fn_1_8DF4(OMOBJ *object, s32 motionIndex, s32 updateShapeMotion)
{
    M676PropState *propState;
    M676PropMotionParam *motionParameters;
    M676PropWork *propWork = object->data;
    s32 modelId, motionId;

    propState = &propWork->state;
    if (propState->actor.currentMotion != motionIndex && motionIndex < 8) {
        motionId = object->mtnId[motionIndex];
        propWork->timer = 0;
        propState->actor.motionTime = 0;
        if (motionId == -1) {
            propState->motionActive = 0;
            return;
        }
        motionParameters = &propWork->param->motions[motionIndex];
        modelId = object->mdlId[0];
        if (propState->actor.currentMotion < 0 || (0.0) == motionParameters->blend) {
            if (updateShapeMotion) {
                Hu3DMotionShapeSet((s16) modelId, (s16) motionId);
            }
            Hu3DMotionSet((s16) modelId, (s16) motionId);
            Hu3DModelAttrSet((s16) modelId, motionParameters->attr);
        } else {
            if (updateShapeMotion) {
                Hu3DMotionShapeSet((s16) modelId, (s16) motionId);
            }
            Hu3DMotionShiftSet((s16) modelId, (s16) motionId, (0.0f),
                (60.0f) * motionParameters->blend, motionParameters->attr);
        }
        propState->actor.currentMotion = motionIndex;
    }
}

void fn_1_8F2C(OMOBJ *object, s32 motionIndex)
{
    fn_1_8DF4(object, motionIndex, 0);
}

void fn_1_9038(OMOBJ *object, s32 motionIndex)
{
    fn_1_8DF4(object, motionIndex, 1);
}

/* Returns whether the main motion ended and no blended motion remains. */
BOOL fn_1_915C(OMOBJ *object)
{
    s32 modelId;

    modelId = object->mdlId[0];
    if (Hu3DMotionEndCheck((HU3D_MODELID)modelId) != 0 &&
        Hu3DMotionShiftIDGet((HU3D_MODELID)modelId) < 0) {
        return TRUE;
    }
    return FALSE;
}

/* Writes the named part's transform if found, then returns whether the main motion has not ended;
 * blends are not checked. */
BOOL fn_1_91C4(s32 propType, s32 propId, char *partName, Vec *position, Vec *rotation)
{
    OMOBJ *object;
    M676PropWork *propWork;
    HSF_OBJECT *part;
    s32 modelId;

    object = fn_1_8D6C(propType, propId);
    if (object == NULL) {
        return FALSE;
    }
    propWork = object->data;
    modelId = object->mdlId[0];
    part = Hu3DModelObjPtrGet(modelId, partName);
    if (part != NULL) {
        position->x = part->mesh.curr.pos.x;
        position->y = part->mesh.curr.pos.y;
        position->z = part->mesh.curr.pos.z;
        rotation->x = part->mesh.curr.rot.x;
        /* Model-part yaw uses a quarter-turn offset from the scene transform. */
        /* The scene's yaw is offset by 90 degrees from the model-part yaw. */
        rotation->y = 90.0 + part->mesh.curr.rot.y;
        rotation->z = part->mesh.curr.rot.z;
    }
    return !Hu3DMotionEndCheck(modelId);
}

/* Prop update hook: copies the actor transform to its model and advances motion state and timer. */
void fn_1_9300(s32 objectAddress)
{
    fn_1_9338((OMOBJ *) objectAddress);
    fn_1_93F4((OMOBJ *) objectAddress);
}

/* Copies the prop's position and rotation to its model during its update. */
void fn_1_9338(OMOBJ *object)
{
    M676PropState *propState;
    M676PropWork *propWork = object->data;
    s32 modelId;

    propState = &propWork->state;
    modelId = object->mdlId[0];
    propState->actor.rotation.y = fmodf(propState->actor.rotation.y, 360.0f);
    Hu3DModelPosSetV(modelId, &propState->actor.position);
    Hu3DModelRotSetV(modelId, &propState->actor.rotation);
}

/* Clears completed motion state and advances this prop's update timer. */
void fn_1_93F4(OMOBJ *object)
{
    M676PropState *propState;
    M676PropWork *propWork = object->data;
    s32 motionFinished, modelId;

    propState = &propWork->state;
    if (propState->motionActive) {
        modelId = object->mdlId[0];
        if (Hu3DMotionEndCheck(modelId) && Hu3DMotionShiftIDGet(modelId) < 0) {
            motionFinished = 1;
        } else {
            motionFinished = 0;
        }
        if (motionFinished) {
            propState->motionActive = 0;
            propState->motionStatus = 0;
        }
    } else {
        propState->motionStatus = 0;
    }
    propWork->timer++;
}

void fn_1_94A0(OMOBJ *object)
{
}

const M676CollisionBounds lbl_1_rodata_340 = {
    {1, 8, 3, 0, 13, NULL, NULL, NULL},
    {-820.0f, 151.0f, 150.0f}, {820.0f, 151.0f, 280.0f}
};

/* Registers the scene's collision bounds when the manager creates this prop. */
void fn_1_94A4(OMOBJ *object)
{
    M676PropWork *propWork = object->data;
    M676SceneRegistration *registration = &lbl_1_bss_E40;

    memset(registration, 0, sizeof(*registration));
    propWork->records = registration;
    registration->bounds = lbl_1_rodata_340;
    /* The increment applies only to this local pointer; the prop keeps the base record. */
    fn_1_13F54(registration++);
    object->objFunc = fn_1_958C;
}

/* Advances collision registration phases from this prop's update callback. */
void fn_1_958C(OMOBJ *object)
{
    M676PropWork *propWork = object->data;
    M676CollisionEntry *collisionEntry = propWork->records;

    switch (propWork->phase) {
    case 0:
        propWork->phase++;
        propWork->frame = 0;
        /* fall through */
    case 1:
        if (fn_1_1498() != 0) {
            propWork->phase++;
            propWork->frame = 0;
            /* fall through */
        } else {
            break;
        }
    case 2:
        if (fn_1_1498() == 0) {
            propWork->phase++;
            propWork->frame = 0;
            collisionEntry->header.flags = 0;
        }
        break;
    }
}

/* Stores this prop in the scene slot and replaces its setup callback. */
void fn_1_9650(OMOBJ *object) {
    void *propWorkData;
    s32 modelId;

    propWorkData = object->data;
    modelId = object->mdlId[0];
    lbl_1_bss_E9C = object;
    object->objFunc = fn_1_9688;
}

/* Runs as the replacement update callback; it reads the prop data and model ID. */
void fn_1_9688(OMOBJ *object) {
    void *propWorkData;
    s32 modelId;

    propWorkData = object->data;
    modelId = object->mdlId[0];
}

/* Sets the prop's initial yaw and installs its bobbing update callback. */
void fn_1_96A8(OMOBJ *object)
{
    M676PropWork *propWork = object->data;
    M676PropWork *actorWork;

    actorWork = propWork;
    actorWork->state.actor.rotation.y = -20.0f;
    propWork->pitchPhase = 0.0f;
    propWork->rollPhase = 0.0f;
    object->objFunc = fn_1_9708;
}

/* Updates the prop's bob and wobble, model transform, and completed motion state. */
void fn_1_9708(OMOBJ *object)
{
    M676PropWork *propWork = object->data;
    M676PropWork *actorWork = propWork;
    s32 modelId;
    s32 motionShiftFinished;
    s32 motionModelId;
    f32 wobbleRotationScale;
    f32 loweredBobPhase;
    f32 pitchPhase;
    f32 rollPhase;
    f32 modelRotationY;
    f32 wrappedBobPhase;
    f32 bobPhase;
    f32 wrappedPitchPhase;
    f32 storedPitchPhase;
    f32 wrappedRollPhase;
    f32 storedRollPhase;
    f32 wrappedModelRotationY;
    f32 modelRotationYForRender;
    M676PropWork *renderPropWork;
    M676PropWork *renderActorWork;
    M676PropWork *motionPropWork;
    M676PropWork *motionPropState;

    propWork->wobbleAmount *= 0.95;
    loweredBobPhase = propWork->bobPhase - 10.0f;
    wrappedBobPhase = (f32)fmod((f64)loweredBobPhase, 360.0);
    bobPhase = wrappedBobPhase;
    propWork->bobPhase = bobPhase;

    wobbleRotationScale = (f32)(1.5 + (3.0 * (f64)propWork->wobbleAmount));
    actorWork->state.actor.rotation.x = (f32)((f64)wobbleRotationScale * sin(
        (3.141592653589793 * (f64)propWork->pitchPhase) / 180.0));
    actorWork->state.actor.rotation.z = (f32)((f64)wobbleRotationScale * sin(
        (3.141592653589793 * (f64)propWork->rollPhase) / 180.0));

    wobbleRotationScale = (f32)(1.0 +
        (1.5 * (f64)propWork->wobbleAmount));
    pitchPhase = propWork->pitchPhase + wobbleRotationScale;
    wrappedPitchPhase = (f32)fmod((f64)pitchPhase, 360.0);
    storedPitchPhase = wrappedPitchPhase;
    propWork->pitchPhase = storedPitchPhase;
    rollPhase = (f32)((f64)propWork->rollPhase +
        (1.5 * (f64)wobbleRotationScale));
    wrappedRollPhase = (f32)fmod((f64)rollPhase, 360.0);
    storedRollPhase = wrappedRollPhase;
    propWork->rollPhase = storedRollPhase;

    actorWork->state.actor.position.y = (f32)((f64)propWork->startPosition.y +
        (25.0 * ((f64)propWork->wobbleAmount * sin(
            (3.141592653589793 * (f64)propWork->bobPhase) / 180.0))));

    renderPropWork = object->data;
    renderActorWork = renderPropWork;
    modelId = object->mdlId[0];
    modelRotationY = renderActorWork->state.actor.rotation.y;
    wrappedModelRotationY =
        (f32)fmod((f64)modelRotationY, 360.0);
    modelRotationYForRender = wrappedModelRotationY;
    renderActorWork->state.actor.rotation.y = modelRotationYForRender;
    Hu3DModelPosSetV((s16)modelId, &renderActorWork->state.actor.position);
    Hu3DModelRotSetV((s16)modelId, &renderActorWork->state.actor.rotation);

    motionPropWork = object->data;
    motionPropState = motionPropWork;
    if (motionPropState->state.motionActive != 0) {
        motionModelId = object->mdlId[0];
        if (Hu3DMotionEndCheck((s16)motionModelId) &&
            Hu3DMotionShiftIDGet((s16)motionModelId) < 0) {
            motionShiftFinished = 1;
        } else {
            motionShiftFinished = 0;
        }
        if (motionShiftFinished != 0) {
            motionPropState->state.motionActive = 0;
            motionPropState->state.motionStatus = 0;
        }
    } else {
        motionPropState->state.motionStatus = 0;
    }
    motionPropWork->timer++;
}

/* Prop setup callback: offsets its starting position and seeds its yaw and wobble phases. */
void fn_1_9A94(OMOBJ *object)
{
    M676PropWork *work = object->data;
    M676PropWork *state = work;
    s32 modelId = object->mdlId[0];

    Hu3DModelAttrSet(modelId, HU3D_ATTR_NOCULL);
    state->state.actor.position.z += 500.0 * (f64)(f32)work->id;
    state->state.actor.rotation.y = (f32)(10.0 +
        (40.0 * (1.52587890625e-05 *
            (f64)(f32)(s32)(frand() & 0xFFFF))));
    work->pitchPhase = (f32)(360.0 *
        (3.0517578125e-05 *
            (f64)(f32)((s32)(frand() & 0xFFFF) - 32768)));
    work->rollPhase = (f32)(360.0 *
        (3.0517578125e-05 *
            (f64)(f32)((s32)(frand() & 0xFFFF) - 32768)));
    object->objFunc = fn_1_9C58;
}

/* Per-frame prop callback: updates its wobble, model transform, and motion state. */
void fn_1_9C58(OMOBJ *object)
{
    M676PropWork *work = object->data;
    M676PropWork *renderWork;
    M676PropWork *renderState;
    M676PropWork *state = work;
    f32 angleX;
    f32 angleZ;
    f32 rotationY;
    f32 wrappedX;
    f32 normalizedX;
    f32 wrappedZ;
    f32 normalizedZ;
    f32 wrappedY;
    f32 normalizedY;
    s32 modelId;
    s32 clearShiftedMotion;
    s32 motionModelId;

    state->state.actor.rotation.x = (f32)(3.0 * sin(
        (3.141592653589793 * (f64)work->pitchPhase) / 180.0));
    state->state.actor.rotation.z = (f32)(3.0 * sin(
        (3.141592653589793 * (f64)work->rollPhase) / 180.0));

    angleX = (f32)(2.0 + work->pitchPhase);
    wrappedX = (f32)fmod((f64)angleX, 360.0);
    normalizedX = wrappedX;
    work->pitchPhase = normalizedX;
    angleZ = (f32)(2.0 + work->rollPhase);
    wrappedZ = (f32)fmod((f64)angleZ, 360.0);
    normalizedZ = wrappedZ;
    work->rollPhase = normalizedZ;

    renderWork = object->data;
    renderState = renderWork;
    modelId = object->mdlId[0];
    rotationY = renderState->state.actor.rotation.y;
    wrappedY = (f32)fmod((f64)rotationY, 360.0);
    normalizedY = wrappedY;
    renderState->state.actor.rotation.y = normalizedY;
    Hu3DModelPosSetV((s16)modelId, &renderState->state.actor.position);
    Hu3DModelRotSetV((s16)modelId, &renderState->state.actor.rotation);

    {
        M676PropWork *motionWork = object->data;
        M676PropWork *motionState = motionWork;

        if (motionState->state.motionActive != 0) {
            motionModelId = object->mdlId[0];
            if (Hu3DMotionEndCheck(motionModelId) &&
                Hu3DMotionShiftIDGet(motionModelId) < 0) {
                clearShiftedMotion = 1;
            } else {
                clearShiftedMotion = 0;
            }

            if (clearShiftedMotion != 0) {
                motionState->state.motionActive = 0;
                motionState->state.motionStatus = 0;
            }
        } else {
            motionState->state.motionStatus = 0;
        }
        motionWork->timer++;
    }
}

/* Prop setup callback: chooses a starting position and scale, then installs its movement update. */
void fn_1_9ED8(OMOBJ *object)
{
    M676PropWork *work = object->data;
    M676PropWork *state = work;
    s32 modelId = object->mdlId[0];

    work->targetChangeTimer = 0;
    state->state.actor.position.x = (f32)(800.0 * (0.000030517578125 *
        (f64)(f32)((s32)(frand() & 0xFFFF) - 32768)));
    state->state.actor.position.y = (f32)(900.0 + (20.0 * (f64)work->id));
    state->state.actor.position.z = (f32)(800.0 + (4000.0 *
        (0.0000152587890625 * (f64)(f32)(s32)(frand() & 0xFFFF))));
    Hu3DModelScaleSet((s16)modelId, 0.3f, 0.3f, 0.3f);
    Hu3DModelAttrSet((s16)modelId, HU3D_ATTR_NOCULL);
    object->objFunc = fn_1_A080;
}

/* Per-frame movement callback steers the prop toward random targets and updates its model. */
void fn_1_A080(OMOBJ *object)
{
    f32 velocityScale;
    f32 angle;
    f32 desiredVelocityX;
    f32 desiredVelocityZ;
    s32 modelId;
    M676PropWork *work = object->data;
    M676PropWork *state = work;

    velocityScale = 8.333333f;
    modelId = object->mdlId[0];
    if (--work->targetChangeTimer <= 0) {
        work->targetChangeTimer =
            (s32)(180.0 * (1.52587890625e-5 *
                (f64)(f32)(s32)(frand() & 0xFFFF))) + 300;
        state->state.targetPosition.x = (f32)(800.0 * (3.0517578125e-5 *
            (f64)(f32)((s32)(frand() & 0xFFFF) - 32768)));
        state->state.targetPosition.y = state->state.actor.position.y;
        state->state.targetPosition.z = (f32)(800.0 + (4000.0 *
            (1.52587890625e-5 *
                (f64)(f32)(s32)(frand() & 0xFFFF))));
    }

    angle = fn_1_5490(&state->state.actor.position, &state->state.targetPosition);
    state->state.actor.rotation.y = fn_1_52C0(state->state.actor.rotation.y, angle,
        0.01f);
    desiredVelocityX = (f32)(velocityScale *
        sin((3.141592653589793 * (f64)state->state.actor.rotation.y) /
            180.0));
    desiredVelocityZ = (f32)(velocityScale *
        cos((3.141592653589793 * (f64)state->state.actor.rotation.y) /
            180.0));
    state->state.actor.velocity.x = (f32)((f64)state->state.actor.velocity.x +
        0.1 * (f64)(desiredVelocityX - state->state.actor.velocity.x));
    state->state.actor.velocity.z = (f32)((f64)state->state.actor.velocity.z +
        0.1 * (f64)(desiredVelocityZ - state->state.actor.velocity.z));
    PSVECAdd(&state->state.actor.position, &state->state.actor.velocity,
             &state->state.actor.position);
    Hu3DModelPosSetV((s16)modelId, &state->state.actor.position);
    Hu3DModelRotSetV((s16)modelId, &state->state.actor.rotation);
}

/* Prop setup callback: chooses its starting transform and scale, then installs its movement
 * update. */
void fn_1_A358(OMOBJ *object)
{
    M676PropWork *work = object->data;
    M676PropWork *state = work;
    s32 modelId = object->mdlId[0];
    f32 scale;

    work->targetChangeTimer = 0;
    state->state.actor.position.x = (f32)(1000.0 * (3.0517578125e-05 *
        (f64)(f32)((s32)(frand() & 0xFFFF) - 32768)));
    state->state.actor.position.y = (f32)(-200.0 -
        (10.0 * (f64)work->id));
    state->state.actor.rotation.y = (f32)(360.0 *
        (1.52587890625e-05 * (f64)(f32)(s32)(frand() & 0xFFFF)));
    if ((work->id & 3) == 0) {
        state->state.actor.position.z = (f32)(300.0 +
            (1200.0 * (1.52587890625e-05 * (f64)(f32)(s32)(frand() & 0xFFFF))));
    } else {
        state->state.actor.position.z = (f32)(1000.0 +
            (4000.0 * (1.52587890625e-05 * (f64)(f32)(s32)(frand() & 0xFFFF))));
    }
    scale = (f32)(0.2 +
        (0.2 * (1.52587890625e-05 * (f64)(f32)(s32)(frand() & 0xFFFF))));
    Hu3DModelScaleSet((s16)modelId, scale, scale, scale);
    Hu3DModelAttrSet((s16)modelId, HU3D_ATTR_NOCULL);
    object->objFunc = fn_1_A624;
}

/* Per-frame movement callback steers the prop toward targets and updates its model and motion. */
void fn_1_A624(OMOBJ *object)
{
    M676PropWork *work = object->data;
    M676PropWork *state = work;
    f32 speed = 2.5f;
    s16 modelId;
    M676PropWork *renderWork;
    M676PropWork *renderState;
    f32 angle;
    f32 desiredVelocityX;
    f32 desiredVelocityZ;
    f32 rotationY;
    f32 wrappedY;
    f32 normalizedY;

    if (--work->targetChangeTimer <= 0) {
        work->targetChangeTimer =
            (s32)(120.0 * (1.52587890625e-05 *
                (f64)(f32)(s32)(frand() & 0xFFFF))) + 60;
        state->state.targetPosition.x = (f32)(1000.0 * (3.0517578125e-05 *
            (f64)(f32)((s32)(frand() & 0xFFFF) - 32768)));
        state->state.targetPosition.y = state->state.actor.position.y;
        if ((work->id & 3) == 0) {
            state->state.targetPosition.z = (f32)(300.0 + (1200.0 *
                (1.52587890625e-05 *
                    (f64)(f32)(s32)(frand() & 0xFFFF))));
        } else {
            state->state.targetPosition.z = (f32)(1000.0 + (4000.0 *
                (1.52587890625e-05 *
                    (f64)(f32)(s32)(frand() & 0xFFFF))));
        }
    }

    angle = fn_1_5490(&state->state.actor.position, &state->state.targetPosition);
    state->state.actor.rotation.y = fn_1_52C0(state->state.actor.rotation.y, angle,
        0.02f);
    desiredVelocityX = (f32)(speed *
        sin((3.141592653589793 * (f64)state->state.actor.rotation.y) /
            180.0));
    state->state.actor.velocity.x = (f32)((f64)state->state.actor.velocity.x +
        0.1 * (f64)(desiredVelocityX - state->state.actor.velocity.x));
    desiredVelocityZ = (f32)(speed *
        cos((3.141592653589793 * (f64)state->state.actor.rotation.y) /
            180.0));
    state->state.actor.velocity.z = (f32)((f64)state->state.actor.velocity.z +
        0.1 * (f64)(desiredVelocityZ - state->state.actor.velocity.z));
    PSVECAdd(&state->state.actor.position, &state->state.actor.velocity,
             &state->state.actor.position);

    renderWork = object->data;
    renderState = renderWork;
    modelId = object->mdlId[0];
    rotationY = renderState->state.actor.rotation.y;
    wrappedY = (f32)fmod((f64)rotationY, 360.0);
    normalizedY = wrappedY;
    renderState->state.actor.rotation.y = normalizedY;
    Hu3DModelPosSetV((s16)modelId, &renderState->state.actor.position);
    Hu3DModelRotSetV((s16)modelId, &renderState->state.actor.rotation);

    {
        M676PropWork *motionWork = object->data;
        M676PropWork *motionState = motionWork;
        s32 clearShiftedMotion;
        s16 motionModelId;

        if (motionState->state.motionActive != 0) {
            motionModelId = object->mdlId[0];
            if (Hu3DMotionEndCheck((s16)motionModelId) &&
                Hu3DMotionShiftIDGet((s16)motionModelId) < 0) {
                clearShiftedMotion = 1;
            } else {
                clearShiftedMotion = 0;
            }
            if (clearShiftedMotion != 0) {
                motionState->state.motionActive = 0;
                motionState->state.motionStatus = 0;
            }
        } else {
            motionState->state.motionStatus = 0;
        }
        motionWork->timer++;
    }
}

const Vec lbl_1_rodata_46C[4] = {{-600,0,800},{-200,0,1000},{200,0,1200},{600,0,800}};

/* Scene setup callback clears the active flag and area-record tables. */
void fn_1_AA3C(OMOBJ *object)
{
    s32 innerIndex;
    s32 outerIndex;
    M676SceneFlagEntry *flagEntry;
    M676SceneAreaRecord *counterEntry;

    lbl_1_bss_E3C = 0;
    for (outerIndex = 0; outerIndex < 7; outerIndex++) {
        for (innerIndex = 0; innerIndex < 4; innerIndex++) {
            flagEntry = &lbl_1_bss_C7C[outerIndex * 4 + innerIndex];
            flagEntry->active = 0;
        }
    }
    for (outerIndex = 0; outerIndex < 14; outerIndex++) {
        for (innerIndex = 0; innerIndex < 5; innerIndex++) {
            counterEntry = &lbl_1_bss_5EC[outerIndex * 5 + innerIndex];
            counterEntry->active = 0;
            counterEntry->propMask = 0;
        }
    }
}

/* Builds paired area records from active scene flags after prop positions are initialized. */
void fn_1_AB10(OMOBJ *object)
{
    s32 outerIndex;
    s32 innerIndex;
    s32 counter;
    s32 firstEntryIndex;
    s32 secondEntryIndex;
    s32 counters[5];
    M676SceneFlagEntry *flagEntry;
    M676SceneAreaRecord *entry;

    for (innerIndex = 0; innerIndex < 5; innerIndex++) {
        counters[innerIndex] = 0;
    }
    for (outerIndex = 0; outerIndex < 7; outerIndex++) {
        for (innerIndex = 0; innerIndex < 4; innerIndex++) {
            flagEntry = &lbl_1_bss_C7C[outerIndex * 4 + innerIndex];
            if (flagEntry->active != 0) {
                counter = counters[innerIndex]++;
                firstEntryIndex = innerIndex + counter * 5;
                counter = counters[innerIndex + 1]++;
                secondEntryIndex = innerIndex + counter * 5 + 1;
                entry = &lbl_1_bss_5EC[firstEntryIndex];
                entry->active = 1;
                entry->next = secondEntryIndex;
                entry->position = flagEntry->position;
                entry->position.x -= 200.0;
                entry = &lbl_1_bss_5EC[secondEntryIndex];
                entry->active = 2;
                entry->next = firstEntryIndex;
                entry->position = flagEntry->position;
                entry->position.x += 200.0;
            }
        }
    }
    lbl_1_bss_E3C = 1;
}

/* Prop setup assigns an unoccupied scene flag and its corresponding starting position. */
void fn_1_AC9C(OMOBJ *work)
{
    M676PropWork *spawnData = work->data;
    M676PropWork *positionData = spawnData;
    M676SceneFlagEntry *entry;
    s32 column;
    s32 row;

    do {
        column = (s32)(4.0 * (1.52587890625e-5 *
                             (f64)(f32)(s32)(frand() & 0xFFFF)));
        row = (s32)(7.0 * (1.52587890625e-5 *
                          (f64)(f32)(s32)(frand() & 0xFFFF)));
        entry = &lbl_1_bss_C7C[column + row * 4];
    } while (entry->active != 0);

    entry->active = 1;
    positionData->state.actor.position = lbl_1_rodata_46C[column];
    positionData->state.actor.position.z += 600.0 * (f64)(f32)row;
    entry->position = positionData->state.actor.position;
}

/* Prop movement finds the active area record with the smallest z not below its current depth. */
s32 fn_1_AE34(void *objectContext, Vec *position)
{
    f32 nearestThreshold = 100000.0f;
    s32 selectedOffset = -1;
    s32 region;
    s32 offset;

    if (lbl_1_bss_E3C == 0) {
        return -1;
    }
    region = fn_1_F7DC(position);
    for (offset = 0; offset < 14; offset++) {
        M676SceneAreaRecord *record = &lbl_1_bss_5EC[region + offset * 5];

        if (record->active != 0 && !(record->position.z < position->z) &&
            record->position.z < nearestThreshold) {
            nearestThreshold = record->position.z;
            selectedOffset = offset;
        }
    }
    if (selectedOffset == -1) {
        return -1;
    }
    return region + selectedOffset * 5;
}

/* Prop setup assigns its scene flag and position, sets up a secondary model, and installs its
 * update callback. */
void fn_1_AF20(OMOBJ *object)
{
    M676PropWork *work = object->data;
    M676PropWork *renderWork = work;
    M676PropWork *motionWork;
    M676PropWork *motionState;
    s32 modelId = object->mdlId[0];

    Hu3DModelAttrSet(modelId, HU3D_ATTR_NOCULL);
    fn_1_AC9C(object);

    fn_1_9338(object);

    motionWork = object->data;
    motionState = motionWork;
    if (motionState->state.motionActive != 0) {
        if (fn_1_915C(object)) {
            motionState->state.motionActive = 0;
            motionState->state.motionStatus = 0;
        }
    } else {
        motionState->state.motionStatus = 0;
    }
    motionWork->timer++;

    modelId = fn_1_50A8(fn_1_1480(19));
    object->mdlId[1] = modelId;
    Hu3DModelLayerSet(modelId, 0);
    Hu3DModelAttrSet(modelId, HU3D_ATTR_NOCULL);
    Hu3DModelPosSetV(modelId, (Vec *)&renderWork->state.actor.position);
    Hu3DModelRotSetV(modelId, &renderWork->state.actor.rotation);
    object->objFunc = fn_1_B228;
}

/* Builds the scene area pairs when they are still unbuilt; subsequent calls leave them
 * unchanged. */
void fn_1_B228(OMOBJ *object)
{
    void *work;
    void *scene;

    work = object->data;
    scene = work;
    if (lbl_1_bss_E3C == 0) {
        fn_1_AB10(lbl_1_bss_10A0);
    }
}

/* Scene setup clears the five prop-object slots before registering new objects. */
void fn_1_B284(OMOBJ *object) {
    s32 index;

    index = 0;
    while (index < 5) {
        lbl_1_bss_5D8[index] = NULL;
        index += 1;
    }
}

/* Prop initialization registers the object in the slot indexed by its work ID. */
void fn_1_B2C4(OMOBJ *object) {
    M676PropWork *work;

    work = object->data;
    lbl_1_bss_5D8[work->id] = object;
}

const Vec lbl_1_rodata_4C4[5] = {
    {-800.0f, 210.0f, 430.0f},
    {-400.0f, 210.0f, 430.0f},
    {0.0f, 210.0f, 430.0f},
    {400.0f, 210.0f, 430.0f},
    {800.0f, 210.0f, 430.0f}
};

/* Prop setup places it in its lane, registers collision, and installs its update. */
void fn_1_B2F4(OMOBJ *object)
{
    M676PropWork *work = object->data;
    M676PropWork *state = work;
    M676PropWork *tableWork = object->data;
    M676CollisionEntry *record;

    lbl_1_bss_5D8[tableWork->id] = object;
    state->state.actor.position = lbl_1_rodata_4C4[work->id];
    work->startPosition = state->state.actor.position;

    work->modelId = fn_1_50A8(fn_1_1480(work->id + 31));
    Hu3DModelLayerSet((s16)work->modelId, 4);
    Hu3DModelAttrSet((s16)work->modelId, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet((s16)work->modelId, HU3D_MOTATTR_PAUSE);

    record = &state->state.actor.collision;
    record->header.flags = 1;
    record->header.collisionMask = M676_COLLISION_MASK_PROP;
    record->header.type = 1;
    record->header.id = work->id;
    record->header.state = 0;
    record->header.callbackArgument = object;
    record->header.updateCallback = fn_1_BFA4;
    record->shape.dynamic.callback = fn_1_BE3C;
    record->shape.dynamic.position = &state->state.actor.position;
    record->shape.dynamic.velocity = &state->state.actor.velocity;
    record->shape.dynamic.currentPosition = *record->shape.dynamic.position;
    record->shape.dynamic.radius = 50.0f;
    record->shape.dynamic.movable = 0.0f;
    fn_1_13EC4(record);

    state->state.savedPosition = state->state.actor.position;
    state->state.poseEnabled = 1;
    state->state.poseHeight = 75.0f;
    state->state.savedPositionPointer = &state->state.savedPosition;
    object->objFunc = fn_1_B4B4;
}

/* Per-frame lane-prop callback advances travel phases, handles prop collisions and area crossings,
 * and updates completion visibility. */
void fn_1_B4B4(OMOBJ *object) {
    f32 desiredVelocityX;
    f32 desiredVelocityZ;
    f32 movementSpeed;
    s32 propMask;
    s32 selectedRecordIndex;
    M676SceneAreaRecord *record;
    M676PropWork *work;
    M676PropWork *state;

    work = object->data;
    state = work;
    movementSpeed = 6.6666665f;
    propMask = 1 << work->id;
    switch (work->phase) {
    case 0:
        work->phase = (u32) (work->phase + 1);
        work->frame = 0;
        state->state.actor.position.y += 10.0;
        state->state.actor.rotation.x = 180.0f;
        state->state.actor.rotation.z = 90.0f;
        work->soundHandle = -1;
        /* fallthrough */
    case 1:
        if ((s32) work->initialized != 0) {
            work->phase = (u32) (work->phase + 1);
            work->frame = 0;
            state->state.actor.position.y -= 10.0;
            state->state.actor.rotation.x = -40.0f;
            state->state.actor.rotation.z = 0.0f;
        case 2:
            if (fn_1_15E0() != 0) {
                work->phase = (u32) (work->phase + 1);
                work->frame = 0;
            case 3:
                if (++work->frame >= 60) {
                    work->phase = (u32) (work->phase + 1);
                    work->frame = 0;
                    /* These phase fields become movement controls here; pitchPhase is cleared twice
                     * and rollPhase is cleared before being set to 1. */
                    work->wobbleAmount = 0.0f;
                    work->bobPhase = 1.0f;
                    work->pitchPhase = 0.0f;
                    work->unknown_1E0 = 0.0f;
                    work->rollPhase = 0.0f;
                    work->pitchPhase = work->unknown_1E0 = 0.0f;
                    work->rollPhase = 1.0f;
                    work->movementState = 1;
                    work->targetX = (f32) state->state.actor.position.x;
                    state->state.actor.collision.shape.dynamic.movable = 0.0f;
                    state->state.actor.collision.shape.dynamic.radius = 70.0f;
                    work->soundHandle = fn_1_49C8(2232, &state->state.actor.position, 1, 0);
                case 4:
                    state->state.actor.position.x =
                        (f32) ((f64) state->state.actor.position.x +
                               (0.1 * (f64) (work->targetX - state->state.actor.position.x)));
                    if ((s32) work->finished != 0) {
                        work->phase = 6U;
                        work->frame = 0;
                        work->state.actor.collision.header.flags = 0;
                            Hu3DModelAttrSet(*object->mdlId, HU3D_ATTR_DISPOFF);
                            Hu3DModelAttrReset((s16) work->modelId, HU3D_ATTR_DISPOFF);
                        Hu3DModelAttrReset((s16) work->modelId, HU3D_MOTATTR_PAUSE);
                        Hu3DModelPosSetV((s16) work->modelId,
                                         (Point3d *) &state->state.actor.position);
                        if ((s32) work->soundHandle != -1) {
                            HuAudFXFadeOut(work->soundHandle, 100);
                            work->soundHandle = -1;
                        }
                    } else {
                        selectedRecordIndex = fn_1_AE34(object, &state->state.actor.position);
                        if (selectedRecordIndex >= 0) {
                            record = &lbl_1_bss_5EC[selectedRecordIndex];
                            if (((u32) (record->propMask & propMask) == 0) &&
                                !(fabs(state->state.actor.position.z - record->position.z) >
                                  30.0)) {
                                record->propMask |= propMask;
                                if ((s32) work->contactState <= 0) {
                                    if ((s32) record->active == 1) {
                                        work->wobbleAmount += 90.0;
                                        work->pitchPhase = 1.0f;
                                        work->rollPhase = 0.0f;
                                    } else {
                                        work->wobbleAmount -= 90.0;
                                        work->bobPhase = (f32) -work->bobPhase;
                                        work->pitchPhase = -1.0f;
                                        work->rollPhase = 0.0f;
                                    }
                                    record = &lbl_1_bss_5EC[record->next];
                                    record->propMask |= propMask;
                                    work->phase = (u32) (work->phase + 1);
                                    work->actionIndex = fn_1_F7DC(&state->state.actor.position);
                                    state->state.actor.collision.shape.dynamic.movable = 1.0f;
                                    state->state.actor.collision.shape.dynamic.radius = 50.0f;
                                    work->targetZ = (f32) record->position.z;
                                    work->movementState = 0;
                                case 5:
                                    state->state.actor.position.z =
                                        (f32) ((f64) state->state.actor.position.z +
                                               (0.1 * (f64) (work->targetZ -
                                                             state->state.actor.position.z)));
                                    if (work->pitchPhase < 0.0) {
                                        work->targetX =
                                            (f32) lbl_1_rodata_4C4[work->actionIndex - 1].x;
                                        if (state->state.actor.position.x > work->targetX) {
                                            break;
                                        }
                                        work->wobbleAmount += 90.0;
                                        work->bobPhase = (f32) -work->bobPhase;
                                    } else {
                                        work->targetX =
                                            (f32) lbl_1_rodata_4C4[work->actionIndex + 1].x;
                                        if (state->state.actor.position.x < work->targetX) {
                                            break;
                                        }
                                        work->wobbleAmount -= 90.0;
                                    }
                                    work->pitchPhase = 0.0f;
                                    work->rollPhase = 1.0f;
                                    work->movementState = 1;
                                    state->state.actor.collision.shape.dynamic.movable = 0.0f;
                                    state->state.actor.collision.shape.dynamic.radius = 70.0f;
                                    work->phase = (u32) (work->phase - 1);
                                }
                            }
                        }
                    }
                }
            }
        }
        break;
    case 6:
        if (++work->frame >= 30) {
            work->phase = (u32) (work->phase + 1);
            work->frame = 0;
            state->state.poseEnabled = 0;
        case 7:
            if ((s32) (work->frame & 2) == 0) {
                Hu3DModelAttrSet((s16) work->modelId, HU3D_ATTR_DISPOFF);
            } else {
                Hu3DModelAttrReset((s16) work->modelId, HU3D_ATTR_DISPOFF);
            }
            if (++work->frame >= 24) {
                work->phase = (u32) (work->phase + 1);
                Hu3DModelAttrSet((s16) work->modelId, 1U);
            }
        }
        break;
    }
    if ((s32) state->state.actor.collision.shape.dynamic.active != 0) {
        if (0.0 != (work->movementState == 0)) {
            if (work->pitchPhase < 0.0) {
                work->actionIndex = (s32) (work->actionIndex - 1);
            }
            if (work->pitchPhase > 0.0) {
                work->actionIndex = (s32) (work->actionIndex + 1);
            }
            state->state.actor.velocity.x = work->pitchPhase = -work->pitchPhase;
        }
        fn_1_8888(state->state.actor.playerSlot);
    }
    state->state.actor.rotation.y =
        fn_1_52C0(state->state.actor.rotation.y, work->wobbleAmount, 0.2f);
    if ((s32) work->settleTimer > 0) {
        work->settleTimer = (s32) (work->settleTimer - 1);
        desiredVelocityX = desiredVelocityZ = 0.0f;
    } else {
        desiredVelocityX = work->pitchPhase * movementSpeed;
        desiredVelocityZ = work->rollPhase * movementSpeed;
    }
    state->state.actor.velocity.x =
        (f32) ((f64) state->state.actor.velocity.x +
               (0.3 * (f64) (desiredVelocityX - state->state.actor.velocity.x)));
    state->state.actor.velocity.z =
        (f32) ((f64) state->state.actor.velocity.z +
               (0.3 * (f64) (desiredVelocityZ - state->state.actor.velocity.z)));
    if (state->state.actor.position.z > 5400.0) {
        work->outOfRange = 1;
        work->pitchPhase = 0.0f;
        work->unknown_1E0 = 0.0f;
        work->rollPhase = 0.0f;
    }
    PSVECAdd((Point3d *) &state->state.actor.position, (Point3d *) &state->state.actor.velocity,
             (Point3d *) &state->state.actor.position);
    /* Both movement components accumulate into X rotation; Z rotation is left unchanged here. */
    state->state.actor.rotation.x =
        (f32) ((f64) state->state.actor.rotation.x +
               (0.01 * (100.0 * (f64) state->state.actor.velocity.z * (f64) work->bobPhase)));
    state->state.actor.rotation.x =
        (f32) ((f64) state->state.actor.rotation.x +
               (0.01 * (100.0 * (f64) state->state.actor.velocity.x * (f64) work->bobPhase)));
    if ((s32) work->contactState > 0) {
        work->contactState = (s32) (work->contactState - 1);
    }
}

void (*lbl_1_data_6D0[4])(M676ParticleRecord *record) = {
    fn_1_D030, fn_1_D1E4, fn_1_D320, fn_1_D488
};
Vec lbl_1_data_6E0[6] = {
    {0.0f, 151.0f, 5550.0f},
    {-400.0f, 151.0f, 4600.0f},
    {0.0f, 151.0f, 3800.0f},
    {400.0f, 151.0f, 2600.0f},
    {0.0f, 151.0f, 1450.0f},
    {0.0f, 151.0f, -200.0f}
};
f32 lbl_1_data_728[6] = {150.0f, 150.0f, 165.0f, 204.0f, 165.0f, 150.0f};

/* On prop contacts, may mark the first prop; for other dynamic collider types, marks the other
 * collider and copies the prop's player slot into its contact ID. */
s32 fn_1_BE3C(M676CollisionEntry *first, M676CollisionEntry *second)
{
    OMOBJ *firstObject;
    OMOBJ *secondObject;
    M676PropWork *firstState;
    M676PropWork *secondState;
    M676PropWork *firstWork;
    M676PropWork *secondWork;
    OMOBJ *fallbackObject;
    M676PropWork *fallbackWork;
    f64 direction;
    f32 directionValue;

    if (second->header.state == 0) {
        if (second->header.type == 1) {
            firstObject = first->header.callbackArgument;
            firstState = firstObject->data;
            firstWork = firstState;
            secondObject = second->header.callbackArgument;
            secondState = secondObject->data;
            secondWork = secondState;
            if (firstState->movementState != 0) {
                if (secondState->movementState != 0) {
                    if (firstWork->state.actor.position.z < secondWork->state.actor.position.z) {
                        firstState->settleTimer = 5;
                    }
                } else {
                    firstState->contactState = 10;
                }
                goto apply_first_result;
            }
                if (secondState->movementState != 0) {
                    if (firstWork->state.actor.position.x + firstState->pitchPhase <
                        secondWork->state.actor.position.x) {
                        direction = 1.0;
                    } else {
                        direction = -1.0;
                    }
                directionValue = (f32)direction;
                if (directionValue * firstState->pitchPhase < 0.0) {
                    return 1;
                }
                goto apply_first_result;
            }
apply_first_result:
            first->shape.dynamic.active = 1;
            first->contactId = secondState->state.actor.playerSlot;
            fn_1_4754(2233, &firstWork->state.actor.position);
            goto done;
        }
        fallbackObject = first->header.callbackArgument;
        fallbackWork = fallbackObject->data;
        second->shape.dynamic.active = 1;
        second->contactId = fallbackWork->state.actor.playerSlot;
    }
done:
    return 1;
}

/* Saves position with Y forced to 3 plus the shared initial height, updates the model, and clears
 * ended motion. */
void fn_1_BFA4(OMOBJ *object)
{
    M676PropWork *initialWork;
    M676PropWork *workForCopy;
    M676PropWork *transformWork;
    M676PropWork *transformState;
    M676PropWork *motionWork;
    M676PropWork *motionState;
    s32 transformModelId;
    s32 resetMotion;
    s32 motionModelId;
    f32 angle;
    f32 reducedAngle;
    f32 storedAngle;

    initialWork = object->data;
    workForCopy = initialWork;
    workForCopy->state.savedPosition = workForCopy->state.actor.position;
    workForCopy->state.savedPosition.y = (f32)(3.0 + lbl_1_data_2D0[1].y);

    transformWork = object->data;
    transformState = transformWork;
    transformModelId = *object->mdlId;
    angle = transformState->state.actor.rotation.y;
    reducedAngle = (f32)fmod((f64)angle, 360.0);
    storedAngle = reducedAngle;
    transformState->state.actor.rotation.y = storedAngle;
    Hu3DModelPosSetV((s16)transformModelId, &transformState->state.actor.position);
    Hu3DModelRotSetV((s16)transformModelId, &transformState->state.actor.rotation);

    motionWork = object->data;
    motionState = motionWork;
    if (motionState->state.motionActive != 0) {
        motionModelId = *object->mdlId;
        if (Hu3DMotionEndCheck((s16)motionModelId) &&
            Hu3DMotionShiftIDGet((s16)motionModelId) < 0) {
            resetMotion = 1;
        } else {
            resetMotion = 0;
        }
        if (resetMotion != 0) {
            motionState->state.motionActive = 0;
            motionState->state.motionStatus = 0;
        }
    } else {
        motionState->state.motionStatus = 0;
    }
    motionWork->timer++;
}

/* Scene setup clears the five lane point totals and all 15-by-5 occupancy entries. */
void fn_1_C110(OMOBJ *object)
{
    s32 laneIndex;
    s32 categoryIndex;
    s32 *entry;

    lbl_1_bss_5D4 = 0;
    for (laneIndex = 0; laneIndex < 5; laneIndex++) {
        lbl_1_bss_494[laneIndex] = 0;
    }
    for (categoryIndex = 0; categoryIndex < 15; categoryIndex++) {
        for (laneIndex = 0; laneIndex < 5; laneIndex++) {
            entry = &lbl_1_bss_4A8[laneIndex + categoryIndex * 5];
            *entry = 0;
        }
    }
}

/* Ranks the five lane point totals in descending order and marks the ranking ready for prop
 * selection. */
void fn_1_C1B8(void)
{
    s32 values[5];
    s32 *indices = lbl_1_bss_480;
    s32 outerIndex;
    s32 leftIndex;
    s32 rightIndex;
    s32 value;

    for (outerIndex = 0; outerIndex < 5; outerIndex++) {
        values[outerIndex] = lbl_1_bss_494[outerIndex];
        indices[outerIndex] = outerIndex;
    }
    for (outerIndex = 0; outerIndex < 5; outerIndex++) {
        for (leftIndex = 0; leftIndex < 4; leftIndex++) {
            for (rightIndex = leftIndex + 1; rightIndex < 5; rightIndex++) {
                if (values[leftIndex] < values[rightIndex]) {
                    value = values[leftIndex];
                    values[leftIndex] = values[rightIndex];
                    values[rightIndex] = value;
                    value = indices[leftIndex];
                    indices[leftIndex] = indices[rightIndex];
                    indices[rightIndex] = value;
                }
            }
        }
    }
    lbl_1_bss_5D4 = 1;
}

const Vec lbl_1_rodata_558[5] = {
    {-800.0f, 180.0f, 750.0f},
    {-400.0f, 180.0f, 750.0f},
    {0.0f, 180.0f, 750.0f},
    {400.0f, 180.0f, 750.0f},
    {800.0f, 180.0f, 750.0f}
};

/* Scene setup reserves a random empty row and sector and adds to that row's count. */
s32 fn_1_C2E4(Vec *position, s32 increment)
{
    s32 row;
    s32 sector;
    s32 occupiedIndex;

    do {
        row = (s32)(5.0 * (1.52587890625e-5 *
                           (f64)(f32)(s32)(frand() & 0xFFFF)));
        sector = (s32)(15.0 * (1.52587890625e-5 *
                               (f64)(f32)(s32)(frand() & 0xFFFF)));
        occupiedIndex = row + sector * 5;
    } while (lbl_1_bss_4A8[occupiedIndex] != 0);

    lbl_1_bss_4A8[occupiedIndex] = 1;
    *position = lbl_1_rodata_558[row];
    position->z += 300.0 * (f64)(f32)sector;
    lbl_1_bss_494[row] += increment;
    return occupiedIndex;
}

/* Probes occupancy at the offset depth; an occupied sector snaps Z to its start plus the requested
 * position offset. */
void fn_1_C4A4(Vec *position, f32 depthOffset, f32 positionDelta)
{
    s32 row;
    s32 sector;
    f32 probeZ;

    row = fn_1_F7DC(position);
    probeZ = position->z + depthOffset;
    sector = (s32)((f64)(probeZ - lbl_1_rodata_558[row].z) /
                   300.0);
    if (sector >= 0 && sector < 15 &&
        lbl_1_bss_4A8[sector * 5 + row] != 0) {
        position->z = (f32)(300.0 * (f32)sector +
                            lbl_1_rodata_558[row].z);
        position->z += positionDelta;
    }
}

/* Checks occupancy at the requested depth offset, then 150 units behind the updated position,
 * using the same final offset. */
void fn_1_C5D8(Vec *position, f32 depthOffset)
{
    fn_1_C4A4(position, depthOffset, depthOffset);
    fn_1_C4A4(position, -150.0f, depthOffset);
}

/* Scene setup creates the initial particle records for each occupied lane sector. */
void fn_1_C800(OMOBJ *owner)
{
    Vec position;
    s32 iteration;
    s32 occupiedIndex;

    for (iteration = 0; iteration < 30; iteration++) {
        occupiedIndex = fn_1_C2E4(&position, 1);
        fn_1_10288(0, 0, &position, occupiedIndex);
    }

    for (iteration = 0; iteration < 2; iteration++) {
        occupiedIndex = fn_1_C2E4(&position, 5);
        fn_1_10288(1, 0, &position, occupiedIndex);
    }
}

/* Allocates the manager's particle records and cached models, stores their render offsets, and
 * registers initially disabled dynamic colliders; object->work[0] supplies the record count. */
void fn_1_CB74(OMOBJ *object, s32 resourceId, s32 collisionType,
    f32 heightOffset, f32 storedValue)
{
    M676PropWork *work = object->data;
    s32 recordCount = object->work[0];
    s32 allocationSize = recordCount * sizeof(M676ParticleRecord);
    M676ParticleRecord *records = HuMemDirectMallocNum(
        HEAP_HEAP, allocationSize, HU_MEMNUM_OVL);
    s32 index;

    memset(records, 0, allocationSize);
    work->records = records;
    for (index = 0; index < recordCount; index++, records++) {
        M676CollisionEntry *callback;

        records->modelId = fn_1_50A8(fn_1_1480(resourceId));
        Hu3DModelLayerSet((s16)records->modelId, 4);
        Hu3DModelAttrSet((s16)records->modelId, HU3D_ATTR_NOCULL);
        Hu3DModelAttrSet((s16)records->modelId, 1U);
        records->heightOffset = heightOffset;
        records->storedValue = storedValue;
        callback = &records->collision;

        callback->header.flags = 0;
        callback->header.collisionMask = M676_COLLISION_MASK_PARTICLE;
        callback->header.type = collisionType;
        callback->header.id = index;
        callback->header.state = 0;
        callback->header.callbackArgument = object;
        callback->header.updateCallback = 0;
        callback->shape.dynamic.callback = 0;
        callback->shape.dynamic.position = &records->position;
        callback->shape.dynamic.velocity = &records->velocity;
        callback->shape.dynamic.currentPosition = *callback->shape.dynamic.position;
        callback->shape.dynamic.radius = 50.0f;
        callback->shape.dynamic.movable = 1.0f;
        fn_1_13EC4(callback);
    }
}

/* Converts a player's score-box screen point into a world-space target for particle movement. */
void fn_1_CCE0(s32 playerIndex, Point3d *destination)
{
    Point3d source;

    fn_1_1263C(playerIndex, &source.x, &source.y);
    source.z = 1200.0f;
    source.y = source.y - 10.0;
    Hu3D2Dto3D(&source, 1, destination);
}

/* Scene setup configures the main particle object with 50 records and its update callback. */
void fn_1_CD50(OMOBJ *object)
{
    lbl_1_bss_E98 = object;
        object->work[0] = 50;
    fn_1_CB74(object, 1, 4, 50.0f, 0.0f);
    object->objFunc = fn_1_CE30;
}

/* Scene setup configures the secondary particle object with four records and its update
 * callback. */
void fn_1_CDC0(OMOBJ *object)
{
    lbl_1_bss_E94 = object;
    object->work[0] = 4;
    fn_1_CB74(object, 2, 5, -10.0f, 0.0f);
    object->objFunc = fn_1_CE30;
}

/* Particle-object callback advances active records and updates each record's model every frame. */
void fn_1_CE30(OMOBJ *owner)
{
    s32 values[5];
    M676ParticleRecord *record;
    M676PropWork *list;
    s32 recordCount;
    s32 index;
    s32 recordIndex;
    s32 left;
    s32 right;
    s32 value;

    list = owner->data;
    record = list->records;
    recordCount = owner->work[0];
    if (lbl_1_bss_5D4 == 0) {
        s32 *indices = lbl_1_bss_480;

        for (index = 0; index < 5; index++) {
            values[index] = lbl_1_bss_494[index];
            indices[index] = index;
        }
        for (index = 0; index < 5; index++) {
            for (left = 0; left < 4; left++) {
                for (right = left + 1; right < 5; right++) {
                    if (values[left] < values[right]) {
                        value = values[left];
                        values[left] = values[right];
                        values[right] = value;
                        value = indices[left];
                        indices[left] = indices[right];
                        indices[right] = value;
                    }
                }
            }
        }
        lbl_1_bss_5D4 = 1;
    }

    for (recordIndex = 0; recordIndex < recordCount;
         recordIndex++, record++) {
        if (record->state != 0) {
            (*lbl_1_data_6D0[record->type])(record);
            PSVECAdd(&record->position, &record->velocity,
                     &record->position);
            PSVECAdd(&record->rotation, &record->rotationVelocity,
                     &record->rotation);
            Hu3DModelPosSet((s16)record->modelId, record->position.x,
                            record->position.y + record->heightOffset,
                            record->position.z);
            Hu3DModelRotSetV((s16)record->modelId, &record->rotation);
            Hu3DModelScaleSet((s16)record->modelId, record->scale,
                              record->scale, record->scale);
        }
    }
}

/* Particle update waits for contact, then moves the record to the player's score box and awards a
 * point. */
void fn_1_D030(M676ParticleRecord *state)
{
    Point3d projected;
    s32 playerIndex;

    switch (state->state) {
    case 1:
        fn_1_D63C(state);
        state->state++;
        state->remainingEmissions = 10;
        /* fall through */
    case 2:
        if (state->collision.shape.dynamic.active == 0) {
            return;
        }
        state->state++;
        state->timer = 0.0f;
        state->velocity.y = 0.0f;
        state->rotationVelocity.y = 10.0f;
        state->previousPosition = state->position;
        state->playerIndex = state->collision.contactId;
        state->collision.header.flags &= ~1u;
        fn_1_4754(2235, &state->position);
        return;
    case 3:
        state->timer += 0.02;
        playerIndex = state->playerIndex;
        fn_1_1263C(playerIndex, &projected.x, &projected.y);
        projected.z = 1200.0f;
        projected.y -= 10.0;
        Hu3D2Dto3D(&projected, 1, &state->projectedPosition);
        fn_1_5698(&state->previousPosition, &state->projectedPosition,
            state->timer, &state->position);
        if (!(state->timer < 1.0)) {
            fn_1_12514(state->playerIndex, 1);
            fn_1_D724(state);
            fn_1_D768(state);
            state->state = 0;
        }
        break;
    }
}

/* Particle update waits for contact, then schedules five player-indexed events before cleanup. */
void fn_1_D1E4(M676ParticleRecord *state)
{
    switch (state->state) {
    case 1:
        fn_1_D63C(state);
        state->state++;
        state->remainingEmissions = 10;
        /* fall through */
    case 2:
        if (state->collision.shape.dynamic.active == 0) {
            return;
        }
        state->state++;
        state->timer = 0.0f;
        state->remainingEmissions = 5;
        state->velocity.y = 0.0f;
        state->playerIndex = state->collision.contactId;
        fn_1_D724(state);
        return;
    case 3:
        if (!((state->timer = (f32)((f64)state->timer - 1.0)) >= 0.0)) {
            state->timer = 9.0f;
            fn_1_10288(2, state->playerIndex, &state->position, -1);
            if (--state->remainingEmissions <= 0) {
                fn_1_D768(state);
                state->state = 0;
            }
        }
        break;
    }
}

/* A type-2 request begins flying to its assigned player's score box after setup and awards one
 * point on arrival. */
void fn_1_D320(M676ParticleRecord *state)
{
    Point3d projected;
    s32 playerIndex;

    switch (state->state) {
    case 1:
        fn_1_D63C(state);
        state->state++;
        state->timer = 0.0f;
        state->rotationVelocity.y = 10.0f;
        state->previousPosition = state->position;
        state->collision.header.flags &= ~1u;
        fn_1_4754(2235, &state->position);
        break;
    case 2:
        state->timer += 0.02;
        playerIndex = state->playerIndex;
        fn_1_1263C(playerIndex, &projected.x, &projected.y);
        projected.z = 1200.0f;
        projected.y -= 10.0;
        Hu3D2Dto3D(&projected, 1, &state->projectedPosition);
        fn_1_5698(&state->previousPosition, &state->projectedPosition,
            state->timer, &state->position);
        if (!(state->timer < 1.0)) {
            fn_1_12514(state->playerIndex, 1);
            fn_1_D724(state);
            state->state = 0;
        }
        break;
    }
}

/* Particle update waits for contact, then moves the record to the player's score box and awards a
 * point. */
void fn_1_D488(M676ParticleRecord *state)
{
    Point3d projected;
    s32 playerIndex;

    switch (state->state) {
    case 1:
        fn_1_D63C(state);
        state->state++;
        state->remainingEmissions = 10;
        /* fall through */
    case 2:
        if (state->collision.shape.dynamic.active == 0) {
            return;
        }
        state->state++;
        state->timer = 0.0f;
        state->velocity.y = 0.0f;
        state->rotationVelocity.y = 10.0f;
        state->previousPosition = state->position;
        state->playerIndex = state->collision.contactId;
        state->collision.header.flags &= ~1u;
        fn_1_4754(2235, &state->position);
        return;
    case 3:
        state->timer += 0.02;
        playerIndex = state->playerIndex;
        fn_1_1263C(playerIndex, &projected.x, &projected.y);
        projected.z = 1200.0f;
        projected.y -= 10.0;
        Hu3D2Dto3D(&projected, 1, &state->projectedPosition);
        fn_1_5698(&state->previousPosition, &state->projectedPosition,
            state->timer, &state->position);
        if (!(state->timer < 1.0)) {
            fn_1_12514(state->playerIndex, 1);
            fn_1_D724(state);
            fn_1_D768(state);
            state->state = 0;
        }
        break;
    }
}

/* Particle setup resets motion and collision state before the record becomes active. */
void fn_1_D63C(M676ParticleRecord *record) {
    record->scale = 1.0f;
    record->velocity.x = 0.0f;
    record->velocity.y = 0.0f;
    record->velocity.z = 0.0f;
    record->rotation.x = 0.0f;
    record->rotation.y = 0.0f;
    record->rotation.z = 0.0f;
    record->rotationVelocity.x = 0.0f;
    record->rotationVelocity.y = 0.0f;
    record->rotationVelocity.z = 0.0f;
    record->collision.header.flags = 1;
    record->collision.shape.dynamic.active = 0;
    Hu3DModelAttrReset((s16)record->modelId, 1U);
}

/* Disables particle collision and hides its model during collection. */
void fn_1_D724(M676ParticleRecord *record) {
    record->collision.header.flags &= ~1u;
    Hu3DModelAttrSet((s16)record->modelId, 1U);
}

/* Particle cleanup releases its lane slot when the record has a valid area index. */
void fn_1_D768(M676ParticleRecord *state) {
    if (state->areaIndex >= 0) {
        lbl_1_bss_4A8[state->areaIndex] = 0;
    }
}

/* Prop setup selects its initial motion and installs the per-frame actor update. */
void fn_1_D794(OMOBJ *object)
{
    M676PropWork *initialWork;
    M676PropWork *animationWork;
    s32 initialModel;

    initialWork = object->data;
    animationWork = initialWork;
    initialModel = object->mdlId[0];
    fn_1_8F2C(object, 0);
    animationWork->state.savedPosition = animationWork->state.actor.position;
    animationWork->state.poseEnabled = 1;
    animationWork->state.poseHeight = 65.0f;
    animationWork->state.savedPositionPointer = &animationWork->state.savedPosition;
    object->objFunc = fn_1_D8E4;
}

/* Per-frame actor callback advances its scripted movement sequence and applies its transform. */
void fn_1_D8E4(OMOBJ *object)
{
    M676PropWork *state;
    M676PropWork *work = object->data;

    state = work;
    fn_1_8690(&state->state.actor);
    fn_1_6CA4(&state->state.actor);
    switch (work->phase) {
    case 0:
        work->phase++;
        state->state.controlState = 0;
    case 1:
        if (fn_1_DB74(state)) { work->phase++; state->state.controlState = 0; }
        break;
    case 2:
        if (fn_1_E024(state)) { work->phase++; state->state.controlState = 0; }
        break;
    case 3:
        if (fn_1_E184(state)) { work->phase++; state->state.controlState = 0; }
        break;
    }
    fn_1_F548(object);
    PSVECAdd(&state->state.actor.position, &state->state.actor.velocity,
             &state->state.actor.position);
    fn_1_DA08(object);
}

/* Saves position with Y forced to 1 plus the shared initial height, refreshes the model, and clears
 * completed motion. */
void fn_1_DA08(OMOBJ *object)
{
    M676PropWork *initialWork;
    M676PropWork *workForCopy;
    M676PropWork *transformWork;
    M676PropWork *transformState;
    M676PropWork *motionWork;
    M676PropWork *motionState;
    s32 transformModelId;
    s32 resetMotion;
    s32 motionModelId;
    f32 angle;
    f32 reducedAngle;
    f32 storedAngle;

    initialWork = object->data;
    workForCopy = initialWork;
    workForCopy->state.savedPosition = workForCopy->state.actor.position;
    workForCopy->state.savedPosition.y = (f32)(1.0 + lbl_1_data_2D0[1].y);

    transformWork = object->data;
    transformState = transformWork;
    transformModelId = *object->mdlId;
    angle = transformState->state.actor.rotation.y;
    reducedAngle = (f32)fmod((f64)angle, 360.0);
    storedAngle = reducedAngle;
    transformState->state.actor.rotation.y = storedAngle;
    Hu3DModelPosSetV((s16)transformModelId, &transformState->state.actor.position);
    Hu3DModelRotSetV((s16)transformModelId, &transformState->state.actor.rotation);

    motionWork = object->data;
    motionState = motionWork;
    if (motionState->state.motionActive != 0) {
        motionModelId = *object->mdlId;
        if (Hu3DMotionEndCheck((s16)motionModelId) &&
            Hu3DMotionShiftIDGet((s16)motionModelId) < 0) {
            resetMotion = 1;
        } else {
            resetMotion = 0;
        }
        if (resetMotion != 0) {
            motionState->state.motionActive = 0;
            motionState->state.motionStatus = 0;
        }
    } else {
        motionState->state.motionStatus = 0;
    }
    motionWork->timer++;
}

/* The actor update calls this sequence step to run the opening route through its lane markers. */
s32 fn_1_DB74(M676PropWork *work)
{
    M676PropWork *state = work;
    s32 stepCount;
    s32 done;

    stepCount = 6;
    done = 0;

    switch (work->state.controlState) {
    case 0:
        lbl_1_bss_E8C = 1;
        work->state.controlState++;
        work->state.controlTimer = 0;
        work->state.actor.position = lbl_1_data_6E0[0];
        work->state.actor.rotation.y = 0.0f;
        /* fall through */
    case 1:
        if (++work->state.controlTimer < 18) {
            break;
        }
        work->state.controlState++;
        work->state.controlTimer = 0;
        fn_1_6CC0(&work->state.actor, 7, 1.0f);
        work->state.motionActive = 1;
        /* fall through */
    case 2:
        if (++work->state.controlTimer > 180) {
            lbl_1_bss_E88 = 1;
        }
        if (work->state.motionActive != 0) {
            break;
        }
        work->state.controlState++;
        work->state.controlTimer = 1;
        lbl_1_bss_E88 = 1;
        /* fall through */
    case 3:
        work->state.controlState++;
        work->state.movementState = 0;
        work->state.targetPosition = lbl_1_data_6E0[work->state.controlTimer];
        fn_1_C5D8(&work->state.targetPosition, lbl_1_data_728[work->state.controlTimer]);
        /* fall through */
    case 4:
    {
        s32 angleMode = (work->state.controlTimer == 1) ? 1 : 0;

        if (fn_1_F214(work, 1.0896277f, 500.0f, 6,
                angleMode) == 0) {
            break;
        }
        if (++work->state.controlTimer < stepCount) {
            lbl_1_bss_E8C = 0;
            work->state.controlState--;
            break;
        }
        work->state.controlState++;
        work->state.controlTimer = 0;
        work->state.movementState = 0;
        work->state.targetAngle = 0.0f;
        /* fall through */
    }
    case 5:
        if (fn_1_EC00(work, 0.3f) == 0) {
            break;
        }
        work->state.controlState++;
        work->state.controlTimer = 0;
        lbl_1_bss_E8C = 0;
        /* fall through */
    case 6:
        if (MgSeqModeGet() != 5) { break; }
        work->state.controlState++;
        work->state.controlTimer = 0;
        /* fall through */
    default:
        done = 1;
        break;
    }
    return done;
}

M676SequenceStep lbl_1_data_75C[3] = {fn_1_E260, fn_1_E4D4, fn_1_EAC8};
Vec lbl_1_data_768 = {0.0f, 151.0f, -200.0f};

/* The actor sequence calls this step to run its three scripted actions in order. */
s32 fn_1_E024(M676PropWork *state)
{
    M676PropWork *work = state;
    s32 done = 0;
    s32 stepCount = 3;

    switch (state->state.controlState) {
    case 0:
        state->state.controlState++;
        state->state.controlTimer = 0;
        work->actionIndex = 0;
        state->state.sequenceState = 0;
        /* fall through */
    case 1:
        if (lbl_1_data_75C[work->actionIndex](state) != 0) {
            state->state.sequenceState = 0;
            if (++work->actionIndex < stepCount) {
                break;
            }
            state->state.controlState++;
            state->state.actor.velocity.x = 0.0f;
            state->state.actor.velocity.y = 0.0f;
            state->state.actor.velocity.z = 0.0f;
            state->state.actor.rotation.x = 0.0f;
            state->state.actor.rotation.y = 0.0f;
            state->state.actor.rotation.z = 0.0f;
            fn_1_6CC0(&state->state.actor, 0, 1.0f);
        default:
            done = 1;
        }
        break;
    }
    return done;
}

/* The actor sequence waits for the scene event, plays its motion, then finishes the step. */
s32 fn_1_E184(M676PropWork *sequence)
{
    s32 done = 0;

    switch (sequence->state.controlState) {
    case 0:
        sequence->state.controlState++;
        /* fall through */
    case 1:
        if (fn_1_1650() == 0) {
            break;
        }
        sequence->state.controlState++;
        sequence->state.controlTimer = 0;
        fn_1_6CC0(&sequence->state.actor, 6, 1.0f);
        /* fall through */
    case 2:
        if (++sequence->state.controlTimer < 78) {
            break;
        }
        sequence->state.controlState++;
        sequence->state.controlTimer = 0;
        fn_1_4754(MSM_SE_GUIDE_11, &sequence->state.actor.position);
        /* fall through */
    default:
        done = 1;
        break;
    }
    return done;
}

/* Oscillates around its marker, then attempts a yaw-zero turn for 30 frames before completing;
 * the turn-completion result is ignored. */
s32 fn_1_E260(M676PropWork *work)
{
    s32 done = 0;
    f32 direction = 2.0f;

    switch (work->state.sequenceState) {
    case 0:
    {
        work->state.sequenceState++;
        work->state.sequenceTimer = 0;
        work->state.movementState = 0;
        work->state.targetAngle = 0.0f;
        work->state.targetPosition = lbl_1_data_768;
        if ((BOOL)(frand() & 1) != 0) {
            work->state.sequenceState++;
        }
        /* The first call falls through this negation even if phase 2 was chosen; the stored phase
         * controls later calls. */
    }
    case 1:
        direction = -direction;
        /* fall through */
    case 2: {
        f32 angleScale;
        f32 horizontalOffset;

        angleScale = (f32)work->state.sequenceTimer++ * direction;
        horizontalOffset = (f32)(100.0 *
            (2.5 * sin(
                (3.141592653589793 * (f64)angleScale) /
                180.0)));
        work->state.targetPosition.x =
            lbl_1_data_768.x + horizontalOffset;
        fn_1_ED6C(work, 5.0f);
        if (fn_1_FCA0() < 450.0) {
            break;
        }
        /* Leaving phase 1 advances only to phase 2: the turn below runs now, then the next update
        * repeats this movement block before entering the 30-frame turn hold. */
        work->state.sequenceState++;
        work->state.sequenceTimer = 0;
        work->state.movementState = 0;
        work->state.targetAngle = 0.0f;
        work->state.actor.velocity.x = 0.0f;
        work->state.actor.velocity.y = 0.0f;
        work->state.actor.velocity.z = 0.0f;
        fn_1_6CC0(&work->state.actor, 0, 1.0f);
        /* fall through */
    }
    case 3:
        fn_1_EC00(work, 0.2f);
        if (++work->state.sequenceTimer < 30) {
            break;
        }
        work->state.sequenceState++;
        /* fall through */
    default:
        done = 1;
        break;
    }
    return done;
}

/* Jumps between lane markers based on prop depth; post-jump phases check whether gameplay
 * (mode 5) has ended before completing. */
s32 fn_1_E4D4(M676PropWork *work)
{
    s32 done = 0;
    f32 shallowestDepth;

    switch (work->state.sequenceState) {
    case 0:
        work->state.sequenceState++;
        work->state.sequenceTimer = 0;
        work->state.movementState = 0;
        work->state.targetAngle = 0.0f;
        fn_1_6CC0(&work->state.actor, 0, 1.0f);
        /* fall through */
    case 1:
        if (fn_1_EC00(work,
                0.2f) == 0 || MgSeqModeGet() != 5) {
            break;
        }
        if (++work->state.sequenceTimer < 30) {
            break;
        }
        work->state.sequenceState++;
        work->state.sequenceTimer = 0;
        /* fall through */
    case 2:
        if (++work->state.sequenceTimer < 18) {
            break;
        }
        {
            shallowestDepth = fn_1_FCA0();

            if ((f64)shallowestDepth > 5000.0) {
                shallowestDepth = 5000.0f;
            }
            if ((f64)(shallowestDepth - work->state.actor.position.z) < 150.0) {
                break;
            }
            {
                s32 areaOffset = fn_1_F7DC(&work->state.actor.position);
                s32 selectedArea = areaOffset + (s32)(2.0 *
                    (3.0517578125e-05 *
                        (f64)(f32)((s32)(frand() & 0xFFFF) - 32768)));

                if (selectedArea < 0) {
                    selectedArea = 0;
                }
                if (selectedArea >= 5) {
                    selectedArea = 4;
                }
                if (selectedArea == areaOffset) {
                    if (selectedArea < 2) {
                        selectedArea++;
                    } else {
                        selectedArea--;
                    }
                }
                work->state.sequenceState++;
                work->state.sequenceTimer = 0;
                work->state.movementState = 0;
                work->state.targetPosition.x =
                    lbl_1_rodata_4C4[selectedArea].x;
                work->state.targetPosition.y = lbl_1_data_2D0[1].y;
                work->state.targetPosition.z =
                    (f32)((f64)shallowestDepth - 150.0);
                /* Apply the same occupancy adjustment twice; the second check uses any depth
                 * change made by the first. */
                fn_1_C4A4(&work->state.targetPosition,
                    -150.0f, -150.0f);
                fn_1_C4A4(&work->state.targetPosition,
                    -150.0f, -150.0f);
            }
        }
        /* fall through */
    case 3:
        if (fn_1_F214(work,
                1.0896277f, 500.0f, 12, 1) == 0) {
            break;
        }
        work->state.sequenceState++;
        work->state.movementState = 0;
        work->state.targetAngle = 0.0f;
        /* fall through */
    case 4:
        if (MgSeqModeGet() != 5) {
            fn_1_6CC0(&work->state.actor, 0, 1.0f);
            done = 1;
        } else if (fn_1_EC00(work,
                0.2f) != 0) {
            shallowestDepth = fn_1_FCA0();
            if ((f64)(shallowestDepth - work->state.actor.position.z) > 350.0) {
                work->state.sequenceState = 2;
                fn_1_6CC0(&work->state.actor, 0, 1.0f);
            } else {
                work->state.sequenceState++;
                work->state.sequenceTimer = 0;
                work->state.movementState = 0;
                HuAudFXPlayVolPan(MSM_SE_GUIDE_11, 88, fn_1_3DD0(&work->state.actor.position));
                /* fall through */
            case 5:
                fn_1_6CC0(&work->state.actor, 2, 1.0f);
                /* fall through */
            case 6:
                if (MgSeqModeGet() != 5) {
                    fn_1_6CC0(&work->state.actor, 0, 1.0f);
                    done = 1;
                } else {
                    if (++work->state.sequenceTimer >= 90) {
                        work->state.sequenceState = 2;
                        fn_1_6CC0(&work->state.actor, 0, 1.0f);
                    }
                }
            }
        }
        break;
    default:
        done = 1;
        break;
    }
    return done;
}

Vec lbl_1_data_790 = {0.0f, 151.0f, 5200.0f};

/* Waits for the pre-winner phase, moves to the final marker, then turns to yaw zero. */
s32 fn_1_EAC8(M676PropWork *work)
{
    s32 done = 0;

    switch (work->state.sequenceState) {
    case 0:
        work->state.sequenceState++;
        /* fall through */
    case 1:
        if (MgSeqModeGet() != 7) {
            break;
        }
        work->state.sequenceState++;
        work->state.sequenceTimer = 0;
        work->state.movementState = 0;
        work->state.targetPosition = lbl_1_data_790;
        /* fall through */
    case 2:
        if (fn_1_F214(work, 1.0896277f,
                      500.0f, 18, 1) == 0) {
            break;
        }
        work->state.sequenceState++;
        work->state.movementState = 0;
        work->state.targetAngle = 0.0f;
        /* fall through */
    case 3:
        if (fn_1_EC00(work, 0.2f) == 0) {
            break;
        }
        work->state.sequenceState++;
        /* fall through */
    default:
        done = 1;
        break;
    }
    return done;
}

/* Turns the actor toward its target angle and reports when the turn is complete. */
s32 fn_1_EC00(M676PropWork *work, f32 speed)
{
    s32 done = 0;

    switch (work->state.movementState) {
    case 0:
        work->state.movementState++;
        if (fabs((double)fn_1_53DC(work->state.actor.rotation.y, work->state.targetAngle)) > 30.0) {
            fn_1_6CC0(&work->state.actor, 3, 1.0f);
        }
        /* fall through */
    case 1:
        work->state.actor.rotation.y =
            fn_1_52C0(work->state.actor.rotation.y, work->state.targetAngle, speed);
        if (fabs((double)fn_1_53DC(work->state.actor.rotation.y, work->state.targetAngle)) > 3.0) {
            break;
        }
        work->state.actor.rotation.y = work->state.targetAngle;
        fn_1_6CC0(&work->state.actor, 0, 1.0f);
        /* fall through */
    default:
        done = 1;
        break;
    }
    return done;
}

/* Moves the actor toward its target position, updates its motion, and reports arrival. */
s32 fn_1_ED6C(M676PropWork *work, f32 speed)
{
    s32 stopMotion = 0;
    s32 movingMotion = 3;
    f32 deltaX;
    f32 deltaZ;
    f32 distance;
    f32 targetAngle;
    f32 targetVelocityX;
    f32 targetVelocityZ;

    deltaX = work->state.targetPosition.x - work->state.actor.position.x;
    deltaZ = work->state.targetPosition.z - work->state.actor.position.z;
    distance = sqrtf(deltaX * deltaX + deltaZ * deltaZ);
    if (distance < 30.0) {
        work->state.actor.velocity.x *= 0.8;
        work->state.actor.velocity.z *= 0.8;
        work->state.actor.rotation.y =
            fn_1_52C0(work->state.actor.rotation.y, work->state.targetAngle, 0.2f);
        if (fabs((f64)fn_1_53DC(work->state.actor.rotation.y, work->state.targetAngle)) >
            3.0) {
            return 0;
        }
        fn_1_6CC0(&work->state.actor, stopMotion, 1.0f);
        work->state.actor.rotation.y = work->state.targetAngle;
        work->state.actor.velocity.x = 0.0f;
        work->state.actor.velocity.y = 0.0f;
        work->state.actor.velocity.z = 0.0f;
        return 1;
    }

    targetAngle = (f32)(180.0 *
        (atan2((f64)deltaX, (f64)deltaZ) /
         3.141592653589793));
    work->state.actor.rotation.y = fn_1_52C0(work->state.actor.rotation.y, targetAngle, 0.3f);
    if (distance < 50.0) {
        speed *= 0.5;
    }
    targetVelocityX = (f32)(speed *
        sin((3.141592653589793 * targetAngle) / 180.0));
    work->state.actor.velocity.x = (f32)(work->state.actor.velocity.x +
        (0.3 * (f64)(targetVelocityX - work->state.actor.velocity.x)));
    targetVelocityZ = (f32)(speed *
        cos((3.141592653589793 * targetAngle) / 180.0));
    work->state.actor.velocity.z = (f32)(work->state.actor.velocity.z +
        (0.3 * (f64)(targetVelocityZ - work->state.actor.velocity.z)));
    work->state.footstepDistance = (f32)(work->state.footstepDistance + (f32)speed);
    if (work->state.footstepDistance > 100.0) {
        work->state.footstepDistance = (f32)(work->state.footstepDistance - 100.0);
        HuAudFXPlayVolPan(MSM_SE_GUIDE_09, 88,
            fn_1_3DD0(&work->state.actor.position));
    }
    fn_1_6CC0(&work->state.actor, movingMotion, 1.0f);
    return 0;
}

/* With angleMode set, waits to face the target before seven preparation frames; otherwise turns
* during preparation and flight. Applies gravity, snaps to the target on landing, emits dust,
* and holds for count landing frames before reporting completion. */
s32 fn_1_F214(M676PropWork *work, f32 delta, f32 step,
    s32 count, s32 angleMode)
{
    Vec effectPosition;
    s32 finished = 0;

    switch (work->state.movementState) {
    case 0:
        work->state.movementState++;
        work->state.movementTimer = 0;
        work->state.targetAngle =
            fn_1_5490(&work->state.actor.position, &work->state.targetPosition);
        /* fall through */
    case 1:
        if (angleMode != 0) {
            work->state.actor.rotation.y =
                fn_1_52C0(work->state.actor.rotation.y, work->state.targetAngle, 0.2f);
            if (fabs((f64)fn_1_53DC(work->state.actor.rotation.y, work->state.targetAngle)) > 3.0) {
                break;
            }
            work->state.actor.rotation.y = work->state.targetAngle;
        }
        work->state.movementState++;
        work->state.movementTimer = 7;
        fn_1_6CC0(&work->state.actor, 4, 1.0f);
        /* fall through */
    case 2:
        if (angleMode == 0) {
            work->state.actor.rotation.y =
                fn_1_52C0(work->state.actor.rotation.y, work->state.targetAngle, 0.2f);
        }
        if (--work->state.movementTimer > 0) {
            break;
        }
        work->state.movementState++;
        work->state.movementTimer =
            fn_1_56F0(&work->state.actor.position, &work->state.targetPosition, delta, step,
                      &work->state.actor.velocity);
        work->state.jumpActive = 1;
        fn_1_4754(MSM_SE_GUIDE_69, &work->state.actor.position);
        /* fall through */
    case 3:
        if (angleMode == 0) {
            work->state.actor.rotation.y =
                fn_1_52C0(work->state.actor.rotation.y, work->state.targetAngle, 0.2f);
        }
        work->state.actor.velocity.y -= delta;
        if (work->state.movementTimer == 7) {
            fn_1_6CC0(&work->state.actor, 5, 1.0f);
        }
        if (--work->state.movementTimer > 0) {
            break;
        }
        work->state.movementState++;
        work->state.movementTimer = 0;
        work->state.jumpActive = 0;
        work->state.actor.position = work->state.targetPosition;
        work->state.actor.velocity.x = 0.0f;
        work->state.actor.velocity.y = 0.0f;
        work->state.actor.velocity.z = 0.0f;
        fn_1_6CC0(&work->state.actor, 5, 1.0f);
        effectPosition = work->state.actor.position;
        effectPosition.y += 10.0;
        CharEffectDustCreate(-1, 1.0f, &effectPosition);
        /* fall through */
    case 4:
        if (++work->state.movementTimer < count) {
            break;
        }
        work->state.movementState++;
        /* fall through */
    default:
        finished = 1;
        break;
    }
    return finished;
}

/* The actor update applies a requested motion and its playback speed when one is pending. */
void fn_1_F548(OMOBJ *object)
{
    M676PropWork *state;
    M676PropWork *work = object->data;

    state = work;
    if (state->state.actor.requestedMotion != -1) {
    fn_1_8F2C(object, state->state.actor.requestedMotion);
        Hu3DMotionSpeedSet(object->mdlId[0], state->state.actor.requestedMotionSpeed);
    }
}

/* Writes the model transform twice with yaw wrapping between writes, advances actor motion time,
 * clears completed motion, and advances the prop timer. */
void fn_1_F678(OMOBJ *object)
{
    M676PropWork *initialWork = object->data;
    M676PropWork *workForSetup = initialWork;
    s32 initialModelId = object->mdlId[0];
    M676PropWork *transformWork;
    M676PropWork *transformState;
    s32 transformModelId;
    f32 angle;
    f32 reducedAngle;
    f32 storedAngle;
    M676PropWork *motionWork;
    M676PropWork *motionState;
    s32 resetShift;
    s32 motionModelId;
    s16 shiftedMotion;

    fn_1_86C4(&workForSetup->state.actor, initialModelId);
    transformWork = object->data;
    transformState = transformWork;
    transformModelId = object->mdlId[0];
    angle = transformState->state.actor.rotation.y;
    reducedAngle = (f32)fmod((f64)angle, 360.0);
    storedAngle = reducedAngle;
    transformState->state.actor.rotation.y = storedAngle;
    Hu3DModelPosSetV(transformModelId, &transformState->state.actor.position);
    Hu3DModelRotSetV(transformModelId, &transformState->state.actor.rotation);

    motionWork = object->data;
    motionState = motionWork;
    if (motionState->state.motionActive != 0) {
        motionModelId = object->mdlId[0];
        if (Hu3DMotionEndCheck(motionModelId) &&
            Hu3DMotionShiftIDGet(motionModelId) < 0) {
            resetShift = 1;
        } else {
            resetShift = 0;
        }
        if (resetShift != 0) {
            motionState->state.motionActive = 0;
            motionState->state.motionStatus = 0;
        }
    } else {
        motionState->state.motionStatus = 0;
    }
    motionWork->timer++;
}

s32 fn_1_F7BC(void) {
    return lbl_1_bss_E8C;
}

s32 fn_1_F7CC(void) {
    return lbl_1_bss_E88;
}

/* Returns the horizontal lane index containing the supplied position. */
s32 fn_1_F7DC(Vec *value)
{
    s32 index;

    for (index = 0; index < 4; index++) {
        if (value->x < lbl_1_rodata_46C[index].x) {
            break;
        }
    }
    return index;
}

/* Gameplay checks whether at least four props have been selected. */
s32 fn_1_F828(void)
{
    OMOBJ *object;
    M676PropWork *work;
    s32 count = 0;
    s32 index;

    for (index = 0; index < 5; index++) {
        object = lbl_1_bss_5D8[index];
        if (object != NULL) {
            work = object->data;
            if (work->selectionState != 0) {
                count++;
            }
        }
    }
    return count >= 4;
}

/* Gameplay checks whether at least four props have left the course bounds. */
s32 fn_1_F8B4(void)
{
    OMOBJ *object;
    M676PropWork *work;
    s32 count = 0;
    s32 index;

    for (index = 0; index < 5; index++) {
        object = lbl_1_bss_5D8[index];
        if (object != NULL) {
            work = object->data;
            if (work->outOfRange != 0) {
                count++;
            }
        }
    }
    return count >= 4;
}

/* Returns one prop's out-of-range state and optionally copies its current position. */
s32 fn_1_F940(s32 index, Vec *resultValues)
{
    OMOBJ *object;
    M676PropWork *result;

    object = lbl_1_bss_5D8[index];
    if (object != NULL) {
        result = object->data;
        if (resultValues != NULL) {
            *resultValues = result->state.actor.position;
        }
        return result->outOfRange;
    }
    return 0;
}

/* When a player boards a selected prop, loads the character's face animation, stores the player
 * slot, and marks the prop initialized. */
void fn_1_F9B0(s32 objectIndex, s32 result, s32 character)
{
    OMOBJ *object = lbl_1_bss_5D8[objectIndex];

    if (object != NULL) {
        M676PropWork *work = object->data;
        M676PropWork *state = work;
        void *animationData;
        s32 animationId;
        s32 modelId;

        modelId = object->mdlId[0];
        animationData = HuDataSelHeapReadNum(character + DATANUM(DATA_mgconst, 0),
            HU_MEMNUM_OVL, HEAP_MODEL);
        animationId = Hu3DAnimCreate(animationData, modelId, "kao");
        Hu3DAnimAttrSet(animationId, 1);
        state->state.actor.playerSlot = result;
        work->initialized = 1;
    }
}

/* Returns an available prop in the player's lane within 200 horizontal X/Z units when the player
* faces it within 90 degrees; vertical separation is ignored. */
s32 fn_1_FA68(Vec *position, f32 angle)
{
    s32 index;
    OMOBJ *object;
    M676PropWork *state;
    M676PropWork *work;
    f32 targetAngle;

    index = fn_1_F7DC(position);
    object = lbl_1_bss_5D8[index];
    if (object != NULL) {
        state = object->data;
        work = state;
        if (state->selectionState != 0) {
            return -1;
        }
        if (fn_1_552C(position, &work->state.actor.position) > 200.0) {
            return -1;
        }
        targetAngle = fn_1_5490(position, &work->state.actor.position);
        if (fabs(fn_1_53DC(targetAngle, angle)) > 90.0) {
            return -1;
        }
        return index;
    }
    return -1;
}

/* Selection claims a prop's selection flag and returns its slot, or -1 when unavailable. */
s32 fn_1_FBCC(s32 objectIndex) {
    M676PropWork *propWork;
    OMOBJ *propObject;
    propObject = lbl_1_bss_5D8[objectIndex];
    if (propObject != NULL)
    {
        propWork = propObject->data;
        if (propWork->selectionState != 0)
        {
            return - 1;
        }
        propWork->selectionState = 1;
        return objectIndex;
    }
    return - 1;
}

/* Returns a prop's selection state (1 for an empty slot) and optionally copies its position. */
s32 fn_1_FC30(s32 index, Vec *resultValues)
{
    OMOBJ *object;
    M676PropWork *result;

    object = lbl_1_bss_5D8[index];
    if (object != NULL) {
        result = object->data;
        if (resultValues != NULL) {
            *resultValues = result->state.actor.position;
        }
        return result->selectionState;
    }
    return 1;
}

/* Returns the shallowest initialized prop depth, or slot zero's depth if none are initialized. */
f32 fn_1_FCA0(void)
{
    f32 result = 0.0f;
    s32 activeCount = 0;
    s32 index;

    for (index = 0; index < 5; index++) {
        OMOBJ *object = lbl_1_bss_5D8[index];
        M676PropWork *work = object->data;
        f32 sample = work->state.actor.position.z;

        if (index == 0) {
            result = sample;
        }
        if (work->initialized != 0) {
            if (activeCount == 0) {
                result = sample;
            }
            if (sample < result) {
                result = sample;
            }
            activeCount++;
        }
    }
    return result;
}

/* Returns the deepest initialized prop depth, or slot zero's depth if none are initialized. */
f32 fn_1_FD64(void)
{
    f32 result = 0.0f;
    s32 activeCount = 0;
    s32 index;

    for (index = 0; index < 5; index++) {
        OMOBJ *object = lbl_1_bss_5D8[index];
        M676PropWork *work = object->data;
        f32 sample = work->state.actor.position.z;

        if (index == 0) {
            result = sample;
        }
        if (work->initialized != 0) {
            if (activeCount == 0) {
                result = sample;
            }
            if (sample > result) {
                result = sample;
            }
            activeCount++;
        }
    }
    return result;
}

/* Selection prefers the ranked prop slot, then the first unselected slot; if none is free, it
 * returns slot zero. */
s32 fn_1_FE28(s32 index)
{
    s32 shouldSearch = 1;
    s32 count;
    OMOBJ *object;
    M676PropWork *work;

    if (index >= 0 && index < 5) {
        count = lbl_1_bss_480[index];
        object = lbl_1_bss_5D8[count];

        if (object != NULL) {
            work = object->data;
            if (work->selectionState == 0) {
                shouldSearch = 0;
            }
        }
    }

    if (shouldSearch != 0) {
        count = 0;
        while (count < 5) {
            object = lbl_1_bss_5D8[count];

            if (object != NULL) {
                work = object->data;
                if (work->selectionState == 0) {
                    break;
                }
            }
            count++;
        }
    }
    if (count < 0) {
        count = 0;
    }
    if (count >= 5) {
        count = 0;
    }
    return count;
}

/* Finds the nearest unfinished prop within the heading limit and horizontal X/Z distance limit;
 * vertical separation is ignored. */
OMOBJ *fn_1_FF18(Vec *position, f32 heading, f32 angleLimit,
                  f32 maximumDistance, OMOBJ *excludedObject)
{
    OMOBJ *closestObject = NULL;
    M676PropWork *work;
    f32 closestDistance = 100000.0f;
    s32 index;

    for (index = 0; index < 5; index++) {
        OMOBJ *candidate = lbl_1_bss_5D8[index];

        if (candidate != NULL && candidate != excludedObject) {
            work = candidate->data;

            if (work != NULL && work->finished == 0) {
                M676PropWork *activeWork = work;
                f32 deltaX = activeWork->state.actor.position.x - position->x;
                f32 deltaY = activeWork->state.actor.position.y - position->y;
                f32 deltaZ = activeWork->state.actor.position.z - position->z;
                f32 wrappedAngle;
                f32 candidateMeasure;

                if (angleLimit < 180.0) {
                    candidateMeasure = (f32)(180.0 *
                        (atan2((f64)deltaX, (f64)deltaZ) /
                            3.141592653589793));
                    wrappedAngle = (f32)fmod(
                        (f64)(candidateMeasure - heading), 360.0);
                    candidateMeasure = wrappedAngle;
                    if (candidateMeasure < 0.0f) {
                        candidateMeasure += 360.0f;
                    }
                    if (candidateMeasure > angleLimit &&
                        candidateMeasure < 360.0f - angleLimit) {
                        continue;
                    }
                }

                candidateMeasure = sqrtf(deltaX * deltaX + deltaZ * deltaZ);

                if (candidateMeasure > maximumDistance) {
                    continue;
                }
                if (candidateMeasure < closestDistance) {
                    closestDistance = candidateMeasure;
                    closestObject = candidate;
                }
            }
        }
    }

    return closestObject;
}

/* Marks the prop in the indexed course slot as finished when that object exists. */
void fn_1_10240(s32 index)
{
    OMOBJ *object;
    M676PropWork *work;
    object = lbl_1_bss_5D8[index];
    if (object != NULL) {
        work = object->data;
        work->finished = 1;
    }
}

const u8 lbl_1_rodata_62C[20] = {0};

OMOBJ *lbl_1_bss_10A0;
OMOBJ *lbl_1_bss_EA0[128];
OMOBJ *lbl_1_bss_E9C;
OMOBJ *lbl_1_bss_E98;
OMOBJ *lbl_1_bss_E94;
s32 lbl_1_bss_E90;
s32 lbl_1_bss_E8C;
s32 lbl_1_bss_E88;
M676SceneRegistration lbl_1_bss_E40;
s32 lbl_1_bss_E3C;
M676SceneFlagEntry lbl_1_bss_C7C[28];
M676SceneAreaRecord lbl_1_bss_5EC[70];
OMOBJ *lbl_1_bss_5D8[5];
s32 lbl_1_bss_5D4;
s32 lbl_1_bss_4A8[75];
s32 lbl_1_bss_494[5];
s32 lbl_1_bss_480[5];
