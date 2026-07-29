#define _MATH_H
#include "dolphin/math.h"

#include "game/board/window.h"
#include "game/board/audio.h"
#include "game/board/camera.h"
#include "game/board/main.h"
#include "game/board/model.h"
#include "game/board/pause.h"
#include "game/board/player.h"
#include "game/board/status.h"

#include "game/charman.h"
#include "game/data.h"
#include "game/esprite.h"
#include "game/flag.h"
#include "game/frand.h"
#include "game/gamework.h"
#include "game/mgdata.h"
#include "game/object.h"
#include "game/wipe.h"

#include "dolphin/mtx.h"

#include <string.h>

extern int mbSingleOppCharGet(void);
extern int mbSingleTeamCharGet(void);
extern void mbDirClose(void);
extern void mbev_CapPlayerMotShiftSet(int modelId, int motionNo, int attr, BOOL waitF);

typedef struct MgListWork_s {
    unsigned killF : 1;
    unsigned slideF : 1;
    unsigned mode : 3;
    unsigned no : 3;
    unsigned dispF : 1;
    s16 winNo;
    s16 sprId;
    s16 time;
    s16 maxTime;
    int hiddenMes;
} MGLISTWORK;

typedef struct MgCallWork_s {
    int type;
    s16 sprId[10];
    s16 unk18;
    s16 unk1A;
    s16 unk1C;
    s16 unk1E;
    MBMODELID guideMdlId;
    MBMODELID guideItemMdlId;
    s16 battleSprId[3];
    s16 battleNameSprId[3];
    s16 battleWinId[3];
    s16 unk36;
} MGCALLWORK;

typedef struct MgCallSingleType_s {
    int playerNo;
    int type;
    u32 dataNum;
    int bank;
} MGCALLSINGLETYPE;

static const HuVec2f statusPos4PBase[GW_PLAYER_MAX] = {
    { 176.0f, 184.0f },
    { 400.0f, 184.0f },
    { 176.0f, 296.0f },
    { 400.0f, 296.0f },
};

static const HuVec2f statusPos2Vs2Base[GW_PLAYER_MAX] = {
    { 176.0f, 200.0f },
    { 176.0f, 280.0f },
    { 400.0f, 200.0f },
    { 400.0f, 280.0f },
};

static const HuVec2f statusPos1Vs3Base[GW_PLAYER_MAX] = {
    { 176.0f, 240.0f },
    { 400.0f, 160.0f },
    { 400.0f, 240.0f },
    { 400.0f, 320.0f },
};

static const HuVec2f statusPosKettouBase[2] = {
    { 152.0f, 240.0f },
    { 424.0f, 240.0f },
};

static s16 mgCallHisOfsTbl[9];
static s16 mgCallHis4P[16];
static s16 mgCallHis1Vs3[6];
static s16 mgCallHis2Vs2[6];
static s16 mgCallHisKettou[14];
static HuVec2f mgStatusPos4P[GW_PLAYER_MAX];
static HuVec2f mgStatusPos1Vs3[GW_PLAYER_MAX];
static HuVec2f mgStatusPos2Vs2[GW_PLAYER_MAX];
static OMOBJ *mgListObj[4];

static s16 mgCallHisBattle[2];

static s16 *mgCallHisPtr[9] = {
    mgCallHis4P,
    mgCallHis1Vs3,
    mgCallHis2Vs2,
    mgCallHisBattle,
    NULL,
    NULL,
    mgCallHisKettou,
    NULL,
    NULL,
};

static int mgCallHisSize[9] = {
    16,
    6,
    6,
    2,
    0,
    0,
    14,
    0,
    0,
};

static int mgCallTypeFileTbl[4] = {
    0x00050084,
    0x00050084,
    0x00050084,
    0x00050084,
};

