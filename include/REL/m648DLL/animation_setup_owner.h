#ifndef REL_M648_ANIMATION_SETUP_OWNER_H
#define REL_M648_ANIMATION_SETUP_OWNER_H
#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"
#include "REL/framework/model.h"

class AnimationSetupOwner : public RequestObject, public MinigameSingleton<AnimationSetupOwner> {
public:
    OwnedModel model0, model1, model2;
    AnimationSetupOwner();
    virtual ~AnimationSetupOwner();
    virtual void flagEnabled(int request);
    virtual void update();
};
#endif
