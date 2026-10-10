// Board player setup, turn flow, movement and player-state helpers.
#include "dolphin/math.h"
#include "dolphin/os.h"

#include "game/board/audio.h"
#include "game/board/branch.h"
#include "game/board/camera.h"
#include "game/board/coin.h"
#include "game/board/effect.h"
#include "game/board/gate.h"
#include "game/board/main.h"
#include "game/board/masu.h"
#include "game/board/object.h"
#include "game/board/pause.h"
#include "game/board/player.h"
#include "game/board/status.h"
#include "game/board/tutorial.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/frand.h"
#include "game/process.h"
#include "game/msm.h"

#include "messdir_enum.h"
#include "msm_se.h"

#include "string.h"

enum {
    MESS_CHARANAME_MARIO,
    MESS_CHARANAME_LUIGI,
    MESS_CHARANAME_PEACH,
    MESS_CHARANAME_YOSHI,
    MESS_CHARANAME_WARIO,
    MESS_CHARANAME_DAISY,
    MESS_CHARANAME_WALUIGI,
    MESS_CHARANAME_KINOPIO,
    MESS_CHARANAME_TERESA,
    MESS_CHARANAME_MINIKOOPA,
    MESS_CHARANAME_KINOPICO,
    MESS_CHARANAME_MINIKOOPAR,
    MESS_CHARANAME_MINIKOOPAG,
    MESS_CHARANAME_MINIKOOPAB
};

enum {
    PLAYER_OBJ_PRIORITY = 256,
    PLAYER_MOVE_NUM_OBJ_PRIORITY = 32258,
    PLAYER_MOVE_PROCESS_PRIORITY = 8205,
    PLAYER_MOVE_PROCESS_STACK_SIZE = 24576,
    PLAYER_MOVE_COUNT_SFX = MSM_SE_BRD00_02,
    PLAYER_COIN_GAIN_SFX = MSM_SE_BRD00_97,
    PLAYER_COIN_LOSS_SFX = MSM_SE_BRD00_98
};

#define FLAG_BOARD_WALKDONE FLAGNUM(FLAG_GROUP_COMMON, 16)

static MBPLAYERWORK playerWork[GW_PLAYER_MAX];
static BOOL turnIntrF;
static BOOL blackoutF;
static void (*turnInitHook)(int playerNo);
static void (*turnCloseHook)(int playerNo);
static GXColor metalShadowColor;
static GXColor metalHiliteColor;
static BOOL playerColSnapF;

typedef struct PlayerColWork {
    // Set when the collision object begins its movement animation.
    u8 motStartF : 1;
    // Requests recalculation of the collision marker's position on the next eligible update.
    u8 resyncF : 1;
    // Requests snapping the player to its board-space position.
    u8 snapF : 1;
    // Stored TRUE when this player is marked as not resting.
    u8 notRestF : 1;
    // Player index represented by this collision object.
    u8 playerNo : 2;
    // Collision transition stage: 0 settled, 1 moving to a corner, 2 turning to yaw zero.
    u8 state : 2;
    // Enables circular collision placement around a shared space.
    u8 circleF;
    // Current board space used by the collision object.
    u8 masuId;
    // Destination board space used by the collision object.
    u8 masuIdNext;
    // Elapsed collision-animation frames.
    s8 time;
    // Total collision-animation frames.
    s8 maxTime;
    // Alignment padding in the collision object's work record.
    u8 _pad06[2];
    // Starting yaw used while rotating around a space.
    float rotYStart;
    // Radius of the circular placement around a space.
    float radius;
} PLAYERCOLWORK;

static GXColor metalDefaultColor[2] = {
    { 128, 190, 140, 255 },
    { 100, 50, 130, 255 }
};

#define CHAR_MDLFILE(name) DATA_##name##mdl1
#define CHAR_MOTDIR(name) DATA_##name##mot

static const int charMdlFileTbl[CHARNO_MAX] = {
    CHAR_MDLFILE(mario), CHAR_MDLFILE(luigi), CHAR_MDLFILE(peach), CHAR_MDLFILE(yoshi),
    CHAR_MDLFILE(wario), CHAR_MDLFILE(daisy), CHAR_MDLFILE(waluigi), CHAR_MDLFILE(kinopio),
    CHAR_MDLFILE(teresa), CHAR_MDLFILE(minikoopa), CHAR_MDLFILE(kinopiko), CHAR_MDLFILE(minikoopaR),
    CHAR_MDLFILE(minikoopaG), CHAR_MDLFILE(minikoopaB)
};

static const int charMotDirTbl[CHARNO_MAX] = {
    CHAR_MOTDIR(mario), CHAR_MOTDIR(luigi), CHAR_MOTDIR(peach), CHAR_MOTDIR(yoshi),
    CHAR_MOTDIR(wario), CHAR_MOTDIR(daisy), CHAR_MOTDIR(waluigi), CHAR_MOTDIR(kinopio),
    CHAR_MOTDIR(teresa), CHAR_MOTDIR(minikoopa), CHAR_MOTDIR(kinopiko), CHAR_MOTDIR(minikoopa),
    CHAR_MOTDIR(minikoopa), CHAR_MOTDIR(minikoopa)
};

static const u16 charMotNoTbl[15] = {
    CHAR_MOTNO(CHARMOT_HSF_c000m1_300), CHAR_MOTNO(CHARMOT_HSF_c000m1_301),
    CHAR_MOTNO(CHARMOT_HSF_c000m1_302), CHAR_MOTNO(CHARMOT_HSF_c000m1_303),
    CHAR_MOTNO(CHARMOT_HSF_c000m1_304), CHAR_MOTNO(CHARMOT_HSF_c000m1_322),
    CHAR_MOTNO(CHARMOT_HSF_c000m1_306), CHAR_MOTNO(CHARMOT_HSF_c000m1_307),
    CHAR_MOTNO(CHARMOT_HSF_c000m1_324), CHAR_MOTNO(CHARMOT_HSF_c000m1_357),
    CHAR_MOTNO(CHARMOT_HSF_c000m1_311), CHAR_MOTNO(CHARMOT_HSF_c000m1_346),
    CHAR_MOTNO(CHARMOT_HSF_c000m1_348), CHAR_MOTNO(CHARMOT_HSF_c000m1_386),
    CHAR_MOTNO(CHARMOT_HSF_c000m1_320)
};

static const int tutorialCharNoTbl[GW_PLAYER_MAX] = {
    CHARNO_YOSHI,
    CHARNO_MARIO,
    CHARNO_PEACH,
    CHARNO_WARIO
};

static const int singleCharNoTbl[GW_PLAYER_MAX] = {
    CHARNO_MARIO,
    CHARNO_MINIKOOPAR,
    CHARNO_MINIKOOPAG,
    CHARNO_MINIKOOPAB
};

#undef CHAR_MDLFILE
#undef CHAR_MOTDIR

static void PlayerColKill(int playerNo);
static void PlayerColOMExec(OMOBJ *obj);
static void PlayerMetalKill(int playerNo);
static void PlayerMetalOMExec(OMOBJ *objP);
static void ResetMetalColor(void);
static void MetalEffectCreate(OMOBJ *objP);
static void MetalEffectHook(
    HU3D_MODEL *modelP, MBPARTICLE *particleP, Mtx matrix);
static void PlayerBiriQKill(int playerNo);
static void PlayerBiriQFlashSet(int playerNo);
static void PlayerBiriQOMExec(OMOBJ *objP);
// Scales a player or board vector component by component.
static inline void HuVecMul(
    register HuVecF *srcP, register HuVecF *scaleP,
    register HuVecF *dstP)
{
    register float srcXY;
    register float scaleXY;
    register float srcZ;
    register float scaleZ;

    asm {
        psq_l srcXY, 0(srcP), 0, 0
        psq_l scaleXY, 0(scaleP), 0, 0
        psq_l srcZ, 8(srcP), 1, 0
        psq_l scaleZ, 8(scaleP), 1, 0
        ps_mul srcXY, srcXY, scaleXY
        ps_mul srcZ, srcZ, scaleZ
        psq_st srcXY, 0(dstP), 0, 0
        psq_st srcZ, 8(dstP), 1, 0
    }
}

// Copies a three-component board position or rotation vector.
static inline void HuVecCopy(
    register HuVecF *srcP, register HuVecF *dstP)
{
    register float xy;
    register float z;

    asm {
        psq_l xy, 0(srcP), 0, 0
        lfs z, 8(srcP)
        psq_st xy, 0(dstP), 0, 0
        stfs z, 8(dstP)
    }
}

static float GetBiriQEffectRadius(
    OMOBJ *objP, int playerNo, int *meshGroupInfo);
static void BiriQEffectCreate(OMOBJ *objP);
static void BiriQEffect1Hook(
    HU3D_MODEL *modelP, MBPARTICLE *particleP, Mtx matrix);
static void BiriQEffect2Hook(
    HU3D_MODEL *modelP, MBPARTICLE *particleP, Mtx matrix);
static void PlayerMove(void);
static void PlayerMoveCall(int playerNo);
static void PlayerMoveDestroy(void);
static void PlayerTurn(int playerNo);
static BOOL DiceRun(int playerNo);
static BOOL PlayerViewSet(
    int playerNo, BOOL intrF, BOOL waitF, BOOL carF);
static void MasuCoinExec(int playerNo, int coinNum);
static void ev_PlayerStartTurn(int playerNo);
static void ev_PlayerEndTurn(int playerNo);
int mbSingleTeamCharGet(void);
int mbCapSelect(void);
int mbSingleCall(int mode, int arg);
int mbDiceProcExec(int playerNo, int diceType, s8 *valueTbl,
    int *tutorialVal, BOOL padWinF, BOOL waitF, HuVecF *pos, int color);
int mbDiceExec(int playerNo, int diceType, s8 *valueTbl, int tutorialVal,
    BOOL padWinF, BOOL waitF, HuVecF *pos, int color);
void mbDiceKill(int playerNo);
int mbDiceMaxGet(int diceType);
int mbDiceValueMaxGet(int diceType);
void mbev_Scroll(int playerNo, BOOL mapF);
void mbev_CapKillerMoveCall(int playerNo);
void mbev_CapCallKettou(int playerNo, s16 id, BOOL stopF);
void mbev_CapCallTrap(int playerNo, s16 id, s16 idNext);
void mbev_CapBiriQShockCreate(int playerNo);
void mbStarDispSetAll(BOOL dispF);
void mbStarMasuDispSet(int masuId, BOOL dispF);
void mbTelopTimeCreate(void);
void mbTelopPlayerCreate(int playerNo);
s16 mbCoinDispMasuCreate(HuVecF *pos, int coinNum, BOOL playSe);
void mbCoinAddExec(int playerNo, int coinNum);
void mbDiceNumKill(int playerNo);
void mbDiceObjHit(int playerNo);
void mbObjMetalCreate(MBMODELID modelId);
void mbObjMetalKill(MBMODELID modelId);
void mbObjMetalTPLvlSet(MBMODELID modelId, float level);
void mbObjMetalColorSet(
    MBMODELID modelId, GXColor shadowColor, GXColor hiliteColor);
void mbObjBiriQCreate(MBMODELID modelId);
void mbObjBiriQKill(MBMODELID modelId);
void mbObjBiriQColorSet(
    MBMODELID modelId, BOOL enableF, float level, GXColor color);
BOOL mbWipeSpecialStatGet(void);
void mbWipeFadeIn(void);
void mbWipeFadeOut(void);
void mbWipeDissolveFadeIn(void);
void mbWipeSpecialFadeOutCreate(int type, int time);
void mbWipeSpecialFadeInCreate(int type, int time);
BOOL mbPauseEnableCheck(void);
void mbPos3DtoNorm(HuVecF *src, s16 cameraMask, HuVecF *dst);
float mbSinDeg(float angle);
float mbAngleEaseOut(float angleStart, float angleEnd, float weight);

// Creates player models and collision objects when the board initializes.
// With noEventF true, resets player setup, spaces, capsules and playerMode.
void mbPlayerInit(BOOL noEventF)
{
    MBPLAYERWORK *playerWorkP = &playerWork[0];
    int characterMotionData[20];
    int playerIndex;
    int entryIndex;

    memset(playerWork, 0, sizeof(playerWork));
    ResetMetalColor();
    if (_CheckFlag(FLAG_BOARD_MG)) {
        GwSystem.turnPlayerNo = 0;
    }
    if (noEventF) {
        int startSpaceId = mbMasuFind_AttrIdGet(-1, MASU_FLAG_START);
        int teamNo;
        int characterId;

        for (playerIndex = 0; playerIndex < GW_PLAYER_MAX; playerIndex++) {
            if (_CheckFlag(FLAG_BOARD_TUTORIAL)) {
                GwPlayer[playerIndex].comF = TRUE;
                GwPlayerConf[playerIndex].type = TRUE;
            }
            if (GWPartyGet() != FALSE) {
                // The flag query's result is ignored in the party-board branch.
                _CheckFlag(FLAG_BOARD_TUTORIAL);
            } else if (playerIndex > 0) {
                GwPlayerConf[playerIndex].charNo = singleCharNoTbl[playerIndex];
                GwPlayer[playerIndex].comF = TRUE;
                GwPlayerConf[playerIndex].type = TRUE;
            }
            GwPlayer[playerIndex].charNo = GwPlayerConf[playerIndex].charNo;
            // The remaining configuration self-assignments leave the selected values unchanged.
            GwPlayerConf[playerIndex].charNo = GwPlayerConf[playerIndex].charNo;
            GwPlayer[playerIndex].padNo = GwPlayerConf[playerIndex].padNo;
            GwPlayerConf[playerIndex].padNo = GwPlayerConf[playerIndex].padNo;
            GwPlayer[playerIndex].comF = GwPlayerConf[playerIndex].type;
            GwPlayerConf[playerIndex].type = GwPlayerConf[playerIndex].type;
            GwPlayer[playerIndex].comDif = GwPlayerConf[playerIndex].comDif;
            GwPlayerConf[playerIndex].comDif = GwPlayerConf[playerIndex].comDif;
            GwPlayer[playerIndex].masuId = startSpaceId;
            GwPlayer[playerIndex].masuIdNext = startSpaceId;
            GwPlayer[playerIndex].statusColor = 0;
            GwPlayer[playerIndex].diceMode = 0;
            for (entryIndex = 0; entryIndex < 3; entryIndex++) {
                GwPlayer[playerIndex].capsule[entryIndex] = -1;
            }
            GwPlayer[playerIndex].team = teamNo = GwPlayerConf[playerIndex].grpNo;
            GwPlayerConf[playerIndex].grpNo = teamNo;
            GwPlayer[playerIndex].orderNo = playerIndex;
            mbPlayerMetalSet(playerIndex, FALSE);
            mbPlayerBiriQSet(playerIndex, FALSE);
        }
        for (characterId = 0; characterId < CHARNO_MAX; characterId++) {
            if (CharMotionAMemPGet(characterId)) {
                if (GWPartyGet() == FALSE &&
                    characterId == mbSingleTeamCharGet()) {
                    continue;
                }
                for (entryIndex = 0; entryIndex < GW_PLAYER_MAX; entryIndex++) {
                    if (characterId == GwPlayer[entryIndex].charNo) {
                        break;
                    }
                }
                if (entryIndex >= GW_PLAYER_MAX) {
                    CharDataClose(characterId);
                }
            }
        }
        for (characterId = 0; characterId < GW_PLAYER_MAX; characterId++) {
            if (!CharMotionAMemPGet(GwPlayer[characterId].charNo)) {
                CharMotionInit(GwPlayer[characterId].charNo);
            }
        }
        if (GWPartyGet() == FALSE) {
            characterId = mbSingleTeamCharGet();

            if (!CharMotionAMemPGet(characterId)) {
                CharMotionInit(characterId);
            }
        }
        GwSystem.playerMode = 0;
    }
    for (playerIndex = 0; playerIndex < GW_PLAYER_MAX; playerIndex++, playerWorkP++) {
        MBMODELID modelId;
        GW_PLAYER *playerData;
        int characterId;

        playerWorkP->startTurnHook = playerWorkP->endTurnHook = NULL;
        playerWorkP->rotateObj = playerWorkP->moveObj = playerWorkP->posFixObj = NULL;
        playerData = GWPlayerGet(playerIndex);
        playerData->playerNo = playerWorkP->playerNo = playerIndex;
        GwPlayerConf[playerIndex].type = GwPlayer[playerIndex].comF;
        GwPlayerConf[playerIndex].padNo = GwPlayer[playerIndex].padNo;
        GwPlayerConf[playerIndex].grpNo = mbPlayerGrpGet(playerIndex);
        GwPlayerConf[playerIndex].comDif = GwPlayer[playerIndex].comDif;
        characterId = GwPlayer[playerIndex].charNo;
        GwPlayerConf[playerIndex].charNo = characterId;
        for (entryIndex = 0; entryIndex < 15; entryIndex++) {
            characterMotionData[entryIndex] = charMotDirTbl[characterId] | charMotNoTbl[entryIndex];
        }
        characterMotionData[entryIndex] = HU_DATANUM_NONE;
        modelId = playerWorkP->objId =
            mbObjCharCreate(characterId, charMdlFileTbl[characterId], characterMotionData, FALSE);
        mbPlayerMotionVoiceOnSet(playerIndex, 7, FALSE);
        mbPlayerMotionVoiceOnSet(playerIndex, 12, FALSE);
        mbPlayerMotionVoiceOnSet(playerIndex, 8, FALSE);
        mbPlayerMotionVoiceOnSet(playerIndex, 13, FALSE);
        playerWorkP->colObj =
            omAddObjEx(mbObjMan, PLAYER_OBJ_PRIORITY, 0, 0, -1,
                PlayerColOMExec);
        omObjGetWork(playerWorkP->colObj, PLAYERCOLWORK)->playerNo = playerIndex;
        omObjGetWork(playerWorkP->colObj, PLAYERCOLWORK)->resyncF = TRUE;
        omObjGetWork(playerWorkP->colObj, PLAYERCOLWORK)->masuIdNext =
            GwPlayer[playerIndex].masuId;
        mbPlayerMatClone(playerIndex);
        playerWorkP->motNo = 1;
        mbObjMotionSet(modelId, playerWorkP->motNo, HU3D_MOTATTR_LOOP);
        GwPlayer[playerIndex].dispLightF = TRUE;
        GwPlayer[playerIndex].masuIdPrev = -1;
        mbPlayerWorkGet(playerIndex)->moveEndF = TRUE;
        CharModelDataClose(characterId);
    }
    mbPlayerColSnapSet(FALSE);
    if (GWPartyGet() != FALSE) {
        mbPlayerPosResetAll();
    } else {
        for (playerIndex = 0; playerIndex < GW_PLAYER_MAX; playerIndex++) {
            if (playerIndex > 0) {
                GwPlayer[playerIndex].masuId = 0;
                GwPlayer[playerIndex].masuIdNext = 0;
                GwPlayer[playerIndex].masuIdPrev = 0;
                mbPlayerDispSet(playerIndex, FALSE);
            } else {
                mbPlayerPosReset(playerIndex);
            }
        }
    }
    CharEffectLayerSet(5);
}

// Releases each player's model, cloned material, effects and dice display on board shutdown.
void mbPlayerClose(void)
{
    MBPLAYERWORK *playerWorkP;
    int playerIndex;

    playerWorkP = &playerWork[0];
    for (playerIndex = 0; playerIndex < GW_PLAYER_MAX; playerIndex++, playerWorkP++) {
        GW_PLAYER *playerData;

        playerData = GWPlayerGet(playerIndex);

        if (playerWorkP->objId != MB_MODEL_NONE) {
            PlayerMetalKill(playerIndex);
            PlayerBiriQKill(playerIndex);
            mbObjKill(playerWorkP->objId);
            playerWorkP->objId = MB_MODEL_NONE;
        }
        if (playerWorkP->matCopy) {
            HSF_MATERIAL *materialCopy = playerWorkP->matCopy;

            HuMemDirectFree(materialCopy);
            playerWorkP->matCopy = NULL;
        }
        mbDiceNumKill(playerIndex);
    }
}

MBPLAYERWORK *mbPlayerWorkGet(int playerNo)
{
    return &playerWork[playerNo];
}

