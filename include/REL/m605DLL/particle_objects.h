#ifndef REL_M605DLL_PARTICLE_OBJECTS_H
#define REL_M605DLL_PARTICLE_OBJECTS_H
#include "REL/framework/game_object.h"
#include <cmath>
#include "REL/framework/particle.h"

// Updates a particle system at a supplied position.
class ParticleGameObject : public GameObject {
public:
    Particle model;
    ParticleGameObject(ANIMDATA *animation, Vec *position);
    virtual ~ParticleGameObject();
    virtual void update();
};

// Updates the two particle systems owned by one game object.
class ParticlePairGameObject : public GameObject {
public:
    Particle model;
    Particle secondModel;
    ParticlePairGameObject(ANIMDATA *firstValue, ANIMDATA *secondValue);
    virtual ~ParticlePairGameObject();
    virtual void update();
};

void fn_1_34D0(GameObject *object, const Vec *input);

#endif
