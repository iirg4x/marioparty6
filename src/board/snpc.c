#include <string.h>
#include <stdlib.h>

#include "dolphin/math.h"
#include "game/board/main.h"
#include "game/board/masu.h"
#include "game/board/object.h"
#include "game/board/audio.h"
#include "game/board/effect.h"
#include "game/board/player.h"

#include "game/data.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/sprite.h"
#include "messdir_enum.h"
#include "msm_se.h"

#include "humath.h"

extern inline float fabsf(float value)
{
    return __fabsf(value);
}

static inline float SNpcAbsFloat(register float value)
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

#define SNPC_MAGIC 'SNPC'
#define MBOBJ_FADE_WORK_MAGIC 'MBTV'
#define MBOBJ_METAL_WORK_MAGIC 'TV01'
#define MBOBJ_BIRIQ_WORK_MAGIC 'TV02'

#define SNPC_DATA_FADE_TEXTURE DATANUM(DATA_board, 103)
#define SNPC_DATA_METAL_TEXMAP4 DATANUM(DATA_board, 105)
#define SNPC_DATA_METAL_TEXMAP5 DATANUM(DATA_board, 104)

typedef struct MBSNPCSAVEWORK {
    u8 isKoopa : 1;
    u8 unusedFlags : 7;
    u8 masuId;
    u8 effectMissCount;
} MBSNPCSAVEWORK;

typedef struct SNPCMOTDATA {
    int dataNum;
    float speed;
    u16 loopF;
    u8 startFrame;
    u8 endFrame;
} SNPCMOTDATA;

typedef struct SNPCMOTNEXTDATA {
    s16 motNo;
    s16 frame[3];
} SNPCMOTNEXTDATA;

typedef struct MBSNPCWORK {
    u32 dataNum;
    u32 unk04;
    int motNo;
    BOOL unk0C;
    int motShiftNo;
    HuVecF pos;
    HuVecF rot;
    OMOBJ *rotateObj;
    OMOBJ *diceNumObj;
    OMOBJ *zoomObj;
    OMOBJ *starObj;
    OMOBJ *moveObj;
    MBMODELID objId[8];
    MBMODELID motionId[19];
    s16 pathTbl[273];
} MBSNPCWORK;

typedef struct SNPCZOOMWORK {
    u8 initF : 1;
    u8 killF : 1;
    u8 unk00 : 6;
    u8 unk01;
    s16 time;
    s16 maxTime;
    s16 unk06;
    float startZoom;
    float targetZoom;
} SNPCZOOMWORK;

typedef struct SNPCROTATEWORK {
    u8 initF : 1;
    u8 killF : 1;
    u8 unk00 : 6;
    u8 unk01;
    s16 time;
    s16 maxTime;
    s16 unk06;
    float targetAngle;
} SNPCROTATEWORK;

typedef struct SNPCSTAREFFWORK {
    u8 initF : 1;
    u8 killF : 1;
    u8 loopF : 1;
    u8 unk00 : 1;
    u8 unk01 : 4;
    s16 time;
    s16 maxTime;
    s16 unk06;
    s16 effectNo;
    int soundId;
} SNPCSTAREFFWORK;

typedef struct SNPCMOVEWORK {
    u8 killF : 1;
    u8 mode : 2;
    u8 playerNo : 2;
    u8 initF : 1;
    u8 unk00 : 2;
    s16 time;
    s16 maxTime;
} SNPCMOVEWORK;

typedef struct SNPCMOVEOMWORK {
    u8 initF : 1;
    u8 killF : 1;
    u8 mode : 2;
    u8 playerNo : 2;
    u8 unk00 : 2;
    s16 time;
    s16 maxTime;
    s16 unk06;
    s16 unk08;
    s16 masuId;
} SNPCMOVEOMWORK;

typedef struct SNPCCHANCEPATH {
    s16 masuId;
    s16 linkNo;
    s16 chance;
    s16 unk06;
} SNPCCHANCEPATH;

typedef struct SNPCCHANCEINDEX {
    s16 start;
    s16 count;
} SNPCCHANCEINDEX;

typedef struct MBOBJFADEWORK {
    u32 magic;
    ANIMDATA *anim;
    HuVecF pos;
    HuVecF rot;
    float alpha;
    GXColor color;
} MBOBJFADEWORK;

typedef struct MBOBJMETALWORK {
    u32 magic;
    ANIMDATA *anim[2];
    float tpLvl;
    GXColor shadowColor;
    GXColor hiliteColor;
} MBOBJMETALWORK;

typedef struct MBOBJBIRIQWORK {
    u32 magic;
    int mode;
    float level;
    GXColor color;
} MBOBJBIRIQWORK;

typedef struct MBOBJBIRIQTEV {
    u8 op;
    u8 outReg;
    u8 input[4];
} MBOBJBIRIQTEV;

static HuVecF masuViewOfs = { 0.0f, 100.0f, 0.0f };

static u32 snpcMesTbl[6][2] = {
    MESSNUM(MESS_BOARD_SNPC, 0), MESSNUM(MESS_BOARD_SNPC, 5),
    MESSNUM(MESS_BOARD_SNPC, 1), MESSNUM(MESS_BOARD_SNPC, 6),
    MESSNUM(MESS_BOARD_SNPC, 4), 0, MESSNUM(MESS_BOARD_SNPC, 9),
    MESSNUM(MESS_BOARD_SNPC, 11), MESSNUM(MESS_BOARD_SNPC, 10),
    MESSNUM(MESS_BOARD_SNPC, 12), MESSNUM(MESS_BOARD_SNPC, 2),
    MESSNUM(MESS_BOARD_SNPC, 7),
};

static u32 snpcStarMesTbl[5][2] = {
    { MESSNUM(MESS_BOARD_SNPC, 13), MESSNUM(MESS_BOARD_SNPC, 18) },
    { MESSNUM(MESS_BOARD_SNPC, 14), MESSNUM(MESS_BOARD_SNPC, 19) },
    { MESSNUM(MESS_BOARD_SNPC, 16), MESSNUM(MESS_BOARD_SNPC, 21) },
    { MESSNUM(MESS_BOARD_SNPC, 17), MESSNUM(MESS_BOARD_SNPC, 22) },
    { MESSNUM(MESS_BOARD_SNPC, 15), MESSNUM(MESS_BOARD_SNPC, 20) },
};

static int snpcEffectChanceTbl[4] = { 30, 55, 80, 95 };

static int snpcDiceTypeTbl[2][2] = {
    { 16, 17 },
    { 18, 18 },
};

static HuVecF snpcDiceOfsTbl[2] = {
    { 0.0f, 90.0f, 0.0f },
    { 0.0f, 90.0f, 0.0f },
};

char lbl_80247724[12] = "itemhook_M";

static GXColor texCol[16];

static MBSNPCWORK *snpcWork;
static MBSNPCSAVEWORK *snpcSaveWork;
static u32 snpcMagic;

#define SNPC_MOTION_NUM 11
#define SNPC_OBJECT_NUM 8
#define SNPC_MOTION_SLOT_NUM 19
#define SNPC_CHANCE_TBL_SIZE MASU_MAX
#define SNPC_CHANCE_BRANCH_PADDING 16
#define SNPC_CHANCE_DISTANCE_BYTES (SNPC_CHANCE_TBL_SIZE + sizeof(float) - 2)
#define SNPC_CHANCE_MASU_BYTES ((SNPC_CHANCE_TBL_SIZE + sizeof(float) - 2) * sizeof(u32))

#define SNPC_DONKEY_DATA_MODEL DATANUM(DATA_capsulechar1, 31)
#define SNPC_KOOPA_DATA_MODEL DATANUM(DATA_capsulechar1, 0)
#define SNPC_DONKEY_EFFECT_DATA_MODEL DATANUM(DATA_capsulechar1, 30)
#define SNPC_EFFECT_DATA_MODEL DATANUM(DATA_board, 92)
#define SNPC_KOOPA_FIRE_DATA_ANIM DATANUM(DATA_board, 102)

static int snpcMesSpeakerTbl[2] = { 6, 13 };
static int playerStarMotNoTbl[2] = { 7, 8 };
static int snpcStarStrmTbl[2] = { 30, 28 };
static int playerStarSeTbl[2] = { MSM_SE_CHARVOICE_MARIO, 585 };
static int snpcStarMesSpeakerTbl[2] = { 6, 13 };
static int snpcMoveStrmTbl[2] = { 30, 28 };
static int playerStarChgMotNoTbl[2] = { 7, 8 };
static int playerStarChgSeTbl[2] = { MSM_SE_CHARVOICE_MARIO, 585 };
static float snpcRotSpeedTbl[2] = { 1.3f, 1.8f };
static float snpcPosFixSpeedTbl[2] = { 1.3f, 1.8f };
static int snpcMoveNumColor[2] = { 1, 2 };
static int snpcDiceMotTimeTbl[2] = { 25, 25 };

enum {
    SNPC_MASU_TYPE_DONKEY = 7,
    SNPC_MASU_TYPE_KOOPA = 10,
    SNPC_MASU_SE_DONKEY = 1527,
    SNPC_MASU_SE_KOOPA = 1528,
    SNPC_MASU_RESET_SE = 1529,
};

enum {
    SNPC_MOVE_OBJ_PRIORITY = 256,
    SNPC_CHANCE_EXCLUDE = 100,
    SNPC_FLAG_KOOPA = 128,
    SNPC_STAR_OBJ_GROUP = 4,
    SNPC_STAR_PARTICLE_ATTR = 100,
    SNPC_STAR_PARTICLE_MAX = 100,
};

static void SNpcStarFunc(void);
static void GetStarTexTevStage(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material,
    int *tevStageNum, int *texGenNum);
static void GetStarNoTexTevStage(HU3D_DRAW_OBJ *drawObj,
    HSF_MATERIAL *material, int *tevStageNum, int *texGenNum);
static void FadeMatHook(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material);
static void MetalMatHook(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material);
static void BiriQMatHook(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material);
static void SNpcObjCreate(void);
static void SNpcObjKill(void);
static void SNpcObjDispSet(BOOL dispF);
static void SNpcObjPosSet(float x, float y, float z);
static void SNpcObjPosSetV(const HuVecF *pos);
static void SNpcObjPosGet(HuVecF *pos);
static void SNpcObjRotSet(float x, float y, float z);
static void SNpcObjRotSetV(const HuVecF *rot);
static void SNpcObjRotGet(HuVecF *rot);
static void SNpcObjMotSet(int motNo);
static void SNpcObjMotShiftSet(int motNo);
static BOOL SNpcObjMotEndCheck(void);
static void SNpcObjMotEndWait(void);
static void SNpcObjMasuSet(int masuId);
static void SNpcMasuEffDispSet(void);
static void SNpcTargetAngleSet(float angle);
static void SNpcRotateWait(void);
static void SNpcRotateUpdate(OMOBJ *obj);
static void SNpcPosFixCreate(void);
static void SNpcPosFixSnap(void);
static void SNpcPosFixUpdate(OMOBJ *obj);
static void SNpcMasuSet(int masuId, BOOL setF);
static void SNpcMasuReset(BOOL setF);
static int SNpcMasuStarNextGet(BOOL playerF);
static void SNpcMotSetNext(void);
static void SNpcZoomSet(float zoom);
static BOOL SNpcZoomCheck(void);
static void SNpcZoomWait(void);
static void SNpcStarWait(void);
static void SNpcSePlay(int seNo, int unused);
static void SNpcZoomUpdate(OMOBJ *obj);
static u16 SNpcDiceBtnHook(int playerNo);
static void SNpcDiceMotHook(int playerNo);
static void SNpcPlayerMoveFunc(int playerNo);
static void SNpcPlayerMoveObjExec(OMOBJ *obj);
static void SNpcStarObjExec(OMOBJ *obj);
static void SNpcStarCreate(int type, BOOL loopF, HuVecF *pos);
static HU3D_MODELID SNpcStarEffCreate(ANIMDATA *anim, int type);
static void SNpcStarEffKill(s16 parManId);
static BOOL SNpcMoveExec(void);
static void SNpcEffectExec(void);
static void SNpcKoopaFireExec(void);
static void SNpcKoopaFireHook(HU3D_MODEL *modelP, MBPARTICLE *particleP,
    Mtx mtx);
static void SNpcKoopaFire2Hook(HU3D_MODEL *modelP, MBPARTICLE *particleP,
    Mtx mtx);
static void SNpcKoopaFire3Hook(HU3D_MODEL *modelP, MBPARTICLE *particleP,
    Mtx mtx);
static void StarChangeExec(s8 *playerNoTbl, int starF, int amount);
static int MasuNextGet(int masuId, int linkNo);
static void SNpcMoveOMExec(OMOBJ *obj);
static OMOBJ *SNpcDiceExec(int diceValue, int diceNum);
static int SNpcDiceValueGet(int masuId, int diceNum, const int *typeTbl,
    u32 mAttr, s16 *pathTbl, float chance);

extern void *mbMallocNum(s32 size, u32 num);
extern void *mbMalloc(s32 size);
extern void mbMtxRot(Mtx mtx, float x, float y, float z);
extern float mbAngleLerp(float from, float to, float weight);
extern BOOL mbBranchAttrCheck(int masuId);
extern s16 mbCapMasuDispTypeGet(s16 masuId);
extern float Hu3DMotionShiftMaxTimeGet(HU3D_MODELID modelId);
extern void Hu3DMotionShiftStartEndSet(HU3D_MODELID modelId, float start,
    float end);
extern void HuPrcVSleep(void);
extern float mbCameraZoomGet(void);
extern void mbCameraZoomSet(float zoom);
extern int mbSNpcMasuStarNextGet(int masuId, int type, int *linkNoTbl,
    u32 attr);
extern void mbDiceObjHit(int playerNo);
extern void mbPlayerMoveHookSet(int playerNo, void (*hook)(int playerNo));
extern void mbWipeSpecialFadeInCreate(int type, int time);
extern void mbWipeSpecialFadeOutCreate(int type, int time);
extern void mbDiceSNpcNumKill(OMOBJ *obj);
extern void mbDiceSNpcNumDispSet(OMOBJ *obj, BOOL dispF);
extern void mbDiceSNpcNumSet(OMOBJ *obj, int value);
extern OMOBJ *mbDiceSNpcNumCreate(int playerNo, HuVecF *pos);
extern void mbDiceSNpcNumPosSet(OMOBJ *obj, HuVecF *pos);
extern int mbDiceProcExec(int playerNo, int diceType, s8 *valueTbl,
    int *tutorialVal, BOOL padWinF, BOOL waitF, HuVecF *pos, int color);
extern void mbDicePadBtnHookSet(int playerNo,
    u16 (*hook)(int playerNo));
extern void mbDiceMotHookSet(int playerNo, void (*hook)(int playerNo));
extern BOOL mbDiceKillCheck(int playerNo);
extern void mbCameraFocusObjSet(MBMODELID modelId);
extern void mbCameraOffsetSet(float offsetX, float offsetY, float offsetZ);
extern void mbCameraMoveOnSet(BOOL moveOn);
extern void mbCapMasuDispSet(BOOL dispF);
extern void mbTelopCreate(int playerNo, int mess, int time);
extern void mbMoveNumDispSet(int playerNo, BOOL dispF);
extern void mbPlayerMotionEndWait(int playerNo);
extern void mbPlayerMotIdleSet(int playerNo);
extern void mbPlayerPosGet(int playerNo, HuVecF *pos);
extern void mbPlayerPosSetV(int playerNo, const HuVecF *pos);
extern void mbPlayerRotGet(int playerNo, HuVecF *rot);
extern void mbPlayerRotSet(int playerNo, float rotX, float rotY, float rotZ);
extern void mbPlayerMotionSet(int playerNo, int motNo, u32 attr);
extern int mbPlayerCoinGet(int playerNo);
extern int mbPlayerStarGet(int playerNo);
extern void mbPlayerStarAdd(int playerNo, int starNum);
extern void mbPlayerWinLoseVoicePlay(int playerNo, int motNo, int seId);
extern int mbPlayerTeamFindPlayer(int teamNo, int memberNo);
extern int mbCoinAddDispExec(int playerNo, int coinNum, BOOL dispF,
    BOOL fastF);
extern int mbCoinAddProcExec(int playerNo, int coinNum, BOOL dispF,
    BOOL fastF);
extern void mbCameraShakeSet(int time, float strength);
extern void mbStarAddAllProcExecV(int *addNum, BOOL *dispF, BOOL fastF);
extern void mbCoinAddAllProcExecV(int *addNum, BOOL *dispF, BOOL fastF);
extern int mbStarAddProcExec(int playerNo, int starNum, BOOL dispF,
    BOOL fastF);
extern int mbMusJinglePlay(s16 id);
extern void mbMusJingleWait(int streamNo);
extern void mbMusPauseFadeOut(int chan, BOOL pauseF, int speed);
extern BOOL mbMusBoardFadeOut(int chan, int nextChan, int speed,
    int fadeSpeed, int streamNo, BOOL waitF);
extern int mbWinCreate(int type, u32 mess, int speakerNo);
extern int mbWinCreateChoice(int type, u32 mess, int speakerNo, int choiceNo);
extern int mbWinChoiceGet(s16 winNo);
extern void mbWinPause(s16 winNo);
extern void mbWinKill(s16 winNo);
extern void mbWinPlayerDisable(s16 winNo, int playerNo);
extern void mbWinWait(s16 winNo);
extern int mbGuideSpeakerNoGet(void);
extern void mbComChoiceUpSet(void);
extern int mbCameraStackPush(void);
extern void mbCameraStackPop(int maxTime);
extern void mbCameraMoveMasu(s16 masuId, HuVecF *rot, HuVecF *offset,
    float zoom, float fov, int maxTime);
extern void mbCameraMovePlayer(s16 playerNo, HuVecF *rot, HuVecF *offset,
    float zoom, float fov, s16 maxTime);
extern void mbCameraMoveWait(void);
extern float mbCameraPlayerViewZoomGet(int viewNo);
extern void mbWipeDissolveFadeIn(void);
extern void mbWipeDissolveFadeOut(void);
extern float mbSinDeg(float deg);
extern float mbAngleWrap2(float angle, float startAngle);
extern const HuVecF lbl_8021AA28;
extern const HuVecF lbl_8021AA64;
extern BOOL mbSaveNewF;
extern void mbStarMasuFuncSet(void (*func)(void));

static int snpcSeTbl[8][2] = {
    { 0, 0 },
    { MSM_SE_GUIDE_11, MSM_SE_GUIDE_47 },
    { MSM_SE_GUIDE_13, MSM_SE_GUIDE_48 },
    { MSM_SE_BRD00_131, MSM_SE_GUIDE_50 },
    { MSM_SE_GUIDE_09, MSM_SE_GUIDE_45 },
    { MSM_SE_GUIDE_10, MSM_SE_GUIDE_46 },
    { MSM_SE_GUIDE_69, MSM_SE_GUIDE_69 },
    { MSM_SE_GUIDE_71, MSM_SE_BRD00_115 },
};

static SNPCMOTNEXTDATA snpcDonkeyNextMotTbl[11] = {
    { 0, { -1, -1, -1 } },
    { 4, { 2, 17, -1 } },
    { 5, { 2, 17, -1 } },
    { 0, { -1, -1, -1 } },
    { 6, { 0, -1, -1 } },
    { 7, { 2, -1, -1 } },
    { 0, { -1, -1, -1 } },
    { 0, { -1, -1, -1 } },
    { 1, { 0, -1, -1 } },
    { 2, { 0, -1, -1 } },
    { 0, { -1, -1, -1 } },
};

static SNPCMOTNEXTDATA snpcKoopaNextMotTbl[11] = {
    { 0, { -1, -1, -1 } },
    { 4, { 6, 30, -1 } },
    { 5, { 6, 30, -1 } },
    { 0, { -1, -1, -1 } },
    { 6, { 0, -1, -1 } },
    { 7, { 2, -1, -1 } },
    { 0, { -1, -1, -1 } },
    { 0, { -1, -1, -1 } },
    { 1, { 0, -1, -1 } },
    { 2, { 0, -1, -1 } },
    { 0, { -1, -1, -1 } },
};

