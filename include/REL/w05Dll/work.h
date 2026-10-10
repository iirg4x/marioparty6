/* Model motion and light transitions used by the board object callback. */
#ifndef W05_WORK_H
#define W05_WORK_H
#include "dolphin/types.h"
#include "game/hu3d.h"

typedef struct W05ModelMotion_s {
    s32 modelId;
    s32 state;
    s32 elapsed;
    s32 delay;
    s32 direction;
    s32 status;
    f32 angle;
    f32 secondAngle;
    s32 delayTimer;
    HuVecF position;
    HuVecF displacement;
    HuVecF rotation;
    HuVecF rotationDelta;
} W05_MODEL_MOTION;

typedef struct W05PlayerMotion_s {
    s32 playerNo;
    s32 state;
    s32 stateFrames;
    s32 jumpHoldFrames;
    s32 reactionFrames;
    s32 stickX;
    s32 stickY;
    s32 buttons;
    s32 buttonsDown;
    f32 facingDirection;
    f32 movementHistory[8];
    s32 movementIndex;
    HuVecF position;
    HuVecF velocity;
    HuVecF rotation;
    s32 comJumpWait;
    s32 comJumpHold;
    s32 comMoveDirection;
    s32 comMoveWait;
    s32 comMoveFrames;
} W05_PLAYER_MOTION;

typedef struct W05FlyingModel_s {
    s32 modelId;
    s32 state;
    s32 type;
    s32 elapsed;
    s32 duration;
    s32 delay;
    s32 motionState;
    f32 drag;
    HuVecF position;
    HuVecF rotation;
    HuVecF rotationSpeed;
    HuVecF velocity;
} W05_FLYING_MODEL;

/* One sea-surface vertex: each axis has its own sine wave; y uses base.y - amplitude.y as its
 * baseline. */
typedef struct W05SeaWave_s {
    HuVecF phase; /* Degrees, advanced by speed each frame and wrapped at 360. */
    HuVecF amplitude;
    HuVecF speed;
    HuVecF base;
} W05_SEA_WAVE;

typedef struct W05TexCoord_s {
    f32 s;
    f32 t;
} W05_TEX_COORD;

typedef struct W05BoardWork_s {
    union {
        u8 modelSetupState[48];
        struct {
            s32 backgroundAttachmentModel;
            u8 modelSetupData[28];
            s32 dockModels[2];
            s32 dockSpaces[2];
        };
    };
    s32 lampModels[2]; /* Shown and faded in at night while lampEnabled is set. */
    union {
        u8 modelSetupRest[16];
        struct {
            u8 dockSetupData[8];
            s32 backgroundMotionModel;
            u8 dockSetupFlags[4];
        };
    };
    s32 rockingModel;
    s32 rockingEffectModel;
    s32 seaModel; /* Its HSF vertices and texture coordinates are rewritten every frame. */
    union {
        u8 modelSetupTail[16];
        struct {
            u8 seaSetupData[8];
            s32 boardEffectModel;
            u8 seaSetupFlags[4];
        };
    };
    s32 raftModel;
    s32 raftParentModel;
    s32 raftMotionFrame;
    s32 raftMotionFrames;
    f32 raftMotionProgress;
    f32 raftPreviousProgress;
    f32 raftMotionBoost;
    s32 raftEventState;
    union {
        u8 remainingObjectState[8];
        struct {
            s32 raftEventRequested;
            s32 raftArrivalModel;
        };
    };
    HuVecF movingModelPositions[4];
    HuVecF effectOrigin;
    HuVecF effectForward;
    HuVecF effectBackward;
    f32 playerFacingAngle;
    f32 effectForwardAngle;
    f32 effectBackwardAngle;
    W05_PLAYER_MOTION *playerMotion;
    W05_MODEL_MOTION *motion;
    W05_FLYING_MODEL *flyingModels;
    s32 collectedCoins;
    union {
        u8 eventState[12];
        s32 eventModels[3];
    };
    HuVecF lightPosition;
    HuVecF lightDirection;
    s32 lightId;
    HuVecF lightRotation;
    HuVecF lightTargetRotation;
    s32 lightElapsed;
    s32 lightDuration;
    s32 lightMoving;
    f32 lampAlpha; /* 0 (hidden) to 1 (opaque), 0.1 per frame. */
    s32 lampEnabled;
    s32 rockingElapsed;
    s32 rockingState;
    s32 rockingFinished;
    s32 rockingResetRequested;
    HuVecF rockingPosition;
    HuVecF rockingEffectPosition;
    s32 firstHookModel;
    s32 firstHookAttachmentModel;
    s32 secondHookModel;
    s32 secondHookModels[3];
    s32 risingEffectModel;
    s32 secondHookAttachmentModel;
    s32 hookModels[3];
    s32 firstHookIndex;
    s32 secondHookIndex;
    s32 hookSide;
    s32 firstHookMoving;
    f32 firstHookPhaseWeight;
    f32 secondHookPhaseWeight;
    f32 firstHookSway[3]; /* Degrees: x tilt, height bob and z tilt; each changes by up to 3 degrees
                           * in either direction per frame. */
    f32 secondHookSway[3];
    f32 hookMovementScale;
    f32 secondHookSwing; /* Extra z swing of the second hook, decaying by 1% per frame. */
    f32 movementFactorX;
    f32 movementFactorZ;
    s32 particlesEnabled;
    s32 splashTimer; /* Frames to the next splash under the second hook while it moves. */
    s32 secondHookMoving;
    s32 firstHookPlayer; /* Player carried on the first hook, or -1; placed at
                          * firstHookPlayerPoint. */
    s32 firstHookPlayerPoint;
    s32 firstHookObject; /* Board object carried on the first hook, or -1. */
    s32 firstHookObjectPoint;
    s32 secondHookPlayer;
    s32 secondHookPlayerPoint;
    s32 secondHookObject;
    s32 secondHookObjectPoint;
    s32 linkedHookModels[5];
    s32 linkedPunchModels[5];
    s32 linkedSpaces[5];
    s32 eventModel;
    s32 eventLampModel;
    s32 eventEnabled;
    u8 boardEventState[8];
    W05_TEX_COORD *seaTexCoordsAlt; /* Flushed with the sea's other buffers every frame. */
    u8 seaState[4];
    W05_TEX_COORD *seaTexCoordBase; /* The model's own texture coordinates. */
    W05_TEX_COORD *seaTexCoords; /* seaTexCoordBase shifted by the flow offset. */
    u8 seaBufferState[8];
    HuVecF *seaVertices;
    s32 seaVertexCount;
    f32 seaFlowOffsetS; /* Texture scroll, moved against the flow direction each frame. */
    f32 seaFlowOffsetT;
    s32 seaFlowTimer;
    s32 seaFlowDuration; /* Frames of one flow cycle: 10 to 20 seconds, chosen again each cycle. */
    f32 seaFlowAngle; /* Degrees: 45 plus up to seaFlowAmplitude over the cycle. */
    f32 seaFlowAmplitude;
    f32 seaFlowSpeedS;
    f32 seaFlowSpeedT;
    HuVecF particleBoundsStart; /* Minimum x/z and initial height of the sampled geometry. */
    HuVecF particleBoundsSize; /* Width/depth of the sampled geometry; height is zero. */
    W05_SEA_WAVE *seaWaves; /* One per sea vertex. */
    union {
        u8 particleEventState[8];
        s32 particleInitState[2];
    };
    s32 skipRequested;
    s32 skipEnabled;
} W05_BOARD_WORK;
#endif