// Installs the callback run as a player's turn begins.
void mbPlayerTurnInitHookSet(void (*hook)(int playerNo))
{
    turnInitHook = hook;
}

// Installs the callback run as a player's turn ends.
void mbPlayerTurnCloseHookSet(void (*hook)(int playerNo))
{
    turnCloseHook = hook;
}

// Stores the per-player callback run at the start of that player's turn.
void mbPlayerStartTurnHookSet(int playerNo, MBPLAYERTURNHOOK hook)
{
    playerWork[playerNo].startTurnHook = hook;
}

// Stores the per-player callback run at the end of that player's turn.
void mbPlayerEndTurnHookSet(int playerNo, MBPLAYERTURNHOOK hook)
{
    playerWork[playerNo].endTurnHook = hook;
}

// Stores the callback used while the player moves between board spaces.
void mbPlayerMoveHookSet(int playerNo, MBPLAYERMOVEHOOK hook)
{
    playerWork[playerNo].moveHook = hook;
}

// Runs turns from GwSystem.turnPlayerNo through the final board player.
// intrF resumes the current turn at its saved player mode.
void mbTurnExec(BOOL intrF)
{
    int playerNo;

    turnIntrF = intrF;
    blackoutF = FALSE;
    playerNo = GwSystem.turnPlayerNo;
    for (; playerNo < GW_PLAYER_MAX; playerNo++) {
        int orderNo;
        int otherPlayerNo;

        GwSystem.turnPlayerNo = playerNo;
        orderNo = 1;
        GwPlayer[playerNo].orderNo = 0;
        for (otherPlayerNo = 0; otherPlayerNo < GW_PLAYER_MAX; otherPlayerNo++) {
            if (playerNo != otherPlayerNo) {
                GwPlayer[otherPlayerNo].orderNo = orderNo++;
            }
            mbPlayerMotionSet(otherPlayerNo, 1, HU3D_MOTATTR_LOOP);
            GwPlayer[otherPlayerNo].masuIdNext = GwPlayer[otherPlayerNo].masuId;
        }
        PlayerTurn(playerNo);
        turnIntrF = FALSE;
    }
}

// Runs the single-player board turn for player zero, optionally resuming it.
void mbSingleTurnExec(BOOL intrF)
{
    turnIntrF = intrF;
    blackoutF = FALSE;
    GwSystem.turnPlayerNo = 0;
    GwPlayer[0].orderNo = 0;
    mbPlayerMotionSet(0, 1, HU3D_MOTATTR_LOOP);
    PlayerTurn(0);
    turnIntrF = FALSE;
}

// mbTurnExec and mbSingleTurnExec call this to run dice, movement, space events and turn callbacks.
static void PlayerTurn(int playerNo)
{
    BOOL telopF = FALSE;
    BOOL killerF;
    BOOL eventResult;
    BOOL partyAtStartF;
    int timeTurn;
    BOOL partyAfterStartHookF;
    BOOL partyAtEndF;
    int telopFrame;

    GwSystem.turnPlayerNo = playerNo;
    mbPlayerPosReset(playerNo);
    mbPlayerColSnapSet(TRUE);
    mbev_PlayerColMasuSet(playerNo, GwPlayer[playerNo].masuId, TRUE);
    mbStarDispSetAll(TRUE);
    mbStarMasuDispSet(GwPlayer[playerNo].masuId, FALSE);
    if (!turnIntrF) {
        MBPLAYERWORK *workP;

        mbCameraPlayerViewSetFast(playerNo, MB_CAMERA_VIEW_ZOOMIN);
        mbCameraMoveOnSet(FALSE);
        mbCameraMoveWait();
        mbPlayerMotionSet(playerNo, 1, HU3D_MOTATTR_LOOP);
        mbPlayerRotYSet(playerNo, 0.0f);
        if (turnInitHook) {
            turnInitHook(playerNo);
        }
        mbStatusDispForceSetAll(TRUE);
        partyAtStartF = GwSystem.partyF;
        if (partyAtStartF && playerNo == 0) {
            timeTurn = GwSystem.timeTurn;
            if (timeTurn > 0) {
                mbTelopTimeCreate();
                telopF = TRUE;
            }
        }
        if (mbWipeSpecialStatGet()) {
            if (playerNo == 0) {
                mbWipeDissolveFadeIn();
            } else {
                mbWipeSpecialFadeOutCreate(5, 42);
            }
        }
        GwPlayer[playerNo].capsuleUse = -1;
        workP = &playerWork[playerNo];
        workP->masuNext = 0;
        if (telopF) {
            for (telopFrame = 0; telopFrame < 100; telopFrame++) {
                HuPrcVSleep();
            }
        }
        ev_PlayerStartTurn(playerNo);
        omVibrate(playerNo, 20, 20, 0);
        mbPauseDisableSet(FALSE);
        mbTutorialCall(3);
        partyAfterStartHookF = GwSystem.partyF;
        if (partyAfterStartHookF || GwSystem.turnNo == 1) {
            mbTelopPlayerCreate(playerNo);
        }
        GwPlayer[playerNo].moveNum = -1;
        GwPlayer[playerNo].skipEventF = FALSE;
        GwSystem.playerMode = 0;
    }
repeat:
    // Completed stages fall through; playerMode records the stage to resume after an interruption.
    switch (GwSystem.playerMode) {
        case 0:
        case 2:
            _ClearFlag(FLAG_BOARD_WALKDONE);
            killerF = DiceRun(playerNo);
            if (GwPlayer[playerNo].moveNum != 0) {
                GwSystem.playerMode = 3;
            }
            if (killerF) {
                mbev_CapKillerMoveCall(playerNo);
                GwSystem.playerMode = 4;
                goto repeat;
            }
        case 3:
            mbPlayerColSnapSet(TRUE);
            if (GwPlayer[playerNo].moveNum != 0) {
                _ClearFlag(FLAG_BOARD_WALKDONE);
                turnIntrF = PlayerViewSet(playerNo, TRUE, TRUE, TRUE);
                mbCameraMoveOnSet(TRUE);
                if (GwPlayer[playerNo].moveNum != 0) {
                    PlayerMoveCall(playerNo);
                }
            }
            mbPlayerMetalSet(playerNo, FALSE);
            mbPlayerBiriQSet(playerNo, FALSE);
            GwSystem.playerMode = 4;
        case 4:
            GwSystem.playerMode = 5;
            if (_CheckFlag(FLAG_BOARD_LAST5) &&
                mbPlayerKettouCheck(playerNo, GwPlayer[playerNo].masuId)) {
                mbev_CapCallKettou(
                    playerNo, GwPlayer[playerNo].masuId, FALSE);
            }
        case 5:
            turnIntrF =
                PlayerViewSet(playerNo, turnIntrF, TRUE, FALSE);
            GwSystem.playerMode = 6;
            mbMasuPlayerColorSet(playerNo);
            eventResult =
                mbev_MasuCapStop(playerNo, GwPlayer[playerNo].masuId);
            if (!eventResult) {
                GwSystem.playerMode = 7;
                goto repeat;
            }
        case 6:
            turnIntrF =
                PlayerViewSet(playerNo, turnIntrF, FALSE, FALSE);
            GwSystem.playerMode = 7;
            mbCameraMoveWait();
            if (!mbev_MasuStop(playerNo, GwPlayer[playerNo].masuId)) {
                break;
            }
        case 7:
            turnIntrF =
                PlayerViewSet(playerNo, turnIntrF, FALSE, FALSE);
            mbPlayerRotateStart(playerNo, 0, 15);
            HuPrcSleep(15);
            mbCameraMoveWait();
            mbPlayerMotIdleSet(playerNo);
        case 1:
        default:
            break;
    }
    ev_PlayerEndTurn(playerNo);
    mbTutorialCall(4);
    partyAtEndF = GwSystem.partyF;
    if (partyAtEndF) {
        if (playerNo != GW_PLAYER_MAX - 1) {
            mbWipeSpecialFadeInCreate(5, 1);
        } else {
            mbWipeFadeOut();
        }
    }
    mbPlayerColSnapPlayerSet(playerNo, TRUE);
    if (turnCloseHook) {
        turnCloseHook(playerNo);
    }
}

// Restores the player's camera view after an interrupt or special wipe.
static BOOL PlayerViewSet(
    int playerNo, BOOL intrF, BOOL waitF, BOOL carF)
{
    BOOL wipeF = mbWipeSpecialStatGet();

    if (intrF || wipeF) {
        if (!wipeF) {
            mbCameraPlayerViewSet(playerNo,
                carF ? MB_CAMERA_VIEW_WALK : MB_CAMERA_VIEW_ZOOMIN);
        } else {
            mbCameraPlayerViewSetFast(playerNo,
                carF ? MB_CAMERA_VIEW_WALK : MB_CAMERA_VIEW_ZOOMIN);
        }
        if (waitF) {
            mbCameraMoveWait();
        }
        if (carF && GwPlayer[playerNo].moveNum != 0) {
            mbMoveNumCreate(playerNo, TRUE);
        }
        if (wipeF) {
            mbWipeFadeIn();
        }
        intrF = FALSE;
    }
    return intrF;
}

// DiceRun uses this to map the selected board dice index to the dice implementation type.
int mbPlayerDiceTypeGet(int diceNo)
{
    int diceTypeTbl[7][2] = {
        { 0, 0 },
        { 1, 1 },
        { 2, 2 },
        { 3, 14 },
        { 4, 0 },
        { 5, 4 },
        { 6, 3 }
    };
    int mappingIndex;

    for (mappingIndex = 0; mappingIndex < 7; mappingIndex++) {
        if (diceNo == diceTypeTbl[mappingIndex][0]) {
            return diceTypeTbl[mappingIndex][1];
        }
    }
    return 0;
}

// PlayerTurn calls this to handle capsule selection and dice use, then store the move count.
static BOOL DiceRun(int playerNo)
{
    BOOL killerF = FALSE;
    int capsuleCount;
    BOOL skipCapsuleSelectF = FALSE;
    int diceResult;
    int presetDiceValue;

    GwPlayer[playerNo].diceNum = 1;
repeat:
    capsuleCount = mbPlayerCapsuleNumGet(playerNo);
    if (GWPartyGet() == FALSE) {
        // Solo mode offers capsule selection regardless of inventory, except on the first turn.
        capsuleCount = 1;
        if (GwSystem.turnNo <= 1) {
            capsuleCount = 0;
        }
    }
    if (GwPlayer[playerNo].capsuleUse == -1 && !skipCapsuleSelectF &&
        capsuleCount != 0) {
        GwSystem.playerMode = 0;
        diceResult = mbCapSelect();
        if (GwPlayer[playerNo].capsuleUse != -1) {
            GwPlayer[playerNo].capsuleUseNum++;
        }
    } else {
        mbStatusDispSetAll(TRUE);
        while (!mbStatusOffCheckAll()) {
            HuPrcVSleep();
        }
        GwSystem.playerMode = 2;
        if (_CheckFlag(FLAG_BOARD_TUTORIAL)) {
            int tutorialDiceValues[4];
            int dieIndex;
            int diceType;
            int diceCount;

            diceType = mbPlayerDiceTypeGet(GwPlayer[playerNo].diceMode);
            diceCount = mbDiceMaxGet(diceType);
            for (dieIndex = 0; dieIndex < diceCount; dieIndex++) {
                presetDiceValue = mbTutorialCall(5);
                if (presetDiceValue < 0) {
                    presetDiceValue = mbRandMod(mbDiceValueMaxGet(diceType));
                }
                tutorialDiceValues[dieIndex] = presetDiceValue;
            }
            if (diceCount <= 1) {
                diceResult = mbDiceExec(playerNo, diceType, NULL,
                    tutorialDiceValues[0], FALSE, TRUE, NULL, 0);
            } else {
                diceResult = mbDiceProcExec(playerNo, diceType, NULL,
                    tutorialDiceValues, FALSE, TRUE, NULL, 0);
            }
            mbTutorialCall(6);
        } else {
            presetDiceValue = -1;

            if (GWPartyGet() == FALSE) {
                presetDiceValue = mbSingleCall(0, -1);
            } else if (GwPlayer[playerNo].comF &&
                mbPlayerDiceTypeGet(GwPlayer[playerNo].diceMode) == 14) {
                presetDiceValue = mbMasuPKinokoValueGet(
                    playerNo, GwPlayer[playerNo].masuId);
            }
            diceResult = mbDiceExec(playerNo,
                mbPlayerDiceTypeGet(GwPlayer[playerNo].diceMode), NULL,
                presetDiceValue, TRUE, TRUE, NULL, 0);
        }
    }
    switch (diceResult) {
        case -3:
            if (GWPartyGet() == FALSE && !_CheckFlag(FLAG_BOARD_TUTORIAL)) {
                mbSingleCall(1, -1);
            }
            mbDiceKill(playerNo);
            mbev_Scroll(playerNo, FALSE);
            break;
        case -4:
            if (GWPartyGet() == FALSE && !_CheckFlag(FLAG_BOARD_TUTORIAL)) {
                mbSingleCall(1, -1);
            }
            mbDiceKill(playerNo);
            mbev_Scroll(playerNo, TRUE);
            break;
        case -6:
            if (GWPartyGet() == FALSE && !_CheckFlag(FLAG_BOARD_TUTORIAL)) {
                mbSingleCall(1, -1);
            }
            skipCapsuleSelectF = FALSE;
            mbDiceKill(playerNo);
            mbAudFXPlay(MSM_SE_CMN_04);
            break;
        case -7:
            skipCapsuleSelectF = TRUE;
            mbDiceKill(playerNo);
            break;
        case -5:
        default:
            break;
    }
    if (diceResult <= 0) {
        goto repeat;
    }
    if (GwPlayer[playerNo].diceMode == 5) {
        killerF = TRUE;
    }
    GwPlayer[playerNo].diceMode = 0;
    GwPlayer[playerNo].moveNum = diceResult;
    mbMoveNumCreate(playerNo, TRUE);
    mbDiceNumKill(playerNo);
    if (GWPartyGet() == FALSE) {
        mbSingleCall(3, diceResult);
    }
    return killerF;
}

// Starts the player's movement process and waits until it finishes.
static void PlayerMoveCall(int playerNo)
{
    MBPLAYERWORK *workP = &playerWork[playerNo];

    mbPlayerColSnapSet(TRUE);
    workP->moveProc =
        HuPrcChildCreate(PlayerMove, PLAYER_MOVE_PROCESS_PRIORITY,
            PLAYER_MOVE_PROCESS_STACK_SIZE, 0, mbMainProc);
    workP->moveProc->property = workP;
    HuPrcDestructorSet2(workP->moveProc, PlayerMoveDestroy);
    while (workP->moveProc) {
        HuPrcVSleep();
    }
    _SetFlag(FLAG_BOARD_WALKDONE);
}

// Movement process created by PlayerMoveCall; advances one board space per pass.
static void PlayerMove(void)
{
    MBPLAYERWORK *playerWorkP = HuPrcCurrentGet()->property;
    int playerNo = playerWorkP->playerNo;
    s16 nextSpaceId;
    BOOL spaceHiddenF;

    mbPlayerWorkGet(playerNo)->_unk0C = 0;
    mbPlayerWorkGet(playerNo)->moveEndF = TRUE;
    playerWorkP->moveHook = NULL;
repeat:
    GwPlayer[playerNo].masuIdPrev = GwPlayer[playerNo].masuId;
    if (!_CheckFlag(FLAG_BOARD_DEBUG) ||
        _CheckFlag(FLAG_BOARD_TUTORIAL)) {
        if (mbev_Branch(playerNo, &nextSpaceId)) {
            goto end;
        }
    } else {
        if (mbev_BranchDebug(playerNo, &nextSpaceId) || nextSpaceId < 0) {
            goto end;
        }
    }
    nextSpaceId = mbev_GateMasu(
        playerNo, GwPlayer[playerNo].masuId, nextSpaceId);
    GwPlayer[playerNo].masuIdNext = nextSpaceId;
    mbPlayerWorkGet(playerNo)->_unk08 = -1;
    playerWorkP->_unk06 = nextSpaceId;
    if (playerWorkP->moveF) {
        HuPrcSleep(-1);
    }
    PlayerColKill(playerNo);
    mbev_CapCallTrap(
        playerNo, GwPlayer[playerNo].masuId, nextSpaceId);
    mbev_MasuMasuEnd(nextSpaceId);
    if (playerWorkP->moveHook) {
        mbPlayerWorkGet(playerNo)->_unk0C = 4;
        playerWorkP->moveHook(playerNo);
        playerWorkP->moveHook = NULL;
    } else {
        mbPlayerMasuMove(playerNo, TRUE);
    }
    mbPlayerWorkGet(playerNo)->_unk0C = 0;
    mbPlayerWorkGet(playerNo)->moveEndF = TRUE;
    mbPlayerWorkGet(playerNo)->masuMoveF = FALSE;
    GwPlayer[playerNo].masuId = nextSpaceId;
    mbTutorialCall(8);
    spaceHiddenF = !mbMasuDispCheck(nextSpaceId);
    if (!spaceHiddenF && GwPlayer[playerNo].biriQF) {
        PlayerBiriQFlashSet(playerNo);
        mbev_CapBiriQShockCreate(playerNo);
    }
    if (!mbev_MasuMasuStart(playerNo)) {
        nextSpaceId = GwPlayer[playerNo].masuId;
        if (!mbev_MasuMove(playerNo, nextSpaceId)) {
            spaceHiddenF = !mbMasuDispCheck(nextSpaceId);
            if (!spaceHiddenF) {
                mbAudFXPlay(PLAYER_MOVE_COUNT_SFX);
                GwPlayer[playerNo].moveNum--;
                if (GwPlayer[playerNo].moveNum < 0) {
                    GwPlayer[playerNo].moveNum = 0;
                }
                if (GwPlayer[playerNo].moveNum == 0) {
                    mbMoveNumKill(playerNo);
                }
            }
        }
    }
    if (playerWorkP->moveF) {
        playerWorkP->_unk10_3 = TRUE;
        HuPrcSleep(-1);
    }
    mbTutorialCall(9);
    if (GwPlayer[playerNo].moveNum != 0) {
        goto repeat;
    }
end:
    playerWorkP->moveHook = NULL;
    mbMoveNumKill(playerNo);
    mbPlayerWorkGet(playerNo)->_unk0C = 0;
    mbPlayerWorkGet(playerNo)->moveEndF = TRUE;
    if (playerWorkP->moveF) {
        mbPlayerRotateStart(playerNo, 0, 15);
        while (!mbPlayerRotateCheck(playerNo)) {
            HuPrcVSleep();
        }
        mbPlayerMotIdleSet(playerNo);
        mbPlayerColOrderReset();
    } else {
        mbPlayerMotIdleSet(playerNo);
    }
    HuPrcEnd();
}

// Process destructor that clears the player's movement-process handle.
static void PlayerMoveDestroy(void)
{
    MBPLAYERWORK *playerWorkP = HuPrcCurrentGet()->property;

    playerWorkP->moveProc = NULL;
}

// Invokes and clears the player's start-turn hook when it reports completion.
static void ev_PlayerStartTurn(int playerNo)
{
    if (playerWork[playerNo].startTurnHook) {
        if (playerWork[playerNo].startTurnHook()) {
            playerWork[playerNo].startTurnHook = NULL;
        }
    }
}

// Invokes and clears the player's end-turn hook when it reports completion.
static void ev_PlayerEndTurn(int playerNo)
{
    if (playerWork[playerNo].endTurnHook) {
        if (playerWork[playerNo].endTurnHook()) {
            playerWork[playerNo].endTurnHook = NULL;
        }
    }
}

// Sets the destination space, then moves the player there.
void mbPlayerMasuMoveTo(int playerNo, int masuId, BOOL waitF)
{
    GwPlayer[playerNo].masuIdNext = masuId;
    mbPlayerMasuMove(playerNo, waitF);
}

