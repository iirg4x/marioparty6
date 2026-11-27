#ifndef REL_MINIGAME_PARTICLE_MODELS_H
#define REL_MINIGAME_PARTICLE_MODELS_H

#include "dolphin/types.h"

/* Effect model IDs and their particle work allocations. */
typedef struct MinigameParticleModels {
    s16 modelIds[1];
    s16 unk02[1]; /* The game's code never reads this field. */
    void *allocation[1];
} MinigameParticleModels;

#endif
