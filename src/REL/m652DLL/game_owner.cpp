#include "REL/framework/list_object.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
#include "REL/framework/sound.h"
#include "msm_stream.h"
#include <math.h>
#include "dolphin.h"
#include "game/gamework_enum.h"
#include "game/mg/timer.h"
#include "REL/framework/minigame_core.h"
#include "REL/framework/thread.h"
extern "C" {
#include "game/audio.h"
#include "game/gamework.h"
#include "game/wipe.h"
}

struct EventManager : MinigameSingleton<EventManager> {
    EventManager();
    ~EventManager();
};
#include "REL/m652DLL/camera_split.h"
#include "REL/m652DLL/player_camera.h"
#include "REL/framework/event_packet.h"
#include "REL/framework/game_object_man.h"
#include "REL/m652DLL/player_work.h"
#include "REL/m652DLL/player_algorithms.h"
#include "REL/m652DLL/module_root.h"

extern "C" {
extern int lbl_1_bss_2C, lbl_1_bss_30;
}
class TimerOwner {
public:
    MGTIMER *timer;
    int getValue();
    unsigned char isDone();
    void turnModeOff();
    void showRecord();
    void reset();
    TimerOwner(int mode);
    ~TimerOwner();
    void setParameters(int,int,int);
    void setPosition(float,float);
    void setMode(int);
    void start();
    void setRecordTime(int);
};

class FrameCountdown {
public:
    int value;
    void update(TimerOwner *resource) { advanceAndUpdate(resource); }
    void advanceAndUpdate(TimerOwner *resource);
};

class ResultView {
public:
    EventManager event;
    GameObjectManager objects;
    FrameCountdown remainingFrames;
    OwnedResource<TimerOwner> firstResource;
    OwnedResource<TimerOwner> secondResource;
    int record;
    unsigned char saved;
    ResultView() {
        remainingFrames.value = lbl_1_bss_2C - lbl_1_bss_30;
        firstResource.value = 0;
        secondResource.value = 0;
        record = fn_8003C860(6);
        saved = 0;
    }
    void createFirstTimer() { firstResource.reset(new TimerOwner(0)); }
    void createSecondTimer() { secondResource.reset(new TimerOwner(1)); }
    unsigned char saveRecord();
};

unsigned char fn_1_203C(ModuleRoot *);
extern "C" {

extern void fn_1_596C(void);
extern void fn_1_59BC(void);
extern void fn_1_5A0C(void);
extern int lbl_1_bss_2C;
extern int lbl_1_bss_30;
}

void process_event_queue();
void updateGameObjects();
void RandomSeed(unsigned long);
extern "C" {

void *__ct__10MapManagerFv(void *);
void fn_1_56D0();

void *__nw__FUl(unsigned long);
}
inline void FrameCountdown::advanceAndUpdate(TimerOwner *resource)
{
    unsigned char expired = 0;
    if (value > 0) {
        if ((value -= 1) == 0) {
            expired = 1;
        }
    }
    if (expired != 0) {
        resource->setMode(1);
    }
}

inline unsigned char ResultView::saveRecord()
{
    if (saved == 0) {
        return 0;
    }
    int score = secondResource->getValue();
    if (score >= record) {
        return 0;
    }
    record = score;
    fn_8003C80C(6, (unsigned int) score);
    secondResource->setRecordTime(-1);
    MinigameCore::showRecord(score);
    return 1;
}

ModuleRoot::ModuleRoot()
    : ThreadEntry(0x8000), result(new ResultView)
{
    registerChannel(0);
}
ModuleRoot::~ModuleRoot() {}
void ModuleRoot::setupObjects(OMOBJMAN *)
{
    void *finalObject;
    int index;

    RandomSeed(OSGetTick());
    new CameraSplit;

    index = 0;
    while (index < 2) {
        new PlayerCamera(index);
        index++;
    }

    finalObject = __nw__FUl(0x54);
    if (finalObject) {
        finalObject = __ct__10MapManagerFv(finalObject);
    }
    fn_1_56D0();
    soundSetCameraLayout(2);
}

