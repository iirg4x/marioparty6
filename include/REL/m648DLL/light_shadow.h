#ifndef REL_M648_LIGHT_SHADOW_H
#define REL_M648_LIGHT_SHADOW_H
#include "REL/m648DLL/list_object.h"
#include "REL/framework/minigame_singleton.h"
#include "REL/framework/model.h"
struct CallbackModelOwner : RequestObject, MinigameSingleton<CallbackModelOwner> {
    OwnedModel modelId;
    OwnedModel shadowModelId;
    CallbackModelOwner(bool shadow);
    virtual ~CallbackModelOwner();
    virtual void clearRequest(int request);
    virtual void update();
};
#endif
