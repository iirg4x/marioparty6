#include "REL/m622DLL/singleton_callback_owner.h"

SingletonCallbackOwner::SingletonCallbackOwner()
{
}

SingletonCallbackOwner::~SingletonCallbackOwner()
{
}

void SingletonCallbackOwner::clearFlag(int bit)
{
    flags &= ~(1U << bit);
}

void SingletonCallbackOwner::update()
{
}
