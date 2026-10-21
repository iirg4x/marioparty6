// Creates and updates Trap Ease Artist's board props, effects, and scenery.
#include "dolphin/types.h"
#include "dolphin/math.h"
#include "dolphin/mtx.h"
#include "game/object.h"
#include "game/main.h"
#include "game/memory.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/hu3d.h"
#include "game/hsfex.h"
#include "game/gamework.h"
#include "game/mg/seqman.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/timer.h"
#include "game/audio.h"
#include "game/frand.h"
#include "REL/m620Dll/module_globals.h"
#include "msm_se.h"
#include <string.h>
#define M620_WATER_ANIMATION_INDEX 21
#define M620_PLAYER_DESCENT_SOUND_ID 1769
#define M620_PLAYER_LANDING_SOUND_ID 1770
#define M620_FIRST_PLAYER_READY_SOUND_ID 1771
#define M620_ALTERNATE_LAUNCH_SOUND_ID 1773
s32 fn_1_35E0(s32 effect, Vec *position);
void fn_1_68A0(OMOBJ *object);
s32 fn_1_FB8(s32 resource);
s32 fn_1_4408(s32 resource);
s32 fn_1_4510(s32 model, s32 resource);
typedef struct M620PropMotionParam_s {
    s32 resourceIndex; /* Motion animation index in the m620 archive. */
    f32 blend; /* Blend duration in seconds, converted to 60-Hz frames when the motion changes. */
    u32 attr; /* Playback attributes applied to this motion. */
} M620PropMotionParam;
typedef struct M620PropParam_s {
    s32 count; /* Number of objects in this prop group. */
    s32 linkModel; /* Whether the prop uses its linked model. */
    s32 type; /* Prop group used by object lookup. */
    s16 firstId; /* First object ID assigned to this group. */
    s16 layer; /* Render layer for the prop models. */
    u32 attr; /* Model or initial motion attributes. */
    s32 shadow; /* Shadow mode: 0 disables setup, 1 casts shadows, and 2 receives the shadow map. */
    s32 modelResourceIndex; /* Model file index in the m620 archive. */
    M620PropMotionParam *motions; /* Available motions, terminated by resource -2. */
    Vec *position; /* Optional initial position copied to every prop in the group. */
    f32 rotationY; /* Initial Y rotation in degrees. */
    OMOBJ_FUNC update; /* Object update callback. */
} M620PropParam;
/* Motion and sequence state shared by each allocated prop and its callbacks. */
typedef struct M620PropMotionState_s {
    f32 movementSoundDistance; /* Travel toward the next movement-sound attempt; cooldowns may
                                * suppress playback. */
    u8 unknown_78_to_87[16];
    s32 motion;
    s32 motionTime;
    u8 unknown_90_to_97[8];
    s32 phase;
    s32 timer;
    u8 unknown_A0_to_C3[36];
    s32 motionActive;
    s32 motionTimer;
    u8 unknown_CC_to_10B[64];
} M620PropMotionState;
/* Collision registrations use this header before their shape-specific fields. */
typedef struct M620PropRegistration_s {
    u32 flags;
    u32 mask;
    u32 type;
    s32 id;
    s32 state;
    OMOBJ *primaryRelation;
    OMOBJ *secondaryRelation;
    OMOBJ_FUNC update;
} M620PropRegistration;
typedef struct M620PlayerLaunchSetup {
    s32 flags;
    s32 mask;
    s32 unknown_08;
    s32 unknown_0C;
    s32 state;
    OMOBJ *primaryRelation;
    s32 secondaryRelation;
    s32 invoke;
    Vec *position;
    f32 lowerHeight;
    f32 heightSpan;
    f32 innerRadius;
    f32 radialThickness;
} M620PlayerLaunchSetup;

typedef struct M620EffectSetup M620B3A0Controller;
typedef struct M620EffectSetup M620B3A0OtherCollision;

typedef struct M620EffectSetup {
    s32 flags;
    s32 mask;
    s32 unknown_08;
    s32 unknown_0C;
    s32 state;
    OMOBJ *object;
    OMOBJ *secondaryRelation;
    void (*update)(OMOBJ *);
    s32 (*collisionCallback)(M620B3A0Controller *, M620B3A0OtherCollision *);
    Vec *positionSource;
    Vec *offsetSource;
    Vec positionCopy;
    f32 unknown_38;
    f32 unknown_3C;
} M620EffectSetup;

/* Every configured prop uses the same allocation; its callback selects the collision
 * shape and the water, launch, or moving-effect members it needs. */
typedef struct M620PropWork_s {
    union {
        M620PropRegistration registration;
        M620PlayerLaunchSetup launch;
        M620EffectSetup effect;
    } collision;
    u8 unknown_40_to_4F[16];
    Vec position;
    Vec velocity;
    Vec rotation;
    M620PropMotionState motionState;
    M620PropParam *param;
    s32 id;
    s32 timer;
    Vec startPosition;
    s32 phase;
    u8 unknown_128_to_12B[4];
    s32 launchTimer;
    u8 unknown_130_to_133[4];
    union {
        s32 launchRequested; /* Requests another launch from the player's start. */
        s32 movementMode; /* Selects the moving effect's assigned-player action. */
    } action;
    union {
        s32 selectionReady; /* This launch prop is ready for nearby-prop selection. */
        s32 playerIndex; /* Player assigned to a moving effect. */
    } selection;
    u8 unknown_13C_to_143[8];
    union {
        Vec destination;
        struct {
            f32 textureOffsetX;
            f32 textureOffsetY;
        } water;
    } appearance;
    Vec target;
    u8 unknown_15C_to_1DF[132];
    f32 burstEmissionAccumulator;
    s32 pointValue;
    f32 steeringAngle; /* Wandering heading offset, in degrees. */
    f32 steeringSpeed; /* Target horizontal travel per update, in world units. */
    s32 shadowActive;
    s32 ringActive;
    s32 ringBlendMode;
    f32 ringRadius;
    Vec *ringPosition;
    s32 firstEffect;
    s32 secondEffect;
    s32 descentSoundPlayed; /* Prevents the descent sound from playing twice during one drop. */
} M620PropWork;

typedef M620PropWork M620PropState;
typedef M620PropWork M620WaterWork;
typedef M620PropWork M620PlayerLaunchWork;
typedef M620PropWork M620EffectMotionWork;

extern u32 MgSeqModeGet(void);

void fn_1_65C4(OMOBJ *obj);
void fn_1_65FC(HUPROCESS *, M620PropParam *);
void fn_1_7090(OMOBJ *);
void fn_1_7AB8(OMOBJ *);
extern s32 lbl_1_bss_3A8;
extern OMOBJ *lbl_1_bss_448[128];
extern OMOBJ *lbl_1_bss_648;
void fn_1_6DD0(OMOBJ *object);
void fn_1_6EB4(OMOBJ *object);
void fn_1_7008(OMOBJ *object);
void fn_1_7130(OMOBJ *object);
void fn_1_7868(OMOBJ *object);
void fn_1_8B00(OMOBJ *object);
void fn_1_8D4C(OMOBJ *object);

Vec lbl_1_data_1F0 = {0.0f, 0.0f, 0.0f};

M620PropMotionParam lbl_1_data_1FC[5] = {
    {8, 0.2f, HU3D_MOTATTR_LOOP},
    {11, 0.2f, HU3D_MOTATTR_NONE},
    {9, 0.2f, HU3D_MOTATTR_LOOP},
    {10, 0.2f, HU3D_MOTATTR_LOOP},
    {-2, 0.0f, HU3D_MOTATTR_NONE}
};

M620PropMotionParam lbl_1_data_238[6] = {
    {13, 0.2f, HU3D_MOTATTR_LOOP},
    {16, 0.2f, HU3D_MOTATTR_NONE},
    {14, 0.2f, HU3D_MOTATTR_LOOP},
    {15, 0.2f, HU3D_MOTATTR_LOOP},
    {-2, 0.0f, HU3D_MOTATTR_NONE},
    {0, 0.0f, HU3D_MOTATTR_NONE}
};

M620PropParam lbl_1_data_280[14] = {
    {1, 0, 0, 0, 3, HU3D_MOTATTR_LOOP, 0, 24, NULL, NULL, 0.0f, fn_1_6DD0},
    {1, 0, 0, 1, 1, HU3D_MOTATTR_NONE, 0, 25, NULL, NULL, 0.0f, fn_1_6EB4},
    {1, 0, 0, 2, 3, HU3D_MOTATTR_PAUSE, 0, 26, NULL, NULL, 0.0f, fn_1_7008},
    {1, 0, 1, 0, 5, HU3D_MOTATTR_PAUSE, 0, 1, NULL, NULL, 0.0f, fn_1_7130},
    {1, 0, 1, 1, 5, HU3D_MOTATTR_PAUSE, 0, 2, NULL, NULL, 0.0f, fn_1_7130},
    {1, 0, 1, 2, 5, HU3D_MOTATTR_PAUSE, 0, 3, NULL, NULL, 0.0f, fn_1_7130},
    {1, 0, 1, 3, 5, HU3D_MOTATTR_PAUSE, 0, 4, NULL, NULL, 0.0f, fn_1_7130},
    {1, 0, 2, 0, 5, HU3D_MOTATTR_PAUSE, 1, 5, NULL, NULL, 0.0f, fn_1_7868},
    {1, 0, 2, 1, 5, HU3D_MOTATTR_PAUSE, 1, 6, NULL, NULL, 0.0f, fn_1_7868},
    {1, 0, 2, 2, 5, HU3D_MOTATTR_PAUSE, 1, 5, NULL, NULL, 0.0f, fn_1_7868},
    {1, 0, 2, 3, 5, HU3D_MOTATTR_PAUSE, 1, 6, NULL, NULL, 0.0f, fn_1_7868},
    {33, 1, 3, 0, 4, HU3D_MOTATTR_NONE, 0, 7, lbl_1_data_1FC, NULL, 0.0f, fn_1_8B00},
    {2, 1, 3, 33, 4, HU3D_MOTATTR_NONE, 0, 12, lbl_1_data_238, NULL, 0.0f, fn_1_8D4C},
    {-1, 0, 0, 0, 0, HU3D_MOTATTR_NONE, 0, 0, NULL, NULL, 0.0f, NULL}
};

