/* Faire Square: the board's day/night setup, its coin machine, star wagers, lottery, transport and
 * effects. */
#include "math.h"
#include "game/gamework.h"
#include "game/audio.h"
#include "msm_se.h"
#include "game/board/main.h"
#include "game/board/branch.h"
#include "game/board/masu.h"
#include "game/board/object.h"
#include "game/board/opening.h"
#include "game/board/player.h"
#include "game/data.h"
#include "game/esprite.h"
#include "game/memory.h"
#include "game/object.h"
#include "game/sprite.h"
#include "dolphin/gx.h"
#include "game/main.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include "messdir_enum.h"
#include <string.h>
#include "game/process.h"
#include "game/board/window.h"
#include "dolphin/types.h"
#include "game/hu3d.h"
#include "game/board/coin.h"
#include "game/board/object_data.h"
#include "game/board/audio.h"
#include "game/board/camera.h"
#include "dolphin/mtx.h"
#include "game/board/comchoice.h"
#include "game/board/effect.h"
#include "game/board/status.h"
#include "datanum/effect.h"
#include "datanum/charmot.h"

#define MSM_SE_W03_SLOT_REEL_SPIN 1475
#define MSM_SE_W03_SLOT_REEL_STOP 1476
#define MSM_SE_W03_SLOT_MACHINE_WAIT 1477
#define MSM_SE_W03_SLOT_SYMBOL_STOP 1478
#define MSM_SE_W03_LOTTERY_OBJECT_MOVE 1479
#define MSM_SE_W03_SLOT_PRIZE_REVEAL 1480
#define MSM_SE_W03_SLOT_COIN_PAYOUT 1481
#define MSM_SE_W03_SLOT_JACKPOT 1482
#define MSM_SE_W03_LAUNCH_ENGINE 1483
#define MSM_SE_W03_MOVE_START 1471
#define MSM_SE_W03_MOVE_STEP 1472
#define MSM_SE_W03_TRANSPORT_START 1473
#define MSM_SE_W03_TRANSPORT_DEPART 1474
#define MSM_SE_W03_TRANSPORT_PASS 1487
#define MSM_SE_W03_TRANSPORT_ARRIVE 1488
#define MSM_SE_W03_LOTTERY_OBJECT_LAUNCH 1490
#define MSM_SE_W03_STAR_REWARD 1491
#define W03_MATTR_TRANSPORT_EXIT 64
#define W03_OFFSET_FINAL_MODEL_5 152
#define W03_OFFSET_MOTION_MODEL 156

typedef void (*VoidFunc)(void);
typedef void (*MBHook)(void);
typedef struct W03SavedState_s {
    u8 nightEventCount;
    u8 otherState;
    u16 displayValue;
    u8 wheelRotations[3];
    u8 nightReward;
} W03SAVEDSTATE;
typedef struct W03Objects_s {
    int initialModels[6];
    int totalSprite;
    int playerSprites[4];
    int playerDigits[4][3];
    int lateModel;
    int carouselModels[6];
    int lotteryModels[4];
    int finalModels[6];
    int motionModel;
    int diceModel;
    int modelState;
    ANIMDATA *digitAnimations[10];
    s16 digitAnimationIds[3];
    s16 otherState;
    int largeDisplaySprites[2];
    int largeDisplayDigits[3];
} W03OBJECTS;
typedef struct W03CoinEffect_s {
    s32 modelId;
    HuVecF position;
    HuVecF rotation;
    HuVecF velocity;
    s16 alpha;
    u8 otherState[2];
    s32 work[6];
} W03COINEFFECT;
typedef struct W03Orbit_s {
    HuVecF position;
    f32 angle;
} W03ORBIT;
#define boardDataNum(fileNo) DATANUM((boardIsDay() ? DATA_w03 : DATA_w03n), FILENUM(fileNo))
typedef struct W03StarAward_s {
    int objectId;
    HuVecF position;
    HuVecF rotation;
    HuVecF scale;
    int active;
    int frame;
    int delay;
    int phase;
    int soundId;
} W03STARAWARD;

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void mbObjectSetup(s32 boardNo, MBHook init, MBHook close);
extern BOOL mbSaveNewF;
void mbev_ShopInit(int dataNum);
void mbScrollInit(int dataNum);
int mbCapThrowColCreate(int dataNum);
void mbMapCameraSet(const HuVecF *rot, const HuVecF *pos, float zoom);
OMOBJ *mbGuideCreateFlag(HuVecF *pos, s8 *motTbl, BOOL screenF, BOOL altMtxF, BOOL layerF);
int mbGuideModelGet(OMOBJ *obj);
void mbGuideMotionNextSet(OMOBJ *obj, s16 motNo);
void mbGuideMotionSet(OMOBJ *obj, s16 motNo, BOOL shiftF);
void mbGuideMotionShiftSet(OMOBJ *obj, s16 motNo, BOOL shiftF);
void mbGuideEnd(OMOBJ *obj, BOOL endF);
void mbGuideKill(OMOBJ *obj);
void mbTelopTimeChangeCreate(void);
BOOL mbTelopTimeChangeCheck(void);
int mbDiceProcExec(int playerNo, int diceType, s8 *valueTbl, int *tutorialVal,
    BOOL padWinF, BOOL waitF, HuVecF *pos, int color);
BOOL mbDiceKillCheck(int playerNo);
int mbDiceResultGet(int playerNo);
void mbDiceNumKill(int playerNo);
MBMODELID mbObjCreate(int dataNum, const int *motionData, BOOL link);
void mbObjKill(MBMODELID modelId);
BOOL mbObjDispGet(MBMODELID modelId);
void mbObjDispSet(MBMODELID modelId, BOOL visible);
void mbObjPosSetV(MBMODELID modelId, const HuVecF *position);
void mbObjPosGet(MBMODELID modelId, HuVecF *position);
void mbObjRotSetV(MBMODELID modelId, const HuVecF *rotation);
void mbObjScaleSet(MBMODELID modelId, float x, float y, float z);
void mbObjAlphaSet(MBMODELID modelId, u8 alpha);
extern f32 mbCosDeg(f32 degrees);
extern f32 mbSinDeg(f32 degrees);
extern u32 mbRandMod(u32 limit);
extern u32 frand(void);
extern f32 frandf(void);
void mbDiceObjHit(s32 playerNo);
extern s16 Hu3DAnimCreate(void *dataP, s16 modelId, char *bitmapName);
extern void Hu3DAnimKill(s16 animId);
int sprintf();
s16 mbCoinDispCapsuleCreate(HuVecF *position, int coinCount);
int mbStarObjCreate(void);
void mbStarObjKill(int objectId);
void mbStarObjPosSetV(int objectId, const HuVecF *position);
void mbStarObjRotSetV(int objectId, const HuVecF *rotation);
void mbStarObjScaleSetV(int objectId, const HuVecF *scale);
void mbStarObjDispSet(int objectId, BOOL visible);
void mbStarObjDispFlagSet(int objectId, BOOL visible);
int mbStarAddExec(int playerNo, int starCount);
int mbStarDispPlayerCreate(int playerNo, int starCount);
BOOL mbStarDispCheck(int displayId);
s8 mbPadStkXGet(s32 padNo);
s8 mbPadStkYGet(s32 padNo);
void mbWipeFadeIn(void);
void mbWipeFadeOut(void);
s32 mbBGRead(s32 boardDataNum);
void mbBGReadWait(s32 readId);
void mbCoinAddAllExec(int player0, int player1, int player2, int player3);
void mbStarAddAllExec(int player0, int player1, int player2, int player3);
void mbev_PlayerColMasu(int playerNo, int spaceId, BOOL active);
void mbWipeSpecialFadeInCreate(int type, int time);
void mbWipeSpecialFadeOutCreate(int type, int time);
int abs(int value);
int mbCoinAddExec(int playerNo, int coinCount);
void mbStarObjPosSet(int objectId, float x, float y, float z);
int mbAudFXPlay(s16 seId);

void fn_1_A0(void);
void fn_1_F4(void);
void fn_1_1790(void);
void fn_1_1BB4(OMOBJ *object);
void fn_1_1BB8(void);
void fn_1_1C2C(void);
int fn_1_1C30(int playerNo, s16 spaceId);
int fn_1_1CCC(int playerNo, s16 spaceId);
int fn_1_1CD4(int playerNo, s16 spaceId);
int fn_1_1D5C(s16 spaceId, u32 attributes, s16 *links, BOOL end);
void fn_1_1D64(void);
void fn_1_1F20(void);
s32 fn_1_2490(void);
void fn_1_24C4(void);
void fn_1_2718(int playerNo, s16 landingSpace);
void fn_1_5900(int playerNo, s16 landingSpace);
void fn_1_8154(s32 reverse);
void fn_1_8758(void);
void fn_1_8F60(int playerNo, s16 landingSpace);
void fn_1_B148(s32 playerNo, s32 coinCount, HuVecF *awardPosition);
void fn_1_B770(s32 playerNo, s32 starCount, HuVecF *awardPosition);
void fn_1_BD88(s32 playerNo, s16 landingSpace);
void fn_1_C59C(s32 playerNo, s16 entranceSpace);
HSF_OBJECT *fn_1_D150(HU3D_MODEL *model, char *name);
static HSF_OBJECT *fn_1_D198(HU3D_MODEL *model, char *name, HSF_OBJECT *object);
void fn_1_D9E0(s32 motionObjectId, s32 movingObjectId, f32 yaw, f32 turnAngle,
    HuVecF *startPosition, HuVecF *endPosition, s32 turnFirst);
void fn_1_DE88(s32 value);
void fn_1_E00C(int coinCount);
void fn_1_F704(HU3D_MODEL *model, MBPARTICLE *system, Mtx matrix);
void fn_1_FCB4(HU3D_MODEL *model, MBPARTICLE *system, Mtx matrix);
void fn_1_104FC(HU3D_MODEL *model, MBPARTICLE *system, Mtx matrix);
void fn_1_1097C(int playerNo);
void fn_1_109C8(void);
s16 fn_1_12220(int playerNo);
int fn_1_12268(int playerNo, int masuCount, s16 *masuIds, BOOL chooseNearest);
void fn_1_124FC(void);
void fn_1_126C0(void);

static inline BOOL boardIsDay(void)
{
    return 0 == GwSystem.curTime;
}

static inline void boardModelLoop(s16 modelId)
{
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
}

HuVecF lbl_1_data_0[100] = {
    {-2611.752f, 1165.225f, -680.96f},
    {-2488.202f, 1164.682f, -609.04f},
    {-2362.564f, 1164.357f, -540.87f},
    {-2232.383f, 1164.563f, -482.03f},
    {-2094.719f, 1165.941f, -445.09f},
    {-1956.348f, 1170.094f, -467.73f},
    {-1860.058f, 1178.418f, -569.79f},
    {-1815.621f, 1191.378f, -704.18f},
    {-1816.306f, 1210.451f, -845.25f},
    {-1859.177f, 1237.092f, -978.12f},
    {-1942.702f, 1270.112f, -1088.49f},
    {-2054.413f, 1303.983f, -1170.26f},
    {-2182.64f, 1334.675f, -1224.57f},
    {-2320.198f, 1359.67f, -1252.87f},
    {-2461.747f, 1377.387f, -1256.9f},
    {-2603.14f, 1387.891f, -1239.75f},
    {-2741.254f, 1391.559f, -1203.43f},
    {-2873.158f, 1388.424f, -1148.8f},
    {-2994.777f, 1377.804f, -1074.93f},
    {-3098.276f, 1358.016f, -978.99f},
    {-3172.593f, 1328.743f, -861.06f},
    {-3208.795f, 1293.68f, -727.85f},
    {-3205.778f, 1258.717f, -589.86f},
    {-3161.407f, 1230.325f, -457.65f},
    {-3080.827f, 1211.737f, -341.7f},
    {-2971.709f, 1198.429f, -251.13f},
    {-2841.852f, 1183.876f, -194.71f},
    {-2702.092f, 1160.64f, -188.28f},
    {-2570.74f, 1127.954f, -232.13f},
    {-2453.157f, 1093.22f, -305.38f},
    {-2343.25f, 1060.391f, -390.65f},
    {-2236.215f, 1030.774f, -480.66f},
    {-2128.691f, 1005.199f, -571.33f},
    {-2018.159f, 982.591f, -659.1f},
    {-1903.01f, 961.062f, -740.98f},
    {-1780.701f, 938.915f, -811.45f},
    {-1651.171f, 914.74f, -866.33f},
    {-1514.956f, 887.374f, -899.01f},
    {-1375.86f, 856.149f, -901.5f},
    {-1238.621f, 823.322f, -879.79f},
    {-1104.651f, 792.482f, -840.72f},
    {-972.028f, 765.677f, -794.64f},
    {-838.721f, 746.526f, -746.79f},
    {-702.972f, 737.042f, -703.25f},
    {-563.626f, 744.003f, -673.64f},
    {-423.932f, 770.968f, -666.29f},
    {-291.054f, 819.22f, -685.78f},
    {-167.285f, 880.286f, -722.26f},
    {-51.874f, 949.813f, -769.87f},
    {57.197f, 1024.872f, -823.73f},
    {163.428f, 1102.997f, -878.93f},
    {270.876f, 1182.561f, -929.45f},
    {380.585f, 1265.179f, -968.87f},
    {481.872f, 1362.833f, -991.37f},
    {539.057f, 1491.908f, -997.65f},
    {537.104f, 1634.117f, -999.47f},
    {492.144f, 1769.188f, -999.97f},
    {402.304f, 1878.909f, -1e+03f},
    {273.767f, 1938.315f, -1e+03f},
    {132.09f, 1936.816f, -1e+03f},
    {3.76f, 1876.407f, -1e+03f},
    {-92.846f, 1772.277f, -1e+03f},
    {-142.237f, 1639.365f, -999.99f},
    {-130.043f, 1498.179f, -999.54f},
    {-59.532f, 1375.012f, -997.78f},
    {49.603f, 1283.706f, -993.99f},
    {176.518f, 1219.525f, -982.27f},
    {309.134f, 1173.352f, -956.4f},
    {440.75f, 1141.544f, -911.16f},
    {565.753f, 1121.076f, -845.35f},
    {682.286f, 1108.561f, -763.68f},
    {791.715f, 1100.399f, -672.12f},
    {895.84f, 1093.807f, -574.41f},
    {996.305f, 1086.469f, -472.98f},
    {1094.33f, 1076.238f, -369.44f},
    {1188.805f, 1061.964f, -263.13f},
    {1276.498f, 1043.412f, -151.82f},
    {1353.499f, 1020.554f, -33.67f},
    {1415.657f, 993.727f, 92.08f},
    {1459.845f, 963.711f, 224.52f},
    {1486.572f, 931.277f, 361.06f},
    {1498.218f, 896.861f, 499.27f},
    {1496.667f, 860.689f, 637.52f},
    {1484.601f, 823.03f, 774.85f},
    {1463.544f, 783.993f, 910.73f},
    {1433.132f, 743.625f, 1044.42f},
    {1393.785f, 702.806f, 1175.6f},
    {1342.479f, 662.421f, 1302.71f},
    {1279.425f, 623.363f, 1424.81f},
    {1202.087f, 586.918f, 1539.26f},
    {1112.271f, 553.699f, 1645.25f},
    {1008.264f, 525.388f, 1738.98f},
    {894.666f, 501.322f, 1822.2f},
    {774.073f, 480.944f, 1896.13f},
    {648.299f, 463.776f, 1961.84f},
    {519.569f, 448.707f, 2022.12f},
    {388.807f, 435.219f, 2078.27f},
    {256.692f, 422.865f, 2131.46f},
    {123.745f, 411.246f, 2182.72f},
    {-9.61f, 4e+02f, 2233.0f}
};

/* Enables board mode and registers the board's load and close callbacks. */

void fn_1_A0(void)
{
    GWPartySet(TRUE);
    mbObjectSetup(2, fn_1_F4, fn_1_1790);
}

/* Board model IDs and saved state used by setup and event callbacks. */

s16 lbl_1_bss_5C4;

W03OBJECTS lbl_1_bss_4D4;

W03SAVEDSTATE *lbl_1_bss_4D0;

s32 lbl_1_bss_4C8[2];

W03ORBIT lbl_1_bss_468[6];

s32 lbl_1_bss_450[6];

W03COINEFFECT lbl_1_bss_10[16];

s32 lbl_1_bss_C;

s32 lbl_1_bss_8;

s32 lbl_1_bss_4;

OMOBJ *lbl_1_bss_0;

void mbev_NextTimeSet(void (*hook)(void));

void mbLightFuncSet(void (*setHook)(void), void (*resetHook)(void));

void mbStarMasuFuncSet(void (*hook)(void));

void mbStarMoveHookSet(void (*hook)(void));

void mbScrollStarFindFuncSet(s16 (*hook)(int playerNo));

/* Day and night Faire Square use the same board-file number layout. */

/* HSF node names for the coin machine's hundreds, tens and ones digits. */
char lbl_1_data_4B0[3][8] = {"keta100", "keta10", "keta1"};

HuVecF lbl_1_data_4C8 = {-4165.0f, 772.0f, 283.0f};

HuVecF lbl_1_data_4D4[4] = { { -575.0f, 1500.0f, -2250.0f },
                             { -195.0f, 1500.0f, -2250.0f },
                             { 195.0f, 1500.0f, -2250.0f },
                             { 575.0f, 1500.0f, -2250.0f } };

int lbl_1_data_504[10] = {
    DATANUM(DATA_w03, 9), DATANUM(DATA_w03, 10), DATANUM(DATA_w03, 11), DATANUM(DATA_w03, 12),
    DATANUM(DATA_w03, 13), DATANUM(DATA_w03, 14), DATANUM(DATA_w03, 15), DATANUM(DATA_w03, 16),
    DATANUM(DATA_w03, 17), DATANUM(DATA_w03, 18)
};

int lbl_1_data_52C[10] = {
    DATANUM(DATA_w03n, 9), DATANUM(DATA_w03n, 10), DATANUM(DATA_w03n, 11), DATANUM(DATA_w03n, 12),
    DATANUM(DATA_w03n, 13), DATANUM(DATA_w03n, 14), DATANUM(DATA_w03n, 15), DATANUM(DATA_w03n, 16),
    DATANUM(DATA_w03n, 17), DATANUM(DATA_w03n, 18)
};

int lbl_1_data_554[5] = {
    DATANUM(DATA_w03, 39), DATANUM(DATA_w03, 40), DATANUM(DATA_w03, 41), DATANUM(DATA_w03, 42), -1
};

int lbl_1_data_568[5] = {
    DATANUM(DATA_w03n, 39), DATANUM(DATA_w03n, 40), DATANUM(DATA_w03n, 41), DATANUM(DATA_w03n, 42),
    -1
};

float lbl_1_data_57C[4] = {45.0f, 77.0f, 102.0f, 127.0f};

float lbl_1_data_58C[4] = {270.0f, 313.0f, 356.0f, 399.0f};

float lbl_1_data_59C[3] = {283.0f, 293.0f, 330.0f};

int lbl_1_data_5A8[3] = {10, 0, 1};

char lbl_1_data_5B4[3][8] = {"r1", "r2", "r3"};

int lbl_1_data_5CC[4] = {0, 3, 2, 1};

HuVecF lbl_1_data_5DC = {-28.5000000f, 0.0f, 0.0f};

HuVecF lbl_1_data_5E8 = {0.0f, -112.0f, 547.0f};

float lbl_1_data_5F4 = 14000.0f;

HuVecF lbl_1_data_5F8 = {318.0f, 0.0f, 0.0f};

HuVecF lbl_1_data_604 = {-70.5000000f, -400.0f, 1458.0f};

float lbl_1_data_610[4] = {14299.0f, 0.0f, 20.0f, 0.0f};

/* Creates the board models, displays and event hooks during board loading. */
void fn_1_F4(void)
{
    HuVecF position;
    int boardNo;
    s32 motionSpace;
    s32 modelId;
    HU3D_MODEL *wheelModel;
    HSF_OBJECT *wheelPart;
    int index;
    int digitIndex;
    int objectId;

    boardNo = MBBoardNoGet();
    HuAudSndGrpSetSet(24);
    lbl_1_bss_4D0 = (W03SAVEDSTATE *)&GwSystem.boardWork;
    if (mbSaveNewF) {
        lbl_1_bss_4D0->nightEventCount = 0;
        lbl_1_bss_4D0->displayValue = 0;
        lbl_1_bss_4D0->nightReward = 20;
        for (index = 0; index < 3; index++) {
            lbl_1_bss_4D0->wheelRotations[index] = 0;
        }
    }
    mbMasuInit(boardDataNum(0));
    lbl_1_bss_5C4 = mbObjCreate(boardDataNum(1), NULL, FALSE);
    mbObjPosSet(lbl_1_bss_5C4, 0.0f, 0.0f, 0.0f);
    boardModelLoop(lbl_1_bss_5C4);
    mbObjCullRadiusSet(lbl_1_bss_5C4, -1.0f);
    mbObjMotionSpeedSet(lbl_1_bss_5C4, 1.0f);
    mbObjMotionTimeSet(lbl_1_bss_5C4, 0.0f);
    boardModelLoop(lbl_1_bss_5C4);
    mbev_ShopInit(boardDataNum(2));
    mbev_NextTimeSet(fn_1_1D64);
    mbScrollInit(boardDataNum(36));
    mbLightFuncSet(fn_1_1BB8, fn_1_1C2C);
    lbl_1_bss_4D4.initialModels[0] = objectId = mbObjCreate(boardDataNum(3), NULL, FALSE);
    mbObjMotionSpeedSet(objectId, 0.0f);
    mbObjPosSetV(objectId, &lbl_1_data_4C8);
    mbObjRotSet(objectId, 0.0f, 45.0f, 0.0f);
    lbl_1_bss_4D4.initialModels[1] = objectId = mbObjCreate(boardDataNum(4), NULL, FALSE);
    mbObjMotionSpeedSet(objectId, 0.0f);
    lbl_1_bss_4D4.initialModels[2] = objectId = mbObjCreate(boardDataNum(5), NULL, FALSE);
    lbl_1_bss_4D4.initialModels[3] = objectId = mbObjCreate(boardDataNum(6), NULL, FALSE);
    mbObjMotionSpeedSet(objectId, 0.0f);
    lbl_1_bss_4D4.initialModels[4] = objectId = mbObjCreate(boardDataNum(7), NULL, FALSE);
    lbl_1_bss_4D4.initialModels[5] = objectId = mbObjCreate(boardDataNum(8), NULL, FALSE);
    mbObjMotionSpeedSet(objectId, 0.0f);
    mbObjDispSet(objectId, FALSE);
    lbl_1_bss_4D4.lateModel = objectId = mbObjCreate(boardDataNum(23), NULL, FALSE);
    mbObjMotionSpeedSet(objectId, 0.0f);
    for (index = 0; index < 6; index++) {
        if (index == 0) {
            lbl_1_bss_4D4.carouselModels[index] = objectId =
                mbObjCreate(boardDataNum(24), NULL, FALSE);
        } else {
            lbl_1_bss_4D4.carouselModels[index] = objectId =
                mbObjCreate(boardDataNum(24), NULL, TRUE);
        }
        mbObjMotionSpeedSet(objectId, 0.0f);
        mbObjDispSet(objectId, FALSE);
    }
    for (index = 0; index < 4; index++) {
        if (index == 0) {
            lbl_1_bss_4D4.lotteryModels[index] = objectId =
                mbObjCreate(boardDataNum(25), NULL, FALSE);
        } else {
            lbl_1_bss_4D4.lotteryModels[index] = objectId =
                mbObjCreate(boardDataNum(25), NULL, TRUE);
        }
        mbObjMotionSpeedSet(objectId, 0.0f);
        mbObjPosSetV(objectId, &lbl_1_data_4D4[index]);
        mbObjScaleSet(objectId, 0.15f, 0.15f, 0.15f);
    }
    lbl_1_bss_4D4.finalModels[0] = objectId = mbObjCreate(boardDataNum(26), NULL, FALSE);
    mbObjMotionSpeedSet(objectId, 0.0f);
    lbl_1_bss_4D4.finalModels[1] = objectId = mbObjCreate(boardDataNum(27), NULL, FALSE);
    mbObjMotionSpeedSet(objectId, 0.0f);
    lbl_1_bss_4D4.finalModels[2] = objectId = mbObjCreate(boardDataNum(28), NULL, FALSE);
    mbObjDispSet(objectId, TRUE);
    mbObjMotionSpeedSet(objectId, 0.0f);
    mbObjMotionTimeSet(objectId, 0.0f);
    lbl_1_bss_4D4.finalModels[3] = objectId = mbObjCreate(boardDataNum(29), NULL, FALSE);
    mbObjMotionSpeedSet(objectId, 0.0f);
    lbl_1_bss_4D4.finalModels[4] = objectId = mbObjCreate(boardDataNum(30), NULL, FALSE);
    mbObjMotionSpeedSet(objectId, 0.0f);
    mbObjDispSet(objectId, FALSE);
    lbl_1_bss_4D4.finalModels[5] = objectId = mbObjCreate(boardDataNum(31), NULL, FALSE);
    mbObjDispSet(objectId, TRUE);
    if (boardIsDay()) {
        lbl_1_bss_4D4.motionModel = objectId =
            mbObjCreate(DATANUM(DATA_w03, 38), lbl_1_data_554, FALSE);
    } else {
        lbl_1_bss_4D4.motionModel = objectId =
            mbObjCreate(DATANUM(DATA_w03n, 38), lbl_1_data_568, FALSE);
    }
    motionSpace = mbMasuFind_MAttrIdGet(-1, (1 << 11));
    mbMasuPosGet(motionSpace, &position);
    position.y += 200.0f;
    position.z -= 200.0f;
    mbObjDispSet((s16)lbl_1_bss_4D4.motionModel, FALSE);
    if (boardIsDay()) {
        for (index = 0; index < 10; index++) {
            lbl_1_bss_4D4.digitAnimations[index] = HuSprAnimRead(
                HuDataSelHeapReadNum(lbl_1_data_504[index], HU_MEMNUM_OVL, HEAP_MODEL));
        }
    } else {
        for (index = 0; index < 10; index++) {
            lbl_1_bss_4D4.digitAnimations[index] = HuSprAnimRead(
                HuDataSelHeapReadNum(lbl_1_data_52C[index], HU_MEMNUM_OVL, HEAP_MODEL));
        }
    }
    lbl_1_bss_4D4.largeDisplaySprites[0] = objectId = espEntry(boardDataNum(33), 120, 0);
    espDrawNoSet(objectId, 32);
    espPosSet(objectId, 288.0f, 224.0f);
    espTPLvlSet(objectId, 0.8f);
    espDispOff(objectId);
    lbl_1_bss_4D4.largeDisplaySprites[1] = objectId = espEntry(boardDataNum(35), 110, 0);
    espDrawNoSet(objectId, 32);
    espPosSet(objectId, 248.0f, 222.0f);
    espDispOff(objectId);
    for (index = 0; index < 3; index++) {
        lbl_1_bss_4D4.largeDisplayDigits[index] = objectId = espEntry(boardDataNum(34), 100, 0);
        espDrawNoSet(objectId, 32);
        espPosSet(objectId, lbl_1_data_59C[index], 224.0f);
        espBankSet(objectId, lbl_1_data_5A8[index]);
        espDispOff(objectId);
    }
    lbl_1_bss_4D4.totalSprite = objectId = espEntry(boardDataNum(33), 120, 0);
    espDrawNoSet(objectId, 32);
    espPosSet(objectId, 85.0f, 335.0f);
    espTPLvlSet(objectId, 0.8f);
    espDispOff(objectId);
    for (index = 0; index < 4; index++) {
        digitIndex = lbl_1_data_5CC[index];
        lbl_1_bss_4D4.playerSprites[index] = objectId =
            espEntry(boardDataNum(DATA_w03 + 19 + digitIndex), 110, 0);
        espDrawNoSet(objectId, 32);
        espPosSet(objectId, lbl_1_data_57C[0], lbl_1_data_58C[index]);
        espDispOff(objectId);
    }
    for (index = 0; index < 4; index++) {
        for (digitIndex = 0; digitIndex < 3; digitIndex++) {
            lbl_1_bss_4D4.playerDigits[index][digitIndex] = objectId =
                espEntry(boardDataNum(34), 100, 0);
            espBankSet(objectId, lbl_1_data_5A8[digitIndex]);
            espDrawNoSet(objectId, 32);
            espPosSet(objectId, lbl_1_data_57C[digitIndex + 1], lbl_1_data_58C[index]);
            espDispOff(objectId);
        }
    }
    for (index = 0; index < 3; index++) {
        lbl_1_bss_4D4.digitAnimationIds[index] = Hu3DAnimCreate(
            lbl_1_bss_4D4.digitAnimations[0], mbObjModelIDGet((s16)lbl_1_bss_4D4.initialModels[2]),
            lbl_1_data_4B0[index]);
    }
    for (index = 0; index < 16; index++) {
        lbl_1_bss_10[index].modelId = -1;
        lbl_1_bss_10[index].position.x = 0.0f;
        lbl_1_bss_10[index].position.y = 0.0f;
        lbl_1_bss_10[index].position.y = 0.0f;
        lbl_1_bss_10[index].rotation.x = 0.0f;
        lbl_1_bss_10[index].rotation.y = 0.0f;
        lbl_1_bss_10[index].rotation.y = 0.0f;
        lbl_1_bss_10[index].velocity.x = 0.0f;
        lbl_1_bss_10[index].velocity.y = 0.0f;
        lbl_1_bss_10[index].velocity.y = 0.0f;
        lbl_1_bss_10[index].alpha = 0;
        lbl_1_bss_10[index].work[0] = 0;
        lbl_1_bss_10[index].work[1] = 0;
        lbl_1_bss_10[index].work[2] = 0;
        lbl_1_bss_10[index].work[3] = 0;
        lbl_1_bss_10[index].work[4] = 0;
        lbl_1_bss_10[index].work[5] = 0;
    }
    modelId = mbObjModelIDGet((s16)lbl_1_bss_4D4.initialModels[0]);
    wheelModel = &Hu3DData[modelId];
    for (index = 0; index < 3; index++) {
        wheelPart = fn_1_D150(wheelModel, lbl_1_data_5B4[index]);
        wheelPart->mesh.base.rot.x = 45.0f * lbl_1_bss_4D0->wheelRotations[index];
    }
    fn_1_DE88(lbl_1_bss_4D0->displayValue);
    mbCapThrowColCreate(boardDataNum(37));
    HuDataDirClose(boardDataNum(0));
    mbev_MasuMoveStartSet(fn_1_1CCC);
    mbev_MasuMoveEndSet(fn_1_1CD4);
    mbev_MasuHatenaSet(fn_1_1C30);
    mbev_MasuLinkTblHookSet(fn_1_1D5C);
    lbl_1_bss_0 = omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_1BB4);
    mbOpeningViewSet(&lbl_1_data_5DC, &lbl_1_data_5E8, lbl_1_data_5F4);
    mbMapCameraSet(&lbl_1_data_5F8, &lbl_1_data_604, lbl_1_data_610[0]);
    mbPlayerTurnInitHookSet(fn_1_1097C);
    mbBranchMAttrSet(1 << 11);
    mbBranchMAttrSet(1 << 12);
    mbStarMasuFuncSet(fn_1_109C8);
    mbStarMoveHookSet(NULL);
    mbScrollStarFindFuncSet(fn_1_12220);
    mbBranchComStarHookSet(fn_1_12268);
    mbOpeningInstHookSet(fn_1_124FC);
    mbOpeningStarInstHookSet(fn_1_126C0);
}

