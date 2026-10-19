/* Player creation, input, character motion, collisions and computer steering. */
#include "dolphin/math.h"
#include "game/charman.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/pad.h"
#include "game/frand.h"
#include "msm_se.h"
#include "datadir_enum.h"
#include "REL/m674Dll/player.h"
#include "REL/m674Dll/effects.h"
#include "REL/m674Dll/collision.h"
#include "REL/m674Dll/scene.h"
#include <string.h>

#define MSM_SE_M674_GROUND_IMPACT 2221

/* Shared result-slot, scene, score, and collision helpers used by player objects. */
extern void fn_1_EE4(s32 slotIndex, u32 scoreIndex, u32 characterNo);
extern s32 fn_1_E014(void);
extern s32 fn_1_E18(s32 resource);
extern s32 fn_1_E20(void);
extern u32 fn_1_E68(u32 scoreIndex);
extern void fn_1_F1C(s32 player);
extern s32 fn_1_F3C(void);
extern s32 fn_1_4994(void);
extern s32 fn_1_4040(s32 resource, void *position);
extern void fn_1_47F4(s32 effect, s32 handle, void *position, s32 pan, s32 volume);
extern s32 fn_1_36BC(void *position);
extern f32 fn_1_4BAC(f32 start, f32 end, f32 factor);
extern f32 fn_1_4CC8(f32 start, f32 end);
extern f32 fn_1_4D7C(Vec *start, Vec *end);
extern s32 fn_1_100F8(s32 player);
extern s32 fn_1_11218(M674Entry *entry);
extern M674Entry *fn_1_11444(s32 type, s32 index);
extern M674ActorState *fn_1_DC2C(Vec *position, f32 heading, f32 range, f32 distance);

void fn_1_52FC(HUPROCESS *manager);
void fn_1_53F8(void);
void fn_1_541C(OMOBJ *object);
void fn_1_547C(OMOBJ *object);
void fn_1_554C(M674ActorState *work);
void fn_1_55CC(M674ActorState *state);
void fn_1_55D0(M674ActorState *state);
void fn_1_56D4(OMOBJ *object);
void fn_1_5AB0(OMOBJ *object);
s32 fn_1_5C04(M674PlayerWork *object);
s32 fn_1_5C98(M674PlayerWork *state);
s32 fn_1_5FC4(M674PlayerWork *work);
s32 fn_1_6170(M674PlayerWork *state);
void fn_1_6288(OMOBJ *object);
void fn_1_6324(OMOBJ *object, s32 effect);
void fn_1_6398(OMOBJ *object);
void fn_1_67B0(M674ActorState *actor);
void fn_1_67CC(M674ActorState *actor, s32 motion, f32 speed);
void fn_1_67D8(OMOBJ *object);
void fn_1_6840(OMOBJ *object, s32 index);
s32 fn_1_6A1C(OMOBJ *object);
s32 fn_1_6A88(OMOBJ *object);
void fn_1_6AE0(M674PlayerWork *work);
void fn_1_6DC0(M674PlayerWork *work);
s32 fn_1_7660(M674CollisionParam *first, M674CollisionParam *second);
void fn_1_77F0(M674PlayerWork *work);
void fn_1_7958(M674PlayerWork *work);
void fn_1_7DB0(M674PlayerWork *work);
void fn_1_7E28(M674PlayerWork *work);
void fn_1_83C0(M674PlayerWork *work);
void fn_1_8744(M674PlayerWork *work);
void fn_1_8748(M674PlayerWork *work, M674PlayerControl callback);
s32 fn_1_8778(M674PlayerWork *work);
void fn_1_8894(M674ActorState *work);
void fn_1_88C8(M674ActorState *work, s32 modelId);
s32 fn_1_8A44(void);

u32 lbl_1_data_D8[8] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 1),
    DATANUM(DATA_mariomot, 2), DATANUM(DATA_mariomot, 3),
    DATANUM(DATA_mariomot, 4), DATANUM(DATA_mario, 33),
    DATANUM(DATA_mariomot, 38), DATANUM(DATA_mariomot, 7)
};
M674PlayerMotionParam lbl_1_data_F8[8] = {
    {0,-1,0.2f,HU3D_MOTATTR_LOOP}, {1,-1,0.2f,HU3D_MOTATTR_LOOP},
    {2,-1,0.2f,HU3D_MOTATTR_LOOP}, {3,-1,0.2f,0},
    {4,-1,0.2f,0}, {5,-1,0.2f,HU3D_MOTATTR_LOOP},
    {6,-1,0.2f,0}, {7,-1,0.2f,0}
};
Vec lbl_1_data_158[4] = {{-570,80,-340},{570,80,-340},{-500,80,480},{500,80,480}};
f32 lbl_1_data_188[4] = {0,0,0,0};
Vec lbl_1_data_198[4] = {{-360,80,-650},{-130,80,-650},{130,80,-650},{360,80,-650}};
f32 lbl_1_data_1C8[4] = {0,0,0,0};
Vec lbl_1_data_1D8 = {0,0,0};
M674PlayerConfig lbl_1_data_1E4 = {
    50,200,10,20,-1.5f,1,0.5f,1.25f,1,
    0,1,1.8f,2,1.2f,5,5,1.8f,5,1.2f,3,4,6,7
};
M674ComputerParam lbl_1_data_240[4] = {
    {0.5f,0,3},{0.6f,0,7},{0.7f,1,10},{0.8f,1,14}
};

/* Create the shared player manager and four player objects during minigame setup. */
void fn_1_52FC(HUPROCESS *manager)
{
    s32 i;
    OMOBJ *object;
    lbl_1_bss_30C = omAddObjEx(manager, 14, 0, 0, -1, fn_1_547C);
    fn_1_541C(lbl_1_bss_30C);
    omMakeGroupEx(manager, 0, 4);
    for (i = 0; i < 4; i++) {
        object = lbl_1_bss_2FC[i] = omAddObjEx(manager, 10, 2, 8, 0, fn_1_56D4);
        object->work[0] = i;
        object->work[1] = i;
        object->work[2] = i;
        fn_1_EE4(i, i, GwPlayerConf[i].charNo);
    }
}

void fn_1_53F8(void)
{
    CharModelKill(-1);
}

