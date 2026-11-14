#include "REL/m681DLL/game_controller.h"
#include <new>
#include "REL/m681DLL/game_sequence.h"
#include "REL/m681DLL/board_setup.h"
#include "REL/framework/random.h"
#include "REL/framework/event.h"
#include "REL/framework/game_object_man.h"
#include "REL/framework/event_manager.h"
#include "REL/framework/minigame_core.h"
#include "REL/framework/frame_time.h"
#include "dolphin/os.h"
extern "C" {
#include "game/audio.h"
}
extern "C" {
#include "game/wipe.h"
}
#include "msm_stream.h"
#include "REL/framework/phase_duration_statics.h"
#include "REL/m681DLL/game_constants.h"

struct GameManagers {
    EventManager events;
    GameObjectManager layer;
    BoardController state;
};

// Creates the event, object and board owners for the game thread.
GameSequenceController::GameSequenceController()
    : ThreadEntry(0x8000), eventReceived(0), ownedObject(new GameManagers)
{
    registerChannel(0);
}

// Releases the game thread and its owned managers.
GameSequenceController::~GameSequenceController() {}

// Seeds the random generator and creates the camera, models, players and monitor.
void GameSequenceController::setupObjects(OMOBJMAN *)
{
    RandomSeed(OSGetTick());
    BoardController *controller = &MinigameSingleton<BoardController>::getChecked();
    fn_1_EEC(controller);
    fn_1_F30(controller);
    fn_1_1014(controller);
    fn_1_10D8(controller);
}

// Resumes the game thread, dispatches events and updates managed objects.
void GameSequenceController::updateGame()
{
    resume(0);
    process_event_queue();
    updateGameObjects();
}

// Runs the introduction, rounds, finish sequence and minigame exit.
void GameSequenceController::run(void *)
{
    s32 audioHandle;
    s32 frame;
    s32 firstWait;
    s32 secondWait;
    s32 thirdWait;
    s32 playersRemaining;
    MinigameCore::wipeIn();
    while (WipeCheckIn() != 0) suspend(0);
    sendEvent(0, 0, 0.0f);
    sendEvent(0, 1, 2.4f);
    sendEvent(0, 2, 4.8f);
    sendEvent(0, 3, 7.2000003f);
    for (frame = 0; (f64)frame < secondsToFrames(8.200000286102295); ++frame) {
        suspend(0);
    }
    audioHandle = HuAudBGMPlay(MSM_STREAM_FAST_AND_FURIOUS);
    sendEvent(0, 4, 0.0f);
    MinigameCore::showStart();
    while (MinigameCore::messageDone() == 0) suspend(0);
    sendEvent(0, 5, 0.0f);
    while (fn_1_1744(this) == 0) suspend(0);
    for (firstWait = 0; firstWait < secondsToFrames(3L); ++firstWait) {
        suspend(0);
    }
    MinigameCore::showFinish();
    while (MinigameCore::messageDone() == 0) suspend(0);
    sendEvent(0, 0xB, 0.0f);
    HuAudSStreamFadeOut(audioHandle, 0x64);
    for (secondWait = 0; secondWait < secondsToFrames(3L); ++secondWait) {
        suspend(0);
    }
    sendEvent(0, 0xC, 0.0f);
    playersRemaining = MinigameSingleton<M681GameState>::getChecked().playersRemaining;
    HuAudJinglePlay((s16)(playersRemaining > 0 ? 0x43 : 0x45));
    for (thirdWait = 0; thirdWait < secondsToFrames(3L); ++thirdWait) {
        suspend(0);
    }
    MinigameCore::requestExit();
    for (;;) suspend((void *)-1);
}

// Reports whether the finish event has arrived.
u8 fn_1_1744(GameSequenceController *owner) {
    if (owner->eventReceived == 1) {
        return 1;
    }
    return 0;
}

// Performs no additional finish work.
void GameSequenceController::finishGame() {}

// Records receipt of the finish event.
void GameSequenceController::receive(const EventPacket *event)
{
    switch (event->type) {
    case 10:
        eventReceived = 1;
        break;
    default:
        break;
    }
}
