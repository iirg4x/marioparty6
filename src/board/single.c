#include "dolphin/math.h"
#include "datadir_enum.h"
#include "game/board/masu.h"
#include "game/board/main.h"
#include "game/board/audio.h"
#include "game/board/coin.h"
#include "game/board/camera.h"
#include "game/board/effect.h"
#include "game/board/pause.h"
#include "game/board/player.h"
#include "game/board/object.h"
#include "game/board/status.h"
#include "game/board/window.h"
#include "game/gamework.h"
#include "game/flag.h"
#include "game/gamemes.h"
#include "game/charman.h"
#include "game/esprite.h"
#include "game/hu3d.h"
#include "game/mgdata.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "datanum/charmot.h"
#include "msm_se.h"
#include "messdir_enum.h"
#include "messnum/mg_name.h"

#include <string.h>

#define SINGLE_DATA_EFFECT_SLOT0 DATA_bsingle
#define SINGLE_DATA_EFFECT_SLOT1 DATANUM(DATA_board, 99)
#define SINGLE_DATA_EFFECT_SLOT2 DATANUM(DATA_board, 94)
#define SINGLE_DATA_EFFECT_SLOT3 DATANUM(DATA_board, 102)
#define SINGLE_MESS_KOOPA_MG_SKIP MESSNUM(MESS_BOARD_SINGLE, 7)
#define SINGLE_MESS_LAST5_INTRO MESSNUM(MESS_BOARD_SINGLE, 44)
#define SINGLE_MESS_LAST5_RULES MESSNUM(MESS_BOARD_SINGLE, 45)
#define SINGLE_PRIZE_FLAG_WORD_MASK ((1 << 5) - 1)

extern void mbExitReq(void);
extern int mbCapObjCreate(int capsuleNo, BOOL flag);
extern void mbCapObjKill(int objId);
extern OMOBJ *mbGuideCreateFlag(HuVecF *pos, s8 *motTbl, BOOL screenF,
    BOOL altMtxF, BOOL layerF);
extern void mbGuideEnd(OMOBJ *obj, BOOL endF);
extern void mbGuideMotionNextSet(OMOBJ *obj, s16 motNo);
extern void mbGuideMotionSet(OMOBJ *obj, s16 motNo, BOOL shiftF);
extern void mbGuideMotionShiftSet(OMOBJ *obj, s16 motNo, BOOL shiftF);
extern void mbGuideMotionStop(OMOBJ *obj);
extern int mbGuideSpeakerNoGet(void);
extern void mbev_MgCallSingle(int mgType);
extern void HuMCListenerKill(void);
extern void HuMCClose(void);
extern void HuMCContextKill(s16 context);
extern s32 HuMCMicGet(void);
extern s32 HuMCProbe(s32 channel);
extern s32 HuMCInit(s16 mountResult);
extern s16 HuMCContextCreate(char *path);
extern void mbSingleSaveFlush(int value);
extern int mbCoinAddExec(int playerNo, int coinNum);
extern BOOL mbWipeSpecialStatGet(void);
extern void mbWipeSpecialCreate(int state, int type, int time);
extern void mbWipeSpecialFadeInCreate(int type, int time);
extern void mbWipeSpecialFadeOutCreate(int type, int time);
extern void mbWipeSpecialWait(void);
extern void mbWipeFadeOutTime(int time);
extern void mbWipeWhiteFadeOutTime(int time);
extern void mbWipeSpecialKill(void);
extern BOOL mbMgCallSingleOnCheck(u16 ovl);
extern BOOL mbSaveNewF;

typedef struct SingleSaveWork_s {
    u8 miniKoopaWinFlags;
    u8 mgEndCount;
    u8 micUseCount;
    s8 micResult;
    u8 micFirstSuccess;
    u8 micSuccessCount;
    u8 mgPlayCount;
    u8 mgEvenCount;
    u8 mgOddCount;
    u16 mgValueTotal;
    u8 mgHistory[3];
    u8 mgHistoryNo;
    u8 capsulePlayCount;
    u8 selectPlayCount;
    u8 selectHistory[3];
    u8 selectHistoryNo;
    u8 capsuleOtherF;
    u8 capsuleTwoF;
    u8 killerPlayCount;
    u8 masuTypeCount[13];
} SINGLE_SAVE_WORK;

typedef struct SingleMicResponse_s {
    s16 status;
    u16 score;
    s16 resultCount;
    s16 *result;
} SINGLE_MIC_RESPONSE;

typedef struct SingleEffData_s {
    int active;
    BOOL unk04;
    BOOL unk08;
    s16 masuType;
    s16 effNo;
    s16 state;
    HU3D_MODELID modelId;
    HU3D_MODELID childModelId[2];
    HuVecF pos;
    HuVecF targetPos;
    float unk30;
    float unk34;
    float unk38;
    HuVecF scale;
    float unk48;
    float unk4C;
    OMOBJ *obj;
    BOOL unk54;
    float unk58;
    float unk5C;
    s16 timer;
    s16 timerMax;
    s32 seId;
} SINGLE_EFF_DATA;

static u32 singleMgUnlock[4];
static u32 mgUnlockOld[4];
static ANIMDATA *singleEffAnim[4];

static int singleTeamChar = -1;
static int mgKoopaCapsuleTbl[] = { 2, 7 };
static s8 guideLast5MotTbl[] = { 12, 6, -1 };

enum {
    SINGLE_EFFECT_OBJ_PRIORITY = 8204,
};

static int singleBoard;
static int singleCancelF;
static int singleEndF;
static s16 singleMicContext;
static int singleMicF;
static int singleListenerCreateF;
static int singleListenerOnF;
static int singleMasuOrderNum;
static u8 singleMasuOrder[256][2];
static u8 masuType[5];
static u8 masuTypeNum;
static int returnMode;
static int mgRareSeNo;
static int miniKoopaMgType;
static int miniKoopaType;
static SINGLE_SAVE_WORK singleSaveWork;
static u32 singleBoardFlagOld[6];
static u32 singleMgRecordOld[GW_RECORD_MAX];
static u32 singleMgRecordPrize[GW_RECORD_MAX];
static SINGLE_EFF_DATA singleEffData[5];

static void SingleMicKill(void);
static void SingleMicListenerKill(void);
static void SingleEffClose(void);
static s16 SingleEffCreate(HuVecF *pos, int masuType);
static void SingleEffKill(s16 effNo);
static void SingleMasuTypeReset(void);
static void SingleMasuOrderSet(void);
static void SingleMgRecordBackup(void);
static void SingleMgRecordRestore(void);
static void SingleMgRecordPrizeInit(void);
static void SingleMgRecordPrizeSet(void);
static void SingleMicCreate(void);
static void SingleMicListenerCreate(void);
static void SingleMicListener(u16 *response);
static void SingleEffInit(void);
static void SingleEffOMExec(OMOBJ *obj);
static void SingleEffMgMasuHook(HU3D_MODEL *model, MBPARTICLE *particle, Mtx mtx);
static void SingleEffMgHook(HU3D_MODEL *model, MBPARTICLE *particle, Mtx mtx);
static void SingleEffMgExplodeHook(HU3D_MODEL *model, MBPARTICLE *particle, Mtx mtx);
static void SingleEffMgCapsuleHook(HU3D_MODEL *model, MBPARTICLE *particle, Mtx mtx);
static void SingleEffMgFireHook(HU3D_MODEL *model, MBPARTICLE *particle, Mtx mtx);
static void SingleEffMgFire2Hook(HU3D_MODEL *model, MBPARTICLE *particle, Mtx mtx);
static void SingleEffMgStop(s16 effNo, int type);
static void SingleMasuOrderInit(void);
static void SingleMgSaveInit(void);
static void SingleFlagFlush(void);
static void SingleLast5(void);
static void ev_SingleMg(int playerNo, s16 masuId);
static void ev_SingleRareMg(int playerNo, s16 effNo);
static void ev_SingleKoopaMg(int playerNo, s16 masuId);
static void ev_SingleMKoopaMg(int playerNo, s16 masuId);
static void ev_SingleMgEnd(int playerNo);
static void ev_SingleKoopaMgEnd(int playerNo);
static void ev_SingleMKoopaMgEnd(int playerNo);
static void ev_SingleKoopaMgSkip(MBMODELID modelId);

extern void HuMCListenerCreate(
    s16 context, void (*callback)(u16 *response), u8 property);
extern float mbSinDeg(float angle);
extern float mbCosDeg(float angle);
extern void mbMtxRot(Mtx mtx, float x, float y, float z);
extern void mbPos3Dto2D(HuVecF *src, HuVecF *dst);
extern void mbPos2Dto3D(HuVecF *src, HuVecF *dst);

void mbSingleMgUnlockInit(void);
void mbSinglePrizeFlagReset(int flag);
void mbSingleTeamCharSet(int character);
int mbSingleTeamCharGet(void);
int mbSingleCall(int mode, int arg);
void mbSingleReturn(void);
void mbSingleReturn(void);

void mbSingleInit(void)
{
    static int effFile[] = {
        SINGLE_DATA_EFFECT_SLOT0,
        SINGLE_DATA_EFFECT_SLOT1,
        SINGLE_DATA_EFFECT_SLOT2,
        SINGLE_DATA_EFFECT_SLOT3,
    };
    static int boardNo[] = {
        GW_BOARD_S01,
        GW_BOARD_S02,
        GW_BOARD_S03,
        GW_BOARD_W11,
    };
    s16 list[12];
    int listNum;
    int i;

    singleMicF = FALSE;
    singleListenerCreateF = FALSE;
    singleListenerOnF = FALSE;
    singleMicContext = -1;
    SingleMicCreate();
    for (i = 0; i < 4; i++) {
        if (boardNo[i] == MBBoardNoGet()) {
            break;
        }
    }
    singleBoard = i;
    if (singleTeamChar < 0) {
        mbSingleTeamCharSet(7);
    }
    if (GwPlayer[GwSystem.turnPlayerNo].charNo == mbSingleTeamCharGet()) {
        if (GwPlayer[GwSystem.turnPlayerNo].charNo != 7) {
            mbSingleTeamCharSet(7);
        } else {
            mbSingleTeamCharSet(10);
        }
    }
    listNum = mbMasuTypeListGet(9, list);
    if (mbSaveNewF) {
        for (i = 0; i < 4; i++) {
            mbPlayerCoinSet(i, 0);
        }
        mbSingleMgUnlockInit();
        SingleMasuTypeReset();
        SingleMgRecordBackup();
        for (i = 0; i < listNum; i++) {
            mbMasuCapsuleSet(list[i], i);
        }
    }
    for (i = 0; i < listNum; i++) {
        mbMasuTypeSet(list[i], mbMasuCapsuleGet(list[i]) + 9);
    }
    if (!_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        if (mbSaveNewF) {
            SingleMasuOrderInit();
        }
        SingleMasuOrderSet();
    }
    for (i = 0; i < 4; i++) {
        singleEffAnim[i] = HuSprAnimDataRead(mbBoardDataNumGet(effFile[i]));
        HuSprAnimLock(singleEffAnim[i]);
    }
    SingleEffInit();
    SingleMgSaveInit();
    singleEndF = FALSE;
    singleCancelF = FALSE;
    HuDataDirClose(DATA_bsingle);
}

void mbSingleClose(void)
{
    int playerNo = GwSystem.turnPlayerNo;
    int i;

    SingleEffClose();
    for (i = 0; i < 4; i++) {
        HuSprAnimKill(singleEffAnim[i]);
        singleEffAnim[i] = NULL;
    }
    if (GwSystem.turnNo > GwSystem.turnMax) {
        singleEndF = TRUE;
    }
    if (singleEndF) {
        if (!singleCancelF) {
            mbSingleSaveFlush(TRUE);
        } else {
            mbSingleSaveFlush(-1);
        }
    }
    SingleMicKill();
}

void mbSingleSaveInit(int teamChar, int mgPack, int storyComDif)
{
    int i;

    GWPartySet(FALSE);
    GwSystem.tagF = FALSE;
    GwSystem.storyComDif = storyComDif;
    GWBonusStarSet(FALSE);
    GwSystem.mgPack = mgPack;
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        GwPlayer[i].handicap = 0;
    }
    GwSystem.turnMax = 50;
    memset(&GwPlayer[0], 0, GW_PLAYER_MAX * sizeof(GW_PLAYER));
    singleTeamChar = teamChar;
    _ClearFlag(0);
    _ClearFlag(1);
    _ClearFlag(2);
    _SetFlag(FLAG_BOARD_INIT);
    _ClearFlag(FLAG_BOARD_TUTORIAL);
    _SetFlag(5);
    _ClearFlag(FLAG_INST_DECA);
    _SetFlag(FLAGNUM(FLAG_GROUP_COMMON, 13));
}

static void SingleMicCreate(void)
{
    if (HuMCMicGet() == TRUE && HuMCProbe(TRUE) == FALSE && !singleMicF) {
        HuMCInit(FALSE);
        singleMicF = TRUE;
        singleMicContext = HuMCContextCreate("/mic/ctx/030_single_voice");
        if (singleListenerOnF) {
            SingleMicListenerCreate();
        }
    }
}

static void SingleMicKill(void)
{
    if (singleMicF) {
        SingleMicListenerKill();

        if (singleMicContext >= 0) {
            HuMCContextKill(singleMicContext);
            singleMicContext = -1;
        }

        HuMCClose();
        singleMicF = FALSE;
    }
}

static void SingleMicListenerCreate(void)
{
    if (!singleMicF || singleListenerCreateF) {
        return;
    }
    HuMCListenerCreate(singleMicContext, SingleMicListener, FALSE);
    singleListenerCreateF = TRUE;
}

static void SingleMicListenerKill(void)
{
    if (!singleMicF || !singleListenerCreateF) {
        return;
    } else {
        HuMCListenerKill();
        singleListenerCreateF = FALSE;
    }
}

static void SingleMasuOrderInit(void)
{
    static int masuNum[][3] = {
        { 1, 1, 5 },
        { 3, 9, 3 },
        { 11, 4, 4 },
    };
    static int masuType[] = { 1, 2, 4 };
    s16 list[256];
    int listNum;
    int i;
    int j;
    int listNo;
    int no1;
    int no2;
    s16 temp;

    listNum = mbMasuTypeListGet(1, list);
    listNum += mbMasuTypeListGet(2, &list[listNum]);
    listNum += mbMasuTypeListGet(4, &list[listNum]);
    for (i = 0; i < 100; i++) {
        no1 = mbRandMod(listNum);
        no2 = mbRandMod(listNum);
        temp = list[no1];
        list[no1] = list[no2];
        list[no2] = temp;
    }
    singleMasuOrderNum = 0;
    listNo = 0;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < masuNum[singleBoard][i]; j++) {
            singleMasuOrder[singleMasuOrderNum][1] = masuType[i];
            singleMasuOrder[singleMasuOrderNum][0] = list[listNo++];
            singleMasuOrderNum++;
        }
    }
}

static void SingleMasuOrderSet(void)
{
    int i;

    for (i = 0; i < singleMasuOrderNum; i++) {
        mbMasuTypeSet(singleMasuOrder[i][0], singleMasuOrder[i][1]);
    }
}

void mbSingleMgUnlockInit(void)
{
    memset(singleMgUnlock, 0, sizeof(singleMgUnlock));
}

void mbSingleMgUnlockWrite(void)
{
    int word;
    int bit;

    for (word = 0; word < 4; word++) {
        for (bit = 0; bit < 32; bit++) {
            if (singleMgUnlock[word] & (1 << bit)) {
                GWMgUnlockSet(GW_MGNO_BASE + (word << 5) + bit);
            }
        }
    }
}

void mbSingleMgUnlockSet(int mgNo)
{
    mgNo -= GW_MGNO_BASE;
    singleMgUnlock[mgNo >> 5] |= (1 << (mgNo % 32));
}

void mbSingleMgUnlockReset(int mgNo)
{
    mgNo -= GW_MGNO_BASE;
    singleMgUnlock[mgNo >> 5] &= ~(1 << (mgNo % 32));
}

