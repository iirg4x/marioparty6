/* Builds the stage, animates scenery, and manages moving scene actors. */
#include "dolphin/math.h"
#include "REL/m626Dll/scenery.h"
#include "game/charman.h"
#include "game/gamework.h"
#include "game/mg/seqman.h"
#include "game/memory.h"
#include "game/data.h"
#include "game/audio.h"
#include "humath.h"
#include <string.h>

#define M626_SURFACE_COLLISION_MASK 0x30
#define M626_PLAYER_COLLISION_MASK 0x35
#define M626_SNOWBALL_KEY_ZERO_COLLISION_MASK 0x60
#define M626_SNOWBALL_KEY_NONZERO_COLLISION_MASK 0xA0
#define M626_SNOWBALL_IMPACT_SOUND_ID 1802
#define M626_CHARACTER_LAND_SOUND_ID 187

extern s32 frand(void);
typedef struct M626CollisionParameters { Vec start, end; } M626CollisionParameters;

s32 fn_1_830(s32);
s32 fn_1_3C94(s32);
s32 fn_1_3D9C(s32, s32);
void fn_1_C94(s32, char *, s32);
void fn_1_DD8(Vec *);
void fn_1_1ECC(Vec *);
s32 fn_1_3340(s32, Vec *);
f32 fn_1_407C(Vec *, Vec *);
f32 fn_1_4118(Vec *, Vec *);
f32 fn_1_3EAC(f32, f32, f32);
void fn_1_E068(const Vec *);
void fn_1_E1D8(M626EffectRecord *);
void fn_1_DBC0(Vec *, Vec *);
void fn_1_DD48(s32, f32, const Vec *);
void fn_1_4CE4(M626ObjectWork *);
s32 fn_1_7050(M626Entry *, M626Entry *);
void fn_1_8570(M626ObjectWork *);
void fn_1_6120(M626ObjectWork *);
s32 fn_1_57A8(M626ObjectWork *);
s32 fn_1_5C20(M626ObjectWork *);
s32 fn_1_5E88(M626ObjectWork *);
void fn_1_85A4(M626ObjectWork *, s32);

void fn_1_8C50(HUPROCESS *process);
void fn_1_8D40(void);
void fn_1_8D44(OMOBJ *obj);
void fn_1_8D54(OMOBJ *object);
void fn_1_8D7C(HUPROCESS *process, M626SceneryParameters *param);
void fn_1_9020(OMOBJ *object);
OMOBJ *fn_1_9074(s32 type, s32 id);
void fn_1_90FC(OMOBJ *object, s32 index);
BOOL fn_1_9208(OMOBJ *object);
BOOL fn_1_9270(s32 type, s32 id, char *name, Vec *position, Vec *rotation);
void fn_1_93AC(OMOBJ *object);
void fn_1_93E4(OMOBJ *object);
void fn_1_94B4(OMOBJ *object);
void fn_1_9560(OMOBJ *object);
void fn_1_9564(OMOBJ *object);
void fn_1_9614(OMOBJ *object);
void fn_1_97A8(OMOBJ *object);
void fn_1_9970(OMOBJ *object);
void fn_1_9974(OMOBJ *object);
void fn_1_9A80(OMOBJ *object);
void fn_1_9A84(OMOBJ *object);
void fn_1_9BA0(OMOBJ *object);
void fn_1_9CD4(OMOBJ *object);
void fn_1_9D24(OMOBJ *object);
void fn_1_9DB8(void *value);
void fn_1_9E18(OMOBJ *object);
void fn_1_9FF8(OMOBJ *object);
void fn_1_A17C(OMOBJ *object, s32 particleCount);
s32 fn_1_A218(M626Entry *context, M626Entry *contact);
void fn_1_A2AC(OMOBJ *object);
void fn_1_A318(OMOBJ *object);
void fn_1_A398(OMOBJ *object);
void fn_1_A680(OMOBJ *object);
s32 fn_1_A87C(OMOBJ *object);
void fn_1_B288(OMOBJ *object);
void fn_1_B494(OMOBJ *object);
void fn_1_B55C(s32 key, const Vec *throwPosition, const Vec *throwVelocity, void *collisionPartner);
s32 fn_1_B62C(u32 key, const Vec *position, f32 angle, f32 angleRange,
               f32 maxDistance);
void *fn_1_B998(s32 scenerySlot);
s32 fn_1_BA10(void);

M626PlayerConfig lbl_1_data_470[2] = {
{50.0000000f, 150.000000f, 1.10000002f, 0.800000012f, 8.33333302f, 36.6666679f, 5.83333349f, {{0, 2, 2.00000000f, 3, 1.00000000f}, {10, 11, 2.00000000f, 12, 1.00000000f}}, 9, 8, 6, 0, 4, 5},
{50.0000000f, 150.000000f, 1.00000000f, 1.00000000f, 8.33333302f, 36.6666679f, 5.83333349f, {{0, 2, 2.00000000f, 3, 1.00000000f}, {10, 11, 2.00000000f, 12, 1.00000000f}}, 9, 8, 6, 0, 4, 5}
};

