/* Controls the Banana Shake prop actors, their scripted movement, and attached effects. */
static inline void DebugValue(int value)
{
}

#include "dolphin/math.h"

#include "dolphin/gx.h"

#include "dolphin/os.h"

#include "game/main.h"

#include "game/object.h"

#include "game/hu3d.h"

#include "game/gamework.h"

#include "game/audio.h"

#include "game/memory.h"

#include "game/data.h"

#include "game/hsfex.h"

#include "game/charman.h"

#include "game/frand.h"

#include "game/mg/seqman.h"

#include "msm_se.h"

#include <string.h>

#define MSM_SE_M675_2225 2225
#define MSM_SE_M675_2226 2226
#define MSM_SE_M675_2227 2227
#define MSM_SE_M675_2228 2228
#define MSM_SE_M675_2229 2229

typedef struct M675EmitterRecord M675EmitterRecord;

typedef struct M675PropMotionParameter {
    s32 resource; /* Motion resource index; -1 skips this slot and -2 ends the list. */
    f32 blend; /* Blend duration multiplier used when switching from another motion. */
    u32 attributes; /* Motion attributes applied when this motion starts. */
} M675PropMotionParameter;

typedef struct M675PropParameter {
    s32 count; /* Number of instances created from this entry. */
    s32 linkModel; /* Reuse an existing model when nonzero. */
    s32 type; /* Category used by the prop lookup table. */
    s16 firstId; /* ID assigned to the first instance. */
    s16 layer; /* Render layer for each instance's model. */
    u32 attributes; /* Model attributes enabled at creation. */
    s32 shadowMode; /* Selects no shadow, model shadow, or shadow-map reception. */
    s32 modelResource; /* Model data resource, or -1 when the prop has no model. */
    M675PropMotionParameter * motions; /* Motion resource and playback settings. */
    Vec * position; /* Optional initial world position. */
    f32 rotationY; /* Initial yaw in degrees. */
    OMOBJ_FUNC update; /* Per-frame callback assigned to each instance. */
} M675PropParameter;

typedef struct M675Collision M675Collision;

struct M675Collision {
    /* Collision enable flags, mask, and effect category. */
    u32 flags, mask, type;
    s32 index; /* Owner-supplied record index. */
    s32 shapeType; /* Collision shape selector: 0 for sphere, 10 for ring, 11 for boundary. */
    OMOBJ *owner;
    void *excludedOwner;
    OMOBJ_FUNC update;
    s32 (*contact)(M675Collision *, M675Collision *);
    Vec *position, *velocity;
    Vec positionCopy;
    f32 radius;
    f32 yieldOnContact; /* Nonzero makes this sphere move against a zero-valued sphere. */
    s32 handled;
};

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
    s32 motion, motionFrame; /* Selected motion and transform-update count since that motion was
                              * selected. */
    s32 requestedMotion; /* Motion queued by the current frame's behavior. */
    f32 motionSpeed; /* Playback speed for the queued motion. */
} M675ActorState;

typedef struct M675PropState {
    M675ActorState actor;
    /* Script phase and its frame timer or repeated-action counter; countdown and desired yaw. */
    s32 phase;
    s32 phaseTimer;
    f32 countdown; /* Remaining frames in a timed action. */
    f32 targetAngle;
    /* World-space destination and sub-action progress. */
    Vec targetPosition;
    s32 actionPhase;
    s32 actionTimer; /* Elapsed frames for the current action. */
    s32 soundTimer; /* Frames until the next repeated sound. */
    /* Unused bytes cleared when the prop is created. */
    u8 unknown_0D0_to_0D3[4];
    s32 movementPhase; /* Substate of a turn, walk, or jump routine. */
    s32 movementTimer; /* Frames remaining or elapsed in that movement substate. */
    /* Unused bytes cleared when the prop is created. */
    u8 unknown_0DC_to_0FB[32];
    s32 motionActive; /* Nonzero while the selected motion is being tracked. */
    s32 motionTimer; /* Cleared when motion tracking is inactive or the motion and blend have
                      * ended. */
    /* Unused bytes cleared when the prop is created. */
    u8 unknown_104_to_107[4];
    s32 airborne; /* Whether display positioning uses the airborne depth bands. */
    /* Unused bytes cleared when the prop is created. */
    u8 unknown_10C_to_127[28];
    s32 positionSyncEnabled; /* Follows the paired sequence actor's position when set. */
    Vec displayOffset; /* Temporary offset added when placing the model. */
    f32 movementSoundDistance; /* Accumulated requested walk speed, used to space walking sounds. */
    s32 effectRegistrationActive; /* Nonzero enables drawing the prop's registered ground ring. */
    f32 effectRegistrationRadius; /* Ground-ring radius in world units. */
    Vec * effectRegistrationPosition; /* Position shared with the effect manager. */
    /* Unused bytes cleared when the prop is created. */
    u8 unknown_148_to_1B3[108];
} M675PropState;

typedef struct M675PropWork {
    M675PropState state;
    M675PropParameter * parameters;
    s32 id;
    s32 updateCount;
    Vec initialPosition;
    /* Outer sequence phase, delayed-start frames, and phase timer. */
    s32 sequencePhase;
    s32 delay;
    s32 sequenceTimer;
    /* Unused bytes cleared when the prop is created. */
    u8 unknown_1D8_to_1DB[4];
    /* Sequence selection values used by the prop callbacks. */
    union {
        s32 controlValue;
        s32 frameCount;
    };
    union {
        s32 secondaryControlValue;
        s32 lastRecordedFrame;
    };
    /* Bounce deformation scale and angle. */
    f32 bounceScale;
    f32 bounceAngle;
    /* Unused bytes cleared when the prop is created. */
    u8 unknown_1EC_to_283[152];
    /* Type 0's path recorder and types 2 through 4's emitters share this slot. */
    union {
        Vec *samples;
        M675EmitterRecord *records;
    };
    s32 soundHandle; /* Active sound-effect handle, or the stored inactive value. */
    s32 eventCount; /* Number of sequence events recorded for this prop. */
    s32 shuffleIndex; /* Current index in the shuffled effect-type list. */
    s32 effectActive; /* Whether the prop's triggered main/shape motion sequence is active. */
    f32 effectMotionSpeed; /* Shared main/shape motion playback-speed multiplier. */
    s32 alternateMotionStart; /* Main/shape motion start selector: zero uses frame 0, nonzero uses
                               * frame 15. */
    u8 shuffledEffectTypes[20]; /* Shuffled effect types emitted by this sequence. */
    Vec displayPosition; /* Ground-ring position; airborne props choose its Y from depth bands. */
} M675PropWork;

typedef M675PropWork M675PlaybackWork;
typedef M675PropWork M675EmitterWork;

typedef struct M675EmitterRecord {
    /* Effect category, visibility/interaction state, and animation timer. */
    s32 type, enabled, state;
    f32 timer;
    u32 unmappedWordAfterTimer; /* Cleared with the emitter record; no later access is shown. */
    /* Phase count, model slot, and source player index. */
    s32 phaseCounter, modelId, playerIndex;
    /* Current transform and per-frame movement/rotation increments. */
    Vec position, rotation, velocity, rotationDelta;
    /* Uniform scale, model height adjustment, and floor cutoff offset from world Y=200. */
    f32 scale, verticalOffset, heightLimit;
    /* Endpoints used when moving the effect toward a screen-space target. */
    Vec sourcePosition, projectedPosition;
    M675Collision collision;
} M675EmitterRecord;

typedef struct M675MotionObjectView_s {
    u8 unmappedHeaderBytes[64];
    s32 modelIdsAddress;
    s32 unmappedWordAfterModelIds;
    s32 unmappedWordBetweenModelIdsAndVector;
    s32 unmappedWordBeforeVector;
    HuVecF unmappedVectorBeforeGap;
    u8 unmappedBytesBetweenVectors[12];
    HuVecF unmappedVectorAfterGap;
    u8 unmappedBytesBeforeTail[4];
    s32 unmappedTailStart;
    s32 unmappedWordBeforeFloatPair;
    f32 unmappedFloatPairStart;
    f32 unmappedFloatPairEnd;
    s32 unmappedWordAfterFloatPair;
    s32 unmappedMiddleTailWord;
    s32 unmappedWordBeforeTailEnd;
    f32 unmappedTailEnd;
} M675MotionObjectView;

typedef void (*M675MotionEmitterUpdate)(M675EmitterRecord *record);

typedef s32 (*M675PhaseCallback)(M675PropWork *work);

OMOBJ *lbl_1_bss_534;
OMOBJ *lbl_1_bss_334[128];
OMOBJ *lbl_1_bss_330;
OMOBJ *lbl_1_bss_32C;
OMOBJ *lbl_1_bss_328;
OMOBJ *lbl_1_bss_324;
s32 lbl_1_bss_320;
s32 lbl_1_bss_31C;
s32 lbl_1_bss_318;

extern void fn_1_6210(M675ActorState *sequence, s32 motion, f32 speed);

extern f32 fn_1_47DC(f32 start, f32 end, f32 speed);

extern f32 fn_1_48F8(f32 start, f32 end);

void fn_1_74FC(M675ActorState *work, s32 modelId);

extern void fn_1_F0B0(s32 playerIndex, float *x, float *y);

extern void fn_1_E4F8(Point3d *position, f32 radius, s32 count);

extern void fn_1_4BB4(Vec *start, Vec *end, f32 timer, Vec *position);

extern void fn_1_EF88(s32 playerIndex, s32 amount);

extern void fn_1_E918(void *registration);

extern f32 fn_1_49AC(Vec *start, Vec *end);

extern s32 fn_1_4C0C(Vec *start, Vec *end, f32 delta, f32 step,
    Vec *velocity);

extern void fn_1_3C70(s32 resource, Vec *position);

extern s32 fn_1_32EC(Vec *position);

extern void fn_1_E0F0(Vec *position);

extern s32 fn_1_A40(void);

extern s32 fn_1_B6C(void);

extern s32 fn_1_A50(void);

extern void fn_1_74C8(M675ActorState *work);

extern void fn_1_61F4(M675ActorState *work);

extern f32 fn_1_4A48(Vec *first, Vec *second);

s32 fn_1_46CC(s32 model, s32 resource);

int fn_1_45C4(int dataNum);

s32 fn_1_A38(s32 resource);

s32 fn_1_10170(M675Collision *registration);

void fn_1_E48(s16 model, char *name, s32 mode);

void fn_1_7944(OMOBJ *object);

OMOBJ *fn_1_7998(s32 type, s32 id);

BOOL fn_1_7D88(M675MotionObjectView *actor);

void fn_1_7F2C(OMOBJ *object);

void fn_1_8624(OMOBJ *obj);

s32 fn_1_C7E0(M675PropWork *work, f32 speed);

void fn_1_D278(OMOBJ *object);

void fn_1_D3DC(s32 type, s32 index, Vec *position);

void fn_1_7554(HUPROCESS *process);

void fn_1_80CC(OMOBJ *object);

void fn_1_80D0(OMOBJ *object);

void fn_1_9C3C(s32 playerIndex, HuVecF *destination);

void fn_1_9CAC(OMOBJ *object);

void fn_1_9E18(OMOBJ *object);

void fn_1_9F84(OMOBJ *object);

void fn_1_A0F0(OMOBJ *object);

void fn_1_A274(M675EmitterRecord *state);

void fn_1_A448(M675EmitterRecord *state);

void fn_1_A8EC(M675EmitterRecord *work);

void fn_1_ACB0(M675EmitterRecord *record);

void fn_1_AD98(M675EmitterRecord *record);

void fn_1_ADDC(OMOBJ *object);

s32 fn_1_C94C(M675PropWork *work, f32 speed);

s32 fn_1_CDF4(M675PropWork *work, f32 delta, f32 step,
    s32 count, s32 angleMode);

void fn_1_8634(OMOBJ *object);

void fn_1_8AC4(s32 value);

void fn_1_8B64(OMOBJ *object);

void fn_1_8CBC(OMOBJ *object);

void fn_1_8FE0(OMOBJ *object);

void fn_1_92A0(OMOBJ *object);

void fn_1_9AF0(OMOBJ *object, s32 resourceId, s32 collisionType,
    f32 verticalOffset, f32 heightLimit);

void fn_1_D148(OMOBJ *object);

void fn_1_D494(s32 type, s32 value);

void fn_1_81A8(OMOBJ *object);

void fn_1_8394(s32 frameOffset, Vec *first, Vec *second);

void fn_1_84C4(OMOBJ *object);

void fn_1_AF38(OMOBJ *object);

void fn_1_B114(OMOBJ *object);

s32 fn_1_B330(M675PropWork *work);

