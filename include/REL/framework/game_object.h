#ifndef REL_FRAMEWORK_GAME_OBJECT_H
#define REL_FRAMEWORK_GAME_OBJECT_H

// Game objects carry a collection link and the category used by their manager.
#include "REL/framework/linknode.h"

struct GameObjectNode : Node {
    ~GameObjectNode() {}
};

struct GameObject : GameObjectNode {
    virtual ~GameObject();
    virtual void update() = 0;
    int category; // Selects the manager's collection for this object.
    explicit GameObject(int category);
};

#endif
