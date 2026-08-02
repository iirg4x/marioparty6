#define _MATH_H
#include "dolphin/math.h"

#include "game/armem.h"
#include "game/board/audio.h"
#include "game/board/camera.h"
#include "game/board/guide.h"
#include "game/board/main.h"
#include "game/board/player.h"
#include "game/board/tutorial.h"
#include "game/board/window.h"
#include "game/esprite.h"
#include "game/mgdata.h"
#include "game/pad.h"
#include "game/wipe.h"

extern BOOL mbSaveNewF;
extern void mbExitReq(void);
extern void mbPauseEnableSet(void);
extern BOOL mbWipeSpecialCheck(void);
extern BOOL mbWipeSpecialStatGet(void);
extern void mbWipeWait(void);

typedef struct TutorialCallWork_s {
    int scene;
    int callNum;
    int result;
    int stat;
    int mode;
} TUTORIALCALLWORK;

typedef struct TutorialWinData_s {
    s8 stat;
    s8 delay;
    s16 time;
} TUTORIALWINDATA;

typedef struct TutorialGuideData_s {
    int message;
    int seId;
} TUTORIALGUIDEDATA;

typedef struct TutorialMgCallWork_s {
    u8 killF : 1;
    u8 slideInF : 1;
    u8 state : 3;
    u8 cursorNo : 3;
    u8 dispF : 1;
    s16 winNo;
    s16 field04;
    s16 time;
    s16 maxTime;
    int message;
} TUTORIALMGCALLWORK;

typedef struct TutorialMgCallData_s {
    int active;
    int field04;
    s16 espId[3];
    s16 field0E[5];
    OMOBJ *obj[4];
} TUTORIALMGCALLDATA;

static s16 tutorialSprId[16];
static s16 tutorialSprGrpId[16];
static s16 tutorialMdlId[32];
static TUTORIALCALLWORK tutorialCallWork;
static TUTORIALWINDATA tutorialWinData[HUWIN_MAX];
static TUTORIALMGCALLDATA tutorialMgCallData;
static HUPROCESS *tutorialWatchProc;
static HUPROCESS *tutorialMainProc;
static BOOL tutorialExitReqF;
static BOOL tutorialExitOnF;
static s16 tutorialMgCallCursorPos;
static TUTORIALMAINFUNC tutorialMain;
static OMOBJ *tutorialGuideObj;

static TUTORIALGUIDEDATA tutorialGuideTbl[] = {
    { 0x002A0000, 0x3B6 },
    { 0x002A0001, 0x3B8 },
    { 0x002A0007, 0x3B8 },
    { 0x002A0008, 0x3B8 },
    { 0x002A000A, 0x3B6 },
    { 0x002A000B, 0x3B8 },
    { 0x002A000C, 0x3B8 },
    { 0x002A000D, 0x3B8 },
    { 0x002A000E, 0x3B8 },
    { 0x002A000F, 0x3B8 },
    { 0x002A0010, 0x3B8 },
    { 0x002A0011, 0x3B8 },
    { 0x002A0014, 0x3B6 },
    { 0x002A0015, 0x3B8 },
    { 0x002A0016, 0x3B6 },
    { 0x002A0017, 0x3B8 },
    { 0x002A0018, 0x3B6 },
    { 0x002A0019, 0x3B6 },
    { 0x002A001B, 0x3B8 },
    { 0x002A001C, 0x3B8 },
    { 0x002A001D, 0x3B8 },
    { 0x002A001E, 0x3B8 },
    { 0x002A001F, 0x3B6 },
    { 0x002A0020, 0x3B8 },
    { 0x002A0021, 0x3B8 },
    { 0x002A0023, 0x3B6 },
    { 0x002A0024, 0x3B8 },
    { 0x002A0025, 0x3B8 },
    { 0x002A0026, 0x3B8 },
    { 0x002A0027, 0x3B6 },
    { 0x002A0028, 0x3B8 },
    { 0x002A0029, 0x3B6 },
    { 0x002A002A, 0x3B8 },
    { 0x002A002D, 0x3B6 },
    { 0x002A002E, 0x3B8 },
    { 0x002A002F, 0x3B8 },
    { 0x002A0031, 0x3B8 },
    { 0x002A0032, 0x3B6 },
    { 0x002A0033, 0x3B8 },
    { 0x002A0034, 0x3B8 },
    { 0x002A0035, 0x3B6 },
    { 0x002A0036, 0x3B8 },
    { 0x002A0038, 0x3B8 },
    { 0x002A0039, 0x3B8 },
    { 0x002A003A, 0x3B6 },
    { 0x002A003C, 0x3B8 },
    { 0x002A003E, 0x3B8 },
    { 0x002A003F, 0x3B6 },
    { 0x002A0040, 0x3B8 },
    { 0x002A0041, 0x3B8 },
    { 0x002A0042, 0x3B8 },
    { 0x002A0043, 0x3B8 },
    { 0x002A0044, 0x3B8 },
    { 0x002A0045, 0x3B8 },
    { 0x002A0046, 0x3B8 },
    { 0x002A0047, 0x3B8 },
    { 0x002A0049, 0x3B6 },
    { 0x002A004A, 0x3B8 },
    { 0x002A004C, 0x3B6 },
    { 0x002A004D, 0x3B6 },
    { 0x002A004E, 0x3B8 },
    { 0x002A004F, 0x3B6 },
    { 0x002A0050, 0x3B6 },
    { 0x002A0051, 0x3B6 },
    { 0x002A0052, 0x3B8 },
    { 0x002A0054, 0x3B6 },
    { 0x002A0055, 0x3B6 },
    { 0x002A0057, 0x3B6 },
    { 0x002A0058, 0x3B8 },
    { 0x002A0059, 0x3B6 },
    { 0x002A005B, 0x3B8 },
    { 0x002A005C, 0x3B6 },
    { -1, -1 },
};