static u32 guideMotFileTbl[2][16] = {
    { 0x00110001, 0x00110004, 0x00110005, 0x00110008, 0x00110010, 0x00110011,
        0x00110012, 0x00110013, 0x00110014, 0xFFFFFFFF },
    { 0x0011001C, 0x0011001F, 0x00110020, 0x00110023, 0x0011002A, 0x0011002B,
        0x0011002C, 0x0011002D, 0x0011002E, 0xFFFFFFFF },
};

static char lbl_8024BA18[] = "itemhook_R";

static u32 mgCallDataDir = -1;
static OMOBJ *mgCallVsEffOMObj;
static s16 mgCallFocus;
static BOOL mgCallSingleKoopaF;

void mbev_MgCallSingleKoopa(int playerNo, BOOL koopaF);
static int SetupTeam(int *colorIn);
static int MgRouletteExec(int type, MGCALLWORK *workP);
static void MgRouletteCreate(MGCALLWORK *workP);
static int MgCallBattleExec(MGCALLWORK *workP);
static void MgCallCallMg(int type, int no);
static void MgCallVsEffCreate(void);
static void MgCallVsEffPosSet(float x, float y);
static void MgCallVsEffKill(void);
static int MgCallVsEffNumGet(void);
static void MgRouletteKill(MGCALLWORK *workP);

void mbMgCallInit(void)
{
    int i;

    for (i = 0; i < 9; i++) {
        mgCallHisOfsTbl[i] = 0;
    }
    memset(mgCallHis4P, 0, sizeof(mgCallHis4P));
    memset(mgCallHis1Vs3, 0, sizeof(mgCallHis1Vs3));
    memset(mgCallHis2Vs2, 0, sizeof(mgCallHis2Vs2));
    memset(mgCallHisBattle, 0, sizeof(mgCallHisBattle));
    memset(mgCallHisKettou, 0, sizeof(mgCallHisKettou));
    memcpy(mgStatusPos4P, statusPos4PBase, sizeof(mgStatusPos4P));
    memcpy(mgStatusPos1Vs3, statusPos1Vs3Base, sizeof(mgStatusPos1Vs3));
    memcpy(mgStatusPos2Vs2, statusPos2Vs2Base, sizeof(mgStatusPos2Vs2));
}

