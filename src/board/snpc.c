#include <string.h>

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

#define SNPC_MAGIC 'SNPC'
#define MBOBJ_FADE_WORK_MAGIC 'MBTV'
#define MBOBJ_METAL_WORK_MAGIC 'TV01'
#define MBOBJ_BIRIQ_WORK_MAGIC 'TV02'

#define SNPC_DATA_FADE_TEXTURE DATANUM(DATA_board, 103)
#define SNPC_DATA_METAL_TEXMAP4 DATANUM(DATA_board, 105)
#define SNPC_DATA_METAL_TEXMAP5 DATANUM(DATA_board, 104)

typedef struct MBSNPCSAVEWORK {
    u8 flags;
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
    u32 unk0C;
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
} SNPCMOVEWORK;

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

static GXColor texCol[16];

static u32 snpcMagic;
static MBSNPCSAVEWORK *snpcSaveWork;
static MBSNPCWORK *snpcWork;

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

static const SNPCMOTDATA snpcDonkeyMotTbl[SNPC_MOTION_NUM] = {
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

static const SNPCMOTDATA snpcKoopaMotTbl[SNPC_MOTION_NUM] = {
    { DATANUM(DATA_capsulechar1, 1), 1.0f, 1, 0, 0 },
    { DATANUM(DATA_capsulechar1, 6), 1.2f, 1, 0, 0 },
    { DATANUM(DATA_capsulechar1, 6), 1.2f, 1, 0, 0 },
    { DATANUM(DATA_capsulechar1, 5), 1.0f, 0, 0, 20 },
    { DATANUM(DATA_capsulechar1, 5), 1.0f, 0, 21, 0 },
    { DATANUM(DATA_capsulechar1, 5), 1.0f, 0, 50, 0 },
    { DATANUM(DATA_capsulechar1, 7), 1.0f, 0, 0, 0 },
    { DATANUM(DATA_capsulechar1, 4), 1.0f, 0, 0, 0 },
    { DATANUM(DATA_capsulechar1, 3), 1.0f, 0, 0, 0 },
    { DATANUM(DATA_capsulechar1, 10), 1.0f, 0, 0, 0 },
    { DATANUM(DATA_capsulechar1, 11), 1.0f, 0, 0, 0 },
};

static const SNPCMOTDATA *snpcMotTbl[2] = {
    snpcDonkeyMotTbl,
    snpcKoopaMotTbl,
};

static const SNPCMOTNEXTDATA snpcDonkeyNextMotTbl[SNPC_MOTION_NUM] = {
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

static const SNPCMOTNEXTDATA snpcKoopaNextMotTbl[SNPC_MOTION_NUM] = {
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

static const SNPCMOTNEXTDATA *snpcNextMotTbl[2] = {
    snpcDonkeyNextMotTbl,
    snpcKoopaNextMotTbl,
};

static HuVecF masuViewOfs = { 0.0f, 100.0f, 0.0f };

static int playerStarMotNoTbl[2] = { 7, 8 };
static int playerStarSeTbl[2] = {
    MSM_SE_CHARVOICE_MARIO,
    585,
};
static int playerStarChgMotNoTbl[2] = { 7, 8 };
static int playerStarChgSeTbl[2] = {
    MSM_SE_CHARVOICE_MARIO,
    585,
};

static int starMdlTbl[2] = {
    DATANUM(DATA_board, 5),
    DATANUM(DATA_capsulechar1, 32),
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

/* Message, effect, and movement tables used by the turn controller.  The
 * packed message ids are the board message group (0x2D) followed by the
 * message index; keeping them as u32s preserves the retail table shape. */
static const u32 snpcMesTbl[12] = {
    MESSNUM(MESS_BOARD_SNPC, 0), MESSNUM(MESS_BOARD_SNPC, 5),
    MESSNUM(MESS_BOARD_SNPC, 1), MESSNUM(MESS_BOARD_SNPC, 6),
    MESSNUM(MESS_BOARD_SNPC, 4), 0, MESSNUM(MESS_BOARD_SNPC, 9),
    MESSNUM(MESS_BOARD_SNPC, 11), MESSNUM(MESS_BOARD_SNPC, 10),
    MESSNUM(MESS_BOARD_SNPC, 12), MESSNUM(MESS_BOARD_SNPC, 2),
    MESSNUM(MESS_BOARD_SNPC, 7),
};

static const u32 snpcStarMesTbl[10] = {
    MESSNUM(MESS_BOARD_SNPC, 13), MESSNUM(MESS_BOARD_SNPC, 18),
    MESSNUM(MESS_BOARD_SNPC, 14), MESSNUM(MESS_BOARD_SNPC, 19),
    MESSNUM(MESS_BOARD_SNPC, 16), MESSNUM(MESS_BOARD_SNPC, 21),
    MESSNUM(MESS_BOARD_SNPC, 17), MESSNUM(MESS_BOARD_SNPC, 22),
    MESSNUM(MESS_BOARD_SNPC, 15), MESSNUM(MESS_BOARD_SNPC, 20),
};

static const int snpcEffectChanceTbl[4] = { 30, 55, 80, 95 };
static const int snpcDiceTypeTbl[2][2] = {
    { 16, 17 },
    { 18, 18 },
};
static const HuVecF snpcDiceOfsTbl[2] = {
    { 0.0f, 90.0f, 0.0f },
    { 0.0f, 90.0f, 0.0f },
};

static const int snpcMesSpeakerTbl[2] = { 6, 13 };
static const int snpcStarStrmTbl[2] = { 30, 28 };
static const int snpcStarMesSpeakerTbl[2] = { 30, 28 };
static const int snpcMoveStrmTbl[2] = { 573, 585 };
static const int snpcMoveNumColor[2] = { 1, 2 };

static int snpcDiceMotTimeTbl[2] = { 25, 25 };
static const float snpcRotSpeedTbl[2] = { 1.3f, 1.8f };
static const float snpcPosFixSpeedTbl[2] = { 1.3f, 1.8f };

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

enum {
    SNPC_MASU_TYPE_DONKEY = 7,
    SNPC_MASU_TYPE_KOOPA = 10,
    SNPC_MASU_SE_DONKEY = 1527,
    SNPC_MASU_SE_KOOPA = 1528,
    SNPC_MASU_RESET_SE = 1529,
};

static const int snpcMasuTypeTbl[2] = {
    SNPC_MASU_TYPE_DONKEY,
    SNPC_MASU_TYPE_KOOPA,
};

static const int snpcMasuSeTbl[2] = {
    SNPC_MASU_SE_DONKEY,
    SNPC_MASU_SE_KOOPA,
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
void mbSNpcStarExec(int playerNo, int masuId);
static BOOL SNpcMoveExec(void);
static void SNpcMoveOMExec(OMOBJ *obj);
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
static OMOBJ *SNpcDiceExec(int diceValue, int diceNum);
static int SNpcDiceValueGet(int masuId, int diceNum, const int *typeTbl,
    u32 mAttr, float chance, s16 *pathTbl);

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
extern int mbSNpcMasuStarNextGet(s16 masuId, int type, int *linkNoTbl,
    u32 attr);
extern void mbDiceObjHit(int playerNo);
extern void mbPlayerMoveHookSet(int playerNo, void (*hook)(int playerNo));
extern void mbWipeSpecialFadeInCreate(int type, int time);
extern void mbWipeSpecialFadeOutCreate(int type, int time);
extern void mbDiceSNpcNumKill(OMOBJ *obj);
extern void mbDiceSNpcNumDispSet(OMOBJ *obj, BOOL dispF);
extern void mbDiceSNpcNumSet(OMOBJ *obj, u8 value);
extern void mbDiceSNpcNumPosSet(OMOBJ *obj, HuVecF *pos);
extern OMOBJ *mbDiceSNpcNumCreate(int playerNo, HuVecF *pos);
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
extern void mbStarAddAllProcExecV(int *addNum, BOOL *dispF, BOOL fastF);
extern void mbCoinAddAllProcExecV(int *addNum, BOOL *dispF, BOOL fastF);
extern int mbStarAddProcExec(int playerNo, int starNum, BOOL dispF, BOOL fastF);
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
    float zoom, float fov, s16 maxTime);
extern void mbCameraMovePlayer(s16 playerNo, HuVecF *rot, HuVecF *offset,
    float zoom, float fov, s16 maxTime);
extern void mbCameraShakeSet(int maxTime, float power);
extern void mbCameraMoveWait(void);
extern float mbCameraPlayerViewZoomGet(int viewNo);
extern float mbSinDeg(float deg);
extern void mbWipeDissolveFadeIn(void);
extern void mbWipeDissolveFadeOut(void);
extern const float lbl_802C3270;
extern const float lbl_802C3288;
extern const float lbl_802C3298;
extern const float lbl_802C329C;
extern const float lbl_802C32A4;
extern const float lbl_802C32A8;
extern const float lbl_802C32AC;
extern const float lbl_802C3354;
extern const float lbl_802C335C;
extern const float lbl_802C3368;
extern const float lbl_802C3370;
extern const float lbl_802C3374;
extern const float lbl_802C3378;
extern const float lbl_802C337C;
extern const float lbl_802C3380;
extern const float lbl_802C3384;
extern const float lbl_802C3388;
extern const float lbl_802C338C;
extern const float lbl_802C3390;
extern const float lbl_802C3394;
extern const float lbl_802C3398;
extern const float lbl_802C339C;
extern const float lbl_802C33A0;
extern const float lbl_802C33B0;
extern const float lbl_802C33B4;
extern const float lbl_802C33B8;
extern const float lbl_802C33BC;
extern const float lbl_802C328C;
extern const float lbl_802C3290;
extern const float lbl_802C3294;
extern const float lbl_802C32B8;
extern const float lbl_802C32BC;
extern const float lbl_802C32C0;
extern const double lbl_802C32C8;
extern const double lbl_802C32D0;
extern const float lbl_802C32D8;
extern const float lbl_802C32DC;
extern const float lbl_802C32E0;
extern const float lbl_802C32E4;
extern const float lbl_802C32E8;
extern const float lbl_802C3308;
extern const float lbl_802C330C;
extern const float lbl_802C3310;
extern const float lbl_802C3314;
extern const float lbl_802C3318;
extern const float lbl_802C332C;
extern const float lbl_802C3328;
extern const float lbl_802C3344;
extern const float lbl_802C3348;
extern const float lbl_802C334C;
extern const float lbl_802C3350;
extern const float lbl_802C3358;
extern const float lbl_802C3360;
extern const float lbl_802C3378;
extern const float lbl_802C336C;
extern const float lbl_802C33E0;
extern const float lbl_802C33E4;
extern const float lbl_802C33E8;
extern const float lbl_802C32EC;
extern const float lbl_802C32F0;
extern const float lbl_802C33EC;
extern const float lbl_802C33F0;
extern const float lbl_802C33F4;
extern const float lbl_802C33F8;
extern const float lbl_802C33FC;
extern const float lbl_802C3400;
extern const float lbl_802C3308;
extern const float lbl_802C332C;
extern const float lbl_802C3330;
extern const float lbl_802C3364;
extern const float lbl_802C32A0;
extern const float lbl_802C33C0;
extern const double lbl_802C33C8;
extern const double lbl_802C3278;
extern const double lbl_802C3280;
extern const double lbl_802C3320;
extern const double lbl_802C32F8;
extern const double lbl_802C3300;
extern const double lbl_802C33D0;
extern BOOL mbSaveNewF;
extern void mbStarMasuFuncSet(void (*func)(void));

void mbSNpcInit(void)
{
    snpcMagic = 0;
    snpcSaveWork = NULL;
    snpcWork = NULL;
}

void mbSNpcCreate(MBSNPCSAVEWORK *saveWork, u32 dataNum)
{
    int isKoopa;
    int i;

    snpcSaveWork = saveWork;
    snpcWork = mbMalloc(sizeof(MBSNPCWORK));
    snpcWork->dataNum = dataNum;
    if (mbSaveNewF) {
        snpcSaveWork->masuId = (u8)mbMasuFind_AttrIdGet(MASU_NULL,
            MASU_FLAG_START);
        snpcSaveWork->masuId = (u8)SNpcMasuStarNextGet(TRUE);
        snpcSaveWork->effectMissCount = 0;
    }
    isKoopa = GwSystem.partyF ? 0 : 1;
    snpcSaveWork->flags = (u8)(isKoopa * SNPC_FLAG_KOOPA);
    SNpcMasuSet(snpcSaveWork->masuId, FALSE);
    SNpcObjCreate();
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerEndTurnHookSet(i, SNpcMoveExec);
    }
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

void mbSNpcMotWinSet(void)
{
    if (snpcMagic == SNPC_MAGIC) {
        SNpcSePlay(1, 0);
        SNpcObjMotShiftSet(8);
    }
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

static BOOL SNpcMoveExec(void)
{
    static const int typeTbl[3] = { 1, 2, -1 };
    s16 coinTbl[GW_PLAYER_MAX];
    s16 starTbl[GW_PLAYER_MAX];
    u8 hitTbl[GW_PLAYER_MAX];
    s8 playerTbl[2][GW_PLAYER_MAX + 1];
    HuVecF pos;
    OMOBJ *obj;
    SNPCMOVEWORK *moveWork;
    int isKoopa;
    int diceNum;
    int diceValue;
    int linkNo;
    int masuId;
    int currentMasu;
    int playerCount;
    int prizeCount;
    int otherCount;
    int disablePlayer;
    int i;

    if (GwSystem.turnPlayerNo < 3) {
        return FALSE;
    }
    isKoopa = (snpcSaveWork->flags >> 7) & 1;
    mbWipeSpecialFadeInCreate(5, 1);
    masuId = snpcSaveWork->masuId;
    mbMasuPosGet(masuId, &pos);
    SNpcObjPosSetV(&pos);
    SNpcObjRotSet(lbl_802C3290, lbl_802C3290, lbl_802C3290);
    mbCapMasuDispSet(FALSE);
    mbMasuPlayerDispSet(FALSE);
    mbCameraFocusObjSet(snpcWork->objId[0]);
    mbCameraOffsetSet(lbl_802C3290, lbl_802C32B8, lbl_802C3290);
    mbCameraZoomSet(mbCameraPlayerViewZoomGet(0));
    mbCameraMoveOnSet(FALSE);
    mbWipeSpecialFadeOutCreate(5, 42);
    mbCameraMoveOnSet(TRUE);
    mbMusPlay(0, snpcMoveStrmTbl[isKoopa], 127, 0);
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
        float roll = frandf();
        roll = lbl_802C32BC + lbl_802C32C0 * roll;
        if (mbRandMod(100) < 10) {
            roll = lbl_802C32BC + lbl_802C32C0 * frandf();
        }
        diceValue = SNpcDiceValueGet(masuId, diceNum, typeTbl,
            snpcWork->unk04, roll, snpcWork->pathTbl);
    } else {
        double roll = lbl_802C32C8 + lbl_802C32D0 * (double)frandf();
        if (diceNum > 1) {
            roll = lbl_802C32BC + lbl_802C32D8 * (double)frandf();
        }
        if (mbRandMod(100) < 10) {
            roll = lbl_802C32BC + lbl_802C32C0 * (double)frandf();
        }
        diceValue = SNpcDiceValueGet(masuId, diceNum, typeTbl,
            snpcWork->unk04, (float)roll, snpcWork->pathTbl);
    }
    snpcWork->diceNumObj = SNpcDiceExec(diceValue, diceNum);
    SNpcZoomSet(mbCameraPlayerViewZoomGet(2));
    SNpcMasuReset(TRUE);
    linkNo = -1;
    currentMasu = masuId;

    for (;;) {
        int nextMasu;
        nextMasu = MasuNextGet(currentMasu, linkNo);
        linkNo = nextMasu;
        nextMasu = snpcWork->pathTbl[linkNo];
        if (nextMasu <= 0) {
            break;
        }
        mbev_PlayerColCircleAdd(-1, (s16)nextMasu, FALSE,
            lbl_802C32DC);
        prizeCount = 0;
        otherCount = 0;
        playerCount = 0;
        disablePlayer = -1;
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            coinTbl[i] = mbPlayerCoinGet(i);
            starTbl[i] = mbPlayerStarGet(i);
            hitTbl[i] = (GwPlayer[i].masuId == nextMasu);
            if (hitTbl[i]) {
                playerCount++;
                if (GwPlayer[i].comF) {
                    disablePlayer = i;
                }
            }
        }
        if (GwSystem.partyF) {
            for (i = 0; i < 2; i++) {
                int first = mbPlayerTeamFindPlayer(i, 0);
                int second = mbPlayerTeamFindPlayer(i, 1);
                if (hitTbl[first] && hitTbl[second]) {
                    starTbl[first] = 0;
                    coinTbl[second] = 0;
                }
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
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            playerTbl[0][i] = -1;
            playerTbl[1][i] = -1;
        }
        prizeCount = 0;
        otherCount = 0;
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (!hitTbl[i]) {
                continue;
            }
            if (!isKoopa) {
                if (coinTbl[i] >= 20) {
                    playerTbl[0][prizeCount++] = (s8)i;
                } else {
                    playerTbl[1][otherCount++] = (s8)i;
                }
            } else if (starTbl[i] > 0) {
                playerTbl[0][prizeCount++] = (s8)i;
            } else {
                playerTbl[1][otherCount++] = (s8)i;
            }
        }
        playerTbl[0][prizeCount] = -1;
        playerTbl[1][otherCount] = -1;
        if (!isKoopa) {
            playerCount = prizeCount;
        }
        obj = omAddObjEx(mbObjMan, SNPC_MOVE_OBJ_PRIORITY, 0, 0,
            OM_GRP_NONE, SNpcMoveOMExec);
        snpcWork->moveObj = obj;
        moveWork = omObjGetWork(obj, SNPCMOVEWORK);
        moveWork->masuId = (s16)nextMasu;
        while (snpcWork->moveObj != NULL) {
            HuPrcVSleep();
        }
        mbev_PlayerColMasuAdd(-1, (s16)currentMasu, FALSE);
        if (mbMasuDispCheck((s16)nextMasu)) {
            diceValue--;
        }
        mbDiceSNpcNumSet(snpcWork->diceNumObj, diceValue);
        if (playerCount > 0) {
            mbDiceSNpcNumDispSet(snpcWork->diceNumObj, FALSE);
            SNpcZoomSet(mbCameraPlayerViewZoomGet(0));
            SNpcTargetAngleSet(lbl_802C3290);
            SNpcRotateWait();
            while (!mbPlayerColCheck()) {
                HuPrcVSleep();
            }
            {
                int winNo = mbWinCreate(2,
                    snpcStarMesTbl[2 + isKoopa],
                    snpcStarMesSpeakerTbl[isKoopa]);
                mbWinPlayerDisable((s16)winNo, disablePlayer);
                for (i = 0; i < otherCount; i++) {
                    int player = playerTbl[1][i];
                    mbPlayerWinLoseVoicePlay(player, isKoopa ? 13 : 12,
                        isKoopa ? 585 : 579);
                    mbPlayerMotionShiftSet(player, isKoopa ? 13 : 12,
                        lbl_802C3290, lbl_802C3294, 0);
                }
                for (i = 0; i < prizeCount; i++) {
                    int player = playerTbl[0][i];
                    mbPlayerWinLoseVoicePlay(player, isKoopa ? 13 : 12,
                        isKoopa ? 585 : 579);
                    mbPlayerMotionShiftSet(player, isKoopa ? 13 : 12,
                        lbl_802C3290, lbl_802C3294, 0);
                }
                if (isKoopa) {
                    SNpcObjMotEndWait();
                    SNpcObjMotShiftSet(0);
                }
                mbWinWait((s16)winNo);
                for (i = 0; i < otherCount; i++) {
                    mbPlayerMotionEndWait(playerTbl[1][i]);
                }
                for (i = 0; i < prizeCount; i++) {
                    mbPlayerMotionEndWait(playerTbl[0][i]);
                }
            }
            SNpcSePlay(1, 0);
            SNpcObjMotShiftSet(8);
            SNpcObjMotEndWait();
            SNpcObjMotShiftSet(0);
            if (!isKoopa) {
                for (i = 0; i < otherCount; i++) {
                    mbCoinAddDispExec(playerTbl[1][i], -20, FALSE, TRUE);
                }
                mbMusJingleWait(mbMusJinglePlay(39));
                StarChangeExec(playerTbl[0], 0, 1);
            } else if (prizeCount > 0) {
                StarChangeExec(playerTbl[0], 0, -1);
            } else {
                StarChangeExec(playerTbl[1], 1, -20);
            }
            if (!isKoopa || prizeCount > 0) {
                HuVecF starPos;
                mbMasuPosGet((s16)nextMasu, &starPos);
                SNpcStarCreate(isKoopa, FALSE, &starPos);
                mbMusPauseFadeOut(0, TRUE, 1000);
                SNpcStarWait();
                SNpcObjMotEndWait();
                SNpcObjMotShiftSet(0);
            }
            mbMusPauseFadeOut(0, FALSE, 1000);
            HuPrcSleep(12);
            if (isKoopa && prizeCount > 0 && otherCount > 0) {
                int winNo = mbWinCreate(2, snpcStarMesTbl[8 + isKoopa],
                    snpcStarMesSpeakerTbl[isKoopa]);
                mbWinPlayerDisable((s16)winNo, disablePlayer);
                mbWinWait((s16)winNo);
            }
        }
        mbDiceSNpcNumDispSet(snpcWork->diceNumObj, TRUE);
        SNpcZoomSet(mbCameraPlayerViewZoomGet(2));
        currentMasu = nextMasu;
    }
    mbDiceSNpcNumKill(snpcWork->diceNumObj);
    snpcWork->diceNumObj = NULL;
    SNpcMasuSet(snpcWork->pathTbl[linkNo], TRUE);
    SNpcZoomSet(mbCameraPlayerViewZoomGet(0));
    SNpcTargetAngleSet(lbl_802C3290);
    SNpcRotateWait();
    {
        int winNo = mbWinCreate(2, snpcStarMesTbl[6 + isKoopa],
            snpcStarMesSpeakerTbl[isKoopa]);
        mbWinPlayerDisable((s16)winNo, -1);
        SNpcSePlay(1, 0);
        SNpcObjMotShiftSet(8);
        mbWinWait((s16)winNo);
        SNpcObjMotEndWait();
        SNpcZoomWait();
    }
    if (GwSystem.turnPlayerNo >= 3 || _CheckFlag(FLAG_BOARD_NOMG)) {
        mbCapMasuDispSet(TRUE);
        mbMasuPlayerDispSet(TRUE);
    }
    SNpcMasuEffDispSet();
    mbMusBoardFadeOut(0, 0, 1000, 1000, -1, FALSE);
    return FALSE;
}

static void StarChangeExec(s8 *playerNoTbl, int starF, int amount)
{
    int addNum[GW_PLAYER_MAX];
    BOOL dispF[GW_PLAYER_MAX];
    int voiceNo;
    int playerNo;
    int i;

    voiceNo = amount < 0;
    for (i = 0; playerNoTbl[i] >= 0; i++) {
        playerNo = playerNoTbl[i];
        mbPlayerWinLoseVoicePlay(playerNo, playerStarChgMotNoTbl[voiceNo],
            playerStarChgSeTbl[voiceNo]);
        mbPlayerMotionShiftSet(playerNo, playerStarChgMotNoTbl[voiceNo],
            lbl_802C3290, lbl_802C3294, 0);
        omVibrate((s16)playerNo, 20, 7, 3);
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
    int nextLinkNo;
    BOOL jumpF;

    nextLinkNo = linkNo + 1;
    jumpF = FALSE;
    if ((mbMasuAttrGet((s16)masuId) & MASU_FLAG_JUMPFROM)
        && (mbMasuAttrGet(snpcWork->pathTbl[nextLinkNo])
            & MASU_FLAG_JUMPTO)) {
        jumpF = TRUE;
    } else if ((mbMasuAttrGet((s16)masuId) & MASU_FLAG_CLIMBFROM)
        && (mbMasuAttrGet(snpcWork->pathTbl[nextLinkNo])
            & MASU_FLAG_CLIMBTO)) {
        jumpF = TRUE;
    } else if ((mbMasuAttrGet(snpcWork->pathTbl[nextLinkNo])
        & MASU_FLAG_CLIMBFROM)
        && (mbMasuAttrGet(snpcWork->pathTbl[nextLinkNo + 1])
            & MASU_FLAG_CLIMBTO)) {
        jumpF = TRUE;
        nextLinkNo += 2;
    }
    snpcWork->unk0C = jumpF;
    return nextLinkNo;
}

static void SNpcStarCreate(int type, BOOL loopF, HuVecF *pos)
{
    OMOBJ *obj;
    SNPCSTAREFFWORK *work;
    int modelData;

    obj = omAddObjEx(mbObjMan, SNPC_MOVE_OBJ_PRIORITY,
        SNPC_STAR_OBJ_GROUP, 0, OM_GRP_NONE, SNpcStarObjExec);
    snpcWork->starObj = obj;
    work = omObjGetWork(obj, SNPCSTAREFFWORK);
    work->unk00 = (type != 0);
    omSetStatBit(obj, SNPC_MOVE_OBJ_PRIORITY);
    work->loopF = loopF;
    obj->trans = *pos;

    modelData = mbBoardDataNumGet(starMdlTbl[type]);
    obj->mdlId[0] = mbObjCreate(modelData, NULL, TRUE);
    mbObjDispSet(obj->mdlId[0], FALSE);
    obj->mdlId[1] = SNpcStarEffCreate(
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_effect, 1),
            HU_MEMNUM_OVL, HEAP_MODEL)), type);
}

static HU3D_MODELID SNpcStarEffCreate(ANIMDATA *anim, int type)
{
    HU3D_MODELID modelId;

    modelId = mbParManCreate(anim, SNPC_STAR_PARTICLE_MAX,
        &snpcStarEffParam[type]);
    mbParManAttrSet((int)modelId, SNPC_STAR_PARTICLE_ATTR);
    mbParManRotSet((int)modelId, lbl_802C328C, lbl_802C3290, lbl_802C3290);
    mbParticleBlendModeSet((int)modelId, 1);
    Hu3DModelLayerSet(modelId, 5);
    Hu3DModelCameraSet(modelId, 1);
    return modelId;
}

static void SNpcKoopaFireExec(void)
{
    HU3D_MODELID fire3Id;
    HU3D_MODELID fire3CopyId;
    HU3D_MODELID fireId;
    HU3D_MODELID fireCopyId;
    MBPARTICLE *fire3P;
    MBPARTICLE *fire3CopyP;
    MBPARTICLE *fireP;
    MBPARTICLE *fireCopyP;
    HuVecF *fireVec;
    HuVecF *fire3Vec;
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
    fire3P = Hu3DData[fire3Id].hookData;
    fire3CopyP = Hu3DData[fire3CopyId].hookData;
    fire3CopyP->hookData = fire3P;
    fire3CopyP->mode = 0;

    fireId = mbParticleCreate(HuSprAnimRead(HuDataReadNum(
        mbBoardDataNumGet(SNPC_KOOPA_FIRE_DATA_ANIM), HU_MEMNUM_OVL)), 500);
    fireCopyId = mbParticleCreate(HuSprAnimRead(HuDataReadNum(
        mbBoardDataNumGet(SNPC_KOOPA_FIRE_DATA_ANIM), HU_MEMNUM_OVL)), 500);
    mbParticleHookSet(fireId, SNpcKoopaFireHook);
    mbParticleHookSet(fireCopyId, SNpcKoopaFire2Hook);
    Hu3DModelLayerSet(fireId, 5);
    Hu3DModelLayerSet(fireCopyId, 5);
    fireP = Hu3DData[fireId].hookData;
    fireCopyP = Hu3DData[fireCopyId].hookData;
    fireCopyP->hookData = fireP;
    fireCopyP->mode = 0;

    mode = 0;
    fire3P->mode = (s16)mode;
    fireP->mode = (s16)mode;
    fireVec = (HuVecF *)fireP->work;
    fire3Vec = (HuVecF *)fire3P->work;
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
            "itemhook_M", mtx);
        fireP->vel.x = mtx[0][3];
        fireP->vel.y = mtx[1][3];
        fireP->vel.z = mtx[2][3];
        mtx[0][3] = lbl_802C3290;
        mtx[1][3] = lbl_802C3290;
        mtx[2][3] = lbl_802C3290;

        offset.x = lbl_802C3358;
        offset.y = lbl_802C335C;
        offset.z = lbl_802C3360;
        PSMTXMultVec(mtx, &offset, &offset);
        PSVECAdd(&fireP->vel, &offset, &fireP->vel);
        fire3P->vel = fireP->vel;

        offset.x = lbl_802C3364;
        offset.y = lbl_802C32EC;
        offset.z = lbl_802C3290;
        PSMTXMultVec(mtx, &offset, fireVec);
        *fire3Vec = *fireVec;
        value = (float)mode * lbl_802C3368;
        PSVECScale(fireVec, &offset,
            lbl_802C32A0 * (lbl_802C336C * value));
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

static void SNpcKoopaFireHook(HU3D_MODEL *modelP, MBPARTICLE *particleP,
    Mtx mtx)
{
    HuVecF axisZ = { 0.0f, 0.0f, 1.0f };
    HuVecF axisX = { 1.0f, 0.0f, 0.0f };
    MBPARTICLEDATA *data;
    HuVecF axisCross;
    HuVecF offset;
    const HuVecF *axis;
    Mtx rotMtx;
    float ratio;
    float blend;
    float speed;
    float value;
    float absY;
    float absZ;
    int gray;
    int i;

    (void)modelP;
    (void)mtx;
    data = particleP->data;
    if (particleP->count == 0) {
        for (i = 0; i < particleP->num; i++) {
            data[i].scale = lbl_802C3290;
            data[i].color.a = 0;
            data[i].time = (s16)-(i >> 3);
        }
        particleP->count = 1;
    }

    for (i = 0; i < particleP->num; i++, data++) {
        if (particleP->mode != 0) {
            if (data->time < 0) {
                data->time++;
                continue;
            }
            if (data->time == 0) {
                data->vel = *(HuVecF *)particleP->work;
                axis = &axisZ;
                absZ = fabsf(data->vel.z);
                absY = fabsf(data->vel.y);
                if (absZ >= absY && absZ >= fabsf(data->vel.x)) {
                    axis = &axisX;
                }
                PSVECCrossProduct(axis, &data->vel, &axisCross);
                value = frandf() * lbl_802C3330 * lbl_802C3370;
                PSMTXRotAxisRad(rotMtx, &data->vel, value);
                PSMTXMultVec(rotMtx, &axisCross, &data->accel);
                data->activeF = 30;
                data->time = 30;
                data->pos = particleP->vel;

                speed = frandf() * lbl_802C3374 * lbl_802C32A0;
                PSVECScale(&data->vel, &offset, speed);
                PSVECAdd(&data->pos, &offset, &data->pos);
                speed = frandf() * lbl_802C3374 * lbl_802C32A0;
                PSVECScale(&data->accel, &offset, speed);
                PSVECAdd(&data->pos, &offset, &data->pos);

                value = (float)particleP->mode * lbl_802C3368;
                speed = (lbl_802C3364 + lbl_802C3378 * value)
                    * (lbl_802C3374 + frandf());
                PSVECScale(&data->vel, &data->vel,
                    lbl_802C337C * speed);
                PSVECScale(&data->accel, &data->accel,
                    lbl_802C3380 * speed);
                data->colorIdx = lbl_802C3290;
                data->scaleBase = lbl_802C3384
                    * (lbl_802C32D8 + lbl_802C32D8 * frandf());
                data->rot.z = (float)mbRandMod(360);
                data->animBank = (s16)mbRandMod(8);
                gray = (int)(lbl_802C32A0 + lbl_802C3388 * frandf());
                data->color.r = (u8)gray;
                data->color.g = (u8)gray;
                data->color.b = (u8)gray;
                data->scale = (lbl_802C3354 + lbl_802C3298 * frandf())
                    * (lbl_802C32D8 + lbl_802C32D8 * value);
                data->color.a = (u8)(mbRandMod(32) + 48);
            }
        }
        if (data->time <= 0) {
            continue;
        }

        ratio = lbl_802C32EC
            - ((float)data->time / (float)data->activeF);
        data->time--;
        speed = mbSinDeg(lbl_802C32A0 + (lbl_802C338C * ratio));
        PSVECScale(&data->vel, &offset, speed);
        PSVECAdd(&data->pos, &offset, &data->pos);
        speed = mbSinDeg(lbl_802C335C + (lbl_802C338C * ratio));
        PSVECScale(&data->accel, &offset, speed);
        PSVECAdd(&data->pos, &offset, &data->pos);

        if (particleP->mode == 0) {
            data->colorIdx += data->scaleBase;
            data->pos.y += data->colorIdx;
            if (ratio > lbl_802C32D8) {
                data->scale *= lbl_802C3390;
            }
        }
        if (ratio > lbl_802C3314) {
            blend = lbl_802C3394 * (ratio - lbl_802C3314);
            value = (float)data->color.r
                + ((lbl_802C3298 - (float)data->color.r) * blend);
            data->color.r = (u8)value;
            value = (float)data->color.g
                + ((lbl_802C3298 - (float)data->color.g) * blend);
            data->color.g = (u8)value;
            value = (float)data->color.b
                + ((lbl_802C3298 - (float)data->color.b) * blend);
            data->color.b = (u8)value;
        }
        if (ratio > lbl_802C3398) {
            blend = lbl_802C339C * (ratio - lbl_802C3398);
            value = (float)data->color.a
                + ((lbl_802C33A0 - (float)data->color.a) * blend);
            data->color.a = (u8)value;
        }
        if (data->time == 0) {
            data->scale = lbl_802C3290;
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

    (void)modelP;
    (void)mtx;
    if (particleP->mode == 0) {
        data = particleP->data;
        for (i = 0; i < particleP->num; i++, data++) {
            data->scale = lbl_802C3290;
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
        alpha = (int)(lbl_802C3378 * (float)data->color.a);
        if (alpha > 255) {
            alpha = 255;
        }
        data->color.a = (u8)alpha;
    }
}

static void SNpcKoopaFire3Hook(HU3D_MODEL *modelP, MBPARTICLE *particleP,
    Mtx mtx)
{
    HuVecF axisZ = { 0.0f, 0.0f, 1.0f };
    HuVecF axisX = { 1.0f, 0.0f, 0.0f };
    MBPARTICLEDATA *data;
    const HuVecF *axis;
    HuVecF axisCross;
    HuVecF offset;
    Mtx rotMtx;
    float modeScale;
    float ratio;
    float blend;
    float speed;
    float value;
    float absY;
    float absZ;
    int gray;
    int i;

    (void)modelP;
    (void)mtx;
    data = particleP->data;
    if (particleP->count == 0) {
        for (i = 0; i < particleP->num; i++) {
            data[i].scale = lbl_802C3290;
            data[i].color.a = 0;
            data[i].time = (s16)-(i >> 3);
        }
        particleP->count = 1;
    }

    for (i = 0; i < particleP->num; i++, data++) {
        if (particleP->mode != 0) {
            if (data->time < 0) {
                data->time++;
                continue;
            }
            if (data->time == 0) {
                axis = &axisZ;
                absZ = fabsf(data->vel.z);
                absY = fabsf(data->vel.y);
                if (absZ >= absY && absZ >= fabsf(data->vel.x)) {
                    axis = &axisX;
                }
                PSVECCrossProduct(axis, &data->vel, &axisCross);
                value = frandf() * lbl_802C3330 * lbl_802C3370;
                PSMTXRotAxisRad(rotMtx, &data->vel, value);
                PSMTXMultVec(rotMtx, &axisCross, &data->accel);
                modeScale = (float)particleP->mode * lbl_802C3368;
                data->activeF = 10;
                data->time = 10;
                data->pos = particleP->vel;

                speed = frandf() * lbl_802C3374 * lbl_802C32A0;
                PSVECScale(&data->vel, &offset, speed * modeScale);
                PSVECAdd(&data->pos, &offset, &data->pos);
                speed = frandf() * lbl_802C33B0 * lbl_802C32A0;
                PSVECScale(&data->accel, &offset, speed * modeScale);
                PSVECAdd(&data->pos, &offset, &data->pos);

                PSVECScale(&data->vel, &data->vel,
                    lbl_802C33B4 * modeScale);
                PSVECScale(&data->accel, &data->accel,
                    lbl_802C33B8 * modeScale);
                data->rot.z = (float)mbRandMod(360);
                data->animBank = (s16)mbRandMod(8);
                gray = (int)(lbl_802C32A0 + lbl_802C3388 * frandf());
                data->color.r = (u8)gray;
                data->color.g = (u8)gray;
                data->color.b = (u8)gray;
                data->color.a = 0;
                data->speedDecay = (float)(mbRandMod(62) + 58);
                data->scale = lbl_802C33BC;
                data->colorIdx = lbl_802C33BC
                    + (lbl_802C3354 * frandf());
            }
        }
        if (data->time <= 0) {
            continue;
        }

        ratio = lbl_802C32EC
            - ((float)data->time / (float)data->activeF);
        data->time--;
        PSVECAdd(&data->pos, &data->vel, &data->pos);
        PSVECAdd(&data->pos, &data->accel, &data->pos);
        blend = lbl_802C332C * ratio;
        if (blend > lbl_802C32EC) {
            blend = lbl_802C32EC;
        }
        value = (float)data->color.a
            + ((data->speedDecay - (float)data->color.a) * blend);
        data->color.a = (u8)value;
        data->scale += (data->colorIdx - data->scale) * blend;
        if (data->time == 0) {
            data->scale = lbl_802C3290;
            data->color.a = 0;
        }
    }
}

static void SNpcEffectExec(void)
{
    HuVecF offset = { -70.0f, 320.0f, 20.0f };
    HuVecF pos;
    MBMODELID modelId;
    float height;
    int i;

    if ((snpcSaveWork->flags >> 7) & 1) {
        SNpcKoopaFireExec();
        return;
    }

    modelId = mbObjCreate(SNPC_DONKEY_EFFECT_DATA_MODEL, NULL, FALSE);
    mbObjPosGet(snpcWork->objId[0], &pos);
    PSVECAdd(&pos, &offset, &pos);
    SNpcObjMotShiftSet(10);
    for (i = 14; i >= 0; i--) {
        height = lbl_802C32A0 * (lbl_802C3350
            * mbSinDeg(lbl_802C3354
                * ((float)i / lbl_802C334C)));
        mbObjPosSet(modelId, pos.x, pos.y + height, pos.z);
        HuPrcVSleep();
    }
    mbObjDispSet(modelId, FALSE);
    SNpcObjMotEndWait();
    mbObjKill(modelId);
}

void mbSNpcStarExec(int playerNo, int masuId)
{
    HuVecF playerPos;
    HuVecF playerRot;
    HuVecF npcPos;
    HuVecF masuPos;
    HuVecF starPos;
    float angle;
    float angleDiff;
    float ratio;
    float zoom;
    int isKoopa;
    int state;
    int winNo;
    int nextMasu;
    int jingleNo;
    BOOL moveStarF;
    int i;

    if (snpcMagic != SNPC_MAGIC || masuId != snpcSaveWork->masuId) {
        return;
    }

    mbCameraStackPush();
    mbMoveNumDispSet(playerNo, FALSE);
    mbCameraMoveMasu((s16)masuId, NULL, &masuViewOfs,
        mbCameraPlayerViewZoomGet(0), lbl_802C3270, 24);

    isKoopa = (snpcSaveWork->flags >> 7) & 1;
    state = 0;
    if (!isKoopa) {
        if (mbPlayerCoinGet(playerNo) >= 20) {
            winNo = mbWinCreateChoice(2, snpcMesTbl[isKoopa],
                snpcMesSpeakerTbl[isKoopa], 0);
            if (GwPlayer[playerNo].comF) {
                mbComChoiceUpSet();
            }
            state = 1;
        } else {
            winNo = mbWinCreate(2, snpcMesTbl[2 + isKoopa],
                snpcMesSpeakerTbl[isKoopa]);
        }
    } else if (mbPlayerStarGet(playerNo) > 0) {
        winNo = mbWinCreate(2, snpcMesTbl[isKoopa],
            snpcMesSpeakerTbl[isKoopa]);
        state = 2;
    } else {
        winNo = mbWinCreate(2, snpcMesTbl[2 + isKoopa],
            snpcMesSpeakerTbl[isKoopa]);
        state = 3;
    }
    SNpcSePlay(1, 0);

    mbPlayerPosGet(playerNo, &playerPos);
    SNpcObjPosGet(&npcPos);
    PSVECSubtract(&npcPos, &playerPos, &npcPos);
    PSVECNormalize(&npcPos, &npcPos);
    angle = (float)(lbl_802C3278
        * (atan2(-npcPos.x, -npcPos.z) / lbl_802C3280));
    SNpcTargetAngleSet(angle);
    PSVECScale(&npcPos, &npcPos, lbl_802C3288 / 7.0f);

    mbPlayerPosGet(playerNo, &playerPos);
    mbPlayerRotGet(playerNo, &playerRot);
    angle = (float)(lbl_802C3278
        * (atan2(-npcPos.x, -npcPos.z) / lbl_802C3280));
    angleDiff = mbAngleWrap2(angle, playerRot.y);
    mbPlayerMotionSet(playerNo, 9, HU3D_MOTATTR_NONE);
    for (i = 0; i < 7; i++) {
        ratio = (float)(i + 1) / 7.0f;
        angle = (float)sin((lbl_802C3280 * (lbl_802C328C * ratio))
            / lbl_802C3278);
        mbPlayerRotSet(playerNo, lbl_802C3290,
            playerRot.y + (angle * angleDiff), lbl_802C3290);
        HuPrcVSleep();
    }
    mbMusPlay(0, snpcStarStrmTbl[isKoopa], 127, 0);
    while (!mbPlayerMotionEndCheck(playerNo)) {
        HuPrcVSleep();
    }
    mbPlayerMotIdleSet(playerNo);
    SNpcRotateWait();

    moveStarF = TRUE;
    if (state == 0) {
        mbWinWait((s16)winNo);
        winNo = mbWinCreate(2, snpcMesTbl[10 + isKoopa],
            snpcMesSpeakerTbl[isKoopa]);
        SNpcSePlay(2, 0);
        SNpcObjMotShiftSet(9);
        mbWinWait((s16)winNo);
        mbPlayerWinLoseVoicePlay(playerNo, 13, 585);
        mbPlayerMotionShiftSet(playerNo, 13, lbl_802C3290,
            lbl_802C3294, HU3D_MOTATTR_NONE);
        mbPlayerMotionEndWait(playerNo);
        moveStarF = FALSE;
    } else if (state == 1) {
        mbWinWait((s16)winNo);
        if (mbWinChoiceGet((s16)winNo) == 0) {
            jingleNo = -1;
            mbCoinAddDispExec(playerNo, -20, FALSE, TRUE);
            mbPlayerPosGet(playerNo, &playerPos);
            SNpcStarCreate(isKoopa, TRUE, &playerPos);
            mbPlayerRotateStart(playerNo, 0, 15);
            SNpcObjMotShiftSet(7);
            HuPrcSleep(150);
            SNpcTargetAngleSet(lbl_802C3290);
            mbPlayerMotionShiftSet(playerNo, 11, lbl_802C3290,
                lbl_802C3294, HU3D_MOTATTR_NONE);
            mbMusPauseFadeOut(0, TRUE, 1000);
            mbPlayerMotionEndWait(playerNo);
            mbPlayerMotIdleSet(playerNo);
            SNpcStarWait();
            omVibrate((s16)playerNo, 20, 7, 3);
            mbPlayerStarAdd(playerNo, 1);
            mbPlayerWinLoseVoicePlay(playerNo, playerStarMotNoTbl[isKoopa],
                playerStarSeTbl[isKoopa]);
            mbPlayerMotionShiftSet(playerNo, playerStarMotNoTbl[isKoopa],
                lbl_802C3290, lbl_802C3294, HU3D_MOTATTR_NONE);
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
            winNo = mbWinCreate(2, snpcMesTbl[4 + isKoopa],
                snpcMesSpeakerTbl[isKoopa]);
            mbWinWait((s16)winNo);
        }
    } else {
        mbWinWait((s16)winNo);
        if (state == 3) {
            winNo = mbWinCreate(2, snpcMesTbl[10 + isKoopa],
                snpcMesSpeakerTbl[isKoopa]);
            mbWinWait((s16)winNo);
        }
        mbPlayerPosGet(playerNo, &playerPos);
        SNpcStarCreate(isKoopa, TRUE, &playerPos);
        SNpcObjMotShiftSet(7);
        SNpcStarWait();
        omVibrate((s16)playerNo, 20, 7, 3);
        SNpcTargetAngleSet(lbl_802C3290);
        if (state == 2) {
            mbStarAddProcExec(playerNo, -1, -1, TRUE);
        } else {
            mbCoinAddProcExec(playerNo, -20, -1, TRUE);
        }
        mbPlayerWinLoseVoicePlay(playerNo, playerStarMotNoTbl[isKoopa],
            playerStarSeTbl[isKoopa]);
        mbPlayerMotionShiftSet(playerNo, playerStarMotNoTbl[isKoopa],
            lbl_802C3290, lbl_802C3294, HU3D_MOTATTR_NONE);
        SNpcSePlay(1, 0);
        SNpcObjMotShiftSet(8);
        SNpcObjMotEndWait();
        mbPlayerMotionEndWait(playerNo);
    }

    if (moveStarF) {
        nextMasu = SNpcMasuStarNextGet(FALSE);
        SNpcObjMotShiftSet(3);
        SNpcObjMotEndWait();
        if (isKoopa) {
            mbCameraShakeSet(12, lbl_802C3298);
        }
        SNpcSePlay(3, 0);
        SNpcObjMotShiftSet(4);
        SNpcObjPosGet(&starPos);
        for (i = 0; i < 30; i++) {
            ratio = (float)i / 30.0f;
            npcPos = starPos;
            npcPos.y += lbl_802C32A0
                * (float)sin((lbl_802C3280 * (lbl_802C329C * ratio))
                    / lbl_802C3278);
            SNpcObjPosSetV(&npcPos);
            HuPrcVSleep();
        }
        SNpcObjDispSet(FALSE);
        SNpcMasuReset(TRUE);
        mbCameraMovePlayer(-1, NULL, NULL,
            mbCameraPlayerViewZoomGet(2), lbl_802C3270, 24);
        mbCameraMoveWait();
        mbMasuPosGet(nextMasu, &masuPos);
        mbMasuPosGet(masuId, &npcPos);
        PSVECSubtract(&npcPos, &masuPos, &npcPos);
        zoom = PSVECMag(&npcPos) * lbl_802C32A4 * lbl_802C32A8;
        mbCameraMoveMasu((s16)nextMasu, NULL, NULL,
            mbCameraPlayerViewZoomGet(2), lbl_802C3270, (s16)zoom);
        winNo = mbWinCreate(2, snpcMesTbl[6 + isKoopa],
            mbGuideSpeakerNoGet());
        mbWinPause((s16)winNo);
        mbCameraMoveWait();
        mbWinKill((s16)winNo);
        mbCameraMoveMasu(-1, NULL, NULL,
            mbCameraPlayerViewZoomGet(0), lbl_802C3270, 21);
        SNpcObjDispSet(TRUE);
        mbMasuPosGet(nextMasu, &starPos);
        SNpcObjRotSet(lbl_802C3290, lbl_802C3290, lbl_802C3290);
        for (i = 0; i < 30; i++) {
            ratio = (float)(29 - i) / 30.0f;
            npcPos = starPos;
            npcPos.y += lbl_802C32A0
                * (float)sin((lbl_802C3280 * (lbl_802C329C * ratio))
                    / lbl_802C3278);
            SNpcObjPosSetV(&npcPos);
            if (i + 5 == 30) {
                SNpcSePlay(7, 0);
                SNpcObjMotShiftSet(5);
            }
            HuPrcVSleep();
        }
        if (isKoopa) {
            mbCameraShakeSet(18, lbl_802C32AC);
        }
        SNpcObjPosSetV(&starPos);
        SNpcMasuSet(nextMasu, TRUE);
        SNpcObjMotEndWait();
        winNo = mbWinCreate(2, snpcMesTbl[8 + isKoopa],
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
        mbPlayerRotSet(playerNo, lbl_802C3290, lbl_802C3290,
            lbl_802C3290);
        SNpcPosFixSnap();
    } else {
        mbWipeDissolveFadeOut();
        mbCameraStackPop(0);
        SNpcObjPosGet(&npcPos);
        mbPlayerPosGet(playerNo, &playerPos);
        SNpcObjPosSetV(&playerPos);
        mbPlayerPosSetV(playerNo, &npcPos);
        mbPlayerRotGet(playerNo, &playerRot);
        SNpcObjRotSet(lbl_802C3290, playerRot.y, lbl_802C3290);
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

static void SNpcMoveOMExec(OMOBJ *obj)
{
    SNPCMOVEWORK *work;
    HuVecF move;
    HuVecF pos;
    float time;
    float weight;
    BOOL endF;

    work = omObjGetWork(obj, SNPCMOVEWORK);
    endF = FALSE;
    if (mbExitCheck() || work->killF) {
        omDelObjEx(HuPrcCurrentGet(), obj);
        snpcWork->moveObj = NULL;
        return;
    }
    if (!work->initF) {
        work->initF = TRUE;
        work->time = 0;
        work->unk06 = 0;
        work->unk08 = 0;
        SNpcObjPosGet(&obj->trans);
        SNpcObjRotGet(&obj->rot);
        mbMasuPosGet(work->masuId, &obj->scale);
        PSVECSubtract(&obj->scale, &obj->trans, &move);
        time = PSVECMag(&move);
        if ((snpcSaveWork->flags >> 7) & 1) {
            time *= lbl_802C32E0;
        }
        work->maxTime = (s16)(time / lbl_802C32E4);
        obj->rot.z = (float)(lbl_802C3278
            * (atan2(move.x, move.z) / lbl_802C3280));
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

    weight = lbl_802C32E8 * (float)work->unk06;
    if (weight > lbl_802C32EC) {
        weight = lbl_802C32EC;
    }
    obj->rot.y = mbAngleLerp(obj->rot.y, obj->rot.z,
        lbl_802C32F0 * weight);
    SNpcObjRotSet(lbl_802C3290, obj->rot.y, lbl_802C3290);

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
            weight = (float)(lbl_802C3300 * sin(
                (lbl_802C3280 * (lbl_802C3308 * time))
                    / lbl_802C3278));
            pos.y += lbl_802C32F8 * weight;
            SNpcObjPosSetV(&pos);
            if (work->time >= work->maxTime) {
                if ((snpcSaveWork->flags >> 7) & 1) {
                    mbCameraShakeSet(12, lbl_802C32A0);
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
    if (snpcWork->diceNumObj != NULL) {
        SNpcObjPosGet(&pos);
        mbDiceSNpcNumPosSet(snpcWork->diceNumObj, &pos);
    }
    SNpcMotSetNext();
    if (endF) {
        omDelObjEx(HuPrcCurrentGet(), obj);
        snpcWork->moveObj = NULL;
    }
}

static int SNpcDiceValueGet(int masuId, int diceNum, const int *typeTbl,
    u32 mAttr, float chance, s16 *pathTbl)
{
    typedef struct SNPCCOMPACTPATH {
        u8 chanceF;
        u8 masuId;
        s16 chance;
    } SNPCCOMPACTPATH;
    u8 *masuState;
    u8 *branchCount;
    SNPCCHANCEPATH *pathStack;
    SNPCCOMPACTPATH *candidateTbl[64];
    int pathCapacity;
    int masuNum;
    int candidateNum;
    int minBranch;
    int current;
    int linkNo;
    int chanceCount;
    int pathCount;
    int i;
    int j;
    int result;

    pathCapacity = diceNum + 12;
    masuState = mbMalloc(256);
    pathStack = mbMalloc(pathCapacity * (int)sizeof(*pathStack));
    branchCount = mbMalloc(pathCapacity);
    for (i = 0; i < 64; i++) {
        candidateTbl[i] = NULL;
    }
    masuNum = mbMasuNumGet();
    for (i = 1; i < masuNum; i++) {
        s8 state = mbBranchAttrCheck(i) ? 0 : -1;
        if (mbMasuDispCheck((s16)i)) {
            state |= 1;
        }
        masuState[i] = (u8)state;
    }
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        masuState[GwPlayer[i].masuId] |= 2;
    }
    candidateNum = 0;
    minBranch = pathCapacity;
    if (diceNum > 0) {
        current = masuId;
        linkNo = 0;
        chanceCount = 0;
        pathCount = 0;
        while (pathCount >= 0) {
            MASU *masu;
            BOOL advanced;

            advanced = FALSE;
            if (pathCount < pathCapacity) {
                masu = mbMasuGet((s16)current);
                while (linkNo < masu->linkNum) {
                    int nextMasu;
                    int usedLink;

                    usedLink = linkNo++;
                    nextMasu = masu->linkTbl[usedLink];
                    if ((s8)masuState[nextMasu] < 0) {
                        continue;
                    }
                    branchCount[pathCount]++;
                    pathStack[pathCount].masuId = (s16)current;
                    pathStack[pathCount].linkNo = (s16)(usedLink + 1);
                    pathStack[pathCount].chance = (s16)chanceCount;
                    pathStack[pathCount].unk06 = 0;
                    pathCount++;
                    current = nextMasu;
                    linkNo = 0;
                    if (masuState[current] & 1) {
                        chanceCount++;
                    }
                    if ((masuState[current] & 112) == 0
                        && (mAttr & mbMasuMAttrGet((s16)current)) == 0) {
                        int type = mbMasuTypeGet((s16)current);
                        for (i = 0; typeTbl[i] >= 0; i++) {
                            if (type == typeTbl[i]) {
                                pathStack[pathCount - 1].unk06 = 1;
                                break;
                            }
                        }
                    }
                    advanced = TRUE;
                    break;
                }
            }
            if (advanced) {
                continue;
            }
            if (pathCount > 1) {
                BOOL marked = FALSE;
                for (i = 1; i < pathCount; i++) {
                    if (pathStack[i].unk06 != 0) {
                        marked = TRUE;
                        break;
                    }
                }
                if (marked && candidateNum < 64) {
                    SNPCCOMPACTPATH *candidate;
                    candidate = HuMemDirectMallocNum(0,
                        (pathCount + 1) * (int)sizeof(*candidate),
                        4096);
                    candidateTbl[candidateNum++] = candidate;
                    for (i = 1; i < pathCount; i++) {
                        candidate[i - 1].chanceF =
                            (u8)pathStack[i].unk06;
                        candidate[i - 1].masuId =
                            (u8)pathStack[i].masuId;
                        candidate[i - 1].chance = pathStack[i].chance;
                    }
                    candidate[pathCount - 1].chance = -1;
                    for (i = 0; i + 1 < pathCount; i++) {
                        if (branchCount[i] > 1 && i < minBranch) {
                            minBranch = i;
                        }
                    }
                }
            }
            if (pathCount == 0) {
                break;
            }
            pathCount--;
            if (pathCount == 0) {
                break;
            }
            current = pathStack[pathCount - 1].masuId;
            linkNo = pathStack[pathCount - 1].linkNo;
            chanceCount = pathStack[pathCount - 1].chance;
        }
    }
    result = 0;
    if (candidateNum > 0) {
        int playerFixed[GW_PLAYER_MAX] = { 0, 0, 0, 0 };
        int score[64];
        int order[64];
        int maxStars = 0;
        int limit;

        for (i = 0; i < GW_PLAYER_MAX; i++) {
            int stars = mbPlayerStarGet(i);
            if (stars > maxStars) {
                maxStars = stars;
            }
        }
        for (i = 0; i < candidateNum; i++) {
            SNPCCOMPACTPATH *candidate = candidateTbl[i];
            for (j = 0; candidate[j].chance >= 0; j++) {
                int player;
                if ((masuState[candidate[j].masuId] & 7) == 0) {
                    continue;
                }
                for (player = 0; player < GW_PLAYER_MAX; player++) {
                    if (GwPlayer[player].masuId == candidate[j].masuId) {
                        playerFixed[player] = 1;
                    }
                }
            }
        }
        limit = minBranch;
        if (limit <= 0 || limit > candidateNum) {
            limit = candidateNum;
        }
        for (i = 0; i < candidateNum; i++) {
            int playerDistance[GW_PLAYER_MAX] = { 0, 0, 0, 0 };
            int scoreValue = 2147483647;
            int player;
            SNPCCOMPACTPATH *candidate = candidateTbl[i];
            for (j = 0; candidate[j].chance >= 0; j++) {
                if ((masuState[candidate[j].masuId] & 7) == 0) {
                    continue;
                }
                for (player = 0; player < GW_PLAYER_MAX; player++) {
                    int value;
                    if (GwPlayer[player].masuId != candidate[j].masuId
                        || playerFixed[player]) {
                        continue;
                    }
                    value = candidate[j].chance;
                    if (value >= diceNum + 1) {
                        playerFixed[player] = 1;
                    } else if (playerDistance[player] == 0
                        || value < playerDistance[player]) {
                        playerDistance[player] = value;
                    }
                }
            }
            for (player = 0; player < GW_PLAYER_MAX; player++) {
                if (playerDistance[player] != 0) {
                    int value = mbPlayerStarGet(player) * 20;
                    if ((snpcSaveWork->flags >> 7) & 1) {
                        value = (maxStars - mbPlayerStarGet(player)) * 20;
                    }
                    value += playerDistance[player];
                    if (value < scoreValue) {
                        scoreValue = value;
                    }
                }
            }
            score[i] = scoreValue;
            order[i] = i;
        }
        for (i = 0; i < candidateNum; i++) {
            for (j = i + 1; j < candidateNum; j++) {
                if (score[order[i]] > score[order[j]]) {
                    int swap = order[i];
                    order[i] = order[j];
                    order[j] = swap;
                }
            }
        }
        if (limit > candidateNum) {
            limit = candidateNum;
        }
        {
            int best = score[order[0]];
            int tieCount = 0;
            int selected;
            for (i = 0; i < limit; i++) {
                if (score[order[i]] == best) {
                    tieCount++;
                }
            }
            if (tieCount == 0) {
                tieCount = 1;
            }
            selected = order[mbRandMod(tieCount)];
            {
                SNPCCOMPACTPATH *candidate = candidateTbl[selected];
                int selectedIndex = 0;
                int activeCount = 0;
                for (j = 0; candidate[j].chance >= 0; j++) {
                    if (masuState[candidate[j].masuId] != 0) {
                        activeCount++;
                    }
                }
                for (j = 0; candidate[j].chance >= 0; j++) {
                    pathTbl[j] = (s16)candidate[j].masuId;
                    if (candidate[j].chanceF != 0) {
                        activeCount--;
                        if (masuState[candidate[j].masuId] > 0) {
                            int threshold = (int)(lbl_802C32D8
                                + ((float)diceNum * chance));
                            if (candidate[j].chance < threshold) {
                                selectedIndex = j + 1;
                            } else {
                                activeCount = 0;
                            }
                        }
                    }
                }
                if (selectedIndex > 0
                    && mbCapMasuDispTypeGet(pathTbl[selectedIndex - 1]) != 0
                    && mbCapMasuDispTypeGet(pathTbl[selectedIndex]) == 0) {
                    selectedIndex++;
                }
                if (selectedIndex > 0) {
                    result = candidate[selectedIndex - 1].chance;
                    pathTbl[selectedIndex] = 0;
                } else {
                    pathTbl[0] = 0;
                }
            }
        }
    }
    for (i = 0; i < 64; i++) {
        if (candidateTbl[i] != NULL) {
            HuMemDirectFree(candidateTbl[i]);
        }
    }
    HuMemDirectFree(masuState);
    HuMemDirectFree(pathStack);
    HuMemDirectFree(branchCount);
    return result;
}

static void SNpcTargetAngleSet(float angle)
{
    snpcWork->rotateObj = omAddObjEx(mbObjMan, SNPC_MOVE_OBJ_PRIORITY, 0,
        0, OM_GRP_NONE, SNpcRotateUpdate);
    omObjGetWork(snpcWork->rotateObj, SNPCROTATEWORK)->targetAngle = angle;
}

static void SNpcRotateWait(void)
{
    BOOL waitF;

    do {
        HuPrcVSleep();
        waitF = snpcWork->rotateObj != NULL;
        if (!waitF) {
            waitF = mbObjMotionShiftIDGet(snpcWork->objId[0]) != -1;
        }
    } while (waitF);
}

static void SNpcRotateUpdate(OMOBJ *obj)
{
    SNPCROTATEWORK *work;
    BOOL endF;
    float time;
    float weight;

    work = omObjGetWork(obj, SNPCROTATEWORK);
    endF = FALSE;
    if (mbExitCheck() || work->killF) {
        omDelObjEx(HuPrcCurrentGet(), obj);
        snpcWork->rotateObj = NULL;
        return;
    }
    if (!work->initF) {
        float absAngle;

        work->initF = TRUE;
        work->time = 0;
        work->unk06 = 0;
        SNpcObjRotGet(&obj->rot);
        obj->scale.y = mbAngleWrap2(work->targetAngle, obj->rot.y);
        absAngle = fabs(obj->scale.y);
        time = lbl_802C330C * absAngle;
        work->maxTime = (s16)(lbl_802C3314
            * (lbl_802C3310 + (time
                * snpcRotSpeedTbl[(snpcSaveWork->flags >> 7) & 1])));
        if ((float)abs((s32)obj->scale.y) < lbl_802C3318) {
            SNpcObjMotShiftSet(0);
        } else {
            SNpcObjMotShiftSet(1);
        }
    }
    time = (float)work->time++ / (float)work->maxTime;
    weight = (float)(lbl_802C3320 + (lbl_802C3320
        * sin((lbl_802C3280 * (lbl_802C3328
            + (lbl_802C3308 * time))) / lbl_802C3278)));
    SNpcObjRotSet(obj->rot.x, obj->rot.y
        + (weight * obj->scale.y), obj->rot.z);
    if (work->time >= work->maxTime) {
        endF = TRUE;
        SNpcObjMotShiftSet(0);
    }
    SNpcMotSetNext();
    if (endF) {
        omDelObjEx(HuPrcCurrentGet(), obj);
        snpcWork->rotateObj = NULL;
    }
}

static void SNpcPosFixUpdate(OMOBJ *obj)
{
    SNPCROTATEWORK *work;
    HuVecF startPos;
    HuVecF startRot;
    HuVecF targetPos;
    HuVecF pos;
    BOOL endF;
    float time;
    float weight;

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
        SNpcObjPosGet(&startPos);
        SNpcObjRotGet(&startRot);
        SNpcPosFixSnap();
        SNpcObjPosGet(&targetPos);
        SNpcObjPosSetV(&startPos);
        SNpcObjRotSetV(&startRot);
        obj->trans = startPos;
        obj->scale = targetPos;
        work->maxTime = 30;
        work->time = 0;
        SNpcObjMotShiftSet(1);
    }
    if (work->unk06 == 0) {
        time = (float)work->time++ / (float)work->maxTime;
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
            work->targetAngle = lbl_802C3290;
            work->targetAngle = mbAngleWrap2(work->targetAngle, obj->rot.y);
            obj->scale.y = work->targetAngle;
            time = lbl_802C330C * fabs(obj->scale.y);
            work->maxTime = (s16)(lbl_802C3314
                * (lbl_802C3310 + (time
                    * snpcPosFixSpeedTbl[(snpcSaveWork->flags >> 7) & 1])));
            work->time = 0;
            if ((float)abs((s32)obj->scale.y) < lbl_802C3318) {
                SNpcObjMotShiftSet(0);
            } else {
                SNpcObjMotShiftSet(1);
            }
        }
    } else {
        time = (float)work->time++ / (float)work->maxTime;
        weight = (float)(lbl_802C3320 + (lbl_802C3320
            * sin((lbl_802C3280 * (lbl_802C3328
                + (lbl_802C3308 * time))) / lbl_802C3278)));
        SNpcObjRotSet(obj->rot.x, obj->rot.y
            + (weight * obj->scale.y), obj->rot.z);
        if (work->time >= work->maxTime) {
            endF = TRUE;
            SNpcObjMotShiftSet(0);
        }
        SNpcMotSetNext();
        if (endF) {
            work->killF = TRUE;
        }
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

static void SNpcStarWait(void)
{
    while (snpcWork->starObj != NULL) {
        HuPrcVSleep();
    }
}

static int SNpcMasuStarNextGet(BOOL playerF)
{
    int type;
    int linkNoTbl[3] = { 0, 0, -1 };

    type = 10;
    if (playerF) {
        type = 4;
    }
    return mbSNpcMasuStarNextGet(snpcSaveWork->masuId, type, linkNoTbl,
        snpcWork->unk04);
}

static void SNpcMasuSet(int masuId, BOOL setF)
{
    int type;
    int speaker;

    snpcSaveWork->masuId = (u8)masuId;
    snpcWork->unk04 = mbMasuTypeGet((s16)masuId);
    type = snpcMasuTypeTbl[(snpcSaveWork->flags >> 7) & 1];
    mbMasuTypeSet((s16)masuId, type);
    mbMasuCapsuleSet((s16)masuId, -1);
    if (setF) {
        speaker = snpcMasuSeTbl[(snpcSaveWork->flags >> 7) & 1];
        mbAudFXPlay((s16)speaker);
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

static void SNpcPosFixCreate(void)
{
    snpcWork->rotateObj = omAddObjEx(mbObjMan, SNPC_MOVE_OBJ_PRIORITY, 0, 0,
        OM_GRP_NONE, SNpcPosFixUpdate);
}

static void SNpcPosFixSnap(void)
{
    int masuId;
    int linkNo;
    s16 linkMasuId;
    u32 branchAttr;
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
        branchAttr = mbBranchMAttrGet();
        if ((branchAttr & mbMasuMAttrGet(linkMasuId)) == 0) {
            break;
        }
        linkNo++;
    }
    mbMasuPosGet((s16)masuId, &pos);
    mbMasuPosGet(linkMasuId, &posLink);
    PSVECSubtract(&posLink, &pos, &posLink);
    posLink.y = lbl_802C3290;
    PSVECNormalize(&posLink, &posLink);
    PSVECScale(&posLink, &posLink, lbl_802C3344);
    mbMasuMtxGet((s16)masuId, masuMtx);
    PSMTXMultVec(masuMtx, &posLink, &pos);
    SNpcObjPosSetV(&pos);
    SNpcObjRotSet(lbl_802C3290, lbl_802C3290, lbl_802C3290);
}

static void SNpcSePlay(int seNo, int unused)
{
    if (snpcSeTbl[seNo][(snpcSaveWork->flags >> 7) & 1] != 0) {
        mbAudFXPlay((s16)snpcSeTbl[seNo][(snpcSaveWork->flags >> 7) & 1]);
    }
}

static void SNpcMotSetNext(void)
{
    const SNPCMOTNEXTDATA *nextMot;
    int motNo;
    int frame;
    int i;

    nextMot = &snpcNextMotTbl[(snpcSaveWork->flags >> 7) & 1]
        [snpcWork->motNo];
    if (nextMot->motNo == 0) {
        return;
    }
    if (mbObjMotionShiftIDGet(snpcWork->objId[0]) != -1) {
        frame = (int)Hu3DMotionShiftTimeGet(
            mbObjModelIDGet(snpcWork->objId[0]));
    } else {
        frame = (int)mbObjMotionTimeGet(snpcWork->objId[0]);
    }
    for (i = 0; i < 3 && snpcWork->motShiftNo < nextMot->frame[i]; i++) {
    }
    if (i < 3 && nextMot->frame[i] >= 0 && nextMot->frame[i] <= frame) {
        SNpcSePlay(nextMot->motNo, 0);
        snpcWork->motShiftNo = frame;
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
    int valueTbl[3] = { 0, 0, 0 };
    int isKoopa;
    HuVecF pos;
    int i;

    isKoopa = (snpcSaveWork->flags >> 7) & 1;
    SNpcObjPosGet(&pos);
    PSVECAdd(&pos, &snpcDiceOfsTbl[isKoopa], &pos);
    if (diceNum > 2) {
        diceNum = 2;
    }
    valueTbl[0] = diceValue;
    if (diceNum == 2 && diceValue > 1) {
        int split;
        int limit;

        split = diceValue >> 1;
        valueTbl[0] = split;
        valueTbl[1] = diceValue - split;
        limit = 10 - valueTbl[1];
        if (limit > valueTbl[0] - 1) {
            limit = valueTbl[0] - 1;
        }
        i = mbRandMod(limit + 1);
        valueTbl[0] -= i;
        valueTbl[1] += i;
        if (mbRandMod(100) < 80) {
            int value;

            value = valueTbl[0];
            valueTbl[0] = valueTbl[1];
            valueTbl[1] = value;
        }
    }
    for (i = 0; i < 3; i++) {
        valueTbl[i]--;
    }
    mbDiceProcExec(-1, snpcDiceTypeTbl[diceNum - 1][isKoopa],
        (s8 *)valueTbl, NULL, FALSE, FALSE, &pos,
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
        if (time++ == snpcDiceMotTimeTbl[(snpcSaveWork->flags >> 7) & 1]) {
            mbDiceObjHit(-1);
        }
        HuPrcVSleep();
    } while (!SNpcObjMotEndCheck());
    SNpcObjMotShiftSet(0);
}

static void SNpcPlayerMoveFunc(int playerNo)
{
    OMOBJ *obj;
    SNPCMOVEWORK *work;
    MASU *masu;
    MASU *masuPrev;
    HuVecF moveDir;
    HuVecF playerRot;
    HuVecF linkPos;
    HuVecF masuPos;
    Mtx masuMtx;
    s16 masuIdNext;
    s16 masuIdPrev;
    int linkNo;
    s16 linkMasu;
    u32 branchAttr;
    s16 maxTime;
    s16 masuId;
    float motSpeed;
    float rotY;
    BOOL setAngle;

    motSpeed = lbl_802C32EC;
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
        for (linkNo = 0; linkNo < mbMasuLinkNumGet(masuId); linkNo++) {
            linkMasu = mbMasuLinkGet(masuId, linkNo);
            branchAttr = mbBranchMAttrGet();
            if ((branchAttr & mbMasuMAttrGet(linkMasu)) == 0) {
                break;
            }
        }
        mbMasuPosGet(masuId, &masuPos);
        mbMasuPosGet(linkMasu, &linkPos);
        PSVECSubtract(&linkPos, &masuPos, &linkPos);
        linkPos.y = lbl_802C3290;
        PSVECNormalize(&linkPos, &linkPos);
        PSVECScale(&linkPos, &linkPos, lbl_802C3348);
        mbMasuMtxGet(masuId, masuMtx);
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
        mbPlayerMotionShiftSet(playerNo, 3, lbl_802C3290,
            lbl_802C336C, HU3D_MOTATTR_LOOP);
        break;
    case 1:
        mbPlayerWorkGet(playerNo)->_unk0C = 2;
        mbPlayerMotionShiftSet(playerNo, 4, lbl_802C33C0,
            lbl_802C332C, 0);
        maxTime = 24;
        break;
    case 2:
        mbPlayerWorkGet(playerNo)->_unk0C = 3;
        mbPlayerMotionShiftSet(playerNo, 14, lbl_802C3290,
            lbl_802C336C, HU3D_MOTATTR_LOOP);
        maxTime = 100;
        motSpeed = lbl_802C332C;
        PSVECSubtract(&obj->rot, &obj->trans, &moveDir);
        maxTime = (int)(PSVECMag(&moveDir) / lbl_802C3358);
        if (obj->trans.y >= obj->rot.y) {
            moveDir.x = -moveDir.x;
            moveDir.y = -moveDir.y;
            moveDir.z = -moveDir.z;
            motSpeed = -motSpeed;
        }
        playerRot.x = lbl_802C3290;
        playerRot.z = lbl_802C3290;
        playerRot.y = (float)(lbl_802C3278
            * (atan2(moveDir.x, moveDir.z) / lbl_802C3280));
        mbPlayerRotSetV(playerNo, &playerRot);
        setAngle = TRUE;
        break;
    }
    mbPlayerMotionSpeedSet(playerNo, motSpeed);
    if (!setAngle) {
        PSVECSubtract(&obj->rot, &obj->trans, &playerRot);
        rotY = (float)(lbl_802C33C8 - (lbl_802C3278
            * (atan2(playerRot.z, playerRot.x) / lbl_802C3280)));
        mbPlayerRotYSet(playerNo, rotY);
    }
    obj->scale.x = obj->trans.x;
    obj->scale.y = obj->trans.y;
    obj->scale.z = obj->trans.z;
    work->playerNo = playerNo;
    work->time = 0;
    work->maxTime = maxTime;
    mbPlayerWorkGet(work->playerNo)->_unk08 = work->maxTime;
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
    mbPlayerWorkGet(work->playerNo)->_unk08 = work->maxTime - work->time;
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
    mbPlayerWorkGet(work->playerNo)->moveF = FALSE;
    if (work->time >= work->maxTime - 2) {
        time = 1.0f;
        mbPlayerWorkGet(work->playerNo)->moveF = TRUE;
    } else {
        time = (float)work->time / (float)(work->maxTime - 2);
    }
    mbPlayerPosSet(work->playerNo, obj->trans.x,
        obj->trans.y + lbl_802C32F8 * (lbl_802C33D0
            * sin((lbl_802C3280 * (lbl_802C3308 * time))
                / lbl_802C3278)), obj->trans.z);
    if (work->time == work->maxTime - 5) {
        mbPlayerMotionShiftSet(work->playerNo, 5, lbl_802C332C,
            lbl_802C332C, 0);
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
        obj->trans.y += lbl_802C33EC;
        if (!work->unk00) {
            obj->trans.y += lbl_802C33F0;
        }
        obj->rot.z = lbl_802C3290;
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
        time = lbl_802C32EC - time;
        obj->rot.z = lbl_802C33F4 * time;
        if (work->time >= work->maxTime) {
            work->effectNo++;
            work->time = 0;
            work->maxTime = 60;
        }
        break;
    case 2:
        time = (float)work->time / (float)work->maxTime;
        obj->rot.z = lbl_802C32A0 * (lbl_802C3364
            * mbSinDeg(lbl_802C3308 + (lbl_802C3330 * time)));
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
        obj->rot.y += lbl_802C33F8;
        if (obj->rot.y >= lbl_802C3330) {
            obj->rot.y -= lbl_802C3330;
        }
        time = (float)work->time / (float)work->maxTime;
        time = lbl_802C32EC - time;
        if (time <= lbl_802C3290) {
            time = lbl_802C33FC;
        }
        obj->scale.z = time;
        obj->scale.x = time;
        if (work->unk00) {
            obj->rot.z -= lbl_802C3400;
        }
        if (work->time >= work->maxTime) {
            work->effectNo++;
            work->killF = TRUE;
        }
        break;
    }
    mbObjPosSet(obj->mdlId[0], obj->trans.x,
        obj->trans.y + obj->rot.z, obj->trans.z);
    mbObjRotSet(obj->mdlId[0], lbl_802C3290, obj->rot.y,
        lbl_802C3290);
    mbObjScaleSet(obj->mdlId[0], obj->scale.x, obj->scale.y,
        obj->scale.z);
    mbParManPosSet((int)obj->mdlId[1], obj->trans.x,
        obj->trans.y + obj->rot.z, obj->trans.z);
}

static void SNpcStarEffKill(s16 parManId)
{
    mbParManKill((s16)parManId);
}

static void SNpcStarFunc(void)
{
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

static void GetStarTexTevStage(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material,
    int *tevStageNum, int *texGenNum)
{
    HSF_CONSTDATA *constData;
    HU3D_ATTR_ANIM *animWork;
    HSF_ATTRIBUTE *attribute;
    HSF_BITMAP *bitmap;
    HSF_OBJECT *object;
    HU3D_MODEL *model;
    u32 flags;
    u16 matHiliteF;
    u16 lightOnF;
    u16 projMask;
    u16 i;
    int specialAttrNo;
    int bumpAttrNo;
    BOOL texBlendF;
    BOOL shineF;
    int tevStage;
    int texGen;

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
            }
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
    shineF = Hu3DShineF && lightOnF;
    constData = object->constData;
    if (material->attrNum == 1) {
        tevStage = 1;
        texGen = 1;
        attribute = &object->mesh.attribute[material->attr[0]];
        if (attribute->unk20 == 1.0f) {
            if (attribute->unk8[2] == 0) {
                tevStage++;
            } else if (!(model->attr & HU3D_ATTR_TOON_MAP)
                && (texCol[0].a == 1 || texCol[0].a == 2)) {
                tevStage++;
            }
        }
        if (model->attr & HU3D_ATTR_TOON_MAP) {
            texGen++;
            tevStage++;
        }
        if (material->refAlpha != 0.0f) {
            texGen++;
            tevStage++;
        }
        if (shineF) {
            tevStage++;
        }
        if (Hu3DShadowF && shadowNum
            && (constData->attr & HU3D_CONST_SHADOW_MAP)) {
            if (constData->attr & HU3D_CONST_SHADOW_MAP_TPLVL) {
                tevStage++;
            }
            texGen++;
            tevStage++;
        }
        if (matHiliteF) {
            if ((model->attr & HU3D_ATTR_HILITE)
                || (flags & HSF_MATERIAL_HILITE)) {
                texGen++;
                tevStage++;
                matHiliteF = FALSE;
            } else {
                if (attribute->unk20 != 1.0f) {
                    texGen++;
                }
                tevStage++;
            }
        } else if (material->invAlpha != 0.0f) {
            tevStage++;
        }
        for (i = 0, projMask = 1; i < 4; i++, projMask <<= 1) {
            if (model->projBit & projMask) {
                texGen++;
                tevStage += 2;
            }
        }
    } else {
        texBlendF = FALSE;
        texGen = 0;
        bumpAttrNo = -1;
        tevStage = 0;
        for (i = 0; i < material->attrNum; i++) {
            attribute = &object->mesh.attribute[material->attr[i]];
            if (attribute->nbtTpLvl != 0.0f) {
                tevStage++;
                bumpAttrNo = i;
                texGen++;
                texBlendF = TRUE;
                continue;
            }
            if (attribute->unk20 != 1.0f) {
                specialAttrNo = i;
                continue;
            }
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
            tevStage++;
        }
        if (model->attr & HU3D_ATTR_TOON_MAP) {
            texGen++;
            tevStage++;
        }
        if (material->refAlpha != 0.0f) {
            if (specialAttrNo != -1) {
                texGen++;
                tevStage++;
            }
            texGen++;
            tevStage++;
        }
        if (shineF) {
            tevStage++;
        }
        if (Hu3DShadowF && shadowNum
            && (constData->attr & HU3D_CONST_SHADOW_MAP)) {
            if (constData->attr & HU3D_CONST_SHADOW_MAP_TPLVL) {
                tevStage++;
            }
            texGen++;
            tevStage++;
        }
        if (matHiliteF) {
            if ((model->attr & HU3D_ATTR_HILITE)
                || (flags & HSF_MATERIAL_HILITE)) {
                if (specialAttrNo != -1) {
                    texGen++;
                    tevStage++;
                }
                texGen++;
                tevStage++;
                matHiliteF = FALSE;
            } else {
                if (specialAttrNo != -1) {
                    texGen++;
                }
                tevStage++;
            }
        } else if (material->invAlpha != 0.0f) {
            tevStage++;
        }
        for (i = 0, projMask = 1; i < 4; i++, projMask <<= 1) {
            if (model->projBit & projMask) {
                texGen++;
                tevStage += 2;
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
    HSF_CONSTDATA *constData;
    HSF_OBJECT *object;
    HU3D_MODEL *model;
    u32 flags;
    s16 matHiliteF;
    s16 projMask;
    int tevStage;
    int texGen;
    int i;

    tevStage = 1;
    texGen = 0;
    object = drawObj->object;
    model = drawObj->model;
    flags = object->flags | material->flags;
    matHiliteF = material->vtxMode == 2 || material->vtxMode == 3;
    if (model->attr & HU3D_ATTR_TOON_MAP) {
        texGen++;
    }
    if (material->refAlpha != 0.0f) {
        tevStage++;
        texGen++;
    }
    constData = object->constData;
    if (Hu3DShadowF && shadowNum
        && (constData->attr & HU3D_CONST_SHADOW_MAP)) {
        if (constData->attr & HU3D_CONST_SHADOW_MAP_TPLVL) {
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
    } else if (material->invAlpha != 0.0f) {
        tevStage++;
    }
    for (i = 0, projMask = 1; i < 4; i++, projMask <<= 1) {
        if (model->projBit & projMask) {
            texGen++;
            tevStage += 2;
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
    work->magic = MBOBJ_FADE_WORK_MAGIC;
    work->pos = *pos;
    work->alpha = lbl_802C32EC;
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
    if (alpha < 0.000001f) {
        alpha = 0.000001f;
    }
    PSMTXScale(workMtx, 0.001f, -0.01f / alpha, 1.0f);
    PSMTXConcat(workMtx, texMtx, texMtx);
    texMtx[1][3] += 0.96875f;
    GXLoadTexMtxImm(texMtx, GX_TEXMTX4, GX_MTX2x4);
    GXSetNumTexGens(texGen + 1);
    GXSetNumTevStages(tevStage + 1);
    GXSetTexCoordGen2(texGen, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX4,
        GX_FALSE, GX_PTIDENTITY);
    GXSetTevOrder(tevStage, texGen, GX_TEXMAP4, GX_COLOR_NULL);
    GXSetTevKColor(GX_KCOLOR3, work->color);
    GXSetTevKColorSel(tevStage, GX_TEV_KCSEL_K3);
    GXSetTevColorIn(tevStage, GX_CC_CPREV, GX_CC_TEXC, GX_CC_KONST,
        GX_CC_ZERO);
    GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV,
        GX_CA_ZERO);
    GXSetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    if ((drawObj->object->flags | material->flags)
        & (HSF_MATERIAL_NEAR | HSF_MATERIAL_DISABLE_ZWRITE)) {
        GXSetAlphaCompare(GX_GREATER, 128, GX_AOP_OR, GX_GREATER, 128);
    } else {
        GXSetAlphaCompare(GX_GREATER, 1, GX_AOP_AND, GX_GREATER, 1);
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
    work->pos = *pos;
    work->rot = *rot;
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
    work->tpLvl = lbl_802C32EC;
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
    HuVecF viewDir = { 0.0f, 0.0f, -1.0f };
    HuVecF lightDir;
    HuVecF axis;
    Mtx texMtx;
    Mtx workMtx;
    GXColor color;
    float angle;
    int tevStage;
    int texGen;

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
        -0.5f / drawObj->scale.y, 0.5f / drawObj->scale.z);
    PSMTXConcat(workMtx, texMtx, texMtx);
    texMtx[0][3] = 0.5f;
    texMtx[1][3] = 0.5f;
    GXLoadTexMtxImm(texMtx, GX_TEXMTX4, GX_MTX2x4);
    PSMTXCopy(drawObj->matrix, texMtx);
    PSMTXMultVecSR(Hu3DCameraMtx, &Hu3DGlobalLight[0].dir, &lightDir);
    C_VECHalfAngle(&viewDir, &lightDir, &lightDir);
    if (fabsf(lightDir.z) < 0.999f) {
        angle = (float)acos(lightDir.z);
        PSVECCrossProduct(&viewDir, &lightDir, &axis);
        PSMTXRotAxisRad(workMtx, &axis, angle);
        PSMTXConcat(workMtx, texMtx, texMtx);
    }
    PSMTXScale(workMtx, 0.5f / drawObj->scale.x,
        -0.5f / drawObj->scale.y, 0.5f / drawObj->scale.z);
    PSMTXConcat(workMtx, texMtx, texMtx);
    texMtx[0][3] = 0.5f;
    texMtx[1][3] = 0.5f;
    GXLoadTexMtxImm(texMtx, GX_TEXMTX5, GX_MTX2x4);
    GXSetNumTexGens(texGen + 2);
    GXSetNumTevStages(tevStage + 3);
    GXSetTexCoordGen2(texGen, GX_TG_MTX2x4, GX_TG_NRM, GX_TEXMTX4,
        GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordGen2(texGen + 1, GX_TG_MTX2x4, GX_TG_NRM, GX_TEXMTX5,
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
    work->level = lbl_802C3290;
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
    GXColor color;
    int tevStage;
    int texGen;
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
            color.r = work->color.r * work->level;
            color.g = work->color.g * work->level;
            color.b = work->color.b * work->level;
            break;
    }
    color.a = 255.0f * work->level;
    GXSetTevKColor(GX_KCOLOR3, color);
    GXSetTevKColorSel(tevStage, GX_TEV_KCSEL_K3);
    GXSetTevKAlphaSel(tevStage, GX_TEV_KASEL_K3_A);
    tevConfig = &biriQMatTbl[work->mode][0][0];
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

static void SNpcObjCreate(void)
{
    int modelData[2] = { SNPC_DONKEY_DATA_MODEL, SNPC_KOOPA_DATA_MODEL };
    int i;
    HuVecF pos;

    snpcWork->objId[0] = mbObjCreate(modelData[(snpcSaveWork->flags >> 7) & 1],
        NULL, TRUE);
    mbObjLayerSet(snpcWork->objId[0], 3);
    for (i = 0; i < SNPC_MOTION_NUM; i++) {
        snpcWork->motionId[i] = mbObjMotionCreate(snpcWork->objId[0],
            snpcMotTbl[(snpcSaveWork->flags >> 7) & 1][i].dataNum);
    }
    snpcWork->motNo = -1;
    SNpcObjMotSet(0);
    mbMasuPosGet(snpcSaveWork->masuId, &pos);
    SNpcObjPosSetV(&pos);
    SNpcObjRotSet(lbl_802C3290, lbl_802C3290, lbl_802C3290);
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
    snpcWork->pos = *pos;
    mbObjPosSetV(snpcWork->objId[0], &snpcWork->pos);
}

static void SNpcObjPosGet(HuVecF *pos)
{
    *pos = snpcWork->pos;
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
    snpcWork->rot = *rot;
    mbObjRotSetV(snpcWork->objId[0], &snpcWork->rot);
}

static void SNpcObjRotGet(HuVecF *rot)
{
    *rot = snpcWork->rot;
}

static void SNpcObjMotSet(int motNo)
{
    const SNPCMOTDATA *motData;
    int start;
    int end;

    motData = &snpcMotTbl[(snpcSaveWork->flags >> 7) & 1][motNo];
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
    motData = &snpcMotTbl[(snpcSaveWork->flags >> 7) & 1][motNo];
    start = 0;
    if (motData->startFrame != 0) {
        start = motData->startFrame;
    }
    mbObjMotionShiftSet(snpcWork->objId[0], snpcWork->motionId[motNo],
        (float)motData->startFrame, lbl_802C3294,
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
    pos.y += lbl_802C3318;
    mbObjPosSetV(snpcWork->objId[2], &pos);
    mbObjDispSet(snpcWork->objId[2], TRUE);
    mbObjMotionTimeSet(snpcWork->objId[2], lbl_802C3290);
}

static void SNpcMasuEffDispSet(void)
{
    mbObjDispSet(snpcWork->objId[2], FALSE);
}

u8 *mbMasuChanceCreate(int masuId, int chance, int branchChance)
{
    u8 *chanceTbl;
    s8 *masuState;
    SNPCCHANCEPATH *pathTbl;
    SNPCCHANCEINDEX *linkIndex;
    u8 *linkTbl;
    int masuNum;
    int current;
    int currentChance;
    int linkNo;
    int linkCount;
    int found;
    int pathCapacity;
    int prevMasuId;
    int i;
    MASU *masu;
    SNPCCHANCEPATH *pathCursor;

    chanceTbl = mbMalloc(SNPC_CHANCE_TBL_SIZE);
    masuState = mbMalloc(SNPC_CHANCE_TBL_SIZE);
    pathTbl = HuMemDirectMallocNum(0,
        SNPC_CHANCE_TBL_SIZE * sizeof(*pathTbl), HU_MEMNUM_OVL);
    masuNum = mbMasuNumGet();
    for (i = 1; i < masuNum; i++) {
        if (mbBranchAttrCheck(i)) {
            masuState[i] = 0;
        } else {
            masuState[i] = -1;
        }
        masuState[i] |= (s8)mbMasuDispCheck((s16)i);
    }
    if (chance != 0) {
        current = masuId;
        currentChance = chance;
        pathCursor = pathTbl;
        pathCapacity = SNPC_CHANCE_TBL_SIZE;
        chanceTbl[current] = (u8)currentChance;
        if (masuState[current] != 0) {
            currentChance--;
        }
        for (;;) {
            found = FALSE;
            masu = mbMasuGet((s16)current);
            linkCount = 0;
            while (linkCount < masu->linkNum && !found) {
                int nextMasuId;

                linkNo = linkCount;
                linkCount++;
                prevMasuId = current;
                nextMasuId = masu->linkTbl[linkNo];
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
                pathCursor->masuId = (s16)prevMasuId;
                pathCursor->linkNo = (s16)linkCount;
                pathCursor->chance = (s16)currentChance;
                pathCursor++;
                pathCapacity--;
                current = nextMasuId;
                currentChance--;
                if (masuState[current] != 0) {
                    currentChance--;
                }
                found = TRUE;
            }
            if (!found) {
                pathCursor--;
                pathCapacity++;
                if (pathCursor < pathTbl) {
                    break;
                }
                current = pathCursor->masuId;
                linkCount = pathCursor->linkNo;
                currentChance = pathCursor->chance;
            }
        }
    }
    if (branchChance != 0) {
        linkIndex = mbMalloc(SNPC_CHANCE_TBL_SIZE * sizeof(*linkIndex));
        for (i = 1; i < masuNum; i++) {
            masu = mbMasuGet((s16)i);
            for (linkNo = 0; linkNo < masu->linkNum; linkNo++) {
                current = masu->linkTbl[linkNo];
                linkIndex[current].count++;
            }
        }
        linkCount = 0;
        for (i = 1; i < masuNum; i++) {
            linkIndex[i].start = (s16)linkCount;
            linkCount += linkIndex[i].count;
            linkIndex[i].count = 0;
        }
        linkTbl = HuMemDirectMallocNum(0,
            (linkCount + SNPC_CHANCE_BRANCH_PADDING) * sizeof(u32),
            HU_MEMNUM_OVL);
        for (i = 1; i < masuNum; i++) {
            masu = mbMasuGet((s16)i);
            for (linkNo = 0; linkNo < masu->linkNum; linkNo++) {
                current = masu->linkTbl[linkNo];
                linkTbl[linkIndex[current].start + linkIndex[current].count]
                    = (u8)i;
                linkIndex[current].count++;
            }
        }
        current = masuId;
        currentChance = branchChance;
        pathCursor = pathTbl;
        pathCapacity = SNPC_CHANCE_TBL_SIZE;
        chanceTbl[current] = (u8)currentChance;
        if (masuState[current] != 0) {
            currentChance--;
        }
        for (;;) {
            found = FALSE;
            linkCount = 0;
            while (linkCount < linkIndex[current].count && !found) {
                int prevMasuId;

                prevMasuId = linkTbl[linkIndex[current].start + linkCount];
                linkCount++;
                if (masuState[prevMasuId] < 0) {
                    continue;
                }
                if (chanceTbl[prevMasuId] >= currentChance) {
                    continue;
                }
                chanceTbl[prevMasuId] = (u8)currentChance;
                if (pathCapacity <= 0) {
                    continue;
                }
                if (currentChance <= 1 && masuState[prevMasuId] == 0) {
                    continue;
                }
                pathCursor->masuId = (s16)current;
                pathCursor->linkNo = (s16)linkCount;
                pathCursor->chance = (s16)currentChance;
                pathCursor++;
                pathCapacity--;
                current = prevMasuId;
                currentChance--;
                if (masuState[current] != 0) {
                    currentChance--;
                }
                found = TRUE;
            }
            if (!found) {
                pathCursor--;
                pathCapacity++;
                if (pathCursor < pathTbl) {
                    break;
                }
                current = pathCursor->masuId;
                linkCount = pathCursor->linkNo;
                currentChance = pathCursor->chance;
            }
        }
        HuMemDirectFree(linkTbl);
        HuMemDirectFree(linkIndex);
    }
    HuMemDirectFree(pathTbl);
    HuMemDirectFree(masuState);
    return chanceTbl;
}

s16 mbMasuChanceSet(u8 *chanceTbl, int masuId)
{
    HuVecF basePos;
    HuVecF pos;
    float distance;
    u8 *masuTbl;
    float *distanceTbl;
    int masuNum;
    int maxNum;
    int i;
    int j;
    int randomIndex;
    int validNum;
    int retryCount;
    int targetId;

    maxNum = 28;
    mbMasuPosGet(masuId, &basePos);
    basePos.y = 0.0f;
    if (mbMasuAttrGet(masuId) & MASU_FLAG_START) {
        if (mbRandMod(100) < 50) {
            basePos.x = lbl_802C33E0;
            basePos.z = lbl_802C33E4;
            maxNum = 20;
        } else {
            maxNum = 50;
        }
    }
    masuTbl = HuMemDirectMallocNum(0, SNPC_CHANCE_DISTANCE_BYTES,
        HU_MEMNUM_OVL);
    distanceTbl = HuMemDirectMallocNum(0, SNPC_CHANCE_MASU_BYTES,
        HU_MEMNUM_OVL);
    masuNum = mbMasuNumGet();
    validNum = 0;
    for (i = 1; i < masuNum; i++) {
        if (chanceTbl[i] != 0) {
            continue;
        }
        mbMasuPosGet((s16)i, &pos);
        pos.y = 0.0f;
        PSVECSubtract(&pos, &basePos, &pos);
        pos.z *= lbl_802C3378;
        distance = PSVECMag(&pos);
        if (validNum < maxNum) {
            distanceTbl[validNum] = distance;
            masuTbl[validNum] = (u8)i;
            validNum++;
        } else {
            float minDistance;

            minDistance = lbl_802C33E8;
            targetId = -1;
            for (j = 0; j < validNum; j++) {
                if (distanceTbl[j] < minDistance) {
                    targetId = j;
                    minDistance = distanceTbl[j];
                }
            }
            if (minDistance < distance) {
                distanceTbl[targetId] = distance;
                masuTbl[targetId] = (u8)i;
            }
        }
    }
    retryCount = 0;
    while (retryCount < 20) {
        randomIndex = mbRandMod(validNum);
        targetId = masuTbl[randomIndex];
        if (mbCapMasuDispTypeGet((s16)targetId) == 0) {
            break;
        }
        retryCount++;
    }
    HuMemDirectFree(masuTbl);
    HuMemDirectFree(distanceTbl);
    return targetId;
}

int mbSNpcMasuStarNextGet(s16 masuId, int type, int *linkNoTbl, u32 attr)
{
    u8 *chanceTbl;
    MASU *masu;
    int masuNum;
    int i;
    int j;
    BOOL typeMatch;
    int targetId;

    chanceTbl = mbMasuChanceCreate(masuId, type + 1, type / 4 + 1);
    masuNum = mbMasuNumGet();
    for (i = 1; i < masuNum; i++) {
        if (chanceTbl[i] != 0) {
            continue;
        }
        masu = mbMasuGet((s16)i);
        typeMatch = FALSE;
        for (j = 0; linkNoTbl[j] >= 0; j++) {
            if (masu->type == linkNoTbl[j]) {
                typeMatch = TRUE;
                break;
            }
        }
        if (!typeMatch) {
            chanceTbl[i] = SNPC_CHANCE_EXCLUDE;
        }
    }
    masuNum = mbMasuNumGet();
    for (i = 1; i < masuNum; i++) {
        if (chanceTbl[i] != 0) {
            continue;
        }
        masu = mbMasuGet((s16)i);
        if ((masu->mAttr & attr) != 0) {
            chanceTbl[i] = SNPC_CHANCE_EXCLUDE;
        }
    }
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        chanceTbl[GwPlayer[i].masuId] = SNPC_CHANCE_EXCLUDE;
    }
    targetId = mbMasuChanceSet(chanceTbl, masuId);
    HuMemDirectFree(chanceTbl);
    return targetId;
}
