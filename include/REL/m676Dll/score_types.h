/* Score values and screen positions displayed for the four players. */
#ifndef REL_M676DLL_SCORE_TYPES_H
#define REL_M676DLL_SCORE_TYPES_H
#include "REL/minigame_score_entry.h"
#include "game/object.h"
#include "game/mg/score.h"
typedef struct M676ScoreWork {
    /* Score display phase and one record for each player. */
    s32 displayState;
    MinigameScoreEntry entries[4];
} M676ScoreWork;
typedef Vec M676ScorePosition;
void fn_1_11F34(HUPROCESS *process);
void fn_1_12514(s32 playerIndex, s32 scoreChange);
s32 fn_1_125E4(s32 playerIndex);
void fn_1_1263C(s32 playerIndex, f32 *x, f32 *y);
#endif
