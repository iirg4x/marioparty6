/* The quiz code the five microphone quiz modules share: dialogue windows, the microphone HUD
 * and its feedback, the answer countdown, the response flow and the pause display.
 *
 * This header holds definitions, not only declarations: its functions and data are compiled
 * into the file that includes it. Exactly one file of each module includes it, the module's
 * main source (quiz.c in micquizdll and micquizseldll, main.c in micquizishidll,
 * micquizmyokodll and micquizohdedll), after that file's own includes and before its code. */

#include "datadir_enum.h"
#include "messdir_enum.h"
#include "msm_se.h"
#include "msm_stream.h"
#include "humath.h"
#include "REL/micquiz/quiz.h"

/* Not in quiz.h: some of the modules' files call these without a prototype in scope (their calls
 * take an int result from a function that returns nothing). */
void MicQuizPauseInit(HUPROCESS *parentProcess);
void MicQuizPortraitShow(s32 playerIndex);
void MicQuizMicContextSet(s32 questionType);
void MicQuizCountdownInit(void);
void MicQuizPresentationDone(void);
void MicQuizPresenterCreate(void);
void MicQuizExpectedAnswerSet(s32 expectedAnswer);
void MicQuizListenStart(void);
void MicQuizQuestionReadySet(void);
void MicQuizMicInit(char *path, s32 questionType);
void MicQuizWinMesSet(u32 messageId, s16 messageSpeed, u8 windowIndex);
void MicQuizWinMesWait(void);
void MicQuizWinOpen(s32 windowNo);

/* These bytes are not read by the quiz modules; their purpose is unknown. */
u8 micQuizUnreadA[10];
s16 micQuizMicHudGroup;
MicquizChoiceRecord micQuizChoiceRecords[8];
HUPROCESS *micQuizParentProc;
s32 micQuizResponseMode;
s32 micQuizListenReady;
s32 micQuizAnswerPlayer;
s32 micQuizRetryPrompted;
s32 micQuizFirstAttempt;
s32 micQuizResponseResult;
s32 micQuizAnswerAccepted;
s32 micQuizResponseReading;
s32 micQuizCountdownState;
s32 micQuizCountdownValue;
s32 micQuizCountdownStepFrames;
s32 micQuizCountdownFrame;
s32 micQuizExpectedAnswer;
s32 micQuizRecognizedAnswer;
s16 micQuizMicContext;
s32 micQuizResponseState;
s16 micQuizSelectionItem;
s32 micQuizAnswerPageOffset;
s32 micQuizQuestionReady;
s32 micQuizRecognitionType;
ANIMDATA *micQuizFlashAnim;
/* These bytes are not read by the quiz modules; their purpose is unknown. */
u8 micQuizUnreadB[6];
s16 micQuizFlashGroup;
s32 micQuizStreamNo;
f32 micQuizBannerX;
f32 micQuizBannerY;
s32 micQuizBannerShown;
MicquizModeWork *micQuizModeWork;
ANIMDATA *micQuizEventIconAnim;
ANIMDATA *micQuizEventFragmentAnim[2];
ANIMDATA *micQuizEventPulseAnim;
ANIMDATA *micQuizEventBadgeAnim;
ANIMDATA *micQuizEventBannerAnim;
ANIMDATA *micQuizTwinkleAnim;
ANIMDATA *micQuizPulseLeftAnim;
ANIMDATA *micQuizPulseRightAnim;
ANIMDATA *micQuizEventSparkAnim;
ANIMDATA *micQuizEventOverlayAnim;
s16 micQuizEventOverlayGroup;
ANIMDATA *micQuizIndicatorAnim;
ANIMDATA *micQuizIndicatorSwingAnim;
ANIMDATA *micQuizIndicatorDoneAnim;
ANIMDATA *micQuizCategoryAnim;
ANIMDATA *micQuizBannerAnim;
ANIMDATA *micQuizMicIconAnim;
ANIMDATA *micQuizMicAnim;
ANIMDATA *micQuizMicGlowAnim;
ANIMDATA *micQuizCountdownDigitAnim[2];
s16 micQuizEventIconGroup;
/* These bytes are not read by the quiz modules; their purpose is unknown. */
u8 micQuizUnreadC[8];
s16 micQuizIndicatorGroup;
s16 micQuizIndicatorSwingGroup;
s16 micQuizIndicatorDoneGroup;
s16 micQuizCategoryGroup;
s16 micQuizBannerGroup;
s16 micQuizMicIconGroup;
s16 micQuizMicGroup;
s16 micQuizMicGlowGroup;
HUSPR_GROUPID micQuizCountdownGroup[3];
ANIMDATA *micQuizPortraitAnim[4];
ANIMDATA *micQuizMicHudBaseAnim;
ANIMDATA *micQuizMicHudMicAnim;
ANIMDATA *micQuizMicHudIdleAnim;
ANIMDATA *micQuizMicHudMarkerAnim;
s32 micQuizPauseCameraBit;
ANIMDATA *micQuizPauseTitleAnim;
s16 micQuizPauseTitleGroup;
ANIMDATA *micQuizPauseRowAnim[4];
s16 micQuizPauseRowGroup[4];
ANIMDATA *micQuizScoreDigitAnim[4][3];
s16 micQuizScoreDigitGroup[4][3];
ANIMDATA *micQuizPauseProgressAnim;
s16 micQuizPauseProgressGroup;
ANIMDATA *micQuizPausePortraitAnim[4];
s16 micQuizPausePortraitGroup[4];
MGSCORE *micQuizPauseProgressScore[2];
s16 micQuizPauseWin0;
s16 micQuizPauseWin1;
HUPROCESS *micQuizPauseProc;
s32 micQuizPauseOverlayFlag;
s32 micQuizSavedPauseEnable;
ANIMDATA *micQuizAcceptIconAnim;
ANIMDATA *micQuizRejectIconAnim;
s16 micQuizAcceptIconGroup;
s16 micQuizRejectIconGroup;
s16 micQuizGameMes;
/* These bytes are not read by the quiz modules; their purpose is unknown. */
u8 micQuizUnreadD[4];
s16 micQuizDialogWin0;
s16 micQuizDialogWin1;
s16 micQuizDialogWin2;
s16 micQuizDialogWin3;
s16 micQuizInstructionWin;
s16 micQuizHintWin;
s16 micQuizActiveWin;
s16 micQuizFeedbackGroup;
HU3D_MODELID micQuizPresenterModel;
HU3D_MOTIONID micQuizPresenterMotion[7];
HUPROCESS *micQuizEffectProc[10];

/* Model and character resource lists used while constructing the quiz scene. */
s32 micQuizPresenterMotionFiles[7] = {
    DATANUM(0xa40000, 122), DATANUM(0xa40000, 123),
    DATANUM(0xa40000, 124), DATANUM(0xa40000, 125),
    DATANUM(0xa40000, 126), DATANUM(0xa40000, 127),
    DATANUM(0xa40000, 128)
};
s32 micQuizPortraitFiles[11] = {
    DATANUM(0x9b0000, 0), DATANUM(0x9b0000, 1),
    DATANUM(0x9b0000, 2), DATANUM(0x9b0000, 3),
    DATANUM(0x9b0000, 4), DATANUM(0x9b0000, 5),
    DATANUM(0x9b0000, 6), DATANUM(0x9b0000, 7),
    DATANUM(0x9b0000, 8), DATANUM(0x9b0000, 9),
    DATANUM(0x9b0000, 10)
};
s32 micQuizCountdownStarts[5] = { 10, 20, 30, 40, 50 };
/* Character-name and microphone-choice message lists. */
u32 micQuizNpcAnswerMes[17] = {
    MESSNUM(MESS_QUIZ_CHARA, 12), MESSNUM(MESS_QUIZ_CHARA, 13),
    MESSNUM(MESS_QUIZ_CHARA, 14), MESSNUM(MESS_QUIZ_CHARA, 15),
    MESSNUM(MESS_QUIZ_CHARA, 16), MESSNUM(MESS_QUIZ_CHARA, 17),
    0, 1,
    2, 3,
    4, 5,
    6, 7,
    9, 10,
    MESSNUM(MESS_MIC_RETYPING, 22)
};
u32 micQuizCharaNameMes[18] = {
    0, 1,
    2, 3,
    4, 5,
    6, 7,
    8, 9,
    10, MESSNUM(MESS_QUIZ_CHARA, 12),
    MESSNUM(MESS_QUIZ_CHARA, 13), MESSNUM(MESS_QUIZ_CHARA, 14),
    MESSNUM(MESS_QUIZ_CHARA, 15), MESSNUM(MESS_QUIZ_CHARA, 16),
    MESSNUM(MESS_QUIZ_CHARA, 17), MESSNUM(MESS_MIC_RETYPING, 22)
};
u32 micQuizCharaAnswerMes[17] = {
    0, 1,
    2, 3,
    4, 5,
    6, 7,
    9, 10,
    MESSNUM(MESS_QUIZ_CHARA, 12), MESSNUM(MESS_QUIZ_CHARA, 13),
    MESSNUM(MESS_QUIZ_CHARA, 14), MESSNUM(MESS_QUIZ_CHARA, 15),
    MESSNUM(MESS_QUIZ_CHARA, 16), MESSNUM(MESS_QUIZ_CHARA, 17),
    MESSNUM(MESS_MIC_RETYPING, 22)
};
u32 micQuizNumberAnswerMes[] = {
    MESSNUM(MESS_QUIZ_CHARA, 34), MESSNUM(MESS_QUIZ_CHARA, 35),
    MESSNUM(MESS_QUIZ_CHARA, 36), MESSNUM(MESS_QUIZ_CHARA, 37),
    MESSNUM(MESS_QUIZ_CHARA, 38), MESSNUM(MESS_QUIZ_CHARA, 39),
    MESSNUM(MESS_QUIZ_CHARA, 40), MESSNUM(MESS_QUIZ_CHARA, 41),
    MESSNUM(MESS_QUIZ_CHARA, 42), MESSNUM(MESS_QUIZ_CHARA, 43),
    MESSNUM(MESS_QUIZ_CHARA, 44), MESSNUM(MESS_QUIZ_CHARA, 45),
    MESSNUM(MESS_QUIZ_CHARA, 46), MESSNUM(MESS_QUIZ_CHARA, 47),
    MESSNUM(MESS_QUIZ_CHARA, 48), MESSNUM(MESS_QUIZ_CHARA, 49),
    MESSNUM(MESS_QUIZ_CHARA, 50), MESSNUM(MESS_QUIZ_CHARA, 51),
    MESSNUM(MESS_QUIZ_CHARA, 52), MESSNUM(MESS_QUIZ_CHARA, 53)
};
u32 micQuizNumberAnswerAltMes[] = {
    MESSNUM(MESS_QUIZ_CHARA, 58), MESSNUM(MESS_QUIZ_CHARA, 59),
    MESSNUM(MESS_QUIZ_CHARA, 60), MESSNUM(MESS_QUIZ_CHARA, 61),
    MESSNUM(MESS_QUIZ_CHARA, 62), MESSNUM(MESS_QUIZ_CHARA, 63),
    MESSNUM(MESS_QUIZ_CHARA, 64), MESSNUM(MESS_QUIZ_CHARA, 65),
    MESSNUM(MESS_QUIZ_CHARA, 66), MESSNUM(MESS_QUIZ_CHARA, 67),
    MESSNUM(MESS_QUIZ_CHARA, 68), MESSNUM(MESS_QUIZ_CHARA, 69),
    MESSNUM(MESS_QUIZ_CHARA, 70), MESSNUM(MESS_QUIZ_CHARA, 71),
    MESSNUM(MESS_QUIZ_CHARA, 72), MESSNUM(MESS_QUIZ_CHARA, 73),
    MESSNUM(MESS_QUIZ_CHARA, 74), MESSNUM(MESS_QUIZ_CHARA, 75),
    MESSNUM(MESS_QUIZ_CHARA, 76), MESSNUM(MESS_QUIZ_CHARA, 77)
};
u32 micQuizCompareAnswerMes[2] = { MESSNUM(MESS_QUIZ_CHARA, 55), MESSNUM(MESS_QUIZ_CHARA, 56) };
u32 micQuizFruitAnswerMes[8] = {
    MESSNUM(MESS_QUIZ_CHARA, 25), MESSNUM(MESS_QUIZ_CHARA, 26),
    MESSNUM(MESS_QUIZ_CHARA, 27), MESSNUM(MESS_QUIZ_CHARA, 28),
    MESSNUM(MESS_QUIZ_CHARA, 29), MESSNUM(MESS_QUIZ_CHARA, 30),
    MESSNUM(MESS_QUIZ_CHARA, 31), MESSNUM(MESS_QUIZ_CHARA, 32)
};
s32 micQuizInstructionState = -1;
s32 micQuizMicHudState = -1;
s32 micQuizMicHudMotion[3] = { -1, -1, -1 };
s16 micQuizTempMesWin = -1;
s16 micQuizEventPulseGroup = -1;
s16 micQuizEventBadgeGroup = -1;
s16 micQuizEventBannerGroup = -1;
s16 micQuizTwinkleGroup[4] = { -1, -1, -1, -1 };
s16 micQuizPulseLeftGroup = -1;
s16 micQuizPulseRightGroup = -1;
struct MicquizScreenPosition micQuizTwinklePos[4] = {
    222.0f, 40.0f, 357.0f, 55.0f, 227.0f, 74.0f, 345.0f, 80.0f
};
u32 micQuizSelectionMes[16] = {
    MESSNUM(MESS_MICQUIZ_I, 0), MESSNUM(MESS_MICQUIZ_I, 1),
    MESSNUM(MESS_MICQUIZ_I, 2), MESSNUM(MESS_MICQUIZ_I, 3),
    MESSNUM(MESS_MICQUIZ_I, 5), MESSNUM(MESS_MICQUIZ_I, 6),
    MESSNUM(MESS_MICQUIZ_I, 7), MESSNUM(MESS_MICQUIZ_I, 8),
    MESSNUM(MESS_MICQUIZ_I, 11), MESSNUM(MESS_MICQUIZ_I, 12),
    MESSNUM(MESS_MICQUIZ_I, 13), MESSNUM(MESS_MICQUIZ_I, 14),
    MESSNUM(MESS_MICQUIZ_I, 15), MESSNUM(MESS_MICQUIZ_I, 16),
    MESSNUM(MESS_MICQUIZ_I, 17), MESSNUM(MESS_MICQUIZ_I, 18)
};

/* Store the answer that the current microphone response must equal. */
void MicQuizExpectedAnswerSet(s32 expectedAnswer)
{
    micQuizExpectedAnswer = expectedAnswer;
}

/* Mark the current question as ready for its response process. */
void MicQuizQuestionReadySet(void) {
    micQuizQuestionReady = 1;
}

/* Switch to the listening display and signal that the response can advance. */
void MicQuizListenStart(void) {
    MicQuizMicHudMotionSet(2);
    micQuizListenReady = 1;
    micQuizResponseState = 1;
}

/* Return the current response process state to the round controller. */
s32 MicQuizResponseStateGet(void) {
    return micQuizResponseState;
}

/* Apply the inactive-player mask to the quiz dialogue and hint windows. */
void MicQuizWinPlayerMaskSet(void) {
    s32 playerIndex;
    u8 disabledPlayers;

    disabledPlayers = 0;
    playerIndex = 0;
    while (playerIndex < 4) {
        if (!MicQuizPlayerActiveCheck(playerIndex)) {
            disabledPlayers |= 1 << playerIndex;
        }
        playerIndex += 1;
    }
    HuWinDisablePlayerReset(micQuizDialogWin0, 255);
    HuWinDisablePlayerReset(micQuizDialogWin1, 255);
    HuWinDisablePlayerReset(micQuizDialogWin2, 255);
    HuWinDisablePlayerReset(micQuizDialogWin3, 255);
    HuWinDisablePlayerReset(micQuizPauseWin0, 255);
    HuWinDisablePlayerReset(micQuizPauseWin1, 255);
    HuWinDisablePlayerSet(micQuizDialogWin0, disabledPlayers);
    HuWinDisablePlayerSet(micQuizDialogWin1, disabledPlayers);
    HuWinDisablePlayerSet(micQuizDialogWin2, disabledPlayers);
    HuWinDisablePlayerSet(micQuizDialogWin3, disabledPlayers);
    HuWinDisablePlayerSet(micQuizPauseWin0, disabledPlayers);
    HuWinDisablePlayerSet(micQuizPauseWin1, disabledPlayers);
}

/* Create the four dialogue windows and the controller instruction line at quiz setup. */
void MicQuizWinCreate(void) {
    u8 colorA[4] = {0, 0, 0, 255};
    u8 colorB[4] = {0, 255, 0, 255};

    micQuizDialogWin0 = HuWinExCreateFrame(16.0f, 344.0f, 544, 96, 6, 4);
    micQuizDialogWin1 = HuWinExCreateFrame(16.0f, 344.0f, 544, 96, 7, 3);
    micQuizDialogWin2 = HuWinExCreateFrame(16.0f, 344.0f, 544, 96, 23, 5);
    micQuizDialogWin3 = HuWinExCreateFrame(16.0f, 344.0f, 544, 96, 13, 0);

    MicQuizWinPlayerMaskSet();
    HuWinAttrSet(micQuizDialogWin0, HUWIN_ATTR_UPAUSE);
    HuWinAttrSet(micQuizDialogWin1, HUWIN_ATTR_UPAUSE);
    HuWinAttrSet(micQuizDialogWin2, HUWIN_ATTR_UPAUSE);
    HuWinAttrSet(micQuizDialogWin3, HUWIN_ATTR_UPAUSE);

    micQuizInstructionWin = HuWinCreate(16.0f, 312.0f, 544, 32, 0);
    HuWinBGTPLvlSet(micQuizInstructionWin, 0.0f);
    HuWinMesSpeedSet(micQuizInstructionWin, 0);
    HuWinAttrSet(micQuizInstructionWin, HUWIN_ATTR_ALIGN_CENTER);
    HuWinDispOff(micQuizDialogWin0);
    HuWinDispOff(micQuizDialogWin1);
    HuWinDispOff(micQuizDialogWin2);
    HuWinDispOff(micQuizDialogWin3);
    HuWinPriSet(micQuizDialogWin0, 100);
    HuWinPriSet(micQuizDialogWin1, 100);
    HuWinPriSet(micQuizDialogWin2, 100);
    HuWinPriSet(micQuizDialogWin3, 100);
    HuWinPriSet(micQuizInstructionWin, 100);
    HuWinCallbackSet(micQuizDialogWin0, MicQuizWinSoundCallback);
    HuWinCallbackSet(micQuizDialogWin1, MicQuizWinSoundCallback);
    HuWinCallbackSet(micQuizDialogWin2, MicQuizWinSoundCallback);
    HuWinCallbackSet(micQuizDialogWin3, MicQuizWinSoundCallback);
    micQuizActiveWin = -1;
}

/* Select the dialogue window used by the requested speaker slot. */
s16 MicQuizWinGet(s32 windowNo)
{
    s16 windowId;

    switch (windowNo) {
    case 0:
        windowId = micQuizDialogWin0;
        break;
    case 1:
        windowId = micQuizDialogWin1;
        break;
    case 2:
        windowId = micQuizDialogWin2;
        break;
    case 3:
        windowId = micQuizDialogWin3;
        break;
    }
    return windowId;
}

/* Slide the response banner into view when a participant starts answering. */
void MicQuizBannerSlideIn(void) {
    f32 travelDistance;
    f32 angle;
    f32 sineOffset;

    travelDistance = 362.0f;
    HuSprAttrReset(micQuizBannerGroup, 0, HUSPR_ATTR_DISPOFF);
    angle = 0.0f;
    if (micQuizBannerShown == 0) {
        do {
            HuPrcVSleep();
            sineOffset =
                (f32) ((f64) travelDistance * sin((M_PI * (f64) angle) / 180.0));
            HuSprPosSet(micQuizBannerGroup, 0, micQuizBannerX, micQuizBannerY + sineOffset);
            angle += 3.0f;
        } while (!(angle >= 90.0f));
        micQuizBannerY = 210.0f;
        HuSprPosSet(micQuizBannerGroup, 0, micQuizBannerX, micQuizBannerY);
        micQuizBannerShown = 1;
    }
}

/* Slide the response banner off the screen after an answer or timeout. */
void MicQuizBannerSlideOut(void) {
    f32 travelDistance;
    f32 angle;
    f32 sineOffset;

    travelDistance = -362.0f;
    angle = 0.0f;
    if (micQuizBannerShown != 0) {
        do {
            HuPrcVSleep();
            sineOffset =
                (f32) ((f64) travelDistance * sin((M_PI * (f64) angle) / 180.0));
            HuSprPosSet(micQuizBannerGroup, 0, micQuizBannerX, micQuizBannerY + sineOffset);
            angle += 3.0f;
        } while (!(angle >= 90.0f));
        micQuizBannerY = -152.0f;
        HuSprPosSet(micQuizBannerGroup, 0, micQuizBannerX, micQuizBannerY);
        HuSprAttrSet(micQuizBannerGroup, 0, HUSPR_ATTR_DISPOFF);
        micQuizBannerShown = 0;
    }
}

/* Start a message at the requested speed in one of the dialogue windows. */
void MicQuizWinMesSet(u32 messageId, s16 messageSpeed, u8 windowIndex) {
    s16 window = MicQuizWinGet(windowIndex);

    HuWinAttrSet(window, HUWIN_ATTR_NOCANCEL);
    HuWinMesSet(window, messageId);
    HuWinMesSpeedSet(window, messageSpeed);
}

/* Close the active dialogue window between quiz exchanges. */
void MicQuizWinClose(void)
{
    if (micQuizActiveWin != -1) {
        HuWinExClose(micQuizActiveWin);
        micQuizActiveWin = -1;
    }
}

/* Open the requested dialogue window, closing the previous speaker first. */
void MicQuizWinOpen(s32 windowNo) {
    s16 window = MicQuizWinGet(windowNo);

    if (micQuizActiveWin != window) {
        if (micQuizActiveWin != -1) {
            MicQuizWinClose();
        }
        HuWinExOpen(window);
        micQuizActiveWin = window;
    }
}

/* Wait for the active dialogue message before the round process continues. */
void MicQuizWinMesWait(void)
{
    if (micQuizActiveWin != -1) {
        HuWinMesWait(micQuizActiveWin);
    }
}

/* Read a choice from the active dialogue window. */
short MicQuizWinChoiceGet(short choice) {
    return HuWinChoiceGet(micQuizActiveWin,choice);
}

/* Insert a message into a numbered slot in the selected dialogue window. */
void MicQuizWinInsertMesSet(s32 windowNo, u32 messageId, s16 insertNo) {
    HuWinInsertMesSet(MicQuizWinGet(windowNo), messageId, insertNo);
}

