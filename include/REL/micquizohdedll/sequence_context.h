#ifndef REL_MICQUIZOHDEDLL_SEQUENCE_CONTEXT_H
#define REL_MICQUIZOHDEDLL_SEQUENCE_CONTEXT_H

#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
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

typedef struct MicQuizResponse {
    s16 status;
    u16 reserved;
    u32 contextData[2];
    s32 choice;
} MicQuizResponse;

typedef struct MicQuizResponseStorage {
    MicQuizResponse *response;
    u32 unknown; /* Unknown word. */
} MicQuizResponseStorage;

/* Quiz state in MgModeWork; unaccessed bytes have no known meaning. */
typedef struct MicquizModeWork_s {
    s32 playerActive[4];
    u8 secondaryScoreUnits;
    u8 primaryScoreValue;
    u8 currentPlayerIndex;
    s8 responsePlayerIndex;
    u8 answerResult[2]; /* Answer results followed by the corresponding players. */
    s8 answerPlayer[2];
    s32 responseState;
    s16 selectedValue;
    u8 timingIndex;
    u8 field_1F;
    u8 activeMode;
    u8 field_21;
    u8 unknown_22[0x1A]; /* No code names this field. */
    s16 playerScore[4];
    u8 field_44[4];
    u8 characterIndex[4];
    u16 timingMode;
    u8 unknown_4E[2]; /* No code names this field. */
    s32 windowMode;
    s32 waitFlag;
    s32 modeFlag;
    s32 homeMenuEnabled;
    s32 drawMode;
    s32 randomizeSide;
    u8 unknown_68[0x18]; /* No code names this field. */
    s32 sideSelection;
} MicquizModeWork;

typedef struct MicquizResultCopy {
    s32 key;
    s32 result;
    u8 unknown_08[4]; /* No code names this field. */
    s32 state0C;
} MicquizResultCopy;

extern ANIMDATA *micQuizEventFragmentAnim[2];
extern ANIMDATA *micQuizEventIconAnim;
extern MicquizModeWork *micQuizModeWork;
extern s16 micQuizMicContext;
extern HUPROCESS *micQuizParentProc;
extern MicquizResultCopy micQuizChoiceRecords[8];
extern s32 micQuizResponseResult;
extern HUSPR_GROUPID micQuizCountdownGroup[3];
extern s16 micQuizDialogWin0[4];

extern MicquizModeWork *lbl_1_bss_4D8;
extern ANIMDATA *lbl_1_bss_30;
extern ANIMDATA *lbl_1_bss_34;
extern void *lbl_1_bss_568;
extern void *lbl_1_bss_55C[2];
void fn_1_548(OMOBJ *obj);
void MicQuizPauseWatch(void);
void MicQuizHudCreate(void);
void MicQuizEventSpark(void);
void MicQuizHudDrawNormal(void);
void MicQuizHudDrawPause(void);
void MicQuizIndicatorSet(s32 action);
void MicQuizEventIconBreak(void);
void MicQuizFollowupPresent(void);
void MicQuizRoundFollowupNextPresent(void);
void MicQuizRoundFollowupPresent(void);
void MicQuizQuestionIntroPresent(void);
s32 MicQuizResponseRead(s32 playerIndex, s32 responseMode);
void MicQuizListenWatch(void);
void MicQuizTimeoutWatchAlt(void);
void MicQuizMicGlowFlash(void);
void MicQuizAnswerPresent(u32 message);
void MicQuizAcceptExchange(s32 primaryMessageIndex, s32 secondaryMessageIndex);
void fn_1_6D8(void);
void fn_1_19AE4(s16 layerNo);
void fn_1_558(OMOBJ *obj);
void MicQuizNumberAnswerPresent(s32 messageIndex);
void MicQuizNumberAnswerAltPresent(s32 messageIndex);
void MicQuizPauseOpen(void);
void MicQuizPauseClose(void);
void MicQuizPauseDispCreate(void);
void MicQuizPauseDispUpdate(void);
void MicQuizPauseDispOn(void);
void MicQuizPauseDispOff(void);
void MicQuizCompactHudDrawNormal(void);
void MicQuizCompactHudDrawPause(void);
void MicQuizSparksExpand(void);
void MicQuizSparksFade(void);
void MicQuizSparksSmall(void);
void MicQuizEventIconEffect(s32 action, s32 playerCount);
void MicQuizEventIconExpand(void);
void MicQuizEventIconShrink(void);
s32 MicQuizPlayerActiveCheck(s32 playerIndex);
void MicQuizNextPromptPresent(void);
void MicQuizTimeoutFollowupPresent(void);
void MicQuizResponseFollowupPresent(void);
void MicQuizPlayerResponsePresent(s32 playerIndex);
void MicQuizPlayerCallPresent(s32 choice);
void MicQuizSelectionItemsSet(s32 select, s32 padNo);
void MicQuizAnswerMesPresent(s32 mode, s32 wait);
void MicQuizChoiceExchange(s32 primaryMessageIndex, s32 secondaryMessageIndex);
void MicQuizFlaggedChoiceExchange(s32 primaryMessageIndex, s32 secondaryMessageIndex);
void MicQuizReactionLeadIn(void);
void MicQuizAnswerFeedback(s32 isCorrect, s32 playerIndex);
void MicQuizPresenterMotionSet(s32 motionIndex, f32 blendTime, u32 duration);
void MicQuizPortraitHide(int base);

void MicQuizMarkerShow(void);
void MicQuizMarkerHide(void);
void fn_1_14F00(s32 sourceLeftInput, s32 sourceTopInput, s32 sourceWidthInput,
                s32 sourceHeightInput, s32 textureFormatInput, f32 speed);
void fn_1_15F60(s32 sourceLeft, s32 sourceTop, s32 sourceWidth, s32 sourceHeight,
                GXTexFmt textureFormat, f32 speed);
void fn_1_16804(s32 sourceLeft, s32 sourceTop, s32 sourceWidth, s32 sourceHeight,
                GXTexFmt textureFormat, f32 strength);
void fn_1_17478(s32 sourceLeft, s32 sourceTop, s32 sourceWidth, s32 sourceHeight,
                enum _GXTexFmt textureFormat, f32 strength);
void fn_1_187F0(s32 sourceLeft, s32 sourceTop, s32 sourceWidth, s32 sourceHeight,
                GXTexFmt textureFormat, f32 strength);
void fn_1_195DC(s16 unused);
void fn_1_19B24(s16 level);
void fn_1_1A550(s16 layerNo);
void fn_1_1AA0C(s16 unused);
void fn_1_1ADAC(void);
void fn_1_1BD68(void *hookContext, f32 (*positionMatrix)[4]);
void fn_1_1C088(HU3D_MODEL *model, f32 (*positionMatrix)[4]);
void ObjectSetup(void);
void MicQuizMicHudStateSet(s32 mode);
void MicQuizRingsStart(void);
void MicQuizRingsStop(void);
void MicQuizMicHudMotionSet(s32 mode);
void MicQuizPauseAllowSet(s32 value);

#endif
