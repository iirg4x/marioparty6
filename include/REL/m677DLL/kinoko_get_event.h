// Runs the Kinoko get event and its result.

#ifndef REL_SEER_TERROR_KINOKO_GET_EVENT_H
#define REL_SEER_TERROR_KINOKO_GET_EVENT_H

#include "REL/m677DLL/event_base.h"
#include "REL/framework/model.h"

#include <memory>

// Moves the event's mushroom model and handles the character's reaction.
class KinokoGetEvent : public EventBase {
public:
    KinokoGetEvent();
    virtual ~KinokoGetEvent();
    virtual void begin();
    virtual int step();

    typedef void (KinokoGetEvent::*State)();
    State state;
    // Selects the next state handler.
    void setState(State next) { state = next; }
    std::auto_ptr<OwnedModel> model;
    Vec position;
    Vec rotation;
    // Distance the model descends each frame.
    float heightStep;
    // Frames since the last particle emission.
    int elapsedFrames;
    int countdown;

    // Sets the remaining frames in the countdown.
    void setCountdown(int frames) { countdown = frames; }

    void alignModelAndStartCharacterMotion();
    void shiftCharacterMotionAfterCountdown();
    void lowerModelToCharacter();
    void waitForCharacterMotionEnd();
};

#endif
