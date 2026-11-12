#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
#include "REL/framework/debug.h"
// The board camera: two view poses, the blends between them and the phase callbacks that run them.
#include "REL/m639DLL/camera_owner.h"
#include "REL/m639DLL/mode_selection.h"

extern "C" {
#include "game/hu3d.h"
}
#include "math.h"
#include "humath.h"

extern "C" {
void __msl_assertion_failed(const char *expression, const char *file, int line);



}

const CameraPose lbl_1_rodata_158[4] = {
    {{615.0237f, 95.50425f, 1328.094f}, {0.0022042936f, 0.994485f, 0.102768295f, -0.020818291f}},
    {{-0.000539f, 418.76147f, -4409.2456f}, {6.3853003e-09f, 0.9964208f, 0.08453197f, -7.526674e-08f}},
    {{540.27167f, -41.449585f, -1153.2007f}, {0.0f, 0.99503917f, 0.09948477f, 0.0f}},
    {{-96.12441f, -41.449585f, -1789.5972f}, {0.0f, 0.99503917f, 0.09948477f, 0.0f}}
};

CameraOwner::CameraOwner() : callback(&CameraOwner::applyView), state(-1), frame(0), endPoseIndex(1)
{
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 15.0f, 1000.0f, 16000.0f, 1.2f);
    Hu3DBGColorSet(0, 0, 0);
    set_camera_frame_from_pose(&view, lbl_1_rodata_158);
    set_camera_frame_from_pose(&debugCamera.frame, &lbl_1_rodata_158[1]);
}

CameraOwner::~CameraOwner() {}

void CameraOwner::flagEnabled(int mode)
{
    switch (mode) {
    case 0: {
        fn_1_3860();
        void (CameraOwner::*next)() = &CameraOwner::fn_1_33BC;
        callback = next;
        break;
    }
    case 1:
        fn_1_386C();
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

void CameraOwner::update()
{
    (this->*callback)();
    frame++;
    debugResetHook(&debugCamera.frame, 0, 1);
}

template <class T> inline T Square(const T &value)
{
    return value * value;
}

/* Advances the camera transition and installs its completion callback. */
void CameraOwner::fn_1_33BC()
{
    int cameraFrame = secondsToFrames(4L);
    double zoomAmount = Square(1.0 - cos(M_PI * frame / cameraFrame)) / 4.0;
    const CameraPose *startPose = lbl_1_rodata_158;
    const CameraPose *endPose = &lbl_1_rodata_158[1];
    interpolate_camera_poses(&view, startPose, endPose, (float)zoomAmount);
    cameraApplyToHud(&view, 1);
    if (frame >= cameraFrame) {
        flags &= ~1U;
        void (CameraOwner::*next)() = &CameraOwner::applyView;
        callback = next;
    }
}

void CameraOwner::applyView()
{
    cameraApplyToHud(&view, 1);
}

void CameraOwner::fn_1_3584()
{
    int mode = M639StateOwner::getChecked().fn_1_2CAC();
    switch (mode) {
    case 0:
        endPoseIndex = 2;
        break;
    case 1:
        endPoseIndex = 3;
        break;
    default:
        flags &= ~8U;
        return;
    }
    frame = 0;
    void (CameraOwner::*next)() = &CameraOwner::fn_1_3690;
    callback = next;
}

void CameraOwner::fn_1_3690()
{
    double cameraFrames = secondsToFrames(4L);
    double zoomAmount = Square(1.0 - cos(M_PI * (frame / cameraFrames))) / 4.0;
    const CameraPose *startPose = &lbl_1_rodata_158[1];
    const CameraPose *endPose = &lbl_1_rodata_158[endPoseIndex];
    interpolate_camera_poses(&view, startPose, endPose, (float)zoomAmount);
    cameraApplyToHud(&view, 1);
    if (frame >= cameraFrames) {
        flags &= ~8U;
        void (CameraOwner::*next)() = &CameraOwner::applyView;
        callback = next;
    }
}

void CameraOwner::fn_1_3860()
{
    frame = 0;
}

void CameraOwner::fn_1_386C() {}



extern "C" {
int lbl_1_bss_68 = minutesToFrames(5L);
int lbl_1_bss_6C = secondsToFrames(30L);
float lbl_1_bss_70 = (1.0 / 60.0) * ((1.0 / 60.0) * cmToWorld(980.665f));
}
