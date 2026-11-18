extern "C"
{
#include "game/data.h"
#include "game/hu3d.h"
}
#include <new>
#include "REL/framework/event_callback_registration.h"
#include "REL/m628DLL/camera_owner.h"
#include <functional>
#include "REL/framework/event_callback_adapter_inline.h"
#include "REL/framework/event_callback_inline.h"

#include "REL/framework/model.h"

#include "REL/framework/event_hm.h"
#include "REL/framework/game_object.h"

#include "REL/framework/camera_frame.h"

void fn_1_13930(void);

// Copies a camera vector.
static inline Vec *copyCameraVector(Vec *destination, const Vec *source)
{
    destination->x = source->x;
    destination->y = source->y;
    destination->z = source->z;
    return destination;
}

extern "C" const u16 lbl_1_rodata_3B8[4] = {0x0001, 0x0002, 0x0004, 0x0008};

// Creates the camera model and installs the opening camera state.
CameraOwner::CameraOwner()
    : GameObject(0x10), cameraModel(DATANUM(0x570000, 4)), elapsed(0.0f), state(0),
      callback(&CameraOwner::fn_1_13788), callbackStorage(1)
{
    CameraWork *view;
    HU3D_CAMERA *hardwareCamera;
    s16 modelId;
    this->cameraModel.setAttribute(1 << 0);
    Hu3DCameraCreate(1);
    modelId = (s16)this->cameraModel.modelId;
    fn_8002803C(modelId, 1);
    hardwareCamera = Hu3DCamera;
    view = &this->camera;
    Vec *cameraPosition = copyCameraVector(&view->cameraPosition.value, &hardwareCamera->pos);
    copyCameraVector(&view->frame.position, cameraPosition);
    view->frame.up.x = hardwareCamera->up.x;
    view->frame.up.y = hardwareCamera->up.y;
    view->frame.up.z = hardwareCamera->up.z;
    Vec *cameraTarget = copyCameraVector(&view->cameraTarget.value, &hardwareCamera->target);
    copyCameraVector(&view->frame.target, cameraTarget);
    view->enabled = 0;
    this->cameraCopy.cameraPosition.value.x = hardwareCamera->pos.x;
    this->cameraCopy.cameraPosition.value.y = hardwareCamera->pos.y;
    this->cameraCopy.cameraPosition.value.z = hardwareCamera->pos.z;
    this->cameraCopy.cameraTarget.value.x = hardwareCamera->target.x;
    this->cameraCopy.cameraTarget.value.y = hardwareCamera->target.y;
    this->cameraCopy.cameraTarget.value.z = hardwareCamera->target.z;
    registerEventCallback(
        &this->callbackStorage, std::bind1st(std::mem_fun(&CameraOwner::fn_1_1397C), this), 0);
}

// Interpolates the camera and applies it to the HUD.
inline void CameraOwner::fn_1_13788(void)
{
    CameraWork *cameraView = &camera;
    fn_1_13A98(this, 0.05f);
    cameraApplyToHud(&cameraView->frame, 1);
}

// Advances the camera transition after the opening event.
void CameraOwner::fn_1_137DC(void)
{
    switch (this->state)
    {
    case 0:
    {
        if (this->elapsed < 1.5f)
        {
            break;
        }
        HU3D_CAMERA *hardwareCamera = Hu3DCamera;
        CameraWork *view = &this->camera;
        view->cameraPosition.value = this->cameraCopy.cameraPosition.value;
        view->cameraTarget.value = this->cameraCopy.cameraTarget.value;
        this->state++;
    }
    case 1:
        if (!(this->elapsed < 2.0f))
        {
            this->setCallback(&CameraOwner::fn_1_13788);
            this->state++;
        }
        break;
    }
    this->fn_1_13788();
    this->elapsed += (1.0 / 60.0);
}

// Leaves the camera unchanged.
void fn_1_13930(void) {}

// Runs the selected camera state.
void CameraOwner::update()
{
    if (callback)
    {
        (this->*callback)();
    }
}

extern "C"
{
f32 lbl_1_data_86C[3] = {1000.0f, 1500.0f, 1500.0f};
}

extern "C"
{
f32 lbl_1_data_878[3] = {0.0f, -250.0f, 0.0f};
}

// Restores the default camera position and target.
void CameraOwner::fn_1_1397C(const EventPacket &)
{
    HU3D_CAMERA *hardwareCamera = Hu3DCamera;
    CameraWork *cameraView = &this->camera;
    Vec *positionGoal;
    Vec *positionSource;
    Vec *positionComponents;
    Vec *targetGoal;
    Vec *targetSource;
    Vec *targetComponents;

    cameraView->cameraPosition.value.x = lbl_1_data_86C[0];
    cameraView->cameraPosition.value.y = lbl_1_data_86C[1];
    cameraView->cameraPosition.value.z = lbl_1_data_86C[2];
    positionGoal = &cameraView->cameraPosition.value;
    positionSource = positionGoal;
    positionComponents = positionSource;
    cameraView->frame.position.x = positionComponents->x;
    cameraView->frame.position.y = positionComponents->y;
    cameraView->frame.position.z = positionComponents->z;
    cameraView->cameraTarget.value.x = lbl_1_data_878[0];
    cameraView->cameraTarget.value.y = lbl_1_data_878[1];
    cameraView->cameraTarget.value.z = lbl_1_data_878[2];
    targetGoal = &cameraView->cameraTarget.value;
    targetSource = targetGoal;
    targetComponents = targetSource;
    cameraView->frame.target.x = targetComponents->x;
    cameraView->frame.target.y = targetComponents->y;
    cameraView->frame.target.z = targetComponents->z;
    this->setCallback(&CameraOwner::fn_1_137DC);
}

// Moves the camera position and target toward their goals.
void fn_1_13A98(CameraOwner *owner, f32 amount)
{
    CameraWork *work = &owner->camera;
    Vec *position = &work->frame.position;
    Vec *target = &work->frame.target;

    position->x += amount * (work->cameraPosition.value.x - position->x);
    position->y += amount * (work->cameraPosition.value.y - position->y);
    position->z += amount * (work->cameraPosition.value.z - position->z);
    target->x += amount * (work->cameraTarget.value.x - target->x);
    target->y += amount * (work->cameraTarget.value.y - target->y);
    target->z += amount * (work->cameraTarget.value.z - target->z);
}

// Releases the camera model and event registration.
CameraOwner::~CameraOwner() {}
