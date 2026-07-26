#include "game/gamework.h"
#include "game/object.h"
#include "game/process.h"

typedef struct {
    int playerNo;
    int masuId;
    int capsuleNo;
} CAPWORK;

void mbev_CapWait(CAPWORK *work);
BOOL mbCapEffUseCreate(int playerNo, int capsuleNo);
void mbev_CapRandomBonusCoin(int playerNo, int capsuleNo, BOOL waitF);
int mbCapEffUseModeGet(int playerNo);

void mbev_CapKiller(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;

    mbev_CapWait(work);
    mbCapEffUseCreate(work->playerNo, work->capsuleNo);
    omVibrate(work->playerNo, 20, 4, 4);
    HuPrcSleep(20);
    mbev_CapRandomBonusCoin(work->playerNo, work->capsuleNo, TRUE);
    while (mbCapEffUseModeGet(work->playerNo) >= 0) {
        HuPrcVSleep();
    }
    GwPlayer[work->playerNo].diceMode = 5;
    HuPrcEnd();
}

void mbev_CapKinokoKill(void)
{
}

void mbev_CapSKinokoKill(void)
{
}

void mbev_CapPKinokoKill(void)
{
}

void mbev_CapMKinokoKill(void)
{
}

void mbev_CapKillerKill(void)
{
}

void mbev_CapDokanKill(void)
{
}

void mbev_CapHanachanKill(void)
{
}

void mbev_CapNKinokoKill(void)
{
}

void mbev_CapKillerMoveKill(void)
{
}
