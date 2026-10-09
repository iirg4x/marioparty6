/* Player input, team scoring, winner selection, and camera control for Garden Grab. */
#include "REL/m635dll.h"
#include "game/audio.h"
#include "game/pad.h"
#include "game/gamework.h"
#include "game/flag.h"
#include <string.h>

extern u16 lbl_1_data_0[6];
extern s16 lbl_1_data_12[4][2];
extern OM_CAMERA_VIEW lbl_1_data_24[3];
M635Work lbl_1_bss_4;
s32 lbl_1_bss_0;

/* Called by the MGSEQ fade-in callback fn_1_134 to play the team intro, set the camera, and enter
 * the player scene. */
void fn_1_408(s16 frameNo)
{
    /* Called from fn_1_134 each fade-in frame; stage the camera and player entrances. */
    OM_CAMERA_VIEW cameraView;
    int playerIndex;
    s16 characterId;

    if (frameNo == 30) {
        if (lbl_1_bss_4.nightF == 0) {
            lbl_1_bss_0 = HuAudFXPlay(M635_SFX_DAY_INTRO);
        } else {
            lbl_1_bss_0 = HuAudFXPlay(M635_SFX_NIGHT_INTRO);
        }
    } else if (frameNo == 90) {
        HuAudFXStop(lbl_1_bss_0);
    }
    switch (lbl_1_bss_4.state) {
        case 0:
            MgSeqModeChangeOff();
            lbl_1_bss_4.state++;
            break;
        case 1:
            if (frameNo == 5) {
                cameraView.center.x = 0.0f;
                cameraView.center.y = 20.0f;
                cameraView.center.z = -400.0f;
                cameraView.rot.x = -25.0f;
                cameraView.rot.y = cameraView.rot.z = 0.0f;
                cameraView.zoom = 2500.0f;
                omCameraViewMoveSimple(&cameraView, 120);
                lbl_1_bss_4.state++;
            }
            break;
        case 2:
            if (omCameraViewCheck(1)) {
                for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                    characterId = lbl_1_bss_BC[playerIndex].charNo;
                    fn_1_30C8(playerIndex, characterId);
                }
                for (playerIndex = 0; playerIndex < 2; playerIndex++) {
                    fn_1_33FC(playerIndex);
                }
                for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                    fn_1_2F40(playerIndex);
                }
                lbl_1_bss_4.state++;
            }
            break;
        case 3:
            MgSeqModeNext();
            lbl_1_bss_4.state = 0;
            break;
    }
}

/* Called from the pre-winner sequence callback after the result wipe; frame the winner or both tied
 * teams. */
void fn_1_638(void)
{
    /* fn_1_2D0 calls this after the wipe-out completes and before the result wipe-in. */
    int playerIndex;
    M635Team *teamState;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        fn_1_3218(playerIndex, 1, 0.0f);
    }
    Center.x = lbl_1_data_24[lbl_1_bss_4.winner + 1].center.x;
    Center.y = lbl_1_data_24[lbl_1_bss_4.winner + 1].center.y;
    Center.z = lbl_1_data_24[lbl_1_bss_4.winner + 1].center.z;
    CRot.x = lbl_1_data_24[lbl_1_bss_4.winner + 1].rot.x;
    CRot.y = lbl_1_data_24[lbl_1_bss_4.winner + 1].rot.y;
    CRot.z = lbl_1_data_24[lbl_1_bss_4.winner + 1].rot.z;
    CZoom = lbl_1_data_24[lbl_1_bss_4.winner + 1].zoom;
    if (lbl_1_bss_4.winner == -1) {
        teamState = lbl_1_bss_4.team;
        fn_1_3340();
        for (playerIndex = 0; playerIndex < 2; playerIndex++) {
            Hu3DModelAttrSet(teamState[playerIndex].teamScoreModel.model, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(teamState[playerIndex].memberOneScoreModel.model, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(teamState[playerIndex].memberZeroScoreModel.model, HU3D_ATTR_DISPOFF);
        }
    } else {
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            if (lbl_1_bss_4.winner != lbl_1_bss_BC[playerIndex].teamNo) {
                Hu3DModelAttrSet(lbl_1_bss_BC[playerIndex].model, HU3D_ATTR_DISPOFF);
            }
        }
    }
}