M626SceneryMotion lbl_1_data_528[14] = {
{12, 0.200000003f, HU3D_MOTATTR_LOOP},
{-1, 0.00000000f, HU3D_ATTR_NONE},
{13, 0.200000003f, HU3D_MOTATTR_LOOP},
{14, 0.200000003f, HU3D_MOTATTR_LOOP},
{15, 0.200000003f, HU3D_ATTR_NONE},
{16, 0.200000003f, HU3D_ATTR_NONE},
{17, 0.200000003f, HU3D_ATTR_NONE},
{18, 0.200000003f, HU3D_ATTR_NONE},
{19, 0.200000003f, HU3D_ATTR_NONE},
{20, 0.200000003f, HU3D_ATTR_NONE},
{21, 0.200000003f, HU3D_MOTATTR_LOOP},
{22, 0.200000003f, HU3D_MOTATTR_LOOP},
{23, 0.200000003f, HU3D_MOTATTR_LOOP},
{-2, 0.00000000f, HU3D_ATTR_NONE}
};

M626SceneryMotion lbl_1_data_5D0[15] = {
{-1, 0.00000000f, HU3D_ATTR_NONE},
{5, 0.200000003f, HU3D_MOTATTR_LOOP},
{-2, 0.00000000f, HU3D_ATTR_NONE},
{0, 0.00000000f, HU3D_ATTR_NONE},
{0, 0.00000000f, HU3D_ATTR_NONE},
{0, 0.00000000f, HU3D_ATTR_NONE},
{0, 0.00000000f, HU3D_ATTR_NONE},
{0, 0.00000000f, HU3D_ATTR_NONE},
{0, 0.00000000f, HU3D_ATTR_NONE},
{0, 0.00000000f, HU3D_ATTR_NONE},
{0, 0.00000000f, HU3D_ATTR_NONE},
{0, 0.00000000f, HU3D_ATTR_NONE},
{0, 0.00000000f, HU3D_ATTR_NONE},
{0, 0.00000000f, HU3D_ATTR_NONE},
{0, 0.00000000f, HU3D_ATTR_NONE}
};

M626SceneryParameters lbl_1_data_684[8] = {
{5, 1, 1, 0, 2, HU3D_MOTATTR_LOOP, 0, 10, NULL, NULL, 0.00000000f, fn_1_9974},
{1, 0, 2, 0, 2, HU3D_ATTR_NONE, 0, 1, NULL, NULL, 0.00000000f, fn_1_A2AC},
{1, 0, 0, 0, 2, HU3D_MOTATTR_LOOP, 0, 8, NULL, NULL, 0.00000000f, fn_1_9564},
{1, 0, 0, 1, 2, HU3D_ATTR_DISPOFF, 0, 3, NULL, NULL, 0.00000000f, fn_1_97A8},
{30, 1, 3, 0, 5, HU3D_ATTR_NONE, 0, 9, NULL, NULL, 0.00000000f, fn_1_9E18},
{4, 0, 4, 0, 1, HU3D_ATTR_NONE, 1, 11, lbl_1_data_528, NULL, 0.00000000f, fn_1_A398},
{5, 1, 5, 0, 0, HU3D_ATTR_NONE, 0, 4, lbl_1_data_5D0, NULL, 0.00000000f, fn_1_9A84},
{-1, 0, 0, 0, 0, HU3D_ATTR_NONE, 0, 0, NULL, NULL, 0.00000000f, NULL}
};

Vec lbl_1_data_7E4 = {0.00000000f, -1.00000000f, 0.00000000f};

OMOBJ *lbl_1_bss_6C0;
OMOBJ *lbl_1_bss_4C0[128];
OMOBJ *lbl_1_bss_448[30];
M626CollisionRecord lbl_1_bss_310[3];
s32 lbl_1_bss_30C;
s32 lbl_1_bss_308;

/* Creates the scenery manager and each configured stage object during setup. */
void fn_1_8C50(HUPROCESS *process) {
    OMOBJ *managerObject;
    OMOBJ **objectSlot;
    M626SceneryParameters *sceneryParameters;
    s32 slotIndex;
    void *managerState;

    objectSlot = lbl_1_bss_4C0;
    slotIndex = 0;
    while (slotIndex < 128) {
        *objectSlot = 0;
        slotIndex += 1;
        objectSlot += 1;
    }
    managerObject = omAddObjEx(process, 12, 0U, 0U, -1, fn_1_8D44);
    lbl_1_bss_6C0 = managerObject;
    managerObject->stat |= OM_STAT_MODELPAUSE;
    managerState = HuMemDirectMallocNum(HEAP_HEAP, 4, HU_MEMNUM_OVL);
    managerObject->data = managerState;
    memset(managerState, 0, 4);
    fn_1_9CD4(managerObject);
    sceneryParameters = lbl_1_data_684;
    while (sceneryParameters->count != -1) {
        fn_1_8D7C(process, sceneryParameters);
        sceneryParameters++;
    }
}

void fn_1_8D40(void)
{
}

void fn_1_8D44(OMOBJ *obj)
{
    obj->objFunc = fn_1_8D54;
}

void fn_1_8D54(OMOBJ *object)
{
    fn_1_9D24(object);
}

