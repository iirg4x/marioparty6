// Runs the Bubble falls event and its result.

#include "REL/m677DLL/motion_attributes.h"
#include "datanum/charmot.h"
#include "REL/framework/vector_math.h"
#include <math.h>
#include "REL/m677DLL/scale_hundred.h"
#include "REL/framework/vector3.h"
#include "humath.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/game_object2.h"
#include "REL/m677DLL/bubble_falls_event.h"
#include "REL/m677DLL/bubble_falls_model.h"
#include "REL/m677DLL/player_work.h"
#include "datadir_enum.h"
#include <memory>

// Starts at the playback model spawn state.
BubbleFallsEvent::BubbleFallsEvent() : state(&BubbleFallsEvent::spawnFallingModels), countdown(0)
{
}

// Destroys the callback and its owned model.
BubbleFallsEvent::~BubbleFallsEvent()
{
}

// Loads the character motions and the playback model.
void BubbleFallsEvent::begin()
{
    PlayerWork &character = PlayerWork::getChecked();
    // Character motions loaded before the playback models spawn.
    static const unsigned int motions[] = { CHARMOT_HSF_c000m1_324, CHARMOT_HSF_c000m1_377,
                                            CHARMOT_HSF_c000m1_404, CHARMOT_HSF_c000m1_474 };
    createCharacterMotions(&static_cast<MotionOwner &>(character), motions, 4);
    model.reset(new OwnedModel(DATANUM(_m677 << 16, 0x1E)));
    model->setAttribute(1);
}

// Runs the current state and reports whether the callback has ended.
int BubbleFallsEvent::step()
{
    if (state) {
        (this->*state)();
    }
    return !state;
}

// Spawns six playback models and starts the first motion.
void BubbleFallsEvent::spawnFallingModels()
{
    PlayerWork &character = PlayerWork::getChecked();
    Vec &position = character.getPosition();
    for (int index = 0; index < 6; ++index) {
        float angle = M_PI * (2.0f * (index / 6.0f));
        Vec spawn;
        vectorSet(&spawn, scaleHundred(sin(angle)), scaleHundred(index * 4 + 7),
                  scaleHundred(cos(angle)));
        PSVECAdd(&position, &spawn, &spawn);
        new BubbleFallsModel(*model, spawn, 180.0 * (angle / M_PI), index == 0);
    }
    Vec eye;
    Vec other;
    fn_80027CD0(1, &eye, &other, &other);
    fn_1_1CEA8(&character, 180.0 * (atan2(eye.x - position.x, eye.z - position.z) / M_PI), false);
    shiftCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_324, 8.0f,
                         0.0f, 0);
    setState(&BubbleFallsEvent::shiftAfterCharacterMotion);
}

// Starts the next motion once the current motion ends.
void BubbleFallsEvent::shiftAfterCharacterMotion()
{
    PlayerWork &character = PlayerWork::getChecked();
    if (characterMotionEnded(&character.charNo)) {
        fn_1_1CFB4(&character, false);
        shiftCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_377, 16.0f,
                             0.0f, loopMotion);
        character.setMotionSpeed(2.5f);
        setState(&BubbleFallsEvent::startCharacterOverlay);
    }
}

// Starts the overlay and sets the countdown once fewer than six objects remain.
void BubbleFallsEvent::startCharacterOverlay()
{
    if (countBubbleFallsModels() < 6) {
        PlayerWork *character = &PlayerWork::getChecked();
        shiftCharacterMotion(&static_cast<MotionOwner &>(*character), CHARMOT_HSF_c000m1_300, 16.0f,
                             0.0f, loopMotion | loopOverlayMotion);
        setCharacterMotionOverlay(&static_cast<MotionOwner &>(*character), *character,
                                  CHARMOT_HSF_c000m1_474);
        setCountdown(secondsToFrames<long>(2));
        setState(&BubbleFallsEvent::waitForModelsAndCountdown);
    }
}

// Ends the callback when the objects are gone and the countdown expires.
void BubbleFallsEvent::waitForModelsAndCountdown()
{
    if (countBubbleFallsModels() <= 0) {
        if (::updateCountdown(countdown)) {
            setState(0);
        }
    }
}
