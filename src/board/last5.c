#include "dolphin/math.h"

#include "game/board/audio.h"
#include "game/board/camera.h"
#include "game/board/capsule.h"
#include "game/board/coin.h"
#include "game/board/main.h"
#include "game/board/masu.h"
#include "game/board/object_data.h"
#include "game/board/player.h"
#include "game/board/status.h"
#include "game/board/window.h"

#include "game/charman.h"
#include "game/data.h"
#include "game/frand.h"
#include "game/memory.h"
#include "game/process.h"

#include "string.h"

OMOBJ *mbGuideCreateFlag(HuVecF *pos, s8 *motTbl, BOOL screenF,
    BOOL altMtxF, BOOL layerF);
void mbGuideKill(OMOBJ *obj);
void mbGuideEnd(OMOBJ *obj, BOOL endF);
MBMODELID mbGuideModelGet(OMOBJ *obj);
void mbGuideMotionNextSet(OMOBJ *obj, s16 motNo);
void mbGuideMotionSet(OMOBJ *obj, s16 motNo, BOOL shiftF);
void mbGuideMotionShiftSet(OMOBJ *obj, s16 motNo, BOOL shiftF);
void mbGuideMotionStop(OMOBJ *obj);
BOOL mbGuideMotionCheck(OMOBJ *obj);
int mbGuideSpeakerNoGet(void);
float mbSinDeg(float deg);

static inline void Last5VecCopy(register HuVecF *src,
    register HuVecF *dst)
{
    register __vec2x32float__ xy;
    register float z;

    asm {
        psq_l xy, 0(src), 0, 0
        lfs z, 8(src)
        psq_st xy, 0(dst), 0, 0
        stfs z, 8(dst)
    }
}

MBMODELID mbObjCreate(int dataNum, int *motData, BOOL link);
BOOL mbObjMotionEndCheck(int modelId);
void mbObjMotionSet(int modelId, int motNo, u32 attr);
void mbObjMotionShiftSet(int modelId, int motNo, float start, float end,
    BOOL loopF);
void mbObjMotionTimeSet(int modelId, float time);
void mbObjMotionSpeedSet(int modelId, float speed);
void mbObjPosSetV(int modelId, HuVecF *pos);
void mbObjPosSet(int modelId, float x, float y, float z);
void mbObjPosGet(int modelId, HuVecF *pos);
void mbObjRotSet(int modelId, float rotX, float rotY, float rotZ);
void mbObjScaleSet(int modelId, float x, float y, float z);
void mbObjLayerSet(int modelId, int layer);
void mbObjAttrSet(int modelId, u32 attr);
void mbObjDispSet(int modelId, BOOL dispF);
void mbObjKill(int modelId);
void mbObjHookSet(int modelId, char *objName, int hookMdlId);
void mbObjHookReset(int modelId);

#define LAST5_COIN_NUM 40
#define LAST5_COIN_SCATTER_RANGE (100.0f * 0.6f)
#define LAST5_COIN_START_VELOCITY (-13.333334f)
#define LAST5_COIN_GRAVITY (-0.5444445f)
#define LAST5_MESS_DIRECTORY 46
#define LAST5_MESS_ID(file) \
    ((u32)((LAST5_MESS_DIRECTORY << 16) | (file)))

#define LAST5_MUSIC 33
#define LAST5_GUIDE_VOICE_INTRO 950
#define LAST5_GUIDE_VOICE_EXPLAIN 952
#define LAST5_KOOPA_EXIT_SFX 976
#define LAST5_DICE_RESULT_SFX 1019
#define LAST5_KOOPA_APPEAR_SFX 1119
#define LAST5_KOOPA_TALK_SFX 971
#define LAST5_KOOPA_REACT_SFX 975

#define LAST5_ROULETTE_PRIORITY 256
#define LAST5_ROULETTE_MODEL_NUM 3
#define LAST5_ROULETTE_RESULT_NUM 4
#define LAST5_ROULETTE_CHANCE_TBL_NUM 5
#define LAST5_ROULETTE_MODEL_LAYER 3
#define LAST5_ROULETTE_SHUFFLE_NUM 20
#define LAST5_ROULETTE_UPDATE_MASK 3
#define LAST5_ROULETTE_POS_OFFSET 50.0f
#define LAST5_ROULETTE_SCALE 2.0f
#define LAST5_ROULETTE_MOTION_TIME_OFFSET 0.5f

#define LAST5_CAPSULE_ADD_NUM 5
#define LAST5_CAPSULE_LIST_MAX 33
#define LAST5_CAPSULE_MASU_LIST_NUM 3
#define LAST5_CAPSULE_MASU_LIST_MAX 256

#define LAST5_KOOPA_RISE_TIME 18
#define LAST5_KOOPA_RISE_TRIGGER 8
#define LAST5_KOOPA_RISE_ANGLE 60.0f
#define LAST5_KOOPA_RISE_HEIGHT 100.0f
#define LAST5_KOOPA_RISE_SCALE 5.0f
#define LAST5_KOOPA_MODEL_ROT_Y -5.0f
#define LAST5_KOOPA_MODEL_MOTION_TIME 45.0f

