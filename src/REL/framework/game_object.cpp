// Registers minigame objects with the active manager and unlinks them on destruction.
static const char rcsid[] = "$Id: game_object.cpp,v 1.21 2004/09/09 12:24:38 saf Exp $";

#include "REL/framework/game_object.h"

extern "C" {
#include "REL/framework/assertion.h"
}

void operator delete(void *);

// Receives game objects as their base constructors run.
struct GameObjectManager {
    void add(GameObject *object);
};

// Active manager used by newly constructed objects; null means none is available.
extern GameObjectManager *gameObjectManager;

// Obtain the manager during object construction, aborting if no manager is active.
inline GameObjectManager *currentGameObjectManager()
{
    gameObjectManager != 0 ? (void)0 :
        __msl_assertion_failed("self != 0", "singleton.h", 38);
    return gameObjectManager;
}

// Register the object and its category while a derived game object is being constructed.
GameObject::GameObject(int category) : category(category)
{
    currentGameObjectManager()->add(this);
}

// On object destruction, the base node destructor unlinks it from its circular list.
GameObject::~GameObject() {}
