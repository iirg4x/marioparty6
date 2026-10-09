/* Board coin models, particles, and the floating coin-count display. */
#include "dolphin/math.h"

#include "game/board/audio.h"
#include "game/board/camera.h"
#include "game/board/coin.h"
#include "game/board/effect.h"
#include "game/board/main.h"
#include "game/board/object.h"
#include "game/board/player.h"

#include "game/data.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/object.h"
#include "game/process.h"
#include "game/sprite.h"

#include "msm_se.h"
#include "string.h"

#define COIN_OBJ_BANK_MAX 64
#define COIN_OBJ_BANK_SIZE 64
#define COIN_EFF_MAX 64
#define COIN_MODEL_MAX 14

#define COIN_OBJ_ID_MASK 0xFFF
#define COIN_OBJ_ID_BASE 16384
#define COIN_OBJ_SLOT_MASK (COIN_OBJ_BANK_SIZE - 1)

#define COIN_OBJ_ATTR_USED (1 << 0)
#define COIN_OBJ_ATTR_DISP (1 << 20)
#define COIN_OBJ_KIND_SHIFT 16
#define COIN_OBJ_KIND_ALTERNATE (1 << COIN_OBJ_KIND_SHIFT)
#define COIN_OBJ_KIND_MASK (15 << COIN_OBJ_KIND_SHIFT)
#define COIN_OBJ_LAYER_MASK 0xFFFF
#define COIN_OBJ_KIND_FIELD_MASK (~COIN_OBJ_LAYER_MASK)

#define COINDISP_MODE_ON 0
#define COINDISP_MODE_MAIN 1
#define COINDISP_MODE_NONE 2
#define COINDISP_MODE_OFF 3
#define COINDISP_MODEL_MAX 5

enum {
    COIN_DATA_MODEL_RED = DATANUM(DATA_board, 3),
    COIN_DATA_MODEL = DATANUM(DATA_board, 4),
    COIN_DATA_NUMBER_ZERO = DATANUM(DATA_board, 11),
    COIN_DATA_NUMBER_ONE = DATANUM(DATA_board, 12),
    COIN_DATA_NUMBER_TWO = DATANUM(DATA_board, 13),
    COIN_DATA_NUMBER_THREE = DATANUM(DATA_board, 14),
    COIN_DATA_NUMBER_FOUR = DATANUM(DATA_board, 15),
    COIN_DATA_NUMBER_FIVE = DATANUM(DATA_board, 16),
    COIN_DATA_NUMBER_SIX = DATANUM(DATA_board, 17),
    COIN_DATA_NUMBER_SEVEN = DATANUM(DATA_board, 18),
    COIN_DATA_NUMBER_EIGHT = DATANUM(DATA_board, 19),
    COIN_DATA_NUMBER_NINE = DATANUM(DATA_board, 20),
    COIN_DATA_PLUS = DATANUM(DATA_board, 21),
    COIN_DATA_PLUS_MINUS = DATANUM(DATA_board, 22),
    COIN_DATA_EFFECT = DATANUM(DATA_board, 101),
};

typedef struct MbCoinObjBank_s {
    int count; /* Number of allocated coin objects in this bank. */
    u32 attr[COIN_OBJ_BANK_SIZE]; /* Allocation, visibility, camera, and model-kind bits per
                                   * slot. */
    s8 motNo[COIN_OBJ_BANK_SIZE]; /* Motion sample selected for each numbered model. */
    MBCOINOBJ obj[COIN_OBJ_BANK_SIZE]; /* Transform and alpha state for each slot. */
} MBCOINOBJBANK;

typedef struct MbCoinObjData_s {
    int hookModelId[3]; /* Draw hooks for the three board cameras. */
    s16 modelId[14]; /* Coin, red coin, digits, and sign model handles. */
    HSF_OBJECT *modelObj[14]; /* Model object roots drawn for each model handle. */
    MBCOINOBJBANK *bank[COIN_OBJ_BANK_MAX]; /* Allocated object banks, each with 64 slots. */
} MBCOINOBJDATA;

typedef struct CoinDispWork_s {
    unsigned killF : 1; /* Set when the display object should be removed. */
    unsigned sign : 1; /* Nonzero for a loss display, which uses a minus sign. */
    unsigned mode : 3; /* Current appear, spread, or exit animation mode. */
    u8 modelNum; /* Number of coin/sign/digit models in this display. */
    s16 no; /* Slot in coinDispOMObj for this display. */
    u16 delay; /* Frames to wait before advancing the animation. */
    u16 time; /* Appearance angle in degrees, then elapsed frames in the spread and exit modes. */
    u16 maxTime; /* Frame duration used by the spread and exit modes. */
} COINDISPWORK;

typedef struct coinDispModel_s {
    HuVecF pos; /* Model offset from the display object's position. */
    HuVecF scale; /* Per-axis model scale before perspective adjustment. */
    float rotY; /* Model yaw in degrees. */
} COINDISPMODEL;

typedef struct CoinEffData_s {
    HuVecF pos; /* World position of the coin sparkle effect. */
    s16 modelId; /* Particle model handle, or zero when no effect is allocated. */
    s16 count; /* Number of particles still alive in this effect. */
} COINEFFDATA;

extern void mbMtxRotYDeg(Mtx mtx, float angle);
extern void mbMtxRotZDeg(Mtx mtx, float angle);
extern void mbMtxScaleRotXDeg(Mtx mtx, HuVecF *scale, float angle);
extern float mbSinDeg(float angle);
extern float mbCosDeg(float angle);
extern void mbPos3Dto2D(HuVecF *src, HuVecF *dst);
extern void mbPos2Dto3D(HuVecF *src, HuVecF *dst);

static void CoinInit(void);
static void CoinClose(void);
static void CoinDraw(HU3D_MODEL *modelP, Mtx *mtxP);
static void CoinMain(void);
static void CoinEffCreate(int no, HuVecF *pos);
static void CoinEffHook(HU3D_MODEL *modelP, MBPARTICLE *particleP, Mtx mtx);
static void CoinDispCreate(OMOBJ *obj, int coinNum);
static void CoinDispOMExec(OMOBJ *obj);
static void CoinDispObjUpdate(OMOBJ *obj);
static void CoinDispOn(OMOBJ *obj);
static void CoinDispMain(OMOBJ *obj);
static void CoinDispOff(OMOBJ *obj);
static void CoinDispObjKill(OMOBJ *obj);
static void CoinAddAllProc(int *addNum, BOOL fastF, int *result);

static OMOBJ *coinDispOMObj[GW_PLAYER_MAX + 1] = {};

static int numberFileTbl[] = {
    COIN_DATA_NUMBER_ZERO, COIN_DATA_NUMBER_ONE, COIN_DATA_NUMBER_TWO, COIN_DATA_NUMBER_THREE,
    COIN_DATA_NUMBER_FOUR, COIN_DATA_NUMBER_FIVE, COIN_DATA_NUMBER_SIX, COIN_DATA_NUMBER_SEVEN,
    COIN_DATA_NUMBER_EIGHT, COIN_DATA_NUMBER_NINE, COIN_DATA_PLUS, COIN_DATA_PLUS_MINUS,
};

static int coinObjFileTbl[] = {
    COIN_DATA_MODEL_RED, COIN_DATA_MODEL, COIN_DATA_NUMBER_ZERO, COIN_DATA_NUMBER_ONE,
    COIN_DATA_NUMBER_TWO, COIN_DATA_NUMBER_THREE, COIN_DATA_NUMBER_FOUR, COIN_DATA_NUMBER_FIVE,
    COIN_DATA_NUMBER_SIX, COIN_DATA_NUMBER_SEVEN, COIN_DATA_NUMBER_EIGHT, COIN_DATA_NUMBER_NINE,
    COIN_DATA_PLUS, COIN_DATA_PLUS_MINUS,
};

static char *coinObjNameTbl[] = {
    "Rcoin-coin", "coin", "num0-no_0", "num1-no_1",
    "num2-no_2", "num3-no_3", "num4-no_4", "num5-no_5",
    "num6-no_6", "num7-no_7", "num8-no_8", "num9-no_9",
    "plus", "plus-minus",
};

static GXColor coinEffColorTbl[] = {
    { 220, 220, 64, 120 },
    { 220, 220, 220, 120 },
    { 64, 160, 220, 140 },
    { 220, 140, 140, 140 },
};

