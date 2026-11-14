#ifndef REL_M672DLL_MEMBER_HANDLER_H
#define REL_M672DLL_MEMBER_HANDLER_H
#include "REL/m672DLL/member_callback.h"

template<class Receiver>
struct MemberCallbackRecord : EventCallback {
    std::mem_fun1_t<void, Receiver, EventMessage *> callback;
    Receiver *receiver;
    // Store the bound callback and receiver in the event record.
    explicit MemberCallbackRecord(const CallbackBinding<Receiver> &payload)
        : EventCallback(), callback(payload.function), receiver(payload.receiver) {}
    // Destroy the bound event callback record.
    virtual ~MemberCallbackRecord() {}
    // Invoke the bound member callback for this event.
    virtual void receive(EventMessage *message)
    {
        Receiver *receiverValue = receiver;
        callback(receiverValue, message);
    }
};
// Registers a receiver's member function on an event key.
template<class Receiver>
// Allocate and register the bound member callback.
void registerMemberHandler(EventHandlerManager *owner,
                           const CallbackBinding<Receiver> &payload, int key)
{
    owner->addHandler(new MemberCallbackRecord<Receiver>(payload), key);
}

#endif
