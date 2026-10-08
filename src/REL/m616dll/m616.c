/* M616's timed four-player choice rounds, character presentation, sequence transitions, and choice,
 * result, and motion-completion sounds. */
#include "REL/m616dll.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/pad.h"
#include "game/frand.h"
#include "datadir_enum.h"
#include <string.h>

/* Sounds for choice changes, results, and completed character motions. */
#define M616_SFX_CHOICE_CHANGED 1741
#define M616_SFX_RESULT_FIRST 1742
#define M616_SFX_RESULT_SECOND 1743
#define M616_SFX_MOTION_COMPLETE_FIRST 1745
#define M616_SFX_MOTION_COMPLETE_SECOND 1744

typedef struct M616CpuParam {
    u32 unreadCpuSetting; /* CPU setting stored with the difficulty data but not read by this
                           * game. */
    u32 repeatPercent; /* Chance, from 0 to 100, of repeating the prior answer. */
} M616CpuParam;
extern MGSEQ_PARAM lbl_1_data_0;
extern s32 lbl_1_data_1F0[17];
extern s32 lbl_1_data_234[6];
extern s32 lbl_1_data_24C[13];
extern unsigned int lbl_1_data_280[12];
extern M616CpuParam lbl_1_data_2B0[4];
extern s32 lbl_1_data_2D0[3];

/* Position sampled from the P1stmov object and used to place the lead character's landing dust. */
HuVecF lbl_1_bss_0;
/* Sequence manager configuration: callbacks for initialization, fade-in, start, main, finish,
 * pre-winner, winner, fade-out, and close. */
MGSEQ_PARAM lbl_1_data_0 = {
    0, 0, fn_1_140, fn_1_160, fn_1_4BC, fn_1_4C0, fn_1_F14,
    fn_1_F78, fn_1_1284, fn_1_135C, fn_1_1360
};

/* Start the requested BGM when there is no streamed track and the start-message effect is
 * active. */
s32 fn_1_A0(s32 streamNo, s32 bgmId)
{
    s32 activeStream = streamNo;

    if (activeStream == -1) {
        if (GameMesFXPlayCheck(MgSeqGameMesIdGet()) != 0) {
            activeStream = HuAudBGMPlay((s16)bgmId);
        }
    }
    return activeStream;
}

/* Called on frame 0 of the finish hook to fade the active streamed track. */
void fn_1_104(s32 streamNo)
{
    if (streamNo != -1) {
        HuAudSStreamFadeOut(streamNo, 100);
    }
}

/* MGSEQ_PARAM initHook: advance from sequence initialization to the next mode. */
void fn_1_140(s16 mode, s16 frameNo)
{
    MgSeqModeNext();
}

/* MGSEQ_PARAM fadeInHook: on frame 0 start the opening animation, camera motion, and streamed
 * music; each callback copies P1stmov's transform to the lead character, then on camera-motion
 * completion resets the hooks and character transform and advances to the round sequence. */
