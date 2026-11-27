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
    s32 unk24; /* The game's code never reads this field. */
    HU3D_MODELID model; /* Model that draws the effect's particles. */
} M619ParticleEffect;

/* Functions shared by the module's source files. */
void fn_1_C69C(M619ParticleEffect *work, const HuVecF *position,
               const HuVecF *rotation);

/* Particle model setup and emission. */
void fn_1_C890(M619ParticleEffect *effect);
void fn_1_C8CC(M619ParticleEffect *effect);
void fn_1_C8D8(M619ParticleEffect *flash, HuVecF *position);
void fn_1_CE20(M619ParticleEffect *work, const HuVecF *position,
               const HuVecF *rotation);
void fn_1_D420(M619ParticleEffect *work, const HuVecF *position,
               const HuVecF *rotation);
void fn_1_D9EC(M619ParticleEffect *work, const HuVecF *position,
               const HuVecF *rotation);

#endif
