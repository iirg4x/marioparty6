// Keeps game objects in keyed groups and runs each group's update with its hooks.
#include <algorithm>
#include <functional>

#include "REL/framework/game_object_man.h"

static const char rcsid[] = "$Id: game_object_man.cpp,v 1.21 2004/09/09 12:24:38 saf Exp $";

static void forEachGameObject(GameObjectGroup *group, void (*callback)(GameObject *));

// The minigame constructs the manager before its first game object.
GameObjectManager::GameObjectManager() {}

// Delete each group's remaining objects, then the group. An object's destructor unlinks it.
GameObjectManager::~GameObjectManager()
{
    GroupMap::iterator position = groups.begin();
    GroupMap::iterator finish = groups.end();
    for (; position != finish; ++position) {
        GameObjectGroup *group = (*position).second;
        while (!group->empty()) delete group->back();
        delete (*position).second;
    }
}

// Append a newly constructed object to its category's group, creating the group on first use.
void GameObjectManager::add(GameObject *object)
{
    GroupMap::iterator position = groups.find(object->getCategory());
    if (position == groups.end()) {
        position = groups.insert(GroupMap::value_type(object->getCategory(), new GameObjectGroup())).first;
    }
    GameObjectGroup *group = (*position).second;
    object->insert4(group);
}

// The per-object step of a group's update.
static void updateGameObject(GameObject *object) { object->update(); }

// Update one non-empty group between its category's hooks.
void GameObjectManager::updateGroup(const GroupMap::value_type &entry)
{
    int category = entry.first;
    if (countGameObjects(category) == 0) return;
    if (preUpdate.count(category) != 0) preUpdate[category]();
    forEachGameObject(entry.second, updateGameObject);
    if (postUpdate.count(category) != 0) postUpdate[category]();
}

// Each frame's object update: every group in category order.
void updateGameObjects()
{
    GameObjectManager &manager = GameObjectManager::getChecked();
    std::for_each(manager.groups.begin(), manager.groups.end(),
                  std::bind1st(std::mem_fun(&GameObjectManager::updateGroup), &manager));
}

// Install or replace the hook run before a category's update; a null hook removes it.
void setPreUpdateHook(int category, void (*hook)())
{
    GameObjectManager *manager = &GameObjectManager::getChecked();
    if (hook == 0) {
        manager->preUpdate.erase(category);
        return;
    }
    if (manager->preUpdate.count(category) == 0) {
        manager->preUpdate.insert(std::make_pair(category, hook));
        return;
    }
    manager->preUpdate[category] = hook;
}

// Install or replace the hook run after a category's update; a null hook removes it.
void setPostUpdateHook(int category, void (*hook)())
{
    GameObjectManager *manager = &GameObjectManager::getChecked();
    if (hook == 0) {
        manager->postUpdate.erase(category);
        return;
    }
    if (manager->postUpdate.count(category) == 0) {
        manager->postUpdate.insert(std::make_pair(category, hook));
        return;
    }
    manager->postUpdate[category] = hook;
}

// Count a category's objects; a category with no group has none.
int countGameObjects(int category)
{
    GameObjectManager &manager = GameObjectManager::getChecked();
    GameObjectManager::GroupMap::iterator position = manager.groups.find(category);
    if (position == manager.groups.end()) return 0;
    return (*position).second->size();
}

// Fill the caller's list with a category's objects, in group order.
void collectGameObjects(int category, std::vector<GameObject *> *list)
{
    list->clear();
    GameObjectManager &manager = GameObjectManager::getChecked();
    GameObjectManager::GroupMap::iterator position = manager.groups.find(category);
    if (position != manager.groups.end()) {
        GameObjectGroup *group = (*position).second;
        GameObject *object = group->begin();
        GameObject *end = group->end();
        while (object != end) {
            list->push_back(object);
            object = object->next();
        }
    }
}

// Run the callback over a group, moving each object back as it runs. Objects added or destroyed
// by the callback change the group, not the pending list being walked.
static void forEachGameObject(GameObjectGroup *group, void (*callback)(GameObject *))
{
    if (!group->empty()) {
        GameObjectGroup pending;
        group->moveAllTo(&pending);
        while (!pending.empty()) {
            GameObject *object = pending.begin();
            object->unlink();
            object->insert4(group);
            callback(object);
        }
    }
}
