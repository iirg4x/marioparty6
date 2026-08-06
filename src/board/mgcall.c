#include "dolphin/math.h"

#include "game/board/window.h"
#include "game/board/audio.h"
#include "game/board/camera.h"
#include "game/board/main.h"
#include "game/board/masu.h"
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
#include "game/pad.h"
#include "game/wipe.h"

#include "messnum/mg_name.h"

#include "dolphin/mtx.h"

#include <string.h>

extern int mbSingleOppCharGet(void);
extern int mbSingleTeamCharGet(void);
extern void mbDirClose(void);
extern void mbev_CapPlayerMotShiftSet(int modelId, int motionNo, int attr, BOOL waitF);
extern void mbev_CapEffColorSet(GXColor *color, int colorNo);
extern s32 mbBGRead(s32 dataNum);
extern void mbBGReadWait(s32 statId);
extern void mbOvlCall(OMOVL ovl);
extern void mbWipeCreate(s16 mode, s16 type, s16 time);
extern void mbWipeSpecialWait(void);

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

typedef struct MgCallVsEffWork_s {
    BOOL activeF;
    s16 sprId;
    int time;
    int maxTime;
    float scaleStep;
    float scaleBase;
    float zRotStep;
    HuVecF pos;
    HuVecF vel;
    float scaleX;
    float scaleY;
    u32 unk3C;
    u32 unk40;
    u32 unk44;
    float zRot;
} MGCALLVSEFFWORK;

typedef struct MgCallSingleType_s {
    int playerNo;
    int type;
    u32 dataNum;
    int bank;
} MGCALLSINGLETYPE;

typedef struct MgPackType_s {
    int pack;
    u8 flag;
} MGPACKTYPE;

enum {
    MGCALL_BOARD_FILE_TYPE = 132,
    MGCALL_BOARD_FILE_KOOPA_TYPE,
    MGCALL_BOARD_FILE_DONKEY_TYPE,
    MGCALL_BOARD_FILE_ROULETTE_LIST = 135,
    MGCALL_BOARD_FILE_ROULETTE_MASK,
    MGCALL_BOARD_FILE_ROULETTE_CURSOR,
    MGCALL_BOARD_FILE_VERSUS,
    MGCALL_BOARD_FILE_VERSUS_EFFECT,
    MGCALL_BOARD_FILE_BATTLE_COIN_LABEL = 143,
    MGCALL_BOARD_FILE_BATTLE_COIN_NUMBER,
    MGCALL_BOARD_FILE_BATTLE_NAME_BACKDROP,
    MGCALL_BOARD_FILE_BATTLE_PLAYER_CURSOR,
    MGCALL_BOARD_FILE_BATTLE_CHAR_MARIO,
    MGCALL_BOARD_FILE_BATTLE_CHAR_LUIGI,
    MGCALL_BOARD_FILE_BATTLE_CHAR_PEACH,
    MGCALL_BOARD_FILE_BATTLE_CHAR_YOSHI,
    MGCALL_BOARD_FILE_BATTLE_CHAR_WARIO,
    MGCALL_BOARD_FILE_BATTLE_CHAR_DAISY,
    MGCALL_BOARD_FILE_BATTLE_CHAR_WALUIGI,
    MGCALL_BOARD_FILE_BATTLE_CHAR_KINOPIO,
    MGCALL_BOARD_FILE_BATTLE_CHAR_TERESA,
    MGCALL_BOARD_FILE_BATTLE_CHAR_MINIKOOPA,
    MGCALL_BOARD_FILE_BATTLE_CHAR_KINOPICO,
    MGCALL_BOARD_FILE_BATTLE_M646,
    MGCALL_BOARD_FILE_BATTLE_M647_DAY,
    MGCALL_BOARD_FILE_BATTLE_M647_NIGHT,
    MGCALL_BOARD_FILE_BATTLE_M649,
    MGCALL_BOARD_FILE_BATTLE_M664_DAY,
    MGCALL_BOARD_FILE_BATTLE_M664_NIGHT,
    MGCALL_BOARD_FILE_BATTLE_M680,
    MGCALL_BOARD_FILE_BATTLE_M681,
    MGCALL_BOARD_FILE_EFFECT_EXPLODE = 91,
};

enum {
    MGCALL_GUIDE_DAY_MODEL,
    MGCALL_GUIDE_DAY_MOTION_1,
    MGCALL_GUIDE_DAY_MOTION_2 = 4,
    MGCALL_GUIDE_DAY_MOTION_3,
    MGCALL_GUIDE_DAY_MOTION_4 = 8,
    MGCALL_GUIDE_DAY_MOTION_5 = 16,
    MGCALL_GUIDE_DAY_MOTION_6,
    MGCALL_GUIDE_DAY_MOTION_7,
    MGCALL_GUIDE_DAY_MOTION_8,
    MGCALL_GUIDE_DAY_MOTION_9,
    MGCALL_GUIDE_NIGHT_MODEL = 27,
    MGCALL_GUIDE_NIGHT_MOTION_1,
    MGCALL_GUIDE_NIGHT_MOTION_2 = 31,
    MGCALL_GUIDE_NIGHT_MOTION_3,
    MGCALL_GUIDE_NIGHT_MOTION_4 = 35,
    MGCALL_GUIDE_NIGHT_MOTION_5 = 42,
    MGCALL_GUIDE_NIGHT_MOTION_6,
    MGCALL_GUIDE_NIGHT_MOTION_7,
    MGCALL_GUIDE_NIGHT_MOTION_8,
    MGCALL_GUIDE_NIGHT_MOTION_9,
    MGCALL_GUIDE_NIGHT_ITEM_MODEL = 53,
};

enum {
    MGCALL_BATTLE_FILE_COIN_SPRITE_0,
    MGCALL_BATTLE_FILE_COIN_SPRITE_1,
    MGCALL_BATTLE_FILE_COIN_SPRITE_2,
    MGCALL_BATTLE_FILE_COIN_SPRITE_3,
    MGCALL_BATTLE_FILE_COIN_SPRITE_4,
};

enum {
    MGCALL_ROULETTE_OBJECT_PRIORITY = 257,
    MGCALL_MESSAGE_COLOR_CONTROL = 30,
    MGCALL_VS_EFFECT_COUNT = 64,
    MGCALL_VS_EFFECT_SPRITE_PRIORITY = 99,
    MGCALL_VS_EFFECT_OBJECT_PRIORITY = -32768,
};

#define MGCALL_MESSAGE_POINTER_BASE (1u << 31)
#define MGCALL_MESSAGE_BATTLE_INTRO MESSNUM(MESS_MGPACK_NAME, 0)
#define MGCALL_MESSAGE_BATTLE_COIN_HIGH MESSNUM(MESS_MGPACK_NAME, 1)
#define MGCALL_MESSAGE_BATTLE_COIN_LOW MESSNUM(MESS_MGPACK_NAME, 2)
#define MGCALL_MESSAGE_BATTLE_RESULT MESSNUM(MESS_MGPACK_NAME, 3)
#define MGCALL_MESSAGE_BATTLE_HELP MESSNUM(MESS_MGPACK_NAME, 8)

#define MGCALL_TUTORIAL_MESSAGE_BEGIN MESSNUM(MESS_BOARD_TUTORIAL, 45)
#define MGCALL_TUTORIAL_MESSAGE_ENDTURN MESSNUM(MESS_BOARD_TUTORIAL, 46)
#define MGCALL_TUTORIAL_MESSAGE_STATUS MESSNUM(MESS_BOARD_TUTORIAL, 47)
#define MGCALL_TUTORIAL_MESSAGE_4P MESSNUM(MESS_BOARD_TUTORIAL, 49)
#define MGCALL_TUTORIAL_MESSAGE_GROUP MESSNUM(MESS_BOARD_TUTORIAL, 51)
#define MGCALL_TUTORIAL_MESSAGE_2VS2 MESSNUM(MESS_BOARD_TUTORIAL, 52)
#define MGCALL_TUTORIAL_MESSAGE_1VS3 MESSNUM(MESS_BOARD_TUTORIAL, 54)
#define MGCALL_TUTORIAL_MESSAGE_3VS1 MESSNUM(MESS_BOARD_TUTORIAL, 55)
#define MGCALL_TUTORIAL_MESSAGE_TYPE MESSNUM(MESS_BOARD_TUTORIAL, 57)
#define MGCALL_TUTORIAL_MESSAGE_CALL MESSNUM(MESS_BOARD_TUTORIAL, 58)
#define MGCALL_TUTORIAL_MESSAGE_BATTLE MESSNUM(MESS_BOARD_TUTORIAL, 59)
#define MGCALL_TUTORIAL_MESSAGE_COIN MESSNUM(MESS_BOARD_TUTORIAL, 60)

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
    DATANUM(DATA_board, MGCALL_BOARD_FILE_TYPE),
    DATANUM(DATA_board, MGCALL_BOARD_FILE_TYPE),
    DATANUM(DATA_board, MGCALL_BOARD_FILE_TYPE),
    DATANUM(DATA_board, MGCALL_BOARD_FILE_TYPE),
};

static u32 guideMotFileTbl[2][16] = {
    { DATANUM(DATA_capsulechar4, MGCALL_GUIDE_DAY_MOTION_1),
        DATANUM(DATA_capsulechar4, MGCALL_GUIDE_DAY_MOTION_2),
        DATANUM(DATA_capsulechar4, MGCALL_GUIDE_DAY_MOTION_3),
        DATANUM(DATA_capsulechar4, MGCALL_GUIDE_DAY_MOTION_4),
        DATANUM(DATA_capsulechar4, MGCALL_GUIDE_DAY_MOTION_5),
        DATANUM(DATA_capsulechar4, MGCALL_GUIDE_DAY_MOTION_6),
        DATANUM(DATA_capsulechar4, MGCALL_GUIDE_DAY_MOTION_7),
        DATANUM(DATA_capsulechar4, MGCALL_GUIDE_DAY_MOTION_8),
        DATANUM(DATA_capsulechar4, MGCALL_GUIDE_DAY_MOTION_9), HU_DATANUM_NONE },
    { DATANUM(DATA_capsulechar4, MGCALL_GUIDE_NIGHT_MOTION_1),
        DATANUM(DATA_capsulechar4, MGCALL_GUIDE_NIGHT_MOTION_2),
        DATANUM(DATA_capsulechar4, MGCALL_GUIDE_NIGHT_MOTION_3),
        DATANUM(DATA_capsulechar4, MGCALL_GUIDE_NIGHT_MOTION_4),
        DATANUM(DATA_capsulechar4, MGCALL_GUIDE_NIGHT_MOTION_5),
        DATANUM(DATA_capsulechar4, MGCALL_GUIDE_NIGHT_MOTION_6),
        DATANUM(DATA_capsulechar4, MGCALL_GUIDE_NIGHT_MOTION_7),
        DATANUM(DATA_capsulechar4, MGCALL_GUIDE_NIGHT_MOTION_8),
        DATANUM(DATA_capsulechar4, MGCALL_GUIDE_NIGHT_MOTION_9), HU_DATANUM_NONE },
};

static char lbl_8024BA18[] = "itemhook_R";

static int mgCallColorChanceTbl[3] = { 60, 10, 30 };
static int mgCallColorChanceTbl2[3] = { 30, 40, 30 };

static u32 mgCallDataDir = -1;
static OMOBJ *mgCallVsEffOMObj;
static s16 mgCallFocus;
static BOOL mgCallSingleKoopaF;