#define LAST5_KOOPA_DATA_MODEL DATANUM(DATA_capsulechar1, 0)
#define LAST5_KOOPA_DATA_MOTION_IDLE DATANUM(DATA_capsulechar1, 1)
#define LAST5_KOOPA_DATA_MOTION_APPEAR DATANUM(DATA_capsulechar1, 3)
#define LAST5_KOOPA_DATA_MOTION_TALK DATANUM(DATA_capsulechar1, 4)
#define LAST5_KOOPA_DATA_MOTION_EXIT DATANUM(DATA_capsulechar1, 5)

#define LAST5_MESS_INTRO LAST5_MESS_ID(0)
#define LAST5_MESS_RANK_FIRST LAST5_MESS_ID(2)
#define LAST5_MESS_RANK_SECOND LAST5_MESS_ID(4)
#define LAST5_MESS_RANK_THIRD LAST5_MESS_ID(6)
#define LAST5_MESS_RANK_FOURTH LAST5_MESS_ID(8)
#define LAST5_MESS_ROULETTE_INTRO LAST5_MESS_ID(10)
#define LAST5_MESS_PLAYER_CALL LAST5_MESS_ID(12)
#define LAST5_MESS_DICE_PROMPT LAST5_MESS_ID(14)
#define LAST5_MESS_EFFECT_NO_RED_SPACES LAST5_MESS_ID(16)
#define LAST5_MESS_EFFECT_COINS LAST5_MESS_ID(18)
#define LAST5_MESS_EFFECT_CAPSULES LAST5_MESS_ID(20)
#define LAST5_MESS_EFFECT_KOOPA LAST5_MESS_ID(22)
#define LAST5_MESS_KOOPA_REVEAL LAST5_MESS_ID(24)
#define LAST5_MESS_EFFECT_NO_RED_SPACES_EXPLAIN LAST5_MESS_ID(26)
#define LAST5_MESS_EFFECT_COINS_EXPLAIN LAST5_MESS_ID(28)
#define LAST5_MESS_NO_RED_SPACES_CONFIRM LAST5_MESS_ID(30)
#define LAST5_MESS_EFFECT_CAPSULES_EXPLAIN LAST5_MESS_ID(32)
#define LAST5_MESS_EFFECT_KOOPA_EXPLAIN LAST5_MESS_ID(34)
#define LAST5_MESS_KOOPA_EQUALIZE LAST5_MESS_ID(36)
#define LAST5_MESS_EFFECT_WRAPUP LAST5_MESS_ID(42)
#define LAST5_MESS_EFFECT_RULES LAST5_MESS_ID(44)
#define LAST5_MESS_KOOPA_INTRO LAST5_MESS_ID(46)
#define LAST5_MESS_EFFECT_START LAST5_MESS_ID(47)
#define LAST5_MESS_KOOPA_EXIT LAST5_MESS_ID(49)
#define LAST5_MESS_TEAM_RANK_FIRST LAST5_MESS_ID(50)
#define LAST5_MESS_TEAM_RANK_SECOND LAST5_MESS_ID(52)

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

extern int mbDiceProcExec(int playerNo, int diceType, s8 *valueTbl,
    int *tutorialVal, BOOL padWinF, BOOL waitF, HuVecF *pos, int color);
extern void mbDiceMotHookSet(int playerNo, void (*hook)(int));
extern BOOL mbDiceKillCheck(int playerNo);
extern void mbDiceObjHit(int playerNo);
extern void mbSNpcDispSet(BOOL dispF);
extern void mbWipeFadeIn(void);
extern void mbWipeFadeOut(void);
extern void mbWipeDissolveFadeIn(void);
extern void mbWipeDissolveFadeOut(void);

static OMOBJ *last5RouletteOMObj;

static void ev_Last5SDiceMotHook(int playerNo);
static OMOBJ *Last5RouletteCreate(int masuId);
static void Last5RouletteKill(OMOBJ *obj);
static void Last5RouletteOMExec(OMOBJ *obj);
static void Last5PlayerOrderGet(int *playerOrder, int playerNum);
static void ev_Last5Dice(int playerNo);
static void ev_Last5Coin40(int playerNo, OMOBJ *guideObj);
static void ev_Last5CapsuleAdd5(int playerNo, OMOBJ *rouletteObj,
    OMOBJ *guideObj);
static void ev_Last5Koopa();

static inline void Last5RouletteResultSet(OMOBJ *obj)
{
    LAST5ROULETTEWORK *work = omObjGetWork(obj, LAST5ROULETTEWORK);
    s8 *table = obj->data;
    int i;
    int index;

    if (work->chanceNumCur <= 0) {
        work->chanceNumCur = work->chanceNum;
    }
    for (i = 0; i < LAST5_ROULETTE_SHUFFLE_NUM; i++) {
        index = mbRandMod(work->chanceNumCur);
        if (work->result != table[index]) {
            break;
        }
    }
    work->chanceNumCur--;
    i = table[index];
    table[index] = table[work->chanceNumCur];
    table[work->chanceNumCur] = i;
    work->result = i;
}

