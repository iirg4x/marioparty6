// Event and object managers owned by the scene.
#ifndef REL_M634DLL_SCENE_CHILD_H
#define REL_M634DLL_SCENE_CHILD_H
#include "REL/framework/game_object_man.h"
#include "REL/m634DLL/game_manager.h"

#include "REL/framework/event_manager.h"

struct SceneChild {
    EventManager baseState;
    GameObjectManager secondaryState;
    GameManager tertiaryState;
};
#endif
