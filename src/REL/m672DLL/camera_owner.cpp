#include "REL/framework/random.h"
#include "REL/framework/minigame_core.h"
#include "REL/framework/thread.h"
#include "dolphin/types.h"
#include <string.h>
#include "REL/m672DLL/collision_object.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event_hm.h"
#include "REL/framework/model.h"
#include "REL/framework/event_callback_inline.h"

#include "dolphin/os.h"
#include "dolphin/mtx.h"
#include "game/data.h"
#include "game/hu3d.h"
#include <stdio.h>
#include <stdlib.h>

#include "REL/m672DLL/member_handler.h"

/* No code in this module reads these constants; their values suggest three vectors. */
extern const double lbl_1_rodata_438 = 60.0;
extern const Vec lbl_1_rodata_440 = {0.0f, 1500.0f, 1500.0f};
extern const Vec lbl_1_rodata_44C = {0.0f, 0.0f, 0.0f};
extern const Vec lbl_1_rodata_458 = {0.0f, 1.0f, 0.0f};

#include "REL/m672DLL/camera_owner.h"
// Return the vector after checking that the receiver exists.
inline const Vec &EffectVector::getValue() const { ASSERT(this != 0); return value; }
// Create the camera and initialize its frame and event callback.
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
    model.setAttribute(1);
    modelId = (s16)model.modelId;
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

    registerMemberHandler<CameraOwner>(&this->callbackState,
             bindMember(std::mem_fun(&CameraOwner::fn_1_A924), this), 0);
    this->setUpdate(&CameraOwner::fn_1_A6E4);
}
// Run the selected camera update and apply the frame to the HUD camera.
void CameraOwner::update()
{
    if (updateCallback) {
        (this->*updateCallback)();
    }
    cameraApplyToHud(&camera.frame, 1);
}
// Leave the camera frame unchanged while idle.
void CameraOwner::fn_1_A6E4() {}
// Apply a fading random offset to the saved camera position and target.
void CameraOwner::fn_1_A6E8()
{
    double xOffset;
    double zOffset;
    float blendFactor;
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
    fn_1_A978(0.5f);
    fade = fade - 0.03333333333333333;
    if (!(fade > 0.1f)) {
        camera = cameraCopy;
        setUpdate(&CameraOwner::fn_1_A6E4);
    }
}
// Start the fading camera shake.
void CameraOwner::fn_1_A924(EventMessage *)
{
    fade = 1.0f;
    setUpdate(&CameraOwner::fn_1_A6E8);
}
// Move the frame position and target toward their stored destinations.
void CameraOwner::fn_1_A978(float amount)
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
// Destroy the owned camera model and callback manager.
CameraOwner::~CameraOwner() {}