/* Releases the board models, digit animations and sprites when the overlay closes. */
void fn_1_1790(void)
{
    s32 i;
    s32 j;

    if (lbl_1_bss_5C4 >= 0) {
        mbObjKill(lbl_1_bss_5C4);
        lbl_1_bss_5C4 = -1;
    }
    lbl_1_bss_0 = 0;

    if (lbl_1_bss_4D4.initialModels[0] != -1) {
        mbObjKill((s16)lbl_1_bss_4D4.initialModels[0]);
    }
    if (lbl_1_bss_4D4.initialModels[1] != -1) {
        mbObjKill((s16)lbl_1_bss_4D4.initialModels[1]);
    }
    if (lbl_1_bss_4D4.initialModels[2] != -1) {
        mbObjKill((s16)lbl_1_bss_4D4.initialModels[2]);
    }
    if (lbl_1_bss_4D4.initialModels[3] != -1) {
        mbObjKill((s16)lbl_1_bss_4D4.initialModels[3]);
    }
    if (lbl_1_bss_4D4.initialModels[4] != -1) {
        mbObjKill((s16)lbl_1_bss_4D4.initialModels[4]);
    }
    if (lbl_1_bss_4D4.initialModels[5] != -1) {
        mbObjKill((s16)lbl_1_bss_4D4.initialModels[5]);
    }
    if (lbl_1_bss_4D4.lateModel != -1) {
        mbObjKill((s16)lbl_1_bss_4D4.lateModel);
    }
    for (i = 0; i < 6; i++) {
        if (lbl_1_bss_4D4.carouselModels[i] != -1) {
            mbObjKill((s16)lbl_1_bss_4D4.carouselModels[i]);
        }
    }
    for (i = 0; i < 4; i++) {
        if (lbl_1_bss_4D4.lotteryModels[i] != -1) {
            mbObjKill((s16)lbl_1_bss_4D4.lotteryModels[i]);
        }
    }
    if (lbl_1_bss_4D4.finalModels[0] != -1) {
        mbObjKill((s16)lbl_1_bss_4D4.finalModels[0]);
    }
    if (lbl_1_bss_4D4.finalModels[1] != -1) {
        mbObjKill((s16)lbl_1_bss_4D4.finalModels[1]);
    }
    if (lbl_1_bss_4D4.finalModels[2] != -1) {
        mbObjKill((s16)lbl_1_bss_4D4.finalModels[2]);
    }
    if (lbl_1_bss_4D4.finalModels[3] != -1) {
        mbObjKill((s16)lbl_1_bss_4D4.finalModels[3]);
    }
    if (lbl_1_bss_4D4.finalModels[4] != -1) {
        mbObjKill((s16)lbl_1_bss_4D4.finalModels[4]);
    }
    if (lbl_1_bss_4D4.motionModel != -1) {
        mbObjKill((s16)lbl_1_bss_4D4.motionModel);
    }
    for (i = 0; i < 3; i++) {
        Hu3DAnimKill(lbl_1_bss_4D4.digitAnimationIds[i]);
    }
    espKill((s16)lbl_1_bss_4D4.largeDisplaySprites[0]);
    espKill((s16)lbl_1_bss_4D4.largeDisplaySprites[1]);
    for (i = 0; i < 3; i++) {
        espKill((s16)lbl_1_bss_4D4.largeDisplayDigits[i]);
    }
    espKill((s16)lbl_1_bss_4D4.totalSprite);
    for (i = 0; i < 4; i++) {
        espKill((s16)lbl_1_bss_4D4.playerSprites[i]);
    }
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 3; j++) {
            espKill((s16)lbl_1_bss_4D4.playerDigits[i][j]);
        }
    }
}

/* Called by the board object manager; this callback has no per-frame work. */
void fn_1_1BB4(OMOBJ *object)
{
}

/* Refreshes the board model's light information whenever the board light hook runs. */
void fn_1_1BB8(void) {
        if ((u8) (((*(u8 *)((s8 *)(&GwSystem) + 16)) >> 6U) & 1) == 0) {
            Hu3DBGColorSet(255U, 255U, 255U);
    } else {
        Hu3DBGColorSet(0U, 0U, 0U);
    }
    Hu3DModelLightInfoSet(mbObjModelIDGet(lbl_1_bss_5C4), 1);
}

/* The board's light reset hook runs without changing any light state. */
void fn_1_1C2C(void)
{
}

/* Dispatches a player's transport, launch or lottery event from the hatena-space hook. */
int fn_1_1C30(int playerNo, s16 spaceId) {
    s32 spaceAttributes;

    spaceAttributes = mbMasuMAttrGet(spaceId);
    if ((u32) (spaceAttributes & 0x28) != 0) {
        fn_1_C59C(playerNo, spaceId);
    }
    if ((u32) (spaceAttributes & 0x10) != 0) {
        fn_1_BD88(playerNo, spaceId);
    }
    if ((u32) (spaceAttributes & 2) != 0) {
        fn_1_8F60(playerNo, spaceId);
    }
    return 1;
}

s32 fn_1_1CC4(void)
{
    return 0;
}

/* The board movement-start hook returns zero without starting a space event. */
int fn_1_1CCC(int playerNo, s16 spaceId)
{
    return 0;
}

/* Runs the coin machine or star wager selected by the move-end space's attributes. */
int fn_1_1CD4(int playerNo, s16 spaceId) {
    MASU *space;
    s32 spaceAttributes;

    space = mbMasuGet(spaceId);
    spaceAttributes = mbMasuMAttrGet(spaceId);
    if ((u32) (spaceAttributes & 1) != 0) {
        fn_1_2718(playerNo, spaceId);
    }
    if ((u32) (spaceAttributes & 4) != 0) {
        fn_1_5900(playerNo, spaceId);
    }
    return 0;
}

/* The board link-table hook returns -1 for every queried space. */
int fn_1_1D5C(s16 spaceId, u32 attributes, s16 *links, BOOL end) {
    return -1;
}

/* Hides the players, runs the time-change guide, then restores them when the event ends. */
void fn_1_1D64(void) {
    HUPROCESS *eventProcess;
    s16 helpWindow;
    s32 *timeChangeState;
    s32 playerNo;

    timeChangeState = lbl_1_bss_4C8;
    for (playerNo = 0; playerNo < 4; playerNo++) {
        mbPlayerDispSet(playerNo, FALSE);
    }
    memset(timeChangeState, 0, 8);
    eventProcess = HuPrcChildCreate(fn_1_1F20, 8199, 32768, 0, mbMainProc);
    helpWindow = -1;
    do {
        if (timeChangeState[0] != 0) {
            if (helpWindow < 0) {
                helpWindow = mbWinCreateHelp(MESSNUM(MESS_BOARD_OPE, 12));
                mbWinPosSet(helpWindow, 228, 408);
            }
            for (playerNo = 0; playerNo < 4; playerNo++) {
                if (GwPlayer[playerNo].comF == FALSE &&
                    (HuPadBtnDown[GwPlayer[playerNo].padNo] & PAD_BUTTON_START) != 0) {
                    HuPrcKill(eventProcess);
                    timeChangeState[1] = 1;
                    break;
                }
            }
        }
        HuPrcVSleep();
    } while (timeChangeState[1] == 0);
    mbWipeWait();
    if (WipeCheckIn() == 0) {
        mbWipeFadeOut();
    }
    if (helpWindow != -1) {
        mbWinKill(helpWindow);
    }
    for (playerNo = 0; playerNo < 4; playerNo++) {
        mbPlayerDispSet(playerNo, TRUE);
    }
}

/* Runs the guide announcement and nighttime reward roll when the board changes time. */

void mbDicePadBtnHookSet(int playerNo, u16 (*hook)(int));

void mbDiceMotHookSet(int playerNo, void (*hook)(int));

s32 lbl_1_data_620[6] = {3, 5, 10, 20, 30, 40};
s8 lbl_1_data_638[5] = {1, 2, 4, 5, -1};
s8 lbl_1_data_63D[4] = {12, 23, 7, -1};
/* Nighttime reward amounts, dice outcomes and guide motion slots. */

/* Runs the time-change announcement and the guide's nighttime reward roll. */
void fn_1_1F20(void)
{
    HuVecF guidePosition;
    HuVecF cameraRotation;
    char rewardText[16];
    HuVecF dicePosition;
    int diceResult;
    s32 *processState;
    s16 masuId;
    s32 playerNo;
    OMOBJ *guide;
    s16 windowNo;

    processState = lbl_1_bss_4C8;
    masuId = mbMasuFind_MAttrIdGet(-1, (1 << 11));
    mbMasuPosGet(masuId, &guidePosition);
    guidePosition.y += 100.0;
    cameraRotation.x = 331.8f;
    cameraRotation.y = 0.0f;
    cameraRotation.z = 0.0f;
    mbCameraMovePos(&guidePosition, &cameraRotation, NULL,
        12000.0f, 30.0f, 0);
    mbWipeFadeIn();
    mbTelopTimeChangeCreate();
    while (mbTelopTimeChangeCheck()) {
        HuPrcVSleep();
    }
    HuPrcSleep(20);
    mbWipeFadeOut();
    mbCameraMovePos(&guidePosition, NULL, NULL,
        1600.0f, 30.0f, 0);
    mbCameraMoveWait();
    mbWipeFadeIn();
    guidePosition.y -= 100.0;
    guide = mbGuideCreateFlag(&guidePosition, lbl_1_data_63D, FALSE, TRUE, FALSE);
    mbGuideMotionNextSet(guide, 1);
    lbl_1_bss_4D4.diceModel = (s16)mbGuideModelGet(guide);
    if (GwSystem.curTime == 0) {
        mbAudGuidePlay(MSM_SE_GUIDE_25);
        mbGuideMotionShiftSet(guide, 12, TRUE);
        windowNo = mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 68), 6);
        mbWinPlayerDisable(windowNo, -1);
        mbWinTopWait();
        mbAudGuidePlay(MSM_SE_GUIDE_25);
        mbGuideMotionSet(guide, 7, TRUE);
        windowNo = mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 61), 6);
        mbWinPlayerDisable(windowNo, -1);
        mbWinTopWait();
        mbGuideEnd(guide, TRUE);
        mbWipeFadeOut();
        for (playerNo = 0; playerNo < 4; playerNo++) {
            mbPlayerDispSet(playerNo, TRUE);
        }
    } else {
        if (lbl_1_bss_4D0->nightEventCount == 0) {
            mbAudGuidePlay(MSM_SE_GUIDE_25);
            mbGuideMotionShiftSet(guide, 12, TRUE);
            windowNo = mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 63), 7);
            mbWinPlayerDisable(windowNo, -1);
            mbWinTopWait();
        } else {
            mbAudGuidePlay(MSM_SE_GUIDE_25);
            mbGuideMotionShiftSet(guide, 12, TRUE);
            windowNo = mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 64), 7);
            mbWinPlayerDisable(windowNo, -1);
            mbWinTopWait();
        }
        lbl_1_bss_4D0->nightEventCount++;
        mbAudGuidePlay(MSM_SE_GUIDE_26);
        mbGuideMotionShiftSet(guide, 12, TRUE);
        windowNo = mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 65), 7);
        mbWinPlayerDisable(windowNo, -1);
        mbWinTopWait();
        mbObjPosGet((s16)lbl_1_bss_4D4.diceModel, &dicePosition);
        diceResult = mbDiceProcExec(-1, 13, lbl_1_data_638, NULL,
            FALSE, FALSE, &dicePosition, 0);
        lbl_1_bss_4 = 50.0f * (0.3f +
            0.7f * (1.5258789e-05f * (f32)(u16)frand()));
        mbDicePadBtnHookSet(-1, (u16 (*)(int))fn_1_2490);
        mbDiceMotHookSet(-1, (void (*)(int))fn_1_24C4);
        while (!mbDiceKillCheck(-1)) {
            HuPrcVSleep();
        }
        playerNo = mbDiceResultGet(-1) - 1;
        lbl_1_bss_4D0->nightReward = (u8)lbl_1_data_620[playerNo];
        sprintf(rewardText, "%d", lbl_1_bss_4D0->nightReward);
        mbAudGuidePlay(MSM_SE_GUIDE_26);
        mbGuideMotionShiftSet(guide, 12, TRUE);
        windowNo = mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 66), 7);
        mbWinTopInsertMesSet((u32)rewardText, 0);
        mbWinPlayerDisable(windowNo, -1);
        mbWinTopWait();
        mbAudGuidePlay(MSM_SE_GUIDE_25);
        mbGuideMotionSet(guide, 7, TRUE);
        windowNo = mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 62), 7);
        mbWinPlayerDisable(windowNo, -1);
        mbWinTopWait();
        mbWipeFadeOut();
        mbGuideKill(guide);
        for (playerNo = 0; playerNo < 4; playerNo++) {
            mbPlayerDispSet(playerNo, TRUE);
        }
        mbDiceNumKill(-1);
    }
    processState[1] = 1;
    HuPrcEnd();
}

/* Returns a dice input after the time-change event's randomized countdown expires. */
s32 fn_1_2490(void) {
    if (--lbl_1_bss_4 == 0) {
        return 256;
    }
    return 0;
}

/* Bounces the board dice and triggers its hit effect when the dice-motion hook runs. */
void fn_1_24C4(void) {
    HuVecF dicePosition;
    f32 frameCount;
    f32 time;
    s32 frame;

    frameCount = 24.0f;
    mbObjPosGet((s16)lbl_1_bss_4D4.diceModel, &dicePosition);
    mbObjMotionShiftSet((s16)lbl_1_bss_4D4.diceModel, 23, 0.0f,
        8.0f, 0);
    for (frame = 0; (f32)frame < frameCount; frame++) {
        time = (f32)frame / frameCount;
        mbObjPosSet(
            (s16)lbl_1_bss_4D4.diceModel,
            dicePosition.x,
            (f32)(dicePosition.y +
                  (100.0 *
                   mbSinDeg(90.0f * time))),
            dicePosition.z);
        HuPrcVSleep();
    }
    mbDiceObjHit(-1);
    for (frame = 0; (f32)frame < (1.0f + frameCount); frame++) {
        time = (f32)frame / frameCount;
        mbObjPosSet(
            (s16)lbl_1_bss_4D4.diceModel,
            dicePosition.x,
            (f32)(dicePosition.y +
                  (100.0 *
                   mbCosDeg(90.0f * time))),
            dicePosition.z);
        HuPrcVSleep();
    }
    HuPrcSleep(60);
    mbObjMotionShiftSet((s16)lbl_1_bss_4D4.diceModel, 1, 0.0f,
        8.0f, HU3D_MOTATTR_LOOP);
}

/* Runs the three-reel coin machine and its accumulated jackpot display. */

float lbl_1_data_644[4] = {45.0f, 77.0f, 102.0f, 127.0f};
float lbl_1_data_654[4] = {2.7e+02f, 313.0f, 356.0f, 399.0f};
HuVecF lbl_1_data_664[2] = {{-3.9e+03f, 5.8e+02f, 1e+02f}, {-3.65e+03f, 5.8e+02f, 3.7e+02f}};
HuVecF lbl_1_data_67C = {-3.58e+03f, 9e+02f, 865.0f};
HuVecF lbl_1_data_688 = {-5.0f, 45.0f, 0.0f};
int lbl_1_data_694[4] = {0, 20, 15, 10};
char lbl_1_data_6A4[3][8] = {"r1", "r2", "r3"};
s16 lbl_1_data_6BC[3][8] = { { 0, 2, 3, 2, 1, 3, 3, 1 },
                             { 0, 1, 3, 2, 3, 3, 1, 2 },
                             { 0, 3, 2, 1, 2, 1, 3, 3 } };
