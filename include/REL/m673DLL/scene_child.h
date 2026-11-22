// Declares the scene managers created by the minigame thread.

#ifndef REL_M673DLL_SCENE_CHILD_H
#define REL_M673DLL_SCENE_CHILD_H

#include "REL/framework/event_manager.h"
#include "REL/framework/game_object_man.h"
#include "REL/m673DLL/scene_resources.h"

// Owns the managers created by the minigame thread.
struct SceneChild {
    EventManager eventManager; // Dispatches scene events.
    GameObjectManager gameObjects; // Owns the scene object registry.
    SceneResources resources; // Owns the scene's loaded resources.

    // Constructs the child scene managers.
    SceneChild()
    {
    }
};

#endif
