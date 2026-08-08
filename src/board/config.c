#include "dolphin/math.h"
#include "messdir_enum.h"

extern inline float fabsf(float x)
{
    return fabs(x);
}

#include "game/board/main.h"
#include "game/board/object.h"
#include "game/board/player.h"
#include "game/board/window.h"
#include "game/data.h"
#include "game/disp.h"
#include "game/esprite.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/process.h"
#include "game/sprite.h"

#include "dolphin/gx.h"
#include "dolphin/mtx.h"

extern void *mbMalloc(s32 size);
extern void *mbMallocFlush(s32 size);
extern void *mbMallocFlushModel(s32 size);
extern void mbNormPosto2D(HuVecF *src, HuVecF *dst);
extern void mbNormPosto3D(HuVecF *src, s16 cameraMask, HuVecF *dst);
extern void mbPos3DtoNorm(HuVecF *src, s16 cameraMask, HuVecF *dst);
extern float mbSinDeg(float angle);
extern float mbCosDeg(float angle);
extern void HuPadRumbleAllStop(void);
extern s32 HuMCProbe(s32 chan);
extern s32 HuMCMicGet(void);
extern void HuMCMicSet(s32 value);

typedef struct PausePanelWork_s {
int modelId;              /* offset 0 */
int batsuModelId;         /* offset 4 */
int sprId;                /* offset 8 */
ANIMDATA *anim;           /* offset 12 */
HU3D_ANIMID animId[2];    /* offset 16 */
HuVecF pos;               /* offset 20 */
HuVecF posStart;          /* offset 32 */
HuVecF posTarget;         /* offset 44 */
BOOL batsuF;              /* offset 56 */
float scale;              /* offset 60 */
float scaleStart;         /* offset 64 */
float scaleTarget;        /* offset 68 */
float scaleBase;          /* offset 72 */
s16 bank;                 /* offset 76 */
s16 motion;               /* offset 78 */
s16 time;                 /* offset 80 */
s16 maxTime;              /* offset 82 */
s16 delay;                /* offset 84 */
s16 animTime;             /* offset 86 */
s16 animMaxTime;          /* offset 88 */
} PAUSE_PANEL_WORK;

typedef struct PausePadWork_s {
    s32 padNo;
    s32 port;
    s32 playerNo;
    BOOL activeF;
} PAUSE_PAD_WORK;

typedef struct ConfigMenuWork_s {
    s32 value;
    s32 valueMin;
    s32 valueMax;
    s16 panelId;
    s16 labelPanelId;
    BOOL enabled;
    s32 initialValue;
} CONFIG_MENU_WORK;

typedef struct PauseCursorWork_s {
    HuVecF pos;
    HuVecF posStart;
    HuVecF posDelta;
    BOOL activeF;
    s32 cursorNo;
    s32 mask;
    s32 cursorPos;
    s32 moveTime;
    s32 maxMoveTime;
    float alpha[4];
    s16 sprId[4];
    s16 hiliteSprId[4];
} PAUSE_CURSOR_WORK;

typedef struct PauseWork_s {
    s32 playerNo;
    s32 cursorPos;
    s32 state08;
    s32 state0C;
    s32 activeF;
    s32 selectedRow;
    s32 selectedColumn;
    s32 padWinNo;
    s32 helpWinNo;
    s32 state24;
    s32 state28;
    s32 state2C;
    s32 state30;
    s32 state34;
    s32 state38;
    s32 talkTime;
    s32 prevTalkTime;
    CONFIG_MENU_WORK menu[14];
    PAUSE_CURSOR_WORK cursor;
    PAUSE_PAD_WORK padWork[GW_PLAYER_MAX];
} PAUSE_WORK;

typedef struct ConfigPadWork_s {
    s32 padNo;
    s32 playerNo;
    s32 comDif;
    BOOL activeF;
    HuVecF pos;
    s16 panelId;
    s16 sprId[3];
} CONFIG_PAD_WORK;

static HuVecF playerPos;
static PAUSE_WORK pauseWork;
static BOOL playerDispF[GW_PLAYER_MAX];

static HUPROCESS *configProc;
static s32 configPadDisable;
static HUPROCESS *pauseGuideProc;
static PAUSE_PANEL_WORK *pausePanelWork;
static BOOL pauseGuideKillF;
static s32 pauseDispCopyModelId;
static s32 pauseDispCopyCounter;
static void *pauseDispCopyFb;
static s16 pausePlayer;
static s32 configResult;
static BOOL configDoneF;

static s32 mesSpeedAnmNo[4] = {
    GW_MESS_SPEED_SLOW,
    GW_MESS_SPEED_NORMAL,
    GW_MESS_SPEED_FAST,
    GW_MESS_SPEED_FAST
};
static HuVecF pauseGuidePos = { -0.7f, -0.75f, -750.0f };
static HuVecF pauseGuideQuitPos = { 0.0f, -0.75f, -500.0f };
static s32 pauseMicValueTbl[3] = { 1, 0, 2 };
static s16 pauseGridPosTbl[16] = {
    0, 0, 1, 0, 2, 0, 3, 0,
    0, 1, 1, 1, 2, 1, 3, 1
};
static s32 pausePanelFileTbl[8] = {
    DATANUM(DATA_bpause6, 3), DATANUM(DATA_bpause6, 4),
    DATANUM(DATA_bpause6, 5), DATANUM(DATA_bpause6, 6),
    DATANUM(DATA_bpause6, 7), DATANUM(DATA_bpause6, 8),
    DATANUM(DATA_bpause6, 9), DATANUM(DATA_bpause6, 10)
};
static s32 pausePanelLabelFileTbl[8] = {
    DATANUM(DATA_bpause6, 11), DATANUM(DATA_bpause6, 12),
    DATANUM(DATA_bpause6, 13), DATANUM(DATA_bpause6, 14),
    DATANUM(DATA_bpause6, 15), DATANUM(DATA_bpause6, 16),
    DATANUM(DATA_bpause6, 17), DATANUM(DATA_bpause6, 18)
};
static s16 pauseValueNumTbl[8] = { 1, 2, 2, 5, 2, 3, 3, 1 };
static s16 pauseBatsuValueTbl[8] = { -1, 1, 1, -1, 1, -1, 1, -1 };
static u32 pauseWinMesTbl[8][5] = {
    MESSNUM(MESS_BOARD_PAUSE, 0), MESSNUM(MESS_BOARD_PAUSE, 1),
    MESSNUM(MESS_BOARD_PAUSE, 2), 0, 0,
    MESSNUM(MESS_BOARD_PAUSE, 3), MESSNUM(MESS_BOARD_PAUSE, 4), 0, 0, 0,
    MESSNUM(MESS_BOARD_PAUSE, 5), MESSNUM(MESS_BOARD_PAUSE, 6), 0, 0, 0,
    MESSNUM(MESS_BOARD_PAUSE, 7), MESSNUM(MESS_BOARD_PAUSE, 8),
    MESSNUM(MESS_BOARD_PAUSE, 9), MESSNUM(MESS_BOARD_PAUSE, 10),
    MESSNUM(MESS_BOARD_PAUSE, 23),
    MESSNUM(MESS_BOARD_PAUSE, 11), MESSNUM(MESS_BOARD_PAUSE, 12), 0, 0, 0,
    MESSNUM(MESS_BOARD_PAUSE, 15), MESSNUM(MESS_BOARD_PAUSE, 14),
    MESSNUM(MESS_BOARD_PAUSE, 13), 0, 0,
    MESSNUM(MESS_BOARD_PAUSE, 16), MESSNUM(MESS_BOARD_PAUSE, 17),
    MESSNUM(MESS_BOARD_PAUSE, 18), 0, 0,
    MESSNUM(MESS_BOARD_PAUSE, 19), MESSNUM(MESS_BOARD_PAUSE, 40), 0, 0, 0
};
static u32 pauseSingleWinMesTbl[8][5] = {
    MESSNUM(MESS_BOARD_PAUSE, 42), MESSNUM(MESS_BOARD_PAUSE, 1),
    MESSNUM(MESS_BOARD_PAUSE, 2), 0, 0,
    MESSNUM(MESS_BOARD_PAUSE, 3), MESSNUM(MESS_BOARD_PAUSE, 4), 0, 0, 0,
    MESSNUM(MESS_BOARD_PAUSE, 5), MESSNUM(MESS_BOARD_PAUSE, 43), 0, 0, 0,
    MESSNUM(MESS_BOARD_PAUSE, 7), MESSNUM(MESS_BOARD_PAUSE, 8),
    MESSNUM(MESS_BOARD_PAUSE, 9), MESSNUM(MESS_BOARD_PAUSE, 10),
    MESSNUM(MESS_BOARD_PAUSE, 23),
    MESSNUM(MESS_BOARD_PAUSE, 11), MESSNUM(MESS_BOARD_PAUSE, 12), 0, 0, 0,
    MESSNUM(MESS_BOARD_PAUSE, 15), MESSNUM(MESS_BOARD_PAUSE, 14),
    MESSNUM(MESS_BOARD_PAUSE, 13), 0, 0,
    MESSNUM(MESS_BOARD_PAUSE, 16), MESSNUM(MESS_BOARD_PAUSE, 17),
    MESSNUM(MESS_BOARD_PAUSE, 18), 0, 0,
    MESSNUM(MESS_BOARD_PAUSE, 40), MESSNUM(MESS_BOARD_PAUSE, 40), 0, 0, 0
};
static HuVecF pauseGuidePos2 = { -0.7f, -0.75f, -750.0f };
static char configExitMessage[] =
    "-------------------- Config Exit! ----------------------\n";
