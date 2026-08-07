#include "datadir_enum.h"
#include "game/board/masu.h"
#include "game/board/main.h"
#include "game/board/audio.h"
#include "game/board/coin.h"
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

#include <math.h>
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
extern OMOBJ *mbGuideCreateFlag(HuVecF *pos, s8 *motTbl, BOOL screenF,
    BOOL altMtxF, BOOL layerF);
extern void mbGuideEnd(OMOBJ *obj, BOOL endF);
extern void mbGuideMotionNextSet(OMOBJ *obj, s16 motNo);
extern void mbGuideMotionSet(OMOBJ *obj, s16 motNo, BOOL shiftF);
extern void mbGuideMotionShiftSet(OMOBJ *obj, s16 motNo, BOOL shiftF);
extern void mbGuideMotionStop(OMOBJ *obj);
extern int mbGuideSpeakerNoGet(void);
extern void HuMCListenerKill(void);
extern void HuMCClose(void);
extern void HuMCContextKill(s16 context);
extern s32 HuMCMicGet(void);
extern s32 HuMCProbe(s32 channel);
extern s32 HuMCInit(s16 mountResult);
extern s16 HuMCContextCreate(char *path);
extern void mbSingleSaveFlush(int value);
extern void mbCoinAddExec(int playerNo, int coinNum);
extern BOOL mbWipeSpecialStatGet(void);
extern void mbWipeSpecialCreate(int state, int type, int time);
extern void mbWipeSpecialFadeInCreate(int type, int time);
extern void mbWipeSpecialWait(void);
extern void mbWipeFadeOutTime(int time);
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
static int miniKoopaType;
static int miniKoopaMgType;
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

