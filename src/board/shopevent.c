/* Retail shopevent.o omits math.h's weak sqrtf constants. */
#define _MATH_H
#include "dolphin/math.h"

#include "game/board/main.h"
#include "game/board/audio.h"
#include "game/board/masu.h"
#include "game/board/object.h"
#include "game/board/pause.h"
#include "game/board/player.h"
#include "game/board/status.h"
#include "game/board/tutorial.h"
#include "game/board/window.h"
#include "game/flag.h"
#include "game/memory.h"

#include "humath.h"
#include "string.h"

typedef void (*MBSHOPOBJHOOK)(int modelId, int shopNo);

typedef struct MBSHOPWORK {
    int playerNo;
    int shopNo;
} MBSHOPWORK;

typedef struct MBSHOPOMWORK {
    int modelId;
    int shopNo;
    int masuId;
    int masuLinkId;
    BOOL pathF;
    int masuEndId;
    int unk18;
    int unk1C;
    BOOL modelMotionF;
    BOOL motionExecF;
    BOOL openF;
    int backModelId[8];
    int backMotNo[8];
    BOOL backDispF[8];
    HuVecF masuPos;
    HuVecF capsulePos;
    HuVecF shopPos;
} MBSHOPOMWORK;

typedef struct ShopOffer_s {
    int capsuleNo;
    int cost;
    int messageId;
    char costText[16];
} SHOP_OFFER;

