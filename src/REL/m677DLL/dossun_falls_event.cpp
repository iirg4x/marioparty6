// Runs the Dossun falls event and its result.

#include "msm_se.h"
#include "datanum/charmot.h"
#include "REL/m677DLL/game_constants.h"
#include "REL/m677DLL/motion_attributes.h"
#include "REL/framework/vector_math.h"
#include "REL/m677DLL/dossun_falls_event.h"
#include "REL/framework/world_units.h"
#include "REL/framework/sound.h"
#include "REL/m677DLL/event_base.h"
#include "REL/m677DLL/seer_terror_camera.h"
#include "REL/m677DLL/particle_animations.h"
#include "REL/m677DLL/player_work.h"
#include "REL/framework/model.h"
#include "datadir_enum.h"
#include <memory>

// Initializes the descent of a single Dossun from above the player.
DossunFallsEvent::DossunFallsEvent() : state(&DossunFallsEvent::beginDescent)
{
    elapsedTime = 2.0f;
    countdown = 0;
    vectorSet(&position, 0.0f, 0.0f, 0.0f);
}

// Releases the callback's owned state.
DossunFallsEvent::~DossunFallsEvent()
{
}
// Runs the current callback and reports completion.
int DossunFallsEvent::step()
{
    if (state)
        (this->*state)();
    return !state;
}
// Loads the character motions used by the callback.
void DossunFallsEvent::begin()
{
    PlayerWork &character = PlayerWork::getChecked();
    createCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_325);
    createCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_357);
    model.reset(new OwnedModel(DATANUM(_m677 << 16, 0x17)));
    model->setAttribute(pauseMotion);
    model->setAttribute(1);
    model->setShadow();
    model->setLayer(2);
}
// Places the Dossun above the character and begins its descent.
void DossunFallsEvent::beginDescent()
{
    PlayerWork *character = &PlayerWork::getChecked();
    Vec *positionVector = &character->getPosition();
    this->position.x = positionVector->x;
    this->position.y = positionVector->y;
    this->position.z = positionVector->z;
    this->position.y += 1000.0f;
    this->model->resetAttribute(pauseMotion);
    this->model->resetAttribute(1);
    this->model->setPosition(&this->position);
    shiftCharacterMotion(&static_cast<MotionOwner &>(*character), CHARMOT_HSF_c000m1_325, 16.0f,
                         0.0f, 0);
    this->setState(&DossunFallsEvent::updateDescent);
}
// Moves the Dossun down. At character height plus 150, triggers the reaction and rumble.
// At ground contact, spawns particles and shakes the camera.
void DossunFallsEvent::updateDescent()
{
    PlayerWork &character = PlayerWork::getChecked();
    float initialHeight = position.y;
    float threshold = (float) (character.height() + cmToWorld(150.0));
    float elapsedSquared = elapsedTime * elapsedTime;
    position.y -= 4.903325f * elapsedSquared;
    elapsedTime += 1.0 / 60.0;
    model->setPosition(&position);
    if (initialHeight > threshold) {
        if (position.y <= threshold) {
            shiftCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_357,
                                 16.0f, 0.0f, 0);
            character.rumble(2, 24);
        }
    }
    if (position.y > 0.0) {
        return;
    }
    position.y = 0.0f;
    model->setPosition(&position);
    ParticleAnimations *resource = &ParticleAnimations::getChecked();
    resource->createGrayImpactParticles(&position, 256, 0.5f, 4.0f, 1);
    SeerTerrorCamera::getChecked().startShake(0.5f, 0.5f);
    fn_1_1CF40(&character, false);
    soundPlay(MSM_SE_MG677_HEAVY_LANDING);
    soundPlay(MSM_SE_GUIDE_HEAVY_SLAM);
    setCountdown(secondsToFrames<double>(3.0));
    setState(&DossunFallsEvent::waitForLandingCountdown);
}

// Ends the sequence when the landing countdown expires.
void DossunFallsEvent::waitForLandingCountdown()
{
    if (::updateCountdown(countdown)) {
        setState(0);
    }
}
