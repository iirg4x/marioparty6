#ifndef REL_M672DLL_HANGING_CAGE_H
#define REL_M672DLL_HANGING_CAGE_H
#include "REL/framework/game_object.h"
#include "REL/m672DLL/position_bf64.h"
#include <memory>
class OwnedModel;

struct RisingParticles;
struct HangingCage : GameObject {
    WorldPosition position;
    std::auto_ptr<OwnedModel> motion;
    std::auto_ptr<OwnedModel> secondModel;
    std::auto_ptr<RisingParticles> child;
    f32 angle;
    f32 rotation;
    u32 frame;
    f32 fade;
    HangingCage(s32 type, const WorldPosition *position);
    virtual ~HangingCage();
    virtual void update();
};

#endif
