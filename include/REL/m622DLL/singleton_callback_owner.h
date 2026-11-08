#ifndef REL_M622DLL_SINGLETON_CALLBACK_OWNER_H
#define REL_M622DLL_SINGLETON_CALLBACK_OWNER_H

#include "REL/m622DLL/game_object.h"
#include "REL/framework/minigame_singleton.h"

struct SingletonCallbackOwner : GameObject, MinigameSingleton<SingletonCallbackOwner> {
    SingletonCallbackOwner();
    virtual ~SingletonCallbackOwner();
    virtual void clearFlag(int bit);
    virtual void update();
};

#endif