// Walks toward the selected space, setting masuMoveF only while this call runs.
// With waitF false, that flag is cleared before the movement finishes.
void mbPlayerMasuMove(int playerNo, BOOL waitF)
{
    MBPLAYERWORK *playerWorkP = &playerWork[playerNo];
    MBPLAYERWORK *playerWorkAfterMoveP;

    playerWorkP->masuMoveF = TRUE;
    mbPlayerMoveExec(
        playerNo, NULL, NULL, mbPlayerWalkSpeedGet(), NULL, waitF);
    playerWorkAfterMoveP = &playerWork[playerNo];
    playerWorkAfterMoveP->masuMoveF = FALSE;
}

// Moves the player to an explicit world position using the normal walking speed.
void mbPlayerMasuMovePos(int playerNo, HuVecF *pos, BOOL waitF)
{
    mbPlayerMoveExec(
        playerNo, NULL, pos, mbPlayerWalkSpeedGet(), NULL, waitF);
}

// Walks to the given board space with a caller-selected duration.
void mbPlayerMasuMoveSpeed(
    int playerNo, int masuId, s16 maxTime, BOOL waitF)
{
    MBPLAYERWORK *workP;
    MBPLAYERWORK *workP2;
    HuVecF pos;

    mbMasuPosGet(masuId, &pos);
    workP = &playerWork[playerNo];
    workP->masuMoveF = TRUE;
    mbPlayerMoveExec(playerNo, NULL, &pos, maxTime, NULL, waitF);
    workP2 = &playerWork[playerNo];
    workP2->masuMoveF = FALSE;
}

// Starts movement; adjacent-space jumps and climbs select their own motion and duration.
void mbPlayerMoveExec(int playerNo, HuVecF *srcPos, HuVecF *dstPos,
    s16 maxTime, HuVecF *rot, BOOL waitF)
{
    mbPlayerMoveMain(playerNo, srcPos, dstPos, 0, 1.0f, HU3D_MOTATTR_LOOP,
        maxTime, rot, waitF);
}

enum {
    PLAYER_MOVE_MODE_RUN,
    PLAYER_MOVE_MODE_JUMP,
    PLAYER_MOVE_MODE_CLIMB
};

typedef struct PlayerMoveWork {
    // Stops the movement object's update callback.
    u8 stopUpdateF : 1;
    // Selects walking, jumping or climbing movement.
    u8 movementMode : 2;
    // Player whose model is being moved.
    u8 playerNo : 2;
    // Elapsed movement frames.
    s16 elapsedFrames;
    // Total movement frames.
    s16 totalFrames;
} PLAYERMOVEWORK;

static void PlayerMoveOMExec(OMOBJ *objP);

// Called by movement helpers to start travel between positions or adjacent spaces.
void mbPlayerMoveMain(int playerNo, HuVecF *srcPos, HuVecF *dstPos, u32 motNo,
    float motSpeed, u32 motAttr, s16 maxTime, HuVecF *rot, BOOL waitF)
{
    BOOL setAngle = FALSE;
    OMOBJ *objP = playerWork[playerNo].moveObj = omAddObjEx(mbObjMan,
        PLAYER_OBJ_PRIORITY, 0, 0, -1, PlayerMoveOMExec);
    PLAYERMOVEWORK *workP = omObjGetWork(objP, PLAYERMOVEWORK);
    int mode;
    HuVecF moveDir;
    HuVecF playerRot;

    if (srcPos != NULL) {
        objP->trans.x = srcPos->x;
        objP->trans.y = srcPos->y;
        objP->trans.z = srcPos->z;
    } else {
        mbPlayerPosGet(playerNo, &objP->trans);
    }
    if (dstPos != NULL) {
        objP->rot.x = dstPos->x;
        objP->rot.y = dstPos->y;
        objP->rot.z = dstPos->z;
        workP->movementMode = PLAYER_MOVE_MODE_RUN;
    } else {
        MASU *masuPrev;
        MASU *masuNext;
        s16 masuIdPrev;
        int masuIdNext;

        masuIdNext = GwPlayer[playerNo].masuIdNext;
        masuNext = mbMasuGet(masuIdNext);
        masuIdPrev = GwPlayer[playerNo].masuIdPrev;
        if (masuIdPrev < 0) {
            mode = PLAYER_MOVE_MODE_RUN;
        } else {
            masuPrev = mbMasuGet(masuIdPrev);
            if ((masuPrev->flag & MASU_FLAG_JUMPFROM)
                && (masuNext->flag & MASU_FLAG_JUMPTO)) {
                mode = PLAYER_MOVE_MODE_JUMP;
            } else if ((masuPrev->flag & MASU_FLAG_CLIMBFROM)
                && (masuNext->flag & MASU_FLAG_CLIMBTO)) {
                mode = PLAYER_MOVE_MODE_CLIMB;
            } else {
                mode = PLAYER_MOVE_MODE_RUN;
            }
        }
        workP->movementMode = mode;
        mbMasuPosGet(GwPlayer[playerNo].masuIdNext, &objP->rot);
    }
    if (motNo == 0) {
        switch (workP->movementMode) {
            case PLAYER_MOVE_MODE_RUN:
                mbPlayerWorkGet(playerNo)->_unk0C = 1;
                mbPlayerMotionShiftSet(playerNo, 3, 0.0f, 4.0f,
                    HU3D_MOTATTR_LOOP);
                break;
            case PLAYER_MOVE_MODE_JUMP:
                mbPlayerWorkGet(playerNo)->_unk0C = 2;
                mbPlayerMotionShiftSet(playerNo, 4, 6.0f, 2.0f,
                    HU3D_MOTATTR_NONE);
                maxTime = 24;
                break;
            case PLAYER_MOVE_MODE_CLIMB:
                mbPlayerWorkGet(playerNo)->_unk0C = 3;
                mbPlayerMotionShiftSet(playerNo, 14, 0.0f, 4.0f,
                    HU3D_MOTATTR_LOOP);
                maxTime = 100;
                motSpeed = 2.0f;
                VECSubtract(&objP->rot, &objP->trans, &moveDir);
                // The climb duration is then replaced with a distance-based frame count.
                maxTime = VECMag(&moveDir) / 15.000001f;
                if (objP->trans.y >= objP->rot.y) {
                    moveDir.x = -moveDir.x;
                    moveDir.y = -moveDir.y;
                    moveDir.z = -moveDir.z;
                    motSpeed = -motSpeed;
                }
                playerRot.x = playerRot.z = 0.0f;
                playerRot.y = HuAtan(moveDir.x, moveDir.z);
                mbPlayerRotSetV(playerNo, &playerRot);
                setAngle = TRUE;
                break;
        }
    } else {
        mbPlayerMotionShiftSet(
            playerNo, motNo, 0.0f, 4.0f, motAttr);
    }
    mbPlayerMotionSpeedSet(playerNo, motSpeed);
    if (setAngle == FALSE) {
        if (rot == NULL) {
            float rotY;

            VECSubtract(&objP->rot, &objP->trans, &playerRot);
            rotY = 90.0f - HuAtan(playerRot.z, playerRot.x);
            mbPlayerRotYSet(playerNo, rotY);
        } else {
            mbPlayerRotSetV(playerNo, rot);
        }
    }
    if (srcPos) {
        mbPlayerPosSetV(playerNo, srcPos);
    }
    objP->scale.x = objP->trans.x;
    objP->scale.y = objP->trans.y;
    objP->scale.z = objP->trans.z;
    workP->playerNo = playerNo;
    workP->elapsedFrames = 0;
    workP->totalFrames = maxTime;
    {
        int movePlayerNo = workP->playerNo;

        if (mbPlayerWorkGet(movePlayerNo)->masuMoveF) {
            int movePlayerNo2;
            int moveMaxTime;

            moveMaxTime = workP->totalFrames;
            movePlayerNo2 = workP->playerNo;
            mbPlayerWorkGet(movePlayerNo2)->_unk08 = moveMaxTime;
        }
    }
    GwPlayer[playerNo].moveF = TRUE;
    if (waitF) {
        while (GwPlayer[playerNo].moveF) {
            HuPrcVSleep();
        }
    }
    // These work flags reset on return even when nonblocking movement is still active.
    mbPlayerWorkGet(playerNo)->_unk0C = 0;
    mbPlayerWorkGet(playerNo)->moveEndF = TRUE;
}

// Object update callback that interpolates movement and raises the jump arc when needed.
static void PlayerMoveOMExec(OMOBJ *objP)
{
    PLAYERMOVEWORK *workP = omObjGetWork(objP, PLAYERMOVEWORK);
    float weight;

    if (mbExitCheck() || workP->stopUpdateF) {
        GwPlayer[workP->playerNo].moveF = FALSE;
        omDelObjEx(HuPrcCurrentGet(), objP);
        playerWork[workP->playerNo].moveObj = NULL;
        return;
    }
    workP->elapsedFrames++;
    weight = (float)workP->elapsedFrames / workP->totalFrames;
    objP->trans.x = objP->scale.x
        + (weight * (objP->rot.x - objP->scale.x));
    objP->trans.y = objP->scale.y
        + (weight * (objP->rot.y - objP->scale.y));
    objP->trans.z = objP->scale.z
        + (weight * (objP->rot.z - objP->scale.z));
    {
        int movePlayerNo = workP->playerNo;
        MBPLAYERWORK *moveWorkP = &playerWork[movePlayerNo];

        if (moveWorkP->masuMoveF) {
            int movePlayerNo2;
            MBPLAYERWORK *moveWorkP2;
            int moveTime = workP->totalFrames - workP->elapsedFrames;

            movePlayerNo2 = workP->playerNo;
            moveWorkP2 = &playerWork[movePlayerNo2];

            moveWorkP2->_unk08 = moveTime;
        }
    }
    if (workP->elapsedFrames >= workP->totalFrames) {
        GwPlayer[workP->playerNo].moveF = FALSE;
        mbPlayerPosSet(workP->playerNo, objP->rot.x, objP->rot.y,
            objP->rot.z);
        omDelObjEx(HuPrcCurrentGet(), objP);
        playerWork[workP->playerNo].moveObj = NULL;
    } else if (workP->movementMode != PLAYER_MOVE_MODE_JUMP) {
        mbPlayerPosSet(workP->playerNo, objP->trans.x, objP->trans.y,
            objP->trans.z);
    } else {
        int movePlayerNo;
        int jumpPlayerNo;

        {
            MBPLAYERWORK *moveWorkP;

            movePlayerNo = workP->playerNo;
            moveWorkP = &playerWork[movePlayerNo];

            moveWorkP->moveEndF = FALSE;
        }
        if (workP->elapsedFrames >= workP->totalFrames - 2) {
            weight = 1.0f;
            {
                MBPLAYERWORK *moveWorkP;

                jumpPlayerNo = workP->playerNo;
                moveWorkP = &playerWork[jumpPlayerNo];

                moveWorkP->moveEndF = TRUE;
            }
        } else {
            weight = (float)workP->elapsedFrames / (workP->totalFrames - 2);
        }
        mbPlayerPosSet(workP->playerNo, objP->trans.x,
            objP->trans.y + (100.0f * (2.0f * HuSin(weight * 180.0f))),
            objP->trans.z);
        if (workP->elapsedFrames == workP->totalFrames - 5) {
            mbPlayerMotionShiftSet(workP->playerNo, 5, 2.0f, 2.0f,
                HU3D_MOTATTR_NONE);
        }
    }
}

typedef struct PlayerRotateWork {
    // Stops the rotation object's update callback.
    u8 stopUpdateF : 1;
    // Player whose facing direction is changing.
    s8 playerNo;
    // Total rotation frames.
    s16 totalFrames;
    // Elapsed rotation frames.
    s16 elapsedFrames;
} PLAYERROTATEWORK;

static void PlayerRotateOMExec(OMOBJ *objP);

// Turn and movement flow use this to start a turn toward endAngle.
// Nonpositive maxTime does nothing; a difference truncating to zero degrees snaps next update.
// Other turns apply the final angle after maxTime interpolation updates.
void mbPlayerRotateStart(int playerNo, s16 endAngle, s16 maxTime)
{
    OMOBJ *objP;
    PLAYERROTATEWORK *workP;
    float angle;

    if (maxTime <= 0) {
        return;
    }
    if (playerWork[playerNo].rotateObj) {
        objP = playerWork[playerNo].rotateObj;
    } else {
        playerWork[playerNo].rotateObj = objP = omAddObjEx(mbObjMan,
            PLAYER_OBJ_PRIORITY, 0, 0, -1, PlayerRotateOMExec);
    }
    workP = omObjGetWork(objP, PLAYERROTATEWORK);
    workP->stopUpdateF = FALSE;
    workP->totalFrames = maxTime;
    workP->playerNo = playerNo;
    workP->elapsedFrames = 0;
    objP->rot.y = mbPlayerRotYGet(playerNo);
    objP->scale.z = endAngle;
    angle = fmod(endAngle - objP->rot.y, 360.0f);
    if ((s16)angle == 0) {
        mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 5.0f,
            HU3D_MOTATTR_LOOP);
        workP->stopUpdateF = TRUE;
    } else {
        if (angle < 0.0f) {
            angle += 360.0f;
        }
        if (angle > 180.0f) {
            angle -= 360.0f;
        }
        objP->scale.y = angle;
        if (fabs(angle) > 5.0f) {
            mbPlayerMotionShiftSet(playerNo, 2, 0.0f, 5.0f,
                HU3D_MOTATTR_LOOP);
        } else {
            mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 5.0f,
                HU3D_MOTATTR_LOOP);
        }
    }
}

// Rotation-object update callback that eases the player's yaw to its requested angle.
static void PlayerRotateOMExec(OMOBJ *objP)
{
    PLAYERROTATEWORK *workP = omObjGetWork(objP, PLAYERROTATEWORK);
    float rotY;
    float angle;
    float weight;

    if (workP->stopUpdateF || mbExitCheck()) {
        mbPlayerRotYSet(workP->playerNo, objP->scale.z);
        playerWork[workP->playerNo].rotateObj = NULL;
        omDelObjEx(HuPrcCurrentGet(), objP);
        return;
    }
    angle = (float)(workP->elapsedFrames++) / workP->totalFrames;
    weight = HuSin(angle * 90.0f);
    rotY = objP->rot.y + (weight * objP->scale.y);
    mbPlayerRotYSet(workP->playerNo, rotY);
    if (workP->elapsedFrames >= workP->totalFrames) {
        workP->stopUpdateF = TRUE;
        mbPlayerMotionSet(workP->playerNo, 1, HU3D_MOTATTR_LOOP);
        return;
    }
}

// Reports whether the player's timed rotation object has finished.
BOOL mbPlayerRotateCheck(int playerNo)
{
    return playerWork[playerNo].rotateObj == NULL;
}

// Reports whether every player's timed rotation object has finished.
BOOL mbPlayerRotateCheckAll(void)
{
    int i;

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (playerWork[i].rotateObj != NULL) {
            return FALSE;
        }
    }
    return TRUE;
}

// Called when the player throws a dice; hits the dice object after 27 frames, then idles.
void mbPlayerDiceMotExec(int playerNo)
{
    int time;

    mbPlayerMotionSet(playerNo, 11, HU3D_MOTATTR_NONE);
    time = 0;
    do {
        if (time++ == 27) {
            mbDiceObjHit(playerNo);
        }
        HuPrcVSleep();
    } while (!mbPlayerMotionEndCheck(playerNo));
    mbPlayerMotIdleSet(playerNo);
}

typedef struct MoveNumWork {
    // Stops the move-count display object's update callback.
    u8 killF : 1;
    // Whether the move-count display is visible.
    u8 dispF : 1;
    // Player whose remaining movement is shown.
    u8 playerNo : 2;
    // Records the walking-view creation flag; the display update never reads this bit.
    u8 carF : 1;
} MOVENUMWORK;

static void MoveNumOMExec(OMOBJ *objP);

// Called when the turn UI needs to show remaining moves, including after a dice roll.
// Creates the numbered move display in the requested color; an existing display is left unchanged.
void mbMoveNumCreateColor(int playerNo, BOOL carF, int color)
{
    int modelId;
    HU3D_CAMERA *cameraP;
    OMOBJ *objP;
    MOVENUMWORK *workP;
    int i;
    HuVecF pos;
    HuVecF posNorm;

    cameraP = &Hu3DCamera[0];
    if (playerWork[playerNo].moveNumObj) {
        return;
    }
    playerWork[playerNo].moveNumObj = objP = omAddObjEx(mbObjMan,
        PLAYER_MOVE_NUM_OBJ_PRIORITY, 20, 0, -1, MoveNumOMExec);
    omSetStatBit(objP, OM_STAT_MODELPAUSE);
    workP = omObjGetWork(objP, MOVENUMWORK);
    workP->dispF = TRUE;
    workP->killF = FALSE;
    workP->playerNo = playerNo;
    workP->carF = carF;
    for (i = 0; i < 20; i++) {
        modelId = mbCoinObjCreate(i % 10, color);
        mbCoinObjLayerSet(modelId, 4);
        mbCoinObjDispSet(modelId, FALSE);
        objP->mdlId[i] = modelId;
    }
    mbPlayerPosGet(playerNo, &pos);
    mbPos3DtoNorm(&pos, 1, &posNorm);
    objP->trans.y = posNorm.y;
    pos.y += 300.0f;
    mbPos3DtoNorm(&pos, 1, &posNorm);
    objP->trans.y = posNorm.y - objP->trans.y;
    objP->trans.z = posNorm.z;
    if (carF) {
        Mtx lookAt;
        float tanFov;

        mbPlayerPosGet(playerNo, &pos);
        pos.y += 300.0f;
        MTXLookAt(lookAt, &cameraP->pos, &cameraP->up, &cameraP->target);
        MTXMultVec(lookAt, &pos, &posNorm);
        tanFov = HuSin(cameraP->fov * 0.5f)
            / HuCos(cameraP->fov * 0.5f);
        objP->rot.y = posNorm.y / (tanFov * posNorm.z);
        objP->rot.z = -posNorm.z;
    }
}

// Creates the standard-color move display used by ordinary turn movement.
void mbMoveNumCreate(int playerNo, BOOL carF)
{
    mbMoveNumCreateColor(playerNo, carF, 0);
}

// Move-display object callback; positions visible digits beside the moving player.
static void MoveNumOMExec(OMOBJ *objP)
{
    int digitNum = 0;
    MOVENUMWORK *workP = omObjGetWork(objP, MOVENUMWORK);
    HU3D_CAMERA *cameraP = &Hu3DCamera[0];
    HU3D_CAMERA *camera2P = &Hu3DCamera[2];
    int i;
    HuVecF pos;
    HuVecF posNorm;
    float scaleX;
    float scaleY;
    float tanFov;
    float scale;
    float rotZ;

    if (workP->killF || mbExitCheck()) {
        for (i = 0; i < 20; i++) {
            if (objP->mdlId[i] != -1) {
                mbCoinObjNumDec(objP->mdlId[i]);
                objP->mdlId[i] = -1;
            }
        }
        omDelObjEx(HuPrcCurrentGet(), objP);
        playerWork[workP->playerNo].moveNumObj = NULL;
        return;
    }
    if (mbPauseEnableCheck()) {
        return;
    }
    mbPlayerPosGet(workP->playerNo, &pos);
    mbPos3DtoNorm(&pos, 1, &posNorm);
    posNorm.y += objP->trans.y;
    posNorm.z = objP->trans.z;
    tanFov = HuSin(camera2P->fov * 0.5f)
        / HuCos(camera2P->fov * 0.5f);
    scaleX = 1.2f * (tanFov * -posNorm.z);
    scaleY = tanFov * -posNorm.z;
    posNorm.x *= scaleX;
    posNorm.y *= scaleY;
    HuVecCopy(&posNorm, &pos);
    mbCameraRotGet(&posNorm);
    rotZ = -posNorm.x;
    for (i = 0; i < 20; i++) {
        mbCoinObjDispSet(objP->mdlId[i], FALSE);
    }
    if (workP->dispF) {
        int modelNo;

        scale = HuSin(cameraP->fov * 0.5f)
            / HuCos(cameraP->fov * 0.5f);
        scale = tanFov / scale;
        modelNo = GwPlayer[workP->playerNo].moveNum / 10;
        if (modelNo != 0) {
            mbCoinObjDispSet(objP->mdlId[modelNo], TRUE);
            mbCoinObjPosSet(objP->mdlId[modelNo],
                pos.x - (60.000004f * scale), pos.y, pos.z);
            mbCoinObjRotSet(objP->mdlId[modelNo], rotZ, 0.0f, 0.0f);
            mbCoinObjScaleSet(
                objP->mdlId[modelNo], scale, scale, scale);
            digitNum++;
        }
        modelNo = (GwPlayer[workP->playerNo].moveNum % 10) + 10;
        if (modelNo != 0) {
            mbCoinObjDispSet(objP->mdlId[modelNo], TRUE);
            if (digitNum == 0) {
                mbCoinObjPosSet(
                    objP->mdlId[modelNo], pos.x, pos.y, pos.z);
            } else {
                mbCoinObjPosSet(objP->mdlId[modelNo],
                    pos.x + (60.000004f * scale), pos.y, pos.z);
            }
            mbCoinObjRotSet(objP->mdlId[modelNo], rotZ, 0.0f, 0.0f);
            mbCoinObjScaleSet(
                objP->mdlId[modelNo], scale, scale, scale);
        }
    }
}

