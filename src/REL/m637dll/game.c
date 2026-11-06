/* Implements Slot Trot setup, slot motion, player input, scoring, and results. */
#include <math.h>
#include "REL/m637dll.h"
#include "game/flag.h"
#include "msm_stream.h"
#include "humath.h"

#define M637_ESP_SCORE_FRONT DATANUM(0x9b0000, 30)
#define M637_ESP_SCORE_BACK DATANUM(0x9b0000, 31)

u32 lbl_1_data_28[9] = {
    DATANUM(0x930000, 0), DATANUM(0x930000, 1),
    DATANUM(0x930000, 2), DATANUM(0x8e0000, 106),
    DATANUM(0x8e0000, 107), DATANUM(0x930000, 36),
    DATANUM(0x930000, 37), DATANUM(0x930000, 38),
    DATANUM(0x930000, 40),
};
/* Reversed input plans before aiming normally, from easiest to hardest difficulty. */
s32 lbl_1_data_4C[4] = { 12, 7, 2, 0 };
/* Computer input delays in updates: ordinary plan, alignment correction, unused third entry. */
s32 lbl_1_data_5C[3] = { 15, 25, 1 };
char lbl_1_data_68[13] = "itemhook_sao";

s32 lbl_1_bss_374; /* Round phase: target selection, start, input, score, next round, timeout. */
s32 lbl_1_bss_370; /* Completed rounds; the game ends after five or a team's third point. */
s32 lbl_1_bss_36C; /* -1 before first round; 1 extends its target shuffle; 0 for later rounds. */
s32 lbl_1_bss_368; /* Round-result poses: 0 starts them, 1 waits for the first character. */
s32 lbl_1_bss_364; /* Opening scripted spin phase: forward first, then reverse. */
s32 lbl_1_bss_360; /* Updates elapsed in the current scripted spin phase. */
s32 lbl_1_bss_35C; /* Opening machine-animation updates, including the launch sound cue. */
s32 lbl_1_bss_358; /* Previous symbol for the reel currently being updated. */
s32 lbl_1_bss_354; /* Last input direction: 1 for A, 2 for B; shared by reel tick sounds. */
s32 lbl_1_bss_350; /* Background-music handle; -1 until playback starts. */
MGTIMER *lbl_1_bss_34C; /* Fifteen-second round timer. */
OM_CAMERA_VIEW lbl_1_bss_330; /* Opening, play, or winner view selected by the sequence. */
M637Record2A0 lbl_1_bss_2A0[4]; /* Character state, with each team's teammates adjacent. */
s32 lbl_1_bss_290[4]; /* Original player indices reordered into two adjacent teams. */
M637Record1D0 lbl_1_bss_1D0[2]; /* Left and right teams' controllable reels. */
M637Record118 lbl_1_bss_118[2]; /* Target for the first and second player of each team. */
s32 lbl_1_bss_114; /* Target-symbol face-turn animation step, 0-4. */
s32 lbl_1_bss_110; /* Target selections started in the current shuffle, including any face turn in
                    * progress. */
s16 lbl_1_bss_10E; /* Central model positioned 80 units below the machine's item hook. */
s16 lbl_1_bss_10C; /* Central machine model raised and lowered during sequence transitions. */
s16 lbl_1_bss_10A; /* Central effect model displayed during scored-round poses. */
s16 lbl_1_bss_108; /* Looping motion for the central scored-round effect. */
s32 lbl_1_bss_104; /* Scene-setup matches in the current selection attempt; not recomputed after
                    * replacements. */
f32 lbl_1_bss_100; /* Vertical offset used by the machine's opening and exit movement. */
f32 lbl_1_bss_F8[2]; /* Opening drop and lift sine angles in degrees. */
M637RecordB8 lbl_1_bss_B8[2]; /* Left and right score panels. */
M637Record08 lbl_1_bss_8[4]; /* Computer input plans in team order. */

/* Called during fn_1_F0 scene setup to build the play state and choose each team's reel starts. */
void fn_1_548(void)
{
    fn_1_181C();
    /* Both teams begin each reel on the same randomly chosen symbol. */
    lbl_1_bss_1D0->reelSymbol[0] = lbl_1_bss_1D0[1].reelSymbol[0] = lbl_1_bss_118->targetSymbol =
        frandmod(8);
    lbl_1_bss_1D0->reelSymbol[1] = lbl_1_bss_1D0[1].reelSymbol[1] = lbl_1_bss_118[1].targetSymbol =
        frandmod(8);
    fn_1_1CF0();
    fn_1_2D7C();
    fn_1_3460();
    lbl_1_bss_374 = 0;
    lbl_1_bss_370 = 0;
    lbl_1_bss_36C = -1;
    lbl_1_bss_364 = 0;
    lbl_1_bss_360 = 0;
    lbl_1_bss_35C = 0;
    lbl_1_bss_358 = 0;
    lbl_1_bss_354 = 0;
    lbl_1_bss_350 = -1;
}

/* Called by the round sequence callback to advance countdown, play, and result states. */
s32 fn_1_65C(void)
{
    s32 playerIndex;
    s32 gameEnded;

    gameEnded = 0;
    switch (lbl_1_bss_374) {
    case 0:
        if (lbl_1_bss_36C == -1) {
            lbl_1_bss_36C = 1;
        }
        if (lbl_1_bss_36C != 0) {
            fn_1_5914(1);
        }
        if (fn_1_4C24((lbl_1_bss_36C * 5) + 10) != 0) {
            lbl_1_bss_34C = MgTimerCreate(0);
            MgTimerParamSet(lbl_1_bss_34C, 900, 0, 0);
            MgTimerPosSet(lbl_1_bss_34C, 288.0f, 410.0f);
            lbl_1_bss_36C = 0;
            lbl_1_bss_374 += 1;
            HuAudFXPlay(MSM_SE_MG637_ROUND_TIMER_START);
        }
        break;
    case 1:
        MgTimerModeOnSet(lbl_1_bss_34C, 1);
        playerIndex = 0;
        while (playerIndex < 4) {
            /* Reset the computer player's input plan and seed its detours from difficulty. */
            lbl_1_bss_8[playerIndex].inputPlanActive = 0;
            lbl_1_bss_8[playerIndex].remainingDetours =
                lbl_1_data_4C[lbl_1_bss_8[playerIndex].difficulty];
            fn_8006A030((s16) playerIndex, 20, 7, 3);
            playerIndex += 1;
        }
        lbl_1_bss_374 += 1;
        break;
    case 2:
        fn_1_6BD4();
        if (MgTimerDoneCheck(lbl_1_bss_34C) != 0) {
            lbl_1_bss_368 = 0;
            lbl_1_bss_370 += 1;
            lbl_1_bss_374 += 3;
        } else if (fn_1_36E8(1) != 0) {
            lbl_1_bss_368 = 0;
            MgTimerModeOffSet(lbl_1_bss_34C);
            lbl_1_bss_370 += 1;
            lbl_1_bss_374 += 1;
        }
        break;
    case 3:
        if ((fn_1_36E8(0) != 0) && (fn_1_603C() != 0)) {
            lbl_1_bss_374 += 1;
        }
        break;
    case 4:
        if ((lbl_1_bss_370 == 5) || (lbl_1_bss_2A0->teamScore == 3) ||
            (lbl_1_bss_2A0[2].teamScore == 3)) {
            MgTimerKill(lbl_1_bss_34C);
            gameEnded = 1;
            HuAudSStreamFadeOut(lbl_1_bss_350, 100);
        } else if (fn_1_4C24(5) != 0) {
            MgTimerKill(lbl_1_bss_34C);
            lbl_1_bss_374 = 0;
        }
        break;
    case 5:
        if ((fn_1_36E8(0) != 0) && (fn_1_65A0() != 0)) {
            lbl_1_bss_374 -= 1;
        }
        break;
    }
    return gameEnded;
}

/* During fade-in, drives only the left team's reels forward for 59 updates, then lets them coast
 * to a stop. It then drives them in reverse for 59 updates before coasting again; each call runs
 * the settling update for both teams. */
void fn_1_A54(void)
{
    if (lbl_1_bss_364 == 0) {
        lbl_1_bss_354 = 1;
        if (++lbl_1_bss_360 < 60) {
            lbl_1_bss_1D0->reelSpeed[0] = 4.0f;
            lbl_1_bss_1D0->reelSpeed[1] = 4.0f;
        }
        if (fn_1_36E8(0) != 0) {
            lbl_1_bss_364 += 1;
            lbl_1_bss_360 = 0;
            return;
        }
    } else {
        lbl_1_bss_354 = 2;
        if (++lbl_1_bss_360 < 60) {
            lbl_1_bss_1D0->reelSpeed[0] = -4.0f;
            lbl_1_bss_1D0->reelSpeed[1] = -4.0f;
        }
        fn_1_36E8(0);
    }
}

/* Called each frame of the finish sequence to register the winner; outside practice, sets each
 * winning teammate's bonus-coin total to ten, replacing its current value. */
void fn_1_B94(void)
{
    s32 leftWinnerFirst;
    s32 leftWinnerSecond;
    s32 rightWinnerFirst;
    s32 rightWinnerSecond;

    fn_1_36E8(0);
    if (lbl_1_bss_2A0->teamScore > lbl_1_bss_2A0[2].teamScore) {
        fn_80071C18(lbl_80265728[lbl_1_bss_290[0]].charNo, lbl_80265728[lbl_1_bss_290[1]].charNo,
                       -1, -1);
        leftWinnerFirst = lbl_1_bss_290[0];
        if (fn_8006A124(65551) == 0) {
            lbl_80265750[leftWinnerFirst].mgCoinBonus = 10;
        }
        leftWinnerSecond = lbl_1_bss_290[1];
        if (fn_8006A124(65551) == 0) {
            lbl_80265750[leftWinnerSecond].mgCoinBonus = 10;
        }
        fn_8005DFC0(lbl_80265728[lbl_1_bss_290[0]].charNo, 1);
        fn_8005DFC0(lbl_80265728[lbl_1_bss_290[1]].charNo, 1);
        return;
    }
    if (lbl_1_bss_2A0->teamScore < lbl_1_bss_2A0[2].teamScore) {
        fn_80071C18(lbl_80265728[lbl_1_bss_290[2]].charNo, lbl_80265728[lbl_1_bss_290[3]].charNo,
                       -1, -1);
        rightWinnerFirst = lbl_1_bss_290[2];
        if (fn_8006A124(65551) == 0) {
            lbl_80265750[rightWinnerFirst].mgCoinBonus = 10;
        }
        rightWinnerSecond = lbl_1_bss_290[3];
        if (fn_8006A124(65551) == 0) {
            lbl_80265750[rightWinnerSecond].mgCoinBonus = 10;
        }
        fn_8005DFC0(lbl_80265728[lbl_1_bss_290[2]].charNo, 1);
        fn_8005DFC0(lbl_80265728[lbl_1_bss_290[3]].charNo, 1);
        return;
    }
    fn_80071C18(-1, -1, -1, -1);
}

/* Called when the pre-winner countdown ends to pose winning teammates, or all players on a draw. */
void fn_1_E4C(void)
{
    if (lbl_1_bss_2A0->teamScore > lbl_1_bss_2A0[2].teamScore) {
        fn_8005C2C8(lbl_80265728[lbl_1_bss_290[0]].charNo, lbl_1_bss_2A0->motion[7], 0.0f,
                           8.0f, 0U);
        fn_8005C2C8(lbl_80265728[lbl_1_bss_290[1]].charNo, lbl_1_bss_2A0[1].motion[7], 0.0f,
                           8.0f, 0U);
        return;
    }
    if (lbl_1_bss_2A0->teamScore < lbl_1_bss_2A0[2].teamScore) {
        fn_8005C2C8(lbl_80265728[lbl_1_bss_290[2]].charNo, lbl_1_bss_2A0[2].motion[7], 0.0f,
                           8.0f, 0U);
        fn_8005C2C8(lbl_80265728[lbl_1_bss_290[3]].charNo, lbl_1_bss_2A0[3].motion[7], 0.0f,
                           8.0f, 0U);
        return;
    }
    fn_8005C2C8(lbl_80265728[lbl_1_bss_290[0]].charNo, lbl_1_bss_2A0->motion[8], 0.0f, 8.0f,
                       0U);
    fn_8005C2C8(lbl_80265728[lbl_1_bss_290[1]].charNo, lbl_1_bss_2A0[1].motion[8], 0.0f,
                       8.0f, 0U);
    fn_8005C2C8(lbl_80265728[lbl_1_bss_290[2]].charNo, lbl_1_bss_2A0[2].motion[8], 0.0f,
                       8.0f, 0U);
    fn_8005C2C8(lbl_80265728[lbl_1_bss_290[3]].charNo, lbl_1_bss_2A0[3].motion[8], 0.0f,
                       8.0f, 0U);
}

