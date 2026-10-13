/* Mount Duel tournament characters, rounds, and result scenes. */
#include "dolphin/math.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "game/charman.h"
#include "game/gamework.h"
#include "game/mgdata.h"
#include "REL/mgmtournamentdll/module_types.h"
#include "datadir_enum.h"
#include "game/memory.h"
#include "msm_se.h"
#include "msm_stream.h"
#include "game/frand.h"
#include "dolphin/types.h"
#include <string.h>
#include "REL/mgmtournamentdll/mgmrenshodll_module_types.h"
#include "dolphin/gx.h"
#include "game/main.h"
#include "game/audio.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "REL/mgmtournamentdll/mgmbingodll_module_types.h"
#include "dolphin.h"
#include "dolphin/os.h"
#include "dolphin/mtx.h"
#include "humath.h"
#include "messdir_enum.h"
#include "game/window.h"
#include "game/process.h"
#include "game/flag.h"
#include "game/armem.h"
#include "game/printfunc.h"
#include "game/saveload.h"
#include "game/esprite.h"
#include "game/thpmain.h"
#include "game/board/main.h"
#include "game/board/audio.h"
#include "game/board/masu.h"
#include "game/board/player.h"

#define TOURNAMENT_BONUS_ODDS_ONE_LEVEL 65
#define TOURNAMENT_BONUS_ODDS_TWO_LEVELS 75
#define TOURNAMENT_BONUS_ODDS_THREE_LEVELS 85
#define TOURNAMENT_BONUS_ODDS_EQUAL_LEVEL 50
#define TOURNAMENT_PERCENT_RANGE 100
#define TOURNAMENT_ROUND_COIN_BONUS 10
#define TOURNAMENT_CHARACTER_PRIORITY 4608
#define TOURNAMENT_CHARACTER_MOTION_COUNT 13
#define TOURNAMENT_CHARACTER_LIGHT_INTENSITY 192
#define TOURNAMENT_ORDER_UNUSED 255
#define TOURNAMENT_ALL_MOTION_PAUSE_FLAGS \
    (HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE | \
     HU3D_MOTATTR_SHAPE_PAUSE | HU3D_MOTATTR_OVL_PAUSE)

#define MSM_SE_MGTOUR_PROP_REVEAL 1212
#define MSM_SE_MGTOUR_PROP_LAND 1213
#define MSM_SE_MGTOUR_PROP_DECIDE 1214
#define MSM_SE_MGTOUR_ROUND_WIN 1249
#define MSM_SE_MGTOUR_ROUND_LOSE 1250
#define MSM_SE_MGTOUR_CHAMPION 1252

void fn_1_B628(void);
OMOBJMAN *lbl_1_bss_12C;
s16 lbl_1_bss_128;
OMOBJ *lbl_1_bss_114[5];
s16 lbl_1_bss_10C[4];
s16 lbl_1_bss_10A;
s16 lbl_1_bss_108;

u32 lbl_1_data_510[13] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 1),
    DATANUM(DATA_mariomot, 2), DATANUM(DATA_mariomot, 3),
    DATANUM(DATA_mariomot, 4), DATANUM(DATA_mario, 120),
    DATANUM(DATA_mariomot, 6), DATANUM(DATA_mariomot, 40),
    DATANUM(DATA_mariomot, 36), DATANUM(DATA_mariomot, 37),
    DATANUM(DATA_mariomot, 34), DATANUM(DATA_mariomot, 22),
    DATANUM(DATA_mariomot, 119)
};

f32 lbl_1_data_544 = 6.0f;

Vec lbl_1_data_548[4] = {
    {-3e+02f, -1.55e+03f, 1.6e+03f},
    {-1e+02f, -1.55e+03f, 1.6e+03f},
    {1e+02f, -1.55e+03f, 1.6e+03f},
    {3e+02f, -1.55e+03f, 1.6e+03f}
};

TournamentMotionData lbl_1_data_578[14] = {
    {0, 0.0f, HU3D_MOTATTR_LOOP, 1U, 0},
    {0, 8.0f, HU3D_MOTATTR_LOOP, 1U, 0},
    {1, 8.0f, HU3D_MOTATTR_LOOP, 1U, 0},
    {2, 8.0f, HU3D_MOTATTR_LOOP, 1U, 0},
    {3, 8.0f, HU3D_MOTATTR_LOOP, 1U, 0},
    {4, 0.0f, HU3D_MOTATTR_LOOP, 1U, 0},
    {6, 8.0f, HU3D_MOTATTR_LOOP, 1U, 0},
    {7, 8.0f, HU3D_MOTATTR_LOOP, 1U, 0},
    {8, 8.0f, HU3D_MOTATTR_LOOP, 1U, 0},
    {9, 8.0f, HU3D_MOTATTR_LOOP, 1U, 0},
    {10, 8.0f, HU3D_MOTATTR_LOOP, 1U, 0},
    {11, 8.0f, HU3D_MOTATTR_LOOP, 1U, 0},
    {12, 8.0f, HU3D_MOTATTR_LOOP, 1U, 0},
    {5, 8.0f, HU3D_MOTATTR_LOOP, 1U, 1}
};

char lbl_1_data_690[16] = "f-itemhook-r";

/* Selects a tournament character animation when its object callback changes state.
 * Both table-flag branches set the same motion attribute; requested mode zero is recorded as one.
 */
void fn_1_6278(OMOBJ *object, s16 modeNo)
{
    s16 motionNo;
    TournamentPlayerAnimationWork *work;

    work = object->data;
    if ((work->playerNo < 0) || (work->playerNo >= 4) ||
        (modeNo < 0) || (modeNo >= 14)) {
        return;
    }
    if (work->motionMode != modeNo) {
        motionNo = lbl_1_data_578[modeNo].motionNo;
        if (motionNo != -1) {
            if (lbl_1_data_578[modeNo].transitionTime <= (1.0f)) {
                CharMotionSet(work->characterNo, object->mtnId[motionNo]);
                Hu3DModelAttrReset(object->mdlId[0],
                                   Hu3DModelMotionAttrGet(object->mdlId[0]) | HU3D_MOTATTR);
                CharModelAttrSet(work->characterNo, lbl_1_data_578[modeNo].modelAttributes);
            } else {
                Hu3DModelAttrSet(object->mdlId[0], HU3D_MOTATTR_PAUSE);
                CharMotionShiftSet(work->characterNo, object->mtnId[motionNo], (0.0f),
                                   lbl_1_data_578[modeNo].transitionTime,
                                   lbl_1_data_578[modeNo].modelAttributes);
            }
            if (lbl_1_data_578[modeNo].motionAttributeTableFlag != 0) {
                Hu3DMotionAttrSet(object->mtnId[motionNo], 1);
            } else {
                Hu3DMotionAttrSet(object->mtnId[motionNo], 1);
            }
            CharModelVoiceFlagSet(work->characterNo, lbl_1_data_578[modeNo].motionEffectsEnabled);
        }
        work->motionMode = modeNo;
        if (work->motionMode == 0) {
            work->motionMode = 1;
        }
    }
}

s16 fn_1_A42C(void);

/* Assigns the four players to the active tournament sides before a round. */
void fn_1_64A8(void)
{
    s16 mode;
    s16 player;

    mode = fn_1_A42C();
    player = 0;
    while (player < 4) {
        if (mode == 0) {
            if (MgModeWorkInt[player] == 0) {
                GwPlayerConf[player].grpNo = 0;
            } else if (MgModeWorkInt[player] == 1) {
                GwPlayerConf[player].grpNo = 1;
            } else {
                GwPlayerConf[player].grpNo = 2;
            }
        } else if (mode == 1) {
            if (MgModeWorkInt[player] == 2) {
                GwPlayerConf[player].grpNo = 0;
            } else if (MgModeWorkInt[player] == 3) {
                GwPlayerConf[player].grpNo = 1;
            } else {
                GwPlayerConf[player].grpNo = 2;
            }
        } else if (mode == 2) {
            if ((MgModeWorkInt[player + 4] == 2) &&
                ((MgModeWorkInt[player] == 0) || (MgModeWorkInt[player] == 1))) {
                GwPlayerConf[player].grpNo = 0;
            } else if ((MgModeWorkInt[player + 4] == 2) &&
                       ((MgModeWorkInt[player] == 2) || (MgModeWorkInt[player] == 3))) {
                GwPlayerConf[player].grpNo = 1;
            } else {
                GwPlayerConf[player].grpNo = 2;
            }
        } else {
            if ((MgModeWorkInt[player + 4] == 1) &&
                ((MgModeWorkInt[player] == 0) || (MgModeWorkInt[player] == 1))) {
                GwPlayerConf[player].grpNo = 0;
            } else if ((MgModeWorkInt[player + 4] == 1) &&
                       ((MgModeWorkInt[player] == 2) || (MgModeWorkInt[player] == 3))) {
                GwPlayerConf[player].grpNo = 1;
            } else {
                GwPlayerConf[player].grpNo = 2;
            }
        }
        player++;
    }
}

f32 InterpolateBMLine(f32 *values, f32 *times, f32 value);

/* Eases a character position between two points during a timed scene movement. */
void fn_1_68A4(Vec *result, const Vec *point0, const Vec *point1,
               f32 duration, f32 time)
{
    f32 times[4];
    f32 values[4];

    times[0] = (-1.0f) * duration;
    times[1] = (0.0f);
    times[2] = duration;
    times[3] = (2.0f) * duration;
    time = (f32)((f64)duration *
        sin(((3.141592653589793) *
            (f64)((90.0f) * (time / duration))) /
            (180.0)));

    values[0] = point0->x;
    values[1] = point0->x;
    values[2] = point1->x;
    values[3] = point1->x;
    result->x = InterpolateBMLine(values, times, time);
    values[0] = point0->y;
    values[1] = point0->y;
    values[2] = point1->y;
    values[3] = point1->y;
    result->y = InterpolateBMLine(values, times, time);
    values[0] = point0->z;
    values[1] = point0->z;
    values[2] = point1->z;
    values[3] = point1->z;
    result->z = InterpolateBMLine(values, times, time);
}

extern int MgModeWorkInt[16];
s16 fn_1_A42C(void);

/* Reports whether the selected sides contain at least two players of nonzero configured type. */
s32 fn_1_6A24(void)
{
    s16 mode;
    s16 typeCount;
    s16 player;
    TournamentPlayerAnimationWork *playerState;

    mode = fn_1_A42C();
    typeCount = 0;
    player = 0;
    while (player < 4) {
        playerState = lbl_1_bss_114[player]->data;
        if (mode == 0) {
            if ((MgModeWorkInt[player] == 0) && (playerState->playerType != 0)) {
                typeCount++;
            }
            if ((MgModeWorkInt[player] == 1) && (playerState->playerType != 0)) {
                typeCount++;
            }
        } else if (mode == 1) {
            if ((MgModeWorkInt[player] == 2) && (playerState->playerType != 0)) {
                typeCount++;
            }
            if ((MgModeWorkInt[player] == 3) && (playerState->playerType != 0)) {
                typeCount++;
            }
        } else if (mode == 2) {
            if ((MgModeWorkInt[player + 4] == 2) &&
                ((MgModeWorkInt[player] == 0) || (MgModeWorkInt[player] == 1)) &&
                (playerState->playerType != 0)) {
                typeCount++;
            }
            if ((MgModeWorkInt[player + 4] == 2) &&
                ((MgModeWorkInt[player] == 2) || (MgModeWorkInt[player] == 3)) &&
                (playerState->playerType != 0)) {
                typeCount++;
            }
        } else {
            if ((MgModeWorkInt[player + 4] == 1) &&
                ((MgModeWorkInt[player] == 0) || (MgModeWorkInt[player] == 1)) &&
                (playerState->playerType != 0)) {
                typeCount++;
            }
            if ((MgModeWorkInt[player + 4] == 1) &&
                ((MgModeWorkInt[player] == 2) || (MgModeWorkInt[player] == 3)) &&
                (playerState->playerType != 0)) {
                typeCount++;
            }
        }
        player++;
    }
    if (typeCount >= 2) {
        return 1;
    }
    return 0;
}

 s16 fn_1_A42C(void);

