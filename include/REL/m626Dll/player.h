/* Player model state, motion tuning, and proximity registration. */
#ifndef REL_M626DLL_PLAYER_H
#define REL_M626DLL_PLAYER_H

#include "REL/m626Dll/objects.h"

struct M626ObjectWork_s;
struct M626Entry;
struct M626SceneryParameters;
typedef void (*M626ControlUpdate)(struct M626ObjectWork_s *);

typedef struct M626MotionProfile {
    s32 closeMotion, slowMotion;
    f32 closeMotionSpeed;
    s32 farMotion;
    f32 farMotionSpeed;
} M626MotionProfile;

typedef struct M626PlayerConfig {
    f32 jumpHeight, modelHeight, heightFactor, depthFactor;
    f32 movementScale, horizontalSpeed, startSpeed;
    M626MotionProfile profiles[2];
    s32 effectTargetA, effectTargetB, finishMotion, turnMotion, winMotion, loseMotion;
} M626PlayerConfig;

typedef struct M626ObjectWork_s {
    u32 collisionFlags, collisionMask, collisionType;
    s32 playerIndex, collisionState;
    void *owner, *collisionPartner;
    void (*update)(OMOBJ *);
    s32 (*draw)(struct M626Entry *, struct M626Entry *);
    void *positionData, *offset;
    Vec worldStart, worldEnd;
    MtxPtr matrix;
    Vec segmentStart, segmentEnd;
    f32 radius, collisionMode;
    u32 key, groupIndex;
    s32 variation, flags;
    Mtx rotationMatrix;
    Vec rotationAxis;
    f32 tiltAngle;
    s32 effectComplete;
    Vec position, velocity, rotation;
    f32 travelDistance;
    u32 modelAttributes;
    s32 controlWord;
    f32 targetDistance, movementSpeed;
    s32 motionIndex, motionFrame, requestedMotion;
    f32 motionSpeed;
    s32 statePhase, stateTimer;
    Vec targetPosition;
    f32 targetAngle;
    s32 updateCounter, playerType;
    u32 computerDifficulty;
    s32 controlPhase;
    u8 unknown_12C[4];
    s32 movementState;
    u8 unknown_134[12];
    M626ControlUpdate controlUpdate;
    s32 controlState, controlTimer;
    f32 controlProgress;
    s32 controlAmount, controlDirection, reverseCount, controlCount, outputAngle;
    f32 secondaryAngle;
    s32 motionActive, motionTimer, motionPaused, active;
    u8 unknown_178[4];
    s32 transitionTimer, settled;
    Vec savedPosition;
    f32 effectProgress, acceleration, speedScale;
    s32 firstTimer, cooldown, jitterIndex, status, eventSent;
    s16 effectModel;
    f32 effectRotation, effectRise;
    s32 motionState, modelId;
    f32 pulseProgress;
    M626PlayerConfig config;
    union {
        s32 playerIndex;
        struct M626SceneryParameters *parameters;
    } actor;
    union {
        u32 character;
        s32 sceneryId;
    } identity;
    s32 updatePhase;
    union {
        struct {
            u8 unknown_230[4];
            s32 voiceCount, volumeEnabled;
        } player;
        Vec startPosition;
    } detail;
} M626ObjectWork;

extern M626ObjectWork *lbl_1_bss_2D4[8];

#endif