Vec lbl_1_data_4E8[2] = {
    {-932.0f, -20.0f, -475.0f},
    {-592.0f, -20.0f, -136.0f}
};

GXColor lbl_1_data_500 = {255, 255, 255, 255};

f32 lbl_1_data_504[2][3] = {{0.02f, 0.0f, 0.0f}, {0.0f, 0.02f, 0.0f}};

f32 lbl_1_data_51C[4] = {20.0f, 340.0f, 20.0f, 340.0f};

Vec lbl_1_data_52C[4] = {
    {-275.0f, 1000.0f, -275.0f},
    {275.0f, 1000.0f, -275.0f},
    {-275.0f, 1000.0f, 275.0f},
    {275.0f, 1000.0f, 275.0f}
};

Vec lbl_1_data_55C[4] = {
    {-200.0f, 2000.0f, -200.0f},
    {200.0f, 2000.0f, -200.0f},
    {-200.0f, 2000.0f, 200.0f},
    {200.0f, 2000.0f, 200.0f}
};

Vec lbl_1_data_58C[4] = {
    {-350.0f, 900.0f, -350.0f},
    {350.0f, 900.0f, -350.0f},
    {-350.0f, 900.0f, 350.0f},
    {350.0f, 900.0f, 350.0f}
};

Vec lbl_1_data_5BC[4] = {
    {-450.0f, 100.0f, -450.0f},
    {450.0f, 100.0f, -450.0f},
    {-450.0f, 100.0f, 450.0f},
    {450.0f, 100.0f, 450.0f}
};

Vec lbl_1_data_5EC[4] = {
    {-280.0f, 0.0f, -200.0f},
    {280.0f, 0.0f, -200.0f},
    {-280.0f, 0.0f, 400.0f},
    {280.0f, 0.0f, 400.0f}
};

Vec lbl_1_data_61C[4] = {
    {-50.0f, 0.0f, -50.0f},
    {50.0f, 0.0f, -50.0f},
    {-50.0f, 0.0f, 500.0f},
    {50.0f, 0.0f, 500.0f}
};

Vec lbl_1_data_64C[5] = {
    {-350.0f, 0.0f, -300.0f},
    {-220.0f, 0.0f, -300.0f},
    {-350.0f, 0.0f, -450.0f},
    {-200.0f, 0.0f, -450.0f},
    {-400.0f, 0.0f, -400.0f}
};

Vec lbl_1_data_688[2] = {
    {-500.0f, 0.0f, 0.0f},
    {500.0f, 0.0f, 0.0f}
};

Vec lbl_1_data_6A0 = {0.0f, 0.0f, -500.0f};

/* Module setup clears the prop registry, initializes its manager, and creates
 * each configured prop group before the objects begin their update callbacks. */
void fn_1_64A8(HUPROCESS *process)
{
    OMOBJ *managerObject;
    OMOBJ **registeredObject;
    M620PropParam *param;
    s32 registryIndex;
    void *managerWork;

    registeredObject = lbl_1_bss_448;
    registryIndex = 0;
    while (registryIndex < 128) {
        *registeredObject = NULL;
        registryIndex += 1;
        registeredObject += 1;
    }
    managerObject = omAddObjEx(process, 12, 0U, 0U, -1, fn_1_65C4);
    lbl_1_bss_648 = managerObject;
    managerObject->stat |= 0x100;
    managerWork = HuMemDirectMallocNum(HEAP_HEAP, 8, HU_MEMNUM_OVL);
    managerObject->data = managerWork;
    memset(managerWork, 0, 8);
    lbl_1_bss_3A8 = 0;
    lbl_1_bss_3A4 = 0;
    fn_1_7090(managerObject);
    fn_1_7AB8(managerObject);
    param = lbl_1_data_280;
    while (param->count != -1) {
        fn_1_65FC(process, param);
        param++;
    }
}

extern s32 lbl_1_data_F18;
extern float lbl_1_bss_1BC;
extern s8 lbl_1_bss_218, lbl_1_bss_219, lbl_1_data_2758;
s32 fn_1_367CC(s32 *player);

void fn_1_65C0(void)
{
}

void fn_1_65D4(OMOBJ *obj);

void fn_1_65C4(OMOBJ *obj)
{
    obj->objFunc = fn_1_65D4;
}

void fn_1_8374(OMOBJ *object);

void fn_1_65D4(OMOBJ *object)
{
    fn_1_8374(object);
}

/* Called by fn_1_64A8 for each prop definition; creates its objects, models,
 * motions, initial transforms, render layer, and requested shadow behavior. */
void fn_1_65FC(HUPROCESS *process, M620PropParam *param)
{
    s32 i;
    OMOBJ *object;
    s32 model;
    M620PropWork *work;
    M620PropState *state;
    M620PropMotionParam *motion;
    s32 group;
    s32 resourceIndex;

    for (group = 0; group < param->count; group++) {
        object = omAddObjEx(process, 12, 1, 4, 0, param->update);
        fn_1_68A0(object);
        work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M620PropWork), HU_MEMNUM_OVL);
        object->data = work;
        memset(work, 0, sizeof(M620PropWork));
        object->stat |= 0x100;
        state = work;
        work->param = param;
        work->id = param->firstId + group;
        state->motionState.motion = -1;
        for (i = 0; i < 1; i++) {
            object->mdlId[i] = -1;
        }
        for (i = 0; i < 4; i++) {
            object->mtnId[i] = -1;
        }
        if (param->linkModel != 0) {
            model = fn_1_4408(fn_1_FB8(param->modelResourceIndex));
        } else {
            model = (s16)Hu3DModelCreate(HuDataSelHeapReadNum(fn_1_FB8(param->modelResourceIndex),
                                                             HU_MEMNUM_OVL, HEAP_MODEL));
        }
        object->mdlId[0] = model;
        motion = param->motions;
        if (motion != NULL) {
            for (i = 0; i < 4; i++, motion++) {
                resourceIndex = motion->resourceIndex;
                if (resourceIndex == -2) {
                    break;
                }
                if (resourceIndex != -1) {
                    object->mtnId[i] = fn_1_4510(model, fn_1_FB8(resourceIndex));
                }
            }
        }
        if (param->position != NULL) {
            state->position = *param->position;
        }
        work->startPosition = state->position;
        state->rotation.y = param->rotationY;
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

