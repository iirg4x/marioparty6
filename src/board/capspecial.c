/* Board capsule events with special character scenes and player effects. */
#define _MATH_H
#include "dolphin/math.h"
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

/* Returns the magnitude used by Koopa's sine-driven dice pulse. */
static inline float CapSpecialAbsFloat(register float value)
{
    asm {
        fabs value, value
    }
    return value;
}

#define CAPSPECIAL_MASU_ATTR_TERESA_LINK (1 << 13)
#define CAPSPECIAL_CAPSULE_LIGHT 31
#define CAPSPECIAL_FADE_OBJ_PRIORITY (-32768)
#define CAPSPECIAL_DICE_TABLE_END 255
#define CAPSPECIAL_TERESA_COIN_EFFECT_ARG 65535
#define CAPSPECIAL_MIRACLE_VIBRATION_FRAMES 300
#define CAPSPECIAL_MIRACLE_SPR_PULSE 32
#define CAPSPECIAL_MIRACLE_SPR_HIDE 64
#define CAPSPECIAL_DONKEY_RETURN_ANGLE 135
#define CAPSPECIAL_DONKEY_RETURN_MOTION 11
#define CAPSPECIAL_DONKEY_MOTION_SHIFT_FRAME 27

#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET0_MODEL \
    DATANUM(0x110000, 1)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET0_MOTION1 \
    DATANUM(0x110000, 4)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET0_MOTION2 \
    DATANUM(0x110000, 5)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET0_MOTION3 \
    DATANUM(0x110000, 21)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET0_MOTION4 \
    DATANUM(0x110000, 7)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET0_MOTION5 \
    DATANUM(0x110000, 11)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET0_MOTION6 \
    DATANUM(0x110000, 12)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET0_MOTION7 \
    DATANUM(0x110000, 13)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET0_MOTION8 \
    DATANUM(0x110000, 19)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET1_MODEL \
    DATANUM(0x110000, 28)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET1_MOTION1 \
    DATANUM(0x110000, 31)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET1_MOTION2 \
    DATANUM(0x110000, 32)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET1_MOTION3 \
    DATANUM(0x110000, 47)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET1_MOTION4 \
    DATANUM(0x110000, 34)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET1_MOTION5 \
    DATANUM(0x110000, 37)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET1_MOTION6 \
    DATANUM(0x110000, 38)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET1_MOTION7 \
    DATANUM(0x110000, 39)
#define CAPSPECIAL_DATA_MIRACLE_GUIDE_SET1_MOTION8 \
    DATANUM(0x110000, 45)
#define CAPSPECIAL_DATA_MIRACLE_TRADE_BACK \
    DATANUM(0x110000, 58)
#define CAPSPECIAL_DATA_MIRACLE_TRADE0 DATANUM(0x110000, 59)
#define CAPSPECIAL_DATA_MIRACLE_TRADE1 DATANUM(0x110000, 60)
#define CAPSPECIAL_DATA_MIRACLE_TRADE2 DATANUM(0x110000, 61)
#define CAPSPECIAL_DATA_MIRACLE_TRADE3 DATANUM(0x110000, 62)
#define CAPSPECIAL_DATA_MIRACLE_TRADE4 DATANUM(0x110000, 63)
#define CAPSPECIAL_DATA_MIRACLE_TRADE5 DATANUM(0x110000, 64)

#define CAPSPECIAL_DATA_TERESA_MODEL DATANUM(0x130000, 8)
#define CAPSPECIAL_DATA_TERESA_MOTION_IDLE DATANUM(0x130000, 9)
#define CAPSPECIAL_DATA_TERESA_MOTION_ITEM DATANUM(0x130000, 10)
#define CAPSPECIAL_DATA_TERESA_MOTION_STEAL DATANUM(0x130000, 11)
#define CAPSPECIAL_DATA_TERESA_STOLEN_CAPSULE DATANUM(0xc0000, 73)

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
typedef int (*TERESA_STEAL_BEGIN_HOOK)(int, int);

#define CAP_WORK_MAX 64

typedef struct EvCapWork {
    int motId[CAP_WORK_MAX][4]; /* Motion resource IDs by object and player. */
    int objId[CAP_WORK_MAX]; /* Board object IDs created for this event. */
    int sprId[CAP_WORK_MAX]; /* Sprite IDs created for this event. */
    void *mem[CAP_WORK_MAX]; /* Allocations retained while event objects use them. */
    int masuId[CAP_WORK_MAX]; /* Space associated with each event object. */
    HuVecF objPos[CAP_WORK_MAX]; /* Current placement for each event object. */
    int playerMasuId[4]; /* Saved board space for each player. */
    HuVecF playerPos[4]; /* Saved position for each player. */
    int bgId; /* Background object ID owned by this event. */
    OMOBJ *obj; /* Event object that owns the work record. */
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
    int playerNo; /* Player who triggered the capsule. */
    int targetPlayerNo; /* Selected player affected by the event, when applicable. */
    int capsuleNo; /* Capsule being run. */
    int masuId; /* Space where the capsule event started. */
    int masuIdNext; /* Next space tracked by capsule movement. */
    int _unk14; /* Shared work slot whose meaning is not established in this file. */
    int _unk18; /* Shared work slot whose meaning is not established in this file. */
    int _unk1C; /* Shared work slot whose meaning is not established in this file. */
    EVCAPWORK objWork;
    CAPWORKFLAG flags;
    int eventData[6]; /* Per-event object IDs, counts, or callback results. */
    u8 _unkB84[40]; /* Shared work bytes whose use is not established in this file. */
    OMOBJ *guideObj; /* Guide character object used by capsule scenes. */
    u8 _unkBB0[28]; /* Shared work bytes whose use is not established in this file. */
    int processNo; /* Process ID for the running capsule event. */
    OMOBJ *explodeObj; /* Explosion effect object, when the event uses one. */
    OMOBJ *boostObj; /* Boost effect object, when the event uses one. */
    OMOBJ *snowObj; /* Snow effect object, when the event uses one. */
    OMOBJ *glowObj; /* Glow effect object, when the event uses one. */
    OMOBJ *ringObj; /* Ring effect object, when the event uses one. */
    OMOBJ *coinObj; /* Coin effect object, when the event uses one. */
    OMOBJ *coinManObj; /* Coin character object, when the event uses one. */
    OMOBJ *starManObj; /* Star character object, when the event uses one. */
    OMOBJ *capLoseObj; /* Capsule-loss object, when the event uses one. */
} CAPWORK;

typedef struct TeresaFadeWork_s {
    void *textureData; /* Captured screen image used by the Teresa fade. */
    u32 textureSize; /* Captured image size in bytes. */
    BOOL activeF; /* Whether the fade draw hook is enabled. */
    float alpha; /* Overlay opacity. */
    BOOL copyF; /* Whether the current frame should be copied. */
    OMOBJ *object; /* Teresa object whose material is faded. */
    u32 screenWidth; /* Render target width in pixels. */
    u32 screenHeight; /* Render target height in pixels. */
    u32 textureWidth; /* Captured texture width in pixels. */
    u32 textureHeight; /* Captured texture height in pixels. */
} TERESA_FADE_WORK;

