/* Trick or Tree player, scene, motion, puzzle and particle state shared by its updates. */
#ifndef REL_M662DLL_GAME_H
#define REL_M662DLL_GAME_H
#include "REL/minigame_camera_preset.h"
#include "REL/minigame_shadow_camera.h"

#include "REL/minigame_light.h"
#include "REL/minigame_pad.h"
#include "dolphin/types.h"

extern s16 lbl_1_bss_C60;
#include "game/hu3d.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/charman.h"
#include "game/process.h"
#include "REL/particle_parameters.h"

/* One group representative, with cached input and the state used by the round updater. */
typedef struct M662Player {
    SystemPadInput input;
    /* Configured player, local character slot, character kind and controller port. */
    s8 playerNo, characterIndex, characterNo, padNo;
    s8 unk18; /* The game's code never reads this field. */
    s8 playerType;
    s8 unk1A; /* The game's code never reads this field. */
    /* A nonzero controlState clears cached player input. */
    s8 controlState;
    /* Result initialized to -1 before play. */
    s8 result;
    /* Player update state, with counters measured in update frames. */
    s32 state;
    u8 unk24[4]; /* No code names this field. */
    s32 counter, delay;
    /* World position. */
    HuVecF position;
    u8 unk3C[12]; /* No code names this field. */
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
    f32 requestedSpeed, currentSpeed, requestedTime, previousTime;
    f32 unk20; /* The game's code never reads this field. */
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
    /* Incoming start time, blend duration and playback time, in frames. */
    f32 shiftStartFrame, blendDuration, shiftTime;
    f32 unk58; /* The game's code never reads this field. */
    u8 unk5C[4]; /* No code names this field. */
} M662Motion;

/* Minigame phase state, puzzle selection, result tables and shared audio settings. */
typedef struct M662Sequence {
    u8 unk00[8]; /* No code names this field. */
    /* State-machine steps, cue selection, update-frame timers and completion flag. */
    s16 sequenceState, stage, effectTimer, sceneState, endingState, cueIndex, timer, complete;
    /* Raw input snapshots for all four controller ports. */
    SystemPadInput controllers[4];
    /* Round timer object managed by the minigame timer routines. */
    MGTIMER *timerObject;
    u8 unk6C[4]; /* No code names this field. */
    s16 unk70; /* The game's code never reads this field. */
    s16 selectionValue, winnerIndex, mode;
    /* Selected puzzle patterns and the current path step. */
    s16 patterns[3], pathStep;
    /* Puzzle ordering and the scores assigned to those positions. */
    s16 ranks[4], scores[4];
    /* Cached camera target height used by climb and ending transitions. */
    f32 centerY;
    u8 unk94[4]; /* No code names this field. */
    /* Active music handles and the resource IDs last requested. */
    s32 bgmHandle, bgmId, jingleHandle, jingleId;
    u8 unkA8[4]; /* No code names this field. */
    /* Effect handles and activity flags for four reusable sound slots. */
    s32 soundHandle[4];
    s16 soundState[4];
    /* Stereo pan range and the fixed pan used by ending sound cues. */
    s16 panMinimum, panMaximum, panCenter;
    /* World X range mapped into the pan range. */
    f32 xMinimum, xMaximum;
    /* Effect volume range mapped from the world Z range below. */
    s16 volumeMinimum, volumeMaximum;
    u8 unkD8[4]; /* No code names this field. */
    f32 zMinimum, zMaximum;
} M662Sequence;

/* Group representatives, scene objects, character motions and the shared phase state. */
extern M662Player lbl_1_bss_2F4[2];
extern M662Model lbl_1_bss_4B4[46];
extern M662Motion lbl_1_bss_3F4[2];
extern M662Sequence lbl_1_bss_210;

/* Scenery activation centers; the updater tests centerX plus or minus radius. */
typedef struct M662Bounds {
    f32 centerX;
    f32 unk04; /* No code names this field. */
    f32 unk08; /* No code names this field. */
    f32 radius;
} M662Bounds;
/* Path table index (-1 means inactive) and interpolation mode: 0 sine, 1 linear. */
typedef struct M662PuzzlePath { s16 index, mode; } M662PuzzlePath;
/* One moving puzzle pair, its object attachment and up to twelve path segments. */
typedef struct M662Puzzle {
    s16 status;
    u8 unk02[2]; /* No code names this field. */
    s16 firstModel, secondModel, countdown;
    /* Attachment object name queried on the carrier model. */
    char *objName;
    s16 pathMode, pathIndex;
    u8 unk14[4]; /* No code names this field. */
    /* Curved-path angle in degrees, advanced once per update. */
    f32 progress;
    M662PuzzlePath paths[12];
    /* Small Z adjustment that separates overlapping trees in depth order. */
    f32 depthOffset;
} M662Puzzle;
/* Draw model and per-entry parameter allocation for the particle effect. */
typedef struct M662ParticleModule {
    s16 modelId[1];
    s16 unk02[1]; /* The game's code never reads this field. */
    ParticleParameters *allocation[1];
} M662ParticleModule;
extern M662Bounds lbl_1_bss_0[13];
extern M662Puzzle lbl_1_bss_D0[4];
extern M662ParticleModule lbl_1_bss_A74;
extern MinigameShadowCamera lbl_1_bss_A7C;
extern LightInitState lbl_1_bss_ACC;
typedef struct M662CameraState {
    MinigameCameraPreset cameras[4];
    u8 unk140[8]; /* No code names this field. */
} M662CameraState;

extern M662CameraState lbl_1_bss_B08;

/* Curved X/Z path: duration in frames, center, angle endpoints and X amplitude. */
typedef struct M662SinePath {
    f32 duration, centerX;
    u8 unk08[4]; /* No code names this field. */
    f32 centerZ, start, end, amplitude;
} M662SinePath;
/* Straight X/Z path: duration in frames and start/end coordinates. */
typedef struct M662LinearPath {
    f32 duration, startX;
    u8 unk08[4]; /* No code names this field. */
    f32 startZ, endX;
    u8 unk14[4]; /* No code names this field. */
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
void fn_1_57A8(void);
int fn_1_8B34(void);
s32 fn_1_8B8C(void);
s32 fn_1_8BCC(void);
s32 fn_1_57C8(void);
s32 fn_1_A474(void);
void fn_1_984(void);
void fn_1_2670(void);
s32 fn_1_2DA0(f32 *currentValue, f32 durationFrames, f32 start, f32 end);
f32 fn_1_3C74(f32 limit);
void fn_1_48E8(M662Player *player, s16 spatialMode);
void fn_1_4EFC(s16 streamSlot, s16 fadeSpeed);
void fn_1_5008(s16 soundSlot, s16 spatialMode, HuVecF *position);

MGTIMER *fn_1_2EC0(int timerType, int durationSeconds, int endSeconds, f32 xPosition,
                   f32 yPosition);
void fn_1_2F34(MGTIMER *timer);
s32 fn_1_2F60(MGTIMER *timer);
void fn_1_AD68(void);
s32 fn_1_A430(void);
void fn_1_2F88(MGTIMER *timer);

#endif