void mbev_MgCallSingleKoopa(int playerNo, BOOL koopaF);
static int SetupTeam(int *colorIn);
static int SetupMgType(int *color);
static int MgRouletteExec(int type, MGCALLWORK *workP);
static void MgRouletteCreate(MGCALLWORK *workP);
static int MgCallBattleExec(MGCALLWORK *workP);
static int MgCallBattleSelectExec(MGCALLWORK *workP, int listNum, s16 *list);
static int MgCallBattleCoinGet(void);
static s16 MgNameColorGet(u32 nameMes);
static void MgRouletteOMExec(OMOBJ *obj);
static void MgRouletteSlide(OMOBJ *obj);
static void MgRouletteFocus(OMOBJ *obj);
static BOOL MgRouletteSlideCheck(OMOBJ *obj);
static int MgRouletteNumGet(int type, int height, s16 *noTbl);
static int MgRouletteNumSingleGet(int type, int height, s16 *noTbl);
static int MgRouletteNumMicGet(int type, int height, s16 *noTbl);
static int MgRouletteNumMicSingleGet(int type, int height, s16 *noTbl);
static BOOL MgCallHisCheck(int type, s16 no);
static u32 MgCallBattleMesGet(u32 mess);
static void MgCallCallMg(int type, int no);
static void MgCallVsEffCreate(void);
static void MgCallVsEffOMExec(OMOBJ *obj);
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
        workP->guideMdlId = mbObjCreate(
            DATANUM(DATA_capsulechar4, MGCALL_GUIDE_DAY_MODEL),
            (int *)guideMotFileTbl[0], 0);
        workP->guideItemMdlId = -1;
    } else {
        workP->guideMdlId = mbObjCreate(
            DATANUM(DATA_capsulechar4, MGCALL_GUIDE_NIGHT_MODEL),
            (int *)guideMotFileTbl[1], 0);
        workP->guideItemMdlId = mbObjCreate(
            DATANUM(DATA_capsulechar4, MGCALL_GUIDE_NIGHT_ITEM_MODEL), 0, 0);
        mbObjHookSet(workP->guideMdlId, lbl_8024BA18, workP->guideItemMdlId);
    }
    mbObjMotionSet(workP->guideMdlId, 2, HU3D_MOTATTR_LOOP);
    for (i = 1; i < mbMasuNumGet(); i++) {
        if ((u16)mbMasuAttrGet((s16)i) & MASU_FLAG_START) {
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
    mbAudFXPlay(MSM_SE_BRD00_107);
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
    workP->sprId[2] = espEntry(mbBoardDataNumGet(
        DATANUM(DATA_board, MGCALL_BOARD_FILE_VERSUS)), 90, 0);
    espPosSet(workP->sprId[2], 288.0f, 240.0f);
    espScaleSet(workP->sprId[2], 0.5f, 0.5f);
    mbAudFXPlay(MSM_SE_BRD00_108);
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
        mbAudGuidePlay(MSM_SE_GUIDE_24);
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
        mbAudFXPlay(MSM_SE_BRD00_109);
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

static int SetupTeam(int *colorIn)
{
    int i;
    int j;
    int statusNo;
    int type;
    int colorP1;
    int playerNum;
    BOOL colorChange;
    int origColor[GW_PLAYER_MAX];
    int colorTbl[GW_PLAYER_MAX];

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        GwPlayer[i].teamBackup = GwPlayer[i].team;
    }
    if (!GWTeamFGet()) {
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            origColor[i] = mbStatusColorGet(i);
        }
    } else {
        for (i = 0; i < GW_PLAYER_MAX / 2; i++) {
            origColor[i] = (i == 0) ? STATUS_COLOR_RED : STATUS_COLOR_BLUE;
        }
    }
    if (colorIn != NULL) {
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            mbStatusColorSet(i, colorIn[i]);
            colorTbl[i] = colorIn[i];
        }
    }
    type = SetupMgType(colorTbl);
    if (!GWTeamFGet()) {
        playerNum = GW_PLAYER_MAX;
    } else {
        if (type == MG_TYPE_1VS3) {
            type = (frandf() < 0.5f) ? MG_TYPE_2VS2 : MG_TYPE_4P;
        }
        if (type == MG_TYPE_4P) {
            colorTbl[0] = STATUS_COLOR_RED;
            colorTbl[1] = STATUS_COLOR_RED;
        } else {
            colorTbl[0] = STATUS_COLOR_RED;
            colorTbl[1] = STATUS_COLOR_BLUE;
        }
        playerNum = GW_PLAYER_MAX / 2;
    }
    colorChange = FALSE;
    for (i = 0; i < playerNum; i++) {
        if (origColor[i] != colorTbl[i]) {
            colorChange = TRUE;
        }
    }
    if (colorChange) {
        mbAudFXPlay(MSM_SE_BRD00_127);
        for (i = 0; i <= 60u; i++) {
            for (j = 0; j < playerNum; j++) {
                if (origColor[j] != colorTbl[j]) {
                    mbStatusRainbowSet(j, i / 60.0f, colorTbl[j]);
                }
            }
            HuPrcVSleep();
        }
    }
    HuPrcSleep(30);
    if (GWTeamFGet()) {
        switch (type) {
        case MG_TYPE_2VS2:
            for (i = 0; i < GW_PLAYER_MAX; i++) {
                GwPlayerConf[i].grpNo = GwPlayer[i].team;
            }
            break;
        case MG_TYPE_4P:
            for (i = 0; i < GW_PLAYER_MAX; i++) {
                GwPlayerConf[i].grpNo = 0;
            }
            break;
        }
        return type;
    }
    switch (type) {
    case MG_TYPE_1VS3:
        j = 0;
        colorP1 = colorTbl[0];
        for (i = 1; i < GW_PLAYER_MAX; i++) {
            if (colorP1 == colorTbl[i]) {
                j |= 1 << i;
            }
        }
        if (j == 0) {
            i = 0;
        } else {
            for (i = 1; i < GW_PLAYER_MAX; i++) {
                if ((j & (1 << i)) == 0) {
                    break;
                }
            }
        }
        colorP1 = colorTbl[i];
        statusNo = 0;
        mbStatusMoveSet(i, NULL,
            (HuVecF *)&statusPos1Vs3Base[statusNo++], STATUS_MOVE_SIN, 15);
        GwPlayerConf[i].grpNo = 0;
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (colorP1 != colorTbl[i]) {
                mbStatusMoveSet(i, NULL,
                    (HuVecF *)&statusPos1Vs3Base[statusNo++],
                    STATUS_MOVE_SIN, 15);
                GwPlayerConf[i].grpNo = 1;
            }
        }
        break;
    case MG_TYPE_2VS2:
        statusNo = 0;
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (colorTbl[i] == STATUS_COLOR_RED) {
                mbStatusMoveSet(i, NULL,
                    (HuVecF *)&statusPos2Vs2Base[statusNo++],
                    STATUS_MOVE_SIN, 15);
                GwPlayerConf[i].grpNo = 0;
            }
        }
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (colorTbl[i] == STATUS_COLOR_BLUE) {
                mbStatusMoveSet(i, NULL,
                    (HuVecF *)&statusPos2Vs2Base[statusNo++],
                    STATUS_MOVE_SIN, 15);
                GwPlayerConf[i].grpNo = 1;
            }
        }
        break;
    }
    while (!mbStatusOffCheckAll()) {
        HuPrcVSleep();
    }
    return type;
}

static int SetupMgType(int *color)
{
    int i;
    int no;
    int blueNum;
    int redNum;
    int randomNum;
    int preferRedF;
    int playerNum;
    int chanceTotal;
    int type;
    int hatenaNum;
    int sameGrpF;
    int index1;
    int index2;
    int chance;
    int randomTypeTbl[MG_TYPE_MAX];
    int totalChanceTbl[MG_TYPE_MAX];
    int redPlayerTbl[GW_PLAYER_MAX];
    int bluePlayerTbl[GW_PLAYER_MAX];
    int hatenaPlayerTbl[GW_PLAYER_MAX];

    playerNum = 0;
    hatenaNum = 0;
    blueNum = redNum = 0;
    sameGrpF = TRUE;
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        switch (mbStatusColorGet(i)) {
        case STATUS_COLOR_BLUE:
            bluePlayerTbl[blueNum++] = i;
            if (redNum) {
                sameGrpF = FALSE;
            }
            playerNum++;
            break;
        case STATUS_COLOR_RED:
            redPlayerTbl[redNum++] = i;
            if (blueNum) {
                sameGrpF = FALSE;
            }
            playerNum++;
            break;
        default:
            hatenaPlayerTbl[hatenaNum++] = i;
            break;
        }
    }
    if (playerNum >= GW_PLAYER_MAX) {
        if (sameGrpF) {
            type = MG_TYPE_4P;
        } else if (blueNum == redNum) {
            type = MG_TYPE_2VS2;
        } else {
            type = MG_TYPE_1VS3;
        }
    } else {
        randomNum = 0;
        if (sameGrpF) {
            randomTypeTbl[randomNum++] = MG_TYPE_4P;
            if (playerNum < GW_PLAYER_MAX / 2) {
                randomTypeTbl[randomNum++] = MG_TYPE_2VS2;
            }
        } else {
            randomTypeTbl[randomNum++] = MG_TYPE_2VS2;
        }
        randomTypeTbl[randomNum++] = MG_TYPE_1VS3;
        chanceTotal = 0;
        for (i = 0; i < randomNum; i++) {
            chanceTotal += mgCallColorChanceTbl[randomTypeTbl[i]];
            totalChanceTbl[i] = chanceTotal;
        }
        chance = frandmod(chanceTotal);
        for (i = 0; i < randomNum; i++) {
            if (chance < totalChanceTbl[i]) {
                break;
            }
        }
        type = randomTypeTbl[i];
        for (i = 0; i < 100; i++) {
            index1 = frandmod(hatenaNum);
            index2 = frandmod(hatenaNum);
            no = hatenaPlayerTbl[index1];
            hatenaPlayerTbl[index1] = hatenaPlayerTbl[index2];
            hatenaPlayerTbl[index2] = no;
        }
        switch (type) {
        case MG_TYPE_4P:
            if (playerNum == 0) {
                preferRedF = (frandf() < 0.5f) ? FALSE : TRUE;
            } else {
                preferRedF = (blueNum != 0) ? FALSE : TRUE;
            }
            no = 0;
            if (!preferRedF) {
                for (i = blueNum; i < GW_PLAYER_MAX; i++) {
                    bluePlayerTbl[blueNum++] = hatenaPlayerTbl[no++];
                }
            } else {
                for (i = redNum; i < GW_PLAYER_MAX; i++) {
                    redPlayerTbl[redNum++] = hatenaPlayerTbl[no++];
                }
            }
            break;
        case MG_TYPE_1VS3:
            if (playerNum == 0) {
                preferRedF = (frandf() < 0.5f) ? FALSE : TRUE;
            } else if (blueNum >= 2) {
                preferRedF = FALSE;
            } else if (redNum >= 2) {
                preferRedF = TRUE;
            } else {
                preferRedF = (frandf() < 0.5f) ? FALSE : TRUE;
            }
            no = 0;
            if (!preferRedF) {
                for (i = blueNum; i < GW_PLAYER_MAX - 1; i++) {
                    bluePlayerTbl[blueNum++] = hatenaPlayerTbl[no++];
                }
                for (i = redNum; i < 1; i++) {
                    redPlayerTbl[redNum++] = hatenaPlayerTbl[no++];
                }
            } else {
                for (i = blueNum; i < 1; i++) {
                    bluePlayerTbl[blueNum++] = hatenaPlayerTbl[no++];
                }
                for (i = redNum; i < GW_PLAYER_MAX - 1; i++) {
                    redPlayerTbl[redNum++] = hatenaPlayerTbl[no++];
                }
            }
            break;
        case MG_TYPE_2VS2:
            no = 0;
            for (i = blueNum; i < GW_PLAYER_MAX / 2; i++) {
                bluePlayerTbl[blueNum++] = hatenaPlayerTbl[no++];
            }
            for (i = redNum; i < GW_PLAYER_MAX / 2; i++) {
                redPlayerTbl[redNum++] = hatenaPlayerTbl[no++];
            }
            break;
        }
    }
    for (i = 0; i < blueNum; i++) {
        color[bluePlayerTbl[i]] = STATUS_COLOR_BLUE;
    }
    for (i = 0; i < redNum; i++) {
        color[redPlayerTbl[i]] = STATUS_COLOR_RED;
    }
    return type;
}

