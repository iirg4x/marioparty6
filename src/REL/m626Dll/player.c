/* Updates player models, input, motion, and proximity interactions. */
#include "dolphin/math.h"
#include "REL/m626Dll/player.h"
#include "REL/m626Dll/collision.h"
#include "game/charman.h"
#include "game/gamework.h"
#include "game/board/player.h"
#include "game/pad.h"
#include "game/memory.h"
#include "game/omovl.h"
#include "game/mg/seqman.h"
#include "game/hu3d.h"
#include "datadir_enum.h"
#include "humath.h"
#include <string.h>

extern s32 frand(void);

#define MSM_SE_SNOW_BRAWL_EFFECT_1803 1803
#define MSM_SE_SNOW_BRAWL_EFFECT_1805 1805
#define MSM_SE_SNOW_BRAWL_EFFECT_1804 1804
#define MSM_SE_SNOW_BRAWL_EFFECT_1806 1806
#define M626_COLLISION_MASK_TEAM_0 55
#define M626_COLLISION_MASK_TEAM_1 59
#define M626_COLLISION_MASK_UPPER_BITS 0xE0U

typedef struct M626CharacterMotionParam {
    u16 motionIndex; /* Main character motion in the object's motion table. */
    s16 overlayIndex; /* Overlay motion index, or -1 when there is no overlay. */
    f32 blendFraction; /* Fraction of a second used to blend into the motion. */
    u32 motionAttributes; /* Playback flags passed to the character motion system. */
} M626CharacterMotionParam;
typedef struct M626DllCharacterDimensions {
    f32 horizontalScale; /* Effect model scale in X and Z. */
    f32 verticalScale; /* Effect model scale in Y. */
} M626DllCharacterDimensions;
typedef struct M626CollisionParameters {
    Vec start; /* Start point of the character collision segment, in model space. */
    Vec end; /* End point of the character collision segment, in model space. */
} M626CollisionParameters;

s32 fn_1_824(s16, s16);
s32 fn_1_8A4(s32);
s32 fn_1_86C(void);
s32 fn_1_830(s32);
s32 fn_1_3C94(s32);
s32 fn_1_3D9C(s32, s32);
f32 fn_1_407C(Vec *, Vec *);
f32 fn_1_4118(Vec *, Vec *);
f32 fn_1_3EAC(f32, f32, f32);
s32 fn_1_29BC(Vec *);
s32 fn_1_2BDC(Vec *);
void fn_1_3AF4(s32, s32, Vec *, s32, s32);
s32 fn_1_3340(s32, Vec *);
void fn_1_E114(const Vec *, f32);
void fn_1_DD48(s32, f32, const Vec *);
void fn_1_B55C(s32, const Vec *, const Vec *, void *);
s32 fn_1_B62C(u32, const Vec *, f32, f32, f32);
void *fn_1_B998(s32);

void fn_1_44D4(void);
void fn_1_44F8(OMOBJ *object);
void fn_1_4548(OMOBJ *object);
void fn_1_45F8(M626ObjectWork *work);
void fn_1_48C4(M626ObjectWork *work);
void fn_1_4B90(M626ObjectWork *work);
void fn_1_4CC8(M626ObjectWork *state);
void fn_1_4CE0(M626ObjectWork *work);
void fn_1_4CE4(M626ObjectWork *work);
void fn_1_4EA8(OMOBJ *object);
void fn_1_53F4(OMOBJ *object);
s32 fn_1_552C(M626ObjectWork *work);
s32 fn_1_57A8(M626ObjectWork *work);
s32 fn_1_5C20(M626ObjectWork *work);
s32 fn_1_5E88(M626ObjectWork *state);
void fn_1_5FA4(OMOBJ *object);
void fn_1_6040(OMOBJ *object, s32 effect);
void fn_1_60C4(OMOBJ *object);
void fn_1_6120(M626ObjectWork *state);
void fn_1_613C(M626ObjectWork *state, s32 mode, f32 value);
void fn_1_6148(OMOBJ *object);
void fn_1_61CC(OMOBJ *object, s32 index);
s32 fn_1_63A8(OMOBJ *object);
s32 fn_1_6414(OMOBJ *object);
void fn_1_6474(M626ObjectWork *work);
s32 fn_1_6728(M626ObjectWork *work, Vec *target, f32 targetAngle,
              f32 speed);
void fn_1_6D28(M626ObjectWork *work);
s32 fn_1_7050(M626Entry *trigger, M626Entry *contact);
void fn_1_71AC(M626ObjectWork *controller);
void fn_1_723C(M626ObjectWork *controller,
               M626ControlUpdate callback);
s32 fn_1_7264(M626ObjectWork *model, Vec *target, f32 angle, f32 speed);
void fn_1_7648(M626ObjectWork *controller);
void fn_1_7708(M626ObjectWork *controller);
void fn_1_7808(M626ObjectWork *controller);
void fn_1_78DC(M626ObjectWork *controller);
s32 fn_1_797C(M626ObjectWork *work);
s32 fn_1_7C44(M626ObjectWork *work);
s32 fn_1_7F30(M626ObjectWork *work, f32 baseY);
void fn_1_8570(M626ObjectWork *state);
void fn_1_85A4(M626ObjectWork *state, s32 model);
void *fn_1_879C(u32 mode, void *work, Vec *position, f32 range, f32 angle,
                f32 scale, u32 flags);
s32 fn_1_8ADC(u32 key);
u32 fn_1_8B58(s32 index);
s32 fn_1_8B90(s32 playerIndex);
s32 fn_1_8BD0(u32 key);

u32 lbl_1_data_148[10] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 1),
    DATANUM(DATA_mariomot, 2), DATANUM(DATA_mariomot, 94),
    DATANUM(DATA_mario, 142), DATANUM(DATA_mario, 143),
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 24),
    DATANUM(DATA_mariomot, 6), DATANUM(DATA_mariomot, 40)
};

M626CharacterMotionParam lbl_1_data_170[12] = {
{0, -1, 0.200000003f, HU3D_MOTATTR_LOOP},
{1, -1, 0.200000003f, HU3D_MOTATTR_LOOP},
{2, -1, 0.200000003f, HU3D_MOTATTR_LOOP},
{3, -1, 0.100000001f, HU3D_MOTATTR_NONE},
{4, -1, 0.200000003f, HU3D_MOTATTR_NONE},
{6, -1, 0.200000003f, HU3D_MOTATTR_NONE},
{7, -1, 0.200000003f, HU3D_MOTATTR_NONE},
{8, -1, 0.200000003f, HU3D_MOTATTR_NONE},
{9, -1, 0.200000003f, HU3D_MOTATTR_NONE},
{0, 5, 0.200000003f, HU3D_MOTATTR_LOOP},
{1, 5, 0.200000003f, HU3D_MOTATTR_LOOP},
{2, 5, 0.200000003f, HU3D_MOTATTR_LOOP}
};

Vec lbl_1_data_200[12] = {
{-300.000000f, 0.00000000f, 0.00000000f},
{400.000000f, 0.00000000f, 0.00000000f},
{300.000000f, 0.00000000f, 300.000000f},
{300.000000f, 0.00000000f, -300.000000f},
{-450.000000f, 0.00000000f, 250.000000f},
{-400.000000f, 0.00000000f, -150.000000f},
{-500.000000f, 0.00000000f, 450.000000f},
{-500.000000f, 0.00000000f, -350.000000f},
{0.00000000f, 0.00000000f, 0.00000000f},
{0.00000000f, 0.00000000f, 0.00000000f},
{0.00000000f, 0.00000000f, 0.00000000f},
{0.00000000f, 0.00000000f, 0.00000000f}
};

f32 lbl_1_data_290[15] = {
    60.0000000f, 280.000000f, 260.000000f, 280.000000f,
    90.0000000f, 90.0000000f, 90.0000000f, 90.0000000f,
    90.0000000f, 90.0000000f, 90.0000000f, 90.0000000f,
    0.00000000f, 0.00000000f, 0.00000000f
};

Vec lbl_1_data_2CC[2] = {
{-200.000000f, 0.00000000f, 0.00000000f},
{200.000000f, 0.00000000f, 0.00000000f}
};