static int koopaMotTbl[4] = {
    LAST5_KOOPA_DATA_MOTION_IDLE,
    LAST5_KOOPA_DATA_MOTION_APPEAR,
    LAST5_KOOPA_DATA_MOTION_TALK,
    LAST5_KOOPA_DATA_MOTION_EXIT,
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
    LAST5_MESS_RANK_FIRST,
    LAST5_MESS_RANK_SECOND,
    LAST5_MESS_RANK_THIRD,
    LAST5_MESS_RANK_FOURTH,
};

static int teamRankMesTbl[4] = {
    LAST5_MESS_TEAM_RANK_FIRST,
    LAST5_MESS_TEAM_RANK_SECOND,
    LAST5_MESS_RANK_THIRD,
    LAST5_MESS_RANK_FOURTH,
};

static int last5EffMesTbl[4] = {
    LAST5_MESS_EFFECT_NO_RED_SPACES,
    LAST5_MESS_EFFECT_COINS,
    LAST5_MESS_EFFECT_CAPSULES,
    LAST5_MESS_EFFECT_KOOPA,
};

static int last5EffMes2Tbl[4] = {
    LAST5_MESS_EFFECT_NO_RED_SPACES_EXPLAIN,
    LAST5_MESS_EFFECT_COINS_EXPLAIN,
    LAST5_MESS_EFFECT_CAPSULES_EXPLAIN,
    LAST5_MESS_EFFECT_KOOPA_EXPLAIN,
};

static int rouletteFileTbl[LAST5_ROULETTE_MODEL_NUM] = {
    DATANUM(DATA_blast5, 0),
    DATANUM(DATA_blast5, 1),
    DATANUM(DATA_blast5, 2),
};

