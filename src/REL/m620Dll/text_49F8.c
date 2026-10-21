/* Trap Ease Artist camera interpolation and player actor behavior. */
#include "dolphin/types.h"

#include "dolphin/math.h"

#include "dolphin/pad.h"

#include "REL/m620Dll/module_types.h"

#include "game/main.h"

#include "game/object.h"

#include "game/gamework.h"

#include "game/memory.h"

#include "game/mg/seqman.h"

#include "game/hu3d.h"

#include "game/charman.h"

#include "game/hsfex.h"

#include "game/frand.h"

/* Per-player state updated by the actor callback and its phase helpers. */
typedef struct M620PlayerWork {
    /* The actor helpers do not read these leading bytes. */
    u8 unreadBytesBeforePlayerId[64];
    s32 playerId;
    s32 playerIndex;
    s32 spawnIndex;
    s32 controlEnabled;
    /* Position and velocity are in world units; velocity is added each actor frame. */
    Vec position;
    Vec velocity;
    Vec rotation;
    /* The actor helpers do not read these bytes before the input fields. */
    u8 unreadBytesBeforeButtons[4];
    /* Button masks sampled or synthesized for the current actor update. */
    u32 buttonsDown;
    u32 buttons;
    /* Stick heading in degrees and magnitude normalized to the range 0 through 1. */
    f32 stickAngle;
    f32 stickMagnitude;
    /* Motion-list slot, elapsed actor frames, and a pending motion (-1 means none). */
    s32 motionIndex;
    s32 motionFrame;
    s32 motion;
    /* Multiplier applied when the pending motion is selected. */
    f32 motionSpeed;
    s32 sequencePhase;
    s32 sequenceTimer;
    u8 unreadBytesBeforeUpdateCount[16];
    s32 updateCount;
    s32 computer;
    u32 difficulty;
    s32 computerActionPhase;
    s32 eventTimer;
    s32 motionActive;
    s32 motionTimer;
    u8 unreadBytesBeforeInitialized[8];
    s32 initialized;
    s32 transitionValue;
    M620PlayerConfig config;
    u32 player;
    s32 character;
    s32 objectPhase;
    u8 unreadBytesBeforeActorUpdateCount[4];
    s32 actorUpdateCount;
    s32 volumeEnabled;
} M620PlayerWork;

typedef struct M620AudioRecordView {
    s32 handle;
    s32 effect;
    u32 cameraMask;
    s32 panEnabled;
    s32 volumeEnabled;
    Vec *position;
} M620AudioRecordView;

typedef struct M620AudioStateView {
    void *userData;
    OMOBJ_FUNC callback;
    M620AudioRecordView records[64];
} M620AudioStateView;

extern OMOBJ *lbl_1_bss_2F0;

void fn_1_42B4(u32 cameras, s32 effect, s32 handle, Vec *position, s32 panEnabled,
               s32 volumeEnabled);

void fn_1_4268(s32 effect, s32 handle, Vec *position, s32 panEnabled, s32 volumeEnabled);

extern int lbl_1_bss_1B0[80];

extern int lbl_1_bss_70[80];

void fn_1_432C(void);

int fn_1_4384(int dataNum);

int fn_1_4408(int dataNum);

/* Find an existing registration for this data number, or store it in the first free slot. */
static inline int m620_local_4510(int dataNum)
{
    int index;
    int existing;
    if (dataNum != -1) {
        for (index = 0; index < 80; index++) {
            existing = lbl_1_bss_1B0[index];
            if (dataNum == existing) return index;
            if (existing == -1) {
                lbl_1_bss_1B0[index] = dataNum;
                return index;
            }
        }
    }
    return -1;
}

int fn_1_4510(int model, int dataNum);

float fn_1_4620(float start, float end, float factor);

f32 fn_1_473C(f32 start, f32 end);

float fn_1_47F0(HuVecF *start, HuVecF *end);

float fn_1_488C(Vec *first, Vec *second);

