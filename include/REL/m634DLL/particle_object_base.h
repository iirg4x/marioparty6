#ifndef REL_M634_PARTICLE_OBJECT_BASE_H
#define REL_M634_PARTICLE_OBJECT_BASE_H
#include "REL/framework/game_object.h"
class ParticleObjectBase : public GameObject {
public:
    ParticleObjectBase(int category) : GameObject(category) {}
    virtual ~ParticleObjectBase() {}
    virtual void mode(s32) {}
};
#endif
