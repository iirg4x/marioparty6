#ifndef M661DLL_PLAYER_STATE_H
#define M661DLL_PLAYER_STATE_H

#include "dolphin/types.h"
#include "humath.h"
#include "REL/m661dll/player_actors.h"
#include "REL/m661dll/system_state.h"

/* Player state used by input, movement, and the results sequence. */
typedef struct M661PlayerState {
    SystemPadInput input;
    s8 groupSlot;
    s8 playerIndex;
    s8 character;
    s8 controllerPort;
    s8 unk18; /* The game's code never reads this field. */
    s8 playerType;
    s8 computerDifficulty;
    s8 active;
    s8 resultCharacter;
    u8 unk1D[19]; /* No code names this field. */
    s8 unk30; /* The game's code never reads this field. */
    HuVecF position;
    HuVecF previousPosition;
    HuVecF rotation;
    s8 playerStatus[2];
    s16 currentValue;
    s16 unk5C; /* The game's code never reads this field. */
    s16 pairedIndex;
    s16 previousPairedIndex;
    s16 updateCount;
    s16 score;
    s16 boundaryAdjustmentFrames;
    s32 padX;
    s32 padY;
    s32 stickX;
    s32 stickY;
    s16 inputPhase;
    s16 choicePhase;
    u8 unk7C[2]; /* No code names this field. */
    s16 phase;
    HuVecF movementTarget;
    u8 unk8C[12]; /* No code names this field. */
    f32 chaseSpeed;
    f32 chaseRadius;
    u8 unkA0[2]; /* No code names this field. */
    s16 target;
    s16 targetKind;
    s16 action;
    u8 unkA8[2]; /* No code names this field. */
    s16 firstResetValue;
    s16 secondResetValue;
    s16 settleTimer;
    s16 forceStop;
    s16 selectedModel;
    s16 targetCount;
    s16 roundCount;
    s16 overrideChance;
} M661PlayerState;

#ifdef __cplusplus
extern "C" {
#endif
extern M661PlayerState lbl_1_bss_1C04[2];
void fn_1_4918(M661PlayerState *object, s16 mode);
void fn_1_2B58(void);
void fn_1_243E8(M661PlayerState *self);
s16 fn_1_24470(HuVecF position);
#ifdef __cplusplus
}
#endif

#endif
