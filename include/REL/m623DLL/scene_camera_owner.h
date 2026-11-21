#ifndef REL_M623DLL_SCENE_CAMERA_OWNER_H
#define REL_M623DLL_SCENE_CAMERA_OWNER_H
#include "REL/framework/game_object.h"
#include "REL/framework/singleton.h"
#include "REL/framework/camera_frame.h"
#include "REL/framework/frame_interpolator.h"
#include "REL/framework/debug.h"

// Blends the scene camera and the two player views.
class SceneCamera : public GameObject, public MinigameSingleton<SceneCamera> {
  public:
    typedef void (SceneCamera::*State)();

    SceneCamera();
    virtual ~SceneCamera();
    virtual void update();
    // Selects the camera update callback.
    void setState(State next)
    {
        state = next;
    }
    // Tracks the intro motion until it finishes.
    void fn_1_C2E0();
    // Fits the cameras to the players and applies the active blends.
    void fn_1_C444();
    // Holds the completed camera state.
    void fn_1_C898();

    // Current camera update callback.
    State state;
    // The player camera for view index 0.
    CameraFrame cameraOne;
    // The player camera for view index 1.
    CameraFrame cameraTwo;
    // Camera frame at the start of the split-view transition.
    CameraFrame introView;
    // Shared scene camera and its projection setup.
    CameraSetup view;
    // Blend progress from the introduction to the player views.
    FrameInterpolator<f32> introTimer;
    // Blend progress after the team camera target changes.
    FrameInterpolator<f32> blendTimer;
    // Team camera frame saved at the start of the current blend.
    CameraFrame blendStart;
};
#endif
