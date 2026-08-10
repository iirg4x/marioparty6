#include "math.h"
#include "dolphin/pad.h"
#include "datadir_enum.h"
#include "datanum/charmot.h"
#include "game/board/audio.h"
#include "game/board/camera.h"
#include "game/board/capsule.h"
#include "game/board/comchoice.h"
#include "game/board/coin.h"
#include "game/board/main.h"
#include "game/board/masu.h"
#include "game/board/object.h"
#include "game/board/player.h"
#include "game/board/window.h"
#include "game/charman.h"
#include "game/esprite.h"
#include "game/flag.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/process.h"
#include "messdir_enum.h"

#define CAPSPECIAL_MASU_ATTR_TERESA_LINK (1 << 13)
#define CAPSPECIAL_CAPSULE_LIGHT 31
#define CAPSPECIAL_FADE_OBJ_PRIORITY (-32768)

#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET0_MODEL \
    DATANUM(DATA_capsulechar4, 1)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET0_MOTION1 \
    DATANUM(DATA_capsulechar4, 4)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET0_MOTION2 \
    DATANUM(DATA_capsulechar4, 5)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET0_MOTION3 \
    DATANUM(DATA_capsulechar4, 21)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET0_MOTION4 \
    DATANUM(DATA_capsulechar4, 7)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET0_MOTION5 \
    DATANUM(DATA_capsulechar4, 11)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET0_MOTION6 \
    DATANUM(DATA_capsulechar4, 12)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET0_MOTION7 \
    DATANUM(DATA_capsulechar4, 13)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET0_MOTION8 \
    DATANUM(DATA_capsulechar4, 19)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET1_MODEL \
    DATANUM(DATA_capsulechar4, 28)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET1_MOTION1 \
    DATANUM(DATA_capsulechar4, 31)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET1_MOTION2 \
    DATANUM(DATA_capsulechar4, 32)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET1_MOTION3 \
    DATANUM(DATA_capsulechar4, 47)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET1_MOTION4 \
    DATANUM(DATA_capsulechar4, 34)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET1_MOTION5 \
    DATANUM(DATA_capsulechar4, 37)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET1_MOTION6 \
    DATANUM(DATA_capsulechar4, 38)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET1_MOTION7 \
    DATANUM(DATA_capsulechar4, 39)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET1_MOTION8 \
    DATANUM(DATA_capsulechar4, 45)
#define CAPSPECIAL_DATA_MIRACLE_TRADE_BACK \
    DATANUM(DATA_capsulechar4, 58)
#define CAPSPECIAL_DATA_MIRACLE_TRADE0 DATANUM(DATA_capsulechar4, 59)
#define CAPSPECIAL_DATA_MIRACLE_TRADE1 DATANUM(DATA_capsulechar4, 60)
#define CAPSPECIAL_DATA_MIRACLE_TRADE2 DATANUM(DATA_capsulechar4, 61)
#define CAPSPECIAL_DATA_MIRACLE_TRADE3 DATANUM(DATA_capsulechar4, 62)
#define CAPSPECIAL_DATA_MIRACLE_TRADE4 DATANUM(DATA_capsulechar4, 63)
#define CAPSPECIAL_DATA_MIRACLE_TRADE5 DATANUM(DATA_capsulechar4, 64)

#define CAPSPECIAL_DATA_TERESA_MODEL DATANUM(DATA_capsuleshop, 8)
#define CAPSPECIAL_DATA_TERESA_MOTION_IDLE DATANUM(DATA_capsuleshop, 9)
#define CAPSPECIAL_DATA_TERESA_MOTION_ITEM DATANUM(DATA_capsuleshop, 10)
#define CAPSPECIAL_DATA_TERESA_MOTION_STEAL DATANUM(DATA_capsuleshop, 11)
#define CAPSPECIAL_DATA_TERESA_STOLEN_CAPSULE DATANUM(DATA_capsule, 73)

#define CAPSPECIAL_MESS_TERESA_UNAVAILABLE MESSNUM(MESS_TERESA_MASU, 0)
#define CAPSPECIAL_MESS_TERESA_INSUFFICIENT_COIN \
    MESSNUM(MESS_TERESA_MASU, 1)
#define CAPSPECIAL_MESS_TERESA_NO_TARGET MESSNUM(MESS_TERESA_MASU, 2)
#define CAPSPECIAL_MESS_TERESA_INTRO MESSNUM(MESS_TERESA_MASU, 3)
#define CAPSPECIAL_MESS_TERESA_CUSTOM_INTRO MESSNUM(MESS_TERESA_MASU, 4)
#define CAPSPECIAL_MESS_TERESA_STEAL_CHOICE MESSNUM(MESS_TERESA_MASU, 5)
#define CAPSPECIAL_MESS_TERESA_CUSTOM_CHOICE MESSNUM(MESS_TERESA_MASU, 6)
#define CAPSPECIAL_MESS_TERESA_TARGET_CHOICE MESSNUM(MESS_TERESA_MASU, 7)
#define CAPSPECIAL_MESS_TERESA_PAYMENT MESSNUM(MESS_TERESA_MASU, 8)
#define CAPSPECIAL_MESS_TERESA_COIN_TARGET MESSNUM(MESS_TERESA_MASU, 9)
#define CAPSPECIAL_MESS_TERESA_ITEM_TARGET MESSNUM(MESS_TERESA_MASU, 10)
#define CAPSPECIAL_MESS_TERESA_COIN_RESULT MESSNUM(MESS_TERESA_MASU, 11)
#define CAPSPECIAL_MESS_TERESA_STAR_RESULT MESSNUM(MESS_TERESA_MASU, 12)
#define CAPSPECIAL_MESS_TERESA_SUCCESS MESSNUM(MESS_TERESA_MASU, 13)
#define CAPSPECIAL_MESS_TERESA_CUSTOM_RESULT MESSNUM(MESS_TERESA_MASU, 14)
#define CAPSPECIAL_MESS_TERESA_CANCEL MESSNUM(MESS_TERESA_MASU, 15)
#define CAPSPECIAL_MESS_TERESA_FAILURE MESSNUM(MESS_TERESA_MASU, 16)
#define CAPSPECIAL_MESS_TERESA_MASH_HELP MESSNUM(MESS_TERESA_MASU, 17)

#define CAPSPECIAL_SE_TERESA_MESSAGE 925
#define CAPSPECIAL_SE_TERESA_FAILURE 926

typedef int (*TERESA_STEAL_HOOK)(int);
typedef void (*TERESA_STEAL_BEGIN_HOOK)(int, int);

#define CAP_WORK_MAX 64

typedef struct EvCapWork {
    int motId[CAP_WORK_MAX][GW_PLAYER_MAX];
    int objId[CAP_WORK_MAX];
    int sprId[CAP_WORK_MAX];
    void *mem[CAP_WORK_MAX];
    int masuId[CAP_WORK_MAX];
    HuVecF objPos[CAP_WORK_MAX];
    int playerMasuId[GW_PLAYER_MAX];
    HuVecF playerPos[GW_PLAYER_MAX];
    int bgId;
    OMOBJ *obj;
} EVCAPWORK;

typedef struct CapWorkFlag {
    u8 _flag00 : 1;
    u8 _flag01 : 1;
    u8 _flag02 : 1;
    u8 _flag03 : 1;
    u8 _flag04 : 1;
    u8 _flag05 : 1;
    u8 _flag06 : 1;
    u8 _flag07 : 1;
    u8 _flag08 : 1;
    u8 _flag09 : 1;
    u8 _flag10 : 1;
    u8 _flag11 : 1;
    u8 _flag12 : 1;
    u8 _flag13 : 1;
    u8 _flag14 : 1;
    u8 _flag15 : 1;
    u8 _flag16 : 1;
    u8 _flag17 : 1;
    u8 _flag18 : 1;
    u8 _flag19 : 1;
    u8 _flag20 : 1;
    u8 _flag21 : 1;
    u8 _flag22 : 1;
    u8 _flag23 : 1;
    u8 _flag24 : 1;
    u8 _flag25 : 1;
    u8 _flag26 : 1;
    u8 _flag27 : 1;
    u8 _flag28 : 1;
    u8 _flag29 : 1;
    u8 _flag30 : 1;
    u8 _flag31 : 1;
} CAPWORKFLAG;

typedef struct CapWork {
    int playerNo;
    int targetPlayerNo;
    int capsuleNo;
    int masuId;
    int masuIdNext;
    int _unk14;
    int _unk18;
    int _unk1C;
    EVCAPWORK objWork;
    CAPWORKFLAG flags;
    int _unkB6C;
    int _unkB70;
    int _unkB74;
    u8 _unkB78[84];
    int processNo;
    OMOBJ *explodeObj;
    OMOBJ *boostObj;
    OMOBJ *snowObj;
    OMOBJ *glowObj;
    OMOBJ *ringObj;
    OMOBJ *coinObj;
    OMOBJ *coinManObj;
    OMOBJ *starManObj;
    OMOBJ *capLoseObj;
} CAPWORK;

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

