// Releases the unselected ropes before the selected rope.

#include "msm_se.h"
#include "REL/m677DLL/motion_attributes.h"
#include "datanum/charmot.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/sound.h"
#include "REL/m677DLL/rope_release_sequence.h"
#include "REL/m677DLL/player_work.h"
#include "REL/m677DLL/rope.h"
#include "datanum/mario.h"

// Selects the first callback and performs startup.
RopeReleaseSequence::RopeReleaseSequence()
    : state(&RopeReleaseSequence::startCharacterAction), countdown(0)
{
    fn_1_7518();
}

// Releases the callback singleton when the sequence ends.
RopeReleaseSequence::~RopeReleaseSequence()
{
}

// Leaves the callback state unchanged at startup.
void RopeReleaseSequence::fn_1_7518()
{
}

// Advances the active sequence and deletes it when it completes.
int updateRopeReleaseSequence()
{
    RopeReleaseSequence *callback = RopeReleaseSequence::current();
    bool ended = callback->advanceStates();
    if (ended) {
        delete callback;
    }
    return ended;
}

// Calls the current state, or reports completion once it is cleared.
bool RopeReleaseSequence::advanceStates()
{
    if (!state) {
        return true;
    }
    (this->*state)();
    return false;
}

// Starts the character's action before waiting for its completion.
void RopeReleaseSequence::startCharacterAction()
{
    PlayerWork &character = PlayerWork::getChecked();
    fn_1_1CFEC(&character);
    setState(&RopeReleaseSequence::waitForCharacterIdle);
}

// Waits for the character to return to its idle action.
void RopeReleaseSequence::waitForCharacterIdle()
{
    PlayerWork &character = PlayerWork::getChecked();
    if (!character.isIdle()) {
        return;
    }
    fn_1_1CEA8(&character, 180.0f, false);
    shiftCharacterMotion(&static_cast<MotionOwner &>(character), MARIO_HSF_c000m1_402,
                         16.0f, 0.0f, loopMotion);
    fn_1_1D27C(&character);
    setCountdown(secondsToFrames<double>(0.5));
    setState(&RopeReleaseSequence::releaseUnselectedRopes);
}

// Releases the three ropes the player did not choose.
void RopeReleaseSequence::releaseUnselectedRopes()
{
    if (updateCountdown()) {
        PlayerWork &character = PlayerWork::getChecked();
        int selected = character.selectedRope();
        for (int ropeIndex = 0; ropeIndex < 4; ++ropeIndex) {
            if (ropeIndex != selected) {
                fn_1_1E7F0(fn_1_1DEB8(ropeIndex));
            }
        }
        shiftCharacterMotion(&static_cast<MotionOwner &>(character), MARIO_HSF_c000m1_403,
                             16.0f, 0.0f, 0);
        soundPlay(MSM_SE_MG677_ROPE_RELEASE);
        setCountdown(secondsToFrames<long>(1));
        setState(&RopeReleaseSequence::releaseSelectedRope);
    }
}

// Releases the chosen rope after the character motion.
void RopeReleaseSequence::releaseSelectedRope()
{
    PlayerWork &character = PlayerWork::getChecked();
    if (!(updateCountdown() && characterMotionEnded(&character.charNo))) {
        return;
    }
    character.updatingRopePull = false;
    fn_1_1E7F0(fn_1_1DEB8(character.selectedRope()));
    shiftCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_300,
                         16.0f, 0.0f, loopMotion);
    setCountdown(secondsToFrames<double>(0.8));
    setState(&RopeReleaseSequence::destroyRopesAfterCountdown);
}

// Destroys the ropes and completes the sequence.
void RopeReleaseSequence::destroyRopesAfterCountdown()
{
    if (updateCountdown()) {
        fn_1_1DFA8();
        setState(0);
    }
}

// Returns the current value of the wrapped player selection.
int fn_1_7B6C(Range<int> *range)
{
    return range->current;
}