/* Called by fn_1_2D0 after the result wipe-in completes to move players into result poses. */
void fn_1_8B0(void)
{
    int memberIndex;

    if (lbl_1_bss_4.winner == -1) {
        for (memberIndex = 0; memberIndex < 4; memberIndex++) {
            fn_1_3218(memberIndex, 3, 8.0f);
        }
    } else {
        for (memberIndex = 0; memberIndex < 2; memberIndex++) {
            fn_1_3218(fn_1_32E0(lbl_1_bss_4.winner, memberIndex), 2, 8.0f);
        }
    }
}

/* Called by the MGSEQ main callback fn_1_1AC once per frame to process both teams and detect a
 * winner. */
void fn_1_954(s16 frameNo)
{
    int teamIndex;
    s16 winnerTeam;

    for (teamIndex = 0; teamIndex < 2; teamIndex++) {
        fn_1_9B8(teamIndex);
    }
    winnerTeam = fn_1_D84();
    if (winnerTeam != -1) {
        fn_1_F44(winnerTeam);
    }
}

/* Called from fn_1_954 once per team each gameplay frame to process human or CPU button input. */
void fn_1_9B8(s16 team)
{
    s16 controllerSlots[2];
    s16 playerSlots[2];
    s16 characterIds[2];
    u16 buttonPresses[2];
    M635Team *teamState;
    int memberIndex;
    s16 playerIndex;
    s16 pressTotal;

    teamState = &lbl_1_bss_4.team[team];
    for (memberIndex = 0; memberIndex < 2; memberIndex++) {
        playerIndex = lbl_1_bss_B4[team][memberIndex];
        playerSlots[memberIndex] = lbl_1_bss_BC[playerIndex].playerNo;
        controllerSlots[memberIndex] = lbl_1_bss_BC[playerIndex].padNo;
        characterIds[memberIndex] = lbl_1_bss_BC[playerIndex].charNo;
        if (lbl_1_bss_BC[lbl_1_bss_B4[team][memberIndex]].comF == 0) {
            buttonPresses[memberIndex] = HuPadBtnDown[controllerSlots[memberIndex]];
            buttonPresses[memberIndex] &= (u16)~(PAD_TRIGGER_L | PAD_TRIGGER_R);
        } else {
            buttonPresses[memberIndex] = fn_1_1168(team, memberIndex);
        }
    }
    switch (teamState->inputPhase) {
        case 0:
            if (fn_1_27E4(team, teamState->nextMember) != 3 &&
                buttonPresses[teamState->nextMember] ==
                    lbl_1_data_0[teamState->buttonIndex[teamState->nextMember]]) {
                fn_1_25EC(team, teamState->nextMember, 3);
                fn_1_30C8(playerSlots[teamState->nextMember], characterIds[teamState->nextMember]);
                fn_1_2F40(playerSlots[teamState->nextMember]);
                teamState->alternatingPresses++;
                teamState->nextMember = 1 - teamState->nextMember;
                fn_1_33FC(team);
                if (teamState->alternatingPresses >= 8) {
                    teamState->inputPhase = 1;
                    for (memberIndex = 0; memberIndex < 2; memberIndex++) {
                        teamState->buttonIndex[memberIndex] =
                            fn_1_EC0(teamState->buttonIndex[memberIndex], teamState->inputPhase);
                    }
                } else {
                    teamState->buttonIndex[teamState->nextMember] = fn_1_EC0(
                        teamState->buttonIndex[teamState->nextMember], teamState->inputPhase);
                    fn_1_25EC(team, teamState->nextMember, 1);
                }
            }
            break;
        case 1:
            if (fn_1_27E4(team, 0) != 4 && fn_1_27E4(team, 0) != 3) {
                fn_1_25EC(team, 0, 4);
                fn_1_25EC(team, 1, 4);
            }
            pressTotal = 0;
            for (memberIndex = 0; memberIndex < 2; memberIndex++) {
                if (buttonPresses[memberIndex] ==
                    lbl_1_data_0[teamState->buttonIndex[memberIndex]]) {
                    fn_1_31BC(characterIds[memberIndex]);
                    teamState->pressCount[memberIndex]++;
                }
                pressTotal += teamState->pressCount[memberIndex];
            }
            fn_1_3738(team, pressTotal / 10 + 9);
            break;
    }
}

