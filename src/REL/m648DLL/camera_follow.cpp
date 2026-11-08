#include "REL/m648DLL/time_conversion.h"
#include "REL/m648DLL/camera_frame.h"
extern "C" {
#include "dolphin/types.h"
}
#include "dolphin/mtx.h"
#include "game/memory.h"
#include "REL/m648DLL/framework/model.h"
#include "REL/m648DLL/motion_lists.h"
extern "C" {
#include "game/audio.h"
}
#include <math.h>
extern "C" {
void Hu3DBGColorSet(u8 r, u8 g, u8 b);
void fn_800262D0(s16 modelId);
void Hu3DCameraCreate(int cameraBit);
void fn_80027CD0(int cameraBit, Vec *pos, Vec *up, Vec *target);
s16 fn_800283F4(s32 motionId, s32 flags);
void fn_800284C8(s16 cameraModelId, s32 flags);
s16 fn_8002C4C0(void *motionData);
BOOL fn_8002D75C(s16 modelId);
#include "game/data.h"
#include "REL/framework/assertion.h"
}
#include "REL/m648DLL/camera_view.h"
#include "REL/m648DLL/list_object.h"
void fn_1_BC84();

#include "REL/m648DLL/camera_manager.h"

typedef void (CameraManager::*CameraManagerCallback)();
extern "C" {
void __ct__4NodeFv(void *);
void insert4__4NodeFP4Node(void *, void *);
void __ct__11CameraSetupFv(void *);

}

extern "C" {
extern s32 lbl_1_bss_40;
extern s32 lbl_1_bss_48;
extern s32 fn_1_8024(s32);
extern double fn_1_4B4C(double);

extern s32 lbl_1_bss_278;

extern s32 lbl_1_bss_27C;
extern s32 lbl_1_bss_280;

}

void fn_1_8740(Vec *position, Vec *direction) {
    Vec up;
    Vec target;

    fn_80027CD0(1, position, &up, &target);
    PSVECSubtract(&target, position, direction);
    PSVECNormalize(direction, direction);
}

CameraManager::CameraManager()
{
    callback = &CameraManager::fn_1_8ACC;
    target = 0;
    modelId = -1;
    listenerActive = 0;
    __ct__11CameraSetupFv(cameraSetup);
    Hu3DCameraCreate(1);
    Hu3DBGColorSet(0, 0, 0);
}

extern "C" {

void *__dt__4NodeFv(void *object, s16 mode);
void __dl__FPv(void *object);
}

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
extern "C" {
}

void CameraManager::fn_1_8A68()
{
    if (--timer < 0) {
        CameraManagerCallback nextCallback = &CameraManager::fn_1_8ACC;
        callback = nextCallback;
        state &= ~1;
    }
}

void CameraManager::fn_1_8ACC() { cameraApplyToHud(&view, 1); }

void CameraManager::fn_1_8AFC() { cameraApplyToHud(&view, 1); }

extern "C" {
void Hu3DBGColorSet(u8 r, u8 g, u8 b);
void fn_800262D0(s16 modelId);
void Hu3DCameraCreate(int cameraBit);
void fn_80027CD0(int cameraBit, Vec *pos, Vec *up, Vec *target);
s16 fn_800283F4(s32 motionId, s32 flags);
void fn_800284C8(s16 cameraModelId, s32 flags);
s16 fn_8002C4C0(void *motionData);
BOOL fn_8002D75C(s16 modelId);
}

struct CameraPose {
    Vec position;
    float rotation[4];
};

struct CameraTransforms {
    CameraPose targetTransform;
    CameraPose currentTransform;
    CameraFrame targetFrame;
};

struct CameraTarget {
    s32 unknownFirst;
    s32 unknownSecond;
    Vec position;
};

void get_camera_pose_from_frame(const CameraFrame *, CameraPose *);

/* Initialized bytes of unknown meaning; the camera vectors follow at 0x21C. */
extern "C" const u8 lbl_1_rodata_200[0x1C] = {
    0x00, 0x00, 0x00, 0x00, 0x45, 0x3B, 0x60, 0x77, 0x45, 0x33, 0x44, 0xA8, 0xBE, 0xD1, 0x9D, 0xB6, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x3F, 0x69, 0x8F, 0xEA
};

const Vec lbl_1_rodata_21C = { 0.0f, 300.0f, 600.0f };

const Vec lbl_1_rodata_228 = { 0.0f, 200.0f, 0.0f };

void CameraManager::fn_1_8B2C() {
    CameraTransforms temporary;

    if ((float)(++motionFrame) < 90.0f) {
        return;
    }

    Vec *targetPosition = &reinterpret_cast<CameraTarget *>(target)->position;
    PSVECAdd(targetPosition, &lbl_1_rodata_21C, &temporary.targetFrame.position);
    PSVECAdd(targetPosition, &lbl_1_rodata_228, &temporary.targetFrame.target);
    temporary.targetFrame.up.x = 0.0f;
    temporary.targetFrame.up.y = 1.0f;
    temporary.targetFrame.up.z = 0.0f;
    get_camera_pose_from_frame(&view, &temporary.currentTransform);
    get_camera_pose_from_frame(&temporary.targetFrame, &temporary.targetTransform);
    interpolate_camera_poses(&view,
        reinterpret_cast<const CameraPose *>(&temporary.currentTransform),
        reinterpret_cast<const CameraPose *>(&temporary.targetTransform), 0.05f);
    cameraApplyToHud(&view, 1);
}

typedef void (*Callback)(void);

typedef void (CameraManager::*CallbackState)();

extern const CallbackState lbl_1_data_F0;

extern const CallbackState lbl_1_data_3B4;

void CameraManager::fn_1_8C40(Node *target)
{
    CallbackState callback;

    motionFrame = 0;
    this->target = target;
    callback = &CameraManager::fn_1_8B2C;
    this->callback = callback;
}

static inline void fn_1_17D0(CameraManager *owner, s32 request)
{
    owner->state &= ~(1UL << request);
}

/* Selects the audio listener update behavior for the requested camera mode. */
void CameraManager::clearRequest(int mode)
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
        fn_1_17D0(this, mode);
        return;
    case 2: {
        void (CameraManager::*callback)();
        callback = &CameraManager::fn_1_8AFC;
        this->callback = callback;
        fn_1_17D0(this, mode);
        return;
    }
    case 3:
        fn_1_17D0(this, mode);
        return;
    case 4:
        fn_1_17D0(this, mode);
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
        Vec cameraTarget;
        Vec cameraUp;

        fn_80027CD0(1, &listenerPosition, &cameraUp, &cameraTarget);
        PSVECSubtract(&cameraTarget, &listenerPosition, &heading);
        PSVECNormalize(&heading, &heading);
        HuAudFXListnerUpdate(&listenerPosition, &heading);
    }
}

void CameraManager::fn_1_8F04()
{
    s16 motionId = fn_8002C4C0(fn_80008DD0(0x6B0004, 0x10000000, 2));
    modelId = fn_800283F4(motionId, 1);
    fn_800284C8(modelId, 1);
}

void CameraManager::fn_1_8F6C() {
    fn_800262D0(modelId);
    modelId = -1;
}
