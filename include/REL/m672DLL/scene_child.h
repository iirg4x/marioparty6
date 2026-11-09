#ifndef REL_M672DLL_SCENE_CHILD_H
#define REL_M672DLL_SCENE_CHILD_H
#include "REL/m672DLL/event_lifetime.h"
#include "REL/framework/game_object_man.h"
#include "REL/m672DLL/resources.h"

struct SceneChild672 {
    EventManager root;
    GameObjectManager models;
    Resources animations;
    SceneChild672() {}
    ~SceneChild672() {}
};
#endif
