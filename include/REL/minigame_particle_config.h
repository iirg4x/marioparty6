#ifndef REL_MINIGAME_PARTICLE_CONFIG_H
#define REL_MINIGAME_PARTICLE_CONFIG_H

#include "dolphin/gx.h"
#include "dolphin/mtx.h"

typedef struct MinigameParticleConfig {
    u32 flags; /* Particle behavior flags, including alternating-size animation. */
    GXColor endColor; /* Initial RGBA at emission and the target RGB for interpolation. */
    GXColor startColor; /* Starting RGB for interpolation. Alpha is updated separately. */
    Vec velocity, decay; /* Initial velocity and per-frame velocity multipliers. */
    float gravity; /* Vertical velocity added each frame. */
    float unk28; /* No code names this field. */
    float scaleDelta; /* Per-frame change to the particle's base size. */
    float alphaDelta; /* Per-frame change to particle transparency. */
    float colorTime; /* Interpolation amount between the configured tint colors. */
} MinigameParticleConfig;

#endif
