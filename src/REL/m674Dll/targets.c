/* Stage props, moving targets, collectible model animations and their collision regions. */
#include "dolphin/math.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/memory.h"
#include "game/hsfex.h"
#include "game/gamework.h"
#include "game/mg/seqman.h"
#include "game/frand.h"
#include "msm_se.h"
#include "REL/m674Dll/targets.h"
#include "REL/m674Dll/effects.h"
#include "REL/m674Dll/scene.h"
#include <string.h>

/* Shared archive, sequence, scene, player, and collision helpers used by props. */
extern s32 fn_1_E18(s32 resource);
extern s32 fn_1_E20(void);
extern s32 fn_1_F3C(void);
extern s32 fn_1_4994(void);
extern s32 fn_1_4A9C(s32 model, s32 resource);
extern s32 fn_1_11218(M674Entry *entry);
extern void fn_1_112A8(M674Entry *record);
extern s32 fn_1_4040(s32 effect, void *position);
extern f32 fn_1_4BAC(f32 start, f32 end, f32 factor);
extern f32 fn_1_4CC8(f32 start, f32 end);
extern f32 fn_1_4D7C(Vec *start, Vec *end);
extern s32 fn_1_4FDC(Vec *start, Vec *end, f32 gravity, f32 initialY, Vec *velocity);
extern s32 fn_1_36BC(void *position);
extern s32 fn_1_7660(M674CollisionParam *first, M674CollisionParam *second);
extern void fn_1_10028(s32 scoreIndex, s32 coinAmount);
extern void fn_1_10150(s32 scoreIndex, f32 *x, f32 *y);

void fn_1_8ACC(HUPROCESS *process);
void fn_1_8BCC(void);
void fn_1_8BD0(OMOBJ *object);
void fn_1_8BE0(OMOBJ *object);
void fn_1_8BE4(HUPROCESS *process, M674PropParam *param);
void fn_1_8EA4(OMOBJ *object);
OMOBJ *fn_1_8EF8(s32 type, s32 id);
void fn_1_8F80(OMOBJ *object, s32 motionIndex);
s32 fn_1_908C(OMOBJ *object);
BOOL fn_1_90F4(s32 type, s32 id, char *name, Vec *position, Vec *rotation);
void fn_1_9230(OMOBJ *object);
void fn_1_9268(OMOBJ *object);
void fn_1_9324(OMOBJ *object);
void fn_1_93D0(OMOBJ *object);
void fn_1_93D4(OMOBJ *object);
void fn_1_9578(OMOBJ *object);
void fn_1_957C(OMOBJ *object);
void fn_1_95DC(OMOBJ *object);
void fn_1_95E0(OMOBJ *object);
void fn_1_95F0(OMOBJ *object);
void fn_1_9720(OMOBJ *object);
void fn_1_9780(OMOBJ *object);
void fn_1_9B90(OMOBJ *object);
void fn_1_9CF0(OMOBJ *object);
void fn_1_9F44(OMOBJ *object);
void fn_1_A2D8(OMOBJ *object);
void fn_1_A400(OMOBJ *object);
void fn_1_A730(M674PropWork *owner, s32 resource, s32 count);
void fn_1_A7CC(s32 scoreIndex, Point3d *destination);
void fn_1_A83C(OMOBJ *object);
void fn_1_A8FC(OMOBJ *object);
void fn_1_AF30(OMOBJ *object);
void fn_1_AFE0(OMOBJ *object);
void fn_1_B27C(void *object);
void fn_1_B37C(OMOBJ *object);
void fn_1_B6A4(OMOBJ *object);
void fn_1_BD0C(OMOBJ *object);
void fn_1_BDC8(OMOBJ *object);
void fn_1_BFF0(OMOBJ *object);
void fn_1_C11C(OMOBJ *object);
s32 fn_1_C2E8(M674PropWork *work);
s32 fn_1_C61C(M674PropWork *work);
s32 fn_1_C7E0(M674PropWork *sequence);
s32 fn_1_C8BC(M674PropWork *work);
s32 fn_1_CB10(M674PropWork *work);
s32 fn_1_CCCC(M674PropWork *work);
s32 fn_1_D020(M674PropWork *work, f32 turnFactor);
s32 fn_1_D18C(M674PropWork *work, f32 speed);
s32 fn_1_D5DC(M674PropWork *work, f32 gravity, f32 trajectoryY, s32 landingFrames);
void fn_1_D8CC(OMOBJ *object);
void fn_1_D9FC(OMOBJ *object);
s32 fn_1_DB40(void);
void fn_1_DB50(s32 collectionType, s32 scoreIndex, Vec *position);
M674ActorState *fn_1_DC2C(Vec *origin, f32 heading, f32 headingRange, f32 maxDistance);
s32 fn_1_DF88(void);
s32 fn_1_E014(void);

/* Random offsets use the midpoint of the 16-bit random range. */
#define RANDOM_U16_MIDPOINT 32768
#define PROP_ROTATION_AXIS_Y 121
#define TARGET_INITIAL_COLLISION_MASK 0x19
#define TARGET_COLLISION_MASK 0x11
#define MSM_SE_M674_TARGET_COLLECT 2222
#define MSM_SE_M674_SCORE_FLIGHT 2223

/* Initial scenery and actor positions, in world units. */
Vec lbl_1_data_270[4] = {{0,0,0},{0,160,-850},{-150,-40,-500},{-1013,0,-916}};
Vec lbl_1_data_2A0 = {0, -200, 0};
Vec lbl_1_data_2AC = {0, -210, 0};
Vec lbl_1_data_2B8 = {0, -330, 0};
/* Scripted actor motions: resource index, blend duration in seconds and playback flags. */
M674PropMotionParam lbl_1_data_2C4[9] = {
    {5, 0.2f, HU3D_MOTATTR_LOOP},
    {10, 0.2f, HU3D_MOTATTR_LOOP},
    {11, 0.2f, HU3D_MOTATTR_LOOP},
    {6, 0.2f, HU3D_MOTATTR_LOOP},
    {7, 0.15f, HU3D_MOTATTR_NONE},
    {8, 0.2f, HU3D_MOTATTR_NONE},
    {9, 0.2f, HU3D_MOTATTR_NONE},
    {12, 0.2f, HU3D_MOTATTR_NONE},
    {-2, 0, HU3D_MOTATTR_NONE}
};
/* Prop instances, render settings and first-update callbacks; count -1 ends the table. */
M674PropParam lbl_1_data_330[14] = {
    {1, 0, 1, 0, 6, HU3D_MOTATTR_LOOP, 0, -1, NULL, NULL, 0, fn_1_93D4},
    {1, 0, 0, 0, 6, HU3D_MOTATTR_LOOP, 2, 18, NULL, NULL, 0, fn_1_957C},
    {1, 0, 0, 1, 1, HU3D_MOTATTR_LOOP, 0, 19, NULL, NULL, 0, fn_1_93D0},
    {1, 0, 0, 2, 3, (HU3D_MOTATTR_LOOP | HU3D_MOTATTR_SHAPE_LOOP), 0, 20, NULL, &lbl_1_data_270[3], 0, fn_1_9720},
    {1, 0, 0, 3, 6, HU3D_MOTATTR_LOOP, 0, 21, NULL, NULL, 0, fn_1_95E0},
    {1, 0, 0, 4, 6, (HU3D_MOTATTR_LOOP | HU3D_MOTATTR_SHAPE_LOOP), 0, 22, NULL, NULL, 0, fn_1_93D0},
    {2, 1, 6, 0, 2, HU3D_MOTATTR_LOOP, 0, 25, NULL, &lbl_1_data_2A0, 0, fn_1_9CF0},
    {2, 1, 7, 0, 2, HU3D_MOTATTR_LOOP, 0, 26, NULL, &lbl_1_data_2AC, 0, fn_1_9CF0},
    {2, 1, 8, 0, 2, HU3D_MOTATTR_LOOP, 0, 27, NULL, &lbl_1_data_2B8, 0, fn_1_A2D8},
    {1, 0, 2, 0, 5, HU3D_MOTATTR_NONE, 0, -1, NULL, NULL, 0, fn_1_A83C},
    {1, 0, 3, 0, 5, HU3D_MOTATTR_NONE, 0, -1, NULL, NULL, 0, fn_1_AF30},
    {26, 1, 4, 0, 3, HU3D_MOTATTR_LOOP, 2, 23, NULL, &lbl_1_data_270[2], 0, fn_1_B37C},
    {1, 0, 5, 0, 3, HU3D_MOTATTR_NONE, 1, 4, lbl_1_data_2C4, &lbl_1_data_270[1], 0, fn_1_BDC8},
    {-1, 0, 0, 0, 0, HU3D_MOTATTR_NONE, 0, 0, NULL, NULL, 0, NULL}
};

