#ifndef REL_M672DLL_AC4C_OWNER_H
#define REL_M672DLL_AC4C_OWNER_H

#include "REL/framework/game_object.h"
#include "REL/framework/model.h"

#include "REL/framework/event_hm.h"
typedef EventHandlerManager ChildRecord;

struct AC4COwner;

typedef void (AC4COwner::*AC4CCallback)(EventMessage *);

#include "REL/m672DLL/member_callback.h"
typedef std::mem_fun1_t<void, AC4COwner, EventMessage *> AC4CMemberFunction;
typedef CallbackBinding<AC4COwner> AC4CPayload;

typedef void (AC4COwner::*AC4CUpdate)();
struct AC4COwner : GameObject, OwnedModel {
    int state;
    int nextState;
    float time;
    unsigned char active;
    ChildRecord child;
    AC4CUpdate updateCallback;
    void setUpdate(AC4CUpdate callback) { updateCallback = callback; }
    void fn_1_AF34(EventMessage *);
    void fn_1_AF78(EventMessage *);
    void fn_1_B050();
    void fn_1_B054();
    void fn_1_B140();
    AC4COwner(int firstMode, const float *position);
    virtual ~AC4COwner();
    virtual void update();
};

#endif