s32 fn_1_BA58(M675PropWork *work);

s32 fn_1_BBDC(M675PropWork *work);

s32 fn_1_BD84(M675PropWork *sequence);

s32 fn_1_BE60(M675PropWork *work);

s32 fn_1_C0B8(M675PropWork *work);

s32 fn_1_C29C(M675PropWork *work);

void fn_1_766C(void);

void fn_1_7670(OMOBJ *obj);

void fn_1_7680(OMOBJ *obj);

void fn_1_7684(HUPROCESS *process, M675PropParameter *param);

BOOL fn_1_7DF0(s32 type, s32 id, char *name, Vec *position, Vec *rotation);

void fn_1_7A20(OMOBJ *object, s32 index, s32 setShape);

void fn_1_7B58(OMOBJ *object, s32 index);

void fn_1_7C64(OMOBJ *object, s32 index);

void fn_1_7F64(OMOBJ *object);

void fn_1_8020(OMOBJ *object);

void fn_1_A5B4(M675EmitterRecord *state);

void fn_1_AA48(M675EmitterRecord *state);

s32 fn_1_D3BC(void);

s32 fn_1_D3CC(void);

/* Changes a prop's motion when its sequence requests a different motion index. */
static inline void M675SequenceMotionSet(OMOBJ *object, s32 index, s32 setShape)
{
    M675PropWork *state;
    M675PropMotionParameter *parameter;
    M675PropWork *work = object->data;
    s32 model, motion;

    state = work;
    if (state->state.actor.motion != index && index < 9) {
        motion = object->mtnId[index];
        work->updateCount = 0;
        state->state.actor.motionFrame = 0;
        if (motion == -1) {
            state->state.motionActive = 0;
            return;
        }
        parameter = &work->parameters->motions[index];
        model = object->mdlId[0];
        if (state->state.actor.motion < 0 || 0.0 == parameter->blend) {
            if (setShape) {
                Hu3DMotionShapeSet((s16)model, (s16)motion);
            }
            Hu3DMotionSet((s16)model, (s16)motion);
            Hu3DModelAttrSet((s16)model, parameter->attributes);
        } else {
            if (setShape) {
                Hu3DMotionShapeSet((s16)model, (s16)motion);
            }
            Hu3DMotionShiftSet((s16)model, (s16)motion, 0.0f,
                60.0f * parameter->blend, parameter->attributes);
        }
        state->state.actor.motion = index;
    }
}

/* Reads recorded position and rotation at the offset playback frame, wrapping and clamping the
 * index. */
static inline void M675PlaybackSamples(s32 frameOffset, Vec *first, Vec *second)
{
    OMOBJ *object = lbl_1_bss_330;
    M675PlaybackWork *work = object->data;
    Vec *samples = work->samples;
    s32 model = object->mdlId[0];
    s32 frame;

    frame = (s32) Hu3DMotionTimeGet(model);
    frame = frame + frameOffset;
    if (frame >= work->frameCount) {
        frame -= work->frameCount - 1;
    }
    if (frame < 0) {
        frame += work->frameCount - 1;
    }
    if (frame >= work->frameCount) {
        frame = work->frameCount - 1;
    }
    if (frame > work->lastRecordedFrame) {
        frame = work->lastRecordedFrame;
    }
    if (frame < 0) {
        frame = 0;
    }
    if (first != NULL) {
        *first = samples[frame * 2];
    }
    if (second != NULL) {
        *second = samples[frame * 2 + 1];
    }
}

/* Copies the playback prop's position and wraps its yaw into one turn. */
static inline void M675PlaybackApplyTransform(OMOBJ *object)
{
    void *data = object->data;
    M675PlaybackWork *work = data;
    s32 model = object->mdlId[0];

    work->state.actor.rotation.y = fmodf(work->state.actor.rotation.y, 360.0f);
    Hu3DModelPosSetV(model, &work->state.actor.position);
    Hu3DModelRotSetV(model, &work->state.actor.rotation);
}

/* Reports completion only after the main motion ends and no blend is pending. */
static inline s32 M675PlaybackMotionEnded(s32 model)
{
    if (Hu3DMotionEndCheck(model) && Hu3DMotionShiftIDGet(model) < 0) {
        return 1;
    } else {
        return 0;
    }
}

/* Finds the first live prop whose parameter type and instance ID match. */
static inline OMOBJ *M675MotionFind(s32 type, s32 mode)
{
    OMOBJ *object;
    OMOBJ **entry = lbl_1_bss_334;
    M675PropWork *work;
    s32 i;

    for (i = 0; i < 128; i++, entry++) {
        object = *entry;
        if (object != NULL) {
            work = object->data;
            if (work->parameters->type == type && work->id == mode) {
                return object;
            }
        }
    }
    return NULL;
}

/* Finds a live prop by parameter type and instance ID for sequence callbacks. */
static inline OMOBJ *M675B330Find(s32 type, s32 mode)
{
    OMOBJ *object;
    OMOBJ **entry = lbl_1_bss_334;
    M675PropWork *work;
    s32 i;

    for (i = 0; i < 128; i++, entry++) {
        object = *entry;
        if (object != NULL) {
            work = object->data;
            if (work->parameters->type == type && work->id == mode) {
                return object;
            }
        }
    }
    return NULL;
}

/* Reads recorded position and rotation at the offset playback frame, wrapping and clamping the
 * index. */
static inline void M675SequenceSample(s32 frameOffset, Vec *first, Vec *second)
{
    OMOBJ *object = lbl_1_bss_330;
    M675PlaybackWork *work = object->data;
    Vec *samples = work->samples;
    s32 model = object->mdlId[0];
    s32 frame;

    frame = (s32)Hu3DMotionTimeGet((s16)model);
    frame = frame + frameOffset;
    if (frame >= work->frameCount) {
        frame -= work->frameCount - 1;
    }
    if (frame < 0) {
        frame += work->frameCount - 1;
    }
    if (frame >= work->frameCount) {
        frame = work->frameCount - 1;
    }
    if (frame > work->lastRecordedFrame) {
        frame = work->lastRecordedFrame;
    }
    if (frame < 0) {
        frame = 0;
    }
    if (first != NULL) {
        *first = samples[frame * 2];
    }
    if (second != NULL) {
        *second = samples[frame * 2 + 1];
    }
}

/* Finds a live prop by configured type and instance ID for the named-part query. */
static inline OMOBJ *find_prop_inlined(s32 type, s32 id)
{
    OMOBJ *object;
    OMOBJ **entry = lbl_1_bss_334;
    M675PropWork *work;
    s32 i;

    for (i = 0; i < 128; i++, entry++) {
        object = *entry;
        if (object != NULL) {
            work = object->data;
            if (work->parameters->type == type && work->id == id) {
                return object;
            }
        }
    }
    return NULL;
}

/* Selects a prop motion and applies the configured blend and optional shape motion. */
static inline void MotionSetInline(OMOBJ *object, s32 index, s32 setShape)
{
    M675PropState *state;
    M675PropMotionParameter *parameter;
    M675PropWork *work = object->data;
    s32 model, motion;

    state = &work->state;
    if (state->actor.motion != index && index < 9) {
        motion = object->mtnId[index];
        work->updateCount = 0;
        state->actor.motionFrame = 0;
        if (motion == -1) {
            state->motionActive = 0;
            return;
        }
        parameter = &work->parameters->motions[index];
        model = object->mdlId[0];
        if (state->actor.motion < 0 || (0.0) == parameter->blend) {
            if (setShape) {
                Hu3DMotionShapeSet((s16) model, (s16) motion);
            }
            Hu3DMotionSet((s16) model, (s16) motion);
            Hu3DModelAttrSet((s16) model, parameter->attributes);
        } else {
            if (setShape) {
                Hu3DMotionShapeSet((s16) model, (s16) motion);
            }
            Hu3DMotionShiftSet((s16) model, (s16) motion, 0.0f,
                60.0f * parameter->blend, parameter->attributes);
        }
        state->actor.motion = index;
    }
}

/* Changes the shadow actor's motion using its configured blend settings. */
static inline void M675ShadowMotionSet(OMOBJ *object, s32 index, s32 setShape)
{
    M675PropWork *state;
    M675PropMotionParameter *entry;
    M675PropWork *work = object->data;
    s32 model, motion;

    state = work;
    if (state->state.actor.motion != index && index < 9) {
        motion = object->mtnId[index];
        work->updateCount = 0;
        state->state.actor.motionFrame = 0;
        if (motion == -1) {
            state->state.motionActive = 0;
            return;
        }
        entry = &work->parameters->motions[index];
        model = object->mdlId[0];
        if (state->state.actor.motion < 0 || 0.0 == entry->blend) {
            if (setShape) {
                Hu3DMotionShapeSet((s16)model, (s16)motion);
            }
            Hu3DMotionSet((s16)model, (s16)motion);
            Hu3DModelAttrSet((s16)model, entry->attributes);
        } else {
            if (setShape) {
                Hu3DMotionShapeSet((s16)model, (s16)motion);
            }
            Hu3DMotionShiftSet((s16)model, (s16)motion, 0.0f,
                60.0f * entry->blend, entry->attributes);
        }
        state->state.actor.motion = index;
    }
}

/* Finds a sequence actor by its configured type and instance ID. */
static inline OMOBJ *M675SequenceFind(s32 type, s32 mode)
{
    OMOBJ *object;
    OMOBJ **entry = lbl_1_bss_334;
    M675PropWork *work;
    s32 i;

    for (i = 0; i < 128; i++, entry++) {
        object = *entry;
        if (object != NULL) {
            work = object->data;
            if (work->parameters->type == type &&
                work->id == mode) {
                return object;
            }
        }
    }
    return NULL;
}

static inline void M675ShadowMotionSetPlain(OMOBJ *object, s32 index)
{
    M675ShadowMotionSet(object, index, 0);
}

static inline void M675SequenceMotionSetPlain(OMOBJ *object, s32 index)
{
    M675SequenceMotionSet(object, index, 0);
}

/* Clears playback tracking when its current motion and blend finish. */
static inline void M675PlaybackAdvanceMotion(OMOBJ *object)
{
    M675PropWork *work = object->data;
    M675PropWork *state = work;

    if (state->state.motionActive != 0) {
        if (M675PlaybackMotionEnded(object->mdlId[0])) {
            state->state.motionActive = 0;
            state->state.motionTimer = 0;
        }
    } else {
        state->state.motionTimer = 0;
    }
    work->updateCount++;
}

/* Sets the paired playback prop's bounce and dust request. */
static inline void M675PlaybackBounceRequestSet(s32 requested)
{
    OMOBJ *object;
    M675PropWork *work;

    object = M675SequenceFind(0, 2);
    if (object != NULL) {
        work = object->data;
        work->controlValue = requested;
    }
}

extern Vec lbl_1_data_43C;

Vec lbl_1_data_448 = {1500.0f, 1200.0f, -3000.0f};

Vec lbl_1_data_454 = {3000.0f, 3000.0f, -6500.0f};

Vec lbl_1_data_460 = {0.0f, 30.0f, 0.0f};

M675PropMotionParameter lbl_1_data_46C[10] = {
    {25, 0.20000000298023224f, HU3D_MOTATTR_NONE},
    {26, 0.20000000298023224f, HU3D_MOTATTR_NONE},
    {-2, 0.0f, HU3D_MOTATTR_NONE},
    {0, 0.0f, HU3D_MOTATTR_NONE},
    {0, 0.0f, HU3D_MOTATTR_NONE},
    {0, 0.0f, HU3D_MOTATTR_NONE},
    {0, 0.0f, HU3D_MOTATTR_NONE},
    {0, 0.0f, HU3D_MOTATTR_NONE},
    {0, 0.0f, HU3D_MOTATTR_NONE},
    {0, 0.0f, HU3D_MOTATTR_NONE}
};

M675PropMotionParameter lbl_1_data_4E4[10] = {
    {5, 0.20000000298023224f, HU3D_MOTATTR_LOOP},
    {10, 0.20000000298023224f, HU3D_MOTATTR_LOOP},
    {13, 0.20000000298023224f, HU3D_MOTATTR_LOOP},
    {6, 0.20000000298023224f, HU3D_MOTATTR_LOOP},
    {7, 0.20000000298023224f, HU3D_MOTATTR_NONE},
    {8, 0.20000000298023224f, HU3D_MOTATTR_NONE},
    {9, 0.20000000298023224f, HU3D_MOTATTR_NONE},
    {14, 0.20000000298023224f, HU3D_MOTATTR_NONE},
    {15, 0.20000000298023224f, HU3D_MOTATTR_LOOP},
    {-2, 0.0f, HU3D_MOTATTR_NONE}
};

