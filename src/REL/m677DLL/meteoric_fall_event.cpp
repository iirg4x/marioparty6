// Runs the Meteoric fall event and its result.

#include "msm_se.h"
#include "datanum/charmot.h"
#include "REL/m677DLL/game_constants.h"
#include "REL/m677DLL/motion_attributes.h"
#include "REL/framework/sound.h"
#include "REL/framework/vector_math.h"
#include "REL/framework/world_units.h"
#include "REL/m677DLL/meteoric_fall_event.h"
#include "REL/m677DLL/seer_terror_camera.h"
#include "REL/m677DLL/host_character.h"
#include "REL/m677DLL/particle_animations.h"
#include "REL/m677DLL/player_work.h"
#include "datadir_enum.h"

// Starts the callback in its initial state.
MeteoricFallEvent::MeteoricFallEvent()
    : state(&MeteoricFallEvent::revealModelAndPlaySound), countdown(0)
{
}

// Releases the callback's owned model.
MeteoricFallEvent::~MeteoricFallEvent()
{
}

// Runs the active state and reports completion.
int MeteoricFallEvent::step()
{
    if (state) {
        (this->*state)();
    }
    return !state;
}

// Loads the character motions and prepares the callback model.
void MeteoricFallEvent::begin()
{
    PlayerWork &character = PlayerWork::getChecked();
    static const unsigned int motions[] = {
        CHARMOT_HSF_c000m1_322, CHARMOT_HSF_c000m1_325, CHARMOT_HSF_c000m1_404
    };
    createCharacterMotions(&static_cast<MotionOwner &>(character), motions, 3);
    model.reset(new OwnedModel(DATANUM(_m677 << 16, 0x1D)));
    model->setAttribute(pauseMotion);
    model->setAttribute(1);
    model->setLayer(4);
}

// Reveals the model and starts the character motion and sound.
void MeteoricFallEvent::revealModelAndPlaySound()
{
    model->resetAttribute(pauseMotion);
    model->resetAttribute(1);
    soundPlay(MSM_SE_MG677_METEORIC_FALL_START);
    PlayerWork &character = PlayerWork::getChecked();
    shiftCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_325,
        16.0f, 0.0f,
                         0);
    setCountdown((int)secondsToFrames<double>(0.6));
    setState(&MeteoricFallEvent::shiftCharacterAfterCountdown);
}

// After the countdown, starts the next player motion, triggers the host reaction and cue,
// and spawns impact particles.
void MeteoricFallEvent::shiftCharacterAfterCountdown()
{
    PlayerWork *character;
    if (::updateCountdown(countdown)) {
        character = &PlayerWork::getChecked();
        shiftCharacterMotion(&static_cast<MotionOwner &>(*character), CHARMOT_HSF_c000m1_404,
                             16.0f, 0.0f, 0);
        character->rumble(2, 24);
        HostCharacter &colorModel = HostCharacter::getChecked();
        colorModel.fn_1_189CC();
        Vec position;
        vectorSet(&position, 0.0, 0.0, cmToWorld(667.0));
        ParticleAnimations &particles = ParticleAnimations::getChecked();
        particles.createGrayImpactParticles(&position, 256, 0.6f, 6.0f, 1);
        SeerTerrorCamera::getChecked().startShake(1.0f, 0.5f);
        setState(&MeteoricFallEvent::startFinalCharacterMotion);
    }
}

// Starts the final motion after the current character motion ends.
void MeteoricFallEvent::startFinalCharacterMotion()
{
    PlayerWork &character = PlayerWork::getChecked();
    if (character.isMotionEnd()) {
        shiftCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_322, 16.0f,
                             0.0f, loopMotion);
        setCountdown(secondsToFrames<long>(3));
        setState(&MeteoricFallEvent::waitForFinalCountdown);
    }
}

// Completes the callback when its last countdown expires.
void MeteoricFallEvent::waitForFinalCountdown()
{
    if (::updateCountdown(countdown)) {
        setState(0);
    }
}
