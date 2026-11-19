#ifndef REL_M605DLL_PARTICLE_EFFECT_OBJECT_H
#define REL_M605DLL_PARTICLE_EFFECT_OBJECT_H

#include "REL/framework/game_object.h"
#include "REL/framework/particle_class.h"

/* Creates orange particles at the stage model's hooks. */
class OrangeParticles : public GameObject, public Particle {
public:
    OrangeParticles(ANIMDATA *animation);
    virtual ~OrangeParticles();
    virtual void update();
    void fn_1_B850(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);
};

#endif