static int MgRouletteExec(int type, MGCALLWORK *workP)
{
    static int posYTbl[5][4] = {
        { 0, 0, 0, 0 },
        { 248, 0, 0, 0 },
        { 224, 272, 0, 0 },
        { 208, 248, 288, 0 },
        { 203, 233, 263, 293 },
    };
    static int defaultHeightTbl[9] = {
        4, 3, 3, 3, 3, 3, 3, 3, 3,
    };
    int i;
    MGLISTWORK *listWork;
    int listNum;
    int delay;
    MGDATA *mgData;
    int nextMaxTime;
    int nextTime;
    int speed;
    int focusNo;
    int focusOfs;
    int nameColor;
    int nextDelay;
    int micF;
    BOOL unlocked;
    int group0Num;
    int group1Num;
    int groupNo;
    BOOL soundF;
    int unlockNum;
    HuVecF pos;
    HuVecF guidePos;
    HuVecF movePos;
    MGCALLWORK work;
    int unlockTbl[16];
    s16 mgTbl[128];
    float chance;
    float weight;
    float posY;
    float zRot;
    float velY;

    micF = FALSE;
    unlockNum = 0;
    soundF = FALSE;
    if (workP == NULL) {
        workP = &work;
        MgRouletteCreate(workP);
    }
    listNum = defaultHeightTbl[type];
    if (type == MG_TYPE_1VS3) {
        group0Num = group1Num = 0;
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (GwPlayerConf[i].grpNo == 0) {
                group0Num++;
            } else {
                group1Num++;
            }
        }
        if (group0Num == 1) {
            groupNo = 0;
        } else {
            groupNo = 1;
        }
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (groupNo == GwPlayerConf[i].grpNo && !GwPlayer[i].comF) {
                micF = TRUE;
            }
        }
    }
    if (HuMCMicGet() == 1) {
        if (HuMCProbe(1) != 0) {
            micF = FALSE;
        }
    }
    if (HuMCMicGet() == 0) {
        micF = FALSE;
    }
    if (GWPartyGet() != FALSE) {
        chance = 0.5f;
    } else {
        chance = 0.1f;
    }
    if (micF && frandf() < chance) {
        type = MG_TYPE_1VS3;
        workP->sprId[1] = espEntry(
            mbBoardDataNumGet(DATANUM(DATA_board, MGCALL_BOARD_FILE_TYPE)), 95, 5);
        mbAudFXPlay(1115);
        posY = 88.0f;
        velY = 6.0f;
        zRot = 0.0f;
        i = 0;
        do {
            weight = i / 30.0f;
            if (weight > 1.0f) {
                weight = 1.0f;
            }
            espPosSet(workP->sprId[1], 288.0f,
                88.0f - (120.0f
                    * sin((M_PI * (90.0f * (1.0f - weight))) / 180.0)));
            if (weight >= 0.6f) {
                if (!soundF) {
                    mbAudFXPlay(1116);
                    soundF = TRUE;
                }
                if (zRot > -40.0f) {
                    zRot -= 1.0f;
                }
                posY += velY;
                velY += 0.2f;
                if (workP->sprId[0] >= 0) {
                    espPosSet(workP->sprId[0], 288.0f, posY);
                    espZRotSet(workP->sprId[0], zRot);
                }
            }
            HuPrcVSleep();
            i++;
        } while (posY < 576.0f);
        if (GWPartyGet() != FALSE) {
            listNum = MgRouletteNumMicGet(type, listNum, mgTbl);
        } else {
            listNum = MgRouletteNumMicSingleGet(type, listNum, mgTbl);
        }
    } else {
        if (GWPartyGet() != FALSE) {
            listNum = MgRouletteNumGet(type, listNum, mgTbl);
        } else {
            listNum = MgRouletteNumSingleGet(type, listNum, mgTbl);
        }
    }
    workP->sprId[4] = espEntry(
        mbBoardDataNumGet(
            DATANUM(DATA_board, MGCALL_BOARD_FILE_ROULETTE_LIST)), 100, 0);
    espPosSet(workP->sprId[4], 288.0f, 240.0f);
    espDrawNoSet(workP->sprId[4], 32);
    workP->sprId[5] = espEntry(
        mbBoardDataNumGet(
            DATANUM(DATA_board, MGCALL_BOARD_FILE_ROULETTE_MASK)), 102, 0);
    espPosSet(workP->sprId[5], 288.0f, 248.0f);
    espDrawNoSet(workP->sprId[5], 32);
    espTPLvlSet(workP->sprId[5], 0.5f);
    workP->sprId[6] = espEntry(
        mbBoardDataNumGet(
            DATANUM(DATA_board, MGCALL_BOARD_FILE_ROULETTE_CURSOR)), 101, 0);
    espPosSet(workP->sprId[6], 288.0f, 256.0f);
    espDrawNoSet(workP->sprId[6], 32);
    espTPLvlSet(workP->sprId[6], 0.5f);
    espDispOff(workP->sprId[6]);
    for (i = 1; i <= 30; i++) {
        weight = 1.0f - (i / 30.0f);
        espPosSet(workP->sprId[4], 288.0f, 32.0f + (240.0f + (480.0f * weight)));
        espPosSet(workP->sprId[5], 288.0f, 32.0f + (248.0f + (480.0f * weight)));
        HuPrcVSleep();
    }
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mgListObj[i] = NULL;
    }
    unlockNum = 0;
    for (i = 0; i < listNum; i++) {
        mgData = &MgDataTbl[mgTbl[i]];
        mgListObj[i] = omAddObjEx(mbObjMan, MGCALL_ROULETTE_OBJECT_PRIORITY,
            0, 0, -1, MgRouletteOMExec);
        listWork = omObjGetWork(mgListObj[i], MGLISTWORK);
        listWork->dispF = TRUE;
        listWork->no = i;
        mgListObj[i]->trans.y = posYTbl[listNum][i] + 32;
        unlocked = FALSE;
        if (GWPartyGet() != FALSE) {
            if (GWMgUnlockGet(mgTbl[i] + GW_MGNO_BASE)) {
                unlocked = TRUE;
            }
        } else {
            if (GWMgUnlockGet(mgTbl[i] + GW_MGNO_BASE)
                || mbSingleMgUnlockGet(mgTbl[i] + GW_MGNO_BASE)) {
                unlocked = TRUE;
            }
        }
        if (unlocked) {
            listWork->winNo = mbWinCreateHelp(mgData->nameMes);
            listWork->hiddenMes = -1;
        } else {
            if (mbLanguageGet() != 5) {
                listWork->winNo = mbWinCreateHelp(MESSNUM_PTR("\xC3\xC3\xC3\xC3\xC3\xC3\xC3"));
            } else {
                listWork->winNo = mbWinCreateHelp(MESSNUM_PTR("]\xC3^"));
            }
            listWork->hiddenMes = mgData->nameMes;
            unlockTbl[unlockNum] = i;
            unlockNum++;
        }
        mbWinPriSet(listWork->winNo, 90);
        nameColor = MgNameColorGet(mgData->nameMes);
        MgRouletteSlide(mgListObj[i]);
    }
    mgCallFocus = -1;
    while (!MgRouletteSlideCheck(mgListObj[0])) {
        HuPrcVSleep();
    }
    mgCallFocus = 0;
    focusNo = mbRandMod(listNum);
    if (GWPartyGet() == FALSE && unlockNum != 0) {
        focusNo = unlockTbl[mbRandMod(unlockNum)];
    }
    delay = mbRandMod(30) + 90;
    focusOfs = (int)((-5.0f + sqrtf((delay * 8.0f) + 25.0f)) / 2.0f);
    speed = (focusNo - focusOfs) % listNum;
    if (speed < 0) {
        speed += listNum;
    }
    nextDelay = (listNum * 3) + (speed * 3);
    nextTime = nextMaxTime = 3;
    for (i = 0; i < nextDelay + delay; i++) {
        if (--nextTime == 0) {
            mgCallFocus = (mgCallFocus + 1) % listNum;
            if (i > nextDelay) {
                nextMaxTime++;
                nextTime = nextMaxTime;
            } else {
                nextTime = nextMaxTime;
            }
            espPosSet(workP->sprId[6], 288.0f, mgListObj[mgCallFocus]->trans.y);
            espDispOn(workP->sprId[6]);
            mbAudFXPlay(1009);
        }
        HuPrcVSleep();
    }
    mgCallFocus = focusNo;
    espPosSet(workP->sprId[6], 288.0f, mgListObj[mgCallFocus]->trans.y);
    MgRouletteFocus(mgListObj[mgCallFocus]);
    mbAudFXPlay(1134);
    if (GWPartyGet() == FALSE
        && !mbSingleMgUnlockGet(mgTbl[mgCallFocus] + GW_MGNO_BASE)
        && !GWMgUnlockGet(mgTbl[mgCallFocus] + GW_MGNO_BASE)) {
        omVibrate(GwSystem.turnPlayerNo, 20, 7, 3);
    }
    if (workP->guideMdlId >= 0) {
        float weight;
        float zRot;

        mbObjPosGet(workP->guideMdlId, &pos);
        guidePos.x = pos.x - 300.0f;
        guidePos.y = pos.y - 150.0f;
        guidePos.z = pos.z;
        mbObjMotionShiftSet(workP->guideMdlId, 3, 0.0f, 8.0f, FALSE);
        for (i = 1; i <= 60U; i++) {
            weight = i / 60.0f;
            if (weight <= 0.25f) {
                zRot = -90.0f
                    * sin((M_PI * (90.0f * (4.0f * weight))) / 180.0);
            } else if (weight < 0.75) {
                zRot = -90.0f;
            } else {
                zRot = -90.0f
                    * cos((M_PI * (90.0f * (4.0f * (weight - 0.75f)))) / 180.0);
            }
            mbev_CapVecChase((float)sin((M_PI * (90.0f * weight)) / 180.0),
                &pos, &guidePos, &movePos);
            mbObjPosSetV(workP->guideMdlId, &movePos);
            mbObjRotSet(workP->guideMdlId, 0.0f, zRot, 0.0f);
            HuPrcVSleep();
        }
        mbObjMotionShiftSet(workP->guideMdlId, 2, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
        HuPrcSleep(120);
    } else {
        HuPrcSleep(120);
    }
    return mgTbl[mgCallFocus];
}

static s16 MgNameColorGet(u32 nameMes)
{
    char *mesPtr;
    s16 i;

    if (nameMes > MGCALL_MESSAGE_POINTER_BASE) {
        mesPtr = (char *)nameMes;
    } else {
        mesPtr = HuWinMesPtrGet(nameMes);
    }
    for (i = 0; *mesPtr; mesPtr++) {
        if (*mesPtr == MGCALL_MESSAGE_COLOR_CONTROL) {
            return mesPtr[1] - 1;
        }
    }
    return 7;
}

static int MgRouletteNumGet(int type, int height, s16 *noTbl)
{
    MGPACKTYPE packTbl[4] = {
        { GW_MINIGAME_PACK_EASY, MG_FLAG_PACK_EASY },
        { GW_MINIGAME_PACK_ACTION, MG_FLAG_PACK_ACTION },
        { GW_MINIGAME_PACK_HARD, MG_FLAG_PACK_SKILL },
        { GW_MINIGAME_PACK_WEIRD, MG_FLAG_PACK_WEIRD },
    };
    s16 tempTbl[128];
    s16 historyNo;
    int i;
    int j;
    int no;
    int num;
    int packNo;
    int pack;
    int historyOfs;
    BOOL micF;
    BOOL skipF;
    MGDATA *mgData;

    micF = FALSE;
    for (packNo = 0; packNo < 4; packNo++) {
        if (GwSystem.mgPack >= GW_MINIGAME_PACK_MAX) {
            GwSystem.mgPack = GW_MINIGAME_PACK_ALL;
        }
        pack = GwSystem.mgPack;
        if (packTbl[packNo].pack == pack) {
            break;
        }
    }
    if (packNo >= 4) {
        packNo = -1;
    }
    no = 0;
    num = 0;
    for (mgData = MgDataTbl; mgData->ovl != (u16)DLL_NONE; mgData++, no++) {
        if (mgData->type != type) {
            continue;
        }
        if (packNo >= 0 && !(mgData->flag & packTbl[packNo].flag)) {
            continue;
        }
        if (!micF && (mgData->flag & MG_FLAG_MIC)) {
            continue;
        }
        if (GwSystem.curTime && (mgData->flag & MG_FLAG_DISABLE_NIGHT)) {
            continue;
        }
        if (!GwSystem.curTime && (mgData->flag & MG_FLAG_DISABLE_DAY)) {
            continue;
        }
        if (MgCallHisCheck(type, no + GW_MGNO_BASE)) {
            continue;
        }
        tempTbl[num++] = no;
    }
    if (num < height) {
        historyOfs = mgCallHisOfsTbl[type];
        for (i = 0; i < mgCallHisSize[type]; i++) {
            historyNo = mgCallHisPtr[type][historyOfs];
            if (historyNo) {
                no = historyNo - GW_MGNO_BASE;
                skipF = FALSE;
                if (packNo >= 0 && !(MgDataTbl[no].flag & packTbl[packNo].flag)) {
                    skipF = TRUE;
                }
                if (!micF && (MgDataTbl[no].flag & MG_FLAG_MIC)) {
                    skipF = TRUE;
                }
                if (GwSystem.curTime && (MgDataTbl[no].flag & MG_FLAG_DISABLE_NIGHT)) {
                    skipF = TRUE;
                }
                if (!GwSystem.curTime && (MgDataTbl[no].flag & MG_FLAG_DISABLE_DAY)) {
                    skipF = TRUE;
                }
                for (j = 0; j < num; j++) {
                    if (no == tempTbl[j]) {
                        break;
                    }
                }
                if (j >= num && !skipF) {
                    tempTbl[num++] = no;
                    if (num >= height) {
                        break;
                    }
                }
            }
            historyOfs = (historyOfs + 1) % mgCallHisSize[type];
        }
    }
    for (i = 0; i < 100; i++) {
        j = mbRandMod(num);
        no = mbRandMod(num);
        historyNo = tempTbl[j];
        tempTbl[j] = tempTbl[no];
        tempTbl[no] = historyNo;
    }
    if (num > height) {
        num = height;
    }
    for (i = 0; i < num; i++) {
        noTbl[i] = tempTbl[i];
    }
    for (; i < height; i++) {
        noTbl[i] = -1;
    }
    return num;
}

