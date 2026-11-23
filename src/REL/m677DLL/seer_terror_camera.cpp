// Updates the board camera pose and its shake offset.

#include <math.h>
#include "REL/framework/world_units.h"
#include "REL/m677DLL/seer_terror_camera.h"
#include "REL/m677DLL/game_constants.h"
#include "REL/framework/random.h"

extern "C" {
#include "game/hu3d.h"
}

// Camera poses for the stages of the board sequence.
static const CameraPose cameraPresets[] = {
    { { -0.569805f, 445.75763f, 707.7695f },
      { -0.121586494f, -0.0038053892f, -0.0004661458f, 0.9925735f } },
    { { 817.0f, 598.0f, 1075.0f }, { -0.19487625f, 0.37187445f, 0.080160916f, 0.9040502f } },
    { { -0.569805f, 445.75763f, 707.7695f },
      { -0.121586494f, -0.0038053892f, -0.0004661458f, 0.9925735f } },
};

// Sets the board camera's perspective and initial pose.
SeerTerrorCamera::SeerTerrorCamera()
    : GameObject(2), fromState(cameraPresets[0]), toState(fromState), cameraTransition(0.0f),
      shake(0.0f)
{

    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 45.0f, 10.0f, 5000.0f, 1.2f);
    Hu3DBGColorSet(0, 0, 0);
    set_camera_frame_from_pose(&camera, &fromState);
    set_camera_frame_from_pose(&debugCamera.frame, &cameraPresets[1]);
}

// Releases the controller's owned state.
SeerTerrorCamera::~SeerTerrorCamera()
{
}

// Advances the camera transition and applies the current shake.
void SeerTerrorCamera::update()
{
    updateTransition();
    updateShake();
    debugResetHook(&debugCamera.frame, 0, 1);
}

// Blends from the current camera pose to a supplied pose.
void SeerTerrorCamera::transitionToPose(const CameraPose *source, float duration)
{
    get_camera_pose_from_frame(&camera, &fromState);
    toState = *source;
    cameraTransition.set(0.0f, 1.0f, (long) secondsToFrames(duration));
}

// Blends from the current camera pose to a preset pose.
void SeerTerrorCamera::transitionToPreset(int sourceIndex, float duration)
{
    transitionToPose(&cameraPresets[sourceIndex], duration);
}

// Reports whether the camera transition has finished.
bool SeerTerrorCamera::isTransitionDone()
{
    return cameraTransition.isFinished();
}

// Eases the transition and applies the interpolated pose.
void SeerTerrorCamera::updateTransition()
{
    double dividedScale;
    double square;
    double scale;
    double offset[1];
    offset[0] = 1.0 - cos((double) cameraTransition.update() * M_PI);
    square = offset[0] * offset[0];
    scale = square;
    dividedScale = scale / 4.0;
    interpolate_camera_poses(&camera, &fromState, &toState, (float) dividedScale);
    cameraApplyToHud(&camera, 1);
}

// Starts a shake that decays to zero over the supplied duration.
void SeerTerrorCamera::startShake(float strength, float duration)
{
    float scaledStrength = 100.0f * strength;
    shake.set(scaledStrength, 0.0f, (long) secondsToFrames(duration));
}

// Applies the shake offset to the camera position.
void SeerTerrorCamera::updateShake()
{
    if (!shake.isFinished()) {
        double angle = RandomFloat((float) (2.0 * M_PI));
        Vec offset = { 0.0f, 0.0f, 0.0f };
        offset.x = (float) sin(angle);
        offset.y = (float) cos(angle);
        float scale = shake.update();
        PSVECScale(&offset, &offset, scale);
        CameraFrame view = camera;
        cameraTranslateByLocalOffset(&view, &offset);
        cameraApplyToHud(&view, 1);
    }
}
