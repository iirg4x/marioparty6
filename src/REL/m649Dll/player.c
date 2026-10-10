/* Tracks the four contestants and updates their models during Stamp By Me. */
#include "dolphin/math.h"

#include "dolphin/mtx.h"

#include "game/object.h"

#include "game/gamework.h"

#include "include/game/charman.h"

#include "include/game/memory.h"

#include "include/game/audio.h"

#include "include/game/gamework.h"

#include "include/game/pad.h"

#include "include/game/frand.h"

#include "include/string.h"

#include "dolphin/gx.h"

#include "game/main.h"

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
#include "REL/m649Dll/module_types.h"

#include "game/hu3d.h"

#include <string.h>

#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/arith.h"

typedef struct M649PlayerState_s {
    u32 camera;
    s32 previousPlayer, player, controlEnabled;
    Vec position, velocity, rotation;
    u32 buttonsDown, buttons;
    float stickAngle, stickMagnitude;
    s32 currentMotion, motionTime, requestedMotion;
    float motionSpeed;
    s32 phase, timer;
    /* Cleared at character setup; no player update reads these bytes. */
    u8 unknown_5C_to_6B[16];
    s32 updateCount; /* Count of player-manager updates since registration. */
    s32 computer, difficulty, computerPhase; /* Computer control, difficulty, and input phase. */
    s32 computerTimer; /* Frames until the next computer-controlled button press. */
    float computerDirection; /* Facing angle used to select the computer-controlled button. */
    /* Turn-motion active flag and a counter kept at zero by player updates. */
    s32 motionActive, motionTimer;
    /* Cleared at character setup; no player update reads these bytes. */
    u8 unknown_8C_to_97[12];
    s32 turnTimer, itemModel, turnDirection;
    M649PlayerConfig config;
} M649PlayerState;

typedef struct M649PlayerWork_s {
    M649PlayerState state;
    s32 player, character, phase;
    /* Cleared at character setup; no player update reads these bytes. */
    u8 unknown_F0_to_F3[4];
    s32 timer; /* Number of object updates since the character was created. */
    s32 volumeEnabled;
} M649PlayerWork;

typedef struct M649PlayerMotionParam_s {
    u16 motion;
    s16 overlay;
    float blend;
    u32 attr;
} M649PlayerMotionParam;

typedef struct M649ComputerParam_s {
    float firstDelay, delay, randomDelay, error;
    s32 targetScore;
} M649ComputerParam;

extern OMOBJ *lbl_1_bss_334;

extern OMOBJ *lbl_1_bss_324[4];

void fn_1_5094(OMOBJ *object);

void fn_1_51FC(OMOBJ *object);

void fn_1_5044(OMOBJ *object);

extern void fn_1_1338(s32 player, u32 first, u32 second);

extern s32 lbl_1_bss_310;

extern void *lbl_1_bss_314[4];

void fn_1_512C(void *state);

extern Vec lbl_1_data_128[4];

extern float lbl_1_data_158[4];

void fn_1_64B4(void *state);

void fn_1_6310(void *state);

extern s32 lbl_1_data_1B4[4], lbl_1_data_C8[6];

extern M649PlayerConfig lbl_1_data_174;

extern s32 fn_1_1210(s32 resource);

extern s32 fn_1_1218(s32 camera);

extern s32 fn_1_48DC(s32 resource);

void fn_1_6068(OMOBJ *object, s32 index);

void fn_1_5564(OMOBJ *object);

void fn_1_6858(void *state);

void fn_1_5FD8(void *state);

s32 fn_1_56A4(void *state);

s32 fn_1_57AC(void *state);

s32 fn_1_5B04(void *state);

s32 fn_1_5CEC(void *state);

void fn_1_6000(OMOBJ *object);

void fn_1_5E04(OMOBJ *object);

void fn_1_5F7C(OMOBJ *object);

extern void fn_1_88FC(u32 camera, s32 mode);

void fn_1_5FF4(void *state, s32 motion, f32 speed);

extern float fn_1_4AF4(float current, float target, float speed);

extern float fn_1_4C10(float current, float target);

extern void fn_1_1368(s32 player);

extern BOOL fn_1_1388(void);

extern s32 fn_1_127C(u32 camera);

s32 fn_1_6244(OMOBJ *object);

extern s32 fn_1_3630(s32 camera, Vec *position);

