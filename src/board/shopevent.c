/* Board shop events display capsule offers and let a player buy or replace one. */
/* Use the Dolphin math declarations for the shop's movement and menu animations. */
#define _MATH_H
#include "dolphin/math.h"

#include "game/board/main.h"
#include "game/board/audio.h"
#include "game/board/capsule.h"
#include "game/board/masu.h"
#include "game/board/object.h"
#include "game/board/pause.h"
#include "game/board/player.h"
#include "game/board/status.h"
#include "game/board/tutorial.h"
#include "game/board/window.h"
#include "game/esprite.h"
#include "game/flag.h"
#include "game/hsfex.h"
#include "game/memory.h"
#include "game/pad.h"
#include "game/sprite.h"

#include "humath.h"
#include "datadir_enum.h"
#include "messdir_enum.h"
#include "msm_se.h"
#include "string.h"

typedef void (*MBSHOPOBJHOOK)(int modelId, int shopNo);

enum {
    SHOP_MASU_ATTR_PATH_LINK = (1 << 5),
    SHOP_LIST_ENTRY_COUNT = 33,
    SHOP_LIST_ENTRY_SIZE = 16,
    SHOP_TUTORIAL_ENTRY = 19,
    SHOP_TUTORIAL_SELECT = 20,
    SHOP_SFX_NIGHT_SUCCESS = MSM_SE_GUIDE_36,
    SHOP_SFX_NIGHT_PROMPT = MSM_SE_GUIDE_37,
    SHOP_SFX_NIGHT_UNAVAILABLE = MSM_SE_GUIDE_38,
    SHOP_SFX_DAY_SUCCESS = MSM_SE_GUIDE_62,
    SHOP_SFX_DAY_PROMPT = MSM_SE_GUIDE_63,
    SHOP_SFX_DAY_UNAVAILABLE = MSM_SE_GUIDE_64,
    SHOP_SFX_OPEN = MSM_SE_BRD00_139,
    SHOP_SFX_CLOSE = MSM_SE_BRD00_140,
    SHOP_DATA_NIGHT_MODEL = DATANUM(DATA_capsuleshop, 0),
    SHOP_DATA_NIGHT_MOTION = DATANUM(DATA_capsuleshop, 1),
    SHOP_DATA_NIGHT_MOTION_CLOSE = DATANUM(DATA_capsuleshop, 4),
    SHOP_DATA_DAY_MODEL = DATANUM(DATA_capsuleshop, 5),
    SHOP_DATA_DAY_MOTION = DATANUM(DATA_capsuleshop, 6),
    SHOP_DATA_DAY_MOTION_CLOSE = DATANUM(DATA_capsuleshop, 7),
    SHOP_MESSAGE_ENTER_CHOICE = MESSNUM(MESS_SHOP_EVENT, 0),
    SHOP_MESSAGE_NOT_ENOUGH_COINS = MESSNUM(MESS_SHOP_EVENT, 1),
    SHOP_MESSAGE_GREETING = MESSNUM(MESS_SHOP_EVENT, 2),
    SHOP_MESSAGE_NO_OFFERS = MESSNUM(MESS_SHOP_EVENT, 3),
    SHOP_MESSAGE_PURCHASED = MESSNUM(MESS_SHOP_EVENT, 7),
    SHOP_MESSAGE_THANK_YOU = MESSNUM(MESS_SHOP_EVENT, 8),
    SHOP_MESSAGE_DISCARD_CHOICE = MESSNUM(MESS_SHOP_EVENT, 9),
    SHOP_MESSAGE_REPLACED = MESSNUM(MESS_SHOP_EVENT, 10),
    SHOP_MESSAGE_LAST_TURN = MESSNUM(MESS_SHOP_EVENT, 11),
    SHOP_MESSAGE_NIGHT_RESTRICTION = MESSNUM(MESS_SHOP_EVENT, 13),
};

typedef struct MBSHOPWORK {
    int playerNo; /* Player currently visiting the shop. */
    int shopNo; /* Board space ID used to find this shop. */
} MBSHOPWORK;

typedef struct MBSHOPOMWORK {
    int modelId; /* Main shop model, or -1 when this shop has no model. */
    int shopNo; /* Index in ev_ShopOMObj. */
    int masuId; /* Board space that starts the shop path. */
    int masuLinkId; /* First linked space after the shop space. */
    BOOL pathF; /* TRUE when the shop is reached along linked spaces. */
    int masuEndId; /* Last space the player walks to on the shop path; -1 for direct approach. */
    int reservedStateA; /* Cleared during setup and never read in this file. */
    int reservedStateB; /* Cleared during setup and never read in this file. */
    BOOL modelMotionF; /* TRUE when the main model has separate open and close motions. */
    BOOL motionExecF; /* TRUE while the shop's open or close motion is running. */
    BOOL openF; /* TRUE while the shop is opening or open. */
    int backModelId[8]; /* Attached shop model IDs; -1 marks an unused slot. */
    int backMotionNo[8]; /* 0/1 reverse on closing; 2/3 play forward; 4 loops continuously. */
    BOOL backStaysVisibleF[8]; /* TRUE when an attached model stays visible after closing. */
    HuVecF masuPos; /* World position of the shop model's board space. */
    HuVecF capsulePos; /* World position used for the temporary shop visit model. */
    HuVecF shopPos; /* World position where the player approaches the counter. */
} MBSHOPOMWORK;

typedef struct ShopOffer_s {
    int capsuleNo; /* Capsule ID offered for sale. */
    int cost; /* Coin price for this player. */
    int messageId; /* Capsule use text inserted into purchase and replacement dialogue. */
    char costText[16]; /* Formatted offer price; written here but not read later in this file. */
} SHOP_OFFER;

#define SHOP_SELECT_WINDOW_SPACING 576.0f
#define SHOP_SELECT_CURSOR_OFFSET (-32.0f)

enum {
    SHOP_SELECT_PANEL_FILE = DATANUM(DATA_capsuleshop, 12),
    SHOP_SELECT_CURSOR_FILE = DATANUM(DATA_capsuleshop, 13),
    SHOP_SELECT_PANEL_PRIORITY = 100,
    SHOP_SELECT_ESP_PRIORITY = 90,
    SHOP_SELECT_DRAW_NO = 32,
    SHOP_SELECT_SPR_ATTR = HUSPR_ATTR_NOANIM | HUSPR_ATTR_LINEAR,
    SHOP_SELECT_MODEL_LAYER = 6,
    SHOP_SELECT_CAMERA = HU3D_CAM2,
    SHOP_SELECT_ROTATE_FRAMES = 10,
    SHOP_SELECT_MOVE_FRAMES = 20,
    SHOP_SELECT_STICK_THRESHOLD = 20,
};

