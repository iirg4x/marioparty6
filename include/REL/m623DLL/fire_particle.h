#ifndef REL_M623DLL_FIRE_PARTICLE_H
#define REL_M623DLL_FIRE_PARTICLE_H
#include "REL/framework/model.h"
#include "REL/framework/particle.h"
#include "REL/framework/game_object.h"

// Owns the fire particles advanced by the model hook.
class FireParticle : public GameObject, public Particle {
  public:
    explicit FireParticle(ANIMDATA *animation);
    virtual ~FireParticle();
    virtual void update();
    void fn_1_5F00(HU3D_MODEL *model, HU3D_PARTICLE *particles, Mtx matrix);
};
#endif
