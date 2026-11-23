// Runs the Pitfall event and its result.

#ifndef REL_SEER_TERROR_PITFALL_EVENT_H
#define REL_SEER_TERROR_PITFALL_EVENT_H

#include "REL/m677DLL/event_base.h"

// Opens the chosen pit, lowers the player floor height, and completes when the character
// reaches that height.
class PitfallEvent : public EventBase {
  public:
    PitfallEvent();
    virtual ~PitfallEvent();
    virtual void begin();
    virtual int step();

  private:
    typedef void (PitfallEvent::*State)();
    State state;
    // Selects the next state handler.
    void setState(State next)
    {
        state = next;
    }
    int countdown;

    // Sets the remaining frames in the countdown.
    void setCountdown(int frames)
    {
        countdown = frames;
    }

    void startReaction();
    void waitForReaction();
    void lowerFloor();
    void waitForFall();
};

#endif