/* Reset the shared actor slots and counters after the player manager is created. */
void fn_1_541C(OMOBJ *object) {
    s32 index;

    index = 0;
    while (index < 5) {
        lbl_1_bss_2E8[index] = NULL;
        index += 1;
    }
    lbl_1_bss_2E4 = 0;
    lbl_1_bss_2E0 = 0;
}

/* The manager callback updates each registered actor once per object-manager pass. */
void fn_1_547C(OMOBJ *object)
{
    s32 index;
    M674ActorState *state;
    if (lbl_1_bss_2E4 == 0) {
        lbl_1_bss_2E4 = 1;
    }
    index = 0;
    while (index < 5) {
        state = lbl_1_bss_2E8[index];
        if (state != NULL) {
            fn_1_554C(state);
            fn_1_55CC(state);
            if (lbl_1_bss_2E0 == 0 && fn_1_E014() == 0) {
                lbl_1_bss_2E0 = 1;
            }
            state->registeredFrames = state->registeredFrames + 1;
        }
        index += 1;
    }
}

/* Cache the selected play-region edges the first time a registered actor is updated. */
void fn_1_554C(M674ActorState *work)
{
    M674Entry *region;
    if (work->regionInitialized == 0) {
        region = fn_1_11444(2, 0);
        if (region != NULL) {
            work->radialOffsetX = region->shape.region.center.x + region->shape.region.innerRadiusX;
            work->radialOffsetZ = region->shape.region.center.z + region->shape.region.innerRadiusZ;
            work->regionInitialized = 1;
        }
    }
}

void fn_1_55CC(M674ActorState *state)
{
}

/* Register a character in an open shared slot and initialize its starting movement state. */
void fn_1_55D0(M674ActorState *state)
{
    void *stateContext[1];
    s32 index;
    s32 player;

    stateContext[0] = (u8 *)state + 372;
    for (index = 0; index < 5; index++) {
        if (lbl_1_bss_2E8[index] == NULL) {
            lbl_1_bss_2E8[index] = state;
            player = state->playerNo;
            state->position = lbl_1_data_158[player];
            state->velocity.x =
                state->velocity.y =
                    state->velocity.z = (0.0f);
            state->rotation.y = lbl_1_data_188[player];
            state->blendState = 0;
            state->blendFrame = 0;
            state->blendAmount = (1.0f);
            state->blendValue = (1.0f);
            state->cooldown = 0;
            break;
        }
    }
}

/* The player object's initializer builds its character, motions, collision entry and shadow. */
void fn_1_56D4(OMOBJ *object)
{
    M674PlayerWork *work;
    M674PlayerWork *state;
    M674Entry *task;
    M674PlayerConfig *visualData;
    s32 motionIndex;
    s32 playerIndex;
    s32 playerNo;
    s32 characterNo;
    s32 scoreIndex;
    s32 modelId;
    s32 motionId;

    work = HuMemDirectMallocNum(HEAP_HEAP, 488, HU_MEMNUM_OVL);
    object->data = work;
    memset(work, 0, 488);
    object->stat = (u16)(object->stat | 0x100);

    state = work;
    playerIndex = work->playerIndex = object->work[0];
    characterNo = work->characterNo = GwPlayerConf[playerIndex].charNo;
    scoreIndex = state->state.scoreIndex = object->work[1];
    playerNo = object->work[2];
    state->state.previousPlayer = state->state.playerNo = playerNo;
    state->state.controlEnabled = 1;
    state->state.computer = GwPlayerConf[playerIndex].type;
    state->state.difficulty = GwPlayerConf[playerIndex].comDif & 3;
    state->state.controlPhase = 0;

    modelId = CharModelCreate(characterNo, 8);
    object->mdlId[0] = modelId;
    Hu3DModelLayerSet(modelId, 3);
    Hu3DModelShadowSet(modelId);
    CharEffectLayerSet(3);

    for (motionIndex = 0; motionIndex < 8; motionIndex++) {
        motionId = CharMotionCreate(characterNo, lbl_1_data_D8[motionIndex]);
        object->mtnId[motionIndex] = motionId;
    }
    CharMotionDataClose(characterNo);

    state->state.motionIndex = -1;
    fn_1_6840(object, 0);
    visualData = &state->state.config;
    *visualData = lbl_1_data_1E4;
    visualData->characterHeight = CharModelHeightGet(characterNo);
    fn_1_55D0(&state->state);

    state->state.spareState = 5;
    task = &state->state.task;
    task->flags = 1;
    task->mask = 5;
    task->type = 0;
    task->id = playerIndex;
    task->state = 0;
    task->primaryRelation = object;
    task->invoke = fn_1_6398;
    task->shape.actor.callback = 0;
    task->shape.actor.position = &state->state.position;
    task->shape.actor.velocity = &state->state.velocity;
    task->shape.actor.initialPosition = *task->shape.actor.position;
    task->shape.actor.radius = 50.0f;
    task->shape.actor.collisionFlag = 1.0f;
    fn_1_11218(task);
    /* Discard the resource-number result here, then call the model cache without an explicit
     * data-number argument. */
    fn_1_E18(3);

    state->state.shadowModel = fn_1_4994();
    Hu3DModelLayerSet((s16)state->state.shadowModel, 3);
    Hu3DModelShadowSet((s16)state->state.shadowModel);
    Hu3DModelAttrSet((s16)state->state.shadowModel, 8);
    Hu3DModelScaleSet((s16)state->state.shadowModel, (0.5f),
        (0.5f), (0.5f));
    Hu3DModelShadowDispOff((s16)state->state.shadowModel);
    state->state.shadowEnabled = 0;
    work->processPhase = 0;
    work->volumeEnabled = 1;
    object->objFunc = fn_1_5AB0;
}

/* The player object runs this callback each update to advance its state and movement. */
void fn_1_5AB0(OMOBJ *object)
{
    M674PlayerWork *work = object->data;
    M674PlayerWork *state = work;

    fn_1_8894(&state->state);
    fn_1_67B0(&state->state);
    switch (work->processPhase) {
    case 0:
        if (fn_1_5C04(state)) {
            state->state.task.shape.actor.callback = fn_1_7660;
            work->processPhase++;
            state->state.phase = 0;
        }
        break;
    case 1:
        if (fn_1_5C98(state)) {
            work->processPhase++;
            state->state.phase = 0;
        }
        break;
    case 2:
        if (fn_1_5FC4(state)) {
            work->processPhase++;
            state->state.phase = 0;
        }
        break;
    case 3:
        if (fn_1_6170(state)) {
            work->processPhase++;
            state->state.phase = 0;
        }
        break;
    }
    fn_1_77F0(state);
    fn_1_7958(state);
    fn_1_67D8(object);
    PSVECAdd(&state->state.position, &state->state.velocity, &state->state.position);
    fn_1_6288(object);
}

