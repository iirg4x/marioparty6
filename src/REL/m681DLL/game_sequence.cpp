#include "msm_stream.h"
#include "REL/m681DLL.h"
#include "REL/framework/frame_time.h"
#include "dolphin/types.h"

#include "dolphin/os.h"

#include "math.h"

#include "dolphin/gx.h"

#include "REL/m681DLL/game_sequence.h"
#include "REL/m681DLL/board_setup.h"
#include "REL/framework/game_object_man.h"

#include "REL/framework/thread.h"

#include "REL/m681DLL/global_state.h"

extern "C" {
#include "game/audio.h"
}

extern "C" {
#include "game/wipe.h"
}

#include "game/main.h"

#include "game/object.h"

#include "game/charman.h"

#include "game/gamemes.h"

#include "game/hsfex.h"

#include "game/data.h"

#include "game/gamework.h"

#include "game/memory.h"

#include "game/mg/seqman.h"

#include "game/mg/timer.h"

#include "game/mg/score.h"

#include "game/pad.h"

#include "game/frand.h"

#include "game/sprite.h"

#include "game/mg/actman.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"
#include "REL/framework/world_units.h"

#include "datadir_enum.h"

#include "REL/m681DLL/event_manager.h"

struct EffectLayer {
    EventManager events;
    GameObjectManager layer;
    BoardController state;
};

void *operator new(unsigned long size);

GameSequenceController::GameSequenceController()
    : ThreadEntry(0x8000), eventReceived(0), ownedObject(new EffectLayer)
{
    registerChannel(0);
}

GameSequenceController::~GameSequenceController() {}

extern "C" { void __msl_assertion_failed(const char *, const char *, int); }

void RandomSeed(unsigned long);

extern "C" {}
 void fn_1_EEC(BoardController *);
extern "C" { }

extern "C" {}
 void fn_1_F30(BoardController *);
extern "C" { }

extern "C" {}
 void fn_1_1014(BoardController *);
extern "C" { }

extern "C" {}
 void fn_1_10D8(BoardController *);
extern "C" { }

void GameSequenceController::setupObjects(OMOBJMAN *)
{
    RandomSeed(OSGetTick());
    BoardController *controller = &MinigameSingleton<BoardController>::getChecked();
    fn_1_EEC(controller);
    fn_1_F30(controller);
    fn_1_1014(controller);
    fn_1_10D8(controller);
}

void process_event_queue();

void updateGameObjects();

void GameSequenceController::updateGame()
{
    resume(0);
    process_event_queue();
    updateGameObjects();
}

void sendEvent(int, unsigned long, float);

void GameSequenceController::run(void *)
{
    s32 audioHandle, frame, firstWait, secondWait, thirdWait, cueCount;
    MinigameCore::wipeIn();
    while (WipeCheckIn() != 0) suspend(0);
    sendEvent(0, 0, 0.0f);
    sendEvent(0, 1, 2.4f);
    sendEvent(0, 2, 4.8f);
    sendEvent(0, 3, 7.2000003f);
    frame = 0;
    while ((f64)frame < secondsToFrames(8.200000286102295)) { suspend(0); ++frame; }
    audioHandle = HuAudBGMPlay(MSM_STREAM_FAST_AND_FURIOUS);
    sendEvent(0, 4, 0.0f);
    MinigameCore::showStart();
    while (MinigameCore::messageDone() == 0) suspend(0);
    sendEvent(0, 5, 0.0f);
    while (fn_1_1744(this) == 0) suspend(0);
    firstWait = 0;
    while (firstWait < secondsToFrames(3L)) { suspend(0); ++firstWait; }
    MinigameCore::showFinish();
    while (MinigameCore::messageDone() == 0) suspend(0);
    sendEvent(0, 0xB, 0.0f);
    HuAudSStreamFadeOut(audioHandle, 0x64);
    secondWait = 0;
    while (secondWait < secondsToFrames(3L)) { suspend(0); ++secondWait; }
    sendEvent(0, 0xC, 0.0f);
    cueCount = MinigameSingleton<M681GameState>::getChecked().minimum;
    HuAudJinglePlay((s16)(cueCount > 0 ? 0x43 : 0x45));
    thirdWait = 0;
    while (thirdWait < secondsToFrames(3L)) { suspend(0); ++thirdWait; }
    MinigameCore::requestExit();
    for (;;) suspend((void *)-1);
}

extern "C" {

}
u8 fn_1_1744(GameSequenceController *owner) {
    if (owner->eventReceived == 1) {
        return 1;
    }
    return 0;
}
extern "C" {

}

void GameSequenceController::finishGame() {}

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

extern "C" {
s32 lbl_1_bss_28 = secondsToFrames(2.0);
s32 lbl_1_bss_2C = secondsToFrames(3.5);
s32 lbl_1_bss_30 = secondsToFrames(3.5);
s32 lbl_1_bss_34 = minutesToFrames(60L);
f32 lbl_1_bss_38 = 0.016666666666666666 * (0.016666666666666666 * cmToWorld(980.665f));
}
