/* Quiz selection, answer, score, and result-screen routines for the Mic Quiz overlay. */
#include "msm_se.h"
#include "messdir_enum.h"
#include "datadir_enum.h"

#define QUIZ_PARTICLE_BURST_SOUND 1301
#define QUIZ_BUZZ_IN_SOUND 1271
#define QUIZ_TIMER_TICK_SOUND 1269

/* Runs as a short-lived particle process, scattering and fading a quiz-mode sprite. */
void fn_1_53F0(void) {
    f32 angle;
    f32 direction;
    f32 amplitude;
    f32 positionX;
    f32 positionY;
    f32 transparency;
    s16 spriteGroup;
    s32 delay;
    s32 frame;

    direction = (2.0f) * frandf();
    amplitude = frandf();
    delay = frandmod(10);
    spriteGroup = HuSprGrpCreate(1);
    HuSprGrpMemberSet(spriteGroup, 0,
                     HuSprCreate(lbl_1_bss_1BC, 51, 0));
    HuSprAttrSet(spriteGroup, 0, 4);
    HuSprAttrSet(spriteGroup, 0, 1);
    HuPrcSleep(delay);
    positionX = (f32)((401.0) +
                      ((40.0) * ((f64)amplitude * sin((3.141592653589793) * (f64)direction))));
    positionY = (f32)((62.0) +
                      ((40.0) * ((f64)amplitude * cos((3.141592653589793) * (f64)direction))));
    HuSprPosSet(spriteGroup, 0, positionX, positionY);
    HuSprAttrReset(spriteGroup, 0, 4);
    HuSprAttrReset(spriteGroup, 0, 1);
    angle = (0.0f);
    for (frame = 0; frame < 60; frame++) {
        HuPrcVSleep();
        transparency = (f32)((1.0) -
                            sin(((3.141592653589793) * (f64)angle) / (180.0)));
        angle += (1.5f);
        if (frame <= 50) {
            HuSprTPLvlSet(spriteGroup, 0, transparency);
        }
    }
    HuSprGrpKill(spriteGroup);
    HuPrcEnd();
    /* Keep the process from falling through if HuPrcEnd returns unexpectedly. */
    for (;;) {
        HuPrcVSleep();
    }
}

/* Child process that pulses the group from its property; the first phase is uninitialized. */
void fn_1_5600(void) {
    f32 spriteScale;
    f32 transparency;
    f32 angleDegrees;
    HUPROCESS *currentProcess;
    s16 spriteId;

    currentProcess = HuPrcCurrentGet();
    spriteId = (s16)currentProcess->property;
    for (;;) {
        HuPrcVSleep();
        spriteScale = (f32)((1.0) +
            ((0.30000001192092896) * sin(((3.141592653589793) * angleDegrees) /
            (180.0))));
        transparency = (f32)((1.0) -
            sin(((3.141592653589793) * angleDegrees) / (180.0)));
        HuSprTPLvlSet(spriteId, 0, transparency);
        HuSprScaleSet(spriteId, 0, spriteScale, spriteScale);
        angleDegrees += (1.5f);
        if (!(angleDegrees >= (90.0f))) {
            continue;
        }
        angleDegrees = (0.0f);
    }
}

/* Builds the player/input sprite group and leaves it hidden for the selection display. */
void fn_1_571C(void) {
    s32 characterIndex;
    s32 playerIndex;

    lbl_1_bss_254 = HuPrcCurrentGet();
    lbl_1_bss_1E4 = (MicquizWork *)MgModeWork;
    lbl_1_bss_2D8[0] = HuSprGrpCreate(8);
    HuSprGrpPosSet(lbl_1_bss_2D8[0], 493.0f, 386.0f);
    playerIndex = 0;
    while (playerIndex < 4) {
        if (fn_1_9148(playerIndex) == 0) {
            lbl_1_bss_15C[playerIndex] = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_mgconst, 0), HU_MEMNUM_OVL, HEAP_MODEL));
            HuSprGrpMemberSet(lbl_1_bss_2D8[0], (s16) (playerIndex + 4),
                              HuSprCreate(lbl_1_bss_15C[playerIndex], 48, 0));
            HuSprPosSet(lbl_1_bss_2D8[0], (s16) (playerIndex + 4), -23.0f, -25.0f);
            HuSprAttrSet(lbl_1_bss_2D8[0], (s16) (playerIndex + 4), 4);
        } else {
            characterIndex = GwPlayerConf[playerIndex].charNo;
            lbl_1_bss_15C[playerIndex] = HuSprAnimRead(
                HuDataSelHeapReadNum(lbl_1_data_1C[characterIndex], HU_MEMNUM_OVL, HEAP_MODEL));
            HuSprGrpMemberSet(lbl_1_bss_2D8[0], (s16) (playerIndex + 4),
                              HuSprCreate(lbl_1_bss_15C[playerIndex], 48, 0));
            HuSprPosSet(lbl_1_bss_2D8[0], (s16) (playerIndex + 4), -23.0f, -25.0f);
            HuSprAttrSet(lbl_1_bss_2D8[0], (s16) (playerIndex + 4), 4);
        }
        playerIndex += 1;
    }
    lbl_1_bss_158 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 28), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_2D8[0], 0, HuSprCreate(lbl_1_bss_158, 49, 0));
    HuSprPosSet(lbl_1_bss_2D8[0], 0, 0.0f, 0.0f);
    lbl_1_bss_154 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 29), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_2D8[0], 1, HuSprCreate(lbl_1_bss_154, 49, 0));
    HuSprPosSet(lbl_1_bss_2D8[0], 1, 8.0f, 12.0f);
    lbl_1_bss_150 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 30), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_2D8[0], 2, HuSprCreate(lbl_1_bss_150, 49, 0));
    HuSprPosSet(lbl_1_bss_2D8[0], 2, 13.0f, 9.0f);
    lbl_1_bss_14C =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 32), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_2D8[0], 3, HuSprCreate(lbl_1_bss_14C, 49, 0));
    HuSprPosSet(lbl_1_bss_2D8[0], 3, -23.0f, -23.0f);
    HuSprAttrSet(lbl_1_bss_2D8[0], 2, 4);
    HuSprBankSet(lbl_1_bss_2D8[0], 0, 0);
    HuSprBankSet(lbl_1_bss_2D8[0], 1, 0);
    HuSprGrpAttrSet(lbl_1_bss_2D8[0], 4);
}

/* Scales the player-selection portraits into their chosen arrangement. */
void fn_1_5BB0(s32 mode) {
    f32 scale;
    f32 angle;
    s32 frame;

    scale = (0.0f);
    angle = (0.0f);
    HuSprGrpAttrReset(lbl_1_bss_2D8[0], 4);
    HuSprAttrSet(lbl_1_bss_2D8[0], 2, 4);
    for (frame = 0; frame < 4; frame++) {
        HuSprAttrSet(lbl_1_bss_2D8[0], (s16)(frame + 4), 4);
    }
    HuSprAttrSet(lbl_1_bss_2D8[0], 3, 4);
    HuSprGrpScaleSet(lbl_1_bss_2D8[0], (0.0f), (0.0f));
    fn_1_12B68(mode);
    do {
        HuPrcVSleep();
        scale = (f32)((1.2000000476837158) *
                      sin(((3.141592653589793) * (f64)angle) / (180.0)));
        HuSprGrpScaleSet(lbl_1_bss_2D8[0], scale, scale);
        angle += (4.5f);
    } while (!(angle >= (90.0f)));
    HuSprGrpScaleSet(lbl_1_bss_2D8[0], (1.2f), (1.2f));

    scale = (1.2f);
    angle = (0.0f);
    do {
        HuPrcVSleep();
        scale = (f32)((1.2000000476837158) -
                      ((0.4000000059604645) *
                       sin(((3.141592653589793) * (f64)angle) / (180.0))));
        HuSprGrpScaleSet(lbl_1_bss_2D8[0], scale, scale);
        angle += (18.0f);
    } while (!(angle >= (90.0f)));
    HuSprGrpScaleSet(lbl_1_bss_2D8[0], (0.8f), (0.8f));

    scale = (1.2f);
    angle = (0.0f);
    do {
        HuPrcVSleep();
        scale = (f32)((0.800000011920929) +
                      ((0.20000000298023224) *
                       sin(((3.141592653589793) * (f64)angle) / (180.0))));
        HuSprGrpScaleSet(lbl_1_bss_2D8[0], scale, scale);
        angle += (18.0f);
    } while (!(angle >= (90.0f)));
    HuSprGrpScaleSet(lbl_1_bss_2D8[0], (1.0f), (1.0f));
}

/* Shrinks the portrait group and hides it when the selection display closes. */
void fn_1_5EF0(void) {
    f32 angle;
    f32 scale;

    scale = (0.0f);
    angle = (0.0f);
    HuSprGrpScaleSet(lbl_1_bss_2D8[0], (1.0f), (1.0f));
    do {
        HuPrcVSleep();
        scale = (f32)((1.0) -
                      sin(((3.141592653589793) * (f64)angle) / (180.0)));
        HuSprGrpScaleSet(lbl_1_bss_2D8[0], scale, scale);
        angle += (3.0f);
    } while (!(angle >= (90.0f)));
    HuSprGrpAttrSet(lbl_1_bss_2D8[0], 4);
    HuSprGrpScaleSet(lbl_1_bss_2D8[0], (0.0f), (0.0f));
}

/* Creates the three score digits for each human player at result-screen setup. */
void fn_1_6024(void) {
    s32 playerIndex;
    s32 digitIndex;
    s32 score;
    s32 hundreds;
    s32 tens;
    s32 ones;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (fn_1_9148(playerIndex) != 0) {
            for (digitIndex = 0; digitIndex < 3; digitIndex++) {
                lbl_1_bss_F8[playerIndex][digitIndex] = HuSprAnimRead(
                    HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 86), HU_MEMNUM_OVL, HEAP_MODEL));
                lbl_1_bss_E0[playerIndex][digitIndex] = HuSprGrpCreate(1);
                HuSprGrpMemberSet(lbl_1_bss_E0[playerIndex][digitIndex], 0,
                                  HuSprCreate(lbl_1_bss_F8[playerIndex][digitIndex], 1500, 0));
                HuSprPosSet(lbl_1_bss_E0[playerIndex][digitIndex], 0,
                            (f32)(64 + (playerIndex << 7) + (digitIndex << 5)),
                            (275.0f));
                HuSprAttrSet(lbl_1_bss_E0[playerIndex][digitIndex], 0, 4);
            }

            score = lbl_1_bss_1E4->playerScores[playerIndex];
            hundreds = score / 100;
            tens = (score - (hundreds * 100)) / 10;
            ones = (score - (hundreds * 100)) - (tens * 10);
            HuSprBankSet(lbl_1_bss_E0[playerIndex][0], 0, hundreds);
            HuSprBankSet(lbl_1_bss_E0[playerIndex][1], 0, tens);
            HuSprBankSet(lbl_1_bss_E0[playerIndex][2], 0, ones);
        }
    }
}

/* Refreshes each human player's displayed score after the result changes. */
void fn_1_627C(void) {
    s32 playerIndex;
    s32 score;
    s32 hundreds;
    s32 tens;
    s32 ones;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (fn_1_9148(playerIndex) != 0) {
            score = lbl_1_bss_1E4->playerScores[playerIndex];
            hundreds = score / 100;
            tens = (score - hundreds * 100) / 10;
            ones = score - hundreds * 100 - tens * 10;
            HuSprBankSet(lbl_1_bss_E0[playerIndex][0], 0, hundreds);
            HuSprBankSet(lbl_1_bss_E0[playerIndex][1], 0, tens);
            HuSprBankSet(lbl_1_bss_E0[playerIndex][2], 0, ones);
        }
    }
}

/* Builds the result heading, player portraits, and score display. */
void fn_1_638C(void) {
    s32 player;
    s32 scorePlayer;
    s32 digit;
    s32 hundreds;
    s32 score;
    s32 tens;
    s32 ones;

    lbl_1_bss_14 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 101), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_144 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 102), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_140 = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_140, 0, HuSprCreate(lbl_1_bss_144, 1500, 0));
    HuSprPosSet(lbl_1_bss_140, 0, 288.0f, 64.0f);
    HuSprAttrSet(lbl_1_bss_140, 0, 4);

    for (player = 0; player < 4; player++) {
        lbl_1_bss_130[player] = HuSprAnimRead(
            HuDataSelHeapReadNum(player + DATANUM(DATA_micquiz, 104), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_128[player] = HuSprGrpCreate(1);
        HuSprGrpMemberSet(lbl_1_bss_128[player], 0, HuSprCreate(lbl_1_bss_130[player], 1500, 0));
        HuSprPosSet(lbl_1_bss_128[player], 0, (f32)((player << 7) + 96), 248.0f);
        HuSprAttrSet(lbl_1_bss_128[player], 0, 4);
    }

    fn_1_6024();

    lbl_1_bss_B8[0] = MgScoreInit(DATANUM(DATA_micquiz, 120));
    HuPrcSetStat(lbl_1_bss_B8[0]->proc, 4U);
    MgScoreMaxDigitSet(lbl_1_bss_B8[0], 2);
    MgScoreValueSet(lbl_1_bss_B8[0], lbl_1_bss_1E4->resultPanelValue);
    MgScorePosSet(lbl_1_bss_B8[0], 186.0f, 135.0f);
    MgScoreDispOff(lbl_1_bss_B8[0]);
    MgScorePriSet(lbl_1_bss_B8[0], 50);

    lbl_1_bss_DC =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 108), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_D8 = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_D8, 0, HuSprCreate(lbl_1_bss_DC, 1500, 0));
    HuSprPosSet(lbl_1_bss_D8, 0, 288.0f, 140.0f);
    HuSprAttrSet(lbl_1_bss_D8, 0, 4);

    lbl_1_bss_B8[1] = MgScoreInit(DATANUM(DATA_micquiz, 120));
    HuPrcSetStat(lbl_1_bss_B8[1]->proc, 4U);
    MgScoreMaxDigitSet(lbl_1_bss_B8[1], 2);
    MgScoreValueSet(lbl_1_bss_B8[1], lbl_1_bss_1E4->humanPlayerCount * 5);
    MgScorePosSet(lbl_1_bss_B8[1], 338.0f, 145.0f);
    MgScoreDispOff(lbl_1_bss_B8[1]);
    MgScorePriSet(lbl_1_bss_B8[1], 50);

    for (player = 0; player < 4; player++) {
        if (fn_1_9148(player) != 0) {
            scorePlayer = lbl_1_bss_1E4->playerCharacterNumbers[player];
            lbl_1_bss_C8[player] = HuSprAnimRead(HuDataSelHeapReadNum(
                scorePlayer + DATANUM(DATA_micquiz, 109), HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_bss_C0[player] = HuSprGrpCreate(1);
            HuSprGrpMemberSet(lbl_1_bss_C0[player], 0, HuSprCreate(lbl_1_bss_C8[player], 1500, 0));
            HuSprPosSet(lbl_1_bss_C0[player], 0, (f32)((player << 7) + 95), 214.0f);
            HuSprAttrSet(lbl_1_bss_C0[player], 0, 4);
        }
    }

    lbl_1_bss_B6 = HuWinExCreateFrame(16.0f, 344.0f, 544, 96, 23, 5);
    lbl_1_bss_B4 = HuWinExCreateFrame(16.0f, 344.0f, 544, 96, 13, 0);
    HuWinDispOff(lbl_1_bss_B6);
    HuWinPriSet(lbl_1_bss_B6, 50);
    HuWinDispOff(lbl_1_bss_B4);
    HuWinPriSet(lbl_1_bss_B4, 50);
    lbl_1_bss_88 = HuWinCreate(16.0f, 352.0f, 544, 128, 0);
    HuWinPosSet(lbl_1_bss_88, 16.0f, 352.0f);
    HuWinBGTPLvlSet(lbl_1_bss_88, 0.0f);
    HuWinMesSpeedSet(lbl_1_bss_88, 0);
    HuWinPriSet(lbl_1_bss_88, 50);
    HuWinAttrSet(lbl_1_bss_88, HUWIN_ATTR_ALIGN_CENTER);
}

/* Updates both score panels and the digit sprites during the result flow. */
void fn_1_6C90(void) {
    s32 playerIndex;
    s32 hundreds;
    s32 score;
    s32 tens;
    s32 ones;

    MgScoreValueSet(lbl_1_bss_B8[0], lbl_1_bss_1E4->resultPanelValue);
    MgScoreValueSet(lbl_1_bss_B8[1], lbl_1_bss_1E4->humanPlayerCount * 5);

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (fn_1_9148(playerIndex) != 0) {
            score = lbl_1_bss_1E4->playerScores[playerIndex];
            hundreds = score / 100;
            tens = (score - hundreds * 100) / 10;
            ones = score - hundreds * 100 - tens * 10;
            HuSprBankSet(lbl_1_bss_E0[playerIndex][0], 0, hundreds);
            HuSprBankSet(lbl_1_bss_E0[playerIndex][1], 0, tens);
            HuSprBankSet(lbl_1_bss_E0[playerIndex][2], 0, ones);
        }
    }
}