// Requests move-display removal; the object callback releases its digit models.
void mbMoveNumKill(int playerNo)
{
    if (playerWork[playerNo].moveNumObj) {
        MOVENUMWORK *workP =
            omObjGetWork(playerWork[playerNo].moveNumObj, MOVENUMWORK);

        workP->killF = TRUE;
    }
}

// Shows or hides the move digits while the display object remains active.
void mbMoveNumDispSet(int playerNo, BOOL dispF)
{
    if (playerWork[playerNo].moveNumObj) {
        MOVENUMWORK *workP =
            omObjGetWork(playerWork[playerNo].moveNumObj, MOVENUMWORK);

        workP->dispF = dispF;
    }
}

static void PlayerColCornerSet(int playerNo, int masuIdNext);
static void PlayerColCornerSnap(int playerNo, int masuId, int cornerNo);
static void PlayerColInit(int playerNo, int masuId, int cornerNo);

// Repositions active players after board positions change, keeping shared spaces clear.
// Called by board movement setup when several players may occupy the same spaces.
void mbev_PlayerColMasuAllSet(int *masuIdFix, BOOL snapF)
{
    BOOL circleF;
    s8 orderNo;
    int i;
    int j;
    int cornerNo;
    int masuId;
    HuVecF pos;

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (playerWork[i].colObj) {
            if (GwPlayer[i].masuId == 0) {
                continue;
            }
            omObjGetWork(playerWork[i].colObj, PLAYERCOLWORK)->masuIdNext =
                GwPlayer[i].masuIdNext;
            if (omObjGetWork(playerWork[i].colObj, PLAYERCOLWORK)->notRestF) {
                continue;
            }
        }
        if (masuIdFix[i] >= 0) {
            mbPlayerMasuCornerSet(i, 0);
            PlayerColCornerSnap(i, masuIdFix[i], 0);
            continue;
        }
        if (playerWork[i].colObj) {
            if (!omObjGetWork(playerWork[i].colObj, PLAYERCOLWORK)->snapF) {
                continue;
            }
        }
        masuId = GwPlayer[i].masuId;
        orderNo = GwPlayer[i].orderNo;
        cornerNo = 0;
        for (j = 0; j < GW_PLAYER_MAX; j++) {
            if (i != j && masuId == masuIdFix[j]) {
                cornerNo++;
            }
        }
        for (j = 0; j < GW_PLAYER_MAX; j++) {
            if (i != j && masuIdFix[j] < 0 &&
                masuId == GwPlayer[j].masuId &&
                orderNo > GwPlayer[j].orderNo) {
                cornerNo++;
            }
        }
        if (cornerNo == 0) {
            mbMasuPosGet(masuId, &pos);
        } else {
            mbMasuCornerRotPosGet(masuId, cornerNo - 1, &pos);
        }
        {
            circleF = omObjGetWork(playerWork[i].colObj, PLAYERCOLWORK)->circleF;

            omObjGetWork(playerWork[i].colObj, PLAYERCOLWORK)->circleF = FALSE;
            if (snapF) {
                mbPlayerPosSetV(i, &pos);
                PlayerColCornerSnap(i, masuId, cornerNo);
            } else if (cornerNo != mbPlayerMasuCornerGet(i) || circleF) {
                PlayerColInit(i, masuId, cornerNo);
            }
        }
        mbPlayerMasuCornerSet(i, cornerNo);
    }
}

// Assigns corner positions to players on one space, optionally snapping them now.
// Called when a player's board-space position is set.
void mbev_PlayerColMasu(int playerNo, int masuId, BOOL snapF)
{
    HuVecF pos;
    int orderNo[GW_PLAYER_MAX];
    int playerNoTbl[GW_PLAYER_MAX];
    BOOL circleF;
    int num = 0;
    int i;
    int j;
    int cornerNo;

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (playerNo != i) {
            if (GwPlayer[i].masuId == 0) {
                continue;
            }
            if (masuId != GwPlayer[i].masuId) {
                continue;
            }
        }
        orderNo[num] = GwPlayer[i].orderNo;
        if (playerNo == i) {
            // Put the requested player first before assigning the group's corners.
            orderNo[num] = -1;
        }
        playerNoTbl[num] = i;
        num++;
        mbPlayerColSnapPlayerSet(i, TRUE);
        mbPlayerColRestSet(i, TRUE);
    }
    for (i = 0; i < num - 1; i++) {
        for (j = i + 1; j < num; j++) {
            if (orderNo[i] > orderNo[j]) {
                cornerNo = orderNo[i];
                orderNo[i] = orderNo[j];
                orderNo[j] = cornerNo;
                cornerNo = playerNoTbl[i];
                playerNoTbl[i] = playerNoTbl[j];
                playerNoTbl[j] = cornerNo;
            }
        }
    }
    for (j = 0; j < num; j++) {
        i = playerNoTbl[j];
        cornerNo = j;

        if (cornerNo != 0) {
            mbMasuCornerRotPosGet(masuId, cornerNo - 1, &pos);
        } else {
            mbMasuPosGet(masuId, &pos);
        }
        circleF =
            omObjGetWork(playerWork[i].colObj, PLAYERCOLWORK)->circleF;
        omObjGetWork(playerWork[i].colObj, PLAYERCOLWORK)->circleF = FALSE;
        if (snapF) {
            mbPlayerPosSetV(i, &pos);
            PlayerColCornerSnap(i, masuId, cornerNo);
        } else if (cornerNo != mbPlayerMasuCornerGet(i) || circleF) {
            PlayerColInit(i, masuId, cornerNo);
        }
        mbPlayerMasuCornerSet(i, cornerNo);
    }
}

// Board events place the requested player at the space center and others at the requested radius.
// With playerNo negative, every occupant uses an outer corner.
void mbev_PlayerColCircleAdd(
    int playerNo, int masuId, BOOL snapF, float radius)
{
    HuVecF posCenter;
    HuVecF pos;
    int orderNo[GW_PLAYER_MAX];
    int playerNoTbl[GW_PLAYER_MAX];
    int num = 0;
    int i;
    int j;
    int cornerNo;

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (playerNo != i) {
            if (GwPlayer[i].masuId == 0) {
                continue;
            }
            if (masuId != GwPlayer[i].masuId) {
                continue;
            }
        }
        omObjGetWork(playerWork[i].colObj, PLAYERCOLWORK)->circleF = TRUE;
        omObjGetWork(playerWork[i].colObj, PLAYERCOLWORK)->radius = radius;
        orderNo[num] = GwPlayer[i].orderNo;
        if (playerNo == i) {
            // Put the requested player first before assigning the group's corners.
            orderNo[num] = -1;
        }
        playerNoTbl[num] = i;
        num++;
        mbPlayerColSnapPlayerSet(i, TRUE);
        mbPlayerColRestSet(i, TRUE);
    }
    for (i = 0; i < num - 1; i++) {
        for (j = i + 1; j < num; j++) {
            if (orderNo[i] > orderNo[j]) {
                cornerNo = orderNo[i];
                orderNo[i] = orderNo[j];
                orderNo[j] = cornerNo;
                cornerNo = playerNoTbl[i];
                playerNoTbl[i] = playerNoTbl[j];
                playerNoTbl[j] = cornerNo;
            }
        }
    }
    for (j = 0; j < num; j++) {
        i = playerNoTbl[j];
        cornerNo = j;

        if (playerNo < 0 && cornerNo == 0) {
            // The negative-player form moves the first-ranked player to corner num.
            cornerNo = num;
        }
        mbMasuPosGet(masuId, &posCenter);
        if (cornerNo != 0) {
            float scale;

            mbMasuCornerRotPosGet(masuId, cornerNo - 1, &pos);
            VECSubtract(&pos, &posCenter, &pos);
            scale = radius / VECMag(&pos);
            VECScale(&pos, &pos, scale);
            VECAdd(&posCenter, &pos, &posCenter);
        }
        if (snapF) {
            mbPlayerPosSetV(i, &posCenter);
            PlayerColCornerSnap(i, masuId, cornerNo);
        } else {
            PlayerColInit(i, masuId, cornerNo);
        }
        mbPlayerMasuCornerSet(i, cornerNo);
    }
}

// Shifts the other players on a space when one player joins their group.
// Called by board events that place a player onto an occupied space.
void mbev_PlayerColMasuAdd(int playerNo, int masuId, BOOL snapF)
{
    BOOL circleF;
    HuVecF pos;
    int i;
    int j;

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        s8 orderNo;
        int cornerNo;

        if (masuId != GwPlayer[i].masuId || i == playerNo) {
            continue;
        }
        if (playerWork[i].colObj) {
            omObjGetWork(playerWork[i].colObj, PLAYERCOLWORK)->masuIdNext =
                GwPlayer[i].masuIdNext;
            if (omObjGetWork(playerWork[i].colObj, PLAYERCOLWORK)->notRestF ||
                !omObjGetWork(playerWork[i].colObj, PLAYERCOLWORK)->snapF) {
                continue;
            }
        }
        orderNo = GwPlayer[i].orderNo;
        cornerNo = 1;
        if (playerNo < 0) {
            cornerNo = 0;
        }
        for (j = 0; j < GW_PLAYER_MAX; j++) {
            if (i != j && masuId == GwPlayer[j].masuId &&
                orderNo > GwPlayer[j].orderNo) {
                cornerNo++;
            }
        }
        if (cornerNo != 0) {
            mbMasuCornerRotPosGet(masuId, cornerNo - 1, &pos);
        } else {
            mbMasuPosGet(masuId, &pos);
        }
        circleF =
            omObjGetWork(playerWork[i].colObj, PLAYERCOLWORK)->circleF;
        omObjGetWork(playerWork[i].colObj, PLAYERCOLWORK)->circleF = FALSE;
        if (snapF) {
            mbPlayerPosSetV(i, &pos);
            PlayerColCornerSnap(i, masuId, cornerNo);
        } else if (cornerNo != mbPlayerMasuCornerGet(i) || circleF) {
            PlayerColInit(i, masuId, cornerNo);
        }
        mbPlayerMasuCornerSet(i, cornerNo);
    }
    if (playerNo >= 0) {
        mbPlayerMasuCornerSet(playerNo, 0);
    }
}

// Returns positions for a selected group of players arranged around a space.
// Called by board events that arrange players for a shared-space scene.
void mbev_PlayerColBall(int masuId, int *playerNoTbl, HuVecF *posTbl)
{
    int orderNo[GW_PLAYER_MAX];
    int playerNoSort[GW_PLAYER_MAX];
    int useF[GW_PLAYER_MAX] = { 0, 0, 0, 0 };
    HuVecF posByPlayer[GW_PLAYER_MAX];
    int num;
    int i;
    int j;
    int cornerNo;

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (playerNoTbl[i] >= 0) {
            useF[playerNoTbl[i]] = TRUE;
        }
    }
    num = 0;
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (masuId != GwPlayer[i].masuId && !useF[i]) {
            continue;
        }
        orderNo[num] = GwPlayer[i].orderNo;
        if (orderNo[num] != 0 && !mbPlayerColSnapGet(i)) {
            // Sort players with snapping disabled later, except those with order zero.
            orderNo[num] += GW_PLAYER_MAX;
        }
        playerNoSort[num] = i;
        num++;
    }
    for (i = 0; i < num - 1; i++) {
        for (j = i + 1; j < num; j++) {
            if (orderNo[i] > orderNo[j]) {
                cornerNo = orderNo[i];

                orderNo[i] = orderNo[j];
                orderNo[j] = cornerNo;
                cornerNo = playerNoSort[i];
                playerNoSort[i] = playerNoSort[j];
                playerNoSort[j] = cornerNo;
            }
        }
    }
    for (j = 0; j < num; j++) {
        i = playerNoSort[j];
        cornerNo = j;

        if (cornerNo != 0) {
            mbMasuCornerRotPosGet(masuId, cornerNo - 1, &posByPlayer[i]);
        } else {
            mbMasuPosGet(masuId, &posByPlayer[i]);
        }
    }
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (playerNoTbl[i] >= 0) {
            HuVecCopy(&posByPlayer[playerNoTbl[i]], &posTbl[i]);
        }
    }
}

// Refreshes the group's collision positions using the supplied space for one player.
void mbev_PlayerColMasuSet(int playerNo, int masuId, BOOL snapF)
{
    int masuIdTbl[GW_PLAYER_MAX];
    int i;

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        masuIdTbl[i] = -1;
    }
    masuIdTbl[playerNo] = masuId;
    mbev_PlayerColMasuAllSet(masuIdTbl, snapF);
}

// Reassigns snapped players when this player's next space changes.
static void PlayerColCornerSet(int playerNo, int masuIdNext)
{
    BOOL circleF;
    int masuIdFix[GW_PLAYER_MAX];
    int i;
    int j;
    int cornerNo;
    int masuId;
    s8 orderNo;
    HuVecF pos;

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        masuIdFix[i] = -1;
    }
    masuIdFix[playerNo] = masuIdNext;
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (playerWork[i].colObj == NULL) {
            continue;
        }
        if (GwPlayer[i].masuId == 0) {
            continue;
        }
        omObjGetWork(playerWork[i].colObj, PLAYERCOLWORK)->masuIdNext =
            GwPlayer[i].masuIdNext;
        if (i == playerNo) {
            continue;
        }
        if (omObjGetWork(playerWork[i].colObj, PLAYERCOLWORK)->notRestF ||
            !omObjGetWork(playerWork[i].colObj, PLAYERCOLWORK)->snapF) {
            continue;
        }
        masuId = GwPlayer[i].masuId;
        orderNo = GwPlayer[i].orderNo;
        cornerNo = 0;
        for (j = 0; j < GW_PLAYER_MAX; j++) {
            if (i != j && masuId == masuIdFix[j]) {
                cornerNo++;
            }
        }
        for (j = 0; j < GW_PLAYER_MAX; j++) {
            if (i != j && masuIdFix[j] < 0 &&
                masuId == GwPlayer[j].masuId &&
                orderNo > GwPlayer[j].orderNo) {
                cornerNo++;
            }
        }
        if (cornerNo == 0) {
            mbMasuPosGet(masuId, &pos);
        } else {
            mbMasuCornerRotPosGet(masuId, cornerNo - 1, &pos);
        }
        circleF =
            omObjGetWork(playerWork[i].colObj, PLAYERCOLWORK)->circleF;
        omObjGetWork(playerWork[i].colObj, PLAYERCOLWORK)->circleF = FALSE;
        if (cornerNo != mbPlayerMasuCornerGet(i) || circleF) {
            PlayerColInit(i, masuId, cornerNo);
        }
        mbPlayerMasuCornerSet(i, cornerNo);
    }
}

// When snapping is enabled, records the current model position relative to the supplied space.
// Stores the requested corner and resets collision motion state and yaw.
static void PlayerColCornerSnap(int playerNo, int masuId, int cornerNo)
{
    MBPLAYERWORK *playerWorkP = mbPlayerWorkGet(playerNo);
    PLAYERCOLWORK *workP =
        omObjGetWork(playerWorkP->colObj, PLAYERCOLWORK);
    Mtx masuMtx;
    Mtx masuMtxInv;
    HuVecF pos;

    if (workP->snapF) {
        workP->motStartF = FALSE;
        workP->state = 0;
        workP->resyncF = FALSE;
        workP->masuId = masuId;
        playerWorkP->masuCorner = cornerNo;
        mbMasuMtxGet(workP->masuId, masuMtx);
        MTXInverse(masuMtx, masuMtxInv);
        mbPlayerPosGet(playerNo, &pos);
        MTXMultVec(masuMtxInv, &pos, &playerWork[playerNo]._unk3C);
        MTXMultVec(
            masuMtxInv, &pos, &playerWork[playerNo].colObj->trans);
        mbPlayerRotYSet(playerNo, 0.0f);
        workP->rotYStart = 0.0f;
        GwPlayer[playerNo].moveF = FALSE;
    }
}

// When snapping is enabled, starts a walk from the player's current position to a new space corner.
static void PlayerColInit(int playerNo, int masuId, int cornerNo)
{
    MBPLAYERWORK *playerWorkP = mbPlayerWorkGet(playerNo);
    PLAYERCOLWORK *workP =
        omObjGetWork(playerWorkP->colObj, PLAYERCOLWORK);
    Mtx masuMtx;
    Mtx masuMtxInv;
    HuVecF posPlayer;
    HuVecF posMasu;
    HuVecF posDiff;
    float rotY;

    if (workP->snapF) {
        workP->motStartF = FALSE;
        workP->state = 1;
        workP->resyncF = FALSE;
        workP->masuId = masuId;
        playerWorkP->masuCorner = cornerNo;
        mbMasuMtxGet(workP->masuId, masuMtx);
        MTXInverse(masuMtx, masuMtxInv);
        mbPlayerPosGet(playerNo, &posPlayer);
        MTXMultVec(
            masuMtxInv, &posPlayer, &playerWork[playerNo].colObj->trans);
        GwPlayer[playerNo].moveF = TRUE;
        if (playerWorkP->masuCorner == 0) {
            mbMasuPosGet(workP->masuId, &posMasu);
        } else {
            mbMasuCornerRotPosGet(workP->masuId,
                playerWorkP->masuCorner - 1, &posMasu);
            if (workP->circleF) {
                float scale;

                mbMasuPosGet(workP->masuId, &posDiff);
                VECSubtract(&posMasu, &posDiff, &posMasu);
                scale = workP->radius / VECMag(&posMasu);
                VECScale(&posMasu, &posMasu, scale);
                VECAdd(&posMasu, &posDiff, &posMasu);
            }
        }
        VECSubtract(&posMasu, &posPlayer, &posDiff);
        rotY = 90.0f - HuAtan(posDiff.z, posDiff.x);
        mbPlayerRotYSet(playerNo, rotY);
        workP->rotYStart = rotY;
    }
}

// Reports whether all player collision markers have finished their transitions.
BOOL mbPlayerColCheck(void)
{
    int i;

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        PLAYERCOLWORK *workP =
            omObjGetWork(playerWork[i].colObj, PLAYERCOLWORK);

        if (workP->state) {
            return FALSE;
        }
    }
    return TRUE;
}

static void PlayerColKill(int playerNo)
{
}

// Reserves a space for one player and updates the positions of its group.
void mbev_PlayerColReserve(int playerNo, int masuId, BOOL snapF)
{
    int masuIdTbl[GW_PLAYER_MAX];
    int i;

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        masuIdTbl[i] = -1;
    }
    masuIdTbl[playerNo] = masuId;
    mbev_PlayerColMasuAllSet(masuIdTbl, snapF);
}

