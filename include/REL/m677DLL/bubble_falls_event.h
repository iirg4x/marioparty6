// Runs the Bubble falls event and its result.

#ifndef REL_SEER_TERROR_BUBBLE_FALLS_EVENT_H
#define REL_SEER_TERROR_BUBBLE_FALLS_EVENT_H

#include "REL/m677DLL/event_base.h"
#include "REL/framework/model.h"
#include <memory>

// Runs the callback that spawns playback models around the character.
class BubbleFallsEvent : public EventBase {
public:
    BubbleFallsEvent();
    virtual ~BubbleFallsEvent();
    virtual void begin();
    virtual int step();
private:
    typedef void (BubbleFallsEvent::*State)();
    State state;
    // Selects the next state handler.
    void setState(State next) { state = next; }
    std::auto_ptr<OwnedModel> model;
    int countdown;

    // Sets the remaining frames in the countdown.
    void setCountdown(int frames) { countdown = frames; }
    void spawnFallingModels();
    void shiftAfterCharacterMotion();
    void startCharacterOverlay();
    void waitForModelsAndCountdown();
};

#endif
