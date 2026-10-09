/* Runs the player, camera, course animation and results sequences. */
#include "REL/m608dll.h"
#include "math.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common_Embedded/Math/fdlibm.h"

/* Checked while the results sequence handles team mode. */
#define M608_TEAM_MODE_FLAG 0x30002U

/* Effect IDs are named for the gameplay transition where this file plays them. */
#define M608_JUMP_START_FX 1628
#define M608_JUMP_PROGRESS_FX 1623
#define M608_PATH_SAMPLE_FX 1626
#define M608_DIRECTION_SUCCESS_FX 1625
#define M608_RUN_END_FX 1627
#define M608_CONTROL_START_FX 1624
/* Gates the post-results record prompt and 10-coin winner bonus. */
#define M608_RECORD_PROMPT_FLAG 0x1000FU

/* Sequence callbacks used to start the run, update riders, and show results. */
MGSEQ_PARAM lbl_1_data_0 = {
    0, 1, fn_1_140, fn_1_190, fn_1_57C, fn_1_580, fn_1_1A2C, fn_1_1DD8,
    fn_1_2374, fn_1_2538, fn_1_253C,
};

f32 lbl_1_data_28[14] = {
    -35.0f, -35.0f, -35.0f, -25.0f, -35.0f, -35.0f, -35.0f, -5.0f,
    -35.0f, -10.0f, -5.0f, -10.0f, -10.0f, -10.0f,
};

f32 lbl_1_data_60[14] = {
    -10.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
};

f32 lbl_1_data_98[14] = {
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.4f, 1.0f, 1.0f, 0.4f, 1.0f, 1.0f, 1.0f,
};

char lbl_1_data_D0[9] = "ske_head";

char lbl_1_data_D9[16] = "1P neck ... %f\012";

char lbl_1_data_E9[16] = "2P neck ... %f\012";

char lbl_1_data_F9[16] = "3P neck ... %f\012";

char lbl_1_data_109[16] = "4P neck ... %f\012";

char lbl_1_data_119[14] = "snowboard_txt";

char lbl_1_data_127[5] = "p2st";

char lbl_1_data_12C[22] = "608kurukuru-pmov_null";

char lbl_1_data_142[5] = "p1st";

char lbl_1_data_147[26] = "*** wait vibration start\012";

char lbl_1_data_161[24] = "*** wait vibration end\012";

char lbl_1_data_179[16] = "*** jump start\012";

char lbl_1_data_189[7] = "pmov\000\000";

static M608PathSamples lbl_1_bss_10;

static s32 lbl_1_bss_C;

static s16 lbl_1_bss_8;

static f32 lbl_1_bss_4;

static f32 lbl_1_bss_0;

/* With no current track, starts the fallback only when message-status bit 0x10 is set. */
s32 fn_1_A0(s32 currentTrackHandle, s32 fallbackBgmId)
{
    s32 trackHandle;

    trackHandle = currentTrackHandle;
    if ((trackHandle == -1) && ((s32) (GameMesStatGet(MgSeqGameMesIdGet()) & 0x10) != 0)) {
        trackHandle = HuAudBGMPlay((s16) fallbackBgmId);
    }
    return trackHandle;
}

/* Fades out a streamed background track when its handle is valid. */
void fn_1_104(s32 streamHandle)
{
    if (streamHandle != -1) {
        HuAudSStreamFadeOut(streamHandle, 100);
    }
}

/* Clears the course rotation accumulators and advances to the next mode. */
void fn_1_140(s16 sequenceMode, s16 frameNo)
{
    lbl_1_bss_0 = 0.0f;
    lbl_1_bss_4 = 0.0f;
    MgSeqModeNext();
}

/* Hooks up the riders and course camera, then waits for the intro motion. */
void fn_1_190(s16 sequenceMode, s16 frameNo)
{
    Point3d boardPosition;
    int charNo;
    char *itemHook;
    s32 playerIndex;

    if (frameNo == 0) {
        fn_1_5E6C(0);
        playerIndex = 0;
        while (playerIndex < 4) {
            charNo = lbl_1_bss_3E80.characterNos[playerIndex];
            CharMotionSet((s16) charNo, lbl_1_bss_3E80.characterMotions[playerIndex][0]);
            Hu3DModelAttrSet(lbl_1_bss_3E80.characterModels[playerIndex], HU3D_MOTATTR_LOOP);
            Hu3DModelHookSet(lbl_1_bss_3E80.playerModelsA[playerIndex], lbl_1_data_5D0[playerIndex],
                             lbl_1_bss_3E80.characterModels[playerIndex]);
            itemHook = CharModelItemHookGet(lbl_1_bss_3E80.characterNos[playerIndex], 4, 2);
            Hu3DModelHookSet(lbl_1_bss_3E80.characterModels[playerIndex], itemHook,
                             lbl_1_bss_3E80.secondaryModels[playerIndex]);
            playerIndex += 1;
        }
        Hu3DObjHookSet(lbl_1_bss_3E80.characterModels[0], lbl_1_data_D0, fn_1_38F0);
        Hu3DObjHookSet(lbl_1_bss_3E80.characterModels[1], lbl_1_data_D0, fn_1_39B4);
        Hu3DObjHookSet(lbl_1_bss_3E80.characterModels[2], lbl_1_data_D0, fn_1_3A78);
        Hu3DObjHookSet(lbl_1_bss_3E80.characterModels[3], lbl_1_data_D0, fn_1_3B3C);
        OSReport(lbl_1_data_D9, lbl_1_data_28[lbl_1_bss_3E80.characterNos[0]]);
        OSReport(lbl_1_data_E9, lbl_1_data_28[lbl_1_bss_3E80.characterNos[1]]);
        OSReport(lbl_1_data_F9, lbl_1_data_28[lbl_1_bss_3E80.characterNos[2]]);
        OSReport(lbl_1_data_109, lbl_1_data_28[lbl_1_bss_3E80.characterNos[3]]);
        Hu3DObjHookSet(lbl_1_bss_3E80.secondaryModels[0], lbl_1_data_119, fn_1_3F18);
        Hu3DObjHookSet(lbl_1_bss_3E80.secondaryModels[1], lbl_1_data_119, fn_1_400C);
        Hu3DObjHookSet(lbl_1_bss_3E80.secondaryModels[2], lbl_1_data_119, fn_1_40F0);
        Hu3DObjHookSet(lbl_1_bss_3E80.secondaryModels[3], lbl_1_data_119, fn_1_41D4);
        fn_1_5FDC(3, 0);
        fn_1_5FDC(4, 2);
        if ((s32) lbl_1_bss_3E80.featuredRiderIndex == 0) {
            fn_1_5FDC(5, 4);
        }
        fn_1_5FDC(10, 8);
        Hu3DModelObjPosGet(lbl_1_bss_3E80.playerModelsA[1], lbl_1_data_127, &boardPosition);
        fn_1_5DA4(boardPosition);
        lbl_1_bss_3E80.courseMusicStreamHandle = HuAudSStreamPlay(MSM_STREAM_MGMUS_22);
        return;
    }
    if (fn_1_5F64() != 0) {
        fn_1_614C();
        lbl_1_bss_3E80.activePlayer = 0;
        lbl_1_bss_3E80.state = 0;
        lbl_1_bss_3E80.frame = 0;
        MgSeqModeNext();
    }
}

/* Empty callback-table entry; it performs no work when the sequence reaches this slot. */
void fn_1_57C(s16 sequenceMode, s16 frameNo)
{

}