static SNPCMOTDATA snpcDonkeyMotTbl[11] = {
    { DATANUM(DATA_capsulechar1, 15), 1.0f, 1, 0, 0 },
    { DATANUM(DATA_capsulechar1, 16), 1.0f, 1, 0, 0 },
    { DATANUM(DATA_capsulechar1, 16), 1.0f, 1, 0, 0 },
    { DATANUM(DATA_capsulechar1, 17), 1.0f, 0, 0, 5 },
    { DATANUM(DATA_capsulechar1, 17), 1.0f, 0, 6, 0 },
    { DATANUM(DATA_capsulechar1, 18), 1.0f, 0, 0, 0 },
    { DATANUM(DATA_capsulechar1, 23), 1.0f, 0, 0, 0 },
    { DATANUM(DATA_capsulechar1, 26), 1.0f, 0, 0, 0 },
    { DATANUM(DATA_capsulechar1, 19), 1.0f, 0, 0, 0 },
    { DATANUM(DATA_capsulechar1, 25), 1.0f, 0, 0, 0 },
    { DATANUM(DATA_capsulechar1, 28), 1.0f, 0, 0, 0 },
};

static SNPCMOTDATA snpcKoopaMotTbl[11] = {
    { DATANUM(DATA_capsulechar1, 1), 1.0f, 1, 0, 0 },
    { DATANUM(DATA_capsulechar1, 6), 1.2f, 1, 0, 0 },
    { DATANUM(DATA_capsulechar1, 6), 1.2f, 1, 0, 0 },
    { DATANUM(DATA_capsulechar1, 5), 1.0f, 0, 0, 20 },
    { DATANUM(DATA_capsulechar1, 5), 1.0f, 0, 21, 40 },
    { DATANUM(DATA_capsulechar1, 5), 1.0f, 0, 50, 0 },
    { DATANUM(DATA_capsulechar1, 7), 1.0f, 0, 0, 0 },
    { DATANUM(DATA_capsulechar1, 4), 1.0f, 0, 0, 0 },
    { DATANUM(DATA_capsulechar1, 3), 1.0f, 0, 0, 0 },
    { DATANUM(DATA_capsulechar1, 10), 1.0f, 0, 0, 0 },
    { DATANUM(DATA_capsulechar1, 11), 1.0f, 0, 0, 0 },
};

static HU3D_PARMAN_PARAM snpcStarEffParam[2] = {
    {
        30,
        0,
        3.3f,
        70.0f,
        7.0f,
        { 0.0f, -0.05f, 0.0f },
        2.0f,
        1.0f,
        30.0f,
        0.98f,
        2,
        {
            { 255, 255, 255, 255 },
            { 255, 255, 64, 255 },
            { 0, 0, 0, 0 },
            { 0, 0, 0, 0 },
        },
        {
            { 255, 128, 128, 0 },
            { 255, 64, 32, 0 },
            { 0, 0, 0, 0 },
            { 0, 0, 0, 0 },
        },
    },
    {
        30,
        0,
        3.3f,
        70.0f,
        7.0f,
        { 0.0f, -0.05f, 0.0f },
        2.0f,
        1.0f,
        30.0f,
        0.98f,
        2,
        {
            { 144, 144, 144, 255 },
            { 160, 144, 176, 255 },
            { 0, 0, 0, 0 },
            { 0, 0, 0, 0 },
        },
        {
            { 16, 16, 16, 0 },
            { 32, 0, 48, 0 },
            { 0, 0, 0, 0 },
            { 0, 0, 0, 0 },
        },
    },
};

static GXTevKColorSel kColorTbl[8] = {
    GX_TEV_KCSEL_8_8,
    GX_TEV_KCSEL_7_8,
    GX_TEV_KCSEL_6_8,
    GX_TEV_KCSEL_5_8,
    GX_TEV_KCSEL_4_8,
    GX_TEV_KCSEL_3_8,
    GX_TEV_KCSEL_2_8,
    GX_TEV_KCSEL_1_8,
};

static int biriQMatNumTbl[4] = {
    2,
    1,
    1,
    2,
};

static MBOBJBIRIQTEV biriQMatTbl[4][2][2] = {
    {
        { { GX_TEV_ADD, GX_TEVREG0, { GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST } },
            { GX_TEV_ADD, GX_TEVREG0, { GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST } } },
        { { GX_TEV_ADD, GX_TEVPREV, { GX_CC_CPREV, GX_CC_C0, GX_CC_A0, GX_CC_ZERO } },
            { GX_TEV_ADD, GX_TEVPREV, { GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV } } },
    },
    {
        { { GX_TEV_ADD, GX_TEVPREV, { GX_CC_KONST, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV } },
            { GX_TEV_ADD, GX_TEVPREV, { GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV } } },
        { { GX_TEV_ADD, GX_TEVPREV, { GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV } },
            { GX_TEV_ADD, GX_TEVPREV, { GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV } } },
    },
    {
        { { GX_TEV_SUB, GX_TEVPREV, { GX_CC_KONST, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV } },
            { GX_TEV_ADD, GX_TEVPREV, { GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV } } },
        { { GX_TEV_ADD, GX_TEVPREV, { GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV } },
            { GX_TEV_ADD, GX_TEVPREV, { GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV } } },
    },
    {
        { { GX_TEV_SUB, GX_TEVREG0, { GX_CC_CPREV, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ONE } },
            { GX_TEV_ADD, GX_TEVREG0, { GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST } } },
        { { GX_TEV_ADD, GX_TEVPREV, { GX_CC_CPREV, GX_CC_C0, GX_CC_A0, GX_CC_ZERO } },
            { GX_TEV_ADD, GX_TEVPREV, { GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV } } },
    },
};

static const SNPCMOTNEXTDATA *snpcNextMotTbl[2] = {
    snpcDonkeyNextMotTbl,
    snpcKoopaNextMotTbl,
};

static const SNPCMOTDATA *snpcMotTbl[2] = {
    snpcDonkeyMotTbl,
    snpcKoopaMotTbl,
};

static int starMdlTbl[2] = {
    DATANUM(DATA_board, 5),
    DATANUM(DATA_capsulechar1, 32),
};

void mbSNpcInit(void)
{
    snpcMagic = 0;
    snpcSaveWork = NULL;
    snpcWork = NULL;
}

void mbSNpcCreate(MBSNPCSAVEWORK *saveWork, u32 dataNum)
{
    int isKoopa;

    snpcSaveWork = saveWork;
    snpcWork = mbMalloc(sizeof(MBSNPCWORK));
    snpcWork->dataNum = dataNum;
    if (mbSaveNewF) {
        snpcSaveWork->masuId = (u8)mbMasuFind_AttrIdGet(MASU_NULL,
            MASU_FLAG_START);
        snpcSaveWork->masuId = (u8)SNpcMasuStarNextGet(TRUE);
        snpcSaveWork->effectMissCount = 0;
    }
    isKoopa = (GwSystem.curTime == 0);
    snpcSaveWork->isKoopa = isKoopa ? 0 : 1;
    SNpcMasuSet(snpcSaveWork->masuId, FALSE);
    SNpcObjCreate();
    mbPlayerEndTurnHookSet(0, SNpcMoveExec);
    mbPlayerEndTurnHookSet(1, SNpcMoveExec);
    mbPlayerEndTurnHookSet(2, SNpcMoveExec);
    mbPlayerEndTurnHookSet(3, SNpcMoveExec);
    mbStarMasuFuncSet(SNpcStarFunc);
    snpcMagic = SNPC_MAGIC;
}

void mbSNpcKill(void)
{
    MBSNPCWORK *work;

    if (snpcMagic == SNPC_MAGIC) {
        mbPlayerEndTurnHookSet(GW_PLAYER_MAX - 1, NULL);
        SNpcObjKill();
        work = snpcWork;
        HuMemDirectFree(work);
        snpcWork = NULL;
        snpcSaveWork = NULL;
        snpcMagic = 0;
    }
}

int mbSNpcMasuGet(void)
{
    if (snpcMagic != SNPC_MAGIC) {
        return 0;
    }
    return snpcSaveWork->masuId;
}

void mbSNpcMotWinSet(void)
{
    if (snpcMagic == SNPC_MAGIC) {
        SNpcSePlay(1, 0);
        SNpcObjMotShiftSet(8);
    }
}

void mbSNpcMotIdleSet(void)
{
    if (snpcMagic == SNPC_MAGIC) {
        SNpcObjMotShiftSet(0);
    }
}

void mbSNpcDispSet(BOOL dispF)
{
    if (snpcMagic == SNPC_MAGIC) {
        SNpcObjDispSet(dispF);
    }
}

static void SNpcStarFunc(void)
{
}

void mbSNpcMotReset(void)
{
    if (snpcMagic == SNPC_MAGIC) {
        SNpcPosFixSnap();
        SNpcObjMotSet(0);
    }
}

void mbSNpcPlayerWalkSet(int playerNo, int masuId)
{
    if (snpcMagic == SNPC_MAGIC && masuId == snpcSaveWork->masuId) {
        mbPlayerMoveHookSet(playerNo, SNpcPlayerMoveFunc);
    }
}

void mbSNpcStarExec(int playerNo, int masuId)
{
    HuVecF playerPos;
    HuVecF npcPos;
    HuVecF playerRot;
    float angle;
    int streamNo;
    int zoom;
    int isKoopa;
    int state;
    int winNo;
    int nextMasu;
    BOOL moveStarF;
    int i;
    int loopMax;
    int jingleNo;
    int unused;

    unused = -1;
    if (snpcMagic != SNPC_MAGIC) {
        return;
    }
    if (masuId != snpcSaveWork->masuId) {
        return;
    }

    mbCameraStackPush();
    mbMoveNumDispSet(playerNo, FALSE);
    mbCameraMoveMasu((s16)masuId, NULL, &masuViewOfs,
        mbCameraPlayerViewZoomGet(0), -1.0f, 24);

    isKoopa = snpcSaveWork->isKoopa;
    state = 0;
    if (!isKoopa) {
        if (mbPlayerCoinGet(playerNo) >= 20) {
            winNo = mbWinCreateChoice(2, snpcMesTbl[0][isKoopa],
                snpcMesSpeakerTbl[isKoopa], 0);
            if (GwPlayer[playerNo].comF) {
                mbComChoiceUpSet();
            }
            state = 1;
        } else {
            winNo = mbWinCreate(2, snpcMesTbl[1][isKoopa],
                snpcMesSpeakerTbl[isKoopa]);
        }
    } else if (mbPlayerStarGet(playerNo) > 0) {
        winNo = mbWinCreate(2, snpcMesTbl[0][isKoopa],
            snpcMesSpeakerTbl[isKoopa]);
        state = 2;
    } else {
        winNo = mbWinCreate(2, snpcMesTbl[1][isKoopa],
            snpcMesSpeakerTbl[isKoopa]);
        state = 3;
    }
    SNpcSePlay(1, 0);

    mbPlayerPosGet(playerNo, &playerPos);
    SNpcObjPosGet(&npcPos);
    PSVECSubtract(&npcPos, &playerPos, &npcPos);
    PSVECNormalize(&npcPos, &npcPos);
    SNpcTargetAngleSet((float)(180.0
        * (atan2(-npcPos.x, -npcPos.z) / 3.141592653589793)));
    loopMax = 7;
    PSVECScale(&npcPos, &npcPos, -100.0f / (float)loopMax);

    mbPlayerPosGet(playerNo, &playerPos);
    mbPlayerRotGet(playerNo, &playerRot);
    playerRot.z = mbAngleWrap2((float)(180.0
        * (atan2(-npcPos.x, -npcPos.z) / 3.141592653589793)), playerRot.y);
    mbPlayerMotionSet(playerNo, 9, HU3D_MOTATTR_NONE);
    for (i = 0; i < loopMax; i++) {
        angle = (float)sin((3.141592653589793
            * (90.0f * (float)(i + 1) / (float)loopMax))
            / 180.0);
        mbPlayerRotSet(playerNo, 0.0f,
            playerRot.y + (angle * playerRot.z), 0.0f);
        HuPrcVSleep();
    }
    streamNo = snpcStarStrmTbl[isKoopa];
    mbMusPlay(0, streamNo, 127, 0);
    while (!mbPlayerMotionEndCheck(playerNo)) {
        HuPrcVSleep();
    }
    mbPlayerMotIdleSet(playerNo);
    SNpcRotateWait();

    moveStarF = TRUE;
    switch (state) {
    case 0:
        mbWinWait((s16)winNo);
        winNo = mbWinCreate(2, snpcMesTbl[5][isKoopa],
            snpcMesSpeakerTbl[isKoopa]);
        SNpcSePlay(2, 0);
        SNpcObjMotShiftSet(9);
        mbWinWait((s16)winNo);
        mbPlayerWinLoseVoicePlay(playerNo, 13, 585);
        mbPlayerMotionShiftSet(playerNo, 13, 0.0f,
            8.0f, HU3D_MOTATTR_NONE);
        mbPlayerMotionEndWait(playerNo);
        moveStarF = FALSE;
        break;
    case 1:
        mbWinWait((s16)winNo);
        if (mbWinChoiceGet((s16)winNo) == 0) {
            jingleNo = -1;
            mbCoinAddDispExec(playerNo, -20, FALSE, TRUE);
            mbPlayerPosGet(playerNo, &playerPos);
            SNpcStarCreate(isKoopa, TRUE, &playerPos);
            mbPlayerRotateStart(playerNo, 0, 15);
            SNpcObjMotShiftSet(7);
            HuPrcSleep(150);
            SNpcTargetAngleSet(0.0f);
            mbPlayerMotionShiftSet(playerNo, 11, 0.0f,
                8.0f, HU3D_MOTATTR_NONE);
            mbMusPauseFadeOut(0, TRUE, 1000);
            mbPlayerMotionEndWait(playerNo);
            mbPlayerMotIdleSet(playerNo);
            SNpcStarWait();
            omVibrate((s16)playerNo, 20, 7, 3);
            mbPlayerStarAdd(playerNo, 1);
            mbPlayerWinLoseVoicePlay(playerNo, playerStarMotNoTbl[isKoopa],
                playerStarSeTbl[isKoopa]);
            mbPlayerMotionShiftSet(playerNo, playerStarMotNoTbl[isKoopa],
                0.0f, 8.0f, HU3D_MOTATTR_NONE);
            SNpcSePlay(1, 0);
            SNpcObjMotShiftSet(8);
            jingleNo = mbMusJinglePlay(39);
            SNpcObjMotEndWait();
            mbPlayerMotionEndWait(playerNo);
            HuPrcSleep(60);
            mbMusJingleWait(jingleNo);
            mbMusPauseFadeOut(0, FALSE, 1000);
            mbPlayerMotIdleSet(playerNo);
        } else {
            SNpcSePlay(2, 0);
            SNpcObjMotShiftSet(9);
            winNo = mbWinCreate(2, snpcMesTbl[2][isKoopa],
                snpcMesSpeakerTbl[isKoopa]);
            mbWinWait((s16)winNo);
        }
        break;
    case 2:
    case 3:
        mbWinWait((s16)winNo);
        if (state == 3) {
            winNo = mbWinCreate(2, snpcMesTbl[5][isKoopa],
                snpcMesSpeakerTbl[isKoopa]);
            mbWinWait((s16)winNo);
        }
        mbPlayerPosGet(playerNo, &playerPos);
        SNpcStarCreate(isKoopa, TRUE, &playerPos);
        SNpcObjMotShiftSet(7);
        SNpcStarWait();
        omVibrate((s16)playerNo, 20, 7, 3);
        SNpcTargetAngleSet(0.0f);
        if (state == 2) {
            mbStarAddProcExec(playerNo, -1, -1, TRUE);
        } else {
            mbCoinAddProcExec(playerNo, -20, -1, TRUE);
        }
        mbPlayerWinLoseVoicePlay(playerNo, playerStarMotNoTbl[isKoopa],
            playerStarSeTbl[isKoopa]);
        mbPlayerMotionShiftSet(playerNo, playerStarMotNoTbl[isKoopa],
            0.0f, 8.0f, HU3D_MOTATTR_NONE);
        SNpcSePlay(1, 0);
        SNpcObjMotShiftSet(8);
        SNpcObjMotEndWait();
        mbPlayerMotionEndWait(playerNo);
        break;
    default:
        break;
    }

    if (moveStarF) {
        nextMasu = SNpcMasuStarNextGet(FALSE);
        SNpcObjMotShiftSet(3);
        SNpcObjMotEndWait();
        if (snpcSaveWork->isKoopa == TRUE) {
            mbCameraShakeSet(12, 50.0f);
        }
        SNpcSePlay(3, 0);
        SNpcObjMotShiftSet(4);
        SNpcObjPosGet(&playerPos);
        loopMax = 30;
        for (i = 0; i < loopMax; i++) {
            angle = (float)i / (float)loopMax;
            angle = (float)sin((3.141592653589793
                * (80.0f * angle)) / 180.0);
            SNpcObjPosSet(playerPos.x,
                playerPos.y + 100.0f * (8.0f * angle),
                playerPos.z);
            HuPrcVSleep();
        }
        SNpcObjDispSet(FALSE);
        SNpcMasuReset(TRUE);
        mbCameraMovePlayer(-1, NULL, NULL,
            mbCameraPlayerViewZoomGet(2), -1.0f, 24);
        mbCameraMoveWait();
        mbMasuPosGet(nextMasu, &playerPos);
        mbMasuPosGet(masuId, &npcPos);
        PSVECSubtract(&npcPos, &playerPos, &npcPos);
        angle = PSVECMag(&npcPos) * 0.00066666666f;
        zoom = (s16)(u32)(angle * 60.0f);
        mbCameraMoveMasu((s16)nextMasu, NULL, NULL,
            mbCameraPlayerViewZoomGet(2), -1.0f, zoom);
        winNo = mbWinCreate(2, snpcMesTbl[3][isKoopa],
            mbGuideSpeakerNoGet());
        mbWinPause((s16)winNo);
        mbCameraMoveWait();
        mbWinKill((s16)winNo);
        mbCameraMoveMasu(-1, NULL, NULL,
            mbCameraPlayerViewZoomGet(0), -1.0f, 21);
        SNpcObjDispSet(TRUE);
        mbMasuPosGet(nextMasu, &playerPos);
        SNpcObjRotSet(0.0f, 0.0f, 0.0f);
        loopMax = 30;
        for (i = 0; i < loopMax; i++) {
            angle = (float)(loopMax - i - 1) / (float)loopMax;
            angle = (float)sin((3.141592653589793
                * (80.0f * angle)) / 180.0);
            SNpcObjPosSet(playerPos.x,
                playerPos.y + 100.0f * (8.0f * angle),
                playerPos.z);
            if (i + 5 == loopMax) {
                SNpcSePlay(7, 0);
                SNpcObjMotShiftSet(5);
            }
            HuPrcVSleep();
        }
        if (snpcSaveWork->isKoopa == TRUE) {
            mbCameraShakeSet(18, 200.0f);
        }
        SNpcObjPosSetV(&playerPos);
        SNpcMasuSet(nextMasu, TRUE);
        SNpcObjMotEndWait();
        winNo = mbWinCreate(2, snpcMesTbl[4][isKoopa],
            snpcMesSpeakerTbl[isKoopa]);
        mbWinPlayerDisable((s16)winNo, -1);
        SNpcSePlay(1, 0);
        SNpcObjMotShiftSet(8);
        mbWinWait((s16)winNo);
        mbCameraMoveWait();
        SNpcObjMotEndWait();
        mbWipeDissolveFadeOut();
        mbCameraStackPop(0);
        mbMasuPosGet(masuId, &playerPos);
        mbPlayerPosSetV(playerNo, &playerPos);
        mbPlayerRotSet(playerNo, 0.0f, 0.0f,
            0.0f);
        SNpcPosFixSnap();
    } else {
        mbWipeDissolveFadeOut();
        mbCameraStackPop(0);
        SNpcObjPosGet(&npcPos);
        mbPlayerPosGet(playerNo, &playerPos);
        SNpcObjPosSetV(&playerPos);
        mbPlayerPosSetV(playerNo, &npcPos);
        mbPlayerRotGet(playerNo, &playerRot);
        SNpcObjRotSet(0.0f, playerRot.y, 0.0f);
    }
    SNpcObjMotSet(0);
    mbPlayerMotionSet(playerNo, 1, HU3D_MOTATTR_LOOP);
    SNpcMasuEffDispSet();
    mbMusBoardFadeOut(0, 0, 1000, 1000, -1, FALSE);
    mbWipeDissolveFadeIn();
    mbMoveNumDispSet(playerNo, TRUE);
    if (!moveStarF) {
        SNpcPosFixCreate();
    }
}

