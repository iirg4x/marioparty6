#include "REL/m624DLL/callback_singleton.h"
#include "REL/framework/sound.h"
#include "REL/framework/list_object.h"
#include "msm_stream.h"
#include "REL/framework/phase_durations.h"
#include "REL/m624DLL/game_constants.h"

extern "C" {
#include "math.h"
#include "game/mg/actman.h"
}

#include "REL/framework/thread.h"
#include "REL/framework/timer.h"
#include "REL/framework/random.h"
#include "REL/m624DLL/player_base.h"

#include "REL/m624DLL/sequence_owner.h"
#include "REL/m624DLL/camera_transition.h"
#include "REL/m624DLL/attacker.h"
#include "REL/m624DLL/scene_sequence_contract.h"

extern "C" {
#include "game/audio.h"
#include "game/wipe.h"

}
extern void fn_1_60B8(void);

extern void fn_1_6214(void);

s32 fn_1_65A0();

u8 fn_1_3A08(SequenceOwner *);

SequenceOwner::SequenceOwner()
    : MinigameCore(), ThreadEntry(0x8000), timerState(0), auxiliaryTimer(0x1000)
{

}

SequenceOwner::~SequenceOwner()
{

}

void SequenceOwner::setupObjects(OMOBJMAN *)
{

    RandomSeed(OSGetTick());

    new RegisteredCallbackOwner(&this->auxiliaryTimer);

    new GameCamera;

    new SceneOwner;
    new Attacker;

    fn_1_60B8();
    soundSetCameraLayout(0);
}

void SequenceOwner::updateGame() {
    resume(0);
    fn_1_6214();
    updateRequestObjects();
    fn_800856A8();
}

void SequenceOwner::run(void *) {
    s32 bgmHandle;
    s32 jingleId;
    s32 elapsedFrames;

    MinigameCore::wipeIn();
    while (WipeCheckIn() != 0) {
        suspend(0);
    }
    bgmHandle = HuAudBGMPlay(MSM_STREAM_TIME_TO_DUEL);
    postRequest(0);
    fn_1_62C8();
    while (requestsCleared() == 0) {
        suspend(0);
    }
    postRequest(1);
    MinigameCore::showStart();
    while (MinigameCore::messageDone() == 0) {
        suspend(0);
    }
    postRequest(2);
    fn_1_635C();
    timerState.setParameters(timeLimit, 0, 0);
    timerState.setMode(1);
    while (fn_1_3A08(this) == 0) {
        suspend(0);
    }
    timerState.start();
    postRequest(3);
    fn_1_63F8();
    MinigameCore::showFinish();
    HuAudSStreamFadeOut(bgmHandle, 0x64);
    elapsedFrames = 0;
    while (elapsedFrames < secondsToFrames(3L)) {
        suspend(0);
        elapsedFrames += 1;
    }
    fn_1_6468();
    postRequest(4);
    MinigameCore::showWinners();
    if (fn_1_65A0() > 0) {
        jingleId = 0x43;
    } else {
        jingleId = 0x45;
    }
    HuAudJinglePlay((s16) jingleId);
    while (MinigameCore::messageDone() == 0) {
        suspend(0);
    }
    MinigameCore::requestExit();
    for (;;) {
        suspend((void *)-1);
    }
}

u8 fn_1_3A08(SequenceOwner *object)
{
    if (fn_1_65A0() >= 1) {
        fn_1_45E8(&GameCamera::getChecked());
        return 1;
    }
    return static_cast<SequenceOwner *>(object)->timerState.isDone();
}

extern "C" {
extern "C++" { void fn_1_64D8(); }
}

void SequenceOwner::finishGame()
{
    fn_1_64D8();
    deleteRequestObjects();
}
