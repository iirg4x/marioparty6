#ifndef REL_M648_LIGHT_SHADOW_H
#define REL_M648_LIGHT_SHADOW_H
#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"
#include "REL/framework/model.h"
struct CallbackModelOwner : RequestObject, MinigameSingleton<CallbackModelOwner> {
    OwnedModel modelId;
    OwnedModel shadowModelId;
    CallbackModelOwner(bool shadow);
    virtual ~CallbackModelOwner();
    virtual void flagEnabled(int request);
    virtual void update();
};
#endif