BOOL mbSingleMgUnlockGet(int mgNo)
{
    mgNo -= GW_MGNO_BASE;
    return (singleMgUnlock[mgNo >> 5] & (1 << (mgNo % 32))) != 0;
}

BOOL mbSingleMgUnlockCheckAny(void)
{
    int word;

    for (word = 0; word < 4; word++) {
        if (singleMgUnlock[word]) {
            return TRUE;
        }
    }
    return FALSE;
}

int mbSingleMgUnlockNumGet(void)
{
    int num = 0;
    int word;
    int bit;

    for (word = 0; word < 4; word++) {
        for (bit = 0; bit < 32; bit++) {
            if (singleMgUnlock[word] & (1 << bit)) {
                num++;
            }
        }
    }
    return num;
}

static void SingleMasuTypeReset(void)
{
    masuTypeNum = 0;
    memset(masuType, 0, sizeof(masuType));
}

static inline HU3D_MODELID SingleParticleCreate(int animNo, s16 maxCount,
    MBPARTICLEHOOK hook)
{
    ANIMDATA *anim = singleEffAnim[animNo];
    HU3D_MODELID modelId;

    modelId = mbParticleCreate(anim, maxCount);
    mbParticleHookSet(modelId, hook);
    Hu3DModelCameraSet(modelId, 1);
    Hu3DModelLayerSet(modelId, 5);
    return modelId;
}

static inline MBPARTICLE *SingleParticleDataGet(HU3D_MODELID modelId)
{
    return (MBPARTICLE *)Hu3DData[modelId].hookData;
}

static void SingleEffInit(void)
{
    MBPARTICLE *particle;
    SINGLE_EFF_DATA *work = singleEffData;
    s32 i;

    memset(work, 0, sizeof(singleEffData));
    for (i = 0; i < 5; ++i, ++work) {
        work->modelId = SingleParticleCreate(0, 1, SingleEffMgMasuHook);
        mbParticleAttrSet(work->modelId, MB_PARTICLE_ATTR_3D);
        Hu3DModelAttrSet(work->modelId, HU3D_ATTR_DISPOFF | HU3D_ATTR_NOPAUSE);
        particle = SingleParticleDataGet(work->modelId);
        particle->hookData = work;

        work->childModelId[0] = SingleParticleCreate(1, 20, SingleEffMgHook);
        mbParticleAttrSet(work->childModelId[0], MB_PARTICLE_ATTR_UPAUSE);
        Hu3DModelAttrSet(work->childModelId[0], HU3D_ATTR_DISPOFF);
        mbParticleBlendModeSet((int)work->childModelId[0], MB_PARTICLE_BLEND_ADDCOL);
        particle = SingleParticleDataGet(work->childModelId[0]);
        particle->hookData = work;

        work->childModelId[1] = SingleParticleCreate(1, 100, SingleEffMgExplodeHook);
        Hu3DModelAttrSet(work->childModelId[1], HU3D_ATTR_DISPOFF);
        mbParticleBlendModeSet((int)work->childModelId[1], MB_PARTICLE_BLEND_ADDCOL);

        work->obj = omAddObjEx(mbObjMan, SINGLE_EFFECT_OBJ_PRIORITY, 0, 0, OM_GRP_NONE, SingleEffOMExec);
        work->obj->work[0] = i;
        work->seId = -1;
    }
}

static void SingleEffClose(void)
{
    int i;
    SINGLE_EFF_DATA *work = singleEffData;

    for (i = 0; i < 5; i++, work++) {
        if (work->active != 0) {
            SingleEffKill(i + 1);
        }
    }
}