void fn_1_160(s16 mode, s16 frameNo)
{
    s32 playerNo;
    HU3D_MODELID characterModel;
    Mtx matrix;
    HuVecF pos;
    HuVecF rot;
    HuVecF scale;

    if (frameNo == 0) {
        for (playerNo = 0; playerNo < 4; playerNo++) {
            fn_1_215C(playerNo, 5);
            if (playerNo != 0) {
                Hu3DModelHookSet(lbl_1_bss_10.playerModels[playerNo], "P1st",
                    lbl_1_bss_10.characterModels[playerNo]);
                fn_1_22BC(playerNo, 0, HU3D_MOTATTR_LOOP);
            } else {
                fn_1_22BC(playerNo, 4, HU3D_MOTATTR_LOOP);
                Hu3DMotionTimeSet(lbl_1_bss_10.openingMotionModel, 0.0f);
                Hu3DMotionTimingHookSet(lbl_1_bss_10.openingMotionModel, fn_1_139C);
            }
        }
        fn_1_1FF4(0);
        lbl_1_bss_10.timingState = 0;
        MgSeqModeChangeOff();
        lbl_1_bss_10.streamNo = HuAudSStreamPlay(MSM_STREAM_MGMUS_25);
    }
    characterModel = lbl_1_bss_10.characterModels[0];
    Hu3DModelObjMtxGet(lbl_1_bss_10.openingMotionModel, "P1stmov", matrix);
    Hu3DMtxTransGet(matrix, &pos);
    Hu3DMtxRotGet(matrix, &rot);
    Hu3DMtxScaleGet(matrix, &scale);
    Hu3DModelPosSetV(characterModel, &pos);
    Hu3DModelRotSetV(characterModel, &rot);
    Hu3DModelScaleSetV(characterModel, &scale);
    OSReport("pos %f, %f, %f\n", pos.x, pos.y, pos.z);
    Hu3DModelObjPosGet(lbl_1_bss_10.openingMotionModel, "P1stmov", &lbl_1_bss_0);
    if (fn_1_20D8()) {
        HU3D_MODELID characterModel;
        HuVecF origin;
        HuVecF rotation;
        HuVecF unitScale;

        Hu3DMotionTimingHookReset(lbl_1_bss_10.openingMotionModel);
        Hu3DModelHookReset(lbl_1_bss_10.openingMotionModel);
        Hu3DModelHookSet(lbl_1_bss_10.playerModels[0], "P1st",
            lbl_1_bss_10.characterModels[0]);
        characterModel = lbl_1_bss_10.characterModels[0];
        origin.x = 0.0f;
        origin.y = 0.0f;
        origin.z = 0.0f;
        rotation.x = 0.0f;
        rotation.y = 0.0f;
        rotation.z = 0.0f;
        unitScale.x = 1.0f;
        unitScale.y = 1.0f;
        unitScale.z = 1.0f;
        Hu3DModelPosSetV(characterModel, &origin);
        Hu3DModelRotSetV(characterModel, &rotation);
        Hu3DModelScaleSetV(characterModel, &unitScale);
        fn_1_1FF4(1);
        for (playerNo = 0; playerNo < 4; playerNo++) {
            fn_1_22BC(playerNo, 0, HU3D_MOTATTR_LOOP);
            lbl_1_bss_10.previousButtons[playerNo] = 0;
        }
        fn_1_23B0();
        lbl_1_bss_10.sequenceState = 1;
        lbl_1_bss_10.sequenceFrame = 0;
        MgSeqModeNext();
    }
}

void fn_1_4BC(s16 mode, s16 frameNo)
{
}

/* MGSEQ_PARAM mainHook: gather answers each frame, score unique choices, and advance or start
 * another round. */