/* Clear the short response hint line before its next message. */
void MicQuizHintClear(void) {
    HuWinHomeClear(micQuizHintWin);
}

/* Clear the controller instruction line between response states. */
void MicQuizInstructionClear(void) {
    HuWinHomeClear(micQuizInstructionWin);
}

/* Show or hide the controller instruction line during quiz transitions. */
void MicQuizInstructionDispSet(s32 visible) {
    if (visible)
    {
        HuWinDispOn(micQuizInstructionWin);
        return;
    }
    HuWinDispOff(micQuizInstructionWin);
}

/* Change the controller instruction message only when its response state changes. */
void MicQuizInstructionSet(u32 messageType) {
    u32 messageId;

    if (micQuizInstructionState != (s32) messageType) {
        switch (messageType) {
        case 0:
            messageId = MESSNUM(MESS_SYS_GUIDE, 23);
            break;
        case 1:
            messageId = MESSNUM(MESS_SYS_GUIDE, 24);
            break;
        case 2:
            messageId = MESSNUM(MESS_SYS_GUIDE, 25);
            break;
        case 3:
            messageId = MESSNUM(MESS_SYS_GUIDE, 26);
            break;
        case 4:
            messageId = MESSNUM(MESS_SYS_GUIDE, 27);
            break;
        case 7:
            messageId = MESSNUM(MESS_SYS_GUIDE, 30);
            break;
        case 8:
            messageId = MESSNUM(MESS_SYS_GUIDE, 31);
            break;
        case 9:
            messageId = MESSNUM(MESS_SYS_GUIDE, 30);
            break;
        case 10:
            messageId = MESSNUM(MESS_SYS_GUIDE, 31);
            break;
        }
        HuWinMesSet(micQuizInstructionWin, messageId);
        micQuizInstructionState = messageType;
    }
}

/* Set the short response hint for the requested answer state. */
void MicQuizHintSet(s32 messageType) {
    s32 messageId;
    switch (messageType)
    {
    case 5 :
        messageId = MESSNUM(MESS_SYS_GUIDE, 28);
        break;
    case 6 :
        messageId = MESSNUM(MESS_SYS_GUIDE, 29);
        break;
    }
    HuWinMesSet(micQuizHintWin, messageId);
}

/* Draw and scroll the tiled background used while the quiz is paused. */
void MicQuizPauseBgDraw(s16 layerNo)
{
    Mtx44 projection;
    Mtx textureMatrix;

    if (micQuizPauseBgAnim) {
        GXInvalidateTexAll();
        C_MTXOrtho(projection, 0.0f, 480.0f,
                   0.0f, 576.0f, 0.0f, 10.0f);
        GXSetProjection(projection, GX_ORTHOGRAPHIC);

        if (RenderMode->field_rendering != 0) {
            GXSetViewportJitter(0.0f, 0.0f, 640.0f, 480.0f,
                                0.0f, 1.0f, VIGetNextField());
        } else {
            GXSetViewport(0.0f, 0.0f, 640.0f, 480.0f,
                          0.0f, 1.0f);
        }

        GXSetScissor(0, 0, 640, 480);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
        GXSetCullMode(GX_CULL_NONE);
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
                       GX_LO_NOOP);
        GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
        GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL,
                      GX_DF_CLAMP, GX_AF_SPOT);
        GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL,
                      GX_DF_CLAMP, GX_AF_SPOT);

        PSMTXIdentity(textureMatrix);
        GXLoadPosMtxImm(textureMatrix, GX_PNMTX0);
        PSMTXTrans(textureMatrix, -micQuizPauseBgScroll, -micQuizPauseBgScroll, 0.0f);
        GXLoadTexMtxImm(textureMatrix, GX_TEXMTX0, GX_MTX2x4);
        HuSprTexLoad(micQuizPauseBgAnim, 0, GX_TEXMAP0,
                     GX_REPEAT, GX_REPEAT, GX_LINEAR);
        GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
                          GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0,
                      GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
                        GX_CC_TEXC);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                        GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
                        GX_CA_TEXA);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                        GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetNumTexGens(1);
        GXSetNumTevStages(1);

        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition2f32(0.0f, 0.0f);
        GXTexCoord2f32(0.0f, 0.0f);
        GXPosition2f32(576.0f, 0.0f);
        GXTexCoord2f32(36.0f, 0.0f);
        GXPosition2f32(576.0f, 480.0f);
        GXTexCoord2f32(36.0f, 30.0f);
        GXPosition2f32(0.0f, 480.0f);
        GXTexCoord2f32(0.0f, 30.0f);

        micQuizPauseBgScroll += 0.02f;
        if (micQuizPauseBgScroll >= 0.999f) {
            micQuizPauseBgScroll = 0.0f;
        }
    }
}

/* Start pause handling and remember the active camera and quiz parent process. */
void MicQuizPauseInit(HUPROCESS *parentProcess) {
    f32 fieldOfView;
    f32 nearPlane;
    f32 farPlane;
    s32 cameraIndex;
    s32 activeCameraBit;

    micQuizPauseProc = fn_8000DAC0(MicQuizPauseWatch, -536, 8192, 0, fn_8000DD9C());
    HuPrcSetStat(micQuizPauseProc, 0x4 | 0x8);
    MicQuizPauseAllowSet(0);
    cameraIndex = 0;
    /* When several cameras are active, the last one receives the pause background. */
    while (cameraIndex < 16) {
        fn_80027D8C(1 << cameraIndex, &fieldOfView, &nearPlane, &farPlane);
        if (-1.0f != fieldOfView) {
            activeCameraBit = 1 << cameraIndex;
        }
        cameraIndex += 1;
    }
    micQuizPauseCameraBit = activeCameraBit;
    micQuizParentProc = parentProcess;
}

/* Watch for Start during play, then run the pause menu and resume or leave the quiz. */
void MicQuizPauseWatch(void)
{
    u8 playerIndex;
    u8 selectedPlayer;
    u8 disabledPlayers;
    s16 windowNo;
    u32 homeMessageId;
    u32 confirmMessageId;

    for (;;) {
        s32 playerSelected = 0;
        for (;;) {
            HuPrcVSleep();
            if (WipeCheck() != 0) continue;
            if (fn_8003B774() == -1) continue;
            if (GameMesStatGet(-1) != 0) continue;
            if ((s16)HuMCSelModeGet() == 1) continue;
            if (lbl_802C0234 + fn_8003B748() != 0) continue;
            if (micQuizModeWork->pauseAllowed == 0) continue;

            for (playerIndex = 0; playerIndex < 4U; playerIndex++) {
                if (omPadErrChk(playerIndex) == 0 &&
                    (lbl_802C0008[playerIndex] & PAD_BUTTON_START) != 0) {
                    selectedPlayer = playerIndex;
                    micQuizModeWork->pauseDisplayActive = 1;
                    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 10);
                    while (WipeCheck() != 0) HuPrcVSleep();
                    HuWinDispOff(micQuizInstructionWin);
                    MicQuizPauseOpen();
                    HuWinHomeClear(micQuizHintWin);
                    homeMessageId = MESSNUM(MESS_SYS_GUIDE, 28);
                    HuWinMesSet(micQuizHintWin, homeMessageId);
                    playerSelected = 1;
                    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 10);
                    while (WipeCheck() != 0) HuPrcVSleep();
                    break;
                }
            }
            if (playerSelected != 0) break;
        }

        for (;;) {
            HuPrcVSleep();
            if ((lbl_802C0008[selectedPlayer] & PAD_BUTTON_START) != 0) {
                HuAudFXPlay(MSM_SE_CMN_START_BUTTON_CONFIRM);
                WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 10);
                while (WipeCheck() != 0) HuPrcVSleep();
                MicQuizPauseClose();
                HuWinHomeClear(micQuizHintWin);
                HuWinDispOn(micQuizInstructionWin);
                micQuizModeWork->pauseDisplayActive = 0;
                WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 10);
                while (WipeCheck() != 0) HuPrcVSleep();
                break;
            }
            if ((lbl_802C0008[selectedPlayer] & PAD_BUTTON_X) == 0) continue;

            HuAudFXPlay(MSM_SE_CMN_CHOICE_CANCEL);
            HuWinPosSet(micQuizHintWin, 16.0f, 312.0f);
            disabledPlayers = 0;
            for (playerIndex = 0; playerIndex < 4U; playerIndex++) {
                if (playerIndex != selectedPlayer) disabledPlayers |= 1 << playerIndex;
            }
            if (micQuizModeWork->presenterLayout == 0) windowNo = micQuizPauseWin0;
            else windowNo = micQuizPauseWin1;
            HuWinDisablePlayerSet(windowNo, disabledPlayers);
            confirmMessageId = MESSNUM(MESS_SYS_GUIDE, 29);
            HuWinMesSet(micQuizHintWin, confirmMessageId);
            HuWinExOpen(windowNo);
            HuWinAttrSet(windowNo, HUWIN_ATTR_NOCANCEL);
            if (micQuizModeWork->presenterLayout == 0)
                HuWinMesSet(windowNo, MESSNUM(MESS_MICQUIZ, 47));
            else HuWinMesSet(windowNo, MESSNUM(MESS_MICQUIZ, 101));
            HuWinMesSpeedSet(windowNo, 1);
            HuWinMesWait(windowNo);
            if (HuWinChoiceGet(windowNo, -1) == 0) {
                HuAudFXPlay(MSM_SE_CMN_START_BUTTON_CONFIRM);
                WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 10);
                while (WipeCheck() != 0) HuPrcVSleep();
                HuWinExClose(windowNo);
                HuWinHomeClear(micQuizHintWin);
                MicQuizPauseClose();
                HuWinDispOn(micQuizInstructionWin);
                micQuizModeWork->pauseDisplayActive = 0;
                HuWinDisablePlayerReset(windowNo, 15);
                WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 10);
                while (WipeCheck() != 0) HuPrcVSleep();
                break;
            }

            HuAudSStreamFadeOut(micQuizStreamNo, 600);
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
            while (WipeCheck() != 0) HuPrcVSleep();
            MicQuizPauseClose();
            HuMCClose();
            HuMCSessionClose();
            HuWinDisablePlayerReset(windowNo, 15);
            if (micQuizModeWork->compactPauseDisplay != 0) fn_8003A5E4(1, 1);
            else fn_8003A5E4(2, 1);
        }
    }
}

/* Save pause state, show the pause display, and suspend quiz animation and audio. */
void MicQuizPauseOpen(void) {
    micQuizSavedPauseEnable = (s32) omSysPauseEnableFlag;
    micQuizPauseOverlayFlag = (s32) fn_8003B748();
    lbl_802C0234 = 1;
    omSysPauseEnable(0U);
    fn_8002583C((s16) micQuizPauseCameraBit, 15, MicQuizPauseBgDraw);
    if (micQuizModeWork->compactPauseDisplay != 0) {
        MicQuizCompactHudDrawPause();
        MicQuizPauseDispUpdate();
    } else {
        MicQuizHudDrawPause();
    }
    MicQuizPauseDispOn();
    fn_8003B674(1);
    HuPrcAllPause(1);
    Hu3DPauseSet(1);
    HuSprPauseSet(1);
    fn_80006F3C();
    HuAudFXPauseAll(1);
    HuAudSStreamPauseAll(1);
    HuAudFXPlay(MSM_SE_CMN_START_BUTTON_CONFIRM);
}

/* Restore the quiz display and the pause state saved when the menu opened. */
void MicQuizPauseClose(void) {
    lbl_802C0234 = 0;
    omSysPauseEnable((u8) micQuizSavedPauseEnable);
    fn_8003B674(0);
    fn_800258A8((s16) micQuizPauseCameraBit, 15);
    if (micQuizModeWork->compactPauseDisplay != 0) {
        MicQuizCompactHudDrawNormal();
    } else {
        MicQuizHudDrawNormal();
    }
    MicQuizPauseDispOff();
    /* Keep animation and audio paused if they were already paused before the menu. */
    if (micQuizPauseOverlayFlag == 0) {
        HuPrcAllPause(0);
        Hu3DPauseSet(0);
        HuSprPauseSet(0);
        HuAudFXPauseAll(0);
        HuAudSStreamPauseAll(0);
    }
}

/* Build the question HUD, event decorations, feedback icons, and dialogue windows. */
void MicQuizHudCreate(void) {
    s32 eventType;
    s32 playerIndex;
    s32 character;

    micQuizModeWork = MgModeWorkGet();
micQuizMicHudGroup = HuSprGrpCreate(8);
    HuSprGrpPosSet(micQuizMicHudGroup, 493.0f, 386.0f);
playerIndex = 0;
    while (playerIndex < 4) {
        if (MicQuizPlayerActiveCheck(playerIndex) == 0) {
            micQuizPortraitAnim[playerIndex] = HuSprAnimRead(
                fn_80008DD0(DATANUM(0x9b0000, 0), 0x10000000, 2));
            HuSprGrpMemberSet(micQuizMicHudGroup, (s16) (playerIndex + 4),
                              HuSprCreate(micQuizPortraitAnim[playerIndex], 48, 0));
            HuSprPosSet(micQuizMicHudGroup, (s16) (playerIndex + 4), -23.0f, -25.0f);
            HuSprAttrSet(micQuizMicHudGroup, (s16) (playerIndex + 4), HUSPR_ATTR_DISPOFF);
        } else {
            character = lbl_80265728[playerIndex].charNo;
            micQuizPortraitAnim[playerIndex] = HuSprAnimRead(
                fn_80008DD0(micQuizPortraitFiles[character], 0x10000000, 2));
            HuSprGrpMemberSet(micQuizMicHudGroup, (s16) (playerIndex + 4),
                              HuSprCreate(micQuizPortraitAnim[playerIndex], 48, 0));
            HuSprPosSet(micQuizMicHudGroup, (s16) (playerIndex + 4), -23.0f, -25.0f);
            HuSprAttrSet(micQuizMicHudGroup, (s16) (playerIndex + 4), HUSPR_ATTR_DISPOFF);
        }
        playerIndex += 1;
    }
    micQuizMicHudBaseAnim =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 28), 0x10000000, 2));
    HuSprGrpMemberSet(micQuizMicHudGroup, 0, HuSprCreate(micQuizMicHudBaseAnim, 49, 0));
    HuSprPosSet(micQuizMicHudGroup, 0, 0.0f, 0.0f);
    micQuizMicHudMicAnim =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 29), 0x10000000, 2));
    HuSprGrpMemberSet(micQuizMicHudGroup, 1, HuSprCreate(micQuizMicHudMicAnim, 49, 0));
    HuSprPosSet(micQuizMicHudGroup, 1, 8.0f, 12.0f);
    micQuizMicHudIdleAnim =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 30), 0x10000000, 2));
    HuSprGrpMemberSet(micQuizMicHudGroup, 2, HuSprCreate(micQuizMicHudIdleAnim, 49, 0));
    HuSprPosSet(micQuizMicHudGroup, 2, 13.0f, 9.0f);
    micQuizMicHudMarkerAnim =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 32), 0x10000000, 2));
    HuSprGrpMemberSet(micQuizMicHudGroup, 3, HuSprCreate(micQuizMicHudMarkerAnim, 49, 0));
    HuSprPosSet(micQuizMicHudGroup, 3, -23.0f, -25.0f);
    micQuizMicIconAnim =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 36), 0x10000000, 2));
    micQuizMicIconGroup = HuSprGrpCreate(1);
    HuSprGrpMemberSet(micQuizMicIconGroup, 0, HuSprCreate(micQuizMicIconAnim, 50, 0));
    HuSprPosSet(micQuizMicIconGroup, 0, 288.0f, 68.0f);
    HuSprAttrSet(micQuizMicIconGroup, 0, HUSPR_ATTR_DISPOFF);
    micQuizMicAnim =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 37), 0x10000000, 2));
    micQuizMicGroup = HuSprGrpCreate(1);
    HuSprGrpMemberSet(micQuizMicGroup, 0, HuSprCreate(micQuizMicAnim, 50, 0));
    HuSprPosSet(micQuizMicGroup, 0, 288.0f, 68.0f);
    HuSprBankSet(micQuizMicGroup, 0, 1);
    micQuizMicGlowAnim =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 38), 0x10000000, 2));
    micQuizMicGlowGroup = HuSprGrpCreate(1);
    HuSprGrpMemberSet(micQuizMicGlowGroup, 0, HuSprCreate(micQuizMicGlowAnim, 50, 0));
    HuSprPosSet(micQuizMicGlowGroup, 0, 288.0f, 68.0f);
    HuSprTPLvlSet(micQuizMicGlowGroup, 0, 0.0f);
    micQuizCountdownGroup[0] = HuSprGrpCreate(2);
    micQuizCountdownDigitAnim[0] =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 86), 0x10000000, 2));
    HuSprGrpMemberSet(micQuizCountdownGroup[0], 0,
                      HuSprCreate(micQuizCountdownDigitAnim[0], 50, 0));
    HuSprPosSet(micQuizCountdownGroup[0], 0, -16.0f, 0.0f);
    HuSprBankSet(micQuizCountdownGroup[0], 0, 0);
    micQuizCountdownDigitAnim[1] =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 86), 0x10000000, 2));
    HuSprGrpMemberSet(micQuizCountdownGroup[0], 1,
                      HuSprCreate(micQuizCountdownDigitAnim[1], 50, 0));
    HuSprPosSet(micQuizCountdownGroup[0], 1, 16.0f, 0.0f);
    HuSprBankSet(micQuizCountdownGroup[0], 1, 0);
    HuSprGrpAttrSet(micQuizCountdownGroup[0], HUSPR_ATTR_DISPOFF);
    HuSprGrpPosSet(micQuizCountdownGroup[0], 271.0f, 68.0f);
    eventType = MicQuizQuestionEventGet();
    if (eventType != 8) {
        micQuizEventIconAnim = HuSprAnimRead(
            fn_80008DD0(DATANUM(0xa20000, 21), 0x10000000, 2));
        micQuizEventIconGroup = HuSprGrpCreate(1);
        HuSprGrpMemberSet(micQuizEventIconGroup, 0, HuSprCreate(micQuizEventIconAnim, 50, 0));
        HuSprPosSet(micQuizEventIconGroup, 0, 401.0f, 62.0f);
        HuSprBankSet(micQuizEventIconGroup, 0, (s16) (eventType + 1));
        if (eventType == 3) {
            micQuizEventFragmentAnim[0] = HuSprAnimRead(
                fn_80008DD0(DATANUM(0xa20000, 54), 0x10000000, 2));
            micQuizEventFragmentAnim[1] = HuSprAnimRead(
                fn_80008DD0(DATANUM(0xa20000, 55), 0x10000000, 2));
            micQuizEventOverlayAnim = HuSprAnimRead(
                fn_80008DD0(DATANUM(0xa20000, 56), 0x10000000, 2));
            micQuizEventOverlayGroup = HuSprGrpCreate(1);
            HuSprGrpMemberSet(micQuizEventOverlayGroup, 0,
                              HuSprCreate(micQuizEventOverlayAnim, 49, 0));
            HuSprPosSet(micQuizEventOverlayGroup, 0, 401.0f, 62.0f);
            HuSprAttrSet(micQuizEventOverlayGroup, 0, HUSPR_ATTR_DISPOFF);
        } else if (eventType == 2) {
            micQuizEventPulseAnim = HuSprAnimRead(
                fn_80008DD0(DATANUM(0xa20000, 57), 0x10000000, 2));
            micQuizEventPulseGroup = HuSprGrpCreate(1);
            HuSprGrpMemberSet(micQuizEventPulseGroup, 0, HuSprCreate(micQuizEventPulseAnim, 51, 0));
            HuSprPosSet(micQuizEventPulseGroup, 0, 401.0f, 62.0f);
            micQuizEffectProc[0] = fn_8000DAC0(MicQuizEventPulse, 100U, 6144U, 0, fn_8000DD9C());
            micQuizEffectProc[0]->property = (void *) micQuizEventPulseGroup;
        } else if (eventType == 4) {
            micQuizEventBadgeAnim = HuSprAnimRead(
                fn_80008DD0(DATANUM(0xa20000, 58), 0x10000000, 2));
            micQuizEventBadgeGroup = HuSprGrpCreate(1);
            HuSprGrpMemberSet(micQuizEventBadgeGroup, 0, HuSprCreate(micQuizEventBadgeAnim, 51, 0));
            HuSprPosSet(micQuizEventBadgeGroup, 0, 401.0f, 62.0f);
            micQuizEffectProc[0] = fn_8000DAC0(MicQuizEventPulse, 100U, 6144U, 0, fn_8000DD9C());
            micQuizEffectProc[0]->property = (void *) micQuizEventBadgeGroup;
            micQuizEventBannerAnim = HuSprAnimRead(
                fn_80008DD0(DATANUM(0xa20000, 59), 0x10000000, 2));
            micQuizEventBannerGroup = HuSprGrpCreate(1);
            HuSprGrpMemberSet(micQuizEventBannerGroup, 0,
                              HuSprCreate(micQuizEventBannerAnim, 51, 0));
            HuSprPosSet(micQuizEventBannerGroup, 0, 288.0f, 68.0f);
            micQuizTwinkleAnim = HuSprAnimRead(
                fn_80008DD0(DATANUM(0xa20000, 60), 0x10000000, 2));
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                micQuizTwinkleGroup[playerIndex] = HuSprGrpCreate(1);
                HuSprGrpMemberSet(micQuizTwinkleGroup[playerIndex], 0,
                                  HuSprCreate(micQuizTwinkleAnim, 52, 0));
                HuSprPosSet(micQuizTwinkleGroup[playerIndex], 0,
                            micQuizTwinklePos[playerIndex].positionX,
                            micQuizTwinklePos[playerIndex].positionY);
                HuSprAttrSet(micQuizTwinkleGroup[playerIndex], 0, HUSPR_ATTR_NOANIM);
                micQuizEffectProc[playerIndex + 2] =
                    fn_8000DAC0(MicQuizTwinkle, 100U, 6144U, 0, fn_8000DD9C());
                micQuizEffectProc[playerIndex
                                  + 2]->property = (void *) micQuizTwinkleGroup[playerIndex];
            }
            micQuizPulseLeftAnim = HuSprAnimRead(
                fn_80008DD0(DATANUM(0xa20000, 61), 0x10000000, 2));
            micQuizPulseLeftGroup = HuSprGrpCreate(1);
            HuSprGrpMemberSet(micQuizPulseLeftGroup, 0, HuSprCreate(micQuizPulseLeftAnim, 53, 0));
            HuSprPosSet(micQuizPulseLeftGroup, 0, 248.0f, 52.0f);
            fn_8000DAC0(MicQuizEventPulse, 100U, 6144U, 0, fn_8000DD9C());
            micQuizEffectProc[0] = fn_8000DAC0(MicQuizSpritePulseRandom, 100U, 6144U, 0,
                                               fn_8000DD9C());
            micQuizEffectProc[0]->property = (void *) micQuizPulseLeftGroup;
            micQuizPulseRightAnim = HuSprAnimRead(
                fn_80008DD0(DATANUM(0xa20000, 62), 0x10000000, 2));
            micQuizPulseRightGroup = HuSprGrpCreate(1);
            HuSprGrpMemberSet(micQuizPulseRightGroup, 0, HuSprCreate(micQuizPulseRightAnim, 54, 0));
            HuSprPosSet(micQuizPulseRightGroup, 0, 328.0f, 84.0f);
            micQuizEffectProc[1] = fn_8000DAC0(MicQuizSpritePulseRandom, 100U, 6144U, 0,
                                               fn_8000DD9C());
            micQuizEffectProc[1]->property = (void *) micQuizPulseRightGroup;
        } else if (eventType == 5 || eventType == 7) {
            micQuizEventBadgeAnim = HuSprAnimRead(
                fn_80008DD0(DATANUM(0xa20000, 58), 0x10000000, 2));
            micQuizEventBadgeGroup = HuSprGrpCreate(1);
            HuSprGrpMemberSet(micQuizEventBadgeGroup, 0, HuSprCreate(micQuizEventBadgeAnim, 51, 0));
            HuSprPosSet(micQuizEventBadgeGroup, 0, 401.0f, 62.0f);
            micQuizEffectProc[0] = fn_8000DAC0(MicQuizEventPulse, 100U, 6144U, 0, fn_8000DD9C());
            micQuizEffectProc[0]->property = (void *) micQuizEventBadgeGroup;
        } else if (eventType == 0 || eventType == 1) {
            micQuizEventSparkAnim = HuSprAnimRead(
                fn_80008DD0(DATANUM(0xa20000, 53), 0x10000000, 2));
        } else if (eventType == 6) {
            micQuizEventSparkAnim = HuSprAnimRead(
                fn_80008DD0(DATANUM(0xa20000, 63), 0x10000000, 2));
        }
    }
    micQuizIndicatorAnim =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 33), 0x10000000, 2));
    micQuizIndicatorGroup = HuSprGrpCreate(1);
    HuSprGrpMemberSet(micQuizIndicatorGroup, 0, HuSprCreate(micQuizIndicatorAnim, 50, 0));
    HuSprPosSet(micQuizIndicatorGroup, 0, 506.0f, 65.0f);
    HuSprBankSet(micQuizIndicatorGroup, 0, 0);
    micQuizIndicatorSwingAnim =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 45), 0x10000000, 2));
    micQuizIndicatorSwingGroup = HuSprGrpCreate(1);
    HuSprGrpMemberSet(micQuizIndicatorSwingGroup, 0, HuSprCreate(micQuizIndicatorSwingAnim, 50, 0));
    HuSprPosSet(micQuizIndicatorSwingGroup, 0, 504.0f, 99.0f);
    HuSprBankSet(micQuizIndicatorSwingGroup, 0, 0);
    micQuizIndicatorDoneAnim =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 46), 0x10000000, 2));
    micQuizIndicatorDoneGroup = HuSprGrpCreate(1);
    HuSprGrpMemberSet(micQuizIndicatorDoneGroup, 0, HuSprCreate(micQuizIndicatorDoneAnim, 50, 0));
    HuSprPosSet(micQuizIndicatorDoneGroup, 0, 504.0f, 99.0f);
    HuSprBankSet(micQuizIndicatorDoneGroup, 0, 0);
    HuSprGrpAttrSet(micQuizIndicatorDoneGroup, HUSPR_ATTR_DISPOFF);
    micQuizCategoryAnim =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 44), 0x10000000, 2));
    micQuizCategoryGroup = HuSprGrpCreate(1);
    HuSprGrpMemberSet(micQuizCategoryGroup, 0, HuSprCreate(micQuizCategoryAnim, 50, 0));
    HuSprPosSet(micQuizCategoryGroup, 0, 102.0f, 67.0f);
    HuSprBankSet(micQuizCategoryGroup, 0, (s16) micQuizModeWork->questionColumn);
    micQuizAcceptIconAnim =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 42), 0x10000000, 2));
    micQuizAcceptIconGroup = HuSprGrpCreate(1);
    HuSprGrpMemberSet(micQuizAcceptIconGroup, 0, HuSprCreate(micQuizAcceptIconAnim, 50, 0));
    HuSprPosSet(micQuizAcceptIconGroup, 0, 292.0f, 208.0f);
    HuSprAttrSet(micQuizAcceptIconGroup, 0, HUSPR_ATTR_DISPOFF);
    micQuizRejectIconAnim =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 43), 0x10000000, 2));
    micQuizRejectIconGroup = HuSprGrpCreate(1);
    HuSprGrpMemberSet(micQuizRejectIconGroup, 0, HuSprCreate(micQuizRejectIconAnim, 50, 0));
    HuSprPosSet(micQuizRejectIconGroup, 0, 292.0f, 208.0f);
    HuSprAttrSet(micQuizRejectIconGroup, 0, HUSPR_ATTR_DISPOFF);
    micQuizBannerAnim =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 103), 0x10000000, 2));
    micQuizBannerGroup = HuSprGrpCreate(1);
    HuSprGrpMemberSet(micQuizBannerGroup, 0, HuSprCreate(micQuizBannerAnim, 1500, 0));
    micQuizBannerX = 285.0f;
    micQuizBannerY = -152.0f;
    HuSprPosSet(micQuizBannerGroup, 0, micQuizBannerX, micQuizBannerY);
    HuSprAttrSet(micQuizBannerGroup, 0, HUSPR_ATTR_DISPOFF);
    micQuizBannerShown = 0;
    MicQuizWinCreate();
    MicQuizPauseDispCreate();
    MicQuizMicHudStateSet(0);
}