int lbl_1_data_6EC[2] = {5, 10};
/* Runs the three-reel wager after the move-end hook selects the coin-machine space. */
void fn_1_2718(int playerNo, s16 landingSpace)
{
    float speeds[3];
    float initialSpeeds[3];
    HuVecF savedCameraRotation;
    HuVecF savedCameraCenter;
    HuVecF strikePosition;
    HuVecF particlePosition;
    HuVecF prizePosition;
    HuVecF awardPosition;
    HuVecF playerPosition;
    HuVecF hostPosition;
    HuVecF hostRotation;
    HuVecF initialDirection;
    HuVecF currentDirection;
    HuVecF prizeMovePosition;
    int hitFrame = 0;
    int nextSpace;
    int jackpotCoins;
    int payoutInterval;
    int fadeFrame = 0;
    int awardCoins = 0;
    int stopRequested[3] = { 0, 0, 0 };
    int stopped[3] = { 0, 0, 0 };
    int sectors[3];
    int results[3];
    float sectorOffsets[3];
    float brakingFactors[3];
    int sectorAdvance[3];
    HuVecF spritePosition;
    HSF_OBJECT *reelParts[3];
    HuVecF entranceStart;
    HuVecF entranceEnd;
    HuVecF declineStart;
    HuVecF declineEnd;
    HuVecF exitStart;
    HuVecF exitEnd;
    char betText[10];
    s16 symbols[3];
    int coinDisplay;
    int timerFrames;
    int reelFinished = 0;
    int fading = 0;
    int prizeEffectActive = 0;
    int assistChosen = 0;
    int comMistake = 0;
    int hitState;
    int helpWindow;
    int timeSlot = 0;
    u32 hundreds;
    int soundHandle;
    ANIMDATA *particleAnimation;
    HU3D_MODEL *machineModel;
    HSF_DATA *machineData;
    int modelId;
    int isDay;
    HuVecF *entranceEndPtr;
    HuVecF *entranceStartPtr;
    HuVecF *declineEndPtr;
    HuVecF *declineStartPtr;
    HuVecF *exitEndPtr;
    HuVecF *exitStartPtr;
    s16 prizeParticle;
    int speaker;
    float baseSpeed;
    float secondSpeed;
    float thirdSpeed;
    float hostYaw;
    float turnStep;
    float cameraZoom;
    float initialAngle;
    float currentAngle;
    float rotationDelta;
    float alpha;
    float comDelay;
    float displayScale;
    float strikeProgress;
    float strikeHeight;
    s16 stopParticle;
    int stoppedCount;
    s16 timer;
    int comReel;
    int predictedSector;
    int displayCoins;
    u16 buttons;
    int index;
    int detailIndex;
    int scan;

    if (GwSystem.curTime == 0) {
        speaker = 8;
        baseSpeed = 2.5f;
        secondSpeed = 4.5f;
        thirdSpeed = 6.5f;
    } else {
        timeSlot++;
        speaker = 9;
        baseSpeed = 3.5f;
        secondSpeed = 6.5f;
        thirdSpeed = 9.5f;
    }
    for (index = 0; index < 3; index++) {
        sectorAdvance[index] = 1;
        initialSpeeds[index] = speeds[index] = baseSpeed;
    }
    isDay = GwSystem.curTime == 0;
    sprintf(betText, "%d", lbl_1_data_6EC[isDay]);
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbMoveNumDispSet(playerNo, FALSE);
    if (mbPlayerCoinGet(playerNo) < lbl_1_data_6EC[timeSlot]) {
        mbPlayerWinLoseVoicePlay(playerNo, 13, CHARVOICEID(12));
        mbPlayerMotionShiftSet(playerNo, 13, 0.0f, 8.0f, 0);
        if (GwSystem.curTime == 0) {
            mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 5), -1);
        } else {
            mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 6), -1);
        }
        mbWinTopWait();
        mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    } else {
        modelId = mbObjModelIDGet((s16)lbl_1_bss_4D4.initialModels[0]);
        machineModel = &Hu3DData[modelId];
        machineData = machineModel->hsf;
        for (index = 0; index < 3; index++) {
            reelParts[index] = fn_1_D150(machineModel, lbl_1_data_6A4[index]);
        }
        mbObjDispSet((s16)lbl_1_bss_4D4.motionModel, TRUE);
        mbObjPosSetV((s16)lbl_1_bss_4D4.motionModel, &lbl_1_data_664[0]);
        hostYaw = 45.0f;
        mbPlayerRotateStart(playerNo, -135, 15);
        while (!mbPlayerRotateCheck(playerNo)) {
            HuPrcVSleep();
        }
        mbObjMotionTimeSet((s16)lbl_1_bss_4D4.initialModels[1], 0.0f);
        mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.initialModels[1], 2.0f);
        entranceEnd = lbl_1_data_664[1];
        entranceEndPtr = &entranceEnd;
        entranceStart = lbl_1_data_664[0];
        entranceStartPtr = &entranceStart;
        fn_1_D9E0(lbl_1_bss_4D4.initialModels[1], lbl_1_bss_4D4.motionModel,
            hostYaw, 0.0f, entranceStartPtr, entranceEndPtr, 0);
        mbObjMotionTimeSet((s16)lbl_1_bss_4D4.initialModels[1], 120.0f);
        mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.initialModels[1], 2.0f);
        while (mbObjMotionTimeGet((s16)lbl_1_bss_4D4.initialModels[1]) < 180.0f) {
            HuPrcVSleep();
        }
        mbObjMotionTimeSet((s16)lbl_1_bss_4D4.initialModels[1], 0.0f);
        mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.initialModels[1], 0.0f);
        mbObjMotionShiftSet((s16)lbl_1_bss_4D4.motionModel, 2, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        turnStep = 2.9999998f;
        for (index = 0; (float)index < 15.000001f; index++) {
            hostYaw -= turnStep;
            mbObjRotSet((s16)lbl_1_bss_4D4.motionModel, 0.0f, hostYaw, 0.0f);
            HuPrcVSleep();
        }
        mbObjMotionShiftSet((s16)lbl_1_bss_4D4.motionModel, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_12A14(0);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 0), speaker);
        mbWinTopWait();
        fn_1_12A14(0);
        if (GwSystem.curTime == 0) {
            mbWinCreateChoice(0, MESSNUM(MESS_BOARD_W03, 1), speaker, 0);
        } else {
            mbWinCreateChoice(0, MESSNUM(MESS_BOARD_W03, 2), speaker, 0);
        }
        if (GwPlayer[playerNo].comF) {
            mbComChoiceUpSet();
        }
        mbWinTopWait();
        if (mbWinTopChoiceGet() == 1 || mbWinTopChoiceGet() == -1) {
            mbObjMotionShiftSet((s16)lbl_1_bss_4D4.motionModel, 4, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            fn_1_12A14(1);
            mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 4), speaker);
            mbWinTopWait();
            mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.initialModels[1], 2.0f);
            declineEnd = lbl_1_data_664[0];
            declineEndPtr = &declineEnd;
            declineStart = lbl_1_data_664[1];
            declineStartPtr = &declineStart;
            fn_1_D9E0(lbl_1_bss_4D4.initialModels[1], lbl_1_bss_4D4.motionModel,
                hostYaw, 135.0f, declineStartPtr, declineEndPtr, 1);
        } else {
            for (index = 0; index < lbl_1_data_6EC[timeSlot]; index++) {
                mbPlayerCoinAdd(playerNo, -1);
                mbAudFXPlay(14);
                mbAudFXPlay(MSM_SE_W03_SLOT_COIN_PAYOUT);
                if (++lbl_1_bss_4D0->displayValue > 999) {
                    lbl_1_bss_4D0->displayValue = 999;
                }
                fn_1_DE88(lbl_1_bss_4D0->displayValue);
                HuPrcSleep(6);
            }
            mbAudFXPlay(15);
            mbPlayerMotionShiftSet(playerNo, 2, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            nextSpace = mbMasuTypeFindLink(landingSpace, 0);
            mbPlayerMasuMoveSpeed(playerNo, nextSpace, 20, FALSE);
            mbPlayerPosGet(playerNo, &playerPosition);
            mbObjPosGet((s16)lbl_1_bss_4D4.motionModel, &hostPosition);
            mbObjRotGet((s16)lbl_1_bss_4D4.motionModel, &hostRotation);
            PSVECSubtract(&playerPosition, &hostPosition, &initialDirection);
            for (index = 0; index < 20; index++) {
                mbPlayerPosGet(playerNo, &playerPosition);
                PSVECSubtract(&playerPosition, &hostPosition, &currentDirection);
                initialAngle = atan2(initialDirection.z, initialDirection.x);
                currentAngle = atan2(currentDirection.z, currentDirection.x);
                rotationDelta = 180.0 * ((double)(currentAngle - initialAngle)
                    / 3.141592653589793);
                mbObjRotSet((s16)lbl_1_bss_4D4.motionModel, hostRotation.x,
                    hostRotation.y - rotationDelta, hostRotation.z);
                HuPrcVSleep();
            }
            mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            mbCameraCenterGet(&savedCameraCenter);
            mbCameraRotGet(&savedCameraRotation);
            cameraZoom = mbCameraZoomGet();
            mbCameraFocusPlayerSet(-1);
            mbCameraMovePos(&lbl_1_data_67C, &lbl_1_data_688, NULL, 2000.0f, -1.0f, 30);
            mbStatusDispSetAll(FALSE);
            while (!mbStatusOffCheckAll()) {
                HuPrcVSleep();
            }
            mbStatusDispFocusSet(playerNo, TRUE);
            while (!mbStatusMoveCheck(playerNo)) {
                HuPrcVSleep();
            }
            espDispOn((s16)lbl_1_bss_4D4.totalSprite);
            for (index = 0; index < 4; index++) {
                espDispOn((s16)lbl_1_bss_4D4.playerSprites[index]);
            }
            for (index = 0; index < 4; index++) {
                for (detailIndex = 0; detailIndex < 3; detailIndex++) {
                    if (detailIndex != 0) {
                        espDispOn((s16)lbl_1_bss_4D4.playerDigits[index][detailIndex]);
                    }
                }
                if (GwSystem.curTime == 0) {
                    displayCoins = lbl_1_data_694[index];
                } else {
                    displayCoins = 2.0f * (float)lbl_1_data_694[index];
                }
                if (index == 0) {
                    displayCoins = lbl_1_bss_4D0->displayValue;
                }
                if (displayCoins >= 100) {
                    espDispOn((s16)lbl_1_bss_4D4.playerDigits[index][0]);
                    hundreds = displayCoins / 100;
                    espBankSet((s16)lbl_1_bss_4D4.playerDigits[index][0], hundreds);
                    displayCoins -= hundreds * 100;
                    espPosSet((s16)lbl_1_bss_4D4.playerSprites[index],
                        lbl_1_data_644[index], lbl_1_data_654[index]);
                } else {
                    spritePosition.x = 12.0f + lbl_1_data_644[0];
                    espPosSet((s16)lbl_1_bss_4D4.playerSprites[index], spritePosition.x,
                        lbl_1_data_654[index]);
                }
                espBankSet((s16)lbl_1_bss_4D4.playerDigits[index][1], displayCoins / 10);
                espBankSet((s16)lbl_1_bss_4D4.playerDigits[index][2], displayCoins % 10);
            }
            for (scan = 0; (float)scan <= 10.0f; scan++) {
                strikeProgress = (float)scan / 10.0f;
                displayScale = mbSinDeg(90.0f * strikeProgress);
                espScaleSet((s16)lbl_1_bss_4D4.totalSprite, 0.7f, 2.1f * displayScale);
                for (index = 0; index < 4; index++) {
                    espScaleSet((s16)lbl_1_bss_4D4.playerSprites[index], 0.8f, 0.8f * displayScale);
                }
                for (index = 0; index < 4; index++) {
                    for (detailIndex = 0; detailIndex < 3; detailIndex++) {
                        espScaleSet((s16)lbl_1_bss_4D4.playerDigits[index][detailIndex],
                            0.6f, 0.6f * displayScale);
                    }
                }
                HuPrcVSleep();
            }
            mbPlayerPosGet(playerNo, &particlePosition);
            particlePosition.y += 240.0;
            particleAnimation =
                HuSprAnimRead(HuDataReadNum(DATANUM(DATA_effect, 3), HU_MEMNUM_OVL));
            stopParticle = mbParticleCreate(particleAnimation, 128);
            mbParticleHookSet(stopParticle, fn_1_F704);
            Hu3DModelLayerSet(stopParticle, 5);
            Hu3DModelPosSetV(stopParticle, &particlePosition);
            mbObjPosSet((s16)lbl_1_bss_4D4.initialModels[4], particlePosition.x,
                20.0f + particlePosition.y, particlePosition.z);
            mbObjRotSet((s16)lbl_1_bss_4D4.initialModels[4], 0.0f, 45.0f, 0.0f);
            mbObjAlphaSet((s16)lbl_1_bss_4D4.initialModels[4], 0);
            mbAudFXPlay(MSM_SE_W03_SLOT_PRIZE_REVEAL);
            for (index = 0; (float)index < 25.0f; index++) {
                alpha = (255.0f * (float)index) / 25.0f;
                mbObjAlphaSet((s16)lbl_1_bss_4D4.initialModels[4], (s32)alpha);
                HuPrcVSleep();
            }
            HuPrcSleep(10);
            mbAudFXPlay(MSM_SE_W03_SLOT_REEL_SPIN);
            mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.initialModels[3], 1.0f);
            while (mbObjMotionTimeGet((s16)lbl_1_bss_4D4.initialModels[3]) < 100.0f) {
                HuPrcVSleep();
            }
            mbAudFXPlay(MSM_SE_W03_SLOT_REEL_STOP);
            mbParticleKill(stopParticle);
            stopParticle = -1;
            soundHandle = mbAudFXPlay(MSM_SE_W03_SLOT_MACHINE_WAIT);
            timerFrames = 1200;
            timer = GameMesCreate(1, 20, -1, -1);
            HuSprGrpDrawNoSet(GameMesGet(timer)->grpId[0], 32);
            HuPrcVSleep();
            stoppedCount = 0;
            hitState = 0;
            comDelay = 50.0f + ((float)(u32)frandmod(60) - 30.0f);
            comReel = 0;
            helpWindow = mbWinCreateHelp(MESSNUM(MESS_BOARD_W03, 11));
            mbWinPosSet(helpWindow, 240, 376);
            for (;;) {
                for (index = 0; index < 3; index++) {
                    if ((reelParts[index]->mesh.base.rot.x += speeds[index]) >= 360.0f) {
                        reelParts[index]->mesh.base.rot.x -= 360.0f;
                    }
                }
                if (timer == -1) {
                    if (helpWindow != -1) {
                        mbWinKill(helpWindow);
                        helpWindow = -1;
                    }
                    if (fading == 0) {
                        fading = 1;
                        particleAnimation =
                            HuSprAnimRead(HuDataReadNum(DATANUM(DATA_effect, 3), HU_MEMNUM_OVL));
                        stopParticle = mbParticleCreate(particleAnimation, 128);
                        mbParticleHookSet(stopParticle, fn_1_F704);
                        Hu3DModelLayerSet(stopParticle, 5);
                        Hu3DModelPosSetV(stopParticle, &particlePosition);
                    }
                    for (index = 0; index < 3; index++) {
                        if (stopRequested[index] == 0) {
                            sectors[index] = reelParts[index]->mesh.base.rot.x / 45.0f;
                            sectorOffsets[index] = reelParts[index]->mesh.base.rot.x
                                - 45.0f * (float)sectors[index];
                            brakingFactors[index] = (float)((double)initialSpeeds[index]
                                / fabs(45.0f - sectorOffsets[index]));
                            stopRequested[index] = 1;
                            break;
                        }
                    }
                    for (index = 0; index < 3; index++) {
                        if (stopped[index] == 0 && stopRequested[index] != 0) {
                            speeds[index] -= 0.5f * brakingFactors[index] * initialSpeeds[index];
                            if (speeds[index] <= 0.0f) {
                                sectorOffsets[index] = reelParts[index]->mesh.base.rot.x
                                    - 45.0f * (float)sectors[index];
                                if (sectorOffsets[index] >= 0.0f) {
                                    reelParts[index]->mesh.base.rot.x +=
                                        45.0f - sectorOffsets[index];
                                }
                                results[index] = reelParts[index]->mesh.base.rot.x / 45.0f;
                                speeds[index] = 0.0f;
                                if (results[index] >= 8) {
                                    results[index] -= 8;
                                }
                                reelParts[index]->mesh.base.rot.x = 45.0f * (float)results[index];
                                stopped[index] = 1;
                                mbAudFXPlay(MSM_SE_W03_SLOT_SYMBOL_STOP);
                                stoppedCount++;
                            }
                        }
                    }
                } else {
                    if (GwPlayer[playerNo].comF) {
                        buttons = 0;
                        if (GwPlayerConf[playerNo].comDif <= 1) {
                            if (!(comDelay -= 1.0f)) {
                                buttons |= 0x100;
                                comDelay = 50.0f + ((float)(u32)frandmod(60) - 30.0f);
                            }
                        } else switch (comReel) {
                        case 0:
                            if (stopRequested[0] == 0) {
                                if ((comDelay -= 1.0f) <= 0.0f) {
                                    buttons |= 0x100;
                                    comDelay = 50.0f + ((float)(u32)frandmod(60) - 30.0f);
                                }
                            }
                            break;
                        case 1:
                        case 2:
                                if (stopRequested[comReel] == 0 && stopped[comReel - 1] != 0) {
                                    if ((comDelay -= 1.0f) <= 0.0f && comReel < 3) {
                                        predictedSector = (reelParts[comReel]->mesh.base.rot.x
                                            + 13.0f * speeds[comReel]) / 45.0f;
                                        if (comReel == 2) {
                                            if (comMistake) {
                                                predictedSector++;
                                            }
                                        } else {
                                            predictedSector++;
                                        }
                                        if (predictedSector >= 8) {
                                            predictedSector -= 8;
                                        }
                                        if (lbl_1_data_6BC[comReel - 1][results[comReel - 1]]
                                            == lbl_1_data_6BC[comReel][predictedSector]) {
                                            buttons |= 0x100;
                                            comDelay = 50.0f + ((float)(u32)frandmod(60) - 30.0f);
                                        }
                                    }
                                }
                            break;
                        }
                    } else {
                        buttons = HuPadBtnDown[GwPlayer[playerNo].padNo];
                    }
                    if ((buttons & 0x100) && hitState == 0) {
                        hitState = 1;
                    }
                    for (index = 0; index < 3; index++) {
                        if (stopped[index] == 0 && stopRequested[index] != 0) {
                            speeds[index] -= 0.5f * brakingFactors[index] * initialSpeeds[index];
                            if (speeds[index] <= 0.0f) {
                                sectorOffsets[index] = reelParts[index]->mesh.base.rot.x
                                    - 45.0f * (float)sectors[index];
                                if (sectorOffsets[index] >= 0.0f) {
                                    reelParts[index]->mesh.base.rot.x +=
                                        45.0f * (float) sectorAdvance[index] - sectorOffsets[index];
                                }
                                results[index] = reelParts[index]->mesh.base.rot.x / 45.0f;
                                speeds[index] = 0.0f;
                                if (results[index] >= 8) {
                                    results[index] -= 8;
                                }
                                reelParts[index]->mesh.base.rot.x = 45.0f * (float)results[index];
                                stopped[index] = 1;
                                comReel++;
                                reelFinished = 1;
                                mbAudFXPlay(MSM_SE_W03_SLOT_SYMBOL_STOP);
                                if (index == 0 && mbRandMod(10) < 6) {
                                    assistChosen = 1;
                                }
                                if (GwPlayerConf[playerNo].comDif == 2 && (u32)frandmod(8) == 0) {
                                    comMistake = 1;
                                }
                                if (GwPlayerConf[playerNo].comDif == 3 && (u32)frandmod(5) == 0) {
                                    comMistake = 1;
                                }
                            }
                        }
                    }
                }
                if (stoppedCount >= 3 && soundHandle >= 0) {
                    mbAudFXStop(soundHandle);
                    soundHandle = -1;
                }
                if (stoppedCount >= 3 && alpha <= 0.0f) {
                    if (helpWindow != -1) {
                        mbWinKill(helpWindow);
                        helpWindow = -1;
                    }
                    break;
                }
                if (mbPlayerMotionEndCheck(playerNo)) {
                    mbPlayerMotionSet(playerNo, 1, HU3D_MOTATTR_LOOP);
                }
                switch (hitState) {
                case 0:
                    break;
                case 1:
                    if (hitFrame == 0) {
                        mbPlayerMotionShiftSet(playerNo, 11, 10.0f, 5.0f, 0);
                        hitFrame++;
                    } else if ((float)hitFrame++ > 13.0f) {
                        if (stoppedCount == 2) {
                            fading = 1;
                            particleAnimation = HuSprAnimRead(
                                HuDataReadNum(DATANUM(DATA_effect, 3), HU_MEMNUM_OVL));
                            stopParticle = mbParticleCreate(particleAnimation, 128);
                            mbParticleHookSet(stopParticle, fn_1_F704);
                            Hu3DModelLayerSet(stopParticle, 5);
                            particlePosition.y += 20.0f;
                            Hu3DModelPosSetV(stopParticle, &particlePosition);
                            mbAudFXPlay(MSM_SE_BRD00_45);
                        } else if (stoppedCount == 1) {
                            initialSpeeds[2] = speeds[2] = thirdSpeed;
                        } else if (stoppedCount == 0) {
                            initialSpeeds[1] = initialSpeeds[2] = speeds[1] = speeds[2] =
                                secondSpeed;
                        }
                        mbAudFXPlay(MSM_SE_BRD00_56);
                        omVibrate(playerNo, 20, 7, 3);
                        hitState = 2;
                        hitFrame = 0;
                        for (index = 0; index < 3; index++) {
                            if (stopRequested[index] == 0) {
                                sectors[index] = reelParts[index]->mesh.base.rot.x / 45.0f;
                                if (index == 1) {
                                    detailIndex = 0;
                                    scan = sectors[index] + 1;
                                    for (; detailIndex < 3; detailIndex++, scan++) {
                                        if (scan >= 8) {
                                            scan -= 8;
                                        }
                                        if (lbl_1_data_6BC[0][results[0]] ==
                                            lbl_1_data_6BC[index][scan]) {
                                            sectorAdvance[index] += detailIndex;
                                            break;
                                        }
                                    }
                                } else if (index == 2) {
                                    detailIndex = 0;
                                    scan = sectors[index] + 1;
                                    for (; detailIndex < 2; detailIndex++, scan++) {
                                        if (scan >= 8) {
                                            scan -= 8;
                                        }
                                        if (lbl_1_data_6BC[1][results[1]] ==
                                            lbl_1_data_6BC[index][scan]) {
                                            sectorAdvance[index] += detailIndex;
                                            break;
                                        }
                                    }
                                }
                                sectorOffsets[index] = reelParts[index]->mesh.base.rot.x
                                    - 45.0f * (float)sectors[index];
                                brakingFactors[index] =
                                    (float) ((double) initialSpeeds[index] /
                                             fabs(45.0f * (float) sectorAdvance[index] -
                                                  sectorOffsets[index]));
                                stopRequested[index] = 1;
                                break;
                            }
                        }
                    }
                    break;
                case 2:
                    mbObjPosGet((s16)lbl_1_bss_4D4.initialModels[4], &strikePosition);
                    strikeProgress = (float)hitFrame / 10.0f;
                    hitFrame++;
                    if (strikeProgress > 1.0f) {
                        strikeProgress = 1.0f;
                        hitState = 3;
                        hitFrame = 0;
                    }
                    displayScale = mbSinDeg(180.0f * strikeProgress);
                    strikeHeight = strikePosition.y + 10.0f * displayScale;
                    mbObjPosSet((s16)lbl_1_bss_4D4.initialModels[4], strikePosition.x,
                        strikeHeight, strikePosition.z);
                    break;
                case 3:
                    mbObjPosGet((s16)lbl_1_bss_4D4.initialModels[4], &strikePosition);
                    strikeProgress = (float)hitFrame / 10.0f;
                    hitFrame++;
                    if (strikeProgress > 1.0f) {
                        strikeProgress = 1.0f;
                        hitState = 4;
                        hitFrame = 0;
                    }
                    displayScale = mbSinDeg(180.0f * strikeProgress);
                    strikeHeight = strikePosition.y - 10.0f * displayScale;
                    mbObjPosSet((s16)lbl_1_bss_4D4.initialModels[4], strikePosition.x,
                        strikeHeight, strikePosition.z);
                    break;
                case 4:
                    if (reelFinished && mbPlayerMotionGet(playerNo) == 1) {
                        stoppedCount++;
                        hitState = 0;
                        reelFinished = 0;
                    }
                    break;
                }
                if (fading) {
                    fadeFrame++;
                    alpha = 255.0f - ((255.0f * (float)fadeFrame) / 25.0f);
                    if (alpha <= 0.0f) {
                        alpha = 0.0f;
                    }
                    mbObjAlphaSet((s16)lbl_1_bss_4D4.initialModels[4], (s32)alpha);
                }
                if (timerFrames >= 0) {
                    timerFrames--;
                    if (timerFrames >= 0) {
                        GameMesDispSet(timer, 1, (s16)((timerFrames + 59) / 60));
                    } else if (timer >= 0) {
                        GameMesDispSet(timer, 2, -1);
                        timer = -1;
                    }
                }
                HuPrcVSleep();
            }
            if (timer >= 0) {
                GameMesDispSet(timer, 2, -1);
                timer = -1;
            }
            for (scan = 0; (float)scan <= 10.0f; scan++) {
                strikeProgress = (float)scan / 10.0f;
                displayScale = mbCosDeg(90.0f * strikeProgress);
                espScaleSet((s16)lbl_1_bss_4D4.totalSprite, 0.7f, 2.1f * displayScale);
                for (index = 0; index < 4; index++) {
                    espScaleSet((s16)lbl_1_bss_4D4.playerSprites[index], 0.8f, 0.8f * displayScale);
                }
                for (index = 0; index < 4; index++) {
                    for (detailIndex = 0; detailIndex < 3; detailIndex++) {
                        espScaleSet((s16)lbl_1_bss_4D4.playerDigits[index][detailIndex],
                            0.6f, 0.6f * displayScale);
                    }
                }
                HuPrcVSleep();
            }
            espDispOff((s16)lbl_1_bss_4D4.totalSprite);
            for (index = 0; index < 4; index++) {
                espDispOff((s16)lbl_1_bss_4D4.playerSprites[index]);
            }
            for (index = 0; index < 4; index++) {
                for (detailIndex = 0; detailIndex < 3; detailIndex++) {
                    espDispOff((s16)lbl_1_bss_4D4.playerDigits[index][detailIndex]);
                }
            }
            for (index = 0; index < 3; index++) {
                symbols[index] = lbl_1_data_6BC[index][results[index]];
            }
            if (symbols[0] == symbols[1] && symbols[1] == symbols[2]) {
                prizeEffectActive = 1;
                mbPlayerPosGet(playerNo, &awardPosition);
                if (symbols[0] == 0) {
                    fn_1_12A14(1);
                    mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 8), speaker);
                    mbWinTopWait();
                    if (lbl_1_bss_4D0->displayValue >= 50) {
                        payoutInterval = 1;
                    } else if (lbl_1_bss_4D0->displayValue >= 20) {
                        payoutInterval = 3;
                    } else {
                        payoutInterval = 6;
                    }
                    jackpotCoins = lbl_1_bss_4D0->displayValue;
                    awardCoins = lbl_1_bss_4D0->displayValue;
                    for (index = 0; index < jackpotCoins; index++) {
                        mbAudFXPlay(MSM_SE_W03_SLOT_COIN_PAYOUT);
                        fn_1_DE88(--lbl_1_bss_4D0->displayValue);
                        HuPrcSleep(payoutInterval);
                    }
                } else {
                    fn_1_12A14(0);
                    mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 7), speaker);
                    mbWinTopWait();
                    switch (symbols[0]) {
                    case 1:
                        awardCoins = lbl_1_data_694[2];
                        break;
                    case 2:
                        awardCoins = lbl_1_data_694[1];
                        break;
                    case 3:
                        awardCoins = lbl_1_data_694[3];
                        break;
                    }
                    if (GwSystem.curTime) {
                        awardCoins *= 2.0f;
                    }
                }
                mbCameraMovePos(NULL, NULL, NULL, 1100.0f, -1.0f, 60);
                mbPlayerRotateStart(playerNo, 45, 15);
                while (!mbPlayerRotateCheck(playerNo)) {
                    HuPrcVSleep();
                }
                mbCameraMoveWait();
                mbPlayerPosGet(playerNo, &prizePosition);
                prizePosition.y += 450.0f;
                mbObjDispSet((s16)lbl_1_bss_4D4.initialModels[5], TRUE);
                mbObjRotSet((s16)lbl_1_bss_4D4.initialModels[5], 0.0f, 45.0f, 0.0f);
                mbObjAlphaSet((s16)lbl_1_bss_4D4.initialModels[5], 255);
                mbObjMotionTimeSet((s16)lbl_1_bss_4D4.initialModels[5], 0.0f);
                mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.initialModels[5], 0.0f);
                for (index = 0; index < 60U; index++) {
                    displayScale = (float)index / 60.0f;
                    prizeMovePosition.x = prizePosition.x;
                    prizeMovePosition.y = prizePosition.y + 100.0f
                        * (3.0f * mbCosDeg(90.0f * displayScale));
                    prizeMovePosition.z = prizePosition.z;
                    mbObjPosSetV((s16)lbl_1_bss_4D4.initialModels[5], &prizeMovePosition);
                    HuPrcVSleep();
                }
                mbObjPosSetV((s16)lbl_1_bss_4D4.initialModels[5], &prizePosition);
                mbObjRotSet((s16)lbl_1_bss_4D4.initialModels[5], 0.0f, 45.0f, 0.0f);
                HuPrcSleep(50);
                mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.initialModels[5], 1.0f);
                mbAudFXPlay(MSM_SE_W03_SLOT_JACKPOT);
                mbEffConfettiCreate(&prizePosition, 100, 20.0f);
                fn_1_E00C(awardCoins);
                HuPrcSleep(30);
                mbEffConfettiReset();
                HuPrcSleep(30);
                prizeParticle = mbParticleCreate(particleAnimation, 64);
                mbParticleHookSet(prizeParticle, fn_1_F704);
                Hu3DModelLayerSet(prizeParticle, 5);
                Hu3DModelPosSetV(prizeParticle, &prizePosition);
                mbAudFXPlay(MSM_SE_BRD00_45);
                for (index = 0; (float)index < 25.0f; index++) {
                    alpha = 255.0f - (255.0f * (float)index) / 25.0f;
                    mbObjAlphaSet((s16)lbl_1_bss_4D4.initialModels[5], (s32)alpha);
                    HuPrcVSleep();
                }
                mbObjAlphaSet((s16)lbl_1_bss_4D4.initialModels[5], 0);
                mbPlayerPosGet(playerNo, &playerPosition);
                playerPosition.y += 250.0f;
                coinDisplay = mbCoinDispCapsuleCreate(&playerPosition, awardCoins);
                mbPlayerWinLoseVoicePlay(playerNo, 12, CHARVOICEID(6));
                mbPlayerMotionShiftSet(playerNo, 12, 0.0f, 8.0f, 0);
                while (!mbPlayerMotionEndCheck(playerNo) && !mbCoinDispKillCheck(coinDisplay)) {
                    HuPrcVSleep();
                }
                mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
                mbObjMotionShiftSet((s16) lbl_1_bss_4D4.motionModel, 4, 0.0f, 8.0f,
                                    HU3D_MOTATTR_LOOP);
                fn_1_12A14(1);
                mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 10), speaker);
                mbWinTopWait();
            } else {
                mbPlayerWinLoseVoicePlay(playerNo, 13, CHARVOICEID(12));
                mbPlayerMotionShiftSet(playerNo, 13, 0.0f, 8.0f, 0);
                while (!mbPlayerMotionEndCheck(playerNo)) {
                    HuPrcVSleep();
                }
                mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
                mbObjMotionShiftSet((s16) lbl_1_bss_4D4.motionModel, 4, 0.0f, 8.0f,
                                    HU3D_MOTATTR_LOOP);
                fn_1_12A14(2);
                mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 9), speaker);
                mbWinTopWait();
            }
            mbCameraMovePlayer(playerNo, &savedCameraRotation, NULL, cameraZoom, -1.0f, 30);
            mbCameraMoveWait();
            mbStatusDispFocusSet(playerNo, FALSE);
            while (!mbStatusMoveCheck(playerNo)) {
                HuPrcVSleep();
            }
            mbStatusDispSetAll(TRUE);
            mbObjRotGet((s16)lbl_1_bss_4D4.motionModel, &hostRotation);
            mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.initialModels[1], 2.0f);
            exitEnd = lbl_1_data_664[0];
            exitEndPtr = &exitEnd;
            exitStart = lbl_1_data_664[1];
            exitStartPtr = &exitStart;
            fn_1_D9E0(lbl_1_bss_4D4.initialModels[1], lbl_1_bss_4D4.motionModel,
                hostRotation.y, 90.0f, exitStartPtr, exitEndPtr, 1);
            mbPlayerMotionShiftSet(playerNo, 2, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            mbPlayerMasuMoveTo(playerNo, landingSpace, TRUE);
            mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.initialModels[3], 0.0f);
            mbObjMotionTimeSet((s16)lbl_1_bss_4D4.initialModels[3], 0.0f);
            if (prizeEffectActive) {
                mbParticleKill(prizeParticle);
                prizeParticle = -1;
                prizeEffectActive = 0;
            }
            mbParticleKill(stopParticle);
            stopParticle = -1;
            HuDataDirClose(DATA_effect);
            for (index = 0; index < 3; index++) {
                lbl_1_bss_4D0->wheelRotations[index] = results[index];
            }
        }
    }
    mbObjDispSet((s16)lbl_1_bss_4D4.motionModel, FALSE);
    mbMoveNumDispSet(playerNo, TRUE);
}

/* Runs the star wager in which the player follows a shuffled carousel object. */

