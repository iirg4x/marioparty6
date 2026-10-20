/* Player characters, movement, animation and target selection. */
#include "game/main.h"
#include "game/object.h"
#include "game/gamework.h"
#include "REL/m676Dll/player_types.h"
#include "dolphin/mtx.h"
#include "include/game/charman.h"
#include "include/game/memory.h"
#include "include/game/audio.h"
#include "include/game/gamework.h"
#include "include/game/pad.h"
#include "include/game/frand.h"
#include "include/string.h"
#include "dolphin/gx.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
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
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/arith.h"
#include <math.h>
#include "dolphin/types.h"

#define M676_PLAYER_COLLISION_MASK 0x19
#define M676_PLAYER_LANDING_SOUND_EFFECT 2231

void fn_1_1588(s32 playerIndex, s32 playerSlot, s32 character);

extern s32 fn_1_1498(void);
extern s32 fn_1_150C(u32 player);
extern s32 fn_1_3DD0(void *value);

extern s32 fn_1_A70(f32 x, f32 y, f32 z, f32 radius);
extern void fn_1_4F08(s32 effect, s32 handle, void *position, s32 mode,
    s32 flags);

extern s32 fn_1_FA68(Vec *position, f32 angle);
extern s32 fn_1_FBCC(s32 index);

extern s32 fn_1_FC30(s32 propIndex, Vec *position);
extern s32 fn_1_F940(s32 propIndex, Vec *position);
extern void fn_1_F9B0(s32 propIndex, s32 playerSlot, s32 character);
extern void fn_1_10240(s32 propIndex);
extern void fn_1_11A74(Vec *position, f32 radius, s32 count);
extern void fn_1_15C0(s32 playerIndex);
extern f32 fn_1_5490(Vec *start, Vec *end);
extern f32 fn_1_552C(Vec *start, Vec *end);
extern f32 fn_1_52C0(f32 start, f32 end, f32 amount);
extern s32 fn_1_56F0(Vec *start, Vec *end, f32 gravity, f32 initialY,
    Vec *velocity);
extern f32 fn_1_53DC(f32 start, f32 end);
extern void fn_1_4754(s32 resource, Vec *position);
extern s32 fn_1_FE28(s32 preferredRank);

extern s32 fn_1_13EC4(M676CollisionEntry *registration);

extern u32 MgSeqModeGet(void);
extern s32 fn_1_1630(s32 playerIndex);
extern s32 fn_1_1650(void);

extern s32 fn_1_F7DC(Vec *values);

void fn_1_5A10(HUPROCESS *manager);
void fn_1_5B2C(void);
void fn_1_5B50(OMOBJ *object);
void fn_1_5BA0(OMOBJ *object);
void fn_1_5C54(M676PlayerWork *work);
void fn_1_5C58(void);
void fn_1_5DC4(M676PlayerWork *state);
void fn_1_5ED8(OMOBJ *object);
void fn_1_62B8(OMOBJ *object);
s32 fn_1_6404(M676PlayerWork *state);
s32 fn_1_649C(M676PlayerWork *state);
s32 fn_1_67F8(M676PlayerWork *work);
s32 fn_1_6984(M676PlayerWork *state);
void fn_1_6A9C(OMOBJ *object);
void fn_1_6B98(OMOBJ *object, s32 effect);
void fn_1_6C0C(OMOBJ *object);
void fn_1_6CCC(OMOBJ *object);
void fn_1_6D34(OMOBJ *object, s32 index);
s32 fn_1_6F10(OMOBJ *object);
s32 fn_1_6F7C(OMOBJ *object);
void fn_1_6FD4(M676PlayerWork *work);
void fn_1_704C(M676PlayerWork *work);
void fn_1_767C(M676PlayerWork *work);
s32 fn_1_78B8(M676CollisionEntry *firstOwner, M676CollisionEntry *secondOwner);
s32 fn_1_7920(M676PlayerWork *work, const Vec *targetPosition,
    f32 targetAngle, f32 speed);
void fn_1_7EE8(M676PlayerWork *work);
void fn_1_871C(void);
s32 fn_1_8808(void);

u32 lbl_1_data_130[8] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 1),
    DATANUM(DATA_mariomot, 2), DATANUM(DATA_mariomot, 3),
    DATANUM(DATA_mariomot, 4), DATANUM(DATA_mariomot, 21),
    DATANUM(DATA_mariomot, 38), DATANUM(DATA_mariomot, 40)
};
M676MotionParam lbl_1_data_150[8] = {
    {0, -1, 0.2f, HU3D_MOTATTR_LOOP}, {1, -1, 0.2f, HU3D_MOTATTR_LOOP},
    {2, -1, 0.2f, HU3D_MOTATTR_LOOP}, {3, -1, 0.2f, HU3D_MOTATTR_NONE},
    {4, -1, 0.2f, HU3D_MOTATTR_NONE}, {5, -1, 0.2f, HU3D_MOTATTR_LOOP},
    {6, -1, 0.2f, HU3D_MOTATTR_NONE}, {7, -1, 0.2f, HU3D_MOTATTR_NONE}
};
Vec lbl_1_data_1B0[4] = {
    {-600.0f, 151.0f, 200.0f}, {-200.0f, 151.0f, 200.0f},
    {200.0f, 151.0f, 200.0f}, {600.0f, 151.0f, 200.0f}
};
f32 lbl_1_data_1E0[4] = {0.0f};
Vec lbl_1_data_1F0[4] = {
    {-450.0f, 151.0f, 5600.0f}, {-150.0f, 151.0f, 5600.0f},
    {150.0f, 151.0f, 5600.0f}, {450.0f, 151.0f, 5600.0f}
};
f32 lbl_1_data_220[7] = {0.0f};
M676PlayerConfig lbl_1_data_23C = {
    30.0f, 150.0f, 10.0f, 20.0f, 1.5f,
    0, 1, 1.8f, 2, 1.2f, 3, 4, 5, 6, 7
};

