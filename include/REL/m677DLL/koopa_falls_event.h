// Runs the Koopa falls event and its result.

#ifndef REL_SEER_TERROR_KOOPA_FALLS_EVENT_H
#define REL_SEER_TERROR_KOOPA_FALLS_EVENT_H

#include "REL/m677DLL/event_base.h"

#include <memory>

class KoopaFallsDescent;
struct ScreenCaptureProjection;

// Controls the model's vertical motion during the callback.
class KoopaFallsEvent : public EventBase {
public:
    KoopaFallsEvent();
    virtual ~KoopaFallsEvent();
    virtual void begin();
    virtual int step();
    virtual void fn_1_17104();
    virtual int advanceStates();

    typedef void (KoopaFallsEvent::*State)();
    State state;
    // Selects the next state handler.
    void setState(State next) { state = next; }
    int countdown;

    // Sets the remaining frames in the countdown.
    void setCountdown(int frames) { countdown = frames; }
    std::auto_ptr<KoopaFallsDescent> transition;
    std::auto_ptr<ScreenCaptureProjection> screen;

    void startCameraTransition();
    void createScreenAfterCamera();
    void startHostMotionAfterDelay();
    void updateModelDescent();
    void playFollowupCue();
    void startScreenTransition();
    void waitForScreenCountdown();
};

#endif