static u8 coinEffColorNoTbl[] = {
    0, 1, 0, 1, 0, 1, 0, 1,
    0, 0, 2, 3, 2, 3, 2, 3,
};

static u8 coinEffBankTbl[] = {
    2, 2, 2, 2, 2, 3, 3, 3,
    2, 2, 2, 2, 2, 3, 3, 3,
};

static u8 coinEffBankTbl2[] = {
    2, 2, 2, 2, 2, 3, 3, 3,
    2, 3, 2, 3, 2, 3, 0, 1,
};

static COINEFFDATA coinEffData[COIN_EFF_MAX];
static MBCOINOBJDATA coinObjData;
static HUPROCESS *coinMdlProc;
static int coin2MdlId;
static int coin1MdlId;

/* Copy a three-component position without changing its floating-point values. */
static inline void CoinVecCopy(register const HuVecF *src, register HuVecF *dst)
{
#ifdef __MWERKS__
    register __vec2x32float__ xy;
    register float z;
    asm {
        psq_l xy, 0(src), 0, 0
        lfs z, 8(src)
        psq_st xy, 0(dst), 0, 0
        stfs z, 8(dst)
    }
#else
    HuVecF value = *src;
    *dst = value;
#endif
}

#ifdef __MWERKS__
/* Write a world position into the translation column of a model matrix. */
#define CoinMtxTranslationSet(matrix, position) do { \
    register MtxPtr coinMatrixPtr = (matrix); \
    register const HuVecF *coinPositionPtr = (position); \
    asm { \
        lfs fp4, 0(coinPositionPtr); \
        lfs fp5, 4(coinPositionPtr); \
        lfs fp6, 8(coinPositionPtr); \
        stfs fp4, 12(coinMatrixPtr); \
        stfs fp5, 28(coinMatrixPtr); \
        stfs fp6, 44(coinMatrixPtr); \
    } \
} while (0)
#else
/* Write a world position into the translation column of a model matrix. */
static inline void CoinMtxTranslationSet(Mtx mtx, const HuVecF *pos)
{
    float x = pos->x;
    float y = pos->y;
    float z = pos->z;
    mtx[0][3] = x;
    mtx[1][3] = y;
    mtx[2][3] = z;
}
#endif

/* Board startup loads the coin models, installs draw hooks, and starts resource maintenance. */
void mbCoinInit(void)
{
    s16 redCoinModelId;
    s16 coinModelId;

    /* Board startup calls this before coin models, draw hooks, or effects are used. */
    memset(coinDispOMObj, 0, sizeof(coinDispOMObj));
    memset(coinEffData, 0, sizeof(coinEffData));
    redCoinModelId = mbObjCreate(mbBoardDataNumGet(COIN_DATA_MODEL_RED), NULL, TRUE);
    coin1MdlId = redCoinModelId;
    coinModelId = mbObjCreate(mbBoardDataNumGet(COIN_DATA_MODEL), NULL, TRUE);
    coin2MdlId = coinModelId;
    mbObjDispSet(coin1MdlId, FALSE);
    mbObjDispSet(coin2MdlId, FALSE);
    CoinInit();
    coinMdlProc = HuPrcChildCreate(CoinMain, 8206, 8192, 0, mbMainProc);
}

/* Board shutdown calls this after board objects stop using coin models and effects. */
void mbCoinClose(void)
{
    int i;

    /* Board shutdown calls this to release coin models, effects, and the update process. */
    CoinClose();
    for (i = 0; i < COIN_EFF_MAX; i++) {
        if (coinEffData[i].modelId > 0) {
            mbParticleKill(coinEffData[i].modelId);
            coinEffData[i].modelId = 0;
        }
    }
    if (coin1MdlId > 0) {
        mbObjKill(coin1MdlId);
        coin1MdlId = 0;
    }
    if (coin2MdlId > 0) {
        mbObjKill(coin2MdlId);
        coin2MdlId = 0;
    }
    HuPrcKill(coinMdlProc);
}

/* Turn a temporary board object into a coin sparkle at the object's current position. */
void mbCoinEffObjCreate(int modelId)
{
    HuVecF pos;

    /* Capsule/event callers transfer an object position into a sparkle, then discard the object. */
    mbObjPosGet(modelId, &pos);
    mbCoinEffCreate(&pos);
    mbObjKill(modelId);
}

/* Load the shared coin, sign, and digit models and register camera draw hooks. */
static void CoinInit(void)
{
    int modelIndex;
    int modelId;

    /* Load the shared coin and numeral models, then install hooks for each board camera. */
    for (modelIndex = 0; modelIndex < COIN_MODEL_MAX; modelIndex++) {
        modelId = mbObjCreate(mbBoardDataNumGet(coinObjFileTbl[modelIndex]), NULL, FALSE);
        coinObjData.modelId[modelIndex] = modelId;
        coinObjData.modelObj[modelIndex] =
            Hu3DModelObjPtrGet(mbObjModelIDGet(modelId), coinObjNameTbl[modelIndex]);
        mbObjDispSet(modelId, FALSE);
        if (modelIndex >= 2) {
            mbObjMotionSet(modelId, 0, HU3D_MOTATTR_NONE);
            mbObjMotionTimeSet(modelId, 0.5f);
            mbObjMotionSpeedSet(modelId, 0.0f);
        }
    }
    for (modelIndex = 0; modelIndex < COIN_OBJ_BANK_MAX; modelIndex++) {
        coinObjData.bank[modelIndex] = NULL;
    }
    coinObjData.hookModelId[0] = Hu3DHookFuncCreate(CoinDraw);
    Hu3DModelCameraSet(coinObjData.hookModelId[0], HU3D_CAM0);
    Hu3DModelLayerSet(coinObjData.hookModelId[0], 5);
    coinObjData.hookModelId[1] = Hu3DHookFuncCreate(CoinDraw);
    Hu3DModelCameraSet(coinObjData.hookModelId[1], HU3D_CAM1);
    Hu3DModelLayerSet(coinObjData.hookModelId[1], 5);
    coinObjData.hookModelId[2] = Hu3DHookFuncCreate(CoinDraw);
    Hu3DModelCameraSet(coinObjData.hookModelId[2], HU3D_CAM2);
    Hu3DModelLayerSet(coinObjData.hookModelId[2], 0);
}

/* Remove the camera hooks, release object banks, and unload the shared models. */
static void CoinClose(void)
{
    MBCOINOBJBANK *bankP;
    int bankIndex;

    /* Remove camera hooks, free allocated banks, and release the models loaded by CoinInit. */
    Hu3DModelKill(coinObjData.hookModelId[0]);
    coinObjData.hookModelId[0] = -1;
    Hu3DModelKill(coinObjData.hookModelId[1]);
    coinObjData.hookModelId[1] = -1;
    Hu3DModelKill(coinObjData.hookModelId[2]);
    coinObjData.hookModelId[2] = -1;
    for (bankIndex = 0; bankIndex < COIN_OBJ_BANK_MAX; bankIndex++) {
        if (coinObjData.bank[bankIndex] != NULL) {
            bankP = coinObjData.bank[bankIndex];
            HuMemDirectFree(bankP);
            coinObjData.bank[bankIndex] = NULL;
        }
    }
    for (bankIndex = 0; bankIndex < COIN_MODEL_MAX; bankIndex++) {
        mbObjKill(coinObjData.modelId[bankIndex]);
        coinObjData.modelId[bankIndex] = 0;
    }
}