/* Advances the active rider's course run, score and animation states. */
void fn_1_580(s16 sequenceMode, s16 frameNo)
{
    Point3d courseHookPosition;
    Point3d riderBoardPosition;
    Point3d firstPlayerBoardPosition;
    Point3d courseStartPosition;
    Point3d courseEndPosition;
    Point3d courseWaitPosition;
    Point3d mainModelPosition;
    Point3d activeCoursePosition;
    Point3d finalCoursePosition;
    Point3d resultsCourseHookPosition;
    int characterModelId;
    int characterNumber;
    s32 activeRiderIndex;
    u32 soundPitch;
    s32 playerHookIndex;
    s32 directionMatched;
    s32 loopIndex;
    s32 pressedButtons;
    s32 scorePitch;

    activeRiderIndex = lbl_1_bss_3E80.activePlayer;
    characterModelId = lbl_1_bss_3E80.characterModels[activeRiderIndex];
    characterNumber = lbl_1_bss_3E80.characterNos[activeRiderIndex];
    if (frameNo == 0) {
        fn_1_6578();
    }
    Hu3DModelObjPosGet(lbl_1_bss_3E80.mainModel, lbl_1_data_12C, &courseHookPosition);
    fn_1_38EC(courseHookPosition);
    switch (lbl_1_bss_3E80.state) {  /* switch 1 */
    case 0:                                         /* switch 1 */
        fn_1_5E6C(1);
        CharMotionSet((s16) characterNumber, lbl_1_bss_3E80.characterMotions[activeRiderIndex][1]);
        Hu3DModelAttrReset(lbl_1_bss_3E80.characterModels[activeRiderIndex], HU3D_MOTATTR_LOOP);
        Hu3DMotionSpeedSet(lbl_1_bss_3E80.characterModels[activeRiderIndex], 0.5f);
        Hu3DAnimSpeedSet(lbl_1_bss_3E80.characterModels[activeRiderIndex], 1.0f);
        Hu3DObjHookSet(lbl_1_bss_3E80.characterModels[activeRiderIndex], lbl_1_data_D0, fn_1_3C84);
        Hu3DObjHookSet(lbl_1_bss_3E80.secondaryModels[activeRiderIndex], lbl_1_data_119, fn_1_3E00);
        Hu3DModelObjPosGet(lbl_1_bss_3E80.playerModelsA[activeRiderIndex], lbl_1_data_142,
                           &riderBoardPosition);
        fn_1_5DA4(riderBoardPosition);
        lbl_1_bss_3E80.state = 1U;
        lbl_1_bss_3E80.frame = 0;
        break;
    case 1:                                         /* switch 1 */
        if (fn_1_5F64() != 0) {
            Hu3DObjHookSet(lbl_1_bss_3E80.characterModels[0], lbl_1_data_D0, fn_1_3C00);
            Hu3DObjHookSet(lbl_1_bss_3E80.secondaryModels[0], lbl_1_data_119, fn_1_3D7C);
            Hu3DMotionCalc(lbl_1_bss_3E80.characterModels[0]);
            Hu3DMotionCalc(lbl_1_bss_3E80.secondaryModels[0]);
            fn_1_5E6C(3);
            lbl_1_bss_3E80.state = 4U;
            lbl_1_bss_3E80.frame = 0;
        } else {
            omSetRot(lbl_1_bss_3E80.playerObjects[0], 0.0f, lbl_1_bss_0, 0.0f);
            lbl_1_bss_0 += 1.4f;
            lbl_1_bss_4 += lbl_1_data_98[characterNumber];
            lbl_1_bss_3E80.frame++;
        }
        break;
    case 2:                                         /* switch 1 */
        fn_1_5E6C(3);
        lbl_1_bss_3E80.state = 4U;
        lbl_1_bss_3E80.frame = 0;
        break;
    case 3:                                         /* switch 1 */
        if ((s32) lbl_1_bss_3E80.frame == 0) {
            Hu3DModelObjPosGet(lbl_1_bss_3E80.playerModelsA[0], lbl_1_data_142,
                               &firstPlayerBoardPosition);
            fn_1_5DA4(firstPlayerBoardPosition);
            Hu3DObjHookSet(lbl_1_bss_3E80.characterModels[activeRiderIndex], lbl_1_data_D0,
                           fn_1_3C00);
            Hu3DObjHookSet(lbl_1_bss_3E80.secondaryModels[activeRiderIndex], lbl_1_data_119,
                           fn_1_3E00);
            omSetRot(lbl_1_bss_3E80.playerObjects[activeRiderIndex], 0.0f, 45.0f, 0.0f);
            CharMotionSet((s16) characterNumber,
                          lbl_1_bss_3E80.characterMotions[activeRiderIndex][2]);
            Hu3DMotionCalc(lbl_1_bss_3E80.characterModels[activeRiderIndex]);
            fn_1_5F90(0);
            fn_1_5F90(1);
            fn_1_5F90(7);
            fn_1_5F90(8);
            fn_1_5F90(9);
            fn_1_5F90(5);
            fn_1_5FDC(3, 1);
            fn_1_5FDC(4, 3);
            if ((s32) lbl_1_bss_3E80.featuredRiderIndex == activeRiderIndex) {
                fn_1_5FDC(5, 5);
            }
            fn_1_5FDC(10, 8);
            fn_1_605C(9);
            CharMotionShiftSet((s16) characterNumber,
                               lbl_1_bss_3E80.characterMotions[activeRiderIndex][2], 0.0f, 8.0f,
                               HU3D_MOTATTR_LOOP);
            fn_1_5E6C(3);
            lbl_1_bss_3E80.state = 4U;
            lbl_1_bss_3E80.frame = 0;
        } else {
            lbl_1_bss_3E80.frame++;
        }
        break;
    case 4:                                         /* switch 1 */
        if ((s32) lbl_1_bss_3E80.frame == 0) {
            omSetRot(lbl_1_bss_3E80.playerObjects[activeRiderIndex], 0.0f, 55.0f, 0.0f);
            Hu3DObjHookSet(lbl_1_bss_3E80.characterModels[activeRiderIndex], lbl_1_data_D0,
                           fn_1_3C00);
            Hu3DObjHookSet(lbl_1_bss_3E80.secondaryModels[activeRiderIndex], lbl_1_data_119,
                           fn_1_3D7C);
            CharMotionShiftSet((s16) characterNumber,
                               lbl_1_bss_3E80.characterMotions[activeRiderIndex][2], 0.0f, 8.0f,
                               HU3D_MOTATTR_LOOP);
            omVibrate((s16) activeRiderIndex, 30, 7, 3);
            loopIndex = 0;
            while (loopIndex < 27) {
                Hu3DModelAttrSet(lbl_1_bss_3E80.movingCourseModels[loopIndex], HU3D_ATTR_DISPOFF);
                loopIndex += 1;
            }
            lbl_1_bss_3E80.timingSubstate = 0;
            lbl_1_bss_3E80.riderTurnAngle = 0.0f;
            lbl_1_bss_3E80.riderTurnStartAngle = 0.0f;
            lbl_1_bss_3E80.riderTurnTargetAngle = 0.0f;
            lbl_1_bss_3E80.riderTurnFrame = 0;
            lbl_1_bss_3E80.coursePathSampleCount = 0;
            lbl_1_bss_3E80.scores[lbl_1_bss_3E80.activePlayer] = 0;
            Hu3DModelObjPosGet(lbl_1_bss_3E80.playerModelsA[0], lbl_1_data_142,
                               &courseStartPosition);
            fn_1_5DA4(courseStartPosition);
            Hu3DLayerHookSet(3, fn_1_437C);
            OSReport(lbl_1_data_147);
        } else if ((s32) lbl_1_bss_3E80.frame >= 60) {
            Hu3DModelHookReset(lbl_1_bss_3E80.playerModelsA[0]);
            Hu3DModelHookSet(lbl_1_bss_3E80.mainModel, lbl_1_data_12C, (s16) characterModelId);
            Hu3DMotionSet(lbl_1_bss_3E80.mainModel, lbl_1_bss_3E80.mainMotion);
            Hu3DModelAttrSet(lbl_1_bss_3E80.mainModel, HU3D_MOTATTR_PAUSE);
            omSetRot(lbl_1_bss_3E80.playerObjects[activeRiderIndex], 0.0f, 0.0f, 0.0f);
            Hu3DModelRotSet((s16) characterModelId, 0.0f, 0.0f, 0.0f);
            Hu3DMotionCalc(lbl_1_bss_3E80.mainModel);
            Hu3DMotionCalc((s16) characterModelId);
            Hu3DMotionCalc(lbl_1_bss_3E80.playerModelsA[0]);
            Hu3DMotionCalc(lbl_1_bss_3E80.secondaryModels[activeRiderIndex]);
            Hu3DMotionTimingHookSet(lbl_1_bss_3E80.timingModels[0], NULL);
            Hu3DMotionTimingHookSet(lbl_1_bss_3E80.timingModels[1], NULL);
            if (activeRiderIndex < 3) {
                Hu3DMotionTimingHookSet(lbl_1_bss_3E80.timingModels[0], fn_1_2FD8);
                Hu3DMotionTimeSet(lbl_1_bss_3E80.timingModels[0], 0.0f);
            } else {
                Hu3DMotionTimingHookSet(lbl_1_bss_3E80.timingModels[1], fn_1_2FD8);
                Hu3DMotionTimeSet(lbl_1_bss_3E80.timingModels[1], 0.0f);
            }
            lbl_1_bss_3E80.timingSubstate = 0;
            lbl_1_bss_3E80.state = 5U;
            lbl_1_bss_3E80.frame = 0;
            Hu3DModelObjPosGet(lbl_1_bss_3E80.playerModelsA[0], lbl_1_data_142, &courseEndPosition);
            fn_1_5DA4(courseEndPosition);
            OSReport(lbl_1_data_161);
            break;
        } else {
            Hu3DModelObjPosGet(lbl_1_bss_3E80.playerModelsA[0], lbl_1_data_142,
                               &courseWaitPosition);
            fn_1_5DA4(courseWaitPosition);
        }
        lbl_1_bss_3E80.frame++;
        break;
    case 5:                                         /* switch 1 */
        if ((s32) lbl_1_bss_3E80.frame == 0) {
            lbl_1_bss_C = 0;
            lbl_1_bss_8 = Hu3DHookFuncCreate(fn_1_38E8);
            Hu3DModelLayerSet(lbl_1_bss_8, 1);
            omSetRot(lbl_1_bss_3E80.playerObjects[activeRiderIndex], 0.0f, 0.0f, 0.0f);
            CharMotionShiftSet((s16) characterNumber,
                               lbl_1_bss_3E80.characterMotions[activeRiderIndex][2], 0.0f, 8.0f,
                               HU3D_MOTATTR_LOOP);
            if (activeRiderIndex == 0) {
                fn_1_605C(8);
            }
            omSetRot(lbl_1_bss_3E80.playerObjects[activeRiderIndex], 0.0f, 0.0f, 0.0f);
            Hu3DModelAttrReset(lbl_1_bss_3E80.mainModel, HU3D_MOTATTR_PAUSE);
            HuAudFXPlay(M608_JUMP_START_FX);
            OSReport(lbl_1_data_179);
        }
        if ((s32) lbl_1_bss_3E80.frame == 19) {
            lbl_1_bss_3E80.jumpSoundHandle = HuAudFXPlay(M608_JUMP_PROGRESS_FX);
            HuAudFXPitchSet(lbl_1_bss_3E80.jumpSoundHandle, 0);
        }
        Hu3DModelObjPosGet(lbl_1_bss_3E80.mainModel, lbl_1_data_12C, &mainModelPosition);
        fn_1_5DA4(mainModelPosition);
        if ((s32) lbl_1_bss_3E80.frame >= 19) {
            soundPitch = ((lbl_1_bss_3E80.frame - 19) * 8191) / 46;
            if ((s32) soundPitch >= 8191) {
                soundPitch = 8191;
            }
            HuAudFXPitchSet(lbl_1_bss_3E80.jumpSoundHandle, (s16) soundPitch);
        }
        fn_1_6958(0);
        lbl_1_bss_3E80.frame++;
        break;
    case 6:                                         /* switch 1 */
        pressedButtons = 0;
        directionMatched = 0;
        if (GwPlayerConf[activeRiderIndex].type == 1) {
            pressedButtons = fn_1_6B90(activeRiderIndex);
        } else {
            pressedButtons = HuPadBtnRep[lbl_1_bss_3E80.padNos[activeRiderIndex]];
        }
        Hu3DModelObjPosGet(lbl_1_bss_3E80.mainModel, lbl_1_data_12C, &activeCoursePosition);
        fn_1_5DA4(activeCoursePosition);
        /* Only A/B/X/Y are considered, and a match requires the masked input to equal exactly the
         * expected button. */
        pressedButtons &= (PAD_BUTTON_A | PAD_BUTTON_B | PAD_BUTTON_X | PAD_BUTTON_Y);
        /* switch 2; irregular */
        switch (lbl_1_bss_3E80.scores[activeRiderIndex] % 4) {
        case 0:                                     /* switch 2 */
            if (pressedButtons == PAD_BUTTON_A) {
                directionMatched = 1;
            }
            break;
        case 1:                                     /* switch 2 */
            if (pressedButtons == PAD_BUTTON_B) {
                directionMatched = 1;
            }
            break;
        case 2:                                     /* switch 2 */
            if (pressedButtons == PAD_BUTTON_Y) {
                directionMatched = 1;
            }
            break;
        case 3:                                     /* switch 2 */
            if (pressedButtons == PAD_BUTTON_X) {
                directionMatched = 1;
            }
            break;
        }
        fn_1_6958((lbl_1_bss_3E80.scores[activeRiderIndex] % 4) + 1);
        if (directionMatched != 0) {
            /* Score and path sampling stop at 27 samples; later matched inputs still animate the
             * turn and play the success effect. */
            if ((s32) lbl_1_bss_3E80.coursePathSampleCount < 27) {
                lbl_1_bss_3E80.scores[activeRiderIndex]++;
                if ((lbl_1_bss_3E80.scores[activeRiderIndex] % 4) == 0) {
                    Hu3DModelObjPosGet(
                        lbl_1_bss_3E80.mainModel, lbl_1_data_189,
                        &lbl_1_bss_3E80
                             .movingCoursePositions[lbl_1_bss_3E80.coursePathSampleCount]);
                    (lbl_1_bss_3E80.movingCourseObjects[lbl_1_bss_3E80.coursePathSampleCount])
                        ->work[1] = 30;
                    lbl_1_bss_3E80.coursePathSampleCount++;
                    scorePitch = (lbl_1_bss_3E80.coursePathSampleCount - 1) * 341;
                    if (scorePitch >= 8192) {
                        scorePitch = 8191;
                    }
                    {
                        int effectHandle = HuAudFXPlay(M608_PATH_SAMPLE_FX);
                        HuAudFXPitchSet(effectHandle, (s16) scorePitch);
                    }
                }
                fn_1_6534((s16) activeRiderIndex,
                          (s16) (lbl_1_bss_3E80.scores[activeRiderIndex] * 90));
                Hu3DMotionTimeSet(lbl_1_bss_3E80.courseModels[0], 0.0f);
            }
            lbl_1_bss_3E80.riderTurnStartAngle = (f32) lbl_1_bss_3E80.riderTurnTargetAngle;
            lbl_1_bss_3E80.riderTurnTargetAngle =
                (f32) (lbl_1_bss_3E80.riderTurnStartAngle - 90.0f);
            lbl_1_bss_3E80.riderTurnAngle = (f32) lbl_1_bss_3E80.riderTurnStartAngle;
            lbl_1_bss_3E80.riderTurnFrame = 0;
            HuAudFXPlay(M608_DIRECTION_SUCCESS_FX);
        }
        fn_1_42B8();
        lbl_1_bss_3E80.frame++;
        break;
    case 7:                                         /* switch 1 */
        Hu3DModelObjPosGet(lbl_1_bss_3E80.mainModel, lbl_1_data_12C, &finalCoursePosition);
        fn_1_5DA4(finalCoursePosition);
        if ((s32) lbl_1_bss_3E80.frame == 0) {
            Hu3DModelHookReset(lbl_1_bss_3E80.secondaryModels[activeRiderIndex]);
            fn_1_6958(-1);
            omVibrate((s16) activeRiderIndex, 30, 7, 3);
            HuAudFXPlay(M608_RUN_END_FX);
            lbl_1_bss_3E80.jumpSoundHandle = HuAudFXPlay(M608_JUMP_PROGRESS_FX);
            HuAudFXPitchSet(lbl_1_bss_3E80.jumpSoundHandle, 4096);
        }
        if (activeRiderIndex == 3) {
            if ((lbl_1_bss_3E80.cameraMaxTime - lbl_1_bss_3E80.cameraTime) <= 240.0f) {
                MgSeqModeNext();
            }
        } else if ((lbl_1_bss_3E80.cameraMaxTime - lbl_1_bss_3E80.cameraTime) <= 60.0f) {
            lbl_1_bss_3E80.state = 8U;
            lbl_1_bss_3E80.frame = 0;
            WipeCreate(2, 0, 60);
            HuAudFXFadeOut(lbl_1_bss_3E80.jumpSoundHandle, 1000);
            break;
        }
        fn_1_42B8();
        lbl_1_bss_3E80.frame++;
        break;
    case 8:                                         /* switch 1 */
        if ((s32) lbl_1_bss_3E80.frame == 0) {
            Hu3DLayerHookReset(3);
            Hu3DModelKill(lbl_1_bss_8);
        }
        if ((s32) lbl_1_bss_3E80.frame >= 60) {
            Hu3DModelHookReset((s16) characterModelId);
            Hu3DModelHookSet(lbl_1_bss_3E80.playerModelsB[activeRiderIndex],
                             lbl_1_data_624[activeRiderIndex], (s16) characterModelId);
            CharMotionSet((s16) characterNumber,
                          lbl_1_bss_3E80.characterMotions[activeRiderIndex][5]);
            omSetRot(lbl_1_bss_3E80.playerObjects[activeRiderIndex], 0.0f, -90.0f, 0.0f);
            Hu3DModelHookReset((s16) characterModelId);
            Hu3DModelHookSet(lbl_1_bss_3E80.playerModelsB[activeRiderIndex],
                             lbl_1_data_684[activeRiderIndex],
                             lbl_1_bss_3E80.secondaryModels[activeRiderIndex]);
            loopIndex = 0;
            while (loopIndex < 4) {
                Hu3DModelHookReset(lbl_1_bss_3E80.playerModelsA[loopIndex]);
                loopIndex += 1;
            }
            playerHookIndex = 0;
            loopIndex = activeRiderIndex + 1;
            while (loopIndex < 4) {
                if (playerHookIndex == 0) {
                    omSetRot(lbl_1_bss_3E80.playerObjects[loopIndex], 0.0f, 90.0f, 0.0f);
                }
                Hu3DModelHookSet(lbl_1_bss_3E80.playerModelsA[playerHookIndex],
                                 lbl_1_data_5D0[playerHookIndex],
                                 lbl_1_bss_3E80.characterModels[loopIndex]);
                playerHookIndex += 1;
                loopIndex += 1;
            }
            Hu3DModelObjPosGet(lbl_1_bss_3E80.mainModel, lbl_1_data_12C,
                               &resultsCourseHookPosition);
            fn_1_5DA4(resultsCourseHookPosition);
            if (activeRiderIndex < 3) {
                fn_1_5E6C(2);
                WipeCreate(1, 5, 60);
                lbl_1_bss_3E80.activePlayer++;
                Hu3DObjHookSet(lbl_1_bss_3E80.secondaryModels[lbl_1_bss_3E80.activePlayer],
                               lbl_1_data_119, fn_1_3D7C);
                lbl_1_bss_3E80.state = 3U;
                lbl_1_bss_3E80.frame = 0;
                break;
            } else {
                MgSeqModeNext();
            }
        }
        fn_1_42B8();
        lbl_1_bss_3E80.frame++;
        break;
    }
    Hu3DMotionCalc(lbl_1_bss_3E80.mainModel);
    Hu3DMotionCalc((s16) characterModelId);
    Hu3DMotionCalc(lbl_1_bss_3E80.secondaryModels[activeRiderIndex]);
}