/* Interpolate each component between camera-view vectors for fn_1_B58. */
void fn_1_49F8(Vec *start, Vec *end, f32 amount, Vec *result)
{
    result->x = start->x + amount * (end->x - start->x);
    result->y = start->y + amount * (end->y - start->y);
    result->z = start->z + amount * (end->z - start->z);
}

#include <string.h>

void fn_1_502C(OMOBJ *object);

extern OMOBJ *lbl_1_bss_330;

extern OMOBJ *lbl_1_bss_320[4];

void fn_1_4BD0(OMOBJ *object);

void fn_1_4B70(OMOBJ *object);

void fn_1_4D68(OMOBJ *object);

void fn_1_10D4(s32 player, s32 character, s32 charNo);

/* Called by fn_1_A0 to create the actor manager and four player objects. */
void fn_1_4A50(HUPROCESS *manager)
{
    OMOBJ *object;
    s32 i;

    lbl_1_bss_330 = omAddObjEx(manager, 14, 0, 0, -1, fn_1_4BD0);
    fn_1_4B70(lbl_1_bss_330);
    omMakeGroupEx(manager, 0, 4);
    for (i = 0; i < 4; i++) {
        object = lbl_1_bss_320[i] = omAddObjEx(manager, 10, 1, 6, 0, fn_1_4D68);
        object->work[0] = i;
        object->work[1] = i;
        object->work[2] = i;
        fn_1_10D4(i, i, GwPlayerConf[i].charNo);
    }
}

void fn_1_4B4C(void)
{
    CharModelKill(-1);
}

extern s32 lbl_1_bss_308;

extern s32 lbl_1_bss_30C;

extern void *lbl_1_bss_310[4];

/* Reset the shared player slots and setup counters during fn_1_4A50. */
void fn_1_4B70(OMOBJ *object)
{
    s32 index;

    index = 0;
    while (index < 4) {
        lbl_1_bss_310[index] = NULL;
        index += 1;
    }
    lbl_1_bss_308 = 0;
    lbl_1_bss_30C = 0;
}

extern Vec lbl_1_data_130[4];

void fn_1_4C98(void *state);

/* The manager callback advances registered players and its shared countdown. */
void fn_1_4BD0(OMOBJ *object)
{
    M620PlayerWork *state;
    s32 index;

    if (lbl_1_bss_30C == 0) {
        lbl_1_bss_30C = 1;
    }
    index = 0;
    while (index < 4) {
        state = lbl_1_bss_310[index];
        if (state != NULL) {
            fn_1_4C98(state);
            state->updateCount++;
        }
        index++;
    }
    if (lbl_1_bss_308 > 0) {
        lbl_1_bss_308--;
    }
}

void fn_1_4C98(void *state)
{
}

/* Register a player during fn_1_4D68 and seed its spawn position and heading. */
void fn_1_4C9C(M620PlayerWork *state)
{
    void *configuration;
    s32 index;
    s32 playerNumber;

    configuration = &state->config;
    for (index = 0; index < 4; index++) {
        if (lbl_1_bss_310[index] == NULL) {
            lbl_1_bss_310[index] = state;
            playerNumber = state->spawnIndex;
            state->position = lbl_1_data_130[playerNumber];
            state->velocity.x = state->velocity.y = state->velocity.z = 0.0f;
            state->rotation.y = lbl_1_data_160[playerNumber];
            return;
        }
    }
}

void fn_1_5D54(OMOBJ *object, s32 index);

/* Register a player state and initialize its spawn transform when a slot is free. */
static inline void m620_initialize_player_state(M620PlayerWork *state)
{
    void *playerConfigAddress;
    s32 index;
    s32 player;

    playerConfigAddress = &state->config;
    for (index = 0; index < 4; index++) {
        if (lbl_1_bss_310[index] == NULL) {
            lbl_1_bss_310[index] = state;
            player = state->spawnIndex;
            state->position = lbl_1_data_130[player];
            state->velocity.x = state->velocity.y = state->velocity.z = 0.0f;
            state->rotation.y = lbl_1_data_160[player];
            break;
        }
    }
}

