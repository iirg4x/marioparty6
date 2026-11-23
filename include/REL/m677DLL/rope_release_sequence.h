// Releases the unselected ropes before the selected rope.

#ifndef REL_SEER_TERROR_ROPE_RELEASE_SEQUENCE_H
#define REL_SEER_TERROR_ROPE_RELEASE_SEQUENCE_H

#include "REL/framework/singleton.h"
#include "REL/m677DLL/countdown.h"

// Releases the selected rope after the character reacts.
class RopeReleaseSequence : public MinigameSingleton<RopeReleaseSequence> {
public:
    RopeReleaseSequence();
    ~RopeReleaseSequence();
    void fn_1_7518();
    bool advanceStates();

private:
    typedef void (RopeReleaseSequence::*State)();
    State state;
    // Selects the next state handler.
    void setState(State next) { state = next; }
    int countdown;

    // Sets the remaining frames in the countdown.
    void setCountdown(int frames) { countdown = frames; }

    // Advances a running countdown and reports when it expires.
    bool updateCountdown()
    {
        return ::updateCountdown(countdown);
    }

    void startCharacterAction();
    void waitForCharacterIdle();
    void releaseUnselectedRopes();
    void releaseSelectedRope();
    void destroyRopesAfterCountdown();
};

int updateRopeReleaseSequence();

#endif
