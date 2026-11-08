#ifndef REL_M663DLL_MODELS_H
#define REL_M663DLL_MODELS_H

#include "dolphin/types.h"
#include "dolphin/mtx.h"

typedef struct M663ModelWork {
    s16 modelId;
    s16 animationState;
    u8 unobservedModelData[4]; /* Unknown data, zeroed at initialization. */
    Vec position;
    Vec rotation;
    f32 scale;
} M663ModelWork;

extern M663ModelWork lbl_1_bss_750[86];

#endif
