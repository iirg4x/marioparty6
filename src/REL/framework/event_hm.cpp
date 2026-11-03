/* Dispatches channel events to the minigame callbacks registered for each event type. */
#include <map>

#include "REL/framework/event_hm.h"

static const char rcsid[] = "$Id: event_hm.cpp,v 1.21 2004/09/09 12:24:38 saf Exp $";

struct HandlerMap : std::multimap<int, EventCallback *> {
    HandlerMap() {}
    ~HandlerMap() {}
};

inline HandlerMapOwner::~HandlerMapOwner() { delete map; }

/* Create the callback map and register its channel after the event manager is active. */
EventHandlerManager::EventHandlerManager(int channel) : handlers(new HandlerMap)
{
    registerChannel(channel);
}

/* On destruction, delete callbacks in event-type order before the map releases its nodes. */
EventHandlerManager::~EventHandlerManager()
{
    HandlerMap& handlerMap = *handlers;
    HandlerMap::iterator currentHandler = handlerMap.begin();
    HandlerMap::iterator endHandler = handlerMap.end();
    while (currentHandler != endHandler) {
        delete currentHandler->second;
        ++currentHandler;
    }
}

/* When a caller registers a callback, retain it under its type, including duplicate types. */
void EventHandlerManager::addHandler(EventCallback *handler, int eventType)
{
    HandlerMap::value_type entry(eventType, handler);
    HandlerMap& handlerMap = *handlers;
    handlerMap.insert(entry);
}

/* Channel dispatch calls this to forward the same message to every matching registration. */
void EventHandlerManager::receive(const EventPacket *message)
{
    HandlerMap& handlerMap = *handlers;
    HandlerMap::iterator currentHandler = handlerMap.lower_bound(message->type);
    HandlerMap::iterator pastTypeHandlers = handlerMap.upper_bound(message->type);
    while (currentHandler != pastTypeHandlers) {
        currentHandler->second->receive(message);
        ++currentHandler;
    }
}