static s16 SingleEffCreate(HuVecF *pos, int masuType)
{
    MBPARTICLE *particle;
    SINGLE_EFF_DATA *work = singleEffData;
    int i;

    for (i = 0; i < 5; i++, work++) {
        if (work->active == FALSE) {
            break;
        }
    }
    work->effNo = i + 1;
    work->active = TRUE;
    work->unk04 = TRUE;
    work->unk08 = TRUE;
    work->masuType = masuType;
    work->pos = *pos;
    work->unk30 = work->unk34 = work->unk38 = 0.0f;
    work->scale.x = work->scale.y = work->scale.z = 1.0f;
    work->unk5C = 1.0f;
    work->unk58 = 0.0f;
    work->unk4C = 1.0f;
    work->unk54 = TRUE;
    Hu3DModelAttrReset(work->modelId, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrReset(work->childModelId[0], HU3D_ATTR_DISPOFF);
    particle = SingleParticleDataGet(work->childModelId[0]);
    particle->mode = 0;
    Hu3DModelCameraSet(work->modelId, 1);
    Hu3DModelLayerSet(work->modelId, 5);
    for (i = 0; i < 2; i++) {
        Hu3DModelCameraSet(work->childModelId[i], 1);
        Hu3DModelLayerSet(work->childModelId[i], 5);
    }
    work->seId = -1;
    return work->effNo;
}

static void SingleEffKill(s16 effNo)
{
    SINGLE_EFF_DATA *work = &singleEffData[effNo - 1];
    int i;

    work->active = 0;
    work->unk04 = FALSE;
    Hu3DModelAttrSet(work->modelId, HU3D_ATTR_DISPOFF);
    for (i = 0; i < 2; i++) {
        Hu3DModelAttrSet(work->childModelId[i], HU3D_ATTR_DISPOFF);
    }
}

static void SingleEffMgMasuHook(HU3D_MODEL *model, MBPARTICLE *particle, Mtx mtx)
{
    static s16 masuPatNo[] = { -1, 0, 1, 2, 6, 7, 3, 5, 8, 9, 10, 11 };
    SINGLE_EFF_DATA *work = particle->hookData;
    MBPARTICLEDATA *data;
    HuVecF rot;
    Mtx transform;

    if (particle->mode == 0) {
        data = particle->data;
        data->pos.x = data->pos.y = data->pos.z = 0.0f;
        data->scale = 120.0f;
        data->time = 0;
        particle->colorIn[0] = GX_CC_TEXC;
        particle->colorIn[1] = GX_CC_ONE;
        particle->colorIn[2] = GX_CC_C0;
        particle->colorIn[3] = GX_CC_ZERO;
        particle->tevColor[0].r = particle->tevColor[0].g =
            particle->tevColor[0].b = particle->tevColor[0].a = 255;
        particle->mode = 1;
    }
    data = particle->data;
    mbCameraRotGet(&rot);
    mbMtxRot(transform, rot.x, rot.y, rot.z);
    mtxScaleCat(transform, work->scale.x, work->scale.y, work->scale.z);
    mtxTransCat(transform, work->pos.x, work->pos.y, work->pos.z);
    PSMTXConcat(mtx, transform, mtx);
    data->rot = *(HuVecF *)&work->unk30;
    data->animNo = masuPatNo[work->masuType];
    data->color.a = (u8)(255.0f * work->unk5C);
    particle->tevColor[0].r = particle->tevColor[0].g =
        particle->tevColor[0].b = (u8)(255.0f * work->unk58);
    if (!Hu3DPauseF) {
        data->pos.y = work->unk4C
            * (100.0f * (0.1f * mbSinDeg(4.0f * data->time)));
        data->time++;
    }
}

static void SingleEffMgHook(HU3D_MODEL *model, MBPARTICLE *particle, Mtx mtx)
{
    SINGLE_EFF_DATA *work;
    MBPARTICLEDATA *data;
    float angle;
    float speed;
    GXColor color = { 255, 255, 192, 192 };
    int active;
    int i;

    work = particle->hookData;
    if (particle->mode == 0) {
        data = particle->data;
        for (i = 0; i < particle->num; i++, data++) {
            angle = 360.0f * frandf();
            speed = 200.0f + (100.0f * (2.0f * frandf()));

            data->vel.x = data->vel.y = data->vel.z = 0.0f;
            data->scale = 0.0f;
            data->weight = (float)mbRandMod(360);
            data->color = color;
            data->time = mbRandMod(30) + 1;
            data->activeF = 30;
        }
        particle->mode = 1;
    }
    data = particle->data;
    active = 0;
    for (i = 0; i < particle->num; i++, data++) {
        if (data->time != 0) {
            if (--data->time == 0) {
                data->pos.x = work->pos.x + (work->scale.x * (50.0f - (100.0f * frandf())));
                data->pos.y = work->pos.y + (work->scale.y * (50.0f - (100.0f * frandf())));
                data->pos.z = work->pos.z + (work->scale.z * (50.0f - (100.0f * frandf())));
                data->scale = 30.0f + (10.0f * frandf());
            }
            active++;
        } else if (data->activeF != 0) {
            data->vel.y += (1.0f / 60.0f) * -980.0f;
            data->pos.x += (1.0f / 60.0f) * data->vel.x;
            data->pos.y += (1.0f / 60.0f) * data->vel.y;
            data->pos.z += (1.0f / 60.0f) * data->vel.z;
            data->weight += 10.0f;
            data->activeF--;
            if (data->activeF == 0) {
                if (work->unk54) {
                    data->pos.x = work->pos.x + (work->scale.x * (50.0f - (100.0f * frandf())));
                    data->pos.y = work->pos.y + (work->scale.y * (50.0f - (100.0f * frandf())));
                    data->pos.z = work->pos.z + (work->scale.z * (50.0f - (100.0f * frandf())));
                    data->vel.x = data->vel.y = data->vel.z = 0.0f;
                    data->activeF = 30;
                    data->color.a = 255;
                }
            } else if (data->activeF < 20) {
                float alpha = (float)data->activeF / 20.0f;
                data->color.a = (u8)(255.0f * alpha);
            }
            active++;
        }
    }
    if (active == 0) {
        particle->mode = 0;
        Hu3DModelAttrSet(particle->modelId, HU3D_ATTR_DISPOFF);
        work->unk08 = FALSE;
    }
}

static void SingleEffMgExplodeHook(HU3D_MODEL *model, MBPARTICLE *particle, Mtx mtx)
{
    MBPARTICLEDATA *data;
    GXColor color = { 255, 255, 192, 192 };
    int active;
    int i;

    if (particle->mode == 0) {
        data = particle->data;
        for (i = 0; i < particle->num; i++, data++) {
            float angle = 360.0f * frandf();
            float speed = 1000.0f + (100.0f * (20.0f * frandf()));

            data->pos.x = data->pos.y = data->pos.z = 0.0f;
            data->vel.x = speed * mbCosDeg(angle);
            data->vel.y = speed * mbSinDeg(angle);
            data->vel.z = 0.0f;
            data->scale = 0.0f;
            data->weight = (float)mbRandMod(360);
            data->color = color;
            data->time = mbRandMod(6) + 1;
            data->activeF = (s16)(30.0f + (30.0f * frandf()));
        }
        particle->mode = 1;
    }
    data = particle->data;
    active = 0;
    for (i = 0; i < particle->num; i++, data++) {
        if (data->time != 0) {
            if (--data->time == 0) {
                data->scale = 100.0f + (10.0f * frandf());
            }
            active++;
        } else if (data->activeF != 0) {
            data->pos.x += (1.0f / 60.0f) * data->vel.x;
            data->pos.y += (1.0f / 60.0f) * data->vel.y;
            data->pos.z += (1.0f / 60.0f) * data->vel.z;
            data->activeF--;
            if (data->activeF < 20) {
                float alpha = (float)data->activeF / 20.0f;

                data->color.a = (u8)(255.0f * alpha);
            }
            active++;
        }
    }
    if (active == 0) {
        particle->mode = 0;
        Hu3DModelAttrSet(particle->modelId, HU3D_ATTR_DISPOFF);
    }
}

static void SingleEffMgCapsuleHook(HU3D_MODEL *model, MBPARTICLE *particle, Mtx mtx)
{
    MBPARTICLEDATA *data;
    GXColor color = { 255, 255, 255, 255 };
    float alpha;
    int active;
    int i;

    if (particle->mode == 0) {
        particle->attr |= MB_PARTICLE_ATTR_UPAUSE;
        data = particle->data;
        for (i = 0; i < particle->num; i++, data++) {
            float angle = 360.0f * frandf();
            float speed = 100.0f + (100.0f * frandf());

            data->pos.x = data->pos.y = data->pos.z = 0.0f;
            data->vel.x = speed * mbCosDeg(angle);
            data->vel.y = speed * mbSinDeg(angle);
            data->vel.z = 0.0f;
            data->scale = 100.0f + (10.0f * frandf());
            data->animTime = 0.0f;
            data->animSpeed = 0.5f;
            data->animNo = 0;
            data->dispF = FALSE;
            data->pauseF = TRUE;
            data->time = mbRandMod(6) + 1;
            data->activeF = (s16)(30.0f + (30.0f * frandf()));
        }
        particle->mode = 1;
    }
    data = particle->data;
    active = 0;
    for (i = 0; i < particle->num; i++, data++) {
        if (data->time != 0) {
            if (--data->time == 0) {
                data->dispF = TRUE;
                data->pauseF = FALSE;
            }
            active++;
        } else if (!data->pauseF) {
            data->pos.x += (1.0f / 60.0f) * data->vel.x;
            data->pos.y += (1.0f / 60.0f) * data->vel.y;
            data->pos.z += (1.0f / 60.0f) * data->vel.z;
            data->activeF--;
            if (data->activeF < 20) {
                alpha = (float)data->activeF / 20.0f;
                data->color.a = (u8)(255.0f * alpha);
            }
            active++;
        }
    }
    if (active == 0) {
        particle->mode = 0;
        Hu3DModelAttrSet(particle->modelId, HU3D_ATTR_DISPOFF);
    }
}

static void SingleEffMgFireHook(HU3D_MODEL *model, MBPARTICLE *particle, Mtx mtx)
{
    HuVecF center;
    HuVecF delta;
    MBPARTICLEDATA *data;
    float speed;
    float angle;
    float scale;
    int activeNum;
    int i;

    if (particle->mode == 0) {
        data = particle->data;
        for (i = 0; i < particle->num; i++, data++) {
            data->scale = 0.0f;
            data->color.a = 0;
            data->time = (s16)-(i >> 3);
        }
        particle->mode = 1;
    }
    center.x = 0.0f;
    center.y = 0.0f;
    center.z = 0.0f;

    scale = 1.0f;
    activeNum = 0;
    data = particle->data;
    for (i = 0; i < particle->num; i++, data++) {
        if (data->dispF) {
            if (data->time < 0) {
                data->time++;
                activeNum++;
                continue;
            }
            if (data->time == 0) {
                data->time++;
                data->rot.z = 360.0f * frandf();
                data->rot.x = 360.0f * frandf();
                data->rot.y = 360.0f * frandf();
                data->rot.z = 360.0f * frandf();
                data->color.r = data->color.g = data->color.b =
                    (u8)(160.0f + 95.0f * frandf());
                angle = 360.0f * frandf();
                data->pos.x = mbSinDeg(angle);
                data->pos.z = mbCosDeg(angle);
                angle = 1.0f - mbSinDeg(90.0f * frandf());
                if (mbParticleSRandF() < 0.0f) {
                    angle *= -1.0f;
                }
                data->pos.y = angle;
                angle = 1.0f - (angle * angle);
                if (angle <= 0.0f)
                    angle = 0.0f;
                else
                    angle = sqrtf(1.0f + (angle * angle));
                (data->pos.x) *= angle;
                (data->pos.z) *= angle;
                angle = scale * (2.5000002f + (100.0f * (0.016666668f
                    * (1.5f * frandf()))));
                data->vel.x = data->pos.x * angle;
                data->vel.z = data->pos.z * angle;
                data->vel.y = (data->pos.y * angle);
                data->activeF = 20;
                PSVECScale(&data->pos, &data->pos, 30.000002f * scale);
                if (data->pos.y > 0.0f) {
                    data->pos.y *= 1.0f + (3.0f * frandf());
                }
                PSVECAdd(&center, &data->pos, &data->pos);
                data->accel.x = 100.0f + (80.0f * frandf());
                data->accel.y = scale;
                data->color.a = 0;
                data->scale = scale * (30.0f + (30.0f * frandf()));
                data->animBank = ((s16)(1000.0f * frandf())) & 3;
                angle = 360.0f * frandf();
                speed = scale * (3.3333335f + (100.0f
                    * (0.016666668f * (2.0f * frandf()))));
                data->speedDecay += speed * mbSinDeg(angle);
                data->scaleBase += speed * mbCosDeg(angle);
            }

            PSVECAdd(&data->pos, &data->vel, &data->pos);
            data->vel.y += 0.8166668f * data->accel.y;
            data->vel.x *= 0.95f;
            data->vel.z *= 0.95f;
            if (data->pos.y > center.y + (50.0f * data->accel.y)) {
                PSVECSubtract(&center, &data->pos, &delta);
                data->vel.x += 0.7f * (0.016666668f * delta.x);
                data->vel.z += 0.7f * (0.016666668f * delta.z);
                if (data->pos.y > center.y + (90.0f * data->accel.y)) {
                    data->vel.x += 1.3f * (0.016666668f * data->speedDecay);
                    data->vel.z += 1.3f * (0.016666668f * data->scaleBase);
                }
            }

            data->activeF--;
            if (data->activeF <= 0) {
                if (particle->count < 30) {
                    data->time = 0;
                    data->scale = 0.0f;
                    data->color.a = 0;
                } else {
                    data->dispF = FALSE;
                }
            } else if (data->activeF < 8) {
                if (data->color.a >= 30) {
                    data->color.a -= 30;
                }
                data->scale += 5.0f;
            } else {
                data->color.a = (u8)(data->color.a
                    + (0.3f * (data->accel.x - data->color.a)));
                data->scale += 2.0f;
            }
            activeNum++;
        }
    }
    if (activeNum == 0) {
        particle->mode = 0;
        Hu3DModelAttrSet(particle->modelId, HU3D_ATTR_DISPOFF);
    }
}

static void SingleEffMgFire2Hook(HU3D_MODEL *model, MBPARTICLE *particle, Mtx mtx)
{
    HuVecF center;
    HuVecF delta;
    MBPARTICLEDATA *data;
    float unit;
    float angle;
    int activeNum;
    int i;

    if (particle->count == 0) {
        data = particle->data;
        for (i = 0; i < particle->num; i++, data++) {
            data->scale = 0.0f;
            data->color.a = 0;
            data->time = (s16)-(i >> 2);
        }
        particle->count = 1;
        particle->blendMode = MB_PARTICLE_BLEND_ADDCOL;
    }

    center.x = 0.0f;
    center.y = 0.0f;
    center.z = 0.0f;
    unit = 1.0f;
    activeNum = 0;
    data = particle->data;
    for (i = 0; i < particle->num; i++, data++) {
        if (data->dispF) {
            if (data->time < 0) {
                data->time++;
                activeNum++;
                continue;
            }
            if (data->time == 0) {
                data->time++;
                data->rot.z = 360.0f * frandf();
                data->rot.x = 360.0f * frandf();
                data->rot.y = 360.0f * frandf();
                data->rot.z = 360.0f * frandf();
                angle = frandf();
                data->color.r = data->color.g = data->color.b = 255;
                data->color.r = (u8)(128.0f + (127.0f * frandf()));
                angle = 360.0f * frandf();
                data->pos.x = mbSinDeg(angle);
                data->pos.z = mbCosDeg(angle);
                angle = 1.0f - mbSinDeg(90.0f * frandf());
                if (mbParticleSRandF() < 0.0f) {
                    angle *= -1.0f;
                }
                data->pos.y = angle;
                angle = 1.0f - (angle * angle);
                if (angle <= 0.0f) {
                    angle = 0.0f;
                } else {
                    angle = sqrtf(1.0f + (angle * angle));
                }
                data->pos.x *= angle;
                data->pos.z *= angle;
                angle = unit * (1.3333335f + (100.0f * (0.016666668f
                    * (1.2f * frandf()))));
                data->vel.x = data->pos.x * angle;
                data->vel.z = data->pos.z * angle;
                data->vel.y = data->pos.y * angle;
                data->activeF = 16;
                PSVECScale(&data->pos, &data->pos, 30.000002f * unit);
                if (data->pos.y > 0.0f) {
                    data->pos.y *= 1.0f + (2.0f * frandf());
                }
                PSVECAdd(&center, &data->pos, &data->pos);
                data->accel.x = unit * (50.0f + (70.0f * frandf()));
                data->accel.y = unit;
                data->color.a = 0;
                data->scale = unit * (40.0f + (60.0f * frandf()));
                data->animBank = (s16)(((int)(1000.0f * frandf())) & 3);
            }

            PSVECAdd(&data->pos, &data->vel, &data->pos);
            data->vel.y += 1.088889f * data->accel.y;
            data->vel.x *= 0.95f;
            data->vel.z *= 0.95f;
            if (data->pos.y > center.y + (50.0f * data->accel.y)) {
                PSVECSubtract(&center, &data->pos, &delta);
                data->vel.x += 0.7f * (0.016666668f * delta.x);
                data->vel.z += 0.7f * (0.016666668f * delta.z);
            }

            data->activeF--;
            if (data->activeF <= 0) {
                if (particle->count < 30) {
                    data->time = 0;
                    data->scale = 0.0f;
                    data->color.a = 0;
                } else {
                    data->dispF = FALSE;
                }
            } else if (data->activeF < 8) {
                if (data->color.a >= 30) {
                    data->color.a -= 30;
                }
                data->scale += 5.0f;
            } else {
                data->color.a = (u8)(data->color.a
                    + (0.3f * (data->accel.x - data->color.a)));
                data->scale += 2.0f;
            }
            activeNum++;
        }
    }
    if (activeNum == 0) {
        particle->mode = 0;
        Hu3DModelAttrSet(particle->modelId, HU3D_ATTR_DISPOFF);
    }
}

static void SingleEffOMExec(OMOBJ *obj)
{
    SINGLE_EFF_DATA *work;
    HuVecF pos2d;
    HuVecF pos3d;
    float angle;
    float phase;
    float reverse;

    work = &singleEffData[obj->work[0]];
    if (mbExitCheck()) {
        omDelObjEx(HuPrcCurrentGet(), obj);
        return;
    }
    if (work->active == 0) {
        return;
    }
    if (work->unk04) {
        Hu3DModelAttrReset(work->modelId, HU3D_ATTR_DISPOFF);
        if (work->unk08) {
            Hu3DModelAttrReset(work->childModelId[0], HU3D_ATTR_DISPOFF);
        }
    } else {
        Hu3DModelAttrSet(work->modelId, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(work->childModelId[0], HU3D_ATTR_DISPOFF);
    }

    switch (work->state) {
    case 0:
        break;

    case 1:
        phase = (float)(work->timer++) / (float)work->timerMax;
        work->scale.y = phase;
        work->scale.x = phase;
        work->unk4C = phase;
        work->pos.y = work->targetPos.y
            + (100.0f * (2.0f * mbSinDeg(90.0f * phase)));
        if (work->timer > work->timerMax) {
            work->state = 0;
        }
        break;

    case 2:
        phase = (float)(work->timer++) / (float)work->timerMax;
        reverse = 1.0f - phase;
        work->unk48 += 0.2f;
        work->scale.x = work->scale.y = mbCosDeg(90.0f * phase);
        work->unk4C = reverse;
        work->pos.y = work->targetPos.y
            + (100.0f * (2.0f * mbSinDeg(90.0f * reverse)));
        if (work->timer > work->timerMax) {
            work->state = 0;
        }
        break;

    case 3:
        phase = (float)(work->timer++) / (float)work->timerMax;
        work->unk48 += 0.2f;
        work->unk58 = phase;
        work->unk4C = 1.0f - phase;
        mbPos3Dto2D(&work->targetPos, &pos2d);
        pos2d.x = 114.0f;
        pos2d.y = 80.0f;
        mbPos2Dto3D(&pos2d, &pos3d);
        work->pos.x = work->targetPos.x
            + (phase * (pos3d.x - work->targetPos.x));
        work->pos.y = work->targetPos.y
            + (phase * (pos3d.y - work->targetPos.y));
        work->pos.z = work->targetPos.z
            + (phase * (pos3d.z - work->targetPos.z));
        if ((u32)work->timer == work->timerMax - 12) {
            work->unk54 = FALSE;
        }
        if (work->timer > work->timerMax) {
            SingleEffMgStop(work->effNo, 0);
        }
        break;

    case 4:
        phase = (float)(work->timer++) / (float)work->timerMax;
        work->scale.x = work->scale.y =
            1.0f + (4.0f * mbSinDeg(90.0f * phase));
        work->unk5C = 1.0f - phase;
        if (work->timer > work->timerMax) {
            work->state = 0;
            Hu3DModelAttrSet(work->modelId, HU3D_ATTR_DISPOFF);
            mbParticleBlendModeSet((int)work->modelId, MB_PARTICLE_BLEND_NORMAL);
        }
        break;

    case 5:
        phase = (float)(work->timer++) / (float)work->timerMax;
        work->scale.x = work->scale.y =
            1.0f + (1.5f * mbSinDeg(90.0f * phase)) +
            (0.5f * mbSinDeg(1440.0f * phase));
        work->unk4C = 1.0f - phase;
        work->unk48 += 0.4f;
        work->unk58 = phase;
        if ((u32)work->timer == work->timerMax - 12) {
            work->unk54 = FALSE;
        }
        if (work->timer > work->timerMax) {
            SingleEffMgStop(work->effNo, 1);
            if (work->seId >= 0) {
                mbAudFXStop(work->seId);
                work->seId = -1;
            }
        }
        break;
    }

    work->unk34 += work->unk48;
}

static void SingleEffMgStop(s16 effNo, int type)
{
    SINGLE_EFF_DATA *work;
    int i;

    work = &singleEffData[effNo - 1];
    work->state = 4;
    work->unk34 = 0.0f;
    work->unk48 = 0.0f;
    work->timer = 0;
    work->timerMax = 30;
    Hu3DModelAttrReset(work->childModelId[1], HU3D_ATTR_DISPOFF);
    Hu3DModelPosSetV(work->childModelId[1], &work->pos);
    Hu3DModelCameraSet(work->modelId, 2);
    Hu3DModelLayerSet(work->modelId, 7);
    for (i = 0; i < 2; i++) {
        Hu3DModelCameraSet(work->childModelId[i], 2);
        Hu3DModelLayerSet(work->childModelId[i], 7);
    }
    if (type == 0) {
        mbAudFXPlay(MSM_SE_SBRD_03);
        omVibrate(GwSystem.turnPlayerNo, 20, 7, 3);
    } else {
        mbAudFXPlay(MSM_SE_SBRD_05);
        omVibrate(GwSystem.turnPlayerNo, 20, 20, 0);
    }
}

void mbev_SingleMg(int playerNo, s16 masuId)
{
    int masuType;
    int i;

    mbCameraPlayerViewSet(playerNo, 0);
    mbCameraMoveWait();
    masuType = mbMasuTypeGet(masuId);
    mbPlayerRotateStart(playerNo, 0, 15);
    while (!mbPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    for (i = 0; i < 4; i++) {
        mgUnlockOld[i] = GwCommon.mgUnlock[i];
    }
    SingleMgRecordPrizeInit();
    switch (masuType) {
    case 6:
        ev_SingleKoopaMg(playerNo, masuId);
        break;
    case 9:
    case 10:
    case 11:
        ev_SingleMKoopaMg(playerNo, masuId);
        break;
    default:
        ev_SingleMg(playerNo, masuId);
        break;
    }
}

int mbev_SingleMgEnd(int playerNo)
{
    int mgNo = GwSystem.mgNo;

    if ((mgUnlockOld[mgNo >> 5] & (1 << (mgNo % 32))) == 0) {
        GwCommon.mgUnlock[mgNo >> 5] &= ~(1 << (mgNo % 32));
    }
    mbPlayerColSnapSet(TRUE);
    mbSingleCall(8, 0);
    if (_CheckFlag(FLAG_BOARD_MG)) {
        ev_SingleMgEnd(playerNo);
        _ClearFlag(FLAG_BOARD_MG);
    } else if (_CheckFlag(FLAG_BOARD_MG_KOOPA)) {
        ev_SingleKoopaMgEnd(playerNo);
        _ClearFlag(FLAG_BOARD_MG_KOOPA);
    } else if (_CheckFlag(FLAG_BOARD_MG_KETTOU)) {
        ev_SingleMKoopaMgEnd(playerNo);
        _ClearFlag(FLAG_BOARD_MG_KETTOU);
    }
    return TRUE;
}

static void ev_SingleMg(int playerNo, s16 masuId)
{
    HuVecF effectPos, playerPos;
    SINGLE_EFF_DATA *effect;
    s16 effNo;
    s32 masuType = mbMasuTypeGet(masuId);
    s32 seNo;
    s32 childIndex;
    SINGLE_EFF_DATA *endEffect;
    mbPlayerPosGet(playerNo, &playerPos);
    effectPos.x = playerPos.x;
    effectPos.y = playerPos.y + 100.0f;
    effectPos.z = playerPos.z;
    effNo = SingleEffCreate(&effectPos, masuType);
    effect = &singleEffData[effNo - 1];
    effect->state = 1;
    effect->pos = effectPos;
    effect->targetPos = effect->pos;
    effect->scale.x = effect->scale.y = 0.0f;
    effect->unk4C = 0.0f;
    effect->unk48 = 4.0f;
    effect->timer = 0;
    effect->timerMax = 60;
    if (masuType == 7) {
        mgRareSeNo = mbAudFXPlay(MSM_SE_BRD00_91);
        mbAudFXPlay(MSM_SE_BRD00_92);
    } else {
        seNo = mbAudFXPlay(MSM_SE_SBRD_01);
    }
    {
        SINGLE_EFF_DATA *waitEffect;

        while ((waitEffect = &singleEffData[effNo - 1]),
            waitEffect->state != 0) {
            HuPrcVSleep();
        }
    }

    _SetFlag(FLAG_BOARD_MG);
    switch (masuType) {
    case 1:
        mbev_MgCallSingle(0);
        break;
    case 2:
        mbev_MgCallSingle(1);
        break;
    case 4:
        mbev_MgCallSingle(2);
        break;
    case 5:
        mbev_MgCallSingle(3);
        break;
    case 7:
        ev_SingleRareMg(playerNo, effNo);
        break;
    }
    endEffect = &singleEffData[effNo - 1];
    endEffect->active = 0;
    endEffect->unk04 = FALSE;
    Hu3DModelAttrSet(endEffect->modelId, HU3D_ATTR_DISPOFF);
    for (childIndex = 0; childIndex < 2; ++childIndex) {
        Hu3DModelAttrSet(endEffect->childModelId[childIndex], HU3D_ATTR_DISPOFF);
    }
    mbAudFXStop(seNo);
}

static void ev_SingleMgEnd(int playerNo)
{
    HuVecF effectPos;
    HuVecF pos;
    s16 masuId;
    s16 winId;
    s16 effNo;
    s16 conditionCoin;
    s16 conditionBonus;
    s16 prizeBonus;
    s16 prizeCoin;
    int masuTypeNo;
    int seNo;
    u32 frame;
    int mgNo;
    BOOL unlocked;
    ANIMDATA *particleAnim;
    BOOL unlockResult;
    SINGLE_EFF_DATA *winWait;
    SINGLE_EFF_DATA *loseWait;
    SINGLE_EFF_DATA *initialEffect;

    mgNo = GwSystem.mgNo;
    seNo = mbAudFXPlay(MSM_SE_SBRD_01);
    mbAudFXVolSet(seNo, 0);
    if (!mbWipeSpecialStatGet()) {
        mbWipeFadeOut();
    }
    mbStatusDispForceSet(playerNo, TRUE);
    mbCameraMovePlayer(playerNo, NULL, NULL, 1600.0f, -1.0f, -1);
    mbCameraMoveWait();
    masuId = GwPlayer[playerNo].masuId;
    masuTypeNo = mbMasuTypeGet(masuId);
    mbPlayerRotYSet(playerNo, 0.0f);
    mbPlayerPosGet(playerNo, &pos);
    effectPos.x = pos.x;
    effectPos.y = pos.y + 300.0f;
    effectPos.z = pos.z;
    effNo = SingleEffCreate(&effectPos, masuTypeNo);
    {
        SINGLE_EFF_DATA *initialWork;

        initialEffect = &singleEffData[effNo - 1];
        initialWork = initialEffect;
        initialWork->scale.x = initialWork->scale.y = initialWork->scale.z = 1.0f;
        initialWork->unk48 = 4.0f;
        initialWork->unk4C = 1.0f;
    }
    mbMusBoardPlay();
    mbWipeFadeIn();
    mbPauseDisableSet(FALSE);
    for (frame = 0; frame < 60; frame++) {
        if (frame > 21) {
            mbAudFXVolSet(seNo, (s16)((frame - 21) * 127 / 39));
        }
        HuPrcVSleep();
    }

    if ((MgDataTbl[mgNo].type == MG_TYPE_BATTLE
            && GwPlayer[playerNo].mgCoinBonus == 0)
        || (MgDataTbl[mgNo].type != MG_TYPE_BATTLE
            && ((conditionCoin = GwPlayer[playerNo].mgCoin),
                (conditionBonus = GwPlayer[playerNo].mgCoinBonus),
                conditionBonus + conditionCoin > 0))) {
        mbSingleCall(10, MgDataTbl[mgNo].type);
        mbAudFXStop(seNo);
        {
            SINGLE_EFF_DATA *effect = &singleEffData[effNo - 1];
            int i;

            effect->state = 3;
            effect->targetPos = effect->pos;
            effect->timer = 0;
            effect->timerMax = 58;
            Hu3DModelCameraSet(effect->modelId, 2);
            Hu3DModelLayerSet(effect->modelId, 7);
            for (i = 0; i < 2; i++) {
                Hu3DModelCameraSet(effect->childModelId[i], 2);
                Hu3DModelLayerSet(effect->childModelId[i], 7);
            }
        }
        mbAudFXPlay(MSM_SE_SBRD_02);
        HuPrcSleep(60);

        if (GWMgUnlockGet(mgNo + GW_MGNO_BASE)
            || mbSingleMgUnlockGet(mgNo + GW_MGNO_BASE)) {
            unlockResult = TRUE;
        } else {
            unlockResult = FALSE;
        }
        if (!(unlocked = unlockResult)) {
            mbSingleMgUnlockSet(mgNo + GW_MGNO_BASE);
            GWSingleMgFlagSet(mgNo + GW_MGNO_BASE);
            masuType[masuTypeNum++] = (s16)masuTypeNo;
            masuTypeNum %= 5;
            mbSingleCall(9, mgNo);
        }
        SingleMgRecordPrizeSet();
        {
            while ((winWait = &singleEffData[effNo - 1]), winWait->state != 0) {
                HuPrcVSleep();
            }
        }
        HuPrcSleep(30);
        mbCameraMovePlayer(playerNo, NULL, NULL, 1800.0f, -1.0f, 60);
        if ((MgDataTbl[mgNo].flag & MG_FLAG_COIN) == 0) {
            mbCoinAddExec(playerNo, 10);
        } else {
            prizeBonus = GwPlayer[playerNo].mgCoinBonus;
            prizeCoin = GwPlayer[playerNo].mgCoin;
            mbCoinAddExec(playerNo, prizeCoin + prizeBonus);
        }
        mbPlayerMotionShiftSet(playerNo, 7, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
        mbPlayerWinLoseVoicePlay(playerNo, 7, MSM_SE_CHARVOICE_MARIO + 6);
        if (!unlocked) {
            winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 0), -1);
            mbWinInsertMesSet(winId, mbPlayerNameMesGet(playerNo), 0);
            if (mgNo != (u16)-1) {
                mbWinInsertMesSet(winId, MgDataTbl[mgNo].nameMes, 1);
            } else {
                mbWinInsertMesSet(winId, MESSNUM(MESS_BOARD_SINGLE, 39), 1);
            }
        }
        while (!mbPlayerMotionEndCheck(playerNo)) {
            HuPrcVSleep();
        }
        mbWinWait(winId);
        HuPrcSleep(60);
    } else {
        mbAudFXStop(seNo);
        {
            SINGLE_EFF_DATA *effect = &singleEffData[effNo - 1];

            effect->state = 5;
            effect->targetPos = effect->pos;
            effect->timer = 0;
            effect->timerMax = 90;
            effect->seId = mbAudFXPlay(MSM_SE_SBRD_04);
            HuPrcSleep(90);
            mbPlayerMotionShiftSet(playerNo, 9, 0.0f, 8.0f,
                HU3D_MOTATTR_NONE);
            HuPrcSleep(30);
            mbPlayerMotionShiftSet(playerNo, 6, 0.0f, 8.0f,
                HU3D_MOTATTR_LOOP);
            {
                while ((loseWait = &singleEffData[effNo - 1]),
                    loseWait->state != 0) {
                    HuPrcVSleep();
                }
            }
        }
        HuPrcSleep(120);
    }
    mbWipeFadeOut();
    {
        SINGLE_EFF_DATA *effect = &singleEffData[effNo - 1];
        int i;

        effect->active = 0;
        effect->unk04 = FALSE;
        Hu3DModelAttrSet(effect->modelId, HU3D_ATTR_DISPOFF);
        for (i = 0; i < 2; i++) {
            Hu3DModelAttrSet(effect->childModelId[i], HU3D_ATTR_DISPOFF);
        }
    }
}

static inline BOOL SingleMgUnlockedCheck(int unlockMgNo)
{
    if (GWMgUnlockGet(unlockMgNo)
        || mbSingleMgUnlockGet(unlockMgNo)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static inline int SingleMgListGet(int mgType, u8 *list)
{
    int mgNo;
    int listNum;

    listNum = 0;
    for (mgNo = 0; MgDataTbl[mgNo].ovl != (u16)-1; mgNo++) {
        if ((mgType >= 0 && MgDataTbl[mgNo].type != mgType)
            || MgDataTbl[mgNo].type == MG_TYPE_KUPA
            || MgDataTbl[mgNo].type == MG_TYPE_DONKEY
            || (!(MgDataTbl[mgNo].flag & MG_FLAG_RARE)
                && !mbMgCallSingleOnCheck(MgDataTbl[mgNo].ovl))
            || MgDataTbl[mgNo].nameMes == MG_NAME_M677) {
            continue;
        }
        if (!SingleMgUnlockedCheck(mgNo + GW_MGNO_BASE)) {
            if (list) {
                list[listNum] = mgNo;
            }
            listNum++;
        }
    }
    return listNum;
}

static void ev_SingleRareMg(int playerNo, s16 effNo)
{
    static const HuVecF cameraOfs = { 0.0f, 100.0f, 0.0f };
    static const u32 nameMes[] = {
        MG_NAME_M699,
        MG_NAME_M678,
        MG_NAME_M679,
    };
    HuVecF effectPos;
    HuVecF pos;
    HuVecF cameraRot;
    float phase;
    float rate;
    float cameraZoom;
    float zoom;
    s16 winId;
    s16 mesId[1];
    int streamNo;
    int frame;
    int mgNo;
    int rareMgResult;
    u8 rareMgList[128];

    mbPauseDisableSet(TRUE);
    frame = 0;
    while (MgDataTbl[frame].ovl != (u16)-1) {
        if (MgDataTbl[frame].nameMes == nameMes[singleBoard]) {
            break;
        }
        frame++;
    }
    mgNo = frame;
    _SetFlag(FLAG_BOARD_STAR_RESET);
    mbMusFadeOutSpeed(MB_MUS_CHAN_BG, 1000);
    mbPlayerMotionSet(playerNo, 11, HU3D_MOTATTR_NONE);
    mbPlayerPosGet(playerNo, &pos);
    {
        frame = 0;
        do {
            if ((u32)frame++ == 12) {
                effectPos = pos;
                effectPos.y += 100.0f;
                {
                    SINGLE_EFF_DATA *work = &singleEffData[effNo - 1];

                    work->state = 2;
                    work->pos = effectPos;
                    work->targetPos = work->pos;
                    work->unk54 = FALSE;
                    work->timer = 0;
                    work->timerMax = 60;
                }
            }
            if (frame == 30) {
                if (mgRareSeNo >= 0) {
                    mbAudFXStop(mgRareSeNo);
                }
                mbAudFXPlay(MSM_SE_BRD00_93);
            }
            HuPrcVSleep();
        } while (!mbPlayerMotionEndCheck(playerNo));
        mbPlayerMotIdleSet(playerNo);
        omVibrate(playerNo, 20, 7, 3);
        streamNo = mbMusJinglePlay(39);
        mbPlayerMotionShiftSet(playerNo, 7, 0.0f, 8.0f,
            HU3D_MOTATTR_NONE);
        mbPlayerWinLoseVoicePlay(playerNo, 7, MSM_SE_CHARVOICE_MARIO);
        mbCameraRotGet(&cameraRot);
        cameraRot.x -= 15.0f;
        mbCameraMovePlayer(playerNo, &cameraRot, (HuVecF *)&cameraOfs,
            1600.0f, -1.0f, 102);
        mbCameraMoveWait();
        zoom = mbCameraZoomGet();
        mbCameraFocusObjSet(-1);
        for (frame = 0; (u32)frame < 30; frame++) {
            phase = (float)frame / 30.0f;
            rate = (float)(0.5
                * (1.0 + sin(3.141592653589793
                    * (720.0f * phase) / 180.0)));
            cameraZoom = zoom + (-500.0f * rate);
            mbCameraZoomSet(cameraZoom);
            HuPrcVSleep();
        }
    }

    if (!GWMgUnlockGet(mgNo + GW_MGNO_BASE)) {
        int rareMgNum;
        int mgIndex;
        int rareMgNo;
        int rareCount;
        BOOL unlocked;
        int unlockNo;
        int i;

        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 0), -1);
        mbWinInsertMesSet(winId, mbPlayerNameMesGet(playerNo), 0);
        if (mgNo != (u16)-1) {
            mbWinInsertMesSet(winId, MgDataTbl[mgNo].nameMes, 1);
        } else {
            mbWinInsertMesSet(winId, MESSNUM(MESS_BOARD_SINGLE, 39), 1);
        }
        mgIndex = mgNo + GW_MGNO_BASE;
        mgIndex -= GW_MGNO_BASE;
        singleMgUnlock[mgIndex >> 5] |= 1 << (mgIndex % 32);
        GWSingleMgFlagSet(mgNo + GW_MGNO_BASE);
        masuType[masuTypeNum++] = 7;
        masuTypeNum %= 5;
        GWSinglePrizeFlagSet(8);

        rareCount = 0;
        for (i = 0; MgDataTbl[i].ovl != (u16)-1; i++) {
            if ((MgDataTbl[i].flag & MG_FLAG_RARE)
                && MgDataTbl[i].nameMes != MG_NAME_M677) {
                rareMgList[rareCount++] = i;
            }
        }
        rareMgResult = rareCount;
        rareMgNum = rareMgResult;
        for (frame = 0; frame < rareMgNum; frame++) {
            unlockNo = rareMgList[frame] + GW_MGNO_BASE;
            if (GWMgUnlockGet(unlockNo)
                || (rareMgNo = unlockNo,
                    rareMgNo -= GW_MGNO_BASE,
                    (singleMgUnlock[rareMgNo >> 5]
                        & (1 << (rareMgNo % 32))) != 0)) {
                unlocked = TRUE;
            } else {
                unlocked = FALSE;
            }
            if (!unlocked) {
                break;
            }
        }
        if (frame >= rareMgNum) {
            GWSinglePrizeFlagSet(46);
        }

        rareMgNum = SingleMgListGet(-1, NULL);
        if (rareMgNum == 0) {
            GWSinglePrizeFlagSet(47);
        }
    } else {
        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 1), -1);
        mbWinInsertMesSet(winId, mbPlayerNameMesGet(playerNo), 0);
        mbCoinAddProcExec(playerNo, 50, TRUE, TRUE);
    }
    mbMusJingleWait(streamNo);
    mbWinWait(winId);
    mesId[0] = GameMesCreate(6, TRUE);
    while (GameMesStatGet(mesId[0]) != 0) {
        HuPrcVSleep();
    }
    mbWipeSpecialCreate(1, 6, 90);
    mbWipeSpecialWait();
    mbWipeFadeOutTime(1);
    mbWipeSpecialKill();
    GwSystem.turnNo++;
    mbSingleReturn();
}