/* Called by scene setup and sequence callbacks to choose the opening, play, or winner camera view.
 * Opening view is set immediately and returns 0. Play and winning-team views start a new
 * 100-frame move and return 100, even if already selected; a draw or other mode returns 0. */
s32 fn_1_1110(s32 viewMode)
{
    s32 moveCamera;
    s32 moveDuration;
    HuVecF *openingCenter;
    HuVecF *openingRotation;
    HuVecF *sceneCenter;
    HuVecF *sceneRotation;
    HuVecF *leftTeamCenter;
    HuVecF *leftTeamRotation;
    HuVecF *rightTeamCenter;
    HuVecF *rightTeamRotation;

    moveDuration = 0;
    moveCamera = 0;
    if (viewMode == 0) {
        {
            HuVecF openingCenterValue = { -400.0f, -20.0f, 0.0f };
            openingCenter = &openingCenterValue;
            lbl_1_bss_330.center = *openingCenter;
        }
        {
            HuVecF openingRotationValue = { 0.0f, 40.0f, 0.0f };
            openingRotation = &openingRotationValue;
            lbl_1_bss_330.rot = *openingRotation;
        }
        lbl_1_bss_330.zoom = 1000.0f;
        fn_8003E5A4(&lbl_1_bss_330);
    } else if (viewMode == 1) {
        {
            HuVecF sceneCenterValue = { 0.0f, 0.0f, 0.0f };
            sceneCenter = &sceneCenterValue;
            lbl_1_bss_330.center = *sceneCenter;
        }
        {
            HuVecF sceneRotationValue = { -3.5511f, 0.0f, 0.0f };
            sceneRotation = &sceneRotationValue;
            lbl_1_bss_330.rot = *sceneRotation;
        }
        lbl_1_bss_330.zoom = 1550.0f;
        moveCamera = 1;
    } else if (viewMode == 2) {
        if (lbl_1_bss_2A0->teamScore > lbl_1_bss_2A0[2].teamScore) {
            {
                HuVecF leftTeamCenterValue = { -400.0f, -20.0f, 0.0f };
                leftTeamCenter = &leftTeamCenterValue;
                lbl_1_bss_330.center = *leftTeamCenter;
            }
            {
                HuVecF leftTeamRotationValue = { -15.0f, 40.0f, 0.0f };
                leftTeamRotation = &leftTeamRotationValue;
                lbl_1_bss_330.rot = *leftTeamRotation;
            }
            lbl_1_bss_330.zoom = 800.0f;
            moveCamera = 1;
        } else if (lbl_1_bss_2A0->teamScore < lbl_1_bss_2A0[2].teamScore) {
            {
                HuVecF rightTeamCenterValue = { 400.0f, -20.0f, 0.0f };
                rightTeamCenter = &rightTeamCenterValue;
                lbl_1_bss_330.center = *rightTeamCenter;
            }
            {
                HuVecF rightTeamRotationValue = { -15.0f, -40.0f, 0.0f };
                rightTeamRotation = &rightTeamRotationValue;
                lbl_1_bss_330.rot = *rightTeamRotation;
            }
            lbl_1_bss_330.zoom = 800.0f;
            moveCamera = 1;
        }
    }
    if (moveCamera != 0) {
        fn_8003EA54(&lbl_1_bss_330, 100);
        moveDuration = 100;
    }
    return moveDuration;
}

/* Called by sequence modes to animate the slot machine and align reel markers. */
void fn_1_1464(s32 animationMode)
{
    HuVecF objectPosition;
    s32 targetDisplayIndex;
    s32 symbolIndex;

    if (animationMode != 0) {
        if (animationMode == 1) {
            if (++lbl_1_bss_35C == 56) {
                HuAudFXPlay(MSM_SE_MG637_SLOT_MACHINE_ANIMATION);
            }
            lbl_1_bss_100 =
                (f32) (650.0 * sin((M_PI * (f64) lbl_1_bss_F8[0]) / 180.0));
            fn_80026760(lbl_1_bss_10C, 0.0f, 950.0f - lbl_1_bss_100, -300.0f);
            lbl_1_bss_F8[0] += 0.9f;
        } else if (animationMode == 2) {
            if (lbl_1_bss_F8[1] <= 90.0f) {
                lbl_1_bss_100 =
                    (f32) (50.0 * sin((M_PI * (f64) lbl_1_bss_F8[1]) / 180.0));
                fn_80026760(lbl_1_bss_10C, 0.0f, 300.0f + lbl_1_bss_100, -300.0f);
                lbl_1_bss_F8[1] += 1.125f;
                if (lbl_1_bss_F8[1] > 90.0f) {
                    /* The exit movement uses a 600-unit offset after the 50-unit lift. */
                    lbl_1_bss_100 = 600.0f;
                }
            }
            if (lbl_1_bss_350 == -1) {
            lbl_1_bss_350 = HuAudBGMPlay(MSM_STREAM_MGMUS_25);
            }
        } else {
            lbl_1_bss_100 -= 6.0f;
            fn_80026760(lbl_1_bss_10C, 0.0f, 950.0f - lbl_1_bss_100, -300.0f);
        }
    }
    Hu3DModelObjPosGet(lbl_1_bss_10C, lbl_1_data_68, &objectPosition);
    objectPosition.y -= 80.0f;
    for (targetDisplayIndex = 0; targetDisplayIndex < 2; targetDisplayIndex++) {
        symbolIndex = 0;
        while (symbolIndex < 8) {
            fn_80026790(lbl_1_bss_118[targetDisplayIndex].symbolModels[symbolIndex][0],
                             &objectPosition);
            fn_80026790(lbl_1_bss_118[targetDisplayIndex].symbolModels[symbolIndex][1],
                             &objectPosition);
            symbolIndex += 1;
        }
    }
    fn_80026790(lbl_1_bss_10E, &objectPosition);
    fn_80026790(lbl_1_bss_10A, &objectPosition);
}

/* Queried by sequence callbacks for the 100-frame opening and result transition duration. */
s32 fn_1_1814(void)
{
    return 100;
}

/* Called by fn_1_548 during scene setup to build all four character models and motions. */
void fn_1_181C(void)
{
    f32 combinedTeamOffset;
    s32 characterIndex;
    s32 motionIndex;
    s16 characterId;
    s32 leftTeamCount;
    s32 rightTeamWriteIndex;
    f32 teamAngle;
    f32 playerOffset;
    f32 sideOffset;
    f32 teamSidePosition;
    f32 playerSideOffset;
    f32 teamYawDegrees;

    leftTeamCount = 0;
    rightTeamWriteIndex = 2;
    characterIndex = 0;
    while (characterIndex < 4) {
        if (lbl_80265728[characterIndex].grpNo == 0) {
            lbl_1_bss_290[leftTeamCount++] = characterIndex;
        } else {
            lbl_1_bss_290[rightTeamWriteIndex++] = characterIndex;
        }
        characterIndex += 1;
    }
    for (characterIndex = 0; characterIndex < 4; characterIndex++) {
        characterId = lbl_80265728[lbl_1_bss_290[characterIndex]].charNo;
        if (characterIndex / 2 != 0) {
            teamSidePosition = 400.0f;
        } else {
            teamSidePosition = -400.0f;
        }
        sideOffset = teamSidePosition;
        if (characterIndex % 2 != 0) {
            playerSideOffset = 125.0f;
        } else {
            playerSideOffset = -125.0f;
        }
        playerOffset = playerSideOffset;
        /* The combined offset is calculated here, but placement below uses its two parts. */
        combinedTeamOffset = sideOffset + playerOffset;
        if (characterIndex < 2) {
            teamYawDegrees = 40.0f;
        } else {
            teamYawDegrees = -40.0f;
        }
        teamAngle = teamYawDegrees;
        lbl_1_bss_2A0[characterIndex].model = fn_80057068(characterId, (1 << 1));
        fn_80026760(lbl_1_bss_2A0[characterIndex].model,
                        (f32) ((f64) sideOffset + ((f64) playerOffset *
                                                   cos((M_PI * -teamAngle) / 180.0))),
                        -20.0f,
                        (f32) (playerOffset * sin((M_PI * -teamAngle) / 180.0)));
        fn_80026808(lbl_1_bss_2A0[characterIndex].model, 0.0f, teamAngle, 0.0f);
        fn_80026B64(lbl_1_bss_2A0[characterIndex].model, 1U);
        fn_800269E8(lbl_1_bss_2A0[characterIndex].model, ((1 << 30) | (1 << 0)));
        fn_80026E44(lbl_1_bss_2A0[characterIndex].model);
        motionIndex = 0;
        while (motionIndex < 9) {
            lbl_1_bss_2A0[characterIndex].motion[motionIndex] =
                fn_8005B0B8(characterId, lbl_1_data_28[motionIndex]);
            motionIndex += 1;
        }
        lbl_1_bss_2A0[characterIndex].selectedMotion = lbl_1_bss_2A0[characterIndex].motion[0];
        fn_8005BE80(characterId, lbl_1_bss_2A0[characterIndex].selectedMotion);
        fn_8005B4BC(characterId);
        fn_8005DFC0(characterId, 0);
        lbl_1_bss_2A0[characterIndex].idleInputFrames = 0;
        lbl_1_bss_2A0[characterIndex].teamScore = 0;
        lbl_1_bss_8[characterIndex].difficulty =
            (s32) lbl_80265728[lbl_1_bss_290[characterIndex]].comDif;
        lbl_1_bss_8[characterIndex].inputDelayFrames = lbl_1_data_5C[0];
        lbl_1_bss_8[characterIndex].targetReelIndex = characterIndex % 2;
        lbl_1_bss_8[characterIndex].teamIndex = characterIndex / 2;
        lbl_1_bss_8[characterIndex].teamReelIndex = characterIndex % 2;
    }
    fn_8000DAC0(fn_1_67C4, 100U, 8192U, 0, fn_8000DD9C());
}