static HuVecF ev_ShopCapsulePlayer[3][3] = {
    {
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
    },
    {
        { 90.0f, 50.0f, 0.0f },
        { -90.0f, 50.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
    },
    {
        { 90.0f, 50.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
        { -90.0f, 50.0f, 0.0f },
    },
};

static HuVecF ev_ShopLightPos = { -10000.0f, 10000.0f, -10000.0f };
static HuVecF ev_ShopLightDir = { 1.0f, -1.0f, -1.0f };
static int ev_ShopSprFileTbl[4] = {
    DATANUM(DATA_board, 0x26),
    DATANUM(DATA_board, 0x23),
    DATANUM(DATA_board, 0x23),
    DATANUM(DATA_board, 0x23),
};
static HuVecF ev_ShopWinPos = { 288.0f, 176.0f, 0.0f };
static HuVecF ev_ShopCapsulePos[3][3] = {
    {
        { 288.0f, 170.0f, 1000.0f },
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
    },
    {
        { 214.0f, 170.0f, 1000.0f },
        { 362.0f, 170.0f, 1000.0f },
        { 0.0f, 0.0f, 0.0f },
    },
    {
        { 192.0f, 170.0f, 1000.0f },
        { 288.0f, 170.0f, 1000.0f },
        { 384.0f, 170.0f, 1000.0f },
    },
};

static OMOBJ *ev_ShopOMObj[GW_PLAYER_MAX];
static GXColor ev_ShopLightColor = { 255, 255, 255, 255 };

void mbev_ShopCreate(int dataNum, int motDataNum);
void mbev_ShopBackMotCreate(int dataNum, int motDataNum, int motNo, BOOL linkF, char *hookName);
static void ev_ShopOMExec(OMOBJ *obj);
static void ev_ShopOpenSet(int shopNo, BOOL openF);
static void ev_Shop(MBSHOPWORK *work);
static int ev_ShopSelect(MBSHOPWORK *work, SHOP_OFFER *offer, int offerNum, int winType);
static int ev_ShopMesGet(int messNo);
void mbev_ShopExObjHookSet(MBSHOPOBJHOOK hook);

static BOOL ev_ShopEnableF;
static MBSHOPOBJHOOK ev_ShopExObjHook;
static int ev_ShopNum;

void mbev_ShopEnableSet(BOOL enableF)
{
    ev_ShopEnableF = enableF;
}

void mbev_ShopInit(int dataNum)
{
    mbev_ShopExObjHookSet(NULL);
    mbev_ShopCreate(dataNum, -1);
}

void mbev_ShopExInit(int dataNum, MBSHOPOBJHOOK hook)
{
    mbev_ShopExObjHookSet(hook);
    mbev_ShopCreate(dataNum, -1);
}

void mbev_ShopCreate(int dataNum, int motDataNum)
{
    HuVecF shopPos;
    HuVecF masuPos;
    HuVecF dir;
    int masuTbl[3];
    int motDataNumTbl[2];
    int masuId;
    int i;
    int shopNo;

    ev_ShopEnableF = TRUE;
    for (masuId = 0; masuId < 3; masuId++) {
        ev_ShopOMObj[masuId] = NULL;
    }
    shopNo = 0;
    motDataNumTbl[0] = motDataNum;
    motDataNumTbl[1] = -1;
    for (masuId = 1; masuId < mbMasuNumGet(); masuId++) {
        int linkMasuId;
        int shopLinkMasuId;
        int shopMasuId;
        int nextMasuId;
        MBSHOPOMWORK *work;
        OMOBJ *obj;

        if (mbMasuTypeGet(masuId) != 9) {
            continue;
        }
        linkMasuId = mbMasuAttrFindLink(masuId, 0x20);
        if (linkMasuId < 0) {
            break;
        }
        nextMasuId = mbMasuAttrFindLink(linkMasuId, 0x20);
        work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MBSHOPOMWORK), HU_MEMNUM_OVL);
        memset(work, 0, sizeof(MBSHOPOMWORK));
        shopMasuId = masuId;
        shopLinkMasuId = linkMasuId;
        mbMasuPosGet(shopMasuId, &shopPos);
        mbMasuPosGet(shopLinkMasuId, &masuPos);
        if (nextMasuId != -1) {
            work->pathF = TRUE;
            masuTbl[0] = shopMasuId;
            masuTbl[1] = shopLinkMasuId;
            masuTbl[2] = nextMasuId;
            while ((nextMasuId = mbMasuAttrFindLink(masuTbl[2], 0x20)) > 0) {
                masuTbl[0] = masuTbl[1];
                masuTbl[1] = masuTbl[2];
                masuTbl[2] = nextMasuId;
            }
            mbMasuPosGet(masuTbl[1], &work->shopPos);
            mbMasuPosGet(masuTbl[2], &work->masuPos);
            shopPos = work->shopPos;
            masuPos = work->masuPos;
            work->masuEndId = masuTbl[1];
        } else {
            work->pathF = FALSE;
            work->masuEndId = -1;
            PSVECSubtract(&shopPos, &masuPos, &dir);
            if (PSVECMag(&dir) > 0.0f) {
                PSVECNormalize(&dir, &dir);
            }
            PSVECScale(&dir, &dir, 100.0f);
            PSVECAdd(&masuPos, &dir, &work->shopPos);
            work->shopPos.y = masuPos.y;
            mbMasuPosGet(shopLinkMasuId, &work->masuPos);
        }
        PSVECSubtract(&masuPos, &shopPos, &dir);
        if (PSVECMag(&dir) > 0.0f) {
            PSVECNormalize(&dir, &dir);
        }
        PSVECScale(&dir, &dir, 100.0f);
        PSVECAdd(&masuPos, &dir, &work->capsulePos);
        if (GwSystem.curTime) {
            work->capsulePos.y += 30.000002f;
        }
        obj = ev_ShopOMObj[shopNo] = omAddObjEx(mbObjMan, -32768, 0, 0,
            OM_GRP_NONE, ev_ShopOMExec);
        obj->data = work;
        if (dataNum <= 0) {
            work->modelId = -1;
        } else if (motDataNum != -1) {
            work->modelId = mbObjCreate(dataNum, motDataNumTbl, TRUE);
            work->modelMotionF = TRUE;
        } else {
            work->modelId = mbObjCreate(dataNum, NULL, TRUE);
            work->modelMotionF = FALSE;
        }
        PSVECSubtract(&shopPos, &masuPos, &dir);
        if (work->modelId != -1) {
            mbObjPosSetV(work->modelId, &masuPos);
            mbObjRotSet(work->modelId, 0.0f, HuAtan(dir.x, dir.z), 0.0f);
            mbObjMotionSpeedSet(work->modelId, 0.0f);
        }
        work->shopNo = shopNo;
        work->masuId = shopMasuId;
        work->masuLinkId = shopLinkMasuId;
        work->unk18 = 0;
        work->unk1C = 0;
        work->motionExecF = FALSE;
        work->openF = FALSE;
        if (ev_ShopExObjHook != NULL) {
            ev_ShopExObjHook(work->modelId, shopNo);
        }
        for (i = 0; i < 8; i++) {
            work->backModelId[i] = -1;
            work->backMotNo[i] = 0;
            work->backDispF[i] = FALSE;
        }
        shopNo++;
    }
    ev_ShopNum = shopNo;
}

