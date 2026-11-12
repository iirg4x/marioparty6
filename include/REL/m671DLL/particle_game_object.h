#ifndef REL_M671DLL_PARTICLE_GAME_OBJECT_H
#define REL_M671DLL_PARTICLE_GAME_OBJECT_H
#include "REL/framework/game_object.h"
#include "REL/framework/particle.h"

class ParticleGameObject : public GameObject {
public:
    Particle model;
    ParticleGameObject(ANIMDATA *, Vec *);
    virtual ~ParticleGameObject();
    virtual void update();
};
class TimedParticleEffect : public GameObject {
public:
    s16 modelId;
    HU3D_PARTICLE *owner;
    Vec position;
    f32 elapsedTime, duration;
    s32 state;
    u8 active;
    TimedParticleEffect(ANIMDATA *, Vec *, Vec *);
    virtual ~TimedParticleEffect();
    virtual void update();
    void draw(Mtx);
};
#endif