s32 mbev_MgCall(void)
{
    MBCAMERA *cameraP;
    HuVecF pos;
    HuVec2f statusPos;
    MGCALLWORK work;
    MGCALLWORK *workP;
    int i;
    int spaceNo;
    int result;
    BOOL battleF;
    float weight;
    float scale;
    float scaleX;
    float scaleY;

    workP = &work;
    battleF = FALSE;
    cameraP = mbCameraGet();
    mbPauseDisableSet(TRUE);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerMotionShiftSet(i, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    }
    MgRouletteCreate(&work);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbStatusCapsuleDispSet(i, FALSE);
    }
    mbStatusDispForceSetAll(TRUE);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerDispSet(i, FALSE);
    }
    if (!GwSystem.curTime) {
        workP->guideMdlId = mbObjCreate(0x110000, (int *)guideMotFileTbl[0], 0);
        workP->guideItemMdlId = -1;
    } else {
        workP->guideMdlId = mbObjCreate(0x11001B, (int *)guideMotFileTbl[1], 0);
        workP->guideItemMdlId = mbObjCreate(0x110035, 0, 0);
        mbObjHookSet(workP->guideMdlId, lbl_8024BA18, workP->guideItemMdlId);
    }
    mbObjMotionSet(workP->guideMdlId, 2, HU3D_MOTATTR_LOOP);
    for (i = 1; i < mbMasuNumGet(); i++) {
        if ((u16)mbMasuAttrGet((s16)i) & 0x8000) {
            break;
        }
    }
    if (i < mbMasuNumGet()) {
        spaceNo = i;
    } else {
        spaceNo = 1;
    }
    mbMasuPosGet((s16)spaceNo, &pos);
    pos.y += 250.0f;
    mbObjPosSet(workP->guideMdlId, pos.x, pos.y, pos.z);
    mbCameraMoveOnSet(FALSE);
    mbCameraMoveObj(workP->guideMdlId, 0, 0, 1800.0f, -1.0f, 21);
    mbCameraMoveWait();
    mbCameraFocusObjSet(-1);
    mbWipeFadeIn();
    mbAudFXPlay(0x457);
    if (!GWTeamFGet()) {
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            mbStatusPosOnGet(i, (HuVecF *)&statusPos);
            mbStatusMoveSet(i, (HuVecF *)&statusPos, (HuVecF *)&statusPos4PBase[i], TRUE, 15);
        }
    } else {
        for (i = 0; i < 2; i++) {
            mbStatusPosOnGet(i, (HuVecF *)&statusPos);
            mbStatusNoMoveSet(i, (HuVecF *)&statusPos, (HuVecF *)&statusPosKettouBase[i], TRUE, 15);
        }
    }
    while (!mbStatusOffCheckAll()) {
        HuPrcVSleep();
    }
    workP->type = SetupTeam(NULL);
    mbObjMotionShiftSet(workP->guideMdlId, 9, 0.0f, 8.0f, FALSE);
    workP->sprId[2] = espEntry(mbBoardDataNumGet(DATANUM(DATA_board, 0x8A)), 90, 0);
    espPosSet(workP->sprId[2], 288.0f, 240.0f);
    espScaleSet(workP->sprId[2], 0.5f, 0.5f);
    mbAudFXPlay(0x458);
    for (i = 1; i <= 30; i++) {
        weight = i / 30.0f;
        scaleX = (0.5f * sin((M_PI * (90.0f * weight)) / 180.0))
            + (1.5f * sin((M_PI * (180.0f * weight)) / 180.0));
        scaleY = (0.5f * sin((M_PI * (90.0f * weight)) / 180.0))
            + (1.5f * fabs(sin((M_PI * (360.0f * weight)) / 180.0)));
        espPosSet(workP->sprId[2], 288.0f - (32.0f * (1.0f - weight)),
            240.0f + (64.0f * sin((M_PI * (180.0f * weight)) / 180.0)));
        espScaleSet(workP->sprId[2], scaleX, scaleY);
        espZRotSet(workP->sprId[2], 360.0f * -weight);
        HuPrcVSleep();
    }
    HuPrcSleep(15);
    workP->sprId[0] = espEntry(mbBoardDataNumGet(mgCallTypeFileTbl[workP->type]), 95, 0);
    espPosSet(workP->sprId[0], 288.0f, -32.0f);
    espDrawNoSet(workP->sprId[0], 32);
    espBankSet(workP->sprId[0], workP->type);
    for (i = 0; i < 30u; i++) {
        weight = i / 30.0f;
        espPosSet(workP->sprId[0], 288.0f,
            88.0f - (120.0f * sin((M_PI * (90.0f * (1.0f - weight))) / 180.0)));
        HuPrcVSleep();
    }
    if (workP->type == MG_TYPE_4P && mbRandMod(100) < 10 && mbPlayerMaxCoinGet() >= 10) {
        battleF = TRUE;
        for (i = 1; i <= 12u; i++) {
            weight = i / 12.0f;
            scale = 0.5f * cos((M_PI * (90.0f * weight)) / 180.0);
            espScaleSet(workP->sprId[2], scale, scale);
            HuPrcVSleep();
        }
        mbAudFXDelaySet(10);
        mbAudGuidePlay(0x3B4);
        mbev_CapPlayerMotShiftSet(workP->guideMdlId, 7, 0, TRUE);
        mbObjMotionShiftSet(workP->guideMdlId, 8, 0.0f, 8.0f, FALSE);
        result = MgCallBattleExec(workP);
    } else {
        for (i = 1; i <= 12u; i++) {
            weight = i / 12.0f;
            scale = 0.1f + (0.4f * cos((M_PI * (90.0f * weight)) / 180.0));
            espScaleSet(workP->sprId[2], scale, scale);
            HuPrcVSleep();
        }
        mbObjMotionShiftSet(workP->guideMdlId, 4, 0.0f, 8.0f, FALSE);
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            mbStatusPosGet(i, (HuVecF *)&statusPos);
            if (statusPos.x >= 288.0f) {
                statusPos.x = 704.0f;
            } else {
                statusPos.x = -128.0f;
            }
            if (statusPos.y > 272.0f) {
                statusPos.y = 608.0f;
            } else if (statusPos.y < 208.0f) {
                statusPos.y = -128.0f;
            }
            mbStatusMoveSet(i, NULL, (HuVecF *)&statusPos, TRUE, 15);
        }
        mbAudFXPlay(0x459);
        MgCallVsEffCreate();
        MgCallVsEffPosSet(288.0f, 240.0f);
        for (i = 1; i <= 30u; i++) {
            weight = i / 30.0f;
            scale = 0.1f + (3.0f * sin((M_PI * (90.0f * weight)) / 180.0));
            espScaleSet(workP->sprId[2], scale, scale);
            espTPLvlSet(workP->sprId[2], 1.0f - weight);
            espZRotSet(workP->sprId[2], 360.0f * sin((M_PI * (90.0f * weight)) / 180.0));
            HuPrcVSleep();
        }
        espDispOff(workP->sprId[2]);
        mbObjMotionShiftSet(workP->guideMdlId, 2, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        while (MgCallVsEffNumGet() != 0) {
            HuPrcVSleep();
        }
        MgCallVsEffKill();
        while (!mbStatusOffCheckAll()) {
            HuPrcVSleep();
        }
        result = MgRouletteExec(workP->type, workP);
        _SetFlag(FLAG_BOARD_MG);
    }
    GwSystem.subGameNo = -1;
    MgCallCallMg(workP->type, result);
    MgRouletteKill(workP);
    return 0;
}

