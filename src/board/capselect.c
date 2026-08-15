#define _MATH_H
#include "game/board/main.h"

#include "game/board/audio.h"
#include "game/board/camera.h"
#include "game/board/masu.h"
#include "game/board/object.h"
#include "game/board/pause.h"
#include "game/board/player.h"
#include "game/board/status.h"
#include "game/board/tutorial.h"
#include "game/board/window.h"

#include "game/esprite.h"
#include "game/frand.h"
#include "game/memory.h"
#include "game/pad.h"
#include "game/wipe.h"

#include "dolphin/math.h"
#include "messdir_enum.h"
#include "string.h"

#define M_PI 3.141592653589793

#define CAPSELECT_DATA_ARROW DATANUM(DATA_capsule, 38)
#define CAPSELECT_DATA_MASU_MODEL DATANUM(DATA_capsule, 33)
#define CAPSELECT_RANDOM_MASK ((1 << 15) - 1)

enum {
    CAPSELECT_ARROW_PRIORITY = 1400,
    CAPSELECT_ARROW_DRAW_NO = 32,
    CAPSELECT_CAPSULE_DICE = 47,
    CAPSELECT_TUTORIAL_SELECT = 16,
    CAPSELECT_TUTORIAL_CAPSULE_GET = 21,
    CAPSELECT_TUTORIAL_CAPSULE_END = 22,
    CAPSELECT_SHRINK_PRIORITY = 8197,
    CAPSELECT_SHRINK_STACK_SIZE = 16384,
    CAPMASU_OBJECT_PRIORITY = 260,
};

extern int mbCapObjCreate(int capsuleNo, BOOL flag);
extern void mbCapObjKill(int objId);
extern int mbCapUse(int playerNo, int capsuleNo);
extern int mbCapUseMesGet(int capsuleNo);
extern s16 mbCapUseModeGet(s16 capsuleNo);
extern void mbCapNumInc(int capsuleNo, int mode);
extern int mbCapSelectComGet(int playerNo, int *capsuleTbl,
    int capsuleNum);
extern int mbCapSelectDeleteComGet(int playerNo, int *capsuleTbl,
    int capsuleNum);
extern int mbCapMasuNextGet(int playerNo);
extern int mbCapDescWinCreate(int capsuleNo);
extern int mbSingleCall(int mode, int arg);
extern s8 mbPadStkXGet(s32 playerNo);
extern void mbev_Scroll(int playerNo, BOOL mapF);
extern OMOBJ *mbev_CapEffExplodeCreate(void);
extern void mbev_CapEffDustExplodeAdd(OMOBJ *obj, HuVecF *pos);
extern int mbev_CapEffExplodeAnimGet(OMOBJ *obj);
extern void mbev_CapEffExplodeKill(OMOBJ *obj);
/* Caller contract reconstructed from the matching CapSelect translation unit. */
extern int Hu3D3Dto2D(HuVecF *src, s16 cameraBit, HuVecF *dst);

static HUPROCESS *ev_CapSelectShrinkProc[4] = { NULL, NULL, NULL, NULL };

static int ev_CapSelectObjId[4];
static int ev_CapSelectResult[4];
static int ev_CapSelectExtra[4];
static OMOBJ *ev_CapMasuOMObj[16];

static s8 ev_CapSelectValue[4];
static int ev_CapSelectMdlId;
static BOOL ev_CapMasuDispF;
static BOOL ev_CapSelectStoryF;

typedef struct CapMasuWork_s {
    int objNo;
    int masuId;
    int modelId;
    int angle;
    int playerNo;
    BOOL hiddenF;
    float scale;
    HuVecF pos;
} CAPMASUWORK;

typedef struct CapSelectWork_s {
    int capsuleNum;
    int selectNo;
    int comSelectNo;
    int playerNo;
    int descWinIndex;
    s16 winId[3];
    s16 objId[6];
    s16 arrowSprId[2];
    s16 pulseAngle;
    s16 comDelay;
    float scale[6];
    int extraCapsule;
    BOOL deleteF;
} CAPSELECTWORK;

typedef struct CapSelectShrinkWork_s {
    int playerNo;
    int objId;
    int count;
    s16 objIdTbl[6];
    HuVecF start[6];
    HuVecF end[6];
} CAPSELECTSHRINKWORK;

int mbCapSelect(void);
int mbCapDelete(int capsuleNo, BOOL repeatF);
static void CapSelect(CAPSELECTWORK *work);
float mbCapSelectGrow(HuVecF *start, HuVecF *end, MBMODELID modelId,
    float weight, float baseScale);
static int CapSelectPadExec(CAPSELECTWORK *work);
void mbCapSelectResultSet(int playerNo, int objId, int result);
void mbCapSelectResultGet(int playerNo, int *objId, int *result);
void mbCapSelectResultReset(int playerNo);
static void CapSelectShrinkCreate(int playerNo, int objId,
    CAPSELECTWORK *selectWork, HuVecF *start, HuVecF *end);
static void CapSelectShrink(void);
static void CapSelectShrinkDestroy(void);
BOOL mbCapSelectShrinkCheck(int playerNo);
static void CapSelectStoryFSet(BOOL storyF);
static void CapSelectExtraCapsuleGet(int playerNo, int capsuleNo);
static int CapSelectCapsuleGet(int playerNo, int selectNo);
static int CapSelectNumGet(int playerNo);
static int CapSelectComGet(int playerNo, BOOL deleteF);
void mbCapMasuExec(int playerNo, int masuId);
void mbCapCapsuleGet(int playerNo, int capsuleNo);
static int CapHelpWinCreate(int capsuleNo, BOOL deleteF);

void mbCapMasuObjCreateAll(void);
void mbCapMasuObjCreate(int masuId);

static void CapMasuOMExec(OMOBJ *obj);