HuVecF lbl_1_data_6F4[2] = {{3.48e+03f, 575.0f, -3e+01f}, {3.79e+03f, 575.0f, 2.8e+02f}};
HuVecF lbl_1_data_70C = {4.58e+03f, 6.4e+02f, -1.6e+02f};
HuVecF lbl_1_data_718 = {4615.6f, 940.7f, 248.5f};
/* Charges one star, shuffles the objects, then pays the reward for a correct choice. */
void fn_1_5900(int playerNo, s16 landingSpace)
{
    HuVecF carouselOffsets[6];
    HuVecF carouselSteps[6];
    HuVecF starSteps[3];
    HuVecF starPositions[3];
    HuVecF starRotations[3];
    HuVecF starScales[3];
    HuVecF starOffsets[3];
    int objectTimers[6];
    int objectStates[6];
    int soundHandles[6];
    int starObjects[3];
    int starDelays[3];
    float starAngles[3];
    HuVecF initialDirection;
    HuVecF currentDirection;
    HuVecF screenTarget;
    HuVecF screenPosition;
    HuVecF cursorTarget;
    HuVecF dropPosition;
    HuVecF playerPosition;
    HuVecF hostPosition;
    HuVecF hostRotation;
    HuVecF landingPosition;
    char rewardText[16];
    HuVecF entranceStart;
    HuVecF entranceEnd;
    HuVecF declineStart;
    HuVecF declineEnd;
    HuVecF exitStart;
    HuVecF exitEnd;
    int nextSpace;
    int stickY;
    u32 shuffleDirection;
    int direction;
    int unusedShuffleCount;
    int objectsMoving;
    int starDisplay;
    u32 correctObject;
    u32 comChoice;
    int shuffleRepeats;
    int swapRepeats;
    int helpWindow;
    HuVecF *entranceEndPtr;
    HuVecF *entranceStartPtr;
    HuVecF *declineEndPtr;
    HuVecF *declineStartPtr;
    HuVecF *exitEndPtr;
    HuVecF *exitStartPtr;
    int speaker;
    int objectCount;
    int rewardCount;
    u32 hiddenObject;
    u32 choice;
    int cursorSprite;
    int timerFrames;
    int stickX;
    int buttons;
    s16 timer;
    int repeatDelay;
    int comDelay;
    int index;
    int starIndex;
    float yaw;
    float turnStep;
    float initialYaw;
    float rotationStep;
    float scaleStep;
    float angleStart;
    float angleNow;
    float dropStep;

    if (boardIsDay()) {
        speaker = 8;
        objectCount = 3;
        rewardCount = 2;
    } else {
        speaker = 9;
        objectCount = 6;
        rewardCount = 3;
    }
    for (index = 0; index < objectCount; index++) {
        lbl_1_bss_450[index] = index;
    }
    mbMoveNumDispSet(playerNo, FALSE);
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    if (mbPlayerStarGet(playerNo) < 1) {
        mbPlayerWinLoseVoicePlay(playerNo, 13, CHARVOICEID(12));
        mbPlayerMotionShiftSet(playerNo, 13, 0.0f, 8.0f, 0);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 35), -1);
        mbWinTopWait();
        mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    } else {
        mbObjDispSet((s16)lbl_1_bss_4D4.motionModel, TRUE);
        mbObjPosSetV((s16)lbl_1_bss_4D4.motionModel, &lbl_1_data_6F4[0]);
        yaw = 40.0f;
        mbPlayerRotateStart(playerNo, 180, 15);
        while (!mbPlayerRotateCheck(playerNo)) {
            HuPrcVSleep();
        }
        mbObjMotionTimeSet((s16)lbl_1_bss_4D4.lateModel, 0.0f);
        mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.lateModel, 2.0f);
        entranceEnd = lbl_1_data_6F4[1];
        entranceEndPtr = &entranceEnd;
        entranceStart = lbl_1_data_6F4[0];
        entranceStartPtr = &entranceStart;
        fn_1_D9E0(lbl_1_bss_4D4.lateModel, lbl_1_bss_4D4.motionModel,
            yaw, 0.0f, entranceStartPtr, entranceEndPtr, 0);
        mbObjMotionTimeSet((s16)lbl_1_bss_4D4.lateModel, 120.0f);
        mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.lateModel, 2.0f);
        while (mbObjMotionTimeGet((s16)lbl_1_bss_4D4.lateModel) < 180.0f) {
            HuPrcVSleep();
        }
        mbObjMotionTimeSet((s16)lbl_1_bss_4D4.lateModel, 0.0f);
        mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.lateModel, 0.0f);
        mbObjMotionShiftSet((s16)lbl_1_bss_4D4.motionModel, 2, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        turnStep = 2.3333333f;
        for (index = 0; (float)index < 15.000001f; index++) {
            yaw -= turnStep;
            mbObjRotSet((s16)lbl_1_bss_4D4.motionModel, 0.0f, yaw, 0.0f);
            HuPrcVSleep();
        }
        mbObjMotionShiftSet((s16)lbl_1_bss_4D4.motionModel, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_12A14(0);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 31), speaker);
        mbWinTopWait();
        fn_1_12A14(1);
        sprintf(rewardText, "%d", rewardCount);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 32), speaker);
        mbWinTopInsertMesSet((u32)rewardText, 0);
        mbWinTopWait();
        fn_1_12A14(1);
        mbWinCreateChoice(2, MESSNUM(MESS_BOARD_W03, 33), speaker, 0);
        if (GwPlayer[playerNo].comF) {
            mbComChoiceUpSet();
        }
        mbWinTopWait();
        if (mbWinTopChoiceGet() == 1 || mbWinTopChoiceGet() == -1) {
            mbObjMotionShiftSet((s16)lbl_1_bss_4D4.motionModel, 4, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            fn_1_12A14(2);
            mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 34), speaker);
            mbWinTopWait();
            mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.lateModel, 2.0f);
            declineEnd = lbl_1_data_6F4[0];
            declineEndPtr = &declineEnd;
            declineStart = lbl_1_data_6F4[1];
            declineStartPtr = &declineStart;
            fn_1_D9E0(lbl_1_bss_4D4.lateModel, lbl_1_bss_4D4.motionModel,
                yaw, 130.0f, declineStartPtr, declineEndPtr, 1);
        } else {
            hiddenObject = mbRandMod(objectCount);
            mbPlayerMotionShiftSet(playerNo, 2, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            nextSpace = mbMasuTypeFindLink(landingSpace, 0);
            mbPlayerMasuMoveSpeed(playerNo, nextSpace, 20, FALSE);
            mbPlayerPosGet(playerNo, &playerPosition);
            mbObjPosGet((s16)lbl_1_bss_4D4.motionModel, &hostPosition);
            mbObjRotGet((s16)lbl_1_bss_4D4.motionModel, &hostRotation);
            PSVECSubtract(&playerPosition, &hostPosition, &initialDirection);
            for (index = 0; index < 20; index++) {
                mbPlayerPosGet(playerNo, &playerPosition);
                PSVECSubtract(&playerPosition, &hostPosition, &currentDirection);
                angleStart = atan2(initialDirection.z, initialDirection.x);
                angleNow = atan2(currentDirection.z, currentDirection.x);
                rotationStep = 180.0 * ((double)(angleNow - angleStart)
                    / 3.141592653589793);
                mbObjRotSet((s16)lbl_1_bss_4D4.motionModel, hostRotation.x,
                    hostRotation.y - rotationStep, hostRotation.z);
                HuPrcVSleep();
            }
            mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            fn_1_12A14(0);
            mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 36), speaker);
            mbWinTopWait();
            mbPlayerStarAdd(playerNo, -1);
            for (index = 0; index < rewardCount; index++) {
                starObjects[index] = mbStarObjCreate();
                starPositions[index].x = starPositions[index].y = starPositions[index].z = 0.0f;
                starRotations[index].x = starRotations[index].y = starRotations[index].z = 0.0f;
                starScales[index].x = starScales[index].y = starScales[index].z = 0.0f;
                if (index != 0) {
                    mbStarObjDispSet(starObjects[index], FALSE);
                } else {
                    mbPlayerPosGet(playerNo, &starPositions[index]);
                }
                mbStarObjRotSetV(starObjects[index], &starRotations[index]);
            }
            rotationStep = 12.0f;
            scaleStep = 0.016666668f;
            omVibrate(playerNo, 20, 7, 3);
            for (index = 0; index < 60U; index++) {
                starRotations[0].y += rotationStep;
                starScales[0].x += scaleStep;
                starScales[0].y += scaleStep;
                starScales[0].z += scaleStep;
                starPositions[0].y += 5.0f;
                mbStarObjPosSetV(starObjects[0], &starPositions[0]);
                mbStarObjRotSetV(starObjects[0], &starRotations[0]);
                mbStarObjScaleSetV(starObjects[0], &starScales[0]);
                HuPrcVSleep();
            }
            HuPrcSleep(30);
            for (index = 0; index < 60U; index++) {
                starRotations[0].y += rotationStep;
                starScales[0].x -= scaleStep;
                starScales[0].y -= scaleStep;
                starScales[0].z -= scaleStep;
                starPositions[0].y += 2.0f;
                mbStarObjPosSetV(starObjects[0], &starPositions[0]);
                mbStarObjRotSetV(starObjects[0], &starRotations[0]);
                mbStarObjScaleSetV(starObjects[0], &starScales[0]);
                HuPrcVSleep();
            }
            mbStarObjDispSet(starObjects[0], FALSE);
            HuPrcSleep(20);
            mbCameraFocusPlayerSet(-1);
            mbCameraMovePos(&lbl_1_data_718, NULL, NULL, 2500.0f, -1.0f, 30);
            mbCameraMoveWait();
            dropPosition = lbl_1_data_70C;
            dropPosition.y += 1000.0f;
            dropStep = 33.333332f;
            for (index = 0; index < objectCount; index++) {
                objectTimers[index] = index * 30;
                objectStates[index] = 0;
                mbObjPosSetV((s16)lbl_1_bss_4D4.carouselModels[index], &dropPosition);
                mbObjDispSet((s16)lbl_1_bss_4D4.carouselModels[index], TRUE);
                lbl_1_bss_468[index].position = dropPosition;
                if (boardIsDay()) {
                    lbl_1_bss_468[index].angle = 30.0f + 120.0f * (float)index;
                } else {
                    lbl_1_bss_468[index].angle = 120.0f + 60.0f * (float)index;
                }
                carouselOffsets[index].x = 300.0f * mbCosDeg(lbl_1_bss_468[index].angle);
                carouselOffsets[index].z = 300.0f * mbSinDeg(lbl_1_bss_468[index].angle);
                carouselSteps[index].x = carouselOffsets[index].x / 30.0f;
                carouselSteps[index].z = carouselOffsets[index].z / 30.0f;
            }
            objectsMoving = objectCount;
            while (objectsMoving != 0) {
                for (index = 0; index < objectCount; index++) {
                    switch (objectStates[index]) {
                    case 0:
                        if (--objectTimers[index] <= 0) {
                            objectTimers[index] = 0;
                            objectStates[index] = 1;
                        }
                        break;
                    case 1:
                        lbl_1_bss_468[index].position.y -= dropStep;
                        mbObjPosSetV((s16)lbl_1_bss_4D4.carouselModels[index],
                            &lbl_1_bss_468[index].position);
                        if (objectTimers[index]++ >= 30) {
                            soundHandles[index] = mbAudFXPlay(MSM_SE_W03_LOTTERY_OBJECT_MOVE);
                            lbl_1_bss_468[index].position.y = lbl_1_data_70C.y;
                            objectTimers[index] = 0;
                            objectStates[index] = 2;
                        }
                        break;
                    case 2:
                        lbl_1_bss_468[index].position.x += carouselSteps[index].x;
                        lbl_1_bss_468[index].position.z += carouselSteps[index].z;
                        if (objectTimers[index]++ >= 30) {
                            objectTimers[index] = 0;
                            objectStates[index] = 3;
                            objectsMoving--;
                        }
                        break;
                    case 3:
                        break;
                    }
                    mbObjPosSetV((s16)lbl_1_bss_4D4.carouselModels[index],
                        &lbl_1_bss_468[index].position);
                }
                HuPrcVSleep();
            }
            for (index = 0; index < objectCount; index++) {
                mbAudFXStop(soundHandles[index]);
            }
            mbStarObjDispSet(starObjects[0], TRUE);
            starPositions[0] = lbl_1_bss_468[hiddenObject].position;
            starPositions[0].y += 300.0f;
            for (index = 0; index < 60U; index++) {
                starRotations[0].y += rotationStep;
                starScales[0].x += scaleStep;
                starScales[0].y += scaleStep;
                starScales[0].z += scaleStep;
                mbStarObjPosSetV(starObjects[0], &starPositions[0]);
                mbStarObjRotSetV(starObjects[0], &starRotations[0]);
                mbStarObjScaleSetV(starObjects[0], &starScales[0]);
                HuPrcVSleep();
            }
            HuPrcSleep(30);
            for (index = 0; index < 60U; index++) {
                starRotations[0].y += rotationStep;
                starScales[0].x -= scaleStep;
                starScales[0].y -= scaleStep;
                starScales[0].z -= scaleStep;
                starPositions[0].y -= 5.0f;
                mbStarObjPosSetV(starObjects[0], &starPositions[0]);
                mbStarObjRotSetV(starObjects[0], &starRotations[0]);
                mbStarObjScaleSetV(starObjects[0], &starScales[0]);
                HuPrcVSleep();
            }
            mbStarObjDispSet(starObjects[0], FALSE);
            fn_1_12A14(1);
            mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 37), speaker);
            mbWinTopWait();
            fn_1_12A14(0);
            mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 38), speaker);
            mbWinTopWait();
            unusedShuffleCount = mbRandMod(3) + 2;
            shuffleDirection = mbRandMod(2);
            for (index = 0; index < objectCount; index++) {
            }
            lbl_1_bss_8 = 1;
            for (index = 0; index < mbRandMod(2) + 2; index++) {
                shuffleRepeats = mbRandMod(2) + 1;
                for (starIndex = 0; starIndex < shuffleRepeats; starIndex++) {
                    fn_1_8154(shuffleDirection);
                    shuffleDirection = mbRandMod(2);
                }
                swapRepeats = mbRandMod(6) + 3;
                for (starIndex = 0; starIndex < swapRepeats; starIndex++) {
                    fn_1_8758();
                }
            }
            fn_1_12A14(1);
            mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 39), speaker);
            mbWinTopWait();
            correctObject = hiddenObject;
            cursorSprite = espEntry(mbBoardDataNumGet(DATANUM(DATA_board, 1)), 2000, 0);
            espDispOn(cursorSprite);
            espDrawNoSet(cursorSprite, 32);
            espAttrSet(cursorSprite, 1);
            espBankSet(cursorSprite, 0);
            repeatDelay = 0;
            choice = mbRandMod(objectCount);
            cursorTarget = lbl_1_bss_468[lbl_1_bss_450[choice]].position;
            cursorTarget.x -= 100.0f;
            cursorTarget.y += 200.0f;
            Hu3D3Dto2D(&cursorTarget, 1, &screenPosition);
            espPosSet(cursorSprite, screenPosition.x, screenPosition.y);
            helpWindow = mbWinCreateHelp(MESSNUM(MESS_BOARD_W03, 43));
            mbWinPosSet(helpWindow, 228, 330);
            timerFrames = 600;
            timer = GameMesCreate(1, 10, -1, -1);
            HuSprGrpDrawNoSet(GameMesGet(timer)->grpId[0], 32);
            HuPrcVSleep();
            if (GwPlayer[playerNo].comF) {
                if (mbParticleRandF() <= 0.1f) {
                    comChoice = hiddenObject;
                } else {
                    comChoice = mbRandMod(objectCount);
                }
            }
            if (mbParticleRandF() >= 0.5f) {
                direction = 10;
            } else {
                direction = -10;
            }
            comDelay = mbRandMod(20) + 20;
            for (;;) {
                if (timer == -1) {
                    break;
                }
                if (!GwPlayer[playerNo].comF) {
                    if (repeatDelay <= 0) {
                        stickX = mbPadStkXGet(GwPlayer[playerNo].padNo);
                        stickY = mbPadStkYGet(GwPlayer[playerNo].padNo);
                        buttons = HuPadBtnDown[GwPlayer[playerNo].padNo];
                    } else {
                        stickX = stickY = buttons = 0;
                        repeatDelay--;
                    }
                    if (buttons & 0x100) {
                        break;
                    }
                } else if ((int)choice == (int)comChoice && repeatDelay <= 0 && comDelay <= 0) {
                    /* The CPU closes its choice without sending the A press used by a human
                     * player. */
                    buttons | PAD_BUTTON_A;
                    break;
                } else {
                    if (repeatDelay <= 0) {
                        if (comDelay-- <= 0) {
                            stickX = direction;
                            comDelay = mbRandMod(20) + 20;
                        }
                    } else {
                        stickX = stickY = buttons = 0;
                        repeatDelay--;
                    }
                }
                if (stickX < -8) {
                    repeatDelay = 16;
                    choice--;
                } else if (stickX > 8) {
                    repeatDelay = 16;
                    choice++;
                }
                if ((int)choice < 0) {
                    choice = objectCount - 1;
                } else if ((int)choice > objectCount - 1) {
                    choice = 0;
                }
                cursorTarget = lbl_1_bss_468[lbl_1_bss_450[choice]].position;
                cursorTarget.x -= 100.0f;
                cursorTarget.y += 200.0f;
                Hu3D3Dto2D(&cursorTarget, 1, &screenTarget);
                screenPosition.x += 0.1f * (screenTarget.x - screenPosition.x);
                screenPosition.y += 0.1f * (screenTarget.y - screenPosition.y);
                espPosSet(cursorSprite, screenPosition.x, screenPosition.y);
                if (timerFrames >= 0) {
                    timerFrames--;
                    if (timerFrames >= 0) {
                        GameMesDispSet(timer, 1, (s16)((timerFrames + 59) / 60));
                    } else {
                        GameMesDispSet(timer, 2, -1);
                        timer = -1;
                    }
                }
                HuPrcVSleep();
            }
            if (timer >= 0) {
                GameMesDispSet(timer, 2, -1);
                timer = -1;
            }
            espKill(cursorSprite);
            mbWinKill(helpWindow);
            if ((int)correctObject == lbl_1_bss_450[choice]) {
                starScales[0].x = starScales[0].y = starScales[0].z = 0.0f;
                starRotations[0].x = starRotations[0].y = starRotations[0].z = 0.0f;
                starPositions[0] = lbl_1_bss_468[hiddenObject].position;
                starPositions[0].y += 80.0f;
                mbStarObjDispSet(starObjects[0], TRUE);
                mbStarObjPosSetV(starObjects[0], &starPositions[0]);
                mbStarObjScaleSetV(starObjects[0], &starScales[0]);
                mbStarObjRotSetV(starObjects[0], &starRotations[0]);
            }
            scaleStep = 0.013333334f;
            mbAudFXPlay(MSM_SE_W03_STAR_REWARD);
            for (index = 0; index < 60U; index++) {
                starScales[0].x += scaleStep;
                starScales[0].y += scaleStep;
                starScales[0].z += scaleStep;
                mbStarObjScaleSetV(starObjects[0], &starScales[0]);
                lbl_1_bss_468[lbl_1_bss_450[choice]].position.y += 5.0f;
                mbObjPosSetV((s16)lbl_1_bss_4D4.carouselModels[lbl_1_bss_450[choice]],
                    &lbl_1_bss_468[lbl_1_bss_450[choice]].position);
                HuPrcVSleep();
            }
            HuPrcSleep(10);
            if ((int)correctObject == lbl_1_bss_450[choice]) {
                for (index = 0; index < rewardCount; index++) {
                    if (index != 0) {
                        starPositions[index] = starPositions[0];
                        starScales[index] = starScales[0];
                    }
                    if (boardIsDay()) {
                        starSteps[index].x =
                            (float) ((((double) lbl_1_bss_468[lbl_1_bss_450[choice]].position.x -
                                       100.0 * mbCosDeg(180.0f * (float) index)) -
                                      lbl_1_bss_468[lbl_1_bss_450[choice]].position.x) /
                                     50.0);
                    } else {
                        starSteps[index].x = ((lbl_1_bss_468[lbl_1_bss_450[choice]].position.x
                            - 100.0f * (1.5f * mbCosDeg(90.0f * (float)index)))
                            - lbl_1_bss_468[lbl_1_bss_450[choice]].position.x) / 50.0f;
                    }
                    mbStarObjDispSet(starObjects[index], TRUE);
                    mbStarObjPosSetV(starObjects[index], &starPositions[index]);
                    mbStarObjScaleSetV(starObjects[index], &starScales[index]);
                    mbStarObjRotSetV(starObjects[index], &starRotations[index]);
                }
                for (index = 0; (float)index < 50.0f; index++) {
                    for (starIndex = 0; starIndex < rewardCount; starIndex++) {
                        starPositions[starIndex].x += starSteps[starIndex].x;
                        mbStarObjPosSetV(starObjects[starIndex], &starPositions[starIndex]);
                    }
                    HuPrcVSleep();
                }
                fn_1_12A14(0);
                mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 40), speaker);
                mbWinTopWait();
                rotationStep = 12.0f;
                scaleStep = 0.013333334f;
                for (index = 0; index < 60U; index++) {
                    for (starIndex = 0; starIndex < rewardCount; starIndex++) {
                        starRotations[starIndex].y += rotationStep;
                        starScales[starIndex].x -= scaleStep;
                        starScales[starIndex].y -= scaleStep;
                        starScales[starIndex].z -= scaleStep;
                        mbStarObjRotSetV(starObjects[starIndex], &starRotations[starIndex]);
                        mbStarObjScaleSetV(starObjects[starIndex], &starScales[starIndex]);
                    }
                    HuPrcVSleep();
                }
                for (index = 0; index < rewardCount; index++) {
                    mbStarObjDispFlagSet(starObjects[index], FALSE);
                }
                HuPrcSleep(20);
                for (index = 0; index < rewardCount; index++) {
                    mbStarObjDispSet(starObjects[index], FALSE);
                }
                HuPrcSleep(10);
                mbPlayerRotateStart(playerNo, 0, 15);
                while (!mbPlayerRotateCheck(playerNo)) {
                    HuPrcVSleep();
                }
                mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 1.0f, HU3D_MOTATTR_LOOP);
                mbCameraPlayerViewSet(playerNo, 2);
                for (index = 0; index < rewardCount; index++) {
                    mbStarObjDispSet(starObjects[index], TRUE);
                }
                for (index = 0; index < rewardCount; index++) {
                    mbPlayerPosGet(playerNo, &starPositions[index]);
                    starAngles[index] = 30.0f + 120.0f * (float)index;
                    starOffsets[index].x = 200.0f * mbCosDeg(starAngles[index]);
                    starOffsets[index].z = 200.0f * mbSinDeg(starAngles[index]);
                    starOffsets[index].y = 300.0f;
                    starPositions[index].x += starOffsets[index].x;
                    starPositions[index].y += starOffsets[index].y;
                    starPositions[index].z += starOffsets[index].z;
                    mbStarObjPosSetV(starObjects[index], &starPositions[index]);
                    starDelays[index] = index * 30;
                }
                rotationStep = 12.0f;
                scaleStep = 0.016666668f;
                for (index = 0; index < 60U; index++) {
                    for (starIndex = 0; starIndex < rewardCount; starIndex++) {
                        starRotations[starIndex].y -= rotationStep;
                        starScales[starIndex].x += scaleStep;
                        starScales[starIndex].y += scaleStep;
                        starScales[starIndex].z += scaleStep;
                        mbStarObjRotSetV(starObjects[starIndex], &starRotations[starIndex]);
                        mbStarObjScaleSetV(starObjects[starIndex], &starScales[starIndex]);
                    }
                    HuPrcVSleep();
                }
                HuPrcSleep(30);
                turnStep = 9.0f;
                mbPlayerPosGet(playerNo, &playerPosition);
                for (index = 0; index < 60U; index++) {
                    for (starIndex = 0; starIndex < rewardCount; starIndex++) {
                        starRotations[starIndex].y -= rotationStep;
                        starScales[starIndex].x -= scaleStep;
                        starScales[starIndex].y -= scaleStep;
                        starScales[starIndex].z -= scaleStep;
                        mbStarObjRotSetV(starObjects[starIndex], &starRotations[starIndex]);
                        mbStarObjScaleSetV(starObjects[starIndex], &starScales[starIndex]);
                    }
                    HuPrcVSleep();
                }
                omVibrate(playerNo, 20, 20, 0);
                for (index = 0; index < rewardCount; index++) {
                    mbStarObjDispFlagSet(starObjects[index], FALSE);
                }
                HuPrcSleep(20);
                for (index = 0; index < rewardCount; index++) {
                    mbStarObjDispSet(starObjects[index], FALSE);
                }
                mbStarAddExec(playerNo, rewardCount);
                starDisplay = mbStarDispPlayerCreate(playerNo, rewardCount);
                mbPlayerWinLoseVoicePlay(playerNo, 12, CHARVOICEID(6));
                mbPlayerMotionShiftSet(playerNo, 12, 0.0f, 8.0f, 0);
                while (!mbPlayerMotionEndCheck(playerNo) && !mbStarDispCheck(starDisplay)) {
                    HuPrcVSleep();
                }
            } else {
                fn_1_12A14(2);
                mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 41), speaker);
                mbWinTopWait();
                scaleStep = 0.026666667f;
                for (index = 0; index < 30U; index++) {
                    mbStarObjScaleSetV(starObjects[0], &starScales[0]);
                    lbl_1_bss_468[lbl_1_bss_450[choice]].position.y -= 10.0f;
                    mbObjPosSetV((s16)lbl_1_bss_4D4.carouselModels[lbl_1_bss_450[choice]],
                        &lbl_1_bss_468[lbl_1_bss_450[choice]].position);
                    HuPrcVSleep();
                }
                HuPrcSleep(30);
                fn_1_12A14(0);
                mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 44), speaker);
                mbWinTopWait();
                starScales[0].x = starScales[0].y = starScales[0].z = 0.0f;
                starRotations[0].x = starRotations[0].y = starRotations[0].z = 0.0f;
                starPositions[0] = lbl_1_bss_468[hiddenObject].position;
                starPositions[0].y += 80.0f;
                mbStarObjDispSet(starObjects[0], TRUE);
                mbStarObjPosSetV(starObjects[0], &starPositions[0]);
                mbStarObjScaleSetV(starObjects[0], &starScales[0]);
                mbStarObjRotSetV(starObjects[0], &starRotations[0]);
                scaleStep = 0.013333334f;
                mbAudFXPlay(MSM_SE_W03_STAR_REWARD);
                for (index = 0; index < 60U; index++) {
                    starScales[0].x += scaleStep;
                    starScales[0].y += scaleStep;
                    starScales[0].z += scaleStep;
                    mbStarObjScaleSetV(starObjects[0], &starScales[0]);
                    lbl_1_bss_468[hiddenObject].position.y += 5.0f;
                    mbObjPosSetV((s16)lbl_1_bss_4D4.carouselModels[hiddenObject],
                        &lbl_1_bss_468[hiddenObject].position);
                    HuPrcVSleep();
                }
                mbPlayerWinLoseVoicePlay(playerNo, 13, CHARVOICEID(12));
                mbPlayerMotionShiftSet(playerNo, 13, 0.0f, 8.0f, 0);
                while (!mbPlayerMotionEndCheck(playerNo)) {
                    HuPrcVSleep();
                }
                HuPrcSleep(20);
                scaleStep = 0.026666667f;
                for (index = 0; index < 30U; index++) {
                    starScales[0].x -= scaleStep;
                    starScales[0].y -= scaleStep;
                    starScales[0].z -= scaleStep;
                    mbStarObjScaleSetV(starObjects[0], &starScales[0]);
                    lbl_1_bss_468[hiddenObject].position.y -= 10.0f;
                    mbObjPosSetV((s16)lbl_1_bss_4D4.carouselModels[hiddenObject],
                        &lbl_1_bss_468[hiddenObject].position);
                    HuPrcVSleep();
                }
                mbStarObjDispFlagSet(starObjects[0], FALSE);
                HuPrcSleep(60);
            }
            mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            mbObjMotionShiftSet((s16)lbl_1_bss_4D4.motionModel, 4, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            fn_1_12A14(0);
            mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 42), speaker);
            mbWinTopWait();
            mbObjRotGet((s16)lbl_1_bss_4D4.motionModel, &hostRotation);
            mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.lateModel, 2.0f);
            exitEnd = lbl_1_data_6F4[0];
            exitEndPtr = &exitEnd;
            exitStart = lbl_1_data_6F4[1];
            exitStartPtr = &exitStart;
            fn_1_D9E0(lbl_1_bss_4D4.lateModel, lbl_1_bss_4D4.motionModel,
                hostRotation.y, -170.0f, exitStartPtr, exitEndPtr, 1);
            mbWipeFadeOut();
            for (index = 0; index < objectCount; index++) {
                mbObjDispSet((s16)lbl_1_bss_4D4.carouselModels[index], FALSE);
            }
            for (index = 0; index < rewardCount; index++) {
                mbStarObjKill(starObjects[index]);
            }
            mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 1.0f, HU3D_MOTATTR_LOOP);
            mbCameraPlayerViewSetFast(playerNo, 2);
            mbCameraFocusPlayerSet(playerNo);
            mbMasuPosGet(landingSpace, &landingPosition);
            GwPlayer[playerNo].masuId = landingSpace;
            mbPlayerPosSetV(playerNo, &landingPosition);
            mbWipeFadeIn();
            for (index = 0; index < objectCount; index++) {
            }
        }
    }
    mbMoveNumDispSet(playerNo, TRUE);
    mbObjDispSet((s16)lbl_1_bss_4D4.motionModel, FALSE);
}

/* Moves the board's orbiting objects around their shared center. */

HuVecF lbl_1_data_724 = {4.58e+03f, 6.4e+02f, -1.6e+02f};
/* Spins three daytime objects or six nighttime objects, then slows them to a stop. */
void fn_1_8154(s32 reverse)
{
    HuVecF offsets[6];
    s32 soundHandles[6];
    f32 speed;
    f32 angularStep;
    f32 frames;
    s32 objectCount;
    s32 objectIndex;

    if (GwSystem.curTime == 0) {
        objectCount = 3;
    } else {
        objectCount = 6;
    }

    for (objectIndex = 0; objectIndex < objectCount; objectIndex++) {
        soundHandles[objectIndex] = mbAudFXPlay(MSM_SE_W03_LOTTERY_OBJECT_LAUNCH);
    }

    speed = 0.1f;
    frames = (f32)(mbRandMod(45) + 60);
    angularStep = 4.5f;
    while (frames) {
        lbl_1_bss_8 = 1;
        for (objectIndex = 0; objectIndex < objectCount; objectIndex++) {
            if (reverse == 0) {
                lbl_1_bss_468[objectIndex].angle += angularStep * speed;
                if (lbl_1_bss_468[objectIndex].angle > 360.0f) {
                    lbl_1_bss_468[objectIndex].angle -= 360.0f;
                }
            } else if (reverse != 0) {
                lbl_1_bss_468[objectIndex].angle -= angularStep * speed;
                if (lbl_1_bss_468[objectIndex].angle <= 0.0f) {
                    lbl_1_bss_468[objectIndex].angle += 360.0f;
                }
            }
            offsets[objectIndex].x = 300.0f * mbCosDeg(lbl_1_bss_468[objectIndex].angle);
            offsets[objectIndex].z = 300.0f * mbSinDeg(lbl_1_bss_468[objectIndex].angle);
            lbl_1_bss_468[objectIndex].position.x = lbl_1_data_724.x + offsets[objectIndex].x;
            lbl_1_bss_468[objectIndex].position.z = lbl_1_data_724.z + offsets[objectIndex].z;
            mbObjPosSetV((s16)lbl_1_bss_4D4.carouselModels[objectIndex],
                &lbl_1_bss_468[objectIndex].position);
        }
        speed += 0.05;
        if (speed >= 3.0f) {
            speed = 3.0f;
        }
        frames -= 1.0f;
        HuPrcVSleep();
    }

    while (speed) {
        for (objectIndex = 0; objectIndex < objectCount; objectIndex++) {
            if (reverse == 0) {
                lbl_1_bss_468[objectIndex].angle += angularStep * speed;
                if (lbl_1_bss_468[objectIndex].angle > 360.0f) {
                    lbl_1_bss_468[objectIndex].angle -= 360.0f;
                }
            } else if (reverse != 0) {
                lbl_1_bss_468[objectIndex].angle -= angularStep * speed;
                if (lbl_1_bss_468[objectIndex].angle <= 0.0f) {
                    lbl_1_bss_468[objectIndex].angle += 360.0f;
                }
            }
            offsets[objectIndex].x = 300.0f * mbCosDeg(lbl_1_bss_468[objectIndex].angle);
            offsets[objectIndex].z = 300.0f * mbSinDeg(lbl_1_bss_468[objectIndex].angle);
            lbl_1_bss_468[objectIndex].position.x = lbl_1_data_724.x + offsets[objectIndex].x;
            lbl_1_bss_468[objectIndex].position.z = lbl_1_data_724.z + offsets[objectIndex].z;
            mbObjPosSetV((s16)lbl_1_bss_4D4.carouselModels[objectIndex],
                &lbl_1_bss_468[objectIndex].position);
        }
        speed -= 0.05;
        if (speed < 0.0f) {
            speed = 0.0f;
        }
        lbl_1_bss_8 = 0;
        HuPrcVSleep();
    }

    for (objectIndex = 0; objectIndex < objectCount; objectIndex++) {
        mbAudFXStop(soundHandles[objectIndex]);
    }
}