/* Create the scenery, targets and collection-effect pools during minigame setup. */
void fn_1_8ACC(HUPROCESS *process)
{
    OMOBJ *manager;
    OMOBJ **entry;
    M674PropParam *param;
    s32 index;
    void *work;

    entry = lbl_1_bss_4BC;
    index = 0;
    while (index < 128) {
        *entry = NULL;
        index += 1;
        entry += 1;
    }
    manager = omAddObjEx(process, 12, 0, 0, -1, fn_1_8BD0);
    lbl_1_bss_6BC = manager;
    manager->stat |= OM_STAT_MODELPAUSE;
    work = HuMemDirectMallocNum(HEAP_HEAP, 8, HU_MEMNUM_OVL);
    manager->data = work;
    memset(work, 0, 8);
    lbl_1_bss_4AC = 0;
    fn_1_B27C(manager);
    param = lbl_1_data_330;
    while (param->count != -1) {
        fn_1_8BE4(process, param);
        param++;
    }
}

/* Prop shutdown hook called by the minigame exit sequence. */
void fn_1_8BCC(void)
{
}

/* Finish the prop manager's first update by installing its idle callback. */
void fn_1_8BD0(OMOBJ *object)
{
    object->objFunc = fn_1_8BE0;
}

/* Keep the prop manager alive while the individual prop objects update themselves. */
void fn_1_8BE0(OMOBJ *object)
{
}

/* Create each instance in a prop-table entry during setup, including its models and motions. */
void fn_1_8BE4(HUPROCESS *process, M674PropParam *param)
{
    s32 slot;
    OMOBJ *object;
    s32 model;
    M674PropWork *work;
    M674ActorState *state;
    M674PropMotionParam *motion;
    s32 instanceIndex;
    s32 resourceIndex;

    for (instanceIndex = 0; instanceIndex < param->count; instanceIndex++) {
        object = omAddObjEx(process, 12, 1, 8, 0, param->update);
        fn_1_8EA4(object);
        work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M674PropWork), HU_MEMNUM_OVL);
        object->data = work;
        memset(work, 0, sizeof(M674PropWork));
        object->stat |= OM_STAT_MODELPAUSE;
        state = &work->state;
        work->param = param;
        work->id = param->firstId + instanceIndex;
        state->motionIndex = -1;
        for (slot = 0; slot < 1; slot++) {
            object->mdlId[slot] = -1;
        }
        for (slot = 0; slot < 8; slot++) {
            object->mtnId[slot] = -1;
        }
        if (param->modelResource >= 0) {
            if (param->linkModel != 0) {
                /* Discard the archive-number result, then call the model cache without an explicit
                 * data number. */
                fn_1_E18(param->modelResource);
                model = fn_1_4994();
            } else {
                model = Hu3DModelCreate(HuDataSelHeapReadNum(fn_1_E18(param->modelResource),
                    HU_MEMNUM_OVL, HEAP_MODEL));
            }
        } else {
            model = -1;
        }
        object->mdlId[0] = model;
        motion = param->motions;
        if (motion != NULL) {
            for (slot = 0; slot < 8; slot++, motion++) {
                resourceIndex = motion->resource;
                if (resourceIndex == -2) {
                    break;
                }
                if (resourceIndex != -1) {
                    object->mtnId[slot] = fn_1_4A9C(model, fn_1_E18(resourceIndex));
                }
            }
        }
        if (param->position != NULL) {
            state->position = *param->position;
        }
        work->startPosition = state->position;
        state->rotation.y = param->rotationY;
        if (model >= 0) {
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
}

/* Register a newly created prop for later type-and-instance lookups. */
void fn_1_8EA4(OMOBJ *object) {
    OMOBJ **entry = lbl_1_bss_4BC;
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

/* Find a prop by type and instance ID for stage-part queries and scripted actions. */
OMOBJ *fn_1_8EF8(s32 type, s32 id)
{
    OMOBJ *object;
    OMOBJ **entry = lbl_1_bss_4BC;
    M674PropWork *work;
    s32 index;

    for (index = 0; index < 128; index++, entry++) {
        object = *entry;
        if (object != NULL) {
            work = object->data;
            if (work->param->type == type && work->id == id) {
                return object;
            }
        }
    }
    return NULL;
}

/* Apply a requested prop motion, blending from the previous motion when configured. */
void fn_1_8F80(OMOBJ *object, s32 motionIndex)
{
    M674ActorState *state;
    M674PropMotionParam *param;
    M674PropWork *work = object->data;
    s32 model, motion;

    state = &work->state;
    if (state->motionIndex != motionIndex && motionIndex < 8) {
        motion = object->mtnId[motionIndex];
        work->timer = 0;
        state->motionFrame = 0;
        if (motion == -1) {
            state->motionActive = 0;
            return;
        }
        param = &work->param->motions[motionIndex];
        model = object->mdlId[0];
        if (state->motionIndex < 0 || 0.0 == param->blend) {
            Hu3DMotionSet(model, motion);
            Hu3DModelAttrSet(model, param->attr);
        } else {
            Hu3DMotionShiftSet(model, motion, 0.0f, 60.0f * param->blend, param->attr);
        }
        state->motionIndex = motionIndex;
    }
}

/* Tell the prop motion updater when playback and any motion blend have both finished. */
s32 fn_1_908C(OMOBJ *object)
{
    s32 modelId = object->mdlId[0];

    if (Hu3DMotionEndCheck((s16)modelId) != 0 &&
        Hu3DMotionShiftIDGet((s16)modelId) < 0) {
        return 1;
    }
    return 0;
}

/* Copy a named part's current transform when found, adding 90 degrees to yaw; return whether
 * model motion is unfinished even if the part was absent and outputs were left unchanged. */
BOOL fn_1_90F4(s32 type, s32 id, char *name, Vec *position, Vec *rotation)
{
    OMOBJ *object;
    M674PropWork *work;
    HSF_OBJECT *part;
    s32 model;

    object = fn_1_8EF8(type, id);
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
        /* The returned facing includes a ninety-degree offset from the model part. */
        rotation->y = (90.0) + part->mesh.curr.rot.y;
        rotation->z = part->mesh.curr.rot.z;
    }
    return !Hu3DMotionEndCheck(model);
}

/* Update a prop's model transform and motion timer when used as its object callback. */
void fn_1_9230(OMOBJ *object)
{
    fn_1_9268(object);
    fn_1_9324(object);
}

/* Copy the current prop transform to its model after a movement or collision update. */
void fn_1_9268(OMOBJ *object)
{
    M674PropWork *work = object->data;
    M674PropWork *state = work;
    s32 modelId = object->mdlId[0];
    state->state.rotation.y = fmodf(state->state.rotation.y, 360.0f);
    Hu3DModelPosSetV((s16)modelId, &state->state.position);
    Hu3DModelRotSetV((s16)modelId, &state->state.rotation);
}

/* Advance the prop timer and clear completed motion requests after each model update. */
void fn_1_9324(OMOBJ *object)
{
    M674ActorState *state;
    M674PropWork *work = object->data;

    state = &work->state;
    if (state->motionActive) {
        if (fn_1_908C(object)) {
            state->motionActive = 0;
            state->motionTimer = 0;
        }
    } else {
        state->motionTimer = 0;
    }
    work->timer++;
}

/* Leave static scenery unchanged during its object updates. */
void fn_1_93D0(OMOBJ *object)
{
}

const M674AnnularRegionRecord lbl_1_rodata_2B8 = {
    {1,4,2,0,10,NULL,NULL,NULL}, {{0,0,0},-500,580,600,600,1200,1200}
};
const M674EllipseRecord lbl_1_rodata_2FC = {
    {1,4,2,1,11,NULL,NULL,NULL}, {{0,0,30},680,700,1}
};
const M674EllipseRecord lbl_1_rodata_334 = {
    {1,8,2,2,11,NULL,NULL,NULL}, {{0,0,0},520,520,1}
};
/* Register the stage's annular collision volume and two ellipse boundaries on the first update. */
void fn_1_93D4(OMOBJ *object)
{
    M674RegionRecord *configuration;
    void *objectData;
    objectData = object->data;
    configuration = lbl_1_bss_3E0;
    memset(configuration, 0, sizeof(lbl_1_bss_3E0));
    configuration->annulus = lbl_1_rodata_2B8;
    fn_1_112A8(&configuration++->entry);
    configuration->ellipse = lbl_1_rodata_2FC;
    fn_1_112A8(&configuration++->entry);
    configuration->ellipse = lbl_1_rodata_334;
    fn_1_112A8(&configuration++->entry);
    object->objFunc = fn_1_9578;
}

/* Keep the stage-region object idle after its regions have been registered. */
void fn_1_9578(OMOBJ *object)
{
}

/* Select the scenery model's inf_light light for the stage on its first update. */
void fn_1_957C(OMOBJ *object)
{
    s32 *work = object->data;
    s32 model = object->mdlId[0];

    fn_1_1218(model, "inf_light", 1);
    object->objFunc = fn_1_95DC;
}

/* Keep the light-source scenery idle after its light has been selected. */
void fn_1_95DC(OMOBJ *object)
{
}

/* Install the scenery animation controller on the object's first update. */
void fn_1_95E0(OMOBJ *object)
{
    object->objFunc = fn_1_95F0;
}