// Per-frame callback that animates a player's collision marker and turn.
// Installed as the callback for the player's collision object.
static void PlayerColOMExec(OMOBJ *obj)
{
    PLAYERCOLWORK *workP = omObjGetWork(obj, PLAYERCOLWORK);
    int playerNo = workP->playerNo;
    MBPLAYERWORK *playerWorkP = mbPlayerWorkGet(playerNo);
    Mtx masuMtx;
    Mtx masuMtxInv;
    HuVecF pos;
    HuVecF posDiff;
    float weight;
    float rotY;

    if (mbExitCheck()) {
        omDelObjEx(HuPrcCurrentGet(), obj);
        playerWorkP->colObj = NULL;
        return;
    }
    if (GwPlayer[playerNo].masuId == 0) {
        return;
    }
    if (workP->masuIdNext != GwPlayer[playerNo].masuIdNext) {
        workP->masuIdNext = GwPlayer[playerNo].masuIdNext;
        PlayerColCornerSet(playerNo, workP->masuIdNext);
    }
    if (!workP->snapF || playerWorkP->moveObj || playerWorkP->posFixObj) {
        workP->resyncF = TRUE;
        workP->state = 0;
        return;
    }
    if (workP->resyncF) {
        workP->resyncF = FALSE;
        workP->masuId = GwPlayer[playerNo].masuId;
        workP->masuIdNext = GwPlayer[playerNo].masuIdNext;
        mbMasuMtxGet(workP->masuId, masuMtx);
        MTXInverse(masuMtx, masuMtxInv);
        mbPlayerPosGet(playerNo, &pos);
        MTXMultVec(masuMtxInv, &pos, &playerWorkP->_unk3C);
        MTXMultVec(masuMtxInv, &pos, &obj->trans);
        playerWorkP->_unk3C.y = obj->trans.y = 0.0f;
    }
    switch (workP->state) {
        case 0:
            obj->trans.x = playerWorkP->_unk3C.x;
            obj->trans.y = playerWorkP->_unk3C.y;
            obj->trans.z = playerWorkP->_unk3C.z;
            break;

        case 1:
            if (!workP->motStartF) {
                mbPlayerMotionSet(playerNo, 3, HU3D_MOTATTR_LOOP);
                workP->motStartF = TRUE;
                workP->time = 0;
                workP->maxTime = 12;
                obj->rot.x = obj->trans.x;
                obj->rot.y = obj->trans.y;
                obj->rot.z = obj->trans.z;
            }
            if (playerWorkP->masuCorner == 0) {
                playerWorkP->_unk3C.x = playerWorkP->_unk3C.y =
                    playerWorkP->_unk3C.z = 0.0f;
            } else {
                mbMasuCornerPosGet(workP->masuId,
                    playerWorkP->masuCorner - 1, &playerWorkP->_unk3C);
                if (workP->circleF) {
                    weight = workP->radius / VECMag(&playerWorkP->_unk3C);
                    VECScale(&playerWorkP->_unk3C,
                        &playerWorkP->_unk3C, weight);
                }
            }
            if (workP->time > workP->maxTime) {
                workP->state = 2;
                workP->time = 0;
                workP->maxTime = 8;
                obj->trans.x = playerWorkP->_unk3C.x;
                obj->trans.y = playerWorkP->_unk3C.y;
                obj->trans.z = playerWorkP->_unk3C.z;
                mbPlayerMotionShiftSet(
                    playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            } else {
                weight = (float)(workP->time++) / workP->maxTime;
                VECSubtract(&playerWorkP->_unk3C, &obj->rot, &posDiff);
                obj->trans.x = obj->rot.x +
                    (weight * (playerWorkP->_unk3C.x - obj->rot.x));
                obj->trans.y = obj->rot.y +
                    (weight * (playerWorkP->_unk3C.y - obj->rot.y));
                obj->trans.z = obj->rot.z +
                    (weight * (playerWorkP->_unk3C.z - obj->rot.z));
            }
            break;

        case 2:
            // Completion clears movement, but the interpolation below still overwrites yaw zero.
            if (workP->time > workP->maxTime) {
                mbPlayerRotYSet(playerNo, 0.0f);
                GwPlayer[playerNo].moveF = FALSE;
                workP->state = 0;
            }
            weight = (float)(workP->time++) / workP->maxTime;
            rotY = mbAngleEaseOut(workP->rotYStart, 0.0f, weight);
            mbPlayerRotYSet(playerNo, rotY);
            break;
    }
    mbMasuMtxGet(workP->masuId, masuMtx);
    MTXMultVec(masuMtx, &obj->trans, &pos);
    mbPlayerPosSetV(playerNo, &pos);
}

// Ignores playerNo; with global snapping enabled, snaps stationary players on the selected space.
// Individual snapF flags gate only the collision-state reset; players with snapF false still move.
void mbev_PlayerColSet(int playerNo, int masuId)
{
    int i;
    int cornerNo;
    HuVecF pos;
    Mtx masuMtxInv;
    Mtx masuMtx;
    HuVecF posPlayer;

    if (!playerColSnapF) {
        return;
    }
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        MBPLAYERWORK *playerWorkP;
        PLAYERCOLWORK *workP;

        if (GwPlayer[i].moveF) {
            continue;
        }
        if (masuId != GwPlayer[i].masuId) {
            continue;
        }
        cornerNo = mbPlayerMasuCornerGet(i);
        if (cornerNo == 0) {
            mbMasuPosGet(masuId, &pos);
        } else {
            mbMasuCornerRotPosGet(masuId, cornerNo - 1, &pos);
        }
        mbPlayerPosSetV(i, &pos);
        playerWorkP = mbPlayerWorkGet(i);
        workP = omObjGetWork(playerWorkP->colObj, PLAYERCOLWORK);
        if (!workP->snapF) {
            continue;
        }
        workP->motStartF = FALSE;
        workP->state = 0;
        workP->resyncF = FALSE;
        workP->masuId = masuId;
        playerWorkP->masuCorner = cornerNo;
        mbMasuMtxGet(workP->masuId, masuMtx);
        MTXInverse(masuMtx, masuMtxInv);
        mbPlayerPosGet(i, &posPlayer);
        MTXMultVec(masuMtxInv, &posPlayer, &playerWork[i]._unk3C);
        MTXMultVec(
            masuMtxInv, &posPlayer, &playerWork[i].colObj->trans);
        mbPlayerRotYSet(i, 0.0f);
        workP->rotYStart = 0.0f;
        GwPlayer[i].moveF = FALSE;
    }
}

// Enables or disables snapping for every active player collision object.
void mbPlayerColSnapSet(BOOL snapF)
{
    BOOL snap = snapF ? TRUE : FALSE;
    int i;

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (playerWork[i].colObj) {
            PLAYERCOLWORK *workP =
                omObjGetWork(playerWork[i].colObj, PLAYERCOLWORK);

            workP->snapF = snap;
        }
    }
    playerColSnapF = snapF;
}

// Enables or disables snapping for one player's collision object.
void mbPlayerColSnapPlayerSet(int playerNo, BOOL snapF)
{
    BOOL snap = snapF ? TRUE : FALSE;

    if (playerWork[playerNo].colObj) {
        PLAYERCOLWORK *workP =
            omObjGetWork(playerWork[playerNo].colObj, PLAYERCOLWORK);

        workP->snapF = snap;
    }
}

// Returns whether one player's collision object is currently allowed to snap.
BOOL mbPlayerColSnapGet(int playerNo)
{
    PLAYERCOLWORK *workP =
        omObjGetWork(playerWork[playerNo].colObj, PLAYERCOLWORK);

    return workP->snapF;
}

// Updates one player's collision rest flag; the stored flag is inverse to restF.
void mbPlayerColRestSet(int playerNo, BOOL restF)
{
    BOOL rest = restF ? FALSE : TRUE;

    if (playerWork[playerNo].colObj) {
        PLAYERCOLWORK *workP =
            omObjGetWork(playerWork[playerNo].colObj, PLAYERCOLWORK);

        workP->notRestF = rest;
    }
}

// Gives one player first order and assigns the remaining players afterward.
void mbPlayerColFirstSet(int playerNo)
{
    int orderNo = 1;
    int i;

    GwPlayer[playerNo].orderNo = 0;
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (playerNo != i) {
            GwPlayer[i].orderNo = orderNo++;
        }
    }
}

// Reassigns collision-placement order within each occupied space according to corner order.
void mbPlayerColOrderReset(void)
{
    s8 playerNo[GW_PLAYER_MAX];
    s8 orderNo[GW_PLAYER_MAX];
    s8 fixF[GW_PLAYER_MAX];
    int i;
    int j;
    int k;
    int num;
    s16 masuId;
    s8 orderNoSwap;

    memset(fixF, 0, GW_PLAYER_MAX);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        PLAYERCOLWORK *workP;
        BOOL restF = FALSE;

        if (playerWork[i].colObj) {
            workP = omObjGetWork(playerWork[i].colObj, PLAYERCOLWORK);

            workP->notRestF = restF;
        }
        if (GwPlayer[i].masuId == 0) {
            continue;
        }
        if (fixF[i]) {
            continue;
        }
        masuId = GwPlayer[i].masuId;
        playerNo[0] = i;
        orderNo[0] = GwPlayer[i].orderNo;
        num = 1;
        for (j = 0; j < GW_PLAYER_MAX; j++) {
            if (i != j && masuId == GwPlayer[j].masuId) {
                playerNo[num] = j;
                orderNo[num] = GwPlayer[j].orderNo;
                num++;
            }
        }
        if (num > 1) {
            for (j = 0; j < num - 1; j++) {
                for (k = j + 1; k < num; k++) {
                    if (orderNo[j] > orderNo[k]) {
                        orderNoSwap = orderNo[j];
                        orderNo[j] = orderNo[k];
                        orderNo[k] = orderNoSwap;
                    }
                }
            }
        }
        for (j = 0; j < num; j++) {
            GwPlayer[playerNo[j]].orderNo =
                orderNo[mbPlayerMasuCornerGet(playerNo[j])];
            fixF[playerNo[j]] = TRUE;
        }
    }
}

static char *eyeMatNameTbl[CHARNO_MAX][2] = {
    { "eye1", "eye2" },
    { "eye1", "eye2" },
    { "mat14", "mat16" },
    { "eye1", "eye2" },
    { "Clswario_eye_l1_AUTO14", "Clswario_eye_l1_AUTO15" },
    { "m_donkey_eye4", "m_donkey_eye5" },
    { "mat65", "mat66" },
    { "Clswaluigi_eye_l1_AUTO1", "Clswaluigi_eye_l1_AUTO2" }
};

// Darkens non-eye materials when eye names are available; later character rows pass null names to
// strcmp.
// When darkF is false, restores the saved materials.
void mbPlayerEyeMatDarkSet(int playerNo, BOOL darkF)
{
    BOOL validF;
    HU3D_MODELID modelId = mbObjModelIDGet(mbPlayerObjIDGet(playerNo));
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HSF_DATA *hsf = modelP->hsf;
    HSF_MATERIAL *matP = hsf->material;
    HSF_MATERIAL *matCopy = playerWork[playerNo].matCopy;

    if (darkF) {
        char **name = &eyeMatNameTbl[GwPlayer[playerNo].charNo][0];
        int i;
        int j;

        for (i = 0; i < hsf->materialNum; i++, matP++, matCopy++) {
            validF = TRUE;
            for (j = 0; j < matP->attrNum; j++) {
                HSF_ATTRIBUTE *attrP = &hsf->attribute[matP->attr[j]];

                if (strcmp(name[0], attrP->bitmap->name) == 0
                    || strcmp(name[1], attrP->bitmap->name) == 0) {
                    validF = FALSE;
                }
            }
            if (validF) {
                if (darkF) {
                    matP->color[0] *= 0.0f;
                    matP->color[1] *= 0.0f;
                    matP->color[2] *= 0.0f;
                } else {
                    matP->color[0] = matCopy->color[0];
                    matP->color[1] = matCopy->color[1];
                    matP->color[2] = matCopy->color[2];
                }
            }
        }
    } else {
        memcpy(hsf->material, matCopy,
            hsf->materialNum * sizeof(HSF_MATERIAL));
    }
    DCStoreRange(hsf->material, hsf->materialNum * sizeof(HSF_MATERIAL));
}

// Saves the player's current materials so later effects can restore them.
void mbPlayerMatClone(int playerNo)
{
    HU3D_MODELID modelId = mbObjModelIDGet(mbPlayerObjIDGet(playerNo));
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HSF_DATA *hsf = modelP->hsf;
    int size = hsf->materialNum * sizeof(HSF_MATERIAL);
    void *materialData =
        HuMemDirectMallocNum(HEAP_HEAP, size, HU_MEMNUM_OVL);
    HSF_MATERIAL *matP = materialData;
    HSF_MATERIAL *material = matP;

    memcpy(material, hsf->material, hsf->materialNum * sizeof(HSF_MATERIAL));
    playerWork[playerNo].matCopy = material;
}

typedef struct PlayerMetalWork {
    // Requests removal of the metal-effect object.
    u8 killF : 1;
    // Set after the metal appearance has finished fading in.
    u8 fadeInDoneF : 1;
    // Starts the fade-out after the player's metal status ends.
    u8 fadeOutF : 1;
    // Enables the metal particle effect while the player is visible.
    u8 effectF : 1;
    // Player represented by this metal-effect object.
    u8 playerNo : 2;
    // Elapsed frames in the current metal fade.
    s16 time;
    // Total frames in the current metal fade.
    s16 maxTime;
    // Index of the largest mesh in the character model's object array.
    s16 meshObjectIndex;
    // Number of occupied vertex groups in the five-by-five-by-five grid.
    s16 vertexGroupCount;
} PLAYERMETALWORK;

typedef struct PlayerBiriQWork {
    // Requests removal of the electric-effect object.
    u8 killF : 1;
    // Marks completion of the initial electric flash.
    u8 initialFlashDoneF : 1;
    // Requests a new electric flash.
    u8 flashF : 1;
    // Set when the player's electric status ends.
    u8 statusEndedF : 1;
    // Enables the electric particle effect while the player is visible.
    u8 effectF : 1;
    // Player represented by this electric-effect object.
    u8 playerNo : 2;
    // Elapsed frames in the current flash.
    s16 time;
    // Total frames in the current flash.
    s16 maxTime;
    // Index of the largest mesh in the character model's object array.
    s16 meshObjectIndex;
    // Number of occupied vertex groups in the five-by-five-by-five grid.
    s16 vertexGroupCount;
} PLAYERBIRIQWORK;

static void PlayerBiriQEffectSet(int playerNo, BOOL effectF);

// Per-frame callback that blends the metal effect in or out and removes it.
// Installed as the callback for the player's metal-effect object.
static void PlayerMetalOMExec(OMOBJ *objP)
{
    PLAYERMETALWORK *workP = omObjGetWork(objP, PLAYERMETALWORK);
    BOOL killF = FALSE;
    float weight;

    if (mbExitCheck() || workP->killF) {
        if (workP->killF) {
            killF = TRUE;
        }
        workP->killF = TRUE;
    }
    if (!workP->killF) {
        if (!GwPlayer[workP->playerNo].metalF && !workP->fadeOutF) {
            workP->fadeOutF = TRUE;
            workP->time = 0;
            workP->maxTime = 20;
        }
        if (workP->fadeOutF) {
            workP->time++;
            weight = (float)workP->time / workP->maxTime;
            if (weight > 1.0f) {
                weight = 1.0f;
            }
            mbObjMetalTPLvlSet(
                mbPlayerObjIDGet(workP->playerNo), 1.0f - weight);
            if (workP->time >= workP->maxTime) {
                workP->killF = TRUE;
            }
        } else if (!workP->fadeInDoneF) {
            workP->time++;
            weight = (float)workP->time / workP->maxTime;
            if (weight > 1.0f) {
                weight = 1.0f;
            }
            mbObjMetalTPLvlSet(mbPlayerObjIDGet(workP->playerNo), weight);
            if (workP->time >= workP->maxTime) {
                workP->fadeInDoneF = TRUE;
            }
        }
        if (objP->mdlId[0] >= 0) {
            if (!mbObjGet(mbPlayerObjIDGet(workP->playerNo))->dispF
                || !workP->effectF) {
                Hu3DModelAttrSet(objP->mdlId[0], HU3D_ATTR_DISPOFF);
            } else {
                Hu3DModelAttrReset(objP->mdlId[0], HU3D_ATTR_DISPOFF);
            }
        }
    }
    if (workP->killF) {
        if (!killF) {
            PlayerMetalKill(workP->playerNo);
        }
        if (objP->data) {
            void *dataP = objP->data;

            HuMemDirectFree(dataP);
        }
        omDelObjEx(HuPrcCurrentGet(), objP);
        return;
    }
}

// Starts or requests removal of the player's metal transformation.
void mbPlayerMetalSet(int playerNo, BOOL metalF)
{
    OMOBJ *objP;
    PLAYERMETALWORK *workP;

    if (metalF) {
        if (playerWork[playerNo].metalObj != NULL) {
            OSReport("------------already METAL!!----------");
        }
        GwPlayer[playerNo].metalF = TRUE;
        objP = playerWork[playerNo].metalObj;
        if (objP == NULL) {
            objP = playerWork[playerNo].metalObj = omAddObjEx(mbObjMan,
                PLAYER_OBJ_PRIORITY, 1, 0, -1, PlayerMetalOMExec);
            omSetStatBit(objP, OM_STAT_MODELPAUSE);
            mbObjMetalCreate(mbPlayerObjIDGet(playerNo));
            mbObjMetalTPLvlSet(mbPlayerObjIDGet(playerNo), 0.0f);
            mbObjMetalColorSet(mbPlayerObjIDGet(playerNo),
                metalShadowColor, metalHiliteColor);
        }
        workP = omObjGetWork(objP, PLAYERMETALWORK);
        workP->playerNo = playerNo;
        workP->fadeOutF = FALSE;
        workP->fadeInDoneF = FALSE;
        workP->effectF = TRUE;
        workP->time = 0;
        workP->maxTime = 20;
        if (objP->mdlId[0] <= 0) {
            MetalEffectCreate(objP);
        }
        CharModelStepSet(GwPlayer[playerNo].charNo, 6);
    } else {
        GwPlayer[playerNo].metalF = FALSE;
        CharModelStepSet(GwPlayer[playerNo].charNo, 0);
    }
}

// Stops the player's metal model effect and marks its object for deletion.
static void PlayerMetalKill(int playerNo)
{
    OMOBJ *objP = playerWork[playerNo].metalObj;

    if (objP != NULL) {
        PLAYERMETALWORK *workP = omObjGetWork(objP, PLAYERMETALWORK);

        workP->killF = TRUE;
        playerWork[playerNo].metalObj = NULL;
        mbObjMetalKill(mbPlayerObjIDGet(playerNo));
        mbParticleKill(objP->mdlId[0]);
        objP->mdlId[0] = -1;
    }
}

// Enables or hides the player's metal and electric particles; material effects remain active.
void mbPlayerEffectSet(int playerNo, BOOL effectF)
{
    OMOBJ *objP = playerWork[playerNo].metalObj;

    if (objP) {
        PLAYERMETALWORK *workP = omObjGetWork(objP, PLAYERMETALWORK);

        workP->effectF = effectF;
    }
    PlayerBiriQEffectSet(playerNo, effectF);
}

// Restores the default shadow and highlight colors used by the metal effect.
static void ResetMetalColor(void)
{
    metalShadowColor = metalDefaultColor[0];
    metalHiliteColor = metalDefaultColor[1];
}

// Sets the colors used for the metal transformation's shadow and highlights.
void mbPlayerMetalColorSet(
    const GXColor *shadowColor, const GXColor *hiliteColor)
{
    metalShadowColor = *shadowColor;
    metalHiliteColor = *hiliteColor;
}

