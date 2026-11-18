#include "REL/framework/event_callback_adapter_inline.h"
#include "REL/framework/event_callback_registration.h"
#include "REL/framework/event_callback_inline.h"
#include "REL/framework/event_hm.h"
#include "REL/m628DLL/scene_model.h"
#include "dolphin/types.h"

#include "dolphin/mtx.h"

extern "C"
{
#include "game/hu3d.h"
}

#include "REL/framework/model.h"
#include <functional>

// Creates a scene model and registers its movement event.
SceneModelObject::SceneModelObject(OwnedModel *model, u8 mode)
    : GameObject(0x20), model(0), mode(mode), motionTime(0.0f), callbacks(0)
{
    this->model.reset(model);
    this->model->setAttribute(((1 << 30) | (1 << 0)));
    this->position.x = 0.0f;
    this->position.y = 0.0f;
    this->position.z = 0.0f;
    this->model->setLayer(2);
    registerEventCallback(
        &this->callbacks, std::bind1st(std::mem_fun(&SceneModelObject::fn_1_13F04), this), 0);
}

// Moves the model and advances its animation from the event velocity.
void SceneModelObject::fn_1_13F04(const EventPacket &input)
{
    f32 horizontalSpeed;
    f32 depthSpeed;
    Vec *velocity;

    velocity = (Vec *)input.context;
    if (this->mode == 1)
    {
        this->position.x = this->position.x + velocity->x;
    }
    else
    {
        this->position.z = this->position.z + velocity->z;
    }
    if (this->mode == 1)
    {
        if (velocity->x < 0.0f)
        {
            horizontalSpeed = -velocity->x;
        }
        else
        {
            horizontalSpeed = velocity->x;
        }
        if (horizontalSpeed < 0.2f)
        {
            return;
        }
        if (velocity->x < 0.0f)
        {
            this->motionTime = this->motionTime - 1.0f;
        }
        else
        {
            this->motionTime += 1.0f;
        }
    }
    else
    {
        if (velocity->z < 0.0f)
        {
            depthSpeed = -velocity->z;
        }
        else
        {
            depthSpeed = velocity->z;
        }
        if (depthSpeed < 0.2f)
        {
            return;
        }
        if (velocity->z < 0.0f)
        {
            this->motionTime = this->motionTime - 1.0f;
        }
        else
        {
            this->motionTime += 1.0f;
        }
    }
    if (this->motionTime < 0.0f)
    {
        this->motionTime = 60.0f;
    }
    if (this->motionTime > 60.0f)
    {
        this->motionTime = 0.0f;
    }
}

void fn_1_14124(SceneModelObject *);

// Applies the stored model position and animation time.
void SceneModelObject::update()
{
    this->model->setPosition(&position);
    fn_1_14124(this);
}

// Applies the scene model animation time.
void fn_1_14124(SceneModelObject *target)
{
    fn_8002D414(*target->model, target->motionTime);
}

// Releases the model and its event registration.
SceneModelObject::~SceneModelObject() {}
