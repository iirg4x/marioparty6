#ifndef M661DLL_PLAYER_STATE_H
#define M661DLL_PLAYER_STATE_H

#include "dolphin/types.h"
#include "game/mg/actman.h"
#include "REL/m661dll/player_actors.h"

typedef struct PlayerResult {
    u8 unknown00[0x14]; /* No code names this field. */
    s8 controller;
    u8 unknown15;
    s8 character;
    u8 unknown17[2]; /* No code names this field. */
    s8 playerType;
    u8 unknown1A;
    s8 active;
    s8 resultCharacter;
    u8 unknown1D[0x17]; /* No code names this field. */
    f32 x, y, z;
    f32 previousX, previousY, previousZ;
    f32 rotationX, rotationY, rotationZ;
    s8 playerStatus[2];
    s16 currentValue, previousValue, pairedIndex, previousPairedIndex, updateCount;
    s16 score;
    s32 padX, padY, stickX, stickY;
    u8 unknown78[2]; /* No code names this field. */
    s16 boundaryPhase;
    u8 unknown7C[0xBC - 0x7C]; /* No code names this field. */
} PlayerResult;

#ifdef __cplusplus
extern "C" {
#endif
extern PlayerResult lbl_1_bss_1C04[2];
void fn_1_4918(PlayerResult *object, s16 mode);
void fn_1_2B58(void);
void fn_1_243E8(PlayerResult *self);
s16 fn_1_24470(HuVecF position);
#ifdef __cplusplus
}
#endif

#endif
