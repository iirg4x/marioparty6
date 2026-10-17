/* Player input, movement, character motions and collision responses. */
#include "dolphin/math.h"
#include "game/main.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "game/charman.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/audio.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/mg/seqman.h"
#include "datadir_enum.h"
#include <string.h>

typedef struct M675Collision M675Collision;
struct M675Collision {
    u32 flags, mask; /* Collision enable flags and overlap-channel mask. */
    u32 type; /* Event type reported by this collision record. */
    s32 index; /* Owner-supplied record index. */
    s32 shapeType; /* Collision shape selector: 0 sphere, 10 ring, 11 boundary. */
    OMOBJ *owner; /* Object that owns this collision record. */
    void *excludedOwner; /* Other owner ignored when the collision pair is checked. */
    OMOBJ_FUNC update; /* Callback that refreshes the owner's transform. */
    s32 (*contact)(M675Collision *, M675Collision *); /* Pair-contact callback. */
    Vec *position, *velocity; /* World position and velocity used by collision response. */
    Vec positionCopy; /* Position snapshot stored when the record is initialized. */
    f32 radius; /* Collision radius. */
    f32 yieldOnContact; /* Nonzero makes this sphere move against a zero-valued sphere. */
    s32 handled; /* Nonzero after another callback handles this event. */
};
typedef struct M675EffectRegistration {
    s32 active; /* Nonzero enables drawing this registration's ground ring. */
    f32 radius; /* World-space radius around position. */
    Vec *position; /* Address of the registered effect's world position. */
} M675EffectRegistration;
typedef struct M675PlayerConfig {
    u32 unmappedConfigWordsBeforeMovement[2]; /* Copied setup data with no established player
                                               * use. */
    f32 movementScale; /* Multiplies the player's travel speed. */
    u32 unmappedConfigWordsAfterMovementScale[6]; /* Copied setup data with no established player
                                                   * use. */
    s32 defaultMotion, slowMotion; /* Motions used while stopped and moving slowly. */
    f32 slowMotionSpeed; /* Playback speed multiplier for slowMotion. */
    s32 fastMotion; /* Motion used while moving quickly. */
    f32 fastMotionSpeed; /* Playback speed multiplier for fastMotion. */
    u32 unmappedConfigWordsBeforeMotions[5]; /* Copied setup data with no established player use. */
    s32 idleMotion, actionMotion, firstAlternateMotion, secondAlternateMotion; /* Input motions. */
    s32 contactMotion, winMotion, loseMotion, exitMotion; /* Contact and result motions. */
} M675PlayerConfig;
/* State passed to the shared actor input, motion and transform helpers. */
typedef struct M675ActorState {
    M675Collision collision; /* Player contact data; prop instances leave it disabled. */
    s32 playerMappingIndex; /* Player mapping used by the player gameplay helpers. */
    s32 formationIndexAtCreation; /* Formation slot supplied when a player is created. */
    s32 formationIndex; /* Current player slot in the formation arrays. */
    s32 active; /* Gates human input, motion-offset easing, result arrival reporting and character
                 * feedback. */
    Vec basePosition; /* Player formation position before a motion offset is applied. */
    Vec position, velocity, rotation; /* World position, per-frame movement and Euler angles. */
    f32 baseYaw; /* Player facing angle before its motion offset is applied. */
    u32 buttonsDown, buttons; /* Buttons pressed this frame and currently held. */
    f32 stickAngle, stickMagnitude; /* Stick angle in degrees and its magnitude. */
    s32 motion, motionFrame; /* Selected motion and transform-update count since its selection. */
    s32 requestedMotion; /* Motion queued by the current frame's behavior. */
    f32 motionSpeed; /* Playback speed for the queued motion. */
} M675ActorState;

typedef struct M675PlayerWork {
    M675ActorState actor;
    s32 phase, phaseTimer; /* Current player phase and its frame timer. */
    u8 unmappedPlayerGapBeforeComputerState[48]; /* Existing storage with no demonstrated player
                                                  * use. */
    s32 updateCounter; /* Frames elapsed since the manager registered this player. */
    s32 computer; /* Whether the player uses computer-controlled input. */
    s32 difficulty; /* Computer-player difficulty setting, in the range 0 through 3. */
    s32 computerPhase, actionDelay, randomDelay; /* Computer input phase and its frame timers. */
    f32 triggerHeight; /* Height threshold used by the computer player's effect check. */
    s32 motionActive, motionTimer, movementInProgress; /* Current motion and movement state. */
    s32 unmappedPlayerWordAfterMovement; /* Existing word with no demonstrated player use. */
    /* Hit response state and initialization count. */
    s32 contactEvent, contactCooldown, contactPhase, initializedCounter;
    f32 initializedScale; /* Scale value written during player registration. */
    u8 unmappedPlayerGapBeforeEffect[28]; /* Existing storage with no demonstrated player use. */
    M675EffectRegistration effect; /* Position and radius registered for game effects. */
    M675PlayerConfig config; /* Movement and character-motion settings. */
    s32 playerIndex; /* Player number from the game setup. */
    s32 character; /* Character number used by character-model APIs. */
    s32 processPhase; /* Outer player-object phase advanced by fn_1_55F4. */
    s32 unmappedPlayerWordBeforeUpdateCount; /* Existing word with no demonstrated player use. */
    s32 updateCount; /* Number of times fn_1_5FCC has run. */
    s32 voiceState; /* Voice setting cleared before the result voice-volume update. */
} M675PlayerWork;
typedef struct M675MotionParameter {
    u16 motionIndex; /* Index into the character object's motion array. */
    s16 overlay; /* Overlay motion index, or -1 when no overlay is used. */
    f32 blend; /* Transition time as a fraction of the standard blend duration. */
    u32 attr; /* Main and overlay motion playback attributes. */
} M675MotionParameter;
typedef struct M675SequenceDifficultySettings {
    f32 baseTimingAndHeightScale; /* Factor controlling baseline action delay and trigger height. */
    f32 randomTimingScale; /* Factor controlling random action delay. */
    f32 randomHeightScale; /* Factor controlling random trigger height. */
    s32 baseCount; /* Progress count adjusted by the remaining sequence time. */
} M675SequenceDifficultySettings;