/* Builds the models and motions described by one stage scenery entry. */
void fn_1_8D7C(HUPROCESS *process, M626SceneryParameters *param)
{
    s32 i;
    OMOBJ *object;
    s32 model;
    M626SceneryWork *work;
    M626SceneryWork *state;
    M626SceneryMotion *motion;
    s32 group;
    s32 resource;

    for (group = 0; group < param->count; group++) {
        object = omAddObjEx(process, 12, 1, 13, 0, param->update);
        fn_1_9020(object);
        work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M626SceneryWork), HU_MEMNUM_OVL);
        object->data = work;
        memset(work, 0, sizeof(M626SceneryWork));
        object->stat |= OM_STAT_MODELPAUSE;
        state = work;
        work->common.actor.parameters = param;
        work->common.identity.sceneryId = param->firstId + group;
        state->common.motionIndex = -1;
        for (i = 0; i < 1; i++) {
            object->mdlId[i] = -1;
        }
        for (i = 0; i < 13; i++) {
            object->mtnId[i] = -1;
        }
        if (param->linkModel != 0) {
            model = fn_1_3C94(fn_1_830(param->modelResource));
        } else {
            model = (s16)Hu3DModelCreate(HuDataSelHeapReadNum(fn_1_830(param->modelResource),
                                                             HU_MEMNUM_OVL, HEAP_MODEL));
        }
        object->mdlId[0] = model;
        motion = param->motions;
        if (motion != NULL) {
            for (i = 0; i < 13; i++, motion++) {
                resource = motion->resource;
                if (resource == -2) {
                    break;
                }
                if (resource != -1) {
                    object->mtnId[i] = fn_1_3D9C(model, fn_1_830(resource));
                }
            }
        }
        if (param->position != NULL) {
            state->common.position = *param->position;
        }
        work->common.detail.startPosition = state->common.position;
        state->common.rotation.y = param->rotationY;
        Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
        Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
        Hu3DModelLayerSet(model, param->layer);
        Hu3DModelAttrSet(model, param->attr);
        switch (param->shadow) {
        case 1:
            Hu3DModelShadowSet(model);
            break;
        case 2:
            Hu3DModelShadowMapSet(model);
            break;
        }
    }
}

/* Adds a scenery object to the lookup table used by stage behavior callbacks. */
void fn_1_9020(OMOBJ *object)
{
    OMOBJ **entry = lbl_1_bss_4C0;
    s32 index = 0;
    while (index < 128) {
        if (*entry == NULL) {
            *entry = object;
            break;
        }
        ++index;
        ++entry;
    }
}

/* Finds a scenery object by its configured type and stage-local ID. */
OMOBJ *fn_1_9074(s32 type, s32 id)
{
    OMOBJ *object;
    OMOBJ **entry = lbl_1_bss_4C0;
    M626SceneryWork *work;
    s32 i;
    for (i = 0; i < 128; i++, entry++) {
        object = *entry;
        if (object != NULL) {
            work = object->data;
            if (work->common.actor.parameters->type == type &&
                work->common.identity.sceneryId == id) {
                return object;
            }
        }
    }
    return NULL;
}

/* Selects a configured scenery motion when an object's behavior requests it. */
void fn_1_90FC(OMOBJ *object, s32 index)
{
    M626SceneryWork *state;
    M626SceneryMotion *param;
    M626SceneryWork *work = object->data;
    s32 model, motion;

    state = work;
    if (state->common.motionIndex != index && index < 13) {
        motion = object->mtnId[index];
        work->common.updatePhase = 0;
        state->common.motionFrame = 0;
        if (motion == -1) {
            state->common.motionActive = 0;
            return;
        }
        param = &work->common.actor.parameters->motions[index];
        model = object->mdlId[0];
        if (state->common.motionIndex < 0 || (0.0) == param->blend) {
            Hu3DMotionSet(model, motion);
            Hu3DModelAttrSet(model, param->attr);
        } else {
            Hu3DMotionShiftSet(model, motion, (0.0f), (60.0f) * param->blend,
                               param->attr);
        }
        state->common.motionIndex = index;
    }
}

/* Reports that the model's current motion and pending blend have both ended. */
BOOL fn_1_9208(OMOBJ *object)
{
    s32 model_id;
    model_id = *object->mdlId;
    if (Hu3DMotionEndCheck((HU3D_MODELID)model_id) != 0 &&
        Hu3DMotionShiftIDGet((HU3D_MODELID)model_id) < 0) {
        return TRUE;
    }
    return FALSE;
}

/* If found, copies a scenery attachment's transform with +90-degree yaw; returns whether model
 * motion is still running. */
BOOL fn_1_9270(s32 type, s32 id, char *name, Vec *position, Vec *rotation)
{
    OMOBJ *object;
    M626SceneryWork *work;
    HSF_OBJECT *part;
    s32 model;

    object = fn_1_9074(type, id);
    if (object == NULL) {
        return FALSE;
    }
    work = object->data;
    model = object->mdlId[0];
    part = Hu3DModelObjPtrGet(model, name);
    if (part != NULL) {
        position->x = part->mesh.curr.pos.x;
        position->y = part->mesh.curr.pos.y;
        position->z = part->mesh.curr.pos.z;
        rotation->x = part->mesh.curr.rot.x;
        rotation->y = 90.0 + part->mesh.curr.rot.y;
        rotation->z = part->mesh.curr.rot.z;
    }
    return !Hu3DMotionEndCheck(model);
}

