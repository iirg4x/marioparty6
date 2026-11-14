#include "REL/framework/event.h"
#include "REL/framework/random.h"
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
#include "REL/framework/timer.h"
extern "C" {
#include "game/audio.h"
#include "game/gamework.h"
#include "game/wipe.h"
}

#include "REL/framework/event_manager.h"
#include "REL/m652DLL/camera_split.h"
#include "REL/m652DLL/player_camera.h"
#include "REL/framework/event_packet.h"
#include "REL/framework/game_object_man.h"
#include "REL/m652DLL/player_work.h"
#include "REL/m652DLL/player_functions.h"
#include "REL/m652DLL/module_root.h"
#include "REL/m652DLL/man_map.h"

#include "REL/framework/phase_duration_statics.h"
#include "REL/m652DLL/game_constants.h"
class FrameCountdown {
public:
    int value;
    /* Advances the countdown and updates its timer. */
    void update(TimerOwner *resource) { advanceAndUpdate(resource); }
    void advanceAndUpdate(TimerOwner *resource);
};

class GameResults {
public:
    EventManager event;
    GameObjectManager objects;
    FrameCountdown remainingFrames;
    OwnedResource<TimerOwner> firstResource;
    OwnedResource<TimerOwner> secondResource;
    int record;
    unsigned char saved;
    /* Initializes the results managers, countdown, timer pointers and record. */
    GameResults() {
        remainingFrames.value = gameDuration - timeLimit;
        firstResource.value = 0;
        secondResource.value = 0;
        record = fn_8003C860(6);
        saved = 0;
    }
    /* Creates the first results timer in mode zero. */
    void createFirstTimer() { firstResource.reset(new TimerOwner(0)); }
    /* Creates the second results timer in mode one. */
    void createSecondTimer() { secondResource.reset(new TimerOwner(1)); }
    unsigned char saveRecord();
};

unsigned char fn_1_203C(ModuleRoot *);
/* Decrements a positive countdown and changes the timer mode when it expires. */
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

/* Saves and displays an improved record when record saving is enabled. */
inline unsigned char GameResults::saveRecord()
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

/* Creates the result state and registers event channel zero. */
ModuleRoot::ModuleRoot()
    : ThreadEntry(0x8000), result(new GameResults)
{
    registerChannel(0);
}
/* Releases the object through its member and base destructors. */
ModuleRoot::~ModuleRoot() {}
/* Seeds randomness and creates the cameras, map and players. */
void ModuleRoot::setupObjects(OMOBJMAN *)
{
    int index;

    RandomSeed(OSGetTick());
    new CameraSplit;

    index = 0;
    while (index < 2) {
        new PlayerCamera(index);
        index++;
    }

    new MapManager;
    fn_1_56D0();
    soundSetCameraLayout(2);
}

/* Resumes the game thread, dispatches events and updates request and game objects. */
void ModuleRoot::updateGame()
{
    resume(0);
    process_event_queue();
    updateRequestObjects();
    updateGameObjects();
    fn_800856A8();
}
/* Runs the opening, timed play, finish and result sequence. */
void ModuleRoot::run(void *)
{
    GameResults *result = this->result;
    int music;
    result->createFirstTimer();
    result->firstResource->setPosition(288.0f, 416.0f);
    result->firstResource->setParameters(timeLimit, 0, 0);
    result->createSecondTimer();
    result->secondResource->setParameters(0, gameDuration, result->record);
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

/* Enables record saving when event type one arrives. */
void ModuleRoot::receive(const EventPacket *event)
{
    if (event->type == 1) {
        GameResults *nestedForFlag;
        GameResults *nested = result.value;
        nestedForFlag = nested;
        nestedForFlag->saved = 1;
    }
}
/* Reports whether a winner exists or the second timer has completed. */
unsigned char fn_1_203C(ModuleRoot *owner)
{
    if (Racer::fn_1_5B2C() >= 1) return 1;
    GameResults *nested = owner->result.value;
    TimerOwner **childField = &nested->secondResource.value;
    TimerOwner *child = *childField;
    TimerOwner *childArgument = child;
    return childArgument->isDone();
}
/* Deletes all request objects. */
void ModuleRoot::finishGame() { deleteRequestObjects(); }
