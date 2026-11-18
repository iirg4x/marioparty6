#ifndef REL_M628DLL_SCENE_THREAD_H
#define REL_M628DLL_SCENE_THREAD_H
#include "REL/framework/minigame_core.h"
#include "REL/framework/thread.h"
#include "REL/framework/timer.h"
#include "REL/framework/owned_pointer.h"
#include "REL/framework/event_manager.h"
#include "REL/framework/game_object_man.h"
#include "REL/m628DLL/animation_owner.h"

// Owns the scene event queue, object manager and animation resources.
struct SceneLayer
{
    EventManager base;
    GameObjectManager component;
    SceneAnimations animation; // Models and animations owned by the scene.
};

// Creates the scene and runs the round sequence on a game thread.
class SceneThreadOwner : public MinigameCore, public ThreadEntry
{
  public:
    SceneThreadOwner();
    ~SceneThreadOwner();
    void setupObjects(OMOBJMAN *);
    void updateGame();
    void finishGame();
    void run(void *initialArgument);
    OwnedPointer<SceneLayer> scene;
    OwnedPointer<TimerOwner> sequenceHandle; // Owns the timed round sequence.
    s8 complete;                            // Set when all players have locked in.
    s32 activeCount;                        // Number of players that have not locked in.
};
#endif
