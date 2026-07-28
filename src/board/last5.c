#include "game/board/coin.h"
#include "game/board/audio.h"
#include "game/board/camera.h"
#include "game/board/guide.h"
#include "game/board/main.h"
#include "game/board/masu.h"
#include "game/board/player.h"
#include "game/board/status.h"
#include "game/board/window.h"

#include "game/charman.h"
#include "game/data.h"
#include "game/frand.h"
#include "game/process.h"

#define LAST5_COIN_NUM 40

typedef struct Last5CoinWork_s {
    s16 delay;
    float velocity;
} LAST5COINWORK;

typedef struct Last5RouletteWork_s {
    u8 unk0F : 1;
    u8 killF : 1;
    u8 unk2F : 1;
    u8 unk3F : 1;
    u8 rouletteF : 1;
    u8 diceHitF : 1;
    u8 diceF : 1;
    u8 unk7F : 1;
    s16 time;
    s16 unk4;
    s16 unk6;
    s16 result;
    s16 chanceNum;
    s16 chanceNumCur;
} LAST5ROULETTEWORK;

extern int mbCoinAddProcExec(int playerNo, int coinNum, BOOL dispF, BOOL fastF);
extern int mbDiceProcExec(int playerNo, int diceType, s8 *valueTbl,
    int *tutorialVal, BOOL padWinF, BOOL waitF, HuVecF *pos, int color);
extern void mbDiceMotHookSet(int playerNo, void (*hook)(int));
extern BOOL mbDiceKillCheck(int playerNo);
extern void mbDiceObjHit(int playerNo);
extern void mbSNpcDispSet(BOOL dispF);
extern void mbWipeFadeIn(void);
extern void mbWipeFadeOut(void);

static OMOBJ *last5RouletteOMObj;

static void ev_Last5SDiceMotHook(int playerNo);
static OMOBJ *Last5RouletteCreate(int masuId);
static void Last5RouletteKill(OMOBJ *obj);
static void Last5PlayerOrderGet(int *playerOrder, int playerNum);
static void ev_Last5Dice(int playerNo);
static void ev_Last5Coin40(int playerNo, OMOBJ *guideObj);
static void ev_Last5CapsuleAdd5(int playerNo, OMOBJ *rouletteObj,
    OMOBJ *guideObj);
static void ev_Last5Koopa(int playerNo, OMOBJ *rouletteObj, int modelId);

static int koopaMotTbl[4] = {
    0x000E0001,
    0x000E0003,
    0x000E0004,
    0x000E0005,
};

static HuVec2f statusPosTbl[GW_PLAYER_MAX][2] = {
    { { -98.0f, 72.0f }, { 114.0f, 72.0f } },
    { { -98.0f, 152.0f }, { 114.0f, 152.0f } },
    { { -98.0f, 232.0f }, { 114.0f, 232.0f } },
    { { -98.0f, 312.0f }, { 114.0f, 312.0f } },
};

static HuVec2f statusTeamPosTbl[2][2] = {
    { { -124.0f, 80.0f }, { 140.0f, 80.0f } },
    { { -124.0f, 160.0f }, { 140.0f, 160.0f } },
};

static int rankMesTbl[4] = {
    0x002E0002,
    0x002E0004,
    0x002E0006,
    0x002E0008,
};

static int teamRankMesTbl[4] = {
    0x002E0032,
    0x002E0034,
    0x002E0006,
    0x002E0008,
};

static int last5EffMesTbl[4] = {
    0x002E0010,
    0x002E0012,
    0x002E0014,
    0x002E0016,
};

static int last5EffMes2Tbl[4] = {
    0x002E001A,
    0x002E001C,
    0x002E0020,
    0x002E0022,
};

static s8 guideMotTbl[7] = {
    12,
    21,
    7,
    11,
    8,
    6,
    -1,
};

