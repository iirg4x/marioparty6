// Tracks the active event and its lifecycle.

#ifndef REL_SEER_TERROR_EVENT_BASE_H
#define REL_SEER_TERROR_EVENT_BASE_H

#include "REL/framework/singleton.h"
#include "REL/m677DLL/countdown.h"

// Starts and advances a board event callback.
class EventBase : public MinigameSingleton<EventBase> {
public:
    EventBase();
    virtual ~EventBase();
    virtual void begin();
    virtual int step() = 0;
    virtual void fn_1_17104();
    virtual int advanceStates();
};

int fn_1_16EFC();
void fn_1_16FC4();
int fn_1_17060();

#endif
