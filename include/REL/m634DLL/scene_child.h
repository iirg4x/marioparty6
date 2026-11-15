// Event and object managers constructed in the scene's 0x58-byte child allocation.
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