static HuVecF capsuleCameraOfs = { 0.0f, 100.0f, 0.0f };
static HuVecF teresaCameraRot = { -30.0f, 0.0f, 0.0f };
static HuVecF teresaCameraOfs = { 0.0f, 100.0f, 0.0f };
static HuVecF teresaLightPos = { 0.0f, 0.0f, 0.0f };
static HuVecF teresaLightDir = { 0.0f, 1.0f, -1.0f };
static u32 MiracleGuideMotTbl[2][16] = {
    { CAPSPECIAL_DATA_MIRACLE_GUIDE_SET0_MODEL,
        CAPSPECIAL_DATA_MIRACLE_GUIDE_SET0_MOTION1,
        CAPSPECIAL_DATA_MIRACLE_GUIDE_SET0_MOTION2,
        CAPSPECIAL_DATA_MIRACLE_GUIDE_SET0_MOTION3,
        CAPSPECIAL_DATA_MIRACLE_GUIDE_SET0_MOTION4,
        CAPSPECIAL_DATA_MIRACLE_GUIDE_SET0_MOTION5,
        CAPSPECIAL_DATA_MIRACLE_GUIDE_SET0_MOTION6,
        CAPSPECIAL_DATA_MIRACLE_GUIDE_SET0_MOTION7,
        CAPSPECIAL_DATA_MIRACLE_GUIDE_SET0_MOTION8, -1 },
    { CAPSPECIAL_DATA_MIRACLE_GUIDE_SET1_MODEL,
        CAPSPECIAL_DATA_MIRACLE_GUIDE_SET1_MOTION1,
        CAPSPECIAL_DATA_MIRACLE_GUIDE_SET1_MOTION2,
        CAPSPECIAL_DATA_MIRACLE_GUIDE_SET1_MOTION3,
        CAPSPECIAL_DATA_MIRACLE_GUIDE_SET1_MOTION4,
        CAPSPECIAL_DATA_MIRACLE_GUIDE_SET1_MOTION5,
        CAPSPECIAL_DATA_MIRACLE_GUIDE_SET1_MOTION6,
        CAPSPECIAL_DATA_MIRACLE_GUIDE_SET1_MOTION7,
        CAPSPECIAL_DATA_MIRACLE_GUIDE_SET1_MOTION8, -1 },
};
static char miracleItemHookName[11] = "itemhook_R";
static u32 miracleMasuEffColorTbl[6] = {
    0xFF7F7FFF, /* miracle square sparkle palette color */
    0xFFFF7FFF, /* miracle square sparkle palette color */
    0xFFBE7FFF, /* miracle square sparkle palette color */
    0x7F7FFFFF, /* miracle square sparkle palette color */
    0x7FFFFFFF, /* miracle square sparkle palette color */
    0x7FBEFFFF, /* miracle square sparkle palette color */
};
static int miracleTradeOrderTbl[3][32] = {
    { 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1,
        0, 1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 3, 2, 0, 1, 0, 2, 0, 1, 3, 0, 4, 2, 3, 0, 1, 0,
        1, 5, 2, 4, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 3, 0, 4, 2, 0, 5, 1, 2, 0, 3, 0, 1, 4, 1, 3, 1,
        2, 3, 4, 5, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
static int miracleLeftCharOrderTbl[32] = {
    1, 2, 1, 3, 1, 0, 1, 3, 0, 2, -1,
};
static int miracleRightCharOrderTbl[2][32] = {
    { 2, 0, 2, 1, 2, 1, 2, 0, 2, 0, -1,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 0, 2, 0, 2, 0, 1, 0, 2, 0, -1,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
static HuVecF miracleTradePosTbl[3] = {
    { 138.0f, 90.0f, 0.0f },
    { 288.0f, 90.0f, 0.0f },
    { 438.0f, 90.0f, 0.0f },
};
static int miracleTradeFileTbl[6] = {
    CAPSPECIAL_DATA_MIRACLE_TRADE0,
    CAPSPECIAL_DATA_MIRACLE_TRADE1,
    CAPSPECIAL_DATA_MIRACLE_TRADE2,
    CAPSPECIAL_DATA_MIRACLE_TRADE3,
    CAPSPECIAL_DATA_MIRACLE_TRADE4,
    CAPSPECIAL_DATA_MIRACLE_TRADE5,
};
static u32 kettouGuideMotTbl[2][16] = {
    { DATANUM(DATA_capsulechar4, 1), DATANUM(DATA_capsulechar4, 4),
        DATANUM(DATA_capsulechar4, 17), DATANUM(DATA_capsulechar4, 5),
        DATANUM(DATA_capsulechar4, 18), DATANUM(DATA_capsulechar4, 19),
        DATANUM(DATA_capsulechar4, 20), -1 },
    { DATANUM(DATA_capsulechar4, 28), DATANUM(DATA_capsulechar4, 31),
        DATANUM(DATA_capsulechar4, 43), DATANUM(DATA_capsulechar4, 32),
        DATANUM(DATA_capsulechar4, 44), DATANUM(DATA_capsulechar4, 45),
        DATANUM(DATA_capsulechar4, 46), -1 },
};
static u32 kettouPlayerMotTbl[4] = {
    0x008E0027, /* duel character motion resource pair */
    0x0093001F, /* duel character motion resource pair */
    0x008E0078, /* duel character motion resource pair */
    0x0093004B, /* duel character motion resource pair */
};
static u32 donkeyMotTbl[12] = {
    0x000E000F, /* Donkey model motion resource */
    0x000E0011, /* Donkey model motion resource */
    0x000E0012, /* Donkey model motion resource */
    0x000E0015, /* Donkey model motion resource */
    0x000E0016, /* Donkey model motion resource */
    0x000E0013, /* Donkey model motion resource */
    0x000E0014, /* Donkey model motion resource */
    0x000E0018, /* Donkey model motion resource */
    0x000E0019, /* Donkey model motion resource */
    0x000E001A, /* Donkey model motion resource */
    0x000E001B, /* Donkey model motion resource */
    0xFFFFFFFF, /* all-bits-set motion table terminator */
};
static int donkeyDiceResultTbl[5] = { 5, 10, 20, 30, -1 };
static int donkeyRouletteBankTbl[10] = { 0, 1, 0, 1, 0, 1, 0, 1, 0, 2 };
static char capTreeFook[12] = "tree_fook";
static u32 koopaMotTbl[7] = {
    0x000E0001, /* Koopa model motion resource */
    0x000E0005, /* Koopa model motion resource */
    0x000E0003, /* Koopa model motion resource */
    0x000E0007, /* Koopa model motion resource */
    0x000E0004, /* Koopa model motion resource */
    0x000E000A, /* Koopa model motion resource */
    0xFFFFFFFF, /* all-bits-set motion table terminator */
};
static int koopaDiceResultTbl[5] = { 5, 10, 20, 30, -1 };
static int koopaLoseMesTbl[3] = {
    MESSNUM(MESS_KOOPA_MASU, 10),
    MESSNUM(MESS_KOOPA_MASU, 11),
    MESSNUM(MESS_KOOPA_MASU, 12),
};
static int koopaLoseMesTbl2[3] = {
    MESSNUM(MESS_KOOPA_MASU, 10),
    MESSNUM(MESS_KOOPA_MASU, 11),
    MESSNUM(MESS_KOOPA_MASU, 12),
};

static int koopaMdlId = -1;
static int teresaStealMesId = -1;
static GXColor teresaLightColor = { 255, 190, 255, 255 };
static int miracleBackFile = CAPSPECIAL_DATA_MIRACLE_TRADE_BACK;
static u32 donkeyMgFile[2] = { 0x0005008C, 0x0005008C }; /* minigame archive resource identifier pair */
static u8 donkeyDiceTbl[8] = { 1, 2, 3, 4, 0xFF, 0, 0, 0 }; /* dice value table terminator sentinel */
static u32 koopaMgFile[2] = { 0x0005008D, 0x0005008D }; /* minigame archive resource identifier pair */
static u8 koopaDiceTbl[8] = { 1, 2, 3, 4, 0xFF, 0, 0, 0 }; /* dice value table terminator sentinel */
static int kettouMotId[12];
static int mgResultData[4];
static int diceHitTimer;
static OMOBJ *miracleSprObj;
static TERESA_STEAL_HOOK teresaStealHook;
static TERESA_STEAL_BEGIN_HOOK teresaStealBeginHook;
static int teresaStealCoinNum;
static TERESA_FADE_WORK *teresaFadeWork;

extern void mbDiceObjHit(int playerNo);
extern int mbDiceExec(int playerNo, int diceType, s8 *valueTbl,
    int tutorialVal, BOOL padWinF, BOOL waitF, HuVecF *pos, int color);
extern BOOL mbDiceKillCheck(int playerNo);
extern void mbDiceHitHookSet(int playerNo, void (*hook)(int result));
extern OMOBJ *mbev_CapEffGlowCreate(void);
extern int mbev_CapEffGlowAdd(OMOBJ *obj, HuVecF *pos, HuVecF *vel,
    int time, float scale, float gravity, float rotStep, GXColor *color);
extern int mbev_CapEffGlowKinokoTimeSet(OMOBJ *obj, int index,
    int unk08, int unk0A);
extern void mbev_CapEffColorSet(GXColor *color, int colorNo);
extern OMOBJ *mbev_CapEffCoinCreate(void);
extern OMOBJ *mbev_CapEffExplodeCreate(void);
extern void mbev_CapEffCoinGlowSet(OMOBJ *obj, OMOBJ *glowObj);
extern void mbWipeFadeOut(void);
extern void mbWipeFadeIn(void);
extern void mbev_MgCallDonkey(void);
extern void mbev_MgCallKoopa(void);
extern void mbev_MgCallKettou(void);
extern int mbMgRouletteNumGet(int type);
extern int mbCoinAddExec(int playerNo, int coinNum);
extern s16 mbCoinDispCapsuleCreate(HuVecF *pos, int coinNum);
extern float mbAngleWrap(float angle);
extern s16 mbev_CapSprCreate(EVCAPWORK *work, unsigned int dataNum,
    s16 prio, s16 bank);
extern void mbev_CapObjMotionSet(int modelId, int time, int motNo,
    int nextMotNo, u32 attr, u32 unk18, BOOL shiftF, BOOL nextAttr);
extern s16 mbev_CapCoinDisp(int playerNo, int coinNum, BOOL winMotF,
    BOOL waitF);
extern void mbDiceFadeSet(int playerNo);
extern void mbDicePadBtnHookSet(int playerNo, u16 (*hook)(int playerNo));
extern void mbDiceMotHookSet(int playerNo, void (*hook)(int playerNo));
extern int mbDiceResultGet(int playerNo);
extern int mbev_CapPlayerSquishSet(int *out, int masuId);
extern int mbev_CapPlayerSquishVoiceSet(int *out, int masuId, BOOL voiceF);
extern void mbev_CapPlayerStunSet(int *playerNo, int playerNum, BOOL type);
extern void mbev_CapEffDustHeavyAdd(OMOBJ *obj, HuVecF *pos);
extern void mbev_CapVibrate(int type);
extern OMOBJ *mbGuideCreateIn(void);
extern int mbGuideModelGet(OMOBJ *obj);
extern void mbGuideKill(OMOBJ *obj);
extern void mbev_CapObjClose(EVCAPWORK *work, int objId);
extern void mbev_CapPlayerRotate(int playerNo, float angle);
extern OMOBJ *mbev_CapCoinManCreate(void);
extern OMOBJ *mbev_CapStarManCreate(void);
extern void mbWipeSpecialFadeInCreate(int type, int time);
extern void mbWipeSpecialFadeOutCreate(int type, int time);
extern void mbev_CapDuelStatusDispSet(int leftPlayer, int rightPlayer,
    BOOL waitF);
extern int mbev_CapPlayerOrderGet(int *order, int playerNo1, int playerNo2,
    int type);
extern void mbCoinAddAllProcExecV(int *addNum, BOOL *dispF, BOOL fastF);
extern int mbStarDispPlayerCreate(int playerNo, int num);
extern BOOL mbStarDispCheck(int starObj);
extern int mbev_CapObjCreate(EVCAPWORK *work, int dataNum, int *motFile,
    BOOL linkF, int delay, BOOL closeDir);
extern void mbev_CapWait(CAPWORK *work);
extern void mbev_CapObjPosSet(EVCAPWORK *work, int objId, int masuId,
    HuVecF *pos);
extern void mbev_CapPlayerPosSet(EVCAPWORK *work, int playerNo, int masuId,
    HuVecF *pos);
extern void mbev_CapPlayerMotShiftSet(int modelId, int motionNo, u32 attr,
    BOOL shiftF);
extern float mbev_CapAngleLerp(float a, float b, float t);
extern BOOL mbev_CapPlayerCheck(int playerNo1, int playerNo2);
extern int mbev_CapPlayerComSelKettouGet(int playerNo, int type,
    int *playerList, int playerNum);
extern s16 mbev_CapPlayerMotionCreate(EVCAPWORK *work, int playerNo,
    int dataNum);
extern void mbev_CapPlayerMotShiftWait(int playerNo, int motionNo, u32 attr,
    BOOL waitF);
extern int mbev_CapEffCoinAdd(OMOBJ *obj, HuVecF *pos, HuVecF *vel,
    float scale, float gravity, int time, int arg);
extern BOOL mbev_CapEffCoinMaxYSet(OMOBJ *obj, int coinNo, float maxY);
extern int mbev_CapEffCoinNumGet(OMOBJ *obj);
extern void mbev_CapCoinAdd(OMOBJ *obj, int playerNo, int coinNum,
    BOOL highF);
extern int mbev_CapCoinManAdd(OMOBJ *obj, HuVecF *from, HuVecF *to,
    int targetPlayerNo, BOOL highF);
extern int mbev_CapCoinManNumGet(OMOBJ *obj);
extern int mbev_CapStarManAdd(OMOBJ *obj, HuVecF *from, HuVecF *to,
    int playerNo, BOOL highF);
extern int mbev_CapStarManNumGet(OMOBJ *obj);
extern void mbWipeDissolveFadeIn(void);
extern void mbWipeDissolveFadeOutTime(int time);
extern int mbStarObjCreate(void);
extern void mbStarObjPosSetV(int objNo, const HuVecF *pos);
extern void mbStarObjRotSet(int objNo, float x, float y, float z);
extern void mbStarObjScaleSet(int objNo, float x, float y, float z);
extern void mbStarObjDispSet(int objNo, BOOL dispF);
extern void mbStarObjDispSetAll(BOOL dispF);
extern void mbStarObjKill(int objNo);
extern void mbStarGetExec(int playerNo);

static void ev_CapTeresaFadeMatHook(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material);
static void ev_CapTeresaFadeOMExec(OMOBJ *obj);
static int ev_CapMiracleMesGet(int messNo);
static void ev_CapMiraclePlayerSet(void *unused, int playerNo1,
    int playerNo2, int masuId);
static void ev_CapMiracleCoinTrade(CAPWORK *work, int playerNo1,
    int playerNo2, int coinNum1, int coinNum2);
static void ev_CapMiracleStarTrade(CAPWORK *work, int playerNo1,
    int playerNo2, int starNum1, int starNum2);
static void ev_CapMiracleWindowFadeOut(s16 oldModel, s16 newModel,
    int timeMax, BOOL reverseF);
static void ev_CapMiracleWindowFadeIn(s16 oldModel, s16 newModel,
    int timeMax, BOOL reverseF, int motionStepFrames, int motionTimeCount,
    int *motionTimes);
static int ev_CapMiracleDiceExec(int playerNo, int modelId, int timeMax,
    int valueNum, int *motTimeTbl);
static void ev_CapMiracleSprCreate(void);
static void ev_CapMiracleSprDestroy(void);
static void ev_CapMiracleTradeCreate(HuVecF *pos, int no);
static void ev_CapMiracleTradeFocusSet(void);
static void ev_CapMiracleTradeHideSet(void);
static void ev_CapMiracleRun(CAPWORK *work);

static void ev_CapMiracleSprUpdate(OMOBJ *obj);
static void ev_CapMiracleMasu(CAPWORK *work);
static int ev_CapKettouStart(CAPWORK *work);
static int ev_CapKettouMesGet(int messNo);
static void ev_CapKettouReturn(CAPWORK *work);
static int ev_CapKoopaStart(CAPWORK *work);
static int ev_CapKoopaCoin(CAPWORK *work);
static void ev_CapKoopaReturn(CAPWORK *work);
static u16 ev_CapKoopaDicePadBtnHook(void);
static void ev_CapKoopaDiceMotHook(void);
static int ev_CapDonkeyStart(CAPWORK *work);
static void ev_CapDonkeyCoin(CAPWORK *work);
static void ev_CapDonkeyReturn(CAPWORK *work);
static void ev_CapDonkeyOMExec(OMOBJ *obj);
extern void mbev_CapEffDustCloudAdd(OMOBJ *obj, HuVecF *pos);
void mbev_CapTeresaFadeCreate(int objectId);
void mbev_CapTeresaFadeKill(int objectId);
void mbev_CapTeresaFadeSet(float alpha);

void mbev_CapTeresa(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;
    HuVecF playerPos;
    HuVecF objectPos;
    HuVecF direction;
    HuVecF cameraRot;
    HuVecF targetPos;
    HuVecF targetStartPos;
    HuVecF lightAim;
    HuVecF coinPos;
    HuVecF coinVel;
    Mtx hookMtx;
    GXColor lightColors[HU3D_GLIGHT_MAX];
    int motionFiles[] = {
        CAPSPECIAL_DATA_TERESA_MOTION_IDLE,
        CAPSPECIAL_DATA_TERESA_MOTION_STEAL,
        CAPSPECIAL_DATA_TERESA_MOTION_ITEM,
        -1,
    };
    int targetPlayers[GW_PLAYER_MAX - 1];
    int enabledPlayers[GW_PLAYER_MAX - 1];
    char customMes[16];
    int playerNo = work->playerNo;
    int linkMasu;
    int objectId;
    int targetPlayer = -1;
    int targetNum;
    int coinTargetNum;
    int starTargetNum;
    int stealType = -1;
    int choice;
    int enabledNum;
    int capsuleIndex;
    int itemObjectId;
    int idleMotion;
    int stealMotion;
    int starMotion;
    int starObjectId;
    int helpWin;
    int pressNum;
    int alpha;
    int coinNum;
    int coinEffect;
    int lightId;
    int turnCoinMax;
    int i;
    int j;
    float angle;
    float time;
    float weight;
    float stealRate;
    float randomValue;
    float launchAngle;
    float launchElevation;
    float launchScale;
    char *hookName;
    BOOL musicChanged = FALSE;

    mbev_CapWait(work);
    if (!GwSystem.curTime) {
        mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbWinCreate(2, CAPSPECIAL_MESS_TERESA_UNAVAILABLE, 10);
        mbWinTopWait();
        HuPrcEnd();
        return;
    }

    work->glowObj = mbev_CapEffGlowCreate();
    work->coinObj = mbev_CapEffCoinCreate();
    mbev_CapEffCoinGlowSet(work->coinObj, work->glowObj);
    mbPlayerPosGet(playerNo, &playerPos);
    linkMasu = mbMasuAttrFindLink(GwPlayer[playerNo].masuId,
        CAPSPECIAL_MASU_ATTR_TERESA_LINK);
    if (linkMasu != -1) {
        mbMasuPosGet(linkMasu, &objectPos);
        PSVECSubtract(&objectPos, &playerPos, &direction);
        direction.y = 0.0f;
        if (PSVECMag(&direction) > 0.0f) {
            PSVECNormalize(&direction, &direction);
        }
        PSVECScale(&direction, &direction, 300.0f);
        PSVECAdd(&playerPos, &direction, &objectPos);
        objectPos.y += 125.0f;
    } else {
        objectPos = playerPos;
        objectPos.y -= 100.0f;
        objectPos.z -= 200.0f;
    }
    PSVECSubtract(&objectPos, &playerPos, &direction);
    objectId = mbev_CapObjCreate(&work->objWork,
        CAPSPECIAL_DATA_TERESA_MODEL, motionFiles,
        FALSE, 5, FALSE);
    mbObjMotionSet(objectId, 1, HU3D_MOTATTR_LOOP);
    mbObjLayerSet(objectId, 4);
    mbObjScaleSet(objectId, 2.0f, 2.0f, 2.0f);
    mbObjPosSetV(objectId, &objectPos);
    angle = (float)(180.0 * (atan2(direction.x, direction.z) / M_PI));
    mbObjRotSet(objectId, 0.0f,
        (float)(180.0 + (180.0 * (atan2(direction.x, direction.z) / M_PI))),
        0.0f);
    mbev_CapTeresaFadeCreate(objectId);
    mbPlayerRotSet(playerNo, 0.0f, angle, 0.0f);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (i != playerNo) {
            mbPlayerDispSet(i, FALSE);
        }
    }
    cameraRot = teresaCameraRot;
    cameraRot.y = (float)(180.0
        + (180.0 * (atan2(direction.x, direction.z) / M_PI)));
    mbCameraMovePlayer(playerNo, &cameraRot, &teresaCameraOfs, 1500.0f,
        -1.0f, -1);
    mbCameraMoveWait();
    mbMusBoardFadeOut(0, 0, 1000, 1000, 32, FALSE);
    mbWipeDissolveFadeIn();

    if (!GwSystem.curTime) {
        mbAudFXPlay(CAPSPECIAL_SE_TERESA_MESSAGE);
        mbWinCreate(2, CAPSPECIAL_MESS_TERESA_UNAVAILABLE, 10);
        mbWinTopWait();
    } else if (mbPlayerCoinGet(playerNo) < 5) {
        mbAudFXPlay(CAPSPECIAL_SE_TERESA_MESSAGE);
        mbWinCreate(2, CAPSPECIAL_MESS_TERESA_INSUFFICIENT_COIN, 10);
        mbWinTopWait();
    } else {
        coinTargetNum = 0;
        starTargetNum = 0;
        if (mbPlayerCoinGet(playerNo) >= 5) {
            for (i = 0; i < GW_PLAYER_MAX; i++) {
                if (!mbev_CapPlayerCheck(i, playerNo)
                    && mbPlayerCoinGet(i) > 0) {
                    coinTargetNum++;
                }
            }
        }
        if (mbPlayerCoinGet(playerNo) >= 40) {
            for (i = 0; i < GW_PLAYER_MAX; i++) {
                if (!mbev_CapPlayerCheck(i, playerNo)
                    && mbPlayerStarGet(i) > 0) {
                    starTargetNum++;
                }
            }
        }
        targetNum = 0;
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (i != playerNo) {
                targetPlayers[targetNum++] = i;
            }
        }
        enabledNum = 0;
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (!mbev_CapPlayerCheck(i, playerNo)
                && mbPlayerStarGet(i) > 0) {
                enabledNum++;
            }
        }

        if (coinTargetNum <= 0 && starTargetNum <= 0
            && mbPlayerCoinGet(playerNo) < 40 && enabledNum >= 1) {
            mbAudFXPlay(CAPSPECIAL_SE_TERESA_MESSAGE);
            mbWinCreate(2, CAPSPECIAL_MESS_TERESA_INSUFFICIENT_COIN, 10);
            mbWinTopWait();
        } else if (coinTargetNum <= 0 && starTargetNum <= 0) {
            mbAudFXPlay(CAPSPECIAL_SE_TERESA_MESSAGE);
            mbWinCreate(2, CAPSPECIAL_MESS_TERESA_NO_TARGET, 10);
            mbWinTopWait();
        } else {
            mbAudFXPlay(CAPSPECIAL_SE_TERESA_MESSAGE);
            mbWinCreate(2, teresaStealMesId == -1
                ? CAPSPECIAL_MESS_TERESA_INTRO
                : CAPSPECIAL_MESS_TERESA_CUSTOM_INTRO,
                10);
            mbWinTopWait();

            for (;;) {
                if (teresaStealMesId == -1) {
                    mbWinCreateChoice(1, CAPSPECIAL_MESS_TERESA_STEAL_CHOICE,
                        10, 0);
                    if (coinTargetNum == 0) {
                        mbWinTopChoiceDisable(0);
                    }
                    if (starTargetNum == 0) {
                        mbWinTopChoiceDisable(1);
                    }
                    if (GwPlayer[playerNo].comF) {
                        mbComChoiceListDownSet(
                            coinTargetNum != 0 && starTargetNum != 0);
                    }
                    mbWinTopWait();
                    stealType = mbWinTopChoiceGet();
                    if (stealType == 2 || stealType == -1) {
                        mbWinCreate(2, CAPSPECIAL_MESS_TERESA_CANCEL, 10);
                        mbWinTopWait();
                        stealType = -1;
                        break;
                    }
                } else {
                    mbWinCreateChoice(1, CAPSPECIAL_MESS_TERESA_CUSTOM_CHOICE,
                        10, 0);
                    sprintf(customMes, "%d", teresaStealCoinNum);
                    mbWinTopInsertMesSet(teresaStealMesId, 0);
                    mbWinTopInsertMesSet((u32)customMes, 1);
                    if (coinTargetNum == 0) {
                        mbWinTopChoiceDisable(0);
                    }
                    if (starTargetNum == 0) {
                        mbWinTopChoiceDisable(1);
                    }
                    if (mbPlayerCoinGet(playerNo) < teresaStealCoinNum) {
                        mbWinTopChoiceDisable(2);
                    }
                    if (GwPlayer[playerNo].comF) {
                        mbComChoiceListDownSet(
                            coinTargetNum != 0 && starTargetNum != 0);
                    }
                    mbWinTopWait();
                    stealType = mbWinTopChoiceGet();
                    if (stealType == 3 || stealType == -1) {
                        mbWinCreate(2, CAPSPECIAL_MESS_TERESA_CANCEL, 10);
                        mbWinTopWait();
                        stealType = -1;
                        break;
                    }
                    if (stealType == 2) {
                        break;
                    }
                }

                mbWinCreateChoice(1, CAPSPECIAL_MESS_TERESA_TARGET_CHOICE, 10,
                    0);
                for (i = 0; i < targetNum; i++) {
                    mbWinTopInsertMesSet(
                        mbPlayerNameMesGet(targetPlayers[i]), i);
                    enabledPlayers[i] = targetPlayers[i];
                    if ((stealType == 0
                            && mbPlayerCoinGet(targetPlayers[i]) <= 0)
                        || (stealType == 1
                            && mbPlayerStarGet(targetPlayers[i]) <= 0)
                        || (GwSystem.tagF
                            && mbev_CapPlayerCheck(
                                playerNo, targetPlayers[i]))) {
                        mbWinTopChoiceDisable(i);
                        enabledPlayers[i] = -1;
                    }
                }
                if (GwPlayer[playerNo].comF) {
                    mbComChoiceListDownSet(mbev_CapPlayerComSelKettouGet(
                        playerNo, stealType, enabledPlayers, targetNum));
                }
                mbWinTopWait();
                choice = mbWinTopChoiceGet();
                if (choice == -1) {
                    continue;
                }
                if (choice < targetNum) {
                    targetPlayer = targetPlayers[choice];
                } else {
                    j = mbRandMod(targetNum);
                    targetPlayer = -1;
                    for (i = 0; i < targetNum; i++) {
                        if (enabledPlayers[j] >= 0) {
                            targetPlayer = enabledPlayers[j];
                            break;
                        }
                        if (++j >= targetNum) {
                            j = 0;
                        }
                    }
                }
                if (targetPlayer >= 0) {
                    break;
                }
            }

            if (stealType >= 0) {
                if (teresaStealMesId != -1 && stealType == 2) {
                    mbCoinAddExec(playerNo, -teresaStealCoinNum);
                } else if (stealType == 0) {
                    mbCoinAddExec(playerNo, -5);
                } else {
                    mbCoinAddExec(playerNo, -40);
                }
                mbAudFXPlay(CAPSPECIAL_SE_TERESA_MESSAGE);
                mbWinCreate(2, CAPSPECIAL_MESS_TERESA_PAYMENT, 10);
                mbWinTopWait();

                i = 1;
                for (;;) {
                    if ((float)i > 60.0f) {
                        break;
                    }
                    mbev_CapTeresaFadeSet(
                        255.0f * (1.0f - ((float)i / 60.0f)));
                    HuPrcVSleep();
                    i++;
                }
                mbev_CapTeresaFadeSet(0.0f);

                if (teresaStealMesId != -1 && stealType == 2) {
                    if (teresaStealHook != NULL) {
                        teresaStealHook(TRUE);
                    } else {
                        mbWipeDissolveFadeOutTime(1);
                    }
                    if (teresaStealBeginHook != NULL) {
                        teresaStealBeginHook(playerNo, objectId);
                    }
                } else {
                    mbWipeDissolveFadeOutTime(1);
                    capsuleIndex = -1;
                    for (i = 0; i < mbPlayerCapsuleMaxGet(); i++) {
                        if (mbPlayerCapsuleGet(targetPlayer, i)
                            == CAPSPECIAL_CAPSULE_LIGHT) {
                            capsuleIndex = i;
                        }
                    }
                    targetPos.x = targetPos.y = targetPos.z = 0.0f;
                    mbPlayerPosGet(targetPlayer, &targetStartPos);
                    mbev_PlayerColMasu(targetPlayer,
                        GwPlayer[targetPlayer].masuId, TRUE);
                    for (i = 0; i < GW_PLAYER_MAX; i++) {
                        mbPlayerDispSet(i, i == targetPlayer);
                    }
                    mbCameraPlayerViewSetFast(targetPlayer, 0);
                    mbCameraMoveWait();
                    mbMasuPosGet(GwPlayer[targetPlayer].masuId, &cameraRot);
                    cameraRot.y += 300.0f;
                    if (capsuleIndex != -1) {
                        cameraRot.y -= 150.0f;
                        cameraRot.z -= 200.0f;
                    }
                    mbObjPosSetV(objectId, &cameraRot);
                    mbObjRotSet(objectId, 0.0f, 0.0f, 0.0f);
                    mbObjDispSet(objectId, FALSE);

                    idleMotion = mbev_CapPlayerMotionCreate(&work->objWork,
                        targetPlayer, CHARMOT_HSF_c000m1_323);
                    stealMotion = mbev_CapPlayerMotionCreate(&work->objWork,
                        targetPlayer, CHARMOT_HSF_c000m1_344);
                    starMotion = mbev_CapPlayerMotionCreate(&work->objWork,
                        targetPlayer, CHARMOT_HSF_c000m1_457);
                    mbPlayerMotionShiftSet(targetPlayer, idleMotion, 0.0f,
                        8.0f, HU3D_MOTATTR_LOOP);
                    itemObjectId = mbev_CapObjCreate(&work->objWork,
                        CAPSPECIAL_DATA_TERESA_STOLEN_CAPSULE, NULL, FALSE, 0,
                        FALSE);
                    hookName = CharModelItemHookGet(
                        GwPlayer[targetPlayer].charNo, 4, 0);
                    mbObjHookSet(mbPlayerObjIDGet(targetPlayer), hookName,
                        itemObjectId);
                    mbObjDispSet(itemObjectId, FALSE);

                    for (i = 0; i < HU3D_GLIGHT_MAX; i++) {
                        lightColors[i] = Hu3DGlobalLight[i].color;
                        if (Hu3DGlobalLight[i].type != -1) {
                            Hu3DGlobalLight[i].color.r *= 0.5f;
                            Hu3DGlobalLight[i].color.g *= 0.5f;
                            Hu3DGlobalLight[i].color.b *= 0.5f;
                        }
                    }
                    lightId = -1;
                    mbStarObjDispSetAll(FALSE);
                    mbWipeDissolveFadeIn();

                    if (capsuleIndex != -1) {
                        mbWinCreate(2, CAPSPECIAL_MESS_TERESA_ITEM_TARGET, -1);
                        mbWinTopInsertMesSet(
                            mbPlayerNameMesGet(targetPlayer), 0);
                        mbWinTopPlayerDisable(targetPlayer);
                        mbWinTopWait();
                        mbObjLayerSet(itemObjectId, 5);
                        mbPlayerLayerSet(targetPlayer, 5);
                        mbObjDispSet(objectId, TRUE);
                        i = 1;
                        for (;;) {
                            if ((float)i > 60.0f) {
                                break;
                            }
                            mbev_CapTeresaFadeSet(
                                255.0f * ((float)i / 60.0f));
                            HuPrcVSleep();
                            i++;
                        }
                        mbev_CapTeresaFadeSet(255.0f);
                        mbPlayerRotateStart(targetPlayer, 180, 15);
                        while (!mbPlayerRotateCheck(targetPlayer)) {
                            HuPrcVSleep();
                        }
                        omVibrate(targetPlayer, 20, 7, 3);
                        mbPlayerMotionShiftSet(targetPlayer, starMotion,
                            0.0f, 8.0f, 0);
                        mbObjDispSet(itemObjectId, TRUE);
                        for (i = 1; (float)i < 18.0f; i++) {
                            time = (float)i / 18.0f;
                            mbObjScaleSet(itemObjectId, time, time, time);
                            HuPrcVSleep();
                        }
                        mbObjScaleSet(itemObjectId, 1.0f, 1.0f, 1.0f);
                        while (!mbPlayerMotionEndCheck(targetPlayer)) {
                            HuPrcVSleep();
                        }
                        lightId = Hu3DLLightCreateV(
                            mbObjModelIDGet(objectId), &teresaLightPos,
                            &teresaLightDir, &teresaLightColor);
                        Hu3DLLightSpotSet(mbObjModelIDGet(objectId), lightId,
                            GX_SP_SHARP, 0.001f);
                        Hu3DLLightStaticSet(
                            mbObjModelIDGet(objectId), lightId, TRUE);
                        Hu3DLLightInfinitytSet(
                            mbObjModelIDGet(objectId), lightId);
                        Hu3DMotionCalc(mbObjModelIDGet(
                            mbPlayerObjIDGet(playerNo)));
                        hookName = CharModelItemHookGet(
                            GwPlayer[playerNo].charNo, 4, 0);
                        Hu3DModelObjMtxGet(mbObjModelIDGet(
                                mbPlayerObjIDGet(playerNo)),
                            hookName, hookMtx);
                        objectPos.x = hookMtx[0][3];
                        objectPos.y = hookMtx[1][3];
                        objectPos.z = hookMtx[2][3];
                        lightAim = objectPos;
                        lightAim.y -= 100.0f;
                        lightAim.z += 200.0f;
                        Hu3DLLightPosAimSetV(mbObjModelIDGet(objectId),
                            lightId, &objectPos, &lightAim);
                        Hu3DLLightPosAngleSet(mbObjModelIDGet(objectId),
                            lightId, objectPos.x, objectPos.y, objectPos.z,
                            -45.0f, 0.0f);
                        mbObjMotionShiftSet(objectId, 3, 0.0f, 8.0f,
                            HU3D_MOTATTR_LOOP);
                        HuPrcSleep(180);
                        mbMusBoardFadeOut(0, 0, 1000, 1000, -1, FALSE);
                        musicChanged = TRUE;
                        stealType = -1;
                    } else if (stealType == 0) {
                        mbWinCreate(2, CAPSPECIAL_MESS_TERESA_COIN_TARGET, -1);
                        mbWinTopInsertMesSet(
                            mbPlayerNameMesGet(targetPlayer), 0);
                        mbWinTopPlayerDisable(targetPlayer);
                        mbWinTopWait();
                        mbObjDispSet(objectId, TRUE);
                        i = 1;
                        for (;;) {
                            if ((float)i > 60.0f) {
                                break;
                            }
                            mbev_CapTeresaFadeSet(
                                255.0f * ((float)i / 60.0f));
                            HuPrcVSleep();
                            i++;
                        }
                        mbev_CapTeresaFadeSet(255.0f);
                        mbPlayerMotionShiftSet(targetPlayer, stealMotion,
                            0.0f, 8.0f, HU3D_MOTATTR_LOOP);
                        mbObjMotionShiftSet(objectId, 2, 0.0f, 8.0f,
                            HU3D_MOTATTR_LOOP);
                        helpWin = mbWinCreateHelp(
                            CAPSPECIAL_MESS_TERESA_MASH_HELP);
                        mbPlayerColSnapPlayerSet(targetPlayer, FALSE);
                        for (i = 0; (float)i < 30.0f; i++) {
                            weight = (float)i / 30.0f;
                            mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                                &objectPos);
                            objectPos.y += 100.0f
                                * (3.0f - (2.0f * weight));
                            alpha = 255.0f - (127.0f * weight);
                            mbObjPosSetV(objectId, &objectPos);
                            mbev_CapTeresaFadeSet(alpha);
                            mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                                &targetPos);
                            targetPos.y += 50.0f * weight;
                            mbPlayerPosSetV(targetPlayer, &targetPos);
                            HuPrcVSleep();
                        }
                        pressNum = 0;
                        for (i = 0; (float)i < 120.0f; i++) {
                            weight = (float)i / 120.0f;
                            mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                                &objectPos);
                            objectPos.y += 100.0f
                                + (20.0f * (float)sin((M_PI
                                    * (1440.0f * weight)) / 180.0));
                            mbObjPosSetV(objectId, &objectPos);
                            mbev_CapTeresaFadeSet(alpha);
                            if (HuPadBtnDown[GwPlayer[targetPlayer].padNo]
                                & PAD_BUTTON_A) {
                                pressNum++;
                                if (alpha > 0) {
                                    alpha--;
                                }
                            }
                            alpha += (int)(2.5f * (float)sin((M_PI
                                * (360.0f * weight)) / 180.0));
                            if (alpha < 64 && (i & 7) == 0) {
                                alpha++;
                            }
                            if (alpha < 32) {
                                alpha = 32;
                            } else if (alpha > 255) {
                                alpha = 255;
                            }
                            mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                                &targetPos);
                            targetPos.y += 50.0f;
                            mbPlayerPosSetV(targetPlayer, &targetPos);
                            HuPrcVSleep();
                        }
                        mbWinKill(helpWin);
                        for (i = 0; (float)i < 30.0f; i++) {
                            weight = 1.0f - ((float)i / 30.0f);
                            mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                                &objectPos);
                            objectPos.y += 100.0f
                                + (100.0f * (2.0f - (2.0f * weight)));
                            mbObjPosSetV(objectId, &objectPos);
                            if (alpha < 255) {
                                alpha += 25;
                            }
                            if (alpha > 255) {
                                alpha = 255;
                            }
                            mbev_CapTeresaFadeSet(alpha);
                            mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                                &targetPos);
                            targetPos.y += 50.0f * weight;
                            mbPlayerPosSetV(targetPlayer, &targetPos);
                            HuPrcVSleep();
                        }
                        mbPlayerMotionShiftSet(targetPlayer, 6, 0.0f,
                            8.0f, HU3D_MOTATTR_LOOP);
                        mbPlayerColSnapPlayerSet(targetPlayer, TRUE);
                        mbObjMotionShiftSet(objectId, 1, 0.0f, 8.0f,
                            HU3D_MOTATTR_LOOP);
                        if (GwSystem.turnNo <= 10) {
                            turnCoinMax = 20;
                        } else if (GwSystem.turnNo <= 20) {
                            turnCoinMax = 25;
                        } else if (GwSystem.turnNo <= 30) {
                            turnCoinMax = 30;
                        } else if (GwSystem.turnNo <= 40) {
                            turnCoinMax = 35;
                        } else {
                            turnCoinMax = 40;
                        }
                        if (!GwPlayer[targetPlayer].comF) {
                            stealRate = 1.0f - (pressNum * 0.03125f);
                        } else {
                            randomValue = MBCapsuleEffRandF();
                            stealRate = 1.0f
                                - (0.1f + (GwPlayer[targetPlayer].comDif
                                    * (0.2f + (0.1f * randomValue))));
                        }
                        if (stealRate < 0.1f) {
                            stealRate = 0.1f;
                        } else if (stealRate > 1.0f) {
                            stealRate = 1.0f;
                        }
                        coinNum = stealRate * turnCoinMax;
                        if (coinNum > mbPlayerCoinGet(targetPlayer)) {
                            coinNum = mbPlayerCoinGet(targetPlayer);
                        }
                        omVibrate(targetPlayer, 20, 7, 3);
                        mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                            &objectPos);
                        for (i = 0; i < coinNum; i++) {
                            launchAngle = 360.0f * MBCapsuleEffRandF();
                            coinPos = objectPos;
                            coinPos.y += 100.0f;
                            launchElevation = 70.0f
                                + (15.0f * MBCapsuleEffRandF());
                            launchScale = 0.8f
                                + (0.3f * MBCapsuleEffRandF());
                            coinVel.x = (float)(65.0f * launchScale
                                * sin((M_PI * launchAngle) / 180.0)
                                * cos((M_PI * launchElevation) / 180.0));
                            coinVel.y = (float)(65.0f * launchScale
                                * sin((M_PI * launchElevation) / 180.0));
                            coinVel.z = (float)(65.0f * launchScale
                                * cos((M_PI * launchAngle) / 180.0)
                                * cos((M_PI * launchElevation) / 180.0));
                            coinEffect = mbev_CapEffCoinAdd(work->coinObj,
                                &coinPos, &coinVel, 0.75f, 4.9f, 30, 4);
                            if (coinEffect >= 0) {
                                mbev_CapEffCoinMaxYSet(work->coinObj,
                                    coinEffect, objectPos.y + 300.0f);
                            }
                            mbPlayerCoinAdd(targetPlayer, -1);
                            mbAudFXPlay(14);
                            HuPrcVSleep();
                        }
                        mbAudFXPlay(15);
                        while (mbev_CapEffCoinNumGet(work->coinObj) > 0) {
                            HuPrcVSleep();
                        }
                    } else {
                        mbWinCreate(2, CAPSPECIAL_MESS_TERESA_ITEM_TARGET, -1);
                        mbWinTopInsertMesSet(
                            mbPlayerNameMesGet(targetPlayer), 0);
                        mbWinTopPlayerDisable(targetPlayer);
                        mbWinTopWait();
                        mbObjDispSet(objectId, TRUE);
                        i = 1;
                        for (;;) {
                            if ((float)i > 60.0f) {
                                break;
                            }
                            mbev_CapTeresaFadeSet(
                                255.0f * ((float)i / 60.0f));
                            HuPrcVSleep();
                            i++;
                        }
                        mbev_CapTeresaFadeSet(255.0f);
                        starObjectId = mbStarObjCreate();
                        mbStarObjDispSet(starObjectId, FALSE);
                        mbPlayerMotionShiftSet(targetPlayer, stealMotion,
                            0.0f, 8.0f, HU3D_MOTATTR_LOOP);
                        mbObjMotionShiftSet(objectId, 2, 0.0f, 8.0f,
                            HU3D_MOTATTR_LOOP);
                        mbPlayerColSnapPlayerSet(targetPlayer, FALSE);
                        for (i = 0; (float)i < 30.0f; i++) {
                            weight = (float)i / 30.0f;
                            mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                                &objectPos);
                            objectPos.y += 100.0f
                                * (3.0f - (2.0f * weight));
                            alpha = 255.0f - (127.0f * weight);
                            mbObjPosSetV(objectId, &objectPos);
                            mbev_CapTeresaFadeSet(alpha);
                            mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                                &targetPos);
                            targetPos.y += 50.0f * weight;
                            mbPlayerPosSetV(targetPlayer, &targetPos);
                            HuPrcVSleep();
                        }
                        for (i = 0; (float)i < 30.0f; i++) {
                            weight = 1.0f - ((float)i / 30.0f);
                            mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                                &objectPos);
                            objectPos.y += 100.0f
                                + (100.0f * (2.0f - (2.0f * weight)));
                            mbObjPosSetV(objectId, &objectPos);
                            if (alpha < 255) {
                                alpha += 25;
                            }
                            if (alpha > 255) {
                                alpha = 255;
                            }
                            mbev_CapTeresaFadeSet(alpha);
                            mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                                &targetPos);
                            targetPos.y += 50.0f * weight;
                            mbPlayerPosSetV(targetPlayer, &targetPos);
                            HuPrcVSleep();
                        }
                        mbPlayerMotionShiftSet(targetPlayer, 6, 0.0f,
                            8.0f, HU3D_MOTATTR_LOOP);
                        mbPlayerColSnapPlayerSet(targetPlayer, TRUE);
                        mbPlayerStarAdd(targetPlayer, -1);
                        omVibrate(targetPlayer, 20, 20, 0);
                        for (i = 0; (float)i <= 60.0f; i++) {
                            weight = (float)sin((M_PI
                                * (90.0f * ((float)i / 60.0f))) / 180.0);
                            mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                                &targetPos);
                            targetPos.y += 300.0f * weight;
                            mbStarObjPosSetV(starObjectId, &targetPos);
                            mbStarObjRotSet(starObjectId, 0.0f,
                                360.0f * weight, 0.0f);
                            mbStarObjScaleSet(
                                starObjectId, weight, weight, weight);
                            mbStarObjDispSet(starObjectId, TRUE);
                            mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                                &objectPos);
                            objectPos.y += 300.0f;
                            objectPos.z -= 200.0f * weight;
                            mbObjPosSetV(objectId, &objectPos);
                            HuPrcVSleep();
                        }
                        for (i = 0; (float)i <= 60.0f; i++) {
                            weight = (float)sin((M_PI
                                * (90.0f * ((float)i / 60.0f))) / 180.0);
                            mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                                &targetPos);
                            targetPos.y += 300.0f + (500.0f * weight);
                            mbStarObjPosSetV(starObjectId, &targetPos);
                            mbStarObjRotSet(starObjectId, 0.0f,
                                720.0f * weight, 0.0f);
                            HuPrcVSleep();
                        }
                        mbStarObjKill(starObjectId);
                    }

                    mbWipeDissolveFadeOutTime(1);
                    mbStarObjDispSetAll(TRUE);
                    for (i = 0; i < HU3D_GLIGHT_MAX; i++) {
                        if (Hu3DGlobalLight[i].type != -1) {
                            Hu3DGlobalLight[i].color = lightColors[i];
                        }
                    }
                    if (lightId != -1) {
                        Hu3DLLightKill(mbObjModelIDGet(objectId), lightId);
                    }
                    if (capsuleIndex != -1) {
                        mbPlayerCapsuleRemove(targetPlayer, capsuleIndex);
                        GwPlayer[targetPlayer].capsuleUseNum++;
                    }
                    mbPlayerLayerSet(targetPlayer, 3);
                    hookName = CharModelItemHookGet(
                        GwPlayer[targetPlayer].charNo, 4, 0);
                    mbObjHookObjReset(
                        mbPlayerObjIDGet(targetPlayer), hookName);
                    mbObjDispSet(itemObjectId, FALSE);
                    mbPlayerPosSetV(targetPlayer, &targetStartPos);
                    mbPlayerRotSet(targetPlayer, 0.0f, 0.0f, 0.0f);
                    mbPlayerMotionSet(targetPlayer, 1, HU3D_MOTATTR_LOOP);
                }

                mbObjMotionShiftSet(objectId, 1, 0.0f, 8.0f,
                    HU3D_MOTATTR_LOOP);
                PSVECSubtract(&objectPos, &playerPos, &direction);
                mbObjPosSetV(objectId, &objectPos);
                mbObjRotSet(objectId, 0.0f,
                    (float)(180.0 + (180.0
                        * (atan2(direction.x, direction.z) / M_PI))),
                    0.0f);
                mbev_CapTeresaFadeSet(0.0f);
                mbev_PlayerColMasu(
                    playerNo, GwPlayer[playerNo].masuId, TRUE);
                mbPlayerRotSet(playerNo, 0.0f,
                    (float)(180.0
                        * (atan2(direction.x, direction.z) / M_PI)),
                    0.0f);
                for (i = 0; i < GW_PLAYER_MAX; i++) {
                    mbPlayerDispSet(i, i == playerNo);
                }
                cameraRot = teresaCameraRot;
                cameraRot.y = (float)(180.0 + (180.0
                    * (atan2(direction.x, direction.z) / M_PI)));
                mbCameraMovePlayer(playerNo, &cameraRot, &teresaCameraOfs,
                    1500.0f, -1.0f, -1);
                mbCameraMoveWait();
                if (teresaStealMesId != -1 && stealType == 2) {
                    if (teresaStealHook != NULL) {
                        teresaStealHook(FALSE);
                    } else {
                        mbWipeDissolveFadeIn();
                    }
                } else {
                    mbWipeDissolveFadeIn();
                }
                i = 1;
                for (;;) {
                    if ((float)i > 60.0f) {
                        break;
                    }
                    mbev_CapTeresaFadeSet(
                        255.0f * ((float)i / 60.0f));
                    HuPrcVSleep();
                    i++;
                }
                mbev_CapTeresaFadeSet(255.0f);

                switch (stealType) {
                    case 0:
                        mbAudFXPlay(CAPSPECIAL_SE_TERESA_MESSAGE);
                        mbWinCreate(2, CAPSPECIAL_MESS_TERESA_COIN_RESULT, 10);
                        mbWinTopWait();
                        mbPlayerRotateStart(playerNo,
                            (s16)(180.0 + (180.0
                                * (atan2(direction.x, direction.z) / M_PI))),
                            15);
                        while (!mbPlayerRotateCheck(playerNo)) {
                            HuPrcVSleep();
                        }
                        mbev_CapCoinAdd(
                            work->coinObj, playerNo, coinNum, TRUE);
                        mbAudFXPlay(CAPSPECIAL_SE_TERESA_MESSAGE);
                        mbWinCreate(2, CAPSPECIAL_MESS_TERESA_SUCCESS, 10);
                        mbWinTopWait();
                        break;
                    case 1:
                        mbAudFXPlay(CAPSPECIAL_SE_TERESA_MESSAGE);
                        mbWinCreate(2, CAPSPECIAL_MESS_TERESA_STAR_RESULT, 10);
                        mbWinTopWait();
                        mbMusFadeOutSpeed(1, 1000);
                        while (mbMusCheck(1)) {
                            HuPrcVSleep();
                        }
                        mbPlayerRotateStart(playerNo,
                            (s16)(180.0 + (180.0
                                * (atan2(direction.x, direction.z) / M_PI))),
                            15);
                        while (!mbPlayerRotateCheck(playerNo)) {
                            HuPrcVSleep();
                        }
                        mbStarGetExec(playerNo);
                        mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f,
                            HU3D_MOTATTR_LOOP);
                        mbMusBoardPlay();
                        musicChanged = TRUE;
                        mbAudFXPlay(CAPSPECIAL_SE_TERESA_MESSAGE);
                        mbWinCreate(2, CAPSPECIAL_MESS_TERESA_SUCCESS, 10);
                        mbWinTopWait();
                        break;
                    case 2:
                        mbAudFXPlay(CAPSPECIAL_SE_TERESA_MESSAGE);
                        mbWinCreate(2, CAPSPECIAL_MESS_TERESA_CUSTOM_RESULT, 10);
                        mbWinTopWait();
                        break;
                    default:
                        mbAudFXPlay(CAPSPECIAL_SE_TERESA_FAILURE);
                        mbWinCreate(2, CAPSPECIAL_MESS_TERESA_FAILURE, 10);
                        mbWinTopWait();
                        mbPlayerRotateStart(playerNo,
                            (s16)(180.0 + (180.0
                                * (atan2(direction.x, direction.z) / M_PI))),
                            15);
                        while (!mbPlayerRotateCheck(playerNo)) {
                            HuPrcVSleep();
                        }
                        mbev_CapPlayerMotShiftWait(playerNo, 13, 0, TRUE);
                        break;
                }
            }
        }
    }

    if (!musicChanged) {
        mbMusBoardFadeOut(0, 0, 1000, 1000, -1, FALSE);
    }
    mbWipeDissolveFadeOutTime(1);
    mbObjDispSet(objectId, FALSE);
    mbev_CapTeresaFadeKill(objectId);
    mbPlayerRotSet(playerNo, 0.0f, 0.0f, 0.0f);
    mbPlayerMotionSet(playerNo, 1, HU3D_MOTATTR_LOOP);
    if (!mbMasuDispCheck(GwPlayer[playerNo].masuId)
        || GwPlayer[playerNo].moveNum > 1) {
        mbCameraPlayerViewSetFast(playerNo, 2);
    } else {
        mbCameraPlayerViewSetFast(playerNo, 0);
    }
    mbCameraMoveWait();
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerDispSet(i, TRUE);
    }
    HuDataDirClose(DATA_capsuleshop);
    HuPrcEnd();
}

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

