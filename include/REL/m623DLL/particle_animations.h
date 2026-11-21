#ifndef REL_M623DLL_PARTICLE_ANIMATIONS_H
#define REL_M623DLL_PARTICLE_ANIMATIONS_H
#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"
extern "C" {
#include "game/sprite.h"
}

// Owns the animations and updates the three particle effect lists.
class ParticleAnimations : public RequestObject, public MinigameSingleton<ParticleAnimations> {
  public:
    ParticleAnimations();
    virtual ~ParticleAnimations();
    virtual void flagEnabled(int request);
    virtual void update();
    ANIMDATA *animations[3];
};
#endif