/* Creates the shared player manager and four player objects when this minigame starts. */
void fn_1_5A10(HUPROCESS *manager)
{
    OMOBJ *object;
    s32 i;

    lbl_1_bss_454 = 0;
    lbl_1_bss_450 = 0;
    lbl_1_bss_478 = omAddObjEx(manager, 14, 0, 0, -1, fn_1_5BA0);
    fn_1_5B50(lbl_1_bss_478);
    omMakeGroupEx(manager, 0, 4);
    for (i = 0; i < 4; i++) {
        object = lbl_1_bss_468[i] = omAddObjEx(manager, 10, 1, 8, 0, fn_1_5ED8);
        object->work[0] = i;
        object->work[1] = i;
        object->work[2] = i;
        fn_1_1588(i, i, GwPlayerConf[i].charNo);
    }
}

void fn_1_5B2C(void)
{
    CharModelKill(-1);
}

/* Clears the five slots used to track player work records during manager setup. */
void fn_1_5B50(OMOBJ *object) {
    s32 playerSlot;

    playerSlot = 0;
    while (playerSlot < 5) {
        lbl_1_bss_43C[playerSlot] = NULL;
        playerSlot += 1;
    }
    lbl_1_bss_438 = 0;
}

/* Ticks registered players and updates the order used by the player formations. */
void fn_1_5BA0(OMOBJ *object) {
    s32 playerSlot;
    M676PlayerWork *playerWork;

    if ((s32) lbl_1_bss_438 == 0) {
        lbl_1_bss_438 = 1;
    }
    playerSlot = 0;
    while (playerSlot < 5) {
        playerWork = lbl_1_bss_43C[playerSlot];
        if (playerWork != NULL) {
            fn_1_5C54(playerWork);
            if ((s32) playerWork->cooldown > 0) {
                playerWork->cooldown = (s32) (playerWork->cooldown - 1);
            }
            playerWork->frameCounter = (s32) (playerWork->frameCounter + 1);
        }
        playerSlot += 1;
    }
    fn_1_5C58();
}

void fn_1_5C54(M676PlayerWork *work)
{
}

/* Sorts players from left to right for forced prop selection and the finish formation. */
void fn_1_5C58(void)
{
    f32 positions[4];
    s32 pass;
    s32 index;
    s32 nextIndex;
    s32 savedIndex;
    s32 *indices = lbl_1_bss_458;
    f32 savedPosition;
    OMOBJ *object;
    M676PlayerWork *work;

    for (index = 0; index < 4; index++) {
        object = lbl_1_bss_468[index];
        work = object->data;
        positions[index] = work->actor.position.x;
        indices[index] = index;
    }
    for (pass = 0; pass < 4; pass++) {
        for (index = 0; index < 3; index++) {
            for (nextIndex = index + 1; nextIndex < 4; nextIndex++) {
                if (positions[index] > positions[nextIndex]) {
                    savedPosition = positions[index];
                    positions[index] = positions[nextIndex];
                    positions[nextIndex] = savedPosition;
                    savedIndex = indices[index];
                    indices[index] = indices[nextIndex];
                    indices[nextIndex] = savedIndex;
                }
            }
        }
    }
    for (index = 0; index < 4; index++) {
        object = lbl_1_bss_468[indices[index]];
        work = object->data;
        work->order = index;
    }
}

/* Registers a player's work record and initializes its starting transform. */
void fn_1_5DC4(M676PlayerWork *state)
{
    M676PlayerConfig *config = &state->config;
    s32 index;
    s32 player;

    for (index = 0; index < 5; index++) {
        if (lbl_1_bss_43C[index] == NULL) {
            lbl_1_bss_43C[index] = state;
            player = state->actor.player;
            state->actor.initialPosition = lbl_1_data_1B0[player];
            state->actor.position =
                state->actor.initialPosition;
            state->actor.velocity.x =
                state->actor.velocity.y =
                    state->actor.velocity.z = 0.0f;
            state->actor.rotationYSource = lbl_1_data_1E0[player];
            state->actor.rotation.y =
                state->actor.rotationYSource;
            state->motionFrame = 0;
            state->motionStatus = 0;
            state->motionScale = 1.0f;
            state->selectedPropIndex = -1;
            break;
        }
    }
}