// Builds vertex groups for the electric effect and returns the mesh height.
static float GetBiriQEffectRadius(
    OMOBJ *objP, int playerNo, int *meshGroupInfo)
{
    int count;
    HSF_DATA *hsfP;
    HuVecF size;
    HuVecF min;
    HuVecF pos;
    int vertexNum;
    int objectNo[2];
    int groupNum;
    int maxVertexNum;
    HSF_OBJECT *objectP;
    HSF_OBJECT *meshP;
    HuVecF *vertexP;
    int randomNo;
    s16 *countP;
    int groupNo;
    s16 *groupP;
    s16 *vertexNoP;
    void *dataP;
    int i;
    int x;
    int y;
    int z;
    float meshHeight;

    // Both assignments initialize the first mesh-index slot; the second entry is unused.
    objectNo[0] = objectNo[0] = -1;
    maxVertexNum = groupNum = 0;
    hsfP = Hu3DData[mbPlayerModelIDGet(playerNo)].hsf;
    objectP = hsfP->object;
    for (i = 0; i < hsfP->objectNum; i++, objectP++) {
        meshP = objectP;
        if (meshP->type == HSF_OBJ_MESH
            && meshP->mesh.vertex->count > 0) {
            vertexNum = meshP->mesh.vertex->count;

            if (maxVertexNum < vertexNum) {
                maxVertexNum = vertexNum;
                objectNo[0] = i;
            }
        }
    }
    vertexNum = maxVertexNum;
    meshGroupInfo[0] = objectNo[0];
    dataP = HuMemDirectMallocNum(HEAP_HEAP,
        (125 + 125 + (125 * 8)) * sizeof(s16), HU_MEMNUM_OVL);
    objP->data = dataP;
    groupP = objP->data;
    countP = groupP + 125;
    vertexNoP = countP + 125;
    memset(countP, 0, 125 * sizeof(s16));
    meshP = &hsfP->object[meshGroupInfo[0]];
    VECScale(&meshP->mesh.mesh.min, &min, -1.0f);
    VECSubtract(&meshP->mesh.mesh.max,
        &meshP->mesh.mesh.min, &size);
    size.x += 1.0f;
    size.y += 1.0f;
    size.z += 1.0f;
    if (size.x > 0.0f) {
        size.x = 5.0f / size.x;
    }
    if (size.y > 0.0f) {
        size.y = 5.0f / size.y;
    }
    if (size.z > 0.0f) {
        size.z = 5.0f / size.z;
    }
    vertexNum = meshP->mesh.vertex->count;
    vertexP = meshP->mesh.vertex->data;
    // Divide the mesh into 125 cells, retaining a random sample of at most eight vertices per cell.
    for (i = 0; i < vertexNum; i++, vertexP++) {
        VECAdd(vertexP, &min, &pos);
        HuVecMul(&pos, &size, &pos);
        x = pos.x;
        if (x < 0) {
            x += 5;
        }
        if (x >= 5) {
            x -= 5;
        }
        y = pos.y;
        if (y < 0) {
            y += 5;
        }
        if (y >= 5) {
            y -= 5;
        }
        z = pos.z;
        if (z < 0) {
            z += 5;
        }
        if (z >= 5) {
            z -= 5;
        }
        groupNo = x + (5 * y) + (25 * z);
        count = countP[groupNo];
        if (count < 8) {
            vertexNoP[(groupNo * 8) + count] = i;
        } else {
            randomNo = mbRandMod(count + 1);
            if (randomNo < 8) {
                vertexNoP[(groupNo * 8) + randomNo] = i;
            }
        }
        countP[groupNo]++;
    }
    for (i = 0, count = 0; i < 125; i++) {
        if (countP[i] > 0) {
            groupP[count++] = i;
            if (countP[i] > 8) {
                countP[i] = 8;
            }
        }
    }
    meshGroupInfo[1] = count;
    meshHeight = meshP->mesh.mesh.max.y - meshP->mesh.mesh.min.y;
    return meshHeight;
}

// Creates the metal particles and groups player mesh vertices for their spawn points.
static void MetalEffectCreate(OMOBJ *objP)
{
    PLAYERMETALWORK *workP = omObjGetWork(objP, PLAYERMETALWORK);
    HSF_DATA *hsfP;
    HuVecF size;
    HuVecF min;
    HuVecF pos;
    int objectNo[2];
    int groupNum;
    int maxVertexNum;
    HSF_OBJECT *searchObjectP;
    HuVecF *vertexP;
    int particleNum = 5;
    int vertexNum;
    HSF_OBJECT *meshObjectP;
    s16 *countP;
    int i;
    int count;
    int x;
    int y;
    int z;

    objP->mdlId[0] = mbParticleCreate(HuSprAnimRead(HuDataReadNum(
        mbBoardDataNumGet(DATANUM(DATA_board, 106)), HU_MEMNUM_OVL)),
        (s16)particleNum);
    mbParticleHookSet(objP->mdlId[0], MetalEffectHook);
    Hu3DModelLayerSet(objP->mdlId[0], 5);
    {
        MBPARTICLE *particleP;
        int randomNo;
        int groupNo;
        s16 *groupP;
        s16 *vertexNoP;
        HU3D_MODELID modelId;
        void *hookData;
        MBPARTICLE *modelParticleP;
        void *dataP;

    // Both assignments initialize the first mesh-index slot; the second entry is unused.
    objectNo[0] = objectNo[0] = -1;
    maxVertexNum = groupNum = 0;
    hsfP = Hu3DData[mbPlayerModelIDGet(workP->playerNo)].hsf;
    searchObjectP = hsfP->object;
    for (i = 0; i < hsfP->objectNum; i++, searchObjectP++) {
        meshObjectP = searchObjectP;
        if (meshObjectP->type == HSF_OBJ_MESH
            && meshObjectP->mesh.vertex->count > 0) {
            vertexNum = meshObjectP->mesh.vertex->count;
            if (maxVertexNum < vertexNum) {
                maxVertexNum = vertexNum;
                objectNo[0] = i;
            }
        }
    }
    vertexNum = maxVertexNum;
    workP->meshObjectIndex = objectNo[0];
    {
        modelId = objP->mdlId[0];
        hookData = Hu3DData[modelId].hookData;
        modelParticleP = hookData;
        particleP = modelParticleP;
        particleP->hookData = objP;
    }
    for (i = 0; i < particleNum; i++) {
        particleP->data[i].vertexNo = 0;
    }
    dataP = HuMemDirectMallocNum(HEAP_HEAP,
        (125 + 125 + (125 * 8)) * sizeof(s16), HU_MEMNUM_OVL);
    objP->data = dataP;
    groupP = objP->data;
    countP = groupP + 125;
    vertexNoP = countP + 125;
    memset(countP, 0, 125 * sizeof(s16));
    meshObjectP = &hsfP->object[workP->meshObjectIndex];
    VECScale(&meshObjectP->mesh.mesh.min, &min, -1.0f);
    VECSubtract(&meshObjectP->mesh.mesh.max,
        &meshObjectP->mesh.mesh.min, &size);
    size.x += 1.0f;
    size.y += 1.0f;
    size.z += 1.0f;
    if (size.x > 0.0f) {
        size.x = 5.0f / size.x;
    }
    if (size.y > 0.0f) {
        size.y = 5.0f / size.y;
    }
    if (size.z > 0.0f) {
        size.z = 5.0f / size.z;
    }
    vertexNum = meshObjectP->mesh.vertex->count;
    vertexP = meshObjectP->mesh.vertex->data;
    // Divide the mesh into 125 cells, retaining a random sample of at most eight vertices per cell.
    for (i = 0; i < vertexNum; i++, vertexP++) {
        VECAdd(vertexP, &min, &pos);
        HuVecMul(&pos, &size, &pos);
        x = pos.x;
        if (x < 0) {
            x += 5;
        }
        if (x >= 5) {
            x -= 5;
        }
        y = pos.y;
        if (y < 0) {
            y += 5;
        }
        if (y >= 5) {
            y -= 5;
        }
        z = pos.z;
        if (z < 0) {
            z += 5;
        }
        if (z >= 5) {
            z -= 5;
        }
        groupNo = x + (5 * y) + (25 * z);
        count = countP[groupNo];
        if (count < 8) {
            vertexNoP[(groupNo * 8) + count] = i;
        } else {
            randomNo = mbRandMod(count + 1);
            if (randomNo < 8) {
                vertexNoP[(groupNo * 8) + randomNo] = i;
            }
        }
        countP[groupNo]++;
    }
    for (i = 0, count = 0; i < 125; i++) {
        if (countP[i] > 0) {
            groupP[count++] = i;
            if (countP[i] > 8) {
                countP[i] = 8;
            }
        }
    }
    workP->vertexGroupCount = count;
    }
}

// Particle callback that emits and animates sparks from the metal player mesh.
static void MetalEffectHook(
    HU3D_MODEL *modelP, MBPARTICLE *particleP, Mtx matrix)
{
    HSF_OBJECT *objectP = NULL;
    OMOBJ *objP = particleP->hookData;
    int i;
    PLAYERMETALWORK *workP = omObjGetWork(objP, PLAYERMETALWORK);
    int objectNo = -1;
    HSF_DATA *hsfP;
    MBPARTICLEDATA *dataP;
    Mtx modelMtx;
    HuVecF dir;
    HuVecF pos;
    HuVecF cameraPos;
    s16 *groupP;
    s16 *countP;
    s16 *vertexNoP;
    int groupNo;
    int randomNo;
    BOOL posF;
    BOOL firstF = FALSE;
    float weight;

    if (!particleP->initF) {
        dataP = particleP->data;
        for (i = 0; i < particleP->num; i++, dataP++) {
            dataP->scale = 0.0f;
            dataP->color.a = 0;
            dataP->time = 0;
        }
        particleP->initF = TRUE;
        particleP->colorIn[0] = GX_CC_RASC;
        particleP->colorIn[1] = GX_CC_C0;
        particleP->colorIn[2] = GX_CC_TEXC;
        particleP->colorIn[3] = GX_CC_ZERO;
        particleP->tevColor[0].r = particleP->tevColor[0].g =
            particleP->tevColor[0].b = particleP->tevColor[0].a = 255;
        particleP->blendMode = MB_PARTICLE_BLEND_ADDCOL;
        firstF = TRUE;
    }
    MTXInverse(Hu3DCameraMtx, modelMtx);
    cameraPos.x = modelMtx[0][3];
    cameraPos.y = modelMtx[1][3];
    cameraPos.z = modelMtx[2][3];
    hsfP = Hu3DData[mbPlayerModelIDGet(workP->playerNo)].hsf;
    objectP = &hsfP->object[workP->meshObjectIndex];
    Hu3DModelObjMtxGet(
        mbPlayerModelIDGet(workP->playerNo), objectP->name, modelMtx);
    dataP = particleP->data;
    for (i = 0; i < particleP->num; i++, dataP++) {
        if (dataP->time == 0) {
            if (workP->fadeOutF) {
                continue;
            }
            dataP->time++;
            groupP = objP->data;
            countP = groupP + 125;
            vertexNoP = countP + 125;
            groupNo = groupP[mbRandMod(workP->vertexGroupCount)];
            randomNo = mbRandMod(countP[groupNo]);
            groupNo = vertexNoP[(groupNo * 8) + randomNo];
            dataP->vertexNo = groupNo;
            dataP->color.r = mbRandMod(120) + 120;
            dataP->color.g = mbRandMod(120) + 120;
            dataP->color.b = mbRandMod(120) + 120;
            dataP->color.a = mbRandMod(48) + 100;
            dataP->vel.x = 30.0f + (60.0f * frandf());
            dataP->scale = 0.0f;
            dataP->vel.y = (6.0f * frandf()) - 3.0f;
            dataP->rot.z = 360.0f * frandf();
            dataP->activeF = mbRandMod(30) + 30;
            dataP->animBank = mbRandMod(2);
            dataP->dispF = TRUE;
            if (firstF) {
                dataP->vel.x = 60.0f + (80.0f * frandf());
                dataP->activeF = mbRandMod(40) + 40;
            }
        }
        posF = TRUE;
        dataP->time++;
        weight = (float)dataP->time / dataP->activeF;
        dataP->scale = dataP->vel.x * mbSinDeg(180.0f * weight);
        dataP->rot.z += dataP->vel.y;
        if (weight > 0.8f) {
            dataP->color.a *= 0.9f;
        }
        if (weight > 0.6f) {
            posF = FALSE;
        }
        if (dataP->time >= dataP->activeF) {
            dataP->time = 0;
            dataP->color.a = 0;
            dataP->scale = 0.0f;
            dataP->dispF = FALSE;
        }
        if (posF) {
            MTXMultVec(modelMtx,
                &((HuVecF *)objectP->mesh.vertex->data)[dataP->vertexNo],
                &pos);
            VECSubtract(&cameraPos, &pos, &dir);
            weight = VECMag(&dir);
            if (weight > 0.0f) {
                VECScale(&dir, &dir, 200.0f / weight);
            }
            VECAdd(&pos, &dir, &dataP->pos);
        }
    }
}

// Per-frame callback that flashes the electric effect and removes it when cleared.
// Installed as the callback for the player's electric-effect object.
static void PlayerBiriQOMExec(OMOBJ *objP)
{
    PLAYERBIRIQWORK *workP = omObjGetWork(objP, PLAYERBIRIQWORK);
    BOOL killF = FALSE;
    BOOL dispF = TRUE;

    if (mbExitCheck() || workP->killF) {
        if (workP->killF) {
            killF = TRUE;
        }
        workP->killF = TRUE;
    }
    if (!workP->killF) {
        if (!GwPlayer[workP->playerNo].biriQF && !workP->statusEndedF) {
            workP->statusEndedF = TRUE;
            workP->time = 0;
            workP->maxTime = 20;
        }
        if (workP->statusEndedF) {
            workP->killF = TRUE;
        } else if (workP->flashF) {
            workP->time = 0;
            workP->maxTime = 24;
            workP->flashF = FALSE;
            workP->initialFlashDoneF = FALSE;
        } else if (!workP->initialFlashDoneF) {
            GXColor color = { 255, 255, 255, 255 };
            int colorNoTbl[4] = { 1, 3, 2, 3 };
            float level = 1.0f;
            int colorNo = colorNoTbl[(workP->time >> 1) & 3];

            if (colorNo == 0) {
                level = 0.0f;
            }
            mbObjBiriQColorSet(
                mbPlayerObjIDGet(workP->playerNo), colorNo, level, color);
            if (workP->time >= workP->maxTime) {
                workP->initialFlashDoneF = TRUE;
                mbObjBiriQColorSet(mbPlayerObjIDGet(workP->playerNo),
                    FALSE, 0.0f, color);
                if (objP->mdlId[0] < 0) {
                    BiriQEffectCreate(objP);
                }
            }
            workP->time++;
            dispF = FALSE;
        } else {
            GXColor color = { 255, 255, 255, 255 };
            int colorNoTbl[4] = { 1, 3, 2, 3 };
            float level = 0.1f;
            int colorNo = colorNoTbl[(workP->time >> 1) & 3];

            if (colorNo == 0) {
                level = 0.0f;
            }
            mbObjBiriQColorSet(
                mbPlayerObjIDGet(workP->playerNo), colorNo, level, color);
            workP->time++;
        }
        if (objP->mdlId[0] >= 0) {
            if (!mbObjGet(mbPlayerObjIDGet(workP->playerNo))->dispF
                || !workP->effectF || !dispF) {
                Hu3DModelAttrSet(objP->mdlId[0], HU3D_ATTR_DISPOFF);
                Hu3DModelAttrSet(objP->mdlId[1], HU3D_ATTR_DISPOFF);
            } else {
                Hu3DModelAttrReset(objP->mdlId[0], HU3D_ATTR_DISPOFF);
                Hu3DModelAttrReset(objP->mdlId[1], HU3D_ATTR_DISPOFF);
            }
        }
    }
    if (workP->killF) {
        if (!killF) {
            PlayerBiriQKill(workP->playerNo);
        }
        if (objP->data) {
            void *dataP = objP->data;

            HuMemDirectFree(dataP);
        }
        omDelObjEx(HuPrcCurrentGet(), objP);
        return;
    }
}

// Starts or requests removal of the player's electric status effect.
void mbPlayerBiriQSet(int playerNo, BOOL biriQF)
{
    OMOBJ *objP;
    PLAYERBIRIQWORK *workP;

    if (biriQF) {
        if (playerWork[playerNo].biriQObj != NULL) {
            OSReport("------------already BiriQ!!----------");
        }
        GwPlayer[playerNo].biriQF = TRUE;
        objP = playerWork[playerNo].biriQObj;
        if (objP == NULL) {
            GXColor color = { 255, 255, 255, 255 };

            objP = playerWork[playerNo].biriQObj = omAddObjEx(mbObjMan,
                PLAYER_OBJ_PRIORITY, 2, 0, -1, PlayerBiriQOMExec);
            omSetStatBit(objP, OM_STAT_MODELPAUSE);
            objP->mdlId[0] = objP->mdlId[1] = MB_MODEL_NONE;
            mbObjBiriQCreate(mbPlayerObjIDGet(playerNo));
            mbObjBiriQColorSet(
                mbPlayerObjIDGet(playerNo), TRUE, 0.0f, color);
        }
        workP = omObjGetWork(objP, PLAYERBIRIQWORK);
        workP->playerNo = playerNo;
        workP->statusEndedF = FALSE;
        workP->initialFlashDoneF = FALSE;
        workP->time = 0;
        workP->maxTime = 20;
        workP->effectF = TRUE;
        PlayerBiriQFlashSet(playerNo);
    } else {
        GwPlayer[playerNo].biriQF = FALSE;
    }
}

// Requests a new electric-status flash in the per-frame object callback.
static void PlayerBiriQFlashSet(int playerNo)
{
    OMOBJ *objP = playerWork[playerNo].biriQObj;

    if (objP != NULL) {
        PLAYERBIRIQWORK *workP = omObjGetWork(objP, PLAYERBIRIQWORK);

        workP->flashF = TRUE;
    }
}

// Releases the electric material hook and particle allocations, and marks the effect object
// for later deletion.
static void PlayerBiriQKill(int playerNo)
{
    OMOBJ *objP = playerWork[playerNo].biriQObj;

    if (objP != NULL) {
        PLAYERBIRIQWORK *workP = omObjGetWork(objP, PLAYERBIRIQWORK);

        workP->killF = TRUE;
        playerWork[playerNo].biriQObj = NULL;
        mbObjBiriQKill(mbPlayerObjIDGet(playerNo));
        if (objP->mdlId[0] >= 0) {
            mbParticleKill(objP->mdlId[0]);
            objP->mdlId[0] = -1;
        }
        if (objP->mdlId[1] >= 0) {
            mbParticleKill(objP->mdlId[1]);
            objP->mdlId[1] = -1;
        }
    }
}

// Enables or hides the player's electric particles without clearing the status.
static void PlayerBiriQEffectSet(int playerNo, BOOL effectF)
{
    OMOBJ *objP = playerWork[playerNo].biriQObj;

    if (objP) {
        PLAYERBIRIQWORK *workP = omObjGetWork(objP, PLAYERBIRIQWORK);

        workP->effectF = effectF;
    }
}

// Creates the two electric particle layers around the player's mesh.
static void BiriQEffectCreate(OMOBJ *objP)
{
    PLAYERBIRIQWORK *workP = omObjGetWork(objP, PLAYERBIRIQWORK);
    MBPARTICLE *particleP;
    HU3D_MODELID modelId;
    HU3D_MODELID modelId2;
    HU3D_MODELID sourceModelId;
    // Mesh object index in slot 0 and occupied vertex-group count in slot 1.
    int meshGroupInfo[2];
    float meshHeight;
    int particleNum;

    meshHeight = GetBiriQEffectRadius(objP, workP->playerNo, meshGroupInfo);
    workP->meshObjectIndex = meshGroupInfo[0];
    workP->vertexGroupCount = meshGroupInfo[1];
    particleNum = 21.0f * (0.006666667f * meshHeight);
    objP->mdlId[0] = mbParticleCreate(HuSprAnimRead(HuDataReadNum(
        mbBoardDataNumGet(DATANUM(DATA_board, 107)), HU_MEMNUM_OVL)),
        (s16)particleNum);
    objP->mdlId[1] = mbParticleCreate(HuSprAnimRead(HuDataReadNum(
        mbBoardDataNumGet(DATANUM(DATA_board, 108)), HU_MEMNUM_OVL)),
        (s16)particleNum);
    mbParticleHookSet(objP->mdlId[0], BiriQEffect1Hook);
    mbParticleHookSet(objP->mdlId[1], BiriQEffect2Hook);
    Hu3DModelLayerSet(objP->mdlId[0], 3);
    Hu3DModelLayerSet(objP->mdlId[1], 3);
    {
        void *hookData;
        MBPARTICLE *modelParticleP;

        modelId = objP->mdlId[0];
        hookData = Hu3DData[modelId].hookData;
        modelParticleP = hookData;
        particleP = modelParticleP;
        particleP->hookData = objP;
        particleP->mode = 0;
    }
    {
        void *hookData2;
        MBPARTICLE *modelParticleP2;

        modelId2 = objP->mdlId[1];
        hookData2 = Hu3DData[modelId2].hookData;
        modelParticleP2 = hookData2;
        particleP = modelParticleP2;
        {
            void *sourceHookData;
            MBPARTICLE *sourceParticleP;

            sourceModelId = objP->mdlId[0];
            sourceHookData = Hu3DData[sourceModelId].hookData;
            sourceParticleP = sourceHookData;
            particleP->hookData = sourceParticleP;
        }
        particleP->mode = 0;
    }
}

