/* Shared state records for the Daft Rafts minigame. */
#ifndef M609_STATE_H
#define M609_STATE_H
#include "REL/minigame_shadow_camera.h"

#include "REL/minigame_light.h"
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "dolphin/gx/GXStruct.h"

/* XZ endpoints used when pushing a player out of a moving segment's bounds. */
typedef struct M609CollisionEdge {
    f32 startX; /* First endpoint X, in world units. */
    f32 endX; /* Second endpoint X, in world units. */
    f32 startZ; /* First endpoint Z, in world units. */
    f32 endZ; /* Second endpoint Z, in world units. */
} M609CollisionEdge;

/* Course segment transform, motion limits, and model links used during setup. */
typedef struct M609Placement {
    s16 status; /* Segment slot; -1 retires the segment. */
    s16 pathIndex; /* Path group used by the player route table. */
    Vec position; /* Initial world position. */
    Vec movementValues; /* Initial per-axis movement values. */
    Vec positionStep; /* X is the turning reference; Y starts the integrated Z displacement. */
    Vec movementSelectors; /* Values selecting lateral and longitudinal movement paths. */
    Vec bounds; /* Extents used for contacts with the course. */
    s16 record; /* Course model array index. */
    s16 linkedRecord; /* Associated collision model index, or -1. */
} M609Placement;

typedef struct M609PadInput {
    s32 unk00; /* The game's code never reads this field. */
    s32 unk04; /* The game's code never reads this field. */
    u16 unk08; /* The game's code never reads this field. */
    s8 unk0A; /* The game's code never reads this field. */
    s8 unk0B; /* The game's code never reads this field. */
    s8 unk0C; /* The game's code never reads this field. */
    s8 unk0D; /* The game's code never reads this field. */
    u8 unk0E; /* The game's code never reads this field. */
    u8 unk0F; /* The game's code never reads this field. */
    u8 unk10; /* The game's code never reads this field. */
    u8 unk11; /* The game's code never reads this field. */
    u8 remaining[2];
} M609PadInput;

typedef struct M609Player {
    M609PadInput padInput;
    s8 playerNo;
    s8 characterNo;
    s8 padNo;
    s8 unk17; /* The game's code never reads this field. */
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
    f32 unk5C; /* No code names this field. */
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
    s16 unkA0; /* No code names this field. */
    s16 timer;
    s16 chaseState;
    u8 unkA6[2]; /* No code names this field. */
    s32 computerButtonsHeld;
    s32 computerButtonsDown;
    s32 computerStickX;
    s32 computerStickY;
    f32 range;
    s16 movementState;
    u8 unkBE[2]; /* No code names this field. */
    Vec routePosition;
    f32 chaseSpeed;
    f32 chaseRadius;
    s16 routeStep;
    s16 routePhase;
    s16 unkD8; /* The game's code never reads this field. */
    u8 unkDA[2]; /* No code names this field. */
    Vec chasePosition;
    f32 routeSpeed;
    f32 routeRadius;
    s16 segmentModel;
    s16 candidateSegments[4];
    u8 unkFA[2]; /* No code names this field. */
    Vec origin;
    Vec target;
    s16 targetSegment;
    s16 movementSegment;
    f32 unk118; /* The game's code never reads this field. */
    s16 sample;
    s16 sampleGroup;
    s16 rank;
    u8 unk122[2]; /* No code names this field. */
    s16 wait;
    s16 waitTimer;
    s16 increment;
    s16 period;
    s16 probability;
    u8 unk12E[2]; /* No code names this field. */
} M609Player;