s32 lbl_1_data_730[3][2] = {{0, 1}, {0, 2}, {1, 2}};
s32 lbl_1_data_748[3][2] = {{0, 3}, {1, 4}, {2, 5}};
/* Exchanges a random pair of orbiting objects along opposing curved paths. */
void fn_1_8758(void)
{
    HuVecF secondDisplacement;
    HuVecF firstDisplacement;
    f32 duration;
    f32 progress;
    f32 savedAngle;
    s32 savedObject;
    s32 firstObject;
    s32 secondObject;
    s32 frame;
    u32 selection;

    selection = mbRandMod(3);
    if (GwSystem.curTime == 0) {
        firstObject = lbl_1_data_730[selection][0];
        secondObject = lbl_1_data_730[selection][1];
        duration = 15.0f;
    } else {
        firstObject = lbl_1_data_748[selection][0];
        secondObject = lbl_1_data_748[selection][1];
        duration = 12.0f;
    }
    firstDisplacement.x = lbl_1_bss_468[secondObject].position.x -
        lbl_1_bss_468[firstObject].position.x;
    firstDisplacement.y = lbl_1_bss_468[secondObject].position.y -
        lbl_1_bss_468[firstObject].position.y;
    firstDisplacement.z = lbl_1_bss_468[secondObject].position.z -
        lbl_1_bss_468[firstObject].position.z;
    secondDisplacement.x = lbl_1_bss_468[firstObject].position.x -
        lbl_1_bss_468[secondObject].position.x;
    secondDisplacement.y = lbl_1_bss_468[firstObject].position.y -
        lbl_1_bss_468[secondObject].position.y;
    secondDisplacement.z = lbl_1_bss_468[firstObject].position.z -
        lbl_1_bss_468[secondObject].position.z;
    mbAudFXPlay(MSM_SE_W03_LOTTERY_OBJECT_MOVE);
    for (frame = 0; (f32)frame < duration; frame++) {
        progress = (f32)frame / duration;
        mbObjPosSet((s16)lbl_1_bss_4D4.carouselModels[firstObject],
            lbl_1_bss_468[firstObject].position.x +
                firstDisplacement.x * mbSinDeg(90.0f * progress) +
                (1.2f * (100.0f * mbSinDeg(180.0f * progress))) *
                    fabs(mbSinDeg(lbl_1_bss_468[firstObject].angle)),
            lbl_1_bss_468[firstObject].position.y +
                firstDisplacement.y * mbSinDeg(90.0f * progress),
            lbl_1_bss_468[firstObject].position.z +
                firstDisplacement.z * mbSinDeg(90.0f * progress) +
                (1.2f * (100.0f * mbSinDeg(180.0f * progress))) *
                    fabs(mbCosDeg(lbl_1_bss_468[firstObject].angle)));
        mbObjPosSet((s16)lbl_1_bss_4D4.carouselModels[secondObject],
            lbl_1_bss_468[secondObject].position.x +
                secondDisplacement.x * mbSinDeg(90.0f * progress) +
                (1.2f * (100.0f * mbCosDeg(90.0f + 180.0f * progress))) *
                    fabs(mbSinDeg(lbl_1_bss_468[secondObject].angle)),
            lbl_1_bss_468[secondObject].position.y +
                secondDisplacement.y * mbSinDeg(90.0f * progress),
            lbl_1_bss_468[secondObject].position.z +
                secondDisplacement.z * mbSinDeg(90.0f * progress) +
                (1.2f * (100.0f * mbCosDeg(90.0f + 180.0f * progress))) *
                    fabs(mbCosDeg(lbl_1_bss_468[secondObject].angle)));
        HuPrcVSleep();
    }
    lbl_1_bss_468[firstObject].position.x += firstDisplacement.x;
    lbl_1_bss_468[firstObject].position.z += firstDisplacement.z;
    lbl_1_bss_468[secondObject].position.x += secondDisplacement.x;
    lbl_1_bss_468[secondObject].position.z += secondDisplacement.z;
    mbObjPosSetV((s16)lbl_1_bss_4D4.carouselModels[firstObject],
        &lbl_1_bss_468[firstObject].position);
    mbObjPosSetV((s16)lbl_1_bss_4D4.carouselModels[secondObject],
        &lbl_1_bss_468[secondObject].position);
    savedObject = lbl_1_bss_450[firstObject];
    lbl_1_bss_450[firstObject] = lbl_1_bss_450[secondObject];
    lbl_1_bss_450[secondObject] = savedObject;
    savedAngle = lbl_1_bss_468[firstObject].angle;
    lbl_1_bss_468[firstObject].angle = lbl_1_bss_468[secondObject].angle;
    lbl_1_bss_468[secondObject].angle = savedAngle;
}

/* Runs the board's four-player coin or star lottery and its winning animation. */

u32 lbl_1_data_760[4] = {128, 256, 512, 1024};
float lbl_1_data_770[4] = {1.0f, 1.0f, 0.9f, 0.7f};
float lbl_1_data_780[4] = {-1.5f, -0.5f, 0.5f, 1.5f};
HuVecF lbl_1_data_790 = {-6.8f, 1619.0f, -1623.0f};
HuVecF lbl_1_data_79C = {-1e+01f, 0.0f, 0.0f};
HuVecF lbl_1_data_7A8[2] = {{0.0f, 1.52e+03f, -2.75e+03f}, {0.0f, 1.52e+03f, -2.54e+03f}};
HuVecF lbl_1_data_7C0 = {-6.8f, 1876.6f, -1668.4f};
u32 lbl_1_data_7CC[3] = {MESSNUM(MESS_BOARD_W03, 18), MESSNUM(MESS_BOARD_W03, 19),
                         MESSNUM(MESS_BOARD_W03, 20)};
HuVecF lbl_1_data_7D8[4] = { { -583.0f, 2306.0f, -2215.0f },
                             { -206.0f, 2306.0f, -2215.0f },
                             { 209.0f, 2306.0f, -2215.0f },
                             { 616.0f, 2306.0f, -2215.0f } };
/* Offers the lottery to the landing player, then lets each player choose a position. */
void fn_1_8F60(int playerNo, s16 landingSpace)
{
    HuVecF choicePositions[MASU_LINK_MAX];
    int outcomes[4] = { 0, 0, 0, 0 };
    int coinPayments[4];
    int starPayments[4];
    s16 choiceSpaces[MASU_LINK_MAX];
    int timerFrames;
    int swap;
    u32 randomChoice;
    int messageIndex;
    s32 readId;
    int totalCoins;
    int totalStars;
    int lastChoice = 3;
    int firstChoice = 0;
    int stickY;
    int buttons;
    int comMoves;
    int comDelay;
    int assigned = 0;
    int occupied[4] = { 0, 0, 0, 0 };
    HuVecF cursorPosition;
    HuVecF screenPosition;
    HuVecF landingPosition;
    float stopTimes[4];
    float motionSpeeds[4];
    int chosenPlayers[4];
    int playerOrder[4];
    int ranks[4];
    HuVecF entranceStart;
    HuVecF entranceEnd;
    HuVecF exitStart;
    HuVecF exitEnd;
    HuVecF coinPosition;
    HuVecF starPosition;
    s16 originalSpaces[4];
    int teamFirst[2];
    int teamSecond[2];
    int finished;
    int helpWindow;
    MASU *nextSpace;
    int soundId;
    HuVecF *entranceEndPtr;
    HuVecF *entranceStartPtr;
    HuVecF *exitEndPtr;
    HuVecF *exitStartPtr;
    HuVecF *coinPositionPtr;
    HuVecF *starPositionPtr;
    s16 foundSpace;
    s16 messageWindow;
    int cursorSprite;
    int winner;
    int prizeCount;
    int speaker;
    int selectedPlayer;
    int chosen;
    s16 timer;
    int repeatDelay;
    int stickX;
    int index;
    int scan;
    float yaw;
    float maxTime;

    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    if (boardIsDay()) {
        totalCoins = 0;
        for (index = 0; index < 4; ++index) {
            totalCoins += mbPlayerCoinGet(index);
            coinPayments[index] = 0;
        }
        if (totalCoins == 0) {
            mbPlayerRotateStart(playerNo, 0, 15);
            while (!mbPlayerRotateCheck(playerNo)) {
                HuPrcVSleep();
            }
            mbPlayerWinLoseVoicePlay(playerNo, 13, CHARVOICEID(12));
            mbPlayerMotionShiftSet(playerNo, 13, 0.0f, 8.0f, 0);
            mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 28), -1);
            mbWinTopWait();
            mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            goto finish;
        }
    } else {
        totalStars = 0;
        for (index = 0; index < 4; ++index) {
            totalStars += mbPlayerStarGet(index);
            starPayments[index] = 0;
        }
        if (totalStars == 0) {
            mbPlayerRotateStart(playerNo, 0, 15);
            while (!mbPlayerRotateCheck(playerNo)) {
                HuPrcVSleep();
            }
            mbPlayerWinLoseVoicePlay(playerNo, 13, CHARVOICEID(12));
            mbPlayerMotionShiftSet(playerNo, 13, 0.0f, 8.0f, 0);
            mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 29), -1);
            mbWinTopWait();
            mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            goto finish;
        }
    }
    for (index = 0; index < 4; ++index) {
        ranks[index] = GwPlayer[index].rank;
        playerOrder[index] = index;
    }
    for (index = 0; index < 4; ++index) {
        for (scan = index + 1; scan < 4; ++scan) {
            if (ranks[index] < ranks[scan]) {
                swap = ranks[index];
                ranks[index] = ranks[scan];
                ranks[scan] = swap;
                swap = playerOrder[index];
                playerOrder[index] = playerOrder[scan];
                playerOrder[scan] = swap;
            } else if (ranks[index] == ranks[scan] && playerOrder[index] > playerOrder[scan]) {
                swap = playerOrder[index];
                playerOrder[index] = playerOrder[scan];
                playerOrder[scan] = swap;
            }
        }
    }
    /* The landing player chooses first, followed by descending rank values. */
    for (index = 0; index < 4; ++index) {
        if (index > 0 && playerNo == playerOrder[index]) {
            for (scan = index - 1; scan > -1; --scan) {
                playerOrder[scan + 1] = playerOrder[scan];
            }
            playerOrder[0] = playerNo;
            break;
        }
    }
    if (boardIsDay()) {
        speaker = 8;
    } else {
        speaker = 9;
    }
    mbObjDispSet((s16)lbl_1_bss_4D4.motionModel, TRUE);
    mbObjPosSetV((s16)lbl_1_bss_4D4.motionModel, &lbl_1_data_7A8[0]);
    yaw = 0.0f;
    for (index = 0; index < 4; ++index) {
        mbPlayerColSnapPlayerSet(index, FALSE);
        originalSpaces[index] = GwPlayer[index].masuId;
    }
    for (index = 0; index < 4; ++index) {
        mbMasuMAttrListGet(lbl_1_data_760[index], &foundSpace);
        choiceSpaces[index] = foundSpace;
        mbMasuPosGet(choiceSpaces[index], &choicePositions[index]);
        choicePositions[index].y += 200.0f;
        if (index == 2) {
            choicePositions[index].x -= 50.0f;
        } else if (index == 3) {
            choicePositions[index].x -= 100.0f;
        }
    }
    for (index = 0; index < 4; ++index) {
        while (assigned == 0) {
            randomChoice = mbRandMod(4);
            if (outcomes[randomChoice] == 0) {
                outcomes[randomChoice] = index + 1;
                assigned = 1;
            }
        }
        assigned = 0;
    }
    readId = mbBGRead(DATA_effect);
    if (readId != -1) {
        mbBGReadWait(readId);
    }
    mbPlayerRotateStart(playerNo, 180, 15);
    while (!mbPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    mbCameraFocusPlayerSet(-1);
    mbCameraMovePos(&lbl_1_data_790, &lbl_1_data_79C, NULL, 2000.0f, -1.0f, 30);
    mbCameraMoveWait();
    mbObjMotionTimeSet((s16)lbl_1_bss_4D4.finalModels[0], 0.0f);
    mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.finalModels[0], 2.0f);
    mbObjRotSet((s16)lbl_1_bss_4D4.motionModel, 0.0f, 0.0f, 0.0f);
    entranceEnd = lbl_1_data_7A8[1];
    entranceEndPtr = &entranceEnd;
    entranceStart = lbl_1_data_7A8[0];
    entranceStartPtr = &entranceStart;
    fn_1_D9E0(lbl_1_bss_4D4.finalModels[0], lbl_1_bss_4D4.motionModel,
        yaw, 0.0f, entranceStartPtr, entranceEndPtr, 0);
    mbObjMotionTimeSet((s16)lbl_1_bss_4D4.finalModels[0], 120.0f);
    mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.finalModels[0], 2.0f);
    while (mbObjMotionTimeGet((s16)lbl_1_bss_4D4.finalModels[0]) < 180.0f) {
        HuPrcVSleep();
    }
    mbObjMotionTimeSet((s16)lbl_1_bss_4D4.finalModels[0], 0.0f);
    mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.finalModels[0], 0.0f);
    fn_1_12A14(0);
    mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 12), speaker);
    mbWinTopWait();
    fn_1_12A14(0);
    mbWinCreateChoice(0, MESSNUM(MESS_BOARD_W03, 13), speaker, 0);
    if (GwPlayer[playerNo].comF) {
        mbComChoiceUpSet();
    }
    mbWinTopWait();
    if (mbWinTopChoiceGet() == 1 || mbWinTopChoiceGet() == -1) {
        mbObjMotionShiftSet((s16)lbl_1_bss_4D4.motionModel, 4, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_12A14(2);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 14), speaker);
        mbWinTopWait();
        mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.finalModels[0], 2.0f);
        exitEnd = lbl_1_data_7A8[0];
        exitEndPtr = &exitEnd;
        exitStart = lbl_1_data_7A8[1];
        exitStartPtr = &exitStart;
        fn_1_D9E0(lbl_1_bss_4D4.finalModels[0], lbl_1_bss_4D4.motionModel,
            yaw, 180.0f, exitStartPtr, exitEndPtr, 1);
        mbCameraFocusPlayerSet(playerNo);
        mbCameraPlayerViewSet(playerNo, 2);
    } else {
        fn_1_12A14(0);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 15), speaker);
        mbWinTopWait();
        HuPrcSleep(30);
        mbWipeSpecialFadeInCreate(1, 60);
        mbMasuPosGet(landingSpace, &landingPosition);
        for (index = 3; index > -1; --index) {
            mbPlayerPosSet(index, landingPosition.x + 100.0f * lbl_1_data_780[index],
                landingPosition.y, landingPosition.z);
            mbPlayerRotYSet(index, 180.0f);
            mbPlayerMotionShiftSet(index, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        }
        mbWipeSpecialFadeOutCreate(1, 60);
        HuPrcVSleep();
        prizeCount = 0;
        if (boardIsDay()) {
            fn_1_12A14(1);
            mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 16), speaker);
            mbWinTopWait();
            if (GWTeamFGet()) {
                for (index = 0; index < 2; ++index) {
                    teamFirst[index] = mbPlayerTeamFindPlayer(index, 0);
                    teamSecond[index] = mbPlayerTeamFindPlayer(index, 1);
                    if (mbPlayerCoinGet(teamFirst[index]) < 20) {
                        coinPayments[teamFirst[index]] = mbPlayerCoinGet(teamFirst[index]);
                    } else {
                        coinPayments[teamFirst[index]] = 20;
                    }
                    prizeCount += coinPayments[teamFirst[index]];
                    coinPayments[teamSecond[index]] = 0;
                }
            } else {
                for (index = 0; index < 4; ++index) {
                    if (mbPlayerCoinGet(index) < 10) {
                        coinPayments[index] = mbPlayerCoinGet(index);
                    } else {
                        coinPayments[index] = 10;
                    }
                    prizeCount += coinPayments[index];
                }
            }
            mbCoinAddAllExec(-coinPayments[0], -coinPayments[1],
                -coinPayments[2], -coinPayments[3]);
            if (prizeCount < 40) {
                fn_1_12A14(1);
                mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 17), speaker);
                mbWinTopWait();
            }
        } else {
            fn_1_12A14(1);
            mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 26), speaker);
            mbWinTopWait();
            if (GWTeamFGet()) {
                for (index = 0; index < 2; ++index) {
                    teamFirst[index] = mbPlayerTeamFindPlayer(index, 0);
                    teamSecond[index] = mbPlayerTeamFindPlayer(index, 1);
                    if (mbPlayerStarGet(teamFirst[index]) >= 2) {
                        starPayments[teamFirst[index]] = 2;
                    } else {
                        starPayments[teamFirst[index]] = mbPlayerStarGet(teamFirst[index]);
                    }
                    prizeCount += starPayments[teamFirst[index]];
                    starPayments[teamSecond[index]] = 0;
                }
            } else {
                for (index = 0; index < 4; ++index) {
                    if (mbPlayerStarGet(index) >= 1) {
                        starPayments[index] = 1;
                    } else {
                        starPayments[index] = 0;
                    }
                    prizeCount += starPayments[index];
                }
            }
            mbStarAddAllExec(-starPayments[0], -starPayments[1],
                -starPayments[2], -starPayments[3]);
            if (prizeCount < 4) {
                fn_1_12A14(1);
                mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 17), speaker);
                mbWinTopWait();
            }
        }
            cursorSprite = espEntry(mbBoardDataNumGet(DATANUM(DATA_board, 1)), 2000, 0);
        espDispOff(cursorSprite);
        espDrawNoSet(cursorSprite, 32);
        espAttrSet(cursorSprite, 1);
        espBankSet(cursorSprite, 0);
        HuPrcVSleep();
        index = 0;
        messageIndex = 0;
        for (; index < 3; ++index) {
            selectedPlayer = playerOrder[index];
            omVibrate(selectedPlayer, 20, 7, 3);
            fn_1_12A14(1);
            messageWindow = mbWinCreate(2, lbl_1_data_7CC[messageIndex++], speaker);
            mbWinTopInsertMesSet(mbPlayerNameMesGet(selectedPlayer), 0);
            mbWinPlayerDisable(messageWindow, selectedPlayer);
            mbWinTopWait();
            for (scan = 0; scan < 4; ++scan) {
                if (occupied[scan] == 0) {
                    lastChoice = scan;
                }
            }
            for (scan = 3; scan > -1; --scan) {
                if (occupied[scan] == 0) {
                    firstChoice = scan;
                }
            }
            chosen = firstChoice;
            repeatDelay = 0;
            timerFrames = 300;
            timer = GameMesCreate(1, 5, -1, -1);
            HuSprGrpDrawNoSet(GameMesGet(timer)->grpId[0], 32);
            espDispOn(cursorSprite);
            Hu3D3Dto2D(&choicePositions[chosen], 1, &cursorPosition);
            espPosSet(cursorSprite, cursorPosition.x, cursorPosition.y);
            comDelay = mbRandMod(20) + 30;
            comMoves = 0;
            helpWindow = mbWinCreateHelp(MESSNUM(MESS_BOARD_W03, 30));
            mbWinPosSet(helpWindow, 176, 300);
            while (1) {
                if (timer == -1) {
                    break;
                }
                if (!GwPlayer[selectedPlayer].comF) {
                    if (repeatDelay <= 0) {
                        stickX = mbPadStkXGet(GwPlayer[selectedPlayer].padNo);
                        stickY = mbPadStkYGet(GwPlayer[selectedPlayer].padNo);
                        buttons = HuPadBtnDown[GwPlayer[selectedPlayer].padNo];
                    } else {
                        stickX = stickY = buttons = 0;
                        --repeatDelay;
                    }
                    if (buttons & 0x100) {
                        break;
                    }
                } else if (repeatDelay <= 0 && comDelay-- <= 0) {
                    if (mbParticleRandF() >= 0.5f && comMoves < 3) {
                        comDelay = mbRandMod(20) + 20;
                        ++comMoves;
                        if (chosen <= firstChoice) {
                            stickX = 10;
                        } else if (chosen >= lastChoice) {
                            stickX = -10;
                        } else if (mbParticleRandF() >= 0.3f) {
                            stickX = 10;
                        } else {
                            stickX = -10;
                        }
                    } else {
                        /* The CPU closes its choice without sending a human player's confirmation
                         * press. */
                        buttons | 0x100;
                        break;
                    }
                } else {
                    stickX = stickY = buttons = 0;
                    --repeatDelay;
                }
                if (stickX < -8 && chosen > 0) {
                    for (scan = chosen - 1; scan > firstChoice - 1; --scan) {
                        if (occupied[scan] == 0) {
                            chosen = scan;
                            repeatDelay = 16;
                            break;
                        }
                    }
                } else if (stickX > 8 && chosen < 3) {
                    for (scan = chosen + 1; scan < lastChoice + 1; ++scan) {
                        if (occupied[scan] == 0) {
                            chosen = scan;
                            repeatDelay = 16;
                            break;
                        }
                    }
                }
                if (chosen < firstChoice) {
                    chosen = firstChoice;
                } else if (chosen > lastChoice) {
                    chosen = lastChoice;
                }
                Hu3D3Dto2D(&choicePositions[chosen], 1, &screenPosition);
                cursorPosition.x += 0.1f * (screenPosition.x - cursorPosition.x);
                cursorPosition.y += 0.1f * (screenPosition.y - cursorPosition.y);
                espPosSet(cursorSprite, cursorPosition.x, cursorPosition.y);
                if (timerFrames >= 0) {
                    --timerFrames;
                    if (timerFrames >= 0) {
                        GameMesDispSet(timer, 1, (s16)((timerFrames + 59) / 60));
                    } else if (timer >= 0) {
                        GameMesDispSet(timer, 2, -1);
                        timer = -1;
                    }
                }
                HuPrcVSleep();
            }
            if (timer >= 0) {
                GameMesDispSet(timer, 2, -1);
                timer = -1;
            }
            mbWinKill(helpWindow);
            helpWindow = -1;
            occupied[chosen] = 1;
            espDispOff(cursorSprite);
            chosenPlayers[chosen] = selectedPlayer;
            if (chosen == 1 || chosen == 2) {
                mbPlayerMasuMoveSpeed(selectedPlayer, choiceSpaces[chosen], 30, TRUE);
            } else {
                mbPlayerMasuMoveSpeed(selectedPlayer, choiceSpaces[chosen], 60, TRUE);
            }
            GwPlayer[selectedPlayer].masuId = choiceSpaces[chosen];
            GwPlayer[selectedPlayer].masuIdPrev = choiceSpaces[chosen];
            nextSpace = mbMasuGet(choiceSpaces[chosen]);
            GwPlayer[selectedPlayer].masuIdNext = nextSpace->linkTbl[0];
            mbPlayerMoveExec(selectedPlayer, NULL, NULL, 30, NULL, TRUE);
            mbPlayerRotateStart(selectedPlayer, 0, 30);
            while (!mbPlayerRotateCheck(selectedPlayer)) {
                HuPrcVSleep();
            }
            mbPlayerMotionShiftSet(selectedPlayer, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        }
        selectedPlayer = playerOrder[3];
        omVibrate(selectedPlayer, 20, 7, 3);
        fn_1_12A14(1);
        messageWindow = mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 21), speaker);
        mbWinTopInsertMesSet(mbPlayerNameMesGet(selectedPlayer), 0);
        mbWinPlayerDisable(messageWindow, selectedPlayer);
        mbWinTopWait();
        for (index = 0; index < 4; ++index) {
            if (occupied[index] == 0) {
                occupied[index] = 1;
                chosenPlayers[index] = selectedPlayer;
                if (index == 1 || index == 2) {
                    mbPlayerMasuMoveSpeed(selectedPlayer, choiceSpaces[index], 30, TRUE);
                } else {
                    mbPlayerMasuMoveSpeed(selectedPlayer, choiceSpaces[index], 60, TRUE);
                }
                GwPlayer[selectedPlayer].masuId = choiceSpaces[index];
                GwPlayer[selectedPlayer].masuIdPrev = choiceSpaces[index];
                nextSpace = mbMasuGet(choiceSpaces[index]);
                GwPlayer[selectedPlayer].masuIdNext = nextSpace->linkTbl[0];
                mbPlayerMoveExec(selectedPlayer, NULL, NULL, 30, NULL, TRUE);
                mbPlayerRotateStart(selectedPlayer, 0, 30);
                while (!mbPlayerRotateCheck(selectedPlayer)) {
                    HuPrcVSleep();
                }
                mbPlayerMotionShiftSet(selectedPlayer, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
                break;
            }
        }
        for (index = 0; index < 4; ++index) {
            if (outcomes[index] == 1) {
                winner = index;
            }
        }
        mbCameraMovePos(&lbl_1_data_790, &lbl_1_data_79C, NULL, 2500.0f, -1.0f, 30);
        mbCameraMoveWait();
        fn_1_12A14(0);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 22), speaker);
        mbWinTopWait();
        mbStatusDispSetAll(FALSE);
        while (!mbStatusOffCheckAll()) {
            HuPrcVSleep();
        }
        mbMusPauseFadeOut(0, TRUE, 1000);
        mbCameraMovePos(&lbl_1_data_7C0, NULL, NULL, 2500.0f, -1.0f, 120);
        HuPrcSleep(60);
        soundId = mbAudFXPlay(31);
        for (index = 0; index < 4; ++index) {
            stopTimes[index] = 580.0f;
            stopTimes[index] = stopTimes[index] * lbl_1_data_770[outcomes[index] - 1];
            motionSpeeds[index] = 2.0f;
            finished = 0;
            mbObjMotionStartEndSet((s16) lbl_1_bss_4D4.lotteryModels[index], 0,
                                   (s16) stopTimes[index]);
            mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.lotteryModels[index], motionSpeeds[index]);
            if (outcomes[index] == 1 || outcomes[index] == 2) {
                omVibrate(chosenPlayers[index], (s16)(160.0f + 0.5f * stopTimes[index]), 4, 4);
            } else {
                omVibrate(chosenPlayers[index], (s16)(0.5f * stopTimes[index]), 4, 4);
            }
        }
        while (finished == 0) {
            for (index = 0; index < 4; ++index) {
                if (mbObjMotionTimeGet((s16) lbl_1_bss_4D4.lotteryModels[index]) >=
                    stopTimes[index]) {
                    mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.lotteryModels[index], 0.0f);
                    mbObjMotionTimeSet((s16) lbl_1_bss_4D4.lotteryModels[index],
                                       stopTimes[index] - 1.0f);
                    if (outcomes[index] == 1) {
                        finished = 1;
                    }
                    motionSpeeds[index] = 0.0f;
                }
            }
            HuPrcVSleep();
        }
        HuPrcSleep(160);
        for (index = 0; index < 4; ++index) {
            if (outcomes[index] == 1) {
                stopTimes[index] = mbObjMotionMaxTimeGet((s16)lbl_1_bss_4D4.lotteryModels[index]);
                motionSpeeds[index] = 3.0f;
                mbObjMotionStartEndSet((s16) lbl_1_bss_4D4.lotteryModels[index], 580,
                                       (s16) stopTimes[index]);
                mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.lotteryModels[index], motionSpeeds[index]);
                omVibrate(chosenPlayers[winner], 20, 7, 3);
            }
        }
        while (mbObjMotionTimeGet((s16)lbl_1_bss_4D4.lotteryModels[winner]) < 600.0f) {
            HuPrcVSleep();
        }
        mbAudFXPlay(32);
        if (boardIsDay()) {
            coinPosition = lbl_1_data_7D8[winner];
            coinPositionPtr = &coinPosition;
            fn_1_B148(chosenPlayers[winner], prizeCount, coinPositionPtr);
        } else {
            starPosition = lbl_1_data_7D8[winner];
            starPositionPtr = &starPosition;
            fn_1_B770(chosenPlayers[winner], prizeCount, starPositionPtr);
        }
        mbMusPauseFadeOut(0, FALSE, 1000);
        mbStatusDispSetAll(TRUE);
        while (!mbStatusOffCheckAll()) {
            HuPrcVSleep();
        }
        fn_1_12A14(0);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 23), speaker);
        mbWinTopInsertMesSet(mbPlayerNameMesGet(chosenPlayers[winner]), 0);
        mbWinTopWait();
        for (index = 0; index < 4; ++index) {
            if (winner == index) {
                mbPlayerWinLoseVoicePlay(chosenPlayers[index], 12, CHARVOICEID(6));
                mbPlayerMotionShiftSet(chosenPlayers[index], 12, 0.0f, 8.0f, 0);
            } else {
                mbPlayerMotionShiftSet(chosenPlayers[index], 13, 0.0f, 8.0f, 0);
            }
        }
        while (!mbPlayerMotionEndCheck(chosenPlayers[winner])) {
            HuPrcVSleep();
        }
        HuPrcSleep(10);
        for (index = 0; index < 4; ++index) {
            mbPlayerMotionShiftSet(chosenPlayers[index], 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        }
        HuPrcSleep(60);
        mbObjMotionShiftSet((s16)lbl_1_bss_4D4.motionModel, 4, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_12A14(0);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 25), speaker);
        mbWinTopWait();
        HuPrcSleep(30);
        mbWipeFadeOut();
        mbObjPosSetV((s16)lbl_1_bss_4D4.motionModel, &lbl_1_data_7A8[0]);
        mbObjMotionShiftSet((s16)lbl_1_bss_4D4.motionModel, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        for (index = 3; index > -1; --index) {
            mbMasuPosGet(originalSpaces[index], &screenPosition);
            mbPlayerPosSetV(index, &screenPosition);
            GwPlayer[index].masuId = originalSpaces[index];
            mbev_PlayerColMasu(index, originalSpaces[index], TRUE);
            mbPlayerMotionShiftSet(index, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        }
        maxTime = mbObjMotionMaxTimeGet((s16)lbl_1_bss_4D4.lotteryModels[0]);
        for (index = 0; index < 4; ++index) {
            mbObjMotionStartEndSet((s16)lbl_1_bss_4D4.lotteryModels[index], 0, (s16)maxTime);
            mbObjMotionTimeSet((s16)lbl_1_bss_4D4.lotteryModels[index], 0.0f);
            mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.lotteryModels[index], 0.0f);
        }
        mbev_PlayerColMasu(playerNo, landingSpace, TRUE);
        mbCameraFocusPlayerSet(playerNo);
        mbCameraPlayerViewSetFast(playerNo, 2);
        espKill(cursorSprite);
        mbWipeFadeIn();
    }