static int MgRouletteNumSingleGet(int type, int height, s16 *noTbl)
{
    MGPACKTYPE packTbl[4] = {
        { GW_MINIGAME_PACK_EASY, MG_FLAG_PACK_EASY },
        { GW_MINIGAME_PACK_ACTION, MG_FLAG_PACK_ACTION },
        { GW_MINIGAME_PACK_HARD, MG_FLAG_PACK_SKILL },
        { GW_MINIGAME_PACK_WEIRD, MG_FLAG_PACK_WEIRD },
    };
    s16 partyTbl[128];
    s16 storyTbl[128];
    s16 unownedTbl[128];
    s16 historyNo;
    int i;
    int j;
    int no;
    int num;
    int partyNum;
    int storyNum;
    int packNo;
    BOOL micF;
    MGDATA *mgData;

    micF = FALSE;
    for (packNo = 0; packNo < 4; packNo++) {
        if (GwSystem.mgPack >= GW_MINIGAME_PACK_MAX) {
            GwSystem.mgPack = GW_MINIGAME_PACK_ALL;
        }
        if (packTbl[packNo].pack == GwSystem.mgPack) {
            break;
        }
    }
    if (packNo >= 4) {
        packNo = -1;
    }
    no = 0;
    num = 0;
    storyNum = 0;
    partyNum = 0;
    for (mgData = MgDataTbl; mgData->ovl != (u16)DLL_NONE; mgData++, no++) {
        if (mgData->type != type) {
            continue;
        }
        if (packNo >= 0 && !(mgData->flag & packTbl[packNo].flag)) {
            continue;
        }
        if (!micF && (mgData->flag & MG_FLAG_MIC)) {
            continue;
        }
        if (mgCallSingleKoopaF && (mgData->flag & MG_FLAG_GRPORDER)) {
            continue;
        }
        if (mbSingleMgUnlockGet(no + GW_MGNO_BASE)) {
            storyTbl[storyNum++] = no;
        } else if (GWMgUnlockGet(no + GW_MGNO_BASE)) {
            partyTbl[partyNum++] = no;
        } else {
            unownedTbl[num++] = no;
        }
    }
    if (num < height && partyNum > 0) {
        if (partyNum >= 2) {
            for (i = 0; i < 100; i++) {
                j = i % partyNum;
                no = mbRandMod(partyNum);
                if (j != no) {
                    historyNo = partyTbl[j];
                    partyTbl[j] = partyTbl[no];
                    partyTbl[no] = historyNo;
                }
            }
        }
        for (i = 0; i < height - num && i < partyNum; i++) {
            unownedTbl[num++] = partyTbl[i];
        }
    }
    if (num < height && storyNum > 0) {
        if (storyNum >= 2) {
            for (i = 0; i < 100; i++) {
                j = i % storyNum;
                no = mbRandMod(storyNum);
                if (j != no) {
                    historyNo = storyTbl[j];
                    storyTbl[j] = storyTbl[no];
                    storyTbl[no] = historyNo;
                }
            }
        }
        for (i = 0; i < height - num && i < storyNum; i++) {
            unownedTbl[num++] = storyTbl[i];
        }
    }
    for (i = 0; i < 100; i++) {
        j = mbRandMod(num);
        no = mbRandMod(num);
        historyNo = unownedTbl[j];
        unownedTbl[j] = unownedTbl[no];
        unownedTbl[no] = historyNo;
    }
    if (num > height) {
        num = height;
    }
    for (i = 0; i < num; i++) {
        noTbl[i] = unownedTbl[i];
    }
    for (; i < height; i++) {
        noTbl[i] = -1;
    }
    return num;
}

static int MgRouletteNumMicGet(int type, int height, s16 *noTbl)
{
    MGPACKTYPE packTbl[4] = {
        { GW_MINIGAME_PACK_EASY, MG_FLAG_PACK_EASY },
        { GW_MINIGAME_PACK_ACTION, MG_FLAG_PACK_ACTION },
        { GW_MINIGAME_PACK_HARD, MG_FLAG_PACK_SKILL },
        { GW_MINIGAME_PACK_WEIRD, MG_FLAG_PACK_WEIRD },
    };
    s16 tempTbl[128];
    s16 temp;
    int i;
    int no;
    int num;
    int packNo;
    MGDATA *mgData;

    for (packNo = 0; packNo < 4; packNo++) {
        if (GwSystem.mgPack >= GW_MINIGAME_PACK_MAX) {
            GwSystem.mgPack = GW_MINIGAME_PACK_ALL;
        }
        if (packTbl[packNo].pack == GwSystem.mgPack) {
            break;
        }
    }
    if (packNo >= 4) {
        packNo = -1;
    }
    no = 0;
    num = 0;
    for (mgData = MgDataTbl; mgData->ovl != (u16)DLL_NONE; mgData++, no++) {
        if (mgData->type != type) {
            continue;
        }
        if (packNo >= 0 && !(mgData->flag & packTbl[packNo].flag)) {
            continue;
        }
        if (!(mgData->flag & MG_FLAG_MIC)) {
            continue;
        }
        if (GwSystem.curTime && (mgData->flag & MG_FLAG_DISABLE_NIGHT)) {
            continue;
        }
        if (!GwSystem.curTime && (mgData->flag & MG_FLAG_DISABLE_DAY)) {
            continue;
        }
        tempTbl[num++] = no;
    }
    for (i = 0; i < 100; i++) {
        no = mbRandMod(num);
        packNo = mbRandMod(num);
        temp = tempTbl[no];
        tempTbl[no] = tempTbl[packNo];
        tempTbl[packNo] = temp;
    }
    if (num > height) {
        num = height;
    }
    for (i = 0; i < num; i++) {
        noTbl[i] = tempTbl[i];
    }
    for (; i < height; i++) {
        noTbl[i] = -1;
    }
    return num;
}

static int MgRouletteNumMicSingleGet(int type, int height, s16 *noTbl)
{
    MGPACKTYPE packTbl[4] = {
        { GW_MINIGAME_PACK_EASY, MG_FLAG_PACK_EASY },
        { GW_MINIGAME_PACK_ACTION, MG_FLAG_PACK_ACTION },
        { GW_MINIGAME_PACK_HARD, MG_FLAG_PACK_SKILL },
        { GW_MINIGAME_PACK_WEIRD, MG_FLAG_PACK_WEIRD },
    };
    s16 partyTbl[128];
    s16 storyTbl[128];
    s16 unownedTbl[128];
    s16 temp;
    int i;
    int no;
    int num;
    int partyNum;
    int storyNum;
    int packNo;
    MGDATA *mgData;

    for (packNo = 0; packNo < 4; packNo++) {
        if (GwSystem.mgPack >= GW_MINIGAME_PACK_MAX) {
            GwSystem.mgPack = GW_MINIGAME_PACK_ALL;
        }
        if (packTbl[packNo].pack == GwSystem.mgPack) {
            break;
        }
    }
    if (packNo >= 4) {
        packNo = -1;
    }
    no = 0;
    num = 0;
    storyNum = 0;
    partyNum = 0;
    for (mgData = MgDataTbl; mgData->ovl != (u16)DLL_NONE; mgData++, no++) {
        if (mgData->type != type) {
            continue;
        }
        if (packNo >= 0 && !(mgData->flag & packTbl[packNo].flag)) {
            continue;
        }
        if (!(mgData->flag & MG_FLAG_MIC)) {
            continue;
        }
        if (GwSystem.curTime && (mgData->flag & MG_FLAG_DISABLE_NIGHT)) {
            continue;
        }
        if (!GwSystem.curTime && (mgData->flag & MG_FLAG_DISABLE_DAY)) {
            continue;
        }
        if (mbSingleMgUnlockGet(no + GW_MGNO_BASE)) {
            storyTbl[storyNum++] = no;
        } else if (GWMgUnlockGet(no + GW_MGNO_BASE)) {
            partyTbl[partyNum++] = no;
        } else {
            unownedTbl[num++] = no;
        }
    }
    if (num < height && partyNum > 0) {
        if (partyNum >= 2) {
            for (i = 0; i < 100; i++) {
                no = i % partyNum;
                packNo = mbRandMod(partyNum);
                if (no != packNo) {
                    temp = partyTbl[no];
                    partyTbl[no] = partyTbl[packNo];
                    partyTbl[packNo] = temp;
                }
            }
        }
        for (i = 0; i < height - num && i < partyNum; i++) {
            unownedTbl[num++] = partyTbl[i];
        }
    }
    if (num < height && storyNum > 0) {
        if (storyNum >= 2) {
            for (i = 0; i < 100; i++) {
                no = i % storyNum;
                packNo = mbRandMod(storyNum);
                if (no != packNo) {
                    temp = storyTbl[no];
                    storyTbl[no] = storyTbl[packNo];
                    storyTbl[packNo] = temp;
                }
            }
        }
        for (i = 0; i < height - num && i < storyNum; i++) {
            unownedTbl[num++] = storyTbl[i];
        }
    }
    for (i = 0; i < 100; i++) {
        no = mbRandMod(num);
        packNo = mbRandMod(num);
        temp = unownedTbl[no];
        unownedTbl[no] = unownedTbl[packNo];
        unownedTbl[packNo] = temp;
    }
    if (num > height) {
        num = height;
    }
    for (i = 0; i < num; i++) {
        noTbl[i] = unownedTbl[i];
    }
    for (; i < height; i++) {
        noTbl[i] = -1;
    }
    return num;
}