/* Called by fn_1_548 to create each team's reels, stop controls, and result decorations. */
void fn_1_1CF0(void)
{
    s32 teamIndex;
    s32 slotIndex;
    s16 modelDataNumber;
    s16 teamModel;
    f32 teamRotationY;
    f32 teamPositionX;

    for (teamIndex = 0; teamIndex < 2; teamIndex++) {
        if (teamIndex == 0) {
            modelDataNumber = 4;
            teamPositionX = -400.0f;
            teamRotationY = 40.0f;
        } else {
            modelDataNumber = 5;
            teamPositionX = 400.0f;
            teamRotationY = -40.0f;
        }
        teamModel = fn_80025A04(
            fn_80008DD0(modelDataNumber + 0x600000, 0x10000000, 2));
        fn_80026B64(teamModel, 1U);
        fn_80026760(teamModel, teamPositionX, -200.0f, 0.0f);
        fn_80026808(teamModel, 0.0f, teamRotationY, 0.0f);
        fn_80026B8C(teamModel, 3);
        slotIndex = 0;
        while (slotIndex < 2) {
            modelDataNumber = slotIndex == 0 ? 21 : 23;
            lbl_1_bss_1D0[teamIndex].reelModels[slotIndex][0] = fn_80025A04(
                fn_80008DD0(modelDataNumber + 0x600000, 0x10000000, 2));
            fn_80026B64(lbl_1_bss_1D0[teamIndex].reelModels[slotIndex][0], 1U);
            fn_80026760(lbl_1_bss_1D0[teamIndex].reelModels[slotIndex][0], teamPositionX,
                            -200.0f, 0.0f);
            fn_80026808(lbl_1_bss_1D0[teamIndex].reelModels[slotIndex][0], 0.0f, teamRotationY,
                            0.0f);
            fn_80026B8C(lbl_1_bss_1D0[teamIndex].reelModels[slotIndex][0], 3);
            fn_80026FFC(lbl_1_bss_1D0[teamIndex].reelModels[slotIndex][0]);
            modelDataNumber = slotIndex == 0 ? 20 : 22;
            lbl_1_bss_1D0[teamIndex].reelModels[slotIndex][1] = fn_80025A04(
                fn_80008DD0(modelDataNumber + 0x600000, 0x10000000, 2));
            fn_80026B64(lbl_1_bss_1D0[teamIndex].reelModels[slotIndex][1], 1U);
            fn_80026760(lbl_1_bss_1D0[teamIndex].reelModels[slotIndex][1], teamPositionX,
                            -200.0f, 0.0f);
            fn_80026808(lbl_1_bss_1D0[teamIndex].reelModels[slotIndex][1], 0.0f, teamRotationY,
                            0.0f);
            fn_80026B8C(lbl_1_bss_1D0[teamIndex].reelModels[slotIndex][1], 3);
            fn_80026FFC(lbl_1_bss_1D0[teamIndex].reelModels[slotIndex][1]);
            if (((s32) lbl_1_bss_1D0[teamIndex].reelSymbol[slotIndex] == 0) ||
                ((s32) lbl_1_bss_1D0[teamIndex].reelSymbol[slotIndex] >= 5)) {
                lbl_1_bss_1D0[teamIndex].visibleReelModel[slotIndex] = 0;
                fn_800269E8(lbl_1_bss_1D0[teamIndex].reelModels[slotIndex][1],
                                 (1 << 0));
            } else {
                lbl_1_bss_1D0[teamIndex].visibleReelModel[slotIndex] = 1;
                fn_800269E8(lbl_1_bss_1D0[teamIndex].reelModels[slotIndex][0],
                                 (1 << 0));
            }
            lbl_1_bss_1D0[teamIndex].modelAngle[slotIndex] =
                -60.0f * (f32) lbl_1_bss_1D0[teamIndex].reelSymbol[slotIndex];
            if (lbl_1_bss_1D0[teamIndex].modelAngle[slotIndex] <= -240.0f) {
                lbl_1_bss_1D0[teamIndex].modelAngle[slotIndex] += 240.0f;
            }
            if (lbl_1_bss_1D0[teamIndex].modelAngle[slotIndex] <= -30.0f) {
                lbl_1_bss_1D0[teamIndex].modelAngle[slotIndex] += 240.0f;
            }
            if (lbl_1_bss_1D0[teamIndex].modelAngle[slotIndex] >= 210.0f) {
                lbl_1_bss_1D0[teamIndex].modelAngle[slotIndex] -= 240.0f;
            }
            fn_80026808(
                lbl_1_bss_1D0[teamIndex]
                    .reelModels[slotIndex][lbl_1_bss_1D0[teamIndex].visibleReelModel[slotIndex]],
                -lbl_1_bss_1D0[teamIndex].modelAngle[slotIndex], teamRotationY, 0.0f);
            lbl_1_bss_1D0[teamIndex].symbolAngle[slotIndex] =
                -60.0f * (f32) lbl_1_bss_1D0[teamIndex].reelSymbol[slotIndex];
            lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex] = 0.0f;
            lbl_1_bss_1D0[teamIndex].alignmentState[slotIndex] = 1;
            slotIndex += 1;
        }
        lbl_1_bss_1D0[teamIndex].spinEffectModel =
            fn_80025A04(fn_80008DD0(DATANUM(0x600000, 6), 0x10000000, 2));
        fn_80026B64(lbl_1_bss_1D0[teamIndex].spinEffectModel, 1U);
        fn_80026760(lbl_1_bss_1D0[teamIndex].spinEffectModel, teamPositionX, -200.0f, 0.0f);
        fn_80026808(lbl_1_bss_1D0[teamIndex].spinEffectModel, 0.0f, teamRotationY, 0.0f);
        fn_80026B8C(lbl_1_bss_1D0[teamIndex].spinEffectModel, 3);
        fn_800269E8(lbl_1_bss_1D0[teamIndex].spinEffectModel, ((1 << 30) | (1 << 0)));
        fn_8002D86C(lbl_1_bss_1D0[teamIndex].spinEffectModel, 0.0f);
        modelDataNumber = teamIndex == 0 ? 7 : 8;
        lbl_1_bss_1D0[teamIndex].teamAnimationModel = fn_80025A04(
            fn_80008DD0(modelDataNumber + 0x600000, 0x10000000, 2));
        fn_80026B64(lbl_1_bss_1D0[teamIndex].teamAnimationModel, 1U);
        fn_80026760(lbl_1_bss_1D0[teamIndex].teamAnimationModel, teamPositionX, -200.0f, 0.0f);
        fn_80026808(lbl_1_bss_1D0[teamIndex].teamAnimationModel, 0.0f, teamRotationY, 0.0f);
        fn_80026B8C(lbl_1_bss_1D0[teamIndex].teamAnimationModel, 3);
        lbl_1_bss_1D0[teamIndex].idleMotion =
            fn_80030458(lbl_1_bss_1D0[teamIndex].teamAnimationModel,
                            fn_80008DD0(DATANUM(0x600000, 9), 0x10000000, 2));
        lbl_1_bss_1D0[teamIndex].spinMotion = fn_80030458(
            lbl_1_bss_1D0[teamIndex].teamAnimationModel,
            fn_80008DD0(DATANUM(0x600000, 10), 0x10000000, 2));
        lbl_1_bss_1D0[teamIndex].scoreMotion = fn_80030458(
            lbl_1_bss_1D0[teamIndex].teamAnimationModel,
            fn_80008DD0(DATANUM(0x600000, 11), 0x10000000, 2));
        fn_8002C95C(lbl_1_bss_1D0[teamIndex].teamAnimationModel,
                      lbl_1_bss_1D0[teamIndex].idleMotion);
        fn_800269E8(lbl_1_bss_1D0[teamIndex].teamAnimationModel, ((1 << 30) | (1 << 0)));
        lbl_1_bss_1D0[teamIndex].spinAnimationActive = 0;
        slotIndex = 0;
        while (slotIndex < 2) {
            if (teamIndex == 0) {
                modelDataNumber = slotIndex == 0 ? 12 : 13;
            } else {
                modelDataNumber = slotIndex == 0 ? 14 : 15;
            }
            lbl_1_bss_1D0[teamIndex].reelAnimationModels[slotIndex] = fn_80025A04(
                fn_80008DD0(modelDataNumber + 0x600000, 0x10000000, 2));
            fn_80026B64(lbl_1_bss_1D0[teamIndex].reelAnimationModels[slotIndex], 1U);
            fn_80026760(lbl_1_bss_1D0[teamIndex].reelAnimationModels[slotIndex], teamPositionX,
                            -200.0f, 0.0f);
            fn_80026808(lbl_1_bss_1D0[teamIndex].reelAnimationModels[slotIndex], 0.0f,
                            teamRotationY, 0.0f);
            fn_80026B8C(lbl_1_bss_1D0[teamIndex].reelAnimationModels[slotIndex], 3);
            fn_800269E8(lbl_1_bss_1D0[teamIndex].reelAnimationModels[slotIndex],
                             ((1 << 30) | (1 << 0)));
            modelDataNumber = slotIndex == 0 ? 16 : 17;
            lbl_1_bss_1D0[teamIndex].reelAnimationMotions[slotIndex][0] = fn_80030458(
                lbl_1_bss_1D0[teamIndex].reelAnimationModels[slotIndex],
                fn_80008DD0(modelDataNumber + 0x600000, 0x10000000, 2));
            modelDataNumber = slotIndex == 0 ? 18 : 19;
            lbl_1_bss_1D0[teamIndex].reelAnimationMotions[slotIndex][1] = fn_80030458(
                lbl_1_bss_1D0[teamIndex].reelAnimationModels[slotIndex],
                fn_80008DD0(modelDataNumber + 0x600000, 0x10000000, 2));
            fn_8002C95C(lbl_1_bss_1D0[teamIndex].reelAnimationModels[slotIndex],
                          lbl_1_bss_1D0[teamIndex].reelAnimationMotions[slotIndex][0]);
            fn_800269E8(lbl_1_bss_1D0[teamIndex].reelAnimationModels[slotIndex],
                             ((1 << 30) | (1 << 0)));
            slotIndex += 1;
        }
        slotIndex = 0;
        while (slotIndex < 2) {
            modelDataNumber = slotIndex == 0 ? 58 : 60;
            lbl_1_bss_1D0[teamIndex].matchIndicatorModels[slotIndex] = fn_80025A04(
                fn_80008DD0(modelDataNumber + 0x600000, 0x10000000, 2));
            fn_80026B64(lbl_1_bss_1D0[teamIndex].matchIndicatorModels[slotIndex], 1U);
            fn_80026760(lbl_1_bss_1D0[teamIndex].matchIndicatorModels[slotIndex], teamPositionX,
                            -200.0f, 0.0f);
            fn_80026808(lbl_1_bss_1D0[teamIndex].matchIndicatorModels[slotIndex], 0.0f,
                            teamRotationY, 0.0f);
            fn_80026B8C(lbl_1_bss_1D0[teamIndex].matchIndicatorModels[slotIndex], 3);
            modelDataNumber = slotIndex == 0 ? 59 : 61;
            lbl_1_bss_1D0[teamIndex].matchIndicatorMotions[slotIndex] = fn_80030458(
                lbl_1_bss_1D0[teamIndex].matchIndicatorModels[slotIndex],
                fn_80008DD0(modelDataNumber + 0x600000, 0x10000000, 2));
            fn_8002C95C(lbl_1_bss_1D0[teamIndex].matchIndicatorModels[slotIndex],
                          lbl_1_bss_1D0[teamIndex].matchIndicatorMotions[slotIndex]);
            fn_800269E8(lbl_1_bss_1D0[teamIndex].matchIndicatorModels[slotIndex],
                             ((1 << 30) | (1 << 0)));
            fn_800269E8(lbl_1_bss_1D0[teamIndex].matchIndicatorModels[slotIndex],
                             (1 << 0));
            slotIndex += 1;
        }
        lbl_1_bss_1D0[teamIndex].teamMatchEffectModel = fn_80025A04(
            fn_80008DD0(DATANUM(0x600000, 62), 0x10000000, 2));
        fn_80026B64(lbl_1_bss_1D0[teamIndex].teamMatchEffectModel, 1U);
        fn_80026760(lbl_1_bss_1D0[teamIndex].teamMatchEffectModel, teamPositionX, -200.0f,
                        0.0f);
        fn_80026808(lbl_1_bss_1D0[teamIndex].teamMatchEffectModel, 0.0f, teamRotationY, 0.0f);
        fn_80026B8C(lbl_1_bss_1D0[teamIndex].teamMatchEffectModel, 3);
        lbl_1_bss_1D0[teamIndex].teamMatchEffectMotion = fn_80030458(
            lbl_1_bss_1D0[teamIndex].teamMatchEffectModel,
            fn_80008DD0(DATANUM(0x600000, 63), 0x10000000, 2));
        fn_8002C95C(lbl_1_bss_1D0[teamIndex].teamMatchEffectModel,
                      lbl_1_bss_1D0[teamIndex].teamMatchEffectMotion);
        fn_800269E8(lbl_1_bss_1D0[teamIndex].teamMatchEffectModel, ((1 << 30) | (1 << 0)));
        fn_800269E8(lbl_1_bss_1D0[teamIndex].teamMatchEffectModel, (1 << 0));
    }
}