void fn_1_71E8(s16 *firstPlayer, s16 *secondPlayer);

/* For a round skipped at the prompt, randomly chooses one player from the current matchup to
 * receive a ten-coin bonus, weighted by configured difficulty. Practice mode leaves every player's
 * coin bonus unchanged. */
void fn_1_6D78(void)
{
    s16 firstPlayer;
    s16 secondPlayer;
    s16 player;
    s16 chance;
    TournamentPlayerAnimationWork *firstPlayerData;
    TournamentPlayerAnimationWork *secondPlayerData;

    chance = fn_1_A42C();
    player = 0;
    while (player < 4) {
        if (chance == 0) {
            if (MgModeWorkInt[player] == 0) {
                firstPlayer = player;
            } else if (MgModeWorkInt[player] == 1) {
                secondPlayer = player;
            }
        } else if (chance == 1) {
            if (MgModeWorkInt[player] == 2) {
                firstPlayer = player;
            } else if (MgModeWorkInt[player] == 3) {
                secondPlayer = player;
            }
        } else if (chance == 2) {
            if ((MgModeWorkInt[player + 4] == 2) &&
                ((MgModeWorkInt[player] == 0) || (MgModeWorkInt[player] == 1))) {
                firstPlayer = player;
            } else if ((MgModeWorkInt[player + 4] == 2) &&
                       ((MgModeWorkInt[player] == 2) || (MgModeWorkInt[player] == 3))) {
                secondPlayer = player;
            }
        } else {
            if ((MgModeWorkInt[player + 4] == 1) &&
                ((MgModeWorkInt[player] == 0) || (MgModeWorkInt[player] == 1))) {
                firstPlayer = player;
            } else if ((MgModeWorkInt[player + 4] == 1) &&
                       ((MgModeWorkInt[player] == 2) || (MgModeWorkInt[player] == 3))) {
                secondPlayer = player;
            }
        }
        player++;
    }

    player = 0;
    while (player < 4) {
        if (!_CheckFlag(FLAG_MG_PRACTICE)) {
            GwPlayer[player].mgCoinBonus = 0;
        }
        player++;
    }

    /* A greater configured difficulty increases that player's chance of the ten-coin bonus. */
    firstPlayerData = lbl_1_bss_114[firstPlayer]->data;
    secondPlayerData = lbl_1_bss_114[secondPlayer]->data;
    if (firstPlayerData->difficulty > secondPlayerData->difficulty) {
        chance = firstPlayerData->difficulty - secondPlayerData->difficulty;
        if (chance == 1) {
            chance = TOURNAMENT_BONUS_ODDS_ONE_LEVEL;
        } else if (chance == 2) {
            chance = TOURNAMENT_BONUS_ODDS_TWO_LEVELS;
        } else {
            chance = TOURNAMENT_BONUS_ODDS_THREE_LEVELS;
        }
        if (frandmod(TOURNAMENT_PERCENT_RANGE) < chance) {
            chance = firstPlayer;
        } else {
            chance = secondPlayer;
        }
    } else if (secondPlayerData->difficulty > firstPlayerData->difficulty) {
        chance = secondPlayerData->difficulty - firstPlayerData->difficulty;
        if (chance == 1) {
            chance = TOURNAMENT_BONUS_ODDS_ONE_LEVEL;
        } else if (chance == 2) {
            chance = TOURNAMENT_BONUS_ODDS_TWO_LEVELS;
        } else {
            chance = TOURNAMENT_BONUS_ODDS_THREE_LEVELS;
        }
        if (frandmod(TOURNAMENT_PERCENT_RANGE) < chance) {
            chance = secondPlayer;
        } else {
            chance = firstPlayer;
        }
    } else {
        chance = TOURNAMENT_BONUS_ODDS_EQUAL_LEVEL;
        if (frandmod(TOURNAMENT_PERCENT_RANGE) < chance) {
            chance = firstPlayer;
        } else {
            chance = secondPlayer;
        }
    }
    if (!_CheckFlag(FLAG_MG_PRACTICE)) {
        GwPlayer[chance].mgCoinBonus = TOURNAMENT_ROUND_COIN_BONUS;
    }
}

s16 fn_1_A42C(void);

/* Finds the players occupying the two sides selected for the current round. */
void fn_1_71E8(s16 *firstPlayer, s16 *secondPlayer)
{
    s16 playerMode;
    s16 playerIndex;

    playerMode = fn_1_A42C();
    playerIndex = 0;
    while (playerIndex < 4) {
        if (playerMode == 0) {
            if (MgModeWorkInt[playerIndex] == 0) {
                *firstPlayer = playerIndex;
            } else if (MgModeWorkInt[playerIndex] == 1) {
                *secondPlayer = playerIndex;
            }
        } else if (playerMode == 1) {
            if (MgModeWorkInt[playerIndex] == 2) {
                *firstPlayer = playerIndex;
            } else if (MgModeWorkInt[playerIndex] == 3) {
                *secondPlayer = playerIndex;
            }
        } else if (playerMode == 2) {
            if ((MgModeWorkInt[playerIndex + 4] == 2) &&
                ((MgModeWorkInt[playerIndex] == 0) || (MgModeWorkInt[playerIndex] == 1))) {
                *firstPlayer = playerIndex;
            } else if ((MgModeWorkInt[playerIndex + 4] == 2) &&
                ((MgModeWorkInt[playerIndex] == 2) || (MgModeWorkInt[playerIndex] == 3))) {
                *secondPlayer = playerIndex;
            }
        } else {
            if ((MgModeWorkInt[playerIndex + 4] == 1) &&
                ((MgModeWorkInt[playerIndex] == 0) || (MgModeWorkInt[playerIndex] == 1))) {
                *firstPlayer = playerIndex;
            } else if ((MgModeWorkInt[playerIndex + 4] == 1) &&
                ((MgModeWorkInt[playerIndex] == 2) || (MgModeWorkInt[playerIndex] == 3))) {
                *secondPlayer = playerIndex;
            }
        }
        playerIndex++;
    }
}

void fn_1_8238(OMOBJ *object);
void fn_1_A1F4(void);

/* Changes a character's motion when its tournament scene animation changes.
 * Both table-flag branches set the same motion attribute; requested mode zero is recorded as one.
 */
static inline void tournamentPlayerMotionSet(OMOBJ *object, s16 modeNo)
{
    s16 motionNo;
    TournamentPlayerAnimationWork *work;

    work = object->data;
    if ((work->playerNo < 0) || (work->playerNo >= 4) ||
        (modeNo < 0) || (modeNo >= 14)) {
        return;
    }
    if (work->motionMode != modeNo) {
        motionNo = lbl_1_data_578[modeNo].motionNo;
        if (motionNo != -1) {
            if (lbl_1_data_578[modeNo].transitionTime <= (1.0f)) {
                CharMotionSet(work->characterNo, object->mtnId[motionNo]);
                Hu3DModelAttrReset(*object->mdlId,
                    Hu3DModelMotionAttrGet(*object->mdlId) | HU3D_MOTATTR);
                CharModelAttrSet(work->characterNo,
                    lbl_1_data_578[modeNo].modelAttributes);
            } else {
                Hu3DModelAttrSet(*object->mdlId, HU3D_MOTATTR_PAUSE);
                CharMotionShiftSet(work->characterNo, object->mtnId[motionNo],
                    (0.0f), lbl_1_data_578[modeNo].transitionTime,
                    lbl_1_data_578[modeNo].modelAttributes);
            }
            if (lbl_1_data_578[modeNo].motionAttributeTableFlag != 0) {
                Hu3DMotionAttrSet(object->mtnId[motionNo], 1);
            } else {
                Hu3DMotionAttrSet(object->mtnId[motionNo], 1);
            }
            CharModelVoiceFlagSet(work->characterNo,
                lbl_1_data_578[modeNo].motionEffectsEnabled);
        }
        work->motionMode = modeNo;
        if (work->motionMode == 0) {
            work->motionMode = 1;
        }
    }
}

/* Creates the four player character objects and loads their tournament motions. */
void fn_1_74BC(void)
{
    s32 player;
    s32 motionIndex;
    OMOBJ *playerObject;
    TournamentPlayerAnimationWork *playerData;

    player = 0;
    while (player < 4) {
        playerObject = omAddObjEx(lbl_1_bss_12C, TOURNAMENT_CHARACTER_PRIORITY, 1,
                                  TOURNAMENT_CHARACTER_MOTION_COUNT, -1, fn_1_8238);
        lbl_1_bss_114[player] = playerObject;
        playerObject->stat |= OM_STAT_MODELPAUSE;
        playerObject->work[0] = playerObject->work[1] =
            playerObject->work[2] = playerObject->work[3] = 0;
        playerObject->data = HuMemDirectMallocNum(HEAP_HEAP, 84, HU_MEMNUM_OVL);
        memset(playerObject->data, 0, 84);
        playerData = playerObject->data;
        playerData->playerNo = player;
        playerData->characterNo = GwPlayerConf[player].charNo;
        playerData->padNo = GwPlayerConf[player].padNo;
        playerData->groupNo = GwPlayerConf[player].grpNo;
        playerData->playerType = GwPlayerConf[player].type;
        playerData->difficulty = GwPlayerConf[player].comDif;
        playerData->sceneState = 0;
        *playerObject->mdlId = CharModelCreate(playerData->characterNo, 2);
        motionIndex = 0;
        while (motionIndex < TOURNAMENT_CHARACTER_MOTION_COUNT) {
            playerObject->mtnId[motionIndex] = CharMotionCreate(
                playerData->characterNo, lbl_1_data_510[motionIndex]);
            motionIndex++;
        }
        CharMotionDataClose(playerData->characterNo);
        playerData->motionMode = -1;
        tournamentPlayerMotionSet(playerObject, 0);
        Hu3DModelCameraSet(*playerObject->mdlId, 1);
        Hu3DModelLayerSet(*playerObject->mdlId, 1);
        Hu3DModelShadowSet(*playerObject->mdlId);
        playerData->modelHeight = CharModelHeightGet(playerData->characterNo);
        playerData->position = lbl_1_data_548[player];
        Hu3DModelPosSetV(*playerObject->mdlId, &playerData->position);
        playerData->rotation.x = playerData->rotation.y =
            playerData->rotation.z = (0.0f);
        Hu3DModelRotSetV(*playerObject->mdlId, &playerData->rotation);
        Hu3DLLightCreate(*playerObject->mdlId, (0.0f), (1500.0f), (1500.0f), (0.0f),
                         (-0.699999988079071f), (-0.699999988079071f),
                         TOURNAMENT_CHARACTER_LIGHT_INTENSITY, TOURNAMENT_CHARACTER_LIGHT_INTENSITY,
                         TOURNAMENT_CHARACTER_LIGHT_INTENSITY);
        CharMotionVoiceOnSet(playerData->characterNo,
            (s16)lbl_1_data_510[8], 0);
        CharMotionVoiceOnSet(playerData->characterNo,
            (s16)lbl_1_data_510[3], 0);
        player++;
    }
    CharEffectLayerSet(4);
    fn_1_A1F4();
}

extern f32 MgModeWorkFloat[16];

/* Restores the character positions saved before the minigame and starts the eliminated
 * player's settled pose on return. */
void fn_1_7924(void)
{
    s16 playerIndex;
    OMOBJ *playerObject;
    TournamentPlayerAnimationWork *playerWork;

    playerIndex = 0;
    while (playerIndex < 4) {
        playerObject = lbl_1_bss_114[playerIndex];
        playerWork = playerObject->data;
        Hu3DModelLayerSet(*playerObject->mdlId, 4);
        playerWork->position.x = MgModeWorkFloat[playerIndex];
        playerWork->position.y = MgModeWorkFloat[playerIndex + 4];
        playerWork->position.z = MgModeWorkFloat[playerIndex + 8];
        Hu3DModelPosSetV(*playerObject->mdlId, &playerWork->position);
        if (MgModeWorkInt[playerIndex + 4] == 3) {
            playerWork->sceneState = 9;
            CharMotionVoiceOnSet(playerWork->characterNo,
                (s16)lbl_1_data_510[11], FALSE);
            fn_1_6278(playerObject, 11);
        }
        playerIndex++;
    }
}