/* Shows the result sprites and score panels while the pause display is open. */
void fn_1_6DE4(void) {
    s32 playerIndex;
    s32 digitIndex;

    HuSprAttrReset(lbl_1_bss_140, 0, 4);
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        HuSprAttrReset(lbl_1_bss_128[playerIndex], 0, 4);
    }
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (fn_1_9148(playerIndex) != 0) {
            for (digitIndex = 0; digitIndex < 3; digitIndex++) {
                HuSprAttrReset(lbl_1_bss_E0[playerIndex][digitIndex], 0, 4);
            }
        }
    }
    HuSprAttrReset(lbl_1_bss_D8, 0, 4);
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (fn_1_9148(playerIndex) != 0) {
            HuSprAttrReset(lbl_1_bss_C0[playerIndex], 0, 4);
        }
    }
    MgScoreDispOn(lbl_1_bss_B8[0]);
    MgScoreDispOn(lbl_1_bss_B8[1]);
}

/* Hides the result sprites and score panels when the pause display closes. */
void fn_1_6F38(void) {
    s32 playerIndex;
    s32 digitIndex;

    HuSprAttrSet(lbl_1_bss_140, 0, 4);
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        HuSprAttrSet(lbl_1_bss_128[playerIndex], 0, 4);
    }
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        for (digitIndex = 0; digitIndex < 3; digitIndex++) {
            HuSprAttrSet(lbl_1_bss_E0[playerIndex][digitIndex], 0, 4);
        }
    }
    HuSprAttrSet(lbl_1_bss_D8, 0, 4);
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (fn_1_9148(playerIndex) != 0) {
            HuSprAttrSet(lbl_1_bss_C0[playerIndex], 0, 4);
        }
    }
    MgScoreDispOff(lbl_1_bss_B8[0]);
    MgScoreDispOff(lbl_1_bss_B8[1]);
}

/* Puts the selection and result UI on the normal draw layer after pause. */
void fn_1_707C(void) {
    s32 playerIndex;

    HuSprGrpDrawNoSet(lbl_1_bss_2D8[0], 0);
    HuSprDrawNoSet(lbl_1_bss_176, 0, 0);
    HuSprDrawNoSet(lbl_1_bss_174, 0, 0);
    HuSprDrawNoSet(lbl_1_bss_172, 0, 0);
    HuSprGrpDrawNoSet(lbl_1_bss_16C[0], 0);
    if (fn_1_9330() != 8) {
        HuSprDrawNoSet(lbl_1_bss_18A, 0, 0);
    }
    if (lbl_1_data_208 != -1) {
        HuWinDrawNoSet(lbl_1_data_208, 0);
    }
    HuWinDrawNoSet(lbl_1_bss_92[0], 0);
    HuWinDrawNoSet(lbl_1_bss_90, 0);
    HuWinDrawNoSet(lbl_1_bss_8E, 0);
    HuWinDrawNoSet(lbl_1_bss_8C, 0);
    HuSprDrawNoSet(lbl_1_bss_180[0], 0, 0);
    HuSprDrawNoSet(lbl_1_bss_17E, 0, 0);
    HuSprDrawNoSet(lbl_1_bss_17C, 0, 0);
    HuSprDrawNoSet(lbl_1_bss_17A, 0, 0);
    HuSprDrawNoSet(lbl_1_bss_9C, 0, 0);
    HuSprDrawNoSet(lbl_1_bss_9A, 0, 0);
    HuSprDrawNoSet(lbl_1_bss_178, 0, 0);
    if (lbl_1_data_20A != -1) {
        HuSprDrawNoSet(lbl_1_data_20A, 0, 0);
    }
    if (lbl_1_data_20C != -1) {
        HuSprDrawNoSet(lbl_1_data_20C, 0, 0);
    }
    if (lbl_1_data_20E != -1) {
        HuSprDrawNoSet(lbl_1_data_20E, 0, 0);
    }
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_1_data_210[playerIndex] != -1) {
            HuSprDrawNoSet(lbl_1_data_210[playerIndex], 0, 0);
        }
    }
    if (lbl_1_data_218 != -1) {
        HuSprDrawNoSet(lbl_1_data_218, 0, 0);
    }
    if (lbl_1_data_21A != -1) {
        HuSprDrawNoSet(lbl_1_data_21A, 0, 0);
    }
}

/* Moves the selection UI to draw layer 64 while its pause prompt is open. */
void fn_1_7380(void) {
    s32 playerIndex;

    HuSprGrpDrawNoSet(lbl_1_bss_2D8[0], 64);
    if (lbl_1_data_208 != -1) {
        HuWinDrawNoSet(lbl_1_data_208, 64);
    }
    HuWinDrawNoSet(lbl_1_bss_92[0], 64);
    HuWinDrawNoSet(lbl_1_bss_90, 64);
    HuWinDrawNoSet(lbl_1_bss_8E, 64);
    HuWinDrawNoSet(lbl_1_bss_8C, 64);
    HuSprDrawNoSet(lbl_1_bss_176, 0, 64);
    HuSprDrawNoSet(lbl_1_bss_174, 0, 64);
    HuSprDrawNoSet(lbl_1_bss_172, 0, 64);
    HuSprGrpDrawNoSet(lbl_1_bss_16C[0], 64);
    if (fn_1_9330() != 8) {
        HuSprDrawNoSet(lbl_1_bss_18A, 0, 64);
    }
    HuSprDrawNoSet(lbl_1_bss_180[0], 0, 64);
    HuSprDrawNoSet(lbl_1_bss_17E, 0, 64);
    HuSprDrawNoSet(lbl_1_bss_17C, 0, 64);
    HuSprDrawNoSet(lbl_1_bss_17A, 0, 64);
    HuSprDrawNoSet(lbl_1_bss_9C, 0, 64);
    HuSprDrawNoSet(lbl_1_bss_9A, 0, 64);
    HuSprDrawNoSet(lbl_1_bss_178, 0, 64);
    if (lbl_1_data_20A != -1) {
        HuSprDrawNoSet(lbl_1_data_20A, 0, 64);
    }
    if (lbl_1_data_20C != -1) {
        HuSprDrawNoSet(lbl_1_data_20C, 0, 64);
    }
    if (lbl_1_data_20E != -1) {
        HuSprDrawNoSet(lbl_1_data_20E, 0, 64);
    }
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_1_data_210[playerIndex] != -1) {
            HuSprDrawNoSet(lbl_1_data_210[playerIndex], 0, 64);
        }
    }
    if (lbl_1_data_218 != -1) {
        HuSprDrawNoSet(lbl_1_data_218, 0, 64);
    }
    if (lbl_1_data_21A != -1) {
        HuSprDrawNoSet(lbl_1_data_21A, 0, 64);
    }
}

/* Restores draw layer 0 for the selection UI after pause. */
void fn_1_7684(void) {
    HuSprGrpDrawNoSet(lbl_1_bss_1F8[0], 0);
    HuSprGrpDrawNoSet(lbl_1_bss_2D8[0], 0);
    if (lbl_1_data_208 != -1) {
        HuWinDrawNoSet(lbl_1_data_208, 0);
    }
    HuWinDrawNoSet(lbl_1_bss_92[0], 0);
    HuWinDrawNoSet(lbl_1_bss_90, 0);
    HuWinDrawNoSet(lbl_1_bss_8E, 0);
    HuWinDrawNoSet(lbl_1_bss_8C, 0);
}

/* Moves the overlay's selection groups to draw layer 64 during pause. */
void fn_1_7740(void) {
    HuSprGrpDrawNoSet(lbl_1_bss_1F8[0], 64);
    HuSprGrpDrawNoSet(lbl_1_bss_2D8[0], 64);
    if (lbl_1_data_208 != -1) {
        HuWinDrawNoSet(lbl_1_data_208, 64);
    }
    HuWinDrawNoSet(lbl_1_bss_92[0], 64);
    HuWinDrawNoSet(lbl_1_bss_90, 64);
    HuWinDrawNoSet(lbl_1_bss_8E, 64);
    HuWinDrawNoSet(lbl_1_bss_8C, 64);
}

/* Changes the quiz decoration groups' visibility, particles, and rocking process for the requested
 * state. */
void fn_1_77FC(s32 action) {
    if (action == 0) {
        if ((u32)lbl_1_bss_C != 0) {
            HuPrcKill(lbl_1_bss_C);
            lbl_1_bss_C = NULL;
        }
        HuSprAttrSet(lbl_1_bss_180[0], 0, 4);
        HuSprAttrSet(lbl_1_bss_17E, 0, 4);
    } else if (action == 1) {
        HuSprAttrReset(lbl_1_bss_180[0], 0, 4);
        HuSprAttrReset(lbl_1_bss_17E, 0, 4);
        if ((u32)lbl_1_bss_C == 0) {
            lbl_1_bss_C = HuPrcChildCreate(fn_1_E388, 100, 6144, 0, lbl_1_bss_254);
        }
    } else if (action == 3) {
        fn_1_7B1C();
        HuPrcSleep(20);
        HuSprAttrSet(lbl_1_bss_180[0], 0, 4);
        HuSprScaleSet(lbl_1_bss_180[0], 0, (0.0f), (0.0f));
        HuSprGrpAttrSet(lbl_1_bss_17E, 4);
        HuSprGrpAttrReset(lbl_1_bss_17C, 4);
    } else if (action == 2) {
        if ((u32)lbl_1_bss_C != 0) {
            HuPrcKill(lbl_1_bss_C);
            lbl_1_bss_C = NULL;
        }
        fn_1_7BD0();
        HuPrcSleep(30);
        HuSprAttrSet(lbl_1_bss_17E, 0, 4);
        HuSprAttrSet(lbl_1_bss_17C, 0, 4);
        HuSprScaleSet(lbl_1_bss_17E, 0, (0.0f), (0.0f));
    } else if (action == 4) {
        if ((u32)lbl_1_bss_C != 0) {
            HuPrcKill(lbl_1_bss_C);
            lbl_1_bss_C = NULL;
        }
        HuAudFXPlay(MSM_SE_BRD00_45);
        fn_1_7B1C();
        fn_1_7B78();
        HuPrcSleep(30);
        HuSprAttrSet(lbl_1_bss_180[0], 0, 4);
        HuSprScaleSet(lbl_1_bss_180[0], 0, (0.0f), (0.0f));
        HuSprAttrSet(lbl_1_bss_17E, 0, 4);
        HuSprScaleSet(lbl_1_bss_17E, 0, (0.0f), (0.0f));
    }
}

/* Starts twenty short-lived sparkles around the upper-right quiz decoration. */
void fn_1_7B1C(void) {
    s32 particleIndex;

    particleIndex = 0;
    while (particleIndex < 20) {
        HuPrcChildCreate(fn_1_7C28, 100U, 6144U, 0, lbl_1_bss_254);
        particleIndex += 1;
    }
}

/* Starts ten particles for the selection sparkle animation. */
void fn_1_7B78(void) {
    s32 particleIndex;

    particleIndex = 0;
    while (particleIndex < 10) {
        HuPrcChildCreate(fn_1_7F34, 100U, 6144U, 0, HuPrcCurrentGet());
        particleIndex += 1;
    }
}

/* Starts ten particles for the alternate quiz-decoration transition. */
void fn_1_7BD0(void) {
    s32 particleIndex;

    particleIndex = 0;
    while (particleIndex < 10) {
        HuPrcChildCreate(fn_1_81CC, 100U, 6144U, 0, HuPrcCurrentGet());
        particleIndex += 1;
    }
}

/* Animates and removes one randomly placed selection sparkle. */
void fn_1_7C28(void) {
    s16 sleep;
    ANIMDATA *animation;
    HUSPR_GROUPID group;
    f32 transparency;
    f32 step;
    f32 rotation;
    f32 scale;
    f32 position[2];

    animation = HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 34), HU_MEMNUM_OVL, 2));
    group = HuSprGrpCreate(1);
    HuSprGrpMemberSet(group, 0, HuSprCreate(animation, 50, 0));
    step = (3.0f);
    position[0] =
        (f32) (sin((u8) frand() * (1.4117647f) * (3.141592653589793) / (180.0)) * (25.0) + (506.0));
    position[1] =
        (f32) (cos((u8) frand() * (1.4117647f) * (3.141592653589793) / (180.0)) * (25.0) + (65.0));
    HuSprGrpPosSet(group, position[0], position[1]);
    HuSprScaleSet(group, 0, (0.0f), (0.0f));
    rotation = (0.0f);
    scale = (0.0f);
    transparency = (1.0f);
    sleep = frandmod(20);
    HuPrcSleep(sleep);
    for (;;) {
        if (lbl_1_bss_1E4->drawMode != 0) {
            HuSprGrpDrawNoSet(group, 64);
        } else {
            HuSprGrpDrawNoSet(group, 0);
        }
        HuPrcVSleep();
        scale = (f32)(sin((3.141592653589793) * rotation / (180.0)) * (1.5));
        transparency = (f32)((1.0) - sin((3.141592653589793) * rotation / (180.0)));
        /* The same computed scale also supplies the sprite's z rotation. */
        HuSprZRotSet(group, 0, scale);
        HuSprScaleSet(group, 0, scale, scale);
        HuSprTPLvlSet(group, 0, transparency);
        rotation += step;
        if (rotation >= (90.0f)) {
            break;
        }
    }
    HuSprGrpKill(group);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Animates one randomly placed sparkle for fn_1_7B78, then removes its sprite group. */
void fn_1_7F34(void) {
    s16 sleep;
    ANIMDATA *animation;
    HUSPR_GROUPID group;
    f32 transparency;
    f32 step;
    f32 rotation;
    f32 scale;
    f32 position[2];

    animation = HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 35), HU_MEMNUM_OVL, 2));
    group = HuSprGrpCreate(1);
    HuSprGrpMemberSet(group, 0, HuSprCreate(animation, 50, 0));
    step = (3.0f);
    position[0] =
        (f32) (sin((u8) frand() * (1.4117647f) * (3.141592653589793) / (180.0)) * (45.0) + (504.0));
    position[1] = (99.0f);
    HuSprGrpPosSet(group, position[0], position[1]);
    HuSprScaleSet(group, 0, (0.0f), (0.0f));
    rotation = (0.0f);
    scale = (0.0f);
    transparency = (1.0f);
    sleep = frandmod(20);
    HuPrcSleep(sleep);
    for (;;) {
        if (lbl_1_bss_1E4->drawMode != 0) {
            HuSprGrpDrawNoSet(group, 64);
        } else {
            HuSprGrpDrawNoSet(group, 0);
        }
        HuPrcVSleep();
        scale = (f32)(sin((3.141592653589793) * rotation / (180.0)) * (1.5));
        transparency = (f32)((1.0) - sin((3.141592653589793) * rotation / (180.0)));
        /* The same computed scale also supplies the sprite's z rotation. */
        HuSprZRotSet(group, 0, scale);
        HuSprScaleSet(group, 0, scale, scale);
        HuSprTPLvlSet(group, 0, transparency);
        rotation += step;
        if (rotation >= (90.0f)) {
            break;
        }
    }
    HuSprGrpKill(group);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Animates one narrow-spread sparkle for fn_1_7BD0, then removes its sprite group. */
