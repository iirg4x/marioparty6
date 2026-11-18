#include <new>
#include "msm_stream.h"
#include "REL/framework/event_manager.h"
#include "REL/framework/timer.h"
#include "REL/framework/random.h"
#include "REL/framework/event.h"
#include "REL/framework/frame_durations.h"
#include "dolphin/types.h"

#include "dolphin/os.h"

#include "REL/framework/model.h"

#include "REL/framework/event_packet.h"

#include "REL/framework/game_object.h"

extern "C"
{
#include "game/hu3d.h"
}

extern "C"
{
#include "game/sprite.h"
}

extern "C"
{
#include "game/data.h"
}

extern "C"
{
#include "game/memory.h"
}

#include "REL/m628DLL/animation_owner.h"

#include "REL/m628DLL/scene_thread.h"

#include "REL/framework/game_object_man.h"

#include "REL/framework/minigame_core.h"

#include "REL/framework/thread.h"

extern "C"
{
#include "game/audio.h"
}

extern "C"
{
#include "game/mg/actman.h"
}

#include <vector>

#include <algorithm>

#include <functional>
#include "REL/m628DLL/act_player.h"

#include "REL/m628DLL/board_positions.h"

#include "REL/m628DLL/range_record.h"

#include "REL/m628DLL/camera_owner.h"

u8 fn_1_11D2C(SceneThreadOwner *sceneState);

u8 fn_1_11E60(SceneThreadOwner *scene);

// Container view used to collect and release scene objects.
struct SceneObjectList
{
    u32 capacity; // Allocation capacity read by the vector collection operations.
    s32 count;
    void **items;
};

// Creates the scene and sequence timer and clears round state.
SceneThreadOwner::SceneThreadOwner()
    : MinigameCore(), ThreadEntry(0x10000), scene(new SceneLayer), sequenceHandle(new TimerOwner(0))
{
    RandomSeed(OSGetTick());
    lbl_1_bss_34.clear();
    this->complete = 0;
    this->activeCount = 0;
}

// Deletes the sequence timer and scene layer.
SceneThreadOwner::~SceneThreadOwner() {}

// Creates the camera, board tracker, players and scene effects.
void SceneThreadOwner::setupObjects(OMOBJMAN *)
{
    new CameraOwner;
    BoardPositions *activeSecond = new BoardPositions;
    SceneAnimations *scene = &MinigameSingleton<SceneAnimations>::getChecked();
    fn_1_10760(scene);
    fn_1_10BD8(scene);
    fn_1_10D34(scene);
    fn_1_12474(activeSecond);
}

// Advances the game thread, event queue, objects and collision tests.
void SceneThreadOwner::updateGame()
{
    resume(0);
    process_event_queue();
    updateGameObjects();
    fn_800856A8();
    fn_1_47FC();
}

// Runs the opening, timed round, finish and result sequences.
void SceneThreadOwner::run(void *)
{
    s32 musicHandle;
    s32 openingFrame;
    s32 frame;

    sendEvent(1, 0, 0.0f);
    MinigameCore::wipeIn();
    openingFrame = 0;
    while ((f32)openingFrame < (f32)(4.0 * 60.0))
    {
        suspend(0);
        openingFrame++;
    }
    musicHandle = HuAudBGMPlay(MSM_STREAM_DOOM_AND_GLOOM);
    sendEvent(1, 1, 0.0f);
    MinigameCore::showStart();
    while (MinigameCore::messageDone() == 0)
    {
        suspend(0);
    }
    sendEvent(1, 2, 0.0f);
    sequenceHandle.get()->setParameters(1800, 0, 0);
    sequenceHandle.get()->setMode(1);
    while (fn_1_11D2C(this) == 0)
    {
        suspend(0);
    }
    HuAudSStreamFadeOut(musicHandle, 100);
    sendEvent(1, 3, &activeCount, 0.0f);
    sequenceHandle.get()->start();
    MinigameCore::showFinish();
    while (fn_1_11E60(this) == 0)
    {
        suspend(0);
    }
    frame = 0;
    while (frame < secondsToFrames(2L))
    {
        suspend(0);
        frame++;
    }
    sendEvent(1, 4, &complete, 0.0f);
    HuAudJinglePlay(MSM_STREAM_WINNING);
    MinigameCore::showWinners();
    while (MinigameCore::messageDone() == 0)
    {
        suspend(0);
    }
    MinigameCore::requestExit();
    for (;;)
    {
        suspend(reinterpret_cast<void *>(-1));
    }
}

