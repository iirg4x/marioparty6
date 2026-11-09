#include "REL/m672DLL/random.h"
#include "REL/framework/minigame_core.h"
#include "REL/m672DLL/framework/thread.h"
#include "dolphin/types.h"
#include <string.h>
#include "REL/m672DLL/collision_object.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event_hm.h"
#include "REL/m672DLL/model_bf64.h"
#include "REL/framework/event_callback_inline.h"

#include "dolphin/os.h"
#include "dolphin/mtx.h"
#include "game/data.h"
#include "game/hu3d.h"
#include <stdio.h>
#include <stdlib.h>

/* Unknown initialized bytes between the camera constants. */
extern const unsigned char lbl_1_rodata_438[0x2C] = {
    0x40, 0x4E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x44, 0xBB, 0x80, 0x00,
    0x44, 0xBB, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
};

#include "REL/m672DLL/camera_owner_a348.h"

inline const Vec &CameraVector::getValue() const { ASSERT(this != 0); return value; }

extern "C" {

}

template<class Receiver> void fn_1_AAE4(EventHandlerManager *, const CameraCallbackPayload &, int);

CameraOwner::CameraOwner()
    : GameObject(4), model(DATANUM(0x830000, 3)), callbackState(1)
{
    s16 modelId;
    const HU3D_CAMERA *source;
    CameraWork *camera;

    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 45.0f, 1.0f, 10000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(1, 0, 0, 0x280, 0x1E0);
    model.fn_1_33950(1);
    modelId = (s16)model.modelId;
    fn_8002803C(modelId, 1);

    source = Hu3DCamera;
    camera = &this->camera;
    camera->cameraPosition.value.x = source->pos.x;
    camera->cameraPosition.value.y = source->pos.y;
    camera->cameraPosition.value.z = source->pos.z;
    camera->setPosition(camera->cameraPosition);
    camera->vectors[1].x = source->up.x;
    camera->vectors[1].y = source->up.y;
    camera->vectors[1].z = source->up.z;
    camera->cameraTarget.value.x = source->target.x;
    camera->cameraTarget.value.y = source->target.y;
    camera->cameraTarget.value.z = source->target.z;
    camera->setTarget(camera->cameraTarget);
    camera->enabled = 0;
    this->cameraCopy = this->camera;
    this->fade = 0.0f;

    fn_1_AAE4<CameraOwner>(&this->callbackState,
             BindCamera(CameraMember(&CameraOwner::fn_1_A924), this), 0);
    this->setUpdate(&CameraOwner::fn_1_A6E4);
}

void CameraOwner::update()
{
    if (updateCallback) {
        (this->*updateCallback)();
    }
    cameraApplyToHud(&camera, 1);
}

void CameraOwner::fn_1_A6E4() {}

void CameraOwner::fn_1_A6E8()
{
    double xOffset;
    double zOffset;
    float blendFactor;
    Vec movement;
    Vec *targets = camera.vectors;
    Vec *results = cameraCopy.vectors;

    zOffset = 100.0f * RandomFloat(-0.5f, 0.5f);
    xOffset = 100.0f * RandomFloat(-0.5f, 0.5f);
    movement.x = xOffset;
    movement.y = 0.0f;
    movement.z = zOffset;
    blendFactor = fade;
    PSVECScale(&movement, &movement, blendFactor);
    PSVECAdd(&movement, &results[0], &targets[3]);
    PSVECAdd(&movement, &results[2], &targets[4]);
    fn_1_A978(0.5f);
    fade = (float)((double)fade - 0.03333333333333333);
    if (!(fade > 0.1f)) {
        camera = cameraCopy;
        setUpdate(&CameraOwner::fn_1_A6E4);
    }
}

void CameraOwner::fn_1_A924(EventMessage *)
{
    fade = 1.0f;
    setUpdate(&CameraOwner::fn_1_A6E8);
}

void CameraOwner::fn_1_A978(float amount)
{
    Vec *first = camera.vectors;
    Vec *firstValue = first;
    Vec *second = first + 2;

    firstValue->x += amount * (first[3].x - firstValue->x);
    firstValue->y += amount * (first[3].y - firstValue->y);
    firstValue->z += amount * (first[3].z - firstValue->z);
    second->x += amount * (first[4].x - second->x);
    second->y += amount * (first[4].y - second->y);
    second->z += amount * (first[4].z - second->z);
}

CameraOwner::~CameraOwner() {}

template<class Receiver>
void fn_1_AAE4(EventHandlerManager *owner, const CameraCallbackPayload &payload, int key) {
    owner->addHandler(new MemberCallbackRecord<Receiver>(payload), key);
}
