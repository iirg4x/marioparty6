#include "datadir_enum.h"
#include "REL/m611DLL/angle.h"
#include "REL/m611DLL/motion_transform.h"
#include "REL/m611DLL/game_requests.h"
#include "REL/m611DLL/list_rotation.h"
#include "REL/framework/phase_duration_statics.h"
#include "REL/framework/seconds_to_frames_inline.h"
#include "REL/m611DLL/motion_effect_owner.h"
#include "REL/m611DLL/motion_position_table.h"
#include "REL/framework/random.h"
#include "REL/m611DLL/motion_object.h"
#include "REL/framework/model.h"
#include <cmath>
#include <new>
#include "REL/framework/list.h"
extern "C" {
#include "dolphin/types.h"
#include "math.h"
#include "dolphin/mtx.h"
#include "game/hu3d.h"
#include "humath.h"
}

// Active moving objects.
ListRoot<MotionObject> lbl_1_bss_78;

// Chooses a horizontal destination for a moving object.
// Nothing in this minigame calls this.
void fn_1_1CE0(Vec *position)
{
    randomizeMotionPosition(position);
}

// Creates the shared effect model and five moving instances.
MotionEffectOwner::MotionEffectOwner() : model(DATANUM(0x460000, 24))
{
    s32 index;
    s16 modelId;
    modelId = this->model;
    fn_800269E8(modelId, 0x1);
    for (index = 0; index < 5; ++index) {
        Vector3 position;

        position.x = static_cast<f32>(3000.0 *
                           (RandomFloat() - 0.5));
        position.y = static_cast<f32>(index * 80 + 700);
        position.z = static_cast<f32>(3000.0 *
                           (RandomFloat() - 0.5));
        new MotionObject(this->model, &position);
    }
}

// Deletes the moving effect instances.
MotionEffectOwner::~MotionEffectOwner()
{
    fn_1_2294();
}

// Updates every linked moving effect instance.
void MotionEffectOwner::update()
{
    fn_1_2380();
}

// Acknowledges scene requests without changing the effect models.
void MotionEffectOwner::flagEnabled(int request)
{
    acknowledgeSceneRequest(flags, request);
}

// Initializes the moving model, its position and its linked motion state.
MotionObject::MotionObject(const OwnedModel &modelSource, Vector3 *position)
    : position(*position), model(modelSource)
{
    s16 modelId;
    f32 modelScale;
    void (MotionObject::*initialUpdate)();

    this->speed = 1.0f;
    this->unk40 = 0;
    this->turnRate = 0.0f;
    this->insert4(&lbl_1_bss_78);

    modelId = this->model.id();
    modelScale = 0.15f;
    fn_800268B0(modelId, modelScale, modelScale, modelScale);
    fn_80026A44(modelId, 0x1);
    fn_80026A44(modelId, 0x40000002);
    fn_800269E8(modelId, 0x40000001);
    fn_8002D414(modelId, 0.0f);
    fn_80026B8C(modelId, 4);
    fn_80026B64(modelId, 1);
    this->heading = RandomFloat() * 360.0f - 180.0f;
    fn_1_246C(this);
    initialUpdate = &MotionObject::fn_1_25A0;
    this->callback = initialUpdate;
}

// Deletes all linked moving effect objects.
void fn_1_2294()
{
    deleteEffects(lbl_1_bss_78);
}

// Visits each moving effect object for its frame update.
void fn_1_2380()
{
    lbl_1_bss_78.visit(fn_1_23B0);
}

// Runs the object motion callback and applies its model transform.
void fn_1_23B0(MotionObject *object)
{
    (object->*object->callback)();
    object->fn_1_23F0();
}

// Applies the current heading and position to the model.
void MotionObject::fn_1_23F0()
{
    applyMotionTransform(model, heading, &position);
}

// Chooses the next motion timer, target position, speed, and turn rate.
void fn_1_246C(MotionObject *object)
{
    f32 angleScale;

    object->duration = static_cast<s32>(secondsToFrames(1.0f) +
                                        RandomFloat() * secondsToFrames(2.0f));
    object->target.x = (RandomFloat() - 0.5f) * 30.0f *
                       100.0f;
    object->target.z = (RandomFloat() - 0.5f) * 30.0f *
                       100.0f;
    object->target.y = 0.0f;
    angleScale = 1.0f;
    object->targetTurnRate = 0.1 * angleScale + angleScale * RandomFloat();
    object->targetSpeed = 1.5f + RandomFloat();
}

// Approaches the target motion settings, turns, and advances the object.
void MotionObject::fn_1_25A0()
{
    f32 targetAngle;

    if (--this->duration < 0) {
        fn_1_246C(this);
    }
    this->speed += (1.0f / 20.0f) * (this->targetSpeed - this->speed);
    this->turnRate += (1.0f / 20.0f) * (this->targetTurnRate - this->turnRate);
    targetAngle = static_cast<f32>(
        180.0 * (std::atan2(static_cast<f64>(this->target.x - this->position.x),
                            static_cast<f64>(this->target.z - this->position.z)) / M_PI));
    this->heading += fn_1_B8(targetAngle, this->heading, this->turnRate);
    fn_1_27AC(this);
}

// Advances the object along its current heading.
void fn_1_27AC(MotionObject *object)
{
    Mtx rotation;
    Vec source;
    Vec transformed;

    static f64 speedFrames = secondsToFrames(1.0f);
    source.x = 0.0f;
    source.y = 0.0f;
    source.z = static_cast<f32>(static_cast<f64>(400.0f * object->speed) / speedFrames);
    rotateForwardStep(object, rotation, source, transformed);
    object->position.x += transformed.x;
    object->position.z += transformed.z;
}