/* Update movement and jumping while waiting for MAIN, then report this setup phase complete. */
s32 fn_1_5C04(M674PlayerWork *object)
{
    void *sequenceContext[1];
    s32 done;

    sequenceContext[0] = (u8 *)object + 372;
    done = 0;
    switch (object->state.phase) {
    case 0:
        object->state.phase++;
        /* fall through */
    case 1:
        if (MgSeqModeGet() != 5) {
            break;
        }
        object->state.phase++;
        /* fall through */
    default:
        done = 1;
        break;
    }
    fn_1_6AE0(object);
    fn_1_6DC0(object);
    return done;
}

/* Gather human or computer input during play; the ten-frame finish wait and completion paths
 * return without a value. */
s32 fn_1_5C98(M674PlayerWork *state)
{
    M674PlayerConfig *config = &state->state.config;
    s32 controllerIndex;
    s32 done = 0;
    float x, y;

    switch (state->state.phase) {
    case 0:
        state->state.phase++;
        state->state.timer = 0;
    case 1:
        if (MgSeqModeGet() == MGSEQ_MODE_FINISH) {
            state->state.phase++;
            state->state.timer = 0;
        } else if (state->state.computer) {
            fn_1_7DB0(state);
        } else if (state->state.controlEnabled) {
            M674PlayerWork *input = state;
            controllerIndex = GwPlayerConf[input->playerIndex].padNo;
            x = HuPadStkXf[controllerIndex];
            y = -HuPadStkYf[controllerIndex];
            state->state.buttons = HuPadBtn[controllerIndex];
            state->state.buttonsDown = HuPadBtnDown[controllerIndex];
            state->state.stickAngle = 180.0 * (atan2(x, y) / M_PI);
            state->state.stickMagnitude = sqrtf(x * x + y * y);
        }
        break;
    case 2:
        state->state.phase++;
        state->state.timer = 0;
        state->state.movementLock = 6000;
    case 3:
        if (++state->state.timer < 10) {
            /* No value is returned on this path. */
            return;
        }
        state->state.phase++;
        state->state.timer = 0;
    default:
        done = 1;
        /* No value is returned on this path. */
        return;
    }
    fn_1_6AE0(state);
    fn_1_6DC0(state);
    return done;
}

/* Reset the player after the results wipe, report readiness after 60 frames, and wait for WINNER
 * and all four readiness flags. */
s32 fn_1_5FC4(M674PlayerWork *work)
{
    M674PlayerConfig *actionData = &work->state.config;
    s32 ready = 0;
    s32 variantIndex;

    switch (work->state.phase) {
    case 0:
        work->state.phase++;
        work->state.timer = 0;
        /* fall through */
    case 1:
        if (fn_1_E20() == 0) {
            break;
        }
        work->state.phase++;
        work->state.timer = 0;
        work->state.task.flags &= ~4;
        variantIndex = work->state.playerNo;
        work->state.position = lbl_1_data_198[variantIndex];
        work->state.velocity.x = work->state.velocity.y = work->state.velocity.z = 0.0f;
        work->state.rotation.y = lbl_1_data_1C8[variantIndex];
        work->state.spareState = 5;
        work->state.motionActive = 0;
        work->state.action = 0;
        work->state.actionResult = 0;
        work->state.cooldown = 1;
        fn_1_67CC(&work->state, actionData->idleMotion, 1.0f);
        /* fall through */
    case 2:
        if (work->state.controlEnabled != 0) {
            M674PlayerWork *timedWork = work;

            if (++work->state.timer < 60) {
                break;
            }
            fn_1_F1C(timedWork->playerIndex);
        }
        if (MgSeqModeGet() == 8 && fn_1_F3C() != 0) {
            work->state.phase++;
            work->state.timer = 0;
            /* fall through */
        } else {
            break;
        }
    default:
        ready = 1;
        break;
    }
    fn_1_6AE0(work);
    fn_1_6DC0(work);
    return ready;
}

/* Play the win or lose motion and finish this player's result phase after its delay. */
s32 fn_1_6170(M674PlayerWork *state)
{
    M674PlayerConfig *config = &state->state.config;
    M674PlayerWork *work;
    s32 done = 0;

    switch (state->state.phase) {
    case 0:
        if ((s32)fn_1_E68(state->state.scoreIndex) > 0) {
            fn_1_67CC(&state->state, config->winMotion, (1.0f));
        } else {
            fn_1_67CC(&state->state, config->loseMotion, (1.0f));
        }
        if (state->state.controlEnabled) {
            work = state;
            work->volumeEnabled = 0;
            CharModelVoiceVolSet((s16)work->characterNo, 127);
        }
        state->state.phase++;
        state->state.timer = 0;
        break;
    case 1:
        if (++state->state.timer < 90) {
            break;
        }
        state->state.phase++;
        state->state.timer = 0;
    default:
        done = 1;
        break;
    }
    return done;
}

/* Update motion completion and character voice panning on each player-object update. */
void fn_1_6288(OMOBJ *object)
{
    M674PlayerWork *state;
    M674PlayerWork *work = object->data;

    state = work;
    work->updateCount++;
    if (state->state.motionActive) {
        if (fn_1_6A1C(object) != 0) {
            state->state.motionActive = 0;
            state->state.motionTimer = 0;
        }
    } else {
        state->state.motionTimer = 0;
    }
    CharModelVoicePanSet((s16) work->characterNo,
        (s16) fn_1_36BC(&state->state.position));
}

/* Play a character effect at this player's position and register its sound handle. */
void fn_1_6324(OMOBJ *object, s32 effect)
{
    M674PlayerWork *work = object->data;
    M674PlayerWork *owner = work;
    s32 handle = CharFXPlayPan((s16) work->characterNo, (s16) effect,
        (s16) fn_1_36BC(&owner->state.position));

    fn_1_47F4(effect, handle, &owner->state.position, 1, 0);
}

