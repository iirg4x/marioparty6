/* Per-player score, display position, and score animation state. */
#ifndef REL_M674DLL_SCORE_H
#define REL_M674DLL_SCORE_H
#include "game/mg/score.h"

typedef struct M674ScoreEntry {
    s32 value; /* Displayed minigame coin total, capped at 99. */
    f32 x; /* Horizontal screen position in pixels. */
    f32 y; /* Vertical screen position in pixels. */
    s16 scoreBox; /* Character score-box identifier. */
    s16 spriteGroup; /* Two static decoration sprites displayed with the score box and numeric
                      * score. */
    MGSCORE *score; /* Numeric score display controlled by the manager. */
    s32 animationState; /* Idle or current phase of the score-pop animation. */
    f32 scale; /* Current multiplier applied to both score digits. */
} M674ScoreEntry;

typedef struct M674ScoreWork {
    s32 state; /* Score display phase: hidden, initial reveal, or active update. */
    M674ScoreEntry entries[4]; /* Per-player score display state. */
} M674ScoreWork;

#endif
