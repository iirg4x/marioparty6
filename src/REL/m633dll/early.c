/* Sequence callbacks and shared labels for the Ray of Fright minigame. */
#include "REL/m633dll.h"

#define M633_GAMEPLAY_BGM_ID 75

MGSEQ_PARAM lbl_1_data_0 = {
    0, 0, fn_1_224, fn_1_310, fn_1_55C, fn_1_690, fn_1_AB0, fn_1_C78,
    fn_1_118C, fn_1_1284, fn_1_1288,
};

char lbl_1_data_28[25] = "633biribiri-EDpattern1P1";

char lbl_1_data_41[25] = "633biribiri-EDpattern2P1";

char lbl_1_data_5A[25] = "633biribiri-EDpattern2P2";

char lbl_1_data_73[25] = "633biribiri-EDpattern3P1";

char lbl_1_data_8C[25] = "633biribiri-EDpattern3P2";

char lbl_1_data_A5[27] = "633biribiri-EDpattern3P3";

char lbl_1_data_C0[] = "";

char lbl_1_data_C1[] = "633biribiri-P2st";

char lbl_1_data_D2[] = "633biribiri-P3st";

char lbl_1_data_E3[] = "633biribiri-P4st";

char *lbl_1_data_F4[4] = { lbl_1_data_C0, lbl_1_data_C1, lbl_1_data_D2, lbl_1_data_E3 };

char lbl_1_data_104[27] = "633biribiri-cannonA_shadow";

char lbl_1_data_11F[23] = "633biribiri-nozzleNull";

char lbl_1_data_136[25] = "633biribiri-stage_shadow";

char lbl_1_data_14F[17] = "633biribiri-P1st";

char lbl_1_data_160[23] = "633biribiri-nozzleNull";

char lbl_1_data_177[17] = "hit!!!!!!!!(%f)\n";

char lbl_1_data_188[24] = "633biribiri-nozzleNull";

char lbl_1_data_1A0[24] = "hit!!!!!!!!(%f)\n";

s32 lbl_1_data_1B8[6] = { DATANUM(DATA_m633, 12), DATANUM(DATA_m633, 13), DATANUM(DATA_m633, 14),
                          DATANUM(DATA_m633, 15), DATANUM(DATA_m633, 20), DATANUM(DATA_m633, 21) };

s32 lbl_1_data_1D0[4] = { DATANUM(DATA_m633, 25), DATANUM(DATA_m633, 27), DATANUM(DATA_m633, 26),
                          DATANUM(DATA_m633, 24) };

unsigned int lbl_1_data_1E0[16] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 1),
    DATANUM(DATA_mariomot, 2), DATANUM(DATA_mariomot, 3),
    DATANUM(DATA_mariomot, 4), DATANUM(DATA_mariomot, 6),
    DATANUM(DATA_mariomot, 40), DATANUM(DATA_mariomot, 35),
    DATANUM(DATA_mariomot, 10), DATANUM(DATA_mariomot, 9),
    DATANUM(DATA_mariomot, 30), DATANUM(DATA_mariomot, 20),
    DATANUM(DATA_mariomot, 61), DATANUM(DATA_mario, 86),
    DATANUM(DATA_mariomot, 24), 0,
};

unsigned int lbl_1_data_220[16] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 1),
    DATANUM(DATA_mariomot, 2), DATANUM(DATA_mariomot, 3),
    DATANUM(DATA_mariomot, 4), DATANUM(DATA_mariomot, 38),
    DATANUM(DATA_mariomot, 40), DATANUM(DATA_mariomot, 35),
    DATANUM(DATA_mariomot, 10), DATANUM(DATA_mariomot, 9),
    DATANUM(DATA_mariomot, 30), DATANUM(DATA_mariomot, 20),
    DATANUM(DATA_mariomot, 61), DATANUM(DATA_mario, 86),
    DATANUM(DATA_mariomot, 24), 0,
};

u32 lbl_1_data_260[4] = { 100U, 70U, 30U, 0U };

u32 lbl_1_data_270[4] = { 20U, 45U, 55U, 100U };

u32 lbl_1_data_280[4] = { 15U, 25U, 40U, 100U };

M633Work lbl_1_bss_0;

/* Starts the requested BGM when no handle is active and the sequence message has
 * GAMEMES_STAT_FXPLAY set. */