/* Rows select the offer count; X is angle in degrees, Y is radius, and Z is unused. */
static HuVecF ev_ShopCapsulePlayer[3][3] = {
    {
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
    },
    {
        { 90.0f, 50.0f, 0.0f },
        { -90.0f, 50.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
    },
    {
        { 90.0f, 50.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
        { -90.0f, 50.0f, 0.0f },
    },
};

static HuVecF ev_ShopLightPos = { -10000.0f, 10000.0f, -10000.0f };
static HuVecF ev_ShopLightDir = { 1.0f, -1.0f, -1.0f };
static int ev_ShopSprFileTbl[4] = {
    DATANUM(DATA_board, 38),
    DATANUM(DATA_board, 35),
    DATANUM(DATA_board, 35),
    DATANUM(DATA_board, 35),
};
static HuVecF ev_ShopWinPos = { 288.0f, 176.0f, 0.0f };
/* Screen positions and projection depth for one, two, or three carousel offers. */
static HuVecF ev_ShopCapsulePos[3][3] = {
    {
        { 288.0f, 170.0f, 1000.0f },
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
    },
    {
        { 214.0f, 170.0f, 1000.0f },
        { 362.0f, 170.0f, 1000.0f },
        { 0.0f, 0.0f, 0.0f },
    },
    {
        { 192.0f, 170.0f, 1000.0f },
        { 288.0f, 170.0f, 1000.0f },
        { 384.0f, 170.0f, 1000.0f },
    },
};

/* Board shop objects are indexed independently of their board space IDs. */
static OMOBJ *ev_ShopOMObj[GW_PLAYER_MAX];
static GXColor ev_ShopLightColor = { 255, 255, 255, 255 };

void mbev_ShopCreate(int dataNum, int motDataNum);
void mbev_ShopBackMotCreate(int dataNum, int motDataNum, int motNo, BOOL linkF, char *hookName);
extern int mbCapObjCreate(int capsuleNo, BOOL flag);
extern void mbCapObjKill(int objId);
extern int mbCapDescWinCreate(int capsuleNo);
extern s8 mbPadStkXGet(s32 padNo);
extern void mbev_CapVecChase(float weight, HuVecF *src, HuVecF *target,
    HuVecF *out);
extern s32 mbBGRead(s32 dataNum);
extern void mbBGReadWait(s32 statId);
extern int mbCapUseMesGet(int capsuleNo);
extern int mbCapBuyCostGet(s16 capsuleNo, s16 playerNo);
extern int mbCapShopListGet(int playerNo, s8 *shopList);
extern int mbCapDelete(int capsuleNo, BOOL repeatF);
extern void mbCapNumInc(int capsuleNo, int mode);
extern int mbCapObjColorCreate(int capsuleNo, BOOL createF);
extern void mbCapObjColorLayerSet(int id, int layer);
extern void mbCapObjColorPosSetV(int id, HuVecF *pos);
extern void mbCapCapsuleGet(int playerNo, int capsuleNo);
extern void mbCapObjColorKill(int id);
void mbCapObjColorScaleSet(int id, float x, float y, float z);
extern int mbCoinAddExec(int playerNo, int coinNum);
extern int mbCoinAddProcExec(int playerNo, int coinNum, BOOL dispF,
    BOOL fastF);
extern void mbCameraPlayerViewSet(int playerNo, int viewNo);
extern void mbComChoiceLeftSet(void);
extern void mbComChoiceRightSet(void);
extern void mbev_Scroll(int playerNo, BOOL mapF);
static void ev_ShopOMExec(OMOBJ *obj);
static void ev_ShopOpenSet(int shopNo, BOOL openF);
static void ev_Shop(MBSHOPWORK *visitWork);
static int ev_ShopSelect(MBSHOPWORK *visitWork, SHOP_OFFER *offer, int offerNum, int winType);
static int ev_ShopMesGet(int dayMessageId);
void mbev_ShopExObjHookSet(MBSHOPOBJHOOK hook);

static int ev_ShopNum;
static MBSHOPOBJHOOK ev_ShopExObjHook;
static BOOL ev_ShopEnableF;

/* Enables or disables board shop visits; called by board event setup. */
void mbev_ShopEnableSet(BOOL enableF)
{
    ev_ShopEnableF = enableF;
}

/* Called during board setup to create shop objects without a custom object hook. */
void mbev_ShopInit(int dataNum)
{
    mbev_ShopExObjHookSet(NULL);
    mbev_ShopCreate(dataNum, -1);
}

/* Called during board setup to create shop objects with a caller-supplied setup hook. */
void mbev_ShopExInit(int dataNum, MBSHOPOBJHOOK hook)
{
    mbev_ShopExObjHookSet(hook);
    mbev_ShopCreate(dataNum, -1);
}

/* Enables visits and creates shop objects during board setup; a shop without a marked path link
 * ends the scan. */
void mbev_ShopCreate(int dataNum, int motDataNum)
{
    HuVecF shopPos;
    HuVecF masuPos;
    HuVecF approachDirection;
    int shopPathMasuIds[3];
    int motionDataNums[2];
    int masuId;
    int backModelSlot;
    int shopNo;

    ev_ShopEnableF = TRUE;
    for (masuId = 0; masuId < 3; masuId++) {
        ev_ShopOMObj[masuId] = NULL;
    }
    shopNo = 0;
    motionDataNums[0] = motDataNum;
    motionDataNums[1] = -1;
    for (masuId = 1; masuId < mbMasuNumGet(); masuId++) {
        int linkMasuId;
        int shopLinkMasuId;
        int shopMasuId;
        int nextMasuId;
        MBSHOPOMWORK *shopWork;
        OMOBJ *shopObj;

        if (mbMasuTypeGet(masuId) != 9) {
            continue;
        }
        linkMasuId = mbMasuAttrFindLink(masuId, SHOP_MASU_ATTR_PATH_LINK);
        if (linkMasuId < 0) {
            break;
        }
        nextMasuId = mbMasuAttrFindLink(linkMasuId, SHOP_MASU_ATTR_PATH_LINK);
        shopWork = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MBSHOPOMWORK), HU_MEMNUM_OVL);
        memset(shopWork, 0, sizeof(MBSHOPOMWORK));
        shopMasuId = masuId;
        shopLinkMasuId = linkMasuId;
        mbMasuPosGet(shopMasuId, &shopPos);
        mbMasuPosGet(shopLinkMasuId, &masuPos);
        if (nextMasuId != -1) {
            /* Keep the last two spaces: the player stops before the shop model. */
            shopWork->pathF = TRUE;
            shopPathMasuIds[0] = shopMasuId;
            shopPathMasuIds[1] = shopLinkMasuId;
            shopPathMasuIds[2] = nextMasuId;
            while ((nextMasuId = mbMasuAttrFindLink(shopPathMasuIds[2], SHOP_MASU_ATTR_PATH_LINK)) >
                   0) {
                shopPathMasuIds[0] = shopPathMasuIds[1];
                shopPathMasuIds[1] = shopPathMasuIds[2];
                shopPathMasuIds[2] = nextMasuId;
            }
            mbMasuPosGet(shopPathMasuIds[1], &shopWork->shopPos);
            mbMasuPosGet(shopPathMasuIds[2], &shopWork->masuPos);
            shopPos = shopWork->shopPos;
            masuPos = shopWork->masuPos;
            shopWork->masuEndId = shopPathMasuIds[1];
        } else {
            shopWork->pathF = FALSE;
            shopWork->masuEndId = -1;
            /* Offset the direct approach 100 units toward the starting space, then force it to the
             * model space's height. */
            PSVECSubtract(&shopPos, &masuPos, &approachDirection);
            if (PSVECMag(&approachDirection) > 0.0f) {
                PSVECNormalize(&approachDirection, &approachDirection);
            }
            PSVECScale(&approachDirection, &approachDirection, 100.0f);
            PSVECAdd(&masuPos, &approachDirection, &shopWork->shopPos);
            shopWork->shopPos.y = masuPos.y;
            mbMasuPosGet(shopLinkMasuId, &shopWork->masuPos);
        }
        PSVECSubtract(&masuPos, &shopPos, &approachDirection);
        if (PSVECMag(&approachDirection) > 0.0f) {
            PSVECNormalize(&approachDirection, &approachDirection);
        }
        PSVECScale(&approachDirection, &approachDirection, 100.0f);
        PSVECAdd(&masuPos, &approachDirection, &shopWork->capsulePos);
        if (GwSystem.curTime) {
            shopWork->capsulePos.y += 30.000002f;
        }
        shopObj = ev_ShopOMObj[shopNo] = omAddObjEx(mbObjMan, -32768, 0, 0,
            OM_GRP_NONE, ev_ShopOMExec);
        shopObj->data = shopWork;
        if (dataNum <= 0) {
            shopWork->modelId = -1;
        } else if (motDataNum != -1) {
            shopWork->modelId = mbObjCreate(dataNum, motionDataNums, TRUE);
            shopWork->modelMotionF = TRUE;
        } else {
            shopWork->modelId = mbObjCreate(dataNum, NULL, TRUE);
            shopWork->modelMotionF = FALSE;
        }
        PSVECSubtract(&shopPos, &masuPos, &approachDirection);
        if (shopWork->modelId != -1) {
            mbObjPosSetV(shopWork->modelId, &masuPos);
            mbObjRotSet(shopWork->modelId, 0.0f, HuAtan(approachDirection.x, approachDirection.z),
                        0.0f);
            mbObjMotionSpeedSet(shopWork->modelId, 0.0f);
        }
        shopWork->shopNo = shopNo;
        shopWork->masuId = shopMasuId;
        shopWork->masuLinkId = shopLinkMasuId;
        shopWork->reservedStateA = 0;
        shopWork->reservedStateB = 0;
        shopWork->motionExecF = FALSE;
        shopWork->openF = FALSE;
        if (ev_ShopExObjHook != NULL) {
            ev_ShopExObjHook(shopWork->modelId, shopNo);
        }
        for (backModelSlot = 0; backModelSlot < 8; backModelSlot++) {
            shopWork->backModelId[backModelSlot] = -1;
            shopWork->backMotionNo[backModelSlot] = 0;
            shopWork->backStaysVisibleF[backModelSlot] = FALSE;
        }
        shopNo++;
    }
    ev_ShopNum = shopNo;
}

