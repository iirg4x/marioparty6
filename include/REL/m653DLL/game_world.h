#ifndef REL_M653DLL_GAME_WORLD_H
#define REL_M653DLL_GAME_WORLD_H
#include "REL/framework/event_manager.h"
#include "REL/framework/game_object_man.h"
#include "REL/m653DLL/native_resource.h"
#include "REL/framework/category_sphere.h"
/* What one round of the game owns, made by the game controller and deleted with it: the event
 * and object managers, the resource owner, four registered points of the arena, and the stage
 * models (DATANUM(0x700000, 0) to 9, without 6). The class name is descriptive: the game keeps
 * none. */
struct GameWorld {
    GameWorld();
    EventManager lifetime;
    GameObjectManager registry;
    ResourceOwner animationOwner;
    RegisteredValue points[4];
    OwnedModel model0;
    OwnedModel model1;
    OwnedModel model2;
    OwnedModel model3;
    OwnedModel model4;
    OwnedModel model5;
    OwnedModel model7;
    OwnedModel model8;
    OwnedModel model9;
};
#endif