static inline int SingleMgUnlockListGet(u8 *list)
{
    int word;
    int bit;
    int listNum;

    listNum = 0;
    for (word = 0; word < 4; word++) {
        for (bit = 0; bit < 32; bit++) {
            if (singleMgUnlock[word] & (1u << bit)) {
                if (list) {
                    list[listNum] = bit + (word << 5);
                }
                listNum++;
            }
        }
    }
    return listNum;
}

/* Approved compatibility primitive, as used by CapSpecial: one fabs
 * instruction with a compiler-selected register and a portable fallback. */
#ifdef __MWERKS__
static inline float SingleAbsFloat(register float value)
{
    asm {
        fabs value, value
    }
    return value;
}
#else
static inline float SingleAbsFloat(float value)
{
    return (float)fabs((double)value);
}
#endif

static void ev_SingleKoopaMg(int playerNo, s16 masuId)
{
    static int guideMot[] = {
        DATANUM(DATA_capsulechar1, 1),
        DATANUM(DATA_capsulechar1, 5),
        DATANUM(DATA_capsulechar1, 3),
        DATANUM(DATA_capsulechar1, 7),
        DATANUM(DATA_capsulechar1, 4),
        DATANUM(DATA_capsulechar1, 10),
        -1,
    };
    static HuVecF defCameraRot = { -15.0f, 0.0f, 0.0f };
    static HuVecF cameraPos = { 0.0f, 100.0f, 100.0f };
    static HuVecF cameraRot = { -35.0f, 0.0f, 0.0f };
    static int mgTypeTbl[][2] = {
        { 1, 0 },
        { 2, 1 },
        { 4, 2 },
    };
MBMODELID modelId;
MBMODELID capsuleObj = 0;
    s16 winId;
    s16 masuStart;
    HuVecF tempPos;
    HuVecF capsulePos;
    HuVecF pos3d;
    HuVecF masuPos;
int frame;
int flightFrame;
    int capsuleMode;
int choice;
    s16 playerMotion[2];
    s16 spriteId[2];
    struct {
        HU3D_MODELID ids[2];
    } particleData = { { -1, -1 } };
    ANIMDATA *capsuleAnim;
    ANIMDATA *explodeAnim;
    HU3D_MODELID particleCapsule;
    HU3D_MODELID particleExplode;
    HU3D_MODELID particleCapsuleId;
    HU3D_MODELID particleCapsuleStored;
    HU3D_MODELID particleExplodeId;
    HU3D_MODELID particleExplodeStored;
    float scale, angle, remaining;
    modelId = mbObjCreate(DATA_capsulechar1, guideMot, FALSE);
    mbObjDispSet(modelId, FALSE);
    mbPlayerRotateStart(playerNo, 0, 15);
    playerMotion[0] = mbPlayerMotionCreate(playerNo, CHARMOT_HSF_c000m1_323);
    playerMotion[1] = mbPlayerMotionCreate(playerNo, CHARMOT_HSF_c000m1_325);

    spriteId[0] = espEntry(DATANUM(DATA_capsulechar1, 36), 100, 0);
    espPosSet(spriteId[0], 288.0f, 240.0f);
    espScaleSet(spriteId[0], 4.0f, 4.0f);
    espTPLvlSet(spriteId[0], 0.0f);
    espColorSet(spriteId[0], 255, 0, 0);
    espDispOff(spriteId[0]);
    espAttrSet(spriteId[0], HUSPR_ATTR_LINEAR);
    espDrawNoSet(spriteId[0], HUSPR_DRAWNO_FRONT);
    espDispOff(spriteId[0]);

    mbMusFadeOutSpeed(MB_MUS_CHAN_BG, 1000);
    while (mbMusCheck(MB_MUS_CHAN_BG)) {
        HuPrcVSleep();
    }
    while (!mbPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, playerMotion[0], 0.0f, 8.0f,
        HU3D_MOTATTR_NONE);
    mbCameraPlayerViewSet(playerNo, 0);
    mbEffFadeCreate(30, 160);
    espDispOn(spriteId[0]);
    for (frame = 0; (u32)frame <= 180; frame++) {
        angle = (float)frame / 180.0f;
        if (frame == 30) {
            mbAudFXPlay(MSM_SE_BRD00_103);
        }
        if (frame == 18 || frame == 90 || frame == 162) {
            omVibrate(playerNo, 20, 7, 3);
        }
        espTPLvlSet(spriteId[0], SingleAbsFloat(mbSinDeg(450.0f * angle)));
        HuPrcVSleep();
    }

    mbWipeCreate(WIPE_MODE_OUT, WIPE_TYPE_DISSOLVE_OUT_BLUR, 60);
    mbWipeSpecialWait();
    espDispOff(spriteId[0]);
    mbEffFadeOutSet(30);

    masuStart = mbMasuFind_AttrIdGet(MASU_NULL, MASU_FLAG_START);
    mbMasuPosGet(masuStart, &masuPos);
    tempPos = masuPos;
    tempPos.y += 150.0f;
    mbCameraMovePos(&tempPos, &defCameraRot, NULL, 500.0f, -1.0f, -1);
    mbCameraMoveWait();
    mbObjPosSetV(modelId, &masuPos);
    mbObjDispSet(modelId, TRUE);
    mbObjMotionSet(modelId, 1, HU3D_MOTATTR_LOOP);
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
    mbPlayerPosSet(playerNo, masuPos.x, masuPos.y, masuPos.z + 300.0f);
    mbPlayerMotionSet(playerNo, 1, HU3D_MOTATTR_LOOP);
    mbPlayerRotYSet(playerNo, 180.0f);
    mbWipeFadeIn();
    HuPrcSleep(30);

    mbCameraMoveMasu(masuStart, &cameraRot, &cameraPos, 1600.0f, -1.0f, 60);
    mbMusPlay(MB_MUS_CHAN_BG, 27, MSM_VOL_MAX, 0);
    mbAudFXDelaySet(30);
    mbAudFXPlay(MSM_SE_GUIDE_47);
    mbObjMotionShiftSet(modelId, 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbCameraMoveWait();
    winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 2), 13);
    mbWinWait(winId);
    mbObjMotionShiftSet(modelId, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 3), 13);
    mbWinWait(winId);
    mbAudFXDelaySet(30);
    mbAudFXPlay(MSM_SE_GUIDE_49);
    mbObjMotionShiftSet(modelId, 5, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
    while (!mbObjMotionEndCheck(modelId)) {
        HuPrcVSleep();
    }

    if (!SingleMgUnlockListGet(NULL) && mbPlayerCoinGet(playerNo) == 0
        && mbPlayerCapsuleNumGet(playerNo) == 0) {
        capsuleMode = 0;
    } else {
        capsuleMode = 1;
    }

    spriteId[1] = espEntry(mbBoardDataNumGet(DATANUM(DATA_board, 142)),
        100, (s16)(capsuleMode ^ 1));
    espDrawNoSet(spriteId[1], 32);
    for (frame = 1; (u32)frame < 60; frame++) {
        angle = (float)frame / 60.0f;
        if ((u32)frame == 27) {
            mbAudFXPlay(MSM_SE_BRD00_113);
        }
        scale = mbSinDeg(90.0f * angle);
        espPosSet(spriteId[1], 288.0f,
            240.0f - (250.0f * mbSinDeg(180.0f * angle))
                + (50.0f * mbCosDeg(180.0f * angle)));
        espScaleSet(spriteId[1], scale, scale);
        espZRotSet(spriteId[1], 3.0f * (360.0f * angle));
        HuPrcVSleep();
    }
    omVibrate(playerNo, 20, 7, 3);
    for (frame = 1; (u32)frame < 60; frame++) {
        angle = (float)frame / 60.0f;
        scale = 1.0f + (0.2f * mbSinDeg(720.0f * angle));
        espPosSet(spriteId[1], 288.0f, 240.0f);
        espScaleSet(spriteId[1], scale, scale);
        espZRotSet(spriteId[1], 0.0f);
        HuPrcVSleep();
    }
    for (frame = 1; (u32)frame < 18; frame++) {
        angle = (float)frame / 18.0f;
        scale = 1.0f + (5.0f * mbSinDeg(90.0f * angle));
        espPosSet(spriteId[1], 288.0f, 240.0f);
        espScaleSet(spriteId[1], scale, scale);
        espTPLvlSet(spriteId[1], 1.0f - mbSinDeg(90.0f * angle));
        HuPrcVSleep();
    }
    espDispOff(spriteId[1]);
    mbAudFXDelaySet(30);
    mbAudFXPlay(MSM_SE_GUIDE_47);
    mbObjMotionShiftSet(modelId, 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);

    if (capsuleMode == 0) {
        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 4), 13);
        mbWinInsertMesSet(winId, mbPlayerNameMesGet(playerNo), 0);
        mbWinWait(winId);
        choice = mbRandMod(100) < 50 ? 0 : 1;
        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 5), 13);
        mbWinInsertMesSet(winId,
            MESSNUM(MESS_BOARD_SINGLE, 39) + choice, 0);
        mbWinWait(winId);
        mbAudFXDelaySet(30);
        mbAudFXPlay(MSM_SE_GUIDE_49);
        mbObjMotionShiftSet(modelId, 5, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
        while (!mbObjMotionEndCheck(modelId)) {
            HuPrcVSleep();
        }

        capsuleObj = mbCapObjCreate(mgKoopaCapsuleTbl[choice], FALSE);
        mbObjDispSet(capsuleObj, FALSE);
        mbObjCameraSet(capsuleObj, 2);
        mbObjLayerSet(capsuleObj, 7);
        mbObjAttrSet(capsuleObj, HU3D_MOTATTR_LOOP);
        mbObjMotionSpeedSet(capsuleObj, 0.0f);
        mbObjPosGet(modelId, &capsulePos);
        capsulePos.y += 200.0f;
        capsulePos.z += 200.0f;

        capsuleAnim = singleEffAnim[2];
        particleCapsule = mbParticleCreate(capsuleAnim, 100);
        mbParticleHookSet(particleCapsule, SingleEffMgCapsuleHook);
        Hu3DModelCameraSet(particleCapsule, 1);
        Hu3DModelLayerSet(particleCapsule, 5);
        particleCapsuleId = particleCapsule;
        particleCapsuleStored = particleCapsuleId;
        particleData.ids[0] = particleCapsuleStored;
        tempPos = capsulePos;
        tempPos.y += 80.0f;
        tempPos.z += 100.0f;
        Hu3DModelPosSetV(particleData.ids[0], &tempPos);
        Hu3DModelCameraSet(particleData.ids[0], 2);
        Hu3DModelLayerSet(particleData.ids[0], 7);
        Hu3DModelAttrSet(particleData.ids[0], HU3D_ATTR_DISPOFF);

        explodeAnim = singleEffAnim[1];
        particleExplode = mbParticleCreate(explodeAnim, 100);
        mbParticleHookSet(particleExplode, SingleEffMgExplodeHook);
        Hu3DModelCameraSet(particleExplode, 1);
        Hu3DModelLayerSet(particleExplode, 5);
        particleExplodeId = particleExplode;
        particleExplodeStored = particleExplodeId;
        particleData.ids[1] = particleExplodeStored;
        Hu3DModelPosSetV(particleData.ids[1], &capsulePos);
        mbParticleBlendModeSet((int)particleData.ids[1], MB_PARTICLE_BLEND_ADDCOL);
        Hu3DModelCameraSet(particleData.ids[1], 2);
        Hu3DModelLayerSet(particleData.ids[1], 7);
        Hu3DModelAttrSet(particleData.ids[1], HU3D_ATTR_DISPOFF);
        mbAudFXPlay(MSM_SE_BRD00_59);

        for (frame = 0; frame < 3; frame++) {
            Hu3DModelAttrReset(particleData.ids[0], HU3D_ATTR_DISPOFF);
            mbAudFXPlay(MSM_SE_SBRD_06);
            HuPrcSleep(12);
            mbObjPosSetV(capsuleObj, &capsulePos);
            mbObjScaleSet(capsuleObj, 1.0f, 1.0f, 1.0f);
            mbObjDispSet(capsuleObj, TRUE);
            HuPrcSleep(30);
            for (flightFrame = 0; (u32)flightFrame <= 60; flightFrame++) {
                angle = (float)flightFrame / 60.0f;
                remaining = 1.0f - angle;
                scale = 1080.0f * angle;
                mbPos3Dto2D(&capsulePos, &tempPos);
                tempPos.x = 114.0f + (128.0f * remaining);
                tempPos.y = 80.0f;
                tempPos.x += remaining * (128.0f * -mbCosDeg(-scale));
                tempPos.y += remaining * (128.0f * mbSinDeg(-scale));
                mbPos2Dto3D(&tempPos, &pos3d);
                scale = mbSinDeg(90.0f * angle);
                tempPos.x = capsulePos.x + scale * (pos3d.x - capsulePos.x);
                tempPos.y = capsulePos.y + scale * (pos3d.y - capsulePos.y);
                tempPos.z = capsulePos.z + scale * (pos3d.z - capsulePos.z);
                mbObjPosSetV(capsuleObj, &tempPos);
                scale = 0.2f + (0.8f * (1.0f - angle));
                mbObjScaleSet(capsuleObj, scale, scale, scale);
                HuPrcVSleep();
            }
            Hu3DModelPosSetV(particleData.ids[1], &tempPos);
            Hu3DModelAttrReset(particleData.ids[1], HU3D_ATTR_DISPOFF);
            mbObjDispSet(capsuleObj, FALSE);
            mbPlayerCapsuleAdd(playerNo, mgKoopaCapsuleTbl[choice]);
            omVibrate(playerNo, 20, 7, 3);
            HuPrcSleep(30);
        }
        mbObjDispSet(capsuleObj, FALSE);
        mbAudFXDelaySet(30);
        mbAudFXPlay(MSM_SE_GUIDE_47);
        mbObjMotionShiftSet(modelId, 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        HuPrcSleep(60);
        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 6), 13);
        mbWinWait(winId);
        ev_SingleKoopaMgSkip(modelId);
        mbPlayerRotYSet(playerNo, 0.0f);
        mbPlayerPosReset(playerNo);
        mbCameraPlayerViewSetFast(playerNo, 0);
    } else {
        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 8), 13);
        mbWinWait(winId);
        mbObjMotionShiftSet(modelId, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 9), 13);
        mbWinInsertMesSet(winId, mbPlayerNameMesGet(playerNo), 0);
        mbWinWait(winId);
        mbAudFXDelaySet(30);
        mbAudFXPlay(MSM_SE_GUIDE_49);
        mbObjMotionShiftSet(modelId, 5, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
        if (mbPlayerCoinGet(playerNo) == 0) {
            if (SingleMgUnlockListGet(NULL)) {
                winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 10), 13);
                mbWinWait(winId);
                returnMode = 0;
            } else {
                winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 12), 13);
                mbWinInsertMesSet(winId, mbPlayerNameMesGet(playerNo), 0);
                mbWinWait(winId);
                returnMode = 1;
            }
        } else if (SingleMgUnlockListGet(NULL)
            && mbRandMod(100) < 50) {
            winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 10), 13);
            mbWinWait(winId);
            returnMode = 0;
        } else {
            winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 11), 13);
            mbWinInsertMesSet(winId, mbPlayerNameMesGet(playerNo), 0);
            mbWinWait(winId);
            returnMode = 2;
        }

        mbObjMotionShiftSet(modelId, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 13), 13);
        mbWinWait(winId);
        frame = mbRandMod(3);
        miniKoopaMgType = mgTypeTbl[frame][0];
        _SetFlag(FLAG_BOARD_MG_KOOPA);
        mbev_MgCallSingleKoopa(mgTypeTbl[frame][1], TRUE);
    }

    mbObjKill(modelId);
    if (capsuleObj != 0) {
        mbCapObjKill(capsuleObj);
    }
    for (frame = 0; frame < 2; frame++) {
        espKill(spriteId[frame]);
    }
    for (frame = 0; frame < 2; frame++) {
        mbPlayerMotionKill(playerNo, playerMotion[frame]);
    }
    for (frame = 0; frame < 2; frame++) {
        if (particleData.ids[frame] >= 0) {
            mbParticleKill(particleData.ids[frame]);
        }
    }
    HuDataDirClose(DATA_capsulechar1);
    if (mbWipeSpecialStatGet()) {
        mbWipeFadeIn();
    }
}