static u32 pausePadWinMesTbl[4] = {
    MESSNUM(MESS_BOARD_PAUSE, 21), MESSNUM(MESS_BOARD_PAUSE, 22),
    MESSNUM(MESS_BOARD_PAUSE, 22), MESSNUM(MESS_BOARD_PAUSE, 22)
};
static float pausePadWinPosTbl[4][2] = {
    { 64.0f, 288.0f },
    { 64.0f, 288.0f },
    { 0.0f, 196.0f },
    { 0.0f, 212.0f }
};
static s32 pauseCharPanelFileTbl[17] = {
    DATANUM(DATA_bpause6, 20), DATANUM(DATA_bpause6, 21),
    DATANUM(DATA_bpause6, 22), DATANUM(DATA_bpause6, 23),
    DATANUM(DATA_bpause6, 24), DATANUM(DATA_bpause6, 25),
    DATANUM(DATA_bpause6, 26), DATANUM(DATA_bpause6, 27),
    DATANUM(DATA_bpause6, 28), DATANUM(DATA_bpause6, 29),
    DATANUM(DATA_bpause6, 30), DATANUM(DATA_bpause6, 29),
    DATANUM(DATA_bpause6, 29), DATANUM(DATA_bpause6, 29),
    DATANUM(DATA_bpause6, 19), DATANUM(DATA_bpause6, 31),
    DATANUM(DATA_bpause6, 31)
};
static s16 configPadSprOfsTbl[6] = { 0, 0, 20, -30, 42, -70 };
static s16 pauseCursorSprOfsTbl[2][4][2] = {
    { { 0, -62 }, { 0, 68 }, { 62, 0 }, { -62, 0 } },
    { { 0, -20 }, { 0, 20 }, { 43, -60 }, { -45, -60 } }
};
static s32 pauseCursorMaskTbl[4] = { 8, 4, 2, 1 };
static const s32 pauseCursorBankTbl[4] = { 1, 3, 2, 0 };

static void ConfigKill(void);
static void ConfigMain(void);
static void ConfigOpen(void);
static void ConfigGrowWait(void);
static void ConfigWinUpdate(s32 index);
static void ConfigPadWinSet(s32 index);
static void ConfigPadSprSet(CONFIG_PAD_WORK *workP);
static void ConfigPadTimeSet(CONFIG_PAD_WORK *workP, float scale);
static void ConfigSettingRead(void);
static void ConfigSettingWrite(void);
BOOL mbPausePanelFreezeGet(s16 panelId);
s16 mbPausePanelCreate(int dataNum, unsigned int espDataNum);
void mbPausePanelKill(s16 panelId);
void mbPausePanelPosSet(s16 panelId, float x, float y);
void mbPausePanelPosGet(s16 panelId, HuVecF *pos);
float mbPausePanelScaleGet(s16 panelId);
void mbPausePanelBatsuSet(s16 panelId, BOOL batsuF);
void mbPausePanelGrowSet(s16 panelId, int time, int delay, float scale);
void mbPausePanelShrinkSet(s16 panelId, int time, int delay);
void mbPausePanelBankSet(s16 panelId, int bank);
static BOOL PausePlayerComRead(s32 playerNo);
static void ConfigExec(void);
static void ConfigClose(s32 result);
static void PauseCursorCreate(void);
static void PauseCursorKill(void);
static void PauseCursorHiliteSet(s32 cursorNo, s32 cursorPos, s32 mask);
static void PauseCursorPosNextSet(s32 time, float x, float y);
static int PauseCursorMove(void);
static BOOL PausePadCheck(s32 padNo);
void mbPauseGuideMoveSet(MBMODELID modelId, s32 time, HuVecF *posP,
    HuVecF *posNormP);
static void PauseDispCopyDraw(HU3D_MODEL *modelP, Mtx *mtx);
static void PauseGuideMain(void);
static void PauseGuideDestroy(void);
static BOOL GWStorySingleCheck(void);

BOOL mbConfigExec(int playerNo, MBMODELID modelId)
{
    pausePlayer = modelId;
    mbObjPosGet(pausePlayer, &playerPos);
    configDoneF = FALSE;
    configResult = FALSE;
    memset(&pauseWork, 0, sizeof(PAUSE_WORK));
    pauseWork.padWinNo = pauseWork.helpWinNo = -1;
    pauseWork.playerNo = playerNo;
    pauseWork.cursorPos = -1;
    pauseWork.talkTime = pauseWork.prevTalkTime = 0;
    ConfigSettingRead();
    configProc = HuPrcChildCreate(ConfigMain, 8210, 14336, 0, mbMainProc);
    HuPrcSetStat(configProc, HU_PRC_STAT_PAUSE_ON | HU_PRC_STAT_UPAUSE_ON);
    HuPrcDestructorSet2(configProc, ConfigKill);
    while (!configDoneF) {
        HuPrcVSleep();
    }
    ConfigSettingWrite();
    return configResult;
}

static void ConfigKill(void)
{
    CONFIG_MENU_WORK *menuP;
    int i;

    menuP = pauseWork.menu;
    for (i = 0; i < 8; i++, menuP++) {
        if (menuP->panelId != 0) {
            mbPausePanelKill((s16)menuP->panelId);
            menuP->panelId = 0;
        }
    }
    configProc = NULL;
    configDoneF = TRUE;
}

