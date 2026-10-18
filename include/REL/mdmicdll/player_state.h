/* Per-player choices used by microphone mode's setup screens. */
#ifndef REL_MDMICDLL_PLAYER_STATE_H
#define REL_MDMICDLL_PLAYER_STATE_H

#include "dolphin/types.h"

typedef struct MDMICPlayerWork {
    s16 characterChoiceState; /* Progress state of this player's character selection. */
    s16 reservedSetup; /* Player's team assignment, 0 or 1. */
    s16 isComputer; /* Whether this player is computer-controlled. */
    s16 computerDifficulty; /* Difficulty selected for a computer player. */
    s16 chrSel; /* Selected character number. */
    s16 padNo; /* Controller number assigned to the player. */
    s16 handicap; /* Handicap value selected for the player. */
} MDMIC_PLAYER_WORK;

extern MDMIC_PLAYER_WORK lbl_1_bss_9A4[4];

#endif