static int rouletteChanceTbl[LAST5_ROULETTE_CHANCE_TBL_NUM] = {
    3,
    3,
    3,
    1,
    0,
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
    HuVecF pos;
    HuVecF target;
    int playerOrder[GW_PLAYER_MAX];
    int teamPlayers[GW_PLAYER_MAX];
    int teamOrder[2];
    int masuId;
    OMOBJ *rouletteObj;
    int firstTeamNo = 0;
    int secondTeamNo = 1;
    int otherPlayerNo = 0;
    OMOBJ *guideObj;
    int playerNo = 3;
    int messageOffset;
    int i;
    int j;
    int koopaModelId = -1;
    int result;
    int winId;
    MBMODELID guideModelId;

    masuId = mbMasuFind_AttrIdGet(MASU_NULL, MASU_FLAG_START);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerDispSet(i, FALSE);
    }
    mbSNpcDispSet(FALSE);
    rouletteObj = last5RouletteOMObj = Last5RouletteCreate(masuId);

    mbMasuPosGet(masuId, &pos);
    pos.y += 100.0f;
    mbCameraFocusMasuSet(masuId);
    mbCameraOffsetSet(0.0f, 100.0f, 0.0f);
    mbCameraRotSet(-20.0f, 0.0f, 0.0f);
    mbCameraZoomSet(mbCameraPlayerViewZoomGet(0) - 200.0f);
    mbCameraMoveOnSet(FALSE);
    mbMusPlay(MB_MUS_CHAN_BG, LAST5_MUSIC, MSM_VOL_MAX, 0);
    HuDataDirClose(DATANUM(DATA_blast5, 0));
    mbWipeFadeIn();

    mbMasuPosGet(masuId, &pos);
    pos.x += 200.0f;
    pos.z += 100.0f;
    guideObj = mbGuideCreateFlag(&pos, guideMotTbl, FALSE, TRUE, TRUE);
    mbGuideMotionNextSet(guideObj, 1);
    guideModelId = mbGuideModelGet(guideObj);
    messageOffset = 0;
    if (GwSystem.curTime) {
        messageOffset++;
    }

    mbGuideMotionShiftSet(guideObj, 12, TRUE);
    mbAudGuidePlay(LAST5_GUIDE_VOICE_INTRO);
    winId = mbWinCreate(2, LAST5_MESS_INTRO + messageOffset,
        mbGuideSpeakerNoGet());
    mbWinPlayerDisable(winId, -1);
    mbWinWait(winId);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbStatusCapsuleDispSet(i, FALSE);
    }

    if (!GWTeamFGet()) {
        Last5PlayerOrderGet(playerOrder, GW_PLAYER_MAX);
        otherPlayerNo = playerOrder[0];
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
        firstTeamNo = teamOrder[0];
        secondTeamNo = teamOrder[1];
        for (i = 0; i < 2; i++) {
            for (j = 0; j < 2; j++) {
                teamPlayers[j] = mbPlayerTeamFindPlayer(teamOrder[i], j);
            }
            if (i == 1) {
                playerNo = teamPlayers[mbRandMod(2)];
                if (GwPlayer[playerNo].comF) {
                    playerNo = mbPlayerTeamFind(playerNo);
                }
            } else if (i == 0) {
                otherPlayerNo = teamPlayers[mbRandMod(2)];
            }
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
    mbAudGuidePlay(LAST5_GUIDE_VOICE_EXPLAIN);
    winId = mbWinCreate(2, LAST5_MESS_ROULETTE_INTRO + messageOffset,
        mbGuideSpeakerNoGet());
    mbWinPlayerDisable(winId, -1);
    mbWinWait(winId);

    if (!GWTeamFGet()) {
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            mbStatusPosGet(i, &pos);
            Last5VecCopy(&pos, &target);
            target.x = statusPosTbl[0][0].x;
            mbStatusMoveSet(i, &pos, &target, TRUE, 15);
        }
    } else {
        for (i = 0; i < 2; i++) {
            mbStatusNoPosGet(i, &pos);
            Last5VecCopy(&pos, &target);
            target.x = statusTeamPosTbl[0][0].x;
            mbStatusNoMoveSet(i, &pos, &target, TRUE, 15);
        }
    }
    while (!mbStatusOffCheckAll()) {
        HuPrcVSleep();
    }
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbStatusCapsuleDispSet(i, TRUE);
    }

    mbGuideMotionShiftSet(guideObj, 12, TRUE);
    winId = mbWinCreate(2, LAST5_MESS_PLAYER_CALL + messageOffset,
        mbGuideSpeakerNoGet());
    mbWinInsertMesSet(winId, mbPlayerNameMesGet(playerNo), 0);
    mbWinPlayerDisable(winId, playerNo);
    mbWinWait(winId);

    mbMasuPosGet(masuId, &pos);
    pos.x -= 200.0f;
    pos.z += 100.0f;
    {
        Last5VecCopy(&pos, &target);
        mbPlayerColSnapPlayerSet(playerNo, FALSE);
        mbPlayerRotSet(playerNo, 0.0f, 0.0f, 0.0f);
        mbPlayerMotionSet(playerNo, 6, HU3D_MOTATTR_LOOP);
        HuPrcVSleep();
        mbPlayerDispSet(playerNo, TRUE);
        j = 30;
        for (i = 0; i <= j; i++) {
            float arcOffset;

            arcOffset = 100.0f * (6.0f * mbSinDeg(
                80.0f * ((float)(j - i) / (float)j)));
            target.y = pos.y + arcOffset;
            mbPlayerPosSetV(playerNo, &target);
            i == j - 4;
            HuPrcVSleep();
        }
    }
    omVibrate(playerNo, 20, 7, 3);
    for (i = 0; i < 60; i++) {
        HuPrcVSleep();
    }
    mbPlayerMotIdleSet(playerNo);

    mbGuideMotionShiftSet(guideObj, 12, TRUE);
    mbAudGuidePlay(LAST5_GUIDE_VOICE_EXPLAIN);
    winId = mbWinCreate(2, LAST5_MESS_DICE_PROMPT + messageOffset,
        mbGuideSpeakerNoGet());
    mbWinPlayerDisable(winId, playerNo);
    mbWinWait(winId);
    mbGuideMotionShiftSet(guideObj, 21, TRUE);
    while (!mbGuideMotionCheck(guideObj)) {
        HuPrcVSleep();
    }

    ev_Last5Dice(playerNo);
    result = omObjGetWork(rouletteObj, LAST5ROULETTEWORK)->result;
    if (result != 3) {
        mbGuideMotionShiftSet(guideObj, 12, TRUE);
        winId = mbWinCreate(2, last5EffMesTbl[result] + messageOffset,
            mbGuideSpeakerNoGet());
        mbWinPlayerDisable(winId, playerNo);
        mbWinWait(winId);
        mbGuideMotionShiftSet(guideObj, 12, TRUE);
        mbAudGuidePlay(LAST5_GUIDE_VOICE_EXPLAIN);
        winId = mbWinCreate(2, last5EffMes2Tbl[result] + messageOffset,
            mbGuideSpeakerNoGet());
        mbWinPlayerDisable(winId, playerNo);
        if (result == 2) {
            mbWinInsertMesSet(winId, mbPlayerNameMesGet(playerNo), 0);
        }
        mbWinWait(winId);
        switch (result) {
        case 0:
            mbGuideMotionShiftSet(guideObj, 6, TRUE);
            mbGuideMotionStop(guideObj);
            HuPrcSleep(30);
            mbAudGuidePlay(LAST5_GUIDE_VOICE_INTRO);
            winId = mbWinCreate(2,
                LAST5_MESS_NO_RED_SPACES_CONFIRM + messageOffset,
                mbGuideSpeakerNoGet());
            mbWinPlayerDisable(winId, playerNo);
            mbWinWait(winId);
            GwSystem.last5Effect = 1;
            break;
        case 1:
            ev_Last5Coin40(playerNo, guideObj);
            break;
        case 2:
            ev_Last5CapsuleAdd5(playerNo, rouletteObj, guideObj);
            break;
        }
        mbGuideMotionShiftSet(guideObj, 12, TRUE);
        winId = mbWinCreate(2, LAST5_MESS_EFFECT_WRAPUP + messageOffset,
            mbGuideSpeakerNoGet());
        mbWinPlayerDisable(winId, -1);
        mbWinWait(winId);
        mbGuideMotionSet(guideObj, 12, TRUE);
        mbAudGuidePlay(LAST5_GUIDE_VOICE_EXPLAIN);
        winId = mbWinCreate(2, LAST5_MESS_EFFECT_RULES + messageOffset,
            mbGuideSpeakerNoGet());
        mbWinPlayerDisable(winId, -1);
        mbWinWait(winId);
        mbGuideMotionSet(guideObj, 7, TRUE);
        mbAudGuidePlay(LAST5_GUIDE_VOICE_INTRO);
        winId = mbWinCreate(2, LAST5_MESS_EFFECT_START + messageOffset,
            mbGuideSpeakerNoGet());
        mbWinPlayerDisable(winId, -1);
        mbWinWait(winId);
    } else {
        mbGuideMotionShiftSet(guideObj, 8, TRUE);
        mbAudGuidePlay(LAST5_GUIDE_VOICE_EXPLAIN);
        mbGuideMotionNextSet(guideObj, 11);
        winId = mbWinCreate(2, last5EffMesTbl[result] + messageOffset,
            mbGuideSpeakerNoGet());
        mbWinPlayerDisable(winId, playerNo);
        mbWinWait(winId);
        while (!mbGuideMotionCheck(guideObj)) {
            HuPrcVSleep();
        }
        winId = mbWinCreate(2, LAST5_MESS_KOOPA_REVEAL + messageOffset,
            mbGuideSpeakerNoGet());
        mbWinPlayerDisable(winId, playerNo);
        mbObjPosGet(guideModelId, &pos);
        mbPlayerRotateStart(playerNo, 90, 15);
        mbGuideEnd(guideObj, TRUE);
        guideObj = NULL;
        mbWinWait(winId);
        HuPrcSleep(2);
        koopaModelId = mbObjCreate(LAST5_KOOPA_DATA_MODEL, koopaMotTbl, TRUE);
        mbObjLayerSet(koopaModelId, 3);
        mbObjDispSet((MBMODELID)koopaModelId, FALSE);
        mbObjPosSetV((MBMODELID)koopaModelId, &pos);
        ev_Last5Koopa(playerNo, rouletteObj, koopaModelId);
        winId = mbWinCreate(2, LAST5_MESS_KOOPA_INTRO, 13);
        mbWinPlayerDisable(winId, -1);
        mbWinWait(winId);
        mbObjMotionShiftSet((MBMODELID)koopaModelId, 2, 0.0f, 12.0f,
            HU3D_MOTATTR_NONE);
        mbAudFXPlay(LAST5_KOOPA_EXIT_SFX);
        winId = mbWinCreate(2, LAST5_MESS_KOOPA_EXIT, 13);
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
        guideObj = NULL;
    }
    if (koopaModelId >= 0) {
        mbObjKill((MBMODELID)koopaModelId);
        koopaModelId = -1;
    }
    HuPrcVSleep();
}

