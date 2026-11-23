// Runs the Golden Koopa event and its result.

#include "msm_se.h"
#include "datanum/charmot.h"
#include "REL/m677DLL/game_constants.h"
#include "REL/m677DLL/motion_attributes.h"
#include "REL/framework/world_units.h"
#include "REL/framework/model.h"
#include "REL/framework/sound.h"
#include "REL/m677DLL/golden_koopa_event.h"
#include "REL/m677DLL/seer_terror_camera.h"
#include "REL/m677DLL/particle_animations.h"
#include "REL/m677DLL/player_work.h"
#include "datadir_enum.h"
#include <memory>

// Starts in the state that positions the model.
GoldenKoopaEvent::GoldenKoopaEvent()
    : state(&GoldenKoopaEvent::placeModelAndStartMotion), model(), countdown(0)
{
}

// Releases the model at the end of the callback sequence.
GoldenKoopaEvent::~GoldenKoopaEvent()
{
}

// Advances one state and returns whether the sequence finished.
int GoldenKoopaEvent::step()
{
    if (state) {
        (this->*state)();
    }
    return !state;
}

// Loads the character's motions and prepares the overlaid model.
void GoldenKoopaEvent::begin()
{
    PlayerWork &character = PlayerWork::getChecked();
    createCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_325);
    createCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_357);
    model.reset(new OwnedModel(DATANUM(_m677 << 16, 0x14)));
    model->setAttribute(pauseMotion);
    model->setAttribute(1);
    model->setShadow();
    model->setLayer(6);
}

// Places the model over the character and starts the character motion.
void GoldenKoopaEvent::placeModelAndStartMotion()
{
    PlayerWork &character = PlayerWork::getChecked();
    Vec *position = &PlayerWork::getChecked().getPosition();
    model->setPosition(position);
    shiftCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_325, 16.0f,
                         0.0f, 0);
    setCountdown(secondsToFrames<double>(0.2));
    setState(&GoldenKoopaEvent::releaseModelAttributes);
}

// Releases the model's initial attributes once the countdown expires.
void GoldenKoopaEvent::releaseModelAttributes()
{
    if (::updateCountdown(countdown)) {
        model->resetAttribute(pauseMotion);
        model->resetAttribute(1);
        setState(&GoldenKoopaEvent::reactAtBodyHeight);
    }
}

// Shifts the character's motion when the model's body drops below the threshold.
void GoldenKoopaEvent::reactAtBodyHeight()
{
    PlayerWork &character = PlayerWork::getChecked();
    Vec position;
    model->getObjectPosition("body", &position);
    if (position.y < 3.0f * character.height()) {
        shiftCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_357, 16.0f,
                             0.0f, 0);
        character.rumble(2, 24);
        setState(&GoldenKoopaEvent::playLandingEffects);
    }
}

// Plays the particle and camera effects once the model's motion finishes.
void GoldenKoopaEvent::playLandingEffects()
{
    PlayerWork &character = PlayerWork::getChecked();
    ParticleAnimations &particles = ParticleAnimations::getChecked();
    const Vec &characterPosition = PlayerWork::getChecked().getPosition();
    Vec position = characterPosition;
    if (model->isMotionEnd()) {
        fn_1_1CF40(&character, false);
        particles.createGrayImpactParticles(&position, 256, 0.5f, 4.0f, 1);
        SeerTerrorCamera::getChecked().startShake(0.5f, 0.5f);
        setCountdown(secondsToFrames<double>(3.0));
        soundPlay(MSM_SE_MG677_GOLDEN_KOOPA_LAND);
        setState(&GoldenKoopaEvent::waitForFinalCountdown);
    }
}

// Clears the active state once the last countdown expires.
void GoldenKoopaEvent::waitForFinalCountdown()
{
    if (::updateCountdown(countdown)) {
        setState(0);
    }
}