/* Updates a collision-enabled scenery actor from its object callback. */
void fn_1_93AC(OMOBJ *object)
{
    fn_1_93E4(object);
    fn_1_94B4(object);
}

/* Applies a scenery actor's position and yaw to its model each update. */
void fn_1_93E4(OMOBJ *object)
{
    M626SceneryWork *work = object->data;
    M626SceneryWork *state = work;
    s32 modelId = *object->mdlId;

    f32 angle = state->common.rotation.y;
    f32 wrappedAngle = (f32)fmod((f64)angle, 360.00000000000000);
    f32 normalizedAngle = wrappedAngle;

    state->common.rotation.y = normalizedAngle;
    state->common.rotation.x = state->common.rotation.z = 0.00000000f;
    Hu3DModelPosSetV((s16)modelId, &state->common.position);
    Hu3DModelRotSetV((s16)modelId, &state->common.rotation);
}

/* Advances collision work and the actor's motion timer on each update. */
void fn_1_94B4(OMOBJ *object)
{
    M626SceneryWork *work = object->data;
    M626ObjectWork *state = &work->common;

    if (state->motionActive) {
        if (fn_1_9208(object)) {
            state->motionActive = 0;
            state->motionTimer = 0;
        }
    } else {
        state->motionTimer = 0;
    }
    work->common.updatePhase++;
}

void fn_1_9560(OMOBJ *object)
{
}

/* Chooses the lit model part for the current day or night stage variant. */
void fn_1_9564(OMOBJ *object)
{
    void *work = object->data;
    s32 model = object->mdlId[0];
    s16 nightMode = GwMgNightF;

    if (nightMode == 1) {
        fn_1_C94(model, "inf_light2_1_1", 1);
    } else {
        fn_1_C94(model, "inf_light2_3", 1);
    }
    fn_1_DD8(&lbl_1_data_7E4);
    fn_1_1ECC(&lbl_1_data_7E4);
    object->objFunc = fn_1_9614;
}

Vec lbl_1_data_80C[3] = {
{-390.000000f, 42.0000000f, -820.000000f},
{837.000000f, 205.000000f, -400.000000f},
{880.000000f, 390.000000f, -432.000000f}
};

/* Emits the drifting stage particle from one of three authored positions. */
void fn_1_9614(OMOBJ *object)
{
    M626SceneryWork *work = object->data;
    Vec *base;
    Vec velocity;

    if (++work->sampleIndex > 2) {
        work->sampleIndex = 0;
    }
    base = &lbl_1_data_80C[work->sampleIndex];
    velocity.x = (f32)((f64)base->x +
                       (40.000000000000000 *
                        (0.000030517578125000000 *
                         (f64)(f32)((u16)frand() - 32768))));
    velocity.y = (f32)((f64)base->y +
                       (10.000000000000000 *
                        (0.000030517578125000000 *
                         (f64)(f32)((u16)frand() - 32768))));
    velocity.z = (f32)((f64)base->z +
                       (40.000000000000000 *
                        (0.000030517578125000000 *
                         (f64)(f32)((u16)frand() - 32768))));
    fn_1_E068(&velocity);
}

const M626SurfaceCollider lbl_1_rodata_310 = {
    1, M626_SURFACE_COLLISION_MASK, 3, 0, 12, NULL, NULL, NULL,
    NULL, 0, NULL, NULL, NULL, 0, NULL, NULL,
    NULL
};

const M626RangeBounds lbl_1_rodata_354 = { 1,
                                           4,
                                           3,
                                           1,
                                           11,
                                           NULL,
                                           NULL,
                                           NULL,
                                           { -1000.00000f, 0.00000000f, -1000.00000f },
                                           { -50.0000000f, 0.00000000f, 1000.00000f } };

const M626RangeBounds lbl_1_rodata_38C = { 1,
                                           8,
                                           3,
                                           2,
                                           11,
                                           NULL,
                                           NULL,
                                           NULL,
                                           { 50.0000000f, 0.00000000f, -1000.00000f },
                                           { 1000.00000f, 0.00000000f, 1000.00000f } };

/* Registers the stage's central surface and two edge collision bounds. */
void fn_1_97A8(OMOBJ *object)
{
    M626CollisionRecord *collisionRecord;
    void *unusedObjectData;
    s32 modelId;
    collisionRecord = lbl_1_bss_310;
    unusedObjectData = object->data;
    modelId = object->mdlId[0];
    memset(collisionRecord, 0, sizeof(lbl_1_bss_310));
    collisionRecord->mesh = lbl_1_rodata_310;
    fn_1_10A1C(&collisionRecord->mesh, modelId);
    fn_1_10AA8(&collisionRecord->mesh);
    fn_1_10610(collisionRecord++);
    collisionRecord->bounds = lbl_1_rodata_354;
    fn_1_10610(collisionRecord++);
    collisionRecord->bounds = lbl_1_rodata_38C;
    fn_1_10610(collisionRecord++);
    object->objFunc = fn_1_9970;
}

