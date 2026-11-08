#ifndef M699_WORK_RECORD_PREFIX_V3_H
#define M699_WORK_RECORD_PREFIX_V3_H

#include "dolphin/mtx.h"
#include "dolphin/types.h"
#include "game/object.h"

typedef struct M699MovementProfile {
    f32 searchRangeScale;
    f32 pursuitRangeScale;
    f32 directionProbabilityAndRetargetRate;
    f32 nearbyActionProbability;
    f32 turnProbability;
    f32 transitionTimingAccuracy;
} M699MovementProfile;

extern M699MovementProfile lbl_1_data_5F4[];

struct M699CharacterWorkPrefix;
typedef void (*M699CharacterWorkCallback)(struct M699CharacterWorkPrefix *work);

/*
 * The character work allocation in fn_1_62CC is 0x2A8 bytes and is cleared
 * in full. Unidentified intervals retain their byte extents.
 */
typedef struct M699PlayerMotionData_s {
    f32 proximityRadius;
    f32 height;
    f32 movementSpeed;
    f32 verticalSpeed;
    f32 acceleration;
    f32 distanceScale;
    f32 modelScale;
    f32 dustEffect;
    f32 blendScale;
    f32 blendSpan;
    f32 currentBlend;
    f32 scales[7];
    s32 motion;
    s32 slowMotion;
    f32 closeMotionSpeed;
    s32 farMotion;
    f32 farMotionSpeed;
    s32 firstTarget;
    s32 secondTarget;
    f32 otherStartSpeed;
    s32 thirdTarget;
    f32 otherEndSpeed;
    s32 motionA;
    s32 segmentPlayer;
    s32 actionMotion;
    s32 captureMotion;
    s32 chaseMotion;
    s32 motionB;
    s32 exitMotion;
    s32 activeMotion;
    s32 returnMotion;
    s32 winMotion;
    s32 loseMotion;
} M699PlayerMotionData;

struct M699MotionCallbackView;
struct M699MotionEventView;

typedef struct M699CharacterWorkPrefix {
    s32 state;
    s32 stateLimit;
    s32 controllerValue;
    s32 playerIndex;
    s32 controllerCounter;
    void *object;
    u8 target_bytes_018[0x4];
    void (*update)(OMOBJ *);
    u8 target_bytes_020[0x4];
    s32 (*callback)(struct M699MotionCallbackView *motion, struct M699MotionEventView *event);
    Vec *initialPositionAddress;
    Vec *positionAddress;
    Vec savedPosition;
    f32 boundary;
    f32 verticalOffset;
    f32 scaledHeight;
    f32 heightAdjustment;
    u8 target_bytes_04c[0x20];
    u32 groupId;
    u32 groupFlag;
    s32 groupNumber;
    s32 readPadInput;
    Vec initialPosition;
    Vec position;
    Vec rotation;
    s32 padDown;
    s32 buttonsHeld;
    f32 outputHeading;
    f32 outputSpeed;
    s32 motionState;
    s32 motionTime;
    s32 motion;
    f32 motionSpeed;
    s32 phaseState;
    s32 phaseTimer;
    s32 characterNumber;
    s32 controllerType;
    s32 characterMotionState;
    u8 target_bytes_d4[4];
    s32 arrivalFlag;
    Vec destinationPosition;
    f32 currentAngle;
    M699CharacterWorkCallback callback_ec;
    s32 callbackState;
    s32 callbackStateTimer;
    f32 callbackValue;
    s32 transitionDuration;
    s32 randomTurn;
    s32 avoidanceTimer;
    f32 avoidanceAngle;
    f32 requestedHeading;
    f32 previousHeading;
    f32 previousAngle;
    f32 requestedSpeed;
    f32 previousSpeed;
    s32 timer;
    s32 resetCounter;
    s32 actionActive;
    s32 updateFlag;
    s32 stateFlag;
    s32 lowerMotionTriggered;
    s32 upperMotionTriggered;
    Vec spawnPosition;
    struct M699CharacterWorkPrefix *collisionSource;
    int countdown;
    f32 requestedBlend;
    f32 blendRate;
    s32 soundPlayed;
    s32 soundPlaybackTimer;
    s32 characterEffectPlayed;
    f32 acceleration;
    f32 motionAdjustment;
    s32 actMode;
    s32 actionFrame;
    f32 targetAngle;
    s32 spawnCount;
    s32 targetSlot;
    s32 direction;
    s32 cooldown;
    s32 actionTimer;
    struct M699CharacterWorkPrefix *relatedObjects[2];
    s32 animating;
    s32 modelId;
    f32 animationProgress;
    s32 selected;
    s32 selectionValue1;
    s32 selectionValue2;
    s32 stateFrame;
    Vec targetPosition;
    s32 pointerMotionActive;
    s32 pointerMotionState;
    s32 pointerMotionTimer;
    s32 itemState;
    s32 itemModelId;
    Vec modelObjectPosition; /* Output of Hu3DModelObjPosGet. */
    f32 motionProgress;
    f32 motionAngle;
    f32 launchScale;
    s32 motionValue;
    M699PlayerMotionData motionData;
    s32 groupNumberCopy;
    s32 characterNumberCopy;
    s32 initializationCount;
    s32 additionalStateA;
    s32 additionalStateB;
    s32 sameGroup;
    s32 additionalStateC;
    s32 initialized;
} M699CharacterWorkPrefix;

typedef struct M699MotionWorkPrefix {
    s32 immovable;
    Vec position;
    u8 target_bytes_010[0xC]; /* Unknown target bytes +0x10..+0x1C. */
    Vec field_01C;
} M699MotionWorkPrefix;

typedef struct M699VectorRecordPrefix {
    u8 target_bytes_000[0x4]; /* Unknown target bytes +0x0..+0x4. */
    Vec position;
} M699VectorRecordPrefix;

typedef struct M699DistancePairResult {
    void *firstRecord;
    void *secondRecord;
    f32 distance;
} M699DistancePairResult;

typedef struct M699CollisionRecordPrefix {
    u8 target_bytes_000[0x28]; /* Unknown target bytes +0x0..+0x28. */
    Vec *position_028;
    u8 target_bytes_02C[0x4]; /* Unknown target bytes +0x2C..+0x30. */
    Vec field_030;
} M699CollisionRecordPrefix;

typedef struct M699BoundsRecordPrefix {
    u8 target_bytes_000[0x24]; /* Unknown target bytes +0x0..+0x24. */
    Vec minimum_024;
    Vec maximum_030;
} M699BoundsRecordPrefix;

#endif