static BOOL SNpcMoveExec(void)
{
    int typeTbl[3] = { 1, 2, -1 };
    s8 prizePlayerTbl[GW_PLAYER_MAX + 1];
    s8 otherPlayerTbl[GW_PLAYER_MAX + 1];
    s16 coinTbl[GW_PLAYER_MAX];
    s16 starTbl[GW_PLAYER_MAX];
    HuVecF pos;
    float roll;
    int isKoopa;
    int linkNo;
    int diceValue;
    int masuId;
    int nextMasu;
    int diceNum;
    int jingleNo;
    int prizeCount;
    int otherCount;
    int eligibleCount;
    int humanCount;
    int disablePlayer;
    s8 hitTbl[GW_PLAYER_MAX];
    int first;
    int second;
    int streamNo;
    int winNo;
    int i;
    int j;

    jingleNo = -1;
    if (GwSystem.turnPlayerNo < 3) {
        return;
    }
    isKoopa = snpcSaveWork->isKoopa;
    mbWipeSpecialFadeInCreate(5, 1);
    masuId = snpcSaveWork->masuId;
    mbMasuPosGet(masuId, &pos);
    SNpcObjPosSetV(&pos);
    SNpcObjRotSet(0.0f, 0.0f, 0.0f);
    mbCapMasuDispSet(FALSE);
    mbMasuPlayerDispSet(FALSE);
    mbCameraFocusObjSet(snpcWork->objId[0]);
    mbCameraOffsetSet(0.0f, 150.0f, 0.0f);
    mbCameraZoomSet(mbCameraPlayerViewZoomGet(0));
    mbCameraMoveOnSet(FALSE);
    mbWipeSpecialFadeOutCreate(5, 42);
    mbCameraMoveOnSet(TRUE);
    streamNo = snpcMoveStrmTbl[isKoopa];
    mbMusPlay(0, streamNo, 127, 0);
    mbTelopCreate(-1, isKoopa + 14, 1);

    diceNum = 1;
    if (mbRandMod(100) < snpcEffectChanceTbl[snpcSaveWork->effectMissCount]) {
        diceNum++;
        snpcSaveWork->effectMissCount = 0;
    } else {
        snpcSaveWork->effectMissCount++;
        if (snpcSaveWork->effectMissCount >= 4) {
            snpcSaveWork->effectMissCount = 3;
        }
    }
    if (GwSystem.turnNo < 2) {
        diceNum = 1;
    }
    if (diceNum > 1) {
        SNpcEffectExec();
    }

    if (isKoopa) {
        roll = 0.4f + 0.6f * frandf();
    } else {
        roll = 0.20000000298023224 + 0.6 * frandf();
        if (diceNum > 1) {
            roll = 0.4f + 0.5f * frandf();
        }
        if (mbRandMod(100) < 10) {
            roll = 0.6f + 0.4f * frandf();
        }
    }
    diceValue = SNpcDiceValueGet(snpcSaveWork->masuId, diceNum * 10, typeTbl,
        snpcWork->dataNum, snpcWork->pathTbl, roll);
    snpcWork->diceNumObj = SNpcDiceExec(diceValue, diceNum);
    SNpcZoomSet(mbCameraPlayerViewZoomGet(2));
    SNpcMasuReset(TRUE);
    linkNo = -1;

    while (snpcWork->pathTbl[linkNo + 1] > 0) {
        linkNo = MasuNextGet(masuId, linkNo);
        nextMasu = snpcWork->pathTbl[linkNo];
        mbev_PlayerColCircleAdd(-1, nextMasu, FALSE,
            175.0f);
        for (i = 0, j = 0; i < GW_PLAYER_MAX; i++) {
            coinTbl[i] = mbPlayerCoinGet(i);
            starTbl[i] = mbPlayerStarGet(i);
            hitTbl[i] = 0;
            if (GwPlayer[i].masuId == nextMasu) {
                hitTbl[i] = 1;
            }
        }
        if (GWTeamFGet()) {
            for (j = 0; j < 2; j++) {
                first = mbPlayerTeamFindPlayer(j, 0);
                second = mbPlayerTeamFindPlayer(j, 1);

                if (hitTbl[first] && hitTbl[second]) {
                    coinTbl[second] = starTbl[first] = 0;
                    if (coinTbl[first] > 20) {
                        coinTbl[second] = coinTbl[first] - 20;
                        coinTbl[first] = 20;
                    }
                    if (starTbl[second] > 1) {
                        starTbl[first] = starTbl[second] - 1;
                        starTbl[second] = 1;
                    }
                }
            }
        }
        prizeCount = humanCount = otherCount = eligibleCount = 0;
        disablePlayer = 0;
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (!hitTbl[i]) {
                continue;
            }
            if (GwPlayer[i].comF) {
                disablePlayer = i;
            } else {
                humanCount++;
            }
            if (!isKoopa) {
                if (coinTbl[i] >= 20) {
                    prizePlayerTbl[prizeCount++] = (s8)i;
                    eligibleCount++;
                } else {
                    otherPlayerTbl[otherCount++] = (s8)i;
                }
            } else {
                if (starTbl[i] > 0) {
                    prizePlayerTbl[prizeCount++] = (s8)i;
                } else {
                    otherPlayerTbl[otherCount++] = (s8)i;
                }
                eligibleCount++;
            }
        }
        prizePlayerTbl[prizeCount] = -1;
        otherPlayerTbl[otherCount] = -1;
        snpcWork->rotateObj = omAddObjEx(mbObjMan, SNPC_MOVE_OBJ_PRIORITY, 0, 0,
            OM_GRP_NONE, SNpcMoveOMExec);
        omObjGetWork(snpcWork->rotateObj, SNPCMOVEOMWORK)->masuId =
            (s16)nextMasu;
        while (snpcWork->rotateObj) {
            HuPrcVSleep();
        }
        mbev_PlayerColMasuAdd(-1, masuId, FALSE);
        if (mbMasuDispCheck((s16)nextMasu)) {
            diceValue--;
        }
        mbDiceSNpcNumSet(snpcWork->diceNumObj, diceValue);
        if (humanCount != 0) {
            disablePlayer = -1;
        }
        if (prizeCount + otherCount > 0) {
            mbDiceSNpcNumDispSet(snpcWork->diceNumObj, FALSE);
            SNpcZoomSet(mbCameraPlayerViewZoomGet(0));
            SNpcTargetAngleSet(0.0f);
            SNpcRotateWait();
            while (!mbPlayerColCheck()) {
                HuPrcVSleep();
            }
            {
                winNo = mbWinCreate(2, snpcStarMesTbl[1][isKoopa],
                    snpcStarMesSpeakerTbl[isKoopa]);

                mbWinPlayerDisable((s16)winNo, disablePlayer);
                if (!isKoopa) {
                    int seNo;
                    int motionAttr;
                    int i;

                    motionAttr = 0;
                    seNo = -1;

                    seNo = 579;
                    for (i = 0; otherPlayerTbl[i] >= 0; i++) {
                        if (seNo >= 0) {
                            mbPlayerWinLoseVoicePlay(otherPlayerTbl[i], 12, seNo);
                        }
                        mbPlayerMotionShiftSet(otherPlayerTbl[i], 12,
                            0.0f, 8.0f, motionAttr);
                    }
                    {
                        int seNo;
                        int motionAttr;
                        int i;

                        motionAttr = 0;
                        seNo = -1;

                        seNo = 579;
                        for (i = 0; prizePlayerTbl[i] >= 0; i++) {
                            if (seNo >= 0) {
                                mbPlayerWinLoseVoicePlay(prizePlayerTbl[i], 12,
                                    seNo);
                            }
                            mbPlayerMotionShiftSet(prizePlayerTbl[i], 12,
                                0.0f, 8.0f, motionAttr);
                        }
                    }
                } else {
                    int seNo;
                    int motionAttr;
                    int i;

                    motionAttr = 0;
                    seNo = -1;

                    seNo = 585;
                    for (i = 0; otherPlayerTbl[i] >= 0; i++) {
                        if (seNo >= 0) {
                            mbPlayerWinLoseVoicePlay(otherPlayerTbl[i], 13, seNo);
                        }
                        mbPlayerMotionShiftSet(otherPlayerTbl[i], 13,
                            0.0f, 8.0f, motionAttr);
                    }
                    {
                        int seNo;
                        int motionAttr;
                        int i;

                        motionAttr = 0;
                        seNo = -1;

                        seNo = 585;
                        for (i = 0; prizePlayerTbl[i] >= 0; i++) {
                            if (seNo >= 0) {
                                mbPlayerWinLoseVoicePlay(prizePlayerTbl[i], 13,
                                    seNo);
                            }
                            mbPlayerMotionShiftSet(prizePlayerTbl[i], 13,
                                0.0f, 8.0f, motionAttr);
                        }
                    }
                    SNpcObjMotShiftSet(8);
                }
                SNpcSePlay(1, 0);
                {
                    int i;

                    for (i = 0; otherPlayerTbl[i] >= 0; i++) {
                        mbPlayerMotionEndWait(otherPlayerTbl[i]);
                    }
                }
                {
                    int i;

                    for (i = 0; prizePlayerTbl[i] >= 0; i++) {
                        mbPlayerMotionEndWait(prizePlayerTbl[i]);
                    }
                }
                {
                    int seNo;
                    int motionAttr;
                    int i;

                    motionAttr = 0;
                    seNo = -1;

                    motionAttr = HU3D_MOTATTR_LOOP;
                    for (i = 0; otherPlayerTbl[i] >= 0; i++) {
                        if (seNo >= 0) {
                            mbPlayerWinLoseVoicePlay(otherPlayerTbl[i], 1,
                                seNo);
                        }
                        mbPlayerMotionShiftSet(otherPlayerTbl[i], 1,
                            0.0f, 8.0f, motionAttr);
                    }
                }
                {
                    int seNo;
                    int motionAttr;
                    int i;

                    motionAttr = 0;
                    seNo = -1;

                    motionAttr = HU3D_MOTATTR_LOOP;
                    for (i = 0; prizePlayerTbl[i] >= 0; i++) {
                        if (seNo >= 0) {
                            mbPlayerWinLoseVoicePlay(prizePlayerTbl[i], 1,
                                seNo);
                        }
                        mbPlayerMotionShiftSet(prizePlayerTbl[i], 1,
                            0.0f, 8.0f, motionAttr);
                    }
                }
                if (isKoopa == TRUE) {
                    SNpcObjMotEndWait();
                    SNpcObjMotShiftSet(0);
                }
                mbWinWait((s16)winNo);
            }
            if (eligibleCount != 0) {
                if (isKoopa == TRUE && prizeCount == 0) {
                    winNo = mbWinCreate(2,
                        snpcStarMesTbl[4][isKoopa],
                        snpcStarMesSpeakerTbl[isKoopa]);
                } else {
                    winNo = mbWinCreate(2,
                        snpcStarMesTbl[0][isKoopa],
                        snpcStarMesSpeakerTbl[isKoopa]);
                }

                mbWinPlayerDisable((s16)winNo, disablePlayer);
                mbWinWait((s16)winNo);
                SNpcObjMotShiftSet(7);
                {
                    HuVecF starPos;

                    mbMasuPosGet((s16)nextMasu, &starPos);
                    SNpcStarCreate(isKoopa, FALSE, &starPos);
                    mbMusPauseFadeOut(0, TRUE, 1000);
                    SNpcStarWait();
                    SNpcObjMotEndWait();
                    SNpcObjMotShiftSet(0);
                }
                if (!isKoopa) {
                    for (j = 0; prizePlayerTbl[j] >= 0; j++) {
                        mbCoinAddDispExec(prizePlayerTbl[j], -20, FALSE, TRUE);
                    }
                    jingleNo = mbMusJinglePlay(39);
                    StarChangeExec(prizePlayerTbl, 0, 1);
                    mbMusJingleWait(jingleNo);
                } else if (prizeCount != 0) {
                    StarChangeExec(prizePlayerTbl, 0, -1);
                    {
                        int seNo;
                        int motionAttr;
                        int i;

                        motionAttr = 0;
                        seNo = -1;

                        seNo = 579;
                        for (i = 0; otherPlayerTbl[i] >= 0; i++) {
                            if (seNo >= 0) {
                                mbPlayerWinLoseVoicePlay(otherPlayerTbl[i], 12,
                                    seNo);
                            }
                            mbPlayerMotionShiftSet(otherPlayerTbl[i], 12,
                                0.0f, 8.0f, motionAttr);
                        }
                    }
                    {
                        int i;

                        for (i = 0; otherPlayerTbl[i] >= 0; i++) {
                            mbPlayerMotionEndWait(otherPlayerTbl[i]);
                        }
                    }
                } else {
                    StarChangeExec(otherPlayerTbl, 1, -20);
                }
                mbMusPauseFadeOut(0, FALSE, 1000);
                HuPrcSleep(12);
                if (prizeCount != 0 && otherCount != 0) {
                    winNo = mbWinCreate(2, snpcStarMesTbl[2][isKoopa],
                        snpcStarMesSpeakerTbl[isKoopa]);

                    mbWinPlayerDisable((s16)winNo, disablePlayer);
                    if (!isKoopa) {
                        {
                            int seNo;
                            int motionAttr;
                            int i;

                            motionAttr = 0;
                            seNo = -1;

                            motionAttr = HU3D_MOTATTR_LOOP;
                            for (i = 0; prizePlayerTbl[i] >= 0; i++) {
                                if (seNo >= 0) {
                                    mbPlayerWinLoseVoicePlay(prizePlayerTbl[i], 1,
                                        seNo);
                                }
                                mbPlayerMotionShiftSet(prizePlayerTbl[i], 1,
                                    0.0f, 8.0f, motionAttr);
                            }
                        }
                        SNpcSePlay(2, 0);
                        SNpcObjMotShiftSet(9);
                        mbWinWait((s16)winNo);
                        {
                            int seNo;
                            int motionAttr;
                            int i;

                            motionAttr = 0;
                            seNo = -1;

                            seNo = 585;
                            for (i = 0; otherPlayerTbl[i] >= 0; i++) {
                                if (seNo >= 0) {
                                    mbPlayerWinLoseVoicePlay(otherPlayerTbl[i], 13,
                                        seNo);
                                }
                                mbPlayerMotionShiftSet(otherPlayerTbl[i], 13,
                                    0.0f, 8.0f, motionAttr);
                            }
                        }
                        {
                            int i;

                            for (i = 0; otherPlayerTbl[i] >= 0; i++) {
                                mbPlayerMotionEndWait(otherPlayerTbl[i]);
                            }
                        }
                        SNpcObjMotEndWait();
                    } else {
                        mbWinWait((s16)winNo);
                        StarChangeExec(otherPlayerTbl, 1, -20);
                    }
                }
            } else {
                winNo = mbWinCreate(2, snpcStarMesTbl[4][isKoopa],
                    snpcStarMesSpeakerTbl[isKoopa]);

                mbWinPlayerDisable((s16)winNo, disablePlayer);
                SNpcSePlay(2, 0);
                SNpcObjMotShiftSet(9);
                mbWinWait((s16)winNo);
                {
                    int seNo;
                    int motionAttr;
                    int i;

                    motionAttr = 0;
                    seNo = -1;

                    seNo = 585;
                    for (i = 0; otherPlayerTbl[i] >= 0; i++) {
                        if (seNo >= 0) {
                            mbPlayerWinLoseVoicePlay(otherPlayerTbl[i], 13, seNo);
                        }
                        mbPlayerMotionShiftSet(otherPlayerTbl[i], 13,
                            0.0f, 8.0f, motionAttr);
                    }
                }
                {
                    int i;

                    for (i = 0; otherPlayerTbl[i] >= 0; i++) {
                        mbPlayerMotionEndWait(otherPlayerTbl[i]);
                    }
                }
                SNpcObjMotEndWait();
            }
            mbDiceSNpcNumDispSet(snpcWork->diceNumObj, TRUE);
            SNpcZoomSet(mbCameraPlayerViewZoomGet(2));
        }
        masuId = nextMasu;
    }
    mbDiceSNpcNumKill(snpcWork->diceNumObj);
    snpcWork->diceNumObj = NULL;
    SNpcMasuSet(snpcWork->pathTbl[linkNo], TRUE);
    SNpcZoomSet(mbCameraPlayerViewZoomGet(0));
    SNpcTargetAngleSet(0.0f);
    SNpcRotateWait();
    {
        winNo = mbWinCreate(2, snpcStarMesTbl[3][isKoopa],
            snpcStarMesSpeakerTbl[isKoopa]);

        mbWinPlayerDisable((s16)winNo, -1);
        SNpcSePlay(1, 0);
        SNpcObjMotShiftSet(8);
        mbWinWait((s16)winNo);
        SNpcObjMotEndWait();
        SNpcZoomWait();
    }
    if (GwSystem.turnPlayerNo < 3 || _CheckFlag(FLAG_BOARD_NOMG)) {
        mbCapMasuDispSet(TRUE);
        mbMasuPlayerDispSet(TRUE);
    }
    SNpcMasuEffDispSet();
    mbMusBoardFadeOut(0, 0, 1000, 1000, -1, FALSE);
    return FALSE;
}

static void StarChangeExec(s8 *playerNoTbl, int starF, int amount)
{
    int addNum[GW_PLAYER_MAX] ATTRIBUTE_ALIGN(8);
    BOOL dispF[GW_PLAYER_MAX];
    int voiceNo;
    int playerNo;
    int i;

    voiceNo = 0;
    if (amount < 0) {
        voiceNo++;
    }
    for (i = 0; playerNoTbl[i] >= 0; i++) {
        mbPlayerWinLoseVoicePlay(playerNoTbl[i],
            playerStarChgMotNoTbl[voiceNo],
            playerStarChgSeTbl[voiceNo]);
        mbPlayerMotionShiftSet(playerNoTbl[i],
            playerStarChgMotNoTbl[voiceNo], 0.0f,
            8.0f, 0);
        omVibrate((s16)playerNoTbl[i], 20, 7, 3);
    }

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        addNum[i] = 0;
        dispF[i] = FALSE;
    }
    for (i = 0; playerNoTbl[i] >= 0; i++) {
        playerNo = playerNoTbl[i];
        addNum[playerNo] = amount;
        dispF[playerNo] = amount;
    }
    if (starF == 0) {
        mbStarAddAllProcExecV(addNum, dispF, TRUE);
    } else {
        mbCoinAddAllProcExecV(addNum, dispF, TRUE);
    }
    for (i = 0; playerNoTbl[i] >= 0; i++) {
        mbPlayerMotionEndWait(playerNoTbl[i]);
    }
    HuPrcSleep(20);
}

static int MasuNextGet(int masuId, int linkNo)
{
    int linkNoTbl[3] = { 1, 2, -1 };
    int nextLinkNo;
    BOOL jumpF;

    nextLinkNo = linkNo + 1;
    jumpF = FALSE;
    if ((mbMasuAttrGet((s16)masuId) & MASU_FLAG_JUMPFROM)
        && (mbMasuAttrGet(snpcWork->pathTbl[linkNo + 1])
            & MASU_FLAG_JUMPTO)) {
        jumpF = TRUE;
        nextLinkNo = linkNo + 1;
    } else if ((mbMasuAttrGet((s16)masuId) & MASU_FLAG_CLIMBFROM)
        && (mbMasuAttrGet(snpcWork->pathTbl[linkNo + 1])
            & MASU_FLAG_CLIMBTO)) {
        jumpF = TRUE;
        nextLinkNo = linkNo + 2;
    } else if ((mbMasuAttrGet(
            snpcWork->pathTbl[linkNo + 1]) & MASU_FLAG_CLIMBFROM)
        && (mbMasuAttrGet(snpcWork->pathTbl[linkNo + 2])
            & MASU_FLAG_CLIMBTO)) {
        jumpF = TRUE;
        nextLinkNo = linkNo + 3;
    }
    snpcWork->unk0C = FALSE;
    if (jumpF) {
        snpcWork->unk0C = TRUE;
    }
    return nextLinkNo;
}