int mbCapSelect(void)
{
    CAPSELECTWORK *work;
    BOOL partyF;
    CAPSELECTWORK *workData;
    int playerNo = GwSystem.turnPlayerNo;
    int objId;
    int result;
    BOOL storyPartyF;

    mbCapSelectResultSet(playerNo, -1, -1);
    ev_CapSelectMdlId = -1;
    partyF = GwSystem.partyF;
    if (partyF) {
        mbStatusDispSetAll(FALSE);
        while (!mbStatusOffCheckAll()) {
            HuPrcVSleep();
        }
    }
    for (;;) {
        while (!mbCapSelectShrinkCheck(playerNo)) {
            HuPrcVSleep();
        }
        workData = HuMemDirectMallocNum(HEAP_HEAP,
            sizeof(CAPSELECTWORK),
            HU_MEMNUM_OVL);
        work = workData;
        memset(work, 0, sizeof(CAPSELECTWORK));
        work->playerNo = playerNo;
        work->deleteF = FALSE;
        work->extraCapsule = -1;
        storyPartyF = GwSystem.partyF;
        if (!storyPartyF) {
            CapSelectStoryFSet(TRUE);
        } else {
            CapSelectStoryFSet(FALSE);
        }
        CapSelectExtraCapsuleGet(playerNo, -1);
        ev_CapSelectValue[playerNo] = -9;
        CapSelect(work);
        HuMemDirectFree(work);
        if (ev_CapSelectValue[playerNo] >= 0) {
            if (mbCapUse(playerNo, ev_CapSelectValue[playerNo])) {
                ev_CapSelectValue[playerNo] = -7;
                if (_CheckFlag(FLAGNUM(FLAG_GROUP_COMMON, 29))) {
                    return ev_CapSelectValue[playerNo];
                }
                if (mbPlayerBlackoutGet()) {
                    mbCameraPlayerViewSetFast(playerNo, 0);
                    mbCameraMoveWait();
                    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
                    mbWipeWait();
                    mbPlayerBlackoutSet(FALSE);
                }
                break;
            }
        } else {
            switch (ev_CapSelectValue[playerNo]) {
            case -3:
                mbev_Scroll(playerNo, FALSE);
                break;
            case -4:
                mbev_Scroll(playerNo, TRUE);
                break;
            default:
                goto cleanup;
            }
        }
        HuPrcVSleep();
    }
cleanup:
    while (!mbCapSelectShrinkCheck(playerNo)) {
        HuPrcVSleep();
    }
    mbCapSelectResultGet(playerNo, &objId, &result);
    if (objId != -1 && result != -1) {
        mbCapObjKill(objId);
    }
    return ev_CapSelectValue[playerNo];
}

int mbCapDelete(int capsuleNo, BOOL repeatF)
{
    CAPSELECTWORK *work;
    int winId;
    HuVecF pos;
    HuVecF dustPos;
    CAPSELECTWORK *workData;
    int playerNo = GwSystem.turnPlayerNo;
    int objId;
    int result;
    OMOBJ *effectObj;
    HuVecF *dustPosP;
    BOOL partyF;

    mbCapSelectResultSet(playerNo, -1, -1);
    ev_CapSelectMdlId = -1;
    do {
retry:
        while (!mbCapSelectShrinkCheck(playerNo)) {
            HuPrcVSleep();
        }
        workData = HuMemDirectMallocNum(HEAP_HEAP,
            sizeof(CAPSELECTWORK),
            HU_MEMNUM_OVL);
        work = workData;
        memset(work, 0, sizeof(CAPSELECTWORK));
        work->playerNo = playerNo;
        work->deleteF = TRUE;
        work->extraCapsule = capsuleNo;
        CapSelectStoryFSet(FALSE);
        CapSelectExtraCapsuleGet(playerNo, work->extraCapsule);
        ev_CapSelectValue[playerNo] = -9;
        CapSelect(work);
        HuMemDirectFree(work);
        while (!mbCapSelectShrinkCheck(playerNo)) {
            HuPrcVSleep();
        }
        if (ev_CapSelectValue[playerNo] >= 0) {
            winId = mbWinCreateChoice(3,
                MESSNUM(MESS_CAPSULE_MASU, 2), -1, 0);
            mbWinTopInsertMesSet(
                mbCapUseMesGet(ev_CapSelectValue[playerNo]), 0);
            if (GwPlayer[playerNo].comF) {
                mbComChoiceLeftSet();
            }
            mbWinWait(winId);
            if (mbWinTopChoiceGet() == 1 || mbWinTopChoiceGet() == -1) {
                if (repeatF) {
                    ev_CapSelectValue[playerNo] = -9;
                } else {
                    ev_CapSelectValue[playerNo] = -1;
                }
            } else {
                mbCapSelectResultGet(playerNo, &objId, &result);
                mbObjPosGet(objId, &pos);
                mbAudFXPlay(MSM_SE_BRD00_17);
                effectObj = mbev_CapEffExplodeCreate();
                dustPos = pos;
                dustPosP = &dustPos;
                mbev_CapEffDustExplodeAdd(effectObj, dustPosP);
                mbObjDispSet(objId, FALSE);
                while (mbev_CapEffExplodeAnimGet(effectObj) > 0) {
                    HuPrcVSleep();
                }
                mbev_CapEffExplodeKill(effectObj);
                omVibrate(playerNo, 20, 4, 4);
                partyF = GwSystem.partyF;
                if (!partyF) {
                    mbSingleCall(6, 0);
                }
            }
        } else {
            switch (ev_CapSelectValue[playerNo]) {
            case -3:
                mbev_Scroll(playerNo, FALSE);
                goto retry;
            case -4:
                mbev_Scroll(playerNo, TRUE);
                goto retry;
            default:
                goto cleanup;
            }
        }
    } while (ev_CapSelectValue[playerNo] == -9 && repeatF);

cleanup:
    mbCapSelectResultGet(playerNo, &objId, &result);
    if (objId != -1 && result != -1) {
        mbCapObjKill(objId);
    }
    return ev_CapSelectValue[playerNo];
}

