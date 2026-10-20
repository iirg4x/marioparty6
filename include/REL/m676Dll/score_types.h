/* Score values and screen positions displayed for the four players. */
#ifndef REL_M676DLL_SCORE_TYPES_H
#define REL_M676DLL_SCORE_TYPES_H
#include "game/object.h"
#include "game/mg/score.h"
typedef struct M676ScoreEntry {
    /* Displayed score and its screen position in pixels. */
    s32 value;
    f32 x, y;
    /* Score-box ID and the additional two-sprite group ID. */
    s16 box, extraSprites;
    /* Score display object, animation state, and digit scale. */
    MGSCORE *score;
    s32 animationState;
    f32 digitScale;
} M676ScoreEntry;
typedef struct M676ScoreWork {
    /* Score display phase and one record for each player. */
    s32 displayState;
    M676ScoreEntry entries[4];
} M676ScoreWork;
typedef Vec M676ScorePosition;
#endif