/* Stores the optional hook called once for each shop object as it is created. */
void mbev_ShopExObjHookSet(MBSHOPOBJHOOK hook)
{
    ev_ShopExObjHook = hook;
}

/* Called during board setup to add an attached model to each shop. */
void mbev_ShopBackCreate(int dataNum, int motDataNum, int motNo, BOOL linkF)
{
    mbev_ShopBackMotCreate(dataNum, motDataNum, motNo, linkF, 0);
}

/* Called during board setup to attach models and choose their opening and closing behavior. */
void mbev_ShopBackMotCreate(int dataNum, int motDataNum, int motNo, BOOL linkF, char *hookName)
{
    int motionDataNums[16];
    HuVecF shopModelPos;
    HuVecF shopModelRot;
    int shopIndex;
    int attachedModelSlot;

    for (shopIndex = 0; shopIndex < 3; shopIndex++) {
        OMOBJ *shopObj = ev_ShopOMObj[shopIndex];
        MBSHOPOMWORK *shopWork;

        if (shopObj == NULL) {
            continue;
        }
        shopWork = shopObj->data;
        for (attachedModelSlot = 0; attachedModelSlot < 8; attachedModelSlot++) {
            if (shopWork->backModelId[attachedModelSlot] == -1) {
                break;
            }
        }
        if (motDataNum >= 0) {
            motionDataNums[0] = motDataNum;
            motionDataNums[1] = -1;
            shopWork->backModelId[attachedModelSlot] = mbObjCreate(dataNum, motionDataNums, linkF);
        } else {
            shopWork->backModelId[attachedModelSlot] = mbObjCreate(dataNum, NULL, linkF);
        }
        if (shopWork->modelId != -1) {
            mbObjPosGet(shopWork->modelId, &shopModelPos);
            mbObjRotGet(shopWork->modelId, &shopModelRot);
        } else {
            shopModelPos.x = shopModelPos.y = shopModelPos.z = 0.0f;
            shopModelRot.x = shopModelRot.y = shopModelRot.z = 0.0f;
        }
        if (hookName == NULL) {
            mbObjPosSetV(shopWork->backModelId[attachedModelSlot], &shopModelPos);
            mbObjRotSetV(shopWork->backModelId[attachedModelSlot], &shopModelRot);
        } else if (shopWork->modelId != -1) {
            mbObjHookSet(shopWork->modelId, hookName, shopWork->backModelId[attachedModelSlot]);
        }
        /* Modes 0-3 wait for the shop to open; mode 4 keeps playing throughout. */
        shopWork->backMotionNo[attachedModelSlot] = motNo;
        switch (shopWork->backMotionNo[attachedModelSlot]) {
            case 0:
            case 1:
                mbObjMotionSpeedSet(shopWork->backModelId[attachedModelSlot], 0.0f);
                break;
            case 2:
            case 3:
                mbObjMotionSpeedSet(shopWork->backModelId[attachedModelSlot], 0.0f);
                break;
            case 4: {
                MBMODELID modelId = shopWork->backModelId[attachedModelSlot];

                mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
                break;
            }
        }
        if (shopWork->backMotionNo[attachedModelSlot] == 1 ||
            shopWork->backMotionNo[attachedModelSlot] == 3) {
            shopWork->backStaysVisibleF[attachedModelSlot] = TRUE;
        }
        if (!shopWork->backStaysVisibleF[attachedModelSlot]) {
            mbObjDispSet(shopWork->backModelId[attachedModelSlot], FALSE);
        }
    }
}

/* Per-frame object callback that detects motion completion and hides closed decorations. */
static void ev_ShopOMExec(OMOBJ *shopObj)
{
    MBSHOPOMWORK *shopWork = shopObj->data;
    int attachedModelSlot;

    if (mbExitCheck() || ev_ShopOMObj[shopWork->shopNo] == NULL) {
        omDelObjEx(mbObjMan, shopObj);
        return;
    }
    if (shopWork->motionExecF && shopWork->modelId != -1) {
        if (shopWork->openF) {
            if (mbObjMotionTimeGet(shopWork->modelId) >= mbObjMotionMaxTimeGet(shopWork->modelId)) {
                shopWork->motionExecF = FALSE;
            }
        } else {
            if (shopWork->modelMotionF) {
                if (mbObjMotionTimeGet(shopWork->modelId) >=
                    mbObjMotionMaxTimeGet(shopWork->modelId)) {
                    shopWork->motionExecF = FALSE;
                }
            } else if (mbObjMotionTimeGet(shopWork->modelId) <= 0.0f) {
                shopWork->motionExecF = FALSE;
            }
            if (!shopWork->motionExecF) {
                for (attachedModelSlot = 0; attachedModelSlot < 8; attachedModelSlot++) {
                    if (shopWork->backModelId[attachedModelSlot] != -1 &&
                        !shopWork->backStaysVisibleF[attachedModelSlot]) {
                        mbObjDispSet(shopWork->backModelId[attachedModelSlot], FALSE);
                    }
                }
            }
        }
    }
}

/* Starts the shop's opening or closing motion when the visit sequence requests it. */
static void ev_ShopOpenSet(int shopNo, BOOL openF)
{
    OMOBJ *shopObj = ev_ShopOMObj[shopNo];
    MBSHOPOMWORK *shopWork = shopObj->data;
    int attachedModelSlot;

    if (shopWork->modelId == -1) {
        return;
    }
    shopWork->motionExecF = TRUE;
    shopWork->openF = openF;
    if (shopWork->openF) {
        mbAudFXPlay(SHOP_SFX_OPEN);
        mbObjMotionSet(shopWork->modelId, 0, 0);
        mbObjMotionSpeedSet(shopWork->modelId, 1.0f);
        for (attachedModelSlot = 0; attachedModelSlot < 8; attachedModelSlot++) {
            if (shopWork->backModelId[attachedModelSlot] == -1) {
                continue;
            }
            switch (shopWork->backMotionNo[attachedModelSlot]) {
                case 0:
                case 1:
                case 2:
                case 3:
                    mbObjMotionTimeSet(shopWork->backModelId[attachedModelSlot], 0.0f);
                    mbObjMotionSpeedSet(shopWork->backModelId[attachedModelSlot], 1.0f);
                    mbObjDispSet(shopWork->backModelId[attachedModelSlot], TRUE);
                    break;
                default:
                    mbObjDispSet(shopWork->backModelId[attachedModelSlot], TRUE);
                    break;
            }
        }
    } else {
        mbAudFXPlay(SHOP_SFX_CLOSE);
        if (shopWork->modelMotionF) {
            mbObjMotionSet(shopWork->modelId, 1, 0);
            mbObjMotionSpeedSet(shopWork->modelId, 1.0f);
        } else {
            /* Shops with one motion close by playing their opening motion backward. */
            mbObjMotionSpeedSet(shopWork->modelId, -1.0f);
        }
        for (attachedModelSlot = 0; attachedModelSlot < 8; attachedModelSlot++) {
            if (shopWork->backModelId[attachedModelSlot] == -1) {
                continue;
            }
            switch (shopWork->backMotionNo[attachedModelSlot]) {
                case 0:
                case 1:
                    mbObjMotionSpeedSet(shopWork->backModelId[attachedModelSlot], -1.0f);
                    break;
                case 2:
                case 3:
                    mbObjMotionTimeSet(shopWork->backModelId[attachedModelSlot], 0.0f);
                    mbObjMotionSpeedSet(shopWork->backModelId[attachedModelSlot], 1.0f);
                    break;
            }
        }
    }
}

