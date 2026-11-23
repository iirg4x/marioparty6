// Runs the Stage rotation event and its result.

#ifndef REL_SEER_TERROR_STAGE_ROTATION_EVENT_H
#define REL_SEER_TERROR_STAGE_ROTATION_EVENT_H

#include "REL/m677DLL/event_base.h"

// Runs the cover-motion sequence and updates its sound pitch.
class StageRotationEvent : public EventBase {
public:
    StageRotationEvent();
    virtual ~StageRotationEvent();
    virtual void begin();
    virtual int step();
private:
    typedef void (StageRotationEvent::*State)();
    State state;
    // Selects the next state handler.
    void setState(State next) { state = next; }
    // Joint angle seen on the previous frame.
    float previousAngle;
    // Handle for the sound started with the cover motion.
    int sound;
    void startCharacterMotion();
    void startJointAfterCharacterMotion();
    void trackJointAngle();
};

#endif
