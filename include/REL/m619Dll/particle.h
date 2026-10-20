/* Describes particle effects and the transforms sampled by their emitters. */
#ifndef REL_M619DLL_PARTICLE_H
#define REL_M619DLL_PARTICLE_H
#include "game/hu3d.h"

/* Defines the particle model and source transforms retained by each effect. */
typedef struct M619ParticleEffect {
    HuVecF position; /* Cached position vector for the effect. */
    HuVecF rotation; /* Cached rotation vector in degrees. */
    const HuVecF *positionSource; /* Position sampled when particles spawn. */
    const HuVecF *rotationSource; /* Rotation in degrees used when particles spawn. */
    s32 state; /* Effect-specific particle lifecycle state. */
    s32 active; /* Whether this effect is active. */
    HU3D_MODELID model; /* Model that draws the effect's particles. */
} M619ParticleEffect;
#endif