#define M675_CONFIG_WORD_00_BIT_PATTERN 0x41F00000
#define M675_CONFIG_WORD_04_BIT_PATTERN 0x43160000
#define M675_CONFIG_WORD_0C_BIT_PATTERN 0x41A00000
#define M675_CONFIG_WORD_10_BIT_PATTERN 0xBFC00000
#define M675_CONFIG_WORD_14_BIT_PATTERN 0x3F800000
#define M675_CONFIG_WORD_18_BIT_PATTERN 0x3F000000
#define M675_CONFIG_WORD_1C_BIT_PATTERN 0x3FA00000
#define M675_CONFIG_WORD_20_BIT_PATTERN 0x3F800000
#define M675_CONFIG_WORD_40_BIT_PATTERN 0x40000000
#define M675_CONFIG_WORD_48_BIT_PATTERN 0x40200000

OMOBJ *lbl_1_bss_310;
OMOBJ *lbl_1_bss_300[4];
u32 lbl_1_bss_2FC;
u32 lbl_1_bss_2F8;
M675PlayerWork *lbl_1_bss_2E4[5];
s32 lbl_1_bss_2E0;
void fn_1_B14(s32,s32,s32);
s32 fn_1_A98(s32);
void fn_1_B4C(s32);
s32 fn_1_B6C(void);
s32 fn_1_D3CC(void);
s32 fn_1_D494(s32,s32);
s32 fn_1_A60(void);
s32 fn_1_F058(s32);
Vec *fn_1_D550(s32,s32);
s32 fn_1_32EC(Vec *);
void fn_1_4424(s32,s32,Vec *,s32,s32);
void fn_1_4BB4(Vec *,Vec *,f32,Vec *);
f32 fn_1_47DC(f32,f32,f32);
f32 fn_1_48F8(f32,f32);
s32 fn_1_10170(M675Collision *);
void fn_1_E918(M675EffectRegistration *);
void fn_1_4F2C(HUPROCESS *manager);
void fn_1_5048(void);
void fn_1_506C(OMOBJ *manager);
void fn_1_50BC(OMOBJ *manager);
void fn_1_5170(M675PlayerWork *state);
void fn_1_527C(OMOBJ *object);
void fn_1_55F4(OMOBJ *object);
s32 fn_1_5740(M675PlayerWork *work);
s32 fn_1_5998(M675PlayerWork *work);
s32 fn_1_5CB4(M675PlayerWork *work);
s32 fn_1_5EB4(M675PlayerWork *state);
void fn_1_5FCC(OMOBJ *object);
void fn_1_6068(OMOBJ *object, s32 effect);
void fn_1_60DC(M675PlayerWork *state);
void fn_1_6198(OMOBJ *object);
void fn_1_61F4(M675ActorState *state);
void fn_1_6210(M675ActorState *state, s32 motionIndex, f32 speed);
void fn_1_621C(OMOBJ *object);
void fn_1_6284(OMOBJ *object, s32 index);
s32 fn_1_6460(OMOBJ *object);
s32 fn_1_64CC(OMOBJ *object);
void fn_1_6524(M675PlayerWork *work);
s32 fn_1_66EC(M675Collision *callback, M675Collision *event);
void fn_1_67CC(M675PlayerWork *state);
s32 fn_1_68B0(M675PlayerWork *work, Vec *target, f32 targetAngle,
    f32 speed);
void fn_1_6E78(M675PlayerWork *work);
void fn_1_74C8(M675ActorState *state);
void fn_1_74FC(M675ActorState *state, s32 modelId);
void fn_1_516C(M675PlayerWork *work);

u32 lbl_1_data_100[9] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 1), DATANUM(DATA_mariomot, 2),
    DATANUM(DATA_mario, 206), DATANUM(DATA_mario, 205), DATANUM(DATA_mariomot, 64),
    DATANUM(DATA_mariomot, 21), DATANUM(DATA_mariomot, 38), DATANUM(DATA_mariomot, 40)
};

M675MotionParameter lbl_1_data_124[11] = {
    {0, -1, 0.2f, HU3D_MOTATTR_LOOP},
    {1, -1, 0.2f, HU3D_MOTATTR_LOOP},
    {2, -1, 0.2f, HU3D_MOTATTR_LOOP},
    {3, -1, 0.2f, 0},
    {4, -1, 0.2f, 0},
    {5, -1, 0.2f, 0},
    {0, -1, 0.2f, HU3D_MOTATTR_LOOP},
    {6, -1, 0.2f, HU3D_MOTATTR_LOOP},
    {7, -1, 0.2f, 0},
    {8, -1, 0.2f, 0},
    {0, -1, 0.2f, HU3D_MOTATTR_LOOP}
};

Vec lbl_1_data_1A8[4] = {
    {-1.3e+03f, 2e+02f, 6.6e+02f},
    {-1e+03f, 2e+02f, 6.6e+02f},
    {1e+03f, 2e+02f, 6.6e+02f},
    {1.3e+03f, 2e+02f, 6.6e+02f}
};

f32 lbl_1_data_1D8[4] = {
    0.0f,
    0.0f,
    0.0f,
    0.0f
};

