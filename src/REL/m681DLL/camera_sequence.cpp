#include "REL/m681DLL/game_controller.h"
#include "REL/framework/world_units.h"
#include "REL/framework/debug.h"
#include "REL/framework/frame_time.h"
extern "C" {
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "game/hu3d.h"
}
#include "REL/m681DLL/vector_state.h"
#include "REL/m681DLL/game_constants.h"

extern "C" const CameraPose lbl_1_rodata_350[6] = {
    {{-376.891f, 440.0f, 357.261f}, {-0.12025834f, 0.0f, -0.0f, 0.99274266f}},
    {{-147.315f, 380.0f, 530.0f}, {-0.12025834f, 0.0f, -0.0f, 0.99274266f}},
    {{121.2674f, 390.0f, 355.03198f}, {-0.12025834f, 0.0f, -0.0f, 0.99274266f}},
    {{334.3227f, 470.0f, 488.9128f}, {-0.12025834f, 0.0f, -0.0f, 0.99274266f}},
    {{0.0f, 725.449f, 1221.631f}, {-0.12026043f, 0.0f, -0.0f, 0.9927424f}},
    {{0.0f, 725.449f, 1221.631f}, {-0.12026043f, 0.0f, -0.0f, 0.9927424f}}
};

// Initializes the camera and copies its starting pose to the debug camera.
GameCamera::GameCamera() : GameObject(3), EventListener(), debugCamera()
{
    Callback initialCallback;

    frameCount = 0;
    registerChannel(0);
    initialCallback = &GameCamera::fn_1_7C28;
    callback = initialCallback;
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 45.0f, 1.0f, 8000.0f, 1.2f);
    Hu3DBGColorSet(0, 0, 0);
    set_camera_frame_from_pose(&frame, &lbl_1_rodata_350[4]);
    copyDebugCameraFrame(&debugCamera.frame, &frame);
}

#include "REL/m681DLL/global_state.h"

// Releases the camera controller bases.
GameCamera::~GameCamera()
{
}

// Runs the camera callback and applies the resulting frame.
void GameCamera::update()
{
    (this->*callback)();
    cameraApplyToHud(&frame, 1);
    debugResetHook(&debugCamera.frame, 0, 1);
}

// Leaves the camera pose unchanged.
void GameCamera::fn_1_7C28()
{
}

// Performs no operation.
void fn_1_7C2C(void)
{
}

// Performs no operation.
void fn_1_7C30(void)
{
}

// Returns the smaller value.
static inline double minimumValue(double first, double second)
{
    return first < second ? first : second;
}

// Moves the camera toward the selected player over one second.
void GameCamera::fn_1_7C34()
{
    float progress;
    s32 poseIndex;
    const CameraPose *firstPose;
    const CameraPose *secondPose;
    M681GameState *manager;

    manager = &MinigameSingleton<M681GameState>::getChecked();

    if ((u8)manager->hasWinner() != 0) {
        poseIndex = manager->winnerPlayer;
        progress = minimumValue(0.016666666666666666 * (double)frameCount, 1.0);
        firstPose = &lbl_1_rodata_350[5];
        secondPose = &lbl_1_rodata_350[poseIndex];
        interpolate_camera_poses(&frame, firstPose, secondPose, (float)progress);
        ++frameCount;
    }
}

// Performs no operation.
void fn_1_7DBC(void) {}

// Selects camera interpolation on the winner-camera event.
void GameCamera::receive(const EventPacket *settings)
{
    switch (settings->type) {
    case 11: {
        GameCamera::Callback callback =
            &GameCamera::fn_1_7C34;
        this->callback = callback;
        break;
    }
    default:
        break;
    }
}
