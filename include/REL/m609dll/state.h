/* Shared state records for the Daft Rafts minigame. */
#ifndef M609_STATE_H
#define M609_STATE_H

#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "dolphin/gx/GXStruct.h"

typedef struct M609PadInput {
    s32 buttons;
    s32 buttonsDown;
    u16 buttonsRepeat;
    s8 stickX;
    s8 stickY;
    s8 substickX;
    s8 substickY;
    u8 triggerL;
    u8 triggerR;
    u8 directions;
    u8 directionsRepeat;
    u8 remaining[2];
} M609PadInput;

typedef struct M609Player {
    M609PadInput padInput;
    s8 playerNo;
    s8 characterNo;
    s8 padNo;
    s8 groupNo;
    s8 participantType;
    s8 difficulty;
    s8 disableInput;
    s8 mode;
    s16 state;
    s16 fallState;
    s16 boundaryMode;
    s16 state22;
    Vec position;
    Vec previousPosition;
    Vec rotation;
    Vec velocity;
    f32 spinSpeed;
    f32 turnYaw; /* Opening turn increment or lineup starting yaw. */
    f32 value5C;
    Vec savedPosition;
    s8 direction;
    s8 phase;
    s8 motionFlag;
    s8 movementFlag;
    s16 scaledDown;
    s8 collisionMesh;
    s8 previousCollisionMesh;
    s8 linkedPlayer;
    s8 linkState;
    s16 linkCount;
    s16 slot;
    s16 travelState;
    s16 sequencePhase;
    s16 phaseStep;
    s16 secondLegRequired;
    s16 phaseCounter;
    Vec phasePosition;
    f32 arrivalOffsetX;
    f32 routeTargetY;
    f32 arrivalOffsetZ;
    s16 routeState;
    s16 actionState;
    s16 stateA0;
    s16 timer;
    s16 chaseState;
    u8 stateA6[2];
    s32 computerButtonsHeld;
    s32 computerButtonsDown;
    s32 computerStickX;
    s32 computerStickY;
    f32 range;
    s16 movementState;
    u8 stateBE[2];
    Vec routePosition;
    f32 chaseSpeed;
    f32 chaseRadius;
    s16 routeStep;
    s16 routePhase;
    s16 chaseUpdateCount;
    u8 stateDA[2];
    Vec chasePosition;
    f32 routeSpeed;
    f32 routeRadius;
    s16 segmentModel;
    s16 candidateSegments[4];
    u8 stateFA[2];
    Vec origin;
    Vec target;
    s16 targetSegment;
    s16 movementSegment;
    f32 distance118;
    s16 sample;
    s16 sampleGroup;
    s16 rank;
    u8 state122[2];
    s16 wait;
    s16 waitTimer;
    s16 increment;
    s16 period;
    s16 probability;
    u8 state12E[2];
} M609Player;

typedef struct M609Motion {
    s16 motionIds[11];
    s16 mode;
    s16 previousMode;
    u8 state1A[2];
    f32 speed;
    f32 previousSpeed;
    f32 time;
    f32 previousTime;
    f32 maximumTime;
    u32 attributes;
    u32 previousAttributes;
    u32 attributesToReset;
    s32 motionEnded;
    s16 resetRequested;
    s16 resetMode;
    f32 transition;
    u32 resetAttributes;
    u32 resetAttributesToClear;
    s16 shiftState;
    s16 shiftMotion;
    u32 shiftFlags;
    f32 shiftStart;
    f32 blendDuration;
    f32 shiftTime;
    f32 shiftMaximumTime;
    u8 state68[4];
} M609Motion;

typedef struct M609Model {
    s16 model;
    s16 stateOrTimer; /* Model phase, or reveal countdown for models 11 through 16. */
    /* Segment oscillation phase; auxiliary recoil phase (0 idle, 1 start, 2 update). */
    s16 movementPhase;
    s16 spawnDecisionMade;
    /* Segment slot (-1 retired), obstacle active flag, or auxiliary recoil update counter. */
    s16 status;
    s16 recordIndex;
    s16 pathIndex;
    s16 linkedRecord;
    s16 playerModels[4]; /* Contact-effect record indices, indexed by player number. */
    s16 linkedRecords[6];
    Vec position;
    Vec previousPosition;
    Vec rotation;
    /* Segment X/Z velocity and Y acceleration; auxiliary local offsets;
     * obstacle Y spin; result platform Z velocity. */
    Vec movementValues;
    Vec movementSelectors; /* Nonzero X/Z selects lateral/longitudinal motion. */
    /* Segments use X as a turning reference and Y as integrated Z displacement;
     * auxiliary models use X as their recoil step. */
    Vec positionStep;
    /* Segment joint/contact position; model 67 combines model 75's joint Y/Z with its own X.
     * Auxiliary models use X as their forward drift offset. */
    Vec jointPositionOrDriftOffset;
    Vec collisionRotation;
    Vec bounds;
    s8 resourceIndex;
    u8 state91[3];
    f32 bobOffset;
    f32 bobSpeed;
    f32 motionVerticalPosition;
    f32 motionVerticalSpeed;
} M609Model;

