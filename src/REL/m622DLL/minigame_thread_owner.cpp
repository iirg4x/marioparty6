#include <cmath>
#include "REL/framework/sound.h"
#include "REL/framework/phase_durations.h"
#include "REL/m622DLL/random.h"
#include "types.h"
#include "msm_stream.h"
#include "REL/framework/thread.h"
#include "REL/framework/minigame_core.h"
#include "REL/m622DLL/timer_owner.h"
#include "REL/framework/list_object.h"

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
    updateRequestObjects();
    fn_1_6FB0();
}

extern "C" {
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

    soundSetCameraPan(1, 0x30, 0x50, 1.2f);
    postRequest(0);
    MinigameCore::wipeIn();
    while (requestsCleared() == 0) {
        suspend(0);
    }

    stream = HuAudBGMPlay(MSM_STREAM_SLOW_AND_STEADY);
    postRequest(1);
    MinigameCore::showStart();
    while (MinigameCore::messageDone() == 0) {
        suspend(0);
    }

    postRequest(2);
    timerState.setParameters(0x708, 0, 0);
    timerState.setMode(1);
    while (fn_1_418() == 0) {
        suspend(0);
    }
    timerState.start();
    HuAudSStreamFadeOut(stream, 100);
    fn_1_7054();
    postRequest(3);
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
    deleteRequestObjects();
}