static s16 MgNameColorGet(u32 nameMes)
{
    char *mesPtr;
    s16 i;

    if (nameMes > 0x80000000) {
        mesPtr = (char *)nameMes;
    } else {
        mesPtr = HuWinMesPtrGet(nameMes);
    }
    for (i = 0; *mesPtr; mesPtr++) {
        if (*mesPtr == 0x1E) {
            return mesPtr[1] - 1;
        }
    }
    return 7;
}

static void MgRouletteSlide(OMOBJ *obj)
{
    MGLISTWORK *work = omObjGetWork(obj, MGLISTWORK);

    work->slideF = TRUE;
    work->mode = 0;
    work->time = 0;
    work->maxTime = 30;
}

static void MgRouletteFocus(OMOBJ *obj)
{
    MGLISTWORK *work = omObjGetWork(obj, MGLISTWORK);

    work->mode = 4;
    work->time = 0;
    work->maxTime = 90;
    if (work->hiddenMes >= 0) {
        mbWinKill(work->winNo);
        work->winNo = mbWinCreateHelp(work->hiddenMes);
    }
}

static BOOL MgRouletteSlideCheck(OMOBJ *obj)
{
    MGLISTWORK *work = omObjGetWork(obj, MGLISTWORK);

    if (work->slideF) {
        return FALSE;
    } else {
        return TRUE;
    }
}

void mbMgRouletteFocusKill(BOOL killF)
{
    MGLISTWORK *work = omObjGetWork(mgListObj[mgCallFocus], MGLISTWORK);

    work->dispF = killF;
}

static BOOL MgCallHisCheck(int type, s16 no)
{
    s16 i;

    if (mgCallHisPtr[type] == NULL) {
        return FALSE;
    }
    for (i = 0; i < mgCallHisSize[type]; i++) {
        if (no == mgCallHisPtr[type][i]) {
            return TRUE;
        }
    }
    return FALSE;
}