/* Draw each visible coin object for the camera whose hook invoked this callback. */
static void CoinDraw(HU3D_MODEL *modelP, Mtx *mtxP)
{
    Mtx mtx;
    MBCOINOBJ *objP;
    int alpha[COIN_MODEL_MAX];
    int motNo[COIN_MODEL_MAX];
    int modelId[COIN_MODEL_MAX];
    HU3D_MODEL *modelData[COIN_MODEL_MAX];
    int bankIndex;
    MBCOINOBJBANK **bankPP;
    int *attrP;
    int modelIndex;
    int objectIndex;
    int alphaVal;
    int motion;
    int cameraBit = (u16)modelP->cameraBit;
    float motTimeTbl[] = { 0.5f, 1.5f, 2.5f };

    /* The camera hook draws visible bank entries using each object's transform and motion. */
    Hu3DModelObjDrawInit();
    for (bankIndex = 0; bankIndex < COIN_MODEL_MAX; bankIndex++) {
        alpha[bankIndex] = -1;
        motNo[bankIndex] = -1;
        modelId[bankIndex] = mbObjModelIDGet(coinObjData.modelId[bankIndex]);
        modelData[bankIndex] = &Hu3DData[modelId[bankIndex]];
    }
    for (bankPP = &coinObjData.bank[0], bankIndex = 0;
         bankIndex < COIN_OBJ_BANK_MAX;
         bankIndex++, bankPP++) {
        if (*bankPP != NULL && (*bankPP)->count != 0) {
            attrP = (int *)&(*bankPP)->attr[0];
            for (objectIndex = 0; objectIndex < COIN_OBJ_BANK_SIZE; objectIndex++, attrP++) {
                if (*attrP != 0
                    && (*attrP & cameraBit) != 0
                    && (*attrP & COIN_OBJ_ATTR_DISP) != 0) {
                    objP = &(*bankPP)->obj[objectIndex];
                    if (!(objP->alpha <= 0.0f)
                        && objP->scale.x != 0.0f
                        && objP->scale.y != 0.0f
                        && objP->scale.z != 0.0f) {
                        if (objP->rot.x != 0.0f) {
                            mbMtxScaleRotXDeg(mtx, &objP->scale, objP->rot.x);
                        } else {
                            PSMTXScale(mtx, objP->scale.x, objP->scale.y, objP->scale.z);
                        }
                        if (objP->rot.y != 0.0f) {
                            mbMtxRotYDeg(mtx, objP->rot.y);
                        }
                        if (objP->rot.z != 0.0f) {
                            mbMtxRotZDeg(mtx, objP->rot.z);
                        }
                        CoinMtxTranslationSet(mtx, &objP->pos);
                        PSMTXConcat(*mtxP, mtx, mtx);
                        modelIndex = (*attrP & COIN_OBJ_KIND_MASK) >> COIN_OBJ_KIND_SHIFT;
                        alphaVal = 255.0f * objP->alpha;
                        if (alphaVal != alpha[modelIndex]) {
                            alpha[modelIndex] = alphaVal;
                            mbObjAlphaSet(coinObjData.modelId[modelIndex], alphaVal);
                        }
                        if (modelIndex >= 2) {
                            motion = (*bankPP)->motNo[objectIndex];
                            if (motion != motNo[modelIndex]) {
                                motNo[modelIndex] = motion;
                                Hu3DMotionExec(modelId[modelIndex], modelData[modelIndex]->motId,
                                    motTimeTbl[motion], FALSE);
                            }
                        }
                        Hu3DModelObjPtrDraw(modelId[modelIndex], coinObjData.modelObj[modelIndex],
                                            mtx);
                    }
                }
            }
        }
    }
}

/* Coin constructors allocate 64 object slots with the board overlay's memory tag. */
static inline void *CoinBankAlloc(void)
{
    return HuMemDirectMallocNum(HEAP_HEAP, sizeof(MBCOINOBJBANK), HU_MEMNUM_OVL);
}

/* Initialize an allocated bank with no occupied slots. */
static inline MBCOINOBJBANK *CoinBankCreate(void)
{
    MBCOINOBJBANK *bankP;

    /* Start a new bank with every slot unused and at its default motion. */
    bankP = CoinBankAlloc();
    bankP->count = 0;
    memset(bankP->attr, 0, sizeof(bankP->attr));
    memset(bankP->motNo, 0, sizeof(bankP->motNo));
    return bankP;
}

/* Coin-display creation reserves a red-coin model slot and returns its encoded object ID. */
static inline s16 CoinCreate(void)
{
    MBCOINOBJBANK *bankP;
    int *attrP;
    int bankNo;
    MBCOINOBJ *objP;
    int objNo;

    for (bankNo = 0; bankNo < COIN_OBJ_BANK_MAX; bankNo++) {
        if (coinObjData.bank[bankNo] != NULL) {
            if (coinObjData.bank[bankNo]->count < COIN_OBJ_BANK_SIZE) {
                break;
            }
        } else {
            coinObjData.bank[bankNo] = CoinBankCreate();
            break;
        }
    }
    bankP = coinObjData.bank[bankNo];
    for (attrP = (int *)&bankP->attr[0], objNo = 0;
         objNo < COIN_OBJ_BANK_SIZE; objNo++, attrP++) {
        if (*attrP == 0) {
            break;
        }
    }
    bankP->count++;
    *attrP = COIN_OBJ_ATTR_DISP | COIN_OBJ_ATTR_USED;
    bankP->motNo[objNo] = 0;
    objP = &bankP->obj[objNo];
    memset(objP, 0, sizeof(MBCOINOBJ));
    objP->scale.x = objP->scale.y = objP->scale.z = 1.0f;
    objP->alpha = 1.0f;
    objNo |= (bankNo << 6) | COIN_OBJ_ID_BASE;
    return objNo;
}

/* Create the red-coin model used at the start of a floating coin-change display. */
s16 mbCoinCreate(void)
{
    return CoinCreate();
}

/* Board events reserve a slot for the ordinary coin model. */
s16 mbCoinCreate2(void)
{
    int *attrP;
    MBCOINOBJBANK *bankP;
    int bankNo;
    int objNo;
    MBCOINOBJ *objP;

    for (bankNo = 0; bankNo < COIN_OBJ_BANK_MAX; bankNo++) {
        if (coinObjData.bank[bankNo] != NULL) {
            if (coinObjData.bank[bankNo]->count < COIN_OBJ_BANK_SIZE) {
                break;
            }
        } else {
            coinObjData.bank[bankNo] = CoinBankCreate();
            break;
        }
    }
    bankP = coinObjData.bank[bankNo];
    for (attrP = (int *)&bankP->attr[0], objNo = 0;
         objNo < COIN_OBJ_BANK_SIZE; objNo++, attrP++) {
        if (*attrP == 0) {
            break;
        }
    }
    bankP->count++;
    *attrP = COIN_OBJ_KIND_ALTERNATE | COIN_OBJ_ATTR_DISP | COIN_OBJ_ATTR_USED;
    bankP->motNo[objNo] = 0;
    objP = &bankP->obj[objNo];
    memset(objP, 0, sizeof(MBCOINOBJ));
    objP->scale.x = objP->scale.y = objP->scale.z = 1.0f;
    objP->alpha = 1.0f;
    objNo |= (bankNo << 6) | COIN_OBJ_ID_BASE;
    return objNo;
}

/* Floating displays reserve a digit model selected by its value from zero through nine. */
static inline s16 CoinModelCreate(int modelNo)
{
    MBCOINOBJBANK *bankP;
    int *attrP;
    int bankNo;
    MBCOINOBJ *objP;
    int objNo;

    for (bankNo = 0; bankNo < COIN_OBJ_BANK_MAX; bankNo++) {
        if (coinObjData.bank[bankNo] != NULL) {
            if (coinObjData.bank[bankNo]->count < COIN_OBJ_BANK_SIZE) {
                break;
            }
        } else {
            coinObjData.bank[bankNo] = CoinBankCreate();
            break;
        }
    }
    bankP = coinObjData.bank[bankNo];
    for (attrP = (int *)&bankP->attr[0], objNo = 0;
         objNo < COIN_OBJ_BANK_SIZE; objNo++, attrP++) {
        if (*attrP == 0) {
            break;
        }
    }
    bankP->count++;
    *attrP = ((modelNo + 2) << 16) | COIN_OBJ_ATTR_USED | COIN_OBJ_ATTR_DISP;
    bankP->motNo[objNo] = 0;
    objP = &bankP->obj[objNo];
    memset(objP, 0, sizeof(MBCOINOBJ));
    objP->scale.x = objP->scale.y = objP->scale.z = 1.0f;
    objP->alpha = 1.0f;
    objNo |= (bankNo << 6) | COIN_OBJ_ID_BASE;
    return objNo;
}

/* Create a digit model object and select its fixed animation sample. */
s16 mbCoinObjCreate(int modelNo, int motNo)
{
    int objId;

    objId = CoinModelCreate(modelNo);
    mbCoinObjMotSet(objId, motNo);
    return objId;
}

