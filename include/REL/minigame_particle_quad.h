#ifndef REL_MINIGAME_PARTICLE_QUAD_H
#define REL_MINIGAME_PARTICLE_QUAD_H

#include "dolphin/gx.h"

/* Draw a textured quad centered on the current model origin. */
static inline void DrawParticleQuad(f32 halfWidth, f32 halfHeight)
{
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-halfWidth, halfHeight, 0.0f);
    GXTexCoord2f32(1.0f, 0.0f);
    GXPosition3f32(halfWidth, halfHeight, 0.0f);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition3f32(halfWidth, -halfHeight, 0.0f);
    GXTexCoord2f32(0.0f, 1.0f);
    GXPosition3f32(-halfWidth, -halfHeight, 0.0f);
    GXTexCoord2f32(1.0f, 1.0f);
}

#endif
