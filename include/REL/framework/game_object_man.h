// The manager that keeps live game objects in category groups and updates them each frame.
#ifndef REL_FRAMEWORK_GAME_OBJECT_MAN_H
#define REL_FRAMEWORK_GAME_OBJECT_MAN_H

#include <map>
#include <vector>

#include "REL/framework/game_object.h"
#include "REL/framework/singleton.h"

// The list head of one category. Objects link in through their own node, so the head holds no
// object and doubles as the end position of the list.
struct GameObjectGroup : Node {
    GameObjectGroup() {}
    // The manager empties a group before deleting it; the node destructor does the rest.
    ~GameObjectGroup() {}
    Node *getNext() { return link0; }
    Node *getPrevious() { return link4; }
    // The oldest object, or end() when the group is empty.
    GameObject *begin() { return static_cast<GameObject *>(getNext()); }
    // The newest object; the group must not be empty.
    GameObject *back() { return static_cast<GameObject *>(getPrevious()); }
    // The head itself is the position after the last object.
    GameObject *end() { return reinterpret_cast<GameObject *>(this); }
    // A head linked to itself has no objects.
    bool empty() const { return link0 == this && link0 == link4; }
    // The list keeps no count, so the number of objects comes from walking it.
    int size()
    {
        int count = 0;
        GameObject *object = begin();
        while (object != end()) {
            ++count;
            object = object->next();
        }
        return count;
    }
};

// Owns the groups of live objects and the hooks that run around each group's update.
class GameObjectManager : public MinigameSingleton<GameObjectManager> {
public:
    typedef std::map<int, GameObjectGroup *> GroupMap;
    typedef std::map<int, void (*)()> HookMap;
    GroupMap groups;      // One group per category, created by the category's first object.
    HookMap preUpdate;    // Per category, called before the group's objects update.
    HookMap postUpdate;   // Per category, called after the group's objects update.
    GameObjectManager();
    ~GameObjectManager();
    void add(GameObject *object);
    void updateGroup(const GroupMap::value_type &entry);
};

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
