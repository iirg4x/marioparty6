// Defines the board camera's pose and shake transitions.

#ifndef REL_SEER_TERROR_CAMERA_H
#define REL_SEER_TERROR_CAMERA_H

#include "REL/framework/camera_frame.h"
#include "REL/framework/debug.h"
#include "REL/framework/frame_interpolator.h"
#include "REL/framework/game_object.h"
#include "REL/framework/singleton.h"

// Blends board camera poses and offsets the view when it shakes.
class SeerTerrorCamera : public MinigameSingleton<SeerTerrorCamera>, public GameObject {
public:
    SeerTerrorCamera();
    virtual ~SeerTerrorCamera();
    virtual void update();
    void transitionToPose(const CameraPose *source, float duration);
    void transitionToPreset(int sourceIndex, float duration);
    bool isTransitionDone();
    void updateTransition();
    void startShake(float strength, float duration);
    void updateShake();

    // Current base camera pose before the shake offset is applied.
    CameraFrame camera;
    // Pose at the start of the current blend.
    CameraPose fromState;
    // Pose the current blend approaches.
    CameraPose toState;
    // Progress through the current pose blend.
    FrameInterpolator<float> cameraTransition;
    // Shake strength that decays to zero.
    FrameInterpolator<float> shake;
    CameraSetup debugCamera;
};

#endif
