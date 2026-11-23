// Runs the Taru falls event and its result.

#include "msm_se.h"
#include "datanum/charmot.h"
#include "REL/m677DLL/game_constants.h"
#include "REL/m677DLL/motion_attributes.h"
#include "REL/framework/frame_interpolator.h"
#include "REL/framework/world_units.h"
#include "REL/framework/model.h"
#include "REL/framework/sound.h"
#include "REL/m677DLL/taru_falls_event.h"
#include "REL/m677DLL/player_work.h"
#include "datadir_enum.h"
#include <memory>
#include <vector>

// Motion frames for the model sound cues.
static const int effectFrames[] = { 98, 204 };

// Starts in the state that selects the character motion.
TaruFallsEvent::TaruFallsEvent()
    : state(&TaruFallsEvent::startMotionAndRevealTimer),
      model(), characterScale(1.0f), countdown(0),
      soundFrames(effectFrames, effectFrames + 2)
{
}

// Releases the model and its animation cue list.
TaruFallsEvent::~TaruFallsEvent()
{
}

// Advances the active state and reports whether the sequence finished.
int TaruFallsEvent::step()
{
    if (state) {
        (this->*state)();
    }
    return !state;
}

// Loads the character motions and prepares the model.
void TaruFallsEvent::begin()
{
    PlayerWork &character = PlayerWork::getChecked();
    createCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_325);
    createCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_399);
    model.reset(new OwnedModel(DATANUM(_m677 << 16, 0x16)));
    model->setAttribute(pauseMotion);
    model->setAttribute(1);
    model->setShadow();
    model->setLayer(2);
}

// Starts the first motion and schedules the model reveal.
void TaruFallsEvent::startMotionAndRevealTimer()
{
    PlayerWork &character = PlayerWork::getChecked();
    shiftCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_325,
                         16.0f, 0.0f, 0);
    setCountdown(secondsToFrames<double>(1.0));
    setState(&TaruFallsEvent::revealModelAfterDelay);
}

// Positions and reveals the model once the initial wait ends.
void TaruFallsEvent::revealModelAfterDelay()
{
    if (::updateCountdown(countdown)) {
        PlayerWork &character = PlayerWork::getChecked();
        Vec *position = &character.getPosition();
        model->setPosition(position);
        model->resetAttribute(pauseMotion);
        model->resetAttribute(1);
        shiftCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_399,
                             16.0f, 0.0f, 0);
        setCountdown(secondsToFrames<double>(0.4));
        setState(&TaruFallsEvent::startCharacterScaleChange);
    }
}

// Starts the character scale change and waits for the model motion.
void TaruFallsEvent::startCharacterScaleChange()
{
    if (::updateCountdown(countdown)) {
        PlayerWork &character = PlayerWork::getChecked();
        character.rumble(2, 24);
        characterScale.set(0.1f, static_cast<long>(secondsToFrames<double>(0.25)));
        setCountdown(static_cast<int>(26.0f - model->getMotionTime()));
        setState(&TaruFallsEvent::updateCharacterScale);
    }
}

// Updates the character scale and cues the next sound.
void TaruFallsEvent::updateCharacterScale()
{
    PlayerWork &character = PlayerWork::getChecked();
    character.setUniformScale(characterScale.update());
    if (::updateCountdown(countdown)) {
        fn_1_1CF40(&character, false);
        soundPlay(MSM_SE_MG677_TARU_IMPACT);
        setCountdown(secondsToFrames<long>(2));
        setState(&TaruFallsEvent::playTimedMotionCues);
    }
}

// Plays the sound cues at their animation frames, then finishes after the motion.
void TaruFallsEvent::playTimedMotionCues()
{
    if (!soundFrames.empty()) {
        if (model->getMotionTime() >= *soundFrames.begin()) {
            soundPlay(MSM_SE_MG677_TARU_MOTION_CUE);
            soundFrames.erase(soundFrames.begin());
        }
    }
    if (model->isMotionEnd()) {
        if (::updateCountdown(countdown)) {
            setState(0);
        }
    }
}
