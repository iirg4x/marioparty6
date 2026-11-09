#ifndef GAME_MIC_H
#define GAME_MIC_H

#include "dolphin/types.h"

typedef void (*MCResponseCallback)(u16 *response);

s32 HuMCInit(s16 mountResult);
void HuMCThresholdSet(u32 threshold);
s32 HuMCMount(s32 chan);
s32 HuMCProbe(s32 channel);
s32 HuMCMicGet(void);
void HuMCMicSet(s32 flag);
void HuMCClose(void);
s16 HuMCContextCreate(char *path);
void HuMCContextKill(s16 context);
void HuMCListenerCreate(s16 context, MCResponseCallback callback, u8 flags);
void HuMCListenerKill(void);
int HuMCResponseGet(void);
u16 *HuMCCurResponseGet(void);
s32 HuMCUnkResponseCheck(void);
s32 HuMCNewResponseGet(s32 messageBase);
void HuMCSelWinCreate(f32 x, f32 y);
s32 HuMCSelWinItemSet(s32 messageBase,s16 itemCount,s16 padNo);
void HuMCSelWinContextSet(s16 contextId, MCResponseCallback responseCallback, u8 padNo);
int HuMCSelWinChoiceGet(void);
BOOL HuMCSelWinCheck(void);
void HuMCSelWinContextKill(void);
void HuMCSessionSet(s16 playerIndex);
void HuMCSessionClose(void);
s32 HuMCButtonGet(void);
s32 HuMCButtonDownGet(void);
void HuMCSysInit(void);
void HuMCPeriodicProc(void);

#endif