static void SingleEffInit(void)
{
    SINGLE_EFF_DATA *work;
    int i;

    memset(singleEffData, 0, sizeof(singleEffData));
    work = singleEffData;
    for (i = 0; i < 5; i++, work++) {
        work->modelId = mbParticleCreate(singleEffAnim[0], 1);
        mbParticleHookSet(work->modelId, SingleEffMgMasuHook);
        Hu3DModelCameraSet(work->modelId, 1);
        Hu3DModelLayerSet(work->modelId, 5);
        mbParticleAttrSet(work->modelId, MB_PARTICLE_ATTR_3D);
        Hu3DModelAttrSet(work->modelId, HU3D_ATTR_DISPOFF | HU3D_ATTR_NOPAUSE);
        Hu3DData[work->modelId].hookData = work;

        work->childModelId[0] = mbParticleCreate(singleEffAnim[1], 20);
        mbParticleHookSet(work->childModelId[0], SingleEffMgHook);
        Hu3DModelCameraSet(work->childModelId[0], 1);
        Hu3DModelLayerSet(work->childModelId[0], 5);
        mbParticleAttrSet(work->childModelId[0], MB_PARTICLE_ATTR_STOPCNT);
        Hu3DModelAttrSet(work->childModelId[0], HU3D_ATTR_DISPOFF);
        mbParticleBlendModeSet(work->childModelId[0], MB_PARTICLE_BLEND_ADDCOL);
        Hu3DData[work->childModelId[0]].hookData = work;

        work->childModelId[1] = mbParticleCreate(singleEffAnim[1], 100);
        mbParticleHookSet(work->childModelId[1], SingleEffMgExplodeHook);
        Hu3DModelCameraSet(work->childModelId[1], 1);
        Hu3DModelLayerSet(work->childModelId[1], 5);
        Hu3DModelAttrSet(work->childModelId[1], HU3D_ATTR_DISPOFF);
        mbParticleBlendModeSet(work->childModelId[1], MB_PARTICLE_BLEND_ADDCOL);

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
    extern const float lbl_802C4F40;
    extern const float lbl_802C4F44;
    HU3D_MODELID modelId;
    void *hookData;
    MBPARTICLE *particleWork;
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
    work->unk30 = work->unk34 = work->unk38 = lbl_802C4F40;
    work->scale.x = work->scale.y = work->scale.z = lbl_802C4F44;
    work->unk5C = lbl_802C4F44;
    work->unk58 = lbl_802C4F40;
    work->unk4C = lbl_802C4F44;
    work->unk54 = TRUE;
    Hu3DModelAttrReset(work->modelId, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrReset(work->childModelId[0], HU3D_ATTR_DISPOFF);
    modelId = work->childModelId[0];
    hookData = Hu3DData[modelId].hookData;
    particleWork = hookData;
    particle = particleWork;
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
    static s16 masuPatNo[] = { -1, 0, 0, 1, 2, 2, 6, 7, 3, 5, 8, 9, 10, 11 };
    SINGLE_EFF_DATA *work = particle->hookData;
    MBPARTICLEDATA *data;
    HuVecF rot;
    Mtx transform;
    u8 alpha;

    if (particle->mode == 0) {
        data = particle->data;
        data->pos.x = 0.0f;
        data->pos.y = 0.0f;
        data->pos.z = 0.0f;
        data->scale = 120.0f;
        data->time = 0;
        particle->colorIn[0] = GX_CC_RASC;
        particle->colorIn[1] = GX_CC_TEXC;
        particle->colorIn[2] = GX_CC_C0;
        particle->colorIn[3] = GX_CC_ZERO;
        particle->tevColor[0].r = 255;
        particle->tevColor[0].g = 255;
        particle->tevColor[0].b = 255;
        particle->tevColor[0].a = 255;
        particle->mode = 1;
    }
    data = particle->data;
    mbCameraRotGet(&rot);
    mbMtxRot(transform, rot.x, rot.y, rot.z);
    mtxScaleCat(transform, work->scale.x, work->scale.y, work->scale.z);
    mtxTransCat(transform, work->pos.x, work->pos.y, work->pos.z);
    PSMTXConcat(mtx, transform, mtx);
    data->rot = work->targetPos;
    data->animBank = masuPatNo[work->masuType];
    data->color.a = (u8)(255.0f * work->unk5C);
    alpha = (u8)(255.0f * work->unk58);
    particle->tevColor[0].r = alpha;
    particle->tevColor[0].g = alpha;
    particle->tevColor[0].b = alpha;
    if (!Hu3DPauseF) {
        data->pos.y = work->scale.y * (10.0f * mbSinDeg(4.0f * data->time));
        data->time++;
    }
}

static void SingleEffMgHook(HU3D_MODEL *model, MBPARTICLE *particle, Mtx mtx)
{
    SINGLE_EFF_DATA *work = particle->hookData;
    MBPARTICLEDATA *data;
    GXColor color = { 255, 255, 192, 192 };
    int active = 0;
    int i;

    if (particle->mode == 0) {
        data = particle->data;
        for (i = 0; i < particle->num; i++, data++) {
            data->vel.x = 0.0f;
            data->vel.y = 0.0f;
            data->vel.z = 0.0f;
            data->scale = 0.0f;
            data->weight = (float)mbRandMod(360);
            data->color = color;
            data->time = mbRandMod(30) + 1;
            data->activeF = 30;
        }
        particle->mode = 1;
    }
    data = particle->data;
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
            if (--data->activeF == 0) {
                if (work->unk54) {
                    data->pos.x = work->pos.x + (work->scale.x * (50.0f - (100.0f * frandf())));
                    data->pos.y = work->pos.y + (work->scale.y * (50.0f - (100.0f * frandf())));
                    data->pos.z = work->pos.z + (work->scale.z * (50.0f - (100.0f * frandf())));
                    data->vel.x = 0.0f;
                    data->vel.y = 0.0f;
                    data->vel.z = 0.0f;
                    data->activeF = 30;
                    data->color.a = 255;
                }
            } else if (data->activeF < 20) {
                data->color.a = (u8)(255.0f * ((float)data->activeF / 20.0f));
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
    int active = 0;
    int i;

    if (particle->mode == 0) {
        data = particle->data;
        for (i = 0; i < particle->num; i++, data++) {
            float angle = 360.0f * frandf();
            float speed = 1000.0f + (2000.0f * frandf());

            data->pos.x = 0.0f;
            data->pos.y = 0.0f;
            data->pos.z = 0.0f;
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
                data->color.a = (u8)(255.0f * ((float)data->activeF / 20.0f));
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
    int active = 0;
    int i;

    if (particle->mode == 0) {
        particle->attr |= MB_PARTICLE_ATTR_UPAUSE;
        data = particle->data;
        for (i = 0; i < particle->num; i++, data++) {
            float angle = 360.0f * frandf();
            float speed = 100.0f + (100.0f * frandf());

            data->pos.x = 0.0f;
            data->pos.y = 0.0f;
            data->pos.z = 0.0f;
            data->vel.x = speed * mbCosDeg(angle);
            data->vel.y = speed * mbSinDeg(angle);
            data->vel.z = 0.0f;
            data->scale = 100.0f + (10.0f * frandf());
            data->animSpeed = 0.0f;
            data->animTime = 0.5f;
            data->animBank = 0;
            data->dispF = FALSE;
            data->pauseF = TRUE;
            data->time = mbRandMod(6) + 1;
            data->activeF = (s16)(30.0f + (30.0f * frandf()));
            data->color = color;
        }
        particle->mode = 1;
    }
    data = particle->data;
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
                data->color.a = (u8)(255.0f * ((float)data->activeF / 20.0f));
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
    HuVecF center = { 0.0f, 0.0f, 0.0f };
    HuVecF delta;
    MBPARTICLEDATA *data;
    float speed;
    float angle;
    float height;
    float length;
    float direction;
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

    data = particle->data;
    activeNum = 0;
    for (i = 0; i < particle->num; i++, data++) {
        if (data->dispF) {
            if (data->time < 0) {
                data->time++;
                activeNum++;
                continue;
            }
            if (data->time == 0) {
                data->time++;
                data->rot.x = 360.0f * frandf();
                data->rot.y = 360.0f * frandf();
                data->rot.z = 360.0f * frandf();
                data->color.r = data->color.g = data->color.b =
                    (u8)(160.0f + 95.0f * frandf());
                angle = 360.0f * frandf();
                data->rot.x = mbSinDeg(angle);
                data->rot.z = mbCosDeg(angle);
                direction = 1.0f - mbSinDeg(90.0f * frandf());
                if (mbParticleSRandF() < 0.0f) {
                    direction = -direction;
                }
                data->pos.y = direction;
                height = 1.0f - (direction * direction);
                if (height <= 0.0f) {
                    length = 0.0f;
                } else {
                    length = 1.0f / sqrtf(1.0f + (height * height));
                }
                data->rot.x *= length;
                data->rot.z *= length;
                speed = 2.5f + (100.0f * (0.016666668f
                    * (1.5f * frandf())));
                data->vel.x = data->rot.x * speed;
                data->vel.y = data->pos.y * speed;
                data->vel.z = data->rot.z * speed;
                data->time = 20;
                PSVECScale(&data->pos, &data->pos, 30.0f);
                if (data->pos.y > 0.0f) {
                    data->pos.y *= 1.0f + (3.0f * frandf());
                }
                PSVECAdd(&center, &data->pos, &data->pos);
                data->accel.x = 100.0f + (80.0f * frandf());
                data->accel.y = 1.0f;
                data->color.a = 0;
                data->scale = 30.0f + (30.0f * frandf());
                data->animBank = mbRandMod(4);
                angle = 360.0f * frandf();
                speed = 3.3333335f + (100.0f
                    * (0.016666668f * (2.0f * frandf())));
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

            data->time--;
            if (data->time <= 0) {
                if (particle->count < 30) {
                    data->time = 0;
                    data->scale = 0.0f;
                    data->color.a = 0;
                } else {
                    data->dispF = FALSE;
                }
            } else if (data->time < 8) {
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
    MBPARTICLEDATA *data;
    int active;
    int i;

    if (particle->count == 0) {
        data = particle->data;
        for (i = 0; i < particle->num; i++, data++) {
            data->scale = 0.0f;
            data->color.a = 0;
            data->time = (s16)-(i / 4);
        }
        particle->count = 1;
        particle->blendMode = MB_PARTICLE_BLEND_ADDCOL;
    }

    active = 0;
    data = particle->data;
    for (i = 0; i < particle->num; i++, data++) {
        float angle;
        float speed;
        float fade;

        if (!data->dispF) {
            continue;
        }
        if (data->time < 0) {
            data->time++;
            active++;
            continue;
        }
        if (data->time == 0) {
            angle = 360.0f * frandf();
            speed = 1600.0f + (300.0f * frandf());
            data->time = 1;
            data->color.r = 255;
            data->color.g = 255;
            data->color.b = 255;
            data->color.a = (u8)(128.0f + (127.0f * frandf()));
            data->rot.x = mbSinDeg(angle);
            data->rot.z = mbCosDeg(angle);
            data->rot.y = mbSinDeg(90.0f * frandf());
            data->vel.x = speed * data->rot.x;
            data->vel.y = speed * data->rot.y;
            data->vel.z = speed * data->rot.z;
            data->scale = 30.0f + (40.0f * frandf());
            data->weight = 0.0f;
            data->activeF = 16;
            data->animBank = 0;
            data->animNo = 0;
            data->animSpeed = 0.0f;
            data->animTime = 0.0f;
            data->pauseF = FALSE;
        }

        data->pos.x += (1.0f / 60.0f) * data->vel.x;
        data->pos.y += (1.0f / 60.0f) * data->vel.y;
        data->pos.z += (1.0f / 60.0f) * data->vel.z;
        data->vel.y -= (1.0f / 60.0f) * 980.0f;
        data->weight += 10.0f;
        data->scale *= 0.95f;
        if (data->activeF > 0) {
            data->activeF--;
        }
        if (data->activeF < 20) {
            fade = (float)data->activeF / 20.0f;
            if (fade < 0.0f) {
                fade = 0.0f;
            }
            data->color.a = (u8)(255.0f * fade);
        }
        if (data->activeF != 0) {
            active++;
        } else {
            data->dispF = FALSE;
        }
    }
    if (active == 0) {
        particle->mode = 0;
        Hu3DModelAttrSet(particle->modelId, HU3D_ATTR_DISPOFF);
    }
}

static void SingleEffOMExec(OMOBJ *obj)
{
    SINGLE_EFF_DATA *work;
    HuVecF pos2d;
    HuVecF pos3d;
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
    extern const float lbl_802C4F40;
    SINGLE_EFF_DATA *work;
    int i;

    work = &singleEffData[effNo - 1];
    work->state = 4;
    work->unk34 = lbl_802C4F40;
    work->unk48 = lbl_802C4F40;
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

const float lbl_802C4F40 = 0.0f;
const float lbl_802C4F44 = 1.0f;

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
    HuVecF pos;
    SINGLE_EFF_DATA *work;
    int effNo;
    int masuType;
    int seNo;
    int i;

    masuType = mbMasuTypeGet(masuId);
    mbPlayerPosGet(playerNo, &pos);
    pos.y += 100.0f;
    effNo = SingleEffCreate(&pos, masuType);
    work = &singleEffData[effNo - 1];
    work->state = 1;
    work->targetPos = work->pos;
    work->scale.x = work->scale.y = work->scale.z = 0.0f;
    work->unk4C = 0.0f;
    work->unk48 = 4.0f;
    work->timer = 0;
    work->timerMax = 60;
    if (masuType == 7) {
        mgRareSeNo = mbAudFXPlay(MSM_SE_BRD00_91);
        mbAudFXPlay(MSM_SE_BRD00_92);
    } else {
        seNo = mbAudFXPlay(MSM_SE_SBRD_01);
    }
    while (work->state != 0) {
        HuPrcVSleep();
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
    work->active = 0;
    work->unk04 = FALSE;
    Hu3DModelAttrSet(work->modelId, HU3D_ATTR_DISPOFF);
    for (i = 0; i < 2; i++) {
        Hu3DModelAttrSet(work->childModelId[i], HU3D_ATTR_DISPOFF);
    }
    mbAudFXStop(seNo);
}

static void ev_SingleMgEnd(int playerNo)
{
    HuVecF pos;
    SINGLE_EFF_DATA *work;
    s16 masuId;
    s16 winId;
    int effNo;
    int masuTypeNo;
    int seNo;
    int frame;
    int mgNo;
    int unlockNo;
    BOOL unlocked;

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
    pos.y += 300.0f;
    effNo = SingleEffCreate(&pos, masuTypeNo);
    work = &singleEffData[effNo - 1];
    work->scale.x = work->scale.y = work->scale.z = 1.0f;
    work->unk48 = 4.0f;
    work->unk4C = 1.0f;
    mbMusBoardPlay();
    mbWipeFadeIn();
    mbPauseDisableSet(FALSE);
    for (frame = 0; frame < 60; frame++) {
        if (frame > 21) {
            mbAudFXVolSet(seNo, (s16)((frame - 21) * 127 / 32));
        }
        HuPrcVSleep();
    }

    if ((MgDataTbl[mgNo].type == MG_TYPE_4P
            && GwPlayer[playerNo].mgCoinBonus == 0)
        || (MgDataTbl[mgNo].type != MG_TYPE_4P
            && GwPlayer[playerNo].mgCoin + GwPlayer[playerNo].mgCoinBonus > 0)) {
        mbSingleCall(10, MgDataTbl[mgNo].type);
        mbAudFXStop(seNo);
        work->state = 3;
        work->targetPos = work->pos;
        work->timer = 0;
        work->timerMax = 58;
        Hu3DModelCameraSet(work->modelId, 2);
        Hu3DModelLayerSet(work->modelId, 7);
        Hu3DModelCameraSet(work->childModelId[0], 2);
        Hu3DModelLayerSet(work->childModelId[0], 7);
        Hu3DModelCameraSet(work->childModelId[1], 2);
        Hu3DModelLayerSet(work->childModelId[1], 7);
        mbAudFXPlay(MSM_SE_SBRD_02);
        HuPrcSleep(60);

        unlockNo = mgNo + GW_MGNO_BASE;
        unlocked = GWMgUnlockGet(unlockNo)
            || ((singleMgUnlock[mgNo >> 5] & (1 << (mgNo % 32))) != 0);
        winId = -1;
        if (!unlocked) {
            singleMgUnlock[mgNo >> 5] |= 1 << (mgNo % 32);
            GWSingleMgFlagSet(unlockNo);
            masuType[masuTypeNum++] = (u8)masuTypeNo;
            masuTypeNum %= 5;
            mbSingleCall(9, mgNo);
            winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 0), -1);
            mbWinInsertMesSet(winId, mbPlayerNameMesGet(playerNo), 0);
            mbWinInsertMesSet(winId, MgDataTbl[mgNo].nameMes, 1);
        }
        SingleMgRecordPrizeSet();
        while (work->state != 0) {
            HuPrcVSleep();
        }
        HuPrcSleep(30);
        mbCameraMovePlayer(playerNo, NULL, NULL, 1800.0f, -1.0f, 60);
        if ((MgDataTbl[mgNo].flag & MG_FLAG_COIN) == 0) {
            mbCoinAddExec(playerNo, 10);
        } else {
            mbCoinAddExec(playerNo,
                GwPlayer[playerNo].mgCoin + GwPlayer[playerNo].mgCoinBonus);
        }
        mbPlayerMotionShiftSet(playerNo, 7, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
        mbPlayerWinLoseVoicePlay(playerNo, 7, MSM_SE_CHARVOICE_MARIO + 6);
        while (!mbPlayerMotionEndCheck(playerNo)) {
            HuPrcVSleep();
        }
        if (winId >= 0) {
            mbWinWait(winId);
        }
        HuPrcSleep(60);
    } else {
        mbAudFXStop(seNo);
        work->state = 5;
        work->targetPos = work->pos;
        work->timer = 0;
        work->timerMax = 90;
        work->seId = mbAudFXPlay(MSM_SE_SBRD_04);
        HuPrcSleep(90);
        mbPlayerMotionShiftSet(playerNo, 9, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
        HuPrcSleep(30);
        mbPlayerMotionShiftSet(playerNo, 6, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
        while (work->state != 0) {
            HuPrcVSleep();
        }
        HuPrcSleep(120);
    }
    mbWipeFadeOut();
    work->active = 0;
    work->unk04 = FALSE;
    Hu3DModelAttrSet(work->modelId, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(work->childModelId[0], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(work->childModelId[1], HU3D_ATTR_DISPOFF);
}

static void ev_SingleRareMg(int playerNo, s16 effNo)
{
    static u32 nameMes[] = {
        MG_NAME_M699,
        MG_NAME_M677,
        MG_NAME_M678,
    };
    static HuVecF cameraOfs = { 0.0f, 100.0f, 0.0f };
    HuVecF pos;
    HuVecF cameraRot;
    SINGLE_EFF_DATA *work;
    s16 winId;
    s16 mesId;
    int mgNo;
    int frame;
    int streamNo;
    int unlockNo;
    int i;
    int lockedCount;
    BOOL unlocked;

    mbPauseDisableSet(TRUE);
    mgNo = 0;
    while (MgDataTbl[mgNo].ovl != (u16)-1
        && MgDataTbl[mgNo].nameMes != nameMes[singleBoard]) {
        mgNo++;
    }
    _SetFlag(FLAG_BOARD_STAR_RESET);
    mbMusFadeOutSpeed(MB_MUS_CHAN_BG, 1000);
    mbPlayerMotionSet(playerNo, 11, HU3D_MOTATTR_NONE);
    mbPlayerPosGet(playerNo, &pos);
    for (frame = 0; !mbPlayerMotionEndCheck(playerNo); frame++) {
        if (frame == 12) {
            pos.y += 100.0f;
            work = &singleEffData[effNo - 1];
            work->state = 2;
            work->pos = pos;
            work->targetPos = pos;
            work->unk54 = FALSE;
            work->timer = 0;
            work->timerMax = 60;
        }
        if (frame == 30) {
            if (mgRareSeNo >= 0) {
                mbAudFXStop(mgRareSeNo);
            }
            mbAudFXPlay(MSM_SE_BRD00_93);
        }
        HuPrcVSleep();
    }
    mbPlayerMotIdleSet(playerNo);
    omVibrate(playerNo, 20, 7, 3);
    streamNo = mbMusJinglePlay(39);
    mbPlayerMotionShiftSet(playerNo, 7, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
    mbPlayerWinLoseVoicePlay(playerNo, 7, MSM_SE_CHARVOICE_MARIO);
    mbCameraRotGet(&cameraRot);
    cameraRot.x -= 15.0f;
    mbCameraMovePlayer(playerNo, &cameraRot, &cameraOfs,
        1600.0f, -1.0f, 102);
    mbCameraMoveWait();
    mbCameraFocusObjSet(-1);
    {
        float zoom = mbCameraZoomGet();
        for (frame = 0; frame < 30; frame++) {
            float phase = (float)frame / 30.0f;
            mbCameraZoomSet(zoom - (500.0f
                * (0.5f * (1.0f + sinf(720.0f * phase)))));
            HuPrcVSleep();
        }
    }

    unlockNo = mgNo + GW_MGNO_BASE;
    unlocked = GWMgUnlockGet(unlockNo)
        || ((singleMgUnlock[mgNo >> 5] & (1 << (mgNo % 32))) != 0);
    winId = -1;
    if (!unlocked) {
        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 0), -1);
        mbWinInsertMesSet(winId, mbPlayerNameMesGet(playerNo), 0);
        mbWinInsertMesSet(winId, MgDataTbl[mgNo].nameMes, 1);
        singleMgUnlock[mgNo >> 5] |= 1 << (mgNo % 32);
        GWSingleMgFlagSet(unlockNo);
        masuType[masuTypeNum++] = 7;
        masuTypeNum %= 5;
        GWSinglePrizeFlagSet(8);
        lockedCount = 0;
        for (i = 0; MgDataTbl[i].ovl != (u16)-1; i++) {
            if ((MgDataTbl[i].flag & MG_FLAG_RARE)
                && !GWMgUnlockGet(i + GW_MGNO_BASE)
                && !(singleMgUnlock[i >> 5] & (1 << (i % 32)))) {
                lockedCount++;
            }
        }
        if (lockedCount == 0) {
            GWSinglePrizeFlagSet(46);
        }
    } else {
        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 1), -1);
        mbWinInsertMesSet(winId, mbPlayerNameMesGet(playerNo), 0);
        mbCoinAddProcExec(playerNo, 50, TRUE, TRUE);
    }
    mbMusJingleWait(streamNo);
    if (winId >= 0) {
        mbWinWait(winId);
    }
    mesId = GameMesCreate(6, TRUE);
    while (GameMesStatGet(mesId) != 0) {
        HuPrcVSleep();
    }
    mbWipeSpecialCreate(1, 6, 90);
    mbWipeSpecialWait();
    mbWipeFadeOutTime(1);
    mbWipeSpecialKill();
    GwSystem.turnNo++;
    mbSingleReturn();
}

static void ev_SingleKoopaMg(int playerNo, s16 masuId)
{
    static int guideMot[] = {
        DATANUM(DATA_bsingle, 1),
        DATANUM(DATA_bsingle, 5),
        DATANUM(DATA_bsingle, 3),
        DATANUM(DATA_bsingle, 7),
        DATANUM(DATA_bsingle, 4),
        DATANUM(DATA_bsingle, 10),
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
    MBMODELID capsuleObj;
    HU3D_MODELID particleCapsule;
    HU3D_MODELID particleExplode;
    HU3D_MODELID particleIds[2];
    s16 playerMotion[2];
    s16 spriteId[2];
    s16 masuStart;
    HuVecF masuPos;
    HuVecF cameraTarget;
    HuVecF pos2d;
    HuVecF pos3d;
    float phase;
    float remaining;
    float angle;
    float scale;
    float wave;
    int frame;
    int i;
    int j;
    int choice;
    int capsuleNo;
    int unlockCount;

    (void)masuId;
    capsuleObj = -1;
    particleIds[0] = -1;
    particleIds[1] = -1;
    modelId = mbObjCreate(DATA_bsingle, guideMot, FALSE);
    mbObjDispSet(modelId, FALSE);
    mbPlayerRotateStart(playerNo, 0, 15);
    playerMotion[0] = mbPlayerMotionCreate(playerNo, CHARMOT_HSF_c000m1_323);
    playerMotion[1] = mbPlayerMotionCreate(playerNo, CHARMOT_HSF_c000m1_325);

    spriteId[0] = espEntry(DATANUM(DATA_bsingle, 36), 100, 0);
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
        HU3D_MOTATTR_LOOP);
    mbCameraPlayerViewSet(playerNo, 0);
    mbEffFadeCreate(30, 160);
    espDispOn(spriteId[0]);
    for (frame = 0; frame <= 180; frame++) {
        phase = (float)frame / 180.0f;
        if (frame == 30) {
            mbAudFXPlay(MSM_SE_BRD00_103);
        }
        if (frame == 18 || frame == 90 || frame == 162) {
            omVibrate(playerNo, 20, 7, 3);
        }
        espTPLvlSet(spriteId[0], fabsf(mbSinDeg(360.0f * phase)));
        HuPrcVSleep();
    }

    mbWipeCreate(WIPE_MODE_OUT, WIPE_TYPE_SUN | WIPE_TYPE_FBKEEP, 60);
    mbWipeSpecialWait();
    espDispOff(spriteId[0]);
    mbEffFadeOutSet(30);

    masuStart = mbMasuFind_AttrIdGet(MASU_NULL, MASU_FLAG_START);
    mbMasuPosGet(masuStart, &masuPos);
    cameraTarget = masuPos;
    cameraTarget.y += 150.0f;
    mbCameraMovePos(&cameraTarget, &defCameraRot, NULL, 500.0f, -1.0f, -1);
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
    spriteId[1] = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 2), 13);
    mbWinWait(spriteId[1]);
    mbObjMotionShiftSet(modelId, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    spriteId[1] = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 3), 13);
    mbWinWait(spriteId[1]);
    mbAudFXDelaySet(30);
    mbAudFXPlay(MSM_SE_GUIDE_49);
    mbObjMotionShiftSet(modelId, 5, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
    while (!mbObjMotionEndCheck(modelId)) {
        HuPrcVSleep();
    }

    unlockCount = 0;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 32; j++) {
            if (singleMgUnlock[i] & (1u << j)) {
                unlockCount++;
            }
        }
    }
    if (unlockCount == 0 && mbPlayerCoinGet(playerNo) == 0
        && mbPlayerCapsuleNumGet(playerNo) == 0) {
        returnMode = 0;
    } else {
        returnMode = 1;
    }

    spriteId[1] = espEntry(mbBoardDataNumGet(DATANUM(DATA_board, 142)),
        100, (s16)(returnMode ^ 1));
    espDrawNoSet(spriteId[1], 32);
    for (frame = 1; frame < 60; frame++) {
        phase = (float)frame / 60.0f;
        if (frame == 27) {
            mbAudFXPlay(MSM_SE_BRD00_113);
        }
        wave = mbSinDeg(90.0f * phase);
        angle = 50.0f * mbCosDeg(180.0f * phase);
        espPosSet(spriteId[1], 288.0f,
            240.0f - (250.0f * mbSinDeg(180.0f * phase)) + angle);
        espScaleSet(spriteId[1], wave, wave);
        espZRotSet(spriteId[1], 3.0f * (360.0f * phase));
        HuPrcVSleep();
    }
    omVibrate(playerNo, 20, 7, 3);
    for (frame = 1; frame < 60; frame++) {
        phase = (float)frame / 60.0f;
        scale = 1.0f + (0.2f * mbSinDeg(720.0f * phase));
        espPosSet(spriteId[1], 288.0f, 240.0f);
        espScaleSet(spriteId[1], scale, scale);
        espZRotSet(spriteId[1], 0.0f);
        HuPrcVSleep();
    }
    for (frame = 1; frame < 18; frame++) {
        phase = (float)frame / 18.0f;
        scale = 1.0f + (5.0f * mbSinDeg(90.0f * phase));
        espPosSet(spriteId[1], 288.0f, 240.0f);
        espScaleSet(spriteId[1], scale, scale);
        espTPLvlSet(spriteId[1], 1.0f - mbSinDeg(90.0f * phase));
        HuPrcVSleep();
    }
    espDispOff(spriteId[1]);
    mbAudFXDelaySet(30);
    mbAudFXPlay(MSM_SE_GUIDE_47);
    mbObjMotionShiftSet(modelId, 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);

    if (returnMode == 0) {
        spriteId[1] = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 4), 13);
        mbWinInsertMesSet(spriteId[1], mbPlayerNameMesGet(playerNo), 0);
        mbWinWait(spriteId[1]);
        choice = mbRandMod(100) < 50 ? 0 : 1;
        spriteId[1] = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 5), 13);
        mbWinInsertMesSet(spriteId[1],
            MESSNUM(MESS_BOARD_SINGLE, 39 + choice), 0);
        mbWinWait(spriteId[1]);
        mbAudFXDelaySet(30);
        mbAudFXPlay(MSM_SE_GUIDE_49);
        mbObjMotionShiftSet(modelId, 5, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
        while (!mbObjMotionEndCheck(modelId)) {
            HuPrcVSleep();
        }

        capsuleNo = mgKoopaCapsuleTbl[choice];
        capsuleObj = mbCapObjCreate(capsuleNo, FALSE);
        mbObjDispSet(capsuleObj, FALSE);
        mbObjCameraSet(capsuleObj, 2);
        mbObjLayerSet(capsuleObj, 7);
        mbObjAttrSet(capsuleObj, HU3D_MOTATTR_LOOP);
        mbObjMotionSpeedSet(capsuleObj, 0.0f);
        mbObjPosGet(modelId, &masuPos);
        masuPos.y += 200.0f;
        masuPos.z += 200.0f;

        particleCapsule = mbParticleCreate(singleEffAnim[2], 100);
        particleIds[0] = particleCapsule;
        mbParticleHookSet(particleCapsule, SingleEffMgCapsuleHook);
        Hu3DModelCameraSet(particleCapsule, 1);
        Hu3DModelLayerSet(particleCapsule, 5);
        cameraTarget = masuPos;
        cameraTarget.y += 80.0f;
        cameraTarget.z += 100.0f;
        Hu3DModelPosSetV(particleCapsule, &cameraTarget);
        Hu3DModelCameraSet(particleCapsule, 2);
        Hu3DModelLayerSet(particleCapsule, 7);
        Hu3DModelAttrSet(particleCapsule, HU3D_ATTR_DISPOFF);

        particleExplode = mbParticleCreate(singleEffAnim[1], 100);
        particleIds[1] = particleExplode;
        mbParticleHookSet(particleExplode, SingleEffMgExplodeHook);
        Hu3DModelCameraSet(particleExplode, 1);
        Hu3DModelLayerSet(particleExplode, 5);
        Hu3DModelPosSetV(particleExplode, &masuPos);
        mbParticleBlendModeSet(particleExplode, MB_PARTICLE_BLEND_ADDCOL);
        Hu3DModelCameraSet(particleExplode, 2);
        Hu3DModelLayerSet(particleExplode, 7);
        Hu3DModelAttrSet(particleExplode, HU3D_ATTR_DISPOFF);
        mbAudFXPlay(MSM_SE_BRD00_59);

        for (choice = 0; choice < 3; choice++) {
            Hu3DModelAttrReset(particleCapsule, HU3D_ATTR_DISPOFF);
            mbAudFXPlay(MSM_SE_SBRD_06);
            HuPrcSleep(12);
            mbObjPosSetV(capsuleObj, &masuPos);
            mbObjScaleSet(capsuleObj, 1.0f, 1.0f, 1.0f);
            mbObjDispSet(capsuleObj, TRUE);
            HuPrcSleep(30);
            for (frame = 0; frame <= 60; frame++) {
                phase = (float)frame / 60.0f;
                remaining = 1.0f - phase;
                angle = 1080.0f * phase;
                mbPos3Dto2D(&masuPos, &pos2d);
                pos2d.x = 114.0f + (128.0f * remaining);
                pos2d.y = 80.0f;
                pos2d.x += remaining * (-128.0f * mbCosDeg(-angle));
                pos2d.y += remaining * (128.0f * mbSinDeg(-angle));
                mbPos2Dto3D(&pos2d, &pos3d);
                wave = mbSinDeg(90.0f * phase);
                pos3d.x = masuPos.x + wave * (pos3d.x - masuPos.x);
                pos3d.y = masuPos.y + wave * (pos3d.y - masuPos.y);
                pos3d.z = masuPos.z + wave * (pos3d.z - masuPos.z);
                mbObjPosSetV(capsuleObj, &pos3d);
                scale = 0.2f + (0.8f * remaining);
                mbObjScaleSet(capsuleObj, scale, scale, scale);
                HuPrcVSleep();
            }
            Hu3DModelPosSetV(particleExplode, &pos3d);
            Hu3DModelAttrReset(particleExplode, HU3D_ATTR_DISPOFF);
            mbObjDispSet(capsuleObj, FALSE);
            mbPlayerCapsuleAdd(playerNo, capsuleNo);
            omVibrate(playerNo, 20, 7, 3);
            HuPrcSleep(30);
        }
        mbObjDispSet(capsuleObj, FALSE);
        mbAudFXDelaySet(30);
        mbAudFXPlay(MSM_SE_GUIDE_47);
        mbObjMotionShiftSet(modelId, 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        HuPrcSleep(60);
        spriteId[1] = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 6), 13);
        mbWinWait(spriteId[1]);
        ev_SingleKoopaMgSkip(modelId);
        mbPlayerRotYSet(playerNo, 0.0f);
        mbPlayerPosReset(playerNo);
        mbCameraPlayerViewSetFast(playerNo, 0);
    } else {
        spriteId[1] = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 8), 13);
        mbWinWait(spriteId[1]);
        mbObjMotionShiftSet(modelId, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        spriteId[1] = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 9), 13);
        mbWinInsertMesSet(spriteId[1], mbPlayerNameMesGet(playerNo), 0);
        mbWinWait(spriteId[1]);
        mbAudFXDelaySet(30);
        mbAudFXPlay(MSM_SE_GUIDE_49);
        mbObjMotionShiftSet(modelId, 5, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
        if (mbPlayerCoinGet(playerNo) == 0) {
            if (unlockCount > 0) {
                spriteId[1] = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 10), 13);
                mbWinWait(spriteId[1]);
                returnMode = 0;
            } else {
                spriteId[1] = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 12), 13);
                mbWinInsertMesSet(spriteId[1], mbPlayerNameMesGet(playerNo), 0);
                mbWinWait(spriteId[1]);
                returnMode = 1;
            }
        } else if (unlockCount > 0 && mbRandMod(100) < 50) {
            spriteId[1] = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 10), 13);
            mbWinWait(spriteId[1]);
            returnMode = 0;
        } else {
            spriteId[1] = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 11), 13);
            mbWinInsertMesSet(spriteId[1], mbPlayerNameMesGet(playerNo), 0);
            mbWinWait(spriteId[1]);
            returnMode = 2;
        }
    }

    mbObjMotionShiftSet(modelId, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    spriteId[1] = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 13), 13);
    mbWinWait(spriteId[1]);
    i = mbRandMod(3);
    miniKoopaMgType = mgTypeTbl[i][0];
    _SetFlag(FLAG_BOARD_MG_KOOPA);
    mbev_MgCallSingleKoopa(mgTypeTbl[i][1], TRUE);

    mbObjKill(modelId);
    if (capsuleObj >= 0) {
        mbCapObjKill(capsuleObj);
    }
    for (i = 0; i < 2; i++) {
        espKill(spriteId[i]);
        mbPlayerMotionKill(playerNo, playerMotion[i]);
        if (particleIds[i] >= 0) {
            mbParticleKill(particleIds[i]);
        }
    }
    HuDataDirClose(DATA_bsingle);
    if (mbWipeSpecialStatGet()) {
        mbWipeFadeIn();
    }
}

