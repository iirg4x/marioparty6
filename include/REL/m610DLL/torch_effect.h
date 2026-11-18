#ifndef REL_M610DLL_TORCH_EFFECT_H
#define REL_M610DLL_TORCH_EFFECT_H
#include "REL/framework/game_object.h"
#include "REL/framework/particle.h"
/* Own the torch particle model and its hook state. */
struct TorchEffect : GameObject, Particle
{
    TorchEffect(ANIMDATA *);
    virtual ~TorchEffect();
    virtual void update();
    void fn_1_F850(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);
};
#endif