extern s32 fn_1_3850(s32 camera, Vec *position);

extern void fn_1_4788(s32 camera, s32 effect, s32 handle, Vec *position, s32 pan, s32 volume);

void fn_1_688C(void *state, s32 modelId);

extern M649PlayerMotionParam lbl_1_data_E0[6];

extern void fn_1_88FC(u32 camera, s32 direction);

void fn_1_68E4(s32 player);

extern M649ComputerParam lbl_1_data_1C4[4];

extern s32 fn_1_8D20(s32 camera, s32 direction, s32 frames);

extern s32 fn_1_1244(void);

extern s32 fn_1_A644(s32 player);

extern OMOBJ *lbl_1_bss_324[];

/* fn_1_51FC registers this player's state and sets its starting position and facing. */
static inline void m649_local_5130_051FC(M649PlayerState *state)
{
    M649PlayerConfig *config = &state->config;
    s32 i, player;
    for (i = 0; i < 4; i++) {
        if (lbl_1_bss_314[i] == NULL) {
            lbl_1_bss_314[i] = state;
            player = state->player;
            state->position = lbl_1_data_128[player];
            state->velocity.x = state->velocity.y = state->velocity.z = 0.0f;
            state->rotation.y = lbl_1_data_158[player];
            break;
        }
    }
}

/* Turn handling queues the motion and playback speed applied later by fn_1_6000. */
static inline void m649_local_5FF4_06244(M649PlayerState *state, s32 motion, float speed)
{
    state->requestedMotion = motion;
    state->motionSpeed = speed;
}

/* Interpolates points for fn_1_BE0 and the per-frame camera callback fn_1_C78. */
void fn_1_4ECC(Point3d *start, Point3d *end, Point3d *out, f32 t)
{
    out->x = start->x + (t * (end->x - start->x));
    out->y = start->y + (t * (end->y - start->y));
    out->z = start->z + (t * (end->z - start->z));
}

/* fn_1_A0 creates the four player objects and their shared update manager here. */
void fn_1_4F24(HUPROCESS *manager)
{
    s32 i;
    OMOBJ *object;
    lbl_1_bss_334 = omAddObjEx(manager, 14, 0, 0, -1, fn_1_5094);
    fn_1_5044(lbl_1_bss_334);
    omMakeGroupEx(manager, 0, 4);
    for (i = 0; i < 4; i++) {
        object = lbl_1_bss_324[i] = omAddObjEx(manager, 10, 2, 6, 0, fn_1_51FC);
        object->work[0] = i;
        object->work[1] = i;
        object->work[2] = i;
        fn_1_1338(i, i, GwPlayerConf[i].charNo);
    }
}

/* fn_1_BB0 calls this during the sequence callback to release every character model. */
void fn_1_5020(void)
{
    CharModelKill(-1);
}

/* fn_1_4F24 calls this when it creates the player manager, clearing its four state slots. */
void fn_1_5044(OMOBJ *object) {
    s32 playerIndex;

    playerIndex = 0;
    while (playerIndex < 4) {
        *(s32 *) ((u8 *) &lbl_1_bss_314 + (playerIndex * 4)) = 0;
        playerIndex += 1;
    }
    lbl_1_bss_310 = 0;
}

/* fn_1_4F24 installs this as the manager callback; it updates each registered state per frame. */
void fn_1_5094(OMOBJ *object) {
    void *playerState;
    s32 playerIndex;
    if ((s32) lbl_1_bss_310 == 0)
    {
        lbl_1_bss_310 = 1;
    }
    playerIndex = 0;
    while (playerIndex < 4)
    {
        playerState = * (void * *)((u8 *) & lbl_1_bss_314 + (playerIndex * 4));
        if (playerState != NULL)
        {
            fn_1_512C(playerState);
            ((M649PlayerState *)playerState)->updateCount =
                ((M649PlayerState *)playerState)->updateCount + 1;
        }
        playerIndex += 1;
    }
}

/* fn_1_5094 calls this for each registered player; this update leaves the state unchanged. */
void fn_1_512C(void *state)
{
}

