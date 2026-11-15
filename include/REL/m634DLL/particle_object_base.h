#ifndef REL_M634_PARTICLE_OBJECT_BASE_H
#define REL_M634_PARTICLE_OBJECT_BASE_H
#include "REL/framework/game_object.h"
class ParticleObjectBase : public GameObject {
public:
    // Initializes the particle object category.
    ParticleObjectBase(int category)
        : GameObject(category)
    {
    }
    // Releases the object and its member-owned resources.
    virtual ~ParticleObjectBase() { }
    // Does nothing.
    virtual void mode(s32) { }
};
#endif
