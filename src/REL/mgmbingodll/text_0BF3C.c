/* Runs the Treetop Bingo player, board, and scoring behavior. */
#include "REL/mgmbingodll/module_types.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "game/charman.h"
#include "game/gamework.h"
#include "game/memory.h"
#include <string.h>
void fn_1_D634(OMOBJ *object);
void fn_1_A1F4(void);

void fn_1_D634(OMOBJ *object);

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
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include "msm_se.h"
#include "msm_stream.h"

#define MSM_SE_BINGO_GAME_COUNT_CHANGE 1211
#define MSM_SE_BINGO_GAME_COUNT_WINDOW 1212
#define MSM_SE_BINGO_GAME_COUNT_CONFIRM 1216
#define MSM_SE_BINGO_FINAL_RESULTS 1262
#define MSM_SE_BINGO_RESULTS_CAMERA 1266
#define BINGO_CHARACTER_RESULT_VOICE 579

extern u8 MgModeWork[256];
extern u16 HuPadBtnDown[];
extern u8 HuPadDStkRep[];
s16 fn_1_8FA8(s16 *scores);
void fn_1_8BDC(s16 playerIndex, s16 requiredMarks, s16 *lineScores);
s16 fn_1_901C(s16 *lineScores);

void fn_1_AAC8(HuVecF *position);
s32 _CharFXPlay(s16, s32, s32);

#include "dolphin/math.h"

extern s16 MgModeScore[4];
extern int MgModeWorkInt[16];
void fn_1_2DB4(void);
void fn_1_2B10(void);
void fn_1_9554(void);
void fn_1_2C20(void);
void fn_1_A2A4(void);
void fn_1_108C(s16 pathIndex, s16 frames);
void fn_1_10A64(s16 positionIndex, s16 frames);
void fn_1_109FC(s16 enable);
void fn_1_311C(s16 player);
void fn_1_32CC(s16 player);
void fn_1_70AC(s16 player);
void fn_1_347C(void);
void fn_1_54FC(s16 player);
void fn_1_2114(void);

extern u16 HuPadBtnDown[4];
extern u8 HuPadDStkRep[4];
void fn_1_1C14(void);
void fn_1_1D1C(void);
void fn_1_1D70(void);
s16 fn_1_1DA4(void);

#include "game/process.h"

extern s16 MgGameNo;

void fn_1_108C(s16 pathIndex, s16 duration);
void fn_1_10970(u16 enable);
void fn_1_10A64(s16 positionIndex, s16 duration);
void fn_1_34B0(void);
void fn_1_50B4(void);
void fn_1_B498(void);
void fn_1_BDB4(void);

// The round controller calls this to check whether the player may advance.
static inline s16 PlayerIsReady(s16 playerNo)
{
    MgmbingoPlayerData *playerData = (MgmbingoPlayerData *)lbl_1_bss_ACC[playerNo]->data;

    if ((playerData->state == -1) || (playerData->state == 0) ||
        (playerData->state == 9) || (playerData->state == 12) ||
        (playerData->state == 15) || (playerData->state == 18) ||
        (playerData->state == 6)) {
        return 1;
    }
    return 0;
}

u32 lbl_1_data_848[8] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 3), DATANUM(DATA_mariomot, 4),
    DATANUM(DATA_mariomot, 6), DATANUM(DATA_mariomot, 7), DATANUM(DATA_mariomot, 36),
    DATANUM(DATA_mariomot, 37), DATANUM(DATA_mario, 126)
};

Point3d lbl_1_data_868[4] = { { -430.0f, 1060.0f, 0.0f },
                              { 430.0f, 1060.0f, 0.0f },
                              { -430.0f, 710.0f, 0.0f },
                              { 430.0f, 710.0f, 0.0f } };

Point3d lbl_1_data_898[2] = {{0.0f, 1900.0f, -700.0f}, {0.0f, 2240.0f, 50.0f}};

BingoMotionSetting lbl_1_data_8B0[10] = {
    {0, 0.0f, HU3D_MOTATTR_LOOP, 1, 0},
    {0, 8.0f, HU3D_MOTATTR_LOOP, 1, 0},
    {1, 0.0f, HU3D_MOTATTR_LOOP, 1, 0},
    {2, 0.0f, HU3D_MOTATTR_LOOP, 1, 0},
    {3, 8.0f, HU3D_MOTATTR_LOOP, 1, 0},
    {4, 8.0f, HU3D_MOTATTR_LOOP, 1, 0},
    {5, 8.0f, HU3D_MOTATTR_LOOP, 1, 0},
    {6, 8.0f, HU3D_MOTATTR_LOOP, 1, 0},
    {7, 0.0f, HU3D_MOTATTR_LOOP, 1, 0},
    {7, 8.0f, HU3D_MOTATTR_LOOP, 1, 0}
};

