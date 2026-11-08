#ifndef M661DLL_SYSTEM_STATE_H
#define M661DLL_SYSTEM_STATE_H

#include "dolphin/types.h"
#include "game/mg/timer.h"
#include "dolphin/mtx/GeoTypes.h"

typedef struct SystemPadInput {
    s32 buttons;
    s32 buttonsDown;
    u16 buttonsRepeat;
    s8 stickX;
    s8 stickY;
    s8 subStickX;
    s8 subStickY;
    u8 triggerL;
    u8 triggerR;
    u8 dPad;
    u8 dPadRepeat;
} SystemPadInput;

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
    s16 result;
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
    f32 playerOverlapRatios[16];
    Point3d secondaryPositions[4];
    Point3d secondaryGridStep;
    s16 targetChoiceCounts[4];
    f32 regionOverlapRatios[4];
    Point3d specialBounds[2];
    Point3d specialExpansion;
    s16 playerChoiceCounts[2];
    f32 specialOverlapRatios[2];
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

#endif