// Particle callback that spawns and animates electric sparks on the player mesh.
static void BiriQEffect1Hook(
    HU3D_MODEL *modelP, MBPARTICLE *particleP, Mtx matrix)
{
    HSF_OBJECT *objectP = NULL;
    OMOBJ *objP = particleP->hookData;
    int i;
    PLAYERBIRIQWORK *workP = omObjGetWork(objP, PLAYERBIRIQWORK);
    int objectNo = -1;
    HSF_DATA *hsfP;
    MBPARTICLEDATA *dataP;
    Mtx modelMtx;
    HuVecF dir;
    HuVecF pos;
    HuVecF cameraPos;
    s16 *groupP;
    s16 *countP;
    s16 *vertexNoP;
    int groupNo;
    int randomNo;
    BOOL firstF = FALSE;
    BOOL posF;
    float weight;

    if (particleP->mode == 0) {
        dataP = particleP->data;
        for (i = 0; i < particleP->num; i++, dataP++) {
            dataP->scale = 0.0f;
            dataP->color.a = 0;
            dataP->time = 0;
        }
        particleP->mode = 1;
        firstF = TRUE;
    }
    MTXInverse(Hu3DCameraMtx, modelMtx);
    cameraPos.x = modelMtx[0][3];
    cameraPos.y = modelMtx[1][3];
    cameraPos.z = modelMtx[2][3];
    hsfP = Hu3DData[mbPlayerModelIDGet(workP->playerNo)].hsf;
    objectP = &hsfP->object[workP->meshObjectIndex];
    Hu3DModelObjMtxGet(
        mbPlayerModelIDGet(workP->playerNo), objectP->name, modelMtx);
    dataP = particleP->data;
    for (i = 0; i < particleP->num; i++, dataP++) {
        if (dataP->time == 0) {
            if (workP->statusEndedF) {
                continue;
            }
            dataP->time++;
            groupP = objP->data;
            countP = groupP + 125;
            vertexNoP = countP + 125;
            groupNo = groupP[mbRandMod(workP->vertexGroupCount)];
            randomNo = mbRandMod(countP[groupNo]);
            groupNo = vertexNoP[(groupNo * 8) + randomNo];
            dataP->vertexNo = groupNo;
            dataP->color.r = mbRandMod(40) + 30;
            dataP->color.g = mbRandMod(40) + 90;
            dataP->color.b = mbRandMod(40) + 180;
            dataP->color.a = mbRandMod(58) + 130;
            dataP->vel.x = 30.0f + (60.0f * frandf());
            dataP->scale = 0.0f;
            dataP->rot.z = 360.0f * frandf();
            dataP->activeF = mbRandMod(12) + 12;
            dataP->animBank = mbRandMod(4);
            dataP->dispF = TRUE;
        }
        posF = TRUE;
        dataP->time++;
        weight = (float)dataP->time / dataP->activeF;
        dataP->scale = dataP->vel.x
            * (0.1f + (0.9f * mbSinDeg(180.0f * weight)));
        if (dataP->time & 1) {
            dataP->animBank = mbRandMod(4);
            if (mbRandMod(100) < 25) {
                dataP->rot.z += 180.0f;
            }
        }
        if (weight > 0.8f) {
            dataP->color.a *= 0.9f;
        }
        if (weight > 0.6f) {
            posF = FALSE;
        }
        if (dataP->time >= dataP->activeF) {
            dataP->time = 0;
            dataP->color.a = 0;
            dataP->scale = 0.0f;
            dataP->dispF = FALSE;
        }
        if (posF) {
            MTXMultVec(modelMtx,
                &((HuVecF *)objectP->mesh.vertex->data)[dataP->vertexNo],
                &pos);
            VECSubtract(&cameraPos, &pos, &dir);
            weight = VECMag(&dir);
            if (weight > 0.0f) {
                VECScale(&dir, &dir, 18.0f / weight);
            }
            VECAdd(&pos, &dir, &dataP->pos);
        }
    }
}

// Particle callback that copies the first electric layer and brightens it white.
static void BiriQEffect2Hook(
    HU3D_MODEL *modelP, MBPARTICLE *particleP, Mtx matrix)
{
    MBPARTICLE *sourceP;
    MBPARTICLEDATA *dataP;
    int i;
    int alpha;

    if (particleP->mode == 0) {
        dataP = particleP->data;
        for (i = 0; i < particleP->num; i++, dataP++) {
            dataP->scale = 0.0f;
            dataP->color.a = 0;
            dataP->time = 0;
        }
        particleP->mode = 1;
        particleP->blendMode = MB_PARTICLE_BLEND_ADDCOL;
    }
    sourceP = particleP->hookData;
    memcpy(particleP->data, sourceP->data,
        particleP->num * sizeof(MBPARTICLEDATA));
    dataP = particleP->data;
    for (i = 0; i < particleP->num; i++, dataP++) {
        dataP->color.r = 255;
        dataP->color.g = 255;
        dataP->color.b = 255;
        alpha = 1.2f * dataP->color.a;
        if (alpha > 255) {
            alpha = 255;
        }
        dataP->color.a = alpha;
    }
}

// Opening's player-order setup calls this while sorting roll results to exchange player slots.
// Configuration, board data, and runtime state move together; collision objects stay in place.
void mbPlayerSwap(int playerNo1, int playerNo2)
{
    GW_PLAYER playerData;
    MBPLAYERWORK playerWorkCopy;
    GW_PLAYER_CONF playerConfig;
    OMOBJ *colObj1;
    OMOBJ *colObj2;

    colObj1 = mbPlayerWorkGet(playerNo1)->colObj;
    colObj2 = mbPlayerWorkGet(playerNo2)->colObj;
    playerConfig = GwPlayerConf[playerNo1];
    GwPlayerConf[playerNo1] = GwPlayerConf[playerNo2];
    GwPlayerConf[playerNo2] = playerConfig;
    playerData = GwPlayer[playerNo1];
    GwPlayer[playerNo1] = GwPlayer[playerNo2];
    GwPlayer[playerNo2] = playerData;
    memcpy(&playerWorkCopy, mbPlayerWorkGet(playerNo1), sizeof(MBPLAYERWORK));
    memcpy(mbPlayerWorkGet(playerNo1), mbPlayerWorkGet(playerNo2),
        sizeof(MBPLAYERWORK));
    memcpy(mbPlayerWorkGet(playerNo2), &playerWorkCopy, sizeof(MBPLAYERWORK));
    mbPlayerWorkGet(playerNo1)->colObj = colObj1;
    mbPlayerWorkGet(playerNo2)->colObj = colObj2;
    GwPlayer[playerNo1].padNo = GwPlayerConf[playerNo1].padNo;
    GwPlayerConf[playerNo1].padNo = GwPlayerConf[playerNo1].padNo;
    GwPlayer[playerNo2].padNo = GwPlayerConf[playerNo2].padNo;
    GwPlayerConf[playerNo2].padNo = GwPlayerConf[playerNo2].padNo;
}

// Returns the message ID for the character name currently assigned to a player.
u32 mbPlayerNameMesGet(int playerNo)
{
    u32 nameTbl[CHARNO_MAX] = {
        MESS_CHARANAME_MARIO, MESS_CHARANAME_LUIGI,
        MESS_CHARANAME_PEACH, MESS_CHARANAME_YOSHI,
        MESS_CHARANAME_WARIO, MESS_CHARANAME_DAISY,
        MESS_CHARANAME_WALUIGI, MESS_CHARANAME_KINOPIO,
        MESS_CHARANAME_TERESA, MESS_CHARANAME_MINIKOOPA,
        MESS_CHARANAME_KINOPICO, MESS_CHARANAME_MINIKOOPAR,
        MESS_CHARANAME_MINIKOOPAG, MESS_CHARANAME_MINIKOOPAB
    };

    return nameTbl[GwPlayer[playerNo].charNo];
}

// Returns the display string for the player's character; the three Mini Koopa variants have no
// entry.
char *mbPlayerNameGet(int playerNo)
{
    char *nameTbl[CHARNO_MAX] = {
        "Mario", "Luigi", "Peach", "Yoshi", "Wario", "Daisy", "Waluigi", "Kinopio",
        "Teresa", "Mini Koopa", "Kinopiko"
    };

    return nameTbl[GwPlayer[playerNo].charNo];
}

// Maps sorted, distinct character pairs from IDs 0-10 to message offsets; -1 marks no entry.
// There is no row for a first ID of 10, and the lookup does not bound-check character IDs.
static s8 tagIdTbl[110] = {
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    -1, -1, 10, 11, 12, 13, 14, 15, 16, 17, 18,
    -1, -1, -1, 19, 20, 21, 22, 23, 24, 25, 26,
    -1, -1, -1, -1, 27, 28, 29, 30, 31, 32, 33,
    -1, -1, -1, -1, -1, 34, 35, 36, 37, 38, 39,
    -1, -1, -1, -1, -1, -1, 40, 41, 42, 43, 44,
    -1, -1, -1, -1, -1, -1, -1, 45, 46, 47, 48,
    -1, -1, -1, -1, -1, -1, -1, -1, 49, 50, 51,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, 52, 53,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 54
};

char lbl_8024767A[14] = "%d:%d->%d\n";

// Returns the tag-team name message for the two characters on a team.
u32 mbPlayerTagNameMesGet(int teamNo)
{
    int firstCharNo;
    int secondCharNo;
    int charNoSwap;
    int tagId;

    firstCharNo = GwPlayer[mbPlayerTeamFindPlayer(teamNo, 0)].charNo;
    secondCharNo = GwPlayer[mbPlayerTeamFindPlayer(teamNo, 1)].charNo;
    if (firstCharNo > secondCharNo) {
        charNoSwap = firstCharNo;
        firstCharNo = secondCharNo;
        secondCharNo = charNoSwap;
    }
    tagId = tagIdTbl[(firstCharNo * 11) + secondCharNo];
    OSReport(lbl_8024767A, firstCharNo, secondCharNo, tagId);
    if (tagId == -1) {
        return MESSNUM(MESS_TAG_NAME, 55);
    }
    return MESSNUM(MESS_TAG_NAME, 0) + tagId;
}

// Sets the ambient light color applied to the player's board model.
void mbPlayerAmbSet(int playerNo, float ambR, float ambG, float ambB)
{
    mbObjAmbSet(mbPlayerObjIDGet(playerNo), ambR, ambG, ambB);
}

// Returns the board-model object ID for one player.
MBMODELID mbPlayerObjIDGet(int playerNo)
{
    return playerWork[playerNo].objId;
}

// Returns the rendered model ID for one player's board model.
HU3D_MODELID mbPlayerModelIDGet(int playerNo)
{
    return mbObjModelIDGet(playerWork[playerNo].objId);
}

// Returns whether every player is controlled by the computer.
BOOL mbPlayerAllComCheck(void)
{
    int playerIndex;

    for (playerIndex = 0; playerIndex < GW_PLAYER_MAX; playerIndex++) {
        if (!GwPlayer[playerIndex].comF) {
            return FALSE;
        }
    }
    return TRUE;
}

// Finds the other player assigned to the same team as playerNo.
int mbPlayerTeamFind(int playerNo)
{
    int teammateNo;

    for (teammateNo = 0; teammateNo < GW_PLAYER_MAX; teammateNo++) {
        if (teammateNo == playerNo) {
            continue;
        }
        if (mbPlayerGrpGet(playerNo) == mbPlayerGrpGet(teammateNo)) {
            break;
        }
    }
    return teammateNo;
}

// Finds a player assigned to a different team from playerNo.
int mbPlayerTeamFindOther(int playerNo)
{
    int otherPlayerNo;

    for (otherPlayerNo = 0; otherPlayerNo < GW_PLAYER_MAX; otherPlayerNo++) {
        if (otherPlayerNo == playerNo) {
            continue;
        }
        if (mbPlayerGrpGet(playerNo) != mbPlayerGrpGet(otherPlayerNo)) {
            break;
        }
    }
    return otherPlayerNo;
}

// Finds the memberNo-th player assigned to teamNo.
int mbPlayerTeamFindPlayer(int teamNo, int memberNo)
{
    int playerIndex;
    int teamMemberIndex;

    teamMemberIndex = 0;
    for (playerIndex = 0; playerIndex < GW_PLAYER_MAX; playerIndex++) {
        if (teamNo != mbPlayerGrpGet(playerIndex)) {
            continue;
        }
        if (teamMemberIndex == memberNo) {
            return playerIndex;
        }
        teamMemberIndex++;
    }
    return -1;
}

// Board events use this to choose the other teammate, or the same player outside team mode.
int mbPlayerTeamFindOpp(int playerNo)
{
    if (!GWTeamFGet()) {
        return playerNo;
    }
    return mbPlayerTeamFind(playerNo);
}

// Turn and event logic uses this to compare team IDs when team mode is active.
// Outside team mode, only two references to the same player compare equal.
BOOL mbPlayerTeamCheckSame(int playerNo1, int playerNo2)
{
    BOOL sameF;

    sameF = FALSE;
    if (GWTeamFGet()) {
        if (mbPlayerGrpGet(playerNo1) == mbPlayerGrpGet(playerNo2)) {
            sameF = TRUE;
        }
    } else if (playerNo1 == playerNo2) {
        sameF = TRUE;
    }
    return sameF;
}

// Board ranking and events get a player's team ID, or their own index outside team mode.
int mbPlayerTeamGet(int playerNo)
{
    if (!GWTeamFGet()) {
        return playerNo;
    }
    return mbPlayerGrpGet(playerNo);
}

float mbPlayerWalkSpeedGet(void)
{
    return 20;
}

void mbPlayerLayerSet(int playerNo, int layer)
{
    mbObjLayerSet(mbPlayerObjIDGet(playerNo), layer);
}

void mbPlayerCameraSet(int playerNo, u16 cameraBit)
{
    mbObjCameraSet(mbPlayerObjIDGet(playerNo), cameraBit);
}

void mbPlayerCullRadiusSet(int playerNo, float radius)
{
    mbObjCullRadiusSet(mbPlayerObjIDGet(playerNo), radius);
}

void mbPlayerStubValSet(int playerNo, BOOL unusedFlag)
{
}

// Board setup and player re-entry call this to place one player at their current space.
void mbPlayerPosReset(int playerNo)
{
    HuVecF pos;

    mbMasuPosGet(GwPlayer[playerNo].masuId, &pos);
    mbPlayerPosSetV(playerNo, &pos);
    PlayerColCornerSnap(playerNo, GwPlayer[playerNo].masuId, 0);
}

// Board events call this after moving or revealing players to arrange everyone at their current
// space.
void mbPlayerPosResetAll(void)
{
    int playerIndex;
    int otherPlayerIndex;
    int cornerNo;
    s16 masuId;
    s8 orderNo;
    HuVecF pos;

    for (playerIndex = 0; playerIndex < GW_PLAYER_MAX; playerIndex++) {
        orderNo = GwPlayer[playerIndex].orderNo;
        masuId = GwPlayer[playerIndex].masuId;
        cornerNo = 0;
        for (otherPlayerIndex = 0; otherPlayerIndex < GW_PLAYER_MAX;
             otherPlayerIndex++) {
            if (playerIndex != otherPlayerIndex
                && masuId == GwPlayer[otherPlayerIndex].masuId
                && orderNo > GwPlayer[otherPlayerIndex].orderNo) {
                cornerNo++;
            }
        }
        mbPlayerMasuCornerSet(playerIndex, cornerNo);
        if (cornerNo == 0) {
            mbMasuPosGet(masuId, &pos);
        } else {
            mbMasuCornerRotPosGet(masuId, cornerNo - 1, &pos);
        }
        mbPlayerPosSetV(playerIndex, &pos);
        PlayerColCornerSnap(playerIndex, masuId, cornerNo);
    }
}

void mbPlayerMtxSet(int playerNo, Mtx *matrix)
{
    mbObjMtxSet(mbPlayerObjIDGet(playerNo), matrix);
}

void mbPlayerMtxGet(int playerNo, Mtx *matrix)
{
    mbObjMtxGet(mbPlayerObjIDGet(playerNo), matrix);
}

void mbPlayerPosSetV(int playerNo, const HuVecF *pos)
{
    mbObjPosSetV(mbPlayerObjIDGet(playerNo), pos);
}

void mbPlayerPosSet(int playerNo, float posX, float posY, float posZ)
{
    mbObjPosSet(mbPlayerObjIDGet(playerNo), posX, posY, posZ);
}

void mbPlayerPosGet(int playerNo, HuVecF *pos)
{
    mbObjPosGet(mbPlayerObjIDGet(playerNo), pos);
}

void mbPlayerRotSetV(int playerNo, const HuVecF *rot)
{
    mbObjRotSetV(mbPlayerObjIDGet(playerNo), rot);
}

void mbPlayerRotSet(int playerNo, float rotX, float rotY, float rotZ)
{
    mbObjRotSet(mbPlayerObjIDGet(playerNo), rotX, rotY, rotZ);
}

void mbPlayerRotGet(int playerNo, HuVecF *rot)
{
    mbObjRotGet(mbPlayerObjIDGet(playerNo), rot);
}

// Board movement and setup code call this to set a player's yaw in the [0, 360) degree range.
void mbPlayerRotYSet(int playerNo, float rotY)
{
    rotY = fmod(rotY, 360);
    if (rotY < 0) {
        rotY += 360;
    }
    mbObjRotYSet(mbPlayerObjIDGet(playerNo), rotY);
}

float mbPlayerRotYGet(int playerNo)
{
    return mbObjRotYGet(mbPlayerObjIDGet(playerNo));
}

void mbPlayerScaleSetV(int playerNo, const HuVecF *scale)
{
    mbObjScaleSetV(mbPlayerObjIDGet(playerNo), scale);
}

void mbPlayerScaleSet(int playerNo, float scaleX, float scaleY, float scaleZ)
{
    mbObjScaleSet(mbPlayerObjIDGet(playerNo), scaleX, scaleY, scaleZ);
}

void mbPlayerScaleGet(int playerNo, HuVecF *scale)
{
    mbObjScaleGet(mbPlayerObjIDGet(playerNo), scale);
}

// Changes the player's animation; a current-motion request also ignores attr and offset updates.
// Motion 10 sets a 4-unit vertical offset; other newly selected motions clear it.
void mbPlayerMotionSet(int playerNo, int motNo, u32 attr)
{
    GW_PLAYER *playerP;

    playerP = GWPlayerGet(playerNo);
    if (motNo == playerWork[playerNo].motNo) {
        return;
    }
    mbObjMotionSet(mbPlayerObjIDGet(playerNo), motNo, attr);
    playerWork[playerNo].motNo = motNo;
    if (motNo == 10) {
        mbObjOffsetSet(mbPlayerObjIDGet(playerNo), 0, 4, 0);
    } else {
        mbObjOffsetSet(mbPlayerObjIDGet(playerNo), 0, 0, 0);
    }
}

int mbPlayerMotionGet(int playerNo)
{
    return playerWork[playerNo].motNo;
}

// Board events and minigame code start animation transitions here.
// A request for the current motion is ignored; motion 10 sets a 4-unit offset and others clear it.
void mbPlayerMotionShiftSet(int playerNo, int motNo, float start, float end,
    u32 attr)
{
    GW_PLAYER *playerP;

    playerP = GWPlayerGet(playerNo);
    if (motNo == playerWork[playerNo].motNo) {
        return;
    }
    mbObjMotionShiftSet(mbPlayerObjIDGet(playerNo), motNo, start, end, attr);
    playerWork[playerNo].motNo = motNo;
    if (motNo == 10) {
        mbObjOffsetSet(mbPlayerObjIDGet(playerNo), 0, 4, 0);
    } else {
        mbObjOffsetSet(mbPlayerObjIDGet(playerNo), 0, 0, 0);
    }
}

int mbPlayerMotionCreate(int playerNo, int dataNum)
{
    return mbObjMotionCreate(mbPlayerObjIDGet(playerNo), dataNum);
}

