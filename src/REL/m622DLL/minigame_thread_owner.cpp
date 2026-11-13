#include <cmath>
#include "REL/framework/sound.h"
#include "REL/framework/phase_durations.h"
#include "REL/framework/random.h"
#include "types.h"
#include "msm_stream.h"
#include "REL/framework/thread.h"
#include "REL/framework/minigame_core.h"
#include "REL/framework/timer.h"
#include "REL/framework/list_object.h"

#include "REL/m622DLL/minigame_owner.h"
#include "REL/m622DLL/callback_object.h"
#include "REL/m622DLL/camera_owner.h"
#include "REL/m622DLL/model_setup_owner.h"
#include "REL/m622DLL/singleton_callback_owner.h"
#include "REL/m622DLL/scene_owner.h"
#include "REL/m622DLL/texture_capture_owner.h"
#include "REL/m622DLL/score_display_owner.h"

extern "C" {
#include "game/audio.h"
#include "game/gamework.h"
#include "game/charman.h"
}

/* Initializes the object through its member and base constructors. */
MinigameThreadOwner::MinigameThreadOwner()
    : MinigameCore(), ThreadEntry(0x8000), timerState(0)
{
}

/* Releases the object through its member and base destructors. */
MinigameThreadOwner::~MinigameThreadOwner()
{
}

/* Seeds randomness and creates the camera, stage, displays and four players. */
void MinigameThreadOwner::setupObjects(OMOBJMAN *)
{
    s32 index;

    RandomSeed(OSGetTick());
    this->dayMode = !GWMgNightFGet();

    new CameraOwner();
    new ScoreDisplayOwner();
    new TextureCaptureOwner();
    new SceneOwner();
    fn_8005DDEC(3);
    for (index = 0; index < 4; index++) {
        new CallbackObject(index);
    }
    new ModelSetupOwner();
}

/* Resumes the game thread, updates request objects and advances the players. */
void MinigameThreadOwner::updateGame()
{
    this->resume(0);
    updateRequestObjects();
    fn_1_6FB0();
}

/* Runs the introduction, timed play and result sequence with music and requests. */
void MinigameThreadOwner::run(void *)
{
    s32 stream;
    u8 musicMode;
    s32 elapsedFrames;
    s32 jingleId;

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

/* Returns whether the game timer has completed. */
u8 MinigameThreadOwner::fn_1_418()
{
    return timerState.isDone();
}

/* Returns without changing state. */
void MinigameThreadOwner::fn_1_444()
{
}

/* Deletes the players and all request objects. */
void MinigameThreadOwner::finishGame()
{
    fn_1_67F8();
    deleteRequestObjects();
}
