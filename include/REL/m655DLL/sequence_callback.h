/* Member callbacks registered by the sequence objects on their event channels. */
#ifndef INCLUDE_REL_M655DLL_SEQUENCE_CALLBACK_H
#define INCLUDE_REL_M655DLL_SEQUENCE_CALLBACK_H

extern "C" {
#include <stdio.h>
#include <stdlib.h>
}
#include <functional>
#include "REL/framework/event_hm.h"

inline EventCallback::~EventCallback() {}
struct SequenceEventObject;
typedef void (SequenceEventObject::*SequenceEventHandler)(EventMessage *);

template<class Operation> struct SequenceBinding : std::unary_function<typename Operation::second_argument_type, typename Operation::result_type> {
    Operation operation;
    typename Operation::first_argument_type receiver;
    SequenceBinding(const Operation &op, const typename Operation::first_argument_type &object)
        : operation(op), receiver(object) {}
    void operator()(EventMessage *event) const { operation(receiver, event); }
};
template<class Operation, class T> inline SequenceBinding<Operation> sequenceBind(const Operation &op, const T &receiver)
{
    typedef typename Operation::first_argument_type Receiver;
    return SequenceBinding<Operation>(op, Receiver(receiver));
}
typedef SequenceBinding<std::mem_fun1_t<void, SequenceEventObject, EventMessage *> > SequenceCallback;

template<class Payload> struct EventCallbackValue : EventCallback {
    Payload consumer;
    virtual ~EventCallbackValue() {}
    virtual void receive(EventMessage *message) {
        consumer(message);
    }
    EventCallbackValue(const Payload& payload)
        : EventCallback(), consumer(payload) {}
};

template<class Payload> void fn_1_52B8(EventHandlerManager *owner, const Payload& payload, int key)
{
    owner->addHandler(new EventCallbackValue<Payload>(payload), key);
}
#endif