M675PropParameter lbl_1_data_55C[11] = {
    {1, 0, 0, 0, 1, HU3D_MOTATTR_LOOP | HU3D_MOTATTR_SHAPE_LOOP, 0, 19, NULL, NULL, 0.0f, fn_1_80D0},
    {1, 0, 0, 1, 3, HU3D_MOTATTR_LOOP, 2, 20, NULL, NULL, 0.0f, fn_1_80CC},
    {1, 0, 0, 2, 2, HU3D_MOTATTR_LOOP | HU3D_MOTATTR_SHAPE_LOOP, 0, 21, NULL, &lbl_1_data_460, 0.0f, fn_1_8624},
    {4, 0, 1, 0, 2, HU3D_ATTR_NONE, 1, 24, lbl_1_data_46C, NULL, 0.0f, fn_1_8FE0},
    {2, 1, 6, 0, 4, HU3D_MOTATTR_LOOP, 0, 22, NULL, &lbl_1_data_448, 0.0f, fn_1_8B64},
    {5, 1, 6, 2, 4, HU3D_MOTATTR_LOOP, 0, 22, NULL, &lbl_1_data_454, 0.0f, fn_1_8B64},
    {1, 0, 2, 0, 5, HU3D_ATTR_NONE, 0, -1, NULL, NULL, 0.0f, fn_1_9CAC},
    {1, 0, 3, 0, 5, HU3D_ATTR_NONE, 0, -1, NULL, NULL, 0.0f, fn_1_9E18},
    {1, 0, 4, 0, 5, HU3D_ATTR_NONE, 0, -1, NULL, NULL, 0.0f, fn_1_9F84},
    {1, 0, 5, 0, 4, HU3D_ATTR_NONE, 0, 4, lbl_1_data_4E4, &lbl_1_data_43C, 0.0f, fn_1_ADDC},
    {-1, 0, 0, 0, 0, HU3D_ATTR_NONE, 0, 0, NULL, NULL, 0.0f, NULL}
};

s32 lbl_1_data_740[4] = {48, 56, 72, 80};

/* Creates the Banana Shake prop manager and each configured prop at mode startup. */
void fn_1_7554(HUPROCESS *process) {
    OMOBJ *manager;
    OMOBJ **entry;
    M675PropParameter *parameter;
    s32 index;
    void *managerData;

    entry = lbl_1_bss_334;
    index = 0;
    while (index < 128) {
        *entry = NULL;
        index += 1;
        entry += 1;
    }
    manager = omAddObjEx(process, 12, 0U, 0U, -1, fn_1_7670);
    lbl_1_bss_534 = manager;
    manager->stat |= OM_STAT_MODELPAUSE;
    managerData = HuMemDirectMallocNum(HEAP_HEAP, 8, HU_MEMNUM_OVL);
    manager->data = managerData;
    memset(managerData, 0, 8);
    lbl_1_bss_320 = 0;
    lbl_1_bss_31C = 0;
    lbl_1_bss_318 = 0;
    parameter = lbl_1_data_55C;
    while (parameter->count != -1) {
        fn_1_7684(process, parameter);
        parameter++;
    }
}

void fn_1_766C(void)
{
}

void fn_1_7670(OMOBJ *obj)
{
    obj->objFunc = fn_1_7680;
}

void fn_1_7680(OMOBJ *obj)
{
}

