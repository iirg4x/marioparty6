#ifndef REL_M611DLL_MOTION_EFFECT_OWNER_H
#define REL_M611DLL_MOTION_EFFECT_OWNER_H
#include "REL/framework/model.h"
#include "dolphin/types.h"
#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"
// Owns the shared model and callbacks for the moving model instances.
struct MotionEffectOwner : RequestObject, MinigameSingleton<MotionEffectOwner> {
    MotionEffectOwner();
    virtual ~MotionEffectOwner();
    virtual void flagEnabled(int);
    virtual void update();
    OwnedModel model;
};

void fn_1_2380();

void fn_1_2294();

#endif
