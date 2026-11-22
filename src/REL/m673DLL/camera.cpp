// Creates and updates the scene camera.

#include "REL/framework/event_callback_registration.h"
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "REL/framework/random.h"
#include "REL/framework/camera_frame.h"
extern "C" {
#include "game/hu3d.h"
#include "datadir_enum.h"
}
#include "REL/m673DLL/scene_camera.h"
#include <new>

// No code in this module reads these three vectors.
const Vec lbl_1_rodata_530 = { 0.0f, 1500.0f, 1500.0f };
const Vec lbl_1_rodata_53C = { 0.0f, 0.0f, 0.0f };
const Vec lbl_1_rodata_548 = { 0.0f, 1.0f, 0.0f };

// Wraps a camera event member function.
static inline CameraEventMethod CameraMember(CameraEventCallback function)
{
    return CameraEventMethod(function);
}

// Binds a camera event method to its camera instance.
static inline BoundCameraEvent BindCamera(const CameraEventMethod &callback,
                                          SceneCamera *owner)
{
    return BoundCameraEvent(callback, owner);
}

// Sets the initial camera frame and registers its scene event.
SceneCamera::SceneCamera()
    : GameObject(5), model(DATANUM(0x840000, 0x04)), channel1Events(1)
{
    s16 modelId;
    const HU3D_CAMERA *camera;
    CameraWork *ownerCamera;
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 45.0f, 1.0f, 10000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(1, 0, 0, 640, 480);
    this->model.setAttribute(1);
    modelId = (s16)this->model.modelId;
    fn_8002803C(modelId, 1);
    camera = Hu3DCamera;
    ownerCamera = &this->camera;
    ownerCamera->cameraPosition.value.x = camera->pos.x;
    ownerCamera->cameraPosition.value.y = camera->pos.y;
    ownerCamera->cameraPosition.value.z = camera->pos.z;
    ownerCamera->setPosition(ownerCamera->cameraPosition);
    ownerCamera->frame.up.x = camera->up.x;
    ownerCamera->frame.up.y = camera->up.y;
    ownerCamera->frame.up.z = camera->up.z;
    ownerCamera->cameraTarget.value.x = camera->target.x;
    ownerCamera->cameraTarget.value.y = camera->target.y;
    ownerCamera->cameraTarget.value.z = camera->target.z;
    ownerCamera->setTarget(ownerCamera->cameraTarget);
    ownerCamera->enabled = 0;
    this->cameraCopy = this->camera;
    this->fade = 0.0f;
    registerEventCallback(&this->channel1Events,
        BindCamera(CameraMember(&SceneCamera::fn_1_C690), this), 0);
    setState(&SceneCamera::fn_1_C450);
}

// Runs the current camera callback and applies the camera frames.
void SceneCamera::update()
{
    if (state) {
        (this->*state)();
    }
    cameraApplyToHud(&camera.frame, 1);
    debugResetHook(&debugCamera.frame, 0, 1);
}

// Leaves the camera frame unchanged while idle.
void SceneCamera::fn_1_C450()
{
}

// Applies a decaying random camera offset and restores the saved camera state.
void SceneCamera::fn_1_C454()
{
    f64 xOffset;
    f64 zOffset;
    f32 blendFactor;
    Vec movement;
    CameraWork *targets = &camera;
    CameraWork *results = &cameraCopy;

    zOffset = 100.0f * RandomFloat(-0.5f, 0.5f);
    xOffset = 100.0f * RandomFloat(-0.5f, 0.5f);
    movement.x = xOffset;
    movement.y = 0.0f;
    movement.z = zOffset;
    blendFactor = fade;
    PSVECScale(&movement, &movement, blendFactor);
    PSVECAdd(&movement, &results->frame.position, &targets->cameraPosition.value);
    PSVECAdd(&movement, &results->frame.target, &targets->cameraTarget.value);
    fn_1_C6E4(0.5f);
    fade = fade - 2.0 / 60.0;
    if (!(fade > 0.1f)) {
        camera = cameraCopy;
        setState(&SceneCamera::fn_1_C450);
    }
}

// Starts the camera shake callback.
void SceneCamera::fn_1_C690(const EventPacket &)
{
    fade = 1.0f;
    setState(&SceneCamera::fn_1_C454);
}

// Moves the camera position and target toward their desired values.
void SceneCamera::fn_1_C6E4(f32 amount)
{
    CameraWork *first = &camera;
    Vec *firstValue = &first->frame.position;
    Vec *second = &first->frame.target;

    firstValue->x += amount * (first->cameraPosition.value.x - firstValue->x);
    firstValue->y += amount * (first->cameraPosition.value.y - firstValue->y);
    firstValue->z += amount * (first->cameraPosition.value.z - firstValue->z);
    second->x += amount * (first->cameraTarget.value.x - second->x);
    second->y += amount * (first->cameraTarget.value.y - second->y);
    second->z += amount * (first->cameraTarget.value.z - second->z);
}

// Releases the event handlers and camera vector state.
SceneCamera::~SceneCamera()
{
}
