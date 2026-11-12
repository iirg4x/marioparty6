// The minigame's event interface (event.cpp): constructing the manager starts it.
#ifndef REL_FRAMEWORK_EVENT_MANAGER_H
#define REL_FRAMEWORK_EVENT_MANAGER_H

#include "REL/framework/singleton.h"

/* Minigame-facing lifetime owner of the shared event map, queue, and clock. */
struct EventManager : MinigameSingleton<EventManager> {
    EventManager();
    ~EventManager();
};

#endif
