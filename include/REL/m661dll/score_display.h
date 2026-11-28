#ifndef M661DLL_SCORE_DISPLAY_H
#define M661DLL_SCORE_DISPLAY_H

#include "game/mg/score.h"

typedef struct ScoreDisplay {
    HUSPR_GROUPID characterBox;
    MGSCORE *score;
} ScoreDisplay;

#ifdef __cplusplus
extern "C" {
#endif
extern ScoreDisplay lbl_1_bss_0[2];
void fn_1_249A8(void);

/* Score display updates. */
/* Returns whether the round is finished: the timer ran out or the two scores total 64. */
s16 fn_1_5954(void);
void fn_1_59F0(void);
#ifdef __cplusplus
}
#endif

#endif
