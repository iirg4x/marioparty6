#include "REL/m610DLL/scene_callback.h"
#include "REL/framework/event_packet.h"
#include "REL/framework/debug.h"
#include <vector>
#include "REL/framework/scene_shadow_object.h"
#include "REL/framework/camera_frame.h"
#include "REL/m610DLL/collision_node.h"

#include "REL/m610DLL/camera_transition.h"

/* Initial gameplay camera pose. */
const CameraPose lbl_1_rodata_70 = { { 0.0f, 3591.2285f, -229.67554f },
                                                { -0.40276793f, 0.0f, -0.0f, 0.91530216f } };
/* Camera pose used after a player finishes. */
const CameraPose lbl_1_rodata_8C = { { -39.54978f, 417.2931f, -3156.7214f },
                                                { -0.20790672f, 0.0f, -0.0f, 0.9781487f } };

/* Ease camera transition progress with a cubic smoothstep. */
float fn_1_C34(float progress)
{
    return (3.0f - 2.0f * progress) * (progress * progress);
}

/* Create the camera, shadow and initial transition state. */
CameraTransitionState::CameraTransitionState() : GameObject(0x8000)
{
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 30.0f, 1.0f, 32000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    cameraPerspective.fovRadians = MTXDegToRad(30.0f);
    cameraSetDefaultFrame(&frame);
    cameraTiltToAngle(&frame, -50.0f);
    shadowPerspective.fovRadians = MTXDegToRad(2.0f);
    Hu3DShadowCreate(2.0f, 1.0f, 1e+09f);
    Hu3DShadowSizeSet(0x1E0);
    fn_8002AAB4(0, 0, 0x40);
    Hu3DShadowTPLvlSet(0.3f);
    registerChannel(2);
    callback = &CameraTransitionState::fn_1_E38;
    elapsedFrames = 0;
    lbl_1_bss_64 = 0;
}

/* Blend the opening pose into the camera fitted to the players. */
void CameraTransitionState::fn_1_E38()
{
    CameraFrame *frame = &this->frame;
    CameraPose source;
    this->fn_1_F64();
    get_camera_pose_from_frame(frame, &source);

    float progress = this->elapsedFrames++ / 120.0f;
    if (progress > 1.0f)
    {
        progress = 1.0f;
        this->callback = &CameraTransitionState::fn_1_F64;
    }

    progress = fn_1_C34(progress);
    interpolate_camera_poses(frame, &lbl_1_rodata_70, &source, progress);
}

/* Fit and apply the shadow camera around the registered spheres. */
static inline void updateShadowCamera(CameraFrame &lightFrame, GXColor &color,
                                      CameraPerspective &perspective, ShadowSpheres &spheres)
{
    fn_800297D4(0, &lightFrame.position, &lightFrame.target, &color);
    lightFrame.position.x = 0.0f;
    lightFrame.position.y = 0.0f;
    lightFrame.position.z = 0.0f;
    lightFrame.up.x = 0.0f;
    lightFrame.up.y = 1.0f;
    lightFrame.up.z = 0.0f;
    fitCameraToSpheres(&lightFrame, &perspective, &spheres);
    cameraApplyToShadow(&lightFrame);
}

/* Fit the gameplay camera and shadow to the registered spheres. */
void CameraTransitionState::fn_1_F64()
{
    CameraFrame *frame = &this->frame;
    CameraFrame lightFrame;
    ShadowSpheres spheres;
    GXColor color;

    collectCategoryValues(&spheres, 0);
    fitCameraToSpheres(frame, &this->cameraPerspective, &spheres);
    updateShadowCamera(lightFrame, color, shadowPerspective, spheres);
    if (lbl_1_bss_64 == 1)
        set_camera_frame_from_pose(frame, &lbl_1_rodata_8C);
}

/* Blend the unwritten destination camera pose into the ending pose. */
void CameraTransitionState::fn_1_1240()
{
    float progress;
    CameraFrame *frame;

    frame = &this->frame;
    progress = this->elapsedFrames++ / 120.0f;
    if (progress > 1.0f)
        progress = 1.0f;
    progress = fn_1_C34(progress);
    interpolate_camera_poses(frame, &this->destination, &lbl_1_rodata_8C, progress);
}

/* Advance the camera callback, apply debug input and update the shadow. */
void CameraTransitionState::update()
{
    CameraFrame lightFrame;
    GXColor color;
    CameraFrame *current;
    current = &frame;
    (this->*callback)();
    if (debug.highFlag == 0)
    {
        cameraApplyToHud(current, 1);
        copyDebugCameraFrame(&debug.frame, current);
    }
    else
    {
        *current = debug.frame;
    }
    debugResetHook(&debug.frame, 0, 1);
    ShadowSpheres spheres;
    collectCategoryValues(&spheres, 0);
    updateShadowCamera(lightFrame, color, shadowPerspective, spheres);
}

/* Ignore events delivered to this camera listener. */
void CameraTransitionState::receive(const EventPacket *)
{
}
/* Release the camera listener and game object bases. */
CameraTransitionState::~CameraTransitionState()
{
}
