/* Dunk Bros. player state, movement, actions and character effects. */
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "math.h"
#include "dolphin/pad.h"
#include "dolphin/os.h"
#include "game/charman.h"
#include "game/memory.h"
#include "game/audio.h"
#include "game/gamework.h"
#include "game/pad.h"
#include "string.h"

#include "REL/m699Dll/m699_natural_views.h"
#include "game/object.h"

#include "REL/m699Dll/m699_work_record_prefix_v3.h"
#include "game/hu3d.h"
#include "game/mg/seqman.h"
#include "humath.h"

extern int frand(void);
extern s32 fn_1_39D8(Vec *position);
typedef struct M699MotionRecord {
    u16 motionListIndex;
    u16 unknown_002;
    f32 blendWeight;
    u32 attributes;
} M699MotionRecord;

typedef struct M699PointerData {
    Vec position;
} M699PointerData;

s32 lbl_1_data_F8[16] = { 0x930000, 0x930001, 0x930002, 0x8E00AF, 0x930069, 0x93006A, 0x930003, 0x930004, 0x93005B, 0x93005C, 0x8E00B3, 0x8E00B4, 0x8E00B0, 0x8E00B1, 0x930026, 0x930028 };

M699MotionRecord lbl_1_data_138[] = { { 0, 0, 0.2f, 0x40000001 }, { 1, 0, 0.2f, 0x40000001 }, { 2, 0, 0.2f, 0x40000001 }, { 3, 0, 0.2f, 0x40000001 }, { 4, 0, 0.2f, 0x40000001 }, { 5, 0, 0.2f, 0x40000001 }, { 6, 0, 0.2f, 0 }, { 7, 0, 0.2f, 0 }, { 8, 0, 0.1f, 0 }, { 9, 0, 0.1f, 0 }, { 10, 0, 0.1f, 0 }, { 11, 0, 0.1f, 0 }, { 12, 0, 0.1f, 0 }, { 12, 0, 0.1f, 0x40000004 }, { 13, 0, 0.15f, 0 }, { 14, 0, 0.2f, 0 }, { 15, 0, 0.2f, 0 } };

Vec lbl_1_data_204[] = { { -500.0f, 0.0f, -200.0f }, { -500.0f, 0.0f, 200.0f }, { 500.0f, 0.0f, 200.0f }, { 500.0f, 0.0f, -200.0f }, { -600.0f, 0.0f, 0.0f }, { 600.0f, 0.0f, 0.0f } };

f32 lbl_1_data_24C[] = { 90.0f, 90.0f, 270.0f, 270.0f, 90.0f, 270.0f };

M699PointerData lbl_1_data_264[6] = { { { -500.0f, 0.0f, -300.0f } }, { { -500.0f, 0.0f, 300.0f } }, { { 500.0f, 0.0f, -300.0f } }, { { 500.0f, 0.0f, 300.0f } }, { { -600.0f, 0.0f, 0.0f } }, { { 600.0f, 0.0f, 0.0f } } };

f32 lbl_1_data_2AC[6] = { 90.0f, 90.0f, 270.0f, 270.0f, 90.0f, 270.0f };

Vec lbl_1_data_2C4[] = { { -500.0f, 0.0f, 100.0f }, { -250.0f, 0.0f, 300.0f }, { 250.0f, 0.0f, 300.0f }, { 500.0f, 0.0f, 100.0f }, { -400.0f, 0.0f, -300.0f }, { 400.0f, 0.0f, -300.0f } };

Vec lbl_1_data_30C = { 0.0f, 0.0f, 0.0f };

Vec lbl_1_data_318[2] = { { -70.0f, 0.0f, 0.0f }, { 70.0f, 0.0f, 0.0f } };

f32 lbl_1_data_330[2] = { 90.0f, 270.0f };

Vec lbl_1_data_338[4] = { { -1100.0f, 0.0f, -350.0f }, { -1100.0f, 0.0f, 350.0f }, { 1100.0f, 0.0f, -350.0f }, { 1100.0f, 0.0f, 350.0f } };

f32 lbl_1_data_368[2] = { 90.0f, 270.0f };

M699PlayerMotionData lbl_1_data_370 = { 50.0f, 200.0f, 10.0f, 20.833334f, -1.5f, 20.0f, 1.0f, 1.0f, 0.5f, 1.25f, 1.0f, { -30.0f, 40.0f, 50.0f, 10.0f, 80.0f, 80.0f, 60.0f }, 0, 1, 2.0f, 2, 1.0f, 3, 4, 2.0f, 5, 1.0f, 6, 7, 14, 13, 12, 8, 9, 10, 11, 15, 16 };

f32 lbl_1_data_40C[][7] = { { -40.0f, 65.0f, 50.0f, -5.0f, 80.0f, 80.0f, 60.0f }, { -50.0f, 55.0f, 45.0f, -15.0f, 80.0f, 80.0f, 60.0f }, { -40.0f, 65.0f, 70.0f, 10.0f, 80.0f, 80.0f, 60.0f }, { -40.0f, 65.0f, 45.0f, -10.0f, 80.0f, 80.0f, 60.0f }, { -45.0f, 60.0f, 50.0f, 10.0f, 80.0f, 80.0f, 60.0f }, { -40.0f, 65.0f, 70.0f, 10.0f, 80.0f, 80.0f, 60.0f }, { -45.0f, 60.0f, 65.0f, 10.0f, 80.0f, 80.0f, 40.0f }, { -30.0f, 57.0f, 40.0f, -8.0f, 80.0f, 80.0f, 60.0f }, { -45.0f, 65.0f, 50.0f, -10.0f, 80.0f, 80.0f, 60.0f }, { -75.0f, 65.0f, 40.0f, 5.0f, 80.0f, 80.0f, 60.0f }, { -30.0f, 57.0f, 40.0f, -8.0f, 80.0f, 80.0f, 60.0f }, { -75.0f, 65.0f, 50.0f, 5.0f, 80.0f, 80.0f, 60.0f }, { -75.0f, 65.0f, 40.0f, 5.0f, 80.0f, 80.0f, 60.0f }, { -75.0f, 65.0f, 40.0f, 5.0f, 80.0f, 80.0f, 60.0f }, { -30.0f, 65.0f, 50.0f, 10.0f, 80.0f, 80.0f, 60.0f }, { -30.0f, 65.0f, 50.0f, 10.0f, 80.0f, 80.0f, 60.0f } };

s32 lbl_1_data_5CC[] = { 23, 24 };

extern s32 lbl_1_bss_428;
extern M699CharacterWorkPrefix *lbl_1_bss_42C[6];
extern void fn_1_5A90(int argument);
extern void fn_1_5C24(M699CharacterWorkPrefix *work);
void fn_1_5D78(M699CharacterWorkPrefix *work);

extern void fn_1_5DD0(M699CharacterWorkPrefix *work);
extern s32 fn_1_10C0(void);
extern void fn_1_6020(void);
typedef struct M699RelatedWorkTable {
    M699CharacterWorkPrefix *members[3];
} M699RelatedWorkTable;

extern s32 fn_1_10D0(void);
extern void *fn_1_14ACC(Vec *position, Vec *rotation);
extern s32 fn_1_14F80(void);
extern f32 fn_1_5230(Vec *first, Vec *second);
extern void fn_1_10E0(s32 mode);
extern void *fn_1_10B8(s32 motionData);
extern s32 fn_1_4CB0(void);

extern s32 lbl_1_bss_444[25];
extern void fn_1_76E8(OMOBJ *object, s32 mode);
void fn_1_75B0(OMOBJ *object);

extern void fn_1_67A0(OMOBJ *object);
extern s32 fn_1_6990(M699CharacterWorkPrefix *work);
extern s32 fn_1_6B10(M699CharacterWorkPrefix *work);
extern s32 fn_1_6C74(M699CharacterWorkPrefix *work);
extern s32 fn_1_717C(M699CharacterWorkPrefix *work);
extern s32 fn_1_739C(M699CharacterWorkPrefix *work);
void fn_1_F70C(M699CharacterWorkPrefix *work);

void fn_1_760C(M699CharacterWorkPrefix *work);

void fn_1_7680(OMOBJ *object);

void fn_1_74B4(OMOBJ *self);

extern void fn_1_AD3C(M699CharacterWorkPrefix *work);
extern void fn_1_AEF4(M699CharacterWorkPrefix *work);
extern s32 fn_1_9B60(M699CharacterWorkPrefix *work, Vec *data, f32 speed, f32 duration);
extern u16 lbl_802C0010[];
extern u16 lbl_802C0008[];
void fn_1_B340(M699CharacterWorkPrefix *work);

extern void fn_1_78F0(M699CharacterWorkPrefix *work);
extern void fn_1_A4F4(M699CharacterWorkPrefix *work);
extern s8 lbl_802BFFFC[];
extern s8 lbl_802BFFF8[];
extern Vec lbl_80266A58[];
void fn_1_7628(M699CharacterWorkPrefix *work, s32 motion, f32 speed);

extern void *fn_1_14B68(Vec *position, f32 blend, Vec *rotation, void *targetObject);
extern void fn_1_7D04(M699CharacterWorkPrefix *work);
extern void fn_1_8170(M699CharacterWorkPrefix *work);
extern void fn_1_A160(M699CharacterWorkPrefix *work);
extern s32 fn_1_1248(s32 objectId);
extern s32 fn_1_1268(void);
extern s32 fn_1_7864(OMOBJ *self);
extern s32 fn_1_1154(u32 camera);
extern void fn_1_F740(M699CharacterWorkPrefix *work, s32 model);
extern s32 fn_1_147A8(Vec *position, f32 heading, f32 angleLimit, f32 distanceLimit);
extern f32 fn_1_5098(Vec *first, Vec *second);
extern f32 fn_1_4EC8(f32 start, f32 end, f32 factor);
extern void fn_1_14D50(void *camera, s32 value);
extern void fn_1_539C(Vec *start, Vec *end, f32 fraction, Vec *out);
extern void fn_1_435C(s32 motion, Vec *position);
extern Vec *fn_1_14778(s32 index);
extern void fn_1_14EB0(s32 value);
extern void fn_1_14E58(void *work, void *target);
extern s32 fn_1_53F4(Vec *from, Vec *to, f32 gravity, f32 initialVerticalSpeed, Vec *velocity);
extern void fn_1_14DA8(void *camera, s32 value);
extern void fn_1_14E00(void *camera, s32 value);
extern f32 fn_1_4FE4(f32 current, f32 destination);
typedef struct M699MotionEventView {
    u8 unk_0[8]; /* No code names this field. */
    u32 type;
    u8 unk_C[4]; /* No code names this field. */
    s32 event;
    OMOBJ *owner;
    u8 unk_18[0x10]; /* No code names this field. */
    Vec *position;
    u8 unk_2C[8]; /* No code names this field. */
    f32 duration;
    u8 unk_38[4]; /* No code names this field. */
    f32 boundary;
} M699MotionEventView;

typedef struct M699MotionCallbackView {
    u8 unk_0[0x14]; /* No code names this field. */
    OMOBJ *owner;
    u8 unk_18[0x10]; /* No code names this field. */
    Vec *position;
    Vec *velocity;
    u8 unk_30[0xC]; /* No code names this field. */
    f32 boundary;
    f32 verticalOffset;
} M699MotionCallbackView;

extern void fn_1_B518(M699CharacterWorkPrefix *work, M699CharacterWorkCallback callback);
extern void fn_1_BDE0(M699CharacterWorkPrefix *work);
void fn_1_BFC0(M699CharacterWorkPrefix *work);

typedef M699CharacterWorkCallback M699SequenceCallback;

extern void fn_1_C038(M699CharacterWorkPrefix *work);
extern void fn_1_C188(M699CharacterWorkPrefix *work);
extern s32 fn_1_F568(M699CharacterWorkPrefix *work);
extern void fn_1_BA5C(M699CharacterWorkPrefix *work);
extern void fn_1_BB64(M699CharacterWorkPrefix *work);
extern void fn_1_BC70(M699CharacterWorkPrefix *work);
extern void fn_1_EB74(M699CharacterWorkPrefix *work);
typedef void (*M699CharacterCallback)(M699CharacterWorkPrefix *work);

void fn_1_B944(M699CharacterWorkPrefix *work);

extern void fn_1_C588(M699CharacterWorkPrefix *work);
extern void fn_1_CA68(M699CharacterWorkPrefix *work);
extern void fn_1_D254(M699CharacterWorkPrefix *work);
extern f32 fn_1_5134(Vec *first, Vec *second);
extern s32 fn_1_F2D0(Vec *position, f32 *heading, f32 *distance, s32 reflect);
void *fn_1_F968(u32 searchKey, void *excludedWork, Vec *targetPosition, f32 heading, f32 angleLimit, f32 distanceLimit, s32 returnNearest);

void fn_1_EFDC(M699CharacterWorkPrefix *work);

static inline void M699DebugAddress(const void *address) { }

extern s32 fn_1_F660(M699CharacterWorkPrefix *work, void *target, f32 distanceLimit);
typedef M699CharacterWorkCallback M699UpdateCallback;

extern void fn_1_DBA8(M699CharacterWorkPrefix *work);
extern void fn_1_E300(M699CharacterWorkPrefix *work);
/* Debug hook: preserve the release build's evaluation of this data address. */
static inline void M699DebugParameterAddress(const M699MovementProfile *value)
{
}

/* Debug hook: preserve the release build's evaluation of a discarded object pointer. */
static inline void M699DebugDiscardedObject(void *value)
{
}

/* Debug hook: preserve the release build's evaluation of a discarded query result. */
static inline void M699DebugDiscardedQueryResult(void *value)
{
}

extern OMOBJ *lbl_1_bss_4A8[4];
extern void fn_1_75C(f32 x, f32 y, f32 z, f32 w);

void fn_1_5958(void);
void fn_1_597C(void);
void fn_1_59CC(int argument);

void fn_1_6154(M699CharacterWorkPrefix *work);
void fn_1_62CC(OMOBJ *object);

void fn_1_753C(OMOBJ *object, s32 effect);

void fn_1_7634(M699CharacterWorkPrefix *state, s32 motion, f32 speed);

void fn_1_83C0(M699CharacterWorkPrefix *work);
void *fn_1_8BB4(M699CharacterWorkPrefix *work, void *camera,
                Vec *cameraPosition, Vec *cameraVelocity);
void *fn_1_909C(M699CharacterWorkPrefix *work, void *camera,
                Vec *position, Vec *cameraVelocity);
void *fn_1_96F4(M699CharacterWorkPrefix *work, void *camera,
                Vec *cameraPosition, Vec *velocity);