/* On shop arrival, hides remaining moves and disables pause, then unconditionally enables both
 * after the visit. */
int mbev_Shop(int playerNo, int shopNo)
{
    MBSHOPWORK *visitWork;
    void *allocatedVisitWork;

    mbMoveNumDispSet(playerNo, FALSE);
    allocatedVisitWork = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MBSHOPWORK), HU_MEMNUM_OVL);
    visitWork = allocatedVisitWork;
    memset(visitWork, 0, sizeof(MBSHOPWORK));
    visitWork->playerNo = playerNo;
    visitWork->shopNo = shopNo;
    mbPauseDisableSet(TRUE);
    ev_Shop(visitWork);
    HuMemDirectFree(visitWork);
    mbPauseDisableSet(FALSE);
    mbMoveNumDispSet(playerNo, TRUE);
    return 0;
}

/* Called while preparing a visit to allocate the temporary capsule offer list. */
static inline s8 *ev_ShopListAlloc(void)
{
    return HuMemDirectMallocNum(HEAP_HEAP,
        SHOP_LIST_ENTRY_COUNT * SHOP_LIST_ENTRY_SIZE, HU_MEMNUM_OVL);
}

/* Called while walking to a linked shop to find the player's final approach space. */
static inline int ev_ShopMasuEndGet(int shopNo, OMOBJ **shopObj,
    MBSHOPOMWORK **shopWork)
{
    *shopObj = ev_ShopOMObj[shopNo];
    *shopWork = (*shopObj)->data;

    return (*shopWork)->masuEndId;
}

