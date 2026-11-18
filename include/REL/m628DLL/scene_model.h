#ifndef REL_M628DLL_SCENE_MODEL_H
#define REL_M628DLL_SCENE_MODEL_H
#include "dolphin/types.h"
#include "REL/framework/model.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event_hm.h"
#include <memory>
// Owns a scene model and applies movement events to its animation time.
struct SceneModelObject : GameObject
{
    SceneModelObject(OwnedModel *model, u8 mode);
    virtual ~SceneModelObject();
    virtual void update();
    std::auto_ptr<OwnedModel> model;
    Vec position;
    u8 mode; // Selects horizontal x movement when 1, otherwise z movement.
    f32 motionTime;
    EventHandlerManager callbacks; // Receives the object movement and state events.
    void fn_1_13F04(const EventPacket &);
};
#endif