/* Hold the scenery animation through the introduction, then play it until the outgoing results
 * wipe completes. */
void fn_1_95F0(OMOBJ *object)
{
    M674PropWork *work = object->data;
    s32 modelId = *object->mdlId;

    switch (work->phase) {
    case 0:
        (work->phase)++;
        Hu3DMotionTimeSet(modelId, (0.0f));
        Hu3DModelAttrSet(modelId, HU3D_MOTATTR_PAUSE);
        break;
    case 1:
        if (fn_1_DB40()) {
            break;
        }
        (work->phase)++;
        work->actionTimer = 0;
    case 2:
        if (++work->actionTimer < 12) {
            break;
        }
        (work->phase)++;
        Hu3DModelAttrReset(modelId, HU3D_MOTATTR_PAUSE);
    case 3:
        if (!fn_1_E20()) {
            break;
        }
        (work->phase)++;
        Hu3DMotionTimeSet(modelId, (0.0f));
        Hu3DModelAttrSet(modelId, HU3D_MOTATTR_PAUSE);
    }
}

/* Set the wobbling prop's initial facing and install its frame update. */
void fn_1_9720(OMOBJ *object)
{
    M674PropWork *state;
    M674PropWork *work = object->data;

    state = work;
    state->state.rotation.y = (-30.0f);
    work->pitchPhase = (0.0f);
    work->rollPhase = (0.0f);
    object->objFunc = fn_1_9780;
}

/* Rock the prop each frame; a scripted impact request restarts its bounce and emits a ring and
 * particles. */
void fn_1_9780(OMOBJ *object)
{
    M674PropWork *work;
    M674PropWork *state;
    Vec initialPosition;
    f32 wobbleDegrees;
    work = object->data;
    state = work;

    if (work->actionIndex != 0) {
        work->actionIndex = 0;
        work->amplitude = (1.0f);
        work->phaseAngle = (0.0f);
        initialPosition = state->state.position;
        fn_1_F9A4(&initialPosition, (120.0f), (3.0f),
            (0.05f), 160);
        fn_1_9B90(object);
    }

    work->amplitude *= 0.95;
    work->phaseAngle = fmodf(work->phaseAngle - (10.0f), 360.0f);

    wobbleDegrees = (f32)((1.5) +
        (3.0) * (f64)work->amplitude);
    state->state.rotation.x = (f32)(wobbleDegrees * sin(((3.141592653589793) *
        (f64)work->pitchPhase) / (180.0)));
    state->state.rotation.z = (f32)(wobbleDegrees * sin(((3.141592653589793) *
        (f64)work->rollPhase) / (180.0)));

    wobbleDegrees = (f32)((1.0) +
        (1.5) * (f64)work->amplitude);
    work->pitchPhase = fmodf(work->pitchPhase + wobbleDegrees, 360.0f);
    work->rollPhase = fmodf((f32)((f64)work->rollPhase +
        (1.5) * (f64)wobbleDegrees), 360.0f);

    state->state.position.y = (f32)((f64)work->startPosition.y +
        (25.0) * ((f64)work->amplitude *
            sin(((3.141592653589793) * (f64)work->phaseAngle) /
                (180.0))));

    fn_1_9268(object);

    fn_1_9324(object);
}

Vec lbl_1_data_5A4[4] = {{-300,0,-300},{300,0,-300},{300,0,300},{-300,0,300}};
/* Emit scattered particles around the wobbling prop's rotated perimeter when it is struck. */
void fn_1_9B90(OMOBJ *object)
{
    Mtx rotationMatrix;
    Vec rotatedPoints[4];
    Vec point;
    M674PropWork *rotationData = object->data;
    M674PropWork *positionData = rotationData;
    f32 interpolation;
    s32 pointIndex;
    s32 sampleIndex;

    PSMTXRotRad(rotationMatrix, PROP_ROTATION_AXIS_Y,
        (0.017453292f) * positionData->state.rotation.y);
    PSMTXMultVecArraySR(rotationMatrix, lbl_1_data_5A4, rotatedPoints, 4);
    for (pointIndex = 0; pointIndex < 4; pointIndex++) {
        Vec *startPoint = &rotatedPoints[pointIndex];
        Vec *endPoint = &rotatedPoints[(pointIndex + 1) & 3];
        for (sampleIndex = 0; sampleIndex < 4; sampleIndex++) {
            interpolation = (f32)((0.0000152587890625) * (f64)(f32)(s32)(frand() & 0xFFFF));
            point.x = positionData->state.position.x + startPoint->x +
                       interpolation * (endPoint->x - startPoint->x);
            point.y = positionData->state.position.y;
            point.z = positionData->state.position.z + startPoint->z +
                       interpolation * (endPoint->z - startPoint->z);
            fn_1_F660(&point);
        }
    }
}

/* Give each small ambient prop a random position, heading and scale on its first update. */
void fn_1_9CF0(OMOBJ *object)
{
    M674PropWork *work = object->data;
    M674PropWork *state = work;
    s32 modelId = object->mdlId[0];
    f32 scale;

    work->positionTimer = 0;
    state->state.position.x = (f32)((f64)state->state.position.x +
        (1000.0) * ((0.000030517578125) * (f64)(f32)(
            (s32)(frand() & 0xFFFF) - RANDOM_U16_MIDPOINT)));
    state->state.position.y = (f32)((f64)state->state.position.y -
        (15.0) * (f64)work->id);
    state->state.position.z = (f32)((f64)state->state.position.z +
        (1000.0) * ((0.000030517578125) * (f64)(f32)(
            (s32)(frand() & 0xFFFF) - RANDOM_U16_MIDPOINT)));
    state->state.rotation.y = (f32)((360.0) *
        ((0.0000152587890625) * (f64)(f32)(s32)(frand() & 0xFFFF)) -
        (180.0));
    scale = (f32)((0.2) +
        ((0.2) * ((0.0000152587890625) *
            (f64)(f32)(s32)(frand() & 0xFFFF))));
    Hu3DModelScaleSet((s16)modelId, scale, scale, scale);
    object->objFunc = fn_1_9F44;
}

/* Steer an ambient prop toward periodically chosen destinations during its frame updates. */
void fn_1_9F44(OMOBJ *object)
{
    M674PropWork *work = object->data;
    M674PropWork *state = work;
    f32 speed = (2.5f);
    f32 angle;
    f32 desiredVelocityX;
    f32 desiredVelocityZ;

    if (--work->positionTimer <= 0) {
        work->positionTimer = (s32)((120.0) *
            ((0.0000152587890625) * (f64)(f32)(s32)(frand() & 0xFFFF))) + 60;
        state->state.targetPosition.x = (f32)((1000.0) *
            ((0.000030517578125) * (f64)(f32)((s32)(frand() & 0xFFFF) - RANDOM_U16_MIDPOINT)));
        state->state.targetPosition.y = state->state.position.y;
        state->state.targetPosition.z = (f32)((1000.0) *
            ((0.000030517578125) * (f64)(f32)((s32)(frand() & 0xFFFF) - RANDOM_U16_MIDPOINT)));
    }

    angle = fn_1_4D7C(&state->state.position, &state->state.targetPosition);
    state->state.rotation.y = fn_1_4BAC(state->state.rotation.y, angle,
        (0.02f));
    desiredVelocityX = (f32)(speed *
        sin(((3.141592653589793) * (f64)state->state.rotation.y) /
            (180.0)));
    state->state.velocity.x = (f32)((f64)state->state.velocity.x + (0.1) *
        (f64)(desiredVelocityX - state->state.velocity.x));
    desiredVelocityZ = (f32)(speed *
        cos(((3.141592653589793) * (f64)state->state.rotation.y) /
            (180.0)));
    state->state.velocity.z = (f32)((f64)state->state.velocity.z + (0.1) *
        (f64)(desiredVelocityZ - state->state.velocity.z));
    PSVECAdd(&state->state.position, &state->state.velocity, &state->state.position);

    fn_1_9268(object);
    fn_1_9324(object);
}

/* Scatter the slower ambient props and initialize their movement timers on the first update. */
void fn_1_A2D8(OMOBJ *object) {
    M674PropWork *stateData;
    M674PropWork *effectData;
    s32 modelId;
    stateData = object->data;
    effectData = stateData;
    modelId = * object->mdlId;
    stateData->positionTimer = 0;
    stateData->actionTimer = 0;
    effectData->state.position.x +=
        (f32) ((s32) (frand() & 0xFFFF) - RANDOM_U16_MIDPOINT) * (0.000030517578125) * (800.0);
    effectData->state.position.z +=
        ((f32) ((s32) (frand() & 0xFFFF) - RANDOM_U16_MIDPOINT) * (0.000030517578125) * (800.0));
    object->objFunc = fn_1_A400;
}