static void MgRouletteOMExec(OMOBJ *obj)
{
    MGLISTWORK *work = omObjGetWork(obj, MGLISTWORK);
    float weight;
    float time;
    HuVec2f winSize;

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
    switch (work->mode) {
    case 0:
        if (work->time > work->maxTime) {
            work->slideF = FALSE;
            work->mode = 1;
        } else {
            weight = (float)work->time++ / work->maxTime;
            time = sin((M_PI * (90.0f * weight)) / 180.0);
            if ((work->no & 1) == 0) {
                obj->trans.x = -160.0f + (448.0f * time);
            } else {
                obj->trans.x = 736.0f + (-448.0f * time);
            }
        }
        break;
    case 1:
        if (work->no == mgCallFocus) {
            work->mode = 2;
        }
        break;
    case 2:
        if (work->no != mgCallFocus) {
            work->mode = 3;
            work->time = 0;
            work->maxTime = 8;
        }
        break;
    case 3:
        if (work->time > work->maxTime) {
            work->mode = 1;
        } else {
            weight = (float)work->time++ / work->maxTime;
        }
        break;
    case 4:
        if (work->time <= work->maxTime) {
            weight = (float)work->time++ / work->maxTime;
            obj->scale.x = obj->scale.y =
                (0.2f
                    * sin((M_PI * (720.0f * (1.0f - weight))) / 180.0))
                + 1.0f;
        }
        break;
    case 5:
        if (work->time <= work->maxTime) {
            weight = (float)work->time++ / work->maxTime;
            obj->trans.y = obj->rot.y + (weight * (96.0f - obj->rot.y));
            obj->scale.x = obj->scale.y =
                (0.5f * cos((M_PI * (90.0f * weight)) / 180.0)) + 0.5f;
        }
        break;
    }
    mbWinScaleSet(work->winNo, obj->scale.x, obj->scale.y);
    mbWinMesMaxSizeGet(work->winNo, &winSize);
    mbWinPosSet(work->winNo,
        obj->trans.x - ((winSize.x / 2.0f) * obj->scale.x),
        obj->trans.y - ((winSize.y / 2.0f) * obj->scale.y));
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

static int MgCallBattleExec(MGCALLWORK *workP)
{
    static int battleCoinTbl[5] = {
        5,
        10,
        20,
        30,
        50,
    };
    static int battleCoinChanceTbl[5] = { 6, 6, 3, 3, 1 };
    static u32 battleMgSprTbl[6] = {
        DATANUM(DATA_bbattle, MGCALL_BATTLE_FILE_COIN_SPRITE_0),
        DATANUM(DATA_bbattle, MGCALL_BATTLE_FILE_COIN_SPRITE_0),
        DATANUM(DATA_bbattle, MGCALL_BATTLE_FILE_COIN_SPRITE_1),
        DATANUM(DATA_bbattle, MGCALL_BATTLE_FILE_COIN_SPRITE_2),
        DATANUM(DATA_bbattle, MGCALL_BATTLE_FILE_COIN_SPRITE_3),
        DATANUM(DATA_bbattle, MGCALL_BATTLE_FILE_COIN_SPRITE_4),
    };

    int i;
    int coinType;
    int coinTotal;
    int delay;
    int coinNum;
    int speed;
    int nextMaxTime;
    int nextTime;
    BOOL coinHighF;
    int result;
    int battleCoin;
    BOOL fastFocusF;
    int nextDelay;
    int listNum;
    HuVecF pos;
    HuVecF guidePos;
    HuVecF movePos;
    s16 mgTbl[128];
    char str[32];
    int battleCoinOfs;
    s16 playerMaxSteal;
    s16 teamMaxSteal;
    float weight;
    float scale;
    float posY;
    float zRot;
    float velY;

    espKill(workP->sprId[2]);
    workP->sprId[2] = -1;
    mbMusFadeOutSpeed(0, 1000);
    while (mbMusCheck(0)) {
        HuPrcVSleep();
    }
    mbMusPlay(0, 25, 127, 0);
    workP->type = MG_TYPE_BATTLE;
    workP->sprId[1] = espEntry(
        mbBoardDataNumGet(DATANUM(DATA_board, MGCALL_BOARD_FILE_TYPE)), 95, 3);
    mbAudFXPlay(1115);
    posY = 88.0f;
    velY = 6.0f;
    zRot = 0.0f;
    i = 0;
    do {
        weight = i / 30.0f;
        if (weight > 1.0f) {
            weight = 1.0f;
        }
        espPosSet(workP->sprId[1], 288.0f,
            88.0f - (120.0f * sin((M_PI * (90.0f * (1.0f - weight))) / 180.0)));
        if (weight >= 0.6f) {
            if (i == 0) {
                mbAudFXPlay(1116);
            }
            if (zRot > -40.0f) {
                zRot--;
            }
            posY += velY;
            velY += 0.2f;
            espPosSet(workP->sprId[0], 288.0f, posY);
            espZRotSet(workP->sprId[0], zRot);
        }
        HuPrcVSleep();
        i++;
    } while (posY < 576.0f);
    pos.x = 288.0f;
    pos.y = 240.0f;
    pos.z = 800.0f;
    if (!GWTeamFGet()) {
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            mbStatusColorSet(i, STATUS_COLOR_BLUE);
        }
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            mbStatusPosOnGet(i, &pos);
            mbStatusMoveSet(i, NULL, &pos, STATUS_MOVE_SIN, 12);
        }
    } else {
        for (i = 0; i < GW_PLAYER_MAX / 2; i++) {
            mbStatusPosOnGet(i, &pos);
            mbStatusNoMoveSet(i, NULL, &pos, STATUS_MOVE_SIN, 12);
        }
    }
    while (!mbStatusOffCheckAll()) {
        HuPrcVSleep();
    }
    HuPrcSleep(12);
    mbObjMotionShiftSet(workP->guideMdlId, 2, 0.0f, 0.0f,
        HU3D_MOTATTR_LOOP);
    mbAudGuidePlay(950);
    mbAudFXPlay(0);
    mbWinCreate(2, MgCallBattleMesGet(MGCALL_MESSAGE_BATTLE_INTRO),
        mbGuideSpeakerNoGet());
    mbWinTopWait();
    workP->sprId[7] = espEntry(
        mbBoardDataNumGet(
            DATANUM(DATA_board, MGCALL_BOARD_FILE_BATTLE_COIN_LABEL)), 100, 0);
    workP->sprId[8] = espEntry(
        mbBoardDataNumGet(
            DATANUM(DATA_board, MGCALL_BOARD_FILE_BATTLE_COIN_NUMBER)), 99, 0);
    for (i = 0; i < 30u; i++) {
        weight = i / 30.0f;
        pos.x = 288.0f;
        pos.y = 516.0f - (224.0f * weight);
        espPosSet(workP->sprId[7], pos.x, pos.y);
        espPosSet(workP->sprId[8], pos.x + 32.0f, pos.y);
        espPosSet(workP->sprId[1], 288.0f,
            88.0f - (120.0f * sin((M_PI * (90.0f * weight)) / 180.0)));
        HuPrcVSleep();
    }
    fastFocusF = frand() < 0.1f;
    coinType = 0;
    battleCoin = MgCallBattleCoinGet();
    delay = mbRandMod(30) + 60;
    battleCoinOfs = (int)((-7.0f + sqrtf((delay * 8.0f) + 49.0f)) / 2.0f);
    speed = (battleCoin - battleCoinOfs) % 5;
    if (fastFocusF) {
        speed--;
    }
    if (speed < 0) {
        speed += 5;
    }
    nextDelay = (speed * 3) + 45;
    nextTime = nextMaxTime = 3;
    for (i = 0; i < nextDelay + delay; i++) {
        if (--nextTime == 0) {
            coinType = (coinType + 1) % 5;
            if (i > nextDelay) {
                nextMaxTime++;
                nextTime = nextMaxTime;
            } else {
                nextTime = nextMaxTime;
            }
            mbAudFXPlay(1009);
        }
        espBankSet(workP->sprId[8], coinType);
        HuPrcVSleep();
    }
    coinType = battleCoin;
    espBankSet(workP->sprId[8], coinType);
    mbAudFXPlay(1134);
    for (i = 0; i < 90; i++) {
        weight = i / 90.0f;
        scale = (0.2f * sin((M_PI * (1080.0f * (1.0f - weight))) / 180.0)) + 1.0f;
        espScaleSet(workP->sprId[8], scale, scale);
        espPosSet(workP->sprId[8], pos.x + (32.0f * scale), pos.y);
        HuPrcVSleep();
    }
    coinNum = battleCoinTbl[coinType];
    coinTotal = 0;
    coinHighF = TRUE;
    if (!GWTeamFGet()) {
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (mbPlayerCoinGet(i) >= coinNum) {
                coinTotal += coinNum;
                GwPlayer[i].coinBattle = coinNum;
            } else {
                coinTotal += mbPlayerCoinGet(i);
                coinHighF = FALSE;
                playerMaxSteal = mbPlayerCoinGet(i);
                GwPlayer[i].coinBattle = playerMaxSteal;
            }
        }
    } else {
        for (i = 0; i < GW_PLAYER_MAX / 2; i++) {
            if (mbPlayerTeamCoinGet(i) >= coinNum * 2) {
                coinTotal += coinNum * 2;
                GwPlayer[i].coinBattle = coinNum * 2;
            } else {
                coinTotal += mbPlayerTeamCoinGet(i);
                coinHighF = FALSE;
                teamMaxSteal = mbPlayerTeamCoinGet(i);
                GwPlayer[i].coinBattle = teamMaxSteal;
            }
        }
    }
    mbCoinAddAllExec(-battleCoinTbl[coinType], -battleCoinTbl[coinType],
        -battleCoinTbl[coinType], -battleCoinTbl[coinType]);
    for (i = 1; i < 12; i++) {
        scale = cos((M_PI * (90.0f * (i / 12.0f))) / 180.0);
        espScaleSet(workP->sprId[7], scale, scale);
        espScaleSet(workP->sprId[8], scale, scale);
        espPosSet(workP->sprId[8], pos.x + (32.0f * scale), pos.y);
        HuPrcVSleep();
    }
    espDispOff(workP->sprId[7]);
    espDispOff(workP->sprId[8]);
    if (coinHighF) {
        mbAudGuidePlay(949);
        mbWinCreate(2, MgCallBattleMesGet(MGCALL_MESSAGE_BATTLE_COIN_HIGH),
            mbGuideSpeakerNoGet());
    } else {
        mbAudGuidePlay(954);
        mbWinCreate(2, MgCallBattleMesGet(MGCALL_MESSAGE_BATTLE_COIN_LOW),
            mbGuideSpeakerNoGet());
    }
    sprintf(str, "%d", coinTotal);
    mbWinTopInsertMesSet((u32)str, 0);
    mbWinTopWait();
    listNum = MgRouletteNumGet(MG_TYPE_BATTLE, 3, mgTbl);
    espDispOff(workP->sprId[7]);
    espDispOff(workP->sprId[8]);
    result = MgCallBattleSelectExec(workP, listNum, mgTbl);
    if (workP->guideMdlId >= 0) {
        mbObjPosGet(workP->guideMdlId, &pos);
        guidePos = pos;
        guidePos.x -= 200.0f;
        mbObjMotionShiftSet(workP->guideMdlId, 3, 0.0f, 0.0f, FALSE);
        for (i = 1; i <= 45; i++) {
            weight = i / 45.0f;
            if (weight <= 0.25f) {
                zRot = -90.0f
                    * sin((M_PI * (16.0f * weight)) / 180.0);
            } else if (weight < 0.75f) {
                zRot = -90.0f;
            } else {
                zRot = -90.0f
                    * cos((M_PI * (360.0f * (weight - 0.75f))) / 180.0);
            }
            mbev_CapVecChase(
                sin((M_PI * (90.0f * weight)) / 180.0), &pos, &guidePos,
                &movePos);
            mbObjPosSetV(workP->guideMdlId, &movePos);
            mbObjRotSet(workP->guideMdlId, 0.0f, zRot, 0.0f);
            HuPrcVSleep();
        }
        mbObjMotionShiftSet(workP->guideMdlId, 2, 0.0f, 0.0f,
            HU3D_MOTATTR_LOOP);
    }
    mbAudGuidePlay(952);
    mbWinCreate(2, MgCallBattleMesGet(MGCALL_MESSAGE_BATTLE_RESULT),
        mbGuideSpeakerNoGet());
    mbWinTopWait();
    _SetFlag(FLAG_BOARD_MG);
    mbCameraEyeGet(&pos);
    mbCameraMovePos(&pos, NULL, NULL, 80.0f, -1.0f, 60);
    return mgTbl[result];
}

typedef struct MgCallBattlePlayer_s {
    u8 killF : 1;
    s16 cursorSprId;
    s16 charSprId;
    s16 cursorPos;
    s16 cursorPosPrev;
    s16 time;
    s16 maxTime;
    s16 comSelectPos;
    s16 comBtnTime;
} MGCALLBATTLEPLAYER;

