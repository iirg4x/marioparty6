/* Shared state and records used by Light Up My Night module files. */
#ifndef M617DLL_SHARED_H
#define M617DLL_SHARED_H

#include "dolphin/types.h"
#include "game/hu3d.h"

typedef struct M617Vector2 {
    f32 x;
    f32 y;
} M617Vector2;

typedef struct M617ScoreBoxEntry {
    s16 spriteId; /* Sprite ID, or score-box ID for the two character panels. */
    s16 groupId; /* Sprite group containing this marker. */
    s16 memberIndex; /* Member slot in that group. */
    u8 unreadBeforePosition[6]; /* Not accessed by these display helpers. */
    M617Vector2 position; /* Screen position for a panel, or group-local marker position. */
    s16 width; /* Score-box width supplied at creation. */
    s16 height; /* Score-box height supplied at creation. */
    f32 opacity; /* Stored opacity, initialized to zero here. */
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
    f32 tpLevel; /* Initial opacity sent to the score-box controls. */
    s16 drawPriority; /* Sprite draw priority passed at creation. */
    u8 unreadTail[2]; /* Not accessed by these display helpers. */
} M617ScoreBoxEntry;

typedef struct M617SpriteGroup {
    s16 groupId; /* Group ID for one player's ten progress-marker sprites. */
    u8 unreadBeforePosition[2]; /* Not accessed by these display helpers. */
    M617Vector2 position; /* Position of the group on screen. */
    f32 scaleX; /* Horizontal scale multiplier. */
    f32 scaleY; /* Vertical scale multiplier. */
} M617SpriteGroup;

typedef struct M617PadSnapshot {
    s32 buttons;
    s32 buttonsDown;
    u16 buttonsRepeat;
    s8 stickX;
    s8 stickY;
    s8 subStickX;
    s8 subStickY;
    u8 triggerL;
    u8 triggerR;
    u8 dpad;
    u8 dpadRepeat;
} M617PadSnapshot;

typedef struct M617BoardSelection {
    s16 nodeIndex;
    s16 modelIndex;
} M617BoardSelection;

typedef struct M617BoxEdgeLinks {
    s8 links[2][4];
} M617BoxEdgeLinks;

/* State shared by the sequence, controller snapshots, board setup and audio. */
typedef struct M617State {
    u8 unknown00[8];
    s16 transitionPhase;
    s16 transitionFrame;
    s16 sequencePhase;
    s16 sequenceSelector;
    s16 sequenceFrame;
    M617PadSnapshot padSnapshots[4];
    struct MgTimer_s *timer;
    s32 timerFrames;
    u8 unknown6C[2];
    s16 nightFlag;
    s16 selection;
    f32 limit;
    s16 layout;
    f32 rotation;
    M617BoardSelection selected[9];
    u8 unknownA4[4];
    s32 firstStream;
    s32 bgmId;
    s32 secondStream;
    s32 jingleId;
    u8 unknownB8[4];
    s32 soundId[3];
    s16 soundActive[3];
    s16 panMinimum;
    s16 panMaximum;
    f32 panPositionMinimum;
    f32 panPositionMaximum;
    s16 volumeMinimum;
    s16 volumeMaximum;
    f32 volumePositionMinimum;
    f32 volumePositionMaximum;
} M617State;

extern M617State lbl_1_bss_9C8;

typedef struct M617RouteEntry {
    s16 state;
    s16 pointCount;
    Vec directions[2];
    Vec positions[2];
} M617RouteEntry;

typedef struct M617RouteGroup {
    f32 value;
    M617RouteEntry entries[4];
} M617RouteGroup;

extern M617RouteGroup lbl_1_bss_38F0[24];

