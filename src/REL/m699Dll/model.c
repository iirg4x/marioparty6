/* Dunk Bros. scene models, prop motion and target tracking. */
#include "math.h"

#include "dolphin/mtx.h"

#include "dolphin/types.h"

#include "dolphin/gx.h"

#include "game/object.h"

#include "game/hu3d.h"

#include "game/memory.h"

#include "game/data.h"

#include "REL/m699Dll/m699_callback_state.h"

#include "game/hsfex.h"

#include "game/audio.h"

#include "game/mg/seqman.h"

#include "game/frand.h"

#include "REL/m699Dll/m699_natural_views.h"

#include "game/main.h"

#include "game/charman.h"

#include "game/gamemes.h"

#include "game/gamework.h"

#include "game/mg/timer.h"

#include "game/mg/score.h"

#include "game/pad.h"

#include "game/sprite.h"

#include "game/wipe.h"

#include "game/mg/actman.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "datadir_enum.h"


#include <string.h>

typedef struct M699MotionParam_s {
    s32 resource;
    f32 blend;
    u32 attributes;
} M699MotionParam;

typedef struct M699InitialMotionRecord {
    f32 boundary;
    f32 height;
    f32 movementScale;
    f32 verticalSpeed;
    f32 acceleration;
    f32 motionParameter;
    f32 modelScale;
    f32 dustEffect;
    f32 parameters[3];
    f32 scales[7];
    s32 closeMotion;
    s32 slowMotion;
    f32 closeMotionSpeed;
    s32 farMotion;
    f32 farMotionSpeed;
    s32 firstTarget;
    s32 secondTarget;
    f32 otherStartSpeed;
    s32 thirdTarget;
    f32 otherEndSpeed;
    s32 motionIndices[11];
} M699InitialMotionRecord;

typedef struct M699PropParam_s {
    s32 count, linkModel, type;
    s16 firstId, layer;
    u32 attr;
    s32 shadow, modelResource;
    M699MotionParam *motions;
    Vec *position;
    f32 rotationY;
    void (*update)();
} M699PropParam;

typedef struct M699CameraState_s {
    s32 first, second, model;
} M699CameraState;

extern OMOBJ *lbl_1_bss_7B8[128];

extern OMOBJ *lbl_1_bss_9B8;

extern OMOBJ *lbl_1_bss_4C0;

M699InitialMotionRecord lbl_1_data_688[2] = {
    {70.0f, 180.0f, 13.333333f, 26.666666f, -2.5f, 11.666667f,
     1.2f, 1.5f, {0.45f, 1.3f, 0.9f},
     {-48.0f, 72.0f, 55.0f, 0.0f, 80.0f, 80.0f, 100.0f},
     0, 4, 2.0f, 5, 2.0f, 8, 9, 2.0f, 10, 1.5f,
     {6, 7, 17, 16, 15, 11, 12, 13, 14, 18, 7}},
    {100.0f, 250.0f, 9.166667f, 36.666668f, -4.1666665f, 26.666666f,
     1.5f, 1.8f, {0.4f, 1.35f, 0.8f},
     {-35.0f, 110.0f, 50.0f, 30.0f, 80.0f, 80.0f, 100.0f},
     0, 4, 3.0f, 5, 1.5f, 8, 9, 3.0f, 10, 1.0f,
     {6, 7, 17, 16, 15, 11, 12, 13, 14, 18, 7}}
};

M699MotionParam lbl_1_data_7C0[20] = {
    { -1, 0.0f, 0 }, { 18, 0.0f, 0 }, { 19, 0.0f, 0 },
    { 20, 0.0f, 0 }, { 21, 0.0f, 0 }, { -2, 0.0f, 0 }
};

M699MotionParam lbl_1_data_8B0[20] = {
    { 2, 0.2f, 0x40000001 }, { 5, 0.2f, 0 }, { -1, 0.0f, 0 },
    { -1, 0.0f, 0 }, { 3, 0.2f, 0x40000001 },
    { 4, 0.2f, 0x40000001 }, { -2, 0.0f, 0 }
};

M699MotionParam lbl_1_data_9A0[20] = {
    { 7, 0.2f, 0x40000001 }, { 8, 0.2f, 0x40000001 }, { -2, 0.0f, 0 }
};

M699MotionParam lbl_1_data_A90[20] = {
    { 10, 0.2f, 0x40000001 }, { 11, 0.2f, 0x40000001 }, { -2, 0.0f, 0 }
};

/* Unidentified initialized bytes; no code names an element. */
u8 lbl_1_data_B80[36] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0xC3, 0xFA, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0x43, 0xFA, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

Vec lbl_1_data_BA4 = { 0.0f, 0.0f, -300.0f };

Vec lbl_1_data_BB0 = { 500.0f, 100.0f, 1000.0f };

void fn_1_10A58(OMOBJ *object);

void fn_1_10EA4(OMOBJ *object);

void fn_1_105C8(OMOBJ *object);

void fn_1_105CC(OMOBJ *object);

struct M699MotionInitController;

void fn_1_114A0(struct M699MotionInitController *controller);

void fn_1_12574(OMOBJ *object);

void fn_1_138FC(OMOBJ *object);

M699PropParam lbl_1_data_BBC[10] = {
    { 1, 0, 1, 0, 1, ((1 << 30) | (1 << 1)), 0, 0xF, NULL, NULL, 0.0f, fn_1_10A58 },
    { 1, 0, 1, 1, 1, ((1 << 30) | (1 << 1)), 0, 0x10, NULL, NULL, 0.0f, fn_1_10A58 },
    { 1, 0, 0, 0, 2, ((1 << 30) | (1 << 0)), 2, 0xD, NULL, NULL, 0.0f, fn_1_10EA4 },
    { 1, 0, 0, 1, 3, ((1 << 30) | (1 << 0)), 2, 0xE, NULL, NULL, 0.0f, fn_1_105C8 },
    { 1, 0, 0, 2, 1, 0, 0, 0x11, lbl_1_data_7C0, NULL, 0.0f, fn_1_105CC },
    { 1, 0, 2, 0, 5, 0, 1, 0x16, NULL, &lbl_1_data_BB0, 0.0f, fn_1_114A0 },
    { 1, 0, 4, 0, 4, 0, 1, 1, lbl_1_data_8B0, &lbl_1_data_BA4, 0.0f, fn_1_12574 },
    { 0xA, 1, 5, 0, 4, 0, 0, 6, lbl_1_data_9A0, NULL, 0.0f, fn_1_138FC },
    { 4, 0, 6, 0, 4, 0, 0, 9, lbl_1_data_A90, NULL, 0.0f, fn_1_138FC },
    { -1, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, 0.0f, NULL },
};

s32 lbl_1_data_D74[4] = { 1, 2, 3, 4 };

extern void fn_1_FE70(OMOBJ *);

extern void fn_1_FED4(HUPROCESS *, M699PropParam *);

extern s32 fn_1_10B8(s32 resource);

OMOBJ *fn_1_10190(s32 type, s32 id);

void fn_1_FD3C(HUPROCESS *process)
{
    OMOBJ **entry;
    s32 index;
    OMOBJ *object;
    M699CameraState *state;
    M699PropParam *param;

    entry = lbl_1_bss_7B8;
    index = 0;
    while (index < 0x80) {
        *entry = NULL;
        index++;
        entry++;
    }
    object = fn_8003AB38(process, 0xC, 0, 0, -1, fn_1_FE70);
    lbl_1_bss_9B8 = object;
    object->stat |= 0x100;
    state = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M699CameraState), 0x10000000);
    object->data = state;
    memset(state, 0, sizeof(M699CameraState));
    state->model = (s16)fn_80025A04(fn_80008DD0(fn_1_10B8(0x19), 0x10000000, 2));
    fn_80026B8C((s16)state->model, 4);
    param = lbl_1_data_BBC;
    while (param->count != -1) {
        fn_1_FED4(process, param);
        param++;
    }
    lbl_1_bss_4C0 = fn_1_10190(2, 0);
}


void fn_1_FE6C(void) {
}

extern void fn_1_FEA4(OMOBJ *obj);

/* Reset the object state and timer, then install the timer callback. */
void fn_1_FE70(OMOBJ *obj) {
    M699CallbackStateFields *state = obj->data;
    state->value = 0;
    state->timer = 0;
    obj->objFunc = fn_1_FEA4;
}


/* Decrement the object timer while it is nonzero. */
void fn_1_FEA4(OMOBJ *obj) {
    M699CallbackStateFields *state = obj->data;
    if (state->timer != 0) {
        state->timer -= 1;
    }
}


typedef struct M699PropState_s {
    u8 unk_0[0x7C]; /* No code names this field. */
    Vec position;
    u8 unk_88[0x10]; /* No code names this field. */
    f32 rotationY;
    u8 unk_9C[0x14]; /* No code names this field. */
    s32 motion;
} M699PropState;

typedef struct M699PropWork_s {
    M699PropState state;
    u8 unk_B4[0x1D4]; /* No code names this field. */
    M699PropParam *param;
    s32 id;
    u8 unk_290[4]; /* No code names this field. */
    Vec startPosition;
} M699PropWork;

extern void fn_1_1013C(OMOBJ *object);

extern s32 fn_1_4CB0();

extern s32 fn_1_4DB8(s32 model, s32 resource);

