// Runs the Golden Koopa event and its result.

#ifndef REL_SEER_TERROR_GOLDEN_KOOPA_EVENT_H
#define REL_SEER_TERROR_GOLDEN_KOOPA_EVENT_H

#include "REL/framework/model.h"
#include "REL/m677DLL/event_base.h"
#include <memory>

// Places the Golden Koopa model over the character and animates its fall.
class GoldenKoopaEvent : public EventBase {
  public:
    GoldenKoopaEvent();
    virtual ~GoldenKoopaEvent();
    virtual void begin();
    virtual int step();

  private:
    typedef void (GoldenKoopaEvent::*State)();
    State state;
    // Selects the next state handler.
    void setState(State next)
    {
        state = next;
    }
    std::auto_ptr<OwnedModel> model;
    int countdown;

    // Sets the remaining frames in the countdown.
    void setCountdown(int frames)
    {
        countdown = frames;
    }

    void placeModelAndStartMotion();
    void releaseModelAttributes();
    void reactAtBodyHeight();
    void playLandingEffects();
    void waitForFinalCountdown();
};

#endif