void mbev_ShopExObjHookSet(MBSHOPOBJHOOK hook)
{
    ev_ShopExObjHook = hook;
}

void mbev_ShopBackCreate(int dataNum, int motDataNum, int motNo, BOOL linkF)
{
    mbev_ShopBackMotCreate(dataNum, motDataNum, motNo, linkF, 0);
}

void mbev_ShopBackMotCreate(int dataNum, int motDataNum, int motNo, BOOL linkF, char *hookName)
{
    int motDataNumTbl[16];
    HuVecF pos;
    HuVecF rot;
    int i;
    int j;

    for (i = 0; i < 3; i++) {
        OMOBJ *obj = ev_ShopOMObj[i];
        MBSHOPOMWORK *work;

        if (obj == NULL) {
            continue;
        }
        work = obj->data;
        for (j = 0; j < 8; j++) {
            if (work->backModelId[j] == -1) {
                break;
            }
        }
        if (motDataNum >= 0) {
            motDataNumTbl[0] = motDataNum;
            motDataNumTbl[1] = -1;
            work->backModelId[j] = mbObjCreate(dataNum, motDataNumTbl, linkF);
        } else {
            work->backModelId[j] = mbObjCreate(dataNum, NULL, linkF);
        }
        if (work->modelId != -1) {
            mbObjPosGet(work->modelId, &pos);
            mbObjRotGet(work->modelId, &rot);
        } else {
            pos.x = pos.y = pos.z = 0.0f;
            rot.x = rot.y = rot.z = 0.0f;
        }
        if (hookName == NULL) {
            mbObjPosSetV(work->backModelId[j], &pos);
            mbObjRotSetV(work->backModelId[j], &rot);
        } else if (work->modelId != -1) {
            mbObjHookSet(work->modelId, hookName, work->backModelId[j]);
        }
        work->backMotNo[j] = motNo;
        switch (work->backMotNo[j]) {
            case 0:
            case 1:
                mbObjMotionSpeedSet(work->backModelId[j], 0.0f);
                break;
            case 2:
            case 3:
                mbObjMotionSpeedSet(work->backModelId[j], 0.0f);
                break;
            case 4: {
                MBMODELID modelId = work->backModelId[j];

                mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
                break;
            }
        }
        if (work->backMotNo[j] == 1 || work->backMotNo[j] == 3) {
            work->backDispF[j] = TRUE;
        }
        if (!work->backDispF[j]) {
            mbObjDispSet(work->backModelId[j], FALSE);
        }
    }
}

static void ev_ShopOMExec(OMOBJ *obj)
{
    MBSHOPOMWORK *work = obj->data;
    int i;

    if (mbExitCheck() || ev_ShopOMObj[work->shopNo] == NULL) {
        omDelObjEx(mbObjMan, obj);
        return;
    }
    if (work->motionExecF && work->modelId != -1) {
        if (work->openF) {
            if (mbObjMotionTimeGet(work->modelId) >= mbObjMotionMaxTimeGet(work->modelId)) {
                work->motionExecF = FALSE;
            }
        } else {
            if (work->modelMotionF) {
                if (mbObjMotionTimeGet(work->modelId) >= mbObjMotionMaxTimeGet(work->modelId)) {
                    work->motionExecF = FALSE;
                }
            } else if (mbObjMotionTimeGet(work->modelId) <= 0.0f) {
                work->motionExecF = FALSE;
            }
            if (!work->motionExecF) {
                for (i = 0; i < 8; i++) {
                    if (work->backModelId[i] != -1 && !work->backDispF[i]) {
                        mbObjDispSet(work->backModelId[i], FALSE);
                    }
                }
            }
        }
    }
}