/* Floating displays reserve the plus or plus/minus model selected by modelNo. */
static inline s16 CoinModelCreate2(int modelNo)
{
    MBCOINOBJBANK *bankP;
    int *attrP;
    int bankNo;
    MBCOINOBJ *objP;
    int objNo;

    for (bankNo = 0; bankNo < COIN_OBJ_BANK_MAX; bankNo++) {
        if (coinObjData.bank[bankNo] != NULL) {
            if (coinObjData.bank[bankNo]->count < COIN_OBJ_BANK_SIZE) {
                break;
            }
        } else {
            coinObjData.bank[bankNo] = CoinBankCreate();
            break;
        }
    }
    bankP = coinObjData.bank[bankNo];
    for (attrP = (int *)&bankP->attr[0], objNo = 0;
         objNo < COIN_OBJ_BANK_SIZE; objNo++, attrP++) {
        if (*attrP == 0) {
            break;
        }
    }
    bankP->count++;
    *attrP = ((modelNo + 12) << 16) | COIN_OBJ_ATTR_USED | COIN_OBJ_ATTR_DISP;
    bankP->motNo[objNo] = 0;
    objP = &bankP->obj[objNo];
    memset(objP, 0, sizeof(MBCOINOBJ));
    objP->scale.x = objP->scale.y = objP->scale.z = 1.0f;
    objP->alpha = 1.0f;
    objNo |= (bankNo << 6) | COIN_OBJ_ID_BASE;
    return objNo;
}

/* Floating displays create a sign model and select its sampled motion. */
s16 mbCoinObjCreate2(int modelNo, int motNo)
{
    int objId;

    objId = CoinModelCreate2(modelNo);
    mbCoinObjMotSet(objId, motNo);
    return objId;
}

/* Resolve an encoded coin object ID to its mutable transform and display state. */
MBCOINOBJ *mbCoinObjGet(s16 objId)
{
    MBCOINOBJBANK *bankP;
    int bankNo;
    int objNo;

    objId &= COIN_OBJ_ID_MASK;
    bankNo = objId >> 6;
    objNo = objId & COIN_OBJ_SLOT_MASK;
    bankP = coinObjData.bank[bankNo];
    return &bankP->obj[objNo];
}

/* Release a coin-object slot without spawning its sparkle effect. */
void mbCoinObjNumDec(s16 objId)
{
    MBCOINOBJBANK *bankP;
    int bankNo;
    int objNo;

    objId &= COIN_OBJ_ID_MASK;
    bankNo = objId >> 6;
    objNo = objId & COIN_OBJ_SLOT_MASK;
    bankP = coinObjData.bank[bankNo];
    bankP->attr[objNo] = 0;
    bankP->count--;
}

/* Release a coin-object slot and spawn a sparkle at its last position. */
void mbCoinObjKill(s16 objId)
{
    int bankNo;
    int objNo;
    MBCOINOBJBANK *bankP;

    objId &= COIN_OBJ_ID_MASK;
    bankNo = objId >> 6;
    objNo = objId & COIN_OBJ_SLOT_MASK;
    bankP = coinObjData.bank[bankNo];
    mbCoinEffCreate(&bankP->obj[objNo].pos);
    bankP->attr[objNo] = 0;
    bankP->count--;
}

/* Set an object's world-space position. */
void mbCoinObjPosSet(s16 objId, float x, float y, float z)
{
    MBCOINOBJBANK *bankP;
    MBCOINOBJ *objP;
    int bankNo;
    int objNo;

    objId &= COIN_OBJ_ID_MASK;
    bankNo = objId >> 6;
    objNo = objId & (COIN_OBJ_BANK_SIZE - 1);
    bankP = coinObjData.bank[bankNo];
    objP = &bankP->obj[objNo];

    objP->pos.x = x;
    objP->pos.y = y;
    objP->pos.z = z;
}

/* Set an object's world-space position from a vector. */
void mbCoinObjPosSetV(s16 objId, HuVecF *pos)
{
    mbCoinObjPosSet(objId, pos->x, pos->y, pos->z);
}

/* Copy an object's world-space position to the caller's vector. */
void mbCoinObjPosGet(s16 objId, HuVecF *pos)
{
    MBCOINOBJ *objP;
    MBCOINOBJBANK *bankP;
    int bankNo;
    int objNo;

    objId &= COIN_OBJ_ID_MASK;
    bankNo = objId >> 6;
    objNo = objId & COIN_OBJ_SLOT_MASK;
    bankP = coinObjData.bank[bankNo];
    objP = &bankP->obj[objNo];
    CoinVecCopy(&objP->pos, pos);
}

/* Set an object's rotation in degrees around each axis. */
void mbCoinObjRotSet(s16 objId, float x, float y, float z)
{
    MBCOINOBJBANK *bankP;
    MBCOINOBJ *objP;
    int bankNo;
    int objNo;

    objId &= COIN_OBJ_ID_MASK;
    bankNo = objId >> 6;
    objNo = objId & (COIN_OBJ_BANK_SIZE - 1);
    bankP = coinObjData.bank[bankNo];
    objP = &bankP->obj[objNo];

    objP->rot.x = x;
    objP->rot.y = y;
    objP->rot.z = z;
}

/* Set an object's rotation from a vector of degree values. */
void mbCoinObjRotSetV(s16 objId, HuVecF *rot)
{
    mbCoinObjRotSet(objId, rot->x, rot->y, rot->z);
}

/* Copy an object's rotation in degrees to the caller's vector. */
void mbCoinObjRotGet(s16 objId, HuVecF *rot)
{
    MBCOINOBJ *objP;
    MBCOINOBJBANK *bankP;
    int bankNo;
    int objNo;

    objId &= COIN_OBJ_ID_MASK;
    bankNo = objId >> 6;
    objNo = objId & COIN_OBJ_SLOT_MASK;
    bankP = coinObjData.bank[bankNo];
    objP = &bankP->obj[objNo];
    CoinVecCopy(&objP->rot, rot);
}

/* Set an object's scale independently on each axis. */
void mbCoinObjScaleSet(s16 objId, float x, float y, float z)
{
    MBCOINOBJBANK *bankP;
    MBCOINOBJ *objP;
    int bankNo;
    int objNo;

    objId &= COIN_OBJ_ID_MASK;
    bankNo = objId >> 6;
    objNo = objId & (COIN_OBJ_BANK_SIZE - 1);
    bankP = coinObjData.bank[bankNo];
    objP = &bankP->obj[objNo];

    objP->scale.x = x;
    objP->scale.y = y;
    objP->scale.z = z;
}

/* Set an object's scale from a vector. */
void mbCoinObjScaleSetV(s16 objId, HuVecF *scale)
{
    mbCoinObjScaleSet(objId, scale->x, scale->y, scale->z);
}

/* Copy an object's scale to the caller's vector. */
void mbCoinObjScaleGet(s16 objId, HuVecF *scale)
{
    MBCOINOBJ *objP;
    MBCOINOBJBANK *bankP;
    int bankNo;
    int objNo;

    objId &= COIN_OBJ_ID_MASK;
    bankNo = objId >> 6;
    objNo = objId & COIN_OBJ_SLOT_MASK;
    bankP = coinObjData.bank[bankNo];
    objP = &bankP->obj[objNo];
    CoinVecCopy(&objP->scale, scale);
}

/* Store opacity without clamping; zero is transparent and one is fully visible. */
void mbCoinObjAlphaSet(s16 objId, float alpha)
{
    MBCOINOBJBANK *bankP;
    MBCOINOBJ *objP;
    int bankNo;
    int objNo;

    objId &= COIN_OBJ_ID_MASK;
    bankNo = objId >> 6;
    objNo = objId & (COIN_OBJ_BANK_SIZE - 1);
    bankP = coinObjData.bank[bankNo];
    objP = &bankP->obj[objNo];

    objP->alpha = alpha;
}

/* Return the object's stored opacity without clamping it. */
float mbCoinObjAlphaGet(s16 objId)
{
    MBCOINOBJBANK *bankP;
    MBCOINOBJ *objP;
    int bankNo;
    int objNo;

    objId &= COIN_OBJ_ID_MASK;
    bankNo = objId >> 6;
    objNo = objId & (COIN_OBJ_BANK_SIZE - 1);
    bankP = coinObjData.bank[bankNo];
    objP = &bankP->obj[objNo];

    return objP->alpha;
}

/* Enable or disable drawing this object in its selected camera. */
void mbCoinObjDispSet(s16 objId, BOOL dispF)
{
    MBCOINOBJBANK *bankP;
    int bankNo;
    int objNo;

    objId &= COIN_OBJ_ID_MASK;
    bankNo = objId >> 6;
    objNo = objId & COIN_OBJ_SLOT_MASK;
    bankP = coinObjData.bank[bankNo];
    bankP->attr[objNo] &= ~COIN_OBJ_ATTR_DISP;
    if (dispF) {
        bankP->attr[objNo] |= COIN_OBJ_ATTR_DISP;
    }
}