s32 fn_1_AA54(M699MotionCallbackView *motion, M699MotionEventView *event);

s32 fn_1_B558(M699CharacterWorkPrefix *work, Vec *target,
              f32 heading, f32 speed);
M699SequenceCallback fn_1_B8E8(M699CharacterWorkPrefix *work);

void fn_1_D720(M699CharacterWorkPrefix *work);

s32 fn_1_FCBC(u32 key);

void fn_1_5958(void)
{
    fn_8005BB8C(-1);
}

/* Reset the shared player counters. */
void fn_1_597C(void) {
    s32 index;

    for (index = 0; index < 6; index += 1) {
        lbl_1_bss_42C[index] = 0;
    }
    lbl_1_bss_428 = 0;
}

void fn_1_59CC(int argument)
{
    int index;
    M699CharacterWorkPrefix *work;

    if (lbl_1_bss_428 == 0) {
        fn_1_5A90(argument);
        lbl_1_bss_428 = 1;
    }
    for (index = 0; index < 6; index++) {
        work = lbl_1_bss_42C[index];
        if (work != NULL) {
            fn_1_5C24(work);
            fn_1_5D78(work);
            fn_1_5DD0(work);
            if (work->countdown > 0) {
                work->countdown -= 1;
            }
        }
    }
    fn_1_6020();
}

void fn_1_5A90(int unusedArgument)
{
    M699RelatedWorkTable table[2];
    M699CharacterWorkPrefix *work;
    s32 groupIndex;
    s32 memberIndex;

    for (groupIndex = 0; groupIndex < 2; groupIndex++) {
        for (memberIndex = 0; memberIndex < 3; memberIndex++) {
            table[groupIndex].members[memberIndex] = NULL;
        }
    }

    for (groupIndex = 0; groupIndex < 6; groupIndex++) {
        work = lbl_1_bss_42C[groupIndex];
        if (work != NULL && work->groupFlag < 3) {
            table[work->groupId & 1].members[work->groupFlag] = work;
        }
    }

    for (groupIndex = 0; groupIndex < 2; groupIndex++) {
        work = table[groupIndex].members[0];
        if (work != NULL) {
            work->relatedObjects[0] = table[groupIndex].members[1];
            work->relatedObjects[1] = table[groupIndex].members[1];
        }

        work = table[groupIndex].members[1];
        if (work != NULL) {
            work->relatedObjects[0] = table[groupIndex].members[0];
            work->relatedObjects[1] = table[groupIndex].members[0];
        }

        work = table[groupIndex].members[2];
        if (work != NULL) {
            work->relatedObjects[0] = table[groupIndex].members[0];
            work->relatedObjects[1] = table[groupIndex].members[1];
        }
    }
}

void fn_1_5C24(M699CharacterWorkPrefix *work)
{
    f32 scale;

    if (work->animating != 0) {
        if ((work->animationProgress += 0.05) > 1.0) {
            work->animationProgress = 1.0f;
        }
    } else {
        if ((work->animationProgress -= 0.05) <= 0.0) {
            work->animationProgress = 0.0f;
        }
    }
    if (work->animationProgress > 0.0) {
        scale = work->animationProgress * work->motionData.modelScale;
        fn_80026A44((s16)work->modelId, 1);
        fn_80026760((s16)work->modelId, work->initialPosition.x, 10.0f, work->initialPosition.z);
        fn_800268B0((s16)work->modelId, scale, 1.0f, scale);
    } else {
        fn_800269E8((s16)work->modelId, 1);
    }
}

/* Show the item model while its item state is active. */
void fn_1_5D78(M699CharacterWorkPrefix *work) {
    if (work->itemState != 0) {
        fn_80026A44((HU3D_MODELID)work->itemModelId, 1);
        return;
    }
    fn_800269E8((HU3D_MODELID)work->itemModelId, 1);
}

void fn_1_5DD0(M699CharacterWorkPrefix *work)
{
    f32 x;
    f32 z;
    f32 angle;
    f32 radius;

    if (fn_1_10C0() == 1 && work->groupFlag != 0) {
        radius = 400.0f;
        x = work->initialPosition.x;
        z = work->initialPosition.z;
        if (sqrtf(x * x + z * z) < radius) {
            angle = (f32)(180.0 * (atan2((f64)x, (f64)z) / M_PI));
            work->initialPosition.x = (f32)((f64)radius * sin((M_PI * (f64)angle) / 180.0));
            work->initialPosition.z = (f32)((f64)radius * cos((M_PI * (f64)angle) / 180.0));
        }
    }
}

void fn_1_6020(void)
{
    Vec position;
    M699CharacterWorkPrefix *closest;
    M699CharacterWorkPrefix *candidate;
    void *queriedObject;
    s32 index;
    s32 cameraId;
    f32 distance;
    f32 closestDistance;

    switch (fn_1_10C0()) {
    case 3:
    case 4:
    case 5:
        if (fn_1_10D0() <= 60) {
            return;
        }
        closest = NULL;
        queriedObject = fn_1_14ACC(&position, NULL);
        cameraId = fn_1_14F80();
        closestDistance = 100000.0f;
        for (index = 0; index < 6; index++) {
            candidate = lbl_1_bss_42C[index];
            if (candidate != NULL && candidate->groupId != cameraId) {
                distance = fn_1_5230(&candidate->initialPosition, &position);
                if (distance < closestDistance) {
                    closestDistance = distance;
                    closest = candidate;
                }
            }
        }
        if (closest != NULL) {
            closest->selected = 1;
            closest->selectionValue1 = 0;
            closest->selectionValue2 = 0;
            fn_1_10E0(6);
        }
    }
}

void fn_1_6154(M699CharacterWorkPrefix *work)
{
    s32 index;
    s32 tableIndex;

    index = 0;
    while (index < 6) {
        if (lbl_1_bss_42C[index] == NULL) {
            lbl_1_bss_42C[index] = work;
            tableIndex = work->groupNumber;
            work->initialPosition = lbl_1_data_204[tableIndex];
            work->position.x = work->position.y = work->position.z = 0.0f;
            work->rotation.y = lbl_1_data_24C[tableIndex];
            work->soundPlayed = 0;
            work->soundPlaybackTimer = 0;
            work->acceleration = 1.0f;
            work->motionAdjustment = 1.0f;
            work->characterEffectPlayed = 0;
            fn_1_10B8(lbl_1_data_5CC[work->groupId]);
            work->modelId = fn_1_4CB0();
            fn_800269E8((s16)work->modelId, ((1 << 30) | (1 << 0)));
            fn_80026B8C((s16)work->modelId, 3);
            work->animating = 0;
            work->animationProgress = 0.0f;
            return;
        }
        index++;
    }
}

void fn_1_62CC(OMOBJ *object)
{
    s32 motionIndex;
    s32 groupNumber;
    char *modelHook;
    M699CharacterWorkPrefix *controller;
    M699PlayerMotionData *params;
    const f32 *table;
    s32 playerIndex;
    s32 groupId;
    s32 characterNumber;
    s32 characterModel;
    s32 itemModel;
    M699CharacterWorkPrefix *work;
    s32 motionId;
    M699CharacterWorkPrefix *state;

    work = HuMemDirectMallocNum(HEAP_HEAP, 0x2A8, 0x10000000);
    object->data = work;
    memset(work, 0, 0x2A8);
    object->stat |= 0x100;
    state = work;

    work->groupNumberCopy = playerIndex = object->work[0];
    characterNumber = lbl_80265728[playerIndex].charNo;
    work->characterNumberCopy = characterNumber;
    groupId = state->groupId = object->work[1];
    state->groupNumber = groupNumber = object->work[2];
    state->groupFlag = groupNumber & 1;
    state->readPadInput = 1;
    state->characterNumber = lbl_80265728[playerIndex].type;
    state->controllerType = lbl_80265728[playerIndex].comDif & 3;
    state->characterMotionState = 0;

    characterModel = fn_80057068(characterNumber, 4);
    object->mdlId[0] = characterModel;
    fn_80026B8C(characterModel, 4);
    fn_80026E44(characterModel);
    fn_8005DDEC(5);

    for (motionIndex = 0; motionIndex < 16; motionIndex++) {
        motionId = fn_8005B0B8((s16)characterNumber,
                                    lbl_1_data_F8[motionIndex]);
        object->mtnId[motionIndex] = motionId;
    }
    fn_8005B4BC(characterNumber);

    state->motionState = -1;
    fn_1_76E8(object, 0);
    state->itemState = 0;
    fn_1_10B8(0x16);
    state->itemModelId = fn_1_4CB0();
    modelHook = fn_8005C0F0(characterNumber, 4, 0);
    fn_8002741C(characterModel, modelHook, state->itemModelId);
    fn_80026B8C(state->itemModelId, 5);

    work->sameGroup = (((u32)state->groupNumber ^ (u32)lbl_1_bss_444[0]) & 1) == 0;
    params = &state->motionData;
    *params = lbl_1_data_370;
    table = lbl_1_data_40C[characterNumber];
    params->height = fn_8005C4C8(characterNumber);
    params->scales[0] = table[0];
    params->scales[1] = table[1];
    params->scales[2] = table[2];
    params->scales[3] = table[3];
    params->scales[4] = table[4];
    params->scales[5] = table[5];
    params->scales[6] = table[6];

    controller = state;
    controller->state = 0;
    controller->stateLimit = 0x17;
    controller->controllerValue = 0;
    controller->playerIndex = playerIndex;
    controller->controllerCounter = 0;
    controller->object = object;
    controller->update = fn_1_75B0;
    controller->callback = fn_1_AA54;
    controller->initialPositionAddress = &state->initialPosition;
    controller->positionAddress = &state->position;
    controller->savedPosition = *controller->initialPositionAddress;
    controller->boundary = 60.0f;
    controller->verticalOffset = 0.0f;
    if (params->height < 1.6) {
        controller->scaledHeight = (f32)(1.03 * params->height);
    } else {
        controller->scaledHeight = (f32)(1.07 * params->height);
    }
    controller->heightAdjustment = 1.0f;
    fn_1_1D6BC(controller);
    fn_1_6154(state);
    work->initializationCount = 0;
    work->initialized = 1;
    object->objFunc = fn_1_67A0;
}

void fn_1_67A0(OMOBJ *object)
{
    M699CharacterWorkPrefix *work = object->data;
    M699CharacterWorkPrefix *state = work;
    void *modelItem;

    state->heightAdjustment = 1.0f;
    fn_1_F70C(state);
    fn_1_760C(state);
    switch (work->initializationCount) {
    case 0:
        if (fn_1_6990(state)) {
            work->initializationCount++;
            state->phaseState = 0;
        }
        break;
    case 1:
        if (fn_1_6B10(state)) {
            work->initializationCount++;
            state->phaseState = 0;
        }
        break;
    case 2:
        if (fn_1_6C74(state)) {
            work->initializationCount++;
            state->phaseState = 0;
        }
        break;
    case 3:
        if (fn_1_717C(state)) {
            work->initializationCount++;
            state->phaseState = 0;
        }
        break;
    case 4:
        if (fn_1_739C(state)) {
            work->initializationCount++;
            state->phaseState = 0;
        }
        break;
    }
    fn_1_AD3C(state);
    fn_1_AEF4(state);
    fn_1_7680(object);
    PSVECAdd(&state->initialPosition, &state->position, &state->initialPosition);
    modelItem = fn_8005C0F0((s16)work->characterNumberCopy, 4, 0);
    Hu3DModelObjPosGet(object->mdlId[0],
                       modelItem,
                       &state->modelObjectPosition);
    if (fn_1_14ACC(NULL, NULL) == state) {
        state->stateLimit &= ~0x10;
    } else {
        state->stateLimit |= 0x10;
    }
    fn_1_74B4(object);
    state->collisionSource = 0;
}

s32 fn_1_6990(M699CharacterWorkPrefix *work)
{
    s32 result;

    result = 0;
    switch (work->phaseState) {
    case 0:
        work->phaseState++;
        work->phaseTimer = 0;
        /* fall through */
    case 1:
        if (++work->phaseTimer < 90) {
            break;
        }
        work->phaseState++;
        work->phaseTimer = 0;
        work->animating = 1;
        work->state |= 1;
        if (work->groupFlag != 0) {
            work->phaseState = 4;
            work->phaseTimer = 0;
            break;
        }
        /* fall through */
    case 2:
        if (fn_1_9B60(work, &lbl_1_data_318[work->groupId],
                      lbl_1_data_330[work->groupId],
                      1.0f) == 0) {
            break;
        }
        work->phaseState++;
        work->phaseTimer = 0;
        /* fall through */
    case 3:
        if (++work->phaseTimer < 30) {
            break;
        }
        work->phaseState++;
        work->phaseTimer = 0;
        /* fall through */
    case 4:
        if (fn_80071A8C() != 5) {
            break;
        }
        work->phaseState++;
        work->phaseTimer = 0;
        /* fall through */
    default:
        result = 1;
        break;
    }
    return result;
}

s32 fn_1_6B10(M699CharacterWorkPrefix *work)
{
    s32 result;
    M699CharacterWorkPrefix *playerWork;
    s32 padNo;

    result = 0;
    switch (work->phaseState) {
    case 0:
        if (work->groupFlag != 0) {
            return 1;
        }
        work->phaseState++;
        work->phaseTimer = 0;
    case 1:
        if (work->characterNumber != 0) {
            fn_1_B340(work);
        } else if (work->readPadInput != 0) {
            playerWork = work;
            padNo = lbl_80265728[playerWork->groupNumberCopy].padNo;
            work->buttonsHeld = lbl_802C0010[padNo];
            work->padDown = lbl_802C0008[padNo];
        }
        if (fn_1_10C0() != 1) {
            work->phaseState++;
            work->phaseTimer = 0;
    case 2:
            if (fn_1_14ACC(NULL, NULL) != work) {
                work->phaseState++;
                work->phaseTimer = 0;
            default:
                result = 1;
            }
        }
        fn_1_78F0(work);
        fn_1_A4F4(work);
        return result;
    }
    return result;
}