/* Registers a newly created prop in the first free slot for later lookup. */
void fn_1_68A0(OMOBJ *object)
{
    OMOBJ **entry = lbl_1_bss_448;
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

/* Finds the first registered prop with the requested type and instance ID. */
OMOBJ *fn_1_68F4(s32 type, s32 id)
{
    OMOBJ *object;
    OMOBJ **entry = lbl_1_bss_448;
    M620PropWork *work;
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

/* Selects a prop motion when its requested index changes; a missing motion
 * clears the completion-tracking flag, while an existing motion starts or blends in. */
void fn_1_697C(OMOBJ *object, s32 index)
{
    M620PropState *state;
    M620PropMotionParam *param;
    M620PropWork *work = object->data;
    s32 model, motion;

    state = work;
    if (state->motionState.motion != index && index < 4) {
        motion = object->mtnId[index];
        work->timer = 0;
        state->motionState.motionTime = 0;
        if (motion == -1) {
            state->motionState.motionActive = 0;
            return;
        }
        param = &work->param->motions[index];
        model = object->mdlId[0];
        if (state->motionState.motion < 0 || 0.0 == param->blend) {
            Hu3DMotionSet(model, motion);
            Hu3DModelAttrSet(model, param->attr);
        } else {
            Hu3DMotionShiftSet(model, motion, 0.0f, 60.0f * param->blend, param->attr);
        }
        state->motionState.motion = index;
    }
}
/* Reports completion only after the main motion ends and no shift is pending. */
BOOL fn_1_6A88(OMOBJ *actor)
{
    s32 model_id;

    model_id = actor->mdlId[0];
    if (Hu3DMotionEndCheck((HU3D_MODELID) model_id) != 0 &&
        Hu3DMotionShiftIDGet((HU3D_MODELID) model_id) < 0) {
        return TRUE;
    }
    return FALSE;
}

/* Copy a named prop part's current transform, adding 90 degrees to its Y rotation;
 * return whether the prop's main motion is still running. */
BOOL fn_1_6AF0(s32 type, s32 id, char *name, Vec *position, Vec *rotation)
{
    OMOBJ *object;
    M620PropWork *work;
    HSF_OBJECT *part;
    s32 model;

    object = fn_1_68F4(type, id);
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

extern void fn_1_6C64(OMOBJ *object);
extern void fn_1_6D20(OMOBJ *object);

/* Prop update callback: applies its transform and advances motion completion. */
void fn_1_6C2C(OMOBJ *object)
{
    fn_1_6C64(object);
    fn_1_6D20(object);
}

typedef M620PropState M620ModelTransform;

/* Applies the stored position and rotation to the prop model each update. */
void fn_1_6C64(OMOBJ *object)
{
    M620ModelTransform *work = object->data;
    M620ModelTransform *state = work;
    s32 modelId = object->mdlId[0];

    state->rotation.y = fmodf(state->rotation.y, 360.0f);
    Hu3DModelPosSetV((s16)modelId, &state->position);
    Hu3DModelRotSetV((s16)modelId, &state->rotation);
}

/* Tracks motion completion for an active prop animation and advances its timer. */
void fn_1_6D20(OMOBJ *object)
{
    M620PropWork *work = object->data;
    M620PropState *state = work;

    if (state->motionState.motionActive) {
        if (fn_1_6A88(object)) {
            state->motionState.motionActive = 0;
            state->motionState.motionTimer = 0;
        }
    } else {
        state->motionState.motionTimer = 0;
    }
    work->timer++;
}

void fn_1_6DCC(void)
{
}

typedef struct M620ConfigurationPrefix {
    u32 header[8];
    Vec center;
    f32 radiusX;
    f32 radiusZ;
    s32 direction;
} M620ConfigurationPrefix;

typedef u32 M620ConfigurationBlock[16];

extern u32 lbl_1_bss_338[16];
const M620ConfigurationPrefix lbl_1_rodata_1B0 = {
    {1, 4, 2, 0, 11, 0, 0, 0}, {0.0f, 0.0f, 0.0f}, 580.0f, 580.0f, 1
};
extern s32 fn_1_FFCC(void *value);
extern void fn_1_6EB0(OMOBJ *object);

/* Initializes the shared configuration block when the first prop group starts. */
void fn_1_6DD0(OMOBJ *object)
{
    void *objectData = object->data;
    M620ConfigurationBlock *destination = (M620ConfigurationBlock *)lbl_1_bss_338;

    memset(destination, 0, 64);
    *(M620ConfigurationPrefix *)destination =
        lbl_1_rodata_1B0;
    fn_1_FFCC(*destination++);
    object->objFunc = fn_1_6EB0;
}

void fn_1_6EB0(OMOBJ *object)
{
}

extern s32 fn_1_FB8(s32 resource);

extern void fn_1_6F84(OMOBJ *object);

/* Creates the board water effect and installs its texture-scroll callback. */
void fn_1_6EB4(OMOBJ *object)
{
    M620WaterWork *work = object->data;
    ANIMDATA *animation;

    animation = HuSprAnimRead(HuDataSelHeapReadNum(
        fn_1_FB8(M620_WATER_ANIMATION_INDEX), HU_MEMNUM_OVL, HEAP_MODEL));
    work->appearance.water.textureOffsetX = work->appearance.water.textureOffsetY = 0.0f;
    Hu3DWaterCreate(2, animation, 0, 0, FALSE,
                    lbl_1_data_4E8,
                    lbl_1_data_4E8 + 1);
    Hu3DWaterPadYSet((-15.0f));
    Hu3DWaterGlowSet(8, &lbl_1_data_500);
    Hu3DWaterIndTexMtxSet(lbl_1_data_504);
    object->objFunc = fn_1_6F84;
}

/* Scrolls the water texture a small amount on each object update. */
void fn_1_6F84(OMOBJ *object)
{
    M620WaterWork *work = object->data;

    work->appearance.water.textureOffsetX += 0.001;
    work->appearance.water.textureOffsetY += 0.0003;
    Hu3DWaterTexPosSet(work->appearance.water.textureOffsetX, work->appearance.water.textureOffsetY,
                       0.0f);
}

void fn_1_7018(OMOBJ *object);

void fn_1_7008(OMOBJ *object)
{
    object->objFunc = fn_1_7018;
}

/* Removes the configured model attributes once the sequence reaches phase 7. */
void fn_1_7018(OMOBJ *object)
{
    M620PropWork *work = object->data;
    s32 modelId;

    if (work->phase == 0 && MgSeqModeGet() == 7) {
        modelId = object->mdlId[0];
        Hu3DModelAttrReset(modelId, HU3D_MOTATTR_PAUSE);
        work->phase++;
    }
}

extern OMOBJ *lbl_1_bss_438[4];

/* Clears the four launch-prop registry slots during prop setup. */
void fn_1_7090(OMOBJ *object)
{
    s32 index;

    index = 0;
    while (index < 4) {
        lbl_1_bss_438[index] = NULL;
        index += 1;
    }
}

/* Store a player's launch prop in the first free shared registry slot. */
void fn_1_70D0(void *object)
{
    s32 index;

    for (index = 0; index < 4; index++) {
        if (lbl_1_bss_438[index] == NULL) {
            lbl_1_bss_438[index] = object;
            break;
        }
    }
}

extern s32 fn_1_E570(s32, s32, f32, s32, s32, s32);
extern void fn_1_73A0(OMOBJ *object);

/* Starts a player's launch sequence, creates its effects, and registers the
 * object for the shared launch manager. */
void fn_1_7130(OMOBJ *object)
{
    M620PlayerLaunchWork *work = object->data;
    M620PlayerLaunchWork *setupWork = work;
    M620PlayerLaunchSetup *setup;
    s32 playerIndex = work->id & 3;
    s32 savedModelId = object->mdlId[0];
    Vec positionCopy;
    s32 index;

    setupWork->position = lbl_1_data_52C[playerIndex];
    setupWork->rotation.y = lbl_1_data_51C[playerIndex];
    work->appearance.destination = lbl_1_data_55C[playerIndex];

    positionCopy = setupWork->position;
    positionCopy.y += 260.0;
    work->firstEffect = fn_1_E570((s32)&positionCopy,
                                  (s32)&work->appearance.destination,
                                  800.0f, playerIndex, 1, playerIndex);

    work->target = lbl_1_data_58C[playerIndex];
    positionCopy = lbl_1_data_5BC[playerIndex];
    work->secondEffect = fn_1_E570((s32)&work->target,
                                   (s32)&positionCopy,
                                   800.0f, playerIndex, 0, -1);

    setup = &setupWork->collision.launch;
    setup->flags = 1;
    setup->mask = 8;
    setup->unknown_08 = 3;
    setup->unknown_0C = playerIndex;
    setup->state = 10;
    setup->primaryRelation = object;
    setup->invoke = 0;
    setup->position = &setupWork->position;
    setup->lowerHeight = -10.0f;
    setup->heightSpan = (f32)(260.0 - (f64)setup->lowerHeight);
    setup->innerRadius = 170.0f;
    setup->radialThickness = 40.0f;
    fn_1_FFCC(setup);

    for (index = 0; index < 4; index++) {
        if (lbl_1_bss_438[index] == NULL) {
            lbl_1_bss_438[index] = object;
            break;
        }
    }
    object->objFunc = fn_1_73A0;
}

extern void fn_1_E6D0(s32 effectIndex, Vec *position, Vec *adjustedPosition);
extern void fn_1_E8A8(s32 effectIndex, f32 blend);
extern void fn_1_EB28(s32 effectIndex, f32 amount);
extern void fn_1_B24(void);
extern void fn_1_DD18(const Vec *origin, f32 radius, s32 count);
extern void fn_1_63A0(s32 playerIndex, Vec *position);
extern void fn_1_6434(s32 playerIndex);
extern void fn_1_E640(s32 effectIndex, const Vec *position);
/* Update the launch prop and its trails through release, descent, landing,
 * and the wait for result selection. */
void fn_1_73A0(OMOBJ *object)
{
    M620PlayerLaunchWork *work = object->data;
    M620PlayerLaunchWork *setupWork = work;
    s32 playerIndex = work->id & 3;
    Vec characterPosition;
    Vec effectPosition;
    s32 modelId;

    switch (work->phase) {
    case 0:
        work->phase++;
        work->launchTimer = 0;
    case 1:
        fn_1_E6D0(work->firstEffect, &work->appearance.destination, &effectPosition);
        fn_1_63A0(work->id & 3, &characterPosition);
        fn_1_E6D0(work->secondEffect, &characterPosition, &effectPosition);
        /* The extra 0.2 argument is unused; this helper fully straightens the interior samples. */
        fn_1_E8A8(work->secondEffect, 0.2f);
        if (work->action.launchRequested == 0) {
            break;
        }
        work->phase++;
        work->launchTimer = 0;
        work->target.y += 50.0;
        fn_1_EB28(work->firstEffect, 100.0f);
    case 2:
        if (MgSeqModeGet() == 2) {
            work->target.y += 30.0;
            if (++work->launchTimer < 21) {
                break;
            }
        }
        work->phase++;
        work->launchTimer = 0;
        work->descentSoundPlayed = 0;
    case 3:
        work->target.y += 30.0;
        setupWork->velocity.y = setupWork->velocity.y - 0.5448138888888889;
        setupWork->position.y += setupWork->velocity.y;
        if (work->descentSoundPlayed == 0 && setupWork->position.y < 700.0) {
        HuAudFXPlay(M620_PLAYER_DESCENT_SOUND_ID);
            work->descentSoundPlayed = 1;
        }
        if (setupWork->position.y > 0.0) {
            break;
        }
        work->phase++;
        work->launchTimer = 0;
        setupWork->position.y = 0.0f;
        setupWork->velocity.y = 0.0f;
        work->action.launchRequested = 0;
        fn_1_B24();
        fn_1_DD18(&setupWork->position, 200.0f, 40);
        fn_1_6434(playerIndex);
        HuAudFXPlay(M620_PLAYER_LANDING_SOUND_ID);
    case 4:
        if (++work->launchTimer < 18) {
            break;
        }
        work->selection.selectionReady = 1;
        if (lbl_1_bss_3A8 != 0) {
            modelId = object->mdlId[0];
            Hu3DModelAttrReset(modelId, HU3D_MOTATTR_PAUSE);
            work->phase++;
            work->launchTimer = 0;
            if (!playerIndex) {
                HuAudFXPlay(M620_FIRST_PLAYER_READY_SOUND_ID);
            }
        }
        if (work->action.launchRequested != 0) {
            work->phase = 1;
            work->launchTimer = 0;
            work->action.launchRequested = 0;
            work->selection.selectionReady = 0;
            setupWork->position = lbl_1_data_52C[playerIndex];
            work->appearance.destination = lbl_1_data_55C[playerIndex];
            work->target = lbl_1_data_58C[playerIndex];
        }
        break;
    case 5:
        break;
    }
    characterPosition = setupWork->position;
    characterPosition.y += 260.0;
    fn_1_E640(work->firstEffect, &characterPosition);
    fn_1_E640(work->secondEffect, &work->target);
    fn_1_6C64(object);
    fn_1_6D20(object);
}

extern void fn_1_78C0(OMOBJ *object);

/* Sets the motion-off attribute and installs this prop's transform callback. */
void fn_1_7868(OMOBJ *object)
{
    void *work = object->data;
    s32 modelId = object->mdlId[0];

    Hu3DModelAttrSet(modelId, HU3D_ATTR_MOTION_OFF);
    object->objFunc = fn_1_78C0;
}

/* Each update copies the paired prop's position, halves its height, and advances this prop. */
void fn_1_78C0(OMOBJ *object)
{
    M620PropWork *work = object->data;
    OMOBJ *otherObject;
    M620PropWork *otherWork;
    M620ModelTransform *otherState;
    M620PropWork *rotationWork;
    s32 pairedPropId;

    rotationWork = work;
    pairedPropId = work->id;
    otherObject = fn_1_68F4(1, pairedPropId);
    if (otherObject != NULL) {
        otherWork = otherObject->data;
        otherState = otherWork;
        rotationWork->position = otherState->position;
        rotationWork->position.y *= 0.5;
        fn_1_6C64(object);
        fn_1_6D20(object);
    }
}

extern s32 lbl_1_bss_378;
extern s32 lbl_1_bss_37C;
extern s32 lbl_1_bss_380;
extern s32 lbl_1_bss_384;
extern s32 lbl_1_bss_394[4];
extern OMOBJ *lbl_1_bss_3AC[35];

/* Clears the prop registry and shared selection and sound timers during setup. */
void fn_1_7AB8(OMOBJ *object)
{
    s32 index;

    index = 0;
    while (index < 35) {
        lbl_1_bss_3AC[index] = NULL;
        index += 1;
    }
    index = 0;
    while (index < 4) {
        lbl_1_bss_394[index] = 0;
        index += 1;
    }
    lbl_1_bss_384 = 0;
    lbl_1_bss_380 = 0;
    lbl_1_bss_37C = 0;
    lbl_1_bss_378 = 0;
}

/* Adds a selectable prop to the first free slot in the shared prop registry. */
void fn_1_7B64(void *object)
{
    s32 index;

    for (index = 0; index < 35; index++) {
        if (lbl_1_bss_3AC[index] == NULL) {
            lbl_1_bss_3AC[index] = object;
            break;
        }
    }
}

typedef M620PropMotionParam M620EffectMotionParam;

typedef M620PropParam M620EffectObjectParam;

extern s32 fn_1_B534(s32 playerIndex);

extern M620PlayerLaunchSetup *fn_1_10118(s32 type, s32 playerIndex);
void Hu3DModelShadowMapSet(s16 modelId);
void Hu3DModelShadowMapReset(s16 modelId);

extern float fn_1_488C(Vec *first, Vec *second);

/* Returns the horizontal distance between two board positions. */
static inline float distanceXZInline(Vec *first, Vec *second)
{
    float x;
    float z;

    x = second->x - first->x;
    z = second->z - first->z;
    return sqrtf(x * x + z * z);
}

/* Update shadow-map reception for registered props near the four launch props. */
void fn_1_7BC4(void)
{
    Vec *playerPositions[4];
    Vec *objectPosition;
    Vec *playerPosition;
    f32 distanceLimit;
    f32 distance;
    s32 playerIndex;
    s32 objectIndex;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        M620PlayerLaunchSetup *player = fn_1_10118(3, playerIndex);

        if (player == NULL) {
            return;
        }
        {
            M620PlayerLaunchSetup *playerWork = player;

            playerPositions[playerIndex] = playerWork->position;
            if (playerIndex == 0) {
                distanceLimit = (f32)(40.0 +
                    (playerWork->innerRadius + playerWork->radialThickness));
            }
        }
    }

    for (objectIndex = 0; objectIndex < 35; objectIndex++) {
        OMOBJ *object = lbl_1_bss_3AC[objectIndex];

        if (object != NULL) {
            M620EffectMotionWork *work = object->data;
            M620EffectMotionWork *positionWork = work;
            s32 playerFound;
            s32 modelId;

            objectPosition = &positionWork->position;
            playerFound = 0;
            modelId = object->mdlId[0];

            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                f32 deltaX;
                f32 deltaZ;

                playerPosition = playerPositions[playerIndex];
                deltaX = objectPosition->x - playerPosition->x;
                deltaZ = objectPosition->z - playerPosition->z;
                distance = sqrtf((deltaX * deltaX) + (deltaZ * deltaZ));
                if (distance < distanceLimit) {
                    playerFound = 1;
                }
            }

            if (playerFound != 0) {
                if (work->shadowActive == 0) {
                    Hu3DModelShadowMapSet(modelId);
                    work->shadowActive = 1;
                }
            } else if (work->shadowActive != 0) {
                Hu3DModelShadowMapReset(modelId);
                work->shadowActive = 0;
            }
        }
    }
}

/* Assigns each selectable prop to the first player whose ready launch prop
 * contains it within the horizontal selection radius. */
void fn_1_7E98(void)
{
    M620EffectMotionWork *selectionWork;
    f32 distanceLimit;
    s32 objectIndex;

    for (objectIndex = 0; objectIndex < 35; objectIndex++) {
        OMOBJ *object = lbl_1_bss_3AC[objectIndex];

        if (object != NULL) {
            M620EffectMotionWork *work = object->data;
            s32 playerIndex;

            selectionWork = work;
            work->action.movementMode = -1;
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                if (fn_1_B534(playerIndex) != 0) {
                    OMOBJ *player = lbl_1_bss_438[playerIndex];

                    if (player != NULL) {
                        M620PlayerLaunchWork *playerData = player->data;
                        M620PlayerLaunchWork *playerWork = playerData;
                        f32 deltaX;
                        f32 deltaZ;

                        distanceLimit = (f32)((f64)playerWork->collision.launch.innerRadius +
                            (0.5 * (f64)playerWork->collision.launch.radialThickness));
                        deltaX = selectionWork->position.x - playerWork->position.x;
                        deltaZ = selectionWork->position.z - playerWork->position.z;

                        if (sqrtf((deltaX * deltaX) + (deltaZ * deltaZ)) <
                            distanceLimit) {
                            work->action.movementMode = 0;
                            work->selection.playerIndex = playerIndex;
                            break;
                        }
                    }
                }
            }
        }
    }
}