void fn_1_4C0(s16 mode, s16 frameNo)
{
    s32 playerNo;
    s32 choiceId;
    HU3D_MOTIONID playerMotion;
    HU3D_MODELID playerModel;

    switch (lbl_1_bss_10.sequenceState) {
    case 0:
        if (lbl_1_bss_10.sequenceFrame == 0) {
            for (playerNo = 0; playerNo < 4; playerNo++) {
                fn_1_22BC(playerNo, 0, HU3D_MOTATTR_LOOP);
                lbl_1_bss_10.previousButtons[playerNo] = 0;
            }
            fn_1_23B0();
        }
        if (lbl_1_bss_10.sequenceFrame >= 120) {
            lbl_1_bss_10.sequenceState = 1;
            lbl_1_bss_10.sequenceFrame = 0;
            break;
        }
        lbl_1_bss_10.sequenceFrame++;
        break;
    case 1:
        for (playerNo = 0; playerNo < 4; playerNo++) {
            fn_1_22BC(playerNo, 9, HU3D_MOTATTR_LOOP);
            lbl_1_bss_10.choices[playerNo] = 0;
            fn_1_215C(playerNo, 5);
        }
        MgTimerParamSet(lbl_1_bss_10.timer, 300, 0, 0); /* The answer window lasts 300 frames. */
        MgTimerModeOnSet(lbl_1_bss_10.timer, MGTIMER_OFFTYPE_FADEOUT);
        lbl_1_bss_10.sequenceState = 2;
        lbl_1_bss_10.sequenceFrame = 0;
        break;
    case 2:
        if (MgTimerDoneCheck(lbl_1_bss_10.timer)) {
            lbl_1_bss_10.sequenceState = 3;
            lbl_1_bss_10.sequenceFrame = 0;
            break;
        }
        for (playerNo = 0; playerNo < 4; playerNo++) {
            choiceId = lbl_1_bss_10.choices[playerNo];
            if (GwPlayerConf[playerNo].type == 1) {
                if (lbl_1_bss_10.sequenceFrame >= lbl_1_bss_10.cpuInputFrames[playerNo]) {
                    choiceId = fn_1_2580(playerNo);
                } else {
                    choiceId = 0;
                }
            } else {
                u16 buttons;

                buttons = HuPadBtn[lbl_1_bss_10.padNos[playerNo]];
                buttons &=
                    PAD_BUTTON_A | PAD_BUTTON_B | PAD_BUTTON_TRIGGER_L | PAD_BUTTON_TRIGGER_R;
                /* Only newly pressed choice buttons change the stored answer, which remains
                 * selected between presses; simultaneous new presses use A, B, L, then R
                 * priority. */
                switch (buttons & ~lbl_1_bss_10.previousButtons[playerNo]) {
                case 0:
                    break;
                case PAD_BUTTON_A:
                    choiceId = 1;
                    break;
                case PAD_BUTTON_B:
                    choiceId = 2;
                    break;
                case PAD_BUTTON_TRIGGER_L:
                    choiceId = 3;
                    break;
                case PAD_BUTTON_TRIGGER_R:
                    choiceId = 4;
                    break;
                default:
                    OSReport("同時！！！\n");
                    if (buttons & ~lbl_1_bss_10.previousButtons[playerNo] & PAD_BUTTON_A) {
                        choiceId = 1;
                    } else if (buttons & ~lbl_1_bss_10.previousButtons[playerNo] & PAD_BUTTON_B) {
                        choiceId = 2;
                    } else if (buttons & ~lbl_1_bss_10.previousButtons[playerNo] &
                               PAD_BUTTON_TRIGGER_L) {
                        choiceId = 3;
                    } else if (buttons & ~lbl_1_bss_10.previousButtons[playerNo] &
                               PAD_BUTTON_TRIGGER_R) {
                        choiceId = 4;
                    }
                    break;
                }
                lbl_1_bss_10.previousButtons[playerNo] = buttons;
            }
            if (choiceId != lbl_1_bss_10.choices[playerNo]) {
                fn_1_22BC(playerNo, 10, 0);
                omVibrate(playerNo, 10, 7, 3);
                fn_1_215C(playerNo, 12);
                HuAudFXPlay(M616_SFX_CHOICE_CHANGED);
            }
            lbl_1_bss_10.choices[playerNo] = choiceId;
            playerMotion = lbl_1_bss_10.characterMotions[playerNo][10];
            playerModel = lbl_1_bss_10.characterModels[playerNo];
            if (Hu3DMotionShiftIDGet(playerModel) < 0 &&
                playerMotion == Hu3DMotionIDGet(playerModel) && Hu3DMotionEndCheck(playerModel)) {
                fn_1_22BC(playerNo, 9, 0);
                OSReport("push end\n");
            }
        }
        lbl_1_bss_10.sequenceFrame++;
        break;
    case 3:
        if (lbl_1_bss_10.sequenceFrame == 0) {
            for (playerNo = 0; playerNo < 4; playerNo++) {
                fn_1_22BC(playerNo, 0, HU3D_MOTATTR_LOOP);
                {
                    /* Answers 1 to 4 map to base motions 0 to 3; the zero-answer slot is
                     * skipped. */
                    s32 motions[5] = { 7, 0, 1, 2, 3 };

                    if (lbl_1_bss_10.choices[playerNo] != 0) {
                        fn_1_215C(playerNo, motions[lbl_1_bss_10.choices[playerNo]]);
                    }
                }
            }
        } else if (lbl_1_bss_10.sequenceFrame >= 60) {
            lbl_1_bss_10.sequenceState = 4;
            lbl_1_bss_10.sequenceFrame = 0;
            break;
        }
        lbl_1_bss_10.sequenceFrame++;
        break;
    case 4:
        {
            s32 otherPlayer;
            s32 uniqueChoiceCount = 0;
            s32 matchingPlayerCount;
            s32 hasUniqueChoice[4];

            OSReport("entry ... %d, %d, %d, %d\n", lbl_1_bss_10.choices[0], lbl_1_bss_10.choices[1],
                lbl_1_bss_10.choices[2], lbl_1_bss_10.choices[3]);
            for (playerNo = 0; playerNo < 4; playerNo++) {
                if (lbl_1_bss_10.choices[playerNo] == 0) {
                    hasUniqueChoice[playerNo] = 0;
                } else {
                    matchingPlayerCount = 0;
                    for (otherPlayer = 0; otherPlayer < 4; otherPlayer++) {
                        if (lbl_1_bss_10.choices[otherPlayer] == lbl_1_bss_10.choices[playerNo]) {
                            matchingPlayerCount++;
                        }
                    }
                    if (matchingPlayerCount == 1) {
                        hasUniqueChoice[playerNo] = 1;
                    } else {
                        hasUniqueChoice[playerNo] = 0;
                    }
                }
                if (hasUniqueChoice[playerNo] == 1) {
                    uniqueChoiceCount++;
                }
            }
            if (uniqueChoiceCount != 0) {
                for (playerNo = 0; playerNo < 4; playerNo++) {
                    if (hasUniqueChoice[playerNo] == 1) {
                        lbl_1_bss_10.scores[playerNo]++;
                        fn_1_22BC(playerNo, 7, 0);
                        CharFXPlay(lbl_1_bss_10.characterNos[playerNo], 579);
                        fn_1_2104(playerNo, lbl_1_bss_10.scores[playerNo]);
                        switch (lbl_1_bss_10.choices[playerNo]) {
                        case 1:
                            fn_1_215C(playerNo, 8);
                            break;
                        case 2:
                            fn_1_215C(playerNo, 9);
                            break;
                        case 3:
                            fn_1_215C(playerNo, 10);
                            break;
                        case 4:
                            fn_1_215C(playerNo, 11);
                            break;
                        }
                    } else {
                        fn_1_22BC(playerNo, 8, 0);
                    }
                }
                fn_1_2254(0);
                HuAudFXPlay(M616_SFX_RESULT_FIRST);
                HuAudFXPlay(M616_SFX_RESULT_SECOND);
                lbl_1_bss_10.sequenceState = 5;
                lbl_1_bss_10.sequenceFrame = 0;
            } else {
                for (playerNo = 0; playerNo < 4; playerNo++) {
                    fn_1_22BC(playerNo, 8, 0);
                }
                if (lbl_1_bss_10.roundNo == 9) {
                    lbl_1_bss_10.sequenceState = 8;
                    lbl_1_bss_10.sequenceFrame = 0;
                } else {
                    lbl_1_bss_10.roundNo++;
                    lbl_1_bss_10.sequenceState = 0;
                    lbl_1_bss_10.sequenceFrame = 0;
                }
            }
        }
        break;
    case 5:
        for (playerNo = 0; playerNo < 4; playerNo++) {
            if (Hu3DMotionEndCheck(lbl_1_bss_10.characterModels[playerNo])) {
                fn_1_22BC(playerNo, 0, HU3D_MOTATTR_LOOP);
            }
        }
        {
            for (playerNo = 0; playerNo < 4; playerNo++) {
                if (!Hu3DMotionEndCheck(lbl_1_bss_10.playerModels[playerNo])) {
                    break;
                }
            }
            if (playerNo == 4) {
                HuAudFXPlay(M616_SFX_MOTION_COMPLETE_FIRST);
                HuAudFXPlay(M616_SFX_MOTION_COMPLETE_SECOND);
                for (playerNo = 0; playerNo < 4; playerNo++) {
                    if (lbl_1_bss_10.scores[playerNo] == 3) {
                        break;
                    }
                }
                if (playerNo != 4 || lbl_1_bss_10.roundNo == 9) {
                    lbl_1_bss_10.sequenceState = 8;
                    lbl_1_bss_10.sequenceFrame = 0;
                } else {
                    lbl_1_bss_10.roundNo++;
                    lbl_1_bss_10.sequenceState = 0;
                    lbl_1_bss_10.sequenceFrame = 0;
                }
            } else {
                lbl_1_bss_10.sequenceFrame++;
            }
        }
        break;
    case 8:
        MgSeqModeNext();
        break;
    }
}