static int mgCallWinYOfsTbl[5][4] = {
    { 0, 0, 0, 0 },
    { 238, 0, 0, 0 },
    { 214, 262, 0, 0 },
    { 198, 238, 278, 0 },
    { 193, 223, 253, 283 },
};

static void TutorialWatch(void);
static void TutorialSprClose(void);
static void TutorialSprGrpClose(void);
static void TutorialModelKillAll(void);
static void TutorialWinInit(void);
static void TutorialWinUpdate(void);
static void TutorialMgCallOMExec(OMOBJ *obj);
static void TutorialMgCallListGet(int type, int num, s16 *list);
static void TutorialMgCallSlideInSet(OMOBJ *obj);
static BOOL TutorialMgCallSlideInCheck(OMOBJ *obj);
static void TutorialMgCallGrowSet(OMOBJ *obj);
static void TutorialMgCallKill(OMOBJ *obj);

void mbTutorialInit(void)
{
    int i;

    tutorialGuideObj = NULL;
    for (i = 0; i < 16; i++) {
        tutorialSprId[i] = -1;
    }
    for (i = 0; i < 16; i++) {
        tutorialSprGrpId[i] = -1;
    }
    for (i = 0; i < 32; i++) {
        tutorialMdlId[i] = -1;
    }
    mbTutorialMainFuncSet(NULL);
    mbTutorialMgCallInit();
    tutorialExitReqF = FALSE;
    tutorialExitOnF = FALSE;
}

void mbTutorialMainFuncSet(TUTORIALMAINFUNC func)
{
    tutorialMain = func;
}

void mbTutorialCreate(void)
{
    tutorialCallWork.scene = -1;
    tutorialCallWork.callNum = 0;
    tutorialCallWork.result = -1;
    tutorialCallWork.stat = 0;
    if (mbSaveNewF) {
        tutorialCallWork.mode = 0;
    }
    _ClearFlag(FLAG_BOARD_DEBUG);
    _ClearFlag(FLAGNUM(FLAG_GROUP_BOARD, 3));
    _ClearFlag(FLAGNUM(FLAG_GROUP_BOARD, 7));
    _ClearFlag(FLAGNUM(FLAG_GROUP_BOARD, 8));
    _ClearFlag(FLAG_BOARD_NOMG);
    tutorialWatchProc = HuPrcChildCreate(TutorialWatch, 0x2011, 0x2000, 0, mbMainProc);
    tutorialMainProc = HuPrcChildCreate(tutorialMain, 0x2011, 0x2000, 0, mbMainProc);
    HuPrcVSleep();
}

