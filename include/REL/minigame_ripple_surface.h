#ifndef REL_MINIGAME_RIPPLE_SURFACE_H
#define REL_MINIGAME_RIPPLE_SURFACE_H

#include "game/animdata.h"

typedef struct MinigameRippleSurface {
    s16 model;                     /* HSF model used for the animated surface. */
    void *textureBuffer;           /* 640x480 RGB565 frame copy sampled by its material hook. */
    ANIMDATA *bumpAnimation, *reflectionAnimation; /* Bump and reflection bitmaps, GX maps 1 and
                                                    * 2. */
    f32 textureScrollOffset;       /* Horizontal texture-matrix offset, advanced each frame. */
    f32 rippleX[32];               /* Ripple center along model X; -100000 marks an unused slot. */
    f32 rippleHeight[32];           /* Vertical crest displacement in model units. */
    f32 rippleRadius[32];           /* Expanding radius in model units; a ripple ends above 500. */
} MinigameRippleSurface;

#endif