typedef struct M609Control {
    u8 state00[8];
    s16 step;
    u8 state0A[2];
    s16 modeStep;
    s16 sequenceStep;
    s16 waitTimer;
    s16 index;
    M609PadInput padInputs[4];
    u8 state64[4];
    s8 resultsReady;
    s8 setupFlag;
    s8 participantCount;
    s8 winners[4];
    u8 state6F[1];
    Vec winnerPositions[4][4];
    s16 slotOccupied[8];
    f32 levels[8];
    s16 night;
    u8 state162[2];
    Vec raftOffset; /* Setup initializes all axes; auxiliary placement and route checks use Y. */
    f32 scroll;
    f32 pushSpeed;
    f32 rate;
    f32 playerRadius;
    f32 hazardRadius;
    f32 obstacleRadius;
    f32 startZ;
    s8 update;
    s8 landed;
    s8 resultFlag;
    u8 state18F[1];
    s16 skippedSegmentModel;
    s16 models[4][4]; /* Contact-effect records indexed by redirected route variant and player. */
    s8 sequenceState;
    u8 state1B3[5];
    s32 musicHandle;
    s32 musicId;
    s32 jingleHandle;
    s32 jingleId;
    s32 soundState;
    s32 soundHandles[9];
    s16 soundSlotStates[9];
    s16 panXLow;
    s16 panXHigh;
    f32 positionXLow;
    f32 positionXHigh;
    s16 volumeZLow;
    s16 volumeZHigh;
    f32 positionZLow;
    f32 positionZHigh;
} M609Control;

typedef struct M609PlayerActors {
    s16 models[14];
    struct MgPlayer_s *players[4];
} M609PlayerActors;

typedef struct M609LightState {
    s16 id;
    Vec position;
    Vec orientation;
    Vec aim;
    Vec direction;
    f32 range;
    GXColor color;
} M609LightState;

typedef struct M609ShadowState {
    Vec cameraPosition;
    f32 verticalAngle;
    f32 horizontalAngle;
    f32 state14;
    Vec target;
    f32 state24[3];
    Vec cameraUp;
    f32 state3C[3];
    f32 orbitDistance;
    f32 shadowTransparency;
} M609ShadowState;

typedef struct M609ParticleState {
    s16 modelIds[1];
    s16 particleCounts[1];
    void *allocation[1];
} M609ParticleState;

typedef struct M609CameraRecord {
    s32 cameraIndex;
    u8 state04[12];
    Vec rotation;
    Vec target;
    u8 state28[12];
    /* Indices 0/1: orbit distance/aspect; 2 through 5: viewport x/y/width/height. */
    f32 projectionAndViewport[6];
    f32 fieldOfView;
} M609CameraRecord;

typedef struct M609CameraBank {
    M609CameraRecord cameras[4];
    u8 state140[8];
} M609CameraBank;

extern s16 lbl_1_bss_0[2];
extern M609PlayerActors lbl_1_bss_4;
extern M609Control lbl_1_bss_30;
extern M609Player lbl_1_bss_24C[4];
extern M609Motion lbl_1_bss_70C[4];
extern M609Model lbl_1_bss_8BC[154];
extern M609ParticleState lbl_1_bss_6B64;
extern M609ShadowState lbl_1_bss_6B6C;
extern M609LightState lbl_1_bss_6BF8;
extern u8 lbl_1_bss_6BBC[60];
extern M609CameraBank lbl_1_bss_6C34;
extern struct Process_s *lbl_1_bss_6D7C;
extern u32 lbl_1_bss_6D88[40];
extern Vec *lbl_1_bss_6E28[10];
extern s16 lbl_1_bss_6E50[12];

#endif