extern s16 lbl_1_data_360;
extern OMOBJ *lbl_1_bss_140[4];

/* Supplies a player's character number when the result scene builds its display. */
s16 fn_1_7BC0(s16 player)
{
    TournamentPlayerAnimationWork *playerData;
    s16 characterNo;

    playerData = lbl_1_bss_114[player]->data;
    characterNo = playerData->characterNo;
    return characterNo;
}

extern OMOBJ *lbl_1_bss_ACC[4];

/* Returns the selected player's type for tournament player and side checks. */
s16 fn_1_7C00(s16 playerNo) {
    s32 *playerData = (s32 *) ((OMOBJ **) &lbl_1_bss_114)[playerNo]->data;
    s16 playerType = (s16) playerData[2];

    return playerType;
}

extern OMOBJ *lbl_1_bss_A8;

/* Returns the controller slot assigned to the selected player. */
s16 fn_1_7C44(s16 playerNo) {
    s16 *playerData = lbl_1_bss_114[playerNo]->data;

    return playerData[2];
}

/* Calculates the average world position of the four tournament characters. */
void fn_1_7C78(Vec *center) {
    s16 playerNo;
    TournamentPlayerAnimationWork *playerData;

    center->x = center->y = center->z = (0.0f);
    for (playerNo = 0; playerNo < 4; ++playerNo) {
        playerData = lbl_1_bss_114[playerNo]->data;
        center->x += playerData->position.x;
        center->y += playerData->position.y;
        center->z += playerData->position.z;
    }
    center->x *= (0.25f);
    center->y *= (0.25f);
    center->z *= (0.25f);
}

/* Selects the win or loss motion state for a player in the round result scene. */
void fn_1_7D58(s16 playerNo, s16 isWinner) {
    TournamentPlayerAnimationWork *playerData;

    playerData = lbl_1_bss_114[playerNo]->data;
    if (isWinner != 0) {
        playerData->sceneState = 10;
        return;
    }
    playerData->sceneState = 13;
}

/* Selects the next win or loss motion state for a player after the result. */
void fn_1_7DA8(s16 playerNo, s16 isWinner) {
    TournamentPlayerAnimationWork *playerData;

    playerData = lbl_1_bss_114[playerNo]->data;
    if (isWinner != 0) {
        playerData->sceneState = 16;
        return;
    }
    playerData->sceneState = 19;
}

/* Starts the player's presentation pose with the matchup side-facing turn disabled. */
void fn_1_7DF8(s16 playerNo)
{
    TournamentPlayerAnimationWork *playerData;

    playerData = lbl_1_bss_114[playerNo]->data;
    lbl_1_bss_114[playerNo]->work[1] = 0;
    playerData->sceneState = 25;
}

/* Starts the player's presentation pose with the matchup side-facing turn enabled. */
void fn_1_7E50(s16 playerNo)
{
    TournamentPlayerAnimationWork *playerData;

    playerData = lbl_1_bss_114[playerNo]->data;
    lbl_1_bss_114[playerNo]->work[1] = 1;
    playerData->sceneState = 25;
}

/* Sends any listed result pose to its return-to-idle state when the scene continues,
 * even if that pose has not finished. */
void fn_1_7EA8(s16 playerNo) {
    TournamentPlayerAnimationWork *playerData;

    playerData = lbl_1_bss_114[playerNo]->data;

    if ((playerData->sceneState == 10) || (playerData->sceneState == 11)) {
        playerData->sceneState = 12;
    }
    if ((playerData->sceneState == 13) || (playerData->sceneState == 14)) {
        playerData->sceneState = 15;
    }
    if ((playerData->sceneState == 16) || (playerData->sceneState == 17)) {
        playerData->sceneState = 18;
    }
    if ((playerData->sceneState == 19) || (playerData->sceneState == 20)) {
        playerData->sceneState = 21;
    }
    if ((playerData->sceneState == 25) || (playerData->sceneState == 26)) {
        playerData->sceneState = 27;
    }
}

/* Starts movement to the supplied destination when the scene positions a player. */
void fn_1_7F78(s16 playerNo, Vec *position) {
    TournamentPlayerAnimationWork *playerData;

    playerData = lbl_1_bss_114[playerNo]->data;
    playerData->sceneState = 1;
    playerData->targetPosition = *position;
}

/* Stores the next scene state and position for the selected tournament player. */
void fn_1_7FC8(s16 playerNo, Vec *position) {
    TournamentPlayerAnimationWork *playerData;

    playerData = lbl_1_bss_114[playerNo]->data;
    playerData->sceneState = 4;
    playerData->targetPosition = *position;
}

/* Starts the player's elimination fall to y = -500 unless it is already falling or settled. */
void fn_1_8018(s16 player)
{
    TournamentPlayerAnimationWork *playerData;

    playerData = lbl_1_bss_114[player]->data;
    if ((playerData->sceneState != 7) && (playerData->sceneState != 8) &&
        (playerData->sceneState != 9)) {
        playerData->sceneState = 7;
        lbl_1_bss_114[player]->work[0] = 0;
    }
}

/* Marks the selected player as reacting to the tournament result. */
void fn_1_8094(s16 player)
{
    TournamentPlayerAnimationWork *playerData;

    playerData = lbl_1_bss_114[player]->data;
    playerData->sceneState = 22;
}

/* Reports readiness in the idle, settled, or prop-attached state. The prop-attached state
 * counts as ready before its animation and prop movement finish. */
s16 fn_1_80CC(s16 player)
{
    TournamentPlayerAnimationWork *playerData;

    playerData = lbl_1_bss_114[player]->data;
    if ((playerData->sceneState == 0) || (playerData->sceneState == 9) ||
        (playerData->sceneState == 24)) {
        return 1;
    }
    return 0;
}

/* Snaps a character's current and destination positions to one scene point. */
void fn_1_812C(s16 playerNo, Vec *position) {
    TournamentPlayerAnimationWork *characterWork;

    characterWork = lbl_1_bss_114[playerNo]->data;
    characterWork->position = *position;
    characterWork->targetPosition = *position;
}

/* Copies the four character positions into shared tournament scene work. */
void fn_1_818C(void) {
    s16 playerNo;
    TournamentPlayerAnimationWork *playerData;

    playerNo = 0;
    while (playerNo < 4) {
        playerData = lbl_1_bss_114[playerNo]->data;
        MgModeWorkFloat[playerNo] =
            playerData->position.x;
        MgModeWorkFloat[playerNo + 4] =
            playerData->position.y;
        MgModeWorkFloat[playerNo + 8] =
            playerData->position.z;
        playerNo += 1;
    }
}

