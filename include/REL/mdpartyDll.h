/* Public entry points for Party Mode character and stage effects. */
#ifndef _MDPARTYDLL_H
#define _MDPARTYDLL_H

#include "game/object.h"

typedef struct MdpartyModelTransition {
    s16 state; /* Phase or completion state of the active model transition. */
    s16 unk02; /* No code names this field. */
    HuVecF pos; /* Starting position of the current transition, in world units. */
    HuVecF controlPos; /* Middle control point for the model's quadratic path, in world units. */
    HuVecF endPos; /* Destination of the model's quadratic path, in world units. */
    float time; /* Elapsed frames of the current transition phase. */
    float duration; /* Length of the current transition phase, in frames. */
} MdpartyModelTransition;

void fn_1_43F3C(s16 groupNo, HuVecF *pos, s16 mode);
void fn_1_44DCC(OMOBJMAN *objman);
void fn_1_4581C(void);
void fn_1_45A48(HuVecF *pos);
void fn_1_45C40(s16 playerNo, HuVecF *pos, s16 colorNo);
void fn_1_45F3C(s16 playerNo, HuVecF *pos, s16 colorNo, s16 mode);
void fn_1_463E0(void);
void fn_1_464A0(s16 groupNo, HuVecF *pos, s16 mode, s16 colorNo);

#endif