static void SNpcMoveOMExec(OMOBJ *obj)
{
    SNPCMOVEOMWORK *work;
    HuVecF pos;
    HuVecF move;
    float time;
    float weight;
    BOOL endF;

    work = omObjGetWork(obj, SNPCMOVEOMWORK);
    endF = FALSE;
    if (mbExitCheck() || work->killF) {
        omDelObjEx(HuPrcCurrentGet(), obj);
        snpcWork->rotateObj = NULL;
        return;
    }
    if (!work->initF) {
        work->initF = TRUE;
        work->unk06 = 0;
        work->time = 0;
        work->unk08 = 0;
        SNpcObjPosGet(&obj->trans);
        SNpcObjRotGet(&obj->rot);
        mbMasuPosGet(work->masuId, &obj->scale);
        PSVECSubtract(&obj->scale, &obj->trans, &move);
        time = PSVECMag(&move);
        if (snpcSaveWork->isKoopa == 1) {
            time *= 1.6666666f;
        }
        work->maxTime = (s16)(time / 16.666666f);
        obj->rot.z = (float)(180.0
            * (atan2(move.x, move.z) / 3.141592653589793));
        if (snpcWork->unk0C != 0) {
            work->mode = 1;
            SNpcObjMotShiftSet(3);
        } else {
            SNpcObjMotShiftSet(1);
        }
    }

    work->time++;
    work->unk06++;
    time = (float)work->time / (float)work->maxTime;
    pos.x = obj->trans.x + (time * (obj->scale.x - obj->trans.x));
    pos.y = obj->trans.y + (time * (obj->scale.y - obj->trans.y));
    pos.z = obj->trans.z + (time * (obj->scale.z - obj->trans.z));

    weight = 0.16666667f * (float)work->unk06;
    if (weight > 1.0f) {
        weight = 1.0f;
    }
    obj->rot.y = mbAngleLerp(obj->rot.y, obj->rot.z,
        0.15f * weight);
    SNpcObjRotSet(0.0f, obj->rot.y, 0.0f);

    if (work->mode == 0) {
        SNpcObjPosSetV(&pos);
        if (work->time >= work->maxTime) {
            endF = TRUE;
        }
    } else {
        switch (work->unk08) {
        case 0:
            if (SNpcObjMotEndCheck()) {
                SNpcObjMotShiftSet(4);
                work->time = 0;
                work->maxTime = 35;
                work->unk08++;
            }
            break;
        case 1:
            if (work->time >= work->maxTime - 5) {
                SNpcObjMotShiftSet(5);
                work->unk08++;
            }
            /* fall through */
        case 2:
            pos.y += 100.0 * (4.0 * sin(
                (3.141592653589793 * (180.0f * time))
                    / 180.0));
            SNpcObjPosSetV(&pos);
            if (work->time >= work->maxTime) {
                if (snpcSaveWork->isKoopa == 1) {
                    mbCameraShakeSet(12, 100.0f);
                }
                work->unk08++;
            }
            break;
        case 3:
            if (SNpcObjMotEndCheck()) {
                endF = TRUE;
            }
            break;
        }
    }
    if (snpcWork->diceNumObj) {
        SNpcObjPosGet(&pos);
        mbDiceSNpcNumPosSet(snpcWork->diceNumObj, &pos);
    }
    SNpcMotSetNext();
    if (endF) {
        omDelObjEx(HuPrcCurrentGet(), obj);
        snpcWork->rotateObj = NULL;
        return;
    }
}

static void SNpcTargetAngleSet(float angle)
{
    snpcWork->rotateObj = omAddObjEx(mbObjMan, SNPC_MOVE_OBJ_PRIORITY, 0,
        0, OM_GRP_NONE, SNpcRotateUpdate);
    omObjGetWork(snpcWork->rotateObj, SNPCROTATEWORK)->targetAngle = angle;
}

static void SNpcRotateWait(void)
{
    BOOL endF;

    goto check;
sleep:
    HuPrcVSleep();
check:
    if (snpcWork->rotateObj) {
        endF = FALSE;
    } else if (mbObjMotionShiftIDGet(snpcWork->objId[0]) != -1) {
        endF = FALSE;
    } else {
        endF = TRUE;
    }
    if (!endF) {
        goto sleep;
    }
}

static void SNpcRotateUpdate(OMOBJ *obj)
{
    SNPCROTATEWORK *work;
    BOOL endF;
    float magnitude;
    float time;
    float weight;
    float angle;
    float magnitudeResult;

    work = omObjGetWork(obj, SNPCROTATEWORK);
    endF = FALSE;
    if (mbExitCheck() || work->killF) {
        omDelObjEx(HuPrcCurrentGet(), obj);
        snpcWork->rotateObj = NULL;
        return;
    }
    if (!work->initF) {
        work->initF = TRUE;
        work->time = 0;
        work->unk06 = 0;
        SNpcObjRotGet(&obj->rot);
        obj->scale.y = mbAngleWrap2(work->targetAngle, obj->rot.y);
        magnitude = obj->scale.y;
        magnitude = __fabsf(magnitude);
        magnitudeResult = magnitude;
        time = 0.0055555557f * magnitudeResult;
        work->maxTime = (s16)(15.0f
            * (0.7f + (time
                * snpcRotSpeedTbl[snpcSaveWork->isKoopa])));
        if ((float)abs((s32)obj->scale.y) < 5.0f) {
            SNpcObjMotShiftSet(0);
        } else {
            SNpcObjMotShiftSet(1);
        }
    }
    time = (float)work->time++ / (float)work->maxTime;
    weight = (float)(0.5 + (0.5
        * sin((3.141592653589793 * (2.7e+02f
            + (180.0f * time))) / 180.0)));
    angle = obj->rot.y + (weight * obj->scale.y);
    SNpcObjRotSet(obj->rot.x, angle, obj->rot.z);
    if (work->time >= work->maxTime) {
        endF = TRUE;
        SNpcObjMotShiftSet(0);
    }
    SNpcMotSetNext();
    if (endF) {
        omDelObjEx(HuPrcCurrentGet(), obj);
        snpcWork->rotateObj = NULL;
        return;
    }
}

static void SNpcPosFixCreate(void)
{
    snpcWork->rotateObj = omAddObjEx(mbObjMan, SNPC_MOVE_OBJ_PRIORITY, 0, 0,
        OM_GRP_NONE, SNpcPosFixUpdate);
}

static void SNpcPosFixUpdate(OMOBJ *obj)
{
    SNPCROTATEWORK *work;
    HuVecF pos;
    HuVecF targetPos;
    HuVecF startRot;
    BOOL endF;
    float weight;
    float time;
    float angle;

    work = omObjGetWork(obj, SNPCROTATEWORK);
    endF = FALSE;
    if (mbExitCheck() || work->killF) {
        omDelObjEx(HuPrcCurrentGet(), obj);
        snpcWork->rotateObj = NULL;
        goto done;
    }
    if (!work->initF) {
        work->initF = TRUE;
        work->unk06 = 0;
        SNpcObjPosGet(&pos);
        SNpcObjRotGet(&startRot);
        SNpcPosFixSnap();
        SNpcObjPosGet(&targetPos);
        SNpcObjPosSetV(&pos);
        SNpcObjRotSetV(&startRot);
        obj->trans.x = pos.x;
        obj->trans.y = pos.y;
        obj->trans.z = pos.z;
        obj->scale.x = targetPos.x;
        obj->scale.y = targetPos.y;
        obj->scale.z = targetPos.z;
        work->maxTime = 30;
        work->time = 0;
        SNpcObjMotShiftSet(1);
    }
    if (work->unk06 == 0) {
        work->time++;
        time = (float)work->time / (float)work->maxTime;
        pos.x = obj->trans.x
            + (time * (obj->scale.x - obj->trans.x));
        pos.y = obj->trans.y
            + (time * (obj->scale.y - obj->trans.y));
        pos.z = obj->trans.z
            + (time * (obj->scale.z - obj->trans.z));
        SNpcObjPosSetV(&pos);
        if (work->time >= work->maxTime) {
            work->unk06++;
            SNpcObjRotGet(&obj->rot);
            work->targetAngle = 0.0f;
            obj->scale.y = mbAngleWrap2(work->targetAngle, obj->rot.y);
            time = 0.0055555557f * SNpcAbsFloat(obj->scale.y);
            work->maxTime = (s16)(15.0f
                * (0.7f + (time
                    * snpcPosFixSpeedTbl[snpcSaveWork->isKoopa])));
            work->time = 0;
            if ((float)abs((s32)obj->scale.y) < 5.0f) {
                SNpcObjMotShiftSet(0);
            } else {
                SNpcObjMotShiftSet(1);
            }
        }
    } else {
        time = (float)work->time++ / (float)work->maxTime;
        weight = (float)(0.5 + (0.5
            * sin((3.141592653589793 * (2.7e+02f
                + (180.0f * time))) / 180.0)));
        angle = obj->rot.y + (weight * obj->scale.y);
        SNpcObjRotSet(obj->rot.x, angle, obj->rot.z);
        if (work->time >= work->maxTime) {
            endF = TRUE;
            SNpcObjMotShiftSet(0);
        }
    }
    SNpcMotSetNext();
    if (endF) {
        work->killF = TRUE;
    }
done:
    return;
}

static void SNpcZoomSet(float zoom)
{
    if (!SNpcZoomCheck()) {
        snpcWork->zoomObj = omAddObjEx(mbObjMan, SNPC_MOVE_OBJ_PRIORITY, 0,
            0, OM_GRP_NONE, SNpcZoomUpdate);
    }
    omObjGetWork(snpcWork->zoomObj, SNPCZOOMWORK)->initF = FALSE;
    omObjGetWork(snpcWork->zoomObj, SNPCZOOMWORK)->killF = FALSE;
    omObjGetWork(snpcWork->zoomObj, SNPCZOOMWORK)->targetZoom = zoom;
}

static BOOL SNpcZoomCheck(void)
{
    return snpcWork->zoomObj != NULL;
}

static void SNpcZoomWait(void)
{
    while (snpcWork->zoomObj != NULL) {
        HuPrcVSleep();
    }
}

static void SNpcZoomUpdate(OMOBJ *obj)
{
    SNPCZOOMWORK *work;
    BOOL endF;
    float time;

    work = omObjGetWork(obj, SNPCZOOMWORK);
    endF = FALSE;
    if (mbExitCheck() || work->killF) {
        omDelObjEx(HuPrcCurrentGet(), obj);
        snpcWork->zoomObj = NULL;
        return;
    }
    if (!work->initF) {
        work->initF = TRUE;
        work->startZoom = mbCameraZoomGet();
        work->time = 0;
        work->maxTime = 21;
    }
    work->time++;
    time = (float)work->time / (float)work->maxTime;
    if (time >= 1.0f) {
        time = 1.0f;
        work->killF = TRUE;
    }
    mbCameraZoomSet(work->startZoom
        + (time * (work->targetZoom - work->startZoom)));
}

static OMOBJ *SNpcDiceExec(int diceValue, int diceNum)
{
    HuVecF pos;
    int valueTbl[3] = { 0, 0, 0 };
    int isKoopa;
    int i;

    isKoopa = snpcSaveWork->isKoopa;
    SNpcObjPosGet(&pos);
    PSVECAdd(&pos, &snpcDiceOfsTbl[isKoopa], &pos);
    if (diceNum > 2) {
        diceNum = 2;
    }
    valueTbl[0] = diceValue;
    if (diceNum == 2 && diceValue > 1) {
        i = diceValue >> 1;
        valueTbl[0] = i;
        valueTbl[1] = diceValue - i;
        i = 10 - valueTbl[1];
        if (i > valueTbl[0] - 1) {
            i = valueTbl[0] - 1;
        }
        i = mbRandMod(i + 1);
        valueTbl[0] -= i;
        valueTbl[1] += i;
        if (mbRandMod(100) < 80) {
            i = valueTbl[0];
            valueTbl[0] = valueTbl[1];
            valueTbl[1] = i;
        }
    }
    for (i = 0; i < 3; i++) {
        valueTbl[i]--;
    }
    mbDiceProcExec(-1, snpcDiceTypeTbl[diceNum - 1][isKoopa],
        (s8 *)NULL, valueTbl, FALSE, FALSE, &pos,
        snpcMoveNumColor[isKoopa]);
    mbDicePadBtnHookSet(-1, SNpcDiceBtnHook);
    mbDiceMotHookSet(-1, SNpcDiceMotHook);
    while (!mbDiceKillCheck(-1)) {
        HuPrcVSleep();
    }
    SNpcObjPosGet(&pos);
    return mbDiceSNpcNumCreate(-1, &pos);
}

static u16 SNpcDiceBtnHook(int playerNo)
{
    return PAD_BUTTON_A;
}

static void SNpcDiceMotHook(int playerNo)
{
    int time;

    SNpcObjMotSet(6);
    time = 0;
    do {
        if (time++ == snpcDiceMotTimeTbl[snpcSaveWork->isKoopa]) {
            mbDiceObjHit(-1);
        }
        HuPrcVSleep();
    } while (!SNpcObjMotEndCheck());
    SNpcObjMotShiftSet(0);
}

typedef struct SNPCMASUTBL {
    int value[2];
} SNPCMASUTBL;

const float lbl_802C332C[1] = { 2.0f };
const float lbl_802C3330[1] = { 360.0f };

const SNPCMASUTBL lbl_802C3334 = {{
    SNPC_MASU_TYPE_DONKEY,
    SNPC_MASU_TYPE_KOOPA,
}};
const SNPCMASUTBL lbl_802C333C = {{
    SNPC_MASU_SE_DONKEY,
    SNPC_MASU_SE_KOOPA,
}};

static void SNpcMasuSet(int masuId, BOOL setF)
{
    SNPCMASUTBL masuTypeTbl = lbl_802C3334;
    SNPCMASUTBL masuSeTbl = lbl_802C333C;

    snpcSaveWork->masuId = (u8)masuId;
    snpcWork->unk04 = mbMasuTypeGet((s16)masuId);
    mbMasuTypeSet((s16)masuId,
        masuTypeTbl.value[snpcSaveWork->isKoopa]);
    mbMasuCapsuleSet((s16)masuId, -1);
    if (setF) {
        mbAudFXPlay((s16)masuSeTbl.value[snpcSaveWork->isKoopa]);
        SNpcObjMasuSet(masuId);
    }
}

static void SNpcMasuReset(BOOL setF)
{
    mbMasuTypeSet(snpcSaveWork->masuId, snpcWork->unk04);
    if (setF) {
        mbAudFXPlay((s16)SNPC_MASU_RESET_SE);
        SNpcObjMasuSet(snpcSaveWork->masuId);
    }
    snpcSaveWork->masuId = 0;
}

static int SNpcMasuStarNextGet(BOOL playerF)
{
    int result;
    int type;
    int linkNoTbl[3] = { 1, 2, -1 };

    type = 10;
    if (playerF) {
        type = 4;
    }
    result = mbSNpcMasuStarNextGet(snpcSaveWork->masuId, type, linkNoTbl,
        snpcWork->dataNum);
    return result;
}

static void SNpcPosFixSnap(void)
{
    int masuId;
    int linkNo;
    int linkMasuId;
    HuVecF pos;
    HuVecF posLink;
    Mtx masuMtx;

    if (snpcMagic != SNPC_MAGIC) {
        return;
    }
    masuId = snpcSaveWork->masuId;
    linkNo = 0;
    while (linkNo < mbMasuLinkNumGet((s16)masuId)) {
        linkMasuId = mbMasuLinkGet((s16)masuId, linkNo);
        if ((mbMasuMAttrGet(linkMasuId) & mbBranchMAttrGet()) == 0) {
            break;
        }
        linkNo++;
    }
    mbMasuPosGet((s16)masuId, &pos);
    mbMasuPosGet(linkMasuId, &posLink);
    PSVECSubtract(&posLink, &pos, &posLink);
    posLink.y = 0.0f;
    PSVECNormalize(&posLink, &posLink);
    PSVECScale(&posLink, &posLink, 1.3e+02f);
    mbMasuMtxGet((s16)masuId, masuMtx);
    PSMTXMultVec(masuMtx, &posLink, &pos);
    SNpcObjPosSetV(&pos);
    SNpcObjRotSet(0.0f, 0.0f, 0.0f);
}

const float lbl_802C3348[1] = { -110.000015f };
const float lbl_802C334C[1] = { 14.0f };
const float lbl_802C3350[1] = { 7.0f };
const float lbl_802C3354[1] = { 30.0f };

/* The native vector-copy primitive reads all three components before writing. */
static inline void SNpcVecCopy(register const HuVecF *src, register HuVecF *dst)
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

static inline MBPARTICLE *SNpcParticleGet(HU3D_MODELID modelId)
{
    return Hu3DData[modelId].hookData;
}

static void SNpcKoopaFireExec(void)
{
    int fire3Id;
    int fire3CopyId;
    int fireId;
    int fireCopyId;
    MBPARTICLE *copyP;
    MBPARTICLE *fire3P;
    MBPARTICLE *fireP;
    HuVecF offset;
    Mtx mtx;
    float value;
    int mode;
    int i;

    SNpcObjMotShiftSet(10);
    mbAudFXPlay(MSM_SE_GUIDE_49);
    fire3Id = mbParticleCreate(HuSprAnimRead(HuDataReadNum(
        mbBoardDataNumGet(SNPC_KOOPA_FIRE_DATA_ANIM), HU_MEMNUM_OVL)), 88);
    fire3CopyId = mbParticleCreate(HuSprAnimRead(HuDataReadNum(
        mbBoardDataNumGet(SNPC_KOOPA_FIRE_DATA_ANIM), HU_MEMNUM_OVL)), 88);
    mbParticleHookSet(fire3Id, SNpcKoopaFire3Hook);
    mbParticleHookSet(fire3CopyId, SNpcKoopaFire2Hook);
    Hu3DModelLayerSet(fire3Id, 5);
    Hu3DModelLayerSet(fire3CopyId, 5);
    fire3P = SNpcParticleGet(fire3Id);
    copyP = SNpcParticleGet(fire3CopyId);
    copyP->hookData = fire3P;
    copyP->mode = 0;
    fireId = mbParticleCreate(HuSprAnimRead(HuDataReadNum(
        mbBoardDataNumGet(SNPC_KOOPA_FIRE_DATA_ANIM), HU_MEMNUM_OVL)), 500);
    fireCopyId = mbParticleCreate(HuSprAnimRead(HuDataReadNum(
        mbBoardDataNumGet(SNPC_KOOPA_FIRE_DATA_ANIM), HU_MEMNUM_OVL)), 500);
    mbParticleHookSet(fireId, SNpcKoopaFireHook);
    mbParticleHookSet(fireCopyId, SNpcKoopaFire2Hook);
    Hu3DModelLayerSet(fireId, 5);
    Hu3DModelLayerSet(fireCopyId, 5);
    fireP = SNpcParticleGet(fireId);
    copyP = SNpcParticleGet(fireCopyId);
    copyP->hookData = fireP;
    copyP->mode = 0;
    mode = 0;
    fireP->mode = fire3P->mode = (s16)mode;
    for (i = 0; i < 260; i++) {
        if (i > 30) {
            if (i < 60) {
                mode = 16;
            } else {
                mode += 4;
                if (mode > 256) {
                    mode = 256;
                }
            }
            fireP->mode = (s16)mode;
            if (mode > 64) {
                fire3P->mode = (s16)mode;
            }
        }
        Hu3DModelObjMtxGet(mbObjModelIDGet(snpcWork->objId[0]),
            lbl_80247724, mtx);
        fireP->vel.x = mtx[0][3];
        fireP->vel.y = mtx[1][3];
        fireP->vel.z = mtx[2][3];
        mtx[0][3] = mtx[1][3] = mtx[2][3] = 0.0f;
        offset.x = 15.000001f;
        offset.y = 1e+01f;
        offset.z = (-25.0f);
        PSMTXMultVec(mtx, &offset, &offset);
        PSVECAdd(&fireP->vel, &offset, &fireP->vel);
        SNpcVecCopy(&fireP->vel, &fire3P->vel);
        offset.x = 0.2f;
        offset.y = 1.0f;
        offset.z = 0.0f;
        PSMTXMultVec(mtx, &offset, (HuVecF *)fireP->work);
        SNpcVecCopy((HuVecF *)fireP->work, (HuVecF *)fire3P->work);
        value = (float)mode / 256.0f;
        PSVECScale((HuVecF *)fireP->work, &offset,
            100.0f * (4.0f * value));
        PSVECAdd(&fireP->vel, &offset, &fireP->vel);
        HuPrcVSleep();
    }
    fireP->mode = 0;
    fire3P->mode = 0;
    SNpcObjMotEndWait();
    SNpcObjMotShiftSet(0);
    HuPrcSleep(30);
    mbParticleKill(fireId);
    mbParticleKill(fireCopyId);
    mbParticleKill(fire3Id);
    mbParticleKill(fire3CopyId);
    HuPrcSleep(4);
}

