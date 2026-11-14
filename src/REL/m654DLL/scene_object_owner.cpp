#include <new>
#include "REL/framework/frame_time.h"
#include "REL/framework/sound.h"
#include "REL/framework/world_units.h"
#include "REL/framework/minigame_core.h"
#include "REL/framework/event_manager.h"
#include "REL/m654DLL/camera_node_owner.h"
#include "REL/framework/timer.h"
#include "REL/framework/list_object.h"
#include "REL/m654DLL/model_animation_owner.h"
#include "REL/m654DLL/scene_owner.h"
#include "REL/framework/thread.h"
#include "REL/framework/random.h"
#include "REL/framework/event.h"
extern "C" {
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
}
#include "REL/m654DLL.h"

extern int lbl_1_bss_1C;

// Creates the scene thread with a 0x8000-byte stack and its event/timer owner.
SceneObjectOwner::SceneObjectOwner() : ThreadEntry(0x8000), childOwner(new OwnedSceneChild) {}

// Deletes the owned event manager and timer.
SceneChildOwner::~SceneChildOwner()
{
    delete child;
}

// Releases the scene child owner and scene base objects.
SceneObjectOwner::~SceneObjectOwner() {}

// Seeds randomness and creates the camera, scene models and players.
void SceneObjectOwner::setupObjects(OMOBJMAN *)
{
    RandomSeed(OSGetTick());
    new CameraManager;
    new ModelAnimationOwner;
    fn_1_3274();
    soundSetCameraLayout(0);
}
// Resumes the scene thread and updates queued events, requests, objects and actors.
void SceneObjectOwner::updateGame()
{
    resume(0);
    process_event_queue();
    updateRequestObjects();
    updateGameObjects();
    fn_800856A8();
}
// Sequences the entrance, timed round, finish walk, results and exit request.
void SceneObjectOwner::run(void *argument)
{
    int bgmHandle;
    int frame;
    OwnedSceneChild *manager = childOwner.child;
    TimerOwner *timer = &manager->timerOwner;

    MinigameCore::wipeIn();
    while (WipeCheckIn() != 0) {
        suspend(0);
    }
    bgmHandle = HuAudBGMPlay(MSM_STREAM_TIME_TO_DUEL);
    fn_1_3398(0);
    while ((unsigned char)fn_1_3528() == 0) {
        suspend(0);
    }
    postRequest(0);
    while (requestsCleared() == 0) {
        suspend(0);
    }
    postRequest(1);
    while (requestsCleared() == 0) {
        suspend(0);
    }
    fn_1_3398(1);
    while ((unsigned char)fn_1_3528() == 0) {
        suspend(0);
    }
    postRequest(2);
    MinigameCore::showStart();
    while ((unsigned char)MinigameCore::messageDone() == 0) {
        suspend(0);
    }
    postRequest(3);
    fn_1_3494();
    timer->setPosition(288.0f, 416.0f);
    timer->setParameters(lbl_1_bss_1C, 0, 0);
    timer->setMode(1);
    while (fn_1_10DC() == 0) {
        suspend(0);
    }
    timer->start();
    postRequest(4);
    fn_1_35F8();
    MinigameCore::showFinish();
    HuAudSStreamFadeOut(bgmHandle, 0x64);
    for (frame = 0; frame < secondsToFrames<long>(3); frame++) {
        suspend(0);
    }
    while ((unsigned char)fn_1_3528() == 0) {
        suspend(0);
    }
    fn_1_3768();
    postRequest(5);
    MinigameCore::showWinners();
    HuAudJinglePlay((short)(fn_1_37FC() > 0 ? 0x43 : 0x45));
    while ((unsigned char)MinigameCore::messageDone() == 0) {
        suspend(0);
    }
    MinigameCore::requestExit();
    for (;;) {
        suspend(reinterpret_cast<void *>(-1));
    }
}
// Reports whether a player has won or the round timer has expired.
unsigned char SceneObjectOwner::fn_1_10DC()
{
    OwnedSceneChild *manager;
    if (fn_1_37FC() > 0) {
        return 1;
    }
    manager = childOwner.child;
    return manager->timerOwner.isDone();
}
// Deletes the framework request objects.
void SceneObjectOwner::finishGame()
{
    deleteRequestObjects();
}
#include "REL/framework/phase_durations.h"
// One-minute time limit and gravity in world units per frame squared.
int lbl_1_bss_1C = minutesToFrames(1L);
float lbl_1_bss_20 = 0.016666666666666666 * (0.016666666666666666 * cmToWorld(980.665f));