typedef struct M609Motion {
    s16 motionIds[11];
    s16 mode;
    s16 previousMode;
    u8 unk1A[2]; /* No code names this field. */
    f32 speed;
    f32 previousSpeed;
    f32 time;
    f32 previousTime;
    f32 unk2C; /* The game's code never reads this field. */
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
    f32 unk64; /* The game's code never reads this field. */
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
    u8 unk00[8]; /* No code names this field. */
    s16 step;
    u8 unk0A[2]; /* No code names this field. */
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
    u8 unk6F[1]; /* No code names this field. */
    Vec winnerPositions[4][4];
    s16 slotOccupied[8];
    f32 levels[8];
    s16 night;
    u8 unk162[2]; /* No code names this field. */
    Vec raftOffset; /* Setup initializes all axes; auxiliary placement and route checks use Y. */
    f32 scroll;
    f32 pushSpeed;
    f32 unk178; /* The game's code never reads this field. */
    f32 playerRadius;
    f32 hazardRadius;
    f32 obstacleRadius;
    f32 startZ;
    s8 update;
    s8 landed;
    s8 resultFlag;
    u8 unk18F[1]; /* No code names this field. */
    s16 skippedSegmentModel;
    s16 models[4][4]; /* Contact-effect records indexed by redirected route variant and player. */
    s8 sequenceState;
    u8 unk1B3[5]; /* No code names this field. */
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

typedef struct M609ParticleState {
    s16 modelIds[1];
    s16 unk02[1]; /* The game's code never reads this field. */
    void *allocation[1];
} M609ParticleState;

typedef struct M609CameraRecord {
    s32 unk00; /* The game's code never reads this field. */
    u8 unk04[12]; /* No code names this field. */
    Vec rotation;
    Vec target;
    u8 unk28[12]; /* No code names this field. */
    /* Indices 0/1: orbit distance/aspect; 2 through 5: viewport x/y/width/height. */
    f32 projectionAndViewport[6];
    f32 fieldOfView;
} M609CameraRecord;

typedef struct M609CameraBank {
    M609CameraRecord cameras[4];
    u8 unk140[8]; /* No code names this field. */
} M609CameraBank;

extern s16 lbl_1_bss_0[2];
extern M609PlayerActors lbl_1_bss_4;
extern M609Control lbl_1_bss_30;
extern M609Player lbl_1_bss_24C[4];
extern M609Motion lbl_1_bss_70C[4];
extern M609Model lbl_1_bss_8BC[154];
extern M609ParticleState lbl_1_bss_6B64;
extern MinigameShadowCamera lbl_1_bss_6B6C;
extern LightInitState lbl_1_bss_6BF8;
extern u8 lbl_1_bss_6BBC[60];
extern M609CameraBank lbl_1_bss_6C34;
extern struct Process_s *lbl_1_bss_6D7C;
extern u32 lbl_1_bss_6D88[40];
extern Vec *lbl_1_bss_6E28[10];
extern s16 lbl_1_bss_6E50[12];

void fn_1_670(void);
void fn_1_2780(void);
void fn_1_2DF4(void);
s32 fn_1_30D8(f32 *value, f32 step, f32 start, f32 end);
void fn_1_45EC(s16 streamSlot, s16 fadeFrames);
void fn_1_4F98(void);
void fn_1_52B8(void);
void fn_1_5618(s16 soundSlot, s16 fadeSpeed);
void fn_1_6974(void);
void fn_1_8EE4(void);
void fn_1_11B80(void);
s32 fn_1_118BC(void);
void fn_1_4C68(s16 soundSlot, s16 volume);
s32 fn_1_4EB0(void);
s32 fn_1_1115C(void);
s32 fn_1_111A8(void);
s32 fn_1_11658(void);
s32 fn_1_1183C(void);
int fn_1_1188C(void);
s32 fn_1_D1CC(void);
s32 fn_1_E424(void);
s32 fn_1_E474(void);
void fn_1_11E2C(void);
void fn_1_125C4(void);
void fn_1_3C0(void);
void fn_1_10FC(void);
f32 fn_1_31F8(f32 modulusOrResult);
void fn_1_3FF0(M609Player *player, s16 panMode);
void fn_1_1661C(void);

void fn_1_46F8(s16 voiceSlot, s16 mode, Vec *position);

#endif
