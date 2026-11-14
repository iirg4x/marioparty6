#include "REL/framework/random.h"
#include "REL/m648DLL/particle_entry.h"
#include "REL/framework/frame_time_inline.h"
#include "REL/framework/sound.h"
/* The first object of the module. MSL sqrtf's statics come from this file, so they are the ones at the start of .rodata. */
#include <cmath>

extern "C" {
#include "dolphin/types.h"
#include "game/data.h"
#include "game/memory.h"
#include "game/sprite.h"
#include "REL/framework/assertion.h"
}

#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"
extern "C" {
#include <math.h>
}
#include <new>

#include "REL/m648DLL/first_list_object.h"

#include "REL/m648DLL/managers.h"

/* Initializes the object through its member and base constructors. */
FirstListObject::FirstListObject() {}

void fn_1_618();
void fn_1_728();
void fn_1_5B4();

/* Deletes all flying objects. */
FirstListObject::~FirstListObject()
{
    fn_1_618();
}

/* Updates the flying object list. */
void FirstListObject::update()
{
    fn_1_728();
}

/* Clears the requested flag and starts all flying objects for request 2. */
void FirstListObject::flagEnabled(int bit)
{
    flags &= ~(1UL << bit);
    switch (bit) {
    case 0:
    case 1:
    case 3:
    case 4:
        break;
    case 2:
        fn_1_5B4();
        break;
    }
}

extern "C" {
#include "dolphin/mtx.h"
}

#include "datadir_enum.h"

extern "C" {
#include "game/hu3d.h"
}

#include "REL/framework/model.h"

extern "C" {
#include "game/audio.h"
}

#include "REL/m648DLL/positioned_model_owner.h"

#include "REL/m648DLL/flying_object.h"

/* Links the model pair and initializes its position, motion and callback. */
FlyingObject::FlyingObject(Vec *position)
    : modelA(DATANUM(0x6b0000, 5)), modelB(DATANUM(0x6b0000, 6))
{
    s16 currentModel;

    this->stopped = 0;
    this->complete = 0;
    this->angle = 0.0f;
    this->timer = 0;
    insert4(&lbl_1_bss_18);
    this->startPosition = *position;
    this->fade = 0.0f;
    currentModel = this->modelA;
    fn_80026B8C(currentModel, 4);
    fn_80026B64(currentModel, 1);
    fn_800269E8(currentModel, 1);
    fn_80026E44(currentModel);
    fn_80026F9C(currentModel);
    currentModel = this->modelB;
    fn_80026B8C(currentModel, 4);
    fn_80026B64(currentModel, 1);
    fn_80026A44(currentModel, 1);
    this->fn_1_8B4();
}

/* Releases the object through its member and base destructors. */
FlyingObject::~FlyingObject() {}

/* Puts every object back at its start. */
void fn_1_5B4()
{
    FlyingObject *object = lbl_1_bss_18.front();
    Node *end = lbl_1_bss_18.end_node();
    for (; object != end; object = object->next()) {
        object->fn_1_8B4();
    }
}

/* Deletes every object. */
void fn_1_618()
{
    while (!lbl_1_bss_18.empty()) {
        delete lbl_1_bss_18.back();
    }
}

void fn_1_7A4(FlyingObject *object);

/* Updates every object. */
void fn_1_728()
{
    lbl_1_bss_18.visit(fn_1_7A4);
}

/* Clears the first model display attribute. */
void FlyingObject::fn_1_758()
{
    s16 currentModel;

    currentModel = this->modelA;
    fn_80026A44(currentModel, (1 << 0));
}

/* Runs the flying object callback, updates its models and advances its timer. */
void fn_1_7A4(FlyingObject *object)
{
    (object->*object->callback)();
    object->fn_1_7F0();
    object->timer++;
}

/* Wraps the rotation angle and applies both model transforms and transparency. */
void FlyingObject::fn_1_7F0()
{
    f64 angle;
    Vec rotation;

    if (this->angle > 180.0f) {
        this->angle = this->angle - 360.0f;
    }
    angle = this->angle;
    rotation.x = (f32)angle;
    rotation.y = 0.0f;
    rotation.z = 0.0f;
    modelA.setPosition(&position);
    modelA.setRotation(&rotation);
    modelA.setTransparency(fade);
    modelB.setPosition(&resetPosition);
}