/* Tick player cooldowns and start each ready player's assigned prop nearest its result
 * destination. */
s32 fn_1_80F8(void)
{
    OMOBJ *object;
    s32 didWork = 0;
    s32 playerIndex;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_1_bss_394[playerIndex] != 0) {
            lbl_1_bss_394[playerIndex]--;
            didWork = 1;
        } else if (fn_1_B534(playerIndex) != 0) {
            f32 distance;
            M620EffectMotionWork *work;
            M620EffectMotionWork *bestWork = NULL;
            f32 bestDistance = 100000.0f;
            s32 objectIndex;

            for (objectIndex = 0; objectIndex < 35; objectIndex++) {
                object = lbl_1_bss_3AC[objectIndex];

                if (object != NULL) {
                    work = object->data;

                    if (work->action.movementMode == 0 &&
                        work->selection.playerIndex == playerIndex) {
                        distance = fn_1_488C(
                            &work->position, &lbl_1_data_5EC[playerIndex]);

                        if (distance < bestDistance) {
                            bestDistance = distance;
                            bestWork = work;
                        }
                    }
                }
            }

            if (bestWork != NULL) {
                bestWork->action.movementMode = 1;
                lbl_1_bss_394[playerIndex] = bestWork->pointValue - 1;
                didWork = 1;
            }
        }
    }
    return didWork;
}
extern Vec lbl_1_bss_388;
/* Seeds the shared position from the first prop, then half-averages it with each later prop;
* each later prop gets half the current weight, so later entries outweigh earlier ones. */
void fn_1_8270(void)
{
    OMOBJ *object;
    s32 foundCount = 0;
    s32 index;

    for (index = 0; index < 35; index++) {
        object = lbl_1_bss_3AC[index];

        if (object != NULL) {
            M620PropState *work = object->data;
            M620PropState *position = work;

            if (foundCount == 0) {
                lbl_1_bss_388 = position->position;
            } else {
                lbl_1_bss_388.x = (f32)(0.5 *
                    (f64)(lbl_1_bss_388.x + position->position.x));
                lbl_1_bss_388.z = (f32)(0.5 *
                    (f64)(lbl_1_bss_388.z + position->position.z));
            }
            foundCount++;
        }
    }
}

