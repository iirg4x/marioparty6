#ifndef REL_M663DLL_INPUT_H
#define REL_M663DLL_INPUT_H

#include "dolphin/types.h"
#include "game/mg/timer.h"

typedef struct M663PadInput {
    s32 unk00; /* The game's code never reads this field. */
    s32 buttonsDown;
    u16 unk08; /* The game's code never reads this field. */
    s8 unk0A; /* The game's code never reads this field. */
    s8 unk0B; /* The game's code never reads this field. */
    s8 unk0C; /* The game's code never reads this field. */
    s8 unk0D; /* The game's code never reads this field. */
    u8 unk0E; /* The game's code never reads this field. */
    u8 unk0F; /* The game's code never reads this field. */
    u8 unk10; /* The game's code never reads this field. */
    u8 unk11; /* The game's code never reads this field. */
} M663PadInput;

typedef struct M663ModuleInput {
    u8 unk_00[8]; /* No code names this field. */
    s16 phase;
    s16 phaseTimer;
    s16 transitionMode;
    s16 transitionState;
    s16 transitionTimer;
    s16 winnerReady;
    u8 unk_14[4]; /* No code names this field. */
    M663PadInput padInputs[4];
    MGTIMER *timer;
    s32 frame;
    s16 unk70; /* The game's code never reads this field. */
    s16 effectTimer;
    f32 orbitAngle;
    s16 rumbleEnabled;
    s16 unk7A; /* The game's code never reads this field. */
    u8 unk_7C[4]; /* No code names this field. */
    s32 bgmHandle;
    s32 bgmId;
    s32 jingleHandle;
    s32 jingleId;
    u8 unk_90[4]; /* No code names this field. */
    s32 soundHandles[4];
    s16 soundActive[4];
    s16 minPanX;
    s16 maxPanX;
    s16 centerPanX;
    f32 minX;
    f32 maxX;
    s16 minPanY;
    s16 maxPanY;
    u8 unk_C0[4]; /* No code names this field. */
    f32 minY;
    f32 maxY;
} M663ModuleInput;

extern M663ModuleInput lbl_1_bss_424;

void fn_1_3ABC(MGTIMER *timer);
void M663SoundStop(s16 soundIndex);
void M663SoundStopCopy(s16 soundIndex);
void fn_1_5D14(void);
void fn_1_5EBC(void);
void fn_1_5F0C(s16 streamSlot, s16 fadeFrames);
void fn_1_6018(s16 playerIndex, s16 mode, Vec *position);
void fn_1_67E4(void);

#endif
