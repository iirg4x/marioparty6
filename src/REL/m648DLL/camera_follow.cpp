#include "REL/framework/camera_frame.h"
extern "C" {
#include "dolphin/types.h"
}
#include "dolphin/mtx.h"
#include "game/memory.h"
#include "REL/framework/model.h"
extern "C" {
#include "game/audio.h"
}
#include <math.h>
extern "C" {
#include "game/hu3d.h"
}
#include "REL/m648DLL/player.h"
extern "C" {
#include "game/data.h"
#include "REL/framework/assertion.h"
}
#include "REL/m648DLL/camera_view.h"
#include "REL/framework/list_object.h"
#include "REL/m648DLL.h"

#include "REL/m648DLL/camera_manager.h"

typedef void (CameraManager::*CameraManagerCallback)();

/* Reads camera 1 position and its normalized viewing direction. */
void fn_1_8740(Vec *position, Vec *direction) {
    Vec up;
    Vec target;

    fn_80027CD0(1, position, &up, &target);
    PSVECSubtract(&target, position, direction);
    PSVECNormalize(direction, direction);
}

/* Creates camera 1 with a black background. */
CameraManager::CameraManager()
    : callback(&CameraManager::fn_1_8ACC), target(0), modelId(-1), listenerActive(0)
{
    Hu3DCameraCreate(1);
    Hu3DBGColorSet(0, 0, 0);
}

/* Removes the active audio listener. */
CameraManager::~CameraManager() {
    if (listenerActive != 0) HuAudFXListnerKill();
}
/* Finishes the active camera motion and selects its next update callback. */
void CameraManager::fn_1_89D4()
{
    if (this->modelId != -1) {
        cameraReadFromHud(&this->view, 1);
        if (fn_8002D75C(this->modelId)) {
            fn_1_BC84();
            this->timer = 120;
            CameraManagerCallback nextCallback = &CameraManager::fn_1_8A68;
            this->callback = nextCallback;
        }
    }
}

/* Waits for the camera delay, then restores the camera callback and clears flag 0. */
void CameraManager::fn_1_8A68()
{
    if (--timer < 0) {
        CameraManagerCallback nextCallback = &CameraManager::fn_1_8ACC;
        callback = nextCallback;
        flags &= ~1;
    }
}

/* Applies the stored camera frame to camera 1. */
void CameraManager::fn_1_8ACC() { cameraApplyToHud(&view, 1); }

/* Applies the stored camera frame to camera 1. */
void CameraManager::fn_1_8AFC() { cameraApplyToHud(&view, 1); }

/* Initialized bytes of unknown meaning; the camera vectors follow at 0x21C. */
extern "C" const u8 lbl_1_rodata_200[0x1C] = {
    0x00, 0x00, 0x00, 0x00, 0x45, 0x3B, 0x60, 0x77, 0x45, 0x33, 0x44, 0xA8, 0xBE, 0xD1, 0x9D, 0xB6, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x3F, 0x69, 0x8F, 0xEA
};

const Vec lbl_1_rodata_21C = { 0.0f, 300.0f, 600.0f };

const Vec lbl_1_rodata_228 = { 0.0f, 200.0f, 0.0f };

/* Waits ninety frames, then eases camera 1 toward the selected player. */
void CameraManager::fn_1_8B2C() {
    CameraFrame targetFrame;
    CameraPose currentTransform;
    CameraPose targetTransform;

    if ((float)(++motionFrame) < 90.0f) {
        return;
    }

    Vec *targetPosition = &target->position;
    PSVECAdd(targetPosition, &lbl_1_rodata_21C, &targetFrame.position);
    PSVECAdd(targetPosition, &lbl_1_rodata_228, &targetFrame.target);
    targetFrame.up.x = 0.0f;
    targetFrame.up.y = 1.0f;
    targetFrame.up.z = 0.0f;
    get_camera_pose_from_frame(&view, &currentTransform);
    get_camera_pose_from_frame(&targetFrame, &targetTransform);
    interpolate_camera_poses(&view,
        &currentTransform,
        &targetTransform, 0.05f);
    cameraApplyToHud(&view, 1);
}

typedef void (*Callback)(void);

typedef void (CameraManager::*CallbackState)();

extern const CallbackState lbl_1_data_F0;

extern const CallbackState lbl_1_data_3B4;

/* Selects the tracked player and starts the camera follow delay. */
void CameraManager::fn_1_8C40(CPlayer *target)
{
    CallbackState callback;

    motionFrame = 0;
    this->target = target;
    callback = &CameraManager::fn_1_8B2C;
    this->callback = callback;
}

/* Selects the audio listener update behavior for the requested camera mode. */
void CameraManager::flagEnabled(int mode)
{
    switch (mode) {
    case 0: {
        Vec cameraPosition;
        Vec heading;
        f32 z;
        f32 distance;
        void (CameraManager::*callback)();

        fn_1_8F04();
        fn_1_8740(&cameraPosition, &heading);
        z = cameraPosition.z;
        if (z < 0.0f) {
            distance = -z;
        } else {
            distance = z;
        }
        HuAudFXListnerSet(&cameraPosition, &heading, 1500.0f + distance, 1000.0f);
        this->listenerActive = 1;
        callback = &CameraManager::fn_1_89D4;
        this->callback = callback;
        return;
    }
    case 1:
        fn_1_8F6C();
        flags &= ~(1UL << mode);
        return;
    case 2: {
        void (CameraManager::*callback)();
        callback = &CameraManager::fn_1_8AFC;
        this->callback = callback;
        flags &= ~(1UL << mode);
        return;
    }
    case 3:
        flags &= ~(1UL << mode);
        return;
    case 4:
        flags &= ~(1UL << mode);
    }
}

/* Runs the listener callback, then updates its position and heading from camera 1. */
void CameraManager::update()
{
    if (this->callback) {
        (this->*this->callback)();
    }
    if (this->listenerActive) {
        Vec listenerPosition;
        Vec heading;

        fn_1_8740(&listenerPosition, &heading);
        HuAudFXListnerUpdate(&listenerPosition, &heading);
    }
}

/* Loads the camera motion and attaches it to camera 1. */
void CameraManager::fn_1_8F04()
{
    const s16 motionId = fn_8002C4C0(fn_80008DD0(0x6B0004, 0x10000000, 2));
    modelId = fn_800283F4(motionId, 1);
    fn_800284C8(modelId, 1);
}

/* Kills the camera motion model and clears its ID. */
void CameraManager::fn_1_8F6C() {
    fn_800262D0(modelId);
    modelId = -1;
}
