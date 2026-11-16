#include "REL/m634DLL/act_player.h"
#include "msm_stream.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/sound.h"
#include <vector>
#include "REL/framework/game_object_man.h"
extern "C" {
#include "game/charman.h"
}
void fn_1_D9B0(void);

#include "REL/m634DLL/scene_owner_vtable.h"
#include "REL/framework/random.h"
#include "REL/m634DLL/collision_shape.h"

// Initializes scene timing, random state, and sound camera settings.
SceneObjectOwner::SceneObjectOwner()
    : ThreadEntry(0x10000)
    , child(new SceneChild)
    , animation(0)
    , timer(new TimerOwner(0))
{
    RandomSeed(OSGetTick());
    lbl_1_bss_34.clear();
    mode = 0;
    frame = 0;
    selectedMode = -1;
    complete = 0;
    soundSetCameraPan(2, 0x20, 0x60, 1.2f);
    soundSetCameraPan(1, 0x20, 0x60, 1.2f);
}

// Releases the object and its member-owned resources.
SceneObjectOwner::~SceneObjectOwner() { }

#include "REL/framework/minigame_core.h"
#include "REL/m634DLL/event_calls.h"
#include "REL/m634DLL/random.h"
#include "REL/m634DLL/ActModels.h"
#include "REL/m634DLL/camera.h"
#include "REL/framework/thread.h"
#include "REL/m634DLL/game_manager.h"
#include "REL/m634DLL/scene.h"
#include <math.h>

#include "dolphin/gx.h"

#include "dolphin/types.h"

#include "game/main.h"

#include "game/object.h"

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

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "datadir_enum.h"

extern "C" {
#include "game/audio.h"
#include "game/wipe.h"
}

void *operator new(unsigned long size);

// Creates and initializes the game, camera, player, and scene objects.
void SceneObjectOwner::setupObjects(OMOBJMAN *managerObject)
{
    GameManager *manager;

    new CameraSceneWork();
    manager = &GameManager::getChecked();
    fn_1_C124(manager);
    fn_1_C534(manager);
    fn_1_C708(manager);
    manager->fn_1_C7F0();
    new Scene();
}

extern "C" void fn_800856A8(void);

void fn_1_50C8();

// Updates queued events, game objects, and active collisions.
void SceneObjectOwner::updateGame()
{
    ThreadEntry::resume(0);
    process_event_queue();
    updateGameObjects();
    fn_800856A8();
    if (this->complete != 1) {
        fn_1_50C8();
    }
}

// Returns the scene timer.
static inline TimerOwner *WorkTimer(SceneObjectOwner *work)
{
    return work->timer.get();
}
extern "C" {

extern s16 lbl_802C0278;
}

// Runs the scene opening, game timer, result, and closing sequence.
void SceneObjectOwner::run(void *initialArgument)
{
    s32 effectHandle;
    ActModels *animation = new ActModels;
    s32 musicHandle;
    s16 nightFlag;
    s32 initialWait;
    s32 effectWait;
    s32 modeWait;
    s32 closeWait;
    s32 openWait;
    s32 jingle;

    this->animation.reset(animation);
    sendEvent(1, 0, 0.0f);
    MinigameCore::wipeIn();
    for (initialWait = 0; (f32)initialWait < 30.0f; initialWait++) {
        ThreadEntry::suspend(0);
    }
    nightFlag = lbl_802C0278;
    if (nightFlag == 0) {
        effectHandle = HuAudFXPlay(0x741);
    } else {
        effectHandle = HuAudFXPlay(0x742);
    }
    for (effectWait = 0; (f32)effectWait < 150.0f; effectWait++) {
        ThreadEntry::suspend(0);
    }
    HuAudFXStop(effectHandle);
    musicHandle = HuAudBGMPlay(MSM_STREAM_JAZZY);
    sendEvent(1, 1, 0.0f);
    MinigameCore::showStart();
    while (MinigameCore::messageDone() == 0) {
        ThreadEntry::suspend(0);
    }
    sendEvent(1, 2, 0.0f);
    this->timer->setParameters(1800, 0, 0);
    this->animation.reset();
    while ((u8)fn_1_D840() == 0) {
        ThreadEntry::suspend(0);
    }
    HuAudSStreamFadeOut(musicHandle, 0x64);
    sendEvent(1, 3, 0.0f);
    this->complete = 1;
    if (this->mode == 1) {
        this->timer->start();
    }
    MinigameCore::showFinish();
    while ((u8)fn_1_DAFC() == 0) {
        ThreadEntry::suspend(0);
    }
    for (modeWait = 0; modeWait < secondsToFrames(2L); modeWait++) {
        ThreadEntry::suspend(0);
    }
    if (this->selectedMode != -1) {
        WipeCreate(2, 6, 60);
        for (closeWait = 0; closeWait < 60; closeWait++) {
            ThreadEntry::suspend(0);
        }
        sendEvent(1, 4, &this->selectedMode, 0.0f);
    }
    if (this->selectedMode != -1) {
        WipeCreate(1, 6, 60);
        for (openWait = 0; openWait < 60; openWait++) {
            ThreadEntry::suspend(0);
        }
    }
    sendEvent(1, 5, &this->selectedMode, 0.0f);
    if (this->selectedMode != -1) {
        jingle = 0x43;
    } else {
        jingle = 0x45;
    }
    HuAudJinglePlay((s16)jingle);
    MinigameCore::showWinners();
    while (MinigameCore::messageDone() == 0) {
        ThreadEntry::suspend(0);
    }
    MinigameCore::requestExit();
    for (;;) {
        ThreadEntry::suspend((void *)(-1));
    }
}