static void CapSelect(CAPSELECTWORK *work)
{
    HuVecF temp;
    HuVecF playerPos;
    HuVecF center;
    HuVecF screen[6];
    HuVecF start[6];
    HuVecF end[6];
    HuVec2f descPos;
    HuVec2f helpPos;
    float angleTbl[6];
    float baseAngle;
    float angleStep;
    float weight;
    float scale;
    MBMODELID reuseModelId;
    MBMODELID createModelId;
    BOOL partyF;
    s16 result = -1;
    int j;
    int playerNo = work->playerNo;
    s16 oldSelect = 0;
    s16 helpCapsule;
    s16 capsuleNo;
    s16 oldCapsule;
    int resultObj;
    int resultNo;
    int move;
    int i;

    work->selectNo = oldSelect;
    helpCapsule = CapSelectCapsuleGet(work->playerNo, work->selectNo);
    capsuleNo = helpCapsule;
    oldCapsule = capsuleNo;
    angleStep = 0.0f;
    baseAngle = angleStep;
    for (i = 0; i < 3; i++) {
        work->winId[i] = -1;
    }
    if (GwPlayerConf[work->playerNo].type != 0) {
        work->comSelectNo = CapSelectComGet(work->playerNo, work->deleteF);
        if (work->deleteF && work->comSelectNo < 0) {
            work->comSelectNo = (frand() & CAPSELECT_RANDOM_MASK)
                % mbPlayerCapsuleNumGet(work->playerNo);
        }
        if (!_CheckFlag(FLAG_BOARD_TUTORIAL)) {
            work->comDelay = 30;
        } else {
            work->comDelay = 120;
        }
    } else {
        work->comSelectNo = -2;
    }
    if (!work->deleteF) {
        mbStatusDispFocusSet(work->playerNo, TRUE);
    }
    mbCapSelectResultGet(playerNo, &resultObj, &resultNo);
    ev_CapSelectMdlId = resultObj;
    mbPlayerPosGet(work->playerNo, &playerPos);
    playerPos.y += 100.0f;
    center.x = playerPos.x;
    center.y = playerPos.y;
    center.z = playerPos.z - 125.0f;
    work->capsuleNum = CapSelectNumGet(work->playerNo);
    for (i = 0; i < 6; i++) {
        work->objId[i] = -1;
    }
    if (work->extraCapsule != -1) {
        oldSelect = work->capsuleNum - 1;
        work->selectNo = oldSelect;
        baseAngle = work->selectNo * (360.0f / work->capsuleNum);
    }
    if (resultNo != -1) {
        oldSelect = resultNo;
        work->selectNo = oldSelect;
        baseAngle = work->selectNo * (360.0f / work->capsuleNum);
    }
    helpCapsule = CapSelectCapsuleGet(work->playerNo, work->selectNo);
    capsuleNo = helpCapsule;
    oldCapsule = capsuleNo;
    for (i = 0; i < work->capsuleNum; i++) {
        angleTbl[i] = 360.0f - (i * (360.0f / work->capsuleNum));
        end[i].x = center.x
            + (125.0 * sin((M_PI * (baseAngle + angleTbl[i])) / 180.0));
        end[i].y = center.y + 150.0f;
        end[i].z = center.z
            + (125.0 * cos((M_PI * (baseAngle + angleTbl[i])) / 180.0));
        start[i] = playerPos;
        if (resultObj != -1 && resultNo == i) {
            work->objId[i] = resultObj;
            mbObjLayerSet(work->objId[i], 4);
            reuseModelId = work->objId[i];
            mbObjAttrSet(reuseModelId, HU3D_MOTATTR_LOOP);
            mbObjPosGet(work->objId[i], &start[i]);
        } else {
            work->objId[i] = mbCapObjCreate(
                CapSelectCapsuleGet(work->playerNo, i), FALSE);
            mbObjLayerSet(work->objId[i], 4);
            mbObjCameraSet(work->objId[i], HU3D_CAM1);
            createModelId = work->objId[i];
            mbObjAttrSet(createModelId, HU3D_MOTATTR_LOOP);
            mbObjMotionSpeedSet(work->objId[i], 0.0f);
        }
    }
    for (i = 0; i < 2; i++) {
        work->arrowSprId[i] = espEntry(CAPSELECT_DATA_ARROW,
            CAPSELECT_ARROW_PRIORITY, 0);
        espDispOff(work->arrowSprId[i]);
        espScaleSet(work->arrowSprId[i], 0.5f, 0.5f);
        espDrawNoSet(work->arrowSprId[i], CAPSELECT_ARROW_DRAW_NO);
        if (i == 0) {
            espZRotSet(work->arrowSprId[i], 180.0f);
        } else {
            espZRotSet(work->arrowSprId[i], 360.0f);
        }
    }
    mbAudFXPlay(MSM_SE_BRD00_21);
    for (i = 0; i <= 12U; i++) {
        weight = i / 12.0f;
        for (j = 0; j < work->capsuleNum; j++) {
            if (j == work->selectNo) {
                work->scale[j] = mbCapSelectGrow(&start[j], &end[j],
                    work->objId[j], weight, 1.0f);
            } else {
                work->scale[j] = mbCapSelectGrow(&start[j], &end[j],
                    work->objId[j], weight, 0.75f);
            }
        }
        HuPrcVSleep();
    }

    work->winId[0] = CapHelpWinCreate(helpCapsule, work->deleteF);
    mbWinPosGet(work->winId[0], &helpPos);
    mbWinPosSet(work->winId[0], helpPos.x, 284);
    for (i = 0; i < work->capsuleNum; i++) {
        mbObjPosGet(work->objId[i], &temp);
        Hu3D3Dto2D(&temp, 1, &screen[i]);
    }
    for (i = 0; i < 2; i++) {
        if (i == 0) {
            espPosSet(work->arrowSprId[i],
                screen[work->selectNo].x - 48.0f,
                screen[work->selectNo].y);
        } else {
            espPosSet(work->arrowSprId[i],
                screen[work->selectNo].x + 48.0f,
                screen[work->selectNo].y);
        }
        if (work->capsuleNum > 1) {
            espDispOn(work->arrowSprId[i]);
        }
    }
    work->winId[1] = mbCapDescWinCreate(capsuleNo);
    mbWinPosGet(work->winId[1], &descPos);
    work->descWinIndex = 1;
    for (;;) {
        if (work->capsuleNum > 1) {
            for (i = 0; i < 2; i++) {
                espDispOn(work->arrowSprId[i]);
            }
        }
        if (GwPlayerConf[work->playerNo].type != 0 && work->comSelectNo == -2) {
            work->comSelectNo = CapSelectComGet(work->playerNo, work->deleteF);
            work->comDelay = 30;
        }
        move = CapSelectPadExec(work);
        if (ev_CapSelectValue[playerNo] == -2
            || ev_CapSelectValue[playerNo] == -1
            || ev_CapSelectValue[playerNo] == -3
            || ev_CapSelectValue[playerNo] == -4
            || ev_CapSelectValue[playerNo] == -5
            || ev_CapSelectValue[playerNo] == -7) {
            break;
        }
        if (ev_CapSelectValue[playerNo] == -8) {
            ev_CapSelectValue[playerNo] =
                CapSelectCapsuleGet(work->playerNo, work->selectNo);
            break;
        }
        capsuleNo = CapSelectCapsuleGet(work->playerNo, work->selectNo);
        if (work->selectNo != oldSelect || capsuleNo != oldCapsule) {
            mbObjMotionTimeSet(work->objId[oldSelect], 0.0f);
            mbObjMotionSpeedSet(work->objId[oldSelect], 0.0f);
            scale = work->scale[oldSelect];
            mbObjScaleSet(work->objId[oldSelect], scale, scale, scale);
            work->winId[work->descWinIndex + 1] =
                mbCapDescWinCreate(capsuleNo);
            baseAngle = oldSelect * (360.0f / work->capsuleNum);
            if (move > 0) {
                angleStep = 360.0f / work->capsuleNum;
            } else {
                angleStep = -(360.0f / work->capsuleNum);
            }
            for (i = 0; i < 2; i++) {
                espDispOff(work->arrowSprId[i]);
            }
            for (i = 0; i <= 12U; i++) {
                weight = i / 12.0f;
                for (j = 0; j < work->capsuleNum; j++) {
                    end[j].x = center.x
                        + (125.0 * sin((M_PI * (baseAngle + angleTbl[j]
                        + (weight * angleStep))) / 180.0));
                    end[j].y = center.y + 150.0f;
                    end[j].z = center.z
                        + (125.0 * cos((M_PI * (baseAngle + angleTbl[j]
                        + (weight * angleStep))) / 180.0));
                    mbObjPosSetV(work->objId[j], &end[j]);
                    scale = work->scale[j];
                    if (j == work->selectNo) {
                        scale = (0.75f * scale)
                            + (0.25f * (weight * scale));
                    } else if (j == oldSelect) {
                        scale -= 0.25f * (weight * scale);
                    } else {
                        scale = 0.75f * scale;
                    }
                    mbObjScaleSet(work->objId[j], scale, scale, scale);
                }
                if (oldSelect != work->selectNo) {
                    if (move < 0) {
                        mbWinPosSet(work->winId[(work->descWinIndex ^ 1) + 1],
                            descPos.x - (576.0f * weight), descPos.y);
                        mbWinPosSet(work->winId[work->descWinIndex + 1],
                            (descPos.x + 576.0f) - (576.0f * weight),
                            descPos.y);
                    } else {
                        mbWinPosSet(work->winId[(work->descWinIndex ^ 1) + 1],
                            descPos.x + (576.0f * weight), descPos.y);
                        mbWinPosSet(work->winId[work->descWinIndex + 1],
                            (descPos.x - 576.0f) + (576.0f * weight),
                            descPos.y);
                    }
                }
                HuPrcVSleep();
            }
            mbWinKill(work->winId[(work->descWinIndex ^ 1) + 1]);
            work->winId[(work->descWinIndex ^ 1) + 1] = -1;
            work->descWinIndex ^= 1;
            work->pulseAngle = 0;
            oldSelect = work->selectNo;
            oldCapsule = capsuleNo;
        }
        if (CapSelectCapsuleGet(work->playerNo, oldSelect)
            != CAPSELECT_CAPSULE_DICE) {
            mbObjMotionSpeedSet(work->objId[oldSelect], 1.0f);
        }
        if (work->winId[0] >= 0 && helpCapsule != capsuleNo) {
            helpCapsule = capsuleNo;
            if (!work->deleteF) {
                mbWinKill(work->winId[0]);
                work->winId[0] = CapHelpWinCreate(capsuleNo, work->deleteF);
                mbWinPosGet(work->winId[0], &helpPos);
                mbWinPosSet(work->winId[0], helpPos.x, 284);
            }
        }
        scale = work->scale[work->selectNo]
            * (1.0 + (0.2f * fabs(sin(
            (M_PI * ((90.0f * work->pulseAngle) / 12.0f)) / 180.0))));
        mbObjScaleSet(work->objId[work->selectNo], scale, scale, scale);
        for (i = 0; i < 2; i++) {
            scale = 0.5 + (0.1f * fabs(sin(
                (M_PI * ((90.0f * work->pulseAngle) / 12.0f)) / 180.0)));
            espScaleSet(work->arrowSprId[i], scale, scale);
        }
        work->pulseAngle++;
        HuPrcVSleep();
    }

    for (i = 0; i < 2; i++) {
        espDispOff(work->arrowSprId[i]);
    }
    for (i = 0; i < work->capsuleNum; i++) {
        mbObjPosGet(work->objId[i], &end[i]);
        start[i] = playerPos;
    }
    if (ev_CapSelectValue[playerNo] >= 0) {
        mbCapSelectResultSet(playerNo, work->objId[work->selectNo],
            work->selectNo);
        start[work->selectNo].x = playerPos.x;
        start[work->selectNo].y = playerPos.y + 150.0f;
        start[work->selectNo].z = playerPos.z;
    } else {
        mbCapSelectResultSet(playerNo, -1, -1);
    }
    mbCapSelectResultGet(playerNo, &resultObj, &resultNo);
    ev_CapSelectMdlId = resultObj;
    mbAudFXPlay(MSM_SE_BRD00_22);
    CapSelectShrinkCreate(playerNo, resultObj, work, start, end);
    for (i = 0; i < 3; i++) {
        if (work->winId[i] >= 0) {
            mbWinKill(work->winId[i]);
            work->winId[i] = -1;
        }
    }
    for (i = 0; i < 2; i++) {
        if (work->arrowSprId[i] >= 0) {
            espKill(work->arrowSprId[i]);
            work->arrowSprId[i] = -1;
        }
    }
    if (_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        mbTutorialCall(CAPSELECT_TUTORIAL_SELECT);
    }
    if (!work->deleteF && ev_CapSelectValue[playerNo] != -3
        && ev_CapSelectValue[playerNo] != -4) {
        partyF = GwSystem.partyF;
        if (partyF) {
            mbStatusDispFocusSet(work->playerNo, FALSE);
        }
    }
}

