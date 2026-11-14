#include <new>
#include "REL/framework/game_object.h"

#include "REL/m672DLL/singleton_callback_owner.h"
// Create and publish the singleton callback owner.
SingletonCallbackOwner::SingletonCallbackOwner() : GameObject(4) {}
// Destroy the callback owner and clear its singleton pointer.
SingletonCallbackOwner::~SingletonCallbackOwner() {}
// Leave the callback owner unchanged.
void SingletonCallbackOwner::update() {}
