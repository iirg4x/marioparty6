#include <new>
#include "REL/framework/phase_duration_statics.h"
#include "REL/framework/seconds_to_frames_inline.h"
#include "REL/framework/list_object.h"
#include "REL/framework/sound.h"
#include "REL/m611DLL/motion_light_owner.h"
#include "REL/m611DLL/sprite_manager.h"
#include "REL/m611DLL/animation_owner.h"
#include "REL/m611DLL/score_display.h"
#include "REL/m611DLL/texture_owner.h"
#include "REL/m611DLL/motion_effect_owner.h"
#include "REL/m611DLL/motion_candidate_table.h"
#include "REL/m611DLL/player_state.h"
#include "REL/m611DLL/motion_position_table.h"
#include "REL/m611DLL/minigame_thread_owner.h"
#include "REL/framework/random.h"
#include "REL/m611DLL/scene_camera.h"
#include "REL/framework/minigame_core.h"
extern "C" {
#include "dolphin/types.h"
#include "game/charman.h"
#include "game/audio.h"
#include "game/gamework.h"
#include "dolphin/os.h"
}

// Creates the minigame thread and its sequence timer.
MinigameThreadOwner::MinigameThreadOwner() : ThreadEntry(0x8000), timer(0) {}

// Releases the thread, timer, and minigame base.
MinigameThreadOwner::~MinigameThreadOwner()
{
}

// Creates the scene managers and four players using the day or night models.
void MinigameThreadOwner::setupObjects(OMOBJMAN *)
{
    s32 modelVariant;
    s32 index;
    RandomSeed(OSGetTick());
    dayMode = GWMgNightFGet() == 0;
    new SpriteManager;
    new AnimationOwner;
    new MotionLightOwner(dayMode);
    new SceneCamera;
    new ScoreDisplay;
    new MotionCandidateTable;
    new TextureOwner;
    new MotionPositionTable(dayMode);
    if (dayMode) {
        new MotionEffectOwner;
    }
    fn_8005DDEC(4);
    for (index = 0; index < 4; ++index) {
        modelVariant = (index * 2) + (GWMgNightFGet() != 0);
        new PlayerState(index, modelVariant, dayMode);
    }
}

// Resumes the game thread and updates requests and players.
void MinigameThreadOwner::updateGame()
{
    resume(0);
    updateRequestObjects();
    fn_1_CB18();
}

// Runs the minigame's intro, selection, and results audio sequence.
void MinigameThreadOwner::run(void *)
{
    s32 mainEffect;
    s32 dayEffect;
    s32 nightEffect;
    s32 music;
    s32 jingle;
    u8 playBadNewsJingle;

    soundSetCameraPan(1, 48, 80, 1.2f);
    soundSetVolumeCurve(64, 127, 10000.0f, 1119.0f);
    postRequest(0);
    MinigameCore::wipeIn();
    mainEffect = HuAudFXPlay(0x688);
    dayEffect = -1;
    nightEffect = -1;
    if (GWMgNightFGet() == 1) {
        nightEffect = HuAudFXPlay(0x697);
    } else {
        dayEffect = HuAudFXPlay(0x689);
    }
    while (!requestsCleared()) {
        suspend(0);
    }
    music = HuAudBGMPlay(MSM_STREAM_BLISSFUL);
    postRequest(1);
    MinigameCore::showStart();
    while (!MinigameCore::messageDone()) {
        suspend(0);
    }
    postRequest(2);
    timer.setParameters(secondsToFrames(45.0f), 0, 0);
    timer.setMode(1);
    timer.setPosition(286.0f, 416.0f);
    while (fn_1_1BFC() == 0) {
        suspend(0);
    }
    timer.start();
    HuAudSStreamFadeOut(music, 100);
    fn_1_CDEC();
    MinigameCore::showFinish();
    while (fn_1_CF20() || !MinigameCore::messageDone()) {
        suspend(0);
    }
    postRequest(3);
    fn_1_CBB0();
    postRequest(4);
    playBadNewsJingle = fn_1_D5D0();
    if (playBadNewsJingle) {
        jingle = MSM_STREAM_BAD_NEWS;
    } else {
        jingle = MSM_STREAM_WINNING;
    }
    HuAudJinglePlay(jingle);
    MinigameCore::showWinners();
    while (!MinigameCore::messageDone()) {
        suspend(0);
    }
    HuAudFXFadeOut(mainEffect, 100);
    if (dayEffect >= 0) {
        HuAudFXFadeOut(dayEffect, 100);
    }
    if (nightEffect >= 0) {
        HuAudFXFadeOut(nightEffect, 100);
    }
    MinigameCore::requestExit();
    for (;;) {
        suspend(reinterpret_cast<void *>(-1));
    }
}

// Reports whether the sequence timer is done.
u8 MinigameThreadOwner::fn_1_1BFC()
{
    return timer.isDone();
}

// This minigame callback has no work to do.
void MinigameThreadOwner::fn_1_1C28()
{
}

// Releases player resources and finalizes pending requests.
void MinigameThreadOwner::finishGame()
{
    fn_1_BD38();
    deleteRequestObjects();
}
