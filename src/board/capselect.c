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
#include "game/hsfex.h"
#include "game/memory.h"
#include "game/pad.h"
#include "game/wipe.h"

#include "dolphin/math.h"
#include "string.h"

#define M_PI 3.141592653589793

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
extern s16 mbCapDescWinCreate(int capsuleNo);
extern int mbSingleCall(int mode, int arg);
extern void mbev_Scroll(int playerNo, BOOL mapF);
extern OMOBJ *mbev_CapEffExplodeCreate(void);
extern void mbev_CapEffDustExplodeAdd(OMOBJ *obj, HuVecF *pos);
extern int mbev_CapEffExplodeAnimGet(OMOBJ *obj);
extern void mbev_CapEffExplodeKill(OMOBJ *obj);

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
        work = HuMemDirectMallocNum(HEAP_HEAP,
            sizeof(CAPSELECTWORK),
            HU_MEMNUM_OVL);
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
                if (_CheckFlag(0x1001D)) {
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
    HuVecF pos;
    HuVecF dustPos;
    int playerNo = GwSystem.turnPlayerNo;
    int objId;
    int result;
    OMOBJ *effectObj;
    int choice;
    s16 winId;

    mbCapSelectResultSet(playerNo, -1, -1);
    ev_CapSelectMdlId = -1;
    do {
        do {
            while (!mbCapSelectShrinkCheck(playerNo)) {
                HuPrcVSleep();
            }
            work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAPSELECTWORK),
                HU_MEMNUM_OVL);
            memset(work, 0, sizeof(CAPSELECTWORK));
            work->playerNo = playerNo;
            work->deleteF = TRUE;
            work->extraCapsule = capsuleNo;
            CapSelectStoryFSet(FALSE);
            CapSelectExtraCapsuleGet(playerNo, capsuleNo);
            ev_CapSelectValue[playerNo] = -9;
            CapSelect(work);
            HuMemDirectFree(work);
            while (!mbCapSelectShrinkCheck(playerNo)) {
                HuPrcVSleep();
            }
            result = ev_CapSelectValue[playerNo];
            if (result == -3) {
                mbev_Scroll(playerNo, FALSE);
            } else if (result == -4) {
                mbev_Scroll(playerNo, TRUE);
            } else if (result < 0) {
                goto cleanup;
            }
        } while (result < 0);

        winId = mbWinCreateChoice(3, 0x3A0002, -1, 0);
        mbWinTopInsertMesSet(mbCapUseMesGet(result), 0);
        if (GwPlayer[playerNo].comF) {
            mbComChoiceLeftSet();
        }
        mbWinWait(winId);
        choice = mbWinTopChoiceGet();
        if (choice == 1 || mbWinTopChoiceGet() == -1) {
            ev_CapSelectValue[playerNo] = repeatF ? -9 : -1;
        } else {
            mbCapSelectResultGet(playerNo, &objId, &result);
            mbObjPosGet(objId, &pos);
            mbAudFXPlay(0x3FD);
            effectObj = mbev_CapEffExplodeCreate();
            dustPos = pos;
            mbev_CapEffDustExplodeAdd(effectObj, &dustPos);
            mbObjDispSet(objId, FALSE);
            while (mbev_CapEffExplodeAnimGet(effectObj) > 0) {
                HuPrcVSleep();
            }
            mbev_CapEffExplodeKill(effectObj);
            omVibrate(playerNo, 20, 4, 4);
            if (!GwSystem.partyF) {
                mbSingleCall(6, 0);
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
    HuVecF playerPos;
    HuVecF center;
    HuVecF start[6];
    HuVecF end[6];
    HuVecF screen[6];
    HuVec2f helpPos;
    HuVec2f descPos;
    float angleTbl[6];
    float baseAngle = 0.0f;
    float angleStep;
    float weight;
    float scale;
    float pulse;
    int playerNo = work->playerNo;
    int oldSelect = 0;
    int oldCapsule;
    int helpCapsule;
    int capsuleNo;
    int resultObj;
    int resultNo;
    int move;
    int frame;
    int i;

    work->selectNo = 0;
    oldCapsule = CapSelectCapsuleGet(playerNo, work->selectNo);
    for (i = 0; i < 3; i++) {
        work->winId[i] = -1;
    }
    if (GwPlayerConf[playerNo].type == 0) {
        work->comSelectNo = -2;
    } else {
        work->comSelectNo = CapSelectComGet(playerNo, work->deleteF);
        if (work->deleteF && work->comSelectNo < 0) {
            int num = mbPlayerCapsuleNumGet(playerNo);
            int value = frand() & 0x7FFF;

            work->comSelectNo = value - ((value / num) * num);
        }
        work->comDelay = _CheckFlag(FLAG_BOARD_TUTORIAL) ? 120 : 30;
    }
    if (!work->deleteF) {
        mbStatusDispFocusSet(playerNo, TRUE);
    }
    mbCapSelectResultGet(playerNo, &resultObj, &resultNo);
    ev_CapSelectObjId[0] = resultObj;
    mbPlayerPosGet(playerNo, &playerPos);
    center.x = playerPos.x;
    center.y = playerPos.y + 100.0f;
    center.z = playerPos.z - 125.0f;
    work->capsuleNum = CapSelectNumGet(playerNo);
    for (i = 0; i < 6; i++) {
        work->objId[i] = -1;
    }
    if (work->extraCapsule != -1) {
        work->selectNo = work->capsuleNum - 1;
        baseAngle = work->selectNo * (360.0f / work->capsuleNum);
    }
    if (resultNo != -1) {
        work->selectNo = resultNo;
        baseAngle = work->selectNo * (360.0f / work->capsuleNum);
    }
    oldSelect = work->selectNo;
    oldCapsule = CapSelectCapsuleGet(playerNo, oldSelect);
    helpCapsule = oldCapsule;
    for (i = 0; i < work->capsuleNum; i++) {
        angleTbl[i] = 360.0f - (i * (360.0f / work->capsuleNum));
        end[i].x = center.x
            + (125.0 * sin((M_PI * (baseAngle + angleTbl[i])) / 180.0));
        end[i].y = center.y + 150.0f;
        end[i].z = center.z
            + (125.0 * cos((M_PI * (baseAngle + angleTbl[i])) / 180.0));
        start[i] = playerPos;
        if (resultObj == -1 || resultNo != i) {
            work->objId[i] = mbCapObjCreate(
                CapSelectCapsuleGet(playerNo, i), FALSE);
            mbObjLayerSet(work->objId[i], 4);
            mbObjCameraSet(work->objId[i], HU3D_CAM1);
            mbObjAttrSet(work->objId[i], 0x40000001);
            mbObjMotionSpeedSet(work->objId[i], 0.0f);
        } else {
            work->objId[i] = resultObj;
            mbObjLayerSet(work->objId[i], 4);
            mbObjAttrSet(work->objId[i], 0x40000001);
            mbObjPosGet(work->objId[i], &start[i]);
        }
    }
    for (i = 0; i < 2; i++) {
        work->arrowSprId[i] = espEntry(DATANUM(DATA_capsule, 0x26),
            0x578, 0);
        espDispOff(work->arrowSprId[i]);
        espScaleSet(work->arrowSprId[i], 0.5f, 0.5f);
        espDrawNoSet(work->arrowSprId[i], 0x20);
        espZRotSet(work->arrowSprId[i], i == 0 ? 180.0f : 360.0f);
    }
    mbAudFXPlay(0x401);
    for (frame = 0; frame < 13; frame++) {
        weight = frame / 12.0f;
        for (i = 0; i < work->capsuleNum; i++) {
            work->scale[i] = mbCapSelectGrow(&start[i], &end[i],
                work->objId[i], weight,
                i == work->selectNo ? 1.0f : 0.75f);
        }
        HuPrcVSleep();
    }

    capsuleNo = CapSelectCapsuleGet(playerNo, work->selectNo);
    work->winId[0] = CapHelpWinCreate(capsuleNo, work->deleteF);
    mbWinPosGet(work->winId[0], &helpPos);
    mbWinPosSet(work->winId[0], helpPos.x, 284);
    for (i = 0; i < work->capsuleNum; i++) {
        mbObjPosGet(work->objId[i], &end[i]);
        Hu3D3Dto2D(&end[i], 1, &screen[i]);
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
            espDispOn(work->arrowSprId[0]);
            espDispOn(work->arrowSprId[1]);
        }
        if (GwPlayerConf[playerNo].type != 0 && work->comSelectNo == -2) {
            work->comSelectNo = CapSelectComGet(playerNo, work->deleteF);
            work->comDelay = 30;
        }
        move = CapSelectPadExec(work);
        resultNo = ev_CapSelectValue[playerNo];
        if (resultNo == -2 || resultNo == -1 || resultNo == -3
            || resultNo == -4 || resultNo == -5 || resultNo == -7) {
            break;
        }
        if (resultNo == -8) {
            ev_CapSelectValue[playerNo] =
                CapSelectCapsuleGet(playerNo, work->selectNo);
            break;
        }
        capsuleNo = CapSelectCapsuleGet(playerNo, work->selectNo);
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
            espDispOff(work->arrowSprId[0]);
            espDispOff(work->arrowSprId[1]);
            for (frame = 0; frame < 13; frame++) {
                weight = frame / 12.0f;
                for (i = 0; i < work->capsuleNum; i++) {
                    float angle = baseAngle + angleTbl[i]
                        + (weight * angleStep);

                    end[i].x = center.x
                        + (125.0 * sin((M_PI * angle) / 180.0));
                    end[i].y = center.y + 150.0f;
                    end[i].z = center.z
                        + (125.0 * cos((M_PI * angle) / 180.0));
                    mbObjPosSetV(work->objId[i], &end[i]);
                    scale = work->scale[i];
                    if (i == work->selectNo) {
                        scale = (0.75f * scale)
                            + (0.25f * (weight * scale));
                    } else if (i == oldSelect) {
                        scale -= 0.25f * (weight * scale);
                    } else {
                        scale *= 0.75f;
                    }
                    mbObjScaleSet(work->objId[i], scale, scale, scale);
                }
                if (oldSelect != work->selectNo) {
                    int oldX;
                    int newX;

                    if (move < 0) {
                        oldX = descPos.x - (576.0f * weight);
                        newX = (descPos.x + 576.0f)
                            - (576.0f * weight);
                    } else {
                        oldX = descPos.x + (576.0f * weight);
                        newX = (descPos.x - 576.0f)
                            + (576.0f * weight);
                    }
                    mbWinPosSet(work->winId[(work->descWinIndex ^ 1) + 1],
                        oldX, descPos.y);
                    mbWinPosSet(work->winId[work->descWinIndex + 1],
                        newX, descPos.y);
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
        if (CapSelectCapsuleGet(playerNo, oldSelect) != 0x2F) {
            mbObjMotionSpeedSet(work->objId[oldSelect], 1.0f);
        }
        if (work->winId[0] >= 0 && helpCapsule != capsuleNo
            && !work->deleteF) {
            helpCapsule = capsuleNo;
            mbWinKill(work->winId[0]);
            work->winId[0] = CapHelpWinCreate(capsuleNo, work->deleteF);
            mbWinPosGet(work->winId[0], &helpPos);
            mbWinPosSet(work->winId[0], helpPos.x, 284);
        }
        pulse = fabs(sin((M_PI * ((90.0f * work->pulseAngle) / 12.0f))
            / 180.0));
        scale = work->scale[work->selectNo] * (1.0 + (0.2 * pulse));
        mbObjScaleSet(work->objId[work->selectNo], scale, scale, scale);
        for (i = 0; i < 2; i++) {
            scale = 0.5 + (0.1 * pulse);
            espScaleSet(work->arrowSprId[i], scale, scale);
        }
        work->pulseAngle++;
        HuPrcVSleep();
    }

    espDispOff(work->arrowSprId[0]);
    espDispOff(work->arrowSprId[1]);
    for (i = 0; i < work->capsuleNum; i++) {
        mbObjPosGet(work->objId[i], &end[i]);
        start[i] = playerPos;
    }
    if (ev_CapSelectValue[playerNo] < 0) {
        mbCapSelectResultSet(playerNo, -1, -1);
    } else {
        mbCapSelectResultSet(playerNo, work->objId[work->selectNo],
            work->selectNo);
        start[work->selectNo].y += 150.0f;
    }
    mbCapSelectResultGet(playerNo, &resultObj, &resultNo);
    ev_CapSelectMdlId = resultObj;
    mbAudFXPlay(0x402);
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
        mbTutorialCall(0x10);
    }
    if (!work->deleteF && ev_CapSelectValue[playerNo] != -3
        && ev_CapSelectValue[playerNo] != -4 && GwSystem.partyF) {
        mbStatusDispFocusSet(playerNo, FALSE);
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
    int padNo;
    u16 button;

    if (GwPlayerConf[playerNo].type == 0) {
        padNo = GwPlayer[playerNo].padNo;
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
                button = PAD_BUTTON_B;
            } else if (work->comSelectNo == work->selectNo) {
                button = PAD_BUTTON_A;
            } else if (work->selectNo < work->comSelectNo) {
                button = PAD_BUTTON_RIGHT;
            } else {
                button = PAD_BUTTON_LEFT;
            }
            work->comDelay = 20;
        }
    }
    if (mbPauseProcCheck()) {
        return 0;
    }
    if ((button & PAD_BUTTON_LEFT) && work->capsuleNum > 1) {
        mbAudFXPlay(0);
        if (--work->selectNo < 0) {
            work->selectNo = work->capsuleNum - 1;
        }
        move = -1;
    }
    if ((button & PAD_BUTTON_RIGHT) && work->capsuleNum > 1) {
        mbAudFXPlay(0);
        if (++work->selectNo >= work->capsuleNum) {
            work->selectNo = 0;
        }
        move = 1;
    }
    if (button & PAD_BUTTON_A) {
        ev_CapSelectValue[playerNo] = -8;
        mbAudFXPlay(1);
    } else if ((button & PAD_BUTTON_Y) && GwSystem.partyF) {
        ev_CapSelectValue[playerNo] = -4;
        mbAudFXPlay(1);
    } else if (button & PAD_BUTTON_X) {
        ev_CapSelectValue[playerNo] = -3;
        mbAudFXPlay(1);
    } else if (button & PAD_BUTTON_B) {
        ev_CapSelectValue[playerNo] = -7;
        mbAudFXPlay(3);
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
        0x2005, 0x4000, 0, mbMainProc);
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

static void CapSelectShrink(void)
{
    CAPSELECTSHRINKWORK *work = HuPrcCurrentGet()->property;
    HuVecF pos;
    HuVecF scale;
    HuVecF rot;
    float scaleDelta;
    float startRot;
    float weight;
    float invWeight;
    float objScale;
    float selectedScale;
    int frame;
    int i;

    if (ev_CapSelectMdlId == -1) {
        scaleDelta = 1.0f;
        startRot = 0.0f;
    } else {
        mbObjScaleGet(ev_CapSelectMdlId, &scale);
        scaleDelta = scale.x - 1.0f;
        mbObjRotGet(ev_CapSelectMdlId, &rot);
        startRot = rot.y;
        if (startRot > 180.0f) {
            startRot -= 360.0f;
        }
    }
    for (frame = 0; frame < 10; frame++) {
        weight = frame / 9.0f;
        invWeight = 1.0f - weight;
        selectedScale = 1.0f + (scaleDelta * invWeight);
        for (i = 0; i < work->count; i++) {
            if (work->objIdTbl[i] == ev_CapSelectMdlId) {
                pos.y = work->start[i].y
                    + (sin((M_PI * (90.0f * invWeight)) / 180.0)
                    * (work->end[i].y - work->start[i].y));
                pos.x = work->start[i].x
                    + (invWeight * (work->end[i].x - work->start[i].x));
                pos.z = work->start[i].z
                    + (invWeight * (work->end[i].z - work->start[i].z));
                objScale = work->objIdTbl[i] == ev_CapSelectMdlId
                    ? 1.0f : invWeight;
                mbObjPosSetV(work->objIdTbl[i], &pos);
                mbObjScaleSet(work->objIdTbl[i], objScale * selectedScale,
                    objScale * selectedScale, objScale * selectedScale);
                mbObjRotSet(work->objIdTbl[i], 0.0f,
                    startRot * invWeight, 0.0f);
            } else {
                pos.y = work->start[i].y
                    + (sin((M_PI * (90.0f * invWeight)) / 180.0)
                    * (work->end[i].y - work->start[i].y));
                pos.x = work->start[i].x
                    + (invWeight * (work->end[i].x - work->start[i].x));
                pos.z = work->start[i].z
                    + (invWeight * (work->end[i].z - work->start[i].z));
                objScale = work->objIdTbl[i] == ev_CapSelectMdlId
                    ? 1.0f : invWeight;
                mbObjPosSetV(work->objIdTbl[i], &pos);
                mbObjScaleSet(work->objIdTbl[i], objScale * 0.75f,
                    objScale * 0.75f, objScale * 0.75f);
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
        capsuleNo = mbTutorialCall(0x15);
        if (capsuleNo < 0) {
            capsuleNo = mbCapMasuNextGet(playerNo);
        }
    }
    deleteCapsuleNo = -1;
    deleteIndex = -1;
    mbMoveNumDispSet(playerNo, FALSE);
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, 0x40000001);
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
        winId = mbWinCreate(2, 0x3A0000, -1);
        mbWinTopInsertMesSet(mbCapUseMesGet(capsuleNo), 0);
        mbWinWait(winId);
        mbCameraPlayerViewSet(playerNo, 0);
        cameraChangedF = TRUE;
        winId = mbWinCreate(2, 0x3A0001, -1);
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
            mbPlayerWinLoseVoicePlay(playerNo, 12, 0x243);
            mbPlayerMotionShiftSet(playerNo, 12, 0.0f, 8.0f, 0);
            winId = mbWinCreate(2, 0x3A0003, -1);
            mbWinTopInsertMesSet(mbCapUseMesGet(deleteCapsuleNo), 0);
            mbWinTopInsertMesSet(mbCapUseMesGet(capsuleNo), 1);
            mbWinWait(winId);
        } else {
            winId = mbWinCreate(2, 0x3A0004, -1);
            mbWinTopInsertMesSet(mbCapUseMesGet(capsuleNo), 0);
            mbWinWait(winId);
        }
    } else {
        mbPlayerCapsuleAdd(playerNo, capsuleNo);
        mbCapNumInc(capsuleNo, FALSE);
        mbPlayerWinLoseVoicePlay(playerNo, 12, 0x243);
        mbPlayerMotionShiftSet(playerNo, 12, 0.0f, 8.0f, 0);
        winId = mbWinCreate(2, 0x3A0000, -1);
        mbWinTopInsertMesSet(mbCapUseMesGet(capsuleNo), 0);
        mbWinWait(winId);
        while (!mbPlayerMotionEndCheck(playerNo)
            || mbObjMotionShiftIDGet(mbPlayerObjIDGet(playerNo)) != -1) {
            HuPrcVSleep();
        }
    }
    if (_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        mbTutorialCall(0x16);
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
    obj = ev_CapMasuOMObj[objNo] = omAddObjEx(mbObjMan, 0x104, 0, 0, -1,
        CapMasuOMExec);
    work = obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAPMASUWORK),
        HU_MEMNUM_OVL);
    memset(work, 0, sizeof(CAPMASUWORK));
    work->objNo = objNo;
    work->masuId = masuId;
    work->modelId = mbObjCreate(DATANUM(DATA_capsule, 0x21), NULL, TRUE);
    mbObjLayerSet(work->modelId, 3);
    modelId = work->modelId;
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    work->angle = frand() & 0x7FFF;
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
    mbObjAttrSet(objId, 0x40000001);
    mbObjCameraSet(objId, HU3D_CAM1);
    mbObjLayerSet(objId, 4);
    mbPlayerPosGet(playerNo, &playerPos);
    playerPos.y += 250.0f;
    mbObjPosSetV(objId, &playerPos);
    for (i = 0; i < 15.0f; i++) {
        scale = i / 15.0f;
        mbObjScaleSet(objId, scale, scale, scale);
        HuPrcVSleep();
    }
    mbObjScaleSet(objId, 1.0f, 1.0f, 1.0f);
    HuPrcSleep(15);
    mbAudFXPlay(0x410);
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
    mbAudFXPlay(0x479);
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
                    winId = mbWinCreateHelp(0x37002C);
                    break;
                case 1:
                case 2:
                    winId = mbWinCreateHelp(0x37002D);
                    break;
                case 3:
                    winId = mbWinCreateHelp(0x37002E);
                    break;
            }
        } else {
            switch (mbCapUseModeGet(capsuleNo)) {
                case 0:
                    winId = mbWinCreateHelp(0x370030);
                    break;
                case 1:
                case 2:
                    winId = mbWinCreateHelp(0x370031);
                    break;
                case 3:
                    winId = mbWinCreateHelp(0x370032);
                    break;
            }
        }
    } else {
        BOOL partyF = GwSystem.partyF;

        if (partyF) {
            winId = mbWinCreateHelp(0x37002F);
        } else {
            winId = mbWinCreateHelp(0x370033);
        }
    }
    mbWinAttrSet(winId, 0x800);
    return winId;
}
