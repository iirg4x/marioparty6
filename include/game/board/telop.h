#ifndef _BOARD_TELOP_H
#define _BOARD_TELOP_H

#include "dolphin.h"

s8 mbPadStkXGet(s32 padNo);
s8 mbPadStkYGet(s32 padNo);
void mbBoardDataDirRead(void);
void mbTauntInit(void);
void mbTauntClose(void);
void mbTelopLastTurnCreate(void);
void mbTelopCreate(int playerNo, int telopNo, BOOL waitF);
s32 mbLanguageGet(void);
s16 mbTelopTimeSprCreate(void);
void mbTelopTimeSprRotSet(s16 grpId, float rot);
void mbTelopTimeDispSet(s16 grpId, BOOL dispF);
void mbTelopTimeSprKill(s16 grpId);
void mbTelopTimeChangeCreate(void);
void mbTelopTimeChangeKill(void);
BOOL mbTelopTimeChangeCheck(void);
void mbTelopPlayerCreate(int playerNo);
BOOL mbTelopCheck(void);
void mbTelopTimeCreate(void);
void mbTelopTimeStarSet(s16 grpId, s32 starNum);
void mbTelopTimeTPLvlSet(s16 grpId, float tpLvl);

#endif
