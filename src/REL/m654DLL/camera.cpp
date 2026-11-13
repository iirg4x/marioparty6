#include "REL/framework/square.h"
#include <new>
#include "math.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
#include "REL/framework/debug.h"
#include "dolphin/types.h"
#include "REL/m654DLL/camera_transition.h"
#include "REL/m654DLL/camera_node_owner.h"
extern "C" {
#include "dolphin/mtx.h"
}
#include "humath.h"
extern "C" {
#include "game/hu3d.h"
}

const CameraPosePair lbl_1_rodata_90[2] = {
    {{{0.0f, 923.52576f, 1120.26f}, {-0.17122246f, 0.0f, -0.0f, 0.9852325f}}, {{0.0f, 929.43555f, 980.26f}, {-0.43945006f, 0.0f, -0.0f, 0.8982671f}}},
    {{{0.0f, 1029.89f, 1700.26f}, {-0.24941446f, 0.0f, -0.0f, 0.96839684f}}, {{793.18365f, 997.13837f, 541.37195f}, {-0.100574255f, 0.0f, -0.0f, 0.9949296f}}}
};
// Creates camera 1, installs its initial pose and prepares the debug camera.
CameraManager::CameraManager()
    : callback(&CameraManager::fn_1_1948), startPose(lbl_1_rodata_90[0].first), targetPose(lbl_1_rodata_90[0].second), elapsed(0)
{
    Hu3DCameraCreate(1);
    Hu3DBGColorSet(0, 0, 0);
    set_camera_frame_from_pose(&static_cast<CameraFrame &>(*this), &startPose);
    registerChannel(0);
    set_camera_frame_from_pose(&debugCamera.frame, &lbl_1_rodata_90[1].first);
}

void fn_1_1978(CameraManager *, const CameraPose *, const CameraPose *, int);

// One-minute time limit and gravity in world units per frame squared.
int lbl_1_bss_48 = minutesToFrames(1L);
float lbl_1_bss_4C = 0.016666666666666666 * (0.016666666666666666 * cmToWorld(980.665f));
// Releases the camera manager base objects.
CameraManager::~CameraManager() {}

// Starts the camera transition for the selected phase.
void CameraManager::flagEnabled(int state)
{
    this->activeBit = state;
    switch (state) {
    case 0:
        fn_1_1978(this, &lbl_1_rodata_90[0].first, &lbl_1_rodata_90[0].second, secondsToFrames<long>(1));
        break;
    case 1:
        fn_1_1978(this, &lbl_1_rodata_90[0].second, &lbl_1_rodata_90[1].first, secondsToFrames<long>(2));
        break;
    default:
        this->flags &= ~(1 << state);
        break;
    }
}

// Applies the current camera state, advances its frame and updates the debug camera.
void CameraManager::update()
{
    this->run();
    this->elapsed++;
    debugResetHook(&this->debugCamera.frame, 0, 1);
}

// Starts a camera transition toward the position supplied by event 4.
void CameraManager::receive(const EventPacket *event)
{
    if (event->type == 4 && event->context != 0) {
        Vec *targetPosition = static_cast<Vec *>(event->context);
        double startX;
        double startY;
        double startZ;
        double endX;
        double endY;
        double endZ;
        CameraFrame path;
        startZ = targetPosition->z + cmToWorld(600.0f);
        startY = targetPosition->y + cmToWorld(200.0f);
        startX = targetPosition->x;
        path.position.x = startX;
        path.position.y = startY;
        path.position.z = startZ;
        endZ = targetPosition->z;
        endY = targetPosition->y + cmToWorld(100.0f);
        endX = targetPosition->x;
        path.target.x = endX;
        path.target.y = endY;
        path.target.z = endZ;
        path.up.x = 0.0f;
        path.up.y = 1.0f;
        path.up.z = 0.0f;
        get_camera_pose_from_frame(&path, &this->targetPose);
        fn_1_1978(this, &lbl_1_rodata_90[1].first, &this->targetPose, secondsToFrames<long>(3));
    }
}

// Applies the current camera frame to camera 1.
void CameraManager::fn_1_1948() {
    cameraApplyToHud(&static_cast<CameraFrame &>(*this), 1);
}

// Copies the transition endpoints and selects the interpolating camera state.
void fn_1_1978(CameraManager *owner, const CameraPose *first,
                           const CameraPose *second, int frames)
{
    owner->startPose = *first;
    owner->targetPose = *second;
    owner->duration = frames;
    owner->elapsed = 0;
    void (CameraManager::*nextCallback)() = &CameraManager::fn_1_1A38;
    owner->callback = nextCallback;
}

// Interpolates the camera with cosine easing, then clears the completed phase flag.
void CameraManager::fn_1_1A38()
{
    int transitionFrames = secondsToFrames<long>(3);
    double easedAmount =
        square(1.0 -
               cos(M_PI * (double)elapsed / duration)) / 4.0;

    interpolate_camera_poses(&static_cast<CameraFrame &>(*this), &startPose, &targetPose,
              (float)easedAmount);
    cameraApplyToHud(&static_cast<CameraFrame &>(*this), 1);
    if (elapsed >= duration) {
        startPose = targetPose;
        unsigned int bitIndex = activeBit;
        flags &= ~(1 << bitIndex);
        void (CameraManager::*nextCallback)() = &CameraManager::fn_1_1948;
        this->callback = nextCallback;
    }
}