static void TutorialWatch(void)
{
    s16 winNo;
    HuVec2f size;

    HuPrcSleep(60);
    TutorialWinInit();
    winNo = mbWinCreateHelp(0x002A0048);
    mbWinMesMaxSizeGet(winNo, &size);
    mbWinPosSet(winNo, 288.0f - ((s16)size.x / 2), 400);
    tutorialExitOnF = TRUE;
    while (!tutorialExitReqF) {
        u16 btn = HuPadBtnDown[0];

        TutorialWinUpdate();
        mbWinDispSet(winNo, tutorialExitOnF);
        if (!mbWipeSpecialCheck() && !mbWipeSpecialStatGet()
            && (btn & PAD_BUTTON_START) && tutorialExitOnF) {
            tutorialExitReqF = TRUE;
            break;
        }
        HuPrcVSleep();
    }
    mbPauseEnableSet();
    while (HuARDMACheck()) {
        HuPrcVSleep();
    }
    mbMusFadeOutSpeed(MB_MUS_CHAN_BG, 1000);
    if (tutorialExitOnF) {
        BOOL partyF = GwSystem.partyF;

        if (partyF) {
            WipeColorSet(0, 0, 0);
        } else {
            WipeColorSet(0, 0, 0);
        }
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 30);
        mbWipeWait();
    }
    while (mbMusCheck(MB_MUS_CHAN_BG)) {
        HuPrcVSleep();
    }
    mbWinKill(winNo);
    HuPrcKill(tutorialMainProc);
    tutorialMainProc = NULL;
    TutorialSprClose();
    TutorialSprGrpClose();
    TutorialModelKillAll();
    mbTutorialMgCallClose();
    if (tutorialGuideObj) {
        mbTutorialGuideClose(tutorialGuideObj);
    }
    mbExitReq();
    HuPrcSleep(-1);
}

int mbTutorialCall(int scene)
{
    if (!_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        return -1;
    }
    if (tutorialMainProc == NULL) {
        return -1;
    }
    while (tutorialCallWork.stat == 0) {
        HuPrcVSleep();
    }
    if (tutorialCallWork.stat < 0) {
        return -1;
    }
    if (tutorialCallWork.scene >= 0) {
        if (tutorialCallWork.scene != scene) {
            return -1;
        }
        tutorialCallWork.callNum--;
        if (tutorialCallWork.callNum > 0) {
            return -1;
        }
    }
    tutorialCallWork.callNum = scene;
    tutorialCallWork.result = -1;
    tutorialCallWork.stat = 0;
    while (tutorialCallWork.stat == 0) {
        HuPrcVSleep();
    }
    return tutorialCallWork.result;
}

void mbTutorialCallEnd(void)
{
    tutorialCallWork.stat = -1;
}

void mbTutorialVSleep(void)
{
    if (mbTutorialExitReqGet()) {
        HuPrcSleep(-1);
    }
    HuPrcVSleep();
}

void mbTutorialCallWait(int scene)
{
    tutorialCallWork.scene = scene;
    tutorialCallWork.callNum = 0;
    tutorialCallWork.stat = 1;
    while (tutorialCallWork.stat != 0) {
        mbTutorialVSleep();
    }
}

void mbTutorialMultiCall(int scene, int callNum)
{
    tutorialCallWork.scene = scene;
    tutorialCallWork.callNum = callNum;
    tutorialCallWork.stat = 1;
    while (tutorialCallWork.stat != 0) {
        mbTutorialVSleep();
    }
}

void mbTutorialCallResult(int scene, int result)
{
    tutorialCallWork.scene = scene;
    tutorialCallWork.callNum = 0;
    tutorialCallWork.stat = 1;
    while (tutorialCallWork.stat != 0) {
        mbTutorialVSleep();
    }
    mbTutorialResultSet(result);
}

void mbTutorialMesCall(int scene, int message)
{
    tutorialCallWork.scene = scene;
    tutorialCallWork.callNum = 0;
    tutorialCallWork.stat = 1;
    while (tutorialCallWork.stat != 0) {
        mbTutorialVSleep();
    }
    mbTutorialWinMesExec(message);
}

