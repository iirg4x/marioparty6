/* Defines channel-event callbacks and the owning map used to dispatch them by type. */
#ifndef FRAMEWORK_EVENT_HM_H
#define FRAMEWORK_EVENT_HM_H
#include "REL/framework/event_packet.h"

/* Callback registrations ordered by event type; event_hm.cpp alone sees the map. */
struct HandlerMap;

/* Deletes the map and its nodes when the owning manager is destroyed. */
struct HandlerMapOwner {
    HandlerMap *map; // Owned map; the manager deletes its callbacks separately.
    HandlerMapOwner(HandlerMap *handlerMap) : map(handlerMap) {}
    ~HandlerMapOwner();
    HandlerMap *get() const { return map; }
    HandlerMap *operator->() const { return get(); }
    HandlerMap& operator*() const { return *map; }
};

/* Owns callbacks grouped by event type and receives events from one channel. */
struct EventHandlerManager : EventListener {
    HandlerMapOwner handlers; // Owned callback registrations, ordered by event type.
    EventHandlerManager(int channel);
    virtual ~EventHandlerManager();
    void addHandler(EventCallback *, int);
    virtual void receive(const EventPacket *);
};

#endif
