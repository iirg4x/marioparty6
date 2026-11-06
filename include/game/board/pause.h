#ifndef _BOARD_PAUSE_H
#define _BOARD_PAUSE_H

#include "types.h"

typedef void (*MBPAUSEHOOK)(BOOL pauseF);

void mbPauseInit(void);
void mbPauseCreate(int playerNo);
int mbPauseStartCheck(void);
BOOL mbPauseProcCheck(void);
void mbPauseDisableSet(BOOL disableF);
BOOL mbPauseDisableGet(void);
void mbPauseHookPush(MBPAUSEHOOK hook);
void mbPauseHookPop(MBPAUSEHOOK hook);
void mbPauseSet(BOOL pauseF);
int mbPauseDataDirGet(int type);
int mbPauseDataNumGet(int dataNum);
BOOL mbConfigExec(int playerNo, int modelId);
void mbPauseGuideCreate(void);
void mbPauseGuideKill(void);
void mbPauseDispCopyCreate(void);
void mbPauseDispCopyKill(void);
s16 mbPausePanelCreate(int dataNum, unsigned int espDataNum);
void mbPausePanelPosSet(s16 panelId, float panelX, float panelY);
void mbPausePanelBankSet(s16 panelId, int animBank);
void mbPausePanelGrowSet(s16 panelId, int time, int delay, float scale);
void mbPausePanelKill(s16 panelId);

#endif