void fn_1_FED4(HUPROCESS *process, M699PropParam *param)
{
    s32 group;
    s32 index;
    s32 model;
    s32 resource;
    OMOBJ *object;
    M699PropWork *work;
    M699PropState *state;
    M699MotionParam *motion;

    for (group = 0; group < param->count; group++) {
        object = fn_8003AB38(process, 12, 1, 0x13, 0, param->update);
        fn_1_1013C(object);
        work = HuMemDirectMallocNum(HEAP_HEAP, 0x39C, 0x10000000);
        object->data = work;
        memset(work, 0, 0x39C);
        object->stat |= 0x100;
        state = &work->state;
        work->param = param;
        work->id = param->firstId + group;
        state->motion = -1;
        for (index = 0; index < 1; index++) {
            object->mdlId[index] = -1;
        }
        for (index = 0; index < 0x13; index++) {
            object->mtnId[index] = -1;
        }
        if (param->modelResource >= 0) {
            if (param->linkModel != 0) {
                fn_1_10B8(param->modelResource);
                model = fn_1_4CB0();
            } else {
                model = fn_80025A04(fn_80008DD0(fn_1_10B8(param->modelResource),
                                                             0x10000000, 2));
            }
        } else {
            model = -1;
        }
        object->mdlId[0] = model;
        motion = param->motions;
        if (motion != NULL) {
            for (index = 0; index < 0x13; index++, motion++) {
                resource = motion->resource;
                if (resource == -2) {
                    break;
                }
                if (resource != -1) {
                    object->mtnId[index] = fn_1_4DB8(model, fn_1_10B8(resource));
                }
            }
        }
        if (param->position != NULL) {
            state->position = *param->position;
        }
        work->startPosition = state->position;
        state->rotationY = param->rotationY;
        if (model >= 0) {
            fn_80026B8C(model, param->layer);
            fn_800269E8(model, param->attr);
            switch (param->shadow) {
            case 1:
                fn_80026E44(model);
                break;
            case 2:
                fn_80026FFC(model);
                break;
            }
        }
    }
}