/* Assigns this player's starting position and facing from its player slot. */
/* Registers a player state and assigns its starting position and facing. */
void fn_1_5130(void *state)
{
    M649PlayerConfig *config = &((M649PlayerState *)state)->config;
    s32 i, player;
    for (i = 0; i < 4; i++) {
        if (lbl_1_bss_314[i] == NULL) {
            lbl_1_bss_314[i] = ((M649PlayerState *)state);
            player = ((M649PlayerState *)state)->player;
            ((M649PlayerState *)state)->position = lbl_1_data_128[player];
            ((M649PlayerState *) state)->velocity.x =
                ((M649PlayerState *) state)->velocity.y =
                    ((M649PlayerState *) state)->velocity.z = 0.0f;
            ((M649PlayerState *)state)->rotation.y = lbl_1_data_158[player];
            break;
        }
    }
}

/* The object manager calls this to create a player's model, motions, and initial state. */
void fn_1_51FC(OMOBJ *object)
{
    M649PlayerState *state;
    M649PlayerWork *work;
    s32 i, character, model;
    M649PlayerConfig *config;
    s32 player, camera, index;
    char *hook;
    s32 motion;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M649PlayerWork), HU_MEMNUM_OVL);
    object->data = work;
    memset(work, 0, sizeof(M649PlayerWork));
    object->stat |= OM_STAT_MODELPAUSE;
    state = &work->state;
    player = work->player = object->work[0];
    character = work->character = GwPlayerConf[player].charNo;
    camera = object->work[1];
    state->camera = camera;
    index = object->work[2];
    state->previousPlayer = state->player = index;
    state->controlEnabled = 1;
    state->computer = GwPlayerConf[player].type;
    state->difficulty = GwPlayerConf[player].comDif & 3;
    state->computerPhase = 0;
    model = CharModelCreate(character, 8);
    object->mdlId[0] = model;
    Hu3DModelCameraSet(model, fn_1_1218(camera));
    Hu3DModelLayerSet(model, 4);
    Hu3DModelShadowSet(model);
    CharEffectLayerSet(4);
    state->itemModel = fn_1_48DC(fn_1_1210(lbl_1_data_1B4[player]));
    Hu3DModelCameraSet(state->itemModel, fn_1_1218(camera));
    Hu3DModelLayerSet(state->itemModel, 4);
    Hu3DModelShadowSet(state->itemModel);
    hook = CharModelItemHookGet(character, 8, 0);
    Hu3DModelHookSet(model, hook, state->itemModel);
    for (i = 0; i < 6; i++) {
        motion = CharMotionCreate(character, lbl_1_data_C8[i]);
        object->mtnId[i] = motion;
    }
    CharMotionDataClose(character);
    state->currentMotion = -1;
    fn_1_6068(object, 2);
    config = &state->config;
    *config = lbl_1_data_174;
    config->height = CharModelHeightGet(character);
    m649_local_5130_051FC(state);
    work->phase = 0;
    work->volumeEnabled = 1;
    object->objFunc = fn_1_5564;
}

/* Installed by fn_1_51FC as the per-frame callback for player phases and model updates. */
void fn_1_5564(OMOBJ *object)
{
    M649PlayerState *state;
    M649PlayerWork *work = object->data;
    state = &work->state;
    fn_1_6858(state);
    fn_1_5FD8(state);
    switch (work->phase) {
    case 0:
        if (fn_1_56A4(state)) { work->phase++; state->phase = 0; }
        break;
    case 1:
        if (fn_1_57AC(state)) { work->phase++; state->phase = 0; }
        break;
    case 2:
        if (fn_1_5B04(state)) { work->phase++; state->phase = 0; }
        break;
    case 3:
        if (fn_1_5CEC(state)) { work->phase++; state->phase = 0; }
        break;
    }
    fn_1_6000(object);
    PSVECAdd(&state->position, &state->velocity, &state->position);
    fn_1_5E04(object);
    fn_1_5F7C(object);
}

/* Before MAIN, stamps immediately for nonzero camera slots or sends a scripted A press after
 * 90 updates for slot 0, then waits for MAIN. */