/* Return whether the object's display flag is enabled. */
BOOL mbCoinObjDispGet(s16 objId)
{
    MBCOINOBJBANK *bankP;
    int bankNo;
    int objNo;

    objId &= COIN_OBJ_ID_MASK;
    bankNo = objId >> 6;
    objNo = objId & COIN_OBJ_SLOT_MASK;
    bankP = coinObjData.bank[bankNo];
    return (bankP->attr[objNo] & COIN_OBJ_ATTR_DISP) == COIN_OBJ_ATTR_DISP;
}

/* Board effects and floating displays select the camera mask used to draw this object. */
void mbCoinObjLayerSet(s16 objId, u16 layer)
{
    MBCOINOBJBANK *bankP;
    int bankNo;
    int objNo;

    objId &= COIN_OBJ_ID_MASK;
    bankNo = objId >> 6;
    objNo = objId & COIN_OBJ_SLOT_MASK;
    bankP = coinObjData.bank[bankNo];
    bankP->attr[objNo] &= COIN_OBJ_KIND_FIELD_MASK;
    bankP->attr[objNo] |= layer;
}

/* Select the fixed animation sample at frame 0.5, 1.5, or 2.5 for this digit or sign. */
void mbCoinObjMotSet(s16 objId, int motNo)
{
    MBCOINOBJBANK *bankP;
    int bankNo;
    int objNo;

    objId &= COIN_OBJ_ID_MASK;
    bankNo = objId >> 6;
    objNo = objId & COIN_OBJ_SLOT_MASK;
    bankP = coinObjData.bank[bankNo];
    bankP->motNo[objNo] = motNo;
}

/* Coin process created by mbCoinInit: seed five effects and two banks, then reclaim idle resources
 * beyond those retained slots each frame. */
static void CoinMain(void)
{
    HuVecF pos = { 0.0f, 0.0f, 0.0f };
    MBCOINOBJBANK *bankP;
    COINEFFDATA *effP;
    MBCOINOBJBANK **bankPP;
    int effectIndex;
    int bankIndex;
    int activeEffectCount;
    int resourceIndex;

    for (effectIndex = 0; effectIndex < 5; effectIndex++) {
        CoinEffCreate(effectIndex, &pos);
    }
    for (bankIndex = 0, resourceIndex = 0; resourceIndex < 2; bankIndex++) {
        if (coinObjData.bank[bankIndex] == NULL) {
            coinObjData.bank[bankIndex] = CoinBankCreate();
            resourceIndex++;
        }
    }
    while (TRUE) {
        /* This model count is collected and discarded; each effect's particle count controls
         * cleanup. */
        activeEffectCount = 0;
        for (effP = &coinEffData[effectIndex], resourceIndex = effectIndex;
             resourceIndex < COIN_EFF_MAX;
             resourceIndex++, effP++) {
            if (effP->modelId != 0) {
                activeEffectCount++;
                if (effP->count == 0) {
                    mbParticleKill(effP->modelId);
                    effP->modelId = 0;
                }
            }
        }
        activeEffectCount = 0;
        for (bankPP = &coinObjData.bank[bankIndex], resourceIndex = bankIndex;
             resourceIndex < COIN_OBJ_BANK_MAX;
             resourceIndex++, bankPP++) {
            if (*bankPP != NULL && (*bankPP)->count == 0) {
                bankP = *bankPP;
                HuMemDirectFree(bankP);
                *bankPP = NULL;
            }
        }
        HuPrcVSleep();
    }
}

/* Return the absolute value used for particle distance checks. */
static inline float CoinAbsFloat(register float value)
{
#ifdef __MWERKS__
    asm {
        fabs value, value
    }
    return value;
#else
    return __fabsf(value);
#endif
}

/* Add up to 20 colored particles at a world position for a collected coin. */
void mbCoinEffCreate(HuVecF *pos)
{
    COINEFFDATA *effP;
    int colorNo;
    MBPARTICLE *particleP;
    MBPARTICLEDATA *particleDataP;
    HuVecF offset;
    int unused;
    float angle;
    float angleY;
    int effNo;
    int remaining;
    int i;

    mbAudFXPlay(MSM_SE_CMN_08);
    effNo = -1;
    for (effP = &coinEffData[0], i = 0; i < COIN_EFF_MAX; i++, effP++) {
        if (effP->modelId != 0) {
            if (effP->count == 0) {
                effNo = i;
                break;
            }
            if (effP->count <= 80
                && CoinAbsFloat(pos->x - effP->pos.x) < 300.0f
                && CoinAbsFloat(pos->y - effP->pos.y) < 300.0f
                && CoinAbsFloat(pos->z - effP->pos.z) < 300.0f) {
                effNo = i;
                break;
            }
        } else if (effNo < 0) {
            effNo = i;
            break;
        }
    }
    if (effNo < 0) {
        return;
    }
    CoinEffCreate(effNo, pos);
    effP = &coinEffData[effNo];
    /* This initialized value is unused and does not affect the emitted particles. */
    unused = 0;
    particleP = Hu3DData[effP->modelId].hookData;
    VECSubtract(pos, &effP->pos, &offset);
    remaining = 20;
    effP->count += remaining;
    for (particleDataP = particleP->data, i = 0;
         i < particleP->num && remaining != 0;
         i++, particleDataP++) {
        if (particleDataP->time == 0) {
            colorNo = coinEffColorNoTbl[mbRandMod(16)];
            particleDataP->color.r = coinEffColorTbl[colorNo].r + mbRandMod(20);
            particleDataP->color.g = coinEffColorTbl[colorNo].g + mbRandMod(20);
            particleDataP->color.b = coinEffColorTbl[colorNo].b + mbRandMod(20);
            particleDataP->color.a = coinEffColorTbl[colorNo].a + mbRandMod(20);
            /* The bank choice uses 32 indices although coinEffBankTbl declares only 16 entries. */
            particleDataP->animBank = coinEffBankTbl[mbRandMod(32)];
            particleDataP->pos.x = particleDataP->pos.y = particleDataP->pos.z = 0.0f;
            angle = 360.0f * frandf();
            angleY = (1.7f * frandf()) - 0.7f;
            angleY = 90.0f * (angleY * CoinAbsFloat(angleY));
            particleDataP->vel.x = mbSinDeg(angle) * mbCosDeg(angleY);
            particleDataP->vel.y = mbSinDeg(angleY);
            particleDataP->vel.z = mbCosDeg(angle) * mbCosDeg(angleY);
            VECScale(&particleDataP->vel, &particleDataP->pos,
                100.0f * (0.5f * frandf()));
            VECScale(&particleDataP->vel, &particleDataP->vel,
                (1.0f / 60.0f) * (300.0f + (100.0f * (4.0f * frandf()))));
            VECAdd(&particleDataP->pos, &offset, &particleDataP->pos);
            particleDataP->speedDecay = (1.0f / 60.0f)
                * (100.0f * (0.7f * (0.2f + (0.3f * frandf()))));
            particleDataP->colorIdx = 90.0f + angle;
            particleDataP->scale = 70.0f * (0.5f + (0.5f * frandf()));
            particleDataP->accel.x = -1.3f;
            particleDataP->rot.z = 360.0f * frandf();
            particleDataP->scaleBase = (20.0f * frandf()) - 10.0f;
            particleDataP->time = mbRandMod(10) + 20;
            particleDataP->accel.y = 0.44444448f;
            particleDataP->accel.z = 0.92f;
            remaining--;
        }
    }
}

/* Allocate a particle model or move an idle one; active models retain their existing origin. */
static void CoinEffCreate(int no, HuVecF *pos)
{
    COINEFFDATA *effP;
    MBPARTICLE *particleP;
    MBPARTICLEDATA *particleDataP;
    int i;

    effP = &coinEffData[no];
    if (effP->modelId <= 0) {
        effP->modelId =
            mbParticleCreate(HuSprAnimRead(HuDataSelHeapReadNum(mbBoardDataNumGet(COIN_DATA_EFFECT),
                                                                HU_MEMNUM_OVL, HEAP_MODEL)),
                             100);
        mbParticleHookSet(effP->modelId, CoinEffHook);
        Hu3DModelLayerSet(effP->modelId, 5);
        effP->pos.x = pos->x;
        effP->pos.y = pos->y;
        effP->pos.z = pos->z;
        Hu3DModelPosSetV(effP->modelId, pos);
        effP->count = 0;
        particleP = Hu3DData[effP->modelId].hookData;
        for (particleDataP = particleP->data, i = 0;
             i < particleP->num;
             i++, particleDataP++) {
            particleDataP->scale = 0.0f;
            particleDataP->color.a = 0;
            particleDataP->time = 0;
        }
        particleP->blendMode = MB_PARTICLE_BLEND_ADDCOL;
        /* Store the effect slot in the model's time field so CoinEffHook can update its live
         * count. */
        particleP->time = no;
    } else if (effP->count == 0) {
        effP->pos.x = pos->x;
        effP->pos.y = pos->y;
        effP->pos.z = pos->z;
        Hu3DModelPosSetV(effP->modelId, pos);
    }
}