static void ConfigMain(void)
{
    int i;

    PausePlayerComRead(1);
    ConfigOpen();
    ConfigExec();
    if (pauseWork.helpWinNo >= 0) {
        mbWinKill((s16)pauseWork.helpWinNo);
        pauseWork.helpWinNo = -1;
    }
    ConfigPadWinSet(-1);
    for (i = 0; i < 8; i++) {
        pauseWork.menu[i].enabled = TRUE;
    }
    ConfigClose(2);
    PauseCursorKill();
    pauseWork.selectedRow = 0;
    HuPrcEnd();
}

static void ConfigOpen(void)
{
    int i;
    float x;
    float y;
    CONFIG_MENU_WORK *menuP;

    for (i = 0; i < 8; i++) {
        pauseWork.menu[i].panelId = 0;
    }
    pauseWork.activeF = FALSE;
    for (i = 0; i < 8; i++) {
        menuP = &pauseWork.menu[i];
        menuP->value = i;
        menuP->panelId = mbPausePanelCreate(pausePanelFileTbl[i],
            pausePanelLabelFileTbl[i]);
        if (menuP->initialValue == 0
            || menuP->valueMin == pauseBatsuValueTbl[i]) {
            mbPausePanelBatsuSet(menuP->panelId, TRUE);
        }
        x = 0.4f * ((float)pauseGridPosTbl[i * 2] - 1.5f);
        y = 0.3f + (-0.5f * ((float)pauseGridPosTbl[(i * 2) + 1] - 0.5f));
        mbPausePanelPosSet(menuP->panelId, x, y);
        mbPausePanelGrowSet(menuP->panelId, 16, i * 2, 1.0f);
        menuP->valueMax = pauseValueNumTbl[i];
        mbPausePanelBankSet(menuP->panelId, menuP->valueMin);
    }
    PauseCursorCreate();
    mbPauseGuideMoveSet(pausePlayer, 20, NULL, &pauseGuidePos);
    ConfigGrowWait();
}

static void ConfigGrowWait(void)
{
    BOOL doneF = FALSE;
    CONFIG_MENU_WORK *menuP;
    int i;

    do {
        doneF = TRUE;
        menuP = pauseWork.menu;
        for (i = 0; i < 8; i++, menuP++) {
            if (menuP->panelId != 0
                && !mbPausePanelFreezeGet((s16)menuP->panelId)) {
                doneF = FALSE;
            }
        }
        if (!doneF) {
            HuPrcVSleep();
        }
    } while (!doneF);
}

BOOL mbPausePanelFreezeGet(s16 panelId)
{
    PAUSE_PANEL_WORK *work = &pausePanelWork[panelId];
    BOOL freezeF = FALSE;

    if (work->motion == 0 && work->animMaxTime == 0) {
        freezeF = TRUE;
    }
    return freezeF;
}

static void ConfigClose(s32 result)
{
    BOOL doneF;
    s32 i;
    CONFIG_MENU_WORK *menuP;
    HuVecF pos;

    menuP = pauseWork.menu;
    for (i = 0; i < 8; i++, menuP++) {
        if (menuP->enabled && menuP->panelId != 0) {
            pos.x = 0.4f * ((float)pauseGridPosTbl[i * 2] - 1.5f);
            pos.y = 0.3f
                + (-0.5f
                    * ((float)pauseGridPosTbl[(i * 2) + 1] - 0.5f));
            pos.z = 0.0f;
            mbPausePanelPosSet(menuP->panelId, pos.x, pos.y);
            mbPausePanelShrinkSet(menuP->panelId, 16, 0);
        }
    }
    if (result != 0) {
        if (result == 1) {
            mbPauseGuideMoveSet(pausePlayer, 20, NULL, &pauseGuideQuitPos);
        } else {
            mbPauseGuideMoveSet(pausePlayer, 20, &playerPos, NULL);
        }
    }
    do {
        doneF = TRUE;
        menuP = pauseWork.menu;
        for (i = 0; i < 8; i++, menuP++) {
            if (menuP->panelId != 0
                && !mbPausePanelFreezeGet(menuP->panelId)) {
                doneF = FALSE;
            }
        }
        if (!doneF) {
            HuPrcVSleep();
        }
    } while (!doneF);
    menuP = pauseWork.menu;
    for (i = 0; i < 8; i++, menuP++) {
        if (menuP->enabled && menuP->panelId != 0) {
            mbPausePanelKill(menuP->panelId);
            menuP->panelId = 0;
            menuP->initialValue = 0;
        }
    }
}

static void ConfigWinUpdate(s32 index)
{
    HuVec2f pos;
    BOOL partyF;
    s32 i;
    u32 *winMesTbl;
    s32 mesNo;

    if (pauseWork.helpWinNo >= 0) {
        mbWinKill((s16)pauseWork.helpWinNo);
        pauseWork.helpWinNo = -1;
    }
    if (index >= 0) {
        partyF = GwSystem.partyF;
        if (partyF) {
            winMesTbl = pauseWinMesTbl[index];
        } else {
            winMesTbl = pauseSingleWinMesTbl[index];
        }
        if (index == 0) {
            pauseWork.helpWinNo = mbWinCreateTime(8, winMesTbl[0], -1);
            for (i = 0; i < GW_PLAYER_MAX; i++) {
                if (GwPlayer[pauseWork.padWork[i].port].comF) {
                    mesNo = 2;
                } else {
                    mesNo = 1;
                }
                mbWinInsertMesSet((s16)pauseWork.helpWinNo,
                    pauseWinMesTbl[index][mesNo], i);
            }
        } else {
            pauseWork.helpWinNo = mbWinCreateTime(8,
                winMesTbl[pauseWork.menu[index].valueMin], -1);
        }
        mbWinPosGet((s16)pauseWork.helpWinNo, &pos);
        mbWinPosSet((s16)pauseWork.helpWinNo, 144, (s16)pos.y);
        mbWinPause((s16)pauseWork.helpWinNo);
        pauseWork.talkTime = 30;
    }
}

void mbPauseDispCopyCreate(void)
{
    int i;
    int fbSize;

    pauseDispCopyFb = NULL;
    pauseDispCopyCounter = 0;
    fbSize = GXGetTexBufferSize(HU_FB_WIDTH / 2, HU_FB_HEIGHT / 2, GX_TF_RGB565, GX_FALSE, 0);
    if (!GWStorySingleCheck) {
        pauseDispCopyFb = mbMallocFlush(fbSize);
    } else {
        pauseDispCopyFb = mbMallocFlushModel(fbSize);
    }
    pauseDispCopyModelId = Hu3DHookFuncCreate(PauseDispCopyDraw);
    Hu3DModelCameraSet(pauseDispCopyModelId, 4);
    Hu3DModelLayerSet(pauseDispCopyModelId, 2);
    HuPrcVSleep();
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        playerDispF[i] = mbObjDispGet(mbPlayerObjIDGet(i));
        mbObjDispSet(mbPlayerObjIDGet(i), FALSE);
    }
}

void mbPauseDispCopyKill(void)
{
    int i;

    pauseDispCopyCounter = 0;
    HuMemDirectFree(pauseDispCopyFb);
    pauseDispCopyFb = NULL;
    Hu3DModelKill(pauseDispCopyModelId);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (playerDispF[i]) {
            mbObjDispSet(mbPlayerObjIDGet(i), TRUE);
        }
    }
}