const HuVecF lbl_8021AA28 = { -70.0f, 320.0f, 20.0f };

static void SNpcKoopaFireHook(HU3D_MODEL *modelP, MBPARTICLE *particleP,
    Mtx mtx)
{
    HuVecF axisCross;
    HuVecF axisZ = { 0.0f, 0.0f, 1.0f };
    HuVecF axisX = { 1.0f, 0.0f, 0.0f };
    MBPARTICLEDATA *data;
    const HuVecF *axis;
    Mtx rotMtx;
    float modeScale;
    float blend;
    float speed;
    float value;
    int i;

    if (particleP->count == 0) {
        data = particleP->data;
        for (i = 0; i < particleP->num; i++, data++) {
            data->scale = 0.0f;
            data->color.a = 0;
            data->time = (s16)-(i >> 3);
        }
        particleP->count = 1;
    }

    data = particleP->data;
    for (i = 0; i < particleP->num; i++, data++) {
        if (particleP->mode != 0) {
            if (data->time < 0) {
                data->time++;
                continue;
            }
            if (data->time == 0) {
                SNpcVecCopy((HuVecF *)particleP->work, &data->vel);
                axis = &axisZ;
                if (SNpcAbsFloat(data->vel.z) >= SNpcAbsFloat(data->vel.y)) {
                    if (SNpcAbsFloat(data->vel.z) >= SNpcAbsFloat(data->vel.x)) {
                        axis = &axisX;
                    }
                }

                PSVECCrossProduct(axis, &data->vel, &axisCross);
                PSMTXRotAxisRad(rotMtx, &data->vel,
                    frandf() * lbl_802C3330[0] * 0.017453292f);
                PSMTXMultVec(rotMtx, &axisCross, &data->accel);

                data->time = data->activeF = 30;
                SNpcVecCopy(&particleP->vel, &data->pos);

                speed = frandf() * 0.3f * 100.0f;
                PSVECScale(&data->vel, &axisCross, speed);
                PSVECAdd(&data->pos, &axisCross, &data->pos);

                speed = frandf() * 0.3f * 100.0f;
                PSVECScale(&data->accel, &axisCross, speed);
                PSVECAdd(&data->pos, &axisCross, &data->pos);

                modeScale = (float)particleP->mode / 256.0f;
                speed = (0.2f + (0.8f * modeScale))
                    * (0.3f + (0.7f * frandf()));

                blend = 10.000001f * speed;
                PSVECScale(&data->vel, &data->vel, blend);
                blend = 6.666667f * speed;
                PSVECScale(&data->accel, &data->accel, blend);

                data->colorIdx = 0.0f;
                data->scaleBase = 0.55555564f;
                data->scaleBase *=
                    0.5f + (0.5f * frandf());

                data->rot.z = (float)mbRandMod(360);
                data->animBank = (s16)mbRandMod(8);

                data->color.r = data->color.g = data->color.b =
                    100.0f + 155.0f * frandf();
                blend = 0.5f + (0.5f * modeScale);
                data->color.a = (u8)(mbRandMod(32) + 48);
                data->scale = blend
                    * (lbl_802C3354[0] + (50.0f * frandf()));
            }
        }

        if (data->time <= 0) {
            continue;
        }

        speed = 1.0f
            - ((float)data->time / (float)data->activeF);
        data->time--;

        blend = mbSinDeg(100.0f + (3.2e+02f * speed));
        PSVECScale(&data->vel, &axisCross, blend);
        PSVECAdd(&data->pos, &axisCross, &data->pos);

        blend = mbSinDeg(1e+01f + (3.2e+02f * speed));
        PSVECScale(&data->accel, &axisCross, blend);
        PSVECAdd(&data->pos, &axisCross, &data->pos);

        if (particleP->mode == 0) {
            data->colorIdx += data->scaleBase;
            data->pos.y += data->colorIdx;
            if (speed > 0.5f) {
                data->scale *= 0.9f;
            }
        }

        if (speed > 0.7f) {
            blend = 3.3333333f * (speed - 0.7f);

            value = (float)data->color.r;
            data->color.r = (u8)(value
                + (50.0f - value) * blend);

            value = (float)data->color.g;
            data->color.g = (u8)(value
                + (50.0f - value) * blend);

            value = (float)data->color.b;
            data->color.b = (u8)(value
                + (50.0f - value) * blend);
        }

        if (speed > 0.85f) {
            blend = 6.6666665f * (speed - 0.85f);
            value = (float)data->color.a;
            data->color.a = (u8)(value
                + (3.0f - value) * blend);
        }

        if (data->time == 0) {
            data->scale = 0.0f;
            data->color.a = 0;
        }
    }
}

static void SNpcKoopaFire2Hook(HU3D_MODEL *modelP, MBPARTICLE *particleP,
    Mtx mtx)
{
    MBPARTICLE *sourceP;
    MBPARTICLEDATA *data;
    int alpha;
    int i;

    if (particleP->mode == 0) {
        data = particleP->data;
        for (i = 0; i < particleP->num; i++, data++) {
            data->scale = 0.0f;
            data->color.a = 0;
            data->time = 0;
        }
        particleP->mode = 1;
        particleP->blendMode = MB_PARTICLE_BLEND_ADDCOL;
    }

    sourceP = particleP->hookData;
    memcpy(particleP->data, sourceP->data,
        particleP->num * sizeof(MBPARTICLEDATA));
    data = particleP->data;
    for (i = 0; i < particleP->num; i++, data++) {
        alpha = (int)(0.8f * (float)data->color.a);
        if (alpha > 255) {
            alpha = 255;
        }
        data->color.a = (u8)alpha;
    }
}

static void SNpcKoopaFire3Hook(HU3D_MODEL *modelP, MBPARTICLE *particleP,
    Mtx mtx)
{
    HuVecF axisCross;
    HuVecF axisZ = { 0.0f, 0.0f, 1.0f };
    HuVecF axisX = { 1.0f, 0.0f, 0.0f };
    MBPARTICLEDATA *data;
    const HuVecF *axis;
    Mtx rotMtx;
    float modeScale;
    float blend;
    float speed;
    int i;

    if (particleP->count == 0) {
        data = particleP->data;
        for (i = 0; i < particleP->num; i++, data++) {
            data->scale = 0.0f;
            data->color.a = 0;
            data->time = (s16)-(i >> 3);
        }
        particleP->count = 1;
    }

    data = particleP->data;
    for (i = 0; i < particleP->num; i++, data++) {
        if (particleP->mode != 0) {
            if (data->time < 0) {
                data->time++;
                continue;
            }
            if (data->time == 0) {
                SNpcVecCopy((HuVecF *)particleP->work, &data->vel);
                axis = &axisZ;
                if (SNpcAbsFloat(data->vel.z) >= SNpcAbsFloat(data->vel.y)) {
                    if (SNpcAbsFloat(data->vel.z) >= SNpcAbsFloat(data->vel.x)) {
                        axis = &axisX;
                    }
                }
                PSVECCrossProduct(axis, &data->vel, &axisCross);
                PSMTXRotAxisRad(rotMtx, &data->vel,
                    frandf() * lbl_802C3330[0] * 0.017453292f);
                PSMTXMultVec(rotMtx, &axisCross, &data->accel);
                modeScale = (float)particleP->mode / 256.0f;
                data->time = data->activeF = 10;
                SNpcVecCopy(&particleP->vel, &data->pos);

                speed = frandf() * 0.3f * 100.0f;
                PSVECScale(&data->vel, &axisCross, speed * modeScale);
                PSVECAdd(&data->pos, &axisCross, &data->pos);
                speed = frandf() * 0.05f * 100.0f;
                PSVECScale(&data->accel, &axisCross, speed * modeScale);
                PSVECAdd(&data->pos, &axisCross, &data->pos);

                blend = 25.0f * modeScale;
                PSVECScale(&data->vel, &data->vel, blend);
                blend = 3.0000002f * modeScale;
                PSVECScale(&data->accel, &data->accel, blend);
                data->rot.z = (float)mbRandMod(360);
                data->animBank = (s16)mbRandMod(8);
                data->color.r = data->color.g = data->color.b =
                    100.0f + 155.0f * frandf();
                data->color.a = 0;
                data->speedDecay = (float)(mbRandMod(62) + 58);
                data->scale = 4e+01f;
                data->colorIdx = 4e+01f
                    + (lbl_802C3354[0] * frandf());
            }
        }
        if (data->time <= 0) {
            continue;
        }

        speed = 1.0f
            - ((float)data->time / (float)data->activeF);
        data->time--;
        PSVECAdd(&data->pos, &data->vel, &data->pos);
        PSVECAdd(&data->pos, &data->accel, &data->pos);
        blend = lbl_802C332C[0] * speed;
        if (blend > 1.0f) {
            blend = 1.0f;
        }
        data->color.a = (u8)((float)data->color.a
            + ((data->speedDecay - (float)data->color.a) * blend));
        data->scale += (data->colorIdx - data->scale) * blend;
        if (data->time == 0) {
            data->scale = 0.0f;
            data->color.a = 0;
        }
    }
}

const HuVecF lbl_8021AA64 = { 0.0f, 0.0f, -1.0f };

static void SNpcEffectExec(void)
{
    if (snpcSaveWork->isKoopa == 0) {
        HuVecF pos;
        HuVecF offset;
        int modelId;
        float height;
        int i;

        offset = lbl_8021AA28;
        modelId = (s16)mbObjCreate(SNPC_DONKEY_EFFECT_DATA_MODEL, NULL,
            FALSE);
        mbObjPosGet(snpcWork->objId[0], &pos);
        PSVECAdd(&pos, &offset, &pos);
        SNpcObjMotShiftSet(10);
        for (i = 14; i >= 0; i--) {
            height = (float)i / lbl_802C334C[0];
            height = 100.0f
                * (lbl_802C3350[0] * mbSinDeg(lbl_802C3354[0] * height));
            mbObjPosSet((s16)modelId, pos.x, pos.y + height, pos.z);
            HuPrcVSleep();
        }
        mbObjDispSet((s16)modelId, FALSE);
        SNpcObjMotEndWait();
        mbObjKill((s16)modelId);
    } else {
        SNpcKoopaFireExec();
    }
}

static void SNpcPlayerMoveFunc(int playerNo)
{
    OMOBJ *obj;
    SNPCMOVEWORK *work;
    s16 maxTime;
    MASU *masu;
    MASU *masuPrev;
    HuVecF moveDir;
    HuVecF playerRot;
    HuVecF linkPos;
    HuVecF masuPos;
    Mtx masuMtx;
    BOOL setAngle;
    s16 masuIdNext;
    s16 masuIdPrev;
    int linkNo;
    int linkMasu;
    int masuId;
    float motSpeed;
    float rotY;
    int movePlayerNo;
    int moveTime;

    motSpeed = 1.0f;
    setAngle = FALSE;
    obj = snpcWork->moveObj = omAddObjEx(mbObjMan, SNPC_MOVE_OBJ_PRIORITY, 0,
        0, OM_GRP_NONE,
        SNpcPlayerMoveObjExec);
    work = omObjGetWork(obj, SNPCMOVEWORK);
    masuIdNext = GwPlayer[playerNo].masuIdNext;
    masuIdPrev = GwPlayer[playerNo].masuIdPrev;
    mbPlayerPosGet(playerNo, &obj->trans);
    if (snpcMagic == SNPC_MAGIC) {
        masuId = snpcSaveWork->masuId;
        for (linkNo = 0; linkNo < mbMasuLinkNumGet((s16)masuId); linkNo++) {
            linkMasu = mbMasuLinkGet((s16)masuId, linkNo);
            if ((mbMasuMAttrGet((s16)linkMasu)
                    & mbBranchMAttrGet()) == 0) {
                break;
            }
        }
        mbMasuPosGet((s16)masuId, &masuPos);
        mbMasuPosGet((s16)linkMasu, &linkPos);
        PSVECSubtract(&linkPos, &masuPos, &linkPos);
        linkPos.y = 0.0f;
        PSVECNormalize(&linkPos, &linkPos);
        PSVECScale(&linkPos, &linkPos, lbl_802C3348[0]);
        mbMasuMtxGet((s16)masuId, masuMtx);
        PSMTXMultVec(masuMtx, &linkPos, &obj->rot);
    }
    work->mode = 0;
    maxTime = 20;
    if (masuIdPrev > 0) {
        masuPrev = mbMasuGet(masuIdPrev);
        masu = mbMasuGet(masuIdNext);
        if ((masuPrev->flag & MASU_FLAG_JUMPFROM)
            && (masu->flag & MASU_FLAG_JUMPTO)) {
            work->mode = 1;
        }
        if ((masuPrev->flag & MASU_FLAG_CLIMBFROM)
            && (masu->flag & MASU_FLAG_CLIMBTO)) {
            work->mode = 2;
        }
    }
    switch (work->mode) {
    case 0:
        mbPlayerWorkGet(playerNo)->_unk0C = 1;
        mbPlayerMotionShiftSet(playerNo, 3, 0.0f,
            4.0f, HU3D_MOTATTR_LOOP);
        break;
    case 1:
        mbPlayerWorkGet(playerNo)->_unk0C = 2;
        mbPlayerMotionShiftSet(playerNo, 4, 6.0f,
            lbl_802C332C[0], 0);
        maxTime = 24;
        break;
    case 2:
        mbPlayerWorkGet(playerNo)->_unk0C = 3;
        mbPlayerMotionShiftSet(playerNo, 14, 0.0f,
            4.0f, HU3D_MOTATTR_LOOP);
        maxTime = 100;
        motSpeed = lbl_802C332C[0];
        PSVECSubtract(&obj->rot, &obj->trans, &moveDir);
        maxTime = PSVECMag(&moveDir) / 15.000001f;
        if (obj->trans.y >= obj->rot.y) {
            moveDir.x = -moveDir.x;
            moveDir.y = -moveDir.y;
            moveDir.z = -moveDir.z;
            motSpeed = -motSpeed;
        }
        playerRot.x = playerRot.z = 0.0f;
        playerRot.y = (float)(180.0
            * (atan2(moveDir.x, moveDir.z) / 3.141592653589793));
        mbPlayerRotSetV(playerNo, &playerRot);
        setAngle = TRUE;
        break;
    }
    mbPlayerMotionSpeedSet(playerNo, motSpeed);
    PSVECSubtract(&obj->rot, &obj->trans, &playerRot);
    rotY = (float)(9e+01 - (180.0
        * (atan2(playerRot.z, playerRot.x) / 3.141592653589793)));
    mbPlayerRotYSet(playerNo, rotY);
    obj->scale.x = obj->trans.x;
    obj->scale.y = obj->trans.y;
    obj->scale.z = obj->trans.z;
    work->playerNo = playerNo;
    work->time = 0;
    work->maxTime = maxTime;
    moveTime = work->maxTime;
    movePlayerNo = work->playerNo;
    mbPlayerWorkGet(movePlayerNo)->_unk08 = moveTime;
    GwPlayer[playerNo].moveF = TRUE;
    while (GwPlayer[playerNo].moveF) {
        HuPrcVSleep();
    }
    mbPlayerWorkGet(playerNo)->_unk0C = 0;
    mbPlayerWorkGet(playerNo)->moveEndF = TRUE;
}

static void SNpcPlayerMoveObjExec(OMOBJ *obj)
{
    SNPCMOVEWORK *work;
    float time;

    work = omObjGetWork(obj, SNPCMOVEWORK);
    if (mbExitCheck() || work->killF) {
        GwPlayer[work->playerNo].moveF = FALSE;
        omDelObjEx(HuPrcCurrentGet(), obj);
        snpcWork->moveObj = NULL;
        return;
    }
    work->time++;
    time = (float)work->time / (float)work->maxTime;
    obj->trans.x = obj->scale.x + (time * (obj->rot.x - obj->scale.x));
    obj->trans.y = obj->scale.y + (time * (obj->rot.y - obj->scale.y));
    obj->trans.z = obj->scale.z + (time * (obj->rot.z - obj->scale.z));
    {
        int movePlayerNo;
        int moveTime;

        moveTime = work->maxTime - work->time;
        movePlayerNo = work->playerNo;

        mbPlayerWorkGet(movePlayerNo)->_unk08 = moveTime;
    }
    if (work->time >= work->maxTime) {
        GwPlayer[work->playerNo].moveF = FALSE;
        mbPlayerPosSet(work->playerNo, obj->rot.x, obj->rot.y,
            obj->rot.z);
        omDelObjEx(HuPrcCurrentGet(), obj);
        snpcWork->moveObj = NULL;
        return;
    }
    if (work->mode != 1) {
        mbPlayerPosSet(work->playerNo, obj->trans.x, obj->trans.y,
            obj->trans.z);
        return;
    }
    {
        int movePlayerNo = work->playerNo;

        mbPlayerWorkGet(movePlayerNo)->moveEndF = FALSE;
    }
    if (work->time >= work->maxTime - 2) {
        time = 1.0f;
        {
            int movePlayerNo = work->playerNo;

            mbPlayerWorkGet(movePlayerNo)->moveEndF = TRUE;
        }
    } else {
        time = (float)work->time / (float)(work->maxTime - 2);
    }
    mbPlayerPosSet(work->playerNo, obj->trans.x,
        obj->trans.y + 100.0 * (2.0
            * sin((3.141592653589793 * (180.0f * time))
                / 180.0)), obj->trans.z);
    if (work->time == work->maxTime - 5) {
        mbPlayerMotionShiftSet(work->playerNo, 5, lbl_802C332C[0],
            lbl_802C332C[0], 0);
    }
}

static void SNpcSePlay(int seNo, int unused)
{
    if (snpcSeTbl[seNo][snpcSaveWork->isKoopa] != 0) {
        mbAudFXPlay((s16)snpcSeTbl[seNo][snpcSaveWork->isKoopa]);
    }
}

static void SNpcMotSetNext(void)
{
    const SNPCMOTNEXTDATA *nextMot;
    int frame;
    int i;

    frame = 0;
    nextMot = snpcNextMotTbl[snpcSaveWork->isKoopa];
    nextMot = &nextMot[snpcWork->motNo];
    if (nextMot->motNo == 0) {
        return;
    }
    if (mbObjMotionShiftIDGet(snpcWork->objId[0]) != -1) {
        frame = (int)Hu3DMotionShiftTimeGet(
            mbObjModelIDGet(snpcWork->objId[0]));
    } else {
        frame = (int)mbObjMotionTimeGet(snpcWork->objId[0]);
    }
    i = 0;
    if (snpcWork->motShiftNo >= 0) {
        for (i = 0; i < 3; i++) {
            if (snpcWork->motShiftNo < nextMot->frame[i]) {
                break;
            }
        }
    }
    if (i < 3 && nextMot->frame[i] >= 0 && nextMot->frame[i] <= frame) {
        SNpcSePlay(nextMot->motNo, 0);
    }
    snpcWork->motShiftNo = frame;
}

