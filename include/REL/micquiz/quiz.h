/* Declarations of the quiz code the five microphone quiz modules share (quiz_common.h): its
 * types, the data the modules use from it, and one prototype for each of its functions. */
#ifndef REL_MICQUIZ_QUIZ_H
#define REL_MICQUIZ_QUIZ_H

#include "game/process.h"
#include "game/sprite.h"
#include "game/window.h"
#include "game/hu3d.h"

/* Debug hook: release code ignores the value after evaluating it. */
static inline void DebugValue(s32 value) {
}

int HuMCSelModeGet(void);
void HuSprTexLoad(ANIMDATA *anim, s16 bmpNo, s16 texMapId, GXTexWrapMode wrapS, GXTexWrapMode wrapT,
                  GXTexFilter filter);

typedef struct MicquizChoiceRecord_s {
    s32 answerIndex; /* Zero-based recognized answer index. */
    s32 answered; /* Nonzero once this answer was given; the response then takes the flagged
                   * follow-up. */
    HU3D_MODELID modelId; /* Answer model that micquizishidll creates and draws. */
    s32 availabilityState; /* Zero allows this record to supply recognized-answer feedback. */
} MicquizChoiceRecord;

/* Screen positions for the four repeating event decorations. */
struct MicquizScreenPosition {
              f32 positionX;
              f32 positionY;
};

/* Response fields read after the microphone choice session completes. */
typedef struct MicquizResponse {
    s16 result; /* Response returned by the microphone session. */
    u16 score; /* Recognition score of the first candidate. */
    s16 wordCount; /* Number of words in the first candidate. */
    s16 *words; /* Indices of the first candidate words. */
    s32 recognizedWord; /* First speech word or one-based controller choice; minus one if
                         * unavailable. */
} MicquizResponse;

/* Quiz state shared by the scene controller and microphone response processes. */
typedef struct MicquizModeWork_s {
    s32 playerActive[4]; /* Nonzero for each participating player. */
    u8 playerCount; /* Number of participating players. */
    u8 questionCount; /* Current question count shown by the pause display. */
    u8 currentPlayer; /* Player whose turn selects the question; zero through three. */
    s8 answeringPlayer; /* Player stored with the round result; minus one before a response. */
    s8 answerResult[2]; /* Outcome of the first answer and of the second player's attempt. */
    s8 answerPlayer[2]; /* Player who gave each of those answers. */
    s32 roundResult; /* Result returned by the question response process. */
    s16 roundPoints; /* Point amount returned by the question response process. */
    u8 questionLevel; /* Selected question level. */
    u8 savedQuestionLevel; /* Saved question level restored after a rejected selection. */
    u8 questionColumn; /* Selected question column. */
    u8 savedQuestionColumn; /* Saved question column restored after a rejected selection. */
    u8 questionFlags[5][5]; /* Flag bytes for the five-by-five question grid. */
    s16 playerScore[4]; /* Current score for each player. */
    u8 playerRank[4]; /* One-based placing for each player; equal values indicate ties. */
    u8 playerCharacter[4]; /* Character number for each player. */
    u16 questionEvent; /* Question event used for timing and presentation. */
    s32 presenterLayout; /* Presenter dialogue layout; zero uses the paired windows. */
    s32 presentationBusy; /* Nonzero until the requested presentation transition finishes. */
    s32 compactPauseDisplay; /* Nonzero uses the compact pause HUD and returns one overlay level. */
    s32 pauseAllowed; /* Nonzero allows the Start-button pause menu. */
    s32 pauseDisplayActive; /* Nonzero while the pause display draw pass is active. */
    s32 randomEnabled; /* Nonzero enables random option selection; the quiz setup sets it to one. */
    s32 selectionMode; /* Current selection-screen mode. */
    s32 menuDifficultyIndex; /* Difficulty chosen in the development menu, from 0 through 4. */
    s32 modeOption[4]; /* Option chosen in the selection menu for quiz modes 1 through 4. */
    s32 mode0Option; /* Option chosen in the selection menu for quiz mode 0. */
} MicquizModeWork;

