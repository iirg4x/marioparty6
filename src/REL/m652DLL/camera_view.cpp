extern "C" {
#include "game/hu3d.h"
#include "REL/framework/assertion.h"
}
#include "REL/framework/debug.h"
#include <math.h>
#include "REL/framework/frame_time.h"
#include "REL/m652DLL/motion_algorithms.h"
#include "REL/framework/world_units.h"
#include "REL/framework/list_object.h"
#include <vector>
#include "dolphin/types.h"
#include "REL/framework/game_object.h"
#include "REL/framework/camera_frame.h"
#include "REL/framework/category_sphere.h"
#include "REL/framework/event_packet.h"
#include "REL/m652DLL/camera_split.h"
#include "REL/framework/scene_shadow.h"
#include "REL/m652DLL/player_camera.h"
#include "humath.h"

extern "C" {
#include "REL/m652DLL/game_constants.h"
}

extern "C" const CameraPose lbl_1_rodata_1D0[3] = {
{{0.0f,925.32f,-3348.63f},{-0.13663656f,0.0f,-0.0f,0.99062127f}},
{{0.0f,1613.5106f,904.49774f},{-0.42033017f,0.0f,-0.0f,0.9073713f}},
{{0.0f,571.22656f,-4388.63f},{0.010465809f,0.0f,0.0f,0.9999453f}}
};

/* Creates two camera masks and initializes the split viewports. */
CameraSplit::CameraSplit() : GameObject(0), modified(1), positions(2, 0x280), screenOffset(0)
{
    unsigned short cameraMask;
    int cameraIndex;

    cameraMask = 0;
    for (cameraIndex = 0; cameraIndex < 2; cameraIndex++) {
        cameraMask |= 1u << cameraIndex;
    }
    Hu3DBGColorSet(0, 0, 0);
    Hu3DCameraCreate(cameraMask);
    update();
}

/* Releases the object through its member and base destructors. */
CameraSplit::~CameraSplit() {}

void fn_1_345C(CameraSplit *owner, unsigned int index, int value);
void fn_1_34C0(CameraSplit *self, int value);

/* Sets a split position within the position array and marks the viewports for update. */
void fn_1_345C(CameraSplit *owner, unsigned int index, int value)
{
    if (index < owner->positions.size() - 1) {
        owner->positions[index] = value;
        owner->modified = 1;
    }
}

/* Sets half the supplied screen offset and marks the viewports for update. */
void fn_1_34C0(CameraSplit *self, int value)
{
    self->screenOffset = value / 2;
    self->modified = 1;
}

/* Returns the larger integer. */
static inline int maximumInteger(int first, int second)
{
    return first > second ? first : second;
}

/* Clamps an integer to zero. */
static inline int nonnegative(int value)
{
    return value < 0 ? 0 : value;
}

/* Clamps a viewport position to the 16 through 624 interval. */
static inline int clipViewportPosition(int value)
{
    if (value < 16) return 16;
    if (value > 624) return 624;
    return value;
}

/* Computes the aspect ratio for a viewport width, with a minimum of 0.6. */
static inline double viewportAspect(int width)
{
    return (double)0.6f > (double)0.0018750001f * (double)width
        ? (double)0.6f : (double)0.0018750001f * (double)width;
}

/* Interpolates a viewport edge along the 640-pixel screen width. */
static inline int interpolateViewportEdge(int first, int last, int position)
{
    return (int)((1.0 - (double)position / 640.0) * (double)first +
                 ((double)position / 640.0) * (double)last);
}

/* Keeps a scissor width at least one pixel. */
static inline int positiveScissorWidth(int width)
{
    return width < 1 ? 1 : width;
}

