#ifndef REL_M634DLL_PLAYER_CALLBACK_H
#define REL_M634DLL_PLAYER_CALLBACK_H
#include "REL/framework/event_hm.h"
#include "REL/framework/event_callback_inline.h"
template <typename Payload> struct Callback : EventCallback {
    Payload callback;
    // Releases the object and its member-owned resources.
    virtual ~Callback() { }
    virtual void receive(const EventPacket *value);
    // Stores the callback payload.
    Callback(const Payload &payload)
        : EventCallback()
        , callback(payload)
    {
    }
};

// Invokes the stored event callback.
template <typename Payload> void Callback<Payload>::receive(const EventPacket *value)
{
    callback(*value);
}

// Registers a player event callback for the requested key.
template <typename Payload>
void fn_1_3ED0(EventHandlerManager *owner, const Payload &payload, int key)
{
    owner->addHandler(new Callback<Payload>(payload), key);
}

#endif