/* Build the compact microphone HUD, adding inactive placeholders until the first active
 * player's portrait, then stopping the portrait loop. */
void MicQuizCompactHudCreate(void) {
    s32 characterNo;
    s32 playerNo;

    micQuizModeWork = MgModeWorkGet();
    micQuizMicHudGroup = HuSprGrpCreate(8);
    HuSprGrpPosSet(micQuizMicHudGroup, 493.0f, 386.0f);
    for (playerNo = 0; playerNo < 4; playerNo++) {
        if (MicQuizPlayerActiveCheck(playerNo) == 0) {
            micQuizPortraitAnim[playerNo] = HuSprAnimRead(
                fn_80008DD0(DATANUM(0x9b0000, 0), 0x10000000, 2));
            HuSprGrpMemberSet(micQuizMicHudGroup, (s16) (playerNo + 4),
                              HuSprCreate(micQuizPortraitAnim[playerNo], 48, 0));
            HuSprPosSet(micQuizMicHudGroup, (s16) (playerNo + 4), -23.0f, -25.0f);
            HuSprAttrSet(micQuizMicHudGroup, (s16) (playerNo + 4), HUSPR_ATTR_DISPOFF);
        } else {
        characterNo = lbl_80265728[playerNo].charNo;
        micQuizPortraitAnim[playerNo] = HuSprAnimRead(
            fn_80008DD0(micQuizPortraitFiles[characterNo], 0x10000000, 2));
        HuSprGrpMemberSet(micQuizMicHudGroup, (s16) (playerNo + 4),
                          HuSprCreate(micQuizPortraitAnim[playerNo], 48, 0));
        HuSprPosSet(micQuizMicHudGroup, (s16) (playerNo + 4), -23.0f, -25.0f);
            break;
        }
    }
    micQuizMicHudBaseAnim =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 28), 0x10000000, 2));
    HuSprGrpMemberSet(micQuizMicHudGroup, 0, HuSprCreate(micQuizMicHudBaseAnim, 49, 1));
    HuSprPosSet(micQuizMicHudGroup, 0, 0.0f, 0.0f);
    micQuizMicHudMicAnim =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 29), 0x10000000, 2));
    HuSprGrpMemberSet(micQuizMicHudGroup, 1, HuSprCreate(micQuizMicHudMicAnim, 49, 1));
    HuSprPosSet(micQuizMicHudGroup, 1, 8.0f, 12.0f);
    micQuizMicHudIdleAnim =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 30), 0x10000000, 2));
    HuSprGrpMemberSet(micQuizMicHudGroup, 2, HuSprCreate(micQuizMicHudIdleAnim, 49, 0));
    HuSprPosSet(micQuizMicHudGroup, 2, 13.0f, 9.0f);
    MicQuizWinCreate();
    MicQuizPauseDispCreate();
}

/* Continuously pulse the child process sprite with a randomized starting phase. */
void MicQuizSpritePulseRandom(void) {
    HUPROCESS *process;
    s16 sprite;
    f32 angle;
    f32 randomAngle;
    f32 scale;
    f32 transparency;

    process = fn_8000DD9C();
    sprite = (s16)process->property;
    angle = (f32)(u32)frandmod(360);
    /* Advance the random stream again; this second phase is unused. */
    randomAngle = (f32)(u32)frandmod(360);
    HuSprTPLvlSet(sprite, 0,
        (f32)(0.5 +
        (0.5 * sin((M_PI * (f64)angle) /
        180.0))));
    HuSprScaleSet(sprite, 0,
        (f32)(1.0 +
        (0.5 * sin((M_PI * (f64)angle) /
        180.0))),
        1.0f);
    for (;;) {
        HuPrcVSleep();
        scale = (f32)(1.0 +
            (0.5 * sin((M_PI * (f64)angle) /
            180.0)));
        transparency = (f32)(0.5 +
            (0.5 * sin((M_PI * (f64)angle) /
            180.0)));
        HuSprTPLvlSet(sprite, 0,
            transparency);
        HuSprScaleSet(sprite, 0, scale, 1.0f);
        angle += 0.75f;
    }
}

/* Show, scale, and hide a decorative sprite repeatedly after random delays. */
void MicQuizTwinkle(void) {
    f32 scale;
    f32 angle;
    s32 initialDelay;
    s32 repeatDelay;
    HUPROCESS *currentProcess;
    s16 groupId;

    currentProcess = fn_8000DD9C();
    groupId = (s16) currentProcess->property;
    HuSprAttrSet(groupId, 0, HUSPR_ATTR_DISPOFF);
    HuSprScaleSet(groupId, 0, 0.0f, 0.0f);
    initialDelay = frandmod(60) + 60;
    repeatDelay = frandmod(120) + 60;
    HuPrcSleep(initialDelay);
    angle = 0.0f;
    while (1) {
        HuSprAttrReset(groupId, 0, HUSPR_ATTR_DISPOFF);
        HuSprAttrReset(groupId, 0, HUSPR_ATTR_NOANIM);
        do {
            HuPrcVSleep();
            scale = (f32) sin((M_PI * (f64) angle) / 180.0);
            HuSprScaleSet(groupId, 0, scale, scale);
            angle += 4.285714f;
        } while (!(angle >= 90.0f));
        angle = 0.0f;
        HuSprAttrSet(groupId, 0, HUSPR_ATTR_DISPOFF);
        HuSprAttrSet(groupId, 0, HUSPR_ATTR_NOANIM);
        HuPrcSleep(repeatDelay);
    }
}

/* Animate one short spark around the question event icon, then remove it. */
void MicQuizEventSpark(void) {
    f32 randomAngle;
    f32 randomAmplitude;
    s32 delay;
    s16 spriteGroup;
    f32 x;
    f32 y;
    f32 angle;
    s32 frame;
    f32 transparency;

    randomAngle = 2.0f * frandf();
    randomAmplitude = frandf();
    delay = frandmod(10);
    spriteGroup = HuSprGrpCreate(1);
    HuSprGrpMemberSet(spriteGroup, 0, HuSprCreate(micQuizEventSparkAnim, 51, 0));
    HuSprAttrSet(spriteGroup, 0, HUSPR_ATTR_DISPOFF);
    HuSprAttrSet(spriteGroup, 0, HUSPR_ATTR_NOANIM);
    HuPrcSleep(delay);
    x = (f32)(401.0 +
        (40.0 * ((f64)randomAmplitude *
        sin(M_PI * (f64)randomAngle))));
    y = (f32)(62.0 +
        (40.0 * ((f64)randomAmplitude *
        cos(M_PI * (f64)randomAngle))));
    HuSprPosSet(spriteGroup, 0, x, y);
    HuSprAttrReset(spriteGroup, 0, HUSPR_ATTR_DISPOFF);
    HuSprAttrReset(spriteGroup, 0, HUSPR_ATTR_NOANIM);

    angle = 0.0f;
    for (frame = 0; frame < 60; frame++) {
        HuPrcVSleep();
        transparency = (f32)(1.0 -
            sin((M_PI * (f64)angle) / 180.0));
        angle += 1.5f;
        if (frame <= 50) {
            HuSprTPLvlSet(spriteGroup, 0, transparency);
        }
    }
    HuSprGrpKill(spriteGroup);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Continuously pulse the event sprite assigned to this child process. */
void MicQuizEventPulse(void) {
    f32 spriteScale;
    f32 transparency;
    f32 angleDegrees;
    HUPROCESS *currentProcess;
    s16 spriteId;

    currentProcess = fn_8000DD9C();
    spriteId = (s16)currentProcess->property;
    for (;;) {
        HuPrcVSleep();
        spriteScale = (f32)(1.0 +
            (0.3f * sin((M_PI * angleDegrees) /
            180.0)));
        transparency = (f32)(1.0 -
            sin((M_PI * angleDegrees) / 180.0));
        HuSprTPLvlSet(spriteId, 0, transparency);
        HuSprScaleSet(spriteId, 0, spriteScale, spriteScale);
        angleDegrees += 1.5f;
        if (!(angleDegrees >= 90.0f)) {
            continue;
        }
        angleDegrees = 0.0f;
    }
}

/* Create the microphone HUD and player portraits beneath the current quiz process. */
void MicQuizMicHudCreate(void) {
    s32 characterNo;
    s32 playerIndex;

    micQuizParentProc = fn_8000DD9C();
    micQuizModeWork = MgModeWorkGet();
    micQuizMicHudGroup = HuSprGrpCreate(8);
    HuSprGrpPosSet(micQuizMicHudGroup, 493.0f, 386.0f);
    playerIndex = 0;
    while (playerIndex < 4) {
        if (MicQuizPlayerActiveCheck(playerIndex) == 0) {
            micQuizPortraitAnim[playerIndex] = HuSprAnimRead(
                fn_80008DD0(DATANUM(0x9b0000, 0), 0x10000000, 2));
            HuSprGrpMemberSet(micQuizMicHudGroup, (s16) (playerIndex + 4),
                              HuSprCreate(micQuizPortraitAnim[playerIndex], 48, 0));
            HuSprPosSet(micQuizMicHudGroup, (s16) (playerIndex + 4), -23.0f, -25.0f);
            HuSprAttrSet(micQuizMicHudGroup, (s16) (playerIndex + 4), HUSPR_ATTR_DISPOFF);
        } else {
            characterNo = lbl_80265728[playerIndex].charNo;
            micQuizPortraitAnim[playerIndex] = HuSprAnimRead(
                fn_80008DD0(micQuizPortraitFiles[characterNo], 0x10000000, 2));
            HuSprGrpMemberSet(micQuizMicHudGroup, (s16) (playerIndex + 4),
                              HuSprCreate(micQuizPortraitAnim[playerIndex], 48, 0));
            HuSprPosSet(micQuizMicHudGroup, (s16) (playerIndex + 4), -23.0f, -25.0f);
            HuSprAttrSet(micQuizMicHudGroup, (s16) (playerIndex + 4), HUSPR_ATTR_DISPOFF);
        }
        playerIndex += 1;
    }
    micQuizMicHudBaseAnim =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 28), 0x10000000, 2));
    HuSprGrpMemberSet(micQuizMicHudGroup, 0, HuSprCreate(micQuizMicHudBaseAnim, 49, 0));
    HuSprPosSet(micQuizMicHudGroup, 0, 0.0f, 0.0f);
    micQuizMicHudMicAnim =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 29), 0x10000000, 2));
    HuSprGrpMemberSet(micQuizMicHudGroup, 1, HuSprCreate(micQuizMicHudMicAnim, 49, 0));
    HuSprPosSet(micQuizMicHudGroup, 1, 8.0f, 12.0f);
    micQuizMicHudIdleAnim =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 30), 0x10000000, 2));
    HuSprGrpMemberSet(micQuizMicHudGroup, 2, HuSprCreate(micQuizMicHudIdleAnim, 49, 0));
    HuSprPosSet(micQuizMicHudGroup, 2, 13.0f, 9.0f);
    micQuizMicHudMarkerAnim =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 32), 0x10000000, 2));
    HuSprGrpMemberSet(micQuizMicHudGroup, 3, HuSprCreate(micQuizMicHudMarkerAnim, 49, 0));
    HuSprPosSet(micQuizMicHudGroup, 3, -23.0f, -23.0f);
    HuSprAttrSet(micQuizMicHudGroup, 2, HUSPR_ATTR_DISPOFF);
    HuSprBankSet(micQuizMicHudGroup, 0, 0);
    HuSprBankSet(micQuizMicHudGroup, 1, 0);
    HuSprGrpAttrSet(micQuizMicHudGroup, HUSPR_ATTR_DISPOFF);
}

/* Reveal the selected player portrait and bounce the microphone HUD into view. */
void MicQuizMicHudShow(s32 playerIndex) {
    f32 scale;
    f32 angle;
    s32 frame;

    scale = 0.0f;
    angle = 0.0f;
    HuSprGrpAttrReset(micQuizMicHudGroup, HUSPR_ATTR_DISPOFF);
    HuSprAttrSet(micQuizMicHudGroup, 2, HUSPR_ATTR_DISPOFF);
    for (frame = 0; frame < 4; frame++) {
        HuSprAttrSet(micQuizMicHudGroup, (s16)(frame + 4), HUSPR_ATTR_DISPOFF);
    }
    HuSprAttrSet(micQuizMicHudGroup, 3, HUSPR_ATTR_DISPOFF);
    HuSprGrpScaleSet(micQuizMicHudGroup, 0.0f, 0.0f);
    MicQuizPortraitShow(playerIndex);
    do {
        HuPrcVSleep();
        scale = (f32)(1.2f *
                      sin((M_PI * (f64)angle) / 180.0));
        HuSprGrpScaleSet(micQuizMicHudGroup, scale, scale);
        angle += 4.5f;
    } while (!(angle >= 90.0f));
    HuSprGrpScaleSet(micQuizMicHudGroup, 1.2f, 1.2f);

    scale = 1.2f;
    angle = 0.0f;
    do {
        HuPrcVSleep();
        scale = (f32)(1.2f -
                      (0.4f *
                       sin((M_PI * (f64)angle) / 180.0)));
        HuSprGrpScaleSet(micQuizMicHudGroup, scale, scale);
        angle += 18.0f;
    } while (!(angle >= 90.0f));
    HuSprGrpScaleSet(micQuizMicHudGroup, 0.8f, 0.8f);

    scale = 1.2f;
    angle = 0.0f;
    do {
        HuPrcVSleep();
        scale = (f32)(0.8f +
                      (0.2f *
                       sin((M_PI * (f64)angle) / 180.0)));
        HuSprGrpScaleSet(micQuizMicHudGroup, scale, scale);
        angle += 18.0f;
    } while (!(angle >= 90.0f));
    HuSprGrpScaleSet(micQuizMicHudGroup, 1.0f, 1.0f);
}

/* Shrink the microphone HUD away at the end of its display interval. */
void MicQuizMicHudHide(void) {
    f32 angle;
    f32 scale;

    scale = 0.0f;
    angle = 0.0f;
    HuSprGrpScaleSet(micQuizMicHudGroup, 1.0f, 1.0f);
    do {
        HuPrcVSleep();
        scale = (f32) (1.0 - sin((M_PI * (f64) angle) / 180.0));
        HuSprGrpScaleSet(micQuizMicHudGroup, scale, scale);
        angle += 3.0f;
    } while (!(angle >= 90.0f));
    HuSprGrpAttrSet(micQuizMicHudGroup, HUSPR_ATTR_DISPOFF);
    HuSprGrpScaleSet(micQuizMicHudGroup, 0.0f, 0.0f);
}

/* Create three score digits for each active player before score updates begin. */
void MicQuizScoreDigitsCreate(void) {
    s32 playerIndex;
    s32 digitIndex;
    s32 score;
    s32 hundreds;
    s32 tens;
    s32 ones;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (MicQuizPlayerActiveCheck(playerIndex) != 0) {
            for (digitIndex = 0; digitIndex < 3; digitIndex++) {
                micQuizScoreDigitAnim[playerIndex][digitIndex] = HuSprAnimRead(
                    fn_80008DD0(DATANUM(0xa20000, 86), 0x10000000, 2));
                micQuizScoreDigitGroup[playerIndex][digitIndex] = HuSprGrpCreate(1);
                HuSprGrpMemberSet(micQuizScoreDigitGroup[playerIndex][digitIndex], 0,
                                  HuSprCreate(micQuizScoreDigitAnim[playerIndex][digitIndex], 1500,
                                              0));
                HuSprPosSet(micQuizScoreDigitGroup[playerIndex][digitIndex], 0,
                            (f32)(64 + (playerIndex << 7) + (digitIndex << 5)),
                            275.0f);
                HuSprAttrSet(micQuizScoreDigitGroup[playerIndex][digitIndex], 0,
                             HUSPR_ATTR_DISPOFF);
            }

            score = micQuizModeWork->playerScore[playerIndex];
            hundreds = score / 100;
            tens = (score - (hundreds * 100)) / 10;
            ones = (score - (hundreds * 100)) - (tens * 10);
            HuSprBankSet(micQuizScoreDigitGroup[playerIndex][0], 0, hundreds);
            HuSprBankSet(micQuizScoreDigitGroup[playerIndex][1], 0, tens);
            HuSprBankSet(micQuizScoreDigitGroup[playerIndex][2], 0, ones);
        }
    }
}

/* Refresh each active player score using its three digit sprites. */
void MicQuizScoreDigitsUpdate(void) {
    s32 playerIndex;
    s32 score;
    s32 hundreds;
    s32 tens;
    s32 ones;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (MicQuizPlayerActiveCheck(playerIndex) != 0) {
            score = micQuizModeWork->playerScore[playerIndex];
            hundreds = score / 100;
            tens = (score - hundreds * 100) / 10;
            ones = score - hundreds * 100 - tens * 10;
            HuSprBankSet(micQuizScoreDigitGroup[playerIndex][0], 0, hundreds);
            HuSprBankSet(micQuizScoreDigitGroup[playerIndex][1], 0, tens);
            HuSprBankSet(micQuizScoreDigitGroup[playerIndex][2], 0, ones);
        }
    }
}