void fn_1_9970(OMOBJ *object)
{
}

/* Starts the scenery model's looping motion with a per-object time offset. */
void fn_1_9974(OMOBJ *object)
{
    M626SceneryWork *work = object->data;
    s32 model = object->mdlId[0];

    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DMotionTimeSet(model,
                      (f32)(20.000000000000000 * (f64)work->common.identity.sceneryId +
                            5.0000000000000000 *
                                (0.000030517578125000000 *
                                  (f64)(f32)((u16)frand() - 32768))));
    Hu3DMotionSpeedSet(model, 0.300000000f);
    object->objFunc = fn_1_9A80;
}

void fn_1_9A80(OMOBJ *object)
{
}

/* Starts the snowball model's rolling motion after it is created. */
void fn_1_9A84(OMOBJ *object)
{
    M626SceneryWork *unusedInitialWork;
    s32 unusedInitialModel;
    unusedInitialWork = object->data;
    unusedInitialModel = object->mdlId[0];
    fn_1_90FC(object, 1);
    object->objFunc = fn_1_9BA0;
}

/* Follows the snowball model's sphere attachment and updates its collision actor. */
void fn_1_9BA0(OMOBJ *object)
{
    M626SceneryWork *work = object->data;
    M626SceneryWork *state = work;
    fn_1_9270(1, work->common.identity.sceneryId, "sphere1", &state->common.position,
              &state->common.rotation);
    fn_1_B494(object);
}

/* Clears the snowball lookup slots when the scenery manager is initialized. */
void fn_1_9CD4(OMOBJ *object) {
    s32 slotIndex;

    slotIndex = 0;
    while (slotIndex < 30) {
        lbl_1_bss_448[slotIndex] = NULL;
        slotIndex += 1;
    }
    lbl_1_bss_30C = 0;
}

/* Watches the minigame phase and stops new snowballs after play ends. */
void fn_1_9D24(OMOBJ *object)
{
    s32 *state = object->data;

    switch (*state) {
    case 0:
        if (MgSeqModeGet() == 5) {
            *state += 1;
        }
        break;
    case 1:
        if (MgSeqModeGet() != 5) {
            *state += 1;
            lbl_1_bss_30C = 1;
        }
        break;
    }
}

/* Stores a scenery-related object in the first free shared lookup slot. */
void fn_1_9DB8(void *objectToRegister)
{
    s32 index;

    for (index = 0; index < 30; index++) {
        if (lbl_1_bss_448[index] == NULL) {
            lbl_1_bss_448[index] = objectToRegister;
            break;
        }
    }
}

Vec lbl_1_data_838[2] = {
{0.00000000f, 1.00000000f, 0.00000000f},
{0.00000000f, -1.00000000f, 0.00000000f}
};

Vec lbl_1_data_850 = {0.00000000f, 0.00000000f, 0.00000000f};

M626CollisionParameters lbl_1_data_85C = { { 0.00000000f, 0.00000000f, 0.00000000f },
                                           { 0.00000000f, 100.000000f, 0.00000000f } };

/* Configures a snowball's collision segment, effect and initial display state. */
void fn_1_9E18(OMOBJ *object)
{
    M626SceneryWork *runtimeWork;
    M626SceneryWork *work;
    M626SceneryWork *positionWork;
    s32 modelId;
    s32 index;

    runtimeWork = object->data;
    work = runtimeWork;
    modelId = *object->mdlId;
    positionWork = work;
    positionWork->common.collisionFlags = 0;
    positionWork->common.collisionMask = 0;
    positionWork->common.collisionType = 1;
    positionWork->common.playerIndex = runtimeWork->common.identity.sceneryId;
    positionWork->common.collisionState = 0;
    positionWork->common.owner = object;
    positionWork->common.update = fn_1_93AC;
    positionWork->common.draw = fn_1_A218;
    positionWork->common.positionData = &work->common.position;
    positionWork->common.offset = &work->common.velocity;
    positionWork->common.worldEnd = lbl_1_data_850;
    positionWork->common.matrix = 0;
    positionWork->common.segmentStart = lbl_1_data_838[0];
    positionWork->common.segmentEnd = lbl_1_data_838[1];
    positionWork->common.radius = 40.0f;
    positionWork->common.collisionMode = 1.0f;
    fn_1_10598(positionWork);
    runtimeWork->effect.active = 0;
    runtimeWork->effect.radius = 30.0f;
    runtimeWork->effect.position = &work->common.position;
    fn_1_E1D8(&runtimeWork->effect);
    for (index = 0; index < 30; index++) {
        if (lbl_1_bss_448[index] == NULL) {
            lbl_1_bss_448[index] = object;
            break;
        }
    }
    Hu3DModelScaleSet(modelId, 1.5f, 1.5f, 1.5f);
    Hu3DModelAttrSet(modelId, HU3D_ATTR_DISPOFF);
    object->objFunc = fn_1_9FF8;
}