void mbev_CapTeresaFadeCreate(int objectId)
{
    extern const float lbl_802C42C4;
    int modelId;
    HU3D_MODEL *model;
    HSF_DATA *hsf;
    HSF_MATERIAL *material;
    int i;
    int mallocNo;
    u32 textureSize;
    int textureMallocNo;
    TERESA_FADE_WORK *workData;
    TERESA_FADE_WORK *work;
    void *textureData;
    void *texture;

    modelId = mbObjModelIDGet(objectId);
    model = &Hu3DData[modelId];
    hsf = model->hsf;
    material = hsf->material;
    Hu3DModelMatHookSet(modelId, ev_CapTeresaFadeMatHook);
    for (i = 0; i < hsf->materialNum; i++, material++) {
        material->flags |= HSF_MATERIAL_MATHOOK;
    }

    mallocNo = model->mallocNo;
    workData = HuMemDirectMallocNum(
        HEAP_MODEL, sizeof(TERESA_FADE_WORK), mallocNo);
    work = workData;
    teresaFadeWork = work;
    memset(teresaFadeWork, 0, sizeof(TERESA_FADE_WORK));
    teresaFadeWork->activeF = TRUE;
    teresaFadeWork->alpha = lbl_802C42C4;
    teresaFadeWork->copyF = FALSE;
    teresaFadeWork->screenWidth = 640;
    teresaFadeWork->screenHeight = 480;
    teresaFadeWork->textureWidth = 320;
    teresaFadeWork->textureHeight = 240;
    teresaFadeWork->object = omAddObjEx(
        mbObjMan, CAPSPECIAL_FADE_OBJ_PRIORITY, 0, 0, OM_GRP_NONE,
        ev_CapTeresaFadeOMExec);
    teresaFadeWork->textureSize = GXGetTexBufferSize(
        teresaFadeWork->textureWidth, teresaFadeWork->textureHeight,
        GX_TF_RGB565, GX_FALSE, 0);
    textureMallocNo = model->mallocNo;
    textureSize = teresaFadeWork->textureSize;
    textureData = HuMemDirectMallocNum(
        HEAP_MODEL, textureSize, textureMallocNo);
    texture = textureData;
    teresaFadeWork->textureData = texture;
    memset(teresaFadeWork->textureData, 0, teresaFadeWork->textureSize);
    DCFlushRange(teresaFadeWork->textureData, teresaFadeWork->textureSize);
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

static void ev_CapTeresaFadeMatHook(HU3D_DRAW_OBJ *drawObj,
    HSF_MATERIAL *material)
{
    extern const float lbl_802C4288;
    extern const float lbl_802C42D0;
    extern const float lbl_802C42E0;
    extern const float lbl_802C4368;
    extern const float lbl_802C436C;
    TERESA_FADE_WORK *work = teresaFadeWork;
    HU3D_CAMERA *camera;
    GXTexObj texture;
    GXColor color;
    Mtx perspective;
    Mtx cameraInv;
    Mtx objectMtx;
    Mtx texMtx;
    float fov;

    if (work == NULL) {
        return;
    }
    if (!work->copyF) {
        GXDrawDone();
        GXSetTexCopySrc(0, 0, (u16)work->screenWidth,
            (u16)work->screenHeight);
        GXSetTexCopyDst((u16)work->textureWidth, (u16)work->textureHeight,
            GX_TF_RGB565, work->activeF);
        GXCopyTex(work->textureData, GX_FALSE);
        GXPixModeSync();
        work->copyF = TRUE;
    }
    if (material->attrNum != 1) {
        return;
    }

    GXSetNumTexGens(2);
    GXSetNumTevStages(2);
    GXSetTevKAlphaSel(0, 0);
    GXSetTexCoordGen2(0, 1, 4, 0x3C, GX_FALSE, 0x7D); /* texture-coordinate generation selector for the fade pass */
    GXSetTevOrder(0, 0, 0, 0);
    GXSetTevColorIn(0, 0xF, 8, 0xA, 0xF); /* TEV color-input selectors for sampled fade composition */
    GXSetTevColorOp(0, 0, 0, 0, GX_TRUE, 0);
    GXSetTevAlphaIn(0, 7, 7, 7, 6);
    GXSetTevAlphaOp(0, 0, 0, 0, GX_TRUE, 0);

    Hu3DMatLightSet(drawObj->model, 0, material->hiliteScale);
    camera = &Hu3DCamera[Hu3DCameraNo];
    fov = camera->fov;
    if (fov <= lbl_802C4288) {
        fov = lbl_802C42E0;
    }
    C_MTXLightPerspective(perspective, fov, lbl_802C4368,
        lbl_802C42D0, lbl_802C436C, lbl_802C42D0, lbl_802C42D0);
    PSMTXInverse(Hu3DCameraMtx, cameraInv);
    PSMTXConcat(cameraInv, drawObj->matrix, objectMtx);
    PSMTXConcat(perspective, Hu3DCameraMtx, texMtx);
    PSMTXConcat(texMtx, objectMtx, texMtx);
    GXLoadTexMtxImm(texMtx, 0x21, 0); /* projected texture matrix slot for the fade pass */
    GXSetTexCoordGen2(1, 0, 0, 0x21, GX_FALSE, 0x7D); /* texture-coordinate generation selector for the fade pass */

    color.r = 255;
    color.g = 255;
    color.b = 255;
    color.a = (u8)work->alpha;
    GXSetTevColor(3, color);
    GXSetTevOrder(1, 1, 1, 4);
    GXSetTevKAlphaSel(1, 0);
    GXSetTevColorIn(1, 8, 0, 7, 0xF); /* TEV color-input selectors for sampled fade composition */
    GXSetTevColorOp(1, 0, 0, 0, GX_TRUE, 0);
    GXSetTevAlphaIn(1, 7, 7, 7, 6);
    GXSetTevAlphaOp(1, 0, 0, 0, GX_TRUE, 0);

    GXInitTexObj(&texture, work->textureData,
        (u16)work->textureWidth, (u16)work->textureHeight,
        GX_TF_RGB565, 0, 0, GX_FALSE);
    GXInitTexObjLOD(&texture, 1, 1, lbl_802C4288, lbl_802C4288,
        lbl_802C4288,
        GX_FALSE, GX_FALSE, 0);
    GXLoadTexObj(&texture, 1);
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
    extern const float lbl_802C4288;
    extern const float lbl_802C42C4;

    if (teresaFadeWork) {
        if (alpha < lbl_802C4288) {
            alpha = lbl_802C4288;
        }
        if (alpha > lbl_802C42C4) {
            alpha = lbl_802C42C4;
        }
        teresaFadeWork->alpha = alpha;
    }
}

void mbev_CapMiracle(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;
    HuVecF savedPos[GW_PLAYER_MAX];
    HuVecF masuPos;
    HuVecF cameraRot;
    HuVecF cameraOfs = { 0.0f, 150.0f, 0.0f };
    HuVecF zero = { 0.0f, 0.0f, 0.0f };
    int playerNo = work->playerNo;
    int currentMasu = GwPlayer[playerNo].masuId;
    int nextMasu;
    int guide0;
    int guide1;
    int guide;
    int i;

    mbev_CapWait(work);
    work->glowObj = mbev_CapEffGlowCreate();
    mbPlayerPosGet(playerNo, &savedPos[playerNo]);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerPosGet(i, &savedPos[i]);
    }
    mbCameraPlayerViewSet(playerNo, 0);
    mbev_CapPlayerRotate(playerNo, 0.0f);
    guide0 = mbev_CapObjCreate(&work->objWork, 0x00110000, /* event model resource identifier */
        (int *)MiracleGuideMotTbl[0], FALSE, 5, FALSE);
    mbObjMotionSet(guide0, 3, 0);
    mbObjDispSet(guide0, FALSE);
    guide1 = mbev_CapObjCreate(&work->objWork, 0x0011001B, /* event model resource identifier */
        (int *)MiracleGuideMotTbl[1], FALSE, 5, FALSE);
    guide = mbev_CapObjCreate(&work->objWork, 0x00110035, NULL, /* event model resource identifier */
        FALSE, 5, FALSE);
    mbObjHookSet(guide1, miracleItemHookName, guide);
    HuPrcVSleep();
    mbObjMotionSet(guide1, 3, 0);
    mbObjDispSet(guide1, FALSE);
    work->_unkB6C = guide0;
    work->_unkB70 = guide1;
    ev_CapMiracleMasu(work);
    mbWipeDissolveFadeOutTime(1);
    nextMasu = 1;
    while (nextMasu < mbMasuNumGet()
        && !(mbMasuAttrGet(nextMasu) & 0x10000)) { /* board-space link attribute mask */
        nextMasu++;
    }
    if (nextMasu >= mbMasuNumGet()) {
        nextMasu = 1;
    }
    if (GwSystem.curTime) {
        mbev_CapObjClose(&work->objWork, guide0);
        guide = guide1;
    } else {
        mbev_CapObjClose(&work->objWork, guide1);
        guide = guide0;
    }
    HuPrcSleep(3);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (i != playerNo) {
            mbPlayerDispSet(i, FALSE);
        }
    }
    mbMasuPosGet(nextMasu, &masuPos);
    mbPlayerPosSetV(playerNo, &masuPos);
    mbPlayerMotionSet(playerNo, 1, HU3D_MOTATTR_LOOP);
    mbev_CapPlayerPosSet(&work->objWork, playerNo, nextMasu, &zero);
    masuPos.x -= 200.0f;
    masuPos.y += 200.0f;
    mbObjPosSetV(guide, &masuPos);
    mbObjRotSet(guide, 0.0f, 0.0f, 0.0f);
    mbObjMotionSet(guide, 1, HU3D_MOTATTR_LOOP);
    mbCameraRotGet(&cameraRot);
    mbCameraMoveMasu(nextMasu, &cameraRot, &cameraOfs, 1500.0f, -1.0f,
        -1);
    mbCameraMoveWait();
    work->coinManObj = mbev_CapCoinManCreate();
    work->starManObj = mbev_CapStarManCreate();
    mbStatusDispForceSetAll(FALSE);
    work->_unkB6C = guide;
    work->_unkB70 = nextMasu;
    ev_CapMiracleRun(work);
    mbWipeSpecialFadeInCreate(3, 1);
    mbObjDispSet(guide0, FALSE);
    mbObjDispSet(guide1, FALSE);
    mbMasuPosGet(currentMasu, &masuPos);
    mbPlayerPosSetV(playerNo, &masuPos);
    mbPlayerMotionSet(playerNo, 1, HU3D_MOTATTR_LOOP);
    mbev_CapPlayerPosSet(&work->objWork, playerNo, currentMasu, &zero);
    mbPlayerColSnapPlayerSet(playerNo, TRUE);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerPosSetV(i, &savedPos[i]);
        mbPlayerMotionSet(i, 1, HU3D_MOTATTR_LOOP);
        mbPlayerRotSet(i, 0.0f, 0.0f, 0.0f);
        mbPlayerColSnapPlayerSet(i, TRUE);
        mbPlayerDispSet(i, TRUE);
    }
    mbMasuPosGet(currentMasu, &masuPos);
    mbCameraMoveMasu(currentMasu, &cameraRot, &capsuleCameraOfs,
        -1.0f, -1.0f, -1);
    mbCameraMoveWait();
    mbWipeSpecialFadeOutCreate(3, 60);
    HuPrcEnd();
}