s32 _CharFXPlay(s16, s32, s32);
s16 fn_1_3F00(void);
void fn_1_6278(OMOBJ *, s16);
s32 fn_1_E378(Point3d *, Point3d *);
void fn_1_E4D0(f32 angleX, f32 angleY, f32 angleZ, Vec source, Vec *destination);
void fn_1_A35C(s16, const Vec *);
s16 fn_1_A42C(void);
/* Updates a character's tournament animation and movement from its scene state. */
void fn_1_8238(OMOBJ *object) {
    TournamentPlayerAnimationWork *playerWork;
    Point3d direction, targetDirection;
    f32 angle;
    s16 value;

    playerWork = object->data;
    switch (playerWork->sceneState) {
        case 0:
            fn_1_6278(object, 1);
            break;
        case 1: {
            fn_1_6278(object, 2);
            PSVECSubtract(&playerWork->targetPosition, &playerWork->position, &direction);
            direction.y = 0.0f;
            fn_1_E378(&direction, &targetDirection);
            direction.x = direction.y = 0.0f;
            direction.z = 1.0f;
            fn_1_E4D0(0.0f, playerWork->rotation.y, 0.0f, direction, &direction);
            angle = PSVECDotProduct(&direction, &targetDirection);
            if (angle >= 0.0f) {
                direction.x = 1.0f;
                direction.y = direction.z = 0.0f;
                fn_1_E4D0(0.0f, playerWork->rotation.y, 0.0f, direction, &direction);
                angle = PSVECDotProduct(&direction, &targetDirection);
                if (angle >= 0.0f) {
                    playerWork->rotation.y += 10.0f;
                } else {
                    playerWork->rotation.y -= 10.0f;
                }
                direction.x = direction.y = 0.0f;
                direction.z = 1.0f;
                fn_1_E4D0(0.0f, playerWork->rotation.y, 0.0f, direction, &direction);
                angle = 180.0 * (atan2(targetDirection.x, targetDirection.z) / 3.141592653589793);
                angle -= 180.0 * (atan2(direction.x, direction.z) / 3.141592653589793);
                if ((s32) angle >= 360) {
                    angle -= 360.0f;
                }
                if ((s32) angle <= -360) {
                    angle += 360.0f;
                }
                if (((s32) angle > -10) && ((s32) angle < 10)) {
                    playerWork->rotation.y =
                        180.0 * (atan2(targetDirection.x, targetDirection.z) / 3.141592653589793);
                    playerWork->sceneState = 2;
                }
            } else {
                direction.x = 1.0f;
                direction.y = direction.z = 0.0f;
                fn_1_E4D0(0.0f, playerWork->rotation.y, 0.0f, direction, &direction);
                angle = PSVECDotProduct(&direction, &targetDirection);
                if (angle >= 0.0f) {
                    playerWork->rotation.y += 10.0f;
                } else {
                    playerWork->rotation.y -= 10.0f;
                }
            }
            break;
        }
        case 2:
            fn_1_6278(object, 3);
            PSVECSubtract(&playerWork->targetPosition, &playerWork->position, &direction);
            if (PSVECSquareMag(&direction) <= (lbl_1_data_544 * lbl_1_data_544)) {
                playerWork->position = playerWork->targetPosition;
                playerWork->sceneState = 3;
            } else {
                fn_1_E378(&direction, &targetDirection);
                PSVECScale(&targetDirection, &direction, lbl_1_data_544);
                targetDirection = playerWork->position;
                PSVECAdd(&direction, &targetDirection, &playerWork->position);
            }
            break;
        case 3: {
            fn_1_6278(object, 2);
            targetDirection.x = targetDirection.y = 0.0f;
            targetDirection.z = 1.0f;
            direction.x = direction.y = 0.0f;
            direction.z = 1.0f;
            fn_1_E4D0(0.0f, playerWork->rotation.y, 0.0f, direction, &direction);
            angle = PSVECDotProduct(&direction, &targetDirection);
            if (angle >= 0.0f) {
                direction.x = 1.0f;
                direction.y = direction.z = 0.0f;
                fn_1_E4D0(0.0f, playerWork->rotation.y, 0.0f, direction, &direction);
                angle = PSVECDotProduct(&direction, &targetDirection);
                if (angle >= 0.0f) {
                    playerWork->rotation.y += 10.0f;
                } else {
                    playerWork->rotation.y -= 10.0f;
                }
                direction.x = direction.y = 0.0f;
                direction.z = 1.0f;
                fn_1_E4D0(0.0f, playerWork->rotation.y, 0.0f, direction, &direction);
                angle = 180.0 * (atan2(targetDirection.x, targetDirection.z) / 3.141592653589793);
                angle -= 180.0 * (atan2(direction.x, direction.z) / 3.141592653589793);
                if ((s32) angle >= 360) {
                    angle -= 360.0f;
                }
                if ((s32) angle <= -360) {
                    angle += 360.0f;
                }
                if (((s32) angle > -10) && ((s32) angle < 10)) {
                    playerWork->rotation.y =
                        180.0 * (atan2(targetDirection.x, targetDirection.z) / 3.141592653589793);
                    playerWork->sceneState = 0;
                }
            } else {
                direction.x = 1.0f;
                direction.y = direction.z = 0.0f;
                fn_1_E4D0(0.0f, playerWork->rotation.y, 0.0f, direction, &direction);
                angle = PSVECDotProduct(&direction, &targetDirection);
                if (angle >= 0.0f) {
                    playerWork->rotation.y += 10.0f;
                } else {
                    playerWork->rotation.y -= 10.0f;
                }
            }
            break;
        }
        case 4: {
            fn_1_6278(object, 2);
            PSVECSubtract(&playerWork->targetPosition, &playerWork->position, &direction);
            direction.y = 0.0f;
            fn_1_E378(&direction, &targetDirection);
            direction.x = direction.y = 0.0f;
            direction.z = 1.0f;
            fn_1_E4D0(0.0f, playerWork->rotation.y, 0.0f, direction, &direction);
            angle = PSVECDotProduct(&direction, &targetDirection);
            if (angle >= 0.0f) {
                direction.x = 1.0f;
                direction.y = direction.z = 0.0f;
                fn_1_E4D0(0.0f, playerWork->rotation.y, 0.0f, direction, &direction);
                angle = PSVECDotProduct(&direction, &targetDirection);
                if (angle >= 0.0f) {
                    playerWork->rotation.y += 10.0f;
                } else {
                    playerWork->rotation.y -= 10.0f;
                }
                direction.x = direction.y = 0.0f;
                direction.z = 1.0f;
    fn_1_E4D0(0.0f, playerWork->rotation.y, 0.0f, direction, &direction);
    angle = 180.0 * (atan2(targetDirection.x, targetDirection.z) / 3.141592653589793);
    angle -= 180.0 * (atan2(direction.x, direction.z) / 3.141592653589793);
                if ((s32) angle >= 360) {
                    angle -= 360.0f;
                }
                if ((s32) angle <= -360) {
                    angle += 360.0f;
                }
                if (((s32) angle > -10) && ((s32) angle < 10)) {
                    playerWork->rotation.y =
                        180.0 * (atan2(targetDirection.x, targetDirection.z) / 3.141592653589793);
                    fn_1_6278(object, 4);
                    _CharFXPlay(playerWork->characterNo, CHARVOICEID(20), 0);
                    playerWork->sceneState = 5;
                    playerWork->velocity.x = playerWork->velocity.y = playerWork->velocity.z = 0.0f;
                    playerWork->velocity.y = 14.0f;
                }
            } else {
    direction.x = 1.0f;
    direction.y = direction.z = 0.0f;
    fn_1_E4D0(0.0f, playerWork->rotation.y, 0.0f, direction, &direction);
                angle = PSVECDotProduct(&direction, &targetDirection);
                if (angle >= 0.0f) {
                    playerWork->rotation.y += 10.0f;
                } else {
                    playerWork->rotation.y -= 10.0f;
                }
            }
            playerWork->storedPosition = playerWork->position;
            object->work[2] = 0;
            break;
        }
        case 5:
            if ((playerWork->motionMode == 4) &&
                (CharMotionShiftIDGet(playerWork->characterNo) < 0)) {
                if ((Hu3DData[*object->mdlId].motWork.speed +
                     CharMotionTimeGet(playerWork->characterNo)) >=
                    CharMotionMaxTimeGet(playerWork->characterNo)) {
                    Hu3DModelAttrSet(*object->mdlId, TOURNAMENT_ALL_MOTION_PAUSE_FLAGS);
                }
            }
            PSVECSubtract(&playerWork->targetPosition, &playerWork->position, &direction);
            if (PSVECSquareMag(&direction) > (lbl_1_data_544 * lbl_1_data_544)) {
                fn_1_E378(&direction, &targetDirection);
                PSVECScale(&targetDirection, &direction, 1.0f + lbl_1_data_544);
    targetDirection = playerWork->position;
                PSVECAdd(&direction, &targetDirection, &playerWork->position);
            } else {
                playerWork->position.x = playerWork->targetPosition.x;
                playerWork->position.z = playerWork->targetPosition.z;
            }
            playerWork->velocity.y -= 0.4f;
            /* Restart the ascent if the jump falls more than 14 world units below its
             * destination. */
            if (playerWork->position.y < (playerWork->targetPosition.y - 14.0f)) {
                playerWork->velocity.y = 14.0f;
            }
            playerWork->position.y += playerWork->velocity.y;
            PSVECSubtract(&playerWork->targetPosition, &playerWork->position, &direction);
            direction.y = 0.0f;
            if ((PSVECMag(&direction) <= 150.0f) && (playerWork->velocity.y < 0.0f) &&
                (playerWork->position.y <= (10.0f + playerWork->targetPosition.y))) {
                object->work[0] = 0U;
                playerWork->position.y = playerWork->targetPosition.y;
                PSVECSubtract(&playerWork->targetPosition, &playerWork->position,
                              &playerWork->velocity);
                PSVECScale(&playerWork->velocity, &playerWork->velocity, 0.1f);
                Hu3DModelAttrReset(*object->mdlId, TOURNAMENT_ALL_MOTION_PAUSE_FLAGS);
    fn_1_6278(object, 5);
                playerWork->sceneState = 6;
            }
            break;
        case 6: {
            if (playerWork->motionMode != 2) {
                if (CharMotionShiftIDGet(playerWork->characterNo) < 0) {
                    if ((Hu3DData[*object->mdlId].motWork.speed +
                         CharMotionTimeGet(playerWork->characterNo)) >=
                        CharMotionMaxTimeGet(playerWork->characterNo)) {
                        fn_1_6278(object, 2);
                    }
                }
            } else {
                targetDirection.x = targetDirection.y = 0.0f;
                targetDirection.z = 1.0f;
                direction.x = direction.y = 0.0f;
                direction.z = 1.0f;
    fn_1_E4D0(0.0f, playerWork->rotation.y, 0.0f, direction, &direction);
                angle = PSVECDotProduct(&direction, &targetDirection);
                if (angle >= 0.0f) {
    direction.x = 1.0f;
    direction.y = direction.z = 0.0f;
    fn_1_E4D0(0.0f, playerWork->rotation.y, 0.0f, direction, &direction);
                    angle = PSVECDotProduct(&direction, &targetDirection);
                    if (angle >= 0.0f) {
                        playerWork->rotation.y += 10.0f;
                    } else {
                        playerWork->rotation.y -= 10.0f;
                    }
                    direction.x = direction.y = 0.0f;
                    direction.z = 1.0f;
    fn_1_E4D0(0.0f, playerWork->rotation.y, 0.0f, direction, &direction);
    angle = 180.0 * (atan2(targetDirection.x, targetDirection.z) / 3.141592653589793);
    angle -= 180.0 * (atan2(direction.x, direction.z) / 3.141592653589793);
                    if ((s32) angle >= 360) {
                        angle -= 360.0f;
                    }
                    if ((s32) angle <= -360) {
                        angle += 360.0f;
                    }
                    if (((s32) angle > -10) && ((s32) angle < 10)) {
                        playerWork->rotation.y =
                            180.0 *
                            (atan2(targetDirection.x, targetDirection.z) / 3.141592653589793);
                        playerWork->sceneState = 0;
                    }
                } else {
    direction.x = 1.0f;
    direction.y = direction.z = 0.0f;
    fn_1_E4D0(0.0f, playerWork->rotation.y, 0.0f, direction, &direction);
                    angle = PSVECDotProduct(&direction, &targetDirection);
                    if (angle >= 0.0f) {
                        playerWork->rotation.y += 10.0f;
                    } else {
                        playerWork->rotation.y -= 10.0f;
                    }
                }
            }
            if (object->work[0] < 10U) {
                playerWork->position.x += playerWork->velocity.x;
                playerWork->position.z += playerWork->velocity.z;
                object->work[0]++;
                if (object->work[0] >= 10U) {
                    playerWork->position = playerWork->targetPosition;
                }
            }
            break;
        }
        case 7:
    fn_1_6278(object, 10);
            if (CharMotionShiftIDGet(playerWork->characterNo) < 0) {
                playerWork->sceneState = 8;
                playerWork->velocity.x = playerWork->velocity.z = 0.0f;
                /* The elimination fall begins with an upward hop before gravity pulls the player
                 * down. */
                playerWork->velocity.y = 10.0f;
            } else if (object->work[0] < 8U) {
                playerWork->rotation.x = 5.0f * object->work[0];
                object->work[0]++;
            }
            break;
        case 8:
            playerWork->velocity.y -= 0.4f;
            playerWork->position.y += playerWork->velocity.y;
            if ((s32) playerWork->position.y <= -500) {
    fn_1_6278(object, 11);
                playerWork->position.y = -500.0f;
                playerWork->sceneState = 9;
                playerWork->velocity.x = playerWork->velocity.y = playerWork->velocity.z = 0.0f;
                playerWork->rotation.x = 0.0f;
            }
            break;
        case 10:
    fn_1_6278(object, 6);
            playerWork->sceneState = 11;
            break;
        case 11:
            if (CharMotionShiftIDGet(playerWork->characterNo) < 0) {
                if ((Hu3DData[*object->mdlId].motWork.speed +
                     CharMotionTimeGet(playerWork->characterNo)) >=
                    CharMotionMaxTimeGet(playerWork->characterNo)) {
                    Hu3DModelAttrSet(*object->mdlId, HU3D_MOTATTR_PAUSE);
                }
            }
            break;
        case 12:
    fn_1_6278(object, 1);
            playerWork->sceneState = 0;
            break;
        case 13:
    fn_1_6278(object, 7);
            playerWork->sceneState = 14;
            break;
        case 14:
            if (CharMotionShiftIDGet(playerWork->characterNo) < 0) {
                if ((Hu3DData[*object->mdlId].motWork.speed +
                     CharMotionTimeGet(playerWork->characterNo)) >=
                    CharMotionMaxTimeGet(playerWork->characterNo)) {
                    Hu3DModelAttrSet(*object->mdlId, HU3D_MOTATTR_PAUSE);
                }
            }
            break;
        case 15:
    fn_1_6278(object, 1);
            playerWork->sceneState = 0;
            break;
        case 16:
    fn_1_6278(object, 8);
            _CharFXPlay(playerWork->characterNo, CHARVOICEID(6), 0);
            playerWork->sceneState = 17;
            break;
        case 17:
            if (CharMotionShiftIDGet(playerWork->characterNo) < 0) {
                if ((Hu3DData[*object->mdlId].motWork.speed +
                     CharMotionTimeGet(playerWork->characterNo)) >=
                    CharMotionMaxTimeGet(playerWork->characterNo)) {
                    Hu3DModelAttrSet(*object->mdlId, HU3D_MOTATTR_PAUSE);
                }
            }
            break;
        case 18:
    fn_1_6278(object, 1);
            playerWork->sceneState = 0;
            break;
        case 19:
    fn_1_6278(object, 9);
            playerWork->sceneState = 20;
            break;
        case 20:
            if (CharMotionShiftIDGet(playerWork->characterNo) < 0) {
                if ((Hu3DData[*object->mdlId].motWork.speed +
                     CharMotionTimeGet(playerWork->characterNo)) >=
                    CharMotionMaxTimeGet(playerWork->characterNo)) {
                    Hu3DModelAttrSet(*object->mdlId, HU3D_MOTATTR_PAUSE);
                }
            }
            break;
        case 21:
    fn_1_6278(object, 1);
            playerWork->sceneState = 0;
            break;
        case 22:
            if (playerWork->motionMode != 2) {
    fn_1_6278(object, 2);
                playerWork->rotation.x = playerWork->rotation.y = playerWork->rotation.z = 0.0f;
            } else {
                playerWork->rotation.y += 10.0f;
                if (180.0f <= playerWork->rotation.y) {
    fn_1_6278(object, 12);
                    playerWork->sceneState = 23;
                    CharFXPlay(playerWork->characterNo, CHARVOICEID(4));
                }
            }
            break;
        case 23:
            /* Attach the center model at the frame-15 threshold. State 24 counts as ready while the
             * character animation and the attached model's movement continue. */
            if ((CharMotionShiftIDGet(playerWork->characterNo) < 0) &&
                ((0.1f + CharMotionTimeGet(playerWork->characterNo)) >= 15.0f)) {
                value = fn_1_3F00();
                Hu3DModelPosSet(value, 0.0f, 0.0f, 0.0f);
                Hu3DModelRotSet(value, 0.0f, 180.0f, 0.0f);
                Hu3DModelHookSet(*object->mdlId, lbl_1_data_690, value);
                playerWork->sceneState = 24;
            }
            break;
        case 24:
            if (CharMotionShiftIDGet(playerWork->characterNo) < 0) {
                if ((1.0f + CharMotionTimeGet(playerWork->characterNo)) >=
                    CharMotionMaxTimeGet(playerWork->characterNo)) {
                    Hu3DModelAttrSet(*object->mdlId, TOURNAMENT_ALL_MOTION_PAUSE_FLAGS);
                }
            }
            value = fn_1_3F00();
            Hu3DModelPosGet(value, &direction);
            Hu3DModelRotGet(value, &targetDirection);
            if (direction.y < 50.0f) {
                direction.y += 5.0f;
            }
            if (direction.y >= 50.0f) {
                direction.y = 50.0f;
            }
            if (targetDirection.y < 360.0f) {
                targetDirection.y += 6.0f;
            }
            if (targetDirection.y >= 360.0f) {
                targetDirection.y = 360.0f;
            }
            Hu3DModelPosSetV(value, &direction);
            Hu3DModelRotSetV(value, &targetDirection);
            break;
        case 25:
    fn_1_6278(object, 13);
            playerWork->rotation.y = 0.0f;
            playerWork->sceneState = 26;
            Hu3D3Dto2D(&playerWork->position, 1, &direction);
    value = (s32) (64.0f * (direction.x / 576.0f)) + 32;
            if (value < 32) {
                value = 32;
            }
            if (value > 96) {
                value = 96;
            }
            CharFXPlayPan(playerWork->characterNo, CHARVOICEID(6), value);
            break;
        case 26:
            if (object->work[1] != 0) {
                value = fn_1_A42C();
                if (((value == 0) || (value == 1)) &&
                    ((MgModeWorkInt[playerWork->playerNo] == 0) ||
                     (MgModeWorkInt[playerWork->playerNo] == 2)) &&
                    (MgModeWorkInt[playerWork->playerNo + 4] == 2)) {
                    playerWork->rotation.y += 3.0f;
                    if (playerWork->rotation.y > 45.0f) {
                        playerWork->rotation.y = 45.0f;
                    }
                } else if (((value == 0) || (value == 1)) &&
                           ((MgModeWorkInt[playerWork->playerNo] == 1) ||
                            (MgModeWorkInt[playerWork->playerNo] == 3)) &&
                           (MgModeWorkInt[playerWork->playerNo + 4] == 2)) {
                    playerWork->rotation.y -= 3.0f;
                    if (playerWork->rotation.y < -45.0f) {
                        playerWork->rotation.y = -45.0f;
                    }
                } else if ((value == 2) &&
                           ((MgModeWorkInt[playerWork->playerNo] == 0) ||
                            (MgModeWorkInt[playerWork->playerNo] == 1)) &&
                           (MgModeWorkInt[playerWork->playerNo + 4] == 2)) {
                    playerWork->rotation.y += 3.0f;
                    if (playerWork->rotation.y > 45.0f) {
                        playerWork->rotation.y = 45.0f;
                    }
                } else if ((value == 2) &&
                           ((MgModeWorkInt[playerWork->playerNo] == 2) ||
                            (MgModeWorkInt[playerWork->playerNo] == 3)) &&
                           (MgModeWorkInt[playerWork->playerNo + 4] == 2)) {
                    playerWork->rotation.y -= 3.0f;
                    if (playerWork->rotation.y < -45.0f) {
                        playerWork->rotation.y = -45.0f;
                    }
                } else if ((value == 3) &&
                           ((MgModeWorkInt[playerWork->playerNo] == 0) ||
                            (MgModeWorkInt[playerWork->playerNo] == 1)) &&
                           (MgModeWorkInt[playerWork->playerNo + 4] == 1)) {
                    playerWork->rotation.y += 3.0f;
                    if (playerWork->rotation.y > 45.0f) {
                        playerWork->rotation.y = 45.0f;
                    }
                } else if ((value == 3) &&
                           ((MgModeWorkInt[playerWork->playerNo] == 2) ||
                            (MgModeWorkInt[playerWork->playerNo] == 3)) &&
                           (MgModeWorkInt[playerWork->playerNo + 4] == 1)) {
                    playerWork->rotation.y -= 3.0f;
                    if (playerWork->rotation.y < -45.0f) {
                        playerWork->rotation.y = -45.0f;
                    }
                }
            }
            break;
        case 27:
    fn_1_6278(object, 1);
            playerWork->sceneState = 0;
            break;
        }
    Hu3DModelRotSetV(*object->mdlId, &playerWork->rotation);
    Hu3DModelPosSetV(*object->mdlId, &playerWork->position);
    fn_1_A35C(playerWork->playerNo, &playerWork->position);
}