typedef struct M620SelectionManager {
    s32 phase;
    s32 timer;
} M620SelectionManager;

extern s32 fn_1_FD0(void);
extern void fn_1_7BC4(void);

/* Returns the X/Z-plane distance from a player to one selectable prop. */
static inline f32 m620_selection_distance(Vec *playerPosition,
                                          M620EffectMotionWork *selection)
{
    f32 deltaX, deltaZ;

    deltaX = selection->position.x - playerPosition->x;
    deltaZ = selection->position.z - playerPosition->z;
    return sqrtf(deltaX * deltaX + deltaZ * deltaZ);
}

/* Tick player cooldowns and start each ready player's assigned prop nearest its result
 * destination. */
static inline s32 m620_advance_selected_players(void)
{
    OMOBJ *object;
    s32 didWork = 0;
    s32 playerIndex;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_1_bss_394[playerIndex] != 0) {
            lbl_1_bss_394[playerIndex]--;
            didWork = 1;
        } else if (fn_1_B534(playerIndex) != 0) {
            f32 distance;
            M620EffectMotionWork *work;
            M620EffectMotionWork *bestWork = NULL;
            f32 bestDistance = 100000.0f;
            s32 objectIndex;

            for (objectIndex = 0; objectIndex < 35; objectIndex++) {
                object = lbl_1_bss_3AC[objectIndex];

                if (object != NULL) {
                    work = object->data;

                    if (work->action.movementMode == 0 &&
                        work->selection.playerIndex == playerIndex) {
                        distance = fn_1_488C(
                            &work->position, &lbl_1_data_5EC[playerIndex]);

                        if (distance < bestDistance) {
                            bestDistance = distance;
                            bestWork = work;
                        }
                    }
                }
            }

            if (bestWork != NULL) {
                bestWork->action.movementMode = 1;
                lbl_1_bss_394[playerIndex] = bestWork->pointValue - 1;
                didWork = 1;
            }
        }
    }
    return didWork;
}

/* Seeds the shared position from the first prop, then half-averages it with each later prop;
* each later prop gets half the current weight, so later entries outweigh earlier ones. */
static inline void m620_update_shared_position(void)
{
    OMOBJ *object;
    M620PropState *work;
    s32 foundCount = 0;
    s32 index;

    for (index = 0; index < 35; index++) {
        object = lbl_1_bss_3AC[index];

        if (object != NULL) {
            M620PropState *position;
            work = object->data;
            position = work;

            if (foundCount == 0) {
                lbl_1_bss_388 = position->position;
            } else {
                lbl_1_bss_388.x = (f32)(0.5 *
                    (f64)(lbl_1_bss_388.x + position->position.x));
                lbl_1_bss_388.z = (f32)(0.5 *
                    (f64)(lbl_1_bss_388.z + position->position.z));
            }
            foundCount++;
        }
    }
}

/* Assigns each selectable prop to the first player whose ready launch prop
 * contains it within the horizontal selection radius. */
static inline void m620_select_players(void)
{
    M620EffectMotionWork *selectionWork;
    f32 distanceLimit;
    s32 objectIndex;

    for (objectIndex = 0; objectIndex < 35; objectIndex++) {
        OMOBJ *object = lbl_1_bss_3AC[objectIndex];

        if (object != NULL) {
            M620EffectMotionWork *work = object->data;
            s32 playerIndex;

            selectionWork = work;
            work->action.movementMode = -1;
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                if (fn_1_B534(playerIndex) != 0) {
                    OMOBJ *player = lbl_1_bss_438[playerIndex];

                    if (player != NULL) {
                        M620PlayerLaunchWork *playerData = player->data;
                        M620PlayerLaunchWork *playerWork = playerData;
                        f32 deltaX;
                        f32 deltaZ;

                        distanceLimit = (f32)((f64)playerWork->collision.launch.innerRadius +
                            (0.5 * (f64)playerWork->collision.launch.radialThickness));
                        deltaX = selectionWork->position.x - playerWork->position.x;
                        deltaZ = selectionWork->position.z - playerWork->position.z;

                        if (sqrtf((deltaX * deltaX) + (deltaZ * deltaZ)) <
                            distanceLimit) {
                            work->action.movementMode = 0;
                            work->selection.playerIndex = playerIndex;
                            break;
                        }
                    }
                }
            }
        }
    }
}

/* Runs the prop-selection phases, updates nearby shadows, and ticks shared cooldowns. */
void fn_1_8374(OMOBJ *object)
{
    M620SelectionManager *work = object->data;
    OMOBJ *playerObject;
    switch (work->phase)
    {
    case 0 :
        work->phase++;
    case 1 :
        if (fn_1_FD0() != 0)
        {
            m620_select_players();
            work->phase++;
            work->timer = 60;
        case 2 :
            if (--work->timer <= 0)
            {
                work->phase++;
                work->timer = 48;
                lbl_1_bss_3A8 = 1;
            case 3 :
                if (--work->timer <= 0)
                {
                    work->timer = 72;
                    if (! (m620_advance_selected_players()))
                    {
                        work->phase++;
                        work->timer = 120;
                    case 4 :
                        if (--work->timer <= 0)
                        {
                            work->phase++;
                            lbl_1_bss_3A4 = 1;
                        }
                    }
                }
            }
        }
        break;
    }
    fn_1_7BC4();
    m620_update_shared_position();
    if (lbl_1_bss_384 != 0)
    {
        lbl_1_bss_384--;
    }
    if (lbl_1_bss_380 != 0)
    {
        lbl_1_bss_380--;
    }
    if (lbl_1_bss_37C)
    {
        lbl_1_bss_37C--;
    }
    if (lbl_1_bss_378 != 0)
    {
        lbl_1_bss_378--;
    }
}

/* Starts the prop's slow movement sound when the shared 12-frame timer is clear. */
void fn_1_89BC(M620ModelTransform *object)
{
    if (lbl_1_bss_384 == 0) {
        lbl_1_bss_384 = 12;
        fn_1_35E0(MSM_SE_GUIDE_55, &object->position);
    }
}

/* Starts the prop's fast movement sound when the shared 6-frame timer is clear. */
void fn_1_8A10(M620ModelTransform *object)
{
    if (lbl_1_bss_380 == 0) {
        lbl_1_bss_380 = 6;
        fn_1_35E0(MSM_SE_GUIDE_56, &object->position);
    }
}

/* Plays the state-specific launch sound when its shared 45-frame cooldown is clear. */
void fn_1_8A64(OMOBJ *object)
{
    M620EffectMotionWork *work = object->data;

    if (work->pointValue == 1) {
        if (lbl_1_bss_37C == 0) {
            lbl_1_bss_37C = 45;
            HuAudFXPlay(MSM_SE_GUIDE_57);
        }
    } else if (lbl_1_bss_378 == 0) {
        lbl_1_bss_378 = 45;
        HuAudFXPlay(M620_ALTERNATE_LAUNCH_SOUND_ID);
    }
}

extern void fn_1_B254(OMOBJ *object);
extern s32 fn_1_B3A0(M620B3A0Controller *controller,
                     M620B3A0OtherCollision *otherCollision);