void mbev_CapMiracleKill(void)
{
}

static void ev_CapMiracleMasu(CAPWORK *work)
{
    HuVecF masuPos;
    HuVecF pos;
    HuVecF vel = { 0.0f, 0.0f, 0.0f };
    GXColor color;
    int playerNo = work->playerNo;
    int masuId = GwPlayer[playerNo].masuId;
    int guide0 = work->_unkB6C;
    int guide1 = work->_unkB70;
    int playerMot0;
    int playerMot1;
    int i;
    int j;
    int glowNo;
    float t;
    float angle;

    playerMot0 = mbev_CapPlayerMotionCreate(&work->objWork, playerNo,
        0x00930017); /* event resource identifier */
    playerMot1 = mbev_CapPlayerMotionCreate(&work->objWork, playerNo,
        0x0093004E); /* event resource identifier */
    mbCameraMoveWait();
    omVibrate(playerNo, 0x12C, 4, 4); /* event vibration duration */
    mbCameraMoveMasu(masuId, NULL, &capsuleCameraOfs,
        -1.0f, -1.0f, 60);
    mbPlayerMotionShiftSet(playerNo, playerMot0, 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);
    mbAudFXPlay(0x443); /* event sound-effect resource */
    mbMasuPosGet(masuId, &masuPos);
    for (i = 0; i < 60; i++) {
        for (j = 0; j < 2; j++) {
            pos = masuPos;
            pos.x += (MBCapsuleEffRandF() - 0.5f) * 300.0f;
            pos.y += (MBCapsuleEffRandF() - 0.5f) * 100.0f;
            pos.z += (MBCapsuleEffRandF() - 0.5f) * 300.0f;
            mbev_CapEffColorSet(&color, mbRandMod(6));
            glowNo = mbev_CapEffGlowAdd(work->glowObj, &pos, &vel,
                30, 1.0f, 4.9f, 30.0f, &color);
            if (glowNo >= 0) {
                mbev_CapEffGlowKinokoTimeSet(work->glowObj, glowNo,
                    1, 0x5A); /* glow effect lifetime in frames */
            }
        }
        HuPrcVSleep();
    }
    mbAudFXPlay(0x444); /* event sound-effect resource */
    mbObjMotionShiftSet(guide0, 9, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
    mbPlayerMotionShiftSet(playerNo, playerMot1, 0.0f, 18.0f,
        HU3D_MOTATTR_LOOP);
    for (i = 0; i < 120; i++) {
        t = (float)i / 120.0f;
        pos = masuPos;
        pos.y += 100.0f * (float)sin((M_PI * 180.0 * t) / 180.0);
        mbPlayerPosSetV(playerNo, &pos);
        angle = (float)(M_PI * 360.0) * t / 180.0f;
        mbPlayerRotSet(playerNo, 0.0f,
            180.0f + 30.0f * (float)sin(angle), 0.0f);
        for (j = 0; j < 2; j++) {
            pos = masuPos;
            pos.x += (MBCapsuleEffRandF() - 0.5f) * 300.0f;
            pos.y += (MBCapsuleEffRandF() - 0.5f) * 100.0f;
            pos.z += (MBCapsuleEffRandF() - 0.5f) * 300.0f;
            mbev_CapEffColorSet(&color, mbRandMod(6));
            glowNo = mbev_CapEffGlowAdd(work->glowObj, &pos, &vel,
                30, 1.0f, 4.9f, 30.0f, &color);
            if (glowNo >= 0) {
                mbev_CapEffGlowKinokoTimeSet(work->glowObj, glowNo,
                    1, 0x5A); /* glow effect lifetime in frames */
            }
        }
        HuPrcVSleep();
    }
    mbAudFXPlay(0x445); /* event sound-effect resource */
    mbAudFXPlay(0x3FE); /* event sound-effect resource */
    for (i = 0; i < 120; i++) {
        t = (float)i / 120.0f;
        pos = masuPos;
        pos.y += 100.0f * (float)sin((M_PI * 180.0 * t) / 180.0);
        mbPlayerPosSetV(playerNo, &pos);
        mbPlayerRotSet(playerNo, 0.0f,
            180.0f * (float)sin((M_PI * 180.0 * t) / 180.0), 0.0f);
        for (j = 0; j < 2; j++) {
            pos = masuPos;
            pos.x += (MBCapsuleEffRandF() - 0.5f) * 300.0f;
            pos.y += (MBCapsuleEffRandF() - 0.5f) * 100.0f;
            pos.z += (MBCapsuleEffRandF() - 0.5f) * 300.0f;
            mbev_CapEffColorSet(&color, mbRandMod(6));
            glowNo = mbev_CapEffGlowAdd(work->glowObj, &pos, &vel,
                30, 1.0f, 4.9f, 30.0f, &color);
            if (glowNo >= 0) {
                mbev_CapEffGlowKinokoTimeSet(work->glowObj, glowNo,
                    1, 0x5A); /* glow effect lifetime in frames */
            }
        }
        HuPrcVSleep();
    }
    mbObjMotionShiftSet(guide0, 9, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
    mbObjMotionShiftSet(guide1, 9, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
    mbPlayerMotionShiftSet(playerNo, playerMot1, 0.0f, 18.0f,
        HU3D_MOTATTR_LOOP);
}

static void ev_CapMiracleRun(CAPWORK *work)
{
    HuVecF masuPos;
    HuVecF pos;
    HuVecF zero = { 0.0f, 0.0f, 0.0f };
    int playerNo = work->playerNo;
    int guide = work->_unkB6C;
    int nextMasu = work->_unkB70;
    int modelId;
    int tradeObj;
    int leftObj;
    int rightObj;
    int targetObj;
    int order[GW_PLAYER_MAX];
    int motTimes[32];
    int leftOrder[32];
    int rightOrder[32];
    int tradeOrder[32];
    int leftCount;
    int rightCount;
    int leftChoice;
    int rightChoice;
    int targetPlayer;
    int tradeNo;
    int tradeRow;
    int tradeCount;
    int i;
    char message[16];

    mbMasuPosGet(nextMasu, &masuPos);
    pos.x = masuPos.x + 200.0f;
    pos.y = masuPos.y;
    pos.z = masuPos.z;
    mbPlayerPosSetV(playerNo, &pos);
    pos.x = masuPos.x - 200.0f;
    pos.z = masuPos.z - 30.0f;
    mbObjPosSetV(guide, &pos);
    modelId = mbObjModelIDGet(guide);
    Hu3DMotionForceSet(modelId, "head", 0x80, 0.5f); /* animation keyframe time selector */
    Hu3DMotionForceSet(modelId, "head", 0x100, 0.5f); /* animation keyframe time selector */
    Hu3DMotionCalc(modelId);
    tradeObj = mbev_CapObjCreate(&work->objWork, 0x00110036, NULL, /* event model resource identifier */
        FALSE, 5, FALSE);
    leftObj = mbev_CapObjCreate(&work->objWork, 0x00110038, NULL, /* event model resource identifier */
        FALSE, 5, FALSE);
    rightObj = mbev_CapObjCreate(&work->objWork, 0x00110039, NULL, /* event model resource identifier */
        FALSE, 5, FALSE);
    targetObj = mbev_CapObjCreate(&work->objWork, 0x00110037, NULL, /* event model resource identifier */
        FALSE, 5, FALSE);
    for (i = 0; i < 4; i++) {
        mbObjPosSetV((i == 0) ? tradeObj
            : (i == 1) ? leftObj : (i == 2) ? rightObj : targetObj,
            &masuPos);
        mbObjRotSet((i == 0) ? tradeObj
            : (i == 1) ? leftObj : (i == 2) ? rightObj : targetObj,
            0.0f, 0.0f, 0.0f);
        mbObjScaleSet((i == 0) ? tradeObj
            : (i == 1) ? leftObj : (i == 2) ? rightObj : targetObj,
            1.0f, 1.0f, 1.0f);
    }
    mbObjDispSet(targetObj, TRUE);
    mbObjLayerSet(leftObj, 3);
    mbObjLayerSet(rightObj, 3);
    mbObjLayerSet(targetObj, 3);
    mbObjDispSet(leftObj, FALSE);
    mbObjDispSet(rightObj, FALSE);
    ev_CapMiracleSprCreate();
    mbMusBoardFadeOut(0, 0, 1000, 1000, 0x1F, FALSE); /* board music fade channel mask */
    mbWipeDissolveFadeIn();
    mbObjMotionShiftSet(guide, 7, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbAudFXPlay(0x3B6); /* event sound-effect resource */
    mbWinCreate(2, ev_CapMiracleMesGet(0x003C0000), 13); /* miracle scene message resource */
    mbWinTopWait();
    mbAudFXPlay(0x3B8); /* event sound-effect resource */
    mbWinCreate(2, ev_CapMiracleMesGet(0x003C0001), 13); /* miracle scene message resource */
    mbWinTopWait();
    (void)mbev_CapPlayerOrderGet(order, -1, playerNo, TRUE);
    leftCount = 0;
    while (leftCount < 32
        && miracleLeftCharOrderTbl[leftCount] >= 0) {
        int charNo = miracleLeftCharOrderTbl[leftCount];
        int candidate = order[charNo];
        leftOrder[leftCount] = candidate;
        motTimes[leftCount] = GwPlayer[candidate].charNo;
        leftCount++;
    }
    if (leftCount <= 0) {
        leftOrder[0] = playerNo;
        motTimes[0] = GwPlayer[playerNo].charNo;
        leftCount = 1;
    }
    ev_CapMiracleWindowFadeIn((s16)guide, (s16)leftObj, 60, TRUE,
        3, leftCount, motTimes);
    leftChoice = ev_CapMiracleDiceExec(playerNo, leftObj, 3,
        leftCount, motTimes);
    if (leftChoice < 0 || leftChoice >= leftCount) {
        leftChoice = 0;
    }
    targetPlayer = leftOrder[leftChoice];
    (void)mbev_CapPlayerOrderGet(order, targetPlayer, playerNo, TRUE);
    rightCount = 0;
    i = targetPlayer == playerNo ? 0 : 1;
    while (rightCount < 32
        && miracleRightCharOrderTbl[i][rightCount] >= 0) {
        int charNo = miracleRightCharOrderTbl[i][rightCount];
        int candidate = order[charNo];
        rightOrder[rightCount] = candidate;
        motTimes[rightCount] = GwPlayer[candidate].charNo;
        rightCount++;
    }
    if (rightCount <= 0) {
        rightOrder[0] = playerNo;
        motTimes[0] = GwPlayer[playerNo].charNo;
        rightCount = 1;
    }
    ev_CapMiracleWindowFadeIn((s16)leftObj, (s16)rightObj, 60, TRUE,
        3, rightCount, motTimes);
    rightChoice = ev_CapMiracleDiceExec(playerNo, rightObj, 3,
        rightCount, motTimes);
    if (rightChoice < 0 || rightChoice >= rightCount) {
        rightChoice = 0;
    }
    targetPlayer = rightOrder[rightChoice];
    tradeRow = (GwSystem.turnNo * 3) / GwSystem.turnMax;
    if (tradeRow < 0) {
        tradeRow = 0;
    } else if (tradeRow > 2) {
        tradeRow = 2;
    }
    tradeCount = 0;
    while (tradeCount < 32 && miracleTradeOrderTbl[tradeRow][tradeCount] >= 0) {
        tradeOrder[tradeCount] = miracleTradeOrderTbl[tradeRow][tradeCount];
        tradeCount++;
    }
    if (tradeCount <= 0) {
        tradeOrder[0] = 0;
        tradeCount = 1;
    }
    ev_CapMiracleWindowFadeIn((s16)rightObj, (s16)targetObj, 60, TRUE,
        3, tradeCount, tradeOrder);
    rightChoice = ev_CapMiracleDiceExec(playerNo, targetObj, 3,
        tradeCount, tradeOrder);
    if (rightChoice < 0 || rightChoice >= tradeCount) {
        rightChoice = 0;
    }
    tradeNo = tradeOrder[rightChoice];
    if (tradeNo < 0 || tradeNo > 5) {
        tradeNo = 0;
    }
    mbObjMotionTimeSet(rightObj, 0.5f + (float)tradeNo);
    pos = miracleTradePosTbl[1];
    ev_CapMiracleTradeCreate(&pos, tradeNo);
    ev_CapMiracleTradeFocusSet();
    mbAudFXPlay(0x3B8); /* event sound-effect resource */
    mbWinCreate(2, ev_CapMiracleMesGet(0x003C0004), 13); /* miracle scene message resource */
    mbWinTopWait();
    ev_CapMiracleTradeHideSet();
    HuPrcSleep(60);
    mbWipeSpecialFadeInCreate(3, 1);
    mbObjDispSet(guide, FALSE);
    mbObjDispSet(tradeObj, FALSE);
    mbObjDispSet(leftObj, FALSE);
    mbObjDispSet(rightObj, FALSE);
    mbObjDispSet(targetObj, FALSE);
    mbev_CapPlayerPosSet(&work->objWork, playerNo, -1, &zero);
    ev_CapMiraclePlayerSet(NULL, playerNo, targetPlayer, nextMasu);
    mbWipeSpecialFadeOutCreate(3, 60);
    if (tradeNo == 0) {
        mbWinCreate(2, ev_CapMiracleMesGet(0x003C0006), 13); /* miracle scene message resource */
        mbWinTopInsertMesSet(mbPlayerNameMesGet(playerNo), 0);
        mbWinTopInsertMesSet(mbPlayerNameMesGet(targetPlayer), 1);
        mbWinTopWait();
        ev_CapMiracleCoinTrade(work, playerNo, targetPlayer, 20, 0);
    } else if (tradeNo == 1) {
        mbWinCreate(2, ev_CapMiracleMesGet(0x003C0007), 13); /* miracle scene message resource */
        mbWinTopWait();
        ev_CapMiracleCoinTrade(work, playerNo, targetPlayer,
            mbPlayerCoinGet(playerNo), mbPlayerCoinGet(targetPlayer));
    } else if (tradeNo == 2 || tradeNo == 3) {
        mbWinCreate(2, ev_CapMiracleMesGet(0x003C0006), 13); /* miracle scene message resource */
        mbWinTopWait();
        ev_CapMiracleStarTrade(work, playerNo, targetPlayer,
            tradeNo == 2 ? 1 : 2, 0);
    } else if (tradeNo == 4) {
        ev_CapMiracleStarTrade(work, playerNo, targetPlayer,
            mbPlayerStarGet(playerNo), mbPlayerStarGet(targetPlayer));
    } else {
        ev_CapMiracleCoinTrade(work, playerNo, targetPlayer,
            mbPlayerCoinGet(playerNo), mbPlayerCoinGet(targetPlayer));
        ev_CapMiracleStarTrade(work, playerNo, targetPlayer,
            mbPlayerStarGet(playerNo), mbPlayerStarGet(targetPlayer));
    }
    mbev_CapDuelStatusDispSet(playerNo, targetPlayer, TRUE);
    sprintf(message, "%d", tradeNo);
    mbWinCreate(2, ev_CapMiracleMesGet(0x003C000A), 13); /* miracle scene message resource */
    mbWinTopInsertMesSet((u32)message, 0);
    mbWinTopWait();
    mbMusBoardFadeOut(0, 0, 1000, 1000, -1, FALSE);
    ev_CapMiracleSprDestroy();
}
static void ev_CapMiraclePlayerSet(void *unused, int playerNo1, int playerNo2,
    int masuId)
{
    extern const float lbl_802C4288;
    extern const float lbl_802C429C;
    extern const float lbl_802C42E0;
    extern const float lbl_802C43C0;
    HuVecF masuPos;
    HuVecF pos;
    int i;

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerDispSet(i, FALSE);
    }
    mbMasuPosGet(masuId, &masuPos);

    pos.x = masuPos.x - lbl_802C429C;
    pos.y = masuPos.y;
    pos.z = masuPos.z;
    mbPlayerPosSetV(playerNo1, &pos);
    mbPlayerRotSet(playerNo1, lbl_802C4288, lbl_802C42E0, lbl_802C4288);
    mbPlayerDispSet(playerNo1, TRUE);
    mbPlayerColSnapPlayerSet(playerNo1, FALSE);

    pos.x = lbl_802C429C + masuPos.x;
    pos.y = masuPos.y;
    pos.z = masuPos.z;
    mbPlayerPosSetV(playerNo2, &pos);
    mbPlayerRotSet(playerNo2, lbl_802C4288, lbl_802C43C0, lbl_802C4288);
    mbPlayerDispSet(playerNo2, TRUE);
    mbPlayerColSnapPlayerSet(playerNo2, FALSE);
}

static void ev_CapMiracleCoinTrade(CAPWORK *work, int playerNo1,
    int playerNo2, int coinNum1, int coinNum2)
{
    extern const float lbl_802C42CC;
    HuVecF playerPos1;
    HuVecF playerPos2;
    int coinDelay;
    int coinAddNum;

    if (coinNum1 + coinNum2 < 20) {
        coinDelay = 5;
    } else if (coinNum1 + coinNum2 < 40) {
        coinDelay = 4;
    } else if (coinNum1 + coinNum2 < 60) {
        coinDelay = 3;
    } else if (coinNum1 + coinNum2 < 80) {
        coinDelay = 2;
    } else {
        coinDelay = 1;
    }
    mbPlayerPosGet(playerNo1, &playerPos1);
    playerPos1.y += lbl_802C42CC;
    mbPlayerPosGet(playerNo2, &playerPos2);
    playerPos2.y += lbl_802C42CC;
    do {
        if (coinNum1 > 0 && mbPlayerCoinGet(playerNo1) > 0) {
            coinAddNum = mbev_CapCoinManAdd(work->coinManObj,
                &playerPos1, &playerPos2, playerNo2, TRUE);
            if (coinAddNum != 0) {
                mbPlayerCoinAdd(playerNo1, -coinAddNum);
                coinNum1 -= coinAddNum;
            }
        }
        if (mbPlayerCoinGet(playerNo1) <= 0) {
            coinNum1 = 0;
        }
        if (coinNum2 > 0 && mbPlayerCoinGet(playerNo2) > 0) {
            coinAddNum = mbev_CapCoinManAdd(work->coinManObj,
                &playerPos2, &playerPos1, playerNo1, TRUE);
            if (coinAddNum != 0) {
                mbPlayerCoinAdd(playerNo2, -coinAddNum);
                coinNum2 -= coinAddNum;
            }
        }
        if (mbPlayerCoinGet(playerNo2) <= 0) {
            coinNum2 = 0;
        }
        HuPrcSleep(coinDelay);
    } while (coinNum1 > 0 || coinNum2 > 0
        || mbev_CapCoinManNumGet(work->coinManObj) > 0);
    mbAudFXPlay(MSM_SE_CMN_16);
}

static void ev_CapMiracleStarTrade(CAPWORK *work, int playerNo1, int playerNo2,
    int starNum1, int starNum2)
{
    extern const float lbl_802C42CC;
    HuVecF playerPos1;
    HuVecF playerPos2;
    int starDelay;
    int starAddNum;

    if (starNum1 + starNum2 < 5) {
        starDelay = 15;
    } else if (starNum1 + starNum2 < 10) {
        starDelay = 12;
    } else if (starNum1 + starNum2 < 15) {
        starDelay = 10;
    } else if (starNum1 + starNum2 < 20) {
        starDelay = 8;
    } else {
        starDelay = 4;
    }
    mbPlayerPosGet(playerNo1, &playerPos1);
    playerPos1.y += lbl_802C42CC;
    mbPlayerPosGet(playerNo2, &playerPos2);
    playerPos2.y += lbl_802C42CC;
    do {
        if (starNum1 > 0 && mbPlayerStarGet(playerNo1) > 0) {
            starAddNum = mbev_CapStarManAdd(work->starManObj,
                &playerPos1, &playerPos2, playerNo2, TRUE);
            if (starAddNum != 0) {
                mbPlayerStarAdd(playerNo1, -starAddNum);
                starNum1 -= starAddNum;
            }
        }
        if (mbPlayerStarGet(playerNo1) <= 0) {
            starNum1 = 0;
        }
        if (starNum2 > 0 && mbPlayerStarGet(playerNo2) > 0) {
            starAddNum = mbev_CapStarManAdd(work->starManObj,
                &playerPos2, &playerPos1, playerNo1, TRUE);
            if (starAddNum != 0) {
                mbPlayerStarAdd(playerNo2, -starAddNum);
                starNum2 -= starAddNum;
            }
        }
        if (mbPlayerStarGet(playerNo2) <= 0) {
            starNum2 = 0;
        }
        HuPrcSleep(starDelay);
    } while (starNum1 > 0 || starNum2 > 0
        || mbev_CapStarManNumGet(work->starManObj) > 0);
}

static int ev_CapMiracleMesGet(int messNo)
{
    if (GwSystem.curTime == FALSE) {
        return messNo;
    }
    return messNo + 16;
}

static void ev_CapMiracleDiceHitHook(int result)
{
    (void)result;
    diceHitTimer = 0;
}

static void ev_CapMiracleWindowFadeOut(s16 oldModel, s16 newModel,
    int timeMax, BOOL reverseF)
{
    extern const float lbl_802C42C4;
    extern const float lbl_802C42C8;
    int time;

    if (reverseF) {
        mbObjDispSet(oldModel, TRUE);
        mbObjLayerSet(oldModel, 3);
        mbObjDispSet(newModel, TRUE);
        mbObjAlphaSet(newModel, 255);
        mbObjLayerSet(newModel, 3);
        for (time = 0; time < timeMax; time++) {
            mbObjAlphaSet(newModel, (int)(lbl_802C42C4
                * ((float)time / (float)timeMax)));
            HuPrcVSleep();
        }
        mbObjDispSet(oldModel, FALSE);
        mbObjDispSet(newModel, TRUE);
        mbObjAlphaSet(newModel, 255);
        mbObjLayerSet(newModel, 3);
    } else {
        mbObjDispSet(oldModel, TRUE);
        mbObjAlphaSet(oldModel, 255);
        mbObjLayerSet(oldModel, 3);
        mbObjDispSet(newModel, TRUE);
        mbObjLayerSet(newModel, 3);
        for (time = 0; time < timeMax; time++) {
            mbObjAlphaSet(oldModel, (int)(lbl_802C42C4
                * (lbl_802C42C8
                    - ((float)time / (float)timeMax))));
            HuPrcVSleep();
        }
        mbObjDispSet(oldModel, FALSE);
        mbObjDispSet(newModel, TRUE);
        mbObjLayerSet(newModel, 3);
    }
}

static void ev_CapMiracleWindowFadeIn(s16 oldModel, s16 newModel,
    int timeMax, BOOL reverseF, int motionStepFrames, int motionTimeCount,
    int *motionTimes)
{
    extern const float lbl_802C42C4;
    extern const float lbl_802C42C8;
    extern const float lbl_802C42D0;
    int time;
    int motionTime;
    int motionNo;

    motionTime = 0;
    motionNo = 0;
    if (reverseF) {
        mbObjDispSet(oldModel, TRUE);
        mbObjLayerSet(oldModel, 3);
        mbObjDispSet(newModel, TRUE);
        mbObjAlphaSet(newModel, 255);
        mbObjLayerSet(newModel, 3);
        for (time = 0; time < timeMax; time++) {
            mbObjAlphaSet(newModel, (int)(lbl_802C42C4
                * ((float)time / (float)timeMax)));
            if (++motionTime >= motionStepFrames) {
                motionTime = 0;
                if (++motionNo >= motionTimeCount) {
                    motionNo = 0;
                }
                mbObjMotionTimeSet(newModel, lbl_802C42D0
                    + (float)motionTimes[motionNo]);
            }
            HuPrcVSleep();
        }
        mbObjDispSet(oldModel, FALSE);
        mbObjDispSet(newModel, TRUE);
        mbObjAlphaSet(newModel, 255);
        mbObjLayerSet(newModel, 3);
    } else {
        mbObjDispSet(oldModel, TRUE);
        mbObjAlphaSet(oldModel, 255);
        mbObjLayerSet(oldModel, 3);
        mbObjDispSet(newModel, TRUE);
        mbObjLayerSet(newModel, 3);
        for (time = 0; time < timeMax; time++) {
            mbObjAlphaSet(oldModel, (int)(lbl_802C42C4
                * (lbl_802C42C8
                    - ((float)time / (float)timeMax))));
            if (++motionTime >= motionStepFrames) {
                motionTime = 0;
                if (++motionNo >= motionTimeCount) {
                    motionNo = 0;
                }
                mbObjMotionTimeSet(newModel, lbl_802C42D0
                    + (float)motionTimes[motionNo]);
            }
            HuPrcVSleep();
        }
        mbObjDispSet(oldModel, FALSE);
        mbObjDispSet(newModel, TRUE);
        mbObjAlphaSet(newModel, 255);
        mbObjLayerSet(newModel, 3);
    }
}

static int ev_CapMiracleDiceExec(int playerNo, int modelId, int timeMax,
    int valueNum, int *motTimeTbl)
{
    extern const float lbl_802C42D0;
    int winNo;
    int value;
    int time;

    winNo = mbWinCreateHelp(MESSNUM(MESS_BOARD_OPE, 2));
    mbDiceExec(playerNo, 6, NULL, -1, FALSE, FALSE, NULL, 0);
    diceHitTimer = TRUE;
    mbDiceHitHookSet(playerNo, ev_CapMiracleDiceHitHook);
    value = mbRandMod(valueNum);
    time = 0;
    do {
        if (diceHitTimer) {
            time++;
            if (time >= timeMax) {
                time = 0;
                value++;
                if (value >= valueNum) {
                    value = 0;
                }
            }
            mbObjMotionTimeSet(modelId,
                lbl_802C42D0 + (float)motTimeTbl[value]);
        }
        HuPrcVSleep();
    } while (!mbDiceKillCheck(playerNo));
    if (winNo != -1) {
        mbWinKill(winNo);
    }
    return value;
}

static void ev_CapMiracleSprCreate(void)
{
    extern const float lbl_802C4288;
    MIRACLE_SPR_WORK *work;
    int i;
    int j;
    OMOBJ *obj;

    miracleSprObj = obj = omAddObjEx(
        mbObjMan, -32768, 0, 0, -1, ev_CapMiracleSprUpdate);
    work = obj->data = HuMemDirectMallocNum(
        HEAP_HEAP, 6 * sizeof(MIRACLE_SPR_WORK), HU_MEMNUM_OVL);
    memset(work, 0, 6 * sizeof(MIRACLE_SPR_WORK));
    for (i = 0; i < 6; i++, work++) {
        work->activeF = FALSE;
        work->sprId = -1;
        work->backSprId = -1;
        for (j = 0; j < 6; j++) {
            work->sprIdTbl[j] = -1;
        }
        work->focusTime = 0;
        work->focusNo = 0;
        work->hideF = FALSE;
        work->unk30 = lbl_802C4288;
        work->unk34 = lbl_802C4288;
    }
}

static void ev_CapMiracleSprUpdate(OMOBJ *obj)
{
    MIRACLE_SPR_WORK *work = obj->data;
    int i;
    int j;
    float t;
    float angle;
    float scale;
    float alpha;

    if (mbExitCheck() || miracleSprObj == NULL) {
        for (i = 0; i < 6; i++, work++) {
            if (work->sprId != -1) {
                espKill((s16)work->sprId);
            }
            if (work->backSprId != -1) {
                espKill((s16)work->backSprId);
            }
            for (j = 0; j < 6; j++) {
                if (work->sprIdTbl[j] != -1) {
                    espKill((s16)work->sprIdTbl[j]);
                }
            }
            work->sprId = -1;
            work->backSprId = -1;
        }
        miracleSprObj = NULL;
        omDelObjEx(mbObjMan, obj);
        return;
    }
    for (i = 0; i < 6; i++, work++) {
        if (!work->activeF) {
            continue;
        }
        if (work->focusTime == 0) {
            for (j = 0; j < 6; j++) {
                angle = (float)(j * 60 + work->focusNo * 4)
                    * (float)(M_PI / 180.0);
                espPosSet((s16)work->sprIdTbl[j],
                    work->pos.x + 52.0f * (float)cos(angle),
                    work->pos.y + 52.0f * (float)sin(angle));
                espScaleSet((s16)work->sprIdTbl[j], 0.7f, 0.7f);
                espTPLvlSet((s16)work->sprIdTbl[j], 1.0f);
            }
            work->focusNo++;
            continue;
        }
        if (work->focusTime == 1 || work->focusTime == 32) {
            int maxTime = work->focusTime == 1 ? 18 : 240;
            work->focusNo++;
            t = (float)work->focusNo / (float)maxTime;
            if (t > 1.0f) {
                t = 1.0f;
            }
            scale = (float)sin((M_PI * 90.0 * t) / 180.0);
            alpha = 1.0f - scale;
            espScaleSet((s16)work->sprId, scale, scale);
            espScaleSet((s16)work->backSprId, scale, scale);
            espTPLvlSet((s16)work->sprId, alpha);
            espTPLvlSet((s16)work->backSprId, alpha);
            if (work->focusTime == 32 && work->hideF) {
                espScaleSet((s16)work->sprId, 0.0f, 0.0f);
                espScaleSet((s16)work->backSprId, 0.0f, 0.0f);
            }
            if (work->focusNo >= maxTime) {
                work->focusTime = work->focusTime == 1 ? 2 : 64;
                work->focusNo = 0;
            }
            continue;
        }
        if (work->focusTime == 2) {
            work->focusNo++;
            t = (float)work->focusNo / 30.0f;
            scale = 1.0f + 0.2f
                * (float)sin((M_PI * 720.0 * t) / 180.0);
            espScaleSet((s16)work->sprId, scale, scale);
            espScaleSet((s16)work->backSprId, scale, scale);
            if (work->focusNo >= 30) {
                work->focusTime = 0;
                work->focusNo = 0;
            }
            continue;
        }
        if (work->focusTime == 64) {
            work->focusNo++;
            t = (float)work->focusNo / 30.0f;
            angle = (float)(M_PI * 0.5) * t;
            espPosSet((s16)work->sprId,
                work->pos.x + 280.0f * (float)sin(angle), work->pos.y);
            espPosSet((s16)work->backSprId,
                work->pos.x + 280.0f * (float)sin(angle), work->pos.y);
            if (work->focusNo >= 30) {
                espDispOff((s16)work->sprId);
                espDispOff((s16)work->backSprId);
                espDispOff((s16)work->sprIdTbl[0]);
                work->activeF = FALSE;
                work->focusTime = 0;
                work->focusNo = 0;
            }
        }
    }
}

static void ev_CapMiracleSprDestroy(void)
{
    miracleSprObj = NULL;
}

static void ev_CapMiracleTradeCreate(HuVecF *pos, int no)
{
    extern const float lbl_802C4288;
    MIRACLE_SPR_WORK *work;
    int i;
    int j;
    OMOBJ *obj;
    int file;

    obj = miracleSprObj;
    if (miracleSprObj == NULL) {
        return;
    }
    for (work = obj->data, i = 0; i < 6; i++, work++) {
        if (!work->activeF) {
            break;
        }
    }
    if (i >= 6) {
        return;
    }
    work->activeF = TRUE;
    work->focusTime = 0;
    work->focusNo = 0;
    work->unk30 = lbl_802C4288;
    work->unk34 = lbl_802C4288;
    work->pos = *pos;
    if (no < 0) {
        no = 0;
    } else if (no > 5) {
        no = 5;
    }
    file = miracleTradeFileTbl[no];
    work->sprId = (s16)espEntry(file, 100, 0);
    espPosSet((s16)work->sprId, pos->x, pos->y);
    espScaleSet((s16)work->sprId, lbl_802C4288, lbl_802C4288);
    espAttrSet((s16)work->sprId, HUSPR_ATTR_LINEAR);
    espDrawNoSet((s16)work->sprId, 32);
    work->backSprId = (s16)espEntry(miracleBackFile, 120, 0);
    espPosSet((s16)work->backSprId, pos->x, pos->y);
    espScaleSet((s16)work->backSprId, lbl_802C4288, lbl_802C4288);
    espAttrSet((s16)work->backSprId, HUSPR_ATTR_LINEAR);
    espDrawNoSet((s16)work->backSprId, 32);
    for (j = 0; j < 6; j++) {
        work->sprIdTbl[j] = (s16)espEntry(file, 100, 0);
        espPosSet((s16)work->sprIdTbl[j], pos->x, pos->y);
        espScaleSet((s16)work->sprIdTbl[j], lbl_802C4288,
            lbl_802C4288);
        espAttrSet((s16)work->sprIdTbl[j], HUSPR_ATTR_LINEAR);
        espDrawNoSet((s16)work->sprIdTbl[j], 32);
    }
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

void mbev_CapKettou(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;
    OMOBJ *guideObj;
    int guideModel;
    int guideSet = GwSystem.curTime ? 1 : 0;
    int i;
    int winner;
    int loser;

    mbev_CapWait(work);
    guideObj = mbGuideCreateIn();
    guideModel = mbGuideModelGet(guideObj);
    mbObjDispSet(guideModel, FALSE);
    for (i = 0; i < 12 && kettouGuideMotTbl[guideSet][i] != (u32)-1;
        i++) {
        kettouMotId[i] = mbObjMotionCreate(guideModel,
            kettouGuideMotTbl[guideSet][i]);
    }
    mbObjMotionSet(guideModel, kettouMotId[1], HU3D_MOTATTR_LOOP);
    mbPlayerColSnapPlayerSet(work->playerNo, TRUE);
    work->_unkB6C = guideModel;
    *(OMOBJ **)((u8 *)work + 0xBAC) = guideObj; /* retained CAPWORK field offset */
    if (!work->flags._flag02 && ev_CapKettouStart(work)) {
        winner = *(s16 *)((u8 *)mgResultData + 0);
        loser = *(s16 *)((u8 *)mgResultData + 2);
        if (!_CheckFlag(FLAG_MG_PRACTICE)) {
            GwPlayer[winner].masuId = 0;
            GwPlayer[loser].masuId = 0;
        }
        if ((!mbPlayerAllComCheck() || GwSystem.mgComDispF)
            && mbMgRouletteNumGet(6) > 0) {
            mbWinCreate(2, ev_CapKettouMesGet(0x003D000C), 13); /* duel scene message resource */
            mbWinTopWait();
            mbAudFXDelaySet(30);
            mbAudGuidePlay(0x3B6); /* guide sound-effect resource */
            mbObjMotionShiftSet(guideModel, kettouMotId[5],
                0.0f, 0.0f, HU3D_MOTATTR_LOOP);
            mbev_MgCallKettou();
        } else {
            mbWipeFadeOut();
            ev_CapKettouReturn(work);
        }
    } else {
        mbWipeFadeOut();
        ev_CapKettouReturn(work);
    }
    if (*(OMOBJ **)((u8 *)work + 0xBAC) != NULL) { /* retained CAPWORK field offset */
        mbGuideKill(*(OMOBJ **)((u8 *)work + 0xBAC)); /* retained CAPWORK field offset */
        *(OMOBJ **)((u8 *)work + 0xBAC) = NULL; /* retained CAPWORK field offset */
    }
    HuPrcEnd();
}

void mbev_CapKettouKill(void)
{
}

static int ev_CapKettouStart(CAPWORK *work)
{
    HuVecF masuPos;
    HuVecF pos;
    HuVecF avgPos = { 0.0f, 0.0f, 0.0f };
    int playerNo = work->playerNo;
    int masuId = GwPlayer[playerNo].masuId;
    int modelId = work->_unkB6C;
    int playerList[GW_PLAYER_MAX];
    int playerMot[GW_PLAYER_MAX];
    int playerNum = 0;
    int targetPlayer;
    int targetIndex;
    int resourceType;
    int amount;
    int i;
    int motionIndex;
    int sameMasuNum = 0;
    int playerObj;
    int playerMotion;
    int sprite[5];

    BOOL resumeF = work->flags._flag01;
    if (!resumeF) {
        for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (GwPlayer[i].masuId == masuId) {
            if (sameMasuNum < 2) {
                motionIndex = mbRandMod(2);
            } else {
                motionIndex = mbRandMod(3);
            }
            playerMot[i] = mbev_CapPlayerMotionCreate(&work->objWork, i,
                kettouPlayerMotTbl[motionIndex]);
            if (motionIndex == 2) {
                playerObj = mbPlayerObjIDGet(i);
                playerMotion = mbObjMotionIDGet(playerObj, playerMot[i]);
                Hu3DMotionAttrSet(playerMotion, 1);
            }
            mbPlayerPosGet(i, &pos);
            avgPos.x += pos.x;
            avgPos.y += pos.y;
            avgPos.z += pos.z;
            sameMasuNum++;
            if (i != playerNo) {
                playerList[playerNum++] = i;
            }
        } else {
            playerMot[i] = -1;
        }
        }
        if (playerNum < 1) {
            mbWinCreate(2, ev_CapKettouMesGet(0x003D0004), 13); /* duel scene message resource */
            mbWinTopWait();
            return 0;
        }
        avgPos.x /= (playerNum + 1);
        avgPos.y /= (playerNum + 1);
        avgPos.z /= (playerNum + 1);
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (GwPlayer[i].masuId == masuId) {
                mbPlayerPosGet(i, &pos);
                mbPlayerRotateStart(i,
                    (s16)(180.0 * atan2(avgPos.x - pos.x, avgPos.z - pos.z)
                        / M_PI), 15);
            }
        }
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (GwPlayer[i].masuId == masuId) {
                while (!mbPlayerRotateCheck(i)) {
                    HuPrcVSleep();
                }
                if (playerMot[i] >= 0) {
                    mbPlayerMotionShiftSet(i, playerMot[i], 0.0f, 8.0f,
                        HU3D_MOTATTR_LOOP);
                }
            }
        }
        HuPrcSleep(60);
    }
    mbMasuPosGet(masuId, &masuPos);
    mbObjPosSetV(modelId, &masuPos);
    masuPos.y += 200.0f;
    mbObjPosSetV(modelId, &masuPos);
    mbObjDispSet(modelId, TRUE);
    mbStatusDispForceSetAll(TRUE);
    mbCameraMovePlayer(playerNo, NULL, &capsuleCameraOfs,
        1500.0f, -1.0f, -1);
    mbCameraMoveWait();
    sprite[0] = espEntry(0x00110041, 120, 0); /* event sprite resource identifier */
    sprite[1] = espEntry(0x00110043, 110, 0); /* event sprite resource identifier */
    sprite[2] = espEntry(0x00110042, 100, 10); /* event sprite resource identifier */
    sprite[3] = espEntry(0x00110042, 100, 0); /* event sprite resource identifier */
    sprite[4] = espEntry(0x00110042, 100, 1); /* event sprite resource identifier */
    for (i = 0; i < 5; i++) {
        espDispOff((s16)sprite[i]);
    }
    mbMusBoardFadeOut(0, 0, 1000, 1000, 0x19, FALSE); /* board music fade channel mask */
    mbWipeSpecialFadeOutCreate(2, 60);
    mbAudFXPlay(0x3B6); /* event sound-effect resource */
    mbWinCreate(2, ev_CapKettouMesGet(0x003D0000), 13); /* duel scene message resource */
    mbWinTopWait();
    playerNum = 0;
    if (resumeF) {
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (i == playerNo
                || (mbPlayerCoinGet(i) <= 0 && mbPlayerStarGet(i) <= 0)) {
                continue;
            }
            if (GwSystem.tagF && mbev_CapPlayerCheck(playerNo, i)) {
                continue;
            }
            playerList[playerNum++] = i;
        }
    } else {
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (i == playerNo || GwPlayer[i].masuId != masuId
                || (mbPlayerCoinGet(i) <= 0 && mbPlayerStarGet(i) <= 0)) {
                continue;
            }
            playerList[playerNum++] = i;
        }
    }
    targetIndex = mbev_CapPlayerComSelKettouGet(playerNo,
        GwPlayer[playerNo].comF ? 0 : -1, playerList, playerNum);
    if (targetIndex < 0 || targetIndex >= playerNum
        || playerList[targetIndex] == playerNo) {
        for (i = 0; i < playerNum; i++) {
            targetIndex = i;
            break;
        }
    }
    if (targetIndex < 0 || targetIndex >= playerNum) {
        return 0;
    }
    targetPlayer = playerList[targetIndex];
    for (;;) {
        mbWinCreateChoice(1, ev_CapKettouMesGet(0x003D0003), 13, 0); /* duel scene message resource */
        if (mbPlayerCoinGet(playerNo) < 40
            || mbPlayerCoinGet(targetPlayer) < 40) {
            mbWinTopChoiceDisable(0);
        }
        if (mbPlayerCoinGet(playerNo) <= 0
            || mbPlayerStarGet(targetPlayer) <= 0) {
            mbWinTopChoiceDisable(1);
        }
        if (mbPlayerStarGet(playerNo) <= 0
            || mbPlayerStarGet(targetPlayer) <= 0) {
            mbWinTopChoiceDisable(2);
        }
        mbWinTopWait();
        resourceType = mbWinTopChoiceGet();
        if (resourceType >= 0 && resourceType <= 2) {
            break;
        }
        mbWinCreate(2, ev_CapKettouMesGet(0x003D0004), 13); /* duel scene message resource */
        mbWinTopWait();
        return 0;
    }
    memset(mgResultData, 0, 10);
    *(s16 *)((u8 *)mgResultData + 0) = playerNo;
    *(s16 *)((u8 *)mgResultData + 2) = targetPlayer;
    if (resourceType == 0) {
        amount = mbPlayerCoinGet(playerNo);
        if (mbPlayerCoinGet(targetPlayer) < amount) {
            amount = mbPlayerCoinGet(targetPlayer);
        }
        if (amount > 40) {
            amount = 40;
        }
        if (amount <= 0) {
            return 0;
        }
        {
            int add[GW_PLAYER_MAX] = { 0, 0, 0, 0 };
            BOOL disp[GW_PLAYER_MAX] = { FALSE, FALSE, FALSE, FALSE };
            add[playerNo] = -amount;
            add[targetPlayer] = -amount;
            disp[playerNo] = TRUE;
            disp[targetPlayer] = TRUE;
            mbCoinAddAllProcExecV(add, disp, FALSE);
        }
        *(s16 *)((u8 *)mgResultData + 4) = (s16)(amount * 2);
        *(s16 *)((u8 *)mgResultData + 6) = 0;
    } else if (resourceType == 1) {
        amount = mbPlayerCoinGet(playerNo);
        if (amount > 40) {
            amount = 40;
        }
        if (amount <= 0 || mbPlayerStarGet(targetPlayer) <= 0) {
            return 0;
        }
        mbCoinAddExec(playerNo, -amount);
        mbPlayerStarAdd(targetPlayer, -1);
        *(s16 *)((u8 *)mgResultData + 4) = (s16)amount;
        *(s16 *)((u8 *)mgResultData + 6) = 1;
    } else {
        if (mbPlayerStarGet(playerNo) <= 0
            || mbPlayerStarGet(targetPlayer) <= 0) {
            return 0;
        }
        mbPlayerStarAdd(playerNo, -1);
        mbPlayerStarAdd(targetPlayer, -1);
        *(s16 *)((u8 *)mgResultData + 4) = 0;
        *(s16 *)((u8 *)mgResultData + 6) = 2;
    }
    for (motionIndex = 0; motionIndex < 5; motionIndex++) {
        espDispOff((s16)sprite[motionIndex]);
    }
    mbev_CapDuelStatusDispSet(playerNo, targetPlayer, TRUE);
    return 1;
}