/* Create the pause display, player score rows, and question progress indicators. */
void MicQuizPauseDispCreate(void) {
    s32 animationIndex;
    s32 playerNo;
    s32 digitIndex;
    s32 score;
    s32 scoreRemainder;
    s32 hundreds;
    s32 characterIndex;

    micQuizPauseBgAnim =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 101), 0x10000000, 2));
    micQuizPauseTitleAnim =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 102), 0x10000000, 2));
    micQuizPauseTitleGroup = HuSprGrpCreate(1);
    HuSprGrpMemberSet(micQuizPauseTitleGroup, 0, HuSprCreate(micQuizPauseTitleAnim, 1500, 0));
    HuSprPosSet(micQuizPauseTitleGroup, 0, 288.0f, 64.0f);
    HuSprAttrSet(micQuizPauseTitleGroup, 0, HUSPR_ATTR_DISPOFF);
    animationIndex = 0;
    while (animationIndex < 4) {
        micQuizPauseRowAnim[animationIndex] = HuSprAnimRead(fn_80008DD0(
            animationIndex + DATANUM(0xa20000, 104), 0x10000000, 2));
        micQuizPauseRowGroup[animationIndex] = HuSprGrpCreate(1);
        HuSprGrpMemberSet(micQuizPauseRowGroup[animationIndex], 0,
                          HuSprCreate(micQuizPauseRowAnim[animationIndex], 1500, 0));
        HuSprPosSet(micQuizPauseRowGroup[animationIndex], 0, (f32)((animationIndex << 7) + 96),
                    248.0f);
        HuSprAttrSet(micQuizPauseRowGroup[animationIndex], 0, HUSPR_ATTR_DISPOFF);
        animationIndex++;
    }
    MicQuizScoreDigitsCreate();
    micQuizPauseProgressScore[0] = MgScoreInit(DATANUM(0xa20000, 120));
    HuPrcSetStat(micQuizPauseProgressScore[0]->proc, 0x4);
    MgScoreMaxDigitSet(micQuizPauseProgressScore[0], 2);
    MgScoreValueSet(micQuizPauseProgressScore[0], micQuizModeWork->questionCount);
    MgScorePosSet(micQuizPauseProgressScore[0], 186.0f, 135.0f);
    MgScoreDispOff(micQuizPauseProgressScore[0]);
    MgScorePriSet(micQuizPauseProgressScore[0], 50);
    micQuizPauseProgressAnim =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 108), 0x10000000, 2));
    micQuizPauseProgressGroup = HuSprGrpCreate(1);
    HuSprGrpMemberSet(micQuizPauseProgressGroup, 0, HuSprCreate(micQuizPauseProgressAnim, 1500, 0));
    HuSprPosSet(micQuizPauseProgressGroup, 0, 288.0f, 140.0f);
    HuSprAttrSet(micQuizPauseProgressGroup, 0, HUSPR_ATTR_DISPOFF);
    micQuizPauseProgressScore[1] = MgScoreInit(DATANUM(0xa20000, 120));
    HuPrcSetStat(micQuizPauseProgressScore[1]->proc, 0x4);
    MgScoreMaxDigitSet(micQuizPauseProgressScore[1], 2);
    MgScoreValueSet(micQuizPauseProgressScore[1], micQuizModeWork->playerCount * 5);
    MgScorePosSet(micQuizPauseProgressScore[1], 338.0f, 145.0f);
    MgScoreDispOff(micQuizPauseProgressScore[1]);
    MgScorePriSet(micQuizPauseProgressScore[1], 50);
    animationIndex = 0;
    while (animationIndex < 4) {
        if (MicQuizPlayerActiveCheck(animationIndex) != 0) {
            characterIndex = micQuizModeWork->playerCharacter[animationIndex];
            micQuizPausePortraitAnim[animationIndex] = HuSprAnimRead(fn_80008DD0(
                characterIndex + DATANUM(0xa20000, 109), 0x10000000, 2));
            micQuizPausePortraitGroup[animationIndex] = HuSprGrpCreate(1);
            HuSprGrpMemberSet(micQuizPausePortraitGroup[animationIndex], 0,
                              HuSprCreate(micQuizPausePortraitAnim[animationIndex], 1500, 0));
            HuSprPosSet(micQuizPausePortraitGroup[animationIndex], 0,
                        (f32) ((animationIndex << 7) + 95),
                        214.0f);
            HuSprAttrSet(micQuizPausePortraitGroup[animationIndex], 0, HUSPR_ATTR_DISPOFF);
        }
        animationIndex++;
    }
    micQuizPauseWin0 = HuWinExCreateFrame(16.0f, 344.0f, 544, 96, 23, 5);
    micQuizPauseWin1 = HuWinExCreateFrame(16.0f, 344.0f, 544, 96, 13, 0);
    HuWinDispOff(micQuizPauseWin0);
    HuWinPriSet(micQuizPauseWin0, 50);
    HuWinDispOff(micQuizPauseWin1);
    HuWinPriSet(micQuizPauseWin1, 50);
    micQuizHintWin = HuWinCreate(16.0f, 352.0f, 544, 128, 0);
    HuWinPosSet(micQuizHintWin, 16.0f, 352.0f);
    HuWinBGTPLvlSet(micQuizHintWin, 0.0f);
    HuWinMesSpeedSet(micQuizHintWin, 0);
    HuWinPriSet(micQuizHintWin, 50);
    HuWinAttrSet(micQuizHintWin, HUWIN_ATTR_ALIGN_CENTER);
}

/* Refresh pause-display progress values and player score digits before showing it. */
void MicQuizPauseDispUpdate(void) {
    s32 playerIndex;
    s32 hundreds;
    s32 score;
    s32 tens;
    s32 ones;

    MgScoreValueSet(micQuizPauseProgressScore[0], micQuizModeWork->questionCount);
    MgScoreValueSet(micQuizPauseProgressScore[1], micQuizModeWork->playerCount * 5);

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (MicQuizPlayerActiveCheck(playerIndex) != 0) {
            score = micQuizModeWork->playerScore[playerIndex];
            hundreds = score / 100;
            tens = (score - hundreds * 100) / 10;
            ones = score - hundreds * 100 - tens * 10;
            HuSprBankSet(micQuizScoreDigitGroup[playerIndex][0], 0, hundreds);
            HuSprBankSet(micQuizScoreDigitGroup[playerIndex][1], 0, tens);
            HuSprBankSet(micQuizScoreDigitGroup[playerIndex][2], 0, ones);
        }
    }
}

/* Show the pause display and its score widgets when the menu opens. */
void MicQuizPauseDispOn(void) {
    s32 playerIndex;
    s32 digitIndex;

    HuSprAttrReset(micQuizPauseTitleGroup, 0, HUSPR_ATTR_DISPOFF);
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        HuSprAttrReset(micQuizPauseRowGroup[playerIndex], 0, HUSPR_ATTR_DISPOFF);
    }
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (MicQuizPlayerActiveCheck(playerIndex) != 0) {
            for (digitIndex = 0; digitIndex < 3; digitIndex++) {
                HuSprAttrReset(micQuizScoreDigitGroup[playerIndex][digitIndex], 0,
                               HUSPR_ATTR_DISPOFF);
            }
        }
    }
    HuSprAttrReset(micQuizPauseProgressGroup, 0, HUSPR_ATTR_DISPOFF);
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (MicQuizPlayerActiveCheck(playerIndex) != 0) {
            HuSprAttrReset(micQuizPausePortraitGroup[playerIndex], 0, HUSPR_ATTR_DISPOFF);
        }
    }
    MgScoreDispOn(micQuizPauseProgressScore[0]);
    MgScoreDispOn(micQuizPauseProgressScore[1]);
}

/* Hide the pause display and its score widgets when the menu closes. */
void MicQuizPauseDispOff(void) {
    s32 playerIndex;
    s32 digitIndex;

    HuSprAttrSet(micQuizPauseTitleGroup, 0, HUSPR_ATTR_DISPOFF);
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        HuSprAttrSet(micQuizPauseRowGroup[playerIndex], 0, HUSPR_ATTR_DISPOFF);
    }
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        for (digitIndex = 0; digitIndex < 3; digitIndex++) {
            HuSprAttrSet(micQuizScoreDigitGroup[playerIndex][digitIndex], 0, HUSPR_ATTR_DISPOFF);
        }
    }
    HuSprAttrSet(micQuizPauseProgressGroup, 0, HUSPR_ATTR_DISPOFF);
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (MicQuizPlayerActiveCheck(playerIndex) != 0) {
            HuSprAttrSet(micQuizPausePortraitGroup[playerIndex], 0, HUSPR_ATTR_DISPOFF);
        }
    }
    MgScoreDispOff(micQuizPauseProgressScore[0]);
    MgScoreDispOff(micQuizPauseProgressScore[1]);
}

/* Return the question HUD and dialogue windows to the normal sprite draw pass. */
void MicQuizHudDrawNormal(void) {
    s32 decorationIndex;

    HuSprGrpDrawNoSet(micQuizMicHudGroup, 0);
    HuSprDrawNoSet(micQuizMicIconGroup, 0, 0);
    HuSprDrawNoSet(micQuizMicGroup, 0, 0);
    HuSprDrawNoSet(micQuizMicGlowGroup, 0, 0);
    HuSprGrpDrawNoSet(micQuizCountdownGroup[0], 0);
    if (MicQuizQuestionEventGet() != 8) {
        HuSprDrawNoSet(micQuizEventIconGroup, 0, 0);
    }
    if (micQuizTempMesWin != -1) {
        HuWinDrawNoSet(micQuizTempMesWin, 0);
    }
    HuWinDrawNoSet(micQuizDialogWin0, 0);
    HuWinDrawNoSet(micQuizDialogWin1, 0);
    HuWinDrawNoSet(micQuizDialogWin2, 0);
    HuWinDrawNoSet(micQuizDialogWin3, 0);
    HuSprDrawNoSet(micQuizIndicatorGroup, 0, 0);
    HuSprDrawNoSet(micQuizIndicatorSwingGroup, 0, 0);
    HuSprDrawNoSet(micQuizIndicatorDoneGroup, 0, 0);
    HuSprDrawNoSet(micQuizCategoryGroup, 0, 0);
    HuSprDrawNoSet(micQuizAcceptIconGroup, 0, 0);
    HuSprDrawNoSet(micQuizRejectIconGroup, 0, 0);
    HuSprDrawNoSet(micQuizBannerGroup, 0, 0);
    if (micQuizEventPulseGroup != -1) {
        HuSprDrawNoSet(micQuizEventPulseGroup, 0, 0);
    }
    if (micQuizEventBadgeGroup != -1) {
        HuSprDrawNoSet(micQuizEventBadgeGroup, 0, 0);
    }
    if (micQuizEventBannerGroup != -1) {
        HuSprDrawNoSet(micQuizEventBannerGroup, 0, 0);
    }
    for (decorationIndex = 0; decorationIndex < 4; decorationIndex++) {
        if (micQuizTwinkleGroup[decorationIndex] != -1) {
            HuSprDrawNoSet(micQuizTwinkleGroup[decorationIndex], 0, 0);
        }
    }
    if (micQuizPulseLeftGroup != -1) {
        HuSprDrawNoSet(micQuizPulseLeftGroup, 0, 0);
    }
    if (micQuizPulseRightGroup != -1) {
        HuSprDrawNoSet(micQuizPulseRightGroup, 0, 0);
    }
}

/* Move the question HUD and dialogue windows to the pause sprite draw pass. */
void MicQuizHudDrawPause(void) {
    s32 decorationIndex;

    HuSprGrpDrawNoSet(micQuizMicHudGroup, 64);
    if (micQuizTempMesWin != -1) {
        HuWinDrawNoSet(micQuizTempMesWin, 64);
    }
    HuWinDrawNoSet(micQuizDialogWin0, 64);
    HuWinDrawNoSet(micQuizDialogWin1, 64);
    HuWinDrawNoSet(micQuizDialogWin2, 64);
    HuWinDrawNoSet(micQuizDialogWin3, 64);
    HuSprDrawNoSet(micQuizMicIconGroup, 0, 64);
    HuSprDrawNoSet(micQuizMicGroup, 0, 64);
    HuSprDrawNoSet(micQuizMicGlowGroup, 0, 64);
    HuSprGrpDrawNoSet(micQuizCountdownGroup[0], 64);
    if (MicQuizQuestionEventGet() != 8) {
        HuSprDrawNoSet(micQuizEventIconGroup, 0, 64);
    }
    HuSprDrawNoSet(micQuizIndicatorGroup, 0, 64);
    HuSprDrawNoSet(micQuizIndicatorSwingGroup, 0, 64);
    HuSprDrawNoSet(micQuizIndicatorDoneGroup, 0, 64);
    HuSprDrawNoSet(micQuizCategoryGroup, 0, 64);
    HuSprDrawNoSet(micQuizAcceptIconGroup, 0, 64);
    HuSprDrawNoSet(micQuizRejectIconGroup, 0, 64);
    HuSprDrawNoSet(micQuizBannerGroup, 0, 64);
    if (micQuizEventPulseGroup != -1) {
        HuSprDrawNoSet(micQuizEventPulseGroup, 0, 64);
    }
    if (micQuizEventBadgeGroup != -1) {
        HuSprDrawNoSet(micQuizEventBadgeGroup, 0, 64);
    }
    if (micQuizEventBannerGroup != -1) {
        HuSprDrawNoSet(micQuizEventBannerGroup, 0, 64);
    }
    for (decorationIndex = 0; decorationIndex < 4; decorationIndex++) {
        if (micQuizTwinkleGroup[decorationIndex] != -1) {
            HuSprDrawNoSet(micQuizTwinkleGroup[decorationIndex], 0, 64);
        }
    }
    if (micQuizPulseLeftGroup != -1) {
        HuSprDrawNoSet(micQuizPulseLeftGroup, 0, 64);
    }
    if (micQuizPulseRightGroup != -1) {
        HuSprDrawNoSet(micQuizPulseRightGroup, 0, 64);
    }
}

/* Return the compact microphone HUD and dialogue windows to the normal draw pass. */
void MicQuizCompactHudDrawNormal(void) {
    HuSprGrpDrawNoSet(micQuizFlashGroup, 0);
    HuSprGrpDrawNoSet(micQuizMicHudGroup, 0);
    if (micQuizTempMesWin != -1) {
        HuWinDrawNoSet(micQuizTempMesWin, 0);
    }
    HuWinDrawNoSet(micQuizDialogWin0, 0);
    HuWinDrawNoSet(micQuizDialogWin1, 0);
    HuWinDrawNoSet(micQuizDialogWin2, 0);
    HuWinDrawNoSet(micQuizDialogWin3, 0);
}

/* Move the compact microphone HUD and dialogue windows to the pause draw pass. */
void MicQuizCompactHudDrawPause(void) {
    HuSprGrpDrawNoSet(micQuizFlashGroup, 64);
    HuSprGrpDrawNoSet(micQuizMicHudGroup, 64);
    if (micQuizTempMesWin != -1) {
        HuWinDrawNoSet(micQuizTempMesWin, 64);
    }
    HuWinDrawNoSet(micQuizDialogWin0, 64);
    HuWinDrawNoSet(micQuizDialogWin1, 64);
    HuWinDrawNoSet(micQuizDialogWin2, 64);
    HuWinDrawNoSet(micQuizDialogWin3, 64);
}

/* Change the response indicator animation as listening starts, completes, or stops. */
void MicQuizIndicatorSet(s32 action) {
    if (action == 0) {
        if ((u32)micQuizIndicatorSwingProc != 0) {
            HuPrcKill(micQuizIndicatorSwingProc);
            micQuizIndicatorSwingProc = NULL;
        }
        HuSprAttrSet(micQuizIndicatorGroup, 0, HUSPR_ATTR_DISPOFF);
        HuSprAttrSet(micQuizIndicatorSwingGroup, 0, HUSPR_ATTR_DISPOFF);
    } else if (action == 1) {
        HuSprAttrReset(micQuizIndicatorGroup, 0, HUSPR_ATTR_DISPOFF);
        HuSprAttrReset(micQuizIndicatorSwingGroup, 0, HUSPR_ATTR_DISPOFF);
        if ((u32)micQuizIndicatorSwingProc == 0) {
            micQuizIndicatorSwingProc = fn_8000DAC0(MicQuizIndicatorSwing, 100, 6144, 0,
                                                    micQuizParentProc);
        }
    } else if (action == 3) {
        MicQuizSparksExpand();
        HuPrcSleep(20);
        HuSprAttrSet(micQuizIndicatorGroup, 0, HUSPR_ATTR_DISPOFF);
        HuSprScaleSet(micQuizIndicatorGroup, 0, 0.0f, 0.0f);
        HuSprGrpAttrSet(micQuizIndicatorSwingGroup, HUSPR_ATTR_DISPOFF);
        HuSprGrpAttrReset(micQuizIndicatorDoneGroup, HUSPR_ATTR_DISPOFF);
    } else if (action == 2) {
        if ((u32)micQuizIndicatorSwingProc != 0) {
            HuPrcKill(micQuizIndicatorSwingProc);
            micQuizIndicatorSwingProc = NULL;
        }
        MicQuizSparksSmall();
        HuPrcSleep(30);
        HuSprAttrSet(micQuizIndicatorSwingGroup, 0, HUSPR_ATTR_DISPOFF);
        HuSprAttrSet(micQuizIndicatorDoneGroup, 0, HUSPR_ATTR_DISPOFF);
        HuSprScaleSet(micQuizIndicatorSwingGroup, 0, 0.0f, 0.0f);
    } else if (action == 4) {
        if ((u32)micQuizIndicatorSwingProc != 0) {
            HuPrcKill(micQuizIndicatorSwingProc);
            micQuizIndicatorSwingProc = NULL;
        }
        HuAudFXPlay(MSM_SE_BRD00_PRIZE_SPARKLE);
        MicQuizSparksExpand();
        MicQuizSparksFade();
        HuPrcSleep(30);
        HuSprAttrSet(micQuizIndicatorGroup, 0, HUSPR_ATTR_DISPOFF);
        HuSprScaleSet(micQuizIndicatorGroup, 0, 0.0f, 0.0f);
        HuSprAttrSet(micQuizIndicatorSwingGroup, 0, HUSPR_ATTR_DISPOFF);
        HuSprScaleSet(micQuizIndicatorSwingGroup, 0, 0.0f, 0.0f);
    }
}

/* Launch twenty expanding sparks around the microphone response indicator. */
void MicQuizSparksExpand(void) {
    s32 emissionIndex;

    emissionIndex = 0;
    while (emissionIndex < 20) {
        fn_8000DAC0(MicQuizSparkExpand, 100U, 6144U, 0, micQuizParentProc);
        emissionIndex += 1;
    }
}

/* Launch ten fading sparks around the microphone response indicator. */
void MicQuizSparksFade(void) {
    s32 emissionIndex;

    emissionIndex = 0;
    while (emissionIndex < 10) {
        fn_8000DAC0(MicQuizSparkFadeWide, 100U, 6144U, 0, fn_8000DD9C());
        emissionIndex += 1;
    }
}

/* Launch ten smaller sparks around the microphone response indicator. */
void MicQuizSparksSmall(void) {
    s32 emissionIndex;

    emissionIndex = 0;
    while (emissionIndex < 10) {
        fn_8000DAC0(MicQuizSparkFadeSmall, 100U, 6144U, 0, fn_8000DD9C());
        emissionIndex += 1;
    }
}

/* Show one expanding response spark after a random delay, then remove it. */
void MicQuizSparkExpand(void) {
    s16 sleep;
    ANIMDATA *animation;
    HUSPR_GROUPID group;
    f32 transparency;
    f32 step;
    f32 rotation;
    f32 scale;
    f32 position[2];

    animation = HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 34), 0x10000000, 2));
    group = HuSprGrpCreate(1);
    HuSprGrpMemberSet(group, 0, HuSprCreate(animation, 50, 0));
    step = 3.0f;
    position[0] =
        (f32) (sin((u8) frand() * 1.4117647f * M_PI / 180.0) * 25.0 + 506.0);
    position[1] =
        (f32) (cos((u8) frand() * 1.4117647f * M_PI / 180.0) * 25.0 + 65.0);
    HuSprGrpPosSet(group, position[0], position[1]);
    HuSprScaleSet(group, 0, 0.0f, 0.0f);
    rotation = 0.0f;
    scale = 0.0f;
    transparency = 1.0f;
    sleep = frandmod(20);
    HuPrcSleep(sleep);
    for (;;) {
        if (micQuizModeWork->pauseDisplayActive != 0) {
            HuSprGrpDrawNoSet(group, 64);
        } else {
            HuSprGrpDrawNoSet(group, 0);
        }
        HuPrcVSleep();
        scale = (f32)(sin(M_PI * rotation / 180.0) * 1.5);
        transparency = (f32)(1.0 - sin(M_PI * rotation / 180.0));
        HuSprZRotSet(group, 0, scale);
        HuSprScaleSet(group, 0, scale, scale);
        HuSprTPLvlSet(group, 0, transparency);
        rotation += step;
        if (rotation >= 90.0f) {
            break;
        }
    }
    HuSprGrpKill(group);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Show one fading response spark over a wide random radius, then remove it. */
void MicQuizSparkFadeWide(void) {
    s16 delay;
    ANIMDATA *animation;
    s16 spriteGroup;
    f32 step;
    f32 initialPosition[2];
    f32 angle;
    f32 rotation;
    f32 transparency;

    animation =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 35), 0x10000000, 2));
    spriteGroup = HuSprGrpCreate(1);
    HuSprGrpMemberSet(spriteGroup, 0, HuSprCreate(animation, 50, 0));
    step = 3.0f;
    initialPosition[0] = (f32)(504.0 +
                               (45.0 *
                                sin((M_PI *
                                     (f64)(1.4117647f * (f32)(u8)frand())) /
                                    180.0)));
    initialPosition[1] = 99.0f;
    HuSprGrpPosSet(spriteGroup, initialPosition[0], initialPosition[1]);
    HuSprScaleSet(spriteGroup, 0, 0.0f, 0.0f);
    angle = 0.0f;
    rotation = 0.0f;
    transparency = 1.0f;
    delay = frandmod(20);
    HuPrcSleep(delay);
    do {
        if (micQuizModeWork->pauseDisplayActive != 0) {
            HuSprGrpDrawNoSet(spriteGroup, 64);
        } else {
            HuSprGrpDrawNoSet(spriteGroup, 0);
        }
        HuPrcVSleep();
        rotation = (f32)(1.5 *
                         sin((M_PI * (f64)angle) / 180.0));
        transparency = (f32)(1.0 -
                             sin((M_PI * (f64)angle) / 180.0));
        HuSprZRotSet(spriteGroup, 0, rotation);
        HuSprScaleSet(spriteGroup, 0, rotation, rotation);
        HuSprTPLvlSet(spriteGroup, 0, transparency);
        angle += step;
    } while (!(angle >= 90.0f));
    HuSprGrpKill(spriteGroup);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Show one fading response spark over a small random radius, then remove it. */