/* The object manager calls this to allocate and initialize each player actor. */
void fn_1_4D68(OMOBJ *object)
{
    M620PlayerWork *state;
    M620PlayerWork *work;
    s32 i, player, character, model;
    M620PlayerConfig *config;
    s32 index;
    u32 playerId;
    s32 motion;

    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M620PlayerWork), HU_MEMNUM_OVL);
    object->data = work;
    memset(work, 0, sizeof(M620PlayerWork));
    object->stat |= OM_STAT_MODELPAUSE;
    state = work;
    player = work->player = object->work[0];
    character = work->character = GwPlayerConf[player].charNo;
    state->playerId = playerId = object->work[1];
    state->spawnIndex = index = object->work[2];
    if (playerId == 0) {
        state->playerIndex = 0;
    } else {
        state->playerIndex = index - 1;
    }
    state->controlEnabled = 1;
    state->computer = GwPlayerConf[player].type;
    state->difficulty = GwPlayerConf[player].comDif & 3;
    state->computerActionPhase = 0;
    model = CharModelCreate(character, 4);
    object->mdlId[0] = model;
    Hu3DModelLayerSet(model, 4);
    Hu3DModelShadowSet(model);
    CharEffectLayerSet(4);
    for (i = 0; i < 6; i++) {
        motion = CharMotionCreate(character, lbl_1_data_D0[i]);
        object->mtnId[i] = motion;
    }
    CharMotionDataClose(character);
    state->motionIndex = -1;
    fn_1_5D54(object, 2);
    config = &state->config;
    *config = lbl_1_data_17C;
    fn_1_4C9C(state);
    work->objectPhase = 0;
    work->volumeEnabled = 1;
    object->objFunc = fn_1_502C;
}

void fn_1_6314(M620PlayerWork *actor);

void fn_1_5CC4(M620PlayerWork *actor);

s32 fn_1_516C(M620PlayerWork *work);

s32 fn_1_52CC(M620PlayerWork *work);

s32 fn_1_5724(M620PlayerWork *work);

s32 fn_1_5A50(M620PlayerWork *state);

void fn_1_5CEC(OMOBJ *object);

void fn_1_5B60(OMOBJ *object);

void fn_1_5C68(OMOBJ *object);

/* Per-frame actor callback: advance the phase, motion, position, and model. */
void fn_1_502C(OMOBJ *object)
{
    M620PlayerWork *state;
    M620PlayerWork *work = object->data;

    state = work;
    fn_1_6314(state);
    fn_1_5CC4(state);
    switch (work->objectPhase) {
    case 0:
        if (fn_1_516C(state)) { work->objectPhase++; state->sequencePhase = 0; }
        break;
    case 1:
        if (fn_1_52CC(state)) { work->objectPhase++; state->sequencePhase = 0; }
        break;
    case 2:
        if (fn_1_5724(state)) { work->objectPhase++; state->sequencePhase = 0; }
        break;
    case 3:
        if (fn_1_5A50(state)) { work->objectPhase++; state->sequencePhase = 0; }
        break;
    }
    fn_1_5CEC(object);
    PSVECAdd(&state->position, &state->velocity, &state->position);
    fn_1_5B60(object);
    fn_1_5C68(object);
}

void fn_1_B48C(s32 player);

s32 fn_1_FC0(void);

void fn_1_5CE0(M620PlayerWork *work, s32 motion, f32 speed);

s32 fn_1_110C(s32 actionId);

s32 fn_1_112C(void);