/* Called during fn_1_548 setup to create the central machine and its two shared target displays. */
void fn_1_2D7C(void)
{
    HuVecF reelSymbolPosition;
    s32 targetDisplayIndex;
    s32 symbolIndex;
    s32 symbolModelBase;
    s16 symbolDataBase;
    s16 symbolOverlayDataBase;

    lbl_1_bss_10C =
        fn_80025A04(fn_80008DD0(DATANUM(0x600000, 57), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_10C, 1U);
    fn_80026760(lbl_1_bss_10C, 0.0f, 950.0f, -300.0f);
    fn_80026808(lbl_1_bss_10C, 0.0f, 0.0f, 0.0f);
    fn_800269E8(lbl_1_bss_10C, ((1 << 30) | (1 << 0)));
    fn_80026B8C(lbl_1_bss_10C, 2);
    fn_800268B0(lbl_1_bss_10C, 1.3f, 1.3f, 1.0f);
    for (targetDisplayIndex = 0; targetDisplayIndex < 2; targetDisplayIndex++) {
        lbl_1_bss_118[targetDisplayIndex].openingTargetSymbol =
            lbl_1_bss_118[targetDisplayIndex].targetSymbol;
        lbl_1_bss_118[targetDisplayIndex].lastChosenSymbol =
            lbl_1_bss_118[targetDisplayIndex].targetSymbol;
        lbl_1_bss_118[targetDisplayIndex].initialSymbol =
            lbl_1_bss_118[targetDisplayIndex].targetSymbol;
        symbolIndex = 0;
        while (symbolIndex < 8) {
            lbl_1_bss_118[targetDisplayIndex].symbolSetupHeight[symbolIndex] = 950.0f;
            Hu3DModelObjPosGet(lbl_1_bss_10C, lbl_1_data_68, &reelSymbolPosition);
            /* Place the symbols below the reel object's reference point. */
            reelSymbolPosition.y -= 280.0f;
            if (targetDisplayIndex == 0) {
                symbolDataBase = 25;
            } else {
                symbolDataBase = 41;
            }
            symbolModelBase = symbolDataBase;
            lbl_1_bss_118[targetDisplayIndex].symbolModels[symbolIndex][0] =
                fn_80025A04(fn_80008DD0(symbolModelBase + 0x600000 + symbolIndex,
                                                     0x10000000, 2));
            fn_80026B64(lbl_1_bss_118[targetDisplayIndex].symbolModels[symbolIndex][0], 1U);
            fn_80026790(lbl_1_bss_118[targetDisplayIndex].symbolModels[symbolIndex][0],
                             &reelSymbolPosition);
            fn_80026808(lbl_1_bss_118[targetDisplayIndex].symbolModels[symbolIndex][0], 0.0f,
                            0.0f, 0.0f);
            fn_80026B8C(lbl_1_bss_118[targetDisplayIndex].symbolModels[symbolIndex][0], 2);
            if (symbolIndex != lbl_1_bss_118[targetDisplayIndex].targetSymbol) {
                fn_800269E8(lbl_1_bss_118[targetDisplayIndex].symbolModels[symbolIndex][0],
                                 (1 << 0));
            }
            if (targetDisplayIndex == 0) {
                symbolOverlayDataBase = 33;
            } else {
                symbolOverlayDataBase = 49;
            }
            symbolModelBase = symbolOverlayDataBase;
            lbl_1_bss_118[targetDisplayIndex].symbolModels[symbolIndex][1] =
                fn_80025A04(fn_80008DD0(symbolModelBase + 0x600000 + symbolIndex,
                                                     0x10000000, 2));
            fn_80026B64(lbl_1_bss_118[targetDisplayIndex].symbolModels[symbolIndex][1], 1U);
            fn_80026790(lbl_1_bss_118[targetDisplayIndex].symbolModels[symbolIndex][1],
                             &reelSymbolPosition);
            fn_80026808(lbl_1_bss_118[targetDisplayIndex].symbolModels[symbolIndex][1], 0.0f,
                            0.0f, 0.0f);
            fn_80026B8C(lbl_1_bss_118[targetDisplayIndex].symbolModels[symbolIndex][1], 2);
            if (symbolIndex != lbl_1_bss_118[targetDisplayIndex].targetSymbol) {
                fn_800269E8(lbl_1_bss_118[targetDisplayIndex].symbolModels[symbolIndex][1],
                                 (1 << 0));
            }
            lbl_1_bss_118[targetDisplayIndex].settledSymbol =
                lbl_1_bss_118[targetDisplayIndex].targetSymbol;
            symbolIndex += 1;
        }
    }
    lbl_1_bss_114 = 0;
    lbl_1_bss_110 = 0;
    lbl_1_bss_F8[0] = lbl_1_bss_F8[1] = 0.0f;
    lbl_1_bss_10E =
        fn_80025A04(fn_80008DD0(DATANUM(0x600000, 24), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_10E, 1U);
    Hu3DModelObjPosGet(lbl_1_bss_10C, lbl_1_data_68, &reelSymbolPosition);
    reelSymbolPosition.y -= 80.0f;
    fn_80026790(lbl_1_bss_10E, &reelSymbolPosition);
    fn_80026808(lbl_1_bss_10E, 0.0f, 0.0f, 0.0f);
    fn_80026B8C(lbl_1_bss_10E, 2);
    lbl_1_bss_10A =
        fn_80025A04(fn_80008DD0(DATANUM(0x600000, 64), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_10A, 1U);
    fn_80026790(lbl_1_bss_10A, &reelSymbolPosition);
    fn_80026808(lbl_1_bss_10A, 0.0f, 0.0f, 0.0f);
    fn_80026B8C(lbl_1_bss_10A, 3);
    lbl_1_bss_108 = fn_80030458(
        lbl_1_bss_10A, fn_80008DD0(DATANUM(0x600000, 65), 0x10000000, 2));
    fn_8002C95C(lbl_1_bss_10A, lbl_1_bss_108);
    fn_800269E8(lbl_1_bss_10A, ((1 << 30) | (1 << 0)));
    fn_800269E8(lbl_1_bss_10A, (1 << 0));
}

/* Called by fn_1_548 to load the arena background layers before the opening scene. */
void fn_1_3460(void)
{
    s16 backgroundModel;

    backgroundModel = fn_80025A04(fn_80008DD0(0x600000, 0x10000000, 2));
    fn_80026B64(backgroundModel, 1U);
    fn_80026760(backgroundModel, 0.0f, -200.0f, 0.0f);
    fn_80026808(backgroundModel, 0.0f, 0.0f, 0.0f);
    fn_80026B8C(backgroundModel, 1);
    backgroundModel =
        fn_80025A04(fn_80008DD0(DATANUM(0x600000, 1), 0x10000000, 2));
    fn_80026B64(backgroundModel, 1U);
    fn_80026760(backgroundModel, 0.0f, -200.0f, 0.0f);
    fn_80026808(backgroundModel, 0.0f, 0.0f, 0.0f);
    fn_800269E8(backgroundModel, ((1 << 30) | (1 << 0)));
    fn_80026B8C(backgroundModel, 1);
    backgroundModel =
        fn_80025A04(fn_80008DD0(DATANUM(0x600000, 2), 0x10000000, 2));
    fn_80026B64(backgroundModel, 1U);
    fn_80026760(backgroundModel, 0.0f, -200.0f, 0.0f);
    fn_80026808(backgroundModel, 0.0f, 0.0f, 0.0f);
    fn_800269E8(backgroundModel, ((1 << 30) | (1 << 0)));
    fn_80026B8C(backgroundModel, 1);
    backgroundModel =
        fn_80025A04(fn_80008DD0(DATANUM(0x600000, 3), 0x10000000, 2));
    fn_80026B64(backgroundModel, 1U);
    fn_80026760(backgroundModel, 0.0f, -200.0f, 0.0f);
    fn_80026808(backgroundModel, 0.0f, 0.0f, 0.0f);
    fn_800269E8(backgroundModel, ((1 << 30) | (1 << 0)));
    fn_80026B8C(backgroundModel, 1);
    fn_1_5914(0);
}

/* Updates reels and match indicators during the opening, play, and result callbacks.
 * With input enabled, awards a point to each team whose centered, stopped reels match the targets
 * (both teams can score together) and returns nonzero on a score. Without input, settles the
 * reels and returns nonzero only when all four have stopped. */
s32 fn_1_36E8(s32 acceptPlayerInput)
{
    s32 teamIndex;
    s32 slotIndex;
    s32 playerIndex;
    s32 updateComplete;
    s32 leftTeamMatched;
    s32 rightTeamMatched;
    s32 leftTeamTickSound;
    s32 rightTeamTickSound;
    f32 totalReelAngle;
    f32 teamYawDirection;
    f32 teamYaw;

    updateComplete = 0;
    for (teamIndex = 0; teamIndex < 2; teamIndex++) {
        slotIndex = 0;
        while (slotIndex < 2) {
            playerIndex = slotIndex + (teamIndex * 2);
            if (acceptPlayerInput != 0) {
                if ((s32) (lbl_802C0008[lbl_80265728[lbl_1_bss_290[playerIndex]].padNo] &
                           PAD_BUTTON_A) != 0) {
                    /* The selected direction accelerates the reel, up to four degrees per
                     * update. */
                    lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex] += 1.0f;
                    if (lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex] > 4.0f) {
                        lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex] = 4.0f;
                    }
                    if (lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex] == 0.0f) {
                        /* Keep a direction change moving instead of landing exactly at zero. */
                        lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex] += 0.01f;
                    }
                    lbl_1_bss_2A0[playerIndex].idleInputFrames = 0;
                    lbl_1_bss_354 = 1;
                } else if ((s32) (lbl_802C0008[lbl_80265728[lbl_1_bss_290[playerIndex]].padNo] &
                                  PAD_BUTTON_B) != 0) {
                    lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex] -= 1.0f;
                    if (lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex] < -4.0f) {
                        lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex] = -4.0f;
                    }
                    if (lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex] == 0.0f) {
                        /* B likewise passes through zero with a small negative speed. */
                        lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex] -= 0.01f;
                    }
                    lbl_1_bss_2A0[playerIndex].idleInputFrames = 0;
                    lbl_1_bss_354 = 2;
                } else {
                    lbl_1_bss_2A0[playerIndex].idleInputFrames += 1;
                }
                if (lbl_1_bss_2A0[playerIndex].idleInputFrames > 10) {
                    /* After ten idle updates, ease the stored speed back toward zero. */
                    if (lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex] > 0.0f) {
                        lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex] -= 0.05f;
                        if (lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex] < 0.0f) {
                            lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex] = 0.0f;
                        }
                    } else if (lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex] < 0.0f) {
                        lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex] += 0.05f;
                        if (lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex] > 0.0f) {
                            lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex] = 0.0f;
                        }
                    }
                }
            } else if (lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex] > 0.0f) {
                lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex] -= 0.1f;
                if (lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex] < 0.0f) {
                    lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex] = 0.0f;
                }
            } else if (lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex] < 0.0f) {
                lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex] += 0.1f;
                if (lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex] > 0.0f) {
                    lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex] = 0.0f;
                }
            }
            if (teamIndex == 0) {
                teamYawDirection = 1.0f;
            } else {
                teamYawDirection = -1.0f;
            }
            teamYaw = 40.0f * teamYawDirection;
            if (lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex]) {
                lbl_1_bss_1D0[teamIndex].modelAngle[slotIndex] +=
                    lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex];
                if (lbl_1_bss_1D0[teamIndex].modelAngle[slotIndex] <= -30.0f) {
                    lbl_1_bss_1D0[teamIndex].modelAngle[slotIndex] += 240.0f;
                    fn_800269E8(
                        lbl_1_bss_1D0[teamIndex]
                            .reelModels[slotIndex]
                                       [lbl_1_bss_1D0[teamIndex].visibleReelModel[slotIndex]],
                        (1 << 0));
                    lbl_1_bss_1D0[teamIndex].visibleReelModel[slotIndex] ^= 1;
                    fn_80026A44(
                        lbl_1_bss_1D0[teamIndex]
                            .reelModels[slotIndex]
                                       [lbl_1_bss_1D0[teamIndex].visibleReelModel[slotIndex]],
                        (1 << 0));
                }
                if (lbl_1_bss_1D0[teamIndex].modelAngle[slotIndex] >= 210.0f) {
                    lbl_1_bss_1D0[teamIndex].modelAngle[slotIndex] -= 240.0f;
                    fn_800269E8(
                        lbl_1_bss_1D0[teamIndex]
                            .reelModels[slotIndex]
                                       [lbl_1_bss_1D0[teamIndex].visibleReelModel[slotIndex]],
                        (1 << 0));
                    lbl_1_bss_1D0[teamIndex].visibleReelModel[slotIndex] ^= 1;
                    fn_80026A44(
                        lbl_1_bss_1D0[teamIndex]
                            .reelModels[slotIndex]
                                       [lbl_1_bss_1D0[teamIndex].visibleReelModel[slotIndex]],
                        (1 << 0));
                }
                fn_80026808(
                    lbl_1_bss_1D0[teamIndex]
                        .reelModels[slotIndex]
                                   [lbl_1_bss_1D0[teamIndex].visibleReelModel[slotIndex]],
                    -lbl_1_bss_1D0[teamIndex].modelAngle[slotIndex], teamYaw, 0.0f);
                lbl_1_bss_1D0[teamIndex].symbolAngle[slotIndex] +=
                    lbl_1_bss_1D0[teamIndex].reelSpeed[slotIndex];
                if (lbl_1_bss_1D0[teamIndex].symbolAngle[slotIndex] >= 30.0f) {
                    /* Keep the accumulated reel angle within one eight-symbol revolution. */
                    lbl_1_bss_1D0[teamIndex].symbolAngle[slotIndex] -= 480.0f;
                }
                if (lbl_1_bss_1D0[teamIndex].symbolAngle[slotIndex] < -450.0f) {
                    lbl_1_bss_1D0[teamIndex].symbolAngle[slotIndex] += 480.0f;
                }
                lbl_1_bss_358 = lbl_1_bss_1D0[teamIndex].reelSymbol[slotIndex];
                totalReelAngle = lbl_1_bss_1D0[teamIndex].symbolAngle[slotIndex];
                if ((totalReelAngle >= -30.0f) && (totalReelAngle < 30.0f)) {
                    lbl_1_bss_1D0[teamIndex].reelSymbol[slotIndex] = 0;
                } else if ((totalReelAngle >= -90.0f) && (totalReelAngle < -30.0f)) {
                    lbl_1_bss_1D0[teamIndex].reelSymbol[slotIndex] = 1;
                } else if ((totalReelAngle >= -150.0f) && (totalReelAngle < -90.0f)) {
                    lbl_1_bss_1D0[teamIndex].reelSymbol[slotIndex] = 2;
                } else if ((totalReelAngle >= -210.0f) && (totalReelAngle < -150.0f)) {
                    lbl_1_bss_1D0[teamIndex].reelSymbol[slotIndex] = 3;
                } else if ((totalReelAngle >= -270.0f) && (totalReelAngle < -210.0f)) {
                    lbl_1_bss_1D0[teamIndex].reelSymbol[slotIndex] = 4;
                } else if ((totalReelAngle >= -330.0f) && (totalReelAngle < -270.0f)) {
                    lbl_1_bss_1D0[teamIndex].reelSymbol[slotIndex] = 5;
                } else if ((totalReelAngle >= -390.0f) && (totalReelAngle < -330.0f)) {
                    lbl_1_bss_1D0[teamIndex].reelSymbol[slotIndex] = 6;
                } else if ((totalReelAngle >= -450.0f) && (totalReelAngle < -390.0f)) {
                    lbl_1_bss_1D0[teamIndex].reelSymbol[slotIndex] = 7;
                }
                if (lbl_1_bss_358 != (s32) lbl_1_bss_1D0[teamIndex].reelSymbol[slotIndex]) {
                    if (teamIndex == 0) {
                        if (lbl_1_bss_354 == 1) {
                            leftTeamTickSound = MSM_SE_MG637_REEL_LEFT_SPEED_UP;
                        } else {
                            leftTeamTickSound = MSM_SE_MG637_REEL_LEFT_SPEED_DOWN;
                        }
                        HuAudFXPlay(leftTeamTickSound);
                    } else {
                        if (lbl_1_bss_354 == 1) {
                            rightTeamTickSound = MSM_SE_MG637_REEL_RIGHT_SPEED_UP;
                        } else {
                            rightTeamTickSound = MSM_SE_MG637_REEL_RIGHT_SPEED_DOWN;
                        }
                        HuAudFXPlay(rightTeamTickSound);
                    }
                }
                totalReelAngle = lbl_1_bss_1D0[teamIndex].symbolAngle[slotIndex] +
                                 (60.0f * (f32) lbl_1_bss_1D0[teamIndex].reelSymbol[slotIndex]);
                if ((totalReelAngle > 10.0f) || (totalReelAngle < -10.0f)) {
                    lbl_1_bss_1D0[teamIndex].alignmentState[slotIndex] = 0;
                } else if ((s32) lbl_1_bss_1D0[teamIndex].alignmentState[slotIndex] == 0) {
                    lbl_1_bss_1D0[teamIndex].alignmentState[slotIndex] = 1;
                }
            }
            slotIndex += 1;
        }
        if ((lbl_1_bss_1D0[teamIndex].reelSpeed[0] != 0.0f) ||
            (lbl_1_bss_1D0[teamIndex].reelSpeed[1] != 0.0f)) {
            fn_8002D86C(lbl_1_bss_1D0[teamIndex].spinEffectModel, 1.0f);
            if (lbl_1_bss_1D0[teamIndex].spinAnimationActive == 0) {
                fn_8002C95C(lbl_1_bss_1D0[teamIndex].teamAnimationModel,
                              lbl_1_bss_1D0[teamIndex].spinMotion);
                lbl_1_bss_1D0[teamIndex].spinAnimationActive = 1;
                fn_8002C95C(lbl_1_bss_1D0[teamIndex].reelAnimationModels[0],
                              lbl_1_bss_1D0[teamIndex].reelAnimationMotions[0][1]);
                fn_8002C95C(lbl_1_bss_1D0[teamIndex].reelAnimationModels[1],
                              lbl_1_bss_1D0[teamIndex].reelAnimationMotions[1][1]);
            }
        } else {
            fn_8002D86C(lbl_1_bss_1D0[teamIndex].spinEffectModel, 0.0f);
            if (lbl_1_bss_1D0[teamIndex].spinAnimationActive == 1) {
                fn_8002C95C(lbl_1_bss_1D0[teamIndex].teamAnimationModel,
                              lbl_1_bss_1D0[teamIndex].idleMotion);
                lbl_1_bss_1D0[teamIndex].spinAnimationActive = 0;
                fn_8002C95C(lbl_1_bss_1D0[teamIndex].reelAnimationModels[0],
                              lbl_1_bss_1D0[teamIndex].reelAnimationMotions[0][0]);
                fn_8002C95C(lbl_1_bss_1D0[teamIndex].reelAnimationModels[1],
                              lbl_1_bss_1D0[teamIndex].reelAnimationMotions[1][0]);
            }
        }
    }
    if ((acceptPlayerInput == 0) && (lbl_1_bss_1D0->reelSpeed[0] == 0.0f) &&
        (lbl_1_bss_1D0->reelSpeed[1] == 0.0f) && (lbl_1_bss_1D0[1].reelSpeed[0] == 0.0f) &&
        (lbl_1_bss_1D0[1].reelSpeed[1] == 0.0f)) {
        updateComplete = 1;
    }
    if (acceptPlayerInput != 0) {
        leftTeamMatched = 0;
        rightTeamMatched = 0;
        for (teamIndex = 0; teamIndex < 2; teamIndex++) {
            slotIndex = 0;
            while (slotIndex < 2) {
                if (((s32) lbl_1_bss_1D0[teamIndex].alignmentState[slotIndex] != 0) &&
                    (lbl_1_bss_118[slotIndex].targetSymbol ==
                     (s32) lbl_1_bss_1D0[teamIndex].reelSymbol[slotIndex])) {
                    fn_80026A44(lbl_1_bss_1D0[teamIndex].matchIndicatorModels[slotIndex],
                                       (1 << 0));
                    if ((s32) lbl_1_bss_1D0[teamIndex].alignmentState[slotIndex] == 1) {
                        if (teamIndex == 0) {
                            HuAudFXPlay(MSM_SE_MG637_REEL_ALIGN_TEAM_0);
                        } else {
                            HuAudFXPlay(MSM_SE_MG637_REEL_ALIGN_TEAM_1);
                        }
                        lbl_1_bss_1D0[teamIndex].alignmentState[slotIndex] = 2;
                    }
                } else {
                    fn_800269E8(lbl_1_bss_1D0[teamIndex].matchIndicatorModels[slotIndex],
                                     (1 << 0));
                }
                slotIndex += 1;
            }
        }
        if ((lbl_1_bss_1D0->reelSpeed[0] == 0.0f) &&
            ((s32) lbl_1_bss_1D0->alignmentState[0] != 0) &&
            (lbl_1_bss_118->targetSymbol == (s32) lbl_1_bss_1D0->reelSymbol[0]) &&
            (lbl_1_bss_1D0->reelSpeed[1] == 0.0f) &&
            ((s32) lbl_1_bss_1D0->alignmentState[1] != 0) &&
            (lbl_1_bss_118[1].targetSymbol == (s32) lbl_1_bss_1D0->reelSymbol[1])) {
            leftTeamMatched = 1;
        }
        if ((lbl_1_bss_1D0[1].reelSpeed[0] == 0.0f) &&
            ((s32) lbl_1_bss_1D0[1].alignmentState[0] != 0) &&
            (lbl_1_bss_118->targetSymbol == (s32) lbl_1_bss_1D0[1].reelSymbol[0]) &&
            (lbl_1_bss_1D0[1].reelSpeed[1] == 0.0f) &&
            ((s32) lbl_1_bss_1D0[1].alignmentState[1] != 0) &&
            (lbl_1_bss_118[1].targetSymbol == (s32) lbl_1_bss_1D0[1].reelSymbol[1])) {
            rightTeamMatched = 1;
        }
        if (leftTeamMatched != 0) {
            lbl_1_bss_2A0->scoredRound = lbl_1_bss_2A0[1].scoredRound = 1;
            if (rightTeamMatched == 0) {
                lbl_1_bss_2A0[2].scoredRound = lbl_1_bss_2A0[3].scoredRound = 0;
            }
            lbl_1_bss_2A0->teamScore += 1;
            lbl_1_bss_2A0[1].teamScore += 1;
            updateComplete = 1;
        }
        if (rightTeamMatched != 0) {
            if (leftTeamMatched == 0) {
                lbl_1_bss_2A0->scoredRound = lbl_1_bss_2A0[1].scoredRound = 0;
            }
            lbl_1_bss_2A0[2].scoredRound = lbl_1_bss_2A0[3].scoredRound = 1;
            lbl_1_bss_2A0[2].teamScore += 1;
            lbl_1_bss_2A0[3].teamScore += 1;
            updateComplete = 1;
        }
        if (updateComplete != 0) {
            fn_1_5C98(2);
        }
    }
    return updateComplete;
}