HU3D_PARMAN_PARAM lbl_1_data_710 = {
    50,
    0,
    2.0f,
    4e+01f,
    45.0f,
    { 0.0f, -0.05f, 0.0f },
    2.0f,
    1.01f,
    4e+01f,
    0.99f,
    2,
    { { 255, 255, 255, 255 }, { 255, 255, 64, 255 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
    { { 255, 128, 128, 16 }, { 255, 64, 32, 16 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } }
};

Vec lbl_1_data_760[4] = {
    {-1.2e+02f, -1.55e+03f, 2.5e+02f},
    {-6e+01f, -1.55e+03f, 2.5e+02f},
    {6e+01f, -1.55e+03f, 2.5e+02f},
    {1.2e+02f, -1.55e+03f, 2.5e+02f}
};

Vec lbl_1_data_790[4] = {
    {-6e+02f, -5e+02f, -1.5e+02f},
    {-2e+02f, -5e+02f, -1.5e+02f},
    {2e+02f, -5e+02f, -1.5e+02f},
    {6e+02f, -5e+02f, -1.5e+02f}
};

Vec lbl_1_data_7C0[4] = {
    {-6e+02f, -5e+02f, 3e+02f},
    {-2e+02f, -5e+02f, 3e+02f},
    {2e+02f, -5e+02f, 3e+02f},
    {6e+02f, -5e+02f, 3e+02f}
};

Vec lbl_1_data_7F0[4] = {
    {-6e+02f, 0.0f, 0.0f},
    {-2e+02f, 0.0f, 0.0f},
    {2e+02f, 0.0f, 0.0f},
    {6e+02f, 0.0f, 0.0f}
};

Vec lbl_1_data_820[2] = {
    {-3e+02f, 5e+02f, -4.5e+02f},
    {3e+02f, 5e+02f, -4.5e+02f}
};

Vec lbl_1_data_838 = {0.0f, 1e+03f, -975.0f};

/* Creates each player's result particles during character setup, called by fn_1_74BC. */
void fn_1_A1F4(void) {
    s16 playerIndex;

    playerIndex = 0;
    while (playerIndex < 4) {
        lbl_1_bss_10C[playerIndex] = Hu3DParManCreate(
            HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_effect, 1), HU_MEMNUM_OVL, HEAP_MODEL)),
            100, &lbl_1_data_710);
        Hu3DParManAttrSet(lbl_1_bss_10C[playerIndex],
                          HU3D_PARMAN_ATTR_RANDSPEED70 | HU3D_PARMAN_ATTR_RANDSCALE70 |
                              HU3D_PARMAN_ATTR_SCALEJITTER);
        Hu3DParticleBlendModeSet(
            Hu3DParManModelIDGet(lbl_1_bss_10C[playerIndex]), 1U);
        Hu3DModelCameraSet(
            Hu3DParManModelIDGet(lbl_1_bss_10C[playerIndex]), 1U);
        Hu3DModelLayerSet(
            Hu3DParManModelIDGet(lbl_1_bss_10C[playerIndex]), 4);
        Hu3DParManRotSet(lbl_1_bss_10C[playerIndex], (90.0f), (0.0f),
                         (0.0f));
        Hu3DParManAttrSet(lbl_1_bss_10C[playerIndex], 1);
        playerIndex += 1;
    }
}

/* Moves one player's particle effect to that character's world position. */
void fn_1_A35C(s16 playerNo, const Vec *position) {
    Hu3DParManPosSet(lbl_1_bss_10C[playerNo],
                     position->x, position->y, position->z);
}

/* Enables or stops new particles for one player during result scenes; already emitted
 * particles continue to animate. */
void fn_1_A3B0(s16 playerNo, s32 showEffect) {
    s32 unused;
    if (0 != showEffect)
    {
        Hu3DParManAttrReset(lbl_1_bss_10C[playerNo], 1);
        return;
    }
    Hu3DParManAttrSet(lbl_1_bss_10C[playerNo], 1);
}

/* Classifies which tournament side split or crossover the current round has. */
s16 fn_1_A42C(void)
{
    s16 player;
    s16 firstPlayer;
    s16 secondPlayer;

    player = 0;
    while (player < 4) {
        if (MgModeWorkInt[player] == 0) {
            firstPlayer = player;
        }
        if (MgModeWorkInt[player] == 1) {
            secondPlayer = player;
        }
        player++;
    }
    if (MgModeWorkInt[firstPlayer + 4] ==
        MgModeWorkInt[secondPlayer + 4]) {
        return 0;
    }

    player = 0;
    while (player < 4) {
        if (MgModeWorkInt[player] == 2) {
            firstPlayer = player;
        }
        if (MgModeWorkInt[player] == 3) {
            secondPlayer = player;
        }
        player++;
    }
    if (MgModeWorkInt[firstPlayer + 4] ==
        MgModeWorkInt[secondPlayer + 4]) {
        return 1;
    }

    firstPlayer = secondPlayer = -1;
    player = 0;
    while (player < 4) {
        if (((MgModeWorkInt[player] == 0) ||
             (MgModeWorkInt[player] == 1)) &&
            (MgModeWorkInt[player + 4] == 2)) {
            firstPlayer = player;
        }
        if (((MgModeWorkInt[player] == 2) ||
             (MgModeWorkInt[player] == 3)) &&
            (MgModeWorkInt[player + 4] == 2)) {
            secondPlayer = player;
        }
        player++;
    }
    if ((firstPlayer >= 0) && (secondPlayer >= 0)) {
        return 2;
    }
    return 3;
}

extern s16 lbl_1_bss_D8;

void fn_1_AA04(void);
void fn_1_3394(s16, s16);

/* Moves players into the opening lineup, then shuffles them for the next round. */
void fn_1_A6B4(void)
{
    f32 previousMovementSpeed;
    Vec playerOnePosition;
    Vec playerTwoPosition;
    Vec playerThreePosition;
    Vec playerFourPosition;
    Vec modePosition;
    s16 waiting;
    s16 playerIndex;
    s16 stateReady;
    TournamentPlayerAnimationWork *firstPlayerMotion;
    TournamentPlayerAnimationWork *secondPlayerMotion;
    TournamentPlayerAnimationWork *thirdPlayerMotion;
    TournamentPlayerAnimationWork *fourthPlayerMotion;
    TournamentPlayerAnimationWork *pollMotion;
    TournamentPlayerAnimationWork *modeMotion;

    previousMovementSpeed = lbl_1_data_544;
    lbl_1_data_544 = (12.0f);

    playerOnePosition = lbl_1_data_760[1];
    firstPlayerMotion = lbl_1_bss_114[1]->data;
    firstPlayerMotion->sceneState = 1;
    firstPlayerMotion->targetPosition = playerOnePosition;

    playerTwoPosition = lbl_1_data_760[2];
    secondPlayerMotion = lbl_1_bss_114[2]->data;
    secondPlayerMotion->sceneState = 1;
    secondPlayerMotion->targetPosition = playerTwoPosition;

    HuPrcSleep(30);

    playerThreePosition = lbl_1_data_760[0];
    thirdPlayerMotion = lbl_1_bss_114[0]->data;
    thirdPlayerMotion->sceneState = 1;
    thirdPlayerMotion->targetPosition = playerThreePosition;

    playerFourPosition = lbl_1_data_760[3];
    fourthPlayerMotion = lbl_1_bss_114[3]->data;
    fourthPlayerMotion->sceneState = 1;
    fourthPlayerMotion->targetPosition = playerFourPosition;

    waiting = 1;
    while (waiting != 0) {
        waiting = 0;
        playerIndex = 0;
        while (playerIndex < 4) {
            pollMotion = lbl_1_bss_114[playerIndex]->data;
            if ((pollMotion->sceneState == 0) || (pollMotion->sceneState == 9) ||
                (pollMotion->sceneState == 24)) {
                stateReady = 1;
            } else {
                stateReady = 0;
            }
            if (stateReady == 0) {
                waiting = 1;
            }
            playerIndex++;
        }
        HuPrcVSleep();
    }

    lbl_1_data_544 = previousMovementSpeed;
    fn_1_AA04();
    playerIndex = 0;
    while (playerIndex < 4) {
        modePosition = lbl_1_data_790[MgModeWorkInt[playerIndex]];
        modeMotion = lbl_1_bss_114[playerIndex]->data;
        modeMotion->position = modePosition;
        modeMotion->targetPosition = modePosition;
        playerIndex++;
    }

    /* Suppress the highlight cue while enabling all four starting markers. */
    lbl_1_bss_D8 = 1;
    fn_1_3394(0, 1);
    fn_1_3394(1, 1);
    fn_1_3394(2, 1);
    fn_1_3394(3, 1);
    fn_1_3394(4, 0);
    fn_1_3394(5, 0);
    fn_1_3394(6, 0);
}

