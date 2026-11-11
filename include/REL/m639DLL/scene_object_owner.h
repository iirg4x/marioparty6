#ifndef M639_SCENE_OBJECT_OWNER_H
#define M639_SCENE_OBJECT_OWNER_H
#include "REL/m639DLL/mode_selection.h"
#include "REL/m639DLL/timer_owner.h"
#include "REL/framework/game_object_man.h"
#include "REL/framework/thread.h"
#include "REL/framework/minigame_core.h"
#include "REL/m639DLL/event_manager.h"
struct OwnedSceneChild;
struct SceneChildOwner {
    OwnedSceneChild *child;
    SceneChildOwner(OwnedSceneChild* value) : child(value) {}
    ~SceneChildOwner();
};
struct SceneObjectOwner : MinigameCore, ThreadEntry {
    SceneChildOwner childOwner;
    SceneObjectOwner();
    virtual ~SceneObjectOwner();
    void setupObjects(OMOBJMAN *);
    void updateGame();
    void finishGame();
    void run(void *);
    long fn_1_112C();
};
#endif