void mbev_Last5(void)
{
    int playerOrder[GW_PLAYER_MAX];
    int teamOrder[2];
    int teamPlayers[2];
    HuVecF masuPos;
    HuVecF statusPos;
    HuVecF statusTarget;
    int masuId;
    int playerNo;
    int messageOffset;
    int result;
    int koopaModelId = -1;
    int i;
    int j;
    s16 winId;
    OMOBJ *rouletteObj;
    OMOBJ *guideObj;
    MBMODELID guideModelId;

    masuId = mbMasuFind_AttrIdGet(-1, 0x8000);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerDispSet(i, FALSE);
    }
    mbSNpcDispSet(FALSE);
    last5RouletteOMObj = rouletteObj = Last5RouletteCreate(masuId);

    mbMasuPosGet(masuId, &masuPos);
    masuPos.y += 100.0f;
    mbCameraFocusMasuSet(masuId);
    mbCameraOffsetSet(0.0f, 100.0f, 0.0f);
    mbCameraRotSet(-20.0f, 0.0f, 0.0f);
    mbCameraZoomSet(mbCameraPlayerViewZoomGet(0) - 200.0f);
    mbCameraMoveOnSet(FALSE);
    mbMusPlay(0, 0x21, 0x7F, 0);
    HuDataDirClose(DATANUM(DATA_blast5, 0));
    mbWipeFadeIn();

    mbMasuPosGet(masuId, &masuPos);
    masuPos.x += 200.0f;
    masuPos.z += 100.0f;
    guideObj = mbGuideCreateFlag(&masuPos, guideMotTbl, FALSE, TRUE, TRUE);
    mbGuideMotionNextSet(guideObj, 1);
    guideModelId = mbGuideModelGet(guideObj);
    messageOffset = GwSystem.curTime ? 1 : 0;

    mbGuideMotionShiftSet(guideObj, 12, TRUE);
    mbAudGuidePlay(0x3B6);
    winId = mbWinCreate(2, 0x002E0000 + messageOffset,
        mbGuideSpeakerNoGet());
    mbWinPlayerDisable(winId, -1);
    mbWinWait(winId);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbStatusCapsuleDispSet(i, FALSE);
    }

    if (!GwSystem.tagF) {
        Last5PlayerOrderGet(playerOrder, GW_PLAYER_MAX);
        playerNo = playerOrder[GW_PLAYER_MAX - 1];
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            mbGuideMotionShiftSet(guideObj, 12, TRUE);
            winId = mbWinCreate(2,
                rankMesTbl[GwPlayer[playerOrder[i]].rank] + messageOffset,
                mbGuideSpeakerNoGet());
            mbWinInsertMesSet(winId, mbPlayerNameMesGet(playerOrder[i]), 0);
            mbWinPlayerDisable(winId, -1);
            mbStatusMoveSet(playerOrder[i],
                (HuVecF *)&statusPosTbl[i][0],
                (HuVecF *)&statusPosTbl[i][1], TRUE, 15);
            while (!mbStatusMoveCheck(playerOrder[i])) {
                HuPrcVSleep();
            }
            mbWinWait(winId);
        }
    } else {
        Last5PlayerOrderGet(teamOrder, 2);
        for (j = 0; j < 2; j++) {
            teamPlayers[j] = mbPlayerTeamFindPlayer(teamOrder[1], j);
        }
        playerNo = teamPlayers[mbRandMod(2)];
        if (GwPlayer[playerNo].comF) {
            playerNo = mbPlayerTeamFind(playerNo);
        }
        for (i = 0; i < 2; i++) {
            mbGuideMotionShiftSet(guideObj, 12, TRUE);
            winId = mbWinCreate(2,
                teamRankMesTbl[mbPlayerTeamRankGet(teamOrder[i])]
                    + messageOffset,
                mbGuideSpeakerNoGet());
            mbWinInsertMesSet(winId, mbPlayerTagNameMesGet(teamOrder[i]), 0);
            mbWinPlayerDisable(winId, -1);
            mbStatusNoMoveSet(teamOrder[i],
                (HuVecF *)&statusTeamPosTbl[i][0],
                (HuVecF *)&statusTeamPosTbl[i][1], TRUE, 15);
            while (!mbStatusMoveCheck(teamOrder[i])) {
                HuPrcVSleep();
            }
            mbWinWait(winId);
        }
    }

    mbGuideMotionShiftSet(guideObj, 12, TRUE);
    mbAudGuidePlay(0x3B8);
    winId = mbWinCreate(2, 0x002E000A + messageOffset,
        mbGuideSpeakerNoGet());
    mbWinPlayerDisable(winId, -1);
    mbWinWait(winId);

    if (!GwSystem.tagF) {
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            mbStatusPosGet(i, &statusPos);
            statusTarget = statusPos;
            statusTarget.x = statusPosTbl[0][0].x;
            mbStatusMoveSet(i, &statusPos, &statusTarget, TRUE, 15);
        }
    } else {
        for (i = 0; i < 2; i++) {
            mbStatusNoPosGet(i, &statusPos);
            statusTarget = statusPos;
            statusTarget.x = statusTeamPosTbl[0][0].x;
            mbStatusNoMoveSet(i, &statusPos, &statusTarget, TRUE, 15);
        }
    }
    while (!mbStatusOffCheckAll()) {
        HuPrcVSleep();
    }
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbStatusCapsuleDispSet(i, TRUE);
    }

    mbGuideMotionShiftSet(guideObj, 12, TRUE);
    winId = mbWinCreate(2, 0x002E000C + messageOffset,
        mbGuideSpeakerNoGet());
    mbWinInsertMesSet(winId, mbPlayerNameMesGet(playerNo), 0);
    mbWinPlayerDisable(winId, playerNo);
    mbWinWait(winId);

    mbMasuPosGet(masuId, &masuPos);
    masuPos.x -= 200.0f;
    masuPos.z += 100.0f;
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
    mbPlayerRotSet(playerNo, 0.0f, 0.0f, 0.0f);
    mbPlayerMotionSet(playerNo, 6, HU3D_MOTATTR_LOOP);
    HuPrcVSleep();
    mbPlayerDispSet(playerNo, TRUE);
    for (i = 0; i <= 30; i++) {
        HuVecF playerPos = masuPos;
        float weight = (float)(30 - i) / 30.0f;

        playerPos.y += 100.0f * (6.0f * mbSinDeg(80.0f * weight));
        mbPlayerPosSetV(playerNo, &playerPos);
        HuPrcVSleep();
    }
    omVibrate(playerNo, 20, 7, 3);
    for (i = 0; i < 60; i++) {
        HuPrcVSleep();
    }
    mbPlayerMotIdleSet(playerNo);

    mbGuideMotionShiftSet(guideObj, 12, TRUE);
    mbAudGuidePlay(0x3B8);
    winId = mbWinCreate(2, 0x002E000E + messageOffset,
        mbGuideSpeakerNoGet());
    mbWinPlayerDisable(winId, playerNo);
    mbWinWait(winId);
    mbGuideMotionShiftSet(guideObj, 21, TRUE);
    while (!mbGuideMotionCheck(guideObj)) {
        HuPrcVSleep();
    }

    ev_Last5Dice(playerNo);
    result = omObjGetWork(rouletteObj, LAST5ROULETTEWORK)->result;
    if (result == 3) {
        mbGuideMotionShiftSet(guideObj, 8, TRUE);
        mbAudGuidePlay(0x3B8);
        mbGuideMotionNextSet(guideObj, 11);
        winId = mbWinCreate(2, last5EffMesTbl[result] + messageOffset,
            mbGuideSpeakerNoGet());
        mbWinPlayerDisable(winId, playerNo);
        mbWinWait(winId);
        while (!mbGuideMotionCheck(guideObj)) {
            HuPrcVSleep();
        }
        winId = mbWinCreate(2, 0x002E0018 + messageOffset,
            mbGuideSpeakerNoGet());
        mbWinPlayerDisable(winId, playerNo);
        mbObjPosGet(guideModelId, &masuPos);
        mbPlayerRotateStart(playerNo, 90, 15);
        mbGuideEnd(guideObj);
        guideObj = NULL;
        mbWinWait(winId);
        HuPrcSleep(2);
        koopaModelId = mbObjCreate(0x000E0000, koopaMotTbl, TRUE);
        mbObjLayerSet(koopaModelId, 3);
        mbObjDispSet(koopaModelId, FALSE);
        mbObjPosSetV(koopaModelId, &masuPos);
        ev_Last5Koopa(playerNo, rouletteObj, koopaModelId);
        winId = mbWinCreate(2, 0x002E002E, 13);
        mbWinPlayerDisable(winId, -1);
        mbWinWait(winId);
        mbObjMotionShiftSet(koopaModelId, 2, 0.0f, 12.0f,
            HU3D_MOTATTR_NONE);
        mbAudFXPlay(0x3D0);
        winId = mbWinCreate(2, 0x002E0031, 13);
        mbWinPlayerDisable(winId, -1);
        mbWinWait(winId);
    } else {
        mbGuideMotionShiftSet(guideObj, 12, TRUE);
        winId = mbWinCreate(2, last5EffMesTbl[result] + messageOffset,
            mbGuideSpeakerNoGet());
        mbWinPlayerDisable(winId, playerNo);
        mbWinWait(winId);
        mbGuideMotionShiftSet(guideObj, 12, TRUE);
        mbAudGuidePlay(0x3B8);
        winId = mbWinCreate(2, last5EffMes2Tbl[result] + messageOffset,
            mbGuideSpeakerNoGet());
        mbWinPlayerDisable(winId, playerNo);
        if (result == 2) {
            mbWinInsertMesSet(winId, mbPlayerNameMesGet(playerNo), 0);
        }
        mbWinWait(winId);
        if (result == 1) {
            ev_Last5Coin40(playerNo, guideObj);
        } else if (result == 0) {
            mbGuideMotionShiftSet(guideObj, 6, TRUE);
            mbGuideMotionStop(guideObj);
            HuPrcSleep(30);
            mbAudGuidePlay(0x3B6);
            winId = mbWinCreate(2, 0x002E001E + messageOffset,
                mbGuideSpeakerNoGet());
            mbWinPlayerDisable(winId, playerNo);
            mbWinWait(winId);
            GwSystem.last5Effect = 1;
        } else if (result == 2) {
            ev_Last5CapsuleAdd5(playerNo, rouletteObj, guideObj);
        }
        mbGuideMotionShiftSet(guideObj, 12, TRUE);
        winId = mbWinCreate(2, 0x002E002A + messageOffset,
            mbGuideSpeakerNoGet());
        mbWinPlayerDisable(winId, -1);
        mbWinWait(winId);
        mbGuideMotionSet(guideObj, 12, TRUE);
        mbAudGuidePlay(0x3B8);
        winId = mbWinCreate(2, 0x002E002C + messageOffset,
            mbGuideSpeakerNoGet());
        mbWinPlayerDisable(winId, -1);
        mbWinWait(winId);
        mbGuideMotionSet(guideObj, 7, TRUE);
        mbAudGuidePlay(0x3B6);
        winId = mbWinCreate(2, 0x002E002F + messageOffset,
            mbGuideSpeakerNoGet());
        mbWinPlayerDisable(winId, -1);
        mbWinWait(winId);
    }

    mbMusFadeOutSpeed(0, 1000);
    mbWipeFadeOut();
    while (mbMusCheck(0)) {
        HuPrcVSleep();
    }
    mbPlayerPosReset(playerNo);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerDispSet(i, TRUE);
    }
    mbSNpcDispSet(TRUE);
    Last5RouletteKill(rouletteObj);
    last5RouletteOMObj = NULL;
    if (guideObj) {
        mbGuideKill(guideObj);
    }
    if (koopaModelId >= 0) {
        mbObjKill(koopaModelId);
    }
    HuPrcVSleep();
}

