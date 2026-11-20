#include "datadir_enum.h"
#include "REL/m611DLL/player_state.h"
#include "REL/m611DLL/scene_camera.h"
#include "REL/framework/seconds_to_frames_inline.h"
#include "REL/framework/debug.h"
#include "REL/m611DLL/Sphere.h"
#include "REL/framework/scene_shadow_object.h"
#include "REL/framework/camera_frame.h"
#include <vector>
extern "C" {
#include "game/hsfex.h"
#include "game/audio.h"
#include "game/hu3d.h"
#include "dolphin/types.h"
#include "dolphin/mtx.h"
}

using std::sqrt;

// Creates the camera and installs its initial pose and update callback.
SceneCamera::SceneCamera()
    : updateCallback(&SceneCamera::fn_1_30F0), targetPlayer(0), cameraModelId(-1)
{
    // Places the camera above the scene and points it down toward the play area.
    static const CameraPose startPose = {
        { 0.0f, 2998.029f, 2868.291f },
        { -0.40940636f, 0.0f, -0.0f, 0.9123522f }
    };
    Hu3DCameraCreate(1);
    Hu3DBGColorSet(0, 0, 0);
    set_camera_frame_from_pose(&this->cameraView.frame, &startPose);
}

// Releases the camera owner and its inherited registrations.
SceneCamera::~SceneCamera()
{
}

// Reads the active camera motion and resumes tracking when it finishes.
void SceneCamera::fn_1_2D8C()
{
    if (this->cameraModelId != -1) {
        cameraReadFromHud(&this->camera, 1);
        if (fn_8002D75C(this->cameraModelId) != 0) {
            void (SceneCamera::*nextUpdate)() = &SceneCamera::fn_1_30F0;
            this->updateCallback = nextUpdate;
            this->flags &= ~(1UL << 0);
        }
    }
}

// Fits the camera around four spheres, one at each player.
void fn_1_2E20(SceneCamera *camera)
{
    CameraPerspective perspective;
    Sphere playerSpheres[4];
    Vec positions[4];

    std::vector<Sphere *> spheres;
    fn_1_CE90(positions);
    for (s32 index = 0; index < 4; ++index) {
        Sphere *playerSphere = &playerSpheres[index];
        playerSphere->moveTo(positions[index]);
        playerSphere->setRadius(500.0f);
        spheres.push_back(playerSphere);
    }
    fitCameraToSpheres(&camera->camera, &perspective, &spheres);
}

// Captures the current view and fits the new view around the four players.
void SceneCamera::fn_1_30F0()
{
    CameraPose previousTransform;
    CameraPose currentTransform;

    get_camera_pose_from_frame(&this->camera, &previousTransform);
    fn_1_2E20(this);
    get_camera_pose_from_frame(&this->camera, &currentTransform);
    interpolate_camera_poses(&this->camera, &previousTransform, &currentTransform, 0.05f);
    cameraApplyToHud(&this->camera, 1);
}

// Applies the player-fitted camera pose immediately.
void SceneCamera::fn_1_3244()
{
    fn_1_2E20(this);
    cameraApplyToHud(&this->camera, 1);
}

// After a delay, blends the camera toward the requested object position.
void SceneCamera::fn_1_3364()
{
    static const Vec positionOffset = { 0.0f, 900.0f, 700.0f };
    static const Vec targetOffset = { 0.0f, 200.0f, 0.0f };
    CameraFrame targetFrame;
    CameraPose currentTransform;
    CameraPose targetTransform;

    if (++this->frame < secondsToFrames(1.5f)) {
        return;
    }

    Vec *targetPosition = &this->targetPlayer->position;
    PSVECAdd(targetPosition, &positionOffset, &targetFrame.position);
    PSVECAdd(targetPosition, &targetOffset, &targetFrame.target);
    targetFrame.up.x = 0.0f;
    targetFrame.up.y = 1.0f;
    targetFrame.up.z = 0.0f;
    get_camera_pose_from_frame(&this->camera, &currentTransform);
    get_camera_pose_from_frame(&targetFrame, &targetTransform);
    interpolate_camera_poses(&this->camera, &currentTransform, &targetTransform, 0.05f);
    cameraApplyToHud(&this->camera, 1);
}

// Starts the delayed camera transition toward the requested object.
// Nothing in this minigame calls this.
void fn_1_3478(SceneCamera *camera, PlayerState *player)
{
    camera->frame = 0;
    camera->targetPlayer = player;
    void (SceneCamera::*nextUpdate)() = &SceneCamera::fn_1_3364;
    camera->updateCallback = nextUpdate;
}

