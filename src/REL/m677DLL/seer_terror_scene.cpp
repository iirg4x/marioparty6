// Creates the board resources. BoardView selects the weighted event and its result message
// before the opening wipe, then creates the ropes and waits for the player to choose one.

#include "datadir_enum.h"
#include "REL/m677DLL/message_sequence_window.h"
#include <math.h>
#include "REL/m677DLL/seer_terror_scene.h"
#include "REL/m677DLL/tarai_falls_event.h"
#include "REL/m677DLL/bubble_falls_event.h"
#include "REL/m677DLL/stage_rotation_event.h"
#include "REL/m677DLL/bomb_hey_falls_event.h"
#include "REL/m677DLL/meteoric_fall_event.h"
#include "REL/m677DLL/koopa_bomb_event.h"
#include "REL/m677DLL/coins_stolen_event.h"
#include "REL/m677DLL/seer_terror_camera.h"
#include "REL/m677DLL/seer_terror_map.h"
#include "REL/m677DLL/host_character.h"
#include "REL/m677DLL/player_work.h"
#include "REL/m677DLL/rope.h"
#include "REL/framework/random.h"
#include "REL/framework/world_units.h"

#include "REL/framework/frame_interpolator.h"
#include "REL/framework/model_motion.h"
#include "REL/m677DLL/rope_release_sequence.h"
#include "REL/m677DLL/golden_koopa_event.h"
#include "REL/m677DLL/taru_falls_event.h"
#include "REL/m677DLL/jump_up_event.h"
#include "REL/m677DLL/togezoh_falls_event.h"
#include "REL/m677DLL/pitfall_event.h"
#include "REL/m677DLL/event_record.h"
#include "REL/m677DLL/particle_animations.h"
#include "REL/m677DLL/route_runner.h"
#include "REL/m677DLL/wan_wan_chase_event.h"
#include "REL/m677DLL/kinoko_get_event.h"
#include "REL/m677DLL/coins_get_event.h"
#include "REL/m677DLL/dossun_falls_event.h"
#include "REL/m677DLL/koopa_falls_event.h"

#include <memory>
#include <vector>

extern "C" {
#include "game/gamework.h"
}

// Stores a rope's spawn position and sprite-animation resource.
struct RopeSpawnEntry {
    Vec position;
    int dataNumber;
};

// Initializes the resource owner for the board sequence.
SeerTerrorScene::SeerTerrorScene()
{
}

// Deletes the current callback and the shared board model.
SeerTerrorScene::~SeerTerrorScene()
{
    EventBase *callback = EventBase::self;
    delete callback;
    HostCharacter *model = HostCharacter::self;
    delete model;
}

// Creates the board camera controller.
void SeerTerrorScene::createCamera()
{
    new SeerTerrorCamera;
}

// Creates the shared board scene models.
void SeerTerrorScene::createMap()
{
    new SeerTerrorMap;
}

// Creates the animated board model at its starting position.
void SeerTerrorScene::createHostCharacter()
{
    static Vec position = {0.0f, 0.0f, cmToWorld(-385.0f)};
    new HostCharacter(&position);
}

// Creates the character for the first player in group zero.
void SeerTerrorScene::createPlayer()
{
    static Vec position = {0.0f, 0.0f, cmToWorld(667.0f)};
    for (int playerIndex = 0; playerIndex < 4; ++playerIndex) {
        if (lbl_80265728[playerIndex].grpNo == 0) {
            new PlayerWork(playerIndex, position);
            setBlockedPadMask(playerIndex);
            return;
        }
    }
}

// Creates the four selectable ropes.
void SeerTerrorScene::createRopes()
{
    // Spawn positions and sprite-animation resources in selectable-rope order.
    static RopeSpawnEntry entries[] = {
        {{cmToWorld(-400.0f), 0.0f, cmToWorld(350.0f)}, DATANUM(_m677 << 16, 0xF)},
        {{cmToWorld(-135.0f), 0.0f, cmToWorld(350.0f)}, DATANUM(_m677 << 16, 0x10)},
        {{cmToWorld(135.0f), 0.0f, cmToWorld(350.0f)}, DATANUM(_m677 << 16, 0x11)},
        {{cmToWorld(400.0f), 0.0f, cmToWorld(350.0f)}, DATANUM(_m677 << 16, 0x12)},
    };
    PlayerWork *player = &PlayerWork::getChecked();
    float currentHeight = 0.6 * player->height();
    float length = 580.0f;
    for (int i = 0; i < 4; ++i) {
        RopeSpawnEntry *entry = &entries[i];
        float angle = RandomFloat(6.2831855f);
        float radius = length * RandomFloat(0.1f, 0.2f);
        Vec start = entry->position;
        Vec end = entry->position;
        start.y = currentHeight + length;
        end.y = start.y + 0.6 * length;
        end.x = end.x + radius * sin(angle);
        end.z = end.z + radius * cos(angle);
        new Rope(i, &start, &end, length, entry->dataNumber);
    }
}

// Keeps a requested choice index within the range of available choices.
static inline int clampChoiceIndex(int index, int minimum, int maximum)
{
    if (index < minimum) {
        return minimum;
    }
    if (index > maximum) {
        return maximum;
    }
    return index;
}

// Chooses an event record and starts its callback.
unsigned long SeerTerrorScene::startSelectedEvent(int recordIndex, int choiceIndex)
{
    new RopeReleaseSequence;

    int selectedRecord = recordIndex;
    if (selectedRecord < 0 || selectedRecord >= 17) {
        std::vector<int> choices(34);
        choices.clear();
        for (int option = 0; option < 17; ++option) {
            const EventRecord *record = getEventRecord(option);
            for (int index = 0; index < record->resultMessageCount; ++index) {
                choices.push_back(option);
            }
        }
        selectedRecord = choices.at(RandomInteger(choices.size()));
    }

    selectEventRecord(selectedRecord);
    const EventRecord *record = getEventRecord(selectedRecord);
    int event = record->eventNumber;
    int choice = recordIndex < 0 ? RandomInteger(record->resultMessageCount)
                                 : clampChoiceIndex(choiceIndex, 0, record->resultMessageCount - 1);

    switch (event) {
    case 0:
        break;
    case 1:
        new GoldenKoopaEvent;
        break;
    case 2:
        new StageRotationEvent;
        break;
    case 3:
        new TaruFallsEvent;
        break;
    case 4:
        new DossunFallsEvent;
        break;
    case 5:
        new JumpUpEvent;
        break;
    case 6:
        new TaraiFallsEvent;
        break;
    case 7:
        new PitfallEvent;
        break;
    case 8:
        new TogezohFallsEvent;
        break;
    case 9:
        new MeteoricFallEvent;
        break;
    case 10:
        new BubbleFallsEvent;
        break;
    case 11:
        new BombHeyFallsEvent;
        break;
    case 12:
        new WanWanChaseEvent;
        break;
    case 13:
        new CoinsStolenEvent;
        break;
    case 14:
        new CoinsGetEvent;
        break;
    case 15:
        new KinokoGetEvent;
        break;
    case 16:
        new KoopaFallsEvent;
        break;
    case 17:
        new KoopaBombEvent;
        break;
    }

    EventBase *callback = EventBase::current();
    if (callback != 0) {
        callback->begin();
    }
    return record->resultMessages[choice];
}

// Provides the default callback start behavior.
void EventBase::begin()
{
}

// Loads the shared particle animations.
void SeerTerrorScene::loadParticleAnimations()
{
    new ParticleAnimations;
}