M626DllCharacterDimensions lbl_1_data_2E4[16] = {
{1.10000002f, 1.00000000f},
{1.10000002f, 1.10000002f},
{1.20000005f, 1.20000005f},
{1.29999995f, 1.00000000f},
{1.29999995f, 1.00000000f},
{1.20000005f, 1.20000005f},
{1.10000002f, 1.39999998f},
{1.00000000f, 1.00000000f},
{1.20000005f, 1.00000000f},
{1.39999998f, 0.800000012f},
{1.00000000f, 1.00000000f},
{1.39999998f, 0.800000012f},
{1.39999998f, 0.800000012f},
{1.39999998f, 0.800000012f},
{1.00000000f, 1.00000000f},
{1.00000000f, 1.00000000f}
};

M626PlayerConfig lbl_1_data_364 = {
50.0000000f, 200.000000f, 1.00000000f, 1.00000000f, 11.6666670f, 36.6666679f, 5.83333349f,
{{0, 1, 2.00000000f, 2, 1.00000000f}, {9, 10, 2.00000000f, 11, 1.00000000f}},
3, 4, 5, 6, 7, 8
};

M626CollisionParameters lbl_1_data_3C0 = { { 0.00000000f, 0.00000000f, 0.00000000f },
                                           { 0.00000000f, 140.000000f, 0.00000000f } };

OMOBJ *lbl_1_bss_304;
OMOBJ *lbl_1_bss_2F4[4];
M626ObjectWork *lbl_1_bss_2D4[8];
s32 lbl_1_bss_2D0;

/* Removes the character models and resets character animation state during cleanup. */
void fn_1_44D4(void)
{
    CharModelKill(-1);
}

/* Clears the eight-player effect pool when the Snow Brawl object is initialized. */
void fn_1_44F8(OMOBJ *object) {
    s32 slot;

    slot = 0;
    while (slot < 8) {
        lbl_1_bss_2D4[slot] = NULL;
        slot += 1;
    }
    lbl_1_bss_2D0 = 0;
}

/* Advances each registered player effect once per object-manager update. */
void fn_1_4548(OMOBJ *object) {
    s32 slot;
    M626ObjectWork *work;

    if ((s32) lbl_1_bss_2D0 == 0) {
        lbl_1_bss_2D0 = 1;
    }
    slot = 0;
    while (slot < 8) {
        work = lbl_1_bss_2D4[slot];
        if (work != NULL) {
            fn_1_45F8(work);
            fn_1_48C4(work);
            fn_1_4B90(work);
            fn_1_4CE0(work);
            work->updateCounter = work->updateCounter + 1;
        }
        slot += 1;
    }
}

/* Advances the effect target after each accumulated travel interval and saves its current
 * position. */
void fn_1_45F8(M626ObjectWork *work)
{
    M626ObjectWork *modeWork;
    f32 angle = work->rotation.y;
    f64 angleDelta;
    if (work->flags != 0) {
        modeWork = work;
        if (modeWork->identity.character == 8) {
            goto updatePosition;
        }
    }
    if (work->velocity.y > 0.0000000000000000) {
        goto updatePosition;
    }
    if (work->position.y > 0.0000000000000000) {
        goto updatePosition;
    }

    work->effectProgress += fn_1_4118(&work->position,
                               &work->savedPosition);
    work->effectProgress = (f32)((f64)work->effectProgress +
        (f64)PSVECMag(&work->velocity) * 0.050000000000000003);
    if (work->effectProgress < 45.000000000000000) {
        goto updatePosition;
    }
    work->effectProgress = (f32)((f64)work->effectProgress - 45.000000000000000);
    if ((++work->jitterIndex & 1) == 0) {
        angleDelta = 90.000000000000000;
    } else {
        angleDelta = -90.000000000000000;
    }
    angle = (f32)((f64)angle + angleDelta);
    {
        Vec target;
        target.x = (f32)((f64)work->position.x +
            15.000000000000000 * sin(3.1415926535897931 * angle / 180.00000000000000) +
            8.0000000000000000 *
                (0.000030517578125000000 * (f32)((frand() & 0xFFFF) - 32768)));
        target.y = 0.00000000f;
        target.z = (f32)((f64)work->position.z +
            15.000000000000000 * cos(3.1415926535897931 * angle / 180.00000000000000) +
            8.0000000000000000 *
                (0.000030517578125000000 * (f32)((frand() & 0xFFFF) - 32768)));
        fn_1_E114(&target, 10.0000000f);
    }

updatePosition:
    *&work->savedPosition = work->position;
}

/* Raises or hides the player's snow effect and triggers its impact feedback during the update. */
void fn_1_48C4(M626ObjectWork *work)
{
    if (work->status != 0) {
        f32 step = (f32)((f64)10.000000000000000 * work->effectRise);
        if ((work->tiltAngle += step) > 90.000000000000000) {
            Vec world;
            Vec local;
            M626ObjectWork *playerWork;

            work->tiltAngle = 90.0000000f;
            if (work->effectComplete == 0) {
                local.x = local.z = 0.00000000f;
                local.y = (f32)(0.69999999999999996 * work->config.modelHeight);
                PSMTXMultVec(work->matrix, &local, &world);
                world.y += 30.0;
                PSVECAdd(&world, &work->position, &world);
                fn_1_DD48(50, 120.000000f, &world);
                work->collisionMode = 0.00000000f;
                work->effectComplete = 1;
                if (work->flags != 0) {
                    playerWork = work;
                    omVibrate(playerWork->actor.playerIndex, 20, 20, 0);
                }
            }
        }
        if ((work->effectRise += 0.050000000000000003) > 1.0000000000000000) {
            work->effectRise = 1.00000000f;
        }
        if (work->eventSent == 0 && work->effectRise > 0.80000000000000004) {
            fn_1_3340(MSM_SE_SNOW_BRAWL_EFFECT_1805, &work->position);
            work->eventSent = 1;
        }
    } else {
        if ((work->effectRise -= 0.050000000000000003) <= 0.0000000000000000) {
            work->effectRise = 0.00000000f;
        }
    }

    if (work->effectRise > 0.0000000000000000) {
        f32 xz = work->effectRise * work->config.heightFactor;
        f32 y = work->effectRise * work->config.depthFactor;

        Hu3DModelAttrReset(work->effectModel, HU3D_ATTR_DISPOFF);
        Hu3DModelRotSet(work->effectModel, work->tiltAngle, work->effectRotation,
                        0.00000000f);
        Hu3DModelPosSetV(work->effectModel, &work->position);
        Hu3DModelScaleSet(work->effectModel, xz, y, xz);
    } else {
        Hu3DModelAttrSet(work->effectModel, HU3D_ATTR_DISPOFF);
    }
}

/* Animates the attached model's pulse while its motion state is active. */
void fn_1_4B90(M626ObjectWork *work)
{
    if (work->motionState != 0) {
        if ((work->pulseProgress += 0.2) > 1.0000000000000000) {
            work->pulseProgress = 1.00000000f;
        }
    } else {
        if ((work->pulseProgress = (f32) ((f64) work->pulseProgress - 0.29999999999999999)) <=
            0.0000000000000000) {
            work->pulseProgress = 0.00000000f;
        }
    }
    if (work->pulseProgress > 0.0000000000000000) {
        f32 scale = (f32)(1.5000000000000000 * (f64)work->pulseProgress);
        Hu3DModelAttrReset((s16)work->modelId, HU3D_ATTR_DISPOFF);
        Hu3DModelScaleSet((s16)work->modelId, scale, scale, scale);
    } else {
        Hu3DModelAttrSet((s16)work->modelId, HU3D_ATTR_DISPOFF);
    }
}

/* Clears the motion-state latch when the effect sequence resets it. */
void fn_1_4CC8(M626ObjectWork *state) {
    if (state->motionState != 0) {
        state->motionState = 0;
    }
}

void fn_1_4CE0(M626ObjectWork *work)
{
}

