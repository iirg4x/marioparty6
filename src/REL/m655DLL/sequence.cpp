extern "C" {
#include "game/audio.h"
}
#include <new>
#include "REL/m655DLL/screen_sprite_work.h"
#include "REL/m655DLL/module.h"
#include "REL/m655DLL/player_registry.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/sound.h"
#include "msm_stream.h"
#include "REL/framework/event_manager.h"
#include "REL/m655DLL/screen_sprite_singleton.h"
#include "REL/framework/minigame_core.h"
#include "dolphin/types.h"
#include "REL/m655DLL/camera_work.h"

#include "REL/m655DLL/board_view.h"

#include "REL/m655DLL/goal.h"
#include "REL/framework/game_object_man.h"
#include "REL/m655DLL/effect_layer.h"

void RandomSeed(unsigned long seed);
void process_event_queue();
void updateGameObjects();
void sendEvent(int channel, unsigned long value, float delay);
#include "REL/m655DLL/target_effect.h"

extern "C" {
#include "game/wipe.h"
}

#include "REL/framework/frame_durations.h"
#include "REL/m655DLL/game_constants.h"

BoardView::BoardView() : ThreadEntry(0x8000), nested(new EffectLayer)
{

}

BoardView::~BoardView() {}

void fn_1_7BF8(void);
void fn_1_79E8(void);

void fn_1_2990(BoardView *);
void fn_1_ADE4(void);
void fn_1_39C4(void);

void BoardView::setupObjects(OMOBJMAN *)
{

    RandomSeed(OSGetTick());
    nested->goal.fn_1_31AC();
    new CameraWork;
    new ScreenSpriteWork;
    fn_1_7798();
    fn_1_ADE4();
    fn_1_39C4();
    registerChannel(0);
    soundSetCameraLayout(0);
    soundSetVolumeCurve(0x20, 0x6E, 1500.0f, 500.0f);
}

void fn_1_27E4(void)
{
}

void BoardView::updateGame()
{
    this->resume(0);
    process_event_queue();
    updateGameObjects();
    updateRequestObjects();
}

void BoardView::run(void *)
{
    int stream;
    int frame;

    stream = HuAudBGMPlay(MSM_STREAM_TIME_TO_DUEL);
    MinigameCore::wipeIn();
    while (WipeCheckIn() != 0) {
        suspend(0);
    }
    postRequest(0);
    while (requestsCleared() == 0) {
        suspend(0);
    }
    postRequest(1);
    fn_1_78D4();
    MinigameCore::showStart();
    while (MinigameCore::messageDone() == 0) {
        suspend(0);
    }
    fn_1_2990(this);
    sendEvent(0, 6, 0.0f);
    postRequest(3);
    MinigameCore::showFinish();
    HuAudSStreamFadeOut(stream, 0x64);
    for (frame = 0; frame < secondsToFrames<long>(3); frame++) {
        suspend(0);
    }
    fn_1_7BF8();
    postRequest(4);
    MinigameCore::showWinners();
    HuAudJinglePlay((fn_1_7CA8() > 0) ? 0x43 : 0x45);
    while (MinigameCore::messageDone() == 0) {
        suspend(0);
    }
    MinigameCore::requestExit();
    for (;;) {
        suspend((void *)-1);
    }
}

/* Runs the board-result sequence and waits for its menus and motion state. */
void fn_1_2990(BoardView *owner)
{

    TimerOwner *motion;
    int frame;
    int voice;
    int move;

    motion = &owner->nested->state;
    postRequest(2);
    fn_1_79E8();
    motion->setParameters(timeLimit, 0, 0);
    motion->setMode(1);

    while (owner->nested->active == 0 && motion->isDone() == 0) {
        owner->suspend(0);
    }

    motion->start();
    sendEvent(0, 2, 0.0f);
    while ((u8)fn_1_7AC4() == 0) {
        owner->suspend(0);
    }
    for (frame = 0; frame < secondsToFrames<long>(2); ++frame) {
        owner->suspend(0);
    }

    sendEvent(0, 3, 0.0f);
    for (voice = 0; voice < 3; ++voice) {
        sendEvent(voice + 1, 3, 2.5f);
    }
    while ((u8)fn_1_7AC4() == 0) {
        owner->suspend(0);
    }
    if (fn_1_7CA8() > 0) {
        sendEvent(0, 5, 2.0f);
        return;
    }

    for (move = 0; move < secondsToFrames<long>(2); ++move) {
        owner->suspend(0);
    }
    new TargetEffect();
    while (fn_1_7CA8() <= 0 || (u8)fn_1_7AC4() == 0) {
        owner->suspend(0);
    }
}

void BoardView::receive(const EventPacket *requestData)
{
    if (requestData->type == 1) {
        nested->active = 1;
    }
}

void BoardView::finishGame()
{
    deleteRequestObjects();
}
