// The effect shown at the start: a model and its particles that slide in and leave.
#ifndef REL_M604DLL_CAMERA_EFFECT_OWNER_H
#define REL_M604DLL_CAMERA_EFFECT_OWNER_H

#include <memory>
#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"
extern "C" {
#include "game/hu3d.h"
}
#include "REL/framework/model.h"
#include "REL/m604DLL/particle_owner.h"

struct CameraEffectOwner : RequestObject, MinigameSingleton<CameraEffectOwner> {
    std::auto_ptr<OwnedModel> model;
    std::auto_ptr<ModelParticles> particle;
    Vec position;
    Vec rotation;
    float time;

    CameraEffectOwner();
    virtual ~CameraEffectOwner();
    virtual void flagEnabled(int bit);
    virtual void update();
    Vec getPosition() const;
};

#endif