s32 fn_1_6C74(M699CharacterWorkPrefix *work)
{
    M699PlayerMotionData *motionWork;
    s32 done;
    s32 inputMode;
    M699CharacterWorkPrefix *playerWork;
    f32 stickX;
    f32 stickY;
    f32 combinedAngle;
    f32 wrappedAngle;
    f32 finalAngle;
    s32 pad;

    motionWork = &work->motionData;
    done = 0;
    switch (work->phaseState) {
    case 0:
        work->phaseState++;
        work->phaseTimer = 0;
    case 1:
        if (fn_80071A8C() == 6) {
            work->phaseState++;
            work->phaseTimer = 0;
            break;
        } else {
            inputMode = fn_1_10C0();
            if (inputMode == 2 || inputMode == 6) {
                if (work->characterNumber != 0) {
                    fn_1_B340(work);
                } else if (work->readPadInput != 0) {
                    playerWork = work;
                    pad = lbl_80265728[playerWork->groupNumberCopy].padNo;
                    stickX = (f32)(s8)lbl_802BFFFC[pad];
                    stickY = (f32)-(s8)lbl_802BFFF8[pad];
                    work->buttonsHeld = lbl_802C0010[pad];
                    work->padDown = lbl_802C0008[pad];
                    work->outputHeading = (f32)(180.0 *
                        (atan2((f64)stickX, (f64)stickY) / M_PI));
                    work->outputSpeed = 0.017857144f *
                        sqrtf((stickX * stickX) + (stickY * stickY));
                    if (work->outputSpeed > 1.0f) {
                        work->outputSpeed = 1.0f;
                    }
                    combinedAngle = work->outputHeading + lbl_80266A58[0].y;
                    wrappedAngle = (f32)fmod((f64)combinedAngle, 360.0);
                    finalAngle = wrappedAngle;
                    work->outputHeading = finalAngle;
                    work->buttonsHeld &= ~PAD_BUTTON_Y;
                    work->padDown &= ~PAD_BUTTON_Y;
                }
            }
            break;
        }
    case 2:
        if (fn_80071A8C() == 5) {
            work->position.x *= 0.98;
            work->position.z *= 0.98;
            break;
        } else {
            work->phaseState++;
            work->phaseTimer = 0;
            work->countdown = 0x1770;
            work->animating = 0;
        }
    case 3:
        if (++work->phaseTimer < 0xA) {
            break;
        }
        work->phaseState++;
        work->phaseTimer = 0;
        fn_1_7628(work, motionWork->motion, 1.0f);
    default:
        if (fn_1_14ACC(NULL, NULL) == work) {
            fn_1_14B68(NULL, 1.0f, NULL, NULL);
        }
        work->itemState = 0;
        work->selected = 0;
        work->pointerMotionActive = 0;
        work->updateFlag = 0;
        work->actionTimer = 0;
        done = 1;
        break;
    }
    if (done == 0) {
        fn_1_7D04(work);
        fn_1_8170(work);
        fn_1_83C0(work);
    }
    fn_1_A160(work);
    fn_1_A4F4(work);
    return done;
}

s32 fn_1_717C(M699CharacterWorkPrefix *work)
{
    M699PlayerMotionData *subWork;
    s32 result;
    M699CharacterWorkPrefix *effectWork;
    s32 motionComplete;
    s32 frame;

    subWork = &work->motionData;
    result = 0;
    switch (work->phaseState) {
    case 0:
        work->phaseState++;
        work->phaseTimer = 0;
        work->countdown = 0x1770;
    case 1:
        fn_1_A160(work);
        fn_1_A4F4(work);
        if (fn_80071A8C() != 6 && work->stateFlag == 0) {
            work->phaseState++;
            work->phaseTimer = 0;
    case 2:
            fn_1_A160(work);
            fn_1_A4F4(work);
            if (++work->phaseTimer < 10) {
                break;
            }
            work->phaseState++;
            work->phaseTimer = 0;
            work->resetCounter = 0;
            fn_1_7628(work, subWork->motion, 1.0f);
    case 3:
            frame = work->phaseTimer++;
            motionComplete = fn_1_9B60(work, &lbl_1_data_2C4[work->groupNumber], 0.0f, 1.5f);
            if (motionComplete != 0 || frame >= 0xB4) {
                work->phaseState++;
                work->phaseTimer = 0;
                work->state |= 1;
                work->updateFlag = 0;
                if (work->readPadInput != 0) {
                    effectWork = work;
                    fn_1_1248(effectWork->groupNumberCopy);
                }
            }
        }
        break;
    case 4:
        fn_1_9B60(work, &lbl_1_data_2C4[work->groupNumber], 0.0f, 1.0f);
        if (fn_80071A8C() == 8 && fn_1_1268() != 0) {
            work->phaseState++;
            work->phaseTimer = 0;
            work->updateFlag = 0;
        default:
            result = 1;
        }
        break;
    }
    return result;
}

s32 fn_1_739C(M699CharacterWorkPrefix *state)
{
    M699PlayerMotionData *config = &state->motionData;
    M699CharacterWorkPrefix *work;
    s32 done = 0;

    switch (state->phaseState) {
    case 0:
        if (fn_1_1154(state->groupId) > 0) {
            fn_1_7628(state, config->winMotion, 1.0f);
        } else {
            fn_1_7628(state, config->loseMotion, 1.0f);
        }
        if (state->readPadInput) {
            work = state;
            work->initialized = 0;
            fn_80060F40(work->characterNumberCopy, 127);
        }
        state->phaseState++;
        state->phaseTimer = 0;
        break;
    case 1:
        if (++state->phaseTimer < 90) {
            break;
        }
        state->phaseState++;
        state->phaseTimer = 0;
    default:
        done = 1;
        break;
    }
    return done;
}

/* Update the reset counter and character sound panning. */
void fn_1_74B4(OMOBJ *self) {
    M699CharacterWorkPrefix *owner = self->data;
    M699CharacterWorkPrefix *motion = owner;
    owner->additionalStateB += 1;
    if (motion->resetCounter != 0 && fn_1_7864(self) != 0) {
        motion->resetCounter = 0;
    }
    fn_80060F14((s16)owner->characterNumberCopy,
                         fn_1_39D8(&motion->initialPosition));
}

void fn_1_753C(OMOBJ *object, s32 effect)
{
    M699CharacterWorkPrefix *work = object->data;
    M699CharacterWorkPrefix *owner = work;
    s32 handle = CharFXPlayPan((s16)work->characterNumberCopy, (s16)effect,
                               (s16)fn_1_39D8(&owner->initialPosition));

    fn_1_4B10(effect, handle, &owner->initialPosition, 1, 0);
}

void fn_1_75B0(OMOBJ *object)
{
    M699CharacterWorkPrefix *work;
    M699CharacterWorkPrefix *state;
    s32 model;

    work = object->data;
    state = work;
    model = object->mdlId[0];
    fn_1_F740(state, model);
}

void fn_1_760C(M699CharacterWorkPrefix *work) {
    work->motion = -1;
    work->motionSpeed = 1.0f;
}

void fn_1_7628(M699CharacterWorkPrefix *work, s32 motion, f32 speed) {
    work->motion = motion;
    work->motionSpeed = speed;
}

void fn_1_7634(M699CharacterWorkPrefix *state, s32 motion, f32 speed)
{
    if (speed < 0.8) {
        speed = 0.8f;
    }
    if (speed > 2.0) {
        speed = 2.0f;
    }
    state->motion = motion;
    state->motionSpeed = speed;
}

void fn_1_7680(OMOBJ *object)
{
    M699CharacterWorkPrefix *work = object->data;
    M699CharacterWorkPrefix *state = work;

    if (state->motion != -1) {
        fn_1_76E8(object, state->motion);
        fn_8002D86C(object->mdlId[0], state->motionSpeed);
    }
}

void fn_1_76E8(OMOBJ *object, s32 motionIndex)
{
    M699CharacterWorkPrefix *work;
    M699CharacterWorkPrefix *state;
    s32 modelId;
    s32 motionId;
    u32 attributes;
    f32 blend;

    work = object->data;
    state = work;
    modelId = *object->mdlId;
    if (motionIndex == -1) {
        state->motionState = -1;
        state->resetCounter = 0;
        return;
    }
    if (state->motionState != motionIndex && motionIndex < 0x11) {
        if (state->motionState < 0) {
            blend = 0.0f;
        } else {
            blend = 60.0f * lbl_1_data_138[motionIndex].blendWeight;
        }
        state->motionState = motionIndex;
        state->motionTime = 0;
        motionId = object->mtnId[lbl_1_data_138[motionIndex].motionListIndex];
        if (motionId == -1) {
            state->motionState = -1;
            state->resetCounter = 0;
        }
        attributes = lbl_1_data_138[motionIndex].attributes;
        fn_8005C2C8((s16)work->characterNumberCopy, motionId, 0.0f, blend, attributes);
        fn_8005C390((s16)work->characterNumberCopy, 1.0f);
        if ((attributes & ((1 << 30) | (1 << 2))) != 0) {
            fn_8002D414(modelId, Hu3DMotionMotionMaxTimeGet((s16)motionId));
        }
    }
}

s32 fn_1_7864(OMOBJ *object)
{
    M699CharacterWorkPrefix *work = object->data;
    M699CharacterWorkPrefix *currentWork = work;

    if (currentWork->motionState == -1) {
        return 1;
    }
    if (fn_8005C240((s16)work->characterNumberCopy) != 0 &&
        (s16)fn_8005C284((s16)work->characterNumberCopy) < 0) {
        return 1;
    }
    return 0;
}

void fn_1_78F0(M699CharacterWorkPrefix *work)
{
    Vec position;
    Vec velocity;
    f32 *heightPointer;
    M699CharacterWorkPrefix *spawnData;
    void *camera;
    f32 height;
    f32 angle;
    f32 speed;

    heightPointer = &work->motionData.proximityRadius;
    camera = fn_1_14ACC(&position, &velocity);
    height = (f32)(20.0 + (f64)*heightPointer);
    if (work->cooldown != 0) {
        work->cooldown--;
    }
    if (camera == NULL) {
        if (work->cooldown == 0 &&
            fn_1_147A8(&work->initialPosition, work->rotation.y, 120.0f,
                       (f32)(50.0 + (f64)height)) != 0) {
            spawnData = work->relatedObjects[(s32)(2.0 * (1.52587890625e-05 *
                (f32)(frand() & 0xFFFF))) & 1];
            work->spawnPosition = spawnData->initialPosition;
            work->spawnCount = 0;
            camera = fn_1_14B68(NULL, 1.0f, NULL, work);
            fn_1_14D50(work, 0);
        }
    } else if (camera == work) {
        angle = fn_1_5098(&work->initialPosition, &work->spawnPosition);
        angle = fn_1_4EC8(work->rotation.y, angle, 0.3f);
        work->rotation.y = angle;
        position.x = (f32)((f64)work->initialPosition.x +
            ((f64)height * sin((M_PI * (f64)angle) /
                               180.0)));
        position.y = (f32)(80.0 + (f64)work->initialPosition.y);
        position.z = (f32)((f64)work->initialPosition.z +
            ((f64)height * cos((M_PI * (f64)angle) /
                               180.0)));
        velocity.x = velocity.y = velocity.z = 0.0f;
        if (++work->spawnCount == 0xF) {
            work->cooldown = 0xC;
            speed = (f32)(13.333333333333334 +
                (3.3333333333333335 * (1.52587890625e-05 *
                (f32)(frand() & 0xFFFF))));
            velocity.x = (f32)((f64)speed *
                sin((M_PI * (f64)angle) /
                    180.0));
            velocity.y = (f32)(2.5 + (5.0 *
                (3.0517578125e-05 * (f32)((u16)frand() - 0x8000))));
            velocity.z = (f32)((f64)speed *
                cos((M_PI * (f64)angle) /
                    180.0));
            camera = NULL;
        }
        camera = fn_1_14B68(&position, 0.3f, &velocity, camera);
        fn_1_14D50(work, 0);
    } else {
        work->cooldown = 0x1E;
    }
}

void fn_1_7D04(M699CharacterWorkPrefix *work)
{
    f32 *heightPointer;
    Vec cameraPosition;
    void *camera;
    s32 initialFlags;
    s32 turnSide;
    s32 playerIndex;
    f64 turnOffset;

    heightPointer = &work->motionData.proximityRadius;
    camera = fn_1_14ACC(&cameraPosition, NULL);
    initialFlags = work->padDown;
    if (work->selectionValue2 != 0) {
        work->padDown = 0;
        work->buttonsHeld = 0;
    }
    switch (work->selectionValue2) {
        case 0:
            if (work->selected != 0) {
                work->selectionValue2++;
                work->stateFrame = 0;
                work->resetCounter = 0;
                work->state |= 2;
            } else {
                break;
            }
            /* fall through */
        case 1:
            work->heightAdjustment = 0.0f;
            fn_1_9B60(work, &cameraPosition,
                      0.0f, 1.5f);
            if (camera == work) {
                work->selectionValue2++;
                work->stateFrame = 0;
                work->updateFlag = 0;
            } else {
                break;
            }
            /* fall through */
        case 2:
            if (!((f64)++work->stateFrame < 6.0)) {
                work->selectionValue2++;
                work->stateFrame = 0;
                playerIndex = work->groupId * 2;
                if (work->initialPosition.z > 0.0f) {
                    playerIndex++;
                }
                work->targetPosition = lbl_1_data_338[playerIndex];
            } else {
                break;
            }
            /* fall through */
        case 3:
            if (fn_1_9B60(work, &work->targetPosition,
                          lbl_1_data_368[work->groupId], 1.5f) != 0) {
                work->selectionValue2++;
                work->stateFrame = 0;
            } else {
                break;
            }
            /* fall through */
        case 4:
            fn_1_539C(&work->initialPosition, &work->targetPosition,
                      0.5f, &work->initialPosition);
            if (!((f64)++work->stateFrame < 6.0)) {
                work->selectionValue2++;
                work->stateFrame = 0;
                work->selectionValue1 = 1;
                return;
            }
            break;
        case 5:
            fn_1_539C(&work->initialPosition, &work->targetPosition,
                      0.5f, &work->initialPosition);
            turnSide = 0;
            if ((u32)(initialFlags & 0x600) != 0) {
                turnSide = 1;
            }
            if ((f64)++work->stateFrame > 180.0) {
                turnSide = 1;
            }
            if (turnSide != 0) {
                if ((u32)(initialFlags & 0x400) == 0) {
                    initialFlags |= 0x200;
                }
                work->padDown |= initialFlags & 0x600;
                work->selectionValue2++;
                work->stateFrame = 0;
                fn_1_10E0(2);
                return;
            }
            break;
        case 6:
            fn_1_539C(&work->initialPosition, &work->targetPosition,
                      0.5f, &work->initialPosition);
            if (!((f64)++work->stateFrame < 30.0)) {
                work->selectionValue2++;
                work->stateFrame = 0;
                if (work->targetPosition.x < 0.0f) {
                    turnOffset = 150.0;
                } else {
                    turnOffset = -150.0;
                }
                work->targetPosition.x = (f32)((f64)work->targetPosition.x + turnOffset);
            } else {
                break;
            }
            /* fall through */
        case 7:
            if (fn_1_9B60(work, &work->targetPosition,
                          lbl_1_data_368[work->groupId], 1.5f) != 0) {
                work->selectionValue2 = 0;
                work->stateFrame = 0;
                work->selected = 0;
                work->state &= ~2u;
            }
            break;
        default:
            break;
    }
}

