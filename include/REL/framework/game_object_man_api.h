// Game-object queries and update hooks, independent of the manager singleton storage.
#ifndef REL_FRAMEWORK_GAME_OBJECT_MAN_API_H
#define REL_FRAMEWORK_GAME_OBJECT_MAN_API_H

#include <vector>
#include "REL/framework/game_object.h"

// Update every group, in category order, with its hooks.
void updateGameObjects();
// Install a category's hook; a null hook removes it.
void setPreUpdateHook(int category, void (*hook)());
void setPostUpdateHook(int category, void (*hook)());
// The number of objects in a category; zero when the category has no group.
int countGameObjects(int category);
// Replace the list's contents with the category's objects.
void collectGameObjects(int category, std::vector<GameObject *> *list);

#endif
