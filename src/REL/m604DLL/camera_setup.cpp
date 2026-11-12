#include "REL/m604DLL/camera_masks.h"
#include "REL/m604DLL/camera_effect_owner.h"
#include "REL/m604DLL/camera_owner.h"
#include "dolphin/types.h"
#include "humath.h"
extern "C" {
#include "math.h"
#include "game/hu3d.h"
#include "game/gamework.h"
}

/* Creates the four camera views and installs their projection settings. */

const Vec lbl_1_rodata_358[2] = { { 0.0f, 100.0f, 1300.0f }, { 0.0f, 20000.0f, 2500.0f } };

const Vec lbl_1_rodata_370[2] = { { 0.0f, 100.0f, 0.0f }, { 0.0f, 20000.0f, 0.0f } };

const Vec lbl_1_rodata_388 = { 0.0f, 1.0f, 0.0f };

CameraOwner::CameraOwner() : elapsed(0.0f), mode(1), callback(&CameraOwner::fn_1_7CE0)
{
    f32 fieldOfView;
    f32 nightFieldOfView;
    s32 cameraIndex;
    state = GWMgNightFGet();
    for (cameraIndex = 0; cameraIndex < 4; cameraIndex++)
    {
        CameraView * camera;
        fieldOfView = 0.29625002f;
        camera = & cameras[cameraIndex];
        camera->position = camera->desiredPosition = lbl_1_rodata_358[state];
        camera->up = lbl_1_rodata_388;
        camera->target = camera->desiredTarget = lbl_1_rodata_370[state];
        camera->active = 0;
        Hu3DCameraCreate(cameraMasks[cameraIndex]);
        Hu3DCameraPerspectiveSet(cameraMasks[cameraIndex], 30.0f, 1.0f, 10000.0f, fieldOfView);
        Hu3DCameraViewportSet(cameraMasks[cameraIndex], 640.0f, 0.0f, 0.0f, 480.0f, 0.0f, 1.0f);
        Hu3DCameraScissorSet(cameraMasks[cameraIndex], 0x280, 0, 0, 0x1E0);
    }
    if (state == 0)
    {
        nightFieldOfView = 1.1962501f;
        Hu3DCameraPerspectiveSet(cameraMasks[0], 30.0f, 1.0f, 10000.0f, nightFieldOfView);
        Hu3DCameraViewportSet(cameraMasks[0], 1.0f, 0.0f, 638.0f, 480.0f, 0.0f, 1.0f);
        Hu3DCameraScissorSet(cameraMasks[0], 1, 0, 0x27E, 0x1E0);
    }
}

static inline void setCallback(CameraOwner *owner, CameraOwner::Callback next)
{
    owner->callback = next;
}
void CameraOwner::fn_1_7CE0()
{
    s32 index = 0;
    while (index < 4) {
        CameraView *position = &cameras[index];
        fn_1_87B4(this, index, 12.0f);
        cameraApplyToHud(position, cameraMasks[index]);
        index++;
    }
}

void CameraOwner::fn_1_7D6C()
{
    s32 camera;
    f32 viewportWidth;
    f32 cameraWidth = 1.2f;
    f32 left = 0.0f;
    f32 previousLeft = 0.0f;
    f32 previousWidth;

    for (camera = 0; camera < 4; camera++) {
        CameraView *entry = &this->cameras[camera];
        fn_1_87B4(this, camera, 12.0f);
        cameraApplyToHud(entry, cameraMasks[camera]);
        if (this->elapsed >= 2.0f) {
            if (camera == 0) {
                viewportWidth = 0.5f +
                    (640.0f -
                     (482.0f * (this->elapsed - 2.0f)));
                cameraWidth = viewportWidth / 480.0f;
                cameraWidth *= 0.9f;
                left = 1.0f;
            } else {
                viewportWidth = 0.5f +
                    (158.0f * (this->elapsed - 2.0f));
                cameraWidth = 0.29625002f;
                left = previousLeft + previousWidth;
            }
            Hu3DCameraPerspectiveSet(cameraMasks[camera], 30.0f, 1.0f,
                                     10000.0f, cameraWidth);
            Hu3DCameraViewportSet(cameraMasks[camera], left,
                                  0.0f, viewportWidth,
                                  480.0f, 0.0f,
                                  1.0f);
            Hu3DCameraScissorSet(cameraMasks[camera], left, 0,
                                 viewportWidth, 0x1E0);
            if (this->elapsed > 2.5f) {
                entry->desiredPosition.y = entry->desiredTarget.y = 100.0f +
                    (2.0f *
                     (200.0f *
                      (this->elapsed - 2.5f)));
                entry->desiredPosition.z = 1300.0f +
                    (2.0f *
                     (1200.0f *
                      (this->elapsed - 2.5f)));
            }
            previousWidth = (f32)(s32)viewportWidth;
            previousLeft = 2.0f + (f32)(s32)left;
        }
    }
    this->elapsed += 0.016666666666666666;
    if (this->elapsed <= 3.0f) {
        return;
    }
    this->flags &= ~1;
    setCallback(this, &CameraOwner::fn_1_7CE0);
}