extern s32 fn_1_FF54(void *value);
extern void fn_1_E4E0(void *effect);
extern void fn_1_8FC8(OMOBJ *object);
/* Starts a configured effect, registers it, and installs its frame update callback. */
void fn_1_8B00(OMOBJ *object)
{
    M620EffectMotionWork *work = object->data;
    M620EffectMotionWork *setupOwner = work;
    M620EffectSetup *setup;
    s32 propId = work->id;
    s32 savedModelId = object->mdlId[0];
    M620EffectMotionWork *motionWork;
    M620EffectMotionWork *motionState;
    s32 index;

    work->pointValue = 1;
    work->burstEmissionAccumulator = 0.0f;

    setup = &setupOwner->collision.effect;
    setup->flags = 0;
    setup->mask = -1;
    setup->unknown_08 = 1;
    setup->unknown_0C = propId;
    setup->state = 0;
    setup->object = object;
    setup->update = fn_1_B254;
    setup->collisionCallback = fn_1_B3A0;
    setup->positionSource = &setupOwner->position;
    setup->offsetSource = &setupOwner->velocity;
    setup->positionCopy = *(Vec *)setup->positionSource;
    setup->unknown_38 = 40.0f;
    setup->unknown_3C = 1.0f;
    fn_1_FF54(setup);

    work->ringActive = 1;
    work->ringBlendMode = 0;
    work->ringRadius = 45.0f;
    work->ringPosition = &setupOwner->position;
    fn_1_E4E0(&work->ringActive);

    motionWork = object->data;
    motionState = motionWork;
    if (motionState->motionState.motion != 2) {
        s32 motion = object->mtnId[2];

        motionWork->timer = 0;
        motionState->motionState.motionTime = 0;
        if (motion == -1) {
            motionState->motionState.motionActive = 0;
        } else {
            M620EffectMotionParam *param =
                &motionWork->param->motions[2];
            s32 modelId = object->mdlId[0];

            if (motionState->motionState.motion < 0 || param->blend == 0.0) {
                Hu3DMotionSet(modelId, motion);
                Hu3DModelAttrSet(modelId, param->attr);
            } else {
                Hu3DMotionShiftSet(modelId, motion, 0.0f,
                                   60.0f * param->blend,
                                   param->attr);
            }
            motionState->motionState.motion = 2;
        }
    }

    for (index = 0; index < 35; index++) {
        if (lbl_1_bss_3AC[index] == NULL) {
            lbl_1_bss_3AC[index] = object;
            break;
        }
    }
    object->objFunc = fn_1_8FC8;
}

/* Initialize the alternate prop with a three-point value, additive ring blending, and a larger
 * ring. */
void fn_1_8D4C(OMOBJ *object)
{
    M620EffectMotionWork *finalWork = object->data;
    M620EffectMotionWork *work = object->data;
    M620EffectMotionWork *setupOwner = work;
    M620EffectMotionWork *motionWork;
    M620EffectMotionWork *motionState;
    M620EffectSetup *setup;
    M620EffectMotionParam *param;
    s32 propId = work->id;
    s32 savedModelId = object->mdlId[0];
    s32 motion;
    s32 modelId;
    s32 index;

    work->pointValue = 1;
    work->burstEmissionAccumulator = 0.0f;

    setup = &setupOwner->collision.effect;
    setup->flags = 0;
    setup->mask = -1;
    setup->unknown_08 = 1;
    setup->unknown_0C = propId;
    setup->state = 0;
    setup->object = object;
    setup->update = fn_1_B254;
    setup->collisionCallback = fn_1_B3A0;
    setup->positionSource = &setupOwner->position;
    setup->offsetSource = &setupOwner->velocity;
    setup->positionCopy = *(Vec *)setup->positionSource;
    setup->unknown_38 = 40.0f;
    setup->unknown_3C = 1.0f;
    fn_1_FF54(setup);

    work->ringActive = 1;
    work->ringBlendMode = 0;
    work->ringRadius = 45.0f;
    work->ringPosition = &setupOwner->position;
    fn_1_E4E0(&work->ringActive);

    motionWork = object->data;
    motionState = motionWork;
    if (motionState->motionState.motion != 2) {
        motion = object->mtnId[2];

        motionWork->timer = 0;
        motionState->motionState.motionTime = 0;
        if (motion == -1) {
            motionState->motionState.motionActive = 0;
        } else {
            param = &motionWork->param->motions[2];
            modelId = object->mdlId[0];

            if (motionState->motionState.motion < 0 || param->blend == 0.0) {
                Hu3DMotionSet(modelId, motion);
                Hu3DModelAttrSet(modelId, param->attr);
            } else {
                Hu3DMotionShiftSet(modelId, motion, 0.0f,
                                   60.0f * param->blend,
                                   param->attr);
            }
            motionState->motionState.motion = 2;
        }
    }

    for (index = 0; index < 35; index++) {
        if (lbl_1_bss_3AC[index] == NULL) {
            lbl_1_bss_3AC[index] = object;
            break;
        }
    }
    object->objFunc = fn_1_8FC8;
    finalWork->pointValue = 3;
    finalWork->ringBlendMode = 1;
    finalWork->ringRadius *= 1.6;
}

extern s32 fn_1_9AB4(OMOBJ *object);
extern s32 fn_1_9F88(OMOBJ *object);
extern s32 fn_1_A618(OMOBJ *object);
extern s32 fn_1_A95C(OMOBJ *object);
extern f32 fn_1_4620(f32 start, f32 end, f32 factor);

extern void fn_1_E1F4(const Vec *origin, f32 radius, s32 count);

/* Advances the board effect's movement phases and updates its motion, sounds, and dust. */
void fn_1_8FC8(OMOBJ *object)
{
    M620EffectMotionWork *work = object->data;
    M620EffectMotionWork *state = work;
    s32 model = object->mdlId[0];
    f32 velocityX;
    f32 velocityZ;
    f32 speed;

    switch (work->phase) {
    case 0:
        if (fn_1_9AB4(object)) {
            work->phase++;
            state->motionState.phase = 0;
        }
        break;
    case 1:
        if (!fn_1_9F88(object)) {
            break;
        }
        work->phase++;
        state->motionState.phase = 0;
        /* fall through */
    case 2:
        state->velocity.x *= 0.9;
        state->velocity.z *= 0.9;
        switch (work->action.movementMode) {
        case -1:
            work->phase = 3;
            state->motionState.phase = 0;
            break;
        case 0:
            break;
        case 1:
            work->phase = 4;
            state->motionState.phase = 0;
            break;
        }
        break;
    case 3:
        if (fn_1_A618(object)) {
            work->phase++;
            state->motionState.phase = 0;
        }
        break;
    case 4:
        if (fn_1_A95C(object)) {
            work->phase++;
            state->motionState.phase = 0;
        }
        break;
    }

    velocityX = state->velocity.x;
    velocityZ = state->velocity.z;
    speed = sqrtf((velocityX * velocityX) + (velocityZ * velocityZ));
    state->rotation.y = fn_1_4620(state->rotation.y, ((f32)(180.0 *
        (atan2((f64)velocityX, (f64)velocityZ) / 3.141592653589793))),
        (f32)(0.05 * (f64)speed));

    if (state->motionState.motionActive == 0) {
        if (state->motionState.motion == 0) {
            if (speed > 0.08333333333333333) {
                if (speed < 1.6666666666666667) {
                    fn_1_697C(object, 2);
                } else {
                    fn_1_697C(object, 3);
                }
            }
        } else if (speed < 0.03333333333333333) {
            fn_1_697C(object, 0);
        } else if (state->motionState.motion == 2) {
            if (speed > 2.0) {
                fn_1_697C(object, 3);
            }
        } else if (speed < 1.6666666666666667) {
            fn_1_697C(object, 2);
        }
    }

    if (PSVECMag(&state->position) < 1200.0) {
        if (state->motionState.motion == 2) {
            state->motionState.movementSoundDistance += PSVECMag(&state->velocity);
            if (state->motionState.movementSoundDistance > 30.0) {
                state->motionState.movementSoundDistance -= 30.0;
                if (lbl_1_bss_384 == 0) {
                    lbl_1_bss_384 = 12;
                    fn_1_35E0(MSM_SE_GUIDE_55, &state->position);
                }
            }
        }
        if (state->motionState.motion == 3) {
            state->motionState.movementSoundDistance += PSVECMag(&state->velocity);
            if (state->motionState.movementSoundDistance > 100.0) {
                state->motionState.movementSoundDistance -= 100.0;
                if (lbl_1_bss_380 == 0) {
                    lbl_1_bss_380 = 6;
                    fn_1_35E0(MSM_SE_GUIDE_56, &state->position);
                }
            }
        }
    }
    PSVECAdd(&state->position, &state->velocity, &state->position);
    if (work->pointValue != 1) {
        if ((work->burstEmissionAccumulator += 0.7) >= 1.0) {
            work->burstEmissionAccumulator -= 1.0;
            if ((Hu3DModelAttrGet(model) & 1) == 0) {
                Vec particlePosition = state->position;
                particlePosition.y += 50.0;
                particlePosition.y = (f32)((f64)particlePosition.y +
                    (40.0 * (3.0517578125e-05 *
                    (f64)(f32)(s32)((frand() & 0xFFFF) - 32768))));
                fn_1_E1F4(&particlePosition, 40.0f, 1);
            }
        }
    }
}

typedef Vec M620A618Point;

extern float fn_1_47F0(Vec *first, M620A618Point *second);

/* Selects or blends the requested motion for the board effect. */
static inline void m620_nineab4_set_motion(OMOBJ *object, s32 index)
{
    M620EffectMotionWork *state;
    M620EffectMotionWork *work = object->data;
    M620EffectMotionParam *param;
    s32 model, motion;

    state = work;
    if (state->motionState.motion != index && index < 4) {
        motion = object->mtnId[index];
        work->timer = 0;
        state->motionState.motionTime = 0;
        if (motion == -1) {
            state->motionState.motionActive = 0;
            return;
        }
        param = &work->param->motions[index];
        model = object->mdlId[0];
        if (state->motionState.motion < 0 || 0.0 == param->blend) {
            Hu3DMotionSet(model, motion);
            Hu3DModelAttrSet(model, param->attr);
        } else {
            Hu3DMotionShiftSet(model, motion, 0.0f,
                               60.0f * param->blend, param->attr);
        }
        state->motionState.motion = index;
    }
}

