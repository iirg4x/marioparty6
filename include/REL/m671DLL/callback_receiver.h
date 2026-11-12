/* Shared receiver state used by event callbacks and height updates. */
#ifndef REL_M671_CALLBACK_RECEIVER_H
#define REL_M671_CALLBACK_RECEIVER_H

#include <vector>
#include <memory>
#include "REL/m671DLL/motion_position.h"
#include "REL/framework/model.h"
#include "REL/framework/game_object.h"
#include "REL/framework/singleton.h"
#include "REL/framework/event_hm.h"

#include "REL/framework/model_motion.h"

struct AnimationOwner;

struct CallbackReceiver : GameObject, ModelMotion, MinigameSingleton<CallbackReceiver> {
    CallbackReceiver(unsigned int type, Vec *position, Vec *target);
    virtual ~CallbackReceiver();
    virtual void update();
    MotionPosition position;
    Vec target;
    Vec velocity;
    s16 light;
    AnimationOwner *objects[3];
    u8 entryFlags[4];
    s32 index;
    std::auto_ptr<OwnedModel> lowerModel;
    std::auto_ptr<OwnedModel> upperModel;
    float timer;
    s32 frameCount;
    u8 active;
    s32 motion;
    u8 state;
    EventHandlerManager callbacks;
    void (CallbackReceiver::*currentState)();
    void (CallbackReceiver::*previousState)();
    void fn_1_94C0();
    void fn_1_A614();
    void fn_1_AE78();
    void fn_1_B09C();
    void fn_1_94CC();
    typedef void (CallbackReceiver::*Callback)();
    void fn_1_9784();
    void fn_1_9C7C();
    void fn_1_A794();
    void fn_1_A230(EventMessage *);
    void fn_1_A28C(EventMessage *);
    void fn_1_A3A4(EventMessage *);
    void fn_1_A2E0(EventMessage *);
    void fn_1_A530(EventMessage *);
    void setMotion(u32 value, u32 flags);
    void setState(void (CallbackReceiver::*next)()) { currentState = next; }
    Vec getMotionHeight() { return velocity; }
};

#endif