Vec lbl_1_data_1E8[4] = {
    {-4.5e+02f, 2e+02f, 6.6e+02f},
    {-1.5e+02f, 2e+02f, 6.6e+02f},
    {2.5e+02f, 2e+02f, 6.6e+02f},
    {5.5e+02f, 2e+02f, 6.6e+02f}
};

f32 lbl_1_data_218[4] = {
    1.8e+02f,
    1.8e+02f,
    1.8e+02f,
    1.8e+02f
};

Vec lbl_1_data_228[4] = {
    {-5.4e+02f, 2e+02f, 5.3e+02f},
    {-2.3e+02f, 2e+02f, 5.3e+02f},
    {8e+01f, 2e+02f, 5.3e+02f},
    {3.9e+02f, 2e+02f, 5.3e+02f}
};

f32 lbl_1_data_258[4] = {
    3e+01f,
    3e+01f,
    3e+01f,
    3e+01f
};

Vec lbl_1_data_268[4] = {
    {-4.5e+02f, 2e+02f, 6.6e+02f},
    {-1.5e+02f, 2e+02f, 6.6e+02f},
    {1.5e+02f, 2e+02f, 6.6e+02f},
    {4.5e+02f, 2e+02f, 6.6e+02f}
};

f32 lbl_1_data_298[4] = {
    0.0f,
    0.0f,
    0.0f,
    0.0f
};

Vec lbl_1_data_2A8[11] = {
    {0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f},
    {55.0f, 0.0f, 0.0f},
    {55.0f, 0.0f, 0.0f},
    {-3e+01f, 0.0f, 0.0f},
    {-3e+01f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f},
    {-1e+01f, 0.0f, 0.0f}
};

f32 lbl_1_data_32C[14] = {
    0.0f, 0.0f, 0.0f, 5e+01f, 5e+01f, 0.0f, 0.0f, -1e+01f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
};

M675PlayerConfig lbl_1_data_364 = {
    {M675_CONFIG_WORD_00_BIT_PATTERN, M675_CONFIG_WORD_04_BIT_PATTERN},
    1e+01f,
    {M675_CONFIG_WORD_0C_BIT_PATTERN, M675_CONFIG_WORD_10_BIT_PATTERN,
        M675_CONFIG_WORD_14_BIT_PATTERN, M675_CONFIG_WORD_18_BIT_PATTERN,
        M675_CONFIG_WORD_1C_BIT_PATTERN, M675_CONFIG_WORD_20_BIT_PATTERN},
    0,
    1,
    2.0f,
    2,
    1.5f,
    {0, 0, M675_CONFIG_WORD_40_BIT_PATTERN, 0, M675_CONFIG_WORD_48_BIT_PATTERN},
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10
};

/* Called by fn_1_A0 to create the player manager and four player objects. */
void fn_1_4F2C(HUPROCESS *manager)
{
    OMOBJ *object;
    s32 i;

    lbl_1_bss_2FC = 0;
    lbl_1_bss_2F8 = 0;
    lbl_1_bss_310 = omAddObjEx(manager, 14, 0, 0, -1, fn_1_50BC);
    fn_1_506C(lbl_1_bss_310);
    omMakeGroupEx(manager, 0, 4);
    for (i = 0; i < 4; i++) {
        object = lbl_1_bss_300[i] = omAddObjEx(manager, 10, 1, 9, 0, fn_1_527C);
        object->work[0] = i;
        object->work[1] = i;
        object->work[2] = i;
        fn_1_B14(i, i, GwPlayerConf[i].charNo);
    }
}

void fn_1_5048(void)
{
    CharModelKill(-1);
}

/* Clears the shared player registry before fn_1_4F2C creates the players. */
void fn_1_506C(OMOBJ *manager)
{
    s32 index;

    index = 0;
    while (index < 5) {
        lbl_1_bss_2E4[index] = NULL;
        index += 1;
    }
    lbl_1_bss_2E0 = 0;
}

/* Runs once per manager update to advance registered players' shared timers. */
void fn_1_50BC(OMOBJ *manager)
{
    s32 index;
    M675PlayerWork *object;

    if ((s32)lbl_1_bss_2E0 == 0) {
        lbl_1_bss_2E0 = 1;
    }
    index = 0;
    while (index < 5) {
        object = lbl_1_bss_2E4[index];
        if (object != NULL) {
            fn_1_516C(object);
            if (object->contactCooldown > 0) {
                object->contactCooldown -= 1;
            }
            object->updateCounter += 1;
        }
        index += 1;
    }
}

void fn_1_516C(M675PlayerWork *work)
{
}

/* Registers a new player and gives it its initial formation position. */
void fn_1_5170(M675PlayerWork *state)
{
    M675PlayerConfig *config = &state->config;
    s32 index;
    s32 player;

    for (index = 0; index < 5; index++) {
        if (lbl_1_bss_2E4[index] == NULL) {
            lbl_1_bss_2E4[index] = state;
            player = state->actor.formationIndex;
            state->actor.basePosition = lbl_1_data_1A8[player];
            state->actor.position =
                state->actor.basePosition;
            state->actor.velocity.x =
                state->actor.velocity.y =
                    state->actor.velocity.z = 0.0f;
            state->actor.baseYaw = lbl_1_data_1D8[player];
            state->actor.rotation.y =
                state->actor.baseYaw;
            state->contactPhase = 0;
            state->initializedCounter = 0;
            state->initializedScale = 1.0f;
            break;
        }
    }
}

