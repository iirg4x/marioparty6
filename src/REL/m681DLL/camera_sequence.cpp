#include "REL/framework/debug.h"
#include "REL/m681DLL/frame_time.h"
extern "C" {
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "game/hu3d.h"
}
#include "REL/m681DLL/vector_state.h"

void copyDebugCameraFrame(CameraFrame *, const CameraFrame *);
void set_camera_frame_from_pose(CameraFrame *, const CameraPose *);
void interpolate_camera_poses(CameraFrame *, const CameraPose *, const CameraPose *, float);
void cameraApplyToHud(CameraFrame *, s32);

extern "C" const CameraPose lbl_1_rodata_350[6] = {
    {{-376.891f, 440.0f, 357.261f}, {-0.12025834f, 0.0f, -0.0f, 0.99274266f}},
    {{-147.315f, 380.0f, 530.0f}, {-0.12025834f, 0.0f, -0.0f, 0.99274266f}},
    {{121.2674f, 390.0f, 355.03198f}, {-0.12025834f, 0.0f, -0.0f, 0.99274266f}},
    {{334.3227f, 470.0f, 488.9128f}, {-0.12025834f, 0.0f, -0.0f, 0.99274266f}},
    {{0.0f, 725.449f, 1221.631f}, {-0.12026043f, 0.0f, -0.0f, 0.9927424f}},
    {{0.0f, 725.449f, 1221.631f}, {-0.12026043f, 0.0f, -0.0f, 0.9927424f}}
};

VectorStateComposite::VectorStateComposite() : GameObject(3), EventListener(), debugCamera()
{
    Callback initialCallback;

    frameCount = 0;
    registerChannel(0);
    initialCallback = &VectorStateComposite::fn_1_7C28;
    callback = initialCallback;
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 45.0f, 1.0f, 8000.0f, 1.2f);
    Hu3DBGColorSet(0, 0, 0);
    set_camera_frame_from_pose(&frame, &lbl_1_rodata_350[4]);
    copyDebugCameraFrame(&debugCamera.frame, &frame);
}

#include "REL/m681DLL/global_state.h"
extern "C" {
#include "REL/m681DLL.h"
}

VectorStateComposite::~VectorStateComposite()
{
}

void VectorStateComposite::update()
{
    (this->*callback)();
    cameraApplyToHud(&frame, 1);
    debugResetHook(&debugCamera.frame, 0, 1);
}

void VectorStateComposite::fn_1_7C28()
{
}

void fn_1_7C2C(void)
{
}

void fn_1_7C30(void)
{
}

extern "C" {

}


static inline double minimum7C34(double first, double second)
{
    return first < second ? first : second;
}

void VectorStateComposite::fn_1_7C34()
{
    float progress;
    s32 poseIndex;
    const CameraPose *firstPose;
    const CameraPose *secondPose;
    M681GameState *manager;

    manager = &MinigameSingleton<M681GameState>::getChecked();

    if ((u8)manager->poseActive() != 0) {
        poseIndex = manager->negativeState;
        progress = minimum7C34(0.016666666666666666 * (double)frameCount, 1.0);
        firstPose = &lbl_1_rodata_350[5];
        secondPose = &lbl_1_rodata_350[poseIndex];
        interpolate_camera_poses(&frame, firstPose, secondPose, (float)progress);
        ++frameCount;
    }
}


void fn_1_7DBC(void) {}

void VectorStateComposite::receive(const EventPacket *settings)
{
    switch (settings->type) {
    case 11: {
        VectorStateComposite::Callback callback =
            &VectorStateComposite::fn_1_7C34;
        this->callback = callback;
        break;
    }
    default:
        break;
    }
}
extern "C" {
extern s32 lbl_1_bss_B0;
extern f32 lbl_1_bss_B4;
}


extern "C" {


}

extern "C" {
}

static inline f32 accelerationPerFrame(f64 accelerationPerSecondSquared)
{
    return (f32)((accelerationPerSecondSquared * 100.0) / (60.0 * 60.0));
}

extern "C" {
s32 lbl_1_bss_B0 = minutesToFrames(60L);
f32 lbl_1_bss_B4 = accelerationPerFrame(9.80665);
}
