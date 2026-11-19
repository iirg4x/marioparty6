// Model handles and particle effects used by the C++ minigames.
#ifndef REL_FRAMEWORK_PARTICLE_H
#define REL_FRAMEWORK_PARTICLE_H

#include "REL/framework/model.h"

extern "C" {
#include <math.h>
#include "dolphin/os/OSCache.h"
#include "game/hu3d.h"
}

#include "REL/framework/particle_class.h"

// Particle property setters use the stored model ID to select the engine's particle pool.
inline short ParticleModelId(const Particle *particle)
{
    return particle->modelId;
}

#endif
