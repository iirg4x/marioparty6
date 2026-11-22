// Adapts a scene camera method to the event callback interface.

#ifndef REL_M673DLL_BOUND_CAMERA_EVENT_H
#define REL_M673DLL_BOUND_CAMERA_EVENT_H

#include <functional>
#include "REL/framework/event_callback_inline.h"
#include "REL/framework/event_hm.h"
#include "REL/framework/event_callback_adapter_inline.h"

struct SceneCamera;
typedef void (SceneCamera::*CameraEventCallback)(const EventPacket &);

// Adapts a camera member function to the event interface.
struct CameraEventMethod {
    CameraEventCallback callback; // Camera member that receives the event.
    // Stores the camera member function.
    explicit CameraEventMethod(CameraEventCallback function) : callback(function)
    {
    }
};

// Pairs a camera event method with its receiver.
struct BoundCameraEvent {
    CameraEventMethod function; // Event method bound to this receiver.
    SceneCamera *receiver; // Camera that receives the event.
    // Stores the event method and the camera that receives it.
    BoundCameraEvent(const CameraEventMethod &callback, SceneCamera *owner)
        : function(callback), receiver(owner)
    {
    }

    // Dispatches the event to the camera.
    void operator()(const EventPacket &event) const
    {
        SceneCamera *owner = receiver;
        (owner->*function.callback)(event);
    }
};

#endif