static void ev_CapKettouReturn(CAPWORK *work)
{
    HuVecF masuPos;
    HuVecF pos;
    int playerNo = work->playerNo;
    int initiator = *(s16 *)((u8 *)mgResultData + 0);
    int targetPlayer = *(s16 *)((u8 *)mgResultData + 2);
    int amount = *(s16 *)((u8 *)mgResultData + 4);
    int mode = *(s16 *)((u8 *)mgResultData + 6);
    int winner = -1;
    int loser = -1;
    int coinReward;
    int starReward;
    int starObj;
    int guide = work->_unkB6C;
    int i;
    char message[16];

    mbev_PlayerColMasu(playerNo, GwPlayer[playerNo].masuId, TRUE);
    mbPlayerPosGet(playerNo, &pos);
    mbObjPosSetV(guide, &pos);
    pos.y += 200.0f;
    mbObjPosSetV(guide, &pos);
    mbObjDispSet(guide, TRUE);
    mbObjMotionSet(guide, kettouMotId[1], HU3D_MOTATTR_LOOP);
    mbStatusDispForceSetAll(TRUE);
    mbCameraPlayerViewSetFast(playerNo, 0);
    mbCameraMoveWait();
    if (work->flags._flag02) {
        mbMusPlay(0, 26, 127, 0);
    }
    mbWipeFadeIn();
    mbPauseDisableSet(FALSE);
    if (initiator >= 0 && initiator < GW_PLAYER_MAX
        && targetPlayer >= 0 && targetPlayer < GW_PLAYER_MAX) {
        if (GwPlayer[initiator].mgCoinBonus > 0
            && GwPlayer[targetPlayer].mgCoinBonus <= 0) {
            winner = initiator;
            loser = targetPlayer;
        } else if (GwPlayer[targetPlayer].mgCoinBonus > 0
            && GwPlayer[initiator].mgCoinBonus <= 0) {
            winner = targetPlayer;
            loser = initiator;
        }
    }
    if (winner >= 0 && winner < GW_PLAYER_MAX) {
        mbMasuPosGet(GwPlayer[winner].masuId, &masuPos);
        pos = masuPos;
        pos.y += 200.0f;
        mbObjPosSetV(guide, &pos);
        if (loser >= 0 && loser < GW_PLAYER_MAX) {
            mbev_PlayerColMasu(loser, GwPlayer[loser].masuId, TRUE);
        }
        if (winner != playerNo) {
            mbev_PlayerColMasu(winner, GwPlayer[winner].masuId, TRUE);
            mbCameraPlayerViewSetFast(winner, 0);
            mbCameraMoveWait();
        }
        coinReward = (mode == 0 || mode == 1) ? amount : 0;
        starReward = mode == 1 ? 1 : (mode == 2 ? 2 : 0);
        if (coinReward > 0 && starReward > 0) {
            mbWinCreate(2, ev_CapKettouMesGet(0x003D000F), 13); /* duel scene message resource */
        } else if (starReward > 0) {
            mbWinCreate(2, ev_CapKettouMesGet(0x003D000E), 13); /* duel scene message resource */
        } else {
            mbWinCreate(2, ev_CapKettouMesGet(0x003D000D), 13); /* duel scene message resource */
        }
        mbWinTopInsertMesSet(mbPlayerNameMesGet(winner), 0);
        if (coinReward > 0) {
            sprintf(message, "%d", coinReward);
            mbWinTopInsertMesSet((u32)message, 1);
        }
        mbWinTopWait();
        if (coinReward > 0) {
            mbPlayerPosGet(winner, &pos);
            pos.y += 30.0f;
            mbCoinDispCapsuleCreate(&pos, coinReward);
            mbCoinAddExec(winner, coinReward);
            mbPlayerWinLoseVoicePlay(winner, 12, 0x243); /* win/lose voice resource */
            mbev_CapPlayerMotShiftWait(winner, 12, 0, TRUE);
        }
        if (starReward > 0) {
            mbPlayerStarAdd(winner, starReward);
            starObj = mbStarDispPlayerCreate(winner, starReward);
            while (!mbStarDispCheck(starObj)) {
                HuPrcVSleep();
            }
            mbPlayerWinLoseVoicePlay(winner, 7, 0x23D); /* win/lose voice resource */
            mbev_CapPlayerMotShiftWait(winner, 7, 0, TRUE);
        }
    } else {
        coinReward = (mode == 0 || mode == 1) ? amount : 0;
        starReward = mode == 1 ? 1 : (mode == 2 ? 2 : 0);
        if (coinReward > 0 && starReward > 0) {
            mbWinCreate(2, ev_CapKettouMesGet(0x003D0012), 13); /* duel scene message resource */
        } else if (starReward > 0) {
            mbWinCreate(2, ev_CapKettouMesGet(0x003D0011), 13); /* duel scene message resource */
        } else {
            mbWinCreate(2, ev_CapKettouMesGet(0x003D0010), 13); /* duel scene message resource */
            sprintf(message, "%d", coinReward);
            mbWinTopInsertMesSet((u32)message, 1);
        }
        mbWinTopWait();
        if (mode == 1 && amount > 0) {
            /* Mixed stakes are returned to the players who supplied them. */
            mbCoinAddExec(initiator, amount);
            mbPlayerStarAdd(targetPlayer, 1);
        } else if (mode == 2) {
            /* The two-star stake is split back equally on a draw. */
            mbPlayerStarAdd(initiator, starReward / 2);
            mbPlayerStarAdd(targetPlayer, starReward / 2);
        } else if (mode == 0 && amount > 0) {
            /* Coin stakes were stored as the combined amount. */
            mbCoinAddExec(initiator, amount / 2);
            mbCoinAddExec(targetPlayer, amount / 2);
        }
    }
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerMotionShiftSet(i, 1, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
    }
    mbObjMotionShiftSet(guide, kettouMotId[2], 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);
    mbAudFXPlay(0x3B8); /* event sound-effect resource */
    mbWinCreate(2, ev_CapKettouMesGet(0x003D0013), 13); /* duel scene message resource */
    mbWinTopWait();
    mbMusBoardFadeOut(0, 0, 1000, 1000, -1, FALSE);
}

