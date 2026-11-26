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

const Vec lbl_1_rodata_358[2] = { { 0.0f, 100.0f, 1300.0f }, { 0.0f, 20000.0f, 2500.0f } };

const Vec lbl_1_rodata_370[2] = { { 0.0f, 100.0f, 0.0f }, { 0.0f, 20000.0f, 0.0f } };

const Vec lbl_1_rodata_388 = { 0.0f, 1.0f, 0.0f };

/* Creates the four camera views and installs their projection settings. */
CameraOwner::CameraOwner() : elapsed(0.0f), mode(1), callback(&CameraOwner::fn_1_7CE0)
{
    f32 fieldOfView;
    f32 nightFieldOfView;
    s32 cameraIndex;
    state = GWMgNightFGet();
    for (cameraIndex = 0; cameraIndex < 4; cameraIndex++)
    {
        CameraWork * camera;
        fieldOfView = 0.29625002f;
        camera = & cameras[cameraIndex];
        camera->frame.position = camera->cameraPosition.value = lbl_1_rodata_358[state];
        camera->frame.up = lbl_1_rodata_388;
        camera->frame.target = camera->cameraTarget.value = lbl_1_rodata_370[state];
        camera->enabled = 0;
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

/* Eases each camera toward its player's position and target during play. */
void CameraOwner::fn_1_7CE0()
{
    s32 index = 0;
    while (index < 4) {
        CameraWork *position = &cameras[index];
        fn_1_87B4(this, index, 12.0f);
        cameraApplyToHud(&position->frame, cameraMasks[index]);
        index++;
    }
}

/* Shrinks the main daytime view during the opening and reveals the other three views. */
void CameraOwner::fn_1_7D6C()
{
    s32 camera;
    f32 viewportWidth;
    f32 cameraWidth = 1.2f;
    f32 left = 0.0f;
    f32 previousLeft = 0.0f;
    f32 previousWidth;

    for (camera = 0; camera < 4; camera++) {
        CameraWork *entry = &this->cameras[camera];
        fn_1_87B4(this, camera, 12.0f);
        cameraApplyToHud(&entry->frame, cameraMasks[camera]);
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
                entry->cameraPosition.value.y = entry->cameraTarget.value.y = 100.0f +
                    (2.0f *
                     (200.0f *
                      (this->elapsed - 2.5f)));
                entry->cameraPosition.value.z = 1300.0f +
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

/* Moves the night cameras to their opening targets with all four viewports visible. */
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
            CameraWork *entry = &this->cameras[camera];
            fn_1_87B4(this, camera, 12.0f);
            cameraApplyToHud(&entry->frame, cameraMasks[camera]);
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

/* Eases the cameras toward their results positions with a slower follow rate. */
void CameraOwner::fn_1_84B4()
{
    s32 index = 0;
    while (index < 4) {
        CameraWork *position = &cameras[index];
        fn_1_87B4(this, index, 60.0f);
        cameraApplyToHud(&position->frame, cameraMasks[index]);
        index++;
    }
}

// The game's requests: 0 starts the opening (the opening effect by day), 1 lets the cameras follow the players,
// 3 holds them on the result.
/* Selects the opening or results camera move and acknowledges camera requests. */
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

/* Runs the current camera move. */
void CameraOwner::update()
{
    if (callback != 0) {
        (this->*callback)();
    }
}

/* Updates one camera's desired target and position once player following begins. */
void fn_1_8730(CameraOwner *owner, s32 index, const Vec *target, const Vec *position)
{
    CameraWork *record;

    if (owner->mode != 1) {
        record = &owner->cameras[index];
        record->cameraTarget.value = *target;
        record->cameraPosition.value = *position;
    }
}

/* Returns the selected camera's current position. */
Vec CameraOwner::fn_1_878C(s32 index) const
{
    return cameras[index].frame.position;
}

/* Moves the selected camera by a fraction of its remaining target and position deltas. */
void fn_1_87B4(CameraOwner *records, s32 index, f32 divisor)
{
    CameraWork *record = &records->cameras[index];
    Vec *firstValue = &record->frame.position;
    Vec *second = &record->frame.target;

    firstValue->x += (record->cameraPosition.value.x - firstValue->x) / divisor;
    firstValue->y += (record->cameraPosition.value.y - firstValue->y) / divisor;
    firstValue->z += (record->cameraPosition.value.z - firstValue->z) / divisor;
    second->x += (record->cameraTarget.value.x - second->x) / divisor;
    second->y += (record->cameraTarget.value.y - second->y) / divisor;
    second->z += (record->cameraTarget.value.z - second->z) / divisor;
}

/* Returns the horizontal bearing from the target to the camera, in degrees. */
f32 fn_1_8894(CameraOwner *owner, s32 index)
{
    Vec difference;
    const CameraWork *record = &owner->cameras[index];

    PSVECSubtract(&record->frame.position, &record->frame.target, &difference);
    return (f32)(180.0 * (atan2((f64)difference.x, (f64)difference.z) / M_PI));
}

/* Leaves the four camera views active at destruction. */
CameraOwner::~CameraOwner()
{
}
