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
static SINGLE_SAVE_WORK singleSaveWork;
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
static void SingleMgRecordPrizeInit(void);
static void SingleMicCreate(void);
static void SingleMicListener(u16 *response);
static void SingleEffInit(void);
static void SingleMasuOrderInit(void);
static void SingleMgSaveInit(void);
static void SingleFlagFlush(void);
static void SingleLast5(void);
static void ev_SingleMg(int playerNo, s16 masuId);
static void ev_SingleKoopaMg(int playerNo, s16 masuId);
static void ev_SingleMKoopaMg(int playerNo, s16 masuId);
static void ev_SingleMgEnd(int playerNo);
static void ev_SingleKoopaMgEnd(int playerNo);
static void ev_SingleMKoopaMgEnd(int playerNo);

extern void HuMCListenerCreate(
    s16 context, void (*callback)(u16 *response), u8 property);

void mbSingleMgUnlockInit(void);
void mbSingleTeamCharSet(int character);
int mbSingleTeamCharGet(void);
int mbSingleCall(int mode, int arg);

void mbSingleInit(void)
{
    static int effFile[] = { 0x000A0000, 0x00050063, 0x0005005E, 0x00050066 };
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
    HuDataDirClose(0x000A0000);
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
    if (_CheckFlag(0x10002)) {
        ev_SingleMgEnd(playerNo);
        _ClearFlag(0x10002);
    } else if (_CheckFlag(0x10003)) {
        ev_SingleKoopaMgEnd(playerNo);
        _ClearFlag(0x10003);
    } else if (_CheckFlag(0x10005)) {
        ev_SingleMKoopaMgEnd(playerNo);
        _ClearFlag(0x10005);
    }
    return TRUE;
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

int mbSingleCall(int mode, int arg)
{
    SINGLE_SAVE_WORK *work = &singleSaveWork;
    int playerNo = GwSystem.turnPlayerNo;
    int candidates[6];
    int candidateNum;
    int historyNo;
    int mgType;
    int mgNo;
    int i;
    BOOL unlocked;

    if (GwSystem.partyF || _CheckFlag(0x1000E)) {
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

    case 2:
        if (singleMicF && singleListenerCreateF) {
            HuMCListenerKill();
            singleListenerCreateF = FALSE;
        }
        singleListenerOnF = FALSE;
        if (work->micResult < 0
            || (GwPlayer[playerNo].capsule[0] & 0x3F) != 0) {
            return -1;
        }
        if (work->micUseCount < 99) {
            work->micUseCount++;
        }
        arg = work->micResult;
        if (mbRandMod(100) < 50) {
            candidateNum = 0;
            for (i = 0; i < 6; i++) {
                if (i != work->micResult) {
                    candidates[candidateNum++] = i;
                }
            }
            arg = candidates[mbRandMod(candidateNum)];
        }
        return arg;

    case 3:
        if (work->mgPlayCount < 99) {
            work->mgPlayCount++;
        }
        work->mgHistory[work->mgHistoryNo] = arg;
        if (!GWSinglePrizeFlagGet(12)) {
            historyNo = work->mgHistoryNo;
            for (i = 0; i < 2; i++) {
                historyNo--;
                if (historyNo < 0) {
                    historyNo = 2;
                }
                if (work->mgHistory[historyNo] != arg) {
                    break;
                }
            }
            if (i >= 2) {
                GWSinglePrizeFlagSet(12);
                GwSinglePrizeFlag[0] &= ~(1 << 11);
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
        if (++work->mgHistoryNo > 2) {
            work->mgHistoryNo = 0;
        }
        if (work->micResult < 0) {
            work->micFirstSuccess = TRUE;
        } else if ((u32)(work->micResult + 1) == (u32)arg) {
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
            historyNo = work->selectHistoryNo;
            for (i = 0; i < 2; i++) {
                historyNo--;
                if (historyNo < 0) {
                    historyNo = 2;
                }
                if (work->selectHistory[historyNo] != arg) {
                    break;
                }
            }
            if (i >= 2) {
                GWSinglePrizeFlagSet(26);
                GwSinglePrizeFlag[0] &= ~(1 << 25);
            } else if (i == 1) {
                GWSinglePrizeFlagSet(25);
            }
        }
        if (++work->selectHistoryNo > 2) {
            work->selectHistoryNo = 0;
        }
        break;

    case 6:
        if (work->killerPlayCount < 99) {
            work->killerPlayCount++;
        }
        break;

    case 7:
        if (mbMasuDispCheck(arg)) {
            mgNo = arg - 1;
            singleBoardFlagOld[(singleBoard * 2) + (mgNo >> 5)]
                |= 1 << (mgNo & 0x1F);
        }
        mgType = mbMasuTypeGet(arg);
        if (mgType == 7) {
            GWSinglePrizeFlagSet(39);
        }
        if (work->masuTypeCount[mgType] < 99) {
            work->masuTypeCount[mgType]++;
        }
        break;

    case 8:
        if (work->mgEndCount < 99) {
            work->mgEndCount++;
        }
        break;

    case 9:
        GWSinglePrizeFlagSet(6);
        GWSingleMgWinNumSet(GWSingleMgWinNumGet() + 1);
        mgType = MgDataTbl[arg].type;
        candidateNum = 0;
        for (mgNo = 0; MgDataTbl[mgNo].ovl != (u16)-1; mgNo++) {
            if (MgDataTbl[mgNo].type != mgType
                || MgDataTbl[mgNo].type == MG_TYPE_KUPA
                || MgDataTbl[mgNo].type == MG_TYPE_DONKEY
                || (!(MgDataTbl[mgNo].flag & MG_FLAG_RARE)
                    && !mbMgCallSingleOnCheck(MgDataTbl[mgNo].ovl))
                || MgDataTbl[mgNo].dataDir == 0x0005004C) {
                continue;
            }
            unlocked = GWMgUnlockGet(mgNo + GW_MGNO_BASE)
                || ((singleMgUnlock[mgNo >> 5]
                    & (1 << (mgNo & 0x1F))) != 0);
            if (!unlocked) {
                candidateNum++;
            }
        }
        if (candidateNum == 0) {
            if (mgType == MG_TYPE_4P) {
                GWSinglePrizeFlagSet(41);
            } else if (mgType == MG_TYPE_1VS3) {
                GWSinglePrizeFlagSet(42);
            } else if (mgType == MG_TYPE_2VS2) {
                GWSinglePrizeFlagSet(43);
            } else if (mgType == MG_TYPE_BATTLE) {
                GWSinglePrizeFlagSet(44);
            } else if (mgType == MG_TYPE_KETTOU) {
                GWSinglePrizeFlagSet(45);
            }
        }
        candidateNum = 0;
        for (mgNo = 0; MgDataTbl[mgNo].ovl != (u16)-1; mgNo++) {
            if (MgDataTbl[mgNo].type == MG_TYPE_KUPA
                || MgDataTbl[mgNo].type == MG_TYPE_DONKEY
                || (!(MgDataTbl[mgNo].flag & MG_FLAG_RARE)
                    && !mbMgCallSingleOnCheck(MgDataTbl[mgNo].ovl))
                || MgDataTbl[mgNo].dataDir == 0x0005004C) {
                continue;
            }
            unlocked = GWMgUnlockGet(mgNo + GW_MGNO_BASE)
                || ((singleMgUnlock[mgNo >> 5]
                    & (1 << (mgNo & 0x1F))) != 0);
            if (!unlocked) {
                candidateNum++;
            }
        }
        if (candidateNum == 0) {
            GWSinglePrizeFlagSet(47);
        }
        break;

    case 10:
        if (GwCommon.singleMgWinNum[GwSystem.storyComDif] < 99) {
            GwCommon.singleMgWinNum[GwSystem.storyComDif]++;
        }
        if (arg == 6) {
            work->miniKoopaWinFlags |= 1 << miniKoopaType;
        }
        break;

    case 11:
        if (mbSingleStepGet() < 6 && mbMasuDispCheck(arg)) {
            omVibrate(playerNo, 20, 4, 4);
        }
        break;

    case 12:
        SingleLast5();
        break;
    }
    return 0;
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
