/* Microphone recognition and selection-window entry points used by the quiz. */
#ifndef GAME_MIC_H
#define GAME_MIC_H

#include "dolphin/types.h"

typedef void (*MCResponseCallback)(u16 *response);

s32 HuMCInit(s16 mountResult);
void HuMCClose(void);
s16 HuMCContextCreate(char *path);
void HuMCListenerCreate(s16 context, MCResponseCallback callback, u8 flags);
void HuMCListenerKill(void);
int HuMCResponseGet(void);
void HuMCSelWinCreate(f32 x, f32 y);
/* Legacy callers use this entry without a prototype. */
#if !defined(__MWERKS__)
void HuMCSelWinItemSet(s32 messageBase, s16 itemCount, s16 padNo);
#endif

#endif
