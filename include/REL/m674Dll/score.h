/* Per-player score, display position, and score animation state. */
#ifndef REL_M674DLL_SCORE_H
#define REL_M674DLL_SCORE_H
#include "REL/minigame_score_entry.h"
#include "game/mg/score.h"

typedef struct M674ScoreWork {
    s32 state; /* Score display phase: hidden, initial reveal, or active update. */
    MinigameScoreEntry entries[4]; /* Per-player score display state. */
} M674ScoreWork;

#endif