void fn_1_8170(M699CharacterWorkPrefix *work)
{
    M699PlayerMotionData *subWork;
    s32 mode;

    subWork = &work->motionData;
    mode = fn_1_10C0();
    switch (work->pointerMotionState) {
    case 0:
        if (mode == 6 && work->selected == 0 &&
            work->groupId == fn_1_14F80()) {
            work->pointerMotionState++;
            work->pointerMotionTimer = 0;
            work->pointerMotionActive = 1;
            work->resetCounter = 0;
    case 1:
            work->padDown = 0;
            work->buttonsHeld = 0;
            if (work->pointerMotionTimer >= 0x96 ||
                fn_1_9B60(work,
                          &lbl_1_data_264[work->groupNumber].position,
                          lbl_1_data_2AC[work->groupNumber], 1.5f) != 0) {
                work->pointerMotionState++;
                work->pointerMotionActive = 0;
                work->updateFlag = 0;
    case 2:
                if (lbl_1_data_264[work->groupNumber].position.x < 0.0) {
                    if (work->initialPosition.x > 0.0) {
                        work->initialPosition.x = (f32)((f64)work->initialPosition.x -
                            (2.0 * (f64)subWork->movementSpeed));
                        if (work->initialPosition.x < 0.0) {
                            work->initialPosition.x = 0.0f;
                        }
                    }
                } else if (work->initialPosition.x < 0.0) {
                    work->initialPosition.x = (f32)((f64)work->initialPosition.x +
                        (2.0 * (f64)subWork->movementSpeed));
                    if (work->initialPosition.x > 0.0) {
                        work->initialPosition.x = 0.0f;
                    }
                }
                if (mode != 6) {
                    work->pointerMotionState = 0;
                    work->pointerMotionTimer = 0;
                }
            }
        }
        break;
    }
    work->pointerMotionTimer++;
}

void fn_1_83C0(M699CharacterWorkPrefix *work)
{
    M699PlayerMotionData *profile = &work->motionData;
    Vec cameraPosition;
    Vec cameraRotation;
    void *cameraObject;
    void *previousTarget;
    f32 radius;
    f32 rotationMagnitude;
    f32 distance;
    f32 forwardSpeed;
    f32 lateralSpeed;
    f32 heading;
    f32 orbitHeading;
    s32 shouldTurn;

    shouldTurn = 0;
    if (work->cooldown != 0) {
        work->cooldown--;
    }
    switch (fn_1_10C0()) {
    case 3:
    case 4:
    case 5:
    case 6:
        if (work->selected == 0) {
            work->cooldown = 1;
        }
        break;
    default:
        break;
    }

    radius = (f32)(20.0 + (f64)profile->proximityRadius);
    cameraObject = fn_1_14ACC(&cameraPosition, &cameraRotation);
    rotationMagnitude = sqrtf((cameraRotation.x * cameraRotation.x) +
                              (cameraRotation.z * cameraRotation.z));
    if (cameraObject != work) {
        work->actMode = 0;
        work->spawnCount = 0;
        work->actionActive = 0;
        if (work->cooldown == 0 &&
            (cameraObject == NULL || work->actionTimer != 0) &&
            fn_1_147A8(&work->initialPosition, work->rotation.y, 120.0f,
                       (f32)(50.0 + (f64)radius)) != 0) {
            previousTarget = cameraObject;
            cameraObject = fn_1_14B68(NULL, 1.0f, NULL, work);
            fn_1_14D50(work, 0);
            fn_1_7628(work, profile->captureMotion, 1.0f);
            work->resetCounter = 1;
            if (previousTarget != NULL && cameraObject == work &&
                work->actionTimer != 0) {
                fn_1_435C(0x929, &work->initialPosition);
            }
            work->actionTimer = 0;
        }
        if (work->actionTimer == 0 && cameraObject != work &&
            (work->padDown & 0x200U) != 0) {
            fn_1_7628(work, profile->actionMotion, 1.0f);
            work->actionTimer = 0xC;
        }
        if (work->actionTimer != 0) {
            work->actionTimer--;
            work->position.x *= 0.9;
            work->position.z *= 0.9;
            if (work->actionTimer == 6) {
                work->position.x = (f32)(0.016666666666666666 *
                    (100.0 * (10.0 * sin((M_PI *
                                          (f64)work->rotation.y) / 180.0))));
                work->position.z = (f32)(0.016666666666666666 *
                    (100.0 * (10.0 * cos((M_PI *
                                          (f64)work->rotation.y) / 180.0))));
            }
        }
    }

    while (cameraObject == work) {
        if (fn_1_5230(&work->initialPosition, &cameraPosition) >
            (100.0 + (f64)radius)) {
            cameraObject = fn_1_14B68(NULL, 1.0f, NULL, NULL);
            break;
        } else {
            shouldTurn = 1;
            work->spawnCount++;
            if (work->actMode != 3) {
                forwardSpeed = work->position.x;
                lateralSpeed = work->position.z;
                radius = profile->scales[1];
                distance = (f32)(5.0 * (f64)sqrtf(
                    (forwardSpeed * forwardSpeed) +
                    (lateralSpeed * lateralSpeed)));
                if (distance > 0.8333333333333334) {
                    distance += profile->scales[3];
                }
                heading = work->rotation.y;
                orbitHeading = heading + profile->scales[0];
                cameraPosition.x = (f32)((f64)work->initialPosition.x +
                    (f64)radius * sin((M_PI *
                                             (f64)orbitHeading) / 180.0) +
                    (f64)distance * sin((M_PI *
                                          (f64)heading) / 180.0));
                cameraPosition.y = work->initialPosition.y + profile->scales[4];
                cameraPosition.z = (f32)((f64)work->initialPosition.z +
                    (f64)radius * cos((M_PI *
                                             (f64)orbitHeading) / 180.0) +
                    (f64)distance * cos((M_PI *
                                          (f64)heading) / 180.0));
                cameraRotation.x = cameraRotation.y = cameraRotation.z = 0.0f;
            }
            cameraObject = fn_1_8BB4(work, cameraObject,
                                      &cameraPosition, &cameraRotation);
            cameraObject = fn_1_909C(work, cameraObject,
                                      &cameraPosition, &cameraRotation);
            cameraObject = fn_1_96F4(work, cameraObject,
                                      &cameraPosition, &cameraRotation);
            if (cameraObject != work || work->stateFlag != 0 || work->actMode != 0) {
                shouldTurn = 0;
            }
            fn_1_14B68(&cameraPosition, 0.3f, &cameraRotation, cameraObject);
            fn_1_14D50(work, shouldTurn);
            break;
        }
    }
    if (cameraObject != work || work->actMode != 3) {
        work->itemState = 0;
    }
}

void *fn_1_8BB4(M699CharacterWorkPrefix *work, void *camera,
                Vec *cameraPosition, Vec *cameraVelocity)
{
    M699CharacterWorkPrefix * target;
    M699PlayerMotionData * profile;
    Vec targetPosition;
    Vec otherPosition;
    f32 launchDistance;
    f32 heading;
    f32 distanceRatio;
    f32 distance;
    f32 otherDistance;
    f32 distanceRatio2;
    f32 proposedDistance;
    profile = & work->motionData;
    if (camera != NULL)
    {
        if (work->actMode != 1)
        {
            if ((work->actMode == 0 || work->actMode == 2) && (work->padDown & 0x600U) != 0)
            {
                work->actMode = 1;
                work->actionFrame = 0;
                work->targetSlot = 0;
                work->direction = (work->padDown & 0x200U) != 0 ? 0 : 1;
                fn_1_7628(work, profile->chaseMotion, 1.0f);
                work->resetCounter = 1;
                work->actionActive = 1;
            }
        }
        else
        {
            target = work->relatedObjects[work->targetSlot];
            distance = fn_1_5230(& work->initialPosition, & target->initialPosition);
            distanceRatio = distance / profile->distanceScale;
            targetPosition.x = (f32)((f64) target->initialPosition.x + (0.7 * (f64)(target->position.x * distanceRatio)));
            targetPosition.y = target->initialPosition.y;
            targetPosition.z = (f32)((f64) target->initialPosition.z + (0.7 * (f64)(target->position.z * distanceRatio)));
            heading = fn_1_5098(& work->initialPosition, & targetPosition);
            otherDistance = fn_1_5230(& work->initialPosition, & targetPosition);
            distanceRatio2 = 1500.0f;
            if (otherDistance > distanceRatio2)
            {
                targetPosition.x = (f32)((f64) work->initialPosition.x + ((f64) distanceRatio2 * sin((M_PI * (f64) heading) / 180.0)));
                targetPosition.z = (f32)((f64) work->initialPosition.z + ((f64) distanceRatio2 * cos((M_PI * (f64) heading) / 180.0)));
            }
            work->rotation.y = fn_1_4EC8(work->rotation.y, heading, 0.3f);
            work->acceleration = (f32)((f64) work->acceleration + (0.3 * (0.5 - (f64) work->acceleration)));
            if (++work->actionFrame == 0xD)
            {
                work->cooldown = 0xC;
                otherPosition = * fn_1_14778(work->groupId ^ 1);
                otherDistance = fn_1_5230(cameraPosition, & otherPosition);
                if (otherDistance > 500.0)
                {
                    fn_1_14EB0(3);
                }
                if (work->direction == 0)
                {
                    distance = fn_1_5230(& work->initialPosition, & targetPosition);
                    distanceRatio = (f32)((f64) profile->distanceScale * (((f64) distance / 1000.0) + 0.5));
                    cameraVelocity->x = (f32)((f64) distanceRatio * sin((M_PI * (f64) heading) / 180.0));
                    cameraVelocity->y = (f32)(8.333333333333334 + (2.0 * (0.016666666666666666 * (f64) target->initialPosition.y)));
                    cameraVelocity->z = (f32)((f64) distanceRatio * cos((M_PI * (f64) heading) / 180.0));
                }
                else
                {
                    if (distance > 100.0)
                    {
                        launchDistance = (f32)(1.5 * (f64) distance);
                        if (launchDistance > 1200.0)
                        {
                            launchDistance = 1200.0f;
                        }
                        if (launchDistance < 300.0)
                        {
                            launchDistance = 300.0f;
                        }
                    }
                    else
                    {
                        launchDistance = 300.0f;
                    }
                    fn_1_53F4(cameraPosition, & targetPosition, 1.0896277f, launchDistance, cameraVelocity);
                    fn_1_14E58(work, target);
                }
                camera = fn_1_14B68(NULL, 1.0f, NULL, NULL);
                work->actionActive = 0;
                fn_1_435C(0x928, & work->initialPosition);
            }
        }
    }
    return camera;
}

void *fn_1_909C(M699CharacterWorkPrefix *work, void *camera,
                Vec *position, Vec *cameraVelocity)
{
    Vec otherPosition;
    Vec velocity;
    Vec targetPosition;
    f32 heightScale;
    f32 launchScale;
    f32 motionFrame;
    f32 flightSpeed;
    f32 distance;
    f32 heading;
    f32 adjustedHeading;
    M699PlayerMotionData *profile = &work->motionData;
    s32 randomFrame;
    s32 finished = 0;

    if (camera == work) {
        if (work->actMode != 2) {
            if (work->actMode == 0 && (work->padDown & 0x100U) != 0) {
                work->actMode = 2;
                work->actionFrame = 0;
                work->actionActive = 1;
                otherPosition = *fn_1_14778(work->groupId ^ 1);
                distance = fn_1_5230(position, &otherPosition);
                work->launchScale =
                    (f32)((f64)distance / 500.0);
                if (distance > 500.0) {
                    fn_1_14EB0(3);
                }
            }
        } else {
            otherPosition = *fn_1_14778(work->groupId ^ 1);
            distance = fn_1_5230(position, &otherPosition);
            heading = fn_1_5098(position, &otherPosition);
            if (distance > 50.0) {
                work->rotation.y = fn_1_4EC8(work->rotation.y, heading,
                                        0.3f);
            }
            work->position.x *= 0.98;
            work->position.z *= 0.98;
            work->actionFrame++;
            if ((work->buttonsHeld & 0x100U) != 0) {
                if (work->actionFrame >= 0x27) {
                    finished = 1;
                }
            } else {
                if (work->actionFrame >= 3) {
                    finished = 1;
                } else {
                    work->actMode = 0;
                    work->actionFrame = 0;
                }
                work->actionActive = 0;
            }

            if (work->actionFrame >= 3) {
                motionFrame = (f32)work->actionFrame;
                if ((f64)motionFrame > 27.0) {
                    motionFrame = (f32)(27.0 -
                        ((f64)motionFrame - 27.0));
                }
                launchScale = (f32)((f64)distance / 350.0);
                if ((f64)launchScale > 0.95) {
                    launchScale = 0.95f;
                }
                if ((f64)launchScale < 0.5) {
                    launchScale = 0.5f;
                }
                motionFrame *= 0.037037037037037035;
                if ((f64)motionFrame < 0.3) {
                    motionFrame = 0.3f;
                }
                if ((f64)motionFrame > 1.0) {
                    motionFrame = 1.0f;
                }
                if (distance > 300.0) {
                    flightSpeed = distance * motionFrame;
                } else {
                    flightSpeed = (f32)(1.3 *
                                        (f64)(distance * motionFrame));
                }
                if ((f64)flightSpeed > 550.0) {
                    flightSpeed = 550.0f;
                }
                if (flightSpeed < 100.0) {
                    flightSpeed = 100.0f;
                }
                flightSpeed *= launchScale;
                heightScale = (f32)(700.0 *
                    ((f64)distance / 500.0));
                if ((f64)heightScale > 700.0) {
                    heightScale = 700.0f;
                }
                if ((f64)heightScale < 300.0) {
                    heightScale = 300.0f;
                }
                adjustedHeading = heading;
                randomFrame = 0x1B;
                if (work->actionFrame >= randomFrame - 1 &&
                    work->actionFrame <= randomFrame + 1) {
                    adjustedHeading = (f32)((f64)adjustedHeading +
                        ((f64)work->launchScale *
                         (3.0517578125e-05 *
                          (f64)(f32)((u16)frand() - 0x8000))));
                } else {
                    adjustedHeading = (f32)((f64)adjustedHeading +
                        (2.0 *
                         (f64)work->launchScale *
                         (3.0517578125e-05 *
                          (f64)(f32)((u16)frand() - 0x8000))));
                }
                targetPosition.x = (f32)((f64)position->x +
                    ((f64)flightSpeed *
                     sin((M_PI * (f64)adjustedHeading) / 180.0)));
                targetPosition.y = (f32)(100.0 + (f64)otherPosition.y);
                targetPosition.z = (f32)((f64)position->z +
                    ((f64)flightSpeed *
                     cos((M_PI * (f64)adjustedHeading) / 180.0)));
                fn_1_53F4(position, &targetPosition, 1.0896277f,
                          heightScale, &velocity);
                if (finished != 0) {
                    work->cooldown = 0x1E;
                    *cameraVelocity = velocity;
                    camera = fn_1_14B68(NULL, 1.0f, NULL, NULL);
                    fn_1_7628(work, profile->exitMotion, 1.0f);
                    fn_1_435C(0x923, &work->initialPosition);
                }
            }
        }
    }
    return camera;
}