/* Each including module defines these after its own data. */
extern f32 micQuizPauseBgScroll;
extern ANIMDATA *micQuizPauseBgAnim;
extern HUPROCESS *micQuizMicPulseProc;
extern HUPROCESS *micQuizIndicatorSwingProc;
extern MicquizResponse *micQuizResponse;
extern s32 micQuizTimeoutHandled;
extern HUPROCESS *micQuizRingProc;

/* Data of quiz_common.h that the modules' other files use. */
extern MicquizChoiceRecord micQuizChoiceRecords[8];
extern MicquizModeWork *micQuizModeWork;
extern s16 micQuizGameMes;
extern HU3D_MODELID micQuizPresenterModel;
extern u32 micQuizNpcAnswerMes[17];

s32 MicQuizResponseStateGet(void);
void MicQuizWinCreate(void);
void MicQuizInstructionSet(u32 messageType);
void MicQuizHudCreate(void);
void MicQuizEventSpark(void);
void MicQuizMicHudCreate(void);
void MicQuizScoreDigitsCreate(void);
void MicQuizHudDrawNormal(void);
void MicQuizHudDrawPause(void);
void MicQuizIndicatorSet(s32 action);
void MicQuizSparksSmall(void);
void MicQuizSparkExpand(void);
void MicQuizNextPromptPresent(void);
void MicQuizPlayerResponsePresent(s32 playerIndex);
void MicQuizListenWatch(void);
void MicQuizRingsStart(void);
void MicQuizRingsStop(void);
void MicQuizMicGlowFlash(void);
void MicQuizAnswerPresent(u32 message);
void MicQuizNumberAnswerAltPresent(s32 messageIndex);
void MicQuizChoiceExchange(s32 primaryMessageIndex, s32 secondaryMessageIndex);
void MicQuizFlaggedChoiceExchange(s32 primaryMessageIndex, s32 secondaryMessageIndex);
void MicQuizUnlistedChoiceExchange(s32 messageIndex);
void MicQuizAnswerFeedback(s32 success, s32 choice);
void MicQuizPresenterMotionSet(s32 motionNo, f32 blendFrames, u32 attr);
void MicQuizWinPlayerMaskSet(void);
void MicQuizBannerSlideIn(void);
void MicQuizBannerSlideOut(void);
short MicQuizWinChoiceGet(short choice);
void MicQuizWinInsertMesSet(s32 windowNo, u32 messageId, s16 insertNo);
void MicQuizInstructionDispSet(s32 visible);
void MicQuizHintSet(s32 messageType);
void MicQuizPauseOpen(void);
void MicQuizPauseClose(void);
void MicQuizTwinkle(void);
void MicQuizEventPulse(void);
void MicQuizMicHudShow(s32 playerIndex);
void MicQuizScoreDigitsUpdate(void);
void MicQuizPauseDispUpdate(void);
void MicQuizPauseDispOn(void);
void MicQuizPauseDispOff(void);
void MicQuizCompactHudDrawNormal(void);
void MicQuizCompactHudDrawPause(void);
void MicQuizSparksExpand(void);
void MicQuizSparksFade(void);
s32 MicQuizResponseCompare(s32 response, s32 expectedAnswer);
s32 MicQuizResponseRead(s32 playerIndex, s32 responseMode);
void MicQuizIndicatorSwing(void);
void MicQuizMicHudStateSet(s32 displayState);
void MicQuizMicPulseStart(void);
void MicQuizMicPulseStop(void);
void MicQuizSelectionItemsSet(s32 select, s32 padNo);
void MicQuizSelectionNumbersSet(s32 padNo, s32 direction);
void MicQuizTimeoutWatchAlt(void);
void MicQuizMicHudMotionSet(s32 motionState);
void MicQuizMicHudIdle(void);
void MicQuizMicGlowFadeOut(void);
void MicQuizMicGlowFadeIn(void);
void MicQuizTempMesKill(HUPROCESS* parent);
void MicQuizTempMesClose(void);
void MicQuizTempMes(void);
s32 MicQuizCountdownUpdate(void);
void MicQuizCountdownDigitsSet(s32 pointsRemaining);
s32 MicQuizRandRange(s32 minimum, s32 maximum);
s32 MicQuizRandChance(s32 percentChance);
void MicQuizAcceptExchange(s32 primaryMessageIndex, s32 secondaryMessageIndex);
s32 MicQuizRecognizedAnswerGet(void);
void MicQuizPresenterCue(void);
int MicQuizChoiceFind(int key);
int MicQuizChoiceAvailable(int key);
void MicQuizPauseAllowSet(s32 pauseAllowed);
void MicQuizMicHudHide(void);
s32 MicQuizQuestionEventGet(void);
void MicQuizHintClear(void);
void MicQuizInstructionClear(void);
void MicQuizPauseBgDraw(s16 layerNo);
void MicQuizPauseWatch(void);
void MicQuizCompactHudCreate(void);
void MicQuizSpritePulseRandom(void);
void MicQuizPauseDispCreate(void);
void MicQuizSparkFadeWide(void);
void MicQuizSparkFadeSmall(void);
void MicQuizRingEmit(void);
void MicQuizRing(void);
void MicQuizFeedbackStart(s32 accepted);
void MicQuizFeedbackFlash(void);
void MicQuizMicHudPulse(void);
void MicQuizEventIconEffect(s32 accepted, s32 questionType);
void MicQuizEventIconBreak(void);
void MicQuizEventFragment(void);
void MicQuizEventIconExpand(void);
void MicQuizEventIconShrink(void);
void MicQuizEmptyA(void);
void MicQuizEmptyB(void);
u8 MicQuizQuestionLevelGet(void);
u8 MicQuizQuestionColumnGet(void);
void MicQuizRoundResultSet(s8 playerNo, u16 points, s32 result);
s32 MicQuizCurrentPlayerGet(void);
s32 MicQuizPlayerActiveCheck(s32 playerNo);
f32 MicQuizTimeLeftGet(void);
s16 MicQuizWinGet(s32 windowNo);
void MicQuizWinClose(void);
void MicQuizWaitTwoSeconds(void);
void MicQuizWaitOneSecond(void);
s32 MicQuizPresenterLayoutGet(void);
void MicQuizPresentationRequest(void);
void MicQuizPlayerNamePresent(void);
void MicQuizPromptPresent(u32 message);
void MicQuizPromptAltPresent(u32 message);
void MicQuizTimeoutFollowupPresent(void);
void MicQuizResponseFollowupPresent(void);
void MicQuizNextResponsePresent(void);
void MicQuizResponseMesPresent(u32 message);
void MicQuizFollowupPresent(void);
void MicQuizFollowupAltPresent(void);
void MicQuizRoundFollowupNextPresent(void);
void MicQuizRoundFollowupPresent(void);
void MicQuizQuestionIntroPresent(void);
void MicQuizPlayerCallPresent(s32 messageIndex);
void MicQuizEffectParentSet(void);
void MicQuizEffectParentSetAlt(void);
void MicQuizResponseRun(s32 quizMode);
void MicQuizResponseRunAlt(s32 mode);
void MicQuizTimeoutWatch(void);
void MicQuizAnswerMesPresent(s32 messageType, s32 alternate);
void MicQuizNumberAnswerPresent(s32 messageIndex);
void MicQuizEmptyC(void);
void MicQuizTempMesStart(void);
void MicQuizReactionLeadIn(void);
void MicQuizPortraitHide(int playerIndex);
void MicQuizMarkerShow(void);
void MicQuizMarkerHide(void);
void MicQuizWinSoundCallback(HUWINID windowId, u32 answerId, s16 soundIndex);

#endif
