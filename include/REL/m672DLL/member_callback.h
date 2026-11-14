#ifndef REL_M672DLL_MEMBER_CALLBACK_H
#define REL_M672DLL_MEMBER_CALLBACK_H
#include "REL/framework/event_packet.h"
#include <functional>
#include "REL/framework/event_hm.h"

template<class Receiver> struct CallbackBinding {
    std::mem_fun1_t<void, Receiver, EventMessage *> function;
    Receiver *receiver;
    // Store the member functor and its receiver.
    CallbackBinding(const std::mem_fun1_t<void, Receiver, EventMessage *> &fn, Receiver *object)
        : function(fn), receiver(object) {}
};

// Bind a member callback to its receiver.
template<class Receiver>
inline CallbackBinding<Receiver> bindMember(
    const std::mem_fun1_t<void, Receiver, EventMessage *> &function, Receiver *receiver)
{
    return CallbackBinding<Receiver>(function, receiver);
}

// Registers a receiver's member function on an event key.
template<class Receiver>
void registerMemberHandler(EventHandlerManager *owner,
                           const CallbackBinding<Receiver> &payload, int key);

#endif