s32 fn_1_A0(s32 streamHandle, s32 bgmId)
{
    s32 result;

    result = streamHandle;
    if (result == -1 && (GameMesStatGet(MgSeqGameMesIdGet()) & GAMEMES_STAT_FXPLAY) != 0) {
        result = HuAudBGMPlay((s16) bgmId);
    }
    return result;
}

/* Fades an active sequence stream out; fn_1_AB0 uses this when the finish phase begins. */
void fn_1_104(s32 streamHandle)
{
    if (streamHandle != -1) {
        HuAudSStreamFadeOut(streamHandle, 100);
    }
}

/* Updates each character voice pan from its projected screen position during sequence callbacks. */
void fn_1_140(void)
{
    Point3d pos;
    Point3d projected;
    s32 pan;
    s32 player;

    for (player = 0; player < 4; player++) {
        pos = lbl_1_bss_0.players[player]->actor->pos;
        Hu3D3Dto2D(&pos, 1, &projected);
        pan = (s32) projected.x;
        pan /= 5;
        if (pan < 32) {
            pan = 32;
        } else if (pan > 96) {
            pan = 96;
        }
        CharModelVoicePanSet((s16) lbl_1_bss_0.characterNumbers[player], (s16) pan);
    }
}

/* Sequence callback: updates actors and voice panning, then requests the next sequence mode. */
void fn_1_224(s16 mode, s16 frameNo)
{
    MgActorExec();
    fn_1_140();
    MgSeqModeNext();
}

/* On entry, starts camera motion 0 and assigns slot callbacks; requests the next mode after the
 * motion ends and three group-zero slots reach state 4. */
void fn_1_310(s16 mode, s16 frameNo)
{
    s32 completedPlayerCount;
    s32 playerNo;

    completedPlayerCount = 0;
    fn_1_6DE8();
    lbl_1_bss_0.segmentUpdateCountdown -= 1;
    if (lbl_1_bss_0.segmentReflectionSoundPending != 0) {
        HuAudFXPlay(M633_SEGMENT_WALL_REFLECTION_SE_ID);
    }
    lbl_1_bss_0.segmentReflectionSoundPending = 0;
    MgActorExec();
    fn_1_140();
    lbl_1_bss_0.preventArenaRestart = 1;
    if (frameNo == 0) {
        fn_1_7E70(0);
        playerNo = 0;
        while (playerNo < 4) {
            if ((s32) lbl_1_bss_0.outsideGroupZero[playerNo] != 0) {
                (lbl_1_bss_0.playerObjects[playerNo])->objFunc = fn_1_32C0;
            } else {
                (lbl_1_bss_0.playerObjects[playerNo])->objFunc = fn_1_26B8;
            }
            playerNo += 1;
        }
        return;
    }
    if (fn_1_7F40() != 0) {
        playerNo = 0;
        while (playerNo < 4) {
            if (((s32) lbl_1_bss_0.outsideGroupZero[playerNo] == 0) &&
                ((u32) (lbl_1_bss_0.playerObjects[playerNo])->work[1] == 4U)) {
                completedPlayerCount += 1;
            }
            playerNo += 1;
        }
        if (completedPlayerCount == 3) {
            MgSeqModeNext();
        }
    }
}

/* Starts the BGM only when no stream is active and the sequence message has GAMEMES_STAT_FXPLAY,
 * then updates actors and character voice pan. */
void fn_1_55C(s16 mode, s16 frameNo)
{
    lbl_1_bss_0.bgmHandle = fn_1_A0(lbl_1_bss_0.bgmHandle, M633_GAMEPLAY_BGM_ID);
    MgActorExec();
    fn_1_140();
}

/* Starts the round, updates players, actors, and the arena, then requests the next mode when no
 * outside-group players remain or the timer ends. */