finish:
    mbObjDispSet((s16)lbl_1_bss_4D4.motionModel, FALSE);
    for (index = 0; index < 4; ++index) {
        mbPlayerColSnapPlayerSet(index, TRUE);
    }
}

/* Animates board awards and follows the launch object as it moves the player to an exit. */

/* Drops coins from the award position toward the player and displays the total collected. */
void fn_1_B148(s32 playerNo, s32 coinCount, HuVecF *awardPosition)
{
    HuVecF positions[40];
    HuVecF velocities[40];
    s32 coinIds[40];
    s32 launchDelays[40];
    s32 animationState[40];
    s32 particleIds[40];
    s32 landed[40];
    HuVecF playerPosition;
    HuVecF particlePosition;
    HuVecF impactPosition;
    ANIMDATA *particleAnimation;
    s32 collectedCount;
    s32 coinIndex;

    mbPlayerPosGet(playerNo, &playerPosition);
    particleAnimation = HuSprAnimRead(HuDataReadNum(EFFECT_ANM_glow, HU_MEMNUM_OVL));
    for (coinIndex = 0; coinIndex < 40; coinIndex++) {
        coinIds[coinIndex] = -1;
        positions[coinIndex].x = positions[coinIndex].y = positions[coinIndex].z = 0.0f;
        launchDelays[coinIndex] = 0;
        landed[coinIndex] = 0;
        animationState[coinIndex] = 0;
        particleIds[coinIndex] = -1;
    }
    for (coinIndex = 0; coinIndex < coinCount; coinIndex++) {
        coinIds[coinIndex] = mbCoinCreate();
        positions[coinIndex] = *awardPosition;
        launchDelays[coinIndex] = coinIndex * 4;
        velocities[coinIndex].x = (100.0f * mbCosDeg((f32)mbRandMod(180))) / 45.0f;
        velocities[coinIndex].y = 0.15f * (50.0f + (100.0f * mbSinDeg((f32)mbRandMod(90))));
        velocities[coinIndex].z = (150.0f + (100.0f * mbSinDeg((f32)mbRandMod(90)))) / 45.0f;
        mbCoinObjDispSet((s16)coinIds[coinIndex], FALSE);
    }
    collectedCount = 0;
    while (coinIndex != 0) {
        for (coinIndex = 0; coinIndex < coinCount; coinIndex++) {
            if (collectedCount >= coinCount) {
                break;
            }
            if (--launchDelays[coinIndex] <= 0 && landed[coinIndex] == 0) {
                if (launchDelays[coinIndex] == 0) {
                    particleIds[coinIndex] = mbParticleCreate(particleAnimation, 32);
                    mbParticleHookSet((s16)particleIds[coinIndex], fn_1_104FC);
                    Hu3DModelLayerSet((s16)particleIds[coinIndex], 5);
                    particlePosition = positions[coinIndex];
                    particlePosition.x += 100.0f * (0.5f * mbCosDeg((f32)mbRandMod(180)));
                    particlePosition.y += 100.0f * (0.5f * mbCosDeg((f32)mbRandMod(180)));
                    Hu3DModelPosSetV((s16)particleIds[coinIndex], &particlePosition);
                }
                mbCoinObjDispSet((s16)coinIds[coinIndex], TRUE);
                positions[coinIndex].x += velocities[coinIndex].x;
                positions[coinIndex].z += velocities[coinIndex].z;
                velocities[coinIndex].y += -1.6333334f;
                positions[coinIndex].y += velocities[coinIndex].y;
                if (positions[coinIndex].y <= 150.0f + playerPosition.y) {
                    impactPosition = playerPosition;
                    impactPosition.y += 150.0f;
                    mbCoinEffCreate(&impactPosition);
                    mbCoinObjDispSet((s16)coinIds[coinIndex], FALSE);
                    landed[coinIndex] = 1;
                    collectedCount++;
                    mbAudFXPlay(7);
                }
                mbCoinObjPosSetV((s16)coinIds[coinIndex], &positions[coinIndex]);
            }
        }
        HuPrcVSleep();
    }
    HuPrcSleep(30);
    mbCoinAddDispExec(playerNo, collectedCount, TRUE, TRUE);
    for (coinIndex = 0; coinIndex < coinCount; coinIndex++) {
        mbCoinObjNumDec((s16)coinIds[coinIndex]);
        if (particleIds[coinIndex] != -1) {
            mbParticleKill((s16)particleIds[coinIndex]);
        }
    }
    HuDataDirClose(DATA_effect);
}

/* Releases Stars ten frames apart, credits them on landing, then waits for the gain display. */
void fn_1_B770(s32 playerNo, s32 starCount, HuVecF *awardPosition)
{
    HuVecF positions[40];
    HuVecF velocities[40];
    s32 starIds[40];
    s32 launchDelays[40];
    s32 animationState[40];
    s32 particleIds[40];
    s32 landed[40];
    HuVecF playerPosition;
    HuVecF particlePosition;
    s32 displayId;
    ANIMDATA *particleAnimation;
    s32 collectedCount;
    s32 starIndex;

    collectedCount = 0;
    mbPlayerPosGet(playerNo, &playerPosition);
    particleAnimation = HuSprAnimRead(HuDataReadNum(EFFECT_ANM_glow, HU_MEMNUM_OVL));
    for (starIndex = 0; starIndex < 40; starIndex++) {
        starIds[starIndex] = -1;
        positions[starIndex].x = positions[starIndex].y = positions[starIndex].z = 0.0f;
        launchDelays[starIndex] = 0;
        landed[starIndex] = 0;
        animationState[starIndex] = 0;
        particleIds[starIndex] = -1;
    }
    for (starIndex = 0; starIndex < starCount; starIndex++) {
        starIds[starIndex] = mbStarObjCreate();
        positions[starIndex] = *awardPosition;
        launchDelays[starIndex] = starIndex * 10;
        velocities[starIndex].x = (100.0f * mbCosDeg((f32)mbRandMod(180))) / 45.0f;
        velocities[starIndex].y = 0.15f * (50.0f + (100.0f * mbSinDeg((f32)mbRandMod(90))));
        velocities[starIndex].z = (150.0f + (100.0f * mbSinDeg((f32)mbRandMod(90)))) / 45.0f;
        mbStarObjDispSet(starIds[starIndex], FALSE);
        mbStarObjDispFlagSet(starIds[starIndex], FALSE);
    }
    while (starIndex != 0) {
        for (starIndex = 0; starIndex < starCount; starIndex++) {
            if (collectedCount >= starCount) {
                break;
            }
            if (--launchDelays[starIndex] <= 0 && landed[starIndex] == 0) {
                if (launchDelays[starIndex] == 0) {
                    particleIds[starIndex] = mbParticleCreate(particleAnimation, 160);
                    mbParticleHookSet((s16)particleIds[starIndex], fn_1_104FC);
                    Hu3DModelLayerSet((s16)particleIds[starIndex], 5);
                    particlePosition = positions[starIndex];
                    particlePosition.x += 100.0f * (0.5f * mbCosDeg((f32)mbRandMod(180)));
                    particlePosition.y += 100.0f * (0.5f * mbCosDeg((f32)mbRandMod(180)));
                    Hu3DModelPosSetV((s16)particleIds[starIndex], &particlePosition);
                }
                mbStarObjDispSet(starIds[starIndex], TRUE);
                positions[starIndex].x += velocities[starIndex].x;
                positions[starIndex].z += velocities[starIndex].z;
                velocities[starIndex].y += -1.6333334f;
                positions[starIndex].y += velocities[starIndex].y;
                if (positions[starIndex].y <= 150.0f + playerPosition.y) {
                    mbStarObjDispSet(starIds[starIndex], FALSE);
                    landed[starIndex] = 1;
                    collectedCount++;
                    mbPlayerStarAdd(playerNo, 1);
                }
                mbStarObjPosSetV(starIds[starIndex], &positions[starIndex]);
            }
        }
        HuPrcVSleep();
    }
    HuPrcSleep(30);
    omVibrate((s16)playerNo, 20, 7, 3);
    displayId = mbStarDispPlayerCreate(playerNo, starCount);
    while (mbStarDispCheck(displayId) == 0) {
        HuPrcVSleep();
    }
    for (starIndex = 0; starIndex < starCount; starIndex++) {
        mbStarObjKill(starIds[starIndex]);
        if (particleIds[starIndex] != -1) {
            mbParticleKill((s16)particleIds[starIndex]);
        }
    }
    HuDataDirClose(DATA_effect);
}

f32 lbl_1_data_808[3] = {-5e+01f, 0.0f, 2e+02f};
/* Follows the launch object's animation, then moves the player to one of its two exits. */
void fn_1_BD88(s32 playerNo, s16 landingSpace)
{
    Mtx launchMatrix;
    s32 smokeParticleIds[3];
    HuVecF launchRotation;
    HuVecF playerRotation;
    HuVecF landingPosition;
    HuVecF soundPosition;
    s32 landingSpaces[2] = {42, 44};
    f32 launchTime = 0.0f;
    s32 launchModelId;
    s32 launchMotion;
    s32 firstExitMotion;
    s32 secondExitMotion;
    s32 soundHandle;
    s32 waitingMotion;
    s32 alpha;
    ANIMDATA *smokeAnimation;
    ANIMDATA *launchAnimation;
    s32 launchParticleId;
    s32 active;
    s32 exitIndex;
    s32 frame;

    if (mbRandMod(10) < 4) {
        exitIndex = 0;
    } else {
        exitIndex = 1;
    }
    launchMotion = mbPlayerMotionCreate(playerNo, CHARMOT_HSF_c000m1_344);
    firstExitMotion = mbPlayerMotionCreate(playerNo, CHARMOT_HSF_c000m1_317);
    secondExitMotion = mbPlayerMotionCreate(playerNo, CHARMOT_HSF_c000m1_316);
    waitingMotion = mbPlayerMotionCreate(playerNo, CHARMOT_HSF_c000m1_323);
    mbPlayerRotateStart(playerNo, 0, 15);
    while (mbPlayerRotateCheck(playerNo) == 0) {
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 32.0f, HU3D_MOTATTR_LOOP);
    HuPrcSleep(10);
    launchModelId = mbObjModelIDGet((s16)lbl_1_bss_4D4.finalModels[2]);
    mbObjDispSet((s16)lbl_1_bss_4D4.finalModels[2], TRUE);
    mbObjAlphaSet((s16)lbl_1_bss_4D4.finalModels[2], 255);
    mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.finalModels[2], 1.0f);
    mbObjMotionTimeSet((s16)lbl_1_bss_4D4.finalModels[2], 0.0f);
    launchAnimation = HuSprAnimRead(HuDataReadNum(EFFECT_ANM_glow, HU_MEMNUM_OVL));
    launchParticleId = mbParticleCreate(launchAnimation, 256);
    mbParticleHookSet((s16)launchParticleId, fn_1_FCB4);
    Hu3DModelLayerSet((s16)launchParticleId, 5);
    lbl_1_bss_C = 1;
    mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.finalModels[1], 1.0f);
    mbObjMotionTimeSet((s16)lbl_1_bss_4D4.finalModels[1], 0.0f);
    active = 1;
    mbPlayerMotionShiftSet(playerNo, waitingMotion, 0.0f, 4.0f, HU3D_MOTATTR_LOOP);
    soundHandle = mbAudFXPlay(MSM_SE_W03_LAUNCH_ENGINE);
    while (active != 0) {
        Hu3DModelObjMtxGet((s16)launchModelId, "fook", launchMatrix);
        Hu3DMtxRotGet(launchMatrix, &launchRotation);
        soundPosition.x = launchMatrix[0][3];
        soundPosition.y = launchMatrix[1][3];
        soundPosition.z = launchMatrix[2][3];
        mbAudFXPosPanning(soundHandle, &soundPosition);
        if (148.0f == mbObjMotionTimeGet((s16)lbl_1_bss_4D4.finalModels[2])) {
            mbPlayerMotionShiftSet(playerNo, 9, 0.0f, 4.0f, 0);
        }
        if (mbObjMotionTimeGet((s16)lbl_1_bss_4D4.finalModels[2]) >= 166.0f) {
            mbPlayerMotionShiftSet(playerNo, launchMotion, 0.0f, 4.0f, HU3D_MOTATTR_LOOP);
            mbPlayerRotGet(playerNo, &playerRotation);
            mbPlayerPosSet(playerNo, launchMatrix[0][3], launchMatrix[1][3], launchMatrix[2][3]);
            mbPlayerRotSetV(playerNo, &launchRotation);
        }
        if (exitIndex == 0 &&
            mbObjMotionTimeGet((s16)lbl_1_bss_4D4.finalModels[2]) >= 370.0f) {
            lbl_1_bss_C = 0;
            mbMasuPosGet((s16)landingSpaces[exitIndex], &landingPosition);
            active = 0;
            mbev_PlayerColReserve(playerNo, landingSpaces[exitIndex], FALSE);
            mbPlayerMoveMain(playerNo, NULL, &landingPosition, firstExitMotion,
                1.0f, 0, 30, NULL, TRUE);
        } else if (exitIndex == 1 &&
            mbObjMotionTimeGet((s16)lbl_1_bss_4D4.finalModels[2]) >= 340.0f) {
            lbl_1_bss_C = 0;
            mbMasuPosGet((s16)landingSpaces[exitIndex], &landingPosition);
            active = 0;
            mbev_PlayerColReserve(playerNo, landingSpaces[exitIndex], FALSE);
            mbPlayerMoveMain(playerNo, NULL, &landingPosition, secondExitMotion,
                1.0f, 0, 30, NULL, TRUE);
        }
        HuPrcVSleep();
    }
    mbAudFXStop(soundHandle);
    HuPrcSleep(10);
    mbParticleKill((s16)launchParticleId);
    launchParticleId = -1;
    Hu3DModelObjMtxGet((s16)launchModelId, "fook", launchMatrix);
    smokeAnimation = HuSprAnimRead(HuDataReadNum(EFFECT_ANM_smoke, HU_MEMNUM_OVL));
    for (frame = 0; frame < 3; frame++) {
        smokeParticleIds[frame] = mbParticleCreate(smokeAnimation, 128);
        mbParticleHookSet((s16)smokeParticleIds[frame], fn_1_F704);
        Hu3DModelLayerSet((s16)smokeParticleIds[frame], 5);
        Hu3DModelPosSet((s16)smokeParticleIds[frame],
            launchMatrix[0][3] + lbl_1_data_808[frame], launchMatrix[1][3], launchMatrix[2][3]);
    }
    for (frame = 0; (f32)frame < 25.0f; frame++) {
        alpha = 255.0f - ((255.0f * (f32)frame) / 25.0f);
        mbObjAlphaSet((s16)lbl_1_bss_4D4.finalModels[2], (s16)alpha);
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 32.0f, HU3D_MOTATTR_LOOP);
    GwPlayer[playerNo].masuId = (s16)landingSpaces[exitIndex];
    mbPlayerRotateStart(playerNo, 0, 15);
    while (mbPlayerRotateCheck(playerNo) == 0) {
        HuPrcVSleep();
    }
    mbPlayerRotSet(playerNo, 0.0f, 0.0f, 0.0f);
    mbPlayerMotionKill(playerNo, launchMotion);
    mbPlayerMotionKill(playerNo, firstExitMotion);
    mbPlayerMotionKill(playerNo, secondExitMotion);
    /* The waiting motion remains allocated when the other three motions are removed. */
    mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.finalModels[1], 0.0f);
    mbObjMotionTimeSet((s16)lbl_1_bss_4D4.finalModels[1], 0.0f);
    mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.finalModels[2], 0.0f);
    mbObjMotionTimeSet((s16)lbl_1_bss_4D4.finalModels[2], 0.0f);
    for (frame = 0; frame < 3; frame++) {
        mbParticleKill((s16)smokeParticleIds[frame]);
    }
    HuDataDirClose(DATA_effect);
}

/* Moves players through the board's animated transport and into its exit space. */

HuVecF lbl_1_data_81C[3] = { { 2.74e+03f, 9.2e+02f, -1.04e+03f },
                             { 3005.0f, 9.2e+02f, -425.0f },
                             { 0.0f, 1e+02f, 3.45e+03f } };
f32 lbl_1_data_840[2] = {0.0f, 119.0f};
s32 lbl_1_data_848[2] = {117, 240};
/* Runs the transport animation for everyone sharing the triggering player's space. */
void fn_1_C59C(s32 playerNo, s16 entranceSpace)
{
    s16 particleIds[4] = { -1, -1, -1, -1 };
    int movingCount = 0;
    s16 exitSpace;
    int residentCount = 0;
    s32 movingPlayers[4] = { -1, -1, -1, -1 };
    s32 residentPlayers[4] = { -1, -1, -1, -1 };
    s32 entranceMotions[4] = { -1, -1, -1, -1 };
    s32 exitMotions[4] = { -1, -1, -1, -1 };
    HuVecF cameraPosition;
    HuVecF exitPosition;
    HuVecF cornerPosition;
    HuVecF particlePosition;
    f32 cameraZoom;
    s32 entranceMotionLength;
    ANIMDATA *particleAnimation;
    s16 corner;
    s32 route;
    u32 spaceAttributes;
    s32 effectsStarted;
    s32 playerIndex;

    cameraZoom = mbCameraZoomGet();
    exitSpace = mbMasuFind_MAttrIdGet(-1, W03_MATTR_TRANSPORT_EXIT);
    mbMasuPosGet(exitSpace, &exitPosition);
    particleAnimation = HuSprAnimRead(HuDataReadNum(DATANUM(DATA_effect, 2), HU_MEMNUM_OVL));
    entranceMotionLength = mbObjMotionMaxTimeGet((s16)lbl_1_bss_4D4.finalModels[3]);
    mbObjDispSet((s16)lbl_1_bss_4D4.finalModels[5], FALSE);
    mbObjDispSet((s16)lbl_1_bss_4D4.finalModels[4], TRUE);
    mbObjMotionStartEndSet((s16)lbl_1_bss_4D4.finalModels[4], 0, 138);
    mbObjMotionTimeSet((s16)lbl_1_bss_4D4.finalModels[4], 0.0f);
    mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.finalModels[4], 0.0f);
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (entranceSpace == GwPlayer[playerIndex].masuId) {
            movingPlayers[movingCount] = playerIndex;
            entranceMotions[movingCount] =
                mbPlayerMotionCreate(movingPlayers[movingCount], CHARMOT_HSF_c000m1_344);
            exitMotions[movingCount] =
                mbPlayerMotionCreate(movingPlayers[movingCount], CHARMOT_HSF_c000m1_318);
            mbPlayerColSnapPlayerSet(playerIndex, FALSE);
            movingCount++;
        }
        if (exitSpace == GwPlayer[playerIndex].masuId) {
            residentPlayers[residentCount] = playerIndex;
            mbPlayerColSnapPlayerSet(playerIndex, FALSE);
            residentCount++;
        }
    }
    if (residentCount != 0) {
        for (playerIndex = residentCount - 1; playerIndex > -1; playerIndex--) {
            corner = mbPlayerMasuCornerGet(residentPlayers[playerIndex]);
            if (corner != 0) {
                corner += movingCount - 1;
                mbMasuCornerRotPosGet(exitSpace, corner, &cornerPosition);
                mbPlayerPosSetV(residentPlayers[playerIndex], &cornerPosition);
                mbPlayerMasuCornerSet(residentPlayers[playerIndex], corner);
            }
        }
    }
    HuPrcSleep(30);
    spaceAttributes = mbMasuMAttrGet(entranceSpace);
    if (spaceAttributes & 0x8) {
        route = 0;
    } else if (spaceAttributes & 0x20) {
        route = 1;
    }
    mbObjMotionTimeSet((s16)lbl_1_bss_4D4.finalModels[3], lbl_1_data_840[route]);
    mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.finalModels[3], 1.0f);
    mbObjMotionStartEndSet((s16)lbl_1_bss_4D4.finalModels[3],
        (s16)lbl_1_data_840[route], (s16)(10.0f + lbl_1_data_840[route]));
    for (playerIndex = 0; playerIndex < movingCount; playerIndex++) {
        mbPlayerRotYSet(movingPlayers[playerIndex], 100.0f);
        mbPlayerMotionShiftSet(movingPlayers[playerIndex], 9, 0.0f, 4.0f, 0);
    }
    mbAudFXPlay(MSM_SE_W03_TRANSPORT_START);
    while (!mbObjMotionEndCheck((s16)lbl_1_bss_4D4.finalModels[3])) {
        HuPrcVSleep();
    }
    while (!mbPlayerMotionEndCheck(playerNo)) {
        HuPrcVSleep();
    }
    mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.finalModels[3], 0.0f);
    HuPrcVSleep();
    for (playerIndex = 0; playerIndex < movingCount; playerIndex++) {
        mbPlayerMoveMain(movingPlayers[playerIndex], NULL, &lbl_1_data_81C[route],
            entranceMotions[playerIndex], 1.0f, HU3D_MOTATTR_LOOP, 18, NULL, FALSE);
    }
    HuPrcSleep(10);
    mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.finalModels[3], 1.0f);
    mbObjMotionStartEndSet((s16)lbl_1_bss_4D4.finalModels[3],
        (s16)(10.0f + lbl_1_data_840[route]), (s16)lbl_1_data_848[route]);
    if (route == 0) {
        HuPrcSleep(10);
    }
    HuPrcSleep(20);
    mbAudFXPlay(MSM_SE_W03_TRANSPORT_DEPART);
    mbAudFXPlay(MSM_SE_W03_TRANSPORT_PASS);
    omVibrate(playerNo, 20, 7, 3);
    HuPrcSleep(10);
    mbAudFXPlay(MSM_SE_W03_TRANSPORT_ARRIVE);
    while (!mbObjMotionEndCheck((s16)lbl_1_bss_4D4.finalModels[3])) {
        HuPrcVSleep();
    }
    mbPlayerPosGet(playerNo, &cameraPosition);
    mbCameraMovePos(&cameraPosition, NULL, NULL, 5500.0f, -1.0f, 30);
    mbCameraMoveWait();
    mbCameraFocusPlayerSet(-1);
    mbCameraMoveMasu(exitSpace, NULL, NULL, -1.0f, -1.0f, 120);
    mbCameraMoveWait();
    mbCameraMovePos(&lbl_1_data_81C[2], NULL, NULL, cameraZoom, -1.0f, 30);
    mbCameraMoveWait();
    HuPrcSleep(20);
    omVibrate(playerNo, 20, 7, 3);
    mbAudFXPlay(MSM_SE_W03_TRANSPORT_PASS);
    mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.finalModels[4], 1.0f);
    mbObjMotionTimeSet((s16)lbl_1_bss_4D4.finalModels[4], 0.0f);
    mbObjMotionStartEndSet((s16)lbl_1_bss_4D4.finalModels[4], 0, 90);
    while (mbObjMotionTimeGet((s16)lbl_1_bss_4D4.finalModels[4]) < 80.0f) {
        if (70.0f == mbObjMotionTimeGet((s16)lbl_1_bss_4D4.finalModels[4])) {
            mbAudFXPlay(MSM_SE_W03_TRANSPORT_START);
        }
        HuPrcVSleep();
    }
    for (playerIndex = 0; playerIndex < movingCount; playerIndex++) {
        mbPlayerPosSetV(movingPlayers[playerIndex], &lbl_1_data_81C[2]);
    }
    HuPrcVSleep();
    for (playerIndex = 0; playerIndex < movingCount; playerIndex++) {
        if (residentCount == 0) {
            mbPlayerMoveMain(movingPlayers[playerIndex], &lbl_1_data_81C[2], &exitPosition,
                exitMotions[playerIndex], 1.0f, 0, 60, NULL, FALSE);
        } else {
            corner = mbPlayerMasuCornerGet(movingPlayers[playerIndex]);
            mbMasuCornerRotPosGet(exitSpace, corner, &cornerPosition);
            mbPlayerMoveMain(movingPlayers[playerIndex], &lbl_1_data_81C[2], &cornerPosition,
                exitMotions[playerIndex], 1.0f, 0, 60, NULL, FALSE);
        }
    }
    effectsStarted = 0;
    while (!mbPlayerMotionEndCheck(playerNo)) {
        if (effectsStarted == 0) {
            for (playerIndex = 0; playerIndex < movingCount; playerIndex++) {
                if (mbPlayerMotionTimeGet(movingPlayers[playerIndex]) >= 40.0f) {
                    mbPlayerPosGet(movingPlayers[playerIndex], &particlePosition);
                    particleIds[playerIndex] = mbParticleCreate(particleAnimation, 32);
                    mbParticleHookSet(particleIds[playerIndex], fn_1_F704);
                    Hu3DModelLayerSet(particleIds[playerIndex], 5);
                    Hu3DModelPosSetV(particleIds[playerIndex], &particlePosition);
                    effectsStarted = 1;
                }
            }
        }
        HuPrcVSleep();
    }
    mbObjMotionStartEndSet((s16)lbl_1_bss_4D4.finalModels[4], 90,
        mbObjMotionMaxTimeGet((s16)lbl_1_bss_4D4.finalModels[4]));
    mbAudFXDelaySet(17);
    mbAudFXPlay(MSM_SE_W03_MOVE_STEP);
    HuPrcSleep(18);
    mbAudFXPlay(MSM_SE_W03_TRANSPORT_ARRIVE);
    HuPrcSleep(40);
    for (playerIndex = 0; playerIndex < movingCount; playerIndex++) {
        mbPlayerMotionSpeedSet(movingPlayers[playerIndex], 0.1f);
        mbPlayerMotionShiftSet(movingPlayers[playerIndex], 1, 0.0f, 32.0f, HU3D_MOTATTR_LOOP);
        mbPlayerRotYSet(movingPlayers[playerIndex], 0.0f);
        GwPlayer[movingPlayers[playerIndex]].masuId = exitSpace;
    }
    HuPrcSleep(50);
    for (playerIndex = 0; playerIndex < movingCount; playerIndex++) {
        mbPlayerMotionKill(movingPlayers[playerIndex], entranceMotions[playerIndex]);
        mbPlayerMotionKill(movingPlayers[playerIndex], exitMotions[playerIndex]);
        mbPlayerMotionSpeedSet(movingPlayers[playerIndex], 1.0f);
        mbParticleKill(particleIds[playerIndex]);
    }
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        mbPlayerColSnapPlayerSet(playerIndex, TRUE);
    }
    mbObjMotionStartEndSet((s16)lbl_1_bss_4D4.finalModels[3], 0, entranceMotionLength);
    mbObjMotionTimeSet((s16)lbl_1_bss_4D4.finalModels[3], 0.0f);
    mbObjMotionSpeedSet((s16)lbl_1_bss_4D4.finalModels[3], 0.0f);
    HuDataDirClose(DATA_effect);
}

/* Starts the recursive search for a named HSF object at the model's root. */

HSF_OBJECT *fn_1_D150(HU3D_MODEL *model, char *name)
{
    HSF_DATA *hsf = model->hsf;

    return fn_1_D198(model, name, hsf->root);
}