/* Reserves a player-effect slot and initializes its spawn transform and model. */
void fn_1_4CE4(M626ObjectWork *work)
{
    void *config;
    s32 spawnIndex;
    s32 poolSlot;

    config = &work->config;
    for (poolSlot = 0; poolSlot < 8; poolSlot++) {
        if (lbl_1_bss_2D4[poolSlot] == NULL) {
            lbl_1_bss_2D4[poolSlot] = work;
            spawnIndex = work->variation;
            work->position = lbl_1_data_200[spawnIndex];
            work->savedPosition = work->position;
            work->velocity.x = work->velocity.y = work->velocity.z = 0.0f;
            work->rotation.y = lbl_1_data_290[spawnIndex];
            work->worldStart = lbl_1_data_2CC[work->key];
            work->worldEnd = work->worldStart;
            work->acceleration = 1.0f;
            work->speedScale = 1.0f;
            work->settled = 0;
            work->effectModel = (s16)fn_1_3C94(fn_1_830(2));
            Hu3DModelAttrSet(work->effectModel, HU3D_MOTATTR_LOOP);
            Hu3DModelAttrSet(work->effectModel, HU3D_MOTATTR_PAUSE);
            Hu3DModelLayerSet(work->effectModel, 6);
            work->status = 0;
            work->effectRise = 0.0f;
            return;
        }
    }
}