void ModuleRoot::updateGame()
{
    resume(0);
    process_event_queue();
    updateRequestObjects();
    updateGameObjects();
    fn_800856A8();
}
void ModuleRoot::run(void *)
{
    ResultView *result = this->result;
    int music;
    result->createFirstTimer();
    result->firstResource->setPosition(288.0f, 416.0f);
    result->firstResource->setParameters(lbl_1_bss_30, 0, 0);
    result->createSecondTimer();
    result->secondResource->setParameters(0, lbl_1_bss_2C, result->record);
    result->secondResource->showRecord();
    FrameCountdown * remainingFrames = & this->result->remainingFrames;
    TimerOwner * firstTimer = this->result->firstResource;
    TimerOwner * secondTimer = this->result->secondResource;
    music = HuAudBGMPlay(MSM_STREAM_TIME_TO_DUEL);
    MinigameCore::wipeIn();
    while (WipeCheckIn() != 0)
    {
        ThreadEntry::suspend(0);
    }
    postRequest(0);
    while (requestsCleared() == 0)
    {
        ThreadEntry::suspend(0);
    }
    postRequest(1);
    MinigameCore::showStart();
    while (MinigameCore::messageDone() == 0)
    {
        ThreadEntry::suspend(0);
    }
    postRequest(2);
    fn_1_596C();
    secondTimer->setMode(0);
    while (fn_1_203C(this) == 0)
    {
        remainingFrames->update(firstTimer);
        ThreadEntry::suspend(0);
    }
    secondTimer->turnModeOff();
    int record;
    if (remainingFrames->value <= 0)
    {
        firstTimer->start();
    }
    else
    {
        firstTimer->reset();
    }
    postRequest(3);
    fn_1_59BC();
    MinigameCore::showFinish();
    HuAudSStreamFadeOut(music, 0x64);
    for (record = 0; record < secondsToFrames<long>(3); record++)
    {
        ThreadEntry::suspend(0);
    }
    if (this->result->saveRecord() != 0)
    {
        while (MinigameCore::messageDone() == 0)
        {
            ThreadEntry::suspend(0);
        }
    }
    fn_1_5A0C();
    postRequest(4);
    MinigameCore::showWinners();
    HuAudJinglePlay((short)(Racer::fn_1_5B2C() > 0 ? 0x43 : 0x45));
    while (MinigameCore::messageDone() == 0)
    {
        ThreadEntry::suspend(0);
    }
    MinigameCore::requestExit();
    for (;;)
    {
        ThreadEntry::suspend(reinterpret_cast<void *>(-1));
    }
}

void ModuleRoot::receive(const EventPacket *event)
{
    if (event->type == 1) {
        ResultView *nestedForFlag;
        ResultView *nested = result.value;
        nestedForFlag = nested;
        nestedForFlag->saved = 1;
    }
}
unsigned char fn_1_203C(ModuleRoot *owner)
{
    if (Racer::fn_1_5B2C() >= 1) return 1;
    ResultView *nested = owner->result.value;
    TimerOwner **childField = &nested->secondResource.value;
    TimerOwner *child = *childField;
    TimerOwner *childArgument = child;
    return childArgument->isDone();
}
void ModuleRoot::finishGame() { deleteRequestObjects(); }

extern "C" {

int lbl_1_bss_20 = secondsToFrames<long>(2);
int lbl_1_bss_24 = secondsToFrames(3.5);
int lbl_1_bss_28 = secondsToFrames(3.5);
int lbl_1_bss_2C = minutesToFrames(5L);
int lbl_1_bss_30 = secondsToFrames<long>(30);
float lbl_1_bss_34 = 0.016666666666666666 * (0.016666666666666666 * cmToWorld(980.665f));
}