static int seLoseTbl[] = {
    675, 627, 603,
    676, 628, 604,
    677, 629, 605,
    681, 633, 609,
    -1, -1, -1,
};
static HuVecF viewOfs750 = { 0.0f, 100.0f, 0.0f };
static HuVecF viewOfs825 = { 0.0f, 100.0f, 0.0f };

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

static void ev_SingleKoopaMgEnd(int playerNo)
{
    int guideMot[] = {
        DATANUM(DATA_bsingle, 1),
        DATANUM(DATA_bsingle, 5),
        DATANUM(DATA_bsingle, 3),
        DATANUM(DATA_bsingle, 7),
        DATANUM(DATA_bsingle, 4),
        DATANUM(DATA_bsingle, 10),
        -1,
    };
    HuVecF startPos;
    HuVecF cameraOffset = { 0.0f, 100.0f, 100.0f };
    HuVecF playerPos;
    HuVecF opponentPos;
    HuVecF delta;
    SINGLE_EFF_DATA *work;
    MBMODELID guideModel;
    MBMODELID capsuleObj[3];
    HU3D_MODELID particles[10];
    s16 effects[5];
    s16 masuStart;
    s16 winId;
    s16 seNo;
    int playerMgCoin;
    int playerMgBonus;
    int mgNo;
    int resultMode;
    int unlockNo;
    int unlockedCount;
    int capsuleCount;
    int effectCount;
    int i;
    int frame;
    BOOL unlocked;
    float angle;

    for (i = 0; i < 3; i++) {
        capsuleObj[i] = -1;
    }
    for (i = 0; i < 10; i++) {
        particles[i] = -1;
    }
    for (i = 0; i < 5; i++) {
        effects[i] = -1;
    }

    if (!mbWipeSpecialStatGet()) {
        mbWipeFadeOut();
    }
    mbStatusDispForceSet(playerNo, TRUE);
    mgNo = GwSystem.mgNo;
    mbPlayerRotYSet(playerNo, 0.0f);
    guideModel = mbObjCreate(DATA_bsingle, guideMot, FALSE);
    masuStart = mbMasuFind_AttrIdGet(MASU_NULL, MASU_FLAG_START);
    mbMasuPosGet(masuStart, &startPos);
    mbObjPosSetV(guideModel, &startPos);
    mbObjDispSet(guideModel, TRUE);
    mbObjMotionSet(guideModel, 1, HU3D_MOTATTR_LOOP);
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
    mbPlayerPosSet(playerNo, startPos.x, startPos.y, startPos.z + 300.0f);
    mbPlayerMotionSet(playerNo, 1, HU3D_MOTATTR_LOOP);
    mbPlayerRotYSet(playerNo, 180.0f);
    mbCameraMoveMasu(masuStart, NULL, &cameraOffset, -1.0f, 1600.0f, -1);
    mbCameraMoveWait();
    mbMusPlay(MB_MUS_CHAN_BG, 28, MSM_VOL_MAX, 0);

    playerMgCoin = GwPlayer[playerNo].mgCoin;
    playerMgBonus = GwPlayer[playerNo].mgCoinBonus;
    if (playerMgCoin + playerMgBonus > 0) {
        resultMode = 0;
    } else {
        resultMode = 2;
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (i != playerNo
                && GwPlayer[i].mgCoin + GwPlayer[i].mgCoinBonus > 0) {
                resultMode = 1;
                break;
            }
        }
    }

    if (resultMode == 0) {
        mbSingleCall(10, MgDataTbl[mgNo].type);
        mbAudFXDelaySet(30);
        mbAudFXPlay(MSM_SE_GUIDE_48);
        mbObjMotionSet(guideModel, 6, HU3D_MOTATTR_LOOP);
        mbWipeFadeIn();
        mbPauseDisableSet(FALSE);
        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 14), 13);
        mbWinWait(winId);
        mbAudFXDelaySet(30);
        mbAudFXPlay(MSM_SE_GUIDE_48);
        mbObjMotionShiftSet(guideModel, 6, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
        unlockNo = mgNo + GW_MGNO_BASE;
        unlocked = GWMgUnlockGet(unlockNo) || mbSingleMgUnlockGet(unlockNo);
        if (unlocked) {
            winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 42), 13);
        } else {
            winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 15), 13);
        }
        mbWinWait(winId);
        mbAudFXDelaySet(30);
        mbAudFXPlay(MSM_SE_GUIDE_49);
        mbObjMotionShiftSet(guideModel, 5, 0.0f, 8.0f,
            HU3D_MOTATTR_NONE);
        while (!mbObjMotionEndCheck(guideModel)) {
            HuPrcVSleep();
        }

        startPos.y += 200.0f;
        startPos.z += 200.0f;
        particles[0] = mbParticleCreate(singleEffAnim[2], 100);
        if (particles[0] >= 0) {
            mbParticleHookSet(particles[0], SingleEffMgCapsuleHook);
            Hu3DModelCameraSet(particles[0], 1);
            Hu3DModelLayerSet(particles[0], 5);
            Hu3DModelPosSet(particles[0], startPos.x, startPos.y + 80.0f,
                startPos.z + 100.0f);
            Hu3DModelCameraSet(particles[0], 2);
            Hu3DModelLayerSet(particles[0], 7);
        }
        mbAudFXPlay(MSM_SE_SBRD_06);
        HuPrcSleep(12);
        mbAudFXPlay(MSM_SE_BRD00_59);
        effects[0] = SingleEffCreate(&startPos, miniKoopaMgType);
        HuPrcSleep(30);
        if (effects[0] > 0) {
            work = &singleEffData[effects[0] - 1];
            work->state = 3;
            work->targetPos = work->pos;
            work->timer = 0;
            work->timerMax = 58;
            Hu3DModelCameraSet(work->modelId, 2);
            Hu3DModelLayerSet(work->modelId, 7);
            Hu3DModelCameraSet(work->childModelId[0], 2);
            Hu3DModelLayerSet(work->childModelId[0], 7);
            Hu3DModelCameraSet(work->childModelId[1], 2);
            Hu3DModelLayerSet(work->childModelId[1], 7);
            mbAudFXPlay(MSM_SE_SBRD_02);
            HuPrcSleep(60);
            if (!unlocked) {
                mbSingleMgUnlockSet(mgNo);
                GWSingleMgFlagSet(unlockNo);
                masuType[masuTypeNum++] = (u8)miniKoopaMgType;
                masuTypeNum %= 5;
                mbSingleCall(9, mgNo);
            }
            SingleMgRecordPrizeSet();
            while (work->state != 0) {
                HuPrcVSleep();
            }
        }
        HuPrcSleep(30);
        if (MgDataTbl[mgNo].flag & MG_FLAG_COIN) {
            mbCoinAddExec(playerNo, playerMgCoin + playerMgBonus);
        } else {
            mbCoinAddExec(playerNo, 10);
        }
        mbPlayerMotionShiftSet(playerNo, 7, 0.0f, 8.0f,
            HU3D_MOTATTR_NONE);
        mbPlayerWinLoseVoicePlay(playerNo, 7, 579);
        if (!unlocked) {
            winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 0), -1);
            mbWinInsertMesSet(winId, mbPlayerNameMesGet(playerNo), 0);
            mbWinInsertMesSet(winId, MgDataTbl[mgNo].nameMes, 1);
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
    } else if (resultMode == 1) {
        mbAudFXDelaySet(30);
        mbAudFXPlay(MSM_SE_GUIDE_47);
        mbObjMotionSet(guideModel, 3, HU3D_MOTATTR_LOOP);
        mbWipeFadeIn();
        mbPauseDisableSet(FALSE);
        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 16), 13);
        mbWinInsertMesSet(winId, mbPlayerNameMesGet(playerNo), 0);
        mbWinWait(winId);
        if (returnMode == 0) {
            unlockedCount = mbSingleMgUnlockNumGet();
            if (unlockedCount > 0) {
                mbWinWait(mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 17),
                    13));
                effectCount = unlockedCount;
                if (effectCount > 5) {
                    effectCount = 5;
                }
                startPos.x -= (float)(100 * (effectCount - 1)) * 0.75f;
                startPos.y += 200.0f;
                startPos.z += 200.0f;
                for (i = 0; i < effectCount; i++) {
                    effects[i] = SingleEffCreate(&startPos, masuType[i]);
                    particles[i] = mbParticleCreate(singleEffAnim[2], 100);
                    if (particles[i] >= 0) {
                        mbParticleHookSet(particles[i], SingleEffMgCapsuleHook);
                        Hu3DModelCameraSet(particles[i], 1);
                        Hu3DModelLayerSet(particles[i], 5);
                        Hu3DModelPosSet(particles[i], startPos.x,
                            startPos.y + 80.0f, startPos.z + 100.0f);
                        Hu3DModelCameraSet(particles[i], 2);
                        Hu3DModelLayerSet(particles[i], 7);
                    }
                    startPos.x += 150.0f;
                }
                mbAudFXPlay(MSM_SE_SBRD_06);
                HuPrcSleep(12);
                mbAudFXPlay(MSM_SE_SBRD_01);
                mbObjAttrReset(guideModel, HU3D_MOTATTR_LOOP);
                while (!mbObjMotionEndCheck(guideModel)) {
                    HuPrcVSleep();
                }
                mbAudFXDelaySet(30);
                mbAudFXPlay(MSM_SE_GUIDE_49);
                mbObjMotionShiftSet(guideModel, 5, 0.0f, 8.0f,
                    HU3D_MOTATTR_NONE);
                for (i = 0; i < effectCount; i++) {
                    if (effects[i] > 0) {
                        work = &singleEffData[effects[i] - 1];
                        work->unk04 = FALSE;
                    }
                }
                mbAudFXPlay(MSM_SE_BRD00_59);
                HuPrcSleep(60);
                memset(singleMgUnlock, 0, sizeof(singleMgUnlock));
                memset(masuType, 0, sizeof(masuType));
                masuTypeNum = 0;
                memset(GwSingleMgFlag, 0, sizeof(GwSingleMgFlag));
                mbSinglePrizeFlagReset(5);
                mbSinglePrizeFlagReset(6);
                GWSingleMgWinNumSet(0);
                GWSingleMgRecordNumSet(0);
                SingleMgRecordRestore();
            }
        } else if (returnMode == 1) {
            capsuleCount = mbPlayerCapsuleNumGet(playerNo);
            mbWinWait(mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 19), 13));
            for (i = 0; i < capsuleCount && i < 3; i++) {
                capsuleObj[i] = mbCapObjCreate(mbPlayerCapsuleGet(playerNo,
                    i), FALSE);
                if (capsuleObj[i] >= 0) {
                    mbObjDispSet(capsuleObj[i], TRUE);
                    mbObjCameraSet(capsuleObj[i], 1);
                    mbObjLayerSet(capsuleObj[i], 4);
                }
                mbPlayerCapsuleRemove(playerNo, 0);
            }
            mbAudFXPlay(MSM_SE_SBRD_06);
            HuPrcSleep(30);
        } else {
            mbWinWait(mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 18), 13));
            mbAudFXDelaySet(30);
            mbAudFXPlay(MSM_SE_GUIDE_49);
            mbObjMotionShiftSet(guideModel, 5, 0.0f, 8.0f,
                HU3D_MOTATTR_NONE);
            mbCoinAddProcExec(playerNo, -mbPlayerCoinGet(playerNo), -1, TRUE);
            HuPrcSleep(30);
        }
        while (!mbPlayerMotionEndCheck(playerNo)) {
            HuPrcVSleep();
        }
        mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
    } else {
        mbAudFXDelaySet(30);
        mbAudFXPlay(MSM_SE_GUIDE_47);
        mbObjMotionSet(guideModel, 3, HU3D_MOTATTR_LOOP);
        mbWipeFadeIn();
        mbPauseDisableSet(FALSE);
        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 20), 13);
        mbWinInsertMesSet(winId, mbPlayerNameMesGet(playerNo), 0);
        mbWinWait(winId);
        mbWinWait(mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 21), 13));
    }

    winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 7), 13);
    mbWinWait(winId);
    mbWipeSpecialFadeInCreate(7, 30);
    mbWipeSpecialWait();
    mbWipeFadeOutTime(1);
    mbWipeSpecialKill();
    mbObjDispSet(guideModel, FALSE);
    mbMusBoardPlay();
    mbPlayerPosReset(playerNo);
    mbCameraPlayerViewSetFast(playerNo, 0);
    mbObjKill(guideModel);
    for (i = 0; i < 10; i++) {
        if (particles[i] >= 0) {
            mbParticleKill(particles[i]);
        }
    }
    for (i = 0; i < 3; i++) {
        if (capsuleObj[i] >= 0) {
            mbCapObjKill(capsuleObj[i]);
        }
    }
    for (i = 0; i < 5; i++) {
        if (effects[i] > 0) {
            work = &singleEffData[effects[i] - 1];
            work->active = 0;
            work->unk04 = FALSE;
            Hu3DModelAttrSet(work->modelId, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(work->childModelId[0], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(work->childModelId[1], HU3D_ATTR_DISPOFF);
        }
    }
    HuDataDirClose(DATA_bsingle);
}

static void ev_SingleMKoopaMg(int playerNo, s16 masuId)
{
    HuVecF masuPos;
    HuVecF playerPos;
    HuVecF opponentPos;
    HuVecF delta;
    float angle;
    int opponentPlayerNo;
    int characterNo;
    int winId;
    int i;
    BOOL lockedKettou;
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
    playerMotion = mbPlayerMotionCreate(playerNo, CHARMOT_HSF_c000m1_467);
    opponentMotion = mbPlayerMotionCreate(opponentPlayerNo,
        CHARMOT_HSF_c000m1_467);

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

    mbCameraMoveMasu(masuId, NULL, &viewOfs750, -1.0f, 1600.0f, -1);
    mbCameraMoveWait();
    mbMusPlay(MB_MUS_CHAN_BG, 26, MSM_VOL_MAX, 0);
    mbWipeSpecialFadeOutCreate(8, 30);
    mbWipeSpecialWait();

    winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 22), characterNo);
    mbAudFXPlay(MSM_SE_BRD00_91);
    mbWinInsertMesSet(winId, mbPlayerNameMesGet(playerNo), 0);
    mbWinWait(winId);

    lockedKettou = FALSE;
    for (i = 0; MgDataTbl[i].ovl != (u16)-1; i++) {
        if (MgDataTbl[i].type == MG_TYPE_KETTOU
            && !GWMgUnlockGet(i + GW_MGNO_BASE)
            && !mbSingleMgUnlockGet(i + GW_MGNO_BASE)) {
            lockedKettou = TRUE;
            break;
        }
    }

    if (!lockedKettou) {
        if (mbPlayerCoinGet(playerNo) == 0) {
            mbWinWait(mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 23),
                characterNo));
            winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 24),
                characterNo);
            mbAudFXPlay(MSM_SE_BRD00_91);
            mbWinWait(winId);
            returnMode = 0;
        } else {
            mbWinWait(mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 23),
                characterNo));
            winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 26),
                characterNo);
            mbAudFXPlay(MSM_SE_BRD00_91);
            mbWinWait(winId);
            returnMode = 1;
        }
    } else if (mbPlayerCoinGet(playerNo) != 0) {
        mbWinWait(mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 28),
            characterNo));
        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 26),
            characterNo);
        mbAudFXPlay(MSM_SE_BRD00_91);
        mbWinWait(winId);
        returnMode = 2;
    } else {
        mbWinWait(mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 28),
            characterNo));
        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 24),
            characterNo);
        mbAudFXPlay(MSM_SE_BRD00_91);
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
    mbAudFXPlay(MSM_SE_BRD00_91);
    mbWinWait(winId);
    _SetFlag(FLAG_BOARD_MG_KOOPA);
    mbev_MgCallSingle(6);
    mbPlayerMotionKill(playerNo, playerMotion);
    mbPlayerMotionKill(opponentPlayerNo, opponentMotion);
    mbPlayerDispSet(opponentPlayerNo, FALSE);
    GwPlayer[opponentPlayerNo].masuId = 0;
}

