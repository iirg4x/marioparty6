#ifndef _BOARD_MAIN_H
#define _BOARD_MAIN_H

#include "game/object.h"
#include "game/process.h"
#include "game/gamework.h"

#include "datadir_enum.h"

#ifndef HUSPR_GRP_NONE
#define HUSPR_GRP_NONE -1
#endif

extern OMOBJMAN *mbObjMan;
extern HUPROCESS *mbMainProc;

static inline int MBBoardNoGet(void)
{
    return lbl_80265B70.boardNo;
}

static inline BOOL MBTimeDayGet(void)
{
    return lbl_80265B70.curTime == 0;
}

BOOL mbExitCheck(void);
void mbObjectSetup(s32 boardNo, void (*init)(void), void (*close)(void));
void mbev_NextTimeSet(void (*nextTimeHook)(void));
void mbev_ShopInit(int dataNum);
void mbev_ShopBackCreate(int dataNum, int motDataNum, int motNo, BOOL linkF);
void mbev_ShopBackMotCreate(int dataNum, int motDataNum, int motNo,
    BOOL linkF, char *hookName);
void mbExitReq(void);
void mbExitWatchCreate(void);
void mbPauseEnableSet(void);
void mbPauseEnableReset(void);
BOOL mbPauseEnableCheck(void);
void mbConfigPadDisableSet(BOOL disableF);
void mbLightFuncSet(void (*lightSetHook)(void), void (*lightResetHook)(void));
void mbMapCameraSet(const HuVecF *rot, const HuVecF *pos, float zoom);
void mbMapHookSet(void (*hook)(BOOL enterF));
void mbScrollInit(int dataNum);
void mbSingleReturnWrite(void);
int mbBoardDataNumGet(int dataNum);
void mbDirClose(void);
void mbOvlCall(OMOVL nextOverlay);
u32 mbRandMod(u32 mod);
BOOL mbAngleMoveTo(float *dest, float angle, float speed);
void mbev_Last5(void);
void mbSaveInit(s32 boardNo);
void mbSavePartyInit(BOOL teamF, BOOL bonusStarF, s32 mgPack, s32 turnMax,
    s32 handicapP1, s32 handicapP2, s32 handicapP3, s32 handicapP4);
void mbChangeTime(void);
s32 mbBGRead(s32 boardDataNum);
void mbBGReadWait(s32 dataReadStat);
int mbev_Shop(int playerNo, int shopNo);

#endif