/* Store the object in the first free scene-object slot. */
void fn_1_1013C(OMOBJ *object) {
    OMOBJ **entry = lbl_1_bss_7B8;
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


typedef struct M699PropParamView {
    s32 unknown_00;
    s32 unknown_04;
    s32 type;
} M699PropParamView;

typedef struct M699PropWorkView {
    u8 unk_0[0x288]; /* No code names this field. */
    M699PropParamView *param;
    s32 id;
} M699PropWorkView;

OMOBJ *fn_1_10190(s32 type, s32 id)
{
    OMOBJ *object;
    OMOBJ **entry = lbl_1_bss_7B8;
    M699PropWorkView *work;
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

typedef struct M699MotionParamView_s {
    u8 unk_0[4]; /* No code names this field. */
    f32 blend;
    u32 attributes;
} M699MotionParamView;

typedef struct M699MotionParamOwner_s {
    u8 unk_0[0x1C]; /* No code names this field. */
    M699MotionParamView *motion;
} M699MotionParamOwner;

typedef struct M699MotionControlWork_s {
    u8 unk_0[0x7C]; /* No code names this field. */
    Vec position;
    Vec velocity;
    f32 turnAngle;
    f32 rotationY;
    f32 rotationZ;
    u8 unk_A0[0x10]; /* No code names this field. */
    s32 motionState;
    s32 motionTime;
    u8 unk_B8[0x6C]; /* No code names this field. */
    s32 motionActive;
    u8 unk_128[0x160]; /* No code names this field. */
    M699MotionParamOwner *paramOwner;
    u8 unk_28C[4]; /* No code names this field. */
    s32 timer;
    u8 unk_294[0xC]; /* No code names this field. */
    u32 sequenceState;
    u8 unk_2A4[4]; /* No code names this field. */
    s32 sequenceTimer;
} M699MotionControlWork;


void fn_1_10218(OMOBJ *object, s32 motionIndex)
{
    M699MotionControlWork *work = object->data;
    M699MotionControlWork *state = work;
    M699MotionParamView *param;
    u32 attributes;
    s32 modelId;
    s32 motionId;

    if (motionIndex == -1) {
        state->motionState = -1;
        state->motionActive = 0;
    } else if (state->motionState != motionIndex && motionIndex < 0x13) {
        motionId = object->mtnId[motionIndex];
        work->timer = 0;
        state->motionTime = 0;
        if (motionId == -1) {
            state->motionState = -1;
            state->motionActive = 0;
        } else {
            param = &work->paramOwner->motion[motionIndex];
            attributes = param->attributes;
            modelId = object->mdlId[0];
            if (state->motionState < 0 || 0.0 == param->blend) {
                fn_8002C95C(modelId, motionId);
                fn_800269E8(modelId, attributes);
            } else {
                fn_8002CCC0(modelId, motionId, 0.0f,
                                   60.0f * param->blend, attributes);
            }
            if ((attributes & ((1 << 30) | (1 << 2))) != 0) {
                fn_8002D414(modelId, Hu3DMotionMotionMaxTimeGet(motionId));
            }
            state->motionState = motionIndex;
        }
    }
}


typedef struct M699MotionWork_s {
    u8 unk_0[0x7C]; /* No code names this field. */
    Vec position;
    u8 unk_88[0xC]; /* No code names this field. */
    Vec rotation;
    u8 unk_A0[0x10]; /* No code names this field. */
    s32 motionState;
    u8 unk_B4[0x70]; /* No code names this field. */
    s32 motionEnabled;
    u8 unk_128[0x168]; /* No code names this field. */
    s32 updateCount;
} M699MotionWork;


s32 fn_1_10370(OMOBJ *object)
{
    M699MotionWork *work = object->data;
    M699MotionWork *state = work;
    s32 modelId;

    modelId = object->mdlId[0];
    if (state->motionState == -1) {
        return 1;
    }
    if (fn_8002D75C((s16)modelId) != 0 &&
        fn_8002D3EC((s16)modelId) < 0) {
        return 1;
    }
    return 0;
}

void fn_1_10404(OMOBJ *object)
{
    M699MotionWork *work = object->data;
    M699MotionWork *currentWork = work;
    s32 modelId;

    modelId = object->mdlId[0];
    currentWork->rotation.y = fmodf(currentWork->rotation.y, 360.0f);
    currentWork->rotation.x = currentWork->rotation.z = 0.0f;
    fn_80026790(modelId, &currentWork->position);
    fn_80026838(modelId, &currentWork->rotation);
    if (currentWork->motionEnabled != 0 && fn_1_10370(object) != 0) {
        currentWork->motionEnabled = 0;
    }
    work->updateCount += 1;
}


typedef struct M699ModelWork_s {
    u32 unknown_00;
    u32 unknown_04;
    s32 modelId;
} M699ModelWork;

void fn_1_10540(char *name, Vec *position, float *rotationY)
{
    OMOBJ *object = lbl_1_bss_9B8;
    M699ModelWork *work = object->data;
    HSF_OBJECT *modelObject;

    modelObject = fn_80026BF4((s16)work->modelId, name);
    if (modelObject != NULL) {
        position->x = modelObject->mesh.base.pos.x;
        position->y = modelObject->mesh.base.pos.y;
        position->z = modelObject->mesh.base.pos.z;
        *rotationY = modelObject->mesh.base.rot.y;
    }
}


void fn_1_105C8(OMOBJ *object) {
}


typedef struct M699MotionRequestParam {
    u32 unknown_000;
    f32 blendDuration;
    u32 attributes;
} M699MotionRequestParam;

typedef struct M699MotionSequence {
    u8 unk_0[8]; /* No code names this field. */
    s32 mode;
    u8 unk_C[0x10]; /* No code names this field. */
    M699MotionRequestParam *owner;
} M699MotionSequence;

typedef struct M699MotionUpdateWork {
    u8 unk_0[0x7C]; /* No code names this field. */
    Vec position;
    u8 unk_88[0x10]; /* No code names this field. */
    f32 angle;
    u8 unk_9C[0x14]; /* No code names this field. */
    s32 motionState;
    s32 motionTime;
    u8 unk_B8[0x6C]; /* No code names this field. */
    s32 motionActive;
    u8 unk_128[0x160]; /* No code names this field. */
    M699MotionSequence *sequence;
    s32 dataIndex;
    s32 timer;
} M699MotionUpdateWork;

typedef struct M699PropMotionWork {
    u8 unk_0[0xB0]; /* No code names this field. */
    s32 motionState;
    u8 unk_B4[0x1FC]; /* No code names this field. */
    s32 modelState;
    u8 unk_2B4[0xA8]; /* No code names this field. */
    s32 animationState;
    f32 animationFrame;
} M699PropMotionWork;


extern void fn_1_10648(OMOBJ *object);

extern s32 fn_1_39D8(Vec *position);

void fn_1_105CC(OMOBJ *object)
{
    M699PropMotionWork *work = object->data;
    s16 modelId = object->mdlId[0];

    fn_800269E8((s32)modelId, (1 << 14));
    work->modelState = 0;
    work->animationState = -1;
    work->animationFrame = 0.0f;
    object->objFunc = fn_1_10648;
}

void fn_1_10648(OMOBJ *object)
{
    M699PropMotionWork *work;
    M699PropMotionWork *motionWork;
    s32 motionCount;
    s32 motionIndex;
    s32 modelId;
    s32 soundPan;
    s32 maximumVolume;
    s32 currentVolume;
    s32 soundActive;
    HSF_OBJECT *bus;
    Point3d position;
    Point3d screenPosition;

    work = object->data;
    motionWork = work;
    motionCount = 4;
    modelId = object->mdlId[0];

    if (motionWork->motionState != -1) {
        if (fn_1_10370(object) != 0) {
            motionWork->motionState = -1;
        }
    }
    if (motionWork->motionState == -1) {
        motionIndex = work->modelState++;

        if (work->modelState >= motionCount) {
            work->modelState = 0;
        }
        fn_1_10218(object, lbl_1_data_D74[motionIndex]);
        fn_8002D86C(modelId, 0.5f);
    }

    soundActive = 0;
    soundPan = 0x40;
    currentVolume = maximumVolume = 0x60;
    bus = fn_80026BF4(modelId, "bus");

    if (bus != NULL) {
        position.x = bus->mesh.curr.pos.x;
        position.y = bus->mesh.curr.pos.y;
        position.z = bus->mesh.curr.pos.z;
        soundPan = fn_1_39D8(&position);
        if (position.y > -100.0) {
            Hu3D3Dto2D(&position, 1, &screenPosition);
            if (screenPosition.x >= -150.0 && screenPosition.x <= 726.0) {
                soundActive = 1;
            }
        }
    }
    if (soundActive != 0) {
        work->animationFrame = (f32)((f64)work->animationFrame +
            (0.05 * (f64)(maximumVolume - work->animationFrame)));
    } else {
        work->animationFrame = (f32)((f64)work->animationFrame +
            (0.05 * -(f64)work->animationFrame));
    }
    currentVolume = (s32)work->animationFrame;
    if (currentVolume <= 1) {
        currentVolume = 0;
    }
    if (currentVolume > maximumVolume) {
        currentVolume = maximumVolume;
    }
    if (work->animationState == -1) {
        if (currentVolume != 0) {
            work->animationState =
                HuAudFXPlayVolPan(0x926, (s16)currentVolume, (s16)soundPan);
        }
    } else {
        HuAudFXPanning(work->animationState, (s16)soundPan);
        HuAudFXVolSet(work->animationState, (s16)currentVolume);
    }
}


typedef struct M699MatrixWork_s {
    u8 unk_0[0x28C]; /* No code names this field. */
    s32 matrixIndex;
} M699MatrixWork;

extern Mtx lbl_1_bss_9BC[2];

extern Vec lbl_1_bss_A1C[2];


typedef struct M699SpawnConfig_s {
    u32 values[3];
    s32 spawnCharacter;
    u32 parameters[5];
    Vec *spawnPosition;
    f32 floatParameters[2];
    s32 collision;
} M699SpawnConfig;

typedef struct M699SpawnTemplate_s {
    M699SpawnConfig config;
    s32 trailingValue;
} M699SpawnTemplate;

typedef struct M699SpawnWork_s {
    M699SpawnConfig spawn;
    u8 unk_34[0x258]; /* No code names this field. */
    s32 character;
    u8 unk_290[0x20]; /* No code names this field. */
    s32 cooldown;
    u8 unk_2B4[0x0C]; /* No code names this field. */
    Vec position;
} M699SpawnWork;

extern void fn_1_18300(u32 character);

extern void fn_1_10B88(OMOBJ *object);

extern void fn_1_10E10(OMOBJ *object);

extern void fn_1_10E0(s32 state);


extern s32 fn_1_14EF0(void);

extern void fn_1_14F38(s32 player);

extern s32 fn_1_14FC8(void);

Vec *fn_1_14778(s32 index);

extern void fn_1_18038(Vec *position);

extern void fn_1_14EB0(s32 mode);

void *fn_1_14ACC(Vec *position, Vec *rotation);

extern void fn_1_75C(f32 x, f32 y, f32 z, f32 w);

typedef struct M699ParentTimerWork_s {
    u8 unk_0[4]; /* No code names this field. */
    s32 timer;
} M699ParentTimerWork;

const M699SpawnTemplate lbl_1_rodata_568 = {
    { { 1, 8, 5 }, 0, { 14, 0, 0, 0, 0 }, NULL, { 42.0f, 5.0f }, 0 }, 0
};

void fn_1_10A58(OMOBJ *object)
{
    M699SpawnWork *work = object->data;
    M699SpawnWork *model = work;

    fn_1_10E10(object);
    fn_1_18300(work->character);
    work->position = lbl_1_bss_A1C[work->character];
    work->position.x = 0.0f;
    model->spawn = lbl_1_rodata_568.config;
    model->spawn.spawnCharacter = work->character;
    model->spawn.spawnPosition = &lbl_1_bss_A1C[work->character];
    fn_1_1D734(model);
    object->objFunc = fn_1_10B88;
}

void fn_1_10B88(OMOBJ *object)
{
    M699SpawnWork *work;
    M699ParentTimerWork *parentWork;
    M699SpawnWork *spawn;
    Vec queryPosition;
    Vec relatedPosition;
    s32 player;
    s32 points;
    f32 targetSpeed;
    f32 speedLimit;
    f64 speedLimitDouble;

    parentWork = lbl_1_bss_9B8->data;
    work = object->data;
    spawn = work;
    if (work->cooldown == 0) {
        if (spawn->spawn.collision != 0) {
            player = work->character ^ 1;
            points = fn_1_14EF0();
            fn_1_1B5A0(player, points);
            fn_1_14F38(player);
            if (fn_1_14FC8() == 0) {
                if (points == 1) {
                    fn_1_10E0(3);
                } else {
                    fn_1_10E0(4);
                }
            } else {
                relatedPosition = *fn_1_14778(work->character);
                fn_1_18038(&relatedPosition);
                fn_1_10E0(5);
                HuAudFXPlay(0x925);
            }
            fn_1_14EB0(2);
            parentWork->timer = 0xB4;
            work->cooldown = 0x3C;
        }
    } else {
        work->cooldown--;
    }

    fn_1_14ACC(&queryPosition, NULL);
    targetSpeed = lbl_1_bss_A1C[work->character].x;
    if (fn_80071A8C() == 5) {
        if (0.0f == work->position.x) {
            speedLimitDouble = 200.0;
        } else {
            speedLimitDouble = 300.0;
        }
        speedLimit = (f32)speedLimitDouble;
        if (targetSpeed < 0.0f) {
            if (queryPosition.x > speedLimit) {
                targetSpeed = 0.0f;
            }
        } else if (queryPosition.x < -speedLimit) {
            targetSpeed = 0.0f;
        }
    }
    if (work->position.x < targetSpeed) {
        work->position.x += 15.0;
        if (work->position.x > targetSpeed) {
            work->position.x = targetSpeed;
        }
    }
    if (work->position.x > targetSpeed) {
        work->position.x -= 15.0;
        if (work->position.x < targetSpeed) {
            work->position.x = targetSpeed;
        }
    }
    fn_1_75C(work->position.x, work->position.y, work->position.z,
             100.0f);
    fn_1_10E10(object);
}

char lbl_1_data_D88[16] = "null_GL\0null_GR";

void fn_1_10E10(OMOBJ *object)
{
    M699MatrixWork *work = object->data;
    s32 index = work->matrixIndex;
    s16 modelId = object->mdlId[0];
    Mtx *matrix = &lbl_1_bss_9BC[index];
    Vec *position;

    fn_80023398((s32)modelId, lbl_1_data_D88 + index * 8, *matrix);
    position = &lbl_1_bss_A1C[index];
    position->x = (*matrix)[0][3];
    position->y = (*matrix)[1][3];
    position->z = (*matrix)[2][3];
}

typedef struct M699MotionEntryFullTemplate_s {
    u32 header[3];
    s32 count;
    u32 parameters[5];
    Vec minimum;
    Vec maximum;
} M699MotionEntryFullTemplate;

typedef struct M699MotionEntryShortTemplate_s {
    u32 header[3];
    s32 count;
    u32 parameters[5];
    Vec minimum;
    f32 maximumX;
    f32 maximumY;
} M699MotionEntryShortTemplate;

typedef union M699MotionEntryBounds {
    M699MotionEntryFullTemplate full;
    M699MotionEntryShortTemplate shortBounds;
} M699MotionEntryBounds;

typedef struct M699MotionEntry_s {
    M699MotionEntryBounds bounds;
    u8 unk_3C[0x30]; /* No code names this field. */
} M699MotionEntry;

extern M699MotionEntry lbl_1_bss_4C4[7];


const M699MotionEntryFullTemplate lbl_1_rodata_5BC = {
    { 1, 4, 4 }, 0, { 13, 0, 0, 0, 0 }, { -950.0f, 0.0f, -570.0f }, { 950.0f, 0.0f, 570.0f }
};

const M699MotionEntryShortTemplate lbl_1_rodata_5F8 = {
    { 0, 4, 4 }, 1, { 10, 0, 0, 0, 0 }, { -198.0f, -200.0f, 0.0f }, 20.0f, 580.0f
};

const M699MotionEntryFullTemplate lbl_1_rodata_630 = {
    { 1, 4, 4 }, 3, { 12, 0, 0, 0, 0 }, { -70.0f, 270.0f, -180.0f }, { -50.0f, 480.0f, 180.0f }
};

const M699MotionEntryFullTemplate lbl_1_rodata_66C = {
    { 1, 4, 4 }, 5, { 12, 0, 0, 0, 0 }, { -70.0f, -20.0f, -15.0f }, { -42.0f, 0.0f, 15.0f }
};

void fn_1_10EA4(OMOBJ *object)
{
    M699MotionEntry *entry;

    entry = lbl_1_bss_4C4;
    memset(entry, 0, 0x2F4);
    entry->bounds.full = lbl_1_rodata_5BC;
    fn_1_1D734(entry++);

    entry->bounds.shortBounds = lbl_1_rodata_5F8;
    entry->bounds.shortBounds.minimum.x = lbl_1_bss_A1C[0].x +
                       lbl_1_rodata_5F8.minimum.x;
    fn_1_1D734(entry++);

    entry->bounds.shortBounds = lbl_1_rodata_5F8;
    entry->bounds.shortBounds.count++;
    entry->bounds.shortBounds.minimum.x = lbl_1_bss_A1C[1].x -
                       lbl_1_rodata_5F8.minimum.x;
    fn_1_1D734(entry++);

    entry->bounds.full = lbl_1_rodata_630;
    entry->bounds.full.minimum.x = lbl_1_bss_A1C[0].x +
                       lbl_1_rodata_630.minimum.x;
    entry->bounds.full.maximum.x = lbl_1_bss_A1C[0].x +
                       lbl_1_rodata_630.maximum.x;
    fn_1_1D734(entry++);

    entry->bounds.full = lbl_1_rodata_630;
    entry->bounds.full.count++;
    entry->bounds.full.minimum.x = lbl_1_bss_A1C[1].x -
                       lbl_1_rodata_630.maximum.x;
    entry->bounds.full.maximum.x = lbl_1_bss_A1C[1].x -
                       lbl_1_rodata_630.minimum.x;
    fn_1_1D734(entry++);

    entry->bounds.full = lbl_1_rodata_66C;
    entry->bounds.full.minimum.x = lbl_1_bss_A1C[0].x +
                       lbl_1_rodata_66C.minimum.x;
    entry->bounds.full.maximum.x = lbl_1_bss_A1C[0].x +
                       lbl_1_rodata_66C.maximum.x;
    entry->bounds.full.minimum.y = lbl_1_bss_A1C[0].y +
                       lbl_1_rodata_66C.minimum.y;
    entry->bounds.full.maximum.y = lbl_1_bss_A1C[0].y +
                        lbl_1_rodata_66C.maximum.y;
    fn_1_1D734(entry++);

    entry->bounds.full = lbl_1_rodata_66C;
    entry->bounds.full.count++;
    entry->bounds.full.minimum.x = lbl_1_bss_A1C[1].x -
                       lbl_1_rodata_66C.maximum.x;
    entry->bounds.full.maximum.x = lbl_1_bss_A1C[1].x -
                       lbl_1_rodata_66C.minimum.x;
    entry->bounds.full.minimum.y = lbl_1_bss_A1C[1].y +
                       lbl_1_rodata_66C.minimum.y;
    entry->bounds.full.maximum.y = lbl_1_bss_A1C[1].y +
                        lbl_1_rodata_66C.maximum.y;
    fn_1_1D734(entry++);
    object->objFunc = fn_1_105C8;
}


struct M699EventSource;
struct M699SoundEvent;

typedef struct M699MotionInitWork {
    s32 mode;
    s32 frameCount;
    s32 active;
    s32 parameter;
    s32 secondaryParameter;
    void *owner;
    u8 unk_18[4]; /* No code names this field. */
    OMOBJ_FUNC initCallback;
    u8 unk_20[4]; /* No code names this field. */
    s32 (*updateCallback)(struct M699EventSource *, struct M699SoundEvent *);
    void *positionPointer;
    void *rotationPointer;
    Vec initialPosition;
    f32 scaleX;
    f32 scaleY;
    f32 scaleZ;
    f32 motionScale;
    u8 unk_4C[0x30]; /* No code names this field. */
    Vec position;
    Vec rotation;
    u8 unk_94[0x2A4]; /* No code names this field. */
    f32 rotationX;
    f32 rotationY;
    f32 rotationZ;
    f32 initialFactor;
    u8 unk_348[0x1C]; /* No code names this field. */
    s32 activeState;
    u8 unk_368[4]; /* No code names this field. */
    s32 bounceState;
    u8 unk_370[0x1C]; /* No code names this field. */
    s32 transitionTimer;
} M699MotionInitWork;

typedef struct M699MotionInitController {
    u8 unk_0[0x14]; /* No code names this field. */
    void (*updateCallback)(void);
    u8 unk_18[0x44]; /* No code names this field. */
    M699MotionInitWork *work;
} M699MotionInitController;


void fn_1_12194(OMOBJ *object);

extern s32 fn_1_120C4(struct M699EventSource *source, struct M699SoundEvent *event);

extern void fn_1_11604(OMOBJ *object);

void fn_1_114A0(M699MotionInitController *controller)
{
    M699MotionInitWork *work;
    M699MotionInitWork *state;
    M699MotionInitWork *setup;

    work = controller->work;
    state = work;
    work->activeState = 1;
    work->bounceState = 2;
    work->transitionTimer = 0;
    work->rotationX = 0.0f;
    work->rotationY = 0.0f;
    work->rotationZ = 0.0f;
    work->initialFactor = 1.0f;
    setup = state;
    setup->mode = 0;
    setup->frameCount = 0x1C;
    setup->active = 1;
    setup->parameter = 0;
    setup->secondaryParameter = 0;
    setup->owner = controller;
    setup->initCallback = fn_1_12194;
    setup->updateCallback = fn_1_120C4;
    setup->positionPointer = &state->position;
    setup->rotationPointer = &state->rotation;
    setup->initialPosition = state->position;
    setup->scaleX = 30.0f;
    setup->scaleY = -30.0f;
    setup->scaleZ = 30.0f;
    setup->motionScale = 1.0f;
    fn_1_1D6BC(setup);
    controller->updateCallback = (void (*)(void))fn_1_11604;
}

typedef struct M699MotionPhaseWork {
    u32 flags;
    u8 unk_4[0x2C]; /* No code names this field. */
    Vec startPosition;
    u8 unk_3C[0xC]; /* No code names this field. */
    f32 motionActive;
    u8 unk_4C[0x30]; /* No code names this field. */
    Vec position;
    Vec velocity;
    u8 unk_94[0x20C]; /* No code names this field. */
    s32 phase;
    s32 unknown_2A4;
    s32 phaseTimer;
    u8 unk_2AC[0xB8]; /* No code names this field. */
    s32 activeState;
    u8 unk_368[4]; /* No code names this field. */
    s32 bounceState;
    void *motionEffect;
    s32 effectTimer;
    s32 effectEnabled;
    u8 unk_37C[8]; /* No code names this field. */
    f32 previousAngle;
    s32 frameCount;
    s32 soundEnabled;
    s32 transitionState;
    s32 transitionTimer;
    void *relatedMotion;
} M699MotionPhaseWork;

typedef struct M699MotionEffectSegment {
    u8 unk_0[0x34]; /* No code names this field. */
    f32 radius;
} M699MotionEffectSegment;

typedef struct M699RelatedMotion {
    u8 unk_0[0x1E0]; /* No code names this field. */
    f32 angle;
    u8 unk_1E4[8]; /* No code names this field. */
    M699MotionEffectSegment segment;
} M699RelatedMotion;

typedef struct M699MotionTarget {
    u8 unk_0[0x7C]; /* No code names this field. */
    Vec position;
    Vec velocity;
    u8 unk_94[4]; /* No code names this field. */
    f32 angle;
} M699MotionTarget;

typedef struct M699MotionMode {
    u8 unk_0[8]; /* No code names this field. */
    s32 mode;
} M699MotionMode;

typedef struct M699MotionCandidateWork {
    u8 unk_0[0x288]; /* No code names this field. */
    M699MotionMode *motionMode;
    s32 state;
} M699MotionCandidateWork;

extern OMOBJ *lbl_1_bss_7B8[0x80];


extern f32 fn_1_5098(Vec *first, Vec *second);

extern f32 fn_1_5230(Vec *first, Vec *second);

extern f32 fn_1_4FE4(f32 first, f32 second);

extern void fn_1_17EB0(Vec *position, Vec *velocity);

extern s32 fn_1_435C(s32 sound, Vec *position);


void fn_1_11604(OMOBJ *object)
{
    M699MotionPhaseWork *work = object->data;
    M699MotionPhaseWork *state = work;
    OMOBJ *candidate;
    M699MotionTarget *candidateTarget;
    M699MotionTarget *target;
    M699MotionEffectSegment *segment;
    M699RelatedMotion *motion;
    f32 angle;
    f32 horizontalSpeed;
    f32 targetVelocityX;
    f32 targetVelocityZ;
    f32 distanceToPlayer;
    Vec playerPosition;

    switch (work->phase) {
    case 0:
        if (!((f64)++work->phaseTimer < 18.0)) {
            candidate = fn_1_10190(4, 0);
            if (candidate != NULL) {
                candidateTarget = candidate->data;
                target = candidateTarget;
                angle = fn_1_5098(&state->position, &target->position);
                horizontalSpeed = 20.0f;
                state->velocity.x = (f32)(horizontalSpeed *
                    sin((M_PI * (f64)angle) / 180.0));
                state->velocity.y = 25.0f;
                state->velocity.z = (f32)(horizontalSpeed *
                    cos((M_PI * (f64)angle) / 180.0));
                work->phase++;
                work->phaseTimer = 0;
            }
        }
        break;
    case 1:
        if (!((f64)++work->phaseTimer < 30.0)) {
            work->phase++;
            work->phaseTimer = 0;
            work->soundEnabled = 1;
            /* fall through */
        } else {
            break;
        }
    case 2:
        if ((u32)fn_80071A8C() == 5U) {
            work->phase++;
            state->flags |= 1;
            /* fall through */
    case 3:
            if ((u32)fn_80071A8C() != 5U) {
                work->effectEnabled = 0;
            }
        }
        break;
    }

    motion = work->motionEffect;
    if (motion != NULL) {
        work->effectTimer++;
    }
    if (work->effectEnabled != 0) {
        if (motion == NULL) {
            goto motionUpdated;
        }
        {
        segment = &motion->segment;
        state->position.y = (f32)(15.0 + fabs((f64)(segment->radius *
            sin((M_PI * (f64)motion->angle) / 180.0))));
        state->velocity.x = state->velocity.y = state->velocity.z = 0.0f;
        state->motionActive = 1.0f;
        angle = 150.0f;
        if (work->previousAngle < angle) {
            if (motion->angle >= angle) {
                fn_1_435C(0x922, &state->position);
            }
        }
        angle = 330.0f;
        if (work->previousAngle < angle) {
            if (motion->angle >= angle) {
                fn_1_435C(0x922, &state->position);
            }
        }
        work->previousAngle = motion->angle;
        }
    } else {
        target = work->relatedMotion;
        if (target != NULL) {
            Vec targetPosition;
            angle = target->angle;
            distanceToPlayer = 75.0f;
            horizontalSpeed = 3.0f;
            targetPosition.x = (f32)(((f64)target->position.x + (distanceToPlayer *
                sin((M_PI * angle) / 180.0))) +
                (target->velocity.x * horizontalSpeed));
            targetPosition.y = target->position.y;
            targetPosition.z = (f32)(((f64)target->position.z + (distanceToPlayer *
                cos((M_PI * angle) / 180.0))) +
                (target->velocity.z * horizontalSpeed));
            angle = fn_1_5098(&state->position, &targetPosition);
            targetVelocityX = state->velocity.x;
            targetVelocityZ = state->velocity.z;
            if (fabs(fn_1_4FE4(angle, (f32)(180.0 *
                (atan2((f64)targetVelocityX, (f64)targetVelocityZ) /
                 M_PI)))) < 90.0) {
                horizontalSpeed = sqrtf((targetVelocityX * targetVelocityX) +
                    (targetVelocityZ * targetVelocityZ));
                targetVelocityX = (f32)((f64)horizontalSpeed *
                    sin((M_PI * (f64)angle) / 180.0));
                targetVelocityZ = (f32)((f64)horizontalSpeed *
                    cos((M_PI * (f64)angle) / 180.0));
                state->velocity.x = (f32)((f64)state->velocity.x +
                    0.1 * (f64)(targetVelocityX - state->velocity.x));
                state->velocity.z = (f32)((f64)state->velocity.z +
                    0.1 * (f64)(targetVelocityZ - state->velocity.z));
            }
        }
        state->velocity.y = (f32)((f64)state->velocity.y - 1.0896277777777779);
        PSVECAdd(&state->position, &state->velocity, &state->position);
        if (state->position.y < 30.0) {
            state->position.y = 30.0f;
            if (state->velocity.y < 0.0) {
                state->velocity.y = (f32)(0.7 * -(f64)state->velocity.y);
                if (state->velocity.y < 5.0) {
                    work->relatedMotion = NULL;
                }
                if (state->velocity.y < 1.6666666666666667) {
                    state->velocity.x *= 0.98;
                    state->velocity.z *= 0.98;
                } else {
                    state->velocity.x *= 0.7;
                    state->velocity.z *= 0.7;
                }
                if (state->velocity.y < 0.8333333333333334) {
                    state->velocity.y = 0.0f;
                }
                if (work->soundEnabled != 0 && state->velocity.y > 1.3333333333333333) {
                    fn_1_435C(0x922, &state->position);
                }
                work->bounceState = 2;
                state->motionActive = 1.0f;
            }
        }
    }

motionUpdated:
    if (PSVECMag(&state->velocity) > 16.666666666666668) {
        fn_1_17EB0(&state->position, &state->velocity);
    }
    if (work->frameCount != 0) {
        work->frameCount--;
    }
    if (0.0 == state->motionActive) {
        work->motionEffect = NULL;
        work->effectTimer = 0;
        work->effectEnabled = 0;
        state->flags |= 1;
        work->activeState = 1;
        switch (work->transitionState) {
        case 0:
            work->transitionState++;
            work->transitionTimer = 0;
            /* fall through */
        case 1:
            playerPosition = *fn_1_14778(state->position.x < 0.0 ? 0 : 1);
            if ((distanceToPlayer = fn_1_5230(&state->position, &playerPosition)) < 100.0) {
                state->position.x = playerPosition.x;
                state->position.z = playerPosition.z;
                if (state->velocity.y > 0.0) {
                    state->velocity.y *= 0.3;
                }
                if (work->transitionTimer == 0) {
                    state->startPosition.x = state->position.x;
                    state->startPosition.z = state->position.z;
                }
                if ((f64)++work->transitionTimer < 6.0) {
                    break;
                }
            }
            work->transitionState++;
            work->transitionTimer = 0;
            return;
        case 2:
            break;
        }
    } else {
        work->transitionState = 0;
        work->transitionTimer = 0;
    }
}


typedef struct M699EventSource {
    u8 unk_0[0x14]; /* No code names this field. */
    OMOBJ *object;
} M699EventSource;

typedef struct M699SoundEvent {
    u8 unk_0[0x10]; /* No code names this field. */
    s32 type;
} M699SoundEvent;

typedef struct M699SoundWork {
    u8 unk_0[0x7C]; /* No code names this field. */
    Vec effectPosition;
    Vec distanceVector;
    u8 unk_94[0x2F4]; /* No code names this field. */
    s32 soundState;
} M699SoundWork;


extern s32 fn_1_435C(s32 soundId, Vec *position);

s32 fn_1_120C4(M699EventSource *source, M699SoundEvent *event)
{
    OMOBJ *object;
    M699SoundWork *work;
    M699SoundWork *spatialWork;

    if (event->type != 0) {
        object = source->object;
        work = object->data;
        spatialWork = work;
        if (work->soundState == 0 &&
            PSVECMag(&spatialWork->distanceVector) > 0.8333333333333334) {
            switch (event->type) {
            case 12:
                fn_1_435C(0x924, &spatialWork->effectPosition);
                break;
            case 14:
                fn_1_435C(0x925, &spatialWork->effectPosition);
                break;
            default:
                fn_1_435C(0x922, &spatialWork->effectPosition);
                break;
            }
            work->soundState = 6;
        }
    }
}

typedef struct M699MotionPlacementWork {
    u8 unk_0[0x30]; /* No code names this field. */
    Vec sourcePosition;
    u8 unk_3C[0x40]; /* No code names this field. */
    Vec position;
    Vec velocity;
    f32 turnAngle;
    f32 rotationY;
    f32 rotationZ;
    u8 unk_A0[0x84]; /* No code names this field. */
    s32 motionActive;
    u8 unk_128[0x1B0]; /* No code names this field. */
    Mtx rotationMatrix;
    u8 unk_308[0x30]; /* No code names this field. */
    Quaternion orientation;
    u8 unk_348[0x1C]; /* No code names this field. */
    s32 modelHidden;
} M699MotionPlacementWork;


void fn_1_12194(OMOBJ *object)
{
    M699MotionPlacementWork *work = object->data;
    M699MotionPlacementWork *positionWork;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    f32 horizontalDistance;
    s32 model;

    positionWork = work;
    model = object->mdlId[0];
    deltaX = positionWork->position.x - positionWork->sourcePosition.x;
    deltaY = positionWork->position.y - positionWork->sourcePosition.y;
    deltaZ = positionWork->position.z - positionWork->sourcePosition.z;
    deltaY = (f32)(0.5 * fabs((f64)deltaY));
    deltaX = (f32)(deltaX + (deltaY *
        (3.0517578125e-05 * (f32)((u16)frand() - 0x8000))));
    deltaZ = (f32)(deltaZ + (deltaY *
        (3.0517578125e-05 * (f32)((u16)frand() - 0x8000))));

    horizontalDistance = sqrtf((deltaX * deltaX) + (deltaZ * deltaZ));
    if ((f64)horizontalDistance > 0.0) {
        Quaternion rotation;
        Vec rotationAxis;
        Vec up;
        Vec direction;

        up.x = 0.0f;
        up.y = 1.0f;
        up.z = 0.0f;
        direction.x = deltaX;
        direction.y = 0.0f;
        direction.z = deltaZ;
        PSVECNormalize(&direction, &direction);
        PSVECCrossProduct(&up, &direction, &rotationAxis);
        C_QUATRotAxisRad(&rotation, &rotationAxis,
            (f32)((f64)horizontalDistance / 60.0));
        PSQUATMultiply(&rotation, &work->orientation, &work->orientation);
        PSQUATNormalize(&work->orientation, &work->orientation);
        PSMTXQuat(work->rotationMatrix, &work->orientation);
        PSMTXCopy(work->rotationMatrix, lbl_802C01C8[model].mtx);
    }
    fn_80026790(model, &positionWork->position);
    if (fn_80071A8C() == 5) {
        fn_1_75C(positionWork->position.x, 30.0f,
                 positionWork->position.z, 30.0f);
    }
    if (work->modelHidden != 0) {
        fn_80026A44(model, (1 << 0));
        return;
    }
    fn_800269E8(model, (1 << 0));
}


Vec lbl_1_data_D98 = { 0.0f, 0.0f, 0.0f };

Vec lbl_1_data_DA4 = { 0.0f, 36.666667f, 3.3333333f };

f32 lbl_1_data_DB0[3] = { 0.0f, 0.0f, -750.0f };

extern void *fn_1_14B68(Vec *position, f32 blend, Vec *rotation, void *targetObject);

void fn_1_14D50(void *camera, s32 value);

extern f32 fn_1_4EC8(f32 start, f32 end, f32 factor);

extern s32 fn_1_10C0(void);

extern void fn_1_126A8(OMOBJ *object);

typedef struct M699ObjectMotionUpdateWork {
    u8 unk_0[0x7C]; /* No code names this field. */
    Vec position;
    Vec velocity;
    Vec rotation;
    u8 unk_A0[0x10]; /* No code names this field. */
    s32 motionState;
    u8 unk_B4[0x70]; /* No code names this field. */
    s32 motionEnabled;
    u8 unk_128[0x168]; /* No code names this field. */
    s32 updateCount;
} M699ObjectMotionUpdateWork;


void fn_1_12574(OMOBJ *object)
{
    M699MotionControlWork *work = object->data;
    M699MotionControlWork *state;
    s32 motion;
    s32 model;
    M699MotionParamView *param;
    u32 attributes;

    state = work;
    if (state->motionState != 0) {
        motion = object->mtnId[0];
        work->timer = 0;
        state->motionTime = 0;
        if (motion == -1) {
            state->motionState = -1;
            state->motionActive = 0;
        } else {
            param = work->paramOwner->motion;
            attributes = param->attributes;
            model = object->mdlId[0];
            if (state->motionState < 0 || 0.0 == param->blend) {
                fn_8002C95C(model, motion);
                fn_800269E8(model, attributes);
            } else {
                fn_8002CCC0(model, motion, 0.0f,
                                   60.0f * param->blend, attributes);
            }
            if ((attributes & ((1 << 30) | (1 << 2))) != 0) {
                fn_8002D414(model, Hu3DMotionMotionMaxTimeGet(motion));
            }
            state->motionState = 0;
        }
    }
    object->objFunc = fn_1_126A8;
}

void fn_1_126A8(OMOBJ *object)
{
    M699MotionControlWork *state = object->data;
    M699MotionControlWork *work = state;
    Vec cameraPosition;
    Vec cameraRotation;
    void *cameraObject;
    f32 speed;

    cameraObject = fn_1_14ACC(&cameraPosition, &cameraRotation);
    switch (state->sequenceState) {
    case 0:
        if (fn_1_5230(&work->position, &cameraPosition) > 150.0) {
            break;
        }
        state->sequenceState++;
        state->sequenceTimer = 0;
        cameraObject = fn_1_14B68(NULL, 1.0f, NULL, work);
        fn_1_14D50(work, 0);
    case 1:
        if (++state->sequenceTimer < 48.0) {
            break;
        }
        state->sequenceState++;
        state->sequenceTimer = 0;
        fn_1_10E0(1);
        break;
    case 2:
        if (fn_80071A8C() != 5) {
            break;
        }
        state->sequenceState++;
        state->sequenceTimer = 0;
        fn_1_10218(object, 1);
        work->motionActive = 1;
    case 3:
        if (++state->sequenceTimer < 12.0) {
            break;
        }
        state->sequenceState++;
        state->sequenceTimer = 0;
        cameraRotation = lbl_1_data_DA4;
        cameraObject = fn_1_14B68(NULL, 1.0f, &cameraRotation, NULL);
        fn_1_435C(0x928, &cameraPosition);
    case 4:
        if (work->motionActive != 0) {
            break;
        }
        state->sequenceState++;
        state->sequenceTimer = 0;
        fn_1_10218(object, 4);
    case 5:
        if (work->motionActive != 0) {
            break;
        }
        speed = -6.6666665f;
        work->velocity.z = work->velocity.z + 0.1 * (speed - work->velocity.z);
        if (work->position.z > lbl_1_data_DB0[2]) {
            break;
        }
        state->sequenceState++;
        state->sequenceTimer = 0;
    case 6:
        work->velocity.z *= 0.1;
        if (++state->sequenceTimer < 12.0) {
            break;
        }
        state->sequenceState++;
        state->sequenceTimer = 0;
        work->velocity.z = 0.0f;
    case 7:
        speed = work->position.x - cameraPosition.x;
        if (fabs(speed) < 50.0) {
            speed = 0.0f;
            fn_1_10218(object, 4);
        } else {
            if (speed > 0.0) {
                speed = -5.0f;
            } else {
                speed = 5.0f;
            }
            fn_1_10218(object, 5);
        }
        work->velocity.x = (f32)((f64)work->velocity.x + (0.05 * (f64)(speed - work->velocity.x)));
        speed = fn_1_5098(&work->position, &cameraPosition);
        work->rotationY = fn_1_4EC8(work->rotationY, speed, 0.2f);
        break;
    default:
        break;
    }
    PSVECAdd(&work->position, &work->velocity, &work->position);
    if (fn_1_10C0() == 1 && work != cameraObject
        && (cameraObject != NULL || (cameraRotation.y < 0.0f && cameraPosition.y < 150.0))) {
        fn_1_10E0(2);
    }
    if (work == cameraObject) {
        cameraPosition.x = work->position.x;
        cameraPosition.y = (f32)(60.0 + (f64)work->position.y);
        cameraPosition.z = (f32)(100.0 + (f64)work->position.z);
        cameraObject = fn_1_14B68(&cameraPosition, 0.1f, &lbl_1_data_D98, work);
        fn_1_14D50(work, 0);
    }
    fn_1_10404(object);
}

typedef struct M699MotionControllerInit {
    s32 motionState;
    s32 motionTime;
    s32 mode;
    s32 setupIndex;
    s32 active;
    OMOBJ *owner;
    s32 unknown_018;
    void (*update)(struct M699MotionController *controller);
    s32 unknown_020;
    s32 (*callback)(struct M699MotionCallbackView *motion, struct M699MotionEventView *event);
    Vec *position;
    Vec *velocity;
    Vec initialPosition;
    f32 initialRotation;
    f32 initialScaleX;
    f32 initialScaleY;
    f32 initialScaleZ;
} M699MotionControllerInit;

typedef struct M699InitialMotionWork {
    M699MotionControllerInit controller;
    u8 unk_4C[0x20]; /* No code names this field. */
    s32 setupMode;
    s32 setupCount;
    s32 nextSetupMode;
    u8 unk_78[4]; /* No code names this field. */
    Vec position;
    Vec velocity;
    u8 unk_94[0x1C]; /* No code names this field. */
    s32 motionState;
    s32 motionTime;
    u8 unk_B8[0x10]; /* No code names this field. */
    s32 setupReady;
    s32 nextSetupModeCopy;
    s32 setupState;
    u8 unk_D4[0x50]; /* No code names this field. */
    s32 motionActive;
    u8 unk_128[0x6C]; /* No code names this field. */
    s32 initialized;
    u8 unk_198[0x30]; /* No code names this field. */
    u32 setupValue;
    s32 setupModel;
    u8 unk_1D0[0x1C]; /* No code names this field. */
    M699InitialMotionRecord initialMotionData;
    M699MotionParamOwner *paramOwner;
    u32 id;
    s32 timer;
} M699InitialMotionWork;

char lbl_1_data_DDC[] = "c005m1-itemhook-r";

char lbl_1_data_DEE[] = "g000m0-itemhook_R";

extern void fn_1_6154(void *work);

extern void fn_1_137A4(struct M699MotionController *controller);

extern void fn_1_13460(OMOBJ *object);


void fn_1_1317C(OMOBJ *object)
{
    M699InitialMotionWork *state;
    M699MotionControllerInit *controller;
    s32 setupMode;
    f32 *initialValues;
    M699InitialMotionWork *work;
    s32 setupModel;

    work = object->data;
    state = work;
    initialValues = &state->initialMotionData.boundary;
    setupMode = work->id & 1;
    setupModel = object->mdlId[0];
    object->work[0] = setupMode + 4;
    state->setupMode = setupMode;
    state->setupCount = 2;
    state->nextSetupMode = state->setupMode + 4;
    state->setupReady = 1;
    state->nextSetupModeCopy = setupMode + 4;
    state->setupState = 0;
    state->setupValue = 0;

    if (setupMode == 0) {
        state->setupModel = fn_1_4CB0(fn_1_10B8(0x16));
        fn_8002741C((s16)setupModel, lbl_1_data_DDC,
                         (s16)state->setupModel);
    } else {
        state->setupModel = fn_80025A04(
            fn_80008DD0(fn_1_10B8(0x16), 0x10000000, 2));
    }
    fn_80026B8C((s16)state->setupModel, 5);

    state->initialMotionData = lbl_1_data_688[setupMode];

    fn_1_6154(state);
    state->initialized = 1;

    controller = &state->controller;
    controller->motionState = 0;
    controller->motionTime = 0x17;
    controller->mode = 2;
    controller->setupIndex = setupMode;
    controller->active = 0;
    controller->owner = object;
    controller->update = fn_1_137A4;
    controller->callback = fn_1_AA54;
    controller->position = &state->position;
    controller->velocity = &state->velocity;
    controller->initialPosition = state->position;
    controller->initialRotation = *initialValues;
    controller->initialScaleX = 0.0f;
    controller->initialScaleY = state->initialMotionData.height;
    controller->initialScaleZ = 0.0f;
    fn_1_1D6BC(controller);

    fn_1_10218(object, 0);
    object->objFunc = fn_1_13460;
}

typedef struct M699StageWork_s {
    u8 unk_0[0x48]; /* No code names this field. */
    f32 updateValue;
    u8 unk_4C[0x30]; /* No code names this field. */
    Vec movement;
    Vec position;
    u8 unk_94[0x2C]; /* No code names this field. */
    s32 updateCount;
    u8 unk_C4[0x108]; /* No code names this field. */
    s32 modelObject;
    Vec objectPosition;
    u8 unk_1DC[0xB0]; /* No code names this field. */
    s32 activeFlag;
    u8 unk_290[0x10]; /* No code names this field. */
    s32 updateMode;
} M699StageWork;

extern BOOL fn_1_6990(void *work);

extern BOOL fn_1_6B10(void *work);

extern BOOL fn_1_6C74(void *work);

extern BOOL fn_1_717C(void *work);

extern BOOL fn_1_739C(void *work);

extern void fn_1_F70C(void *work);

extern void fn_1_760C(void *work);

extern void fn_1_AD3C(void *work);

extern void fn_1_AEF4(void *work);

extern void fn_1_13628(OMOBJ *object);

void fn_1_13460(OMOBJ *object)
{
    M699StageWork *work = object->data;
    M699StageWork *state = work;
    s32 active = work->activeFlag & 1;
    s32 modelObject = object->mdlId[0];

    state->updateValue = 1.0f;
    fn_1_F70C(state);
    fn_1_760C(state);
    switch (work->updateMode) {
    case 0:
        if (fn_1_6990(state)) {
            work->updateMode++;
            state->updateCount = 0;
        }
        break;
    case 1:
        if (fn_1_6B10(state)) {
            work->updateMode++;
            state->updateCount = 0;
        }
        break;
    case 2:
        if (fn_1_6C74(state)) {
            work->updateMode++;
            state->updateCount = 0;
        }
        break;
    case 3:
        if (fn_1_717C(state)) {
            work->updateMode++;
            state->updateCount = 0;
        }
        break;
    case 4:
        if (fn_1_739C(state)) {
            work->updateMode++;
            state->updateCount = 0;
        }
        break;
    }
    fn_1_AD3C(state);
    fn_1_AEF4(state);
    fn_1_13628(object);
    PSVECAdd(&state->movement, &state->position, &state->movement);
    if (active == 0) {
        Hu3DModelObjPosGet(modelObject, lbl_1_data_DDC, &state->objectPosition);
    } else {
        Hu3DModelObjPosGet(modelObject, lbl_1_data_DEE, &state->objectPosition);
        fn_80026790((s16)state->modelObject, &state->objectPosition);
    }
}


typedef struct M699RequestedMotionParam {
    u32 unknown_00;
    f32 blend;
    u32 attributes;
} M699RequestedMotionParam;

typedef struct M699RequestedMotionOwner {
    u8 unk_0[0x1C]; /* No code names this field. */
    M699RequestedMotionParam *motions;
} M699RequestedMotionOwner;

typedef struct M699RequestedMotionWork {
    u8 unk_0[0xB0]; /* No code names this field. */
    s32 motionState;
    s32 motionTime;
    s32 requestedMotion;
    f32 motionSpeed;
    u8 unk_C0[0x64]; /* No code names this field. */
    s32 motionActive;
    u8 unk_128[0x160]; /* No code names this field. */
    M699RequestedMotionOwner *paramOwner;
    u8 unk_28C[4]; /* No code names this field. */
    s32 timer;
    u8 unk_294[4]; /* No code names this field. */
} M699RequestedMotionWork;

typedef struct M699MotionRequestOwner {
    u8 unk_0[0x1C]; /* No code names this field. */
    M699MotionRequestParam *motions;
} M699MotionRequestOwner;

typedef struct M699MotionControllerWork {
    u8 unk_0[0x7C]; /* No code names this field. */
    Vec position;
    u8 unk_88[0x10]; /* No code names this field. */
    f32 rotationY;
    u8 unk_9C[0x14]; /* No code names this field. */
    s32 motionState;
    s32 motionTime;
    u8 unk_B8[0x6C]; /* No code names this field. */
    s32 motionActive;
    u8 unk_128[0x160]; /* No code names this field. */
    M699MotionSequence *sequence;
    s32 dataIndex;
    s32 timer;
} M699MotionControllerWork;

typedef struct M699MotionChild {
    u8 unk_0[0x7C]; /* No code names this field. */
    Vec position;
    u8 unk_88[0x0C]; /* No code names this field. */
    Vec rotation;
    u8 unk_A0[0x10]; /* No code names this field. */
    s32 motionId;
    u8 unk_B4[0x70]; /* No code names this field. */
    s32 updateMotion;
    u8 unk_128[0x168]; /* No code names this field. */
    s32 updateCount;
} M699MotionChild;

typedef struct M699MotionController {
    u8 unk_0[0x40]; /* No code names this field. */
    s16 *modelId;
    u8 unk_44[0x18]; /* No code names this field. */
    M699MotionChild *work;
} M699MotionController;

extern void fn_1_F740(M699MotionChild *work, s16 motionId);

void fn_1_13628(OMOBJ *object)
{
    M699RequestedMotionWork *request = object->data;
    M699RequestedMotionWork *work;
    M699RequestedMotionWork *state;
    M699RequestedMotionWork *current;
    M699RequestedMotionParam *param;
    s32 motionIndex;
    s32 motionId;
    s32 modelId;
    u32 attributes;

    state = request;
    if (state->requestedMotion != -1) {
        motionIndex = state->requestedMotion;
        work = object->data;
        current = work;
        if (motionIndex == -1) {
            current->motionState = -1;
            current->motionActive = 0;
        } else if (current->motionState != motionIndex && motionIndex < 0x13) {
            motionId = object->mtnId[motionIndex];
            work->timer = 0;
            current->motionTime = 0;
            if (motionId == -1) {
                current->motionState = -1;
                current->motionActive = 0;
            } else {
                param = &work->paramOwner->motions[motionIndex];
                attributes = param->attributes;
                modelId = object->mdlId[0];
                if (current->motionState < 0 || 0.0 == param->blend) {
                    fn_8002C95C(modelId, motionId);
                    fn_800269E8(modelId, attributes);
                } else {
                    fn_8002CCC0(modelId, motionId, 0.0f,
                                       60.0f * param->blend, attributes);
                }
                if ((attributes & ((1 << 30) | (1 << 2))) != 0) {
                    fn_8002D414(modelId, Hu3DMotionMotionMaxTimeGet(motionId));
                }
                current->motionState = motionIndex;
            }
        }
        fn_8002D86C(object->mdlId[0], state->motionSpeed);
    }
}

void fn_1_137A4(M699MotionController *controller)
{
    M699MotionChild *requestedWork;
    M699MotionChild *initialWork;
    s16 requestModelId;
    M699MotionChild *work;
    M699MotionChild *currentWork;
    s32 modelId;
    M699MotionChild *motionWork;
    s16 motionId;
    M699MotionChild *refreshedWork;
    float rotationY;
    float rotationRemainder;
    float normalizedRotation;
    s32 motionFinished;

    requestedWork = controller->work;
    initialWork = requestedWork;
    requestModelId = *controller->modelId;
    fn_1_F740(initialWork, requestModelId);
    work = controller->work;
    currentWork = work;
    modelId = *controller->modelId;
    rotationY = currentWork->rotation.y;
    rotationRemainder = fmod(rotationY, 360.0);
    normalizedRotation = rotationRemainder;
    currentWork->rotation.y = normalizedRotation;
    currentWork->rotation.x = currentWork->rotation.z = 0.0f;
    fn_80026790(modelId, &currentWork->position);
    fn_80026838(modelId, &currentWork->rotation);

    if (currentWork->updateMotion != 0) {
        motionWork = controller->work;
        refreshedWork = motionWork;
        motionId = *controller->modelId;
        if (refreshedWork->motionId == -1) {
            motionFinished = 1;
        } else if (fn_8002D75C((s32)motionId) != 0 &&
                   fn_8002D3EC((s32)motionId) < 0) {
            motionFinished = 1;
        } else {
            motionFinished = 0;
        }
        if (motionFinished != 0) {
            currentWork->updateMotion = 0;
        }
    }
    work->updateCount += 1;
}

typedef struct M699GlobalMotionWork {
    u8 unk_0[8]; /* No code names this field. */
    s32 modelId;
} M699GlobalMotionWork;

typedef struct M699MotionObjectName {
    char name[8];
} M699MotionObjectName;

M699MotionObjectName lbl_1_data_E00[10] = {
    { "kuri01" }, { "kuri02" }, { "kuri03" }, { "kuri04" }, { "kuri05" },
    { "kuri06" }, { "kuri07" }, { "kuri08" }, { "kuri09" }, { "kuri10" },
};

M699MotionObjectName lbl_1_data_E50[5] = {
    { "kame01" }, { "kame02" }, { "kame03" }, { "kame04" }, { "kame05" },
};


extern void fn_1_13BB4(OMOBJ *object);


void fn_1_138FC(OMOBJ *object)
{
    M699MotionControllerWork *work;
    M699MotionControllerWork *positionWork;
    s32 modelId;
    M699GlobalMotionWork *globalMotionWork;
    OMOBJ *globalObject;
    M699GlobalMotionWork *linkedMotionWork;
    HSF_OBJECT *modelObject;
    HSF_OBJECT *linkedModelObject;
    M699MotionObjectName *objectName;
    OMOBJ *linkedGlobalObject;
    M699MotionObjectName *linkedObjectName;

    work = object->data;
    positionWork = work;
    modelId = object->mdlId[0];
    switch (work->sequence->mode) {
    case 5:
        objectName = &lbl_1_data_E00[work->dataIndex];
        globalObject = lbl_1_bss_9B8;
        globalMotionWork = globalObject->data;
        modelObject = fn_80026BF4(globalMotionWork->modelId, objectName->name);
        if (modelObject != NULL) {
            positionWork->position.x = modelObject->mesh.base.pos.x;
            positionWork->position.y = modelObject->mesh.base.pos.y;
            positionWork->position.z = modelObject->mesh.base.pos.z;
            positionWork->rotationY = modelObject->mesh.base.rot.y;
        }
        break;
    case 6:
        linkedObjectName = &lbl_1_data_E50[work->dataIndex];
        linkedGlobalObject = lbl_1_bss_9B8;
        linkedMotionWork = linkedGlobalObject->data;
        linkedModelObject = fn_80026BF4(linkedMotionWork->modelId,
                                               linkedObjectName->name);
        if (linkedModelObject != NULL) {
            positionWork->position.x = linkedModelObject->mesh.base.pos.x;
            positionWork->position.y = linkedModelObject->mesh.base.pos.y;
            positionWork->position.z = linkedModelObject->mesh.base.pos.z;
            positionWork->rotationY = linkedModelObject->mesh.base.rot.y;
        }
        break;
    }

    fn_1_10218(object, 1);

    fn_8002D414(modelId,
        (f32)(((f32)(s32)(frand() & 0xFFFF) * 1.52587890625e-05) *
              (fn_8002D530(modelId) - 1.0)));
    object->objFunc = fn_1_13BB4;
}

typedef struct M699SceneMotionState {
    u8 unk_0[4]; /* No code names this field. */
    s32 cameraTargetActive;
} M699SceneMotionState;


typedef struct M699MotionBehaviorRenderWork {
    u8 unk_0[0x7C]; /* No code names this field. */
    Vec position;
    u8 unk_88[0xC]; /* No code names this field. */
    Vec rotation;
    u8 unk_A0[0x10]; /* No code names this field. */
    s32 motionState;
    u8 unk_B4[0x70]; /* No code names this field. */
    s32 motionEnabled;
    u8 unk_128[0x168]; /* No code names this field. */
    s32 updateCount;
} M699MotionBehaviorRenderWork;


static inline M699GlobalMotionWork * GlobalMotionOf13BB4(OMOBJ *globalObject)
{
    return globalObject->data;
}

typedef struct M699MotionBehaviorWork {
    u8 unk_0[0xB0]; /* No code names this field. */
    s32 motionState;
    u8 unk_B4[0x1D4]; /* No code names this field. */
    M699MotionSequence *sequence;
    u32 sequenceFlags;
    s32 timer;
    u8 unk_294[0xC]; /* No code names this field. */
    u32 sequenceState;
    u8 unk_2A4[4]; /* No code names this field. */
    s32 sequenceTimer;
} M699MotionBehaviorWork;


void fn_1_13BB4(OMOBJ *object)
{
    M699SceneMotionState * scene = lbl_1_bss_9B8->data;
    M699MotionBehaviorWork * behavior = object->data;
    M699MotionBehaviorWork * motion = behavior;
    s32 modelId = object->mdlId[0];
    f32 motionSpeed;
    switch (behavior->sequenceState)
    {
    case 0 :
        behavior->sequenceState++;
        behavior->sequenceTimer = (s32)(120.0 + (f64)(s32)(180.0 * (1.52587890625e-05 * (f64)(f32)(s32)(frand() & 0xFFFF))));
    case 1 :
        if (--behavior->sequenceTimer > 0)
        {
            break;
        }
        behavior->sequenceState++;
        behavior->sequenceTimer = (s32)(60.0 + (f64)(s32)(600.0 * (1.52587890625e-05 * (f64)(f32)(s32)(frand() & 0xFFFF))));
        fn_1_10218(object, 0);
    case 2 :
        if (--behavior->sequenceTimer <= 0)
        {
            behavior->sequenceState++;
            behavior->sequenceTimer = 1;
            break;
        }
        if (fn_80071A8C() == 6)
        {
            behavior->sequenceState++;
            behavior->sequenceTimer = (s32)((behavior->sequenceFlags & 7) * 5);
        }
        if (scene->cameraTargetActive == 0) {
            break;
        }
        behavior->sequenceState++;
        behavior->sequenceTimer = (s32)((behavior->sequenceFlags & 7) * 5);
    case 3 :
        if (--behavior->sequenceTimer > 0)
        {
            break;
        }
        fn_1_10218(object, 1);
        behavior->sequenceState++;
        behavior->sequenceTimer = 0x78;
    case 4 :
        if (fn_80071A8C() != 5) {
            break;
        }
        if (--behavior->sequenceTimer > 0) {
            break;
        }
        behavior->sequenceState++;
        if (scene->cameraTargetActive == 0) {
            behavior->sequenceState = 2;
            behavior->sequenceTimer = (s32)(60.0 + (f64)(s32)(600.0 * (1.52587890625e-05 * (f64)(f32)(s32)(frand() & 0xFFFF))));
            fn_1_10218(object, 0);
            break;
        }
    case 5 :
        if (scene->cameraTargetActive != 0) {
            break;
        }
        behavior->sequenceState++;
        behavior->sequenceTimer = (s32)(30.0 + (f64)(s32)(120.0 * (1.52587890625e-05 * (f64)(f32)(s32)(frand() & 0xFFFF))));
    case 6 :
        if (fn_80071A8C() == 5)
        {
            if (--behavior->sequenceTimer <= 0)
            {
                behavior->sequenceState = 2;
                behavior->sequenceTimer = (s32)(120.0 + (f64)(s32)(600.0 * (1.52587890625e-05 * (f64)(f32)(s32)(frand() & 0xFFFF))));
                fn_1_10218(object, 0);
            }
        }
        break;
    }
    if (motion->motionState == 0)
    {
        motionSpeed = 1.0;
    }
    else
    {
        switch (behavior->sequence->mode) {
        case 5:
            motionSpeed = scene->cameraTargetActive != 0 ? 1.0 : 0.7;
            break;
        case 6:
            motionSpeed = scene->cameraTargetActive != 0 ? 2.0 : 1.0;
            break;
        default:
            motionSpeed = 1.0f;
            break;
        }
    }
    fn_8002D86C((s16) modelId, motionSpeed);
    fn_1_10404(object);
}


Vec *fn_1_14778(s32 index) {
    return &lbl_1_bss_A1C[index & 1];
}

Mtx *fn_1_14790(s32 index) {
    return &lbl_1_bss_9BC[index & 1];
}

typedef struct M699DirectionWork {
    u8 unk_0[0x7C]; /* No code names this field. */
    Vec position;
} M699DirectionWork;

s32 fn_1_147A8(Vec *position, f32 heading, f32 angleLimit, f32 distanceLimit)
{
    M699DirectionWork *globalWork;
    M699DirectionWork *directionWork;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    f32 angleDifference;
    f32 wrappedAngle;

    if (lbl_1_bss_4C0 != NULL) {
        globalWork = lbl_1_bss_4C0->data;
        directionWork = globalWork;
        deltaX = directionWork->position.x - position->x;
        deltaY = directionWork->position.y - position->y;
        deltaZ = directionWork->position.z - position->z;
        if (angleLimit < 180.0) {
            angleDifference = (f32)(180.0 *
                (atan2((f64)deltaX, (f64)deltaZ) / M_PI));
            wrappedAngle = (f32)fmod((f64)(angleDifference - heading), 360.0);
            angleDifference = wrappedAngle;
            if (angleDifference < 0.0f) {
                angleDifference += 360.0f;
            }
            if (angleDifference > angleLimit && angleDifference < 360.0f - angleLimit) {
                return 0;
            }
        }
        angleDifference = sqrtf((deltaZ * deltaZ) +
                                ((deltaX * deltaX) + (deltaY * deltaY)));
        if (angleDifference > distanceLimit) {
            return 0;
        }
        return 1;
    }
    return 0;
}


typedef struct M699CameraRecord_s {
    u8 unk_0[0x7C]; /* No code names this field. */
    Vec position;
    Vec rotation;
    u8 unk_94[0x2DC]; /* No code names this field. */
    OMOBJ *result;
} M699CameraRecord;

void *fn_1_14ACC(Vec *position, Vec *rotation)
{
    M699CameraRecord *state;
    M699CameraRecord *currentState;

    if (lbl_1_bss_4C0 != NULL) {
        state = lbl_1_bss_4C0->data;
        currentState = state;
        if (position != NULL) {
            *position = currentState->position;
        }
        if (rotation != NULL) {
            *rotation = currentState->rotation;
        }
        return state->result;
    }
    return 0;
}

typedef struct M699CameraAdjustmentState {
    s32 stateFlags;
    u8 unk_4[0x2C]; /* No code names this field. */
    Vec previousPosition;
    u8 unk_3C[0x40]; /* No code names this field. */
    Vec position;
    Vec rotation;
    u8 unk_94[0x2D0]; /* No code names this field. */
    s32 active;
    u8 unk_368[4]; /* No code names this field. */
    s32 mode;
    void *targetObject;
    s32 targetTimer;
    s32 targetParameter;
    s32 targetValue;
    s32 savedTargetValue;
    f32 targetAmount;
    u8 unk_388[0x10]; /* No code names this field. */
    u32 result;
} M699CameraAdjustmentState;

void *fn_1_14B68(Vec *position, f32 blend, Vec *rotation, void *targetObject)
{
    M699CameraAdjustmentState *work;
    M699CameraAdjustmentState *currentState;

    if (lbl_1_bss_4C0 != NULL) {
        work = lbl_1_bss_4C0->data;
        currentState = work;
        if (position != NULL) {
            currentState->position.x = currentState->position.x +
                (blend * (position->x - currentState->position.x));
            currentState->position.y = currentState->position.y +
                (blend * (position->y - currentState->position.y));
            currentState->position.z = currentState->position.z +
                (blend * (position->z - currentState->position.z));
        }
        if (rotation != NULL) {
            currentState->rotation = *rotation;
        if (PSVECMag(rotation) > 0.0) {
                currentState->previousPosition = currentState->position;
            }
        }
        if (targetObject != NULL) {
            if (work->targetObject != targetObject &&
                (work->targetObject == NULL || work->targetTimer > 5)) {
                work->targetObject = targetObject;
                work->targetTimer = 0;
                currentState->stateFlags &= ~1;
                work->mode = 2;
                work->targetValue = 0;
                work->active = 1;
                work->targetAmount = 0.0f;
                work->result = 0;
            }
        } else {
            work->targetObject = NULL;
            work->targetTimer = 0;
            work->targetParameter = 0;
            currentState->stateFlags |= 1;
            work->active = 1;
        }
        return work->targetObject;
    }
    return NULL;
}


void fn_1_14D50(void *camera, s32 value)
{
    M699CameraAdjustmentState *state;

    if (lbl_1_bss_4C0 != NULL) {
        state = lbl_1_bss_4C0->data;
        if (camera != NULL && state->targetObject == camera) {
            state->targetParameter = value;
        }
    }
}

void fn_1_14DA8(void *camera, s32 value)
{
    M699CameraAdjustmentState *state;

    if (lbl_1_bss_4C0 != NULL) {
        state = lbl_1_bss_4C0->data;
        if (camera != NULL && state->targetObject == camera) {
            state->targetValue = value;
        }
    }
}

void fn_1_14E00(void *camera, s32 value)
{
    M699CameraAdjustmentState *state;

    if (lbl_1_bss_4C0 != NULL) {
        state = lbl_1_bss_4C0->data;
        if (camera != NULL && state->targetObject == camera) {
            state->active = value;
        }
    }
}

void fn_1_14E58(void *camera, s32 value)
{
    M699CameraAdjustmentState *state;

    if (lbl_1_bss_4C0 != NULL) {
        state = lbl_1_bss_4C0->data;
        if (camera != NULL && state->targetObject == camera) {
            state->result = value;
        }
    }
}


/* Set the active record score when its owner exists. */
void fn_1_14EB0(s32 value) {
    M699Record *record;

    if (lbl_1_bss_4C0 != NULL) {
        record = lbl_1_bss_4C0->data;
        record->score = value;
    }
}


/* Return the active record score, or zero when its owner is absent. */
s32 fn_1_14EF0(void) {
    M699Record *record;

    if (lbl_1_bss_4C0 != NULL) {
        record = lbl_1_bss_4C0->data;
        return record->score;
    }
    return 0;
}


/* Store the player and snapshot the current target value. */
void fn_1_14F38(s32 player) {
    M699Record *record;

    if (lbl_1_bss_4C0 != NULL) {
        record = lbl_1_bss_4C0->data;
        record->player = player;
        record->savedTargetValue = record->targetValue;
    }
}


/* Return the stored player, or zero when the record owner is absent. */
s32 fn_1_14F80(void) {
    M699Record *record;

    if (lbl_1_bss_4C0 != NULL) {
        record = lbl_1_bss_4C0->data;
        return record->player;
    }
    return 0;
}


/* Return the saved target value, or zero when the record owner is absent. */
s32 fn_1_14FC8(void) {
    M699Record *record;

    if (lbl_1_bss_4C0 != NULL) {
        record = lbl_1_bss_4C0->data;
        return record->savedTargetValue;
    }
    return 0;
}

Vec lbl_1_bss_A1C[2];
Mtx lbl_1_bss_9BC[2];
OMOBJ *lbl_1_bss_9B8;
OMOBJ *lbl_1_bss_7B8[128];
M699MotionEntry lbl_1_bss_4C4[7];
OMOBJ *lbl_1_bss_4C0;