/* Run the prop's opening motion and movement sequence; hide IDs 3 through 33
 * and complete them immediately. */
s32 fn_1_9AB4(OMOBJ *object)
{
    M620EffectMotionWork *motionWork = object->data;
    M620EffectMotionWork *work = motionWork;
    s32 propId;
    s32 done;
    s32 positionIndex;
    s32 modelId;

    done = 0;
    propId = motionWork->id;
    modelId = object->mdlId[0];

    switch (work->motionState.phase) {
    case 0:
        if (propId >= 3 && propId < 34) {
            motionWork->ringActive = 0;
            Hu3DModelAttrSet((s16)modelId, 1);
            done = 1;
            break;
        }
        work->motionState.phase++;
        work->motionState.timer = 0;
        if (propId < 3) {
            positionIndex = propId;
        } else {
            positionIndex = 3;
        }
        work->position = lbl_1_data_64C[positionIndex];
        work->velocity.z = 1.1666666f;
        /* fall through */
    case 1:
        if (++work->motionState.timer < 87) {
            break;
        }
        work->motionState.phase++;
        work->motionState.timer = 0;
        m620_nineab4_set_motion(object, 1);
        work->motionState.motionActive = 1;
        work->velocity.z = 0.0f;
        /* fall through */
    case 2:
        if (work->motionState.motionActive == 0) {
            work->motionState.phase++;
            work->motionState.timer = 0;
            m620_nineab4_set_motion(object, 0);
            /* fall through */
        case 3:
            if (++work->motionState.timer >= 6) {
                work->motionState.phase++;
                work->motionState.timer = 0;
                /* fall through */
            case 4:
                {
                    f32 targetSpeed;
                    f32 angle;
                    f32 targetX;
                    f32 targetZ;

                    angle = fn_1_47F0(&lbl_1_data_64C[4],
                                     (M620A618Point *)&work->position);
                    targetSpeed = 16.666666f;
                    targetX = (f32)(targetSpeed *
                        sin((3.141592653589793 * angle) / 180.0));
                    targetZ = (f32)(targetSpeed *
                        cos((3.141592653589793 * angle) / 180.0));

                    work->velocity.x = (f32)((f64)work->velocity.x +
                        (0.3 * (f64)(targetX - work->velocity.x)));
                    work->velocity.z = (f32)((f64)work->velocity.z +
                        (0.3 * (f64)(targetZ - work->velocity.z)));
                }
                if (++work->motionState.timer >= 18) {
                    work->motionState.phase++;
                    work->motionState.timer = 0;
                    /* fall through */
                case 5:
                    work->velocity.x *= 0.95;
                    work->velocity.z *= 0.95;
                    if (++work->motionState.timer >= 30) {
                        work->motionState.phase++;
                        work->motionState.timer = 0;
                    default:
                        done = 1;
                        break;
                    }
                }
            }
        }
        break;
    }
    return done;
}

/* Randomly positions and steers the floating effect until the sequence changes phase. */
s32 fn_1_9F88(OMOBJ *object)
{
    M620EffectMotionWork *effect = object->data;
    M620EffectMotionWork *work = effect;
    f32 angle;
    f32 distance;
    s32 done = 0;
    s32 propId = effect->id;
    s16 modelId = object->mdlId[0];

    switch (work->motionState.phase) {
    case 0:
        work->motionState.phase++;
        work->motionState.timer = 0;
        /* fall through */
    case 1:
        if (fn_1_FC0()) {
            work->motionState.phase++;
            work->motionState.timer = 0;
            work->position.x = (f32)(450.0 *
                (3.0517578125e-05 *
                  (f32)((u16)frand() - 32768)));
            work->position.z = (f32)(450.0 *
                (3.0517578125e-05 *
                  (f32)((u16)frand() - 32768)));
            work->collision.effect.flags |= 1;
            effect->ringActive = 1;
            Hu3DModelAttrReset((s16)modelId, HU3D_ATTR_DISPOFF);
        }
        break;
    case 2:
        work->motionState.phase++;
        work->motionState.timer = 0;
        if (effect->pointValue != 1) {
            work->collision.effect.unknown_3C = 0.0f;
        }
        if ((propId & 1) == 0) {
            effect->steeringAngle = (f32)(90.0 +
                5.0 * (3.0517578125e-05 *
                  (f32)((u16)frand() - 32768)));
        } else {
            effect->steeringAngle = (f32)((-90.0) +
                5.0 * (3.0517578125e-05 *
                  (f32)((u16)frand() - 32768)));
        }
        effect->steeringSpeed = (f32)(2.5 +
            0.8333333333333334 * (3.0517578125e-05 *
              (f32)((u16)frand() - 32768)));
        /* fall through */
    case 3:
        {
            if (effect->pointValue == 1) {
                angle = 2.0f;
            } else {
                angle = 5.0f;
            }
            effect->steeringAngle = fmodf((f32)(effect->steeringAngle +
                angle * (3.0517578125e-05 *
                 (f32)((u16)frand() - 32768))), 360.0f);
            effect->steeringSpeed = (f32)(effect->steeringSpeed +
                0.16666666666666666 * (3.0517578125e-05 *
                (f32)((u16)frand() - 32768)));
            if (effect->steeringSpeed < 2.0) {
                effect->steeringSpeed = 2.0f;
            }
            if (effect->steeringSpeed > 5.0) {
                effect->steeringSpeed = 5.0f;
            }
        }
        distance = fn_1_488C(&work->position, &lbl_1_data_1F0);
        if (distance < 150.0) {
            angle = fmodf(effect->steeringAngle, 360.0f);
            if (angle < 0.0) angle += 360.0;
            if (angle < 180.0) {
                angle = 135.0f;
            } else {
                angle = (-135.0f);
            }
            effect->steeringAngle = fn_1_4620(effect->steeringAngle, angle, 0.1f);
        }
        {
            f32 targetX;
            f32 targetZ;

            angle = fn_1_47F0(&work->position, &lbl_1_data_1F0);
            angle += effect->steeringAngle;
            targetX = (f32)(effect->steeringSpeed *
                sin((3.141592653589793 * angle) / 180.0));
            targetZ = (f32)(effect->steeringSpeed *
                cos((3.141592653589793 * angle) / 180.0));

            work->velocity.x = (f32)((f64)work->velocity.x +
                0.05 * (f64)(targetX - work->velocity.x));
            work->velocity.z = (f32)((f64)work->velocity.z +
                0.05 * (f64)(targetZ - work->velocity.z));
        }
        if (MgSeqModeGet() != 6U) {
            break;
        }
        work->motionState.phase++;
        work->motionState.timer = 0;
        /* fall through */
    default:
        done = 1;
        break;
    }
    return done;
}

/* Steers the effect toward a side point, then keeps it moving across the board. */
s32 fn_1_A618(OMOBJ *object)
{
    M620EffectMotionWork *work = object->data;
    M620EffectMotionWork *state = work;
    s32 done = 0;
    f32 movementStep = 25.0f;
    f32 moveMagnitude = 0.0f;
    f32 moveX = 0.0f;
    f32 moveZ = 0.0f;
    f64 angle;
    f32 targetMoveZ;

    switch (state->motionState.phase) {
    case 0:
        state->motionState.phase++;
        state->motionState.timer = 0;
        state->collision.effect.unknown_3C = 1.0f;
        /* fall through */
    case 1:
        if (state->position.z > 300.0 || state->position.z < (-300.0)) {
            moveMagnitude = movementStep;
            if (state->position.x < (-100.0)) {
                moveX = moveMagnitude;
            }
            if (state->position.x > 100.0) {
                moveX = -moveMagnitude;
            }
            if (state->position.z < 0.0f) {
                targetMoveZ = moveMagnitude;
            } else {
                targetMoveZ = -moveMagnitude;
            }
            moveZ = targetMoveZ;
        } else {
            state->motionState.phase++;
            state->motionState.timer = 0;
            /* fall through */
        case 2:
            {
                M620A618Point *point = &lbl_1_data_688[
                    state->position.x < 0.0f ? 0 : 1];
                if (fn_1_488C(&state->position, point) > 100.0) {
                    angle = fn_1_47F0(&state->position, point);
                    moveMagnitude = movementStep;
                    moveX = (f32)(moveMagnitude * sin((3.141592653589793 * angle) / 180.0));
                    moveZ = (f32)(moveMagnitude * cos((3.141592653589793 * angle) / 180.0));
                } else {
                    state->motionState.phase++;
                    state->motionState.timer = 0;
                    state->collision.effect.mask &= ~4;
                    /* fall through */
                case 3:
                    moveMagnitude = movementStep;
                    moveX = state->position.x < 0.0f ? -moveMagnitude : moveMagnitude;
                }
            }
        }
        break;
    default:
        done = 1;
        break;
    }
    state->velocity.x += 0.2 * (moveX - state->velocity.x);
    state->velocity.z += 0.2 * (moveZ - state->velocity.z);
    return done;
}

extern void fn_1_EFF4(s32 playerIndex, s32 points);

/* Move an assigned prop through its result destinations, award its points, and send it off the
 * board. */