static void PauseDispCopyDraw(HU3D_MODEL *modelP, Mtx *mtx)
{
    extern float lbl_802C4CB4;
    extern float lbl_802C4CCC;
    extern float lbl_802C4CEC;
    extern float lbl_802C4D14;
    extern float lbl_802C4D18;

    if (!pauseDispCopyCounter) {
        GXSetTexCopySrc(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT);
        GXSetTexCopyDst(HU_FB_WIDTH / 2, HU_FB_HEIGHT / 2, GX_TF_RGB565, GX_TRUE);
        GXCopyTex(pauseDispCopyFb, FALSE);
        GXPixModeSync();
        pauseDispCopyCounter++;
    } else {
        Mtx modelview;
        Mtx44 proj;
        GXTexObj texObj;

        MTXOrtho(proj, lbl_802C4CB4, lbl_802C4D14, lbl_802C4CB4,
            lbl_802C4D18, lbl_802C4CB4, lbl_802C4CEC);
        GXSetProjection(proj, GX_ORTHOGRAPHIC);
        GXSetViewport(lbl_802C4CB4, lbl_802C4CB4, lbl_802C4D18,
            lbl_802C4D14, lbl_802C4CB4, lbl_802C4CCC);
        GXSetScissor(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT);
        MTXIdentity(modelview);
        GXLoadPosMtxImm(modelview, GX_PNMTX0);
        GXSetCullMode(GX_CULL_NONE);
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_CLAMP, GX_AF_NONE);
        GXSetNumTexGens(1);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        GXSetNumTevStages(1);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetBlendMode(GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
        GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
        GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
        GXInitTexObj(&texObj, pauseDispCopyFb, HU_FB_WIDTH / 2, HU_FB_HEIGHT / 2, GX_TF_RGB565, GX_CLAMP,
            GX_CLAMP, GX_TRUE);
        GXInitTexObjLOD(&texObj, GX_LINEAR, GX_LINEAR, lbl_802C4CB4,
            lbl_802C4CB4, lbl_802C4CB4, GX_FALSE, GX_FALSE, GX_ANISO_1);
        GXLoadTexObj(&texObj, GX_TEXMAP0);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_S16, 8);
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3s16(0, 0, -50);
        GXTexCoord2s16(0, 0);
        GXPosition3s16(HU_FB_WIDTH, 0, -50);
        GXTexCoord2s16(256, 0);
        GXPosition3s16(HU_FB_WIDTH, HU_FB_HEIGHT, -50);
        GXTexCoord2s16(256, 256);
        GXPosition3s16(0, HU_FB_HEIGHT, -50);
        GXTexCoord2s16(0, 256);
        GXEnd();
    }
}

void mbPauseGuideCreate(void)
{
    pausePanelWork = mbMalloc(sizeof(PAUSE_PANEL_WORK) * 20);
    pauseGuideKillF = FALSE;
    pauseGuideProc = HuPrcChildCreate(PauseGuideMain, 8209, 8192, 0,
        mbMainProc);
    HuPrcDestructorSet2(pauseGuideProc, PauseGuideDestroy);
    HuPrcSetStat(pauseGuideProc,
        HU_PRC_STAT_PAUSE_ON | HU_PRC_STAT_UPAUSE_ON);
}

static void PauseGuideMain(void)
{
    int cameraNo;
    int i;
    HU3D_CAMERA *camera;
    PAUSE_PANEL_WORK *work;
    BOOL motionDone;
    BOOL frontF;
    float weight;
    float ease;
    float scale;
    float sprScale;
    float flipScale;
    float animWeight;
    HuVecF pos;
    HuVecF rot;
    Mtx lookAtMtx;
    Mtx rotMtx;
    Mtx invRotMtx;

    for (cameraNo = 0; cameraNo < HU3D_CAM_MAX; cameraNo++) {
        if ((1 << cameraNo) & HU3D_CAM2) {
            break;
        }
    }
    camera = &Hu3DCamera[cameraNo];
    while (!pauseGuideKillF) {
        work = pausePanelWork;
        for (i = 0; i < 20; i++, work++) {
            if (work->modelId == 0) {
                continue;
            }
            motionDone = FALSE;
            work->time++;
            if (work->delay != 0) {
                weight = 0.0f;
                if (work->time >= work->delay) {
                    work->delay = work->time = 0;
                    if (work->motion == 4) {
                        mbAudFXPlay(52);
                    } else if (work->motion == 5) {
                        mbAudFXPlay(53);
                    }
                }
            } else {
                weight = 1.0f;
                if (work->time < work->maxTime && work->maxTime > 0) {
                    weight = work->time / (float)work->maxTime;
                } else {
                    work->time = work->maxTime = 0;
                    motionDone = TRUE;
                }
                switch (work->motion) {
                    case 0:
                    case 1:
                    case 2:
                        break;

                    case 4:
                        mbObjRotGet(work->modelId, &pos);
                        pos.y = -500.0f * (1.0f - weight);
                        if (motionDone) {
                            pos.y = 0.0f;
                            if (work->batsuModelId != 0 && work->batsuF) {
                                mbObjDispSet(work->batsuModelId, TRUE);
                            }
                        }
                        mbObjRotSetV(work->modelId, &pos);
                        mbObjDispSet(work->modelId, TRUE);
                        if (work->sprId >= 0) {
                            espDispOn(work->sprId);
                        }
                        break;

                    case 5:
                        mbObjRotGet(work->modelId, &pos);
                        pos.y = 500.0f * weight;
                        if (work->batsuModelId != 0) {
                            mbObjDispSet(work->batsuModelId, FALSE);
                        }
                        if (motionDone) {
                            pos.y = 0.0f;
                            mbObjDispSet(work->modelId, FALSE);
                            if (work->sprId >= 0) {
                                espDispOff(work->sprId);
                            }
                        }
                        work->scale = mbCosDeg(90.0f * weight);
                        work->scaleTarget = work->scaleStart = work->scale;
                        mbObjRotSetV(work->modelId, &pos);
                        break;
                }
            }
            if (work->animMaxTime != 0) {
                if (work->batsuModelId != 0) {
                    mbObjDispSet(work->batsuModelId, FALSE);
                }
                mbObjRotGet(work->modelId, &pos);
                work->animTime++;
                if (work->animTime < work->animMaxTime && work->animMaxTime > 0) {
                    animWeight = work->animTime / (float)work->animMaxTime;
                    pos.y = 180.0f * animWeight;
                } else {
                    work->animTime = work->animMaxTime = 0;
                    Hu3DAnmNoSet(work->animId[0], work->bank);
                    pos.y = 0.0f;
                    if (work->batsuModelId != 0 && work->batsuF) {
                        mbObjDispSet(work->batsuModelId, TRUE);
                    }
                }
                mbObjRotSetV(work->modelId, &pos);
            }
            if (motionDone) {
                work->motion = 0;
                work->pos = work->posTarget;
                work->posStart = work->posTarget;
                work->scaleStart = work->scale = work->scaleTarget;
            } else {
                ease = mbSinDeg(90.0f * weight);
                PSVECSubtract(&work->posTarget, &work->posStart, &pos);
                PSVECScale(&pos, &pos, ease);
                PSVECAdd(&work->posStart, &pos, &work->pos);
                work->scale = work->scaleStart
                    + (ease * (work->scaleTarget - work->scaleStart));
            }
            frontF = FALSE;
            if (work->scaleTarget >= 1.2f) {
                frontF = TRUE;
            }
            mbNormPosto3D(&work->pos, HU3D_CAM2, &pos);
            mbObjPosSetV(work->modelId, &pos);
            if (work->batsuModelId != 0) {
                mbObjPosSetV(work->batsuModelId, &pos);
            }
            if (work->sprId >= 0) {
                pos.y -= 0.3f
                    * (100.0f * (work->scale * work->scaleBase));
                mbPos3DtoNorm(&pos, HU3D_CAM2, &pos);
                mbNormPosto2D(&pos, &pos);
                espPosSet(work->sprId, pos.x, pos.y);
                if (frontF) {
                    espPriSet(work->sprId, 99);
                } else {
                    espPriSet(work->sprId, 100);
                }
            }
            scale = work->scale * work->scaleBase;
            mbObjScaleSet(work->modelId, 0.75f * scale, 0.75f * scale,
                0.75f * scale);
            if (work->batsuModelId != 0) {
                mbObjScaleSet(work->batsuModelId,
                    0.8f * (0.75f * scale), 0.8f * (0.75f * scale),
                    0.75f * scale);
            }
            if (work->sprId >= 0) {
                sprScale = 0.85f * scale;
                if (work->animMaxTime != 0) {
                    weight = work->animTime / (float)work->animMaxTime;
                    flipScale = fabsf(mbCosDeg(180.0f * weight));
                    espScaleSet(work->sprId, sprScale, sprScale * flipScale);
                    if (weight >= 0.5f) {
                        espBankSet(work->sprId, work->bank);
                    }
                    flipScale = 255.0f * flipScale;
                    espColorSet(work->sprId, flipScale, flipScale, flipScale);
                } else {
                    espScaleSet(work->sprId, sprScale, sprScale);
                    espColorSet(work->sprId, 255, 255, 255);
                }
            }
            mbObjRotGet(work->modelId, &rot);
            mtxRot(rotMtx, rot.x, rot.y, rot.z);
            PSMTXInverse(rotMtx, invRotMtx);
            mbObjPosGet(work->modelId, &pos);
            C_MTXLookAt(lookAtMtx, &camera->pos, &camera->up, &pos);
            lookAtMtx[0][3] = lookAtMtx[1][3] = lookAtMtx[2][3] = 0.0f;
            PSMTXInverse(lookAtMtx, lookAtMtx);
            mbObjMtxSet(work->batsuModelId, &lookAtMtx);
            PSMTXConcat(invRotMtx, lookAtMtx, lookAtMtx);
            PSMTXConcat(lookAtMtx, rotMtx, lookAtMtx);
            mbObjMtxSet(work->modelId, &lookAtMtx);
        }
        HuPrcVSleep();
    }
    HuPrcEnd();
}

