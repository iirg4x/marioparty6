#ifndef REL_M634DLL_PLAYER_CALLBACK_H
#define REL_M634DLL_PLAYER_CALLBACK_H
#include "REL/framework/event_hm.h"
#include "REL/framework/event_callback_inline.h"
template <typename Payload>
struct Callback : EventCallback {
    Payload callback;
    virtual ~Callback() {}
    virtual void receive(const EventPacket *value);
    Callback(const Payload &payload)
        : EventCallback(), callback(payload) {}
};

template <typename Payload>
void Callback<Payload>::receive(const EventPacket *value) { callback(*value); }

template <typename Payload>
void fn_1_3ED0(EventHandlerManager *owner, const Payload &payload, int key) { owner->addHandler(new Callback<Payload>(payload), key); }

#endif