static void ev_SingleKoopaMgSkip(MBMODELID modelId)
{
    s16 winId;

    winId = mbWinCreate(2, SINGLE_MESS_KOOPA_MG_SKIP, 13);
    mbWinWait(winId);
    mbWipeSpecialFadeInCreate(7, 30);
    mbWipeSpecialWait();
    mbWipeFadeOutTime(1);
    mbWipeSpecialKill();
    mbObjDispSet(modelId, FALSE);
    mbMusBoardPlay();
}



static inline s16 SingleMgCoinGet(int playerNo)
{
    return GwPlayer[playerNo].mgCoin;
}

static inline s16 SingleMgCoinBonusGet(int playerNo)
{
    return GwPlayer[playerNo].mgCoinBonus;
}

static inline BOOL SingleEffBusyCheck(s16 effNo)
{
    SINGLE_EFF_DATA *work = &singleEffData[effNo - 1];
    return work->state != 0;
}

static inline void SingleMasuTypeAdd(s16 type)
{
    masuType[masuTypeNum++] = (u8)type;
    masuTypeNum %= 5;
}

static inline void SingleEffPrizeStart(s16 effNo)
{
    SINGLE_EFF_DATA *work;
    int i;

    work = &singleEffData[effNo - 1];
    work->state = 3;
    work->targetPos = work->pos;
    work->timer = 0;
    work->timerMax = 58;
    Hu3DModelCameraSet(work->modelId, 2);
    Hu3DModelLayerSet(work->modelId, 7);
    for (i = 0; i < 2; i++) {
        Hu3DModelCameraSet(work->childModelId[i], 2);
        Hu3DModelLayerSet(work->childModelId[i], 7);
    }
    mbAudFXPlay(MSM_SE_SBRD_02);
}

static inline void SingleMgUnlock(int mgNo)
{
    mbSingleMgUnlockSet(mgNo);
    GWSingleMgFlagSet(mgNo);
}

static inline void SingleEffUnk04Set(s16 effNo, BOOL value)
{
    SINGLE_EFF_DATA *work = &singleEffData[effNo - 1];
    work->unk04 = value;
}

static inline void SingleEffPosGet(s16 effNo, HuVecF *pos)
{
    SINGLE_EFF_DATA *work = &singleEffData[effNo - 1];
    *pos = work->pos;
}