static void ConfigPadTimeSet(CONFIG_PAD_WORK *workP, float scale)
{
    HuVecF pos;
    float panelScale;
    s32 sprId;

    mbPausePanelPosGet((s16)workP->panelId, &pos);
    mbNormPosto2D(&pos, &pos);
    panelScale = mbPausePanelScaleGet((s16)workP->panelId);
    pos.x += workP->pos.x * panelScale;
    pos.y += workP->pos.y * panelScale;

    sprId = workP->sprId[0];
    espScaleSet(sprId, scale, scale);
    espPosSet(sprId, pos.x + (configPadSprOfsTbl[0] * scale),
        pos.y + (configPadSprOfsTbl[1] * scale));

    sprId = workP->sprId[1];
    espScaleSet(sprId, scale, scale);
    espPosSet(sprId, pos.x + (configPadSprOfsTbl[2] * scale),
        pos.y + (configPadSprOfsTbl[3] * scale));

    sprId = workP->sprId[2];
    espScaleSet(sprId, scale, scale);
    espPosSet(sprId, pos.x + (configPadSprOfsTbl[4] * scale),
        pos.y + (configPadSprOfsTbl[5] * scale));
}

static void PauseGuideDestroy(void)
{
    int i;
    PAUSE_PANEL_WORK *work;

    if (pausePanelWork) {
        for (i = 0; i < 20; i++) {
        }
        work = pausePanelWork;
        HuMemDirectFree(work);
        pausePanelWork = NULL;
    }
    pauseGuideProc = NULL;
}

s16 mbPausePanelCreate(int dataNum, unsigned int espDataNum)
{
    extern float lbl_802C4CCC;
    extern float lbl_802C4D1C;
    int i;
    PAUSE_PANEL_WORK *work;
    int panelId;

    for (panelId = 1; panelId < 20; panelId++) {
        if (pausePanelWork[panelId].modelId <= 0) {
            break;
        }
    }
    work = &pausePanelWork[panelId];
    memset(work, 0, sizeof(PAUSE_PANEL_WORK));
    work->scale = work->scaleStart = work->scaleTarget = work->scaleBase = lbl_802C4CCC;
    work->pos.z = work->posStart.z = work->posTarget.z = lbl_802C4D1C;
    work->modelId = mbObjCreate(mbBoardDataNumGet(DATANUM(DATA_bpause6, 37)),
        NULL, FALSE);
    mbObjCameraSet(work->modelId, 4);
    mbObjLayerSet(work->modelId, 4);
    {
        MBMODELID modelId = work->modelId;

        mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    }
    {
        MBMODELID modelId = work->modelId;

        mbObjAttrSet(modelId, HU3D_ATTR_NOPAUSE);
    }
    mbObjDispSet(work->modelId, FALSE);
    work->anim = HuSprAnimRead(HuDataSelHeapReadNum(
        mbBoardDataNumGet(dataNum), HU_MEMNUM_OVL, HEAP_MODEL));
    for (i = 0; i < 2; i++) {
        if (i == 0) {
            work->animId[0] = Hu3DAnimCreate(work->anim,
                mbObjModelIDGet(work->modelId), "S3TCys77120");
        } else {
            work->animId[i] = Hu3DAnimLink(work->animId[0],
                mbObjModelIDGet(work->modelId), "S3TCys77121");
        }
        Hu3DAnmNoSet(work->animId[i], 0);
    }
    work->batsuModelId = mbObjCreate(
        mbBoardDataNumGet(DATANUM(DATA_bpause6, 36)), NULL, TRUE);
    mbObjCameraSet(work->batsuModelId, 4);
    mbObjLayerSet(work->batsuModelId, 4);
    {
        MBMODELID modelId = work->batsuModelId;

        mbObjAttrSet(modelId, HU3D_ATTR_NOPAUSE);
    }
    mbObjDispSet(work->batsuModelId, FALSE);
    work->sprId = -1;
    if (espDataNum != 0) {
        work->sprId = espEntry(mbBoardDataNumGet(espDataNum), 100, 0);
        espAttrSet(work->sprId, HUSPR_ATTR_LINEAR);
        espDispOff(work->sprId);
    }
    return panelId;
}

void mbPausePanelKill(s16 panelId)
{
    PAUSE_PANEL_WORK *work = &pausePanelWork[panelId];
    int i;

    for (i = 0; i < 2; i++) {
        if (work->animId[i] >= 0) {
            Hu3DAnimKill(work->animId[i]);
        }
        work->animId[i] = -1;
    }
    if (work->anim) {
        HuSprAnimKill(work->anim);
    }
    work->anim = NULL;
    if (work->modelId != 0) {
        mbObjKill(work->modelId);
        work->modelId = 0;
    }
    if (work->batsuModelId != 0) {
        mbObjKill(work->batsuModelId);
        work->batsuModelId = 0;
    }
    if (work->sprId >= 0) {
        espKill(work->sprId);
        work->sprId = -1;
    }
}

void mbPauseGuideKill(void)
{
    pauseGuideKillF = TRUE;
}

void mbPausePanelPosSet(s16 panelId, float x, float y)
{
    PAUSE_PANEL_WORK *work = &pausePanelWork[panelId];

    work->pos.x = work->posStart.x = work->posTarget.x = x;
    work->pos.y = work->posStart.y = work->posTarget.y = y;
}

void mbPausePanelPosGet(s16 panelId, HuVecF *pos)
{
    extern float lbl_802C4CB4;
    PAUSE_PANEL_WORK *work = &pausePanelWork[panelId];

    pos->x = work->pos.x;
    pos->y = work->pos.y;
    pos->z = lbl_802C4CB4;
}