/* Loads the character, attaches its effect model, and registers collision and update callbacks. */
void fn_1_4EA8(OMOBJ *object)
{
    M626ObjectWork *allocatedWork;
    M626ObjectWork *work;
    M626PlayerConfig *attachedWork;
    M626ObjectWork *objectWork;
    M626DllCharacterDimensions *characterDimensions;
    s32 index;
    s32 playerIndex;
    s32 character;
    s32 variation;
    u32 objectType;
    char *hookName;
    s32 motionId;
    s32 modelId;

    allocatedWork = HuMemDirectMallocNum(HEAP_HEAP, 572, HU_MEMNUM_OVL);
    object->data = allocatedWork;
    memset(allocatedWork, 0, 572);
    object->stat |= 0x100;

    work = allocatedWork;
    allocatedWork->actor.playerIndex = playerIndex = object->work[0];
    character = GwPlayerConf[playerIndex].charNo;
    allocatedWork->identity.character = character;
    work->key = objectType = object->work[1];
    work->variation = variation = object->work[2];
    if (objectType == 0) {
        work->groupIndex = 0;
    } else {
        work->groupIndex = variation - 1;
    }
    work->flags = 1;
    work->playerType = GwPlayerConf[playerIndex].type;
    work->computerDifficulty = GwPlayerConf[playerIndex].comDif & 3;
    work->controlPhase = 0;

    modelId = CharModelCreate((s16)character, 4);
    *object->mdlId = (s16)modelId;
    Hu3DModelPosSet((s16)modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet((s16)modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelLayerSet((s16)modelId, 5);
    Hu3DModelShadowSet((s16)modelId);
    CharEffectLayerSet(5);
    work->modelId = fn_1_3C94(fn_1_830(9));
    Hu3DModelLayerSet((s16)work->modelId, 5);
    hookName = CharModelItemHookGet((s16)character, 4, 0);
    Hu3DModelHookSet((s16)modelId, hookName, (s16)work->modelId);
    work->motionState = 0;

    for (index = 0; index < 10; index++) {
        motionId = CharMotionCreate((s16)character, lbl_1_data_148[index]);
        object->mtnId[index] = motionId;
    }
    CharMotionDataClose((s16)character);
    work->motionIndex = -1;
    fn_1_61CC(object, 0);
    CharModelStepSet((s16)character, 2);

    attachedWork = &work->config;
    *attachedWork = lbl_1_data_364;
    characterDimensions = &lbl_1_data_2E4[character];
    attachedWork->modelHeight = CharModelHeightGet((s16)character);
    attachedWork->heightFactor = characterDimensions->horizontalScale;
    attachedWork->depthFactor = characterDimensions->verticalScale;

    objectWork = work;
    objectWork->collisionFlags = 0;
    objectWork->collisionMask = work->key == 0 ? M626_COLLISION_MASK_TEAM_0 :
                                                   M626_COLLISION_MASK_TEAM_1;
    objectWork->collisionType = 0;
    objectWork->playerIndex = playerIndex;
    objectWork->collisionState = 0;
    objectWork->owner = object;
    objectWork->update = fn_1_60C4;
    objectWork->draw = fn_1_7050;
    objectWork->positionData = &work->position;
    objectWork->offset = &work->velocity;
    objectWork->matrix = work->rotationMatrix;
    objectWork->segmentStart = lbl_1_data_3C0.start;
    objectWork->segmentEnd = lbl_1_data_3C0.end;
    objectWork->segmentEnd.y = (f32)(0.8 * (f64)attachedWork->modelHeight);
    objectWork->radius = 40.0f;
    objectWork->collisionMode = 1.0f;
    fn_1_10598(objectWork);

    fn_1_4CE4(work);
    allocatedWork->updatePhase = 0;
    allocatedWork->detail.player.volumeEnabled = 1;
    object->objFunc = fn_1_53F4;
}

/* Runs the Snow Brawl player update, advances its active phase, and applies movement each frame. */
void fn_1_53F4(OMOBJ *object)
{
    M626ObjectWork *state;
    M626ObjectWork *work = object->data;

    state = work;
    fn_1_8570(state);
    fn_1_6120(state);
    switch (work->updatePhase) {
    case 0:
        if (fn_1_552C(state)) { work->updatePhase++; state->statePhase = 0; }
        break;
    case 1:
        if (fn_1_57A8(state)) { work->updatePhase++; state->statePhase = 0; }
        break;
    case 2:
        if (fn_1_5C20(state)) { work->updatePhase++; state->statePhase = 0; }
        break;
    case 3:
        if (fn_1_5E88(state)) { work->updatePhase++; state->statePhase = 0; }
        break;
    }
    fn_1_6148(object);
    PSVECAdd(&state->position, &state->velocity, &state->position);
    fn_1_5FA4(object);
}

/* Moves the player into the arena and waits for the round to begin; called by the update
 * callback. */
s32 fn_1_552C(M626ObjectWork *work)
{
    s32 done = 0;

    switch (work->statePhase) {
    case 0:
        if (work->key == 0) {
            work->statePhase++;
            work->stateTimer = 0;
        } else {
            work->statePhase = 2;
            work->stateTimer = work->groupIndex * 12 + 36;
        }
        /* fall through */
    case 1:
        if (++work->stateTimer >= 108) {
            work->statePhase = 5;
            work->stateTimer = 0;
            fn_1_613C(work, 6, 1.00000000f);
            work->motionActive = 1;
            work->targetPosition = work->position;
            work->targetPosition.x =
                (f32)((f64)work->targetPosition.x - 300.0);
            work->targetAngle = 90.0f;
        }
        break;
    case 2:
        if (--work->stateTimer <= 0) {
            work->statePhase++;
            work->stateTimer = 0;
            work->modelAttributes |= 0x200;
            /* fall through */
        } else {
            break;
        }
    case 3:
        if (++work->stateTimer >= 48) {
            work->statePhase++;
            work->stateTimer = 0;
            work->modelAttributes |= 0x100;
            /* fall through */
        } else {
            break;
        }
    case 4:
        if (++work->stateTimer >= 108) {
            work->statePhase++;
            work->stateTimer = 0;
            work->targetPosition = work->position;
            work->targetPosition.x += 200.0;
            work->targetAngle = 270.0f;
            /* fall through */
        } else {
            break;
        }
    case 5:
        if (work->motionActive == 0 &&
            fn_1_6728(work, &work->targetPosition, work->targetAngle,
                      1.00000000f) != 0) {
            work->statePhase++;
            work->stateTimer = 0;
            /* fall through */
        } else {
            break;
        }
    case 6:
        if (MgSeqModeGet() != 5) {
            break;
        }
        work->statePhase++;
        work->stateTimer = 0;
        work->collisionFlags |= 1;
        /* fall through */
    default:
        done = 1;
        break;
    }
    fn_1_6474(work);
    fn_1_6D28(work);
    return done;
}

/* Reads human or computer movement input during play and slows the player for round transitions. */
s32 fn_1_57A8(M626ObjectWork *work)
{
    M626PlayerConfig *motionWork;
    s32 done;
    M626ObjectWork *playerWork;
    f32 stickX;
    f32 stickY;
    f32 combinedAngle;
    f32 wrappedAngle;
    f32 finalAngle;
    s32 pad;

    motionWork = &work->config;
    done = 0;

    switch (work->statePhase) {
    case 0:
        work->statePhase++;
        work->stateTimer = 0;
        /* fall through */
    case 1:
        if (MgSeqModeGet() == 6) {
            work->statePhase++;
            work->stateTimer = 0;
            if (work->motionState != 0) {
                work->motionState = 0;
            }
        } else if (work->status == 0) {
            if (work->playerType != 0) {
                fn_1_71AC(work);
            } else if (work->flags != 0) {
                playerWork = work;
                pad = GwPlayerConf[playerWork->actor.playerIndex].padNo;
                stickX = (f32)(s8)HuPadStkX[pad];
                stickY = (f32)-(s8)HuPadStkY[pad];
                work->controlWord = HuPadBtn[pad];
                work->modelAttributes = HuPadBtnDown[pad];
                work->targetDistance = (f32)(180.0 *
                    (atan2((f64)stickX, (f64)stickY) / 3.141592653589793));
                work->movementSpeed = 0.017857144f *
                    sqrtf((stickX * stickX) + (stickY * stickY));
                if (work->movementSpeed > 1.0f) {
                    work->movementSpeed = 1.0f;
                }
                combinedAngle = work->targetDistance + CRotM[0].y;
                wrappedAngle = (f32)fmod((f64)combinedAngle, 360.0);
                finalAngle = wrappedAngle;
                work->targetDistance = finalAngle;
            }
            fn_1_6474(work);
        }
        break;
    case 2:
        if (MgSeqModeGet() == 5) {
            work->velocity.x *= 0.98;
            work->velocity.z *= 0.98;
            break;
        } else {
            work->statePhase++;
            work->stateTimer = 0;
            work->transitionTimer = 6000;
            /* fall through */
        }
    case 3:
        if (++work->stateTimer < 10) {
            break;
        }
        work->statePhase++;
        work->stateTimer = 0;
        fn_1_613C(work, motionWork->profiles[0].closeMotion, 1.00000000f);
    default:
        done = 1;
        break;
    }

    fn_1_6D28(work);
    return done;
}

/* Brakes and turns the player while the round settles, then waits for the result sequence. */
s32 fn_1_5C20(M626ObjectWork *work)
{
    M626PlayerConfig *selection;
    s32 done;
    M626ObjectWork *timerWork;

    selection = &work->config;
    done = 0;

    switch (work->statePhase) {
    case 0:
        work->statePhase++;
        work->stateTimer = 0;
        if (work->status == 0) {
            fn_1_613C(work, selection->profiles[0].closeMotion, 1.00000000f);
        }
        work->motionActive = 0;
        /* fall through */
    case 1:
        work->velocity.x *= 0.98;
        work->velocity.z *= 0.98;
        if (MgSeqModeGet() == 6) {
            break;
        }
        work->statePhase++;
        work->stateTimer = 0;
        work->velocity.x = work->velocity.z = 0.0f;
        /* fall through */
    case 2:
        if (work->status == 0) {
            work->rotation.y = fn_1_3EAC(work->rotation.y, 0.0f, 0.1f);
            if (fabs(fn_1_3FC8(work->rotation.y, 0.0f)) > 3.0) {
                fn_1_613C(work, selection->profiles[0].slowMotion, 1.00000000f);
            } else {
                fn_1_613C(work, selection->profiles[0].closeMotion, 1.00000000f);
            }
        }
        if (work->flags != 0) {
            timerWork = work;
            if (++work->stateTimer > 60) {
                fn_1_998(timerWork->actor.playerIndex);
            }
        }
        if (MgSeqModeGet() != 8) {
            break;
        }
        if (fn_1_9B8() == 0) {
            break;
        }
        work->statePhase++;
        work->stateTimer = 0;
        if (work->status == 0) {
            fn_1_613C(work, selection->profiles[0].closeMotion, 1.00000000f);
            }
        /* fall through */
    default:
        done = 1;
        break;
    }
    return done;
}

/* Selects the win or lose motion and completes the post-round animation phase. */
s32 fn_1_5E88(M626ObjectWork *state)
{
    M626PlayerConfig *config = &state->config;
    M626ObjectWork *work;
    s32 done = 0;
    s32 useWinMotion = 0;

    switch (state->statePhase) {
    case 0:
        if (state->status == 0) {
            if (fn_1_8A4(state->key) > 0) {
                useWinMotion = 1;
                fn_1_613C(state, config->winMotion, 1.00000000f);
            } else {
                fn_1_613C(state, config->loseMotion, 1.00000000f);
            }
            if (state->flags != 0) {
                work = state;
                work->detail.player.volumeEnabled = 0;
                CharModelVoiceVolSet((s16)work->identity.character, 127);
            }
        }
        state->statePhase++;
        state->stateTimer = 0;
        break;
    case 1:
        if (++state->stateTimer < 90) {
            break;
        }
        state->statePhase++;
        state->stateTimer = 0;
    default:
        done = 1;
        break;
    }
    return done;
}

/* Tracks character voice timing and pans voice playback to the player's screen position. */
void fn_1_5FA4(OMOBJ *object)
{
    M626ObjectWork *work = object->data;
    M626ObjectWork *owner = work;
    work->detail.player.voiceCount++;
    if (owner->motionActive) {
        if (fn_1_63A8(object)) {
            owner->motionActive = 0;
            owner->motionTimer = 0;
        }
    } else {
        owner->motionTimer = 0;
    }
    CharModelVoicePanSet((s16)work->identity.character, fn_1_29BC(&owner->position));
}

/* Plays a character sound effect at the player's current volume and stereo pan. */
void fn_1_6040(OMOBJ *object, s32 effect)
{
    M626ObjectWork *work = object->data;
    M626ObjectWork *owner = work;
    s32 handle = CharFXPlayVolPan((s16)work->identity.character, (s16)effect,
                                  (s16)fn_1_2BDC(&owner->position),
                                  (s16)fn_1_29BC(&owner->position));

    fn_1_3AF4(effect, handle, &owner->position, 1, 1);
}

/* Updates the player's model transform from its position and rotation each frame. */
void fn_1_60C4(OMOBJ *object)
{
    M626ObjectWork *work;
    M626ObjectWork *state;
    s32 model;
    work = object->data;
    state = work;
    model = object->mdlId[0];
    fn_1_85A4(state, model);
}

/* Clears the requested motion and restores normal playback speed before phase selection. */
void fn_1_6120(M626ObjectWork *state) {
    state->requestedMotion = -1;
    state->motionSpeed = 1.00000000f;
}

/* Queues a character motion and its playback speed for the next model update. */
void fn_1_613C(M626ObjectWork *state, s32 motion, f32 speed) {
    state->requestedMotion = motion;
    state->motionSpeed = speed;
}

/* Applies queued motion changes, using the knocked-down motion while the player is hit. */
void fn_1_6148(OMOBJ *object)
{
    M626ObjectWork *state = object->data;
    M626ObjectWork *work = state;

    if (work->status != 0) {
        fn_1_61CC(object, 5);
        return;
    }
    if (work->requestedMotion != -1) {
        fn_1_61CC(object, work->requestedMotion);
        Hu3DMotionSpeedSet(object->mdlId[0], work->motionSpeed);
    }
}

/* Starts a configured character motion and optional overlay when the motion selection changes. */
void fn_1_61CC(OMOBJ *object, s32 index)
{
    M626ObjectWork *state;
    M626ObjectWork *work = object->data;
    u32 overlayAttr;
    u32 attr;
    s32 model, motion, overlay;
    f32 blend;

    state = work;
    overlayAttr = 0;
    model = object->mdlId[0];
    if (state->motionIndex != index && index < 12) {
        if (state->motionIndex < 0) {
            blend = 0.00000000f;
        } else {
            blend = 60.0000000f * lbl_1_data_170[index].blendFraction;
        }
        state->motionIndex = index;
        state->motionFrame = 0;
        motion = object->mtnId[lbl_1_data_170[index].motionIndex];
        attr = lbl_1_data_170[index].motionAttributes;
        Hu3DModelAttrReset(model, HU3D_MOTATTR | HU3D_MOTATTR_LOOP |
                            HU3D_MOTATTR_REV | HU3D_MOTATTR_OVL_LOOP |
                            HU3D_MOTATTR_OVL_REV);
        overlay = lbl_1_data_170[index].overlayIndex;
        if (overlay >= 0) {
            CharMotionShiftSet(work->identity.character, motion, 0.00000000f, blend, attr);
            Hu3DMotionOverlaySet(model, object->mtnId[overlay]);
            if (attr & HU3D_MOTATTR_LOOP) {
                overlayAttr |= HU3D_MOTATTR_OVL_LOOP;
            }
            if (attr & HU3D_MOTATTR_REV) {
                overlayAttr |= HU3D_MOTATTR_OVL_REV;
            }
            Hu3DModelAttrSet(model, overlayAttr);
        } else {
            Hu3DMotionOverlayReset(model);
            CharMotionShiftSet(work->identity.character, motion, 0.00000000f, blend, attr);
        }
        CharMotionSpeedSet(work->identity.character, 1.00000000f);
    }
}

/* Reports whether the current character motion ended and no blend remains. */
s32 fn_1_63A8(OMOBJ *object)
{
    M626ObjectWork *work = object->data;
    if (CharMotionEndCheck(work->identity.character) != 0 &&
        CharMotionShiftIDGet(work->identity.character) < 0) {
        return 1;
    }
    return 0;
}

/* Reports whether the character has no pending motion blend. */
s32 fn_1_6414(OMOBJ *object)
{
    M626ObjectWork *state = object->data;
    s32 result = 0;

    if (CharMotionShiftIDGet((s16)state->identity.character) < 0) {
        result = 1;
    }
    return result;
}

/* Handles snowball launch and return timers, motion changes, and their sound and rumble cues. */
void fn_1_6474(M626ObjectWork *work)
{
    M626PlayerConfig *motion = &work->config;
    M626ObjectWork *vibrationWork;
    Vec targetPosition;
    Vec velocity;
    s32 targetB;
    s32 targetA;
    f32 distance;
    f32 angle;
    f32 sine;
    f32 cosine;

    if (work->status != 0) {
        return;
    }
    if (work->firstTimer != 0) {
        work->firstTimer--;
    }
    if (work->cooldown != 0) {
        work->cooldown--;
    }
    if (work->motionState == 0) {
        if (work->firstTimer == 0 && work->transitionTimer == 0 &&
            (work->modelAttributes & 0x200) != 0) {
            targetB = motion->effectTargetB;
            work->requestedMotion = targetB;
            work->motionSpeed = 1.00000000f;
            work->firstTimer = 30;
            work->transitionTimer = 30;
            work->motionActive = 1;
            work->motionTimer = 1;
        }
        if (work->firstTimer == 24) {
            fn_1_3340(MSM_SE_SNOW_BRAWL_EFFECT_1803, (Vec *)&work->position);
        }
        if (work->firstTimer == 20) {
            work->motionState = 1;
        }
    } else {
        if (work->cooldown == 0 && work->transitionTimer == 0 &&
            (work->modelAttributes & 0x100) != 0) {
            targetA = motion->effectTargetA;
            work->requestedMotion = targetA;
            work->motionSpeed = 1.00000000f;
            work->cooldown = 30;
            work->transitionTimer = 30;
            work->motionActive = 1;
        }
        if (work->cooldown == 15) {
            distance = motion->jumpHeight;
            angle = work->rotation.y;
            sine = sin(3.1415926535897931 * (f64)angle / 180.00000000000000);
            cosine = cos(3.1415926535897931 * (f64)angle / 180.00000000000000);
            targetPosition.x = work->position.x + sine * distance;
            targetPosition.y = (f32)((f64)50.000000000000000 + work->position.y);
            targetPosition.z = work->position.z + cosine * distance;
            velocity.x = sine * motion->horizontalSpeed;
            velocity.y = motion->startSpeed;
            velocity.z = cosine * motion->horizontalSpeed;
            fn_1_B55C(work->key, (Vec *)&targetPosition, (Vec *)&velocity,
                      work->owner);
            work->motionState = 0;
            if (work->flags != 0) {
                vibrationWork = work;
                omVibrate((s16)vibrationWork->actor.playerIndex, 12, 4, 4);
            }
            fn_1_3340(MSM_SE_SNOW_BRAWL_EFFECT_1804, (Vec *)&work->position);
        }
    }
}

/* Called by the round setup phase to steer toward a destination and select a walk or idle
 * motion. */
s32 fn_1_6728(M626ObjectWork *work, Vec *target, f32 targetAngle,
              f32 speed)
{
    f32 closeMotionSpeed;
    f32 distance;
    f32 deltaX;
    f32 deltaZ;
    f32 farMotionSpeed;
    f32 angle;
    f32 movementSpeed;
    f32 desiredVelocityX;
    f32 desiredVelocityZ;
    s32 closeMotion;
    s32 slowMotion;
    s32 farMotion;
    M626PlayerConfig * settings = & work->config;
    work->motionPaused = 1;
    closeMotionSpeed = settings->profiles[0].closeMotionSpeed;
    farMotionSpeed = settings->profiles[0].farMotionSpeed;
    closeMotion = settings->profiles[0].closeMotion;
    slowMotion = settings->profiles[0].slowMotion;
    farMotion = settings->profiles[0].farMotion;
    if (work->motionState == 0)
    {
        closeMotionSpeed = settings->profiles[0].closeMotionSpeed;
        farMotionSpeed = settings->profiles[0].farMotionSpeed;
        closeMotion = settings->profiles[0].closeMotion;
        slowMotion = settings->profiles[0].slowMotion;
        farMotion = settings->profiles[0].farMotion;
    }
    else
    {
        closeMotionSpeed = settings->profiles[1].closeMotionSpeed;
        farMotionSpeed = settings->profiles[1].farMotionSpeed;
        closeMotion = settings->profiles[1].closeMotion;
        slowMotion = settings->profiles[1].slowMotion;
        farMotion = settings->profiles[1].farMotion;
    }
    deltaX = target->x - work->position.x;
    deltaZ = target->z - work->position.z;
	distance = sqrtf(deltaX * deltaX + deltaZ * deltaZ);
    if (distance < 120.0)
    {
        speed *= 0.6;
        if (distance < 70.0)
        {
            speed *= 0.3;
        }
    }
    if (distance < 50.0)
    {
        if (work->motionActive == 0)
        {
            if (distance < 15.0)
            {
                work->requestedMotion = closeMotion;
                work->motionSpeed = 1.0f;
            }
            else
            {
                work->requestedMotion = slowMotion;
                work->motionSpeed = (f32)(0.5 * closeMotionSpeed);
            }
        }
        work->position.x = (f32)(work->position.x + (0.1 * deltaX));
        work->position.z = (f32)(work->position.z + (0.1 * deltaZ));
        work->velocity.x *= 0.8;
        work->velocity.z *= 0.8;
        work->rotation.y = fn_1_3EAC(work->rotation.y, targetAngle, 0.2f);
        if (distance > 30.0)
        {
            return 0;
        }
        if (fn_1_3FC8(work->rotation.y, targetAngle) > 4.0)
        {
            return 0;
        }
        if (PSVECMag(& work->velocity) > 2.0)
        {
            return 0;
        }
        if (work->motionActive == 0)
        {
            work->requestedMotion = closeMotion;
            work->motionSpeed = 1.0f;
        }
        work->velocity.x = work->velocity.z = 0.0f;
        work->rotation.y = targetAngle;
        work->motionPaused = 0;
        return 1;
    }
    if (work->motionActive == 0)
    {
        work->requestedMotion = slowMotion;
        work->motionSpeed = closeMotionSpeed * speed;
    }
    angle = (f32)(180.0 *
        (atan2((f64)deltaX, (f64)deltaZ) / 3.141592653589793));
    work->rotation.y = fn_1_3EAC(work->rotation.y, angle, 0.3f);
    movementSpeed = (f32)speed * settings->movementScale;
    desiredVelocityX = (f32)(movementSpeed *
        sin((3.141592653589793 * (f64)angle) / 180.0));
    desiredVelocityZ = (f32)(movementSpeed *
        cos((3.141592653589793 * (f64)angle) / 180.0));
    work->velocity.x = (f32)(work->velocity.x + (0.3 * (f64)(desiredVelocityX - work->velocity.x)));
    work->velocity.z = (f32)(work->velocity.z + (0.3 * (f64)(desiredVelocityZ - work->velocity.z)));
    if (work->motionActive == 0)
    {
        if (speed < (work->motionIndex == farMotion ? 0.45 : 0.5))
        {
            work->requestedMotion = slowMotion;
            work->motionSpeed = speed * closeMotionSpeed;
        }
        else
        {
            work->requestedMotion = farMotion;
            work->motionSpeed = speed * farMotionSpeed;
        }
    }
    return 0;
}

/* Applies desired movement, turning, and locomotion animation during the player update. */
void fn_1_6D28(M626ObjectWork *work)
{
    M626PlayerConfig *motion = &work->config;
    f32 distance = work->movementSpeed * work->acceleration;
    f32 movement;
    f32 startSpeed;
    f32 endSpeed;
    f64 threshold;
    s32 fromPlayer;
    s32 toPlayer;
    s32 otherPlayer;
    s32 mode;

    if (work->transitionTimer != 0) {
        if (--work->transitionTimer == 0) {
            work->motionActive = 0;
        }
    }
    if (work->status != 0) {
        work->velocity.x = work->velocity.z = 0.00000000f;
        return;
    }
    if (work->motionPaused != 0) {
        return;
    }
    if (work->active != 0) {
        if (distance > 0.0500000000f && work->motionTimer == 0) {
            work->rotation.y = fn_1_3EAC(work->rotation.y, work->targetDistance,
                                    0.300000000f);
        }
        return;
    }

    if (work->motionState == 0) {
        startSpeed = motion->profiles[0].closeMotionSpeed;
        endSpeed = motion->profiles[0].farMotionSpeed;
        fromPlayer = motion->profiles[0].closeMotion;
        toPlayer = motion->profiles[0].slowMotion;
        otherPlayer = motion->profiles[0].farMotion;
    } else {
        startSpeed = motion->profiles[1].closeMotionSpeed;
        endSpeed = motion->profiles[1].farMotionSpeed;
        fromPlayer = motion->profiles[1].closeMotion;
        toPlayer = motion->profiles[1].slowMotion;
        otherPlayer = motion->profiles[1].farMotion;
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

    mode = work->motionActive;
    if (distance >= 0.0500000000f && work->transitionTimer == 0) {
        work->motionActive = 0;
        movement = distance * motion->movementScale;
        work->velocity.x = (f32)((f64)movement *
            sin((3.1415926535897931 * (f64)work->targetDistance) /
                180.00000000000000));
        work->velocity.z = (f32)((f64)movement *
            cos((3.1415926535897931 * (f64)work->targetDistance) /
                180.00000000000000));
        if (work->motionTimer == 0) {
            work->rotation.y = fn_1_3EAC(work->rotation.y, work->targetDistance,
                                    0.300000000f);
        }
        if (mode == 0) {
            if (work->motionIndex == otherPlayer) {
                threshold = 0.45000000000000001;
            } else {
                threshold = 0.50000000000000000;
            }
            if (distance < threshold) {
                work->requestedMotion = toPlayer;
                work->motionSpeed = distance * startSpeed;
                return;
            } else {
                work->requestedMotion = otherPlayer;
                work->motionSpeed = distance * endSpeed;
            }
        }
    } else {
        if (mode == 0) {
            work->requestedMotion = fromPlayer;
            work->motionSpeed = 1.00000000f;
        }
        work->velocity.x = work->velocity.z = 0.00000000f;
        if (distance > 0.0500000000f && work->motionTimer == 0) {
            work->rotation.y = fn_1_3EAC(work->rotation.y, work->targetDistance,
                                    0.300000000f);
        }
    }
}

Vec lbl_1_data_3F4 = {0.00000000f, 1.00000000f, 0.00000000f};

/* Handles contact between opposing players, starting the hit reaction and its feedback. */
s32 fn_1_7050(M626Entry *trigger, M626Entry *contact)
{
    OMOBJ *collisionObject;
    OMOBJ *otherObject;
    M626ObjectWork *work;
    M626ObjectWork *state;
    M626ObjectWork *otherWork;
    M626ObjectWork *otherState;
    Vec direction;

    if (contact->type == 1U) {
        collisionObject = trigger->primaryRelation;
        work = collisionObject->data;
        state = work;
        otherObject = contact->primaryRelation;
        otherWork = otherObject->data;
        otherState = otherWork;
        if (state->status == 0) {
            if (state->key != otherState->key) {
                state->status = 1;
                if (state->motionState != 0) {
                    state->motionState = 0;
                }
                direction.x = contact->direction->x;
                direction.y = 0.00000000f;
                direction.z = contact->direction->z;
                PSVECNormalize(&direction, &direction);
                PSVECCrossProduct(&lbl_1_data_3F4, &direction,
                                  &state->rotationAxis);
                state->effectRotation = (f32)(180.00000000000000 *
                                       (atan2((f64)direction.x,
                                              (f64)direction.z) /
                                        3.1415926535897931));
                state->collisionMask &= ~M626_COLLISION_MASK_UPPER_BITS;
                Hu3DModelAttrReset(state->effectModel, HU3D_MOTATTR_PAUSE);
            }
            if (state->flags != 0) {
                omVibrate((s16)work->actor.playerIndex, 12, 7, 3);
            }
            fn_1_3340(MSM_SE_SNOW_BRAWL_EFFECT_1806, &state->position);
        }
    }
    return 1;
}

f32 lbl_1_data_400[6][4] = {
{0.200000003f, 0.0199999996f, 0.00800000038f, 0.800000012f},
{0.400000006f, 0.0399999991f, 0.0149999997f, 0.600000024f},
{0.600000024f, 0.0700000003f, 0.0700000003f, 0.400000006f},
{0.800000012f, 0.100000001f, 0.100000001f, 0.200000003f},
{0.100000001f, 0.0199999996f, 0.0199999996f, 0.699999988f},
{0.100000001f, 0.0199999996f, 0.0199999996f, 0.699999988f}
};

/* Dispatches the active computer-player strategy each update. */
void fn_1_71AC(M626ObjectWork *controller)
{
    void *difficultySettings = lbl_1_data_400[controller->computerDifficulty];

    controller->motionPaused = 0;
    switch (controller->controlPhase) {
    case 0:
        controller->controlPhase++;
        fn_1_723C(controller, fn_1_7648);
    case 1:
        controller->controlUpdate(controller);
    }
}

/* Installs a computer strategy callback and clears its per-strategy state. */
void fn_1_723C(M626ObjectWork *controller,
               M626ControlUpdate callback) {
    controller->controlUpdate = callback;
    controller->controlState = 0;
    controller->controlTimer = 0;
    controller->controlProgress = 0.00000000f;
}

/* Chooses approach speed and facing while a computer player moves toward a target. */
s32 fn_1_7264(M626ObjectWork *model, Vec *target, f32 angle, f32 speed)
{
    f32 *parameters = lbl_1_data_400[model->computerDifficulty];
    Vec targetPosition = *target;
    f32 deltaX;
    f32 deltaZ;
    f32 distance;
    Vec adjustedTarget;

    deltaX = targetPosition.x - model->position.x;
    deltaZ = targetPosition.z - model->position.z;
    distance = sqrtf(deltaX * deltaX + deltaZ * deltaZ);
    if (distance > 120.00000000000000) {
        adjustedTarget.x = (f32)(targetPosition.x -
            sin((3.1415926535897931 * model->rotation.y) / 180.00000000000000) *
            50.000000000000000);
        adjustedTarget.y = targetPosition.y;
        adjustedTarget.z = (f32)(targetPosition.z -
            cos((3.1415926535897931 * model->rotation.y) / 180.00000000000000) *
            50.000000000000000);
        model->targetDistance = fn_1_407C(&model->position, &adjustedTarget);
        model->movementSpeed = speed;
        model->movementState = 0;
        return 0;
    }

    if (distance > 50.000000000000000) {
        model->targetDistance = fn_1_407C(&model->position, &targetPosition);
        model->movementSpeed = (f32)(0.50000000000000000 * speed);
        model->movementState = 0;
        return 0;
    }

    if (model->movementState == 0 && distance > 25.000000000000000) {
        model->targetDistance = fn_1_407C(&model->position, &targetPosition);
        model->movementSpeed = 0.300000000f;
        return 0;
    }

    model->movementState = 1;
    if (fabs(fn_1_3FC8(model->rotation.y, angle)) > 10.000000000000000) {
        model->targetDistance = angle;
        model->movementSpeed = 0.100000000f;
        return 0;
    }
    return 1;
}

/* Chooses a computer player's initial side strategy and resets its strategy timers. */
void fn_1_7648(M626ObjectWork *controller) {
    M626ControlUpdate callback;
    s32 sign;

    controller->controlAmount = controller->groupIndex * 50;
    if ((s32)(frand() & 1) == 0) {
        sign = 1;
    } else {
        sign = -1;
    }
    controller->controlDirection = sign;
    if (controller->key == 0) {
        if (controller->flags != 0) {
            callback = fn_1_7708;
        } else {
            callback = fn_1_7808;
        }
    } else {
        callback = fn_1_78DC;
    }
    controller->controlUpdate = callback;
    controller->controlState = 0;
    controller->controlTimer = 0;
    controller->controlProgress = 0.00000000f;
}

/* Chooses computer movement speed after checking for incoming snowballs and nearby players. */
void fn_1_7708(M626ObjectWork *controller)
{
    void *difficultySettings = lbl_1_data_400[controller->computerDifficulty];
    f32 speed = -500.000000f;

    if (fn_1_797C(controller) == 0 && fn_1_7C44(controller) == 0) {
        if (controller->computerDifficulty >= 1 && fn_1_8ADC(controller->key ^ 1) <= 1) {
            speed = -400.000000f;
        }
        if (controller->computerDifficulty >= 2 && fn_1_86C() < 600) {
            speed = -600.000000f;
        }
        if (controller->computerDifficulty >= 3 && fn_1_8ADC(controller->key) >= 4) {
            speed = -600.000000f;
        }
        fn_1_7F30(controller, speed);
    }
}

/* Chooses a speed for this computer player's movement after checking nearby opponents. */
void fn_1_7808(M626ObjectWork *controller)
{
    void *difficultySettings = lbl_1_data_400[controller->computerDifficulty];
    f32 speed = -500.000000f;

    if (fn_1_797C(controller) == 0 && fn_1_7C44(controller) == 0) {
        if (controller->groupIndex < 3) {
            if (fn_1_8ADC(controller->key ^ 1) <= 1) {
                speed = -400.000000f;
            }
        } else if (fn_1_8ADC(controller->key) >= 4) {
            speed = -400.000000f;
        }
        fn_1_7F30(controller, speed);
    }
}

/* Chooses computer movement speed for this side after checking nearby opponents. */
void fn_1_78DC(M626ObjectWork *controller)
{
    void *difficultySettings = lbl_1_data_400[controller->computerDifficulty];
    f32 speed = 500.000000f;

    if (fn_1_797C(controller) == 0 &&
        fn_1_7C44(controller) == 0) {
        if (controller->computerDifficulty >= 1) {
            speed = 600.000000f;
        }
        fn_1_7F30(controller, speed);
    }
}

/* Detects an incoming snowball and makes the computer dodge for a short interval. */
s32 fn_1_797C(M626ObjectWork *work)
{
    f32 range;
    f32 angle;
    f32 distance;
    f32 difference;
    s32 key;
    M626ObjectWork *target;

    if (work->controlCount == 0) {
        f32 *parameters = lbl_1_data_400[work->computerDifficulty];
        difference = (f32)(work->key == 0 ? 90 : 270);
        range = (f32)(200.00000000000000 + 250.00000000000000 * parameters[0]);
        key = fn_1_B62C(work->key ^ 1, &work->position, difference,
                        80.0000000f, range);
        target = fn_1_B998(key);
        if (target == NULL) {
            return 0;
        }

        angle = (f32)((f64)atan2(target->velocity.x, target->velocity.z) /
                      3.1415926535897931 * 180.00000000000000);
        distance = fn_1_407C(&target->position, &work->position);
        difference = fn_1_3FC8(angle, distance);
        if (fabs(difference) > 40.000000000000000) {
            return 0;
        }
        if (difference < 0.00000000f) {
            difference = distance - 90.0000000f;
        } else {
            difference = 90.0000000f + distance;
        }
        if (work->position.z < -350.00000000000000) {
            difference = 0.00000000f;
        }
        if (work->position.z > 350.00000000000000) {
            difference = 180.000000f;
        }
        work->controlCount = 10;
        work->outputAngle = (s32)difference;
        work->controlDirection = -work->controlDirection;
        work->reverseCount = 0;
    }

    if (work->controlCount != 0) {
        work->controlCount--;
        work->targetDistance = (f32)work->outputAngle;
        work->movementSpeed = 1.00000000f;
        return 1;
    }
    return 0;
}

/* Checks nearby opponents and occasionally dodges or changes direction. */
s32 fn_1_7C44(M626ObjectWork *work)
{
    f32 *parameters = lbl_1_data_400[work->computerDifficulty];
    f32 angle;
    f32 range;
    s32 angleLimit;
    M626ObjectWork *target;

    if (work->cooldown != 0) {
        work->targetDistance = work->secondaryAngle;
        work->movementSpeed = 0.100000000f;
        work->reverseCount = 0;
        return 1;
    }
    if (work->transitionTimer == 0) {
        if (work->key == 0) {
            angleLimit = 90;
        } else {
            angleLimit = 270;
        }
        range = (f32)angleLimit;
        angle = 40.0000000f + 30.0000000f * parameters[0];
        target = fn_1_879C(work->key ^ 1, work, &work->position, range,
                          angle, 2000.00000f, 1);
        if (target == NULL) {
            return 0;
        } else {
            angle = fn_1_4118(&work->position, &target->position);
            if (work->motionState == 0) {
                if (angle > 400.00000000000000 &&
                    0.000015258789062500000 *
                        (f64)(f32)(frand() & 0xFFFF) < parameters[1]) {
                    work->modelAttributes |= 0x200;
                    work->reverseCount = 0;
                    return 1;
                }
            } else if (angle < 900.00000000000000 &&
                       0.000015258789062500000 *
                           (f64)(f32)(frand() & 0xFFFF) < parameters[2]) {
                work->modelAttributes |= 0x100;
                work->controlDirection = -work->controlDirection;
                work->reverseCount = 0;
                work->secondaryAngle = fn_1_407C(
                    &work->position, &target->position);
                work->secondaryAngle = (f32)((f64)work->secondaryAngle +
                    25.000000000000000 * (parameters[3] *
                    (0.000030517578125000000 *
                         (f64)(f32)((frand() & 0xFFFF) - 32768))));
                return 1;
            }
        }
    }
    return 0;
}

/* Advances the computer player's running path, avoiding snowballs and nearby players. */
s32 fn_1_7F30(M626ObjectWork *work, f32 baseY)
{
    void *selectionData = lbl_1_data_400[work->computerDifficulty];
    M626ObjectWork * target;
    Vec goal;
    Vec destination;
    Vec currentTarget;
    f32 targetZ;
    f32 distance;
    f32 targetX;
    f32 angle;
    f32 currentAngle;
    f32 distanceSquared;
    f32 distanceToTarget;
    if ((f64)work->reverseCount > 180.00000000000000)
    {
        target = fn_1_879C(work->key ^ 1, work, &work->position, 0.00000000f, 180.000000f,
                           3000.00000f, 1);
        if (target != NULL)
        {
            angle = fn_1_407C(& work->position, &target->position);
            work->targetDistance = angle;
            work->movementSpeed = 0.800000000f;
            if (work->reverseCount > 360.00000000000000)
            {
                target = fn_1_879C(work->key, work, &work->position, work->rotation.y, 40.0000000f,
                                   200.000000f, 2);
                if (target != NULL)
                {
                    work->reverseCount = 0.0f;
                }
            }
            return 1;
        }
    }
    target =
        fn_1_879C(work->key, work, &work->position, work->rotation.y, 40.0000000f, 200.000000f, 3);
    if (target != NULL)
    {
        work->controlAmount = (work->controlDirection * 10) + work->controlAmount;
    }
    angle = (f32) work->controlAmount * 1.5;
    goal.x = (f32) (baseY + 200.00000000000000 *
                                sin((3.1415926535897931 * (f64) angle) / 180.00000000000000));
    goal.y = work->position.y;
    goal.z =
        (f32) (350.00000000000000 * cos((3.1415926535897931 * (f64) angle) / 180.00000000000000));
    if (fn_1_4118(& work->position, & goal) > 30.000000000000000)
    {
        void *movementData;
        currentAngle = work->rotation.y;
        movementData = lbl_1_data_400[work->computerDifficulty];
        currentTarget = goal;
        targetX = currentTarget.x - work->position.x;
        targetZ = currentTarget.z - work->position.z;
        distanceSquared = targetX * targetX + targetZ * targetZ;
        distanceToTarget = sqrtf(distanceSquared);
        if (distanceToTarget > 120.00000000000000)
        {
            destination.x =
                (f32) (currentTarget.x -
                       50.000000000000000 *
                           sin((3.1415926535897931 * (f64) work->rotation.y) / 180.00000000000000));
            destination.y = currentTarget.y;
            destination.z =
                (f32) (currentTarget.z -
                       cos((3.1415926535897931 * (f64) work->rotation.y) / 180.00000000000000) *
                           50.000000000000000);
            work->targetDistance = fn_1_407C(& work->position, & destination);
            work->movementSpeed = 1.00000000f;
            work->movementState = 0;
        }
        else
        {
            if (distanceToTarget > 50.000000000000000)
            {
                work->targetDistance = fn_1_407C(& work->position, & currentTarget);
                work->movementSpeed = 0.500000000f;
                work->movementState = 0;
            }
            else
            {
                if (work->movementState == 0 && distanceToTarget > 25.000000000000000)
                {
                    work->targetDistance = fn_1_407C(& work->position, & currentTarget);
                    work->movementSpeed = 0.300000000f;
                }
                else
                {
                    work->movementState = 1;
                    if (fabs(fn_1_3FC8(work->rotation.y, currentAngle)) > 10.0)
                    {
                        work->targetDistance = currentAngle;
                        work->movementSpeed = 0.100000000f;
                    }
                }
            }
        }
    }
    work->controlAmount += work->controlDirection;
    work->reverseCount++;
    return 1;
}

/* Clears per-frame control requests before the player state machine runs. */
void fn_1_8570(M626ObjectWork *state) {
    state->modelAttributes = 0;
    state->controlWord = 0;
    state->targetDistance = 0.00000000f;
    state->movementSpeed = 0.00000000f;
}

Vec lbl_1_data_460 = {0.00000000f, 1.00000000f, 0.00000000f};

/* Builds the player model's rotation matrix, applies hit tilt, and updates its transform. */
void fn_1_85A4(M626ObjectWork *state, s32 model)
{
    Qtrn rotation;
    Qtrn tiltRotation;
    void *workEnd;
    f32 previousAngle;
    f32 wrappedAngle;
    f32 rotationAngle;
    f32 ambient;

    workEnd = &state->config;
    previousAngle = state->rotation.y;
    wrappedAngle = (f32)fmod((f64)previousAngle, 360.0);
    rotationAngle = wrappedAngle;
    state->rotation.y = rotationAngle;
    if (state->rotation.y < 0.0) {
        state->rotation.y += 360.0;
    }
    C_QUATRotAxisRad(&rotation, &lbl_1_data_460,
                     0.017453292f * state->rotation.y);
    if (state->status != 0) {
        C_QUATRotAxisRad(&tiltRotation, &state->rotationAxis,
                         0.017453292f * state->tiltAngle);
        PSQUATMultiply(&tiltRotation, &rotation, &rotation);
        ambient = (f32)(1.0 - ((f64)state->tiltAngle / 90.0));
        if (ambient < 0.0) {
            ambient = 0.0f;
        }
        Hu3DModelAmbSet(model, ambient, ambient, 1.0f);
        Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    }
    PSQUATNormalize(&rotation, &rotation);
    PSMTXQuat(state->rotationMatrix, &rotation);
    PSMTXCopy(state->rotationMatrix, Hu3DData[model].mtx);
    Hu3DModelPosSetV(model, &state->position);
    state->motionFrame++;
}

/* Used by computer strategies to find the nearest player that fits a side and status filter. */
void *fn_1_879C(u32 key, void *excludedWork, Vec *position, f32 centerAngle,
                f32 halfAngle, f32 maxDistance, u32 statusFlags)
{
    M626ObjectWork *candidate;
    void *closest = NULL;
    f32 closestDistance = 100000.0f;
    f32 x, z;
    f32 relativeAngle;
    f32 wrappedAngle;
    s32 index;

    for (index = 0; index < 8; index++) {
        candidate = lbl_1_bss_2D4[index];
        if (candidate == NULL || candidate == excludedWork || candidate->key != key) {
            continue;
        }
        if ((statusFlags & 1) == 0 && candidate->status == 0) {
            continue;
        }
        if ((statusFlags & 2) == 0 && candidate->status != 0) {
            continue;
        }
        x = candidate->position.x - position->x;
        z = candidate->position.z - position->z;
        if (halfAngle < 180.0) {
            relativeAngle = (f32)(180.0 *
                (atan2((f64)x, (f64)z) / 3.141592653589793));
            wrappedAngle = (f32)fmod((f64)(relativeAngle - centerAngle), 360.0);
            relativeAngle = wrappedAngle;
            if (relativeAngle < 0.0f) {
                relativeAngle += 360.0f;
            }
            if (relativeAngle > halfAngle && relativeAngle < 360.0f - halfAngle) {
                continue;
            }
        }
        relativeAngle = sqrtf(x * x + z * z);
        if (relativeAngle > maxDistance) {
            continue;
        }
        if (relativeAngle < closestDistance) {
            closestDistance = relativeAngle;
            closest = candidate;
        }
    }
    return closest;
}

/* Counts active registered Snow Brawl players assigned to the requested side. */
s32 fn_1_8ADC(u32 key)
{
    M626ObjectWork *object;
    s32 count = 0;
    s32 index;

    for (index = 0; index < 8; index++) {
        object = lbl_1_bss_2D4[index];
        if (object != NULL && object->key == key && object->status == 0) {
            count++;
        }
    }
    return count;
}

/* Returns whether the indexed player's snowball effect has completed. */
u32 fn_1_8B58(s32 index) {
    OMOBJ *object = lbl_1_bss_2F4[index];
    M626ObjectWork *data = object->data;
    return data->effectComplete;
}

/* Reports whether the indexed character is still in active play. */
s32 fn_1_8B90(s32 playerIndex) {
    OMOBJ *object;
    M626ObjectWork *work;

    object = lbl_1_bss_2F4[playerIndex];
    work = object->data;
    return work->status == 0;
}

/* Reports whether a human-controlled player is assigned to the requested side. */
s32 fn_1_8BD0(u32 key)
{
    OMOBJ *object;
    M626ObjectWork *work;
    M626ObjectWork *currentWork;
    s32 index;

    for (index = 0; index < 4; index++) {
        object = lbl_1_bss_2F4[index];
        work = object->data;
        currentWork = work;
        if (currentWork->key == key && currentWork->playerType == 0) {
            return 1;
        }
    }
    return 0;
}

s32 lbl_1_bss_2D0;
