// Runs the Meteoric fall event and its result.

#ifndef REL_SEER_TERROR_METEORIC_FALL_EVENT_H
#define REL_SEER_TERROR_METEORIC_FALL_EVENT_H

#include <memory>
#include "REL/m677DLL/event_base.h"
#include "REL/framework/model.h"

// Runs the meteor model and player reaction, then triggers the host reaction and particles.
class MeteoricFallEvent : public EventBase {
public:
    MeteoricFallEvent();
    virtual ~MeteoricFallEvent();
    virtual void begin();
    virtual int step();

private:
    typedef void (MeteoricFallEvent::*State)();
    State state;
    // Selects the next state handler.
    void setState(State next) { state = next; }
    std::auto_ptr<OwnedModel> model;
    int countdown;

    // Sets the remaining frames in the countdown.
    void setCountdown(int frames) { countdown = frames; }
    void revealModelAndPlaySound();
    void shiftCharacterAfterCountdown();
    void startFinalCharacterMotion();
    void waitForFinalCountdown();
};

#endif