// Checks scene progress and timer completion.
s32 SceneObjectOwner::fn_1_D840()
{
    s32 matches;
    Scene *parameters;
    f32 firstValue;
    f32 secondValue;

    matches = 0;
    if (mode == 1) {
        if (WorkTimer(this)->isDone() != 0) {
            return 1;
        }
    }
    parameters = &Scene::getChecked();
    firstValue = parameters->firstRatio;
    if (firstValue == 1.0f) {
        selectedMode = 0;
        matches++;
    }
    secondValue = parameters->secondRatio;
    if (secondValue == 1.0f) {
        selectedMode = 1;
        matches++;
    }
    if (matches != 0) {
        if (matches == 2) {
            selectedMode = ((s32)RandomUnsigned()) & 1;
        }
        return 1;
    } else {
        if (mode == 0) {
            if (frame == 16200) {
                WorkTimer(this)->setMode(1);
                mode = 1;
            }
            frame++;
        }
        return 0;
    }
}

// Whether every player's act has started, and the voices the characters use.

// Plays the player result voices.
void fn_1_D9B0(void)
{
    std::vector<GameObject *> firstList;
    collectGameObjects(0x30, &firstList);
    std::vector<GameObject *> secondList;
    collectGameObjects(0x40, &secondList);
    s16 voiceIds[4];
    s32 firstIndex;
    s32 secondIndex;

    for (firstIndex = 0; firstIndex < 1; firstIndex++) {
        ActPlayer *actor = static_cast<ActPlayer *>(firstList[firstIndex]);
        voiceIds[firstIndex] = actor->VoiceIdOf();
    }
    for (secondIndex = 0; secondIndex < 3; secondIndex++) {
        ActPlayer *actor = static_cast<ActPlayer *>(secondList[secondIndex]);
        voiceIds[secondIndex + 1] = actor->VoiceIdOf();
    }
    fn_800611E4(voiceIds[0], voiceIds[1], voiceIds[2], voiceIds[3]);
}

// Sends the selected result event to the player objects.
u8 SceneObjectOwner::fn_1_DAFC()
{
    std::vector<GameObject *> firstList;
    collectGameObjects(0x30, &firstList);
    std::vector<GameObject *> secondList;
    collectGameObjects(0x40, &secondList);
    s32 firstIndex;
    s32 secondIndex;

    for (firstIndex = 0; firstIndex < 1; firstIndex++) {
        ActPlayer *object = static_cast<ActPlayer *>(firstList[firstIndex]);
        if (!object->actStarted)
            return 0;
    }
    for (secondIndex = 0; secondIndex < 3; secondIndex++) {
        ActPlayer *object = static_cast<ActPlayer *>(secondList[secondIndex]);
        if (!object->actStarted)
            return 0;
    }
    return 1;
}

// Does nothing.
void SceneObjectOwner::finishGame() { }

extern "C" {
int lbl_1_bss_50 = secondsToFrames(2L);
int lbl_1_bss_54 = (int)secondsToFrames(3.5);
int lbl_1_bss_58 = (int)secondsToFrames(3.5);
}
