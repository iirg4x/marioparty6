#include <cmath>
#include "REL/m622DLL/frame_time.h"
#include "REL/m622DLL/random.h"
#include "types.h"
#include "msm_stream.h"
#include "REL/framework/thread.h"
#include "REL/framework/minigame_core.h"
#include "REL/m622DLL/timer_owner.h"
#include "REL/m622DLL/root_list.h"

#include "REL/m622DLL/minigame_owner.h"
#include "REL/m622DLL/camera_owner.h"
#include "REL/m622DLL/model_setup_owner.h"
#include "REL/m622DLL/singleton_callback_owner.h"
#include "REL/m622DLL/scene_owner.h"
#include "REL/m622DLL/texture_capture_owner.h"
#include "REL/m622DLL/score_display_owner.h"

extern "C" {
extern s16 lbl_802C0278;
void fn_8005DDEC(s32);
}

MinigameThreadOwner::MinigameThreadOwner()
    : MinigameCore(), ThreadEntry(0x8000), timerState(0)
{
}

MinigameThreadOwner::~MinigameThreadOwner()
{
}

void MinigameThreadOwner::setupObjects(OMOBJMAN *)
{
    s32 index;
    s16 nightMode;

    RandomSeed(OSGetTick());
    nightMode = lbl_802C0278;
    this->dayMode = !nightMode;

    new CameraOwner();
    new ScoreDisplayOwner();
    new TextureCaptureOwner();
    new SceneOwnerB9C();
    fn_8005DDEC(3);
    for (index = 0; index < 4; index++) {
        new CallbackObject(index);
    }
    new ModelSetupOwner();
}

void fn_1_6FB0(void);

void MinigameThreadOwner::updateGame()
{
    this->resume(0);
    fn_1_E46C();
    fn_1_6FB0();
}

extern "C" {
void fn_1_A36C(s32 mode, s32 width, s32 height, f32 scale);
s32 HuAudBGMPlay(s32 songId);
void HuAudSStreamFadeOut(s32 stream, s32 duration);
}
void fn_1_7054(void);
u8 fn_1_7138(void);
extern "C" {
void HuAudJinglePlay(s32 jingleId);

}

void MinigameThreadOwner::run(void *)
{
    s32 stream;
    u8 musicMode;
    s32 elapsedFrames;
    s16 jingleId;

    fn_1_A36C(1, 0x30, 0x50, 1.2f);
    fn_1_E348(0);
    MinigameCore::wipeIn();
    while (fn_1_E3E0() == 0) {
        suspend(0);
    }

    stream = HuAudBGMPlay(MSM_STREAM_SLOW_AND_STEADY);
    fn_1_E348(1);
    MinigameCore::showStart();
    while (MinigameCore::messageDone() == 0) {
        suspend(0);
    }

    fn_1_E348(2);
    timerState.setParameters(0x708, 0, 0);
    timerState.setMode(1);
    while (fn_1_418() == 0) {
        suspend(0);
    }
    timerState.start();
    HuAudSStreamFadeOut(stream, 100);
    fn_1_7054();
    fn_1_E348(3);
    MinigameCore::showFinish();
    while (MinigameCore::messageDone() == 0) {
        suspend(0);
    }

    musicMode = fn_1_7138();
    if (musicMode != 0) {
        jingleId = MSM_STREAM_BAD_NEWS;
    } else {
        jingleId = MSM_STREAM_WINNING;
    }
    HuAudJinglePlay(jingleId);

    elapsedFrames = 0;
    while ((f32)(elapsedFrames += 1) < 240.0f) {
        suspend(0);
    }
    MinigameCore::requestExit();
    for (;;) {
        suspend((void *)-1);
    }
}

u8 MinigameThreadOwner::fn_1_418()
{
    return timerState.isDone();
}

void fn_1_67F8(void);

void MinigameThreadOwner::fn_1_444()
{
}

void MinigameThreadOwner::finishGame()
{
    fn_1_67F8();
    fn_1_E4DC();
}

// Shared zero-initialized module state.
extern "C" {
s32 lbl_1_bss_0 = secondsToFrames(2L);
s32 lbl_1_bss_4 = secondsToFrames(3.5);
s32 lbl_1_bss_8 = secondsToFrames(3.5);
}
