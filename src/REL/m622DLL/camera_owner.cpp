#include "REL/framework/debug.h"
#include "REL/m622DLL/callback_object.h"
#include "REL/framework/linknode.h"
#include <cmath>
extern "C" {
#include "game/hu3d.h"
#include "game/data.h"
#include "game/memory.h"
#include "game/hsfex.h"
#include "game/audio.h"
}

#include "REL/m622DLL/camera_owner.h"

extern "C" {

extern const CameraPose lbl_1_rodata_1A0 = {
    {-90.55096f, 1898.2443f, 12093.716f},
    {-0.0874612f, 0.0f, -0.0f, 0.99616796f}
};
extern const Vec lbl_1_rodata_1BC = {0.0f, 900.0f, 700.0f};
extern const Vec lbl_1_rodata_1C8 = {0.0f, 200.0f, 0.0f};
}

/* Creates camera 1 with a black background and the initial camera pose. */
CameraOwner::CameraOwner() : callback(&CameraOwner::fn_1_5974), target(0), motionModelId(-1)
{
    Hu3DCameraCreate(1);
    Hu3DBGColorSet(0, 0, 0);
    set_camera_frame_from_pose(&setup.frame, &lbl_1_rodata_1A0);
}

/* Releases the object through its member and base destructors. */
CameraOwner::~CameraOwner()
{
}

/* Reads the animated camera and switches to the fixed frame when its motion ends. */
void CameraOwner::fn_1_58DC()
{
    if (motionModelId != -1) {
        cameraReadFromHud(&frame, 1);
        if (fn_8002D75C(motionModelId) != 0) {
            Callback value = &CameraOwner::fn_1_5974;
            callback = value;
            flags &= ~1U;
        }
    }
}

/* Returns without changing state. */
void fn_1_5970() {}

/* Applies the stored camera frame to camera 1. */
void CameraOwner::fn_1_5974()
{
    cameraApplyToHud(&frame, 1);
}

/* Applies the stored camera frame to camera 1. */
void CameraOwner::fn_1_59A4()
{
    cameraApplyToHud(&frame, 1);
}

/* After 90 frames, interpolates the camera toward offsets from the followed player. */
void CameraOwner::fn_1_59D4()
{
    CameraFrame targetFrame;
    CameraPose currentTransform;
    CameraPose targetTransform;
    if ((float)(++frameCount) < 90.0f)
        return;
    Vec *targetPosition = &target->position;
    PSVECAdd(targetPosition, &lbl_1_rodata_1BC, &targetFrame.position);
    PSVECAdd(targetPosition, &lbl_1_rodata_1C8, &targetFrame.target);
    targetFrame.up.x = 0.0f;
    targetFrame.up.y = 1.0f;
    targetFrame.up.z = 0.0f;
    get_camera_pose_from_frame(&frame, &currentTransform);
    get_camera_pose_from_frame(&targetFrame, &targetTransform);
    interpolate_camera_poses(&frame, &currentTransform, &targetTransform, 0.05f);
    cameraApplyToHud(&frame, 1);
}

/* Sets the followed player and starts the delayed camera interpolation. */
void CameraOwner::fn_1_5AE8(CallbackObject *value)
{
    Callback next;
    frameCount = 0;
    target = value;
    next = &CameraOwner::fn_1_59D4;
    callback = next;
}

/* Starts or releases the camera motion and selects the requested camera callback. */
void CameraOwner::flagEnabled(int mode)
{
    switch (mode) {
    case 0: {
        Callback next;
        ::fn_1_5CD0(this);
        next = &CameraOwner::fn_1_58DC;
        callback = next;
        break;
    }
    case 1:
        fn_1_5D38();
        flags &= ~(1U << mode);
        break;
    case 2: {
        Callback next = &CameraOwner::fn_1_59A4;
        callback = next;
        flags &= ~(1U << mode);
        break;
    }
    case 3:
        flags &= ~(1U << mode);
        break;
    case 4:
        flags &= ~(1U << mode);
        break;
    }
}

/* Runs the current camera callback and passes the frame to the debug hook. */
void CameraOwner::update()
{
    if (callback)
        (this->*callback)();
    debugResetHook(&setup.frame, 0, 1);
}

/* Creates the camera model from its motion data and starts the camera motion. */
void fn_1_5CD0(CameraOwner *owner)
{
    s16 motionId = fn_8002C4C0(
        fn_80008DD0(0x510003, 0x10000000, 2));
    owner->motionModelId = fn_800283F4((s32)motionId, 1);
    fn_800284C8(owner->motionModelId, 1);
}

/* Releases the camera motion model and clears its handle. */
void CameraOwner::fn_1_5D38()
{
    fn_800262D0(motionModelId);
    motionModelId = -1;
}

/* Projects a world position into a sound pan value clamped to 0 through 127. */
s16 fn_1_5D70(Vec *pos)
{
    Vec screenPos;
    s32 pan;
    s32 screenPan;

    Hu3D3Dto2D(pos, 1, &screenPos);
    screenPos.x *= 0.055555556f;
    screenPan = (s32)(48.0f + screenPos.x);
    if (screenPan < 0) {
        pan = 0;
    } else if (screenPan > 127) {
        pan = 127;
    } else {
        pan = screenPan;
    }
    return (s16)pan;
}

/* Converts distance from camera 1 into a volume clamped to 0x60 through 0x7F. */
s16 fn_1_5E10(Vec *position)
{
    Vec cameraPosition;
    Vec cameraUp;
    Vec cameraTarget;
    Vec difference;
    float distanceSquared;
    double distanceScaled;
    float normalizedDistance;
    s16 result;
    s32 clampedResult;

    fn_80027CD0(1, &cameraPosition, &cameraUp, &cameraTarget);
    PSVECSubtract(position, &cameraPosition, &difference);
    distanceSquared = difference.z * difference.z + (difference.x * difference.x + difference.y * difference.y);
    distanceScaled = std::sqrt((double)distanceSquared) / 100.0;
    normalizedDistance = (float)((distanceScaled - 14.0) / 11.0);
    result = (s16)(127.0f - (32.0f * normalizedDistance));
    if (result < 0x60) {
        clampedResult = 0x60;
    } else if (result > 0x7F) {
        clampedResult = 0x7F;
    } else {
        clampedResult = result;
    }
    return clampedResult;
}

/* Sets a sound effect volume from depth and its pan from projected screen position. */
void fn_1_6094(Vec *position, s32 soundId)
{
    Vec screenPosition;
    s32 adjustedVolume;
    s32 clampedPan;
    s32 panValue;
    s16 pan;
    float volume;

    volume = 9.0f + position->z / 100.0f;
    if (volume < 0.0f) {
        volume = 0.0f;
    }
    if (volume > 18.0f) {
        volume = 18.0f;
    }
    adjustedVolume = (s32)(96.0 + ((31.0 * (double)volume) / 18.0));

    Hu3D3Dto2D(position, 1, &screenPosition);
    screenPosition.x *= 0.055555556f;
    panValue = (s32)(48.0f + screenPosition.x);
    if (panValue < 0) {
        clampedPan = 0;
    } else if (panValue > 127) {
        clampedPan = 127;
    } else {
        clampedPan = panValue;
    }
    pan = (s16)clampedPan;
    HuAudFXPlayVolPan(soundId, (s16)adjustedVolume, pan);
}
