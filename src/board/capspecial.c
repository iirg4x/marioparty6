#include "dolphin/pad.h"
#include "game/board/main.h"
#include "game/board/object.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/object.h"
#include "game/process.h"

typedef int (*TERESA_STEAL_HOOK)(int);
typedef void (*TERESA_STEAL_BEGIN_HOOK)(int, int);

typedef struct TeresaFadeWork_s {
    void *textureData;
    u32 textureSize;
    BOOL activeF;
    float alpha;
    BOOL copyF;
    OMOBJ *object;
    u32 screenWidth;
    u32 screenHeight;
    u32 textureWidth;
    u32 textureHeight;
} TERESA_FADE_WORK;

typedef struct MiracleSprWork_s {
    BOOL activeF;
    int sprId;
    int backSprId;
    int sprIdTbl[6];
    int focusTime;
    int focusNo;
    BOOL hideF;
    float unk30;
    float unk34;
    HuVecF pos;
} MIRACLE_SPR_WORK;

static int koopaMdlId = -1;
static int teresaStealMesId = -1;
static int diceHitTimer;
static OMOBJ *miracleSprObj;
static TERESA_STEAL_HOOK teresaStealHook;
static TERESA_STEAL_BEGIN_HOOK teresaStealBeginHook;
static int teresaStealCoinNum;
static TERESA_FADE_WORK *teresaFadeWork;

extern void mbDiceObjHit(int playerNo);

void mbev_CapTeresaKill(void)
{
}

void mbev_CapTeresaStealSet(int mesId, int coinNum, TERESA_STEAL_BEGIN_HOOK beginHook,
    TERESA_STEAL_HOOK hook)
{
    teresaStealMesId = mesId;
    teresaStealCoinNum = coinNum;
    teresaStealBeginHook = beginHook;
    teresaStealHook = hook;
}

void mbev_CapTeresaFadeKill(int objectId)
{
    Hu3DModelMatHookSet(mbObjModelIDGet(objectId), NULL);
    if (teresaFadeWork) {
        HuMemDirectFree(teresaFadeWork->textureData);
        HuMemDirectFree(teresaFadeWork);
        teresaFadeWork = NULL;
    }
}

static void ev_CapTeresaFadeOMExec(OMOBJ *obj)
{
    if (mbExitCheck() || !teresaFadeWork) {
        omDelObjEx(mbObjMan, obj);
    } else {
        teresaFadeWork->copyF = FALSE;
    }
}

void mbev_CapTeresaFadeSet(float alpha)
{
    if (teresaFadeWork) {
        if (alpha < 0.0f) {
            alpha = 0.0f;
        }
        if (alpha > 255.0f) {
            alpha = 255.0f;
        }
        teresaFadeWork->alpha = alpha;
    }
}

void mbev_CapMiracleKill(void)
{
}

static int ev_CapMiracleMesGet(int messNo)
{
    if (GwSystem.curTime == FALSE) {
        return messNo;
    }
    return messNo + 16;
}

static void ev_CapMiracleDiceHitHook(void)
{
    diceHitTimer = 0;
}

static void ev_CapMiracleSprDestroy(void)
{
    miracleSprObj = NULL;
}

static void ev_CapMiracleTradeFocusSet(void)
{
    int i;
    OMOBJ *obj = miracleSprObj;
    MIRACLE_SPR_WORK *work;

    if (miracleSprObj != NULL) {
        work = obj->data;
        for (i = 0; i < 6; i++, work++) {
            if (work->activeF) {
                work->focusTime = 32;
                work->focusNo = 0;
            }
        }
    }
}

static void ev_CapMiracleTradeHideSet(void)
{
    int i;
    OMOBJ *obj = miracleSprObj;
    MIRACLE_SPR_WORK *work;

    if (miracleSprObj != NULL) {
        work = obj->data;
        for (i = 0; i < 6; i++, work++) {
            if (work->activeF) {
                work->hideF = TRUE;
            }
        }
    }
}

void mbev_CapKettouKill(void)
{
}

static int ev_CapKettouMesGet(int messNo)
{
    if (GwSystem.curTime == FALSE) {
        return messNo;
    }
    return messNo + 23;
}

void mbev_CapDonkeyKill(void)
{
}

void mbev_CapKoopaKill(void)
{
}

static int ev_CapKoopaDicePadBtnHook(void)
{
    if (--diceHitTimer <= 0) {
        return PAD_BUTTON_A;
    }
    return 0;
}

static void ev_CapKoopaDiceMotHook(void)
{
    int i;

    if (koopaMdlId != -1) {
        mbObjMotionSet(koopaMdlId, 4, 0);
        i = 0;
        do {
            if (i++ == 27) {
                mbDiceObjHit(-1);
            }
            HuPrcVSleep();
        } while (!mbObjMotionEndCheck(koopaMdlId));
        mbObjMotionSet(koopaMdlId, 1, HU3D_MOTATTR_LOOP);
    }
}