/* Advances a thrown snowball, then hides it with an impact effect when play ends or its Y falls
 * below 20. */
void fn_1_9FF8(OMOBJ *object)
{
    M626SceneryWork *work = object->data;
    M626SceneryWork *state = work;
    s32 model = object->mdlId[0];
    s32 targetHeight;

    switch (work->state) {
    case 0:
        break;
    case 1:
        work->state++;
        state->common.collisionFlags |= (1 << 0);
        if (state->common.key == 0) {
            targetHeight = M626_SNOWBALL_KEY_ZERO_COLLISION_MASK;
        } else {
            targetHeight = M626_SNOWBALL_KEY_NONZERO_COLLISION_MASK;
        }
        state->common.collisionMask = targetHeight;
        work->effect.active = 1;
        Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);
        /* A new throw begins moving in the frame it is launched. */
    case 2:
        if (lbl_1_bss_30C != 0) {
            fn_1_A17C(object, 20);
            return;
        }
        if (state->common.position.y < 20.000000000000000) {
            fn_1_A17C(object, 10);
            return;
        }
        state->common.velocity.x *= 0.99;
        state->common.velocity.y -= 0.54481388888888893;
        state->common.velocity.z *= 0.99;
        PSVECAdd(&state->common.position, &state->common.velocity, &state->common.position);
        if (PSVECMag(&state->common.velocity) > 33.333333333333336) {
            fn_1_DBC0(&state->common.position, &state->common.velocity);
        }
    }
}

/* Ends the snowball, always emits 20 snow particles at its position, and plays the impact sound;
 * particleCount is ignored. */
void fn_1_A17C(OMOBJ *object, s32 particleCount)
{
    M626SceneryWork *state = object->data;
    M626SceneryWork *work = state;
    s32 model = object->mdlId[0];

    state->state = 0;
    work->common.collisionFlags &= ~(1 << 0);
    state->effect.active = 0;
    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
    fn_1_DD48(20, 30.0000000f, &work->common.position);
    fn_1_3340(M626_SNOWBALL_IMPACT_SOUND_ID, &work->common.position);
}

/* Ends the snowball when the collision system reports contact. */
s32 fn_1_A218(M626Entry *context, M626Entry *contact)
{
    fn_1_A17C(context->primaryRelation, 20);
    return 1;
}

/* Prepares the stage feature's opening animation when its object is created. */
void fn_1_A2AC(OMOBJ *object)
{
    void *work = object->data;
    s32 model = object->mdlId[0];

    lbl_1_bss_308 = 0;
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    object->objFunc = fn_1_A318;
}

/* Removes the opening pause attribute and speeds up the model after 90 frames. */
void fn_1_A318(OMOBJ *object)
{
    M626SceneryWork *work = object->data;
    s32 model = object->mdlId[0];

    if (work->timer == 90) {
        Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
        Hu3DMotionSpeedSet(model, 2.00000000f);
    }
    work->timer++;
}

/* Initializes a character actor, its collision record and its motion state. */
void fn_1_A398(OMOBJ *object)
{
        s32 model;
    M626SceneryWork *state = object->data;
    M626SceneryWork *work = state;
    M626PlayerConfig *configuration = &work->common.config;
    s32 selector = state->common.identity.sceneryId;
    M626SceneryWork *task;
    s16 nightMode;
    s16 primaryMotion;
    s16 secondaryMotion;

    model = object->mdlId[0];

    object->work[0] = selector + 4;
    work->common.key = 0;
    work->common.groupIndex = selector + 1;
    work->common.variation = selector + 4;
    work->common.playerType = 1;
    work->common.computerDifficulty = 4;
    work->common.controlPhase = 0;
    work->common.modelId = fn_1_3C94(fn_1_830(9));
    Hu3DModelLayerSet((s16)work->common.modelId, 5);
    Hu3DModelHookSet(model, "itemhook_R",
                     (s16)work->common.modelId);
    work->common.motionState = 0;

    nightMode = GwMgNightF;
    if (nightMode == 1) {
        *configuration = lbl_1_data_470[1];
        work->common.computerDifficulty++;
    } else {
        *configuration = lbl_1_data_470[0];
    }

    task = work;
    task->common.collisionFlags = 0;
    task->common.collisionMask = M626_PLAYER_COLLISION_MASK;
    task->common.collisionType = 2;
    task->common.playerIndex = selector;
    task->common.collisionState = 0;
    task->common.owner = object;
    task->common.update = fn_1_B494;
    task->common.draw = fn_1_7050;
    task->common.positionData = &work->common.position;
    task->common.offset = &work->common.velocity;
    task->common.matrix = work->common.rotationMatrix;
    task->common.segmentStart = lbl_1_data_85C.start;
    task->common.segmentEnd = lbl_1_data_85C.end;
    task->common.radius = configuration->jumpHeight;
    task->common.collisionMode = 1.00000000f;
    fn_1_10598(task);
    fn_1_4CE4(&work->common);

    fn_1_90FC(object, 0);
    object->objFunc = fn_1_A680;
}

