// Declares a scene object that owns a model-backed particle effect.

#ifndef REL_M673DLL_PARTICLE_GAME_OBJECT_H
#define REL_M673DLL_PARTICLE_GAME_OBJECT_H
#include "REL/framework/game_object.h"
#include "REL/framework/particle.h"

// Updates the particles in a scene particle model.
struct ParticleGameObject : GameObject {
    Particle model; // Engine particle model owned by this object.
    // Creates and positions the particle model, then initializes its particles.
    ParticleGameObject(ANIMDATA *animation, Vec *position);
    // Releases the particle model.
    virtual ~ParticleGameObject();
    // Deletes the particle object when the particle callback's live count reaches zero.
    virtual void update();
};

// Advances and fades the active particles.
void fn_1_1170C(HU3D_MODEL *model, HU3D_PARTICLE *particles, Mtx matrix);

#endif