static void SNpcObjCreate(void)
{
    int modelData[2] = { SNPC_DONKEY_DATA_MODEL, SNPC_KOOPA_DATA_MODEL };
    int i;
    HuVecF pos;

    snpcWork->objId[0] = mbObjCreate(modelData[snpcSaveWork->isKoopa],
        NULL, TRUE);
    mbObjLayerSet(snpcWork->objId[0], 3);
    for (i = 0; i < SNPC_MOTION_NUM; i++) {
        snpcWork->motionId[i] = mbObjMotionCreate(snpcWork->objId[0],
            snpcMotTbl[snpcSaveWork->isKoopa][i].dataNum);
    }
    snpcWork->motNo = -1;
    SNpcObjMotSet(0);
    mbMasuPosGet(snpcSaveWork->masuId, &pos);
    SNpcObjPosSetV(&pos);
    SNpcObjRotSet(0.0f, 0.0f, 0.0f);
    snpcWork->objId[2] = mbObjCreate(mbBoardDataNumGet(SNPC_EFFECT_DATA_MODEL),
        NULL, FALSE);
    mbObjDispSet(snpcWork->objId[2], FALSE);
    mbObjLayerSet(snpcWork->objId[2], 2);
}

static void SNpcObjKill(void)
{
    int i;

    for (i = 0; i < SNPC_OBJECT_NUM; i++) {
        if (snpcWork->objId[i] != 0) {
            mbObjKill(snpcWork->objId[i]);
            snpcWork->objId[i] = 0;
        }
    }
    for (i = 0; i < SNPC_MOTION_SLOT_NUM; i++) {
        snpcWork->motionId[i] = 0;
    }
}

static void SNpcObjDispSet(BOOL dispF)
{
    mbObjDispSet(snpcWork->objId[0], dispF);
}

static void SNpcObjPosSet(float x, float y, float z)
{
    snpcWork->pos.x = x;
    snpcWork->pos.y = y;
    snpcWork->pos.z = z;
    mbObjPosSetV(snpcWork->objId[0], &snpcWork->pos);
}

static void SNpcObjPosSetV(const HuVecF *pos)
{
    SNpcVecCopy(pos, &snpcWork->pos);
    mbObjPosSetV(snpcWork->objId[0], &snpcWork->pos);
}

static void SNpcObjPosGet(HuVecF *pos)
{
    SNpcVecCopy(&snpcWork->pos, pos);
}

static void SNpcObjRotSet(float x, float y, float z)
{
    snpcWork->rot.x = x;
    snpcWork->rot.y = y;
    snpcWork->rot.z = z;
    mbObjRotSetV(snpcWork->objId[0], &snpcWork->rot);
}

static void SNpcObjRotSetV(const HuVecF *rot)
{
    SNpcVecCopy(rot, &snpcWork->rot);
    mbObjRotSetV(snpcWork->objId[0], &snpcWork->rot);
}

static void SNpcObjRotGet(HuVecF *rot)
{
    SNpcVecCopy(&snpcWork->rot, rot);
}

static void SNpcObjMotSet(int motNo)
{
    const SNPCMOTDATA *motData;
    int start;
    int end;

    motData = &snpcMotTbl[snpcSaveWork->isKoopa][motNo];
    mbObjMotionSet(snpcWork->objId[0], snpcWork->motionId[motNo],
        motData->loopF ? HU3D_MOTATTR_LOOP : HU3D_MOTATTR_NONE);
    mbObjMotionSpeedSet(snpcWork->objId[0], motData->speed);
    if (motData->startFrame != 0 || motData->endFrame != 0) {
        start = end = -1;
        if (motData->startFrame != 0) {
            start = motData->startFrame;
        }
        if (motData->endFrame != 0) {
            end = motData->endFrame;
        }
        mbObjMotionStartEndSet(snpcWork->objId[0], start, end);
    }
    snpcWork->motShiftNo = -1;
    snpcWork->motNo = motNo;
}

static void SNpcObjMotShiftSet(int motNo)
{
    const SNPCMOTDATA *motData;
    int start;
    int end;

    if (snpcWork->motNo == motNo) {
        return;
    }
    motData = &snpcMotTbl[snpcSaveWork->isKoopa][motNo];
    start = 0;
    if (motData->startFrame != 0) {
        start = motData->startFrame;
    }
    mbObjMotionShiftSet(snpcWork->objId[0], snpcWork->motionId[motNo],
        (float)motData->startFrame, 8.0f,
        motData->loopF ? HU3D_MOTATTR_LOOP : HU3D_MOTATTR_NONE);
    mbObjMotionSpeedSet(snpcWork->objId[0], motData->speed);
    if (motData->startFrame != 0 || motData->endFrame != 0) {
        end = (int)Hu3DMotionShiftMaxTimeGet(
            mbObjModelIDGet(snpcWork->objId[0]));
        if (motData->endFrame != 0) {
            end = motData->endFrame;
        }
        Hu3DMotionShiftStartEndSet(mbObjModelIDGet(snpcWork->objId[0]),
            (float)start, (float)end);
    }
    snpcWork->motShiftNo = -1;
    snpcWork->motNo = motNo;
}

static BOOL SNpcObjMotEndCheck(void)
{
    BOOL endF;

    endF = FALSE;
    if (mbObjMotionEndCheck(snpcWork->objId[0])
        && mbObjMotionShiftIDGet(snpcWork->objId[0]) == -1) {
        endF = TRUE;
    }
    return endF;
}

static void SNpcObjMotEndWait(void)
{
    while (!SNpcObjMotEndCheck()) {
        HuPrcVSleep();
    }
}

static void SNpcObjMasuSet(int masuId)
{
    HuVecF pos;

    mbMasuPosGet(masuId, &pos);
    pos.y += 5.0f;
    mbObjPosSetV(snpcWork->objId[2], &pos);
    mbObjDispSet(snpcWork->objId[2], TRUE);
    mbObjMotionTimeSet(snpcWork->objId[2], 0.0f);
}

static void SNpcMasuEffDispSet(void)
{
    mbObjDispSet(snpcWork->objId[2], FALSE);
}

static inline int SNpcChanceBranchMask(int masuId)
{
    if (mbBranchAttrCheck(masuId)) {
        return 0;
    }
    return -1;
}

static inline u8 *SNpcChanceLinkAlloc(int linkTblNum)
{
    return HuMemDirectMallocNum(0,
        (linkTblNum + SNPC_CHANCE_BRANCH_PADDING) * sizeof(u32),
        HU_MEMNUM_OVL);
}

u8 *mbMasuChanceCreate(int masuId, int chance, int branchChance)
{
    int linkCount;
    u8 *chanceTbl;
    int nextMasuId;
    s8 *masuState;
    int found;
    SNPCCHANCEPATH *pathTbl;
    SNPCCHANCEINDEX *linkIndex;
    int masuNum;
    int maxPaths;
    u8 *linkTbl;
    int current;
    int currentChance;
    int linkNo;
    int linkNum;
    int linkTblNum;
    u32 *pathMem;
    int pathCapacity;
    int i;
    MASU *masu;
    SNPCCHANCEINDEX *linkEntry;
    SNPCCHANCEPATH *pathCursor;

    maxPaths = SNPC_CHANCE_TBL_SIZE;
    chanceTbl = mbMalloc(SNPC_CHANCE_TBL_SIZE);
    masuState = mbMalloc(SNPC_CHANCE_TBL_SIZE);
    pathMem = HuMemDirectMallocNum(0,
        maxPaths * sizeof(*pathTbl), HU_MEMNUM_OVL);
    pathTbl = (SNPCCHANCEPATH *)pathMem;
    masuNum = mbMasuNumGet();
    for (i = 1; i < masuNum; i++) {
        masuState[i] = (s8)(mbMasuDispCheck((s16)i) | SNpcChanceBranchMask(i));
    }
    if (chance != 0) {
        current = masuId;
        linkCount = 0;
        currentChance = chance;
        pathCursor = pathTbl;
        pathCapacity = maxPaths;
        chanceTbl[current] = (u8)currentChance;
        if (masuState[current] != 0) {
            currentChance--;
        }
        while (pathCursor >= pathTbl) {
            found = FALSE;
            masu = mbMasuGet((s16)current);
            while (linkCount < masu->linkNum && !found) {
                nextMasuId = masu->linkTbl[linkCount++];
                if (masuState[nextMasuId] < 0) {
                    continue;
                }
                if (chanceTbl[nextMasuId] >= currentChance) {
                    continue;
                }
                chanceTbl[nextMasuId] = (u8)currentChance;
                if (pathCapacity <= 0) {
                    continue;
                }
                if (currentChance <= 1 && masuState[nextMasuId] == 0) {
                    continue;
                }
                pathCursor->masuId = (s16)current;
                pathCursor->linkNo = (s16)linkCount;
                pathCursor->chance = (s16)currentChance;
                pathCursor++;
                pathCapacity--;
                current = nextMasuId;
                linkCount = 0;
                if (masuState[nextMasuId] != 0) {
                    currentChance--;
                }
                found = TRUE;
            }
            if (!found) {
                pathCursor--;
                pathCapacity++;
                if (pathCursor >= pathTbl) {
                    current = pathCursor->masuId;
                    linkCount = pathCursor->linkNo;
                    currentChance = pathCursor->chance;
                }
            }
        }
    }
    if (branchChance != 0) {
        linkIndex = mbMalloc(SNPC_CHANCE_TBL_SIZE * sizeof(*linkIndex));
        for (i = 1; i < masuNum; i++) {
            masu = mbMasuGet((s16)i);
            linkNum = masu->linkNum;
            for (linkNo = 0; linkNo < linkNum; linkNo++) {
                linkIndex[masu->linkTbl[linkNo]].count++;
            }
        }
        linkTblNum = 0;
        linkEntry = &linkIndex[1];
        for (i = 1; i < masuNum; i++, linkEntry++) {
            linkEntry->start = (s16)linkTblNum;
            linkTblNum += linkEntry->count;
            linkEntry->count = 0;
        }
        linkTbl = SNpcChanceLinkAlloc(linkTblNum);
        for (i = 1; i < masuNum; i++) {
            masu = mbMasuGet((s16)i);
            linkNum = masu->linkNum;
            for (linkNo = 0; linkNo < linkNum; linkNo++) {
                linkEntry = &linkIndex[masu->linkTbl[linkNo]];
                linkTbl[linkEntry->start + linkEntry->count++] = (u8)i;
            }
        }
        current = masuId;
        linkCount = 0;
        currentChance = branchChance;
        pathCursor = pathTbl;
        pathCapacity = maxPaths;
        chanceTbl[current] = (u8)currentChance;
        if (masuState[current] != 0) {
            currentChance--;
        }
        while (pathCursor >= pathTbl) {
            found = FALSE;
            linkEntry = &linkIndex[current];
            while (linkCount < linkEntry->count && !found) {
                nextMasuId = linkTbl[linkEntry->start + linkCount++];
                if (masuState[current] < 0) {
                    continue;
                }
                if (chanceTbl[nextMasuId] >= currentChance) {
                    continue;
                }
                chanceTbl[nextMasuId] = (u8)currentChance;
                if (pathCapacity <= 0) {
                    continue;
                }
                if (currentChance <= 1 && masuState[current] == 0) {
                    continue;
                }
                pathCursor->masuId = (s16)current;
                pathCursor->linkNo = (s16)linkCount;
                pathCursor->chance = (s16)currentChance;
                pathCursor++;
                pathCapacity--;
                current = nextMasuId;
                linkCount = 0;
                if (masuState[nextMasuId] != 0) {
                    currentChance--;
                }
                found = TRUE;
            }
            if (!found) {
                pathCursor--;
                pathCapacity++;
                if (pathCursor >= pathTbl) {
                    current = pathCursor->masuId;
                    linkCount = pathCursor->linkNo;
                    currentChance = pathCursor->chance;
                }
            }
        }
        HuMemDirectFree(linkTbl);
        HuMemDirectFree(linkIndex);
    }
    HuMemDirectFree(masuState);
    HuMemDirectFree(pathTbl);
    return chanceTbl;
}

void mbMasuChanceKill(void *work)
{
    HuMemDirectFree(work);
}

void mbMasuChanceTypeSet(u8 *chanceTbl, u8 value, int *typeTbl, BOOL inverseF)
{
    int masuNum;
    int masuType;
    BOOL inverseWork;
    int i;
    u8 *chanceTblP;
    int typeNo;

    masuNum = mbMasuNumGet();
    inverseWork = inverseF ? TRUE : FALSE;
    chanceTblP = chanceTbl + 1;
    for (i = 1; i < masuNum; i++, chanceTblP++) {
        if (*chanceTblP == 0) {
            masuType = mbMasuGet(i)->type;
            for (typeNo = 0; typeTbl[typeNo] >= 0; typeNo++) {
                if (masuType == typeTbl[typeNo]) {
                    break;
                }
            }
            if (inverseWork == (typeTbl[typeNo] < 0)) {
                *chanceTblP = (u8)value;
            }
        }
    }
}

void mbMasuChanceFlagSet(u8 *chanceTbl, u8 value, u32 flag, u32 mAttr,
    BOOL inverseF)
{
    u8 *chanceTblP;
    int masuNum;
    BOOL inverseWork;
    int i;
    MASU *masu;

    masuNum = mbMasuNumGet();
    inverseWork = inverseF ? TRUE : FALSE;
    chanceTblP = chanceTbl + 1;
    for (i = 1; i < masuNum; i++, chanceTblP++) {
        if (*chanceTblP == 0) {
            masu = mbMasuGet(i);
            if (inverseWork == (((masu->flag & flag) | (masu->mAttr & mAttr)) == 0)) {
                *chanceTblP = (u8)value;
            }
        }
    }
}

void mbMasuChancePlayerSet(u8 *chanceTbl, int value)
{
    int i;

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        chanceTbl[GwPlayer[i].masuId] = value;
    }
}

int mbMasuChanceSet(u8 *chanceTbl, int masuId)
{
    HuVecF basePos;
    HuVecF pos;
    float distance;
    int masuNum;
    u8 *masuTbl;
    float *distanceTbl;
    int maxNum;
    int i;
    int j;
    float *distanceTblBase;
    int validNum;
    int targetId;
    u8 *masuTblBase;

    maxNum = 28;
    mbMasuPosGet(masuId, &basePos);
    basePos.y = 0.0f;
    if (mbMasuAttrGet(masuId) & MASU_FLAG_START) {
        if (mbRandMod(100) < 50) {
            basePos.x = 1.3e+03f;
            basePos.z = (-3e+03f);
            maxNum = 20;
        } else {
            maxNum = 50;
        }
    }
    masuTbl = HuMemDirectMallocNum(0, SNPC_CHANCE_DISTANCE_BYTES,
        HU_MEMNUM_OVL);
    masuTblBase = masuTbl;
    distanceTbl = HuMemDirectMallocNum(0, SNPC_CHANCE_MASU_BYTES,
        HU_MEMNUM_OVL);
    distanceTblBase = distanceTbl;
    masuNum = mbMasuNumGet();
    for (i = 1, validNum = 0; i < masuNum; i++) {
        if (chanceTbl[i] != 0) {
            continue;
        }
        mbMasuPosGet((s16)i, &pos);
        PSVECSubtract(&pos, &basePos, &pos);
        pos.y = 0.0f;
        pos.z *= 0.8f;
        distance = PSVECMag(&pos);
        if (validNum < maxNum) {
            distanceTblBase[validNum] = distance;
            masuTblBase[validNum++] = (u8)i;
        } else {
            float minDistance;

            minDistance = 1e+06f;
            for (j = 0, targetId = -1; j < validNum; j++) {
                if (distanceTblBase[j] < minDistance) {
                    targetId = j;
                    minDistance = distanceTblBase[j];
                }
            }
            if (minDistance < distance) {
                distanceTblBase[targetId] = distance;
                masuTblBase[targetId] = (u8)i;
            }
        }
    }
    i = 0;
    while (i < 20) {
        j = mbRandMod(validNum);
        targetId = masuTblBase[j];
        if (mbCapMasuDispTypeGet((s16)targetId) == 0) {
            break;
        }
        i++;
    }
    HuMemDirectFree(masuTblBase);
    HuMemDirectFree(distanceTblBase);
    return targetId;
}