void fn_1_81CC(void) {
    s16 sleep;
    ANIMDATA *animation;
    HUSPR_GROUPID group;
    f32 transparency;
    f32 step;
    f32 rotation;
    f32 scale;
    f32 position[2];

    animation = HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 35), HU_MEMNUM_OVL, 2));
    group = HuSprGrpCreate(1);
    HuSprGrpMemberSet(group, 0, HuSprCreate(animation, 50, 0));
    step = (3.0f);
    position[0] =
        (f32) (sin((u8) frand() * (1.4117647f) * (3.141592653589793) / (180.0)) * (25.0) + (504.0));
    position[1] = (99.0f);
    HuSprGrpPosSet(group, position[0], position[1]);
    HuSprScaleSet(group, 0, (0.0f), (0.0f));
    rotation = (0.0f);
    scale = (0.0f);
    transparency = (1.0f);
    sleep = frandmod(20);
    HuPrcSleep(sleep);
    for (;;) {
        if (lbl_1_bss_1E4->drawMode != 0) {
            HuSprGrpDrawNoSet(group, 64);
        } else {
            HuSprGrpDrawNoSet(group, 0);
        }
        HuPrcVSleep();
        scale = (f32)(sin((3.141592653589793) * rotation / (180.0)) * (1.5));
        transparency = (f32)((1.0) - sin((3.141592653589793) * rotation / (180.0)));
        /* The same computed scale also supplies the sprite's z rotation. */
        HuSprZRotSet(group, 0, scale);
        HuSprScaleSet(group, 0, scale, scale);
        HuSprTPLvlSet(group, 0, transparency);
        rotation += step;
        if (rotation >= (90.0f)) {
            break;
        }
    }
    HuSprGrpKill(group);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* While the input effect is active, starts a new shrinking sprite every 20 scheduler ticks. */
void fn_1_8464(void) {
    for (;;) {
        HuPrcChildCreate(fn_1_84A0, 100U, 6144U, 0, lbl_1_bss_254);
        HuPrcSleep(20);
    }
}

/* Child process that shrinks and reveals one input-effect sprite, then removes it. */
void fn_1_84A0(void) {
    ANIMDATA *animation;
    s16 spriteGroup;
    f32 scalePhase;
    f32 transparencyPhase;
    f32 scale;
    f32 transparency;

    animation =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 31), HU_MEMNUM_OVL, HEAP_MODEL));
    spriteGroup = HuSprGrpCreate(1);
    HuSprGrpMemberSet(spriteGroup, 0, HuSprCreate(animation, 47, 0));
    HuSprPosSet(spriteGroup, 0, (516.0f), (362.0f));
    HuSprTPLvlSet(spriteGroup, 0, (0.0f));

    scalePhase = (90.0f);
    transparencyPhase = (0.0f);
    do {
        if (lbl_1_bss_1E4->drawMode != 0) {
            HuSprGrpDrawNoSet(spriteGroup, 64);
        } else {
            HuSprGrpDrawNoSet(spriteGroup, 0);
        }
        HuPrcVSleep();
        scale = (f32)sin(((3.141592653589793) * (f64)scalePhase) / (180.0));
        transparency = (f32)sin(((3.141592653589793) * (f64)transparencyPhase) / (180.0));
        HuSprScaleSet(spriteGroup, 0, scale, scale);
        HuSprTPLvlSet(spriteGroup, 0, transparency);
        scalePhase += (2.0f);
        transparencyPhase += (2.0f);
        /* This local clamp occurs after the alpha update and does not change the drawn sprite. */
        if (transparency >= (1.0f)) {
            transparency = (1.0f);
        }
    } while (!(scalePhase >= (180.0f)));
    HuSprGrpKill(spriteGroup);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Plays the selection sound and starts the selected sprite's blink process. */
void fn_1_868C(s32 selectionState) {
    s32 unused;

    if (selectionState) {
        HuAudFXPlay(MSM_SE_CMN_11);
        lbl_1_bss_84 = lbl_1_bss_9C;
        fn_1_F820(3);
    } else {
        HuAudFXPlay(MSM_SE_CMN_12);
        lbl_1_bss_84 = lbl_1_bss_9A;
    }
    HuPrcChildCreate(fn_1_8728, 100U, 6144U, 0, lbl_1_bss_254);
}

/* Child process that flashes the selected result marker twice, with 30 ticks between changes. */
void fn_1_8728(void) {
    HuSprAttrReset(lbl_1_bss_84, 0, 4);
    HuPrcSleep(30);
    HuSprAttrSet(lbl_1_bss_84, 0, 4);
    HuPrcSleep(30);
    HuSprAttrReset(lbl_1_bss_84, 0, 4);
    HuPrcSleep(30);
    HuSprAttrSet(lbl_1_bss_84, 0, 4);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Continuously pulses portrait-group member 1 while its animation process is active. */
void fn_1_87B8(void) {
    s32 frame = 0;
    f32 angle = (0.0f);

    for (; ; frame++) {
        f32 scale;

        HuPrcVSleep();
        scale = (f32)((1.0) + ((0.30000001192092896) *
                 sin(((3.141592653589793) * (f64)angle) / (180.0))));
        HuSprScaleSet(lbl_1_bss_2D8[0], 1, scale, scale);
        angle += (5.0f);
        if (angle >= (180.0f)) {
            angle = (0.0f);
        }
    }
}

/* Chooses the quiz-mode decoration transition for an accepted or rejected answer. */
void fn_1_888C(s32 selectionAccepted, s32 responseMode) {
    s32 index;

    if (responseMode == 0 || responseMode == 1) {
        if (selectionAccepted != 0) {
            for (index = 0; index < 15; index++) {
                HuPrcChildCreate(fn_1_53F0, 100U, 6144U, 0, lbl_1_bss_254);
            }
            fn_1_8E1C();
            return;
        }
        fn_1_8F74();
        return;
    }
    if (responseMode == 3) {
        if (selectionAccepted != 0) {
            fn_1_8F74();
            return;
        }
        fn_1_899C();
        return;
    }
    if (responseMode == 6) {
        if (selectionAccepted != 0) {
            for (index = 0; index < 15; index++) {
                HuPrcChildCreate(fn_1_53F0, 100U, 6144U, 0, lbl_1_bss_254);
            }
            fn_1_8E1C();
            return;
        }
        fn_1_8F74();
    }
}

/* For the mode-3 rejection transition, gathers 15 particles, reveals a decoration, then wipes it
 * away. */
void fn_1_899C(void) {
    f32 angle = (0.0f);
    f32 transparency;
    s32 index;

    HuAudFXPlay(QUIZ_PARTICLE_BURST_SOUND);
    for (index = 0; index < 15; index++) {
        HuPrcChildCreate(fn_1_8BAC, 100U, 6144U, 0, lbl_1_bss_254);
    }
    HuPrcSleep(45);
    HuSprTPLvlSet(lbl_1_bss_1B4, 0, (0.0f));
    HuSprAttrReset(lbl_1_bss_1B4, 0, 4);
    do {
        if (lbl_1_bss_1E4->drawMode != 0) {
            HuSprGrpDrawNoSet(lbl_1_bss_1B4, 64);
        } else {
            HuSprGrpDrawNoSet(lbl_1_bss_1B4, 0);
        }
        HuPrcVSleep();
        transparency = (f32)sin(((3.141592653589793) * (f64)angle) / (180.0));
        HuSprTPLvlSet(lbl_1_bss_1B4, 0, transparency);
        angle += (18.0f);
    } while (!(angle >= (90.0f)));
    fn_1_12A6C(0);
    WipeCreate(2, 6, 5);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    HuSprAttrSet(lbl_1_bss_18A, 0, 4);
    HuSprAttrSet(lbl_1_bss_1B4, 0, 4);
    WipeCreate(1, 6, 5);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    fn_1_12A6C(1);
}

/* Runs as a quiz decoration process, moving a particle toward the quiz-mode indicator. */
void fn_1_8BAC(void) {
    f32 stepX;
    f32 stepY;
    f32 originX;
    f32 originY;
    f32 randomAngle;
    f32 randomRadius;
    f32 positionX;
    f32 positionY;
    s16 particleGroup;
    s32 startDelay;
    s32 frame;
    randomAngle = (2.0f) * frandf();
    randomRadius = frandf();
    startDelay = frandmod(10);
    particleGroup = HuSprGrpCreate(1);
    HuSprGrpMemberSet(particleGroup, 0, HuSprCreate(lbl_1_bss_1D8[0], 49, 0));
    HuSprAttrSet(particleGroup, 0, 4);
    HuSprAttrSet(particleGroup, 0, 1);
    HuPrcSleep(startDelay);
    originX = (401.0f);
    originY = (62.0f);
    positionX =
    (f32) ((f64) originX +
        ((35.0) * ((f64) randomRadius * sin((3.141592653589793) * (f64) randomAngle))));
    positionY =
    (f32) ((f64) originY +
        ((35.0) * ((f64) randomRadius * cos((3.141592653589793) * (f64) randomAngle))));
    stepX = (originX - positionX) / (45.0f);
    stepY = (originY - positionY) / (45.0f);
    HuSprAttrReset(particleGroup, 0, 4);
    HuSprAttrReset(particleGroup, 0, 1);
    frame = 0;
    while (frame < 45)
    {
        if ((s32) lbl_1_bss_1E4->drawMode != 0)
        {
            HuSprGrpDrawNoSet(particleGroup, 64);
        }
        else
        {
            HuSprGrpDrawNoSet(particleGroup, 0);
        }
        HuPrcVSleep();
        positionX += stepX;
        positionY += stepY;
        HuSprPosSet(particleGroup, 0, positionX, positionY);
        if (frame == 10)
        {
            HuSprGrpKill(particleGroup);
            particleGroup = HuSprGrpCreate(1);
            HuSprGrpMemberSet(particleGroup, 0, HuSprCreate(lbl_1_bss_1D8[1], 49, 0));
            HuSprPosSet(particleGroup, 0, positionX, positionY);
        }
        frame += 1;
    }
    HuSprGrpKill(particleGroup);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Expands and fades the quiz-mode indicator after an accepted answer, then hides it. */
void fn_1_8E1C(void) {
    f32 transparency;
    f32 scale;
    f32 angle;

    scale = (1.0f);
    transparency = (1.0f);
    angle = (0.0f);
    do {
        HuPrcVSleep();
        HuSprScaleSet(lbl_1_bss_18A, 0, scale, scale);
        HuSprTPLvlSet(lbl_1_bss_18A, 0, transparency);
        scale = (f32) ((1.0) + sin(((3.141592653589793) * (f64) angle) / (180.0)));
        transparency = (f32) ((1.0) - sin(((3.141592653589793) * (f64) angle) / (180.0)));
        angle += (1.5f);
    } while (!(angle >= (90.0f)));
    HuSprAttrSet(lbl_1_bss_18A, 0, 4);
}

/* Shrinks and fades the quiz-mode indicator for the answer transition, then hides it. */
void fn_1_8F74(void) {
    f32 transparency;
    f32 scale;
    f32 angle;

    scale = (1.0f);
    transparency = (1.0f);
    angle = (0.0f);
    do {
        HuPrcVSleep();
        HuSprScaleSet(lbl_1_bss_18A, 0, scale, scale);
        HuSprTPLvlSet(lbl_1_bss_18A, 0, transparency);
        scale = (f32) ((1.0) - sin(((3.141592653589793) * (f64) angle) / (180.0)));
        transparency = (f32) ((1.0) - sin(((3.141592653589793) * (f64) angle) / (180.0)));
        angle += (3.0f);
    } while (!(angle >= (90.0f)));
    HuSprAttrSet(lbl_1_bss_18A, 0, 4);
}

void fn_1_90CC(void)
{
}

void fn_1_90D0(void)
{
}

u8 fn_1_90D4(void) {
    return lbl_1_bss_1E4->difficultyIndex;
}

u8 fn_1_90E8(void) {
    return lbl_1_bss_1E4->selectedQuizMode;
}

/* Stores the responding player, remaining countdown, and success value when an answer flow ends. */
void fn_1_90FC(s8 responsePlayerIndex, u16 remainingCountdown, s32 answerSucceeded) {
    lbl_1_bss_1E4->responsePlayerIndex = responsePlayerIndex;
    lbl_1_bss_1E4->remainingCountdown = remainingCountdown;
    lbl_1_bss_1E4->answerSucceeded = answerSucceeded;
}

s32 fn_1_9134(void) {
    return lbl_1_bss_1E4->currentPlayerIndex;
}

s32 fn_1_9148(s32 playerIndex) {
    return lbl_1_bss_1E4->isHumanPlayer[playerIndex];
}

/* Returns the fractional countdown remaining, normalized by the difficulty's initial timer
 * value. */
f32 fn_1_9160(void) {
    u8 nextIndex;
    u8 previousIndex;
    u8 currentIndex;
    s32 selectedIndex;

    if (fn_1_9330() == 2) {
        nextIndex = lbl_1_bss_1E4->difficultyIndex;
        selectedIndex = nextIndex + 1;
    } else if (fn_1_9330() == 5) {
        previousIndex = lbl_1_bss_1E4->difficultyIndex;
        selectedIndex = previousIndex - 1;
    } else {
        currentIndex = lbl_1_bss_1E4->difficultyIndex;
        selectedIndex = currentIndex;
    }
    return ((f32)lbl_1_bss_22C - ((f32)lbl_1_bss_224 / (f32)lbl_1_bss_228)) /
           (f32)lbl_1_data_48[selectedIndex];
}

void fn_1_92D4(void) {
    HuPrcSleep(120);
}

void fn_1_92F8(void) {
    HuPrcSleep(60);
}

s32 fn_1_931C(void) {
    return lbl_1_bss_1E4->useAlternatePrompt;
}

s32 fn_1_9330(void) {
    return lbl_1_bss_1E4->quizSubmode;
}

void fn_1_9344(void) {
    lbl_1_bss_1E4->answerFlowPending = 1;
}

void fn_1_935C(void) {
    lbl_1_bss_1E4->answerFlowPending = 0;
}

/* Shows the current player's character name in the initial answer prompt and waits for the
 * message. */
void fn_1_9374(void) {
    if (lbl_1_bss_1E4->useAlternatePrompt == 0) {
        fn_1_1CD0(0);
        fn_1_1E64(
            0,
            lbl_1_data_A0[lbl_1_bss_1E4->playerCharacterNumbers[lbl_1_bss_1E4->currentPlayerIndex]],
            0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 13), 1, 0);
    } else {
        fn_1_1CD0(3);
        fn_1_1E64(
            3,
            lbl_1_data_A0[lbl_1_bss_1E4->playerCharacterNumbers[lbl_1_bss_1E4->currentPlayerIndex]],
            0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 76), 1, 3);
    }
    fn_1_1DF0();
}

/* Shows a supplied answer message in the initial reply template and waits for it. */
void fn_1_9620(u32 message) {
    if (lbl_1_bss_1E4->useAlternatePrompt == 0) {
        fn_1_1CD0(1);
        fn_1_1E64(1, message, 0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 16), 1, 1);
    } else {
        fn_1_1CD0(3);
        fn_1_1E64(3, message, 0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 79), 1, 3);
    }
    fn_1_1DF0();
}

/* Shows a supplied answer message in the retry reply template and waits for it. */
void fn_1_9860(u32 message) {
    if (lbl_1_bss_1E4->useAlternatePrompt == 0) {
        fn_1_1CD0(1);
        fn_1_1E64(1, message, 0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 27), 1, 1);
    } else {
        fn_1_1CD0(3);
        fn_1_1E64(3, message, 0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 83), 1, 3);
    }
    fn_1_1DF0();
}

/* Logs the timer state and shows the response-error prompt unless the timer has expired. */
void fn_1_9AA0(void) {
    OSReport("SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSScoreMode:%d\n", lbl_1_bss_230);
    if (lbl_1_bss_230 != 2) {
        if (lbl_1_bss_1E4->useAlternatePrompt == 0) {
            fn_1_1CD0(1);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 48), 1, 1);
        } else {
            fn_1_1CD0(3);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 102), 1, 3);
        }
        fn_1_1DF0();
    }
}

/* Shows and waits for the answer-timeout prompt in the normal or alternate speaker window. */
void fn_1_9CCC(void) {
    if (lbl_1_bss_1E4->useAlternatePrompt == 0) {
        fn_1_1CD0(1);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 17), 1, 1);
    } else {
        fn_1_1CD0(3);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 80), 1, 3);
    }
    fn_1_1DF0();
}

/* Shows and waits for the prompt inviting another player to answer after the first attempt
 * fails. */
void fn_1_9EC8(void) {
    if (lbl_1_bss_1E4->useAlternatePrompt == 0) {
        fn_1_1CD0(0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 25), 1, 0);
    } else {
        fn_1_1CD0(3);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 81), 1, 3);
    }
    if (lbl_1_bss_86 != -1) {
        HuWinMesWait(lbl_1_bss_86);
    }
}

