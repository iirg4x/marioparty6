#ifndef REL_M672DLL_EVENT_LIFETIME_H
#define REL_M672DLL_EVENT_LIFETIME_H
/* Consumed lifetime interface from the shared event.cpp. */
#include "REL/framework/minigame_singleton.h"
struct EventManager : MinigameSingleton<EventManager> {
    EventManager();
    ~EventManager();
};

#endif