// Applies a player's requested character animation and voice settings during state updates.
void fn_1_BF3C(OMOBJ *object, s16 motionSettingIndex) {
    s16 motionIndex;
    MgmbingoPlayerData *motionData;

    motionData = object->data;
    if ((motionData->playerIndex < 0) || (motionData->playerIndex >= 4) ||
        (motionSettingIndex < 0) || (motionSettingIndex >= 10)) {
        return;
    }
    if (motionData->currentMotion != motionSettingIndex) {
        motionIndex = lbl_1_data_8B0[motionSettingIndex].motionIndex;
        if (motionIndex != -1) {
            if (lbl_1_data_8B0[motionSettingIndex].duration <= 1.0f) {
                CharMotionSet(motionData->characterNo, object->mtnId[motionIndex]);
                Hu3DModelAttrReset(
                    object->mdlId[0],
                    Hu3DModelMotionAttrGet(object->mdlId[0]) | HU3D_MOTATTR);
                CharModelAttrSet(
                    motionData->characterNo,
                    lbl_1_data_8B0[motionSettingIndex].modelAttributes);
            } else {
                CharModelAttrSet(motionData->characterNo, HU3D_MOTATTR_PAUSE);
                CharMotionShiftSet(motionData->characterNo, object->mtnId[motionIndex],
                    0.0f,
                    lbl_1_data_8B0[motionSettingIndex].duration,
                    lbl_1_data_8B0[motionSettingIndex].modelAttributes);
            }
            // Both settings enable the same motion-record flag.
            if (lbl_1_data_8B0[motionSettingIndex].setMotionAttribute != 0) {
                Hu3DMotionAttrSet(object->mtnId[motionIndex], 1U);
            } else {
                Hu3DMotionAttrSet(object->mtnId[motionIndex], 1U);
            }
            CharModelVoiceFlagSet(motionData->characterNo,
                                  lbl_1_data_8B0[motionSettingIndex].voiceFlag);
        }
        motionData->currentMotion = motionSettingIndex;
        // Record the immediate idle setting as setting one so the next idle update does not request
        // a blend.
        if (motionData->currentMotion == 0) {
            motionData->currentMotion = 1;
        }
    }
}

// Creates the four player objects, character models, and their motion sets at mode start.
void fn_1_C168(void)
{
    s32 player;
    s32 motionIndex;
    OMOBJ *playerObject;
    MgmbingoPlayerData *playerData;

    player = 0;
    while (player < 4) {
        playerObject = omAddObjEx(lbl_1_bss_B00, 4352, 1, 8, -1,
            fn_1_D634);
        lbl_1_bss_ACC[player] = playerObject;
        playerObject->stat |= 0x100;
        playerObject->work[0] = playerObject->work[1] =
            playerObject->work[2] = playerObject->work[3] = 0;
        playerObject->data = HuMemDirectMallocNum(HEAP_HEAP, 72, HU_MEMNUM_OVL);
        memset(playerObject->data, 0, 72);
        playerData = playerObject->data;
        playerData->playerIndex = player;
        playerData->characterNo = GwPlayerConf[player].charNo;
        playerData->padIndex = GwPlayerConf[player].padNo;
        playerData->groupNo = GwPlayerConf[player].grpNo;
        playerData->inputMode = GwPlayerConf[player].type;
        playerData->difficulty = GwPlayerConf[player].comDif;
        playerData->state = -1;
        *playerObject->mdlId = CharModelCreate(playerData->characterNo, 4);
        motionIndex = 0;
        while (motionIndex < 8) {
            playerObject->mtnId[motionIndex] = CharMotionCreate(
                playerData->characterNo, lbl_1_data_848[motionIndex]);
            motionIndex++;
        }
        CharMotionDataClose(playerData->characterNo);
        playerData->currentMotion = -1;
        fn_1_BF3C(playerObject, 8);
        CharModelAttrSet(playerData->characterNo, HU3D_MOTATTR_PAUSE);
        Hu3DModelCameraSet(*playerObject->mdlId, 1);
        Hu3DModelLayerSet(*playerObject->mdlId, 1);
        Hu3DModelShadowSet(*playerObject->mdlId);
        Hu3DModelAttrSet(*playerObject->mdlId, HU3D_ATTR_NOCULL);
        playerData->position = lbl_1_data_868[player];
        playerData->position.y -= 70.0f;
        Hu3DModelPosSetV(*playerObject->mdlId, &playerData->position);
        playerData->rotation.x = playerData->rotation.y =
            playerData->rotation.z = 0.0f;
        Hu3DModelRotSetV(*playerObject->mdlId, &playerData->rotation);
        CharMotionVoiceOnSet(playerData->characterNo,
            (s16)lbl_1_data_848[5], 0);
        player++;
    }
    CharEffectLayerSet(2);
}

// Resets player characters to their starting positions before a new board round.
void fn_1_C578(void) {
    s16 playerIndex = 0;
    s16 motionSettingIndex;
    MgmbingoPlayerData *playerData;
    OMOBJ *object;

    while (playerIndex < 4) {
        object = lbl_1_bss_ACC[playerIndex];
        playerData = object->data;
        playerData->state = 0;
        playerData->currentMotion = -1;
        fn_1_BF3C(object, 0);

        playerData->position = lbl_1_data_868[playerIndex];
        playerData->position.z += 75.0f;
        Hu3DModelPosSetV(object->mdlId[0], &playerData->position);
        playerData->rotation.x = playerData->rotation.z =
            playerData->rotation.y = 0.0f;
        Hu3DModelRotSetV(object->mdlId[0], &playerData->rotation);
        playerIndex++;
    }

    playerIndex = 0;
    while (playerIndex < 2) {
        object = lbl_1_bss_ACC[playerIndex];
        playerData = object->data;
        if ((playerData->characterNo == 2) || (playerData->characterNo == 5) ||
            (playerData->characterNo == 6)) {
            playerData->position.y -= 30.0f;
        }
        playerIndex++;
    }
}