/* Shuffles the four player-side assignments used for a tournament round. */
void fn_1_AA04(void)
{
    s16 i;
    s16 j;
    s32 value;

    for (i = 0; i < 4; i++) {
        MgModeWorkInt[i + 4] = 2;
    }
    for (i = 0; i < 4; i++) {
        MgModeWorkInt[i] = i;
    }
    for (i = 0; i < 4; i++) {
        for (j = i + 1; j < 4; j++) {
            if (frandmod(100) < 50) {
                value = MgModeWorkInt[i];
                MgModeWorkInt[i] = MgModeWorkInt[j];
                MgModeWorkInt[j] = value;
            }
        }
    }
}

void omVibrate(s16 player, s16 time, s16 on, s16 off);

/* Brings each player forward in turn and waits for its scene animation. */
void fn_1_AB48(void)
{
    s16 mode;
    s16 playerIndex;
    s16 selectedPlayer;
    TournamentPlayerAnimationWork *motion;
    TournamentPlayerAnimationWork *motionState;
    s16 stateReady;
    TournamentPlayerAnimationWork *pollMotion;
    TournamentPlayerAnimationWork *finalMotion;
    s16 playerType;
    Vec position;

    mode = 0;
    while (mode < 4) {
        playerIndex = 0;
        while (playerIndex < 4) {
            if (mode == MgModeWorkInt[playerIndex]) {
                selectedPlayer = playerIndex;
                break;
            }
            playerIndex++;
        }
        position = lbl_1_data_7C0[MgModeWorkInt[selectedPlayer]];
        motion = lbl_1_bss_114[selectedPlayer]->data;
        motion->sceneState = 1;
        motion->targetPosition = position;
        motionState = lbl_1_bss_114[selectedPlayer]->data;
        playerType = motionState->playerType;
        if (playerType == 0) {
            omVibrate(selectedPlayer, 20, 4, 4);
        }
        playerIndex = 1;
        while (playerIndex != 0) {
            playerIndex = 0;
            pollMotion = lbl_1_bss_114[selectedPlayer]->data;
            if ((pollMotion->sceneState == 0) ||
                (pollMotion->sceneState == 9) ||
                (pollMotion->sceneState == 24)) {
                stateReady = 1;
            } else {
                stateReady = 0;
            }
            if (stateReady == 0) {
                playerIndex = 1;
            }
            HuPrcVSleep();
        }
        finalMotion = lbl_1_bss_114[selectedPlayer]->data;
        lbl_1_bss_114[selectedPlayer]->work[1] = 0;
        finalMotion->sceneState = 25;
        mode++;
    }
    HuPrcSleep(76);
}

void fn_1_3394(s16, s16);

/* When fn_1_AD50 resets the bracket, send any listed result pose to its return-to-idle state,
 * even if that pose has not finished. */
static inline void tournamentSequenceNormalize(s16 playerIndex)
{
    TournamentPlayerAnimationWork *motion;

    motion = lbl_1_bss_114[playerIndex]->data;
    if ((motion->sceneState == 10) || (motion->sceneState == 11)) {
        motion->sceneState = 12;
    }
    if ((motion->sceneState == 13) || (motion->sceneState == 14)) {
        motion->sceneState = 15;
    }
    if ((motion->sceneState == 16) || (motion->sceneState == 17)) {
        motion->sceneState = 18;
    }
    if ((motion->sceneState == 19) || (motion->sceneState == 20)) {
        motion->sceneState = 21;
    }
    if ((motion->sceneState == 25) || (motion->sceneState == 26)) {
        motion->sceneState = 27;
    }
}

/* Returns the players to their starting bracket positions, then pauses and rewinds all seven
 * markers. */
void fn_1_AD50(void)
{
    s16 playerIndex;
    s16 modeIndex;
    s16 mode;
    Vec position;
    TournamentPlayerAnimationWork *motion;

    playerIndex = 0;
    while (playerIndex < 4) {
        tournamentSequenceNormalize(playerIndex);
        playerIndex++;
    }
    HuPrcSleep(10);
    mode = 0;
    while (mode < 4) {
        modeIndex = 0;
        while (modeIndex < 4) {
            if (mode == MgModeWorkInt[modeIndex]) {
                playerIndex = modeIndex;
                break;
            }
            modeIndex++;
        }
        position = lbl_1_data_7F0[MgModeWorkInt[playerIndex]];
        motion = lbl_1_bss_114[playerIndex]->data;
        motion->sceneState = 4;
        motion->targetPosition = position;
        mode++;
    }
    modeIndex = 1;
    while (modeIndex != 0) {
        modeIndex = 0;
        playerIndex = 0;
        while (playerIndex < 4) {
            {
                TournamentPlayerAnimationWork *pollMotion;
                s16 stateReady;

                pollMotion = lbl_1_bss_114[playerIndex]->data;
                if ((pollMotion->sceneState == 0) ||
                    (pollMotion->sceneState == 9) ||
                    (pollMotion->sceneState == 24)) {
                    stateReady = 1;
                } else {
                    stateReady = 0;
                }
                if (stateReady == 0) {
                    modeIndex = 1;
                }
            }
            playerIndex++;
        }
        HuPrcVSleep();
    }
    fn_1_3394(0, 0);
    fn_1_3394(1, 0);
    fn_1_3394(2, 0);
    fn_1_3394(3, 0);
    fn_1_3394(4, 0);
    fn_1_3394(5, 0);
    fn_1_3394(6, 0);
}

void fn_1_143C(s16, s16);
s16 fn_1_A42C(void);

/* Before the consolation matchup, moves any remaining player in an outer bracket slot inward.
 * This repositioning happens only once per tournament. */
void fn_1_B014(void)
{
    s16 waiting;
    s16 stateReady;
    s16 modeResult;
    TournamentPlayerAnimationWork *firstPlayerMotion;
    TournamentPlayerAnimationWork *secondPlayerMotion;
    TournamentPlayerAnimationWork *pollMotion;
    Vec firstPlayerPosition;
    Vec secondPlayerPosition;

    modeResult = fn_1_A42C();
    if (modeResult == 2) {
        fn_1_3394(0, 0);
        fn_1_3394(1, 1);
        fn_1_3394(2, 1);
        fn_1_3394(3, 0);
        fn_1_3394(4, 0);
        fn_1_3394(5, 0);
        fn_1_3394(6, 0);
    }
    modeResult = fn_1_A42C();
    if ((modeResult == 2) && (MgModeWorkInt[12] == 0)) {
        fn_1_143C(2, 60);
        HuPrcSleep(60);
        modeResult = 0;
        while (modeResult < 4) {
            if ((MgModeWorkInt[modeResult] == 0) && (MgModeWorkInt[modeResult + 4] == 2)) {
                firstPlayerPosition = lbl_1_data_7F0[1];
                firstPlayerMotion = lbl_1_bss_114[modeResult]->data;
                firstPlayerMotion->sceneState = 4;
                firstPlayerMotion->targetPosition = firstPlayerPosition;
            }
            if ((MgModeWorkInt[modeResult] == 3) && (MgModeWorkInt[modeResult + 4] == 2)) {
                secondPlayerPosition = lbl_1_data_7F0[2];
                secondPlayerMotion = lbl_1_bss_114[modeResult]->data;
                secondPlayerMotion->sceneState = 4;
                secondPlayerMotion->targetPosition = secondPlayerPosition;
            }
            modeResult++;
        }
        HuPrcVSleep();
        waiting = 1;
        while (waiting != 0) {
            waiting = 0;
            modeResult = 0;
            while (modeResult < 4) {
                pollMotion = lbl_1_bss_114[modeResult]->data;
                if ((pollMotion->sceneState == 0) || (pollMotion->sceneState == 9) ||
                    (pollMotion->sceneState == 24)) {
                    stateReady = 1;
                } else {
                    stateReady = 0;
                }
                if (stateReady == 0) {
                    waiting = 1;
                }
                modeResult++;
            }
            HuPrcVSleep();
        }
        MgModeWorkInt[12] = 1;
    }
}

s16 fn_1_A42C(void);

/* Marks players on the active tournament sides for the next scene callback. */
void fn_1_B2E0(void)
{
    s16 stageOrPlayer;

    stageOrPlayer = fn_1_A42C();
    if (stageOrPlayer == 0) {
        stageOrPlayer = 0;
        while (stageOrPlayer < 4) {
            if ((MgModeWorkInt[stageOrPlayer] == 0) || (MgModeWorkInt[stageOrPlayer] == 1)) {
                TournamentPlayerAnimationWork *playerSequence;

                playerSequence = lbl_1_bss_114[stageOrPlayer]->data;
                lbl_1_bss_114[stageOrPlayer]->work[1] = 1;
                playerSequence->sceneState = 25;
            }
            stageOrPlayer++;
        }
    } else if (stageOrPlayer == 1) {
        stageOrPlayer = 0;
        while (stageOrPlayer < 4) {
            if ((MgModeWorkInt[stageOrPlayer] == 2) || (MgModeWorkInt[stageOrPlayer] == 3)) {
                TournamentPlayerAnimationWork *playerSequence;

                playerSequence = lbl_1_bss_114[stageOrPlayer]->data;
                lbl_1_bss_114[stageOrPlayer]->work[1] = 1;
                playerSequence->sceneState = 25;
            }
            stageOrPlayer++;
        }
    } else if (stageOrPlayer == 2) {
        stageOrPlayer = 0;
        while (stageOrPlayer < 4) {
            if (MgModeWorkInt[stageOrPlayer + 4] == 2) {
                TournamentPlayerAnimationWork *playerSequence;

                playerSequence = lbl_1_bss_114[stageOrPlayer]->data;
                lbl_1_bss_114[stageOrPlayer]->work[1] = 1;
                playerSequence->sceneState = 25;
            }
            stageOrPlayer++;
        }
    } else {
        stageOrPlayer = 0;
        while (stageOrPlayer < 4) {
            if (MgModeWorkInt[stageOrPlayer + 4] == 1) {
                TournamentPlayerAnimationWork *playerSequence;

                playerSequence = lbl_1_bss_114[stageOrPlayer]->data;
                lbl_1_bss_114[stageOrPlayer]->work[1] = 1;
                playerSequence->sceneState = 25;
            }
            stageOrPlayer++;
        }
    }
}

HU3D_MODELID fn_1_0(s16, s16, HuVecF *);

/* Creates the framed minigame preview using game index zero, hides it at zero scale,
 * and prepares the still-image cycle for the tournament reveal. */
void fn_1_B57C(void) {
    HuVecF position;
    u8 gameNo;

    position.x = (0.0f);
    position.y = (10.0f);
    position.z = (-2000.0f);
    gameNo = 0;
    lbl_1_bss_10A = fn_1_0(gameNo, 0, &position);
    Hu3DModelScaleSet(lbl_1_bss_10A, (0.0f), (0.0f), (0.0f));
    fn_1_B628();
}

extern u8 MgModeWork[256];
void fn_1_2A4(s16 count, u8 *gameOrder);

