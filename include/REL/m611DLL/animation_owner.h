#ifndef REL_M611DLL_ANIMATION_OWNER_H
#define REL_M611DLL_ANIMATION_OWNER_H
#include "dolphin/types.h"
#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"

// Owns the animation shared by the positioned particles.
struct AnimationOwner : RequestObject, MinigameSingleton<AnimationOwner> {
    AnimationOwner();
    virtual ~AnimationOwner();
    virtual void flagEnabled(int);
    virtual void update();
};

#endif