// Forces each player into a separate group numbered by player index and saves those assignments.
void fn_1_C848(void) {
    s16 playerIndex;
    MgmbingoPlayerData *playerData;

    playerIndex = 0;
    while (playerIndex < 4) {
        playerData = lbl_1_bss_ACC[playerIndex]->data;
        playerData->groupNo = playerIndex;
        GwPlayerConf[playerIndex].grpNo = playerData->groupNo;
        playerIndex += 1;
    }
}

// Returns live human input or the timed button sequence chosen for a computer player.
u16 fn_1_C8B8(s16 playerIndex, s16 targetCell) {
    MgmbingoPlayerData *input;
    u16 buttons;
    s16 lineScores[25];
    s16 boardCell;
    s16 rowDifference;
    s16 eventIndex;
    s16 eventFrame;

    buttons = 0;
    input = (MgmbingoPlayerData *)lbl_1_bss_ACC[playerIndex]->data;
    if (input->inputMode == 0) {
        buttons = HuPadBtnDown[input->padIndex] & 0x320;
        buttons |= HuPadDStkRep[input->padIndex];
    } else {
        if (lbl_1_bss_A70 == 0) {
            eventIndex = 0;
            while (eventIndex < 10) {
                lbl_1_bss_AA4[eventIndex].frame = 0;
                lbl_1_bss_AA4[eventIndex].button = 0;
                eventIndex++;
            }
            switch (input->difficulty) {
            case 0:
                fn_1_8BDC(playerIndex, 4, lineScores);
                boardCell = fn_1_901C(lineScores);
                break;
            case 1:
                fn_1_8BDC(playerIndex, 4, lineScores);
                boardCell = fn_1_8FA8(lineScores);
                if (boardCell < 0) {
                    fn_1_8BDC(playerIndex, 3, lineScores);
                    boardCell = fn_1_901C(lineScores);
                }
                break;
            case 2:
                fn_1_8BDC(playerIndex, 4, lineScores);
                boardCell = fn_1_8FA8(lineScores);
                if (boardCell < 0) {
                    fn_1_8BDC(playerIndex, 3, lineScores);
                    boardCell = fn_1_8FA8(lineScores);
                }
                if (boardCell < 0) {
                    fn_1_8BDC(playerIndex, 2, lineScores);
                    boardCell = fn_1_8FA8(lineScores);
                }
                break;
            case 3:
                fn_1_8BDC(playerIndex, 4, lineScores);
                boardCell = fn_1_8FA8(lineScores);
                if (boardCell < 0) {
                    fn_1_8BDC(playerIndex, 3, lineScores);
                    boardCell = fn_1_8FA8(lineScores);
                }
                if (boardCell < 0) {
                    fn_1_8BDC(playerIndex, 2, lineScores);
                    boardCell = fn_1_8FA8(lineScores);
                }
                if (boardCell < 0) {
                    fn_1_8BDC(playerIndex, 1, lineScores);
                    boardCell = fn_1_901C(lineScores);
                }
                break;
            }

            if (boardCell < 0) {
                eventIndex = 0;
                rowDifference = 0;
                for (; eventIndex < 25; eventIndex++) {
                    if ((MgModeWork[101 + playerIndex * 25 + eventIndex] & 0x80) == 0) {
                        rowDifference++;
                    }
                }
                // The fallback reuses the random ordinal as a cell index and keeps scanning; later
                // matches can replace it again.
                boardCell = frandmod(rowDifference);
                eventIndex = 0;
                rowDifference = 0;
                for (; eventIndex < 25; eventIndex++) {
                    if ((MgModeWork[101 + playerIndex * 25 + eventIndex] & 0x80) == 0) {
                        if (boardCell == rowDifference) {
                            boardCell = eventIndex;
                        } else {
                            rowDifference++;
                        }
                    }
                }
            }

            // Script one cursor step every 30 frames, then confirm the selected cell.
            eventIndex = 0;
            rowDifference = (boardCell / 5) - (targetCell / 5);
            while (rowDifference < 0) {
                lbl_1_bss_AA4[eventIndex].frame = (eventIndex * 30) + 30;
                lbl_1_bss_AA4[eventIndex].button = 8;
                rowDifference++;
                eventIndex++;
            }
            while (rowDifference > 0) {
                lbl_1_bss_AA4[eventIndex].frame = (eventIndex * 30) + 30;
                lbl_1_bss_AA4[eventIndex].button = 4;
                rowDifference--;
                eventIndex++;
            }
            rowDifference = (boardCell % 5) - (targetCell % 5);
            while (rowDifference < 0) {
                lbl_1_bss_AA4[eventIndex].frame = (eventIndex * 30) + 30;
                lbl_1_bss_AA4[eventIndex].button = 1;
                rowDifference++;
                eventIndex++;
            }
            while (rowDifference > 0) {
                lbl_1_bss_AA4[eventIndex].frame = (eventIndex * 30) + 30;
                lbl_1_bss_AA4[eventIndex].button = 2;
                rowDifference--;
                eventIndex++;
            }
            lbl_1_bss_AA4[eventIndex].frame = (eventIndex * 30) + 30;
            lbl_1_bss_AA4[eventIndex].button = PAD_BUTTON_A;
        } else {
            eventIndex = 0;
            while (eventIndex < 10) {
                if (lbl_1_bss_A70 == lbl_1_bss_AA4[eventIndex].frame) {
                    buttons = lbl_1_bss_AA4[eventIndex].button;
                }
                eventIndex++;
            }
        }
        lbl_1_bss_A70++;
        if (buttons & 0x100) {
            lbl_1_bss_A70 = 0;
        }
    }
    return buttons;
}