void mbTutorialMesMasuCall(int scene, int message, int masuId)
{
    tutorialCallWork.scene = scene;
    tutorialCallWork.callNum = 0;
    tutorialCallWork.stat = 1;
    while (tutorialCallWork.stat != 0) {
        mbTutorialVSleep();
    }
    mbTutorialWinMesMasuExec(message, masuId);
}

void mbTutorialTurnCall(int turn)
{
    tutorialCallWork.scene = 5;
    tutorialCallWork.callNum = 0;
    tutorialCallWork.stat = 1;
    while (tutorialCallWork.stat != 0) {
        mbTutorialVSleep();
    }
    mbTutorialResultSet(turn - 1);
}

void mbTutorialGuideCall(int guideNo)
{
    tutorialCallWork.scene = 13;
    tutorialCallWork.callNum = 0;
    tutorialCallWork.stat = 1;
    while (tutorialCallWork.stat != 0) {
        mbTutorialVSleep();
    }
    mbTutorialResultSet(guideNo);
}

void mbTutorialCapsuleUseCall(int capsuleNo)
{
    int capsuleIndex;

    tutorialCallWork.scene = 15;
    tutorialCallWork.callNum = 0;
    tutorialCallWork.stat = 1;
    while (tutorialCallWork.stat != 0) {
        mbTutorialVSleep();
    }
    capsuleIndex = mbPlayerCapsuleFind(GwSystem.turnPlayerNo, capsuleNo);
    mbTutorialResultSet(capsuleIndex);
}

void mbTutorialResultSet(int result)
{
    tutorialCallWork.result = result;
}

int mbTutorialSceneGet(void)
{
    return tutorialCallWork.scene;
}

void mbTutorialModeSet(int mode)
{
    tutorialCallWork.mode = mode;
}

int mbTutorialModeGet(void)
{
    return tutorialCallWork.mode;
}

s16 mbTutorialViewSet(void)
{
    HuVecF rot = { -35.0f, 0.0f, 0.0f };
    HuVecF offset = { 0.0f, 200.0f, 0.0f };
    int masuId;

    masuId = (s16)mbMasuFind_AttrIdGet(-1, 0x8000);
    mbCameraMoveMasu(masuId, &rot, &offset, mbCameraPlayerViewZoomGet(0), -1.0f, -1);
    mbCameraMoveOnSet(FALSE);
    return masuId;
}

void mbTutorialViewMasuSet(s16 masuId)
{
    HuVecF rot = { -35.0f, 0.0f, 0.0f };
    HuVecF offset = { 0.0f, 50.0f, 0.0f };

    mbCameraMoveMasu(masuId, &rot, &offset, mbCameraPlayerViewZoomGet(0), -1.0f, -1);
    mbCameraMoveOnSet(FALSE);
}

s16 mbTutorialSprCreate(unsigned int dataNum)
{
    s16 sprId;
    int i;

    for (i = 0; i < 16; i++) {
        if (tutorialSprId[i] < 0) {
            break;
        }
    }
    sprId = tutorialSprId[i] = espEntry(dataNum, 1500, 0);
    espAttrSet(sprId, HUSPR_ATTR_LINEAR);
    return sprId;
}

s16 mbTutorialSprDispOn(unsigned int dataNum)
{
    s16 sprId;
    int i;

    sprId = mbTutorialSprCreate(dataNum);
    espPosSet(sprId, 288.0f, 144.0f);
    for (i = 0; i <= 12u; i++) {
        float scale = i / 12.0f;
        espScaleSet(sprId, scale, scale);
        mbTutorialVSleep();
    }
    return sprId;
}

void mbTutorialSprKill(s16 sprId)
{
    int i;
    int index;

    for (i = 0; i < 16; i++) {
        if (sprId == tutorialSprId[i]) {
            break;
        }
    }
    index = i;
    espKill(sprId);
    tutorialSprId[index] = -1;
}

void mbTutorialSprDispOff(s16 sprId)
{
    int i;

    for (i = 0; i <= 12u; i++) {
        float scale = 1.0f - (i / 12.0f);
        espScaleSet(sprId, scale, scale);
        mbTutorialVSleep();
    }
    mbTutorialSprKill(sprId);
}

static void TutorialSprClose(void)
{
    int i;

    for (i = 0; i < 16; i++) {
        if (tutorialSprId[i] >= 0) {
            espKill(tutorialSprId[i]);
        }
        tutorialSprId[i] = -1;
    }
}

