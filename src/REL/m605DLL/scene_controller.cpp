#include "REL/minigame_value_bounds.h"
#include "REL/framework/registered_value.h"
#include "REL/framework/sound.h"
#include "REL/framework/debug.h"
#include "REL/framework/camera_frame.h"
#include "REL/framework/scene_shadow_object.h"
#include "REL/framework/random.h"
#include <cmath>
#include <vector>
#include "dolphin/types.h"
#include "dolphin/mtx.h"
extern "C" {
#include "game/hu3d.h"
}
#include "REL/m605DLL/scene_controller.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/sphere_type.h"

// Controls the scene camera, shadow and transitions timed to the song.
using std::sin;

// Initializes the camera, shadow and scene transition timers.
SceneController::SceneController()
{
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 45.0f, 100.0f, 5000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    perspective.fovRadians = (float) M_PI / 4.0f;
    Hu3DShadowCreate(10.0f, 100.0f, 20000.0f);
    Hu3DShadowSizeSet(480);
    fn_8002AAB4(0, 0, 0x20);
    Hu3DShadowTPLvlSet(0.6f);
    savedPerspective.fovRadians = (float) (M_PI / 18.0);
    cameraSetDefaultFrame(&cameraWork);
    cameraTiltToAngle(&cameraWork, -6.0f);
    thresholdReached = 0;
    elapsedThresholdReached = 0;
    transitionFrame = 0;
    secondTransition.set(0.0f, -1);
    registerChannel(1);
}

// Starts a camera noise transition for the requested duration.
void fn_1_23DC(SceneController *camera, f32 value, f32 duration)
{
    camera->noiseTransition.set(value, (s32)secondsToFrames(duration));
}

// Resets the initial camera transition and clears the completed step flag.
void SceneController::flagEnabled(int step)
{
    if (step == 0) {
        thresholdReached = 0;
        transitionFrame = 0;
    }
    flags &= ~(1UL << step);
}

// Limits camera height according to progress along the course.
void fn_1_2458(Vec *position)
{
    f32 limit;
    f32 z = position->z;
    if (z < 36275.0f) {
        limit = 695.0f;
    } else if (z < 37154.0f) {
        f32 rate = (z - 36275.0f) / 879.0f;
        limit = 695.0f * (1.0f - rate) + 110.0f * rate;
    } else {
        limit = 110.0f;
    }
    position->y = minimumValue(position->y, limit);
}

// Fits and animates the camera, applies noise, then submits its frame.
void SceneController::update()
{
    static const CameraPose endPose = {
        { 15.661986f, -234.67691f, 50343.723f },
        { -0.14653616f, 0.0f, -0.0f, 0.98920536f }
    };
    CameraFrame *work;
    f32 maximum;
    f32 depth;
    f32 transitionRatio;
    f32 rate;
    f32 height;
    f32 heightLimit;
    f32 clampedHeight;
    f32 transitionProgress;
    f64 randomZ;
    f64 randomY;
    f64 randomX;
    f32 finalHeight;
    f32 cameraHeight;
    f32 transitionElapsed;
    f32 shakeSize;
    work = &cameraWork;
    if (!elapsedThresholdReached) {
        elapsedThresholdReached = work->position.z > 36500.0f;
        if (elapsedThresholdReached != 0) {
            transition.set(0.0f, 0);
            transition.set((float) M_PI, (s32)secondsToFrames(3.8f));
        }
    }
    if (transition.elapsed <= transition.duration) {
        cameraTiltToAngle(work, (f32)(-6.0 + 3.0 * sin(transition.update())));
    }
    std::vector<Sphere *> intermediate;
    collectCategoryValues(&intermediate, 0);
    fitCameraToSpheres(work, &perspective, &intermediate);
    depth = work->position.z;
    if (depth < 36275.0f) {
        maximum = 695.0f;
    } else if (depth < 37154.0f) {
        rate = (depth - 36275.0f) / 879.0f;
        maximum = 695.0f * (1.0f - rate) + 110.0f * rate;
    } else {
        maximum = 110.0f;
    }
    heightLimit = maximum;
    height = work->position.y;
    if (!(!(height < heightLimit))) {
        clampedHeight = height;
    } else {
        clampedHeight = heightLimit;
    }
    finalHeight = clampedHeight;
    cameraHeight = finalHeight;
    work->position.y = cameraHeight;
    lbl_1_bss_38 = cameraWork.position.z;
    if (!thresholdReached) {
        thresholdReached = lbl_1_bss_38 >= 48900.0f;
        if (thresholdReached) {
            get_camera_pose_from_frame(work, &savedPose);
        }
    } else {
        transitionFrame++;
        transitionRatio = (f32)transitionFrame / secondsToFrames(1.5f);
        if (transitionRatio < 1.0f) {
            transitionProgress = transitionRatio;
        } else {
            transitionProgress = 1.0f;
        }
        transitionElapsed = transitionProgress;
        interpolate_camera_poses(work, &savedPose, &endPose, transitionElapsed);
    }
    if (secondTransition.elapsed <= secondTransition.duration) {
        interpolate_camera_frames(work, &savedCamera, work, secondTransition.update());
    }
    shakeSize = noiseTransition.update();
    if (shakeSize > 0.0f) {
        Vec noise;
        randomX = RandomFloat(-shakeSize, shakeSize);
        randomY = RandomFloat(-shakeSize, shakeSize);
        randomZ = RandomFloat(-shakeSize, shakeSize);
        noise.x = (f32)randomZ;
        noise.y = (f32)randomY;
        noise.z = (f32)randomX;
        cameraTranslateByLocalOffset(work, &noise);
    }
    cameraApplyToHud(work, 1);
    if (!cameraOutput.highFlag) {
        copyDebugCameraFrame(&cameraOutput.frame, work);
    }
    debugResetHook(&cameraOutput.frame, 1, 1);
}

// Starts the camera shake and sound for the handled scene events.
void SceneController::receive(const EventPacket *packet)
{
    switch (packet->type) {
    case 0:
    case 3:
        fn_1_23DC(this, 30.0f, 0.0f);
        fn_1_23DC(this, 0.0f, 1.5f);
        soundPlay(0x63E);
    }
}

// Saves the current camera frame and starts its interpolation timer.
void fn_1_2CB4()
{
    SceneController *self = &SceneController::getChecked();

    self->secondTransition.set(0.0f, 1.0f, (s32)secondsToFrames(0.8f));
    self->savedCamera = self->cameraWork;
}

// Destroys the scene controller and its member state.
SceneController::~SceneController() {}
