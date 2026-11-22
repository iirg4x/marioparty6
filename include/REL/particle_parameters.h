#ifndef REL_PARTICLE_PARAMETERS_H
#define REL_PARTICLE_PARAMETERS_H

#include "dolphin/gx.h"
#include "dolphin/mtx.h"

/* Mutable per-particle work initialized by emitters in m617dll, m660dll and m662dll.
 * The color and motion hooks update this work. The m660dll object-follow hook positions
 * its particles directly. */
typedef struct ParticleParameters {
    u32 unk00; /* The game's code never reads this field. */
    /* Emitters clear this at spawn. In m617dll, particles attached to a player accumulate
     * lifetime-weighted player displacement here. Their position combines this offset with
     * the saved spawn position and the accumulated effect displacement. */
    Vec playerMovementOffset;
    GXColor startColor; /* Initial engine particle RGBA color. */
    /* Destination RGB for the per-frame color change. Emitters also initialize its alpha. */
    GXColor targetColor;
    f32 velocityScale[3]; /* Per-frame X/Y/Z velocity multipliers. */
    f32 verticalAcceleration; /* Added to Y velocity each frame. */
    f32 rotationIncrement; /* Added to Z rotation each frame. */
    /* Updated by scaleIncrementMultiplier, then added to particle scale each frame. */
    f32 scaleIncrement;
    f32 scaleIncrementMultiplier; /* Per-frame multiplier for scaleIncrement. */
    f32 alphaIncrement; /* Added to alpha in byte units each frame. */
    f32 colorBlendStep; /* Fraction of the starting-to-target RGB difference added each frame. */
} ParticleParameters;

#endif