/* Determines tied winners, records a human high score and keeps the course shadow positioned. */
void fn_1_1A2C(s16 sequenceMode, s16 frameNo)
{
    Point3d summaryCoursePosition;
    s16 winners[4];
    s16 highestScore;
    s32 playerIndex;
    s32 humanWinnerFound;
    s32 audioStreamHandle;
    s32 winnerCount;

    if (frameNo == 0) {
        /* If all four scores are zero, mark no winners and skip the record comparison. */
        if (((s32) lbl_1_bss_3E80.scores[0] == 0) && ((s32) lbl_1_bss_3E80.scores[1] == 0) &&
            ((s32) lbl_1_bss_3E80.scores[2] == 0) && ((s32) lbl_1_bss_3E80.scores[3] == 0)) {
            lbl_1_bss_3E80.winners[0] = lbl_1_bss_3E80.winners[1] = lbl_1_bss_3E80.winners[2] =
                lbl_1_bss_3E80.winners[3] = 0;
            lbl_1_bss_3E80.recordBeaten = 0;
            winners[0] = winners[1] = winners[2] = winners[3] = -1;
        } else {
            highestScore = 0;
            winnerCount = 0;
            humanWinnerFound = 0;
            playerIndex = 0;
            while (playerIndex < 4) {
                if (highestScore < (s32) lbl_1_bss_3E80.scores[playerIndex]) {
                    highestScore = (s16) lbl_1_bss_3E80.scores[playerIndex];
                }
                playerIndex += 1;
            }
            playerIndex = 0;
            while (playerIndex < 4) {
                if (lbl_1_bss_3E80.scores[playerIndex] == highestScore) {
                    winners[winnerCount++] = playerIndex;
                    lbl_1_bss_3E80.winners[playerIndex] = 1;
                    if (GwPlayerConf[playerIndex].type == 0) {
                        humanWinnerFound = 1;
                    }
                } else {
                    lbl_1_bss_3E80.winners[playerIndex] = 0;
                }
                playerIndex += 1;
            }
            while (winnerCount < 4) {
                winners[winnerCount] = -1;
                winnerCount += 1;
            }
            if ((humanWinnerFound != 0) &&
                (highestScore > (s32) (((s32) lbl_1_bss_3E80.record) / 90))) {
                OSReport(lbl_1_data_1B4, (s16) highestScore, ((s32) lbl_1_bss_3E80.record));
                lbl_1_bss_3E80.recordBeaten = 1;
                lbl_1_bss_3E80.record = (s32) highestScore;
                MgSeqRecordSet(highestScore * 90);
            }
        }
        MgSeqWinnerSet(
            winners[0] >= 0 ? lbl_1_bss_3E80.characterNos[winners[0]] : -1,
            winners[1] >= 0 ? lbl_1_bss_3E80.characterNos[winners[1]] : -1,
            winners[2] >= 0 ? lbl_1_bss_3E80.characterNos[winners[2]] : -1,
            winners[3] >= 0 ? lbl_1_bss_3E80.characterNos[winners[3]] : -1);
        audioStreamHandle = lbl_1_bss_3E80.courseMusicStreamHandle;
        if (audioStreamHandle != -1) {
            HuAudSStreamFadeOut(audioStreamHandle, 100);
        }
    }
    Hu3DModelObjPosGet(lbl_1_bss_3E80.mainModel, lbl_1_data_12C, &summaryCoursePosition);
    fn_1_5DA4(summaryCoursePosition);
}