typedef struct MiracleSprWork_s {
    BOOL activeF; /* Whether the trade-selection sprites are active. */
    int sprId; /* Current foreground sprite. */
    int backSprId; /* Background sprite behind the trade choices. */
    int sprIdTbl[6]; /* Sprite IDs for the available trade choices. */
    int focusTime; /* Frames spent animating the current focus. */
    int focusNo; /* Focused trade choice index. */
    BOOL hideF; /* Whether trade sprites are hidden. */
    float resetFloatA; /* Set to 0.0 during trade-sprite setup; runtime purpose is unknown here. */
    float resetFloatB; /* Set to 0.0 during trade-sprite setup; runtime purpose is unknown here. */
    HuVecF pos; /* Screen position used to place the trade sprites. */
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
static GXColor miracleMasuEffColorTbl[2][3] = {
    {
        { 255, 127, 127, 255 },
        { 255, 255, 127, 255 },
        { 255, 190, 127, 255 },
    },
    {
        { 127, 127, 255, 255 },
        { 127, 255, 255, 255 },
        { 127, 190, 255, 255 },
    },
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
    { DATANUM(0x110000, 1), DATANUM(0x110000, 4),
        DATANUM(0x110000, 17), DATANUM(0x110000, 5),
        DATANUM(0x110000, 18), DATANUM(0x110000, 19),
        DATANUM(0x110000, 20), -1 },
    { DATANUM(0x110000, 28), DATANUM(0x110000, 31),
        DATANUM(0x110000, 43), DATANUM(0x110000, 32),
        DATANUM(0x110000, 44), DATANUM(0x110000, 45),
        DATANUM(0x110000, 46), -1 },
};
static u32 kettouPlayerMotTbl[4] = {
    DATANUM(0x8e0000, 39), /* duel character motion resource pair */
    DATANUM(0x930000, 31), /* duel character motion resource pair */
    DATANUM(0x8e0000, 120), /* duel character motion resource pair */
    DATANUM(0x930000, 75), /* duel character motion resource pair */
};
static u32 donkeyMotTbl[12] = {
    DATANUM(0xe0000, 15), /* Donkey model motion resource */
    DATANUM(0xe0000, 17), /* Donkey model motion resource */
    DATANUM(0xe0000, 18), /* Donkey model motion resource */
    DATANUM(0xe0000, 21), /* Donkey model motion resource */
    DATANUM(0xe0000, 22), /* Donkey model motion resource */
    DATANUM(0xe0000, 19), /* Donkey model motion resource */
    DATANUM(0xe0000, 20), /* Donkey model motion resource */
    DATANUM(0xe0000, 24), /* Donkey model motion resource */
    DATANUM(0xe0000, 25), /* Donkey model motion resource */
    DATANUM(0xe0000, 26), /* Donkey model motion resource */
    DATANUM(0xe0000, 27), /* Donkey model motion resource */
    -1, /* all-bits-set motion table terminator */
};
static int donkeyDiceResultTbl[5] = { 5, 10, 20, 30, -1 };
static int donkeyRouletteBankTbl[10] = { 0, 1, 0, 1, 0, 1, 0, 1, 0, 2 };
static char capTreeFook[12] = "tree_fook";
static u32 koopaMotTbl[7] = {
    DATANUM(0xe0000, 1), /* Koopa model motion resource */
    DATANUM(0xe0000, 5), /* Koopa model motion resource */
    DATANUM(0xe0000, 3), /* Koopa model motion resource */
    DATANUM(0xe0000, 7), /* Koopa model motion resource */
    DATANUM(0xe0000, 4), /* Koopa model motion resource */
    DATANUM(0xe0000, 10), /* Koopa model motion resource */
    -1, /* all-bits-set motion table terminator */
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
static char capspecialMesFormat[3] = "%d";
static char capspecialMotionNode[5] = "head";
static char capspecialTargetNode[8] = "target";
static int miracleBackFile = CAPSPECIAL_DATA_MIRACLE_TRADE_BACK;
/* minigame archive resource identifier pair */
static u32 donkeyMgFile[2] = { DATANUM(0x50000, 140), DATANUM(0x50000, 140) };
/* dice value table terminator sentinel */
static u8 donkeyDiceTbl[8] = { 0, 1, 2, 3, 4, CAPSPECIAL_DICE_TABLE_END, 0, 0 };
/* minigame archive resource identifier pair */
static u32 koopaMgFile[2] = { DATANUM(0x50000, 141), DATANUM(0x50000, 141) };
/* dice value table terminator sentinel */
static u8 koopaDiceTbl[8] = { 0, 1, 2, 3, 4, CAPSPECIAL_DICE_TABLE_END, 0, 0 };
static int kettouMotId[12];
typedef struct {
    s16 playerNo1;
    s16 playerNo2;
    s16 coinNum;
    s16 starNum;
    s16 resultNo;
} CAP_MG_RESULT;

static CAP_MG_RESULT mgResultData;
static TERESA_FADE_WORK *teresaFadeWork;
static int teresaStealCoinNum;
static TERESA_STEAL_BEGIN_HOOK teresaStealBeginHook;
static TERESA_STEAL_HOOK teresaStealHook;
static OMOBJ *miracleSprObj;
static int diceHitTimer;

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
extern MBMODELID mbGuideModelGet(OMOBJ *obj);
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
extern void mbStatusMoveTo(int playerNo, HuVecF *posBegin, HuVecF *posEnd);
extern void mbStatusPosOnGet(int statusNo, HuVecF *pos);
extern void mbStatusPosOffGet(int statusNo, HuVecF *pos);
extern void mbStatusDispSet(int playerNo, BOOL dispF);
extern void mbStatusDispForceSet(int playerNo, BOOL dispF);
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
static void ev_CapMiracleWindowFadeOut(int oldModel, int newModel,
    int timeMax, BOOL reverseF);
static void ev_CapMiracleWindowFadeIn(int oldModel, int newModel,
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
static int ev_CapKoopaReturn(CAPWORK *work);
static u16 ev_CapKoopaDicePadBtnHook(void);
static void ev_CapKoopaDiceMotHook(int playerNo);
static int ev_CapDonkeyStart(CAPWORK *work);
static void ev_CapDonkeyCoin(CAPWORK *work);
static void ev_CapDonkeyReturn(CAPWORK *work);
static void ev_CapDonkeyOMExec(OMOBJ *obj);
extern void mbev_CapEffDustCloudAdd(OMOBJ *obj, HuVecF *pos);
void mbev_CapTeresaFadeCreate(int objectId);
void mbev_CapTeresaFadeKill(int objectId);
void mbev_CapTeresaFadeSet(float alpha);

/* Runs the Teresa capsule event from the capsule callback table. It presents
 * Teresa's choices, then charges the player and performs the selected steal. */
void mbev_CapTeresa(void)
{
    CAPWORK *work = fn_8000DD9C()->property;
    HuVecF playerPos;
    HuVecF targetStartPos;
    HuVecF objectPos;
    HuVecF direction;
    HuVecF cameraRot;
    HuVecF lightAim;
    HuVecF coinPos;
    HuVecF coinVel;
    Mtx hookMtx;
    GXColor lightColors[8];
    HU3D_LIGHT *lightP;
    int motionFiles[16];
    int targetPlayers[4];
    int playerMotions[3];
    int enabledPlayers[4];
    char customMes[16];
    int targetNum;
    int currentMasu;
    int linkMasu;
    int turnCoinMax;
    int coinNum;
    int pressNum;
    int coinEffect;
    int capsuleIndex;
    int enabledNum;
    int starEnabledNum;
    int helpWin;
    int choice;
    int playerNo;
    int targetMasu;
    int objectId;
    int targetPlayer;
    int coinTargetNum;
    int starTargetNum;
    int stealType;
    int itemObjectId;
    int starObjectId;
    int alpha;
    int lightId;
    int i;
    int j;
    float weight;
    float stealRate;
    float launchAngle;
    float launchElevation;
    BOOL cancelF;
    BOOL musicChanged = FALSE;

    mbev_CapWait(work);
    if (!lbl_80265B70.curTime) {
        mbPlayerMotionShiftSet(work->playerNo, 1, 0.0f, 8.0f, ((1 << 30) | (1 << 0)));
        mbWinCreate(2, CAPSPECIAL_MESS_TERESA_UNAVAILABLE, 10);
        mbWinTopWait();
        HuPrcEnd();
        return;
    }

    work->glowObj = mbev_CapEffGlowCreate();
    work->coinObj = mbev_CapEffCoinCreate();
    mbev_CapEffCoinGlowSet(work->coinObj, work->glowObj);
    playerNo = work->playerNo;
    currentMasu = lbl_80265750[playerNo].masuId;
    mbPlayerPosGet(playerNo, &playerPos);
    linkMasu = mbMasuAttrFindLink(currentMasu,
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
    {
        motionFiles[0] = CAPSPECIAL_DATA_TERESA_MOTION_IDLE;
        motionFiles[1] = CAPSPECIAL_DATA_TERESA_MOTION_STEAL;
        motionFiles[2] = CAPSPECIAL_DATA_TERESA_MOTION_ITEM;
        motionFiles[3] = -1;
        objectId = mbev_CapObjCreate(&work->objWork,
            CAPSPECIAL_DATA_TERESA_MODEL, motionFiles,
            FALSE, 5, FALSE);
    }
    mbObjMotionSet(objectId, 1, ((1 << 30) | (1 << 0)));
    mbObjLayerSet(objectId, 4);
    mbObjScaleSet(objectId, 2.0f, 2.0f, 2.0f);
    mbObjPosSetV(objectId, &objectPos);
    mbObjRotSet(objectId, 0.0f,
        (float)(180.0 + (180.0 * (atan2(direction.x, direction.z) / 3.141592653589793))),
        0.0f);
    mbev_CapTeresaFadeCreate(objectId);
    mbPlayerRotSet(playerNo, 0.0f,
        (float)(180.0 * (atan2(direction.x, direction.z) / 3.141592653589793)), 0.0f);
    for (i = 0; i < 4; i++) {
        if (i != playerNo) {
            mbPlayerDispSet(i, FALSE);
        }
    }
    cameraRot = teresaCameraRot;
    cameraRot.y = (float)(180.0
        + (180.0 * (atan2(direction.x, direction.z) / 3.141592653589793)));
    mbCameraMovePlayer(playerNo, &cameraRot, &teresaCameraOfs, 1500.0f,
        -1.0f, -1);
    mbCameraMoveWait();
    mbMusBoardFadeOut(0, 0, 1000, 1000, 32, FALSE);
    mbWipeDissolveFadeIn();

    if (!lbl_80265B70.curTime) {
        mbAudFXPlay(CAPSPECIAL_SE_TERESA_MESSAGE);
        mbWinCreate(2, CAPSPECIAL_MESS_TERESA_UNAVAILABLE, 10);
        mbWinTopWait();
    } else if (mbPlayerCoinGet(playerNo) < 5) {
        mbAudFXPlay(CAPSPECIAL_SE_TERESA_MESSAGE);
        mbWinCreate(2, CAPSPECIAL_MESS_TERESA_INSUFFICIENT_COIN, 10);
        mbWinTopWait();
    } else {
        coinTargetNum = starTargetNum = 0;
        if (mbPlayerCoinGet(playerNo) >= 5) {
            for (i = 0; i < 4; i++) {
                if (!mbev_CapPlayerCheck(i, playerNo)
                    && mbPlayerCoinGet(i) > 0) {
                    coinTargetNum++;
                }
            }
        }
        if (mbPlayerCoinGet(playerNo) >= 40) {
            for (i = 0; i < 4; i++) {
                if (!mbev_CapPlayerCheck(i, playerNo)
                    && mbPlayerStarGet(i) > 0) {
                    starTargetNum++;
                }
            }
        }
for (i = 0, targetNum = 0; i < 4; i++) {
            if (i != playerNo) {
                targetPlayers[targetNum] = i;
                targetNum++;
            }
        }
for (i = 0, starEnabledNum = 0; i < 4; i++) {
            if (!mbev_CapPlayerCheck(i, playerNo)
                && mbPlayerStarGet(i) > 0) {
                starEnabledNum++;
            }
        }

        if (coinTargetNum <= 0 && starTargetNum <= 0
            && mbPlayerCoinGet(playerNo) < 40 && starEnabledNum >= 1) {
            mbAudFXPlay(CAPSPECIAL_SE_TERESA_MESSAGE);
            mbWinCreate(2, CAPSPECIAL_MESS_TERESA_INSUFFICIENT_COIN, 10);
            mbWinTopWait();
        } else if (coinTargetNum <= 0 && starTargetNum <= 0) {
            mbAudFXPlay(CAPSPECIAL_SE_TERESA_MESSAGE);
            mbWinCreate(2, CAPSPECIAL_MESS_TERESA_NO_TARGET, 10);
            mbWinTopWait();
        } else {
            if (teresaStealMesId == -1) {
                mbAudFXPlay(CAPSPECIAL_SE_TERESA_MESSAGE);
                mbWinCreate(2, CAPSPECIAL_MESS_TERESA_INTRO, 10);
                mbWinTopWait();
            } else {
                mbAudFXPlay(CAPSPECIAL_SE_TERESA_MESSAGE);
                mbWinCreate(2, CAPSPECIAL_MESS_TERESA_CUSTOM_INTRO, 10);
                mbWinTopWait();
            }

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
                    if (lbl_80265750[playerNo].comF) {
                        if (coinTargetNum != 0 && starTargetNum != 0) {
                            mbComChoiceListDownSet(1);
                        } else {
                            mbComChoiceListDownSet(0);
                        }
                    }
                    mbWinTopWait();
                    stealType = mbWinTopChoiceGet();
                    if (stealType == 2 || stealType == -1) {
                        mbWinCreate(2, CAPSPECIAL_MESS_TERESA_CANCEL, 10);
                        mbWinTopWait();
                        targetPlayer = -1;
                        goto cleanup;
                    }
                } else {
                    mbWinCreateChoice(1, CAPSPECIAL_MESS_TERESA_CUSTOM_CHOICE,
                        10, 0);
                    sprintf(customMes, capspecialMesFormat, teresaStealCoinNum);
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
                    if (lbl_80265750[playerNo].comF) {
                        if (coinTargetNum != 0 && starTargetNum != 0) {
                            mbComChoiceListDownSet(1);
                        } else {
                            mbComChoiceListDownSet(0);
                        }
                    }
                    mbWinTopWait();
                    stealType = mbWinTopChoiceGet();
                    if (stealType == 3 || stealType == -1) {
                        mbWinCreate(2, CAPSPECIAL_MESS_TERESA_CANCEL, 10);
                        mbWinTopWait();
                        targetPlayer = -1;
                        goto cleanup;
                    }
                    if (stealType == 2) {
                        break;
                    }
                }

                {
                mbWinCreateChoice(1, CAPSPECIAL_MESS_TERESA_TARGET_CHOICE, 10,
                    0);
                mbWinTopInsertMesSet(mbPlayerNameMesGet(targetPlayers[0]), 0);
                mbWinTopInsertMesSet(mbPlayerNameMesGet(targetPlayers[1]), 1);
                mbWinTopInsertMesSet(mbPlayerNameMesGet(targetPlayers[2]), 2);
                for (i = 0; i < 4 - 1; i++) {
                    enabledPlayers[i] = targetPlayers[i];
                }
                if (stealType == 0) {
                    for (i = 0; i < 4 - 1; i++) {
                        if (mbPlayerCoinGet(targetPlayers[i]) <= 0) {
                            mbWinTopChoiceDisable(i);
                            enabledPlayers[i] = -1;
                        }
                    }
                } else {
                    for (i = 0; i < 4 - 1; i++) {
                        if (mbPlayerStarGet(targetPlayers[i]) <= 0) {
                            mbWinTopChoiceDisable(i);
                            enabledPlayers[i] = -1;
                        }
                    }
                }
                for (i = 0; i < 4 - 1; i++) {
                    if (GWTeamFGet()
                        && mbev_CapPlayerCheck(playerNo, targetPlayers[i])) {
                        mbWinTopChoiceDisable(i);
                        enabledPlayers[i] = -1;
                    }
                }
                {
                    i = 0;
                    enabledNum = 0;
                    for (; i < 4 - 1; i++) {
                        if (enabledPlayers[i] >= 0) {
                            enabledNum++;
                        }
                    }
                }
                if (lbl_80265750[playerNo].comF) {
                    if (stealType == 0) {
                        j = mbev_CapPlayerComSelKettouGet(playerNo,
                            0, enabledPlayers, 4 - 1);
                        mbComChoiceListDownSet(j);
                    } else {
                        j = mbev_CapPlayerComSelKettouGet(playerNo,
                            1, enabledPlayers, 4 - 1);
                        mbComChoiceListDownSet(j);
                    }
                }
                mbWinTopWait();
                choice = mbWinTopChoiceGet();
                cancelF = FALSE;
                if (choice == -1) {
                    cancelF = TRUE;
                } else if (choice < 4 - 1) {
                    targetPlayer = targetPlayers[choice];
                } else {
                    j = mbRandMod(4 - 1);
                    targetPlayer = -1;
                    for (i = 0; i < 4 - 1; i++) {
                        if (enabledPlayers[j] >= 0) {
                            targetPlayer = enabledPlayers[j];
                            break;
                        }
                        if (++j >= 4 - 1) {
                            j = 0;
                        }
                    }
                }
                if (cancelF) {
                    continue;
                }
                break;
                }
            }

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

                for (i = 1; (float)i <= 60.0f; i++) {
                    weight = (float)i / 60.0f;
                    mbev_CapTeresaFadeSet(255.0f * (1.0f - weight));
                    HuPrcVSleep();
                }
                mbev_CapTeresaFadeSet(0.0f);

                if (teresaStealMesId != -1 && stealType == 2) {
                    if (teresaStealHook != NULL) {
                        teresaStealHook(TRUE);
                    } else {
                        mbWipeDissolveFadeOutTime(1);
                    }
                    teresaStealBeginHook(playerNo, objectId);
                } else {
                    mbWipeDissolveFadeOutTime(1);
                    for (i = 0, capsuleIndex = -1;
                        i < mbPlayerCapsuleMaxGet(); i++) {
                        if (mbPlayerCapsuleGet(targetPlayer, i)
                            == CAPSPECIAL_CAPSULE_LIGHT) {
                            capsuleIndex = i;
                            break;
                        }
                    }
                    targetMasu = lbl_80265750[targetPlayer].masuId;
                    mbPlayerPosGet(targetPlayer, &targetStartPos);
                    mbev_PlayerColMasu(targetPlayer,
                        targetMasu, TRUE);
                    for (i = 0; i < 4; i++) {
                        mbPlayerDispSet(i, TRUE);
                        if (i != targetPlayer) {
                            mbPlayerDispSet(i, FALSE);
                        }
                    }
                    mbCameraPlayerViewSetFast(targetPlayer, 0);
                    mbCameraMoveWait();
                    mbMasuPosGet(targetMasu, &cameraRot);
                    cameraRot.y += 300.0f;
                    if (capsuleIndex != -1) {
                        cameraRot.y -= 150.0f;
                        cameraRot.z -= 200.0f;
                    }
                    mbObjPosSetV(objectId, &cameraRot);
                    mbObjRotSet(objectId, 0.0f, 0.0f, 0.0f);
                    mbObjDispSet(objectId, FALSE);

                    playerMotions[0] = mbev_CapPlayerMotionCreate(&work->objWork,
                        targetPlayer, CHARMOT_HSF_c000m1_323);
                    playerMotions[1] = mbev_CapPlayerMotionCreate(&work->objWork,
                        targetPlayer, CHARMOT_HSF_c000m1_344);
                    playerMotions[2] = mbev_CapPlayerMotionCreate(&work->objWork,
                        targetPlayer, CHARMOT_HSF_c000m1_457);
                    mbPlayerMotionShiftSet(targetPlayer, playerMotions[0], 0.0f,
                        8.0f, ((1 << 30) | (1 << 0)));
                    itemObjectId = mbev_CapObjCreate(&work->objWork,
                        CAPSPECIAL_DATA_TERESA_STOLEN_CAPSULE, NULL, FALSE, 0,
                        FALSE);
                    mbObjHookSet(mbPlayerObjIDGet(targetPlayer),
                        fn_8005C0F0(
                            lbl_80265750[targetPlayer].charNo, 4, 0),
                        itemObjectId);
                    mbObjDispSet(itemObjectId, FALSE);

                    lightP = Hu3DGlobalLight;
                    for (i = 0; i < 8; i++, lightP++) {
                        if (lightP->type != -1) {
                            lightColors[i] = lightP->color;
                            lightP->color.r *= 0.5f;
                            lightP->color.g *= 0.5f;
                            lightP->color.b *= 0.5f;
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
                        for (i = 1; (float)i <= 60.0f; i++) {
                            weight = (float)i / 60.0f;
                            mbev_CapTeresaFadeSet(255.0f * weight);
                            HuPrcVSleep();
                        }
                        mbev_CapTeresaFadeSet(255.0f);
                        mbPlayerRotateStart(targetPlayer, 180, 15);
                        while (!mbPlayerRotateCheck(targetPlayer)) {
                            HuPrcVSleep();
                        }
                        fn_8006A030(targetPlayer, 20, 7, 3);
                        mbPlayerMotionShiftSet(targetPlayer, playerMotions[2],
                            0.0f, 8.0f, 0);
                        mbObjDispSet(itemObjectId, TRUE);
                        for (i = 1; (float)i < 18.0f; i++) {
                            weight = (float)i / 18.0f;
                            mbObjScaleSet(itemObjectId, weight, weight, weight);
                            HuPrcVSleep();
                        }
                        mbObjScaleSet(itemObjectId, 1.0f, 1.0f, 1.0f);
                        while (!mbPlayerMotionEndCheck(targetPlayer)) {
                            HuPrcVSleep();
                        }
                        lightId = fn_800288B4(
                            mbObjModelIDGet(objectId), &teresaLightPos,
                            &teresaLightDir, &teresaLightColor);
                        fn_80028AA8(mbObjModelIDGet(objectId), lightId,
                            0.001f, GX_SP_SHARP);
                        fn_800296E0(
                            mbObjModelIDGet(objectId), lightId, TRUE);
                        fn_80028B58(
                            mbObjModelIDGet(objectId), lightId);
                        fn_80030644(mbObjModelIDGet(
                            mbPlayerObjIDGet(playerNo)));
                        {
                            fn_80023398(mbObjModelIDGet(
                                    mbPlayerObjIDGet(playerNo)),
                                fn_8005C0F0(
                                    lbl_80265750[playerNo].charNo, 4, 0),
                                hookMtx);
                            cameraRot.x = hookMtx[0][3];
                            cameraRot.y = hookMtx[1][3];
                            cameraRot.z = hookMtx[2][3];
                            lightAim.x = cameraRot.x;
                            lightAim.y = cameraRot.y - 100.0f;
                            lightAim.z = cameraRot.z + 200.0f;
                            fn_80029434(mbObjModelIDGet(objectId),
                                lightId, &cameraRot, &lightAim);
                            fn_80029240(mbObjModelIDGet(objectId),
                                lightId, cameraRot.x, cameraRot.y,
                                cameraRot.z, -45.0f, 0.0f);
                        }
                        mbObjMotionShiftSet(objectId, 3, 0.0f, 8.0f,
                            ((1 << 30) | (1 << 0)));
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
                        for (i = 1; (float)i <= 60.0f; i++) {
                            weight = (float)i / 60.0f;
                            mbev_CapTeresaFadeSet(255.0f * weight);
                            HuPrcVSleep();
                        }
                        mbev_CapTeresaFadeSet(255.0f);
                        mbPlayerMotionShiftSet(targetPlayer, playerMotions[1],
                            0.0f, 8.0f, ((1 << 30) | (1 << 0)));
                        mbObjMotionShiftSet(objectId, 2, 0.0f, 8.0f,
                            ((1 << 30) | (1 << 0)));
                        helpWin = mbWinCreateHelp(
                            CAPSPECIAL_MESS_TERESA_MASH_HELP);
                        mbPlayerColSnapPlayerSet(targetPlayer, FALSE);
                        for (i = 0; (float)i < 30.0f; i++) {
                            weight = (float)i / 30.0f;
                            mbMasuPosGet(targetMasu,
                                &cameraRot);
                            cameraRot.y += 100.0f
                                * (3.0f - (2.0f * weight));
                            alpha = 255.0f - (127.0f * weight);
                            mbObjPosSetV(objectId, &cameraRot);
                            mbev_CapTeresaFadeSet(alpha);
                            mbMasuPosGet(targetMasu,
                                &cameraRot);
                            cameraRot.y += 50.0f * weight;
                            mbPlayerPosSetV(targetPlayer, &cameraRot);
                            HuPrcVSleep();
                        }
                        for (i = 0, pressNum = 0; (float)i < 120.0f; i++) {
                            weight = (float)i / 120.0f;
                            mbMasuPosGet(targetMasu,
                                &cameraRot);
                            cameraRot.y += 100.0
                                + (20.0 * sin((3.141592653589793
                                    * (1440.0f * weight)) / 180.0));
                            mbObjPosSetV(objectId, &cameraRot);
                            mbev_CapTeresaFadeSet(alpha);
                            if (lbl_802C0008[lbl_80265750[targetPlayer].padNo]
                                & PAD_BUTTON_A) {
                                pressNum++;
                                if (alpha > 0) {
                                    alpha--;
                                }
                            }
                            alpha += 2.5 * sin((3.141592653589793
                                * (360.0f * weight)) / 180.0);
                            if (alpha < 64 && (i & 7) == 0) {
                                alpha++;
                            }
                            if (alpha < 32) {
                                alpha = 32;
                            } else if (alpha > 255) {
                                alpha = 255;
                            }
                            mbMasuPosGet(targetMasu,
                                &cameraRot);
                            cameraRot.y += 50.0f;
                            mbPlayerPosSetV(targetPlayer, &cameraRot);
                            HuPrcVSleep();
                        }
                        mbWinKill(helpWin);
                        for (i = 0; (float)i < 30.0f; i++) {
                            weight = 1.0f - ((float)i / 30.0f);
                            mbMasuPosGet(targetMasu,
                                &cameraRot);
                            cameraRot.y += 100.0f
                                + (100.0f * (2.0f - (2.0f * weight)));
                            mbObjPosSetV(objectId, &cameraRot);
                            if (alpha < 255) {
                                alpha += 25;
                            }
                            if (alpha > 255) {
                                alpha = 255;
                            }
                            mbev_CapTeresaFadeSet(alpha);
                            mbMasuPosGet(targetMasu,
                                &cameraRot);
                            cameraRot.y += 50.0f * weight;
                            mbPlayerPosSetV(targetPlayer, &cameraRot);
                            HuPrcVSleep();
                        }
                        mbPlayerMotionShiftSet(targetPlayer, 6, 0.0f,
                            8.0f, ((1 << 30) | (1 << 0)));
                        mbPlayerColSnapPlayerSet(targetPlayer, TRUE);
                        mbObjMotionShiftSet(objectId, 1, 0.0f, 8.0f,
                            ((1 << 30) | (1 << 0)));
                        if (lbl_80265B70.turnNo <= 10) {
                            turnCoinMax = 20;
                        } else if (lbl_80265B70.turnNo <= 20) {
                            turnCoinMax = 25;
                        } else if (lbl_80265B70.turnNo <= 30) {
                            turnCoinMax = 30;
                        } else if (lbl_80265B70.turnNo <= 40) {
                            turnCoinMax = 35;
                        } else {
                            turnCoinMax = 40;
                        }
                        if (!lbl_80265750[targetPlayer].comF) {
                            stealRate = 1.0f - ((float)pressNum / 32.0f);
                        } else {
                            stealRate = 1.0f
                                - (0.1f + (lbl_80265750[targetPlayer].comDif
                                    * (0.2f
                                        + (0.1f * MBCapsuleEffRandF()))));
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
                        fn_8006A030(targetPlayer, 20, 7, 3);
                        mbMasuPosGet(targetMasu,
                            &cameraRot);
                        for (i = 0; i < coinNum; i++) {
                            launchAngle = 360.0f * MBCapsuleEffRandF();
                            coinPos = cameraRot;
                            coinPos.y += 100.0f;
                            launchElevation = 70.0f
                                + (15.0f * MBCapsuleEffRandF());
                            weight = 65.0f
                                * (0.8f + (0.3f * MBCapsuleEffRandF()));
                            coinVel.x = (float)(weight
                                * (sin((3.141592653589793 * launchAngle) / 180.0)
                                * cos((3.141592653589793 * launchElevation) / 180.0)));
                            coinVel.z = (float)(weight
                                * (cos((3.141592653589793 * launchAngle) / 180.0)
                                * cos((3.141592653589793 * launchElevation) / 180.0)));
                            coinVel.y = (float)(weight
                                * sin((3.141592653589793 * launchElevation) / 180.0));
                            coinEffect =
                                mbev_CapEffCoinAdd(work->coinObj, &coinPos, &coinVel, 0.75f, 4.9f,
                                                   30, CAPSPECIAL_TERESA_COIN_EFFECT_ARG);
                            if (coinEffect >= 0) {
                                mbev_CapEffCoinMaxYSet(work->coinObj,
                                    coinEffect, cameraRot.y + 300.0f);
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
                        for (i = 1; (float)i <= 60.0f; i++) {
                            weight = (float)i / 60.0f;
                            mbev_CapTeresaFadeSet(255.0f * weight);
                            HuPrcVSleep();
                        }
                        mbev_CapTeresaFadeSet(255.0f);
                        starObjectId = mbStarObjCreate();
                        mbStarObjDispSet(starObjectId, FALSE);
                        mbPlayerMotionShiftSet(targetPlayer, playerMotions[1],
                            0.0f, 8.0f, ((1 << 30) | (1 << 0)));
                        mbObjMotionShiftSet(objectId, 2, 0.0f, 8.0f,
                            ((1 << 30) | (1 << 0)));
                        mbPlayerColSnapPlayerSet(targetPlayer, FALSE);
                        for (i = 0; (float)i < 30.0f; i++) {
                            weight = (float)i / 30.0f;
                            mbMasuPosGet(targetMasu,
                                &cameraRot);
                            cameraRot.y += 100.0f
                                * (3.0f - (2.0f * weight));
                            alpha = 255.0f - (127.0f * weight);
                            mbObjPosSetV(objectId, &cameraRot);
                            mbev_CapTeresaFadeSet(alpha);
                            mbMasuPosGet(targetMasu,
                                &cameraRot);
                            cameraRot.y += 50.0f * weight;
                            mbPlayerPosSetV(targetPlayer, &cameraRot);
                            HuPrcVSleep();
                        }
                        for (i = 0; (float)i < 30.0f; i++) {
                            weight = 1.0f - ((float)i / 30.0f);
                            mbMasuPosGet(targetMasu,
                                &cameraRot);
                            cameraRot.y += 100.0f
                                + (100.0f * (2.0f - (2.0f * weight)));
                            mbObjPosSetV(objectId, &cameraRot);
                            if (alpha < 255) {
                                alpha += 25;
                            }
                            if (alpha > 255) {
                                alpha = 255;
                            }
                            mbev_CapTeresaFadeSet(alpha);
                            mbMasuPosGet(targetMasu,
                                &cameraRot);
                            cameraRot.y += 50.0f * weight;
                            mbPlayerPosSetV(targetPlayer, &cameraRot);
                            HuPrcVSleep();
                        }
                        mbPlayerMotionShiftSet(targetPlayer, 6, 0.0f,
                            8.0f, ((1 << 30) | (1 << 0)));
                        mbPlayerColSnapPlayerSet(targetPlayer, TRUE);
                        mbPlayerStarAdd(targetPlayer, -1);
                        fn_8006A030(targetPlayer, 20, 20, 0);
                        for (i = 0; (float)i <= 60.0f; i++) {
                            weight = (float)i / 60.0f;
                            weight = (float)sin((3.141592653589793
                                * (90.0f * weight)) / 180.0);
                            mbMasuPosGet(targetMasu,
                                &cameraRot);
                            cameraRot.y += 3.0f * (100.0f * weight);
                            mbStarObjPosSetV(starObjectId, &cameraRot);
                            mbStarObjRotSet(starObjectId, 0.0f,
                                360.0f * weight, 0.0f);
                            mbStarObjScaleSet(
                                starObjectId, weight, weight, weight);
                            mbStarObjDispSet(starObjectId, TRUE);
                            mbMasuPosGet(targetMasu,
                                &cameraRot);
                            cameraRot.y += 300.0f;
                            cameraRot.z -= 2.0f * (100.0f * weight);
                            mbObjPosSetV(objectId, &cameraRot);
                            HuPrcVSleep();
                        }
                        for (i = 0; (float)i <= 60.0f; i++) {
                            weight = (float)i / 60.0f;
                            weight = (float)sin((3.141592653589793
                                * (90.0f * weight)) / 180.0);
                            mbMasuPosGet(targetMasu,
                                &cameraRot);
                            cameraRot.y += 300.0f
                                + (5.0f * (100.0f * weight));
                            mbStarObjPosSetV(starObjectId, &cameraRot);
                            mbStarObjRotSet(starObjectId, 0.0f,
                                720.0f * weight, 0.0f);
                            HuPrcVSleep();
                        }
                        mbStarObjKill(starObjectId);
                    }

                    mbWipeDissolveFadeOutTime(1);
                    mbStarObjDispSetAll(TRUE);
                    lightP = Hu3DGlobalLight;
                    for (i = 0; i < 8; i++, lightP++) {
                        if (lightP->type != -1) {
                            lightP->color = lightColors[i];
                        }
                    }
                    if (lightId != -1) {
                        fn_80028CB8(mbObjModelIDGet(objectId), lightId);
                    }
                    if (capsuleIndex != -1) {
                        mbPlayerCapsuleRemove(targetPlayer, capsuleIndex);
                        lbl_80265750[targetPlayer].capsuleUseNum++;
                    }
                    mbPlayerLayerSet(targetPlayer, 3);
                    mbObjHookObjReset(
                        mbPlayerObjIDGet(targetPlayer),
                        fn_8005C0F0(
                            lbl_80265750[targetPlayer].charNo, 4, 0));
                    mbObjDispSet(itemObjectId, FALSE);
                    mbPlayerPosSetV(targetPlayer, &targetStartPos);
                    mbPlayerRotSet(targetPlayer, 0.0f, 0.0f, 0.0f);
                    mbPlayerMotionSet(targetPlayer, 1, ((1 << 30) | (1 << 0)));
                }

                mbObjMotionShiftSet(objectId, 1, 0.0f, 8.0f,
                    ((1 << 30) | (1 << 0)));
                PSVECSubtract(&objectPos, &playerPos, &direction);
                mbObjPosSetV(objectId, &objectPos);
                mbObjRotSet(objectId, 0.0f,
                    (float)(180.0 + (180.0
                        * (atan2(direction.x, direction.z) / 3.141592653589793))),
                    0.0f);
                mbev_CapTeresaFadeSet(0.0f);
                mbev_PlayerColMasu(
                    playerNo, lbl_80265750[playerNo].masuId, TRUE);
                mbPlayerRotSet(playerNo, 0.0f,
                    (float)(180.0
                        * (atan2(direction.x, direction.z) / 3.141592653589793)),
                    0.0f);
                for (i = 0; i < 4; i++) {
                    mbPlayerDispSet(i, TRUE);
                    if (i != playerNo) {
                        mbPlayerDispSet(i, FALSE);
                    }
                }
                cameraRot = teresaCameraRot;
                cameraRot.y = (float)(180.0 + (180.0
                    * (atan2(direction.x, direction.z) / 3.141592653589793)));
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
                for (i = 1; (float)i <= 60.0f; i++) {
                    weight = (float)i / 60.0f;
                    mbev_CapTeresaFadeSet(255.0f * weight);
                    HuPrcVSleep();
                }
                mbev_CapTeresaFadeSet(255.0f);

                switch (stealType) {
                    case 0:
                        mbAudFXPlay(CAPSPECIAL_SE_TERESA_MESSAGE);
                        mbWinCreate(2, CAPSPECIAL_MESS_TERESA_COIN_RESULT, 10);
                        mbWinTopWait();
                        mbPlayerRotateStart(playerNo,
                            (s16)(180.0 + (180.0
                                * (atan2(direction.x, direction.z) / 3.141592653589793))),
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
                                * (atan2(direction.x, direction.z) / 3.141592653589793))),
                            15);
                        while (!mbPlayerRotateCheck(playerNo)) {
                            HuPrcVSleep();
                        }
                        mbStarGetExec(playerNo);
                        mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f,
                            ((1 << 30) | (1 << 0)));
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
                                * (atan2(direction.x, direction.z) / 3.141592653589793))),
                            15);
                        while (!mbPlayerRotateCheck(playerNo)) {
                            HuPrcVSleep();
                        }
                        mbev_CapPlayerMotShiftWait(playerNo, 13, 0, TRUE);
                        break;
                }
            }
        }

cleanup:
    if (!musicChanged) {
        mbMusBoardFadeOut(0, 0, 1000, 1000, -1, FALSE);
    }
    mbWipeDissolveFadeOutTime(1);
    mbObjDispSet(objectId, FALSE);
    mbev_CapTeresaFadeKill(objectId);
    mbPlayerRotSet(playerNo, 0.0f, 0.0f, 0.0f);
    mbPlayerMotionSet(playerNo, 1, ((1 << 30) | (1 << 0)));
    if (!mbMasuDispCheck(lbl_80265750[playerNo].masuId)
        || lbl_80265750[playerNo].moveNum > 1) {
        mbCameraPlayerViewSetFast(playerNo, 2);
    } else {
        mbCameraPlayerViewSetFast(playerNo, 0);
    }
    mbCameraMoveWait();
    for (i = 0; i < 4; i++) {
        mbPlayerDispSet(i, TRUE);
    }
    fn_800097F4(0x130000);
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

static const float lbl_802C42C4 = 255.0f;

/* Called by the Teresa capsule event to prepare the screen-capture fade overlay. */
void mbev_CapTeresaFadeCreate(int objectId)
{
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
    model = &lbl_802C01C8[modelId];
    hsf = model->hsf;
    material = hsf->material;
    fn_80027884(modelId, ev_CapTeresaFadeMatHook);
    for (i = 0; i < hsf->materialNum; i++, material++) {
        material->flags |= (1 << 13);
    }

    mallocNo = model->mallocNo;
    workData = HuMemDirectMallocNum(
        2, sizeof(TERESA_FADE_WORK), mallocNo);
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
    teresaFadeWork->object = fn_8003AB38(
        mbObjMan, CAPSPECIAL_FADE_OBJ_PRIORITY, 0, 0, -1,
        ev_CapTeresaFadeOMExec);
    teresaFadeWork->textureSize = GXGetTexBufferSize(
        teresaFadeWork->textureWidth, teresaFadeWork->textureHeight,
        GX_TF_RGB565, GX_FALSE, 0);
    textureMallocNo = model->mallocNo;
    textureSize = teresaFadeWork->textureSize;
    textureData = HuMemDirectMallocNum(
        2, textureSize, textureMallocNo);
    texture = textureData;
    teresaFadeWork->textureData = texture;
    memset(teresaFadeWork->textureData, 0, teresaFadeWork->textureSize);
    DCFlushRange(teresaFadeWork->textureData, teresaFadeWork->textureSize);
}

/* Called when the Teresa scene ends to remove its material hook and capture buffer. */
void mbev_CapTeresaFadeKill(int objectId)
{
    fn_80027884(mbObjModelIDGet(objectId), NULL);
    if (teresaFadeWork) {
        HuMemDirectFree(teresaFadeWork->textureData);
        HuMemDirectFree(teresaFadeWork);
        teresaFadeWork = NULL;
    }
}

/* Adds the captured scene image over Teresa while the event fades her in. */
static void ev_CapTeresaFadeMatHook(HU3D_DRAW_OBJ *drawObj,
    HSF_MATERIAL *material)
{
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

    if (!work) {
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
    if (material->attrNum == 1) {
    GXSetNumTexGens(2);
    GXSetNumTevStages(2);
    GXSetTevKAlphaSel(0, 0);
    /* texture-coordinate generation selector for the fade pass */
    GXSetTexCoordGen2(0, 1, 4, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetTevOrder(0, 0, 0, 0);
    /* TEV color-input selectors for sampled fade composition */
    GXSetTevColorIn(0, GX_CC_ZERO, 8, GX_CC_RASC, GX_CC_ZERO);
    GXSetTevColorOp(0, 0, 0, 0, GX_TRUE, 0);
    GXSetTevAlphaIn(0, 7, 7, 7, 6);
    GXSetTevAlphaOp(0, 0, 0, 0, GX_TRUE, 0);

    fn_8001CDF0(drawObj->model, 0, material->hiliteScale);
    camera = &Hu3DCamera[lbl_802C01A6];
    fov = camera->fov;
    if (fov <= 0.0f) {
        fov = 30.0f;
    }
    C_MTXLightPerspective(perspective, fov, lbl_802C4368,
        0.5f, lbl_802C436C, 0.5f, 0.5f);
    PSMTXInverse(lbl_802613F4, cameraInv);
    PSMTXConcat(cameraInv, drawObj->matrix, objectMtx);
    PSMTXConcat(perspective, lbl_802613F4, texMtx);
    PSMTXConcat(texMtx, objectMtx, texMtx);
    GXLoadTexMtxImm(texMtx, GX_TEXMTX1, 0); /* projected texture matrix slot for the fade pass */
    /* texture-coordinate generation selector for the fade pass */
    GXSetTexCoordGen2(1, 0, 0, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);

    color.r = color.g = color.b = 255;
    color.a = (u8)teresaFadeWork->alpha;
    GXSetTevColor(3, color);
    GXSetTevOrder(1, 1, 1, 4);
    GXSetTevKAlphaSel(1, 0);
    GXSetTevColorIn(1, 8, 0, 7, GX_CC_ZERO); /* TEV color-input selectors for sampled fade
                                              * composition */
    GXSetTevColorOp(1, 0, 0, 0, GX_TRUE, 0);
    GXSetTevAlphaIn(1, 7, 7, 7, 6);
    GXSetTevAlphaOp(1, 0, 0, 0, GX_TRUE, 0);

    GXInitTexObj(&texture, work->textureData,
        (u16)work->textureWidth, (u16)work->textureHeight,
        GX_TF_RGB565, 0, 0, GX_FALSE);
    GXInitTexObjLOD(&texture, 1, 1, 0.0f, 0.0f,
        0.0f,
        GX_FALSE, GX_FALSE, 0);
    GXLoadTexObj(&texture, 1);
    } else {
        return;
    }
}

/* Requests a fresh screen capture and removes the fade object on event exit. */
static void ev_CapTeresaFadeOMExec(OMOBJ *obj)
{
    if (mbExitCheck() || !teresaFadeWork) {
        omDelObjEx(mbObjMan, obj);
    } else {
        teresaFadeWork->copyF = FALSE;
    }
}

/* Called by the Teresa scene while fading its captured screen overlay. */
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

const float lbl_802C4368 = 1.2f;
const float lbl_802C436C = -0.5f;
const float lbl_802C4370 = 10.0f;
const float lbl_802C4374 = -20.0f;

/* Runs the Miracle capsule scene from the capsule callback table, moving the
 * player to the start space while the guides present the trade choices. */
void mbev_CapMiracle(void)
{
    CAPWORK *work = fn_8000DD9C()->property;
    HuVecF savedPos[4];
    HuVecF playerPos;
    HuVecF firstCameraRot;
    HuVecF cameraOfs;
    HuVecF masuPos;
    HuVecF guidePos;
    HuVecF cameraRot;
    int playerNo;
    int currentMasu;
    int nextMasu;
    int guideIds[2];
    int guide;
    int i;

    mbev_CapWait(work);
    work->glowObj = mbev_CapEffGlowCreate();
    playerNo = work->playerNo;
    currentMasu = lbl_80265750[playerNo].masuId;
    mbPlayerPosGet(playerNo, &playerPos);
    mbMasuPosGet(currentMasu, &masuPos);
    for (i = 0; i < 4; i++) {
        mbPlayerPosGet(i, &savedPos[i]);
    }
    mbCameraPlayerViewSet(playerNo, 0);
    mbev_CapPlayerRotate(playerNo, 0.0f);
    /* event model resource identifier */
    guideIds[0] = mbev_CapObjCreate(&work->objWork, DATANUM(0x110000, 0),
        (int *)MiracleGuideMotTbl[0], FALSE, 5, FALSE);
    mbObjMotionSet(guideIds[0], 3, 0);
    mbObjDispSet(guideIds[0], FALSE);
    /* event model resource identifier */
    guideIds[1] = mbev_CapObjCreate(&work->objWork, DATANUM(0x110000, 27),
        (int *)MiracleGuideMotTbl[1], FALSE, 5, FALSE);
    mbObjHookSet(guideIds[1], miracleItemHookName,
        /* event model resource identifier */
        mbev_CapObjCreate(&work->objWork, DATANUM(0x110000, 53), NULL,
            FALSE, 5, FALSE));
    HuPrcVSleep();
    mbObjMotionSet(guideIds[1], 3, 0);
    mbObjDispSet(guideIds[1], FALSE);
    work->eventData[0] = guideIds[0];
    work->eventData[1] = guideIds[1];
    ev_CapMiracleMasu(work);
    mbWipeDissolveFadeOutTime(1);
    i = 1;
    while (i < mbMasuNumGet()) {
        if (mbMasuAttrGet(i) & MASU_FLAG_START) {
            break;
        }
        i++;
    }
    if (i < mbMasuNumGet()) {
        nextMasu = i;
    } else {
        nextMasu = 1;
    }
    if (!lbl_80265B70.curTime) {
        guide = guideIds[0];
    } else {
        guide = guideIds[1];
    }
    for (i = 0; i < 2; i++) {
        if (guide != guideIds[i]) {
            mbev_CapObjClose(&work->objWork, guideIds[i]);
        }
    }
    HuPrcSleep(3);
    for (i = 0; i < 4; i++) {
        if (i != playerNo) {
            mbPlayerDispSet(i, FALSE);
        }
    }
    mbMasuPosGet(nextMasu, &masuPos);
    mbPlayerPosSetV(playerNo, &masuPos);
    mbPlayerMotionSet(playerNo, 1, ((1 << 30) | (1 << 0)));
    cameraOfs.x = 200.0f;
    cameraOfs.y = 0.0f;
    cameraOfs.z = 0.0f;
    mbev_CapPlayerPosSet(&work->objWork, playerNo, nextMasu, &cameraOfs);
    guidePos.x = masuPos.x - 200.0f;
    guidePos.y = masuPos.y + 200.0f;
    guidePos.z = 0.0f;
    mbObjPosSetV(guide, &guidePos);
    mbObjRotSet(guide, 0.0f, 0.0f, 0.0f);
    mbObjMotionSet(guide, 1, ((1 << 30) | (1 << 0)));
    mbCameraRotGet(&cameraRot);
    cameraOfs.x = 0.0f;
    cameraOfs.y = 150.0f;
    cameraOfs.z = 0.0f;
    firstCameraRot.x = *((const float *)&lbl_802C4374);
    firstCameraRot.y = firstCameraRot.z = 0.0f;
    mbCameraMoveMasu(nextMasu, &firstCameraRot, &cameraOfs, 1500.0f, -1.0f,
        -1);
    mbCameraMoveWait();
    work->coinManObj = mbev_CapCoinManCreate();
    work->starManObj = mbev_CapStarManCreate();
    mbStatusDispForceSetAll(FALSE);
    work->eventData[0] = guide;
    work->eventData[1] = nextMasu;
    ev_CapMiracleRun(work);
    mbWipeSpecialFadeInCreate(3, 1);
    for (i = 0; i < 2; i++) {
        mbObjDispSet(guideIds[i], FALSE);
    }
    mbMasuPosGet(currentMasu, &masuPos);
    mbPlayerPosSetV(playerNo, &masuPos);
    mbPlayerMotionSet(playerNo, 1, ((1 << 30) | (1 << 0)));
    mbev_CapPlayerPosSet(&work->objWork, playerNo, currentMasu, NULL);
    mbPlayerColSnapPlayerSet(playerNo, TRUE);
    for (i = 0; i < 4; i++) {
        mbPlayerMotionSet(i, 1, ((1 << 30) | (1 << 0)));
        mbPlayerPosSetV(i, &savedPos[i]);
        mbPlayerRotSet(i, 0.0f, 0.0f, 0.0f);
        mbPlayerColSnapPlayerSet(i, TRUE);
        mbPlayerDispSet(i, TRUE);
    }
    cameraOfs.x = 0.0f;
    cameraOfs.y = 100.0f;
    cameraOfs.z = 0.0f;
    mbCameraMoveMasu(currentMasu, &cameraRot, &cameraOfs,
        -1.0f, -1.0f, -1);
    mbCameraMoveWait();
    mbWipeSpecialFadeOutCreate(3, 60);
    HuPrcEnd();
}

void mbev_CapMiracleKill(void)
{
}

/* Animates the guide characters and player around the Miracle capsule space. */
static void ev_CapMiracleMasu(CAPWORK *work)
{
    HuVecF playerPos;
    HuVecF masuPos;
    HuVecF guidePos[2];
    HuVecF guideRot[2];
    HuVecF velTemp;
    HuVecF posTemp;
    GXColor colorTemp;
    GXColor color0;
    GXColor color1;
    GXColor color2;
    GXColor color3;
    GXColor color4;
    GXColor color5;
    int glowNo;
    int playerNo = work->playerNo;
    int masuId = lbl_80265750[playerNo].masuId;
    int guide[2];
    int playerMot[2];
    int sound[4];
    int i;
    int j;
    int k;
    float t;

    mbPlayerPosGet(playerNo, &playerPos);
    mbMasuPosGet(masuId, &masuPos);
    guide[0] = work->eventData[0];
    guide[1] = work->eventData[1];
    playerMot[0] = mbev_CapPlayerMotionCreate(&work->objWork, playerNo,
        DATANUM(0x930000, 23)); /* event resource identifier */
    playerMot[1] = mbev_CapPlayerMotionCreate(&work->objWork, playerNo,
        DATANUM(0x930000, 78)); /* event resource identifier */
    mbCameraMoveWait();
    fn_8006A030(playerNo, CAPSPECIAL_MIRACLE_VIBRATION_FRAMES, 4, 4);
    posTemp.x = 0.0f;
    posTemp.y = 100.0f;
    posTemp.z = 0.0f;
    mbCameraMoveMasu(masuId, NULL, &posTemp, -1.0f, -1.0f, 60);
    mbPlayerMotionShiftSet(playerNo, playerMot[0], 0.0f, 8.0f,
        ((1 << 30) | (1 << 0)));
    sound[0] = mbAudFXPlay(1091); /* event sound-effect resource */
    {
        HuVecF pos;
        HuVecF vel;
        GXColor *colorP;
        HuVecF *velP;
        HuVecF *posP;

        for (i = 0; (float)i < 60.0f; i++) {
            for (j = 0; j < 2; j++) {
                posTemp.x = masuPos.x
                    + 2.0f * (100.0f * (*((const float *)&lbl_802C436C)
                        + MBCapsuleEffRandF()));
                posTemp.y = masuPos.y + 100.0f * MBCapsuleEffRandF();
                posTemp.z = masuPos.z
                    + 2.0f * (100.0f * (*((const float *)&lbl_802C436C)
                        + MBCapsuleEffRandF()));
                velTemp.x = velTemp.y = velTemp.z = 0.0f;
                mbev_CapEffColorSet(&colorTemp, mbRandMod(1 << 15));
                color0 = colorTemp;
                colorP = &color0;
                vel = velTemp;
                velP = &vel;
                pos = posTemp;
                posP = &pos;
                glowNo = mbev_CapEffGlowAdd(work->glowObj, posP, velP,
                    (int)(60.0f * (1.0f + (0.3f * MBCapsuleEffRandF()))),
                    100.0f * (0.15f + (0.05f * MBCapsuleEffRandF())),
                    0.05f + (0.02f * MBCapsuleEffRandF()), -0.08166666f,
                    colorP);
                mbev_CapEffGlowKinokoTimeSet(work->glowObj, glowNo, 1, 90);
            }
            HuPrcVSleep();
        }
    }
    sound[1] = mbAudFXPlay(1092); /* event sound-effect resource */
    for (i = 1; (float)i <= 120.0f; i++) {
        t = (float)i / 120.0f;
        for (j = 0; j < 2; j++) {
            guideRot[j].x = 0.0f;
            guideRot[j].y = 540.0f * t;
            if (j & 1) {
                guideRot[j].y += 270.0f;
            } else {
                guideRot[j].y += 90.0f;
            }
            guideRot[j].z = 0.0f;
            guidePos[j].x = (float)(masuPos.x
                + (1.5 * (100.0f
                    * sin(3.141592653589793 * (180.0f + guideRot[j].y) / 180.0f))));
            guidePos[j].z = (float)(masuPos.z
                + (1.5 * (100.0f
                    * cos(3.141592653589793 * (180.0f + guideRot[j].y) / 180.0f))));
            guidePos[j].y = (float)(masuPos.y + 100.0f
                + (6.0 * (100.0f
                    * cos(3.141592653589793 * (90.0f * t) / 180.0f))));
            mbObjPosSetV(guide[j], &guidePos[j]);
            mbObjRotSetV(guide[j], &guideRot[j]);
            mbObjDispSet(guide[j], TRUE);
            {
                HuVecF pos;
                HuVecF vel;
                    GXColor *colorP;
                HuVecF *velP;
                HuVecF *posP;

                for (k = 0; k < 3; k++) {
                    posTemp.x = guidePos[j].x
                        + 2.0f * (100.0f
                            * (*((const float *)&lbl_802C436C)
                                + MBCapsuleEffRandF()));
                    posTemp.y = guidePos[j].y
                        + 100.0f * MBCapsuleEffRandF();
                    posTemp.z = guidePos[j].z
                        + 2.0f * (100.0f
                            * (*((const float *)&lbl_802C436C)
                                + MBCapsuleEffRandF()));
                    velTemp.x = velTemp.y = velTemp.z = 0.0f;
                    color1 = miracleMasuEffColorTbl[j][mbRandMod(3)];
                    colorP = &color1;
                    vel = velTemp;
                    velP = &vel;
                    pos = posTemp;
                    posP = &pos;
                    glowNo = mbev_CapEffGlowAdd(work->glowObj, posP, velP,
                        (int)(60.0f
                            * (1.0f + (0.3f * MBCapsuleEffRandF()))),
                        100.0f * (0.15f + (0.05f * MBCapsuleEffRandF())),
                        0.05f + (0.02f * MBCapsuleEffRandF()), -0.08166666f,
                        colorP);
                    mbev_CapEffGlowKinokoTimeSet(work->glowObj, glowNo, 2,
                        (int)(60.0f
                            * (0.5f + (0.5f * MBCapsuleEffRandF()))));
                }
            }
        }
        {
            HuVecF pos;
            HuVecF vel;
            GXColor *colorP;
            HuVecF *velP;
            HuVecF *posP;

            for (j = 0; j < 2; j++) {
                posTemp.x = masuPos.x + 2.0f
                    * (100.0f * (*((const float *)&lbl_802C436C)
                        + MBCapsuleEffRandF()));
                posTemp.y = masuPos.y
                    + 100.0f * MBCapsuleEffRandF();
                posTemp.z = masuPos.z
                    + 2.0f * (100.0f * (*((const float *)&lbl_802C436C)
                        + MBCapsuleEffRandF()));
                velTemp.x = velTemp.y = velTemp.z = 0.0f;
                mbev_CapEffColorSet(&colorTemp, mbRandMod(1 << 15));
                color2 = colorTemp;
                colorP = &color2;
                vel = velTemp;
                velP = &vel;
                pos = posTemp;
                posP = &pos;
                glowNo = mbev_CapEffGlowAdd(work->glowObj, posP, velP,
                    (int)(60.0f * (1.0f + (0.3f * MBCapsuleEffRandF()))),
                    100.0f * (0.15f + (0.05f * MBCapsuleEffRandF())),
                    0.05f + (0.02f * MBCapsuleEffRandF()), -0.08166666f,
                    colorP);
                mbev_CapEffGlowKinokoTimeSet(work->glowObj, glowNo, 1, 90);
            }
        }
        HuPrcVSleep();
    }
    if (sound[1] != -1) {
        mbAudFXStop(sound[1]);
    }
    for (j = 0; j < 2; j++) {
        mbObjMotionShiftSet(guide[j], 9, 0.0f, 8.0f,
            0);
    }
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
    mbPlayerMotionShiftSet(playerNo, playerMot[1], 0.0f, 18.0f,
        ((1 << 30) | (1 << 0)));
    {
        HuVecF pos;
        HuVecF vel;
        GXColor *colorP;
        HuVecF *velP;
        HuVecF *posP;

        for (i = 1; (float)i <= 45.0f; i++) {
            t = (float)i / 45.0f;
            playerPos.y = masuPos.y + (100.0f
                * sin(3.141592653589793 * (90.0f * t) / 180.0f));
            mbPlayerPosSetV(playerNo, &playerPos);
            for (j = 0; j < 2; j++) {
                posTemp.x = playerPos.x
                    + 2.0f * (100.0f * (*((const float *)&lbl_802C436C)
                        + MBCapsuleEffRandF()));
                posTemp.y = playerPos.y
                    + 2.0f * (100.0f * (*((const float *)&lbl_802C436C)
                        + MBCapsuleEffRandF()));
                posTemp.z = playerPos.z
                    + 2.0f * (100.0f * (*((const float *)&lbl_802C436C)
                        + MBCapsuleEffRandF()));
                velTemp.x = velTemp.y = velTemp.z = 0.0f;
                mbev_CapEffColorSet(&colorTemp, mbRandMod(1 << 15));
                color3 = colorTemp;
                colorP = &color3;
                vel = velTemp;
                velP = &vel;
                pos = posTemp;
                posP = &pos;
                glowNo = mbev_CapEffGlowAdd(work->glowObj, posP, velP,
                    (int)(60.0f * (1.0f + (0.3f * MBCapsuleEffRandF()))),
                    100.0f * (0.15f + (0.05f * MBCapsuleEffRandF())),
                    0.05f + (0.02f * MBCapsuleEffRandF()), -0.08166666f,
                    colorP);
                mbev_CapEffGlowKinokoTimeSet(work->glowObj, glowNo, 1, 90);
            }
            HuPrcVSleep();
        }
    }
    sound[2] = mbAudFXPlay(1093); /* event sound-effect resource */
    sound[3] = mbAudFXPlay(1022); /* event sound-effect resource */
    for (i = 1; (float)i <= 120.0f; i++) {
        t = (float)i / 120.0f;
        playerPos.y = (float)(masuPos.y + 100.0f
            + (6.0 * (100.0f
                * sin(3.141592653589793 * (90.0f * t) / 180.0f))));
        mbPlayerPosSetV(playerNo, &playerPos);
        mbPlayerRotSet(playerNo, 0.0f,
            720.0 * sin(3.141592653589793 * (90.0f * t) / 180.0f), 0.0f);
        {
            HuVecF pos;
            HuVecF vel;
            GXColor *colorP;
            HuVecF *velP;
            HuVecF *posP;

            for (j = 0; j < 2; j++) {
                guideRot[j].x = 0.0f;
                guideRot[j].y = 720.0 * sin(
                    3.141592653589793 * (90.0f * (t * t)) / 180.0f);
                if (j & 1) {
                    guideRot[j].y += 90.0f;
                } else {
                    guideRot[j].y += 270.0f;
                }
                guideRot[j].z = 0.0f;
                guidePos[j].x = (float)(masuPos.x
                    + (1.5 * (100.0f
                        * sin(3.141592653589793 * (180.0f + guideRot[j].y) / 180.0f))));
                guidePos[j].z = (float)(masuPos.z
                    + (1.5 * (100.0f
                        * cos(3.141592653589793 * (180.0f + guideRot[j].y) / 180.0f))));
                guidePos[j].y = (float)(masuPos.y + 100.0f
                    + (6.0 * (100.0f
                        * sin(3.141592653589793 * (90.0f * (t * t)) / 180.0f))));
                mbObjPosSetV(guide[j], &guidePos[j]);
                mbObjRotSetV(guide[j], &guideRot[j]);
                for (k = 0; k < 3; k++) {
                    posTemp.x = guidePos[j].x + 2.0f * (100.0f
                        * (*((const float *)&lbl_802C436C)
                            + MBCapsuleEffRandF()));
                    posTemp.y = guidePos[j].y
                        + 100.0f * MBCapsuleEffRandF();
                    posTemp.z = guidePos[j].z + 2.0f * (100.0f
                        * (*((const float *)&lbl_802C436C)
                            + MBCapsuleEffRandF()));
                    velTemp.x = velTemp.y = velTemp.z = 0.0f;
                    color4 = miracleMasuEffColorTbl[j][mbRandMod(3)];
                    colorP = &color4;
                    vel = velTemp;
                    velP = &vel;
                    pos = posTemp;
                    posP = &pos;
                    glowNo = mbev_CapEffGlowAdd(work->glowObj, posP, velP,
                        (int)(60.0f
                            * (1.0f + (0.3f * MBCapsuleEffRandF()))),
                        100.0f * (0.15f + (0.05f * MBCapsuleEffRandF())),
                        0.05f + (0.02f * MBCapsuleEffRandF()), -0.08166666f,
                        colorP);
                    mbev_CapEffGlowKinokoTimeSet(work->glowObj, glowNo, 2,
                        (int)(60.0f
                            * (0.5f + (0.5f * MBCapsuleEffRandF()))));
                }
            }
        }
        {
            HuVecF pos;
            HuVecF vel;
            GXColor *colorP;
            HuVecF *velP;
            HuVecF *posP;

            for (j = 0; j < 2; j++) {
                posTemp.x = playerPos.x + 2.0f
                    * (100.0f * (*((const float *)&lbl_802C436C)
                        + MBCapsuleEffRandF()));
                posTemp.y = playerPos.y + 2.0f
                    * (100.0f * (*((const float *)&lbl_802C436C)
                        + MBCapsuleEffRandF()));
                posTemp.z = playerPos.z + 2.0f
                    * (100.0f * (*((const float *)&lbl_802C436C)
                        + MBCapsuleEffRandF()));
                velTemp.x = velTemp.y = velTemp.z = 0.0f;
                mbev_CapEffColorSet(&colorTemp, mbRandMod(1 << 15));
                color5 = colorTemp;
                colorP = &color5;
                vel = velTemp;
                velP = &vel;
                pos = posTemp;
                posP = &pos;
                glowNo = mbev_CapEffGlowAdd(work->glowObj, posP, velP,
                    (int)(60.0f * (1.0f + (0.3f * MBCapsuleEffRandF()))),
                    100.0f * (0.15f + (0.05f * MBCapsuleEffRandF())),
                    0.05f + (0.02f * MBCapsuleEffRandF()), -0.08166666f,
                    colorP);
                mbev_CapEffGlowKinokoTimeSet(work->glowObj, glowNo, 1, 90);
            }
        }
        HuPrcVSleep();
    }
    if (sound[0] != -1) {
        mbAudFXStop(sound[0]);
    }
    if (sound[2] != -1) {
        mbAudFXStop(sound[2]);
    }
    if (sound[3] != -1) {
        mbAudFXStop(sound[3]);
    }
}

/* Runs the Miracle capsule scene after its event entry point selects guide and spaces. */
static void ev_CapMiracleRun(CAPWORK *work)
{
    HuVecF guidePos;
    HuVecF playerPos;
    HuVecF rot;
    HuVecF pos;
    HuVecF masuPos;
    HuVec2f statusOff;
    HuVec2f statusOn;
    Mtx mtx;
    int playerNo;
    int guide = work->eventData[0];
    int rightRow;
    int nextMasu = work->eventData[1];
    int tradeObj;
    int targetObj;
    int leftObj;
    int rightObj;
    int order[4];
    int playerList[32];
    int tradeOrder[32];
    int motTimes[32];
    int coin[2];
    int star[2];
    int oldCoin[2];
    int oldStar[2];
    int players[2];
    int playerNo1;
    int playerNo2;
    int playerCount;
    int diceNo;
    int tradeRow;
    int tradeNo;
    int i;

    mbMasuPosGet(nextMasu, &masuPos);
    playerNo = work->playerNo;
    playerPos.x = masuPos.x + 200.0f;
    playerPos.y = masuPos.y;
    playerPos.z = masuPos.z;
    mbPlayerPosSetV(playerNo, &playerPos);
    guidePos.x = masuPos.x - 200.0f;
    guidePos.y = masuPos.y;
    guidePos.z = masuPos.z;
    mbObjPosSetV(guide, &guidePos);
    fn_8002DA64(mbObjModelIDGet(guide), capspecialMotionNode, (1 << 7), -5.0f);
    fn_8002DA64(mbObjModelIDGet(guide), capspecialMotionNode, (1 << 8), 5.0f);
    pos.x = playerPos.x - 200.0f;
    pos.y = playerPos.y;
    pos.z = playerPos.z - 75.0f;
    mbCameraRotGet(&rot);
    /* event model resource identifier */
    tradeObj = mbev_CapObjCreate(&work->objWork, DATANUM(0x110000, 54), NULL,
        FALSE, 0, FALSE);
    mbObjPosSetV(tradeObj, &pos);
    mbObjRotSetV(tradeObj, &rot);
    mbObjScaleSet(tradeObj, 2.0f, 2.0f, 2.0f);
    fn_80030644(mbObjModelIDGet(tradeObj));
    fn_80023398(mbObjModelIDGet(tradeObj), capspecialTargetNode, mtx);
    pos.x = mtx[0][3];
    pos.y = mtx[1][3];
    pos.z = mtx[2][3];
    /* event model resource identifier */
    leftObj = mbev_CapObjCreate(&work->objWork, DATANUM(0x110000, 56), NULL,
        FALSE, 0, FALSE);
    mbObjMotionTimeSet(leftObj, 0.5f + lbl_80265750[playerNo].charNo);
    mbObjMotionSpeedSet(leftObj, 0.0f);
    mbObjPosSetV(leftObj, &pos);
    mbObjRotSetV(leftObj, &rot);
    mbObjScaleSet(leftObj, 2.0f, 2.0f, 2.0f);
    mbObjDispSet(leftObj, FALSE);
    mbObjLayerSet(leftObj, 3);
    /* event model resource identifier */
    rightObj = mbev_CapObjCreate(&work->objWork, DATANUM(0x110000, 57), NULL,
        FALSE, 0, FALSE);
    mbObjMotionTimeSet(rightObj, 0.5f);
    mbObjMotionSpeedSet(rightObj, 0.0f);
    mbObjPosSetV(rightObj, &pos);
    mbObjRotSetV(rightObj, &rot);
    mbObjScaleSet(rightObj, 2.0f, 2.0f, 2.0f);
    mbObjDispSet(rightObj, FALSE);
    mbObjLayerSet(rightObj, 3);
    /* event model resource identifier */
    targetObj = mbev_CapObjCreate(&work->objWork, DATANUM(0x110000, 55), NULL,
        FALSE, 0, FALSE);
    mbObjAttrSet(targetObj, ((1 << 30) | (1 << 0)));
    mbObjPosSetV(targetObj, &pos);
    mbObjRotSetV(targetObj, &rot);
    mbObjDispSet(targetObj, TRUE);
    mbObjLayerSet(targetObj, 3);
    mbObjScaleSet(targetObj, 2.0f, 2.0f, 2.0f);
    ev_CapMiracleSprCreate();
    mbMusBoardFadeOut(0, 0, 1000, 1000, MSM_STREAM_STORY_END, FALSE);
    mbWipeDissolveFadeIn();
    mbObjMotionShiftSet(guide, 7, 0.0f, 8.0f, ((1 << 30) | (1 << 0)));
    mbAudGuidePlay(950); /* event guide voice resource */
    /* miracle scene message resource */
    mbWinCreate(2, ev_CapMiracleMesGet(MESSNUM(MESS_MIRACLE_MASU, 0)), mbGuideSpeakerNoGet());
    mbWinTopWait();
    mbAudGuidePlay(952); /* event guide voice resource */
    /* miracle scene message resource */
    mbWinCreate(2, ev_CapMiracleMesGet(MESSNUM(MESS_MIRACLE_MASU, 1)), mbGuideSpeakerNoGet());
    mbWinTopWait();
    for (i = 1; i < 5; i++) {
        fn_8002DA64(mbObjModelIDGet(guide), capspecialMotionNode, (1 << 7),
            (float)-(5 - i));
        fn_8002DA64(mbObjModelIDGet(guide), capspecialMotionNode, (1 << 8),
            (float)(5 - i));
        HuPrcVSleep();
    }
    Hu3DMotionNoMotReset(mbObjModelIDGet(guide), capspecialMotionNode, (1 << 7));
    Hu3DMotionNoMotReset(mbObjModelIDGet(guide), capspecialMotionNode, (1 << 8));
    mbAudFXDelaySet(30);
    mbAudFXPlay(1017); /* event sound-effect resource */
    mbev_CapPlayerMotShiftSet(guide, 4, 0, TRUE);
    mbObjMotionShiftSet(guide, 1, 0.0f, 8.0f, ((1 << 30) | (1 << 0)));
    (void)mbev_CapPlayerOrderGet(order, -1, playerNo, TRUE);
    for (i = 0, playerCount = 0; i < 32; i++) {
        if (miracleLeftCharOrderTbl[i] == -1) {
            break;
        }
        playerList[playerCount] = order[miracleLeftCharOrderTbl[i]];
        motTimes[playerCount] = lbl_80265750[playerList[playerCount]].charNo;
        playerCount++;
    }
    ev_CapMiracleWindowFadeIn(targetObj, leftObj, 60, TRUE,
        3, playerCount, motTimes);
    diceNo = ev_CapMiracleDiceExec(playerNo, leftObj, 3,
        playerCount, motTimes);
    playerNo1 = playerList[diceNo];
    mbAudFXPlay(1136); /* event sound-effect resource */
    mbStatusPosOffGet(0, (HuVecF *)&statusOff);
    mbStatusPosOnGet(0, (HuVecF *)&statusOn);
    mbStatusDispForceSet(playerNo1, TRUE);
    mbStatusMoveTo(playerNo1, (HuVecF *)&statusOff, (HuVecF *)&statusOn);
    ev_CapMiracleWindowFadeOut(leftObj, targetObj, 60,
        FALSE);
    HuPrcSleep(60);
    mbObjMotionShiftSet(guide, 7, 0.0f, 8.0f, ((1 << 30) | (1 << 0)));
    mbAudGuidePlay(952); /* event guide voice resource */
    /* miracle scene message resource */
    mbWinCreate(2, ev_CapMiracleMesGet(MESSNUM(MESS_MIRACLE_MASU, 2)), mbGuideSpeakerNoGet());
    mbWinTopWait();
    mbAudFXDelaySet(30);
    mbAudFXPlay(1017); /* event sound-effect resource */
    mbev_CapPlayerMotShiftSet(guide, 4, 0, TRUE);
    mbObjMotionShiftSet(guide, 1, 0.0f, 8.0f, ((1 << 30) | (1 << 0)));
    (void)mbev_CapPlayerOrderGet(order, playerNo1, playerNo, TRUE);
    {
        if (playerNo1 == playerNo) {
            rightRow = 0;
        } else {
            rightRow = 1;
        }

        for (i = 0, playerCount = 0; i < 32; i++) {
            if (miracleRightCharOrderTbl[rightRow][i] == -1) {
                break;
            }
            playerList[playerCount] =
                order[miracleRightCharOrderTbl[rightRow][i]];
            motTimes[playerCount] = lbl_80265750[playerList[playerCount]].charNo;
            playerCount++;
        }
    }
    mbObjMotionTimeSet(leftObj, 0.5f + (float)motTimes[0]);
    mbObjMotionSpeedSet(leftObj, 0.0f);
    ev_CapMiracleWindowFadeIn(targetObj, leftObj, 60, TRUE,
        3, playerCount, motTimes);
    diceNo = ev_CapMiracleDiceExec(playerNo, leftObj, 3,
        playerCount, motTimes);
    playerNo2 = playerList[diceNo];
    mbAudFXPlay(1137); /* event sound-effect resource */
    if ((int)lbl_80265B70.tagF != FALSE
        && mbev_CapPlayerCheck(playerNo1, playerNo2)) {
        HuPrcSleep(60);
        mbObjMotionShiftSet(guide, 6, 0.0f, 8.0f,
            ((1 << 30) | (1 << 0)));
        mbAudGuidePlay(951); /* event guide voice resource */
            /* miracle scene message resource */
        mbWinCreate(2, ev_CapMiracleMesGet(MESSNUM(MESS_MIRACLE_MASU, 5)), mbGuideSpeakerNoGet());
        mbWinTopWait();
        mbPlayerWinLoseVoicePlay(playerNo, 13, (573+12)); /* player loss voice resource */
        mbev_CapPlayerMotShiftWait(playerNo, 13, 0,
            TRUE);
        mbev_CapPlayerMotShiftWait(playerNo, 1, ((1 << 30) | (1 << 0)),
            TRUE);
        mbObjMotionShiftSet(guide, 1, 0.0f, 8.0f,
            ((1 << 30) | (1 << 0)));
        if (mbStatusDispGet(playerNo1)) {
            mbStatusDispSet(playerNo1, FALSE);
        }
        ev_CapMiracleTradeHideSet();
    } else {
        mbStatusPosOffGet(1, (HuVecF *)&statusOff);
        mbStatusPosOnGet(1, (HuVecF *)&statusOn);
        mbStatusDispForceSet(playerNo2, TRUE);
        mbStatusMoveTo(playerNo2, (HuVecF *)&statusOff, (HuVecF *)&statusOn);
        ev_CapMiracleWindowFadeOut(leftObj, targetObj, 60,
            FALSE);
        HuPrcSleep(60);
        mbAudGuidePlay(952); /* event guide voice resource */
        mbObjMotionShiftSet(guide, 7, 0.0f, 8.0f,
            ((1 << 30) | (1 << 0)));
            /* miracle scene message resource */
        mbWinCreate(2, ev_CapMiracleMesGet(MESSNUM(MESS_MIRACLE_MASU, 3)), mbGuideSpeakerNoGet());
        mbWinTopWait();
        mbAudFXDelaySet(30);
        mbAudFXPlay(1017); /* event sound-effect resource */
        mbev_CapPlayerMotShiftSet(guide, 4, 0, TRUE);
        mbObjMotionShiftSet(guide, 1, 0.0f, 8.0f,
            ((1 << 30) | (1 << 0)));
    tradeRow = (lbl_80265B70.turnNo * 3) / lbl_80265B70.turnMax;
    if (tradeRow < 0) {
        tradeRow = 0;
    }
    if (tradeRow > 2) {
        tradeRow = 2;
    }
    for (i = 0, playerCount = 0; i < 32; i++) {
        if (miracleTradeOrderTbl[tradeRow][i] == -1) {
            break;
        }
        tradeOrder[playerCount] = miracleTradeOrderTbl[tradeRow][i];
        playerCount++;
    }
    ev_CapMiracleWindowFadeIn(targetObj, rightObj, 60, TRUE,
        3, playerCount, tradeOrder);
    tradeNo = tradeOrder[diceNo = ev_CapMiracleDiceExec(playerNo,
        rightObj, 3, playerCount, tradeOrder)];
    if (tradeNo < 0 || tradeNo > 5) {
        tradeNo = 0;
        mbObjMotionTimeSet(rightObj, 0.5f + (float)tradeNo);
    }
    ev_CapMiracleTradeCreate(&miracleTradePosTbl[1], tradeNo);
    ev_CapMiracleWindowFadeOut(rightObj, targetObj, 60,
        FALSE);
    HuPrcSleep(60);
    ev_CapMiracleTradeFocusSet();
    mbObjMotionShiftSet(guide, 7, 0.0f, 8.0f,
        ((1 << 30) | (1 << 0)));
    /* miracle scene message resource */
    mbWinCreate(2, ev_CapMiracleMesGet(MESSNUM(MESS_MIRACLE_MASU, 4)), mbGuideSpeakerNoGet());
    mbWinTopWait();
    ev_CapMiracleTradeHideSet();
    HuPrcSleep(60);
    oldCoin[0] = coin[0] = mbPlayerCoinGet(playerNo1);
    oldStar[0] = star[0] = mbPlayerStarGet(playerNo1);
    oldCoin[1] = coin[1] = mbPlayerCoinGet(playerNo2);
    oldStar[1] = star[1] = mbPlayerStarGet(playerNo2);
    mbWipeSpecialFadeInCreate(3, 1);
    mbObjDispSet(targetObj, FALSE);
    mbObjDispSet(tradeObj, FALSE);
    guidePos.x = masuPos.x;
    guidePos.y = masuPos.y;
    guidePos.z = masuPos.z - 150.0f;
    mbObjPosSetV(guide, &guidePos);
    mbObjRotSet(guide, 0.0f, 0.0f, 0.0f);
    mbev_CapPlayerPosSet(&work->objWork, playerNo, -1, NULL);
    ev_CapMiraclePlayerSet(work, playerNo1, playerNo2, nextMasu);
    mbWipeSpecialFadeOutCreate(3, 60);
    switch (tradeNo) {
        case 0:
            mbAudGuidePlay(949); /* event guide voice resource */
                    /* miracle scene message resource */
            mbWinCreate(2, ev_CapMiracleMesGet(MESSNUM(MESS_MIRACLE_MASU, 6)),
                        mbGuideSpeakerNoGet());
            mbWinTopInsertMesSet(mbPlayerNameMesGet(playerNo1), 0);
            /* miracle scene insert resource */
            mbWinTopInsertMesSet(MESSNUM(MESS_MIRACLE_MASU, 11), 1);
            mbWinTopInsertMesSet(mbPlayerNameMesGet(playerNo2), 2);
            mbWinTopWait();
            if (mbPlayerCoinGet(playerNo1) <= 0) {
                mbAudGuidePlay(951); /* event guide voice resource */
                            mbWinCreate(2, ev_CapMiracleMesGet(MESSNUM(MESS_MIRACLE_MASU, 8)),
                    mbGuideSpeakerNoGet()); /* miracle scene message resource */
                mbWinTopInsertMesSet(mbPlayerNameMesGet(playerNo1), 0);
                /* miracle scene insert resource */
                mbWinTopInsertMesSet(MESSNUM(MESS_MIRACLE_MASU, 11), 1);
                mbWinTopWait();
            } else {
                mbObjMotionShiftSet(guide, 1, 0.0f, 8.0f,
                    ((1 << 30) | (1 << 0)));
                ev_CapMiracleCoinTrade(work, playerNo1, playerNo2,
                    20, 0);
            }
            break;
        case 1:
            mbAudGuidePlay(949); /* event guide voice resource */
                    /* miracle scene message resource */
            mbWinCreate(2, ev_CapMiracleMesGet(MESSNUM(MESS_MIRACLE_MASU, 7)),
                        mbGuideSpeakerNoGet());
            mbWinTopInsertMesSet(mbPlayerNameMesGet(playerNo1), 0);
            mbWinTopInsertMesSet(mbPlayerNameMesGet(playerNo2), 1);
            /* miracle scene insert resource */
            mbWinTopInsertMesSet(MESSNUM(MESS_MIRACLE_MASU, 11), 2);
            mbWinTopWait();
            if (mbPlayerCoinGet(playerNo1) <= 0
                && mbPlayerCoinGet(playerNo2) <= 0) {
                mbAudGuidePlay(951); /* event guide voice resource */
                            mbWinCreate(2, ev_CapMiracleMesGet(MESSNUM(MESS_MIRACLE_MASU, 9)),
                    mbGuideSpeakerNoGet()); /* miracle scene message resource */
                /* miracle scene insert resource */
                mbWinTopInsertMesSet(MESSNUM(MESS_MIRACLE_MASU, 11), 0);
                mbWinTopWait();
            } else {
                mbObjMotionShiftSet(guide, 1, 0.0f, 8.0f,
                    ((1 << 30) | (1 << 0)));
                ev_CapMiracleCoinTrade(work, playerNo1, playerNo2,
                    mbPlayerCoinGet(playerNo1),
                    mbPlayerCoinGet(playerNo2));
            }
            break;
        case 2:
            mbAudGuidePlay(949); /* event guide voice resource */
                    /* miracle scene message resource */
            mbWinCreate(2, ev_CapMiracleMesGet(MESSNUM(MESS_MIRACLE_MASU, 6)),
                        mbGuideSpeakerNoGet());
            mbWinTopInsertMesSet(mbPlayerNameMesGet(playerNo1), 0);
            /* miracle scene insert resource */
            mbWinTopInsertMesSet(MESSNUM(MESS_MIRACLE_MASU, 13), 1);
            mbWinTopInsertMesSet(mbPlayerNameMesGet(playerNo2), 2);
            mbWinTopWait();
            if (mbPlayerStarGet(playerNo1) <= 0) {
                mbAudGuidePlay(951); /* event guide voice resource */
                            mbWinCreate(2, ev_CapMiracleMesGet(MESSNUM(MESS_MIRACLE_MASU, 8)),
                    mbGuideSpeakerNoGet()); /* miracle scene message resource */
                mbWinTopInsertMesSet(mbPlayerNameMesGet(playerNo1), 0);
                /* miracle scene insert resource */
                mbWinTopInsertMesSet(MESSNUM(MESS_MIRACLE_MASU, 12), 1);
                mbWinTopWait();
            } else {
                mbObjMotionShiftSet(guide, 1, 0.0f, 8.0f,
                    ((1 << 30) | (1 << 0)));
                ev_CapMiracleStarTrade(work, playerNo1, playerNo2, 1,
                    0);
            }
            break;
        case 3:
            mbAudGuidePlay(949); /* event guide voice resource */
                    /* miracle scene message resource */
            mbWinCreate(2, ev_CapMiracleMesGet(MESSNUM(MESS_MIRACLE_MASU, 6)),
                        mbGuideSpeakerNoGet());
            mbWinTopInsertMesSet(mbPlayerNameMesGet(playerNo1), 0);
            /* miracle scene insert resource */
            mbWinTopInsertMesSet(MESSNUM(MESS_MIRACLE_MASU, 14), 1);
            mbWinTopInsertMesSet(mbPlayerNameMesGet(playerNo2), 2);
            mbWinTopWait();
            if (mbPlayerStarGet(playerNo1) <= 0) {
                mbAudGuidePlay(951); /* event guide voice resource */
                            mbWinCreate(2, ev_CapMiracleMesGet(MESSNUM(MESS_MIRACLE_MASU, 8)),
                    mbGuideSpeakerNoGet()); /* miracle scene message resource */
                mbWinTopInsertMesSet(mbPlayerNameMesGet(playerNo1), 0);
                /* miracle scene insert resource */
                mbWinTopInsertMesSet(MESSNUM(MESS_MIRACLE_MASU, 12), 1);
                mbWinTopWait();
            } else {
                mbObjMotionShiftSet(guide, 1, 0.0f, 8.0f,
                    ((1 << 30) | (1 << 0)));
                ev_CapMiracleStarTrade(work, playerNo1, playerNo2, 2,
                    0);
            }
            break;
        case 4:
            mbAudGuidePlay(949); /* event guide voice resource */
                    /* miracle scene message resource */
            mbWinCreate(2, ev_CapMiracleMesGet(MESSNUM(MESS_MIRACLE_MASU, 7)),
                        mbGuideSpeakerNoGet());
            mbWinTopInsertMesSet(mbPlayerNameMesGet(playerNo1), 0);
            mbWinTopInsertMesSet(mbPlayerNameMesGet(playerNo2), 1);
            /* miracle scene insert resource */
            mbWinTopInsertMesSet(MESSNUM(MESS_MIRACLE_MASU, 12), 2);
            mbWinTopWait();
            if (mbPlayerStarGet(playerNo1) <= 0
                && mbPlayerStarGet(playerNo2) <= 0) {
                mbAudGuidePlay(951); /* event guide voice resource */
                            mbWinCreate(2, ev_CapMiracleMesGet(MESSNUM(MESS_MIRACLE_MASU, 9)),
                    mbGuideSpeakerNoGet()); /* miracle scene message resource */
                /* miracle scene insert resource */
                mbWinTopInsertMesSet(MESSNUM(MESS_MIRACLE_MASU, 12), 0);
                mbWinTopWait();
            } else {
                mbObjMotionShiftSet(guide, 1, 0.0f, 8.0f,
                    ((1 << 30) | (1 << 0)));
                ev_CapMiracleStarTrade(work, playerNo1, playerNo2,
                    mbPlayerStarGet(playerNo1),
                    mbPlayerStarGet(playerNo2));
            }
            break;
        case 5:
            mbAudGuidePlay(949); /* event guide voice resource */
                    /* miracle scene message resource */
            mbWinCreate(2, ev_CapMiracleMesGet(MESSNUM(MESS_MIRACLE_MASU, 7)),
                        mbGuideSpeakerNoGet());
            mbWinTopInsertMesSet(mbPlayerNameMesGet(playerNo1), 0);
            mbWinTopInsertMesSet(mbPlayerNameMesGet(playerNo2), 1);
            /* miracle scene insert resource */
            mbWinTopInsertMesSet(MESSNUM(MESS_MIRACLE_MASU, 15), 2);
            mbWinTopWait();
            if (mbPlayerCoinGet(playerNo1) <= 0
                && mbPlayerCoinGet(playerNo2) <= 0
                && mbPlayerStarGet(playerNo1) <= 0
                && mbPlayerStarGet(playerNo2) <= 0) {
                mbAudGuidePlay(951); /* event guide voice resource */
                            mbWinCreate(2, ev_CapMiracleMesGet(MESSNUM(MESS_MIRACLE_MASU, 9)),
                    mbGuideSpeakerNoGet()); /* miracle scene message resource */
                /* miracle scene insert resource */
                mbWinTopInsertMesSet(MESSNUM(MESS_MIRACLE_MASU, 15), 0);
                mbWinTopWait();
            } else {
                mbObjMotionShiftSet(guide, 1, 0.0f, 8.0f,
                    ((1 << 30) | (1 << 0)));
                ev_CapMiracleCoinTrade(work, playerNo1, playerNo2,
                    mbPlayerCoinGet(playerNo1),
                    mbPlayerCoinGet(playerNo2));
                ev_CapMiracleStarTrade(work, playerNo1, playerNo2,
                    mbPlayerStarGet(playerNo1),
                    mbPlayerStarGet(playerNo2));
            }
            break;
    }
        if (mbStatusDispGet(playerNo1)) {
            mbStatusDispSet(playerNo1, FALSE);
        }
        if (mbStatusDispGet(playerNo2)) {
            mbStatusDispSet(playerNo2, FALSE);
        }
        players[0] = playerNo1;
        players[1] = playerNo2;
        {
            int motionF[2];

            motionF[0] = FALSE;
        motionF[1] = FALSE;
        for (i = 0; i < 2; i++) {
            if (oldStar[i] < mbPlayerStarGet(players[i])) {
                /* player win voice resource */
                mbPlayerWinLoseVoicePlay(players[i], 7, (573+0));
                mbPlayerMotionShiftSet(players[i], 7, 0.0f, 8.0f,
                    0);
                motionF[i] = TRUE;
            } else if (oldStar[i] > mbPlayerStarGet(players[i])) {
                /* player loss voice resource */
                mbPlayerWinLoseVoicePlay(players[i], 8, (573+12));
                mbPlayerMotionShiftSet(players[i], 8, 0.0f, 8.0f,
                    0);
                motionF[i] = TRUE;
            } else if (oldCoin[i] < mbPlayerCoinGet(players[i])) {
                /* player win voice resource */
                mbPlayerWinLoseVoicePlay(players[i], 12, (573+6));
                mbPlayerMotionShiftSet(players[i], 12, 0.0f, 8.0f,
                    0);
                motionF[i] = TRUE;
            } else if (oldCoin[i] > mbPlayerCoinGet(players[i])) {
                /* player loss voice resource */
                mbPlayerWinLoseVoicePlay(players[i], 13, (573+12));
                mbPlayerMotionShiftSet(players[i], 13, 0.0f, 8.0f,
                    0);
                motionF[i] = TRUE;
            }
        }
        HuPrcSleep(30);
        do {
            for (i = 0; i < 2; i++) {
                if (motionF[i] && !mbPlayerMotionEndCheck(players[i])) {
                    break;
                }
            }
            HuPrcVSleep();
        } while (i < 2);
        if (motionF[0]) {
            mbPlayerMotionShiftSet(players[0], 1, 0.0f, 8.0f,
                ((1 << 30) | (1 << 0)));
        }
        if (motionF[1]) {
            mbPlayerMotionShiftSet(players[1], 1, 0.0f, 8.0f,
                ((1 << 30) | (1 << 0)));
        }
        }
        mbObjMotionShiftSet(guide, 5, 0.0f, 8.0f,
            ((1 << 30) | (1 << 0)));
        mbAudGuidePlay(952); /* event guide voice resource */
            /* miracle scene message resource */
        mbWinCreate(2, ev_CapMiracleMesGet(MESSNUM(MESS_MIRACLE_MASU, 10)), mbGuideSpeakerNoGet());
        mbWinTopWait();
    }
    mbMusBoardFadeOut(0, 0, 1000, 1000, -1, FALSE);
    ev_CapMiracleSprDestroy();
}
static void ev_CapMiraclePlayerSet(void *unused, int playerNo1, int playerNo2,
    int masuId)
{
    HuVecF masuPos;
    HuVecF pos;
    int i;

    for (i = 0; i < 4; i++) {
        mbPlayerDispSet(i, FALSE);
    }
    mbMasuPosGet(masuId, &masuPos);

    pos.x = masuPos.x - 200.0f;
    pos.y = masuPos.y;
    pos.z = masuPos.z;
    mbPlayerPosSetV(playerNo1, &pos);
    mbPlayerRotSet(playerNo1, 0.0f, 30.0f, 0.0f);
    mbPlayerDispSet(playerNo1, TRUE);
    mbPlayerColSnapPlayerSet(playerNo1, FALSE);

    pos.x = 200.0f + masuPos.x;
    pos.y = masuPos.y;
    pos.z = masuPos.z;
    mbPlayerPosSetV(playerNo2, &pos);
    mbPlayerRotSet(playerNo2, 0.0f, -30.0f, 0.0f);
    mbPlayerDispSet(playerNo2, TRUE);
    mbPlayerColSnapPlayerSet(playerNo2, FALSE);
}

static void ev_CapMiracleCoinTrade(CAPWORK *work, int playerNo1,
    int playerNo2, int coinNum1, int coinNum2)
{
    HuVecF playerPos1;
    HuVecF playerPos2;
    int coinDelay = 1;
    int coinAddNum;

    mbPlayerPosGet(playerNo1, &playerPos1);
    playerPos1.y += 150.0f;
    mbPlayerPosGet(playerNo2, &playerPos2);
    playerPos2.y += 150.0f;
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
    do {
        if (coinNum1 > 0 && mbPlayerCoinGet(playerNo1) > 0) {
            if ((coinAddNum = mbev_CapCoinManAdd(work->coinManObj,
                &playerPos1, &playerPos2, playerNo2, TRUE)) != 0) {
                mbPlayerCoinAdd(playerNo1, -coinAddNum);
                coinNum1 -= coinAddNum;
            }
        }
        if (mbPlayerCoinGet(playerNo1) <= 0) {
            coinNum1 = 0;
        }
        if (coinNum2 > 0 && mbPlayerCoinGet(playerNo2) > 0) {
            if ((coinAddNum = mbev_CapCoinManAdd(work->coinManObj,
                &playerPos2, &playerPos1, playerNo1, TRUE)) != 0) {
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
    mbAudFXPlay(15);
}

static void ev_CapMiracleStarTrade(CAPWORK *work, int playerNo1, int playerNo2,
    int starNum1, int starNum2)
{
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
    playerPos1.y += 150.0f;
    mbPlayerPosGet(playerNo2, &playerPos2);
    playerPos2.y += 150.0f;
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

/* Maps a Miracle message choice to the message displayed by its event flow. */
static int ev_CapMiracleMesGet(int messNo)
{
    if (lbl_80265B70.curTime == FALSE) {
        return messNo;
    }
    return messNo + 16;
}

static void ev_CapMiracleDiceHitHook(int result)
{
    (void)result;
    diceHitTimer = 0;
}

static const float lbl_802C42C8 = 1.0f;

static void ev_CapMiracleWindowFadeOut(int oldModel, int newModel,
    int timeMax, BOOL reverseF)
{
    int time;
    float weight;

    if (reverseF) {
        mbObjDispSet((s16)oldModel, TRUE);
        mbObjLayerSet((s16)oldModel, 3);
        mbObjDispSet((s16)newModel, TRUE);
        mbObjAlphaSet((s16)newModel, 255);
        mbObjLayerSet((s16)newModel, 3);
        for (time = 0; time < timeMax; time++) {
            weight = (float)time / (float)timeMax;
            mbObjAlphaSet(newModel,
                (u8)(lbl_802C42C4 * weight));
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
            weight = (float)time / (float)timeMax;
            mbObjAlphaSet(oldModel,
                (u8)(lbl_802C42C4 * (lbl_802C42C8 - weight)));
            HuPrcVSleep();
        }
        mbObjDispSet(oldModel, FALSE);
        mbObjDispSet(newModel, TRUE);
        mbObjLayerSet(newModel, 3);
    }
}

static const float lbl_802C42D0 = 0.5f;

static void ev_CapMiracleWindowFadeIn(int oldModel, int newModel,
    int timeMax, BOOL reverseF, int motionStepFrames, int motionTimeCount,
    int *motionTimes)
{
    int time;
    int motionTime;
    int motionNo;
    float weight;

    motionTime = 0;
    motionNo = 0;
    if (reverseF) {
        mbObjDispSet(oldModel, TRUE);
        mbObjLayerSet(oldModel, 3);
        mbObjDispSet(newModel, TRUE);
        mbObjAlphaSet(newModel, 255);
        mbObjLayerSet(newModel, 3);
        for (time = 0; time < timeMax; time++) {
            weight = (float)time / (float)timeMax;
            mbObjAlphaSet(newModel,
                (u8)(lbl_802C42C4 * weight));
            if (++motionTime >= motionStepFrames) {
                motionTime = 0;
                if (++motionNo >= motionTimeCount) {
                    motionNo = 0;
                }
                mbObjMotionTimeSet((s16)newModel, lbl_802C42D0
                    + (float)motionTimes[motionNo]);
            }
            HuPrcVSleep();
        }
        mbObjDispSet((s16)oldModel, FALSE);
        mbObjDispSet((s16)newModel, TRUE);
        mbObjAlphaSet((s16)newModel, 255);
        mbObjLayerSet((s16)newModel, 3);
    } else {
        mbObjDispSet((s16)oldModel, TRUE);
        mbObjAlphaSet((s16)oldModel, 255);
        mbObjLayerSet((s16)oldModel, 3);
        mbObjDispSet((s16)newModel, TRUE);
        mbObjLayerSet((s16)newModel, 3);
        for (time = 0; time < timeMax; time++) {
            weight = (float)time / (float)timeMax;
            mbObjAlphaSet(oldModel,
                (u8)(lbl_802C42C4 * (lbl_802C42C8 - weight)));
            if (++motionTime >= motionStepFrames) {
                motionTime = 0;
                if (++motionNo >= motionTimeCount) {
                    motionNo = 0;
                }
                mbObjMotionTimeSet((s16)newModel, lbl_802C42D0
                    + (float)motionTimes[motionNo]);
            }
            HuPrcVSleep();
        }
        mbObjDispSet((s16)oldModel, FALSE);
        mbObjDispSet((s16)newModel, TRUE);
        mbObjLayerSet((s16)newModel, 3);
    }
}

static int ev_CapMiracleDiceExec(int playerNo, int modelId, int timeMax,
    int valueNum, int *motTimeTbl)
{
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
                0.5f + (float)motTimeTbl[value]);
        }
        HuPrcVSleep();
    } while (!mbDiceKillCheck(playerNo));
    if (winNo != -1) {
        mbWinKill(winNo);
    }
    return value;
}

/* Creates the Miracle trade-choice sprites and initializes their animation state. */
static void ev_CapMiracleSprCreate(void)
{
    MIRACLE_SPR_WORK *work;
    int i;
    int j;
    OMOBJ *obj;

    miracleSprObj = obj = fn_8003AB38(
        mbObjMan, -32768, 0, 0, -1, ev_CapMiracleSprUpdate);
    work = obj->data = HuMemDirectMallocNum(
        HEAP_HEAP, 6 * sizeof(MIRACLE_SPR_WORK), 0x10000000);
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
        work->resetFloatA = 0.0f;
        work->resetFloatB = 0.0f;
    }
}

/* Object-manager callback that animates and positions the Miracle trade sprites. */
static void ev_CapMiracleSprUpdate(OMOBJ *obj)
{
    MIRACLE_SPR_WORK *work = obj->data;
    int i;
    int j;
    float time;
    float angle;
    float angle2;
    float angle3;
    float radius;
    float scale;
    HuVecF pos;

    if (mbExitCheck() || miracleSprObj == NULL) {
        for (i = 0; i < 6; i++, work++) {
            if (work->activeF) {
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
        }
        omDelObjEx(mbObjMan, obj);
        miracleSprObj = NULL;
        return;
    }
    for (i = 0; i < 6; i++, work++) {
        if (!work->activeF) {
            continue;
        }
        switch (work->focusTime) {
            case 0:
                time = (float)++work->focusNo / 30.0f;
                if (time < 1.0f) {
                    scale = 1.0 + 5.0 * cos(
                        (3.141592653589793 * (90.0f * time)) / 180.0);
                    espScaleSet((s16)work->sprId, scale, scale);
                    espTPLvlSet((s16)work->sprId, time);
                    espZRotSet((s16)work->sprId, 0.0f);
                    scale = time + 0.5 * sin(
                        (3.141592653589793 * (180.0f * time)) / 180.0);
                    espScaleSet((s16)work->backSprId, scale, scale);
                    angle = 90.0f * time;
                    radius = 300.0f * (1.0f - time);
                    for (j = 0; j < 6; j++) {
                        angle2 = angle + 60.0f * (float)j;
                        angle3 = angle2 + angle;
                        pos.x = work->pos.x + radius * sin(
                            (3.141592653589793 * angle3) / 180.0);
                        pos.y = work->pos.y + radius * cos(
                            (3.141592653589793 * angle3) / 180.0);
                        espPosSet((s16)work->sprIdTbl[j], pos.x, pos.y);
                        espScaleSet((s16)work->sprId, scale, scale);
                        espTPLvlSet((s16)work->sprIdTbl[j], 0.25f * time);
                        espZRotSet((s16)work->sprIdTbl[j],
                            (1.0f - time) * angle2);
                    }
                } else {
                    mbAudFXPlay(1138); /* event sound-effect resource */
                    scale = 1.0f;
                    espTPLvlSet((s16)work->sprId, 1.0f);
                    espZRotSet((s16)work->sprId, 0.0f);
                    espScaleSet((s16)work->sprId, scale, scale);
                    espScaleSet((s16)work->backSprId, scale, scale);
                    for (j = 0; j < 6; j++) {
                        espScaleSet((s16)work->sprIdTbl[j], 0.0f, 0.0f);
                    }
                    espKill((s16)work->sprIdTbl[1]);
                    work->sprIdTbl[1] = (s16)espEntry(
                        DATANUM(0x110000, 58), 500, 0); /* event sprite resource */
                    espDrawNoSet((s16)work->sprIdTbl[1], 32);
                    espPosSet((s16)work->sprIdTbl[1],
                        work->pos.x, work->pos.y);
                    espScaleSet((s16)work->sprIdTbl[1], 0.0f, 0.0f);
                    espTPLvlSet((s16)work->sprIdTbl[1], 0.25f);
                    espAttrSet((s16)work->sprIdTbl[1], HUSPR_ATTR_LINEAR);
                    work->focusTime++;
                    work->focusNo = 0;
                }
                break;

            case 1:
                time = (float)++work->focusNo / 18.0f;
                scale = 1.0 + 10.0 * sin(
                    (3.141592653589793 * (90.0f * time)) / 180.0);
                if (time < 1.0f) {
                    {
                        espPosSet((s16)work->sprIdTbl[0],
                            work->pos.x, work->pos.y);
                        espScaleSet((s16)work->sprIdTbl[0], scale, scale);
                        espTPLvlSet((s16)work->sprIdTbl[0],
                            cos((3.141592653589793 * (90.0f * time)) / 180.0)
                                * cos((3.141592653589793 * (90.0f * time)) / 180.0)
                                * 0.5);
                        espZRotSet((s16)work->sprIdTbl[0], 0.0f);
                    }
                    {
                        espPosSet((s16)work->sprIdTbl[1],
                            work->pos.x, work->pos.y);
                        espScaleSet((s16)work->sprIdTbl[1], scale, scale);
                        espTPLvlSet((s16)work->sprIdTbl[1],
                            cos((3.141592653589793 * (90.0f * time)) / 180.0)
                                * cos((3.141592653589793 * (90.0f * time)) / 180.0)
                                * 0.5);
                        espZRotSet((s16)work->sprIdTbl[1], 0.0f);
                    }
                } else {
                    scale = 1.0f;
                    espScaleSet((s16)work->sprId, scale, scale);
                    espScaleSet((s16)work->backSprId, scale, scale);
                    espScaleSet((s16)work->sprIdTbl[0], 0.0f, 0.0f);
                    espScaleSet((s16)work->sprIdTbl[1], 0.0f, 0.0f);
                    work->focusTime++;
                    work->focusNo = 0;
                }
                break;

            case 2:
                if (work->hideF) {
                    scale = 1.0f;
                    espScaleSet((s16)work->sprId, scale, scale);
                    espScaleSet((s16)work->backSprId, scale, scale);
                    work->focusTime = CAPSPECIAL_MIRACLE_SPR_HIDE;
                    work->focusNo = 0;
                }
                break;

            case CAPSPECIAL_MIRACLE_SPR_PULSE:
                time = (float)++work->focusNo / 240.0f;
                if (time >= 1.0f && !work->hideF) {
                    time = fmod(time, 1.0);
                    work->focusNo = (int)((float)work->focusNo - 240.0f);
                }
                scale = 1.0 + 0.5 * fabs(sin(
                    (3.141592653589793 * (1440.0f * time)) / 180.0));
                if (time < 1.0f && !work->hideF) {
                    espScaleSet((s16)work->sprIdTbl[0], scale, scale);
                    espTPLvlSet((s16)work->sprIdTbl[0],
                        0.25 + 0.25 * fabs(sin(
                            (3.141592653589793 * (1440.0f * time)) / 180.0)));
                    espZRotSet((s16)work->sprIdTbl[0], 0.0f);
                    espScaleSet((s16)work->sprIdTbl[1], scale, scale);
                    espTPLvlSet((s16)work->sprIdTbl[1],
                        0.25 + 0.25 * fabs(sin(
                            (3.141592653589793 * (1440.0f * time)) / 180.0)));
                    espZRotSet((s16)work->sprIdTbl[0], 0.0f);
                } else {
                    scale = 1.0f;
                    espScaleSet((s16)work->sprId, scale, scale);
                    espScaleSet((s16)work->backSprId, scale, scale);
                    espScaleSet((s16)work->sprIdTbl[0], 0.0f, 0.0f);
                    espScaleSet((s16)work->sprIdTbl[1], 0.0f, 0.0f);
                    work->focusTime = CAPSPECIAL_MIRACLE_SPR_HIDE;
                    work->focusNo = 0;
                }
                break;

            case CAPSPECIAL_MIRACLE_SPR_HIDE:
                time = (float)++work->focusNo / 30.0f;
                scale = cos((3.141592653589793 * (90.0f * time)) / 180.0);
                if (time < 1.0f) {
                    espPosSet((s16)work->sprId, work->pos.x,
                        work->pos.y - 280.0f * time);
                    espPosSet((s16)work->backSprId, work->pos.x,
                        work->pos.y - 280.0f * time);
                } else {
                    espDispOff((s16)work->sprId);
                    espDispOff((s16)work->backSprId);
                    espDispOff((s16)work->sprIdTbl[0]);
                    espDispOff((s16)work->sprIdTbl[1]);
                    work->focusTime++;
                    work->focusNo = 0;
                }
                break;
        }
    }
}

static void ev_CapMiracleSprDestroy(void)
{
    miracleSprObj = NULL;
}

/* Shows the Miracle trade choices at pos, initially focusing the selected choice. */
static void ev_CapMiracleTradeCreate(HuVecF *pos, int no)
{
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
    work->resetFloatA = 0.0f;
    work->resetFloatB = 0.0f;
    work->pos = *pos;
    if (no < 0) {
        no = 0;
    } else if (no > 5) {
        no = 5;
    }
    file = miracleTradeFileTbl[no];
    work->sprId = (s16)espEntry(file, 100, 0);
    espPosSet((s16)work->sprId, pos->x, pos->y);
    espScaleSet((s16)work->sprId, 0.0f, 0.0f);
    espAttrSet((s16)work->sprId, HUSPR_ATTR_LINEAR);
    espDrawNoSet((s16)work->sprId, 32);
    work->backSprId = (s16)espEntry(miracleBackFile, 120, 0);
    espPosSet((s16)work->backSprId, pos->x, pos->y);
    espScaleSet((s16)work->backSprId, 0.0f, 0.0f);
    espAttrSet((s16)work->backSprId, HUSPR_ATTR_LINEAR);
    espDrawNoSet((s16)work->backSprId, 32);
    for (j = 0; j < 6; j++) {
        work->sprIdTbl[j] = (s16)espEntry(file, 100, 0);
        espPosSet((s16)work->sprIdTbl[j], pos->x, pos->y);
        espScaleSet((s16)work->sprIdTbl[j], 0.0f, 0.0f);
        espAttrSet((s16)work->sprIdTbl[j], HUSPR_ATTR_LINEAR);
        espDrawNoSet((s16)work->sprIdTbl[j], 32);
    }
}

/* Updates which Miracle trade choice is highlighted from the current pad input. */
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

/* Hides the Miracle trade-choice sprites while the event continues its scene. */
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

/* Runs the Kettou duel capsule event from the capsule callback table. */
void mbev_CapKettou(void)
{
    CAPWORK *work = fn_8000DD9C()->property;
    OMOBJ *guideObj;
    int guideModel;
    int guideSet;
    int i;
    int dif;
    int total;

    mbev_CapWait(work);
    if (!lbl_80265B70.curTime) {
        guideSet = 0;
    } else {
        guideSet = 1;
    }
    guideObj = mbGuideCreateIn();
    guideModel = mbGuideModelGet(guideObj);
    mbObjDispSet(guideModel, FALSE);
    for (i = 0; (s32)kettouGuideMotTbl[guideSet][i] >= 0; i++) {
        kettouMotId[i] = mbObjMotionCreate(guideModel,
            kettouGuideMotTbl[guideSet][i]);
    }
    mbObjMotionSet(guideModel, kettouMotId[1], ((1 << 30) | (1 << 0)));
    mbPlayerColSnapPlayerSet(work->playerNo, TRUE);
    work->eventData[0] = guideModel;
    work->guideObj = guideObj;
    if (!work->flags._flag02) {
        if (ev_CapKettouStart(work)) {
        {
        int winner = mgResultData.playerNo1;
        if (!fn_8006A124(65551)) {
            lbl_80265750[winner].mgCoinBonus = 0;
        }
        }
        {
        int loser = mgResultData.playerNo2;
        if (!fn_8006A124(65551)) {
            lbl_80265750[loser].mgCoinBonus = 0;
        }
        }
        for (i = 0; i < 4; i++) {
            lbl_80265728[i].grpNo = 2;
        }
        lbl_80265728[mgResultData.playerNo1].grpNo = 0;
        lbl_80265728[mgResultData.playerNo2].grpNo = 1;
        if ((lbl_80265750[mgResultData.playerNo1].comF
                && lbl_80265750[mgResultData.playerNo2].comF
                && !GWMgComDispGet())
            || mbMgRouletteNumGet(6) <= 0) {
            int weight[3];
            int roll;

            dif = abs((int)lbl_80265750[
                mgResultData.playerNo1].comDif
                - (int)lbl_80265750[
                mgResultData.playerNo2].comDif);
            if (dif < 0) {
                dif = 0;
            } else if (dif > 3) {
                dif = 3;
            }
            weight[0] = 15 - (dif * 4);
            weight[1] = 50 + ((int)lbl_80265750[
                mgResultData.playerNo1].comDif * 20);
            weight[2] = 50 + ((int)lbl_80265750[
                mgResultData.playerNo2].comDif * 20);
            total = weight[0] + weight[1] + weight[2];
            roll = mbRandMod(total);
            if (roll < weight[0]) {
                int winner = mgResultData.playerNo1;
                if (!fn_8006A124(65551)) {
                    lbl_80265750[winner].mgCoinBonus = 0;
                }
                {
                int loser = mgResultData.playerNo2;
                if (!fn_8006A124(65551)) {
                    lbl_80265750[loser].mgCoinBonus = 0;
                }
                }
            } else if (roll < weight[1]) {
                int winner = mgResultData.playerNo1;
                if (!fn_8006A124(65551)) {
                    lbl_80265750[winner].mgCoinBonus = 10;
                }
            } else {
                int loser = mgResultData.playerNo2;
                if (!fn_8006A124(65551)) {
                    lbl_80265750[loser].mgCoinBonus = 10;
                }
            }
            mbWipeFadeOut();
            ev_CapKettouReturn(work);
        } else {
            int guideSpeaker = mbGuideSpeakerNoGet();
            /* duel scene message resource */
            mbWinCreate(2, ev_CapKettouMesGet(MESSNUM(MESS_KETTOU_MASU, 12)), guideSpeaker);
            mbWinTopWait();
            mbAudFXDelaySet(30);
            mbAudGuidePlay(950); /* guide voice resource */
            mbObjMotionShiftSet(guideModel, kettouMotId[5],
                0.0f, 8.0f, 0);
            mbev_MgCallKettou();
        }
        }
    } else {
        ev_CapKettouReturn(work);
    }
    if (work->guideObj != NULL) {
        mbGuideKill(guideObj);
    }
    HuPrcEnd();
}

void mbev_CapKettouKill(void)
{
}

/* Runs the Kettou capsule battle scene and returns its selected result. */
static int ev_CapKettouStart(CAPWORK *work)
{
    extern void mbStatusDispForceSetAll(BOOL dispF);
    extern void mbev_Scroll(int playerNo, BOOL mapF);
    extern void mbev_CapStatusDispSetAll(BOOL dispF, BOOL waitF);
    extern void mbGuideEnd(OMOBJ *obj, BOOL endF);
    extern const float lbl_802C4434;
    HuVecF masuPos;
    HuVecF pos;
    HuVecF cameraOfs;
    HuVecF direction;
    HuVecF avgPos;
    int playerNo = work->playerNo;
    int masuId = lbl_80265750[playerNo].masuId;
    int playerList[4];
    int compactList[4];
    int playerNum;
    int targetPlayer;
    int amount;
    int coinTakeNum;
    int i;
    int counter;
    int sprId;
    int sprite0;
    int sprite1;
    int sprite[3];
    int add[4];
    int starObj[2];
    int coinDisp[2];
    int modelId;
    int resourceType;

    float t;
    int prevCoin;
    int prevCoinDir;
    int helpWin;
    int padBtn;
    int choiceStar;

    BOOL starEnable;
    BOOL coinEnable;
    BOOL coinChoiceF;
    BOOL starChoiceF;
    BOOL initCoinNumF;
    BOOL resumeF;
    BOOL failureF;

    mbPlayerPosGet(playerNo, &masuPos);
    modelId = work->eventData[0];
    if (work->flags._flag01) {
        resumeF = TRUE;
    } else {
        int playerMot[4];

        resumeF = FALSE;
        avgPos.x = avgPos.y = avgPos.z = 0.0f;
        for (i = 0, playerNum = 0; i < 4; i++) {
        if (lbl_80265750[i].masuId == masuId) {
            if (playerNum >= 2) {
                counter = mbRandMod(3);
            } else {
                counter = mbRandMod(2);
            }
            playerMot[i] = mbev_CapPlayerMotionCreate(&work->objWork, i,
                kettouPlayerMotTbl[counter]);
            if (counter == 2) {
                fn_8002DC74(
                    mbObjMotionIDGet(mbPlayerObjIDGet(i), playerMot[i]),
                    1);
            }
            mbPlayerPosGet(i, &cameraOfs);
            PSVECAdd(&avgPos, &cameraOfs, &avgPos);
            if (i != playerNo) {
                playerList[playerNum] = i;
                playerNum++;
            }
        }
        }
        PSVECScale(&avgPos, &avgPos, 1.0f / (playerNum + 1));
        for (i = 0; i < 4; i++) {
            if (lbl_80265750[i].masuId == masuId) {
                mbPlayerPosGet(i, &cameraOfs);
                PSVECSubtract(&avgPos, &cameraOfs, &direction);
                mbPlayerRotateStart(i,
                    (s16)((atan2(direction.x, direction.z) / 3.141592653589793)
                        * 180.0), 15);
            }
        }
        do {
            for (i = 0; i < 4; i++) {
                if (lbl_80265750[i].masuId == masuId
                    && !mbPlayerRotateCheck(i)) {
                    break;
                }
            }
            HuPrcVSleep();
        } while (i < 4);
        for (i = 0; i < 4; i++) {
            if (lbl_80265750[i].masuId == masuId) {
                mbPlayerMotionShiftSet(i, playerMot[i], 0.0f, 8.0f,
                    ((1 << 30) | (1 << 0)));
            }
        }
        HuPrcSleep(60);
    }
    mbMasuPosGet(masuId, &masuPos);
    mbWipeSpecialFadeInCreate(2, 1);
    for (i = 0; i < 4; i++) {
        mbPlayerMotionSet(i, 1, ((1 << 30) | (1 << 0)));
        mbPlayerRotSet(i, 0.0f, 0.0f, 0.0f);
    }
    pos.x = masuPos.x;
    pos.y = masuPos.y + 200.0f;
    pos.z = masuPos.z - 50.0f;
    mbObjPosSetV(modelId, &pos);
    mbObjDispSet(modelId, TRUE);
    mbStatusDispForceSetAll(TRUE);
    cameraOfs.x = 0.0f;
    cameraOfs.y = 100.0f;
    cameraOfs.z = 0.0f;
    mbCameraMovePlayer(playerNo, NULL, &cameraOfs,
        1500.0f, -1.0f, -1);
    mbCameraMoveWait();
    /* event sprite resource identifier */
    sprite0 = sprId = mbev_CapSprCreate(&work->objWork, DATANUM(0x110000, 65), 120, 0);
    espPosSet((s16)sprId, 288.0f, 224.0f);
    espTPLvlSet((s16)sprId, 0.8f);
    espDispOff((s16)sprId);
    /* event sprite resource identifier */
    sprite1 = sprId = mbev_CapSprCreate(&work->objWork, DATANUM(0x110000, 67), 110, 0);
    espPosSet((s16)sprId, 230.0f, 224.0f);
    espDispOff((s16)sprId);
    /* event sprite resource identifier */
    sprite[0] = sprId = mbev_CapSprCreate(&work->objWork, DATANUM(0x110000, 66), 100, 0);
    espPosSet((s16)sprId, 272.0f, 224.0f);
    espBankSet((s16)sprId, 10);
    espDispOff((s16)sprId);
    /* event sprite resource identifier */
    sprite[1] = sprId = mbev_CapSprCreate(&work->objWork, DATANUM(0x110000, 66), 100, 0);
    espPosSet((s16)sprId, 312.0f, 224.0f);
    espBankSet((s16)sprId, 0);
    espDispOff((s16)sprId);
    /* event sprite resource identifier */
    sprite[2] = sprId = mbev_CapSprCreate(&work->objWork, DATANUM(0x110000, 66), 100, 0);
    espPosSet((s16)sprId, 352.0f, 224.0f);
    espBankSet((s16)sprId, 1);
    espDispOff((s16)sprId);
    initCoinNumF = FALSE;
    mbMusBoardFadeOut(0, 0, 1000, 1000, MSM_STREAM_STORY_LOSE, FALSE);
    mbWipeSpecialFadeOutCreate(2, 60);
    mbAudGuidePlay(950); /* event guide-voice resource */
    /* duel scene message resource */
    mbWinCreate(2, ev_CapKettouMesGet(MESSNUM(MESS_KETTOU_MASU, 0)), mbGuideSpeakerNoGet());
    mbWinTopWait();
    if (mbPlayerStarGet(playerNo) <= 0 && mbPlayerCoinGet(playerNo) < 40) {
        starEnable = FALSE;
    } else {
        starEnable = TRUE;
    }
    if (mbPlayerCoinGet(playerNo) <= 0) {
        coinEnable = FALSE;
    } else {
        coinEnable = TRUE;
    }

    if (resumeF) {
        for (i = 0, playerNum = 0; i < 4; i++) {
            if (mbPlayerCoinGet(i) <= 0 && mbPlayerStarGet(i) <= 0) {
                continue;
            }
            if (!starEnable && mbPlayerCoinGet(i) <= 0) {
                continue;
            }
            if (!coinEnable && mbPlayerStarGet(i) <= 0) {
                continue;
            }
            if (GWTeamFGet() && mbev_CapPlayerCheck(playerNo, i)) {
                continue;
            }
            if (i == playerNo) {
                continue;
            }
            playerList[playerNum] = i;
                playerNum++;
        }
    } else {
        for (i = 0, playerNum = 0; i < 4; i++) {
            if (mbPlayerCoinGet(i) <= 0 && mbPlayerStarGet(i) <= 0) {
                continue;
            }
            if (!starEnable && mbPlayerCoinGet(i) <= 0) {
                continue;
            }
            if (!coinEnable && mbPlayerStarGet(i) <= 0) {
                continue;
            }
            if (GWTeamFGet() && mbev_CapPlayerCheck(playerNo, i)) {
                continue;
            }
            if (i == playerNo || lbl_80265750[i].masuId != masuId) {
                continue;
            }
            playerList[playerNum] = i;
                playerNum++;
        }
    }
    if (mbPlayerCoinGet(playerNo) <= 0 && mbPlayerStarGet(playerNo) <= 0) {
        playerNum = 0;
    }

repeatTarget:
    switch (playerNum) {
    case 3:
        do {
            mbWinCreateChoice(1, ev_CapKettouMesGet(MESSNUM(MESS_KETTOU_MASU, 1)),
                              mbGuideSpeakerNoGet(), 0);
            mbWinTopAttrSet(HUWIN_ATTR_NOCANCEL);
            for (i = 0; i < playerNum; i++) {
                compactList[i] = playerList[i];
            }
            if (lbl_80265750[playerNo].comF) {
                if (starEnable) {
                    targetPlayer = mbev_CapPlayerComSelKettouGet(playerNo,
                        1, compactList, playerNum);
                } else {
                    targetPlayer = mbev_CapPlayerComSelKettouGet(playerNo,
                        0, compactList, playerNum);
                }
                mbComChoiceListDownSet(targetPlayer);
            }
            mbWinTopInsertMesSet(mbPlayerNameMesGet(playerList[0]), 0);
            mbWinTopInsertMesSet(mbPlayerNameMesGet(playerList[1]), 1);
            mbWinTopInsertMesSet(mbPlayerNameMesGet(playerList[2]), 2);
            mbWinTopWait();
            targetPlayer = mbWinTopChoiceGet();
            if (targetPlayer == 4) {
                mbev_Scroll(playerNo, FALSE);
                mbev_CapStatusDispSetAll(TRUE, TRUE);
            }
        } while (targetPlayer == 4);
        if (targetPlayer >= 3 || targetPlayer == -1) {
            targetPlayer = playerList[mbRandMod(3)];
            if (targetPlayer < 0) {
                targetPlayer = playerList[0];
            }
        } else {
            targetPlayer = playerList[targetPlayer];
        }
        break;
    case 2:
        do {
            mbWinCreateChoice(1, ev_CapKettouMesGet(MESSNUM(MESS_KETTOU_MASU, 2)),
                              mbGuideSpeakerNoGet(), 0);
            mbWinTopAttrSet(HUWIN_ATTR_NOCANCEL);
            for (i = 0; i < playerNum; i++) {
                compactList[i] = playerList[i];
            }
            if (lbl_80265750[playerNo].comF) {
                if (starEnable) {
                    targetPlayer = mbev_CapPlayerComSelKettouGet(playerNo,
                        1, compactList, playerNum);
                } else {
                    targetPlayer = mbev_CapPlayerComSelKettouGet(playerNo,
                        0, compactList, playerNum);
                }
                mbComChoiceListDownSet(targetPlayer);
            }
            mbWinTopInsertMesSet(mbPlayerNameMesGet(playerList[0]), 0);
            mbWinTopInsertMesSet(mbPlayerNameMesGet(playerList[1]), 1);
            mbWinTopWait();
            targetPlayer = mbWinTopChoiceGet();
            if (targetPlayer == 3) {
                mbev_Scroll(playerNo, FALSE);
                mbev_CapStatusDispSetAll(TRUE, TRUE);
            }
        } while (targetPlayer == 3);
        if (targetPlayer >= 2 || targetPlayer == -1) {
            targetPlayer = playerList[mbRandMod(2)];
            if (targetPlayer < 0) {
                targetPlayer = playerList[0];
            }
        } else {
            targetPlayer = playerList[targetPlayer];
        }
        break;
    case 1:
        targetPlayer = playerList[0];
        break;
    default:
        failureF = FALSE;
        if (GWTeamFGet() && !resumeF) {
            for (i = 0, playerNum = 0; i < 4; i++) {
                if (i != playerNo
                    && lbl_80265750[playerNo].masuId == lbl_80265750[i].masuId) {
                    playerList[playerNum] = i;
                    playerNum++;
                }
            }
            if (playerNum == 1
                && mbev_CapPlayerCheck(playerNo, playerList[0])) {
                failureF = TRUE;
            }
        }
        mbAudGuidePlay(951);
        if (failureF) {
            mbWinCreate(2, ev_CapKettouMesGet(MESSNUM(MESS_KETTOU_MASU, 21)),
                mbGuideSpeakerNoGet());
            mbWinTopInsertMesSet(mbPlayerNameMesGet(playerNo), 0);
            mbWinTopInsertMesSet(mbPlayerNameMesGet(playerList[0]), 1);
            mbWinTopWait();
        } else {
            mbWinCreate(2, ev_CapKettouMesGet(MESSNUM(MESS_KETTOU_MASU, 20)),
                mbGuideSpeakerNoGet());
            mbWinTopWait();
        }
        mbMusBoardFadeOut(0, 0, 1000, 1000, -1, FALSE);
        mbGuideEnd(work->guideObj, TRUE);
        work->guideObj = NULL;
        return 0;
    }

    {
        extern void mbev_CapStatusDispSetAll(BOOL dispF, BOOL waitF);
        extern void mbev_CapDuelStatusOnSet(int playerNo1, int playerNo2,
            BOOL waitF);
        extern s8 mbPadStkYGet(int padNo);

        int coinDir;
        int coinDelay;
        float padStk;
        char message[16];
        float scaleY;

    repeatWager:
        mbWinCreateChoice(1, ev_CapKettouMesGet(MESSNUM(MESS_KETTOU_MASU, 3)),
                          mbGuideSpeakerNoGet(), 0);
        if (playerNum < 2) {
            mbWinTopAttrSet(HUWIN_ATTR_NOCANCEL);
            mbWinTopChoiceDisable(2);
        }
        coinChoiceF = starChoiceF = TRUE;
        if (mbPlayerCoinGet(targetPlayer) <= 0 || !coinEnable) {
            mbWinTopChoiceDisable(0);
            coinChoiceF = FALSE;
        }
        if (mbPlayerStarGet(targetPlayer) <= 0 || !starEnable) {
            mbWinTopChoiceDisable(1);
            starChoiceF = FALSE;
        }
        if (lbl_80265750[playerNo].comF) {
            if (coinChoiceF && starChoiceF) {
                mbComChoiceDownSet();
            } else {
                mbComChoiceUpSet();
            }
        }
        mbWinTopWait();
        resourceType = mbWinTopChoiceGet();
        if (resourceType == 2 || resourceType == -1) {
            goto repeatTarget;
        }
        if (resourceType == 0) {
            prevCoin = amount = 1;
            sprintf(message, capspecialMesFormat, amount);
            prevCoinDir = coinDir = coinDelay = 0;
            mbWinCreate(2, ev_CapKettouMesGet(MESSNUM(MESS_KETTOU_MASU, 5)), mbGuideSpeakerNoGet());
            mbWinTopWait();
            if (!initCoinNumF) {
                espDispOn((s16)sprite0);
                espDispOn((s16)sprite1);
                for (i = 0; i < 3; i++) {
                    espDispOn((s16)sprite[i]);
                }
                espBankSet((s16)sprite[1], 0);
                espBankSet((s16)sprite[2], 1);
                for (counter = 0; counter <= 12.0f; counter++) {
                    t = (float)counter / 12.0f;
                    scaleY = sin(3.141592653589793*(t * 90.0f)/180.0);
                    espScaleSet((s16)sprite0, 1, scaleY);
                    espScaleSet((s16)sprite1, 1, scaleY);
                    for (i = 0; i < 3; i++) {
                        espScaleSet((s16)sprite[i], 1, scaleY);
                    }
                    HuPrcVSleep();
                }
                helpWin = mbWinCreateHelp(MESSNUM(MESS_KETTOU_MASU, 22));
                mbWinPosSet(helpWin, 115, 288);
                initCoinNumF = TRUE;
            }
            coinTakeNum = mbPlayerCoinGet(playerNo);
            coinTakeNum = (int)(coinTakeNum * MBCapsuleEffRandF());
            if (MBCapsuleEffRandF() < 0.3f) {
                coinTakeNum = mbPlayerCoinGet(playerNo);
            }
            if (coinTakeNum > mbPlayerCoinGet(targetPlayer)) {
                coinTakeNum = mbPlayerCoinGet(targetPlayer);
            }
            if (coinTakeNum < 1) {
                coinTakeNum = 1;
            }
            if (coinTakeNum > 99) {
                coinTakeNum = 99;
            }

            for (;;) {
                padBtn = lbl_802C0008[lbl_80265750[playerNo].padNo];
                padStk = mbPadStkYGet(lbl_80265750[playerNo].padNo);
                if (lbl_80265750[playerNo].comF) {
                    padBtn = 0;
                    padStk = 0.0f;
                    if (amount < coinTakeNum) {
                        padStk = 32.0f;
                    } else {
                        HuPrcSleep(5);
                        padBtn = PAD_BUTTON_A;
                    }
                }
                if (padBtn & PAD_BUTTON_A) {
                    mbAudFXPlay(2);
                    break;
                } else if (padBtn & PAD_BUTTON_B) {
                    if (!initCoinNumF) {
                        goto repeatWager;
                    }
                    for (counter = 0; counter <= 12.0f; counter++) {
                        t = (float)counter / 12.0f;
                        scaleY = cos(3.141592653589793*(t * 90.0f)/180.0);
                        espScaleSet((s16)sprite0, 1, scaleY);
                        espScaleSet((s16)sprite1, 1, scaleY);
                        for (i = 0; i < 3; i++) {
                            espScaleSet((s16)sprite[i], 1, scaleY);
                        }
                        HuPrcVSleep();
                    }
                    espDispOff((s16)sprite0);
                    espDispOff((s16)sprite1);
                    for (i = 0; i < 3; i++) {
                        espDispOff((s16)sprite[i]);
                    }
                    mbWinKill(helpWin);
                    initCoinNumF = FALSE;
                    goto repeatWager;
                } else {
                    if (fabs(padStk) >= 8.0) {
                        if (padStk >= *((const float *)&lbl_802C4370)) {
                            coinDir = 1;
                        }
                        if (padStk <= *((const float *)&lbl_802C4370)) {
                            coinDir = -1;
                        }
                        if (coinDir == 0) {
                            coinDelay = 0;
                        } else if (prevCoinDir == coinDir) {
                            if (++coinDelay > 30.0f) {
                                amount += coinDir;
                                coinDelay -= 2;
                            }
                        } else {
                            amount += coinDir;
                            coinDelay = 0;
                        }
                        prevCoinDir = coinDir;
                        if (amount < 1) {
                            amount = 1;
                        } else if (amount > 99) {
                            amount = 99;
                        } else if (amount > mbPlayerCoinGet(playerNo)) {
                            amount = mbPlayerCoinGet(playerNo);
                        } else if (amount > mbPlayerCoinGet(targetPlayer)) {
                            amount = mbPlayerCoinGet(targetPlayer);
                        }
                    } else {
                        prevCoinDir = coinDir = coinDelay = 0;
                    }
                    if (prevCoin != amount) {
                        mbAudFXPlay(0);
                        if (amount >= 100) {
                            espBankSet((s16)sprite[1], 9);
                        } else {
                            espBankSet((s16)sprite[1], amount / 10);
                        }
                        espBankSet((s16)sprite[2], amount % 10);
                        prevCoin = amount;
                        if (coinDelay < 27.0f) {
                            HuPrcSleep(3);
                        }
                    }
                }
                HuPrcVSleep();
            }

            if (initCoinNumF) {
                for (counter = 0; counter <= 12.0f; counter++) {
                    t = (float)counter / 12.0f;
                    scaleY = cos(3.141592653589793*(t * 90.0f)/180.0);
                    espScaleSet((s16)sprite0, 1, scaleY);
                    espScaleSet((s16)sprite1, 1, scaleY);
                    for (i = 0; i < 3; i++) {
                        espScaleSet((s16)sprite[i], 1, scaleY);
                    }
                    HuPrcVSleep();
                }
                espDispOff((s16)sprite0);
                espDispOff((s16)sprite1);
                for (i = 0; i < 3; i++) {
                    espDispOff((s16)sprite[i]);
                }
                mbWinKill(helpWin);
                initCoinNumF = FALSE;
            }
            choiceStar = 0;
        } else {
            mbWinCreateChoice(1, ev_CapKettouMesGet(MESSNUM(MESS_KETTOU_MASU, 4)),
                              mbGuideSpeakerNoGet(), 0);
            coinChoiceF = starChoiceF = TRUE;
            if (mbPlayerCoinGet(playerNo) < 40) {
                mbWinTopChoiceDisable(0);
                coinChoiceF = FALSE;
            }
            if (mbPlayerStarGet(playerNo) <= 0) {
                mbWinTopChoiceDisable(1);
                starChoiceF = FALSE;
            }
            if (lbl_80265750[playerNo].comF) {
                if (coinChoiceF) {
                    mbComChoiceUpSet();
                } else {
                    mbComChoiceRightSet();
                }
            }
            mbWinTopWait();
            choiceStar = mbWinTopChoiceGet();
            if (choiceStar == 2 || choiceStar == -1) {
                goto repeatWager;
            }
            if (choiceStar == 0) {
                amount = 40;
            } else {
                amount = 0;
            }
        }

        mbev_CapStatusDispSetAll(FALSE, TRUE);
        mbev_CapDuelStatusOnSet(playerNo, targetPlayer, TRUE);
        if (resourceType == 0) {
            coinTakeNum = amount;
            mbAudGuidePlay(952);
            sprintf(message, capspecialMesFormat, amount);
            mbWinCreate(2, ev_CapKettouMesGet(MESSNUM(MESS_KETTOU_MASU, 6)), mbGuideSpeakerNoGet());
            mbWinTopInsertMesSet((u32)message, 0);
            mbWinTopWait();
            for (i = 0; i < 4; i++) {
                add[i] = 0;
            }
            add[playerNo] = -amount;
            add[targetPlayer] = -amount;
            mbCoinAddAllProcExecV(add, (BOOL *)add, FALSE);
            sprintf(message, capspecialMesFormat, amount + coinTakeNum);
            mbWinCreate(2, ev_CapKettouMesGet(MESSNUM(MESS_KETTOU_MASU, 9)), mbGuideSpeakerNoGet());
            mbWinTopInsertMesSet((u32)message, 0);
            mbWinTopWait();
            memset(&mgResultData, 0, sizeof(mgResultData));
            mgResultData.playerNo1 = playerNo;
            mgResultData.playerNo2 = targetPlayer;
            mgResultData.coinNum = (s16)(amount + coinTakeNum);
        } else if (choiceStar != 0) {
            mbAudGuidePlay(952);
            mbWinCreate(2, ev_CapKettouMesGet(MESSNUM(MESS_KETTOU_MASU, 7)), mbGuideSpeakerNoGet());
            mbWinTopWait();
            mbPlayerStarAdd(playerNo, -1);
            mbPlayerStarAdd(targetPlayer, -1);
            starObj[0] = mbStarDispPlayerCreate(playerNo, -1);
            HuPrcVSleep();
            starObj[1] = mbStarDispPlayerCreate(targetPlayer, -1);
            do {
                HuPrcVSleep();
            } while (!mbStarDispCheck(starObj[0])
                || !mbStarDispCheck(starObj[1]));
            mbWinCreate(2, ev_CapKettouMesGet(MESSNUM(MESS_KETTOU_MASU, 10)),
                        mbGuideSpeakerNoGet());
            mbWinTopWait();
            memset(&mgResultData, 0, sizeof(mgResultData));
            mgResultData.playerNo1 = playerNo;
            mgResultData.playerNo2 = targetPlayer;
            mgResultData.starNum = 2;
        } else {
            mbAudGuidePlay(952);
            mbWinCreate(2, ev_CapKettouMesGet(MESSNUM(MESS_KETTOU_MASU, 8)), mbGuideSpeakerNoGet());
            mbWinTopWait();
            mbCoinAddExec(playerNo, -amount);
            mbPlayerStarAdd(targetPlayer, -1);
            mbPlayerPosGet(playerNo, &cameraOfs);
            cameraOfs.y += lbl_802C4434;
            coinDisp[0] = mbCoinDispCapsuleCreate(&cameraOfs, -amount);
            starObj[1] = mbStarDispPlayerCreate(targetPlayer, -1);
            do {
                HuPrcVSleep();
            } while (!mbCoinDispKillCheck(coinDisp[0])
                || !mbStarDispCheck(starObj[1]));
            mbWinCreate(2, ev_CapKettouMesGet(MESSNUM(MESS_KETTOU_MASU, 11)),
                        mbGuideSpeakerNoGet());
            mbWinTopWait();
            memset(&mgResultData, 0, sizeof(mgResultData));
            mgResultData.playerNo1 = playerNo;
            mgResultData.playerNo2 = targetPlayer;
            mgResultData.coinNum = (s16)amount;
            mgResultData.starNum = 1;
        }

        mbev_CapDuelStatusDispSet(playerNo, targetPlayer, TRUE);
        return 1;
    }
}

/* Restores player and board state after the Kettou capsule battle scene. */
static void ev_CapKettouReturn(CAPWORK *work)
{
    extern void mbStatusDispForceSetAll(BOOL dispF);
    extern void mbGuideEnd(OMOBJ *obj, BOOL motionF);
    extern const float lbl_802C4434;
    BOOL dissolveF = FALSE;
    int playerNo = work->playerNo;
    int masuId = lbl_80265750[playerNo].masuId;
    int guideModel;
    int winner;
    int otherPlayer;
    int guidePlayer;
    int starMotion;
    int i;
    int playerNo1;
    int playerNo2;
    int playerNo3;
    int playerNo4;
    s16 coinBonus1Source;
    struct {
        s16 coinBonus7;
        s16 coinBonus6;
        s16 coinBonus5;
        s16 coinBonus4;
        s16 coinBonus3;
        s16 coinBonus2;
        s16 coinBonus1;
        int coinDisp[2];
        int starObj[2];
        char message[16];
        HuVecF coinPos;
        HuVecF pos;
        HuVecF masuPos;
    } state;
    mbev_PlayerColMasu(playerNo, masuId, TRUE);
    mbPlayerPosGet(playerNo, &state.masuPos);
    guideModel = work->eventData[0];
    guidePlayer = playerNo;
    mbPlayerMotionSet(playerNo, 1, ((1 << 30) | (1 << 0)));
    mbPlayerRotSet(playerNo, 0.0f, 0.0f, 0.0f);
    state.pos.x = state.masuPos.x;
    state.pos.y = state.masuPos.y + 200.0f;
    state.pos.z = state.masuPos.z - 50.0f;
    mbObjPosSetV(guideModel, &state.pos);
    mbObjDispSet(guideModel, TRUE);
    mbStatusDispForceSetAll(TRUE);
    state.coinPos.x = 0.0f;
    state.coinPos.y = 100.0f;
    state.coinPos.z = 0.0f;
    mbCameraPlayerViewSetFast(playerNo, 0);
    mbCameraMoveWait();

    playerNo1 = mgResultData.playerNo1;
    coinBonus1Source = lbl_80265750[playerNo1].mgCoinBonus;
    state.coinBonus1 = coinBonus1Source;
    if (state.coinBonus1 > 0) {
        playerNo2 = mgResultData.playerNo2;
        state.coinBonus2 = lbl_80265750[playerNo2].mgCoinBonus;
        state.coinBonus3 = state.coinBonus2;
        if (state.coinBonus3 == 0) {
            winner = mgResultData.playerNo1;
            goto winner_done;
        }
    }
    playerNo3 = mgResultData.playerNo2;
    state.coinBonus4 = lbl_80265750[playerNo3].mgCoinBonus;
    state.coinBonus5 = state.coinBonus4;
    if (state.coinBonus5 > 0) {
        playerNo4 = mgResultData.playerNo1;
        state.coinBonus6 = lbl_80265750[playerNo4].mgCoinBonus;
        state.coinBonus7 = state.coinBonus6;
        if (state.coinBonus7 == 0) {
            winner = mgResultData.playerNo2;
            goto winner_done;
        }
    }
    winner = -1;
winner_done:

    if (winner == mgResultData.playerNo2
        && winner != -1
        && lbl_80265750[mgResultData.playerNo1].masuId
            != lbl_80265750[mgResultData.playerNo2].masuId) {
        mbMasuPosGet(lbl_80265750[winner].masuId, &state.masuPos);
        state.pos.x = state.masuPos.x;
        state.pos.y = state.masuPos.y + 200.0f;
        state.pos.z = state.masuPos.z - 50.0f;
        mbObjPosSetV(guideModel, &state.pos);
        mbev_PlayerColMasu(
            mgResultData.playerNo2,
            lbl_80265750[mgResultData.playerNo2].masuId, TRUE);
        mbCameraPlayerViewSetFast(
            mgResultData.playerNo2, 0);
        mbCameraMoveWait();
        dissolveF = TRUE;
    }
    if (winner == mgResultData.playerNo2
        && winner != -1) {
        guidePlayer = winner;
    }

    if (work->flags._flag02) {
        mbMusPlay(0, 26, 127, 0);
    }
    mbWipeFadeIn();
    mbPauseDisableSet(FALSE);

    if (winner != -1) {
        if (mgResultData.playerNo1
            == winner) {
            otherPlayer = mgResultData.playerNo2;
        } else {
            otherPlayer = mgResultData.playerNo1;
        }
        if (mgResultData.coinNum > 0
            && mgResultData.starNum > 0) {
            mbAudGuidePlay(952);
            mbWinCreate(2, ev_CapKettouMesGet(MESSNUM(MESS_KETTOU_MASU, 15)),
                mbGuideSpeakerNoGet()); /* duel scene message resource */
            mbWinTopInsertMesSet(mbPlayerNameMesGet(winner), 0);
        } else if (mgResultData.starNum > 0) {
            mbAudGuidePlay(952);
            mbWinCreate(2, ev_CapKettouMesGet(MESSNUM(MESS_KETTOU_MASU, 14)),
                mbGuideSpeakerNoGet()); /* duel scene message resource */
            mbWinTopInsertMesSet(mbPlayerNameMesGet(winner), 0);
        } else {
            mbAudGuidePlay(952);
            mbWinCreate(2, ev_CapKettouMesGet(MESSNUM(MESS_KETTOU_MASU, 13)),
                mbGuideSpeakerNoGet()); /* duel scene message resource */
            mbWinTopInsertMesSet(mbPlayerNameMesGet(winner), 0);
            sprintf(state.message, capspecialMesFormat, mgResultData.coinNum);
            mbWinTopInsertMesSet((u32)state.message, 1);
        }
        mbWinTopPlayerDisable(guidePlayer);
        mbWinTopWait();

        if (mgResultData.coinNum != 0) {
            mbPlayerPosGet(winner, &state.coinPos);
            state.coinPos.y += lbl_802C4434;
            state.coinDisp[0] = mbCoinDispCapsuleCreate(
                &state.coinPos, mgResultData.coinNum);
            mbCoinAddExec(winner, mgResultData.coinNum);
            mbPlayerWinLoseVoicePlay(winner, 12, (573+6));
            mbPlayerMotionShiftSet(winner, 12, 0.0f, 8.0f, 0);
            mbPlayerMotionShiftSet(otherPlayer, 13, 0.0f, 8.0f, 0);
            do {
                HuPrcVSleep();
            } while (
                mbObjMotionShiftIDGet(mbPlayerObjIDGet(
                    mgResultData.playerNo1)) != -1
                || !mbPlayerMotionEndCheck(
                    mgResultData.playerNo1)
                || mbObjMotionShiftIDGet(mbPlayerObjIDGet(
                    mgResultData.playerNo2)) != -1
                || !mbPlayerMotionEndCheck(
                    mgResultData.playerNo2));
            for (i = 0; i < 4; i++) {
                mbPlayerMotionShiftSet(i, 1, 0.0f, 8.0f,
                    ((1 << 30) | (1 << 0)));
            }
            do {
                HuPrcVSleep();
            } while (!mbCoinDispKillCheck(state.coinDisp[0]));
        }
        if (mgResultData.starNum != 0) {
            mbPlayerStarAdd(winner, mgResultData.starNum);
            state.starObj[0] = mbStarDispPlayerCreate(
                winner, mgResultData.starNum);
            mbPlayerWinLoseVoicePlay(winner, 7, (573+0));
            mbPlayerMotionShiftSet(winner, 7, 0.0f, 8.0f, 0);
            starMotion = mbev_CapPlayerMotionCreate(
                &work->objWork, otherPlayer, DATANUM(0x930000, 40));
            mbPlayerMotionShiftSet(otherPlayer, starMotion,
                0.0f, 8.0f, 0);
            do {
                HuPrcVSleep();
            } while (
                mbObjMotionShiftIDGet(mbPlayerObjIDGet(
                    mgResultData.playerNo1)) != -1
                || !mbPlayerMotionEndCheck(
                    mgResultData.playerNo1)
                || mbObjMotionShiftIDGet(mbPlayerObjIDGet(
                    mgResultData.playerNo2)) != -1
                || !mbPlayerMotionEndCheck(
                    mgResultData.playerNo2)
                || !mbStarDispCheck(state.starObj[0]));
            for (i = 0; i < 4; i++) {
                mbPlayerMotionShiftSet(i, 1, 0.0f, 8.0f,
                    ((1 << 30) | (1 << 0)));
            }
        }
        if (dissolveF) {
            mbWipeDissolveFadeOutTime(1);
            mbPlayerPosGet(playerNo, &state.masuPos);
            state.pos.x = state.masuPos.x;
            state.pos.y = state.masuPos.y + 200.0f;
            state.pos.z = state.masuPos.z - 50.0f;
            mbObjPosSetV(guideModel, &state.pos);
            mbCameraPlayerViewSetFast(
                mgResultData.playerNo1, 0);
            mbCameraMoveWait();
            mbWipeDissolveFadeIn();
        }
    } else {
        mbAudGuidePlay(951);
        if (mgResultData.coinNum > 0
            && mgResultData.starNum > 0) {
            mbWinCreate(2, ev_CapKettouMesGet(MESSNUM(MESS_KETTOU_MASU, 18)),
                mbGuideSpeakerNoGet()); /* duel scene message resource */
        } else if (mgResultData.starNum > 0) {
            mbWinCreate(2, ev_CapKettouMesGet(MESSNUM(MESS_KETTOU_MASU, 17)),
                mbGuideSpeakerNoGet()); /* duel scene message resource */
        } else {
            mbWinCreate(2, ev_CapKettouMesGet(MESSNUM(MESS_KETTOU_MASU, 16)),
                mbGuideSpeakerNoGet()); /* duel scene message resource */
        }
        mbWinTopPlayerDisable(guidePlayer);
        mbWinTopWait();
        if (mgResultData.coinNum > 0
            && mgResultData.starNum > 0) {
            mbCoinAddExec(
                mgResultData.playerNo1,
                mgResultData.coinNum);
            mbPlayerStarAdd(
                mgResultData.playerNo2,
                mgResultData.starNum);
        } else if (mgResultData.starNum > 0) {
            mbPlayerStarAdd(
                mgResultData.playerNo1,
                mgResultData.starNum / 2);
            mbPlayerStarAdd(
                mgResultData.playerNo2,
                mgResultData.starNum / 2);
        } else {
            mbCoinAddExec(
                mgResultData.playerNo1,
                mgResultData.coinNum / 2);
            mbCoinAddExec(
                mgResultData.playerNo2,
                mgResultData.coinNum / 2);
        }
        mbPlayerWinLoseVoicePlay(playerNo, 13, (573+12));
        mbev_CapPlayerMotShiftWait(playerNo, 13, 0, TRUE);
        mbev_CapPlayerMotShiftWait(playerNo, 1,
            ((1 << 30) | (1 << 0)), TRUE);
    }

    mbev_CapPlayerMotShiftSet(guideModel, kettouMotId[2],
        ((1 << 30) | (1 << 0)), TRUE);
    mbAudGuidePlay(952);
    mbWinCreate(2, ev_CapKettouMesGet(MESSNUM(MESS_KETTOU_MASU, 19)),
        mbGuideSpeakerNoGet()); /* duel scene message resource */
    if (dissolveF) {
        mbWinTopPlayerDisable(playerNo);
    } else {
        mbWinTopPlayerDisable(guidePlayer);
    }
    mbWinTopWait();
    mbMusBoardFadeOut(0, 0, 1000, 1000, -1, FALSE);
    mbGuideEnd(work->guideObj, TRUE);
    work->guideObj = NULL;
}

/* Maps a Kettou message choice to the message used by its capsule scene. */
static int ev_CapKettouMesGet(int messNo)
{
    if (lbl_80265B70.curTime == FALSE) {
        return messNo;
    }
    return messNo + 23;
}

/* Runs the Donkey capsule event from the capsule callback table, including its
 * roulette and minigame sequence. */
void mbev_CapDonkey(void)
{
    CAPWORK *work = fn_8000DD9C()->property;
    int obj1;
    int i;
    int obj2;
    int obj3;

    mbPlayerMotionShiftSet(work->playerNo, 1, 0.0f, 8.0f,
        ((1 << 30) | (1 << 0)));
    mbev_CapWait(work);
    work->explodeObj = mbev_CapEffExplodeCreate();
    HuPrcVSleep();
    work->coinObj = mbev_CapEffCoinCreate();
    HuPrcVSleep();

    /* event model resource identifier */
    obj1 = mbev_CapObjCreate(&work->objWork, DATANUM(0xe0000, 14),
        (int *)donkeyMotTbl, FALSE, 5, FALSE);
    mbObjDispSet(obj1, FALSE);
    /* event model resource identifier */
    obj2 = mbev_CapObjCreate(&work->objWork, DATANUM(0xe0000, 33),
        NULL, FALSE, 5, FALSE);
    mbObjDispSet(obj2, FALSE);
    mbObjLayerSet(obj2, 3);
    mbev_CapObjPosSet(&work->objWork, obj2,
        lbl_80265750[work->playerNo].masuId, NULL);
    /* event model resource identifier */
    obj3 = mbev_CapObjCreate(&work->objWork, DATANUM(0xc0000, 68),
        NULL, FALSE, 5, FALSE);
    mbObjDispSet(obj3, FALSE);
    work->eventData[0] = obj1;
    work->eventData[1] = obj2;
    work->eventData[2] = obj3;

    if (!work->flags._flag03) {
        if (ev_CapDonkeyStart(work)) {
            if ((mbPlayerAllComCheck() && !GWMgComDispGet())
                || mbMgRouletteNumGet(7) <= 0) {
                for (i = 0; i < 4; i++) {
                    s16 bonus = mbRandMod(10);
                    if (!fn_8006A124(65551)) {
                        lbl_80265750[i].mgCoinBonus = bonus;
                    }
                }
                mbWipeFadeOut();
                ev_CapDonkeyCoin(work);
            } else {
                /* Donkey scene message resource */
                mbWinCreate(2, MESSNUM(MESS_DONKEY_MASU, 8), -1);
                mbWinTopWait();
                mbObjMotionShiftSet(obj1, 10, 0.0f, 8.0f, 0);
                mbev_MgCallDonkey();
            }
        }
    } else {
        ev_CapDonkeyCoin(work);
    }
    ev_CapDonkeyReturn(work);
    HuPrcEnd();
}

void mbev_CapDonkeyKill(void)
{
}

const float lbl_802C4434 = 250.0f;

/* Runs Donkey's capsule scene, including its choice and coin presentation. */
static int ev_CapDonkeyStart(CAPWORK *work)
{
    HuVecF pos;
    HuVecF playerRot;
    HuVecF objectPos;
    HuVecF cameraRot;
    HuVecF masuPos;
    HuVecF direction;
    Mtx mtx;
    OMOBJ *omObj;
    int spr[2];
    int playerNo = work->playerNo;
    int masuId = lbl_80265750[playerNo].masuId;
    int obj1 = work->eventData[0];
    int obj2 = work->eventData[1];
    int prevBank;
    int obj3 = work->eventData[2];
    int i;
    int j;
    int mode;
    int diceNo;
    int bank;
    float time;
    float value;
    float phase;
    float step;

    mbMusBoardFadeOut(0, 0, 1000, 1000, 29, FALSE);
    mbAudFXPlay(1108); /* event sound-effect resource */
    mbev_CapObjPosSet(&work->objWork, obj2, masuId, NULL);
    mbObjDispSet(obj2, TRUE);
    mbObjMotionTimeSet(obj2, 0.0f);
    mbObjMotionSpeedSet(obj2, 1.0f);
    mbMasuPosGet(masuId, &masuPos);
    objectPos = masuPos;
    pos = objectPos;
    mbObjPosSetV(obj3, &objectPos);
    mbCameraRotGet(&cameraRot);
    mbCameraMoveObj(obj3, NULL, &capsuleCameraOfs, 1500.0f, -1.0f,
        60);
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
    mbAudFXPlay(1109); /* event sound-effect resource */
    fn_8006A030(playerNo, 20, 7, 3);
    do {
        time = mbObjMotionTimeGet(obj2) / mbObjMotionMaxTimeGet(obj2);
        if (time > 1.0f) {
            time = 1.0f;
        }
        fn_80030644(mbObjModelIDGet(obj2));
        fn_80023398(mbObjModelIDGet(obj2), capTreeFook, mtx);
        pos.x = mtx[0][3];
        pos.y = mtx[1][3];
        pos.z = mtx[2][3];
        mbPlayerPosSetV(playerNo, &pos);
        objectPos.x = mtx[0][3];
        objectPos.y = mtx[1][3];
        objectPos.z = mtx[2][3];
        mbObjPosSetV(obj3, &objectPos);
        HuPrcVSleep();
    } while (time < 1.0f);
    mbMasuPosGet(masuId, &masuPos);
    PSVECSubtract(&pos, &masuPos, &direction);
    mbev_CapPlayerPosSet(&work->objWork, playerNo, masuId, &direction);
    mbAudFXPlay(1110); /* event sound-effect resource */

    omObj = fn_8003AB38(mbObjMan, -32768, 0, 0, -1, ev_CapDonkeyOMExec);
    omObj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAPWORK), 0x10000000);
    memcpy(omObj->data, work, sizeof(CAPWORK));
    omObj->work[0] = omObj->work[1] = omObj->work[2] = omObj->work[3] = 0;
    while (omObj->work[0] < 2) {
        HuPrcVSleep();
    }
    mbev_CapPlayerPosSet(&work->objWork, playerNo, -1, &direction);
    mbPlayerRotGet(playerNo, &playerRot);
    mbPlayerMotionShiftSet(playerNo, 9, 0.0f, 8.0f, 0);
    for (i = 1; i <= 30.0f; i++) {
        time = (float)i / 30.0f;
        fn_80023398(mbObjModelIDGet(obj2), capTreeFook, mtx);
        pos.x = mtx[0][3] - 80.0f * time;
        pos.y = mtx[1][3] + 100.0
            * sin((3.141592653589793 * (180.0f * time)) / 180.0) * 1.5;
        pos.z = mtx[2][3] + 80.0f * time;
        mbPlayerPosSetV(playerNo, &pos);
        playerRot.y = mbev_CapAngleLerp(135.0f, playerRot.y,
            *((const float *)&lbl_802C4370));
        mbPlayerRotSetV(playerNo, &playerRot);
        HuPrcVSleep();
    }
    while (omObj->work[2] == 0) {
        fn_80023398(mbObjModelIDGet(obj2), capTreeFook, mtx);
        pos.x = mtx[0][3] - 80.0f;
        pos.y = mtx[1][3];
        pos.z = mtx[2][3] + 80.0f;
        mbPlayerPosSetV(playerNo, &pos);
        HuPrcVSleep();
    }
    omObj->work[3] = 1;
    mbMasuPosGet(masuId, &masuPos);
    PSVECSubtract(&pos, &masuPos, &direction);
    mbev_CapPlayerPosSet(&work->objWork, playerNo, masuId, &direction);
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f,
        ((1 << 30) | (1 << 0)));

    if (MBCapsuleEffRandF() < 0.3f) {
        mode = 0;
    } else {
        mode = 1;
    }
    mbev_CapPlayerMotShiftSet(obj1, 6, 0, TRUE);
    mbObjMotionShiftSet(obj1, 1, 0.0f, 8.0f, ((1 << 30) | (1 << 0)));
    mbWinCreate(2, MESSNUM(MESS_DONKEY_MASU, 0), -1); /* Donkey scene message resource */
    mbWinTopWait();
    mbAudFXPlay(936); /* event sound-effect resource */
    mbObjMotionShiftSet(obj1, 10, 0.0f, 8.0f, 0);
    HuPrcSleep(30);
    mbAudFXPlay(1117); /* event sound-effect resource */
    {
        int sprNo;
        sprNo = mbev_CapSprCreate(&work->objWork,
            mbBoardDataNumGet(donkeyMgFile[mode]), 100, (s16)(mode ^ 1));
        for (i = 1; i < 60.0f; i++) {
            time = (float)i / 60.0f;
            value = sin((3.141592653589793 * (90.0f * time)) / 180.0);
            espPosSet(sprNo,
                288.0f + 50.0f * cos(
                    (3.141592653589793 * (90.0f * time)) / 180.0),
                240.0f - 250.0f * sin(
                    (3.141592653589793 * (180.0f * time)) / 180.0));
            espScaleSet(sprNo, value, value);
            espZRotSet(sprNo, 3.0f * (360.0f * -time));
            HuPrcVSleep();
        }
        for (i = 1; i < 60.0f; i++) {
            time = (float)i / 60.0f;
            value = 1.0f + 0.2f * sin(
                (3.141592653589793 * (720.0f * time)) / 180.0);
            espPosSet(sprNo, 288.0f, 240.0f);
            espScaleSet(sprNo, value, value);
            espZRotSet(sprNo, 0.0f);
            HuPrcVSleep();
        }
        for (i = 1; i < 18.0f; i++) {
            time = (float)i / 18.0f;
            value = 1.0f + 5.0f * sin(
                (3.141592653589793 * (90.0f * time)) / 180.0);
            espPosSet(sprNo, 288.0f, 240.0f);
            espScaleSet(sprNo, value, value);
            espTPLvlSet(sprNo, 1.0f
                - sin((3.141592653589793 * (90.0f * time)) / 180.0));
            HuPrcVSleep();
        }
        espDispOff(sprNo);
    }
    mbObjMotionShiftSet(obj1, 1, 0.0f, 8.0f, ((1 << 30) | (1 << 0)));

    {
        int coinNo;
        int frameCount;
        int boardNo;
        HuVecF coinPos;
        HuVecF vel;
        char message[16];

    if (!mode) {
        mbWinCreate(2, MESSNUM(MESS_DONKEY_MASU, 1), -1); /* Donkey scene message resource */
        mbWinTopWait();
        mbWinCreate(2, MESSNUM(MESS_DONKEY_MASU, 2), -1); /* Donkey scene message resource */
        mbWinTopWait();
        mbPlayerRotateStart(playerNo, 0, 15);
        while (!mbPlayerRotateCheck(playerNo)) {
            HuPrcVSleep();
        }
        {
            boardNo = lbl_80265B70.boardNo;
            if (boardNo != 3) {
                diceNo = mbRandMod(5);
            } else {
                diceNo = mbRandMod(4);
            }
        }
        diceNo = mbDiceExec(playerNo, 11, (s8 *)donkeyDiceTbl,
            diceNo, TRUE, TRUE, NULL, 0);
        if (donkeyDiceResultTbl[diceNo] >= 0) {
            sprintf(message, capspecialMesFormat, donkeyDiceResultTbl[diceNo]);
            mbWinCreate(2, MESSNUM(MESS_DONKEY_MASU, 3), -1); /* Donkey scene message resource */
            mbWinTopInsertMesSet((u32)message, 0);
            mbWinTopWait();
            mbDiceFadeSet(playerNo);
            mbAudFXPlay(935); /* event sound-effect resource */
            mbev_CapPlayerMotShiftSet(obj1, 10, 0, TRUE);
            mbPlayerPosGet(playerNo, &direction);
            for (i = 0; i < donkeyDiceResultTbl[diceNo]; i++) {
                coinPos = direction;
                coinPos.y += 600.0f;
                coinPos.x += (MBCapsuleEffRandF()
                    + *((const float *)&lbl_802C436C))
                    * 100.0f * 0.5f;
                vel.x = vel.y = vel.z = 0.0f;
                coinNo = mbev_CapEffCoinAdd(work->coinObj, &coinPos,
                    &vel, 0.75f, 4.9f, 30, 4);
                if (coinNo >= 0) {
                    mbev_CapEffCoinMaxYSet(work->coinObj, coinNo,
                        direction.y + 150.0f);
                }
                HuPrcVSleep();
            }
            while (mbev_CapEffCoinNumGet(work->coinObj) > 0) {
                HuPrcVSleep();
            }
            mbObjMotionShiftSet(obj1, 1, 0.0f, 8.0f,
                ((1 << 30) | (1 << 0)));
            mbCoinAddDispExec(playerNo, donkeyDiceResultTbl[diceNo], FALSE,
                TRUE);
            mbev_CapCoinDisp(playerNo, donkeyDiceResultTbl[diceNo], TRUE,
                TRUE);
        } else {
            mbWinCreate(2, MESSNUM(MESS_DONKEY_MASU, 4), -1); /* Donkey scene message resource */
            mbWinTopWait();
            mbMusPauseFadeOut(0, TRUE, -1);
            mbDiceFadeSet(playerNo);
            mbAudFXPlay(935); /* event sound-effect resource */
            mbev_CapObjMotionSet(obj1, 30, 10, 1,
                0, ((1 << 30) | (1 << 0)), TRUE, TRUE);
            mbStarGetExec(playerNo);
            mbCameraMoveObj(obj3, &cameraRot, &capsuleCameraOfs,
                1500.0f, -1.0f, 60);
            mbMusPlay(0, 29, 127, 0);
        }
        mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f,
            ((1 << 30) | (1 << 0)));
        return 0;
    }

    mbWinCreate(2, MESSNUM(MESS_DONKEY_MASU, 5), -1); /* Donkey scene message resource */
    mbWinTopWait();
    mbWinCreate(2, MESSNUM(MESS_DONKEY_MASU, 6), -1); /* Donkey scene message resource */
    mbWinTopWait();
    mbAudFXPlay(936); /* event sound-effect resource */
    mbev_CapPlayerMotShiftSet(obj1, 8, 0, TRUE);
    /* event sprite resource identifier */
    spr[0] = mbev_CapSprCreate(&work->objWork, DATANUM(0xe0000, 34), 100, 0);
    espPosSet(spr[0], 288.0f, 240.0f);
    espScaleSet(spr[0], 0.0f, 0.0f);
    /* event sprite resource identifier */
    spr[1] = mbev_CapSprCreate(&work->objWork, DATANUM(0xe0000, 35), 100, 0);
    espPosSet(spr[1], 304.0f, 240.0f);
    espScaleSet(spr[1], 0.0f, 0.0f);
    espAttrSet(spr[1], 1);
    espBankSet(spr[1], 0);
    for (i = 0; i <= 30.0f; i++) {
        time = (float)i / 30.0f;
        value = sin((3.141592653589793 * (90.0f * time)) / 180.0)
            + sin((3.141592653589793 * (180.0f * time)) / 180.0);
        for (j = 0; j < 2; j++) {
            espScaleSet(spr[j], value, value);
        }
        espPosSet(spr[1], 288.0f + 16.0f * value, 240.0f);
        HuPrcVSleep();
    }
    frameCount = (int)(60.0f + MBCapsuleEffRandF() * 60.0f * 0.5f);
    phase = 0.0f;
    step = 0.3f;
    bank = prevBank = 0;
    for (i = 0; i <= frameCount; i++) {
        phase += step;
        if (phase >= *((const float *)&lbl_802C4370)) {
            phase -= *((const float *)&lbl_802C4370);
        }
        bank = (int)phase;
        if (bank >= 10) {
            bank = 9;
        }
        espBankSet(spr[1], (s16)donkeyRouletteBankTbl[bank]);
        if (bank != prevBank) {
            mbAudFXPlay(1009); /* event sound-effect resource */
        }
        prevBank = bank;
        HuPrcVSleep();
    }
    for (i = 0; i <= 60; i++) {
        if (step > 0.02f) {
            step -= 0.005f;
        }
        phase += step;
        if (phase >= *((const float *)&lbl_802C4370)) {
            phase -= *((const float *)&lbl_802C4370);
        }
        bank = (int)phase;
        if (bank >= 10) {
            bank = 9;
        }
        espBankSet(spr[1], (s16)donkeyRouletteBankTbl[bank]);
        if (bank != prevBank) {
            mbAudFXPlay(1009); /* event sound-effect resource */
        }
        prevBank = bank;
        HuPrcVSleep();
    }
    mbAudFXPlay(1134); /* event sound-effect resource */
    for (i = 0; i <= 60; i++) {
        time = (float)i / 60.0f;
        time = 1.0f + 0.5f * sin(
            (3.141592653589793 * (720.0f * time)) / 180.0);
        espScaleSet(spr[1], time, time);
        espPosSet(spr[1], 288.0f + 16.0f * time, 240.0f);
        HuPrcVSleep();
    }
    for (i = 0; i <= 30.0f; i++) {
        time = (float)i / 30.0f;
        value = 1.0f + 7.0f * sin(
            (3.141592653589793 * (90.0f * time)) / 180.0);
        for (j = 0; j < 2; j++) {
            espScaleSet(spr[j], value, value);
            espTPLvlSet(spr[j], 1.0f
                - sin((3.141592653589793 * (90.0f * time)) / 180.0));
        }
        espPosSet(spr[1], 288.0f + 16.0f * value, 240.0f);
        HuPrcVSleep();
    }
    for (j = 0; j < 2; j++) {
        espDispOff(spr[j]);
    }
    mbObjMotionShiftSet(obj1, 1, 0.0f, 8.0f, ((1 << 30) | (1 << 0)));
    sprintf(message, capspecialMesFormat, donkeyRouletteBankTbl[bank] + 1);
    mbWinCreate(2, MESSNUM(MESS_DONKEY_MASU, 7), -1); /* Donkey scene message resource */
    mbWinTopInsertMesSet((u32)message, 0);
    mbWinTopWait();
    memset(&mgResultData, 0, sizeof(mgResultData));
    mgResultData.playerNo1 = (s16)playerNo;
    mgResultData.coinNum =
        (s16)(donkeyRouletteBankTbl[bank] + 1);
    if (!fn_8006A124(65551)) {
        lbl_80265750[0].mgCoinBonus = 0;
    }
    if (!fn_8006A124(65551)) {
        lbl_80265750[1].mgCoinBonus = 0;
    }
    if (!fn_8006A124(65551)) {
        lbl_80265750[2].mgCoinBonus = 0;
    }
    if (!fn_8006A124(65551)) {
        lbl_80265750[3].mgCoinBonus = 0;
    }
        return 1;
        }
}

