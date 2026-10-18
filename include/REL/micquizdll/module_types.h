/* Shared question selection, scoring, and pause state for Speak Up. */
#ifndef REL_MICQUIZDLL_MODULE_TYPES_H
#define REL_MICQUIZDLL_MODULE_TYPES_H

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

/* Quiz state shared by the scene controller and microphone response processes. */
typedef struct MicquizModeWorkView_s {
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
    s32 setupValue; /* Initialized to one at quiz setup; not read by this quiz code. */
} MicquizModeWorkView;

extern MicquizModeWorkView *lbl_1_bss_4D8;
extern ANIMDATA *lbl_1_bss_30;
extern ANIMDATA *lbl_1_bss_34;

#endif