/* Shows the selected player's character-name prompt unless the timer has expired. */
void fn_1_A0C4(s32 playerIndex) {
    if (lbl_1_bss_230 != 2) {
        if (lbl_1_bss_1E4->useAlternatePrompt == 0) {
            fn_1_1CD0(0);
            fn_1_1E64(0, lbl_1_data_A0[lbl_1_bss_1E4->playerCharacterNumbers[playerIndex]], 0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 26), 1, 0);
        } else {
            fn_1_1CD0(3);
            fn_1_1E64(3, lbl_1_data_A0[lbl_1_bss_1E4->playerCharacterNumbers[playerIndex]], 0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 82), 1, 3);
        }
        if (lbl_1_bss_86 != -1) {
            HuWinMesWait(lbl_1_bss_86);
        }
    }
}

/* Shows and waits for the normal or alternate reply message selected by this helper. */
void fn_1_A368(void) {
    if (lbl_1_bss_1E4->useAlternatePrompt == 0) {
        fn_1_1CD0(1);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 28), 1, 1);
    } else {
        fn_1_1CD0(3);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 84), 1, 3);
    }
    if (lbl_1_bss_86 != -1) {
        HuWinMesWait(lbl_1_bss_86);
    }
}

/* Shows the supplied answer in the reveal template, waits, and closes the prompt window. */
void fn_1_A564(u32 message) {
    if (lbl_1_bss_1E4->useAlternatePrompt == 0) {
        fn_1_1CD0(0);
        fn_1_1E64(0, message, 0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 29), 1, 0);
    } else {
        fn_1_1CD0(3);
        fn_1_1E64(3, message, 0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 85), 1, 3);
    }
    fn_1_1DF0();
    fn_1_1C80();
}

/* Shows and waits for the success message after an answer is accepted. */
void fn_1_A7D8(void) {
    if (lbl_1_bss_1E4->useAlternatePrompt == 0) {
        fn_1_1CD0(2);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 40), 1, 2);
    } else {
        fn_1_1CD0(3);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 96), 1, 3);
    }
    fn_1_1DF0();
}

/* Shows and waits for the normal or alternate follow-up message selected by this helper. */
void fn_1_A9D4(void) {
    if (lbl_1_bss_1E4->useAlternatePrompt == 0) {
        fn_1_1CD0(1);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 49), 1, 1);
    } else {
        fn_1_1CD0(3);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 103), 1, 3);
    }
    fn_1_1DF0();
}

/* Shows and waits for the repeated-answer prompt when another attempt is requested. */
void fn_1_ABD0(void) {
    if (lbl_1_bss_1E4->useAlternatePrompt == 0) {
        fn_1_1CD0(1);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 50), 1, 1);
    } else {
        fn_1_1CD0(3);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 104), 1, 3);
    }
    fn_1_1DF0();
}

/* Shows and waits for the response-status prompt unless the timer has expired. */
void fn_1_ADCC(void) {
    if (lbl_1_bss_230 != 2) {
        if (lbl_1_bss_1E4->useAlternatePrompt == 0) {
            fn_1_1CD0(0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 55), 1, 0);
        } else {
            fn_1_1CD0(3);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 114), 1, 3);
        }
        fn_1_1DF0();
    }
}

/* Shows and waits for the prompt used before the answer flow switches to another attempt. */
void fn_1_AFDC(void) {
    if (lbl_1_bss_1E4->useAlternatePrompt == 0) {
        fn_1_1CD0(0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ_Q, 2), 1, 0);
    } else {
        fn_1_1CD0(3);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ_Q, 42), 1, 3);
    }
    fn_1_1DF0();
}

/* Shows an introduction message, delays 120 ticks, then addresses the selected player's
 * character. */
void fn_1_B1D8(s32 playerIndex) {
    fn_1_1CD0(0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ_I, 20), 1, 0);
    fn_1_1DF0();
    HuPrcSleep(120);
    fn_1_1CD0(1);
    fn_1_1E64(1, lbl_1_data_A0[lbl_1_bss_1E4->playerCharacterNumbers[playerIndex]], 0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ_I, 4), 1, 1);
    fn_1_1DF0();
}

/* Initializes speech input, creates pad-answer window storage, and loads the supplied word
 * context. */
void fn_1_B434(char *path, s32 mode) {
    lbl_1_bss_204 = mode;
    HuMCInit(0);
    HuMCSelWinCreate((-10000.0f), (88.0f));
    lbl_1_bss_218 = HuMCContextCreate(path);
}

/* Initializes answer input and chooses a speech context for numbers, characters, NPCs, or fruit. */
void fn_1_B4A0(s32 contextKind) {
    lbl_1_bss_204 = contextKind;
    HuMCInit(0);
    HuMCSelWinCreate((-10000.0f), (88.0f));
    if (contextKind == 0) {
        lbl_1_bss_218 = HuMCContextCreate("/mic/ctx/micquiz_num");
    } else if (contextKind == 1) {
        lbl_1_bss_218 = HuMCContextCreate("/mic/ctx/micquiz_num2");
    } else if (contextKind == 2) {
        lbl_1_bss_218 = HuMCContextCreate("/mic/ctx/micquiz_chara");
    } else if (contextKind == 3) {
        lbl_1_bss_218 = HuMCContextCreate("/mic/ctx/micquiz_chara");
    } else if (contextKind == 4) {
        lbl_1_bss_218 = HuMCContextCreate("/mic/ctx/micquiz_npc");
    } else if (contextKind == 5) {
        lbl_1_bss_218 = HuMCContextCreate("/mic/ctx/micquiz_kurabe1");
    } else if (contextKind == 6) {
        lbl_1_bss_218 = HuMCContextCreate("/mic/ctx/micquiz_fruit");
    } else {
        lbl_1_bss_218 = HuMCContextCreate("/mic/ctx/micquiz_chara");
    }
}

void fn_1_B60C(void) {
    lbl_1_bss_254 = HuPrcCurrentGet();
}

void fn_1_B638(void) {
    lbl_1_bss_254 = HuPrcCurrentGet();
}

/* Runs the answer flow through initial and buzz-in attempts, records the outcome, then yields
 * indefinitely. */
void fn_1_B664(s32 quizMode) {
    f32 firstVerticalOffset;
    f32 outgoingAmplitude;
    f32 secondVerticalOffset;
    f32 incomingAmplitude;
    void *motionProcess;
    HUPROCESS *inputProcess;
    f32 verticalOffset;
    f32 alternateVerticalOffset;
    f64 motionAngle;
    f64 returnAngle;
    f64 messageMotionAngle;
    f64 initialMotionAngle;
    f64 closingAngle;
    f64 openingAngle;
    s32 hasSelection;
    s32 selectedAnswer;
    s32 currentAnswer;
    s32 currentPlayer;
    s32 playerIndex;
    u32 *messageEntry;
    u32 *alternateMessageEntry;
    s32 initialPlayer;

    hasSelection = 0;
    if (quizMode == 3 || quizMode == 2) {
        if ((s32) lbl_1_bss_220 >= 9) {
            lbl_1_bss_220 -= 1;
        }
    } else if (quizMode == 4) {
        if ((s32) lbl_1_bss_220 >= 15) {
            lbl_1_bss_220 -= 1;
        }
    }
    lbl_1_bss_208 = 0;
    lbl_1_bss_234 = 0;
    lbl_1_bss_250 = quizMode;
    lbl_1_bss_24C = 0;
    lbl_1_bss_244 = 0;
    lbl_1_bss_240 = 1;
    lbl_1_bss_4 = 0;
    fn_1_935C();
    lbl_1_bss_1E4->initialAnswerPlayer = (s8)(lbl_1_bss_248 = fn_1_9134());
    fn_1_EC18(lbl_1_bss_204, lbl_1_bss_248);
    fn_1_12D78();
    fn_1_E8A4(0);
    fn_1_12A6C(0);
    lbl_1_bss_230 = 1;
    do {
        HuPrcVSleep();
    } while ((s32) lbl_1_bss_208 == 0);
    fn_1_F820(0);
    lbl_1_bss_214 = 0;
    HuPrcVSleep();
    lbl_1_bss_214 = 1;
    HuPrcSleep(30);
    lbl_1_bss_230 = 0;
    fn_1_77FC(1);
    fn_1_12D78();
    fn_1_E8A4(1);
    motionProcess = HuPrcChildCreate(fn_1_EE48, 100U, 6144U, 0, lbl_1_bss_254);
    inputProcess = HuPrcChildCreate(fn_1_E4A8, 100U, 6144U, 0, lbl_1_bss_254);
    fn_1_12A6C(1);
    for (;;) {
        HuPrcVSleep();
        if (HuMCSelWinCheck()) {
            fn_1_1FC0(1);
        }
        lbl_1_bss_234 = 1;
        lbl_1_bss_23C = fn_1_DF28(lbl_1_bss_248, quizMode);
        fn_1_12A6C(1);
        lbl_1_bss_234 = 0;
        fn_1_12D78();
        if (HuMCSelWinCheck() != 0) {
            HuWinHomeClear(lbl_1_bss_8A);
        }
        if ((s32) lbl_1_bss_230 == 2) {
            for (;;) {
                HuPrcVSleep();
            }
        }
        fn_1_E8A4(0);
        if ((s32) lbl_1_bss_23C == 0) {
            if ((s32) lbl_1_bss_244 == 0) {
                fn_1_1A40();
                lbl_1_bss_214 = 2;
                fn_1_77FC(3);
                fn_1_AFDC();
                HuPrcVSleep();
                for (;;) {
                    HuPrcVSleep();
                    if ((s32) lbl_1_bss_24C != 0) {
                        fn_1_77FC(2);
                        lbl_1_bss_244 = 1;
                        lbl_1_bss_240 = 0;
                        lbl_1_bss_230 = 0;
                        fn_1_12D78();
                        fn_1_E8A4(1);
                        fn_1_F820(2);
                        break;
                    } else {
                        fn_1_1A40();
                        continue;
                    }
                }
                fn_1_A0C4(lbl_1_bss_248);
                fn_1_E8A4(1);
            } else {
                fn_1_ABD0();
                HuPrcSleep(120);
                fn_1_A0C4(lbl_1_bss_248);
                fn_1_E8A4(1);
            }
            continue;
        } else if ((s32) lbl_1_bss_23C == 1) {
            lbl_1_bss_230 = 1;
            lbl_1_bss_240 = 0;
            if ((s32) lbl_1_bss_24C == 0) {
                fn_1_77FC(4);
            }
            HuAudFXPlay(MSM_SE_CMN_11);
            lbl_1_bss_84 = lbl_1_bss_9C;
            fn_1_F820(3);
            HuPrcChildCreate(fn_1_8728, 100U, 6144U, 0, lbl_1_bss_254);
            lbl_1_bss_238 = 1;
            fn_1_888C(1, fn_1_9330());
            lbl_1_bss_1E4->initialAnswerResult = 0;
            break;
        } else if ((s32) lbl_1_bss_23C == 2) {
            if ((fn_1_9330() == 3) && (hasSelection == 0)) {
                fn_1_888C(0, 3U);
                lbl_1_bss_230 = 1;
                hasSelection = 1;
                fn_1_B1D8(lbl_1_bss_248);
                HuSprAttrSet(lbl_1_bss_18A, 0, 4);
                fn_1_E8A4(1);
                continue;
            }
            if (hasSelection == 0) {
                lbl_1_bss_230 = 1;
                lbl_1_bss_240 = 0;
                HuAudFXPlay(MSM_SE_CMN_12);
                lbl_1_bss_84 = lbl_1_bss_9A;
                HuPrcChildCreate(fn_1_8728, 100U, 6144U, 0, lbl_1_bss_254);
                fn_1_100E0(quizMode, 0);
                HuPrcSleep(120);
                lbl_1_bss_238 = 0;
                lbl_1_bss_1E4->initialAnswerResult = 1;
                if ((s32) lbl_1_bss_24C == 0) {
                    fn_1_77FC(4);
                }
            } else {
                lbl_1_bss_230 = 1;
                lbl_1_bss_240 = 0;
                HuAudFXPlay(MSM_SE_CMN_12);
                lbl_1_bss_84 = lbl_1_bss_9A;
                HuPrcChildCreate(fn_1_8728, 100U, 6144U, 0, lbl_1_bss_254);
                fn_1_100E0(quizMode, 1);
                HuPrcSleep(60);
                lbl_1_bss_238 = 0;
                lbl_1_bss_1E4->initialAnswerResult = 1;
                if ((s32) lbl_1_bss_24C == 0) {
                    fn_1_77FC(4);
                }
                break;
            }
        } else if ((s32) lbl_1_bss_23C == -2) {
            fn_1_9AA0();
            HuPrcSleep(120);
            fn_1_A0C4(lbl_1_bss_248);
            fn_1_E8A4(1);
            continue;
        } else if ((s32) lbl_1_bss_23C == -3) {
            fn_1_ADCC();
            HuPrcSleep(120);
            fn_1_A0C4(lbl_1_bss_248);
            fn_1_E8A4(1);
            continue;
        } else if ((s32) lbl_1_bss_23C == -4) {
            fn_1_A0C4(lbl_1_bss_248);
            fn_1_E8A4(1);
            continue;
        } else {
            continue;
        }
        lbl_1_bss_240 = 0;
        if (HuMCSelWinCheck() != 0) {
            fn_1_1FC0(3);
        }
        HuPrcVSleep();
        fn_1_12C7C();
        fn_1_E8A4(2);
        fn_1_12A80(lbl_1_bss_248);
        fn_1_1A40();
        fn_1_F820(2);
        lbl_1_bss_230 = 0;
        HuPrcKill(motionProcess);
        fn_1_9EC8();
        for (;;) {
            HuPrcVSleep();
            if ((s32) lbl_1_bss_230 == 2) {
                fn_1_9CCC();
                HuPrcSleep(60);
                lbl_1_bss_238 = 0;
                lbl_1_bss_1E4->retryAnswerResult = 2;
                break;
            } else {
                for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                    if (lbl_1_bss_1E4->isHumanPlayer[playerIndex] != 0 &&
                        playerIndex != fn_1_9134() &&
                        (HuPadBtnDown[playerIndex] & 0x100) != 0) {
                        HuAudFXPlay(QUIZ_BUZZ_IN_SOUND);
                        lbl_1_bss_1E4->retryAnswerPlayer = (s8)(lbl_1_bss_248 = playerIndex);
                        lbl_1_bss_230 = 1;
                        fn_1_12D78();
                        fn_1_E8A4(1);
                        fn_1_12A80(fn_1_9134());
                        fn_1_12B68(lbl_1_bss_248);
                        fn_1_18D4();
                        fn_1_A0C4(lbl_1_bss_248);
                        fn_1_EC18(lbl_1_bss_204, lbl_1_bss_248);
                        /* Stop scanning as soon as a player buzzes in. */
                        goto answer_buzzing_player;
                    }
                }
                continue;
            answer_buzzing_player:
                if (HuMCSelWinCheck() != 0) {
                    fn_1_1FC0(4);
                }
                for (;;) {
                    lbl_1_bss_234 = 1;
                    lbl_1_bss_23C = fn_1_DF28(playerIndex, quizMode);
                    fn_1_12A6C(1);
                    lbl_1_bss_234 = 0;
                    if (HuMCSelWinCheck() != 0) {
                        HuWinHomeClear(lbl_1_bss_8A);
                    }
                    fn_1_E8A4(0);
                    if ((s32) lbl_1_bss_23C == 0) {
                        fn_1_ABD0();
                        HuPrcSleep(120);
                        if (HuMCSelWinCheck() != 0) {
                            fn_1_1FC0(4);
                        }
                        fn_1_A0C4(lbl_1_bss_248);
                        fn_1_E8A4(1);
                        continue;
                    }
                    else if ((s32) lbl_1_bss_23C == 1) {
                        lbl_1_bss_230 = 1;
                        lbl_1_bss_238 = 1;
                        HuAudFXPlay(MSM_SE_CMN_11);
                        lbl_1_bss_84 = lbl_1_bss_9C;
                        fn_1_F820(3);
                        HuPrcChildCreate(fn_1_8728, 100U, 6144U, 0, lbl_1_bss_254);
                        lbl_1_bss_1E4->retryAnswerResult = 0;
                        fn_1_888C(1, fn_1_9330());
                        break;
                    } else if ((s32) lbl_1_bss_23C == 2) {
                        lbl_1_bss_230 = 1;
                        HuAudFXPlay(MSM_SE_CMN_12);
                        lbl_1_bss_84 = lbl_1_bss_9A;
                        HuPrcChildCreate(fn_1_8728, 100U, 6144U, 0, lbl_1_bss_254);
                        fn_1_100E0(quizMode, 1);
                        HuPrcSleep(60);
                        lbl_1_bss_238 = 0;
                        lbl_1_bss_1E4->retryAnswerResult = 1;
                        fn_1_888C(1, fn_1_9330());
                        break;
                    } else {
                        if ((s32) lbl_1_bss_23C == -2) {
                            fn_1_9AA0();
                            HuPrcSleep(120);
                            fn_1_A0C4(lbl_1_bss_248);
                            fn_1_E8A4(1);
                        } else if ((s32) lbl_1_bss_23C == -3) {
                            fn_1_ADCC();
                            HuPrcSleep(120);
                            fn_1_A0C4(lbl_1_bss_248);
                            fn_1_E8A4(1);
                        } else if ((s32) lbl_1_bss_23C == -4) {
                            fn_1_A0C4(lbl_1_bss_248);
                            fn_1_E8A4(1);
                        }
                        continue;
                    }
                }
            }
            break;
        }
        break;
    }
    if (HuMCSelWinCheck() != 0) {
        HuWinHomeClear(lbl_1_bss_8A);
    }
    fn_1_90FC((s8) lbl_1_bss_248, (u16) lbl_1_bss_22C, lbl_1_bss_238);
    fn_1_12A6C(0);
    fn_1_E8A4(0);
    if ((s32) lbl_1_bss_238 != 0) {
        fn_1_A7D8();
    } else if ((s32) lbl_1_bss_4 == 0) {
        fn_1_1A40();
        fn_1_E8A4(0);
        lbl_1_bss_4 = 1;
        if (quizMode == 0) {
            fn_1_108C8(lbl_1_bss_220);
        } else if (quizMode == 1) {
            fn_1_10B40(lbl_1_bss_220);
        } else if (quizMode == 2) {
            fn_1_10688(lbl_1_data_E8[lbl_1_bss_220]);
        } else if (quizMode == 3) {
            fn_1_10688(lbl_1_data_E8[lbl_1_bss_220]);
        } else if (quizMode == 4) {
            fn_1_10688(lbl_1_data_5C[lbl_1_bss_220]);
        } else if (quizMode == 5) {
            fn_1_10688(lbl_1_data_1CC[lbl_1_bss_220]);
        } else if (quizMode == 6) {
            fn_1_10688(lbl_1_data_1D4[lbl_1_bss_220]);
        }
    }
    HuPrcSleep(100);
    fn_1_1A40();
    HuPrcSleep(30);
    if ((s32) lbl_1_bss_238 != 0) {
        lbl_1_bss_214 = 3;
        do {
            HuPrcVSleep();
        } while ((s32) lbl_1_bss_1E4->answerFlowPending != 0);
    } else {
        lbl_1_bss_214 = 4;
        do {
            HuPrcVSleep();
        } while ((s32) lbl_1_bss_1E4->answerFlowPending != 0);
    }
    for (;;) {
        HuPrcVSleep();
    }
}

