#ifndef REL_M672DLL_SINGLETON_CALLBACK_OWNER_H
#define REL_M672DLL_SINGLETON_CALLBACK_OWNER_H
#include "REL/framework/game_object.h"
#include "REL/framework/singleton.h"

class SingletonCallbackOwner : public GameObject, public MinigameSingleton<SingletonCallbackOwner> {
public:
    SingletonCallbackOwner();
    virtual ~SingletonCallbackOwner();
    virtual void update();
};

#endif
