#include "game/board/masu.h"
#include "game/board/main.h"
#include "game/board/audio.h"
#include "game/board/camera.h"
#include "game/board/effect.h"
#include "game/board/pause.h"
#include "game/board/player.h"
#include "game/board/object.h"
#include "game/board/window.h"
#include "game/gamework.h"
#include "game/flag.h"
#include "game/gamemes.h"
#include "game/hu3d.h"
#include "game/mgdata.h"
#include "game/sprite.h"

#include <string.h>

extern void mbExitReq(void);
extern void HuMCListenerKill(void);
extern void HuMCClose(void);
extern void HuMCContextKill(s16 context);
extern void mbSingleSaveFlush(int value);
extern BOOL mbWipeSpecialStatGet(void);
extern void mbWipeSpecialCreate(int state, int type, int time);
extern void mbWipeSpecialFadeInCreate(int type, int time);
extern void mbWipeSpecialWait(void);
extern void mbWipeFadeOutTime(int time);
extern void mbWipeSpecialKill(void);

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
static ANIMDATA *singleEffAnim[4];

static int singleTeamChar = -1;
static int mgKoopaCapsuleTbl[] = { 2, 7 };
static u8 guideLast5MotTbl[] = { 12, 6, 0xFF };

static int singleBoard;
static int singleCancelF;
static int singleEndF;
static s16 singleMicContext;
static int singleMicF;
static int singleListenerCreateF;
static int singleListenerOnF;
static int singleMasuOrderNum;
static u8 singleMasuOrder[0x100][2];
static u8 masuType[5];
static u8 masuTypeNum;
static int returnMode;
static int mgRareSeNo;
static int miniKoopaMgType;
static u32 singleBoardFlagOld[6];
static u32 singleMgRecordOld[GW_RECORD_MAX];
static u32 singleMgRecordPrize[GW_RECORD_MAX];
static SINGLE_EFF_DATA singleEffData[5];

static void SingleMicKill(void);
static void SingleMicListenerKill(void);
static void SingleEffClose(void);
static void SingleEffKill(s16 effNo);
static void SingleMasuTypeReset(void);
static void SingleMasuOrderSet(void);
static void SingleMgRecordBackup(void);

void mbSingleMgUnlockInit(void);
void mbSingleTeamCharSet(int character);
int mbSingleTeamCharGet(void);

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

static void SingleMicListenerKill(void)
{
    if (!singleMicF || !singleListenerCreateF) {
        return;
    } else {
        HuMCListenerKill();
        singleListenerCreateF = FALSE;
    }
}

static void SingleMasuOrderSet(void)
{
    int i;

    for (i = 0; i < singleMasuOrderNum; i++) {
        mbMasuTypeSet(singleMasuOrder[i][0], singleMasuOrder[i][1]);
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

static void ev_SingleKoopaMgSkip(MBMODELID modelId)
{
    s16 winId;

    winId = mbWinCreate(2, 0x002B0007, 13);
    mbWinWait(winId);
    mbWipeSpecialFadeInCreate(7, 30);
    mbWipeSpecialWait();
    mbWipeFadeOutTime(1);
    mbWipeSpecialKill();
    mbObjDispSet(modelId, FALSE);
    mbMusBoardPlay();
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
static int miniKoopaType;

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
                GWMgUnlockSet((word << 5) + bit + GW_MGNO_BASE);
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

void mbSinglePrizeFlagReset(int flag)
{
    if (flag <= 63) {
        GwSinglePrizeFlag[flag >> 5] &= ~(1 << (flag & 0x1F));
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