void MicQuizSparkFadeSmall(void) {
    s16 delay;
    ANIMDATA *animation;
    s16 spriteGroup;
    f32 step;
    f32 initialPosition[2];
    f32 angle;
    f32 rotation;
    f32 transparency;

    animation =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 35), 0x10000000, 2));
    spriteGroup = HuSprGrpCreate(1);
    HuSprGrpMemberSet(spriteGroup, 0, HuSprCreate(animation, 50, 0));
    step = 3.0f;
    initialPosition[0] = (f32)(504.0 +
                               (25.0 *
                                sin((M_PI *
                                     (f64)(1.4117647f * (f32)(u8)frand())) /
                                    180.0)));
    initialPosition[1] = 99.0f;
    HuSprGrpPosSet(spriteGroup, initialPosition[0], initialPosition[1]);
    HuSprScaleSet(spriteGroup, 0, 0.0f, 0.0f);
    angle = 0.0f;
    rotation = 0.0f;
    transparency = 1.0f;
    delay = frandmod(20);
    HuPrcSleep(delay);
    do {
        if (micQuizModeWork->pauseDisplayActive != 0) {
            HuSprGrpDrawNoSet(spriteGroup, 64);
        } else {
            HuSprGrpDrawNoSet(spriteGroup, 0);
        }
        HuPrcVSleep();
        rotation = (f32)(1.5 *
                         sin((M_PI * (f64)angle) / 180.0));
        transparency = (f32)(1.0 -
                             sin((M_PI * (f64)angle) / 180.0));
        HuSprZRotSet(spriteGroup, 0, rotation);
        HuSprScaleSet(spriteGroup, 0, rotation, rotation);
        HuSprTPLvlSet(spriteGroup, 0, transparency);
        angle += step;
    } while (!(angle >= 90.0f));
    HuSprGrpKill(spriteGroup);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Emit a microphone activity ring every twenty frames while this process runs. */
void MicQuizRingEmit(void) {
    for (;;) {
        fn_8000DAC0(MicQuizRing, 100U, 6144U, 0, micQuizParentProc);
        HuPrcSleep(20);
    }
}

/* Shrink one microphone activity ring while increasing its opacity, then remove its sprite
 * group. Select the draw pass from the current pause display state. */
void MicQuizRing(void) {
    ANIMDATA *animation;
    s16 spriteGroup;
    f32 scalePhase;
    f32 transparencyPhase;
    f32 scale;
    f32 transparency;

    animation =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 31), 0x10000000, 2));
    spriteGroup = HuSprGrpCreate(1);
    HuSprGrpMemberSet(spriteGroup, 0, HuSprCreate(animation, 47, 0));
    HuSprPosSet(spriteGroup, 0, 516.0f, 362.0f);
    HuSprTPLvlSet(spriteGroup, 0, 0.0f);

    scalePhase = 90.0f;
    transparencyPhase = 0.0f;
    do {
        if (micQuizModeWork->pauseDisplayActive != 0) {
            HuSprGrpDrawNoSet(spriteGroup, 64);
        } else {
            HuSprGrpDrawNoSet(spriteGroup, 0);
        }
        HuPrcVSleep();
        scale = (f32)sin((M_PI * (f64)scalePhase) / 180.0);
        transparency = (f32)sin((M_PI * (f64)transparencyPhase) / 180.0);
        HuSprScaleSet(spriteGroup, 0, scale, scale);
        HuSprTPLvlSet(spriteGroup, 0, transparency);
        scalePhase += 2.0f;
        transparencyPhase += 2.0f;
        if (transparency >= 1.0f) {
            transparency = 1.0f;
        }
    } while (!(scalePhase >= 180.0f));
    HuSprGrpKill(spriteGroup);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Choose the accepted or rejected feedback icon and start its flashing process. */
void MicQuizFeedbackStart(s32 accepted) {
    if (accepted)
    {
        HuAudFXPlay(MSM_SE_CMN_SELECTION_ACCEPT);
        micQuizFeedbackGroup = micQuizAcceptIconGroup;
        MicQuizMicHudMotionSet(3);
    }
    else
    {
        HuAudFXPlay(MSM_SE_CMN_SELECTION_REJECT);
        micQuizFeedbackGroup = micQuizRejectIconGroup;
    }
    fn_8000DAC0(MicQuizFeedbackFlash, 100U, 6144U, 0, micQuizParentProc);
}

/* Flash the selected answer feedback icon twice before ending this process. */
void MicQuizFeedbackFlash(void) {
    HuSprAttrReset(micQuizFeedbackGroup, 0, HUSPR_ATTR_DISPOFF);
    HuPrcSleep(30);
    HuSprAttrSet(micQuizFeedbackGroup, 0, HUSPR_ATTR_DISPOFF);
    HuPrcSleep(30);
    HuSprAttrReset(micQuizFeedbackGroup, 0, HUSPR_ATTR_DISPOFF);
    HuPrcSleep(30);
    HuSprAttrSet(micQuizFeedbackGroup, 0, HUSPR_ATTR_DISPOFF);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Pulse the microphone HUD while a response is being requested. */
void MicQuizMicHudPulse(void) {
    s32 frame = 0;
    f32 angle = 0.0f;

    for (; ; frame++) {
        f32 scale;

        HuPrcVSleep();
        scale = (f32)(1.0 + (0.3f *
                 sin((M_PI * (f64)angle) / 180.0)));
        HuSprScaleSet(micQuizMicHudGroup, 1, scale, scale);
        angle += 5.0f;
        if (angle >= 180.0f) {
            angle = 0.0f;
        }
    }
}

/* Play the event icon effect selected by the question type and answer result. */
void MicQuizEventIconEffect(s32 accepted, s32 questionType) {
    s32 index;

    if (questionType == 0 || questionType == 1) {
        if (accepted != 0) {
            for (index = 0; index < 15; index++) {
                fn_8000DAC0(MicQuizEventSpark, 100U, 6144U, 0, micQuizParentProc);
            }
            MicQuizEventIconExpand();
            return;
        }
        MicQuizEventIconShrink();
        return;
    }
    if (questionType == 3) {
        if (accepted != 0) {
            MicQuizEventIconShrink();
            return;
        }
        MicQuizEventIconBreak();
        return;
    }
    if (questionType == 6) {
        if (accepted != 0) {
            for (index = 0; index < 15; index++) {
                fn_8000DAC0(MicQuizEventSpark, 100U, 6144U, 0, micQuizParentProc);
            }
            MicQuizEventIconExpand();
            return;
        }
        MicQuizEventIconShrink();
    }
}

/* Break apart the event icon, fade its overlay, and hide it during a white wipe. */
void MicQuizEventIconBreak(void) {
    f32 angle = 0.0f;
    f32 transparency;
    s32 index;

    HuAudFXPlay(MSM_SE_MICQUIZ_EVENT_FRAGMENT);
    for (index = 0; index < 15; index++) {
        fn_8000DAC0(MicQuizEventFragment, 100U, 6144U, 0, micQuizParentProc);
    }
    HuPrcSleep(45);
    HuSprTPLvlSet(micQuizEventOverlayGroup, 0, 0.0f);
    HuSprAttrReset(micQuizEventOverlayGroup, 0, HUSPR_ATTR_DISPOFF);
    do {
        if (micQuizModeWork->pauseDisplayActive != 0) {
            HuSprGrpDrawNoSet(micQuizEventOverlayGroup, 64);
        } else {
            HuSprGrpDrawNoSet(micQuizEventOverlayGroup, 0);
        }
        HuPrcVSleep();
        transparency = (f32)sin((M_PI * (f64)angle) / 180.0);
        HuSprTPLvlSet(micQuizEventOverlayGroup, 0, transparency);
        angle += 18.0f;
    } while (!(angle >= 90.0f));
    MicQuizPauseAllowSet(0);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_WHITE, 5);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    HuSprAttrSet(micQuizEventIconGroup, 0, HUSPR_ATTR_DISPOFF);
    HuSprAttrSet(micQuizEventOverlayGroup, 0, HUSPR_ATTR_DISPOFF);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_WHITE, 5);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    MicQuizPauseAllowSet(1);
}

/* Move one event fragment toward the icon center and change its animation in flight. */
void MicQuizEventFragment(void) {
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
    randomAngle = 2.0f * frandf();
    randomRadius = frandf();
    startDelay = frandmod(10);
    particleGroup = HuSprGrpCreate(1);
    HuSprGrpMemberSet(particleGroup, 0, HuSprCreate(micQuizEventFragmentAnim[0], 49, 0));
    HuSprAttrSet(particleGroup, 0, 4);
    HuSprAttrSet(particleGroup, 0, 1);
    HuPrcSleep(startDelay);
    originX = 401.0f;
    originY = 62.0f;
    positionX =
    (f32) ((f64) originX +
        (35.0 * ((f64) randomRadius * sin(M_PI * (f64) randomAngle))));
    positionY =
    (f32) ((f64) originY +
        (35.0 * ((f64) randomRadius * cos(M_PI * (f64) randomAngle))));
    stepX = (originX - positionX) / 45.0f;
    stepY = (originY - positionY) / 45.0f;
    HuSprAttrReset(particleGroup, 0, 4);
    HuSprAttrReset(particleGroup, 0, 1);
    frame = 0;
    while (frame < 45)
    {
        if ((s32) micQuizModeWork->pauseDisplayActive != 0)
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
            HuSprGrpMemberSet(particleGroup, 0, HuSprCreate(micQuizEventFragmentAnim[1], 49, 0));
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

/* Expand and fade the event icon after an accepted response. */
void MicQuizEventIconExpand(void) {
    f32 transparency;
    f32 scale;
    f32 angle;

    scale = 1.0f;
    transparency = 1.0f;
    angle = 0.0f;
    do {
        HuPrcVSleep();
        HuSprScaleSet(micQuizEventIconGroup, 0, scale, scale);
        HuSprTPLvlSet(micQuizEventIconGroup, 0, transparency);
        scale = (f32) (1.0 + sin((M_PI * (f64) angle) / 180.0));
        transparency = (f32) (1.0 - sin((M_PI * (f64) angle) / 180.0));
        angle += 1.5f;
    } while (!(angle >= 90.0f));
    HuSprAttrSet(micQuizEventIconGroup, 0, HUSPR_ATTR_DISPOFF);
}

/* Shrink and fade the event icon when the response removes it. */
void MicQuizEventIconShrink(void) {
    f32 transparency;
    f32 scale;
    f32 angle;

    scale = 1.0f;
    transparency = 1.0f;
    angle = 0.0f;
    do {
        HuPrcVSleep();
        HuSprScaleSet(micQuizEventIconGroup, 0, scale, scale);
        HuSprTPLvlSet(micQuizEventIconGroup, 0, transparency);
        scale = (f32) (1.0 - sin((M_PI * (f64) angle) / 180.0));
        transparency = (f32) (1.0 - sin((M_PI * (f64) angle) / 180.0));
        angle += 3.0f;
    } while (!(angle >= 90.0f));
    HuSprAttrSet(micQuizEventIconGroup, 0, HUSPR_ATTR_DISPOFF);
}

void MicQuizEmptyA(void)
{
}

void MicQuizEmptyB(void)
{
}

/* Return the selected question level to the round process. */
u8 MicQuizQuestionLevelGet(void) {
    return micQuizModeWork->questionLevel;
}

/* Return the selected question column to the round process. */
u8 MicQuizQuestionColumnGet(void) {
    return micQuizModeWork->questionColumn;
}

/* Store the answering player, remaining points, and result for the round controller. */
void MicQuizRoundResultSet(s8 playerNo, u16 points, s32 result) {
    micQuizModeWork->answeringPlayer = playerNo;
    micQuizModeWork->roundPoints = points;
    micQuizModeWork->roundResult = result;
}

/* Return the player whose turn selected the question. */
s32 MicQuizCurrentPlayerGet(void) {
    return micQuizModeWork->currentPlayer;
}

/* Return whether the requested player participates in this quiz. */
s32 MicQuizPlayerActiveCheck(s32 playerNo) {
    return micQuizModeWork->playerActive[playerNo];
}

/* Return the remaining answer time as a fraction of the selected question allowance. */
f32 MicQuizTimeLeftGet(void) {
    s32 selectedIndex;

    if (MicQuizQuestionEventGet() == 2) {
        selectedIndex = MicQuizQuestionLevelGet() + 1;
    } else if (MicQuizQuestionEventGet() == 5) {
        selectedIndex = MicQuizQuestionLevelGet() - 1;
    } else {
        selectedIndex = MicQuizQuestionLevelGet();
    }
    return ((f32)micQuizCountdownValue
            - ((f32)micQuizCountdownFrame / (f32)micQuizCountdownStepFrames)) /
           (f32)micQuizCountdownStarts[selectedIndex];
}

/* Wait two seconds between quiz presentation steps. */
void MicQuizWaitTwoSeconds(void) {
    HuPrcSleep(120);
}

/* Wait one second between quiz presentation steps. */
void MicQuizWaitOneSecond(void) {
    HuPrcSleep(60);
}

/* Return which dialogue layout the current quiz mode uses. */
s32 MicQuizPresenterLayoutGet(void) {
    return micQuizModeWork->presenterLayout;
}

/* Return the current question event to its HUD and timing helpers. */
s32 MicQuizQuestionEventGet(void) {
    return micQuizModeWork->questionEvent;
}

/* Request the round presentation transition and mark it unfinished. */
void MicQuizPresentationRequest(void) {
    micQuizModeWork->presentationBusy = 1;
}

/* Mark the requested round presentation transition as finished. */
void MicQuizPresentationDone(void) {
    micQuizModeWork->presentationBusy = 0;
}

/* Present the current player name in the dialogue layout used by this mode. */
void MicQuizPlayerNamePresent(void) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        MicQuizWinInsertMesSet(0,
                micQuizCharaNameMes[
                        micQuizModeWork->playerCharacter[micQuizModeWork->currentPlayer]],
                   0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 13), 1, 0);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinInsertMesSet(3,
                micQuizCharaNameMes[
                        micQuizModeWork->playerCharacter[micQuizModeWork->currentPlayer]],
                   0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 76), 1, 3);
    }
    MicQuizWinMesWait();
}

/* Insert a supplied message into the first response prompt for this dialogue layout. */
void MicQuizPromptPresent(u32 message) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(1);
        MicQuizWinInsertMesSet(1, message, 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 16), 1, 1);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinInsertMesSet(3, message, 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 79), 1, 3);
    }
    MicQuizWinMesWait();
}

/* Insert a supplied message into the alternate response prompt for this layout. */
void MicQuizPromptAltPresent(u32 message) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(1);
        MicQuizWinInsertMesSet(1, message, 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 27), 1, 1);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinInsertMesSet(3, message, 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 83), 1, 3);
    }
    MicQuizWinMesWait();
}

/* Present the next response prompt unless the answer timer has expired. */
void MicQuizNextPromptPresent(void) {
    OSReport("SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSScoreMode:%d\n", micQuizCountdownState);
    if (micQuizCountdownState != 2) {
        if (micQuizModeWork->presenterLayout == 0) {
            MicQuizWinOpen(1);
            MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 48), 1, 1);
        } else {
            MicQuizWinOpen(3);
            MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 102), 1, 3);
        }
        MicQuizWinMesWait();
    }
}

/* Present the timeout follow-up using the current dialogue layout. */
void MicQuizTimeoutFollowupPresent(void) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(1);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 17), 1, 1);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 80), 1, 3);
    }
    MicQuizWinMesWait();
}

/* Present the response follow-up using the current dialogue layout. */
void MicQuizResponseFollowupPresent(void) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 25), 1, 0);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 81), 1, 3);
    }
    MicQuizWinMesWait();
}

/* Present a named-player response message unless the answer timer has expired. */
void MicQuizPlayerResponsePresent(s32 playerIndex)
{
    if (micQuizCountdownState != 2) {
        if (micQuizModeWork->presenterLayout == 0) {
            MicQuizWinOpen(0);
            MicQuizWinInsertMesSet(0,
                    micQuizCharaNameMes[micQuizModeWork->playerCharacter[playerIndex]], 0);
            MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 26), 1, 0);
        } else {
            MicQuizWinOpen(3);
            MicQuizWinInsertMesSet(3,
                    micQuizCharaNameMes[micQuizModeWork->playerCharacter[playerIndex]], 0);
            MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 82), 1, 3);
        }
        MicQuizWinMesWait();
    }
}

/* Present the next quiz response message using the current dialogue layout. */
void MicQuizNextResponsePresent(void) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(1);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 28), 1, 1);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 84), 1, 3);
    }
    MicQuizWinMesWait();
}

/* Present an inserted response message, wait for it, and close the dialogue. */
void MicQuizResponseMesPresent(u32 message) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        MicQuizWinInsertMesSet(0, message, 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 29), 1, 0);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinInsertMesSet(3, message, 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 85), 1, 3);
    }
    MicQuizWinMesWait();
    MicQuizWinClose();
}

/* Present a response follow-up using the current dialogue layout. */
void MicQuizFollowupPresent(void) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(2);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 40), 1, 2);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 96), 1, 3);
    }
    MicQuizWinMesWait();
}

/* Present the alternate response follow-up using the current dialogue layout. */
void MicQuizFollowupAltPresent(void) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(1);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 49), 1, 1);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 103), 1, 3);
    }
    MicQuizWinMesWait();
}

/* Present the next round follow-up using the current dialogue layout. */
void MicQuizRoundFollowupNextPresent(void) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(1);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 50), 1, 1);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 104), 1, 3);
    }
    MicQuizWinMesWait();
}

/* Present the round follow-up unless the answer timer has expired. */
void MicQuizRoundFollowupPresent(void) {
    if (micQuizCountdownState != 2) {
        if (micQuizModeWork->presenterLayout == 0) {
            MicQuizWinOpen(0);
            MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 55), 1, 0);
        } else {
            MicQuizWinOpen(3);
            MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 114), 1, 3);
        }
        MicQuizWinMesWait();
    }
}

/* Present the question introduction using the current dialogue layout. */
void MicQuizQuestionIntroPresent(void) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_Q, 2), 1, 0);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_Q, 42), 1, 3);
    }
    MicQuizWinMesWait();
}

/* Present two dialogue lines with a delay before naming the selected player. */
void MicQuizPlayerCallPresent(s32 messageIndex) {
    MicQuizWinOpen(0);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_I, 20), 1, 0);
    MicQuizWinMesWait();
    HuPrcSleep(120);
    MicQuizWinOpen(1);
    MicQuizWinInsertMesSet(1, micQuizCharaNameMes[micQuizModeWork->playerCharacter[messageIndex]],
                           0);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_I, 4), 1, 1);
    MicQuizWinMesWait();
}

/* Initialize microphone recognition and load the requested response context. */
void MicQuizMicInit(char *path, s32 questionType) {
    micQuizRecognitionType = questionType;
    HuMCInit(0);
    HuMCSelWinCreate(-10000.0f, 88.0f);
    micQuizMicContext = HuMCContextCreate(path);
}

/* Choose the microphone recognition context for the current question type. */
void MicQuizMicContextSet(s32 questionType) {
    micQuizRecognitionType = questionType;
    HuMCInit(0);
    HuMCSelWinCreate(-10000.0f, 88.0f);
    if (questionType == 0) {
        micQuizMicContext = HuMCContextCreate("/mic/ctx/micquiz_num");
    } else if (questionType == 1) {
        micQuizMicContext = HuMCContextCreate("/mic/ctx/micquiz_num2");
    } else if (questionType == 2) {
        micQuizMicContext = HuMCContextCreate("/mic/ctx/micquiz_chara");
    } else if (questionType == 3) {
        micQuizMicContext = HuMCContextCreate("/mic/ctx/micquiz_chara");
    } else if (questionType == 4) {
        micQuizMicContext = HuMCContextCreate("/mic/ctx/micquiz_npc");
    } else if (questionType == 5) {
        micQuizMicContext = HuMCContextCreate("/mic/ctx/micquiz_kurabe1");
    } else if (questionType == 6) {
        micQuizMicContext = HuMCContextCreate("/mic/ctx/micquiz_fruit");
    } else {
        micQuizMicContext = HuMCContextCreate("/mic/ctx/micquiz_chara");
    }
}

/* Remember the current process as the parent of quiz response effects. */
void MicQuizEffectParentSet(void) {
    micQuizParentProc = fn_8000DD9C();
}

/* Remember the current process as the parent of quiz response effects. */
void MicQuizEffectParentSetAlt(void) {
    micQuizParentProc = fn_8000DD9C();
}