/* MGSEQ_PARAM finishHook: on frame 0, begin fading out the active streamed track. */
void fn_1_F14(s16 mode, s16 frameNo)
{
    if (frameNo == 0) {
        fn_1_104(lbl_1_bss_10.streamNo);
        OSReport("enter sequence ... finish\n");
    }
}

/* MGSEQ_PARAM preWinnerHook: record players reaching three points, present the outcome, then wait
 * for motions. */
void fn_1_F78(s16 mode, s16 frameNo)
{
    s32 playerNo;
    s32 winnerSeatMask;
    s32 winnerCount = 0;
    s16 resultMotion = -1;
    s16 winnerCharacterNos[4] = { -1, -1, -1, -1 };
    BOOL isDraw = TRUE;

    if (frameNo == 0) {
        for (playerNo = 0; playerNo < 4; playerNo++) {
            if (lbl_1_bss_10.scores[playerNo] == 3) {
                winnerCharacterNos[winnerCount++] = lbl_1_bss_10.characterNos[playerNo];
                if (!_CheckFlag(FLAG_MG_PRACTICE)) {
                    /* Outside practice mode, reaching three points sets this player's minigame coin
                     * bonus to 10. */
                    GwPlayer[playerNo].mgCoinBonus = 10;
                }
                isDraw = FALSE;
            }
        }
        MgSeqWinnerSet(winnerCharacterNos[0], winnerCharacterNos[1], winnerCharacterNos[2],
                       winnerCharacterNos[3]);
        if (!isDraw) {
            for (playerNo = 0; playerNo < 4; playerNo++) {
                if (lbl_1_bss_10.scores[playerNo] == 1) {
                    fn_1_2104(playerNo, 4);
                    /* A one-point player makes the center gear use motion 1, overriding motion 2
                     * selected for a two-point player. */
                    if (resultMotion != 1) {
                        resultMotion = 1;
                    }
                } else if (lbl_1_bss_10.scores[playerNo] == 2) {
                    fn_1_2104(playerNo, 5);
                    if (resultMotion == -1) {
                        resultMotion = 2;
                    }
                }
                fn_1_215C(playerNo, 5);
            }
            if (resultMotion != -1) {
                fn_1_2254(resultMotion);
                HuAudFXPlay(M616_SFX_RESULT_FIRST);
                HuAudFXPlay(M616_SFX_RESULT_SECOND);
            }
            winnerSeatMask = 0;
            {
                s32 cameraByWinnerMask[16] = {
                    0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
                };
                /* The mask identifies seats with three points, and its value selects their result
                 * camera. */
                for (playerNo = 0; playerNo < 4; playerNo++) {
                    if (lbl_1_bss_10.scores[playerNo] == 3) {
                        winnerSeatMask |= 0x1 << playerNo;
                    }
                }
                fn_1_1FF4(cameraByWinnerMask[winnerSeatMask]);
            }
        }
        OSReport("enter sequence ... end\n");
    } else {
        for (playerNo = 0; playerNo < 4; playerNo++) {
            if (!Hu3DMotionEndCheck(lbl_1_bss_10.playerModels[playerNo])) {
                break;
            }
        }
        if (playerNo == 4) {
            HuAudFXPlay(M616_SFX_MOTION_COMPLETE_FIRST);
            HuAudFXPlay(M616_SFX_MOTION_COMPLETE_SECOND);
            MgSeqModeNext();
        }
    }
}