/* Called by mbev_Shop to prepare offers, walk to the counter, and handle one purchase. */
static void ev_Shop(MBSHOPWORK *visitWork)
{
    int motionDataNum[16];
    SHOP_OFFER offer[3];
    SHOP_OFFER savedOffer;
    SHOP_OFFER *currentOffer;
    HuVecF playerPos;
    HuVecF masuPos;
    HuVecF eventPos;
    HuVecF direction;
    HuVecF shopPos;
    HuVecF savedPlayerPos;
    HuVecF displayOffset;
    s8 *shopCapsuleList;
    int firstOfferIndex;
    int capsuleObjId[3];
    int secondOfferIndex;
    BOOL visitDoneF;
    int coinAddResult; /* The coin update return value is captured but never used. */
    int discardedCapsuleNo;
    int shopCapsuleCount;
    int archiveReadStatus;
    int selectionResult; /* Tutorial capsule ID, later replaced by the selected offer index. */
    int shopNo;
    int lightId;
    int pathSpaceCount;
    int shopIndex;
    int offerNum;
    int winType;
    int masuId;
    int loopIndex;
    int shopModelId;
    int capsuleModelId; /* Additional model slot stays unused during this visit. */
    int currentMasuId;
    BOOL comSaveCoinsF;
    float facingAngle;

    if (!ev_ShopEnableF) {
        return;
    }
    shopModelId = capsuleModelId = lightId = -1;
    for (loopIndex = 0; loopIndex < 3; loopIndex++) {
        capsuleObjId[loopIndex] = -1;
    }
    if (!GwSystem.curTime) {
        winType = 8;
    } else {
        winType = 9;
    }
    archiveReadStatus = mbBGRead(SHOP_DATA_NIGHT_MODEL);
    mbPlayerMotionShiftSet(visitWork->playerNo, 1, 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);
    {
        int foundShopNo;
        OMOBJ *shopObj;
        MBSHOPOMWORK *candidateShopWork;
        int shopMasuId;

        shopMasuId = visitWork->shopNo;
        for (shopIndex = 0; shopIndex < ev_ShopNum; shopIndex++) {
            shopObj = ev_ShopOMObj[shopIndex];
            candidateShopWork = shopObj->data;

            if (candidateShopWork->masuId == shopMasuId) {
                foundShopNo = shopIndex;
                goto shop_found;
            }
        }
        foundShopNo = -1;

shop_found:
        shopNo = foundShopNo;
    }
    {
        shopCapsuleList = ev_ShopListAlloc();
        shopCapsuleCount = mbCapShopListGet(visitWork->playerNo, shopCapsuleList);

        currentOffer = offer;
        for (loopIndex = 0, offerNum = 0;
            loopIndex < 3 && loopIndex < shopCapsuleCount;
            loopIndex++, currentOffer++) {
            currentOffer->capsuleNo = shopCapsuleList[loopIndex * SHOP_LIST_ENTRY_SIZE];
            currentOffer->cost = mbCapBuyCostGet((s16)currentOffer->capsuleNo,
                (s16)visitWork->playerNo);
            currentOffer->messageId = mbCapUseMesGet(currentOffer->capsuleNo);
            sprintf(currentOffer->costText, "%d", currentOffer->cost);
            offerNum++;
        }
        for (loopIndex = 0, currentOffer = offer; loopIndex < offerNum;
             loopIndex++, currentOffer++) {
            if (currentOffer->cost > mbPlayerCoinGet(visitWork->playerNo)) {
                /* An unaffordable offer becomes capsule zero and gets its price and text. */
                currentOffer->capsuleNo = 0;
                currentOffer->cost = mbCapBuyCostGet((s16)currentOffer->capsuleNo,
                    (s16)visitWork->playerNo);
                currentOffer->messageId = mbCapUseMesGet(currentOffer->capsuleNo);
                sprintf(currentOffer->costText, "%d", currentOffer->cost);
            }
        }
        for (loopIndex = 0, currentOffer = offer; loopIndex < offerNum;
             loopIndex++, currentOffer++) {
            mbCapNumInc(currentOffer->capsuleNo, 1);
        }
        for (loopIndex = 0; loopIndex < 64 && offerNum >= 2; loopIndex++) {
            firstOfferIndex = mbRandMod(offerNum);
            secondOfferIndex = mbRandMod(offerNum);

            if (firstOfferIndex != secondOfferIndex) {
                /* Both writes target the first offer, restoring it instead of swapping. */
                savedOffer = offer[firstOfferIndex];
                offer[firstOfferIndex] = offer[secondOfferIndex];
                offer[firstOfferIndex] = savedOffer;
            }
        }
        HuMemDirectFree(shopCapsuleList);
    }
    HuPrcVSleep();

    if (!_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        /* Ordinary visits stop on the final turn; the tutorial controls entry itself. */
        if (GwSystem.turnNo >= GwSystem.turnMax) {
            if (!GwSystem.curTime) {
                mbAudFXPlay(SHOP_SFX_DAY_PROMPT);
            } else {
                mbAudFXPlay(SHOP_SFX_NIGHT_PROMPT);
            }
            mbWinCreate(2, ev_ShopMesGet(SHOP_MESSAGE_LAST_TURN), winType);
            mbWinTopWait();
            goto cleanup;
        }
        if (mbPlayerCoinGet(visitWork->playerNo) > 4) {
            mbWinCreateChoice(2, ev_ShopMesGet(SHOP_MESSAGE_ENTER_CHOICE),
                -1, 0);
            if (GwPlayer[visitWork->playerNo].comF) {
                comSaveCoinsF = FALSE;
                /* Save coins when a nearby star or team budget takes priority. */
                if (mbMasuFind_TypeStepGet((s16)visitWork->shopNo, 7)
                    < GwPlayer[visitWork->playerNo].moveNum) {
                    comSaveCoinsF = TRUE;
                }
                if (mbMasuFind_TypeStepGet((s16)visitWork->shopNo, 7) < 20
                    && mbPlayerCoinGet(visitWork->playerNo) < 25
                    && mbPlayerCoinGet(visitWork->playerNo) >= 20) {
                    comSaveCoinsF = TRUE;
                }
                if (GWTeamFGet()
                    && mbPlayerCoinGet(visitWork->playerNo) < 25
                    && mbPlayerCoinGet(visitWork->playerNo) >= 20) {
                    comSaveCoinsF = TRUE;
                }
                if (MBCapsuleEffRandF() < 0.7f
                    && !comSaveCoinsF
                    && mbPlayerCapsuleNumGet(visitWork->playerNo)
                        < mbPlayerCapsuleMaxGet()) {
                    mbComChoiceLeftSet();
                } else {
                    mbComChoiceRightSet();
                }
            }
            mbWinTopWait();
            if (mbWinTopChoiceGet() == 0 && mbWinTopChoiceGet() != -1) {
                goto enter_shop;
            }
        } else {
            if (!GwSystem.curTime) {
                mbAudFXPlay(SHOP_SFX_DAY_UNAVAILABLE);
            } else {
                mbAudFXPlay(SHOP_SFX_NIGHT_UNAVAILABLE);
            }
            mbWinCreate(2, ev_ShopMesGet(SHOP_MESSAGE_NOT_ENOUGH_COINS),
                winType);
            mbWinTopWait();
        }
        goto cleanup;
    } else {
        if (mbTutorialCall(SHOP_TUTORIAL_ENTRY) == 1) {
            goto enter_shop;
        }
        goto cleanup;
    }

enter_shop:
    /* Arrange the offer capsules around the counter, relative to the approach direction. */
    for (loopIndex = 0; loopIndex < offerNum; loopIndex++) {
        capsuleObjId[loopIndex] = mbCapObjColorCreate(offer[loopIndex].capsuleNo, 0);
        mbCapObjColorLayerSet(capsuleObjId[loopIndex], 4);
        {
            OMOBJ *shopObj = ev_ShopOMObj[shopNo];
            MBSHOPOMWORK *shopWork = shopObj->data;

            masuPos = shopWork->masuPos;
        }
        {
            OMOBJ *shopObj = ev_ShopOMObj[shopNo];
            MBSHOPOMWORK *shopWork = shopObj->data;

            shopPos = shopWork->shopPos;
        }
        PSVECSubtract(&masuPos, &shopPos, &direction);
        eventPos = masuPos;
        eventPos.y += 100.0f;
        displayOffset = ev_ShopCapsulePlayer[offerNum - 1][loopIndex];
        eventPos.x += displayOffset.y
            * sin((M_PI * (displayOffset.x
                + (180.0 * (atan2(direction.x, direction.z) / M_PI))))
                / 180.0);
        eventPos.z += displayOffset.y
            * cos((M_PI * (displayOffset.x
                + (180.0 * (atan2(direction.x, direction.z) / M_PI))))
                / 180.0);
        mbCapObjColorPosSetV(capsuleObjId[loopIndex], &eventPos);
        mbCapObjColorScaleSet(capsuleObjId[loopIndex], 0.5f, 0.5f, 0.5f);
        HuPrcVSleep();
    }
    if (archiveReadStatus != -1) {
        mbBGReadWait(archiveReadStatus);
    }
    if (!GwSystem.curTime) {
        motionDataNum[0] = SHOP_DATA_DAY_MOTION;
        motionDataNum[1] = SHOP_DATA_DAY_MOTION;
        motionDataNum[2] = SHOP_DATA_DAY_MOTION_CLOSE;
        motionDataNum[3] = -1;
        shopModelId = mbObjCreate(SHOP_DATA_DAY_MODEL, motionDataNum, FALSE);
    } else {
        motionDataNum[0] = SHOP_DATA_NIGHT_MOTION;
        motionDataNum[1] = SHOP_DATA_NIGHT_MOTION;
        motionDataNum[2] = SHOP_DATA_NIGHT_MOTION_CLOSE;
        motionDataNum[3] = -1;
        shopModelId = mbObjCreate(SHOP_DATA_NIGHT_MODEL, motionDataNum, FALSE);
    }
    lightId = Hu3DLLightCreateV(mbObjModelIDGet(shopModelId),
        &ev_ShopLightPos, &ev_ShopLightDir, &ev_ShopLightColor);
    Hu3DLLightStaticSet(mbObjModelIDGet(shopModelId), lightId, TRUE);
    Hu3DLLightInfinitytSet(mbObjModelIDGet(shopModelId), lightId);

    {
        OMOBJ *shopObj = ev_ShopOMObj[shopNo];
        MBSHOPOMWORK *shopWork = shopObj->data;

        masuPos = shopWork->masuPos;
    }
    {
        OMOBJ *shopObj = ev_ShopOMObj[shopNo];
        MBSHOPOMWORK *shopWork = shopObj->data;

        shopPos = shopWork->shopPos;
    }
    mbMasuPosGet((s16)visitWork->shopNo, &playerPos);
    PSVECSubtract(&masuPos, &shopPos, &direction);
    {
        OMOBJ *shopObj = ev_ShopOMObj[shopNo];
        MBSHOPOMWORK *shopWork = shopObj->data;

        eventPos = shopWork->capsulePos;
    }
    mbObjPosSetV(shopModelId, &eventPos);
    mbObjRotSet(shopModelId, 0.0f,
        (float)(180.0 + ((atan2(direction.x, direction.z) / M_PI) * 180.0)),
        0.0f);
    mbObjMotionSet(shopModelId, 1, HU3D_MOTATTR_LOOP);
    ev_ShopOpenSet(shopNo, TRUE);
    omVibrate((s16)visitWork->playerNo, 20, 7, 3);

    {
        OMOBJ *shopObj = ev_ShopOMObj[shopNo];
        MBSHOPOMWORK *shopWork = shopObj->data;

        masuPos = shopWork->masuPos;
    }
    {
        OMOBJ *shopObj = ev_ShopOMObj[shopNo];
        MBSHOPOMWORK *shopWork = shopObj->data;

        shopPos = shopWork->shopPos;
    }
    mbMasuPosGet((s16)visitWork->shopNo, &playerPos);
    PSVECSubtract(&shopPos, &playerPos, &direction);
    facingAngle = (float)((atan2(direction.x, direction.z) / M_PI) * 180.0);
    mbPlayerRotateStart(visitWork->playerNo, facingAngle, 15);
    while (!mbPlayerRotateCheck(visitWork->playerNo)) {
        HuPrcVSleep();
    }
    {
        OMOBJ *shopObj;
        MBSHOPOMWORK *shopWork;

        while ((shopObj = ev_ShopOMObj[shopNo]),
            (shopWork = shopObj->data),
            shopWork->motionExecF != FALSE) {
            HuPrcVSleep();
        }
    }
    savedPlayerPos = playerPos; /* Saved here but not read later in this event. */
    {
        OMOBJ *shopObj = ev_ShopOMObj[shopNo];
        MBSHOPOMWORK *shopWork = shopObj->data;

        shopPos = shopWork->shopPos;
    }
    {
        int visitedSpaceIds[16];

        mbStatusDispSetAll(FALSE);
        mbCameraPlayerViewSet(visitWork->playerNo, 0);
        mbPlayerColSnapPlayerSet(visitWork->playerNo, FALSE);
        {
        OMOBJ *shopObj = ev_ShopOMObj[shopNo];
        MBSHOPOMWORK *shopWork = shopObj->data;

        if (shopWork->pathF) {
            /* Record each walked space so the return trip can follow the same path. */
            pathSpaceCount = 1;
            visitedSpaceIds[0] = currentMasuId = GwPlayer[visitWork->playerNo].masuId;
            {
                OMOBJ *shopObj;
                MBSHOPOMWORK *shopWork;

                while (currentMasuId != ev_ShopMasuEndGet(shopNo,
                    &shopObj, &shopWork)) {
                    mbMasuPosGet(
                        (masuId = mbMasuAttrFindLink((s16)currentMasuId,
                            SHOP_MASU_ATTR_PATH_LINK)), &eventPos);
                    GwPlayer[visitWork->playerNo].masuIdNext = masuId;
                    mbPlayerMasuMovePos(visitWork->playerNo, &eventPos, TRUE);
                    GwPlayer[visitWork->playerNo].masuId = masuId;
                    visitedSpaceIds[pathSpaceCount] = currentMasuId = masuId;
                    pathSpaceCount++;
                }
            }
        } else {
            mbPlayerMasuMovePos(visitWork->playerNo, &shopPos, TRUE);
        }
    }
    PSVECSubtract(&masuPos, &shopPos, &direction);
    mbPlayerRotateStart(visitWork->playerNo,
        (atan2(direction.x, direction.z) / M_PI) * 180.0, 15);
    while (!mbPlayerRotateCheck(visitWork->playerNo)) {
        HuPrcVSleep();
    }
        mbPlayerMotionShiftSet(visitWork->playerNo, 1, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);

    if (GwPlayer[visitWork->playerNo].comF) {
        /* Computer visitors buy the first affordable offer without opening the carousel. */
        for (loopIndex = 0; loopIndex < offerNum; loopIndex++) {
            if (offer[loopIndex].cost <= mbPlayerCoinGet(visitWork->playerNo)) {
                break;
            }
        }
        if (loopIndex < offerNum) {
            if (_CheckFlag(FLAG_BOARD_TUTORIAL)) {
                selectionResult = mbTutorialCall(SHOP_TUTORIAL_SELECT);

                if (selectionResult >= 0) {
                    /* The tutorial replaces this offer's capsule, price and text without checking
                     * the new price against the player's coins. */
                    offer[loopIndex].capsuleNo = selectionResult;
                    offer[loopIndex].cost = mbCapBuyCostGet((s16)selectionResult,
                        (s16)visitWork->playerNo);
                    offer[loopIndex].messageId = mbCapUseMesGet(selectionResult);
                }
            }
            selectionResult = loopIndex;
            coinAddResult = mbCoinAddProcExec(visitWork->playerNo,
                -offer[selectionResult].cost, -1, TRUE);
            mbCapCapsuleGet(visitWork->playerNo, offer[selectionResult].capsuleNo);
            /* A full inventory silently rejects this add; payment and the pickup animation have
             * already occurred. */
            mbPlayerCapsuleAdd(visitWork->playerNo, offer[selectionResult].capsuleNo);
            mbPlayerWinLoseVoicePlay(visitWork->playerNo, 12, CHARVOICEID(6));
            mbPlayerMotionShiftSet(visitWork->playerNo, 12, 0.0f, 4.0f, 0);
            mbWinCreate(2, ev_ShopMesGet(SHOP_MESSAGE_PURCHASED), -1);
            mbWinTopInsertMesSet(offer[selectionResult].messageId, 0);
            mbWinTopWait();
            while (!mbPlayerMotionEndCheck(visitWork->playerNo)) {
                HuPrcVSleep();
            }
            mbPlayerMotIdleSet(visitWork->playerNo);
            mbObjMotionShiftSet(shopModelId, 3, 0.0f, 8.0f,
                HU3D_MOTATTR_LOOP);
        }
    } else {
        while (!mbStatusOffCheckAll()) {
            HuPrcVSleep();
        }
        mbStatusDispFocusSet(visitWork->playerNo, TRUE);
        if (!GwSystem.curTime) {
            mbAudFXPlay(SHOP_SFX_DAY_PROMPT);
        } else {
            mbAudFXPlay(SHOP_SFX_NIGHT_PROMPT);
        }
        mbWinCreate(2, ev_ShopMesGet(SHOP_MESSAGE_GREETING), winType);
        mbWinTopWait();
        if (GwSystem.curTime && GwPlayer[visitWork->playerNo].rank >= 2) {
            if (!GwSystem.curTime) {
                mbAudFXPlay(SHOP_SFX_DAY_SUCCESS);
            } else {
                mbAudFXPlay(SHOP_SFX_NIGHT_SUCCESS);
            }
            mbWinCreate(2, ev_ShopMesGet(SHOP_MESSAGE_NIGHT_RESTRICTION),
                winType);
            mbWinTopWait();
        }
        selectionResult = -1;
        visitDoneF = FALSE;
        do {
            switch (offerNum) {
            case 1:
            case 2:
            case 3:
                selectionResult = ev_ShopSelect(visitWork, offer, offerNum, winType);
                break;
            default:
                if (!GwSystem.curTime) {
                    mbAudFXPlay(SHOP_SFX_DAY_UNAVAILABLE);
                } else {
                    mbAudFXPlay(SHOP_SFX_NIGHT_UNAVAILABLE);
                }
                mbWinCreate(2, ev_ShopMesGet(SHOP_MESSAGE_NO_OFFERS), winType);
                mbWinTopWait();
                visitDoneF = TRUE;
                selectionResult = -1;
                break;
            }

        if (selectionResult >= offerNum || selectionResult == -1) {
            visitDoneF = TRUE;
        } else {
            int discardedInventoryIndex; /* -1 means no discard; -2 means selection canceled. */

            discardedCapsuleNo = -1;
                discardedInventoryIndex = -1;
                if (mbPlayerCapsuleNumGet(visitWork->playerNo)
                    >= mbPlayerCapsuleMaxGet()) {
                    /* A full inventory requires permission and a capsule to discard. */
                    mbWinCreateChoice(2,
                        ev_ShopMesGet(SHOP_MESSAGE_DISCARD_CHOICE), winType, 0);
                    if (GwPlayer[visitWork->playerNo].comF) {
                        mbComChoiceLeftSet();
                    }
                    mbWinTopWait();
                    if (mbWinTopChoiceGet() != 0) {
                        continue;
                    }
                    do {
                        discardedCapsuleNo = mbCapDelete(-1, TRUE);
                        switch (discardedCapsuleNo) {
                            default:
                                /* If several slots contain the chosen capsule type, discard the
                                 * last of those slots. */
                                for (loopIndex = 0; loopIndex < mbPlayerCapsuleMaxGet();
                                     loopIndex++) {
                                    if (discardedCapsuleNo
                                        == mbPlayerCapsuleGet(visitWork->playerNo, loopIndex)) {
                                        discardedInventoryIndex = loopIndex;
                                    }
                                }
                                if (discardedInventoryIndex != -1) {
                                    mbPlayerCapsuleRemove(visitWork->playerNo,
                                        discardedInventoryIndex);
                                }
                                break;
                            case -3:
                                /* Resume the discard prompt after viewing the board. */
                                mbev_Scroll(visitWork->playerNo, FALSE);
                                discardedInventoryIndex = -1;
                                break;
                            case -7:
                                /* Canceling the discard returns to the shop offers. */
                                discardedInventoryIndex = -2;
                                break;
                            }
                    } while (discardedInventoryIndex == -1);
                    if (discardedInventoryIndex == -2) {
                        continue;
                    }
                }
                coinAddResult = mbCoinAddExec(visitWork->playerNo,
                    -offer[selectionResult].cost);
                mbCapCapsuleGet(visitWork->playerNo, offer[selectionResult].capsuleNo);
                mbPlayerCapsuleAdd(visitWork->playerNo, offer[selectionResult].capsuleNo);
                mbPlayerWinLoseVoicePlay(visitWork->playerNo, 12, CHARVOICEID(6));
                mbPlayerMotionShiftSet(visitWork->playerNo, 12, 0.0f, 4.0f, 0);
                if (discardedInventoryIndex == -1) {
                    mbWinCreate(2, ev_ShopMesGet(SHOP_MESSAGE_PURCHASED), -1);
                    mbWinTopInsertMesSet(offer[selectionResult].messageId, 0);
                } else {
                    mbWinCreate(2, ev_ShopMesGet(SHOP_MESSAGE_REPLACED), -1);
                    mbWinTopInsertMesSet(mbCapUseMesGet(discardedCapsuleNo), 0);
                    mbWinTopInsertMesSet(offer[selectionResult].messageId, 1);
                }
                mbWinTopWait();
                while (!mbPlayerMotionEndCheck(visitWork->playerNo)) {
                    HuPrcVSleep();
                }
                mbPlayerMotIdleSet(visitWork->playerNo);
                mbObjMotionShiftSet(shopModelId, 3, 0.0f, 8.0f,
                    HU3D_MOTATTR_LOOP);
                if (!GwSystem.curTime) {
                    mbAudFXPlay(SHOP_SFX_DAY_SUCCESS);
                } else {
                    mbAudFXPlay(SHOP_SFX_NIGHT_SUCCESS);
                }
                mbWinCreate(2, ev_ShopMesGet(SHOP_MESSAGE_THANK_YOU), winType);
                mbWinTopWait();
                visitDoneF = TRUE;
            }
        } while (!visitDoneF);
        mbStatusDispFocusSet(visitWork->playerNo, FALSE);
    }

    {
        OMOBJ *shopObj = ev_ShopOMObj[shopNo];
        MBSHOPOMWORK *shopWork = shopObj->data;

        shopPos = shopWork->shopPos;
    }
    mbMasuPosGet((s16)visitWork->shopNo, &playerPos);
    PSVECSubtract(&playerPos, &shopPos, &direction);
    /* The return angle is calculated but is not applied to the player. */
    facingAngle = (float)((atan2(direction.x, direction.z) / M_PI) * 180.0);
    {
        OMOBJ *shopObj = ev_ShopOMObj[shopNo];
        MBSHOPOMWORK *shopWork = shopObj->data;

        if (shopWork->pathF) {
            for (loopIndex = 1; loopIndex < pathSpaceCount; loopIndex++) {
                masuId = visitedSpaceIds[pathSpaceCount - (loopIndex + 1)];

                GwPlayer[visitWork->playerNo].masuIdNext = masuId;
                mbMasuPosGet(masuId, &shopPos);
                mbPlayerMasuMovePos(visitWork->playerNo, &shopPos, TRUE);
                GwPlayer[visitWork->playerNo].masuId = masuId;
            }
        } else {
            mbPlayerMasuMovePos(visitWork->playerNo, &playerPos, TRUE);
        }
        }
    }
    mbPlayerColSnapPlayerSet(visitWork->playerNo, TRUE);
    mbPlayerMotionShiftSet(visitWork->playerNo, 1, 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);
    while (!mbStatusOffCheckAll()) {
        HuPrcVSleep();
    }
    mbStatusDispSetAll(TRUE);
    {
        OMOBJ *shopObj = ev_ShopOMObj[shopNo];
        MBSHOPOMWORK *shopWork = shopObj->data;

        if (shopWork->openF) {
            ev_ShopOpenSet(shopNo, FALSE);
        }
    }
    {
        OMOBJ *shopObj;
        MBSHOPOMWORK *shopWork;

        while ((shopObj = ev_ShopOMObj[shopNo]),
            (shopWork = shopObj->data),
            shopWork->motionExecF != FALSE) {
            HuPrcVSleep();
        }
    }
    mbCameraPlayerViewSet(visitWork->playerNo, 2);

cleanup:
    /* Entry can be declined before any models exist, so release only valid handles. */
    mbPlayerColSnapPlayerSet(visitWork->playerNo, TRUE);
    if (shopModelId != -1) {
        if (lightId != -1) {
            Hu3DLLightKill(mbObjModelIDGet(shopModelId), lightId);
        }
        mbObjKill(shopModelId);
    }
    if (capsuleModelId != -1) {
        mbObjKill(capsuleModelId);
    }
    for (loopIndex = 0; loopIndex < offerNum; loopIndex++) {
        if (capsuleObjId[loopIndex] != -1) {
            mbCapObjColorKill(capsuleObjId[loopIndex]);
        }
    }
    HuDataDirClose(SHOP_DATA_NIGHT_MODEL);
}

