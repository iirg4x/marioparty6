#include "REL/m654DLL/time_conversion.h"
#include "REL/m654DLL/sound_spatial.h"
#include "REL/m654DLL/unit_conversion.h"
#include "REL/m654DLL/camera_node_owner.h"
#include "REL/m654DLL/timer_owner.h"
#include "REL/m654DLL/scene_owner.h"
#include "REL/m654DLL/manager_event_base.h"
#include "math.h"
#include "REL/m654DLL/singleton_32930.h"
#include "REL/framework/thread.h"
extern "C" {
#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
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
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include "REL/m654DLL/pad_control.h"
#include "REL/m654DLL.h"

}
#include "dolphin/types.h"
template<class Time> Time secondsToFrames(Time);
inline int secondsToFrames(int seconds) { return 60 * seconds; }
void RandomSeed(unsigned long);
void process_event_queue();
void updateGameObjects();
void *operator new(unsigned long);
/* Scales the argument by 60 before forwarding it to the sequence helper. */
extern void fn_1_192E4(void);

extern int fn_1_191C0(int mode);
extern unsigned char fn_1_19258(void);
extern "C" {

extern int lbl_1_bss_1C;

}

extern "C" {

extern void *__ct__19ModelAnimationOwnerFv(void *owner);

/* Starts the module services and creates their game and scene owners. */
}

/* Initializes timing values and the startup sequence state. */
extern "C" {
extern int lbl_1_bss_10;
extern int lbl_1_bss_14;
extern int lbl_1_bss_18;
extern float lbl_1_bss_20;

}

void fn_1_19354(void);

extern "C" {
/* Initializes the callback table and attached registry/timer storage. */
}
SceneObjectOwner::SceneObjectOwner() : ThreadEntry(0x8000), childOwner(new OwnedSceneChild) {}

SceneChildOwner::~SceneChildOwner()
{
    delete child;
}

SceneObjectOwner::~SceneObjectOwner() {}

void SceneObjectOwner::setupObjects(OMOBJMAN *)
{
    void *sceneOwner;

    RandomSeed(OSGetTick());
    new CameraManager;
    sceneOwner = operator new(0x3C);
    if (sceneOwner != 0) {
        sceneOwner = __ct__19ModelAnimationOwnerFv(sceneOwner);
    }
    fn_1_3274();
    fn_1_A134(0);
}
void SceneObjectOwner::updateGame()
{
    resume(0);
    process_event_queue();
    fn_1_192E4();
    updateGameObjects();
    fn_800856A8();
}
void SceneObjectOwner::run(void *argument)
{
    int bgmHandle;
    int frame;
    int manager = reinterpret_cast<int>(childOwner.child);
    int view = manager + 0x34;

    MinigameCore::wipeIn();
    while (WipeCheckIn() != 0) {
        suspend(0);
    }
    bgmHandle = HuAudBGMPlay(MSM_STREAM_TIME_TO_DUEL);
    fn_1_3398(0);
    while ((unsigned char)fn_1_3528() == 0) {
        suspend(0);
    }
    fn_1_191C0(0);
    while ((unsigned char)fn_1_19258() == 0) {
        suspend(0);
    }
    fn_1_191C0(1);
    while ((unsigned char)fn_1_19258() == 0) {
        suspend(0);
    }
    fn_1_3398(1);
    while ((unsigned char)fn_1_3528() == 0) {
        suspend(0);
    }
    fn_1_191C0(2);
    MinigameCore::showStart();
    while ((unsigned char)MinigameCore::messageDone() == 0) {
        suspend(0);
    }
    fn_1_191C0(3);
    fn_1_3494();
    reinterpret_cast<TimerOwner *>(view)->setPosition(288.0f, 416.0f);
    reinterpret_cast<TimerOwner *>(view)->setParameters(lbl_1_bss_1C, 0, 0);
    reinterpret_cast<TimerOwner *>(view)->setMode(1);
    while ((unsigned char)fn_1_10DC() == 0) {
        suspend(0);
    }
    reinterpret_cast<TimerOwner *>(view)->start();
    fn_1_191C0(4);
    fn_1_35F8();
    MinigameCore::showFinish();
    HuAudSStreamFadeOut(bgmHandle, 0x64);
    for (frame = 0; frame < secondsToFrames<long>(3); frame++) {
        suspend(0);
    }
    while ((unsigned char)fn_1_3528() == 0) {
        suspend(0);
    }
    fn_1_3768();
    fn_1_191C0(5);
    MinigameCore::showWinners();
    HuAudJinglePlay((short)(fn_1_37FC() > 0 ? 0x43 : 0x45));
    while ((unsigned char)MinigameCore::messageDone() == 0) {
        suspend(0);
    }
    MinigameCore::requestExit();
    for (;;) {
        suspend(reinterpret_cast<void *>(-1));
    }
}
unsigned char SceneObjectOwner::fn_1_10DC()
{
    OwnedSceneChild *manager;
    if (fn_1_37FC() > 0) {
        return 1;
    }
    manager = childOwner.child;
    return manager->timerOwner.isDone();
}
void SceneObjectOwner::finishGame()
{
    fn_1_19354();
}
extern "C" {
int lbl_1_bss_10 = secondsToFrames(2);
int lbl_1_bss_14 = secondsToFrames(3.5);
int lbl_1_bss_18 = secondsToFrames(3.5);
int lbl_1_bss_1C = fn_1_B80(1);
float lbl_1_bss_20 = framesToSeconds(framesToSeconds(980.665));
}

void* operator new(unsigned long);

// Same-game framework time conversion.
template<class Time> Time secondsToFrames(Time seconds) { return 60 * seconds; }

// Active game-object registry.
template<> GameObjectManager *MinigameSingleton<GameObjectManager>::self = 0;

// Current scene and camera.
class ModelAnimationOwner;
template<> ModelAnimationOwner *MinigameSingleton<ModelAnimationOwner>::self = 0;
template<> CameraManager *MinigameSingleton<CameraManager>::self = 0;