// Dispatches camera requests and selects the corresponding update callback.
void SceneCamera::flagEnabled(int request)
{
    switch (request) {
    case 0: {
        fn_1_3660(this);
        void (SceneCamera::*nextUpdate)() = &SceneCamera::fn_1_2D8C;
        this->updateCallback = nextUpdate;
        break;
    }
    case 1:
        fn_1_36C8(this);
        flags &= ~(1UL << request);
        break;
    case 2: {
        void (SceneCamera::*nextUpdate)() = &SceneCamera::fn_1_3244;
        this->updateCallback = nextUpdate;
        flags &= ~(1UL << request);
        break;
    }
    case 3:
        flags &= ~(1UL << request);
        break;
    case 4:
        flags &= ~(1UL << request);
        break;
    }
}

// Runs the current camera update and resets its debug frame hook.
void SceneCamera::update()
{
    if (updateCallback) {
        (this->*updateCallback)();
    }
    debugResetHook(&cameraView.frame, 0, 1);
}

// Loads and starts the camera motion.
void fn_1_3660(SceneCamera *camera)
{
    s16 motionId = fn_8002C4C0(fn_80008DD0(DATANUM(0x460000, 5), 0x10000000, 2));
    camera->cameraModelId = fn_800283F4(static_cast<s32>(motionId), 1);
    fn_800284C8(camera->cameraModelId, 1);
}

// Stops the camera motion and clears its handle.
void fn_1_36C8(SceneCamera *camera)
{
    fn_800262D0(camera->cameraModelId);
    camera->cameraModelId = -1;
}

// Projects a world position to a clamped audio pan value.
// Nothing in this minigame calls this.
s16 fn_1_3700(Vec *position)
{
    Vec screenPos;
    s32 clampedX;
    s32 screenX;

    Hu3D3Dto2D(position, 1, &screenPos);
    screenPos.x *= (1.0f / 18.0f);
    screenX = static_cast<s32>(48.0f + screenPos.x);
    if (screenX < 0) {
        clampedX = 0;
    } else if (screenX > 127) {
        clampedX = 127;
    } else {
        clampedX = screenX;
    }
    return clampedX;
}

// Maps distance from the camera to a clamped audio volume.
// Nothing in this minigame calls this.
s16 fn_1_37A0(Vec *position)
{
    Vec cameraPosition;
    Vec cameraUp;
    Vec cameraTarget;
    Vec difference;
    f32 distanceSquared;
    f64 distanceScaled;
    f32 normalizedDistance;
    s16 result;
    s32 clampedResult;

    fn_80027CD0(1, &cameraPosition, &cameraUp, &cameraTarget);
    PSVECSubtract(position, &cameraPosition, &difference);
    distanceSquared = difference.z * difference.z
        + (difference.x * difference.x + difference.y * difference.y);
    distanceScaled = sqrt(static_cast<f64>(distanceSquared)) / 100.0;
    normalizedDistance = static_cast<f32>(
        (distanceScaled - 14.0) / 11.0);
    result = static_cast<s16>(127.0f - (32.0f * normalizedDistance));
    if (result < 96) {
        clampedResult = 96;
    } else if (result > 127) {
        clampedResult = 127;
    } else {
        clampedResult = result;
    }
    return clampedResult;
}

// Plays an effect with volume and pan derived from its world position.
// Nothing in this minigame calls this.
void fn_1_3A24(Vec *position, s32 soundId)
{
    Vec screenPosition;
    s32 adjustedVolume;
    s32 clampedPan;
    s32 panValue;
    s16 pan;
    f32 volume;

    volume = 9.0f + position->z / 100.0f;
    if (volume < 0.0f) {
        volume = 0.0f;
    }
    if (volume > 18.0f) {
        volume = 18.0f;
    }
    adjustedVolume = static_cast<s32>(96.0 + ((31.0 * static_cast<f64>(volume)) / 18.0));

    Hu3D3Dto2D(position, 1, &screenPosition);
    screenPosition.x *= (1.0f / 18.0f);
    panValue = static_cast<s32>(48.0f + screenPosition.x);
    if (panValue < 0) {
        clampedPan = 0;
    } else if (panValue > 127) {
        clampedPan = 127;
    } else {
        clampedPan = panValue;
    }
    pan = clampedPan;
    HuAudFXPlayVolPan(soundId, static_cast<s16>(adjustedVolume), pan);
}