/* Runs the character-reply answer flow, including a buzz-in retry, and records its final
 * outcome. */
void fn_1_CD50(s32 mode) {
    HUPROCESS *buttonProcess;
    HUPROCESS *selectionProcess;
    f64 angleFinal;
    f64 angleFourth;
    f64 angleThird;
    f64 angleSecond;
    f64 angleFirst;
    s32 playerIndex;

    lbl_1_bss_208 = 0;
    lbl_1_bss_234 = 0;
    lbl_1_bss_250 = mode;
    lbl_1_bss_24C = 0;
    lbl_1_bss_244 = 0;
    lbl_1_bss_240 = 1;
    fn_1_935C();
    lbl_1_bss_1E4->initialAnswerPlayer = (s8)(lbl_1_bss_248 = fn_1_9134());
    fn_1_EC18(lbl_1_bss_204, lbl_1_bss_248);
    fn_1_12D78();
    fn_1_E8A4(0);
    fn_1_12A6C(0);
    lbl_1_bss_254 = HuPrcCurrentGet();
    do {
        HuPrcVSleep();
    } while ((s32) lbl_1_bss_208 == 0);
    fn_1_F820(0);
    lbl_1_bss_214 = 0;
    HuPrcVSleep();
    lbl_1_bss_214 = 1;
    HuPrcSleep(30);
    lbl_1_bss_230 = 0;
    fn_1_77FC(1);
    fn_1_12D78();
    fn_1_E8A4(1);
    HuPrcChildCreate(fn_1_12724, 100U, 6144U, 0, lbl_1_bss_254);
    buttonProcess = HuPrcChildCreate(fn_1_F40C, 100U, 6144U, 0, lbl_1_bss_254);
    selectionProcess = HuPrcChildCreate(fn_1_E4A8, 100U, 6144U, 0, lbl_1_bss_254);
    fn_1_12A6C(1);
    for (;;) {
        HuPrcVSleep();
        if (HuMCSelWinCheck() != 0) {
            fn_1_1FC0(1);
        }
        lbl_1_bss_234 = 1;
        lbl_1_bss_23C = fn_1_DF28(lbl_1_bss_248, mode);
        fn_1_12A6C(1);
        lbl_1_bss_234 = 0;
        fn_1_12D78();
        if (HuMCSelWinCheck() != 0) {
            HuWinHomeClear(lbl_1_bss_8A);
        }
        if ((s32) lbl_1_bss_230 == 2) {
            for (;;) {
                HuPrcVSleep();
            }
        }
        fn_1_E8A4(0);
        if ((s32) lbl_1_bss_23C == 0) {
            if ((s32) lbl_1_bss_244 == 0) {
                fn_1_12D78();
                fn_1_E8A4(0);
                fn_1_1A40();
                lbl_1_bss_214 = 2;
                fn_1_77FC(3);
                fn_1_AFDC();
                HuPrcVSleep();
                for (;;) {
                    HuPrcVSleep();
                    if ((s32) lbl_1_bss_24C != 0) {
                        fn_1_77FC(2);
                        lbl_1_bss_244 = 1;
                        lbl_1_bss_240 = 0;
                        lbl_1_bss_230 = 0;
                        fn_1_12D78();
                        fn_1_E8A4(1);
                        fn_1_F820(2);
                        break;
                    } else {
                        fn_1_1A40();
                        continue;
                    }
                }
                fn_1_A0C4(lbl_1_bss_248);
                fn_1_E8A4(1);
            } else {
                fn_1_ABD0();
                HuPrcSleep(120);
                fn_1_A0C4(lbl_1_bss_248);
                fn_1_E8A4(1);
            }
            continue;
        }
        if ((s32) lbl_1_bss_23C == 1) {
            lbl_1_bss_230 = 1;
            lbl_1_bss_240 = 0;
            if ((s32) lbl_1_bss_24C == 0) {
                fn_1_77FC(4);
            }
            lbl_1_bss_238 = 1;
            fn_1_888C(1, fn_1_9330());
            lbl_1_bss_1E4->initialAnswerResult = 0;
            fn_1_1224C(lbl_1_bss_238, lbl_1_bss_1E4->initialAnswerPlayer);
            break;
        }
        if ((s32) lbl_1_bss_23C == 2) {
            fn_1_888C(0, fn_1_9330());
            lbl_1_bss_230 = 1;
            lbl_1_bss_240 = 0;
            lbl_1_bss_238 = 0;
            lbl_1_bss_1E4->initialAnswerResult = 1;
            if ((s32) lbl_1_bss_24C == 0) {
                fn_1_77FC(4);
            }
            fn_1_1224C(lbl_1_bss_238, lbl_1_bss_1E4->initialAnswerPlayer);
            HuPrcSleep(120);
        } else {
            if ((s32) lbl_1_bss_23C == -2) {
                fn_1_9AA0();
                HuPrcSleep(120);
                fn_1_A0C4(lbl_1_bss_248);
                fn_1_E8A4(1);
            } else if ((s32) lbl_1_bss_23C == -3) {
                fn_1_ADCC();
                HuPrcSleep(120);
                fn_1_A0C4(lbl_1_bss_248);
                fn_1_E8A4(1);
            } else if ((s32) lbl_1_bss_23C == -4) {
                fn_1_A0C4(lbl_1_bss_248);
                fn_1_E8A4(1);
            }
            continue;
        }
        fn_1_129C8(0, 8.0f, HU3D_MOTATTR_LOOP);
        lbl_1_bss_240 = 0;
        if (HuMCSelWinCheck() != 0) {
            fn_1_1FC0(3);
        }
        HuPrcVSleep();
        fn_1_12C7C();
        fn_1_E8A4(2);
        fn_1_12A80(lbl_1_bss_248);
        fn_1_1A40();
        fn_1_129C8(1, 16.0f, HU3D_MOTATTR_LOOP);
        fn_1_F820(2);
        lbl_1_bss_230 = 0;
        HuPrcKill(buttonProcess);
        fn_1_9EC8();
        for (;;) {
            HuPrcVSleep();
            if ((s32) lbl_1_bss_230 == 2) {
                fn_1_9CCC();
                HuPrcSleep(60);
                lbl_1_bss_238 = 0;
                lbl_1_bss_1E4->retryAnswerResult = 2;
            } else {
                for (playerIndex = 0; playerIndex < 4; playerIndex += 1) {
                    if (lbl_1_bss_1E4->isHumanPlayer[playerIndex] != 0 &&
                        playerIndex != fn_1_9134() &&
                        (HuPadBtnDown[playerIndex] & 0x100) != 0) {
                        HuAudFXPlay(QUIZ_BUZZ_IN_SOUND);
                        lbl_1_bss_1E4->retryAnswerPlayer = (s8)(lbl_1_bss_248 = playerIndex);
                        lbl_1_bss_230 = 1;
                        fn_1_12D78();
                        fn_1_E8A4(1);
                        fn_1_12A80(fn_1_9134());
                        fn_1_12B68(lbl_1_bss_248);
                        fn_1_18D4();
                        fn_1_A0C4(lbl_1_bss_248);
                        fn_1_EC18(lbl_1_bss_204, lbl_1_bss_248);
                        /* Stop scanning as soon as a player buzzes in. */
                        goto answer_buzzing_player;
                    }
                }
                continue;
            answer_buzzing_player:
                if (HuMCSelWinCheck() != 0) {
                    fn_1_1FC0(4);
                }
                for (;;) {
                    lbl_1_bss_234 = 1;
                    lbl_1_bss_23C = fn_1_DF28(playerIndex, mode);
                    fn_1_12A6C(1);
                    lbl_1_bss_234 = 0;
                    fn_1_12D78();
                    if (HuMCSelWinCheck() != 0) {
                        HuWinHomeClear(lbl_1_bss_8A);
                    }
                    fn_1_E8A4(0);
                    if ((s32) lbl_1_bss_23C == 0) {
                        fn_1_ABD0();
                        HuPrcSleep(120);
                        if (HuMCSelWinCheck()) {
                            fn_1_1FC0(4);
                        }
                        fn_1_A0C4(lbl_1_bss_248);
                        fn_1_E8A4(1);
                        continue;
                    }
                    if ((s32) lbl_1_bss_23C == 1) {
                        lbl_1_bss_230 = 1;
                        lbl_1_bss_238 = 1;
                        lbl_1_bss_1E4->retryAnswerResult = 0;
                        fn_1_1224C(lbl_1_bss_238, lbl_1_bss_1E4->retryAnswerPlayer);
                    } else if ((s32) lbl_1_bss_23C == 2) {
                        lbl_1_bss_230 = 1;
                        lbl_1_bss_238 = 0;
                        lbl_1_bss_1E4->retryAnswerResult = 1;
                        fn_1_1224C(lbl_1_bss_238, lbl_1_bss_1E4->retryAnswerPlayer);
                    } else {
                        if ((s32) lbl_1_bss_23C == -2) {
                            fn_1_9AA0();
                            HuPrcSleep(120);
                            fn_1_A0C4(lbl_1_bss_248);
                            fn_1_E8A4(1);
                        } else if ((s32) lbl_1_bss_23C == -3) {
                            fn_1_ADCC();
                            HuPrcSleep(120);
                            fn_1_A0C4(lbl_1_bss_248);
                            fn_1_E8A4(1);
                        } else if ((s32) lbl_1_bss_23C == -4) {
                            fn_1_A0C4(lbl_1_bss_248);
                            fn_1_E8A4(1);
                        }
                        continue;
                    }
                    break;
                }
            }
            break;
        }
        break;
    }
    if (HuMCSelWinCheck() != 0) {
        HuWinHomeClear(lbl_1_bss_8A);
    }
    fn_1_90FC((s8) lbl_1_bss_248, (u16) lbl_1_bss_22C, lbl_1_bss_238);
    fn_1_12A6C(0);
    fn_1_1A40();
    fn_1_E8A4(0);
    if ((s32) lbl_1_bss_238 != 0) {
        lbl_1_bss_214 = 3;
    } else {
        HuPrcSleep(120);
        lbl_1_bss_214 = 4;
    }
    for (;;) {
        HuPrcVSleep();
    }
}

/* Waits for speech/pad input (ignoring the choice getter's return), resolves ambiguity, and returns
 * 0/no answer, 1/match, 2/mismatch, or a negative status. */
s32 fn_1_DF28(s32 playerIndex, s32 responseMode) {
    s32 newResponseMessage;
    s32 selectedResponse;
    s32 responseData;

    if (responseMode == 0) {
        newResponseMessage = MESSNUM(MESS_MIC_RETYPING, 44);
    } else if (responseMode == 1) {
        newResponseMessage = MESSNUM(MESS_MIC_RETYPING, 65);
    } else if (responseMode == 2) {
        newResponseMessage = MESSNUM(MESS_MIC_RETYPING, 5);
    } else if (responseMode == 3) {
        newResponseMessage = MESSNUM(MESS_MIC_RETYPING, 5);
    } else if (responseMode == 4) {
        newResponseMessage = MESSNUM(MESS_MIC_RETYPING, 23);
    } else if (responseMode == 5) {
        newResponseMessage = MESSNUM(MESS_MIC_RETYPING, 41);
    } else if (responseMode == 6) {
        newResponseMessage = MESSNUM(MESS_QUIZ_CHARA, 24);
    }

    HuMCSelWinContextSet(lbl_1_bss_218, 0, (u8) playerIndex);
    HuMCSelWinChoiceGet();
    if (lbl_1_bss_230 == 2) {
        return -1;
    }

    lbl_1_bss_8 = (MicquizResponse *) HuMCCurResponseGet();
    if (lbl_1_bss_8->result == 0) {
        lbl_1_bss_21C = lbl_1_bss_8->selectedWord;
    } else if (lbl_1_bss_8->result == -1) {
        return -2;
    } else if (lbl_1_bss_8->result == -2) {
        return -2;
    } else if (lbl_1_bss_8->result == -4) {
        return -3;
    } else {
        return -2;
    }

    if (HuMCUnkResponseCheck() != 0) {
        fn_1_12A6C(0);
        fn_1_E8A4(1);
        HuPrcVSleep();
        fn_1_E8A4(1);
        fn_1_10DFC(lbl_1_bss_254);
        selectedResponse = HuMCNewResponseGet(newResponseMessage);
        if (selectedResponse == -1) {
            fn_1_E8A4(0);
            fn_1_10DBC();
            fn_1_12A6C(1);
            return -4;
        }
        lbl_1_bss_21C = selectedResponse;
        fn_1_12A6C(1);
        fn_1_E8A4(0);
    }

    if (HuMCSelWinCheck() != 0) {
        lbl_1_bss_21C -= 1;
    }
    if (lbl_1_bss_21C == 0) {
        return 0;
    }

    if (responseMode == 0) {
        if (HuMCSelWinCheck() != 0) {
            responseData = fn_1_E370(lbl_1_bss_21C + lbl_1_bss_20C, lbl_1_bss_220);
        } else {
            responseData = fn_1_E370(lbl_1_bss_21C, lbl_1_bss_220);
        }
    } else if (responseMode == 1) {
        if (HuMCSelWinCheck() != 0) {
            responseData = fn_1_E370(lbl_1_bss_21C + lbl_1_bss_20C, lbl_1_bss_220);
        } else {
            responseData = fn_1_E370(lbl_1_bss_21C, lbl_1_bss_220);
        }
    } else if (responseMode == 2) {
        responseData = fn_1_E370(lbl_1_bss_21C - 1, lbl_1_bss_220);
    } else if (responseMode == 3) {
        responseData = fn_1_E370(lbl_1_bss_21C - 1, lbl_1_bss_220);
    } else if (responseMode == 4) {
        responseData = fn_1_E370(lbl_1_bss_21C - 1, lbl_1_bss_220);
    } else if (responseMode == 5) {
        responseData = fn_1_E370(lbl_1_bss_21C - 1, lbl_1_bss_220);
    } else if (responseMode == 6) {
        responseData = fn_1_E370(lbl_1_bss_21C - 1, lbl_1_bss_220);
    }
    return responseData;
}