static MGCALLSINGLETYPE mgCallSingleTypeTbl[] = {
    { 0, MG_TYPE_4P, DATANUM(DATA_board, 0x84), 0 },
    { 1, MG_TYPE_1VS3, DATANUM(DATA_board, 0x84), 1 },
    { 2, MG_TYPE_2VS2, DATANUM(DATA_board, 0x84), 2 },
    { 3, MG_TYPE_BATTLE, DATANUM(DATA_board, 0x84), 3 },
    { 6, MG_TYPE_KETTOU, DATANUM(DATA_board, 0x84), 4 },
    { 4, MG_TYPE_KUPA, DATANUM(DATA_board, 0x85), 0 },
    { -1, -1, -1, -1 },
};

void mbev_MgCallSingle(int playerNo)
{
    mbev_MgCallSingleKoopa(playerNo, FALSE);
}

void mbev_MgCallSingleKoopa(int playerNo, BOOL koopaF)
{
    int i;
    int result;
    int no;
    int noNew;
    int temp;
    MGDATA *mgDataP;
    int sprId;
    int comDif;
    MGCALLWORK work;
    MGCALLWORK *workP;
    int charNo;
    int mgTbl[16];
    int charTbl[3];
    int mgNum;
    int mgCount;
    int type;
    MGCALLSINGLETYPE *singleType;
    float weight;

    mgDataP = MgDataTbl;
    singleType = mgCallSingleTypeTbl;
    workP = &work;
    sprId = -1;
    charNo = -1;
    MgRouletteCreate(workP);
    _SetFlag(FLAG_MG_CIRCUIT);
    mgCallSingleKoopaF = koopaF;
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        comDif = GwSystem.storyComDif;
        GwPlayerConf[i].comDif = comDif;
    }
    if (playerNo == 2) {
        charNo = mbSingleTeamCharGet();
        GwPlayerConf[1].charNo = charNo;
        GwPlayerConf[1].comDif = 2;
        charTbl[0] = CHARNO_MINIKOOPAR;
        charTbl[1] = CHARNO_MINIKOOPAG;
        charTbl[2] = CHARNO_MINIKOOPAB;
        for (i = 0; i < 64; i++) {
            no = i % 3;
            noNew = mbRandMod(3);

            if (no != noNew) {
                temp = charTbl[no];

                charTbl[no] = charTbl[noNew];
                charTbl[noNew] = temp;
            }
        }
        GwPlayerConf[2].charNo = charTbl[0];
        GwPlayerConf[3].charNo = charTbl[1];
    } else {
        GwPlayerConf[1].charNo = CHARNO_MINIKOOPAR;
        GwPlayerConf[2].charNo = CHARNO_MINIKOOPAG;
        GwPlayerConf[3].charNo = CHARNO_MINIKOOPAB;
    }
    switch (playerNo) {
        case -1:
            for (i = 0; i < GW_PLAYER_MAX; i++) {
                GwPlayerConf[i].grpNo = i;
            }
            break;

        case 1:
            GwPlayerConf[0].grpNo = 0;
            GwPlayerConf[1].grpNo = 1;
            GwPlayerConf[2].grpNo = 1;
            GwPlayerConf[3].grpNo = 1;
            break;

        case 2:
            GwPlayerConf[0].grpNo = 0;
            GwPlayerConf[1].grpNo = 0;
            GwPlayerConf[2].grpNo = 1;
            GwPlayerConf[3].grpNo = 1;
            break;

        case 3:
            for (i = 0; i < GW_PLAYER_MAX; i++) {
                GwPlayerConf[i].grpNo = i;
            }
            break;

        case 4:
            for (i = 0; i < GW_PLAYER_MAX; i++) {
                GwPlayerConf[i].grpNo = i;
            }
            break;

        case 5:
            break;

        case 6:
            for (i = 0; i < GW_PLAYER_MAX; i++) {
                GwPlayerConf[i].grpNo = 2;
            }
            GwPlayerConf[0].grpNo = 0;
            GwPlayerConf[1].grpNo = 1;
            GwPlayerConf[1].charNo = mbSingleOppCharGet();
            break;

        case 0:
            for (i = 0; i < GW_PLAYER_MAX; i++) {
                GwPlayerConf[i].grpNo = i;
            }
            break;
    }
    if (playerNo == -1) {
        switch (GwSystem.boardNo) {
            case 6:
                mgNum = 1;
                break;

            case 7:
                mgNum = 2;
                break;

            default:
                mgNum = 3;
                break;
        }
        for (i = 0, mgCount = 0; mgDataP->ovl != (u16)DLL_NONE;
             i++, mgDataP++) {
            if (mgDataP->flag & MG_FLAG_RARE) {
                mgTbl[mgCount] = i;
                mgCount++;
            }
        }
        if (mgCount < mgNum) {
            result = 0;
        } else {
            result = mgTbl[mgNum - 1];
        }
        type = MgDataTbl[mgNum].type;
    } else {
        for (i = 0; i < 5; i++, singleType++) {
            if (singleType->playerNo == playerNo) {
                break;
            }
        }
        mbPauseDisableSet(TRUE);
        sprId = espEntry(mbBoardDataNumGet(singleType->dataNum), 100, 0);
        workP->sprId[0] = sprId;
        espPosSet(sprId, 288.0f, -32.0f);
        espBankSet(sprId, singleType->bank);
        for (i = 0; i < 30u; i++) {
            weight = i / 30.0f;
            espPosSet(sprId, 288.0f,
                88.0f
                    - (120.0f
                        * sin((M_PI * (90.0f * (1.0f - weight))) / 180.0)));
            HuPrcVSleep();
        }
        result = MgRouletteExec(singleType->type, workP);
        type = singleType->type;
    }
    GwSystem.curTime = FALSE;
    GwMgNightF = FALSE;
    MgCallCallMg(type, result);
    mbDirClose();
    if (charNo != -1) {
        CharMotionInit(charNo);
    }
    if (sprId != -1) {
        espKill(sprId);
    }
    HuPrcSleep(-1);
}

