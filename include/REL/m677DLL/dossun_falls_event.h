// Runs the Dossun falls event and its result.

#ifndef REL_SEER_TERROR_DOSSUN_FALLS_EVENT_H
#define REL_SEER_TERROR_DOSSUN_FALLS_EVENT_H

#include "REL/m677DLL/event_base.h"
#include "REL/framework/model.h"

#include <memory>

// Animates a single Dossun descending from above the player.
class DossunFallsEvent : public EventBase {
public:
    typedef void (DossunFallsEvent::*State)();
    DossunFallsEvent();
    virtual ~DossunFallsEvent();
    virtual int step();
    virtual void begin();

    State state;
    // Selects the next state handler.
    void setState(State next) { state = next; }
    std::auto_ptr<OwnedModel> model;
    Vec position;
    float elapsedTime;
    int countdown;

    // Sets the remaining frames in the countdown.
    void setCountdown(int frames) { countdown = frames; }

    void beginDescent();
    void updateDescent();
    void waitForLandingCountdown();
};

#endif