static int MgCallBattleSelectExec(MGCALLWORK *workP, int listNum, s16 *list)
{
    static const u32 battleCharFileTbl[11] = {
        DATANUM(DATA_board, MGCALL_BOARD_FILE_BATTLE_CHAR_MARIO),
        DATANUM(DATA_board, MGCALL_BOARD_FILE_BATTLE_CHAR_LUIGI),
        DATANUM(DATA_board, MGCALL_BOARD_FILE_BATTLE_CHAR_PEACH),
        DATANUM(DATA_board, MGCALL_BOARD_FILE_BATTLE_CHAR_YOSHI),
        DATANUM(DATA_board, MGCALL_BOARD_FILE_BATTLE_CHAR_WARIO),
        DATANUM(DATA_board, MGCALL_BOARD_FILE_BATTLE_CHAR_DAISY),
        DATANUM(DATA_board, MGCALL_BOARD_FILE_BATTLE_CHAR_WALUIGI),
        DATANUM(DATA_board, MGCALL_BOARD_FILE_BATTLE_CHAR_KINOPIO),
        DATANUM(DATA_board, MGCALL_BOARD_FILE_BATTLE_CHAR_TERESA),
        DATANUM(DATA_board, MGCALL_BOARD_FILE_BATTLE_CHAR_MINIKOOPA),
        DATANUM(DATA_board, MGCALL_BOARD_FILE_BATTLE_CHAR_KINOPICO),
    };
    static const u32 battleMgDataTbl[6][3] = {
        { MG_NAME_M646,
            DATANUM(DATA_board, MGCALL_BOARD_FILE_BATTLE_M646),
            DATANUM(DATA_board, MGCALL_BOARD_FILE_BATTLE_M646) },
        { MG_NAME_M647,
            DATANUM(DATA_board, MGCALL_BOARD_FILE_BATTLE_M647_DAY),
            DATANUM(DATA_board, MGCALL_BOARD_FILE_BATTLE_M647_NIGHT) },
        { MG_NAME_M649,
            DATANUM(DATA_board, MGCALL_BOARD_FILE_BATTLE_M649),
            DATANUM(DATA_board, MGCALL_BOARD_FILE_BATTLE_M649) },
        { MG_NAME_M664,
            DATANUM(DATA_board, MGCALL_BOARD_FILE_BATTLE_M664_DAY),
            DATANUM(DATA_board, MGCALL_BOARD_FILE_BATTLE_M664_NIGHT) },
        { MG_NAME_M680,
            DATANUM(DATA_board, MGCALL_BOARD_FILE_BATTLE_M680),
            DATANUM(DATA_board, MGCALL_BOARD_FILE_BATTLE_M680) },
        { MG_NAME_M681,
            DATANUM(DATA_board, MGCALL_BOARD_FILE_BATTLE_M681),
            DATANUM(DATA_board, MGCALL_BOARD_FILE_BATTLE_M681) },
    };
    static HuVec2f mgBattlePosTbl[4][3] = {
        { { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f } },
        { { 288.0f, 240.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f } },
        { { 168.0f, 240.0f }, { 408.0f, 240.0f }, { 0.0f, 0.0f } },
        { { 118.0f, 240.0f }, { 288.0f, 240.0f }, { 458.0f, 240.0f } },
    };
    static HuVec2f mgBattleCursorOfsTbl[GW_PLAYER_MAX] = {
        { -56.0f, -16.0f },
        { -38.0f, -48.0f },
        { 6.0f, -48.0f },
        { 24.0f, -16.0f },
    };
    MGCALLBATTLEPLAYER *battlePlayer;
    int i;
    u16 btn;
    int j;
    int cursorDupeNum;
    int ignoreNum;
    int selectNum;
    s16 playerDoneNum;
    u16 padNo;
    MBCAMERA *cameraP;
    s16 helpWinNo;
    MGCALLBATTLEPLAYER playerAll[GW_PLAYER_MAX];
    int ignoreTbl[3];
    int selectTbl[3];
    HuVec2f winPosOfs[3];
    HuVec2f winScale[3];
    HuVec2f scale;
    HuVec2f pos;
    HuVec2f winSize;
    s16 cursorNumTbl[3];
    float posX;
    float posY;
    float weight;
    float time;
    float scaleX;
    float scaleY;
    int result;

    cameraP = mbCameraGet();
    for (i = 0; i < listNum; i++) {
        for (j = 0; j < 6; j++) {
            if (battleMgDataTbl[j][0] == MgDataTbl[list[i]].nameMes) {
                break;
            }
        }
        if (!GwSystem.curTime) {
            workP->battleSprId[i] = espEntry(
                mbBoardDataNumGet(battleMgDataTbl[j][1]), 200, 0);
        } else {
            workP->battleSprId[i] = espEntry(
                mbBoardDataNumGet(battleMgDataTbl[j][2]), 200, 0);
        }
        pos.x = mgBattlePosTbl[listNum][i].x;
        pos.y = mgBattlePosTbl[listNum][i].y;
        espPosSet(workP->battleSprId[i], pos.x, pos.y);
        workP->battleNameSprId[i] = espEntry(
            mbBoardDataNumGet(
                DATANUM(DATA_board, MGCALL_BOARD_FILE_BATTLE_NAME_BACKDROP)),
            150, 0);
        espPosSet(workP->battleNameSprId[i],
            mgBattlePosTbl[listNum][i].x,
            mgBattlePosTbl[listNum][i].y + 80.0f);
        workP->battleWinId[i] = mbWinCreateHelp(battleMgDataTbl[j][0]);
        mbWinPriSet(workP->battleWinId[i], 120);
        mbWinMesMaxSizeGet(workP->battleWinId[i], &winSize);
        winScale[i].x = winScale[i].y = 1.0f;
        if (winSize.x > 160.0f) {
            scale.x = scale.y = 160.0f / winSize.x;
            mbWinScaleSet(workP->battleWinId[i], scale.x, scale.y);
            winSize.x *= scale.x;
            winSize.y *= scale.y;
            winScale[i] = scale;
        }
        winPosOfs[i].x = 2.0f - (winSize.x / 2.0f);
        winPosOfs[i].y = 80.0f - (winSize.y / 2.0f);
        mbWinPosSet(workP->battleWinId[i],
            mgBattlePosTbl[listNum][i].x + winPosOfs[i].x,
            mgBattlePosTbl[listNum][i].y + winPosOfs[i].y);
    }
    for (i = 1; i <= 30; i++) {
        weight = i / 30.0f;
        posY = 480.0f * cos((M_PI * (90.0f * weight)) / 180.0);
        for (j = 0; j < listNum; j++) {
            espPosSet(workP->battleSprId[j], mgBattlePosTbl[listNum][j].x,
                mgBattlePosTbl[listNum][j].y + posY);
            espPosSet(workP->battleNameSprId[j], mgBattlePosTbl[listNum][j].x,
                mgBattlePosTbl[listNum][j].y + 80.0f + posY);
            mbWinPosSet(workP->battleWinId[j],
                winPosOfs[j].x + mgBattlePosTbl[listNum][j].x,
                winPosOfs[j].y + mgBattlePosTbl[listNum][j].y + posY);
            mbWinScaleSet(workP->battleWinId[j], winScale[j].x, winScale[j].y);
        }
        HuPrcVSleep();
    }
    helpWinNo = mbWinCreateHelp(MGCALL_MESSAGE_BATTLE_HELP);
    mbWinMesMaxSizeGet(helpWinNo, &winSize);
    mbWinPosSet(helpWinNo, 288.0f - (winSize.x / 2.0f), 340);
    for (battlePlayer = &playerAll[0], i = 0; i < GW_PLAYER_MAX;
         i++, battlePlayer++) {
        battlePlayer->cursorSprId = espEntry(
            mbBoardDataNumGet(
                DATANUM(DATA_board, MGCALL_BOARD_FILE_BATTLE_PLAYER_CURSOR)),
            (i * 2) + 81, 0);
        battlePlayer->charSprId = espEntry(
            mbBoardDataNumGet(battleCharFileTbl[GwPlayer[i].charNo]),
            (i * 2) + 80, 0);
        battlePlayer->killF = FALSE;
        battlePlayer->cursorPos = battlePlayer->cursorPosPrev = listNum / 2;
        battlePlayer->time = 0;
        battlePlayer->maxTime = 18;
        battlePlayer->comSelectPos = mbRandMod(listNum);
        battlePlayer->comBtnTime = 120 + (frandf() * 60.0f);
        espPosSet(battlePlayer->cursorSprId,
            mgBattleCursorOfsTbl[i].x
                + mgBattlePosTbl[listNum][battlePlayer->cursorPos].x,
            mgBattleCursorOfsTbl[i].y
                + mgBattlePosTbl[listNum][battlePlayer->cursorPos].y);
        espPosSet(battlePlayer->charSprId,
            mgBattleCursorOfsTbl[i].x
                + mgBattlePosTbl[listNum][battlePlayer->cursorPos].x,
            mgBattleCursorOfsTbl[i].y
                + mgBattlePosTbl[listNum][battlePlayer->cursorPos].y - 6.0f);
    }
    for (i = 0; i < listNum; i++) {
        cursorNumTbl[i] = 0;
    }
    HuPrcVSleep(10);
    for (playerDoneNum = 0; playerDoneNum < GW_PLAYER_MAX;) {
        for (j = 0; j < listNum; j++) {
            mbWinScaleSet(workP->battleWinId[j], winScale[j].x, winScale[j].y);
        }
        for (battlePlayer = &playerAll[0], i = 0; i < GW_PLAYER_MAX;
             i++, battlePlayer++) {
            if (battlePlayer->killF) {
                continue;
            }
            if (battlePlayer->cursorPos != battlePlayer->cursorPosPrev) {
                weight = (float)(++battlePlayer->time) / battlePlayer->maxTime;
                time = sin((M_PI * (90.0f * weight)) / 180.0);
                posX = mgBattleCursorOfsTbl[i].x
                    + (mgBattlePosTbl[listNum][battlePlayer->cursorPosPrev].x
                        + (time
                            * (mgBattlePosTbl[listNum][battlePlayer->cursorPos].x
                                - mgBattlePosTbl[listNum]
                                      [battlePlayer->cursorPosPrev]
                                          .x)));
                posY = mgBattlePosTbl[listNum][battlePlayer->cursorPosPrev].y
                    + mgBattleCursorOfsTbl[i].y;
                espPosSet(battlePlayer->cursorSprId, posX, posY);
                espPosSet(battlePlayer->charSprId, posX, posY - 6.0f);
                if (battlePlayer->time >= battlePlayer->maxTime) {
                    battlePlayer->cursorPosPrev = battlePlayer->cursorPos;
                    battlePlayer->time = 0;
                }
            } else {
                if (GwPlayerConf[i].type == 0) {
                    padNo = GwPlayer[i].padNo;
                    btn = HuPadBtnDown[padNo];
                    if (mbPadStkXGet(padNo) < -20) {
                        btn |= PAD_BUTTON_LEFT;
                    } else if (mbPadStkXGet(padNo) > 20) {
                        btn |= PAD_BUTTON_RIGHT;
                    }
                } else {
                    btn = 0;
                    if (battlePlayer->comBtnTime == 0) {
                        btn = PAD_BUTTON_A;
                    } else {
                        if (frand() % 64 == 0) {
                            battlePlayer->comSelectPos = mbRandMod(listNum);
                        }
                        if (battlePlayer->cursorPos != battlePlayer->comSelectPos) {
                            if (battlePlayer->cursorPos
                                > battlePlayer->comSelectPos) {
                                btn |= PAD_BUTTON_LEFT;
                            } else {
                                btn |= PAD_BUTTON_RIGHT;
                            }
                        }
                    }
                }
                if (btn & PAD_BUTTON_LEFT) {
                    if (battlePlayer->cursorPos > 0) {
                        mbAudFXPlay(0);
                        battlePlayer->cursorPos--;
                    }
                }
                if (btn & PAD_BUTTON_RIGHT) {
                    if (battlePlayer->cursorPos < listNum - 1) {
                        mbAudFXPlay(0);
                        battlePlayer->cursorPos++;
                    }
                }
                if (btn & PAD_BUTTON_A) {
                    battlePlayer->killF = TRUE;
                    cursorNumTbl[battlePlayer->cursorPos]++;
                    playerDoneNum++;
                    espDispOff(battlePlayer->cursorSprId);
                    espDispOff(battlePlayer->charSprId);
                    mbAudFXPlay(1);
                }
            }
            if (battlePlayer->comBtnTime) {
                battlePlayer->comBtnTime--;
            }
        }
        HuPrcVSleep();
    }
    mbWinKill(helpWinNo);
    for (battlePlayer = &playerAll[0], i = 0; i < GW_PLAYER_MAX;
         i++, battlePlayer++) {
        espKill(battlePlayer->cursorSprId);
        espKill(battlePlayer->charSprId);
    }
    ignoreNum = 0;
    selectNum = 0;
    for (i = 0; i < listNum; i++) {
        cursorDupeNum = 0;
        for (j = 0; j < listNum; j++) {
            if (i != j && cursorNumTbl[i] < cursorNumTbl[j]) {
                cursorDupeNum++;
            }
        }
        if (cursorDupeNum == 0) {
            ignoreTbl[ignoreNum++] = i;
        } else {
            selectTbl[selectNum++] = i;
        }
    }
    if (ignoreNum == 1) {
        result = ignoreTbl[0];
    } else {
        result = selectTbl[mbRandMod(selectNum)];
    }
    for (j = 0; j < listNum; j++) {
        if (j != result) {
            mbWinPosSet(workP->battleWinId[j], 0, 480);
        }
    }
    for (j = 1; j <= 60; j++) {
        weight = j / 60.0f;
        scaleY = cos((M_PI * (90.0f * weight)) / 180.0);
        scaleX = cos((M_PI * (720.0f * weight)) / 180.0);
        for (cursorDupeNum = 0; cursorDupeNum < listNum; cursorDupeNum++) {
            if (cursorDupeNum != result) {
                espScaleSet(workP->battleSprId[cursorDupeNum],
                    scaleY * scaleX, scaleY);
                espScaleSet(workP->battleNameSprId[cursorDupeNum],
                    scaleY * scaleX, scaleY);
            }
        }
        HuPrcVSleep();
    }
    for (j = 0; j < listNum; j++) {
        if (j != result) {
            espDispOff(workP->battleSprId[j]);
            espDispOff(workP->battleNameSprId[j]);
        }
    }
    for (j = 1; j <= 30; j++) {
        weight = sin((M_PI * (90.0f * (j / 30.0f))) / 180.0);
        pos.x = mgBattlePosTbl[listNum][result].x
            + (weight
                * (mgBattlePosTbl[1][0].x
                    - mgBattlePosTbl[listNum][result].x));
        pos.y = mgBattlePosTbl[listNum][result].y
            + (weight
                * (mgBattlePosTbl[1][0].y
                    - mgBattlePosTbl[listNum][result].y));
        espPosSet(workP->battleSprId[result], pos.x, pos.y);
        espPosSet(workP->battleNameSprId[result], pos.x, pos.y + 80.0f);
        mbWinPosSet(workP->battleWinId[result], pos.x + winPosOfs[result].x,
            pos.y + winPosOfs[result].y);
        HuPrcVSleep();
    }
    return result;
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

static int MgCallBattleCoinGet(void)
{
    static int maxTurnTbl[9] = {
        10,
        15,
        20,
        25,
        30,
        35,
        40,
        45,
        50,
    };
    static int turnTbl[9][2] = {
        { 3, 6 },
        { 5, 10 },
        { 5, 15 },
        { 8, 16 },
        { 10, 20 },
        { 10, 20 },
        { 13, 26 },
        { 15, 30 },
        { 15, 35 },
    };
    static int maxCoinTbl[3] = {
        20,
        30,
        50,
    };
    static int chanceTbl[4][3][5] = {
        { { 10, 85, 5, 0, 0 }, { 10, 75, 15, 0, 0 }, { 10, 65, 25, 0, 0 } },
        { { 5, 70, 20, 5, 0 }, { 5, 60, 25, 10, 0 }, { 5, 50, 30, 15, 0 } },
        { { 5, 60, 20, 10, 5 }, { 5, 45, 30, 15, 5 }, { 5, 35, 30, 20, 10 } },
        { { 5, 45, 30, 15, 5 }, { 5, 30, 35, 20, 10 }, { 5, 20, 35, 25, 15 } },
    };
    int no;
    int i;
    int totalChance;
    int part;
    int maxCoin;
    int chanceGrp;
    int chance;
    int gamePart;
    int gameLen;
    int typeChanceTbl[5];

    for (i = 0; i < 8u; i++) {
        if (GwSystem.turnMax <= maxTurnTbl[i]) {
            break;
        }
    }
    gameLen = i;
    for (i = 0; i < 2; i++) {
        if (GwSystem.turnNo <= turnTbl[gameLen][i]) {
            break;
        }
    }
    gamePart = i;
    part = gamePart;
    maxCoin = mbPlayerMaxCoinGet();
    for (no = 0; no < 3; no++) {
        if (maxCoin < maxCoinTbl[no]) {
            break;
        }
    }
    chanceGrp = no;
    totalChance = 0;
    for (no = 0; no < 5; no++) {
        totalChance += chanceTbl[chanceGrp][part][no];
        typeChanceTbl[no] = totalChance;
    }
    chance = mbRandMod(totalChance);
    for (no = 0; no < 4; no++) {
        if (chance < typeChanceTbl[no]) {
            break;
        }
    }
    return no;
}

void mbev_MgCallKettou(void)
{
    int i;
    int result;
    int sprId;
    float weight;

    _SetFlag(FLAG_MG_CIRCUIT);
    mbPauseDisableSet(TRUE);
    sprId = espEntry(mbBoardDataNumGet(mgCallTypeFileTbl[MG_TYPE_BATTLE]),
        100, 0);
    espPosSet(sprId, 288.0f, -32.0f);
    espBankSet(sprId, MG_TYPE_KUPA);
    for (i = 0; i < 30u; i++) {
        weight = i / 30.0f;
        espPosSet(sprId, 288.0f,
            88.0f - (120.0f
                * sin((M_PI * (90.0f * (1.0f - weight))) / 180.0)));
        HuPrcVSleep();
    }
    _SetFlag(FLAG_BOARD_MG_KETTOU);
    result = MgRouletteExec(MG_TYPE_KETTOU, NULL);
    MgCallCallMg(MG_TYPE_KETTOU, result);
    espKill(sprId);
    HuPrcSleep(-1);
}

void mbev_MgCallDonkey(void)
{
    int i;
    int result;
    int sprId;
    float weight;

    mbPauseDisableSet(TRUE);
    sprId = espEntry(mbBoardDataNumGet(
        DATANUM(DATA_board, MGCALL_BOARD_FILE_DONKEY_TYPE)), 95, 0);
    espPosSet(sprId, 288.0f, -32.0f);
    espDrawNoSet(sprId, 32);
    for (i = 0; i < 30u; i++) {
        weight = i / 30.0f;
        espPosSet(sprId, 288.0f,
            88.0f - (120.0f
                * sin((M_PI * (90.0f * (1.0f - weight))) / 180.0)));
        HuPrcVSleep();
    }
    _SetFlag(FLAG_BOARD_MG_DONKEY);
    result = MgRouletteExec(MG_TYPE_DONKEY, NULL);
    MgCallCallMg(MG_TYPE_DONKEY, result);
    HuPrcSleep(-1);
}

void mbev_MgCallKoopa(void)
{
    int i;
    int result;
    int sprId;
    float weight;

    mbPauseDisableSet(TRUE);
    sprId = espEntry(mbBoardDataNumGet(
        DATANUM(DATA_board, MGCALL_BOARD_FILE_KOOPA_TYPE)), 95, 0);
    espPosSet(sprId, 288.0f, -32.0f);
    espDrawNoSet(sprId, 32);
    for (i = 0; i < 30u; i++) {
        weight = i / 30.0f;
        espPosSet(sprId, 288.0f,
            88.0f - (120.0f
                * sin((M_PI * (90.0f * (1.0f - weight))) / 180.0)));
        HuPrcVSleep();
    }
    _SetFlag(FLAG_BOARD_MG_KOOPA);
    result = MgRouletteExec(MG_TYPE_KUPA, NULL);
    MgCallCallMg(MG_TYPE_KUPA, result);
    HuPrcSleep(-1);
}

static MGCALLSINGLETYPE mgCallSingleTypeTbl[] = {
    { 0, MG_TYPE_4P, DATANUM(DATA_board, MGCALL_BOARD_FILE_TYPE), 0 },
    { 1, MG_TYPE_1VS3, DATANUM(DATA_board, MGCALL_BOARD_FILE_TYPE), 1 },
    { 2, MG_TYPE_2VS2, DATANUM(DATA_board, MGCALL_BOARD_FILE_TYPE), 2 },
    { 3, MG_TYPE_BATTLE, DATANUM(DATA_board, MGCALL_BOARD_FILE_TYPE), 3 },
    { 6, MG_TYPE_KETTOU, DATANUM(DATA_board, MGCALL_BOARD_FILE_TYPE), 4 },
    { 4, MG_TYPE_KUPA, DATANUM(DATA_board, MGCALL_BOARD_FILE_KOOPA_TYPE), 0 },
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
        workP->sprId[0] = sprId = espEntry(
            mbBoardDataNumGet(singleType->dataNum), 100, 0);
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

static void MgCallCallMg(int type, int no)
{
    int id;
    int readStat;

    id = no + GW_MGNO_BASE;
    GWMgUnlockSet(id);
    GwSystem.mgNo = id - GW_MGNO_BASE;
    if (mgCallHisPtr[type] != NULL) {
        mgCallHisPtr[type][mgCallHisOfsTbl[type]] = id;
        if (++mgCallHisOfsTbl[type] >= mgCallHisSize[type]) {
            mgCallHisOfsTbl[type] = 0;
        }
    }
    mgCallDataDir = MgDataTbl[no].dataDir;
    mbDirClose();
    readStat = mbBGRead(DATA_inst);
    if (GwSystem.curTime && GWPartyGet()) {
        mbWipeCreate(WIPE_MODE_OUT,
            WIPE_TYPE_MOON | WIPE_TYPE_FBKEEP, 60);
    } else {
        mbWipeCreate(WIPE_MODE_OUT,
            WIPE_TYPE_SUN | WIPE_TYPE_FBKEEP, 60);
    }
    mbMusFadeOutSpeed(MB_MUS_CHAN_BG, 1000);
    mbMusFadeOutSpeed(MB_MUS_CHAN_FG, 1000);
    mbAudFXStopAll(1000);
    mbBGReadWait(readStat);
    mbWipeSpecialWait();
    GwMgNightF = !GwSystem.curTime;
    mbOvlCall(DLL_instdll);
}

void mbMgCallDataClose(void)
{
    if (mgCallDataDir != -1) {
        HuDataDirClose(mgCallDataDir);
    }
}

void mbev_MgCallTutorial(void)
{
    MGCALLWORK work;
    Mtx explodeMtx;
    HuVecF pos;
    HuVecF posNew;
    HuVecF explodePos2D;
    HuVecF explodePos3D;
    int colorTbl[GW_PLAYER_MAX];
    int i;
    MGCALLWORK *workP;
    s16 winNo;
    MBCAMERA *cameraP;
    MGLISTWORK *listWork;
    OMOBJ *listObj;
    float weight;
    float scale;

    workP = &work;
    cameraP = mbCameraGet();
    mbWipeDissolveFadeIn();
    mbAudFXPlay(0);
    mbWinCreate(MBWIN_TYPE_GUIDE, MGCALL_TUTORIAL_MESSAGE_BEGIN, -1);
    mbWinTopWait();
    mbWinCreate(MBWIN_TYPE_GUIDE, MGCALL_TUTORIAL_MESSAGE_ENDTURN, -1);
    mbWinTopWait();

    pos.x = pos.y = pos.z = 0.0f;
    posNew = pos;
    for (i = 0; i <= 12u; i++) {
        weight = i / 12.0f;
        posNew.y = pos.y + (weight * (119.0f - pos.y));
        HuPrcVSleep();
    }
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mgStatusPos4P[i].y += 64.0f;
        mgStatusPos1Vs3[i].y += 64.0f;
        mgStatusPos2Vs2[i].y += 64.0f;
    }
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbStatusPosOnGet(i, &pos);
        mbStatusMoveSet(i, &pos, (HuVecF *)&mgStatusPos4P[i],
            STATUS_MOVE_SIN, 15);
    }
    while (!mbStatusOffCheckAll()) {
        HuPrcVSleep();
    }
    mbWinCreate(MBWIN_TYPE_RESULT, MGCALL_TUTORIAL_MESSAGE_STATUS, -1);
    mbWinTopWait();
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        colorTbl[i] = STATUS_COLOR_BLUE;
    }
    SetupTeam(colorTbl);
    workP->sprId[2] = espEntry(
        mbBoardDataNumGet(DATANUM(DATA_board, MGCALL_BOARD_FILE_VERSUS)),
        100, 0);
    espPosSet(workP->sprId[2], 288.0f, 304.0f);
    for (i = 0; i < 30u; i++) {
        weight = i / 30.0f;
        scale = 1.5 + sin((M_PI * (360.0f * weight)) / 180.0);
        if (weight < 0.2f) {
            scale *= weight / 0.2f;
        }
        espScaleSet(workP->sprId[2], scale, scale);
        HuPrcVSleep();
    }
    mbWinCreate(MBWIN_TYPE_RESULT, MGCALL_TUTORIAL_MESSAGE_4P, -1);
    mbWinTopWait();
    colorTbl[1] = STATUS_COLOR_RED;
    colorTbl[3] = STATUS_COLOR_RED;
    SetupTeam(colorTbl);
    mbWinCreate(MBWIN_TYPE_RESULT, MGCALL_TUTORIAL_MESSAGE_GROUP, -1);
    mbWinTopWait();
    mbWinCreate(MBWIN_TYPE_RESULT, MGCALL_TUTORIAL_MESSAGE_2VS2, -1);
    mbWinTopWait();
    colorTbl[0] = STATUS_COLOR_RED;
    SetupTeam(colorTbl);
    mbWinCreate(MBWIN_TYPE_RESULT, MGCALL_TUTORIAL_MESSAGE_1VS3, -1);
    mbWinTopWait();
    colorTbl[0] = STATUS_COLOR_BLUE;
    colorTbl[1] = STATUS_COLOR_BLUE;
    SetupTeam(colorTbl);
    mbWinCreate(MBWIN_TYPE_RESULT, MGCALL_TUTORIAL_MESSAGE_3VS1, -1);
    mbWinTopWait();
    mbWinCreate(MBWIN_TYPE_RESULT, MGCALL_TUTORIAL_MESSAGE_TYPE, -1);
    mbWinTopWait();
    mbWinCreate(MBWIN_TYPE_RESULT, MGCALL_TUTORIAL_MESSAGE_CALL, -1);
    mbWinTopWait();

    workP->sprId[0] = espEntry(
        mbBoardDataNumGet(mgCallTypeFileTbl[1]), 1500, 0);
    espPosSet(workP->sprId[0], 288.0f, -32.0f);
    espDrawNoSet(workP->sprId[0], 32);
    for (i = 0; i < 30u; i++) {
        weight = i / 30.0f;
        espPosSet(workP->sprId[0], 288.0f,
            88.0f - (120.0f
                * sin((M_PI * (90.0f * (1.0f - weight))) / 180.0)));
        HuPrcVSleep();
    }
    for (i = 0; i < 60u; i++) {
        weight = i / 60.0f;
        scale = 1.5
            + (0.8 * sin((M_PI
                * (weight * (720.0f + (720.0f * weight)))) / 180.0));
        espScaleSet(workP->sprId[2], scale, scale);
        HuPrcVSleep();
    }
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbStatusPosGet(i, &pos);
        if (pos.x >= 0.0f) {
            pos.x = 704.0f;
        } else {
            pos.x = -128.0f;
        }
        mbStatusMoveSet(i, NULL, &pos, STATUS_MOVE_SIN, 15);
    }
    workP->sprId[3] = mbObjCreate(
        mbBoardDataNumGet(DATANUM(DATA_board,
            MGCALL_BOARD_FILE_EFFECT_EXPLODE)), NULL, FALSE);
    mbObjLayerSet(workP->sprId[3], 6);
    explodePos2D.x = 288.0f;
    explodePos2D.y = 304.0f;
    explodePos2D.z = 500.0f;
    mbPos2Dto3D(&explodePos2D, &explodePos3D);
    mbObjPosSetV(workP->sprId[3], &explodePos3D);
    C_MTXLookAt(explodeMtx, &cameraP->eye, &cameraP->up, &explodePos3D);
    PSMTXInverse(explodeMtx, explodeMtx);
    explodeMtx[0][3] = explodeMtx[1][3] = explodeMtx[2][3] = 0.0f;
    mbObjMtxSet(workP->sprId[3], &explodeMtx);
    espKill(workP->sprId[2]);
    workP->sprId[2] = -1;
    mbAudFXPlay(MSM_SE_BRD00_109);
    for (i = 0; i < 30u; i++) {
        weight = i / 30.0f;
        scale = ((1.0f + (5.0f * weight))
            + (0.5 * sin((M_PI * (16.0f * (weight * 360.0f))) / 180.0)))
            * 0.4;
        mbObjScaleSet(workP->sprId[3], scale, scale, scale);
        if (i >= 25) {
            mbObjAlphaSet(workP->sprId[3],
                (int)(255.0 * cos((M_PI
                    * (90.0f * ((i - 25) / 5.0f))) / 180.0)));
        }
        HuPrcVSleep();
    }
    mbObjKill(workP->sprId[3]);
    workP->sprId[3] = -1;
    while (!mbStatusOffCheckAll()) {
        HuPrcVSleep();
    }
    MgRouletteExec(MG_TYPE_1VS3, workP);
    winNo = mbWinCreate(MBWIN_TYPE_RESULT, MGCALL_TUTORIAL_MESSAGE_BATTLE, -1);
    mbWinWait(winNo);
    winNo = mbWinCreate(MBWIN_TYPE_RESULT, MGCALL_TUTORIAL_MESSAGE_COIN, -1);
    mbWinWait(winNo);
    mbWipeDissolveFadeOut();
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if ((u32)mgListObj[i] != 0) {
            listObj = mgListObj[i];
            listWork = omObjGetWork(listObj, MGLISTWORK);
            listWork->killF = TRUE;
        }
    }
    espKill(workP->sprId[0]);
}