/* Runs the character actor's action phases and advances its movement each frame. */
void fn_1_A680(OMOBJ *object)
{
    M626SceneryWork *state;
    M626SceneryWork *work;

    state = object->data;
    work = state;
    fn_1_8570(&work->common);
    fn_1_6120(&work->common);
    switch (state->state) {
    case 0:
        if (fn_1_A87C(object) != 0) {
            state->state++;
            work->common.statePhase = 0;
        }
        break;
    case 1:
        if (fn_1_57A8(&work->common) != 0) {
            state->state++;
            work->common.statePhase = 0;
        }
        break;
    case 2:
        if (fn_1_5C20(&work->common) != 0) {
            state->state++;
            work->common.statePhase = 0;
        }
        break;
    case 3:
        if (fn_1_5E88(&work->common) != 0) {
            state->state++;
            work->common.statePhase = 0;
        }
        break;
    }
    fn_1_B288(object);
    if (work->common.active == 0) {
        if (work->common.motionIndex == 2) {
            work->common.travelDistance += PSVECMag(&work->common.velocity);
            if (work->common.travelDistance > 80.0) {
                work->common.travelDistance = (f32)((f64)work->common.travelDistance - 80.0);
                fn_1_3340(MSM_SE_GUIDE_34, &work->common.position);
            }
        }
        if (work->common.motionIndex == 3) {
            work->common.travelDistance += PSVECMag(&work->common.velocity);
            if (work->common.travelDistance > 180.0) {
                work->common.travelDistance = (f32)((f64)work->common.travelDistance - 180.0);
                fn_1_3340(MSM_SE_GUIDE_35, &work->common.position);
            }
        }
    }
    PSVECAdd(&work->common.position, &work->common.velocity, &work->common.position);
}

char lbl_1_data_87F[4][8] = { "heyho_a", "heyho_b", "heyho_c", "heyho_d" };

/* Runs the opening character approach, jump and landing phases for the actor update. */
s32 fn_1_A87C(OMOBJ *object) {
    M626SceneryWork *state = object->data;
    M626SceneryWork *work = state;
    M626PlayerConfig *configuration = &work->common.config;
    f32 horizontalSpeed;
    f32 angleDifference;
    s32 done = 0;

    switch (work->common.statePhase) {
    case 0:
        work->common.statePhase++;
        work->common.stateTimer = 0;
        state->initialPosition = work->common.position;
        fn_1_90FC(object, 2);
        /* fall through */
    case 1:
        fn_1_9270(2, 0, lbl_1_data_87F[state->common.identity.sceneryId],
                  &work->common.position, &work->common.rotation);
        work->common.position.y = (f32)((f64)work->common.position.y - 50.0);
        if (++work->common.stateTimer < 6) {
            state->targetPosition = work->common.position;
        }
        if (!(fn_1_4118(&work->common.position, &state->targetPosition) < 1.0)) {
            work->common.statePhase++;
            work->common.stateTimer = 0;
            state->targetPosition = work->common.position;
        }
        break;
    case 2:
        fn_1_9270(2, 0, lbl_1_data_87F[state->common.identity.sceneryId],
                  &work->common.position, &work->common.rotation);
        work->common.position.y = (f32)((f64)work->common.position.y - 50.0);
        if (fn_1_4118(&work->common.position, &state->targetPosition) > 1.0) {
            state->targetPosition = work->common.position;
        } else {
            work->common.statePhase++;
            work->common.stateTimer = 0;
        }
        break;
    case 3:
        angleDifference = fn_1_407C(&work->common.position, &state->initialPosition);
        work->common.rotation.y = fn_1_3EAC(work->common.rotation.y, angleDifference, 0.2f);
        if (++work->common.stateTimer >= 12) {
            work->common.statePhase++;
            work->common.stateTimer = 0;
            work->common.active = 1;
            work->common.velocity.y = 33.333332f;
            fn_1_90FC(object, 7);
            fn_1_3340(MSM_SE_GUIDE_69, &work->common.position);
        }
        break;
    case 4:
        if (++work->common.stateTimer >= 2) {
            work->common.statePhase++;
            work->common.stateTimer = 0;
            horizontalSpeed = (f32)(0.04 * fn_1_4118(&work->common.position,
                                                     &state->initialPosition));
            work->common.velocity.x = (f32)((f64)horizontalSpeed *
                sin((3.141592653589793 * (f64)work->common.rotation.y) / 180.0));
            work->common.velocity.z = (f32)((f64)horizontalSpeed *
                cos((3.141592653589793 * (f64)work->common.rotation.y) / 180.0));
        }
        break;
    case 5:
        work->common.rotation.y = fn_1_3EAC(work->common.rotation.y, 90.0f,
                                      0.05f);
        work->common.velocity.x *= 0.98;
        work->common.velocity.y -= 1.6344416666666668;
        work->common.velocity.z *= 0.98;
        if (!(work->common.velocity.y > 0.0f) && !(work->common.position.y > 0.0f)) {
            work->common.statePhase++;
            work->common.stateTimer = 0;
            work->common.position.y = 0.0f;
            work->common.velocity.x = 0.0f;
            work->common.velocity.y = 0.0f;
            work->common.velocity.z = 0.0f;
            work->common.active = 0;
            fn_1_90FC(object, 0);
            {
                Vec dustPosition = work->common.position;

                dustPosition.y += 10.0;
                CharEffectDustCreate(-1, 1.0f, &dustPosition);
            }
            work->common.collisionFlags |= 1;
            lbl_1_bss_308++;
            fn_1_3340(M626_CHARACTER_LAND_SOUND_ID, &work->common.position);
        } else {
            break;
        }
        /* After landing, the intro completes when the live-play phase begins. */
    case 6:
        if (MgSeqModeGet() != 5) {
            break;
        }
        work->common.statePhase++;
        work->common.stateTimer = 0;
    default:
        done = 1;
        break;
    }
    return done;
}