// Reports timeout or whether every player has locked in.
u8 fn_1_11D2C(SceneThreadOwner *sceneState)
{
    SceneObjectList objects;
    s32 loopLimit;
    u8 completeResult;
    u8 incompleteResult;
    SceneObjectList *objectsOwner;
    void **completeAllocation;
    void **incompleteAllocation;
    TimerOwner *timer;
    s32 count;
    SceneObjectList *objectsArgument;
    s32 index;
    u8 allComplete;
    ActPlayer *object;
    ActPlayer **entryAddress;
    ActPlayer **entry;

    timer = sceneState->sequenceHandle.pointer;
    if (timer->isDone() != 0)
    {
        return 1;
    }

    objectsOwner = &objects;
    objectsArgument = objectsOwner;
    objectsArgument->capacity = 0;
    objectsArgument->count = 0;
    objectsArgument->items = 0;
    collectGameObjects(0x40, reinterpret_cast<std::vector<GameObject *> *>(&objects));

    count = objects.count;
    loopLimit = count;
    sceneState->activeCount = 0;
    allComplete = 1;
    index = 0;
    for (; index < loopLimit; index++)
    {
        entryAddress = (ActPlayer **)objects.items + index;
        entry = entryAddress;
        object = *entry;
        if (object->isLocked() != 1)
        {
            allComplete = 0;
            sceneState->activeCount = sceneState->activeCount + 1;
        }
    }

    if (allComplete == 1)
    {
        sceneState->complete = 1;
        completeResult = allComplete;
        if (objects.items)
        {
            completeAllocation = objects.items;
            ::operator delete(completeAllocation);
        }
        return completeResult;
    }

    incompleteResult = 0;
    if (objects.items)
    {
        incompleteAllocation = objects.items;
        ::operator delete(incompleteAllocation);
    }
    return incompleteResult;
}

// Reports whether every player has finished its result animation.
u8 fn_1_11E60(SceneThreadOwner *scene)
{
    SceneObjectList list;
    s32 loopLimit;
    u8 inactiveResult;
    u8 completeResult;
    SceneObjectList *listOwner = &list;
    void **inactiveAllocation;
    void **completeAllocation;
    s32 count;
    SceneObjectList *listArgument = listOwner;
    s32 index;

    listArgument->capacity = 0;
    listArgument->count = 0;
    listArgument->items = 0;
    collectGameObjects(0x40, reinterpret_cast<std::vector<GameObject *> *>(&list));
    count = list.count;
    loopLimit = count;
    index = 0;
    for (; index < loopLimit; index++)
    {
        ActPlayer *object;
        ActPlayer **entryAlias;
        ActPlayer **entry;

        entry = (ActPlayer **)list.items + index;
        entryAlias = entry;
        object = *entryAlias;
        if (!object->finishComplete)
        {
            inactiveResult = 0;
            if (list.items != 0)
            {
                inactiveAllocation = list.items;
                ::operator delete(inactiveAllocation);
            }
            return inactiveResult;
        }
    }
    completeResult = 1;
    if (list.items != 0)
    {
        completeAllocation = list.items;
        ::operator delete(completeAllocation);
    }
    return completeResult;
}

// Leaves final cleanup to the scene owners.
void SceneThreadOwner::finishGame() {}
