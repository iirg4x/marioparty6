#include "REL/m672DLL/random.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "REL/m672DLL/scene_child.h"
#include "REL/m672DLL/scene_thread_owner.h"
#include "REL/m672DLL/camera_owner_a348.h"
#include "REL/m672DLL/singleton_callback_owner.h"
#include "REL/m672DLL/event.h"
#include "REL/m672DLL/linknode.h"
extern "C" {
#include <math.h>
#include "game/audio.h"
#include "msm_stream.h"
#include "game/mg/actman.h"
}
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "REL/m672DLL/early_callback_8980.h"
#include "REL/framework/event_hm.h"
#include "REL/m672DLL/framework/thread.h"
#include "REL/m672DLL/effect_bf64.h"
#include "REL/m672DLL/collision_object.h"
#include "REL/m672DLL/event_queue.h"
#include "REL/framework/game_object.h"
#include "REL/m672DLL/singleton_32930.h"

class OwnedReleasedModel : public Model {
public:
    ~OwnedReleasedModel() {}
};

extern "C" void fn_1_749C(std::vector<OwnedReleasedModel *> *models);

void fn_1_749C(std::vector<OwnedReleasedModel *> *models)
{
    OwnedReleasedModel **first = models->begin();
    OwnedReleasedModel **last = models->end();
    while (first != last) delete *first++;
    models->clear();
}

extern "C" unsigned long OSGetTick(void);
extern "C" unsigned long lbl_1_bss_C[];

SceneOwner672::SceneOwner672()
    : MinigameCore(), ThreadEntry(0x10000), sequence(new TimerOwner(0)),
      child(new SceneChild672)
{
    RandomSeed(OSGetTick());
    lbl_1_bss_C[1] = 0;
}

class ModelHolder {
public:
    OwnedReleasedModel *value;
    ~ModelHolder() { delete value; }
};

void *operator new(unsigned long);

extern "C" {
char lbl_1_data_1C0[] = "self != 0";
char lbl_1_data_1CA[] = "singleton.h";
}

SceneOwner672::~SceneOwner672() {}

extern "C" {

extern void fn_1_23014(s32, s32, s32, s32);
extern void fn_1_2308C(s32, f32, f32);
extern void fn_1_2310C(s32, s32);
extern u8 fn_1_7BDC(SceneOwner672 *);
extern void fn_1_231D8(s32);

extern s32 fn_1_7EE8(void *);
extern u8 fn_1_7E10(void *);

}

extern "C" {

void *__nw__FUl(unsigned long);

void fn_1_6E04(void *);
}

extern "C" { void *__nw__FUl(unsigned long); }

extern "C" { void fn_1_6E04(void *); }

void fn_1_71BC(void *);
void fn_1_7230(void *);
void fn_1_7404(void *);
void fn_1_7408(void *);

static inline void *sceneState_inlined();

static inline void *sceneState_inlined()
{
    (Resources::self != 0) ? (void)0
        : __msl_assertion_failed(lbl_1_data_1C0, lbl_1_data_1CA, 0x26);
    return Resources::self;
}

void SceneOwner672::setupObjects(OMOBJMAN *)
{
    new CameraOwner;

    void *state = sceneState_inlined();
    fn_1_6E04(state);
    fn_1_71BC(state);
    fn_1_7230(state);
    fn_1_7404(state);
    fn_1_7408(state);

    new SingletonCallbackOwner;
}

extern "C" {
extern void fn_1_15818(void);
extern void fn_1_1B460(void);
extern void fn_1_1DFB4(void);

extern void fn_1_14764(void);

}

void updateGameObjects();

void SceneOwner672::updateGame() {
    resume(0);
    process_event_queue();
    updateGameObjects();
    fn_800856A8();
}