/* Called by fn_1_65C before and between rounds to advance the two central target displays.
 *
 * Returns 1 when the requested number of selections and their face turns is complete, otherwise 0.
 */
s32 fn_1_4C24(s32 selectionLimit)
{
    s32 reelIndex;
    s32 stopComplete;
    s32 useStopSelection;

    stopComplete = 0;
    switch (lbl_1_bss_114) {
    case 0:
        lbl_1_bss_110 += 1;
        lbl_1_bss_104 = 0;
        reelIndex = 0;
        while (reelIndex < 2) {
            if (lbl_1_bss_110 == selectionLimit) {
                useStopSelection = 1;
            } else {
                useStopSelection = 0;
            }
            lbl_1_bss_118[reelIndex].targetSymbol = fn_1_5444(reelIndex, useStopSelection);
            if (lbl_1_bss_118[reelIndex].targetSymbol == lbl_1_bss_118[reelIndex].initialSymbol) {
                lbl_1_bss_104 += 1;
            }
            reelIndex += 1;
        }
        if ((lbl_1_bss_104 == 2) && (lbl_1_bss_110 == selectionLimit) &&
            (lbl_1_bss_374 == 0)) {
            /* Before a round starts, replace both targets if both would return to their scene-setup
             * symbols. */
            fn_1_55A4(selectionLimit);
        } else {
            reelIndex = 0;
            while (reelIndex < 2) {
                lbl_1_bss_118[reelIndex].lastChosenSymbol = lbl_1_bss_118[reelIndex].targetSymbol;
                if ((lbl_1_bss_110 == selectionLimit) && (lbl_1_bss_374 == 0)) {
                    lbl_1_bss_118[reelIndex].openingTargetSymbol =
                        lbl_1_bss_118[reelIndex].targetSymbol;
                }
                lbl_1_bss_118[reelIndex].incomingAngle = -20.0f;
                fn_80026808(
                    lbl_1_bss_118[reelIndex].symbolModels[lbl_1_bss_118[reelIndex].targetSymbol][0],
                    lbl_1_bss_118[reelIndex].incomingAngle, 0.0f, 0.0f);
                fn_80026A44(
                    lbl_1_bss_118[reelIndex].symbolModels[lbl_1_bss_118[reelIndex].targetSymbol][0],
                    (1 << 0));
                fn_800269E8(
                    lbl_1_bss_118[reelIndex].symbolModels[lbl_1_bss_118[reelIndex].targetSymbol][1],
                    (1 << 0));
                lbl_1_bss_118[reelIndex].outgoingAngle = 0.0f;
                fn_80026808(lbl_1_bss_118[reelIndex]
                                    .symbolModels[lbl_1_bss_118[reelIndex].settledSymbol][0],
                                lbl_1_bss_118[reelIndex].outgoingAngle, 0.0f, 0.0f);
                fn_80026A44(lbl_1_bss_118[reelIndex]
                                       .symbolModels[lbl_1_bss_118[reelIndex].settledSymbol][0],
                                   (1 << 0));
                /* Reset the previous target's first face to zero again after showing it. */
                fn_80026808(lbl_1_bss_118[reelIndex]
                                    .symbolModels[lbl_1_bss_118[reelIndex].settledSymbol][0],
                                0.0f, 0.0f, 0.0f);
                fn_80026A44(lbl_1_bss_118[reelIndex]
                                       .symbolModels[lbl_1_bss_118[reelIndex].settledSymbol][1],
                                   (1 << 0));
                reelIndex += 1;
            }
        }
        lbl_1_bss_114 += 1;
        HuAudFXPlay(MSM_SE_MG637_REEL_SYMBOLS_SET);
        break;
    case 1:
    case 2:
    case 3:
        reelIndex = 0;
        while (reelIndex < 2) {
            lbl_1_bss_118[reelIndex].outgoingAngle += 20.0f;
            fn_80026808(
                lbl_1_bss_118[reelIndex].symbolModels[lbl_1_bss_118[reelIndex].settledSymbol][0],
                lbl_1_bss_118[reelIndex].outgoingAngle, 0.0f, 0.0f);
            if (lbl_1_bss_118[reelIndex].outgoingAngle >= 90.0f) {
                fn_800269E8(lbl_1_bss_118[reelIndex]
                                     .symbolModels[lbl_1_bss_118[reelIndex].settledSymbol][0],
                                 (1 << 0));
                fn_80026A44(
                    lbl_1_bss_118[reelIndex].symbolModels[lbl_1_bss_118[reelIndex].targetSymbol][1],
                    (1 << 0));
                fn_80026808(
                    lbl_1_bss_118[reelIndex].symbolModels[lbl_1_bss_118[reelIndex].targetSymbol][1],
                    -180.0f + lbl_1_bss_118[reelIndex].outgoingAngle, 0.0f, 0.0f);
            }
            if (lbl_1_bss_114 == 1) {
                lbl_1_bss_118[reelIndex].incomingAngle += 20.0f;
                fn_80026808(
                    lbl_1_bss_118[reelIndex].symbolModels[lbl_1_bss_118[reelIndex].targetSymbol][0],
                    lbl_1_bss_118[reelIndex].incomingAngle, 0.0f, 0.0f);
                fn_80026A44(
                    lbl_1_bss_118[reelIndex].symbolModels[lbl_1_bss_118[reelIndex].targetSymbol][0],
                    (1 << 0));
            }
            if (lbl_1_bss_114 == 3) {
                fn_800269E8(lbl_1_bss_118[reelIndex]
                                     .symbolModels[lbl_1_bss_118[reelIndex].settledSymbol][1],
                                 (1 << 0));
            }
            reelIndex += 1;
        }
        if ((lbl_1_bss_118->outgoingAngle == 20.0f) || (lbl_1_bss_118->outgoingAngle == 160.0f) ||
            (lbl_1_bss_118->outgoingAngle == 180.0f)) {
            lbl_1_bss_114 += 1;
        }
        break;
    case 4:
        lbl_1_bss_114 = 0;
        lbl_1_bss_118->settledSymbol = lbl_1_bss_118->targetSymbol;
        lbl_1_bss_118[1].settledSymbol = lbl_1_bss_118[1].targetSymbol;
        if (lbl_1_bss_110 == selectionLimit) {
            lbl_1_bss_110 = 0;
            stopComplete = 1;
        }
        break;
    }
    return stopComplete;
}