static int ev_CapKettouMesGet(int messNo)
{
    if (GwSystem.curTime == FALSE) {
        return messNo;
    }
    return messNo + 23;
}

void mbev_CapDonkey(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;
    int playerNo = work->playerNo;
    int obj1;
    int obj2;
    int obj3;
    int i;

    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);
    mbev_CapWait(work);
    work->explodeObj = mbev_CapEffExplodeCreate();
    HuPrcVSleep();
    work->coinObj = mbev_CapEffCoinCreate();
    HuPrcVSleep();

    obj1 = mbev_CapObjCreate(&work->objWork, 0x000E000E, /* event model resource identifier */
        (int *)donkeyMotTbl, FALSE, 5, FALSE);
    mbObjDispSet(obj1, FALSE);
    obj2 = mbev_CapObjCreate(&work->objWork, 0x000E0021, /* event model resource identifier */
        NULL, FALSE, 5, FALSE);
    mbObjDispSet(obj2, FALSE);
    mbObjLayerSet(obj2, 3);
    mbev_CapObjPosSet(&work->objWork, obj2,
        GwPlayer[playerNo].masuId, NULL);
    obj3 = mbev_CapObjCreate(&work->objWork, 0x000C0044, /* event model resource identifier */
        NULL, FALSE, 5, FALSE);
    mbObjDispSet(obj3, FALSE);
    work->_unkB6C = obj1;
    work->_unkB70 = obj2;
    work->_unkB74 = obj3;

    if (!work->flags._flag03) {
        if (ev_CapDonkeyStart(work)) {
            if ((!mbPlayerAllComCheck() || GwSystem.mgComDispF)
                && mbMgRouletteNumGet(7) > 0) {
                mbWinCreate(2, 0x003E0008, -1); /* Donkey scene message resource */
                mbWinTopWait();
                mbObjMotionShiftSet(obj1, 10, 0.0f, 8.0f, 0);
                mbev_MgCallDonkey();
            } else {
                for (i = 0; i < GW_PLAYER_MAX; i++) {
                    int bonus = (int)mbRandMod(10);
                    if (!_CheckFlag(FLAG_MG_PRACTICE)) {
                        GwPlayer[i].mgCoinBonus = (s16)bonus;
                    }
                }
                mbWipeFadeOut();
                ev_CapDonkeyCoin(work);
            }
        }
    } else {
        ev_CapDonkeyCoin(work);
    }
    ev_CapDonkeyReturn(work);
    HuPrcEnd();
}