void *fn_1_96F4(M699CharacterWorkPrefix *work, void *camera,
                Vec *cameraPosition, Vec *velocity)
{
    Vec otherPosition;
    M699PlayerMotionData *profile;
    f32 targetAngle;
    f32 radius;

    profile = &work->motionData;
    if (camera == work) {
        otherPosition = *fn_1_14778(work->groupId ^ 1);
        if (work->actMode != 3) {
            if (!(work->cooldown != 0 || work->stateFlag == 0 ||
                  (work->groupId == 0 &&
                   work->initialPosition.x > otherPosition.x) ||
                  (work->groupId != 0 &&
                   work->initialPosition.x < otherPosition.x) ||
                  cameraPosition->y < (f64)otherPosition.y - 50.0 ||
                  fn_1_5230(cameraPosition, &otherPosition) > 120.0)) {
                work->actMode = 3;
                work->actionFrame = 0;
                targetAngle = fmodf(fn_1_5098(&work->initialPosition, &otherPosition), 360.0f);
                if (targetAngle < 0.0) {
                    targetAngle += 360.0;
                }
                if (work->groupId == 0) {
                    if (targetAngle < 30.0) {
                        targetAngle = 30.0f;
                    }
                    if (targetAngle > 150.0) {
                        targetAngle = 150.0f;
                    }
                } else {
                    if (targetAngle < 210.0) {
                        targetAngle = 210.0f;
                    }
                    if (targetAngle > 330.0) {
                        targetAngle = 330.0f;
                    }
                }
                work->targetAngle = targetAngle;
                fn_1_7628(work, profile->activeMotion, 1.0f);
                work->resetCounter = 1;
                fn_1_14DA8(work, 1);
                work->itemState = 1;
                fn_1_14E00(work, 0);
            }
        } else {
            work->heightAdjustment = 0.0f;
            cameraPosition->x = otherPosition.x;
            cameraPosition->y = (f32)(80.0 + (f64)otherPosition.y);
            cameraPosition->z = otherPosition.z;
            velocity->x = 0.0f;
            velocity->y = 0.0f;
            velocity->z = 0.0f;
            if (++work->actionFrame < 0x23) {
                work->rotation.y = fn_1_4EC8(work->rotation.y, work->targetAngle,
                                        0.2f);
                targetAngle = work->targetAngle;
                radius = profile->scales[6];
                {
                    Vec targetPosition;
                    targetPosition.x = (f32)((f64)otherPosition.x -
                        ((f64)radius * sin((M_PI *
                            (f64)targetAngle) / 180.0)));
                    targetPosition.y = (f32)((f64)otherPosition.y -
                        ((f64)profile->height - 70.0));
                    targetPosition.z = (f32)((f64)otherPosition.z -
                        ((f64)radius * cos((M_PI *
                            (f64)targetAngle) / 180.0)));
                    fn_1_539C(&work->initialPosition, &targetPosition, 0.05f,
                              &work->initialPosition);
                }
                work->position.x = work->position.y = work->position.z =
                    0.0f;
            } else {
                velocity->y = -16.666666f;
                work->cooldown = 0x3C;
                work->itemState = 0;
                camera = fn_1_14B68(NULL, 1.0f, NULL, NULL);
                fn_1_7628(work, profile->returnMotion, 1.0f);
                work->resetCounter = 1;
            }
        }
    }
    return camera;
}

s32 fn_1_9B60(M699CharacterWorkPrefix *work, Vec *data,
              f32 targetAngle, f32 speed)
{
    M699PlayerMotionData *segment = &work->motionData;
    f32 closeMotionSpeed;
    f32 movementSpeed;
    f32 farMotionSpeed;
    f32 deltaX;
    f32 deltaZ;
    f32 distance;
    f32 angle;
    f32 desiredVelocityX;
    f32 desiredVelocityZ;
    s32 closeMotion;
    s32 slowMotion;
    s32 farMotion;

    work->updateFlag = 1;
    if (fn_1_14ACC(0, 0) != work) {
        closeMotionSpeed = segment->closeMotionSpeed;
        farMotionSpeed = segment->farMotionSpeed;
        closeMotion = segment->motion;
        slowMotion = segment->slowMotion;
        farMotion = segment->farMotion;
    } else {
        closeMotionSpeed = segment->otherStartSpeed;
        farMotionSpeed = segment->otherEndSpeed;
        closeMotion = segment->firstTarget;
        slowMotion = segment->secondTarget;
        farMotion = segment->thirdTarget;
        speed *= 0.8;
    }
    deltaX = data->x - work->initialPosition.x;
    deltaZ = data->z - work->initialPosition.z;
    distance = sqrtf(deltaX * deltaX + deltaZ * deltaZ);
    if (distance < 120.0) {
        speed *= 0.8;
        if (distance < 70.0) {
            speed *= 0.5;
        }
    }
    if (distance < 50.0) {
        if (work->resetCounter == 0) {
            work->motion = slowMotion;
            work->motionSpeed = (f32)(0.5 * closeMotionSpeed);
        }
        work->initialPosition.x = (f32)(work->initialPosition.x + (0.2 * deltaX));
        work->initialPosition.z = (f32)(work->initialPosition.z + (0.2 * deltaZ));
        work->position.x *= 0.8;
        work->position.z *= 0.8;
        work->rotation.y = fn_1_4EC8(work->rotation.y, targetAngle, 0.2f);
        if (distance > 30.0) {
            return 0;
        }
        if (fn_1_4FE4(work->rotation.y, targetAngle) > 4.0) {
            return 0;
        }
        if (PSVECMag(&work->position) > 2.0) {
            return 0;
        }
        if (work->resetCounter == 0) {
            work->motion = closeMotion;
            work->motionSpeed = 1.0f;
        }
        work->position.x = work->position.z = 0.0f;
        work->rotation.y = targetAngle;
        work->updateFlag = 0;
        return 1;
    }
    if (work->resetCounter == 0) {
        work->motion = slowMotion;
        work->motionSpeed = closeMotionSpeed * speed;
    }
    angle = (f32)(180.0 *
        (atan2((f64)deltaX, (f64)deltaZ) / M_PI));
    if (work->collisionSource != NULL) {
        angle += 45.0;
    }
    work->rotation.y = fn_1_4EC8(work->rotation.y, angle, 0.3f);
    movementSpeed = speed * segment->movementSpeed;
    desiredVelocityX = (f32)(movementSpeed *
        sin((M_PI * (f64)angle) / 180.0));
    desiredVelocityZ = (f32)(movementSpeed *
        cos((M_PI * (f64)angle) / 180.0));
    work->position.x = (f32)(work->position.x +
        (0.3 * (f64)(desiredVelocityX - work->position.x)));
    work->position.z = (f32)(work->position.z +
        (0.3 * (f64)(desiredVelocityZ - work->position.z)));
    if (work->resetCounter == 0) {
        if (speed < (work->motionState == farMotion ? 0.3 : 0.6)) {
            work->motion = slowMotion;
            work->motionSpeed = speed * closeMotionSpeed;
        } else {
            work->motion = farMotion;
            work->motionSpeed = speed * farMotionSpeed;
        }
    }
    return 0;
}

void fn_1_A160(M699CharacterWorkPrefix *work)
{
    M699PlayerMotionData *segment = &work->motionData;
    f32 distance = work->outputSpeed * work->acceleration;
    f32 movement;
    f32 startSpeed;
    f32 endSpeed;
    f64 threshold;
    s32 fromPlayer;
    s32 toPlayer;
    s32 otherPlayer;
    s32 playerIndex;

    if (work->selected != 0) {
        return;
    }
    if (work->pointerMotionActive != 0) {
        return;
    }
    if (work->updateFlag != 0) {
        return;
    }
    if (work->actionTimer != 0) {
        return;
    }
    if (work->stateFlag != 0) {
        if (distance > 0.05f && work->actionActive == 0) {
            work->rotation.y = fn_1_4EC8(work->rotation.y, work->outputHeading,
                                    0.5f);
        }
        return;
    }

    if (fn_1_14ACC(0, 0) != work) {
        startSpeed = segment->closeMotionSpeed;
        endSpeed = segment->farMotionSpeed;
        fromPlayer = segment->motion;
        toPlayer = segment->slowMotion;
        otherPlayer = segment->farMotion;
    } else {
        startSpeed = segment->otherStartSpeed;
        endSpeed = segment->otherEndSpeed;
        fromPlayer = segment->firstTarget;
        toPlayer = segment->secondTarget;
        otherPlayer = segment->thirdTarget;
        distance *= 0.8;
    }

    if (toPlayer == -1) {
        if (otherPlayer == -1) {
            otherPlayer = fromPlayer;
            toPlayer = otherPlayer;
        } else {
            toPlayer = otherPlayer;
        }
    } else if (otherPlayer == -1) {
        otherPlayer = toPlayer;
    }

    playerIndex = work->resetCounter;
    if (work->motionState == segment->segmentPlayer) {
        playerIndex = 0;
    }
    if (distance >= 0.05f && work->countdown == 0) {
        work->resetCounter = 0;
        movement = distance * segment->movementSpeed;
        work->position.x = (f32)((f64)movement *
            sin((M_PI * (f64)work->outputHeading) / 180.0));
        work->position.z = (f32)((f64)movement *
            cos((M_PI * (f64)work->outputHeading) / 180.0));
        if (work->actionActive == 0) {
            work->rotation.y = fn_1_4EC8(work->rotation.y, work->outputHeading,
                                    0.5f);
        }
        if (playerIndex == 0) {
            if (work->motionState == otherPlayer) {
                threshold = 0.45;
            } else {
                threshold = 0.5;
            }
            if (distance < threshold) {
                fn_1_7634(work, toPlayer, distance * startSpeed);
                return;
            }
            fn_1_7634(work, otherPlayer, distance * endSpeed);
        }
    } else {
        if (playerIndex == 0) {
            work->motion = fromPlayer;
            work->motionSpeed = 1.0f;
        }
        work->position.x = work->position.z = 0.0f;
    }
}

void fn_1_A4F4(M699CharacterWorkPrefix *work)
{
    M699PlayerMotionData *segment = &work->motionData;
    Vec dustPosition;
    f32 targetVelocity;
    f32 targetAngle;
    f32 velocityX;
    f32 velocityZ;
    f32 blend;
    s32 motionAllowed = 0;

    if ((u32)fn_80071A8C() == 5U) {
        if (work->actMode == 3) {
            return;
        }
    } else {
        motionAllowed = 1;
    }
    if (work->selected != 0) {
        motionAllowed = 1;
    }
    if (work->pointerMotionActive != 0) {
        motionAllowed = 1;
    }
    if (work->updateFlag != 0) {
        motionAllowed = 1;
    }
    if (work->countdown > 0) {
        motionAllowed = 1;
    }

    targetVelocity = 0.7 * (work->outputSpeed * segment->movementSpeed);
    targetVelocity *= work->acceleration;
    targetAngle = work->outputHeading;
    targetAngle = fmodf(targetAngle + lbl_80266A58[0].y, 360.0f);

    if (work->stateFlag == 0) {
        if (motionAllowed == 0 && work->acceleration != 0.0f &&
            (work->padDown & 0x100U) != 0) {
            if (targetVelocity < 0.2) {
                targetAngle = work->rotation.y;
            }
            work->position.x = (f32)(targetVelocity *
                sin((M_PI * (f64)targetAngle) / 180.0));
            work->position.z = (f32)(targetVelocity *
                cos((M_PI * (f64)targetAngle) / 180.0));
            work->position.y = (f32)(segment->verticalSpeed * work->acceleration);
            work->stateFlag = 1;
            work->motionValue = 0;
            if (fn_1_14ACC(NULL, NULL) != work) {
                fn_1_7628(work, segment->motionA, 1.0f);
                return;
            }
            fn_1_7628(work, segment->motionB, 1.0f);
            work->position.y *= 0.85;
        }
    } else {
        if (motionAllowed != 0) {
            if (work->initialPosition.y > 0.0f) {
                work->position.y = (f32)(work->position.y + segment->acceleration);
                work->position.x *= 0.98;
                work->position.z *= 0.98;
            }
        } else {
            if (targetVelocity < 0.2) {
                targetAngle = work->rotation.y;
            }
            velocityX = targetVelocity *
                sin((M_PI * (f64)targetAngle) / 180.0);
            velocityZ = targetVelocity *
                cos((M_PI * (f64)targetAngle) / 180.0);
            blend = work->outputSpeed;
            work->position.x = work->position.x + blend * (velocityX - work->position.x);
            work->position.z = work->position.z + blend * (velocityZ - work->position.z);
            work->position.x *= 0.98;
            work->position.z *= 0.98;
            if (work->initialPosition.y > 0.0f) {
                if ((work->buttonsHeld & 0x100U) != 0 && work->position.y >= 0.0f) {
                    work->position.y = work->position.y + 0.55 * segment->acceleration;
                } else {
                    work->position.y = work->position.y + segment->acceleration;
                }
            }
        }

        if (work->position.y < 0.0f && work->initialPosition.y <= 0.0f) {
            work->initialPosition.y = 0.0f;
            work->position.x = work->position.y = work->position.z = 0.0f;
            work->stateFlag = 0;
            if (work->groupFlag >= 2U) {
                dustPosition = work->initialPosition;
                dustPosition.y += 10.0;
                fn_8006065C(-1, segment->dustEffect, &dustPosition);
            }
            if (work->acceleration != 0.0f) {
                fn_1_7628(work, segment->segmentPlayer, 1.0f);
                work->resetCounter = 1;
            }
        }
    }
}