static int SNpcDiceValueGet(int masuId, int diceNum, const int *typeTbl,
    u32 mAttr, s16 *pathTbl, float chance)
{
    typedef struct SNPCCHANCEPATHLOCAL {
        s16 masuId;
        s16 linkNo;
        s16 chance;
        u8 unk06;
    } SNPCCHANCEPATHLOCAL;
    typedef struct SNPCCOMPACTPATH {
        u8 chanceF;
        u8 masuId;
        s16 chance;
    } SNPCCOMPACTPATH;
    s8 *masuState;
    s8 *branchCount;
    int nextMasu;
    SNPCCHANCEPATHLOCAL *pathStack;
    SNPCCHANCEPATHLOCAL *path;
    SNPCCOMPACTPATH *candidate;
    SNPCCOMPACTPATH *candidateTbl[64];
    int masuNum;
    int allocationCapacity;
    int type;
    int pathCapacity;
    int candidateNum;
    int minBranch;
    int current;
    int linkNo;
    int chanceCount;
    int pathCount;
    int i;
    int j;
    BOOL advanced;
    MASU *masu;
    int maxStars;
    int selectedIndex;
    SNPCCOMPACTPATH *freeCandidate;
    int branchState;
    int displayState;
    SNPCCOMPACTPATH *newCandidate;

    pathCapacity = diceNum + 10;
    allocationCapacity = pathCapacity + 2;
    masuState = mbMalloc(256);
    pathStack = mbMalloc(allocationCapacity * (int)sizeof(*pathStack));
    branchCount = mbMalloc(allocationCapacity);
    candidateNum = 0;
    for (i = 0; i < 64; i++) {
        candidateTbl[i] = NULL;
    }
    masuNum = mbMasuNumGet();
    for (i = 1; i < masuNum; i++) {

        if (mbBranchAttrCheck(i)) {
            branchState = 0;
        } else {
            branchState = -1;
        }
        if (mbMasuDispCheck((s16)i)) {
            displayState = 1;
        } else {
            displayState = 0;
        }
        masuState[i] = (s8)(displayState | branchState);
    }
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        current = GwPlayer[i].masuId;
        masuState[current] = masuState[current] | 2;
    }
    if (diceNum != 0) {
        current = masuId;
        linkNo = 0;
        chanceCount = 0;
        path = pathStack;
        pathCount = 0;
        minBranch = allocationCapacity;
        while (path >= pathStack) {

            advanced = FALSE;
            masu = mbMasuGet((s16)current);
            while (linkNo < masu->linkNum && !advanced) {
                nextMasu = masu->linkTbl[linkNo++];
                if ((s8)masuState[nextMasu] < 0) {
                    continue;
                }
                if (pathCount < pathCapacity) {
                    branchCount[pathCount]++;
                    path->masuId = (s16)current;
                    path->linkNo = (s16)linkNo;
                    path->chance = (s16)chanceCount;
                    path++;
                    path->unk06 = 0;
                    pathCount++;
                    current = nextMasu;
                    linkNo = 0;
                    if (masuState[current] & 1) {
                        chanceCount++;
                    }
                    if ((masuState[current] & 14) == 0
                        && (mAttr & mbMasuMAttrGet((s16)current)) == 0) {
                        type = mbMasuTypeGet((s16)current);

                        for (j = 0; typeTbl[j] >= 0; j++) {
                            if (type == typeTbl[j]) {
                                path->unk06 = 1;
                                break;
                            }
                        }
                    }
                    advanced = TRUE;
                } else {
                    i = 1;
                    j = 0;
                    for (;
                        i < pathCount && pathStack[i].chance <= diceNum;
                        i++) {
                        if (pathStack[i].unk06 != 0) {
                            j++;
                        }
                    }
                    if (j != 0 && candidateNum < 64) {

                        newCandidate = HuMemDirectMallocNum(0,
                            (pathCount + 1) * (int)sizeof(*candidate),
                            HU_MEMNUM_OVL);
                        candidate = newCandidate;
                        candidateTbl[candidateNum++] = candidate;
                        for (i = 1; i < pathCount; i++, candidate++) {
                            candidate->masuId = (u8)pathStack[i].masuId;
                            candidate->chanceF = pathStack[i].unk06;
                            candidate->chance = pathStack[i].chance;
                        }
                        candidate->chance = -1;
                        for (i = 0; i < pathCount - 1; i++) {
                            if (branchCount[i] > 1) {
                                break;
                            }
                        }
                        if (minBranch > i) {
                            minBranch = i;
                        }
                    }
                }
            }
            if (!advanced) {
                path--;
                pathCount--;
                if (path >= pathStack) {
                    current = path->masuId;
                    linkNo = path->linkNo;
                    chanceCount = path->chance;
                }
            }
        }
    }
    chanceCount = 0;
    if (candidateNum != 0) {
        int score[64];
        int playerDistance[GW_PLAYER_MAX];
        int playerFixed[GW_PLAYER_MAX];
        int minScore;
        maxStars = 0;

        for (j = 0; j < GW_PLAYER_MAX; j++) {
            nextMasu = mbPlayerStarGet(j);

            if (maxStars < nextMasu) {
                maxStars = nextMasu;
            }
            playerFixed[j] = 0;
        }
        {
            candidate = candidateTbl[0];
            for (i = 0; i < minBranch && candidate->chance >= 0;
                i++, candidate++) {
                if ((masuState[candidate->masuId] & 14) == 0) {
                    continue;
                }
                for (j = 0; j < GW_PLAYER_MAX; j++) {
                    if (GwPlayer[j].masuId == candidate->masuId) {
                        playerFixed[j] = 1;
                    }
                }
            }
            for (i = 0; i < candidateNum; i++) {
                for (j = 0; j < GW_PLAYER_MAX; j++) {
                    playerDistance[j] = 0;
                }
                candidate = candidateTbl[i] + minBranch;
                while (candidate->chance >= 0) {
                    if ((masuState[candidate->masuId] & 14) != 0) {
                        for (j = 0; j < GW_PLAYER_MAX; j++) {
                            if (GwPlayer[j].masuId
                                != candidate->masuId) {
                                continue;
                            }
                            if (playerDistance[j] != 0) {
                                continue;
                            }
                            if (playerFixed[j] != 0) {
                                continue;
                            }
                            if (candidate->chance < diceNum + 1) {
                                playerDistance[j] = candidate->chance;
                            } else {
                                playerFixed[j] = 1;
                            }
                        }
                    }
                    candidate++;
                }
                minScore = 1000000;
                for (j = 0; j < GW_PLAYER_MAX; j++) {
                    if (playerDistance[j] != 0) {
                        if (!snpcSaveWork->isKoopa) {
                            nextMasu = playerDistance[j]
                                + mbPlayerStarGet(j) * 20;
                        } else {
                            nextMasu = playerDistance[j]
                                + (maxStars - mbPlayerStarGet(j)) * 20;
                        }
                        if (minScore > nextMasu) {
                            minScore = nextMasu;
                        }
                    }
                }
                score[i] = minScore;
            }
            for (i = 0; i < candidateNum - 1; i++) {
                for (j = i + 1; j < candidateNum; j++) {
                    if (score[i] > score[j]) {
                        nextMasu = score[i];
                        candidate = candidateTbl[i];

                        score[i] = score[j];
                        candidateTbl[i] = candidateTbl[j];
                        score[j] = nextMasu;
                        candidateTbl[j] = candidate;
                    }
                }
            }
            {
                minScore = 10000000;
                i = 0;
                while (i < candidateNum && minScore >= score[i]) {
                    minScore = score[i];
                    i++;
                }
                nextMasu = i;
                candidateNum = mbRandMod(nextMasu);
                {

                    candidate = candidateTbl[candidateNum];
                    i = 0;
                    j = 0;
                    for (;
                        candidate[i].chance >= 0
                            && candidate[i].chance <= diceNum;
                        i++) {
                        if (candidate[i].chanceF != 0) {
                            j++;
                        }
                    }
                    selectedIndex = -1;
                    for (i = 0; j > 0; i++) {
                        pathTbl[i] = (s16)candidate[i].masuId;
                        if (candidate[i].chanceF != 0) {
                            j--;
                            if (masuState[pathTbl[i]] > 0
                                && candidate[i].chance >= (int)(0.5f
                                    + ((float)diceNum * chance))) {
                                j = 0;
                            } else {
                                selectedIndex = i;
                            }
                        }
                    }
                    if (selectedIndex > 0
                        && mbCapMasuDispTypeGet(pathTbl[i - 1]) != 0
                        && mbCapMasuDispTypeGet(pathTbl[selectedIndex]) == 0) {
                        i = selectedIndex + 1;
                    }
                    chanceCount = candidate[i - 1].chance;
                    pathTbl[i] = 0;
                }
            }
        }
    }
    for (i = 0; i < 64; i++) {
        if (candidateTbl[i]) {

            freeCandidate = candidateTbl[i];
            HuMemDirectFree(freeCandidate);
            candidateTbl[i] = NULL;
        }
    }
    HuMemDirectFree(masuState);
    HuMemDirectFree(pathStack);
    HuMemDirectFree(branchCount);
    return chanceCount;
}

int mbSNpcMasuStarNextGet(int masuId, int type, int *linkNoTbl, u32 attr)
{
    u8 *chanceTbl;
    int targetId;

    chanceTbl = mbMasuChanceCreate(masuId, type + 1, type / 4 + 1);
    mbMasuChanceTypeSet(chanceTbl, SNPC_CHANCE_EXCLUDE, linkNoTbl, TRUE);
    mbMasuChanceFlagSet(chanceTbl, SNPC_CHANCE_EXCLUDE, 0, attr, FALSE);
    mbMasuChancePlayerSet(chanceTbl, SNPC_CHANCE_EXCLUDE);
    targetId = mbMasuChanceSet(chanceTbl, masuId);
    HuMemDirectFree(chanceTbl);
    return targetId;
}

static void SNpcStarCreate(int type, BOOL loopF, HuVecF *pos)
{
    OMOBJ *obj;

    if (type == 0) {
        obj = omAddObjEx(mbObjMan, SNPC_MOVE_OBJ_PRIORITY,
            SNPC_STAR_OBJ_GROUP, 0, OM_GRP_NONE, SNpcStarObjExec);
        snpcWork->starObj = obj;
        omObjGetWork(snpcWork->starObj, SNPCSTAREFFWORK)->loopF = FALSE;
    } else {
        obj = omAddObjEx(mbObjMan, SNPC_MOVE_OBJ_PRIORITY,
            SNPC_STAR_OBJ_GROUP, 0, OM_GRP_NONE, SNpcStarObjExec);
        snpcWork->starObj = obj;
        omObjGetWork(snpcWork->starObj, SNPCSTAREFFWORK)->loopF = TRUE;
    }

    omSetStatBit(obj, SNPC_MOVE_OBJ_PRIORITY);
    if (loopF) {
        omObjGetWork(snpcWork->starObj, SNPCSTAREFFWORK)->unk00 = TRUE;
    }
    SNpcVecCopy(pos, &obj->trans);

    obj->mdlId[0] = mbObjCreate(mbBoardDataNumGet(starMdlTbl[type]),
        NULL, TRUE);
    mbObjDispSet(obj->mdlId[0], FALSE);
    obj->mdlId[1] = SNpcStarEffCreate(
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_effect, 1),
            HU_MEMNUM_OVL, HEAP_MODEL)), type);
}

static void SNpcStarWait(void)
{
    while (snpcWork->starObj != NULL) {
        HuPrcVSleep();
    }
}

static void SNpcStarObjExec(OMOBJ *obj)
{
    SNPCSTAREFFWORK *work;
    float time;

    work = omObjGetWork(obj, SNPCSTAREFFWORK);
    if (mbExitCheck() || work->killF) {
        mbObjKill(obj->mdlId[0]);
        SNpcStarEffKill(obj->mdlId[1]);
        obj->mdlId[0] = 0;
        obj->mdlId[1] = 0;
        omDelObjEx(HuPrcCurrentGet(), obj);
        snpcWork->starObj = NULL;
        return;
    }
    if (!work->initF) {
        work->initF = TRUE;
        work->time = 0;
        work->maxTime = 0;
        work->effectNo = 0;
        obj->trans.y += 3e+02f;
        if (!work->unk00) {
            obj->trans.y += 7e+01f;
        }
        obj->rot.z = 0.0f;
        mbObjDispSet(obj->mdlId[0], TRUE);
        if (!work->loopF) {
            work->soundId = mbAudFXPlay(MSM_SE_BRD00_91);
        } else {
            work->soundId = mbAudFXPlay(MSM_SE_BRD00_114);
        }
    }
    work->time++;
    if (work->time > work->maxTime) {
        work->time = work->maxTime;
    }
    switch (work->effectNo) {
    case 0:
        work->maxTime = 120;
        work->effectNo++;
    case 1:
        time = (float)work->time / (float)work->maxTime;
        time = 1.0f - time;
        obj->rot.z = 5e+02f * time;
        if (work->time >= work->maxTime) {
            work->effectNo++;
            work->time = 0;
            work->maxTime = 60;
        }
        break;
    case 2:
        time = (float)work->time / (float)work->maxTime;
        obj->rot.z = 100.0f * (0.2f
            * mbSinDeg(180.0f + (lbl_802C3330[0] * time)));
        if (work->time >= work->maxTime) {
            work->effectNo++;
            work->time = 0;
            work->maxTime = 30;
            mbParManAttrSet((int)obj->mdlId[1], 1);
            if (work->soundId > 0) {
                mbAudFXStop(work->soundId);
                work->soundId = -1;
            }
            if (!work->loopF) {
                mbAudFXPlay(MSM_SE_BRD00_93);
            } else {
                mbAudFXPlay(MSM_SE_BRD00_93);
            }
        }
        break;
    case 3:
        obj->rot.y += 2e+01f;
        if (obj->rot.y >= lbl_802C3330[0]) {
            obj->rot.y -= lbl_802C3330[0];
        }
        time = (float)work->time / (float)work->maxTime;
        time = 1.0f - time;
        if (time <= 0.0f) {
            time = 1e-05f;
        }
        obj->scale.z = time;
        obj->scale.x = time;
        if (work->unk00) {
            obj->rot.z -= 5.0000005f;
        }
        if (work->time >= work->maxTime) {
            work->effectNo++;
            work->killF = TRUE;
        }
        break;
    }
    mbObjPosSet(obj->mdlId[0], obj->trans.x,
        obj->trans.y + obj->rot.z, obj->trans.z);
    mbObjRotSet(obj->mdlId[0], 0.0f, obj->rot.y,
        0.0f);
    mbObjScaleSet(obj->mdlId[0], obj->scale.x, obj->scale.y,
        obj->scale.z);
    mbParManPosSet((HU3D_MODELID)(int)obj->mdlId[1], obj->trans.x,
        obj->trans.y + obj->rot.z, obj->trans.z);
}

static HU3D_MODELID SNpcStarEffCreate(ANIMDATA *anim, int type)
{
    HU3D_MODELID modelId;

    modelId = mbParManCreate(anim, SNPC_STAR_PARTICLE_MAX,
        &snpcStarEffParam[type]);
    mbParManAttrSet((int)modelId, SNPC_STAR_PARTICLE_ATTR);
    mbParManRotSet((int)modelId, 90.0f, 0.0f,
        0.0f);
    mbParticleBlendModeSet((int)modelId, 1);
    Hu3DModelLayerSet(modelId, 5);
    Hu3DModelCameraSet(modelId, 1);
    return modelId;
}

static void SNpcStarEffKill(s16 parManId)
{
    mbParManKill((s16)parManId);
}

static void GetStarTexTevStage(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material,
    int *tevStageNum, int *texGenNum)
{
    u16 matHiliteF;
    HSF_BITMAP *bitmap;
    HSF_CONSTDATA *shadowData;
    HU3D_ATTR_ANIM *animWork;
    HSF_ATTRIBUTE *attribute;
    HSF_OBJECT *object;
    HU3D_MODEL *model;
    u16 lightOnF;
    u16 projMask;
    u16 i;
    s16 specialAttrNo;
    int bumpAttrNo;
    BOOL texBlendF;
    u32 flags;
    u32 shineF;
    u16 tevStage;
    u16 texGen;

    specialAttrNo = -1;
    object = drawObj->object;
    model = drawObj->model;
    for (i = 0; i < material->attrNum; i++) {
        attribute = &object->mesh.attribute[material->attr[i]];
        bitmap = attribute->bitmap;
        texCol[i].a = 0;
        if (attribute->animWorkP) {
            animWork = attribute->animWorkP;
            if ((animWork->attr & HU3D_ATTRANIM_ATTR_ANIM2D)
                && !(Hu3DTexAnimData[animWork->animId].attr & HU3D_ANIM_ATTR_NOUSE)) {
                continue;
            }
            if (animWork->attr & HU3D_ATTRANIM_ATTR_BMPANIM) {
                bitmap = animWork->bitMapPtr;
                switch (bitmap->dataFmt) {
                    case HSF_BMPFMT_I4:
                    case HSF_BMPFMT_I8:
                    case HSF_BMPFMT_IA4:
                    case HSF_BMPFMT_IA8:
                        texCol[i].a = 1;
                        break;
                    case HSF_BMPFMT_CI_IA8:
                        texCol[i].a = 2;
                        break;
                }
            } else {
                continue;
            }
            continue;
        }
        switch (bitmap->dataFmt) {
            case HSF_BMPFMT_I4:
            case HSF_BMPFMT_I8:
            case HSF_BMPFMT_IA4:
            case HSF_BMPFMT_IA8:
                texCol[i].a = 1;
                break;
            case HSF_BMPFMT_CI_IA8:
                texCol[i].a = 2;
                break;
        }
    }
    flags = object->flags | material->flags;
    if (material->vtxMode == 2 || material->vtxMode == 3) {
        matHiliteF = TRUE;
    } else {
        matHiliteF = FALSE;
        if (material->vtxMode == 0 || material->vtxMode == 5) {
            lightOnF = FALSE;
        } else {
            lightOnF = TRUE;
        }
    }
    if (Hu3DShineF && lightOnF) {
        shineF = TRUE;
    } else {
        shineF = FALSE;
    }
    if (material->attrNum == 1) {
        texGen = tevStage = 1;
        attribute = &object->mesh.attribute[material->attr[0]];
        if (1.0f == attribute->unk20) {
            if (attribute->unk8[2] == 0) {
                tevStage++;
            } else if (!(model->attr & HU3D_ATTR_TOON_MAP)) {
                if (texCol[0].a == 1) {
                    tevStage++;
                } else if (texCol[0].a == 2) {
                    tevStage++;
                }
            }
        }
        if (model->attr & HU3D_ATTR_TOON_MAP) {
            texGen++;
            tevStage++;
        }
        if (0.0f != material->refAlpha) {
            texGen++;
            tevStage++;
        }
        if (shineF) {
            tevStage++;
        }
        shadowData = drawObj->object->constData;
        if (Hu3DShadowF && shadowNum
            && (shadowData->attr & HU3D_CONST_SHADOW_MAP)) {
            HSF_CONSTDATA *shadowTpData;

            shadowTpData = object->constData;
            if (shadowTpData->attr & HU3D_CONST_SHADOW_MAP_TPLVL) {
                tevStage++;
            }
            tevStage++;
            texGen++;
        }
        if (matHiliteF) {
            if ((model->attr & HU3D_ATTR_HILITE)
                || (flags & HSF_MATERIAL_HILITE)) {
                tevStage++;
                texGen++;
                matHiliteF = FALSE;
            } else {
                if (1.0f != attribute->unk20) {
                    texGen++;
                }
                tevStage++;
            }
        } else if (0.0f != material->invAlpha) {
            tevStage++;
        }
        if (model->projBit) {
            for (i = 0, projMask = 1; i < 4; i++, projMask <<= 1) {
                if (projMask & model->projBit) {
                    texGen++;
                    tevStage += 2;
                }
            }
        }
    } else {
        texBlendF = FALSE;
        texGen = 0;
        bumpAttrNo = -1;
        i = tevStage = 0;
        for (; i < material->attrNum; i++) {
            attribute = &object->mesh.attribute[material->attr[i]];
            if (0.0f != attribute->nbtTpLvl) {
                tevStage++;
                bumpAttrNo = i;
                texGen++;
                texBlendF = TRUE;
            } else if (1.0f != attribute->unk20) {
                specialAttrNo = i;
                continue;
            } else {
                texGen++;
                if (i == 0) {
                    if (texCol[i].a == 1 || texCol[i].a == 2) {
                        tevStage++;
                    }
                } else if (texBlendF) {
                    texBlendF = FALSE;
                } else if (attribute->unk8[2] == 0) {
                    tevStage++;
                } else if (texCol[i].a == 1 || texCol[i].a == 2) {
                    tevStage++;
                }
            }
            tevStage++;
        }
        if (model->attr & HU3D_ATTR_TOON_MAP) {
            texGen++;
            tevStage++;
        }
        if (0.0f != material->refAlpha) {
            if (specialAttrNo != -1) {
                tevStage++;
                texGen++;
            }
            tevStage++;
            texGen++;
        }
        if (shineF) {
            tevStage++;
        }
        shadowData = drawObj->object->constData;
        if (Hu3DShadowF && shadowNum
            && (shadowData->attr & HU3D_CONST_SHADOW_MAP)) {
            HSF_CONSTDATA *shadowTpData;

            shadowTpData = object->constData;
            if (shadowTpData->attr & HU3D_CONST_SHADOW_MAP_TPLVL) {
                tevStage++;
            }
            tevStage++;
            texGen++;
        }
        if (matHiliteF) {
            if ((model->attr & HU3D_ATTR_HILITE)
                || (flags & HSF_MATERIAL_HILITE)) {
                if (specialAttrNo != -1) {
                    tevStage++;
                    texGen++;
                }
                tevStage++;
                texGen++;
                matHiliteF = FALSE;
            } else {
                if (specialAttrNo != -1) {
                    texGen++;
                }
                tevStage++;
            }
        } else if (0.0f != material->invAlpha) {
            tevStage++;
        }
        if (model->projBit) {
            for (i = 0, projMask = 1; i < 4; i++, projMask <<= 1) {
                if (projMask & model->projBit) {
                    texGen++;
                    tevStage += 2;
                }
            }
        }
        if (bumpAttrNo != -1) {
            texGen++;
        }
    }
    *tevStageNum = (u16)tevStage;
    *texGenNum = (u16)texGen;
}

static void GetStarNoTexTevStage(HU3D_DRAW_OBJ *drawObj,
    HSF_MATERIAL *material, int *tevStageNum, int *texGenNum)
{
    HSF_OBJECT *object;
    s16 matHiliteF;
    HSF_CONSTDATA *shadowData;
    HSF_CONSTDATA *shadowTpData;
    HU3D_MODEL *model;
    u32 flags;
    s16 i;
    s16 projMask;
    int tevStage;
    int texGen;

    tevStage = 1;
    texGen = 0;
    object = drawObj->object;
    model = drawObj->model;
    flags = object->flags | material->flags;
    if (material->vtxMode == 2 || material->vtxMode == 3) {
        matHiliteF = TRUE;
    } else {
        matHiliteF = FALSE;
    }
    if (model->attr & HU3D_ATTR_TOON_MAP) {
        texGen++;
    }
    if (0.0f != material->refAlpha) {
        tevStage++;
        texGen++;
    }
    shadowData = drawObj->object->constData;
    if (Hu3DShadowF && shadowNum
        && (shadowData->attr & HU3D_CONST_SHADOW_MAP)) {
        shadowTpData = object->constData;
        if (shadowTpData->attr & HU3D_CONST_SHADOW_MAP_TPLVL) {
            tevStage++;
        }
        tevStage++;
        texGen++;
    }
    if (matHiliteF) {
        if ((model->attr & HU3D_ATTR_HILITE)
            || (flags & HSF_MATERIAL_HILITE)) {
            texGen++;
        }
        tevStage++;
    } else if (0.0f != material->invAlpha) {
        tevStage++;
    }
    if (model->projBit) {
        for (i = 0, projMask = 1; i < 4; i++, projMask <<= 1) {
            if (projMask & model->projBit) {
                texGen++;
                tevStage += 2;
            }
        }
    }
    *tevStageNum = (s16)tevStage;
    *texGenNum = (s16)texGen;
}

