#ifndef REL_MINIGAME_TEXTURE_TRANSITION_H
#define REL_MINIGAME_TEXTURE_TRANSITION_H

#include "dolphin/gx.h"

typedef struct MinigameTextureTransition {
    f32 indirectMatrix[2][3]; /* Two indirect-texture transform rows. */
    GXColor color; /* Tint used by the overlay material. */
    float textureScrollY; /* Current vertical texture offset. */
    float textureScrollStep; /* Vertical offset added whenever the overlay material hook runs. */
    float remainingFrames; /* Frames left before the overlay is hidden. */
} MinigameTextureTransition;

#endif
