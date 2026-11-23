// Runs the Wan-wan chase and moves the player along its route.

#ifndef REL_SEER_TERROR_WAN_WAN_CHASE_EVENT_H
#define REL_SEER_TERROR_WAN_WAN_CHASE_EVENT_H

#include "REL/m677DLL/event_base.h"
#include "REL/framework/model_motion.h"
#include "REL/m677DLL/path_follower.h"
#include "REL/m677DLL/player_work.h"

#include <memory>
#include <vector>

// Controls the approach and walks the character along the selected route.
struct WanWanChaseEvent : EventBase {
    typedef void (WanWanChaseEvent::*State)();

    State state;
    // Selects the next state handler.
    void setState(State next) { state = next; }

    std::auto_ptr<ModelMotion> model;

    std::vector<Vec> route;

    std::vector<Vec> unk20; // No code names this field.

    PathFollower follower;

    int stage;

    float motionTime;

    int stepSoundCountdown;

    int rumbleCountdown;

    Range<float> heading;

    float targetHeading;

    int countdown;

    // Sets the remaining frames in the countdown.
    void setCountdown(int frames) { countdown = frames; }

    WanWanChaseEvent();

    ~WanWanChaseEvent();

    void begin();

    int step();

    void startCharacterMotionBeforeChase();

    void waitBeforeRoute();

    void walkRoute();

    void finishRoute();
};

#endif
