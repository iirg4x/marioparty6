/* Shared particle, quad, and texture state for this minigame's effects. */
#ifndef REL_M674DLL_EFFECTS_H
#define REL_M674DLL_EFFECTS_H
#include "game/object.h"
#include "game/sprite.h"
/* The same quad color as RGBA channels or one packed GX vertex value. */
typedef union M674EffectColor { GXColor channels; u32 packed; } M674EffectColor;
typedef struct M674EffectQuad {
    s32 enabled;              /* Nonzero while this ground quad is fading. */
    M674EffectColor color;    /* Vertex color, including its fading alpha. */
    f32 size;                 /* Base half-width in world units. */
    f32 sizeDelta;            /* Per-update change to sizeScale. */
    f32 sizeScale;            /* Current multiplier applied to size. */
    Vec position;             /* Center of the quad in world coordinates. */
} M674EffectQuad;
typedef struct M674TextureWork {
    GXTexObj outputTexture;       /* Captured scene image used on the water. */
    GXTexObj animationTexture;    /* Animated water pattern texture. */
    void *textureBuffer;          /* Render target storage for outputTexture. */
    void *textureData;             /* Packed source data for animationTexture. */
    ANIMDATA *animationData;      /* Sprite data whose bitmap zero supplies the second water
                                   * texture. */
    Mtx textureMatrices[4];       /* Water-surface world transform and three texture-coordinate
                                   * transforms. */
    f32 scrollX;                   /* Horizontal scroll offset in texture space. */
    f32 scrollY;                   /* Vertical scroll offset in texture space. */
} M674TextureWork;
typedef struct M674EffectWork {
    GXColor color;                 /* Color copied into newly emitted particles. */
    ANIMDATA *animation;           /* Animation data used by the quad renderer. */
    M674EffectQuad quads[128];     /* Reusable ground-quad slots. */
    s32 enabled;                   /* Enables projected-water updates. */
    M674TextureWork texture;       /* State for the projected water overlay. */
} M674EffectWork;
void fn_1_F660(Vec *origin);
void fn_1_F9A4(Vec *origin, f32 size, f32 scale, f32 scaleStep, u8 alpha);
#endif