static void MgCallVsEffCreate(void)
{
    MGCALLVSEFFWORK *work;
    int i;

    mgCallVsEffOMObj = omAddObjEx(mbObjMan,
        MGCALL_VS_EFFECT_OBJECT_PRIORITY, 0, 0, -1, MgCallVsEffOMExec);
    work = HuMemDirectMallocNum(HEAP_HEAP,
        sizeof(MGCALLVSEFFWORK) * MGCALL_VS_EFFECT_COUNT, HU_MEMNUM_OVL);
    mgCallVsEffOMObj->data = work;
    memset(work, 0, sizeof(MGCALLVSEFFWORK) * MGCALL_VS_EFFECT_COUNT);
    mgCallVsEffOMObj->work[0] = mgCallVsEffOMObj->work[1]
        = mgCallVsEffOMObj->work[2] = mgCallVsEffOMObj->work[3] = 0;
    for (i = 0; i < MGCALL_VS_EFFECT_COUNT; i++, work++) {
        work->activeF = FALSE;
        work->sprId = espEntry(
            mbBoardDataNumGet(DATANUM(
                DATA_board, MGCALL_BOARD_FILE_VERSUS_EFFECT)),
            MGCALL_VS_EFFECT_SPRITE_PRIORITY, i % 4);
        espDispOff(work->sprId);
        work->time = 0;
        work->maxTime = 0;
        work->scaleStep = 1.0f;
        work->scaleBase = 1.0f;
        work->zRotStep = 0.0f;
    }
}

