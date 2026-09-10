static int ev_CapKoopaCoin(CAPWORK *work)
{
    extern void mbStatusDispForceSetAll(BOOL dispF);
    extern void mbPauseDisableSet(BOOL disableF);
    HuVecF masuPos;
    int add[GW_PLAYER_MAX];
    int ids[4];
    int teamTbl[2][GW_PLAYER_MAX];
    int teamCount[2];
    BOOL teamLose[2];
    int playerNo = work->playerNo;
    int masuId = GwPlayer[playerNo].masuId;
    int objId = work->eventData[0];
    int squishCount;
    int resultType;
    int loseCount;
    BOOL hasResource;
    int i;
    int t;
    BOOL removed;

    mbev_PlayerColMasu(playerNo, masuId, TRUE);
    squishCount = mbev_CapPlayerSquishVoiceSet(ids, masuId, TRUE);
    work->eventData[1] = squishCount;
    for (i = 0; i < 4; i++) {
        work->eventData[i + 2] = ids[i];
    }
    mbMasuPosGet(masuId, &masuPos);
    mbObjPosSetV(objId, &masuPos);
    mbObjDispSet(objId, TRUE);
    mbObjMotionSet(objId, 1, HU3D_MOTATTR_LOOP);
    mbev_CapObjPosSet(&work->objWork, objId, masuId, NULL);
    mbStatusDispForceSetAll(TRUE);
    mbCameraEyeSetV(&masuPos);
    mbCameraPlayerViewSetFast(playerNo, 0);
    mbCameraMoveWait();
    if (work->flags._flag04) {
        mbMusPlay(0, 27, 127, 0);
    }
    mbWipeFadeIn();
    mbPauseDisableSet(FALSE);

    resultType = mgResultData.resultNo;
    if (!GWTeamFGet()) {
        for (i = 0, loseCount = 0; i < GW_PLAYER_MAX; i++) {
            if (GWMgCoinBonusGet(i) <= 0) {
                loseCount++;
            }
        }
        if (resultType == 2) {
            for (i = 0, hasResource = TRUE; i < GW_PLAYER_MAX; i++) {
                if (GWMgCoinBonusGet(i) <= 0 && mbPlayerCapsuleNumGet(i) > 0) {
                    hasResource = FALSE;
                }
            }
        } else {
            for (i = 0, hasResource = TRUE; i < GW_PLAYER_MAX; i++) {
                if (GWMgCoinBonusGet(i) <= 0 && mbPlayerCoinGet(i) > 0) {
                    hasResource = FALSE;
                }
            }
        }
    } else {
        teamCount[0] = teamCount[1] = 0;
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (mbPlayerGrpGet(i) == 0) {
                teamTbl[0][teamCount[0]] = i;
                teamCount[0]++;
            } else {
                teamTbl[1][teamCount[1]] = i;
                teamCount[1]++;
            }
        }
        loseCount = teamLose[0] = teamLose[1] = FALSE;
        if (GWMgCoinBonusGet(teamTbl[0][0]) <= 0
            && GWMgCoinBonusGet(teamTbl[0][1]) <= 0) {
            teamLose[0] = TRUE;
            loseCount++;
        }
        if (GWMgCoinBonusGet(teamTbl[1][0]) <= 0
            && GWMgCoinBonusGet(teamTbl[1][1]) <= 0) {
            teamLose[1] = TRUE;
            loseCount++;
        }
        if (resultType == 2) {
            for (i = 0, hasResource = TRUE; i < 2; i++) {
                if (teamLose[i]
                    && mbPlayerCapsuleNumGet(teamTbl[i][0]) > 0) {
                    hasResource = FALSE;
                }
            }
        } else {
            for (i = 0, hasResource = TRUE; i < 2; i++) {
                if (teamLose[i] && mbPlayerCoinGet(teamTbl[i][0]) > 0) {
                    hasResource = FALSE;
                }
            }
        }
    }
    mbAudFXDelaySet(30);
    mbAudFXPlay(0x3CB); /* event sound-effect resource */
    mbObjMotionShiftSet(objId, 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    if (!GWTeamFGet()) {
        mbWinCreate(2, 0x003F000E, 13); /* Koopa scene message resource */
    } else {
        mbWinCreate(2, 0x003F000F, 13); /* Koopa scene message resource */
    }
    mbWinTopInsertMesSet(koopaLoseMesTbl2[resultType], 0);
    mbWinTopWait();
    mbAudFXDelaySet(30);
    mbAudFXPlay(0x3CD); /* event sound-effect resource */
    mbev_CapPlayerMotShiftSet(objId, 5, 0, TRUE);
    mbAudFXPlay(0x427); /* event sound-effect resource */
    if (loseCount == 0) {
        HuPrcSleep(30);
        mbAudFXPlay(0x3CC); /* event sound-effect resource */
        mbObjMotionShiftSet(objId, 6, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
        if (!GWTeamFGet()) {
            mbWinCreate(2, 0x003F0012, 13); /* Koopa scene message resource */
        } else {
            mbWinCreate(2, 0x003F0013, 13); /* Koopa scene message resource */
        }
        mbWinTopWait();
    } else if (hasResource) {
        HuPrcSleep(30);
        mbAudFXPlay(0x3CC); /* event sound-effect resource */
        mbObjMotionShiftSet(objId, 6, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
        if (resultType == 2) {
            mbWinCreate(2, 0x003F0011, 13); /* Koopa scene message resource */
        } else {
            mbWinCreate(2, 0x003F0010, 13); /* Koopa scene message resource */
        }
        mbWinTopWait();
    } else if (!GWTeamFGet()) {
        switch (resultType) {
        case 0:
            for (i = 0; i < GW_PLAYER_MAX; i++) {
                if (GWMgCoinBonusGet(i) <= 0) {
                    add[i] = -((mbPlayerCoinGet(i) + 1) / 2);
                    omVibrate(i, 20, 20, 0);
                } else {
                    add[i] = 0;
                }
            }
            mbCoinAddAllProcExecV(add, (BOOL *)add, FALSE);
            break;
        case 1:
            for (i = 0; i < GW_PLAYER_MAX; i++) {
                if (GWMgCoinBonusGet(i) <= 0) {
                    add[i] = -mbPlayerCoinGet(i);
                    omVibrate(i, 20, 20, 0);
                } else {
                    add[i] = 0;
                }
            }
            mbCoinAddAllProcExecV(add, (BOOL *)add, FALSE);
            break;
        default:
            for (i = 0; i < GW_PLAYER_MAX; i++) {
                if (GWMgCoinBonusGet(i) <= 0) {
                    omVibrate(i, 20, 20, 0);
                }
            }
            for (i = 0; i < mbPlayerCapsuleMaxGet(); i++) {
                for (t = 0, removed = FALSE; t < GW_PLAYER_MAX; t++) {
                    if (GWMgCoinBonusGet(t) <= 0) {
                        mbPlayerCapsuleRemove(t, 0);
                        removed = TRUE;
                    }
                }
                if (removed) {
                    HuPrcSleep(10);
                }
            }
            break;
        }
    } else {
        switch (resultType) {
        case 0:
            for (i = 0; i < 2; i++) {
                if (teamLose[i]) {
                    mbCoinAddDispExec(teamTbl[i][0],
                        -(mbPlayerCoinGet(teamTbl[i][0]) / 2),
                        FALSE, FALSE);
                    omVibrate(teamTbl[i][0], 20, 20, 0);
                    omVibrate(teamTbl[i][1], 20, 20, 0);
                }
            }
            break;
        case 1:
            for (i = 0; i < 2; i++) {
                if (teamLose[i]) {
                    mbCoinAddDispExec(teamTbl[i][0],
                        -mbPlayerCoinGet(teamTbl[i][0]), FALSE, FALSE);
                    omVibrate(teamTbl[i][0], 20, 20, 0);
                    omVibrate(teamTbl[i][1], 20, 20, 0);
                }
            }
            break;
        default:
            for (i = 0; i < 2; i++) {
                if (teamLose[i]) {
                    omVibrate(teamTbl[i][0], 20, 20, 0);
                    omVibrate(teamTbl[i][1], 20, 20, 0);
                }
            }
            for (i = 0; i < mbPlayerCapsuleMaxGet(); i++) {
                for (t = 0, removed = FALSE; t < 2; t++) {
                    if (teamLose[t]) {
                        mbPlayerCapsuleRemove(teamTbl[t][0], 0);
                        removed = TRUE;
                    }
                }
                if (removed) {
                    HuPrcSleep(10);
                }
            }
            break;
        }
    }
    mbev_CapPlayerMotShiftSet(objId, 1, HU3D_MOTATTR_LOOP, TRUE);
    return 0;
}
