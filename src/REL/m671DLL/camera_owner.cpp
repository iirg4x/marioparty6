#include "REL/m671DLL/member_callback.h"
#include "dolphin/types.h"

#include "REL/framework/model.h"
#include "REL/framework/random.h"

#include "REL/m671DLL/scene_object.h"

#include "REL/framework/minigame_core.h"

#include "REL/framework/thread.h"

#include "REL/m671DLL/camera_motion.h"

#include "game/hu3d.h"

#include "REL/framework/event_callback_inline.h"

// The game never reads this value.
extern const double lbl_1_rodata_3C0 = 60.0;
// The game never reads this value.
extern const Vec lbl_1_rodata_3C8[3] = {
    {0.0f, 1500.0f, 1500.0f},
    {0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f}
};

template<class Payload> void fn_1_BC1C(EventHandlerManager *owner, const Payload &payload, int key)
{
    owner->addHandler(new MemberCallback<CameraOwner>(payload), key);
}

void fn_1_BAB0(CameraOwner *, f32);

inline const Vec &CameraVector::getValue() const
{
    ASSERT(FALSE);
    return value;
}

CameraOwner::CameraOwner()
    : GameObject(3), model(0x820003), callbackState(1)
{
    s16 modelId;
    const HU3D_CAMERA *source;
    CameraWork *camera;

    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 45.0f, 1.0f, 10000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(1, 0, 0, 0x280, 0x1E0);
    model.setAttribute(1);
    modelId = (s16)this->model.modelId;
    fn_8002803C(modelId, 1);

    source = Hu3DCamera;
    camera = &this->camera;
    camera->cameraPosition.value.x = source->pos.x;
    camera->cameraPosition.value.y = source->pos.y;
    camera->cameraPosition.value.z = source->pos.z;
    camera->setPosition(camera->cameraPosition);
    camera->frame.up.x = source->up.x;
    camera->frame.up.y = source->up.y;
    camera->frame.up.z = source->up.z;
    camera->cameraTarget.value.x = source->target.x;
    camera->cameraTarget.value.y = source->target.y;
    camera->cameraTarget.value.z = source->target.z;
    camera->setTarget(camera->cameraTarget);
    camera->enabled = 0;
    this->cameraCopy = this->camera;
    this->fade = 0.0f;

    fn_1_BC1C(&this->callbackState,
             std::bind1st(std::mem_fun(&CameraOwner::fn_1_BA5C), this), 0);
    this->setUpdate(&CameraOwner::fn_1_B81C);
}

void CameraOwner::update()
{
    if (this->updateCallback) {
        (this->*this->updateCallback)();
    }
    cameraApplyToHud(&this->camera.frame, 1);
}

void CameraOwner::fn_1_B81C() {}

void CameraOwner::fn_1_B820()
{
    f64 xOffset;
    f64 zOffset;
    f32 blendFactor;
    Vec movement;

    CameraWork *targets = &camera;
    CameraWork *results = &cameraCopy;

    zOffset = 100.0f *
              RandomFloat(-0.5f, 0.5f);
    xOffset = 100.0f *
              RandomFloat(-0.5f, 0.5f);
    movement.x = xOffset;
    movement.y = 0.0f;
    movement.z = zOffset;
    blendFactor = this->fade;
    PSVECScale(&movement, &movement, blendFactor);
    PSVECAdd(&movement, &results->frame.position, &targets->cameraPosition.value);
    PSVECAdd(&movement, &results->frame.target, &targets->cameraTarget.value);
    fn_1_BAB0(this, 0.5f);
    this->fade = (f32)((double)this->fade - 0.03333333333333333);
    if (!(this->fade > 0.1f)) {
        camera = cameraCopy;
        setUpdate(&CameraOwner::fn_1_B81C);
    }
}

void CameraOwner::fn_1_BA5C(EventMessage *)
{
    fade = 1.0f;
    setUpdate(&CameraOwner::fn_1_B820);
}

void fn_1_BAB0(CameraOwner *state, f32 amount)
{
    CameraWork *first = &state->camera;
    Vec *firstValue = &first->frame.position;
    Vec *second = &first->frame.target;

    firstValue->x += amount * (first->cameraPosition.value.x - firstValue->x);
    firstValue->y += amount * (first->cameraPosition.value.y - firstValue->y);
    firstValue->z += amount * (first->cameraPosition.value.z - firstValue->z);
    second->x += amount * (first->cameraTarget.value.x - second->x);
    second->y += amount * (first->cameraTarget.value.y - second->y);
    second->z += amount * (first->cameraTarget.value.z - second->z);
}

CameraOwner::~CameraOwner() {}