/* In team mode, saves scores and returns immediately; otherwise runs results and winner bonuses. */
void fn_1_1DD8(s16 sequenceMode, s16 frameNo)
{
    Point3d resultsHookPosition;
    Point3d resultsPosition;
    OMOBJ *recordPromptObject;
    int characterModelId;
    int characterNumber;
    s32 resultsPlayerIndex;
    s32 playerIndex;
    s32 score;

    resultsPlayerIndex = lbl_1_bss_3E80.activePlayer;
    characterModelId = lbl_1_bss_3E80.characterModels[resultsPlayerIndex];
    characterNumber = lbl_1_bss_3E80.characterNos[resultsPlayerIndex];
    if ((frameNo == 0) && (_CheckFlag(M608_TEAM_MODE_FLAG) != 0)) {
        playerIndex = 0;
        while (playerIndex < 4) {
            score = lbl_1_bss_3E80.scores[playerIndex] * 90;
            GwPlayer[playerIndex].mgScore = score;
            playerIndex += 1;
        }
        MgSeqModeSet(9U);
        return;
    }
    if (frameNo == 0) {
        WipeCreate(2, 0, 60);
        HuAudFXFadeOut(lbl_1_bss_3E80.jumpSoundHandle, 1000);
        lbl_1_bss_3E80.frame = 0;
        playerIndex = 0;
        while (playerIndex < 60) {
            Hu3DModelObjPosGet(lbl_1_bss_3E80.mainModel, lbl_1_data_12C, &resultsHookPosition);
            fn_1_5DA4(resultsHookPosition);
            HuPrcVSleep();
            playerIndex += 1;
        }
        Hu3DModelAttrSet(lbl_1_bss_3E80.timingModels[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_3E80.timingModels[1], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_3E80.courseModels[6], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_3E80.mainModel, HU3D_ATTR_DISPOFF);
        Hu3DModelHookReset(lbl_1_bss_3E80.mainModel);
        Hu3DModelHookReset((s16) characterModelId);
        Hu3DModelHookSet(lbl_1_bss_3E80.playerModelsB[resultsPlayerIndex],
                         lbl_1_data_624[resultsPlayerIndex], (s16) characterModelId);
        CharMotionSet((s16) characterNumber,
                      lbl_1_bss_3E80.characterMotions[resultsPlayerIndex][5]);
        omSetRot(lbl_1_bss_3E80.playerObjects[resultsPlayerIndex], 0.0f, -90.0f, 0.0f);
        Hu3DModelHookReset((s16) characterModelId);
        Hu3DModelHookSet(lbl_1_bss_3E80.playerModelsB[resultsPlayerIndex],
                         lbl_1_data_684[resultsPlayerIndex],
                         lbl_1_bss_3E80.secondaryModels[resultsPlayerIndex]);
        fn_1_5E6C(5);
        playerIndex = 0;
        while (playerIndex < 4) {
            characterNumber = lbl_1_bss_3E80.characterNos[playerIndex] =
                GwPlayerConf[playerIndex].charNo;
            Hu3DModelHookReset(lbl_1_bss_3E80.playerModelsB[playerIndex]);
            Hu3DModelHookSet(lbl_1_bss_3E80.playerModelsB[playerIndex], lbl_1_data_684[playerIndex],
                             lbl_1_bss_3E80.secondaryModels[resultsPlayerIndex]);
            if ((s32) lbl_1_bss_3E80.winners[playerIndex] == 1) {
                Hu3DModelHookSet(lbl_1_bss_3E80.playerModelsC[playerIndex],
                                 lbl_1_data_64C[playerIndex],
                                 lbl_1_bss_3E80.characterModels[playerIndex]);
                if (_CheckFlag(M608_RECORD_PROMPT_FLAG) == 0) {
                    GwPlayer[playerIndex].mgCoinBonus = 10;
                }
            } else {
                Hu3DModelHookSet(lbl_1_bss_3E80.playerModelsB[playerIndex],
                                 lbl_1_data_624[playerIndex],
                                 lbl_1_bss_3E80.characterModels[playerIndex]);
            }
            omSetRot(lbl_1_bss_3E80.playerObjects[playerIndex], 0.0f, -90.0f, 0.0f);
            CharMotionSet((s16) characterNumber, lbl_1_bss_3E80.characterMotions[playerIndex][5]);
            Hu3DModelAttrSet(lbl_1_bss_3E80.characterModels[playerIndex], HU3D_MOTATTR_LOOP);
            playerIndex += 1;
        }
        if ((_CheckFlag(M608_RECORD_PROMPT_FLAG) == 0) &&
            ((s32) lbl_1_bss_3E80.recordBeaten != 0)) {
            recordPromptObject =
                omAddObjEx(lbl_1_bss_3E80.objectManager, 16, 0U, 0U, -1, fn_1_66C0);
            ((float *) recordPromptObject->work)[0] = 1.0f;
            recordPromptObject->work[1] = 1;
            recordPromptObject->work[2] = 0;
        }
        Hu3DModelObjPosGet(lbl_1_bss_3E80.playerModelsB[1], lbl_1_data_1D1, &resultsPosition);
        fn_1_5DA4(resultsPosition);
        WipeCreate(1, 5, 60);
        lbl_1_bss_3E80.frame = 0;
        playerIndex = 0;
        while (playerIndex < 60) {
            HuPrcVSleep();
            playerIndex += 1;
        }
        if (((s32) lbl_1_bss_3E80.recordBeaten == 0) ||
            (_CheckFlag(M608_RECORD_PROMPT_FLAG) != 0)) {
            MgSeqModeNext();
        }
    }
}

/* Gives every winner a celebration motion; starts a camera motion only for a sole winner. */
void fn_1_2374(s16 sequenceMode, s16 frameNo)
{
    int charNo;
    s32 winnerPlayerIndex;
    s32 winnerCount;
    s32 playerIndex;

    /* Assigned in the winner branch; consumed only when its count is one. */
    winnerCount = 0;
    if (frameNo == 0) {
        OSReport(lbl_1_data_1D6);
        OSReport(lbl_1_data_1D6);
        OSReport(lbl_1_data_1D6);
        playerIndex = 0;
        while (playerIndex < 4) {
            charNo = lbl_1_bss_3E80.characterNos[playerIndex] = GwPlayerConf[playerIndex].charNo;
            CharMotionSet(charNo, lbl_1_bss_3E80.characterMotions[playerIndex][7]);
            playerIndex += 1;
        }
        playerIndex = 0;
        while (playerIndex < 4) {
            charNo = lbl_1_bss_3E80.characterNos[playerIndex] = GwPlayerConf[playerIndex].charNo;
            if ((s32) lbl_1_bss_3E80.winners[playerIndex] == 1) {
                CharMotionShiftSet((s16) charNo, lbl_1_bss_3E80.characterMotions[playerIndex][6],
                                   0.0f, 0.5f, 0U);
                winnerCount += 1;
                winnerPlayerIndex = playerIndex;
            } else {
                CharMotionShiftSet((s16) charNo, lbl_1_bss_3E80.characterMotions[playerIndex][7],
                                   0.0f, 0.5f, 0U);
            }
            playerIndex += 1;
        }
        if (winnerCount == 1) {
            fn_1_5E6C(winnerPlayerIndex + 6);
        }
    }
}

/* Empty callback-table entry; it performs no work when the results sequence reaches this slot. */
void fn_1_2538(s16 sequenceMode, s16 frameNo)
{

}

/* Empty callback-table entry; it performs no work when the results sequence reaches this slot. */
void fn_1_253C(s16 sequenceMode, s16 frameNo)
{

}

/* Updates the gameplay camera position and orientation from its motion. */
void fn_1_2540(OMOBJ *cameraObject)
{
    Point3d cameraPosition;
    Point3d cameraTarget;
    Point3d cameraUp;
    Point3d cameraOffset;
    Point3d cameraPositionNew;
    Point3d cameraTargetNew;
    Point3d cameraUpNew;
    Point3d cameraUpBase;
    Point3d cameraForward;
    f32 yawDegrees;
    f32 pitchDegrees;
    f32 rollDegrees;

    lbl_1_bss_3E80.cameraTime = Hu3DMotionTimeGet(lbl_1_bss_3E80.activeCameraMotion);
    Hu3DCameraPosGet(1, &cameraPosition, &cameraUp, &cameraTarget);
    Center = cameraTarget;
    cameraOffset.x = cameraPosition.x - cameraTarget.x;
    cameraOffset.y = cameraPosition.y - cameraTarget.y;
    cameraOffset.z = cameraPosition.z - cameraTarget.z;
    CRot.x =
        (f32) (180.0 * (atan2(-cameraOffset.y, (f64) sqrtf((cameraOffset.x * cameraOffset.x) +
                                                           (cameraOffset.z * cameraOffset.z))) /
                        3.141592653589793));
    CRot.y =
        (f32) (180.0 * (atan2((f64) cameraOffset.x, (f64) cameraOffset.z) / 3.141592653589793));
    CRot.z = 0.0f;
    CZoom = sqrtf((cameraOffset.z * cameraOffset.z) +
                  ((cameraOffset.x * cameraOffset.x) + (cameraOffset.y * cameraOffset.y)));
    pitchDegrees = CRot.x;
    yawDegrees = CRot.y;
    rollDegrees = CRot.z;
    cameraPositionNew.x =
        (f32) ((f64) Center.x +
               ((f64) CZoom * (sin((3.141592653589793 * (f64) yawDegrees) / 180.0) *
                               (cos((3.141592653589793 * (f64) pitchDegrees) / 180.0)))));
    cameraPositionNew.y =
        (f32) ((f64) Center.y +
               ((f64) CZoom * -sin((3.141592653589793 * (f64) pitchDegrees) / 180.0)));
    cameraPositionNew.z =
        (f32) ((f64) Center.z +
               ((f64) CZoom * (cos((3.141592653589793 * (f64) yawDegrees) / 180.0) *
                               (cos((3.141592653589793 * (f64) pitchDegrees) / 180.0)))));
    cameraTargetNew.x = Center.x;
    cameraTargetNew.y = Center.y;
    cameraTargetNew.z = Center.z;
    cameraUpBase.x = (f32) (sin((3.141592653589793 * (f64) yawDegrees) / 180.0) *
                            (sin((3.141592653589793 * (f64) pitchDegrees) / 180.0)));
    cameraUpBase.y = (f32) cos((3.141592653589793 * (f64) pitchDegrees) / 180.0);
    cameraUpBase.z = (f32) (cos((3.141592653589793 * (f64) yawDegrees) / 180.0) *
                            (sin((3.141592653589793 * (f64) pitchDegrees) / 180.0)));
    PSVECSubtract(&cameraPositionNew, &cameraTargetNew, &cameraForward);
    PSVECNormalize(&cameraForward, &cameraForward);
    cameraUpNew.x =
        (f32) (((f64) cameraUpBase.x * ((f64) (cameraForward.x * cameraForward.x) +
                                        ((f64) (1.0f - (cameraForward.x * cameraForward.x)) *
                                         cos((3.141592653589793 * (f64) rollDegrees) / 180.0)))) +
               ((f64) cameraUpBase.y *
                (((f64) (cameraForward.x * cameraForward.y) *
                  (1.0 - cos((3.141592653589793 * (f64) rollDegrees) / 180.0))) -
                 ((f64) cameraForward.z * sin((3.141592653589793 * (f64) rollDegrees) / 180.0)))) +
               ((f64) cameraUpBase.z *
                (((f64) (cameraForward.x * cameraForward.z) *
                  (1.0 - cos((3.141592653589793 * (f64) rollDegrees) / 180.0))) +
                 ((f64) cameraForward.y * sin((3.141592653589793 * (f64) rollDegrees) / 180.0)))));
    cameraUpNew.y =
        (f32) (((f64) cameraUpBase.y * ((f64) (cameraForward.y * cameraForward.y) +
                                        ((f64) (1.0f - (cameraForward.y * cameraForward.y)) *
                                         cos((3.141592653589793 * (f64) rollDegrees) / 180.0)))) +
               ((f64) cameraUpBase.x *
                (((f64) (cameraForward.x * cameraForward.y) *
                  (1.0 - cos((3.141592653589793 * (f64) rollDegrees) / 180.0))) +
                 ((f64) cameraForward.z * sin((3.141592653589793 * (f64) rollDegrees) / 180.0)))) +
               ((f64) cameraUpBase.z *
                (((f64) (cameraForward.y * cameraForward.z) *
                  (1.0 - cos((3.141592653589793 * (f64) rollDegrees) / 180.0))) -
                 ((f64) cameraForward.x * sin((3.141592653589793 * (f64) rollDegrees) / 180.0)))));
    cameraUpNew.z =
        (f32) (((f64) cameraUpBase.z * ((f64) (cameraForward.z * cameraForward.z) +
                                        ((f64) (1.0f - (cameraForward.z * cameraForward.z)) *
                                         cos((3.141592653589793 * (f64) rollDegrees) / 180.0)))) +
               (((f64) cameraUpBase.x *
                 (((f64) (cameraForward.x * cameraForward.z) *
                   (1.0 - cos((3.141592653589793 * (f64) rollDegrees) / 180.0))) -
                  ((f64) cameraForward.y * sin((3.141592653589793 * (f64) rollDegrees) / 180.0)))) +
                ((f64) cameraUpBase.y *
                 (((f64) (cameraForward.y * cameraForward.z) *
                   (1.0 - cos((3.141592653589793 * (f64) rollDegrees) / 180.0))) +
                  ((f64) cameraForward.x *
                   sin((3.141592653589793 * (f64) rollDegrees) / 180.0))))));
    PSVECNormalize(&cameraUpNew, &cameraUpNew);
    Hu3DCameraPosSet(1, cameraPositionNew.x, cameraPositionNew.y, cameraPositionNew.z,
                     cameraUpNew.x, cameraUpNew.y, cameraUpNew.z, cameraTargetNew.x,
                     cameraTargetNew.y, cameraTargetNew.z);
}

/* Advances timed rider animations when the course motion timing hook fires. */
void fn_1_2FD8(s16 timingModelId, s16 motionId, BOOL timingCallbackF)
{
    s32 timingCharacterModelId;
    f32 normalizedAngle;
    s16 timingCharacterNumber;
    s32 timingPlayerIndex;

    timingPlayerIndex = lbl_1_bss_3E80.activePlayer;
    timingCharacterModelId = (s32) lbl_1_bss_3E80.characterModels[timingPlayerIndex];
    timingCharacterNumber = lbl_1_bss_3E80.characterNos[timingPlayerIndex];
    if (timingCallbackF == 1) {
        switch ((s32) lbl_1_bss_3E80.timingSubstate) { /* irregular */
        case 0:
            (lbl_1_bss_3E80.playerObjects[timingPlayerIndex])->objFunc = fn_1_33A4;
            CharMotionShiftSet((s16) timingCharacterNumber,
                               lbl_1_bss_3E80.characterMotions[timingPlayerIndex][3], 0.0f, 8.0f,
                               HU3D_MOTATTR_LOOP);
            Hu3DMotionCalc(lbl_1_bss_3E80.characterModels[timingPlayerIndex]);
            fn_1_605C(0);
            OSReport(lbl_1_data_217, lbl_1_bss_3E80.frame);
            lbl_1_bss_3E80.state = 6;
            lbl_1_bss_3E80.frame = 0;
            HuAudFXPlay(M608_CONTROL_START_FX);
            HuAudFXStop(lbl_1_bss_3E80.jumpSoundHandle);
            lbl_1_bss_3E80.timingSubstate++;
            return;
        case 1:
            CharMotionShiftSet((s16) timingCharacterNumber,
                               lbl_1_bss_3E80.characterMotions[timingPlayerIndex][0], 0.0f, 5.0f,
                               HU3D_MOTATTR_LOOP);
            lbl_1_bss_3E80.timingSubstate++;
            return;
        case 2:
            CharMotionShiftSet((s16) timingCharacterNumber,
                               lbl_1_bss_3E80.characterMotions[timingPlayerIndex][4], 0.0f, 8.0f,
                               0U);
            CharMotionSpeedSet((s16) timingCharacterNumber, 1.0f);
            Hu3DAnimSpeedSet(lbl_1_bss_3E80.characterModels[timingPlayerIndex], 1.0f);
            Hu3DModelAttrReset(lbl_1_bss_3E80.characterModels[timingPlayerIndex],
                               HU3D_MOTATTR_LOOP);
            Hu3DMotionCalc(lbl_1_bss_3E80.characterModels[timingPlayerIndex]);
            (lbl_1_bss_3E80.playerObjects[timingPlayerIndex])->objFunc = fn_1_34D0;
            lbl_1_bss_3E80.state = 7;
            lbl_1_bss_3E80.frame = 0;
            normalizedAngle = lbl_1_bss_3E80.riderTurnAngle;
            while (normalizedAngle < -360.0f) {
                normalizedAngle += 360.0f;
            }
            if ((normalizedAngle == 0.0f) || (normalizedAngle == 360.0f)) {
                lbl_1_bss_3E80.riderTurnStartAngle = 0.0f;
                lbl_1_bss_3E80.riderTurnTargetAngle = 0.0f;
            } else {
                lbl_1_bss_3E80.riderTurnStartAngle = normalizedAngle;
                lbl_1_bss_3E80.riderTurnTargetAngle = -360.0f;
            }
            lbl_1_bss_3E80.riderTurnAngle = normalizedAngle;
            lbl_1_bss_3E80.riderTurnFrame = 0;
            fn_1_605C(1);
            fn_1_605C(7);
            lbl_1_bss_3E80.timingSubstate++;
            break;
        }
    }
}

/* Rotates the selected rider through the first five-frame turn. */
void fn_1_33A4(OMOBJ *playerObject)
{
    lbl_1_bss_3E80.riderTurnAngle =
        (f32) (lbl_1_bss_3E80.riderTurnStartAngle +
               ((f32) lbl_1_bss_3E80.riderTurnFrame *
                ((lbl_1_bss_3E80.riderTurnTargetAngle - lbl_1_bss_3E80.riderTurnStartAngle) /
                 5.0f)));
    if ((s32) lbl_1_bss_3E80.riderTurnFrame < 5) {
        lbl_1_bss_3E80.riderTurnFrame += 1.0f;
    }
    omSetRot(playerObject, 0.0f, lbl_1_bss_3E80.riderTurnAngle, 0.0f);
}

/* Finishes the rider's turn and restores its idle motion at animation end. */
void fn_1_34D0(OMOBJ *playerObject)
{
    lbl_1_bss_3E80.riderTurnAngle =
        (f32) (lbl_1_bss_3E80.riderTurnStartAngle +
               ((f32) lbl_1_bss_3E80.riderTurnFrame *
                ((lbl_1_bss_3E80.riderTurnTargetAngle - lbl_1_bss_3E80.riderTurnStartAngle) /
                 10.0f)));
    if ((s32) lbl_1_bss_3E80.riderTurnFrame < 10) {
        lbl_1_bss_3E80.riderTurnFrame += 1.0f;
    }
    CharMotionSpeedSet(lbl_1_bss_3E80.characterNos[lbl_1_bss_3E80.activePlayer], 0.5f);
    Hu3DAnimSpeedSet(lbl_1_bss_3E80.characterModels[lbl_1_bss_3E80.activePlayer], 1.0f);
    omSetRot(playerObject, 0.0f, lbl_1_bss_3E80.riderTurnAngle, 0.0f);
    if (Hu3DMotionEndCheck(lbl_1_bss_3E80.characterModels[lbl_1_bss_3E80.activePlayer]) == 1) {
        s16 charNo = lbl_1_bss_3E80.characterNos[lbl_1_bss_3E80.activePlayer];
        CharMotionShiftSet((s16) charNo,
                           lbl_1_bss_3E80.characterMotions[lbl_1_bss_3E80.activePlayer][0], 0.0f,
                           5.0f, HU3D_MOTATTR_LOOP);
        OSReport(lbl_1_data_227);
        playerObject->objFunc = NULL;
    }
}

/* Places course props at recorded path points as the scene advances. */
void fn_1_3718(OMOBJ *courseObject)
{
    float positionX, positionY, positionZ;
    u32 courseObjectIndex;

    courseObjectIndex = courseObject->work[0];
    if ((s32) lbl_1_bss_3E80.coursePathSampleCount > (s32) courseObjectIndex) {
        if ((u32) (Hu3DModelAttrGet(lbl_1_bss_3E80.movingCourseModels[courseObjectIndex]) &
                   HU3D_ATTR_DISPOFF) != 0) {
            Hu3DModelAttrReset(lbl_1_bss_3E80.movingCourseModels[courseObjectIndex],
                               HU3D_ATTR_DISPOFF);
            Hu3DMotionTimeSet(lbl_1_bss_3E80.movingCourseModels[courseObjectIndex], 0.0f);
        }
        if ((u32) courseObject->work[1] != 0) {
            positionX = lbl_1_bss_10.points[lbl_1_bss_C - 1].x;
            positionY = lbl_1_bss_10.points[lbl_1_bss_C - 1].y;
            positionZ = lbl_1_bss_10.points[lbl_1_bss_C - 1].z;
            omSetTra(courseObject, positionX, positionY, positionZ);
            courseObject->work[1] -= 1;
        }
    }
}

/* Binds each course object to its model and motion before its updates. */
void fn_1_388C(OMOBJ *courseObject)
{
    u32 courseDataIndex;

    courseDataIndex = courseObject->work[0];
    *courseObject->mdlId = lbl_1_bss_3E80.movingCourseModels[courseDataIndex];
    *courseObject->mtnId = lbl_1_bss_3E80.movingCourseMotions[courseDataIndex];
    courseObject->objFunc = fn_1_3718;
}

/* Camera hook created when a rider starts jumping; it leaves the supplied matrix unchanged. */
void fn_1_38E8(HU3D_MODEL *cameraModel, f32 (*cameraMatrix)[3][4])
{

}

/* Per-frame course-hook callback; this entry currently leaves the supplied position unused. */
void fn_1_38EC(Point3d position)
{

}

/* Applies player one's course-facing rotation to its character transform. */
void fn_1_38F0(HSF_OBJECT *hookObject, HSF_TRANSFORM *transform, f32 (*previousMatrix)[3][4],
               f32 (*currentMatrix)[3][4])
{
    Mtx localTransform;

    PSMTXIdentity(localTransform);
    PSMTXScale(localTransform, transform->scale.x, transform->scale.y, transform->scale.z);
    mtxRotCat(localTransform, transform->rot.x, transform->rot.y, transform->rot.z);
    mtxRotCat(localTransform, 0.0f, lbl_1_data_28[lbl_1_bss_3E80.characterNos[0]], 0.0f);
    mtxTransCat(localTransform, transform->pos.x, transform->pos.y, transform->pos.z);
    PSMTXConcat(*previousMatrix, localTransform, *currentMatrix);
}

/* Applies player two's course-facing rotation to its character transform. */
void fn_1_39B4(HSF_OBJECT *hookObject, HSF_TRANSFORM *transform, f32 (*previousMatrix)[3][4],
               f32 (*currentMatrix)[3][4])
{
    Mtx localTransform;

    PSMTXIdentity(localTransform);
    PSMTXScale(localTransform, transform->scale.x, transform->scale.y, transform->scale.z);
    mtxRotCat(localTransform, transform->rot.x, transform->rot.y, transform->rot.z);
    mtxRotCat(localTransform, 0.0f, lbl_1_data_28[lbl_1_bss_3E80.characterNos[1]], 0.0f);
    mtxTransCat(localTransform, transform->pos.x, transform->pos.y, transform->pos.z);
    PSMTXConcat(*previousMatrix, localTransform, *currentMatrix);
}

/* Applies player three's course-facing rotation to its character transform. */
void fn_1_3A78(HSF_OBJECT *hookObject, HSF_TRANSFORM *transform, f32 (*previousMatrix)[3][4],
               f32 (*currentMatrix)[3][4])
{
    Mtx localTransform;

    PSMTXIdentity(localTransform);
    PSMTXScale(localTransform, transform->scale.x, transform->scale.y, transform->scale.z);
    mtxRotCat(localTransform, transform->rot.x, transform->rot.y, transform->rot.z);
    mtxRotCat(localTransform, 0.0f, lbl_1_data_28[lbl_1_bss_3E80.characterNos[2]], 0.0f);
    mtxTransCat(localTransform, transform->pos.x, transform->pos.y, transform->pos.z);
    PSMTXConcat(*previousMatrix, localTransform, *currentMatrix);
}

/* Applies player four's course-facing rotation to its character transform. */
void fn_1_3B3C(HSF_OBJECT *hookObject, HSF_TRANSFORM *transform, f32 (*previousMatrix)[3][4],
               f32 (*currentMatrix)[3][4])
{
    Mtx localTransform;

    PSMTXIdentity(localTransform);
    PSMTXScale(localTransform, transform->scale.x, transform->scale.y, transform->scale.z);
    mtxRotCat(localTransform, transform->rot.x, transform->rot.y, transform->rot.z);
    mtxRotCat(localTransform, 0.0f, lbl_1_data_28[lbl_1_bss_3E80.characterNos[3]], 0.0f);
    mtxTransCat(localTransform, transform->pos.x, transform->pos.y, transform->pos.z);
    PSMTXConcat(*previousMatrix, localTransform, *currentMatrix);
}

/* Builds the rider transform without adding the character-specific yaw. */
void fn_1_3C00(HSF_OBJECT *hookObject, HSF_TRANSFORM *transform, f32 (*previousMatrix)[3][4],
               f32 (*currentMatrix)[3][4])
{
    Mtx localTransform;

    PSMTXIdentity(localTransform);
    PSMTXScale(localTransform, transform->scale.x, transform->scale.y, transform->scale.z);
    mtxRotCat(localTransform, transform->rot.x, transform->rot.y, transform->rot.z);
    mtxTransCat(localTransform, transform->pos.x, transform->pos.y, transform->pos.z);
    PSMTXConcat(*previousMatrix, localTransform, *currentMatrix);
}

/* Adds the current course turn to player one's character transform. */
void fn_1_3C84(HSF_OBJECT *hookObject, HSF_TRANSFORM *transform, f32 (*previousMatrix)[3][4],
               f32 (*currentMatrix)[3][4])
{
    Mtx localTransform;

    PSMTXIdentity(localTransform);
    PSMTXScale(localTransform, transform->scale.x, transform->scale.y, transform->scale.z);
    mtxRotCat(localTransform, transform->rot.x, transform->rot.y, transform->rot.z);
    mtxRotCat(localTransform, 0.0f,
              lbl_1_bss_4 + (lbl_1_data_60[lbl_1_bss_3E80.characterNos[0]] +
                             lbl_1_data_28[lbl_1_bss_3E80.characterNos[0]]),
              0.0f);
    mtxTransCat(localTransform, transform->pos.x, transform->pos.y, transform->pos.z);
    PSMTXConcat(*previousMatrix, localTransform, *currentMatrix);
}

/* Builds an unmodified secondary-model transform for the active rider. */
void fn_1_3D7C(HSF_OBJECT *hookObject, HSF_TRANSFORM *transform, f32 (*previousMatrix)[3][4],
               f32 (*currentMatrix)[3][4])
{
    Mtx localTransform;

    PSMTXIdentity(localTransform);
    PSMTXScale(localTransform, transform->scale.x, transform->scale.y, transform->scale.z);
    mtxRotCat(localTransform, transform->rot.x, transform->rot.y, transform->rot.z);
    mtxTransCat(localTransform, transform->pos.x, transform->pos.y, transform->pos.z);
    PSMTXConcat(*previousMatrix, localTransform, *currentMatrix);
}

/* Counter-rotates player one's secondary model for its special character. */
void fn_1_3E00(HSF_OBJECT *hookObject, HSF_TRANSFORM *transform, f32 (*previousMatrix)[3][4],
               f32 (*currentMatrix)[3][4])
{
    Mtx localTransform;
    int hookCharacterNumber;

    hookCharacterNumber = lbl_1_bss_3E80.characterNos[0];
    PSMTXIdentity(localTransform);
    PSMTXScale(localTransform, transform->scale.x, transform->scale.y, transform->scale.z);
    mtxRotCat(localTransform, transform->rot.x, transform->rot.y, transform->rot.z);
    if (hookCharacterNumber == 8) {
        mtxRotCat(localTransform, 0.0f,
                  -(lbl_1_bss_4 + (lbl_1_data_60[lbl_1_bss_3E80.characterNos[0]] +
                                   lbl_1_data_28[lbl_1_bss_3E80.characterNos[0]])),
                  0.0f);
    }
    mtxTransCat(localTransform, transform->pos.x, transform->pos.y, transform->pos.z);
    PSMTXConcat(*previousMatrix, localTransform, *currentMatrix);
}

/* For character number 8, adds the course-turn correction to player one's board transform. */
void fn_1_3F18(HSF_OBJECT *hookObject, HSF_TRANSFORM *transform, f32 (*previousMatrix)[3][4],
               f32 (*currentMatrix)[3][4])
{
    Mtx localTransform;
    int hookCharacterNumber;

    hookCharacterNumber = lbl_1_bss_3E80.characterNos[0];
    PSMTXIdentity(localTransform);
    PSMTXScale(localTransform, transform->scale.x, transform->scale.y, transform->scale.z);
    mtxRotCat(localTransform, transform->rot.x, transform->rot.y, transform->rot.z);
    if (hookCharacterNumber == 8) {
        mtxRotCat(localTransform, 0.0f,
                  lbl_1_bss_0 + -lbl_1_data_28[lbl_1_bss_3E80.characterNos[0]], 0.0f);
    }
    mtxTransCat(localTransform, transform->pos.x, transform->pos.y, transform->pos.z);
    PSMTXConcat(*previousMatrix, localTransform, *currentMatrix);
}

/* For character number 8, counter-rotates player two's attached board. */
void fn_1_400C(HSF_OBJECT *hookObject, HSF_TRANSFORM *transform, f32 (*previousMatrix)[3][4],
               f32 (*currentMatrix)[3][4])
{
    Mtx localTransform;
    int hookCharacterNumber;

    hookCharacterNumber = lbl_1_bss_3E80.characterNos[1];
    PSMTXIdentity(localTransform);
    PSMTXScale(localTransform, transform->scale.x, transform->scale.y, transform->scale.z);
    mtxRotCat(localTransform, transform->rot.x, transform->rot.y, transform->rot.z);
    if (hookCharacterNumber == 8) {
        mtxRotCat(localTransform, 0.0f, -lbl_1_data_28[lbl_1_bss_3E80.characterNos[1]], 0.0f);
    }
    mtxTransCat(localTransform, transform->pos.x, transform->pos.y, transform->pos.z);
    PSMTXConcat(*previousMatrix, localTransform, *currentMatrix);
}

/* For character number 8, counter-rotates player three's attached board. */
void fn_1_40F0(HSF_OBJECT *hookObject, HSF_TRANSFORM *transform, f32 (*previousMatrix)[3][4],
               f32 (*currentMatrix)[3][4])
{
    Mtx localTransform;
    int hookCharacterNumber;

    hookCharacterNumber = lbl_1_bss_3E80.characterNos[2];
    PSMTXIdentity(localTransform);
    PSMTXScale(localTransform, transform->scale.x, transform->scale.y, transform->scale.z);
    mtxRotCat(localTransform, transform->rot.x, transform->rot.y, transform->rot.z);
    if (hookCharacterNumber == 8) {
        mtxRotCat(localTransform, 0.0f, -lbl_1_data_28[lbl_1_bss_3E80.characterNos[2]], 0.0f);
    }
    mtxTransCat(localTransform, transform->pos.x, transform->pos.y, transform->pos.z);
    PSMTXConcat(*previousMatrix, localTransform, *currentMatrix);
}

/* For character number 8, counter-rotates player four's attached board. */
void fn_1_41D4(HSF_OBJECT *hookObject, HSF_TRANSFORM *transform, f32 (*previousMatrix)[3][4],
               f32 (*currentMatrix)[3][4])
{
    Mtx localTransform;
    int hookCharacterNumber;

    hookCharacterNumber = lbl_1_bss_3E80.characterNos[3];
    PSMTXIdentity(localTransform);
    PSMTXScale(localTransform, transform->scale.x, transform->scale.y, transform->scale.z);
    mtxRotCat(localTransform, transform->rot.x, transform->rot.y, transform->rot.z);
    if (hookCharacterNumber == 8) {
        mtxRotCat(localTransform, 0.0f, -lbl_1_data_28[lbl_1_bss_3E80.characterNos[3]], 0.0f);
    }
    mtxTransCat(localTransform, transform->pos.x, transform->pos.y, transform->pos.z);
    PSMTXConcat(*previousMatrix, localTransform, *currentMatrix);
}

/* Stores indices 0 through 658 (659 samples); each call after the limit logs the limit. */
void fn_1_42B8(void)
{
    Point3d coursePoint;

    if ((s32) (lbl_1_bss_C + 1) < 660) {
        Hu3DMotionCalc(lbl_1_bss_3E80.mainModel);
        Hu3DModelObjPosGet(lbl_1_bss_3E80.mainModel, lbl_1_data_235, &coursePoint);
        lbl_1_bss_10.points[lbl_1_bss_C] = coursePoint;
        lbl_1_bss_C += 1;
        return;
    }
    OSReport(lbl_1_data_242);
}

/* Copies the main-model hook matrix to model 0; layerNo is ignored. */
void fn_1_437C(s16 layerNo)
{
    Mtx hookMatrix;

    Hu3DModelObjMtxGet(lbl_1_bss_3E80.mainModel, lbl_1_data_24E, hookMatrix);
    Hu3DModelMtxSet(lbl_1_bss_3E80.courseModels[0], &hookMatrix);
}

char lbl_1_data_1B4[29] = "*** new record ... %d -> %d\012";

char lbl_1_data_1D1[5] = "p2ls";

char lbl_1_data_1D6[65] = "###############################################################\012";

char lbl_1_data_217[16] = "now timing(%d)\012";

char lbl_1_data_227[14] = "*** loop ***\012";

char lbl_1_data_235[13] = "pmov_ptEFatt";

char lbl_1_data_242[12] = "*******max\012";

char lbl_1_data_24E[18] = "snowBDEFhook\000\000\000\000\000";