/* Compares a received answer with the expected answer: returns 1 for equality and 2 otherwise. */
s32 fn_1_E370(s32 firstResult, s32 secondResult) {
    if (firstResult == secondResult) {
        return 1;
    }
    return 2;
}

/* Runs the quiz selection sprite's continuous rocking animation. */
void fn_1_E388(void) {
    f32 baseRotation;
    f32 phaseDegrees;
    f32 sineOffset;
    s16 spriteId;
    s32 movingForward;

    spriteId = lbl_1_bss_17E;
    phaseDegrees = (0.0f);
    movingForward = 0;
    baseRotation = (0.0f);
    for (;;) {
        HuPrcVSleep();
        sineOffset = (f32) ((10.0) * sin(((3.141592653589793) * (f64) phaseDegrees) / (180.0)));
        HuSprZRotSet(spriteId, 0, baseRotation + sineOffset);
        if (movingForward == 0) {
            phaseDegrees += (3.0f);
            if (phaseDegrees >= (90.0f)) {
                phaseDegrees = (90.0f);
                movingForward = 1;
            }
            continue;
        }
        phaseDegrees -= (3.0f);
        if (!(phaseDegrees <= (-90.0f))) {
            continue;
        }
        phaseDegrees = (-90.0f);
        movingForward = 0;
    }
}

/* Monitors active speech/pad answering and updates the input effects, heading, and timer pause
 * state. */
void fn_1_E4A8(void)
{
    s32 selectionHandled = 0;

    for (;;) {
        HuPrcVSleep();
        if (lbl_1_bss_230 == 2) {
            fn_1_E8A4(0);
            continue;
        }
        if (lbl_1_bss_234 == 0) {
            continue;
        }
        if (HuMCSelWinCheck() != 0) {
            if (HuMCSelModeGet() == 1) {
                if (selectionHandled == 0) {
                    fn_1_12A6C(0);
                    fn_1_EB68();
                    fn_1_F820(1);
                    fn_1_1FC0(2);
                    fn_1_E8A4(3);
                    if (lbl_1_bss_230 != 2) {
                        lbl_1_bss_230 = 1;
                    }
                    fn_1_18D4();
                    selectionHandled = 1;
                }
            } else {
                fn_1_EBC8();
                selectionHandled = 0;
            }
            continue;
        }
        if (HuMCButtonGet() != 0) {
            fn_1_12A6C(0);
            fn_1_EB68();
            fn_1_E8A4(3);
            fn_1_18D4();
            fn_1_F820(1);
            if (lbl_1_bss_230 != 2) {
                lbl_1_bss_230 = 1;
            }
        } else {
            fn_1_EBC8();
        }
        if (HuMCButtonGet() != 0) {
            if (selectionHandled == 0) {
                fn_1_12A6C(0);
                fn_1_EB68();
                selectionHandled = 1;
            }
            continue;
        }
        selectionHandled = 0;
    }
}

/* Applies a changed answer-display state to input-sprite banks, visibility, and effect
 * processes. */
void fn_1_E8A4(s32 displayState) {
    if (displayState == 0) {
        if (lbl_1_data_1F8 != displayState) {
            lbl_1_data_1F8 = displayState;
            HuSprAttrReset(lbl_1_bss_2D8[0], 2, 4);
            HuSprBankSet(lbl_1_bss_2D8[0], 0, 1);
            HuSprBankSet(lbl_1_bss_2D8[0], 1, 1);
            fn_1_EB18();
            HuSprScaleSet(lbl_1_bss_2D8[0], 1, (1.0f), (1.0f));
            fn_1_EBC8();
        }
    } else if (displayState == 1) {
        if (lbl_1_data_1F8 != displayState) {
            lbl_1_data_1F8 = displayState;
            HuSprAttrSet(lbl_1_bss_2D8[0], 2, 4);
            HuSprBankSet(lbl_1_bss_2D8[0], 0, 0);
            HuSprBankSet(lbl_1_bss_2D8[0], 1, 0);
            fn_1_EAB8();
        }
    } else if (displayState == 2) {
        if (lbl_1_data_1F8 != displayState) {
            lbl_1_data_1F8 = displayState;
            HuSprAttrReset(lbl_1_bss_2D8[0], 2, 4);
            HuSprBankSet(lbl_1_bss_2D8[0], 0, 1);
            HuSprBankSet(lbl_1_bss_2D8[0], 1, 1);
        }
    } else if (displayState == 3 && lbl_1_data_1F8 != displayState) {
        lbl_1_data_1F8 = displayState;
        fn_1_EB18();
        HuSprScaleSet(lbl_1_bss_2D8[0], 1, (1.0f), (1.0f));
    }
}

/* Starts the input-sprite pulse process when requested and none is running. */
void fn_1_EAB8(void) {
    if(lbl_1_bss_10==0) {
        lbl_1_bss_10=HuPrcChildCreate(fn_1_87B8,100,8192,0,lbl_1_bss_254);
    }
}

/* Stops the input-sprite pulse process when the answer display leaves its pulsing state. */
void fn_1_EB18(void) {
    if ((HUPROCESS *) lbl_1_bss_10) {
        HuPrcKill(lbl_1_bss_10);
        lbl_1_bss_10 = NULL;
    }
}

/* Starts the repeated input-effect sprite process if it is not already running. */
void fn_1_EB68(void) {
    if(lbl_1_bss_0==0) {
        lbl_1_bss_0=HuPrcChildCreate(fn_1_8464,100,8192,0,lbl_1_bss_254);
    }
}

/* Stops the input-effect spawning process when the answer input effect ends. */
void fn_1_EBC8(void) {
    if ((HUPROCESS *) lbl_1_bss_0) {
        HuPrcKill(lbl_1_bss_0);
        lbl_1_bss_0 = NULL;
    }
}

/* Sets pad-answer choices for the response kind and selects the answering player's speech
 * session. */
void fn_1_EC18(s32 select, s32 padNo) {
    if (select == 0 || select == 1) {
        fn_1_ED58(padNo, select);
    } else if (select == 2) {
        lbl_1_bss_210 = HuMCSelWinItemSet(MESSNUM(MESS_QUIZ_CHARA, 0), 11, (s16)padNo);
    } else if (select == 3) {
        lbl_1_bss_210 = HuMCSelWinItemSet(MESSNUM(MESS_QUIZ_CHARA, 0), 11, (s16)padNo);
    } else if (select == 4) {
        lbl_1_bss_210 = HuMCSelWinItemSet(MESSNUM(MESS_QUIZ_CHARA, 11), 7, (s16)padNo);
    } else if (select == 5) {
        lbl_1_bss_210 = HuMCSelWinItemSet(MESSNUM(MESS_QUIZ_CHARA, 54), 3, (s16)padNo);
    } else if (select == 6) {
        lbl_1_bss_210 = HuMCSelWinItemSet(MESSNUM(MESS_MIC_RETYPING, 118), 9, (s16)padNo);
    }
    HuMCSessionSet((s16)padNo);
}

/* Chooses the numeric pad-answer message bank and records its zero-or-ten answer offset. */
void fn_1_ED58(s32 padNo, s32 numberMode) {
    s32 messageBase;

    if (numberMode == 0) {
        if (lbl_1_bss_220 <= 10) {
            messageBase = MESSNUM(MESS_QUIZ_CHARA, 81);
            lbl_1_bss_20C = 0;
        } else {
            messageBase = MESSNUM(MESS_QUIZ_CHARA, 92);
            lbl_1_bss_20C = 10;
        }
    } else if (lbl_1_bss_220 <= 10) {
        messageBase = MESSNUM(MESS_QUIZ_CHARA, 103);
        lbl_1_bss_20C = 0;
    } else {
        messageBase = MESSNUM(MESS_QUIZ_CHARA, 114);
        lbl_1_bss_20C = 10;
    }
    lbl_1_bss_210 = HuMCSelWinItemSet(messageBase, 11, (s16) padNo);
}

/* Waits for initial-answer timeout, cancels input, records failure, and reveals the expected answer
 * once. */
void fn_1_EE48(void)
{
    s8 playerIndex;
    u16 selectedValue;
    s32 currentState;
    f32 verticalOffset;
    f32 angle;
    f32 amplitude;

    do {
        HuPrcVSleep();
    } while (lbl_1_bss_230 != 2);

    fn_1_F820(1);
    fn_1_77FC(0);
    HuMCSelWinContextKill();

    if (lbl_1_bss_1E4->useAlternatePrompt == 0) {
        fn_1_1CD0(1);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 17), 1, 1);
    } else {
        fn_1_1CD0(3);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 80), 1, 3);
    }
    fn_1_1DF0();
    HuPrcSleep(120);

    lbl_1_bss_238 = 0;
    lbl_1_bss_1E4->initialAnswerResult = 2;
    currentState = lbl_1_bss_238;
    selectedValue = lbl_1_bss_22C;
    playerIndex = (s8)lbl_1_bss_248;
    lbl_1_bss_1E4->responsePlayerIndex = playerIndex;
    lbl_1_bss_1E4->remainingCountdown = (s16)selectedValue;
    lbl_1_bss_1E4->answerSucceeded = currentState;
    lbl_1_bss_214 = 4;

    do {
        HuPrcVSleep();
    } while (lbl_1_bss_1E4->answerFlowPending != 0);

    if (lbl_1_bss_4 == 0) {
        lbl_1_bss_4 = 1;
        amplitude = (-362.0f);
        angle = (0.0f);
        if (lbl_1_bss_1E8 != 0) {
            do {
                HuPrcVSleep();
                verticalOffset = amplitude * sin(((3.141592653589793) * (f64)angle) / (180.0));
                HuSprPosSet(lbl_1_bss_178, 0, lbl_1_bss_1F0, lbl_1_bss_1EC + verticalOffset);
                angle += (3.0f);
            } while (!(angle >= (90.0f)));
            lbl_1_bss_1EC = (-152.0f);
            HuSprPosSet(lbl_1_bss_178, 0, lbl_1_bss_1F0, lbl_1_bss_1EC);
            HuSprAttrSet(lbl_1_bss_178, 0, 4);
            lbl_1_bss_1E8 = 0;
        }

        if (lbl_1_bss_250 == 0) {
            fn_1_108C8(lbl_1_bss_220);
        } else if (lbl_1_bss_250 == 1) {
            fn_1_10B40(lbl_1_bss_220);
        } else if (lbl_1_bss_250 == 2) {
            fn_1_10688(lbl_1_data_E8[lbl_1_bss_220]);
        } else if (lbl_1_bss_250 == 3) {
            fn_1_10688(lbl_1_data_E8[lbl_1_bss_220]);
        } else if (lbl_1_bss_250 == 4) {
            fn_1_10688(lbl_1_data_5C[lbl_1_bss_220]);
        } else if (lbl_1_bss_250 == 5) {
            fn_1_10688(lbl_1_data_1CC[lbl_1_bss_220]);
        } else if (lbl_1_bss_250 == 6) {
            fn_1_10688(lbl_1_data_1D4[lbl_1_bss_220]);
        }
    }

    do {
        HuPrcVSleep();
    } while (lbl_1_bss_1E4->answerFlowPending != 0);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Waits for timeout in the character-reply flow, cancels input, and records the timeout outcome. */
void fn_1_F40C(void) {
    s8 playerIndex;
    u16 selectedValue;
    s32 currentState;
    do
    {
        HuPrcVSleep();
    }
    while (lbl_1_bss_230 != 2);
    fn_1_F820(1);
    fn_1_77FC(0);
    HuMCSelWinContextKill();
    fn_1_1A40();
    fn_1_129C8(1, (16.0f), HU3D_MOTATTR_LOOP);
    if (lbl_1_bss_1E4->useAlternatePrompt == 0) {
        fn_1_1CD0(1);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 17), 1, 1);
    } else {
        fn_1_1CD0(3);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 80), 1, 3);
    }
    fn_1_1DF0();
    HuPrcSleep(120);
    lbl_1_bss_238 = 0;
    lbl_1_bss_1E4->initialAnswerResult = 2;
    currentState = lbl_1_bss_238;
    selectedValue = lbl_1_bss_22C;
    playerIndex = (s8) lbl_1_bss_248;
    lbl_1_bss_1E4->responsePlayerIndex = playerIndex;
    lbl_1_bss_1E4->remainingCountdown = (s16) selectedValue;
    lbl_1_bss_1E4->answerSucceeded = currentState;
    HuPrcSleep(120);
    lbl_1_bss_214 = 4;
    HuPrcEnd();
    for (;;)
    {
        HuPrcVSleep();
    }
}

/* Starts the requested timer-display animation only when its state changes. */
void fn_1_F820(s32 animationState) {
    if (animationState == 0) {
        if (lbl_1_data_1FC[0] != animationState) {
            lbl_1_data_1FC[0] = animationState;
            HuPrcChildCreate(fn_1_F984, 100U, 6144U, 0, lbl_1_bss_254);
        }
    } else if (animationState == 1) {
        if (lbl_1_data_1FC[0] != animationState) {
            lbl_1_data_1FC[0] = animationState;
            HuPrcChildCreate(fn_1_FBFC, 100U, 6144U, 0, lbl_1_bss_254);
        }
    } else if (animationState == 2) {
        if (lbl_1_data_1FC[0] != animationState) {
            lbl_1_data_1FC[0] = animationState;
            HuPrcChildCreate(fn_1_FD0C, 100U, 6144U, 0, lbl_1_bss_254);
        }
    } else if (animationState == 3 && lbl_1_data_1FC[0] != animationState) {
        lbl_1_data_1FC[0] = animationState;
        HuPrcChildCreate(fn_1_FE04, 100U, 6144U, 0, lbl_1_bss_254);
    }
}

/* Child process that reveals and scales the countdown digits and transitions the timer
 * decorations. */