static int ev_CapDonkeyStart(CAPWORK *work)
{
    HuVecF masuPos;
    HuVecF pos;
    HuVecF direction;
    HuVecF cameraRot;
    HuVecF vel = { 0.0f, 0.0f, 0.0f };
    Mtx mtx;
    OMOBJ *omObj;
    s16 sprA;
    s16 sprB;
    int playerNo = work->playerNo;
    int masuId = GwPlayer[playerNo].masuId;
    int obj1 = work->_unkB6C;
    int obj2 = work->_unkB70;
    int obj3 = work->_unkB74;
    int i;
    int mode;
    int diceNo;
    int amount;
    int coinNo;
    int bank;
    int prevBank;
    int frameCount;
    int fileNum;
    float time;
    float value;
    float maxTime;
    float curTime;
    float ratio;
    float x;
    float y;
    float phase;
    float step;
    char message[16];

    mbMusBoardFadeOut(0, 0, 1000, 1000, 29, FALSE);
    mbAudFXPlay(0x454); /* event sound-effect resource */
    mbev_CapObjPosSet(&work->objWork, obj2, masuId, NULL);
    mbObjDispSet(obj2, TRUE);
    mbObjMotionTimeSet(obj2, 0.0f);
    mbObjMotionSpeedSet(obj2, 1.0f);
    mbMasuPosGet(masuId, &masuPos);
    mbObjPosSetV(obj3, &masuPos);
    mbCameraRotGet(&cameraRot);
    mbCameraMoveObj(obj3, NULL, &capsuleCameraOfs, 1500.0f, -1.0f,
        60);
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
    mbAudFXPlay(0x455); /* event sound-effect resource */
    omVibrate(playerNo, 20, 7, 3);
    do {
        maxTime = mbObjMotionMaxTimeGet(obj2);
        curTime = mbObjMotionTimeGet(obj2);
        ratio = curTime / maxTime;
        if (ratio > 1.0f) {
            ratio = 1.0f;
        }
        Hu3DMotionCalc(mbObjModelIDGet(obj2));
        Hu3DModelObjMtxGet(mbObjModelIDGet(obj2), capTreeFook, mtx);
        pos.x = mtx[0][3];
        pos.y = mtx[1][3];
        pos.z = mtx[2][3];
        mbPlayerPosSetV(playerNo, &pos);
        mbObjPosSetV(obj3, &pos);
        HuPrcVSleep();
    } while (ratio < 1.0f);
    mbMasuPosGet(masuId, &masuPos);
    PSVECSubtract(&pos, &masuPos, &direction);
    mbev_CapPlayerPosSet(&work->objWork, playerNo, masuId, &direction);
    mbAudFXPlay(0x456); /* event sound-effect resource */

    omObj = omAddObjEx(mbObjMan, -32768, 0, 0, -1, ev_CapDonkeyOMExec);
    omObj->data = HuMemDirectMallocNum(HEAP_HEAP, 0xBF4, HU_MEMNUM_OVL); /* CAPWORK object storage size */
    memcpy(omObj->data, work, 0xBF4); /* CAPWORK object storage size */
    omObj->work[3] = 0;
    omObj->work[2] = 0;
    omObj->work[1] = 0;
    omObj->work[0] = 0;
    while (omObj->work[0] < 2) {
        HuPrcVSleep();
    }
    mbev_CapPlayerPosSet(&work->objWork, playerNo, -1, &direction);
    mbPlayerRotGet(playerNo, &cameraRot);
    mbPlayerMotionShiftSet(playerNo, 9, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);

    value = MBCapsuleEffRandF();
    mode = value >= 0.3f;
    mbev_CapPlayerMotShiftSet(obj1, 6, 0, TRUE);
    mbObjMotionShiftSet(obj1, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbWinCreate(2, 0x003E0000, -1); /* Donkey scene message resource */
    mbWinTopWait();
    mbAudFXPlay(0x3A8); /* event sound-effect resource */
    mbObjMotionShiftSet(obj1, 10, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
    HuPrcSleep(30);
    mbAudFXPlay(0x45D); /* event sound-effect resource */
    fileNum = mbBoardDataNumGet(donkeyMgFile[mode]);
    sprA = mbev_CapSprCreate(&work->objWork, fileNum, 100, (s16)(mode ^ 1));
    for (i = 1; i < 60; i++) {
        time = (float)i / 60.0f;
        value = (float)sin((M_PI * 90.0 * time) / 180.0);
        x = 288.0f + 50.0f * (float)cos(
            (M_PI * 90.0 * time) / 180.0);
        y = 240.0f - 250.0f * (float)sin(
            (M_PI * 180.0 * time) / 180.0);
        espScaleSet(sprA, value, value);
        espPosSet(sprA, x, y);
        espZRotSet(sprA, -3.0f * 360.0f * time);
        HuPrcVSleep();
    }
    for (i = 1; i < 60; i++) {
        time = (float)i / 60.0f;
        value = 1.0f + 0.2f * (float)sin(
            (M_PI * 720.0 * time) / 180.0);
        espScaleSet(sprA, value, value);
        espPosSet(sprA, 288.0f, 240.0f);
        espZRotSet(sprA, 0.0f);
        HuPrcVSleep();
    }
    for (i = 1; i < 18; i++) {
        time = (float)i / 18.0f;
        value = 1.0f + 5.0f * (float)sin(
            (M_PI * 90.0 * time) / 180.0);
        espScaleSet(sprA, value, value);
        espPosSet(sprA, 288.0f, 240.0f);
        espTPLvlSet(sprA, 1.0f
            - (float)sin((M_PI * 90.0 * time) / 180.0));
        HuPrcVSleep();
    }
    espDispOff(sprA);
    mbObjMotionShiftSet(obj1, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);

    if (!mode) {
        mbWinCreate(2, 0x003E0001, -1); /* Donkey scene message resource */
        mbWinTopWait();
        mbWinCreate(2, 0x003E0002, -1); /* Donkey scene message resource */
        mbWinTopWait();
        mbPlayerRotateStart(playerNo, 0, 15);
        while (!mbPlayerRotateCheck(playerNo)) {
            HuPrcVSleep();
        }
        diceNo = mbDiceExec(playerNo, 11, (s8 *)donkeyDiceTbl,
            (GwSystem.boardNo == 3) ? mbRandMod(4) : mbRandMod(5),
            TRUE, TRUE, NULL, 0);
        amount = donkeyDiceResultTbl[diceNo];
        if (amount >= 0) {
            sprintf(message, "%d", amount);
            mbWinCreate(2, 0x003E0003, -1); /* Donkey scene message resource */
            mbWinTopInsertMesSet((u32)message, 0);
            mbWinTopWait();
            mbDiceFadeSet(playerNo);
            mbAudFXPlay(0x3A7); /* event sound-effect resource */
            mbev_CapPlayerMotShiftSet(obj1, 10, 0, TRUE);
            mbPlayerPosGet(playerNo, &pos);
            for (i = 0; i < amount; i++) {
                HuVecF coinPos = pos;
                coinPos.y += 600.0f;
                coinPos.x += (MBCapsuleEffRandF() - 0.5f)
                    * 100.0f * 0.5f;
                coinNo = mbev_CapEffCoinAdd(work->coinObj, &coinPos, &vel,
                    0.75f, 4.9f, 30, 4);
                if (coinNo >= 0) {
                    mbev_CapEffCoinMaxYSet(work->coinObj, coinNo,
                        pos.y + 150.0f);
                }
                HuPrcVSleep();
            }
            while (mbev_CapEffCoinNumGet(work->coinObj) > 0) {
                HuPrcVSleep();
            }
            mbObjMotionShiftSet(obj1, 1, 0.0f, 8.0f,
                HU3D_MOTATTR_LOOP);
            mbCoinAddDispExec(playerNo, amount, FALSE, TRUE);
            mbev_CapCoinDisp(playerNo, amount, TRUE, TRUE);
        } else {
            mbWinCreate(2, 0x003E0004, -1); /* Donkey scene message resource */
            mbWinTopWait();
            mbMusPauseFadeOut(0, TRUE, -1);
            mbDiceFadeSet(playerNo);
            mbAudFXPlay(0x3A7); /* event sound-effect resource */
            mbev_CapObjMotionSet(obj1, 30, 10, 1,
                0, HU3D_MOTATTR_LOOP, TRUE, TRUE);
            mbStarGetExec(playerNo);
            mbCameraMoveObj(obj3, &cameraRot, &capsuleCameraOfs,
                1500.0f, -1.0f, 60);
            mbMusPlay(0, 29, 127, 0);
        }
        mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
        return 0;
    }

    mbWinCreate(2, 0x003E0005, -1); /* Donkey scene message resource */
    mbWinTopWait();
    mbWinCreate(2, 0x003E0006, -1); /* Donkey scene message resource */
    mbWinTopWait();
    mbAudFXPlay(0x3A8); /* event sound-effect resource */
    mbev_CapPlayerMotShiftSet(obj1, 8, 0, TRUE);
    sprA = mbev_CapSprCreate(&work->objWork, 0x000E0022, 100, 0); /* event sprite resource identifier */
    espPosSet(sprA, 288.0f, 240.0f);
    espScaleSet(sprA, 0.0f, 0.0f);
    sprB = mbev_CapSprCreate(&work->objWork, 0x000E0023, 100, 0); /* event sprite resource identifier */
    espPosSet(sprB, 304.0f, 240.0f);
    espScaleSet(sprB, 0.0f, 0.0f);
    espAttrSet(sprB, 1);
    espBankSet(sprB, 0);
    for (i = 0; i <= 30; i++) {
        time = (float)i / 30.0f;
        value = (float)sin((M_PI * 180.0 * time) / 180.0)
            + (float)sin((M_PI * 90.0 * time) / 180.0);
        espScaleSet(sprA, value, value);
        espScaleSet(sprB, value, value);
        espPosSet(sprB, 288.0f + 16.0f * value, 240.0f);
        HuPrcVSleep();
    }
    frameCount = (int)(60.0f + MBCapsuleEffRandF() * 60.0f * 0.5f);
    phase = 0.0f;
    step = 0.3f;
    bank = 0;
    prevBank = 0;
    for (i = 0; i <= frameCount; i++) {
        phase += step;
        if (phase >= 10.0f) {
            phase -= 10.0f;
        }
        bank = (int)phase;
        if (bank >= 10) {
            bank = 9;
        }
        espBankSet(sprB, (s16)donkeyRouletteBankTbl[bank]);
        if (bank != prevBank) {
            mbAudFXPlay(0x3F1); /* event sound-effect resource */
            prevBank = bank;
        }
        HuPrcVSleep();
    }
    for (i = 0; i <= 60; i++) {
        if (step > 0.02f) {
            step -= 0.005f;
        }
        phase += step;
        if (phase >= 10.0f) {
            phase -= 10.0f;
        }
        bank = (int)phase;
        if (bank >= 10) {
            bank = 9;
        }
        espBankSet(sprB, (s16)donkeyRouletteBankTbl[bank]);
        if (bank != prevBank) {
            mbAudFXPlay(0x3F1); /* event sound-effect resource */
            prevBank = bank;
        }
        HuPrcVSleep();
    }
    mbAudFXPlay(0x46E); /* event sound-effect resource */
    for (i = 0; i <= 60; i++) {
        time = (float)i / 60.0f;
        value = 1.0f + 0.5f * (float)sin(
            (M_PI * 720.0 * time) / 180.0);
        espScaleSet(sprB, value, value);
        espPosSet(sprB, 288.0f + 16.0f * value, 240.0f);
        HuPrcVSleep();
    }
    for (i = 0; i <= 30; i++) {
        time = (float)i / 30.0f;
        value = 1.0f + 7.0f * (float)sin(
            (M_PI * 90.0 * time) / 180.0);
        espScaleSet(sprA, value, value);
        espScaleSet(sprB, value, value);
        espTPLvlSet(sprA, 1.0f
            - (float)sin((M_PI * 90.0 * time) / 180.0));
        espTPLvlSet(sprB, 1.0f
            - (float)sin((M_PI * 90.0 * time) / 180.0));
        espPosSet(sprB, 288.0f + 16.0f * value, 240.0f);
        HuPrcVSleep();
    }
    espDispOff(sprA);
    espDispOff(sprB);
    mbObjMotionShiftSet(obj1, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    sprintf(message, "%d", donkeyRouletteBankTbl[bank] + 1);
    mbWinCreate(2, 0x003E0007, -1); /* Donkey scene message resource */
    mbWinTopInsertMesSet((u32)message, 0);
    mbWinTopWait();
    memset(mgResultData, 0, 10);
    *(s16 *)((u8 *)mgResultData + 0) = (s16)playerNo;
    *(s16 *)((u8 *)mgResultData + 4) =
        (s16)(donkeyRouletteBankTbl[bank] + 1);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (!_CheckFlag(FLAG_MG_PRACTICE)) {
            GwPlayer[i].mgCoinBonus = 0;
        }
    }
    return 1;
}

static void ev_CapDonkeyCoin(CAPWORK *work)
{
    HuVecF masuPos;
    HuVecF pos;
    HuVecF direction;
    HuVecF vel = { 0.0f, 0.0f, 0.0f };
    Mtx mtx;
    int playerNo = work->playerNo;
    int masuId = GwPlayer[playerNo].masuId;
    int obj1 = work->_unkB6C;
    int obj2 = work->_unkB70;
    int obj3 = work->_unkB74;
    int coinVals[GW_PLAYER_MAX];
    int i;
    int coinNo;
    int amount = *(s16 *)((u8 *)mgResultData + 4);
    int ownBonus;

    mbStatusDispForceSetAll(TRUE);
    mbev_CapObjPosSet(&work->objWork, obj2, masuId, NULL);
    mbObjDispSet(obj2, TRUE);
    mbObjMotionTimeSet(obj2, 0.0f);
    mbObjMotionSpeedSet(obj2, 1.0f);
    mbMasuPosGet(masuId, &masuPos);
    mbObjPosSetV(obj3, &masuPos);
    mbObjMotionTimeSet(obj2, mbObjMotionMaxTimeGet(obj2));
    Hu3DMotionCalc(mbObjModelIDGet(obj2));
    HuPrcVSleep();
    Hu3DModelObjMtxGet(mbObjModelIDGet(obj2), capTreeFook, mtx);
    pos.x = mtx[0][3] - 80.0f;
    pos.y = mtx[1][3];
    pos.z = mtx[2][3] + 80.0f;
    mbObjPosSetV(obj3, &pos);
    mbev_PlayerColMasu(playerNo, masuId, TRUE);
    mbMasuPosGet(masuId, &masuPos);
    PSVECSubtract(&pos, &masuPos, &direction);
    mbPlayerRotSet(playerNo, 0.0f, 0.0f, 0.0f);
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
    mbev_CapPlayerPosSet(&work->objWork, playerNo, masuId, &direction);
    mbObjDispSet(obj1, TRUE);
    mbObjRotSet(obj1, 0.0f, -45.0f, 0.0f);
    mbObjMotionSet(obj1, 1, HU3D_MOTATTR_LOOP);
    pos.x = mtx[0][3];
    pos.y = mtx[1][3];
    pos.z = mtx[2][3];
    mbObjPosSetV(obj1, &pos);
    mbMasuPosGet(masuId, &masuPos);
    PSVECSubtract(&pos, &masuPos, &direction);
    mbev_CapObjPosSet(&work->objWork, obj1, masuId, &direction);
    mbCameraEyeSetV(&pos);
    mbCameraMoveObj(obj3, NULL, &capsuleCameraOfs, 1500.0f, -1.0f, 1);
    mbCameraMoveWait();
    if (work->flags._flag03) {
        mbMusPlay(0, 30, 127, 0);
    }
    mbWipeFadeIn();
    mbPauseDisableSet(FALSE);
    {
        char message[16];
        sprintf(message, "%d", amount);
        mbWinCreate(2, 0x003E0009, -1); /* Donkey scene message resource */
        mbWinTopInsertMesSet((u32)message, 0);
        mbWinTopWait();
    }
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        coinVals[i] = amount * GwPlayer[i].mgCoinBonus;
    }
    do {
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (coinVals[i] > 0) {
                HuVecF playerPos;
                HuVecF coinPos;
                mbPlayerPosGet(i, &playerPos);
                coinPos = playerPos;
                coinPos.y += 1500.0f;
                coinPos.x += (MBCapsuleEffRandF() - 0.5f)
                    * 100.0f * 0.5f;
                coinNo = mbev_CapEffCoinAdd(work->coinObj, &coinPos, &vel,
                    0.75f, 4.9f, 30, 4);
                if (coinNo >= 0) {
                    mbev_CapEffCoinMaxYSet(work->coinObj, coinNo,
                        playerPos.y + 150.0f);
                    coinVals[i]--;
                }
            }
        }
        HuPrcVSleep();
    } while (coinVals[0] > 0 || coinVals[1] > 0
        || coinVals[2] > 0 || coinVals[3] > 0);
    while (mbev_CapEffCoinNumGet(work->coinObj) > 0) {
        HuPrcVSleep();
    }
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        coinVals[i] = amount * GwPlayer[i].mgCoinBonus;
    }
    mbCoinAddAllProcExecV(coinVals, (BOOL *)coinVals, FALSE);
    ownBonus = GwPlayer[playerNo].mgCoinBonus;
    if (ownBonus > 0) {
        mbPlayerWinLoseVoicePlay(playerNo, 12, 0x243); /* win/lose voice resource */
        mbev_CapPlayerMotShiftWait(playerNo, 12, 0, TRUE);
    } else {
        mbPlayerWinLoseVoicePlay(playerNo, 13, 0x249); /* win/lose voice resource */
        mbev_CapPlayerMotShiftWait(playerNo, 13, 0, TRUE);
    }
    mbev_CapPlayerMotShiftWait(playerNo, 1, HU3D_MOTATTR_LOOP, TRUE);
}

