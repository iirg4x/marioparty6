#include "REL/m671DLL/player_object.h"
#include "REL/m671DLL/collision.h"
#include "REL/m671DLL/callback_receiver.h"
#include "REL/m671DLL/position_table.h"

#include "REL/m671DLL/scene_object.h"

#include "REL/framework/phase_durations.h"

#include "REL/framework/thread.h"

#include "REL/framework/minigame_core.h"
#include "REL/framework/event.h"
#include "REL/framework/game_object_man.h"

#include "msm_stream.h"

#include "REL/framework/game_object.h"

#include "REL/m671DLL/resources.h"

#include "REL/m671DLL/camera_motion.h"

extern "C" {
#include "game/audio.h"
}

#include "game/mg/actman.h"
#include <vector>

extern "C" unsigned long OSGetTick(void);

void RandomSeed(unsigned long);

SceneObjectOwner::SceneObjectOwner() : ThreadEntry(0x10000), childOwner(new OwnedSceneChild)
{
    RandomSeed(OSGetTick());
    triggered = 0;
}

void collectGameObjects(int category, std::vector<GameObject *> *list);

SceneObjectOwner::~SceneObjectOwner() {}

void SceneObjectOwner::setupObjects(OMOBJMAN *)
{
    new CameraOwner;

    Resources *state = &MinigameSingleton<Resources>::getChecked();
    state->fn_1_718C();
    state->fn_1_74B8();
    state->fn_1_751C();
    state->fn_1_75A4();
    state->fn_1_763C();

    new PositionTable;
}

void SceneObjectOwner::updateGame()
{
    resume(0);
    process_event_queue();
    updateGameObjects();
    fn_800856A8();
    fn_1_C58();
}

extern int fn_1_7D68(SceneObjectOwner *);

extern bool fn_1_7F40(void *owner);

extern int fn_1_8004(void);

void SceneObjectOwner::run(void *)
{
    int musicHandle;
    int waitIndex;
    int transitionIndex;
    bool resultCode;
    int jingle;
    int tailWaitIndex;
    int index;

    MinigameCore::wipeIn();
    musicHandle = HuAudBGMPlay(MSM_STREAM_GAMING_WITH_BOWSER);
    sendEvent(0, 0, 0.0f);
    waitIndex = 0;
    while (waitIndex < 0x78) {
        suspend(0);
        waitIndex++;
    }
    sendEvent(0, 1, 0.0f);
    transitionIndex = 0;
    while ((double)transitionIndex < 90.0) {
        suspend(0);
        transitionIndex++;
    }
    MinigameCore::showStart();
    sendEvent(0, 2, 0.0f);
    while (MinigameCore::messageDone() == 0) {
        suspend(0);
    }
    sendEvent(0, 3, 0.0f);
    while ((unsigned char)fn_1_7D68(this) == 0) {
        suspend(0);
    }
    HuAudSStreamFadeOut(musicHandle, 0x64);
    MinigameCore::showFinish();
    sendEvent(0, 5, 0.0f);
    tailWaitIndex = 0;
    while (tailWaitIndex < 0x78) {
        suspend(0);
        tailWaitIndex++;
    }
    sendEvent(0, 6, 0.0f);
    while ((unsigned char)fn_1_8004() == 0) {
        suspend(0);
    }
    resultCode = fn_1_7F40(this);
    if (resultCode == 1) {
        jingle = 0x43;
    } else {
        jingle = 0x45;
    }
    HuAudJinglePlay((short)jingle);
    sendEvent(0, 7, &resultCode, 0.0f);
    index = 0;
    while ((double)index < 180.0) {
        suspend(0);
        index++;
    }
    MinigameCore::requestExit();
    for (;;) {
        suspend((void *)-1);
    }
}

int fn_1_7D68(SceneObjectOwner *owner)
{
    static float elapsed = 0.0f;
    elapsed += 0.016666666666666666;
    if (elapsed >= 25.0f) {
        return 1;
    }
    if (!owner->triggered && elapsed >= 20.0f) {
        sendEvent(0, 4, 0.0f);
        owner->triggered = 1;
    }

    std::vector<GameObject *> list;
    int index;
    collectGameObjects(0, &list);
    int count = list.size();
    int activeCount = 0;
    for (index = 0; index < count; index++) {
        Player *object = static_cast<Player *>(list[index]);
        if (!object->isActive()) {
            activeCount++;
        }
    }
    if (activeCount == 4) {
        return 1;
    }
    return 0;
}

void SceneObjectOwner::finishGame()
{
}

bool fn_1_7F40(void *owner)
{
    std::vector<GameObject *> list;
    collectGameObjects(0, &list);
    int count = list.size();
    int activeCount = 0;
    for (int index = 0; index < count; index++) {
        Player *object = static_cast<Player *>(list[index]);
        if (object->isActive()) {
            activeCount++;
        }
    }
    return activeCount != 0;
}

int fn_1_8004(void)
{
    std::vector<GameObject *> firstList;
    int firstCount;
    int secondCount;
    Player *firstObject;
    CallbackReceiver *secondObject;
    int firstIndex;
    int secondIndex;

    collectGameObjects(0, &firstList);
    firstCount = firstList.size();
    for (firstIndex = 0; firstIndex < firstCount; firstIndex++) {
        firstObject = (Player *)firstList[firstIndex];
        if (!firstObject->finished) return 0;
    }
    std::vector<GameObject *> secondList;
    collectGameObjects(4, &secondList);
    secondCount = secondList.size();
    for (secondIndex = 0; secondIndex < secondCount; secondIndex++) {
        secondObject = (CallbackReceiver *)secondList[secondIndex];
        if (!secondObject->active) return 0;
    }
    return 1;
}
