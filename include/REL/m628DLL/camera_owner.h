#ifndef REL_M628DLL_CAMERA_OWNER_H
#define REL_M628DLL_CAMERA_OWNER_H
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "REL/framework/camera_work.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event_hm.h"
#include "REL/framework/model.h"
typedef struct CameraOwner CameraOwner;
void fn_1_13A98(CameraOwner *, f32);
typedef void (CameraOwner::*CameraMethod)(void);

// Owns the scene camera, its model and movement-event state.
struct CameraOwner : GameObject
{
    CameraOwner();
    virtual ~CameraOwner();
    virtual void update();
    CameraWork camera;
    CameraWork cameraCopy; // Saved camera parameters used by the movement state.
    OwnedModel cameraModel;
    f32 elapsed;
    s32 state;
    CameraMethod callback; // Runs the current camera interpolation state.
    // Selects the current update callback.
    void setCallback(CameraMethod value)
    {
        callback = value;
    }
    EventHandlerManager callbackStorage; // Receives camera movement events.
    inline void fn_1_13788(void);
    void fn_1_137DC(void);
    void fn_1_1397C(const EventPacket &);
};

#endif