float mbCapSelectGrow(HuVecF *start, HuVecF *end, MBMODELID modelId,
    float weight, float baseScale)
{
    HuVecF pos;
    float scale = sin((M_PI * (90.0f * weight)) / 180.0);

    pos.y = start->y + (scale * (end->y - start->y));
    pos.x = start->x + (weight * (end->x - start->x));
    pos.z = start->z + (weight * (end->z - start->z));
    if (modelId != ev_CapSelectMdlId) {
        scale = weight;
    } else {
        scale = 1.0f;
    }
    mbObjPosSetV(modelId, &pos);
    mbObjScaleSet(modelId, scale * baseScale, scale * baseScale,
        scale * baseScale);
    return scale;
}

static int CapSelectPadExec(CAPSELECTWORK *work)
{
    int playerNo = work->playerNo;
    int move = 0;
    u16 padNo;
    u16 button;
    u16 buttonCopy;
    u16 buttonDown;
    BOOL partyF;

    if (GwPlayerConf[work->playerNo].type == 0) {
        padNo = GwPlayer[work->playerNo].padNo;
        button = HuPadBtnDown[padNo];
        if (mbPadStkXGet(padNo) < -20) {
            button |= PAD_BUTTON_LEFT;
        } else if (mbPadStkXGet(padNo) > 20) {
            button |= PAD_BUTTON_RIGHT;
        }
    } else {
        button = 0;
        if (--work->comDelay == 0) {
            if (work->comSelectNo == -1) {
                button |= PAD_BUTTON_B;
            } else if (work->comSelectNo == work->selectNo) {
                button |= PAD_BUTTON_A;
            } else if (work->selectNo < work->comSelectNo) {
                button |= PAD_BUTTON_RIGHT;
            } else {
                button |= PAD_BUTTON_LEFT;
            }
            work->comDelay = 20;
        }
    }
    buttonCopy = button;
    buttonDown = buttonCopy;
    if (mbPauseProcCheck()) {
        buttonDown = 0;
        return move;
    }
    if ((buttonDown & PAD_BUTTON_LEFT) && work->capsuleNum > 1) {
        mbAudFXPlay(0);
        if (--work->selectNo < 0) {
            work->selectNo = work->capsuleNum - 1;
        }
        move = -1;
    }
    if ((buttonDown & PAD_BUTTON_RIGHT) && work->capsuleNum > 1) {
        mbAudFXPlay(0);
        if (++work->selectNo >= work->capsuleNum) {
            work->selectNo = 0;
        }
        move = 1;
    }
    if (buttonDown & PAD_BUTTON_A) {
        ev_CapSelectValue[playerNo] = -8;
        mbAudFXPlay(1);
        return move;
    }
    if (buttonDown & PAD_BUTTON_Y) {
        partyF = GwSystem.partyF;
        if (partyF) {
            ev_CapSelectValue[playerNo] = -4;
            mbAudFXPlay(1);
            return move;
        }
    }
    if (buttonDown & PAD_BUTTON_X) {
        ev_CapSelectValue[playerNo] = -3;
        mbAudFXPlay(1);
        return move;
    }
    if (buttonDown & PAD_BUTTON_B) {
        ev_CapSelectValue[playerNo] = -7;
        mbAudFXPlay(3);
        return move;
    }
    return move;
}