/* Run question responses, retries, and participant changes until the round result is ready. */
void MicQuizResponseRun(s32 quizMode) {
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
        if (micQuizExpectedAnswer >= 9) {
            micQuizExpectedAnswer -= 1;
        }
    } else if (quizMode == 4) {
        if (micQuizExpectedAnswer >= 15) {
            micQuizExpectedAnswer -= 1;
        }
    }
    micQuizQuestionReady = 0;
    micQuizResponseReading = 0;
    micQuizResponseMode = quizMode;
    micQuizListenReady = 0;
    micQuizRetryPrompted = 0;
    micQuizFirstAttempt = 1;
    micQuizTimeoutHandled = 0;
    MicQuizPresentationDone();
    micQuizModeWork->answerPlayer[0] = (s8)(micQuizAnswerPlayer = MicQuizCurrentPlayerGet());
    MicQuizSelectionItemsSet(micQuizRecognitionType, micQuizAnswerPlayer);
    MicQuizMarkerHide();
    MicQuizMicHudStateSet(0);
    MicQuizPauseAllowSet(0);
    micQuizCountdownState = 1;
    do {
        HuPrcVSleep();
    } while (micQuizQuestionReady == 0);
    MicQuizMicHudMotionSet(0);
    micQuizResponseState = 0;
    HuPrcVSleep();
    micQuizResponseState = 1;
    HuPrcSleep(30);
    micQuizCountdownState = 0;
    MicQuizIndicatorSet(1);
    MicQuizMarkerHide();
    MicQuizMicHudStateSet(1);
    motionProcess = fn_8000DAC0(MicQuizTimeoutWatch, 100U, 6144U, 0, micQuizParentProc);
    inputProcess = fn_8000DAC0(MicQuizListenWatch, 100U, 6144U, 0, micQuizParentProc);
    MicQuizPauseAllowSet(1);
    for (;;) {
        HuPrcVSleep();
        if (HuMCSelWinCheck()) {
            MicQuizInstructionSet(1);
        }
        micQuizResponseReading = 1;
        micQuizResponseResult = MicQuizResponseRead(micQuizAnswerPlayer, quizMode);
        MicQuizPauseAllowSet(1);
        micQuizResponseReading = 0;
        MicQuizMarkerHide();
        if (HuMCSelWinCheck() != 0) {
            HuWinHomeClear(micQuizInstructionWin);
        }
        if (micQuizCountdownState == 2) {
            for (;;) {
                HuPrcVSleep();
            }
        }
        MicQuizMicHudStateSet(0);
        if (micQuizResponseResult == 0) {
            if (micQuizRetryPrompted == 0) {
                MicQuizBannerSlideOut();
                micQuizResponseState = 2;
                MicQuizIndicatorSet(3);
                MicQuizQuestionIntroPresent();
                HuPrcVSleep();
                for (;;) {
                    HuPrcVSleep();
                    if (micQuizListenReady != 0) {
                        MicQuizIndicatorSet(2);
                        micQuizRetryPrompted = 1;
                        micQuizFirstAttempt = 0;
                        micQuizCountdownState = 0;
                        MicQuizMarkerHide();
                        MicQuizMicHudStateSet(1);
                        MicQuizMicHudMotionSet(2);
                        break;
                    } else {
                        MicQuizBannerSlideOut();
                        continue;
                    }
                }
                MicQuizPlayerResponsePresent(micQuizAnswerPlayer);
                MicQuizMicHudStateSet(1);
            } else {
                MicQuizRoundFollowupNextPresent();
                HuPrcSleep(120);
                MicQuizPlayerResponsePresent(micQuizAnswerPlayer);
                MicQuizMicHudStateSet(1);
            }
            continue;
        } else if (micQuizResponseResult == 1) {
            micQuizCountdownState = 1;
            micQuizFirstAttempt = 0;
            if (micQuizListenReady == 0) {
                MicQuizIndicatorSet(4);
            }
            HuAudFXPlay(MSM_SE_CMN_SELECTION_ACCEPT);
            micQuizFeedbackGroup = micQuizAcceptIconGroup;
            MicQuizMicHudMotionSet(3);
            fn_8000DAC0(MicQuizFeedbackFlash, 100U, 6144U, 0, micQuizParentProc);
            micQuizAnswerAccepted = 1;
            MicQuizEventIconEffect(1, MicQuizQuestionEventGet());
            micQuizModeWork->answerResult[0] = 0;
            break;
        } else if (micQuizResponseResult == 2) {
            if ((MicQuizQuestionEventGet() == 3) && (hasSelection == 0)) {
                MicQuizEventIconEffect(0, 3U);
                micQuizCountdownState = 1;
                hasSelection = 1;
                MicQuizPlayerCallPresent(micQuizAnswerPlayer);
                HuSprAttrSet(micQuizEventIconGroup, 0, 4);
                MicQuizMicHudStateSet(1);
                continue;
            }
            if (hasSelection == 0) {
                micQuizCountdownState = 1;
                micQuizFirstAttempt = 0;
                HuAudFXPlay(MSM_SE_CMN_SELECTION_REJECT);
                micQuizFeedbackGroup = micQuizRejectIconGroup;
                fn_8000DAC0(MicQuizFeedbackFlash, 100U, 6144U, 0, micQuizParentProc);
                MicQuizAnswerMesPresent(quizMode, 0);
                HuPrcSleep(120);
                micQuizAnswerAccepted = 0;
                micQuizModeWork->answerResult[0] = 1;
                if (micQuizListenReady == 0) {
                    MicQuizIndicatorSet(4);
                }
            } else {
                micQuizCountdownState = 1;
                micQuizFirstAttempt = 0;
                HuAudFXPlay(MSM_SE_CMN_SELECTION_REJECT);
                micQuizFeedbackGroup = micQuizRejectIconGroup;
                fn_8000DAC0(MicQuizFeedbackFlash, 100U, 6144U, 0, micQuizParentProc);
                MicQuizAnswerMesPresent(quizMode, 1);
                HuPrcSleep(60);
                micQuizAnswerAccepted = 0;
                micQuizModeWork->answerResult[0] = 1;
                if (micQuizListenReady == 0) {
                    MicQuizIndicatorSet(4);
                }
                break;
            }
        } else if (micQuizResponseResult == -2) {
            MicQuizNextPromptPresent();
            HuPrcSleep(120);
            MicQuizPlayerResponsePresent(micQuizAnswerPlayer);
            MicQuizMicHudStateSet(1);
            continue;
        } else if (micQuizResponseResult == -3) {
            MicQuizRoundFollowupPresent();
            HuPrcSleep(120);
            MicQuizPlayerResponsePresent(micQuizAnswerPlayer);
            MicQuizMicHudStateSet(1);
            continue;
        } else if (micQuizResponseResult == -4) {
            MicQuizPlayerResponsePresent(micQuizAnswerPlayer);
            MicQuizMicHudStateSet(1);
            continue;
        } else {
            continue;
        }
        micQuizFirstAttempt = 0;
        if (HuMCSelWinCheck() != 0) {
            MicQuizInstructionSet(3);
        }
        HuPrcVSleep();
        MicQuizMarkerShow();
        MicQuizMicHudStateSet(2);
        MicQuizPortraitHide(micQuizAnswerPlayer);
        MicQuizBannerSlideOut();
        MicQuizMicHudMotionSet(2);
        micQuizCountdownState = 0;
        HuPrcKill(motionProcess);
        MicQuizResponseFollowupPresent();
        for (;;) {
            HuPrcVSleep();
            if (micQuizCountdownState == 2) {
                MicQuizTimeoutFollowupPresent();
                HuPrcSleep(60);
                micQuizAnswerAccepted = 0;
                micQuizModeWork->answerResult[1] = 2;
                break;
            } else {
                for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                    if (micQuizModeWork->playerActive[playerIndex] != 0 &&
                        playerIndex != MicQuizCurrentPlayerGet() &&
                        (lbl_802C0008[playerIndex] & 0x100) != 0) {
                        HuAudFXPlay(MSM_SE_MICQUIZ_PLAYER_BUZZ_IN);
                        micQuizModeWork->answerPlayer[1] = (s8)(micQuizAnswerPlayer = playerIndex);
                        micQuizCountdownState = 1;
                        MicQuizMarkerHide();
                        MicQuizMicHudStateSet(1);
                        MicQuizPortraitHide(MicQuizCurrentPlayerGet());
                        MicQuizPortraitShow(micQuizAnswerPlayer);
                        MicQuizBannerSlideIn();
                        MicQuizPlayerResponsePresent(micQuizAnswerPlayer);
                        MicQuizSelectionItemsSet(micQuizRecognitionType, micQuizAnswerPlayer);
                        /* Stop scanning as soon as a player buzzes in. */
                        goto answer_buzzing_player;
                    }
                }
                continue;
            answer_buzzing_player:
                if (HuMCSelWinCheck() != 0) {
                    MicQuizInstructionSet(4);
                }
                for (;;) {
                    micQuizResponseReading = 1;
                    micQuizResponseResult = MicQuizResponseRead(playerIndex, quizMode);
                    MicQuizPauseAllowSet(1);
                    micQuizResponseReading = 0;
                    if (HuMCSelWinCheck() != 0) {
                        HuWinHomeClear(micQuizInstructionWin);
                    }
                    MicQuizMicHudStateSet(0);
                    if (micQuizResponseResult == 0) {
                        MicQuizRoundFollowupNextPresent();
                        HuPrcSleep(120);
                        if (HuMCSelWinCheck() != 0) {
                            MicQuizInstructionSet(4);
                        }
                        MicQuizPlayerResponsePresent(micQuizAnswerPlayer);
                        MicQuizMicHudStateSet(1);
                        continue;
                    }
                    else if (micQuizResponseResult == 1) {
                        micQuizCountdownState = 1;
                        micQuizAnswerAccepted = 1;
                        HuAudFXPlay(MSM_SE_CMN_SELECTION_ACCEPT);
                        micQuizFeedbackGroup = micQuizAcceptIconGroup;
                        MicQuizMicHudMotionSet(3);
                        fn_8000DAC0(MicQuizFeedbackFlash, 100U, 6144U, 0, micQuizParentProc);
                        micQuizModeWork->answerResult[1] = 0;
                        MicQuizEventIconEffect(1, MicQuizQuestionEventGet());
                        break;
                    } else if (micQuizResponseResult == 2) {
                        micQuizCountdownState = 1;
                        HuAudFXPlay(MSM_SE_CMN_SELECTION_REJECT);
                        micQuizFeedbackGroup = micQuizRejectIconGroup;
                        fn_8000DAC0(MicQuizFeedbackFlash, 100U, 6144U, 0, micQuizParentProc);
                        MicQuizAnswerMesPresent(quizMode, 1);
                        HuPrcSleep(60);
                        micQuizAnswerAccepted = 0;
                        micQuizModeWork->answerResult[1] = 1;
                        MicQuizEventIconEffect(1, MicQuizQuestionEventGet());
                        break;
                    } else {
                        if (micQuizResponseResult == -2) {
                            MicQuizNextPromptPresent();
                            HuPrcSleep(120);
                            MicQuizPlayerResponsePresent(micQuizAnswerPlayer);
                            MicQuizMicHudStateSet(1);
                        } else if (micQuizResponseResult == -3) {
                            MicQuizRoundFollowupPresent();
                            HuPrcSleep(120);
                            MicQuizPlayerResponsePresent(micQuizAnswerPlayer);
                            MicQuizMicHudStateSet(1);
                        } else if (micQuizResponseResult == -4) {
                            MicQuizPlayerResponsePresent(micQuizAnswerPlayer);
                            MicQuizMicHudStateSet(1);
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
        HuWinHomeClear(micQuizInstructionWin);
    }
    MicQuizRoundResultSet((s8) micQuizAnswerPlayer, (u16) micQuizCountdownValue,
                          micQuizAnswerAccepted);
    MicQuizPauseAllowSet(0);
    MicQuizMicHudStateSet(0);
    if (micQuizAnswerAccepted != 0) {
        MicQuizFollowupPresent();
    } else if (micQuizTimeoutHandled == 0) {
        MicQuizBannerSlideOut();
        MicQuizMicHudStateSet(0);
        micQuizTimeoutHandled = 1;
        if (quizMode == 0) {
            MicQuizNumberAnswerPresent(micQuizExpectedAnswer);
        } else if (quizMode == 1) {
            MicQuizNumberAnswerAltPresent(micQuizExpectedAnswer);
        } else if (quizMode == 2) {
            MicQuizAnswerPresent(micQuizCharaAnswerMes[micQuizExpectedAnswer]);
        } else if (quizMode == 3) {
            MicQuizAnswerPresent(micQuizCharaAnswerMes[micQuizExpectedAnswer]);
        } else if (quizMode == 4) {
            MicQuizAnswerPresent(micQuizNpcAnswerMes[micQuizExpectedAnswer]);
        } else if (quizMode == 5) {
            MicQuizAnswerPresent(micQuizCompareAnswerMes[micQuizExpectedAnswer]);
        } else if (quizMode == 6) {
            MicQuizAnswerPresent(micQuizFruitAnswerMes[micQuizExpectedAnswer]);
        }
    }
    HuPrcSleep(100);
    MicQuizBannerSlideOut();
    HuPrcSleep(30);
    if (micQuizAnswerAccepted != 0) {
        micQuizResponseState = 3;
        do {
            HuPrcVSleep();
        } while ((s32) micQuizModeWork->presentationBusy != 0);
    } else {
        micQuizResponseState = 4;
        do {
            HuPrcVSleep();
        } while ((s32) micQuizModeWork->presentationBusy != 0);
    }
    for (;;) {
        HuPrcVSleep();
    }
}

/* Run the alternate question response sequence and return its result to the round controller. */
void MicQuizResponseRunAlt(s32 mode) {
    HUPROCESS *buttonProcess;
    HUPROCESS *selectionProcess;
    f64 angleFinal;
    f64 angleFourth;
    f64 angleThird;
    f64 angleSecond;
    f64 angleFirst;
    s32 playerIndex;

    micQuizQuestionReady = 0;
    micQuizResponseReading = 0;
    micQuizResponseMode = mode;
    micQuizListenReady = 0;
    micQuizRetryPrompted = 0;
    micQuizFirstAttempt = 1;
    MicQuizPresentationDone();
    micQuizModeWork->answerPlayer[0] = (s8)(micQuizAnswerPlayer = MicQuizCurrentPlayerGet());
    MicQuizSelectionItemsSet(micQuizRecognitionType, micQuizAnswerPlayer);
    MicQuizMarkerHide();
    MicQuizMicHudStateSet(0);
    MicQuizPauseAllowSet(0);
    micQuizParentProc = fn_8000DD9C();
    do {
        HuPrcVSleep();
    } while (micQuizQuestionReady == 0);
    MicQuizMicHudMotionSet(0);
    micQuizResponseState = 0;
    HuPrcVSleep();
    micQuizResponseState = 1;
    HuPrcSleep(30);
    micQuizCountdownState = 0;
    MicQuizIndicatorSet(1);
    MicQuizMarkerHide();
    MicQuizMicHudStateSet(1);
    fn_8000DAC0(MicQuizPresenterCue, 100U, 6144U, 0, micQuizParentProc);
    buttonProcess = fn_8000DAC0(MicQuizTimeoutWatchAlt, 100U, 6144U, 0, micQuizParentProc);
    selectionProcess = fn_8000DAC0(MicQuizListenWatch, 100U, 6144U, 0, micQuizParentProc);
    MicQuizPauseAllowSet(1);
    for (;;) {
        HuPrcVSleep();
        if (HuMCSelWinCheck() != 0) {
            MicQuizInstructionSet(1);
        }
        micQuizResponseReading = 1;
        micQuizResponseResult = MicQuizResponseRead(micQuizAnswerPlayer, mode);
        MicQuizPauseAllowSet(1);
        micQuizResponseReading = 0;
        MicQuizMarkerHide();
        if (HuMCSelWinCheck() != 0) {
            HuWinHomeClear(micQuizInstructionWin);
        }
        if (micQuizCountdownState == 2) {
            for (;;) {
                HuPrcVSleep();
            }
        }
        MicQuizMicHudStateSet(0);
        if (micQuizResponseResult == 0) {
            if (micQuizRetryPrompted == 0) {
                MicQuizMarkerHide();
                MicQuizMicHudStateSet(0);
                MicQuizBannerSlideOut();
                micQuizResponseState = 2;
                MicQuizIndicatorSet(3);
                MicQuizQuestionIntroPresent();
                HuPrcVSleep();
                for (;;) {
                    HuPrcVSleep();
                    if (micQuizListenReady != 0) {
                        MicQuizIndicatorSet(2);
                        micQuizRetryPrompted = 1;
                        micQuizFirstAttempt = 0;
                        micQuizCountdownState = 0;
                        MicQuizMarkerHide();
                        MicQuizMicHudStateSet(1);
                        MicQuizMicHudMotionSet(2);
                        break;
                    } else {
                        MicQuizBannerSlideOut();
                        continue;
                    }
                }
                MicQuizPlayerResponsePresent(micQuizAnswerPlayer);
                MicQuizMicHudStateSet(1);
            } else {
                MicQuizRoundFollowupNextPresent();
                HuPrcSleep(120);
                MicQuizPlayerResponsePresent(micQuizAnswerPlayer);
                MicQuizMicHudStateSet(1);
            }
            continue;
        }
        if (micQuizResponseResult == 1) {
            micQuizCountdownState = 1;
            micQuizFirstAttempt = 0;
            if (micQuizListenReady == 0) {
                MicQuizIndicatorSet(4);
            }
            micQuizAnswerAccepted = 1;
            MicQuizEventIconEffect(1, MicQuizQuestionEventGet());
            micQuizModeWork->answerResult[0] = 0;
            MicQuizAnswerFeedback(micQuizAnswerAccepted, micQuizModeWork->answerPlayer[0]);
            break;
        }
        if (micQuizResponseResult == 2) {
            MicQuizEventIconEffect(0, MicQuizQuestionEventGet());
            micQuizCountdownState = 1;
            micQuizFirstAttempt = 0;
            micQuizAnswerAccepted = 0;
            micQuizModeWork->answerResult[0] = 1;
            if (micQuizListenReady == 0) {
                MicQuizIndicatorSet(4);
            }
            MicQuizAnswerFeedback(micQuizAnswerAccepted, micQuizModeWork->answerPlayer[0]);
            HuPrcSleep(120);
        } else {
            if (micQuizResponseResult == -2) {
                MicQuizNextPromptPresent();
                HuPrcSleep(120);
                MicQuizPlayerResponsePresent(micQuizAnswerPlayer);
                MicQuizMicHudStateSet(1);
            } else if (micQuizResponseResult == -3) {
                MicQuizRoundFollowupPresent();
                HuPrcSleep(120);
                MicQuizPlayerResponsePresent(micQuizAnswerPlayer);
                MicQuizMicHudStateSet(1);
            } else if (micQuizResponseResult == -4) {
                MicQuizPlayerResponsePresent(micQuizAnswerPlayer);
                MicQuizMicHudStateSet(1);
            }
            continue;
        }
        MicQuizPresenterMotionSet(0, 8.0f, ((1 << 30) | (1 << 0)));
        micQuizFirstAttempt = 0;
        if (HuMCSelWinCheck() != 0) {
            MicQuizInstructionSet(3);
        }
        HuPrcVSleep();
        MicQuizMarkerShow();
        MicQuizMicHudStateSet(2);
        MicQuizPortraitHide(micQuizAnswerPlayer);
        MicQuizBannerSlideOut();
        MicQuizPresenterMotionSet(1, 16.0f, ((1 << 30) | (1 << 0)));
        MicQuizMicHudMotionSet(2);
        micQuizCountdownState = 0;
        HuPrcKill(buttonProcess);
        MicQuizResponseFollowupPresent();
        for (;;) {
            HuPrcVSleep();
            if (micQuizCountdownState == 2) {
                MicQuizTimeoutFollowupPresent();
                HuPrcSleep(60);
                micQuizAnswerAccepted = 0;
                micQuizModeWork->answerResult[1] = 2;
            } else {
                for (playerIndex = 0; playerIndex < 4; playerIndex += 1) {
                    if (micQuizModeWork->playerActive[playerIndex] != 0 &&
                        playerIndex != MicQuizCurrentPlayerGet() &&
                        (lbl_802C0008[playerIndex] & 0x100) != 0) {
                        HuAudFXPlay(MSM_SE_MICQUIZ_PLAYER_BUZZ_IN);
                        micQuizModeWork->answerPlayer[1] = (s8)(micQuizAnswerPlayer = playerIndex);
                        micQuizCountdownState = 1;
                        MicQuizMarkerHide();
                        MicQuizMicHudStateSet(1);
                        MicQuizPortraitHide(MicQuizCurrentPlayerGet());
                        MicQuizPortraitShow(micQuizAnswerPlayer);
                        MicQuizBannerSlideIn();
                        MicQuizPlayerResponsePresent(micQuizAnswerPlayer);
                        MicQuizSelectionItemsSet(micQuizRecognitionType, micQuizAnswerPlayer);
                        /* Stop scanning as soon as a player buzzes in. */
                        goto answer_buzzing_player;
                    }
                }
                continue;
            answer_buzzing_player:
                if (HuMCSelWinCheck() != 0) {
                    MicQuizInstructionSet(4);
                }
                for (;;) {
                    micQuizResponseReading = 1;
                    micQuizResponseResult = MicQuizResponseRead(playerIndex, mode);
                    MicQuizPauseAllowSet(1);
                    micQuizResponseReading = 0;
                    MicQuizMarkerHide();
                    if (HuMCSelWinCheck() != 0) {
                        HuWinHomeClear(micQuizInstructionWin);
                    }
                    MicQuizMicHudStateSet(0);
                    if (micQuizResponseResult == 0) {
                        MicQuizRoundFollowupNextPresent();
                        HuPrcSleep(120);
                        if (HuMCSelWinCheck()) {
                            MicQuizInstructionSet(4);
                        }
                        MicQuizPlayerResponsePresent(micQuizAnswerPlayer);
                        MicQuizMicHudStateSet(1);
                        continue;
                    }
                    if (micQuizResponseResult == 1) {
                        micQuizCountdownState = 1;
                        micQuizAnswerAccepted = 1;
                        micQuizModeWork->answerResult[1] = 0;
                        MicQuizAnswerFeedback(micQuizAnswerAccepted,
                                              micQuizModeWork->answerPlayer[1]);
                    } else if (micQuizResponseResult == 2) {
                        micQuizCountdownState = 1;
                        micQuizAnswerAccepted = 0;
                        micQuizModeWork->answerResult[1] = 1;
                        MicQuizAnswerFeedback(micQuizAnswerAccepted,
                                              micQuizModeWork->answerPlayer[1]);
                    } else {
                        if (micQuizResponseResult == -2) {
                            MicQuizNextPromptPresent();
                            HuPrcSleep(120);
                            MicQuizPlayerResponsePresent(micQuizAnswerPlayer);
                            MicQuizMicHudStateSet(1);
                        } else if (micQuizResponseResult == -3) {
                            MicQuizRoundFollowupPresent();
                            HuPrcSleep(120);
                            MicQuizPlayerResponsePresent(micQuizAnswerPlayer);
                            MicQuizMicHudStateSet(1);
                        } else if (micQuizResponseResult == -4) {
                            MicQuizPlayerResponsePresent(micQuizAnswerPlayer);
                            MicQuizMicHudStateSet(1);
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
        HuWinHomeClear(micQuizInstructionWin);
    }
    MicQuizRoundResultSet((s8) micQuizAnswerPlayer, (u16) micQuizCountdownValue,
                          micQuizAnswerAccepted);
    MicQuizPauseAllowSet(0);
    MicQuizBannerSlideOut();
    MicQuizMicHudStateSet(0);
    if (micQuizAnswerAccepted != 0) {
        micQuizResponseState = 3;
    } else {
        HuPrcSleep(120);
        micQuizResponseState = 4;
    }
    for (;;) {
        HuPrcVSleep();
    }
}

/* Read and validate a microphone or selection-window response for the question type. */
s32 MicQuizResponseRead(s32 playerIndex, s32 responseMode) {
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

    HuMCSelWinContextSet(micQuizMicContext, 0, (u8) playerIndex);
    HuMCSelWinChoiceGet();
    if (micQuizCountdownState == 2) {
        return -1;
    }

    micQuizResponse = (MicquizResponse *) HuMCCurResponseGet();
    if (micQuizResponse->result == 0) {
        micQuizRecognizedAnswer = micQuizResponse->recognizedWord;
    } else if (micQuizResponse->result == -1) {
        return -2;
    } else if (micQuizResponse->result == -2) {
        return -2;
    } else if (micQuizResponse->result == -4) {
        return -3;
    } else {
        return -2;
    }

    if (HuMCUnkResponseCheck() != 0) {
        MicQuizPauseAllowSet(0);
        MicQuizMicHudStateSet(1);
        HuPrcVSleep();
        MicQuizMicHudStateSet(1);
        MicQuizTempMesKill(micQuizParentProc);
        selectedResponse = HuMCNewResponseGet(newResponseMessage);
        if (selectedResponse == -1) {
            MicQuizMicHudStateSet(0);
            MicQuizTempMesStart();
            MicQuizPauseAllowSet(1);
            return -4;
        }
        micQuizRecognizedAnswer = selectedResponse;
        MicQuizPauseAllowSet(1);
        MicQuizMicHudStateSet(0);
    }

    if (HuMCSelWinCheck() != 0) {
        micQuizRecognizedAnswer -= 1;
    }
    if (micQuizRecognizedAnswer == 0) {
        return 0;
    }

    if (responseMode == 0) {
        if (HuMCSelWinCheck() != 0) {
            responseData = MicQuizResponseCompare(micQuizRecognizedAnswer + micQuizAnswerPageOffset,
                                                  micQuizExpectedAnswer);
        } else {
            responseData = MicQuizResponseCompare(micQuizRecognizedAnswer, micQuizExpectedAnswer);
        }
    } else if (responseMode == 1) {
        if (HuMCSelWinCheck() != 0) {
            responseData = MicQuizResponseCompare(micQuizRecognizedAnswer + micQuizAnswerPageOffset,
                                                  micQuizExpectedAnswer);
        } else {
            responseData = MicQuizResponseCompare(micQuizRecognizedAnswer, micQuizExpectedAnswer);
        }
    } else if (responseMode == 2) {
        responseData = MicQuizResponseCompare(micQuizRecognizedAnswer - 1, micQuizExpectedAnswer);
    } else if (responseMode == 3) {
        responseData = MicQuizResponseCompare(micQuizRecognizedAnswer - 1, micQuizExpectedAnswer);
    } else if (responseMode == 4) {
        responseData = MicQuizResponseCompare(micQuizRecognizedAnswer - 1, micQuizExpectedAnswer);
    } else if (responseMode == 5) {
        responseData = MicQuizResponseCompare(micQuizRecognizedAnswer - 1, micQuizExpectedAnswer);
    } else if (responseMode == 6) {
        responseData = MicQuizResponseCompare(micQuizRecognizedAnswer - 1, micQuizExpectedAnswer);
    }
    return responseData;
}

/* Classify the response as equal to or different from the expected answer. */
s32 MicQuizResponseCompare(s32 response, s32 expectedAnswer) {
    if (response == expectedAnswer) {
        return 1;
    }
    return 2;
}

/* Swing the listening indicator back and forth while its child process runs. */
void MicQuizIndicatorSwing(void) {
    f32 baseRotation;
    f32 phaseDegrees;
    f32 sineOffset;
    s16 spriteId;
    s32 movingForward;

    spriteId = micQuizIndicatorSwingGroup;
    phaseDegrees = 0.0f;
    movingForward = 0;
    baseRotation = 0.0f;
    for (;;) {
        HuPrcVSleep();
        sineOffset = (f32) (10.0 * sin((M_PI * (f64) phaseDegrees) / 180.0));
        HuSprZRotSet(spriteId, 0, baseRotation + sineOffset);
        if (movingForward == 0) {
            phaseDegrees += 3.0f;
            if (phaseDegrees >= 90.0f) {
                phaseDegrees = 90.0f;
                movingForward = 1;
            }
            continue;
        }
        phaseDegrees -= 3.0f;
        if (!(phaseDegrees <= -90.0f)) {
            continue;
        }
        phaseDegrees = -90.0f;
        movingForward = 0;
    }
}

/* Update listening feedback as the microphone selection state changes during a response. */
void MicQuizListenWatch(void)
{
    s32 selectionHandled = 0;

    for (;;) {
        HuPrcVSleep();
        if (micQuizCountdownState == 2) {
            MicQuizMicHudStateSet(0);
            continue;
        }
        if (micQuizResponseReading == 0) {
            continue;
        }
        if (HuMCSelWinCheck() != 0) {
            if ((s16)HuMCSelModeGet() == 1) {
                if (selectionHandled == 0) {
                    MicQuizPauseAllowSet(0);
                    MicQuizRingsStart();
                    MicQuizMicHudMotionSet(1);
                    MicQuizInstructionSet(2);
                    MicQuizMicHudStateSet(3);
                    if (micQuizCountdownState != 2) {
                        micQuizCountdownState = 1;
                    }
                    MicQuizBannerSlideIn();
                    selectionHandled = 1;
                }
            } else {
                MicQuizRingsStop();
                selectionHandled = 0;
            }
            continue;
        }
        if (HuMCButtonGet() != 0) {
            MicQuizPauseAllowSet(0);
            MicQuizRingsStart();
            MicQuizMicHudStateSet(3);
            MicQuizBannerSlideIn();
            MicQuizMicHudMotionSet(1);
            if (micQuizCountdownState != 2) {
                micQuizCountdownState = 1;
            }
        } else {
            MicQuizRingsStop();
        }
        if (HuMCButtonGet() != 0) {
            if (selectionHandled == 0) {
                MicQuizPauseAllowSet(0);
                MicQuizRingsStart();
                selectionHandled = 1;
            }
            continue;
        }
        selectionHandled = 0;
    }
}

/* Switch the microphone HUD between idle, listening, and completed response displays. */
void MicQuizMicHudStateSet(s32 displayState) {
    if (displayState == 0) {
        if (micQuizMicHudState != displayState) {
            micQuizMicHudState = displayState;
            HuSprAttrReset(micQuizMicHudGroup, 2, HUSPR_ATTR_DISPOFF);
            HuSprBankSet(micQuizMicHudGroup, 0, 1);
            HuSprBankSet(micQuizMicHudGroup, 1, 1);
            MicQuizMicPulseStop();
            HuSprScaleSet(micQuizMicHudGroup, 1, 1.0f, 1.0f);
            MicQuizRingsStop();
        }
    } else if (displayState == 1) {
        if (micQuizMicHudState != displayState) {
            micQuizMicHudState = displayState;
            HuSprAttrSet(micQuizMicHudGroup, 2, HUSPR_ATTR_DISPOFF);
            HuSprBankSet(micQuizMicHudGroup, 0, 0);
            HuSprBankSet(micQuizMicHudGroup, 1, 0);
            MicQuizMicPulseStart();
        }
    } else if (displayState == 2) {
        if (micQuizMicHudState != displayState) {
            micQuizMicHudState = displayState;
            HuSprAttrReset(micQuizMicHudGroup, 2, HUSPR_ATTR_DISPOFF);
            HuSprBankSet(micQuizMicHudGroup, 0, 1);
            HuSprBankSet(micQuizMicHudGroup, 1, 1);
        }
    } else if (displayState == 3 && micQuizMicHudState != displayState) {
        micQuizMicHudState = displayState;
        MicQuizMicPulseStop();
        HuSprScaleSet(micQuizMicHudGroup, 1, 1.0f, 1.0f);
    }
}

/* Start the microphone pulse process if it is not already running. */
void MicQuizMicPulseStart(void) {
    if(micQuizMicPulseProc==0) {
        micQuizMicPulseProc=fn_8000DAC0(MicQuizMicHudPulse,100,8192,0,micQuizParentProc);
    }
}

/* Stop the microphone pulse process when listening ends. */
void MicQuizMicPulseStop(void) {
    if (micQuizMicPulseProc) {
        HuPrcKill(micQuizMicPulseProc);
        micQuizMicPulseProc = NULL;
    }
}

/* Start the repeating microphone activity rings if they are not already running. */
void MicQuizRingsStart(void) {
    if(micQuizRingProc==0) {
        micQuizRingProc=fn_8000DAC0(MicQuizRingEmit,100,8192,0,micQuizParentProc);
    }
}

/* Stop the repeating microphone activity rings when listening ends. */
void MicQuizRingsStop(void) {
    if (micQuizRingProc) {
        HuPrcKill(micQuizRingProc);
        micQuizRingProc = NULL;
    }
}

/* Configure selection-window answer entries and bind them to the answering controller. */
void MicQuizSelectionItemsSet(s32 select, s32 padNo) {
    if (select == 0 || select == 1) {
        MicQuizSelectionNumbersSet(padNo, select);
    } else if (select == 2) {
        micQuizSelectionItem = HuMCSelWinItemSet(MESSNUM(MESS_QUIZ_CHARA, 0), 11, (s16)padNo);
    } else if (select == 3) {
        micQuizSelectionItem = HuMCSelWinItemSet(MESSNUM(MESS_QUIZ_CHARA, 0), 11, (s16)padNo);
    } else if (select == 4) {
        micQuizSelectionItem = HuMCSelWinItemSet(MESSNUM(MESS_QUIZ_CHARA, 11), 7, (s16)padNo);
    } else if (select == 5) {
        micQuizSelectionItem = HuMCSelWinItemSet(MESSNUM(MESS_QUIZ_CHARA, 54), 3, (s16)padNo);
    } else if (select == 6) {
        micQuizSelectionItem = HuMCSelWinItemSet(MESSNUM(MESS_MIC_RETYPING, 118), 9, (s16)padNo);
    }
    HuMCSessionSet((s16)padNo);
}

/* Configure the numeric selection range and bind it to the answering controller. */
void MicQuizSelectionNumbersSet(s32 padNo, s32 direction) {
    s32 messageBase;

    if (direction == 0) {
        /* Numeric answers use either the first or second ten-entry selection page. */
    if (micQuizExpectedAnswer <= 10) {
            messageBase = MESSNUM(MESS_QUIZ_CHARA, 81);
            micQuizAnswerPageOffset = 0;
        } else {
            messageBase = MESSNUM(MESS_QUIZ_CHARA, 92);
            micQuizAnswerPageOffset = 10;
        }
    } else if (micQuizExpectedAnswer <= 10) {
        messageBase = MESSNUM(MESS_QUIZ_CHARA, 103);
        micQuizAnswerPageOffset = 0;
    } else {
        messageBase = MESSNUM(MESS_QUIZ_CHARA, 114);
        micQuizAnswerPageOffset = 10;
    }
    micQuizSelectionItem = HuMCSelWinItemSet(messageBase, 11, (s16) padNo);
}

/* Handle answer timeout, close recognition, and present the correct answer. */
void MicQuizTimeoutWatch(void)
{
    s8 playerIndex;
    u16 selectedValue;
    s32 currentState;
    f32 verticalOffset;
    f32 angle;
    f32 amplitude;

    do {
        HuPrcVSleep();
    } while (micQuizCountdownState != 2);

    MicQuizMicHudMotionSet(1);
    MicQuizIndicatorSet(0);
    HuMCSelWinContextKill();

    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(1);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 17), 1, 1);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 80), 1, 3);
    }
    MicQuizWinMesWait();
    HuPrcSleep(120);

    micQuizAnswerAccepted = 0;
    micQuizModeWork->answerResult[0] = 2;
    currentState = micQuizAnswerAccepted;
    selectedValue = micQuizCountdownValue;
    playerIndex = (s8)micQuizAnswerPlayer;
    micQuizModeWork->answeringPlayer = playerIndex;
    micQuizModeWork->roundPoints = (s16)selectedValue;
    micQuizModeWork->roundResult = currentState;
    micQuizResponseState = 4;

    do {
        HuPrcVSleep();
    } while (micQuizModeWork->presentationBusy != 0);

    if (micQuizTimeoutHandled == 0) {
        micQuizTimeoutHandled = 1;
        amplitude = -362.0f;
        angle = 0.0f;
        if (micQuizBannerShown != 0) {
            do {
                HuPrcVSleep();
                verticalOffset = amplitude * sin((M_PI * (f64)angle) / 180.0);
                HuSprPosSet(micQuizBannerGroup, 0, micQuizBannerX, micQuizBannerY + verticalOffset);
                angle += 3.0f;
            } while (!(angle >= 90.0f));
            micQuizBannerY = -152.0f;
            HuSprPosSet(micQuizBannerGroup, 0, micQuizBannerX, micQuizBannerY);
            HuSprAttrSet(micQuizBannerGroup, 0, HUSPR_ATTR_DISPOFF);
            micQuizBannerShown = 0;
        }

        if (micQuizResponseMode == 0) {
            MicQuizNumberAnswerPresent(micQuizExpectedAnswer);
        } else if (micQuizResponseMode == 1) {
            MicQuizNumberAnswerAltPresent(micQuizExpectedAnswer);
        } else if (micQuizResponseMode == 2) {
            MicQuizAnswerPresent(micQuizCharaAnswerMes[micQuizExpectedAnswer]);
        } else if (micQuizResponseMode == 3) {
            MicQuizAnswerPresent(micQuizCharaAnswerMes[micQuizExpectedAnswer]);
        } else if (micQuizResponseMode == 4) {
            MicQuizAnswerPresent(micQuizNpcAnswerMes[micQuizExpectedAnswer]);
        } else if (micQuizResponseMode == 5) {
            MicQuizAnswerPresent(micQuizCompareAnswerMes[micQuizExpectedAnswer]);
        } else if (micQuizResponseMode == 6) {
            MicQuizAnswerPresent(micQuizFruitAnswerMes[micQuizExpectedAnswer]);
        }
    }

    do {
        HuPrcVSleep();
    } while (micQuizModeWork->presentationBusy != 0);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Handle the alternate answer timeout and finish its presenter animation. */
void MicQuizTimeoutWatchAlt(void) {
    s8 playerIndex;
    u16 selectedValue;
    s32 currentState;
    do
    {
        HuPrcVSleep();
    }
    while (micQuizCountdownState != 2);
    MicQuizMicHudMotionSet(1);
    MicQuizIndicatorSet(0);
    HuMCSelWinContextKill();
    MicQuizBannerSlideOut();
    MicQuizPresenterMotionSet(1, 16.0f, ((1 << 30) | (1 << 0)));
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(1);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 17), 1, 1);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 80), 1, 3);
    }
    MicQuizWinMesWait();
    HuPrcSleep(120);
    micQuizAnswerAccepted = 0;
    micQuizModeWork->answerResult[0] = 2;
    currentState = micQuizAnswerAccepted;
    selectedValue = micQuizCountdownValue;
    playerIndex = (s8) micQuizAnswerPlayer;
    micQuizModeWork->answeringPlayer = playerIndex;
    micQuizModeWork->roundPoints = (s16) selectedValue;
    micQuizModeWork->roundResult = currentState;
    HuPrcSleep(120);
    micQuizResponseState = 4;
    HuPrcEnd();
    for (;;)
    {
        HuPrcVSleep();
    }
}