/* Searches a model's mesh and replica hierarchy for the named object. */
static HSF_OBJECT *fn_1_D198(HU3D_MODEL *model, char *name, HSF_OBJECT *object)
{
    int childIndex;
    HSF_OBJECT *meshObject;
    HSF_OBJECT *replicaObject;
    HSF_OBJECT *result;

    switch (object->type) {
    case HSF_OBJ_NULL1:
    case HSF_OBJ_MESH:
    case HSF_OBJ_ROOT:
    case HSF_OBJ_JOINT:
    case HSF_OBJ_NULL2:
    case HSF_OBJ_NULL3:
    case HSF_OBJ_MAP:
        meshObject = object;
        if (!strcmp(name, meshObject->name)) {
            return meshObject;
        }
        for (childIndex = 0; childIndex < meshObject->mesh.childNum; childIndex++) {
            result = fn_1_D198(model, name, meshObject->mesh.child[childIndex]);
            if (result) {
                return result;
            }
        }
        break;
    case HSF_OBJ_REPLICA:
        replicaObject = object;
        if (!strcmp(name, replicaObject->name)) {
            return replicaObject;
        }
        for (childIndex = 0; childIndex < replicaObject->mesh.childNum; childIndex++) {
            result = fn_1_D198(model, name, replicaObject->mesh.child[childIndex]);
            if (result) {
                return result;
            }
        }
        break;
    }
    return NULL;
}

/* Moves an object between two positions, optionally turning it before the move. */
void fn_1_D9E0(s32 motionObjectId, s32 movingObjectId, f32 yaw, f32 turnAngle,
    HuVecF *startPosition, HuVecF *endPosition, s32 turnFirst)
{
    HuVecF position;
    f32 progress;
    f32 yawStep;
    s32 frame;

    mbObjMotionShiftSet((s16)lbl_1_bss_4D4.motionModel, 3, 0.0f,
        8.0f, HU3D_MOTATTR_LOOP);
    if (turnFirst == 0) {
        mbAudFXPlay(MSM_SE_W03_MOVE_START);
        for (frame = 0; (f32)frame < 75.0f; frame++) {
            if (mbObjMotionTimeGet((s16)motionObjectId) >= 60.0f) {
                mbObjMotionSpeedSet((s16)motionObjectId, 0.0f);
            }
            progress = (f32)frame / 75.0f;
            position.x = startPosition->x + progress * (endPosition->x - startPosition->x);
            position.y = startPosition->y + progress * (endPosition->y - startPosition->y);
            position.z = startPosition->z + progress * (endPosition->z - startPosition->z);
            mbObjPosSetV((s16)movingObjectId, &position);
            mbObjRotSet((s16)movingObjectId, 0.0f, yaw, 0.0f);
            HuPrcVSleep();
        }
        mbAudFXDelaySet(4);
        mbAudFXPlay(MSM_SE_W03_MOVE_STEP);
    } else {
        mbAudFXPlay(MSM_SE_W03_MOVE_START);
        yawStep = turnAngle / 15.000001f;
        for (frame = 0; (f32)frame < 15.000001f; frame++) {
            yaw -= yawStep;
            mbObjRotSet((s16)movingObjectId, 0.0f, yaw, 0.0f);
            HuPrcVSleep();
        }
        mbObjMotionShiftSet((s16)lbl_1_bss_4D4.motionModel, 1, 0.0f,
            8.0f, HU3D_MOTATTR_LOOP);
        HuPrcSleep(30);
        mbObjMotionTimeSet((s16)motionObjectId, 90.0f);
        mbObjMotionSpeedSet((s16)motionObjectId, 0.0f);
        mbObjMotionShiftSet((s16)lbl_1_bss_4D4.motionModel, 3, 0.0f,
            8.0f, HU3D_MOTATTR_LOOP);
        for (frame = 0; (f32)frame < 75.0f; frame++) {
            if (25.0f == (f32)frame) {
                mbAudFXDelaySet(20);
                mbAudFXPlay(MSM_SE_W03_MOVE_STEP);
                mbObjMotionSpeedSet((s16)motionObjectId, 2.0f);
            }
            progress = (f32)frame / 75.0f;
            position.x = startPosition->x + progress * (endPosition->x - startPosition->x);
            position.y = startPosition->y + progress * (endPosition->y - startPosition->y);
            position.z = startPosition->z + progress * (endPosition->z - startPosition->z);
            mbObjPosSetV((s16)movingObjectId, &position);
            HuPrcVSleep();
        }
    }
    mbObjMotionShiftSet((s16)lbl_1_bss_4D4.motionModel, 1, 0.0f,
        8.0f, HU3D_MOTATTR_LOOP);
}

/* Caps the displayed jackpot at 999, then refreshes its three digit display. */
void fn_1_DE88(s32 coinTotal)
{
    s32 digit;
    s32 i;

    if (coinTotal > 999) {
        coinTotal = 999;
    }
    for (i = 0; i < 3; i++) {
        Hu3DAnimKill(lbl_1_bss_4D4.digitAnimationIds[i]);
    }
    digit = coinTotal / 100;
    lbl_1_bss_4D4.digitAnimationIds[0] = Hu3DAnimCreate(
        lbl_1_bss_4D4.digitAnimations[digit],
        mbObjModelIDGet((s16)lbl_1_bss_4D4.initialModels[2]), lbl_1_data_4B0[0]);
    coinTotal -= digit * 100;
    digit = coinTotal / 10;
    lbl_1_bss_4D4.digitAnimationIds[1] = Hu3DAnimCreate(
        lbl_1_bss_4D4.digitAnimations[digit],
        mbObjModelIDGet((s16)lbl_1_bss_4D4.initialModels[2]), lbl_1_data_4B0[1]);
    coinTotal -= digit * 10;
    lbl_1_bss_4D4.digitAnimationIds[2] = Hu3DAnimCreate(
        lbl_1_bss_4D4.digitAnimations[coinTotal],
        mbObjModelIDGet((s16)lbl_1_bss_4D4.initialModels[2]), lbl_1_data_4B0[2]);
}

void mbDicePadBtnHookSet(int playerNo, u16 (*hook)(int));

void mbDiceMotHookSet(int playerNo, void (*hook)(int));

/* The coin-machine award drops its prize toward the active player, then releases the models. */
void fn_1_E00C(int coinCount)
{
    HuVecF playerPosition;
    int result = 0;
    int awarded;
    int spawned;
    int spawnDelay;
    int coinIndex;
    s16 endingModelId;
    s16 createdModelId;

    awarded = 0;
    spawned = 0;
    spawnDelay = 0;
    for (coinIndex = 0; coinIndex < 16; coinIndex++) {
        createdModelId = mbObjCreate(mbBoardDataNumGet(DATANUM(DATA_board, 4)), NULL, TRUE);
        lbl_1_bss_10[coinIndex].modelId = createdModelId;
        mbObjDispSet((s16)lbl_1_bss_10[coinIndex].modelId, FALSE);
    }
    for (;;) {
        if (awarded >= coinCount) {
            break;
        }
        if (--spawnDelay <= 0 && spawned < coinCount) {
            for (coinIndex = 0; coinIndex < 16; coinIndex++) {
                if (!mbObjDispGet((s16)lbl_1_bss_10[coinIndex].modelId)) {
                    mbObjDispSet((s16)lbl_1_bss_10[coinIndex].modelId, TRUE);
                    spawnDelay = 6;
                    mbPlayerPosGet(GwSystem.turnPlayerNo, &playerPosition);
                    lbl_1_bss_10[coinIndex].position.x = playerPosition.x +
                        100.0f * (0.2f * frandf()) - 10.0f;
                    lbl_1_bss_10[coinIndex].position.y = 4e+02f + playerPosition.y;
                    lbl_1_bss_10[coinIndex].position.z = playerPosition.z;
                    lbl_1_bss_10[coinIndex].work[0] = 0;
                    lbl_1_bss_10[coinIndex].velocity.y = -8.0f;
                    lbl_1_bss_10[coinIndex].rotation.y = mbRandMod(360);
                    lbl_1_bss_10[coinIndex].work[2] = 1;
                    lbl_1_bss_10[coinIndex].alpha = 0;
                    mbObjPosSetV((s16)lbl_1_bss_10[coinIndex].modelId,
                        &lbl_1_bss_10[coinIndex].position);
                    mbObjRotSetV((s16)lbl_1_bss_10[coinIndex].modelId,
                        &lbl_1_bss_10[coinIndex].rotation);
                    mbObjScaleSet((s16)lbl_1_bss_10[coinIndex].modelId,
                        0.8f, 0.8f, 0.8f);
                    mbObjAlphaSet((s16)lbl_1_bss_10[coinIndex].modelId,
                        lbl_1_bss_10[coinIndex].alpha);
                    spawned++;
                    break;
                }
            }
        }
        for (coinIndex = 0; coinIndex < 16; coinIndex++) {
            if (mbObjDispGet((s16)lbl_1_bss_10[coinIndex].modelId)) {
                VECAdd(&lbl_1_bss_10[coinIndex].position, &lbl_1_bss_10[coinIndex].velocity,
                    &lbl_1_bss_10[coinIndex].position);
                lbl_1_bss_10[coinIndex].velocity.y -= 0.4f;
                if (lbl_1_bss_10[coinIndex].velocity.y <= -3e+01f) {
                    lbl_1_bss_10[coinIndex].velocity.y = -3e+01f;
                }
                lbl_1_bss_10[coinIndex].rotation.y += 5.0f;
                lbl_1_bss_10[coinIndex].alpha += 20;
                if (lbl_1_bss_10[coinIndex].alpha >= 255) {
                    lbl_1_bss_10[coinIndex].alpha = 255;
                }
                mbObjPosSetV((s16)lbl_1_bss_10[coinIndex].modelId,
                    &lbl_1_bss_10[coinIndex].position);
                mbObjRotSetV((s16)lbl_1_bss_10[coinIndex].modelId,
                    &lbl_1_bss_10[coinIndex].rotation);
                mbObjAlphaSet((s16)lbl_1_bss_10[coinIndex].modelId,
                    lbl_1_bss_10[coinIndex].alpha);
                if (lbl_1_bss_10[coinIndex].position.y <
                    100.0f + playerPosition.y) {
                    if (mbPlayerCoinGet(GwSystem.turnPlayerNo) < 999) {
                        mbPlayerCoinAdd(GwSystem.turnPlayerNo, 1);
                        mbAudFXPlay(7);
                    }
                    mbCoinEffCreate(&lbl_1_bss_10[coinIndex].position);
                    mbObjDispSet((s16)lbl_1_bss_10[coinIndex].modelId, FALSE);
                    awarded++;
                    lbl_1_bss_10[coinIndex].position.x =
                        lbl_1_bss_10[coinIndex].position.y =
                        lbl_1_bss_10[coinIndex].position.z = 0.0f;
                    lbl_1_bss_10[coinIndex].rotation.x =
                        lbl_1_bss_10[coinIndex].rotation.y =
                        lbl_1_bss_10[coinIndex].rotation.z = 0.0f;
                    lbl_1_bss_10[coinIndex].velocity.x =
                        lbl_1_bss_10[coinIndex].velocity.y =
                        lbl_1_bss_10[coinIndex].velocity.z = 0.0f;
                    lbl_1_bss_10[coinIndex].work[0] = 0;
                    lbl_1_bss_10[coinIndex].work[1] = 0;
                    lbl_1_bss_10[coinIndex].work[2] = 0;
                    lbl_1_bss_10[coinIndex].work[3] = 0;
                    lbl_1_bss_10[coinIndex].work[4] = 0;
                    lbl_1_bss_10[coinIndex].work[5] = 0;
                }
            }
        }
        HuPrcVSleep();
    }
    mbAudFXPlay(15);
    for (coinIndex = 0; coinIndex < 16; coinIndex++) {
        endingModelId = (s16)lbl_1_bss_10[coinIndex].modelId;
        mbObjKill(endingModelId);
    }
}

HuVecF lbl_1_data_850[8] = { { 128.0f, 128.0f, 128.0f }, { 2e+02f, 0.0f, 0.0f },
                             { 0.0f, 2e+02f, 0.0f },     { 0.0f, 0.0f, 2e+02f },
                             { 2e+02f, 2e+02f, 0.0f },   { 0.0f, 2e+02f, 2e+02f },
                             { 2e+02f, 0.0f, 2e+02f },   { 2e+02f, 1.5e+02f, 0.0f } };
/* Initializes and advances the first curved particle burst. */
void fn_1_E7A8(HU3D_MODEL *model, MBPARTICLE *system, Mtx matrix)
{
    HuVecF direction;
    MBPARTICLEDATA *particle;
    s32 particleIndex;
    u32 colorIndex;
    f32 angle;

    if (system->initF == 0) {
        particle = system->data;
        for (particleIndex = 0; particleIndex < system->num;
             particleIndex++, particle++) {
            colorIndex = mbRandMod(6);
            particle->time = 1;
            particle->activeF = 50.0f *
                (0.4f + 1.5258789e-05f * (f32)(u16)frand());
            particle->animBank = (s16)((s32)(16.0f *
                (1.5258789e-05f * (f32)(u16)frand())) & 0xF);
            angle = 360.0f *
                (3.0517578e-05f * (f32)(u16)frand() - 1.0f);
            direction.x = 4.0f + mbSinDeg(angle);
            direction.z = mbCosDeg(angle) - 4.0f;
            direction.y = 4.0f +
                mbSinDeg(360.0f *
                    (1.5258789e-05f * (f32)(u16)frand()));
            VECScale(&direction, &particle->pos,
                100.0f *
                (0.5f * (0.5f +
                    0.5f * (1.5258789e-05f * (f32)(u16)frand()))));
            VECScale(&direction, &particle->vel,
                2.0f * (1.5f *
                    (0.7f + 0.3f *
                        (1.5258789e-05f * (f32)(u16)frand()))));
            particle->rot.z = 360.0f *
                (1.5258789e-05f * (f32)(u16)frand());
            particle->color.a = 255;
            particle->scale = 20.0f + 10.0f *
                (1.5258789e-05f * (f32)(u16)frand());
            particle->color.r = lbl_1_data_850[colorIndex].x;
            particle->color.g = lbl_1_data_850[colorIndex].y;
            particle->color.b = lbl_1_data_850[colorIndex].z;
        }
        system->initF = 1;
    }

    particle = system->data;
    for (particleIndex = 0; particleIndex < system->num;
         particleIndex++, particle++) {
        if (particle->time != 0) {
            VECAdd(&particle->pos, &particle->vel, &particle->pos);
            particle->vel.y -= 0.392f;
            particle->activeF--;
            if (particle->activeF <= 0) {
                particle->time = 0;
                particle->scale = 0.0f;
            } else if (particle->activeF < 10) {
                /* These particles have no extra fade during their final ten frames. */
            }
        }
    }
}

HuVecF lbl_1_data_8B0[8] = { { 128.0f, 128.0f, 128.0f }, { 2e+02f, 0.0f, 0.0f },
                             { 0.0f, 2e+02f, 0.0f },     { 0.0f, 0.0f, 2e+02f },
                             { 2e+02f, 2e+02f, 0.0f },   { 0.0f, 2e+02f, 2e+02f },
                             { 2e+02f, 0.0f, 2e+02f },   { 2e+02f, 1.5e+02f, 0.0f } };
/* Initializes and advances the second curved particle burst. */
void fn_1_ECC8(HU3D_MODEL *model, MBPARTICLE *system, Mtx matrix)
{
    HuVecF direction;
    MBPARTICLEDATA *particle;
    s32 particleIndex;
    u32 colorIndex;
    f32 angle;

    if (system->initF == 0) {
        particle = system->data;
        for (particleIndex = 0; particleIndex < system->num;
             particleIndex++, particle++) {
            colorIndex = mbRandMod(6);
            particle->time = 1;
            particle->activeF = 50.0f *
                (0.4f + 1.5258789e-05f * (f32)(u16)frand());
            particle->animBank = (s16)((s32)(16.0f *
                (1.5258789e-05f * (f32)(u16)frand())) & 0xF);
            angle = 360.0f *
                (3.0517578e-05f * (f32)(u16)frand() - 1.0f);
            direction.x = mbSinDeg(angle) - 4.0f;
            direction.z = 4.0f + mbCosDeg(angle);
            direction.y = 4.0f +
                mbSinDeg(360.0f *
                    (1.5258789e-05f * (f32)(u16)frand()));
            VECScale(&direction, &particle->pos,
                100.0f *
                (0.5f * (0.5f +
                    0.5f * (1.5258789e-05f * (f32)(u16)frand()))));
            VECScale(&direction, &particle->vel,
                2.0f * (1.5f *
                    (0.7f + 0.3f *
                        (1.5258789e-05f * (f32)(u16)frand()))));
            particle->rot.z = 360.0f *
                (1.5258789e-05f * (f32)(u16)frand());
            particle->color.a = 255;
            particle->scale = 20.0f + 10.0f *
                (1.5258789e-05f * (f32)(u16)frand());
            particle->color.r = lbl_1_data_8B0[colorIndex].x;
            particle->color.g = lbl_1_data_8B0[colorIndex].y;
            particle->color.b = lbl_1_data_8B0[colorIndex].z;
        }
        system->initF = 1;
    }

    particle = system->data;
    for (particleIndex = 0; particleIndex < system->num;
         particleIndex++, particle++) {
        if (particle->time != 0) {
            VECAdd(&particle->pos, &particle->vel, &particle->pos);
            particle->vel.y -= 0.392f;
            particle->activeF--;
            if (particle->activeF <= 0) {
                particle->time = 0;
                particle->scale = 0.0f;
            } else if (particle->activeF < 10) {
                /* These particles have no extra fade during their final ten frames. */
            }
        }
    }
}

HuVecF lbl_1_data_910[12] = { { 128.0f, 128.0f, 128.0f }, { 2e+02f, 0.0f, 0.0f },
                              { 0.0f, 2e+02f, 0.0f },     { 0.0f, 0.0f, 2e+02f },
                              { 2e+02f, 2e+02f, 0.0f },   { 0.0f, 2e+02f, 2e+02f },
                              { 2e+02f, 0.0f, 2e+02f },   { 2e+02f, 1.5e+02f, 0.0f },
                              { 192.0f, 64.0f, 64.0f },   { 0.0f, 131.0f, 131.0f },
                              { 64.0f, 64.0f, 192.0f },   { 131.0f, 131.0f, 0.0f } };
/* Initializes and advances the vertical particle burst. */
void fn_1_F1E8(HU3D_MODEL *model, MBPARTICLE *system, Mtx matrix)
{
    HuVecF direction;
    MBPARTICLEDATA *particle;
    s32 particleIndex;
    u32 colorIndex;
    f32 angle;
    f32 elevation;

    if (system->initF == 0) {
        particle = system->data;
        for (particleIndex = 0; particleIndex < system->num;
             particleIndex++, particle++) {
            colorIndex = mbRandMod(6);
            particle->time = 1;
            particle->activeF = 50.0f *
                (0.4f + 1.5258789e-05f * (f32)(u16)frand());
            particle->animBank = (s16)((s32)(16.0f *
                (1.5258789e-05f * (f32)(u16)frand())) & 0xF);
            angle = 360.0f *
                (3.0517578e-05f * (f32)(u16)frand() - 1.0f);
            elevation = 180.0f *
                (3.0517578e-05f * (f32)(u16)frand() - 1.0f);
            direction.x = mbCosDeg(angle) - mbCosDeg(elevation);
            direction.z = 0.0f;
            direction.y = 3.0f + (mbSinDeg(angle) - mbSinDeg(elevation));
            VECScale(&direction, &particle->pos,
                100.0f *
                (0.5f * (0.5f +
                    0.5f * (1.5258789e-05f * (f32)(u16)frand()))));
            VECScale(&direction, &particle->vel,
                2.0f * (1.5f *
                    (0.7f + 0.3f *
                        (1.5258789e-05f * (f32)(u16)frand()))));
            particle->rot.z = 90.0f;
            particle->color.a = 255;
            particle->scale = 5.0f + 10.0f *
                (1.5258789e-05f * (f32)(u16)frand());
            particle->color.r = lbl_1_data_910[colorIndex].x;
            particle->color.g = lbl_1_data_910[colorIndex].y;
            particle->color.b = lbl_1_data_910[colorIndex].z;
        }
        system->initF = 1;
    }

    particle = system->data;
    for (particleIndex = 0; particleIndex < system->num;
         particleIndex++, particle++) {
        if (particle->time != 0) {
            VECAdd(&particle->pos, &particle->vel, &particle->pos);
            particle->vel.y -= 0.588f;
            particle->activeF--;
            if (particle->activeF <= 0) {
                particle->time = 0;
                particle->scale = 0.0f;
            } else if (particle->activeF < 10) {
                /* These particles have no extra fade during their final ten frames. */
            }
        }
    }
}

/* Spawns rising white particles and fades them as their lifetime expires. */
void fn_1_F704(HU3D_MODEL *model, MBPARTICLE *system, Mtx matrix)
{
    HuVecF direction;
    MBPARTICLEDATA *particle;
    s32 particleIndex;
    f32 angle;
    f32 brightness;

    if (system->initF == 0) {
        particle = system->data;
        for (particleIndex = 0; particleIndex < system->num;
             particleIndex++, particle++) {
            particle->time = 1;
            particle->activeF = 50.0f *
                (0.4f + 0.3f *
                    (1.5258789e-05f * (f32)(u16)frand()));
            angle = 360.0f *
                (3.0517578e-05f * (f32)(u16)frand() - 1.0f);
            direction.x = mbSinDeg(angle);
            direction.z = mbCosDeg(angle);
            direction.y = 1.0f +
                mbSinDeg(360.0f *
                    (1.5258789e-05f * (f32)(u16)frand()));
            if (direction.y > 1.0f) {
                direction.y -= 2.0f;
            }
            brightness = 0.5f + 0.5f * direction.y;
            brightness *= 0.7f;
            VECScale(&direction, &particle->pos,
                100.0f *
                (0.5f * (0.5f +
                    0.5f * (1.5258789e-05f * (f32)(u16)frand()))));
            VECScale(&direction, &particle->vel,
                2.0f * (1.5f *
                    (0.7f + 0.3f *
                        (1.5258789e-05f * (f32)(u16)frand()))));
            particle->rot.z = 360.0f *
                (1.5258789e-05f * (f32)(u16)frand());
            particle->scale = 60.0f + 60.0f *
                (1.5258789e-05f * (f32)(u16)frand());
            particle->color.a = 20.0f + 80.0f *
                (1.5258789e-05f * (f32)(u16)frand());
            /* The calculated shade is ignored; every particle starts white. */
            angle = 0.7f * brightness + 0.3f *
                (1.5258789e-05f * (f32)(u16)frand());
            particle->color.r = 255;
            particle->color.g = 255;
            particle->color.b = 255;
        }
        system->initF = 1;
    }

    particle = system->data;
    for (particleIndex = 0; particleIndex < system->num;
         particleIndex++, particle++) {
        if (particle->time != 0) {
            VECAdd(&particle->pos, &particle->vel, &particle->pos);
            particle->vel.y += 0.392f;
            particle->activeF--;
            if (particle->activeF <= 0) {
                particle->time = 0;
                particle->scale = 0.0f;
                particle->color.a = 0;
            } else if (particle->activeF < 10 && particle->color.a >= 10) {
                particle->color.a -= 10;
            }
            if (particle->color.a >= 10) {
                particle->color.a -= 2;
            }
            particle->scale += 4.0f;
        }
    }
}

/* Updates the colored sparkle burst attached to the launch model during transport. */
void fn_1_FCB4(HU3D_MODEL *model, MBPARTICLE *system, Mtx matrix)
{
    Mtx modelMatrix;
    GXColor colorTable[5] = { { 255, 255, 255, 255 },
                              { 255, 255, 0, 255 },
                              { 128, 255, 0, 255 },
                              { 128, 255, 255, 255 },
                              { 255, 128, 255, 255 } };
    HuVecF rotation;
    MBPARTICLEDATA *particle;
    s32 particleIndex;
    s32 modelId;
    f32 angle;
    f32 radius;

    modelId = mbObjModelIDGet((s16)lbl_1_bss_4D4.finalModels[2]);
    Hu3DModelObjMtxGet(modelId, "fook", modelMatrix);
    Hu3DMtxRotGet(modelMatrix, &rotation);
    particle = system->data;
    for (particleIndex = 0; particleIndex < system->num;
         particleIndex++, particle++) {
        if (particle->time == 0) {
            particle->activeF = (s16)(50.0f *
                (0.45f + 0.45f *
                 (1.5258789e-05f * (f32)(u16)frand())));
            angle = 360.0f * frandf();
            radius = 100.0f *
                     (0.5f * frandf());
            particle->pos.x = modelMatrix[0][3] +
                                  radius * mbCosDeg(angle);
            particle->pos.y = modelMatrix[1][3];
            particle->pos.z = modelMatrix[2][3] +
                                  radius * mbSinDeg(angle);
            particle->vel.x = 0.0f;
            particle->vel.y = 0.0f;
            particle->vel.z = 0.0f;
            particle->scale = 10.0f +
                             40.0f * frandf();
            {
                particle->color = colorTable[mbRandMod(5)];
            }
            particle->time = 1;
        } else {
            particle->pos.x += 0.016666668f * particle->vel.x;
            particle->pos.y += 0.016666668f * particle->vel.y;
            particle->pos.z += 0.016666668f * particle->vel.z;
            particle->vel.x *= 0.9f;
            particle->vel.y *= 0.9f;
            particle->vel.z *= 0.9f;
            particle->color.a -= 5;
            if (particle->color.a == 0) {
                particle->color.a = 0;
            }
            particle->scale -= 0.5f;
            if (particle->scale <= 0.0f) {
                particle->scale = 0.0f;
            }
            if (particle->activeF-- <= 0) {
                particle->time = 0;
            }
        }
    }
}

/* Seeds and advances a radial particle burst when ParticleDraw invokes its hook. */
void fn_1_100CC(HU3D_MODEL *model, MBPARTICLE *system, Mtx matrix)
{
    GXColor initialColor = {255, 255, 192, 192};
    MBPARTICLEDATA *particle;
    s32 particleIndex;
    s32 activeCount;
    f32 angle;
    f32 radius;

    if (system->mode == 0) {
        particle = system->data;
        for (particleIndex = 0; particleIndex < system->num;
             particleIndex++, particle++) {
            if (lbl_1_bss_8 == 0) {
                particle->activeF = 0;
            } else {
                particle->activeF = (s16)(50.0f *
                    (0.1f + 0.3f *
                     (1.5258789e-05f * (f32)(u16)frand())));
            }
            angle = 360.0f * frandf();
            radius = 100.0f * (0.5f * frandf());
            particle->pos.x = radius * mbCosDeg(angle);
            particle->pos.y = 0.0f;
            particle->pos.z = radius * mbSinDeg(angle);
            particle->vel.x = 100.0f *
                                  (10.0f * mbCosDeg(angle));
            particle->vel.y = 0.0f;
            particle->vel.z = 100.0f *
                                  (10.0f * mbSinDeg(angle));
            particle->scale = 100.0f +
                             20.0f * frandf();
            particle->color = initialColor;
            particle->color.a = (u8)(20.0f +
                80.0f * (1.5258789e-05f * (f32)(u16)frand()));
        }
        system->mode = 1;
    }

    activeCount = 0;
    particle = system->data;
    for (particleIndex = 0; particleIndex < system->num;
         particleIndex++, particle++) {
        if (particle->activeF != 0) {
            particle->pos.x += 0.016666668f * particle->vel.x;
            particle->pos.y += 0.016666668f * particle->vel.y;
            particle->pos.z += 0.016666668f * particle->vel.z;
            particle->vel.x *= 0.9f;
            particle->vel.y *= 0.9f;
            particle->vel.z *= 0.9f;
            particle->color.a -= 5;
            if (particle->color.a == 0) {
                particle->color.a = 0;
            }
            particle->scale -= 0.5f;
            if (particle->scale <= 0.0f) {
                particle->scale = 0.0f;
            }
            activeCount++;
            particle->activeF--;
        }
    }
    if (activeCount == 0) {
        system->mode = 0;
    }
}

/* Initializes and advances the radial particles attached to coin and star awards. */
void fn_1_104FC(HU3D_MODEL *model, MBPARTICLE *system, Mtx matrix)
{
    GXColor colorTable[5] = { { 255, 255, 255, 255 },
                              { 255, 255, 0, 255 },
                              { 128, 255, 0, 255 },
                              { 128, 255, 255, 255 },
                              { 255, 128, 255, 255 } };
    MBPARTICLEDATA *particle;
    s32 particleIndex;
    s32 activeCount;
    f32 angle;
    f32 radius;
    f32 elevation;

    if (system->mode == 0) {
        particle = system->data;
        for (particleIndex = 0; particleIndex < system->num;
             particleIndex++, particle++) {
            particle->activeF = (s16)(50.0f *
                (0.3f + 0.4f *
                 (1.5258789e-05f * (f32)(u16)frand())));
            angle = 360.0f * frandf();
            elevation = 45.0f * frandf();
            radius = 100.0f * (0.5f * frandf());
            particle->pos.x = radius * mbCosDeg(angle);
            particle->pos.y = 0.0f;
            particle->pos.z = radius * mbSinDeg(angle);
            particle->vel.x = mbCosDeg(angle) *
                (3000.0f * (0.5f + mbSinDeg(elevation)));
            particle->vel.y = mbSinDeg(angle) *
                (3000.0f * (0.5f + mbSinDeg(elevation)));
            particle->vel.z = 0.0f;
            particle->scale = 30.0f + 30.0f * frandf();
            particle->color = colorTable[mbRandMod(5)];
        }
        system->mode = 1;
    }

    activeCount = 0;
    particle = system->data;
    for (particleIndex = 0; particleIndex < system->num;
         particleIndex++, particle++) {
        if (particle->activeF != 0) {
            particle->pos.x += 0.016666668f * particle->vel.x;
            particle->pos.y += 0.016666668f * particle->vel.y;
            particle->pos.z += 0.016666668f * particle->vel.z;
            particle->vel.x *= 0.9f;
            particle->vel.y *= 0.9f;
            particle->vel.z *= 0.9f;
            particle->activeF--;
            if (particle->activeF <= 0) {
                particle->time = 0;
                particle->scale = 0.0f;
                particle->color.a = 0;
            } else if (particle->activeF < 10 &&
                       particle->color.a >= 10) {
                particle->color.a -= 5;
            }
            if (particle->color.a >= 10) {
                particle->color.a -= 2;
            }
            activeCount++;
        }
    }
}