static void ev_CapDonkeyReturn(CAPWORK *work)
{
    HuVecF pos;
    HuVecF masuPos;
    Mtx mtx;
    int playerNo = work->playerNo;
    int masuId = GwPlayer[playerNo].masuId;
    int obj1 = work->_unkB6C;
    int obj2 = work->_unkB70;
    int obj3 = work->_unkB74;
    int i;
    float ratio;
    float scale;
    float yOffset;
    float yOffset2;
    float maxTime;
    float curTime;

    mbPlayerRotateStart(playerNo, 0x87, 15); /* Donkey return facing angle */
    while (!mbPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    mbAudFXPlay(0x3A8); /* event sound-effect resource */
    mbev_CapPlayerMotShiftSet(obj1, 0x0B, HU3D_MOTATTR_NONE, TRUE); /* Donkey return motion slot */
    mbWinCreate(2, 0x003E000A, -1); /* Donkey scene message resource */
    mbWinTopWait();
    mbObjMotionShiftSet(obj1, 1, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
    HuPrcSleep(20);
    mbev_CapObjPosSet(&work->objWork, obj1, -1, NULL);
    mbAudFXPlay(0x46F); /* event sound-effect resource */
    mbObjMotionShiftSet(obj1, 4, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);

    for (i = 1; i <= 30; i++) {
        ratio = (float)i / 30.0f;
        scale = 1.0f - 0.7f * (float)sin(
            (M_PI * 90.0 * ratio) / 180.0);
        Hu3DModelObjMtxGet(mbObjModelIDGet(obj2), capTreeFook, mtx);
        pos.x = mtx[0][3] + 80.0f;
        yOffset = 100.0f * (float)sin(
            (M_PI * 90.0 * ratio) / 180.0);
        yOffset2 = 2.5f * yOffset;
        yOffset = 100.0f * (float)sin(
            (M_PI * 180.0 * ratio) / 180.0);
        pos.y = mtx[1][3] + yOffset + yOffset2;
        pos.z = mtx[2][3] - 80.0f;
        mbObjPosSetV(obj1, &pos);
        mbObjScaleSet(obj1, scale, scale, scale);
        HuPrcVSleep();
    }
    mbev_CapEffDustCloudAdd(work->explodeObj, &pos);
    HuPrcVSleep();
    mbObjDispSet(obj1, FALSE);
    mbev_CapPlayerPosSet(&work->objWork, playerNo, -1, NULL);
    mbPlayerMotionShiftSet(playerNo, 2, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);

    for (i = 1; i <= 30; i++) {
        ratio = (float)i / 30.0f;
        Hu3DModelObjMtxGet(mbObjModelIDGet(obj2), capTreeFook, mtx);
        pos.x = mtx[0][3] - 80.0f * (1.0f - ratio);
        pos.y = mtx[1][3];
        pos.z = mtx[2][3] + 80.0f * (1.0f - ratio);
        mbPlayerPosSetV(playerNo, &pos);
        HuPrcVSleep();
    }

    mbCameraPlayerViewSet(playerNo, 0);
    mbPlayerRotateStart(playerNo, 0, 15);
    while (!mbPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    mbMusBoardFadeOut(0, 0, 1000, 1000, -1, FALSE);
    mbAudFXPlay(0x476); /* event sound-effect resource */
    mbObjMotionSpeedSet(obj2, -1.0f);
    do {
        maxTime = mbObjMotionMaxTimeGet(obj2);
        curTime = mbObjMotionTimeGet(obj2);
        ratio = curTime / maxTime;
        if (ratio < 0.0f) {
            ratio = 0.0f;
        }
        Hu3DMotionCalc(mbObjModelIDGet(obj2));
        Hu3DModelObjMtxGet(mbObjModelIDGet(obj2), capTreeFook, mtx);
        pos.x = mtx[0][3];
        pos.y = mtx[1][3];
        pos.z = mtx[2][3];
        mbPlayerPosSetV(playerNo, &pos);
        mbObjPosSetV(obj3, &pos);
        HuPrcVSleep();
    } while (ratio > 0.0f);
    mbMasuPosGet(masuId, &masuPos);
    mbPlayerPosSetV(playerNo, &masuPos);
    mbPlayerColSnapPlayerSet(playerNo, TRUE);
    mbCameraMoveWait();
}

static void ev_CapDonkeyOMExec(OMOBJ *obj)
{
    CAPWORK *work = obj->data;
    HuVecF pos;
    HuVecF masuPos;
    HuVecF direction;
    Mtx mtx;
    int playerNo;
    int masuId;
    int obj1;
    int obj2;
    int obj3;
    float time;
    float nextTime;
    float s;
    float scaleA;
    float scaleB;
    float trig;

    if (mbExitCheck() || obj->work[3] != 0) {
        omDelObjEx(mbObjMan, obj);
        return;
    }
    playerNo = work->playerNo;
    masuId = GwPlayer[playerNo].masuId;
    obj1 = work->_unkB6C;
    obj2 = work->_unkB70;
    obj3 = work->_unkB74;

    switch (obj->work[0]) {
    case 0:
        mbObjDispSet(obj1, TRUE);
        mbObjRotSet(obj1, 0.0f, -45.0f, 0.0f);
        mbObjMotionSet(obj1, 4, HU3D_MOTATTR_NONE);
        obj->work[0]++;
        obj->work[1] = 0;
        /* fall through */
    case 1:
        obj->work[1]++;
        time = (float)obj->work[1] / 48.0f;
        Hu3DModelObjMtxGet(mbObjModelIDGet(obj2), capTreeFook, mtx);
        pos.x = mtx[0][3] + 80.0f + 100.0f;
        trig = 100.0f * (float)cos(
            (M_PI * 90.0 * time) / 180.0);
        pos.y = mtx[1][3] + trig * 10.0f;
        pos.z = mtx[2][3] - 80.0f - 100.0f;
        mbObjPosSetV(obj1, &pos);
        if (time >= 1.0f) {
            mbObjMotionShiftSet(obj1, 2, 0.0f, 8.0f,
                HU3D_MOTATTR_NONE);
            CharEffectHipDropCreate((s16)GwPlayer[playerNo].charNo, &pos);
            obj->work[0]++;
            obj->work[1] = 0;
        }
        break;
    case 2:
        obj->work[1]++;
        time = (float)obj->work[1] / 36.0f;
        nextTime = (float)(obj->work[1] + 1) / 36.0f;
        if (nextTime > 1.0f) {
            nextTime = 1.0f;
        }
        s = (float)sin((M_PI * 180.0 * nextTime) / 180.0);
        scaleA = 1.0f + 0.33f * s;
        scaleB = 1.0f - 0.33f * s;
        mbObjScaleSet(obj2, scaleA, scaleB, scaleA);
        Hu3DModelObjMtxGet(mbObjModelIDGet(obj2), capTreeFook, mtx);
        trig = 100.0f * (float)cos(
            (M_PI * 90.0 * time) / 180.0);
        pos.x = mtx[0][3] + 80.0f + trig;
        pos.y = mtx[1][3] + 100.0f
            * (float)sin((M_PI * 180.0 * time) / 180.0) * 2.0f;
        pos.z = mtx[2][3] - 80.0f - trig;
        mbObjPosSetV(obj1, &pos);
        if (obj->work[1] == 0x1B) { /* Donkey animation state tag */
            mbObjMotionShiftSet(obj1, 3, 0.0f, 8.0f,
                HU3D_MOTATTR_NONE);
        }
        if (time >= 1.0f) {
            CharModelLandDustCreate((s16)GwPlayer[playerNo].charNo, &pos);
            obj->work[0]++;
            obj->work[1] = 0;
        }
        break;
    case 3:
        obj->work[1]++;
        time = (float)(obj->work[1] + 1) / 12.0f;
        if (time > 1.0f) {
            time = 1.0f;
        }
        s = (float)sin((M_PI * 180.0 * time) / 180.0);
        scaleA = 1.0f + 0.1f * s;
        scaleB = 1.0f - 0.1f * s;
        mbObjScaleSet(obj2, scaleA, scaleB, scaleA);
        Hu3DModelObjMtxGet(mbObjModelIDGet(obj2), capTreeFook, mtx);
        pos.x = mtx[0][3] + 80.0f;
        pos.y = mtx[1][3];
        pos.z = mtx[2][3] - 80.0f;
        mbObjPosSetV(obj1, &pos);
        if (mbObjMotionShiftIDGet(obj1) == -1
            && mbObjMotionEndCheck(obj1) && time >= 1.0f) {
            mbObjMotionShiftSet(obj1, 1, 0.0f, 8.0f,
                HU3D_MOTATTR_LOOP);
            mbMasuPosGet(masuId, &masuPos);
            PSVECSubtract(&pos, &masuPos, &direction);
            mbev_CapObjPosSet(&work->objWork, obj1, masuId, &direction);
            obj->work[0]++;
            obj->work[1] = 0;
        }
        break;
    default:
        obj->work[3] = 1;
        break;
    }
}

void mbev_CapDonkeyKill(void)
{
}

void mbev_CapKoopa(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;
    int playerNo = work->playerNo;
    int objId;
    int i;

    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);
    mbev_CapWait(work);
    work->explodeObj = mbev_CapEffExplodeCreate();
    HuPrcVSleep();
    work->coinObj = mbev_CapEffCoinCreate();
    HuPrcVSleep();
    objId = mbev_CapObjCreate(&work->objWork, 0x000E0000, /* event model resource identifier */
        (int *)koopaMotTbl, FALSE, 5, FALSE);
    mbObjDispSet(objId, FALSE);
    mbPlayerColSnapPlayerSet(playerNo, TRUE);
    work->_unkB6C = objId;

    if (!work->flags._flag04) {
        if (ev_CapKoopaStart(work)) {
            if ((!mbPlayerAllComCheck() || GwSystem.mgComDispF)
                && mbMgRouletteNumGet(4) > 0) {
                mbWinCreate(2, 0x003F000D, 13); /* Koopa scene message resource */
                mbWinTopWait();
                mbObjMotionShiftSet(objId, 5, 0.0f, 8.0f, 0);
                mbev_MgCallKoopa();
            } else {
                for (i = 0; i < GW_PLAYER_MAX; i++) {
                    int bonus = (int)mbRandMod(2);
                    if (!_CheckFlag(FLAG_MG_PRACTICE)) {
                        GwPlayer[i].mgCoinBonus = (s16)bonus;
                    }
                }
                mbWipeFadeOut();
                ev_CapKoopaCoin(work);
            }
        }
    } else {
        ev_CapKoopaCoin(work);
    }
    ev_CapKoopaReturn(work);
    HuPrcEnd();
}

void mbev_CapKoopaKill(void)
{
}

static int ev_CapKoopaStart(CAPWORK *work)
{
    HuVecF masuPos;
    HuVecF pos;
    HuVecF cameraPos;
    int ids[4];
    int playerNo = work->playerNo;
    int masuId = GwPlayer[playerNo].masuId;
    int objId = work->_unkB6C;
    int playerMot0;
    int playerMot1;
    int spr;
    int starObj;
    int fileNum;
    int diceNo;
    int value;
    int selector;
    int i;
    int mode;
    float t;
    float scale;
    float randomValue;
    char message[16];

    mbPlayerRotateStart(playerNo, 0, 15);
    playerMot0 = mbev_CapPlayerMotionCreate(&work->objWork, playerNo,
        0x00930017); /* event resource identifier */
    playerMot1 = mbev_CapPlayerMotionCreate(&work->objWork, playerNo,
        0x00930019); /* event resource identifier */
    spr = mbev_CapSprCreate(&work->objWork, 0x000E0024, 100, 0); /* event sprite resource identifier */
    espPosSet((s16)spr, 288.0f, 240.0f);
    espScaleSet((s16)spr, 4.0f, 4.0f);
    espTPLvlSet((s16)spr, 0.0f);
    espColorSet((s16)spr, 255, 0, 0);
    espDispOff((s16)spr);
    espAttrSet((s16)spr, 8);
    espDrawNoSet((s16)spr, 0);
    espDispOff((s16)spr);
    mbMusFadeOutSpeed(0, 1000);
    while (mbMusCheck(0)) {
        HuPrcVSleep();
    }
    while (!mbPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, playerMot0, 0.0f, 8.0f, 0);
    mbCameraPlayerViewSet(playerNo, 0);
    mbEffFadeCreate(30, 160);
    work->flags._flag06 = TRUE;
    mbMusPlay(0, 27, 127, 0);
    espDispOn((s16)spr);
    for (i = 0; i < 360; i += 2) {
        if (i == 60) {
            mbAudFXPlay(0x453); /* event sound-effect resource */
        }
        if (i == 90 || i == 270 || i == 450) {
            omVibrate(playerNo, 20, 7, 3);
        }
        t = (float)sin((M_PI * mbAngleWrap((float)i)) / 180.0);
        espTPLvlSet((s16)spr, (float)fabs(t));
        HuPrcVSleep();
    }
    espDispOff((s16)spr);
    mbPlayerMotionShiftSet(playerNo, playerMot1, 0.0f, 8.0f, 0);
    mbPlayerPosGet(playerNo, &cameraPos);
    mbObjDispSet(objId, TRUE);
    mbObjMotionSet(objId, 2, 0);
    mbObjMotionTimeSet(objId, 30.0f);
    mbObjMotionSpeedSet(objId, 1.0f);
    while (mbObjMotionTimeGet(objId) < 60.0f) {
        t = (mbObjMotionTimeGet(objId) - 30.0f) / 30.0f;
        mbMasuPosGet(masuId, &masuPos);
        masuPos.y += (float)cos((M_PI * 90.0 * t) / 180.0)
            * 100.0f * 6.0f;
        mbObjPosSetV(objId, &masuPos);
        HuPrcVSleep();
    }
    mbev_CapEffDustHeavyAdd(work->explodeObj, &masuPos);
    mbev_CapObjPosSet(&work->objWork, objId, masuId, NULL);
    mbAudFXPlay(0x45F); /* event sound-effect resource */
    mbev_CapVibrate(1);
    work->_unkB70 = mbev_CapPlayerSquishSet(ids, masuId);
    for (i = 0; i < 4; i++) {
        ((int *)((u8 *)work + 0xB74))[i] = ids[i]; /* retained CAPWORK field offset */
    }
    while (!mbObjMotionEndCheck(objId)) {
        HuPrcVSleep();
    }
    HuPrcVSleep();
    mbObjMotionShiftSet(objId, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    HuPrcSleep(30);
    mbEffFadeOutSet(30);
    HuPrcSleep(30);
    randomValue = MBCapsuleEffRandF();
    mode = randomValue >= 0.3f;
    HuDataDirClose(0x000E0000); /* event archive resource identifier */
    mbAudFXDelaySet(30);
    mbAudFXPlay(0x3CB); /* event sound-effect resource */
    mbev_CapPlayerMotShiftSet(objId, 3, 0, TRUE);
    mbObjMotionShiftSet(objId, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbWinCreate(2, 0x003F000D, 13); /* Koopa scene message resource */
    mbWinTopWait();
    if (mbPlayerCoinGet(playerNo) <= 0) {
        mbObjMotionShiftSet(objId, 6, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
        mbWinCreate(2, 0x003F0001, 13); /* Koopa scene message resource */
        mbWinTopWait();
        mbev_CapPlayerMotShiftSet(objId, 1, HU3D_MOTATTR_LOOP, TRUE);
        work->flags._flag05 = TRUE;
        return 0;
    }
    mbAudFXDelaySet(30);
    mbAudFXPlay(0x3CD); /* event sound-effect resource */
    mbObjMotionShiftSet(objId, 5, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
    HuPrcSleep(30);
    mbAudFXPlay(0x45D); /* event sound-effect resource */
    fileNum = mbBoardDataNumGet(koopaMgFile[mode]);
    spr = mbev_CapSprCreate(&work->objWork, fileNum, 100, (s16)(mode ^ 1));
    for (i = 1; i < 60; i++) {
        t = (float)i / 60.0f;
        scale = (float)sin((M_PI * 90.0 * t) / 180.0);
        pos.x = 288.0f;
        pos.y = 240.0f
            - 250.0f * (float)sin((M_PI * 180.0 * t) / 180.0)
            + 50.0f * (float)cos((M_PI * 180.0 * t) / 180.0);
        pos.z = 0.0f;
        espPosSet((s16)spr, pos.x, pos.y);
        espScaleSet((s16)spr, scale, scale);
        espZRotSet((s16)spr, 3.0f * 360.0f * t);
        HuPrcVSleep();
    }
    for (i = 1; i < 60; i++) {
        t = (float)i / 60.0f;
        scale = 1.0f
            + 0.2f * (float)sin((M_PI * 720.0 * t) / 180.0);
        espPosSet((s16)spr, 288.0f, 240.0f);
        espScaleSet((s16)spr, scale, scale);
        espZRotSet((s16)spr, 0.0f);
        HuPrcVSleep();
    }
    for (i = 1; i < 18; i++) {
        t = (float)i / 18.0f;
        scale = 1.0f
            + 5.0f * (float)sin((M_PI * 180.0 * t) / 180.0);
        espPosSet((s16)spr, 288.0f, 240.0f);
        espScaleSet((s16)spr, scale, scale);
        espTPLvlSet((s16)spr,
            1.0f - (float)sin((M_PI * 90.0 * t) / 180.0));
        HuPrcVSleep();
    }
    espDispOff((s16)spr);
    if (!mode) {
        mbAudFXDelaySet(30);
        mbAudFXPlay(0x3CB); /* event sound-effect resource */
        mbObjMotionShiftSet(objId, 3, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
        mbWinCreate(2, 0x003F0002, 13); /* Koopa scene message resource */
        mbWinTopWait();
        mbObjMotionShiftSet(objId, 1, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
        mbWinCreate(2, 0x003F0003, 13); /* Koopa scene message resource */
        mbWinTopWait();
        mbObjPosGet(objId, &pos);
        diceHitTimer = (int)(60.0f * (0.33f + MBCapsuleEffRandF()));
        koopaMdlId = objId;
        selector = (GwSystem.boardNo != 3) ? mbRandMod(5) : mbRandMod(4);
        mbDiceExec(-1, 12, (s8 *)koopaDiceTbl, selector,
            FALSE, FALSE, &pos, 2);
        mbDicePadBtnHookSet(-1,
            (u16 (*)(int))ev_CapKoopaDicePadBtnHook);
        mbDiceMotHookSet(-1,
            (void (*)(int))ev_CapKoopaDiceMotHook);
        while (!mbDiceKillCheck(-1)) {
            HuPrcVSleep();
        }
        diceNo = mbDiceResultGet(-1);
        HuPrcSleep(30);
        value = koopaDiceResultTbl[diceNo];
        if (value >= 0) {
            sprintf(message, "%d", -value);
            mbWinCreate(2, 0x003F0004, 13); /* Koopa scene message resource */
            mbWinTopInsertMesSet((u32)message, 0);
            mbWinTopWait();
            mbDiceFadeSet(-1);
            mbAudFXDelaySet(30);
            mbAudFXPlay(0x3CD); /* event sound-effect resource */
            mbev_CapPlayerMotShiftSet(objId, 5, 0, TRUE);
            if (value > mbPlayerCoinGet(playerNo)) {
                value = mbPlayerCoinGet(playerNo);
            }
            mbCoinAddProcExec(playerNo, -value, -value, FALSE);
            mbev_CapPlayerMotShiftSet(objId, 1, HU3D_MOTATTR_LOOP, TRUE);
        } else {
            mbWinCreate(2, 0x003F0005, 13); /* Koopa scene message resource */
            mbWinTopWait();
            mbDiceFadeSet(-1);
            if (mbPlayerStarGet(playerNo) > 0) {
                mbAudFXDelaySet(30);
                mbAudFXPlay(0x3CD); /* event sound-effect resource */
                mbev_CapPlayerMotShiftSet(objId, 5, 0, TRUE);
                HuPrcSleep(12);
                starObj = mbStarDispPlayerCreate(playerNo, -1);
                while (!mbStarDispCheck(starObj)) {
                    HuPrcVSleep();
                }
                mbPlayerStarAdd(playerNo, -1);
                mbev_CapPlayerMotShiftSet(objId, 1,
                    HU3D_MOTATTR_LOOP, TRUE);
            } else {
                mbAudFXDelaySet(30);
                mbAudFXPlay(0x3CD); /* event sound-effect resource */
                mbObjMotionShiftSet(objId, 6, 0.0f, 8.0f,
                    HU3D_MOTATTR_LOOP);
                mbWinCreate(2, 0x003F0006, 13); /* Koopa scene message resource */
                mbWinTopWait();
                mbev_CapPlayerMotShiftSet(objId, 1,
                    HU3D_MOTATTR_LOOP, TRUE);
            }
        }
        return 0;
    }

    mbAudFXDelaySet(30);
    mbAudFXPlay(0x3CB); /* event sound-effect resource */
    mbObjMotionShiftSet(objId, 3, 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);
    mbWinCreate(2, 0x003F0007, 13); /* Koopa scene message resource */
    mbWinTopWait();
    selector = mbRandMod(3);
    mbAudFXDelaySet(30);
    mbAudFXPlay(0x3CD); /* event sound-effect resource */
    mbev_CapPlayerMotShiftSet(objId, 5, 0, TRUE);
    mbWinCreate(2, GwSystem.tagF ? 0x003F0009 : 0x003F0008, 13); /* Koopa scene message resource */
    mbWinTopInsertMesSet(koopaLoseMesTbl[selector], 0);
    mbWinTopWait();
    mbev_CapPlayerMotShiftSet(objId, 1, HU3D_MOTATTR_LOOP, TRUE);
    memset(mgResultData, 0, 10);
    *(s16 *)((u8 *)mgResultData + 8) = (s16)selector;
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (!_CheckFlag(FLAG_MG_PRACTICE)) {
            GwPlayer[i].mgCoinBonus = 0;
        }
    }
    return 1;
}

static int ev_CapKoopaCoin(CAPWORK *work)
{
    HuVecF masuPos;
    int ids[4];
    int add[GW_PLAYER_MAX];
    int teamTbl[2][GW_PLAYER_MAX];
    int teamCount[2] = { 0, 0 };
    BOOL teamLose[2] = { FALSE, FALSE };
    int playerNo = work->playerNo;
    int masuId = GwPlayer[playerNo].masuId;
    int objId = work->_unkB6C;
    int resultType = *(s16 *)((u8 *)mgResultData + 8);
    int loseCount = 0;
    BOOL hasResource = TRUE;
    int i;
    int t;
    int outer;
    BOOL removed;
    BOOL tagF = GwSystem.tagF;

    mbev_PlayerColMasu(playerNo, masuId, TRUE);
    work->_unkB70 = mbev_CapPlayerSquishVoiceSet(ids, masuId, TRUE);
    for (i = 0; i < 4; i++) {
        ((int *)((u8 *)work + 0xB74))[i] = ids[i]; /* retained CAPWORK field offset */
    }
    mbMasuPosGet(masuId, &masuPos);
    mbObjPosSetV(objId, &masuPos);
    mbObjDispSet(objId, TRUE);
    mbObjMotionSet(objId, 1, HU3D_MOTATTR_LOOP);
    mbev_CapObjPosSet(&work->objWork, objId, masuId, NULL);
    mbStatusDispForceSetAll(TRUE);
    mbCameraEyeSetV(&masuPos);
    mbCameraPlayerViewSetFast(playerNo, 0);
    mbCameraMoveWait();
    if (work->flags._flag04) {
        mbMusPlay(0, 27, 127, 0);
    }
    mbWipeFadeIn();
    mbPauseDisableSet(FALSE);

    if (!tagF) {
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (GwPlayer[i].mgCoinBonus <= 0) {
                loseCount++;
                if (resultType == 2) {
                    if (mbPlayerCapsuleNumGet(i) > 0) {
                        hasResource = FALSE;
                    }
                } else if (mbPlayerCoinGet(i) > 0) {
                    hasResource = FALSE;
                }
            }
        }
    } else {
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            int team = GwPlayer[i].team ? 1 : 0;
            teamTbl[team][teamCount[team]++] = i;
        }
        for (t = 0; t < 2; t++) {
            if (teamCount[t] > 1
                && GwPlayer[teamTbl[t][0]].mgCoinBonus <= 0
                && GwPlayer[teamTbl[t][1]].mgCoinBonus <= 0) {
                teamLose[t] = TRUE;
                loseCount++;
                if (resultType == 2) {
                    if (mbPlayerCapsuleNumGet(teamTbl[t][0]) > 0) {
                        hasResource = FALSE;
                    }
                } else if (mbPlayerCoinGet(teamTbl[t][0]) > 0) {
                    hasResource = FALSE;
                }
            }
        }
    }
    mbAudFXDelaySet(30);
    mbAudFXPlay(0x3CB); /* event sound-effect resource */
    mbObjMotionShiftSet(objId, 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbWinCreate(2, tagF ? 0x003F000F : 0x003F000E, 13); /* Koopa scene message resource */
    mbWinTopInsertMesSet(koopaLoseMesTbl2[resultType], 0);
    mbWinTopWait();
    mbAudFXDelaySet(30);
    mbAudFXPlay(0x3CD); /* event sound-effect resource */
    mbev_CapPlayerMotShiftSet(objId, 5, 0, TRUE);
    mbAudFXPlay(0x427); /* event sound-effect resource */
    if (loseCount == 0) {
        HuPrcSleep(30);
        mbAudFXPlay(0x3CC); /* event sound-effect resource */
        mbObjMotionShiftSet(objId, 6, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
        mbWinCreate(2, tagF ? 0x003F0013 : 0x003F0012, 13); /* Koopa scene message resource */
        mbWinTopWait();
    } else if (hasResource) {
        HuPrcSleep(30);
        mbAudFXPlay(0x3CC); /* event sound-effect resource */
        mbObjMotionShiftSet(objId, 6, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
        mbWinCreate(2, resultType == 2 ? 0x003F0011 : 0x003F0010, /* Koopa scene message resource */
            13);
        mbWinTopWait();
    } else if (!tagF) {
        if (resultType == 0 || resultType == 1) {
            for (i = 0; i < GW_PLAYER_MAX; i++) {
                if (GwPlayer[i].mgCoinBonus <= 0) {
                    int coins = mbPlayerCoinGet(i);
                    add[i] = resultType == 0 ? -((coins + 1) >> 1)
                        : -coins;
                    omVibrate(i, 20, 20, 0);
                } else {
                    add[i] = 0;
                }
            }
            mbCoinAddAllProcExecV(add, (BOOL *)add, FALSE);
        } else {
            for (i = 0; i < GW_PLAYER_MAX; i++) {
                if (GwPlayer[i].mgCoinBonus <= 0) {
                    omVibrate(i, 20, 20, 0);
                }
            }
            for (outer = 0; outer < mbPlayerCapsuleMaxGet(); outer++) {
                removed = FALSE;
                for (i = 0; i < GW_PLAYER_MAX; i++) {
                    if (GwPlayer[i].mgCoinBonus <= 0) {
                        mbPlayerCapsuleRemove(i, 0);
                        removed = TRUE;
                    }
                }
                if (removed) {
                    HuPrcSleep(10);
                }
            }
        }
    } else {
        for (t = 0; t < 2; t++) {
            if (teamLose[t]) {
                int leader = teamTbl[t][0];
                int member = teamTbl[t][teamCount[t] > 1 ? 1 : 0];
                if (resultType == 0 || resultType == 1) {
                    int coins = mbPlayerCoinGet(leader);
                    int delta = resultType == 0 ? -((coins + 1) >> 1)
                        : -coins;
                    mbCoinAddDispExec(leader, delta, FALSE, FALSE);
                }
                omVibrate(leader, 20, 20, 0);
                omVibrate(member, 20, 20, 0);
            }
        }
        if (resultType == 2) {
            for (outer = 0; outer < mbPlayerCapsuleMaxGet(); outer++) {
                removed = FALSE;
                for (t = 0; t < 2; t++) {
                    if (teamLose[t]) {
                        mbPlayerCapsuleRemove(teamTbl[t][0], 0);
                        removed = TRUE;
                    }
                }
                if (removed) {
                    HuPrcSleep(10);
                }
            }
        }
    }
    mbev_CapPlayerMotShiftSet(objId, 1, HU3D_MOTATTR_LOOP, TRUE);
    return 0;
}

static void ev_CapKoopaReturn(CAPWORK *work)
{
    HuVecF masuPos;
    int ids[4];
    int playerNo = work->playerNo;
    int masuId = GwPlayer[playerNo].masuId;
    int objId = work->_unkB6C;
    int count = work->_unkB70;
    int i;
    float time;

    for (i = 0; i < 4; i++) {
        ids[i] = ((int *)((u8 *)work + 0xB74))[i]; /* retained CAPWORK field offset */
    }
    mbAudFXDelaySet(30);
    mbAudFXPlay(0x3CB); /* event sound-effect resource */
    mbev_CapPlayerMotShiftSet(objId, 3, 0, TRUE);
    mbObjMotionShiftSet(objId, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbWinCreate(2, 0x003F0014, 13); /* Koopa scene message resource */
    mbWinTopWait();
    mbMusFadeOutSpeed(0, 1000);
    while (mbMusCheck(0)) {
        HuPrcVSleep();
    }
    mbAudFXDelaySet(30);
    mbAudFXPlay(0x3CE); /* event sound-effect resource */
    mbObjMotionShiftSet(objId, 2, 0.0f, 0.0f, HU3D_MOTATTR_NONE);
    while (mbObjMotionShiftIDGet(objId) != -1) {
        HuPrcVSleep();
    }
    while (mbObjMotionTimeGet(objId) <= 25.0f) {
        HuPrcVSleep();
    }
    mbev_CapObjPosSet(&work->objWork, objId, -1, NULL);
    for (i = 1; i <= 24; i++) {
        time = (float)i / 24.0f;
        mbMasuPosGet(masuId, &masuPos);
        masuPos.y += (float)sin((M_PI * 90.0 * time) / 180.0)
            * 100.0f * 6.0f;
        mbObjPosSetV(objId, &masuPos);
        HuPrcVSleep();
    }
    mbObjDispSet(objId, FALSE);
    mbMusBoardPlay();
    mbev_CapPlayerStunSet(ids, count, FALSE);
    HuPrcSleep(60);
    for (i = 0; i < count; i++) {
        mbPlayerMotionShiftSet(ids[i], 1, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
    }
    if (work->flags._flag05) {
        mbCoinAddProcExec(playerNo, 10, 1, FALSE);
    }
}

static u16 ev_CapKoopaDicePadBtnHook(void)
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