/* Builds the configured prop instances and loads their models and motions at startup. */
void fn_1_7684(HUPROCESS *process, M675PropParameter *param)
{
    s32 i;
    OMOBJ *object;
    s32 model;
    M675PropWork *work;
    M675PropState *state;
    M675PropMotionParameter *motion;
    s32 group;
    s32 resource;

    for (group = 0; group < param->count; group++) {
        object = omAddObjEx(process, 12, 1, 9, 0, param->update);
        fn_1_7944(object);
        work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M675PropWork), HU_MEMNUM_OVL);
        object->data = work;
        memset(work, 0, sizeof(M675PropWork));
        object->stat |= OM_STAT_MODELPAUSE;
        state = &work->state;
        work->parameters = param;
        work->id = param->firstId + group;
        state->actor.motion = -1;
        for (i = 0; i < 1; i++) {
            object->mdlId[i] = -1;
        }
        for (i = 0; i < 9; i++) {
            object->mtnId[i] = -1;
        }
        if (param->modelResource >= 0) {
            if (param->linkModel != 0) {
                model = fn_1_45C4(fn_1_A38(param->modelResource));
            } else {
                model = Hu3DModelCreate(HuDataSelHeapReadNum(fn_1_A38(param->modelResource),
                    HU_MEMNUM_OVL, HEAP_MODEL));
            }
        } else {
            model = -1;
        }
        object->mdlId[0] = model;
        motion = param->motions;
        if (motion != NULL) {
            for (i = 0; i < 9; i++, motion++) {
                resource = motion->resource;
                if (resource == -2) {
                    break;
                }
                if (resource != -1) {
                    object->mtnId[i] = fn_1_46CC(model, fn_1_A38(resource));
                }
            }
        }
        if (param->position != NULL) {
            state->actor.position = *param->position;
        }
        work->initialPosition = state->actor.position;
        state->actor.rotation.y = param->rotationY;
        if (model >= 0) {
            Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
            Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
            Hu3DModelLayerSet(model, param->layer);
            Hu3DModelAttrSet(model, param->attributes);
            switch (param->shadowMode) {
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

/* Adds a newly created prop to the first free slot in the manager's lookup table. */
void fn_1_7944(OMOBJ *object) {
    OMOBJ **entry = lbl_1_bss_334;
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

/* Looks up a live prop by its configured type and instance ID. */
OMOBJ *fn_1_7998(s32 type, s32 id)
{
    OMOBJ *object;
    OMOBJ **entry = lbl_1_bss_334;
    M675PropWork *work;
    s32 i;
    for (i = 0; i < 128; i++, entry++) {
        object = *entry;
        if (object != NULL) {
            work = object->data;
            if (work->parameters->type == type && work->id == id) {
                return object;
            }
        }
    }
    return NULL;
}

/* Selects a prop motion, optionally resetting its shape motion at the same time. */
void fn_1_7A20(OMOBJ *object, s32 index, s32 setShape)
{
    M675PropState *state;
    M675PropMotionParameter *parameter;
    M675PropWork *work = object->data;
    s32 model, motion;

    state = &work->state;
    if (state->actor.motion != index && index < 9) {
        motion = object->mtnId[index];
        work->updateCount = 0;
        state->actor.motionFrame = 0;
        if (motion == -1) {
            state->motionActive = 0;
            return;
        }
        parameter = &work->parameters->motions[index];
        model = object->mdlId[0];
        if (state->actor.motion < 0 || (0.0) == parameter->blend) {
            if (setShape) {
                Hu3DMotionShapeSet((s16) model, (s16) motion);
            }
            Hu3DMotionSet((s16) model, (s16) motion);
            Hu3DModelAttrSet((s16) model, parameter->attributes);
        } else {
            if (setShape) {
                Hu3DMotionShapeSet((s16) model, (s16) motion);
            }
            Hu3DMotionShiftSet((s16) model, (s16) motion, 0.0f,
                60.0f * parameter->blend, parameter->attributes);
        }
        state->actor.motion = index;
    }
}

void fn_1_7B58(OMOBJ *object, s32 index)
{
    MotionSetInline(object, index, 0);
}

void fn_1_7C64(OMOBJ *object, s32 index)
{
    MotionSetInline(object, index, 1);
}

/* Tests whether the actor's main motion and blend have both finished. */
BOOL fn_1_7D88(M675MotionObjectView *actor)
{
    s32 model_id;

    model_id = *(s16 *) actor->modelIdsAddress;
    if (Hu3DMotionEndCheck((HU3D_MODELID) model_id) != 0 &&
        Hu3DMotionShiftIDGet((HU3D_MODELID) model_id) < 0) {
        return TRUE;
    }
    return FALSE;
}

/* Reads a named prop part's current transform and reports whether its motion is active. */
BOOL fn_1_7DF0(s32 type, s32 id, char *name, Vec *position, Vec *rotation)
{
    OMOBJ *object;
    M675PropWork *work;
    HSF_OBJECT *part;
    s32 model;

    object = find_prop_inlined(type, id);
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
        /* The part's stored yaw is relative to the prop's 90-degree model orientation. */
        rotation->y = 90.0 + part->mesh.curr.rot.y;
        rotation->z = part->mesh.curr.rot.z;
    }
    return !Hu3DMotionEndCheck(model);
}

void fn_1_7F2C(OMOBJ *object) {
    fn_1_7F64(object);
    fn_1_8020(object);
}

/* Applies a prop's stored position and wrapped yaw to its model each update. */
void fn_1_7F64(OMOBJ *object)
{
    M675PropState *state;
    M675PropWork *work = object->data;
    s32 model;

    state = &work->state;
    model = object->mdlId[0];
    state->actor.rotation.y = fmodf(state->actor.rotation.y, 360.0f);
    Hu3DModelPosSetV(model, &state->actor.position);
    Hu3DModelRotSetV(model, &state->actor.rotation);
}

/* Clears motion tracking after playback and any blend finish during the prop update. */
void fn_1_8020(OMOBJ *object)
{
    M675PropState *state;
    M675PropWork *work = object->data;
    s32 done;
    s16 model;
    state = &work->state;

    if (state->motionActive) {
        model = object->mdlId[0];
        if (Hu3DMotionEndCheck((s16) model) && Hu3DMotionShiftIDGet((s16) model) < 0) {
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
    work->updateCount++;
}

void fn_1_80CC(OMOBJ *object)
{
}

char lbl_1_data_750[] = "inf_light";

/* Sets up the path recorder and allocates one pair of transform samples per frame. */
void fn_1_80D0(OMOBJ *object) {
    s32 sampleBytes;
    M675PlaybackWork *work;
    s16 model;
    work = object->data;
    model = object->mdlId[0];
    lbl_1_bss_330 = object;
    fn_1_E48(model, lbl_1_data_750, 1);
    work->frameCount = (s32)(1.5 + Hu3DMotionMaxTimeGet((s16) model));
    work->lastRecordedFrame = -1;
    sampleBytes = work->frameCount * 12 * 2;
    work->samples = HuMemDirectMallocNum(HEAP_HEAP, sampleBytes, HU_MEMNUM_OVL);
    memset(work->samples, 0, sampleBytes);
    object->objFunc = fn_1_81A8;
}

char lbl_1_data_75A[] = "tatiiti";

/* Records the named part's position and rotation during the recorder object's updates. */
void fn_1_81A8(OMOBJ *object)
{
    M675PlaybackWork *work = object->data;
    M675PropWork *searchWork;
    OMOBJ **entry;
    OMOBJ *candidate;
    s32 selectedModelId;
    OMOBJ *selectedEntry;
    OMOBJ *selected;
    s32 selectedDataAddress;
    s32 frame;
    HSF_OBJECT *part;
    Vec *samples = work->samples;
    s32 modelId;
    s32 index;

    modelId = object->mdlId[0];
    frame = (s32)Hu3DMotionTimeGet(modelId);
    if (frame >= work->frameCount) {
        frame = work->frameCount - 1;
    }
    if (frame < 0) {
        frame = 0;
    }
    if (work->lastRecordedFrame < frame) {
        work->lastRecordedFrame = frame;
    }

    entry = lbl_1_bss_334;
    index = 0;
    while (index < 128) {
        candidate = *entry;
        if (candidate != NULL) {
            searchWork = candidate->data;
            if (searchWork->parameters->type == 0 &&
                searchWork->id == 0) {
                selectedEntry = candidate;
                goto foundSampleObject;
            }
        }
        index++;
        entry++;
    }
    selectedEntry = NULL;
foundSampleObject:
    selected = selectedEntry;
    if (selected != NULL) {
        selectedDataAddress = (s32)selected->data;
        selectedModelId = selected->mdlId[0];
        part = Hu3DModelObjPtrGet((s16)selectedModelId, lbl_1_data_75A);
        if (part != NULL) {
            samples[frame * 2].x = part->mesh.curr.pos.x;
            samples[frame * 2].y = part->mesh.curr.pos.y;
            samples[frame * 2].z = part->mesh.curr.pos.z;
            samples[frame * 2 + 1].x = part->mesh.curr.rot.x;
            samples[frame * 2 + 1].y =
                (f32)(90.0 + (f64)part->mesh.curr.rot.y);
            samples[frame * 2 + 1].z = part->mesh.curr.rot.z;
        }
        /* The motion-end query runs after copying the transform, and its result is discarded. */
        !Hu3DMotionEndCheck((s16)selectedModelId);
    }
    /* Subtract 90 degrees even when no part was found and the stored sample was not refreshed. */
    samples[frame * 2 + 1].y -= 90.0;
}

/* Returns recorded position and rotation samples relative to the current motion frame. */
void fn_1_8394(s32 frameOffset, Vec *first, Vec *second)
{
    OMOBJ *object = lbl_1_bss_330;
    M675PlaybackWork *work = object->data;
    Vec *samples = work->samples;
    s32 model = object->mdlId[0];
    s32 frame;

    frame = (s32) Hu3DMotionTimeGet(model);
    frame = frame + frameOffset;
    if (frame >= work->frameCount) {
        frame -= work->frameCount - 1;
    }
    if (frame < 0) {
        frame += work->frameCount - 1;
    }
    if (frame >= work->frameCount) {
        frame = work->frameCount - 1;
    }
    if (frame > work->lastRecordedFrame) {
        frame = work->lastRecordedFrame;
    }
    if (frame < 0) {
        frame = 0;
    }
    if (first != NULL) {
        *first = samples[frame * 2];
    }
    if (second != NULL) {
        *second = samples[frame * 2 + 1];
    }
}

Vec lbl_1_data_764[4] = {
    {-300.0f, 0.0f, -300.0f},
    {300.0f, 0.0f, -300.0f},
    {300.0f, 0.0f, 300.0f},
    {-300.0f, 0.0f, 300.0f}
};

/* Emits dust along two rotated edges of the playback prop on each update. */
static inline void M675PlaybackEmitMotion(OMOBJ *object)
{
    Mtx rotationMatrix;
    Vec rotatedPoints[4];
    Vec point;
    void *data = object->data;
    M675PlaybackWork *work = data;
    s32 pointIndex;
    s32 sampleIndex;

    PSMTXRotRad(rotationMatrix, 'y',
        0.01745329238474369f * work->state.actor.rotation.y);
    PSMTXMultVecArraySR(rotationMatrix, lbl_1_data_764, rotatedPoints, 4);
    for (pointIndex = 0; pointIndex < 2; pointIndex++) {
        Vec *startPoint = &rotatedPoints[pointIndex];
        Vec *endPoint = &rotatedPoints[(pointIndex + 1) & 3];

        for (sampleIndex = 0; sampleIndex < 3; sampleIndex++) {
            f32 interpolation = (f32)(1.52587890625e-05 *
                (f64)(f32)(s32)(frand() & 0xFFFF));
            point.x = work->state.actor.position.x + startPoint->x + interpolation *
                (endPoint->x - startPoint->x);
            point.y = work->state.actor.position.y;
            point.z = work->state.actor.position.z + startPoint->z + interpolation *
                (endPoint->z - startPoint->z);
            fn_1_E0F0(&point);
        }
    }
}

/* Emits six dust points along two adjacent edges of the rotated prop each update. */
void fn_1_84C4(OMOBJ *object)
{
    Mtx rotationMatrix;
    Vec rotatedPoints[4];
    Vec point;
    void *data = object->data;
    M675PropWork *work = data;
    s32 pointIndex;
    s32 sampleIndex;

    PSMTXRotRad(rotationMatrix, 'y',
        0.01745329238474369f * work->state.actor.rotation.y);
    PSMTXMultVecArraySR(rotationMatrix, lbl_1_data_764,
        rotatedPoints, 4);
    for (pointIndex = 0; pointIndex < 2; pointIndex++) {
        Vec *startPoint = &rotatedPoints[pointIndex];
        Vec *endPoint = &rotatedPoints[(pointIndex + 1) & 3];
        for (sampleIndex = 0; sampleIndex < 3; sampleIndex++) {
            f32 interpolation = (f32)(1.52587890625e-05 *
                (f64)(f32)(s32)(frand() & 0xFFFF));
            point.x = work->state.actor.position.x + startPoint->x + interpolation *
                (endPoint->x - startPoint->x);
            point.y = work->state.actor.position.y;
            point.z = work->state.actor.position.z + startPoint->z + interpolation *
                (endPoint->z - startPoint->z);
            fn_1_E0F0(&point);
        }
    }
}

void fn_1_8624(OMOBJ *obj)
{
    obj->objFunc = fn_1_8634;
}

/* Replays recorded transforms and restarts its bounce and dust burst for each nonzero request. */
void fn_1_8634(OMOBJ *object)
{
    M675PlaybackWork *work = object->data;
    M675PlaybackWork *state = work;

    M675PlaybackSamples(0, &state->state.actor.position, &state->state.actor.rotation);
    if (work->controlValue != 0) {
        work->controlValue = 0;
        work->bounceScale = 1.0f;
        work->bounceAngle = 0.0f;
        M675PlaybackEmitMotion(object);
    }

    work->bounceScale *= 0.95;
    work->bounceAngle = fmodf(work->bounceAngle - 10.0f, 360.0f);
    state->state.actor.position.y += 25.0 * (work->bounceScale *
        sin((3.141592653589793 * work->bounceAngle) / 180.0));

    M675PlaybackApplyTransform(object);
    M675PlaybackAdvanceMotion(object);
}

/* Updates the control value on the type-0, ID-2 prop when it is registered. */
void fn_1_8AC4(s32 value)
{
    M675PropWork *work;
    OMOBJ **entry;
    OMOBJ *candidate;
    OMOBJ *object;
    M675PropWork *resultWork;
    OMOBJ *selected;
    s32 i;

    entry = lbl_1_bss_334;
    i = 0;
    while (i < 128) {
        candidate = *entry;
        if (candidate != NULL) {
            work = candidate->data;
            if (work->parameters->type == 0 && work->id == 2) {
                selected = candidate;
                goto selectedObject;
            }
        }
        i++;
        entry++;
    }
    selected = NULL;
selectedObject:
    object = selected;
    if (object != NULL) {
        resultWork = object->data;
        resultWork->controlValue = value;
    }
}

/* Randomizes an ambient prop's horizontal start, offsets its height by ID, and starts its flight
 * update. */
void fn_1_8B64(OMOBJ *object)
{
    s32 modelId[1];
    M675PropWork *state = object->data;
    M675PropWork *work = state;

    modelId[0] = object->mdlId[0];
    state->delay = 0;
    work->state.actor.position.x = (f32)((f64)state->initialPosition.x *
        (3.0517578125e-05 *
            (f64)(f32)((u16)frand() - 32768)));
    work->state.actor.position.y = (f32)((f64)work->state.actor.position.y -
        (200.0 * (f64)state->id));
    work->state.actor.position.z = (f32)((f64)state->initialPosition.z +
        (1500.0 * (3.0517578125e-05 *
            (f64)(f32)((u16)frand() - 32768))));
    object->objFunc = fn_1_8CBC;
}

/* Steers an ambient prop horizontally toward periodic random targets and banks its model each
 * frame. */
void fn_1_8CBC(OMOBJ *object)
{
    f32 targetAngle;
    M675PropWork *state = object->data;
    M675PropWork *work = state;
    f32 movementScale = 20.0f;
    s32 modelId = object->mdlId[0];
    f32 targetVelocityX;
    f32 targetVelocityZ;

    if (--state->delay <= 0) {
        state->delay = (s32)(180.0 *
            (1.52587890625e-05 *
                (f64)(f32)(s32)(frand() & 0xFFFF))) + 300;
        work->state.targetPosition.x = (f32)((f64)state->initialPosition.x *
            (3.0517578125e-05 *
                (f64)(f32)((u16)frand() - 32768)));
        work->state.targetPosition.y = work->state.actor.position.y;
        work->state.targetPosition.z = (f32)((f64)state->initialPosition.z +
            (1500.0 * (3.0517578125e-05 *
                (f64)(f32)((u16)frand() - 32768))));
    }

    targetAngle = fn_1_49AC(&work->state.actor.position, &work->state.targetPosition);
    work->state.actor.rotation.y = fn_1_47DC(work->state.actor.rotation.y, targetAngle,
        0.009999999776482582f);
    if (fn_1_48F8(targetAngle, work->state.actor.rotation.y) < 0.0) {
        targetAngle = 10.0f;
    } else {
        targetAngle = -10.0f;
    }
    work->state.actor.rotation.z = fn_1_47DC(work->state.actor.rotation.z, targetAngle,
        0.05000000074505806f);

    targetVelocityX = (f32)((f64)movementScale *
        sin((3.141592653589793 * (f64)work->state.actor.rotation.y) /
            180.0));
    targetVelocityZ = (f32)((f64)movementScale *
        cos((3.141592653589793 * (f64)work->state.actor.rotation.y) /
            180.0));
    work->state.actor.velocity.x = (f32)((f64)work->state.actor.velocity.x +
        (0.03 * (f64)(targetVelocityX - work->state.actor.velocity.x)));
    work->state.actor.velocity.z = (f32)((f64)work->state.actor.velocity.z +
        (0.03 * (f64)(targetVelocityZ - work->state.actor.velocity.z)));
    PSVECAdd(&work->state.actor.position, &work->state.actor.velocity, &work->state.actor.position);
    Hu3DModelPosSetV(modelId, &work->state.actor.position);
    Hu3DModelRotSetV(modelId, &work->state.actor.rotation);
}

/* Shuffles each actor's emitted effect types, sets its transform, and starts its sequence. */
void fn_1_8FE0(OMOBJ *object)
{
    static const Vec lbl_1_rodata_2B0[4] = { { -410.0f, 200.0f, 550.0f },
                                             { -100.0f, 200.0f, 550.0f },
                                             { 210.0f, 200.0f, 550.0f },
                                             { 520.0f, 200.0f, 550.0f } };
    static const f32 lbl_1_rodata_2E0[4] = {8.0f, 4.0f, -2.0f, -6.0f};
    static const u8 lbl_1_rodata_2F0[20] = {
        0, 0, 0, 0, 1, 2, 2, 2, 0, 0, 0, 0, 0, 0, 1, 2,
        2, 2, 0, 0
    };
    M675PropWork *work = object->data;
    M675PropWork *motionWork;
    M675PropWork *motionState;
    u8 *shuffle = work->shuffledEffectTypes;
    s32 round;
    s32 swappedValue;
    M675PropWork *activeWork;
    M675PropWork *activeState;
    s32 position;
    s32 swapPosition;
    s32 swapIndex;
    s16 modelId;
    s32 motionEnded;
    s32 motionModelId;
    f32 rotation;
    f32 wrappedRotation;
    f32 storedRotation;

    for (position = 0; position < 20; position++) {
        shuffle[position] = lbl_1_rodata_2F0[position];
    }
    for (round = 0; round < 4; round++) {
        for (position = 1; position <= 8; position++) {
            swapPosition = position;
            swapIndex = (s32)(8.0 *
                (1.52587890625e-05 * (f64)(f32)(s32)(frand() & 0xFFFF))) + 1;
            swappedValue = shuffle[swapPosition];
            shuffle[swapPosition] = shuffle[swapIndex];
            shuffle[swapIndex] = (u8)swappedValue;

            swapPosition = position + 10;
            swapIndex = (s32)(8.0 *
                (1.52587890625e-05 * (f64)(f32)(s32)(frand() & 0xFFFF))) + 11;
            swappedValue = shuffle[swapPosition];
            shuffle[swapPosition] = shuffle[swapIndex];
            shuffle[swapIndex] = (u8)swappedValue;
        }
    }

    work->state.actor.position = lbl_1_rodata_2B0[work->id];
    work->state.actor.rotation.y = lbl_1_rodata_2E0[work->id];

    motionWork = object->data;
    motionState = motionWork;
    modelId = object->mdlId[0];
    rotation = motionState->state.actor.rotation.y;
    wrappedRotation = (f32)fmod((f64)rotation, 360.0);
    storedRotation = wrappedRotation;
    motionState->state.actor.rotation.y = storedRotation;
    Hu3DModelPosSetV((s16)modelId, &motionState->state.actor.position);
    Hu3DModelRotSetV((s16)modelId, &motionState->state.actor.rotation);

    activeWork = object->data;
    activeState = activeWork;
    if (activeState->state.motionActive != 0) {
        motionModelId = object->mdlId[0];
        if (Hu3DMotionEndCheck(motionModelId) != 0 &&
            Hu3DMotionShiftIDGet(motionModelId) < 0) {
            motionEnded = 1;
        } else {
            motionEnded = 0;
        }
        if (motionEnded != 0) {
            activeState->state.motionActive = 0;
            activeState->state.motionTimer = 0;
        }
    } else {
        activeState->state.motionTimer = 0;
    }
    activeWork->updateCount++;
    object->objFunc = fn_1_92A0;
}

/* Runs one Banana Shake actor's event and motion sequence from its object callback. */
void fn_1_92A0(OMOBJ *object)
{
    M675PropWork *work = object->data;
    M675PropWork *state = work;
    u8 *shuffledEffectTypes = work->shuffledEffectTypes;
    s32 modelId = object->mdlId[0];
    Vec effectPosition;

    switch (work->sequencePhase) {
    case 0:
        work->sequencePhase++;
        work->sequenceTimer = 0;
        work->eventCount = 0;
        work->shuffleIndex = 0;
        work->effectActive = 0;
        work->effectMotionSpeed = 0.0f;
        work->soundHandle = -1;
        M675SequenceMotionSet(object, 0, 1);
        Hu3DModelAttrSet((s16)modelId,
            HU3D_MOTATTR_LOOP | HU3D_MOTATTR_SHAPE_LOOP);

    case 1:
        if (MgSeqModeGet() == 4) {
            work->sequenceTimer = 0;
            work->eventCount = 0;
        }
        if (work->controlValue == 0) {
            break;
        }
        work->sequencePhase++;
        if (work->soundHandle == -1) {
            work->soundHandle = HuAudFXPlayPan(MSM_SE_M675_2225,
                (s16)lbl_1_data_740[work->id]);
        }

    case 2:
        if (work->controlValue != 0) {
                work->sequenceTimer = 30;
                work->effectActive = 1;
                work->effectMotionSpeed = 1.0f;
                if (++work->eventCount >= 10) {
                    s32 effectType;
                    work->eventCount = 0;
                    effectPosition = work->state.actor.position;
                    effectPosition.x -= 70.0;
                    effectPosition.y += 550.0;
                    if (work->controlValue == 1) {
                        effectType = shuffledEffectTypes[work->shuffleIndex];
                        if (++work->shuffleIndex >= 20) {
                            work->shuffleIndex = 0;
                        }
                    } else {
                        OMOBJ *selected = M675MotionFind(5, 0);
                        if (selected != NULL) {
                            M675PropWork *selectedWork = selected->data;
                            effectPosition.x = selectedWork->state.actor.position.x - 45.0;
                        }
                        effectType = 4;
                    }
                    fn_1_D3DC(effectType, work->id, &effectPosition);
                    HuAudFXPlayPan(MSM_SE_M675_2226, (s16)lbl_1_data_740[work->id]);
                }
                work->controlValue = 0;
        }
        if (--work->sequenceTimer <= 0) {
            work->sequencePhase--;
            if (work->soundHandle != -1) {
                HuAudFXFadeOut(work->soundHandle, 1000);
                work->soundHandle = -1;
            }
        }
        break;
    }

    if (fn_1_A40() != 0) {
        work->effectActive = 0;
    }
    if (work->effectActive != 0) {
        if (state->state.actor.motion != 1) {
            f32 startTime;
            M675SequenceMotionSet(object, 1, 1);
            startTime = work->alternateMotionStart == 0 ? 0.0 : 15.0;
            Hu3DMotionTimeSet((s16)modelId, startTime);
            Hu3DMotionShapeTimeSet((s16)modelId, startTime);
        }
        if ((work->effectMotionSpeed -= 0.01) < 0.1) {
            work->effectMotionSpeed = 0.1f;
        }
        Hu3DMotionSpeedSet((s16)modelId, work->effectMotionSpeed);
        Hu3DMotionShapeSpeedSet((s16)modelId, work->effectMotionSpeed);
        if (M675PlaybackMotionEnded(object->mdlId[0])) {
            Hu3DMotionTimeSet((s16)modelId, 0.0f);
            Hu3DMotionShapeTimeSet((s16)modelId, 0.0f);
            if (work->effectMotionSpeed <= 0.11) {
                work->effectActive = 0;
            }
        }
    }
    if (work->effectActive == 0) {
        work->effectMotionSpeed = 0.0f;
        if (state->state.actor.motion != 0) {
        M675SequenceMotionSet(object, 0, 1);
        Hu3DMotionTimeSet((s16)modelId, 0.0f);
        Hu3DMotionSpeedSet((s16)modelId, 1.0f);
        Hu3DMotionShapeTimeSet((s16)modelId, 0.0f);
        Hu3DMotionShapeSpeedSet((s16)modelId, 1.0f);
        }
    }
}

/* Allocates emitter records and collision registrations for a configured effect group. */
void fn_1_9AF0(OMOBJ *object, s32 resourceId, s32 collisionType,
    f32 verticalOffset, f32 heightLimit)
{
    M675EmitterWork *work = object->data;
    s32 recordCount = object->work[0];
    s32 allocationSize = recordCount * sizeof(M675EmitterRecord);
    M675EmitterRecord *records = HuMemDirectMallocNum(
        HEAP_HEAP, allocationSize, HU_MEMNUM_OVL);
    s32 index;

    memset(records, 0, allocationSize);
    work->records = records;
    for (index = 0; index < recordCount; index++, records++) {
        M675Collision *callback;

        records->modelId = fn_1_45C4(fn_1_A38(resourceId));
        Hu3DModelAttrSet((s16)records->modelId, 1);
        records->verticalOffset = verticalOffset;
        records->heightLimit = heightLimit;
        callback = &records->collision;

        callback->flags = 0;
        callback->mask = 8;
        callback->type = collisionType;
        callback->index = index;
        callback->shapeType = 0;
        callback->owner = object;
        callback->update = 0;
        callback->contact = 0;
        callback->position = &records->position;
        callback->velocity = &records->velocity;
        callback->positionCopy = *callback->position;
        callback->radius = 30.0f;
        callback->yieldOnContact = 1.0f;
        fn_1_10170(callback);
    }
}

/* Converts a point near the player's score display from screen coordinates to world space. */
void fn_1_9C3C(s32 playerIndex, HuVecF *destination) {
    HuVecF source;
    fn_1_F0B0(playerIndex, &source.x, &source.y);
    source.z = 1200.0f;
    source.y = source.y - 10.0;
    Hu3D2Dto3D(&source, 1, destination);
}

/* Sets up the emitter pool with collision category 3 and installs its frame callback. */
void fn_1_9CAC(OMOBJ *object)
{
    s32 allocationSize;
    s32 recordCount;
    M675EmitterWork *work;
    M675EmitterRecord *records;
    s32 index;

    lbl_1_bss_32C = object;
    object->work[0] = 32;
    work = object->data;
    recordCount = object->work[0];
    allocationSize = recordCount * sizeof(M675EmitterRecord);
    records = HuMemDirectMallocNum(HEAP_HEAP, allocationSize, HU_MEMNUM_OVL);
    memset(records, 0, (u32)allocationSize);
    work->records = records;

    for (index = 0; index < recordCount; index++, records++) {
        M675Collision *callback;

        records->modelId = fn_1_45C4(fn_1_A38(1));
        Hu3DModelAttrSet((s16)records->modelId, 1);
        records->verticalOffset = 50.0f;
        records->heightLimit = 0.0f;
        callback = &records->collision;
        callback->flags = 0;
        callback->mask = 8;
        callback->type = 3;
        callback->index = index;
        callback->shapeType = 0;
        callback->owner = object;
        callback->update = 0;
        callback->contact = 0;
        callback->position = &records->position;
        callback->velocity = &records->velocity;
        callback->positionCopy = *callback->position;
        callback->radius = 30.0f;
        callback->yieldOnContact = 1.0f;
        fn_1_10170(callback);
    }
    object->objFunc = fn_1_A0F0;
}

/* Sets up the emitter pool with collision category 4 and installs its frame callback. */
void fn_1_9E18(OMOBJ *object)
{
    s32 allocationSize;
    s32 recordCount;
    M675EmitterWork *work;
    M675EmitterRecord *records;
    s32 index;

    lbl_1_bss_328 = object;
    object->work[0] = 8;
    work = object->data;
    recordCount = object->work[0];
    allocationSize = recordCount * sizeof(M675EmitterRecord);
    records = HuMemDirectMallocNum(HEAP_HEAP, allocationSize, HU_MEMNUM_OVL);
    memset(records, 0, (u32)allocationSize);
    work->records = records;

    for (index = 0; index < recordCount; index++, records++) {
        M675Collision *callback;

        records->modelId = fn_1_45C4(fn_1_A38(2));
        Hu3DModelAttrSet((s16)records->modelId, 1);
        records->verticalOffset = -10.0f;
        records->heightLimit = 0.0f;
        callback = &records->collision;
        callback->flags = 0;
        callback->mask = 8;
        callback->type = 4;
        callback->index = index;
        callback->shapeType = 0;
        callback->owner = object;
        callback->update = 0;
        callback->contact = 0;
        callback->position = &records->position;
        callback->velocity = &records->velocity;
        callback->positionCopy = *callback->position;
        callback->radius = 30.0f;
        callback->yieldOnContact = 1.0f;
        fn_1_10170(callback);
    }
    object->objFunc = fn_1_A0F0;
}

/* Sets up the emitter pool with collision category 5 and installs its frame callback. */
void fn_1_9F84(OMOBJ *object)
{
    s32 allocationSize;
    s32 recordCount;
    M675EmitterWork *work;
    M675EmitterRecord *records;
    s32 index;

    lbl_1_bss_324 = object;
    object->work[0] = 8;
    work = object->data;
    recordCount = object->work[0];
    allocationSize = recordCount * sizeof(M675EmitterRecord);
    records = HuMemDirectMallocNum(HEAP_HEAP, allocationSize, HU_MEMNUM_OVL);
    memset(records, 0, (u32)allocationSize);
    work->records = records;

    for (index = 0; index < recordCount; index++, records++) {
        M675Collision *callback;

        records->modelId = fn_1_45C4(fn_1_A38(23));
        Hu3DModelAttrSet((s16)records->modelId, 1);
        records->verticalOffset = 100.0f;
        records->heightLimit = -80.0f;
        callback = &records->collision;
        callback->flags = 0;
        callback->mask = 8;
        callback->type = 5;
        callback->index = index;
        callback->shapeType = 0;
        callback->owner = object;
        callback->update = 0;
        callback->contact = 0;
        callback->position = &records->position;
        callback->velocity = &records->velocity;
        callback->positionCopy = *callback->position;
        callback->radius = 30.0f;
        callback->yieldOnContact = 1.0f;
        fn_1_10170(callback);
    }
    object->objFunc = fn_1_A0F0;
}

M675MotionEmitterUpdate lbl_1_data_794[5] = {fn_1_A274, fn_1_A448, fn_1_A5B4, fn_1_A8EC, fn_1_AA48};

/* Advances active emitter records and updates their models once per object tick. */
void fn_1_A0F0(OMOBJ *object)
{
    M675EmitterWork *work = object->data;
    M675EmitterRecord *record = work->records;
    s32 recordCount = object->work[0];
    s32 index;

    for (index = 0; index < recordCount; index++, record++) {
        if (record->state != 0) {
            lbl_1_data_794[record->type](record);
            if (record->type != 2 && record->state != 0 &&
                record->enabled != 0 &&
                record->position.y < 200.0 + record->heightLimit) {
                Vec position;

                position = record->position;
                position.y = 200.0f;
                fn_1_E4F8((Point3d *)&position, 10.0f, 4);
                fn_1_AD98(record);
                record->state = 0;
            } else {
                PSVECAdd(&record->position, &record->velocity,
                    &record->position);
                PSVECAdd(&record->rotation, &record->rotationDelta,
                    &record->rotation);
                Hu3DModelPosSet((s16)record->modelId, record->position.x,
                    record->position.y + record->verticalOffset,
                    record->position.z);
                Hu3DModelRotSetV((s16)record->modelId, &record->rotation);
                Hu3DModelScaleSet((s16)record->modelId, record->scale,
                    record->scale, record->scale);
            }
        }
    }
}

/* Updates a falling pickup and sends it toward its player's score display after contact. */
void fn_1_A274(M675EmitterRecord *state)
{
    Point3d projected;
    s32 playerIndex;

    switch (state->state) {
    case 1:
        fn_1_ACB0(state);
        state->state++;
        state->enabled = 1;
        state->velocity.y = -19.16666603088379f;

    case 2:
        if (state->collision.handled == 0) {
            return;
        }
        state->state++;
        state->timer = 0.0f;
        state->enabled = 0;
        state->velocity.y = 0.0f;
        state->rotationDelta.y = 10.0f;
        state->sourcePosition = state->position;
        playerIndex = state->playerIndex;
        fn_1_F0B0(playerIndex, &projected.x, &projected.y);
        projected.z = 1200.0f;
        projected.y = (f32)((f64)projected.y - 10.0);
        Hu3D2Dto3D(&projected, 1, &state->projectedPosition);
        state->collision.flags &= ~1u;
        HuAudFXPlayPan(MSM_SE_M675_2227, lbl_1_data_740[state->playerIndex]);
        return;
    case 3:
        state->timer += 0.05;
        fn_1_4BB4(&state->sourcePosition, &state->projectedPosition,
            state->timer, &state->position);
        if (!(state->timer < 1.0)) {
            fn_1_EF88(state->playerIndex, 1);
            fn_1_AD98(state);
            state->state = 0;
        }
        break;
    }
}

/* Drops a pickup, then emits a short repeated burst after collision. */
void fn_1_A448(M675EmitterRecord *state)
{
    switch (state->state) {
    case 1:
        fn_1_ACB0(state);
        state->state++;
        state->phaseCounter = 10;
        state->enabled = 1;
        state->velocity.y = -19.16666603088379f;

    case 2:
        if (state->collision.handled == 0) {
            return;
        }
        state->state++;
        state->timer = 0.0f;
        state->phaseCounter = 5;
        state->enabled = 0;
        state->velocity.y = 0.0f;
        fn_1_AD98(state);
        HuAudFXPlayPan(MSM_SE_M675_2228, (s16)lbl_1_data_740[state->playerIndex]);
        return;
    case 3:
        if (!((state->timer = (f32)((f64)state->timer -
                     1.0)) >= 0.0)) {
            state->timer = 9.0f;
            fn_1_D3DC(3, state->playerIndex, &state->position);
            if (--state->phaseCounter <= 0) {
                state->state = 0;
            }
        }
        break;
    }
}

/* Bounces and spins a pickup after collision or ground impact, then removes it. */
void fn_1_A5B4(M675EmitterRecord *state)
{
    Vec position;

    switch (state->state) {
    case 1:
        fn_1_ACB0(state);
        state->state++;
        state->phaseCounter = 0;
        state->enabled = 1;
        state->velocity.y = -19.16666603088379f;
        state->rotation.y = 90.0f;
        state->rotation.z = 90.0f;
        state->rotationDelta.z = 8.0f;
    case 2:
        if (state->collision.handled != 0) {
            state->state++;
            state->timer = 0.0f;
            state->enabled = 0;
            state->velocity.x = -6.666666507720947f;
            state->velocity.y = 13.333333015441895f;
            state->velocity.z = 6.666666507720947f;
            state->collision.flags &= ~1U;
            HuAudFXPlayPan(MSM_SE_M675_2229, (s16)lbl_1_data_740[state->playerIndex]);
            break;
        }
        if (state->position.y < 200.0 +
            (f64)state->heightLimit) {
            state->state++;
            state->timer = 0.0f;
            state->enabled = 0;
            state->velocity.x = 3.3333332538604736f;
            state->velocity.y = 13.333333015441895f;
            state->velocity.z = 6.666666507720947f;
            state->rotationDelta.z = -10.0f;
            state->collision.flags &= ~1U;
            position = state->position;
            position.y = 200.0f;
            fn_1_E4F8(&position, 20.0f, 3);
            break;
        }
        break;
    case 3:
        if ((++state->phaseCounter & 1) == 0) {
            Hu3DModelAttrSet((s16)state->modelId, 1U);
        } else {
            Hu3DModelAttrReset((s16)state->modelId, 1U);
        }
        state->velocity.y -= 1.0896277777777779;
        state->velocity.x *= 0.98;
        state->velocity.z *= 0.98;
        if (!(state->velocity.y >= 0.0) &&
            !(state->position.y > 200.0 +
                (f64)state->heightLimit)) {
            position = state->position;
            position.y = 200.0f;
            fn_1_E4F8(&position, 20.0f, 4);
            fn_1_AD98(state);
            state->state = 0;
        }
    }
}

/* Moves an emitted reward to its player's score display and adds one point on arrival. */
void fn_1_A8EC(M675EmitterRecord *work)
{
    Point3d point;
    s32 playerIndex;

    switch (work->state) {
    case 1:
        fn_1_ACB0(work);
        work->state++;
        work->timer = 0.0f;
        work->rotationDelta.y = 10.0f;
        work->sourcePosition = work->position;
        playerIndex = work->playerIndex;
        fn_1_F0B0(playerIndex, &point.x, &point.y);
        point.z = 1200.0f;
        point.y = (f32)((f64)point.y - 10.0);
        Hu3D2Dto3D(&point, 1, &work->projectedPosition);
        work->collision.flags &= ~1u;
        break;
    case 2:
        work->timer += 0.05;
        fn_1_4BB4(&work->sourcePosition, &work->projectedPosition,
            work->timer, &work->position);
        if (!(work->timer < 1.0)) {
            fn_1_EF88(work->playerIndex, 1);
            fn_1_AD98(work);
            work->state = 0;
        }
        break;
    }
}

/* Tracks a falling pickup, moves it toward its player's score display after contact, and raises the
 * drop flag. */
void fn_1_AA48(M675EmitterRecord *state)
{
    Point3d projected;
    s32 playerIndex;

    /* This state check discards its result. */
    DebugValue(state->state == 1);
    switch (state->state) {
    case 1:
        fn_1_ACB0(state);
        state->state++;
        state->enabled = 1;
        state->velocity.y = -19.16666603088379f;

    case 2:
        if (state->collision.handled != 0) {
            state->state++;
            state->timer = 0.0f;
            state->enabled = 0;
            state->velocity.y = 0.0f;
            state->rotationDelta.y = 10.0f;
            state->sourcePosition = state->position;
            playerIndex = state->playerIndex;
            fn_1_F0B0(playerIndex, &projected.x, &projected.y);
            projected.z = 1200.0f;
            projected.y = (f32)((f64)projected.y - 10.0);
            Hu3D2Dto3D(&projected, 1, &state->projectedPosition);
            state->collision.flags &= ~1u;
            HuAudFXPlayPan(MSM_SE_M675_2227,
                (s16)lbl_1_data_740[state->playerIndex]);
        }
        break;
    case 3:
        state->timer += 0.05;
        fn_1_4BB4(&state->sourcePosition, &state->projectedPosition,
            state->timer, &state->position);
        if (!(state->timer < 1.0)) {
            fn_1_EF88(state->playerIndex, 1);
            fn_1_AD98(state);
            state->state = 0;
        }
        break;
    }
    /* Collection still forces this fall speed and rotation each update, and the below-400 cutoff
     * applies in every state. */
    state->velocity.y = -19.16666603088379f;
    state->rotation.y = 180.0f;
    state->rotation.z = -190.0f;
    if (state->position.y < 700.0) {
        lbl_1_bss_320 = 1;
    }
    if (state->position.y < 400.0) {
        fn_1_AD98(state);
        state->state = 0;
    }
}

/* Resets an emitter record before its type-specific update begins. */
void fn_1_ACB0(M675EmitterRecord *record) {
    record->scale = (f32) 1.0f;
    record->velocity.x = (f32) 0.0f;
    record->velocity.y = (f32) 0.0f;
    record->velocity.z = (f32) 0.0f;
    record->rotation.x = (f32) 0.0f;
    record->rotation.y = (f32) 0.0f;
    record->rotation.z = (f32) 0.0f;
    record->rotationDelta.x = (f32) 0.0f;
    record->rotationDelta.y = (f32) 0.0f;
    record->rotationDelta.z = (f32) 0.0f;
    record->collision.flags = 1;
    record->collision.handled = 0;
    Hu3DModelAttrReset((s16) record->modelId, 1U);
}

/* Disables the emitter's collision and hides its model. */
void fn_1_AD98(M675EmitterRecord *record) {
    record->collision.flags &= ~1U;
    Hu3DModelAttrSet((s16) record->modelId, 1U);
}

/* Registers the prop's effect position and begins its idle-to-sequence update callback. */
void fn_1_ADDC(OMOBJ *object)
{
    M675PropWork *activeWork;
    M675PropWork *active;
    s32 modelId;

    activeWork = object->data;
    active = activeWork;
    modelId = object->mdlId[0];
    M675ShadowMotionSetPlain(object, 0);
    Hu3DModelShadowMapSet((s16)modelId);
    activeWork->displayPosition = active->state.actor.position;
    active->state.effectRegistrationActive = 1;
    active->state.effectRegistrationRadius = 60.0f;
    active->state.effectRegistrationPosition = &activeWork->displayPosition;
    fn_1_E918(&active->state.effectRegistrationActive);
    object->objFunc = fn_1_AF38;
}

/* Advances the prop's scripted sequence, synchronizes its position, and applies its model. */
void fn_1_AF38(OMOBJ *object)
{
    M675PropWork *state = object->data;
    M675PropWork *work = state;
    M675PropWork *searchWork;
    OMOBJ **entry;
    OMOBJ *candidate;
    OMOBJ *selected;
    M675PropWork *selectedWork;
    s32 i;
    OMOBJ *selectedEntry;

    fn_1_74C8(&work->state.actor);
    fn_1_61F4(&work->state.actor);
    switch (state->sequencePhase) {
    case 0:
        state->sequencePhase++;
        work->state.phase = 0;

    case 1:
        if (fn_1_B330(work) != 0) {
            state->sequencePhase++;
            work->state.phase = 0;
        }
        break;
    case 2:
        if (fn_1_BA58(work) != 0) {
            state->sequencePhase++;
            work->state.phase = 0;
        }
        break;
    case 3:
        if (fn_1_BBDC(work) != 0) {
            state->sequencePhase++;
            work->state.phase = 0;
        }
        break;
    case 4:
        if (fn_1_BD84(work) != 0) {
            state->sequencePhase++;
            work->state.phase = 0;
        }
        break;
    }
    fn_1_D148(object);
    PSVECAdd(&work->state.actor.position, &work->state.actor.velocity, &work->state.actor.position);
    if (work->state.positionSyncEnabled != 0) {
        entry = lbl_1_bss_334;
        i = 0;
        while (i < 128) {
            candidate = *entry;
            if (candidate != NULL) {
                searchWork = candidate->data;
                if (searchWork->parameters->type == 0 &&
                    searchWork->id == 2) {
                    selectedEntry = candidate;
                    goto foundSequenceObject;
                }
            }
            i++;
            entry++;
        }
        selectedEntry = NULL;
foundSequenceObject:
        selected = selectedEntry;
        if (selected != NULL) {
            selectedWork = selected->data;
            work->state.actor.position = selectedWork->state.actor.position;
        }
    }
    fn_1_B114(object);
}

/* Applies the prop transform, updates its ground-ring position, and tracks motion completion. */
void fn_1_B114(OMOBJ *object)
{
    M675PropWork *work;
    M675PropWork *state;
    M675PropWork *motionWork;
    M675PropWork *motionState;
    Vec modelPosition;
    f32 rotation;
    f32 wrappedRotation;
    f32 storedRotation;
    s32 positionModel;
    s32 motionEnded;
    s32 motionModel;

    work = object->data;
    state = work;
    positionModel = object->mdlId[0];
    rotation = state->state.actor.rotation.y;
    wrappedRotation = (f32)fmod((f64)rotation, 360.0);
    storedRotation = wrappedRotation;
    state->state.actor.rotation.y = storedRotation;
    PSVECAdd(&state->state.actor.position, &state->state.displayOffset, &modelPosition);
    Hu3DModelPosSetV(positionModel, &modelPosition);
    Hu3DModelRotSetV(positionModel, &state->state.actor.rotation);
    work->displayPosition = modelPosition;
    if (state->state.airborne != 0) {
        if (modelPosition.z > 600.0) {
            work->displayPosition.y = 220.0f;
        } else if (modelPosition.z > 300.0) {
            work->displayPosition.y = 200.0f;
        } else if (modelPosition.z > -100.0) {
            work->displayPosition.y = 180.0f;
        } else if (modelPosition.z > -500.0) {
            work->displayPosition.y = 10000.0f;
        } else {
            work->displayPosition.y = 0.0f;
        }
    }

    motionWork = object->data;
    motionState = motionWork;
    if (motionState->state.motionActive != 0) {
        motionModel = object->mdlId[0];
        if (Hu3DMotionEndCheck((s16)motionModel) != 0 &&
            Hu3DMotionShiftIDGet((s16)motionModel) < 0) {
            motionEnded = 1;
        } else {
            motionEnded = 0;
        }
        if (motionEnded != 0) {
            motionState->state.motionActive = 0;
            motionState->state.motionTimer = 0;
        }
    } else {
        motionState->state.motionTimer = 0;
    }
    motionWork->updateCount++;
}

Vec lbl_1_data_7A8 = {400.0f, 200.0f, 1500.0f};

/* Runs the prop's approach and repeated event phases from the outer sequence callback. */
s32 fn_1_B330(M675PropWork *work)
{
    M675PropWork *state = work;
    OMOBJ *target;
    M675PropWork *targetWork;
    s32 done = 0;
    switch (work->state.phase) {
    case 0:
        lbl_1_bss_31C = 1;
        work->state.phase++;
        work->state.phaseTimer = 0;
        work->state.actor.position = lbl_1_data_7A8;
        work->state.actor.rotation.y = 180.0f;

    case 1:
        work->state.phase++;
        work->state.phaseTimer = 0;
        work->state.movementPhase = 0;
        target = M675B330Find(1, 1);
        if (target != NULL) {
            targetWork = target->data;
            work->state.targetPosition = targetWork->state.actor.position;
            work->state.targetPosition.x += 140.0;
        }

    case 2:
        if (fn_1_CDF4(work, 1.089627742767334f,
                500.0f, 12, 0) == 0) {
            break;
        }
        work->state.phase++;
        work->state.phaseTimer = 0;
        work->state.movementPhase = 0;
        work->state.targetAngle = 270.0f;

    case 3:
        fn_1_C7E0(work, 0.30000001192092896f);
        if (++work->state.phaseTimer >= 30) {
            work->state.phase++;
            work->state.phaseTimer = 0;
            work->state.countdown = 12.0f;
            work->state.movementPhase = 0;
            work->state.targetAngle = 10.0f;
            fn_1_6210(&work->state.actor, 8, 1.0f);

        } else {
            break;
        }
    case 4:
        if (work->state.phaseTimer < 9) {
            work->state.displayOffset.x = (f32)(work->state.displayOffset.x +
                (0.2 * (-30.0 - (f64)work->state.displayOffset.x)));
        } else {
            work->state.displayOffset.x = (f32)(work->state.displayOffset.x +
                (0.2 * -(f64)work->state.displayOffset.x));
            fn_1_C7E0(work, 0.20000000298023224f);
        }
        if ((work->state.countdown -= 1.0) > 0.0) {
            break;
        }
        work->state.countdown = 8.0f;
        fn_1_D494(5, 0);
        if (++work->state.phaseTimer < 10) {
            break;
        }
        work->state.phase++;
        work->state.phaseTimer = 0;

    case 5:
        fn_1_C7E0(work, 0.20000000298023224f);
        if (lbl_1_bss_320 != 0) {
            work->state.phase++;
            work->state.phaseTimer = 0;
            fn_1_6210(&work->state.actor, 7, 1.0f);
            work->state.motionActive = 1;

        } else {
            break;
        }
    case 6:
        if (++work->state.phaseTimer > 240) {
            lbl_1_bss_318 = 1;
        }
        if (work->state.motionActive == 0) {
            work->state.phase++;
            work->state.phaseTimer = 0;
            lbl_1_bss_318 = 1;
            work->state.movementPhase = 0;
            target = M675B330Find(1, 1);
            if (target != NULL) {
                targetWork = target->data;
                work->state.targetPosition = targetWork->state.actor.position;
                work->state.targetPosition.x += 60.0;
                work->state.targetPosition.y += 150.0;
            }

        } else {
            break;
        }
    case 7:
        if (fn_1_CDF4(work, 1.089627742767334f,
                200.0f, 0, 1) == 0) {
            break;
        }
        work->state.phase++;
        work->state.phaseTimer = 0;
        work->state.airborne = 1;
        fn_1_D494(5, 1);
        work->state.movementPhase = 0;
        target = M675B330Find(1, 2);
        if (target != NULL) {
            targetWork = target->data;
            work->state.targetPosition = targetWork->state.actor.position;
            work->state.targetPosition.x -= 60.0;
            work->state.targetPosition.y += 300.0;
        }

    case 8:
        if (fn_1_CDF4(work, 1.089627742767334f,
                200.0f, 0, 0) == 0) {
            break;
        }
        work->state.phase++;
        work->state.phaseTimer = 0;
        work->state.airborne = 1;
        work->state.movementPhase = 0;
        work->state.targetPosition = state->initialPosition;
        fn_1_D494(6, 0);

    case 9:
        if (fn_1_CDF4(work, 1.089627742767334f,
                500.0f, 18, 0) == 0) {
            break;
        }
        work->state.phase++;
        work->state.phaseTimer = 0;
        work->state.movementPhase = 0;
        work->state.targetAngle = 0.0f;

    case 10:
        if (fn_1_C7E0(work, 0.30000001192092896f) == 0) {
            break;
        }
        work->state.phase++;
        work->state.phaseTimer = 0;
        lbl_1_bss_31C = 0;

    case 11:
        if (MgSeqModeGet() == 5) {
            work->state.phase++;
            work->state.phaseTimer = 0;
        } else {
            break;
        }

    default:
        done = 1;
        break;
    }
    return done;
}

M675PhaseCallback lbl_1_data_7E4[4] = {fn_1_BE60, fn_1_C0B8, fn_1_C29C, fn_1_C0B8};

/* Selects and runs the prop's next scripted action until sequence mode 7 settles. */
s32 fn_1_BA58(M675PropWork *work)
{
    M675PropWork *state = work;
    s32 ready = 0;
    s32 callbackCount = 4;
    s32 nextCallback;

    switch (work->state.phase) {
    case 0:
        work->state.phase++;
        work->state.phaseTimer = 0;
        state->controlValue = 0;

    case 1:
        if (fn_1_A50() != 0) {
            work->state.phase = 3;
            goto done;
        }
        work->state.phase++;
        work->state.actionPhase = 0;
        nextCallback = state->controlValue + 1;
        /* In the normal 0..3 action cycle, the increment makes this equality test always false. */
        if (state->controlValue == nextCallback) {
            nextCallback++;
        }
        if (nextCallback >= callbackCount) {
            nextCallback = 0;
        }
        state->controlValue = nextCallback;

    case 2:
        if (lbl_1_data_7E4[state->controlValue](work) != 0) {
            work->state.phase--;
        }

    case 3:
        if (MgSeqModeGet() != 7 || 0.0 != work->state.actor.velocity.y) {
            goto done;
        }
        work->state.actor.velocity.x = 0.0f;
        work->state.actor.velocity.y = 0.0f;
        work->state.actor.velocity.z = 0.0f;
        fn_1_6210(&work->state.actor, 0, 1.0f);
        goto finished;
    default:
        goto finished;
    }
finished:
    ready = 1;
done:
    return ready;
}

extern s32 fn_1_BE60(M675PropWork *work);

Vec lbl_1_data_7F4[1] = {{50.0f, 180.0f, 0.0f}};

/* Moves the prop to its waiting point, then turns and walks until it arrives. */
s32 fn_1_BBDC(M675PropWork *work)
{
    s32 done = 0;

    switch (work->state.phase) {
    case 0:
        work->state.phase++;

    case 1:
        if (fn_1_4A48(&work->state.actor.position, lbl_1_data_7F4) > 500.0) {
            work->state.phase++;
        } else {
            work->state.phase += 3;
        }
        break;
    case 2:
        work->state.phase++;
        work->state.movementPhase = 0;
        work->state.targetPosition = lbl_1_data_7F4[0];

    case 3:
        if (fn_1_CDF4(work, 1.089627742767334f,
                800.0f, 18, 1) == 0) {
            break;
        }
        work->state.phase++;

    case 4:
        work->state.phase++;
        work->state.positionSyncEnabled = 0;
        work->state.movementPhase = 0;
        work->state.targetAngle = 0.0f;
        work->state.targetPosition = lbl_1_data_7F4[0];

    case 5:
        if (fn_1_C94C(work, 6.666666507720947f) == 0) {
            break;
        }
        work->state.phase++;

    default:
        done = 1;
        break;
    }
    return done;
}

/* Waits for all players to arrive, starts motion 6, then plays a positioned sound after 78
 * updates. */
s32 fn_1_BD84(M675PropWork *sequence)
{
    s32 done = 0;

    switch (sequence->state.phase) {
    case 0:
        sequence->state.phase++;

    case 1:
        if (fn_1_B6C() == 0) {
            break;
        }
        sequence->state.phase++;
        sequence->state.phaseTimer = 0;
        fn_1_6210(&sequence->state.actor, 6, 1.0f);

    case 2:
        if (++sequence->state.phaseTimer < 78) {
            break;
        }
        sequence->state.phase++;
        sequence->state.phaseTimer = 0;
        fn_1_3C70(935, &sequence->state.actor.position);

    default:
        done = 1;
        break;
    }
    return done;
}

Vec lbl_1_data_800 = {0.0f, 180.0f, 0.0f};

/* Runs one alternating side-to-side prop movement action. */
s32 fn_1_BE60(M675PropWork *work)
{
    s32 ready = 0;
    f32 direction = 2.0f;
    f32 turnAngle;
    f32 offset;

    switch (work->state.actionPhase) {
    case 0:
        work->state.actionPhase++;
        work->state.movementPhase = 0;
        work->state.targetPosition = lbl_1_data_800;

    case 1:
        if (fn_1_CDF4(work, 1.089627742767334f, 800.0f,
                18, 1) == 0) {
            break;
        }
        work->state.actionPhase++;
        work->state.actionTimer = 0;
        work->state.movementPhase = 0;
        work->state.targetAngle = 0.0f;
        if (((s32)frand() & 1) != 0) {
            work->state.actionPhase++;
        }

    case 2:
        direction = -direction;

    case 3:
        turnAngle = (f32)work->state.actionTimer * direction;
        offset = 100.0 * (3.0 *
            sin((3.141592653589793 * turnAngle) / 180.0));
        work->state.targetPosition.x = lbl_1_data_800.x + offset;
        fn_1_C94C(work, 6.666666507720947f);

    case 4:
        if (++work->state.actionTimer < 180) {
            break;
        }
        work->state.actionPhase = -1;
        work->state.actor.velocity.x = 0.0f;
        work->state.actor.velocity.y = 0.0f;
        work->state.actor.velocity.z = 0.0f;
        fn_1_6210(&work->state.actor, 0, 1.0f);

    default:
        ready = 1;
        break;
    }
    return ready;
}

Vec lbl_1_data_80C = {-500.0f, 0.0f, -1000.0f};

/* After jumping into place, plays a walking animation in place for 120 updates with footstep
 * audio. */
s32 fn_1_C0B8(M675PropWork *work)
{
    s32 ready = 0;

    switch (work->state.actionPhase) {
    case 0:
        work->state.actionPhase++;
        work->state.movementPhase = 0;
        work->state.targetPosition = lbl_1_data_80C;

    case 1:
        if (fn_1_CDF4(work, 1.089627742767334f, 800.0f, 18, 1) == 0) {
            break;
        }
        work->state.actionPhase++;
        work->state.movementPhase = 0;
        work->state.targetAngle = 30.0f;

    case 2:
        if (fn_1_C7E0(work, 0.20000000298023224f) == 0) {
            break;
        }
        work->state.actionPhase++;
        work->state.actionTimer = 0;
        work->state.soundTimer = 0;
        work->state.movementPhase = 0;

    case 3:
        fn_1_6210(&work->state.actor, 2, 1.0f);
        if (--work->state.soundTimer <= 0) {
            /* The initial pan consumes fn_1_32EC's result even though that wrapper discards its pan
             * query's return value. */
            HuAudFXPlayVolPan(MSM_SE_GUIDE_11, 72,
                (s16)fn_1_32EC(&work->state.actor.position));
            work->state.soundTimer = 72;
        }

    case 4:
        if (++work->state.actionTimer < 120) {
            break;
        }
        work->state.actionPhase = -1;
        work->state.actor.velocity.x = 0.0f;
        work->state.actor.velocity.y = 0.0f;
        work->state.actor.velocity.z = 0.0f;
        fn_1_6210(&work->state.actor, 0, 1.0f);

    default:
        ready = 1;
        break;
    }
    return ready;
}

/* Jumps toward a recorded path sample, requests the playback prop's bounce, then follows that
 * prop's position while turning. */
s32 fn_1_C29C(M675PropWork *work)
{
    s32 done = 0;
    s32 frameDelta;
    Vec sampledRotation;
    f32 delta = 1.089627742767334f;

    switch (work->state.actionPhase) {
    case 0:
        work->state.actionPhase++;
        M675SequenceSample(0, &work->state.targetPosition, &sampledRotation);
        work->state.targetAngle = fn_1_49AC(&work->state.actor.position,
            &work->state.targetPosition);

    case 1:
        work->state.actor.rotation.y =
            fn_1_47DC(work->state.actor.rotation.y, work->state.targetAngle, 0.20000000298023224f);
        if (fabs((f64)fn_1_48F8(work->state.actor.rotation.y, work->state.targetAngle)) > 3.0) {
            break;
        }
        work->state.actor.rotation.y = work->state.targetAngle;
        work->state.actionPhase++;
        work->state.movementPhase = 0;
        frameDelta = fn_1_4C0C(&work->state.actor.position,
            &work->state.targetPosition, delta, 800.0f,
            &work->state.actor.velocity);
        M675SequenceSample(frameDelta, &work->state.targetPosition,
            &sampledRotation);

    case 2:
        if (work->state.actor.velocity.y < 0.0 &&
            (work->state.actor.position.y - work->state.targetPosition.y) < 120.0) {
            M675PlaybackBounceRequestSet(1);
        }
        if (fn_1_CDF4(work, delta,
                800.0f, 0, 0) != 0) {
            work->state.actionPhase++;
            work->state.actionTimer = 0;
            work->state.positionSyncEnabled = 1;

        } else {
            break;
        }
    case 3:
        if (++work->state.actionTimer >= 18) {
            work->state.actionPhase++;
            work->state.movementPhase = 0;
            work->state.targetAngle = 30.0f;

        } else {
            break;
        }
    case 4:
        if (fn_1_C7E0(work, 0.20000000298023224f) != 0) {
            work->state.actionPhase++;
            work->state.actionTimer = 0;
            work->state.movementPhase = 0;

        } else {
            break;
        }
    case 5:
        fn_1_6210(&work->state.actor, 1, 1.0f);

    case 6:
        if (++work->state.actionTimer >= 120) {
            work->state.actionPhase = -1;
            work->state.actor.velocity.x = 0.0f;
            work->state.actor.velocity.y = 0.0f;
            work->state.actor.velocity.z = 0.0f;
            fn_1_6210(&work->state.actor, 0, 1.0f);
        } else {
            break;
        }

    default:
        done = 1;
        break;
    }
    return done;
}

/* Turns toward the stored target yaw and reports when the turn is complete. */
s32 fn_1_C7E0(M675PropWork *work, f32 speed)
{
    s32 done = 0;

    switch (work->state.movementPhase) {
    case 0:
        work->state.movementPhase++;
        if (fabs((double)fn_1_48F8(work->state.actor.rotation.y, work->state.targetAngle)) > 30.0) {
            fn_1_6210(&work->state.actor, 3, 1.0f);
        }

    case 1:
        work->state.actor.rotation.y =
            fn_1_47DC(work->state.actor.rotation.y, work->state.targetAngle, speed);
        if (fabs((double)fn_1_48F8(work->state.actor.rotation.y, work->state.targetAngle)) > 3.0) {
            break;
        }
        work->state.actor.rotation.y = work->state.targetAngle;
        fn_1_6210(&work->state.actor, 0, 1.0f);

    default:
        done = 1;
        break;
    }
    return done;
}

/* Steers toward the target, then stops and reports completion within 30 horizontal world units. */
s32 fn_1_C94C(M675PropWork *work, f32 speed)
{
    s32 stopMotion = 0;
    s32 movingMotion = 3;
    f32 deltaX;
    f32 deltaZ;
    f32 distance;
    f32 targetAngle;
    f32 targetVelocityX;
    f32 targetVelocityZ;

    deltaX = work->state.targetPosition.x - work->state.actor.position.x;
    deltaZ = work->state.targetPosition.z - work->state.actor.position.z;
    distance = sqrtf(deltaX * deltaX + deltaZ * deltaZ);
    if (distance < 30.0) {
        work->state.actor.velocity.x *= 0.8;
        work->state.actor.velocity.z *= 0.8;
        work->state.actor.rotation.y =
            fn_1_47DC(work->state.actor.rotation.y, work->state.targetAngle, 0.20000000298023224f);
        if (fabs((f64)fn_1_48F8(work->state.actor.rotation.y, work->state.targetAngle)) >
            3.0) {
            return 0;
        }
        fn_1_6210(&work->state.actor, stopMotion, 1.0f);
        work->state.actor.rotation.y = work->state.targetAngle;
        work->state.actor.velocity.x = 0.0f;
        work->state.actor.velocity.y = 0.0f;
        work->state.actor.velocity.z = 0.0f;
        return 1;
    }

    targetAngle = (f32)(180.0 *
        (atan2((f64)deltaX, (f64)deltaZ) /
         3.141592653589793));
    work->state.actor.rotation.y = fn_1_47DC(work->state.actor.rotation.y, targetAngle, 0.3f);
    if (distance < 50.0) {
        speed *= 0.5;
    }
    targetVelocityX = (f32)(speed *
        sin((3.141592653589793 * targetAngle) / 180.0));
    work->state.actor.velocity.x = (f32)(work->state.actor.velocity.x +
        (0.3 * (f64)(targetVelocityX - work->state.actor.velocity.x)));
    targetVelocityZ = (f32)(speed *
        cos((3.141592653589793 * targetAngle) / 180.0));
    work->state.actor.velocity.z = (f32)(work->state.actor.velocity.z +
        (0.3 * (f64)(targetVelocityZ - work->state.actor.velocity.z)));
    work->state.movementSoundDistance = (f32)(work->state.movementSoundDistance + (f32)speed);
    if (work->state.movementSoundDistance > 100.0) {
        work->state.movementSoundDistance -= 100.0;
        /* The pan consumes fn_1_32EC's result even though that wrapper discards its pan
         * query's return value. */
        HuAudFXPlayVolPan(MSM_SE_GUIDE_09, 72,
            fn_1_32EC(&work->state.actor.position));
    }
    fn_1_6210(&work->state.actor, movingMotion, 1.0f);
    return 0;
}

/* Runs the prop's jump arc, lands it at the target, and emits its landing dust. */
s32 fn_1_CDF4(M675PropWork *work, f32 delta, f32 step,
    s32 count, s32 angleMode)
{
    Vec effectPosition;
    s32 finished = 0;

    switch (work->state.movementPhase) {
    case 0:
        work->state.movementPhase++;
        work->state.movementTimer = 0;
        work->state.targetAngle =
            fn_1_49AC(&work->state.actor.position, &work->state.targetPosition);

    case 1:
        if (angleMode != 0) {
            work->state.actor.rotation.y = fn_1_47DC(work->state.actor.rotation.y,
                                                     work->state.targetAngle, 0.20000000298023224f);
            if (fabs((f64)fn_1_48F8(work->state.actor.rotation.y, work->state.targetAngle)) > 3.0) {
                break;
            }
            work->state.actor.rotation.y = work->state.targetAngle;
        }
        work->state.movementPhase++;
        work->state.movementTimer = 7;
        fn_1_6210(&work->state.actor, 4, 1.0f);

    case 2:
        if (angleMode == 0) {
            work->state.actor.rotation.y = fn_1_47DC(work->state.actor.rotation.y,
                                                     work->state.targetAngle, 0.20000000298023224f);
        }
        if (--work->state.movementTimer > 0) {
            break;
        }
        work->state.movementPhase++;
        work->state.movementTimer =
            fn_1_4C0C(&work->state.actor.position, &work->state.targetPosition, delta, step,
                      &work->state.actor.velocity);
        work->state.airborne = 1;
        work->state.positionSyncEnabled = 0;
        fn_1_3C70(993, &work->state.actor.position);

    case 3:
        if (angleMode == 0) {
            work->state.actor.rotation.y = fn_1_47DC(work->state.actor.rotation.y,
                                                     work->state.targetAngle, 0.20000000298023224f);
        }
        work->state.actor.velocity.y -= delta;
        if (work->state.movementTimer == 7) {
            fn_1_6210(&work->state.actor, 5, 1.0f);
        }
        if (--work->state.movementTimer > 0) {
            break;
        }
        work->state.movementPhase++;
        work->state.movementTimer = 0;
        work->state.airborne = 0;
        work->state.actor.position = work->state.targetPosition;
        work->state.actor.velocity.x = 0.0f;
        work->state.actor.velocity.y = 0.0f;
        work->state.actor.velocity.z = 0.0f;
        fn_1_6210(&work->state.actor, 5, 1.0f);
        if (work->state.actor.position.y < 300.0) {
            effectPosition = work->state.actor.position;
            effectPosition.y += 10.0;
            CharEffectDustCreate(-1, 1.0f, &effectPosition);
        }

    case 4:
        if (++work->state.movementTimer < count) {
            break;
        }
        work->state.movementPhase++;

    default:
        finished = 1;
        break;
    }
    return finished;
}

/* Applies a pending sequence motion and its configured playback speed in the prop callback. */
void fn_1_D148(OMOBJ *object)
{
    M675PropWork *state;
    M675PropWork *work = object->data;

    state = work;
    if (state->state.actor.requestedMotion != -1) {
        M675SequenceMotionSetPlain(object, state->state.actor.requestedMotion);
        Hu3DMotionSpeedSet(object->mdlId[0], state->state.actor.motionSpeed);
    }
}

/* Applies the prop transform and clears motion tracking after its blend finishes. */
void fn_1_D278(OMOBJ *object)
{
    M675PropWork *initialWork = object->data;
    M675PropWork *workForSetup = initialWork;
    s32 initialModelId = object->mdlId[0];
    M675PropWork *transformWork;
    M675PropWork *transformState;
    s32 transformModelId;
    f32 angle;
    f32 reducedAngle;
    f32 storedAngle;
    M675PropWork *motionWork;
    M675PropWork *motionState;
    s32 resetShift;
    s32 motionModelId;
    s16 shiftedMotion;

    fn_1_74FC(&workForSetup->state.actor, initialModelId);
    transformWork = object->data;
    transformState = transformWork;
    transformModelId = object->mdlId[0];
    angle = transformState->state.actor.rotation.y;
    reducedAngle = (f32)fmod((f64)angle, 360.0);
    storedAngle = reducedAngle;
    transformState->state.actor.rotation.y = storedAngle;
    Hu3DModelPosSetV(transformModelId, &transformState->state.actor.position);
    Hu3DModelRotSetV(transformModelId, &transformState->state.actor.rotation);

    motionWork = object->data;
    motionState = motionWork;
    if (motionState->state.motionActive != 0) {
        motionModelId = object->mdlId[0];
        if (Hu3DMotionEndCheck(motionModelId) &&
            Hu3DMotionShiftIDGet(motionModelId) < 0) {
            resetShift = 1;
        } else {
            resetShift = 0;
        }
        if (resetShift != 0) {
            motionState->state.motionActive = 0;
            motionState->state.motionTimer = 0;
        }
    } else {
        motionState->state.motionTimer = 0;
    }
    motionWork->updateCount++;
}

s32 fn_1_D3BC(void) {
    return lbl_1_bss_31C;
}

s32 fn_1_D3CC(void) {
    return lbl_1_bss_318;
}

OMOBJ **lbl_1_data_834[5] = {
    &lbl_1_bss_32C, &lbl_1_bss_328, &lbl_1_bss_324, &lbl_1_bss_32C, &lbl_1_bss_32C
};

/* Queues a position and player index in the first free record of the chosen emitter pool. */
void fn_1_D3DC(s32 type, s32 index, Vec *position)
{
    OMOBJ *manager;
    M675EmitterWork *storage;
    M675EmitterRecord *request;
    s32 requestLimit;
    s32 i;

    if (type >= 0 && (u32) type < 5U) {
        manager = *lbl_1_data_834[type];
        requestLimit = manager->work[0];
        storage = manager->data;
        request = storage->records;
        for (i = 0; i < requestLimit; i++, request++) {
            if (request->state == 0) {
                request->state = 1;
                request->enabled = 0;
                request->type = type;
                request->playerIndex = index;
                request->position = *position;
                return;
            }
        }
    }
}

/* Sets the control value and alternate-motion start on the selected type-1 prop. */
void fn_1_D494(s32 type, s32 value)
{
    M675PropWork *resultWork;
    M675PropWork *work;
    OMOBJ **entry;
    OMOBJ *candidate;
    OMOBJ *object;
    OMOBJ *found;
    s32 i;
    s32 selectedValue;

    entry = lbl_1_bss_334;
    i = 0;
    while (i < 128) {
        candidate = *entry;
        if (candidate != NULL) {
            work = candidate->data;
            if (work->parameters->type == 1 &&
                work->id == (type & 3)) {
                found = candidate;
                goto foundObject;
            }
        }
        i++;
        entry++;
    }
    found = NULL;
foundObject:
    object = found;
    if (object != NULL) {
        resultWork = object->data;
        if (type < 4) {
            selectedValue = 1;
        } else {
            selectedValue = 2;
        }
        resultWork->controlValue = selectedValue;
        resultWork->alternateMotionStart = value;
    }
}