static void Last5RouletteKill(OMOBJ *obj)
{
    LAST5ROULETTEWORK *work = omObjGetWork(obj, LAST5ROULETTEWORK);

    work->killF = TRUE;
}

static void Last5PlayerOrderGet(int *playerOrder, int playerNum)
{
    int rank[GW_PLAYER_MAX];
    int i;
    int j;
    int swap;

    for (i = 0; i < playerNum; i++) {
        playerOrder[i] = i;
    }
    for (i = 0; i < playerNum - 1; i++) {
        j = i + mbRandMod(playerNum - i);
        swap = playerOrder[j];
        playerOrder[j] = playerOrder[i];
        playerOrder[i] = swap;
    }

    if (!GWTeamFGet()) {
        if (GwPlayer[playerOrder[playerNum - 1]].comF) {
            for (i = playerNum - 1; i >= 0; i--) {
                if (!GwPlayer[playerOrder[i]].comF) {
                    swap = playerOrder[playerNum - 1];
                    playerOrder[playerNum - 1] = playerOrder[i];
                    playerOrder[i] = swap;
                    break;
                }
            }
        }
    } else if (GwPlayer[mbPlayerTeamFindPlayer(playerOrder[playerNum - 1], 0)].comF
        && GwPlayer[mbPlayerTeamFindPlayer(playerOrder[playerNum - 1], 1)].comF) {
        for (i = playerNum - 1; i >= 0; i--) {
            if (!GwPlayer[mbPlayerTeamFindPlayer(playerOrder[i], 0)].comF
                || !GwPlayer[mbPlayerTeamFindPlayer(playerOrder[i], 1)].comF) {
                swap = playerOrder[playerNum - 1];
                playerOrder[playerNum - 1] = playerOrder[i];
                playerOrder[i] = swap;
                break;
            }
        }
    }

    for (i = 0; i < playerNum; i++) {
        if (playerNum == GW_PLAYER_MAX) {
            rank[i] = GwPlayer[playerOrder[i]].rank;
        } else {
            rank[i] = mbPlayerTeamRankGet(playerOrder[i]);
        }
    }
    for (i = 0; i < playerNum - 1; i++) {
        for (j = i + 1; j < playerNum; j++) {
            if (rank[i] > rank[j]) {
                swap = rank[j];
                rank[j] = rank[i];
                rank[i] = swap;
                swap = playerOrder[j];
                playerOrder[j] = playerOrder[i];
                playerOrder[i] = swap;
            }
        }
    }
}

