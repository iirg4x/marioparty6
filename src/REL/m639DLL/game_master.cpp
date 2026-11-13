#include "REL/m639DLL/shadow_fit.h"
#include "REL/m639DLL/player.h"
#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
#include "REL/framework/minigame_core.h"
#include "REL/m639DLL/camera_owner.h"
#include "REL/m639DLL/scene_owner.h"
#include "REL/m639DLL/participant_owner.h"
#include "REL/m639DLL/mode_game.h"
#include "REL/m639DLL/mode_selection.h"
#include "REL/framework/timer.h"
#include "REL/framework/game_object_man.h"
#include "REL/framework/thread.h"
#include "REL/framework/sound.h"
#include "REL/framework/list_object.h"
#include "humath.h"

// These constants keep translation-unit-local initialized storage.
// Additional frame durations, kept in their initialized order.
#include "REL/framework/frame_durations.h"
// The two 3.5-second values have separate storage.
// Game durations include the closing interval.
#include "REL/m639DLL/game_constants.h"
// Gravity uses world units per frame squared.
// Frame and world-unit conversions are defined by the framework.
extern "C" {
#include "game/audio.h"
#include "game/wipe.h"
}
#include "REL/framework/random.h"
#include "REL/framework/event.h"

#include "REL/framework/event_manager.h"
#include "REL/m639DLL/scene_object_owner.h"

// The game state: its events, objects and timer, and the state of the players. The timer counts down
// from the game length less its closing seconds.
struct OwnedSceneChild {
    EventManager eventManager;
    GameObjectManager objectManager;
    TimerOwner timer;
    long timeLeft;
    M639StateOwner modeChoice;
    OwnedSceneChild() : timer(0), timeLeft(timeLimit - closingTime)
    {
        long timer_total = timeLeft;
        assert(timer_total > 0);
    }
};

SceneObjectOwner::SceneObjectOwner() : ThreadEntry(0x8000), childOwner(new OwnedSceneChild) {}
SceneChildOwner::~SceneChildOwner() { delete child; }
SceneObjectOwner::~SceneObjectOwner() {}
void SceneObjectOwner::setupObjects(OMOBJMAN *)
{
    Vec direction;

    RandomSeed(OSGetTick());
    new CameraOwner;
    new SceneOwner;
    fn_1_5F08();
    new ParticipantOwner;
    fn_1_C28C(SceneOwner::getChecked().fn_1_5AAC(&direction));
    soundSetCameraLayout(0);
    soundPlay(0x771);
}

void SceneObjectOwner::updateGame() {
    resume(0);
    process_event_queue();
    updateGameObjects();
    updateRequestObjects();
}

void SceneObjectOwner::run(void *initialArgument)
{
    s32 *modeCounter;
    TimerOwner *message;
    s32 music;
    s32 frame;
    u8 showMessage;
    s32 jingle;
    OwnedSceneChild *counterState;
    OwnedSceneChild *messageState;

    counterState = childOwner.child;
    modeCounter = &counterState->timeLeft;
    messageState = childOwner.child;
    message = &messageState->timer;

    MinigameCore::wipeIn();
    while (WipeCheckIn() != 0)
        suspend(0);

    postRequest(0);
    while (requestsCleared() == 0)
        suspend(0);

    music = HuAudBGMPlay(MSM_STREAM_LAID_BACK);
    postRequest(1);
    MinigameCore::showStart();
    while (MinigameCore::messageDone() == 0)
        suspend(0);

    postRequest(2);
    fn_1_6058();
    while ((u8)fn_1_112C() == 0) {
        showMessage = 0;
        if (*modeCounter > 0) {
            if (--*modeCounter == 0)
                showMessage = 1;
        }
        if (showMessage != 0) {
            message->setPosition(288.0f, 72.0f);
            message->setParameters(closingTime, 0, 0);
            message->setMode(1);
        }
        suspend(0);
    }

    sendEvent(0, 2, 0.0f);
    M639StateOwner::getChecked().fn_1_2CAC();
    if (*modeCounter <= 0)
        message->start();
    else
        message->reset();

    postRequest(3);
    fn_1_626C();
    MinigameCore::showFinish();
    HuAudSStreamFadeOut(music, 0x64);

    frame = 0;
    while (frame < secondsToFrames(3L) || requestsCleared() == 0) {
        suspend(0);
        ++frame;
    }

    fn_1_63A4();
    postRequest(4);
    MinigameCore::showWinners();
    if (M639StateOwner::getChecked().fn_1_2CAC() != -1)
        jingle = MSM_STREAM_WINNING;
    else
        jingle = MSM_STREAM_BAD_NEWS;
    HuAudJinglePlay(jingle);

    while (MinigameCore::messageDone() == 0)
        suspend(0);
    MinigameCore::requestExit();
    for (;;)
        suspend((void *)(-1));
}

/* The game ends when its timer finishes or either team becomes fully charged. */
s32 SceneObjectOwner::fn_1_112C()
{
    s32 *timeLeft;
    bool timerDone;
    bool teamFullyCharged;
    s32 result;
    bool timerFinished;
    bool chargeComplete;
    OwnedSceneChild *timeState;
    bool timeExpired;
    bool timeIsUp;
    OwnedSceneChild *timerState;

    timerDone = false;
    timeState = this->childOwner.child;
    timeLeft = &timeState->timeLeft;
    timeExpired = *timeLeft <= 0;
    timeIsUp = timeExpired;
    if (timeIsUp) {
        timerState = this->childOwner.child;
        if (timerState->timer.isDone() != 0) {
            timerDone = true;
        }
    }
    timerFinished = timerDone;
    teamFullyCharged = true;
    if (!ModeGame::getChecked().fn_1_3B0(0)) {
        if (!ModeGame::getChecked().fn_1_3B0(1)) {
            teamFullyCharged = false;
        }
    }
    chargeComplete = teamFullyCharged;
    result = 1;
    if (!timerFinished && !chargeComplete) {
        result = 0;
    }
    return result;
}

void SceneObjectOwner::finishGame() { deleteRequestObjects(); }
