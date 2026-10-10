/* Creates and updates the moving props used by Stamp By Me. */
#include "dolphin/math.h"

#include "dolphin/gx.h"

#include "game/main.h"

#include "game/object.h"

#include "game/audio.h"

#include "game/charman.h"

#include "game/gamemes.h"

#include "game/hsfex.h"

#include "game/data.h"

#include "game/gamework.h"

#include "game/memory.h"

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

#include "game/hu3d.h"

#include <string.h>

#include "REL/m649Dll/module_types.h"

#define MSM_SE_M649_PROP_2018 2018
#define MSM_SE_M649_PROP_2019 2019
#define MSM_SE_M649_PROP_2020 2020
#define MSM_SE_M649_PROP_2021 2021
#define MSM_SE_M649_PROP_2022 2022
#define MSM_SE_M649_PROP_2023 2023
#define MSM_SE_M649_PROP_2024 2024
#define MSM_SE_M649_PROP_2025 2025
#define MSM_SE_M649_PROP_2026 2026
#define MSM_SE_M649_PROP_2027 2027

/* Model creation parameters shared by every prop callback. */
typedef struct M649PropMotionParam_s {
    s32 resource;
    float blend;
    u32 attr;
} M649PropMotionParam;

typedef struct M649PropParam_s {
    s32 count, linkModel, type;
    s16 firstId, layer;
    u32 attr;
    s32 shadow, modelResource;
    M649PropMotionParam *motions;
    Vec *position;
    float rotationY;
    OMOBJ_FUNC update;
} M649PropParam;

/* A lane marker stores its world position and whether it has already scored a hit. */
typedef struct M649Point_s {
    Vec position;
    s16 active, hit;
} M649Point;

/* Each lane stores its travel speed and 64 point slots for route targets or stamped markers. */
typedef struct M649Motion_s {
    s32 id, timer;
    float position, speed, targetSpeed;
    s32 mode, group, character, nextPoint;
    M649Point points[64];
} M649Motion;

/* The route timer selects a terminated list of destination indices. */
typedef struct M649PathView_s {
    s32 time;
    s32 points[7];
} M649PathView;

typedef struct M649Scene_s {
    s32 phase, delay, direction;
} M649Scene;

/* Shadow display record registered by the held-item prop. */
typedef struct M649EffectView_s {
    s32 active; /* Nonzero enables the shadow draw. */
    s32 camera; /* Camera index whose shadow hook draws this record. */
    float radius; /* World-space radius of the shadow fan. */
    Vec *position, *offset;
} M649EffectView;

/* Position and motion state shared by prop setup and its frame callbacks. */
typedef struct M649PropState_s {
    s32 camera;
    /* Cleared when the prop is created; no prop callback reads these bytes. */
    u8 unknown_04_to_0F[12];
    Vec position, velocity, rotation;
    /* Cleared when the prop is created; no prop callback reads these bytes. */
    u8 unknown_34_to_43[16];
    s32 motion, motionTime;
    /* Cleared when the prop is created; no prop callback reads these bytes. */
    u8 unknown_4C_to_83[56];
    s32 motionActive, motionTimer;
    /* Cleared when the prop is created; no prop callback reads these bytes. */
    u8 unknown_8C_to_E3[88];
} M649PropState;

/* Prop types retain either a lane's marker list, an effect's live speed, or a route cursor. */
typedef union M649PropMovement_u {
    M649Motion *motion;
    float *speed;
    s32 *path;
} M649PropMovement;

/* Each prop stores its position, model setup, sounds, and current movement state. */
typedef struct M649PropWork_s {
    M649PropState state;
    M649PropParam *param;
    s32 id, timer;
    Vec startPosition;
    s32 phase, pathTime;
    /* Cleared when the prop is created; no prop callback reads these bytes. */
    u8 unknown_104_to_1B3[176];
    M649PropMovement movement;
    s32 leftSound, rightSound;
    s32 childModel;
    M649EffectView effect;
} M649PropWork;

void fn_1_6A88(OMOBJ *obj);

void fn_1_6A9C(HUPROCESS *process, M649PropParam *param);

extern OMOBJ *lbl_1_bss_39C[128];

extern OMOBJ *lbl_1_bss_59C;

extern M649PropParam lbl_1_data_248[];

void fn_1_6A98(OMOBJ *obj);

extern s32 fn_1_1210(s32 resource);

extern s32 fn_1_48DC(s32 resource);

extern s32 fn_1_49E4(s32 model, s32 resource);

void fn_1_6D5C(OMOBJ *object);

void fn_1_7120(OMOBJ *object);

void fn_1_71DC(OMOBJ *object);

extern float fn_1_4CC4(Vec *from, Vec *to);

extern float fn_1_4D60(Vec *from, Vec *to);

extern float fn_1_4AF4(float current, float target, float speed);

extern s32 fn_1_1234(void);

extern s32 lbl_1_bss_398;

extern void fn_1_1664(s16 model, void *sequence, s32 mode);

void fn_1_72FC(OMOBJ *object);

extern s32 fn_1_6958(s32 group);

