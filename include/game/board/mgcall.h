#ifndef _BOARD_MGCALL_H
#define _BOARD_MGCALL_H

#include "dolphin/types.h"

void mbMgCallInit(void);
void mbMgCallDataClose(void);
s32 mbev_MgCall(void);
void mbev_MgCallDonkey(void);
void mbev_MgCallKoopa(void);
void mbev_MgCallKettou(void);
int mbMgRouletteNumGet(int type);
void mbev_MgCallSingle(int playerNo);
int mbMgCallVsEffCreate(void);

#endif