void mbTutorialSprGrpSet(s16 grpId)
{
    s16 id;
    int i;

    for (i = 0; i < 16; i++) {
        if (tutorialSprGrpId[i] < 0) {
            break;
        }
    }
    id = tutorialSprGrpId[i] = grpId;
}

void mbTutorialSprGrpKill(s16 grpId)
{
    int i;
    int index;

    for (i = 0; i < 16; i++) {
        if (grpId == tutorialSprGrpId[i]) {
            break;
        }
    }
    index = i;
    HuSprGrpKill(grpId);
    tutorialSprId[index] = -1;
}

static void TutorialSprGrpClose(void)
{
    int i;

    for (i = 0; i < 16; i++) {
        if (tutorialSprGrpId[i] >= 0) {
            HuSprGrpKill(tutorialSprGrpId[i]);
        }
        tutorialSprGrpId[i] = -1;
    }
}

MBMODELID mbTutorialModelCreate(int dataNum, BOOL linkF)
{
    MBMODELID modelId;
    int i;

    for (i = 0; i < 32; i++) {
        if (tutorialMdlId[i] < 0) {
            break;
        }
    }
    modelId = tutorialMdlId[i] = mbObjCreate(dataNum, NULL, linkF);
    return modelId;
}

void mbTutorialModelKill(int modelId)
{
    int i;
    int modelNo;

    for (i = 0; i < 32; i++) {
        if ((MBMODELID)modelId == tutorialMdlId[i]) {
            break;
        }
    }
    modelNo = i;
    mbObjKill(modelId);
    tutorialMdlId[modelNo] = -1;
}

static void TutorialModelKillAll(void)
{
    int i;

    for (i = 0; i < 32; i++) {
        if (tutorialMdlId[i] >= 0) {
            mbObjKill(tutorialMdlId[i]);
        }
        tutorialMdlId[i] = -1;
    }
}

OMOBJ *mbTutorialGuideCreate(s8 *motTbl, BOOL screenF)
{
    tutorialGuideObj = mbGuideCreateFlag(NULL, motTbl, screenF, FALSE, TRUE);
    mbObjDispSet(mbGuideModelGet(tutorialGuideObj), FALSE);
    mbGuideMotionNextSet(tutorialGuideObj, 1);
    mbGuideMotionShiftSet(tutorialGuideObj, 1, FALSE);
    return tutorialGuideObj;
}

void mbTutorialGuideClose(OMOBJ *obj)
{
    mbGuideKill(obj);
    tutorialGuideObj = NULL;
}

OMOBJ *mbTutorialGuideGet(void)
{
    return tutorialGuideObj;
}

BOOL mbTutorialExitReqGet(void)
{
    return tutorialExitReqF;
}

void mbTutorialExitSet(void)
{
    tutorialExitReqF = TRUE;
}

void mbTutorialExitOnSet(BOOL exitOnF)
{
    tutorialExitOnF = exitOnF;
}

static void TutorialWinInit(void)
{
    int i;

    for (i = 0; i < HUWIN_MAX; i++) {
        tutorialWinData[i].delay = tutorialWinData[i].stat = -1;
        tutorialWinData[i].time = -1;
    }
}

static void TutorialWinUpdate(void)
{
    int i;

    for (i = 0; i < HUWIN_MAX; i++) {
        if (winData[i].grpId != HUSPR_GROUP_NONE) {
            if (tutorialWinData[i].stat < 0) {
                tutorialWinData[i].stat = tutorialWinData[i].time = 0;
                tutorialWinData[i].delay = 0;
            }
            if (winData[i].stat == 2) {
                tutorialWinData[i].stat = 1;
                tutorialWinData[i].delay = 1;
            } else if (winData[i].stat != 0 && tutorialWinData[i].stat != 0) {
                if (tutorialWinData[i].delay != 0) {
                    tutorialWinData[i].delay--;
                } else {
                    tutorialWinData[i].stat = 0;
                    tutorialWinData[i].time++;
                }
            }
        } else {
            tutorialWinData[i].stat = tutorialWinData[i].time = -1;
        }
    }
}

