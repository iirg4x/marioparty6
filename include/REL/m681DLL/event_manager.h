#ifndef M681_EVENT_MANAGER_H
#define M681_EVENT_MANAGER_H
#include "REL/framework/singleton.h"

// The event manager publishes this empty base at construction and clears it on destruction.
struct EventManager : MinigameSingleton<EventManager> {
    EventManager();
    ~EventManager();
};
#endif
