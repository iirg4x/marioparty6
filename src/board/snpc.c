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
#define SNPC_EFFECT_DATA_MODEL DATANUM(DATA_board, 92)

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

static const int snpcDiceMotTimeTbl[2] = { 25, 25 };
static const float snpcRotSpeedTbl[2] = { 1.3f, 1.8f };
static const float snpcPosFixSpeedTbl[2] = { 1.3f, 1.8f };

static const int snpcSeTbl[8][2] = {
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
static void SNpcDiceMotHook(void);
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
static void SNpcDiceExec(int diceValue, int diceNum);

extern void *mbMallocNum(s32 size, u32 num);
extern void *mbMalloc(s32 size);
extern void mbMtxRot(Mtx mtx, float x, float y, float z);
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
extern float mbSinDeg(float deg);
extern const float lbl_802C328C;
extern const float lbl_802C3290;
extern const float lbl_802C3294;
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
extern const float lbl_802C3358;
extern const float lbl_802C3378;
extern const float lbl_802C336C;
extern const float lbl_802C33E0;
extern const float lbl_802C33E4;
extern const float lbl_802C33E8;
extern const float lbl_802C32EC;
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
    if (snpcMagic == SNPC_MAGIC) {
        mbPlayerEndTurnHookSet(GW_PLAYER_MAX - 1, NULL);
        SNpcObjKill();
        HuMemDirectFree(snpcWork);
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

static void SNpcTargetAngleSet(float angle)
{
    OMOBJ *obj;

    obj = omAddObjEx(mbObjMan, SNPC_MOVE_OBJ_PRIORITY, 0, 0, OM_GRP_NONE,
        SNpcRotateUpdate);
    snpcWork->rotateObj = obj;
    omObjGetWork(obj, SNPCROTATEWORK)->targetAngle = angle;
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
    SNPCZOOMWORK *work;

    if (!SNpcZoomCheck()) {
        snpcWork->zoomObj = omAddObjEx(mbObjMan, SNPC_MOVE_OBJ_PRIORITY, 0,
            0, OM_GRP_NONE, SNpcZoomUpdate);
    }
    work = omObjGetWork(snpcWork->zoomObj, SNPCZOOMWORK);
    work->initF = FALSE;
    work->killF = FALSE;
    work->targetZoom = zoom;
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

static u16 SNpcDiceBtnHook(int playerNo)
{
    return PAD_BUTTON_A;
}

static void SNpcDiceMotHook(void)
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
    BOOL endF;

    for (;;) {
        endF = FALSE;
        if (mbObjMotionEndCheck(snpcWork->objId[0])
            && mbObjMotionShiftIDGet(snpcWork->objId[0]) == -1) {
            endF = TRUE;
        }
        if (!endF) {
            HuPrcVSleep();
        } else {
            break;
        }
    }
}

static void SNpcObjMasuSet(int masuId)
{
    HuVecF pos;

    mbMasuPosGet(masuId, &pos);
    pos.y += lbl_802C3318;
    mbObjPosSetV(snpcWork->objId[2], &pos);
    mbObjDispSet(snpcWork->objId[2], TRUE);
    mbObjMotionTimeSet(snpcWork->objId[2], 0.0f);
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
