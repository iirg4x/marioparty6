#ifndef M661DLL_SYSTEM_STATE_H
#define M661DLL_SYSTEM_STATE_H

#include "REL/minigame_shadow_camera.h"
#include "REL/minigame_pad.h"
#include "dolphin/types.h"
#include "game/mg/timer.h"
#include "dolphin/mtx/GeoTypes.h"

typedef struct SystemInputState {
    /* fn_1_118 clears the full 0x298-byte object; sequenceState follows these bytes. */
    u8 unknown00[0x08]; /* No code names this field. */
    s16 sequenceState;
    s16 sequenceTimer;
    f32 heading;
    f32 transparency;
    s16 phase;
    s16 roundMode;
    /* The timer follows this unidentified interval in the cleared object. */
    u8 unk_18[2]; /* No code names this field. */
    s16 timer;
    s16 waitTimer;
    s16 unk20; /* The game's code never reads this field. */
    s16 activeCount;
    SystemPadInput pads[4];
    MGTIMER *action;
    s32 frameCounter;
    s16 finished;
    s16 mode;
    s16 debugSelection;
    Point3d savedCameraCenter;
    Point3d savedCameraRotation;
    f32 savedCameraZoom;
    f32 modelSpeed;
    Point3d initialPositions[16];
    Point3d initialGridStep;
    s16 boardChoiceCounts[16];
    f32 unk190[16]; /* The game's code never reads this field. */
    Point3d secondaryPositions[4];
    Point3d secondaryGridStep;
    s16 targetChoiceCounts[4];
    f32 unk214[4]; /* The game's code never reads this field. */
    Point3d specialBounds[2];
    Point3d specialExpansion;
    s16 playerChoiceCounts[2];
    f32 unk24C[2]; /* The game's code never reads this field. */
    /* The sound setup accesses bgmHandle after this unidentified interval. */
    u8 unk_254[4]; /* No code names this field. */
    s32 bgmHandle;
    s32 bgmId;
    s32 jingleHandle;
    s32 jingleId;
    /* The sound routines access soundHandles after this unidentified interval. */
    u8 unk_268[4]; /* No code names this field. */
    s32 soundHandles[3];
    s16 soundActive[3];
    s16 panMin;
    s16 panMax;
    f32 panPositionMin;
    f32 panPositionMax;
    s16 volumeMin;
    s16 volumeMax;
    f32 volumePositionMin;
    f32 volumePositionMax;
} SystemInputState;

extern SystemInputState lbl_1_bss_196C;

/* Shared state defined by the module core. */
extern MinigameShadowCamera lbl_1_bss_2FEC;

typedef struct ModelAnimationWork {
    s16 motionIds[5];
    s16 motionId;
    s16 appliedMotionId;
    f32 motionSpeed;
    f32 appliedMotionSpeed;
    f32 motionTime;
    f32 appliedMotionTime;
    f32 unk20; /* The game's code never reads this field. */
    u32 modelAttributes;
    u32 appliedModelAttributes;
    u32 attributesToReset;
    s32 motionEnded;
    s16 repeatMotion;
    s16 nextMotionId;
    f32 nextMotionTime;
    u32 nextModelAttributes;
    u32 transitionState;
    s16 shiftState;
    s16 shiftedMotionId;
    u32 shiftAttributes;
    f32 shiftTime;
    f32 shiftDuration;
    f32 shiftElapsed;
    f32 unk58; /* The game's code never reads this field. */
    /* The two animation records have a 0x60-byte stride; these bytes are unidentified. */
    u8 unknown5C[4]; /* No code names this field. */
} ModelAnimationWork;

extern ModelAnimationWork lbl_1_bss_1D7C[2];

#endif