static void ev_SingleMKoopaMgEnd(int playerNo)
{
    HuVecF masuPos;
    HuVecF playerPos;
    HuVecF opponentPos;
    HuVecF delta;
    SINGLE_EFF_DATA *work;
    int mgNo;
    int masuTypeNo;
    int opponentPlayerNo;
    int characterNo;
    int totalMgCoin;
    int unlockNo;
    BOOL unlocked;
    BOOL newUnlock;
    s16 masuId;
    s16 winId;
    s16 particleId;
    s16 seNo;
    s16 effNo;
    float angle;

    particleId = -1;
    effNo = 0;
    mgNo = GwSystem.mgNo;
    masuId = GwPlayer[playerNo].masuId;
    masuTypeNo = mbMasuTypeGet(masuId);

    if (!mbWipeSpecialStatGet()) {
        mbWipeFadeOut();
    }
    mbStatusDispForceSet(playerNo, TRUE);
    mbCameraMoveMasu(masuId, NULL, &viewOfs825, -1.0f, 1600.0f, -1);
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

    totalMgCoin = GwPlayer[playerNo].mgCoin + GwPlayer[playerNo].mgCoinBonus;
    if (totalMgCoin > 0) {
        mbSingleCall(10, 6);
        mbPlayerMotionSet(opponentPlayerNo, 6, HU3D_MOTATTR_LOOP);
        mbWipeFadeIn();
        mbPauseDisableSet(FALSE);
        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 31), characterNo);
        mbAudFXPlay(seLoseTbl[3 * 3 + miniKoopaType]);
        mbWinWait(winId);

        if (returnMode < 2) {
            if (returnMode >= 0) {
                mbWinWait(mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 33),
                    characterNo));
                masuPos.y += 200.0f;
                masuPos.z += 200.0f;
                particleId = mbParticleCreate(singleEffAnim[2], 100);
                if (particleId >= 0) {
                    mbParticleHookSet(particleId, SingleEffMgCapsuleHook);
                    Hu3DModelCameraSet(particleId, 1);
                    Hu3DModelLayerSet(particleId, 5);
                    Hu3DModelPosSet(particleId, masuPos.x,
                        masuPos.y + 80.0f, masuPos.z + 100.0f);
                    Hu3DModelCameraSet(particleId, 2);
                    Hu3DModelLayerSet(particleId, 7);
                }
                HuPrcSleep(12);
                effNo = SingleEffCreate(&masuPos, masuTypeNo);
                seNo = mbAudFXPlay(MSM_SE_SBRD_01);
                HuPrcSleep(30);
                mbAudFXStop(seNo);
                if (effNo > 0) {
                    work = &singleEffData[effNo - 1];
                    work->state = 3;
                    work->targetPos = work->pos;
                    work->timer = 0;
                    work->timerMax = 58;
                    Hu3DModelCameraSet(work->modelId, 2);
                    Hu3DModelLayerSet(work->modelId, 7);
                    Hu3DModelCameraSet(work->childModelId[0], 2);
                    Hu3DModelLayerSet(work->childModelId[0], 7);
                    Hu3DModelCameraSet(work->childModelId[1], 2);
                    Hu3DModelLayerSet(work->childModelId[1], 7);
                    mbAudFXPlay(MSM_SE_SBRD_02);
                    HuPrcSleep(60);
                    unlockNo = mgNo + GW_MGNO_BASE;
                    unlocked = GWMgUnlockGet(unlockNo)
                        || mbSingleMgUnlockGet(unlockNo);
                    newUnlock = !unlocked;
                    if (newUnlock) {
                        mbSingleMgUnlockSet(mgNo);
                        GWSingleMgFlagSet(unlockNo);
                        masuType[masuTypeNum++] = (u8)(miniKoopaType + 9);
                        masuTypeNum %= 5;
                        mbSingleCall(9, mgNo);
                    }
                    while (work->state != 0) {
                        HuPrcVSleep();
                    }
                    HuPrcSleep(30);
                }
                mbCoinAddExec(playerNo, 10);
                mbPlayerMotionShiftSet(playerNo, 7, 0.0f, 8.0f,
                    HU3D_MOTATTR_NONE);
                mbPlayerWinLoseVoicePlay(playerNo, 7, 579);
                if (newUnlock) {
                    winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 0), -1);
                    mbWinInsertMesSet(winId, mbPlayerNameMesGet(playerNo), 0);
                    mbWinInsertMesSet(winId, MgDataTbl[mgNo].nameMes, 1);
                    mbWinWait(winId);
                }
            }
        } else if (returnMode < 4) {
            mbWinWait(mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 36),
                characterNo));
            mbPlayerMotionShiftSet(playerNo, 7, 0.0f, 8.0f,
                HU3D_MOTATTR_NONE);
            mbPlayerWinLoseVoicePlay(playerNo, 7, 579);
            mbCoinAddProcExec(playerNo, 10, TRUE, TRUE);
        }
        SingleMgRecordPrizeSet();
        while (!mbPlayerMotionEndCheck(playerNo)) {
            HuPrcVSleep();
        }
        HuPrcSleep(30);
        mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
    } else if (mbPlayerCoinGet(playerNo) > 0) {
        mbPlayerMotionSet(playerNo, 6, HU3D_MOTATTR_LOOP);
        mbPlayerMotionSet(opponentPlayerNo, 7, HU3D_MOTATTR_NONE);
        mbWipeFadeIn();
        mbPauseDisableSet(FALSE);
        mbWinWait(mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 30),
            characterNo));
        if (returnMode == 1 || returnMode == 2) {
            mbWinWait(mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 38),
                characterNo));
            mbCoinAddProcExec(playerNo,
                -(mbPlayerCoinGet(playerNo) + 1) / 2, -1, TRUE);
        } else {
            mbWinWait(mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 37),
                characterNo));
        }
        mbPlayerPosGet(playerNo, &playerPos);
        mbPlayerPosGet(opponentPlayerNo, &opponentPos);
        PSVECSubtract(&opponentPos, &playerPos, &delta);
        angle = (float)(180.0 * (atan2(delta.x, delta.z) / M_PI));
        mbPlayerRotateStart(playerNo, (s16)angle, 15);
        mbPlayerRotateStart(opponentPlayerNo, (s16)(180.0f + angle), 15);
        HuPrcSleep(30);
        winId = mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 32), characterNo);
        mbAudFXPlay(seLoseTbl[1 * 3 + miniKoopaType]);
        mbWinWait(winId);
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
        mbWinWait(mbWinCreate(2, MESSNUM(MESS_BOARD_SINGLE, 43),
            characterNo));
    }

    mbWipeSpecialFadeInCreate(8, 30);
    mbWipeSpecialWait();
    mbWipeFadeOutTime(1);
    mbWipeSpecialKill();
    mbPlayerDispSet(opponentPlayerNo, FALSE);
    GwPlayer[opponentPlayerNo].masuId = 0;
    if (effNo > 0) {
        work = &singleEffData[effNo - 1];
        work->active = 0;
        work->unk04 = FALSE;
        Hu3DModelAttrSet(work->modelId, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(work->childModelId[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(work->childModelId[1], HU3D_ATTR_DISPOFF);
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

static inline BOOL SingleMgUnlockedCheck(int unlockMgNo)
{
    BOOL unlocked;

    if (GWMgUnlockGet(unlockMgNo)
        || mbSingleMgUnlockGet(unlockMgNo)) {
        unlocked = TRUE;
    } else {
        unlocked = FALSE;
    }
    return unlocked;
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
    int firstEmptyBoard;
    int boardNo;
    int bit;
    int count;
    u32 boardFlag[6];
    const u32 *currentBoardFlag;

    if (_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        return;
    }

    playerNo = GwSystem.turnPlayerNo;
    if (singleSaveWork.mgPlayCount >= 3) {
        if (singleSaveWork.mgEndCount == 0) {
            GWSinglePrizeFlagSet(10);
        }
        if (singleSaveWork.mgEvenCount * 100 / singleSaveWork.mgPlayCount
            >= 75) {
            GWSinglePrizeFlagSet(13);
        }
        if (singleSaveWork.mgOddCount * 100 / singleSaveWork.mgPlayCount
            >= 75) {
            GWSinglePrizeFlagSet(14);
        }
        if ((float)singleSaveWork.mgValueTotal
                / (float)singleSaveWork.mgPlayCount
            >= 5.0f) {
            GWSinglePrizeFlagSet(15);
        }
        if ((float)singleSaveWork.mgValueTotal
                / (float)singleSaveWork.mgPlayCount
            <= 2.0f) {
            GWSinglePrizeFlagSet(16);
        }
        if (singleSaveWork.micUseCount != 0
            && singleSaveWork.micSuccessCount == singleSaveWork.micUseCount) {
            GWSinglePrizeFlagSet(19);
        }
        if (singleSaveWork.mgHistory[0] == 0
            && singleSaveWork.mgHistory[1] == 0
            && singleSaveWork.mgHistory[2] == 0) {
            GWSinglePrizeFlagSet(21);
        }
        if (mbPlayerCapsuleNumGet(playerNo) == 3) {
            GWSinglePrizeFlagSet(22);
        }
        if (singleSaveWork.selectPlayCount == 0
            && singleSaveWork.selectHistoryNo != 0) {
            GWSinglePrizeFlagSet(27);
        }
        if (singleSaveWork.masuTypeCount[1] * 100
                / singleSaveWork.mgPlayCount >= 50) {
            GWSinglePrizeFlagSet(33);
        }
        if (singleSaveWork.masuTypeCount[2] * 100
                / singleSaveWork.mgPlayCount >= 50) {
            GWSinglePrizeFlagSet(34);
        }
        if (singleSaveWork.masuTypeCount[4] * 100
                / singleSaveWork.mgPlayCount >= 50) {
            GWSinglePrizeFlagSet(35);
        }
        count = singleSaveWork.masuTypeCount[9]
            + singleSaveWork.masuTypeCount[10]
            + singleSaveWork.masuTypeCount[11];
        if (count * 100 / singleSaveWork.mgPlayCount >= 50) {
            GWSinglePrizeFlagSet(36);
        }
        if (singleSaveWork.masuTypeCount[3] * 100
                / singleSaveWork.mgPlayCount >= 50) {
            GWSinglePrizeFlagSet(37);
        }
        if (singleSaveWork.masuTypeCount[6] * 100
                / singleSaveWork.mgPlayCount >= 50) {
            GWSinglePrizeFlagSet(38);
        }
    }

    if (singleSaveWork.miniKoopaWinFlags == 7) {
        GWSinglePrizeFlagSet(7);
    }
    if (singleSaveWork.mgEndCount >= 10) {
        GWSinglePrizeFlagSet(9);
    }
    if (singleSaveWork.micSuccessCount != 0) {
        GWSinglePrizeFlagSet(17);
    }
    if (singleSaveWork.micUseCount != 0
        && singleSaveWork.micFirstSuccess == 0) {
        GWSinglePrizeFlagSet(18);
    }
    if (singleSaveWork.mgPlayCount >= 10) {
        GWSinglePrizeFlagSet(20);
    }
    if (singleSaveWork.selectPlayCount != 0) {
        GWSinglePrizeFlagSet(23);
    }
    if (singleSaveWork.mgPlayCount == 0
        && singleSaveWork.selectPlayCount >= 3) {
        GWSinglePrizeFlagSet(24);
    }
    if (singleSaveWork.selectPlayCount >= 5) {
        GWSinglePrizeFlagSet(28);
    }
    if (singleSaveWork.selectPlayCount >= 3) {
        if (singleSaveWork.capsuleOtherF == 0) {
            GWSinglePrizeFlagSet(29);
        }
        if (singleSaveWork.capsuleTwoF == 0) {
            GWSinglePrizeFlagSet(30);
        }
    }

    firstEmptyBoard = 0;
    while (firstEmptyBoard < 3
        && GwCommon.singleBoardPlayNum[firstEmptyBoard] != 0) {
        firstEmptyBoard++;
    }
    if (GwCommon.singleBoardPlayNum[singleBoard] < 100) {
        GwCommon.singleBoardPlayNum[singleBoard]++;
        if (firstEmptyBoard >= 3) {
            GWSinglePrizeFlagSet(48);
        }
    }
    count = 0;
    for (boardNo = 0; boardNo < 3; boardNo++) {
        count += GwCommon.singleBoardPlayNum[boardNo];
    }
    if (count == 10) {
        GWSinglePrizeFlagSet(49);
    } else if (count == 100) {
        GWSinglePrizeFlagSet(50);
    }

    currentBoardFlag = (const u32 *)GwCommon.singleBoardFlag;
    for (boardNo = 0; boardNo < 6; boardNo++) {
        boardFlag[boardNo] = currentBoardFlag[boardNo]
            | singleBoardFlagOld[boardNo];
    }
    if (memcmp(boardFlag, GwCommon.singleBoardFlag, sizeof(boardFlag)) != 0) {
        count = 0;
        for (boardNo = 0; boardNo < 6; boardNo++) {
            for (bit = 0; bit < 32; bit++) {
                if (boardFlag[boardNo] & (1u << bit)) {
                    count++;
                }
            }
        }
        if (count >= 71) {
            GWSinglePrizeFlagSet(40);
        }
        memcpy(GwCommon.singleBoardFlag, boardFlag, sizeof(boardFlag));
    }
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
    extern const float lbl_802C4F50;
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
    pos.y += lbl_802C4F50;
    pos.z -= lbl_802C4F50;
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

const float lbl_802C4F50 = 100.0f;

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
