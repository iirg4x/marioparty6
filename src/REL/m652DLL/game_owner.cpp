void fn_1_86AC(long);
bool fn_1_17FB8();
void fn_1_18044();
void fn_1_180B4();
#include "REL/m652DLL/time_conversion.h"
template<> long secondsToFrames<long>(long);
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
#include "REL/framework/event_packet.h"
#include "REL/framework/game_object_man.h"
#include "REL/m652DLL/player_work.h"
#include "REL/m652DLL/player_algorithms.h"

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

template <class Resource> class OwnedResource {
public:
    Resource *value;
    OwnedResource() {}
    OwnedResource(Resource *initial) : value(initial) {}
    ~OwnedResource() { delete value; }
    operator Resource *() const { return value; }
    Resource *operator->() const { return value; }
    void reset(Resource *replacement)
    {
        if (value != replacement) {
            delete value;
            value = replacement;
        }
    }
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

class ModuleRoot : public MinigameCore, public ThreadEntry,
                            public EventListener {
public:
    OwnedResource<ResultView> result;
    ModuleRoot();
    virtual ~ModuleRoot();
    virtual void setupObjects(OMOBJMAN *);
    virtual void updateGame();
    virtual void finishGame();
    virtual void receive(const EventPacket *event);
    virtual void run(void *);
};
extern void fn_1_17F20(int);
unsigned char fn_1_203C(ModuleRoot *);
extern "C" {

extern int fn_1_1F998(TimerOwner *, int);

extern void fn_1_596C(void);
extern void fn_1_59BC(void);
extern void fn_1_5A0C(void);
extern unsigned int fn_1_A900(int);
extern int lbl_1_bss_2C;
extern int lbl_1_bss_30;
extern void __dt__10TimerOwnerFv(void *, int);
extern void fn_1_1503C(void *, int);
extern void fn_1_F8D4(void *, int);
extern void fn_1_FFA0(void *, int);
extern void fn_1_1F20C(void *, int);
extern void __dt__12MinigameCoreFv(void *, int);
extern void __dl__FPv(void *);
}

void process_event_queue();
void updateGameObjects();
void RandomSeed(unsigned long);
extern "C" {

void *__ct__12PlayerCameraFUi(void *, int);
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
    void *indexedObject;
    void *finalObject;
    int index;

    RandomSeed(OSGetTick());
    new CameraSplit;

    index = 0;
    while (index < 2) {
        indexedObject = __nw__FUl(0x94);
        if (indexedObject) {
            indexedObject = __ct__12PlayerCameraFUi(indexedObject, index);
        }
        index++;
    }

    finalObject = __nw__FUl(0x54);
    if (finalObject) {
        finalObject = __ct__10MapManagerFv(finalObject);
    }
    fn_1_56D0();
    fn_1_86AC(2);
}

void ModuleRoot::updateGame()
{
    resume(0);
    process_event_queue();
    fn_1_18044();
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
    fn_1_17F20(0);
    while (fn_1_17FB8() == 0)
    {
        ThreadEntry::suspend(0);
    }
    fn_1_17F20(1);
    MinigameCore::showStart();
    while (MinigameCore::messageDone() == 0)
    {
        ThreadEntry::suspend(0);
    }
    fn_1_17F20(2);
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
    fn_1_17F20(3);
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
    fn_1_17F20(4);
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
void ModuleRoot::finishGame() { fn_1_180B4(); }

template<> inline long secondsToFrames<long>(long value)
{
    return value * 60;
}

extern "C" {

int lbl_1_bss_20 = secondsToFrames<long>(2);
int lbl_1_bss_24 = secondsToFrames(3.5);
int lbl_1_bss_28 = secondsToFrames(3.5);
int lbl_1_bss_2C = fn_1_2178(5);
int lbl_1_bss_30 = secondsToFrames<long>(30);
float lbl_1_bss_34 = framesToSeconds(framesToSeconds(980.665));
}

template<class T> int fn_1_2178(T amount)
{
    return amount * 0x3C * 0x3C;
}

template<class T> T secondsToFrames(T value)
{
    return 60.0 * value;
}