void mbMgCallDataClose(void)
{
    if (mgCallDataDir != -1) {
        HuDataDirClose(mgCallDataDir);
    }
}

static void MgCallVsEffKill(void)
{
    mgCallVsEffOMObj = NULL;
}

BOOL mbMgCallSingleOnCheck(void)
{
    return TRUE;
}

static u32 MgCallBattleMesGet(u32 mess)
{
    if (!GwSystem.curTime) {
        return mess;
    } else {
        return mess + 4;
    }
}

static void MgRouletteCreate(MGCALLWORK *workP)
{
    int i;

    memset(workP, 0, sizeof(MGCALLWORK));
    for (i = 0; i < 10; i++) {
        workP->sprId[i] = -1;
    }
    for (i = 0; i < 3; i++) {
        workP->battleSprId[i] = -1;
        workP->battleNameSprId[i] = -1;
        workP->battleWinId[i] = -1;
    }
    workP->guideMdlId = -1;
    workP->guideItemMdlId = -1;
}

static void MgRouletteKill(MGCALLWORK *workP)
{
    int i;

    for (i = 0; i < 10; i++) {
        if (workP->sprId[i] >= 0) {
            espKill(workP->sprId[i]);
        }
    }
    if (workP->guideMdlId >= 0) {
        mbObjKill(workP->guideMdlId);
    }
    if (workP->guideItemMdlId >= 0) {
        mbObjKill(workP->guideItemMdlId);
    }
    for (i = 0; i < 3; i++) {
        if (workP->battleSprId[i] >= 0) {
            espKill(workP->battleSprId[i]);
        }
        if (workP->battleNameSprId[i] >= 0) {
            espKill(workP->battleNameSprId[i]);
        }
        if (workP->battleWinId[i] >= 0) {
            mbWinKill(workP->battleWinId[i]);
        }
    }
    MgRouletteCreate(workP);
}
