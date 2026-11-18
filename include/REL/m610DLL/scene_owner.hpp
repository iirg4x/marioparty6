#ifndef REL_M610DLL_SCENE_OWNER_HPP
#define REL_M610DLL_SCENE_OWNER_HPP

#include "REL/framework/timer.h"
#include "REL/framework/minigame_core.h"
#include "REL/framework/thread.h"

#include "REL/framework/category_sphere.h"
#include "REL/framework/owned_pointer.h"

#include "REL/framework/event_manager.h"
#include "REL/framework/game_object_man.h"
#include "REL/m610DLL/effect_manager.h"
/* Own the scene event, object and effect managers. */
struct OwnedSceneChild
{
    EventManager events;
    GameObjectManager objects;
    EffectManager effects;
    RegisteredValue spheres[4];
    OwnedSceneChild();
};

/* Keep the remaining gameplay frame count. */
struct RemainingFrames
{
    int value;
    /* Initialize the remaining frame count. */
    RemainingFrames(int value) : value(value)
    {
    }
};

/* Run the minigame scene and own its gameplay objects. */
class SceneObjectOwner : public MinigameCore, public ThreadEntry
{
  public:
    SceneObjectOwner();
    virtual ~SceneObjectOwner();
    virtual void setupObjects(OMOBJMAN *);
    virtual void updateGame();
    virtual void finishGame();
    virtual void run(void *);
    RemainingFrames remainingFrames;
    TimerOwner timer;
    OwnedPointer<OwnedSceneChild> childOwner;
};

#endif