// Releases the motion slot, then returns TRUE unconditionally.
int mbPlayerMotionKill(int playerNo, int motNo)
{
    mbObjMotionKill(mbPlayerObjIDGet(playerNo), motNo);
    return TRUE;
}

void mbPlayerMotionSpeedSet(int playerNo, float speed)
{
    mbObjMotionSpeedSet(mbPlayerObjIDGet(playerNo), speed);
}

void mbPlayerMotionTimeSet(int playerNo, float time)
{
    mbObjMotionTimeSet(mbPlayerObjIDGet(playerNo), time);
}

float mbPlayerMotionTimeGet(int playerNo)
{
    return mbObjMotionTimeGet(mbPlayerObjIDGet(playerNo));
}

float mbPlayerMotionMaxTimeGet(int playerNo)
{
    return mbObjMotionMaxTimeGet(mbPlayerObjIDGet(playerNo));
}

void mbPlayerMotionStartEndSet(int playerNo, float start, float end)
{
    mbObjMotionStartEndSet(mbPlayerObjIDGet(playerNo), start, end);
}

// Board effects call this to enable model attributes on a player's model.
void mbPlayerAttrSet(int playerNo, u32 attr)
{
    MBMODELID modelId = mbPlayerObjIDGet(playerNo);

    mbObjAttrSet(modelId, attr);
}

// Board effects call this to clear model attributes from a player's model.
void mbPlayerAttrReset(int playerNo, u32 attr)
{
    MBMODELID modelId = mbPlayerObjIDGet(playerNo);

    mbObjAttrReset(modelId, attr);
}

void mbPlayerMotionVoiceOnSet(int playerNo, int motNo, BOOL voiceOnF)
{
    mbObjMotionVoiceOnSet(mbPlayerObjIDGet(playerNo), motNo, voiceOnF);
}

// Event processes poll this after changing animation to wait for shifts and motions to finish.
BOOL mbPlayerMotionEndCheck(int playerNo)
{
    int modelId;
    BOOL endF;

    modelId = mbPlayerObjIDGet(playerNo);
    return mbObjMotionShiftIDGet(modelId) < 0 && mbObjMotionEndCheck(modelId);
}

// Board opening and event processes poll this until every player's motion has ended.
BOOL mbPlayerMotionEndCheckAll(void)
{
    int playerIndex;

    for (playerIndex = 0; playerIndex < GW_PLAYER_MAX; playerIndex++) {
        if (!mbPlayerMotionEndCheck(playerIndex)) {
            return FALSE;
        }
    }
    return TRUE;
}

// Event processes call this to sleep until one player's current motion ends.
void mbPlayerMotionEndWait(int playerNo)
{
    while (!mbPlayerMotionEndCheck(playerNo)) {
        HuPrcVSleep();
    }
}

void mbPlayerMotIdleSet(int playerNo)
{
    mbPlayerMotionShiftSet(playerNo, 1, 0, 8, HU3D_MOTATTR_LOOP);
}

// Board events call this to set a player's coins, using the first teammate's shared balance in team
// play.
void mbPlayerCoinSet(int playerNo, int coinNum)
{
    if (!GWTeamFGet()) {
        GWPlayerCoinSet(playerNo, coinNum);
    } else {
        GWPlayerCoinSet(
            mbPlayerTeamFindPlayer(mbPlayerGrpGet(playerNo), 0), coinNum);
    }
}

// Board events and menus call this to read a player's coins, including the shared team balance.
int mbPlayerCoinGet(int playerNo)
{
    if (!GWTeamFGet()) {
        return GWPlayerCoinGet(playerNo);
    } else {
        return GWPlayerCoinGet(
            mbPlayerTeamFindPlayer(mbPlayerGrpGet(playerNo), 0));
    }
}

// Requests a balance change and records positive amounts in this game's earned total, capped at
// 999.
// Team mode uses the first member's totals; practice mode skips only the balance update.
void mbPlayerCoinAdd(int playerNo, int coinNum)
{
    GW_PLAYER *playerP;

    if (GWTeamFGet()) {
        playerNo = mbPlayerTeamFindPlayer(mbPlayerGrpGet(playerNo), 0);
    }
    playerP = GWPlayerGet(playerNo);
    if (coinNum > 0 && playerP->coinTotal < 999) {
        playerP->coinTotal += coinNum;
        if (playerP->coinTotal > 999) {
            playerP->coinTotal = 999;
        }
    }
    mbPlayerCoinSet(playerNo, coinNum + mbPlayerCoinGet(playerNo));
}

void mbPlayerTeamCoinSet(int teamNo, int coinNum)
{
    GWPlayerCoinSet(mbPlayerTeamFindPlayer(teamNo, 0), coinNum);
}

s16 mbPlayerTeamCoinGet(int teamNo)
{
    return GWPlayerCoinGet(mbPlayerTeamFindPlayer(teamNo, 0));
}

// Board event selection uses this to find the largest current coin balance.
int mbPlayerMaxCoinGet(void)
{
    int maxCoin;
    int playerIndex;

    maxCoin = 0;
    for (playerIndex = 0; playerIndex < GW_PLAYER_MAX; playerIndex++) {
        if (mbPlayerCoinGet(playerIndex) >= maxCoin) {
            maxCoin = mbPlayerCoinGet(playerIndex);
        }
    }
    return maxCoin;
}

// Board events call this to set a player's stars, using the first teammate's shared count in team
// play.
void mbPlayerStarSet(int playerNo, int starNum)
{
    if (!GWTeamFGet()) {
        GWPlayerStarSet(playerNo, starNum);
    } else {
        GWPlayerStarSet(
            mbPlayerTeamFindPlayer(mbPlayerGrpGet(playerNo), 0), starNum);
    }
}

// Board events and menus call this to read a player's stars, including the shared team count.
int mbPlayerStarGet(int playerNo)
{
    if (!GWTeamFGet()) {
        return GWPlayerStarGet(playerNo);
    } else {
        return GWPlayerStarGet(
            mbPlayerTeamFindPlayer(mbPlayerGrpGet(playerNo), 0));
    }
}

// Board events call this when awarding or removing stars and play the star-count sound.
void mbPlayerStarAdd(int playerNo, int starNum)
{
    int star;

    mbAudFXPlay(MSM_SE_CMN_09);
    star = mbPlayerStarGet(playerNo) + starNum;
    if (star < 0) {
        star = 0;
    }
    mbPlayerStarSet(playerNo, star);
}

void mbPlayerGrpStarSet(int teamNo, int starNum)
{
    GWPlayerStarSet(mbPlayerTeamFindPlayer(teamNo, 0), starNum);
}

s16 mbPlayerGrpStarGet(int teamNo)
{
    return GWPlayerStarGet(mbPlayerTeamFindPlayer(teamNo, 0));
}

// Roulette CPU selection uses the player with the shortest route to a type-7 Star space.
// Returns a random player if every route search fails.
int mbPlayerBestPathGet(void)
{
    int playerIndex;
    int bestPlayerNo;
    int pathLength;
    int shortestPathLength;

    shortestPathLength = 9999;
    bestPlayerNo = -1;
    for (playerIndex = 0; playerIndex < GW_PLAYER_MAX; playerIndex++) {
        pathLength = mbMasuFind_TypeStepGet2(
            GwPlayer[playerIndex].masuId, 7, TRUE, TRUE);
        if (pathLength < shortestPathLength) {
            shortestPathLength = pathLength;
            bestPlayerNo = playerIndex;
        }
    }
    if (bestPlayerNo < 0) {
        return mbRandMod(GW_PLAYER_MAX);
    } else {
        return bestPlayerNo;
    }
}

// Board results and event logic use this to rank players by stars first, then coins.
int mbPlayerRankGet(int playerNo)
{
    int score[GW_PLAYER_MAX];
    int otherPlayerNo;
    int rank;

    for (otherPlayerNo = 0; otherPlayerNo < GW_PLAYER_MAX; otherPlayerNo++) {
        score[otherPlayerNo] = mbPlayerCoinGet(otherPlayerNo)
            | (mbPlayerStarGet(otherPlayerNo) * 1024);
    }
    rank = 0;
    for (otherPlayerNo = 0; otherPlayerNo < GW_PLAYER_MAX; otherPlayerNo++) {
        if (otherPlayerNo != playerNo
            && score[playerNo] < score[otherPlayerNo]) {
            rank++;
        }
    }
    return rank;
}

// Team results use this to rank teams by stars first, then their shared coin balance.
s16 mbPlayerTeamRankGet(int teamNo)
{
    int score[2];
    int otherTeamNo;
    int rank;

    for (otherTeamNo = 0; otherTeamNo < 2; otherTeamNo++) {
        score[otherTeamNo] = mbPlayerTeamCoinGet(otherTeamNo)
            | (mbPlayerGrpStarGet(otherTeamNo) * 2048);
    }
    rank = 0;
    for (otherTeamNo = 0; otherTeamNo < 2; otherTeamNo++) {
        if (otherTeamNo != teamNo
            && score[teamNo] < score[otherTeamNo]) {
            rank++;
        }
    }
    return rank;
}

void mbPlayerCapsuleUseSet(int capsuleNo)
{
    GwPlayer[GwSystem.turnPlayerNo].capsuleUse = capsuleNo;
}

int mbPlayerCapsuleUseGet(void)
{
    return GwPlayer[GwSystem.turnPlayerNo].capsuleUse;
}

int mbPlayerCapsuleMaxGet(void)
{
    return (GWTeamFGet() == FALSE) ? 3 : 5;
}

// Capsule inventory helpers use this to address a slot, splitting the six team slots across two
// players.
static inline s8 *PlayerCapsulePtrGet(int playerNo, int index)
{
    if (!GWTeamFGet()) {
        return &GwPlayer[playerNo].capsule[index];
    } else {
        int memberNo = (index < 3) ? 0 : 1;
        int teamNo = mbPlayerGrpGet(playerNo);
        int teamMemberIndex = -1;
        int scanPlayerNo;

        for (scanPlayerNo = 0; scanPlayerNo < GW_PLAYER_MAX; scanPlayerNo++) {
            if (teamNo == mbPlayerGrpGet(scanPlayerNo)) {
                teamMemberIndex++;
                if (teamMemberIndex == memberNo) {
                    break;
                }
            }
        }
        if (scanPlayerNo >= GW_PLAYER_MAX) {
            return NULL;
        }
        return &GwPlayer[scanPlayerNo].capsule[index - (memberNo * 3)];
    }
}

// Capsule shop and board events call this to place a capsule in the first empty inventory slot.
int mbPlayerCapsuleAdd(int playerNo, int capsuleNo)
{
    GW_PLAYER *playerP = GWPlayerGet(playerNo);
    int capsuleLimit = mbPlayerCapsuleMaxGet();
    int capsuleIndex;

    for (capsuleIndex = 0; capsuleIndex < capsuleLimit; capsuleIndex++) {
        s8 *capsuleP;

        if (mbPlayerCapsuleGet(playerNo, capsuleIndex) != -1) {
            continue;
        }
        *PlayerCapsulePtrGet(playerNo, capsuleIndex) = capsuleNo;
        return capsuleIndex;
    }
    return -1;
}

// Capsule use and shop code call this to remove a slot and shift later capsules down.
int mbPlayerCapsuleRemove(int playerNo, int index)
{
    int capsuleNo = mbPlayerCapsuleGet(playerNo, index);
    GW_PLAYER *playerP = GWPlayerGet(playerNo);
    int capsuleLimit;
    int capsuleIndex;

    if (capsuleNo == -1) {
        return capsuleNo;
    }
    capsuleLimit = mbPlayerCapsuleMaxGet();
    for (capsuleIndex = index; capsuleIndex < capsuleLimit - 1;
         capsuleIndex++) {
        *PlayerCapsulePtrGet(playerNo, capsuleIndex) =
            *PlayerCapsulePtrGet(playerNo, capsuleIndex + 1);
    }
    for (; capsuleIndex < capsuleLimit; capsuleIndex++) {
        *PlayerCapsulePtrGet(playerNo, capsuleIndex) = -1;
    }
    return capsuleNo;
}

// Capsule menu code calls this to find the slot containing a selected capsule.
int mbPlayerCapsuleFind(int playerNo, int capsuleNo)
{
    int capsuleLimit = mbPlayerCapsuleMaxGet();
    int capsuleIndex;

    for (capsuleIndex = 0; capsuleIndex < capsuleLimit; capsuleIndex++) {
        if (capsuleNo == mbPlayerCapsuleGet(playerNo, capsuleIndex)) {
            return capsuleIndex;
        }
    }
    return -1;
}

s8 mbPlayerCapsuleGet(int playerNo, int index)
{
    return *PlayerCapsulePtrGet(playerNo, index);
}

s8 mbPlayerTeamCapsuleGet(int teamNo, int index)
{
    return mbPlayerCapsuleGet(mbPlayerTeamFindPlayer(teamNo, 0), index);
}

// Counts capsules up to the first empty slot in one player's three-slot share.
static inline int PlayerCountCapsules(int playerNo)
{
    int capsuleCount;

    for (capsuleCount = 0; capsuleCount < 3; capsuleCount++) {
        if (GwPlayer[playerNo].capsule[capsuleCount] == -1) {
            break;
        }
    }
    return capsuleCount;
}

// Board menus and events call this to count a player's capsules, including their teammate's share.
int mbPlayerCapsuleNumGet(int playerNo)
{
    int capsuleCount = PlayerCountCapsules(playerNo);

    if (GWTeamFGet()) {
        int teammateNo = mbPlayerTeamFind(playerNo);

        capsuleCount += PlayerCountCapsules(teammateNo);
        // Teams can use at most five of their six capsule storage slots.
        if (capsuleCount > 5) {
            capsuleCount = 5;
        }
    }
    return capsuleCount;
}

int mbPlayerTeamCapsuleNumGet(int teamNo)
{
    return mbPlayerCapsuleNumGet(mbPlayerTeamFindPlayer(teamNo, 0));
}

// PlayerTurn uses this after movement to test for another player on the specified space.
// Teammates also count as other players.
BOOL mbPlayerKettouCheck(int playerNo, s16 masuId)
{
    int otherPlayerNo;

    for (otherPlayerNo = 0; otherPlayerNo < GW_PLAYER_MAX; otherPlayerNo++) {
        if (playerNo != otherPlayerNo && masuId == GwPlayer[otherPlayerNo].masuId) {
            return TRUE;
        }
    }
    return FALSE;
}

// Board events call this to play the character voice associated with a win or loss animation.
void mbPlayerWinLoseVoicePlay(int playerNo, int motNo, int seId)
{
    MBOBJMODEL *objP = mbObjGet(mbPlayerObjIDGet(playerNo));
    u8 charNo = GwPlayer[playerNo].charNo;

    CharWinLoseVoicePlay(charNo, objP->motId[motNo], seId);
}

int mbPlayerVoicePanPlay(int playerNo, s16 seId)
{
    return mbObjSePlay(mbPlayerObjIDGet(playerNo), seId);
}

// Board events call this to play a character voice positioned at the player's current location.
int mbPlayerVoicePlay(int playerNo, s16 seId)
{
    HuVecF pos;
    u8 pan;

    mbPlayerPosGet(playerNo, &pos);
    pan = mbAudFXPosPanGet(&pos);
    return CharFXPlayVolPan(
        GwPlayer[playerNo].charNo, seId, MSM_VOL_MAX, pan);
}

// Board events request player visibility; players on space zero are always hidden.
void mbPlayerDispSet(int playerNo, BOOL dispF)
{
    if (GwPlayer[playerNo].masuId == 0) {
        dispF = FALSE;
    }
    mbObjDispSet(mbPlayerObjIDGet(playerNo), dispF);
}

BOOL mbPlayerDispGet(int playerNo)
{
    return mbObjDispGet(mbPlayerObjIDGet(playerNo));
}

// Indexes a 13-color table by character ID; Mini Koopa B's index 13 reads beyond the table.
GXColor mbPlayerColorGet(int playerNo)
{
    GXColor color[] = {
        { 227, 67, 67, 255 },
        { 68, 67, 227, 255 },
        { 241, 158, 220, 255 },
        { 67, 228, 68, 255 },
        { 138, 60, 180, 255 },
        { 227, 228, 68, 255 },
        { 192, 192, 192, 255 },
        { 227, 227, 227, 255 },
        { 40, 227, 227, 255 },
        { 227, 139, 40, 255 },
        { 180, 40, 40, 255 },
        { 40, 180, 40, 255 },
        { 40, 40, 180, 255 }
    };

    return color[GwPlayer[playerNo].charNo];
}

const s8 lbl_8021A9E4[20] = {
    -1, -1, -1, -1,
    -1, -1, -1, -1,
    -1, -1, -1, -1,
    -1, -1, -1, -1,
    0, 0, 0, 0
};

void mbPlayerBlackoutSet(BOOL blackoutEnabledF)
{
    blackoutF = blackoutEnabledF;
}

BOOL mbPlayerBlackoutGet(void)
{
    return blackoutF;
}

void mbPlayerMasuCornerSet(int playerNo, s8 cornerNo)
{
    playerWork[playerNo].masuCorner = cornerNo;
}

s8 mbPlayerMasuCornerGet(int playerNo)
{
    return playerWork[playerNo].masuCorner;
}

// Shows and applies coin changes from plus, cap-coin, and minus spaces; the last-five effect
// triples the amount.
static void MasuCoinExec(int playerNo, int coinNum)
{
    HuVecF pos;
    BOOL motionDoneF;
    s8 coinDisplayId;

    if (coinNum < 0) {
        omVibrate(playerNo, 20, 4, 4);
    }
    mbPlayerRotateStart(playerNo, 0, 15);
    if (GwSystem.last5Effect == 1) {
        coinNum *= 3;
    }
    mbPlayerPosGet(playerNo, &pos);
    pos.y += 250.0f;
    if (coinNum >= 0) {
        mbAudFXPlay(PLAYER_COIN_GAIN_SFX);
    } else {
        mbAudFXPlay(PLAYER_COIN_LOSS_SFX);
    }
    coinDisplayId = mbCoinDispMasuCreate(&pos, coinNum, FALSE);
    while (!mbPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    if (coinNum >= 0) {
        mbPlayerWinLoseVoicePlay(playerNo, 12, CHARVOICEID(6));
        mbPlayerMotionShiftSet(
            playerNo, 12, 0.0f, 4.0f, HU3D_ATTR_NONE);
    } else {
        mbPlayerWinLoseVoicePlay(playerNo, 13, CHARVOICEID(12));
        mbPlayerMotionShiftSet(
            playerNo, 13, 0.0f, 4.0f, HU3D_ATTR_NONE);
    }
    mbCoinAddExec(playerNo, coinNum);
    mbCameraMoveWait();
    for (motionDoneF = FALSE;
         !mbCoinDispKillCheck(coinDisplayId) || !motionDoneF;) {
        if (mbPlayerMotionEndCheck(playerNo) && !motionDoneF) {
            mbPlayerMotIdleSet(playerNo);
            motionDoneF = TRUE;
        }
        HuPrcVSleep();
    }
}

// The plus-space event shows a base three-coin reward with a close-up; MasuCoinExec may triple it.
void mbPlayerPlusMasuExec(int playerNo)
{
    mbCameraMoveOnSet(TRUE);
    mbCameraPlayerViewSet(playerNo, MB_CAMERA_VIEW_ZOOMIN);
    MasuCoinExec(playerNo, 3);
}

// The coin-cap event shows a base five-coin reward with a close-up; MasuCoinExec may triple it.
void mbPlayerCapCoinMasuExec(int playerNo)
{
    mbCameraMoveOnSet(TRUE);
    mbCameraPlayerViewSet(playerNo, MB_CAMERA_VIEW_ZOOMIN);
    MasuCoinExec(playerNo, 5);
}

// The minus-space event shows a base three-coin loss with a close-up; MasuCoinExec may triple it.
void mbPlayerMinusMasuExec(int playerNo)
{
    mbCameraMoveOnSet(TRUE);
    mbCameraPlayerViewSet(playerNo, MB_CAMERA_VIEW_ZOOMIN);
    MasuCoinExec(playerNo, -3);
}