/* Called by fn_1_4C24 while advancing a reel; picks a symbol not already used
 * by the stop positions required for this stop phase. */
s32 fn_1_5444(s32 reelIndex, s32 stopPhase)
{
    s32 candidateSymbols[8];
    s32 candidateSymbolIndex;
    s32 candidateCount;
    s32 candidateSymbolCount;

    candidateSymbolIndex = candidateCount = 0;
    if (stopPhase == 0) {
        while (candidateSymbolIndex < 8) {
            if (candidateSymbolIndex != lbl_1_bss_118[reelIndex].lastChosenSymbol) {
                (&candidateSymbols[0])[candidateCount] = candidateSymbolIndex;
                candidateCount += 1;
            }
            candidateSymbolIndex += 1;
        }
    } else {
        while (candidateSymbolIndex < 8) {
            if (stopPhase == 1) {
                if ((candidateSymbolIndex != lbl_1_bss_118[reelIndex].openingTargetSymbol) &&
                    (candidateSymbolIndex != lbl_1_bss_118[reelIndex].lastChosenSymbol)) {
                    (&candidateSymbols[0])[candidateCount] = candidateSymbolIndex;
                    candidateCount += 1;
                }
            } else if ((candidateSymbolIndex != lbl_1_bss_118[reelIndex].openingTargetSymbol) &&
                       (candidateSymbolIndex != lbl_1_bss_118[reelIndex].lastChosenSymbol) &&
                       (candidateSymbolIndex != lbl_1_bss_118[reelIndex].initialSymbol)) {
                (&candidateSymbols[0])[candidateCount] = candidateSymbolIndex;
                candidateCount += 1;
            }
            candidateSymbolIndex += 1;
        }
    }
    candidateSymbolCount = candidateCount;
    return (&candidateSymbols[0])[frandmod(candidateSymbolCount)];
}

/* Called before a round starts if both final targets would show their scene-setup symbols;
 * chooses replacement symbols and resets both displays. stopFrame is unused. */
void fn_1_55A4(s32 stopFrame)
{
    s32 candidateSymbols[8];
    s32 reelIndex;
    s32 symbolIndex;
    s32 candidateCount;
    s32 selectedSymbol;
    s32 candidateSymbolCount;

    for (reelIndex = 0; reelIndex < 2; reelIndex++) {
        symbolIndex = candidateCount = 0;
        while (symbolIndex < 8) {
            if ((symbolIndex != lbl_1_bss_118[reelIndex].openingTargetSymbol) &&
                (symbolIndex != lbl_1_bss_118[reelIndex].lastChosenSymbol) &&
                (symbolIndex != lbl_1_bss_118[reelIndex].initialSymbol)) {
                (&candidateSymbols[0])[candidateCount] = symbolIndex;
                candidateCount += 1;
            }
            symbolIndex += 1;
        }
        candidateSymbolCount = candidateCount;
        selectedSymbol = (&candidateSymbols[0])[frandmod(candidateSymbolCount)];
        lbl_1_bss_118[reelIndex].targetSymbol = selectedSymbol;
        lbl_1_bss_118[reelIndex].openingTargetSymbol = lbl_1_bss_118[reelIndex].targetSymbol;
        lbl_1_bss_118[reelIndex].lastChosenSymbol = lbl_1_bss_118[reelIndex].targetSymbol;
        lbl_1_bss_118[reelIndex].incomingAngle = -20.0f;
        fn_80026808(
            lbl_1_bss_118[reelIndex].symbolModels[lbl_1_bss_118[reelIndex].targetSymbol][0],
            lbl_1_bss_118[reelIndex].incomingAngle, 0.0f, 0.0f);
        fn_80026A44(
            lbl_1_bss_118[reelIndex].symbolModels[lbl_1_bss_118[reelIndex].targetSymbol][0],
            (1 << 0));
        fn_800269E8(
            lbl_1_bss_118[reelIndex].symbolModels[lbl_1_bss_118[reelIndex].targetSymbol][1],
            (1 << 0));
        lbl_1_bss_118[reelIndex].outgoingAngle = 0.0f;
        fn_80026808(
            lbl_1_bss_118[reelIndex].symbolModels[lbl_1_bss_118[reelIndex].settledSymbol][0],
            lbl_1_bss_118[reelIndex].outgoingAngle, 0.0f, 0.0f);
        fn_80026A44(
            lbl_1_bss_118[reelIndex].symbolModels[lbl_1_bss_118[reelIndex].settledSymbol][0],
            (1 << 0));
        /* Reset the previous target's first face to zero again after showing it. */
        fn_80026808(
            lbl_1_bss_118[reelIndex].symbolModels[lbl_1_bss_118[reelIndex].settledSymbol][0], 0.0f,
            0.0f, 0.0f);
        fn_80026A44(
            lbl_1_bss_118[reelIndex].symbolModels[lbl_1_bss_118[reelIndex].settledSymbol][1],
            (1 << 0));
    }
}

/* Called during fn_1_548 setup and by fn_1_65C to place the team score panels. */
void fn_1_5914(s32 setupMode)
{
    s32 teamIndex;

    teamIndex = 0;
    while (teamIndex < 2) {
        if (setupMode == 0) {
            lbl_1_bss_B8[teamIndex].sizeX = 108;
            lbl_1_bss_B8[teamIndex].sizeY = 36;
            if (teamIndex == 0) {
                lbl_1_bss_B8[teamIndex].visiblePositionX =
                    16.0f + ((f32) lbl_1_bss_B8[teamIndex].sizeX / 2.0f);
                lbl_1_bss_B8[teamIndex].positionX =
                    lbl_1_bss_B8[teamIndex].visiblePositionX - 150.0f;
            } else {
                lbl_1_bss_B8[teamIndex].visiblePositionX =
                    560.0f - ((f32) lbl_1_bss_B8[teamIndex].sizeX / 2.0f);
                lbl_1_bss_B8[teamIndex].positionX =
                    150.0f + lbl_1_bss_B8[teamIndex].visiblePositionX;
            }
            lbl_1_bss_B8[teamIndex].positionY =
                40.0f + ((f32) lbl_1_bss_B8[teamIndex].sizeY / 2.0f);
            lbl_1_bss_B8[teamIndex].scoreBox =
                MgScoreBoxCreate(lbl_1_bss_B8[teamIndex].sizeX, lbl_1_bss_B8[teamIndex].sizeY);
            MgScoreBoxPosSet(lbl_1_bss_B8[teamIndex].scoreBox, lbl_1_bss_B8[teamIndex].positionX,
                             lbl_1_bss_B8[teamIndex].positionY);
            if (teamIndex == 0) {
                MgScoreBoxColorSet(lbl_1_bss_B8[teamIndex].scoreBox, 250U, 0U, 30U);
            } else {
                MgScoreBoxColorSet(lbl_1_bss_B8[teamIndex].scoreBox, 0U, 50U, 250U);
            }
        } else {
            lbl_1_bss_B8[teamIndex].positionX = lbl_1_bss_B8[teamIndex].visiblePositionX;
            MgScoreBoxPosSet(lbl_1_bss_B8[teamIndex].scoreBox, lbl_1_bss_B8[teamIndex].positionX,
                             lbl_1_bss_B8[teamIndex].positionY);
        }
        teamIndex += 1;
    }
    fn_1_5C98(setupMode);
}

/* Called during setup and after a match to create, position, or update each team's three point
 * markers. */
void fn_1_5C98(s32 displayMode)
{
    s32 teamIndex;
    s32 pointMarkerIndex;
    f32 pointMarkerX;

    for (teamIndex = 0; teamIndex < 2; teamIndex++) {
        pointMarkerIndex = 0;
        while (pointMarkerIndex < 3) {
            pointMarkerX =
                (32.0f * (f32) pointMarkerIndex) + (lbl_1_bss_B8[teamIndex].positionX - 32.0f);
            if (displayMode == 0) {
                lbl_1_bss_B8[teamIndex].unearnedPointSprites[pointMarkerIndex] =
                    espEntry(M637_ESP_SCORE_FRONT, 0, 0);
                espDrawNoSet(lbl_1_bss_B8[teamIndex].unearnedPointSprites[pointMarkerIndex], 0);
                espPriSet(lbl_1_bss_B8[teamIndex].unearnedPointSprites[pointMarkerIndex], 1);
                espPosSet(lbl_1_bss_B8[teamIndex].unearnedPointSprites[pointMarkerIndex],
                          pointMarkerX, lbl_1_bss_B8[teamIndex].positionY);
                espTPLvlSet(lbl_1_bss_B8[teamIndex].unearnedPointSprites[pointMarkerIndex], 0.5f);
                lbl_1_bss_B8[teamIndex].earnedPointSprites[pointMarkerIndex] =
                    espEntry(M637_ESP_SCORE_BACK, 0, 0);
                espDrawNoSet(lbl_1_bss_B8[teamIndex].earnedPointSprites[pointMarkerIndex], 0);
                espPriSet(lbl_1_bss_B8[teamIndex].earnedPointSprites[pointMarkerIndex], 1);
                espPosSet(lbl_1_bss_B8[teamIndex].earnedPointSprites[pointMarkerIndex],
                          pointMarkerX, lbl_1_bss_B8[teamIndex].positionY);
                espDispOff(lbl_1_bss_B8[teamIndex].earnedPointSprites[pointMarkerIndex]);
            } else if (displayMode == 1) {
                espPosSet(lbl_1_bss_B8[teamIndex].unearnedPointSprites[pointMarkerIndex],
                          pointMarkerX, lbl_1_bss_B8[teamIndex].positionY);
                espPosSet(lbl_1_bss_B8[teamIndex].earnedPointSprites[pointMarkerIndex],
                          pointMarkerX, lbl_1_bss_B8[teamIndex].positionY);
            } else if (lbl_1_bss_2A0[teamIndex * 2].teamScore > pointMarkerIndex) {
                /* Swap the layered sprites once this score position has been earned. */
                espDispOn(lbl_1_bss_B8[teamIndex].earnedPointSprites[pointMarkerIndex]);
                espDispOff(lbl_1_bss_B8[teamIndex].unearnedPointSprites[pointMarkerIndex]);
            } else {
                espDispOn(lbl_1_bss_B8[teamIndex].unearnedPointSprites[pointMarkerIndex]);
                espDispOff(lbl_1_bss_B8[teamIndex].earnedPointSprites[pointMarkerIndex]);
            }
            pointMarkerIndex += 1;
        }
    }
}