void fn_1_F984(void) {
    f32 angle;
    f32 transparency;
    f32 scale;
    s32 frame;

    HuSprGrpAttrReset(lbl_1_bss_16C[0], 4);
    scale = (0.0f);
    angle = (0.0f);
    for (frame = 0; frame < 20; frame++) {
        HuPrcVSleep();
        scale = (f32)((2.0) - sin(((3.141592653589793) * (f64)angle) / (180.0)));
        HuSprGrpScaleSet(lbl_1_bss_16C[0], scale, scale);
        angle += (4.5f);
        if (angle >= (90.0f)) {
            break;
        }
    }
    HuSprGrpScaleSet(lbl_1_bss_16C[0], (1.0f), (1.0f));
    HuSprBankSet(lbl_1_bss_174, 0, 1);
    angle = (0.0f);
    for (frame = 0; frame < 10; frame++) {
        HuPrcVSleep();
        transparency = (f32)sin(((3.141592653589793) * (f64)angle) / (180.0));
        HuSprTPLvlSet(lbl_1_bss_172, 0, transparency);
        angle += (9.0f);
        if (angle >= (90.0f)) {
            break;
        }
    }
    HuSprAttrReset(lbl_1_bss_176, 0, 4);
    HuSprAttrSet(lbl_1_bss_174, 0, 4);
    angle = (90.0f);
    for (frame = 0; frame < 10; frame++) {
        HuPrcVSleep();
        transparency = (f32)sin(((3.141592653589793) * (f64)angle) / (180.0));
        HuSprTPLvlSet(lbl_1_bss_172, 0, transparency);
        angle -= (9.0f);
        if (angle <= (45.0f)) {
            break;
        }
    }
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Child process that fades the timer overlay over ten ticks and switches its decoration bank. */
void fn_1_FBFC(void) {
    f32 angle = (90.0f);
    f32 tpLevel;
    s32 count;

    for (count = 0; count < 10; count++) {
        HuPrcVSleep();
        tpLevel = (f32)((0.5) * sin(((3.141592653589793) * (f64)angle) / (180.0)));
        HuSprTPLvlSet(lbl_1_bss_172, 0, tpLevel);
        angle += (9.0f);
        if (angle <= (0.0f)) {
            break;
        }
    }

    HuSprAttrSet(lbl_1_bss_176, 0, 4);
    HuSprAttrReset(lbl_1_bss_174, 0, 4);
    HuSprBankSet(lbl_1_bss_174, 0, 1);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Child process that raises the timer overlay alpha and switches to the other timer decoration. */
void fn_1_FD0C(void) {
    f32 angle = (0.0f);
    s32 frame;

    for (frame = 0; frame < 10; frame++) {
        f32 transparency;

        HuPrcVSleep();
        transparency = (f32) ((0.5) * sin(((3.141592653589793) * (f64) angle) / (180.0)));
        HuSprTPLvlSet(lbl_1_bss_172, 0, transparency);
        angle += (9.0f);
        if (angle >= (90.0f)) {
            break;
        }
    }
    HuSprAttrReset(lbl_1_bss_176, 0, 4);
    HuSprAttrSet(lbl_1_bss_174, 0, 4);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Child process that flashes timer decorations for success; its second phase runs only one
 * iteration. */
void fn_1_FE04(void) {
    f32 angle;
    f32 transparencyLevel;
    s32 frame;

    angle = (0.0f);
    for (frame = 0; frame < 10; frame++) {
        HuPrcVSleep();
        transparencyLevel = (f32)sin(((3.141592653589793) * (f64)angle) / (180.0));
        HuSprTPLvlSet(lbl_1_bss_172, 0, transparencyLevel);
        angle += (9.0f);
        if (angle >= (90.0f)) {
            break;
        }
    }
    HuSprBankSet(lbl_1_bss_174, 0, 1);
    angle = (90.0f);
    for (frame = 0; frame < 10; frame++) {
        HuPrcVSleep();
        transparencyLevel = (f32)sin(((3.141592653589793) * (f64)angle) / (180.0));
        HuSprTPLvlSet(lbl_1_bss_172, 0, transparencyLevel);
        angle += (9.0f);
        if (angle >= (90.0f)) {
            break;
        }
    }
    HuSprBankSet(lbl_1_bss_174, 0, 0);
    angle = (90.0f);
    for (frame = 0; frame < 3; frame++) {
        HuPrcVSleep();
        transparencyLevel = (f32)sin(((3.141592653589793) * (f64)angle) / (180.0));
        HuSprTPLvlSet(lbl_1_bss_172, 0, transparencyLevel);
        angle -= (30.0f);
        if (angle <= (0.0f)) {
            break;
        }
    }
    HuSprAttrReset(lbl_1_bss_176, 0, 4);
    HuSprAttrSet(lbl_1_bss_174, 0, 4);
    HuSprSpeedSet(lbl_1_bss_176, 0, (3.0f));
    angle = (0.0f);
    for (frame = 0; frame < 10; frame++) {
        HuPrcVSleep();
        transparencyLevel = (f32)sin(((3.141592653589793) * (f64)angle) / (180.0));
        HuSprTPLvlSet(lbl_1_bss_172, 0, transparencyLevel);
        angle += (9.0f);
        if (angle >= (90.0f)) {
            break;
        }
    }
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Shows the supplied answer in the initial reply template and waits during answer feedback. */
static inline void fn_1_8BC8_inlined(u32 message)
{
    if (lbl_1_bss_1E4->useAlternatePrompt == 0) {
        fn_1_1CD0(1);
        fn_1_1E64(1, message, 0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 16), 1, 1);
    } else {
        fn_1_1CD0(3);
        fn_1_1E64(3, message, 0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 79), 1, 3);
    }
    fn_1_1DF0();
}

/* Shows the supplied answer in the retry reply template and waits during answer feedback. */
static inline void fn_1_8E08_inlined(u32 message)
{
    if (lbl_1_bss_1E4->useAlternatePrompt == 0) {
        fn_1_1CD0(1);
        fn_1_1E64(1, message, 0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 27), 1, 1);
    } else {
        fn_1_1CD0(3);
        fn_1_1E64(3, message, 0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 83), 1, 3);
    }
    fn_1_1DF0();
}

/* Maps the submitted answer to a message and selects the initial or retry feedback template. */
void fn_1_100E0(s32 mode, s32 retryReply)
{
    s32 messageIndex;
    s32 message;

    if (HuMCSelWinCheck() != 0) {
        if (mode == 0 || mode == 1) {
            messageIndex = lbl_1_bss_21C + lbl_1_bss_20C - 1;
        } else {
            messageIndex = lbl_1_bss_21C - 1;
        }
    } else {
        messageIndex = lbl_1_bss_21C - 1;
    }

    switch (mode) {
    case 0:
        message = messageIndex + MESSNUM(MESS_QUIZ_CHARA, 34);
        break;
    case 1:
        message = messageIndex + MESSNUM(MESS_QUIZ_CHARA, 58);
        break;
    case 2:
        message = lbl_1_data_E8[messageIndex];
        break;
    case 3:
        message = lbl_1_data_E8[messageIndex];
        break;
    case 4:
        message = lbl_1_data_5C[messageIndex];
        break;
    case 5:
        message = lbl_1_data_1CC[messageIndex];
        break;
    case 6:
        message = lbl_1_data_1D4[messageIndex];
        break;
    }

    if (retryReply == 0) {
        fn_1_8BC8_inlined(message);
    } else {
        fn_1_8E08_inlined(message);
    }
}

/* Labels and choices for the development selection menu. */
s32 lbl_1_data_384 = -1;
u16 lbl_1_data_388[] = {105, 103, 103, 103, 103, 103, 103, 103};
s32 lbl_1_data_398[] = {1, 1, 1, 1, 2, -1, -1, -1};
char *lbl_1_data_424[] = {
    "Picture Quiz", "Counting Quiz", "Memory Quiz", "Comparison Quiz",
    "Variety Quiz", "Unreasonable Quiz", "Tutorial", "Introduction"
};
char *lbl_1_data_45C[] = {"1:Spin", "2:Screen effect"};
char *lbl_1_data_47C[] = {"1:Scroll", "2:Spotlight"};
char *lbl_1_data_494[] = {"1:Walk", "2:Hide"};
char *lbl_1_data_4B4[] = {"1:Characters", "2:Stars"};
char *lbl_1_data_4D8[] = {"1:Slit", "2:Blcoks", "3:Shadow"};
char lbl_1_data_4E4[] = "******* MQSel ObjectSetup *********\n";
char lbl_1_data_509[] = "MICQUIZ SELECT MENU";
char lbl_1_data_51D[] = "<DIFF:%d>";
char lbl_1_data_527[] = "%s";

/* Shows the expected-answer message in the normal or alternate reveal window and waits. */
void fn_1_10688(u32 message) {
    if (lbl_1_bss_1E4->useAlternatePrompt == 0) {
        fn_1_1CD0(2);
        fn_1_1E64(2, message, 0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 29), 1, 2);
    } else {
        fn_1_1CD0(3);
        fn_1_1E64(3, message, 0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 85), 1, 3);
    }
    fn_1_1DF0();
}

/* Reveals the expected picture-quiz number from the one-based index; the preliminary comparison has
 * no effect. */
void fn_1_108C8(s32 messageIndex) {
    messageIndex == 0;
    if (lbl_1_bss_1E4->useAlternatePrompt == 0) {
        fn_1_1CD0(2);
        fn_1_1E64(2, lbl_1_data_12C[messageIndex - 1], 0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 29), 1, 2);
    } else {
        fn_1_1CD0(3);
        fn_1_1E64(3, lbl_1_data_12C[messageIndex - 1], 0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 85), 1, 3);
    }
    fn_1_1DF0();
}

/* Reveals the expected counting-quiz number from the one-based index; the preliminary comparison
 * has no effect. */
void fn_1_10B40(s32 messageIndex) {
    messageIndex == 0;
    if (lbl_1_bss_1E4->useAlternatePrompt == 0) {
        fn_1_1CD0(2);
        fn_1_1E64(2, lbl_1_data_17C[messageIndex - 1], 0);
        fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 29), 1, 2);
    } else {
        fn_1_1CD0(3);
        fn_1_1E64(3, lbl_1_data_17C[messageIndex - 1], 0);
        fn_1_1BAC(MESSNUM(MESS_MICQUIZ, 85), 1, 3);
    }
    fn_1_1DF0();
}

void fn_1_10DB8(void)
{
}

void fn_1_10DBC(void) {
    HuPrcChildCreate(fn_1_10E84, 100U, 6144U, 0, lbl_1_bss_254);
}

void fn_1_10DFC(HUPROCESS* parent) {
    if(lbl_1_data_208!=-1)
        HuPrcChildCreate(fn_1_10E4C,100,6144,0,parent);
}

/* Child process that removes the auxiliary response-message window and clears its handle. */
void fn_1_10E4C(void) {
    HuWinKill(lbl_1_data_208);
    lbl_1_data_208 = -1;
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Creates the auxiliary response-message window if absent, waits for its text, then closes it after
 * 180 ticks. */
void fn_1_10E84(void) {
    HuVec2f messageSize;

    HuWinMesMaxSizeGet(1, &messageSize, MESSNUM(MESS_MICQUIZ, 58));
    if (lbl_1_data_208 != -1) {
        HuPrcEnd();
        for (;;) {
            HuPrcVSleep();
        }
    }
    lbl_1_data_208 = HuWinExCreateFrame((-10000.0f), (200.0f),
        messageSize.x, messageSize.y, -1, 0);
    HuWinExOpen(lbl_1_data_208);
    HuWinMesSpeedSet(lbl_1_data_208, 0);
    HuWinAttrSet(lbl_1_data_208, HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSet(lbl_1_data_208, MESSNUM(MESS_MICQUIZ, 58));
    HuWinMesWait(lbl_1_data_208);
    HuPrcSleep(180);
    if (lbl_1_data_208 != -1) {
        HuWinExClose(lbl_1_data_208);
        HuWinKill(lbl_1_data_208);
        lbl_1_data_208 = -1;
    }
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Initializes the countdown and integer tick interval: 900 divided by the starting count for
 * submode 4, or 1800 divided by it otherwise. */
void fn_1_10FD0(void) {
    u8 currentIndex;
    s32 firstMode;
    u8 nextIndex;
    s32 nextMode;
    u8 previousIndex;
    u8 fallbackIndex;
    s32 timingMode;

    lbl_1_bss_230 = 1;
    currentIndex = lbl_1_bss_1E4->difficultyIndex;
    lbl_1_bss_22C = lbl_1_data_48[currentIndex];
    firstMode = lbl_1_bss_1E4->quizSubmode;

    if (firstMode == 2) {
        nextIndex = lbl_1_bss_1E4->difficultyIndex;
        lbl_1_bss_22C = lbl_1_data_48[nextIndex + 1];
    } else {
        nextMode = lbl_1_bss_1E4->quizSubmode;
        if (nextMode == 5) {
            previousIndex = lbl_1_bss_1E4->difficultyIndex;
            lbl_1_bss_22C = lbl_1_data_48[previousIndex - 1];
        } else {
            fallbackIndex = lbl_1_bss_1E4->difficultyIndex;
            lbl_1_bss_22C = lbl_1_data_48[fallbackIndex];
        }
    }

    timingMode = lbl_1_bss_1E4->quizSubmode;
    if (timingMode == 4) {
        lbl_1_bss_228 = 900 / lbl_1_bss_22C;
    } else {
        lbl_1_bss_228 = 1800 / lbl_1_bss_22C;
    }
    lbl_1_bss_224 = 0;
    fn_1_11278(lbl_1_bss_22C);
}

/* Advances the active countdown by one update, refreshes its digits on each tick, and returns the
 * timer state. */
s32 fn_1_11198(void) {
    if ((s32) lbl_1_bss_230 == 0) {
        lbl_1_bss_224 += 1;
        if ((s32) lbl_1_bss_224 >= (s32) lbl_1_bss_228) {
            HuAudFXPlay(QUIZ_TIMER_TICK_SOUND);
            lbl_1_bss_22C -= 1;
            lbl_1_bss_224 = 0;
            fn_1_11278(lbl_1_bss_22C);
            if ((s32) lbl_1_bss_22C <= 0) {
                lbl_1_bss_230 = 2;
            }
        }
    }
    return lbl_1_bss_230;
}

/* Updates the countdown's two digit banks when the supplied value is nonnegative. */
void fn_1_11278(s32 value) {
    if (value >= 0) {
        s32 tens = value / 10;
        s32 ones = value - tens * 10;
        HuSprBankSet(lbl_1_bss_16C[0], 0, tens);
        HuSprBankSet(lbl_1_bss_16C[0], 1, ones);
    }
}

/* Returns a random integer from the requested minimum up to, but not including, the maximum. */
s32 fn_1_11308(s32 minimum, s32 maximum) {
    s32 unused;
    return minimum + frandmod(maximum - minimum);
}

/* Returns whether a 0-99 random roll is below the requested chance; even zero chance consumes a
 * roll. */
s32 fn_1_11344(s32 chancePercent) {
    s32 roll;

    roll = frandmod(100);
    if (chancePercent == 0) {
        return 0;
    }
    if (roll < chancePercent) {
        return 1;
    }
    return 0;
}

/* Chooses one of three successful-answer replies, waiting for the shared reply transition before
 * speaking. */
void fn_1_113A4(s32 primaryMessageIndex, s32 secondaryMessageIndex) {
    extern s32 lbl_1_bss_214;
    extern u32 lbl_1_data_1D4[8];
    extern u32 lbl_1_data_A0[18];
    s32 randomChoice;
    s32 previousSelection;

    randomChoice = frandmod(3);
    previousSelection = lbl_1_bss_214;
    fn_1_12120();
    lbl_1_bss_214 = 5;
    lbl_1_bss_1E4->answerFlowPending = 1;
    do {
        HuPrcVSleep();
    } while (lbl_1_bss_1E4->answerFlowPending != 0);
    lbl_1_bss_214 = previousSelection;
    HuPrcSleep(60);

    fn_1_1CD0(3);
    fn_1_129C8(1, (16.0f), HU3D_MOTATTR_LOOP);

    if (randomChoice == 0) {
        fn_1_1E64(3, lbl_1_data_1D4[secondaryMessageIndex], 0);
        fn_1_1E64(3, lbl_1_data_A0[primaryMessageIndex], 1);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ_Q, 63), 1, 3);
        fn_1_1DF0();
    } else if (randomChoice == 1) {
        fn_1_1E64(3, lbl_1_data_1D4[secondaryMessageIndex], 0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ_Q, 64), 1, 3);
        fn_1_1DF0();
        HuPrcSleep(120);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ_Q, 65), 1, 3);
        fn_1_1DF0();
    } else {
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ_Q, 66), 1, 3);
        fn_1_1DF0();
        HuPrcSleep(120);
        fn_1_1E64(3, lbl_1_data_A0[primaryMessageIndex], 0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ_Q, 67), 1, 3);
        fn_1_1DF0();
    }
}

/* Chooses one of four unsuccessful-answer replies; all but the first wait for the shared reply
 * transition. */
void fn_1_117FC(s32 primaryMessageIndex, s32 secondaryMessageIndex)
{
    s32 randomChoice = frandmod(4);

    if (randomChoice != 0) {
        s32 previousSelection = lbl_1_bss_214;

        fn_1_12120();
        lbl_1_bss_214 = 5;
        lbl_1_bss_1E4->answerFlowPending = 1;
        do {
            HuPrcVSleep();
        } while (lbl_1_bss_1E4->answerFlowPending != 0);
        lbl_1_bss_214 = previousSelection;
        HuPrcSleep(60);
    }

    fn_1_1CD0(3);
    fn_1_129C8(1, (16.0f), HU3D_MOTATTR_LOOP);

    if (randomChoice == 0) {
        fn_1_1E64(3, lbl_1_data_1D4[secondaryMessageIndex], 0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ_Q, 68), 1, 3);
    } else if (randomChoice == 1) {
        fn_1_1E64(3, lbl_1_data_A0[primaryMessageIndex], 0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ_Q, 69), 1, 3);
    } else if (randomChoice == 2) {
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ_Q, 70), 1, 3);
        fn_1_1DF0();
        HuPrcSleep(120);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ_Q, 71), 1, 3);
        fn_1_1DF0();
    } else {
        fn_1_1E64(3, lbl_1_data_1D4[secondaryMessageIndex], 0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ_Q, 72), 1, 3);
        fn_1_1DF0();
        HuPrcSleep(120);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ_Q, 73), 1, 3);
        fn_1_1DF0();
    }
    fn_1_1DF0();
}

/* Chooses one of two alternate replies; the final branch delays without waiting for its last
 * message to finish. */