void mbCapSelectResultSet(int playerNo, int objId, int result)
{
    ev_CapSelectObjId[playerNo] = objId;
    ev_CapSelectResult[playerNo] = result;
}

void mbCapSelectResultGet(int playerNo, int *objId, int *result)
{
    if (objId != NULL) {
        *objId = ev_CapSelectObjId[playerNo];
    }
    if (result != NULL) {
        *result = ev_CapSelectResult[playerNo];
    }
}

void mbCapSelectResultReset(int playerNo)
{
    ev_CapSelectObjId[playerNo] = -1;
    ev_CapSelectResult[playerNo] = -1;
}

static void CapSelectShrinkCreate(int playerNo, int objId,
    CAPSELECTWORK *selectWork, HuVecF *start, HuVecF *end)
{
    CAPSELECTSHRINKWORK *work;
    CAPSELECTSHRINKWORK *workData;
    int i;

    ev_CapSelectShrinkProc[playerNo] = HuPrcChildCreate(CapSelectShrink,
        CAPSELECT_SHRINK_PRIORITY, CAPSELECT_SHRINK_STACK_SIZE, 0,
        mbMainProc);
    HuPrcDestructorSet2(ev_CapSelectShrinkProc[playerNo],
        CapSelectShrinkDestroy);
    workData = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAPSELECTSHRINKWORK),
        HU_MEMNUM_OVL);
    work = workData;
    ev_CapSelectShrinkProc[playerNo]->property = work;
    memset(work, 0, sizeof(CAPSELECTSHRINKWORK));
    work->playerNo = playerNo;
    work->objId = objId;
    work->count = selectWork->capsuleNum;
    for (i = 0; i < work->count; i++) {
        work->objIdTbl[i] = selectWork->objId[i];
        work->start[i] = start[i];
        work->end[i] = end[i];
    }
}

static inline float CapSelectShrinkScaleGet(void)
{
    return 0.75f;
}