/* Called from fn_1_954 each main-game frame; return the team at 100 presses, or randomly break a
 * simultaneous tie. */
s16 fn_1_D84(void)
{
    s16 teamPressTotals[2];
    s16 teamIndex;
    s16 memberIndex;

    if (lbl_1_bss_4.winner != -1) {
        return -1;
    }
    for (teamIndex = 0; teamIndex < 2; teamIndex++) {
        teamPressTotals[teamIndex] = 0;
        if (lbl_1_bss_4.team[teamIndex].inputPhase != 0) {
            for (memberIndex = 0; memberIndex < 2; memberIndex++) {
                teamPressTotals[teamIndex] += lbl_1_bss_4.team[teamIndex].pressCount[memberIndex];
            }
        }
    }
    /* If both totals have reached the goal by this check, the game breaks the tie randomly. */
    if (teamPressTotals[0] >= 100 && teamPressTotals[1] >= 100) {
        return frandmod(2);
    }
    if (teamPressTotals[0] >= 100) {
        return 0;
    }
    if (teamPressTotals[1] >= 100) {
        return 1;
    }
    return -1;
}

/* Called by fn_1_9B8 after an accepted alternating press or phase change to choose another button
 * index. */
u16 fn_1_EC0(u16 previousButton, s16 inputPhase)
{
    u16 candidateButton;
    s16 buttonChoiceCount;
    int attempt;

    if (inputPhase == 0) {
        buttonChoiceCount = 6;
    } else {
        buttonChoiceCount = 4;
    }
    /* Five failed retries leave the final candidate in place, even if it repeats the previous
     * button. */
    for (attempt = 0; attempt < 5; attempt++) {
        candidateButton = frandmod(buttonChoiceCount);
        if (candidateButton != previousButton) {
            break;
        }
    }
    return candidateButton;
}

/* Called by fn_1_954 when fn_1_D84 finds a winner; store the result, award non-practice bonuses,
 * and advance MGSEQ. */
void fn_1_F44(s16 team)
{
    s16 winningCharacters[2];
    int memberIndex;
    int firstWinningPlayer;
    int secondWinningPlayer;

    lbl_1_bss_4.winner = team;
    fn_1_33FC(team);
    /* The team-to-player lookup runs before the tie branch, so a -1 tie value also indexes this
     * table here. */
    for (memberIndex = 0; memberIndex < 2; memberIndex++) {
        winningCharacters[memberIndex] = lbl_1_bss_BC[lbl_1_bss_B4[team][memberIndex]].charNo;
    }
    if (team == -1) {
        MgSeqDrawSet();
    } else {
        MgSeqWinnerSet(winningCharacters[0], winningCharacters[1], -1, -1);
        firstWinningPlayer = lbl_1_bss_B4[team][0];
        if (!_CheckFlag(FLAG_MG_PRACTICE)) {
            GwPlayer[firstWinningPlayer].mgCoinBonus = 10;
        }
        secondWinningPlayer = lbl_1_bss_B4[team][1];
        if (!_CheckFlag(FLAG_MG_PRACTICE)) {
            GwPlayer[secondWinningPlayer].mgCoinBonus = 10;
        }
    }
    MgSeqModeNext();
}