void fn_1_690(s16 mode, s16 frameNo)
{
    s16 model;
    Point3d spawnPosition;
    MGPLAYER *player;
    s32 playerNo;

    if (frameNo == 0) {
        fn_1_7E70(1);
        fn_1_70A8(0);
        playerNo = 0;
        while (playerNo < 4) {
            lbl_1_bss_0.playerRemoved[playerNo] = 0;
            if ((s32) lbl_1_bss_0.outsideGroupZero[playerNo] == 0) {
                (lbl_1_bss_0.playerObjects[playerNo])->objFunc = fn_1_28A4;
            } else {
                model = (s16) lbl_1_bss_0.players[playerNo]->actor->mdlId;
                Hu3DModelPosGet(model, &spawnPosition);
                MgPlayerSpawn(lbl_1_bss_0.players[playerNo], &spawnPosition);
                (lbl_1_bss_0.playerObjects[playerNo])->objFunc = fn_1_34AC;
                CharMotionShiftSet((lbl_1_bss_0.players[playerNo])->charNo,
                                   *((lbl_1_bss_0.players[playerNo])->omObj)->mtnId, 0.0f, 6.0f,
                                   0U);
            }
            playerNo += 1;
        }
        MgTimerParamSet(lbl_1_bss_0.timer, 1800, 0, 0);
        MgTimerModeOnSet(lbl_1_bss_0.timer, 1);
        MgTimerRecordDispOn(lbl_1_bss_0.timer);
        lbl_1_bss_0.segmentReflectionSoundPending = 0;
        lbl_1_bss_0.aiTurning = 0;
    }
    lbl_1_bss_0.preventArenaRestart = 0;
    playerNo = 0;
    while (playerNo < 4) {
        fn_1_3830(playerNo);
        playerNo += 1;
    }
    if (lbl_1_bss_0.segmentReflectionSoundPending != 0) {
        HuAudFXPlay(M633_SEGMENT_WALL_REFLECTION_SE_ID);
    }
    MgActorExec();
    fn_1_140();
    fn_1_6DE8();
    lbl_1_bss_0.segmentUpdateCountdown -= 1;
    lbl_1_bss_0.segmentReflectionSoundPending = 0;
    if ((lbl_1_bss_0.activePlayerCount == 0) || (MgTimerDoneCheck(lbl_1_bss_0.timer) != 0)) {
        playerNo = 0;
        while (playerNo < 4) {
            player = lbl_1_bss_0.players[playerNo];
            MgPlayerDespawn(player);
            if ((s32) lbl_1_bss_0.outsideGroupZero[playerNo] == 0) {
                CharMotionShiftSet(player->charNo, *player->omObj->mtnId, 0.0f, 6.0f, 0U);
                (lbl_1_bss_0.playerObjects[playerNo])->objFunc = fn_1_26B4;
            } else if ((s32) lbl_1_bss_0.playerRemoved[playerNo] == 0) {
                CharMotionShiftSet(player->charNo, *player->omObj->mtnId, 0.0f, 6.0f, 0U);
            }
            playerNo += 1;
        }
        MgSeqModeNext();
    }
}

/* On entry, fades the BGM and stops rotation sound; hides the countdown display while the timer
 * has not reached its end, then clears or decrements the segment countdown. */
void fn_1_AB0(s16 mode, s16 frameNo)
{

    lbl_1_bss_0.preventArenaRestart = 1;
    MgActorExec();
    fn_1_140();
    if (frameNo == 0) {
        fn_1_104(lbl_1_bss_0.bgmHandle);
        if (lbl_1_bss_0.rotationSoundHandle != -1) {
            HuAudFXStop(lbl_1_bss_0.rotationSoundHandle);
            lbl_1_bss_0.rotationSoundHandle = -1;
        }
    }
    if (MgTimerDoneCheck(lbl_1_bss_0.timer) == 0) {
        MgTimerRecordDispOff(lbl_1_bss_0.timer);
    }
    if (lbl_1_bss_0.segmentUpdateCountdown > 0) {
        lbl_1_bss_0.segmentUpdateCountdown = 0;
        return;
    }
    lbl_1_bss_0.segmentUpdateCountdown -= 1;
}

/* On frame 0, places surviving outside-group characters; marks every outside-group slot as a
 * winner, including removed slots, or picks the first group-zero slot when none remain; assigns
 * winners 10 coins outside practice and requests the next mode. */
