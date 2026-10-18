/* Model and animation state for microphone mode's selection screens. */
#ifndef REL_MDMICDLL_MODEL_STATE_H
#define REL_MDMICDLL_MODEL_STATE_H

#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "game/hu3d.h"

typedef struct MDMICModelWork {
    HU3D_MODELID modelId; /* Model displayed by this work entry. */
    HU3D_ANIMID animId[4]; /* Animation IDs associated with the model. */
    HuVecF pos; /* Model position. */
    HuVecF rot; /* Model rotation in degrees. */
    HuVecF scale; /* Model scale on each axis. */
    s16 highlightSpinFrame; /* Counter for the selection spin update, capped at 35. */
    u8 reservedState[6]; /* Additional model state with no established use. */
} MDMIC_MODEL_WORK;

extern MDMIC_MODEL_WORK lbl_1_bss_29C[25];

#endif