/* Start the requested microphone HUD transition only when its state changes. */
void MicQuizMicHudMotionSet(s32 motionState) {
    if (motionState == 0) {
        if (micQuizMicHudMotion[0] != motionState) {
            micQuizMicHudMotion[0] = motionState;
            fn_8000DAC0(MicQuizMicHudIdle, 100U, 6144U, 0, micQuizParentProc);
        }
    } else if (motionState == 1) {
        if (micQuizMicHudMotion[0] != motionState) {
            micQuizMicHudMotion[0] = motionState;
            fn_8000DAC0(MicQuizMicGlowFadeOut, 100U, 6144U, 0, micQuizParentProc);
        }
    } else if (motionState == 2) {
        if (micQuizMicHudMotion[0] != motionState) {
            micQuizMicHudMotion[0] = motionState;
            fn_8000DAC0(MicQuizMicGlowFadeIn, 100U, 6144U, 0, micQuizParentProc);
        }
    } else if (motionState == 3 && micQuizMicHudMotion[0] != motionState) {
        micQuizMicHudMotion[0] = motionState;
        fn_8000DAC0(MicQuizMicGlowFlash, 100U, 6144U, 0, micQuizParentProc);
    }
}

/* Reveal the answer timer and transition the microphone HUD into its idle display. */
void MicQuizMicHudIdle(void) {
    f32 angle;
    f32 transparency;
    f32 scale;
    s32 frame;

    HuSprGrpAttrReset(micQuizCountdownGroup[0], HUSPR_ATTR_DISPOFF);
    scale = 0.0f;
    angle = 0.0f;
    for (frame = 0; frame < 20; frame++) {
        HuPrcVSleep();
        scale = (f32)(2.0 - sin((M_PI * (f64)angle) / 180.0));
        HuSprGrpScaleSet(micQuizCountdownGroup[0], scale, scale);
        angle += 4.5f;
        if (angle >= 90.0f) {
            break;
        }
    }
    HuSprGrpScaleSet(micQuizCountdownGroup[0], 1.0f, 1.0f);
    HuSprBankSet(micQuizMicGroup, 0, 1);
    angle = 0.0f;
    for (frame = 0; frame < 10; frame++) {
        HuPrcVSleep();
        transparency = (f32)sin((M_PI * (f64)angle) / 180.0);
        HuSprTPLvlSet(micQuizMicGlowGroup, 0, transparency);
        angle += 9.0f;
        if (angle >= 90.0f) {
            break;
        }
    }
    HuSprAttrReset(micQuizMicIconGroup, 0, HUSPR_ATTR_DISPOFF);
    HuSprAttrSet(micQuizMicGroup, 0, HUSPR_ATTR_DISPOFF);
    angle = 90.0f;
    for (frame = 0; frame < 10; frame++) {
        HuPrcVSleep();
        transparency = (f32)sin((M_PI * (f64)angle) / 180.0);
        HuSprTPLvlSet(micQuizMicGlowGroup, 0, transparency);
        angle -= 9.0f;
        if (angle <= 45.0f) {
            break;
        }
    }
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Fade the microphone glow and restore its idle animation after listening. */
void MicQuizMicGlowFadeOut(void) {
    f32 angle = 90.0f;
    f32 tpLevel;
    s32 count;

    for (count = 0; count < 10; count++) {
        HuPrcVSleep();
        tpLevel = (f32)(0.5 * sin((M_PI * (f64)angle) / 180.0));
        HuSprTPLvlSet(micQuizMicGlowGroup, 0, tpLevel);
        angle += 9.0f;
        if (angle <= 0.0f) {
            break;
        }
    }

    HuSprAttrSet(micQuizMicIconGroup, 0, HUSPR_ATTR_DISPOFF);
    HuSprAttrReset(micQuizMicGroup, 0, HUSPR_ATTR_DISPOFF);
    HuSprBankSet(micQuizMicGroup, 0, 1);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Fade in the microphone glow and show the listening animation. */
void MicQuizMicGlowFadeIn(void) {
    f32 angle = 0.0f;
    s32 frame;

    for (frame = 0; frame < 10; frame++) {
        f32 glowAlpha;

        HuPrcVSleep();
        glowAlpha = (f32) (0.5 * sin((M_PI * (f64) angle) / 180.0));
        HuSprTPLvlSet(micQuizMicGlowGroup, 0, glowAlpha);
        angle += 9.0f;
        if (angle >= 90.0f) {
            break;
        }
    }
    HuSprAttrReset(micQuizMicIconGroup, 0, HUSPR_ATTR_DISPOFF);
    HuSprAttrSet(micQuizMicGroup, 0, HUSPR_ATTR_DISPOFF);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Flash the microphone glow and switch to its fast completed-response animation. */
void MicQuizMicGlowFlash(void) {
    f32 angle;
    f32 transparencyLevel;
    s32 frame;

    angle = 0.0f;
    for (frame = 0; frame < 10; frame++) {
        HuPrcVSleep();
        transparencyLevel = (f32)sin((M_PI * (f64)angle) / 180.0);
        HuSprTPLvlSet(micQuizMicGlowGroup, 0, transparencyLevel);
        angle += 9.0f;
        if (angle >= 90.0f) {
            break;
        }
    }
    HuSprBankSet(micQuizMicGroup, 0, 1);
    angle = 90.0f;
    for (frame = 0; frame < 10; frame++) {
        HuPrcVSleep();
        transparencyLevel = (f32)sin((M_PI * (f64)angle) / 180.0);
        HuSprTPLvlSet(micQuizMicGlowGroup, 0, transparencyLevel);
        angle += 9.0f;
        if (angle >= 90.0f) {
            break;
        }
    }
    HuSprBankSet(micQuizMicGroup, 0, 0);
    angle = 90.0f;
    for (frame = 0; frame < 3; frame++) {
        HuPrcVSleep();
        transparencyLevel = (f32)sin((M_PI * (f64)angle) / 180.0);
        HuSprTPLvlSet(micQuizMicGlowGroup, 0, transparencyLevel);
        angle -= 30.0f;
        if (angle <= 0.0f) {
            break;
        }
    }
    HuSprAttrReset(micQuizMicIconGroup, 0, HUSPR_ATTR_DISPOFF);
    HuSprAttrSet(micQuizMicGroup, 0, HUSPR_ATTR_DISPOFF);
    HuSprSpeedSet(micQuizMicIconGroup, 0, 3.0f);
    angle = 0.0f;
    for (frame = 0; frame < 10; frame++) {
        HuPrcVSleep();
        transparencyLevel = (f32)sin((M_PI * (f64)angle) / 180.0);
        HuSprTPLvlSet(micQuizMicGlowGroup, 0, transparencyLevel);
        angle += 9.0f;
        if (angle >= 90.0f) {
            break;
        }
    }
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Insert the selected answer into the normal or alternate response message. */
void MicQuizAnswerMesPresent(s32 messageType, s32 alternate)
{
    s32 messageIndex;
    s32 insertMessage;

    if (HuMCSelWinCheck() != 0) {
        if (messageType == 0 || messageType == 1) {
            messageIndex = micQuizRecognizedAnswer + micQuizAnswerPageOffset - 1;
        } else {
            messageIndex = micQuizRecognizedAnswer - 1;
        }
    } else {
        messageIndex = micQuizRecognizedAnswer - 1;
    }

    switch (messageType) {
    case 0:
        insertMessage = messageIndex + MESSNUM(MESS_QUIZ_CHARA, 34);
        break;
    case 1:
        insertMessage = messageIndex + MESSNUM(MESS_QUIZ_CHARA, 58);
        break;
    case 2:
        insertMessage = micQuizCharaAnswerMes[messageIndex];
        break;
    case 3:
        insertMessage = micQuizCharaAnswerMes[messageIndex];
        break;
    case 4:
        insertMessage = micQuizNpcAnswerMes[messageIndex];
        break;
    case 5:
        insertMessage = micQuizCompareAnswerMes[messageIndex];
        break;
    case 6:
        insertMessage = micQuizFruitAnswerMes[messageIndex];
        break;
    }

    if (alternate == 0) {

        if (micQuizModeWork->presenterLayout == 0) {

            MicQuizWinOpen(1);

            MicQuizWinInsertMesSet(1, insertMessage, 0);

            MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 16), 1, 1);

        } else {

            MicQuizWinOpen(3);

            MicQuizWinInsertMesSet(3, insertMessage, 0);

            MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 79), 1, 3);

        }

        MicQuizWinMesWait();

    } else {

        if (micQuizModeWork->presenterLayout == 0) {

            MicQuizWinOpen(1);

            MicQuizWinInsertMesSet(1, insertMessage, 0);

            MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 27), 1, 1);

        } else {

            MicQuizWinOpen(3);

            MicQuizWinInsertMesSet(3, insertMessage, 0);

            MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 83), 1, 3);

        }

        MicQuizWinMesWait();

    }
}