s32 fn_1_AA54(M699MotionCallbackView *motion, M699MotionEventView *event)
{
    M699CharacterWorkPrefix *callbackState;
    M699CharacterWorkPrefix *motionState;
    M699PlayerMotionData *motionParameters;
    M699CharacterWorkPrefix *eventState;
    OMOBJ *eventOwner;
    M699MotionEventView *eventWork;
    OMOBJ *motionOwner;
    M699MotionCallbackView *motionWork;
    f32 verticalDifference;
    f32 motionHeight;
    f32 eventHeight;
    void *eventData;

    if (event->event == 0 && event->type != 1) {
        eventOwner = event->owner;
        eventData = eventOwner->data;
        eventState = eventData;
        eventWork = event;
        if (eventState->soundPlayed == 0) {
            motionOwner = motion->owner;
            motionState = motionOwner->data;
            callbackState = motionState;
            motionWork = motion;
            verticalDifference = eventWork->position->y - motionWork->position->y;
            if (verticalDifference < (0.7 * -eventWork->boundary)) {
                callbackState->lowerMotionTriggered = 1;
                if (callbackState->readPadInput != 0) {
                    fn_8006A030((s16)motionState->groupNumberCopy, 0x14, 0x14, 0);
                }
            }
            if (callbackState->selected == 0 &&
                verticalDifference > 0.7 * motionWork->boundary) {
                callbackState->upperMotionTriggered = 1;
                if (callbackState->readPadInput != 0) {
                    fn_8006A030((s16)motionState->groupNumberCopy, 0x14, 0x14, 0);
                    if (callbackState->soundPlayed == 0 &&
                        callbackState->characterEffectPlayed == 0) {
                        fn_1_753C(motionOwner, 0x24C);
                    }
                }
                if (fn_1_14ACC(0, 0) == callbackState) {
                    fn_1_14B68(NULL, 1.0f, NULL, NULL);
                    callbackState->cooldown = 0x1E;
                }
            }
            callbackState->collisionSource = eventState;
        }
    }
    if (event->event == 0xC) {
        OMOBJ *motionOwner;
        M699CharacterWorkPrefix *motionData;
        M699MotionCallbackView *motionWork;
        M699MotionEventView *eventWork;
        void *objectData;

        motionOwner = motion->owner;
        objectData = motionOwner->data;
        motionData = objectData;
        motionParameters = &motionData->motionData;
        motionWork = motion;
        eventWork = event;
        motionHeight = motionWork->position->y + motionWork->verticalOffset;
        eventHeight = (f32)((f64)eventWork->duration - 1.0);
        if (motionHeight >= eventHeight) {
            if (motionWork->position->x > 0.0) {
                motionWork->velocity->x = (f32)(0.7 * -motionParameters->movementSpeed);
            } else {
                motionWork->velocity->x = (f32)(0.7 * motionParameters->movementSpeed);
            }
            motionWork->velocity->y = (f32)(0.5 * motionParameters->verticalSpeed);
            motionData->countdown = 0x18;
        }
    }
    return 1;
}

void fn_1_AD3C(M699CharacterWorkPrefix *work)
{
    M699PlayerMotionData *angleInfo;
    M699CharacterWorkPrefix *angleSource;
    f32 angle;
    f32 horizontalScale;
    f32 verticalSpeed;
    s32 motionId;

    if (work->lowerMotionTriggered != 0) {
        if (0.0 != work->heightAdjustment) {
            angleInfo = &work->motionData;
            angleSource = work->collisionSource;
            angle = fn_1_5098(&work->initialPosition, &angleSource->initialPosition);
            horizontalScale = 5.0f;
            verticalSpeed = 16.666666f;
            work->position.x -= horizontalScale *
                sin((M_PI * angle) / 180.0);
            work->position.z -= horizontalScale *
                cos((M_PI * angle) / 180.0);
            if (++work->motionValue <= 1) {
                work->position.y += verticalSpeed;
                if (work->position.y > verticalSpeed) {
                    work->position.y = verticalSpeed;
                }
                verticalSpeed *= 0.5;
                if (work->position.y < verticalSpeed) {
                    work->position.y = verticalSpeed;
                }
                work->stateFlag = 1;
                motionId = angleInfo->motionA;
                work->motion = motionId;
                work->motionSpeed = 1.0f;
            }
            work->countdown = 4;
        }
        work->lowerMotionTriggered = 0;
    }
}

void fn_1_AEF4(M699CharacterWorkPrefix *work)
{
    M699PlayerMotionData *blendState = &work->motionData;
    M699PlayerMotionData *profile;
    M699CharacterWorkPrefix *motionResource;
    f32 targetBlend = 1.0f;
    f32 targetValue = 1.0f;
    f32 value;

    if (work->upperMotionTriggered != 0) {
        if ((work->soundPlayed == 0) && (work->characterEffectPlayed == 0) &&
            (work->position.y <= 0.0f)) {
            motionResource = work->collisionSource;
            profile = &motionResource->motionData;
            work->soundPlayed = 1;
            value = (f32)(1.0 +
                (1.0 - blendState->currentBlend));
            work->requestedBlend = profile->blendScale * value;
            work->blendRate = (f32)(value *
                ((1.0 / profile->blendScale) *
                    ((f64)profile->blendSpan - 1.0)));
        }
        work->position.x *= 0.5;
        work->position.z *= 0.5;
        work->upperMotionTriggered = 0;
    }

    switch (work->soundPlayed) {
    case 0:
        break;
    case 1:
        work->soundPlayed++;
        work->soundPlaybackTimer = 0;
        /* fall through */
    case 2:
        targetBlend = 0.5f;
        targetValue = (f32)(0.1282051282051282 * (f64)work->soundPlaybackTimer);
        targetValue = (f32)(1.0 -
            ((f64)targetValue *
                (1.0 - work->requestedBlend)));
        if (!((f64)++work->soundPlaybackTimer < 7.800000000000001)) {
            work->soundPlayed++;
            work->soundPlaybackTimer = 0;
        }
        break;
    case 3:
        targetBlend = 0.5f;
        targetValue = work->requestedBlend;
        if (!((f64)++work->soundPlaybackTimer < 90.0)) {
            work->soundPlayed++;
            work->soundPlaybackTimer = 0;
        }
        break;
    case 4:
        targetBlend = 0.5f;
        targetValue = (f32)(0.050505050505050504 * work->soundPlaybackTimer);
        targetValue = (f32)(0.5 *
            (1.0 + -cos((M_PI * 360.0 / 180.0) *
                (1.5 * (f64)targetValue))));
        targetValue = (f32)(work->requestedBlend +
            ((f64)targetValue *
                (1.0 - work->requestedBlend)));
        if (!((f64)++work->soundPlaybackTimer < 19.8)) {
            work->soundPlayed = 0;
            work->characterEffectPlayed = 0x5A;
            targetValue = 1.0f;
        }
        break;
    }

    work->acceleration = (f32)((f64)work->acceleration +
        (0.2 * (f64)(targetBlend - work->acceleration)));
    work->motionAdjustment = targetValue;
}

void fn_1_B340(M699CharacterWorkPrefix *work)
{
    M699MovementProfile *dataRecord;
    f32 angle;

    dataRecord = &lbl_1_data_5F4[work->controllerType];
    angle = 0.0f;
    work->updateFlag = 0;
    work->requestedHeading = 0.0f;
    work->requestedSpeed = 0.0f;
    if (work->timer != 0) {
        work->timer--;
    }
    switch (work->characterMotionState) {
    case 0:
        work->characterMotionState++;
        if (work->groupFlag == 0) {
            fn_1_B518(work, fn_1_BDE0);
        } else {
            fn_1_B518(work, fn_1_BFC0);
        }
        /* fall through */
    case 1:
        work->callback_ec(work);
        angle = fn_1_4FE4(work->requestedHeading, work->previousHeading);
        if (angle * work->previousAngle >= 0.0) {
            work->outputHeading = fn_1_4EC8(work->rotation.y, work->requestedHeading, 0.5f);
        } else {
            work->outputHeading = fn_1_4EC8(work->rotation.y, work->requestedHeading, 0.1f);
        }
        work->outputSpeed = (f32)((f64)work->previousSpeed +
            (0.5 * (f64)(work->requestedSpeed - work->previousSpeed)));
        if (work->outputSpeed < 0.1) {
            work->outputSpeed = 0.0f;
        }
        break;
    }
    work->previousHeading = work->outputHeading;
    work->previousAngle = angle;
    work->previousSpeed = work->outputSpeed;
}

void fn_1_B518(M699CharacterWorkPrefix *work,
               M699CharacterWorkCallback callback) {
    work->callback_ec = callback;
    work->callbackState = 0;
    work->callbackStateTimer = 0;
    work->callbackValue = 0.0f;
    work->transitionDuration = 0;
    work->randomTurn = 0;
    work->avoidanceTimer = 0;
}

s32 fn_1_B558(M699CharacterWorkPrefix *work, Vec *target,
              f32 heading, f32 speed)
{
    const M699MovementProfile *dataRecord = &lbl_1_data_5F4[work->controllerType];
    f32 distanceX;
    f32 distanceZ;
    f32 headingAdjustment;
    f32 distance;
    Vec targetPosition;

    headingAdjustment = 0.0f;
    if (work->collisionSource != NULL) {
        headingAdjustment = 45.0f;
    }
    targetPosition = *target;
    distanceX = targetPosition.x - work->initialPosition.x;
    distanceZ = targetPosition.z - work->initialPosition.z;
    distance = sqrtf((distanceX * distanceX) + (distanceZ * distanceZ));
    if (distance > 100.0) {
        work->requestedHeading = headingAdjustment + fn_1_5098(&work->initialPosition, &targetPosition);
        work->requestedSpeed = speed;
        work->arrivalFlag = 0;
        return 0;
    }
    if (distance > 60.0) {
        work->requestedHeading = headingAdjustment + fn_1_5098(&work->initialPosition, &targetPosition);
        work->requestedSpeed = ((f32)(0.5 * (f64)speed));
        work->arrivalFlag = 0;
        return 0;
    }
    if (work->arrivalFlag == 0 && distance > 30.0) {
        work->requestedHeading = headingAdjustment + fn_1_5098(&work->initialPosition, &targetPosition);
        work->requestedSpeed = 0.3f;
        return 0;
    }
    work->arrivalFlag = 1;
    if (fabs(fn_1_4FE4(work->rotation.y, heading)) > 10.0) {
        work->requestedHeading = heading;
        work->requestedSpeed = 0.05f;
        return 0;
    }
    return 1;
}

M699SequenceCallback fn_1_B8E8(M699CharacterWorkPrefix *work)
{
    if (fn_1_10C0() != 6) {
        return NULL;
    }
    if (work->selected != 0) {
        return fn_1_C038;
    }
    return fn_1_C188;
}

void fn_1_B944(M699CharacterWorkPrefix *work)
{
    M699SequenceCallback callback;
    M699SequenceCallback selected;

    if (fn_1_10C0() != 6) {
        callback = NULL;
    } else {
        if (work->selected != 0) {
            callback = fn_1_C038;
        } else {
            callback = fn_1_C188;
        }
    }
    selected = callback;
    if (selected == NULL) {
        switch (fn_1_F568(work)) {
        case 0:
            selected = fn_1_BA5C;
            break;
        case 1:
        case 2:
            selected = fn_1_BB64;
            break;
        case 3:
            selected = fn_1_BC70;
            break;
        default:
            selected = fn_1_EB74;
            break;
        }
    }
    work->callback_ec = selected;
    work->callbackState = 0;
    work->callbackStateTimer = 0;
    work->callbackValue = 0.0f;
    work->transitionDuration = 0;
    work->randomTurn = 0;
    work->avoidanceTimer = 0;
    work->callback_ec(work);
}

void fn_1_BA5C(M699CharacterWorkPrefix *work)
{
    Vec position;
    void (*initialCallback)(M699CharacterWorkPrefix *work);
    void (*callback)(M699CharacterWorkPrefix *work);
    void *relatedObject;
    s32 relation;

    if (fn_1_10C0() != 6) {
        initialCallback = NULL;
    } else {
        if (work->selected != 0) {
            initialCallback = fn_1_C038;
        } else {
            initialCallback = fn_1_C188;
        }
    }
    callback = initialCallback;
    while (callback == NULL) {
        relatedObject = fn_1_14ACC(&position, NULL);
        relation = fn_1_F568(work);
        if (relatedObject != NULL) {
            callback = fn_1_B944;
        } else if (relation == 0) {
            callback = fn_1_C588;
        } else {
            callback = fn_1_EB74;
        }
        break;
    }
    work->callback_ec = callback;
    work->callbackState = 0;
    work->callbackStateTimer = 0;
    work->callbackValue = 0.0f;
    work->transitionDuration = 0;
    work->randomTurn = 0;
    work->avoidanceTimer = 0;
}

void fn_1_BB64(M699CharacterWorkPrefix *work)
{
    Vec position;
    void (*initialCallback)(M699CharacterWorkPrefix *work);
    void (*callback)(M699CharacterWorkPrefix *work);
    void *relatedObject;
    s32 relation;

    if (fn_1_10C0() != 6) {
        initialCallback = NULL;
    } else if (work->selected != 0) {
        initialCallback = fn_1_C038;
    } else {
        initialCallback = fn_1_C188;
    }
    callback = initialCallback;
    while (callback == NULL) {
        relatedObject = fn_1_14ACC(&position, NULL);
        relation = fn_1_F568(work);
        if (relation != 1 && relation != 2) {
            callback = fn_1_B944;
        } else if (relatedObject == work) {
            callback = fn_1_CA68;
        } else {
            callback = fn_1_D254;
        }
        break;
    }
    work->callback_ec = callback;
    work->callbackState = 0;
    work->callbackStateTimer = 0;
    work->callbackValue = 0.0f;
    work->transitionDuration = 0;
    work->randomTurn = 0;
    work->avoidanceTimer = 0;
}

void fn_1_BC70(M699CharacterWorkPrefix *controller)
{
    Vec position;
    void *queryResult;
    s32 relation;
    M699CharacterWorkPrefix *currentObject;
    M699UpdateCallback nextUpdate;
    M699UpdateCallback update;

    if (fn_1_10C0() != 6) {
        nextUpdate = NULL;
    } else if (controller->selected != 0) {
        nextUpdate = fn_1_C038;
    } else {
        nextUpdate = fn_1_C188;
    }
    update = nextUpdate;
    while (update == NULL) {
        currentObject = controller->relatedObjects[0];
        queryResult = fn_1_14ACC(&position, NULL);
        relation = fn_1_F568(controller);
        if (relation != 3) {
            update = fn_1_B944;
        } else if ((f64)(f32)(frand() & 0xFFFF) * 1.52587890625e-05 < 0.3) {
            update = (M699UpdateCallback)fn_1_DBA8;
        } else if (currentObject->callback_ec == (M699UpdateCallback)fn_1_E300) {
            update = (M699UpdateCallback)fn_1_DBA8;
        } else {
            update = (M699UpdateCallback)fn_1_E300;
        }
        break;
    }
    controller->callback_ec = update;
    controller->callbackState = 0;
    controller->callbackStateTimer = 0;
    controller->callbackValue = 0.0f;
    controller->transitionDuration = 0;
    controller->randomTurn = 0;
    controller->avoidanceTimer = 0;
}

