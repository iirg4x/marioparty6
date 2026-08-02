#ifndef _GAME_SAVELOAD_H
#define _GAME_SAVELOAD_H

#include "dolphin.h"
#include "game/gamework.h"
#include "game/saveload_layout.h"
#include "game/window.h"

extern u8 ATTRIBUTE_ALIGN(32) saveBuf[2][SAVE_BUF_SIZE];
extern u64 SLSerialNo[2];
extern BOOL SaveEnableF;
extern char SLSaveFileName[];
extern char SLEraseStr[];
extern HUWINID SLWinId;
extern CARDFileInfo curFileInfo;
extern GW_COMMON SLGwCommonBackup;
extern BOOL saveExecF;
extern u8 curBoxNo;
extern s16 curSlotNo;

void SLWinInit(void);
s32 SLFileOpen(char *fileName);
s32 SLFileCreate(char *fileName, u32 size, void *addr);
s32 SLFileWrite(s32 length, void *addr);
s32 SLFileRead(s32 length, void *addr);
s32 SLFileClose(void);
void SLCurSlotNoSet(s16 slotNo);
s16 SLCurSlotNoGet(void);
void SLCurBoxNoSet(s16 boxNo);
s16 SLCurBoxNoGet(void);
void SLSaveFlagSet(BOOL saveFlag);
BOOL SLSaveFlagGet(void);
void SLSaveEmptySet(s16 slotNo, s16 boxNo);
void SLSaveDataSlotMake(s16 slotNo, BOOL eraseF, OSTime *saveTime);
void SLSaveDataMake(BOOL eraseF, OSTime *saveTime);
void SLSaveDataInfoSet(s16 slotNo, OSTime *saveTime);
void SLCommonSet(void);
void SLCommonSaveCopy(GW_COMMON *commonP, s16 slotNo, s16 boxNo);
void SLBoardSave(void);
s32 SLSave(void);
s32 SLLoad(void);
void SLCommonLoad(void);
void SLCommonLoadCopy(GW_COMMON *commonP, s16 slotNo, s16 boxNo);
void SLBoardLoad(void);
s32 SLSerialNoGet(void);
BOOL SLSerialNoCheck(void);
BOOL SLCheckSumBoxSlotCheck(s16 slotNo, s16 boxNo);
BOOL SLCheckSumCheck(void);
u16 SLCheckSumSlotGet(s16 slotNo, u32 begin, u32 size);
u16 SLCheckSumGet(u32 begin, u32 size);
void SLCheckSumBoxSet(void);
void SLCheckSumBoxAllSet(void);
void SLSaveBackup(void);
void SLBoxBackupSlotLoad(s16 slotNo, s16 boxNo);
void SLBoxBackupLoad(s16 boxNo);
u32 SLBoxDataOffsetGet(s16 boxNo);
s32 SLStatSet(BOOL errorOutF);
s32 SLCardMount(s16 slotNo);
s32 SLFormat(s16 slotNo);
void SLWinIdSet(HUWINID winId);
s16 SLMessOut(s16 messId);
void SLSaveBoardTurnExec(void);
void SLSaveBoardEndExec(void);
void SLSaveModeExec(s16 sdModeF);
s32 SLSaveCheck(void);

#endif
