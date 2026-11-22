// Creates the scene and runs the minigame sequence.

#include "REL/m673DLL/moving_actor.h"
#include "REL/framework/phase_duration_statics.h"
#include "REL/framework/frame_time.h"
#include "dolphin/types.h"
extern "C" {
#include "dolphin/os.h"
#include "game/audio.h"
#include "msm_stream.h"
}
#include "dolphin/mtx.h"
#include "REL/m673DLL/particle_game_object.h"
#include "REL/framework/model.h"
#include "REL/framework/sound.h"
#include "REL/framework/random.h"
#include "REL/framework/timer.h"
#include "REL/m673DLL/minigame_thread.h"
#include "REL/m673DLL/scene_child.h"
#include "REL/m673DLL/position_table.h"
#include "REL/m673DLL/scene_camera.h"
#include "REL/m673DLL/player_actor.h"
#include "REL/framework/event.h"
#include <new>

// Creates a particle effect for a position event.
void fn_1_6800(SceneResources *resources, const EventPacket *event)
{
    switch (event->type) {
    case 0: {
        Vec position = *static_cast<Vec *>(event->context);
        new ParticleGameObject(*resources->animations.begin(), &position);
        break;
    }
    }
}

// Deletes the owned models and clears their list.
void fn_1_6898(std::vector<OwnedModel *> *models)
{
    OwnedModel **first = models->begin();
    OwnedModel **last = models->end();
    while (first != last) {
        delete *first++;
    }
    models->clear();
}

// Creates the game timer and child scene managers.
MinigameThread::MinigameThread()
    : ThreadEntry(0x10000), hudTimer(new TimerOwner(0)), child(new SceneChild())
{
    RandomSeed(OSGetTick());
    // Sets the minimum and maximum stereo pan values.
    soundSetCameraPan(1, 0x20, 0x60, 1.2f);
}

// Destroys the game timer and child scene managers.
MinigameThread::~MinigameThread()
{
}

// Creates the camera, scene objects, player actors, and position table.
void MinigameThread::setupObjects(OMOBJMAN *)
{
    new SceneCamera();

    SceneResources *resources = &SceneResources::getChecked();
    fn_1_6104(resources);
    fn_1_6638(resources);
    fn_1_66AC(resources);
    fn_1_677C(resources);
    fn_1_6780(resources);

    new ScenePositionTable();
}

// Resumes the thread and advances scene events, objects, and collision checks.
void MinigameThread::updateGame()
{
    ThreadEntry::resume(0);
    process_event_queue();
    updateGameObjects();
    fn_800856A8();
    fn_1_368();
}

// Runs the minigame's opening, play, result, and closing phases.
void MinigameThread::run(void *initialArgument)
{
    s32 musicHandle;
    s32 waitIndex;
    s32 transitionIndex;
    s32 setupWaitIndex;
    s32 resultCode;
    s32 jingle;
    s32 tailWaitIndex;
    s32 index;

    MinigameCore::wipeIn();
    musicHandle = HuAudBGMPlay(MSM_STREAM_GAMING_WITH_BOWSER);
    sendEvent(0, 0, 0.0f);
    waitIndex = 0;
    while ((f64)waitIndex < 120.0) {
        ThreadEntry::suspend(0);
        waitIndex++;
    }
    sendEvent(1, 5, 0.0f);
    transitionIndex = 0;
    while ((f64)transitionIndex < 60.0) {
        ThreadEntry::suspend(0);
        transitionIndex++;
    }
    sendEvent(1, 8, 0.0f);
    setupWaitIndex = 0;
    while ((f64)setupWaitIndex < 30.0) {
        ThreadEntry::suspend(0);
        setupWaitIndex++;
    }
    MinigameCore::showStart();
    sendEvent(0, 1, 0.0f);
    while (MinigameCore::messageDone() == 0) {
        ThreadEntry::suspend(0);
    }
    sendEvent(0, 2, 0.0f);
    hudTimer->setParameters(1800, 0, 0);
    hudTimer->setPosition(526.0f, 63.0f);
    hudTimer->setMode(1);
    while (fn_1_7080(this) == 0) {
        ThreadEntry::suspend(0);
    }
    HuAudSStreamFadeOut(musicHandle, 100);
    sendEvent(0, 4, 0.0f);
    sendEvent(1, 6, 0.0f);
    sendEvent(1, 7, 0.0f);
    hudTimer->start();
    MinigameCore::showFinish();
    resultCode = fn_1_74A4(this);
    tailWaitIndex = 0;
    while (tailWaitIndex < 120) {
        ThreadEntry::suspend(0);
        tailWaitIndex++;
    }
    while (fn_1_731C(this) == 0) {
        ThreadEntry::suspend(0);
    }
    if (resultCode != 4) {
        jingle = MSM_STREAM_WINNING;
    } else {
        jingle = MSM_STREAM_BAD_NEWS;
    }
    HuAudJinglePlay(jingle);
    sendEvent(0, 5, &resultCode, 0.0f);
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

// Accumulates elapsed time and polls the HUD timer. Play ends when the timer expires
// or four players are inactive.
u8 fn_1_7080(MinigameThread *owner)
{
    s32 loopLimit;
    s32 inactiveCount;
    s32 index;

    static f32 elapsed = 0.0f;
    elapsed += 1.0 / 60.0;
    if (owner->hudTimer->isDone()) {
        return 1;
    }

    std::vector<GameObject *> players;
    collectGameObjects(1, &players);
    loopLimit = players.size();
    inactiveCount = 0;
    for (index = 0; index < loopLimit; index++) {
        PlayerActor *player;
        player = static_cast<PlayerActor *>(players[index]);
        if (player->active == 0) {
            inactiveCount++;
        }
    }

    if (inactiveCount == 4) {
        return 1;
    }

    return 0;
}

// Leaves the scene unchanged at the end of the game.
void MinigameThread::finishGame()
{
}

// Returns whether all players and the moving actor have finished their current phase.
u8 fn_1_731C(MinigameThread *)
{
    std::vector<GameObject *> players;
    int playerCount;
    PlayerActor *player;
    MovingActor *actor;
    int playerIndex;

    collectGameObjects(1, &players);
    playerCount = players.size();
    for (playerIndex = 0; playerIndex < playerCount; playerIndex++) {
        player = static_cast<PlayerActor *>(players[playerIndex]);
        if (!player->ready) {
            return 0;
        }
    }
    std::vector<GameObject *> actors;
    collectGameObjects(2, &actors);
    actor = static_cast<MovingActor *>(actors[0]);
    if (!actor->ready) {
        return 0;
    }
    return 1;
}

// Counts players who are out of play.
s32 fn_1_74A4(MinigameThread *)
{
    std::vector<GameObject *> players;
    s32 loopLimit;
    s32 inactiveCount;
    s32 index;

    collectGameObjects(1, &players);
    loopLimit = players.size();
    inactiveCount = 0;
    for (index = 0; index < loopLimit; index++) {
        PlayerActor *player;
        player = static_cast<PlayerActor *>(players[index]);
        if (player->active == 0) {
            inactiveCount++;
        }
    }
    return inactiveCount;
}
