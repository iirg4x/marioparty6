#ifndef REL_MINIGAME_PARTICLE_SYSTEM_H
#define REL_MINIGAME_PARTICLE_SYSTEM_H

#include "game/animdata.h"

/* Resource, creation settings and handles for a particle system. */
typedef struct MinigameParticleSystem {
    u32 flags; /* Reuse and initialization bits. */
    s32 resourceId; /* Packed sprite animation resource number. A negative value ends the list. */
    s32 count; /* Maximum number of particle slots. */
    s32 blendMode; /* Blend mode passed to the particle renderer. */
    s32 modelId; /* Particle-system model handle. */
    ANIMDATA *unk14; /* The game's code never reads this field. */
    u8 unk18[4]; /* No code names this field. */
} MinigameParticleSystem;

#endif