/* The collision entry calls this to place the shadow proxy and choose which model casts. */
void fn_1_6398(OMOBJ *object)
{
    M674PlayerWork *work = object->data;
    M674PlayerWork *shadowWork = work;
    s32 modelId = object->mdlId[0];
    f32 distanceX;
    f32 distanceZ;
    f32 heading;
    f32 positionDistance;
    f32 minimumDistance;
    s32 useShadowModel;

    fn_1_88C8(&shadowWork->state, modelId);
    Hu3DModelPosSetV((s16)shadowWork->state.shadowModel, &shadowWork->state.position);
    if (shadowWork->state.actionResult != 0) {
        Hu3DModelShadowDispOff(modelId);
        Hu3DModelShadowDispOff((s16)shadowWork->state.shadowModel);
        return;
    }

    distanceX = shadowWork->state.position.x;
    distanceZ = shadowWork->state.position.z;
    heading = (f32)((180.0) *
        (atan2((f64)distanceX, (f64)distanceZ) / (3.141592653589793)));

    positionDistance = sqrtf(distanceX * distanceX + distanceZ * distanceZ);

    distanceX = (f32)((f64)shadowWork->state.radialOffsetX *
        sin(((3.141592653589793) * (f64)heading) / (180.0)));
    distanceZ = (f32)((f64)shadowWork->state.radialOffsetZ *
        cos(((3.141592653589793) * (f64)heading) / (180.0)));
    minimumDistance = (f32)((f64)sqrtf(distanceX * distanceX + distanceZ * distanceZ) -
        (100.0));
    if (positionDistance < minimumDistance) {
        useShadowModel = 1;
    } else {
        useShadowModel = 0;
    }
    shadowWork->state.shadowEnabled = useShadowModel;
    if (shadowWork->state.shadowEnabled != 0) {
        Hu3DModelShadowDispOff(modelId);
        Hu3DModelShadowDispOn((s16)shadowWork->state.shadowModel);
        return;
    }
    Hu3DModelShadowDispOn(modelId);
    Hu3DModelShadowDispOff((s16)shadowWork->state.shadowModel);
}

/* Clear the previous motion request before the player object processes its next update. */
void fn_1_67B0(M674ActorState *actor) {
    actor->requestedMotion = -1;
    actor->motionSpeed = (1.0f);
}

/* Queue a motion and speed for the object's later motion-request processing. */
void fn_1_67CC(M674ActorState *actor, s32 motion, f32 speed) {
    actor->requestedMotion = motion;
    actor->motionSpeed = speed;
}

/* Apply a pending motion request during the player's object update. */
void fn_1_67D8(OMOBJ *object)
{
    M674PlayerWork *work = object->data;
    M674PlayerWork *state = work;

    if (state->state.requestedMotion != -1) {
        fn_1_6840(object, state->state.requestedMotion);
        Hu3DMotionSpeedSet(object->mdlId[0], state->state.motionSpeed);
    }
}

/* Switch the character's main motion and optional overlay when its motion index changes. */
void fn_1_6840(OMOBJ *object, s32 index)
{
    M674PlayerWork *state;
    M674PlayerWork *work = object->data;
    u32 overlayAttr;
    u32 attr;
    s32 model, motion, overlay;
    float blend;
    state = work;
    overlayAttr = 0;
    model = object->mdlId[0];
    if (state->state.motionIndex != index && index < 8) {
        if (state->state.motionIndex < 0) {
            blend = (0.0f);
        } else {
            blend = (60.0f) * lbl_1_data_F8[index].blend;
        }
        state->state.motionIndex = index;
        state->state.motionFrame = 0;
        motion = object->mtnId[lbl_1_data_F8[index].motion];
        attr = lbl_1_data_F8[index].attr;
        Hu3DModelAttrReset(model, HU3D_MOTATTR_LOOP | HU3D_MOTATTR_REV |
            HU3D_MOTATTR_OVL_LOOP | HU3D_MOTATTR_OVL_REV);
        overlay = lbl_1_data_F8[index].overlay;
        if (overlay >= 0) {
            CharMotionShiftSet(work->characterNo, motion, (0.0f), blend, attr);
            Hu3DMotionOverlaySet(model, object->mtnId[overlay]);
            if (attr & HU3D_MOTATTR_LOOP) { overlayAttr |= HU3D_MOTATTR_OVL_LOOP; }
            if (attr & HU3D_MOTATTR_REV) { overlayAttr |= HU3D_MOTATTR_OVL_REV; }
            Hu3DModelAttrSet(model, overlayAttr);
        } else {
            Hu3DMotionOverlayReset(model);
            CharMotionShiftSet(work->characterNo, motion, (0.0f), blend, attr);
        }
        CharMotionSpeedSet(work->characterNo, (1.0f));
    }
}

/* Check whether the character motion has ended and no motion shift remains active. */
s32 fn_1_6A1C(OMOBJ *object)
{
    M674PlayerWork *work = object->data;
    if (CharMotionEndCheck(work->characterNo) != 0 &&
        CharMotionShiftIDGet(work->characterNo) < 0) {
        return 1;
    }
    return 0;
}

/* Report whether this character has finished blending into its current motion. */
s32 fn_1_6A88(OMOBJ *object)
{
    M674PlayerWork *work = object->data;
    if ((s16) CharMotionShiftIDGet((s16) work->characterNo) < 0) {
        return 1;
    }
    return 0;
}