/* Initializes a player object, its character model, and collision/effect records. */
void fn_1_527C(OMOBJ *object)
{
    M675PlayerWork *work;
    M675PlayerWork *state;
    M675Collision *callback;
    s32 index;
    s32 player;
    s32 character;
    s32 formationIndex;
    s32 mappingIndex;
    M675PlayerConfig *configDestination;
    s32 modelId;
    s32 motionId;

    state = HuMemDirectMallocNum(HEAP_HEAP, 460, HU_MEMNUM_OVL);
    object->data = state;
    memset(state, 0, 460);
    object->stat |= OM_STAT_MODELPAUSE;
    work = state;
    state->playerIndex = player = object->work[0];
    character = GwPlayerConf[player].charNo;
    state->character = character;
    mappingIndex = work->actor.playerMappingIndex = object->work[1];
    formationIndex = object->work[2];
    work->actor.formationIndexAtCreation = work->actor.formationIndex = formationIndex;
    work->actor.active = 1;
    work->computer = GwPlayerConf[player].type;
    work->difficulty = GwPlayerConf[player].comDif & 3;
    work->computerPhase = 0;
    modelId = CharModelCreate(character, 4);
    object->mdlId[0] = modelId;
    Hu3DModelLayerSet(modelId, 4);
    Hu3DModelShadowMapSet(modelId);
    CharEffectLayerSet(6);
    for (index = 0; index < 9; index++) {
        motionId = CharMotionCreate(character, lbl_1_data_100[index]);
        object->mtnId[index] = motionId;
    }
    CharMotionDataClose(character);
    work->actor.motion = -1;
    fn_1_6284(object, 0);
    configDestination = &work->config;
    *configDestination = lbl_1_data_364;
    fn_1_5170(work);
    callback = &work->actor.collision;
    callback->flags = 1;
    callback->mask = 9;
    callback->type = 0;
    callback->index = player;
    callback->shapeType = 0;
    callback->owner = object;
    callback->update = fn_1_6198;
    callback->contact = 0;
    callback->position = &work->actor.position;
    callback->velocity = &work->actor.velocity;
    callback->positionCopy = *callback->position;
    callback->radius = 50.0f;
    callback->yieldOnContact = 0.0f;
    fn_1_10170(callback);
    work->effect.active = 1;
    work->effect.radius = 50.0f;
    work->effect.position = &work->actor.position;
    fn_1_E918(&work->effect);
    state->processPhase = 0;
    state->voiceState = 1;
    object->objFunc = fn_1_55F4;
}

/* Advances a player's setup, input, finish, and results phases on each update. */
void fn_1_55F4(OMOBJ *object)
{
    M675PlayerWork *work = object->data;
    M675PlayerWork *state = work;

    fn_1_74C8(&state->actor);
    fn_1_61F4(&state->actor);
    switch (work->processPhase) {
    case 0:
        if (fn_1_5740(state)) {
            state->actor.collision.contact = fn_1_66EC;
            work->processPhase++;
            state->phase = 0;
        }
        break;
    case 1:
        if (fn_1_5998(state)) {
            work->processPhase++;
            state->phase = 0;
        }
        break;
    case 2:
        if (fn_1_5CB4(state)) {
            work->processPhase++;
            state->phase = 0;
        }
        break;
    case 3:
        if (fn_1_5EB4(state)) {
            work->processPhase++;
            state->phase = 0;
        }
        break;
    }
    fn_1_67CC(state);
    fn_1_621C(object);
    PSVECAdd(&state->actor.position, &state->actor.velocity, &state->actor.position);
    fn_1_5FCC(object);
}

/* Moves players through their entry formation when called by fn_1_55F4. */
s32 fn_1_5740(M675PlayerWork *work)
{
    s32 done;
    M675PlayerConfig *config;
    Vec *placement;
    f32 targetYaw;

    config = &work->config;
    done = 0;

    switch (work->phase) {
    case 0:
        work->phase++;
        work->phaseTimer = 0;
        break;
    case 1:
        if (fn_1_D3CC() == 0) {
            break;
        }
        work->phase++;
        work->phaseTimer = 0;
        /* fall through */
    case 2:
        placement = &lbl_1_data_1E8[work->actor.formationIndex];
        targetYaw = lbl_1_data_218[work->actor.formationIndex];
        if (fn_1_68B0(work, placement, targetYaw, 1.0f) == 0) {
            break;
        }
        work->phase++;
        work->phaseTimer = 0;
        work->actor.basePosition = *placement;
        work->actor.baseYaw = targetYaw;
        lbl_1_bss_2FC++;
        /* fall through */
    case 3:
        if (lbl_1_bss_2FC < 4U) {
            break;
        }
        work->phase++;
        work->phaseTimer = 0;
        /* fall through */
    case 4:
        placement = &lbl_1_data_228[work->actor.formationIndex];
        targetYaw = lbl_1_data_258[work->actor.formationIndex];
        if (fn_1_68B0(work, placement, targetYaw, 1.0f) == 0) {
            break;
        }
        work->phase++;
        work->phaseTimer = 0;
        work->actor.basePosition = *placement;
        work->actor.baseYaw = targetYaw;
        lbl_1_bss_2F8++;
        /* fall through */
    case 5:
        if (lbl_1_bss_2F8 < 4U) {
            break;
        }
        work->phase++;
        work->phaseTimer = 0;
        /* fall through */
    case 6:
        if (MgSeqModeGet() != 5) {
            break;
        }
        work->phase++;
        /* fall through */
    default:
        done = 1;
        break;
    }
    return done;
}