/* Alternate drifting and resting for a slow ambient prop during its frame updates. */
void fn_1_A400(OMOBJ *object)
{
    f32 velocityScale;
    f32 angle;
    f32 desiredVelocityX;
    f32 desiredVelocityZ;
    s32 modelId;
    M674PropWork *work = object->data;
    M674PropWork *state = work;

    velocityScale = (0.8333333f);
    modelId = object->mdlId[0];
    if (--work->positionTimer <= 0) {
        work->positionTimer =
            (s32)((120.0) * ((0.0000152587890625) *
                (f64)(f32)(s32)(frand() & 0xFFFF))) + 180;
        state->state.targetPosition.x = (f32)((800.0) *
            ((0.000030517578125) *
                (f64)(f32)((s32)(frand() & 0xFFFF) - RANDOM_U16_MIDPOINT)));
        state->state.targetPosition.y = state->state.position.y;
        state->state.targetPosition.z = (f32)((800.0) *
            ((0.000030517578125) *
                (f64)(f32)((s32)(frand() & 0xFFFF) - RANDOM_U16_MIDPOINT)));
    }

    if (--work->actionTimer <= 0) {
        work->actionTimer =
            (s32)((120.0) * ((0.0000152587890625) *
                (f64)(f32)(s32)(frand() & 0xFFFF))) + 240;
    }

    if (work->actionTimer <= 180) {
        angle = fn_1_4D7C(&state->state.position, &state->state.targetPosition);
        desiredVelocityX = (f32)(velocityScale *
            sin(((3.141592653589793) * angle) / (180.0)));
        desiredVelocityZ = (f32)(velocityScale *
            cos(((3.141592653589793) * angle) / (180.0)));
    } else {
        desiredVelocityX = desiredVelocityZ = (0.0f);
    }

    state->state.velocity.x = (f32)((f64)state->state.velocity.x +
        (0.1) * (f64)(desiredVelocityX - state->state.velocity.x));
    state->state.velocity.z = (f32)((f64)state->state.velocity.z +
        (0.1) * (f64)(desiredVelocityZ - state->state.velocity.z));
    PSVECAdd(&state->state.position, &state->state.velocity, &state->state.position);
    Hu3DModelPosSetV(modelId, &state->state.position);
}

/* Allocate a manager's hidden model pool when its collection-effect object is initialized. */
void fn_1_A730(M674PropWork *owner, s32 resource, s32 count)
{
    s32 index;
    M674ModelEntry *entries;
    s32 allocationSize;

    allocationSize = count * sizeof(M674ModelEntry);
    entries = HuMemDirectMallocNum(HEAP_HEAP, allocationSize, HU_MEMNUM_OVL);
    memset(entries, 0, allocationSize);
    owner->models = entries;
    for (index = 0; index < count; index++, entries++) {
        /* Discard the archive-number result, then call the model cache without an explicit data
         * number. */
        fn_1_E18(resource);
        entries->modelId = fn_1_4994();
        Hu3DModelAttrSet((s16)entries->modelId, HU3D_ATTR_DISPOFF);
    }
}

/* Project a point 30 screen pixels below the player's score box, at depth 1000, into the world. */
void fn_1_A7CC(s32 scoreIndex, Point3d *destination)
{
    Point3d source;

    fn_1_10150(scoreIndex, &source.x, &source.y);
    source.z = (1000.0f);
    source.y += 30.0;
    Hu3D2Dto3D(&source, 1, destination);
}

/* Create the single-item collection model pool on the effect manager's first update. */
void fn_1_A83C(OMOBJ *object)
{
    M674PropWork *storage = object->data;

    lbl_1_bss_4B8 = object;
    fn_1_A730(storage, 1, 32);
    lbl_1_bss_4B0 = 0;
    object->objFunc = fn_1_A8FC;
}

Vec lbl_1_data_5D4 = {-45, 245, 25};
/* Animate collected models each frame, sending scored items to the player's score box. */
void fn_1_A8FC(OMOBJ *object)
{
    M674PropWork *storage = object->data;
    M674ModelEntry *entry = storage->models;
    s32 index;
    s32 modelIndex;
    Vec targetPosition;
    Vec position;

    for (index = 0; index < 32; index++, entry++) {
        switch (entry->state) {
        case 1:
            entry->state = 4;
            entry->timer = 10;
            entry->scale = 0.8f;
            entry->rotation.x = 0.0f;
            entry->rotation.y = 0.0f;
            entry->rotation.z = 0.0f;
            entry->yawStep = 0.0f;
            Hu3DModelAttrReset((s16)entry->modelId, HU3D_ATTR_DISPOFF);
            fn_1_4040(MSM_SE_M674_SCORE_FLIGHT, &entry->position);
            break;
        case 2:
            entry->state = 6;
            entry->scale = 0.8f;
            entry->rotation.x = 0.0f;
            entry->rotation.y = 0.0f;
            entry->rotation.z = 0.0f;
            entry->yawStep = 5.0f;
            Hu3DModelAttrReset((s16)entry->modelId, HU3D_ATTR_DISPOFF);
            fn_1_4040(MSM_SE_M674_SCORE_FLIGHT, &entry->position);
            break;
        case 3:
            entry->state = 8;
            entry->timer = 15;
            entry->scale = 1.0f;
            entry->rotation.x = 0.0f;
            entry->rotation.y = 0.0f;
            entry->rotation.z = 0.0f;
            entry->yawStep = 0.0f;
            Hu3DModelAttrReset((s16)entry->modelId, HU3D_ATTR_DISPOFF);
            break;
        case 4:
            if (entry->timer <= 5) {
                entry->scale += 0.05;
            }
            entry->position.y += 18.0;
            if (--entry->timer <= 0) {
                entry->state++;
                entry->timer = 12;
                /* fall through */
            } else {
                break;
            }
        case 5:
            if (entry->timer < 8) {
                if ((entry->scale = (f32)((f64)entry->scale - 0.08)) < 0.8) {
                    entry->scale = 0.8f;
                }
            }
            if (--entry->timer <= 0) {
                /* fall through */
            } else {
                break;
            }
        case 6:
            entry->state = 7;
            entry->progress = 0.0f;
            entry->scale = 0.8f;
            entry->yawStep = 6.0f;
            entry->trajectoryVector = entry->position;
            /* The score destination follows the owning player's screen-space score box. */
            fn_1_A7CC(entry->scoreIndex, &entry->targetPosition);
            /* fall through */
        case 7:
            entry->progress += 0.02;
            fn_1_4F84(&entry->trajectoryVector, &entry->targetPosition,
                entry->progress, &entry->position);
            if (!(entry->progress < 1.0)) {
                entry->state = 0;
                fn_1_10028(entry->scoreIndex, 1);
                Hu3DModelAttrSet((s16)entry->modelId, HU3D_ATTR_DISPOFF);
            }
            break;
        case 8:
            if (entry->timer <= 5) {
                entry->scale += 0.05;
            }
            entry->position.y += 15.0;
            if (--entry->timer <= 0) {
                entry->state++;
                entry->timer = 15;
                /* fall through */
            } else {
                break;
            }
        case 9:
            if (entry->timer < 10) {
                if ((entry->scale = (f32)((f64)entry->scale - 0.05)) < 1.0) {
                    entry->scale = 1.0f;
                }
            }
            if (--entry->timer <= 0) {
                entry->state++;
                entry->scale = 1.0f;
                PSVECAdd(&lbl_1_data_270[1], &lbl_1_data_5D4, &targetPosition);
                entry->timer = fn_1_4FDC(&entry->position, &targetPosition,
                    0.5448139f, 1600.0f, &entry->trajectoryVector);
                /* fall through */
            } else {
                break;
            }
        case 10:
            if ((entry->rotation.y += 3.0) > 180.0) {
                entry->rotation.y = 180.0f;
            }
            if ((entry->rotation.z -= 3.0) < -190.0) {
                entry->rotation.z = -190.0f;
            }
            entry->trajectoryVector.y = (f32)((f64)entry->trajectoryVector.y -
                0.5448138888888889);
            PSVECAdd(&entry->position, &entry->trajectoryVector, &entry->position);
            if (entry->timer <= 16) {
                lbl_1_bss_4B0 = 1;
            }
            if (--entry->timer <= 0) {
                entry->state = 0;
                Hu3DModelAttrSet((s16)entry->modelId, HU3D_ATTR_DISPOFF);
            }
            break;
        }

        if (entry->state != 0) {
            entry->rotation.y += entry->yawStep;
            Hu3DModelPosSetV((s16)entry->modelId, &entry->position);
            Hu3DModelRotSetV((s16)entry->modelId, &entry->rotation);
            Hu3DModelScaleSet((s16)entry->modelId, entry->scale,
                entry->scale, entry->scale);
        }
    }
}

/* Create the five-item collection model pool on the effect manager's first update. */
void fn_1_AF30(OMOBJ *object)
{
    M674PropWork *storage = object->data;

    lbl_1_bss_4B4 = object;
    fn_1_A730(storage, 2, 4);
    object->objFunc = fn_1_AFE0;
}