/* Update horizontal movement and request idle, walk or run motions during player phases. */
void fn_1_6AE0(M674PlayerWork *work)
{
    M674PlayerConfig *motion = &work->state.config;
    f32 movementInput = work->state.stickMagnitude * work->state.blendAmount;
    f32 movement;
    f32 startSpeed;
    f32 endSpeed;
    f64 threshold;
    s32 idleMotion;
    s32 walkMotion;
    s32 runMotion;
    s32 motionLocked;

    if (work->state.motionType != 0) {
        return;
    }
    if (work->state.actionResult != 0) {
        movementInput = (0.0f);
    }
    if (work->state.action != 0) {
        if (movementInput > (0.05f) && work->state.motionTimer == 0) {
            work->state.rotation.y =
                fn_1_4BAC(work->state.rotation.y, work->state.stickAngle, (0.5f));
        }
        return;
    }

    if (work->state.actionResult == 0) {
        startSpeed = motion->walkSpeed;
        endSpeed = motion->runSpeed;
        idleMotion = motion->idleMotion;
        walkMotion = motion->walkMotion;
        runMotion = motion->runMotion;
    } else {
        startSpeed = motion->lowerSurfaceWalkSpeed;
        endSpeed = motion->lowerSurfaceRunSpeed;
        idleMotion = motion->lowerSurfaceIdleMotion;
        walkMotion = motion->lowerSurfaceWalkMotion;
        runMotion = motion->lowerSurfaceRunMotion;
        movementInput *= 0.5;
    }

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

    motionLocked = work->state.motionActive;
    if (work->state.motionIndex == motion->landMotion) {
        motionLocked = 0;
    }
    if (movementInput >= (0.05f) && work->state.movementLock == 0) {
        work->state.motionActive = 0;
        movement = movementInput * motion->moveSpeed;
        work->state.velocity.x =
            (f32) ((f64) movement *
                   sin(((3.141592653589793) * (f64) work->state.stickAngle) / (180.0)));
        work->state.velocity.z =
            (f32) ((f64) movement *
                   cos(((3.141592653589793) * (f64) work->state.stickAngle) / (180.0)));
        if (work->state.motionTimer == 0) {
            work->state.rotation.y =
                fn_1_4BAC(work->state.rotation.y, work->state.stickAngle, (0.5f));
        }
        if (motionLocked == 0) {
            if (work->state.motionIndex == runMotion) {
                threshold = (0.45);
            } else {
                threshold = (0.5);
            }
            if (movementInput < threshold) {
                work->state.requestedMotion = walkMotion;
                work->state.motionSpeed = movementInput * startSpeed;
                return;
            } else {
                work->state.requestedMotion = runMotion;
                work->state.motionSpeed = movementInput * endSpeed;
            }
        }
    } else {
        if (motionLocked == 0) {
            work->state.requestedMotion = idleMotion;
            work->state.motionSpeed = (1.0f);
        }
        work->state.velocity.x = work->state.velocity.z = (0.0f);
    }
}

/* Update jump, gravity, landing and bounce state while the player object advances. */
void fn_1_6DC0(M674PlayerWork *work)
{
    f32 zero = (0.0f);
    f32 speed;
    void *contactRegion = work->state.task.shape.actor.contactRegion;
    M674PlayerConfig * motion = &work->state.config;
    s32 mode = 0;
    s32 i;
    f32 angle;
    if (work->state.spareState != 0)
    {
        work->state.spareState--;
        return;
    }
    if (work->state.actionResult != 0)
    {
        mode = 1;
    }
    if (MgSeqModeGet() != 5)
    {
        mode = 1;
    }
    if (work->state.motionType != 0)
    {
        mode = 1;
    }
    speed = (f32)(0.7 * (f64)(work->state.stickMagnitude * motion->moveSpeed));
    speed *= work->state.blendAmount;
    /* This camera-relative heading is calculated but unused; steering below uses actor rotation. */
    angle = work->state.stickAngle;
    angle = fmodf((angle + CRotM[0].y), 360.0);
    if (work->state.actionResult != 0)
    {
        work->state.position.y = (f32)((f64)zero +
            sin((3.141592653589793 * (f64)work->state.bounceAngle) / 180.0) *
                (f64)work->state.bounceHeight);
        work->state.bounceHeight *= 0.95;
        work->state.bounceAngle = fmodf(work->state.bounceAngle - 8.0f, 360.0f);
        return;
    }
    if (work->state.action == 0)
    {
        if (mode == 0 && (0.0f) != work->state.blendAmount &&
            (work->state.buttonsDown & 0x100) != 0) {
            work->state.velocity.x =
                (f32) ((f64) speed *
                       sin((3.141592653589793 * (f64) work->state.rotation.y) / 180.0));
            work->state.velocity.z =
                (f32) ((f64) speed *
                       cos((3.141592653589793 * (f64) work->state.rotation.y) / 180.0));
            work->state.velocity.y = motion->jumpSpeed * work->state.blendAmount;
            work->state.action = 1;
            work->state.impactJump = 0;
            fn_1_67CC(&work->state, motion->jumpMotion, 1.0f);
        }
        if (work->state.action == 0 && work->state.position.y > zero && contactRegion == NULL)
        {
            work->state.velocity.y =
                (f32) ((f64) work->state.velocity.y + 5.0 * (f64) motion->gravity);
            work->state.action = 1;
            work->state.impactJump = 0;
            fn_1_67CC(&work->state, motion->jumpMotion, 1.0f);
        }
    }
    else
    {
        f32 speedScale;
        if (0 != mode)
        {
            if (work->state.position.y > (0.0f))
            {
                work->state.velocity.y = work->state.velocity.y + (motion->gravity);
                work->state.velocity.x *= 0.98;
                work->state.velocity.z *= 0.98;
            }
        }
        else
        {
            M674ActorState * other = fn_1_DC2C(& work->state.position, (0.0f), (180.0f), (180.0f));
            if (other != NULL && work->state.velocity.y < 0.0 &&
                fn_1_4E18(&work->state.position, &other->position) > 30.0) {
                f64 xOffset;
                f64 zOffset;
                if (! (! (work->state.position.x < other->position.x)))
                {
                    xOffset = 8.0;
                }
                else
                {
                    xOffset = - 8.0;
                }
                work->state.position.x = (f32)((f64) work->state.position.x + xOffset);
                if (work->state.position.z < other->position.z)
                {
                    zOffset = 8.0;
                }
                else
                {
                    zOffset = - 8.0;
                }
                work->state.position.z = (f32)((f64) work->state.position.z + zOffset);
            }
            {
                f32 desiredX =
                    (f32) ((f64) speed *
                           sin((3.141592653589793 * (f64) work->state.rotation.y) / 180.0));
                f32 desiredZ =
                    (f32) ((f64) speed *
                           cos((3.141592653589793 * (f64) work->state.rotation.y) / 180.0));
                speedScale = work->state.stickMagnitude;
                work->state.velocity.x += (desiredX - work->state.velocity.x) * speedScale;
                work->state.velocity.z += speedScale * (desiredZ - work->state.velocity.z);
                work->state.velocity.x *= 0.98;
                work->state.velocity.z *= 0.98;
                if (work->state.position.y > zero)
                {
                    if ((work->state.buttons & 0x100U) != 0U &&
                        work->state.velocity.y >= (0.0f) &&
                        work->state.impactJump == 0)
                    {
                        work->state.velocity.y =
                            (f32) ((f64) work->state.velocity.y + 0.55 * (f64) motion->gravity);
                    }
                    else
                    {
                        work->state.velocity.y += motion->gravity;
                    }
                }
            }
        }
    if (work->state.velocity.y < (0.0f) &&
        (work->state.position.y <= zero || contactRegion != NULL))
    {
        Vec effectPosition;
        if (work->state.position.y <= zero)
        {
            work->state.position.y = zero;
            work->state.actionResult = 1;
            work->state.bounceHeight = fabs(3.0 * (f64) work->state.velocity.y);
            work->state.bounceAngle = - 30.0f;
            work->state.task.shape.actor.callback = 0;
            work->state.task.shape.actor.radius = 30.0f;
            work->state.task.flags |= 4;
            effectPosition = work->state.position;
            effectPosition.y = - 20.0f;
    fn_1_F9A4(& effectPosition, 50.0f, 0.5f, 0.05f, 208);
            effectPosition = work->state.position;
            effectPosition.y = 50.0f;
            for (i = 0; i < 8; ++i)
            {
                fn_1_F660(& effectPosition);
            }
            if (work->state.controlEnabled == 0)
            {
                M674PlayerWork * work2 = work;
            omVibrate((s16) work2->playerIndex, 20, 20, 0);
            }
            fn_1_4040(MSM_SE_M674_GROUND_IMPACT, & work->state.position);
        }
        else
        {
            work->state.actionResult = 0;
            if (work->state.blendAmount != 0.0f)
            {
                fn_1_67CC(&work->state, motion->landMotion, 1.0f);
                work->state.motionActive = 1;
            }
            if (work->state.controlEnabled == 0)
            {
                effectPosition = work->state.position;
                effectPosition.y += 10.0;
                CharEffectDustCreate(- 1, motion->dustStrength, &effectPosition);
            }
        }
        work->state.velocity.x = work->state.velocity.y = work->state.velocity.z = (0.0f);
        work->state.action = 0;
        work->state.impactJump = 0;
    }
    }
}