static void ev_SingleKoopaMgEnd(int playerNo)
{
    static int guideMot[] = {
        DATANUM(DATA_capsulechar1, 1),
        DATANUM(DATA_capsulechar1, 5),
        DATANUM(DATA_capsulechar1, 3),
        DATANUM(DATA_capsulechar1, 7),
        DATANUM(DATA_capsulechar1, 4),
        DATANUM(DATA_capsulechar1, 10),
        -1,
    };
    HU3D_MODELID particleCapsule[5] = { -1, -1, -1, -1, -1 };
    MBMODELID capsuleObj[3] = { 0, 0, 0 };
    HU3D_MODELID particles[10] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    };
    s16 effects[5] = { 0, 0, 0, 0, 0 };
    HuVecF effectPos;
    HuVecF pos;
    HuVecF startPos;
    s16 masuId;
    s16 masuStart;
    s16 winId;
    int resultMode;
    int seNo;
    BOOL unlocked;
    MBMODELID guideModel;
    static const HuVecF cameraOffset = { 0.0f, 100.0f, 100.0f };
    int mgNo;
    int effectCount;
    int particleCount;
    int i;
    u8 effectMasuType;
    u8 unlockedMg[128];

    if (!mbWipeSpecialStatGet()) {
        void mbWipeFadeOut(void);
        mbWipeFadeOut();
    }
    mbStatusDispForceSet(playerNo, TRUE);
    mgNo = GwSystem.mgNo;
    masuId = GwPlayer[playerNo].masuId;
    mbPlayerRotYSet(playerNo, 0.0f);
    guideModel = mbObjCreate(DATA_capsulechar1, guideMot, FALSE);
    masuStart = mbMasuFind_AttrIdGet(MASU_NULL, MASU_FLAG_START);
    mbMasuPosGet(masuStart, &startPos);
    mbObjPosSetV(guideModel, &startPos);
    mbObjDispSet(guideModel, TRUE);
    mbObjMotionSet(guideModel, 1, HU3D_MOTATTR_LOOP);
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
    mbPlayerPosSet(playerNo, startPos.x, startPos.y, startPos.z + 300.0f);
    mbPlayerMotionSet(playerNo, 1, HU3D_MOTATTR_LOOP);
    mbPlayerRotYSet(playerNo, 180.0f);
    mbCameraMoveMasu(masuStart, NULL, (HuVecF *)&cameraOffset, 1600.0f, -1.0f, -1);
    mbCameraMoveWait();
    mbMusPlay(MB_MUS_CHAN_BG, 28, MSM_VOL_MAX, 0);

    if (SingleMgCoinBonusGet(playerNo) + SingleMgCoinGet(playerNo) > 0) {
        resultMode = 0;
    } else {
        for (i = 1; i < GW_PLAYER_MAX; i++) {
            if (SingleMgCoinBonusGet(i) + SingleMgCoinGet(i) > 0) {
                break;
            }
        }
        resultMode = (i < GW_PLAYER_MAX) ? 1 : 2;
    }

    switch (resultMode) {
    case 0: {
        mbSingleCall(10, MgDataTbl[mgNo].type);
        mbAudFXDelaySet(30);
        mbAudFXPlay(MSM_SE_GUIDE_48);
        mbObjMotionSet(guideModel, 6, HU3D_MOTATTR_LOOP);
        {
            void mbWipeFadeIn(void);
            mbWipeFadeIn();
        }
        mbPauseDisableSet(FALSE);
        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 14), 13);
        mbWinWait(winId);
        mbAudFXDelaySet(30);
        mbAudFXPlay(MSM_SE_GUIDE_48);
        mbObjMotionShiftSet(guideModel, 6, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
        unlocked = SingleMgUnlockedCheck(mgNo + GW_MGNO_BASE);
        if (!unlocked) {
            winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 15), 13);
            mbWinWait(winId);
        } else {
            winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 42), 13);
            mbWinWait(winId);
        }
        mbAudFXDelaySet(30);
        mbAudFXPlay(MSM_SE_GUIDE_49);
        mbObjMotionShiftSet(guideModel, 5, 0.0f, 8.0f,
            HU3D_MOTATTR_NONE);
        while (!mbObjMotionEndCheck(guideModel)) {
            HuPrcVSleep();
        }

        pos.x = startPos.x;
        pos.y = startPos.y + 200.0f;
        pos.z = startPos.z + 200.0f;
        particleCapsule[0] = SingleParticleCreate(2, 100, SingleEffMgCapsuleHook);
        effectPos.x = pos.x;
        effectPos.y = pos.y + 80.0f;
        effectPos.z = pos.z + 100.0f;
        Hu3DModelPosSetV(particleCapsule[0], &effectPos);
        Hu3DModelCameraSet(particleCapsule[0], 2);
        Hu3DModelLayerSet(particleCapsule[0], 7);
        mbAudFXPlay(MSM_SE_SBRD_06);
        HuPrcSleep(12);
        mbAudFXPlay(MSM_SE_BRD00_59);
        effects[0] = SingleEffCreate(&effectPos, miniKoopaMgType);
        HuPrcSleep(30);
        SingleEffPrizeStart(effects[0]);
        HuPrcSleep(60);
        if (!unlocked) {
            SingleMgUnlock(mgNo + GW_MGNO_BASE);
            SingleMasuTypeAdd(miniKoopaMgType);
            mbSingleCall(9, mgNo);
        }
        SingleMgRecordPrizeSet();
        while (SingleEffBusyCheck(effects[0])) {
            HuPrcVSleep();
        }
        HuPrcSleep(30);
        if (!(MgDataTbl[mgNo].flag & MG_FLAG_COIN)) {
            mbCoinAddExec(playerNo, 10);
        } else {
            mbCoinAddExec(playerNo, SingleMgCoinGet(playerNo) + SingleMgCoinBonusGet(playerNo));
        }
        mbPlayerMotionShiftSet(playerNo, 7, 0.0f, 8.0f,
            HU3D_MOTATTR_NONE);
        mbPlayerWinLoseVoicePlay(playerNo, 7, 579);
        if (!unlocked) {
            winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 0), -1);
            mbWinInsertMesSet(winId, mbPlayerNameMesGet(playerNo), 0);
            if (mgNo != (u16)-1) {
                mbWinInsertMesSet(winId, MgDataTbl[mgNo].nameMes, 1);
            } else {
                mbWinInsertMesSet(winId, MESSNUM(MESS_BOARD_SINGLE, 39), 1);
            }
            mbWinWait(winId);
        }
        while (!mbPlayerMotionEndCheck(playerNo)) {
            HuPrcVSleep();
        }
        HuPrcSleep(30);
        mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
        mbAudFXDelaySet(30);
        mbAudFXPlay(MSM_SE_GUIDE_47);
        mbObjMotionShiftSet(guideModel, 3, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
        HuPrcSleep(30);
        break;
    }
    case 1:
        mbAudFXDelaySet(30);
        mbAudFXPlay(MSM_SE_GUIDE_47);
        mbObjMotionSet(guideModel, 3, HU3D_MOTATTR_LOOP);
        {
            void mbWipeFadeIn(void);
            mbWipeFadeIn();
        }
        mbPauseDisableSet(FALSE);
        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 16), 13);
        mbWinInsertMesSet(winId, mbPlayerNameMesGet(playerNo), 0);
        mbWinWait(winId);
        switch (returnMode) {
        case 0:
            effectCount = SingleMgUnlockListGet(unlockedMg);
            if (effectCount != 0) {
                winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 17), 13);
                mbWinWait(winId);
                if (effectCount > 5) {
                    effectCount = 5;
                }
                effectPos.x = startPos.x
                    - (100.0f * (0.75f * (float)(effectCount - 1)));
                effectPos.y = startPos.y + 200.0f;
                effectPos.z = startPos.z + 200.0f;
                for (i = 0; i < effectCount; i++) {
                    effectMasuType = masuType[i];
                    effects[i] = SingleEffCreate(&effectPos, effectMasuType);
                    SingleEffUnk04Set(effects[i], FALSE);
                    particleCapsule[i] = SingleParticleCreate(2, 100, SingleEffMgCapsuleHook);
                    Hu3DModelPosSet(particleCapsule[i], effectPos.x,
                        effectPos.y + 80.0f, effectPos.z + 100.0f);
                    Hu3DModelCameraSet(particleCapsule[i], 2);
                    Hu3DModelLayerSet(particleCapsule[i], 7);
                    effectPos.x += 150.0f;
                }
                mbAudFXPlay(MSM_SE_SBRD_06);
                HuPrcSleep(12);
                for (i = 0; i < effectCount; i++) {
                    SingleEffUnk04Set(effects[i], TRUE);
                }
                seNo = mbAudFXPlay(MSM_SE_SBRD_01);
                HuPrcSleep(12);
                mbObjAttrReset(guideModel, HU3D_MOTATTR_LOOP);
                while (!mbObjMotionEndCheck(guideModel)) {
                    HuPrcVSleep();
                }
                mbAudFXDelaySet(30);
                mbAudFXPlay(MSM_SE_GUIDE_49);
                mbObjMotionShiftSet(guideModel, 5, 0.0f, 8.0f,
                    HU3D_MOTATTR_NONE);
                HuPrcVSleep();
                while (mbObjMotionShiftIDGet(guideModel) != -1
                    || !mbObjMotionEndCheck(guideModel)) {
                    HuPrcVSleep();
                }
                mbAudFXPlay(MSM_SE_BRD00_59);
                mbAudFXStop(seNo);
                mbAudFXPlay(MSM_SE_SBRD_07);
                omVibrate(playerNo, 20, 20, 0);
                particleCount = 0;
                for (i = 0; i < effectCount; i++) {
                    SingleEffPosGet(effects[i], &effectPos);
                    particles[particleCount] = SingleParticleCreate(3, 256, SingleEffMgFireHook);
                    Hu3DModelPosSetV(particles[particleCount], &effectPos);
                    particleCount++;
                    particles[particleCount] = SingleParticleCreate(3, 64, SingleEffMgFire2Hook);
                    Hu3DModelPosSet(particles[particleCount], 0.0f, 0.05f, 0.05f);
                    Hu3DModelPosSetV(particles[particleCount], &effectPos);
                    particleCount++;
                }
                HuPrcSleep(12);
                mbSingleMgUnlockInit();
                SingleMasuTypeReset();
                GwSingleMgFlag[0] = GwSingleMgFlag[1] = GwSingleMgFlag[2] = 0;
                mbSinglePrizeFlagReset(6);
                GWSingleMgWinNumSet(0);
                mbSinglePrizeFlagReset(5);
                GWSingleMgRecordNumSet(0);
                SingleMgRecordRestore();
                for (i = 0; i < effectCount; i++) {
                    SingleEffUnk04Set(effects[i], FALSE);
                }
                HuPrcSleep(60);
                mbPlayerMotionShiftSet(playerNo, 8, 0.0f, 8.0f,
                    HU3D_MOTATTR_NONE);
                mbAudFXDelaySet(30);
                mbAudFXPlay(MSM_SE_GUIDE_47);
                mbObjMotionShiftSet(guideModel, 3, 0.0f, 8.0f,
                    HU3D_MOTATTR_LOOP);
            }
            break;
        case 1:
            winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 19), 13);
            mbWinWait(winId);
            effectCount = mbPlayerCapsuleNumGet(playerNo);
            effectPos.x = startPos.x
                - (100.0f * (0.75f * (float)(effectCount - 1)));
            effectPos.y = startPos.y + 200.0f;
            effectPos.z = startPos.z + 200.0f;
            for (i = 0; i < effectCount; i++) {
                capsuleObj[i] = mbCapObjCreate(
                    mbPlayerCapsuleGet(playerNo, i), FALSE);
                mbObjDispSet(capsuleObj[i], FALSE);
                mbObjPosSetV(capsuleObj[i], &effectPos);
                mbObjCameraSet(capsuleObj[i], 1);
                mbObjLayerSet(capsuleObj[i], 4);
                mbObjMotionSpeedSet(capsuleObj[i], 0.0f);
                particleCapsule[i] = SingleParticleCreate(2, 100, SingleEffMgCapsuleHook);
                Hu3DModelPosSet(particleCapsule[i], effectPos.x,
                    effectPos.y + 80.0f, effectPos.z + 100.0f);
                Hu3DModelCameraSet(particleCapsule[i], 2);
                Hu3DModelLayerSet(particleCapsule[i], 7);
                effectPos.x += 150.0f;
            }
            mbAudFXPlay(MSM_SE_SBRD_06);
            for (i = 0; i < effectCount; i++) {
                mbPlayerCapsuleRemove(playerNo, 0);
            }
            HuPrcSleep(12);
            for (i = 0; i < effectCount; i++) {
                mbObjDispSet(capsuleObj[i], TRUE);
            }
            HuPrcSleep(12);
            mbObjAttrReset(guideModel, HU3D_MOTATTR_LOOP);
            while (!mbObjMotionEndCheck(guideModel)) {
                HuPrcVSleep();
            }
            mbAudFXDelaySet(30);
            mbAudFXPlay(MSM_SE_GUIDE_49);
            mbObjMotionShiftSet(guideModel, 5, 0.0f, 8.0f,
                HU3D_MOTATTR_NONE);
            HuPrcVSleep();
            while (mbObjMotionShiftIDGet(guideModel) != -1
                || !mbObjMotionEndCheck(guideModel)) {
                HuPrcVSleep();
            }
            mbAudFXPlay(MSM_SE_BRD00_59);
            mbAudFXPlay(MSM_SE_SBRD_07);
            omVibrate(playerNo, 20, 20, 0);
            particleCount = 0;
            for (i = 0; i < effectCount; i++) {
                mbObjPosGet(capsuleObj[i], &effectPos);
                particles[particleCount] = SingleParticleCreate(3, 256, SingleEffMgFireHook);
                Hu3DModelPosSetV(particles[particleCount], &effectPos);
                particleCount++;
                particles[particleCount] = SingleParticleCreate(3, 64, SingleEffMgFire2Hook);
                Hu3DModelPosSet(particles[particleCount], 0.0f, 0.05f, 0.05f);
                Hu3DModelPosSetV(particles[particleCount], &effectPos);
                particleCount++;
            }
            HuPrcSleep(12);
            for (i = 0; i < effectCount; i++) {
                mbObjDispSet(capsuleObj[i], FALSE);
            }
            HuPrcSleep(60);
            mbPlayerMotionShiftSet(playerNo, 8, 0.0f, 8.0f,
                HU3D_MOTATTR_NONE);
            mbAudFXDelaySet(30);
            mbAudFXPlay(MSM_SE_GUIDE_47);
            mbObjMotionShiftSet(guideModel, 3, 0.0f, 8.0f,
                HU3D_MOTATTR_LOOP);
            break;
        case 2:
            winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 18), 13);
            mbWinWait(winId);
            mbAudFXDelaySet(30);
            mbAudFXPlay(MSM_SE_GUIDE_49);
            mbObjMotionShiftSet(guideModel, 5, 0.0f, 8.0f,
                HU3D_MOTATTR_NONE);
            while (!mbObjMotionEndCheck(guideModel)) {
                HuPrcVSleep();
            }
            mbWipeWhiteFadeOutTime(1);
            {
                void mbWipeWhiteFadeInTime(int time);
                mbAudFXPlay(MSM_SE_BRD00_59);
                mbWipeWhiteFadeInTime(90);
            }
            omVibrate(playerNo, 20, 20, 0);
            mbPlayerMotionShiftSet(playerNo, 8, 0.0f, 8.0f,
                HU3D_MOTATTR_NONE);
            mbCoinAddProcExec(playerNo, -mbPlayerCoinGet(playerNo), -1, TRUE);
            mbAudFXDelaySet(30);
            mbAudFXPlay(MSM_SE_GUIDE_47);
            mbObjMotionShiftSet(guideModel, 3, 0.0f, 8.0f,
                HU3D_MOTATTR_LOOP);
            HuPrcSleep(30);
            break;
        }
        while (!mbPlayerMotionEndCheck(playerNo)) {
            HuPrcVSleep();
        }
        HuPrcSleep(30);
        mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
        break;
    case 2:
        mbAudFXDelaySet(30);
        mbAudFXPlay(MSM_SE_GUIDE_47);
        mbObjMotionSet(guideModel, 3, HU3D_MOTATTR_LOOP);
        {
            void mbWipeFadeIn(void);
            mbWipeFadeIn();
        }
        mbPauseDisableSet(FALSE);
        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 20), 13);
        mbWinInsertMesSet(winId, mbPlayerNameMesGet(playerNo), 0);
        mbWinWait(winId);
        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 21), 13);
        mbWinWait(winId);
        break;
    }

    ev_SingleKoopaMgSkip(guideModel);
    mbPlayerPosReset(playerNo);
    mbCameraPlayerViewSetFast(playerNo, 0);
    mbObjKill(guideModel);
    for (i = 0; i < 5; i++) {
        if (particleCapsule[i] >= 0) {
            mbParticleKill(particleCapsule[i]);
        }
    }
    for (i = 0; i < 10; i++) {
        if (particles[i] >= 0) {
            mbParticleKill(particles[i]);
        }
    }
    for (i = 0; i < 3; i++) {
        if (capsuleObj[i] > 0) {
            mbCapObjKill(capsuleObj[i]);
        }
    }
    for (i = 0; i < 5; i++) {
        if (effects[i] > 0) {
            SingleEffKill(effects[i]);
        }
    }
    HuDataDirClose(DATA_capsulechar1);
}
static inline int SingleMKoopaSePlay(int seId)
{
    int i;
    int j;
    static int seLoseTbl[][3] = {
        { 675, 627, 603 },
        { 676, 628, 604 },
        { 677, 629, 605 },
        { 681, 633, 609 },
        { -1, -1, -1 },
    };

    for (i = 0; seLoseTbl[i][0] >= 0; i++) {
        for (j = 0; j < 3; j++) {
            if (seLoseTbl[i][j] == seId) {
                break;
            }
        }
        if (j < 3) {
            break;
        }
    }
    if (seLoseTbl[i][0] >= 0) {
        return mbAudFXPlay(seLoseTbl[i][miniKoopaType]);
    } else {
        return mbAudFXPlay(seId);
    }
}
static HuVecF viewOfs750 = { 0.0f, 100.0f, 0.0f };
static HuVecF viewOfs825 = { 0.0f, 100.0f, 0.0f };