/* Particle drawing calls this once per update to advance sparkles and count the survivors. */
static void CoinEffHook(HU3D_MODEL *modelP, MBPARTICLE *particleP, Mtx mtx)
{
    MBPARTICLEDATA *particleDataP;
    int liveParticleCount;
    int particleCapacity;
    int particleIndex;

    liveParticleCount = 0;
    /* This capacity snapshot is unused; the particle loop reads the model's slot count directly. */
    particleCapacity = particleP->num;
    for (particleDataP = particleP->data, particleIndex = 0;
         particleIndex < particleP->num;
         particleIndex++, particleDataP++) {
        if (particleDataP->time != 0) {
            liveParticleCount++;
            VECScale(&particleDataP->vel, &particleDataP->vel, particleDataP->accel.z);
            VECAdd(&particleDataP->pos, &particleDataP->vel, &particleDataP->pos);
            particleDataP->vel.y += particleDataP->accel.y;
            particleDataP->vel.x += particleDataP->speedDecay * mbSinDeg(particleDataP->colorIdx);
            particleDataP->vel.z += particleDataP->speedDecay * mbCosDeg(particleDataP->colorIdx);
            particleDataP->colorIdx += 5.0f;
            particleDataP->speedDecay += 0.02777778f;
            particleDataP->scale += particleDataP->accel.x;
            particleDataP->rot.z += particleDataP->scaleBase;
            particleDataP->time--;
            if (particleDataP->time < 10) {
                particleDataP->color.a *= 0.8f;
                if (particleDataP->time == 0) {
                    particleDataP->color.a = 0;
                    particleDataP->scale = 0.0f;
                    liveParticleCount--;
                }
            }
        }
    }
    coinEffData[particleP->time].count = liveParticleCount;
}

/* Board events create a floating signed coin amount; sign selects the sign only for zero. */
s16 mbCoinDispCreate(HuVecF *pos, int coinNum, int sign, BOOL playSe)
{
    OMOBJ *displayObj;
    COINDISPWORK *displayWork;
    s8 displaySlot;

    for (displaySlot = 1; displaySlot < GW_PLAYER_MAX + 1; displaySlot++) {
        if (!coinDispOMObj[displaySlot]) {
            break;
        }
    }
    if (displaySlot >= GW_PLAYER_MAX + 1) {
        return -1;
    }
    if (coinNum > 999) {
        coinNum = 999;
    } else if (coinNum < -999) {
        coinNum = -999;
    }
    displayObj = omAddObjEx(mbObjMan, 261, 5, 0, OM_GRP_NONE, CoinDispOMExec);
    displayObj->data = HuMemDirectMallocNum(
        HEAP_HEAP,
        COINDISP_MODEL_MAX * sizeof(COINDISPMODEL),
        HU_MEMNUM_OVL);
    omSetStatBit(displayObj, OM_STAT_MODELPAUSE);
    coinDispOMObj[displaySlot] = displayObj;
    displayWork = omObjGetWork(displayObj, COINDISPWORK);
    displayWork->killF = FALSE;
    if (coinNum != 0) {
        displayWork->sign = (coinNum < 0) ? 1 : 0;
    } else {
        displayWork->sign = (sign < 0) ? 1 : 0;
    }
    displayWork->mode = COINDISP_MODE_ON;
    displayWork->no = displaySlot;
    displayWork->delay = 0;
    displayWork->time = 0;
    displayObj->trans.x = pos->x;
    displayObj->trans.y = pos->y;
    displayObj->trans.z = pos->z;
    displayObj->rot.x = 0.0f;
    displayObj->rot.y = 0.01f;
    CoinDispCreate(displayObj, coinNum);
    CoinDispObjUpdate(displayObj);
    if (playSe) {
        if (!displayWork->sign) {
            mbAudFXPlay(MSM_SE_BRD00_123);
        } else {
            mbAudFXPlay(MSM_SE_BRD00_124);
        }
    }
    return displayWork->no;
}

/* Board-space caller variant that supplies a positive sign when the total is zero. */
s16 mbCoinDispMasuCreate(HuVecF *pos, int coinNum, BOOL playSe)
{
    return mbCoinDispCreate(pos, coinNum, 1, playSe);
}

/* Capsule callers request the gain/loss cue when a display slot is available. */
s16 mbCoinDispCapsuleCreate(HuVecF *pos, int coinNum)
{
    return mbCoinDispCreate(pos, coinNum, 1, TRUE);
}

/* Display creation builds the coin, sign, and decimal digits, suppressing leading zeroes. */
static void CoinDispCreate(OMOBJ *displayObj, int coinNum)
{
    int modelSlot;
    int digit;
    int digitPlace;
    COINDISPWORK *displayWork = omObjGetWork(displayObj, COINDISPWORK);
    COINDISPMODEL *digitModel = displayObj->data;
    BOOL showLeadingDigits = FALSE;
    float displayMotionTime;
    int modelIndex;

    displayObj->mdlId[0] = mbCoinCreate();
    /* This motion-time choice is unused; the sign and digit constructors select their samples. */
    if (displayWork->sign) {
        displayMotionTime = 2.5f;
    } else {
        displayMotionTime = 1.5f;
    }
    displayObj->mdlId[1] = mbCoinObjCreate2(displayWork->sign, displayWork->sign ? 2 : 1);
    digitPlace = 100;
    displayWork->modelNum = 0;
    modelSlot = 2;
    coinNum = abs(coinNum);
    for (modelIndex = 0; modelIndex < 3; modelIndex++) {
        digit = coinNum / digitPlace;

        if (modelIndex == 2) {
            showLeadingDigits = TRUE;
        }
        if (showLeadingDigits || digit != 0) {
            showLeadingDigits = TRUE;
            displayObj->mdlId[modelSlot] = mbCoinObjCreate(digit, displayWork->sign ? 2 : 1);
            modelSlot++;
        }
        coinNum -= digit * digitPlace;
        digitPlace /= 10;
    }
    displayWork->modelNum = modelSlot;
    for (digitModel = displayObj->data, modelIndex = 0;
         modelIndex < displayWork->modelNum;
         modelIndex++, digitModel++) {
        digitModel->pos.x = digitModel->pos.y = digitModel->pos.z = 0.0f;
        digitModel->rotY = 0.0f;
        digitModel->scale.x = digitModel->scale.y = digitModel->scale.z = 0.001f;
        mbCoinObjLayerSet(displayObj->mdlId[modelIndex], HU3D_CAM1);
    }
}

/* Per-frame object callback: process its animation mode or remove a killed display. */
static void CoinDispOMExec(OMOBJ *obj)
{
    COINDISPWORK *displayWork = omObjGetWork(obj, COINDISPWORK);

    if (displayWork->killF || mbExitCheck()) {
        CoinDispObjKill(obj);
        coinDispOMObj[displayWork->no] = NULL;
        omDelObjEx(HuPrcCurrentGet(), obj);
        return;
    }
    if (displayWork->delay == 0) {
        switch (displayWork->mode) {
            case COINDISP_MODE_ON:
                CoinDispOn(obj);
                break;

            case COINDISP_MODE_MAIN:
                CoinDispMain(obj);
                break;

            case COINDISP_MODE_OFF:
                CoinDispOff(obj);
                break;
        }
    } else {
        displayWork->delay--;
    }
    CoinDispObjUpdate(obj);
}

/* Display creation and each object update place the models at depth 800 with their screen size
 * preserved. */