/* Present a supplied answer message using the current dialogue layout. */
void MicQuizAnswerPresent(u32 message) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(2);
        MicQuizWinInsertMesSet(2, message, 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 29), 1, 2);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinInsertMesSet(3, message, 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 85), 1, 3);
    }
    MicQuizWinMesWait();
}

/* Present the selected answer from the first answer-name table. An index of 0 has no entry in
 * the index-minus-one lookup; the guard's report is a debug hook that release code drops. */
void MicQuizNumberAnswerPresent(s32 messageIndex) {
    if (messageIndex == 0) {
        DebugValue(messageIndex);
    }
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(2);
        MicQuizWinInsertMesSet(2, micQuizNumberAnswerMes[messageIndex - 1], 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 29), 1, 2);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinInsertMesSet(3, micQuizNumberAnswerMes[messageIndex - 1], 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 85), 1, 3);
    }
    MicQuizWinMesWait();
}

/* Present the selected answer from the alternate answer-name table. An index of 0 has no entry
 * in the index-minus-one lookup; the guard's report is a debug hook that release code drops. */
void MicQuizNumberAnswerAltPresent(s32 messageIndex) {
    if (messageIndex == 0) {
        DebugValue(messageIndex);
    }
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(2);
        MicQuizWinInsertMesSet(2, micQuizNumberAnswerAltMes[messageIndex - 1], 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 29), 1, 2);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinInsertMesSet(3, micQuizNumberAnswerAltMes[messageIndex - 1], 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 85), 1, 3);
    }
    MicQuizWinMesWait();
}

void MicQuizEmptyC(void)
{
}

/* Start the temporary quiz message beneath the active quiz process. */
void MicQuizTempMesStart(void) {
    fn_8000DAC0(MicQuizTempMes, 100U, 6144U, 0, micQuizParentProc);
}

/* Schedule removal of the temporary quiz message if its window exists. */
void MicQuizTempMesKill(HUPROCESS* parent) {
    if(micQuizTempMesWin!=-1)
        fn_8000DAC0(MicQuizTempMesClose,100,6144,0,parent);
}

/* Remove the temporary quiz message window and clear its active handle. */
void MicQuizTempMesClose(void) {
    HuWinKill(micQuizTempMesWin);
    micQuizTempMesWin = -1;
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Show the temporary quiz message for three seconds, then close and remove it. */
void MicQuizTempMes(void) {
    HuVec2f messageSize;

    HuWinMesMaxSizeGet(1, &messageSize, MESSNUM(MESS_MICQUIZ, 58));
    if (micQuizTempMesWin != -1) {
        HuPrcEnd();
        for (;;) {
            HuPrcVSleep();
        }
    }
    micQuizTempMesWin = HuWinExCreateFrame(-10000.0f, 200.0f,
        messageSize.x, messageSize.y, -1, 0);
    HuWinExOpen(micQuizTempMesWin);
    HuWinMesSpeedSet(micQuizTempMesWin, 0);
    HuWinAttrSet(micQuizTempMesWin, HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSet(micQuizTempMesWin, MESSNUM(MESS_MICQUIZ, 58));
    HuWinMesWait(micQuizTempMesWin);
    HuPrcSleep(180);
    if (micQuizTempMesWin != -1) {
        HuWinExClose(micQuizTempMesWin);
        HuWinKill(micQuizTempMesWin);
        micQuizTempMesWin = -1;
    }
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Initialize the answer countdown from the selected level and question event. */
void MicQuizCountdownInit(void)
{
    micQuizCountdownState = 1;
    micQuizCountdownValue = micQuizCountdownStarts[MicQuizQuestionLevelGet()];
    if (MicQuizQuestionEventGet() == 2) {
        micQuizCountdownValue = micQuizCountdownStarts[MicQuizQuestionLevelGet() + 1];
    } else if (MicQuizQuestionEventGet() == 5) {
        micQuizCountdownValue = micQuizCountdownStarts[MicQuizQuestionLevelGet() - 1];
    } else {
        micQuizCountdownValue = micQuizCountdownStarts[MicQuizQuestionLevelGet()];
    }

    /* This event gives each countdown step half the usual number of frames. */
    if (MicQuizQuestionEventGet() == 4) {
        micQuizCountdownStepFrames = 900 / micQuizCountdownValue;
    } else {
        micQuizCountdownStepFrames = 1800 / micQuizCountdownValue;
    }
    micQuizCountdownFrame = 0;
    MicQuizCountdownDigitsSet(micQuizCountdownValue);
}

/* Advance an active answer countdown and update its digits until time expires. */
s32 MicQuizCountdownUpdate(void) {
    if (micQuizCountdownState == 0) {
        micQuizCountdownFrame += 1;
        if (micQuizCountdownFrame >= micQuizCountdownStepFrames) {
            HuAudFXPlay(MSM_SE_MICQUIZ_COUNTDOWN_TICK);
            micQuizCountdownValue -= 1;
            micQuizCountdownFrame = 0;
            MicQuizCountdownDigitsSet(micQuizCountdownValue);
            if (micQuizCountdownValue <= 0) {
                micQuizCountdownState = 2;
            }
        }
    }
    return micQuizCountdownState;
}

/* Set the two answer countdown digits when its value is nonnegative. */
void MicQuizCountdownDigitsSet(s32 pointsRemaining) {
    if (pointsRemaining >= 0) {
        s32 tens = pointsRemaining / 10;
        s32 ones = pointsRemaining - tens * 10;
        HuSprBankSet(micQuizCountdownGroup[0], 0, tens);
        HuSprBankSet(micQuizCountdownGroup[0], 1, ones);
    }
}

/* Choose an integer from the requested minimum up to, but excluding, the maximum. */
s32 MicQuizRandRange(s32 minimum, s32 maximum) {
    return minimum + frandmod(maximum - minimum);
}

/* Test a percentage chance using a random value from zero through ninety-nine. */
s32 MicQuizRandChance(s32 percentChance) {
    s32 roll;

    roll = frandmod(100);
    if (percentChance == 0) {
        return 0;
    }
    if (roll < percentChance) {
        return 1;
    }
    return 0;
}

/* Present one of three accepted-answer exchanges with the animated presenter. */
void MicQuizAcceptExchange(s32 primaryMessageIndex, s32 secondaryMessageIndex) {
    s32 randomChoice;
    s32 previousSelection;

    randomChoice = frandmod(3);
    previousSelection = micQuizResponseState;
    MicQuizReactionLeadIn();
    micQuizResponseState = 5;
    micQuizModeWork->presentationBusy = 1;
    do {
        HuPrcVSleep();
    } while (micQuizModeWork->presentationBusy != 0);
    micQuizResponseState = previousSelection;
    HuPrcSleep(60);

    MicQuizWinOpen(3);
    MicQuizPresenterMotionSet(1, 16.0f, ((1 << 30) | (1 << 0)));

    if (randomChoice == 0) {
        MicQuizWinInsertMesSet(3, micQuizFruitAnswerMes[secondaryMessageIndex], 0);
        MicQuizWinInsertMesSet(3, micQuizCharaNameMes[primaryMessageIndex], 1);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_Q, 63), 1, 3);
        MicQuizWinMesWait();
    } else if (randomChoice == 1) {
        MicQuizWinInsertMesSet(3, micQuizFruitAnswerMes[secondaryMessageIndex], 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_Q, 64), 1, 3);
        MicQuizWinMesWait();
        HuPrcSleep(120);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_Q, 65), 1, 3);
        MicQuizWinMesWait();
    } else {
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_Q, 66), 1, 3);
        MicQuizWinMesWait();
        HuPrcSleep(120);
        MicQuizWinInsertMesSet(3, micQuizCharaNameMes[primaryMessageIndex], 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_Q, 67), 1, 3);
        MicQuizWinMesWait();
    }
}

/* Present one of four follow-up exchanges for a recognized answer choice. */
void MicQuizChoiceExchange(s32 primaryMessageIndex, s32 secondaryMessageIndex)
{
    s32 randomChoice = frandmod(4);

    if (randomChoice != 0) {
        s32 previousSelection = micQuizResponseState;

        MicQuizReactionLeadIn();
        micQuizResponseState = 5;
        micQuizModeWork->presentationBusy = 1;
        do {
            HuPrcVSleep();
        } while (micQuizModeWork->presentationBusy != 0);
        micQuizResponseState = previousSelection;
        HuPrcSleep(60);
    }

    MicQuizWinOpen(3);
    MicQuizPresenterMotionSet(1, 16.0f, ((1 << 30) | (1 << 0)));

    if (randomChoice == 0) {
        MicQuizWinInsertMesSet(3, micQuizFruitAnswerMes[secondaryMessageIndex], 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_Q, 68), 1, 3);
    } else if (randomChoice == 1) {
        MicQuizWinInsertMesSet(3, micQuizCharaNameMes[primaryMessageIndex], 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_Q, 69), 1, 3);
    } else if (randomChoice == 2) {
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_Q, 70), 1, 3);
        MicQuizWinMesWait();
        HuPrcSleep(120);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_Q, 71), 1, 3);
        MicQuizWinMesWait();
    } else {
        MicQuizWinInsertMesSet(3, micQuizFruitAnswerMes[secondaryMessageIndex], 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_Q, 72), 1, 3);
        MicQuizWinMesWait();
        HuPrcSleep(120);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_Q, 73), 1, 3);
        MicQuizWinMesWait();
    }
    MicQuizWinMesWait();
}

/* Present one of two follow-up exchanges for a recognized flagged answer choice. */
void MicQuizFlaggedChoiceExchange(s32 primaryMessageIndex, s32 secondaryMessageIndex)
{
    s32 randomChoice = frandmod(2);

    if (randomChoice != 0) {
        s32 previousSelection = micQuizResponseState;

        MicQuizReactionLeadIn();
        micQuizResponseState = 5;
        micQuizModeWork->presentationBusy = 1;
        do {
            HuPrcVSleep();
        } while (micQuizModeWork->presentationBusy != 0);
        micQuizResponseState = previousSelection;
        HuPrcSleep(60);
    }

    MicQuizWinOpen(3);
    MicQuizPresenterMotionSet(1, 16.0f, ((1 << 30) | (1 << 0)));

    if (randomChoice == 0) {
        MicQuizWinInsertMesSet(3, micQuizFruitAnswerMes[secondaryMessageIndex], 0);
        MicQuizWinInsertMesSet(3, micQuizCharaNameMes[primaryMessageIndex], 1);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_Q, 74), 1, 3);
        MicQuizWinMesWait();
        HuPrcSleep(120);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_Q, 75), 1, 3);
        MicQuizWinMesWait();
    } else {
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_Q, 76), 1, 3);
        MicQuizWinMesWait();
        HuPrcSleep(120);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_Q, 77), 1, 3);
        HuPrcSleep(120);
    }
}

/* Present the follow-up for an answer absent from the active choice records. */
void MicQuizUnlistedChoiceExchange(s32 messageIndex) {

    MicQuizWinOpen(3);
    MicQuizPresenterMotionSet(1, 16.0f, ((1 << 30) | (1 << 0)));
    MicQuizWinInsertMesSet(3, micQuizFruitAnswerMes[messageIndex], 0);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_Q, 79), 1, 3);
    MicQuizWinMesWait();
}

/* Present and wait for the line preceding a presenter reaction. */
void MicQuizReactionLeadIn(void) {
    MicQuizWinOpen(3);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_Q, 62), 1, 3);
    if (micQuizActiveWin != -1) {
        HuWinMesWait(micQuizActiveWin);
    }
}

/* Return the zero-based index of the current recognized answer. */
s32 MicQuizRecognizedAnswerGet(void) {
    return micQuizRecognizedAnswer - 1;
}

/* Present accepted or rejected answer feedback and animate the presenter accordingly. */
void MicQuizAnswerFeedback(s32 success, s32 choice) {
    s32 slot;

    MicQuizBannerSlideOut();
    HuPrcSleep(30);

    if (success != 0) {
        MicQuizAcceptExchange(micQuizModeWork->playerCharacter[choice],
                              micQuizRecognizedAnswer - 1);
        HuAudFXPlay(MSM_SE_CMN_SELECTION_ACCEPT);
        micQuizFeedbackGroup = micQuizAcceptIconGroup;
        if (micQuizMicHudMotion[0] != 3) {
            micQuizMicHudMotion[0] = 3;
            fn_8000DAC0(MicQuizMicGlowFlash, 100U, 6144U, 0, micQuizParentProc);
        }
        fn_8000DAC0(MicQuizFeedbackFlash, 100U, 6144U, 0, micQuizParentProc);
        HuAudFXPlay(MSM_SE_GUIDE_KOOPA_TALK);
        MicQuizPresenterMotionSet(3, 8.0f, ((1 << 30) | (1 << 0)));
        HuPrcSleep(120);
        return;
    }

    if (MicQuizChoiceAvailable(micQuizRecognizedAnswer - 1) != 0) {
        slot = MicQuizChoiceFind(micQuizRecognizedAnswer - 1);
        if (micQuizChoiceRecords[slot].answered != 0) {
            MicQuizFlaggedChoiceExchange(micQuizModeWork->playerCharacter[choice],
                                         micQuizRecognizedAnswer - 1);
        } else {
            MicQuizChoiceExchange(micQuizModeWork->playerCharacter[choice],
                                  micQuizRecognizedAnswer - 1);
        }
    } else {
        MicQuizUnlistedChoiceExchange(micQuizRecognizedAnswer - 1);
    }

    HuAudFXPlay(MSM_SE_CMN_SELECTION_REJECT);
    micQuizFeedbackGroup = micQuizRejectIconGroup;
    fn_8000DAC0(MicQuizFeedbackFlash, 100U, 6144U, 0, micQuizParentProc);
    MicQuizPresenterMotionSet(6, 8.0f, ((1 << 30) | (1 << 0)));
    HuPrcSleep(120);
}

/* Play the presenter motion cue at the marked frames of its active motion. */
void MicQuizPresenterCue(void) {
    f32 motionTime;
    s16 motionId;

    for (;;) {
        HuPrcVSleep();
        motionTime = fn_8002D4E0(micQuizPresenterModel);
        motionId = fn_8002D3C4(micQuizPresenterModel);
        if (motionId == micQuizPresenterMotion[6]) {
            if ((20.0f == motionTime) || (55.0f == motionTime)) {
                HuAudFXPlay(MSM_SE_GUIDE_KOOPA_STEP);
            }
            continue;
        }
        if (motionId != micQuizPresenterMotion[2]) {
            continue;
        }
        if ((10.0f != motionTime) && (60.0f != motionTime)) {
            continue;
        }
        HuAudFXPlay(MSM_SE_GUIDE_KOOPA_STEP);
    }
}

/* Find an answer key in the eight choice records, returning minus one if absent. */
int MicQuizChoiceFind(int key) {
    int index;
    for(index=0;index<8;index++) {
        if(key==micQuizChoiceRecords[index].answerIndex) return index;
    }
    return -1;
}

/* Test whether an answer key has an available choice record. */
int MicQuizChoiceAvailable(int key) {
    int index;
    for (index = 0; index < 8; index++) {
        if (key == micQuizChoiceRecords[index].answerIndex &&
            micQuizChoiceRecords[index].availabilityState == 0) {
            return 1;
        }
    }
    return 0;
}

/* Create the presenter model and load its seven dialogue motions. */
void MicQuizPresenterCreate(void) {
    s32 index;

    micQuizPresenterModel = fn_80025A04(
        fn_80008DD0(DATANUM(0xa40000, 121), 0x10000000, 2));
    for (index = 0; index < 7; index++) {
        micQuizPresenterMotion[index] = fn_80030458(
            micQuizPresenterModel,
            fn_80008DD0(micQuizPresenterMotionFiles[index], 0x10000000, 2)
        );
    }
    fn_800269E8(micQuizPresenterModel, ((1 << 30) | (1 << 0)));
    MicQuizPresenterMotionSet(0, 0.0f, ((1 << 30) | (1 << 0)));
    fn_80026760(micQuizPresenterModel, 0.0f, 0.0f, -410.0f);
}

/* Change the presenter motion only when the requested motion is not already active. */
void MicQuizPresenterMotionSet(s32 motionNo, f32 blendFrames, u32 attr) {
    HU3D_MOTIONID currentMotion;

    currentMotion = fn_8002D3C4(micQuizPresenterModel);
    if (currentMotion != micQuizPresenterMotion[motionNo]) {
        fn_8002CCC0(micQuizPresenterModel, micQuizPresenterMotion[motionNo],
                           0.0f, blendFrames, attr);
    }
}

/* Enable or disable pausing as the response process enters a protected interval. */
void MicQuizPauseAllowSet(s32 pauseAllowed) {
    micQuizModeWork->pauseAllowed = pauseAllowed;
}

/* Shrink and hide the selected player portrait on the microphone HUD. */
void MicQuizPortraitHide(int playerIndex) {
    s16 sprite = (s16)(playerIndex + 4);
    f32 scale;
    f32 angle;

    angle = 90.0f;
    do {
        HuPrcVSleep();
        scale = (f32) sin((M_PI * (f64) angle) / 180.0);
        HuSprScaleSet(micQuizMicHudGroup, sprite, scale, scale);
        angle += 9.0f;
    } while (!(angle >= 180.0f));
    HuSprAttrSet(micQuizMicHudGroup, sprite, HUSPR_ATTR_DISPOFF);
}

/* Scale the selected player portrait into view on the microphone HUD. */
void MicQuizPortraitShow(s32 playerIndex) {
    f32 scale;
    f32 angle;
    s16 spriteIndex;

    spriteIndex = playerIndex + 4;
    angle = 0.0f;
    HuSprScaleSet(micQuizMicHudGroup, spriteIndex, 0.0f, 0.0f);
    HuSprAttrReset(micQuizMicHudGroup, spriteIndex, HUSPR_ATTR_DISPOFF);
    do {
        HuPrcVSleep();
        scale = (f32) sin((M_PI * (f64) angle) / 180.0);
        HuSprScaleSet(micQuizMicHudGroup, spriteIndex, scale, scale);
        angle += 9.0f;
    } while (!(angle >= 90.0f));
}

/* Scale the microphone HUD response marker into view. */
void MicQuizMarkerShow(void) {
    f32 scale;
    f32 angle;

    angle = 0.0f;
    HuSprScaleSet(micQuizMicHudGroup, 3, 0.0f, 0.0f);
    HuSprAttrReset(micQuizMicHudGroup, 3, HUSPR_ATTR_DISPOFF);
    do {
        HuPrcVSleep();
        scale = (f32) sin((M_PI * (f64) angle) / 180.0);
        HuSprScaleSet(micQuizMicHudGroup, 3, scale, scale);
        angle += 9.0f;
    } while (!(angle >= 90.0f));
}

/* Shrink and hide the microphone HUD response marker. */
void MicQuizMarkerHide(void) {
    f32 scale;
    f32 angle;

    angle = 90.0f;
    do {
        HuPrcVSleep();
        scale = (f32) sin((M_PI * (f64) angle) / 180.0);
        HuSprScaleSet(micQuizMicHudGroup, 3, scale, scale);
        angle += 9.0f;
    } while (!(angle >= 180.0f));
    HuSprAttrSet(micQuizMicHudGroup, 3, HUSPR_ATTR_DISPOFF);
}

/* Play dialogue sound cues, panning selected speakers for the listed messages. */
void MicQuizWinSoundCallback(HUWINID windowId, u32 answerId, s16 soundIndex) {
    /* Dialogue messages whose speaker cues use separate left and right pan positions. */
    s32 answerIds[20] = {
        MESSNUM(MESS_MICQUIZ, 0),           MESSNUM(MESS_MICQUIZ, 6),
        MESSNUM(MESS_MICQUIZ, 12),          MESSNUM(MESS_MICQUIZ, 30),
        MESSNUM(MESS_MICQUIZ, 34),          MESSNUM(MESS_MICQUIZ, 35),
        MESSNUM(MESS_MICQUIZ, 36),          MESSNUM(MESS_MICQUIZ, 37),
        MESSNUM(MESS_MICQUIZ, 39),          MESSNUM(MESS_MICQUIZ, 40),
        MESSNUM(MESS_MICQUIZ, 54),          MESSNUM(MESS_MICQUIZ, 57),
        MESSNUM(MESS_MICQUIZ, 58),          MESSNUM(MESS_MICQUIZ_TUTORIAL, 40),
        MESSNUM(MESS_MICQUIZ_TUTORIAL, 41), MESSNUM(MESS_MICQUIZ_TUTORIAL, 42),
        MESSNUM(MESS_MICQUIZ_TUTORIAL, 44), MESSNUM(MESS_MICQUIZ_TUTORIAL, 45),
        MESSNUM(MESS_MICQUIZ_INTRO, 0),     -1
    };
    /* Speaker cue numbers indexed by the dialogue sound callback. */
    s32 sounds[17] = {
        MSM_SE_GUIDE_DAY_INTRO, MSM_SE_GUIDE_DAY_APOLOGY, MSM_SE_GUIDE_DAY_EXPLAIN,
        MSM_SE_GUIDE_NIGHT_INTRO, MSM_SE_GUIDE_NIGHT_APOLOGY, MSM_SE_GUIDE_NIGHT_EXPLAIN,
        MSM_SE_GUIDE_KOOPA_TALK, MSM_SE_GUIDE_KOOPA_LAUGH, MSM_SE_GUIDE_KOOPA_ANNOUNCE,
        MSM_SE_GUIDE_KOOPA_FAREWELL, MSM_SE_GUIDE_KOOPA_REACT, MSM_SE_GUIDE_KOOPA_EXIT,
        MSM_SE_GUIDE_KOOPA_QUIZ_CHEER, MSM_SE_GUIDE_KOOPA_QUIZ_JEER,
        MSM_SE_GUIDE_DAY_DISAPPOINTED, MSM_SE_GUIDE_NIGHT_DISAPPOINTED, -1
    };
    s16 answerIndex;

    soundIndex--;

    /* Only the listed dialogue messages separate these speaker cues by pan. */
    for (answerIndex = 0; ; answerIndex++) {
        if (answerIds[answerIndex] == -1) {
            HuAudFXPlay(sounds[soundIndex]);
            return;
        }
        if (answerId == (u32)answerIds[answerIndex]) {
            if (soundIndex == 0 || soundIndex == 1 ||
                soundIndex == 2 || soundIndex == 14) {
                HuAudFXPlayPan(sounds[soundIndex], 48);
                return;
            }
            if (soundIndex == 3 || soundIndex == 4 ||
                soundIndex == 5 || soundIndex == 15) {
                HuAudFXPlayPan(sounds[soundIndex], 80);
            }
            break;
        }
    }
}