s32 fn_1_56A4(void *state)
{
    M649PlayerConfig *config = &((M649PlayerState *)state)->config;
    s32 done = 0;
    switch (((M649PlayerState *)state)->phase) {
    case 0:
        if (((M649PlayerState *)state)->camera != 0) {
            ((M649PlayerState *)state)->phase = 2;
            ((M649PlayerState *)state)->timer = 0;
            fn_1_88FC(((M649PlayerState *)state)->camera, 1);
            break;
        }
        ((M649PlayerState *)state)->phase++;
        ((M649PlayerState *)state)->timer = 0;
    case 1:
        if (++((M649PlayerState *)state)->timer < 90) {
            break;
        }
        ((M649PlayerState *)state)->phase++;
        ((M649PlayerState *)state)->timer = 0;
        ((M649PlayerState *)state)->buttonsDown |= (1 << 8);
    case 2:
        if (MgSeqModeGet() != 5) {
            break;
        }
        ((M649PlayerState *)state)->phase++;
        ((M649PlayerState *)state)->timer = 0;
    default:
        done = 1;
        break;
    }
    fn_1_6310(((M649PlayerState *)state));
    return done;
}

/* Reads controller or computer input during play and releases the held item after play;
 * finish-state branches return without a value even though fn_1_5564 tests the result. */
s32 fn_1_57AC(void *state)
{
    M649PlayerConfig *config = &((M649PlayerState *)state)->config;
    s32 controllerIndex;
    M649PlayerWork *work;
    s32 done = 0;
    M649PlayerWork *input;
    char *hook;
    float x, y;
    switch (((M649PlayerState *)state)->phase) {
    case 0:
        ((M649PlayerState *)state)->phase++;
        ((M649PlayerState *)state)->timer = 0;
    case 1:
        if (MgSeqModeGet() == MGSEQ_MODE_FINISH) {
            ((M649PlayerState *)state)->phase++;
            ((M649PlayerState *)state)->timer = 0;
            break;
        }
        if (((M649PlayerState *)state)->computer) {
            fn_1_64B4(((M649PlayerState *)state));
            break;
        }
        if (((M649PlayerState *)state)->controlEnabled) {
            input = (M649PlayerWork *)((M649PlayerState *)state);
            controllerIndex = GwPlayerConf[input->player].padNo;
            x = HuPadStkXf[controllerIndex];
            y = -HuPadStkYf[controllerIndex];
            ((M649PlayerState *)state)->buttons = HuPadBtn[controllerIndex];
            ((M649PlayerState *)state)->buttonsDown = HuPadBtnDown[controllerIndex];
            ((M649PlayerState *)state)->stickAngle = 180.0 * (atan2(x, y) / M_PI);
            ((M649PlayerState *)state)->stickMagnitude = sqrtf(x * x + y * y);
        }
        break;
    case 2:
        if (MgSeqModeGet() == MGSEQ_MODE_MAIN) { break; }
        if (((M649PlayerState *)state)->controlEnabled) {
            work = (M649PlayerWork *)((M649PlayerState *)state);
            hook = CharModelItemHookGet(work->character, 8, 0);
            CharModelHookDustCreate(work->character, hook);
        }
        ((M649PlayerState *)state)->phase++;
        ((M649PlayerState *)state)->timer = 0;
        /* The finish branches store this long turn timer but return before turn handling uses
         * it. */
        ((M649PlayerState *)state)->turnTimer = 6000;
    case 3:
        if (++((M649PlayerState *)state)->timer < 10) { return; }
        ((M649PlayerState *)state)->phase++;
        ((M649PlayerState *)state)->timer = 0;
    default:
        done = 1;
        return;
    }
    fn_1_6310(((M649PlayerState *)state));
    return done;
}

/* After play, turns toward the front and waits for WINNER to mark the player ready;
 * if others are unready, this still completes on its next update. */
s32 fn_1_5B04(void *state)
{
    M649PlayerConfig *config = &((M649PlayerState *)state)->config;
    s32 done = 0;
    M649PlayerWork *work;
    switch (((M649PlayerState *)state)->phase) {
    case 0:
        ((M649PlayerState *)state)->phase++;
        ((M649PlayerState *)state)->timer = 0;
        ((M649PlayerState *)state)->motionActive = 0;
        fn_1_5FF4(((M649PlayerState *)state), config->finishIdleMotion, 1.0f);
    case 1:
        if (++((M649PlayerState *)state)->timer < 60) { break; }
        ((M649PlayerState *)state)->phase++;
        ((M649PlayerState *)state)->timer = 0;
    case 2:
        ((M649PlayerState *) state)->rotation.y =
            fn_1_4AF4(((M649PlayerState *) state)->rotation.y, 0.0f, 0.1f);
        if (fabs(fn_1_4C10(((M649PlayerState *)state)->rotation.y, 0.0f)) > 3.0) {
            fn_1_5FF4(((M649PlayerState *)state), config->moveMotion, 1.0f);
            break;
        }
        fn_1_5FF4(((M649PlayerState *)state), config->finishIdleMotion, 1.0f);
        if (MgSeqModeGet() != 8) { break; }
        ((M649PlayerState *)state)->phase++;
        ((M649PlayerState *)state)->timer = 0;
        if (((M649PlayerState *)state)->controlEnabled) {
            work = (M649PlayerWork *)((M649PlayerState *)state);
            fn_1_1368(work->player);
        }
        if (!fn_1_1388()) { break; }
        ((M649PlayerState *)state)->phase++;
        ((M649PlayerState *)state)->timer = 0;
    default:
        done = 1;
    }
    return done;
}