/* Called by fn_1_55F4 each frame to read input or advance the computer player. */
s32 fn_1_5998(M675PlayerWork *work)
{
    M675PlayerConfig *configuration = &work->config;
    s32 controllerIndex;
    s32 done = 0;
    M675PlayerWork *input;
    float x, y;

    switch (work->phase) {
    case 0:
        work->phase++;
        work->phaseTimer = 0;
    case 1:
        if (MgSeqModeGet() == MGSEQ_MODE_FINISH) {
            work->phase++;
            work->phaseTimer = 0;
        } else if (work->computer) {
            fn_1_6E78(work);
        } else if (work->actor.active) {
            input = work;
            controllerIndex = GwPlayerConf[input->playerIndex].padNo;
            x = HuPadStkXf[controllerIndex];
            y = -HuPadStkYf[controllerIndex];
            work->actor.buttons = HuPadBtn[controllerIndex];
            work->actor.buttonsDown = HuPadBtnDown[controllerIndex];
            work->actor.stickAngle = 180.0 * (atan2(x, y) / M_PI);
            work->actor.stickMagnitude = sqrtf(x * x + y * y);
        }
        break;
    case 2:
        work->phase++;
        work->phaseTimer = 0;
    case 3:
        if (++work->phaseTimer < 10) { break; }
        work->phase++;
        work->phaseTimer = 0;
    default:
        done = 1;
        /* This completion path returns without a value even though the player update
         * tests its result. */
        return;
    }
    fn_1_6524(work);
    return done;
}

/* Called by fn_1_55F4 each frame to run the exit and wait phases after play. */
s32 fn_1_5CB4(M675PlayerWork *work)
{
    Vec targetPosition;
    M675PlayerConfig *motionSettings = &work->config;
    s32 complete = 0;
    f32 targetYaw;
    M675PlayerWork *callbackWork;

    switch (work->phase) {
    case 0:
        work->phase++;
        work->phaseTimer = 0;
        /* fall through */
    case 1:
        fn_1_6524(work);
        if ((u32) MgSeqModeGet() == 6) {
            break;
        }
        work->phase++;
        work->phaseTimer = 0;
        work->motionActive = 0;
        work->motionTimer = 0;
        work->contactCooldown = 0;
        fn_1_6210(&work->actor, motionSettings->exitMotion, 1.0f);
        /* fall through */
    case 2:
        fn_1_60DC(work);
        if (++work->phaseTimer < 30) {
            break;
        }
        work->phase++;
        work->phaseTimer = 0;
        /* fall through */
    case 3:
        targetPosition = lbl_1_data_268[work->actor.formationIndex];
        targetYaw = lbl_1_data_298[work->actor.formationIndex];
        if (fn_1_68B0(work, &targetPosition, targetYaw, 0.800000011920929f) == 0) {
            break;
        }
        work->phase++;
        work->phaseTimer = 0;
        /* fall through */
    case 4:
        if (work->actor.active != 0) {
            callbackWork = work;
            if (++work->phaseTimer < 60) {
                break;
            }
            fn_1_B4C(callbackWork->playerIndex);
        }
        if ((u32) MgSeqModeGet() == 8 && fn_1_B6C() != 0) {
            work->phase++;
            work->phaseTimer = 0;
        } else {
            break;
        }
        /* fall through */
    default:
        complete = 1;
        break;
    }
    return complete;
}

/* Called by fn_1_55F4 to play the result motion and finish after 90 frames. */
s32 fn_1_5EB4(M675PlayerWork *state)
{
    M675PlayerConfig *config = &state->config;
    M675PlayerWork *work;
    s32 done = 0;

    switch (state->phase) {
    case 0:
        if (fn_1_A98(state->actor.playerMappingIndex) > 0) {
            fn_1_6210(&state->actor, config->winMotion, 1.0f);
        } else {
            fn_1_6210(&state->actor, config->loseMotion, 1.0f);
        }
        if (state->actor.active) {
            work = state;
            work->voiceState = 0;
            CharModelVoiceVolSet(work->character, 127);
        }
        state->phase++;
        state->phaseTimer = 0;
        break;
    case 1:
        if (++state->phaseTimer < 90) {
            break;
        }
        state->phase++;
        state->phaseTimer = 0;
    default:
        done = 1;
        break;
    }
    return done;
}

/* Refreshes motion completion and voice pan each frame; the pan call consumes a wrapper
 * that discards its result. */
void fn_1_5FCC(OMOBJ *object)
{
    M675PlayerWork *state;
    M675PlayerWork *work = object->data;

    state = work;
    work->updateCount++;
    if (state->motionActive) {
        if (fn_1_6460(object) != 0) {
            state->motionActive = 0;
            state->motionTimer = 0;
        }
    } else {
        state->motionTimer = 0;
    }
    CharModelVoicePanSet((s16) work->character,
                         (s16) fn_1_32EC(&state->actor.position));
}

/* Plays and tracks a character effect; its initial pan comes from a wrapper that discards
 * the calculated pan. */
void fn_1_6068(OMOBJ *object, s32 effect)
{
    M675PlayerWork *work = object->data;
    M675PlayerWork *owner = work;
    s32 handle = CharFXPlayPan((s16)work->character, (s16)effect,
                               (s16)fn_1_32EC(&owner->actor.position));

    fn_1_4424(effect, handle, &owner->actor.position, 1, 0);
}

/* Eases the active player toward its exit position during fn_1_5CB4. */
void fn_1_60DC(M675PlayerWork *state)
{
    Vec position;
    f32 targetAngle;

    if (state->actor.active != 0) {
        PSVECAdd(&state->actor.basePosition, &lbl_1_data_2A8[state->actor.motion], &position);
        fn_1_4BB4(&state->actor.position, &position, 0.30000001192092896f, &state->actor.position);
        targetAngle = state->actor.baseYaw + lbl_1_data_32C[state->actor.motion];
        state->actor.rotation.y =
            fn_1_47DC(state->actor.rotation.y, targetAngle, 0.30000001192092896f);
    }
}

/* Supplies the player's model transform when the collision system updates it. */
void fn_1_6198(OMOBJ *object)
{
    M675PlayerWork *work;
    M675PlayerWork *state;
    s32 model;

    work = object->data;
    state = work;
    model = object->mdlId[0];
    fn_1_74FC(&state->actor, model);
}

