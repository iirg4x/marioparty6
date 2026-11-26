#ifndef REL_MINIGAME_SCORE_ENTRY_H
#define REL_MINIGAME_SCORE_ENTRY_H

#include "game/mg/score.h"

typedef struct MinigameScoreEntry {
    s32 value; /* Displayed score, capped at 99. */
    f32 x; /* Score-box position in screen pixels. */
    f32 y; /* Score-box position in screen pixels. */
    s16 box; /* Score-panel group containing the player's character tile. */
    s16 decorationGroup; /* Sprite group for the two static score decorations. */
    MGSCORE *score; /* Numeric score renderer. */
    s32 animationState; /* Digit pop-animation phase. */
    f32 digitScale; /* Uniform scale used by the pop animation. */
} MinigameScoreEntry;

#endif