static OMOBJ *Last5RouletteCreate(int masuId)
{
    LAST5ROULETTEWORK *work;
    OMOBJ *obj;
    HuVecF pos;
    int chanceNum;
    void *tableP;
    s8 *tableP2;
    s8 *table;
    int i;
    int num;
    int j;

    obj = omAddObjEx(mbObjMan, LAST5_ROULETTE_PRIORITY,
        LAST5_ROULETTE_MODEL_NUM, 0, OM_GRP_NONE, Last5RouletteOMExec);
    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    work = omObjGetWork(obj, LAST5ROULETTEWORK);
    memset(work, 0, sizeof(*work));

    for (i = 0; i < LAST5_ROULETTE_MODEL_NUM; i++) {
        int modelId;

        obj->mdlId[i] = modelId =
            mbObjCreate(rouletteFileTbl[i], NULL, FALSE);
        mbObjLayerSet((MBMODELID)modelId, LAST5_ROULETTE_MODEL_LAYER);
        mbObjAttrSet((MBMODELID)modelId, HU3D_MOTATTR_LOOP);
    }

    mbMasuPosGet(masuId, &pos);
    pos.z -= LAST5_ROULETTE_POS_OFFSET;
    mbObjPosSetV(obj->mdlId[0], &pos);
    mbObjScaleSet(obj->mdlId[0], LAST5_ROULETTE_SCALE,
        LAST5_ROULETTE_SCALE, LAST5_ROULETTE_SCALE);
    mbObjHookSet(obj->mdlId[0], "target", obj->mdlId[1]);
    mbObjDispSet(obj->mdlId[2], FALSE);
    mbObjMotionSpeedSet(obj->mdlId[2], 0.0f);

    work->result = 0;
    for (i = 0, num = 0; i < LAST5_ROULETTE_RESULT_NUM; i++) {
        num += rouletteChanceTbl[i];
    }
    work->chanceNum = work->chanceNumCur = num;
    chanceNum = work->chanceNum;
    tableP = HuMemDirectMallocNum(HEAP_HEAP, chanceNum, HU_MEMNUM_OVL);
    tableP2 = tableP;
    table = tableP2;
    obj->data = table;

    for (i = 0, j = 0; i < LAST5_ROULETTE_RESULT_NUM; i++) {
        for (num = 0; num < rouletteChanceTbl[i]; num++) {
            table[j++] = i;
        }
    }

    for (i = 0; i < LAST5_ROULETTE_SHUFFLE_NUM; i++) {
        Last5RouletteResultSet(obj);
    }
    return obj;
}

