#ifndef REL_M648_ANIMATION_OBJECT_H
#define REL_M648_ANIMATION_OBJECT_H
#include "REL/m648DLL/list_object.h"
#include "REL/framework/minigame_singleton.h"
struct AnimationUpdateOwner : RequestObject, MinigameSingleton<AnimationUpdateOwner> {
    AnimationUpdateOwner();
    virtual ~AnimationUpdateOwner();
    virtual void clearRequest(int bit);
    virtual void update();
};
#endif
