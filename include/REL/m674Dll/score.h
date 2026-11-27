/* Per-player score, display position, and score animation state. */
#ifndef REL_M674DLL_SCORE_H
#define REL_M674DLL_SCORE_H
#include "REL/minigame_score_entry.h"
#include "game/mg/score.h"

typedef struct M674ScoreWork {
    s32 state; /* Score display phase: hidden, initial reveal, or active update. */
    MinigameScoreEntry entries[4]; /* Per-player score display state. */
} M674ScoreWork;

/* Score and result updates. */
s32 fn_1_100F8(s32 scoreIndex);
void fn_1_10028(s32 scoreIndex, s32 coinAmount);
void fn_1_10150(s32 scoreIndex, f32 *x, f32 *y);

#endif
