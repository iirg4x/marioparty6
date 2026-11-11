#ifndef REL_MICQUIZDLL_MODULE_TYPES_H
#define REL_MICQUIZDLL_MODULE_TYPES_H

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

typedef struct MicquizModeWorkView_s {
    s32 playerStatus[4];
    u8 displayValueInFives;
    u8 displayValue;
    u8 playerIndex;
    s8 selectedPlayer;
    u8 unknown_14[4]; /* No code names this field. */
    s32 result;
    s16 selectedValue;
    u8 configurationIndex;
    u8 field_1F;
    u8 selectionType;
    u8 field_21;
    u8 unknown_22[0x1A]; /* No code names this field. */
    s16 field_3C[4];
    u8 field_44[4];
    u8 playerCharacter[4];
    u16 timerAndDisplayMode;
    u8 unknown_4E[2]; /* No code names this field. */
    s32 dialogueWindowMode;
    s32 characterActionPending;
    s32 field_58;
    s32 field_5C;
} MicquizModeWorkView;

typedef struct MicquizPauseModeWorkView_s {
    MicquizModeWorkView mode;
    s32 pauseActive;
} MicquizPauseModeWorkView;

extern MicquizModeWorkView *lbl_1_bss_4D8;
extern ANIMDATA *lbl_1_bss_30;
extern ANIMDATA *lbl_1_bss_34;

#endif