void SceneOwner672::run(void *)
{
    s32 musicHandle;
    s32 waitIndex;
    s32 transitionIndex;
    s32 resultCode;
    s32 jingle;
    TimerOwner *playSequence;
    TimerOwner *timingSequence;
    TimerOwner *modeSequence;
    TimerOwner *stopSequence;
    s32 tailWaitIndex;
    s32 index;

    MinigameCore::wipeIn();
    musicHandle = HuAudBGMPlay(MSM_STREAM_GAMING_WITH_BOWSER);
    sendEvent(0, 0, 0.0f);
    waitIndex = 0;
    while ((f64)waitIndex < 180.0) {
        ThreadEntry::suspend(0);
        waitIndex++;
    }
    sendEvent(1, 1, 0.0f);
    transitionIndex = 0;
    while ((f64)transitionIndex < 30.0) {
        ThreadEntry::suspend(0);
        transitionIndex++;
    }
    MinigameCore::showStart();
    sendEvent(0, 1, 0.0f);
    while (MinigameCore::messageDone() == 0) {
        ThreadEntry::suspend(0);
    }
    sendEvent(0, 2, 0.0f);
    playSequence = this->sequence.pointer;
    playSequence->setParameters(0x708, 0, 0);
    timingSequence = this->sequence.pointer;
    timingSequence->setPosition(50.0f, 63.0f);
    modeSequence = this->sequence.pointer;
    modeSequence->setMode(1);
    while (fn_1_7BDC(this) == 0) {
        ThreadEntry::suspend(0);
    }
    HuAudSStreamFadeOut(musicHandle, 0x64);
    sendEvent(0, 3, 0.0f);
    stopSequence = this->sequence.pointer;
    stopSequence->start();
    MinigameCore::showFinish();
    resultCode = fn_1_7EE8(this);
    tailWaitIndex = 0;
    while (tailWaitIndex < 0x78) {
        ThreadEntry::suspend(0);
        tailWaitIndex++;
    }
    while (fn_1_7E10(this) == 0) {
        ThreadEntry::suspend(0);
    }
    if (resultCode != 4) {
        jingle = 0x43;
    } else {
        jingle = 0x45;
    }
    HuAudJinglePlay((s16)jingle);
    sendEvent(0, 4, &resultCode, 0.0f);
    index = 0;
    while ((f64)index < 180.0) {
        ThreadEntry::suspend(0);
        index++;
    }
    MinigameCore::requestExit();
    for (;;) {
        ThreadEntry::suspend((void *)-1);
    }
}

void collectGameObjects(int category, std::vector<GameObject *> *objects);

/* Only the two flags these questions read. */
struct PlayerFlags {
    unsigned char unknown00[0x48]; /* No code names this field. */
    unsigned char active;
    unsigned char unknown49[0xB]; /* No code names this field. */
    unsigned char finished;
};

static inline TimerOwner *getSequence(SceneOwner672 *object)
{
    return object->sequence.pointer;
}

extern "C" u8 fn_1_7BDC(SceneOwner672 *object);
u8 fn_1_7BDC(SceneOwner672 *object)
{
    if (getSequence(object)->isDone()) {
        return 1;
    }

    std::vector<GameObject *> list;
    collectGameObjects(1, &list);
    int loopLimit = list.size();
    unsigned char result = 1;
    for (int index = 0; index < loopLimit; index++) {
        PlayerFlags *entryObject = reinterpret_cast<PlayerFlags *>(list[index]);
        if (!entryObject->active) {
            result = 0;
        }
    }
    if (result == 1) {
        return 1;
    }
    return 0;
}

extern "C" {

u8 fn_1_7E10(void *);
s32 fn_1_7EE8(void *);
}

void SceneOwner672::finishGame()
{
}

u8 fn_1_7E10(void *)
{
    std::vector<GameObject *> list;
    collectGameObjects(1, &list);
    int count = list.size();
    for (int index = 0; index < count; index++) {
        PlayerFlags *player = (PlayerFlags *)list[index];
        if (!player->finished) {
            return 0;
        }
    }
    return 1;
}

s32 fn_1_7EE8(void *)
{
    std::vector<GameObject *> list;
    collectGameObjects(1, &list);
    int count = list.size();
    int inactiveCount = 0;
    for (int index = 0; index < count; index++) {
        PlayerFlags *player = (PlayerFlags *)list[index];
        if (!player->active) {
            inactiveCount++;
        }
    }
    return inactiveCount;
}

template <class Time> Time secondsToFrames(Time seconds);

extern "C" s32 lbl_1_bss_30;
extern "C" s32 lbl_1_bss_34;
extern "C" s32 lbl_1_bss_38;
s32 lbl_1_bss_30 = secondsToFrames(2L);
s32 lbl_1_bss_34 = secondsToFrames(3.5);
s32 lbl_1_bss_38 = secondsToFrames(3.5);

/* Shared secondsToFrames template; native instances multiply by 60. */
template<class Time> Time secondsToFrames(Time seconds) { return 60 * seconds; }