/* fn_1_5564 calls this in the results phase to play the win/loss motion and finish its wait. */
s32 fn_1_5CEC(void *state)
{
    M649PlayerConfig *config = &((M649PlayerState *)state)->config;
    M649PlayerWork *work;
    s32 done = 0;
    switch (((M649PlayerState *)state)->phase) {
    case 0:
        if (((M649PlayerState *)state)->controlEnabled) {
            work = (M649PlayerWork *)((M649PlayerState *)state);
            work->volumeEnabled = 0;
            CharModelVoiceVolSet(work->character, 127);
        }
        if (fn_1_127C(((M649PlayerState *)state)->camera) > 0) {
            fn_1_5FF4(((M649PlayerState *)state), config->winMotion, 1.0f);
        } else {
            fn_1_5FF4(((M649PlayerState *)state), config->loseMotion, 1.0f);
        }
        ((M649PlayerState *)state)->phase++;
        ((M649PlayerState *)state)->timer = 0;
        break;
    case 1:
        if (++((M649PlayerState *)state)->timer < 90) {
            break;
        }
        ((M649PlayerState *)state)->phase++;
        ((M649PlayerState *)state)->timer = 0;
    default:
        done = 1;
        break;
    }
    return done;
}

/* fn_1_5564 calls this each frame to check motion completion and update voice pan and volume. */
void fn_1_5E04(OMOBJ *object)
{
    M649PlayerState *state;
    M649PlayerWork *work = object->data;
    s32 camera;
    state = &work->state;
    camera = fn_1_1218(state->camera);
    work->timer++;
    if (state->motionActive) {
        if (fn_1_6244(object)) {
            state->motionActive = 0;
            state->motionTimer = 0;
        }
    } else {
        state->motionTimer = 0;
    }
    CharModelVoicePanSet(work->character, fn_1_3630(camera, &state->position));
    if (work->volumeEnabled) {
        CharModelVoiceVolSet(work->character, fn_1_3850(camera, &state->position));
    }
}

/* Plays a character sound with camera-based pan and volume, then tracks its live position. */
void fn_1_5EE0(OMOBJ *object, s32 effect)
{
    M649PlayerState *state;
    M649PlayerWork *work = object->data;
    s32 camera, handle;
    state = &work->state;
    camera = fn_1_1218(state->camera);
    handle = CharFXPlayVolPan(work->character, effect,
        fn_1_3850(camera, &state->position), fn_1_3630(camera, &state->position));
    fn_1_4788(camera, effect, handle, &state->position, 1, 1);
}

/* fn_1_5564 calls this each frame to copy the player's current position and rotation to its
 * model. */
void fn_1_5F7C(OMOBJ *object)
{
    M649PlayerWork *work;
    M649PlayerState *state;
    s32 model;
    work = object->data;
    state = &work->state;
    model = object->mdlId[0];
    fn_1_688C(state, model);
}

/* fn_1_5564 resets the pending motion before its phase logic and motion update. */
void fn_1_5FD8(void *state) {
    ((M649PlayerState *)state)->requestedMotion = -1;
    ((M649PlayerState *)state)->motionSpeed = 1.0f;
}

/* Player phase handlers call this to queue a motion and speed for fn_1_6000. */
void fn_1_5FF4(void *state, s32 motion, f32 speed) {
    ((M649PlayerState *)state)->requestedMotion = motion;
    ((M649PlayerState *)state)->motionSpeed = speed;
}