/* Called by fn_1_502C to run the actor's opening sequence phases. */
s32 fn_1_516C(M620PlayerWork *work)
{
    M620PlayerConfig *resourceWindow = &work->config;
    s32 result = 0;

    switch (work->sequencePhase) {
    case 0:
        work->sequencePhase++;
        work->sequenceTimer = 0;
        /* fall through */
    case 1:
        if (++work->sequenceTimer >= 60) {
            work->sequencePhase++;
            work->sequenceTimer = 0;
            if (work->controlEnabled != 0) {
                M620PlayerWork *transitionWork = work;
                if (transitionWork->player == 0) {
                    fn_1_B48C(transitionWork->player);
                    fn_1_5CE0(work, resourceWindow->nextResourceId,
                              1.0f);
                }
            }
        } else {
            break;
        }
        /* fall through */
    case 2:
        if (fn_1_FC0() == 0) {
            break;
        }
        work->sequencePhase++;
        work->sequenceTimer = 0;
        if (work->controlEnabled != 0) {
            M620PlayerWork *transitionWork = work;
            if (transitionWork->player == 0) {
                fn_1_B48C(transitionWork->player);
                fn_1_5CE0(work, resourceWindow->resourceId,
                          1.0f);
            }
        }
        /* fall through */
    case 3:
        if (MgSeqModeGet() != 5) {
            break;
        }
        work->sequencePhase++;
        work->sequenceTimer = 0;
        /* fall through */
    default:
        result = 1;
        break;
    }
    return result;
}

extern s8 HuPadStkX[4];

extern s8 HuPadStkY[4];

extern u16 HuPadBtn[4];

extern u16 HuPadBtnDown[4];

void fn_1_6094(M620PlayerWork *work);

void fn_1_5FFC(M620PlayerWork *work);

/* Called by fn_1_502C to read human input or advance the computer player's turn. */
s32 fn_1_52CC(M620PlayerWork *work)
{
    M620PlayerConfig *resources = &work->config;
    s32 done = 0;

    switch (work->sequencePhase) {
    case 0:
        work->sequencePhase++;
        work->sequenceTimer = 0;
        /* fall through */
    case 1:
        if (MgSeqModeGet() == 6) {
            work->sequencePhase++;
            work->sequenceTimer = 0;
            /* Force the actor's pending A-button action when the finish phase begins. */
            work->buttonsDown = PAD_BUTTON_A;
            break;
        }
        if (work->computer != 0) {
            fn_1_6094(work);
            break;
        }
        if (work->controlEnabled == 0) {
            break;
        }
        {
            M620PlayerWork *playerWork = work;
            s32 pad = GwPlayerConf[playerWork->player].padNo;
            f32 stickX = (f32)HuPadStkX[pad];
            f32 stickY = (f32)-HuPadStkY[pad];
            f32 angle;
            f32 wrappedAngle;
            f32 finalAngle;

            work->buttons = HuPadBtn[pad];
            work->buttonsDown = HuPadBtnDown[pad];
            work->stickAngle = (f32)(180.0 *
                (atan2((f64)stickX, (f64)stickY) / 3.141592653589793));
            work->stickMagnitude = 0.017857144f *
                sqrtf(stickX * stickX + stickY * stickY);
            if (work->stickMagnitude > 1.0f) {
                work->stickMagnitude = 1.0f;
            }
            angle = work->stickAngle + CRotM[0].y;
            wrappedAngle = (f32)fmod((f64)angle, 360.0);
            finalAngle = wrappedAngle;
            work->stickAngle = finalAngle;
        }
        break;
    case 2:
        if (MgSeqModeGet() == 5) {
            work->velocity.x *= 0.98;
            work->velocity.z *= 0.98;
            break;
        }
        work->sequencePhase++;
        work->sequenceTimer = 0;
        work->transitionValue = 6000;
        /* fall through */
    case 3:
        if (++work->sequenceTimer < 10) {
            break;
        }
        work->sequencePhase++;
        work->sequenceTimer = 0;
        fn_1_5CE0(work, resources->firstResourceId, 1.0f);
    default:
        done = 1;
        break;
    }

    fn_1_5FFC(work);
    return done;
}