static void ev_Last5Dice(int playerNo)
{
    OMOBJ *obj = last5RouletteOMObj;

    mbObjHookReset(obj->mdlId[0]);
    mbObjDispSet(obj->mdlId[1], FALSE);
    mbObjDispSet(obj->mdlId[2], TRUE);
    mbObjHookSet(obj->mdlId[0], "target", obj->mdlId[2]);
    omObjGetWork(obj, LAST5ROULETTEWORK)->rouletteF = TRUE;
    omObjGetWork(obj, LAST5ROULETTEWORK)->diceF = TRUE;
    mbDiceProcExec(playerNo, 6, NULL, NULL, TRUE, FALSE, NULL, 0);
    mbDiceMotHookSet(playerNo, ev_Last5SDiceMotHook);
    while (!mbDiceKillCheck(playerNo)) {
        HuPrcVSleep();
    }
    mbAudFXPlay(0x3FB);
}

static void ev_Last5SDiceMotHook(int playerNo)
{
    int i;

    mbPlayerMotionSet(playerNo, 11, HU3D_MOTATTR_NONE);
    i = 0;
    do {
        if (i++ == 27) {
            omObjGetWork(last5RouletteOMObj, LAST5ROULETTEWORK)->diceHitF = TRUE;
            mbDiceObjHit(playerNo);
        }
        HuPrcVSleep();
    } while (!mbPlayerMotionEndCheck(playerNo));
    mbPlayerMotIdleSet(playerNo);
}

