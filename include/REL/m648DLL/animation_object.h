#ifndef REL_M648_ANIMATION_OBJECT_H
#define REL_M648_ANIMATION_OBJECT_H
#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"
struct AnimationUpdateOwner : RequestObject, MinigameSingleton<AnimationUpdateOwner> {
    AnimationUpdateOwner();
    virtual ~AnimationUpdateOwner();
    virtual void flagEnabled(int bit);
    virtual void update();
};
#endif