/* Called in round-sequence state 3 after a team scores; plays each player's round-result pose,
 * then returns characters and team models to idle and hides the effects when the first character
 * finishes. The transition query uses that character number directly as a model ID. */
s32 fn_1_603C(void)
{
    s32 playerSlot;
    s32 animationFinished;
    s32 stopMotionIndex;

    animationFinished = 0;
    if (lbl_1_bss_368 == 0) {
        playerSlot = 0;
        while (playerSlot < 4) {
            if (lbl_1_bss_2A0[playerSlot].scoredRound != 0) {
                stopMotionIndex = 5;
            } else {
                stopMotionIndex = 6;
            }
            fn_8005C2C8(lbl_80265728[lbl_1_bss_290[playerSlot]].charNo,
                               lbl_1_bss_2A0[playerSlot].motion[stopMotionIndex], 0.0f, 4.0f, 0U);
            if (lbl_1_bss_2A0[playerSlot].scoredRound != 0) {
                CharFXPlay(lbl_80265728[lbl_1_bss_290[playerSlot]].charNo,
                           MSM_SE_CHARVOICE_HAPPY_CHEER);
            }
            playerSlot += 1;
        }
        lbl_1_bss_368 += 1;
        /* Switch the stopped timer back to on mode with stopF set so its end-display fade runs
         * without advancing the countdown. */
        if (lbl_1_bss_34C->stopF == 0) {
            lbl_1_bss_34C->stopF = 1;
            lbl_1_bss_34C->mode = 1;
        }
        if (lbl_1_bss_2A0->scoredRound != 0) {
            fn_8002C95C(lbl_1_bss_1D0->teamAnimationModel, lbl_1_bss_1D0->scoreMotion);
            fn_8002C95C(lbl_1_bss_1D0[1].teamAnimationModel, lbl_1_bss_1D0[1].idleMotion);
        }
        if (lbl_1_bss_2A0[2].scoredRound != 0) {
            /* If both teams score together, this second branch leaves the left model idle. */
            fn_8002C95C(lbl_1_bss_1D0->teamAnimationModel, lbl_1_bss_1D0->idleMotion);
            fn_8002C95C(lbl_1_bss_1D0[1].teamAnimationModel, lbl_1_bss_1D0[1].scoreMotion);
        }
        fn_80026A44(lbl_1_bss_10A, (1 << 0));
        if (lbl_1_bss_2A0->scoredRound != 0) {
            fn_80026A44(lbl_1_bss_1D0->matchIndicatorModels[0], (1 << 0));
            fn_80026A44(lbl_1_bss_1D0->matchIndicatorModels[1], (1 << 0));
            fn_80026A44(lbl_1_bss_1D0->teamMatchEffectModel, (1 << 0));
            HuAudFXPlay(MSM_SE_MG637_TEAM_MATCH_TEAM_0);
        } else {
            fn_800269E8(lbl_1_bss_1D0->matchIndicatorModels[0], (1 << 0));
            fn_800269E8(lbl_1_bss_1D0->matchIndicatorModels[1], (1 << 0));
            fn_800269E8(lbl_1_bss_1D0->teamMatchEffectModel, (1 << 0));
        }
        if (lbl_1_bss_2A0[2].scoredRound != 0) {
            fn_80026A44(lbl_1_bss_1D0[1].matchIndicatorModels[0], (1 << 0));
            fn_80026A44(lbl_1_bss_1D0[1].matchIndicatorModels[1], (1 << 0));
            fn_80026A44(lbl_1_bss_1D0[1].teamMatchEffectModel, (1 << 0));
            HuAudFXPlay(MSM_SE_MG637_TEAM_MATCH_TEAM_1);
        } else {
            fn_800269E8(lbl_1_bss_1D0[1].matchIndicatorModels[0], (1 << 0));
            fn_800269E8(lbl_1_bss_1D0[1].matchIndicatorModels[1], (1 << 0));
            fn_800269E8(lbl_1_bss_1D0[1].teamMatchEffectModel, (1 << 0));
        }
    } else if ((fn_8002D3EC(lbl_80265728[lbl_1_bss_290[0]].charNo) == -1) &&
               (fn_8005C240(lbl_80265728[lbl_1_bss_290[0]].charNo) != 0)) {
        playerSlot = 0;
        while (playerSlot < 4) {
            fn_8005C2C8(lbl_80265728[lbl_1_bss_290[playerSlot]].charNo,
                               lbl_1_bss_2A0[playerSlot].motion[0], 0.0f, 8.0f,
                                   ((1 << 30) | (1 << 0)));
            playerSlot += 1;
        }
        lbl_1_bss_368 = 0;
        animationFinished = 1;
        fn_8002C95C(lbl_1_bss_1D0->teamAnimationModel, lbl_1_bss_1D0->idleMotion);
        fn_8002C95C(lbl_1_bss_1D0[1].teamAnimationModel, lbl_1_bss_1D0[1].idleMotion);
        lbl_1_bss_1D0->spinAnimationActive = lbl_1_bss_1D0[1].spinAnimationActive = 0;
        /* The first player's flag is assigned twice; the second player's flag is left as it was. */
        lbl_1_bss_2A0->scoredRound = lbl_1_bss_2A0->scoredRound = 0;
        lbl_1_bss_2A0[2].scoredRound = lbl_1_bss_2A0[3].scoredRound = 0;
        fn_800269E8(lbl_1_bss_10A, (1 << 0));
        playerSlot = 0;
        while (playerSlot < 2) {
            fn_800269E8(lbl_1_bss_1D0[playerSlot].matchIndicatorModels[0], (1 << 0));
            fn_800269E8(lbl_1_bss_1D0[playerSlot].matchIndicatorModels[1], (1 << 0));
            fn_800269E8(lbl_1_bss_1D0[playerSlot].teamMatchEffectModel, (1 << 0));
            playerSlot += 1;
        }
    }
    return animationFinished;
}

/* Called in round-sequence state 5 after the timer expires; resets all character poses and
 * reports when the first character's reset animation has completed.
 * The transition query again uses the character number directly as a model ID. */
s32 fn_1_65A0(void)
{
    s32 playerSlot;
    s32 animationFinished;

    animationFinished = 0;
    if (lbl_1_bss_368 == 0) {
        playerSlot = 0;
        while (playerSlot < 4) {
            fn_8005C2C8(lbl_80265728[lbl_1_bss_290[playerSlot]].charNo,
                               lbl_1_bss_2A0[playerSlot].motion[6], 0.0f, 4.0f, 0U);
            playerSlot += 1;
        }
        playerSlot = 0;
        while (playerSlot < 2) {
            fn_800269E8(lbl_1_bss_1D0[playerSlot].matchIndicatorModels[0], (1 << 0));
            fn_800269E8(lbl_1_bss_1D0[playerSlot].matchIndicatorModels[1], (1 << 0));
            fn_800269E8(lbl_1_bss_1D0[playerSlot].teamMatchEffectModel, (1 << 0));
            playerSlot += 1;
        }
        lbl_1_bss_368 += 1;
    } else if ((fn_8002D3EC(lbl_80265728[lbl_1_bss_290[0]].charNo) == -1) &&
               (fn_8005C240(lbl_80265728[lbl_1_bss_290[0]].charNo) != 0)) {
        playerSlot = 0;
        while (playerSlot < 4) {
            fn_8005C2C8(lbl_80265728[lbl_1_bss_290[playerSlot]].charNo,
                               lbl_1_bss_2A0[playerSlot].motion[0], 0.0f, 8.0f,
                                   ((1 << 30) | (1 << 0)));
            playerSlot += 1;
        }
        lbl_1_bss_368 = 0;
        animationFinished = 1;
    }
    return animationFinished;
}

/* Child process started by fn_1_181C after model setup; selects each player's pose from reel
 * speed, updates the machine attachments, and yields once without a local repeat loop. */
void fn_1_67C4(void)
{
    s32 playerSlot;
    s32 motionChanged;
    s32 teamIndex;
    s32 teamSlot;

    fn_1_1464(0);
    if (lbl_1_bss_368 == 0) {
        playerSlot = 0;
        while (playerSlot < 4) {
            motionChanged = 0;
            /* This query uses the character number as a model ID rather than the stored model. */
            if (fn_8002D3EC(lbl_80265728[lbl_1_bss_290[playerSlot]].charNo) == -1) {
                teamIndex = playerSlot / 2;
                teamSlot = playerSlot % 2;
                if (lbl_1_bss_1D0[teamIndex].reelSpeed[teamSlot] == 0.0f) {
                    if (lbl_1_bss_2A0[playerSlot].selectedMotion !=
                        lbl_1_bss_2A0[playerSlot].motion[0]) {
                        lbl_1_bss_2A0[playerSlot].selectedMotion =
                            lbl_1_bss_2A0[playerSlot].motion[0];
                        motionChanged = 1;
                    }
                } else if (lbl_1_bss_1D0[teamIndex].reelSpeed[teamSlot] > 3.0f) {
                    if (lbl_1_bss_2A0[playerSlot].selectedMotion !=
                        lbl_1_bss_2A0[playerSlot].motion[2]) {
                        lbl_1_bss_2A0[playerSlot].selectedMotion =
                            lbl_1_bss_2A0[playerSlot].motion[2];
                        motionChanged = 1;
                    }
                } else if (lbl_1_bss_1D0[teamIndex].reelSpeed[teamSlot] > 0.0f) {
                    if (lbl_1_bss_2A0[playerSlot].selectedMotion !=
                        lbl_1_bss_2A0[playerSlot].motion[1]) {
                        lbl_1_bss_2A0[playerSlot].selectedMotion =
                            lbl_1_bss_2A0[playerSlot].motion[1];
                        motionChanged = 1;
                    }
                } else if (lbl_1_bss_1D0[teamIndex].reelSpeed[teamSlot] < -3.0f) {
                    if (lbl_1_bss_2A0[playerSlot].selectedMotion !=
                        lbl_1_bss_2A0[playerSlot].motion[3]) {
                        lbl_1_bss_2A0[playerSlot].selectedMotion =
                            lbl_1_bss_2A0[playerSlot].motion[3];
                        motionChanged = 1;
                    }
                } else if ((lbl_1_bss_1D0[teamIndex].reelSpeed[teamSlot] < 0.0f) &&
                           (lbl_1_bss_2A0[playerSlot].selectedMotion !=
                            lbl_1_bss_2A0[playerSlot].motion[4])) {
                    lbl_1_bss_2A0[playerSlot].selectedMotion = lbl_1_bss_2A0[playerSlot].motion[4];
                    motionChanged = 1;
                }
                if (motionChanged != 0) {
                    fn_8005C2C8(lbl_80265728[lbl_1_bss_290[playerSlot]].charNo,
                                       lbl_1_bss_2A0[playerSlot].selectedMotion, 0.0f, 4.0f,
                                       ((1 << 30) | (1 << 0)));
                }
            }
            playerSlot += 1;
        }
    }
    HuPrcVSleep();
}

/* Called by fn_1_65C during active play to generate computer input, with difficulty-dependent
 * detours. Clears each computer's button-down state first; when a planned pulse finds the reel
 * centered on its target, replaces that pulse with the opposite button. */