/* Restores the starting position and resets the flying object animation state. */
inline void FlyingObject::fn_1_8B4()
{
    s16 currentModel;
    Callback callback;

    this->position.x = this->startPosition.x;
    this->position.y = this->startPosition.y;
    this->position.z = this->startPosition.z;
    this->resetPosition.x = this->startPosition.x;
    this->resetPosition.y = this->startPosition.y;
    this->resetPosition.z = this->startPosition.z;
    this->position.y += 7.0f;
    currentModel = this->modelA;
    fn_80026A44(currentModel, (1 << 0));
    callback = &FlyingObject::fn_1_BA8;
    this->callback = callback;
}

/* Starts the object's flight toward the camera; FlyingObject::fn_1_BF0 moves it from then on. */
void fn_1_984(FlyingObject *object, u8 stopped)
{
    Vec cameraPosition;
    Vec cameraTarget;
    Vec cameraUp;
    FlyingObject::Callback callback;

    object->stopped = stopped;
    object->complete = 0;
    fn_80027CD0(1, &cameraPosition, &cameraTarget, &cameraUp);
    object->groundHeight = (s32)cameraPosition.z;
    PSVECSubtract(&cameraPosition, &object->position, &object->movement);
    PSVECScale(&object->movement, &object->movement, 0.03f);
    object->movement.y += 5.44813888888889;
    object->updateCount = 0;
    object->audioHandle = HuAudFXEmiterPlay(MSM_SE_MG648_OBJECT_FLIGHT_LOOP, &object->position);
    soundPlayAt(1, MSM_SE_MG648_OBJECT_FLIGHT_START, &object->position);
    new ColorParticles(&object->position);
    callback = &FlyingObject::fn_1_BF0;
    object->callback = callback;
}

/* Starts the object's bounce; FlyingObject::fn_1_E4C moves it from then on. */
void fn_1_AB8(FlyingObject *object)
{
    Vec cameraPosition;
    Vec cameraTarget;
    Vec cameraUp;
    FlyingObject::Callback callback;

    object->complete = 0;
    fn_80027CD0(1, &cameraPosition, &cameraTarget, &cameraUp);
    object->groundHeight = (s32)cameraPosition.z;
    object->movement.x = 3.0f * (RandomFloat() - 0.5f);
    object->movement.y = 4.0861044f;
    object->movement.z = 1.6666666f;
    object->updatesRemaining = 0x78;
    soundPlayAt(1, MSM_SE_MG648_OBJECT_BOUNCE_START, &object->position);
    callback = &FlyingObject::fn_1_E4C;
    object->callback = callback;
}

/* Increases the fade value to one and marks the animation complete. */
void FlyingObject::fn_1_BA8()
{
    if ((fade += 0.13333334f) > 1.0f) {
        fade = 1.0f;
        complete = 1;
    }
}

/* Applies gravity and movement, then changes the callback at the camera depth limit. */
void FlyingObject::fn_1_BF0()
{
    this->movement.y = (f32)((f64)this->movement.y - 0.27240694444444447);
    PSVECAdd(&this->position, &this->movement, &this->position);
    if (this->position.z > (f32)this->groundHeight - 20.0f) {
        this->fade = 0.0f;
        if (this->stopped == 0) {
            this->fn_1_8B4();
        } else {
            Callback callback = &FlyingObject::fn_1_E00;
            this->callback = callback;
        }
        HuAudFXStop(this->audioHandle);
    } else {
        HuAudFXEmiterUpDate(this->audioHandle, &this->position);
    }
    this->angle += 20.0f;
    this->updateCount += 1;
    if ((this->updateCount & 3) == 0) {
        new PositionedModelOwner(&this->position);
    }
}

/* Sets the first model display attribute. */
void FlyingObject::fn_1_E00()
{
    s16 currentModel;

    currentModel = this->modelA;
    fn_800269E8(currentModel, (1 << 0));
}

/* Moves and bounces the object while its animation timer runs. */
void FlyingObject::fn_1_E4C()
{
    movement.y -= 0.27240694444444447;
    PSVECAdd(&position, &movement, &position);
    if (movement.y < 0.0f && position.y < 3.0f) {
        position.y = 3.0f;
        movement.y *= -0.8;
    }
    if (--updatesRemaining < 0) {
        fn_1_8B4();
    } else {
        f32 progress = (f32)updatesRemaining;
        fade = 0.05f * progress;
        if (fade > 1.0f)
            fade = 1.0f;
    }
    angle += 10.0f;
}

extern "C" {
ListRoot<FlyingObject> lbl_1_bss_18;
}
