#ifndef REL_M663DLL_PLAYER_H
#define REL_M663DLL_PLAYER_H

#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "REL/m663dll/input.h"

typedef struct M663PlayerWork {
    M663PadInput input;
    s8 playerNo;
    s8 playerIndex;
    s8 characterNo;
    s8 padNo;
    s8 groupNo;
    s8 playerType;
    s8 difficulty;
    s8 inputMode;
    s8 mode;
    s32 score;
    u8 unk_24[4]; /* No code names this field. */
    s32 stateTimer;
    u8 unk_2C[4]; /* No code names this field. */
    s16 requestPending;
    s8 enabled;
    Vec position;
    Vec previousPosition;
    Vec rotation;
    s8 transformMode;
    s16 motionIndex;
    s16 previousMotionIndex;
    s16 pairedIndex;
    s16 previousPairedIndex;
    s16 chaseTimer;
    char *objectName;
    s16 skipApproach;
    s16 rumbleFrames;
    s16 scoreStatus;
    s16 carrying;
    s16 progress;
    s16 effectPending;
    f32 effectFrame;
    f32 lastAngle;
    s32 motionCount;
    s32 resetValues[4];
    s16 step;
    s16 sequenceState;
    u8 unk_94[2]; /* No code names this field. */
    s16 state;
    s16 targetState;
    s16 waitTimer;
    s16 communicationState;
    Vec target;
    f32 chaseSpeed;
    f32 chaseRadius;
    s16 stopTimer;
    s16 actionTimer;
    f32 orbitRadius;
    s16 slowChaseChance;
    u8 unk_BE[2]; /* No code names this field. */
    s16 actionFrames;
    s16 startFrame;
    s16 endFrame;
    s16 endFrame2;
    s16 callTimer;
} M663PlayerWork;

extern M663PlayerWork lbl_1_bss_4F0[2];

#endif