BOOL mbTutorialWinWait(int winNo)
{
    MBWIN *win;
    BOOL waitF = FALSE;

    while (TRUE) {
        mbTutorialVSleep();
        if (mbWinDoneCheck(winNo)) {
            break;
        }
        win = mbWinGet(winNo);
        if (win->winId < 0) {
            break;
        }
        if (tutorialWinData[win->winId].stat < 0) {
            break;
        }
        if (tutorialWinData[win->winId].time > 0) {
            tutorialWinData[win->winId].time--;
            waitF = TRUE;
            break;
        }
    }
    return waitF;
}

void mbTutorialWinClose(int winNo)
{
    MBWIN *win;

    if (!mbWinDoneCheck(winNo)) {
        win = mbWinGet(winNo);
        if (win->winId >= 0 && tutorialWinData[win->winId].stat >= 0) {
            tutorialWinData[win->winId].stat = tutorialWinData[win->winId].time = 0;
        }
    }
}

void mbTutorialWinMesExec(int message)
{
    int winNo;
    int i;

    winNo = mbWinCreateTime(MBWIN_TYPE_GUIDE, message, -1);
    mbWinPlayerDisable(winNo, -1);
    for (i = 0; tutorialGuideTbl[i].message >= 0; i++) {
        if (message == tutorialGuideTbl[i].message) {
            mbAudGuidePlay(tutorialGuideTbl[i].seId);
            break;
        }
    }
    do {
        OMOBJ *guideObj = tutorialGuideObj;

        mbGuideMotionShiftSet(guideObj, 12, TRUE);
    } while (mbTutorialWinWait(winNo));
    mbWinWait(winNo);
}

void mbTutorialWinMesMasuExec(int message, int masuId)
{
    int winNo;
    int sprId;
    int i;

    winNo = mbWinCreateTime(MBWIN_TYPE_GUIDE, message, -1);
    mbWinPlayerDisable(winNo, -1);
    for (i = 0; tutorialGuideTbl[i].message >= 0; i++) {
        if (message == tutorialGuideTbl[i].message) {
            mbAudGuidePlay(tutorialGuideTbl[i].seId);
            break;
        }
    }
    mbGuideMotionShiftSet(mbTutorialGuideGet(), 12, TRUE);
    sprId = mbTutorialSprDispOn(masuId);
    while (mbTutorialWinWait(winNo)) {
        mbGuideMotionShiftSet(mbTutorialGuideGet(), 12, TRUE);
    }
    mbWinWait(winNo);
    mbTutorialSprDispOff(sprId);
}

int mbTutorialWinCreate(int message)
{
    int winNo;
    int i;

    winNo = mbWinCreateTime(MBWIN_TYPE_GUIDE, message, -1);
    mbWinPlayerDisable(winNo, -1);
    for (i = 0; tutorialGuideTbl[i].message >= 0; i++) {
        if (message == tutorialGuideTbl[i].message) {
            mbAudGuidePlay(tutorialGuideTbl[i].seId);
            break;
        }
    }
    {
        OMOBJ *guideObj = tutorialGuideObj;

        mbGuideMotionShiftSet(guideObj, 12, TRUE);
    }
    return winNo;
}

void mbTutorialWinKeyWait(int winNo)
{
    while (mbTutorialWinWait(winNo)) {
        OMOBJ *guideObj = tutorialGuideObj;

        mbGuideMotionShiftSet(guideObj, 12, TRUE);
    }
    mbWinWait(winNo);
}

void mbTutorialMgCallInit(void)
{
    memset(&tutorialMgCallData, 0, sizeof(tutorialMgCallData));
}

void mbTutorialMgCallClose(void)
{
    int i;

    if (tutorialMgCallData.active != 0) {
        for (i = 0; i < 3; i++) {
            if (tutorialMgCallData.espId[i] >= 0) {
                espKill(tutorialMgCallData.espId[i]);
            }
            tutorialMgCallData.espId[i] = -1;
        }
        for (i = 0; i < 4; i++) {
            if (tutorialMgCallData.obj[i]) {
                TutorialMgCallKill(tutorialMgCallData.obj[i]);
            }
            tutorialMgCallData.obj[i] = NULL;
        }
        tutorialMgCallData.active = 0;
    }
}

