#ifndef REL_M672DLL_HANGING_CAGE_H
#define REL_M672DLL_HANGING_CAGE_H
#include "REL/framework/game_object.h"
#include "REL/m672DLL/collision_object.h"
#include <memory>
struct OwnedModel;

struct RisingParticles;
struct HangingCage : GameObject {
    EffectVector position;
    std::auto_ptr<OwnedModel> motion;
    std::auto_ptr<OwnedModel> secondModel;
    std::auto_ptr<RisingParticles> child;
    f32 angle;
    f32 rotation;
    u32 frame;
    f32 fade;
    HangingCage(s32 type, const EffectVector *position);
    virtual ~HangingCage();
    virtual void update();
};

#endif