/* Updates both cameras' scissor rectangles, viewports and perspective from the split positions. */
void CameraSplit::update()
{
    if (modified != 0) {
        int camera = 1;
        int previousPosition = 0;
        int index = 0;
        while (index < 2) {
            int targetPosition = positions[index];
            int positionDelta = targetPosition - previousPosition;
            int leftMargin;
            int rightMargin;
            int clip;
            int viewportStart;
            int viewportEnd;
            int viewportWidth;
            unsigned int scissorStart;
            int scissorEnd;
            int scissorWidth;
            if (positionDelta <= 0) {
                Hu3DCameraKill(camera);
            } else {
                leftMargin = nonnegative(16 - previousPosition);
                rightMargin = nonnegative(targetPosition - 624);
                clip = maximumInteger(leftMargin, rightMargin);
                viewportStart = clipViewportPosition(previousPosition) - clip;
                viewportEnd = clip + clipViewportPosition(targetPosition);
                viewportWidth = nonnegative(viewportEnd - viewportStart);

                scissorStart = (unsigned int)screenOffset +
                    (unsigned int)interpolateViewportEdge(-screenOffset,
                                           screenOffset + 640, previousPosition);
                scissorEnd = interpolateViewportEdge(-screenOffset,
                                      screenOffset + 640, targetPosition) - screenOffset;
                scissorWidth = positiveScissorWidth(scissorEnd - (int)scissorStart);

                Hu3DCameraViewportSet(camera, (float)viewportStart, 0.0f,
                                      (float)viewportWidth, 480.0f, 0.0f, 1.0f);
                Hu3DCameraScissorSet(camera, scissorStart, 0, scissorWidth,
                                     0x1E0);
                Hu3DCameraPerspectiveSet(camera, 45.0f, 100.0f, 10000.0f,
                                         (float)viewportAspect(viewportWidth));
                previousPosition = targetPosition;
            }
            index++;
            camera *= 2;
        }
        modified = 0;
    }
}

/* Initializes the player camera, its event channel and its saved camera frame. */
PlayerCamera::PlayerCamera(unsigned int index)
    : action(&PlayerCamera::fn_1_4118), playerIndex(index), cameraMask((unsigned short)(1 << index)), frame(0)
{
    set_camera_frame_from_pose(&static_cast<CameraFrame &>(*this), lbl_1_rodata_1D0);
    registerChannel(playerIndex + 1);
    set_camera_frame_from_pose(&state.frame, &lbl_1_rodata_1D0[1]);
}

/* Releases the object through its member and base destructors. */
PlayerCamera::~PlayerCamera() {}

/* Starts the opening camera motion for flag zero and clears other requested flags. */
void PlayerCamera::flagEnabled(int mask) {
    switch (mask) {
    case 0: {
        PlayerCamera::Action temporary;
        frame = 0;
        temporary = &PlayerCamera::fn_1_3E2C;
        action = temporary;
        break;
    }
    default: {
        flags &= ~(1U << mask);
        break;
    }
    }
}

/* Starts the closing camera motion when event type two arrives. */
void PlayerCamera::receive(const EventPacket *event)
{
    if (event->type == 2) {
        PlayerCamera::Action temporary;

        frame = 0;
        get_camera_pose_from_frame(&static_cast<CameraFrame &>(*this), &position);
        temporary = &PlayerCamera::fn_1_4458;
        action = temporary;
    }
}

/* Dispatches the stored callback, increments its playerIndex, and updates its state. */

void PlayerCamera::update()
{
 (this->*action)();
 frame++;
 debugResetHook(&state.frame, playerIndex, cameraMask);
}

void fn_1_41B4(PlayerCamera *object, CameraPose *pose);