/* Applies a requested character motion or the motion for a status transition. */
void fn_1_B288(OMOBJ *object)
{
    M626SceneryWork *state = object->data;
    M626SceneryWork *work = state;

    if (work->common.status != 0) {
        fn_1_90FC(object, 6);
        return;
    }

    if (work->common.requestedMotion != -1) {
        fn_1_90FC(object, work->common.requestedMotion);
        Hu3DMotionSpeedSet(*object->mdlId, work->common.motionSpeed);
    }
}

/* Runs the character collision update and the shared motion timer each frame. */
void fn_1_B494(OMOBJ *object)
{
    M626SceneryWork *hookWork = object->data;
    M626SceneryWork *hookState = hookWork;
    s32 hookModel = object->mdlId[0];
    fn_1_85A4(&hookState->common, hookModel);
    fn_1_94B4(object);
}

/* Activates the first free snowball with its throw key, position, velocity and owner. */
void fn_1_B55C(s32 key, const Vec *throwPosition, const Vec *throwVelocity, void *collisionPartner)
{
    OMOBJ *object;
    M626SceneryWork *work;
    M626SceneryWork *state;
    s32 index;

    if (lbl_1_bss_30C == 0) {
        for (index = 0; index < 30; index++) {
            object = lbl_1_bss_448[index];
            if (object != NULL) {
                work = object->data;
                if (work->state == 0) {
                    state = work;
                    state->common.key = key;
                    state->common.position = *throwPosition;
                    state->common.velocity = *throwVelocity;
                    state->common.collisionPartner = collisionPartner;
                    work->state++;
                    return;
                }
            }
        }
    }
}

/* Finds the nearest active snowball with the same key inside the requested cone. */
s32 fn_1_B62C(u32 key, const Vec *position, f32 angle, f32 angleRange,
               f32 maxDistance)
{
    OMOBJ *snowballObject;
    M626SceneryWork *snowballWork;
    M626SceneryWork *candidateSnowballWork;
    f32 xDifference;
    f32 yDifference;
    f32 zDifference;
    f32 angleDifference;
    f32 wrappedAngle;
    f32 closestDistance = 100000.000f;
    s32 closestIndex = -1;
    s32 index;

    for (index = 0; index < 30; index++) {
        snowballObject = lbl_1_bss_448[index];
        if (snowballObject == NULL) {
            continue;
        }
        snowballWork = snowballObject->data;
        if (snowballWork->state == 0) {
            continue;
        }
        candidateSnowballWork = snowballWork;
        if (candidateSnowballWork->common.key != key) {
            continue;
        }

        xDifference = candidateSnowballWork->common.position.x - position->x;
        yDifference = candidateSnowballWork->common.position.y - position->y;
        zDifference = candidateSnowballWork->common.position.z - position->z;
        if (angleRange < 180.00000000000000) {
            angleDifference = (f32)(180.00000000000000 *
                (atan2(xDifference, zDifference) / 3.1415926535897931));
            wrappedAngle = (f32)fmod(
                (f64)(angleDifference - angle), 360.00000000000000);
            angleDifference = wrappedAngle;
            if (angleDifference < 0.0f) {
                angleDifference += 360.000000f;
            }
            if (angleDifference > angleRange &&
                angleDifference < 360.000000f - angleRange) {
                /* A same-key snowball outside the cone makes the routine return no target. */
                return 0;
            }
        }

        angleDifference = sqrtf((zDifference * zDifference) +
                                ((xDifference * xDifference) +
                                 (yDifference * yDifference)));
        if (angleDifference > maxDistance) {
            continue;
        }
        if (angleDifference < closestDistance) {
            closestDistance = angleDifference;
            closestIndex = index;
        }
    }
    return closestIndex;
}

/* Returns active snowball work for a slot, or NULL for an empty slot. */
void *fn_1_B998(s32 scenerySlot) {
    OMOBJ *snowballObject;
    M626SceneryWork *snowballWork;
    if ((scenerySlot < 0) || (scenerySlot >= 30))
    {
        return NULL;
    }
    snowballObject = lbl_1_bss_448[scenerySlot];
    if (snowballObject == NULL)
    {
        return NULL;
    }
    snowballWork = snowballObject->data;
    if ((s32) snowballWork->state == 0)
    {
        return NULL;
    }
    return snowballWork;
}

s32 fn_1_BA10(void) {
    return lbl_1_bss_308 >= 4;
}

OMOBJ *lbl_1_bss_6C0;
