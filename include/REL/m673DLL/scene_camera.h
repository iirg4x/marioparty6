// Declares the scene camera and its update states.

#ifndef REL_M673DLL_SCENE_CAMERA_H
#define REL_M673DLL_SCENE_CAMERA_H
#include "REL/framework/camera_work.h"
#include "REL/framework/game_object.h"
#include "REL/framework/model.h"
#include "REL/framework/debug.h"
#include "REL/framework/event_hm.h"
#include "REL/m673DLL/bound_camera_event.h"

// Skips the camera vector check during play.
static inline void ReleaseCheck()
{
}

// Returns the current camera vector.
inline const Vec &CameraVector::getValue() const
{
    ReleaseCheck();
    return value;
}

// Controls the scene camera frame and its event response.
struct SceneCamera : GameObject {
    typedef void (SceneCamera::*State)();

    OwnedModel model; // Model carrying the camera animation.
    CameraWork camera; // Current position and target for the scene camera.
    CameraWork cameraCopy; // Saved camera frame used as the interpolation target.
    CameraSetup debugCamera; // Camera frame supplied to the debug controls.
    f32 fade; // Blend weight applied during the camera transition.
    EventHandlerManager channel1Events; // Camera shake event handler.
    State state; // Current camera update.
    // Sets the initial camera frame and registers its scene event.
    SceneCamera();
    // Releases the event handlers and camera vector state.
    virtual ~SceneCamera();
    // Runs the current camera callback and applies the camera frames.
    virtual void update();
    // Selects the callback to run on subsequent updates.
    void setState(State method)
    {
        state = method;
    }

    // Moves the camera position and target toward their desired values.
    void fn_1_C6E4(f32 amount);
    // Starts the camera shake callback.
    void fn_1_C690(const EventPacket &);
    // Applies a decaying random camera offset and restores the saved camera state.
    void fn_1_C454();
    // Leaves the camera frame unchanged while idle.
    void fn_1_C450();
};

#endif