/* Clears the queued motion and restores normal speed before player and prop behavior callbacks
 * run. */
void fn_1_61F4(M675ActorState *state)
{
    state->requestedMotion = -1;
    state->motionSpeed = 1.0f;
}

/* Behavior callbacks queue a motion and playback speed for the player or prop update to apply. */
void fn_1_6210(M675ActorState *state, s32 motionIndex, f32 speed)
{
    state->requestedMotion = motionIndex;
    state->motionSpeed = speed;
}

/* Called by fn_1_55F4 after phase logic to apply the queued player motion. */
void fn_1_621C(OMOBJ *object)
{
    M675PlayerWork *work = object->data;
    M675PlayerWork *state = work;

    if (state->actor.requestedMotion != -1) {
        fn_1_6284(object, state->actor.requestedMotion);
        Hu3DMotionSpeedSet(object->mdlId[0], state->actor.motionSpeed);
    }
}

/* Selects the requested character motion and its optional overlay. */
void fn_1_6284(OMOBJ *object, s32 motionIndex)
{
    M675PlayerWork *state;
    M675PlayerWork *work = object->data;
    u32 overlayMotionAttributes;
    u32 motionAttributes;
    s32 modelId, motionId, overlayIndex;
    float blend;
    state = work;
    overlayMotionAttributes = 0;
    modelId = object->mdlId[0];
    if (state->actor.motion != motionIndex && motionIndex < 11) {
        if (state->actor.motion < 0) {
            blend = 0.0f;
        } else {
            blend = 60.0f * lbl_1_data_124[motionIndex].blend;
        }
        state->actor.motion = motionIndex;
        state->actor.motionFrame = 0;
        motionId = object->mtnId[lbl_1_data_124[motionIndex].motionIndex];
        motionAttributes = lbl_1_data_124[motionIndex].attr;
        Hu3DModelAttrReset(modelId, HU3D_MOTATTR_LOOP | HU3D_MOTATTR_REV |
            HU3D_MOTATTR_OVL_LOOP | HU3D_MOTATTR_OVL_REV);
        overlayIndex = lbl_1_data_124[motionIndex].overlay;
        if (overlayIndex >= 0) {
            CharMotionShiftSet(work->character, motionId, 0.0f, blend, motionAttributes);
            Hu3DMotionOverlaySet(modelId, object->mtnId[overlayIndex]);
            if (motionAttributes & HU3D_MOTATTR_LOOP) {
                overlayMotionAttributes |= HU3D_MOTATTR_OVL_LOOP;
            }
            if (motionAttributes & HU3D_MOTATTR_REV) {
                overlayMotionAttributes |= HU3D_MOTATTR_OVL_REV;
            }
            Hu3DModelAttrSet(modelId, overlayMotionAttributes);
        } else {
            Hu3DMotionOverlayReset(modelId);
            CharMotionShiftSet(work->character, motionId, 0.0f, blend, motionAttributes);
        }
        CharMotionSpeedSet(work->character, 1.0f);
    }
}

/* Called by fn_1_5FCC to check whether the motion and blend have both ended. */
s32 fn_1_6460(OMOBJ *object)
{
    M675PlayerWork *work = object->data;
    if (CharMotionEndCheck(work->character) != 0 &&
        CharMotionShiftIDGet(work->character) < 0) {
        return 1;
    }
    return 0;
}

/* Checks whether the character motion blend has finished. */
s32 fn_1_64CC(OMOBJ *object)
{
    M675PlayerWork *work = object->data;
    if ((s16) CharMotionShiftIDGet((s16) work->character) < 0) {
        return 1;
    }
    return 0;
}

/* Chooses button-driven motions during play/finish updates and eases toward each motion's
 * formation offset. */
void fn_1_6524(M675PlayerWork *work)
{
    s32 firstMotion;
    s32 secondMotion;
    s32 activeMotion;
    s32 idleMotion;
    M675PlayerConfig *settings =
        &work->config;

    if (work->contactCooldown <= 0) {
        if (work->motionActive != 0 && work->actor.motion == settings->firstAlternateMotion) {
            goto movementUpdate;
        }
        if ((work->actor.buttons & PAD_BUTTON_B) != 0) {
            if (work->actor.motion == settings->secondAlternateMotion) {
                goto movementUpdate;
            }
            if (work->actor.motion != settings->firstAlternateMotion) {
                firstMotion = settings->firstAlternateMotion;
                work->actor.requestedMotion = firstMotion;
                work->actor.motionSpeed = 1.0f;
                work->motionActive = 1;
                goto movementUpdate;
            }
            secondMotion = settings->secondAlternateMotion;
            work->actor.requestedMotion = secondMotion;
            work->actor.motionSpeed = 1.0f;
            goto movementUpdate;
        }
        if ((work->actor.buttonsDown & PAD_BUTTON_A) != 0) {
            fn_1_D494(work->actor.playerMappingIndex, 0);
            work->actor.motion = settings->defaultMotion;
            activeMotion = settings->actionMotion;
            work->actor.requestedMotion = activeMotion;
            work->actor.motionSpeed = 1.0f;
            work->motionActive = 1;
        }
        if (work->motionActive == 0) {
            idleMotion = settings->idleMotion;
            work->actor.requestedMotion = idleMotion;
            work->actor.motionSpeed = 1.0f;
        }
    }
movementUpdate:
    if (work->actor.active != 0) {
        Vec destination;
        f32 adjustedAngle;

        PSVECAdd(&work->actor.basePosition, &lbl_1_data_2A8[work->actor.motion], &destination);
        fn_1_4BB4(&work->actor.position, &destination, 0.30000001192092896f,
            &work->actor.position);
        adjustedAngle = work->actor.baseYaw + lbl_1_data_32C[work->actor.motion];
        work->actor.rotation.y =
            fn_1_47DC(work->actor.rotation.y, adjustedAngle, 0.30000001192092896f);
    }
}

