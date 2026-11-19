#include "REL/m605DLL/player_character.h"
#include "REL/framework/list_object.h"
#include "REL/framework/frame_durations.h"
#include "REL/m605DLL/module_globals.h"
#include "REL/m605DLL/board_view.h"
#include "REL/m605DLL/animation_owner.h"
#include "REL/framework/game_object_man.h"
#include "REL/framework/model.h"

#include "REL/framework/octtree_man.h"

#include "dolphin/types.h"
#include "REL/framework/event.h"
#include "REL/framework/random.h"
extern "C" {
#include "game/audio.h"
#include "msm_stream.h"
}
#include "REL/m605DLL/scene_stage.h"
#include "REL/m605DLL/falling_rock.h"
#include "REL/m605DLL/scene_controller.h"

// Owns the event manager, scene objects and their animations.
struct EffectLayer {
    EventManager events;
    GameObjectManager layer;
    AnimationOwner state;
};

extern u32 lbl_1_bss_40;

// Creates the scene layer and initializes the game clock.
BoardView::BoardView() : ThreadEntry(0x8000), effectOwner(new EffectLayer)
{
    lbl_1_bss_40 = 0;
    lbl_1_bss_3C = 0.0f;
}

// Destroys the board view and its owned scene layer.
BoardView::~BoardView() {}

// Creates the stage, course model, four players and camera controller.
void BoardView::setupObjects(OMOBJMAN *)
{
    int childIndex;

    enableDefaultAttributeA();
    RandomSeed(OSGetTick());

    new SceneStage;

    new FallingRock;

    for (childIndex = 0; childIndex < 4; childIndex++) {
        EffectLayer *effects = effectOwner.pointer;
        fn_1_5D4(&effects->state.collisionTree, childIndex);
    }
    registerChannel(1);
    registerChannel(0);

    new SceneController;
}

// Accepts board events without additional handling.
void BoardView::receive(const EventPacket *) {}

// Resumes the game sequence, updates objects and advances the game clock.
void BoardView::updateGame()
{
    OctreeDebugNode *currentValue;
    OctreeDebugNode *nextValue;
    Octree *object;

    resume(0);
    process_event_queue();
    updateGameObjects();
    updateRequestObjects();
    lbl_1_bss_3C = framesToSeconds((f32)++lbl_1_bss_40);
    object = lbl_1_bss_2C;
    currentValue = object->nodes;
    nextValue = currentValue;
    lbl_1_bss_2C->select(nextValue);
}

struct SceneController;

// Runs the introduction, timed game, finish sequence and exit request.
void BoardView::run(void *)
{
    SceneController *selectedObject;
    s32 waitFrames;
    s32 frames;
    s32 countdown;
    s32 stream;
    u8 finished;
    postRequest(0);
    sendEvent(0, 2, 0.0f);
    MinigameCore::wipeIn();
    selectedObject = &MinigameSingleton<SceneController>::getChecked();
    fn_1_23DC(selectedObject, 5.0f, 2.0f);
    sendEvent(0, 0, 2.0f);
    for (waitFrames = 0; (f32) waitFrames < secondsToFrames(3.5f); waitFrames++)
    {
        suspend(0);
    }
    sendEvent(0, 3, 0.0f);
    postRequest(1);
    MinigameCore::showStart();
    stream = HuAudSStreamPlay(MSM_STREAM_DOOM_AND_GLOOM);
    while (MinigameCore::messageDone() == 0)
    {
        suspend(0);
    }
    finished = 0;
    postRequest(2);
    countdown = secondsToFrames(60L);
    while (1)
    {
        if (fn_1_9018())
        {
            finished = 1;
        }
        else
        {
            if (fn_1_9120())
            {
                fn_1_92EC();
                break;
            }
            else
            {
                countdown = countdown - 1;
                if (countdown < 0)
                {
                    finished = 1;
                }
                else
                {
                    suspend(0);
                    continue;
                }
            }
        }
        break;
    }
    postRequest(3);
    HuAudSStreamFadeOut(stream, 100);
    MinigameCore::showFinish();
    for (frames = 0; frames < secondsToFrames(2L); frames++)
    {
        suspend(0);
    }
    postRequest(4);
    HuAudJinglePlay((s16)(finished != 0 ? MSM_STREAM_BAD_NEWS : MSM_STREAM_WINNING));
    if (finished == 0)
    {
        sendEvent(0, 4, 0.0f);
    }
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

// Clears the scheduled node list when the game finishes.
void BoardView::finishGame() { deleteRequestObjects(); }

Octree *lbl_1_bss_2C;
FallingRock *lbl_1_bss_30;
f32 lbl_1_bss_34;
f32 lbl_1_bss_38;
f32 lbl_1_bss_3C;
u32 lbl_1_bss_40;

template <> GameObjectManager *MinigameSingleton<GameObjectManager>::self = 0;