/* MGSEQ_PARAM winnerHook: on frame 0, give three-point winners their winner pose and reveal their
 * seat's result prop; give every other player the non-winner pose. */
void fn_1_1284(s16 mode, s16 frameNo)
{
    s32 playerNo;

    if (frameNo == 0) {
        for (playerNo = 0; playerNo < 4; playerNo++) {
            if (lbl_1_bss_10.scores[playerNo] == 3) {
                fn_1_22BC(playerNo, 2, 0);
                Hu3DMotionTimeSet(lbl_1_bss_10.resultModels[playerNo], 0.0f);
                Hu3DModelAttrReset(lbl_1_bss_10.resultModels[playerNo], HU3D_ATTR_DISPOFF);
            } else {
                fn_1_22BC(playerNo, 3, 0);
            }
        }
        OSReport("enter sequence ... result\n");
    }
}

void fn_1_135C(s16 mode, s16 frameNo)
{
}

void fn_1_1360(s16 mode, s16 frameNo)
{
}

/* Object callback registered by fn_1_1590; ignore the callback object and sample the active camera
 * model's motion time each update. */
void fn_1_1364(OMOBJ *cameraObject)
{
    lbl_1_bss_10.cameraTime = Hu3DMotionTimeGet(lbl_1_bss_10.activeCameraModel);
}

