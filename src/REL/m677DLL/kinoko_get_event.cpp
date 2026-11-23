// Runs the Kinoko get event and its result.

#include "msm_se.h"
#include "datanum/charmot.h"
#include "REL/m677DLL/game_constants.h"
#include "REL/m677DLL/motion_attributes.h"
#include "REL/framework/vector_math.h"
#include "REL/m677DLL/kinoko_get_event.h"
#include "REL/framework/world_units.h"
#include "REL/framework/model.h"
#include "REL/framework/sound.h"
#include "REL/m677DLL/event_base.h"
#include "REL/m677DLL/player_work.h"
#include "REL/m677DLL/particle_animations.h"
#include "datadir_enum.h"
#include "game/charman.h"
#include <memory>

extern "C" {
#include "game/audio.h"
}

// Initializes the model position and descent step.
KinokoGetEvent::KinokoGetEvent() : state(&KinokoGetEvent::alignModelAndStartCharacterMotion)
{
    heightStep = (f32)framesToSeconds(200.0);
    elapsedFrames = 0;
    countdown = 0;
    double initialY;
    double initialZ;
    initialZ = 667.0;
    initialY = 700.0;
    position.x = 0.0f;
    position.y = (f32)initialY;
    position.z = (f32)initialZ;
    vectorSet(&rotation, 0.0f, 0.0f, 0.0f);
}

// Releases the moving model.
KinokoGetEvent::~KinokoGetEvent() {}

// Runs the active state and reports completion.
int KinokoGetEvent::step()
{
    if (state) {
        (this->*state)();
    }
    return !state;
}

// Creates the model and loads the character motions.
void KinokoGetEvent::begin()
{
    model.reset(new OwnedModel(DATANUM(_m677 << 16, 0x2D)));
    model->setAttribute(1);
    model->setLayer(2);
    model->setShadow();
    PlayerWork &character = PlayerWork::getChecked();
    createCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_306);
    createCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_325);
}

// Aligns the model above the character and starts the reaction motion.
void KinokoGetEvent::alignModelAndStartCharacterMotion()
{
    PlayerWork &character = PlayerWork::getChecked();
    PlayerWork &positionOwner = PlayerWork::getChecked();
    const Vec *target = positionOwner.getPositionPtr();
    vectorSet(&position, target->x, position.y, target->z);
    model->resetAttribute(1);
    model->setPosition(&position);
    shiftCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_325,
        16.0f, 0.0f,
                         0);
    setCountdown((int)secondsToFrames(1.0));
    setState(&KinokoGetEvent::shiftCharacterMotionAfterCountdown);
}

// Starts the next character motion when the countdown expires.
void KinokoGetEvent::shiftCharacterMotionAfterCountdown()
{
    if (::updateCountdown(countdown)) {
        PlayerWork &character = PlayerWork::getChecked();
        shiftCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_300, 16.0f,
                             0.0f, loopMotion);
        soundPlay(MSM_SE_MG677_KINOKO_LOWER);
        setState(&KinokoGetEvent::lowerModelToCharacter);
    }
}

// Lowers the model and triggers the effect when it reaches the character.
void KinokoGetEvent::lowerModelToCharacter()
{
    PlayerWork *player = &PlayerWork::getChecked();
    position.y -= heightStep;
    if (position.y <= player->height()) {
        fn_8005D5A0(1, &position);
        model->setAttribute(1);
        model->setPosition(&position);
        soundPlay(MSM_SE_MG677_KINOKO_RECEIVE);
        fn_1_1CFB4(player, false);
        shiftCharacterMotion(&static_cast<MotionOwner &>(*player), CHARMOT_HSF_c000m1_306, 16.0f,
                             0.0f, 0);
        player->rumble(1, 24);
        CharFXPlay(player->characterNo(), MSM_SE_CHARVOICE_HAPPY_CHEER);
        setState(&KinokoGetEvent::waitForCharacterMotionEnd);
    }
    rotation.y = rotation.y + framesToSeconds(180.0);
    if ((double)elapsedFrames > secondsToFrames(0.1)) {
        ParticleAnimations *animations = &ParticleAnimations::getChecked();
        animations->createRisingYellowParticles(&position, 16, 0.15f);
        elapsedFrames = 0;
    }
    ++elapsedFrames;
    model->setPosition(&position);
    model->setRotation(&rotation);
}

// Ends the callback after the character motion finishes.
void KinokoGetEvent::waitForCharacterMotionEnd()
{
    PlayerWork &character = PlayerWork::getChecked();
    if (character.isMotionEnd()) {
        setState(0);
    }
}