/* Animate a five-item collection each frame, then release five single-item score flights. */
void fn_1_AFE0(OMOBJ *object)
{
    M674PropWork *storage = object->data;
    M674ModelEntry *entry = storage->models;
    s32 index;

    for (index = 0; index < 4; index++, entry++) {
        switch (entry->state) {
        case 0:
            break;
        case 1:
            entry->state++;
            entry->timer = 10;
            entry->scale = 1.0f;
            Hu3DModelAttrReset((s16)entry->modelId, HU3D_ATTR_DISPOFF);
            /* fall through */
        case 2:
            if (entry->timer <= 5) {
                entry->scale += 0.05;
            }
            entry->position.y += 18.0;
            if (--entry->timer > 0) {
                break;
            }
            entry->state++;
            entry->timer = 15;
            /* fall through */
        case 3:
            if (entry->timer < 10) {
                if ((entry->scale = (f32)((f64)entry->scale - 0.05)) < 1.0) {
                    entry->scale = 1.0f;
                }
            }
            if (--entry->timer > 0) {
                break;
            }
            entry->state++;
            entry->progress = 0.0f;
            /* A five-item reward becomes five independent score flights. */
            entry->timer = 5;
            Hu3DModelAttrSet((s16)entry->modelId, HU3D_ATTR_DISPOFF);
            /* fall through */
        case 4:
            if (!((entry->progress = (f32)((f64)entry->progress - 1.0)) >= 0.0)) {
                Vec spawnPosition;

                entry->progress = 9.0f;
                spawnPosition = entry->position;
                spawnPosition.y += 30.0;
                fn_1_DB50(2, entry->scoreIndex, &spawnPosition);
                if (--entry->timer <= 0) {
                    entry->state = 0;
                }
            }
            break;
        }
        if (entry->state != 0) {
            Hu3DModelPosSetV((s16)entry->modelId, &entry->position);
            Hu3DModelScaleSet((s16)entry->modelId, entry->scale, entry->scale, entry->scale);
        }
    }
}

/* Clear the target registry and assign the target reward kinds during minigame setup. */
void fn_1_B27C(void *object) {
    s32 tableIndex;
    s32 loopIndex;

    tableIndex = 0;
    loopIndex = 0;
    while (loopIndex < 26) {
        lbl_1_bss_310[loopIndex] = 0;
        loopIndex++;
    }

    lbl_1_bss_378[tableIndex++] = 9;
    for (loopIndex = 0; loopIndex < 15; loopIndex++) {
        lbl_1_bss_378[tableIndex++] = 1;
    }
    for (loopIndex = 0; loopIndex < 4; loopIndex++) {
        lbl_1_bss_378[tableIndex++] = 5;
    }
    while (tableIndex < 26) {
        lbl_1_bss_378[tableIndex++] = 0;
    }
}