s32 fn_1_A95C(OMOBJ *object)
{
    M620EffectMotionWork *work = object->data;
    M620EffectMotionWork *state = work;
    s32 done = 0;
    f32 normalSpeed = 8.333333f;
    f32 exitSpeed = 16.666666f;
    f32 movementSpeed = 0.0f;
    f32 moveX = 0.0f;
    f32 moveZ = 0.0f;
    f32 angle;
    M620A618Point *destination;

    switch (state->motionState.phase) {
    case 0:
        state->motionState.phase++;
        state->motionState.timer = 0;
        state->collision.effect.unknown_3C = 0.0f;
        /* fall through */
    case 1:
        destination = (M620A618Point *)&lbl_1_data_5EC[work->selection.playerIndex];
        if (fn_1_488C(&state->position, destination) > 30.0) {
            angle = fn_1_47F0(&state->position, destination);
            movementSpeed = normalSpeed;
            moveX = (f32)(movementSpeed *
                sin((3.141592653589793 * angle) / 180.0));
            moveZ = (f32)(movementSpeed *
                cos((3.141592653589793 * angle) / 180.0));
            break;
        }
        state->motionState.phase++;
        state->motionState.timer = 0;
        state->collision.effect.mask &= ~8;
        /* fall through */
    case 2:
        destination = (M620A618Point *)&lbl_1_data_61C[work->selection.playerIndex];
        if (fn_1_488C(&state->position, destination) > 30.0) {
            angle = fn_1_47F0(&state->position, destination);
            movementSpeed = normalSpeed;
            moveX = (f32)(movementSpeed *
                sin((3.141592653589793 * angle) / 180.0));
            moveZ = (f32)(movementSpeed *
                cos((3.141592653589793 * angle) / 180.0));
            break;
        }
        if (work->pointValue == 1) {
            fn_1_8A64(object);
        }
        fn_1_EFF4(work->selection.playerIndex, work->pointValue);
        state->collision.effect.mask |= 8;
        state->collision.effect.unknown_3C = 1.0f;
        state->motionState.phase++;
        state->motionState.timer = 0;
        /* fall through */
    case 3:
        if (work->selection.playerIndex < 2 && ++state->motionState.timer < 12) {
            break;
        }
        if (work->pointValue == 1) {
            state->motionState.phase = 7;
            state->motionState.timer = 0;
            break;
        }
        state->motionState.phase++;
        state->motionState.timer = 0;
        state->collision.effect.unknown_3C = 0.0f;
        /* fall through */
    case 4:
        if (++state->motionState.timer < 12) {
            break;
        }
        state->motionState.phase++;
        state->motionState.timer = 0;
        fn_1_697C(object, 1);
        state->motionState.motionActive = 1;
        fn_1_8A64(object);
        /* fall through */
    case 5:
        state->rotation.y = fn_1_4620(state->rotation.y, 0.0f, 0.05f);
        if (state->motionState.motionActive != 0) {
            break;
        }
        /* Force the three-point prop's motion to restart until four playbacks have completed. */
        if (++state->motionState.timer < 4) {
            state->motionState.motion = -1;
            fn_1_697C(object, 1);
            state->motionState.motionActive = 1;
            break;
        }
        state->motionState.phase++;
        state->motionState.timer = 0;
        fn_1_697C(object, 0);
        /* fall through */
    case 6:
        if (++state->motionState.timer < 12) {
            break;
        }
        state->motionState.phase++;
        state->motionState.timer = 0;
        state->collision.effect.unknown_3C = 1.0f;
        /* fall through */
    case 7:
        destination = (M620A618Point *)&lbl_1_data_6A0;
        if (fn_1_488C(&state->position, destination) > 100.0) {
            angle = fn_1_47F0(&state->position, destination);
            movementSpeed = exitSpeed;
            moveX = (f32)(movementSpeed *
                sin((3.141592653589793 * angle) / 180.0));
            moveZ = (f32)(movementSpeed *
                cos((3.141592653589793 * angle) / 180.0));
            break;
        }
        state->motionState.phase++;
        state->motionState.timer = 0;
        state->collision.effect.mask &= ~4;
        /* fall through */
    case 8:
        movementSpeed = exitSpeed;
        moveZ = -movementSpeed;
        if (state->position.z < -500.0) {
            moveX = (f32)(movementSpeed *
                (-0.0015 * ((f64)state->position.z - -500.0)));
        }
        break;
    default:
        done = 1;
        break;
    }
    state->velocity.x = (f32)((f64)state->velocity.x +
        0.2 * (f64)(moveX - state->velocity.x));
    state->velocity.z = (f32)((f64)state->velocity.z +
        0.2 * (f64)(moveZ - state->velocity.z));
    return done;
}
/* Refresh the collision reference position and model transform, then update motion-completion
 * state. */
void fn_1_B254(OMOBJ *object)
{
    M620EffectMotionWork *initialWork;
    M620EffectMotionWork *workForCopy;
    M620EffectMotionWork *transformWork;
    M620EffectMotionWork *transformState;
    M620EffectMotionWork *motionWork;
    M620EffectMotionWork *motionState;
    s32 transformModelId;
    s32 motionComplete;
    s32 motionModelId;
    f32 angle;
    f32 reducedAngle;
    f32 storedAngle;

    initialWork = object->data;
    workForCopy = initialWork;
    workForCopy->collision.effect.positionCopy = *workForCopy->collision.effect.positionSource;

    transformWork = object->data;
    transformState = transformWork;
    transformModelId = object->mdlId[0];
    angle = transformState->rotation.y;
    reducedAngle = (f32)fmod((f64)angle, 360.0);
    storedAngle = reducedAngle;
    transformState->rotation.y = storedAngle;
    Hu3DModelPosSetV(transformModelId, &transformState->position);
    Hu3DModelRotSetV(transformModelId, &transformState->rotation);

    motionWork = object->data;
    motionState = motionWork;
    if (motionState->motionState.motionActive != 0) {
        motionModelId = object->mdlId[0];
        if (Hu3DMotionEndCheck(motionModelId) &&
            Hu3DMotionShiftIDGet(motionModelId) < 0) {
            motionComplete = 1;
        } else {
            motionComplete = 0;
        }
        if (motionComplete != 0) {
            motionState->motionState.motionActive = 0;
            motionState->motionState.motionTimer = 0;
        }
    } else {
        motionState->motionState.motionTimer = 0;
    }
    motionWork->timer++;
}

/* During sequence phase 5, changes the steering angle and damps X/Z velocity. */
s32 fn_1_B3A0(M620B3A0Controller *controller,
              M620B3A0OtherCollision *otherCollision)
{
    OMOBJ *object = controller->object;
    M620EffectMotionWork *modeWork = object->data;
    M620EffectMotionWork *motionWork = modeWork;

    if (MgSeqModeGet() == 5 &&
        (otherCollision->state != 0 || modeWork->pointValue == 1)) {
        if ((modeWork->id & 1) == 0) {
            modeWork->steeringAngle += 5.0;
        } else {
            modeWork->steeringAngle -= 5.0;
        }
        motionWork->velocity.x *= 0.9;
        motionWork->velocity.z *= 0.9;
    }
    return 1;
}

/* Sets the queued launch request for the selected player's object. */
void fn_1_B48C(s32 playerIndex)
{
    OMOBJ *playerObject;
    M620PlayerLaunchWork *playerWork;

    playerObject = lbl_1_bss_438[playerIndex & 3];
    if (playerObject != (void *)0) {
        playerWork = playerObject->data;
        playerWork->action.launchRequested = 1;
    }
}

/* Count launch props ready for nearby-prop selection. */
s32 fn_1_B4D8(void)
{
    s32 selectableCount;
    s32 playerIndex;

    selectableCount = 0;
    playerIndex = 0;
    while (playerIndex < 4) {
        if (fn_1_B534(playerIndex) != 0) {
            selectableCount += 1;
        }
        playerIndex += 1;
    }
    return selectableCount;
}

/* Reports whether a player's launch object is ready for selection checks. */
s32 fn_1_B534(s32 playerIndex)
{
    OMOBJ *playerObject = lbl_1_bss_438[playerIndex & 3];
    if (playerObject != (void *)0) {
        M620PlayerLaunchWork *playerWork = playerObject->data;
        if (playerWork->selection.selectionReady != 0)
            return 1;
    }
    return 0;
}

/* Counts registered props within the player's horizontal selection radius. */
s32 fn_1_B590(s32 playerIndex)
{
    M620PropWork *work;
    s32 index;
    OMOBJ *referenceObject = lbl_1_bss_438[playerIndex & 3];
    s32 count = 0;

    if (referenceObject != NULL) {
        M620PlayerLaunchWork *referenceData = referenceObject->data;
        M620PlayerLaunchWork *referenceWork = referenceData;
        f32 distanceLimit = (f32)((f64)referenceWork->collision.launch.innerRadius +
            (0.5 * (f64)referenceWork->collision.launch.radialThickness));

        for (index = 0; index < 35; index++) {
            OMOBJ *object = lbl_1_bss_3AC[index];
            if (object != NULL) {
                M620PropWork *workData = object->data;
                f32 deltaX;
                f32 deltaZ;

                work = workData;
                deltaX = work->position.x - referenceWork->position.x;
                deltaZ = work->position.z - referenceWork->position.z;

                if (sqrtf((deltaX * deltaX) + (deltaZ * deltaZ)) <
                    distanceLimit) {
                    count++;
                }
            }
        }
    }
    return count;
}

s32 fn_1_B7C4(void)
{
    return lbl_1_bss_3A4;
}
