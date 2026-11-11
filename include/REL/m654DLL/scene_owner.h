#include "REL/framework/game_object_man.h"
#include "REL/m654DLL/timer_owner.h"
#include "REL/framework/minigame_core.h"
#include "REL/framework/thread.h"
struct EventManager : MinigameSingleton<EventManager> {
    EventManager();
    ~EventManager();
};
struct OwnedSceneChild {
    EventManager eventManager;
    GameObjectManager registry;
    TimerOwner timerOwner;
    OwnedSceneChild() : timerOwner(0) {}
};
struct SceneChildOwner {
    OwnedSceneChild *child;
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
