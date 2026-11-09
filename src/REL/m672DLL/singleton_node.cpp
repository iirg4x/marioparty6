#include "REL/framework/game_object.h"
extern "C" {
#include "REL/framework/assertion.h"
}
void operator delete(void *);

#include "REL/m672DLL/singleton_callback_owner.h"

SingletonCallbackOwner::SingletonCallbackOwner() : GameObject(4) {}

SingletonCallbackOwner::~SingletonCallbackOwner() {}

void SingletonCallbackOwner::update() {}
