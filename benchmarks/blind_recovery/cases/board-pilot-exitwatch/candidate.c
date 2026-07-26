static void ExitWatch(void)
{
    s32 pauseNo;
    s32 i;
    ExitObjList *list;

    while (exitReq == 0 && omSysExitReq == 0) {
        pauseNo = mbPauseStartCheck();
        if (pauseNo >= 0) {
            mbPauseCreate(pauseNo);
        }
        HuPrcVSleep();
    }

    exitFlag[0] = 1;

    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }

    while (HuARDMACheck() != 0) {
        HuPrcVSleep();
    }

    HuPrcResetStat(mbObjMan, 2);
    list = mbObjMan->list;
    for (i = 0; i < list->count; i++) {
        if ((list->entries[i].stat & 0x21) == 0) {
            omResetStatBit(&list->entries[i], 0x10);
        }
    }

    mbPauseEnableReset();
    HuPrcVSleep();
    omSysPauseCtrl(0);
    HuPrcAllPause(0);
    Hu3DPauseSet(0);
    HuSprPauseSet(0);
    HuPrcKill(mbObjMan);
    HuPrcEnd();
}