void mbTutorialMgCallExec(int type)
{
    TUTORIALMGCALLDATA *data = &tutorialMgCallData;
    s16 list[4];
    TUTORIALMGCALLWORK *work;
    int nextMaxTime;
    int delay;
    int nextDelay;
    int listNum;
    int delayOfs;
    int nextTime;
    int result;
    int speed;
    MGDATA *mgData;
    BOOL unlocked;
    int i;
    float weight;

    for (i = 0; i < 3; i++) {
        data->espId[i] = -1;
    }
    for (i = 0; i < 4; i++) {
        data->obj[i] = NULL;
    }
    data->active = TRUE;
    listNum = 4;
    TutorialMgCallListGet(type, listNum, list);

    data->espId[0] = espEntry(mbBoardDataNumGet(DATANUM(DATA_board, 0x87)), 100, 0);
    espPosSet(data->espId[0], 288.0f, 230.4f);
    espDrawNoSet(data->espId[0], 32);
    data->espId[1] = espEntry(mbBoardDataNumGet(DATANUM(DATA_board, 0x88)), 102, 0);
    espPosSet(data->espId[1], 288.0f, 238.4f);
    espDrawNoSet(data->espId[1], 32);
    espTPLvlSet(data->espId[1], 0.5f);
    data->espId[2] = espEntry(mbBoardDataNumGet(DATANUM(DATA_board, 0x89)), 101, 0);
    espPosSet(data->espId[2], 288.0f, 246.4f);
    espDrawNoSet(data->espId[2], 32);
    espTPLvlSet(data->espId[2], 0.5f);
    espDispOff(data->espId[2]);
    for (i = 1; i <= 30; i++) {
        weight = 1.0f - (i / 30.0f);
        espPosSet(data->espId[0], 288.0f, 230.4f + (480.0f * weight));
        espPosSet(data->espId[1], 288.0f, 238.4f + (480.0f * weight));
        mbTutorialVSleep();
    }

    for (i = 0; i < listNum; i++) {
        mgData = NULL;
        if (list[i] >= 0) {
            mgData = &MgDataTbl[list[i]];
        }
        data->obj[i] = omAddObjEx(mbObjMan, 257, 0, 0, -1, TutorialMgCallOMExec);
        work = omObjGetWork(data->obj[i], TUTORIALMGCALLWORK);
        work->dispF = TRUE;
        work->cursorNo = i;
        data->obj[i]->trans.y = mgCallWinYOfsTbl[listNum][i];
        unlocked = FALSE;
        work->winNo = mbWinCreateHelp(0x002A003D);
        work->message = -1;
        if (mgData) {
            work->message = mgData->nameMes;
        }
        mbWinPriSet(work->winNo, 90);
        TutorialMgCallSlideInSet(data->obj[i]);
    }
    tutorialMgCallCursorPos = -1;
    while (!TutorialMgCallSlideInCheck(data->obj[0])) {
        mbTutorialVSleep();
    }

    tutorialMgCallCursorPos = 0;
    result = mbRandMod(listNum);
    delay = mbRandMod(30) + 90;
    delayOfs = (int)((-7.0f + sqrtf((delay * 8.0f) + 49.0f)) / 2.0f);
    speed = (result - delayOfs) % listNum;
    if (speed < 0) {
        speed += listNum;
    }
    nextDelay = (listNum * 12) + (speed * 4);
    nextTime = nextMaxTime = 4;
    for (i = 0; i < nextDelay + delay; i++) {
        if (--nextTime == 0) {
            tutorialMgCallCursorPos = (tutorialMgCallCursorPos + 1) % listNum;
            if (i > nextDelay) {
                nextMaxTime++;
                nextTime = nextMaxTime;
            } else {
                nextTime = nextMaxTime;
            }
            espPosSet(data->espId[2], 288.0f, data->obj[tutorialMgCallCursorPos]->trans.y);
            espDispOn(data->espId[2]);
            mbAudFXPlay(1009);
        }
        mbTutorialVSleep();
    }
    tutorialMgCallCursorPos = result;
    espPosSet(data->espId[2], 288.0f, data->obj[tutorialMgCallCursorPos]->trans.y);
    TutorialMgCallGrowSet(data->obj[tutorialMgCallCursorPos]);
    mbAudFXPlay(1134);
    HuPrcSleep(120);
}