static void CoinDispObjUpdate(OMOBJ *displayObj)
{
    COINDISPWORK *displayWork = omObjGetWork(displayObj, COINDISPWORK);
    COINDISPMODEL *displayModel = displayObj->data;
    HuVecF worldPos;
    HuVecF transformVec;
    Mtx cameraYawMtx;
    float cameraRotY;
    int modelIndex;

    mbCameraRotGet(&transformVec);
    cameraRotY = transformVec.y;
    PSMTXRotRad(cameraYawMtx, 'Y', 0.017453292f * cameraRotY);
    for (modelIndex = 0; modelIndex < displayWork->modelNum; modelIndex++, displayModel++) {
        float depthScale;
        float originalDepth;

        PSMTXMultVec(cameraYawMtx, &displayModel->pos, &transformVec);
        PSVECAdd(&displayObj->trans, &transformVec, &worldPos);
        mbPos3Dto2D(&worldPos, &transformVec);
        originalDepth = transformVec.z;
        /* A common camera depth keeps the models together; scaling preserves their apparent
         * size. */
        transformVec.z = 800.0f;
        mbPos2Dto3D(&transformVec, &worldPos);
        depthScale = 800.0f / originalDepth;
        mbCoinObjPosSetV(displayObj->mdlId[modelIndex], &worldPos);
        mbCoinObjRotSet(
            displayObj->mdlId[modelIndex],
            0.0f,
            displayModel->rotY + cameraRotY,
            0.0f);
        mbCoinObjScaleSet(
            displayObj->mdlId[modelIndex],
            displayModel->scale.x * depthScale,
            displayModel->scale.y * depthScale,
            displayModel->scale.z * depthScale);
    }
}

/* During the appear mode, the display callback grows and spins the coin before revealing digits. */
static void CoinDispOn(OMOBJ *displayObj)
{
    COINDISPWORK *displayWork = omObjGetWork(displayObj, COINDISPWORK);
    COINDISPMODEL *displayModel = displayObj->data;
    float appearanceWeight;
    float modelScale;
    int modelIndex;

    appearanceWeight = mbSinDeg((float)displayWork->time);
    modelScale = appearanceWeight;
    displayObj->rot.x = 405.0f * appearanceWeight;
    displayModel->scale.x = displayModel->scale.y = displayModel->scale.z = modelScale;
    displayModel->pos.x = displayModel->pos.y = displayModel->pos.z = 0.0f;
    displayModel->rotY = displayObj->rot.x;
    if (displayWork->time < 90) {
        displayWork->time += 6;
        return;
    }
    displayWork->mode = COINDISP_MODE_MAIN;
    displayWork->time = 0;
    displayWork->maxTime = 30;
    /* The sign and digits stay tiny until the coin has finished growing. */
    displayModel = ((COINDISPMODEL *)displayObj->data) + 1;
    for (modelIndex = 1; modelIndex < displayWork->modelNum; modelIndex++, displayModel++) {
        displayModel->scale.x = displayModel->scale.y = displayModel->scale.z = modelScale;
        displayModel->pos.x = displayModel->pos.y = displayModel->pos.z = 0.0f;
        displayModel->rotY = displayObj->rot.x;
    }
}

/* During the spread mode, the display callback fans out the models, then selects exit with a
 * pause. */
static void CoinDispMain(OMOBJ *displayObj)
{
    COINDISPWORK *displayWork = omObjGetWork(displayObj, COINDISPWORK);
    COINDISPMODEL *displayModel = displayObj->data;
    float spreadProgress = (float)displayWork->time / displayWork->maxTime;
    float slotOffsetX = -0.5f * ((displayWork->modelNum - 1) * 120.00001f);
    float spreadWeight = mbSinDeg(spreadProgress * 90.0f);
    float bounceAngle;
    float bounceHeightWeight;
    int modelIndex;

    displayObj->rot.x = 45.0f + (spreadWeight * 315.0f);
    bounceAngle = spreadProgress * (((displayWork->modelNum - 1) * 30.0f) + 180.0f);
    for (modelIndex = 0; modelIndex < displayWork->modelNum; modelIndex++, displayModel++) {
        bounceHeightWeight = mbSinDeg(bounceAngle);
        if (bounceHeightWeight < 0.0f) {
            bounceHeightWeight = 0.0f;
        }
        displayModel->pos.x = spreadWeight * slotOffsetX;
        displayModel->pos.y = 200.0f * bounceHeightWeight;
        displayModel->pos.z = 0.0f;
        displayModel->rotY = displayObj->rot.x;
        slotOffsetX += 120.00001f;
        bounceAngle -= 30.0f;
    }
    if (++displayWork->time > displayWork->maxTime) {
        displayWork->mode = COINDISP_MODE_OFF;
        displayWork->time = 0;
        displayWork->maxTime = 24;
        displayWork->delay = 30;
    }
}

/* During the exit mode, the display callback raises gains or lowers losses while fading them. */
static void CoinDispOff(OMOBJ *displayObj)
{
    COINDISPWORK *displayWork = omObjGetWork(displayObj, COINDISPWORK);
    COINDISPMODEL *displayModel = displayObj->data;
    float exitProgress = (float)displayWork->time / displayWork->maxTime;
    float fadeAngle;
    float travelWeight;
    float travelY;
    float leadingFadeAngle;
    int modelIndex;

    displayObj->rot.x = (270.0f * mbSinDeg(exitProgress * 90.0f)) + 90.0f;
    leadingFadeAngle = exitProgress * (((displayWork->modelNum - 1) * 30.0f) + 90.0f);
    for (modelIndex = 0; modelIndex < displayWork->modelNum; modelIndex++, displayModel++) {
        fadeAngle = leadingFadeAngle - (modelIndex * 30.0f);
        if (fadeAngle < 0.0f) {
            fadeAngle = 0.0f;
        } else if (fadeAngle > 90.0f) {
            fadeAngle = 90.0f;
        }
        travelWeight = mbSinDeg(fadeAngle);
        if (displayWork->sign) {
            travelY = -100.0f * travelWeight;
        } else {
            travelY = 100.0f * travelWeight;
        }
        displayModel->pos.y = travelY;
        displayModel->rotY = displayObj->rot.x;
        mbCoinObjAlphaSet(displayObj->mdlId[modelIndex], mbCosDeg(fadeAngle));
    }
    if (++displayWork->time > displayWork->maxTime) {
        for (modelIndex = 0; modelIndex < displayWork->modelNum; modelIndex++) {
            mbCoinObjDispSet(displayObj->mdlId[modelIndex], FALSE);
        }
        displayWork->killF = TRUE;
    }
}

/* Release every coin model owned by a floating-total object. */
static void CoinDispObjKill(OMOBJ *obj)
{
    COINDISPWORK *work = omObjGetWork(obj, COINDISPWORK);
    int i;

    for (i = 0; i < work->modelNum; i++) {
        if (obj->mdlId[i] >= 0) {
            mbCoinObjNumDec(obj->mdlId[i]);
        }
        obj->mdlId[i] = -1;
    }
}

/* Request removal of one floating-total object by its display slot. */
void mbCoinDispKill(s16 no)
{
    if (no <= 0 || no >= GW_PLAYER_MAX + 1) {
        return;
    }
    if (coinDispOMObj[no]) {
        omObjGetWork(coinDispOMObj[no], COINDISPWORK)->killF = TRUE;
    }
}

/* Return true after the display object leaves its slot; a pending kill still returns false. */
BOOL mbCoinDispKillCheck(s16 no)
{
    if (no <= 0 || no >= GW_PLAYER_MAX + 1) {
        return TRUE;
    }
    if (coinDispOMObj[no]) {
        COINDISPWORK *work = omObjGetWork(coinDispOMObj[no], COINDISPWORK);

        return FALSE;
    }
    return TRUE;
}

/* Apply one player's coin change, clamping the total to 0..999. */
static inline int CoinAdd(int playerNo, int coinNum, BOOL fastF)
{
    int num;
    int coinChg;
    int coinDiff;
    int i;
    int delay;
    int coinNew;
    s16 seId;

    if (abs(coinNum) >= 50) {
        delay = 1;
    } else if (abs(coinNum) >= 20) {
        delay = 3;
    } else {
        delay = 6;
    }
    coinNew = coinNum + mbPlayerCoinGet(playerNo);
    num = coinNum;
    if (coinNew > 999) {
        num = 999 - mbPlayerCoinGet(playerNo);
    } else if (coinNew < 0) {
        num = -mbPlayerCoinGet(playerNo);
    }
    coinDiff = num;
    if (!fastF) {
        coinChg = (num >= 0) ? 1 : -1;
        seId = (coinChg > 0) ? MSM_SE_CMN_08 : MSM_SE_CMN_15;
        for (i = 0; i < abs(num); i++) {
            mbPlayerCoinAdd(playerNo, coinChg);
            mbAudFXPlay(seId);
            HuPrcSleep(delay);
        }
    } else {
        mbPlayerCoinAdd(playerNo, coinDiff);
    }
    return coinDiff;
}