static void ev_ShopOpenSet(int shopNo, BOOL openF)
{
    OMOBJ *obj = ev_ShopOMObj[shopNo];
    MBSHOPOMWORK *work = obj->data;
    int i;

    if (work->modelId == -1) {
        return;
    }
    work->motionExecF = TRUE;
    work->openF = openF;
    if (work->openF) {
        mbAudFXPlay(0x477);
        mbObjMotionSet(work->modelId, 0, 0);
        mbObjMotionSpeedSet(work->modelId, 1.0f);
        for (i = 0; i < 8; i++) {
            if (work->backModelId[i] == -1) {
                continue;
            }
            switch (work->backMotNo[i]) {
                case 0:
                case 1:
                case 2:
                case 3:
                    mbObjMotionTimeSet(work->backModelId[i], 0.0f);
                    mbObjMotionSpeedSet(work->backModelId[i], 1.0f);
                    mbObjDispSet(work->backModelId[i], TRUE);
                    break;
                default:
                    mbObjDispSet(work->backModelId[i], TRUE);
                    break;
            }
        }
    } else {
        mbAudFXPlay(0x478);
        if (work->modelMotionF) {
            mbObjMotionSet(work->modelId, 1, 0);
            mbObjMotionSpeedSet(work->modelId, 1.0f);
        } else {
            mbObjMotionSpeedSet(work->modelId, -1.0f);
        }
        for (i = 0; i < 8; i++) {
            if (work->backModelId[i] == -1) {
                continue;
            }
            switch (work->backMotNo[i]) {
                case 0:
                case 1:
                    mbObjMotionSpeedSet(work->backModelId[i], -1.0f);
                    break;
                case 2:
                case 3:
                    mbObjMotionTimeSet(work->backModelId[i], 0.0f);
                    mbObjMotionSpeedSet(work->backModelId[i], 1.0f);
                    break;
            }
        }
    }
}

int mbev_Shop(int playerNo, int shopNo)
{
    MBSHOPWORK *work;
    void *workP;

    mbMoveNumDispSet(playerNo, FALSE);
    workP = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MBSHOPWORK), HU_MEMNUM_OVL);
    work = workP;
    memset(work, 0, sizeof(MBSHOPWORK));
    work->playerNo = playerNo;
    work->shopNo = shopNo;
    mbPauseDisableSet(TRUE);
    ev_Shop(work);
    HuMemDirectFree(work);
    mbPauseDisableSet(FALSE);
    mbMoveNumDispSet(playerNo, TRUE);
    return 0;
}

