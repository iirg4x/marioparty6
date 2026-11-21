// Model handles and particle effects used by the C++ minigames.
#ifndef REL_FRAMEWORK_PARTICLE_H
#define REL_FRAMEWORK_PARTICLE_H

#include "REL/framework/model.h"

extern "C" {
#include <math.h>
#include "dolphin/os/OSCache.h"
#include "game/hu3d.h"
}

// Owns a fixed-capacity engine particle model with an effect's update callback.
class Particle : public Model {
public:
    HU3D_PARTICLE_HOOK updateHook; // Required effect callback invoked when the engine dispatches a
                                   // draw update.

    Particle(HU3D_PARTICLE_HOOK hook, ANIMDATA *animation, int capacity);
    ~Particle();
    HU3D_PARTICLE *GetParticle();
    HU3D_PARTICLE_DATA *Begin();
    HU3D_PARTICLE_DATA *End();
    static void Update(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
    void SetScale(float scale);
    void SetTransparency(float level);
};

// Particle property setters use the stored model ID to select the engine's particle pool.
inline short ParticleModelId(const Particle *particle)
{
    return particle->modelId;
}

#endif
