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

static void ConfigKill(void);
static void ConfigMain(void);
static void ConfigSettingRead(void);
static void ConfigSettingWrite(void);
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

        MTXOrtho(proj, 0, HU_FB_HEIGHT, 0, HU_FB_WIDTH, 0, 100);
        GXSetProjection(proj, GX_ORTHOGRAPHIC);
        GXSetViewport(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, 0, 1);
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
        GXInitTexObjLOD(&texObj, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
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
    work->scale = work->scaleStart = work->scaleTarget = work->scaleBase = 1.0f;
    work->pos.z = work->posStart.z = work->posTarget.z = -500.0f;
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
    PAUSE_PANEL_WORK *work = &pausePanelWork[panelId];

    pos->x = work->pos.x;
    pos->y = work->pos.y;
    pos->z = 0.0f;
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

BOOL mbPausePanelFreezeGet(s16 panelId)
{
    PAUSE_PANEL_WORK *work = &pausePanelWork[panelId];
    BOOL freezeF = FALSE;

    if (work->motion == 0 && work->animMaxTime == 0) {
        freezeF = TRUE;
    }
    return freezeF;
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
    PAUSE_PANEL_WORK *work = &pausePanelWork[panelId];

    work->motion = 4;
    work->maxTime = time;
    work->delay = delay + 1;
    work->time = 0;
    work->scaleTarget = 1.0f;
    work->scaleStart = work->scale = 0.00001f;
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