/* Handles player contacts and hit feedback; the hit sound's initial pan uses a wrapper
 * that discards the calculated pan. */
s32 fn_1_66EC(M675Collision *callback, M675Collision *event)
{
    void *callbackContext[1];
    OMOBJ *object;
    M675PlayerWork *state;
    M675PlayerWork *work;
    M675PlayerWork *voiceOwner;
    M675PlayerWork *voiceState;
    s32 handle;

    if (event->shapeType == 0 && event->handled == 0) {
        object = callback->owner;
        work = object->data;
        state = work;
        callbackContext[0] = callback;
        event->handled = 1;
        if (event->type == 5) {
            state->contactEvent = 1;
            if (state->actor.active != 0) {
                omVibrate((s16) work->playerIndex, 20, 20, 0);
                voiceOwner = object->data;
                voiceState = voiceOwner;
                handle = CharFXPlayPan((s16) voiceOwner->character, 581,
                    (s16) fn_1_32EC(&voiceState->actor.position));
                fn_1_4424(581, handle, &voiceState->actor.position, 1, 0);
            }
        }
    }
    return 1;
}

/* Called by fn_1_55F4 to disable collision during a hit, then restore contact. */
void fn_1_67CC(M675PlayerWork *state)
{
    M675PlayerConfig *transition = &state->config;
    s32 targetMotion;
    s32 defaultMotion;
    if (state->contactEvent != 0) {
        if (state->contactPhase == 0) {
            state->contactPhase = 1;
        }
        state->contactEvent = 0;
    }
    switch (state->contactPhase) {
    case 0:
        break;
    case 1:
        state->contactPhase++;
        state->actor.collision.flags &= ~1;
        state->contactCooldown = 90;
        targetMotion = transition->contactMotion;
        state->actor.requestedMotion = targetMotion;
        state->actor.motionSpeed = 1.0f;
    case 2:
        if (state->contactCooldown > 0) {
            break;
        }
        state->contactPhase = 0;
        state->actor.collision.flags |= 1;
        defaultMotion = transition->defaultMotion;
        state->actor.requestedMotion = defaultMotion;
        state->actor.motionSpeed = 1.0f;
    }
}

/* Steers a player toward a formation point and reports when it has arrived. */
s32 fn_1_68B0(M675PlayerWork *work, Vec *target, f32 targetAngle,
    f32 speed)
{
    f32 closeMotionSpeed;
    f32 farMotionSpeed;
    f32 deltaX;
    f32 deltaZ;
    f32 distance;
    f32 angle;
    f32 movementSpeed;
    f32 desiredVelocityX;
    f32 desiredVelocityZ;
    M675PlayerConfig *settings = &work->config;
    s32 closeMotion;
    s32 slowMotion;
    s32 farMotion;

    work->movementInProgress = 1;
    closeMotionSpeed = settings->slowMotionSpeed;
    farMotionSpeed = settings->fastMotionSpeed;
    closeMotion = settings->defaultMotion;
    slowMotion = settings->slowMotion;
    farMotion = settings->fastMotion;
    deltaX = target->x - work->actor.position.x;
    deltaZ = target->z - work->actor.position.z;
    distance = sqrtf(deltaX * deltaX + deltaZ * deltaZ);

    if (distance < 120.0) {
        speed *= 0.6;
        if (distance < 70.0) {
            speed *= 0.3;
        }
    }

    if (distance < 50.0) {
        if (work->motionActive == 0) {
            if (distance < 15.0) {
                work->actor.requestedMotion = closeMotion;
                work->actor.motionSpeed = 1.0f;
            } else {
                work->actor.requestedMotion = slowMotion;
                work->actor.motionSpeed =
                (f32)(0.5 * closeMotionSpeed);
            }
        }
        work->actor.position.x = (f32)(work->actor.position.x +
            (0.1 * deltaX));
        work->actor.position.z = (f32)(work->actor.position.z +
            (0.1 * deltaZ));
        work->actor.velocity.x *= 0.8;
        work->actor.velocity.z *= 0.8;
        work->actor.rotation.y =
            fn_1_47DC(work->actor.rotation.y, targetAngle, 0.20000000298023224f);
        if (distance > 30.0) {
            return 0;
        }
        /* Only positive signed yaw errors above four degrees delay arrival here;
         * negative errors pass this test. */
        if (fn_1_48F8(work->actor.rotation.y, targetAngle) > 4.0) {
            return 0;
        }
        if (PSVECMag(&work->actor.velocity) > 2.0) {
            return 0;
        }
        if (work->motionActive == 0) {
            work->actor.requestedMotion = closeMotion;
            work->actor.motionSpeed = 1.0f;
        }
        work->actor.velocity.x = work->actor.velocity.z = 0.0f;
        work->actor.rotation.y = targetAngle;
        work->movementInProgress = 0;
        return 1;
    }

    if (work->motionActive == 0) {
        work->actor.requestedMotion = slowMotion;
        work->actor.motionSpeed =
            (f32)(closeMotionSpeed * (f32)speed);
    }
    angle = (f32)(180.0 *
        (atan2((f64)deltaX, (f64)deltaZ) / 3.141592653589793));
    work->actor.rotation.y = fn_1_47DC(work->actor.rotation.y, angle, 0.30000001192092896f);
    movementSpeed = (f32)speed * settings->movementScale;
    desiredVelocityX =
        (f32)(movementSpeed *
            sin((3.141592653589793 * angle) / 180.0));
    desiredVelocityZ =
        (f32)(movementSpeed *
            cos((3.141592653589793 * angle) / 180.0));
    work->actor.velocity.x = (f32)(work->actor.velocity.x +
        (0.3 * (f64)(desiredVelocityX - work->actor.velocity.x)));
    work->actor.velocity.z = (f32)(work->actor.velocity.z +
        (0.3 * (f64)(desiredVelocityZ - work->actor.velocity.z)));
    if (work->motionActive == 0) {
        if (speed < (work->actor.motion == farMotion ?
            0.45 : 0.5)) {
            work->actor.requestedMotion = slowMotion;
            work->actor.motionSpeed =
                (f32)(speed * closeMotionSpeed);
        } else {
            work->actor.requestedMotion = farMotion;
            work->actor.motionSpeed =
                (f32)(speed * farMotionSpeed);
        }
    }
    return 0;
}