/* Creates a character model, motions, and collision registration for a player object. */
void fn_1_5ED8(OMOBJ *object)
{
    M676PlayerWork *state;
    M676PlayerWork *work;
    M676CollisionEntry *callback;
    M676PlayerConfig *configDestination;
    s32 motionIndex;
    s32 playerIndex;
    s32 playerSlot;
    s32 playerValue;
    s32 characterModel;
    s32 character;
    s32 motionId;

    state = HuMemDirectMallocNum(HEAP_HEAP, 448, HU_MEMNUM_OVL);
    object->data = state;
    memset(state, 0, 448);
    object->stat |= 0x100;
    work = state;

    state->playerIndex = playerIndex = object->work[0];
    character = GwPlayerConf[playerIndex].charNo;
    state->character = character;
    playerSlot = work->actor.playerSlot = object->work[1];
    playerValue = object->work[2];
    work->actor.workValue = work->actor.player = playerValue;
    work->actor.active = 1;
    work->playerType = GwPlayerConf[playerIndex].type;
    work->difficulty = GwPlayerConf[playerIndex].comDif & 3;
    work->aiState = 0;

    characterModel = CharModelCreate((s16)character, 4);
    object->mdlId[0] = (s16)characterModel;
    state->modelId = characterModel;
    Hu3DModelLayerSet((s16)characterModel, 4);
    Hu3DModelShadowSet((s16)characterModel);
    CharEffectLayerSet(4);

    for (motionIndex = 0; motionIndex < 8; motionIndex++) {
        motionId = CharMotionCreate((s16)character,
            lbl_1_data_130[motionIndex]);
        object->mtnId[motionIndex] = motionId;
    }
    CharMotionDataClose((s16)character);
    work->actor.currentMotion = -1;
    fn_1_6D34(object, 0);

    configDestination = &work->config;
    *configDestination = lbl_1_data_23C;
    fn_1_5DC4(work);

    callback = &work->actor.collision;
    callback->header.flags = 1;
    callback->header.collisionMask = M676_PLAYER_COLLISION_MASK;
    callback->header.type = 0;
    callback->header.id = playerIndex;
    callback->header.state = 0;
    callback->header.callbackArgument = object;
    callback->header.updateCallback = fn_1_6C0C;
    callback->shape.dynamic.callback = 0;
    callback->shape.dynamic.position = &work->actor.position;
    callback->shape.dynamic.velocity = &work->actor.velocity;
    callback->shape.dynamic.currentPosition = *callback->shape.dynamic.position;
    callback->shape.dynamic.radius = 50.0f;
    callback->shape.dynamic.movable = 1.0f;
    fn_1_13EC4(callback);

    work->positionCopy = work->actor.position;
    work->motionReady = 1;
    work->motionSpeed = 55.0f;
    work->positionCopyPointer = &work->positionCopy;
    state->callbackState = 0;
    state->callbackActive = 1;
    object->objFunc = fn_1_62B8;
}

/* Advances one player through input, movement, and result phases on each object tick. */
void fn_1_62B8(OMOBJ *object)
{
    M676PlayerWork *state;
    M676PlayerWork *work = object->data;

    state = work;
    fn_1_8690(&state->actor);
    fn_1_6CA4(&state->actor);
    switch (work->callbackState) {
    case 0:
        if (fn_1_6404(state)) {
            state->actor.collision.shape.dynamic.callback = fn_1_78B8;
            work->callbackState++;
            state->phase = 0;
        }
        break;
    case 1:
        if (fn_1_649C(state)) { work->callbackState++; state->phase = 0; }
        break;
    case 2:
        if (fn_1_67F8(state)) { work->callbackState++; state->phase = 0; }
        break;
    case 3:
        if (fn_1_6984(state)) { work->callbackState++; state->phase = 0; }
        break;
    }
    fn_1_6CCC(object);
    PSVECAdd(&state->actor.position, &state->actor.velocity, &state->actor.position);
    fn_1_6A9C(object);
    state->ownerLink = 0;
}

/* Waits for the gameplay phase (mode 5) before allowing the player callback to proceed. */
s32 fn_1_6404(M676PlayerWork *state)
{
    M676PlayerConfig *config = &state->config;
    s32 result = 0;

    switch (state->phase) {
    case 0:
        state->phase++;
        state->timer = 0;
        break;
    case 1:
        if (MgSeqModeGet() != 5) {
            break;
        }
        state->phase++;
        /* fall through */
    default:
        result = 1;
        break;
    }
    fn_1_767C(state);
    return result;
}

/* Reads player input or AI during play, then waits through the turn transition. */
s32 fn_1_649C(M676PlayerWork *state)
{
    M676PlayerConfig *config = &state->config;
    s32 done = 0;
    s32 controllerIndex;
    M676PlayerWork *input;
    float x, y;

    switch (state->phase) {
    case 0:
        state->phase++;
        state->timer = 0;
        /* fall through */
    case 1:
        if (fn_1_1498() == 0) {
            break;
        }
        state->phase++;
        state->timer = 0;
        /* fall through */
    case 2:
        if (MgSeqModeGet() == 6) {
            state->phase++;
            state->timer = 0;
        } else if (state->playerType != 0) {
            fn_1_7EE8(state);
        } else if (state->actor.active != 0) {
            input = state;
            controllerIndex = GwPlayerConf[input->playerIndex].padNo;
            x = HuPadStkXf[controllerIndex];
            y = -HuPadStkYf[controllerIndex];
            state->actor.buttons = HuPadBtn[controllerIndex];
            state->actor.buttonsDown = HuPadBtnDown[controllerIndex];
            state->actor.stickAngle =
                180.0 * (atan2(x, y) / M_PI);
            state->actor.stickMagnitude = sqrtf(x * x + y * y);
        }
        break;
    case 3:
        if (state->turnState != 0) {
            break;
        }
        state->phase++;
        state->timer = 0;
        /* fall through */
    case 4:
        if (++state->timer < 10) {
            break;
        }
        state->phase++;
        state->timer = 0;
        /* fall through */
    default:
        done = 1;
        /* No value is returned on this path. */
        return;
    }

    fn_1_767C(state);
    fn_1_6FD4(state);
    fn_1_704C(state);
    return done;
}