void mbPausePanelRotSet(s16 panelId, float rotX, float rotY, float rotZ)
{
    PAUSE_PANEL_WORK *work = &pausePanelWork[panelId];

    mbObjRotSet(work->modelId, rotX, rotY, rotZ);
}

void mbPausePanelScaleSet(s16 panelId, float scale)
{
    PAUSE_PANEL_WORK *work = &pausePanelWork[panelId];

    work->scale = work->scaleStart = work->scaleTarget = scale;
}

float mbPausePanelScaleGet(s16 panelId)
{
    PAUSE_PANEL_WORK *work = &pausePanelWork[panelId];

    return work->scale;
}

void mbPausePanelBankSet(s16 panelId, int bank)
{
    PAUSE_PANEL_WORK *work = &pausePanelWork[panelId];

    work->bank = bank;
    Hu3DAnmNoSet(work->animId[0], bank);
    Hu3DAnmNoSet(work->animId[1], bank);
    if (work->sprId >= 0) {
        espBankSet(work->sprId, bank);
    }
}

void mbPausePanelBatsuSet(s16 panelId, BOOL batsuF)
{
    PAUSE_PANEL_WORK *work = &pausePanelWork[panelId];

    work->batsuF = batsuF;
}

void mbPausePanelSizeSet(s16 panelId, int time, float scale)
{
    PAUSE_PANEL_WORK *work = &pausePanelWork[panelId];

    work->motion = 2;
    work->maxTime = time;
    work->time = 0;
    work->scaleTarget = scale;
    work->scaleStart = work->scale;
}

void mbPausePanelGrowSet(s16 panelId, int time, int delay, float scale)
{
    extern float lbl_802C4CCC;
    extern float lbl_802C4D34;
    PAUSE_PANEL_WORK *work = &pausePanelWork[panelId];

    work->motion = 4;
    work->maxTime = time;
    work->delay = delay + 1;
    work->time = 0;
    work->scaleTarget = lbl_802C4CCC;
    work->scaleStart = work->scale = lbl_802C4D34;
    work->scaleBase = scale;
    mbObjDispSet(work->modelId, FALSE);
    if (work->batsuModelId != 0) {
        mbObjDispSet(work->batsuModelId, FALSE);
    }
}

void mbPausePanelShrinkSet(s16 panelId, int time, int delay)
{
    PAUSE_PANEL_WORK *work = &pausePanelWork[panelId];

    work->motion = 5;
    work->maxTime = time;
    work->delay = delay + 1;
    work->time = 0;
}

void mbConfigPadDisableSet(BOOL disableF)
{
    configPadDisable = disableF;
}

static BOOL GWStorySingleCheck(void)
{
    return !GWPartyGet();
}

static void ConfigPadWinSet(s32 index)
{
    HuVec2f pos;

    if (pauseWork.padWinNo >= 0) {
        mbWinKill((s16)pauseWork.padWinNo);
        pauseWork.padWinNo = -1;
    }
    if (index < 0) {
        return;
    }
    pauseWork.padWinNo = mbWinCreateHelp(pausePadWinMesTbl[index]);
    mbWinCenterGet((s16)pauseWork.padWinNo, &pos);
    pos.x += pausePadWinPosTbl[index][0];
    pos.y = pausePadWinPosTbl[index][1];
    mbWinPosSet((s16)pauseWork.padWinNo, (s16)pos.x, (s16)pos.y);
}

static void ConfigSettingRead(void)
{
    s32 mic;
    s32 packValue;
    s32 messSpeedIndex;
    BOOL instDispF;
    BOOL comDispF;
    BOOL vibrateF;
    BOOL partyF;
    s32 i;
    s32 instValue;
    s32 comValue;
    s32 vibrateValue;

    pauseWork.menu[0].valueMin = 0;
    instDispF = GwSystem.mgInstDispF;
    if (instDispF) {
        instValue = 0;
    } else {
        instValue = 1;
    }
    pauseWork.menu[1].valueMin = instValue;
    comDispF = GwSystem.mgComDispF;
    if (comDispF) {
        comValue = 0;
    } else {
        comValue = 1;
    }
    pauseWork.menu[2].valueMin = comValue;
    if (GwSystem.mgPack >= GW_MINIGAME_PACK_MAX) {
        GwSystem.mgPack = GW_MINIGAME_PACK_ALL;
    }
    packValue = GwSystem.mgPack;
    pauseWork.menu[3].valueMin = packValue;
    vibrateF = GwCommon.vibrateF;
    if (vibrateF) {
        vibrateValue = 0;
    } else {
        vibrateValue = 1;
    }
    pauseWork.menu[4].valueMin = vibrateValue;
    if (GwSystem.messSpeed == GW_MESS_SPEED_MAX) {
        GwSystem.messSpeed = GW_MESS_SPEED_NORMAL;
    }
    messSpeedIndex = GwSystem.messSpeed;
    pauseWork.menu[5].valueMin = mesSpeedAnmNo[messSpeedIndex];
    mic = HuMCMicGet();
    for (i = 0; i < 3; i++) {
        if (mic == pauseMicValueTbl[i]) {
            break;
        }
    }
    if (i >= 3) {
        i = 0;
    }
    pauseWork.menu[6].valueMin = i;
    if (i == 0 && HuMCProbe(1)) {
        pauseWork.menu[6].valueMin = 1;
    }
    for (i = 0; i < 8; i++) {
        pauseWork.menu[i].initialValue = TRUE;
    }
    partyF = GwSystem.partyF;
    if (!partyF) {
        pauseWork.menu[0].initialValue = FALSE;
        pauseWork.menu[2].valueMin = 1;
        pauseWork.menu[2].initialValue = FALSE;
    }
    if (configPadDisable == 0) {
        pauseWork.menu[0].initialValue = FALSE;
    }
}

static void ConfigSettingWrite(void)
{
    BOOL vibrateF;
    s32 value;
    BOOL instDispF;
    BOOL comDispF;
    s32 packValue;
    BOOL partyF;

    instDispF = !pauseWork.menu[1].valueMin;
    GwSystem.mgInstDispF = instDispF;
    partyF = GwSystem.partyF;
    if (partyF) {
        comDispF = !pauseWork.menu[2].valueMin;
        GwSystem.mgComDispF = comDispF;
    }
    packValue = pauseWork.menu[3].valueMin;
    GwSystem.mgPack = packValue;
    vibrateF = !pauseWork.menu[4].valueMin;
    GwCommon.vibrateF = vibrateF;
    if (!vibrateF) {
        HuPadRumbleAllStop();
    }
    value = mesSpeedAnmNo[pauseWork.menu[5].valueMin];
    GwSystem.messSpeed = value;
    switch (value) {
        case GW_MESS_SPEED_FAST:
            GwSystem.comKeyDelay = 16;
            break;
        case GW_MESS_SPEED_SLOW:
            GwSystem.comKeyDelay = 48;
            break;
        default:
            GwSystem.comKeyDelay = 32;
            break;
    }
    if (pauseWork.menu[6].valueMin == 0 && HuMCProbe(1)) {
        pauseWork.menu[6].valueMin = 1;
    }
    HuMCMicSet(pauseMicValueTbl[pauseWork.menu[6].valueMin]);
}

