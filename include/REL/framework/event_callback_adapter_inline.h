#ifndef FRAMEWORK_EVENT_CALLBACK_ADAPTER_INLINE_H
#define FRAMEWORK_EVENT_CALLBACK_ADAPTER_INLINE_H
#include "REL/framework/event_hm.h"

/* Adapts a stored callable to the channel-event interface. */
template<class Callable> struct EventCallbackAdapter : EventCallback {
    Callable callback;
    // Stores the callable used to receive channel events.
    EventCallbackAdapter(const Callable &value) : EventCallback(), callback(value) {}
    // Passes the received event to the stored callback.
    virtual void receive(const EventPacket *event) { callback(*event); }
};

#endif
