/* Camera, fog, and positional audio settings used when the game starts. */
#ifndef REL_M626DLL_SETUP_H
#define REL_M626DLL_SETUP_H
#include "REL/minigame_positional_audio.h"
#include "REL/minigame_fog.h"
#include "game/object.h"
#include "dolphin/gx.h"

extern OM_CAMERA_VIEW lbl_1_data_0;
extern Vec lbl_1_data_1C;
extern Vec lbl_1_data_28;
extern GXColor lbl_1_data_34;
extern MinigameFogParams lbl_1_data_38[3];
extern MinigameFogParams lbl_1_data_80[3];
extern MinigameAudioRange lbl_1_data_C8[2];

void fn_1_52C(s16 mode, s16 frameNo);
void fn_1_6EC(s16 mode, s16 frameNo);
void fn_1_7C4(short mode, short state);
void fn_1_7FC(s16 mode, s16 frameNo);
s32 fn_1_824(s16 mode, s16 frameNo);
s32 fn_1_830(s32 fileIndex);
s32 fn_1_86C(void);
s32 fn_1_8A4(u32 sideId);
void fn_1_960(s32 playerIndex, s32 sideId, s32 winnerId);

#endif