/* Board events apply a coin change and wait for its display; dispF also requests a zero display. */
int mbCoinAddProcExec(int playerNo, int coinNum, BOOL dispF, BOOL fastF)
{
    int coinDiff;
    int dispNo;

    coinDiff = CoinAdd(playerNo, coinNum, fastF);
    if (coinDiff != 0 || dispF) {
        HuVecF pos;

        mbAudFXPlay(MSM_SE_CMN_16);
        mbPlayerPosGet(playerNo, &pos);
        pos.y += 250.0f;
        dispNo = mbCoinDispCreate(&pos, coinDiff, dispF, TRUE);
        while (!mbCoinDispKillCheck(dispNo)) {
            HuPrcVSleep();
        }
    }
    return coinDiff;
}

/* Board events apply a coin change and, when requested, wait for its nonzero change display. */
int mbCoinAddDispExec(int playerNo, int coinNum, BOOL dispF, BOOL fastF)
{
    int coinDiff;
    int dispNo;

    coinDiff = CoinAdd(playerNo, coinNum, fastF);
    if (coinDiff != 0) {
        mbAudFXPlay(MSM_SE_CMN_16);
    }
    if (dispF && coinDiff != 0) {
        HuVecF pos;

        mbPlayerPosGet(playerNo, &pos);
        pos.y += 250.0f;
        dispNo = mbCoinDispCapsuleCreate(&pos, coinDiff);
        while (!mbCoinDispKillCheck(dispNo)) {
            HuPrcVSleep();
        }
    }
    return coinDiff;
}

/* Convenience path for a normal, animated coin change without a floating total. */
int mbCoinAddExec(int playerNo, int coinNum)
{
    return mbCoinAddDispExec(playerNo, coinNum, FALSE, FALSE);
}

/* Clamp a two-player team's combined value and adjust changes to respect zero and max. */
int mbStatTeamMinValGet(int teamNo, int value, int max, int *addNum,
    int *result)
{
    int playerNo[2];
    int stat;
    int player;
    int i;
    int j;

    stat = value;
    for (i = 0; i < 2; i++) {
        player = mbPlayerTeamFindPlayer(teamNo, i);
        playerNo[i] = player;
        stat += addNum[player];
        result[player] = addNum[player];
    }
    /* On overflow, apply the smaller change first; on underflow, apply the larger change first. */
    if (stat > max) {
        if (addNum[playerNo[0]] > addNum[playerNo[1]]) {
            player = playerNo[0];
            playerNo[0] = playerNo[1];
            playerNo[1] = player;
        }
        stat = value;
        for (j = 0; j < 2; j++) {
            player = playerNo[j];
            if (stat + addNum[player] > max) {
                result[player] = max - stat;
                stat = max;
            } else {
                stat += addNum[player];
            }
        }
    } else if (stat < 0) {
        if (addNum[playerNo[0]] < addNum[playerNo[1]]) {
            player = playerNo[0];
            playerNo[0] = playerNo[1];
            playerNo[1] = player;
        }
        stat = value;
        for (j = 0; j < 2; j++) {
            player = playerNo[j];
            if (stat + addNum[player] < 0) {
                result[player] = -stat;
                stat = 0;
            } else {
                stat += addNum[player];
            }
        }
    }
    return stat - value;
}

/* Apply changes to all players, splitting team adjustments and optionally animating each coin.
 * The per-coin loop uses the gain cue for both positive and negative changes. */
static void CoinAddAllProc(int *addNum, BOOL fastF, int *result)
{
    int coinNum[4];
    int delay[4];
    int time[4];
    int coinChg[4];
    BOOL activeF[4];
    int playerNum;
    int i;

    for (i = 0; i < 4; i++) {
        coinNum[i] = addNum[i];
        activeF[i] = FALSE;
    }
    if (!GWTeamFGet()) {
        for (i = 0; i < 4; i++) {
            int coinNew = coinNum[i] + mbPlayerCoinGet(i);

            if (coinNew > 999) {
                coinNum[i] = 999 - mbPlayerCoinGet(i);
            } else if (coinNew < 0) {
                coinNum[i] = -mbPlayerCoinGet(i);
            }
            result[i] = coinNum[i];
            activeF[i] = TRUE;
        }
    } else {
        for (i = 0; i < 2; i++) {
            playerNum = mbPlayerTeamFindPlayer(i, 0);
            activeF[playerNum] = TRUE;
            coinNum[playerNum] = mbStatTeamMinValGet(i,
                mbPlayerTeamCoinGet(i), 999, coinNum, result);
        }
    }
    if (!fastF) {
        for (i = 0; i < 4; i++) {
            if (!activeF[i]) {
                continue;
            }
            if (abs(coinNum[i]) >= 50) {
                delay[i] = 1;
            } else if (abs(coinNum[i]) >= 20) {
                delay[i] = 3;
            } else {
                delay[i] = 6;
            }
            time[i] = delay[i];
            coinChg[i] = (coinNum[i] >= 0) ? 1 : -1;
        }
        do {
            playerNum = 0;
            for (i = 0; i < 4; i++) {
                if (!activeF[i] || coinNum[i] == 0) {
                    continue;
                }
                if (--time[i] == 0) {
                    mbPlayerCoinAdd(i, coinChg[i]);
                    coinNum[i] -= coinChg[i];
                    mbAudFXPlay(MSM_SE_CMN_08);
                    time[i] = delay[i];
                }
                playerNum++;
            }
            HuPrcVSleep();
        } while (playerNum != 0);
    } else {
        for (i = 0; i < 4; i++) {
            if (activeF[i]) {
                mbPlayerCoinAdd(i, coinNum[i]);
            }
        }
    }
    /* One change-end cue plays after the batch, regardless of the changes' direction. */
    mbAudFXPlay(MSM_SE_CMN_16);
}

/* Change all four players using per-player display choices, then wait for active displays. */
void mbCoinAddAllProcExecV(int *addNum, BOOL *dispF, BOOL fastF)
{
    int result[4];
    int dispNo[4];
    HuVecF pos;
    BOOL waitF;
    int i;

    CoinAddAllProc(addNum, fastF, result);
    for (i = 0; i < 4; i++) {
        dispNo[i] = 0;
        if (result[i] != 0 || dispF[i]) {
            mbPlayerPosGet(i, &pos);
            pos.y += 250.0f;
            dispNo[i] = mbCoinDispCreate(&pos, result[i], dispF[i], TRUE);
        }
    }
    do {
        waitF = FALSE;
        for (i = 0; i < 4; i++) {
            if (dispNo[i] != 0 && !mbCoinDispKillCheck(dispNo[i])) {
                waitF = TRUE;
            }
        }
        HuPrcVSleep();
    } while (waitF);
}

/* Board events apply four coin changes and optionally display each nonzero applied change.
 * The wait uses the last created display; with dispF true and no applied changes, its slot is
 * uninitialized but still passed to the removal check. */
void mbCoinAddAllProcExec(int player0Change, int player1Change, int player2Change,
                          int player3Change, BOOL dispF, BOOL fastF)
{
    int addNum[4];
    int result[4];
    HuVecF pos;
    int dispNo;
    int i;

    addNum[0] = player0Change;
    addNum[1] = player1Change;
    addNum[2] = player2Change;
    addNum[3] = player3Change;
    CoinAddAllProc(addNum, fastF, result);
    if (dispF) {
        for (i = 0; i < 4; i++) {
            if (result[i] != 0) {
                mbPlayerPosGet(i, &pos);
                pos.y += 250.0f;
                dispNo = mbCoinDispCapsuleCreate(&pos, result[i]);
            }
        }
        while (!mbCoinDispKillCheck(dispNo)) {
            HuPrcVSleep();
        }
    }
}

/* Board events apply four coin changes with per-coin animation and no floating displays. */
void mbCoinAddAllExec(int player0Change, int player1Change, int player2Change, int player3Change)
{
    int addNum[4];
    int result[4];

    addNum[0] = player0Change;
    addNum[1] = player1Change;
    addNum[2] = player2Change;
    addNum[3] = player3Change;
    CoinAddAllProc(addNum, FALSE, result);
}