static void MgCallVsEffOMExec(OMOBJ *obj)
{
    MGCALLVSEFFWORK *work = obj->data;
    int i;
    float weight;
    float scale;

    if (obj->work[3] != 0 && MgCallVsEffNumGet() <= 0) {
        mgCallVsEffOMObj = NULL;
    }
    if (mbExitCheck() || mgCallVsEffOMObj == NULL) {
        for (i = 0; i < MGCALL_VS_EFFECT_COUNT; i++, work++) {
            espKill(work->sprId);
        }
        omDelObjEx(mbObjMan, obj);
        mgCallVsEffOMObj = NULL;
        return;
    }
    for (i = 0; i < MGCALL_VS_EFFECT_COUNT; i++, work++) {
        if (work->activeF) {
            weight = (float)++work->time / work->maxTime;
            PSVECAdd(&work->pos, &work->vel, &work->pos);
            work->zRot += work->zRotStep;
            scale = work->scaleBase + (weight * work->scaleStep);
            espPosSet(work->sprId, work->pos.x, work->pos.y);
            espScaleSet(work->sprId,
                work->scaleX * scale, work->scaleY * scale);
            espZRotSet(work->sprId, work->zRot);
            espTPLvlSet(work->sprId,
                cos((M_PI * (90.0f * weight)) / 180.0));
            if (weight >= 1.0f) {
                work->activeF = FALSE;
                espDispOff(work->sprId);
            }
        }
    }
}

static void MgCallVsEffPosSet(float x, float y)
{
    MGCALLVSEFFWORK *work = mgCallVsEffOMObj->data;
    int i;
    float angle;
    float speed;
    GXColor color;

    for (i = 0; i < MGCALL_VS_EFFECT_COUNT; i++, work++) {
        if (!work->activeF) {
            angle = 360.0f * frandf();
            speed = 15.0f * (0.3f + (0.7f * frandf()));
            work->activeF = TRUE;
            work->time = 0;
            work->maxTime = (int)(60.0f * (0.5f + (0.5f * frandf())));
            work->scaleStep = 0.5f + (0.5f * frandf());
            work->scaleBase = 0.2f + (0.2f * frandf());
            work->zRotStep = 3.0f * (frandf() - 0.5f);
            work->pos.x = x;
            work->pos.y = y;
            work->vel.x = speed * sin((M_PI * angle) / 180.0);
            work->vel.y = speed * cos((M_PI * angle) / 180.0);
            work->scaleX = 1.0f + frandf();
            work->scaleY = 1.0f + frandf();
            work->zRot = frandf();
            mbev_CapEffColorSet(&color, frand() % 7);
            espPosSet(work->sprId, work->pos.x, work->pos.y);
            espScaleSet(work->sprId,
                work->scaleX * work->scaleBase,
                work->scaleY * work->scaleBase);
            espZRotSet(work->sprId, work->zRot);
            espColorSet(work->sprId, color.r, color.g, color.b);
            espTPLvlSet(work->sprId, 1.0f);
            espDispOn(work->sprId);
        }
    }
}

static void MgCallVsEffKill(void)
{
    mgCallVsEffOMObj = NULL;
}

static int MgCallVsEffNumGet(void)
{
    MGCALLVSEFFWORK *work;
    int i;
    int num;

    if (mgCallVsEffOMObj == NULL) {
        return 0;
    }
    work = mgCallVsEffOMObj->data;
    num = 0;
    for (i = 0; i < MGCALL_VS_EFFECT_COUNT; i++, work++) {
        if (work->activeF) {
            num++;
        }
    }
    return num;
}

int mbMgCallVsEffCreate(void)
{
    MGCALLVSEFFWORK *work;
    int i;

    mgCallVsEffOMObj = omAddObjEx(mbObjMan,
        MGCALL_VS_EFFECT_OBJECT_PRIORITY, 0, 0, -1, MgCallVsEffOMExec);
    work = HuMemDirectMallocNum(HEAP_HEAP,
        sizeof(MGCALLVSEFFWORK) * MGCALL_VS_EFFECT_COUNT, HU_MEMNUM_OVL);
    mgCallVsEffOMObj->data = work;
    memset(work, 0, sizeof(MGCALLVSEFFWORK) * MGCALL_VS_EFFECT_COUNT);
    mgCallVsEffOMObj->work[0] = mgCallVsEffOMObj->work[1]
        = mgCallVsEffOMObj->work[2] = mgCallVsEffOMObj->work[3] = 0;
    for (i = 0; i < MGCALL_VS_EFFECT_COUNT; i++, work++) {
        work->activeF = FALSE;
        work->sprId = espEntry(
            mbBoardDataNumGet(DATANUM(
                DATA_board, MGCALL_BOARD_FILE_VERSUS_EFFECT)),
            MGCALL_VS_EFFECT_SPRITE_PRIORITY, i % 4);
        espDispOff(work->sprId);
        work->time = 0;
        work->maxTime = 0;
        work->scaleStep = 1.0f;
        work->scaleBase = 1.0f;
        work->zRotStep = 0.0f;
    }
    MgCallVsEffPosSet(288.0f, 240.0f);
    mgCallVsEffOMObj->work[3] = TRUE;
    return 60;
}

BOOL mbMgCallSingleOnCheck(void)
{
    return TRUE;
}

int mbMgRouletteNumGet(int type)
{
    static int defaultHeightTbl[MG_TYPE_MAX] = {
        4, 3, 3, 3, 3, 3, 3, 3, 3,
    };
    s16 noTbl[128];
    int height;

    height = defaultHeightTbl[type];
    height = MgRouletteNumGet(type, height, noTbl);
    return height;
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