/* Called by fn_1_502C to settle the actor's movement and wait for the action. */
s32 fn_1_5724(M620PlayerWork *work)
{
    M620PlayerConfig *resources = &work->config;
    s32 result = 0;
    f32 targetRotation;
    f32 rotationStep;

    switch (work->sequencePhase) {
    case 0:
        work->sequencePhase++;
        work->sequenceTimer = 0;
        work->motionActive = 0;
        fn_1_5CE0(work, resources->firstResourceId, 1.0f);
        /* fall through */
    case 1:
        work->velocity.x *= 0.98;
        work->velocity.z *= 0.98;
        if (MgSeqModeGet() == 6) {
            break;
        }
        work->sequencePhase++;
        work->sequenceTimer = 0;
        work->velocity.x = work->velocity.z = 0.0f;
        /* fall through */
    case 2:
        work->sequenceTimer++;
        if (work->position.z > 0.0) {
            if (work->sequenceTimer < 10) {
                rotationStep = 1.4f;
                work->position.x += work->position.x < 0.0 ?
                    rotationStep : -rotationStep;
                work->position.z -= rotationStep;
            }
            targetRotation = work->position.x < 0.0 ? 15.0 : 345.0;
        } else {
            targetRotation = 0.0f;
        }
        work->rotation.y = fn_1_4620(work->rotation.y, targetRotation, 0.1f);
        if (fabs(fn_1_473C(work->rotation.y, targetRotation)) > 3.0) {
            fn_1_5CE0(work, resources->secondResourceId, 1.0f);
        } else {
            fn_1_5CE0(work, resources->firstResourceId, 1.0f);
        }
        if (work->controlEnabled != 0) {
            M620PlayerWork *actionWork = work;
            if (work->sequenceTimer > 60) {
                fn_1_110C(actionWork->player);
            }
        }
        if (MgSeqModeGet() == 8 && fn_1_112C() != 0) {
            work->sequencePhase++;
            work->sequenceTimer = 0;
            fn_1_5CE0(work, resources->firstResourceId, 1.0f);
        } else {
            break;
        }
        /* fall through */
    default:
        result = 1;
        break;
    }
    return result;
}

void fn_1_5D54(OMOBJ *object, s32 index);

s32 fn_1_5F30(OMOBJ *object);

extern s32 fn_1_32EC(void *voiceData);

extern s32 fn_1_3130(void *voiceData);

extern s32 fn_1_3350(void *voiceData);

extern void fn_1_4268(s32 effect, s32 handle, Vec *position, s32 panEnabled, s32 volumeEnabled);

void fn_1_6348(M620PlayerWork *actor, s32 modelId);

s32 fn_1_1018(u32 playerId);

/* Called by fn_1_502C to play the result motion and finish after its delay. */
s32 fn_1_5A50(M620PlayerWork *state)
{
    M620PlayerConfig *config =
        &state->config;
    M620PlayerWork *work;
    s32 done = 0;
    s32 useWinMotion = 0;

    switch (state->sequencePhase) {
    case 0:
        if (fn_1_1018(state->playerId) > 0) {
            useWinMotion = 1;
            fn_1_5CE0(state, config->winMotion, 1.0f);
        } else {
            fn_1_5CE0(state, config->loseMotion, 1.0f);
        }
        if (state->controlEnabled) {
            work = state;
            work->volumeEnabled = 0;
            CharModelVoiceVolSet(work->character, 127);
        }
        state->sequencePhase++;
        state->sequenceTimer = 0;
        break;
    case 1:
        if (++state->sequenceTimer < 90) {
            break;
        }
        state->sequencePhase++;
        state->sequenceTimer = 0;
    default:
        done = 1;
        break;
    }
    return done;
}

/* Called each actor frame by fn_1_502C to update motion-completion state. */
void fn_1_5B60(OMOBJ *object)
{
    M620PlayerWork *state;
    M620PlayerWork *work = object->data;

    state = work;
    work->actorUpdateCount++;
    if (state->motionActive) {
        if (fn_1_5F30(object) != 0) {
            state->motionActive = 0;
            state->motionTimer = 0;
        }
    } else {
        state->motionTimer = 0;
    }
}

/* Play a character effect and track its handle at the actor's world position. */
void fn_1_5BE4(OMOBJ *object, s32 effect)
{
    M620PlayerWork *work = object->data;
    M620PlayerWork *owner = work;
    s32 handle = CharFXPlayVolPan((s16)work->character, (s16)effect,
                                  (s16)fn_1_3350(&owner->position),
                                  (s16)fn_1_3130(&owner->position));

    fn_1_4268(effect, handle, &owner->position, 1, 1);
}

