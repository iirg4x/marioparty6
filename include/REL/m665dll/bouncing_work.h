#ifndef REL_M665DLL_BOUNCING_WORK_H
#define REL_M665DLL_BOUNCING_WORK_H

#include "dolphin/types.h"

typedef struct M665BouncingWork {
    s16 model; /* Detached effect model ID. */
    s16 state; /* Detached-effect state. */
    s16 blinkFrames; /* Blink counter; parity alternates visibility each update. */
    f32 velocityX; /* X movement per update. */
    f32 velocityY; /* Y movement per update. */
    f32 velocityZ; /* Z movement per update. */
} M665BouncingWork;

#endif