/* Interpolates the opening camera pose and moves the split boundary over five seconds. */
void PlayerCamera::fn_1_3E2C()
{
    CameraSplit *current;
    int frameCount;
    int step;
    const CameraPose *defaultPose;
    double amount;
    double fraction;
    CameraPose cameraPose;

    frameCount = secondsToFrames<long>(5);
    amount = square(1.0 -
        cos((M_PI * (double)this->frame) / (double)frameCount)) / 4.0;
    if (CameraSplit::current() != 0 && amount > 0.2) {
        current = &CameraSplit::getChecked();
        step = (((int)this->playerIndex + 1) * 640) / 2;
        fraction = (amount - 0.2) / 0.8;
        fn_1_345C(current, this->playerIndex, interpolateValue(640, step, fraction));
        fn_1_34C0(current, 4);
    }

    defaultPose = lbl_1_rodata_1D0;
    fn_1_41B4(this, &cameraPose);
    interpolate_camera_poses(&static_cast<CameraFrame &>(*this),
                             defaultPose, &cameraPose, (float)amount);
    cameraApplyToHud(&static_cast<CameraFrame &>(*this), this->cameraMask);
    if (this->frame >= frameCount) {
        this->flags &= ~1u;
        PlayerCamera::Action next = &PlayerCamera::fn_1_414C;
        this->action = next;
    }
}

/* Applies the current camera frame to the player's camera mask. */
void PlayerCamera::fn_1_4118()
{
 cameraApplyToHud(&static_cast<CameraFrame &>(*this), cameraMask);
}
/* Moves the current camera pose toward the pose fitted to the player's registered spheres. */
void PlayerCamera::fn_1_414C()
{
 CameraPose parameterState;
 CameraPose motionState;
 get_camera_pose_from_frame(&static_cast<CameraFrame &>(*this), &parameterState);
 fn_1_41B4(this, &motionState);
 interpolate_camera_poses(&static_cast<CameraFrame &>(*this),
                             &parameterState, &motionState, 0.1f);
 cameraApplyToHud(&static_cast<CameraFrame &>(*this), cameraMask);
}

/* Clamps a floating-point value to a minimum. */
static inline float minimumCameraHeight(float value, float minimum)
{
    return value > minimum ? value : minimum;
}

/* Fits a camera pose to the player's registered spheres and keeps its height at least 1300. */
void fn_1_41B4(PlayerCamera *object, CameraPose *pose)
{
    std::vector<Sphere *> setup;
    CameraFrame camera;

    setup.clear();
    collectCategoryValues(&setup, object->playerIndex);
    if (setup.size() == 0) {
        get_camera_pose_from_frame(&static_cast<CameraFrame &>(*object), pose);
        return;
    }

    CameraPerspective calculation;
    calculation.read(object->cameraMask);
    calculation.width /= 2;
    set_camera_frame_from_pose(&camera, &lbl_1_rodata_1D0[1]);
    fitCameraToSpheres(&camera, &calculation, &setup);
    camera.position.y = minimumCameraHeight(camera.position.y, 1300.0f);
    get_camera_pose_from_frame(&camera, pose);
}

/* Interpolates the closing camera pose and split boundary over three seconds. */
void PlayerCamera::fn_1_4458()
{
    CameraSplit *current;
    int frameCount;
    int index;
    double amount;
    CameraPose *cameraTarget;
    const CameraPose *cameraDefault;

    frameCount = secondsToFrames<long>(3);
    amount = square(1.0 -
        cos((M_PI * (double)this->frame) / (double)frameCount)) / 4.0;
    cameraTarget = &position;
    cameraDefault = &lbl_1_rodata_1D0[2];
    interpolate_camera_poses(&static_cast<CameraFrame &>(*this),
                             cameraTarget, cameraDefault, (float)amount);
    cameraApplyToHud(&static_cast<CameraFrame &>(*this), this->cameraMask);

    if (CameraSplit::current() != 0) {
        current = &CameraSplit::getChecked();
        index = 0;
        for (; index < 1; index++) {
            int step = ((index + 1) * 0x280) / 2;
            int end = (index >= this->playerIndex) ? 0x280 : 0;
            fn_1_345C(current, index, interpolateValue(step, end, amount));
        }
    }

    if (this->frame >= frameCount) {
        PlayerCamera::Action next = &PlayerCamera::fn_1_4118;
        this->action = next;
    }
}
