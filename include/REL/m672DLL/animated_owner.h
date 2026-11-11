#ifndef REL_M672DLL_ANIMATED_OWNER_H
#define REL_M672DLL_ANIMATED_OWNER_H

#include "REL/framework/event_hm.h"
typedef EventHandlerManager StateEventTable;

#ifdef __cplusplus
#include "REL/m672DLL/resources.h"
#include "REL/m672DLL/model_motion_bf64.h"
#include "REL/framework/game_object.h"

struct AnimatedOwner;
typedef void (AnimatedOwner::*AnimatedOwnerHandler)(EventMessage *);
#include "REL/m672DLL/member_callback.h"
typedef std::mem_fun1_t<void, AnimatedOwner, EventMessage *> StateMemberMethod;
typedef CallbackBinding<AnimatedOwner> StateCallbackBinding;

#include "REL/framework/singleton.h"
typedef MinigameSingleton<AnimatedOwner> AnimatedOwnerSingleton;
struct VecValue { Vec value; };
typedef void (AnimatedOwner::*AnimatedOwnerUpdate)();
struct AnimatedOwner : GameObject, ModelMotion, AnimatedOwnerSingleton {
    VecValue position;
    u8 unknown_30[0xC]; /* No code names this field. */
    int state;
    u32 frame;
    u8 active;
    u8 unknown45;
    short lightId;
    StateEventTable states;
    StateEventTable events;
    AnimatedOwnerUpdate updateCallback;
    void setUpdate(AnimatedOwnerUpdate callback) { updateCallback = callback; }
    AnimatedOwner(int, VecValue *, float);
    virtual ~AnimatedOwner();
    virtual void update();
    void fn_1_9AFC(unsigned int, unsigned char);
    void fn_1_9F7C(EventMessage *);
    void fn_1_9E08(EventMessage *);
    void fn_1_9B90();
    void fn_1_9C94();
    void fn_1_9DF8();
    void fn_1_9DFC(EventMessage *);
    void fn_1_9E00(EventMessage *);
    void fn_1_9E04(EventMessage *);
    void fn_1_9F38(EventMessage *);
};
#endif

#endif