void fn_1_C78(s16 mode, s16 frameNo)
{
    s16 winners[4] = { -1, -1, -1, -1 };
    char *oneResult[1] = { lbl_1_data_28 };
    char *twoResults[2] = {
        lbl_1_data_41,
        lbl_1_data_5A,
    };
    char *threeResults[3] = {
        lbl_1_data_73,
        lbl_1_data_8C,
        lbl_1_data_A5,
    };
    s16 playerBonus[4] = { 0, 0, 0, 0 };
    char **results;
    Point3d pos;
    MGPLAYER *player;
    s16 coinBonus;
    s32 resultCount;
    s32 playerNo;

    if (frameNo == 0) {
        WipeCreate(2, 0, 60);
        for (playerNo = 0; playerNo < 60; playerNo++) {
            HuPrcVSleep();
        }

        switch (lbl_1_bss_0.activePlayerCount) {
        case 1:
            results = oneResult;
            break;
        case 2:
            results = twoResults;
            break;
        case 3:
            results = threeResults;
            break;
        }

        resultCount = 0;
        for (playerNo = 0; playerNo < 4; playerNo++) {
            player = lbl_1_bss_0.players[playerNo];
            if (lbl_1_bss_0.outsideGroupZero[playerNo] == 0) {
                (lbl_1_bss_0.playerObjects[playerNo])->objFunc = fn_1_26B4;
                Hu3DModelRotSet(lbl_1_bss_0.rotatingStageModelId, 0.0f, 0.0f, 0.0f);
                Hu3DModelRotSet(lbl_1_bss_0.secondRotatingModelId, 0.0f, 0.0f, 0.0f);
                Hu3DMotionSet(lbl_1_bss_0.rotatingStageModelId, lbl_1_bss_0.stageIdleMotionId);
            } else if (lbl_1_bss_0.playerRemoved[playerNo] == 0) {
                Hu3DModelObjPosGet(lbl_1_bss_0.placementModelId, results[resultCount], &pos);
                Hu3DModelPosSetV((s16)player->actor->mdlId, &pos);
                Hu3DModelAttrReset((s16)player->actor->mdlId, HU3D_ATTR_DISPOFF);
                CharMotionSet((s16)lbl_1_bss_0.characterNumbers[playerNo],
                    *((player)->omObj)->mtnId);
                Hu3DModelRotSet((s16)player->actor->mdlId, 0.0f, 0.0f, 0.0f);
                resultCount++;
            }
        }

        Hu3DModelAttrSet(lbl_1_bss_0.nozzleModelIds[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_0.nozzleModelIds[1], HU3D_ATTR_DISPOFF);
        fn_1_70A8(3);
        if (lbl_1_bss_0.activePlayerCount == 0) {
            fn_1_7E70(2);
        } else {
            fn_1_7E70(3);
        }

        WipeCreate(1, 5, 60);
        for (playerNo = 0; playerNo < 60; playerNo++) {
            HuPrcVSleep();
        }

        for (playerNo = 0; playerNo < 4; playerNo++) {
            lbl_1_bss_0.winnerFlags[playerNo] = 0;
        }

        if (lbl_1_bss_0.activePlayerCount == 0) {
            for (playerNo = 0; playerNo < 4; playerNo++) {
                if (lbl_1_bss_0.outsideGroupZero[playerNo] == 0) {
                    winners[0] = (s16)lbl_1_bss_0.characterNumbers[playerNo];
                    lbl_1_bss_0.winnerFlags[playerNo] = 1;
                    playerBonus[playerNo] = 10;
                    break;
                }
            }
        } else {
            resultCount = 0;
            for (playerNo = 0; playerNo < 4; playerNo++) {
                if (lbl_1_bss_0.outsideGroupZero[playerNo] != 0) {
                    winners[resultCount++] = (s16)lbl_1_bss_0.characterNumbers[playerNo];
                    lbl_1_bss_0.winnerFlags[playerNo] = 1;
                    playerBonus[playerNo] = 10;
                }
            }
        }

        MgSeqWinnerSet(winners[0], winners[1], winners[2], winners[3]);
        for (playerNo = 0; playerNo < 4; playerNo++) {
            coinBonus = playerBonus[playerNo];
            if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
                GwPlayer[playerNo].mgCoinBonus = coinBonus;
            }
        }
        MgSeqModeNext();
    }
}

/* On frame 0, starts the winner or non-winner motion for each player not marked removed. */
void fn_1_118C(s16 mode, s16 frameNo)
{
    s32 playerNo;
    MGPLAYER *player;
    s32 motion;

    if (frameNo == 0) {
        for (playerNo = 0; playerNo < 4; playerNo++) {
            player = lbl_1_bss_0.players[playerNo];
            if (lbl_1_bss_0.playerRemoved[playerNo] == 0) {
                if (lbl_1_bss_0.winnerFlags[playerNo] == 1) {
                    motion = 5;
                } else {
                    motion = 6;
                }
                CharMotionShiftSet((s16)lbl_1_bss_0.characterNumbers[playerNo],
                    player->omObj->mtnId[motion], 0.0f, 6.0f, 0);
            }
        }
    }
}

void fn_1_1284(s16 mode, s16 frameNo)
{

}

void fn_1_1288(s16 mode, s16 frameNo)
{

}