void fn_1_6BD4(void)
{
    s32 playerSlotIndex;
    s32 aDirectionDistance;
    s32 bDirectionDistance;
    s32 aDirectionPresses;
    s32 bDirectionPresses;
    s32 tieAPresses;
    s32 tieBPresses;
    s32 alternateStepCount;
    s32 randomAlternateStepCount;
    s32 settleDirectionButton;
    s32 settleCorrectionButton;
    s32 spinDirectionButton;
    s32 spinCorrectionButton;

    playerSlotIndex = 0;
    while (playerSlotIndex < 4) {
        if (lbl_80265728[lbl_1_bss_290[playerSlotIndex]].type != 0) {
            lbl_802C0008[lbl_80265728[lbl_1_bss_290[playerSlotIndex]].padNo] = 0;
            if (lbl_1_bss_8[playerSlotIndex].inputPlanActive == 0) {
                /* Compare both ways around the eight-position reel and choose the shorter route.
                * On the target symbol, retain the stored button direction for any alignment
                * correction; a detour can still reverse it. */
                if (lbl_1_bss_118[lbl_1_bss_8[playerSlotIndex].targetReelIndex].settledSymbol >
                    (s32) lbl_1_bss_1D0[lbl_1_bss_8[playerSlotIndex].teamIndex]
                        .reelSymbol[lbl_1_bss_8[playerSlotIndex].teamReelIndex]) {
                    aDirectionDistance =
                        (lbl_1_bss_1D0[lbl_1_bss_8[playerSlotIndex].teamIndex]
                             .reelSymbol[lbl_1_bss_8[playerSlotIndex].teamReelIndex] +
                         8) -
                        lbl_1_bss_118[lbl_1_bss_8[playerSlotIndex].targetReelIndex].settledSymbol;
                    bDirectionDistance =
                        lbl_1_bss_118[lbl_1_bss_8[playerSlotIndex].targetReelIndex].settledSymbol -
                        lbl_1_bss_1D0[lbl_1_bss_8[playerSlotIndex].teamIndex]
                            .reelSymbol[lbl_1_bss_8[playerSlotIndex].teamReelIndex];
                    lbl_1_bss_8[playerSlotIndex].correctionState = 0;
                } else if (lbl_1_bss_118[lbl_1_bss_8[playerSlotIndex].targetReelIndex]
                               .settledSymbol <
                           (s32) lbl_1_bss_1D0[lbl_1_bss_8[playerSlotIndex].teamIndex]
                               .reelSymbol[lbl_1_bss_8[playerSlotIndex].teamReelIndex]) {
                    aDirectionDistance =
                        lbl_1_bss_1D0[lbl_1_bss_8[playerSlotIndex].teamIndex]
                            .reelSymbol[lbl_1_bss_8[playerSlotIndex].teamReelIndex] -
                        lbl_1_bss_118[lbl_1_bss_8[playerSlotIndex].targetReelIndex].settledSymbol;
                    bDirectionDistance =
                        (lbl_1_bss_118[lbl_1_bss_8[playerSlotIndex].targetReelIndex].settledSymbol +
                         8) -
                        lbl_1_bss_1D0[lbl_1_bss_8[playerSlotIndex].teamIndex]
                            .reelSymbol[lbl_1_bss_8[playerSlotIndex].teamReelIndex];
                    lbl_1_bss_8[playerSlotIndex].correctionState = 0;
                } else if (lbl_1_bss_118[lbl_1_bss_8[playerSlotIndex].targetReelIndex]
                               .settledSymbol ==
                           (s32) lbl_1_bss_1D0[lbl_1_bss_8[playerSlotIndex].teamIndex]
                               .reelSymbol[lbl_1_bss_8[playerSlotIndex].teamReelIndex]) {
                    aDirectionDistance = bDirectionDistance = 0;
                    lbl_1_bss_8[playerSlotIndex].correctionState = -1;
                    if ((s32) lbl_1_bss_1D0[lbl_1_bss_8[playerSlotIndex].teamIndex]
                            .alignmentState[lbl_1_bss_8[playerSlotIndex].teamReelIndex] == 0) {
                        lbl_1_bss_8[playerSlotIndex].pendingButtonPresses = 2;
                        lbl_1_bss_8[playerSlotIndex].inputDelayFrames = 0;
                        lbl_1_bss_8[playerSlotIndex].correctionState = 1;
                    }
                } else {
                    aDirectionDistance = bDirectionDistance = 4;
                    lbl_1_bss_8[playerSlotIndex].correctionState = 0;
                }
                if (aDirectionDistance < bDirectionDistance) {
                    lbl_1_bss_8[playerSlotIndex].buttonDirection = 0;
                    if (aDirectionDistance > 2) {
                        aDirectionPresses = 2;
                    } else {
                        aDirectionPresses = aDirectionDistance;
                    }
                    lbl_1_bss_8[playerSlotIndex].pendingButtonPresses = aDirectionPresses;
                } else if (aDirectionDistance > bDirectionDistance) {
                    lbl_1_bss_8[playerSlotIndex].buttonDirection = 1;
                    if (bDirectionDistance > 2) {
                        bDirectionPresses = 2;
                    } else {
                        bDirectionPresses = bDirectionDistance;
                    }
                    lbl_1_bss_8[playerSlotIndex].pendingButtonPresses = bDirectionPresses;
                } else if ((aDirectionDistance != 0) && (bDirectionDistance != 0)) {
                    lbl_1_bss_8[playerSlotIndex].buttonDirection = frandmod(2);
                    if (lbl_1_bss_8[playerSlotIndex].buttonDirection == 0) {
                        if (aDirectionDistance > 2) {
                            tieAPresses = 2;
                        } else {
                            tieAPresses = aDirectionDistance;
                        }
                        lbl_1_bss_8[playerSlotIndex].pendingButtonPresses = tieAPresses;
                    } else {
                        if (bDirectionDistance > 2) {
                            tieBPresses = 2;
                        } else {
                            tieBPresses = bDirectionDistance;
                        }
                        lbl_1_bss_8[playerSlotIndex].pendingButtonPresses = tieBPresses;
                    }
                } else if (lbl_1_bss_8[playerSlotIndex].correctionState != 1) {
                    lbl_1_bss_8[playerSlotIndex].pendingButtonPresses = 0;
                }
                lbl_1_bss_8[playerSlotIndex].inputPlanActive = 1;
                if (lbl_1_bss_8[playerSlotIndex].remainingDetours > 0) {
                    /* Difficulty adds a short detour by reversing the planned direction. */
                    lbl_1_bss_8[playerSlotIndex].buttonDirection ^= 1;
                    if (lbl_1_bss_8[playerSlotIndex].pendingButtonPresses == 1) {
                        alternateStepCount = 2;
                    } else {
                        if (lbl_1_bss_8[playerSlotIndex].pendingButtonPresses == 2) {
                            randomAlternateStepCount = 1;
                        } else {
                            randomAlternateStepCount = frandmod(2) + 1;
                        }
                        alternateStepCount = randomAlternateStepCount;
                    }
                    lbl_1_bss_8[playerSlotIndex].pendingButtonPresses = alternateStepCount;
                    lbl_1_bss_8[playerSlotIndex].remainingDetours -= 1;
                }
            }
            if (lbl_1_bss_8[playerSlotIndex].correctionState == 1) {
                if (lbl_1_bss_8[playerSlotIndex].inputDelayFrames == 0) {
                    if (lbl_1_bss_8[playerSlotIndex].pendingButtonPresses > 0) {
                        if (lbl_1_bss_8[playerSlotIndex].buttonDirection == 0) {
                            settleDirectionButton = PAD_BUTTON_A;
                        } else {
                            settleDirectionButton = PAD_BUTTON_B;
                        }
                        lbl_802C0008[lbl_80265728[lbl_1_bss_290[playerSlotIndex]].padNo] =
                            settleDirectionButton;
                        lbl_1_bss_8[playerSlotIndex].pendingButtonPresses -= 1;
                        lbl_1_bss_8[playerSlotIndex].inputDelayFrames = lbl_1_data_5C[1];
                        if ((lbl_1_bss_118[lbl_1_bss_8[playerSlotIndex].targetReelIndex]
                                 .settledSymbol ==
                             (s32) lbl_1_bss_1D0[lbl_1_bss_8[playerSlotIndex].teamIndex]
                                 .reelSymbol[lbl_1_bss_8[playerSlotIndex].teamReelIndex]) &&
                            ((s32) lbl_1_bss_1D0[lbl_1_bss_8[playerSlotIndex].teamIndex]
                                 .alignmentState[lbl_1_bss_8[playerSlotIndex].teamReelIndex] !=
                             0)) {
                            if (lbl_1_bss_8[playerSlotIndex].buttonDirection != 0) {
                                settleCorrectionButton = PAD_BUTTON_A;
                            } else {
                                settleCorrectionButton = PAD_BUTTON_B;
                            }
                            lbl_802C0008[lbl_80265728[lbl_1_bss_290[playerSlotIndex]].padNo] =
                                settleCorrectionButton;
                        }
                    }
                    if (lbl_1_bss_1D0[lbl_1_bss_8[playerSlotIndex].teamIndex]
                            .reelSpeed[lbl_1_bss_8[playerSlotIndex].teamReelIndex] == 0.0f) {
                        if (lbl_1_bss_8[playerSlotIndex].pendingButtonPresses == 0) {
                            lbl_1_bss_8[playerSlotIndex].inputPlanActive = 0;
                        } else if ((lbl_1_bss_118[lbl_1_bss_8[playerSlotIndex].targetReelIndex]
                                        .settledSymbol ==
                                    (s32) lbl_1_bss_1D0[lbl_1_bss_8[playerSlotIndex].teamIndex]
                                        .reelSymbol[lbl_1_bss_8[playerSlotIndex].teamReelIndex]) &&
                                   ((s32) lbl_1_bss_1D0[lbl_1_bss_8[playerSlotIndex].teamIndex]
                                        .alignmentState[lbl_1_bss_8[playerSlotIndex]
                                                            .teamReelIndex] != 0)) {
                            lbl_1_bss_8[playerSlotIndex].inputPlanActive = 0;
                        }
                    }
                } else {
                    lbl_1_bss_8[playerSlotIndex].inputDelayFrames -= 1;
                }
            } else if (lbl_1_bss_8[playerSlotIndex].inputDelayFrames == 0) {
                if (lbl_1_bss_8[playerSlotIndex].pendingButtonPresses > 0) {
                    if (lbl_1_bss_8[playerSlotIndex].buttonDirection == 0) {
                        spinDirectionButton = PAD_BUTTON_A;
                    } else {
                        spinDirectionButton = PAD_BUTTON_B;
                    }
                    lbl_802C0008[lbl_80265728[lbl_1_bss_290[playerSlotIndex]].padNo] =
                        spinDirectionButton;
                    lbl_1_bss_8[playerSlotIndex].pendingButtonPresses -= 1;
                    if ((lbl_1_bss_118[lbl_1_bss_8[playerSlotIndex].targetReelIndex]
                             .settledSymbol ==
                         (s32) lbl_1_bss_1D0[lbl_1_bss_8[playerSlotIndex].teamIndex]
                             .reelSymbol[lbl_1_bss_8[playerSlotIndex].teamReelIndex]) &&
                        ((s32) lbl_1_bss_1D0[lbl_1_bss_8[playerSlotIndex].teamIndex]
                             .alignmentState[lbl_1_bss_8[playerSlotIndex].teamReelIndex] != 0)) {
                        if (lbl_1_bss_8[playerSlotIndex].buttonDirection != 0) {
                            spinCorrectionButton = PAD_BUTTON_A;
                        } else {
                            spinCorrectionButton = PAD_BUTTON_B;
                        }
                        lbl_802C0008[lbl_80265728[lbl_1_bss_290[playerSlotIndex]].padNo] =
                            spinCorrectionButton;
                    }
                }
                if (lbl_1_bss_1D0[lbl_1_bss_8[playerSlotIndex].teamIndex]
                        .reelSpeed[lbl_1_bss_8[playerSlotIndex].teamReelIndex] == 0.0f) {
                    lbl_1_bss_8[playerSlotIndex].inputPlanActive = 0;
                    if (lbl_1_bss_8[playerSlotIndex].pendingButtonPresses == 0) {
                        lbl_1_bss_8[playerSlotIndex].inputDelayFrames = lbl_1_data_5C[0];
                    }
                }
            } else {
                lbl_1_bss_8[playerSlotIndex].inputDelayFrames -= 1;
            }
        }
        playerSlotIndex += 1;
    }
}
