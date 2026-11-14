#include "REL/m622DLL/singleton_callback_owner.h"

/* Initializes the object through its member and base constructors. */
SingletonCallbackOwner::SingletonCallbackOwner()
{
}

/* Releases the object through its member and base destructors. */
SingletonCallbackOwner::~SingletonCallbackOwner()
{
}

/* Clears the requested flag. */
void SingletonCallbackOwner::flagEnabled(int bit)
{
    flags &= ~(1U << bit);
}

/* Returns without changing state. */
void SingletonCallbackOwner::update()
{
}