// Returns the character selected for one player when menus build name messages.
s16 fn_1_CEF0(s16 playerNo) {
    MgmbingoPlayerData *playerData = lbl_1_bss_ACC[playerNo]->data;
    s16 characterNo = playerData->characterNo;

    return characterNo;
}

// Returns whether the player is human controlled or computer controlled.
s16 fn_1_CF30(s16 playerNo) {
    MgmbingoPlayerData *playerData = lbl_1_bss_ACC[playerNo]->data;
    s16 inputMode = (s16) playerData->inputMode;

    return inputMode;
}

// Returns the controller port assigned to one player for window input.
s16 fn_1_CF74(s16 playerNo) {
    MgmbingoPlayerData *playerData = lbl_1_bss_ACC[playerNo]->data;

    return playerData->padIndex;
}

// Sets every player's state to the ready position before a bingo turn.
void fn_1_CFA8(void) {
    s16 playerIndex;
    MgmbingoPlayerData *playerData;

    playerIndex = 0;
    while (playerIndex < 4) {
        playerData = lbl_1_bss_ACC[playerIndex]->data;
        playerData->state = 1;
        playerIndex += 1;
    }
}

// Starts every character's transition to the hidden state before the final-results entrance.
void fn_1_D000(void) {
    s16 playerNo;
    for (playerNo = 0; playerNo < 4; ++playerNo) {
        MgmbingoPlayerData *playerData = (MgmbingoPlayerData *)lbl_1_bss_ACC[playerNo]->data;
        playerData->state = 4;
    }
}

// Queues one of two character reactions; the final-results presentation enables it for qualifying
// players.
void fn_1_D058(s16 playerNo, s16 enabled) {
    MgmbingoPlayerData *playerData = (MgmbingoPlayerData *)lbl_1_bss_ACC[playerNo]->data;
    if (enabled != 0) {
        playerData->state = 7;
        return;
    }
    playerData->state = 10;
}

// Queues the selected scorer's result animation when enabled, or the non-scoring result animation
// otherwise.
void fn_1_D0A8(s16 playerNo, s16 enabled) {
    MgmbingoPlayerData *playerData = (MgmbingoPlayerData *)lbl_1_bss_ACC[playerNo]->data;
    if (enabled != 0) {
        playerData->state = 13;
        return;
    }
    playerData->state = 16;
}

// Sets the player's entrance and target positions for the selected result placement.
void fn_1_D0F8(s16 playerNo, s16 placementType, s16 placementIndex) {
    MgmbingoPlayerData *playerData =
        (MgmbingoPlayerData *)lbl_1_bss_ACC[playerNo]->data;

    playerData->state = 19;
    if (placementType <= 1) {
        playerData->position = lbl_1_data_898[0];
        playerData->position.x = 0.0f;
        playerData->targetPosition = lbl_1_data_898[1];
        playerData->targetPosition.x = playerData->position.x;
        return;
    }
    if (placementType <= 2) {
        playerData->position.x = (f32)(placementIndex * 140 - 70);
        playerData->position.y = lbl_1_data_898[0].y;
        playerData->position.z = lbl_1_data_898[0].z;
        playerData->targetPosition = lbl_1_data_898[1];
        playerData->targetPosition.x = playerData->position.x;
        return;
    }
    if (placementType <= 3) {
        playerData->position.x = (f32)(placementIndex * 140 - 140);
        playerData->position.y = lbl_1_data_898[0].y;
        playerData->position.z = lbl_1_data_898[0].z;
        playerData->targetPosition = lbl_1_data_898[1];
        playerData->targetPosition.x = playerData->position.x;
        return;
    }
    playerData->position.x = (f32)(placementIndex * 100 - 150);
    playerData->position.y = lbl_1_data_898[0].y;
    playerData->position.z = lbl_1_data_898[0].z +
        (-100.0f * (f32)(1 - (placementIndex % 2)));
    playerData->targetPosition = lbl_1_data_898[1];
    playerData->targetPosition.x = playerData->position.x;
    playerData->targetPosition.z +=
        -100.0f * (f32)(1 - (placementIndex % 2));
}

// Returns either result-reaction sequence to idle when the player returns to the board.
void fn_1_D3D4(s16 playerNo) {
    MgmbingoPlayerData *playerData = (MgmbingoPlayerData *)lbl_1_bss_ACC[playerNo]->data;

    if ((playerData->state == 7) || (playerData->state == 8) ||
        (playerData->state == 9) || (playerData->state == 13) ||
        (playerData->state == 14) || (playerData->state == 15)) {
        playerData->state = 0;
        return;
    }
    if ((playerData->state == 10) || (playerData->state == 11) ||
        (playerData->state == 12) || (playerData->state == 16) ||
        (playerData->state == 17) || (playerData->state == 18)) {
        playerData->state = 0;
    }
}