static void ev_Last5Coin40(int playerNo, OMOBJ *guideObj)
{
    HuVecF playerPos;
    MBCOINOBJ *coinObj;
    LAST5COINWORK *coinWork;
    s16 coinObjId[LAST5_COIN_NUM];
    int activeNum;
    int coinNum = 1;
    int i;

    mbGuideMotionShiftSet(guideObj, 6, TRUE);
    mbGuideMotionStop(guideObj);
    HuPrcSleep(48);
    coinNum = LAST5_COIN_NUM;
    mbPlayerPosGet(playerNo, &playerPos);
    for (i = 0; i < coinNum; i++) {
        coinObjId[i] = mbCoinCreate2();
        mbCoinObjDispSet(coinObjId[i], FALSE);
        coinObj = mbCoinObjGet(coinObjId[i]);
        coinObj->pos.x = playerPos.x + (60.000004f * (frandf() - 0.5f));
        coinObj->pos.y = 800.0f + (playerPos.y + (50.0f * frandf()));
        coinObj->pos.z = playerPos.z + (60.000004f * (frandf() - 0.5f));
        coinObj->rot.x = 40.0f * (frandf() - 0.5f);
        coinObj->rot.y = 360.0f * frandf();
        coinObj->scale.x = coinObj->scale.y = coinObj->scale.z = 0.7f;
        coinWork = (LAST5COINWORK *)coinObj->work;
        coinWork->delay = (float)(i * 30) / coinNum;
        coinWork->velocity = -13.333334f;
    }

    activeNum = coinNum;
    while (activeNum != 0) {
        activeNum = 0;
        for (i = 0; i < coinNum; i++) {
            if (coinObjId[i] == 0) {
                continue;
            }
            activeNum++;
            coinObj = mbCoinObjGet(coinObjId[i]);
            coinWork = (LAST5COINWORK *)coinObj->work;
            if (coinWork->delay != 0) {
                coinWork->delay--;
                continue;
            }
            mbCoinObjDispSet(coinObjId[i], TRUE);
            coinWork->velocity += -0.5444445f;
            coinObj->pos.y += coinWork->velocity;
            if (coinObj->pos.y < 100.0f + playerPos.y) {
                mbCoinObjKill(coinObjId[i]);
                coinObjId[i] = 0;
            }
        }
        HuPrcVSleep();
    }

    mbGuideMotionShiftSet(guideObj, 1, TRUE);
    mbPlayerWinLoseVoicePlay(playerNo, 12, CHARVOICEID(6));
    mbPlayerMotionShiftSet(playerNo, 12, 0.0f, 12.0f, HU3D_MOTATTR_NONE);
    mbCoinAddProcExec(playerNo, coinNum, TRUE, TRUE);
    mbPlayerMotionEndWait(playerNo);
    mbPlayerMotIdleSet(playerNo);
}