/* fn_1_5564 calls this each frame to apply a queued motion to the player's model. */
void fn_1_6000(OMOBJ *object)
{
    M649PlayerWork *work = object->data;
    M649PlayerState *state = &work->state;
    if (state->requestedMotion != -1) {
        fn_1_6068(object, state->requestedMotion);
        Hu3DMotionSpeedSet(object->mdlId[0], state->motionSpeed);
    }
}

/* fn_1_51FC starts the first motion and fn_1_6000 applies later selections here. */
/* A changed selection uses zero blend time for the first motion and the table's blend
* thereafter; its configured overlay is applied when present. */
void fn_1_6068(OMOBJ *object, s32 index)
{
    M649PlayerState *state;
    M649PlayerWork *work = object->data;
    u32 overlayAttr;
    u32 attr;
    s32 model, motion, overlay;
    float blend;
    state = &work->state;
    overlayAttr = 0;
    model = object->mdlId[0];
    if (state->currentMotion != index && index < 6) {
        if (state->currentMotion < 0) {
            blend = 0.0f;
        } else {
            blend = 60.0f * lbl_1_data_E0[index].blend;
        }
        state->currentMotion = index;
        state->motionTime = 0;
        motion = object->mtnId[lbl_1_data_E0[index].motion];
        attr = lbl_1_data_E0[index].attr;
        /* HU3D_MOTATTR makes this entire mask clear motion flags, including the bits named as model
         * attributes here. */
        Hu3DModelAttrReset(model, HU3D_MOTATTR | HU3D_ATTR_DISPOFF | HU3D_ATTR_SHADOW |
                                   HU3D_ATTR_TOON_MAP | HU3D_ATTR_MOT_EXEC);
        overlay = lbl_1_data_E0[index].overlay;
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

/* fn_1_5E04 uses this to detect when the character's current motion and blend have ended. */
s32 fn_1_6244(OMOBJ *object)
{
    M649PlayerWork *work = object->data;
    if (CharMotionEndCheck(work->character) != 0 &&
        CharMotionShiftIDGet(work->character) < 0) {
        return 1;
    }
    return 0;
}

/* Reports whether the character has no active motion blend. */
s32 fn_1_62B0(OMOBJ *object)
{
    M649PlayerWork *work = object->data;
    s32 done = 0;
    if (CharMotionShiftIDGet(work->character) < 0) {
        done = 1;
    }
    return done;
}

/* fn_1_56A4 and fn_1_57AC call this to animate turns and rotate the player toward the chosen
 * side. */
void fn_1_6310(void *state)
{
    M649PlayerConfig *config = &((M649PlayerState *)state)->config;
    float angle;
    if (((M649PlayerState *)state)->turnTimer == 0) {
        if (((M649PlayerState *)state)->motionActive == 0) {
            m649_local_5FF4_06244(((M649PlayerState *)state), config->idleMotion, 1.0f);
        }
        if (((M649PlayerState *)state)->currentMotion != config->turnMotion) {
            ((M649PlayerState *) state)->rotation.y =
                fn_1_4AF4(((M649PlayerState *) state)->rotation.y, 0.0f, 0.3f);
        }
        if (((M649PlayerState *)state)->buttonsDown & ((1 << 8) | (1 << 9))) {
            if (((M649PlayerState *)state)->buttonsDown & (1 << 8)) {
                ((M649PlayerState *)state)->turnDirection = 1;
            } else {
                ((M649PlayerState *)state)->turnDirection = 0;
            }
            /* Mark the selection as idle so this press restarts the turn motion even if it was
             * selected. */
            ((M649PlayerState *)state)->currentMotion = config->idleMotion;
            m649_local_5FF4_06244(((M649PlayerState *)state), config->turnMotion, 1.0f);
            ((M649PlayerState *)state)->motionActive = 1;
            ((M649PlayerState *)state)->turnTimer = 24;
        }
    } else {
        if (((M649PlayerState *)state)->turnTimer == 6) {
            fn_1_88FC(((M649PlayerState *) state)->camera,
                      ((M649PlayerState *) state)->turnDirection);
            fn_1_68E4(((M649PlayerState *)state)->camera);
        }
        angle = ((M649PlayerState *)state)->turnDirection == 0 ? 270.0 : 90.0;
        ((M649PlayerState *) state)->rotation.y =
            fn_1_4AF4(((M649PlayerState *) state)->rotation.y, angle, 0.3f);
        ((M649PlayerState *)state)->turnTimer--;
    }
}

/* fn_1_57AC calls this for computer-controlled players to time left/right button presses. */
void fn_1_64B4(void *state)
{
    M649ComputerParam *param = &lbl_1_data_1C4[((M649PlayerState *)state)->difficulty];
    s32 limit = -24;
    s32 left, right, direction, targetScore, score;
    float error, progress;
    switch (((M649PlayerState *)state)->computerPhase) {
    case 0:
        ((M649PlayerState *)state)->computerPhase++;
        ((M649PlayerState *) state)->computerTimer =
            (u32) (param->firstDelay +
                   param->randomDelay * (1.52587890625e-05 * (float) (s32) (frand() & 0xFFFF)));
    case 1:
        if (--((M649PlayerState *)state)->computerTimer > 0) { break; }
        left = fn_1_8D20(((M649PlayerState *)state)->camera, 0, abs(limit));
        left -= 18;
        right = fn_1_8D20(((M649PlayerState *)state)->camera, 1, abs(limit));
        right -= 18;
        if (left <= limit && right <= limit) { break; }
        direction = left < right ? 0 : 1;
        if (left <= limit) { direction = 1; }
        if (right <= limit) { direction = 0; }
        if (direction == 0) {
            ((M649PlayerState *)state)->computerTimer = left;
            ((M649PlayerState *)state)->computerDirection = 270.0f;
        } else {
            ((M649PlayerState *)state)->computerTimer = right;
            ((M649PlayerState *)state)->computerDirection = 90.0f;
        }
        /* Computer targeting uses this call's result although the timer wrapper defines no return
         * value. */
        progress = (float)fn_1_1244() / 1800.0f;
        targetScore = (s32)((float)param->targetScore * (1.0 - progress)) + 1;
        score = fn_1_A644(((M649PlayerState *)state)->camera);
        error = param->error;
        if (score < targetScore - 1) { error *= 0.5; }
        if (score > targetScore + 1) { error *= 2.5; }
        ((M649PlayerState *)state)->computerTimer += (s32)(20.0 *
            (error * (3.0517578125e-05 * (float)((u16)frand() - 32768))));
        ((M649PlayerState *)state)->computerPhase++;
    case 2:
        if (--((M649PlayerState *)state)->computerTimer > 0) { break; }
        if (((M649PlayerState *)state)->computerDirection < 180.0f) {
            ((M649PlayerState *)state)->buttonsDown = PAD_BUTTON_A;
        } else {
            ((M649PlayerState *)state)->buttonsDown = PAD_BUTTON_B;
        }
        ((M649PlayerState *)state)->computerPhase--;
        ((M649PlayerState *)state)->computerTimer = (u32)(param->delay + param->randomDelay *
            (1.52587890625e-05 * (float)(s32)(frand() & 0xFFFF)));
        break;
    }
}

/* fn_1_5564 clears button and stick values before the current phase updates them. */
void fn_1_6858(void *state) {
    ((M649PlayerState *)state)->buttonsDown = 0;
    ((M649PlayerState *)state)->buttons = 0;
    ((M649PlayerState *)state)->stickAngle = 0.0f;
    ((M649PlayerState *)state)->stickMagnitude = 0.0f;
}

/* fn_1_5F7C calls this each frame to apply a player's position and rotation to its model. */
void fn_1_688C(void *state, s32 modelId) {
    Hu3DModelPosSetV((s16) modelId, &((M649PlayerState *)state)->position);
    Hu3DModelRotSetV((s16) modelId, &((M649PlayerState *)state)->rotation);
    ((M649PlayerState *)state)->motionTime++;
}

/* fn_1_6310 requests rumble for this player during a turn, or all four slots in mode 2.
* The engine acts only for human players with rumble enabled while the wipe overlay is disabled;
* fade-in, fade-out, and the completed fade-out screen suppress the request. */
void fn_1_68E4(s32 player)
{
    if (MgSeqModeGet() == 2) {
        for (player = 0; player < 4; player++) {
            omVibrate(player, 10, 20, 0);
        }
    } else {
        omVibrate(player, 10, 20, 0);
    }
}

/* prop.c calls this to get the character assigned to the requested player group. */
s32 fn_1_6958(s32 player)
{
    OMOBJ *object = lbl_1_bss_324[player];
    M649PlayerWork *work = object->data;
    return work->character;
}
