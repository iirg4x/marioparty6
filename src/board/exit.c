#include <math.h>
#include "game/board/main.h"
#include "game/board/pause.h"
#include "game/armem.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/process.h"
#include "game/sprite.h"
#include "game/wipe.h"

static s32 exitFlag[2];
static s32 exitReq;
static HUPROCESS *exitWatchProc;

static void ExitWatch(void);
static void ExitWatchDestroy(void);

void mbExitWatchCreate(void)
{
    exitWatchProc = fn_8000D818(ExitWatch, 0x2013, 0x2000, 0);
    HuPrcSetStat(exitWatchProc, 0x4 | 0x8);
    HuPrcDestructorSet2(exitWatchProc, ExitWatchDestroy);
    exitFlag[0] = 0;
    exitReq = 0;
}

static void ExitWatch(void)
{
    s32 pauseNo;
    OMOBJWORK *objWork;
    s32 i;

    while (exitReq == 0 && omSysExitReq == 0) {
        pauseNo = mbPauseStartCheck();
        if (pauseNo >= 0) {
            mbPauseCreate(pauseNo);
        }
        HuPrcVSleep();
    }
    exitFlag[0] = 1;
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    while (fn_80055FB4()) {
        HuPrcVSleep();
    }
    HuPrcResetStat(mbObjMan, 0x2);
    objWork = mbObjMan->property;
    for (i = 0; i < objWork->objMax; i++) {
        if ((objWork->objData[i].stat & ((1 << 0) | (1 << 5))) == 0) {
            omResetStatBit(&objWork->objData[i], (1 << 4));
        }
    }
    mbPauseEnableReset();
    HuPrcVSleep();
    fn_8003E3F0(0);
    HuPrcAllPause(0);
    Hu3DPauseSet(0);
    HuSprPauseSet(0);
    HuPrcKill(mbObjMan);
    HuPrcEnd();
}

static void ExitWatchDestroy(void)
{
    exitWatchProc = NULL;
}

void mbExitReq(void)
{
    exitReq = 1;
}

BOOL mbExitCheck(void)
{
    return exitFlag[0];
}

HUPROCESS *mbExitWatchProcGet(void)
{
    return exitWatchProc;
}
