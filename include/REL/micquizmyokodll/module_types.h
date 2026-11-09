#ifndef _REL_MICQUIZMYOKODLL_MODULE_TYPES_H_
#define _REL_MICQUIZMYOKODLL_MODULE_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "math.h"
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

void HuSprTexLoad(ANIMDATA *anim, s16 bmpNo, s16 texMapId, GXTexWrapMode wrapS,
                  GXTexWrapMode wrapT, GXTexFilter filter);
void fn_1_9F08(void);
void fn_1_DDF8(void);
void fn_1_10C08(void);
void fn_1_1122C(s32 playerIndex);
void fn_1_11D38(void);
void fn_1_11F34(void);
void fn_1_12144(void);
void fn_1_12340(s32 messageIndex);
void fn_1_127CC(s32 quizMode);
void fn_1_15D80(s32 select, s32 padNo);
void fn_1_15FB0(void);
void fn_1_16574(void);
void fn_1_1850C(s32 primaryMessageIndex, s32 secondaryMessageIndex);
void fn_1_18964(s32 primaryMessageIndex, s32 secondaryMessageIndex);
void fn_1_18DD0(s32 primaryMessageIndex, s32 secondaryMessageIndex);
void fn_1_19118(s32 messageIndex);
void fn_1_19288(void);
void fn_1_19B30(s32 motionNo, f32 startTime, u32 attr);
void fn_1_8200(void);
void fn_1_8280(s32 modelIndex);
void fn_1_829C(s16 layerNo);
void fn_1_1BAC(s32 messageSet);
void fn_1_3258(void);
void fn_1_9868(void);
void fn_1_D4F4(void);
void fn_1_E7EC(void);
void fn_1_E8A8(void);
void fn_1_E964(s32 action);
void fn_1_EC84(void);
void fn_1_ECE0(void);
void fn_1_ED38(void);
void fn_1_ED90(void);
void fn_1_F09C(void);
void fn_1_F334(void);
void fn_1_F9F4(s32 flag, s32 mode);
void fn_1_FF84(void);
void fn_1_100DC(void);
s32 fn_1_102B0(s32 player);
void fn_1_8F58(void);
void fn_1_10E34(void);
void fn_1_11030(void);
void fn_1_11940(void);
void fn_1_16E74(void);
void fn_1_16F6C(void);
void fn_1_177F0(u32 message);
void fn_1_17A30(s32 choice);
void fn_1_17CA8(u32 choice);
void fn_1_17F24(void);
void fn_1_C1E4(void);
void fn_1_C558(void);
void fn_1_E1E4(void);
void fn_1_E4E8(void);
void fn_1_FB04(void);

typedef struct MicquizModeWork_s {
    s32 playerActive[4]; /* Nonzero for each participating player. */
    u8 playerCount; /* Number of participating players. */
    u8 questionCount; /* Current question count shown by the pause display. */
    u8 currentPlayer; /* Player whose turn selects the question; zero through three. */
    s8 answeringPlayer; /* Player stored with the round result; minus one before a response. */
    s8 responseRecord[4]; /* Response outcomes in the first pair, answering player numbers in the
                           * last pair. */
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
} MicquizModeWork;

typedef struct MicquizControlWork_s {
    s16 sideModelIds[2];
    s16 shadowModelIds[2];
    s16 modelIds[5];
    s16 phase;
    s16 timer;
    s16 completionFlag;
    s16 characterModelsCreated;
    u8 mode;
    u8 currentPlayer;
    u8 flag_80 : 1;
    u8 flag_40 : 1;
    u8 unknown_1C_2 : 6;
} MicquizControlWork;

typedef struct MicquizAnimationState_s {
    s16 modelIds[5];
    s16 states[5];
    s16 frame;
    s16 currentModel;
    s16 animationIds[5];
    u8 unknown_22[2]; /* No code names this field. */
    ANIMDATA *spriteAnimations[5];
    s16 playerStates[5];
} MicquizAnimationState;

typedef struct MicquizHostState_s {
    s16 charNo;
    s16 modelId;
    s16 motionIds[9];
} MicquizHostState;

typedef struct MicquizPlayerWork_s {
    s16 positionIndex;
    s16 characterId;
    s16 accessorySetIndex;
    s16 accessoryCount;
    s16 secondarySetIndex;
    s16 secondaryCount;
    s16 motionStep;
    s16 modelId;
    s16 alternateModelId;
    s16 motionId;
    s16 characterModelId;
    s16 characterMotions[9];
    s16 models_28[5];
    s16 models_32[5];
    f32 motionValueA;
    f32 motionValueB;
    f32 motionValueC;
    f32 motionValueD;
} MicquizPlayerWork;

typedef struct MicquizAccessoryPoolEntry_s {
    s16 modelId;
    s16 jointMotions[4];
} MicquizAccessoryPoolEntry;

typedef struct MicquizSequence_s {
    s16 playerIndex;
    s16 modelIndex;
    s16 phase;
    s16 playerSlot;
    s16 responseMode;
    s16 motionStep;
    s16 timer;
    s16 unknown_0E;
    f32 sequenceValue;
    f32 sequenceOffset;
    f32 oscillationPhase;
    f32 verticalMotion;
} MicquizSequence;

#ifdef __cplusplus
}
#endif

#endif