/* Moves players into finish order and waits for the sequence to reach its result phase. */
s32 fn_1_67F8(M676PlayerWork *work)
{
    M676PlayerConfig *sequenceSettings = &work->config;
    const Vec *parameters;
    s32 done = 0;
    f32 targetYaw;
    M676PlayerWork *callbackWork;

    switch (work->phase) {
    case 0:
        work->phase++;
        work->timer = 0;
        work->motionActive = 0;
        /* fall through */
    case 1:
        if (MgSeqModeGet() != 6) {
            work->phase++;
            work->timer = 0;
            work->arrivalTimer = 0;
        } else {
            break;
        }
        /* fall through */
    case 2:
        parameters = &lbl_1_data_1F0[work->order];
        targetYaw = lbl_1_data_220[work->order];
        if (fn_1_7920(work, parameters, targetYaw, 0.8f) == 0 &&
            ++work->timer < 180) {
            break;
        }
        if (work->actor.active != 0) {
            callbackWork = work;
            if (++work->arrivalTimer < 30) {
                break;
            }
            fn_1_1630(callbackWork->playerIndex);
        }
        if (MgSeqModeGet() != 8 || fn_1_1650() == 0) {
            break;
        }
        work->phase++;
        work->timer = 0;
        /* fall through */
    default:
        done = 1;
        break;
    }
    return done;
}

/* Plays the win or loss motion and holds the result pose before finishing. */
s32 fn_1_6984(M676PlayerWork *state)
{
    M676PlayerConfig *config = &state->config;
    M676PlayerWork *work;
    s32 done = 0;

    switch (state->phase) {
    case 0:
        if (fn_1_150C(state->actor.playerSlot) > 0) {
            fn_1_6CC0(&state->actor, config->winMotion, 1.0f);
        } else {
            fn_1_6CC0(&state->actor, config->loseMotion, 1.0f);
        }
        if (state->actor.active) {
            work = state;
            work->callbackActive = 0;
            CharModelVoiceVolSet(work->character, 127);
        }
        state->phase++;
        state->timer = 0;
        break;
    case 1:
        if (++state->timer < 90) {
            break;
        }
        state->phase++;
        state->timer = 0;
    default:
        done = 1;
        break;
    }
    return done;
}

/* Updates motion completion, voice panning, and the player's two camera-fit spheres each frame. */
void fn_1_6A9C(OMOBJ *object)
{
    M676PlayerWork *work = object->data;
    M676PlayerWork *state = work;

    work->updateCount++;
    if (state->motionActive != 0) {
        if (fn_1_6F10(object) != 0) {
            state->motionActive = 0;
            state->motionTimer = 0;
        }
    } else {
        state->motionTimer = 0;
    }
    CharModelVoicePanSet((s16)work->character, (s16)fn_1_3DD0(&state->actor.position.x));
    fn_1_A70(state->actor.position.x, state->actor.position.y,
        state->actor.position.z - 50.0, 80.0f);
    fn_1_A70(state->actor.position.x, state->actor.position.y,
        300.0 + state->actor.position.z, 80.0f);
}

/* Plays a character effect at the player's screen-relative sound position. */
void fn_1_6B98(OMOBJ *object, s32 effect)
{
    M676PlayerWork *work = object->data;
    M676PlayerWork *owner = work;
    s32 handle = CharFXPlayPan((s16)work->character, (s16)effect,
                               (s16)fn_1_3DD0(&owner->actor.position));

    fn_1_4F08(effect, handle, &owner->actor.position, 1, 0);
}

/* Copies the player position with Y forced to 1 plus the initial player height, then updates the
 * model transform. */
void fn_1_6C0C(OMOBJ *object)
{
    M676PlayerWork *data;
    M676PlayerWork *record;
    s32 modelId;

    data = object->data;
    record = data;
    modelId = *object->mdlId;
    record->positionCopy = record->actor.position;
    record->positionCopy.y = 1.0 + lbl_1_data_1B0[0].y;
    fn_1_86C4(&record->actor, modelId);
}

void fn_1_6CA4(M676ActorState *state) {
    state->requestedMotion = -1;
    state->requestedMotionSpeed = 1.0f;
}

void fn_1_6CC0(M676ActorState *state, s32 status, f32 value) {
    state->requestedMotion = status;
    state->requestedMotionSpeed = value;
}

/* Applies a requested motion and speed from the player actor state. */
void fn_1_6CCC(OMOBJ *object)
{
    M676PlayerWork *work = object->data;
    M676PlayerWork *state = work;

    if (state->actor.requestedMotion != -1) {
        fn_1_6D34(object, state->actor.requestedMotion);
        Hu3DMotionSpeedSet(object->mdlId[0], state->actor.requestedMotionSpeed);
    }
}

