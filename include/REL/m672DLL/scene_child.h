#ifndef REL_M672DLL_SCENE_CHILD_H
#define REL_M672DLL_SCENE_CHILD_H
#include "REL/framework/event_manager.h"
#include "REL/framework/game_object_man.h"
#include "REL/m672DLL/resources.h"

struct SceneChild {
    EventManager eventManager;
    GameObjectManager gameObjects;
    Resources resources;
    // Construct the child scene managers.
    SceneChild() {}
    // Destroy the child scene managers.
    ~SceneChild() {}
};
#endif
