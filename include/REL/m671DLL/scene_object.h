#ifndef REL_M671_SCENE_OBJECT_H
#define REL_M671_SCENE_OBJECT_H
#include <memory>
#include "REL/framework/thread.h"
#include "REL/framework/game_object_man.h"
#include "REL/m671DLL/resources.h"
#include "REL/framework/singleton.h"
#include "REL/framework/event_manager.h"
#include "REL/framework/minigame_core.h"
struct OwnedSceneChild {
    EventManager events;
    GameObjectManager objects;
    Resources resources;
};
struct SceneObjectOwner : MinigameCore, ThreadEntry {
    std::auto_ptr<OwnedSceneChild> childOwner;
    unsigned char triggered;
    SceneObjectOwner();
    ~SceneObjectOwner();
    void setupObjects(OMOBJMAN *);
    void updateGame();
    void finishGame();
    void run(void *);
};
#endif