void fn_1_BDE0(M699CharacterWorkPrefix *work)
{
    M699MovementProfile *dataRecord;
    Vec position;

    dataRecord = &lbl_1_data_5F4[work->controllerType];
    fn_1_14ACC(&position, NULL);
    switch (work->callbackState) {
    case 0:
        work->callbackState++;
        work->callbackStateTimer = (s32)(48.0 + (f64)(s32)(
            8.0 * (1.52587890625e-05 *
            (f64)(f32)(frand() & 0xFFFF))));
        /* fall through */
    case 1:
        if ((work->callbackStateTimer -= 1) > 0) {
            break;
        }
        work->callbackState++;
        work->callbackStateTimer = 0;
        work->padDown |= 0x100;
        /* fall through */
    case 2:
        work->buttonsHeld |= 0x100;
        if (++work->callbackStateTimer < 30) {
            break;
        }
        work->callbackState++;
        work->callbackStateTimer = 0;
        /* fall through */
    case 3:
        if (++work->callbackStateTimer < 30) {
            break;
        }
        work->callback_ec = fn_1_B944;
        work->callbackState = 0;
        work->callbackStateTimer = 0;
        work->callbackValue = 0.0f;
        work->transitionDuration = 0;
        work->randomTurn = 0;
        work->avoidanceTimer = 0;
        break;
    }
}

/* Reset callback state when the game enters state 2. */
void fn_1_BFC0(M699CharacterWorkPrefix *work) {
    if (fn_1_10C0() == 2) {
        work->callback_ec = fn_1_B944;
        work->callbackState = 0;
        work->callbackStateTimer = 0;
        work->callbackValue = 0.0f;
        work->transitionDuration = 0;
        work->randomTurn = 0;
        work->avoidanceTimer = 0;
    }
}

void fn_1_C038(M699CharacterWorkPrefix *work)
{
    struct {
        M699MovementProfile *data;
        Vec position;
    } query;
    void *relatedObject;

    query.data = &lbl_1_data_5F4[work->controllerType];
    relatedObject = fn_1_14ACC(&query.position, NULL);
    switch (work->callbackState) {
    case 0:
        if (work->selectionValue1 != 0) {
            work->callbackState += 1;
            work->callbackStateTimer = 0;
        }
        break;
    case 1:
        if (++work->callbackStateTimer >= 30) {
            work->callbackState += 1;
            work->callbackStateTimer = 0;
        } else {
            break;
        }
    case 2:
        work->padDown |= 0x200;
        if (relatedObject != work) {
            break;
        }
        work->callbackState += 1;
        work->callbackStateTimer = 0;
    case 3:
        if (++work->callbackStateTimer >= 0x12) {
            work->callback_ec = fn_1_B944;
            work->callbackState = 0;
            work->callbackStateTimer = 0;
            work->callbackValue = 0.0f;
            work->transitionDuration = 0;
            work->randomTurn = 0;
            work->avoidanceTimer = 0;
        }
        break;
    }
}

void fn_1_C188(M699CharacterWorkPrefix *work)
{
    M699CharacterWorkPrefix *relatedObject;
    const M699MovementProfile *profile;

    profile = &lbl_1_data_5F4[work->controllerType];
    M699DebugAddress(profile);
    relatedObject = work->relatedObjects[0];
    if (fn_1_10C0() == 6) {
        work->callbackStateTimer = 0;
    }
    if (++work->callbackStateTimer > 0xC) {
        fn_1_B518(work, fn_1_B944);
        return;
    }

    work->destinationPosition = lbl_1_data_204[work->groupNumber];
    work->currentAngle = fn_1_5098(&work->initialPosition, &relatedObject->initialPosition);
    fn_1_EFDC(work);
    fn_1_B558(work, &work->destinationPosition,
              work->currentAngle, 1.0f);
}

void fn_1_C588(M699CharacterWorkPrefix *work)
{
    Vec queryPosition;
    Vec queryRotation;
    M699CharacterWorkPrefix *relatedObject;
    M699CharacterWorkPrefix *foundObject;
    void (*callback)(M699CharacterWorkPrefix *);
    M699MovementProfile *dataRecord;
    f32 distanceToTarget;
    f32 relatedDistance;
    f32 heading;

    dataRecord = &lbl_1_data_5F4[work->controllerType];
    relatedObject = work->relatedObjects[0];
    foundObject = fn_1_14ACC(&queryPosition, &queryRotation);
    callback = fn_1_B8E8(work);
    if (callback != NULL) {
        fn_1_B518(work, callback);
        return;
    }
    if (foundObject != NULL) {
        fn_1_B518(work, fn_1_B944);
        return;
    }

    distanceToTarget = fn_1_5230(&work->initialPosition, &queryPosition);
    relatedDistance = fn_1_5230(&relatedObject->initialPosition, &queryPosition);
    heading = fn_1_5098(&work->initialPosition, &queryPosition);
    if (distanceToTarget < relatedDistance) {
        fn_1_14ACC(&work->destinationPosition, NULL);
    } else {
        work->destinationPosition = lbl_1_data_204[work->groupNumber];
        work->destinationPosition.z = (f32)(0.5 * queryPosition.z);
    }
    fn_1_B558(work, &work->destinationPosition, heading, 1.0f);
}

void fn_1_CA68(M699CharacterWorkPrefix *work)
{
    M699MovementProfile *profile;
    M699CharacterWorkPrefix *relatedObject;
    M699CharacterWorkPrefix *relatedObject2;
    Vec cameraPosition;
    Vec playerPosition;
    Vec cameraRotation;
    void *cameraObject;
    M699CharacterWorkPrefix *foundObject;
    void (*nextCallback)(M699CharacterWorkPrefix *);
    s32 relation;
    s32 shouldTurn;
    s32 turnFlag;
    f32 distance;
    f32 distanceToPlayer;
    f32 distanceToRelated;
    f32 distanceToOther;
    f32 searchDistance;
    s32 otherTurnFlag;

    profile = &lbl_1_data_5F4[work->controllerType];
    relatedObject = work->relatedObjects[0];
    relatedObject2 = work->relatedObjects[1];
    cameraObject = fn_1_14ACC(&cameraPosition, &cameraRotation);
    relation = fn_1_F568(work);
    nextCallback = fn_1_B8E8(work);
    if (nextCallback != NULL) {
        fn_1_B518(work, nextCallback);
        return;
    }
    if (cameraObject != work) {
        fn_1_B518(work, fn_1_B944);
        return;
    }
    if (work->actMode != 1) {
        if (work->actMode == 2) {
            if (work->actionFrame < work->transitionDuration) {
                work->buttonsHeld |= 0x100;
                work->requestedSpeed = 0.0f;
            }
            return;
        }

        if (work->callbackState == 0) {
            for (; ++work->callbackStateTimer < 30; ) {
                if (work->callbackStateTimer == 1) {
                    if ((work->groupId == 0 && work->initialPosition.x < 0.0f) ||
                        (work->groupId == 1 && work->initialPosition.x > 0.0f)) {
                        break;
                    }
                    foundObject = fn_1_F968(work->groupId ^ 1, NULL, &cameraPosition,
                        work->rotation.y, 50.0f, 200.0f, 1);
                    if (foundObject == NULL) {
                        break;
                    }
                    work->destinationPosition = foundObject->initialPosition;
                }
                work->requestedHeading = (f32)(180.0 + fn_1_5098(&work->initialPosition,
                                                        &work->destinationPosition));
                work->requestedSpeed = 1.0f;
                return;
            }
            work->callbackState++;
            work->callbackStateTimer = 0;
        }

    playerPosition = *fn_1_14778(work->groupId ^ 1);
    distanceToPlayer = (f32)fabs(playerPosition.x - work->initialPosition.x);
    distanceToRelated = (f32)fabs(playerPosition.x - relatedObject->initialPosition.x);
    distanceToOther = (f32)fabs(playerPosition.x - relatedObject2->initialPosition.x);
    turnFlag = 0x200;
    otherTurnFlag = 0x400;
    shouldTurn = 0;
    if (work->controllerType != 0U) {
        if (work->avoidanceTimer == 0) {
        foundObject = fn_1_F968(work->groupId ^ 1, NULL, &work->initialPosition,
                                work->rotation.y,
                                (f32)(50.0 * (f64)profile->searchRangeScale),
                                (f32)(250.0 + 100.0 * (f64)profile->searchRangeScale),
                                1);
        if (foundObject != NULL) {
            if (fn_1_F968(work->groupId ^ 1, NULL,
                          &relatedObject->initialPosition,
                          0.0f, 180.0f,
                          (f32)(200.0 + 100.0 * (f64)profile->searchRangeScale),
                          1) == NULL) {
                shouldTurn = 1;
            }
            if (fn_1_F660(work, relatedObject, 40.0f) == 0) {
                shouldTurn = 0;
            }
            if (shouldTurn == 0) {
                work->avoidanceTimer = 0x2A;
                if (work->initialPosition.z < foundObject->initialPosition.z) {
                    work->avoidanceAngle = 180.0f;
                } else {
                    work->avoidanceAngle = 0.0f;
                }
                if (work->initialPosition.z > 450.0) {
                    work->avoidanceAngle = 180.0f;
                }
                if (work->initialPosition.z < -450.0) {
                    work->avoidanceAngle = 0.0f;
                }
            }
        }
        }
        if (work->avoidanceTimer != 0) {
        work->requestedHeading = work->avoidanceAngle;
        work->requestedSpeed = 1.0f;
        work->avoidanceTimer--;
        return;
    }
    } else if (distanceToPlayer > 100.0 + distanceToRelated) {
        shouldTurn = 1;
    }
        playerPosition = *fn_1_14778(work->groupId ^ 1);
        distance = fn_1_5230(&cameraPosition, &playerPosition);
        searchDistance = (f32)(400.0 + 250.0 * (f64)profile->pursuitRangeScale);
        if (distance < 300.0) {
            if (work->stateFlag == 0) {
                playerPosition = lbl_1_data_30C;
                shouldTurn = 1;
            }
        } else if (distance < searchDistance) {
            work->buttonsHeld |= 0x100;
            work->padDown |= 0x100;
            work->destinationPosition = playerPosition;
            work->transitionDuration = (s32)(26.0 + 8.0 *
                ((1.0 - (f64)profile->transitionTimingAccuracy) *
                 (3.0517578125e-05 * (f64)(f32)((u16)frand() - 0x8000))));
        }
        work->requestedHeading = fn_1_5098(&work->initialPosition, &playerPosition);
        work->requestedSpeed = 1.0f;
        if (shouldTurn != 0 &&
            (1.52587890625e-05 * (f32)(s32)(frand() & 0xFFFF)) < profile->turnProbability &&
            fn_1_F660(work, relatedObject, 40.0f) != 0) {
            work->padDown |= turnFlag;
            return;
        }
    }
}

void fn_1_D254(M699CharacterWorkPrefix *work)
{
    M699UpdateCallback callback;
    M699MovementProfile *dataRecord;
    Vec position;
    Vec peerPosition;
    Vec rotation;
    M699CharacterWorkPrefix *peer;
    void *queryResult;
    s32 relation;

    dataRecord = &lbl_1_data_5F4[work->controllerType];
    queryResult = fn_1_14ACC(&position, &rotation);
    relation = fn_1_F568(work);
    if ((callback = fn_1_B8E8(work)) != NULL) {
        fn_1_B518(work, callback);
        return;
    }
    if (relation != 2) {
        fn_1_B518(work, fn_1_B944);
        return;
    }

    peer = work->relatedObjects[0];
    peerPosition = peer->initialPosition;
    peerPosition.x = (f32)((f64)peerPosition.x + ((u32)work->groupId == 0U ? 300.0 : (-300.0)));
    work->destinationPosition = peerPosition;
    work->currentAngle = fn_1_5098(&work->initialPosition, &position);
    fn_1_EFDC(work);
    fn_1_B558(work, &work->destinationPosition, work->currentAngle, 1.0f);
}

void fn_1_D720(M699CharacterWorkPrefix *work)
{
    M699UpdateCallback callback;
    const M699MovementProfile *aimParameters;
    Vec currentPosition;
    Vec currentRotation;
    Vec targetPosition;
    s32 relation;
    s32 playerIndex;
    f32 targetAngle;

    aimParameters = &lbl_1_data_5F4[work->controllerType];
    M699DebugParameterAddress(aimParameters);
    relation = fn_1_F568(work);
    callback = fn_1_B8E8(work);
    if (callback != NULL) {
        fn_1_B518(work, callback);
        return;
    }
    if (relation != 2) {
        fn_1_B518(work, fn_1_B944);
        return;
    }

    playerIndex = work->groupNumber;
    targetPosition = lbl_1_data_204[playerIndex];
    fn_1_14ACC(&currentPosition, &currentRotation);
    targetAngle = fn_1_5098(&work->initialPosition, &currentPosition);
    targetPosition.z = (f32)(0.5 * (f64)currentPosition.z);
    fn_1_B558(work, &targetPosition, targetAngle, 1.0f);
}