// Reports whether the player is in one of the states that may advance the round.
s32 fn_1_D4A8(s16 playerNo) {
    MgmbingoPlayerData *playerData = (MgmbingoPlayerData *)lbl_1_bss_ACC[playerNo]->data;

    if ((playerData->state == -1) || (playerData->state == 0) ||
        (playerData->state == 9) || (playerData->state == 12) ||
        (playerData->state == 15) || (playerData->state == 18) ||
        (playerData->state == 6)) {
        return 1;
    }
    return 0;
}

// Yields each frame until all four players have reached a state ready to continue.
void fn_1_D538(void) {
    s16 isReadyState;
    s16 shouldWait;
    s16 playerNo;
    MgmbingoPlayerData *playerData;

    shouldWait = 1;
    while (shouldWait != 0) {
        shouldWait = 0;
        playerNo = 0;
        while (playerNo < 4) {
            if (PlayerIsReady(playerNo) == 0) {
                shouldWait = 1;
            }
            playerNo++;
        }
        HuPrcVSleep();
    }
}

// Advances one player's movement, board animation, and result states each frame.
void fn_1_D634(OMOBJ *object) {
    MgmbingoPlayerData *work = (MgmbingoPlayerData *)object->data;
    f32 motionEnd;

    switch (work->state) {
    case 0:
        fn_1_BF3C(object, 1);
        break;
    case 1:
        CharModelAttrReset(work->characterNo, HU3D_MOTATTR_PAUSE);
        if (CharMotionShiftIDGet(work->characterNo) < 0) {
            work->state = 2;
        }
        break;
    case 2:
        work->position.y += 10.0f;
        if ((work->playerIndex == 0 || work->playerIndex == 1) &&
            (work->characterNo == 2 || work->characterNo == 5 || work->characterNo == 6)) {
            if (work->position.y >= lbl_1_data_868[work->playerIndex].y - 30.0f) {
                work->state = 3;
                work->position.y = lbl_1_data_868[work->playerIndex].y;
                work->position.y -= 30.0f;
            }
        } else if (work->position.y >= lbl_1_data_868[work->playerIndex].y) {
            work->state = 3;
            work->position.y = lbl_1_data_868[work->playerIndex].y;
        }
        if (CharMotionShiftIDGet(work->characterNo) < 0) {
            motionEnd = CharMotionMaxTimeGet(work->characterNo);
            if (Hu3DData[object->mdlId[0]].motWork.speed +
                CharMotionTimeGet(work->characterNo) >= motionEnd) {
                fn_1_BF3C(object, 1);
            }
        }
        break;
    case 3:
        if (CharMotionShiftIDGet(work->characterNo) < 0) {
            motionEnd = CharMotionMaxTimeGet(work->characterNo);
            if (Hu3DData[object->mdlId[0]].motWork.speed +
                CharMotionTimeGet(work->characterNo) >= motionEnd) {
                fn_1_BF3C(object, 1);
                work->state = 0;
            }
        }
        break;
    case 4:
        fn_1_BF3C(object, 9);
        work->state = 5;
        break;
    case 5:
        if (CharMotionShiftIDGet(work->characterNo) < 0) {
            work->state = 6;
        }
        break;
    case 6:
        Hu3DModelAttrSet(object->mdlId[0], 1U);
        break;
    case 7:
        fn_1_BF3C(object, 4);
        work->state = 8;
        break;
    case 8:
        if (CharMotionShiftIDGet(work->characterNo) < 0) {
            motionEnd = CharMotionMaxTimeGet(work->characterNo);
            if (Hu3DData[object->mdlId[0]].motWork.speed +
                CharMotionTimeGet(work->characterNo) >= motionEnd) {
                Hu3DModelAttrSet(object->mdlId[0], HU3D_MOTATTR_PAUSE);
                work->state = 9;
            }
        }
        break;
    case 10:
        fn_1_BF3C(object, 5);
        work->state = 11;
        break;
    case 11:
        if (CharMotionShiftIDGet(work->characterNo) < 0) {
            motionEnd = CharMotionMaxTimeGet(work->characterNo);
            if (Hu3DData[object->mdlId[0]].motWork.speed +
                CharMotionTimeGet(work->characterNo) >= motionEnd) {
                Hu3DModelAttrSet(object->mdlId[0], HU3D_MOTATTR_PAUSE);
                work->state = 12;
            }
        }
        break;
    case 13:
        fn_1_BF3C(object, 6);
        _CharFXPlay(work->characterNo, BINGO_CHARACTER_RESULT_VOICE, 0);
        work->state = 14;
        break;
    case 14:
        if (CharMotionShiftIDGet(work->characterNo) < 0) {
            motionEnd = CharMotionMaxTimeGet(work->characterNo);
            if (Hu3DData[object->mdlId[0]].motWork.speed +
                CharMotionTimeGet(work->characterNo) >= motionEnd) {
                Hu3DModelAttrSet(object->mdlId[0], HU3D_MOTATTR_PAUSE);
                work->state = 15;
            }
        }
        break;
    case 16:
        fn_1_BF3C(object, 7);
        work->state = 17;
        break;
    case 17:
        if (CharMotionShiftIDGet(work->characterNo) < 0) {
            motionEnd = CharMotionMaxTimeGet(work->characterNo);
            if (Hu3DData[object->mdlId[0]].motWork.speed +
                CharMotionTimeGet(work->characterNo) >= motionEnd) {
                Hu3DModelAttrSet(object->mdlId[0], HU3D_MOTATTR_PAUSE);
                work->state = 18;
            }
        }
        break;
    case 19: {
        Vec position;
        Vec target;
        HuVecF *targetPointer;
        Hu3DModelAttrReset(object->mdlId[0], 1U);
        fn_1_BF3C(object, 2);
        work->velocityY = 20.0f;
        work->state = 20;
        position = work->position;
        position.y += 100.0f;
        target = position;
        targetPointer = &target;
        fn_1_AAC8(targetPointer);
        break;
    }
    case 20:
        if (work->currentMotion == 2 && CharMotionShiftIDGet(work->characterNo) < 0) {
            motionEnd = CharMotionMaxTimeGet(work->characterNo);
            if (Hu3DData[object->mdlId[0]].motWork.speed +
                CharMotionTimeGet(work->characterNo) >= motionEnd) {
                // Hold every motion channel while the character finishes the jump.
                Hu3DModelAttrSet(object->mdlId[0], HU3D_MOTATTR_PAUSE |
                    HU3D_MOTATTR_SHIFT_PAUSE | HU3D_MOTATTR_SHAPE_PAUSE |
                    HU3D_MOTATTR_OVL_PAUSE);
            }
        }
        work->position.z += 10.0f;
        if (work->position.z >= work->targetPosition.z) {
            work->position.z = work->targetPosition.z;
        }
        work->position.y += work->velocityY;
        work->velocityY -= 0.4f;
        if (work->position.y < work->targetPosition.y && work->velocityY < 0.0f) {
            work->velocityY = 0.0f;
            work->position = work->targetPosition;
            Hu3DModelAttrReset(object->mdlId[0], HU3D_MOTATTR_PAUSE |
                HU3D_MOTATTR_SHIFT_PAUSE | HU3D_MOTATTR_SHAPE_PAUSE |
                HU3D_MOTATTR_OVL_PAUSE);
            fn_1_BF3C(object, 3);
            work->state = 21;
        }
        break;
    case 21:
        if (CharMotionShiftIDGet(work->characterNo) < 0) {
            motionEnd = CharMotionMaxTimeGet(work->characterNo);
            if (Hu3DData[object->mdlId[0]].motWork.speed +
                CharMotionTimeGet(work->characterNo) >= motionEnd) {
                work->state = 0;
                fn_1_BF3C(object, 1);
            }
        }
        break;
    default:
        break;
    }

    if (work->position.x <= -200.0f) {
        work->rotation.y = 20.0f;
    } else if (200.0f <= work->position.x) {
        work->rotation.y = -20.0f;
    } else {
        work->rotation.y = 0.0f;
    }
    Hu3DModelRotSetV(object->mdlId[0], &work->rotation);
    Hu3DModelPosSetV(object->mdlId[0], &work->position);
}