/* Called during a shop visit to browse offers; returns the offer index or -1 on cancel. */
static int ev_ShopSelect(MBSHOPWORK *visitWork, SHOP_OFFER *offer, int offerNum,
    int winType)
{
    HuVecF capsuleWorldPos;
    HuVecF previousCursorPos;
    HuVecF selectedCursorPos;
    HuVecF screenPos;
    HuVecF descriptionScroll;
    HuVecF descriptionBasePos[3];
    HuVec2f descriptionWindowPos;
    ANIMDATA *panelAnim;
    int capsuleObjId[3];
    int descWinId[3];
    int digitSprId[3][4];
    int pulseFrame = 0;
    int previousOfferIndex = pulseFrame;
    int selectedOfferIndex = previousOfferIndex;
    int panelGrpId;
    int cursorSprId;
    int panelSprId;
    int heldButtons;
    int pressedButtons;
    int padNo;
    int loopIndex;
    int componentIndex;
    int helpWinId;
    BOOL selectionDoneF;
    float capsuleScale;
    float panelRotation;
    float scrollProgress;
    float previousWindowOffset;
    float selectedWindowOffset;

    descriptionScroll.x = descriptionScroll.y = descriptionScroll.z = 0.0f;

    panelGrpId = HuSprGrpCreate(1);
    HuSprGrpCenterSet(panelGrpId, ev_ShopWinPos.x, ev_ShopWinPos.y);
    HuSprGrpDrawNoSet(panelGrpId, SHOP_SELECT_DRAW_NO);
    panelSprId = HuSprCreate(
        panelAnim = HuSprAnimRead(HuDataSelHeapReadNum(SHOP_SELECT_PANEL_FILE,
            HU_MEMNUM_OVL, HEAP_MODEL)),
        SHOP_SELECT_PANEL_PRIORITY, 0);
    HuSprGrpMemberSet(panelGrpId, 0, panelSprId);
    HuSprDrawNoSet(panelGrpId, 0, SHOP_SELECT_DRAW_NO);
    HuSprAttrSet(panelGrpId, 0, SHOP_SELECT_SPR_ATTR);
    HuSpr3DSet(panelSprId);
    HuSpr3DRotSet(panelSprId, 90.0f, 0.0f, 0.0f);
    /* Fold the panel into view before showing its capsules and prices. */
    for (loopIndex = 1; loopIndex <= SHOP_SELECT_ROTATE_FRAMES; loopIndex++) {
        panelRotation = 90.0 - (9.0 * loopIndex);
        HuSpr3DRotSet(panelSprId, panelRotation, 0.0f, 0.0f);
        HuPrcVSleep();
    }
    HuSpr3DRotSet(panelSprId, 0.0f, 0.0f, 0.0f);

    cursorSprId = espEntry(SHOP_SELECT_CURSOR_FILE, SHOP_SELECT_ESP_PRIORITY,
        0);
    espDrawNoSet(cursorSprId, 0);
    espPosSet(cursorSprId,
        ev_ShopCapsulePos[offerNum - 1][selectedOfferIndex].x
            + SHOP_SELECT_CURSOR_OFFSET,
        ev_ShopCapsulePos[offerNum - 1][selectedOfferIndex].y
            + SHOP_SELECT_CURSOR_OFFSET);
    espAttrSet(cursorSprId, SHOP_SELECT_SPR_ATTR);
    espDispOn(cursorSprId);

    for (loopIndex = 0; loopIndex < offerNum; loopIndex++) {
        capsuleObjId[loopIndex] = mbCapObjCreate(offer[loopIndex].capsuleNo, FALSE);
        mbObjLayerSet(capsuleObjId[loopIndex], SHOP_SELECT_MODEL_LAYER);
        mbObjCameraSet(capsuleObjId[loopIndex], SHOP_SELECT_CAMERA);
        {
            MBMODELID modelId = capsuleObjId[loopIndex];
            mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
        }
        mbObjMotionSpeedSet(capsuleObjId[loopIndex], 0.0f);
        Hu3D2Dto3D(&ev_ShopCapsulePos[offerNum - 1][loopIndex],
            SHOP_SELECT_CAMERA, &capsuleWorldPos);
        mbObjPosSetV(capsuleObjId[loopIndex], &capsuleWorldPos);
        mbObjRotSet(capsuleObjId[loopIndex], 30.0f, 0.0f, 0.0f);
        mbObjScaleSet(capsuleObjId[loopIndex], 1.0f, 1.0f, 1.0f);
    }
    mbObjMotionSpeedSet(capsuleObjId[0], 1.0f);

    for (loopIndex = 0; loopIndex < offerNum; loopIndex++) {
        screenPos = ev_ShopCapsulePos[offerNum - 1][loopIndex];
        if (offer[loopIndex].cost >= 10) {
            screenPos.x -= 24.0f;
        } else {
            screenPos.x -= 18.0f;
        }
        for (componentIndex = 0; componentIndex < 4; componentIndex++) {
            digitSprId[loopIndex][componentIndex] = espEntry(
                mbBoardDataNumGet(ev_ShopSprFileTbl[componentIndex]),
                SHOP_SELECT_ESP_PRIORITY, 0);
            espDrawNoSet(digitSprId[loopIndex][componentIndex], 0);
            espAttrSet(digitSprId[loopIndex][componentIndex], SHOP_SELECT_SPR_ATTR);
            espPosSet(digitSprId[loopIndex][componentIndex], screenPos.x + (16 * componentIndex),
                screenPos.y + 32.0f);
        }
        espBankSet(digitSprId[loopIndex][1], 10);
        /* The price row keeps two digit slots, hiding the last for a one-digit cost. */
        if (offer[loopIndex].cost >= 10) {
            espBankSet(digitSprId[loopIndex][2], offer[loopIndex].cost / 10);
            espBankSet(digitSprId[loopIndex][3], offer[loopIndex].cost % 10);
        } else {
            espBankSet(digitSprId[loopIndex][2], offer[loopIndex].cost % 10);
            espDispOff(digitSprId[loopIndex][3]);
        }
    }

    for (loopIndex = 0; loopIndex < offerNum; loopIndex++) {
        descWinId[loopIndex] = mbCapDescWinCreate(offer[loopIndex].capsuleNo);
        mbWinPosGet(descWinId[loopIndex], &descriptionWindowPos);
        descriptionBasePos[loopIndex].x =
            (SHOP_SELECT_WINDOW_SPACING * loopIndex) + descriptionWindowPos.x;
        descriptionBasePos[loopIndex].y = descriptionWindowPos.y;
        descriptionBasePos[loopIndex].z = 0.0f;
        PSVECAdd(&descriptionBasePos[loopIndex], &descriptionScroll, &screenPos);
        mbWinPosSet(descWinId[loopIndex], screenPos.x, screenPos.y);
    }
    helpWinId = mbWinCreateHelp(MESSNUM(MESS_SHOP_EVENT, 29));
    mbWinAttrSet(+(s16)helpWinId, HUWIN_ATTR_ALIGN_CENTER);

    do {
        selectionDoneF = FALSE;
        padNo = GwPlayer[visitWork->playerNo].padNo;
        heldButtons = HuPadBtn[padNo];
        pressedButtons = HuPadBtnDown[padNo];
        if (mbPadStkXGet(padNo) < -SHOP_SELECT_STICK_THRESHOLD) {
            heldButtons |= PAD_BUTTON_LEFT;
        } else if (mbPadStkXGet(padNo) > SHOP_SELECT_STICK_THRESHOLD) {
            heldButtons |= PAD_BUTTON_RIGHT;
        }
        if (heldButtons & PAD_BUTTON_LEFT) {
            selectedOfferIndex--;
        } else if (heldButtons & PAD_BUTTON_RIGHT) {
            selectedOfferIndex++;
        }
        if (GwPlayer[visitWork->playerNo].comF) {
            /* Automated selection immediately confirms the currently displayed offer. */
            pressedButtons = PAD_BUTTON_A;
            selectedOfferIndex = previousOfferIndex;
        }
        if (selectedOfferIndex < 0) {
            selectedOfferIndex = 0;
        }
        if (selectedOfferIndex >= offerNum) {
            selectedOfferIndex = offerNum - 1;
        }
        if (selectedOfferIndex != previousOfferIndex) {
            /* Move the cursor and scroll all descriptions together over twenty frames. */
            previousCursorPos = ev_ShopCapsulePos[offerNum - 1][previousOfferIndex];
            selectedCursorPos = ev_ShopCapsulePos[offerNum - 1][selectedOfferIndex];
            previousWindowOffset = SHOP_SELECT_WINDOW_SPACING * -previousOfferIndex;
            selectedWindowOffset = SHOP_SELECT_WINDOW_SPACING * -selectedOfferIndex;
            mbObjMotionSpeedSet(capsuleObjId[previousOfferIndex], 0.0f);
            mbObjMotionTimeSet(capsuleObjId[previousOfferIndex], 0.0f);
            mbObjScaleSet(capsuleObjId[previousOfferIndex], 1.0f, 1.0f, 1.0f);
            mbAudFXPlay(MSM_SE_CMN_01);
            for (loopIndex = 1; loopIndex <= SHOP_SELECT_MOVE_FRAMES; loopIndex++) {
                scrollProgress = loopIndex / 20.0f;

                mbev_CapVecChase(sin((M_PI * (90.0f * scrollProgress)) / 180.0),
                    &previousCursorPos, &selectedCursorPos, &screenPos);
                espPosSet(cursorSprId,
                    screenPos.x + SHOP_SELECT_CURSOR_OFFSET,
                    screenPos.y + SHOP_SELECT_CURSOR_OFFSET);
                descriptionScroll.x = previousWindowOffset
                    + ((selectedWindowOffset - previousWindowOffset)
                        * sin((M_PI * (90.0f * scrollProgress)) / 180.0));
                for (componentIndex = 0; componentIndex < offerNum; componentIndex++) {
                    PSVECAdd(&descriptionBasePos[componentIndex], &descriptionScroll, &screenPos);
                    mbWinPosSet(descWinId[componentIndex], screenPos.x, screenPos.y);
                }
                HuPrcVSleep();
            }
            mbObjMotionSpeedSet(capsuleObjId[selectedOfferIndex], 1.0f);
            mbObjMotionTimeSet(capsuleObjId[selectedOfferIndex], 0.0f);
            mbObjScaleSet(capsuleObjId[selectedOfferIndex], 1.0f, 1.0f, 1.0f);
            previousOfferIndex = selectedOfferIndex;
            pulseFrame = 0;
        }
        capsuleScale = 1.0f + (0.2f * fabs(sin((M_PI
            * ((90.0f * pulseFrame) / 12.0f)) / 180.0)));
        /* Only the selected capsule plays its motion and pulses above normal size. */
        mbObjScaleSet(capsuleObjId[selectedOfferIndex], capsuleScale, capsuleScale, capsuleScale);
        pulseFrame++;
        if (pressedButtons & PAD_BUTTON_A) {
            mbAudFXPlay(MSM_SE_CMN_02);
            selectionDoneF = TRUE;
        } else if (pressedButtons & PAD_BUTTON_B) {
            mbAudFXPlay(MSM_SE_CMN_04);
            selectedOfferIndex = -1;
            selectionDoneF = TRUE;
        }
        HuPrcVSleep();
    } while (!selectionDoneF);

    espKill(cursorSprId);
    for (loopIndex = 0; loopIndex < offerNum; loopIndex++) {
        mbCapObjKill(capsuleObjId[loopIndex]);
    }
    for (loopIndex = 0; loopIndex < offerNum; loopIndex++) {
        for (componentIndex = 0; componentIndex < 4; componentIndex++) {
            espKill(digitSprId[loopIndex][componentIndex]);
        }
    }
    for (loopIndex = 0; loopIndex < offerNum; loopIndex++) {
        mbWinKill(descWinId[loopIndex]);
    }
    mbWinKill(helpWinId);
    for (loopIndex = 1; loopIndex <= SHOP_SELECT_ROTATE_FRAMES; loopIndex++) {
        panelRotation = 9.0 * loopIndex;
        HuSpr3DRotSet(panelSprId, panelRotation, 0.0f, 0.0f);
        HuPrcVSleep();
    }
    HuSprGrpMemberKill(panelGrpId, 0);
    HuSprGrpKill(panelGrpId);
    return selectedOfferIndex;
}

/* Called when shop dialogue opens; selects the message bank's day or night text. */
static int ev_ShopMesGet(int dayMessageId)
{
    if (!GwSystem.curTime) {
        return dayMessageId;
    }
    return dayMessageId + 14;
}