static void CapSelectShrink(void)
{
    CAPSELECTSHRINKWORK *work = HuPrcCurrentGet()->property;
    HuVecF transform;
    float scaleDelta;
    float startRot;
    float weight;
    float selectedScale;
    float rotY;
    int frame;
    int i;

    if (ev_CapSelectMdlId != -1) {
        mbObjScaleGet(ev_CapSelectMdlId, &transform);
        scaleDelta = transform.x - 1.0f;
        mbObjRotGet(ev_CapSelectMdlId, &transform);
        startRot = transform.y;
        if (startRot > 180.0f) {
            startRot -= 360.0f;
        }
    } else {
        scaleDelta = 1.0f;
        startRot = 0.0f;
    }
    for (frame = 0; frame <= 9U; frame++) {
        weight = frame / 9.0f;
        selectedScale = 1.0f + (scaleDelta * (1.0f - weight));
        rotY = startRot * (1.0f - weight);
        for (i = 0; i < work->count; i++) {
            if (work->objIdTbl[i] == ev_CapSelectMdlId) {
                s16 objId = work->objIdTbl[i];
                HuVecF pos;
                float scale;

                scale = sin((M_PI * (90.0f * (1.0f - weight))) / 180.0);
                pos.y = ((float *)&work->start[i])[1]
                    + (scale * (((float *)&work->end[i])[1]
                    - ((float *)&work->start[i])[1]));
                pos.x = (work->start + i)->x
                    + ((1.0f - weight)
                    * ((work->end + i)->x - (work->start + i)->x));
                pos.z = ((float *)&work->start[i])[2]
                    + ((1.0f - weight)
                    * (((float *)&work->end[i])[2]
                    - ((float *)&work->start[i])[2]));
                if (objId != ev_CapSelectMdlId) {
                    scale = 1.0f - weight;
                } else {
                    scale = 1.0f;
                }
                mbObjPosSetV(objId, &pos);
                mbObjScaleSet(objId, scale * selectedScale,
                    scale * selectedScale, scale * selectedScale);
                mbObjRotSet(work->objIdTbl[i], 0.0f,
                    rotY, 0.0f);
    } else {
        s16 objId = work->objIdTbl[i];
        HuVecF pos;
        float scale;

        scale = sin((M_PI * (90.0f * (1.0f - weight))) / 180.0);
        pos.y = ((float *)&work->start[i])[1]
            + (scale * (((float *)&work->end[i])[1]
            - ((float *)&work->start[i])[1]));
        pos.x = (work->start + i)->x
            + ((1.0f - weight)
            * ((work->end + i)->x - (work->start + i)->x));
        pos.z = ((float *)&work->start[i])[2]
            + ((1.0f - weight)
            * (((float *)&work->end[i])[2]
            - ((float *)&work->start[i])[2]));
        if (objId != ev_CapSelectMdlId) {
            scale = 1.0f - weight;
        } else {
            scale = 1.0f;
        }
        mbObjPosSetV(objId, &pos);
        mbObjScaleSet(objId, scale * CapSelectShrinkScaleGet(),
            scale * CapSelectShrinkScaleGet(),
            scale * CapSelectShrinkScaleGet());
    }
        }
        HuPrcVSleep();
    }
    HuPrcEnd();
}

static void CapSelectShrinkDestroy(void)
{
    CAPSELECTSHRINKWORK *work = HuPrcCurrentGet()->property;
    int i;

    for (i = 0; i < work->count; i++) {
        if (work->objIdTbl[i] != work->objId && work->objIdTbl[i] != -1) {
            mbCapObjKill(work->objIdTbl[i]);
        }
    }
    HuMemDirectFree(work);
    ev_CapSelectShrinkProc[work->playerNo] = NULL;
}

BOOL mbCapSelectShrinkCheck(int playerNo)
{
    if (ev_CapSelectShrinkProc[playerNo] == NULL) {
        return TRUE;
    }
    return FALSE;
}

static void CapSelectStoryFSet(BOOL storyF)
{
    ev_CapSelectStoryF = storyF;
}

static void CapSelectExtraCapsuleGet(int playerNo, int capsuleNo)
{
    ev_CapSelectExtra[playerNo] = capsuleNo;
}

static int CapSelectCapsuleGet(int playerNo, int selectNo)
{
    if (ev_CapSelectStoryF) {
        if (selectNo == 0) {
            return 47;
        }
        if (selectNo == 1) {
            return 48;
        }
        return mbPlayerCapsuleGet(playerNo, selectNo - 2);
    }
    if (selectNo == mbPlayerCapsuleNumGet(playerNo)) {
        return ev_CapSelectExtra[playerNo];
    }
    return mbPlayerCapsuleGet(playerNo, selectNo);
}

static int CapSelectNumGet(int playerNo)
{
    if (ev_CapSelectStoryF) {
        return mbPlayerCapsuleNumGet(playerNo) + 2;
    }
    if (ev_CapSelectExtra[playerNo] >= 0) {
        return mbPlayerCapsuleNumGet(playerNo) + 1;
    }
    return mbPlayerCapsuleNumGet(playerNo);
}

static int CapSelectComGet(int playerNo, BOOL deleteF)
{
    int capsule[5];
    int capsuleNum;
    int i;

    capsuleNum = CapSelectNumGet(playerNo);
    for (i = 0; i < capsuleNum; i++) {
        capsule[i] = CapSelectCapsuleGet(playerNo, i);
    }
    if (!deleteF) {
        return mbCapSelectComGet(playerNo, capsule, capsuleNum);
    }
    return mbCapSelectDeleteComGet(playerNo, capsule, capsuleNum);
}

void fn_8019A618(void)
{
}

void fn_8019A61C(void)
{
}

void fn_8019A620(void)
{
}

void fn_8019A624(void)
{
}

void fn_8019A628(void)
{
}

void fn_8019A62C(void)
{
}