void fn_1_DBA8(M699CharacterWorkPrefix *work)
{
    M699MovementProfile *profile;
    M699UpdateCallback nextUpdate;
    void *relatedObject0;
    void *relatedObject1;
    Vec cameraPosition;
    Vec cameraRotation;
    void *queryResult;
    s32 relation;
    s32 randomDirection;

    profile = &lbl_1_data_5F4[work->controllerType];
    relatedObject0 = work->relatedObjects[0];
    relatedObject1 = work->relatedObjects[1];
    queryResult = fn_1_14ACC(&cameraPosition, &cameraRotation);
    relation = fn_1_F568(work);
    nextUpdate = fn_1_B8E8(work);
    if (nextUpdate != NULL) {
        fn_1_B518(work, nextUpdate);
        return;
    }
    if (relation != 3) {
        fn_1_B518(work, fn_1_B944);
        return;
    }

    if (--work->callbackStateTimer <= 0) {
        if ((1.52587890625e-05 * (f64)(f32)(frand() & 0xFFFF)) < profile->directionProbabilityAndRetargetRate) {
            randomDirection = 0;
        } else {
            randomDirection = 1;
        }
        work->randomTurn = randomDirection;
        work->callbackStateTimer = 60;
    }
    if ((work->callbackValue -= profile->directionProbabilityAndRetargetRate) < 0.0) {
        work->callbackValue = 2.0f;
        work->destinationPosition = cameraPosition;
    }

    fn_1_B558(work, &work->destinationPosition,
              0.0f, 1.0f);

    if (fabs(work->initialPosition.x) > 800.0 || fabs(work->initialPosition.z) > 550.0) {
        if (fn_1_5230(&work->initialPosition, &cameraPosition) < 250.0) {
            if (work->stateFlag != 0) {
                work->buttonsHeld |= 0x100;
                work->padDown |= 0x100;
            } else if ((1.52587890625e-05 * (f64)(f32)(frand() & 0xFFFF)) < profile->nearbyActionProbability) {
                work->buttonsHeld |= 0x100;
                work->padDown |= 0x100;
                work->timer = 60;
            }
            return;
        }
        goto updateFlags;
    }
    if (work->randomTurn != 0) {
        work->requestedHeading += 15.0;
    }

updateFlags:
    if (work->timer == 0 && fn_1_5230(&work->initialPosition, &cameraPosition) < 250.0 &&
        (1.52587890625e-05 * (f64)(f32)(frand() & 0xFFFF)) < profile->nearbyActionProbability) {
        work->buttonsHeld |= 0x200;
        work->padDown |= 0x200;
        work->timer = 60;
    }
    if ((f64)work->timer < 30.0) {
        work->requestedHeading += 10.0;
    }
}

void fn_1_E300(M699CharacterWorkPrefix *work)
{
    M699MovementProfile * profile;
    M699UpdateCallback nextUpdate;
    void * relatedObject0;
    void * relatedObject1;
    Vec cameraPosition;
    Vec playerPosition;
    Vec cameraRotation;
    Vec targetPosition;
    s32 relation;
    f32 heading;
    f32 distance;
    profile = &lbl_1_data_5F4[work->controllerType];
    relation = fn_1_F568(work);
    nextUpdate = fn_1_B8E8(work);
    if (nextUpdate != NULL)
    {
        fn_1_B518(work, nextUpdate);
        return;
    }
    if (relation != 3)
    {
        fn_1_B518(work, fn_1_B944);
        return;
    }
    if (++work->callbackStateTimer > 0x78)
    {
        fn_1_B518(work, fn_1_B944);
        return;
    }
    fn_1_14ACC(& cameraPosition, & cameraRotation);
    distance = fn_1_5230(& work->initialPosition, & cameraPosition);
    if (cameraPosition.y > 200.0)
    {
        if (distance < 400.0)
        {
            work->buttonsHeld |= 0x100;
            work->padDown |= 0x100;
            fn_1_B558(work, & cameraPosition,
                      fn_1_5098(& work->initialPosition, & cameraPosition), 0.5f);
            return;
        }
        goto update;
    }
    if (work->timer == 0 && distance < 250.0 && (1.52587890625e-05 * (f64)(f32)(frand() & 0xFFFF)) < profile->nearbyActionProbability)
    {
        work->buttonsHeld = work->buttonsHeld | 0x200;
        work->padDown |= 0x200;
        work->timer = 60;
    }
    update : playerPosition = * fn_1_14778(work->groupId);
    heading = fn_1_5098(& work->initialPosition, & cameraPosition);
    fn_1_539C(& playerPosition, & cameraPosition, 0.4f, & targetPosition);
    fn_1_B558(work, & targetPosition, heading, 1.0f);
}

void fn_1_EB74(M699CharacterWorkPrefix *work)
{
    M699MovementProfile *dataRecord;
    void (*callback)(M699CharacterWorkPrefix *work);
    s32 relation;
    s32 targetIndex;

    dataRecord = &lbl_1_data_5F4[work->controllerType];
    relation = fn_1_F568(work);
    if ((callback = fn_1_B8E8(work)) != NULL) {
        fn_1_B518(work, callback);
        return;
    }

    if (relation == -1) {
        work->callbackStateTimer = 0;
    }
    if (++work->callbackStateTimer > 0x2A) {
        fn_1_B518(work, fn_1_B944);
        return;
    }

    targetIndex = work->groupNumber;
    fn_1_B558(work,
              &lbl_1_data_204[targetIndex],
              lbl_1_data_24C[targetIndex], 1.0f);
}

void fn_1_EFDC(M699CharacterWorkPrefix *work)
{
    M699MovementProfile *dataRecord;
    M699CharacterWorkPrefix *motionData;
    M699CharacterWorkPrefix *firstObstacle;
    M699CharacterWorkPrefix *secondObstacle;
    s32 repositioned;
    f32 heading;
    f32 distance;
    f32 headingDifference;
    f32 secondHeading;
    f32 adjustedHeading;
    f32 firstHeading;

    dataRecord = &lbl_1_data_5F4[work->controllerType];
    motionData = work->relatedObjects[0];
    repositioned = 0;
    heading = fn_1_5134(&motionData->initialPosition, &work->destinationPosition);
    distance = fn_1_5230(&motionData->initialPosition, &work->destinationPosition);
    if (heading < 650.0) {
        repositioned = 1;
        distance = 700.0f;
        fn_1_F2D0(&motionData->initialPosition, &heading, &distance, 1);
    }
    firstObstacle = fn_1_F968(work->groupId ^ 1, NULL, &motionData->initialPosition, heading,
                              60.0f, (f32)(200.0 + (f64)distance), 1);
    if (firstObstacle != NULL) {
        repositioned = 1;
        firstHeading = fn_1_5134(&motionData->initialPosition, &firstObstacle->initialPosition);
        headingDifference = fn_1_4FE4(firstHeading, heading);
        if (headingDifference > 0.0) {
            adjustedHeading = 30.0f;
        } else {
            adjustedHeading = -30.0f;
        }
        heading += adjustedHeading;
        secondObstacle = fn_1_F968(work->groupId ^ 1, firstObstacle, &motionData->initialPosition,
                                   heading, 60.0f, (f32)(200.0 + (f64)distance), 1);
        if (secondObstacle != NULL) {
            secondHeading = fn_1_5134(&motionData->initialPosition, &secondObstacle->initialPosition);
            headingDifference = fn_1_4FE4(firstHeading, secondHeading);
            if (headingDifference > 0.0) {
                heading = (f32)(30.0 + (f64)secondHeading);
            } else {
                heading = (f32)((f64)secondHeading - 30.0);
            }
        }
        fn_1_F2D0(&motionData->initialPosition, &heading, &distance, 0);
    }
    if (repositioned != 0) {
        work->destinationPosition.x = (f32)((f64)motionData->initialPosition.x +
            ((f64)distance * sin((M_PI * (f64)heading) / 180.0)));
        work->destinationPosition.z = (f32)((f64)motionData->initialPosition.z +
            ((f64)distance * cos((M_PI * (f64)heading) / 180.0)));
    }
}

s32 fn_1_F2D0(Vec *position, f32 *heading, f32 *distance, s32 reflect)
{
    Vec candidatePosition;
    f32 deltaX;
    f32 deltaZ;
    s32 changed;

    changed = 0;
    deltaX = (f32)((f64)*distance *
        sin((M_PI * (f64)*heading) / 180.0));
    deltaZ = (f32)((f64)*distance *
        cos((M_PI * (f64)*heading) / 180.0));
    if (reflect != 0) {
        if (fabs((f64)(position->x + deltaX)) > 800.0) {
            changed = 1;
            deltaX = -deltaX;
        }
        if (fabs((f64)(position->z + deltaZ)) > 550.0) {
            changed = 1;
            deltaZ = -deltaZ;
        }
    }

    candidatePosition.x = position->x + deltaX;
    candidatePosition.y = position->z;
    candidatePosition.z = position->z + deltaZ;
    if (candidatePosition.x < (-800.0)) {
        changed = 1;
        candidatePosition.x = -800.0f;
    }
    if (candidatePosition.x > 800.0) {
        changed = 1;
        candidatePosition.x = 800.0f;
    }
    if (candidatePosition.z < (-550.0)) {
        changed = 1;
        candidatePosition.z = -550.0f;
    }
    if (candidatePosition.z > 550.0) {
        changed = 1;
        candidatePosition.z = 550.0f;
    }
    if (changed != 0) {
        *heading = fn_1_5134(position, &candidatePosition);
        *distance = fn_1_5230(position, &candidatePosition);
    }
    return changed;
}

s32 fn_1_F568(M699CharacterWorkPrefix *work)
{
    void *object;
    s32 mode;
    s32 index;

    object = fn_1_14ACC(NULL, NULL);
    mode = fn_1_10C0();
    if (mode == 6) {
        return -1;
    }
    if (object == NULL) {
        return 0;
    }
    if (object == work) {
        return 1;
    }
    if (object == work->relatedObjects[0]) {
        return 2;
    }
    if (object == work->relatedObjects[1]) {
        return 2;
    }
    for (index = 0; index < 6; index++) {
        if (lbl_1_bss_42C[index] == object) {
            break;
        }
    }
    if (index >= 6) {
        return -1;
    }
    return 3;
}

s32 fn_1_F660(M699CharacterWorkPrefix *work, void *target, f32 distanceLimit)
{

    if (fn_1_F968(work->groupId ^ 1, NULL, &work->initialPosition,
                  fn_1_5098(&work->initialPosition, target), distanceLimit, (fn_1_5230(&work->initialPosition, target) + 300.0), 1) != NULL) {
        return 0;
    }
    return 1;
}

void fn_1_F70C(M699CharacterWorkPrefix *work)
{
    work->padDown = 0;
    work->buttonsHeld = 0;
    work->outputHeading = 0.0f;
    work->outputSpeed = 0.0f;
}

void fn_1_F740(M699CharacterWorkPrefix *work, s32 modelId)
{
    f32 motionDuration;
    f32 scale;
    M699PlayerMotionData *targetPosition;
    f32 previousRotation;
    f32 wrappedRotation;
    f32 storedRotation;

    targetPosition = &work->motionData;
    previousRotation = work->rotation.y;
    wrappedRotation = (f32)fmod((f64)previousRotation, 360.0);
    storedRotation = wrappedRotation;
    work->rotation.y = storedRotation;
    if (work->rotation.y < 0.0) {
        work->rotation.y += 360.0;
    }
    fn_80026790((s16)modelId, &work->initialPosition);
    fn_80026838((s16)modelId, &work->rotation);
    scale = (f32)(1.0 + (1.0 - (f64)work->motionAdjustment) * (f64)work->blendRate);
    fn_800268B0((s16)modelId, scale, work->motionAdjustment, scale);
    if (work->characterEffectPlayed > 0) {
        if ((--work->characterEffectPlayed & 1) != 0) {
            fn_800269E8((s16)modelId, 1U);
        } else {
            fn_80026A44((s16)modelId, 1U);
        }
    }
    motionDuration = fn_8002D530((s16)modelId);
    if (motionDuration <= 0.0) {
        motionDuration = 1.0f;
    }
    work->motionProgress = fn_8002D4E0((s16)modelId) / motionDuration;
    work->motionAngle = fn_1_4EC8(work->motionAngle, (f32)(360.0 * (f64)work->motionProgress), 0.3f);
    work->motionTime++;
    fn_1_75C(work->initialPosition.x, (f32)(0.5 * (f64)targetPosition->height), work->initialPosition.z, targetPosition->proximityRadius);
}

void *fn_1_F968(u32 searchKey, void *excludedWork, Vec *targetPosition,
                f32 heading, f32 angleLimit, f32 distanceLimit,
                s32 returnNearest)
{
    M699CharacterWorkPrefix *nearestWork;
    M699CharacterWorkPrefix *farthestWork;
    M699CharacterWorkPrefix *candidate;
    f32 nearestDistance;
    f32 farthestDistance;
    f32 deltaX;
    f32 deltaZ;
    f32 angle;
    f32 wrappedAngle;
    s32 index;

    nearestWork = NULL;
    farthestWork = NULL;
    nearestDistance = 100000.0f;
    farthestDistance = 0.0f;
    for (index = 0; index < 6; index++) {
        candidate = lbl_1_bss_42C[index];
        if (candidate != NULL && candidate != excludedWork &&
            candidate->groupId == searchKey) {
            deltaX = candidate->initialPosition.x - targetPosition->x;
            deltaZ = candidate->initialPosition.z - targetPosition->z;
            if (angleLimit < 180.0) {
                angle = (f32)(180.0 *
                    (atan2((f64)deltaX, (f64)deltaZ) / M_PI));
                wrappedAngle = (f32)fmod((f64)(angle - heading), 360.0);
                angle = wrappedAngle;
                if (angle < 0.0f) {
                    angle += 360.0f;
                }
                if (angle > angleLimit && angle < 360.0f - angleLimit) {
                    continue;
                }
            }
            angle = sqrtf((deltaX * deltaX) + (deltaZ * deltaZ));
            if (angle > distanceLimit) {
                continue;
            }
            if (angle < nearestDistance) {
                nearestDistance = angle;
                nearestWork = candidate;
            }
            if (angle > farthestDistance) {
                farthestDistance = angle;
                farthestWork = candidate;
            }
        }
    }
    if (returnNearest != 0) {
        return nearestWork;
    }
    return farthestWork;
}

M699MovementProfile lbl_1_data_5F4[6] = {
    { 0.1f, 0.2f, 0.1f, 0.2f, 0.2f, 0.2f },
    { 0.3f, 0.4f, 0.4f, 0.4f, 0.3f, 0.4f },
    { 0.6f, 0.6f, 0.6f, 0.6f, 0.4f, 0.6f },
    { 0.95f, 0.8f, 0.8f, 0.8f, 0.5f, 0.8f },
    { 0.7f, 0.5f, 0.6f, 0.75f, 0.1f, 0.5f },
    { 0.7f, 0.5f, 0.6f, 0.75f, 0.1f, 0.5f }
};

OMOBJ *lbl_1_bss_4B8;
OMOBJ *lbl_1_bss_4A8[4];
s32 lbl_1_bss_444[25];
M699CharacterWorkPrefix *lbl_1_bss_42C[6];
s32 lbl_1_bss_428;

s32 fn_1_FCBC(u32 key)
{
    OMOBJ *object;
    M699CharacterWorkPrefix *work;
    M699CharacterWorkPrefix *currentWork;
    s32 index;

    for (index = 0; index < 4; index++) {
        object = lbl_1_bss_4A8[index];
        work = object->data;
        currentWork = work;
        if (currentWork->groupId == key && currentWork->characterNumber == 0) {
            return 1;
        }
    }
    return 0;
}
