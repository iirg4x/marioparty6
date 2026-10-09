/* Shared state for the motion-check surface model and ripple animation. */
#ifndef REL_SAFDLL_SAF_H
#define REL_SAFDLL_SAF_H
#include "game/hsfex.h"
typedef struct SafState {
    s16 model;                     /* HSF model used for the animated test surface. */
    void *textureBuffer;           /* 640x480 RGB565 frame copy sampled by its material hook. */
    ANIMDATA *bumpAnimation, *reflectionAnimation; /* Bump and reflection bitmaps, GX maps 1 and
                                                    * 2. */
    f32 textureScrollOffset;       /* Horizontal texture-matrix offset, advanced each frame. */
    f32 rippleX[32];               /* Ripple center along model X; -100000 marks an unused slot. */
    f32 rippleHeight[32];           /* Vertical crest displacement in model units. */
    f32 rippleRadius[32];           /* Expanding radius in model units; a ripple ends above 500. */
} SafState;
extern SafState lbl_1_bss_4;
#endif