void mbCapMasuExec(int playerNo, int masuId)
{
    OMOBJ *obj;
    CAPMASUWORK *work;
    int capsuleNo;
    BOOL partyF;
    int capsuleMax;
    int deleteCapsuleNo;
    int deleteIndex;
    int i;
    int winId;
    BOOL cameraChangedF = FALSE;

    if (GwSystem.turnNo >= GwSystem.turnMax) {
        return;
    }
    for (i = 0; i < 16; i++) {
        obj = ev_CapMasuOMObj[i];
        if (obj != NULL) {
            work = omObjGetDataAs(obj, CAPMASUWORK);
            if (work->masuId == masuId) {
                break;
            }
        }
    }
    work->playerNo = playerNo;
    work->hiddenF = TRUE;
    work->scale = 0.0f;
    mbObjDispSet(work->modelId, FALSE);
    if (!_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        capsuleNo = mbCapMasuNextGet(playerNo);
    } else {
        capsuleNo = mbTutorialCall(CAPSELECT_TUTORIAL_CAPSULE_GET);
        if (capsuleNo < 0) {
            capsuleNo = mbCapMasuNextGet(playerNo);
        }
    }
    deleteCapsuleNo = -1;
    deleteIndex = -1;
    mbMoveNumDispSet(playerNo, FALSE);
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);
    mbPlayerRotateStart(playerNo, 0, 15);
    while (!mbPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    mbCapCapsuleGet(playerNo, capsuleNo);
    partyF = GwSystem.partyF;
    if (!partyF) {
        mbSingleCall(5, capsuleNo);
    }
    capsuleMax = mbPlayerCapsuleMaxGet();
    if (mbPlayerCapsuleNumGet(playerNo) >= capsuleMax) {
        winId = mbWinCreate(2, MESSNUM(MESS_CAPSULE_MASU, 0), -1);
        mbWinTopInsertMesSet(mbCapUseMesGet(capsuleNo), 0);
        mbWinWait(winId);
        mbCameraPlayerViewSet(playerNo, 0);
        cameraChangedF = TRUE;
        winId = mbWinCreate(2, MESSNUM(MESS_CAPSULE_MASU, 1), -1);
        mbWinTopInsertMesSet(mbCapUseMesGet(capsuleNo), 0);
        mbWinWait(winId);
        do {
            deleteCapsuleNo = mbCapDelete(capsuleNo, TRUE);
            switch (deleteCapsuleNo) {
            default:
                if (deleteCapsuleNo != capsuleNo) {
                    for (i = 0; i < mbPlayerCapsuleMaxGet(); i++) {
                        if (deleteCapsuleNo
                            == mbPlayerCapsuleGet(playerNo, i)) {
                            deleteIndex = i;
                        }
                    }
                    if (deleteIndex != -1) {
                        mbPlayerCapsuleRemove(playerNo, deleteIndex);
                    }
                } else {
                    deleteCapsuleNo = -1;
                    deleteIndex = -2;
                }
                break;
            case -3:
                mbev_Scroll(playerNo, FALSE);
                deleteIndex = -1;
                break;
            case -4:
                mbev_Scroll(playerNo, TRUE);
                deleteIndex = -1;
                break;
            case -7:
                deleteIndex = -2;
                break;
            }
        } while (deleteIndex == -1);
        if (deleteIndex >= 0) {
            mbPlayerCapsuleAdd(playerNo, capsuleNo);
            mbCapNumInc(capsuleNo, FALSE);
            mbPlayerWinLoseVoicePlay(playerNo, 12,
                MSM_SE_CHARVOICE_MARIO + 6);
            mbPlayerMotionShiftSet(playerNo, 12, 0.0f, 8.0f, 0);
            winId = mbWinCreate(2, MESSNUM(MESS_CAPSULE_MASU, 3), -1);
            mbWinTopInsertMesSet(mbCapUseMesGet(deleteCapsuleNo), 0);
            mbWinTopInsertMesSet(mbCapUseMesGet(capsuleNo), 1);
            mbWinWait(winId);
        } else {
            winId = mbWinCreate(2, MESSNUM(MESS_CAPSULE_MASU, 4), -1);
            mbWinTopInsertMesSet(mbCapUseMesGet(capsuleNo), 0);
            mbWinWait(winId);
        }
    } else {
        mbPlayerCapsuleAdd(playerNo, capsuleNo);
        mbCapNumInc(capsuleNo, FALSE);
        mbPlayerWinLoseVoicePlay(playerNo, 12, MSM_SE_CHARVOICE_MARIO + 6);
        mbPlayerMotionShiftSet(playerNo, 12, 0.0f, 8.0f, 0);
        winId = mbWinCreate(2, MESSNUM(MESS_CAPSULE_MASU, 0), -1);
        mbWinTopInsertMesSet(mbCapUseMesGet(capsuleNo), 0);
        mbWinWait(winId);
        while (!mbPlayerMotionEndCheck(playerNo)
            || mbObjMotionShiftIDGet(mbPlayerObjIDGet(playerNo)) != -1) {
            HuPrcVSleep();
        }
    }
    if (_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        mbTutorialCall(CAPSELECT_TUTORIAL_CAPSULE_END);
    }
    mbMoveNumDispSet(playerNo, TRUE);
    if (cameraChangedF) {
        mbCameraPlayerViewSet(playerNo, 2);
    }
}

void mbCapMasuObjInit(void)
{
    int i;

    for (i = 0; i < 16; i++) {
        ev_CapMasuOMObj[i] = NULL;
    }
    mbCapMasuObjCreateAll();
    ev_CapMasuDispF = TRUE;
}

void mbCapMasuObjClose(void)
{
    int i;

    for (i = 0; i < 16; i++) {
        ev_CapMasuOMObj[i] = NULL;
    }
}

void mbCapMasuObjCreateAll(void)
{
    int masuId;

    for (masuId = 1; masuId <= mbMasuRawNumGet(); masuId++) {
        if (mbMasuTypeGet(masuId) == 8) {
            mbCapMasuObjCreate(masuId);
        }
    }
}

void mbCapMasuObjCreate(int masuId)
{
    OMOBJ *obj;
    CAPMASUWORK *work;
    int objNo;
    int i;
    MBMODELID modelId;

    for (objNo = 0; objNo < 16; objNo++) {
        if (ev_CapMasuOMObj[objNo] == NULL) {
            break;
        }
    }
    obj = ev_CapMasuOMObj[objNo] = omAddObjEx(mbObjMan,
        CAPMASU_OBJECT_PRIORITY, 0, 0, -1,
        CapMasuOMExec);
    work = obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAPMASUWORK),
        HU_MEMNUM_OVL);
    memset(work, 0, sizeof(CAPMASUWORK));
    work->objNo = objNo;
    work->masuId = masuId;
    work->modelId = mbObjCreate(CAPSELECT_DATA_MASU_MODEL, NULL, TRUE);
    mbObjLayerSet(work->modelId, 3);
    modelId = work->modelId;
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    work->angle = frand() & CAPSELECT_RANDOM_MASK;
    work->playerNo = -1;
    work->hiddenF = FALSE;
    work->scale = 1.0f;
    mbMasuPosGet(work->masuId, &work->pos);
    mbObjPosSetV(work->modelId, &work->pos);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (work->masuId == GwPlayer[i].masuId) {
            break;
        }
    }
    if (i < GW_PLAYER_MAX || GwSystem.turnNo >= GwSystem.turnMax) {
        work->hiddenF = TRUE;
        work->scale = 0.0f;
        mbObjDispSet(work->modelId, FALSE);
    }
}

