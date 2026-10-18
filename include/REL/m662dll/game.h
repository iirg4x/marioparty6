/* Trick or Tree player, scene, motion, puzzle and particle state shared by its updates. */
#ifndef REL_M662DLL_GAME_H
#define REL_M662DLL_GAME_H

#include "dolphin/types.h"
#include "game/hu3d.h"
#include "game/mg/seqman.h"
#include "game/process.h"

/* A frame of button, stick, trigger and directional-repeat input from one pad. */
typedef struct M662PadInput {
    s32 buttons;
    s32 buttonsDown;
    u16 buttonsRepeat;
    s8 stickX, stickY, subStickX, subStickY;
    u8 triggerL, triggerR, dpad, dpadRepeat;
} M662PadInput;

/* One group representative, with cached input and the state used by the round updater. */
typedef struct M662Player {
    M662PadInput input;
    /* Configured player, local character slot, character kind and controller port. */
    s8 playerNo, characterIndex, characterNo, padNo;
    /* Group and computer settings; a nonzero controlState clears cached player input. */
    s8 groupNo, playerType, comDifficulty, controlState;
    /* Result initialized to -1 before play. */
    s8 result;
    /* Player update state, with counters measured in update frames. */
    s32 state;
    /* Unknown state bytes, cleared during scene setup. */
    u8 unknownAfterState[4];
    s32 counter, delay;
    /* World position. */
    HuVecF position;
    /* Unknown state bytes, cleared during scene setup. */
    u8 unknownAfterPosition[12];
    /* Model rotation in degrees. */
    HuVecF rotation;
    /* Selected route slot and the current and previous movement steps. */
    s16 slot, stepSize, lastStep;
    /* Target facing in degrees and the starting position of the current movement. */
    f32 targetAngle, startX, startY, startZ;
    /* Movement flag and accumulated score. */
    s16 sideways, score;
    /* Vertical movement per frame and its per-frame change. */
    f32 velocityY, gravity;
    /* Computer decision state, waiting frames and lateral choices still to attempt. */
    s16 decisionState, cooldown, remainingMoves;
} M662Player;

/* Model handle, motion counters and world transform for a scene object. */
typedef struct M662Model {
    s16 modelId, motionState, motionFrame;
    HuVecF position, rotation;
} M662Model;

/* Pending and active character animation settings, including the blend to the next motion. */
typedef struct M662Motion {
    /* Engine motion handles; requestedMotion and currentMotion index this array. */
    s16 motionIds[6];
    s16 requestedMotion, currentMotion;
    /* Playback speed and animation times in frames. */
    f32 requestedSpeed, currentSpeed, requestedTime, previousTime, maximumTime;
    /* Motion flags to apply, active flags and flags to clear. */
    u32 requestedAttributes, currentAttributes, resetAttributes;
    /* Cached end check and an automatic next-motion request. */
    s32 motionEnded;
    s16 completionEvent, nextMotion;
    f32 nextTime;
    u32 nextAttributes, nextResetAttributes;
    /* Blend state and incoming motion index. */
    s16 shiftState, shiftMotion;
    u32 shiftAttributes;
    /* Incoming start time, blend duration, playback time and maximum playback time, in frames. */
    f32 shiftStartFrame, blendDuration, shiftTime, shiftMaximumTime;
    /* Unknown state bytes, cleared during scene setup. */
    u8 unknownTail[4];
} M662Motion;

/* Minigame phase state, puzzle selection, result tables and shared audio settings. */
typedef struct M662Sequence {
    /* Unknown state bytes, cleared during scene setup. */
    u8 unknownPrefix[8];
    /* State-machine steps, cue selection, update-frame timers and completion flag. */
    s16 sequenceState, stage, effectTimer, sceneState, endingState, cueIndex, timer, complete;
    /* Raw input snapshots for all four controller ports. */
    M662PadInput controllers[4];
    /* Round timer object managed by the minigame timer routines. */
    void *timerObject;
    /* Unknown state bytes, cleared during scene setup. */
    u8 unknownAfterTimer[4];
    s16 nightMode, selectionValue, winnerIndex, mode;
    /* Selected puzzle patterns and the current path step. */
    s16 patterns[3], pathStep;
    /* Puzzle ordering and the scores assigned to those positions. */
    s16 ranks[4], scores[4];
    /* Cached camera target height used by climb and ending transitions. */
    f32 centerY;
    /* Unknown state bytes, cleared during scene setup. */
    u8 unknownAfterCenter[4];
    /* Active music handles and the resource IDs last requested. */
    s32 bgmHandle, bgmId, jingleHandle, jingleId;
    /* Unknown state bytes, cleared during scene setup. */
    u8 unknownAfterMusic[4];
    /* Effect handles and activity flags for four reusable sound slots. */
    s32 soundHandle[4];
    s16 soundState[4];
    /* Stereo pan range and the fixed pan used by ending sound cues. */
    s16 panMinimum, panMaximum, panCenter;
    /* World X range mapped into the pan range. */
    f32 xMinimum, xMaximum;
    /* Effect volume range mapped from the world Z range below. */
    s16 volumeMinimum, volumeMaximum;
    /* Unknown state bytes, cleared during scene setup. */
    u8 unknownAfterVolume[4];
    f32 zMinimum, zMaximum;
} M662Sequence;