/* Per-player input, motion, route selection and computer-player state. */
typedef struct M617PlayerStateData {
    M617PadSnapshot input;
    s8 playerIndex;
    s8 participantIndex;
    s8 character;
    s8 controllerPort;
    s8 group;
    s8 computerControlled;
    s8 difficulty;
    s8 motionState;
    s8 resultState;
    s32 completionState;
    u8 unknown24[4];
    s32 actionFrame;
    u8 unknown2C[4];
    s8 characterMode;
    Vec position;
    Vec target;
    Vec rotation;
    s8 transformState;
    u8 unknown59;
    s16 collisionMeshIndex;
    s16 previousCollisionMeshIndex;
    s16 linkedPlayer;
    s16 previousLinkedPlayer;
    s16 orbitCounter;
    u8 unknown64[4];
    char *itemHook;
    Vec previousPosition;
    Vec snapshotPosition;
    Vec movement;
    s16 rangeIndex;
    f32 upperIntensityLimit[3];
    f32 lowerIntensityLimit[3];
    f32 upperScaleLimit[3];
    f32 lowerScaleLimit[3];
    f32 ambient;
    s16 progress;
    f32 lightRadius;
    f32 lightAngle;
    f32 lightHeight;
    s32 computerButtonsHeld;
    s32 computerButtonsDown;
    s32 computerStickX;
    s32 computerStickY;
    s16 controlPhase;
    s16 selectionPhase;
    s16 actionPhase;
    s16 routeMode;
    s16 orbitActive;
    s16 routeStateFrame;
    s16 orbitDelay;
    s16 selectionTimer;
    s16 orbitFrame;
    s16 chasePhase;
    Vec targetPosition;
    f32 targetStepX;
    u8 unknown10C[4];
    f32 targetStepZ;
    f32 chaseSpeed;
    f32 chaseRadius;
    s16 targetObstructed;
    s16 selectedModel;
    s16 collisionModels[2];
    s16 previousCollisionModels[2];
    s16 collisionEdges[2];
    s16 previousCollisionEdges[2];
    Vec collisionDirections[2];
    s16 routeIndex;
    s16 routePoint;
    s16 routeElement;
    f32 orbitAngleStep;
    Vec orbitOrigin;
    s16 stageFrame;
    s16 routeFrame;
    s16 obstacle;
    s16 previousObstacle;
    s16 selectionFrames;
    s16 selectionThreshold;
    s16 selectionDuration;
    s16 retryLimit;
    s16 selectionChanged;
    s16 targetSelectionChance;
    s16 chaseSpeedChance;
    f32 rangeLimits[3];
    s16 selectionDelayFrames;
    s16 elapsedFrames;
} M617PlayerStateData;

extern M617PlayerStateData lbl_1_bss_AB0[2];

typedef struct M617PlayerBindings {
    s16 collisionModel;
    struct MgPlayer_s *players[2];
} M617PlayerBindings;

extern M617PlayerBindings lbl_1_bss_9BC;

typedef struct M617ModelSetupRecord {
    s16 modelId;
    s16 motionState;
    u8 unknown04[4];
    Vec position;
    Vec secondaryPosition;
    Vec rotation;
    Vec dimensions;
    Vec edgeDirections[4];
    f32 scaleStep;
} M617ModelSetupRecord;

typedef struct M617ParticipantMotionState {
    s16 motionIds[7];
    s16 mode;
    s16 previousMode;
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
    f32 shiftEnd;
    f32 shiftTime;
    f32 shiftMaximumTime;
    u8 unknown60[4];
} M617ParticipantMotionState;

typedef struct ParticleModuleData {
    s16 modelIds[1];
    s16 maximumCounts[1];
    void *allocation[1];
} ParticleModuleData;

typedef struct M617LightInitState {
    HU3D_LIGHTID id;
    HuVecF position;
    HuVecF orientation;
    HuVecF aim;
    HuVecF direction;
    f32 range;
    GXColor color;
} M617LightInitState;

typedef struct M617ShadowWork {
    f32 positionX;
    f32 positionY;
    f32 positionZ;
    f32 angleY;
    f32 angleX;
    f32 angleZ;
    f32 originX;
    f32 originY;
    f32 originZ;
    u8 unknown24[12];
    f32 directionX;
    f32 directionY;
    f32 directionZ;
    u8 unknown3C[12];
    f32 radius;
    f32 shadowOpacity;
} M617ShadowWork;

typedef struct M617LightSource {
    u8 unknown00[4];
    s16 lightId;
    s16 modelId;
    s16 modelIndex;
    s16 localLightPoolId;
    Vec position;
    Vec referencePosition;
    Vec rotation;
    Vec direction;
    GXColor color;
    f32 distance;
    f32 intensity;
    f32 intensityStep;
    s16 enabled;
    s16 frame;
    Vec objectPosition;
} M617LightSource;

extern M617LightSource lbl_1_bss_458[15];
extern M617ScoreBoxEntry lbl_1_bss_0[22];
extern M617SpriteGroup lbl_1_bss_370[2];
extern M617ModelSetupRecord lbl_1_bss_E88[96];
extern M617ParticipantMotionState lbl_1_bss_DC0[2];
extern M617BoxEdgeLinks lbl_1_bss_398[24];
extern struct Process_s *lbl_1_bss_38E4;
extern struct MgSeqParam_s lbl_1_data_0;
extern u8 lbl_1_bss_379C[328];
extern M617LightInitState lbl_1_bss_3760;
extern M617ShadowWork lbl_1_bss_3710;
extern ParticleModuleData lbl_1_bss_3708;
extern s16 lbl_1_data_F6[4];
extern u32 lbl_1_data_118[51];

#endif
