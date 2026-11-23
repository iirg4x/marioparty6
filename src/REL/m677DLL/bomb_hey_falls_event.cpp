// Runs the Bomb-hey falls event and its result.

#include "msm_se.h"
#include "datanum/charmot.h"
#include "REL/framework/vector_math.h"
#include "REL/m677DLL/game_constants.h"
#include "REL/m677DLL/motion_attributes.h"
#include <math.h>
#include "datadir_enum.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/model_motion.h"
#include "REL/framework/sound.h"
#include "REL/m677DLL/bomb_hey_falls_event.h"
#include "REL/m677DLL/seer_terror_camera.h"
#include "REL/m677DLL/particle_animations.h"
#include "REL/m677DLL/player_work.h"

extern "C" {
#include "game/audio.h"
}

static float lbl_1_bss_C8 = cmToWorld(150.0f);

// Starts the impact callback in its initial state.
BombHeyFallsEvent::BombHeyFallsEvent()
    : state(&BombHeyFallsEvent::initializeImpactEntry), countdown(0), transitionCountdown(0)
{
}

// Releases the entry's owned model.
BombHeyFallsEvent::Entry::~Entry()
{
}

// Constructs the entry's owned model handle.
BombHeyFallsEvent::Entry::Entry()
{
}

// Releases all impact entries.
BombHeyFallsEvent::~BombHeyFallsEvent()
{
}

// Advances the callback and updates its impact entries.
int BombHeyFallsEvent::step()
{
    if (state) {
        (this->*state)();
        updateImpactEntry();
    }
    return !state;
}

// Loads the character motions and prepares the impact models.
void BombHeyFallsEvent::begin()
{
    PlayerWork &character = PlayerWork::getChecked();
    static const unsigned int motions[] = { CHARMOT_HSF_c000m1_325, CHARMOT_HSF_c000m1_404,
                                            CHARMOT_HSF_c000m1_320 };
    createCharacterMotions(&static_cast<MotionOwner &>(character), motions, 3);
    for (int index = 0; index < 1; ++index) {
        Entry &entry = entries[index];
        entry.model.reset(new ModelMotion(DATANUM(_m677 << 16, 0x1F)));
        entry.model->setAttribute(1);
        entry.model->addMotion(DATANUM(_m677 << 16, 0x24));
        entry.model->setAttribute(pauseMotion);
        entry.model->setShadow();
        entry.model->setLayer(2);
        entry.overlay.reset(new OwnedModel(DATANUM(_m677 << 16, 0x21)));
        entry.overlay->setAttribute(1);
        entry.overlay->setAttribute(pauseMotion);
        entry.overlay->setLayer(5);
        vectorSet(&entry.position, 0.0f, 0.0f, 0.0f);
        entry.unk14 = 0.0f;
        entry.angle = static_cast<double>(index * 360 - 90);
        entry.unk1C = 0.0f;
        entry.fallScale = 2.5f;
        entry.unk24 = 0.2f * index;
        entry.elapsed = 0.0f;
        entry.state = 0;
    }
    countdown = 0;
}

// Places the bomb above and horizontally offset from the character.
void BombHeyFallsEvent::initializeImpactEntry()
{
    PlayerWork &character = PlayerWork::getChecked();
    for (int index = 0; index < 1; ++index) {
        Entry &entry = entries[index];
        entry.position = character.getPosition();
        entry.position.x += lbl_1_bss_C8 * cos(M_PI * entry.angle / 180.0);
        entry.position.y += 800.0f;
        entry.position.z += lbl_1_bss_C8 * sin(M_PI * entry.angle / 180.0);
        entry.model->setPosition(&entry.position);
        entry.model->resetAttribute(1);
        entry.state = 1;
    }
    shiftCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_325, 16.0f,
                         0.0f, 0);
    setState(&BombHeyFallsEvent::startFollowupCountdown);
}

// Starts the follow-up countdown after the bomb lands and enters state 3.
void BombHeyFallsEvent::startFollowupCountdown()
{
    if (entries[0].state == 3) {
        setCountdown(secondsToFrames<double>(1.0));
        setState(&BombHeyFallsEvent::startFollowupEffect);
    }
}

// Triggers the follow-up effect when its countdown expires.
void BombHeyFallsEvent::startFollowupEffect()
{
    if (::updateCountdown(countdown)) {
        PlayerWork &character = PlayerWork::getChecked();
        Vec launchVelocity = { 0.0f, 0.0f, 0.0f };
        launchVelocity.y = framesToSeconds<int>(1500);
        launchVelocity.z = framesToSeconds<int>(800);
        character.offset = launchVelocity;
        shiftCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_320, 16.0f,
                             0.0f, loopMotion);
        character.rumble(2, 24);
        CharFXPlay(character.characterNo(), MSM_SE_CHARVOICE_HIT_YELP);
        entries[0].model->setAttribute(1);
        entries[0].overlay->resetAttribute(pauseMotion);
        entries[0].overlay->resetAttribute(1);
        entries[0].state = 4;
        SeerTerrorCamera::getChecked().startShake(0.5f, 0.5f);
        soundPlay(MSM_SE_MG677_BOMB_HEY_LAUNCH);
        setCountdown(secondsToFrames<double>(2.0));
        setState(&BombHeyFallsEvent::disableEffectAfterCountdown);
    }
}

// Disables the character effect and completes the callback after the countdown.
void BombHeyFallsEvent::disableEffectAfterCountdown()
{
    if (::updateCountdown(countdown)) {
        PlayerWork *character = &PlayerWork::getChecked();
        fn_1_1CF40(character, false);
        setState(0);
    }
}

// Updates entry motion and emits particles when the model impacts.
void BombHeyFallsEvent::updateImpactEntry()
{
    PlayerWork &character = PlayerWork::getChecked();
    ParticleAnimations &particles = ParticleAnimations::getChecked();
    for (int index = 0; index < 1; ++index) {
        Entry &entry = entries[index];
        switch (entry.state) {
        case 1:
            entry.state = 2;
            soundPlay(MSM_SE_GUIDE_OBJECT_CATCH);
            break;
        case 2: {
            float elapsedSquared = entry.elapsed * entry.elapsed;
            entry.position.y -= 4.903325f * entry.fallScale * elapsedSquared;
            if (entry.position.y <= 0.0f) {
                entry.position.y = 0.0f;
                particles.createGrayImpactParticles(&entry.position, 16, 0.3f, 2.0f, 1);
                entry.model->resetAttribute(pauseMotion);
                entry.model->shiftMotion(DATANUM(_m677 << 16, 0x24), 16.0f, 0.0f, loopMotion);
                setTransitionCountdown(secondsToFrames<double>(0.2));
                entry.state = 3;
            }
            entry.elapsed += 1.0 / 60.0;
            break;
        }
        case 3: {
            if (!::updateCountdown(transitionCountdown)) {
                return;
            }
            if (entries[0].state == 3) {
                shiftCharacterMotion(&static_cast<MotionOwner &>(character),
                                     CHARMOT_HSF_c000m1_345, 16.0f, 0.0f, loopMotion);
                setTransitionCountdown(secondsToFrames<double>(0.3));
                entry.state = 4;
            }
            break;
        }
        case 4: {
            if (!::updateCountdown(transitionCountdown)) {
                return;
            }
            fn_1_1CEA8(&character, 0.0f, false);
            entry.state = 5;
            break;
        }
        case 5:
            break;
        }
        entry.model->setPosition(&entry.position);
        entry.overlay->setPosition(&entry.position);
    }
}