/* Called by fn_1_502C to copy the actor transform onto its character model. */
void fn_1_5C68(OMOBJ *object)
{
    M620PlayerWork *work;
    M620PlayerWork *state;
    s32 model;

    work = object->data;
    state = work;
    model = object->mdlId[0];
    fn_1_6348(state, model);
}

/* Called by fn_1_502C to clear last frame's motion request before phase updates. */
void fn_1_5CC4(M620PlayerWork *actor)
{
    actor->motion = -1;
    actor->motionSpeed = 1.0f;
}

/* Queue a motion and playback speed for fn_1_5CEC to apply during the actor update. */
void fn_1_5CE0(M620PlayerWork *work, s32 motion, f32 speed)
{
    work->motion = motion;
    work->motionSpeed = speed;
}

/* Called by fn_1_502C to apply any pending character motion and playback speed. */
void fn_1_5CEC(OMOBJ *object)
{
    M620PlayerWork *work = object->data;
    M620PlayerWork *state = work;

    if (state->motion != -1) {
        fn_1_5D54(object, state->motion);
        Hu3DMotionSpeedSet(object->mdlId[0], state->motionSpeed);
    }
}

typedef struct M620MotionParam {
    u16 motion;
    s16 overlay;
    float blend;
    u32 attr;
} M620MotionParam;

extern M620MotionParam lbl_1_data_E8[6];

/* Called by fn_1_5CEC to change motion only when the requested index differs. */
void fn_1_5D54(OMOBJ *object, s32 index)
{
    M620PlayerWork *state;
    M620PlayerWork *work = object->data;
    u32 overlayAttr;
    u32 attr;
    s32 model, motion, overlay;
    float blend;

    state = work;
    overlayAttr = 0;
    model = object->mdlId[0];
    if (state->motionIndex != index && index < 6) {
        if (state->motionIndex < 0) {
            blend = 0.0f;
        } else {
            blend = 60.0f * lbl_1_data_E8[index].blend;
        }
        state->motionIndex = index;
        state->motionFrame = 0;
        motion = object->mtnId[lbl_1_data_E8[index].motion];
        attr = lbl_1_data_E8[index].attr;
        Hu3DModelAttrReset(model, HU3D_MOTATTR_LOOP | HU3D_MOTATTR_REV |
                                   HU3D_MOTATTR_OVL_LOOP | HU3D_MOTATTR_OVL_REV);
        overlay = lbl_1_data_E8[index].overlay;
        if (overlay >= 0) {
            CharMotionShiftSet(work->character, motion, 0.0f, blend, attr);
            Hu3DMotionOverlaySet(model, object->mtnId[overlay]);
            if (attr & HU3D_MOTATTR_LOOP) { overlayAttr |= HU3D_MOTATTR_OVL_LOOP; }
            if (attr & HU3D_MOTATTR_REV) { overlayAttr |= HU3D_MOTATTR_OVL_REV; }
            Hu3DModelAttrSet(model, overlayAttr);
        } else {
            Hu3DMotionOverlayReset(model);
            CharMotionShiftSet(work->character, motion, 0.0f, blend, attr);
        }
        CharMotionSpeedSet(work->character, 1.0f);
    }
}

/* Called by fn_1_5B60 to detect an ended motion after its blend finishes. */
s32 fn_1_5F30(OMOBJ *object)
{
    M620PlayerWork *work = object->data;

    if (CharMotionEndCheck(work->character) != 0 &&
        CharMotionShiftIDGet(work->character) < 0) {
        return 1;
    }
    return 0;
}

/* Report whether the character's motion blend has completed. */
s32 fn_1_5F9C(OMOBJ *object)
{
    M620PlayerWork *work = object->data;
    s32 done = 0;

    if (CharMotionShiftIDGet(work->character) < 0) {
        done = 1;
    }
    return done;
}

extern void fn_1_B48C(s32 owner);