/* Selects the requested character motion and applies any configured overlay. */
void fn_1_6D34(OMOBJ *object, s32 index)
{
    M676PlayerWork *state;
    M676PlayerWork *work = object->data;
    u32 overlayAttr;
    u32 attr;
    s32 model, motion, overlay;
    f32 blend;

    state = work;
    overlayAttr = 0;
    model = object->mdlId[0];
    if (state->actor.currentMotion != index && index < 8) {
        if (state->actor.currentMotion < 0) {
            blend = 0.0f;
        } else {
            blend = 60.0f * lbl_1_data_150[index].blend;
        }
        state->actor.currentMotion = index;
        state->actor.motionTime = 0;
        motion = object->mtnId[lbl_1_data_150[index].motion];
        attr = lbl_1_data_150[index].attr;
        Hu3DModelAttrReset(model, HU3D_MOTATTR | HU3D_MOTATTR_OVL_LOOP |
                                  HU3D_MOTATTR_OVL_REV | HU3D_ATTR_MOT_RESET_LOCK |
                                  HU3D_ATTR_SHADOW);
        overlay = lbl_1_data_150[index].overlay;
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

/* Reports when the current character motion has ended and no shift remains. */
s32 fn_1_6F10(OMOBJ *object)
{
    M676PlayerWork *work = object->data;
    if (CharMotionEndCheck(work->character) != 0 &&
        CharMotionShiftIDGet(work->character) < 0) {
        return 1;
    }
    return 0;
}

/* Reports whether the character motion blend has completed. */
s32 fn_1_6F7C(OMOBJ *object)
{
    M676PlayerWork *work = object->data;
    if ((s16) CharMotionShiftIDGet((s16) work->character) < 0) {
        return 1;
    }
    return 0;
}

/* Claims a nearby available prop when the player presses the action button. */
void fn_1_6FD4(M676PlayerWork *work)
{
    M676PlayerConfig *config;
    s32 candidate;

    config = &work->config;
    if (work->selectedPropIndex < 0 && (work->actor.buttonsDown & 0x100) != 0) {
        candidate = fn_1_FA68(&work->actor.position, work->actor.rotation.y);
        if (candidate >= 0) {
            work->selectedPropIndex = fn_1_FBCC(candidate);
        }
    }
}

/* Handles landing, bouncing, and settled states for the player's selected object. */
void fn_1_704C(M676PlayerWork *work)
{
    M676PlayerConfig *params = &work->config;
    M676PlayerWork *object = work;
    Vec targetPosition;
    s32 propOutOfRange;
    s32 motion;
    s32 jumpMotion;
    s32 landingMotion;
    s32 settledMotion;
    s32 timeoutMotion;

    if (work->actor.active == 0) {
        return;
    }

    switch (work->landingState) {
    case 0:
        if (work->selectedPropIndex < 0) {
            break;
        }
        work->landingState++;
        work->landingTimer = 0;
        work->actor.collision.shape.dynamic.movable = 0.0f;
        work->turnState = 1;
        fn_1_FC30(work->selectedPropIndex, &work->targetPosition);
        work->targetPosition.y = work->actor.position.y;
        work->targetAngle = fn_1_5490(&work->actor.position,
            &work->targetPosition);
        if (fn_1_552C(&work->actor.position, &work->targetPosition) <
            210.0) {
            work->landingState++;
            work->landingTimer = 0;
            break;
        }
        /* fall through */
    case 1:
        targetPosition = work->targetPosition;
        targetPosition.z = (f32)((f64)targetPosition.z - 180.0);
        if (fn_1_7920(work, &targetPosition,
                0.0f, 1.0f) == 0) {
            break;
        }
        work->turnState = 1;
        work->landingState++;
        work->landingTimer = 0;
        work->targetAngle = fn_1_5490(&work->actor.position,
            &work->targetPosition);
        /* fall through */
    case 2:
        work->actor.rotation.y = fn_1_52C0(work->actor.rotation.y, work->targetAngle,
            0.2f);
        if (fabs((double)fn_1_53DC(work->actor.rotation.y, work->targetAngle)) > 3.0) {
            break;
        }
        work->actor.rotation.y = work->targetAngle;
        work->landingState++;
        work->landingTimer = fn_1_56F0(&work->actor.position, &work->targetPosition,
            params->gravity, 1000.0f, &work->actor.velocity);
        work->actor.collision.header.flags &= ~1U;
        motion = params->jumpMotion;
        work->actor.requestedMotion = motion;
        work->actor.requestedMotionSpeed = 1.0f;
        /* fall through */
    case 3:
        work->actor.velocity.y -= params->gravity;
        if (work->landingTimer == 5) {
            fn_1_FC30(work->selectedPropIndex, &targetPosition);
            targetPosition.y -= 20.0;
            targetPosition.z += 30.0;
            fn_1_11A74(&targetPosition, 50.0f, 20);
            fn_1_4754(M676_PLAYER_LANDING_SOUND_EFFECT, &work->actor.position);
        }
        if (--work->landingTimer > 0) {
            break;
        }
        work->landingState++;
        work->landingTimer = 0;
        work->actor.position = work->targetPosition;
        work->actor.velocity.x = 0.0f;
        work->actor.velocity.y = 0.0f;
        work->actor.velocity.z = 0.0f;
        fn_1_F9B0(work->selectedPropIndex, work->actor.playerSlot, object->character);
        omVibrate((s16)work->actor.playerSlot, 20, 20, 0);
        work->motionReady = 0;
        Hu3DModelAttrSet((s16)object->modelId, 1U);
        fn_1_15C0(object->playerIndex);
        /* fall through */
    case 4:
    {
        propOutOfRange = fn_1_F940(work->selectedPropIndex, &targetPosition);

        work->actor.position.x = targetPosition.x;
        work->actor.position.z = targetPosition.z;
        if (propOutOfRange == 0) {
            break;
        }
        work->landingState++;
        work->landingTimer = 0;
        targetPosition.y -= 20.0;
        targetPosition.z += 30.0;
        fn_1_11A74(&targetPosition, 50.0f, 10);
        fn_1_4754(M676_PLAYER_LANDING_SOUND_EFFECT, &work->actor.position);
        /* fall through */
    }
    case 5:
        if (++work->landingTimer < 5) {
            break;
        }
        work->landingState++;
        work->landingTimer = 0;
        work->motionReady = 1;
        work->actor.collision.header.flags |= 1U;
        Hu3DModelAttrReset((s16)object->modelId, 1U);
        fn_1_10240(work->selectedPropIndex);
        fn_1_FC30(work->selectedPropIndex, &targetPosition);
        work->actor.position.x = targetPosition.x;
        work->actor.position.z = targetPosition.z;
        work->actor.rotation.y = 0.0f;
        work->actor.velocity.y = 30.0f;
        jumpMotion = params->jumpMotion;
        work->actor.requestedMotion = jumpMotion;
        work->actor.requestedMotionSpeed = 1.0f;
        omVibrate((s16)work->actor.playerSlot, 20, 20, 0);
        /* fall through */
    case 6:
        work->actor.velocity.y -= params->gravity;
        if (work->actor.velocity.y > 0.0 ||
            work->actor.position.y > lbl_1_data_1F0[0].y) {
            break;
        }
        work->landingState++;
        work->landingTimer = 0;
        work->actor.position.y = lbl_1_data_1F0[0].y;
        work->actor.velocity.x = 0.0f;
        work->actor.velocity.y = 0.0f;
        work->actor.velocity.z = 0.0f;
        landingMotion = params->landingMotion;
        work->actor.requestedMotion = landingMotion;
        work->actor.requestedMotionSpeed = 1.0f;
        /* fall through */
    case 7:
        if (++work->landingTimer < 12) {
            break;
        }
        work->landingState++;
        work->landingTimer = 0;
        settledMotion = params->idleMotion;
        work->actor.requestedMotion = settledMotion;
        work->actor.requestedMotionSpeed = 1.0f;
        work->turnState = 0;
        return;
    case 10:
        if (++work->landingTimer < 120) {
            break;
        }
        work->landingState++;
        work->landingTimer = 0;
        timeoutMotion = params->idleMotion;
        work->actor.requestedMotion = timeoutMotion;
        work->actor.requestedMotionSpeed = 1.0f;
        work->turnState = 0;
        break;
    }
}

/* Uses stick input to update movement and animation while no prop is selected and no scripted
 * turn is active. */
void fn_1_767C(M676PlayerWork *work)
{
    M676PlayerConfig *motion = &work->config;
    f32 inputMagnitude = work->actor.stickMagnitude * work->motionScale;
    f32 movement;
    f32 startSpeed;
    f32 endSpeed;
    f64 threshold;
    s32 idleMotion;
    s32 walkMotion;
    s32 runMotion;
    s32 motionWasActive;

    if (work->turnState == 0 && work->selectedPropIndex < 0) {
        startSpeed = motion->moveSpeed;
        endSpeed = motion->runSpeed;
        idleMotion = motion->idleMotion;
        walkMotion = motion->moveMotion;
        runMotion = motion->runMotion;

        if (walkMotion == -1) {
            if (runMotion == -1) {
                runMotion = idleMotion;
                walkMotion = runMotion;
            } else {
                walkMotion = runMotion;
            }
        } else if (runMotion == -1) {
            runMotion = walkMotion;
        }

        motionWasActive = work->motionActive;
        if (inputMagnitude >= 0.05f && work->cooldown == 0) {
            work->motionActive = 0;
            movement = inputMagnitude * motion->movementScale;
            work->actor.velocity.x =
                (f32) ((f64) movement *
                       sin((3.141592653589793 * (f64) work->actor.stickAngle) / 180.0));
            work->actor.velocity.z =
                (f32) ((f64) movement *
                       cos((3.141592653589793 * (f64) work->actor.stickAngle) / 180.0));
            if (work->motionTimer == 0) {
                work->actor.rotation.y =
                    fn_1_52C0(work->actor.rotation.y, work->actor.stickAngle, 0.5f);
            }
            if (motionWasActive == 0) {
                if (work->actor.currentMotion == runMotion) {
                    threshold = 0.45;
                } else {
                    threshold = 0.5;
                }
                if (inputMagnitude < threshold) {
                    work->actor.requestedMotion = walkMotion;
                    work->actor.requestedMotionSpeed = inputMagnitude * startSpeed;
                    return;
                }
                work->actor.requestedMotion = runMotion;
                work->actor.requestedMotionSpeed = inputMagnitude * endSpeed;
            }
        } else {
            if (motionWasActive == 0) {
                work->actor.requestedMotion = idleMotion;
                work->actor.requestedMotionSpeed = 1.0f;
            }
            work->actor.velocity.x = work->actor.velocity.z = 0.0f;
        }
    }
}

/* On player-to-player contact, stores each object's link in the other player's work record. */
s32 fn_1_78B8(M676CollisionEntry *firstOwner, M676CollisionEntry *secondOwner)
{
    OMOBJ *firstLink;
    OMOBJ *secondLink;
    M676PlayerWork *firstObject;
    M676PlayerWork *firstObjectTarget;
    M676PlayerWork *secondObject;
    M676PlayerWork *secondObjectTarget;

    if (secondOwner->header.state == 0 && secondOwner->header.type == 0) {
        firstLink = firstOwner->header.callbackArgument;
        firstObject = firstLink->data;
        firstObjectTarget = firstObject;
        secondLink = secondOwner->header.callbackArgument;
        secondObject = secondLink->data;
        secondObjectTarget = secondObject;
        firstObjectTarget->ownerLink = secondLink;
        secondObjectTarget->ownerLink = firstLink;
    }
    return 1;
}

/* Steers the player toward a position and final yaw, returning 1 on arrival; the final heading test
 * rejects only signed differences greater than +4 degrees. */
s32 fn_1_7920(M676PlayerWork *work, const Vec *targetPosition,
    f32 targetAngle, f32 speed)
{
    M676PlayerConfig *params = &work->config;
    f32 nearSpeed;
    f32 farSpeed;
    s32 nearMotion;
    s32 farMotion;
    s32 defaultMotion;
    f32 deltaX;
    f32 deltaZ;
    f32 distance;
    f32 angle;
    f32 movementSpeed;
    f32 desiredVelocityX;
    f32 desiredVelocityZ;

    work->turnState = 1;
    nearSpeed = params->moveSpeed;
    farSpeed = params->runSpeed;
    nearMotion = params->idleMotion;
    farMotion = params->moveMotion;
    defaultMotion = params->runMotion;
    deltaX = targetPosition->x - work->actor.position.x;
    deltaZ = targetPosition->z - work->actor.position.z;
    distance = sqrtf((deltaX * deltaX) + (deltaZ * deltaZ));
    if (distance < 120.0) {
        speed *= 0.8;
        if (distance < 70.0) {
            speed *= 0.5;
        }
    }
    if (distance < 50.0) {
        if (work->motionActive == 0) {
            if (distance < 15.0) {
                work->actor.requestedMotion = nearMotion;
                work->actor.requestedMotionSpeed = 1.0f;
            } else {
                work->actor.requestedMotion = farMotion;
                work->actor.requestedMotionSpeed = (f32)(0.5 * (f64)nearSpeed);
            }
        }
        work->actor.position.x = (f32)((f64)work->actor.position.x +
            (0.2 * (f64)deltaX));
        work->actor.position.z = (f32)((f64)work->actor.position.z +
            (0.2 * (f64)deltaZ));
        work->actor.velocity.x *= 0.8;
        work->actor.velocity.z *= 0.8;
        work->actor.rotation.y = fn_1_52C0(work->actor.rotation.y, targetAngle,
            0.2f);
        if (distance > 30.0) {
            return 0;
        }
        if (fn_1_53DC(work->actor.rotation.y, targetAngle) > 4.0) {
            return 0;
        }
        if (PSVECMag(&work->actor.velocity) > 2.0) {
            return 0;
        }
        if (work->motionActive == 0) {
            work->actor.requestedMotion = nearMotion;
            work->actor.requestedMotionSpeed = 1.0f;
        }
        work->actor.velocity.x = work->actor.velocity.z = 0.0f;
        work->actor.rotation.y = targetAngle;
        work->turnState = 0;
        return 1;
    }
    if (work->motionActive == 0) {
        work->actor.requestedMotion = farMotion;
        work->actor.requestedMotionSpeed = (f32)(nearSpeed * (f32)speed);
    }
    angle = (f32)(180.0 * (atan2((f64)deltaX, (f64)deltaZ) /
        3.141592653589793));
    work->actor.rotation.y = fn_1_52C0(work->actor.rotation.y, angle, 0.3f);
    movementSpeed = (f32)speed * params->movementScale;
    desiredVelocityX = (f32)(movementSpeed * sin(
        (3.141592653589793 * (f64)angle) / 180.0));
    desiredVelocityZ = (f32)(movementSpeed * cos(
        (3.141592653589793 * (f64)angle) / 180.0));
    work->actor.velocity.x = (f32)((f64)work->actor.velocity.x +
        (0.3 * (f64)(desiredVelocityX - work->actor.velocity.x)));
    work->actor.velocity.z = (f32)((f64)work->actor.velocity.z +
        (0.3 * (f64)(desiredVelocityZ - work->actor.velocity.z)));
    if (work->motionActive == 0) {
        if (speed < (work->actor.currentMotion == defaultMotion ? 0.45 : 0.5)) {
            work->actor.requestedMotion = farMotion;
            work->actor.requestedMotionSpeed = (f32)(speed * nearSpeed);
        } else {
            work->actor.requestedMotion = defaultMotion;
            work->actor.requestedMotionSpeed = (f32)(speed * farSpeed);
        }
    }
    return 0;
}

/* Chooses and performs the computer player's action during the input phase. */
void fn_1_7EE8(M676PlayerWork *work)
{
    M676WanderEntry * entry = & lbl_1_data_2A4[work->difficulty];
    Vec targetPosition;
    f32 steeringMeasure;
    f64 initialTurnSpeed;
    f64 turnAngle;
    f32 turnAngleFloat;
    f32 targetX;
    f32 targetZ;
    s32 turn;
    s32 finished;
    switch (work->aiState)
    {
    case 0 :
        work->aiState++;
        work->aiWait =
            (s32) (60.0 + ((1.52587890625e-5 * (f64) (f32) (s32) (frand() & 0xFFFF)) * 60.0));
        work->elapsed = 0;
        work->turnCooldown = 0;
        work->selectionTimer =
            (s32) (180.0 + ((60.0 * (2.0 * (1.0 - (f64) entry->selectionTimingFactor))) +
                            (60.0 * (1.52587890625e-5 * (f64) (f32) (s32)(frand() & 0xFFFF)))));
        work->heading = (f32)(360.0 * (1.52587890625e-5 * (f64)(f32)(s32)(frand() & 0xFFFF)));
        if (! (! ((s32)(2.0 * (1.52587890625e-5 * (f64)(f32)(s32)(frand() & 0xFFFF))) == 0)))
        {
            initialTurnSpeed = 1.1;
        }
        else
        {
            initialTurnSpeed = - 1.1;
        }
        work->turnSpeed = (f32) initialTurnSpeed;
        work->movementSpeed = 0.7f;
    case 1 :
        finished = 0;
        if (work->turnCooldown <= 0 && work->ownerLink != NULL)
        {
            work->turnSpeed = - work->turnSpeed;
            if (! (! (work->turnSpeed > 0.0)))
            {
                turnAngle = 90.0;
            }
            else
            {
                turnAngle = - 90.0;
            }
            steeringMeasure = (f32) turnAngle;
            work->heading = (f32) ((f64) work->heading +
                                   (steeringMeasure +
                                    (40.0 * (3.0517578125e-5 *
                                             (f64) (f32) ((s32) (frand() & 0xFFFF) - 32768)))));
            work->turnCooldown = 12;
        }
        if (work->ownerLink != NULL)
        {
            work->heading = (f32)((f64) work->heading + (5.0 * (f64) work->turnSpeed));
        }
        work->heading = fmodf(work->heading + work->turnSpeed, 360.0f);
        if (work->heading < 0.0f)
        {
            work->heading += 360.0f;
        }
        targetPosition = lbl_1_data_2C4;
        targetPosition.x =
            (f32) ((f64) targetPosition.x +
                   (100.0 * (8.0 * sin((3.141592653589793 * (f64) work->heading) / 180.0))));
        targetPosition.z = (f32) ((f64) targetPosition.z +
                                  (100.0 * cos((3.141592653589793 * (f64) work->heading) / 180.0)));
        steeringMeasure = fn_1_552C(& work->actor.position, & targetPosition);
        if (steeringMeasure > 100.0)
        {
            work->movementSpeed = 0.7f;
        }
        if (steeringMeasure < 70.0)
        {
            work->movementSpeed = 0.5f;
        }
        work->actor.stickMagnitude = work->movementSpeed;
        work->actor.stickAngle = fn_1_5490(& work->actor.position, & targetPosition);
        if (work->turnCooldown > 0)
        {
            work->actor.stickAngle += 8.0;
        }
        if (work->elapsed > work->aiWait)
        {
            if (--work->selectionTimer < 0)
            {
                finished = 1;
            }
            if (fn_1_8808() == 0)
            {
                finished = 1;
            }
        }
        if (finished)
        {
            work->aiState++;
            work->targetPropIndex = fn_1_FE28(entry->preferredRank);
        }
        else
        {
            break;
        }
    case 2 :
        if (work->selectedPropIndex < 0)
        {
            if (0 >= work->turnCooldown && work->ownerLink != NULL)
            {
                /* Truncation normally leaves the target slot unchanged; only the minimum random
                 * value subtracts one. */
                work->targetPropIndex +=
                    (s32) (3.0517578125e-5 * (f64) (f32) ((s32)(frand() & 0xFFFF) - 32768));
                if (work->targetPropIndex < 0)
                {
                    work->targetPropIndex = 4;
                }
                if (work->targetPropIndex >= 5)
                {
                    work->targetPropIndex = 0;
                }
                work->turnCooldown = 18;
            }
            turn = 0;
            while (turn < 5)
            {
                if (fn_1_FC30(work->targetPropIndex, & targetPosition) == 0)
                {
                    break;
                }
                if (++work->targetPropIndex >= 5)
                {
                    work->targetPropIndex = 0;
                }
                turn += 1;
            }
            if (fn_1_552C(& work->actor.position, & targetPosition) > 300.0)
            {
                targetPosition.z -= 300.0;
            }
            work->actor.stickAngle = fn_1_5490(& work->actor.position, & targetPosition);
            work->actor.stickMagnitude = 0.8f;
            work->actor.buttonsDown |= 0x100U;
            if (work->turnCooldown > 0)
            {
                work->actor.stickAngle += 8.0;
            }
        }
        break;
    }
    work->elapsed++;
    if (work->turnCooldown > 0)
    {
        work->turnCooldown--;
    }
}

/* Clears per-frame controller input before fn_1_62B8 processes the player. */
void fn_1_8690(M676ActorState *state) {
    state->buttonsDown = 0;
    state->buttons = 0;
    state->stickAngle = 0.0f;
    state->stickMagnitude = 0.0f;
}

/* Copies an actor's transform to its model and advances its motion frame. */
void fn_1_86C4(M676ActorState *record, s32 model_id) {
    Hu3DModelPosSetV((s16)model_id, &record->position);
    Hu3DModelRotSetV((s16)model_id, &record->rotation);
    record->motionTime += 1;
}

/* Selects a nearby available object for each player that has none assigned. */
void fn_1_871C(void)
{
    s32 slot;

    for (slot = 0; slot < 4; slot++) {
        OMOBJ *object = lbl_1_bss_468[lbl_1_bss_458[slot]];
        M676PlayerWork *work = object->data;
        M676PlayerWork *selectionWork = work;

        if (selectionWork->selectedPropIndex < 0) {
            s32 candidateBase = fn_1_F7DC(&selectionWork->actor.position);
            s32 offset;

            for (offset = 0; offset < 10; offset++) {
                s32 candidate = (offset + 1) >> 1;

                if ((offset & 1) != 0) {
                    candidate = -candidate;
                }
                candidate = candidateBase + candidate;
                if (candidate >= 0 && candidate < 5 && fn_1_FC30(candidate, 0) == 0) {
                    selectionWork->selectedPropIndex = fn_1_FBCC(candidate);
                    break;
                }
            }
        }
    }
}

/* Counts human players who have not selected an object yet. */
s32 fn_1_8808(void)
{
    OMOBJ *object;
    M676PlayerWork *work;
    M676PlayerWork *status;
    s32 count = 0;
    s32 index;

    for (index = 0; index < 4; index++) {
        object = lbl_1_bss_468[index];
        work = object->data;
        status = work;
        if (status->playerType == 0 && status->selectedPropIndex < 0) {
            count++;
        }
    }
    return count;
}

/* Requests controller rumble for the supplied player slot. */
void fn_1_8888(s32 playerIndex) {
    s32 unused;
    omVibrate((s16) playerIndex, 20, 20, 0);
}

M676WanderEntry lbl_1_data_2A4[4] = {
    {0.3f, 4}, {0.6f, 3}, {0.8f, 2}, {1.0f, 1}
};
Vec lbl_1_data_2C4 = {0.0f, 151.0f, 200.0f};

OMOBJ *lbl_1_bss_478;
OMOBJ *lbl_1_bss_468[4];
s32 lbl_1_bss_458[4];
s32 lbl_1_bss_454;
s32 lbl_1_bss_450;
M676PlayerWork *lbl_1_bss_43C[5];
s32 lbl_1_bss_438;
