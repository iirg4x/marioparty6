#ifndef REL_M675DLL_PROP_WORK_H
#define REL_M675DLL_PROP_WORK_H

#include "REL/minigame_ground_effect.h"
#include "REL/minigame_prop_params.h"
#include "REL/m675Dll/actor_state.h"

typedef struct M675EmitterRecord M675EmitterRecord;

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
    u8 unkD0[4]; /* No code names this field. */
    s32 movementPhase; /* Substate of a turn, walk, or jump routine. */
    s32 movementTimer; /* Frames remaining or elapsed in that movement substate. */
    u8 unkDC[32]; /* No code names this field. */
    s32 motionActive; /* Nonzero while the selected motion is being tracked. */
    s32 unk100; /* The game's code never reads this field. */

    u8 unk104[4]; /* No code names this field. */
    s32 airborne; /* Whether display positioning uses the airborne depth bands. */
    u8 unk10C[28]; /* No code names this field. */
    s32 positionSyncEnabled; /* Follows the paired sequence actor's position when set. */
    Vec displayOffset; /* Temporary offset added when placing the model. */
    f32 movementSoundDistance; /* Accumulated requested walk speed, used to space walking sounds. */
    MinigameGroundEffect effect; /* Ground-ring registration for the prop. */
    u8 unk148[108]; /* No code names this field. */
} M675PropState;

typedef struct M675PropWork {
    M675PropState state;
    MinigamePropParams * parameters;
    s32 id;
    s32 unk1BC; /* The game's code never reads this field. */
    Vec initialPosition;
    /* Outer sequence phase, delayed-start frames, and phase timer. */
    s32 sequencePhase;
    s32 delay;
    s32 sequenceTimer;
    u8 unk1D8[4]; /* No code names this field. */
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
    u8 unk1EC[152]; /* No code names this field. */
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


typedef struct M675EmitterRecord {
    /* Effect category, visibility/interaction state, and animation timer. */
    s32 type, enabled, state;
    f32 timer;
    u8 unk10[4]; /* No code names this field. */
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

#endif