/* Called by fn_1_52CC to start the next motion on the actor's first action. */
void fn_1_5FFC(M620PlayerWork *work)
{
    M620PlayerConfig *state;
    M620PlayerWork *owner;
    s32 motionIndex;

    state = &work->config;
    if (work->initialized == 0 && (work->buttonsDown & PAD_BUTTON_A) != 0) {
        if (work->controlEnabled != 0) {
            owner = work;
            fn_1_B48C(owner->player);
        }
        motionIndex = state->nextResourceId;
        work->motion = motionIndex;
        work->motionSpeed = 1.0f;
        work->initialized = 1;
    }
}

typedef struct M620EventProfile {
    s32 initialTimer;
    s32 timerRange;
    f32 advanceChance;
    s32 nearbyObjectLimit;
} M620EventProfile;

extern M620EventProfile lbl_1_data_1AC[];

extern s32 fn_1_B590(s32 objectIndex);

extern s32 fn_1_FE0(void);

/* Called from fn_1_52CC to choose and time computer-player actions. */
void fn_1_6094(M620PlayerWork *work)
{
    M620EventProfile *profile = &lbl_1_data_1AC[work->difficulty];
    s32 advance = 0;

    switch (work->computerActionPhase) {
    case 0:
        work->computerActionPhase++;
        work->eventTimer = profile->initialTimer;
        /* fall through */
    case 1:
        if (work->eventTimer > 0) {
            break;
        }
        work->computerActionPhase++;
        work->eventTimer = (s32)(((f64)(f32)(s32)(frand() & 0xFFFF) *
                             1.52587890625e-05) *
                            profile->timerRange);
        /* fall through */
    case 2:
        if (fn_1_B590(work->playerId) > profile->nearbyObjectLimit) {
            advance = 1;
        } else if (work->eventTimer < 0 &&
                   (f64)(f32)(s32)(frand() & 0xFFFF) *
                       1.52587890625e-05 < profile->advanceChance) {
            advance = 1;
        }
        if (work->difficulty != 0 && fn_1_FE0() < 60) {
            advance = 1;
        }
        if (advance == 0) {
            break;
        }
        work->computerActionPhase++;
        /* fall through */
    case 3:
        if (lbl_1_bss_308 == 0 && (s32)(frand() & 1) == 0) {
            work->buttonsDown |= PAD_BUTTON_A;
            work->computerActionPhase++;
            lbl_1_bss_308 = (s32)(18.0 +
                ((f64)(f32)(s32)(frand() & 0xFFFF) * 1.52587890625e-05) *
                    60.0);
        }
        break;
    }
    work->eventTimer--;
}

/* Called at the start of fn_1_502C to clear input values for this update. */
void fn_1_6314(M620PlayerWork *actor) {
    actor->buttonsDown = 0;
    actor->buttons = 0;
    actor->stickAngle = 0.0f;
    actor->stickMagnitude = 0.0f;
}

/* Called by fn_1_5C68 to update model transforms and advance the motion frame. */
void fn_1_6348(M620PlayerWork *actor, s32 modelId) {
    Hu3DModelPosSetV((HU3D_MODELID) modelId, &actor->position);
    Hu3DModelRotSetV((HU3D_MODELID) modelId, &actor->rotation);
    actor->motionFrame = actor->motionFrame + 1;
}

/* Called by launch effects to get a player's character hook position. */
void fn_1_63A0(s32 playerIndex, HuVecF *position)
{
    OMOBJ *object;
    M620PlayerWork *work;
    char *hookName;
    s32 modelId;

    object = lbl_1_bss_320[playerIndex & 3];
    work = object->data;
    modelId = object->mdlId[0];
    hookName = CharModelItemHookGet((s16)work->character, 4, 0);
    Hu3DModelObjPosGet(modelId, hookName, position);
}

/* Called by launch effects to rumble one player or all players during mode 2. */
void fn_1_6434(s32 player)
{
    if (MgSeqModeGet() == 2) {
        for (player = 0; player < 4; player++) {
            omVibrate((s16) player, 20, 20, 0);
        }
    } else {
        omVibrate((s16) player, 20, 20, 0);
    }
}
