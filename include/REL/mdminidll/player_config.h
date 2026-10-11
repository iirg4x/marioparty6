/* Player choices and controller assignments collected by the minigame-mode menu. */
#ifndef REL_MDMINIDLL_PLAYER_CONFIG_H
#define REL_MDMINIDLL_PLAYER_CONFIG_H
#include "dolphin/types.h"

/* These choices are copied to the game player configuration when a mode starts. */
typedef struct MDMinidllPlayerConfig {
    s16 selectionPhase; /* Selection phase: 0 choosing, 1 character chosen, 2 CPU difficulty
                         * chosen. */
    s16 unusedSetupValue; /* Cleared on player setup; the menu does not otherwise access this
                           * value. */
    s16 comF; /* 0 for a human player, 1 for a CPU player. */
    s16 comDif; /* CPU difficulty index, 0-2 or 0-3 when the extra difficulty is available. */
    s16 charNo; /* Character-card index selected for this player. */
    s16 padNo; /* Controller index assigned to this player, 0-3. */
    u8 unusedTail[2]; /* Not accessed by the player-selection callbacks. */
} MDMinidllPlayerConfig;
extern MDMinidllPlayerConfig lbl_1_bss_7CC[4];
/* Entry 0 holds the human-player count; entry 1 holds the selected menu mode. */
extern s16 lbl_1_bss_804[];
#endif
