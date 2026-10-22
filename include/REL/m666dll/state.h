/* Persistent sequence and actor state for Shoot Yer Mouth Off. */
#ifndef REL_M666DLL_STATE_H
#define REL_M666DLL_STATE_H

#include "dolphin/types.h"

typedef struct {
    s16 shadowModelId;
    s16 modelId;
    /* Game code never reads these bytes. */
    u8 unk04[18];
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
    /* Game code never reads these bytes. */
    u8 unk1C[40];
} M666Bss98Work;

extern M666Bss98Work lbl_1_bss_98;

typedef struct {
    s16 modelId;
    union {
        struct {
            s16 slotIndices[8];
            s16 activeCount;
            s16 unknown14;
        } movingGroup;
        struct {
            s16 slotIndices[8];
            s16 activeCount;
            s16 unknown14;
        } obstacleGroup;
        struct {
            s16 obstacleIndices[8];
            s16 obstacleCount;
            s16 unknown14;
        } obstacleQueue;
        struct {
            s16 entryValues[10];
        } entryRow;
        struct {
            s16 obstacleIndices[8];
            s16 obstacleCount;
            s16 state;
        } motionEvent;
    } data;
} M666Bss580Entry;

extern M666Bss580Entry lbl_1_bss_580[6];

#endif
