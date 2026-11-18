#ifndef FRAMEWORK_EVENT_CALLBACK_REGISTRATION_H
#define FRAMEWORK_EVENT_CALLBACK_REGISTRATION_H
#include "REL/framework/event_callback_adapter_inline.h"

/* Allocates the callback adapter owned by the event-handler manager. */
template<class Callable>
void registerEventCallback(EventHandlerManager *owner, const Callable &payload, int key)
{
    owner->addHandler(new EventCallbackAdapter<Callable>(payload), key);
}

#endif