static void ev_Shop(MBSHOPWORK *work)
{
    SHOP_OFFER offer[3];
    SHOP_OFFER swap;
    SHOP_OFFER *offerP;
    MBSHOPOMWORK *shopWork;
    HuVecF direction;
    HuVecF pos;
    HuVecF returnPos;
    HuVecF shopPos;
    HuVecF playerPos;
    int path[16];
    int capsuleObjId[3];
    int motionDataNum[4];
    int readStat;
    int shopNo;
    int offerNum;
    int shopListNum;
    int winType;
    int pathNum;
    int deleteCapsuleNo;
    int deleteIndex;
    int selection;
    int i;
    int j;
    s8 *shopList;
    s16 shopModelId;
    s16 lightId;
    BOOL doneF;
    BOOL comDeclineF;
    float angle;

    if (!ev_ShopEnableF) {
        return;
    }
    shopModelId = -1;
    lightId = -1;
    for (i = 0; i < 3; i++) {
        capsuleObjId[i] = -1;
    }
    if (!GwSystem.curTime) {
        winType = 8;
    } else {
        winType = 9;
    }
    readStat = mbBGRead(0x130000);
    mbPlayerMotionShiftSet(work->playerNo, 1, 0.0f, 8.0f, 0x40000001);
    for (shopNo = 0; shopNo < ev_ShopNum; shopNo++) {
        OMOBJ *shopObj = ev_ShopOMObj[shopNo];
        MBSHOPOMWORK *findWork = shopObj->data;

        if (findWork->masuId == work->shopNo) {
            break;
        }
    }
    if (shopNo == ev_ShopNum) {
        shopNo = -1;
    }

    shopList = HuMemDirectMallocNum(HEAP_HEAP, 0x210, HU_MEMNUM_OVL);
    shopListNum = mbCapShopListGet(work->playerNo, shopList);
    offerNum = 0;
    offerP = offer;
    for (i = 0; i < 3 && i < shopListNum; i++, offerP++) {
        offerP->capsuleNo = shopList[i * 0x10];
        offerP->cost = mbCapBuyCostGet((s16)offerP->capsuleNo, (s16)work->playerNo);
        offerP->messageId = mbCapUseMesGet(offerP->capsuleNo);
        sprintf(offerP->costText, "%d", offerP->cost);
        offerNum++;
    }
    offerP = offer;
    for (i = 0; i < offerNum; i++, offerP++) {
        if (mbPlayerCoinGet(work->playerNo) < offerP->cost) {
            offerP->capsuleNo = 0;
            offerP->cost = mbCapBuyCostGet(0, (s16)work->playerNo);
            offerP->messageId = mbCapUseMesGet(0);
            sprintf(offerP->costText, "%d", offerP->cost);
        }
    }
    offerP = offer;
    for (i = 0; i < offerNum; i++, offerP++) {
        mbCapNumInc(offerP->capsuleNo, 1);
    }
    for (i = 0; i < 64 && offerNum > 1; i++) {
        int first = mbRandMod(offerNum);
        int second = mbRandMod(offerNum);

        if (first != second) {
            swap = offer[first];
            offer[first] = offer[second];
            offer[first] = swap;
        }
    }
    HuMemDirectFree(shopList);
    HuPrcVSleep();

    if (!_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        if (GwSystem.turnNo >= GwSystem.turnMax) {
            mbAudFXPlay(GwSystem.curTime ? 0x3C1 : 0x3DB);
            mbWinCreate(2, ev_ShopMesGet(0x39000B), winType);
            mbWinTopWait();
            goto cleanup;
        }
        if (mbPlayerCoinGet(work->playerNo) <= 4) {
            mbAudFXPlay(GwSystem.curTime ? 0x3C2 : 0x3DC);
            mbWinCreate(2, ev_ShopMesGet(0x390001), winType);
            mbWinTopWait();
            goto cleanup;
        }
        mbWinCreateChoice(2, ev_ShopMesGet(0x390000), -1, 0);
        if (GwPlayer[work->playerNo].comF) {
            comDeclineF = mbMasuFind_TypeStepGet((s16)work->shopNo, 7)
                < GwPlayer[work->playerNo].moveNum;
            if (mbMasuFind_TypeStepGet((s16)work->shopNo, 7) < 20
                && mbPlayerCoinGet(work->playerNo) < 25
                && mbPlayerCoinGet(work->playerNo) >= 20) {
                comDeclineF = TRUE;
            }
            if (GwSystem.tagF && mbPlayerCoinGet(work->playerNo) < 25
                && mbPlayerCoinGet(work->playerNo) >= 20) {
                comDeclineF = TRUE;
            }
            if ((float)mbRandMod(0x10000000) / 268435456.0f < 0.3f
                && !comDeclineF
                && mbPlayerCapsuleNumGet(work->playerNo) < mbPlayerCapsuleMaxGet()) {
                mbComChoiceLeftSet();
            } else {
                mbComChoiceRightSet();
            }
        }
        mbWinTopWait();
        if (mbWinTopChoiceGet() != 0 || mbWinTopChoiceGet() == -1) {
            goto cleanup;
        }
    } else if (mbTutorialCall(0x13) != 1) {
        goto cleanup;
    }

    for (i = 0; i < offerNum; i++) {
        capsuleObjId[i] = mbCapObjColorCreate(offer[i].capsuleNo, 0);
        mbCapObjColorLayerSet(capsuleObjId[i], 4);
        shopWork = ev_ShopOMObj[shopNo]->data;
        pos = shopWork->masuPos;
        pos.y += 50.0f;
        direction.x = shopWork->masuPos.x - shopWork->shopPos.x;
        direction.y = shopWork->masuPos.y - shopWork->shopPos.y;
        direction.z = shopWork->masuPos.z - shopWork->shopPos.z;
        angle = (float)((atan2(direction.x, direction.z) / M_PI) * 180.0);
        angle += ev_ShopCapsulePlayer[offerNum - 1][i].x;
        pos.x += ev_ShopCapsulePlayer[offerNum - 1][i].y
            * (float)sin((M_PI * angle) / 180.0);
        pos.z += ev_ShopCapsulePlayer[offerNum - 1][i].y
            * (float)cos((M_PI * angle) / 180.0);
        mbCapObjColorPosSetV(capsuleObjId[i], &pos);
        mbCapObjColorScaleSet(capsuleObjId[i], 0.8f, 0.8f, 0.8f);
        HuPrcVSleep();
    }
    if (readStat != -1) {
        mbBGReadWait(readStat);
    }
    if (!GwSystem.curTime) {
        motionDataNum[0] = 0x130006;
        motionDataNum[1] = 0x130006;
        motionDataNum[2] = 0x130007;
        motionDataNum[3] = -1;
        shopModelId = mbObjCreate(0x130005, motionDataNum, FALSE);
    } else {
        motionDataNum[0] = 0x130001;
        motionDataNum[1] = 0x130001;
        motionDataNum[2] = 0x130004;
        motionDataNum[3] = -1;
        shopModelId = mbObjCreate(0x130000, motionDataNum, FALSE);
    }
    lightId = Hu3DLLightCreateV(mbObjModelIDGet(shopModelId),
        &ev_ShopLightPos, &ev_ShopLightDir, &ev_ShopLightColor);
    Hu3DLLightStaticSet(mbObjModelIDGet(shopModelId), lightId, TRUE);
    Hu3DLLightInfinitytSet(mbObjModelIDGet(shopModelId), lightId);

    shopWork = ev_ShopOMObj[shopNo]->data;
    mbMasuPosGet((s16)work->shopNo, &playerPos);
    direction.x = shopWork->masuPos.x - shopWork->shopPos.x;
    direction.y = shopWork->masuPos.y - shopWork->shopPos.y;
    direction.z = shopWork->masuPos.z - shopWork->shopPos.z;
    shopPos = shopWork->capsulePos;
    mbObjPosSetV(shopModelId, &shopPos);
    mbObjRotSet(shopModelId, 0.0f,
        180.0f + (float)((atan2(direction.x, direction.z) / M_PI) * 180.0), 0.0f);
    mbObjMotionSet(shopModelId, 1, 0x40000001);
    ev_ShopOpenSet(shopNo, TRUE);
    omVibrate((s16)work->playerNo, 20, 7, 3);

    direction.x = shopWork->shopPos.x - playerPos.x;
    direction.y = shopWork->shopPos.y - playerPos.y;
    direction.z = shopWork->shopPos.z - playerPos.z;
    mbPlayerRotateStart(work->playerNo,
        (int)((atan2(direction.x, direction.z) / M_PI) * 180.0), 15);
    while (!mbPlayerRotateCheck(work->playerNo)) {
        HuPrcVSleep();
    }
    while (shopWork->motionExecF) {
        HuPrcVSleep();
    }
    returnPos = playerPos;
    mbStatusDispSetAll(FALSE);
    mbCameraPlayerViewSet(work->playerNo, 0);
    mbPlayerColSnapPlayerSet(work->playerNo, FALSE);
    pathNum = 1;
    if (!shopWork->pathF) {
        mbPlayerMasuMovePos(work->playerNo, &shopWork->shopPos, TRUE);
    } else {
        int masuId = GwPlayer[work->playerNo].masuId;

        path[0] = masuId;
        while (masuId != shopWork->masuEndId) {
            masuId = mbMasuAttrFindLink((s16)masuId, 0x20);
            mbMasuPosGet(masuId, &pos);
            GwPlayer[work->playerNo].masuIdNext = masuId;
            mbPlayerMasuMovePos(work->playerNo, &pos, TRUE);
            GwPlayer[work->playerNo].masuId = masuId;
            path[pathNum++] = masuId;
        }
    }
    direction.x = shopWork->masuPos.x - shopWork->shopPos.x;
    direction.y = shopWork->masuPos.y - shopWork->shopPos.y;
    direction.z = shopWork->masuPos.z - shopWork->shopPos.z;
    mbPlayerRotateStart(work->playerNo,
        (int)((atan2(direction.x, direction.z) / M_PI) * 180.0), 15);
    while (!mbPlayerRotateCheck(work->playerNo)) {
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(work->playerNo, 1, 0.0f, 8.0f, 0x40000001);

    if (!GwPlayer[work->playerNo].comF) {
        while (!mbStatusOffCheckAll()) {
            HuPrcVSleep();
        }
        mbStatusDispFocusSet(work->playerNo, TRUE);
        mbAudFXPlay(GwSystem.curTime ? 0x3C1 : 0x3DB);
        mbWinCreate(2, ev_ShopMesGet(0x390002), winType);
        mbWinTopWait();
        if (GwSystem.curTime && GwPlayer[work->playerNo].rank > 1) {
            mbAudFXPlay(0x3C0);
            mbWinCreate(2, ev_ShopMesGet(0x39000D), winType);
            mbWinTopWait();
        }
        doneF = FALSE;
        do {
            if (offerNum > 0 && offerNum < 4) {
                selection = ev_ShopSelect(work, offer, offerNum, winType);
            } else {
                mbAudFXPlay(GwSystem.curTime ? 0x3C2 : 0x3DC);
                mbWinCreate(2, ev_ShopMesGet(0x390003), winType);
                mbWinTopWait();
                doneF = TRUE;
                selection = -1;
            }
            if (selection >= 0 && selection < offerNum) {
                deleteCapsuleNo = -1;
                deleteIndex = -1;
                if (mbPlayerCapsuleNumGet(work->playerNo) < mbPlayerCapsuleMaxGet()) {
                    deleteIndex = -1;
                } else {
                    mbWinCreateChoice(2, ev_ShopMesGet(0x390009), winType, 0);
                    if (GwPlayer[work->playerNo].comF) {
                        mbComChoiceLeftSet();
                    }
                    mbWinTopWait();
                    if (mbWinTopChoiceGet() != 0) {
                        continue;
                    }
                    do {
                        deleteCapsuleNo = mbCapDelete(-1, TRUE);
                        if (deleteCapsuleNo == -3) {
                            mbev_Scroll(work->playerNo, FALSE);
                            deleteIndex = -1;
                        } else if (deleteCapsuleNo == -7) {
                            deleteIndex = -2;
                        } else {
                            for (i = 0; i < mbPlayerCapsuleMaxGet(); i++) {
                                if (deleteCapsuleNo == mbPlayerCapsuleGet(work->playerNo, i)) {
                                    deleteIndex = i;
                                }
                            }
                            if (deleteIndex != -1) {
                                mbPlayerCapsuleRemove(work->playerNo, deleteIndex);
                            }
                        }
                    } while (deleteIndex == -1);
                    if (deleteIndex == -2) {
                        continue;
                    }
                }
                mbCoinAddExec(work->playerNo, -offer[selection].cost);
                mbCapCapsuleGet(work->playerNo, offer[selection].capsuleNo);
                mbPlayerCapsuleAdd(work->playerNo, offer[selection].capsuleNo);
                mbPlayerWinLoseVoicePlay(work->playerNo, 12, 0x243);
                mbPlayerMotionShiftSet(work->playerNo, 12, 0.0f, 12.0f, 0);
                if (deleteCapsuleNo == -1) {
                    mbWinCreate(2, ev_ShopMesGet(0x390007), -1);
                    mbWinTopInsertMesSet(offer[selection].messageId, 0);
                } else {
                    mbWinCreate(2, ev_ShopMesGet(0x39000A), -1);
                    mbWinTopInsertMesSet(mbCapUseMesGet(deleteCapsuleNo), 0);
                    mbWinTopInsertMesSet(offer[selection].messageId, 1);
                }
                mbWinTopWait();
                while (!mbPlayerMotionEndCheck(work->playerNo)) {
                    HuPrcVSleep();
                }
                mbPlayerMotIdleSet(work->playerNo);
                mbObjMotionShiftSet(shopModelId, 3, 0.0f, 8.0f, 0x40000001);
                mbAudFXPlay(GwSystem.curTime ? 0x3C0 : 0x3DA);
                mbWinCreate(2, ev_ShopMesGet(0x390008), winType);
                mbWinTopWait();
                doneF = TRUE;
            } else {
                doneF = TRUE;
            }
        } while (!doneF);
        mbStatusDispFocusSet(work->playerNo, FALSE);
    } else {
        for (selection = 0; selection < offerNum; selection++) {
            if (mbPlayerCoinGet(work->playerNo) >= offer[selection].cost) {
                break;
            }
        }
        if (selection < offerNum) {
            if (_CheckFlag(FLAG_BOARD_TUTORIAL)) {
                int capsuleNo = mbTutorialCall(0x14);

                if (capsuleNo >= 0) {
                    offer[selection].capsuleNo = capsuleNo;
                    offer[selection].cost = mbCapBuyCostGet((s16)capsuleNo,
                        (s16)work->playerNo);
                    offer[selection].messageId = mbCapUseMesGet(capsuleNo);
                }
            }
            mbCoinAddProcExec(work->playerNo, -offer[selection].cost, -1, TRUE);
            mbCapCapsuleGet(work->playerNo, offer[selection].capsuleNo);
            mbPlayerCapsuleAdd(work->playerNo, offer[selection].capsuleNo);
            mbPlayerWinLoseVoicePlay(work->playerNo, 12, 0x243);
            mbPlayerMotionShiftSet(work->playerNo, 12, 0.0f, 12.0f, 0);
            mbWinCreate(2, ev_ShopMesGet(0x390007), -1);
            mbWinTopInsertMesSet(offer[selection].messageId, 0);
            mbWinTopWait();
            while (!mbPlayerMotionEndCheck(work->playerNo)) {
                HuPrcVSleep();
            }
            mbPlayerMotIdleSet(work->playerNo);
            mbObjMotionShiftSet(shopModelId, 3, 0.0f, 8.0f, 0x40000001);
        }
    }

    mbMasuPosGet((s16)work->shopNo, &playerPos);
    if (!shopWork->pathF) {
        mbPlayerMasuMovePos(work->playerNo, &returnPos, TRUE);
    } else {
        for (i = 1; i < pathNum; i++) {
            s16 masuId = path[pathNum - (i + 1)];

            GwPlayer[work->playerNo].masuIdNext = masuId;
            mbMasuPosGet(masuId, &pos);
            mbPlayerMasuMovePos(work->playerNo, &pos, TRUE);
            GwPlayer[work->playerNo].masuId = masuId;
        }
    }
    mbPlayerColSnapPlayerSet(work->playerNo, TRUE);
    mbPlayerMotionShiftSet(work->playerNo, 1, 0.0f, 8.0f, 0x40000001);
    while (!mbStatusOffCheckAll()) {
        HuPrcVSleep();
    }
    mbStatusDispSetAll(TRUE);
    if (shopWork->openF) {
        ev_ShopOpenSet(shopNo, FALSE);
    }
    while (shopWork->motionExecF) {
        HuPrcVSleep();
    }
    mbCameraPlayerViewSet(work->playerNo, 2);

cleanup:
    mbPlayerColSnapPlayerSet(work->playerNo, TRUE);
    if (shopModelId != -1) {
        if (lightId != -1) {
            Hu3DLLightKill(mbObjModelIDGet(shopModelId), lightId);
        }
        mbObjKill(shopModelId);
    }
    for (j = 0; j < offerNum; j++) {
        if (capsuleObjId[j] != -1) {
            mbCapObjColorKill(capsuleObjId[j]);
        }
    }
    HuDataDirClose(0x130000);
}

static int ev_ShopMesGet(int messNo)
{
    if (!GwSystem.curTime) {
        return messNo;
    }
    return messNo + 14;
}