/* When the opening-motion timing callback reports timingLag, advance the lead character's motion
 * and voice events and create landing dust at the two landing events. */
void fn_1_139C(HU3D_MODELID modelId, HU3D_MOTIONID motionId, BOOL timingLag)
{
    if (timingLag == TRUE) {
        switch (lbl_1_bss_10.timingState) {
        case 0:
            fn_1_22BC(0, 1, 0);
            lbl_1_bss_10.timingState++;
            CharModelVoiceFlagSet(lbl_1_bss_10.characterNos[0], FALSE);
            break;
        case 1:
            fn_1_22BC(0, 6, 0);
            lbl_1_bss_10.timingState++;
            break;
        case 2:
            fn_1_22BC(0, 4, HU3D_MOTATTR_LOOP);
            CharModelLandDustCreate(lbl_1_bss_10.characterNos[0], &lbl_1_bss_0);
            lbl_1_bss_10.timingState++;
            CharModelVoiceFlagSet(lbl_1_bss_10.characterNos[0], TRUE);
            break;
        case 3:
            CharModelVoiceFlagSet(lbl_1_bss_10.characterNos[0], FALSE);
            fn_1_22BC(0, 5, 0);
            lbl_1_bss_10.timingState++;
            break;
        case 4:
            fn_1_22BC(0, 6, 0);
            lbl_1_bss_10.timingState++;
            break;
        case 5:
            CharModelVoiceFlagSet(lbl_1_bss_10.characterNos[0], TRUE);
            fn_1_22BC(0, 0, 0);
            CharModelLandDustCreate(lbl_1_bss_10.characterNos[0], &lbl_1_bss_0);
            lbl_1_bss_10.timingState++;
            break;
        }
    }
}