void mbCapCapsuleGet(int playerNo, int capsuleNo)
{
    int objId;
    int i;
    HuVecF playerPos;
    HuVecF pos;
    float time;
    float scale;
    float sinTime;

    objId = mbCapObjCreate(capsuleNo, FALSE);
    mbObjAttrSet(objId, HU3D_MOTATTR_LOOP);
    mbObjCameraSet(objId, HU3D_CAM1);
    mbObjLayerSet(objId, 4);
    mbPlayerPosGet(playerNo, &playerPos);
    playerPos.y += 250.0f;
    mbObjPosSetV(objId, &playerPos);
    for (i = 0; i < 15.0f; i++) {
        time = i / 15.0f;
        mbObjScaleSet(objId, time, time, time);
        HuPrcVSleep();
    }
    mbObjScaleSet(objId, 1.0f, 1.0f, 1.0f);
    HuPrcSleep(15);
    mbAudFXPlay(MSM_SE_BRD00_36);
    for (i = 0; i < 33.0f; i++) {
        time = i / 33.0f;
        scale = cos((M_PI * (90.0f * time)) / 180.0);
        sinTime = sin((M_PI * (90.0f * time)) / 180.0);
        pos.x = playerPos.x + (2.0 * (100.0
            * (sin((M_PI * (270.0f * sinTime)) / 180.0)
            * sin((M_PI * (180.0f * time)) / 180.0))));
        pos.z = playerPos.z + (2.0 * (100.0
            * (cos((M_PI * (270.0f * sinTime)) / 180.0)
            * sin((M_PI * (180.0f * time)) / 180.0))));
        pos.y = playerPos.y - (1.5 * (100.0
            * sin((M_PI * (90.0f * time)) / 180.0)));
        mbObjPosSetV(objId, &pos);
        mbObjScaleSet(objId, scale, scale, scale);
        HuPrcVSleep();
    }
    mbAudFXPlay(MSM_SE_BRD00_141);
    mbCapObjKill(objId);
}

void mbCapMasuDispSet(BOOL dispF)
{
    ev_CapMasuDispF = dispF;
}

static void CapMasuOMExec(OMOBJ *obj)
{
    CAPMASUWORK *work = omObjGetDataAs(obj, CAPMASUWORK);
    int i;

    if (mbExitCheck() || ev_CapMasuOMObj[work->objNo] == NULL) {
        omDelObjEx(mbObjMan, obj);
        return;
    }
    if (work->hiddenF) {
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (work->masuId == GwPlayer[i].masuId) {
                break;
            }
        }
        if (i >= GW_PLAYER_MAX && GwSystem.turnNo < GwSystem.turnMax
            && ev_CapMasuDispF) {
            work->scale += 0.033333335f;
            mbObjDispSet(work->modelId, TRUE);
            if (work->scale >= 1.0f) {
                work->hiddenF = FALSE;
                work->scale = 1.0f;
            }
        }
    }
    if (GwSystem.turnNo >= GwSystem.turnMax) {
        ev_CapMasuDispF = FALSE;
    }
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (work->masuId == GwPlayer[i].masuIdNext
            && GwSystem.turnPlayerNo != i) {
            break;
        }
    }
    if (i < GW_PLAYER_MAX || !ev_CapMasuDispF) {
        work->scale -= 0.06666667f;
        mbObjDispSet(work->modelId, TRUE);
        if (work->scale <= 0.0f) {
            mbObjDispSet(work->modelId, FALSE);
            work->hiddenF = TRUE;
            work->scale = 0.0f;
        }
    }
    mbMasuPosGet(work->masuId, &work->pos);
    work->pos.y += (100.0f * work->scale)
        + (work->scale * (10.0 * sin((M_PI * work->angle) / 180.0)));
    mbObjPosSetV(work->modelId, &work->pos);
    mbObjScaleSet(work->modelId, work->scale, work->scale, work->scale);
    if ((work->angle += 2) > 360) {
        work->angle -= 360;
    }
}

static int CapHelpWinCreate(int capsuleNo, BOOL deleteF)
{
    int winId;

    if (!deleteF) {
        BOOL partyF = GwSystem.partyF;

        if (partyF) {
            switch (mbCapUseModeGet(capsuleNo)) {
                case 0:
                    winId = mbWinCreateHelp(
                        MESSNUM(MESS_CAPSULE_EX99, 44));
                    break;
                case 1:
                case 2:
                    winId = mbWinCreateHelp(
                        MESSNUM(MESS_CAPSULE_EX99, 45));
                    break;
                case 3:
                    winId = mbWinCreateHelp(
                        MESSNUM(MESS_CAPSULE_EX99, 46));
                    break;
            }
        } else {
            switch (mbCapUseModeGet(capsuleNo)) {
                case 0:
                    winId = mbWinCreateHelp(
                        MESSNUM(MESS_CAPSULE_EX99, 48));
                    break;
                case 1:
                case 2:
                    winId = mbWinCreateHelp(
                        MESSNUM(MESS_CAPSULE_EX99, 49));
                    break;
                case 3:
                    winId = mbWinCreateHelp(
                        MESSNUM(MESS_CAPSULE_EX99, 50));
                    break;
            }
        }
    } else {
        BOOL partyF = GwSystem.partyF;

        if (partyF) {
            winId = mbWinCreateHelp(MESSNUM(MESS_CAPSULE_EX99, 47));
        } else {
            winId = mbWinCreateHelp(MESSNUM(MESS_CAPSULE_EX99, 51));
        }
    }
    mbWinAttrSet(winId, HUWIN_ATTR_ALIGN_CENTER);
    return winId;
}
