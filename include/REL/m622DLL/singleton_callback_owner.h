#ifndef REL_M622DLL_SINGLETON_CALLBACK_OWNER_H
#define REL_M622DLL_SINGLETON_CALLBACK_OWNER_H

#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"

struct SingletonCallbackOwner : RequestObject, MinigameSingleton<SingletonCallbackOwner> {
    SingletonCallbackOwner();
    virtual ~SingletonCallbackOwner();
    virtual void flagEnabled(int bit);
    virtual void update();
};

#endif