/* Presents and applies the coin result selected during Donkey's capsule scene. */
static void ev_CapDonkeyCoin(CAPWORK *work)
{
    HuVecF targetPos;
    HuVecF pos;
    HuVecF objectPos;
    HuVecF masuPos;
    HuVecF direction;
    HuVecF coinPos;
    HuVecF vel;
    Mtx mtx;
    int playerNo = work->playerNo;
    int masuId = lbl_80265750[playerNo].masuId;
    int obj1 = work->eventData[0];
    int obj2 = work->eventData[1];
    int obj3 = work->eventData[2];
    int coinVals[4];
    int i;
    int coinNo;

    mbStatusDispForceSetAll(TRUE);
    mbev_CapObjPosSet(&work->objWork, obj2, masuId, NULL);
    mbObjDispSet(obj2, TRUE);
    mbObjMotionTimeSet(obj2, 0.0f);
    mbObjMotionSpeedSet(obj2, 1.0f);
    mbMasuPosGet(masuId, &masuPos);
    targetPos = pos = masuPos;
    mbObjPosSetV(obj3, &pos);
    mbObjMotionTimeSet(obj2, mbObjMotionMaxTimeGet(obj2));
    fn_80030644(mbObjModelIDGet(obj2));
    HuPrcVSleep();
    fn_80023398(mbObjModelIDGet(obj2), capTreeFook, mtx);
    targetPos.x = mtx[0][3] - 80.0f;
    targetPos.y = mtx[1][3];
    targetPos.z = mtx[2][3] + 80.0f;
    pos.x = mtx[0][3];
    pos.y = mtx[1][3];
    pos.z = mtx[2][3];
    mbObjPosSetV(obj3, &pos);
    mbev_PlayerColMasu(playerNo, lbl_80265750[playerNo].masuId, TRUE);
    mbMasuPosGet(masuId, &masuPos);
    PSVECSubtract(&targetPos, &masuPos, &direction);
    mbPlayerRotSet(playerNo, 0.0f, 0.0f, 0.0f);
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
    mbev_CapPlayerPosSet(&work->objWork, playerNo, masuId, &direction);
    mbObjDispSet(obj1, TRUE);
    mbObjRotSet(obj1, 0.0f, -45.0f, 0.0f);
    mbObjMotionSet(obj1, 1, ((1 << 30) | (1 << 0)));
    objectPos.x = mtx[0][3] + 80.0f;
    objectPos.y = mtx[1][3];
    objectPos.z = mtx[2][3] - 80.0f;
    mbObjPosSetV(obj1, &objectPos);
    mbMasuPosGet(masuId, &masuPos);
    PSVECSubtract(&objectPos, &masuPos, &direction);
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
        sprintf(message, capspecialMesFormat, mgResultData.coinNum);
        mbWinCreate(2, MESSNUM(MESS_DONKEY_MASU, 9), -1); /* Donkey scene message resource */
        mbWinTopInsertMesSet((u32)message, 0);
        mbWinTopWait();
    }
    for (i = 0; i < 4; i++) {
        s16 coinBonus = lbl_80265750[i].mgCoinBonus;
        coinVals[i] = mgResultData.coinNum * coinBonus;
    }
    do {
        for (i = 0; i < 4; i++) {
            if (coinVals[i] > 0) {
                mbPlayerPosGet(i, &direction);
                coinPos = direction;
                coinPos.y += 1500.0f;
                coinPos.x += (*((const float *)&lbl_802C436C)
                                + MBCapsuleEffRandF())
                    * 100.0f * 0.5f;
                vel.x = vel.y = vel.z = 0.0f;
                coinNo = mbev_CapEffCoinAdd(work->coinObj, &coinPos, &vel,
                    0.75f, 4.9f, 30, 4);
                if (coinNo >= 0) {
                    mbev_CapEffCoinMaxYSet(work->coinObj, coinNo,
                        direction.y + 150.0f);
                    coinVals[i]--;
                }
            }
        }
        for (i = 0; i < 4; i++) {
            if (coinVals[i] > 0) {
                break;
            }
        }
        HuPrcVSleep();
    } while (i < 4
        || mbev_CapEffCoinNumGet(work->coinObj) > 0);
    for (i = 0; i < 4; i++) {
        s16 coinBonus = lbl_80265750[i].mgCoinBonus;
        coinVals[i] = mgResultData.coinNum * coinBonus;
    }
    mbCoinAddAllProcExecV(coinVals, (BOOL *)coinVals, FALSE);
    {
        s16 ownBonus = lbl_80265750[playerNo].mgCoinBonus;
        if (ownBonus > 0) {
            mbPlayerWinLoseVoicePlay(playerNo, 12, (573+6)); /* win/lose voice resource */
            mbev_CapPlayerMotShiftWait(playerNo, 12, 0, TRUE);
        } else {
            mbPlayerWinLoseVoicePlay(playerNo, 13, (573+12)); /* win/lose voice resource */
            mbev_CapPlayerMotShiftWait(playerNo, 13, 0, TRUE);
        }
    }
    mbev_CapPlayerMotShiftWait(playerNo, 1, ((1 << 30) | (1 << 0)), TRUE);
}

