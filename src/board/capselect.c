#include "game/board/main.h"

#include "game/board/masu.h"
#include "game/board/object.h"

#include "game/frand.h"
#include "game/memory.h"

#include "math.h"
#include "string.h"

#define M_PI 3.141592653589793

extern s8 mbPlayerCapsuleGet(int playerNo, int capsuleNo);
extern int mbPlayerCapsuleNumGet(int playerNo);

static HUPROCESS *ev_CapSelectShrinkProc[4] = { NULL, NULL, NULL, NULL };

static int ev_CapSelectObjId[4];
static int ev_CapSelectResult[4];
static int ev_CapSelectExtra[4];
static OMOBJ *ev_CapMasuOMObj[16];

static BOOL ev_CapSelectStoryF;
static BOOL ev_CapMasuDispF;
static int ev_CapSelectMdlId;
static int ev_CapSelectValue;

typedef struct CapMasuWork_s {
    int objNo;
    int masuId;
    int modelId;
    int angle;
    int unk10;
    BOOL unk14;
    float scale;
    HuVecF pos;
} CAPMASUWORK;

void mbCapMasuObjCreateAll(void);
void mbCapMasuObjCreate(int masuId);

static void CapMasuOMExec(OMOBJ *obj);

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
    work->unk10 = -1;
    work->unk14 = FALSE;
    work->scale = 1.0f;
    mbMasuPosGet(work->masuId, &work->pos);
    mbObjPosSetV(work->modelId, &work->pos);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (work->masuId == GwPlayer[i].masuId) {
            break;
        }
    }
    if (i < GW_PLAYER_MAX || GwSystem.turnNo >= GwSystem.turnMax) {
        work->unk14 = TRUE;
        work->scale = 0.0f;
        mbObjDispSet(work->modelId, FALSE);
    }
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
    if (work->unk14) {
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
                work->unk14 = FALSE;
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
            work->unk14 = TRUE;
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