/* Called by setup callback fn_1_F0 to reset round state and choose each team's initial
 * alternating-phase button. */
void fn_1_10A0(void)
{
    int teamIndex;
    s16 nightScene;

    memset(&lbl_1_bss_4, 0, sizeof(M635Work));
    memset(lbl_1_bss_BC, 0, sizeof(lbl_1_bss_BC));
    lbl_1_bss_4.winner = -1;
    nightScene = GwMgNightF;
    lbl_1_bss_4.nightF = nightScene;
    for (teamIndex = 0; teamIndex < 2; teamIndex++) {
        lbl_1_bss_4.team[teamIndex].nextMember = 1;
        lbl_1_bss_4.team[teamIndex].buttonIndex[1] = frandmod(6);
    }
}

/* Called from fn_1_9B8 for CPU-controlled members to return a simulated press when their frame
 * timer expires. */
u16 fn_1_1168(s16 team, s16 member)
{
    M635Team *teamState;
    s16 playerIndex;
    s16 cpuDifficulty;

    playerIndex = lbl_1_bss_B4[team][member];
    cpuDifficulty = lbl_1_bss_BC[playerIndex].difficulty;
    teamState = &lbl_1_bss_4.team[team];
    switch (teamState->inputPhase) {
        case 0:
            if (teamState->nextMember != member) {
                return 0;
            }
            if (lbl_1_bss_BC[playerIndex].timer-- <= 0) {
                lbl_1_bss_BC[playerIndex].timer = fn_1_1308(cpuDifficulty);
                return lbl_1_data_0[teamState->buttonIndex[member]];
            }
            break;
        case 1:
            if (lbl_1_bss_BC[playerIndex].timer-- <= 0) {
                lbl_1_bss_BC[playerIndex].timer = fn_1_13F8(cpuDifficulty);
                return lbl_1_data_0[teamState->buttonIndex[member]];
            }
            break;
    }
    return 0;
}

/* Called by fn_1_1168 and player setup to calculate the CPU delay for alternating-button input. */
s16 fn_1_1308(s16 difficulty)
{
    if (difficulty < 0 || difficulty > 3) {
        return 0;
    }
    return 0.75 * lbl_1_data_12[difficulty][0] + frandmod(lbl_1_data_12[difficulty][0] / 2);
}

/* Called by fn_1_1168 to calculate the CPU delay for rapid-press input. */
s16 fn_1_13F8(s16 difficulty)
{
    if (difficulty < 0 || difficulty > 3) {
        return 0;
    }
    return 0.75 * lbl_1_data_12[difficulty][1] + frandmod(lbl_1_data_12[difficulty][1] / 2);
}

/* Debug camera helper: when called, pad 0 adjusts zoom, rotation, and the horizontal/depth camera
 * center. */
void fn_1_14E8(void)
{
    if (HuPadBtn[0] & PAD_BUTTON_UP) {
        CZoom -= 10.0f;
    }
    if (HuPadBtn[0] & PAD_BUTTON_DOWN) {
        CZoom += 10.0f;
    }
    if (HuPadSubStkX[0] > 20) {
        CRot.y += 1.0f;
    }
    if (HuPadSubStkX[0] < -20) {
        CRot.y -= 1.0f;
    }
    if (HuPadSubStkY[0] > 20) {
        CRot.x += 1.0f;
    }
    if (HuPadSubStkY[0] < -20) {
        CRot.x -= 1.0f;
    }
    if (HuPadStkX[0] > 5 || HuPadStkX[0] < -5) {
        Center.x += HuPadStkX[0] / 2;
    }
    if (HuPadStkY[0] > 5 || HuPadStkY[0] < -5) {
        Center.z -= HuPadStkY[0] / 2;
    }
}