// Reads a sprite group's member position for the current board display.
void fn_1_EE18(s16 spriteGroup, s16 memberIndex, f32 *positionX,
               f32 *positionY) {
    HUSPRITE *sprite;

    sprite = &HuSprData[HuSprGrpData[spriteGroup].sprId[memberIndex]];
    *positionX = sprite->pos.x;
    *positionY = sprite->pos.y;
}

s32 lbl_1_data_9D0[8] = {
    DATANUM(DATA_mgmbingo, 20), DATANUM(DATA_mgmbingo, 21), DATANUM(DATA_mgmbingo, 19),
    DATANUM(DATA_mgmbingo, 22), DATANUM(DATA_mgmbingo, 23), DATANUM(DATA_mgmbingo, 24),
    DATANUM(DATA_mgmbingo, 25), DATANUM(DATA_mgmbingo, 26)
};

Point3d lbl_1_data_9F0 = {288.0f, 122.0f, 0.0f};

Point3d lbl_1_data_9FC[8] = { { -116.0f, 0.0f, 0.0f }, { 116.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f },    { 0.0f, 0.0f, 0.0f },   { 0.0f, 0.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f },    { 0.0f, 0.0f, 0.0f } };

// Loads the eight sprites used to select the required bingo-line count.
void fn_1_EE74(void) {
    s16 spriteIndex;
    s16 unused;
    spriteIndex = 0;
    while (spriteIndex < 8)
    {
        lbl_1_bss_A84[spriteIndex] = HuSprAnimRead(
            HuDataSelHeapReadNum(lbl_1_data_9D0[spriteIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        spriteIndex += 1;
    }
    lbl_1_bss_A82 = HuSprGrpCreate(8);
    spriteIndex = 0;
    while (spriteIndex < 8)
    {
        lbl_1_bss_A72[spriteIndex] = HuSprCreate(lbl_1_bss_A84[spriteIndex], spriteIndex, 0);
        HuSprGrpMemberSet(lbl_1_bss_A82, spriteIndex, lbl_1_bss_A72[spriteIndex]);
        HuSprPosSet(lbl_1_bss_A82, spriteIndex, lbl_1_data_9FC[spriteIndex].x,
                    lbl_1_data_9FC[spriteIndex].y);
        spriteIndex += 1;
    }
    HuSprGrpPosSet(lbl_1_bss_A82, lbl_1_data_9F0.x, lbl_1_data_9F0.y);
    HuSprGrpScaleSet(lbl_1_bss_A82, 0.0f, 0.0f);
}

// Selects the required bingo-line count; returns zero to proceed or one after confirming exit.
s16 fn_1_F018(void) {
    s16 result;
    s16 playerIndex;
    s16 selectedPlayer;
    s16 previousGameNo;
    s16 loopActive;
    u16 dpadRepeat;
    u16 buttonDown;
    MgmbingoPlayerData *controllerData;
    f32 scale;
    f32 transitionScale;

    result = 0;
    playerIndex = 0;
    selectedPlayer = 0;
    loopActive = 1;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        controllerData = (MgmbingoPlayerData *)lbl_1_bss_ACC[playerIndex]->data;
        if (controllerData->inputMode == 0) {
            selectedPlayer = playerIndex;
            break;
        }
    }
    controllerData = (MgmbingoPlayerData *)lbl_1_bss_ACC[selectedPlayer]->data;
    HuPrcVSleep();
    fn_1_1C14();

restart_animation:
    HuSprGrpScaleSet(lbl_1_bss_A82, 0.0f, 0.0f);
    playerIndex = 0;
    while (playerIndex < 5) {
        HuSprScaleSet(lbl_1_bss_A82, playerIndex + 3,
                      0.0f, 0.0f);
        playerIndex++;
    }
    HuSprScaleSet(lbl_1_bss_A82, MgGameNo + 2,
                  1.0f, 1.0f);
    HuSprScaleSet(lbl_1_bss_A82, 2, 1.0f, 1.0f);
    HuSprScaleSet(lbl_1_bss_A82, 0, 1.0f, 1.0f);
    HuSprScaleSet(lbl_1_bss_A82, 1, 1.0f, 1.0f);
    HuAudFXPlay(MSM_SE_BINGO_GAME_COUNT_WINDOW);
    scale = 0.0f;
    while (scale < 1.0f) {
        HuSprGrpScaleSet(lbl_1_bss_A82, scale, scale);
        HuPrcVSleep();
        scale += 0.1f;
    }
    HuSprGrpScaleSet(lbl_1_bss_A82, 1.0f, 1.0f);
    fn_1_1D1C();
    loopActive = 1;
    previousGameNo = MgGameNo;

    while (loopActive != 0) {
        dpadRepeat = buttonDown = 0;
        if (selectedPlayer >= 0) {
            dpadRepeat = HuPadDStkRep[controllerData->padIndex];
            buttonDown = HuPadBtnDown[controllerData->padIndex];
        }
        if (previousGameNo != MgGameNo) {
            HuSprGrpScaleSet(lbl_1_bss_A82,
                             1.0f, 1.0f);
            HuSprScaleSet(lbl_1_bss_A82, previousGameNo + 2,
                          0.0f, 0.0f);
            HuSprScaleSet(lbl_1_bss_A82, MgGameNo + 2,
                          1.0f, 1.0f);
            scale = 0.0f;
            while (scale < 1.0f) {
                if (scale >= 1.0f) {
                    scale = 1.0f;
                }
                transitionScale = 1.0f +
                    ((1.0f - scale) *
                     (1.0f - scale));
                if (((MgGameNo < previousGameNo) &&
                     ((previousGameNo < 5) || (MgGameNo > 1))) ||
                    ((previousGameNo == 1) && (MgGameNo > 2))) {
                    HuSprScaleSet(lbl_1_bss_A82, 0,
                                  transitionScale, transitionScale);
                } else {
                    HuSprScaleSet(lbl_1_bss_A82, 1,
                                  transitionScale, transitionScale);
                }
                HuPrcVSleep();
                scale += 0.1f;
            }
            HuSprScaleSet(lbl_1_bss_A82, 0,
                          1.0f, 1.0f);
            HuSprScaleSet(lbl_1_bss_A82, 1,
                          1.0f, 1.0f);
            HuPrcVSleep();
            previousGameNo = MgGameNo;
        } else if ((buttonDown & 0x100) != 0) {
            HuAudFXPlay(MSM_SE_BINGO_GAME_COUNT_CONFIRM);
            loopActive = 0;
            result = 0;
        } else if ((buttonDown & 0x200) != 0) {
            HuAudFXPlay(MSM_SE_CMN_04);
            loopActive = 0;
            result = 1;
        } else if ((dpadRepeat & 1) != 0) {
            HuAudFXPlay(MSM_SE_BINGO_GAME_COUNT_CHANGE);
            MgGameNo--;
            if (MgGameNo < 1) {
                MgGameNo = 5;
            }
        } else if ((dpadRepeat & 2) != 0) {
            HuAudFXPlay(MSM_SE_BINGO_GAME_COUNT_CHANGE);
            MgGameNo++;
            if (MgGameNo > 5) {
                MgGameNo = 1;
            }
        }
        HuPrcVSleep();
    }
    fn_1_1D70();
    HuAudFXPlay(MSM_SE_BINGO_GAME_COUNT_WINDOW);
    scale = 1.0f;
    while (scale > 0.0f) {
        scale -= 0.1f;
        if (scale <= 0.0f) {
            scale = 0.0f;
        }
        HuSprGrpScaleSet(lbl_1_bss_A82,
                         scale, scale);
        HuPrcVSleep();
    }
    HuSprGrpScaleSet(lbl_1_bss_A82, 0.0f, 0.0f);
    HuPrcVSleep();
    if ((result != 0) && (fn_1_1DA4() != 0)) {
        goto restart_animation;
    }
    HuPrcVSleep();
    return result;
}

// Presents the round's winners and runs their exit or return-to-board sequence.
void fn_1_F704(void) {
    s16 activePlayerCount;
    s16 scorePlayer;
    s16 selectedPlayer;

    activePlayerCount = 0;
    scorePlayer = 0;
    while (scorePlayer < 4) {
        if (MgModeScore[scorePlayer] > 0) {
            activePlayerCount++;
            selectedPlayer = scorePlayer;
        }
        scorePlayer++;
    }
    if (activePlayerCount > 0) {
        if (activePlayerCount == 1) {
            fn_1_2DB4();
        } else {
            if (activePlayerCount > 1) {
                fn_1_2B10();
                fn_1_9554();
                fn_1_2C20();
                fn_1_A2A4();
            }
            activePlayerCount = 0;
            scorePlayer = 0;
            while (scorePlayer < 4) {
                if (MgModeScore[scorePlayer] > 0) {
                    activePlayerCount++;
                    selectedPlayer = scorePlayer;
                }
                scorePlayer++;
            }
        }
        fn_1_D0A8(selectedPlayer, 1);
        fn_1_D538();
        if (selectedPlayer == 0) {
            fn_1_108C(2, 50);
            fn_1_10A64(1, 30);
        } else if (selectedPlayer == 1) {
            fn_1_108C(3, 50);
            fn_1_10A64(2, 30);
        } else if (selectedPlayer == 2) {
            fn_1_108C(4, 50);
            fn_1_10A64(3, 30);
        } else {
            fn_1_108C(5, 50);
            fn_1_10A64(4, 30);
        }
        HuPrcSleep(50);
        fn_1_109FC(1);
        fn_1_D3D4(selectedPlayer);
        HuPrcSleep(8);
        fn_1_311C(selectedPlayer);
        fn_1_70AC(selectedPlayer);
        fn_1_347C();
        if ((MgModeWorkInt[15] / 5 >= 1) && (MgModeWorkInt[15] % 5 == 0)) {
            fn_1_32CC(selectedPlayer);
            fn_1_70AC(selectedPlayer);
            fn_1_347C();
        }
        fn_1_109FC(0);
        fn_1_108C(1, 50);
        fn_1_10A64(0, 30);
        HuPrcSleep(51);
        fn_1_54FC(selectedPlayer);
        HuPrcSleep(100);
        return;
    }

    fn_1_2DB4();
    scorePlayer = 0;
    while (scorePlayer < 4) {
        fn_1_D0A8(scorePlayer, 0);
        scorePlayer++;
    }
    fn_1_D538();
    HuPrcSleep(60);
    scorePlayer = 0;
    while (scorePlayer < 4) {
        fn_1_D3D4(scorePlayer);
        scorePlayer++;
    }
    HuPrcSleep(8);
}

// Shows reach and next-game messages when the bingo session continues after a round.
void fn_1_FCF4(void) {
    fn_1_2114();
}

// Runs the final celebration and winner display for players who reached the required bingo-line
// count.
void fn_1_FD14(void) {
    s32 eligible[4];
    s16 players[4];
    s16 eligibleCount;
    s16 player;
    s16 placementIndex;

    player = 0;
    eligibleCount = 0;
    while (player < 4) {
        eligible[player] = 0;
        if (MgModeWorkInt[player] >= MgGameNo) {
            eligibleCount++;
            eligible[player] = 1;
        }
        player++;
    }

    fn_1_34B0();
    fn_1_D000();
    fn_1_D538();

    fn_1_50B4();
    fn_1_10A64(5, 180);
    fn_1_108C(0, 180);
    HuPrcSleep(120);
    fn_1_BDB4();
    HuPrcSleep(60);
    fn_1_10A64(6, 30);
    HuAudFXPlay(MSM_SE_BINGO_RESULTS_CAMERA);
    fn_1_B498();
    HuAudSStreamFadeOut(lbl_1_data_AC8, 200);
    HuAudFXPlay(MSM_SE_BINGO_FINAL_RESULTS);

    player = 0;
    placementIndex = 0;
    while (player < 4) {
        if (eligible[player] != 0) {
            fn_1_D0F8(player, eligibleCount, placementIndex);
            placementIndex++;
        }
        player++;
    }

    fn_1_D538();

    player = 0;
    while (player < 4) {
        players[player] = -1;
        if (eligible[player] != 0) {
            players[player] = fn_1_CEF0(player);
        }
        player++;
    }
    GameMesCreate(4, 3, players[0], players[1],
                  players[2], players[3]);
    lbl_1_data_AD4 = HuAudJinglePlay(MSM_STREAM_MGMUS_12);
    fn_1_10970(1);

    player = 0;
    while (player < 4) {
        if (eligible[player] != 0) {
            fn_1_D058(player, 1);
        }
        player++;
    }
    fn_1_D538();

    if (lbl_1_data_AD4 >= 0) {
        player = 1;
        while (player != 0) {
            if (HuAudSStreamStatGet(lbl_1_data_AD4) == 0) {
                player = 0;
            }
            HuPrcVSleep();
        }
    }
    lbl_1_data_ACC = HuAudSStreamPlay(MSM_STREAM_MGMUS_KOOPA);
}

OMOBJ *lbl_1_bss_ACC[4];
MgmbingoInputEvent lbl_1_bss_AA4[10];
ANIMDATA *lbl_1_bss_A84[8];
s16 lbl_1_bss_A82;
s16 lbl_1_bss_A72[8];
s16 lbl_1_bss_A70;
