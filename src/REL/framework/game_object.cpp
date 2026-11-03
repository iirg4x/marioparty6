// Registers minigame objects with the active manager and unlinks them on destruction.
static const char rcsid[] = "$Id: game_object.cpp,v 1.21 2004/09/09 12:24:38 saf Exp $";

#include "REL/framework/game_object_man.h"

// Register the object and its category while a derived game object is being constructed.
GameObject::GameObject(int category) : category(category)
{
    GameObjectManager::getChecked().add(this);
}

// On object destruction, the base node destructor unlinks it from its circular list.
GameObject::~GameObject() {}