static void Last5RouletteKill(OMOBJ *obj)
{
    LAST5ROULETTEWORK *work = omObjGetWork(obj, LAST5ROULETTEWORK);

    work->killF = TRUE;
}

static void Last5RouletteOMExec(OMOBJ *obj)
{
    LAST5ROULETTEWORK *work = omObjGetWork(obj, LAST5ROULETTEWORK);
    int i;

    if (mbExitCheck() || work->killF) {
        for (i = 0; i < LAST5_ROULETTE_MODEL_NUM; i++) {
            if (obj->mdlId[i] >= 0) {
                mbObjKill(obj->mdlId[i]);
                obj->mdlId[i] = -1;
            }
        }
        if (obj->data) {
            void *data = obj->data;

            HuMemDirectFree(data);
            obj->data = NULL;
        }
        omDelObjEx(HuPrcCurrentGet(), obj);
        return;
    }

    if (work->rouletteF && !work->diceHitF) {
        work->time++;
        if (!(work->time & LAST5_ROULETTE_UPDATE_MASK)) {
            Last5RouletteResultSet(obj);
            mbObjMotionTimeSet(obj->mdlId[2],
                LAST5_ROULETTE_MOTION_TIME_OFFSET + work->result);
        }
    }
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
    mbAudFXPlay(LAST5_DICE_RESULT_SFX);
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
    int validNum = 0;
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
        coinObj->pos.x = playerPos.x
            + (LAST5_COIN_SCATTER_RANGE * (frandf() - 0.5f));
        coinObj->pos.y = 800.0f + (playerPos.y + (50.0f * frandf()));
        coinObj->pos.z = playerPos.z
            + (LAST5_COIN_SCATTER_RANGE * (frandf() - 0.5f));
        coinObj->rot.x = 40.0f * (frandf() - 0.5f);
        coinObj->rot.y = 360.0f * frandf();
        coinObj->scale.x = coinObj->scale.y = coinObj->scale.z = 0.7f;
        coinWork = (LAST5COINWORK *)coinObj->work;
        coinWork->delay = (float)(i * 30) / coinNum;
        coinWork->velocity = LAST5_COIN_START_VELOCITY;
    }

    activeNum = coinNum;
    while (activeNum != 0) {
        for (i = 0, activeNum = 0; i < coinNum; i++) {
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
            coinWork->velocity += LAST5_COIN_GRAVITY;
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

static void ev_Last5CapsuleAdd5(int playerNo, OMOBJ *rouletteObj,
    OMOBJ *guideObj)
{
    int capsuleList[LAST5_CAPSULE_LIST_MAX];
    HuVecF pos;
    HuVecF startPos;
    s16 *masuList[LAST5_CAPSULE_MASU_LIST_NUM];
    int masuNum[LAST5_CAPSULE_MASU_LIST_NUM];
    int type;
    int validNum = 0;
    int index;
    int capsuleNum = 1;
    int capsuleCount;
    int useMode;
    int listNo;
    int i;
    int capsuleNo;
    int masuId;

    mbGuideMotionShiftSet(guideObj, 6, TRUE);
    while (!mbGuideMotionCheck(guideObj)) {
        HuPrcVSleep();
    }
    mbCameraStackPush();
    mbWipeDissolveFadeOut();
    mbPlayerDispSet(playerNo, FALSE);
    {
        LAST5ROULETTEWORK *rouletteWork =
            omObjGetWork(rouletteObj, LAST5ROULETTEWORK);

        mbObjDispSet(rouletteObj->mdlId[0], FALSE);
        if (rouletteWork->diceF) {
            mbObjDispSet(rouletteObj->mdlId[2], FALSE);
        } else {
            mbObjDispSet(rouletteObj->mdlId[1], FALSE);
        }
    }

    capsuleCount = mbCapRandomListGet(capsuleList, LAST5_CAPSULE_LIST_MAX);
    for (i = 0, index = 0; i < capsuleCount; i++) {
        useMode = mbCapUseModeGet(capsuleList[i]);
        if (useMode == 1 || useMode == 2) {
            capsuleList[index++] = capsuleList[i];
        }
    }
    capsuleCount = index;
    if (capsuleCount < LAST5_CAPSULE_ADD_NUM) {
        while (capsuleCount < LAST5_CAPSULE_ADD_NUM) {
            capsuleList[capsuleCount] = capsuleList[0];
            capsuleCount++;
        }
    }

    {
        s16 *freeMasuP;
        LAST5ROULETTEWORK *finalRouletteWork;

    for (listNo = 0; listNo < LAST5_CAPSULE_MASU_LIST_NUM; listNo++) {
        s16 *masuP;

        masuP = HuMemDirectMallocNum(HEAP_HEAP,
            LAST5_CAPSULE_MASU_LIST_MAX * sizeof(s16), HU_MEMNUM_OVL);
        masuList[listNo] = masuP;
        masuNum[listNo] = 0;
    }

    for (i = 1, type = 0; i < mbMasuNumGet(); i++) {
        if (mbCapThrowMasuCheck(i)) {
            listNo = 0;
            if (mbCapMasuDispTypeGet(i) == 0) {
                listNo = 0;
            } else if (!mbPlayerTeamCheckSame(
                           playerNo, mbCapMasuPlayerGet(i))) {
                listNo = 1;
            } else {
                listNo = 2;
            }
            masuList[listNo][masuNum[listNo]++] = i;
        }
    }

    startPos.x = startPos.y = pos.z = 0.0f;
    listNo = 0;
    for (i = 0; i < LAST5_CAPSULE_ADD_NUM; i++) {
        s16 *masuP;

        for (; listNo < LAST5_CAPSULE_MASU_LIST_NUM; listNo++) {
            if (masuNum[listNo] != 0) {
                break;
            }
        }
        if (listNo >= LAST5_CAPSULE_MASU_LIST_NUM) {
            break;
        }

        masuP = masuList[listNo];
        index = mbRandMod(masuNum[listNo]--);
        masuId = masuP[index];
        masuP[index] = masuP[masuNum[listNo]];
        index = mbRandMod(capsuleCount--);
        capsuleNo = capsuleList[index];
        capsuleList[index] = capsuleList[capsuleCount];

        mbCameraFocusMasuSet(masuId);
        mbCameraRotSet(-30.0f, 0.0f, 0.0f);
        mbCameraZoomSet(mbCameraPlayerViewZoomGet(1));
        mbCameraMoveOnSet(FALSE);
        mbWipeDissolveFadeIn();
        mbMasuPosGet(masuId, &pos);
        mbCapAutoThrow(&startPos, &startPos, &pos, playerNo, masuId,
            capsuleNo, TRUE, 1.0f);
        HuPrcSleep(2);
        mbWipeDissolveFadeOut();
    }

    for (listNo = 0; listNo < LAST5_CAPSULE_MASU_LIST_NUM; listNo++) {
        freeMasuP = masuList[listNo];
        HuMemDirectFree(freeMasuP);
    }
    mbPlayerDispSet(playerNo, TRUE);
    {
        finalRouletteWork = omObjGetWork(rouletteObj, LAST5ROULETTEWORK);

        mbObjDispSet(rouletteObj->mdlId[0], TRUE);
        if (finalRouletteWork->diceF) {
            mbObjDispSet(rouletteObj->mdlId[2], TRUE);
        } else {
            mbObjDispSet(rouletteObj->mdlId[1], TRUE);
        }
    }
    }
    mbCameraFocusReset();
    HuPrcVSleep();
    mbCameraStackPop(0);
    HuPrcVSleep();
    mbWipeDissolveFadeIn();
    mbPlayerWinLoseVoicePlay(playerNo, 12, CHARVOICEID(6));
    mbPlayerMotionShiftSet(playerNo, 12, 0.0f, 12.0f,
        HU3D_MOTATTR_NONE);
    mbPlayerMotionEndWait(playerNo);
    mbPlayerMotIdleSet(playerNo);
}

static void ev_Last5Koopa(playerNo, rouletteObj, modelId)
int playerNo;
OMOBJ *rouletteObj;
MBMODELID modelId;
{
    HuVecF modelPos;
    HuVecF playerPos;
    int playerTbl[GW_PLAYER_MAX + 1];
    int coinAverage;
    int value;
    LAST5ROULETTEWORK *hideWork;
    LAST5ROULETTEWORK *showWork;
    int teamNo;
    int winId;
    float riseProgress;
    int i;

    mbObjPosGet(modelId, &modelPos);
    mbObjRotSet(modelId, 0.0f, LAST5_KOOPA_MODEL_ROT_Y, 0.0f);
    mbObjMotionSet(modelId, 4, HU3D_MOTATTR_NONE);
    mbObjMotionTimeSet(modelId, LAST5_KOOPA_MODEL_MOTION_TIME);
    mbObjMotionSpeedSet(modelId, 0.0f);
    mbObjDispSet(modelId, TRUE);

    for (i = LAST5_KOOPA_RISE_TIME, value = TRUE; i >= 0; i--) {
        if (i < LAST5_KOOPA_RISE_TRIGGER && value) {
            mbObjMotionSpeedSet(modelId, 1.0f);
            mbAudFXPlay(LAST5_KOOPA_APPEAR_SFX);
            mbPlayerMotionShiftSet(playerNo, 9, 0.0f, 6.0f,
                HU3D_MOTATTR_NONE);
            value = FALSE;
        }
        riseProgress =
            (float)i * (1.0f / LAST5_KOOPA_RISE_TIME);
        mbObjPosSet(modelId, modelPos.x,
            modelPos.y
                + LAST5_KOOPA_RISE_HEIGHT
                    * (LAST5_KOOPA_RISE_SCALE
                        * mbSinDeg(LAST5_KOOPA_RISE_ANGLE
                            * riseProgress)),
            modelPos.z);
        HuPrcVSleep();
    }
    while (!mbObjMotionEndCheck(modelId)) {
        HuPrcVSleep();
    }
    mbPlayerMotIdleSet(playerNo);
    mbObjMotionShiftSet(modelId, 2, 0.0f, 12.0f, HU3D_MOTATTR_NONE);
    mbAudFXPlay(LAST5_KOOPA_TALK_SFX);
    winId = mbWinCreate(2, LAST5_MESS_EFFECT_KOOPA_EXPLAIN, 13);
    mbWinPlayerDisable(winId, playerNo);
    mbWinWait(winId);
    while (!mbObjMotionEndCheck(modelId)) {
        HuPrcVSleep();
    }
    mbObjMotionShiftSet(modelId, 1, 0.0f, 12.0f, HU3D_MOTATTR_LOOP);
    winId = mbWinCreate(2, LAST5_MESS_KOOPA_EQUALIZE, 13);
    mbWinPlayerDisable(winId, playerNo);
    mbWinWait(winId);
    mbAudFXPlay(LAST5_KOOPA_REACT_SFX);
    mbObjMotionShiftSet(modelId, 3, 0.0f, 12.0f, HU3D_MOTATTR_NONE);
    while (!mbObjMotionEndCheck(modelId)) {
        HuPrcVSleep();
    }

    mbCameraStackPush();
    mbWipeDissolveFadeOut();
    mbObjMotionSet(modelId, 1, HU3D_MOTATTR_LOOP);
    mbPlayerRotSet(playerNo, 0.0f, 0.0f, 0.0f);
    mbPlayerPosGet(playerNo, &playerPos);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerDispSet(i, TRUE);
    }
    mbPlayerPosResetAll();
    mbObjDispSet(modelId, FALSE);
    {
        hideWork = omObjGetWork(rouletteObj, LAST5ROULETTEWORK);

        mbObjDispSet(rouletteObj->mdlId[0], FALSE);
        if (hideWork->diceF) {
            mbObjDispSet(rouletteObj->mdlId[2], FALSE);
        } else {
            mbObjDispSet(rouletteObj->mdlId[1], FALSE);
        }
    }

    for (i = 0; i <= GW_PLAYER_MAX; i++) {
        playerTbl[i] = -1;
    }
    coinAverage = 0;
    if (!GWTeamFGet()) {
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            coinAverage += mbPlayerCoinGet(i);
            playerTbl[i] = i;
        }
        coinAverage /= GW_PLAYER_MAX;
    } else {
        for (i = 0; i < 2; i++) {
            coinAverage += mbPlayerTeamCoinGet(i);
            playerTbl[i] = mbPlayerTeamFindPlayer(i, 0);
            if (GwPlayer[playerTbl[i]].comF) {
                value = mbPlayerTeamFindPlayer(i, 1);
                if (!GwPlayer[value].comF) {
                    playerTbl[i] = value;
                }
            }
            teamNo = GwPlayer[playerNo].team;
            if (i == teamNo) {
                playerTbl[i] = playerNo;
            }
        }
        coinAverage /= 2;
    }

    for (i = 0; playerTbl[i] >= 0; i++) {
        mbCameraPlayerViewSetFast(playerTbl[i], FALSE);
        mbStatusDispFocusSet(playerTbl[i], TRUE);
        mbWipeDissolveFadeIn();
        value = coinAverage - mbPlayerCoinGet(playerTbl[i]);
        if (value > 0) {
            mbPlayerWinLoseVoicePlay(playerTbl[i], 12, CHARVOICEID(6));
            mbPlayerMotionShiftSet(playerTbl[i], 12, 0.0f, 12.0f,
                HU3D_MOTATTR_NONE);
        } else if (value < 0) {
            mbPlayerWinLoseVoicePlay(playerTbl[i], 13, CHARVOICEID(12));
            mbPlayerMotionShiftSet(playerTbl[i], 13, 0.0f, 12.0f,
                HU3D_MOTATTR_NONE);
        }
        mbCoinAddProcExec(playerTbl[i], value, value - 1, TRUE);
        mbWipeDissolveFadeOut();
        mbPlayerMotionSet(playerTbl[i], 1, HU3D_MOTATTR_LOOP);
        mbStatusDispForceSet(playerTbl[i], FALSE);
    }

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerDispSet(i, FALSE);
    }
    mbPlayerDispSet(playerNo, TRUE);
    mbPlayerPosSetV(playerNo, &playerPos);
    mbObjDispSet(modelId, TRUE);
    {
        showWork = omObjGetWork(rouletteObj, LAST5ROULETTEWORK);

        mbObjDispSet(rouletteObj->mdlId[0], TRUE);
        if (showWork->diceF) {
            mbObjDispSet(rouletteObj->mdlId[2], TRUE);
        } else {
            mbObjDispSet(rouletteObj->mdlId[1], TRUE);
        }
    }
    mbCameraStackPop(0);
    HuPrcVSleep();
    mbWipeDissolveFadeIn();
}