/* Builds and shuffles minigame indices for the preview's still-image cycle, loading at most five
 * image pairs. The initial prefix count is computed but unused. */
void fn_1_B628(void)
{
    u8 gameOrder[96];
    s16 scannedCount;
    s16 gameIndex;
    s16 otherGameIndex;
    u8 swappedGame;

    lbl_1_bss_108 = 1;
    /* Unused entries stop the later shuffle and selection scans. */
    memset(gameOrder, TOURNAMENT_ORDER_UNUSED, sizeof(gameOrder));

    scannedCount = 0;
    gameIndex = 0;
    while ((gameIndex < 256) && (MgModeWork[gameIndex] != TOURNAMENT_ORDER_UNUSED)) {
        scannedCount++;
        gameIndex++;
    }
    if (gameIndex >= 256) {
        scannedCount = 1;
    }

    gameIndex = 255;
    otherGameIndex = 0;
    for (; gameIndex >= 0; gameIndex--) {
        if (MgModeWork[gameIndex] != TOURNAMENT_ORDER_UNUSED) {
            gameOrder[otherGameIndex] = MgModeWork[gameIndex];
            otherGameIndex++;
        } else {
            gameOrder[otherGameIndex] = TOURNAMENT_ORDER_UNUSED;
        }
    }

    for (gameIndex = 0;
         (gameIndex < 96) && (gameOrder[gameIndex] != TOURNAMENT_ORDER_UNUSED);
         gameIndex++) {
        for (otherGameIndex = gameIndex + 1;
             (otherGameIndex < 96) && (gameOrder[otherGameIndex] != TOURNAMENT_ORDER_UNUSED);
             otherGameIndex++) {
            if (frandmod(100) < 50) {
                swappedGame = gameOrder[gameIndex];
                gameOrder[gameIndex] = gameOrder[otherGameIndex];
                gameOrder[otherGameIndex] = swappedGame;
            }
        }
    }

    HuPrcVSleep();
    if (otherGameIndex >= 5) {
        fn_1_2A4(5, gameOrder);
    } else {
        fn_1_2A4(otherGameIndex, gameOrder);
    }
    HuPrcVSleep();
}

/* Stops the active tournament selection effect when the board leaves it. */
void fn_1_B80C(void)
{
    if (lbl_1_bss_108 != 0) {
        fn_1_4F8();
    }
    lbl_1_bss_108 = 0;
}

s32 fn_1_43C();                                 /* Cycles the preview still image. */
void fn_1_4F8();                                 /* Releases the preview cycle images. */
s32 fn_1_5E0(u8, s16);                          /* Replaces the selected preview still. */
s32 fn_1_6F4();                                 /* Starts the selected preview movie. */
extern u8 MgModePlayNum;

extern s32 lbl_1_data_850;

/* Reveals the framed preview in scene submode 5 and cycles its still images, then chooses
 * day or night and starts the already selected minigame's movie. */
void fn_1_B850(void) {
    Point3d cameraRelativePosition;
    f32 scale;
    s16 appearanceWait;

    cameraRelativePosition.x = (0.0f);
    cameraRelativePosition.y = (10.0f);
    cameraRelativePosition.z = (-2000.0f);
    /* Place the prop just above and in front of the active camera. */
    PSVECAdd(&Hu3DCamera->pos, &cameraRelativePosition, &cameraRelativePosition);
    Hu3DModelPosSetV(lbl_1_bss_10A, &cameraRelativePosition);
    scale = (0.0f);
    Hu3DModelScaleSet(lbl_1_bss_10A, scale, scale, scale);
    HuPrcVSleep();
    fn_1_43C();
    HuPrcVSleep();
    HuAudFXPlay(MSM_SE_MGTOUR_PROP_REVEAL);
    while (scale < (1.0f)) {
        Hu3DModelScaleSet(lbl_1_bss_10A, scale, scale, scale);
        scale += (0.10000000149011612f);
        if (scale >= (1.0f)) {
            scale = (1.0f);
        }
        HuPrcVSleep();
    }
    Hu3DModelScaleSet(lbl_1_bss_10A, (1.0f), (1.0f), (1.0f));
    HuPrcVSleep();
    lbl_1_data_850 = HuAudFXPlay(MSM_SE_MGTOUR_PROP_LAND);
    /* Keep updating the prop for 25 four-tick waits after its entrance sound. */
    appearanceWait = 1;
    while (appearanceWait != 0) {
        fn_1_43C();
        HuPrcSleep(4);
        appearanceWait += 1;
        if (appearanceWait > 25) {
            appearanceWait = 0;
        }
    }
    if (lbl_1_bss_108 != 0) {
        fn_1_4F8();
    }
    lbl_1_bss_108 = 0;
    if ((s32) lbl_1_data_850 >= 0) {
        HuAudFXStop(lbl_1_data_850);
    }
    lbl_1_data_850 = -1;
    HuAudFXPlay(MSM_SE_MGTOUR_PROP_DECIDE);
    if (frandmod(2) != 0) {
        lbl_1_bss_128 = 1;
    } else {
        lbl_1_bss_128 = 0;
    }
    fn_1_5E0(MgModeWork[MgModePlayNum], lbl_1_bss_128);
    fn_1_6F4();
    HuPrcVSleep();
}

s16 fn_1_A42C(void);
void fn_1_143C(s16 cameraMode, s16 duration);
void fn_1_2728(s16 player);
void fn_1_3394(s16 model, s16 mode);
void fn_1_34B0(s16 model);
void fn_1_3F18(s16 mode);
void fn_1_56B4(s16 mode);
void omVibrate(s16 player, s16 time, s16 on, s16 off);

void fn_1_7DA8(s16 player, s16 flag);

void fn_1_7EA8(s16 player);

/* Starts the selected player's win or loss pose in the result sequence. */
static inline void tournamentResultStateSet(s16 player, s16 flag)
{
    TournamentPlayerAnimationWork *playerData;

    playerData = lbl_1_bss_114[player]->data;
    if (flag != 0) {
        playerData->sceneState = 16;
        return;
    }
    playerData->sceneState = 19;
}

/* After a round, send any listed result pose to its return-to-idle state,
 * even if that pose has not finished. */
static inline void tournamentResultSequenceAdvance(s16 player)
{
    TournamentPlayerAnimationWork *playerData;

    playerData = lbl_1_bss_114[player]->data;
    if ((playerData->sceneState == 10) || (playerData->sceneState == 11)) {
        playerData->sceneState = 12;
    }
    if ((playerData->sceneState == 13) || (playerData->sceneState == 14)) {
        playerData->sceneState = 15;
    }
    if ((playerData->sceneState == 16) || (playerData->sceneState == 17)) {
        playerData->sceneState = 18;
    }
    if ((playerData->sceneState == 19) || (playerData->sceneState == 20)) {
        playerData->sceneState = 21;
    }
    if ((playerData->sceneState == 25) || (playerData->sceneState == 26)) {
        playerData->sceneState = 27;
    }
}

/* Starts the losing player's elimination fall to y = -500 unless it is already falling
 * or settled. */
static inline void tournamentResultSceneStart(s16 player)
{
    TournamentPlayerAnimationWork *playerData;

    playerData = lbl_1_bss_114[player]->data;
    if ((playerData->sceneState != 7) && (playerData->sceneState != 8) &&
        (playerData->sceneState != 9)) {
        playerData->sceneState = 7;
        lbl_1_bss_114[player]->work[0] = 0;
    }
}

/* Reports readiness in the idle, settled, or prop-attached state. The prop-attached state
 * counts as ready before its animation and prop movement finish. */
static inline s16 tournamentResultSceneDone(s16 player)
{
    TournamentPlayerAnimationWork *playerData;

    playerData = lbl_1_bss_114[player]->data;
    if ((playerData->sceneState == 0) || (playerData->sceneState == 9) ||
        (playerData->sceneState == 24)) {
        return 1;
    }
    return 0;
}

/* Gets the character number for the selected tournament player. */
static inline s16 tournamentPlayerCharacterGet(s16 player)
{
    TournamentPlayerAnimationWork *playerData;
    s16 characterNo;

    playerData = lbl_1_bss_114[player]->data;
    characterNo = playerData->characterNo;
    return characterNo;
}

/* Queues the player's turn and jump toward the supplied result-scene destination. */
static inline void tournamentResultPositionSet(s16 player, Vec position)
{
    TournamentPlayerAnimationWork *playerData;

    playerData = lbl_1_bss_114[player]->data;
    playerData->sceneState = 4;
    playerData->targetPosition = position;
}

/* Starts the player's elimination fall when the result scene requests it. */
void fn_1_8018(s16 player);

/* Tests scene readiness; the prop-attached state counts as ready before its animation
* and prop movement finish. */
s16 fn_1_80CC(s16 player);

/* Requests rumble for player type zero during the tournament result scene. */
static inline void tournamentPlayerVibrate(s16 player, s16 time, s16 on, s16 off)
{
    TournamentPlayerAnimationWork *playerData;
    s16 playerType;

    playerData = lbl_1_bss_114[player]->data;
    playerType = playerData->playerType;
    if (playerType == 0) {
        omVibrate(player, time, on, off);
    }
}

