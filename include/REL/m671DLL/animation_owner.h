#ifndef REL_M671_ANIMATION_OWNER_H
#define REL_M671_ANIMATION_OWNER_H
class ParticleOwner;
struct AttributeOwner;
#include <memory>
#include "REL/framework/model.h"
#include "REL/framework/game_object.h"
#include "REL/m671DLL/collision.h"
#include "REL/m671DLL/motion_position.h"
#include "REL/framework/event_hm.h"
struct AnimationOwner : GameObject, OwnedModel {
    AnimationOwner(int model, int mode);
    virtual ~AnimationOwner();
    virtual void update();
    CollisionObject position;
    MotionPosition previousPosition;
    Mtx matrix;
    Quaternion rotation;
    unsigned char enabled;
    unsigned char isActive() { return enabled; }
    int argument;
    float descentSpeed;
    float speedStep;
    float modelScale;
    float elapsedTime;
    unsigned char transitionComplete;
    unsigned char complete;
    unsigned char preserveSpeed;
    unsigned char entered;
    std::auto_ptr<ParticleOwner> particle;
    std::auto_ptr<AttributeOwner> attribute;
    EventHandlerManager callbacks;
    void (AnimationOwner::*callback)();
    void fn_1_62E8(EventMessage *);
    void fn_1_634C(EventMessage *);
    Vec getPosition() { return position.value; }
    void fn_1_6008();
    void fn_1_600C();
    void fn_1_6158();
    void fn_1_6350();
    void fn_1_65BC();
    void fn_1_65C0();
    typedef void (AnimationOwner::*Callback)();
    void setCallback(void (AnimationOwner::*callback)()) { this->callback = callback; }
};

#endif
