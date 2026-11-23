// Runs the Koopa falls event and its result.

#include "msm_se.h"
#include "datadir_enum.h"
#include "REL/m677DLL/game_constants.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/sound.h"
#include "REL/m677DLL/seer_terror_camera.h"
#include "REL/m677DLL/host_character.h"
#include "REL/m677DLL/player_work.h"
#include "REL/m677DLL/screen_capture_projection.h"
#include "REL/m677DLL/koopa_falls_event.h"

// Camera pose used to start the event.
static const CameraPose cameraPose = {
    {271.93527f, 493.52408f, 177.50407f},
    {-0.28636655f, 0.31021193f, 0.098580435f, 0.90113527f}
};

// Accepts the captured position for debugging.
static inline void DebugPosition(float x) {}

// Tracks the host model's position and downward movement.
class KoopaFallsDescent {
public:
    // Captures the host model position before it begins to fall.
    KoopaFallsDescent() : speed(0.0f), endHeight(-1000.0f)
    {
        HostCharacter &model = HostCharacter::getChecked();
        Vec *captured = model.getPosition(&position);
        DebugPosition(captured->x);
    }

    // Drops the host model and reports when its motion has finished.
    bool update()
    {
        HostCharacter &model = HostCharacter::getChecked();
        if (position.y > endHeight) {
            position.y += speed;
            speed -= gravity;
            model.setPosition(&position);
            if (position.y < -100.0f) {
                model.resetShadow();
            }
            return false;
        }
        position.y = endHeight;
        speed = 0.0f;
        return model.isMotionEnd();
    }

    // Position applied to the host model during the descent.
    Vec position;
    // Vertical displacement per frame.
    float speed;
    // Height at which the descent stops.
    float endHeight;
};

// Starts with the camera transition and captures the host model's position.
KoopaFallsEvent::KoopaFallsEvent()
    : state(&KoopaFallsEvent::startCameraTransition), countdown(0),
      transition(new KoopaFallsDescent)
{
}

// Releases the transition's owned state.
KoopaFallsEvent::~KoopaFallsEvent()
{
}

// Leaves the initial transition state ready for the first step.
void KoopaFallsEvent::begin()
{
}

// Runs the current state and arms the next sequence if it clears the callback.
int KoopaFallsEvent::step()
{
    if (state) {
        (this->*state)();
    }
    if (!state) {
        setState(&KoopaFallsEvent::playFollowupCue);
        return 1;
    }
    return 0;
}

// Runs the remaining states until the callback has cleared.
int KoopaFallsEvent::advanceStates()
{
    if (state) {
        (this->*state)();
    }
    return !state;
}

// Starts the camera movement.
void KoopaFallsEvent::startCameraTransition()
{
    SeerTerrorCamera::getChecked().transitionToPose(&cameraPose, 1.5f);
    setState(&KoopaFallsEvent::createScreenAfterCamera);
}

// Creates the screen work after the camera movement and starts the delay.
void KoopaFallsEvent::createScreenAfterCamera()
{
    if (SeerTerrorCamera::getChecked().isTransitionDone()) {
        HostCharacter &model = HostCharacter::getChecked();
        Vec position;
        model.getPosition(&position);
        screen.reset(new ScreenCaptureProjection(&position, 160.0f, 0.8f));
        setCountdown(secondsToFrames(1.8));
        PlayerWork::getChecked().rumble(2, 24);
        setState(&KoopaFallsEvent::startHostMotionAfterDelay);
    }
}

// Starts the host model's motion once the countdown expires.
void KoopaFallsEvent::startHostMotionAfterDelay()
{
    if (::updateCountdown(countdown)) {
        HostCharacter &model = HostCharacter::getChecked();
        model.shiftMotion(DATANUM(_m677 << 16, 0xA), secondsToFrames(0.14), 0.0f, 0);
        soundPlay(MSM_SE_MG677_KOOPA_FALLS);
        soundPlay(MSM_SE_GUIDE_KOOPA_REACT);
        setState(&KoopaFallsEvent::updateModelDescent);
    }
}

// Moves the model down until its motion completes.
void KoopaFallsEvent::updateModelDescent()
{
    if (transition->update()) {
        setState(0);
    }
}

// Plays the first cue of the follow-up sequence.
void KoopaFallsEvent::playFollowupCue()
{
    soundPlay(MSM_SE_MG677_KOOPA_FALLS);
    setState(&KoopaFallsEvent::startScreenTransition);
}

// Starts the screen transition and waits its duration.
void KoopaFallsEvent::startScreenTransition()
{
    soundPlay(MSM_SE_GUIDE_KOOPA_LAUGH);
    screen->setProjectionScaleTarget(0.0f, 2.0f);
    setCountdown(secondsToFrames(2L));
    setState(&KoopaFallsEvent::waitForScreenCountdown);
}

// Ends the sequence once the screen transition has had time to finish.
void KoopaFallsEvent::waitForScreenCountdown()
{
    if (::updateCountdown(countdown)) {
        setState(0);
    }
}

// Provides the callback's empty transition step.
void KoopaFallsEvent::fn_1_17104()
{
}
