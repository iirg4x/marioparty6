// Tracks the active event and its lifecycle.

#include "REL/m677DLL/event_base.h"
#include "REL/m677DLL/rope_release_sequence.h"
#include "REL/m677DLL/seer_terror_camera.h"

// Construction calls the empty EventBase::begin. SeerTerrorScene calls callback->begin()
// after construction to start the derived event.
EventBase::EventBase()
{
    begin();
}

// The singleton base unregisters the current event during destruction.
EventBase::~EventBase()
{
}

// Advances rope release first, or dispatches the current event callback.
int fn_1_16EFC()
{
    RopeReleaseSequence *owner = RopeReleaseSequence::self;
    if (owner) {
        return updateRopeReleaseSequence();
    }

    EventBase *callback = EventBase::self;
    if (!callback) {
        return 1;
    }
    return EventBase::getChecked().step();
}

// Starts the active callback's closing camera transition.
void fn_1_16FC4()
{
    EventBase *callback = EventBase::self;
    if (callback) {
        EventBase::getChecked().fn_1_17104();
    }
}

// Advances the active callback's closing stage and reports completion.
int fn_1_17060()
{
    EventBase *callback = EventBase::self;
    if (!callback) {
        return 1;
    }
    return EventBase::getChecked().advanceStates();
}

// Transitions to the third camera pose.
void EventBase::fn_1_17104()
{
    SeerTerrorCamera::getChecked().transitionToPreset(2, 3.0f);
}

// Reports immediate completion for the default closing stage.
int EventBase::advanceStates()
{
    return 1;
}