/* Handle actor contacts, recording contact pulses and requesting an upper-actor bounce and
 * lower-actor compression when the vertical test succeeds. */
s32 fn_1_7660(M674CollisionParam *first, M674CollisionParam *second)
{
    OMOBJ *firstObject;
    OMOBJ *secondObject;
    M674PlayerWork *firstState;
    M674PlayerWork *secondState;
    M674PlayerWork *firstWork;
    M674PlayerWork *secondWork;
    M674PlayerWork *linkedWork;
    M674CollisionParam *firstParam;
    M674CollisionParam *secondParam;
    M674PlayerWork *voiceData;
    f32 verticalDelta;
    s32 soundHandle;

    if (second->state == 0) {
    firstObject = first->primaryRelation;
    firstState = firstObject->data;
    firstWork = firstState;
    firstParam = first;
    secondObject = second->primaryRelation;
    secondState = secondObject->data;
    secondWork = secondState;
    secondParam = second;
    verticalDelta = secondParam->shape.actor.position->y - firstParam->shape.actor.position->y;
    if (firstWork->state.action != 0 && firstWork->state.actionResult == 0 &&
        verticalDelta < 0.7 * -secondParam->shape.actor.radius) {
        if (firstWork->state.controlEnabled != 0 && firstWork->state.pendingImpact == 0) {
            omVibrate((s16)firstState->playerIndex, 20, 20, 0);
        }
        if (secondWork->state.controlEnabled != 0 && secondWork->state.blendRequest == 0) {
            omVibrate((s16)secondState->playerIndex, 20, 20, 0);
            linkedWork = secondObject->data;
            voiceData = linkedWork;
            soundHandle = CharFXPlayPan((s16)linkedWork->characterNo,
                MSM_SE_CHARVOICE_MARIO + 15,
                (s16)fn_1_36BC(&voiceData->state.position));
            fn_1_47F4(MSM_SE_CHARVOICE_MARIO + 15, soundHandle,
                &voiceData->state.position, 1, 0);
        }
        firstWork->state.pendingImpact = 1;
        firstWork->state.partner = &secondWork->state;
        secondWork->state.blendRequest = 1;
        secondWork->state.partner = &firstWork->state;
        secondWork->state.partnerScoreIndex = firstWork->state.scoreIndex;
    }
    firstWork->state.contact = 1;
    secondWork->state.contact = 1;
    }
    return 1;
}

/* Apply a pending contact bounce using the partner actor's position during this player's update. */
void fn_1_77F0(M674PlayerWork *work)
{
    M674ActorState *linkedState;
    M674PlayerConfig *motion;
    s32 resource;
    f32 angle;
    f32 horizontalScale;
    f32 verticalSpeed;

    if (work->state.pendingImpact != 0) {
        if (work->state.actionResult == 0) {
            motion = &work->state.config;
            linkedState = work->state.partner;
            angle = fn_1_4D7C(&linkedState->position, &work->state.position);
            horizontalScale = (2.1666667f);
            verticalSpeed = (25.0f);
            work->state.velocity.x += horizontalScale * sin((3.141592653589793) * angle / (180.0));
            work->state.velocity.z += horizontalScale * cos((3.141592653589793) * angle / (180.0));
            if (MgSeqModeGet() == 5) {
                work->state.velocity.y = verticalSpeed;
            }
            work->state.action = 1;
            work->state.impactJump = 1;
            resource = motion->jumpMotion;
            work->state.requestedMotion = resource;
            work->state.motionSpeed = (1.0f);
        }
        work->state.pendingImpact = 0;
    }
}

