#include "REL/m624DLL/player_math.h"
#include "REL/framework/debug.h"
#include "REL/m624DLL/game_constants.h"
#include "REL/m624DLL/camera_transition.h"
/* Camera setup, mode updates, effects and transition callbacks. */
#include "REL/framework/camera_frame.h"
#include <math.h>
#include "dolphin/mtx.h"
extern "C" {
#include "game/hu3d.h"
}
#include "REL/framework/linknode.h"
#include "REL/m624DLL/tail_types.h"
#include "humath.h"
#include "REL/framework/random.h"
void fn_1_43F0(GameCamera *);

extern "C" const CameraPose lbl_1_rodata_1C0[3] = {
    { { -547.0323f, 1100.9785f, 159.6759f },
      { -0.21730009f, 0.10483333f, 0.023480602f, 0.97017497f } },
    { { 34.092903f, 1321.6859f, 1881.372f },
      { -0.24643132f, 3.303631e-08f, 1.2992459e-07f, 0.9691603f } },
    { { 793.18365f, 997.13837f, 541.37195f }, { -0.100574255f, 0.0f, -0.0f, 0.9949296f } },
};

GameCamera::GameCamera()
    : callback(&GameCamera::fn_1_43C0), state(-1), frameCount(0), frameLimit(0)
{
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 45.0f, 100.0f,
                             5000.0f, 1.2f);
    Hu3DBGColorSet(0, 0, 0);
    set_camera_frame_from_pose(&camera, &lbl_1_rodata_1C0[0]);
    set_camera_frame_from_pose(&setupCamera.frame, &lbl_1_rodata_1C0[1]);
}

GameCamera::~GameCamera() {}

// The length of the camera's shake after a hit.
static long shakeFrames = secondsToFrames(2L);

static inline s32 frameLimitGet(GameCamera *owner)
{
    return owner->frameLimit;
}

static inline s32 frameLimitNext(s32 currentFrame, f32 progress)
{
    return currentFrame > (s32)(progress * (f32)shakeFrames)
        ? currentFrame : (s32)(progress * (f32)shakeFrames);
}

void cameraApplyToHud(CameraFrame *, s32);

void interpolate_camera_poses(CameraFrame *, const CameraPose *, const CameraPose *, f32);

extern "C" {
extern const CameraPose lbl_1_rodata_1C0[];

}

static inline void setCallback(GameCamera *camera, GameCamera::Callback next)
{
    camera->callback = next;
}
void GameCamera::flagEnabled(int mode)
{
    switch (mode) {
    case 0: {
        fn_1_47F0();
        setCallback(this, &GameCamera::fn_1_4228);
        break;
    }
    case 1:
        fn_1_47FC();
        flags &= ~(1U << mode);
        break;
    case 2:
        flags &= ~(1U << mode);
        break;
    case 3:
        flags &= ~(1U << mode);
        break;
    case 4:
        flags &= ~(1U << mode);
        break;
    }
}

void GameCamera::update()
{
    run();
    fn_1_43F0(this);
    frameCount++;
    debugResetHook(&setupCamera.frame, 0, 1);
}

void fn_1_40F4(GameCamera *owner, f32 progress)
{
    s32 nextFrame;

    progress = Clamp(progress, 0.0f, 1.0f);
    nextFrame = frameLimitNext(frameLimitGet(owner), progress);
    owner->frameLimit = nextFrame;
}

void GameCamera::fn_1_4228()
{
    s32 cameraFrame;
    f64 zoomAmount;
    const CameraPose *startPose;
    const CameraPose *endPose;

    cameraFrame = secondsToFrames(3L);
    zoomAmount = Square(1.0 - cos(M_PI * frameCount / cameraFrame)) / 4.0;
    startPose = &lbl_1_rodata_1C0[0];
    endPose = &lbl_1_rodata_1C0[1];
    interpolate_camera_poses(&camera, startPose, endPose, (f32)zoomAmount);
    cameraApplyToHud(&camera, 1);
    if (frameCount >= cameraFrame) {
        flags &= ~1U;
        setCallback(this, &GameCamera::fn_1_43C0);
    }
}

/* Native callback at .text:0x43C0 updates the embedded camera view. */
void GameCamera::fn_1_43C0()
{
    cameraApplyToHud(&camera, 1);
}

void fn_1_43F0(GameCamera *owner) {
    s32 timerExpired;
    s32 currentTimer;
    f32 timerFraction;
    f32 scale;
    f64 scaleProduct;

    timerExpired = 0;
    if (owner->frameLimit > 0) {
        if (--owner->frameLimit == 0) {
            timerExpired = 1;
        }
    }
    if (owner->frameLimit > 0) {
        Point3d direction = { 0.0f, 0.0f, 0.0f };
        Point3d *directionVector;
        Point3d *scaledDirection;

        direction.x = RandomFloat(-1.0f, 1.0f);
        direction.y = RandomFloat(-1.0f, 1.0f);
        currentTimer = owner->frameLimit;
        timerFraction = (f32)currentTimer / (f32)shakeFrames;
        scaleProduct = 100.0 * (0.3 * (f64)timerFraction);
        scale = (f32)scaleProduct;
        PSVECNormalize(&direction, &direction);
        directionVector = &direction;
        scaledDirection = directionVector;
        PSVECScale(scaledDirection, scaledDirection, scale);
        CameraFrame parameters = owner->camera;
        cameraTranslateByLocalOffset(&parameters, &direction);
        cameraApplyToHud(&parameters, 1);
    }
}

void fn_1_45E8(GameCamera *owner)
{
    owner->frameCount = 0;
    setCallback(owner, &GameCamera::fn_1_4634);
}

void GameCamera::fn_1_4634()
{
    f64 cameraFrames;
    f64 zoomAmount;
    const CameraPose *startPose;
    const CameraPose *endPose;

    cameraFrames = secondsToFrames(3L);
    zoomAmount = Square(1.0 - cos(M_PI * (frameCount / cameraFrames))) / 4.0;
    startPose = &lbl_1_rodata_1C0[1];
    endPose = &lbl_1_rodata_1C0[2];
    interpolate_camera_poses(&camera, startPose, endPose, (f32)zoomAmount);
    cameraApplyToHud(&camera, 1);
    if (frameCount >= cameraFrames) {
        setCallback(this, &GameCamera::fn_1_43C0);
    }
}

void GameCamera::fn_1_47F0()
{
    frameCount = 0;
}

void GameCamera::fn_1_47FC() {}
