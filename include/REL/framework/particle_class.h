#ifndef REL_FRAMEWORK_PARTICLE_CLASS_H
#define REL_FRAMEWORK_PARTICLE_CLASS_H

#include "REL/framework/model.h"
#include "game/hu3d_particle_hook.h"

extern "C" {
typedef struct AnimData_s ANIMDATA;
typedef struct Hu3DParticleData_s HU3D_PARTICLE_DATA;
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

#endif
