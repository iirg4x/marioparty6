/* Persistent sequence and actor state for Shoot Yer Mouth Off. */
#ifndef REL_M666DLL_STATE_H
#define REL_M666DLL_STATE_H

#include "dolphin/types.h"

typedef struct {
    s16 shadowModelId;
    s16 modelId;
    u8 unk_04[18]; /* No code names this field. */
    s16 night;
    s16 sequenceState;
    s16 timer;
    /* Selects the result after gameplay: zero for three players, one for the microphone player. */
    s16 microphonePlayerWon;
    s32 soundHandle;
} M666InitialState;

typedef struct {
    s16 modelId;
    s16 state;
    s16 perimeterIndex;
    /* Game code never reads this field. */
    u16 unknown6;
    s32 soundHandle;
} M666SoundWork;

extern M666InitialState lbl_1_bss_DF0;
extern M666SoundWork lbl_1_bss_DC[9];

typedef struct {
    s16 modelId;
    s16 jointMotionIds[7];
    s16 spriteGroupId;
    s16 motionState;
    s16 targetIndex;
    s16 direction;
    s16 turnDirection;
    s16 countdown;
    f32 facingAngle;
    f32 rotation;
} M666MotionWork;

extern M666MotionWork lbl_1_bss_D84[3];

typedef M666MotionWork M666PerimeterWork;

typedef struct {
    s16 modelId;
    s16 motionIds[6];
    s16 playerIndex;
    s16 difficulty;
    s16 padNo;
    s16 charNo;
    s16 motionState;
    s16 eventStatus;
    s16 delayCounter;
    u8 unk_1C[40]; /* No code names this field. */
} M666MicrophonePlayer;

extern M666MicrophonePlayer lbl_1_bss_98;

typedef struct {
    s16 modelId;
    s16 obstacleIndices[8];
    s16 obstacleCount;
    s16 state;
} M666ObstacleQueue;

extern M666ObstacleQueue lbl_1_bss_580[6];

#endif