/* Shows the player display and hides the alternate display at turn initialization. */
void fn_1_1097C(int playerNo) {
    mbObjDispSet((s16) (*(s32 *)((s8 *)(&lbl_1_bss_4D4) + (W03_OFFSET_MOTION_MODEL))), 1);
    mbObjDispSet((s16) (*(s32 *)((s8 *)(&lbl_1_bss_4D4) + (W03_OFFSET_FINAL_MODEL_5))), 0);
}

s8 lbl_1_data_9A0[4] = {12, 6, 7, -1};
/* When the star-space hook runs, offers stars and plays the successive award animations. */
void fn_1_109C8(void)
{
    W03STARAWARD stars[8];
    char priceText[16];
    char quantityText[16];
    HuVecF guidePosition;
    HuVecF playerPosition;
    HuVecF fallingPosition;
    HuVecF awardPosition;
    int returnSpace;
    int starCount;
    int coinCount;
    int guideSpace;
    int starDisplay;
    int lastQuantity;
    int totalCost;
    int animationControl;
    int animationWork;
    int originalSpace;
    int awardedCount;
    int previousDirection;
    int helpWindow;
    int jingle;
    int messageOffset;
    OMOBJ *guide;
    int guideCharacter;
    int panelVisible;
    int maxQuantity;
    int direction;
    int repeatFrames;
    int animationFrame;
    int quantity;
    int index;
    int buttons;
    int stickY;
    int playerNo;
    float panelScale;
    float progress;
    float starScale;

    if (GwSystem.curTime == 0) {
        guideCharacter = 6;
        messageOffset = 0;
    } else {
        guideCharacter = 7;
        messageOffset = 1;
    }
    if (GwSystem.curTime == 0) {
        lbl_1_bss_4D0->nightReward = 20;
    }
    panelVisible = 0;
    playerNo = GwSystem.turnPlayerNo;
    originalSpace = GwPlayer[playerNo].masuId;
    returnSpace = originalSpace;
    starCount = mbPlayerStarGet(playerNo);
    coinCount = mbPlayerCoinGet(playerNo);
    mbMoveNumDispSet(playerNo, FALSE);
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    if (starCount >= 999) {
        mbWinCreate(2, MESSNUM(MESS_BOARD_STAR, 10) + messageOffset, -1);
        mbWinTopWait();
    } else {
        sprintf(priceText, "%d", lbl_1_bss_4D0->nightReward);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 45), -1);
        mbWinTopInsertMesSet((u32)priceText, 0);
        mbWinTopWait();
        if (coinCount < lbl_1_bss_4D0->nightReward) {
            mbPlayerWinLoseVoicePlay(playerNo, 13, CHARVOICEID(12));
            mbPlayerMotionShiftSet(playerNo, 13, 0.0f, 8.0f, 0);
            mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 47), -1);
            mbWinTopWait();
            mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        } else {
            mbWinCreateChoice(2, MESSNUM(MESS_BOARD_W03, 46), -1, 0);
            if (GwPlayer[playerNo].comF) {
                mbComChoiceUpSet();
            }
            mbWinTopWait();
            if (mbWinTopChoiceGet() != 1 && mbWinTopChoiceGet() != -1) {
                mbMusFadeOutSpeed(0, 1000);
                mbCameraPlayerViewSet(playerNo, 0);
                mbStatusDispSetAll(FALSE);
                while (!mbStatusOffCheckAll()) {
                    HuPrcVSleep();
                }
                mbStatusDispFocusSet(playerNo, TRUE);
                while (!mbStatusMoveCheck(playerNo)) {
                    HuPrcVSleep();
                }
                guideSpace = mbMasuFind_MAttrIdGet(-1, (1 << 11));
                GwPlayer[playerNo].masuIdPrev = originalSpace;
                mbPlayerMasuMoveTo(playerNo, guideSpace, TRUE);
                GwPlayer[playerNo].masuId = guideSpace;
                mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
                HuPrcSleep(30);
                mbMusPlay(1, 12, 127, 0);
                mbPlayerPosGet(playerNo, &playerPosition);
                guidePosition = playerPosition;
                guidePosition.y += 200.0f;
                guidePosition.z -= 200.0f;
                guide = mbGuideCreateFlag(&guidePosition, lbl_1_data_9A0, FALSE, TRUE, FALSE);
                mbGuideMotionNextSet(guide, 1);
                lbl_1_bss_4D4.diceModel = (MBMODELID)mbGuideModelGet(guide);
                mbAudGuidePlay(MSM_SE_GUIDE_25);
                mbGuideMotionShiftSet(guide, 12, TRUE);
                mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 48) + messageOffset, guideCharacter);
                mbWinTopWait();
                mbAudGuidePlay(MSM_SE_GUIDE_26);
                mbGuideMotionShiftSet(guide, 12, TRUE);
                mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 50) + messageOffset, guideCharacter);
                mbWinTopInsertMesSet((u32)priceText, 0);
                mbWinTopWait();
                mbAudGuidePlay(MSM_SE_GUIDE_26);
                mbGuideMotionShiftSet(guide, 12, TRUE);
                mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 52) + messageOffset, guideCharacter);
                mbWinTopWait();

selectQuantity:
                maxQuantity = coinCount / lbl_1_bss_4D0->nightReward;
                if (maxQuantity >= 5) {
                    maxQuantity = 5;
                }
                lastQuantity = quantity = 1;
                if (panelVisible == 0) {
                    espDispOn((s16)lbl_1_bss_4D4.largeDisplaySprites[0]);
                    espDispOn((s16)lbl_1_bss_4D4.largeDisplaySprites[1]);
                    for (index = 1; index < 3; index++) {
                        espDispOn((s16)lbl_1_bss_4D4.largeDisplayDigits[index]);
                    }
                    espBankSet((s16)lbl_1_bss_4D4.largeDisplayDigits[1], 10);
                    espBankSet((s16)lbl_1_bss_4D4.largeDisplayDigits[2], 1);
                    for (animationFrame = 0; (float)animationFrame <= 10.0f; animationFrame++) {
                        progress = (float)animationFrame / 10.0f;
                        panelScale = mbSinDeg(90.0f * progress);
                        espScaleSet((s16) lbl_1_bss_4D4.largeDisplaySprites[0], 0.8f,
                                    0.8f * panelScale);
                        espScaleSet((s16) lbl_1_bss_4D4.largeDisplaySprites[1], 0.77f,
                                    0.77f * panelScale);
                        for (index = 0; index < 3; index++) {
                            espScaleSet((s16)lbl_1_bss_4D4.largeDisplayDigits[index], 0.7f,
                                0.7f * panelScale);
                        }
                        HuPrcVSleep();
                    }
                    helpWindow = mbWinCreateHelp(MESSNUM(MESS_BOARD_W03, 54));
                    mbWinPosSet(helpWindow, 115, 288);
                    panelVisible = 1;
                }
                direction = previousDirection = repeatFrames = 0;
                for (;;) {
                    buttons = HuPadBtnDown[GwPlayer[playerNo].padNo];
                    stickY = mbPadStkYGet(GwPlayer[playerNo].padNo);
                    if (GwPlayer[playerNo].comF) {
                        buttons = 0;
                        stickY = 0;
                        if (quantity < maxQuantity) {
                            stickY = 32;
                        } else {
                            HuPrcSleep(5);
                            buttons = PAD_BUTTON_A;
                        }
                    }
                    if (buttons & PAD_BUTTON_A) {
                        mbAudFXPlay(2);
                        break;
                    } else if (buttons & PAD_BUTTON_B) {
                        if (panelVisible) {
                            for (animationFrame = 0; (float)animationFrame <= 10.0f;
                                 animationFrame++) {
                                progress = (float)animationFrame / 10.0f;
                                panelScale = mbCosDeg(90.0f * progress);
                                espScaleSet((s16)lbl_1_bss_4D4.largeDisplaySprites[0], 0.8f,
                                    0.8f * panelScale);
                                espScaleSet((s16)lbl_1_bss_4D4.largeDisplaySprites[1], 0.77f,
                                    0.77f * panelScale);
                                for (index = 0; index < 3; index++) {
                                    espScaleSet((s16)lbl_1_bss_4D4.largeDisplayDigits[index], 0.7f,
                                        0.7f * panelScale);
                                }
                                HuPrcVSleep();
                            }
                            espDispOff((s16)lbl_1_bss_4D4.largeDisplaySprites[0]);
                            espDispOff((s16)lbl_1_bss_4D4.largeDisplaySprites[1]);
                            for (index = 0; index < 3; index++) {
                                espDispOff((s16)lbl_1_bss_4D4.largeDisplayDigits[index]);
                            }
                            mbWinKill(helpWindow);
                            panelVisible = 0;
                        }
                        mbAudGuidePlay(MSM_SE_GUIDE_26);
                        mbGuideMotionShiftSet(guide, 12, TRUE);
                        mbWinCreateChoice(2, MESSNUM(MESS_BOARD_W03, 59) + messageOffset,
                                          guideCharacter, 0);
                        if (GwPlayer[playerNo].comF) {
                            mbComChoiceDownSet();
                        }
                        mbWinTopWait();
                        if (mbWinTopChoiceGet() != 1 && mbWinTopChoiceGet() != -1) {
                            mbAudGuidePlay(MSM_SE_GUIDE_27);
                            mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 61) + messageOffset,
                                        guideCharacter);
                            mbWinTopWait();
                            goto finishPurchase;
                        } else {
                            goto selectQuantity;
                        }
                    } else {
                        if (abs(stickY) >= 8) {
                            if (stickY >= 10) {
                                direction = 1;
                            }
                            if (stickY <= -10) {
                                direction = -1;
                            }
                            if (direction == 0) {
                                repeatFrames = 0;
                            } else if (previousDirection == direction) {
                                if ((float)++repeatFrames > 25.0f) {
                                    quantity += direction;
                                    repeatFrames -= 2;
                                }
                            } else {
                                quantity += direction;
                                repeatFrames = 0;
                            }
                            previousDirection = direction;
                            if (quantity < 1) {
                                quantity = 1;
                            } else if (quantity > 99) {
                                quantity = 99;
                            } else if (quantity > maxQuantity) {
                                quantity = maxQuantity;
                            }
                        } else {
                            previousDirection = direction = repeatFrames = 0;
                        }
                        if (lastQuantity != quantity) {
                            mbAudFXPlay(0);
                            espBankSet((s16)lbl_1_bss_4D4.largeDisplayDigits[2], quantity);
                            lastQuantity = quantity;
                            if ((float)repeatFrames < 22.5f) {
                                HuPrcSleep(3);
                            }
                        }
                        HuPrcVSleep();
                        continue;
                    }
                }
                if (panelVisible) {
                    for (animationFrame = 0; (float)animationFrame <= 10.0f;
                         animationFrame++) {
                        progress = (float)animationFrame / 10.0f;
                        panelScale = mbCosDeg(90.0f * progress);
                        espScaleSet((s16)lbl_1_bss_4D4.largeDisplaySprites[0], 0.8f,
                            0.8f * panelScale);
                        espScaleSet((s16)lbl_1_bss_4D4.largeDisplaySprites[1], 0.77f,
                            0.77f * panelScale);
                        for (index = 0; index < 3; index++) {
                            espScaleSet((s16)lbl_1_bss_4D4.largeDisplayDigits[index], 0.7f,
                                0.7f * panelScale);
                        }
                        HuPrcVSleep();
                    }
                    espDispOff((s16)lbl_1_bss_4D4.largeDisplaySprites[0]);
                    espDispOff((s16)lbl_1_bss_4D4.largeDisplaySprites[1]);
                    for (index = 0; index < 3; index++) {
                        espDispOff((s16)lbl_1_bss_4D4.largeDisplayDigits[index]);
                    }
                    mbWinKill(helpWindow);
                    panelVisible = 0;
                }
                totalCost = quantity * lbl_1_bss_4D0->nightReward;
                sprintf(quantityText, "%d", quantity);
                sprintf(priceText, "%d", totalCost);
                mbAudGuidePlay(MSM_SE_GUIDE_25);
                mbGuideMotionShiftSet(guide, 12, TRUE);
                mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 55) + messageOffset, guideCharacter);
                mbWinTopInsertMesSet((u32)quantityText, 0);
                mbWinTopInsertMesSet((u32)priceText, 1);
                mbWinTopWait();
                mbAudGuidePlay(MSM_SE_GUIDE_26);
                mbGuideMotionShiftSet(guide, 12, TRUE);
                mbWinCreateChoice(2, MESSNUM(MESS_BOARD_W03, 57) + messageOffset, guideCharacter,
                                  0);
                if (GwPlayer[playerNo].comF) {
                    mbComChoiceUpSet();
                }
                mbWinTopWait();
                if (mbWinTopChoiceGet() != 1 && mbWinTopChoiceGet() != -1) {
                    mbAudGuidePlay(MSM_SE_GUIDE_25);
                    mbGuideMotionShiftSet(guide, 12, TRUE);
                    mbWinCreate(2, MESSNUM(MESS_BOARD_STAR, 6) + messageOffset, guideCharacter);
                    mbWinTopWait();
                    mbPlayerRotateStart(playerNo, 0, 15);
                    while (!mbPlayerRotateCheck(playerNo)) {
                        HuPrcVSleep();
                    }
                    mbCoinAddExec(playerNo, -totalCost);
                    mbObjMotionShiftSet((s16)lbl_1_bss_4D4.diceModel, 6,
                        0.0f, 8.0f, 0);
                    HuPrcSleep(60);
                    mbPlayerPosGet(playerNo, &awardPosition);
                    awardedCount = animationWork = animationControl = 0;
                    for (index = 0; index < 8; index++) {
                        stars[index].objectId = mbStarObjCreate();
                        stars[index].active = 1;
                        stars[index].frame = 0;
                        stars[index].delay = index * 90;
                        stars[index].position = awardPosition;
                        stars[index].position.y += 350.0f;
                        stars[index].scale.x = stars[index].scale.y = stars[index].scale.z = 0.0f;
                        stars[index].rotation.x = stars[index].rotation.y =
                            stars[index].rotation.z = 0.0f;
                        mbStarObjPosSetV(stars[index].objectId, &stars[index].position);
                        mbStarObjScaleSetV(stars[index].objectId, &stars[index].scale);
                        mbStarObjRotSetV(stars[index].objectId, &stars[index].rotation);
                        mbStarObjDispSet(stars[index].objectId, FALSE);
                        stars[index].phase = 0;
                    }
                    while (awardedCount < quantity) {
                        for (index = 0; index < quantity; index++) {
                            if (--stars[index].delay <= 0 && stars[index].active != 0) {
                                stars[index].frame++;
                                switch (stars[index].phase) {
                                case 0:
                                    if (stars[index].frame == 1) {
                                        stars[index].soundId = mbAudFXPlay(MSM_SE_BRD00_91);
                                        mbAudFXPlay(MSM_SE_BRD00_92);
                                        mbStarObjDispSet(stars[index].objectId, TRUE);
                                    }
                                    progress = (float)stars[index].frame / 60.0f;
                                    if (progress >= 1.0f) {
                                        progress = 1.0f;
                                        stars[index].phase = 1;
                                        stars[index].frame = 0;
                                    }
                                    starScale = mbSinDeg(90.0f * progress);
                                    stars[index].scale.z = starScale;
                                    stars[index].scale.y = starScale;
                                    stars[index].scale.x = starScale;
                                    stars[index].rotation.y =
                                        720.0f * mbSinDeg(90.0f * progress);
                                    break;
                                case 1:
                                    if (stars[index].frame >= 10) {
                                        mbAudFXStop(stars[index].soundId);
                                        stars[index].phase = 2;
                                        stars[index].frame = 0;
                                    }
                                    break;
                                case 2:
                                    progress = (float)stars[index].frame / 90.0f;
                                    if (stars[index].frame == 1) {
                                        mbAudFXPlay(MSM_SE_BRD00_93);
                                    }
                                    if (stars[index].frame == 30) {
                                        mbStarObjDispFlagSet(stars[index].objectId, FALSE);
                                    }
                                    if (stars[index].frame == 45) {
                                        omVibrate(playerNo, 20, 7, 3);
                                        mbPlayerStarAdd(playerNo, 1);
                                    }
                                    if (progress >= 1.0f) {
                                        progress = 1.0f;
                                        stars[index].phase = 3;
                                        stars[index].frame = 0;
                                        stars[index].active = 0;
                                        awardedCount++;
                                        mbStarObjDispSet(stars[index].objectId, FALSE);
                                    }
                                    starScale = mbCosDeg(90.0f * (2.0f * progress));
                                    if (starScale <= 0.0f) {
                                        starScale = 0.0f;
                                    }
                                    stars[index].scale.z = starScale;
                                    stars[index].scale.y = starScale;
                                    stars[index].scale.x = starScale;
                                    stars[index].rotation.y =
                                        720.0f * mbSinDeg(90.0f * progress);
                                    fallingPosition = stars[index].position;
                                    mbStarObjPosSet(stars[index].objectId, fallingPosition.x,
                                        fallingPosition.y -
                                        (100.0f * (3.5f * mbSinDeg(90.0f * progress))),
                                        fallingPosition.z);
                                    break;
                                case 3:
                                    break;
                                }
                                mbStarObjScaleSetV(stars[index].objectId, &stars[index].scale);
                                mbStarObjRotSetV(stars[index].objectId, &stars[index].rotation);
                            }
                        }
                        HuPrcVSleep();
                    }
                    mbMusFadeOutSpeed(1, 1000);
                    while (mbMusCheck(1)) {
                        HuPrcVSleep();
                    }
                    HuPrcSleep(10);
                    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 1.0f, HU3D_MOTATTR_LOOP);
                    mbObjMotionShiftSet((s16)lbl_1_bss_4D4.diceModel, 1,
                        0.0f, 8.0f, HU3D_MOTATTR_LOOP);
                    jingle = mbMusJinglePlay(39);
                    starDisplay = mbStarDispPlayerCreate(playerNo, quantity);
                    mbPlayerWinLoseVoicePlay(playerNo, 7, MSM_SE_CHARVOICE_MARIO);
                    mbPlayerMotionShiftSet(playerNo, 7, 0.0f, 8.0f, 0);
                    mbMusJingleWait(jingle);
                    mbPlayerMotionEndWait(playerNo);
                    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
                    HuPrcSleep(10);
                    mbMusPlay(1, 12, 127, 0);
                    mbAudGuidePlay(MSM_SE_GUIDE_25);
                    mbGuideMotionSet(guide, 7, TRUE);
                    mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 61) + messageOffset, guideCharacter);
                    mbWinTopWait();
                    for (index = 0; index < 8; index++) {
                        mbStarObjKill(stars[index].objectId);
                    }
                } else {
                    goto selectQuantity;
                }
finishPurchase:
                mbMusFadeOutSpeed(1, 1000);
                mbGuideEnd(guide, TRUE);
                mbStatusDispFocusSet(playerNo, FALSE);
                while (!mbStatusMoveCheck(playerNo)) {
                    HuPrcVSleep();
                }
                mbStatusDispSetAll(TRUE);
                mbPlayerRotateStart(playerNo, 0, 15);
                while (!mbPlayerRotateCheck(playerNo)) {
                    HuPrcVSleep();
                }
                mbCameraPlayerViewSet(playerNo, 2);
                GwPlayer[playerNo].masuIdPrev = originalSpace;
                mbPlayerMasuMoveTo(playerNo, returnSpace, TRUE);
                GwPlayer[playerNo].masuId = returnSpace;
                mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
                mbMusBoardPlay();
            }
        }
    }
    mbMoveNumDispSet(playerNo, TRUE);
}

/* Returns the next star space from the player's current space. */

s16 fn_1_12220(int playerNo) {
    return mbMasuFind_TypeIdGet(GwPlayer[playerNo].masuId, 7, TRUE, TRUE);
}

/* Chooses the nearest space when requested, or favors qualifying coin/star spaces before
 * randomizing. */
int fn_1_12268(int playerNo, int masuCount, s16 *masuIds, BOOL chooseNearest) {
    s16 candidateMasu[5];
    s16 stepCounts[5];
    s32 candidateCount = 0;
    s32 coinCount;
    s32 starCount;
    s32 randomValue;
    s32 i;
    s32 j;
    s32 nearestSteps;
    s32 nearestSpace;
    s16 attributes;
    s16 candidateAttributes;

    for (i = 0; i < 5; i++) {
        candidateMasu[i] = 0;
    }
    coinCount = mbPlayerCoinGet(playerNo);
    starCount = mbPlayerStarGet(playerNo);

    if (chooseNearest != 0) {
        for (i = 0; i < masuCount; i++) {
            stepCounts[i] = (s16)mbMasuFind_TypeStepGet(masuIds[i], 7);
        }
        {
            nearestSteps = 10000;
            nearestSpace = -1;
            for (i = 0; i < masuCount; i++) {
                if (nearestSteps > stepCounts[i]) {
                    nearestSteps = stepCounts[i];
                    nearestSpace = i;
                }
            }
            return nearestSpace;
        }
    }

    for (i = 0; i < masuCount; i++) {
        attributes = (s16)mbMasuMAttrGet(masuIds[i]);
        if ((attributes & 0x2000) != 0) {
            for (j = 0; j < masuCount; j++) {
                candidateAttributes = (s16)mbMasuMAttrGet(masuIds[j]);
                if ((candidateAttributes & 0x2000) == 0) {
                    candidateMasu[candidateCount] = (s16)j;
                    candidateCount++;
                }
            }
            randomValue = (s32)mbRandMod(10);
            if (GwSystem.curTime == 0) {
                if (coinCount >= 20 && randomValue >= 2) {
                    return i;
                }
                return candidateMasu[mbRandMod((u32)candidateCount)];
            }
            if (coinCount >= lbl_1_bss_4D0->nightReward && randomValue >= 5) {
                return i;
            }
            return candidateMasu[mbRandMod((u32)candidateCount)];
        }
        if ((attributes & 0x4000) != 0) {
            for (j = 0; j < masuCount; j++) {
                candidateAttributes = (s16)mbMasuMAttrGet(masuIds[j]);
                if ((candidateAttributes & 0x4000) == 0) {
                    candidateMasu[candidateCount] = (s16)j;
                    candidateCount++;
                }
            }
            if (starCount >= 1) {
                return i;
            }
            return candidateMasu[mbRandMod((u32)candidateCount)];
        }
    }
    return (s32)mbRandMod((u32)masuCount);
}

HuVecF lbl_1_data_9A4 = {0.0f, 6.5e+02f, 1.77e+03f};
HuVecF lbl_1_data_9B0 = {-35.0f, 0.0f, 0.0f};
f32 lbl_1_data_9BC[4] = {1.8e+03f, 0.0f, 2.5e+02f, 0.0f};
/* Plays the guide's four-message introduction when the opening-install hook runs. */
void fn_1_124FC(void) {
    HuVecF masuPosition;
    HuVecF guidePosition;
    s32 windowNo;
    s16 masuId;
    s16 guideObjectId;

    mbAudGuidePlay(MSM_SE_GUIDE_28);
    windowNo = mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 69), 6);
    mbWinPlayerDisable(windowNo, -1);
    mbWinTopWait();
    windowNo = -1;
    mbWipeDissolveFadeOutTime(1);
    masuId = mbMasuFind_MAttrIdGet(-1, (1 << 11));
    mbMasuPosGet(masuId, &masuPosition);
    mbCameraZoomSet(lbl_1_data_9BC[0]);
    mbCameraRotSetV(&lbl_1_data_9B0);
    mbCameraCenterSetV(&lbl_1_data_9A4);
    guidePosition.x = lbl_1_data_9A4.x;
    guidePosition.y = 200.0f + masuPosition.y;
    guidePosition.z = lbl_1_data_9A4.z - 4e+02f;
    guideObjectId = mbOpeningGuideObjIdGet();
    mbObjPosSetV(guideObjectId, &guidePosition);
    mbWipeDissolveFadeIn();
    mbAudGuidePlay(MSM_SE_GUIDE_28);
    windowNo = mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 70), 6);
    mbWinPlayerDisable(windowNo, -1);
    mbWinTopWait();
    windowNo = -1;
    mbAudGuidePlay(MSM_SE_GUIDE_28);
    windowNo = mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 71), 6);
    mbWinPlayerDisable(windowNo, -1);
    mbWinTopWait();
    windowNo = -1;
    mbAudGuidePlay(MSM_SE_GUIDE_28);
    windowNo = mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 72), 6);
    mbWinPlayerDisable(windowNo, -1);
    mbWinTopWait();
    windowNo = -1;
    mbWipeDissolveFadeOutTime(1);
    mbOpeningCameraPosRestore();
    mbOpeningGuidePosRestore();
    mbWipeDissolveFadeIn();
}

/* Shows the guide's board introduction and returns to the opening camera afterward. */
void fn_1_126C0(void) {
    HuVecF startPosition;
    HuVecF cameraPosition;
    HuVecF guidePosition;
    f32 progress;
    s32 frame;
    s32 windowNo;
    s32 guideObjectId;
    s16 startMasuId;
    s16 guideMasuId;

    mbWipeFadeOut();
    for (frame = 0; frame < 4; frame++) {
        mbPlayerMotionShiftSet(frame, 1, 0.0f, 1.0f, HU3D_MOTATTR_LOOP);
    }
    mbCameraZoomSet(2400.0f);
    mbCameraRotSet(-35.0f, 0.0f, 0.0f);
    startMasuId = mbMasuFind_AttrIdGet(-1, MASU_FLAG_START);
    mbMasuPosGet(startMasuId, &startPosition);
    guideMasuId = mbMasuFind_MAttrIdGet(-1, (1 << 11));
    mbMasuPosGet(guideMasuId, &cameraPosition);
    mbWipeFadeIn();
    HuPrcSleep(10);
    mbAudGuidePlay(MSM_SE_GUIDE_28);
    windowNo = mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 73), 6);
    mbWinPlayerDisable(windowNo, -1);
    guidePosition = cameraPosition;
    guidePosition.y += 200.0f;
    guidePosition.z -= 200.0f;
    cameraPosition.y += 100.0f;
    cameraPosition.z -= 100.0f;
    mbCameraMovePos(&cameraPosition, NULL, NULL, 2400.0f, -1.0f, 150);
    mbCameraMoveWait();
    mbWinTopWait();
    windowNo = -1;
    HuPrcSleep(20);
    mbCameraMovePos(NULL, NULL, NULL, 1800.0f, -1.0f, 30);
    mbCameraMoveWait();
    guideObjectId = mbOpeningGuideObjIdGet();
    for (frame = 0; frame < 100.0f; frame++) {
        progress = frame / 100.0f;
        mbObjPosSet(guideObjectId, guidePosition.x,
            guidePosition.y + 100.0f * (6.0f * mbCosDeg(90.0f * progress)),
            guidePosition.z);
        HuPrcVSleep();
    }
    mbObjMotionShiftSet(guideObjectId, 12, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbAudGuidePlay(MSM_SE_GUIDE_28);
    windowNo = mbWinCreate(2, MESSNUM(MESS_BOARD_W03, 74), 6);
    mbWinPlayerDisable(windowNo, -1);
    mbWinTopWait();
    windowNo = -1;
    mbWipeFadeOut();
    mbOpeningGuidePosRestore();
    mbOpeningCameraPosRestore();
    mbWipeFadeIn();
}

s32 lbl_1_data_9CC[4][2] = {{986, 960}, {987, 961}, {988, 962}, {-1, -1}};
/* Board events call this with a cue index to play its day or night voice effect. */
int fn_1_12A14(s32 cueIndex) {
    s32 timeSlot;

    if (GwSystem.curTime == FALSE) {
        timeSlot = 0;
    } else {
        timeSlot = 1;
    }
    return mbAudFXPlay((s16)lbl_1_data_9CC[cueIndex][timeSlot]);
}