/* Advance the partner-triggered blend and damp movement during each player update. */
void fn_1_7958(M674PlayerWork *work)
{
    M674PlayerConfig *blendState = &work->state.config;
    M674PlayerConfig *profile;
    M674ActorState *motionResource;
    f32 targetBlend = (1.0f);
    f32 targetValue = (1.0f);
    f32 value;

    if (work->state.blendRequest != 0) {
        if ((work->state.blendState == 0) && (work->state.cooldown == 0) &&
            (work->state.velocity.y <= (0.0f))) {
            motionResource = work->state.partner;
            profile = &motionResource->config;
            work->state.blendState = 1;
            value = (f32)((1.0) +
                ((1.0) - blendState->currentBlend));
            work->state.requestedBlend = profile->blendScale * value;
            work->state.xzScaleGain = (f32)(value *
                (((1.0) / profile->blendScale) *
                    ((f64)profile->blendSpan - (1.0))));
        }
        work->state.velocity.x *= 0.5;
        work->state.velocity.z *= 0.5;
        work->state.blendRequest = 0;
    }

    switch (work->state.blendState) {
    case 0:
        break;
    case 1:
        work->state.blendState++;
        work->state.blendFrame = 0;
        /* fall through */
    case 2:
        targetBlend = (0.5f);
        targetValue = (f32)((0.1282051282051282) * (f64)work->state.blendFrame);
        targetValue = (f32)((1.0) -
            ((f64)targetValue *
                ((1.0) - work->state.requestedBlend)));
        if (!((f64)++work->state.blendFrame < (7.800000000000001))) {
            work->state.blendState++;
            work->state.blendFrame = 0;
        }
        break;
    case 3:
        targetBlend = (0.5f);
        targetValue = work->state.requestedBlend;
        if (!((f64)++work->state.blendFrame < (90.0))) {
            work->state.blendState++;
            work->state.blendFrame = 0;
        }
        break;
    case 4:
        targetBlend = (0.5f);
        targetValue = (f32)((0.050505050505050504) * work->state.blendFrame);
        targetValue = (f32)((0.5) *
            ((1.0) + -cos((6.283185307179586) *
                ((1.5) * (f64)targetValue))));
        targetValue = (f32)(work->state.requestedBlend +
            ((f64)targetValue *
                ((1.0) - work->state.requestedBlend)));
        if (!((f64)++work->state.blendFrame < (19.8))) {
            work->state.blendState = 0;
            if (work->state.actionResult == 0) {
                work->state.cooldown = 90;
            }
            targetValue = (1.0f);
        }
        break;
    }

    work->state.blendAmount = (f32)((f64)work->state.blendAmount +
        ((0.2) * (f64)(targetBlend - work->state.blendAmount)));
    work->state.blendValue = targetValue;
}

/* Run the active computer-control callback and clear the contact pulse each update. */
void fn_1_7DB0(M674PlayerWork *work)
{
    switch (work->state.controlPhase) {
    case 0:
        work->state.controlPhase++;
        fn_1_8748(work, fn_1_7E28);
        /* Continue into the newly installed update callback. */
    case 1:
        work->state.controlUpdate(work);
        break;
    }
    work->state.contact = 0;
}

/* Steer a grounded CPU toward a rim point aligned with a target, or an offset patrol point,
 * then turn inward and request a jump. */
void fn_1_7E28(M674PlayerWork *work)
{
    M674ComputerParam *motionParam = &lbl_1_data_240[work->state.difficulty];
    Vec targetPosition;
    f32 heading;
    f32 scanRange;
    f32 scanDistance;
    f64 randomHeadingOffset;
    M674ActorState *target = NULL;

    if (fn_1_8778(work) == 0) {
        switch (work->state.controlState) {
        case 0:
            work->state.controlState++;
            if ((s32)((2.0) *
                    ((0.0000152587890625) * (f64)(f32)(s32)(frand() & 0xFFFF))) == 0) {
                randomHeadingOffset = (85.0);
            } else {
                randomHeadingOffset = (-85.0);
            }
            work->state.headingOffset = (f32)randomHeadingOffset;
            /* fall through */
        case 1:
            if (work->state.controlTimer < 30 && work->state.contact != 0) {
                work->state.headingOffset = -work->state.headingOffset;
                work->state.controlTimer = 90;
            }
            if (work->state.controlTimer > 0) {
                work->state.controlTimer--;
            } else {
                scanDistance = (f32)((10.0) *
                    (f64)((f32)work->state.controlFrame * motionParam->speed));
                if (lbl_1_bss_2E0 != 0) {
                    scanDistance *= 3.0;
                }
                target = fn_1_DC2C(&work->state.position, work->state.rotation.y,
                    (90.0f), scanDistance);
            }
            if (target != NULL) {
                heading = (f32)((180.0) *
                    (atan2((f64)target->position.x,
                        (f64)target->position.z) / (3.141592653589793)));
                targetPosition.x = (f32)(((100.0) +
                    (f64)work->state.radialOffsetX) *
                    sin(((3.141592653589793) * (f64)heading) /
                        (180.0)));
                targetPosition.z = (f32)(((100.0) +
                    (f64)work->state.radialOffsetZ) *
                    cos(((3.141592653589793) * (f64)heading) /
                        (180.0)));
            } else {
                heading = (f32)((180.0) *
                    (atan2((f64)work->state.position.x,
                        (f64)work->state.position.z) / (3.141592653589793)));
                targetPosition.x = (f32)(((100.0) +
                    (f64)work->state.radialOffsetX) *
                    sin(((3.141592653589793) * (f64)heading) /
                        (180.0)));
                targetPosition.z = (f32)(((100.0) +
                    (f64)work->state.radialOffsetZ) *
                    cos(((3.141592653589793) * (f64)heading) /
                        (180.0)));
                heading += work->state.headingOffset;
                targetPosition.x = (f32)((f64)targetPosition.x +
                    ((100.0) *
                        ((1.5) *
                            sin(((3.141592653589793) * (f64)heading) /
                                (180.0)))));
                targetPosition.z = (f32)((f64)targetPosition.z +
                    ((100.0) *
                        ((1.5) *
                            cos(((3.141592653589793) * (f64)heading) /
                                (180.0)))));
            }
            targetPosition.y = work->state.position.y;
            work->state.stickAngle = fn_1_4D7C(&work->state.position, &targetPosition);
            work->state.stickMagnitude = 0.8f;
            scanDistance = fn_1_4E18(&work->state.position, &targetPosition);
            if (lbl_1_bss_2E0 == 0) {
                scanRange = (80.0f);
            } else {
                scanRange = (400.0f);
            }
            if (scanDistance > scanRange) {
                break;
            }
            work->state.controlState++;
            /* fall through */
        case 2:
            heading = (f32)((180.0) *
                (atan2((f64)-work->state.position.x,
                    (f64)-work->state.position.z) / (3.141592653589793)));
            work->state.stickAngle = heading;
            work->state.stickMagnitude = 0.3f;
            if (fabs((f64)fn_1_4CC8(work->state.rotation.y, heading)) < 10.0f) {
                work->state.buttonsDown |= 0x100;
            }
            break;
        }
        work->state.controlFrame++;
    }
}

