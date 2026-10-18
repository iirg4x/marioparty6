/* Position curves used while microphone mode's characters and effects move. */
#ifndef REL_MDMICDLL_MOTION_STATE_H
#define REL_MDMICDLL_MOTION_STATE_H

#include "dolphin/types.h"
#include "dolphin/mtx/GeoTypes.h"

typedef struct MDMicTrajectory {
    s16 state; /* Current state of this movement. */
    HuVecF startPosition; /* Position at the start of the curve. */
    HuVecF controlPosition; /* Control point shaping the curve. */
    HuVecF endPosition; /* Destination position. */
    f32 elapsed; /* Time progressed along the curve. */
    f32 duration; /* Total time for the curve. */
} MDMIC_TRAJECTORY;

extern MDMIC_TRAJECTORY lbl_1_bss_118[4];
extern MDMIC_TRAJECTORY lbl_1_bss_1D8[2];
extern MDMIC_TRAJECTORY lbl_1_bss_238[2];

#endif