/* Restores player and board presentation after Donkey's capsule scene. */
static void ev_CapDonkeyReturn(CAPWORK *work)
{
    HuVecF playerPos;
    HuVecF pos;
    HuVecF objectPos;
    Mtx mtx;
    int playerNo = work->playerNo;
    int masuId = lbl_80265750[playerNo].masuId;
    int obj1 = work->eventData[0];
    int obj2 = work->eventData[1];
    int obj3 = work->eventData[2];
    int i;
    float ratio;
    float scale;
    HuVecF *dustPosP;

    mbPlayerRotateStart(playerNo, CAPSPECIAL_DONKEY_RETURN_ANGLE, 15);
    while (!mbPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    mbAudFXPlay(936); /* event sound-effect resource */
    mbev_CapPlayerMotShiftSet(obj1, CAPSPECIAL_DONKEY_RETURN_MOTION, 0, TRUE);
    mbWinCreate(2, MESSNUM(MESS_DONKEY_MASU, 10), -1); /* Donkey scene message resource */
    mbWinTopWait();
    mbObjMotionShiftSet(obj1, 1, 0.0f, 8.0f, 0);
    HuPrcSleep(20);
    mbev_CapObjPosSet(&work->objWork, obj1, -1, NULL);
    mbAudFXPlay(1135); /* event sound-effect resource */
    mbObjMotionShiftSet(obj1, 4, 0.0f, 8.0f, ((1 << 30) | (1 << 0)));

    for (i = 1; i <= 30.0f; i++) {
        ratio = (float)i / 30.0f;
        scale = 1.0f - 0.7f * sin(
            (3.141592653589793 * (90.0f * ratio)) / 180.0);
        fn_80023398(mbObjModelIDGet(obj2), capTreeFook, mtx);
        pos.x = mtx[0][3] + 80.0f;
        pos.y = mtx[1][3]
            + 100.0 * sin((3.141592653589793 * (90.0f * ratio)) / 180.0)
            + 2.5 * (100.0
                * sin((3.141592653589793 * (180.0f * ratio)) / 180.0));
        pos.z = mtx[2][3] - 80.0f;
        mbObjPosSetV(obj1, &pos);
        mbObjScaleSet(obj1, scale, scale, scale);
        HuPrcVSleep();
    }
    {
        HuVecF dustPos = pos;
        dustPosP = &dustPos;
        mbev_CapEffDustCloudAdd(work->explodeObj, dustPosP);
    }
    HuPrcVSleep();
    mbObjDispSet(obj1, FALSE);
    mbev_CapPlayerPosSet(&work->objWork, playerNo, -1, NULL);
    mbPlayerMotionShiftSet(playerNo, 2, 0.0f, 8.0f, ((1 << 30) | (1 << 0)));

    for (i = 1; i <= 30.0f; i++) {
        ratio = (float)i / 30.0f;
        fn_80023398(mbObjModelIDGet(obj2), capTreeFook, mtx);
        playerPos.x = mtx[0][3] - 80.0f * (1.0f - ratio);
        playerPos.y = mtx[1][3];
        playerPos.z = mtx[2][3] + 80.0f * (1.0f - ratio);
        mbPlayerPosSetV(playerNo, &playerPos);
        HuPrcVSleep();
    }

    mbCameraPlayerViewSet(playerNo, 0);
    mbPlayerRotateStart(playerNo, 0, 15);
    while (!mbPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    mbMusBoardFadeOut(0, 0, 1000, 1000, -1, FALSE);
    mbAudFXPlay(1142); /* event sound-effect resource */
    mbObjMotionSpeedSet(obj2, -1.0f);
    do {
        ratio = mbObjMotionTimeGet(obj2) / mbObjMotionMaxTimeGet(obj2);
        if (ratio < 0.0f) {
            ratio = 0.0f;
        }
        fn_80030644(mbObjModelIDGet(obj2));
        fn_80023398(mbObjModelIDGet(obj2), capTreeFook, mtx);
        playerPos.x = mtx[0][3];
        playerPos.y = mtx[1][3];
        playerPos.z = mtx[2][3];
        mbPlayerPosSetV(playerNo, &playerPos);
        objectPos.x = mtx[0][3];
        objectPos.y = mtx[1][3];
        objectPos.z = mtx[2][3];
        mbObjPosSetV(obj3, &objectPos);
        HuPrcVSleep();
    } while (ratio > 0.0f);
    mbMasuPosGet(masuId, &playerPos);
    mbPlayerPosSetV(playerNo, &playerPos);
    mbPlayerColSnapPlayerSet(playerNo, TRUE);
    mbCameraMoveWait();
}

/* Object-manager callback that advances the Donkey scene's board object. */
static void ev_CapDonkeyOMExec(OMOBJ *obj)
{
    extern void CharEffectHipDropCreate();
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
    float scaleA;
    float scaleB;

    if (mbExitCheck() || obj->work[3] != 0) {
        omDelObjEx(mbObjMan, obj);
        return;
    }
    playerNo = work->playerNo;
    masuId = lbl_80265750[playerNo].masuId;
    obj1 = work->eventData[0];
    obj2 = work->eventData[1];
    obj3 = work->eventData[2];

    switch (obj->work[0]) {
    case 0:
        mbObjDispSet(obj1, TRUE);
        mbObjRotSet(obj1, 0.0f, -45.0f, 0.0f);
        mbObjMotionSet(obj1, 4, 0);
        obj->work[0]++;
        obj->work[1] = 0;
        /* fall through */
    case 1:
        time = (float)++obj->work[1] / 48.0f;
        fn_80023398(mbObjModelIDGet(obj2), capTreeFook, mtx);
        pos.x = mtx[0][3] + 80.0f + 100.0f;
        pos.y = mtx[1][3] + 100.0f * cos(
            (3.141592653589793 * (90.0f * time)) / 180.0) * 10.0f;
        pos.z = mtx[2][3] - 80.0f - 100.0f;
        mbObjPosSetV(obj1, &pos);
        if (time >= 1.0f) {
            mbObjMotionShiftSet(obj1, 2, 0.0f, 8.0f,
                0);
            CharEffectHipDropCreate(
                lbl_80265750[playerNo].charNo, &pos);
            obj->work[0]++;
            obj->work[1] = 0;
        }
        break;
    case 2:
        time = (float)++obj->work[1] / 36.0f;
        nextTime = (float)(obj->work[1] + 1) / 36.0f;
        if (nextTime >= 1.0f) {
            nextTime = 1.0f;
        }
        scaleA = 1.0f + 0.33f
            * sin((3.141592653589793 * (180.0f * nextTime)) / 180.0);
        scaleB = 1.0f - 0.33f
            * sin((3.141592653589793 * (180.0f * nextTime)) / 180.0);
        mbObjScaleSet(obj2, scaleA, scaleB, scaleA);
        fn_80023398(mbObjModelIDGet(obj2), capTreeFook, mtx);
        pos.x = mtx[0][3] + 80.0f + 100.0f * cos(
            (3.141592653589793 * (90.0f * time)) / 180.0);
        pos.y = mtx[1][3] + 100.0f
            * sin((3.141592653589793 * (180.0f * time)) / 180.0) * 2.0f;
        pos.z = mtx[2][3] - 80.0f - 100.0f * cos(
            (3.141592653589793 * (90.0f * time)) / 180.0);
        mbObjPosSetV(obj1, &pos);
        if (obj->work[1] == CAPSPECIAL_DONKEY_MOTION_SHIFT_FRAME) {
            mbObjMotionShiftSet(obj1, 3, 0.0f, 8.0f,
                0);
        }
        if (time >= 1.0f) {
            fn_8005F968((s16)lbl_80265750[playerNo].charNo, &pos);
            obj->work[0]++;
            obj->work[1] = 0;
        }
        break;
    case 3:
        nextTime = (float)(++obj->work[1] + 1) / 12.0f;
        if (nextTime >= 1.0f) {
            nextTime = 1.0f;
        }
        scaleA = 1.0f + 0.1f
            * sin((3.141592653589793 * (180.0f * nextTime)) / 180.0);
        scaleB = 1.0f - 0.1f
            * sin((3.141592653589793 * (180.0f * nextTime)) / 180.0);
        mbObjScaleSet(obj2, scaleA, scaleB, scaleA);
        fn_80023398(mbObjModelIDGet(obj2), capTreeFook, mtx);
        pos.x = mtx[0][3] + 80.0f;
        pos.y = mtx[1][3];
        pos.z = mtx[2][3] - 80.0f;
        mbObjPosSetV(obj1, &pos);
        if (mbObjMotionShiftIDGet(obj1) == -1
            && mbObjMotionEndCheck(obj1) && nextTime >= 1.0f) {
            mbObjMotionShiftSet(obj1, 1, 0.0f, 8.0f,
                ((1 << 30) | (1 << 0)));
            mbMasuPosGet(masuId, &masuPos);
            PSVECSubtract(&pos, &masuPos, &direction);
            mbev_CapObjPosSet(&work->objWork, obj1, masuId, &direction);
            obj->work[0]++;
            obj->work[1] = 0;
        }
        break;
    default:
        obj->work[2] = 1;
        break;
    }
}

/* Runs the Koopa capsule event from the capsule callback table and resolves
 * the roulette result as a coin, star, or minigame penalty. */
void mbev_CapKoopa(void)
{
    CAPWORK *work = fn_8000DD9C()->property;
    int objId;
    int i;

    mbPlayerMotionShiftSet(work->playerNo, 1, 0.0f, 8.0f,
        ((1 << 30) | (1 << 0)));
    mbev_CapWait(work);
    work->explodeObj = mbev_CapEffExplodeCreate();
    HuPrcVSleep();
    work->coinObj = mbev_CapEffCoinCreate();
    HuPrcVSleep();
    /* event model resource identifier */
    objId = mbev_CapObjCreate(&work->objWork, DATANUM(0xe0000, 0),
        (int *)koopaMotTbl, FALSE, 5, FALSE);
    mbObjDispSet(objId, FALSE);
    mbPlayerColSnapPlayerSet(work->playerNo, TRUE);
    work->eventData[0] = objId;

    if (!work->flags._flag04) {
        if (ev_CapKoopaStart(work)) {
            if ((mbPlayerAllComCheck() && !GWMgComDispGet())
                || mbMgRouletteNumGet(4) <= 0) {
                for (i = 0; i < 4; i++) {
                    s16 bonus = mbRandMod(2);
                    if (!fn_8006A124(65551)) {
                        lbl_80265750[i].mgCoinBonus = bonus;
                    }
                }
                mbWipeFadeOut();
                ev_CapKoopaCoin(work);
            } else {
                mbWinCreate(2, MESSNUM(MESS_KOOPA_MASU, 13), 13); /* Koopa scene message resource */
                mbWinTopWait();
                mbObjMotionShiftSet(objId, 5, 0.0f, 8.0f, 0);
                mbev_MgCallKoopa();
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

/* Runs Koopa's capsule scene and prepares its coin result. */
static int ev_CapKoopaStart(CAPWORK *work)
{
    HuVecF playerPos;
    HuVecF masuPos;
    HuVecF cameraPos;
    int ids[4];
    int playerNo = work->playerNo;
    int masuId = lbl_80265750[playerNo].masuId;
    int objId = work->eventData[0];
    int playerMot[2];
    int spr;
    int rouletteSpr;
    int starObj;
    int squishCount;
    int diceNo;
    int value;
    int selector;
    int i;
    int mode;
    float t;
    float scale;
    char message[16];

    mbPlayerRotateStart(playerNo, 0, 15);
    playerMot[0] = mbev_CapPlayerMotionCreate(&work->objWork, playerNo,
        DATANUM(0x930000, 23)); /* event resource identifier */
    playerMot[1] = mbev_CapPlayerMotionCreate(&work->objWork, playerNo,
        DATANUM(0x930000, 25)); /* event resource identifier */
    /* event sprite resource identifier */
    spr = mbev_CapSprCreate(&work->objWork, DATANUM(0xe0000, 36), 100, 0);
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
    mbPlayerMotionShiftSet(playerNo, playerMot[0], 0.0f, 8.0f, 0);
    mbCameraPlayerViewSet(playerNo, 0);
    mbEffFadeCreate(30, 160);
    work->flags._flag06 = TRUE;
    mbMusPlay(0, 27, 127, 0);
    espDispOn((s16)spr);
    for (i = 0; i < 360; i += 2) {
        if (i == 60) {
            mbAudFXPlay(1107); /* event sound-effect resource */
        }
        if (i == 90 || i == 270 || i == 450) {
            fn_8006A030(playerNo, 20, 7, 3);
        }
        espTPLvlSet((s16)spr, CapSpecialAbsFloat((float)sin(
            (3.141592653589793 * mbAngleWrap((float)i)) / 180.0)));
        HuPrcVSleep();
    }
    espDispOff((s16)spr);
    mbPlayerMotionShiftSet(playerNo, playerMot[1], 0.0f, 8.0f, 0);
    mbPlayerPosGet(playerNo, &playerPos);
    mbObjDispSet(objId, TRUE);
    mbObjMotionSet(objId, 2, 0);
    mbObjMotionTimeSet(objId, 30.0f);
    mbObjMotionSpeedSet(objId, 1.0f);
    while (mbObjMotionTimeGet(objId) < 60.0f) {
        t = (mbObjMotionTimeGet(objId) - 30.0f) / 30.0f;
        mbMasuPosGet(masuId, &masuPos);
        masuPos.y += 100.0
            * cos((3.141592653589793 * (90.0f * t)) / 180.0) * 6.0;
        mbObjPosSetV(objId, &masuPos);
        HuPrcVSleep();
    }
    {
        HuVecF pos;
        HuVecF *dustPosP;

        pos = masuPos;
        dustPosP = &pos;
        mbev_CapEffDustHeavyAdd(work->explodeObj, dustPosP);
    }
    mbev_CapObjPosSet(&work->objWork, objId, masuId, NULL);
    mbAudFXPlay(1119); /* event sound-effect resource */
    mbev_CapVibrate(1);
    squishCount = mbev_CapPlayerSquishSet(ids, masuId);
    work->eventData[1] = squishCount;
    for (i = 0; i < 4; i++) {
        work->eventData[i + 2] = ids[i];
    }
    while (!mbObjMotionEndCheck(objId)) {
        HuPrcVSleep();
    }
    HuPrcVSleep();
    mbObjMotionShiftSet(objId, 1, 0.0f, 8.0f, ((1 << 30) | (1 << 0)));
    HuPrcSleep(30);
    mbEffFadeOutSet(30);
    HuPrcSleep(30);
    if (MBCapsuleEffRandF() < 0.3f) {
        mode = FALSE;
    } else {
        mode = TRUE;
    }
    fn_800097F4(DATANUM(0xe0000, 0)); /* event archive resource identifier */
    mbAudFXDelaySet(30);
    mbAudFXPlay(971); /* event sound-effect resource */
    mbev_CapPlayerMotShiftSet(objId, 3, 0, TRUE);
    mbObjMotionShiftSet(objId, 1, 0.0f, 8.0f, ((1 << 30) | (1 << 0)));
    mbWinCreate(2, MESSNUM(MESS_KOOPA_MASU, 0), 13); /* Koopa scene message resource */
    mbWinTopWait();
    if (mbPlayerCoinGet(playerNo) <= 0) {
        mbObjMotionShiftSet(objId, 6, 0.0f, 8.0f,
            ((1 << 30) | (1 << 0)));
        mbWinCreate(2, MESSNUM(MESS_KOOPA_MASU, 1), 13); /* Koopa scene message resource */
        mbWinTopWait();
        mbev_CapPlayerMotShiftSet(objId, 1, ((1 << 30) | (1 << 0)), TRUE);
        work->flags._flag05 = TRUE;
        return 0;
    }
    mbAudFXDelaySet(30);
    mbAudFXPlay(973); /* event sound-effect resource */
    mbObjMotionShiftSet(objId, 5, 0.0f, 8.0f, 0);
    HuPrcSleep(30);
    mbAudFXPlay(1117); /* event sound-effect resource */
    rouletteSpr = mbev_CapSprCreate(&work->objWork,
        mbBoardDataNumGet(koopaMgFile[mode]), 100, (s16)(mode ^ 1));
    for (i = 1; (float)i < 60.0f; i++) {
        t = (float)i / 60.0f;
        scale = (float)sin((3.141592653589793 * (90.0f * t)) / 180.0);
        espPosSet((s16)rouletteSpr, 288.0f,
            (float)(240.0
                - 250.0 * sin((3.141592653589793 * (180.0f * t)) / 180.0)
                + 50.0 * cos((3.141592653589793 * (180.0f * t)) / 180.0)));
        espScaleSet((s16)rouletteSpr, scale, scale);
        espZRotSet((s16)rouletteSpr, 3.0f * (360.0f * t));
        HuPrcVSleep();
    }
    for (i = 1; (float)i < 60.0f; i++) {
        t = (float)i / 60.0f;
        scale = (float)(1.0
            + 0.2f * sin((3.141592653589793 * (720.0f * t)) / 180.0));
        espPosSet((s16)rouletteSpr, 288.0f, 240.0f);
        espScaleSet((s16)rouletteSpr, scale, scale);
        espZRotSet((s16)rouletteSpr, 0.0f);
        HuPrcVSleep();
    }
    for (i = 1; (float)i < 18.0f; i++) {
        t = (float)i / 18.0f;
        scale = (float)(1.0
            + 5.0 * sin((3.141592653589793 * (90.0f * t)) / 180.0));
        espPosSet((s16)rouletteSpr, 288.0f, 240.0f);
        espScaleSet((s16)rouletteSpr, scale, scale);
        espTPLvlSet((s16)rouletteSpr,
            (float)(1.0 - sin((3.141592653589793 * (90.0f * t)) / 180.0)));
        HuPrcVSleep();
    }
    espDispOff((s16)rouletteSpr);
    if (!mode) {
        int boardNo;

        mbAudFXDelaySet(30);
        mbAudFXPlay(971); /* event sound-effect resource */
        mbObjMotionShiftSet(objId, 3, 0.0f, 8.0f,
            ((1 << 30) | (1 << 0)));
        mbWinCreate(2, MESSNUM(MESS_KOOPA_MASU, 2), 13); /* Koopa scene message resource */
        mbWinTopWait();
        mbObjMotionShiftSet(objId, 1, 0.0f, 8.0f,
            ((1 << 30) | (1 << 0)));
        mbWinCreate(2, MESSNUM(MESS_KOOPA_MASU, 3), 13); /* Koopa scene message resource */
        mbWinTopWait();
        mbObjPosGet(objId, &masuPos);
        cameraPos.x = masuPos.x;
        cameraPos.y = (masuPos.y += 50.0f);
        cameraPos.z = masuPos.z;
        diceHitTimer = (int)(60.0f * (0.33f + MBCapsuleEffRandF()));
        koopaMdlId = objId;
        boardNo = lbl_80265B70.boardNo;
        if (boardNo != 3) {
            diceNo = mbRandMod(5);
        } else {
            diceNo = mbRandMod(4);
        }
        mbDiceExec(-1, 12, (s8 *)koopaDiceTbl, diceNo,
            FALSE, FALSE, &cameraPos, 2);
        mbDicePadBtnHookSet(-1,
            (u16 (*)(int))ev_CapKoopaDicePadBtnHook);
        mbDiceMotHookSet(-1, ev_CapKoopaDiceMotHook);
        while (!mbDiceKillCheck(-1)) {
            HuPrcVSleep();
        }
        diceNo = mbDiceResultGet(-1);
        HuPrcSleep(30);
        if (koopaDiceResultTbl[diceNo] >= 0) {
            sprintf(message, capspecialMesFormat, -koopaDiceResultTbl[diceNo]);
            mbWinCreate(2, MESSNUM(MESS_KOOPA_MASU, 4), 13); /* Koopa scene message resource */
            mbWinTopInsertMesSet((u32)message, 0);
            mbWinTopWait();
            mbDiceFadeSet(-1);
            mbAudFXDelaySet(30);
            mbAudFXPlay(973); /* event sound-effect resource */
            mbev_CapPlayerMotShiftSet(objId, 5, 0, TRUE);
            if ((value = koopaDiceResultTbl[diceNo])
                > mbPlayerCoinGet(playerNo)) {
                value = mbPlayerCoinGet(playerNo);
            }
            mbCoinAddProcExec(playerNo, -value, -value, FALSE);
            mbev_CapPlayerMotShiftSet(objId, 1, ((1 << 30) | (1 << 0)), TRUE);
        } else {
            mbWinCreate(2, MESSNUM(MESS_KOOPA_MASU, 5), 13); /* Koopa scene message resource */
            mbWinTopWait();
            mbDiceFadeSet(-1);
            if (mbPlayerStarGet(playerNo) > 0) {
                mbAudFXDelaySet(30);
                mbAudFXPlay(973); /* event sound-effect resource */
                mbev_CapPlayerMotShiftSet(objId, 5, 0, TRUE);
                HuPrcSleep(12);
                starObj = mbStarDispPlayerCreate(playerNo, -1);
                while (!mbStarDispCheck(starObj)) {
                    HuPrcVSleep();
                }
                mbPlayerStarAdd(playerNo, -1);
                mbev_CapPlayerMotShiftSet(objId, 1,
                    ((1 << 30) | (1 << 0)), TRUE);
            } else {
                mbAudFXDelaySet(30);
                mbAudFXPlay(973); /* event sound-effect resource */
                mbObjMotionShiftSet(objId, 6, 0.0f, 8.0f,
                    ((1 << 30) | (1 << 0)));
                mbWinCreate(2, MESSNUM(MESS_KOOPA_MASU, 6), 13); /* Koopa scene message resource */
                mbWinTopWait();
                mbev_CapPlayerMotShiftSet(objId, 1,
                    ((1 << 30) | (1 << 0)), TRUE);
            }
        }
        return 0;
    }

    mbAudFXDelaySet(30);
    mbAudFXPlay(971); /* event sound-effect resource */
    mbObjMotionShiftSet(objId, 3, 0.0f, 8.0f,
        ((1 << 30) | (1 << 0)));
    mbWinCreate(2, MESSNUM(MESS_KOOPA_MASU, 7), 13); /* Koopa scene message resource */
    mbWinTopWait();
    selector = mbRandMod(3);
    mbAudFXDelaySet(30);
    mbAudFXPlay(973); /* event sound-effect resource */
    mbObjMotionShiftSet(objId, 5, 0.0f, 8.0f, 0);
    if (!GWTeamFGet()) {
        mbWinCreate(2, MESSNUM(MESS_KOOPA_MASU, 8), 13); /* Koopa scene message resource */
    } else {
        mbWinCreate(2, MESSNUM(MESS_KOOPA_MASU, 9), 13); /* Koopa scene message resource */
    }
    mbWinTopInsertMesSet(koopaLoseMesTbl[selector], 0);
    mbWinTopWait();
    mbev_CapPlayerMotShiftSet(objId, 1, ((1 << 30) | (1 << 0)), TRUE);
    memset(&mgResultData, 0, sizeof(mgResultData));
    mgResultData.resultNo = (s16)selector;
    if (!fn_8006A124(65551)) {
        lbl_80265750[0].mgCoinBonus = 0;
    }
    if (!fn_8006A124(65551)) {
        lbl_80265750[1].mgCoinBonus = 0;
    }
    if (!fn_8006A124(65551)) {
        lbl_80265750[2].mgCoinBonus = 0;
    }
    if (!fn_8006A124(65551)) {
        lbl_80265750[3].mgCoinBonus = 0;
    }
    return 1;
}

/* Presents and applies the coin result during Koopa's capsule scene. */
static int ev_CapKoopaCoin(CAPWORK *work)
{
    extern void mbStatusDispForceSetAll(BOOL dispF);
    extern void mbPauseDisableSet(BOOL disableF);
    HuVecF masuPos;
    int add[4];
    int ids[4];
    int teamTbl[2][4];
    int teamCount[2];
    BOOL teamLose[2];
    int playerNo = work->playerNo;
    int masuId = lbl_80265750[playerNo].masuId;
    int objId = work->eventData[0];
    int squishCount;
    int resultType;
    int loseCount;
    BOOL hasResource;
    int i;
    int t;
    BOOL removed;

    mbev_PlayerColMasu(playerNo, masuId, TRUE);
    squishCount = mbev_CapPlayerSquishVoiceSet(ids, masuId, TRUE);
    work->eventData[1] = squishCount;
    for (i = 0; i < 4; i++) {
        work->eventData[i + 2] = ids[i];
    }
    mbMasuPosGet(masuId, &masuPos);
    mbObjPosSetV(objId, &masuPos);
    mbObjDispSet(objId, TRUE);
    mbObjMotionSet(objId, 1, ((1 << 30) | (1 << 0)));
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

    resultType = mgResultData.resultNo;
    if (!GWTeamFGet()) {
        for (i = 0, loseCount = 0; i < 4; i++) {
            if (GWMgCoinBonusGet(i) <= 0) {
                loseCount++;
            }
        }
        if (resultType == 2) {
            for (i = 0, hasResource = TRUE; i < 4; i++) {
                if (GWMgCoinBonusGet(i) <= 0 && mbPlayerCapsuleNumGet(i) > 0) {
                    hasResource = FALSE;
                }
            }
        } else {
            for (i = 0, hasResource = TRUE; i < 4; i++) {
                if (GWMgCoinBonusGet(i) <= 0 && mbPlayerCoinGet(i) > 0) {
                    hasResource = FALSE;
                }
            }
        }
    } else {
        teamCount[0] = teamCount[1] = 0;
        for (i = 0; i < 4; i++) {
            if (mbPlayerGrpGet(i) == 0) {
                teamTbl[0][teamCount[0]] = i;
                teamCount[0]++;
            } else {
                teamTbl[1][teamCount[1]] = i;
                teamCount[1]++;
            }
        }
        teamLose[0] = teamLose[1] = loseCount = 0;
        if (GWMgCoinBonusGet(teamTbl[0][0]) <= 0
            && GWMgCoinBonusGet(teamTbl[0][1]) <= 0) {
            teamLose[0] = TRUE;
            loseCount++;
        }
        if (GWMgCoinBonusGet(teamTbl[1][0]) <= 0
            && GWMgCoinBonusGet(teamTbl[1][1]) <= 0) {
            teamLose[1] = TRUE;
            loseCount++;
        }
        if (resultType == 2) {
            for (i = 0, hasResource = TRUE; i < 2; i++) {
                if (teamLose[i]
                    && mbPlayerCapsuleNumGet(teamTbl[i][0]) > 0) {
                    hasResource = FALSE;
                }
            }
        } else {
            for (i = 0, hasResource = TRUE; i < 2; i++) {
                if (teamLose[i] && mbPlayerCoinGet(teamTbl[i][0]) > 0) {
                    hasResource = FALSE;
                }
            }
        }
    }
    mbAudFXDelaySet(30);
    mbAudFXPlay(971); /* event sound-effect resource */
    mbObjMotionShiftSet(objId, 3, 0.0f, 8.0f, ((1 << 30) | (1 << 0)));
    if (!GWTeamFGet()) {
        mbWinCreate(2, MESSNUM(MESS_KOOPA_MASU, 14), 13); /* Koopa scene message resource */
    } else {
        mbWinCreate(2, MESSNUM(MESS_KOOPA_MASU, 15), 13); /* Koopa scene message resource */
    }
    mbWinTopInsertMesSet(koopaLoseMesTbl2[resultType], 0);
    mbWinTopWait();
    mbAudFXDelaySet(30);
    mbAudFXPlay(973); /* event sound-effect resource */
    mbev_CapPlayerMotShiftSet(objId, 5, 0, TRUE);
    mbAudFXPlay(1063); /* event sound-effect resource */
    if (loseCount == 0) {
        HuPrcSleep(30);
        mbAudFXPlay(972); /* event sound-effect resource */
        mbObjMotionShiftSet(objId, 6, 0.0f, 8.0f,
            ((1 << 30) | (1 << 0)));
        if (!GWTeamFGet()) {
            mbWinCreate(2, MESSNUM(MESS_KOOPA_MASU, 18), 13); /* Koopa scene message resource */
        } else {
            mbWinCreate(2, MESSNUM(MESS_KOOPA_MASU, 19), 13); /* Koopa scene message resource */
        }
        mbWinTopWait();
    } else if (hasResource) {
        HuPrcSleep(30);
        mbAudFXPlay(972); /* event sound-effect resource */
        mbObjMotionShiftSet(objId, 6, 0.0f, 8.0f,
            ((1 << 30) | (1 << 0)));
        if (resultType == 2) {
            mbWinCreate(2, MESSNUM(MESS_KOOPA_MASU, 17), 13); /* Koopa scene message resource */
        } else {
            mbWinCreate(2, MESSNUM(MESS_KOOPA_MASU, 16), 13); /* Koopa scene message resource */
        }
        mbWinTopWait();
    } else if (!GWTeamFGet()) {
        switch (resultType) {
        case 0:
            for (i = 0; i < 4; i++) {
                if (GWMgCoinBonusGet(i) <= 0) {
                    add[i] = -((mbPlayerCoinGet(i) + 1) / 2);
                    fn_8006A030(i, 20, 20, 0);
                } else {
                    add[i] = 0;
                }
            }
            mbCoinAddAllProcExecV(add, (BOOL *)add, FALSE);
            break;
        case 1:
            for (i = 0; i < 4; i++) {
                if (GWMgCoinBonusGet(i) <= 0) {
                    add[i] = -mbPlayerCoinGet(i);
                    fn_8006A030(i, 20, 20, 0);
                } else {
                    add[i] = 0;
                }
            }
            mbCoinAddAllProcExecV(add, (BOOL *)add, FALSE);
            break;
        default:
            for (i = 0; i < 4; i++) {
                if (GWMgCoinBonusGet(i) <= 0) {
                    fn_8006A030(i, 20, 20, 0);
                }
            }
            for (i = 0; i < mbPlayerCapsuleMaxGet(); i++) {
                for (t = 0, removed = FALSE; t < 4; t++) {
                    if (GWMgCoinBonusGet(t) <= 0) {
                        mbPlayerCapsuleRemove(t, 0);
                        removed = TRUE;
                    }
                }
                if (removed) {
                    HuPrcSleep(10);
                }
            }
            break;
        }
    } else {
        switch (resultType) {
        case 0:
            for (i = 0; i < 2; i++) {
                if (teamLose[i]) {
                    mbCoinAddDispExec(teamTbl[i][0],
                        -(mbPlayerCoinGet(teamTbl[i][0]) / 2),
                        FALSE, FALSE);
                    fn_8006A030(teamTbl[i][0], 20, 20, 0);
                    fn_8006A030(teamTbl[i][1], 20, 20, 0);
                }
            }
            break;
        case 1:
            for (i = 0; i < 2; i++) {
                if (teamLose[i]) {
                    mbCoinAddDispExec(teamTbl[i][0],
                        -mbPlayerCoinGet(teamTbl[i][0]), FALSE, FALSE);
                    fn_8006A030(teamTbl[i][0], 20, 20, 0);
                    fn_8006A030(teamTbl[i][1], 20, 20, 0);
                }
            }
            break;
        default:
            for (i = 0; i < 2; i++) {
                if (teamLose[i]) {
                    fn_8006A030(teamTbl[i][0], 20, 20, 0);
                    fn_8006A030(teamTbl[i][1], 20, 20, 0);
                }
            }
            for (i = 0; i < mbPlayerCapsuleMaxGet(); i++) {
                for (t = 0, removed = FALSE; t < 2; t++) {
                    if (teamLose[t]) {
                        mbPlayerCapsuleRemove(teamTbl[t][0], 0);
                        removed = TRUE;
                    }
                }
                if (removed) {
                    HuPrcSleep(10);
                }
            }
            break;
        }
    }
    mbev_CapPlayerMotShiftSet(objId, 1, ((1 << 30) | (1 << 0)), TRUE);
    return 0;
}

/* Restores player and board state after Koopa's capsule scene. */
static int ev_CapKoopaReturn(CAPWORK *work)
{
    HuVecF masuPos;
    int ids[4];
    int playerNo = work->playerNo;
    int masuId = lbl_80265750[playerNo].masuId;
    int objId = work->eventData[0];
    int count = work->eventData[1];
    int i;
    float time;

    for (i = 0; i < 4; i++) {
        ids[i] = work->eventData[i + 2]; /* Player IDs saved by the squish step. */
    }
    mbAudFXDelaySet(30);
    mbAudFXPlay(971); /* event sound-effect resource */
    mbev_CapPlayerMotShiftSet(objId, 3, 0, TRUE);
    mbObjMotionShiftSet(objId, 1, 0.0f, 8.0f, ((1 << 30) | (1 << 0)));
    mbWinCreate(2, MESSNUM(MESS_KOOPA_MASU, 20), 13); /* Koopa scene message resource */
    mbWinTopWait();
    mbMusFadeOutSpeed(0, 1000);
    while (mbMusCheck(0)) {
        HuPrcVSleep();
    }
    mbAudFXDelaySet(30);
    mbAudFXPlay(974); /* event sound-effect resource */
    mbObjMotionShiftSet(objId, 2, 0.0f, 8.0f, 0);
    while (mbObjMotionShiftIDGet(objId) != -1
        || mbObjMotionTimeGet(objId) <= 25.0f) {
        HuPrcVSleep();
    }
    mbev_CapObjPosSet(&work->objWork, objId, -1, NULL);
    for (i = 1; i <= 24.0f; i++) {
        time = (float)i / 24.0f;
        mbMasuPosGet(masuId, &masuPos);
        masuPos.y += sin((3.141592653589793 * (90.0f * time)) / 180.0)
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
            ((1 << 30) | (1 << 0)));
    }
    if (work->flags._flag05) {
        mbCoinAddProcExec(playerNo, 10, 1, FALSE);
    }
    return 0;
}

/* Supplies an A-button press after Koopa's dice-hit timer expires. */
static u16 ev_CapKoopaDicePadBtnHook(void)
{
    if (--diceHitTimer <= 0) {
        return PAD_BUTTON_A;
    }
    return 0;
}

/* Dice motion hook that advances Koopa's animation and triggers the dice hit during his roll. */
static void ev_CapKoopaDiceMotHook(int playerNo)
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
        mbObjMotionSet(koopaMdlId, 1, ((1 << 30) | (1 << 0)));
    }
}