M675SequenceDifficultySettings lbl_1_data_3EC[5] = {
    {0.15f, 0.1f, 0.02f, 3},
    {0.3f, 0.2f, 0.05f, 5},
    {0.8f, 0.4f, 0.3f, 12},
    {1.0f, 0.5f, 0.5f, 15},
    {0.0f, 0.0f, 0.0f, 0}
};

/* Called by fn_1_5998 to time computer-player buttons from difficulty/progress. */
void fn_1_6E78(M675PlayerWork *work)
{
    M675SequenceDifficultySettings *settings =
        &lbl_1_data_3EC[work->difficulty];
    f32 firstScale;
    f32 secondScale;
    f32 thirdScale;
    f32 progressLimit;
    f32 progress;
    f32 timerRatio;
    f32 firstSpread;
    f32 secondSpread;
    f32 thirdSpread;
    Vec *effect;

    /* This timer ratio consumes fn_1_A60's result even though that wrapper discards
     * the sequence timer value. */
    timerRatio = (f32)fn_1_A60() / 900.0f;
    progressLimit = (f32)((s32)((f64)(f32)settings->baseCount *
        (1.0 - (f64)timerRatio)) + 1);
    progress = (f32)fn_1_F058(work->actor.playerMappingIndex);
    firstScale = settings->baseTimingAndHeightScale;
    secondScale = settings->randomTimingScale;
    thirdScale = settings->randomHeightScale;
    if (progress < progressLimit - 2.0f) {
        firstScale *= 1.5;
        if (firstScale > 1.0) {
            firstScale = 1.0f;
        }
        secondScale *= 2.0;
        if (secondScale > 1.0) {
            secondScale = 1.0f;
        }
        thirdScale *= 2.0;
        if (thirdScale > 1.0) {
            thirdScale = 1.0f;
        }
    }
    if (progress >= progressLimit) {
        firstScale *= 0.6;
        secondScale *= 0.3;
        thirdScale *= 0.2;
    }
    firstSpread = (f32)(1.0 - (f64)firstScale);
    secondSpread = (f32)(1.0 - (f64)secondScale);
    thirdSpread = (f32)(1.0 - (f64)thirdScale);

    switch (work->computerPhase) {
    case 0:
        work->computerPhase++;
        work->actionDelay = (s32)(5.0 +
            (10.0 * (f64)firstSpread) +
            (10.0 * (f64)secondSpread *
                (1.52587890625e-05 * (f64)(f32)(s32)(frand() & 0xFFFF))));
        work->randomDelay = (s32)(10.0 *
            (1.52587890625e-05 * (f64)(f32)(s32)(frand() & 0xFFFF))) + 10;
        work->triggerHeight = 600.0f;
        /* fall through */
    case 1:
        effect = fn_1_D550(2, work->actor.playerMappingIndex);
        if (effect != NULL && effect->y < work->triggerHeight) {
            work->computerPhase++;
            work->actor.buttonsDown |= PAD_BUTTON_B;
            work->actor.buttons |= PAD_BUTTON_B;
        } else if (--work->actionDelay <= 0) {
            work->actor.buttonsDown |= PAD_BUTTON_A;
            work->actionDelay = (s32)(4.5 +
                (5.0 * (f64)firstSpread) +
                (8.0 * (f64)secondSpread *
                (1.52587890625e-05 * (f64)(f32)(s32)(frand() & 0xFFFF))));
        }
        break;
    case 2:
        effect = fn_1_D550(2, work->actor.playerMappingIndex);
        if (effect != NULL) {
            work->actor.buttons |= PAD_BUTTON_B;
        } else {
            work->computerPhase--;
        }
        break;
    }

    if (--work->randomDelay <= 0) {
        work->randomDelay = (s32)(10.0 *
            (1.52587890625e-05 * (f64)(f32)(s32)(frand() & 0xFFFF))) + 10;
        work->triggerHeight = (f32)(100.0 *
            ((6.0 - (3.0 *
                (f64)firstSpread)) - (8.0 *
                    (f64)thirdSpread *
                    (1.52587890625e-05 * (f64)(f32)(s32)(frand() & 0xFFFF)))));
    }
}

/* Clears actor input before the player and prop behavior callbacks run each frame. */
void fn_1_74C8(M675ActorState *state) {
    state->buttonsDown = 0;
    state->buttons = 0;
    state->stickAngle = (f32) 0.0f;
    state->stickMagnitude = (f32) 0.0f;
}

/* Applies the actor transform during player collision updates and prop display setup. */
void fn_1_74FC(M675ActorState *state, s32 modelId) {
    Hu3DModelPosSetV((s16) modelId, &state->position);
    Hu3DModelRotSetV((s16) modelId, &state->rotation);
    state->motionFrame = (s32) (state->motionFrame + 1);
}

Vec lbl_1_data_43C = {0.0f, 180.0f, 0.0f};
