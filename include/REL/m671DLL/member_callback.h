#ifndef REL_M671_MEMBER_CALLBACK_H
#define REL_M671_MEMBER_CALLBACK_H
#include <functional>
#include "REL/framework/event_hm.h"

template<class Owner>
struct StoredCallback : std::binder1st<std::mem_fun1_t<void, Owner, EventMessage *> > {
    typedef std::binder1st<std::mem_fun1_t<void, Owner, EventMessage *> > Payload;
    StoredCallback(const Payload &payload) : Payload(payload) {}
    void invoke(EventMessage *message) { this->op(this->value, message); }
};

template<class Owner>
struct MemberCallback : EventCallback {
    typedef std::binder1st<std::mem_fun1_t<void, Owner, EventMessage *> > Payload;
    StoredCallback<Owner> callback;
    MemberCallback(const Payload &payload) : EventCallback(), callback(payload) {}
    virtual ~MemberCallback() {}
    virtual void receive(EventMessage *message) { callback.invoke(message); }
};
#endif