void fn_1_11C68(s32 primaryMessageIndex, s32 secondaryMessageIndex)
{
    s32 randomChoice = frandmod(2);

    if (randomChoice != 0) {
        s32 previousSelection = lbl_1_bss_214;

        fn_1_12120();
        lbl_1_bss_214 = 5;
        lbl_1_bss_1E4->answerFlowPending = 1;
        do {
            HuPrcVSleep();
        } while (lbl_1_bss_1E4->answerFlowPending != 0);
        lbl_1_bss_214 = previousSelection;
        HuPrcSleep(60);
    }

    fn_1_1CD0(3);
    fn_1_129C8(1, (16.0f), HU3D_MOTATTR_LOOP);

    if (randomChoice == 0) {
        fn_1_1E64(3, lbl_1_data_1D4[secondaryMessageIndex], 0);
        fn_1_1E64(3, lbl_1_data_A0[primaryMessageIndex], 1);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ_Q, 74), 1, 3);
        fn_1_1DF0();
        HuPrcSleep(120);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ_Q, 75), 1, 3);
        fn_1_1DF0();
    } else {
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ_Q, 76), 1, 3);
        fn_1_1DF0();
        HuPrcSleep(120);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ_Q, 77), 1, 3);
        HuPrcSleep(120);
    }
}

/* Shows the fallback incorrect-answer reply with the selected response name and waits for the
 * message. */
void fn_1_11FB0(s32 messageIndex)
{
    fn_1_1CD0(3);
    fn_1_129C8(1, (16.0f), HU3D_MOTATTR_LOOP);
    fn_1_1E64(3, lbl_1_data_1D4[messageIndex], 0);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ_Q, 79), 1, 3);
    fn_1_1DF0();
}

/* Shows and waits for the message preceding the shared character-reply transition. */
void fn_1_12120(void) {
    fn_1_1CD0(3);
    fn_1_1BAC(MESSNUM(MESS_MICQUIZ_Q, 62), 1, 3);
    fn_1_1DF0();
}

s32 fn_1_12238(void) {
    return lbl_1_bss_21C - 1;
}

/* Selects success or failure character dialogue and animations for the answering player. */
void fn_1_1224C(s32 success, s32 choice) {
    s32 slot;

    fn_1_1A40();
    HuPrcSleep(30);

    if (success != 0) {
        fn_1_113A4(lbl_1_bss_1E4->playerCharacterNumbers[choice], lbl_1_bss_21C - 1);
        HuAudFXPlay(MSM_SE_CMN_11);
        lbl_1_bss_84 = lbl_1_bss_9C;
        if (lbl_1_data_1FC[0] != 3) {
            lbl_1_data_1FC[0] = 3;
            HuPrcChildCreate(fn_1_FE04, 100U, 6144U, 0, lbl_1_bss_254);
        }
        HuPrcChildCreate(fn_1_8728, 100U, 6144U, 0, lbl_1_bss_254);
        HuAudFXPlay(MSM_SE_GUIDE_47);
        fn_1_129C8(3, 8.0f, HU3D_MOTATTR_LOOP);
        HuPrcSleep(120);
        return;
    }

    if (fn_1_12850(lbl_1_bss_21C - 1) != 0) {
        slot = fn_1_12800(lbl_1_bss_21C - 1);
        if (lbl_1_bss_258[slot].useAlternateReply != 0) {
            fn_1_11C68(lbl_1_bss_1E4->playerCharacterNumbers[choice], lbl_1_bss_21C - 1);
        } else {
            fn_1_117FC(lbl_1_bss_1E4->playerCharacterNumbers[choice], lbl_1_bss_21C - 1);
        }
    } else {
        fn_1_11FB0(lbl_1_bss_21C - 1);
    }

    HuAudFXPlay(MSM_SE_CMN_12);
    lbl_1_bss_84 = lbl_1_bss_9A;
    HuPrcChildCreate(fn_1_8728, 100U, 6144U, 0, lbl_1_bss_254);
    fn_1_129C8(6, 8.0f, HU3D_MOTATTR_LOOP);
    HuPrcSleep(120);
}

/* During character animation, plays the voice effect at selected times in motions 2 and 6. */
void fn_1_12724(void) {
    f32 motionTime;
    s16 motionId;

    for (;;) {
        HuPrcVSleep();
        motionTime = Hu3DMotionTimeGet(lbl_1_bss_82);
        motionId = Hu3DMotionIDGet(lbl_1_bss_82);
        if (motionId == lbl_1_bss_74[6]) {
            if (((20.0f) == motionTime) || ((55.0f) == motionTime)) {
                HuAudFXPlay(MSM_SE_GUIDE_45);
            }
            continue;
        }
        if (motionId != lbl_1_bss_74[2]) {
            continue;
        }
        if (((10.0f) != motionTime) && ((60.0f) != motionTime)) {
            continue;
        }
        HuAudFXPlay(MSM_SE_GUIDE_45);
    }
}

/* Finds the first choice-table record with the requested response key, or returns -1. */
int fn_1_12800(int key) {
    int index;

    for (index = 0;index<8;index++) {
        if(key==lbl_1_bss_258[index].key) return index;
    }
    return -1;
}

/* Reports whether a response key has a choice-table record enabled for reply selection. */
int fn_1_12850(int key) {
    int index;

    for (index = 0; index < 8; index++) {
        if (key == lbl_1_bss_258[index].key &&
            lbl_1_bss_258[index].responseSelectionDisabled == 0) {
            return 1;
        }
    }
    return 0;
}

/* Creates the reply character model, loads its seven motions, and sets its quiz-scene position. */
void fn_1_128BC(void) {
    s32 index;

    lbl_1_bss_82 = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_micquizishi, 121), HU_MEMNUM_OVL, HEAP_MODEL));
    for (index = 0; index < 7; index++) {
        lbl_1_bss_74[index] = Hu3DJointMotion(
            lbl_1_bss_82,
            HuDataSelHeapReadNum(lbl_1_data_0[index], HU_MEMNUM_OVL, HEAP_MODEL)
        );
    }
    Hu3DModelAttrSet(lbl_1_bss_82, HU3D_MOTATTR_LOOP);
    fn_1_129C8(0, (0.0f), HU3D_MOTATTR_LOOP);
    Hu3DModelPosSet(lbl_1_bss_82, (0.0f), (0.0f), (-410.0f));
}

/* Starts a transition to the indexed character motion if it differs from the current main motion;
 * the float argument is blend duration. */
void fn_1_129C8(s32 motionIndex, f32 blendFrames, u32 attributes) {
    s16 currentMotion;

    currentMotion = Hu3DMotionIDGet(lbl_1_bss_82);
    if (lbl_1_bss_74[motionIndex] != currentMotion) {
        Hu3DMotionShiftSet(lbl_1_bss_82, lbl_1_bss_74[motionIndex], (0.0f), blendFrames,
                           attributes);
    }
}

void fn_1_12A6C(s32 promptReady) {
    lbl_1_bss_1E4->startPromptEnabled = promptReady;
}

/* Shrinks and hides the specified player's portrait when the answering player changes. */
void fn_1_12A80(int base) {
    s16 sprite = (s16)(base + 4);
    f32 scale;
    f32 angle;

    angle = (90.0f);
    do {
        HuPrcVSleep();
        scale = (f32) sin(((3.141592653589793) * (f64) angle) / (180.0));
        HuSprScaleSet(lbl_1_bss_2D8[0], sprite, scale, scale);
        angle += (9.0f);
    } while (!(angle >= (180.0f)));
    HuSprAttrSet(lbl_1_bss_2D8[0], sprite, 4);
}

/* Shows and scales in the specified player's portrait when that player is selected to answer. */
void fn_1_12B68(s32 baseIndex) {
    f32 scale;
    f32 angle;
    s16 spriteIndex;

    spriteIndex = baseIndex + 4;
    angle = (0.0f);
    HuSprScaleSet(lbl_1_bss_2D8[0], spriteIndex, (0.0f), (0.0f));
    HuSprAttrReset(lbl_1_bss_2D8[0], spriteIndex, 4);
    do {
        HuPrcVSleep();
        scale = (f32) sin(((3.141592653589793) * (f64) angle) / (180.0));
        HuSprScaleSet(lbl_1_bss_2D8[0], spriteIndex, scale, scale);
        angle += (9.0f);
    } while (!(angle >= (90.0f)));
}

/* Shows and scales in the extra portrait-group marker while a buzz-in attempt is available. */
void fn_1_12C7C(void) {
    f32 scale;
    f32 angle;

    angle = (0.0f);
    HuSprScaleSet(lbl_1_bss_2D8[0], 3, (0.0f), (0.0f));
    HuSprAttrReset(lbl_1_bss_2D8[0], 3, 4);
    do {
        HuPrcVSleep();
        scale = (f32) sin(((3.141592653589793) * (f64) angle) / (180.0));
        HuSprScaleSet(lbl_1_bss_2D8[0], 3, scale, scale);
        angle += (9.0f);
    } while (!(angle >= (90.0f)));
}

/* Shrinks and hides the extra portrait-group marker when the answer flow closes that state. */
void fn_1_12D78(void) {
    f32 scale;
    f32 angle;

    angle = (90.0f);
    do {
        HuPrcVSleep();
        scale = (f32) sin(((3.141592653589793) * (f64) angle) / (180.0));
        HuSprScaleSet(lbl_1_bss_2D8[0], 3, scale, scale);
        angle += (9.0f);
    } while (!(angle >= (180.0f)));
    HuSprAttrSet(lbl_1_bss_2D8[0], 3, 4);
}

/* Message-control callback: converts the one-based sound number and selects centered or panned
 * voice playback; listed messages suppress other sound numbers. */
void fn_1_12E48(HUWINID windowId, u32 messageId, s16 soundIndex) {
    QuizMessageIds messageIds = {
        { MESSNUM(MESS_MICQUIZ, 0),           MESSNUM(MESS_MICQUIZ, 6),
          MESSNUM(MESS_MICQUIZ, 12),          MESSNUM(MESS_MICQUIZ, 30),
          MESSNUM(MESS_MICQUIZ, 34),          MESSNUM(MESS_MICQUIZ, 35),
          MESSNUM(MESS_MICQUIZ, 36),          MESSNUM(MESS_MICQUIZ, 37),
          MESSNUM(MESS_MICQUIZ, 39),          MESSNUM(MESS_MICQUIZ, 40),
          MESSNUM(MESS_MICQUIZ, 54),          MESSNUM(MESS_MICQUIZ, 57),
          MESSNUM(MESS_MICQUIZ, 58),          MESSNUM(MESS_MICQUIZ_TUTORIAL, 40),
          MESSNUM(MESS_MICQUIZ_TUTORIAL, 41), MESSNUM(MESS_MICQUIZ_TUTORIAL, 42),
          MESSNUM(MESS_MICQUIZ_TUTORIAL, 44), MESSNUM(MESS_MICQUIZ_TUTORIAL, 45),
          MESSNUM(MESS_MICQUIZ_INTRO, 0),     -1 }
    };
    QuizSoundIds sounds = { { MSM_SE_GUIDE_26, MSM_SE_GUIDE_27, MSM_SE_GUIDE_28, MSM_SE_GUIDE_18,
                              MSM_SE_GUIDE_19, MSM_SE_GUIDE_20, MSM_SE_GUIDE_47, MSM_SE_GUIDE_48,
                              MSM_SE_GUIDE_49, MSM_SE_GUIDE_50, MSM_SE_GUIDE_51, MSM_SE_GUIDE_52,
                              MSM_SE_GUIDE_53, MSM_SE_GUIDE_54, MSM_SE_GUIDE_30, MSM_SE_GUIDE_22,
                              -1 } };
    s16 messageIndex;

    soundIndex--;

    for (messageIndex = 0; ; messageIndex++) {
        if (messageIds.values[messageIndex] == -1) {
            HuAudFXPlay(sounds.values[soundIndex]);
            return;
        }
        if (messageId == (u32)messageIds.values[messageIndex]) {
            if (soundIndex == 0 || soundIndex == 1 ||
                soundIndex == 2 || soundIndex == 14) {
                HuAudFXPlayPan(sounds.values[soundIndex], 48);
                return;
            }
            if (soundIndex == 3 || soundIndex == 4 ||
                soundIndex == 5 || soundIndex == 15) {
                HuAudFXPlayPan(sounds.values[soundIndex], 80);
            }
            break;
        }
    }
}

/* Menu processes, sprite groups, answer state and display resources. */
s16 lbl_1_bss_2D8[6];
MicquizChoiceRecord lbl_1_bss_258[8];
HUPROCESS * lbl_1_bss_254;
s32 lbl_1_bss_250;
s32 lbl_1_bss_24C;
s32 lbl_1_bss_248;
s32 lbl_1_bss_244;
s32 lbl_1_bss_240;
s32 lbl_1_bss_23C;
s32 lbl_1_bss_238;
s32 lbl_1_bss_234;
s32 lbl_1_bss_230;
s32 lbl_1_bss_22C;
s32 lbl_1_bss_228;
s32 lbl_1_bss_224;
s32 lbl_1_bss_220;
s32 lbl_1_bss_21C;
s16 lbl_1_bss_218;
s32 lbl_1_bss_214;
s16 lbl_1_bss_210;
s32 lbl_1_bss_20C;
s32 lbl_1_bss_208;
s32 lbl_1_bss_204;
s16 lbl_1_bss_1F8[6];
s32 lbl_1_bss_1F4;
f32 lbl_1_bss_1F0;
f32 lbl_1_bss_1EC;
s32 lbl_1_bss_1E8;
MicquizWork * lbl_1_bss_1E4;
ANIMDATA * lbl_1_bss_1E0;
ANIMDATA * lbl_1_bss_1D8[2];
ANIMDATA * lbl_1_bss_1D4;
ANIMDATA * lbl_1_bss_1D0;
ANIMDATA * lbl_1_bss_1CC;
ANIMDATA * lbl_1_bss_1C8;
ANIMDATA * lbl_1_bss_1C4;
ANIMDATA * lbl_1_bss_1C0;
ANIMDATA * lbl_1_bss_1BC;
ANIMDATA * lbl_1_bss_1B8;
s16 lbl_1_bss_1B4;
ANIMDATA * lbl_1_bss_1B0;
ANIMDATA * lbl_1_bss_1AC;
ANIMDATA * lbl_1_bss_1A8;
ANIMDATA * lbl_1_bss_1A4;
ANIMDATA * lbl_1_bss_1A0;
ANIMDATA * lbl_1_bss_19C;
ANIMDATA * lbl_1_bss_198;
ANIMDATA * lbl_1_bss_194;
ANIMDATA * lbl_1_bss_18C[2];
s16 lbl_1_bss_18A;
s16 lbl_1_bss_180[5];
s16 lbl_1_bss_17E;
s16 lbl_1_bss_17C;
s16 lbl_1_bss_17A;
s16 lbl_1_bss_178;
s16 lbl_1_bss_176;
s16 lbl_1_bss_174;
s16 lbl_1_bss_172;
s16 lbl_1_bss_16C[3];
ANIMDATA * lbl_1_bss_15C[4];
ANIMDATA * lbl_1_bss_158;
ANIMDATA * lbl_1_bss_154;
ANIMDATA * lbl_1_bss_150;
ANIMDATA * lbl_1_bss_14C;
s32 lbl_1_bss_148;
ANIMDATA * lbl_1_bss_144;
s16 lbl_1_bss_140;
ANIMDATA * lbl_1_bss_130[4];
s16 lbl_1_bss_128[4];
ANIMDATA * lbl_1_bss_F8[4][3];
s16 lbl_1_bss_E0[4][3];
ANIMDATA * lbl_1_bss_DC;
s16 lbl_1_bss_D8;
ANIMDATA * lbl_1_bss_C8[4];
s16 lbl_1_bss_C0[4];
MGSCORE * lbl_1_bss_B8[2];
s16 lbl_1_bss_B6;
s16 lbl_1_bss_B4;
HUPROCESS * lbl_1_bss_B0;
s32 lbl_1_bss_AC;
s32 lbl_1_bss_A8;
ANIMDATA * lbl_1_bss_A4;
ANIMDATA * lbl_1_bss_A0;
s16 lbl_1_bss_9C;
s16 lbl_1_bss_9A;
s16 lbl_1_bss_92[4];
s16 lbl_1_bss_90;
s16 lbl_1_bss_8E;
s16 lbl_1_bss_8C;
s16 lbl_1_bss_8A;
s16 lbl_1_bss_88;
s16 lbl_1_bss_86;
s16 lbl_1_bss_84;
s16 lbl_1_bss_82;
s16 lbl_1_bss_74[7];
HUPROCESS * lbl_1_bss_4C[10];
HUPROCESS * lbl_1_bss_20[11];
OMOBJ * lbl_1_bss_1C;
f32 lbl_1_bss_18;
ANIMDATA * lbl_1_bss_14;
HUPROCESS * lbl_1_bss_10;
HUPROCESS * lbl_1_bss_C;
MicquizResponse * lbl_1_bss_8;
s32 lbl_1_bss_4;
HUPROCESS * lbl_1_bss_0;