static void TutorialMgCallListGet(int type, int num, s16 *list)
{
    s8 candidates[64];
    int candidateNum = 0;
    int mgNo = -1;
    int i;

    for (i = 0; MgDataTbl[i].ovl != 0xFFFF; i++) {
        if ((MgDataTbl[i].flag & 0x2C0) == 0 && type == MgDataTbl[i].type) {
            candidates[candidateNum++] = i;
        }
    }
    if (candidateNum > 0) {
        mgNo = candidates[mbRandMod(candidateNum)];
    }
    for (i = 0; i < num; i++) {
        list[i] = mgNo;
    }
}

static void TutorialMgCallOMExec(OMOBJ *obj)
{
    TUTORIALMGCALLWORK *work = omObjGetWork(obj, TUTORIALMGCALLWORK);
    HuVec2f size;
    float weight;
    float sinValue;

    if (work->killF || mbExitCheck()) {
        mbWinKill(work->winNo);
        omDelObjEx(HuPrcCurrentGet(), obj);
        return;
    }
    if (work->dispF) {
        mbWinDispSet(work->winNo, TRUE);
    } else {
        mbWinDispSet(work->winNo, FALSE);
    }
    switch (work->state) {
        case 0:
            if (work->time > work->maxTime) {
                work->slideInF = FALSE;
                work->state = 1;
            } else {
                weight = (float)work->time++ / work->maxTime;
                sinValue = sin((M_PI * (90.0f * weight)) / 180.0);
                if ((work->cursorNo & 1) == 0) {
                    obj->trans.x = -160.0f + (448.0f * sinValue);
                } else {
                    obj->trans.x = 736.0f + (-448.0f * sinValue);
                }
            }
            break;
        case 1:
            if (work->cursorNo == tutorialMgCallCursorPos) {
                work->state = 2;
            }
            break;
        case 2:
            if (work->cursorNo != tutorialMgCallCursorPos) {
                work->state = 3;
                work->time = 0;
                work->maxTime = 8;
            }
            break;
        case 3:
            if (work->time > work->maxTime) {
                work->state = 1;
            } else {
                weight = (float)work->time++ / work->maxTime;
            }
            break;
        case 4:
            if (work->time <= work->maxTime) {
                weight = (float)work->time++ / work->maxTime;
                obj->scale.x = obj->scale.y = 1.0f + (0.2f * sin((M_PI * (720.0f * (1.0f - weight))) / 180.0));
            }
            break;
        case 5:
            if (work->time <= work->maxTime) {
                weight = (float)work->time++ / work->maxTime;
                obj->trans.y = obj->rot.y + (weight * (96.0f - obj->rot.y));
                obj->scale.x = obj->scale.y = 0.5f + (0.5f * cos((M_PI * (90.0f * weight)) / 180.0));
            }
            break;
    }
    mbWinScaleSet(work->winNo, obj->scale.x, obj->scale.y);
    mbWinMesMaxSizeGet(work->winNo, &size);
    mbWinPosSet(work->winNo,
        obj->trans.x - ((size.x / 2) * obj->scale.x),
        obj->trans.y - ((size.y / 2) * obj->scale.y));
}

static void TutorialMgCallSlideInSet(OMOBJ *obj)
{
    TUTORIALMGCALLWORK *work = omObjGetWork(obj, TUTORIALMGCALLWORK);

    work->slideInF = TRUE;
    work->state = 0;
    work->time = 0;
    work->maxTime = 30;
}

static BOOL TutorialMgCallSlideInCheck(OMOBJ *obj)
{
    TUTORIALMGCALLWORK *work = omObjGetWork(obj, TUTORIALMGCALLWORK);

    if (work->slideInF) {
        return FALSE;
    }
    return TRUE;
}

static void TutorialMgCallGrowSet(OMOBJ *obj)
{
    TUTORIALMGCALLWORK *work = omObjGetWork(obj, TUTORIALMGCALLWORK);

    work->state = 4;
    work->time = 0;
    work->maxTime = 90;
    if (work->message >= 0) {
        mbWinKill(work->winNo);
        work->winNo = mbWinCreateHelp(work->message);
    }
}

static void TutorialMgCallKill(OMOBJ *obj)
{
    TUTORIALMGCALLWORK *work = omObjGetWork(obj, TUTORIALMGCALLWORK);

    work->killF = TRUE;
}