static void ConfigPadSprSet(CONFIG_PAD_WORK *workP)
{
    s32 sprId = workP->sprId[0];
    s32 bank = 0;

    if (workP->comDif < GW_PLAYER_COM_DIF_MAX) {
        bank = 1;
    }
    espBankSet(sprId, bank);
    sprId = workP->sprId[1];
    bank = workP->padNo;
    if (workP->comDif < GW_PLAYER_COM_DIF_MAX) {
        bank = 4;
    }
    espBankSet(sprId, bank);
    sprId = workP->sprId[2];
    if (workP->comDif < GW_PLAYER_COM_DIF_MAX) {
        espDispOn(sprId);
        espBankSet(sprId, workP->comDif + 5);
    } else {
        espDispOff(sprId);
    }
}

static void PauseCursorCreate(void)
{
    PAUSE_CURSOR_WORK *cursorP;
    int i;
    int dataNum;

    cursorP = &pauseWork.cursor;
    cursorP->activeF = FALSE;
    cursorP->mask = 0;
    cursorP->cursorPos = 0;
    cursorP->cursorNo = -1;
    cursorP->moveTime = -1;
    cursorP->maxMoveTime = -1;
    cursorP->pos.x = 0.0f;
    cursorP->pos.y = 0.0f;
    cursorP->pos.z = 0.0f;
    for (i = 0; i < 4; i++) {
        cursorP->alpha[i] = 0.0f;
        dataNum = mbBoardDataNumGet(DATANUM(DATA_board, 32));
        cursorP->sprId[i] = espEntry(dataNum, 90,
            (s16)(pauseCursorBankTbl[i] + 4));
        espAttrSet(cursorP->sprId[i], HUSPR_ATTR_LINEAR);
        espPosSet(cursorP->sprId[i], cursorP->pos.x, cursorP->pos.y);
        espDispOff(cursorP->sprId[i]);
        dataNum = mbBoardDataNumGet(DATANUM(DATA_board, 32));
        cursorP->hiliteSprId[i] = espEntry(dataNum, 89,
            (s16)pauseCursorBankTbl[i]);
        espAttrSet(cursorP->hiliteSprId[i], HUSPR_ATTR_LINEAR);
        espPosSet(cursorP->hiliteSprId[i], cursorP->pos.x, cursorP->pos.y);
        espDispOff(cursorP->hiliteSprId[i]);
    }
}

static void PauseCursorKill(void)
{
    PAUSE_CURSOR_WORK *cursorP;
    int i;

    cursorP = &pauseWork.cursor;
    for (i = 0; i < 4; i++) {
        if (cursorP->sprId[i] >= 0) {
            espKill(cursorP->sprId[i]);
        }
        if (cursorP->hiliteSprId[i] >= 0) {
            espKill(cursorP->hiliteSprId[i]);
        }
        cursorP->sprId[i] = cursorP->hiliteSprId[i] = -1;
    }
}

static void PauseCursorHiliteSet(s32 cursorNo, s32 cursorPos, s32 mask)
{
    PAUSE_CURSOR_WORK *cursorP;
    int i;

    cursorP = &pauseWork.cursor;
    cursorP->activeF = FALSE;
    if (cursorNo >= 0) {
        cursorP->activeF = TRUE;
    }
    if (cursorP->cursorNo < 0 && cursorNo >= 0) {
        cursorP->moveTime = -1;
        cursorP->maxMoveTime = -1;
    }
    cursorP->mask = mask;
    cursorP->cursorPos = cursorPos;
    cursorP->cursorNo = cursorNo;
    for (i = 0; i < 4; i++) {
        espDispOff(cursorP->hiliteSprId[i]);
        if (cursorP->activeF) {
            espDispOn(cursorP->sprId[i]);
            if (mask & pauseCursorMaskTbl[i]) {
                espDispOn(cursorP->hiliteSprId[i]);
            }
        } else {
            espDispOff(cursorP->sprId[i]);
        }
    }
}

static void PauseCursorPosNextSet(s32 time, float x, float y)
{
    PAUSE_CURSOR_WORK *cursorP;
    HuVecF pos2D;
    int i;

    cursorP = &pauseWork.cursor;
    if (cursorP->moveTime < 0 || time <= 0) {
        cursorP->moveTime = 0;
        cursorP->maxMoveTime = 0;
        cursorP->pos.x = x;
        cursorP->pos.y = y;
        for (i = 0; i < 4; i++) {
            cursorP->alpha[i] = 0.0f;
            if (cursorP->cursorPos & pauseCursorMaskTbl[i]) {
                cursorP->alpha[i] = 1.0f;
            }
        }
    } else {
        HuVecF *srcP = &cursorP->pos;
        HuVecF *dstP = &cursorP->posStart;

        cursorP->maxMoveTime = time;
        cursorP->moveTime = time;
        *dstP = *srcP;
        cursorP->posDelta.x = x - cursorP->posStart.x;
        cursorP->posDelta.y = y - cursorP->posStart.y;
        cursorP->posDelta.z = 0.0f;
    }
    mbNormPosto2D(&cursorP->pos, &pos2D);
    for (i = 0; i < 4; i++) {
        espPosSet(cursorP->sprId[i],
            pos2D.x + pauseCursorSprOfsTbl[cursorP->cursorNo][i][0],
            pos2D.y + pauseCursorSprOfsTbl[cursorP->cursorNo][i][1]);
        espTPLvlSet(cursorP->sprId[i], cursorP->alpha[i]);
        espPosSet(cursorP->hiliteSprId[i],
            pos2D.x + pauseCursorSprOfsTbl[cursorP->cursorNo][i][0],
            pos2D.y + pauseCursorSprOfsTbl[cursorP->cursorNo][i][1]);
        espTPLvlSet(cursorP->hiliteSprId[i], cursorP->alpha[i]);
    }
}

static int PauseCursorMove(void)
{
    extern float lbl_802C4CB4;
    extern float lbl_802C4CCC;
    extern float lbl_802C4D0C;
    extern float lbl_802C4D10;
    PAUSE_CURSOR_WORK *cursorP;
    HuVecF pos;
    float t;
    int i;

    cursorP = &pauseWork.cursor;
    if (cursorP->moveTime <= 0) {
        return 0;
    }
    cursorP->moveTime--;
    t = (float)(cursorP->maxMoveTime - cursorP->moveTime)
        / (float)cursorP->maxMoveTime;
    t = mbSinDeg(lbl_802C4D0C * t);
    PSVECScale(&cursorP->posDelta, &pos, t);
    PSVECAdd(&cursorP->posStart, &pos, &cursorP->pos);
    for (i = 0; i < 4; i++) {
        if (cursorP->cursorPos & pauseCursorMaskTbl[i]) {
            if (cursorP->alpha[i] < lbl_802C4CCC) {
                cursorP->alpha[i] += lbl_802C4D10;
            }
            if (cursorP->alpha[i] > lbl_802C4CCC) {
                cursorP->alpha[i] = lbl_802C4CCC;
            }
        } else {
            if (cursorP->alpha[i] > lbl_802C4CB4) {
                cursorP->alpha[i] -= lbl_802C4D10;
            }
            if (cursorP->alpha[i] < lbl_802C4CB4) {
                cursorP->alpha[i] = lbl_802C4CB4;
            }
        }
    }
    mbNormPosto2D(&cursorP->pos, &pos);
    for (i = 0; i < 4; i++) {
        espPosSet(cursorP->sprId[i],
            pos.x + pauseCursorSprOfsTbl[cursorP->cursorNo][i][0],
            pos.y + pauseCursorSprOfsTbl[cursorP->cursorNo][i][1]);
        espTPLvlSet(cursorP->sprId[i], cursorP->alpha[i]);
        espPosSet(cursorP->hiliteSprId[i],
            pos.x + pauseCursorSprOfsTbl[cursorP->cursorNo][i][0],
            pos.y + pauseCursorSprOfsTbl[cursorP->cursorNo][i][1]);
        espTPLvlSet(cursorP->hiliteSprId[i], cursorP->alpha[i]);
    }
    return cursorP->moveTime;
}