void mbObjStarTevStageSet(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material,
    int *tevStageNum, int *texGenNum)
{
    if (material->attrNum == 0) {
        Hu3DTevStageNoTexSet(drawObj, material);
        GetStarNoTexTevStage(drawObj, material, tevStageNum, texGenNum);
    } else {
        Hu3DTevStageTexSet(drawObj, material);
        GetStarTexTevStage(drawObj, material, tevStageNum, texGenNum);
    }
}

void mbObjFadeCreate(MBMODELID modelId, HuVecF *pos)
{
    int hu3DModelId;
    MBOBJFADEWORK *work;
    HU3D_MODEL *model;
    HSF_DATA *hsf;
    int i;

    hu3DModelId = mbObjModelIDGet((int)modelId);
    model = &Hu3DData[hu3DModelId];
    if (model->attr & HU3D_ATTR_LINK) {
        work = mbMallocNum(sizeof(*work), model->mallocNoLink);
        model->hookData = work;
    } else {
        work = mbMallocNum(sizeof(*work), model->mallocNo);
        model->hookData = work;
    }
    work->magic = MBOBJ_FADE_WORK_MAGIC;
    SNpcVecCopy(pos, &work->pos);
    work->alpha = 1.0f;
    work->color.r = work->color.g = work->color.b = 255;
    Hu3DModelMatHookSet(hu3DModelId, FadeMatHook);
    hsf = model->hsf;
    for (i = 0; i < hsf->materialNum; i++) {
        hsf->material[i].flags |= HSF_MATERIAL_MATHOOK;
    }
    work->anim = HuSprAnimRead(HuDataSelHeapReadNum(
        mbBoardDataNumGet(SNPC_DATA_FADE_TEXTURE), HU_MEMNUM_OVL, HEAP_MODEL));
}

void mbObjFadeKill(MBMODELID modelId)
{
    int hu3DModelId;
    MBOBJFADEWORK *work;
    HSF_DATA *hsf;
    HU3D_MODEL *model;
    int i;

    hu3DModelId = mbObjModelIDGet((int)modelId);
    model = &Hu3DData[hu3DModelId];
    work = model->hookData;
    Hu3DModelMatHookSet(hu3DModelId, NULL);
    hsf = model->hsf;
    for (i = 0; i < hsf->materialNum; i++) {
        hsf->material[i].flags &= ~HSF_MATERIAL_MATHOOK;
    }
    HuSprAnimKill(work->anim);
    HuMemDirectFree(work);
    model->hookData = NULL;
}

static void FadeMatHook(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material)
{
    MBOBJFADEWORK *work;
    Mtx texMtx;
    Mtx workMtx;
    float alpha;
    int tevStage;
    int texGen;
    int texCoord;

    work = drawObj->model->hookData;
    if (material->attrNum == 0) {
        Hu3DTevStageNoTexSet(drawObj, material);
        GetStarNoTexTevStage(drawObj, material, &tevStage, &texGen);
    } else {
        Hu3DTevStageTexSet(drawObj, material);
        GetStarTexTevStage(drawObj, material, &tevStage, &texGen);
    }
    HuSprTexLoad(work->anim, 0, GX_TEXMAP4, GX_CLAMP, GX_CLAMP, GX_LINEAR);
    PSMTXInverse(Hu3DCameraMtx, texMtx);
    PSMTXConcat(texMtx, drawObj->matrix, texMtx);
    PSMTXTrans(workMtx, -work->pos.x, -work->pos.y, -work->pos.z);
    PSMTXConcat(workMtx, texMtx, texMtx);
    mbMtxRot(workMtx, work->rot.x, work->rot.y, work->rot.z);
    PSMTXInverse(workMtx, workMtx);
    PSMTXConcat(workMtx, texMtx, texMtx);
    alpha = work->alpha;
    if (alpha < 1e-06f) {
        alpha = 1e-06f;
    }
    PSMTXScale(workMtx, 0.001f, (-0.01f) / alpha,
        1.0f);
    PSMTXConcat(workMtx, texMtx, texMtx);
    texMtx[1][3] += 0.96875f;
    GXLoadTexMtxImm(texMtx, GX_TEXMTX4, GX_MTX2x4);
    GXSetNumTexGens(texGen + 1);
    GXSetNumTevStages(tevStage + 1);
    texCoord = texGen;
    GXSetTexCoordGen2(texCoord, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX4,
        GX_FALSE, GX_PTIDENTITY);
    GXSetTevOrder(tevStage, texGen, GX_TEXMAP4, GX_COLOR_NULL);
    GXSetTevKColor(GX_KCOLOR3, work->color);
    GXSetTevKColorSel(tevStage, GX_TEV_KCSEL_K3);
    GXSetTevColorIn(tevStage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST,
        GX_CC_CPREV);
    GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV,
        GX_CA_ZERO);
    GXSetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    if ((drawObj->object->flags | material->flags)
        & (HSF_MATERIAL_NEAR | HSF_MATERIAL_DISABLE_ZWRITE)) {
        GXSetAlphaCompare(GX_GEQUAL, 128, GX_AOP_OR, GX_GEQUAL, 128);
    } else {
        GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
    }
}

void mbObjFadeTexRotSet(MBMODELID modelId, HuVecF *pos, HuVecF *rot)
{
    int hu3DModelId;
    MBOBJFADEWORK *work;
    HU3D_MODEL *model;

    hu3DModelId = mbObjModelIDGet((int)modelId);
    model = &Hu3DData[hu3DModelId];
    work = model->hookData;
    SNpcVecCopy(pos, &work->pos);
    SNpcVecCopy(rot, &work->rot);
}

void mbObjFadeTexColorSet(MBMODELID modelId, u8 r, u8 g, u8 b, float alpha)
{
    int hu3DModelId;
    MBOBJFADEWORK *work;
    HU3D_MODEL *model;

    hu3DModelId = mbObjModelIDGet((int)modelId);
    model = &Hu3DData[hu3DModelId];
    work = model->hookData;
    work->color.r = (int)r;
    work->color.g = (int)g;
    work->color.b = (int)b;
    work->alpha = alpha;
}

void mbObjMetalCreate(MBMODELID modelId)
{
    int hu3DModelId;
    MBOBJMETALWORK *work;
    HSF_DATA *hsf;
    HU3D_MODEL *model;
    int i;

    hu3DModelId = mbObjModelIDGet((int)modelId);
    model = &Hu3DData[hu3DModelId];
    if (model->attr & HU3D_ATTR_LINK) {
        work = mbMallocNum(sizeof(*work), model->mallocNoLink);
        model->hookData = work;
    } else {
        work = mbMallocNum(sizeof(*work), model->mallocNo);
        model->hookData = work;
    }
    work->magic = MBOBJ_METAL_WORK_MAGIC;
    work->tpLvl = 1.0f;
    work->shadowColor.r = work->shadowColor.g = work->shadowColor.b = 255;
    work->hiliteColor.r = work->hiliteColor.g = work->hiliteColor.b = 255;
    work->shadowColor.r = 129;
    work->shadowColor.g = 255;
    work->shadowColor.b = 174;
    work->hiliteColor.r = 202;
    work->hiliteColor.g = 87;
    work->hiliteColor.b = 255;
    Hu3DModelMatHookSet(hu3DModelId, MetalMatHook);
    hsf = model->hsf;
    for (i = 0; i < hsf->materialNum; i++) {
        hsf->material[i].flags |= HSF_MATERIAL_MATHOOK;
    }
    work->anim[0] = HuSprAnimRead(HuDataSelHeapReadNum(
        mbBoardDataNumGet(SNPC_DATA_METAL_TEXMAP4), HU_MEMNUM_OVL, HEAP_MODEL));
    work->anim[1] = HuSprAnimRead(HuDataSelHeapReadNum(
        mbBoardDataNumGet(SNPC_DATA_METAL_TEXMAP5), HU_MEMNUM_OVL, HEAP_MODEL));
}

BOOL mbObjMetalKill(MBMODELID modelId)
{
    int hu3DModelId;
    MBOBJMETALWORK *work;
    HSF_DATA *hsf;
    HU3D_MODEL *model;
    int i;

    hu3DModelId = mbObjModelIDGet((int)modelId);
    model = &Hu3DData[hu3DModelId];
    if (model->hookData == NULL) {
        return FALSE;
    }
    work = model->hookData;
    if (work->magic != MBOBJ_METAL_WORK_MAGIC) {
        return FALSE;
    }
    Hu3DModelMatHookSet(hu3DModelId, NULL);
    hsf = model->hsf;
    for (i = 0; i < hsf->materialNum; i++) {
        hsf->material[i].flags &= ~HSF_MATERIAL_MATHOOK;
    }
    HuSprAnimKill(work->anim[0]);
    HuSprAnimKill(work->anim[1]);
    HuMemDirectFree(work);
    model->hookData = NULL;
    return TRUE;
}

void mbObjMetalTPLvlSet(MBMODELID modelId, float tpLvl)
{
    int hu3DModelId;
    MBOBJMETALWORK *work;
    HU3D_MODEL *model;

    hu3DModelId = mbObjModelIDGet((int)modelId);
    model = &Hu3DData[hu3DModelId];
    work = model->hookData;
    work->tpLvl = tpLvl;
}

void mbObjMetalColorSet(MBMODELID modelId, GXColor shadowColor,
    GXColor hiliteColor)
{
    int hu3DModelId;
    MBOBJMETALWORK *work;
    HU3D_MODEL *model;

    hu3DModelId = mbObjModelIDGet((int)modelId);
    model = &Hu3DData[hu3DModelId];
    work = model->hookData;
    work->shadowColor = shadowColor;
    work->hiliteColor = hiliteColor;
}

static void MetalMatHook(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material)
{
    MBOBJMETALWORK *work;
    HuVecF lightDir;
    HuVecF axis;
    HuVecF viewDir;
    Mtx texMtx;
    Mtx workMtx;
    float angle;
    float absInput;
    float absZ;
    float lightZ;
    float angleResult;
    float angleFloat;
    int tevStage;
    int texGen;
    int texCoord;
    int texCoordNext;
    GXColor color;

    work = drawObj->model->hookData;
    if (material->attrNum == 0) {
        Hu3DTevStageNoTexSet(drawObj, material);
        GetStarNoTexTevStage(drawObj, material, &tevStage, &texGen);
    } else {
        Hu3DTevStageTexSet(drawObj, material);
        GetStarTexTevStage(drawObj, material, &tevStage, &texGen);
    }
    if (work->tpLvl <= 0.05f) {
        return;
    }
    HuSprTexLoad(work->anim[0], 0, GX_TEXMAP4, GX_REPEAT, GX_REPEAT,
        GX_LINEAR);
    HuSprTexLoad(work->anim[1], 0, GX_TEXMAP5, GX_REPEAT, GX_REPEAT,
        GX_LINEAR);
    PSMTXCopy(drawObj->matrix, texMtx);
    PSMTXScale(workMtx, 0.5f / drawObj->scale.x,
        (-0.5f) / drawObj->scale.y,
        0.5f / drawObj->scale.z);
    PSMTXConcat(workMtx, texMtx, texMtx);
    texMtx[0][3] = texMtx[1][3] = 0.5f;
    GXLoadTexMtxImm(texMtx, GX_TEXMTX4, GX_MTX2x4);
    PSMTXCopy(drawObj->matrix, texMtx);
    viewDir = lbl_8021AA64;
    PSMTXMultVecSR(Hu3DCameraMtx, &Hu3DGlobalLight[0].dir, &lightDir);
    C_VECHalfAngle(&viewDir, &lightDir, &lightDir);
    absInput = lightDir.z;
    absInput = __fabsf(absInput);
    absZ = absInput;
    if (absZ < 0.999f) {
        lightZ = lightDir.z;
        angleResult = acos(lightZ);
        angleFloat = angleResult;
        angle = angleFloat;
        PSVECCrossProduct(&viewDir, &lightDir, &axis);
        PSMTXRotAxisRad(workMtx, &axis, angle);
        PSMTXConcat(workMtx, texMtx, texMtx);
    }
    PSMTXScale(workMtx, 0.5f / drawObj->scale.x,
        (-0.5f) / drawObj->scale.y,
        0.5f / drawObj->scale.z);
    PSMTXConcat(workMtx, texMtx, texMtx);
    texMtx[0][3] = 0.5f;
    texMtx[1][3] = 0.5f;
    GXLoadTexMtxImm(texMtx, GX_TEXMTX5, GX_MTX2x4);
    GXSetNumTexGens(texGen + 2);
    GXSetNumTevStages(tevStage + 3);
    texCoord = texGen;
    GXSetTexCoordGen2(texCoord, GX_TG_MTX2x4, GX_TG_NRM, GX_TEXMTX4,
        GX_FALSE, GX_PTIDENTITY);
    texCoordNext = texGen + 1;
    GXSetTexCoordGen2(texCoordNext, GX_TG_MTX2x4, GX_TG_NRM, GX_TEXMTX5,
        GX_FALSE, GX_PTIDENTITY);
    GXSetTevOrder(tevStage, texGen, GX_TEXMAP4, GX_COLOR_NULL);
    GXSetTevKColorSel(tevStage,
        kColorTbl[(int)(7.9f * (1.0f - work->tpLvl))]);
    GXSetTevColorIn(tevStage, GX_CC_CPREV, GX_CC_TEXC, GX_CC_KONST,
        GX_CC_ZERO);
    GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_CA_APREV);
    GXSetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    tevStage++;
    GXSetTevOrder(tevStage, texGen + 1, GX_TEXMAP5, GX_COLOR_NULL);
    color.r = work->shadowColor.r * work->tpLvl;
    color.g = work->shadowColor.g * work->tpLvl;
    color.b = work->shadowColor.b * work->tpLvl;
    GXSetTevKColor(GX_KCOLOR2, color);
    GXSetTevKColorSel(tevStage, GX_TEV_KCSEL_K2);
    GXSetTevColorIn(tevStage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST,
        GX_CC_CPREV);
    GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_CA_TEXA);
    GXSetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVREG0);
    tevStage++;
    GXSetTevOrder(tevStage, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    color.r = work->hiliteColor.r * work->tpLvl;
    color.g = work->hiliteColor.g * work->tpLvl;
    color.b = work->hiliteColor.b * work->tpLvl;
    GXSetTevKColor(GX_KCOLOR3, color);
    GXSetTevKColorSel(tevStage, GX_TEV_KCSEL_K3);
    GXSetTevColorIn(tevStage, GX_CC_ZERO, GX_CC_A0, GX_CC_KONST,
        GX_CC_CPREV);
    GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_CA_APREV);
    GXSetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
}

void mbObjBiriQCreate(MBMODELID modelId)
{
    int hu3DModelId;
    MBOBJBIRIQWORK *work;
    HSF_DATA *hsf;
    HU3D_MODEL *model;
    int i;

    hu3DModelId = mbObjModelIDGet((int)modelId);
    model = &Hu3DData[hu3DModelId];
    if (model->attr & HU3D_ATTR_LINK) {
        work = mbMallocNum(sizeof(*work), model->mallocNoLink);
        model->hookData = work;
    } else {
        work = mbMallocNum(sizeof(*work), model->mallocNo);
        model->hookData = work;
    }
    work->magic = MBOBJ_BIRIQ_WORK_MAGIC;
    work->level = 0.0f;
    work->color.r = work->color.g = work->color.b = work->color.a = 255;
    Hu3DModelMatHookSet(hu3DModelId, BiriQMatHook);
    hsf = model->hsf;
    for (i = 0; i < hsf->materialNum; i++) {
        hsf->material[i].flags |= HSF_MATERIAL_MATHOOK;
    }
}

BOOL mbObjBiriQKill(MBMODELID modelId)
{
    int hu3DModelId;
    MBOBJBIRIQWORK *work;
    HSF_DATA *hsf;
    HU3D_MODEL *model;
    int i;

    hu3DModelId = mbObjModelIDGet((int)modelId);
    model = &Hu3DData[hu3DModelId];
    if (model->hookData == NULL) {
        return FALSE;
    }
    work = model->hookData;
    if (work->magic != MBOBJ_BIRIQ_WORK_MAGIC) {
        return FALSE;
    }
    Hu3DModelMatHookSet(hu3DModelId, NULL);
    hsf = model->hsf;
    for (i = 0; i < hsf->materialNum; i++) {
        hsf->material[i].flags &= ~HSF_MATERIAL_MATHOOK;
    }
    HuMemDirectFree(work);
    model->hookData = NULL;
    return TRUE;
}

void mbObjBiriQColorSet(MBMODELID modelId, BOOL mode, float level,
    GXColor color)
{
    int hu3DModelId;
    MBOBJBIRIQWORK *work;
    HU3D_MODEL *model;

    hu3DModelId = mbObjModelIDGet((int)modelId);
    model = &Hu3DData[hu3DModelId];
    if (model->hookData == NULL) {
        return;
    }
    work = model->hookData;
    if (work->magic != MBOBJ_BIRIQ_WORK_MAGIC) {
        return;
    }
    work->mode = mode;
    work->level = level;
    work->color = color;
}

static void BiriQMatHook(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material)
{
    MBOBJBIRIQWORK *work;
    MBOBJBIRIQTEV *tevConfig;
    int tevStage;
    int texGen;
    GXColor color;
    int i;

    work = drawObj->model->hookData;
    if (material->attrNum == 0) {
        Hu3DTevStageNoTexSet(drawObj, material);
        GetStarNoTexTevStage(drawObj, material, &tevStage, &texGen);
    } else {
        Hu3DTevStageTexSet(drawObj, material);
        GetStarTexTevStage(drawObj, material, &tevStage, &texGen);
    }
    if (work->level <= 0.01f) {
        return;
    }
    GXSetNumTevStages(tevStage + biriQMatNumTbl[work->mode]);
    switch (work->mode) {
        case 0:
        case 3:
            color.r = work->color.r;
            color.g = work->color.g;
            color.b = work->color.b;
            break;
        case 1:
        case 2:
            color.r = work->color.r * work->level;
            color.g = work->color.g * work->level;
            color.b = work->color.b * work->level;
            break;
    }
    color.a = 255.0f * work->level;
    GXSetTevKColor(GX_KCOLOR3, color);
    GXSetTevKColorSel(tevStage, GX_TEV_KCSEL_K3);
    GXSetTevKAlphaSel(tevStage, GX_TEV_KASEL_K3_A);
    tevConfig = &biriQMatTbl[0][0][0] + work->mode * 4;
    for (i = 0; i < biriQMatNumTbl[work->mode]; i++, tevStage++) {
        GXSetTevOrder(tevStage, GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
            GX_COLOR_NULL);
        GXSetTevColorOp(tevStage, tevConfig->op, GX_TB_ZERO,
            GX_CS_SCALE_1, GX_TRUE, tevConfig->outReg);
        GXSetTevColorIn(tevStage, tevConfig->input[0], tevConfig->input[1],
            tevConfig->input[2], tevConfig->input[3]);
        tevConfig++;
        GXSetTevAlphaOp(tevStage, tevConfig->op, GX_TB_ZERO,
            GX_CS_SCALE_1, GX_TRUE, tevConfig->outReg);
        GXSetTevAlphaIn(tevStage, tevConfig->input[0], tevConfig->input[1],
            tevConfig->input[2], tevConfig->input[3]);
        tevConfig++;
    }
}