/* Steer an airborne CPU toward an uncollected target, falling back to a radial heading or
 * toward the center according to its difficulty settings. */
void fn_1_83C0(M674PlayerWork *work)
{
    M674ComputerParam * motionParam = & lbl_1_data_240[work->state.difficulty];
    f32 maxDistance;
    f32 speed;
    if (fn_1_8778(work) == 0)
    {
        speed = motionParam->speed;
        if (fn_1_100F8(work->state.scoreIndex) >= motionParam->playerThreshold)
        {
            speed = (f32)((f64) speed / 2.0);
        }
        switch (work->state.controlState)
        {
        case 0 :
            work->state.controlState++;
        case 1 :
            {
                f32 heading;
                f32 searchRange;
                M674ActorState * target;
                work->state.buttons |= 0x100;
                heading = (f32) ((180.0) * (atan2((f64) work->state.velocity.x,
                                                  (f64) work->state.velocity.z) /
                                            (3.141592653589793)));
                searchRange = (f32)((40.0) * (f64) speed);
                maxDistance = speed *= (400.0);
                target = fn_1_DC2C(& work->state.position, heading, searchRange, maxDistance);
                if (target == NULL)
                {
                    target =
                        fn_1_DC2C(&work->state.position, heading, (f32) ((2.0) * (f64) searchRange),
                                  (f32) ((1.2) * (f64) maxDistance));
                }
                if (target == NULL)
                {
                    target = fn_1_DC2C(&work->state.position, heading, (180.0f),
                                       (f32) ((1.5) * (f64) maxDistance));
                }
                if (target != NULL)
                {
                    work->state.stickAngle = fn_1_4D7C(& work->state.position, & target->position);
                    maxDistance = fn_1_4E18(& work->state.position, & target->position);
                    if (maxDistance > (50.0))
                    {
                        work->state.stickMagnitude = (1.0f);
                    }
                }
                else
                {
                    if (motionParam->useFixedDirection != 0)
                    {
                        heading = (f32) ((180.0) * (atan2((f64) work->state.position.x,
                                                          (f64) work->state.position.z) /
                                                    (3.141592653589793)));
                        if (fabs((f64) fn_1_4CC8(work->state.rotation.y, heading)) > 90.0)
                        {
                            heading += 180.0;
                        }
                        work->state.stickAngle = heading;
                        work->state.stickMagnitude = (1.0f);
                    }
                    else
                    {
                        if (fn_1_4E18(& work->state.position, & lbl_1_data_1D8) > 200.0)
                        {
                            heading = (f32) ((180.0) * (atan2((f64) -work->state.position.x,
                                                              (f64) -work->state.position.z) /
                                                        (3.141592653589793)));
                            work->state.stickAngle = heading;
                            work->state.stickMagnitude = (1.0f);
                        }
                    }
                }
                break;
            }
        }
        work->state.controlFrame++;
    }
}

void fn_1_8744(M674PlayerWork *work)
{
}

/* Install the next computer-control callback and reset its per-state counters. */
void fn_1_8748(M674PlayerWork *work, M674PlayerControl callback)
{
    work->state.controlUpdate = callback;
    work->state.controlState = 0;
    work->state.controlFrame = 0;
    work->state.controlTimer = 0;
    work->state.headingOffset = 0.0f;
}

/* Install the no-op control handler after a lower-surface landing; otherwise select the
 * grounded or airborne handler. */
s32 fn_1_8778(M674PlayerWork *work)
{
    if (work->state.controlUpdate != fn_1_8744 && work->state.actionResult != 0) {
        fn_1_8748(work, fn_1_8744);
        return 1;
    }
    if (work->state.controlUpdate != fn_1_83C0 && work->state.action != 0) {
        fn_1_8748(work, fn_1_83C0);
        return 1;
    }
    if (work->state.controlUpdate != fn_1_7E28 && work->state.action == 0) {
        fn_1_8748(work, fn_1_7E28);
        return 1;
    }
    return 0;
}

/* Clear the buttons and stick values before the player object gathers this frame's input. */
void fn_1_8894(M674ActorState *work)
{
    work->buttonsDown = 0;
    work->buttons = 0;
    work->stickAngle = 0.0f;
    work->stickMagnitude = 0.0f;
}

/* Apply the character transform and cooldown flash before the shadow callback selects a model. */
void fn_1_88C8(M674ActorState *work, s32 modelId)
{
    M674PlayerConfig *config;
    f32 previousRotation;
    f32 wrappedRotation;
    f32 storedRotation;
    f32 scale;

    config = &work->config;
    previousRotation = work->rotation.y;
    wrappedRotation = (f32)fmod((f64)previousRotation, 360.0);
    storedRotation = wrappedRotation;
    work->rotation.y = storedRotation;
    if (work->rotation.y < (0.0)) {
        work->rotation.y += 360.0;
    }
    Hu3DModelPosSetV((s16)modelId, &work->position);
    Hu3DModelRotSetV((s16)modelId, &work->rotation);
    scale = (f32)(1.0 +
                  (1.0 - work->blendValue) * work->xzScaleGain);
    Hu3DModelScaleSet(modelId, scale, work->blendValue, scale);
    if (work->cooldown > 0) {
        if ((--work->cooldown) & 1) {
            Hu3DModelAttrSet(modelId, 1);
        } else {
            Hu3DModelAttrReset(modelId, 1);
        }
    }
    work->motionFrame++;
}

/* Count player objects whose work data exists and whose action result is still clear. */
s32 fn_1_8A44(void)
{
    OMOBJ *object;
    M674PlayerWork *work;
    s32 count = 0;
    s32 index;

    for (index = 0; index < 4; index++) {
        object = lbl_1_bss_2FC[index];
        if (object != NULL) {
            work = object->data;
            if (work != NULL && work->state.actionResult == 0) {
                count++;
            }
        }
    }
    return count;
}

OMOBJ *lbl_1_bss_30C;
OMOBJ *lbl_1_bss_2FC[4];
M674ActorState *lbl_1_bss_2E8[5];
s32 lbl_1_bss_2E4;
s32 lbl_1_bss_2E0;