/* Place a target and register its collision body on the target's first update. */
void fn_1_B37C(OMOBJ *object)
{
    s32 modelId;
    s32 tableIndex;
    M674PropWork *work = object->data;
    M674PropWork *state = work;
    M674PropWork *modelState;

    tableIndex = work->id;
    modelId = object->mdlId[0];
    lbl_1_bss_310[tableIndex] = object;
    if (tableIndex != 0) {
        state->state.position.x = (f32)((450.0) *
            ((0.000030517578125) * (f64)(f32)((s32)(frand() & 0xFFFF) - RANDOM_U16_MIDPOINT)));
        state->state.position.z = (f32)((450.0) *
            ((0.000030517578125) * (f64)(f32)((s32)(frand() & 0xFFFF) - RANDOM_U16_MIDPOINT)));
        state->state.rotation.y = (f32)((360.0) *
            ((0.0000152587890625) * (f64)(f32)(s32)(frand() & 0xFFFF)));
        work->pitchPhase = (f32)((360.0) *
            ((0.000030517578125) * (f64)(f32)((s32)(frand() & 0xFFFF) - RANDOM_U16_MIDPOINT)));
        work->rollPhase = (f32)((360.0) *
            ((0.000030517578125) * (f64)(f32)((s32)(frand() & 0xFFFF) - RANDOM_U16_MIDPOINT)));
    }

    /* Discard the archive-number result, then call the model cache without an explicit data
     * number. */
    fn_1_E18(24);
    work->secondaryModel = fn_1_4994();
    Hu3DModelLayerSet((s16)work->secondaryModel, 3);
    Hu3DModelAttrSet((s16)work->secondaryModel, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet((s16)work->secondaryModel, HU3D_MOTATTR_PAUSE);
    modelState = state;
    modelState->state.task.flags = 1;
    modelState->state.task.mask = TARGET_INITIAL_COLLISION_MASK;
    modelState->state.task.type = 3;
    modelState->state.task.id = tableIndex;
    modelState->state.task.state = 0;
    modelState->state.task.primaryRelation = object;
    modelState->state.task.invoke = fn_1_BD0C;
    modelState->state.task.shape.actor.callback = 0;
    modelState->state.task.shape.actor.position = &state->state.position;
    modelState->state.task.shape.actor.velocity = &state->state.velocity;
    modelState->state.task.shape.actor.initialPosition =
        *(Vec *) modelState->state.task.shape.actor.position;
    if (tableIndex != 0) {
        modelState->state.task.shape.actor.radius = (100.0f);
        modelState->state.task.shape.actor.collisionFlag = (1.0f);
    } else {
        modelState->state.task.shape.actor.radius = (120.0f);
        modelState->state.task.shape.actor.collisionFlag = (0.0f);
    }
    fn_1_11218(&modelState->state.task);
    object->objFunc = fn_1_B6A4;
}

/* Animate a target each frame, then show its collection effects and fade its replacement model. */
void fn_1_B6A4(OMOBJ *object)
{
    M674PropWork *work = object->data;
    M674PropWork *state = work;
    s32 tableIndex = work->id;
    s32 modelId = object->mdlId[0];
    Vec effectPosition;
    f64 phaseXSum;
    f64 phaseZSum;
    f32 wrappedPhaseX;
    f32 normalizedPhaseX;
    f32 wrappedPhaseZ;
    f32 normalizedPhaseZ;

    switch (work->phase) {
    case 0:
        work->phase++;
        work->actionTimer = 0;
        /* fall through */
    case 1:
        if (++work->actionTimer >= 30) {
            work->phase++;
            work->actionTimer = (s32)(90.0 *
                (1.52587890625e-05 * (f64)(f32)(
                    (s32)(frand() & 0xFFFF)))) + 30;
            /* After the appearance delay, use the smaller collision body. */
            state->state.task.mask = TARGET_COLLISION_MASK;
            state->state.task.shape.actor.radius = 60.0f;
            state->state.task.shape.actor.collisionFlag = 0.0f;
            /* fall through */
        } else {
            break;
        }
    case 2:
        if (--work->actionTimer <= 0) {
            work->actionTimer = (s32)(120.0 *
                (1.52587890625e-05 * (f64)(f32)(
                    (s32)(frand() & 0xFFFF)))) + 120;
            effectPosition = state->state.position;
            effectPosition.y += 30.0;
            fn_1_F9A4(&effectPosition, 50.0f, 0.3f, 0.04f, 112);
        }
        if (state->state.blendRequest != 0) {
            work->phase++;
            work->actionTimer = 0;
            state->state.task.flags &= ~1;
            Hu3DModelAttrSet(modelId, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrReset((s16)work->secondaryModel, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrReset((s16)work->secondaryModel, HU3D_MOTATTR_PAUSE);
            fn_1_4040(MSM_SE_M674_TARGET_COLLECT, &state->state.position);
            effectPosition = state->state.position;
            switch (lbl_1_bss_378[tableIndex]) {
            case 1:
                {
                    effectPosition.y += 10.0;
                    fn_1_DB50(0, state->state.partnerScoreIndex, &effectPosition);
                }
                break;
            case 5:
                {
                    effectPosition.y -= 10.0;
                    fn_1_DB50(1, state->state.partnerScoreIndex, &effectPosition);
                }
                break;
            case 9:
                {
                    effectPosition.y -= 40.0;
                    fn_1_DB50(3, state->state.partnerScoreIndex, &effectPosition);
                }
                break;
            }
            {
                effectPosition = state->state.position;
                effectPosition.y += 30.0;
                fn_1_F9A4(&effectPosition, 40.0f, 0.3f, 0.05f, 176);
            }
            /* fall through */
        } else {
            break;
        }
    case 3:
        if (work->actionTimer < 4) {
            effectPosition = state->state.position;
            effectPosition.y += 30.0;
            fn_1_F660(&effectPosition);
        }
        if (++work->actionTimer >= 45) {
            work->phase++;
            work->actionTimer = 0;
            /* fall through */
        } else {
            break;
        }
    case 4:
        Hu3DModelTPLvlSet((s16)work->secondaryModel,
            (f32)(1.0 - ((f64)(f32)work->actionTimer / 24.0)));
        if (++work->actionTimer >= 24) {
            work->phase++;
            Hu3DModelAttrSet((s16)work->secondaryModel, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet((s16)work->secondaryModel, HU3D_MOTATTR_PAUSE);
        }
        break;
    }

    state->state.rotation.x = (f32)(3.0 * sin((3.141592653589793 *
        (f64)work->pitchPhase) / 180.0));
    state->state.rotation.y = (f32)((f64)state->state.rotation.y +
        (0.2 * (1.52587890625e-05 *
            (f64)(f32)(s32)(frand() & 0xFFFF))));
    state->state.rotation.z = (f32)(3.0 * sin((3.141592653589793 *
        (f64)work->rollPhase) / 180.0));
    phaseXSum = 2.0 + (f64)work->pitchPhase;
    phaseXSum = (f64)(f32)phaseXSum;
    wrappedPhaseX = (f32)fmod(phaseXSum, 360.0);
    normalizedPhaseX = wrappedPhaseX;
    work->pitchPhase = normalizedPhaseX;
    phaseZSum = 2.0 + (f64)work->rollPhase;
    phaseZSum = (f64)(f32)phaseZSum;
    wrappedPhaseZ = (f32)fmod(phaseZSum, 360.0);
    normalizedPhaseZ = wrappedPhaseZ;
    work->rollPhase = normalizedPhaseZ;
}

/* Position both target models below the collision center when collision processing invokes the
 * target. */
void fn_1_BD0C(OMOBJ *object)
{
    M674PropWork *work = object->data;
    M674PropWork *state = work;
    Point3d position;
    s32 modelId;

    modelId = object->mdlId[0];
    position = state->state.position;
    position.y = (f32)((f64)position.y - (40.0));
    Hu3DModelPosSetV(modelId, &position);
    Hu3DModelRotSetV(modelId, &state->state.rotation);
    Hu3DModelPosSetV((s16)work->secondaryModel, &position);
    Hu3DModelRotSetV((s16)work->secondaryModel, &state->state.rotation);
}

/* Register the scripted actor's collision body and shadow model on its first update. */
void fn_1_BDC8(OMOBJ *object)
{
    M674PropWork *initialWork = object->data;
    M674PropWork *modelWork = initialWork;
    M674PropWork *work = modelWork;

    work->state.task.flags = 1;
    work->state.task.mask = TARGET_COLLISION_MASK;
    work->state.task.type = 1;
    work->state.task.id = 0;
    work->state.task.state = 0;
    work->state.task.primaryRelation = object;
    work->state.task.invoke = fn_1_C11C;
    work->state.task.shape.actor.callback = fn_1_7660;
    work->state.task.shape.actor.position = &modelWork->state.position;
    work->state.task.shape.actor.velocity = &modelWork->state.velocity;
    work->state.task.shape.actor.initialPosition = *work->state.task.shape.actor.position;
    work->state.task.shape.actor.radius = (80.0f);
    work->state.task.shape.actor.collisionFlag = (1.0f);
    fn_1_11218(&work->state.task);
    /* Discard the archive-number result, then call the model cache without an explicit data
     * number. */
    fn_1_E18(3);
    modelWork->state.shadowModel = fn_1_4994();
    Hu3DModelLayerSet((s16)modelWork->state.shadowModel, 3);
    Hu3DModelShadowSet((s16)modelWork->state.shadowModel);
    Hu3DModelAttrSet((s16)modelWork->state.shadowModel, HU3D_ATTR_MOTION_OFF);
    Hu3DModelScaleSet((s16)modelWork->state.shadowModel, (0.6f),
        (0.6f), (0.6f));
    Hu3DModelShadowDispOff((s16)modelWork->state.shadowModel);
    modelWork->state.shadowEnabled = 0;

    fn_1_8F80(object, 0);
    object->objFunc = fn_1_BFF0;
}

/* Run the actor's introduction, repeating actions and finish sequence on each object update. */
void fn_1_BFF0(OMOBJ *object)
{
    M674PropWork *work = object->data;
    M674PropWork *state = work;

    fn_1_8894(&state->state);
    fn_1_67B0(&state->state);
    switch (work->phase) {
    case 0:
        work->phase++;
        state->state.phase = 0;
    case 1:
        if (fn_1_C2E8(state)) {
            state->state.task.shape.actor.collisionFlag = (0.0f);
            work->phase++;
            state->state.phase = 0;
        }
        break;
    case 2:
        if (fn_1_C61C(state)) {
            work->phase++;
            state->state.phase = 0;
        }
        break;
    case 3:
        if (fn_1_C7E0(state)) {
            work->phase++;
            state->state.phase = 0;
        }
        break;
    }
    fn_1_D8CC(object);
    PSVECAdd(&state->state.position, &state->state.velocity, &state->state.position);
}

/* Update the actor's transform and choose its visible shadow after collision processing. */
void fn_1_C11C(OMOBJ *object)
{
    M674PropWork *initialWork = object->data;
    M674PropWork *shadowWork = initialWork;
    s32 modelId = object->mdlId[0];

    if (shadowWork->state.shadowEnabled != 0) {
        Hu3DModelShadowDispOff((s16)modelId);
        if (shadowWork->state.position.y < (500.0)) {
            Hu3DModelShadowDispOn((s16)shadowWork->state.shadowModel);
        } else {
            Hu3DModelShadowDispOff((s16)shadowWork->state.shadowModel);
        }
        Hu3DModelPosSetV((s16)shadowWork->state.shadowModel,
            (HuVecF *)&shadowWork->state.position);
    } else {
        if (shadowWork->state.position.y < (500.0)) {
            Hu3DModelShadowDispOn((s16)modelId);
        } else {
            Hu3DModelShadowDispOff((s16)modelId);
        }
        Hu3DModelShadowDispOff((s16)shadowWork->state.shadowModel);
    }

    fn_1_9268(object);
    fn_1_9324(object);
}

Vec lbl_1_data_60C = {-1000, 300, 500};
/* Run the actor's introductory jumps and animation until the main game phase begins. */
s32 fn_1_C2E8(M674PropWork *work)
{
    M674PropWork *sequenceState = work;
    s32 finished = 0;

    switch (work->state.phase) {
    case 0:
        lbl_1_bss_4AC = 1;
        work->state.phase++;
        work->state.timer = 0;
        work->state.position = lbl_1_data_60C;
        work->state.rotation.y = (150.0f);
        /* fall through */
    case 1: {
        M674PropWork *selectedWork;
        OMOBJ *candidate;
        Vec approachPosition;

        if (++work->state.timer < 6) {
            break;
        }
        work->state.phase++;
        work->state.timer = 0;
        work->state.turnState = 0;
        candidate = fn_1_8EF8(4, 0);
        if (candidate != NULL) {
            selectedWork = candidate->data;
            approachPosition = selectedWork->state.position;
        }
        /* A failed lookup leaves approachPosition uninitialized; this adjustment and copy still
         * run. */
        approachPosition.y += (100.0);
        work->state.targetPosition = approachPosition;
        /* fall through */
    }
    case 2:
        if (fn_1_D5DC(work, (0.5448139f), (500.0f), 3) == 0) {
            break;
        }
        work->state.phase++;
        work->state.timer = 0;
        work->state.turnState = 0;
        work->state.targetPosition = sequenceState->startPosition;
        /* fall through */
    case 3:
        if (fn_1_D5DC(work, (1.0896277f), (700.0f), 18) == 0) {
            break;
        }
        work->state.phase++;
        work->state.timer = 0;
        work->state.turnState = 0;
        work->state.targetAngle = 0.0f;
        /* fall through */
    case 4:
        if (fn_1_D020(work, (0.3f)) != 0 && lbl_1_bss_4B0 != 0) {
            work->state.phase++;
            work->state.timer = 0;
            fn_1_67CC(&work->state, 7, (1.0f));
            work->state.motionActive = 1;
        }
        break;
    case 5:
        if (work->state.motionActive == 0) {
            work->state.phase++;
            work->state.timer = 0;
            lbl_1_bss_4AC = 0;
            fn_1_67CC(&work->state, 0, (1.0f));
            /* fall through */
        } else {
            break;
        }
    case 6:
        if ((u32)MgSeqModeGet() == 5) {
            work->state.phase++;
            work->state.timer = 0;
            work->state.shadowEnabled = 1;
        } else {
            break;
        }
        /* fall through */
    default:
        finished = 1;
        break;
    }
    return finished;
}

M674TargetAction lbl_1_data_634[4] = {fn_1_C8BC,fn_1_CB10,fn_1_C8BC,fn_1_CCCC};
/* Cycle the actor's scripted actions during play, then reset it for the finish sequence. */
s32 fn_1_C61C(M674PropWork *work)
{
    M674PropWork *state = work;
    s32 ready = 0;
    s32 callbackCount = 4;
    s32 nextCallback;

    switch (work->state.phase) {
    case 0:
        work->state.phase++;
        work->state.timer = 0;
        state->actionIndex = 0;
        /* fall through */
    case 1:
        if (MgSeqModeGet() != 5) {
            work->state.phase = 3;
            break;
        }
        work->state.phase++;
        work->state.actionPhase = 0;
        nextCallback = state->actionIndex + 1;
        /* Before wrapping, current and current + 1 cannot be equal for these four action
         * indices. */
        if (state->actionIndex == nextCallback) {
            nextCallback++;
        }
        if (nextCallback >= callbackCount) {
            nextCallback = 0;
        }
        state->actionIndex = nextCallback;
        /* fall through */
    case 2:
        if (lbl_1_data_634[state->actionIndex](work) != 0) {
            work->state.phase--;
        }
        /* fall through */
    case 3:
        if (fn_1_E20() == 0) {
            break;
        }
        work->state.phase = 4;
        work->state.position = state->startPosition;
        work->state.velocity.x = (0.0f);
        work->state.velocity.y = (0.0f);
        work->state.velocity.z = (0.0f);
        work->state.rotation.x = (0.0f);
        work->state.rotation.y = (0.0f);
        work->state.rotation.z = (0.0f);
        fn_1_67CC(&work->state, 0, (1.0f));
        work->state.shadowEnabled = 0;
        /* fall through */
    default:
        ready = 1;
        break;
    }
    return ready;
}

/* Play the actor's finish animation once all player finish flags are set. */
s32 fn_1_C7E0(M674PropWork *sequence)
{
    s32 done = 0;

    switch (sequence->state.phase) {
    case 0:
        sequence->state.phase++;
        /* fall through */
    case 1:
        if (fn_1_F3C() == 0) {
            break;
        }
        sequence->state.phase++;
        sequence->state.timer = 0;
        fn_1_67CC(&sequence->state, 6, (1.0f));
        /* fall through */
    case 2:
        if (++sequence->state.timer < 78) {
            break;
        }
        sequence->state.phase++;
        sequence->state.timer = 0;
        fn_1_4040(MSM_SE_GUIDE_11, &sequence->state.position);
        /* fall through */
    default:
        done = 1;
        break;
    }
    return done;
}

Vec lbl_1_data_644 = {0, 160, -850};
Vec lbl_1_data_650 = {760, 80, -620};
Vec lbl_1_data_65C = {0,0,0};
/* Jump to the side and pace along a sinusoidal path when selected by the action cycle. */
s32 fn_1_C8BC(M674PropWork *work)
{
    s32 ready = 0;
    f32 direction = (2.0f);
    f32 turnAngle;
    f32 offset;

    switch (work->state.actionPhase) {
    case 0:
        work->state.actionPhase++;
        work->state.turnState = 0;
        work->state.targetPosition = lbl_1_data_644;
        /* fall through */
    case 1:
        if (fn_1_D5DC(work, (1.0896277f),
                (1500.0f), 18) == 0) {
            break;
        }
        work->state.actionPhase++;
        work->state.actionFrame = 0;
        work->state.turnState = 0;
        work->state.targetAngle = (0.0f);
        if (((s32)frand() & 1) != 0) {
            work->state.actionPhase++;
        }
        /* fall through */
    case 2:
        direction = -direction;
        /* fall through */
    case 3:
        turnAngle = (f32)work->state.actionFrame * direction;
        offset = (100.0) * ((2.4) *
            sin(((3.141592653589793) * turnAngle) / (180.0)));
        work->state.targetPosition.x = lbl_1_data_644.x + offset;
        fn_1_D18C(work, (6.6666665f));
        /* fall through */
    case 4:
        if (++work->state.actionFrame < 180) {
            break;
        }
        work->state.actionPhase = -1;
        work->state.velocity.x = (0.0f);
        work->state.velocity.y = (0.0f);
        work->state.velocity.z = (0.0f);
        fn_1_67CC(&work->state, 0, (1.0f));
        /* fall through */
    default:
        ready = 1;
        break;
    }
    return ready;
}

/* Jump to the specified spot, turn and play a timed motion when selected by the action cycle. */
s32 fn_1_CB10(M674PropWork *work)
{
    s32 ready = 0;

    switch (work->state.actionPhase) {
    case 0:
        work->state.actionPhase++;
        work->state.turnState = 0;
        work->state.targetPosition = lbl_1_data_650;
        /* fall through */
    case 1:
        if (fn_1_D5DC(work, (1.0896277f), (1500.0f), 18) == 0) {
            break;
        }
        work->state.actionPhase++;
        work->state.turnState = 0;
        work->state.targetAngle = (330.0f);
        /* fall through */
    case 2:
        if (fn_1_D020(work, (0.2f)) == 0) {
            break;
        }
        work->state.actionPhase++;
        work->state.actionFrame = 0;
        work->state.turnState = 0;
        HuAudFXPlayVolPan(MSM_SE_GUIDE_11, 90,
            (s16)fn_1_36BC(&work->state.position));
        /* fall through */
    case 3:
        fn_1_67CC(&work->state, 2, (1.0f));
        /* fall through */
    case 4:
        if (++work->state.actionFrame < 120) {
            break;
        }
        work->state.actionPhase = -1;
        work->state.velocity.x = (0.0f);
        work->state.velocity.y = (0.0f);
        work->state.velocity.z = (0.0f);
        fn_1_67CC(&work->state, 0, (1.0f));
        /* fall through */
    default:
        ready = 1;
        break;
    }
    return ready;
}

/* Jump onto the wobbling prop and remain on it for the timed action chosen by the cycle. */
s32 fn_1_CCCC(M674PropWork *work)
{
    OMOBJ *candidate = NULL;
    M674PropWork *selectedWork = NULL;
    s32 ready = 0;

    candidate = fn_1_8EF8(0, 2);

    if (candidate != NULL) {
        selectedWork = candidate->data;
        lbl_1_data_65C = selectedWork->state.position;
        lbl_1_data_65C.y += 35.0;
    }

    switch (work->state.actionPhase) {
    case 0:
        work->state.actionPhase++;
        work->state.turnState = 0;
        work->state.targetPosition = lbl_1_data_65C;
        /* fall through */
    case 1:
        if (work->state.velocity.y < 0.0 && work->state.position.y < 120.0 &&
            selectedWork != NULL) {
            selectedWork->actionIndex = 1;
        }
        if (fn_1_D5DC(work, (1.0896277f),
                (2000.0f), 0) == 0) {
            break;
        }
        work->state.actionPhase++;
        work->state.actionFrame = 0;
        /* fall through */
    case 2:
        work->state.position = lbl_1_data_65C;
        if (++work->state.actionFrame < 18) {
            break;
        }
        work->state.actionPhase++;
        work->state.turnState = 0;
        work->state.targetAngle = (30.0f);
        /* fall through */
    case 3:
        work->state.position = lbl_1_data_65C;
        if (fn_1_D020(work, (0.2f)) == 0) {
            break;
        }
        work->state.actionPhase++;
        work->state.actionFrame = 0;
        work->state.turnState = 0;
        /* fall through */
    case 4:
        work->state.position = lbl_1_data_65C;
        fn_1_67CC(&work->state, 1, (1.0f));
        /* fall through */
    case 5:
        work->state.position = lbl_1_data_65C;
        if (++work->state.actionFrame < 120) {
            break;
        }
        work->state.actionPhase = -1;
        work->state.velocity.x = (0.0f);
        work->state.velocity.y = (0.0f);
        work->state.velocity.z = (0.0f);
        fn_1_67CC(&work->state, 0, (1.0f));
        /* fall through */
    default:
        ready = 1;
        break;
    }
    return ready;
}

/* Turn the scripted actor toward its target angle while a jump or timed action waits. */
s32 fn_1_D020(M674PropWork *work, f32 turnFactor)
{
    s32 done = 0;

    switch (work->state.turnState) {
    case 0:
        work->state.turnState++;
        if (fabs((double)fn_1_4CC8(work->state.rotation.y, work->state.targetAngle)) > 30.0) {
            fn_1_67CC(&work->state, 3, (1.0f));
        }
        /* fall through */
    case 1:
        work->state.rotation.y =
            fn_1_4BAC(work->state.rotation.y, work->state.targetAngle, turnFactor);
        if (fabs((double)fn_1_4CC8(work->state.rotation.y, work->state.targetAngle)) > 3.0) {
            break;
        }
        work->state.rotation.y = work->state.targetAngle;
        fn_1_67CC(&work->state, 0, (1.0f));
        /* fall through */
    default:
        done = 1;
        break;
    }
    return done;
}

/* Move the scripted actor toward its target position during the pacing action. */
s32 fn_1_D18C(M674PropWork *work, f32 speed)
{
    f32 deltaX;
    f32 deltaZ;
    f32 distance;
    f32 heading;
    f32 xSpeed;
    f32 zSpeed;
    s32 closeMotionMode = 0;
    s32 travelMotionMode = 3;

    deltaX = work->state.targetPosition.x - work->state.position.x;
    deltaZ = work->state.targetPosition.z - work->state.position.z;
    distance = sqrtf(deltaX * deltaX + deltaZ * deltaZ);

    if (distance < (30.0)) {
        work->state.velocity.x *= 0.8;
        work->state.velocity.z *= 0.8;
        work->state.rotation.y = fn_1_4BAC(work->state.rotation.y, work->state.targetAngle, (0.2f));
        if (fabs((f64)fn_1_4CC8(work->state.rotation.y, work->state.targetAngle)) >
            (3.0)) {
            return 0;
        }
        fn_1_67CC(&work->state, closeMotionMode, (1.0f));
        work->state.rotation.y = work->state.targetAngle;
        work->state.velocity.x = (0.0f);
        work->state.velocity.y = (0.0f);
        work->state.velocity.z = (0.0f);
        return 1;
    }

    heading = (f32)((180.0) *
        (atan2((f64)deltaX, (f64)deltaZ) / (3.141592653589793)));
    work->state.rotation.y = fn_1_4BAC(work->state.rotation.y, heading, (0.3f));
    if (distance < (50.0)) {
        speed *= 0.5;
    }

    xSpeed = (f32)(speed * sin(((3.141592653589793) * (f64)heading) / (180.0)));
    work->state.velocity.x = (f32)((f64)work->state.velocity.x +
        ((0.3) * (f64)(xSpeed - work->state.velocity.x)));
    zSpeed = (f32)(speed * cos(((3.141592653589793) * (f64)heading) / (180.0)));
    work->state.velocity.z = (f32)((f64)work->state.velocity.z +
        ((0.3) * (f64)(zSpeed - work->state.velocity.z)));
    fn_1_67CC(&work->state, travelMotionMode, (1.0f));
    return 0;
}

/* Advance a scripted jump through turning, takeoff, flight and a caller-selected landing delay. */
s32 fn_1_D5DC(M674PropWork *work, f32 gravity, f32 trajectoryY, s32 landingFrames)
{
    Vec effectPosition;
    s32 finished = 0;

    switch (work->state.turnState) {
    case 0:
        work->state.turnState++;
        work->state.turnFrame = 0;
        work->state.partner = NULL;
        work->state.targetAngle = fn_1_4D7C(&work->state.position, &work->state.targetPosition);
        /* fall through */
    case 1:
        work->state.rotation.y = fn_1_4BAC(work->state.rotation.y, work->state.targetAngle, (0.2f));
        if (fabs((f64)fn_1_4CC8(work->state.rotation.y, work->state.targetAngle)) > 3.0) {
            break;
        }
        work->state.rotation.y = work->state.targetAngle;
        work->state.turnState++;
        work->state.turnFrame = 7;
        fn_1_67CC(&work->state, 4, 1.0f);
        /* fall through */
    case 2:
        if (--work->state.turnFrame > 0) {
            break;
        }
        work->state.turnState++;
        work->state.action = 1;
        /* Contact can end flight early, but completion still snaps the actor to targetPosition. */
        work->state.turnFrame = fn_1_4FDC(&work->state.position, &work->state.targetPosition,
                                          gravity, trajectoryY, &work->state.velocity);
        fn_1_4040(MSM_SE_GUIDE_69, &work->state.position);
        /* fall through */
    case 3:
        work->state.velocity.y -= gravity;
        if (work->state.partner == NULL) {
            if (work->state.turnFrame == 7) {
                fn_1_67CC(&work->state, 5, 1.0f);
            }
            if (--work->state.turnFrame > 0) {
                break;
            }
        }
        work->state.turnState++;
        work->state.turnFrame = 0;
        work->state.position = work->state.targetPosition;
        work->state.velocity.x = (0.0f);
        work->state.velocity.y = (0.0f);
        work->state.velocity.z = (0.0f);
        fn_1_67CC(&work->state, 5, 1.0f);
        effectPosition = work->state.position;
        effectPosition.y += 10.0;
        CharEffectDustCreate(-1, 1.0f, &effectPosition);
        /* fall through */
    case 4:
        if (++work->state.turnFrame < landingFrames) {
            break;
        }
        work->state.turnState++;
        work->state.action = 0;
        /* fall through */
    default:
        finished = 1;
        break;
    }
    return finished;
}

/* Apply the actor's queued motion and playback speed at the end of its object update. */
void fn_1_D8CC(OMOBJ *object)
{
    M674PropWork *work = object->data;
    M674ActorState *state = &work->state;

    if (state->requestedMotion != -1) {
        fn_1_8F80(object, state->requestedMotion);
        Hu3DMotionSpeedSet(object->mdlId[0], state->motionSpeed);
    }
}

/* Update the prop's scale effect, transform and motion timer when used as its object callback. */
void fn_1_D9FC(OMOBJ *object)
{
    M674PropWork *initialWork = object->data;
    M674PropWork *workForSetup = initialWork;
    s32 initialModelId = object->mdlId[0];

    fn_1_88C8(&workForSetup->state, initialModelId);
    fn_1_9268(object);
    fn_1_9324(object);
}

/* Expose the actor's introduction flag so the start sequence and scenery can wait for it. */
s32 fn_1_DB40(void)
{
    return lbl_1_bss_4AC;
}

/* Queue a collection animation in the appropriate model pool when a target is collected. */
void fn_1_DB50(s32 collectionType, s32 scoreIndex, Vec *position)
{
    OMOBJ *manager;
    M674PropWork *storage;
    M674ModelEntry *entry;
    s32 entryCount;
    s32 index;

    if (collectionType != 1) {
        manager = lbl_1_bss_4B8;
        entryCount = 32;
    } else {
        manager = lbl_1_bss_4B4;
        entryCount = 4;
    }
    storage = manager->data;
    entry = storage->models;
    for (index = 0; index < entryCount; index++, entry++) {
        if (entry->state == 0) {
            switch (collectionType) {
            case 2:
                entry->state = 2;
                break;
            case 3:
                entry->state = 3;
                break;
            default:
                entry->state = 1;
                break;
            }
            entry->scoreIndex = scoreIndex;
            entry->position = *position;
            return;
        }
    }
}

/* Find the nearest uncollected target within the caller's distance and optional heading limits. */
M674ActorState *fn_1_DC2C(Vec *origin, f32 heading, f32 headingRange, f32 maxDistance)
{
    M674PropWork *candidateWork;
    M674PropWork *target;
    M674ActorState *bestTarget = 0;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 headingOrDistance;
    f32 wrappedAngle;
    f32 bestDistance = 100000.0f;
    s32 index;

    for (index = 0; index < 26; index++) {
        OMOBJ *object = lbl_1_bss_310[index];
        if (object != NULL) {
            candidateWork = object->data;
            if (candidateWork != NULL) {
                target = candidateWork;
                if (target->state.blendRequest == 0) {
                    dx = target->state.position.x - origin->x;
                    dy = target->state.position.y - origin->y;
                    dz = target->state.position.z - origin->z;

                /* A half-turn range accepts every heading. */
                if (headingRange < 180.0) {
                    headingOrDistance = (f32)(180.0 *
                        (atan2((f64)dx, (f64)dz) / 3.141592653589793));
                    wrappedAngle = (f32)fmod(
                        (f64)(headingOrDistance - heading), 360.0);
                    headingOrDistance = wrappedAngle;
                    if (headingOrDistance < 0.0f) {
                        headingOrDistance += 360.0f;
                    }
                    if (headingOrDistance > headingRange &&
                        headingOrDistance < 360.0f - headingRange) {
                        continue;
                    }
                }
                {
                    headingOrDistance = sqrtf(dz * dz + (dx * dx + dy * dy));
                    if (!(headingOrDistance > maxDistance) && headingOrDistance < bestDistance) {
                        bestDistance = headingOrDistance;
                        bestTarget = &target->state;
                    }
                }
                }
            }
        }
    }
    return bestTarget;
}

/* Count uncollected targets so the main game sequence can detect when none remain. */
s32 fn_1_DF88(void)
{
    OMOBJ *object;
    s32 count = 0;
    M674PropWork *work;
    M674PropWork *status;
    s32 index;

    for (index = 0; index < 26; index++) {
        object = lbl_1_bss_310[index];
        if (object != NULL) {
            work = object->data;
            if (work != NULL) {
                status = work;
                if (status->state.blendRequest == 0) {
                    count++;
                }
            }
        }
    }
    return count;
}

Vec lbl_1_data_668 = {0,0,0};
f32 lbl_1_data_674 = 280.0f;
/* Report uncollected targets outside the center radius to the per-frame player manager. */
s32 fn_1_E014(void)
{
    OMOBJ *object;
    M674PropWork *work;
    M674PropWork *status;
    s32 count = 0;
    s32 index;

    for (index = 0; index < 26; index++) {
        object = lbl_1_bss_310[index];
        if (object != NULL) {
            work = object->data;
            if (work != NULL) {
                status = work;
                if (status->state.blendRequest == 0 &&
                    !(fn_1_4E18(&lbl_1_data_668, &status->state.position) < lbl_1_data_674)) {
                    count++;
                }
            }
        }
    }
    return count;
}

/* Read-only storage with no accesses in this module. */
const u8 lbl_1_rodata_558[8] = {0};

OMOBJ *lbl_1_bss_6BC;
OMOBJ *lbl_1_bss_4BC[128];
OMOBJ *lbl_1_bss_4B8;
OMOBJ *lbl_1_bss_4B4;
s32 lbl_1_bss_4B0;
s32 lbl_1_bss_4AC;
M674RegionRecord lbl_1_bss_3E0[3];
u32 lbl_1_bss_378[26];
OMOBJ *lbl_1_bss_310[26];
