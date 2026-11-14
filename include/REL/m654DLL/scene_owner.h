#ifndef M654_SCENE_OWNER_H
#define M654_SCENE_OWNER_H
#include "REL/framework/game_object_man.h"
#include "REL/framework/timer.h"
#include "REL/framework/minigame_core.h"
#include "REL/framework/thread.h"
#include "REL/framework/event_manager.h"
struct OwnedSceneChild {
    EventManager eventManager;
    GameObjectManager registry;
    TimerOwner timerOwner;
    // Creates the round timer on channel zero.
    OwnedSceneChild() : timerOwner(0) {}
};
struct SceneChildOwner {
    OwnedSceneChild *child;
    // Stores the owned scene child.
    SceneChildOwner(OwnedSceneChild *value) : child(value) {}
    ~SceneChildOwner();
};
struct SceneObjectOwner : MinigameCore, ThreadEntry {
    SceneObjectOwner();
    virtual ~SceneObjectOwner();
    virtual void setupObjects(OMOBJMAN *);
    virtual void run(void *argument);
    virtual void updateGame();
    virtual void finishGame();
    unsigned char fn_1_10DC();
    SceneChildOwner childOwner;
};

#endif