static inline BOOL SingleMKoopaMgLockedCheck(void)
{
    int i;

    for (i = 0; MgDataTbl[i].ovl != (u16)-1; i++) {
        if (MgDataTbl[i].type == MG_TYPE_KETTOU
            && !GWMgUnlockGet(i + GW_MGNO_BASE)
            && !mbSingleMgUnlockGet(i + GW_MGNO_BASE)) {
            return FALSE;
        }
    }
    return TRUE;
}

static void ev_SingleMKoopaMg(int playerNo, s16 masuId)
{
    HuVecF masuPos;
    HuVecF playerPos;
    HuVecF opponentPos;
    HuVecF delta;
    float angle;
    s16 winId;
    int opponentPlayerNo;
    int characterNo;
    s16 playerMotion;
    s16 opponentMotion;

    miniKoopaType = mbMasuCapsuleGet(masuId);
    if (miniKoopaType < 0) {
        miniKoopaType = 0;
    }
opponentPlayerNo = miniKoopaType + 1;
    characterNo = miniKoopaType + 19;

    mbWipeSpecialFadeInCreate(8, 30);
    mbWipeSpecialWait();
    GwPlayer[opponentPlayerNo].masuId = masuId;
    GwPlayer[opponentPlayerNo].masuIdNext = masuId;
    mbPlayerDispSet(opponentPlayerNo, TRUE);
    mbPlayerMotionSet(playerNo, 1, HU3D_MOTATTR_LOOP);
    mbPlayerMotionSet(opponentPlayerNo, 1, HU3D_MOTATTR_LOOP);
    playerMotion = mbPlayerMotionCreate(playerNo,
        DATANUM(DATA_mario, 120));
    opponentMotion = mbPlayerMotionCreate(opponentPlayerNo,
        DATANUM(DATA_mario, 120));

    mbMasuPosGet(masuId, &masuPos);
    mbPlayerPosSet(playerNo, masuPos.x + 70.0f, masuPos.y,
        masuPos.z - 70.0f);
    mbPlayerPosSet(opponentPlayerNo, masuPos.x - 70.0f, masuPos.y,
        masuPos.z + 70.0f);
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
    mbPlayerColSnapPlayerSet(opponentPlayerNo, FALSE);
    mbPlayerPosGet(playerNo, &playerPos);
    mbPlayerPosGet(opponentPlayerNo, &opponentPos);
    PSVECSubtract(&opponentPos, &playerPos, &delta);
    angle = (float)(180.0 * (atan2(delta.x, delta.z) / M_PI));
    mbPlayerRotYSet(playerNo, angle);
    mbPlayerRotYSet(opponentPlayerNo, 180.0f + angle);

    mbCameraMoveMasu(masuId, NULL, &viewOfs750, 1600.0f, -1.0f,
        -1);
    mbCameraMoveWait();
    mbMusPlay(MB_MUS_CHAN_BG, 26, MSM_VOL_MAX, 0);
    mbWipeSpecialFadeOutCreate(8, 30);
    mbWipeSpecialWait();

    winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 22), characterNo);
    SingleMKoopaSePlay(677);
    mbWinInsertMesSet(winId, mbPlayerNameMesGet(playerNo), 0);
    mbWinWait(winId);

    if (!SingleMKoopaMgLockedCheck()) {
        if (mbPlayerCoinGet(playerNo) == 0) {
            winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 23),
                characterNo);
            mbWinWait(winId);
            winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 24),
                characterNo);
            SingleMKoopaSePlay(677);
            mbWinWait(winId);
            returnMode = 0;
        } else {
            winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 23),
                characterNo);
            mbWinWait(winId);
            winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 26),
                characterNo);
            SingleMKoopaSePlay(677);
            mbWinWait(winId);
            returnMode = 1;
        }
    } else if (mbPlayerCoinGet(playerNo) != 0) {
        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 28),
            characterNo);
        mbWinWait(winId);
        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 26),
            characterNo);
        SingleMKoopaSePlay(677);
        mbWinWait(winId);
        returnMode = 2;
    } else {
        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 28),
            characterNo);
        mbWinWait(winId);
        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 24),
            characterNo);
        SingleMKoopaSePlay(677);
        mbWinWait(winId);
        returnMode = 3;
    }

    mbPlayerMotionShiftSet(playerNo, playerMotion, 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);
    Hu3DMotionAttrSet(mbObjMotionIDGet(mbPlayerObjIDGet(playerNo),
        playerMotion), 1);
    mbPlayerMotionShiftSet(opponentPlayerNo, opponentMotion, 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);
    Hu3DMotionAttrSet(mbObjMotionIDGet(mbPlayerObjIDGet(opponentPlayerNo),
        opponentMotion), 1);

    winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 29), characterNo);
    SingleMKoopaSePlay(675);
    mbWinWait(winId);
    _SetFlag(FLAG_BOARD_MG_KETTOU);
    mbev_MgCallSingle(6);
    mbPlayerMotionKill(playerNo, playerMotion);
    mbPlayerMotionKill(opponentPlayerNo, opponentMotion);
    mbPlayerDispSet(opponentPlayerNo, FALSE);
    GwPlayer[opponentPlayerNo].masuId = 0;
}

static void ev_SingleMKoopaMgEnd(int playerNo)
{
    HuVecF effectPos;
    HuVecF playerPos;
    HuVecF opponentPos;
    HuVecF masuPos;
    HuVecF delta;
    int mgNo;
    int masuTypeNo;
    int opponentPlayerNo;
    int seNo;
    BOOL unlocked;
    int characterNo;
    s16 particleId;
    s16 effNo;
    s16 masuId;
    s16 winId;
    float angle;

    effNo = 0;
    particleId = -1;

    if (!mbWipeSpecialStatGet()) {
        mbWipeFadeOut();
    }
    mgNo = GwSystem.mgNo;
    masuId = GwPlayer[playerNo].masuId;
    masuTypeNo = mbMasuTypeGet(masuId);
    mbStatusDispForceSet(playerNo, TRUE);
    mbCameraMoveMasu(masuId, NULL, &viewOfs825, 1600.0f, -1.0f, -1);
    mbCameraMoveWait();

    miniKoopaType = mbMasuCapsuleGet(masuId);
    if (miniKoopaType < 0) {
        miniKoopaType = 0;
    }
    opponentPlayerNo = miniKoopaType + 1;
    characterNo = miniKoopaType + 19;
    GwPlayer[opponentPlayerNo].masuId = masuId;
    GwPlayer[opponentPlayerNo].masuIdNext = masuId;
    mbPlayerDispSet(opponentPlayerNo, TRUE);
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
    mbPlayerColSnapPlayerSet(opponentPlayerNo, FALSE);
    mbMasuPosGet(masuId, &masuPos);
    mbPlayerPosSet(playerNo, masuPos.x + 70.0f, masuPos.y,
        masuPos.z - 70.0f);
    mbPlayerPosSet(opponentPlayerNo, masuPos.x - 70.0f, masuPos.y,
        masuPos.z + 70.0f);
    mbMusPlay(MB_MUS_CHAN_BG, 26, MSM_VOL_MAX, 0);

    if (SingleMgCoinBonusGet(playerNo) + SingleMgCoinGet(playerNo) > 0) {
        mbSingleCall(10, 6);
        mbPlayerMotionSet(opponentPlayerNo, 6, HU3D_MOTATTR_LOOP);
        mbWipeFadeIn();
        mbPauseDisableSet(FALSE);
        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 31), characterNo);
        SingleMKoopaSePlay(681);
        mbWinWait(winId);

        switch (returnMode) {
        case 0:
        case 1:
                winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 33),
                    characterNo);
                mbWinWait(winId);
                mbMasuPosGet(masuId, &effectPos);
                effectPos.y += 200.0f;
                effectPos.z += 200.0f;
                particleId = SingleParticleCreate(2, 100, SingleEffMgCapsuleHook);
                Hu3DModelPosSet(particleId, effectPos.x,
                    effectPos.y + 80.0f, effectPos.z + 100.0f);
                Hu3DModelCameraSet(particleId, 2);
                Hu3DModelLayerSet(particleId, 7);
                HuPrcSleep(12);
                effNo = SingleEffCreate(&effectPos, masuTypeNo);
                seNo = mbAudFXPlay(MSM_SE_SBRD_01);
                HuPrcSleep(30);
                mbAudFXStop(seNo);
                SingleEffPrizeStart(effNo);
                HuPrcSleep(60);
                if (!(unlocked = SingleMgUnlockedCheck(mgNo + GW_MGNO_BASE))) {
                    SingleMgUnlock(mgNo + GW_MGNO_BASE);
                    SingleMasuTypeAdd(miniKoopaType + 9);
                    mbSingleCall(9, mgNo);
                }
                while (SingleEffBusyCheck(effNo)) {
                    HuPrcVSleep();
                }
                HuPrcSleep(30);
                mbCoinAddExec(playerNo, 10);
                mbPlayerMotionShiftSet(playerNo, 7, 0.0f, 8.0f,
                    HU3D_MOTATTR_NONE);
                mbPlayerWinLoseVoicePlay(playerNo, 7, 579);
                if (!unlocked) {
                    winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 0), -1);
                    mbWinInsertMesSet(winId, mbPlayerNameMesGet(playerNo), 0);
                    if (mgNo != (u16)-1) {
                        mbWinInsertMesSet(winId, MgDataTbl[mgNo].nameMes, 1);
                    } else {
                        mbWinInsertMesSet(winId,
                            MESSNUM(MESS_BOARD_SINGLE, 39), 1);
                    }
                    mbWinWait(winId);
                }
            break;
        case 2:
        case 3:
            winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 36),
                characterNo);
            mbWinWait(winId);
            mbPlayerMotionShiftSet(playerNo, 7, 0.0f, 8.0f,
                HU3D_MOTATTR_NONE);
            mbPlayerWinLoseVoicePlay(playerNo, 7, 579);
            mbCoinAddProcExec(playerNo, 10, TRUE, TRUE);
            break;
        }
        SingleMgRecordPrizeSet();
        while (!mbPlayerMotionEndCheck(playerNo)) {
            HuPrcVSleep();
        }
        HuPrcSleep(30);
        mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
    } else if (SingleMgCoinBonusGet(1) + SingleMgCoinGet(1) > 0) {
        mbPlayerMotionSet(playerNo, 6, HU3D_MOTATTR_LOOP);
        mbPlayerMotionSet(opponentPlayerNo, 7, HU3D_MOTATTR_NONE);
        mbWipeFadeIn();
        mbPauseDisableSet(FALSE);
        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 30),
            characterNo);
        mbWinWait(winId);
        switch (returnMode) {
        case 0:
        case 3:
            winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 37),
                characterNo);
            mbWinWait(winId);
            break;
        case 1:
        case 2:
            winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 38),
                characterNo);
            mbWinWait(winId);
            mbCoinAddProcExec(playerNo,
                -(mbPlayerCoinGet(playerNo) + 1) / 2, -1, TRUE);
            break;
        }
    } else {
        mbPlayerMotionSet(playerNo, 1, HU3D_MOTATTR_LOOP);
        mbPlayerMotionSet(opponentPlayerNo, 1, HU3D_MOTATTR_LOOP);
        mbPlayerPosGet(playerNo, &playerPos);
        mbPlayerPosGet(opponentPlayerNo, &opponentPos);
        PSVECSubtract(&opponentPos, &playerPos, &delta);
        angle = (float)(180.0 * (atan2(delta.x, delta.z) / M_PI));
        mbPlayerRotYSet(playerNo, angle);
        mbPlayerRotYSet(opponentPlayerNo, 180.0f + angle);
        mbWipeFadeIn();
        mbPauseDisableSet(FALSE);
        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 43),
            characterNo);
        mbWinWait(winId);
        goto cleanup;
    }

    mbPlayerPosGet(playerNo, &playerPos);
    mbPlayerPosGet(opponentPlayerNo, &opponentPos);
    PSVECSubtract(&opponentPos, &playerPos, &delta);
    angle = (float)(180.0 * (atan2(delta.x, delta.z) / M_PI));
    mbPlayerRotateStart(playerNo, (s16)angle, 15);
    mbPlayerRotateStart(opponentPlayerNo, (s16)(180.0f + angle), 15);
    HuPrcSleep(30);
    winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 32), characterNo);
    SingleMKoopaSePlay(676);
    mbWinWait(winId);

cleanup:
    mbWipeSpecialFadeInCreate(8, 30);
    mbWipeSpecialWait();
    mbWipeFadeOutTime(1);
    mbWipeSpecialKill();
    mbPlayerDispSet(opponentPlayerNo, FALSE);
    GwPlayer[opponentPlayerNo].masuId = 0;
    if (effNo != 0) {
        SingleEffKill(effNo);
    }
    if (particleId >= 0) {
        mbParticleKill(particleId);
    }
}

static void SingleMgSaveInit(void)
{
    SINGLE_SAVE_WORK *saveWork = &singleSaveWork;

    if (mbSaveNewF && !_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        GWSingleDataInit();
        GWSingleMgWinNumSet(0);
        GWSingleMgRecordNumSet(0);
        memset(singleBoardFlagOld, 0, sizeof(singleBoardFlagOld));
        memset(saveWork, 0, sizeof(*saveWork));
    }
}
void mbSinglePrizeFlagReset(int flag)
{
    if (flag <= 63) {
        GwSinglePrizeFlag[flag >> 5] &=
            ~(1 << (flag & SINGLE_PRIZE_FLAG_WORD_MASK));
    }
}

