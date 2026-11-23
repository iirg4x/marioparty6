// Runs the Taru falls event and its result.

#ifndef REL_SEER_TERROR_TARU_FALLS_EVENT_H
#define REL_SEER_TERROR_TARU_FALLS_EVENT_H

#include "REL/framework/model.h"
#include "REL/m677DLL/event_base.h"
#include <memory>
#include "REL/framework/frame_interpolator.h"
#include <vector>

// Reveals the model and advances its character motions.
class TaruFallsEvent : public EventBase {
public:
    TaruFallsEvent();
    virtual ~TaruFallsEvent();
    virtual void begin();
    virtual int step();

private:
    typedef void (TaruFallsEvent::*State)();
    State state;
    // Selects the next state handler.
    void setState(State next) { state = next; }
    std::auto_ptr<OwnedModel> model;
    FrameInterpolator<float> characterScale;
    int countdown;

    // Sets the remaining frames in the countdown.
    void setCountdown(int frames) { countdown = frames; }
    std::vector<int> soundFrames;

    void startMotionAndRevealTimer();
    void revealModelAfterDelay();
    void startCharacterScaleChange();
    void updateCharacterScale();
    void playTimedMotionCues();
};

#endif
