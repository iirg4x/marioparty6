#ifndef M639_EVENT_MANAGER_H
#define M639_EVENT_MANAGER_H
#include "REL/framework/singleton.h"
// Declaration shared with the same game's framework/event.cpp, lines 98-101.
struct EventManager : MinigameSingleton<EventManager> {
    EventManager();
    ~EventManager();
};
#endif
