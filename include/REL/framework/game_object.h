#ifndef REL_FRAMEWORK_GAME_OBJECT_H
#define REL_FRAMEWORK_GAME_OBJECT_H

// Game objects carry a collection link and the category used by their manager.
#include "REL/framework/list.h"

struct GameObject : ListNode<GameObject> {
    virtual ~GameObject();
    virtual void update() = 0;
    int category; // Selects the manager's collection for this object.
    explicit GameObject(int category);
    // The category the manager files this object under.
    int getCategory() const { return category; }
};

#endif