/* Group representatives, scene objects, character motions and the shared phase state. */
extern M662Player lbl_1_bss_2F4[2];
extern M662Model lbl_1_bss_4B4[46];
extern M662Motion lbl_1_bss_3F4[2];
extern M662Sequence lbl_1_bss_210;

/* Scenery activation centers; the updater tests centerX plus or minus radius. */
typedef struct M662Bounds { f32 centerX, centerY, centerZ, radius; } M662Bounds;
/* Path table index (-1 means inactive) and interpolation mode: 0 sine, 1 linear. */
typedef struct M662PuzzlePath { s16 index, mode; } M662PuzzlePath;
/* One moving puzzle pair, its object attachment and up to twelve path segments. */
typedef struct M662Puzzle {
    s16 status;
    /* Unknown state bytes, cleared during scene setup. */
    u8 unknownAfterStatus[2];
    s16 firstModel, secondModel, countdown;
    /* Attachment object name queried on the carrier model. */
    char *objName;
    s16 pathMode, pathIndex;
    /* Unknown state bytes, cleared during scene setup. */
    u8 unknownBeforeProgress[4];
    /* Curved-path angle in degrees, advanced once per update. */
    f32 progress;
    M662PuzzlePath paths[12];
    /* Small Z adjustment that separates overlapping trees in depth order. */
    f32 depthOffset;
} M662Puzzle;
/* Draw model, capacity and per-entry parameter allocation for the particle effect. */
typedef struct M662ParticleModule {
    s16 modelId[1], maximumCount[1];
    void *allocation[1];
} M662ParticleModule;
/* Shadow camera position, orbit angles, target and opacity. */
typedef struct M662Shadow {
    /* World position. */
    HuVecF position;
    f32 angle[3];
    HuVecF target;
    /* Unknown state bytes, cleared during scene setup. */
    u8 unknownAfterTarget[12];
    HuVecF up;
    /* Unknown state bytes, cleared during scene setup. */
    u8 unknownAfterUp[12];
    /* Orbit radius in world units and opacity multiplier supplied to the shadow renderer. */
    f32 radius, textureLevel;
} M662Shadow;
/* Global light handle, world position and direction, aim controls and RGBA color. */
typedef struct M662Light {
    HU3D_LIGHTID id;
    HuVecF position, orientation, aim, direction;
    f32 range;
    GXColor color;
} M662Light;
/* One camera preset with rotation in degrees and position in world units. */
typedef struct M662CameraRecord {
    s32 cameraIndex;
    /* Unknown state bytes, cleared during scene setup. */
    u8 unknownAfterIndex[12];
    HuVecF rotation, position;
    /* Unknown state bytes, cleared during scene setup. */
    u8 unknownAfterPosition[12];
    /* Zoom, aspect ratio, viewport X/Y and width/height; field of view is in degrees. */
    f32 viewport[6], fieldOfView;
} M662CameraRecord;
/* Storage for four camera presets; setup loads a selected record into the shared camera
 * controls. */
typedef struct M662CameraState {
    M662CameraRecord cameras[4];
    /* Unknown state bytes, cleared during scene setup. */
    u8 unknownTail[8];
} M662CameraState;
extern M662Bounds lbl_1_bss_0[13];
extern M662Puzzle lbl_1_bss_D0[4];
extern M662ParticleModule lbl_1_bss_A74;
extern M662Shadow lbl_1_bss_A7C;
extern M662Light lbl_1_bss_ACC;
extern M662CameraState lbl_1_bss_B08;

/* Per-particle emission settings copied into the effect allocation at spawn time. */
typedef struct M662ParticleParameters {
    /* Copied emission setting, currently initialized to zero. */
    u32 control;
    /* Auxiliary values reset to zero when the entry is emitted. */
    f32 resetValues[3];
    /* Starting RGBA color; finalColor supplies the RGB destination for each step. */
    GXColor initialColor;
    u8 finalColor[4];
    /* Per-frame X/Y/Z velocity multipliers. */
    f32 velocityScale[3];
    /* Per-frame vertical acceleration, rotation in degrees and size-change controls. */
    f32 gravity, rotationStep, scaleStep, scaleStepMultiplier;
    /* Alpha change in byte units and fraction of the initial-to-final RGB difference per frame. */
    f32 alphaStep, colorBlendStep;
} M662ParticleParameters;

/* Curved X/Z path: duration in frames, center, angle endpoints and X amplitude. */
typedef struct M662SinePath {
    f32 duration, centerX;
    /* Unknown initialized bytes between the X and Z path components. */
    u8 unknownBetweenComponents[4];
    f32 centerZ, start, end, amplitude;
} M662SinePath;
/* Straight X/Z path: duration in frames and start/end coordinates. */
typedef struct M662LinearPath {
    f32 duration, startX;
    /* Unknown initialized bytes between the X and Z start components. */
    u8 unknownBetweenStartComponents[4];
    f32 startZ, endX;
    /* Unknown initialized bytes between the X and Z endpoint components. */
    u8 unknownBetweenEndpoints[4];
    f32 endZ;
} M662LinearPath;

/* Phase callbacks, effect IDs, path tables and resources shared by the scene files. */
extern HUPROCESS *lbl_1_bss_C50[2];
extern MGSEQ_PARAM lbl_1_data_0;
extern s16 lbl_1_data_F6[5];
extern M662SinePath lbl_1_data_150[17];
extern M662LinearPath lbl_1_data_32C[10];
extern s32 lbl_1_data_448[40];
extern char lbl_1_data_4E8[4][32];

#endif
