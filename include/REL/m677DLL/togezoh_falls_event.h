// Animates the falling Togezoh and the player's reaction.

#ifndef REL_SEER_TERROR_TOGEZOH_FALLS_EVENT_H
#define REL_SEER_TERROR_TOGEZOH_FALLS_EVENT_H

#include "REL/framework/model.h"
#include "REL/m677DLL/event_base.h"
#include <memory>

class TogezohFallsModel;

// Spawns and advances the falling models during Togezoh.
class TogezohFallsEvent : public EventBase {
public:
    TogezohFallsEvent();
    virtual ~TogezohFallsEvent();
    virtual void begin();
    virtual int step();
private:
    typedef void (TogezohFallsEvent::*State)();
    State state;
    // Selects the next state handler.
    void setState(State next) { state = next; }
    std::auto_ptr<OwnedModel> model;
    std::auto_ptr<TogezohFallsModel> children[6];
    int frame;
    void startMotionAndPrepareChildren();
    void updateChildren();
};

#endif
