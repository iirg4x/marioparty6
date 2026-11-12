#include "REL/framework/debug.h"
#include "dolphin/types.h"

#include "dolphin/mtx.h"

#include "REL/m653DLL/allocation.h"

#include "REL/m653DLL/random.h"

#include "REL/m653DLL/camera_controller.h"

#include "REL/m653DLL/model_matrix.h"

#include "REL/framework/frame_time.h"

#include "REL/m653DLL/motion_work.h"

#include "REL/framework/event_manager.h"
#include "REL/framework/game_object.h"

#include "REL/framework/event_hm.h"

#include "REL/framework/camera_frame.h"

extern "C" {
#include "game/data.h"
}

extern "C" {
#include "game/memory.h"
}

extern "C" {
#include "game/hu3d.h"
}

#include <cmath>

extern "C" { extern void fn_1_12FF4(void *work, s32 mode); }

extern void fn_1_410(CameraController653 *);

extern "C" { extern CameraFrame lbl_1_bss_0; }

void cameraTranslateByLocalOffset(CameraFrame *, Vec *);

void cameraApplyToHud(CameraFrame *, s32);

CameraController653::CameraController653() : GameObject(0x8000), shake(0.0f)
{
    Callback initialCallback;
    Hu3DCameraCreate(1);
    registerChannel(0);
    cameraModelId = -1;
    fn_1_410(this);
    initialCallback = &CameraController653::fn_1_334;
    callback = initialCallback;
}

/* Copies the object's transform parameters and applies a random offset while its effect is active. */
void CameraController653::update()
{
    f32 distance;
    Vec offset;
    f64 depthOffset;
    f64 lateralOffset;
    f64 verticalOffset;

    (this->*callback)();
    if (debugCamera.highFlag == 0) {
        distance = shake.update();
        if (distance > 0.0f) {
            lbl_1_bss_0 = frame;
            verticalOffset = RandomFloat(-distance, distance);
            lateralOffset = RandomFloat(-distance, distance);
            depthOffset = RandomFloat(-distance, distance);
            offset.x = depthOffset;
            offset.y = lateralOffset;
            offset.z = verticalOffset;
            cameraTranslateByLocalOffset(&lbl_1_bss_0, &offset);
            cameraApplyToHud(&lbl_1_bss_0, 1);
        }
    }
    debugResetHook(&debugCamera.frame, 0, 1);
}

extern "C" {
#include <stdio.h>
}

extern "C" {
#include <stdlib.h>
}

void CameraController653::receive(EventMessage *event)
{
    switch (event->type) {
    case 0:
        shake.set(25.0f, 0);
        shake.set(0.0f, secondsToFrames(2L));
        break;
    case 1: {
        Callback next;
        printf("EVENT_ENV_RESULT_CAM\n");
        next = &CameraController653::fn_1_3CC;
        callback = next;
        break;
    }
    default:
        break;
    }
}

void fn_1_48C(CameraController653 *);

extern "C" {
extern const CameraPose lbl_1_rodata_18 = {
    {0.0f, 914.58264f, 1521.375f},
    {-0.24166475f, 0.0f, -0.0f, 0.97035986f}
};
}

void CameraController653::fn_1_334()
{
    CameraController653::Callback callback;
    cameraReadFromHud(&this->frame, 1);
    cameraReadFromHud(&this->debugCamera.frame, 1);
    if (fn_8002D75C(this->cameraModelId) != 0) {
        callback = &CameraController653::fn_1_3C8;
        this->callback = callback;
        fn_1_48C(this);
        return;
    }
}

void CameraController653::fn_1_3C8() {}

void CameraController653::fn_1_3CC()
{
    set_camera_frame_from_pose(&frame, &lbl_1_rodata_18);
    cameraApplyToHud(&frame, 1);
}

void fn_1_410(CameraController653 *owner)
{
    s16 motionId = fn_8002C4C0(
        fn_80008DD0(DATANUM(0x700000, 6), 0x10000000, 2));
    owner->motionId = (s16)motionId;
    owner->cameraModelId = fn_800283F4((s16)motionId, 1);
    fn_800284C8(owner->cameraModelId, 1);
    fn_80026B8C(owner->cameraModelId, 0);
}

void fn_1_48C(CameraController653 *work)
{
    if (work->cameraModelId != -1) {
        fn_800262D0(work->cameraModelId);
        work->cameraModelId = -1;
    }
}

CameraController653::~CameraController653() {}

extern "C" {
CameraFrame lbl_1_bss_0;
}