/* Resolves the current match winner, updates round state, and plays its result. */
/* In scene mode 10, exactly one active player with a bonus of at least ten coins wins.
* Bonuses are normalized to zero or ten. Returns -1 to retry, zero to continue rounds,
* or one to begin the champion presentation. */
s16 fn_1_BAD4(void)
{
    s16 i;
    s16 count;
    s16 winner;
    s16 loser;
    s16 result;

    result = 0;
    i = fn_1_A42C();
    if (i == 0) {
        for (i = 0, count = 0; i < 4; i++) {
            if ((MgModeWorkInt[i] == 0) || (MgModeWorkInt[i] == 1)) {
                if (MgModeWorkInt[i + 8] >= 10) {
                    MgModeWorkInt[i + 8] = 10;
                    winner = i;
                    count++;
                } else {
                    MgModeWorkInt[i + 8] = 0;
                    loser = i;
                }
            } else {
                MgModeWorkInt[i + 8] = 0;
            }
        }
        if (count != 1) {
            fn_1_2728(-1);
            MgModeWorkInt[8] = MgModeWorkInt[9] = MgModeWorkInt[10] = MgModeWorkInt[11] = 0;
            for (i = 0, count = 0; i < 4; i++) {
                if ((MgModeWorkInt[i] == 0) || (MgModeWorkInt[i] == 1)) {
                    tournamentResultStateSet(i, 0);
                }
            }
            result = -1;
        } else {
            fn_1_3394(0, 0);
            fn_1_3394(1, 0);
            fn_1_3394(2, 0);
            fn_1_3394(3, 0);
            fn_1_3394(4, 1);
            fn_1_3394(5, 0);
            fn_1_3394(6, 0);
            fn_1_2728(winner);
            fn_1_143C(4, 30);
            HuPrcSleep(30);
            HuAudFXPlay(MSM_SE_MGTOUR_ROUND_WIN);
            tournamentResultPositionSet(winner, lbl_1_data_820[0]);
            Hu3DParManAttrReset(lbl_1_bss_10C[winner], 1);
            HuPrcVSleep();
            count = 1;
            while (count != 0) {
                count = 0;
                for (i = 0; i < 4; i++) {
                    if (tournamentResultSceneDone(i) == 0) {
                        count = 1;
                    }
                }
                HuPrcVSleep();
            }
            Hu3DParManAttrSet(lbl_1_bss_10C[winner], 1);
            fn_1_56B4(1);
            tournamentResultStateSet(winner, 1);
            tournamentResultStateSet(loser, 0);
            MgModeWorkInt[winner + 4] = 1;
        }
        HuPrcSleep(150);
        fn_1_56B4(0);
        for (i = 0; i < 4; i++) {
            tournamentResultSequenceAdvance(i);
        }
        HuPrcSleep(16);
        for (i = 0; i < 7; i++) {
            fn_1_3394(i, 0);
        }
    } else if (i == 1) {
        for (i = 0, count = 0; i < 4; i++) {
            if ((MgModeWorkInt[i] == 2) || (MgModeWorkInt[i] == 3)) {
                if (MgModeWorkInt[i + 8] >= 10) {
                    MgModeWorkInt[i + 8] = 10;
                    winner = i;
                    count++;
                } else {
                    MgModeWorkInt[i + 8] = 0;
                    loser = i;
                }
            } else {
                MgModeWorkInt[i + 8] = 0;
            }
        }
        if (count != 1) {
            fn_1_2728(-1);
            MgModeWorkInt[8] = MgModeWorkInt[9] = MgModeWorkInt[10] = MgModeWorkInt[11] = 0;
            for (i = 0, count = 0; i < 4; i++) {
                if ((MgModeWorkInt[i] == 2) || (MgModeWorkInt[i] == 3)) {
                    tournamentResultStateSet(i, 0);
                }
            }
            result = -1;
        } else {
            fn_1_3394(0, 0);
            fn_1_3394(1, 0);
            fn_1_3394(2, 0);
            fn_1_3394(3, 0);
            fn_1_3394(4, 0);
            fn_1_3394(5, 1);
            fn_1_3394(6, 0);
            fn_1_2728(winner);
            fn_1_143C(6, 30);
            HuPrcSleep(30);
            HuAudFXPlay(MSM_SE_MGTOUR_ROUND_WIN);
            tournamentResultPositionSet(winner, lbl_1_data_820[1]);
            Hu3DParManAttrReset(lbl_1_bss_10C[winner], 1);
            HuPrcVSleep();
            count = 1;
            while (count != 0) {
                count = 0;
                for (i = 0; i < 4; i++) {
                    if (tournamentResultSceneDone(i) == 0) {
                        count = 1;
                    }
                }
                HuPrcVSleep();
            }
            Hu3DParManAttrSet(lbl_1_bss_10C[winner], 1);
            fn_1_56B4(1);
            tournamentResultStateSet(winner, 1);
            tournamentResultStateSet(loser, 0);
            MgModeWorkInt[winner + 4] = 1;
        }
        HuPrcSleep(150);
        fn_1_56B4(0);
        for (i = 0; i < 4; i++) {
            tournamentResultSequenceAdvance(i);
        }
        HuPrcSleep(16);
        for (i = 0; i < 7; i++) {
            fn_1_3394(i, 0);
        }
    } else if (i == 2) {
        for (i = 0, count = 0; i < 4; i++) {
            if (MgModeWorkInt[i + 4] == 2) {
                if (MgModeWorkInt[i + 8] >= 10) {
                    MgModeWorkInt[i + 8] = 10;
                    winner = i;
                    count++;
                } else {
                    MgModeWorkInt[i + 8] = 0;
                    loser = i;
                }
            } else {
                MgModeWorkInt[i + 8] = 0;
            }
        }
        if (count != 1) {
            fn_1_2728(-1);
            MgModeWorkInt[8] = MgModeWorkInt[9] = MgModeWorkInt[10] = MgModeWorkInt[11] = 0;
            for (i = 0, count = 0; i < 4; i++) {
                if (MgModeWorkInt[i + 4] == 2) {
                    tournamentResultStateSet(i, 0);
                }
            }
            result = -1;
        } else {
            fn_1_3394(0, 0);
            fn_1_3394(1, 0);
            fn_1_3394(2, 0);
            fn_1_3394(3, 0);
            fn_1_3394(4, 0);
            fn_1_3394(5, 0);
            fn_1_3394(6, 0);
            if (MgModeWorkInt[loser] < 2) {
                fn_1_3394(2, 1);
                fn_1_34B0(1);
            } else {
                fn_1_3394(1, 1);
                fn_1_34B0(2);
            }
            fn_1_2728(winner);
            HuAudFXPlay(MSM_SE_MGTOUR_ROUND_LOSE);
            tournamentPlayerVibrate(loser, 20, 20, 0);
            tournamentResultSceneStart(loser);
            if (MgModeWorkInt[loser] < 2) {
                fn_1_3F18(0);
            } else {
                fn_1_3F18(1);
            }
            HuPrcVSleep();
            count = 1;
            while (count != 0) {
                count = 0;
                for (i = 0; i < 4; i++) {
                    if (tournamentResultSceneDone(i) == 0) {
                        count = 1;
                    }
                }
                HuPrcVSleep();
            }
            fn_1_143C(8, 30);
            HuPrcSleep(30);
            fn_1_56B4(1);
            tournamentResultStateSet(winner, 1);
            MgModeWorkInt[loser + 4] = 3;
        }
        HuPrcSleep(150);
        fn_1_56B4(0);
        for (i = 0; i < 4; i++) {
            tournamentResultSequenceAdvance(i);
        }
        HuPrcSleep(16);
        for (i = 0; i < 7; i++) {
            fn_1_3394(i, 0);
        }
    } else {
        for (i = 0, count = 0; i < 4; i++) {
            if (MgModeWorkInt[i + 4] == 1) {
                if (MgModeWorkInt[i + 8] >= 10) {
                    MgModeWorkInt[i + 8] = 10;
                    winner = i;
                    count++;
                } else {
                    MgModeWorkInt[i + 8] = 0;
                    loser = i;
                }
            } else {
                MgModeWorkInt[i + 8] = 0;
            }
        }
        if (count != 1) {
            fn_1_2728(-1);
            MgModeWorkInt[8] = MgModeWorkInt[9] = MgModeWorkInt[10] = MgModeWorkInt[11] = 0;
            for (i = 0, count = 0; i < 4; i++) {
                if (MgModeWorkInt[i + 4] == 1) {
                    tournamentResultStateSet(i, 0);
                }
            }
            result = -1;
        } else {
            fn_1_3394(0, 0);
            fn_1_3394(1, 0);
            fn_1_3394(2, 0);
            fn_1_3394(3, 0);
            fn_1_3394(4, 0);
            fn_1_3394(5, 0);
            fn_1_3394(6, 1);
            fn_1_2728(winner);
            fn_1_143C(10, 30);
            HuPrcSleep(30);
            HuAudFXPlay(MSM_SE_MGTOUR_ROUND_WIN);
            tournamentResultPositionSet(winner, lbl_1_data_838);
            Hu3DParManAttrReset(lbl_1_bss_10C[winner], 1);
            HuPrcVSleep();
            count = 1;
            while (count != 0) {
                count = 0;
                for (i = 0; i < 4; i++) {
                    if (tournamentResultSceneDone(i) == 0) {
                        count = 1;
                    }
                }
                HuPrcVSleep();
            }
            Hu3DParManAttrSet(lbl_1_bss_10C[winner], 1);
            fn_1_56B4(1);
            tournamentResultStateSet(winner, 1);
            tournamentResultStateSet(loser, 0);
            MgModeWorkInt[winner + 4] = 0;
            result = 1;
        }
        HuPrcSleep(150);
        fn_1_56B4(0);
        for (i = 0; i < 4; i++) {
            tournamentResultSequenceAdvance(i);
        }
        HuPrcSleep(16);
    }
    return result;
}

extern s32 lbl_1_data_84C;
extern s32 lbl_1_data_854;
extern s32 lbl_1_data_858;

void fn_1_43BC(void);
void fn_1_56B4(s16 mode);
void fn_1_143C(s16 mode, s16 duration);
void fn_1_5740(s16 mode, s16 duration);
void fn_1_4A94(s16 player, s16 characterNo, s16 group, Vec position);

s16 fn_1_7BC0(s16 player);

void fn_1_7D58(s16 player, s16 flag);

s16 fn_1_80CC(s16 player);

/* Called by fn_1_CF6C to set one player's scene state for the winner presentation. */
static inline void tournamentPlayerStateSet(s16 player, s16 state)
{
    TournamentPlayerAnimationWork *playerData;

    playerData = lbl_1_bss_114[player]->data;
    playerData->sceneState = state;
}

/* Presents the tournament champion, opens the wider final view, and places each player's
 * portrait and ranking badge beside its character. */
void fn_1_CF6C(void)
{
    s16 player;
    s16 firstPlayer;
    s16 messageCharacters[4];
    s16 keepPolling;
    s16 fourthPlacePlayer;
    Vec position;
    TournamentPlayerAnimationWork *scenePlayerData;
    for (player = 0; player < 4; player++)
    {
        if (MgModeWorkInt[player + 4] == 0)
        {
            firstPlayer = player;
        }
        if (MgModeWorkInt[player + 4] == 3)
        {
            fourthPlacePlayer = player;
        }
    }
    for (player = 0; player < 4; player++)
    {
        messageCharacters[player] = - 1;
    }
    messageCharacters[firstPlayer] = tournamentPlayerCharacterGet(firstPlayer);
    GameMesCreate(4, 3, messageCharacters[0], messageCharacters[1], messageCharacters[2],
                  messageCharacters[3]);
    lbl_1_data_858 = HuAudJinglePlay(MSM_STREAM_MGMUS_12);
    fn_1_43BC();
    fn_1_56B4(1);
    tournamentPlayerStateSet(firstPlayer, 22);
    for (player = 0; player < 4; player++)
    {
        if ((firstPlayer != player) && (fourthPlacePlayer != player))
        {
            fn_1_7D58(player, 0);
        }
    }
    keepPolling = 1;
    while (keepPolling != 0)
    {
        keepPolling = 0;
        if (fn_1_80CC(firstPlayer) == 0)
        {
            keepPolling = 1;
        }
        else
        {
            tournamentPlayerVibrate(firstPlayer, 20, 20, 0);
        }
        HuPrcVSleep();
    }
    lbl_1_data_854 = HuAudFXPlay(MSM_SE_MGTOUR_CHAMPION);
    HuPrcSleep(60);
    fn_1_56B4(0);
    if (0 <= lbl_1_data_858)
    {
        player = 1;
        while (player != 0)
        {
            if (HuAudSStreamStatGet(lbl_1_data_858) == 0)
            {
                player = 0;
            }
            HuPrcVSleep();
        }
    }
    lbl_1_data_84C = HuAudSStreamPlay(MSM_STREAM_MGMUS_KOOPA);
    fn_1_143C(12, 60);
    fn_1_5740(15, 60);
    HuPrcSleep(60);
    for (player = 0; player < 4; player++)
    {
        scenePlayerData = lbl_1_bss_114[player]->data;
        position = scenePlayerData->position;
        if (MgModeWorkInt[player + 4] == 0)
        {
            position.y -= (100.0f);
            if ((MgModeWorkInt[player] == 0) || (1 == MgModeWorkInt[player]))
            {
                position.x -= (330.0f);
            }
            else
            {
                position.x += (330.0f);
            }
        }
        else
        {
            if (MgModeWorkInt[player + 4] == 1)
            {
                position.y += (160.0f);
                if ((MgModeWorkInt[player] == 2) || (MgModeWorkInt[player] == 3))
                {
                    position.x += (320.0f);
                }
                else
                {
                    position.x -= (320.0f);
                }
            }
            else
            {
                if (MgModeWorkInt[player + 4] == 2)
                {
                    position.y += (150.0f);
                    position.x += (300.0f);
                }
                else
                {
                    position.y += (240.0f);
                    position.x += (300.0f);
                }
            }
        }
        fn_1_4A94(player, tournamentPlayerCharacterGet(player), MgModeWorkInt[player + 4],
                  position);
    }
    HuPrcVSleep();
}

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

void fn_1_E9C(void);
void fn_1_DC28(void);

/* Starts the tournament menu's object manager and board process. */
void fn_1_D544(void)
{
    lbl_1_bss_12C = omInitObjMan(100, 8192);
    MgModeSubMode = 0;
    fn_1_E9C();
    HuPrcChildCreate(fn_1_DC28, 100U, 12288U, 0, lbl_1_bss_12C);
}

/* Runs static constructors before starting the tournament board process. */
s32 _prolog(void)
{
    const VoidFunc *constructor;

    constructor = _ctors;
    while (*constructor != 0) {
        (*constructor)();
        constructor++;
    }
    lbl_1_bss_12C = omInitObjMan(100, 8192);
    *(s16 *)&MgModeSubMode = 0;
    fn_1_E9C();
    HuPrcChildCreate(fn_1_DC28, 100U, 12288U, 0, lbl_1_bss_12C);
    return 0;
}

/* Runs static destructors when the tournament module is unloaded. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}