extern void fn_1_A0BC(M649Motion *motion);

void fn_1_76DC(OMOBJ *object);

extern OMOBJ *lbl_1_bss_368[12], *lbl_1_bss_338[12];

void fn_1_78D0(void);

void fn_1_7CA4(OMOBJ *object);

void fn_1_7DE8(OMOBJ *object);

extern s32 fn_1_1218(s32 camera);

void fn_1_80B0(OMOBJ *object);

void fn_1_8154(OMOBJ *object);

extern void fn_1_A018(M649EffectView *effect);

void fn_1_83BC(OMOBJ *object);

extern void fn_1_A5A4(s32 player, s32 score);

void fn_1_8C2C(s32 type, s32 id, Vec *position, float *speed);

extern void fn_1_A168(s32 type, Vec *position, float *speed, s32 generation);

/* Finds the prop object with the requested type and ID in the manager's object table. */
static inline OMOBJ *m649_local_6DB0_06FAC(s32 type, s32 id)
{
    OMOBJ *object;
    OMOBJ **entry = lbl_1_bss_39C;
    M649PropWork *work;
    s32 i;
    for (i = 0; i < 128; i++, entry++) {
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

_MATH_INLINE float fmodf(float x, float m) { return (float)fmod((double)x, (double)m); }

/* Called by fn_1_83BC when a route starts to select the prop's motion and blend time. */
static inline void m649_local_6E38_07120(OMOBJ *object, s32 index)
{
    M649PropState *state;
    M649PropMotionParam *param;
    M649PropWork *work = object->data;
    s32 model, motion;
    state = &work->state;
    if (state->motion != index && index < 2) {
    motion = object->mtnId[index];
    work->timer = 0;
    state->motionTime = 0;
    if (motion == -1) {
        state->motionActive = 0;
        return;
    }
    param = &work->param->motions[index];
    model = object->mdlId[0];
    if (state->motion < 0 || (0.0) == param->blend) {
        Hu3DMotionSet(model, motion);
        Hu3DModelAttrSet(model, param->attr);
    } else {
        Hu3DMotionShiftSet(model, motion, (0.0f),
            (60.0f) * param->blend, param->attr);
    }
    state->motion = index;
    }
}

/* Applies a prop's current position and rotation to its model each update. */
static inline void m649_local_7120(OMOBJ *object)
{
    M649PropState *state;
    M649PropWork *work = object->data;
    s32 model;
    state = &work->state;
    model = object->mdlId[0];
    state->rotation.y = fmodf(state->rotation.y, 360.0f);
    Hu3DModelPosSetV(model, &state->position);
    Hu3DModelRotSetV(model, &state->rotation);
}

/* Checks whether the model's motion reached its endpoint with no transition pending. */
static inline BOOL m649_local_6F44_07120(OMOBJ *object)
{
    s32 model;
    model = object->mdlId[0];
    if (Hu3DMotionEndCheck(model) != 0 && Hu3DMotionShiftIDGet(model) < 0) {
        return TRUE;
    }
    return FALSE;
}

/* Called by fn_1_83BC while a prop moves; clears its motion state at the motion endpoint. */
static inline void m649_local_71DC_07120(OMOBJ *object)
{
    M649PropState *state;
    M649PropWork *work = object->data;
    state = &work->state;
    if (state->motionActive) {
        if (m649_local_6F44_07120(object)) {
            state->motionActive = 0;
            state->motionTimer = 0;
        }
    } else {
        state->motionTimer = 0;
    }
    work->timer++;
}

/* Finds a registered prop by type and ID. */
static inline OMOBJ *m649_local_6DB0_07300(s32 type, s32 id)
{
    OMOBJ *object;
    OMOBJ **entry = lbl_1_bss_39C;
    M649PropWork *work;
    s32 i;
    for (i = 0; i < 128; i++, entry++) {
        object = *entry;
        if (object != NULL) {
            work = object->data;
            if (work->param->type == type && work->id == id) { return object; }
        }
    }
    return NULL;
}

/* Used by fn_1_7CA4 to find the moving prop that owns this linked prop. */
static inline OMOBJ *m649_local_6DB0_07CA4(s32 type, s32 id)
{
    OMOBJ *object;
    OMOBJ **entry = lbl_1_bss_39C;
    M649PropWork *work;
    s32 i;
    for (i = 0; i < 128; i++, entry++) {
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

/* Used by fn_1_7DE8 to find the moving prop that owns these route props. */
static inline OMOBJ *m649_local_6DB0_07DE8(s32 type, s32 id)
{
    OMOBJ *object;
    OMOBJ **entry = lbl_1_bss_39C;
    M649PropWork *work;
    s32 i;
    for (i = 0; i < 128; i++, entry++) {
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

/* Checks whether the model's motion reached its endpoint with no transition pending. */
static inline BOOL m649_local_6F44_08154(OMOBJ *object)
{
    s32 model = object->mdlId[0];
    if (Hu3DMotionEndCheck(model) && Hu3DMotionShiftIDGet(model) < 0) {
        return TRUE;
    }
    return FALSE;
}

/* Used by fn_1_88FC to find a lane's moving prop. */
static inline OMOBJ *m649_local_6DB0_088FC(s32 type, s32 id)
{
    OMOBJ *object;
    OMOBJ **entry = lbl_1_bss_39C;
    M649PropWork *work;
    s32 i;
    for (i = 0; i < 128; i++, entry++) {
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

/* Used by fn_1_8C2C to find a stamp-effect prop by type and ID;
* the caller checks whether it is idle. */
static inline OMOBJ *m649_local_6DB0_08C2C(s32 type, s32 id)
{
    OMOBJ *object;
    OMOBJ **entry = lbl_1_bss_39C;
    M649PropWork *work;
    s32 i;
    for (i = 0; i < 128; i++, entry++) {
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

/* Runs during minigame setup to clear the prop registry and create its configured props. */
void fn_1_6990(HUPROCESS *manager) {
    OMOBJ *object;
    OMOBJ **objectSlot;
    M649PropParam *param;
    void *managerData;
    s32 objectIndex;
    objectSlot = lbl_1_bss_39C;
    objectIndex = 0;
    while (objectIndex < 128)
    {
        *objectSlot = 0;
        objectIndex += 1;
        objectSlot += 1;
    }
    object = omAddObjEx(manager, 12, 0U, 0U, -1, fn_1_6A88);
    lbl_1_bss_59C = object;
    object->stat |= 0x100;
    managerData = HuMemDirectMallocNum(HEAP_HEAP, 12, HU_MEMNUM_OVL);
    object->data = managerData;
    memset(managerData, 0, 12);
    (*(s32 *)((s8 *)(managerData) + (8))) = (s32)(frand() & 1);
    param = lbl_1_data_248;
    while (param->count != -1)
    {
        fn_1_6A9C(manager, param);
        param += 1;
    }
}

void fn_1_6A84(void)
{
}

void fn_1_6A88(OMOBJ *obj)
{
    obj->objFunc = fn_1_6A98;
}

void fn_1_6A98(OMOBJ *obj)
{
}

/* Called by fn_1_6990 during setup to create a configured prop group and initialize its models. */
void fn_1_6A9C(HUPROCESS *process, M649PropParam *param)
{
    s32 i;
    OMOBJ *object;
    s32 model;
    M649PropWork *work;
    M649PropState *state;
    M649PropMotionParam *motion;
    s32 group;
    s32 resource;
    for (group = 0; group < param->count; group++) {
        object = omAddObjEx(process, 12, 1, 2, 0, param->update);
        fn_1_6D5C(object);
        work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M649PropWork), HU_MEMNUM_OVL);
        object->data = work;
        memset(work, 0, sizeof(M649PropWork));
        object->stat |= 0x100;
        state = &work->state;
        work->param = param;
        work->id = param->firstId + group;
        state->motion = -1;
        for (i = 0; i < 1; i++) {
            object->mdlId[i] = -1;
        }
        for (i = 0; i < 2; i++) {
            object->mtnId[i] = -1;
        }
        if (param->modelResource >= 0) {
            if (param->linkModel != 0) {
                model = fn_1_48DC(fn_1_1210(param->modelResource));
            } else {
                model = Hu3DModelCreate(HuDataSelHeapReadNum(fn_1_1210(param->modelResource),
                                                             HU_MEMNUM_OVL, HEAP_MODEL));
            }
        } else {
            model = -1;
        }
        object->mdlId[0] = model;
        motion = param->motions;
        if (motion != NULL) {
            for (i = 0; i < 2; i++, motion++) {
                resource = motion->resource;
                if (resource == -2) {
                    break;
                }
                if (resource != -1) {
                    object->mtnId[i] = fn_1_49E4(model, fn_1_1210(resource));
                }
            }
        }
        if (param->position != NULL) {
            state->position = *param->position;
        }
        work->startPosition = state->position;
        state->rotation.y = param->rotationY;
        if (model >= 0) {
            Hu3DModelPosSet(model, (0.0f), (0.0f), (0.0f));
            Hu3DModelRotSet(model, (0.0f), (0.0f), (0.0f));
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

/* Called by fn_1_6A9C to register each newly created prop for type-and-ID lookups. */
void fn_1_6D5C(OMOBJ *object) {
    OMOBJ **entry = lbl_1_bss_39C;
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

/* Returns the registered prop object with the requested type and ID, or NULL if absent. */
OMOBJ *fn_1_6DB0(s32 type, s32 id)
{
    OMOBJ *object;
    OMOBJ **entry = lbl_1_bss_39C;
    M649PropWork *work;
    s32 i;
    for (i = 0; i < 128; i++, entry++) {
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

/* Selects a routed prop's motion: missing motions clear the active flag; first or zero-blend
* selections switch immediately, and other selections blend from the current motion. */
void fn_1_6E38(OMOBJ *object, s32 index)
{
    M649PropState *state;
    M649PropMotionParam *param;
    M649PropWork *work = object->data;
    s32 model, motion;
    state = &work->state;
    if (state->motion != index && index < 2) {
    motion = object->mtnId[index];
    work->timer = 0;
    state->motionTime = 0;
    if (motion == -1) {
        state->motionActive = 0;
        return;
    }
    param = &work->param->motions[index];
    model = object->mdlId[0];
    if (state->motion < 0 || (0.0) == param->blend) {
        Hu3DMotionSet(model, motion);
        Hu3DModelAttrSet(model, param->attr);
    } else {
        Hu3DMotionShiftSet(model, motion, (0.0f),
            (60.0f) * param->blend, param->attr);
    }
    state->motion = index;
    }
}

/* Checks the prop model's motion endpoint and whether a motion transition is pending. */
BOOL fn_1_6F44(OMOBJ *actor)
{
    s32 model_id;

    model_id = actor->mdlId[0];
    if (Hu3DMotionEndCheck((HU3D_MODELID) model_id) != 0 &&
        Hu3DMotionShiftIDGet((HU3D_MODELID) model_id) < 0) {
        return TRUE;
    }
    return FALSE;
}

/* Reads a named prop part's transforms with 90 degrees added to yaw; returns whether motion
 * continues even if the part is absent and the outputs were left unchanged. */
BOOL fn_1_6FAC(s32 type, s32 id, char *name, Vec *position, Vec *rotation)
{
    OMOBJ *object;
    M649PropWork *work;
    HSF_OBJECT *part;
    s32 model;
    object = m649_local_6DB0_06FAC(type, id);
    if (object == NULL) { return FALSE; }
    work = object->data;
    model = object->mdlId[0];
    part = Hu3DModelObjPtrGet(model, name);
    if (part != NULL) {
        position->x = part->mesh.curr.pos.x;
        position->y = part->mesh.curr.pos.y;
        position->z = part->mesh.curr.pos.z;
        rotation->x = part->mesh.curr.rot.x;
        rotation->y = (90.0) + part->mesh.curr.rot.y;
        rotation->z = part->mesh.curr.rot.z;
    }
    return !Hu3DMotionEndCheck(model);
}

void fn_1_70E8(OMOBJ *object) {
    fn_1_7120(object);
    fn_1_71DC(object);
}

/* Wraps the prop's stored yaw and copies its position and rotation to the model. */
void fn_1_7120(OMOBJ *object)
{
    M649PropState *state;
    s32 model;
    M649PropWork *work = object->data;
    state = &work->state;
    model = object->mdlId[0];
    state->rotation.y = fmodf(state->rotation.y, 360.0f);
    Hu3DModelPosSetV(model, &state->position);
    Hu3DModelRotSetV(model, &state->rotation);
}

/* Clears completed motion state when no blend remains and advances the prop's update counter. */
void fn_1_71DC(OMOBJ *object)
{
    M649PropState *state;
    M649PropWork *work = object->data;
    s32 done, model;
    state = &work->state;
    if (state->motionActive) {
        model = object->mdlId[0];
        if (Hu3DMotionEndCheck(model) && Hu3DMotionShiftIDGet(model) < 0) {
            done = 1;
        } else {
            done = 0;
        }
        if (done) {
            state->motionActive = 0;
            state->motionTimer = 0;
        }
    } else {
        state->motionTimer = 0;
    }
    work->timer++;
}

void fn_1_7288(void)
{
}

/* Runs during prop setup to select the model's inf_light1 light, then installs its idle
 * callback. */
void fn_1_728C(OMOBJ *object)
{
    M649PropWork *work = object->data;
    s16 model = object->mdlId[0];
    lbl_1_bss_398 = 0;
    fn_1_1664(model, "inf_light1", 1);
    object->objFunc = fn_1_72FC;
}

void fn_1_72FC(OMOBJ *object)
{
}

/* Runs on a moving prop's first update to build route points from its lane and direction. */
void fn_1_7300(OMOBJ *object)
{
    M649Motion *motion;
    M649Point *point;
    s32 i, special, id;
    M649Scene *scene;
    M649PropWork *work;
    s32 count;
    OMOBJ *manager;
    s32 group;
    float z, speed, offset, x, y, step;
    manager = lbl_1_bss_59C;
    scene = manager->data;
    work = object->data;
    id = work->id;
    group = (id >> 2) & 3;
    motion = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M649Motion), HU_MEMNUM_OVL);
    work->movement.motion = motion;
    memset(motion, 0, sizeof(M649Motion));
    if ((id & 1) == 0) {
        x = (-170.0f);
        if (scene->direction == 0) {
            speed = (4.1666665f);
            offset = (1350.0f);
        } else {
            speed = (2.0f);
            offset = (650.0f);
        }
        count = abs((s32)((1800.0f) * speed / (400.0f)));
    } else {
        x = (170.0f);
        if (scene->direction != 0) {
            speed = (-4.1666665f);
            offset = (-1350.0f);
        } else {
            speed = (-2.0f);
            offset = (-650.0f);
        }
        count = abs((s32)((1800.0f) * speed / (400.0f))) + 1;
    }
    motion->id = id;
    motion->position = (0.0f);
    motion->speed = (0.0f);
    motion->targetSpeed = speed;
    motion->group = group;
    motion->character = fn_1_6958(group);
    motion->nextPoint = 0;
    if ((id & 3) >= 2) {
        motion->mode = 0;
        y = (115.0f);
    } else {
        motion->mode = -1;
        y = (105.0f);
    }
    z = (0.0f);
    if (speed > (0.0f)) { step = (400.0f); }
    else { step = (-400.0f); }
    for (i = 0; i < 64; i++) {
        point = &motion->points[i];
        point->position.x = x;
        point->position.y = y;
        point->position.z = z;
        if (i < count) {
            if (motion->mode == -1) {
                motion->nextPoint++;
                point->active = 1;
                special = 0;
                if (i == 0) { z += offset; }
                if ((id & 1) == 0) {
                    if (i == 0) { special = 1; }
                } else {
                    if (i == 0) { special = -1; }
                    if (i == 1) { special = 1; }
                }
                if (i == count - 1) { special = 1; }
                switch (special) {
                case 0:
                    point->position.z = z + (100.0) *
                        ((3.0517578125e-05) * (float)((u16)frand() - 32768));
                    break;
                case 1:
                    point->position.z = z;
                    break;
                }
            }
            /* Marker lanes reach this test without assigning special; only route lanes set it
             * above. */
            if (special != -1) { z += step; }
        }
    }
    fn_1_A0BC(motion);
    object->objFunc = fn_1_76DC;
}

/* Advances a moving lane each frame, starts its sounds, and fades them during FADEOUT. */
void fn_1_76DC(OMOBJ *object)
{
    OMOBJ *manager = lbl_1_bss_59C;
    M649Scene *scene = manager->data;
    M649PropWork *work = object->data;
    M649Motion *motion = work->movement.motion;
    switch (work->phase) {
    case 0:
        if (work->id == 0) { fn_1_78D0(); }
        work->phase++;
        work->leftSound = work->rightSound = -1;
    case 1:
        if (!lbl_1_bss_398) { return; }
        work->phase++;
        motion->timer = 60;
    case 2:
        if (motion->timer == 0) {
            if (work->id == 0) {
                if (scene->direction == 0) {
                    work->leftSound = HuAudFXPlayPan(MSM_SE_M649_PROP_2026, 48);
                    work->rightSound = HuAudFXPlayPan(MSM_SE_M649_PROP_2027, 80);
                } else {
                    work->leftSound = HuAudFXPlayPan(MSM_SE_M649_PROP_2027, 48);
                    work->rightSound = HuAudFXPlayPan(MSM_SE_M649_PROP_2026, 80);
                }
            }
            work->phase++;
        }
    }
    if (motion->timer) {
        motion->timer--;
        motion->speed = (0.0f);
    } else {
        motion->speed += (0.05) * (motion->targetSpeed - motion->speed);
        motion->position += motion->speed;
    }
    if (MgSeqModeGet() == 9 && work->id == 0) {
        if (work->leftSound != -1) {
            HuAudFXFadeOut(work->leftSound, 1000);
            work->leftSound = -1;
        }
        if (work->rightSound != -1) {
            HuAudFXFadeOut(work->rightSound, 1000);
            work->rightSound = -1;
        }
    }
}

/* Called when the first mover starts to space the route points of paired props. */
void fn_1_78D0(void)
{
    OMOBJ *objectA, *objectB;
    M649Point *pointA, *pointB;
    s32 previous;
    M649Motion *motionA, *motionB;
    s32 group;
    M649PropWork *candidateWorkA;
    OMOBJ **entryA;
    OMOBJ *candidateA;
    M649PropWork *candidateWorkB;
    OMOBJ **entryB;
    OMOBJ *candidateB;
    s32 timeA, timeB;
    s32 entryIndexA;
    float speedB, speedA, offsetA, offsetB;
    OMOBJ *objects[2];
    M649PropWork *works[2];
    s32 entryIndexB;

    for (group = 0; group < 4; group++) {
        entryA = lbl_1_bss_39C;
        for (entryIndexA = 0; entryIndexA < 128; entryIndexA++, entryA++) {
            candidateA = *entryA;
            if (candidateA != NULL) {
                candidateWorkA = candidateA->data;
                if (candidateWorkA->param->type == 1 && candidateWorkA->id == group * 4) {
                    objectA = candidateA;
                    goto object_a_found;
                }
            }
        }
        objectA = NULL;
    object_a_found:
        objects[1] = objectA;
        works[1] = objects[1]->data;
        motionA = works[1]->movement.motion;
        pointA = motionA->points;
        speedA = motionA->targetSpeed;
        if ((0.0) == speedA) {
            speedA = (0.001f);
        }
        offsetA = (30.0) * motionA->targetSpeed;

        entryB = lbl_1_bss_39C;
        for (entryIndexB = 0; entryIndexB < 128; entryIndexB++, entryB++) {
            candidateB = *entryB;
            if (candidateB != NULL) {
                candidateWorkB = candidateB->data;
                if (candidateWorkB->param->type == 1 && candidateWorkB->id == group * 4 + 1) {
                    objectB = candidateB;
                    goto object_b_found;
                }
            }
        }
        objectB = NULL;
    object_b_found:
        objects[0] = objectB;
        works[0] = objects[0]->data;
        motionB = works[0]->movement.motion;
        pointB = motionB->points;
        speedB = motionB->targetSpeed;
        if ((0.0) == speedB) {
            speedB = (0.001f);
        }
        offsetB = (30.0) * motionB->targetSpeed;
        previous = abs((s32)(pointB->position.z / speedB));
        pointB++;
        for (;;) {
            if (pointA->active == 0 || pointB->active == 0) {
                break;
            }
            timeA = abs((s32)(pointA->position.z / speedA));
            timeB = abs((s32)(pointB->position.z / speedB));
            if (timeA < timeB) {
                if (timeA < (30.0) + previous) {
                    pointA->position.z = offsetA + previous * speedA;
                }
                /* Retain the original arrival time for the next comparison even if this point is
                 * shifted. */
                previous = timeA;
                pointA++;
            } else {
                if (timeB < (30.0) + previous) {
                    pointB->position.z = offsetB + previous * speedB;
                }
                /* Retain the original arrival time for the next comparison even if this point is
                 * shifted. */
                previous = timeB;
                pointB++;
            }
        }
    }
}

void fn_1_7C94(OMOBJ *obj)
{
    obj->objFunc = fn_1_7CA4;
}

/* Runs each linked-prop update to keep its playback speed and direction with its moving owner. */
void fn_1_7CA4(OMOBJ *object)
{
    M649PropWork *work = object->data;
    M649PropWork *ownerWork;
    M649Motion *motion;
    OMOBJ *owner;
    float speed;
    s32 model = object->mdlId[0];
    owner = m649_local_6DB0_07CA4(1, work->id & 1);
    if (owner != NULL) {
        ownerWork = owner->data;
        motion = ownerWork->movement.motion;
        speed = motion->speed / (2.3583333333333334);
        if (speed >= (0.0)) {
            Hu3DModelAttrReset(model, HU3D_MOTATTR_REV);
            Hu3DMotionSpeedSet(model, speed);
        } else {
            Hu3DModelAttrSet(model, HU3D_MOTATTR_REV);
            Hu3DMotionSpeedSet(model, -speed);
        }
    }
}

void fn_1_7DD8(OMOBJ *obj)
{
    obj->objFunc = fn_1_7DE8;
}

/* Each frame, shows and positions up to three nearby route props for each of four camera lanes. */
void fn_1_7DE8(OMOBJ *object)
{
    M649PropWork *work = object->data;
    M649PropWork *ownerWork;
    OMOBJ *owner;
    M649Motion *motion;
    M649Point *point;
    OMOBJ **slot;
    OMOBJ *prop;
    s32 i, j, count, model;
    float z;
    for (i = 0; i < 4; i++) {
        owner = m649_local_6DB0_07DE8(1, work->id + i * 4);
        if (owner != NULL) {
            ownerWork = owner->data;
            motion = ownerWork->movement.motion;
            count = 0;
            if (work->id == 0) { slot = &lbl_1_bss_368[i * 3]; }
            else { slot = &lbl_1_bss_338[i * 3]; }
            for (j = 0; j < 64; j++) {
                point = &motion->points[j];
                if (point->active) {
                    z = point->position.z - motion->position;
                    if (!(z < (-480.0) || z > (480.0))) {
                        prop = *slot++;
                        model = prop->mdlId[0];
                        Hu3DModelAttrReset(model, 1);
                        Hu3DModelPosSet(model, point->position.x, point->position.y, z);
                        if (++count >= 3) { break; }
                    }
                }
            }
            for (; count < 3; count++) {
                prop = *slot++;
                model = prop->mdlId[0];
                Hu3DModelAttrSet(model, 1);
            }
        }
    }
}

/* Runs during prop setup to assign its camera and register it in the set selected by its type. */
void fn_1_7FD4(OMOBJ *object)
{
    M649PropWork *work = object->data;
    M649PropState *state = &work->state;
    s32 model = object->mdlId[0];
    state->camera = work->id / 3;
    Hu3DModelCameraSet(model, fn_1_1218(state->camera));
    Hu3DModelAttrSet(model, 1);
    if (work->param->type == 4) {
        lbl_1_bss_368[work->id] = object;
    } else {
        lbl_1_bss_338[work->id] = object;
    }
    object->objFunc = fn_1_80B0;
}

void fn_1_80B0(OMOBJ *object)
{
}

/* Runs during prop setup to assign its camera, pause its motion, and install its update
 * callback. */
void fn_1_80B4(OMOBJ *object)
{
    M649PropState *state;
    M649PropWork *work = object->data;
    s32 model;
    state = &work->state;
    model = object->mdlId[0];
    state->camera = work->id / 2;
    Hu3DModelCameraSet(model, fn_1_1218(state->camera));
    Hu3DModelAttrSet(model, 1);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    object->objFunc = fn_1_8154;
}

/* Runs each frame while active, then hides and pauses the prop at its motion endpoint. */
void fn_1_8154(OMOBJ *object)
{
    M649PropState *state;
    M649PropWork *work = object->data;
    s32 model;
    state = &work->state;
    model = object->mdlId[0];
    switch (work->phase) {
    case 0:
        break;
    case 1:
        Hu3DModelAttrReset(model, 1);
        Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
        Hu3DMotionTimeSet(model, (0.0f));
        work->phase++;
    case 2:
        state->position.z -= *work->movement.speed;
        Hu3DModelPosSetV(model, &state->position);
        if (m649_local_6F44_08154(object)) {
            Hu3DModelAttrSet(model, 1);
            Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
            work->phase = 0;
        }
        break;
    }
}

static Vec lbl_1_data_464 = {5.0f, 3.0f, -5.0f};

/* During routed-prop setup, attaches the held model and registers the prop's shadow. */
void fn_1_8278(OMOBJ *object)
{
    M649PropState *state;
    M649EffectView *effect;
    M649PropWork *work = object->data;
    s32 model, cameraMask;
    state = &work->state;
    effect = &work->effect;
    model = object->mdlId[0];
    state->camera = work->id & 3;
    cameraMask = fn_1_1218(state->camera);
    Hu3DModelCameraSet(model, cameraMask);
    work->childModel = fn_1_48DC(fn_1_1210(14));
    Hu3DModelCameraSet(work->childModel, cameraMask);
    Hu3DModelLayerSet(work->childModel, 4);
    Hu3DModelHookSet(model, "itemhook_L", work->childModel);
    Hu3DModelPosSet(work->childModel, (0.0f), (25.0f), (0.0f));
    Hu3DModelAttrSet(model, 1);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    effect->active = 0;
    effect->camera = state->camera;
    effect->radius = (50.0f);
    effect->position = &state->position;
    effect->offset = &lbl_1_data_464;
    fn_1_A018(effect);
    object->objFunc = fn_1_83BC;
}

static Vec lbl_1_data_47C[6] = {
    {-500.0f, 0.0f, -200.0f},
    {-300.0f, 0.0f, -200.0f},
    {-300.0f, 0.0f, 400.0f},
    {500.0f, 0.0f, 50.0f},
    {320.0f, 0.0f, 50.0f},
    {320.0f, 0.0f, -500.0f}
};

static M649PathView lbl_1_data_4C4[3] = {
    {180, {0, 1, 2, -1, 0, 0, 0}},
    {630, {3, 4, 5, -1, 0, 0, 0}},
    {-1, {0, 0, 0, 0, 0, 0, 0}}
};

static s32 lbl_1_data_524[5] = {0, 210, 480, 690};

/* Runs each frame to move a prop along its scheduled route and update its facing and motion
 * state. */
void fn_1_83BC(OMOBJ *object)
{
    M649PropState *state;
    M649PropWork *work;
    s32 *path;
    M649PathView *route;
    s32 model;
    Vec *destination;
    float angle, speed, distance, x, z;
    work = object->data;
    state = &work->state;
    speed = (5.0f);
    model = object->mdlId[0];
    switch (work->phase) {
    case 0:
        work->phase++;
        work->pathTime = lbl_1_data_524[state->camera];
    case 1:
        for (route = lbl_1_data_4C4; route->time >= 0; route++) {
            if (route->time == work->pathTime) {
                work->phase++;
                work->movement.path = route->points;
                break;
            }
        }
        break;
    case 2:
        work->phase++;
        path = work->movement.path;
        state->position = lbl_1_data_47C[*path];
        state->velocity.x = state->velocity.z = (0.0f);
        state->rotation.y = (0.0f);
        path++;
        work->movement.path = path;
        Hu3DModelAttrReset(model, 1);
        Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
        m649_local_6E38_07120(object, 1);
        work->effect.active = 1;
    case 3:
        path = work->movement.path;
        destination = &lbl_1_data_47C[*path];
        angle = fn_1_4CC4(&state->position, destination);
        distance = fn_1_4D60(&state->position, destination);
        state->rotation.y = fn_1_4AF4(state->rotation.y, angle, (0.1f));
        x = speed * sin((3.141592653589793) * angle / (180.0));
        z = speed * cos((3.141592653589793) * angle / (180.0));
        state->velocity.x += (0.2) * (x - state->velocity.x);
        state->velocity.z += (0.2) * (z - state->velocity.z);
        VECAdd(&state->position, &state->velocity, &state->position);
        if (distance > (20.0)) {
            m649_local_7120(object);
            m649_local_71DC_07120(object);
            break;
        }
        path++;
        if (*path >= 0) {
            work->movement.path = path;
        } else {
            work->phase = 1;
            Hu3DModelAttrSet(model, 1);
            Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
            work->effect.active = 0;
        }
        break;
    }
    if (!fn_1_1234()) {
        if (++work->pathTime > 900) {
            work->pathTime = 0;
        }
    }
}

static const s32 lbl_1_rodata_2C8[4] = {
    MSM_SE_M649_PROP_2018, MSM_SE_M649_PROP_2019, MSM_SE_M649_PROP_2020, MSM_SE_M649_PROP_2021};
static const s32 lbl_1_rodata_2D8[4] = {
    MSM_SE_M649_PROP_2022, MSM_SE_M649_PROP_2023, MSM_SE_M649_PROP_2024, MSM_SE_M649_PROP_2025};

/* Called by a player stamp to add a marker, check its route lane, and award a hit point.
* Clears startup delays on the found lanes; group 0 also releases every lane's startup wait. */
void fn_1_88FC(s32 group, s32 id)
{
    M649Point *point;
    M649Motion *motion;
    OMOBJ *object;
    M649PropWork *work;
    s32 i;
    Vec position;
    s32 objectId = (4 * group + 2) + id;
    float *speed;
    s32 effect = 0, score = 0;
    float difference, z;
    if (lbl_1_bss_398 || group == 0) {
        HuAudFXPlay(lbl_1_rodata_2C8[group]);
    }
    object = m649_local_6DB0_088FC(1, objectId);
    if (object != NULL) {
        work = object->data;
        motion = work->movement.motion;
        motion->timer = 0;
        if (motion->nextPoint >= 64) {
            motion->nextPoint = 0;
        }
        point = &motion->points[motion->nextPoint++];
        point->active = 1;
        z = motion->position;
        point->position.z = z;
        position.x = point->position.x;
        position.y = point->position.y;
        position.z = (0.0f);
        speed = &motion->speed;
        object = m649_local_6DB0_088FC(1, objectId - 2);
        if (object != NULL) {
            work = object->data;
            motion = work->movement.motion;
            motion->timer = 0;
            for (i = 0; i < 64; i++) {
                point = &motion->points[i];
                if (point->active && point->hit == 0) {
                    difference = point->position.z - z;
                    if (difference > (-35.0) && difference < (35.0)) {
                        effect = 1;
                        score = 1;
                        point->hit = 1;
                        if (lbl_1_bss_398 || group == 0) {
                            HuAudFXPlay(lbl_1_rodata_2D8[group]);
                        }
                    }
                    if (score != 0) {
                        fn_1_A5A4(group, score);
                        break;
                    }
                }
            }
        }
        fn_1_8C2C(effect, group, &position, speed);
        if (effect == 1) {
            fn_1_A168(group, &position, speed, 6);
        }
    }
    if (group == 0) {
        lbl_1_bss_398 = 1;
    }
}

/* Called after a stamp to activate a free prop of the selected miss or hit effect type
* for its lane. */
void fn_1_8C2C(s32 type, s32 id, Vec *position, float *speed)
{
    M649PropWork *work;
    OMOBJ *object;
    s32 objectId, i;
    s32 propType = type + 6;
    objectId = id * 2;
    for (i = 0; i < 2; i++, objectId++) {
        object = m649_local_6DB0_08C2C(propType, objectId);
        if (object != NULL) {
            work = object->data;
            if (work->phase == 0) {
                work->phase++;
                work->state.position = *position;
                work->movement.speed = speed;
                break;
            }
        }
    }
}

/* Computer timing skips points passed within the look-ahead and returns arrival time from the
 * current lane position; returns 0 for a missing/stopped lane and -600 if no point remains. */
s32 fn_1_8D20(s32 group, s32 id, s32 frames)
{
    float futurePosition;
    M649PropWork * candidateWork;
    float pointPosition;
    OMOBJ * * entry;
    OMOBJ * candidate;
    OMOBJ * object;
    M649PropWork * work;
    OMOBJ * foundObject;
    M649Motion * motion;
    M649Point * point;
    s32 entryIndex;
    s32 objectId;
    s32 pointIndex;
    float remainingTime;
    objectId = id + group * 4;
    entry = lbl_1_bss_39C;
    entryIndex = 0;
    for (; entryIndex < 128; entryIndex++, entry++)
    {
        candidate = * entry;
        if (candidate != NULL)
        {
            candidateWork = candidate->data;
            if (candidateWork->param->type == 1 && candidateWork->id == objectId)
            {
                foundObject = candidate;
                goto object_found;
            }
        }
    }
    foundObject = NULL;
object_found:
    object = foundObject;
    if (object != NULL)
    {
        work = object->data;
        motion = work->movement.motion;
        futurePosition = motion->position + motion->speed * frames;
        if ((0.0) == motion->speed)
        {
            return 0;
        }
        if (motion->speed < (0.0))
        {
            for (pointIndex = 0; pointIndex < 64; pointIndex++)
            {
                point = & motion->points[pointIndex];
                if (point->active != 0)
                {
                    pointPosition = point->position.z;
                    if (pointPosition < futurePosition)
                    {
                        break;
                    }
                }
            }
        }
        else
        {
            for (pointIndex = 0; pointIndex < 64; pointIndex++)
            {
                point = & motion->points[pointIndex];
                if (point->active != 0)
                {
                    pointPosition = point->position.z;
                    if (pointPosition > futurePosition)
                    {
                        break;
                    }
                }
            }
        }
        if (pointIndex >= 64)
        {
            return - 600;
        }
        remainingTime = (f32) fabs((pointPosition - motion->position) / motion->speed);
        return (0.5) + remainingTime;
    }
    return 0;
}
