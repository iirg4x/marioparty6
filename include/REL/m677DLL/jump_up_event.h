// Runs the Jump up! event and its result.

#ifndef REL_SEER_TERROR_JUMP_UP_EVENT_H
#define REL_SEER_TERROR_JUMP_UP_EVENT_H

#include "REL/framework/model.h"
#include "REL/m677DLL/event_base.h"
#include <memory>

// Plays the character reaction and moves the spring model.
class JumpUpEvent : public EventBase {
public:
    JumpUpEvent();
    virtual ~JumpUpEvent();
    virtual void begin();
    virtual int step();

private:
    typedef void (JumpUpEvent::*State)();
    State state;
    // Selects the next state handler.
    void setState(State next) { state = next; }
    int countdown;

    // Sets the remaining frames in the countdown.
    void setCountdown(int frames) { countdown = frames; }
    std::auto_ptr<OwnedModel> model;
    int sound;

    void startSelectedJointMotion();
    void reactAtHeight();
    void revealModelAfterCountdown();
    void waitForModelMotion();
};

#endif
