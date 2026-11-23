// Runs Coins get! and animates the coin models.

#include "msm_se.h"
#include "datanum/charmot.h"
#include "REL/m677DLL/game_constants.h"
#include "REL/framework/vector_math.h"
#include "REL/m677DLL/coins_get_event.h"
#include "REL/m677DLL/coin_model_transform.h"
#include "REL/framework/world_units.h"
#include "REL/framework/random.h"
#include "REL/framework/sound.h"
#include "REL/framework/model.h"
#include "REL/m677DLL/event_base.h"
#include "REL/m677DLL/player_work.h"
#include "datadir_enum.h"
#include "game/charman.h"
#include <memory>

// Character motion resources loaded by this event.
static unsigned int entryMotionData[] = { CHARMOT_HSF_c000m1_306, CHARMOT_HSF_c000m1_325,
                                          CHARMOT_HSF_c000m1_467 };

// Initializes the event entries with random orientations.
CoinsGetEvent::CoinsGetEvent() : state(&CoinsGetEvent::initializeDrops)
{
    frame = 0;
    countdown = 0;
    for (int i = 0; i < 20; ++i) {
        CoinsGetDropModel *entry = &entries[i];
        double angle;
        vectorSet(&entry->position, 0.0f, 0.0f, 0.0f);
        angle = RandomFloat(0.0f, 360.0f);
        vectorSet(&entry->rotation, 0.0f, angle, 0.0f);
        entry->unk1C = 0.0f;
        entry->unk20 = 0.0f;
        entry->unk24 = 0.0f;
        entry->elapsedTime = 0.0f;
        entry->startDelay = 0.06 * i;
        entry->active = 1;
    }
}

// Releases the entry's owned model.
CoinsGetDropModel::~CoinsGetDropModel()
{
}

// Constructs the entry's owned model handle.
CoinsGetDropModel::CoinsGetDropModel()
{
}

// Releases all event entries.
CoinsGetEvent::~CoinsGetEvent()
{
}

// Advances the event and updates the entry models.
int CoinsGetEvent::step()
{
    if (!state) {
        return 1;
    }
    (this->*state)();
    updateDropTransforms();
    return 0;
}

// Rotates the entries and applies their positions to the models.
void CoinsGetEvent::updateDropTransforms()
{
    for (int i = 0; i < 20; ++i) {
        CoinsGetDropModel *entry = &entries[i];
        updateCoinModelTransform(entry);
    }
}

// Loads the character motions and creates the event models.
void CoinsGetEvent::begin()
{
    PlayerWork &character = PlayerWork::getChecked();
    createCharacterMotions(&static_cast<MotionOwner &>(character), entryMotionData, 3);
    for (int i = 0; i < 20; ++i) {
        CoinsGetDropModel &entry = entries[i];
        entry.model.reset(new OwnedModel(DATANUM(_m677 << 16, 0x2C)));
        entry.model->setAttribute(1);
        entry.model->setShadow();
        entry.active = 1;
    }
}

// Sets up the entries around the character and starts the event.
void CoinsGetEvent::initializeDrops()
{
    PlayerWork *motionOwner = &PlayerWork::getChecked();
    PlayerWork *positionOwner = &PlayerWork::getChecked();
    const Vec *position = positionOwner->getPositionPtr();
    for (int i = 0; i < 20; ++i) {
        CoinsGetDropModel *entry = &entries[i];
        entry->model->resetAttribute(1);
        entry->position = *position;
        entry->position.x += RandomFloat(-30.0f, 30.0f);
        entry->position.z += RandomFloat(-30.0f, 30.0f);
        entry->position.y += 1000.0f;
        entry->unk28 = 1;
    }
    shiftCharacterMotion(&static_cast<MotionOwner &>(*motionOwner), CHARMOT_HSF_c000m1_325, 16.0f,
                         0.0f, 0);
    setCountdown((int) secondsToFrames<double>(1.0));
    setState(&CoinsGetEvent::shiftCharacterAfterCountdown);
}

// Waits for the countdown and starts the next character motion.
void CoinsGetEvent::shiftCharacterAfterCountdown()
{
    PlayerWork &character = PlayerWork::getChecked();
    if (::updateCountdown(countdown)) {
        shiftCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_467, 30.0f,
                             0.0f, (1ul << 30) | 1);
        setState(&CoinsGetEvent::advanceActiveDrops);
    }
}

// Advances the active entries relative to the character's height.
void CoinsGetEvent::advanceActiveDrops()
{
    PlayerWork &character = PlayerWork::getChecked();
    float characterHeight = character.height();
    for (int i = 0; i < 20; ++i) {
        CoinsGetDropModel *entry = &entries[i];
        if (entry->active && secondsToFrames(entry->startDelay) < (float) frame) {
            float elapsedSquared = entry->elapsedTime * entry->elapsedTime;
            entry->position.y -= 19.6133f * elapsedSquared;
            if (entry->position.y <= characterHeight) {
                fn_8005D5A0(1, &entry->position);
                entry->model->setAttribute(1);
                entry->active = 0;
                soundPlay(MSM_SE_CMN_COIN_GET);
            }
            entry->elapsedTime += 1.0 / 60.0;
        }
    }
    ++frame;
    if (entries[19].active == 0) {
        shiftCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_306, 16.0f,
                             0.0f, 0);
        setState(&CoinsGetEvent::waitForFinalCharacterMotion);
    }
}

// Completes the event after the character motion ends.
void CoinsGetEvent::waitForFinalCharacterMotion()
{
    PlayerWork &character = PlayerWork::getChecked();
    if (character.isMotionEnd()) {
        setState(0);
    }
}
