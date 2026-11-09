#ifndef REL_M672DLL_PARTICLE_GAME_OBJECT_H
#define REL_M672DLL_PARTICLE_GAME_OBJECT_H

#include "REL/framework/game_object.h"
#include "REL/m672DLL/particle.h"

class ParticleGameObject : public GameObject {
public:
    Particle model;
    ParticleGameObject(ANIMDATA *animation, Vec *position);
    virtual ~ParticleGameObject();
    virtual void update();
};

#endif
