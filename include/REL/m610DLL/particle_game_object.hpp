#ifndef REL_M610DLL_PARTICLE_GAME_OBJECT_HPP
#define REL_M610DLL_PARTICLE_GAME_OBJECT_HPP
#include "REL/framework/game_object.h"
#include "REL/framework/particle.h"
/* Own the particle model and dispatch its update hook. */
class ParticleGameObject : public GameObject
{
  public:
    Particle model;
    ParticleGameObject(ANIMDATA *animation, Vec *position);
    virtual ~ParticleGameObject();
    virtual void update();
    void fn_1_137EC();
};

/* Own the cloud model and dispatch its update hook. */
class ParticleCloudObject : public GameObject
{
  public:
    Particle model;
    ParticleCloudObject(ANIMDATA *animation, Vec *position);
    virtual ~ParticleCloudObject();
    virtual void update();
};
#endif
