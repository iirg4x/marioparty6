#ifndef REL_M672DLL_SPINNER_H
#define REL_M672DLL_SPINNER_H

#include <memory>
#include "REL/m672DLL/particle_groups.h"
#include "REL/framework/event_hm.h"
#include "REL/framework/game_object.h"
#include "REL/framework/model.h"
#include "REL/m672DLL/collision_object.h"

struct Spinner : OwnedModel, GameObject {
    CollisionObject motion;
    float progress;
    float velocity;
    unsigned char mode;
    float angle;
    float angleDelta;
    float amplitude;
    unsigned char active;
    unsigned int remainingFrames;
    unsigned int duration;
    std::auto_ptr<CollidingParticles> output;
    Spinner(s32 setupMode, WorldPosition position, ANIMDATA *, s32, s32, ANIMDATA *, s32,
            ANIMDATA *texture, s32 count, s32 layer, s32 itemIndex);
    virtual void update();
    virtual ~Spinner();
    void fn_1_8814(EventMessage *);
    void fn_1_8820(EventMessage *);
    Vec fn_1_885C(Vec *source);
    WorldPosition fn_1_88D0(s32 index);
    EventHandlerManager firstCallback;
    EventHandlerManager secondCallback;
};

#endif