int mbSingleCall(int mode, int arg)
{
    GW_PLAYER_COM_DIF storyComDif;
    int listNum;
    int mgType;
    int candidateNum;
    int result;
    int i;
    int playerNo = GwSystem.turnPlayerNo;
    SINGLE_SAVE_WORK *work = &singleSaveWork;
    u8 candidates[10];
    u8 mgCandidates[128];
    int historyNo;

    if ((GWPartyGet() != FALSE) || _CheckFlag(FLAG_BOARD_TUTORIAL)) {
        return 0;
    }
    switch (mode) {
    case 0:
        work->micResult = -1;
        singleListenerOnF = TRUE;
        if (singleMicF && !singleListenerCreateF) {
            HuMCListenerCreate(singleMicContext, SingleMicListener, FALSE);
            singleListenerCreateF = TRUE;
        }
        return -1;

    case 1:
        if (singleMicF && singleListenerCreateF) {
            HuMCListenerKill();
            singleListenerCreateF = FALSE;
        }
        work->micResult = -1;
        singleListenerOnF = FALSE;
        return -1;

    case 2: {
        if (singleMicF && singleListenerCreateF) {
            HuMCListenerKill();
            singleListenerCreateF = FALSE;
        }
        singleListenerOnF = FALSE;
        if (work->micResult >= 0 && GwPlayer[playerNo].diceMode == 0) {
            work->micUseCount++;
            result = work->micResult;
            if (mbRandMod(100) < 50) {
                candidateNum = 0;
                for (i = 0; i < 6; i++) {
                    if (i != work->micResult) {
                        candidates[candidateNum++] = i;
                    }
                }
                result = candidates[mbRandMod(candidateNum)];
            }
            return result;
        }
        return -1;
    }

    case 3:
        if (work->mgPlayCount < 99) {
            work->mgPlayCount++;
        }
        work->mgHistory[work->mgHistoryNo] = arg;
        if (!GWSinglePrizeFlagGet(12)) {
            historyNo = work->mgHistoryNo - 1;
            for (i = 0; i < 2; i++, historyNo--) {
                if (historyNo < 0) {
                    historyNo = 2;
                }
                if (work->mgHistory[historyNo] != arg) {
                    break;
                }
            }
            if (i >= 2) {
                GWSinglePrizeFlagSet(12);
                mbSinglePrizeFlagReset(11);
            } else if (i == 1) {
                GWSinglePrizeFlagSet(11);
            }
        }
        if ((arg & 1) == 0) {
            work->mgEvenCount++;
        } else {
            work->mgOddCount++;
        }
        work->mgValueTotal += arg;
        if (++work->mgHistoryNo >= 3) {
            work->mgHistoryNo = 0;
        }
        if (work->micResult < 0) {
            work->micFirstSuccess = TRUE;
        } else if (work->micResult + 1 == arg) {
            work->micSuccessCount++;
        }
        break;

    case 4:
        if (work->capsulePlayCount < 99) {
            work->capsulePlayCount++;
        }
        if (arg == 2) {
            work->capsuleTwoF = TRUE;
        } else {
            work->capsuleOtherF = TRUE;
        }
        break;

    case 5:
        if (work->selectPlayCount < 99) {
            work->selectPlayCount++;
        }
        work->selectHistory[work->selectHistoryNo] = arg;
        if (!GWSinglePrizeFlagGet(26)) {
            historyNo = work->selectHistoryNo - 1;
            for (i = 0; i < 2; i++, historyNo--) {
                if (historyNo < 0) {
                    historyNo = 2;
                }
                if (work->selectHistory[historyNo] != arg) {
                    break;
                }
            }
            if (i >= 2) {
                GWSinglePrizeFlagSet(26);
                mbSinglePrizeFlagReset(25);
            } else if (i == 1) {
                GWSinglePrizeFlagSet(25);
            }
        }
        if (++work->selectHistoryNo >= 3) {
            work->selectHistoryNo = 0;
        }
        break;

    case 6:
        if (work->killerPlayCount < 99) {
            work->killerPlayCount++;
        }
        break;

    case 7: {
        int masuType;

        if (mbMasuDispCheck(arg)) {
            u32 *boardFlag = &singleBoardFlagOld[singleBoard * 2] + 1;

            i = arg - 1;
            if (i >= 32) {
                boardFlag--;
                i -= 32;
            }
            *boardFlag |= 1 << i;
        }
        masuType = mbMasuTypeGet(arg);
        if (masuType == 7) {
            GWSinglePrizeFlagSet(39);
        }
        if (work->masuTypeCount[masuType] < 99) {
            work->masuTypeCount[masuType]++;
        }
        break;
    }

    case 8:
        if (work->mgEndCount < 99) {
            work->mgEndCount++;
        }
        break;

    case 9: {
        GWSinglePrizeFlagSet(6);
        GWSingleMgWinNumSet(GWSingleMgWinNumGet() + 1);
        mgType = MgDataTbl[arg].type;
        listNum = SingleMgListGet(mgType, mgCandidates);
        if (listNum == 0) {
            switch (mgType) {
            case MG_TYPE_4P:
                GWSinglePrizeFlagSet(41);
                break;
            case MG_TYPE_1VS3:
                GWSinglePrizeFlagSet(42);
                break;
            case MG_TYPE_2VS2:
                GWSinglePrizeFlagSet(43);
                break;
            case MG_TYPE_BATTLE:
                GWSinglePrizeFlagSet(44);
                break;
            case MG_TYPE_KETTOU:
                GWSinglePrizeFlagSet(45);
                break;
            }
        }
        listNum = SingleMgListGet(-1, NULL);
        if (listNum == 0) {
            GWSinglePrizeFlagSet(47);
        }
        break;
    }

    case 10:
        storyComDif = GWStoryComDifGet();
        GWSingleMgWinInc(storyComDif);
        if (arg == 6) {
            work->miniKoopaWinFlags |= 1 << miniKoopaType;
        }
        break;

    case 11:
        if (mbSingleStepGet() <= 5 && mbMasuDispCheck(arg)) {
            omVibrate(playerNo, 20, 4, 4);
        }
        break;

    case 12:
        SingleLast5();
        break;
    }
    return 0;
}

static void SingleMicListener(u16 *response)
{
    SINGLE_SAVE_WORK *saveWork = &singleSaveWork;

    if (((SINGLE_MIC_RESPONSE *)response)->status != 0
        || ((SINGLE_MIC_RESPONSE *)response)->resultCount == 0) {
        saveWork->micResult = -1;
    } else {
        saveWork->micResult = (s8)*((SINGLE_MIC_RESPONSE *)response)->result;
        if (saveWork->micResult > 5) {
            saveWork->micResult = -1;
        }
    }
}

static void SingleFlagFlush(void)
{
    int playerNo;
    int bit;
    int boardPlayCount;
    int count;
    int i;
    u32 boardFlag[6];
    const u32 *currentBoardFlag;
    const u32 *oldBoardFlag;
    SINGLE_SAVE_WORK *work;

    playerNo = GwSystem.turnPlayerNo;
    work = &singleSaveWork;
    if (_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        return;
    }

    if (work->mgPlayCount >= 3) {
        if (work->mgEndCount == 0) {
            GWSinglePrizeFlagSet(10);
        }
        if (work->mgEvenCount * 100 / work->mgPlayCount
            >= 75) {
            GWSinglePrizeFlagSet(13);
        }
        if (work->mgOddCount * 100 / work->mgPlayCount
            >= 75) {
            GWSinglePrizeFlagSet(14);
        }
        if ((float)work->mgValueTotal
                / (float)work->mgPlayCount
            >= 5.0f) {
            GWSinglePrizeFlagSet(15);
        }
        if ((float)work->mgValueTotal
                / (float)work->mgPlayCount
            <= 2.0f) {
            GWSinglePrizeFlagSet(16);
        }
        if (work->micUseCount != 0
            && work->micSuccessCount == work->mgPlayCount) {
            GWSinglePrizeFlagSet(19);
        }
        if (work->selectHistory[0] == 0
            && work->selectHistory[1] == 0
            && work->selectHistory[2] == 0) {
            GWSinglePrizeFlagSet(21);
        }
        if (mbPlayerCapsuleNumGet(playerNo) == 3) {
            GWSinglePrizeFlagSet(22);
        }
        if (work->capsulePlayCount == 0
            && work->selectPlayCount != 0) {
            GWSinglePrizeFlagSet(27);
        }
        if (work->masuTypeCount[1] * 100
                / work->mgPlayCount >= 50) {
            GWSinglePrizeFlagSet(33);
        }
        if (work->masuTypeCount[2] * 100
                / work->mgPlayCount >= 50) {
            GWSinglePrizeFlagSet(34);
        }
        if (work->masuTypeCount[4] * 100
                / work->mgPlayCount >= 50) {
            GWSinglePrizeFlagSet(35);
        }
        count = 0;
        for (i = 0; i < 3; i++) {
            count += work->masuTypeCount[9 + i];
        }
        if (count * 100 / work->mgPlayCount >= 50) {
            GWSinglePrizeFlagSet(36);
        }
        if (work->masuTypeCount[3] * 100
                / work->mgPlayCount >= 50) {
            GWSinglePrizeFlagSet(37);
        }
        if (work->masuTypeCount[6] * 100
                / work->mgPlayCount >= 50) {
            GWSinglePrizeFlagSet(38);
        }
    }

    if (work->miniKoopaWinFlags == 7) {
        GWSinglePrizeFlagSet(7);
    }
    if (work->mgEndCount >= 10) {
        GWSinglePrizeFlagSet(9);
    }
    if (work->micSuccessCount != 0) {
        GWSinglePrizeFlagSet(17);
    }
    if (work->micUseCount != 0
        && work->micFirstSuccess == 0) {
        GWSinglePrizeFlagSet(18);
    }
    if (work->mgPlayCount >= 10) {
        GWSinglePrizeFlagSet(20);
    }
    if (work->killerPlayCount != 0) {
        GWSinglePrizeFlagSet(23);
    }
    if (work->capsulePlayCount == 0
        && work->killerPlayCount >= 3) {
        GWSinglePrizeFlagSet(24);
    }
    if (work->capsulePlayCount >= 5) {
        GWSinglePrizeFlagSet(28);
    }
    if (work->capsulePlayCount >= 3) {
        if (work->capsuleTwoF == 0) {
            GWSinglePrizeFlagSet(29);
        } else if (work->capsuleOtherF == 0) {
            GWSinglePrizeFlagSet(30);
        }
    }

    i = 0;
    while (i < 3) {
        if (GwCommon.singleBoardPlayNum[i] == 0) {
            break;
        }
        i++;
    }
    if (GwCommon.singleBoardPlayNum[singleBoard] < 100) {
        GwCommon.singleBoardPlayNum[singleBoard]++;
        if (i < 3) {
            i = 0;
            while (i < 3) {
                if (GwCommon.singleBoardPlayNum[i] == 0) {
                    break;
                }
                i++;
            }
            if (i >= 3) {
                GWSinglePrizeFlagSet(48);
            }
        }
        boardPlayCount = 0;
        for (i = 0; i < 3; i++) {
            boardPlayCount += GwCommon.singleBoardPlayNum[i];
        }
        if (boardPlayCount == 10) {
            GWSinglePrizeFlagSet(49);
        } else if (boardPlayCount == 100) {
            GWSinglePrizeFlagSet(50);
        }
    }

    currentBoardFlag = (const u32 *)GwCommon.singleBoardFlag;
    oldBoardFlag = singleBoardFlagOld;
    i = 0;
    while (i < 6) {
        boardFlag[i] = *currentBoardFlag | *oldBoardFlag;
        i++;
        currentBoardFlag++;
        oldBoardFlag++;
    }
    if (memcmp(boardFlag, GwCommon.singleBoardFlag, sizeof(boardFlag)) != 0) {
        count = 0;
        for (i = 0; i < 6; i++) {
            for (bit = 0; bit < 32; bit++) {
                if (boardFlag[i] & (1u << bit)) {
                    count++;
                }
            }
        }
        if (count >= 71) {
            GWSinglePrizeFlagSet(40);
        }
    }
    memcpy(GwCommon.singleBoardFlag, boardFlag, sizeof(boardFlag));
}

static void SingleMgRecordBackup(void)
{
    int i;

    for (i = 0; i < GW_RECORD_MAX; i++) {
        singleMgRecordOld[i] = GwCommon.record[i];
    }
}

static void SingleMgRecordRestore(void)
{
    int i;

    for (i = 0; i < GW_RECORD_MAX; i++) {
        GwCommon.record[i] = singleMgRecordOld[i];
    }
}

static void SingleMgRecordPrizeInit(void)
{
    int i;

    for (i = 0; i < GW_RECORD_MAX; i++) {
        singleMgRecordPrize[i] = GwCommon.record[i];
    }
}

static void SingleMgRecordPrizeSet(void)
{
    int i;

    for (i = 0; i < GW_RECORD_MAX; i++) {
        if (GwCommon.record[i] != singleMgRecordPrize[i]) {
            break;
        }
    }
    if (i < GW_RECORD_MAX) {
        GWSinglePrizeFlagSet(5);
        GWSingleMgRecordNumSet(GWSingleMgRecordNumGet() + 1);
    }
}

static void SingleLast5(void)
{
    static u32 mesTbl[] = {
        MESSNUM(MESS_MAP_NAME, 6),
        MESSNUM(MESS_MAP_NAME, 7),
        MESSNUM(MESS_MAP_NAME, 8),
        MESSNUM(MESS_MAP_NAME, 8),
    };
    int playerNo;
    s16 winId;
    HuVecF pos;
    OMOBJ *guideObj;

    playerNo = GwSystem.turnPlayerNo;
    mbPlayerMotionSet(playerNo, 1, HU3D_MOTATTR_LOOP);
    mbCameraPlayerViewSetFast(playerNo, 0);
    mbPlayerPosReset(playerNo);
    if (mbWipeSpecialStatGet()) {
        mbWipeFadeIn();
    }
    mbPlayerPosGet(playerNo, &pos);
    pos.y += 100.0f;
    pos.z -= 100.0f;
    guideObj = mbGuideCreateFlag(&pos, guideLast5MotTbl, FALSE, TRUE, FALSE);
    mbGuideMotionNextSet(guideObj, 1);
    winId = mbWinCreate(2, SINGLE_MESS_LAST5_INTRO, mbGuideSpeakerNoGet());
    mbWinTopInsertMesSet(mesTbl[singleBoard], 0);
    mbGuideMotionShiftSet(guideObj, 12, TRUE);
    mbWinWait(winId);
    winId = mbWinCreate(2, SINGLE_MESS_LAST5_RULES, mbGuideSpeakerNoGet());
    mbGuideMotionShiftSet(guideObj, 6, TRUE);
    mbGuideMotionStop(guideObj);
    mbWinWait(winId);
    mbGuideMotionSet(guideObj, 7, TRUE);
    HuPrcSleep(30);
    mbGuideEnd(guideObj, TRUE);
}

void mbSingleReturn(void)
{
    singleEndF = TRUE;
    mbExitReq();
    HuPrcSleep(-1);
}

void mbSingleReturnWrite(void)
{
    singleCancelF = TRUE;
    singleEndF = TRUE;
    mbExitReq();
    HuPrcSleep(-1);
}

void mbSingleGameEnd(void)
{
    int playerNo = GwSystem.turnPlayerNo;
    GAMEMESID mesId;

    mbPauseDisableSet(TRUE);
    mbPlayerMotionSet(playerNo, 1, HU3D_MOTATTR_LOOP);
    mbCameraPlayerViewSetFast(playerNo, 0);
    mbPlayerPosReset(playerNo);

    if (mbWipeSpecialStatGet()) {
        mbWipeFadeIn();
    }

    mesId = GameMesCreate(6, TRUE);
    while (GameMesStatGet(mesId) != 0) {
        HuPrcVSleep();
    }

    mbWipeSpecialCreate(1, 6, 90);
    mbMusFadeOutSpeed(0, 1000);
    mbWipeSpecialWait();
    mbWipeFadeOutTime(1);
    mbWipeSpecialKill();

    singleEndF = TRUE;
    mbExitReq();
    HuPrcSleep(-1);
}

void mbSingleSaveFlush(int value)
{
    int playerNo = GwSystem.turnPlayerNo;

    switch (value) {
    case -1:
        SingleMgRecordRestore();
        if (!_CheckFlag(FLAG_MG_PRACTICE)) {
            GwPlayer[playerNo].mgCoinBonus = -1;
        }
        break;
    case 0:
        if (!_CheckFlag(FLAG_MG_PRACTICE)) {
            GwPlayer[playerNo].mgCoinBonus = 0;
        }
        break;
    case 1:
        mbSingleMgUnlockWrite();
        SingleFlagFlush();
        if (!_CheckFlag(FLAG_MG_PRACTICE)) {
            GwPlayer[playerNo].mgCoinBonus = 1;
        }
        break;
    }
}

int mbSingleStepGet(void)
{
    s16 masuId = GwPlayer[GwSystem.turnPlayerNo].masuId;
    return mbMasuFind_TypeStepGet(masuId, 7);
}

int mbSingleOppCharGet(void)
{
    return miniKoopaType + 11;
}

void mbSingleTeamCharSet(int character)
{
    singleTeamChar = character;
}

int mbSingleTeamCharGet(void)
{
    return singleTeamChar;
}

BOOL mbSingleMgUnlockCheck(void)
{
    return SingleMgListGet(-1, NULL) == 0;
}
