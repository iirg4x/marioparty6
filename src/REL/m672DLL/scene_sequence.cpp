#include "dolphin/os.h"
#include <new>
#include "REL/framework/random.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "REL/m672DLL/scene_child.h"
#include "REL/m672DLL/scene_thread_owner.h"
#include "REL/m672DLL/camera_owner.h"
#include "REL/m672DLL/singleton_callback_owner.h"
#include "REL/framework/event.h"
#include "REL/framework/linknode.h"
#include "REL/framework/phase_durations.h"
extern "C" {
#include <math.h>
#include "game/audio.h"
#include "msm_stream.h"
#include "game/mg/actman.h"
}
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "REL/m672DLL/member_callback.h"
#include "REL/framework/event_hm.h"
#include "REL/framework/thread.h"
#include "REL/m672DLL/ground_sprite.h"
#include "REL/m672DLL/collision_object.h"
#include "REL/framework/game_object.h"
#include "REL/m672DLL/resources.h"
#include "REL/m672DLL/scene_sequence.h"
#include "REL/m672DLL/player.h"

// Deletes each model pointer in the vector and then clears the vector.
void fn_1_749C(std::vector<OwnedModel *> *models)
{
    OwnedModel **first = models->begin();
    OwnedModel **last = models->end();
    while (first != last) delete *first++;
    models->clear();
}

// Initializes the core/thread/hudTimer objects, seeds randomness, and clears collision shapes.
SceneOwner::SceneOwner()
    : MinigameCore(), ThreadEntry(0x10000), hudTimer(new TimerOwner(0)),
      child(new SceneChild)
{
    RandomSeed(OSGetTick());
    collisionShapes.clear();
}

// Performs no additional work when the scene owner is destroyed.
SceneOwner::~SceneOwner() {}

extern "C" {

extern u8 fn_1_7BDC(SceneOwner *);

extern s32 fn_1_7EE8(void *);
extern u8 fn_1_7E10(void *);

}

// Creates the camera, shared resources, players, spinners, callbacks, and resource objects.
void SceneOwner::setupObjects(OMOBJMAN *)
{
    new CameraOwner;

    Resources *state = &Resources::getChecked();
    fn_1_6E04(state);
    fn_1_71BC(state);
    fn_1_7230(state);
    fn_1_7404(state);
    fn_1_7408(state);

    new SingletonCallbackOwner;
}

// Resumes the core, processes queued events, updates game objects, and advances the engine frame.
void SceneOwner::updateGame() {
    resume(0);
    process_event_queue();
    updateGameObjects();
    fn_800856A8();
}

// Runs the minigame hudTimer, waits on phase conditions, plays result audio, and requests exit.
void SceneOwner::run(void *)
{
    s32 musicHandle;
    s32 waitIndex;
    s32 transitionIndex;
    s32 resultCode;
    s32 jingle;
    s32 tailWaitIndex;
    s32 index;

    MinigameCore::wipeIn();
    musicHandle = HuAudBGMPlay(MSM_STREAM_GAMING_WITH_BOWSER);
    sendEvent(0, 0, 0.0f);
    waitIndex = 0;
    while ((f64)waitIndex < 180.0) {
        ThreadEntry::suspend(0);
        waitIndex++;
    }
    sendEvent(1, 1, 0.0f);
    transitionIndex = 0;
    while ((f64)transitionIndex < 30.0) {
        ThreadEntry::suspend(0);
        transitionIndex++;
    }
    MinigameCore::showStart();
    sendEvent(0, 1, 0.0f);
    while (MinigameCore::messageDone() == 0) {
        ThreadEntry::suspend(0);
    }
    sendEvent(0, 2, 0.0f);
    hudTimer->setParameters(0x708, 0, 0);
    hudTimer->setPosition(50.0f, 63.0f);
    hudTimer->setMode(1);
    while (fn_1_7BDC(this) == 0) {
        ThreadEntry::suspend(0);
    }
    HuAudSStreamFadeOut(musicHandle, 0x64);
    sendEvent(0, 3, 0.0f);
    hudTimer->start();
    MinigameCore::showFinish();
    resultCode = fn_1_7EE8(this);
    tailWaitIndex = 0;
    while (tailWaitIndex < 0x78) {
        ThreadEntry::suspend(0);
        tailWaitIndex++;
    }
    while (fn_1_7E10(this) == 0) {
        ThreadEntry::suspend(0);
    }
    if (resultCode != 4) {
        jingle = 0x43;
    } else {
        jingle = 0x45;
    }
    HuAudJinglePlay((s16)jingle);
    sendEvent(0, 4, &resultCode, 0.0f);
    index = 0;
    while ((f64)index < 180.0) {
        ThreadEntry::suspend(0);
        index++;
    }
    MinigameCore::requestExit();
    for (;;) {
        ThreadEntry::suspend((void *)-1);
    }
}

// Returns one when the hudTimer is done or all collected players are active; otherwise zero.
u8 fn_1_7BDC(SceneOwner *object)
{
    if (object->hudTimer->isDone()) {
        return 1;
    }

    std::vector<GameObject *> list;
    collectGameObjects(1, &list);
    int loopLimit = list.size();
    unsigned char result = 1;
    for (int index = 0; index < loopLimit; index++) {
        Player *entryObject = static_cast<Player *>(list[index]);
        if (!entryObject->active) {
            result = 0;
        }
    }
    if (result == 1) {
        return 1;
    }
    return 0;
}

// Empty scene-owner finish hook.
void SceneOwner::finishGame()
{
}

// Returns one when every collected player is finished; otherwise returns zero.
u8 fn_1_7E10(void *)
{
    std::vector<GameObject *> list;
    collectGameObjects(1, &list);
    int count = list.size();
    for (int index = 0; index < count; index++) {
        Player *player = static_cast<Player *>(list[index]);
        if (!player->finished) {
            return 0;
        }
    }
    return 1;
}

// Counts and returns collected players whose active flag is clear.
s32 fn_1_7EE8(void *)
{
    std::vector<GameObject *> list;
    collectGameObjects(1, &list);
    int count = list.size();
    int inactiveCount = 0;
    for (int index = 0; index < count; index++) {
        Player *player = static_cast<Player *>(list[index]);
        if (!player->active) {
            inactiveCount++;
        }
    }
    return inactiveCount;
}
