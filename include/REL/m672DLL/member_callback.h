#ifndef REL_M672DLL_MEMBER_CALLBACK_H
#define REL_M672DLL_MEMBER_CALLBACK_H
#include "REL/framework/event_packet.h"
#include <functional>
struct EventHandlerManager;

template<class Receiver> struct CallbackBinding {
    std::mem_fun1_t<void, Receiver, EventMessage *> function;
    Receiver *receiver;
    CallbackBinding(const std::mem_fun1_t<void, Receiver, EventMessage *> &fn, Receiver *object)
        : function(fn), receiver(object) {}
};
template<class Receiver>
struct MemberCallbackRecord : EventCallback {
    std::mem_fun1_t<void, Receiver, EventMessage *> callback;
    Receiver *receiver;
    explicit MemberCallbackRecord(const CallbackBinding<Receiver> &payload)
        : EventCallback(), callback(payload.function), receiver(payload.receiver) {}
    virtual ~MemberCallbackRecord() {}
    virtual void receive(EventMessage *message)
    {
        Receiver *receiverValue = receiver;
        callback(receiverValue, message);
    }
};
#endif