static BOOL PausePlayerComRead(s32 playerNo)
{
    extern s16 HuPadStatGet(s16 padNo);
    PAUSE_PAD_WORK *work;
    BOOL changeF;
    s32 i;
    s32 j;
    s32 temp;

    work = pauseWork.padWork;
    changeF = FALSE;
    if (playerNo) {
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            work[i].port = i;
            work[i].padNo = GwPlayer[i].padNo;
            work[i].playerNo = 0;
            work[i].activeF = FALSE;
        }
        for (i = 0; i < GW_PLAYER_MAX - 1; i++) {
            for (j = i + 1; j < GW_PLAYER_MAX; j++) {
                if (work[i].padNo > work[j].padNo
                    || (work[i].padNo == work[j].padNo
                        && work[i].port > work[j].port)) {
                    temp = work[i].padNo;
                    work[i].padNo = work[j].padNo;
                    work[j].padNo = temp;
                    temp = work[i].port;
                    work[i].port = work[j].port;
                    work[j].port = temp;
                }
            }
        }
        {
            BOOL partyF;

            partyF = GwSystem.partyF;
            if (partyF) {
                for (i = 0; i < GW_PLAYER_MAX; i++) {
                    work[i].playerNo = HuPadStatGet((s16)work[i].padNo);
                    if (work[i].playerNo != 0
                        && !GwPlayer[work[i].port].comF) {
                        GwPlayer[work[i].port].comF = TRUE;
                        GwPlayerConf[work[i].port].type = TRUE;
                    }
                }
            }
        }
    }
    {
        BOOL partyF;

        partyF = GwSystem.partyF;
        if (!partyF) {
            return FALSE;
        }
    }
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        temp = HuPadStatGet((s16)work[i].padNo);
        if (temp != work[i].playerNo) {
            work[i].activeF++;
        } else {
            work[i].activeF = 0;
        }
        if (work[i].activeF < 4) {
            continue;
        }
        if (temp == 0) {
            GwPlayer[work[i].port].comF = FALSE;
            GwPlayerConf[work[i].port].type = FALSE;
            changeF = TRUE;
            work[i].playerNo = temp;
        } else if (temp == -1) {
            GwPlayer[work[i].port].comF = TRUE;
            GwPlayerConf[work[i].port].type = TRUE;
            changeF = TRUE;
            work[i].playerNo = temp;
        }
    }
    return changeF;
}

static BOOL PausePadCheck(s32 padNo)
{
    PAUSE_PAD_WORK *work = pauseWork.padWork;

    return work[padNo].playerNo != -1;
}

void mbPausePanelUnlockSet(s16 panelId)
{
    PAUSE_PANEL_WORK *workP;

    workP = &pausePanelWork[panelId];
    workP->motion = 0;
    workP->posStart = workP->pos;
    workP->posTarget = workP->posStart;
    workP->scaleStart = workP->scale;
    workP->scaleTarget = workP->scale;
    workP->maxTime = 0;
    workP->time = 0;
}

void mbPausePanelSlideSet(s16 panelId, s16 time, HuVecF *pos)
{
    PAUSE_PANEL_WORK *workP;

    workP = &pausePanelWork[panelId];
    workP->motion = 1;
    workP->maxTime = time;
    workP->time = 0;
    workP->posTarget = *pos;
    workP->posTarget.z = -500.0f;
    workP->posStart = workP->pos;
}

void mbPausePanelAnmNoSet(s16 panelId, s32 animMaxTime, s32 bank)
{
    PAUSE_PANEL_WORK *workP;

    workP = &pausePanelWork[panelId];
    workP->animMaxTime = (s16)animMaxTime;
    workP->animTime = 0;
    Hu3DAnmNoSet(workP->animId[0], (u16)workP->bank);
    workP->bank = (s16)bank;
    Hu3DAnmNoSet(workP->animId[1], (u16)bank);
    workP->sprId >= 0;
}

void mbPauseGuideTalkSet(MBMODELID modelId)
{
    extern float lbl_802C4CB4;
    extern float lbl_802C4CDC;
    extern float lbl_802C4D50;
    extern float lbl_802C4D54;

    if (pauseWork.talkTime == 0 && pauseWork.prevTalkTime > 0) {
        pauseWork.talkTime = 1;
    } else if (mbObjMotionShiftIDGet(modelId) != -1) {
        return;
    }
    if (pauseWork.prevTalkTime == 0 && pauseWork.talkTime > 0) {
        mbObjMotionShiftSet(modelId, 12, lbl_802C4CB4, lbl_802C4CDC,
            HU3D_MOTATTR_LOOP);
        mbObjMotionSpeedSet(modelId, lbl_802C4D50);
    }
    if (pauseWork.talkTime != 0) {
        pauseWork.talkTime--;
        if (pauseWork.talkTime == 0) {
            mbObjMotionShiftSet(modelId, 1, lbl_802C4CB4, lbl_802C4D54,
                HU3D_MOTATTR_LOOP);
        }
    }
    pauseWork.prevTalkTime = pauseWork.talkTime;
}

void mbPauseGuideMoveSet(MBMODELID modelId, s32 time, HuVecF *posP,
    HuVecF *posNormP)
{
    HuVecF targetPos;
    HuVecF modelPos;
    float weight;
    float rotY;
    s32 workTime;
    s32 i;

    if (posP == NULL) {
        mbNormPosto3D(posNormP, HU3D_CAM2, &targetPos);
    } else {
        targetPos = *posP;
    }
    mbObjPosGet(modelId, &modelPos);
    if (abs((int)(targetPos.x - modelPos.x)) < 200) {
        for (i = 0; i <= time; i++) {
            weight = mbSinDeg(90.0f * ((float)i / (float)time));
            mbObjPosSet(modelId,
                modelPos.x + weight * (targetPos.x - modelPos.x),
                modelPos.y + weight * (targetPos.y - modelPos.y),
                modelPos.z + weight * (targetPos.z - modelPos.z));
            HuPrcVSleep();
        }
        return;
    }
    workTime = (s32)(time * 1.5);
    rotY = 90.0f;
    if (targetPos.x < modelPos.x) {
        rotY = -90.0f;
    }
    mbObjMotionShiftSet(modelId, 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbObjMotionSpeedSet(modelId, 1.2f);
    for (i = 0; i <= workTime; i++) {
        weight = (float)i / (float)workTime;
        mbObjPosSet(modelId,
            modelPos.x + weight * (targetPos.x - modelPos.x),
            modelPos.y + weight * (targetPos.y - modelPos.y),
            modelPos.z + weight * (targetPos.z - modelPos.z));
        weight = 3.0f * (float)i / (float)workTime;
        if (weight > 1.0f) {
            weight = 1.0f;
        }
        weight = mbSinDeg(90.0f * weight);
        mbObjRotSet(modelId, 0.0f, rotY * weight, 0.0f);
        HuPrcVSleep();
    }
    mbObjMotionShiftSet(modelId, 1, 0.0f, 12.0f, HU3D_MOTATTR_LOOP);
    for (i = 0; i <= 12; i++) {
        weight = (float)i / 12.0f;
        if (weight > 1.0f) {
            weight = 1.0f;
        }
        weight = mbSinDeg(90.0f * weight);
        mbObjRotSet(modelId, 0.0f, rotY * (1.0f - weight), 0.0f);
        HuPrcVSleep();
    }
    mbObjRotSet(modelId, 0.0f, 0.0f, 0.0f);
}
