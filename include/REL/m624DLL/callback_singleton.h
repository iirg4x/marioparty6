#ifndef M624_CALLBACK_SINGLETON_H
#define M624_CALLBACK_SINGLETON_H
#include "dolphin/types.h"
#include "REL/framework/singleton.h"
#include "REL/framework/list_object.h"

class HandleAllocator;

// Keeps the render objects' handle allocator, and starts, updates and finishes the render objects
// with the game.
struct RegisteredCallbackOwner : RequestObject, MinigameSingleton<RegisteredCallbackOwner> {
    HandleAllocator *registeredNode;

    RegisteredCallbackOwner(HandleAllocator *allocator);
    virtual ~RegisteredCallbackOwner();
    virtual void flagEnabled(int bit);
    virtual void update();
};
#endif