void CameraOwner::fn_1_81B4()
{
    f32 widthStep;
    f32 left;
    f32 width;
    f32 aspect;
    f32 viewportBase;
    s32 camera;

    if (0.0f == this->elapsed) {
        width = 158.0f;
        aspect = 0.29625002f;
        left = 0.0f;
        viewportBase = 0.0f;
        for (camera = 0; camera < 4; camera++) {
            CameraView *entry = &this->cameras[camera];
            fn_1_87B4(this, camera, 12.0f);
            cameraApplyToHud(entry, cameraMasks[camera]);
            if (camera == 0) {
                left = 1.0f;
            } else {
                left = viewportBase + widthStep;
            }
            Hu3DCameraPerspectiveSet(cameraMasks[camera], 30.0f,
                                     1.0f, 10000.0f,
                                     aspect);
            Hu3DCameraViewportSet(cameraMasks[camera], left,
                                  0.0f, width, 480.0f,
                                  0.0f, 1.0f);
            Hu3DCameraScissorSet(cameraMasks[camera], left, 0,
                                 width, 0x1E0);
            widthStep = (f32)(s32)width;
            viewportBase = 2.0f + (f32)(s32)left;
        }
    }
    this->elapsed += 0.016666666666666666;
    if (this->elapsed <= 3.0f) {
        return;
    }
    this->flags &= ~1;
    setCallback(this, &CameraOwner::fn_1_7CE0);
}

void CameraOwner::fn_1_84B4()
{
    s32 index = 0;
    while (index < 4) {
        CameraView *position = &cameras[index];
        fn_1_87B4(this, index, 60.0f);
        cameraApplyToHud(position, cameraMasks[index]);
        index++;
    }
}

// The game's requests: 0 starts the opening (the opening effect by day), 1 lets the cameras follow the players,
// 3 holds them on the result.
void CameraOwner::flagEnabled(int bit)
{
    switch (bit)
    {
    case 0 :
        if (state == 0)
        {
            new CameraEffectOwner;
            setCallback(this, & CameraOwner::fn_1_7D6C);
            return;
        }
        {
            setCallback(this, & CameraOwner::fn_1_81B4);
        }
        return;
    case 1 :
        mode = 0;
        flags &= ~(1U << bit);
        return;
    case 2 :
        flags &= ~(1U << bit);
        return;
    case 3 :
        {
            setCallback(this, & CameraOwner::fn_1_84B4);
        }
        flags &= ~(1U << bit);
        return;
    case 4 :
        flags &= ~(1U << bit);
    }
}

void CameraOwner::update()
{
    if (callback != 0) {
        (this->*callback)();
    }
}

void fn_1_8730(CameraOwner *owner, s32 index, const Vec *target, const Vec *position)
{
    CameraView *record;

    if (owner->mode != 1) {
        record = &owner->cameras[index];
        record->desiredTarget = *target;
        record->desiredPosition = *position;
    }
}

Vec CameraOwner::fn_1_878C(s32 index) const
{
    return cameras[index].position;
}

void fn_1_87B4(CameraOwner *records, s32 index, f32 divisor)
{
    CameraView *record = &records->cameras[index];
    Vec *firstValue = &record->position;
    Vec *second = &record->target;

    firstValue->x += (record->desiredPosition.x - firstValue->x) / divisor;
    firstValue->y += (record->desiredPosition.y - firstValue->y) / divisor;
    firstValue->z += (record->desiredPosition.z - firstValue->z) / divisor;
    second->x += (record->desiredTarget.x - second->x) / divisor;
    second->y += (record->desiredTarget.y - second->y) / divisor;
    second->z += (record->desiredTarget.z - second->z) / divisor;
}

f32 fn_1_8894(CameraOwner *owner, s32 index)
{
    Vec difference;
    const CameraView *record = &owner->cameras[index];

    PSVECSubtract(&record->position, &record->target, &difference);
    return (f32)(180.0 * (atan2((f64)difference.x, (f64)difference.z) / M_PI));
}

CameraOwner::~CameraOwner()
{
}

